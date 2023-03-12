/*
	File:		MoreMPLog.c

	Contains:	MP-safe logging.

	Written by:	Quinn

	Copyright:	Copyright (c) 2000-2001 by Apple Computer, Inc., All Rights Reserved.

	Disclaimer:	IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc.
				("Apple") in consideration of your agreement to the following terms, and your
				use, installation, modification or redistribution of this Apple software
				constitutes acceptance of these terms.  If you do not agree with these terms,
				please do not use, install, modify or redistribute this Apple software.

				In consideration of your agreement to abide by the following terms, and subject
				to these terms, Apple grants you a personal, non-exclusive license, under Apple’s
				copyrights in this original Apple software (the "Apple Software"), to use,
				reproduce, modify and redistribute the Apple Software, with or without
				modifications, in source and/or binary forms; provided that if you redistribute
				the Apple Software in its entirety and without modifications, you must retain
				this notice and the following text and disclaimers in all such redistributions of
				the Apple Software.  Neither the name, trademarks, service marks or logos of
				Apple Computer, Inc. may be used to endorse or promote products derived from the
				Apple Software without specific prior written permission from Apple.  Except as
				expressly stated in this notice, no other rights or licenses, express or implied,
				are granted by Apple herein, including but not limited to any patent rights that
				may be infringed by your derivative works or by other works in which the Apple
				Software may be incorporated.

				The Apple Software is provided by Apple on an "AS IS" basis.  APPLE MAKES NO
				WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED
				WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS FOR A PARTICULAR
				PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND OPERATION ALONE OR IN
				COMBINATION WITH YOUR PRODUCTS.

				IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL OR
				CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
				GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
				ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION, MODIFICATION AND/OR DISTRIBUTION
				OF THE APPLE SOFTWARE, HOWEVER CAUSED AND WHETHER UNDER THEORY OF CONTRACT, TORT
				(INCLUDING NEGLIGENCE), STRICT LIABILITY OR OTHERWISE, EVEN IF APPLE HAS BEEN
				ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

	Change History (most recent first):

$Log: MoreMPLog.c,v $
Revision 1.6  2002/11/08 23:38:27         
Include our prototype early to flush out any missing dependencies. Convert nil to NULL. Convert MoreAssertQ to assert.

Revision 1.5  2001/11/07 15:53:20         
Tidy up headers, add CVS logs, update copyright.


         <4>     21/9/01    Quinn   Changes for CWPro7 Mach-O build.
         <3>      9/7/01    Quinn   Removed MPLogPrintfSlow because of problems explained in header
                                    file. Removed workaround for Mac OS X Public Beta bugs.
         <2>     15/2/01    Quinn   Changes to work with Project Builder. Because FSp_fopen is a
                                    Metrowerks thing we have to define our own version of the
                                    routine when building with PB.
         <1>     7/11/00    Quinn   First checked in.
*/

/////////////////////////////////////////////////////////////////////

// Our prototypes

#include "MoreMPLog.h"

// Mac OS Interfaces

#if ! MORE_FRAMEWORK_INCLUDES
	#include <Multiprocessing.h>
	#include <DriverSynchronization.h>
	#include <Gestalt.h>
#endif

// Standard C interfaces

#include <string.h>
#include <stdarg.h>
#include <stdio.h>

// MIB interfaces

#include "MoreMemory.h"

/////////////////////////////////////////////////////////////////////
#pragma mark ----- Core Logging Routines -----

// CFM export these data structures so that we can find them in MacsBug.
// Have to use a macro so that the declspecs don’t confuse gcc.

#if defined(__MWERKS__) && TARGET_RT_MAC_CFM
    #define MOREMPLOGVARIABLEEXPORT __declspec (export)
#else
    #define MOREMPLOGVARIABLEEXPORT 
#endif

MOREMPLOGVARIABLEEXPORT ByteCount gMPLogBufferSize = 0;

MOREMPLOGVARIABLEEXPORT UInt8 *gMPLogBuffer = NULL;

MOREMPLOGVARIABLEEXPORT UInt32 gMPLogCurrentBufferIndex = 0;

const char *gBufferTerminator = "___End Buffer___";

extern pascal OSStatus InitMPLog(ByteCount logBufferSize)
	// See comment in header file.
{
	OSStatus err;
	
	// The log buffer size must be a power of 2.  This is because the 
	// gMPLogCurrentBufferIndex is a free running counter.  I simply 
	// mod the counter with the buffer size to get the offset into the  
	// buffer.  When gMPLogCurrentBufferIndex wraps around of the 4 GB 
	// mark, things work OK as long as the buffer size is a power of 2.
	
	assert((logBufferSize & (logBufferSize - 1)) == 0);
	
	// Allocate the buffer.
	
	err = noErr;
	gMPLogBufferSize = logBufferSize;
	gMPLogCurrentBufferIndex = 0;
	gMPLogBuffer = (UInt8 *) NewPtrClear(gMPLogBufferSize + strlen(gBufferTerminator));
	if (gMPLogBuffer == NULL) {
		err = memFullErr;
	}
	if (err == noErr) {

		// Copy the terminator into the end of the log buffer.

		BlockMoveData(gBufferTerminator, &gMPLogBuffer[gMPLogBufferSize], strlen(gBufferTerminator));

		// If we're not building Carbon, hold the memory so that we can 
		// log at non-deferred interrupt time.  Carbon builds don't have 
		// access to HoldMemory; this isn't an issue on Mac OS X but could 
		// cause problems with CarbonLib on Mac OS 9.
		
		#if ! TARGET_API_MAC_CARBON
			err = SafeHoldMemory(gMPLogBuffer, gMPLogBufferSize);
			if (err != noErr) {
				DisposePtr( (Ptr) gMPLogBuffer);
				assert(MemError() == noErr);
				gMPLogBuffer = NULL;
			}
		#endif
	}
	
	// If we're building a debug version, the following code will define a 
	// MacsBug macro called "MPLog" which will find the last log entry 
	// marker in the buffer (the '•' character) and then display the last 
	// 256 bytes of the log.  This doesn't always work perfectly (for example, 
	// if the log has just wrapped), but it was easy to implement and it works 
	// most of the time.
	
	// Don't do this when building Mach-O because there's no MacsBug to 
	// listen to the macro definition.
	
	#if MORE_DEBUG && !TARGET_RT_MAC_MACHO
		if (err == noErr) {
			char str[256];
			
			sprintf(&str[1], "; mc MPLog \"f %08X %08X '•' ; dm .-100 110\" ; g", gMPLogBuffer, gMPLogBufferSize);
			str[0] = strlen(&str[1]);
			DebugStr( (StringPtr) str);
		}
	#endif

	return err;
}

extern pascal void TermMPLog(void)
	// See comment in header file.
{
	UInt8 *tmp;
	
	if (gMPLogBuffer != NULL) {
		tmp = gMPLogBuffer;
		gMPLogBuffer = NULL;
		
		#if ! TARGET_API_MAC_CARBON
			{
				OSStatus junk;
				
				junk = SafeUnholdMemory(gMPLogBuffer, gMPLogBufferSize);
				assert(junk == noErr);
			}
		#endif
		
		DisposePtr( (Ptr) tmp);
		assert(MemError() == noErr);
	}
}

extern pascal void MPLogText(const UInt8 *text, ByteCount len)
	// See comment in header file.
{
	UInt32 lastCharIndex;
	
	assert(gMPLogBuffer != NULL);			// You forgot to call InitMPLog.
	
	// Allocate the next N bytes in the log buffer by atomically 
	// adding to the index.
	
	lastCharIndex = AddAtomic(len + 2, (SInt32 *) &gMPLogCurrentBufferIndex) + len + 2;

	// Fill in the log buffer that we allocated with the text.
	
	// Plotting this bullet is not 100% certain but it's
	// sufficiently useful in helping you find the last logged text
	// that I'm prepared to accept the uncertainty.
	
	gMPLogBuffer[lastCharIndex % gMPLogBufferSize] = '•';
	lastCharIndex -= 1;
	
	gMPLogBuffer[lastCharIndex % gMPLogBufferSize] = '»';
	while (len != 0) {
		len -= 1;
		lastCharIndex -= 1;
		gMPLogBuffer[lastCharIndex % gMPLogBufferSize] = text[len];
	}
	lastCharIndex -= 1;
	gMPLogBuffer[lastCharIndex % gMPLogBufferSize] = '«';
}

#if defined(__MWERKS__) && !TARGET_RT_MAC_MACHO
    #include <FSp_fopen.h>
#elif TARGET_RT_MAC_MACHO

    // Mach-O compiler -- You must be building for Carbon on Mac OS X, 
    // so we’ll just use FSRefMakePath to make the path to pass to fopen.
    
    #include <sys/param.h>
    
    static FILE	*FSp_fopen(const FSSpec *fss, const char *attr)
    {
        OSStatus err;
        FSRef fsRef;
        char *path;
        FILE *result;
        
        result = NULL;
        err = noErr;
        path = (char *) malloc(MAXPATHLEN);
        if (path == NULL) {
            err = memFullErr;
        }
        if (err == noErr) {
            err = FSpCreate(fss, 'CWIE', 'TEXT', smSystemScript);
            if (err == dupFNErr) {
                err = noErr;
            }
        }
        if (err == noErr) {
            err = FSpMakeFSRef(fss, &fsRef);
        }
        if (err == noErr) {
            err = FSRefMakePath(&fsRef, (UInt8 *) path, MAXPATHLEN);
        }
        if (err == noErr) {
            result = fopen(path, attr);
        }
        
        // Clean up.
        
        if (path != NULL) {
            free(path);
        }

        return result;
    }

#else
    #error What compiler are you using?
#endif

extern pascal OSStatus MPLogWriteToFile(const FSSpec *fss)
	// See comment in header file.
{
	OSStatus err;
	FILE *output;
	SInt32 rowMax;
	SInt32 row;
	SInt32 col;
	UInt32 index;
	UInt8  ch;

	assert(gMPLogBuffer != NULL);			// You forgot to call InitMPLog.

	err = noErr;	
	(void) FSpDelete(fss);
	output = FSp_fopen(fss, "w");
	if (output == NULL) {
		err = -1;
	}
	
	// Print the log buffer to the file.  Note that the output is an 
	// exact match of the MacsBug "dm" command.  This is not a coincidence.
	
	if (err == noErr) {
		rowMax = (gMPLogBufferSize + 15) / 16;
		index = 0;
		for (row = 0; row < rowMax; row++) {
			fprintf(output, "  %08lX  ", index);
			for (col = 0; col < 16; col+=2) {
				if ((index + col) < gMPLogBufferSize) {
					fprintf(output, "%04X ", *((UInt16 *) &gMPLogBuffer[index + col]));
				} else {
					fprintf(output, "     ");
				}
			}
			fprintf(output, " ");
			for (col = 0; col < 16; col++) {
				if ((index + col) < gMPLogBufferSize) {
					ch = gMPLogBuffer[index + col];
					if (ch < 32) {
						ch = '.';
					}
					fprintf(output, "%c", ch);
				} else {
					fprintf(output, " ");
				}
			}
			fprintf(output, "\n");
			index += 16;
		}
		(void) fclose(output);
	}
	return err;
}

/////////////////////////////////////////////////////////////////
#pragma mark ----- Log Points -----

#if MORE_DEBUG

	static UInt32 gLogMask = 0;

	extern pascal void MPLogSetMask(UInt32 logMask)
	{
		gLogMask = logMask;
	}

	extern pascal UInt32 MPLogGetMask(void)
	{
		return gLogMask;
	}

	// This section of code makes heavy use of sprintf.
	// I'm a little worried about this because there's no guarantee 
	// that sprintf is MP safe.  For the moment I'm going to let it 
	// slide, but it would be good to revisit.
	
	extern pascal void MPLog(UInt32  module, OSType tag)
	{
		char tmpStr[256];
		
		if ( gLogMask & (1 << module) ) {
			(void) sprintf(tmpStr, "[%08lx]%4.4s", (UInt32) MPCurrentTaskID(), (char *) &tag);
			MPLogText( (UInt8 *) tmpStr, strlen(tmpStr));       
		}
	}

	extern pascal void MPLog1(UInt32 module, OSType tag, const void *p1)
	{
		char tmpStr[256];
		
		if ( gLogMask & (1 << module) ) {
			(void) sprintf(tmpStr, "[%08lx]%4.4s %08lx   ", (UInt32) MPCurrentTaskID(), (char *) &tag, (UInt32) p1);
			MPLogText( (UInt8 *) tmpStr, strlen(tmpStr));       
		}
	}

	extern pascal void MPLog2(UInt32 module, OSType tag, const void *p1, const void *p2)
	{
		char tmpStr[256];
		
		if ( gLogMask & (1 << module) ) {
			(void) sprintf(tmpStr, "[%08lx]%4.4s %08lx %08lx  ", (UInt32) MPCurrentTaskID(), (char *) &tag, (UInt32) p1, (UInt32) p2);
			MPLogText( (UInt8 *) tmpStr, strlen(tmpStr));       
		}
	}

	extern pascal void MPLog3(UInt32 module, OSType tag, const void *p1, const void *p2, const void *p3)
	{
		char tmpStr[256];
		
		if ( gLogMask & (1 << module) ) {
			(void) sprintf(tmpStr, "[%08lx]%4.4s %08lx %08lx %08lx ", (UInt32) MPCurrentTaskID(), (char *) &tag, (UInt32) p1, (UInt32) p2, (UInt32) p3);
			MPLogText( (UInt8 *) tmpStr, strlen(tmpStr));       
		}
	}

	extern pascal void MPLog4(UInt32 module, OSType tag, const void *p1, const void *p2, const void *p3, const void *p4)
	{
		char tmpStr[256];
		
		if ( gLogMask & (1 << module) ) {
			(void) sprintf(tmpStr, "[%08lx]%4.4s %08lx %08lx %08lx %08lx", (UInt32) MPCurrentTaskID(), (char *) &tag, (UInt32) p1, (UInt32) p2, (UInt32) p3, (UInt32) p4);
			MPLogText( (UInt8 *) tmpStr, strlen(tmpStr));       
		}
	}

#endif

/////////////////////////////////////////////////////////////////////
#pragma mark ----- Utility printf Routines -----

extern void MPLogPrintf(const char *format, ...)
	// See comment in header file.
{
	char 	tmpStr[256];
	va_list args;                            

	va_start(args, format);

	vsprintf(tmpStr, format, args);
	MPLogText( (UInt8 *) tmpStr, strlen(tmpStr));       
	
	va_end(args);
}

#if 0

// See comment in header file.

// MPLogPrintfSlow uses MPRemoteCall to get back to system task 
// time in order to print to the console.  It needs multiple 
// parameters to do this, which it collects into the BluePrinterArgs 
// parameter block.

enum {
	kBluePrinterArgsMagic = 'BlPr'
};

struct BluePrinterArgs {
	OSType		 magic;			// must be kBluePrinterArgsMagic
	const char * format;
	va_list      args;
};
typedef struct BluePrinterArgs BluePrinterArgs;

static void *BluePrinter(void *parameter)
{
	BluePrinterArgs *blue;
	
	blue = (BluePrinterArgs *) parameter;
	assert(blue->magic == kBluePrinterArgsMagic);
	
	vprintf(blue->format, blue->args);
	fflush(stdout);
	
	return NULL;
}

extern void MPLogPrintfSlow(const char *format, ...)
	// See comment in header file.
{
	BluePrinterArgs blue;
	va_list  		args;

	va_start(args, format);
	            
	blue.magic  = kBluePrinterArgsMagic;
	blue.format = format;
	blue.args   = args;	

	(void) MPRemoteCall(BluePrinter, &blue, kMPOwningProcessRemoteContext);
	
	va_end(args);
}

#endif
