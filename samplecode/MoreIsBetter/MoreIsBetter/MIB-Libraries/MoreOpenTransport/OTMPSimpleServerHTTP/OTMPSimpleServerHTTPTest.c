/*
	File:		OTMPSimpleServerHTTPTest.c

	Contains:	A test program for the simple HTTP server code (using MP tasks).

	Written by:	Quinn "The Eskimo!"

	Copyright:	Copyright (c) 1997-2001 by Apple Computer, Inc., All Rights Reserved.

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

$Log: OTMPSimpleServerHTTPTest.c,v $
Revision 1.11  2002/11/08 23:44:57         
Include <Files.h> before <OpenTransportProviders.h> because of a bug in UI. Convert nil to NULL. Convert MoreAssertQ to assert.

Revision 1.10  2001/11/07 15:57:09         
Tidy up headers, add CVS logs, update copyright.


         <9>     21/9/01    Quinn   Fix Project Builder warning.
         <8>     21/9/01    Quinn   Version 1.0a5 update.
         <7>      9/7/01    Quinn   More MPLogPrintfSlow elimination. Also added a UI command for
                                    printing the number of active threads.
         <6>      5/7/01    Quinn   Don't init BlueActions because OTMP now does it for us.
         <5>     15/2/01    Quinn   Minor tweaks to get it building under Project Builder.
         <4>      8/2/01    Quinn   [2619462] The fix for the MP bug is in CarbonLib 1.2.5.
         <3>     5/12/00    Quinn   Looks like the bug fix won't make it into CarbonLib 1.2.
         <2>    10/11/00    Quinn   Call OTMPGetCanRunStatus and refuse to run on unsupported
                                    systems.
         <1>     7/11/00    Quinn   First checked in.
*/

/////////////////////////////////////////////////////////////////

// MIB Setup

#include "MoreSetup.h"

// Mac OS Interfaces

#if ! MORE_FRAMEWORK_INCLUDES
	#include <Files.h>
	#include <OpenTransportProviders.h>
	#include <Gestalt.h>
	#include <Multiprocessing.h>
	#include <Folders.h>
#endif

// Standard C interfaces

#include <stdio.h>

// MIB prototypes

#include "MoreProcesses.h"
#include "OTMP.h"

// Prototypes for the actual core HTTP server code.

#include "MoreMPLog.h"
#include "OTMPSimpleServerHTTP.h"

/////////////////////////////////////////////////////////////////
// The only way to tell whether OT supports IP single link
// multihoming is to check the version number.  The feature was
// added in OT 1.3.  The initialisation code sets 
// gHaveIPSingleLinkMultihoming depending on the version number
// to avoid the rest of the code having to call Gestalt repeatedly.

enum
{
	kOTIPSingleLinkMultihomingVersion = 0x01300000
};

static Boolean gHaveIPSingleLinkMultihoming;

/////////////////////////////////////////////////////////////////

// When this boolean is to set true, this module assumes that the
// host application is trying to quit and all the threads created by
// this module start dying.  See the associated comment in the
// YieldingNotifier routine.

static Boolean gQuitNow = false;

static Boolean gServerStarted = false;

/////////////////////////////////////////////////////////////////

static OSErr FSpGetCatInfo(FSSpecPtr fss, short ioFDirIndex, CInfoPBPtr cpb)
	// A simple wrapper for GetCatInfo.
{
	cpb->hFileInfo.ioVRefNum = fss->vRefNum;
	cpb->hFileInfo.ioDirID = fss->parID;
	cpb->hFileInfo.ioNamePtr = fss->name;
	cpb->hFileInfo.ioFDirIndex = ioFDirIndex;
	return PBGetCatInfoSync(cpb);
}

/////////////////////////////////////////////////////////////////

// MP tasks are started with a single parameter.  Because we need 
// more than one parameter for our HTTPServerProc task, we pack them 
// into a parameter block (ServerContext).

struct ServerContext {
	InetHost 		ipAddr;
	FSVolumeRefNum	rootVRefNum;
	UInt32			rootDirID;
};
typedef struct ServerContext ServerContext;

static OSStatus HTTPServerProc(void *param)
	// This routine is the initial start routine for the server's 
	// preemptive thread.  It unpacks the parameters from the 
	// context then calls our HTTP server module to execute the server.
{
	OSStatus 	  err;
	ServerContext *context;
	
	context = (ServerContext *) param;

	err = RunHTTPServerMP(context->ipAddr, context->rootVRefNum, context->rootDirID);

	MPFree(context);
	
	return err;
}

static OSStatus RunOneServer(InetHost ipAddr)
	// This routine is the main line of the thread that runs
	// an HTTP server.  ipAddr is the address on which the
	// server is listening.
	//
	// The routine uses a directory whose name is the
	// dotted decimal string representation of ipAddr as the
	// root directory of the HTTP server.
{
	OSStatus 	err;
	OSStatus 	junk;
	Str255 		ipAddrString;
	FSSpec 		dirSpec;
	CInfoPBRec 	cpb;
	ServerContext *context;
	MPTaskID    junkServerThread;
	
	// Allocate a context for the preemptive thread.
	
	err = noErr;
	context = (ServerContext *) MPAllocateAligned(sizeof(*context), kMPAllocateDefaultAligned, kNilOptions);
	if (context == NULL) {
		err = memFullErr;
	}
	
	// Fill out the context.  We do this here because it needs 
	// to use services that aren't MP-safe.
	
	if (err == noErr) {
		context->ipAddr = ipAddr;
		
		// Get ipAddr as a dotted decimal Pascal string.
		OTInetHostToString(ipAddr, ((char *) ipAddrString) + 1);
		ipAddrString[0] = OTStrLength(((char *) ipAddrString) + 1);
		
		// Find the associated dirID, creating the directory
		// if necessary.

		junk = MoreProcGetCurrentProcessFSSpec(&dirSpec);
		assert(junk == noErr);
			
		(void) FSMakeFSSpec(dirSpec.vRefNum, dirSpec.parID, ipAddrString, &dirSpec);
		context->rootVRefNum = dirSpec.vRefNum;
		err = FSpGetCatInfo(&dirSpec, 0, &cpb);
		if (err == noErr && ( (cpb.hFileInfo.ioFlAttrib & (1 << 4)) != 0) ) {
			context->rootDirID = cpb.hFileInfo.ioDirID;
		} else {
			err = FSpDirCreate(&dirSpec, 0, (SInt32 *) &context->rootDirID);
		}
	}
	
	// Start a preemptive thread to run an HTTP server on the IP address.
	
	if (err == noErr) {
		err = MPCreateTask(HTTPServerProc, context, 65536, kInvalidID, NULL, NULL, kNilOptions, &junkServerThread);
		if (err == noErr) {
			context = NULL;				// it's now the task's responsibility
		}
	}
	if (context != NULL) {
		MPFree(context);
	}
	
	return err;
}

/////////////////////////////////////////////////////////////////

static OSStatus RunServersForInterface(InetInterfaceInfo* interfaceInfo, SInt32 interfaceIndex)
	// Run HTTP servers for all of the IP addresses associated
	// with the interface denoted by interfaceInfo and interfaceIndex.
	// This routine first starts a server for the primary address
	// of the interface, then iterates through the secondary addresses on
	// the interface, starting a server thread for each one.
{
	OSStatus err;
	InetHost *secondaryAddressBuffer;
	UInt32   numberOfSecondaryAddresses;
	UInt32   addressIndex;

	secondaryAddressBuffer = NULL;
	
	// First run the server for the interfaces primary address.
	
	err = RunOneServer(interfaceInfo->fAddress);
	
	// Now start a server for each of the interface's secondary
	// addresses.  This stuff can only be done on systems that
	// support IP single link multihoming.

	numberOfSecondaryAddresses = interfaceInfo->fIPSecondaryCount;
	
	if ( err == noErr && gHaveIPSingleLinkMultihoming && numberOfSecondaryAddresses > 0 ) {

		// Allocate a buffer for the secondary address info.
		
		secondaryAddressBuffer = (InetHost *) OTAllocMemInContext( numberOfSecondaryAddresses * sizeof(InetHost) , NULL);
		if (secondaryAddressBuffer == NULL) {
			err = kENOMEMErr;
		}
		
		// Ask OT for the list of secondary addresses on this interface.
		
		if (err == noErr) {
			err = OTInetGetSecondaryAddresses(secondaryAddressBuffer, &numberOfSecondaryAddresses, interfaceIndex);
		}
		
		// Start a server for each secondary address.
		
		addressIndex = 0;
		while (err == noErr && addressIndex < numberOfSecondaryAddresses) {
			err = RunOneServer(secondaryAddressBuffer[addressIndex]);
			if (err == noErr) {
				addressIndex += 1;
			}
		}
	}

	// Clean up.
	
	if (secondaryAddressBuffer != NULL) {
		OTFreeMem(secondaryAddressBuffer);
	}

	return err;
}

/////////////////////////////////////////////////////////////////

static OSStatus RunAllHTTPServers(void)
	// Run HTTP servers for all of the IP addresses on the machine.
	// This routine iterates through the active Internet interfaces, 
	// starting server threads for each active IP address on each
	// interface.
{
	OSStatus err;
	OSStatus junk;
	EndpointRef dummyEP;
	InetInterfaceInfo info;
	SInt32 interfaceIndex;
	Boolean done;
	TEndpointInfo epInfo;
	
	// Force TCP to load by creating a dummy endpoint.  Otherwise,
	// if we're the first TCP application to run, OTInetGetInterfaceInfo
	// will not return any active interfaces )-:
	
	// Note that we do this with OTOpenEndpoint rather than OTMPXOpenEndpoint 
	// because we know we're running at system task time so we might as 
	// well give OT time to dial the modem etc.
	
	dummyEP = OTOpenEndpointInContext(OTCreateConfiguration("tcp"), 0, &epInfo, &err, NULL);
	if (err == noErr) {
	
		// Iterate through the interfaces, starting HTTP servers on each.
		
		done = false;
		interfaceIndex = 0; 
		do {
			done = ( OTInetGetInterfaceInfo(&info, interfaceIndex) != noErr );
			if ( ! done ) {
				err = RunServersForInterface(&info, interfaceIndex);
				interfaceIndex += 1;
			}
		} while (err == noErr && !done);
	}
	
	if (dummyEP != NULL) {
		junk = OTCloseProvider(dummyEP);
		assert(junk == noErr);
	}
	
	return err;
}

/////////////////////////////////////////////////////////////////

#if MORE_DEBUG

	static void DoWriteMPLog(void)
		// This routine writes out the MP log contents to 
		// a file "MPLog.txt" on the desktop.
	{
		OSStatus err;
		FSSpec fss;
		
		err = FindFolder(kOnSystemDisk, kDesktopFolderType, true, &fss.vRefNum, &fss.parID);
		if (err == noErr) {
			(void) FSMakeFSSpec(fss.vRefNum, fss.parID, "\pMPLog.txt", &fss);
			err = MPLogWriteToFile(&fss);
		}

		if (err == noErr) {
			printf("Success!\n");
		} else {
			printf("Failed with error %ld.\n", err);
		}
	}

#endif

static void ServerCommandLoop(void)
	// After initialising the various server modules, the main 
	// routine calls this routine to execute the server's user 
	// interface command loop.  This loop asks the user to 
	// enter a command and then executes that command.
	//
	// This loop is radicially different from the loop used by 
	// the cooperative scheduled predecessor of this sample.  
	// Because we use preemptive threads, we can block inside 
	// the standard C library gets routine.  The preemptive threads 
	// will continue to handle network operations even while we're 
	// blocked.
{
	OSStatus 			err;
	char 				commandStr[256];

	// In the debug version, set the default debug log mask to 
	// log all OTMP API calls.
		
	#if MORE_DEBUG
		MPLogSetMask(1 << kOTMPAPILogID);
	#endif
	
	// The server command loop.  Print the list of commands, get 
	// a command, execute it.
	
	do {
		printf("s) Start HTTP servers\n");
		printf("S) Stop  HTTP servers\n");
		#if MORE_DEBUG
			printf("l) Set MPLog mask\n");
			printf("w) Write MPLog to disk\n");
			printf("t) Print running thread count\n");
		#endif
		printf("q) Quit\n");
		printf("Enter a command:\n");
		fflush(stdout);
		if ( fgets(commandStr, sizeof(commandStr), stdin) == NULL ) {
            commandStr[0] = 0;
        }
		
		switch (commandStr[0]) {
			case 's':
				if (gServerStarted) {
					printf("Server is already running (supposedly).\n");
				} else {
					err = RunAllHTTPServers();
					if (err == noErr) {
						gServerStarted = true;
						printf("Success!\n");
					} else {
						printf("Failed with error %ld\n", err);
					}
				}
				break;
			case 'S':
				if (gServerStarted) {
					TermOTMPSimpleServerHTTP();
					gServerStarted = false;
					
					err = InitOTMPSimpleServerHTTP();
					if (err == noErr) {
						printf("Success!\n");
					} else {
						printf("Failed to reinitialise server (%ld)\n", err);
					}
				} else {
					printf("Server is not running.\n");
				}
				break;
			
			#if MORE_DEBUG
			
				case 'l':
					printf("Enter new log mask (in hex):\n");
					gets(commandStr);
					if (commandStr[0] != 0) {
						UInt32 newMask;
						
						sscanf(commandStr, "%lx", &newMask);
						MPLogSetMask(newMask);
						printf("MPLogSetMask(0x%08lx)\n", newMask);
					}
					break;

				case 'w':
					DoWriteMPLog();
					break;
				
				case 't':
					{
						extern volatile SInt32 gRunningThreads;
						
						printf("gRunningThreads = %ld\n", gRunningThreads);
					}
					break;

			#endif
			
			case 'q':
				gQuitNow = true;
				break;
			default:
				printf("Huh?\n");
				break;
		}
	} while ( ! gQuitNow );
	
}

int main(void)
	// The main line of the application.  This routine performs
	// two functions.  At startup, it initialises the various modules.
	// It then calls ServerCommandLoop to enter the server's user interface.
{
	OSStatus err;
	OTClientContextPtr 	junkContext;
	NumVersionVariant 	otVersion;
	
	printf("OTMPSimpleServerHTTP!\n");				// Use printf because MP is not yet initialised.
	printf("-- World's dumbest HTTP server.\n");
	printf("-- Now using preemptive threads.\n");
	printf("\n");

	err = noErr;
	if ( ! MPLibraryIsLoaded() ) {
		err = -1;
	}

	#if MORE_DEBUG
		if (err == noErr) {
			OSStatus junk;
			
			junk = InitMPLog(65536);
		}
	#endif
	
	if (err == noErr) {
		UInt32 response;
		
		err = Gestalt(gestaltSystemVersion, (SInt32 *) &response);
		if ( (err == noErr) && (response == 0x0910) ) {
			printf("••• This code may be unreliable on Mac OS 9.1.\n");
			printf("••• See the documentation for details.\n");
		}
	}
	
	if (err == noErr) {
		switch ( OTMPGetCanRunStatus() ) {
			case kOTMPCanRun:
				// do nothing
				break;
			case kOTMPSystemTooOld:
				printf("OTMP is not supported on this system\n");
				err = unimpErr;
				break;
			case kOTMPBadCarbon:
				printf("OTMP can not be used when CarbonLib < 1.2.5 is installed.\n");
				err = unimpErr;
				break;
			default:
				assert(false);
				break;
		}
	}

	if (err == noErr) {
		gHaveIPSingleLinkMultihoming = ( Gestalt(gestaltOpenTptVersions, (long *) &otVersion) == noErr
											&& (otVersion.whole >= kOTIPSingleLinkMultihomingVersion ) );
		err = InitOpenTransportMPXInContext(kInitOTForApplicationMask, &junkContext);
		if (err == noErr) {
			err = InitOTMPSimpleServerHTTP();
			if (err == noErr) {
				ServerCommandLoop();
				if (gServerStarted) {
					TermOTMPSimpleServerHTTP();
				}
			}
			CloseOpenTransportMPXInContext(NULL);
		}
	}
	
	if (err == noErr) {
		printf("Success.\n");		// Use printf because we might not have successfully initialised MP.
	} else {
		printf("Failed with error %ld.\n", err);
	}	
	printf("Done.  Press command-Q to Quit.\n");
    
    return 0;
}
