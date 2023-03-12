/*
	File:		MoreMPLog.h

	Contains:	MP-safe logging.

	Written by:	Quinn

	Copyright:	Copyright (c) 2000-2001 by Apple Computer, Inc., All Rights Reserved.

	Disclaimer:	IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc.
				("Apple") in consideration of your agreement to the following terms, and your
				use, installation, modification or redistribution of this Apple software
				constitutes acceptance of these terms.  If you do not agree with these terms,
				please do not use, install, modify or redistribute this Apple software.

				In consideration of your agreement to abide by the following terms, and subject
				to these terms, Apple grants you a personal, non-exclusive license, under Apple�s
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

$Log: MoreMPLog.h,v $
Revision 1.6  2002/11/08 23:37:57         
When using framework includes, explicitly include the frameworks we need.

Revision 1.5  2001/11/07 15:53:23         
Tidy up headers, add CVS logs, update copyright.


         <4>     21/9/01    Quinn   Changes for CWPro7 Mach-O build.
         <3>      9/7/01    Quinn   Removed MPLogPrintfSlow because of problems I'm having on Mac OS
                                    9.1.
         <2>      5/7/01    Quinn   Delete space between MPLog and ( in macro definition. It
                                    previously didn't do what I wanted it to do, although it was
                                    somewhat prettier.
         <1>     7/11/00    Quinn   First checked in.
*/

#pragma once

/////////////////////////////////////////////////////////////////

// MoreIsBetter Setup

#include "MoreSetup.h"

// Mac OS interfaces

#if MORE_FRAMEWORK_INCLUDES
	#include <CoreServices/CoreServices.h>
#else
	#include <Files.h>
#endif

/////////////////////////////////////////////////////////////////

#ifdef __cplusplus
extern "C" {
#endif

/////////////////////////////////////////////////////////////////////

// Overview
// --------
// This module implements a very low-level logging mechanism that is safe 
// to call from /all/ contexts, including MP tasks and interrupts.  It was 
// originally written to help develop OTMP (another part of MoreIsBetter) but 
// it is generally useful.
//
// The core design is stolen directly from "InterruptSafeDebug", another 
// MoreIsBetterModule.  The main change is that this module logs into 
// memory rather than directly to the screen.  I should unify these two 
// modules eventually, but at the moment I'm concentrating on shipping 
// OTMP.
//
// The log data goes into a log buffer pointed to by gMPLogBuffer.
// After a crash you can find this buffer in MacsBug and then look 
// through the buffer for a '�' character to find the last item logged.
// When you start up the module it defines a MacsBug macro, MPLog, to do 
// this for you.
//
// For InterfaceLib-based code you can call this library when paging is 
// unsafe, as long as your guarantee that the library's code and global 
// data is held.  This is typically the case for device driver code.  
// For Carbon code, paging must be safe when you call this module. 
// Paging is always safe on Mac OS X, and is typically safe in application-
// level code on traditional Mac OS.  See Technote 1094 "Virtual Memory 
// Application Compatibility" for a list of situations when paging is 
// unsafe on traditional Mac OS.

/////////////////////////////////////////////////////////////////////
// Core Logging Routines

extern pascal OSStatus InitMPLog(ByteCount logBufferSize);
	// Initialises the MP-safe log to contain a log of logBufferSize 
	// bytes.  The buffer is allocated in the current zone using NewPtrClear.
	// logBufferSize must be a power of 2.

extern pascal void TermMPLog(void);
	// Shuts down the MP-safe log.  Do not call this routine while 
	// there are MP threads running that are logging data.
	
extern pascal void MPLogText(const UInt8 *text, ByteCount len);
	// Log's the text described by text and len to the buffer.

extern pascal OSStatus MPLogWriteToFile(const FSSpec *fss);
	// Writes the MP log to the file pointed to by fss, 
	// overwriting any data that was there previously.  This 
	// routine is not coordinated with the logging, so if 
	// there is ongoing logging happening while the write is 
	// in progress you will get weird results.

/////////////////////////////////////////////////////////////////////
// Log Points

// The followed routines let you easily define log points throughout 
// your code.  They are conditionalised with the MORE_DEBUG macro so 
// the log points go away when you compile a non-debug version.  They 
// are runtime conditionalised by a mask, so you can selectively enable 
// logging from a particular part of your code at runtime.  The "module"
// parameter to each MPLog routine is a bit number in this mask.  If the 
// bit is set, the log entry is created.
//
// IMPORTANT: The default value for the log mask is 0, so you'll have to 
// set it yourself if you expect to see anything in the log.
//
// Log points are identified by a tag, ie an OSType that you can easily 
// recognise.  I use an OSType rather than a string because it's fast 
// and simple.  This module encourages you to use tags of the form '>Foo'
// for routine entry points and '<Foo' for routine exit points.  MPLogTagReturn 
// takes an entry point tag and returns the equivalent return tag.
//
// Log points can take up to four parameters.  These parameters are 
// written to the log as hex.

#define MPLogTagReturn(tag) ( ((tag) & 0x00FFFFFF) | ('<' << 24) )

#if MORE_DEBUG

	extern pascal void   MPLogSetMask(UInt32 logMask);
		// Sets the log mask.  Only log points whose bit is set in this 
		// mask will actually appear in the log.
		
	extern pascal UInt32 MPLogGetMask(void);
		// Returns the mask last set with MPLogSetMask.
	
	extern pascal void MPLog(UInt32 module, OSType tag);
	extern pascal void MPLog1(UInt32 module, OSType tag, const void *p1);
	extern pascal void MPLog2(UInt32 module, OSType tag, const void *p1, const void *p2);
	extern pascal void MPLog3(UInt32 module, OSType tag, const void *p1, const void *p2, const void *p3);
	extern pascal void MPLog4(UInt32 module, OSType tag, const void *p1, const void *p2, const void *p3, const void *p4);
		// If the bit whose number is "module" is set in the log mask, 
		// create a log entry recording "tag" and "pX".
		// Safe to call from all execution environments.
		// You must initialise the log buffer, using InitMPLog, 
		// before calling this routine.

#else

	// Macros so that non-debug code will compile.
	
	#define MPLogSetMask(logMask)
	#define MPLogGetMask()			0

	#define MPLog(module, tag)
	#define MPLog1(module, tag, p1)
	#define MPLog2(module, tag, p1, p2)
	#define MPLog3(module, tag, p1, p2, p3)
	#define MPLog4(module, tag, p1, p2, p3, p4)

#endif

/////////////////////////////////////////////////////////////////////
// Utility printf Routines

extern void MPLogPrintf(const char *format, ...);
	// Like printf, but result goes to the MP log.
	// Safe to call from all execution environments.
	// You must initialise the log buffer, using InitMPLog, 
	// before calling this routine.
	// 
	// The fully expanded string must be 256 characters or less.

#if 0

// This routine is fatally broken and I'm not entirely sure 
// why.  Something about the MPRemoteCall is causing the MP 
// task to wedge.  I believe this is a bug in Mac OS 9.1 
// [2607064] because it works just fine on Mac OS 9.0.4 and 
// pre-release versions of Mac OS 9.2.  Until I figure this 
// out I'm just not going use this routine.

extern void MPLogPrintfSlow(const char *format, ...);
	// MP-safe version of printf.  Result goes to console.
	// The routine is slow because it does an MPRemoteCall to 
	// do the printing, which means the MP task waits until 
	// the application is scheduled and can run the called 
	// routine before it unblocks.
	//
	// You cannot call this routine from an interrupt. You can only
	// call it from system task code or from an MP task.
	//
	// You don't need to call InitMPLog before calling this routine.
	//
	// Doesn't strictly belong in this module, but it was a convenient 
	// place to put it.

#endif

#ifdef __cplusplus
}
#endif
