/*
	File:		MoreScheduledExec.c

	Contains:	Module for relaunching an application after a time delay.

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

$Log: MoreScheduledExec.c,v $
Revision 1.6  2002/11/08 23:56:50         
Include our prototype early to flush out any missing dependencies. Convert nil to NULL. Convert MoreAssertQ to assert. Move compile time environment check to header.

Revision 1.5  2001/11/07 15:55:06         
Tidy up headers, add CVS logs, update copyright.


         <4>     21/9/01    Quinn   Changes for CWPro7 Mach-O build.
         <3>     20/9/01    Quinn   Upgrade to CWPro7.
         <2>    22/11/00    Quinn   Update for changes in MoreProcesses.
         <1>      6/3/00    Quinn   First checked in.
*/

/////////////////////////////////////////////////////////////////

// Our Prototypes

#include "MoreScheduledExec.h"

// Standard Mac OS interfaces

#if ! MORE_FRAMEWORK_INCLUDES
	#include <MacTypes.h>
	#include <Timer.h>
	#include <Notification.h>
	#include <Gestalt.h>
	#include <Traps.h>
#endif

// MIB Modules

#include "MoreInterfaceLib.h"
#include "MoreProcesses.h"

/////////////////////////////////////////////////////////////////

// High-Level Overview
// -------------------
// This module contains a bunch of tricky code that is carefully commented.
// However, to understand it you really need a big picture idea of what’s going
// on.  This comment attempts to provide that.
//
// When you first schedule an application to be launched by calling 
// MoreDelayedLaunchApplication, the module creates a structure in the
// system heap.  This contains the global variables required to remember
// which application to launch, how to launch it, what parameters to pass it,
// and how long to delay.  It also contains a small chunk of 68K assembly
// language (called the "68K stub") that acts at the target for the deferred
// execution mechanism.
//
// The deferred execution mechanism itself is, as to be expected, somewhat sneaky.
// To implement the delay we use a Time Manager task.  That fires at interrupt
// time, and you can’t call _Launch at interrupt time, so the Time Manager task’s
// sole function is to install a Notification Manager task.  The Notification Manager
// Task’s response procedure then calls _Launch to run the application.

// The following compile time variable determines whether we build the actual
// main code, or the tiny 68K assembly stub we use to generate the hex for
// the gMSECode global variable.  This is explained further in comments below.

#ifndef MORE_SCHEDULED_EXEC_BUILDING_CODE_RESOURCE
	#define MORE_SCHEDULED_EXEC_BUILDING_CODE_RESOURCE 0
#endif

// The following magic values are used to tag data structures for ease of debugging.

enum {
	kMSEGlobalsMagic = 'MSEG',
	kMSECodeMagic    = 'MSEC'
};

// The MoreScheduleExecState enumeration indicates the state of the global
// variables we get at via Gestalt.  This allows us to stop the client from
// scheduling an application twice (which would be system fatal because we’d
// end up reusing our TMTask and NMRec).

typedef UInt32 MoreScheduleExecState;
enum {
	kMSEFreeState = 0,
	kMSEBusyState
};

// The global variables must be aligned with 68K alignment because they are
// potentially shared by 68K (for example, an 'INIT') and PowerPC (for example,
// an application) code.

#if PRAGMA_STRUCT_ALIGN
	#pragma options align=mac68k
#elif PRAGMA_STRUCT_PACKPUSH
	#pragma pack(push, 2)
#elif PRAGMA_STRUCT_PACK
	#pragma pack(2)
#endif

// The MSEGlobals and MSECodeHeader structures are mutually dependent, so we
// have to forward declare one of them.

typedef struct MSECodeHeader MSECodeHeader, *MSECodeHeaderPtr;

// MSEGlobals is the structure that we find using Gestalt.  It includes
// the bulk of the variables we need to do the job.  It also contains
// a pointer to the 68K stub which contains the rest of the global data.

struct MSEGlobals {
	OSType				magic;			// must be kMSEGlobalsMagic
	OSType				signature;		// signature passed to MoreLaunchApplicationAfterDelay
	MSECodeHeaderPtr	codePtr;		// a pointer to our 68K stub
	TMTask				timerTask;		// Time Manager task that implements the delay
	NMRec				noteTask;		// Notification Manager task that gets us the system task time we need for calling _Launch
	LaunchParamBlockRec	launchPB;		// parameters for calling _Launch
	FSSpec				appToLaunch;	// referenced by the launchPB
};
typedef struct MSEGlobals MSEGlobals, *MSEGlobalsPtr;

// MSECodeHeader mirrors the beginning of the 68K stub.  It allows the C
// code to set important parameters for the 68K stub.  The 68K stub picks
// up these parameters using PC-relative addressing, which means that we
// can have an arbitrary number of 68K stubs installed on the system.
// We put these parameters at the beginning of the MSECodeHeader (rather
// than have the 68K stub extract them from the globals) because it makes
// the 68K stub much simpler.

// IMPORTANT: If you change MSECodeHeader, you must also change the
// the __Startup__ code and the gMSECode global data.

struct MSECodeHeader {
	OSType			magic;				// must be kMSECodeMagic
	MoreScheduleExecState state;		// used to detect (and return an error for) double scheduling
	TMTaskPtr		timerTaskPtr;		// pointer to globals->timerTask
	NMRecPtr		noteTaskPtr;		// pointer to globals->noteTask
	LaunchPBPtr		launchPtr;			// pointer to globals->launchPB
	MSEGlobalsPtr	globals;			// pointer to globals
	UInt16			timerTaskEntry;		// entry point for timer task, globals->timerTask.tmAddr points here
	UInt16			noteTaskEntry;		// entry point for notification task, globals->noteTask.nmResp points here
	UInt16			gestaltEntry;		// entry point for Gestalt function
	// ... after this there is code ...	
};

#if PRAGMA_STRUCT_ALIGN
	#pragma options align=reset
#elif PRAGMA_STRUCT_PACKPUSH
	#pragma pack(pop)
#elif PRAGMA_STRUCT_PACK
	#pragma pack()
#endif

#if MORE_SCHEDULED_EXEC_BUILDING_CODE_RESOURCE

	// If MORE_SCHEDULED_EXEC_BUILDING_CODE_RESOURCE is true, we’re building
	// a code resource that generates the code for the 68K stub.  Rather than
	// force clients to include that code resource in their binaries, we have
	// taken a copy of the code resource and included it as data (gMSECode)
	// in the actual C code.  But I wanted to build both C and assembly code
	// from the same source file (helps with keeping things in sync) so I use
	// a compile time variable to say which we’re building.

	// IMPORTANT: If you change the __Startup__ code, you must also
	// sync the MSECodeHeader structure and the gMSECode global data.
	
	asm extern void __Startup__(void);
	
	asm extern void __Startup__(void)
	{
	
	// If you change the following lines, you must update both gMSECode
	// and the MSECodeHeader data structure.
	
					dc.l		kMSECodeMagic		// MSECodeHeader
	state:			dc.l		0					// MSECodeHeader
	timerTaskPtr:	dc.l		0					// MSECodeHeader
	noteTaskPtr:	dc.l		0					// MSECodeHeader
	launchPtr:		dc.l		0					// MSECodeHeader
	globals:		dc.l		0					// MSECodeHeader
					bra.s		TimerTaskEntry		// MSECodeHeader
					bra.s		NoteTaskEntry		// MSECodeHeader
					bra.s		GestaltEntry		// MSECodeHeader

	// From here, if you change the code you only have to update the
	// gMSECode variable.

	TimerTaskEntry:
					move.l		noteTaskPtr,a0		// get NMRec
					_NMInstall						// install it
					rts								// return

	NoteTaskEntry:
					move.w		#_OSDispatch,d0		// determine whether _OSDispatch is implemente
					_GetToolBoxTrapAddress
					move.l		a0,a1
					move.w		#_Unimplemented,d0
					_GetToolBoxTrapAddress
					cmp.l		a0,a1
					beq.s		@Retry				// it isn’t, so retry the launch in a minute
	@TryLaunch:		move.l		launchPtr,a0		// get LaunchParamBlockRec
					_Launch							// launch it
					cmp.w		#memFullErr,d0		// if any error except out of memory,
					bne.s		@NoRetry			// bail
	@Retry:			move.l		#(60*1000),d0		// set timer 1 minute in the future
					move.l		timerTaskPtr,a0		// and schedule it
					_PrimeTime						// 
					bra.s		@NoteTaskLeave		// then leave
	@NoRetry:		move.l		timerTaskPtr,a0		// we failed totally or succeeded totally
					_RmvTime						// remove the timer task
					lea			state,a0			// clear our state
					clr.l		(a0)				// clr => #kMSEFreeState
	@NoteTaskLeave:	move.l		noteTaskPtr,a0		// remove the Notification Manager task
					_NMRemove						// 
					move.l		(sp)+,a0			// pop return address
					addq.l		#4,sp				// junk parameter
					jmp			(a0)				// return

	GestaltEntry:
					move.l		(sp)+,a0			// pop return address
					move.l		(sp)+,a1			// pop response address
					addq.l		#4,sp				// junk selector
					move.l		globals,(a1)		// set response
					clr.w		(sp)				// set result to noErr
					jmp			(a0)				// return
	}

#else

	// This variable holds the machine code for assembly given above.  Every time
	// you change the assembly, you need to rebuild this static data.  You can do
	// this by:
	//
	//   1.	switching to the DummyCodeResource target in the 
	//		TestMoreScheduledExec.mcp project
	//   2.	building the code resource
	//   3.	opening the code resource with ResEdit (in hex view)
	//	 4.	copying out the hex for the resource
	//	 5. pasting it in here, replacing the existing data (you should 
	//      get one long line of hex)
	//	 6. running the following CodeWarrior regexp search and replace:
	//      search for:   “([0-9a-f][0-9a-f][0-9a-f][0-9a-f])”
	//      replace with: “0x\1, ”
	//   7. adjusting the size of the array to match the number of words i the code
	//
	// IMPORTANT:
	// Well, that's the theory!  However, CodeWarrior Pro 7 and above no longer 
	// support building 68K code resources.  So if you want to rebuild the assembly 
	// language you have to create your own dummy code resource project and 
	// build it with an older CodeWarrior (or alternative tools).
	
	static const UInt16 gMSECode[61] = { 
		0x4D53, 0x4543, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 
		0x0000, 0x0000, 0x0000, 0x0000, 0x6004, 0x600A, 0x604C, 0x207A, 
		0xFFEC, 0xA05E, 0x4E75, 0x303C, 0xA88F, 0xA746, 0x2248, 0x303C, 
		0xA89F, 0xA746, 0xB3C8, 0x670C, 0x207A, 0xFFD6, 0xA9F2, 0xB07C, 
		0xFF94, 0x660E, 0x203C, 0x0000, 0xEA60, 0x207A, 0xFFBC, 0xA05A, 
		0x600C, 0x207A, 0xFFB4, 0xA059, 0x41FA, 0xFFAA, 0x4290, 0x207A, 
		0xFFAC, 0xA05F, 0x205F, 0x588F, 0x4ED0, 0x205F, 0x225F, 0x588F, 
		0x22BA, 0xFFA2, 0x4257, 0x4ED0, 0x4E75
	};

	static OSStatus CreateAndRegisterGlobals(OSType signature, MSEGlobalsPtr *globalsPtr)
		// This routine creates the globals variables for this module
		// in the system heap, and registers a Gestalt selector so that we
		// can find them later.
	{
		OSStatus err;
		MSEGlobalsPtr globals;
		
		assert(globalsPtr != NULL);

		// Allocate space for both the globals and our 68K stub.
		// Both of these have to go in the system heap so that they
		// persist beyond the lifetime of the current application/extension
		// context.  Also note that, because the TMTask and NMRec are
		// embedded in the MSEGlobals and hence in the system heap, 
		// they won’t be cleaned up by the Process Manager when the current
		// application quits.
				
		globals = (MSEGlobalsPtr) NewPtrSysClear(sizeof(MSEGlobals));
		err = MemError();
		if (err == noErr) {
			globals->codePtr = (MSECodeHeaderPtr) NewPtrSys(sizeof(gMSECode));
			err = MemError();
		}
		if (err == noErr) {

			// Use BlockMove instead of BlockMoveData because we want to flush 
			// the 68K code cache for this data.
			
			BlockMove(gMSECode, globals->codePtr, sizeof(gMSECode));
			assert(globals->codePtr->magic == kMSECodeMagic);
			
			// Initialise the various parameters in the 68K stub.
			
			globals->codePtr->state        = kMSEFreeState;
			globals->codePtr->timerTaskPtr = &globals->timerTask;
			globals->codePtr->noteTaskPtr  = &globals->noteTask;
			globals->codePtr->launchPtr    = &globals->launchPB;
			globals->codePtr->globals      = globals;
			
			// Initialise the global variables.  Note that we can set up
			// the UPPs in the globals without creating routine descriptors
			// because they always point to 68K code and a pointer to 68K
			// code is a valid UPP.
			
			globals->magic     = kMSEGlobalsMagic;
			globals->signature = signature;
			globals->timerTask.tmAddr = (TimerUPP) &globals->codePtr->timerTaskEntry;
			globals->noteTask.nmResp  = (NMUPP)    &globals->codePtr->noteTaskEntry;
			globals->noteTask.qType   = nmType;

			globals->launchPB.launchBlockID       = extendedBlock;
			globals->launchPB.launchEPBLength     = extendedBlockLen;
			globals->launchPB.launchAppSpec       = &globals->appToLaunch;
			globals->launchPB.launchAppParameters = NULL;
			
			// Register our global variables with Gestalt.  Note that we
			// don’t use Gestalt Value because a) it requires System 7.5 and
			// above and I didn’t want to take that dependencies, and b)
			// using Gestalt is easy anyway because we already have a 68K
			// code stub in the system heap, all we have to do is extend it
			// with another entry point.
			
			err = NewGestalt(signature, (SelectorFunctionUPP) &globals->codePtr->gestaltEntry);
		}
		
		// Clean up.
		
		if (err != noErr) {
			if (globals != NULL) {
				if (globals->codePtr != NULL) {
					DisposePtr( (Ptr) globals->codePtr );
					assert(MemError() == noErr);
				}
				DisposePtr( (Ptr) globals);
				assert(MemError() == noErr);
				globals = NULL;
			}
		}
		*globalsPtr = globals;
		
		return err;
	}

	static OSStatus FindOrCreateGlobals(OSType signature, MSEGlobalsPtr *globalsPtr)
		// This routine attempts to find the global variables using
		// Gestalt, creating them if they haven’t already been.
	{
		OSStatus err;
		
		assert(globalsPtr != NULL);
		
		*globalsPtr = NULL;
		err = Gestalt(signature, (SInt32 *) globalsPtr);
		if (err == gestaltUndefSelectorErr) {
			err = CreateAndRegisterGlobals(signature, globalsPtr);
		} else {
			assert((*globalsPtr)->magic == kMSEGlobalsMagic);
		}
		return err;
	}
	
	extern pascal OSStatus MoreDelayedLaunchApplication(
										OSType signature,
										const FSSpec *appToLaunch,		// may be NULL
										UInt32 appLaunchFlags,
										const AEDesc *appParams,		// may be NULL
										Duration launchDelay)
		// See comments in interface part.
	{
		OSStatus err;
		MSEGlobals *globals;
		ProcessSerialNumber appPSN;

		// Get/create our globals.  We error if the globals are already in use.
				
		err = FindOrCreateGlobals(signature, &globals);
		if ( (err == noErr) && (globals->codePtr->state != kMSEFreeState) ) {
			err = paramErr;
		}
		
		// Start putting parameters to this routine into the various parameters
		// stored in our globals.  Note that we use the philosophy of having
		// the C code do all the complex setup, leaving the assembly language code
		// in the 68K stub to just run the state machine.  This makes the assembly
		// language code easier to write and maintain, which is a good thing.
		
		if (err == noErr) {
			if (appToLaunch != NULL) {
				globals->appToLaunch = *appToLaunch;
			} else {
				err = MoreProcFindProcessByCreator(signature, &appPSN);
				if (err == noErr) {
					err = MoreProcGetProcessAppFile(&appPSN, &globals->appToLaunch);
				}
			}
		}
		if (err == noErr) {
			globals->launchPB.launchControlFlags = launchContinue | launchNoFileFlags | appLaunchFlags;
			if (globals->launchPB.launchAppParameters != NULL) {
				DisposePtr( (Ptr) globals->launchPB.launchAppParameters );
				assert(MemError() == noErr);
				globals->launchPB.launchAppParameters = NULL;
			}

			if (appParams != NULL) {
				globals->launchPB.launchAppParameters = (AppParametersPtr) NewPtrSys(GetHandleSize(appParams->dataHandle));
				err = MemError();
				if (err == noErr) {
					BlockMoveData( *(appParams->dataHandle), globals->launchPB.launchAppParameters, GetHandleSize(appParams->dataHandle) );
				}
			}			
		}
		
		// The parameters for the assembly language stub are all set up, so
		// let’s schedule the timer task.  When it fires, it’ll kick off the
		// Notification Manager task, and when that fires it’ll call _Launch to
		// kick off the application.
		
		if (err == noErr) {
			assert(globals->timerTask.tmAddr != NULL);

			err = MoreInstallTimeTask( (QElemPtr) &globals->timerTask);
			if (err == noErr) {
				globals->codePtr->state = kMSEBusyState;
				err = MorePrimeTimeTask( (QElemPtr) &globals->timerTask, launchDelay);
				if (err != noErr) {
					(void) MoreRemoveTimeTask( (QElemPtr) &globals->timerTask );
					globals->codePtr->state = kMSEFreeState;
				}
			}
		}
		
		return err;
	}

	extern pascal OSStatus MoreCancelDelayedLaunchApplication(OSType signature)
		// See comments in interface part.
	{
		OSStatus err;
		MSEGlobalsPtr globals;
		
		err = FindOrCreateGlobals(signature, &globals);
		if (err == noErr && globals->codePtr->state == kMSEBusyState) {
		
			// It’s important to remove the timer timer before cancelling
			// the Notification Manager task, otherwise the timer could fire
			// between the operations.
			
			(void) MoreRemoveTimeTask( (QElemPtr) &globals->timerTask );
			(void) NMRemove( &globals->noteTask );
		}
		
		return unimpErr;
	}

#endif
