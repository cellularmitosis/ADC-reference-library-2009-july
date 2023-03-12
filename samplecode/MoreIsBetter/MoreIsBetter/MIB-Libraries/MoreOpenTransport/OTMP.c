/*
	File:		OTMP.c

	Contains:	MP task friendly OT library.

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

$Log: OTMP.c,v $
Revision 1.13  2002/11/08 23:43:01         
Include our prototype early to flush out any missing dependencies. Convert nil to NULL. Convert MoreAssertQ to assert.

Revision 1.12  2001/11/07 15:56:51         
Tidy up headers, add CVS logs, update copyright.


        <11>     21/9/01    Quinn   Changes for CWPro7 Mach-O build.
        <10>      9/7/01    Quinn   Fixed a nasty problem with the synchronisation model exposed by
                                    a developer's testing. Also added more debugging stuff (using
                                    TaskLevel) and made this file build with C++ turned on.
         <9>      5/7/01    Quinn   By popular demand, OTMP now inits MoreBlueActions for you.
         <8>     15/2/01    Quinn   Minor tweaks to get it building under Project Builder.
         <7>      8/2/01    Quinn   [2619462] The fix for the MP bug is in CarbonLib 1.2.5.
         <6>      8/2/01    Quinn   Fix bug in SndUDataNotifier which would prevent OTMPSndUData
                                    from recovering from flow control. Also added preliminary
                                    implementations of OTMPPutMessage and OTMPGetMessage.
         <5>     5/12/00    Quinn   Looks like the bug fix won't make it into CarbonLib 1.2.
         <4>    22/11/00    Quinn   CWPro6 is more picky about pointer type compatibility. Add some
                                    casts.
         <3>    10/11/00    Quinn   Added OTMPGetCanRunStatus.
         <2>     9/11/00    Quinn   Changed OTMPOpenEndpointQInContext to open the endpoint
                                    synchronously at system task time. This works around a nasty
                                    race condition, and also eliminates the possibility of quitting
                                    with outstanding async opens.
         <1>     7/11/00    Quinn   First checked in.
*/

/////////////////////////////////////////////////////////////////

// Our Prototypes

#include "OTMP.h"

// Mac OS Interfaces

#if ! MORE_FRAMEWORK_INCLUDES
	#include <Multiprocessing.h>
	#include <Events.h>
	#include <OpenTransportProtocol.h>
	#include <Gestalt.h>
	#include <Devices.h>
	#include <DriverServices.h>
	#include <Debugging.h>
	#include <LowMem.h>
#endif

// MIB Interfaces

#include "MoreMPLog.h"

#include "MoreBlueActions.h"

/////////////////////////////////////////////////////////////////
#pragma mark ***** Can Run *****

enum {
	kMacOSXSystemVersion = 0x01000,
	kCarbonMPFixVersion  = 0x00125		// Version of CarbonLib that fixes Radar ID 2563553.
};

static UInt32 gSystemVersion = 0;

extern pascal OTMPCanRunStatus OTMPGetCanRunStatus(void)
	// See comment in header file.
	// Should modify this routine to setup and use gSystemVersion 
	// but I'm too lazy right now.
{
	OTMPCanRunStatus 	result;
	UInt32				response;
	
	// Assume the best.
	
	result = kOTMPCanRun;
	
	// We only need to do testing on traditional Mac OS.
	// OTMPX will work on all Mac OS X versions.
	
	if (Gestalt(gestaltSystemVersion, (SInt32 *) &response) == noErr) {
		if (response < kMacOSXSystemVersion) {
			// Test that the File Manager is callable from MP tasks.
			// There is no Gestalt bit defined to test for DTInstall, but 
			// this should be close enough.
			
			if ( Gestalt(gestaltMPCallableAPIsAttr, (SInt32 *) &response) == noErr ) {
				if ( (response & (1 << gestaltMPFileManager)) == 0 ) {
					result = kOTMPSystemTooOld;
				}
			} else {
				result = kOTMPSystemTooOld;
			}

			// Now look at the Carbon version, checking for a version of 
			// 1.2.5 or greater.  We do this regardless of whether we're actually 
			// compiled for Carbon or not.  CarbonLibs prior to 1.2.5 contain a 
			// bug [2563553] that makes it unsafe to run OTMP code (even code 
			// compiled against InterfaceLib) on systems on which they are installed.
			
			if (result == kOTMPCanRun) {
				switch ( Gestalt(gestaltCarbonVersion, (SInt32 *) &response) ) {
					case noErr:
						if (response < kCarbonMPFixVersion) {
							result = kOTMPBadCarbon;
						}
						break;
					case gestaltUndefSelectorErr:
						// If we're running under Carbon and we have no gestaltCarbonVersion, 
						// this means we're running on CarbonLib 1.0, which is bad.
						// Unfortunately I can't detect CarbonLib 1.0 when running under 
						// InterfaceLib.
						#if TARGET_API_MAC_CARBON
							result = kOTMPBadCarbon;
						#endif
						break;
					default:
						result = kOTMPSystemTooOld;
						break;
				}
			}
		}
	} else {
		result = kOTMPSystemTooOld;
	}
	
	return result;
}

/////////////////////////////////////////////////////////////////
#pragma mark ***** Utilities *****

static OSStatus MPAllocateQ(ByteCount numBytes, void **result)
	// I got sick of dealing with the wacky parameters to 
	// MPAllocatedAligned, so I wrote a nice wrapper.
{
	OSStatus err;
	
	*result = MPAllocateAligned(numBytes, kMPAllocateDefaultAligned, kMPAllocateClearMask);
	if (*result == NULL) {
		err = memFullErr;
	} else {
		err = noErr;
	}
	return err;
}

// And now we come to the flaw in the Carbon single binary story (-:
//
// Oh I wish OTCanLoadLibraries was part of Carbon.  I need this 
// routine for OTMP on Mac OS 9 and, because it's not part of Carbon, 
// I have to jump through hoops to get to it.

static Boolean DummyOTCanLoadLibraries()
	// This should never be called because Mach-O only runs 
	// on Mac OS X and on Mac OS X you should be using OTMPX 
	// routines, which call the OT compatibility directly.
{
	assert(false);
	return true;
}

typedef Boolean (*OTCanLoadLibrariesProc)(void);		// Note that this is a "C" function, not a "pascal" function!

static OTCanLoadLibrariesProc gOTCanLoadLibrariesProc = &DummyOTCanLoadLibraries;

static void InitOTCanLoadLibrariesProc(void)
{
	#if TARGET_API_MAC_CARBON
		#if TARGET_RT_MAC_CFM
			{
				CFragConnectionID 	connID;
				Ptr					junkMain;
				Str255				junkMsg;
				Ptr					symAddr;
				CFragSymbolClass  	junkSymClass;
				
				if ( GetSharedLibrary("\pOTUtilityLib", kPowerPCCFragArch, kFindCFrag, &connID, &junkMain, junkMsg) == noErr
						&& FindSymbol(connID, "\pOTCanLoadLibraries", &symAddr, &junkSymClass) == noErr) {
					gOTCanLoadLibrariesProc = (OTCanLoadLibrariesProc) symAddr;
				}
			}
		#endif
	#else
		gOTCanLoadLibrariesProc = OTCanLoadLibraries;
	#endif
}

/////////////////////////////////////////////////////////////////
#pragma mark ***** Task Data *****

// OTMP uses per-task data to track critical data structures on a 
// task by task basis.  Specifically, OTMP allocates an MP event 
// group for each task.  The task blocks on this event group while 
// waiting for Blue to complete asynchronous operations.
//
// This event group is stored in a structure that we allocate when 
// the task is prepared (OTMPPrepareThisTask) and disposed when the 
// task is unprepared (OTMPUnprepareThisTask).  The structure contains 
// just the event group and a magic cookie.  I could have avoided 
// using a structure but I decided to go with a structure so that I 
// could extend it if necessary.  Also, the magic number is useful 
// while debugging.
//
// We need to store an event group per task rather than an event group 
// per endpoint because it's possible for more than one task to be 
// blocked on an endpoint.  For example, imagine you have two endpoints 
// and you want to pipe data between.  To do this you might use two tasks,
// one to read from A and write to B and one to do the reverse.  Let's say 
// the first task unblocks with new data and goes to write that data to B,
// but B is send-side flow controlled.  Now B has two tasks block on it, 
// one in receive and one in send.  An event group per endpoint is obviously 
// not going to work here.  However, an event group per task will work 
// because any given task can only be blocked on one synchronous OT call 
// at a time.
// 
// Note that you don't need to (and can't) prepare a non-MP task for 
// calling OTMP.  The reason is that the wait record structure (see 
// below) doesn't need an event group in order to block a non-MP task, 
// so you don't need per-task data to store that event group.  This is 
// good because the Blue task can't have per-task data and, even if it did, 
// it would shared by all traditional Mac OS processes and all the 
// Thread Manager threads within those processes.

enum {
	kOTMPTaskDataMagic = 'TskD'
};

struct OTMPTaskData {
	OSType 		magic;				// must be kOTMPTaskDataMagic
	MPEventID 	waitEvents;			// we block on this while waiting for Blue to complete async operations
};
typedef struct OTMPTaskData OTMPTaskData;
typedef OTMPTaskData * OTMPTaskDataPtr;

// When we initialise the library we allocate a task storage 
// index to access the per-task data for each thread.

static TaskStorageIndex gOTMPTaskStorageIndex = 0;		// 0 implies unitialised

static void DisposeOTMPTaskData(OTMPTaskDataPtr taskData)
	// Dispose of the per-task data for the current MP task.
	// A trivial routine, but I wanted to factor it out from the 
	// two call sites.
{
	OSStatus junk;

	assert(taskData != NULL);
	assert(taskData->magic == kOTMPTaskDataMagic);
		
	if (taskData->waitEvents != NULL) {
		junk = MPDeleteEvent(taskData->waitEvents);
		assert(junk == noErr);
	}
	MPFree(taskData);
}

#if MORE_DEBUG

	// In the debug version we keep track of the number of prepared tasks 
	// so that we can detect if you quit without unpreparing one.
	
	static SInt32 gOTMPPreparedTaskCount = 0;
	
#endif

static OSStatus GetOTMPTaskData(OTMPTaskDataPtr *taskDataPtr)
	// Gets the per-task data for the current task.  Without the 
	// debugging stuff this routine is trivial and it probably 
	// should be inlined into each of the call sites.  However, I'm 
	// not going to do that until I prove that the lack of inlining 
	// actually results in performance problems.
{
	OSStatus err;
	
	assert(taskDataPtr != NULL);

	assert(gOTMPTaskStorageIndex != 0);		// If this fires, it's probably because you forget to call InitOpenTransportMPInContext.
	
	// Attempt to find the task's per-task OTMP storage.
	
	*taskDataPtr = (OTMPTaskDataPtr) MPGetTaskStorageValue(gOTMPTaskStorageIndex);
	if (*taskDataPtr == NULL) {
		err = paramErr;
	} else {
		// We found it.  Check we have good data and then we're done.
		assert((*taskDataPtr)->magic == kOTMPTaskDataMagic);
		assert((*taskDataPtr)->waitEvents != kInvalidID);
		err = noErr;
	}
	
	assert( (err == noErr) == (*taskDataPtr != NULL) );
	
	return err;
}

extern pascal OSStatus OTMPPrepareThisTask(void)
	// See comment in header file.
	//
	// Note that I originally had preparation happen automatically 
	// the first time you called OTMP with a given task.  I no 
	// longer do this because it leads to bugs where you forget 
	// to call OTMPUnprepareThisTask.  Forcing you to explicitly call 
	// this routine reminds you that you need to explicitly call 
	// OTMPUnprepareThisTask.
{
	OSStatus err;
	OTMPTaskDataPtr taskData;

	assert(gOTMPTaskStorageIndex != 0);		// If this fires, it's probably because you forget to call InitOpenTransportMPInContext.
	assert(MPTaskIsPreemptive(kInvalidID));	// Not allowed to prepare blue task.
	assert(MPGetTaskStorageValue(gOTMPTaskStorageIndex) == NULL);	// If this fires, it probably means that you're preparing a task twice.

	// Allocate the per-task data and its associated event group.
		
	err = MPAllocateQ(sizeof(*taskData), (void **) &taskData);
	if (err == noErr) {
		taskData->magic = kOTMPTaskDataMagic;
		err = MPCreateEvent(&taskData->waitEvents);
	}

	// Now record the data in the task.  It's vital that this is the
	// last error we can generate in this branch of the code.  After
	// this point the data has either been recorded in the task (noErr)
	// or not (err).  If not, we dispose of the taskData before returning.

	if (err == noErr) {
		err = MPSetTaskStorageValue(gOTMPTaskStorageIndex, (UInt32) taskData);
	}
	if (err == noErr) {
		#if MORE_DEBUG
			(void) OTAtomicAdd32(1, &gOTMPPreparedTaskCount);
		#endif
	}

	// Clean up after error creating task data.
	
	if (err != noErr && taskData != NULL) {
		DisposeOTMPTaskData(taskData);
	}
	
	return err;
}

extern pascal void OTMPUnprepareThisTask(void)
	// See comment in header file.
{
	OTMPTaskDataPtr taskData;
	
	assert(gOTMPTaskStorageIndex != 0);
	assert(MPTaskIsPreemptive(kInvalidID));	// Not allowed to unprepare blue task.
	
	if (gOTMPTaskStorageIndex != 0) {
		taskData = (OTMPTaskDataPtr) MPGetTaskStorageValue(gOTMPTaskStorageIndex);
		// Calling this routine with an unprepared task is documented as being legal. 
		// That way clients don't have to keep around a record of whether they've 
		// prepared this task or not.
		//
		// assert(taskData != NULL);
		if (taskData != NULL) {
			OSStatus err;
			
			err = MPSetTaskStorageValue(gOTMPTaskStorageIndex, (UInt32) NULL);
			assert(err == noErr);		// If this fails, we leak memory and an event ID.
			if (err == noErr) {
				DisposeOTMPTaskData(taskData);
				#if MORE_DEBUG
					(void) OTAtomicAdd32(-1, &gOTMPPreparedTaskCount);
					assert(gOTMPPreparedTaskCount >= 0);
				#endif
			}
		}
	}
}

/////////////////////////////////////////////////////////////////
#pragma mark ***** Wait Records *****

// The wait record is the key Blue-to-MP communications mechanism. 
// Every operations allocates a wait record on its stack.  It then 
// kicks off some async Blue operation and blocks on the wait record. 
// When the operation is complete Blue unblocks the task waiting on 
// the wait record.
//
// Each wait record embeds a blue action.  This makes sense, because 
// in order for your MP task to be waiting for an event it must have 
// kicked off the process that will generate the event.
//
// There are a number of interesting details here.  One relates to 
// unblocking.  MP tasks always block on an event group, which we 
// get from the per-task data.  See the "Task Data" comments (above) 
// for more details on how this is allocated.  However, non-MP tasks 
// (the main thread and other Thread Manager threads) don't need an 
// event group because they never block.  Instead, they just spin 
// waiting for the waitComplete Boolean to become true.  And while 
// they spin they call the yielder routine that the client installed.
//
// Note that we can't use the standard Mac OS trick of setting the 
// waitResult field to ioInProgress (1) and having the sync wait 
// spin waiting for it to go non-positive.  It's possible for some 
// OT routines, such as OTIoctl, to return positive result.
//
// Another interesting point relates to the waitingFor field.  This 
// field contains an OTEventCode that is the event that the blocked 
// task is waiting for.  For example, if you're blocked inside OTMPRcv, 
// you're waiting for the T_DATA event.  However, this event code is 
// matched 'fuzzily'.  For example, if you're waiting for a T_DATA 
// event and a T_DISCONNECT event comes in, we unblock you (because, 
// hey, you're not getting any more data).  See WaitRecordSearchProc 
// for details on the fuzzy matching algorithm.
//
// Each wait record has a callback, noteProc, that is called when 
// the fuzzy matching algorithm detects that an event should unblock 
// the waiting task. This callback is called at deferred task time from 
// within the context of the OT notifier (something that is significant 
// for synchronisation purposes, see below). If you don't supply a callback, 
// the default action is to simply unblock the waiting task by calling 
// CompleteWaitRecord with the result parameter set to the result 
// parameter of the notifier.

enum {
	kOTMPWaitRecordMagic = 'Wait'
};

// Flags in the waitEvents event group.  We only use one.

enum {
	kOTMPSyncCallComplete  = 0x0001
};

typedef struct OTMPWaitRecord OTMPWaitRecord;		// forward declare so that BlueNotifierProc can reference it

typedef pascal void (*BlueNotifierProc)(OTMPWaitRecord *waitRec, OTEventCode code, OTResult result, void *cookie);

struct OTMPWaitRecord {
	OTLink 			 	link;					// link to next OTMPWaitRecord on OTMPProvider
	OSType 			 	magic;					// must be kOTMPWaitRecordMagic
	OTEventCode 	 	waitingFor;				// event that unblocks task
	BlueNotifierProc 	noteProc;				// if not NULL, called by BlueNotifier within context of endpoint's notifier
	MPEventID		 	waitEvents;				// event group to signal to unblock task, may be kInvalidID if non-MP task is waiting
	volatile Boolean	waitComplete;			// set to true when the wait is done, polled by non-MP task sync wait
	OTResult			waitResult;				// result of operation, valid once waitComplete is set
	MoreBlueAction		blue;					// Blue action used to kick of async operation that we're waiting for
};

static OSStatus InitWaitRecord(OTMPWaitRecord *waitRec, MoreBlueActionProc action, 
							   OTEventCode waitingFor, BlueNotifierProc noteProc)
	// Initialises a wait record.  action is the Blue action that kicks 
	// off the asynchronous operation.  waitingFor is the OT event that 
	// completes the operation.  noteProc is the routine called (at deferred 
	// task time from within the OT notifier) when the event happens.  If NULL, 
	// the default just calls CompleteWaitRecord.
{
	OSStatus 	    err;
	MPEventID		waitEvents;

	assert(waitRec != NULL);
	assert((action != NULL) || (waitingFor == T_PASSCON));
	// action can be NULL in cases where the wait record's blue field is not 
	// actually used.  The only example of this at the moment is the second 
	// wait record used as part of an accept, hence the special case in the 
	// assert above.

	// First establish waitEvents, the event group on which the 
	// task will be waiting.  For non-preemptive task we don't have 
	// an event group (because the task is polling anyway, and creating 
	// a unique per-task event group is difficult because all blue applications
	// (and all threads within those applications) look the same to 
	// MPGetTaskStorageValue), so we just set the field to kInvalidID.
	
	if ( MPTaskIsPreemptive(kInvalidID) ) {
		OTMPTaskDataPtr thisTaskData;
		
		err = GetOTMPTaskData(&thisTaskData);
		if (err == noErr) {
			waitEvents = thisTaskData->waitEvents;
			assert(waitEvents != kInvalidID);
		}
	} else {
		err = noErr;
		waitEvents = kInvalidID;
	}
	
	// Now fill out the rest of the fields of the wait records.
	
	if (err == noErr) {
		waitRec->link.fNext     = NULL;					// only needed for debugging
		waitRec->magic 			= kOTMPWaitRecordMagic;
		waitRec->waitingFor 	= waitingFor;
		waitRec->noteProc   	= noteProc;
		waitRec->waitEvents		= waitEvents;
		waitRec->waitComplete   = false;
		waitRec->waitResult 	= ioInProgress;

		waitRec->blue.magic         = kMoreBlueActionMagic;
		waitRec->blue.link.fNext	= NULL;				// only needed for debugging
		waitRec->blue.action 		= action;
	}
	return err;
}

static OTMPMainThreadYielder gMainThreadYielder = NULL;

extern pascal void InstallOTMPMainThreadYielder(OTMPMainThreadYielder yielder)
	// See comment in header file.
{
	gMainThreadYielder = yielder;
}

// Forward declare gTrueBlueNotifierUPP so that we can use it as the default 
// value of gNotifierToInstall.

static OTNotifyUPP gTrueBlueNotifierUPP;				// -> TrueBlueNotifier

static OTNotifyUPP gNotifierToInstall;

#if MORE_DEBUG

	static Boolean gRunAsSystemTasks = false;

	static OTNotifyUPP gQueuingBlueNotifierUPP;		// -> QueuingBlueNotifier

	extern pascal void RunOTMPAsSystemTasks(Boolean asSystemTasks)
		// See comment in header file.
	{
		// Record that we need to run at system task time.
		
		gRunAsSystemTasks = asSystemTasks;
		
		// Tell MoreBlueActions to run at system task time.
		
		MoreBTRunAsSystemTasks(asSystemTasks);
		
		// Use a queuing notifier rather if system task operations requested.
		
		if (asSystemTasks) {
			gNotifierToInstall = gQueuingBlueNotifierUPP;
		} else {
			gNotifierToInstall = gTrueBlueNotifierUPP;
		}
	}

	// Forward declare RunBlueNotifications so it can be called by 
	// QueueBlueAndWait in the debug build when system task operation 
	// is used.

	static void RunBlueNotifications(void);

#endif

static OSStatus QueueBlueAndWait(OTMPWaitRecord *waitRec, Boolean cancelOK)
	// This routine is responsible for dispatching a Blue action to kick off 
	// an asynchronous event and then waiting on the wait record for the 
	// event to complete.  cancelOK is true if we expect that the operation 
	// might be cancelled.  It doesn't actually affect the behaviour of the 
	// routine, but I thought it might be help catch potential bugs in the 
	// future.
{
	OSStatus err;
	MPEventFlags events;
	#if ! MORE_DEBUG
		#pragma unused(cancelOK)
	#endif

	MPLog1(kOTMPWaitRecordLogID, '>WTE', waitRec);
	
	assert(waitRec->magic == kOTMPWaitRecordMagic);

	// If there are already any events set in our event group, something 
	// has gone horribly wrong.  Use a kDurationImmediate test to check 
	// for this.
	
	assert( (waitRec->waitEvents == kInvalidID) || 
				 (MPWaitForEvent(waitRec->waitEvents, &events, kDurationImmediate) == kMPTimeoutErr)
			   );

	// Kick off the Blue action.
	
	MoreBlueActionInstall(&waitRec->blue);
	
	// If the waiting task is an MP task, just wait for the completion 
	// bit to be set in the event group.  Otherwise, spin wait on the
	// waitComplete field.  While we're spinning, call the client's 
	// yielder callback (if present) and, in the debug version, run any 
	// pending Blue actions and notifications.
	
	if (waitRec->waitEvents != kInvalidID) {
		assert( MPTaskIsPreemptive(kInvalidID) );
		MPLog1(kOTMPWaitRecordLogID, 'bWTE', waitRec);
		err = MPWaitForEvent(waitRec->waitEvents, &events, kDurationForever);
		MPLog1(kOTMPWaitRecordLogID, 'uWTE', waitRec);
	} else {
		assert( ! MPTaskIsPreemptive(kInvalidID) );
		err = noErr;
		while ( (err == noErr) && !waitRec->waitComplete) {
			#if MORE_DEBUG
				if (gRunAsSystemTasks) {
					MoreBTRunNow();
					RunBlueNotifications();
				}
				#if !TARGET_API_MAC_CARBON
					SystemTask();			// Call SystemTask so that MPRemoteCall's have a chance to run.
				#endif
			#endif
			if (gMainThreadYielder != NULL) {
				err = gMainThreadYielder();
			}
		}
	}

	// An error here is potentially very bad.  If the wait fails with an 
	// error, it's possible that our Blue action hasn't run yet, in which case
	// the Blue action list has dangling pointers to the parameter block
	// on the stack.  I'm not sure why the wait would fail (except if the 
	// yielder callback returns an error, which I've warned against in the 
	// comments for that callback) , but I'm interested to see if it even 
	// happens.

	assert(err == noErr);
	assert(waitRec->waitComplete);			// We unblocked but the wait record isn't complete.  Weird!
	assert(waitRec->link.fNext == NULL);

	if (err == noErr) {
		err = waitRec->waitResult;
		if (err == kOTCanceledErr) {
			assert(cancelOK);
		}
	}
	
	// Set the magic field of the wait record to a bad value.  This helps 
	// detect wait records that are accidentally reused.
	
	waitRec->magic = 'bad!';
	
	MPLog2(kOTMPWaitRecordLogID, '<WTE', waitRec, (void *) err);
	return err;
}

static void CompleteWaitRecord(OTMPWaitRecord *waitRec, OTResult result)
	// Completes a wait record, setting the result and unblocking 
	// the task waiting.
{
	OSStatus junk;
	
	assert(waitRec->magic == kOTMPWaitRecordMagic);
	assert(waitRec->link.fNext  == NULL);
	
	MPLog2(kOTMPWaitRecordLogID, '!WTE', waitRec, (void *) result);

	waitRec->waitResult = result;
	waitRec->waitComplete = true;				// unblock non-MP
	if (waitRec->waitEvents != kInvalidID) {	// unblock MP
		junk = MPSetEvent(waitRec->waitEvents, kOTMPSyncCallComplete);
		assert(junk == noErr);
	}
}

/////////////////////////////////////////////////////////////////
#pragma mark ***** Providers *****

// OTMPProvider is a data structure to hold information about 
// a provider, such as the underlying OT provider and the list of 
// synchronous calls waiting on the provider.  We allocate (using 
// the MP allocator) one of these structures each time you create 
// a provider, and dispose it when you close the provider.
//
// There are two key fields in this structure.  The first is otEP, 
// which is a reference to the underlying OT provider that this 
// OTMP provider operates on.
//
// The second key field in the structure is waitRecords, a list 
// of OTMPWaitRecord's that enumerates the tasks that are blocked 
// waiting for events to happen on the provider. For example, if 
// you call OTMPRcv and there's no data on the endpoint, we add 
// your wait record to this list.  When a T_DATA event arrives at 
// the notifier we walk the list of wait records unblocking everyone 
// waiting for a T_DATA event.
//
// This list is an OTList, and is manipulated by OT list management 
// routines that are not reentrant.  We guarantee mutual exclusion in 
// our access to the list by always accessing the list from within 
// a deferred task.  Deferred tasks are always run single threaded.  
// Under most circumstances OT delivers notifications (ie calls your 
// notifier) at deferred task time.  In those cases where it doesn't, 
// we switch to deferred task time before processing the notification.
//
// I also enter the OT notifier when I access this list.  This used 
// to be my primary synchronisation mechanism.  It no longer is because 
// a developer came across a case where it broke down horribly.  So I 
// switched to using deferred tasks as my primary synchronisation 
// mechanism for the wait record list.  However, I continue to enter 
// the notifier to prevent reentrancy on my own thread when I call OT.
//
// This synchronisation model breaks down in a number of interesting 
// cases.  I describe them as they come up later in this source file. 
// It's possible that I will eventually switch to a better 
// synchronisation system, possibly using OTGate, but for the moment 
// this seems to work well enough.
//
// The first field of the OTMPProvider is a magic field that contains 
// the value 'EndP'.  This field should always stay first.  Because of 
// the way OT providers are structured, if you pass an OTMPProvider to 
// an OT routine (for example, passing an OTMPEndpointRef to OTSnd), 
// this field will cause OT to quickly bus error, thereby alerting you 
// to your bug.

enum {
	kOTMPProviderMagic = 'EndP'
};

struct OTMPProvider {
	OSType		magic;				// must be kOTMPProviderMagic
	Boolean		otEPCreated;		// true if otEP is valid
	ProviderRef otEP;				// underlying OT provider
	OTList		waitRecords;		// list of OTMPWaitRecord
	OSStatus	cancelErr;			// normally noErr, set to an error by OTMPCancelSynchronousCalls
};

static Boolean ValidOTMPProviderRef(OTMPProviderRef mpEP, Boolean noEndpointOK)
{
	Boolean result;
	
	result = (mpEP != NULL)
			&& (mpEP->magic == kOTMPProviderMagic)
			&& ( ! mpEP->otEPCreated || ((mpEP->otEP != kOTInvalidProviderRef) || noEndpointOK) );
	if (result) {
		// Should walk waitRecords list but we can't do that in
		// a way that's safe for both MP and non-MP tasks (specifically, code
		// running at deferred task time).  So we leave that unchecked at
		// the moment.
	}
	return result;
}

// For debugging purposes we keep track of two values.  One is the 
// total number of open providers.  If you attempt to quit with 
// providers open, the debug build will warn you in your call to 
// CloseOpenTransportMPInContext.  This is a generally useful 
// debugging tool.

#if MORE_DEBUG
	static SInt32 gOTMPProviderCount = 0;
#endif
	
// Forward declare CloseCore because BlueNotifier needs to use it 
// to handle kOTProviderIsClosed and kOTProviderWillClose messages.

static OSStatus CloseCore(OTMPProviderRef mpEP);

/////////////////////////////////////////////////////////////////
#pragma mark ***** Blue Notifier *****

// BlueNotifier is the OT notifier we use for each of our endpoints. 
// It's job is to look for incoming OT events, match them against 
// wait records pending on the endpoint, and do the appropriate 
// action based on the wait record.  This typically involves 
// unblocking the waiting task.
//
// BlueNotifier uses a fuzzy matching algorithm to match incoming 
// events to wait records. For example, a block OTMPRcv will have 
// a wait record that says that it's waiting for a T_DATA event. 
// If a T_DISCONNECT event comes in, BlueNotifier will unblock 
// the T_DATA wait record because we know there's no more data coming 
// in.  The wait record's notifier is smart enough to look at the 
// unblocking event, see that it's not T_DATA, and unblock the task 
// with an kOTLookErr (see below).

static OTListSearchUPP gWaitRecordSearchProcUPP;	// -> WaitRecordSearchProc

static Boolean WaitRecordSearchProc(const void *ref, OTLink *linkToCheck)
	// This routine is 
{
	Boolean 		 result;
	OTMPWaitRecord * waitRecord;
	
	waitRecord = OTGetLinkObject(linkToCheck, OTMPWaitRecord, link);
	assert(waitRecord->magic == kOTMPWaitRecordMagic);

	result = ( ((OTEventCode) ref) == waitRecord->waitingFor);
	if ( ! result ) {
		switch ( (OTEventCode) ref ) {
			case T_DISCONNECT:
				// Disconnects will cancel blocked Connect's, Snd's, Rcv's, and Listen's.
				result =   (waitRecord->waitingFor == T_CONNECT)		// Connect
						|| (waitRecord->waitingFor == T_GODATA)			// Snd
						|| (waitRecord->waitingFor == T_DATA)			// Rcv
						|| (waitRecord->waitingFor == T_LISTEN);		// Listen
				break;
			case T_ORDREL:
				// Orderly disconnects will cancel blocked Snd's and Rcv's.
				result =   (waitRecord->waitingFor == T_GODATA)			// Snd
						|| (waitRecord->waitingFor == T_DATA);			// Rcv
				break;
			case T_UDERR:
				result =   (waitRecord->waitingFor == T_GODATA)			// SndUData
						|| (waitRecord->waitingFor == T_DATA);			// RcvUData
				break;
			default:
				// do nothing
				break;
		}
	}
	return result;
}

// BlueNotification is the structure used to hold the parameters 
// to the BlueNotifier routine.  There are a variety of reasons 
// why I need to package the notification parameters up into a 
// single structure.  The reasons are explain in the places where 
// BlueNotification is called.

enum {
	kBlueNotificationMagic = 'BlNt'
};

struct BlueNotification {
	OTLink 		link;
	OSType 		magic;					// must be kBlueNotificationMagic
	void *		contextPtr;
	OTEventCode code;
	OTResult 	result;
	void *		cookie;
	Boolean		dtDidRun;				// only used by
};
typedef struct BlueNotification BlueNotification;

static pascal void BlueNotifier(BlueNotification *thisNote)
	// This routine is called by the OT notifier for all OT providers opened 
	// by OTMP.  It is called by OT when events happen on one of 
	// our providers.  When we installed the notifier (done by 
	// OTInstallNotifier after opening the endpoint synchronously) we 
	// provided the OTMPProviderRef as the contextPtr, which OT then 
	// supplies back to use as the thisNote->contextPtr parameter of this 
	// routine.
	//
	// The routine's job is to match the OT event (code) against the 
	// provider's list of wait records.  If it finds a match, it removes 
	// the wait record from the provider and either calls the wait record's 
	// notifier or, if it has no notifier, just completes the wait record, 
	// using this routine's result parameter as the final status.
{
	OSStatus		junk;
	OTMPProviderRef mpEP;
	Boolean			noMatchIsOK;
	Boolean			gotOneMatch;
	OTLink *		thisLink;
	Boolean			forceClose;
	
	assert( ! MPTaskIsPreemptive(kInvalidID) );
	assert( (TaskLevel() & kInDeferredTaskMask) == kInDeferredTaskMask );

	assert(thisNote != NULL);
	assert(thisNote->magic == kBlueNotificationMagic);

	MPLog4(kOTMPNotificationLogID, '>BNt', thisNote->contextPtr, (void *) thisNote->code, (void *) thisNote->result, thisNote->cookie);

	// Convert contextPtr to our OTMPProviderRef.
		
	mpEP = (OTMPProviderRef) thisNote->contextPtr;
	assert(ValidOTMPProviderRef(mpEP, false));

	// Set some flags based on the particular event code. 
	//
	// noMatchIsOK is use to quiet an assert (see later) that's 
	// generated when an event comes in but there's nobody 
	// waiting for it.  In some cases, for example T_BINDCOMPLETE, 
	// this is bad.  In other cases, like T_DATA, this isn't 
	// a problem.
	//
	// forecClose is set if the endpoint is being force closed 
	// by OT.  If so, we run a completely different code path.

	noMatchIsOK = false;
	forceClose  = false;
	switch (thisNote->code) {
		case T_DATA:
		case T_GODATA:
		case T_LISTEN:
		case T_ORDREL:
		case T_DISCONNECT:
			noMatchIsOK = true;
			break;
		case kOTProviderIsClosed:
		case kOTProviderWillClose:
			forceClose = true;
			break;
		default:
			// do nothing
			break;
	}

	if (forceClose) {
		// Guarantee that any future calls fail with an EBADF 
		// and then call the close code to error any blocked requests.
		// Note that we don't enter the notifier because the first 
		// thing that CloseCore does is close the endpoint, which 
		// effectively removes the notifier.  This causes other 
		// concurrency issues, which together require that CloseCore 
		// actually be reentrant.

		mpEP->cancelErr = kOTBadReferenceErr;
		junk = CloseCore(mpEP);
		assert(junk == noErr);
	} else {
		// Search the list of wait records for a matching wait record, 
		// remove that wait record from the list, and then call its 
		// notifier (which typically unblocks the waiting task).
		//
		// We do this repeatedly because the fuzzy matching algorithm 
		// might cause a single event code to hit multiple targets. For 
		// example, a T_DISCONNECT will unblock a OTMPConnect, OTMPListen, 
		// OTMPSnd, and OTMPRcv.
		//
		// We keep track of whether we found any matches and, if noMatchIsOK 
		// is not set, we DebugStr.  This helped find a bunch of bugs while 
		// bringing up the OTMP library.
		//
		// We can walk the list safely at this point because we're at deferred 
		// task time, and the only code that modifies the list is also 
		// running at deferred task time call, thus we're mutually excluded 
		// from them.
		
		gotOneMatch = false;
		do {
			thisLink = OTFindAndRemoveLink(&mpEP->waitRecords, gWaitRecordSearchProcUPP, (void *) thisNote->code);
			if (thisLink == NULL) {
				if ( ! gotOneMatch && ! noMatchIsOK ) {
					#if MORE_DEBUG
						// This DebugStr is more information than anything else.  It is 
						// definitely not a fatal error.
						DebugStr("\pBlueNotifier: failed to find event code in wait record list");
					#endif
				}
			} else {
				OTMPWaitRecord *waitRec;

				gotOneMatch = true;
				
				waitRec = OTGetLinkObject(thisLink, OTMPWaitRecord, link);
				assert(waitRec->magic == kOTMPWaitRecordMagic);
				
				MPLog1(kOTMPWaitRecordLogID, 'WNTE', waitRec);
				
				waitRec->link.fNext = NULL;				// only needed for debugging
				
				// If there is a BlueNotifierProc, call it.  The notifier proc
				// is responsible for unblocking the MP task if necessary.  If
				// there is no BlueNotifierProc then we're a simple operation
				// so just unblock the task in the default manner.
				
				if (waitRec->noteProc != NULL) {
					(waitRec->noteProc)(waitRec, thisNote->code, thisNote->result, thisNote->cookie);
				} else {
					CompleteWaitRecord(waitRec, thisNote->result);
				}
			}
		} while ( thisLink != NULL );
	}

	MPLog(kOTMPNotificationLogID, '<BNt');
}

static DeferredTaskUPP gLocalDeferredTaskUPP = NULL;

static pascal void LocalDeferredTask(long dtParam)
	// When the real notifier (TrueBlueNotifier) discovers that 
	// OT has called it at system task time, it can't call 
	// BlueNotifier directly because of synchronisation concerns
	// (discussed above).  Instead it schedules this routine as a 
	// deferred task, which in turn calls the notifier.  The reason 
	// why this works is discussed in detail in the comments in 
	// TrueBlueNotifier.
{
	BlueNotification *thisNote;
	
	assert( ! MPTaskIsPreemptive(kInvalidID) );
	assert( (TaskLevel() & kInDeferredTaskMask) == kInDeferredTaskMask );

	MPLog1(kOTMPNotificationLogID, '>LDT', (void *) dtParam);
	
	thisNote = (BlueNotification *) dtParam;

	assert(thisNote != NULL);
	assert(thisNote->magic == kBlueNotificationMagic);

	BlueNotifier(thisNote);

	// Tell TrueBlueNotifier that we ran so that it knows it 
	// can safely ignore the fact that it has used a stack-based 
	// DeferredTask structure.
	
	thisNote->dtDidRun = true;

	MPLog(kOTMPNotificationLogID, '<LDT');
}

static pascal void TrueBlueNotifier(void *contextPtr, OTEventCode code, 
									OTResult result, void *cookie)
	// This routine is the actual notifier install for our providers. 
	// It works around a really nasty problem that a developer found 
	// while using this library (thanks Larry!).  Under some circumstances 
	// (typically when there's a client, such as Internet Explorer, that makes 
	// lots of OT calls at system task time) OT can deliver notifications at 
	// system task time rather than deferred task time.  If this happens then 
	// there's a possibility that the notifier will be interrupted 
	// by a BlueAction, which gets us into all sorts of trouble.
	// My fix for this is to force all OT notifications to run at 
	// deferred task time.  This routine implements this fix.  
	// It tests whether we're at system task or deferred task time.
	// If we're at deferred task time it just calls BlueNotifier 
	// directly.  Otherwise it installs a deferred task (LocalDeferredTask) 
	// that actually does the work.  This is a very tricky workaround;
	// you'll have to read the comments below to understand the 
	// true nature of the trickiness.
{
	BlueNotification thisNote;
	
	MPLog4(kOTMPNotificationLogID, '>TBN', contextPtr, (void *) code, (void *) result, cookie);

	// The techniques used in this routine definitely will not work 
	// on Mac OS X.  This isn't a problem because, if you call the MPX
	// routines, they'll automatically use the real Mac OS X OT code, 
	// which will do the right thing.  However, just to be paranoid, 
	// I check that the right thing has happened.
	
	assert( (gSystemVersion != 0) && (gSystemVersion < kMacOSXSystemVersion) );
	
	// In this routine we're using OTCanLoadLibraries to determine whether 
	// the notifier is being called at deferred task time or not.  This 
	// is legal within the limited context of an OT notifier.  [See
	// the discussion in DTS Technote 1104 "Interrupt-Safe Routines" 
	// for an explanation of why OTCanLoadLibraries is safe here but not 
	// necessarily in other places.
	//
	// <http://developer.apple.com/technotes/tn/tn1104.html#DeterminingExecutionLevel>
	//
	// ]  In the debug build we check that OTCanLoadLibraries is behaving 
	// correctly by correlating the results with TaskLevel.
	
	assert( gOTCanLoadLibrariesProc != NULL );
	assert( gOTCanLoadLibrariesProc() == ( (TaskLevel() & kInDeferredTaskMask) != kInDeferredTaskMask ) );
	
	// Fill out all of the parameters to BlueNotification.
	
	thisNote.link.fNext = NULL;
	thisNote.magic 	    = kBlueNotificationMagic;
	thisNote.contextPtr = contextPtr;
	thisNote.code		= code;
	thisNote.result	    = result;
	thisNote.cookie	    = cookie;
	thisNote.dtDidRun   = false;
	
	// If we can load libraries then OT has delivered the notification 
	// at system task time so we have to schedule a deferred task to do 
	// the actual work.  OTOH, if we're being called at deferred task time 
	// (which is the case for most notifications) we can just call 
	// BlueNotifier directly.
	
	if ( gOTCanLoadLibrariesProc() ) {
		DeferredTask localDTask;
		OSStatus 	 junk;
		
		MPLog(kOTMPNotificationLogID, '!TBN');
		
		// Create a deferred task on the stack and install it.  This is 
		// *very* sneaky.  We require BlueNotifier to run at deferred task 
		// time because otherwise we run into wacky bugs where OT calls 
		// our notifier at system task time (thus we're "inside the notifier") 
		// and then our BlueAction deferred task runs, interrupts the system 
		// task time notifier, and asserts trying to call OTEnterNotifier.  
		// We prevent this by always running our notifier at deferred task 
		// time.  If we're not at deferred task time we switch to it 
		// by using DTInstall.
		//
		// There are a bunch of important subtleties here.
		//
		//   1.	We know that we're either at system task time or deferred task 
		//		time because we're inside an OT notifier.  If it was possible 
		//		that we were in some other context (such as a secondary interrupt 
		//		level) this technique wouldn't work.
		//
		// 	 2.	If you call DTInstall at system task time, the Deferred Task 
		//		Manager will immediately call the deferred task.  [On Mac Plus 
		//		era machines this wasn't the case, but we require PowerPC 
		//		which requires SuperMario (or NewWorld) ROMs, which all have 
		//		the ‘new' behaviour.]  It does this by testing the interrupt 
		//		mask and its own internal "in deferred task" state.  Hence the 
		//		requirement that we at either system task or deferred task time.
		//
		//	 3.	We know that the "in deferred task" state can't change between 
		//		our OTCanLoadLibraries test above and our DTInstall call because 
		//		the state can only be changed by an interrupt and interrupts 
		//		(on traditional Mac OS) run to completion, ie they should undo 
		//		any changes they did before we get scheduled again.  This is 
		//		not true on Mac OS X, which is one of the many reasons why 
		//		this code won't run on Mac OS X (even though it compiles for Carbon).
		//
		//	 4.	We use DTInstall to run BlueNotifier because it sets the 
		//		system's "in deferred task" state, which in turn prevents any 
		//		deferred tasks (including our BlueActions) from running.
		//		We could just set the "in deferred task" bit directly 
		//		(it's stored in a relatively well known low memory global)
		//		but DTInstall has the advantage that it will run any 
		//		BlueActions that were queued while BlueNotifier is running.
		
		assert(gLocalDeferredTaskUPP != NULL);	// Check if we forgot to initialise our UPP.
		
		localDTask.qLink      = NULL;
		localDTask.qType      = dtQType;
		localDTask.dtFlags    = 0;
		localDTask.dtAddr     = gLocalDeferredTaskUPP;
		localDTask.dtParam    = (long) &thisNote;
		localDTask.dtReserved = 0;
		junk = DTInstall(&localDTask);
		assert(junk == noErr);
		
		assert(thisNote.dtDidRun);				// Oh boy this bad.  The DT we installed 
													// didn't run immediately, which means 
													// that the deferred task queue still has 
													// a pointer to our localDTask variable. 
													// We attempt to recover from this (with 
													// some evil hackery) but it should never 
													// happen.
		if ( ! thisNote.dtDidRun ) {
			// The DT didn't run.  This is bad.  We attempt to recover 
			// by pulling our task out of the deferred task queue, but 
			// if this ever happens we're basically dead.
			//
			// There is no DTRemove, so we have to pull the element out 
			// of the queue manually.  However, LMGetDTQueue is not part 
			// of Carbon, so if we're building for Carbon we just define 
			// our own.  Remember, this code can't be run on Mac OS X.
			//
			// While this is a hack, it should never get executed.  If 
			// it does the worst it can do is crash the system, which is 
			// what'll happen anyway if we don't yank the queue element.
			
			#if TARGET_API_MAC_CARBON
				#define LMGetDTQueue()  ( (QHdrPtr) 0x0D92)
			#endif
			junk = Dequeue( (QElemPtr) &localDTask, LMGetDTQueue() );
			assert(junk == noErr);
		}
	} else {
		BlueNotifier(&thisNote);
	}

	MPLog(kOTMPNotificationLogID, '<TBN');
}

#if MORE_DEBUG

	// While bringing up the library it was useful to run all events 
	// at system task time.  This part of the code is responsible for 
	// deferring OT notifications, which ususally happen at deferred 
	// task time, until system task time.  The idea is that we install 
	// QueuingBlueNotifier rather than TrueBlueNotifier as each provider's 
	// notifier.  That routine allocates a structure (using the OT 
	// memory allocator -- this is important, because the MP memory 
	// allocator is not interrupt safe), puts all the parameters to the 
	// notifier in that structure, and then puts the structure on 
	// a global list (gQueuedNotifications).  At system task time our 
	// sync wait loop (inside QueueBlueAndWait) periodically calls 
	// RunBlueNotifications, which pulls the notifications off the list 
	// and calls the real notifier (BlueNotifier) for each one.

	// gQueuedNotifications (a list of BlueNotification structures) is 
	// the list of notifications that have been delivered at deferred 
	// time but have been deferred until system task time for debugging reasons.  
	// QueuingBlueNotifier adds structures to this list and RunBlueNotifications 
	// pulls them off.
	
	static OTLIFO gQueuedNotifications = {NULL};		// list of BlueNotification's
	
	static void RunBlueNotifications(void)
		// This routine is called periodically by our sync wait routine 
		// (QueueBlueAndWait).  It pulls all the BlueNotification entries 
		// of gQueuedNotifications and, for each one, calls the real 
		// notifier (BlueNotifier).
	{
		OTLink *noteList;
		Boolean entered;
		
		// Grab the list of queued notifications and iterate over it.
		
		noteList = OTReverseList(OTLIFOStealList(&gQueuedNotifications));
		while ( noteList != NULL ) {
			BlueNotification *thisNote;
			OTMPProviderRef thisMPEP;
			
			thisNote = OTGetLinkObject(noteList, BlueNotification, link);
			assert(thisNote->magic == kBlueNotificationMagic);

			// We need to enter the notifier for the underlying OT 
			// endpoint before we call BlueNotifier.  This is because 
			// BlueNotifier expects to be called by OT as the real 
			// notifier, and hence it expects to be running with 
			// notifier-style mutual exclusion.  In order to enter 
			// the notifier, we need the OT provider.  We get that by 
			// assuming that the notification's contextPtr is an 
			// OTMPProviderRef.
			//
			// Note that we don't always have an underlying provider; 
			// in the case where the provider is being opened
			// (ie in the T_OPENCOMPLETE event) this will be NULL.
			// We have to handle that appropriately.
			//
			// I don't think this last case can happen any more 
			// since we changed to use synchronous opens.  However, 
			// I'm not 100% sure and I can't be bothered testing it 
			// so I'll leave the check in place.
			
			thisMPEP = (OTMPProviderRef) thisNote->contextPtr;
			assert(ValidOTMPProviderRef(thisMPEP, false));
			
			if (thisMPEP->otEP == kOTInvalidProviderRef) {
				entered = false;
			} else {
				entered = OTEnterNotifier(thisMPEP->otEP);
				assert(entered);		// We're doing all operations, including this 
											// one, at system task time.  We should never 
											// have a problem entering the notifier.
													
			}
			
			// Call the real BlueNotifier with the parameters from the 
			// BlueNotification structure.
			
			BlueNotifier(thisNote);
			
			if (entered) {
				OTLeaveNotifier(thisMPEP->otEP);
			}

			// Move along to the next list element and free the current one.
			
			noteList = noteList->fNext;
			
			OTFreeMem(thisNote);
		}
	}

	static pascal void QueuingBlueNotifier(void *contextPtr, OTEventCode code, 
										   OTResult result, void *cookie)
		// This routine is installed as our provider notifier if we're 
		// running everything at system task time (ie we're debugging).
		// It's purpose is to allocate a BlueNotification structure, copy all 
		// the notifier parameters to that structure, and put it on the 
		// list of pending notifications (gQueuedNotifications).  Our 
		// sync wait routine (QueueBlueAndWait) will periodically call 
		// RunBlueNotifications which will pull the notifications off 
		// the list and run them at system task time.
		//
		// If gRunAsSystemTasks is true, this is the only part of this 
		// module that runs at deferred task time and can't be debugged 
		// with a source level debugger.
	{
		BlueNotification *thisNote;
		
		// Allocate memory for the BlueNotification record.  We don't handle
		// failure very well, so if the client pool is full we try the
		// the shared client pool.  If we still can't get memory then
		// we've broken the state machine so we assert.  I would try to
		// come up with a better solution but this is only a debugging
		// mechanism so there's little point.
		//
		// Also note that we pass NULL as the client context for this 
		// allocate.  We're assuming that we have a valid application 
		// OT client context.  This only really makes sense in a 
		// debugging scenario.  Fortunately, that's the only case in 
		// which this code is ever executed.
		
		thisNote = (BlueNotification *) OTAllocMemInContext(sizeof(*thisNote), NULL);
		if (thisNote == NULL) {
			#if !TARGET_API_MAC_CARBON
				thisNote = (BlueNotification *) OTAllocSharedClientMem(sizeof(*thisNote));
			#endif
		}
		assert(thisNote != NULL);

		// Copy the notification parameters into the structure and 
		// put it on the gQueuedNotifications list.
				
		if (thisNote != NULL) {
			thisNote->link.fNext  = NULL;		// only needed for debugging
			thisNote->magic       = kBlueNotificationMagic;
			thisNote->contextPtr  = contextPtr;
			thisNote->code        = code;
			thisNote->result      = result;
			thisNote->cookie      = cookie;
			thisNote->dtDidRun    = false;		// not needed for this code path, but hey

			OTLIFOEnqueue(&gQueuedNotifications, &thisNote->link);
		}
	}

#endif

/////////////////////////////////////////////////////////////////
#pragma mark ***** Standard Actions *****

// Many OTMP routines work in a similar way.

// 1. The API called by clients initialises a wait record then queues 
//    a Blue action to kick off the operation.
//
// 2. If the kick off succeeds, the wait record is completed by a 
//    notification so we have to add the wait record to the provider's 
//    list of wait records.
//
// 3. If the kick off fails, we complete the wait record with an 
//    immediate error.
//
// This process is known (within OTMP) as a standard action.  OTMP 
// explicitly supports standard actions in order to factor out 
// common code from a bunch of OTMP API routines.  The code below 
// is the standard action abstraction.
//
// StdParam is a structure specifically designed to contain all 
// of the information needed by a standard action.  The key fields are:
//
// o stdAction -- The function to call to kick off the operation.
//   This routine is called at deferred task time 'inside' the provider's 
//   notifier.  It returns a Boolean that indicates whether the action's 
//   wait record should be completed or queued.
//   
// o waitRecord -- The calling task's wait record.  Remember that the 
//   wait record contains another callback.  If the wait record is 
//   queued, this callback will be called when the wait record's 
//   event is delivered.
//
// o mpEP -- A reference to the OTMPProvider.  Necessary because the 
//   standard action enters the notifier on the underlying OT provider.

enum {
	kStdParamMagic = 'StdP'
};

typedef struct StdParam StdParam;	// forward declare

typedef pascal Boolean (*OTMPStdAction)(StdParam *stdParam, OSStatus *resultPtr);
	// The standard action kick off callback.  The function result determines 
	// whether the action should complete immediate (true) or be queued (false). 
	// If the action completes immediately then *resultPtr is the wait record 
	// result.
	
struct StdParam {
	OSType				stdMagic;				// must be kStdParamMagic
	OSType				magic;					// magic for use by client
	OTMPStdAction	 	stdAction;				// the standard action routine to call
	OTMPWaitRecord  	waitRecord;				// the calling tasks' wait record
	OTMPProviderRef 	mpEP;					// the endpoint on which the action is performed
};

static pascal void StdAction(MoreBlueAction *thisTask)
	// A Blue action that performs the standard action 
	// operation (see algorithm described above).  Called a deferred 
	// task time by the Blue action mechanism (see MoreBlueActions for 
	// details).
{
	OSStatus 	err;
	StdParam *	stdParam;
	Boolean 	entered;
	Boolean		completeIt;
	
	stdParam = OTGetLinkObject(thisTask, StdParam, waitRecord.blue);
	assert(stdParam->stdMagic == kStdParamMagic);
	
	assert(ValidOTMPProviderRef(stdParam->mpEP, false));
	
	MPLog2(kOTMPStdActionLogID, '>Std', stdParam, (void *) stdParam->magic);
	
	// Enter the notifier for the endpoint so that we're mutually excluded
	// with respect to the notifier (BlueNotifier).  We no longer need to do this 
	// to protect the waitRecords list, which is now protected because all 
	// operations on it run at deferred task time (which is single threaded). 
	// However, we continue to enter the notifier because it prevents OT delivering 
	// notifications to us when we call it.  This notification can confused matters 
	// greatly, as illustrated by the OTMPXAccept code which has to handle them 
	// in one case when they're not deferred, namely T_PASSCON.
	
	// I don't believe the following test is useful any more because 
	// endpoints are now always opened synchronously.  However, I'm 
	// not 100% sure so I left it in.
	
	if ( stdParam->mpEP->otEP == kOTInvalidProviderRef ) {
		// Handle the case for OTMPOpenEndpoint where the EndpointRef
		// has not yet been set up.
		entered = false;
	} else {
		entered = OTEnterNotifier(stdParam->mpEP->otEP);
		if ( ! entered ) {
			// In previous incarnations of this code this assert was fatal. 
			// In current version we no longer exclusively rely on the 
			// notifier for mutual exclusion (because this assert would 
			// fail if OT called our notifier at system task time) so 
			// this assert isn't fatal.  Instead we just log the event.
			// Mutual exclusion of the wait record list is provided by 
			// always accessing it at deferred task time.
			MPLog(kOTMPStdActionLogID, '!Std');
		}
	}

	// Call the operation kick off routine.

	if (stdParam->mpEP->cancelErr == noErr) {
		completeIt = (stdParam->stdAction)(stdParam, &err);
	} else {
		err = stdParam->mpEP->cancelErr;
		completeIt = true;
	}

	MPLog3(kOTMPStdActionLogID, '<Std', stdParam, (void *) (UInt32) completeIt, (void *) err);

	// If we're told that the operation is already complete, immediately 
	// signal the completion of the standard action's embedded wait record.
	// If not, add the wait record to the OTMProvider's list.  The operation 
	// will continue when its event is delivered to the notifier.
	
	if (completeIt) {

		// We're done.  Tell the MP task about it.
		// But first leave the notifier because stdParam is on the MP 
		// task's stack and it may be destroyed as soon as we call 
		// CompleteWaitRecord.
		
		if (entered) {
			OTLeaveNotifier(stdParam->mpEP->otEP);
		}
		CompleteWaitRecord(&stdParam->waitRecord, err);

	} else {

		// We're now expecting our notifier (BlueNotifier) to be called with
		// a stdParam->waitRecord.waitingFor event.  Put the wait record on our 
		// list of wait records so that BlueNotifier knows what to do when it
		// gets that event.  We do this at this point because a failure of
		// the kick off operation (which typically just makes a simple OT call)
		// would mean that BlueNotifier would not get called.
		//
		// Also, make sure that we call OTLeaveNotifier after putting the 
		// wait record on the list so that the non-atomic list operation can't 
		// be interrupted by our notifier code.
		
		assert(stdParam->waitRecord.link.fNext == NULL);
		OTAddFirst(&stdParam->mpEP->waitRecords, &stdParam->waitRecord.link);

		if (entered) {
			OTLeaveNotifier(stdParam->mpEP->otEP);
		}

	}
}

static pascal OSStatus InitStdParam(StdParam *stdParam, OSType magic, 
									OTMPStdAction stdAction, OTMPProviderRef mpEP,
									OTEventCode waitingFor, BlueNotifierProc noteProc)
	// Initialises the standard action structure.  magic is the 
	// magic cookie for the operation.  It's used for debug asserts 
	// and logging only. stdAction is the routine to call to start the 
	// operation.  mpEP is the OTMPProvider on which the on operation is 
	// occuring.  We record this in StdParam so that StdAction can 
	// enter and leave the notifier for you.  waitingFor and noteProc 
	// are used to initialise the action's wait record; they are passed 
	// directly to InitWaitRecord.
{
	OSStatus err;
	
	assert(stdParam != NULL);
	assert(ValidOTMPProviderRef(mpEP, false));

	err = InitWaitRecord(&stdParam->waitRecord, StdAction, waitingFor, noteProc);
	if (err == noErr) {
		stdParam->stdMagic  = kStdParamMagic;
		stdParam->magic     = magic;
		stdParam->stdAction = stdAction;
		stdParam->mpEP      = mpEP;
	}
	return err;
}

/////////////////////////////////////////////////////////////////
#pragma mark ***** Open *****

// OTMP uses a standard pattern to implement most of its operations.
// To learn more about the standard pattern, see the comments above 
// OTMPIoctl.  However, OTMPOpenEndpointQInContext can't use the 
// standard pattern, for a variety of reasons.
//
//    o	An OT client can not quit (ie call CloseOpenTransportInContext 
// 		for its context) with outstanding OTAsyncOpenEndpointInContext
//		requests.  Doing so will crash a Mac OS 9 system.  However, OTMP 
//		has no obvious way to defer the CloseOpenTransportMPInContext call 
//		waiting for async opens to complete.  Instead of trying to 
//		implement that, I decided to switch to using sync opens.
//
//	  o OTAsyncOpenEndpointInContext often requires a deferral until 
//		system task time anyway.  For example, if OT needs to load 
//		a module and dial the modem, it will defer the open to a system 
//		task.  If that's going to happen anyway, we might as well defer 
//		it here instead.
//
//	  o	OT's deferral when opening endpoints asynchronously exposes a 
//		nasty race condition in OTMP.  The following sequence of events 
//		happens.
//
//		 1.	MP task schedules a Blue action to start the open.
//		 2.	MP task blocks on event group.
//		 3.	Blue task runs Blue action at deferred task time.
//		 4.	Blue action calls OTAsyncOpenEndpointInContext.
//		 5.	The open is deferred, so Blue action returns from deferred 
//			task time.
//		 6.	Blue code calls SystemTask, which runs the OT system task 
//			to do the deferred open.  OT does the open and then calls 
//			the notifier.  It does this at system task time, while 
//			holding the notifier lock.
//		 7.	BlueNotifier completes the open operation, which unblocks 
//			the MP task.
//		 8. The MP task issues another operation on that endpoint 
//			(such as on OTMPBind).  This results in another Blue action.
//		 9.	The Blue action interrupts BlueNotifier before it returns.
//			It trips over an assert when it attempts to call OTEnterNotifier
//			to acquire mutual exclusion on the endpoint.
//
//		Nasty!
//
//		Curiously enough, a developer discovered this sort of problem happening 
//		in another context and I had to fix it properly, so this scenario 
//		is no longer relevant.  However, we still still with sync opens 
//		for all the other reasons described above.
//
// Instead OTMPOpenEndpointQInContext uses an MPRemoteCall to execute 
// the OpenRemoteProc at system task time.  This avoids all of the 
// problems with opening endpoints asynchronously.  It does, however, 
// have a negative effect on performance.  However, OT's native endpoint 
// creation code isn't particularly fast, so most high-performance 
// applications have learnt to keep a cache of endpoints.

enum {
	kOTMPOpenMagic = '>Opn'
};

struct OpenParam {
	OSType				magic;				// must be kOTMPOpenMagic
	OTMPEndpointRef 	mpEP;
	const char *    	cfig;
	OTOpenFlags     	oflag;
	TEndpointInfo * 	info;
	OTClientContextPtr	clientContext;
};
typedef struct OpenParam OpenParam;

static void *OpenRemoteProc(void *parameter)
	// OTMPOpenEndpointQInContext calls this routine is called via MPRemoteCall, 
	// which means it always runs at system task time.  It is responsible 
	// for calling OTOpenEndpointInContext to actually open the endpoint.
{
	OSStatus 	err;
	OSStatus 	junk;
	OpenParam *	openParam;

	assert( ! MPTaskIsPreemptive(kInvalidID) );
	assert( TaskLevel() == 0 );

	openParam = (OpenParam *) parameter;
	assert(openParam->magic == kOTMPOpenMagic);
	
	openParam->mpEP->otEP = OTOpenEndpointInContext(OTCreateConfiguration(openParam->cfig), 
								openParam->oflag, openParam->info, 
								&err, openParam->clientContext);
	if (err == noErr) {
		err = OTSetAsynchronous(openParam->mpEP->otEP);
	}
	if (err == noErr) {
		err = OTInstallNotifier(openParam->mpEP->otEP, gNotifierToInstall, openParam->mpEP);
		// This is running at system task time.  We could potentially get into 
		// trouble here if a notification arrives at deferred task time between here 
		// and the end of OTMPOpenEndpointQInContext.  Hasn't happened yet though.
	}
	
	// Clean up.
	
	if ( (err != noErr) && (openParam->mpEP->otEP != kOTInvalidProviderRef) ) {
		junk = OTCloseProvider(openParam->mpEP->otEP);
		assert(junk == noErr);
		openParam->mpEP->otEP = kOTInvalidProviderRef;
	}

	// Post condition.
	
	assert( (err == noErr) == (openParam->mpEP->otEP != kOTInvalidProviderRef) );

	return (void *) err;
}

extern pascal OTMPEndpointRef OTMPOpenEndpointQInContext(
								 const char *           cfig,
                                 OTOpenFlags            oflag,
                                 TEndpointInfo *        info,
                                 OSStatus *             errPtr,
                                 OTClientContextPtr     clientContext)
	// See comment in header file.
	// Note: Don't need to worry about testing cancelErr in this routine 
	// because client can't call OTMPCancelSynchronousCalls until this 
	// routine completes (because they don't have a OTMPEndpointRef yet).
{
	OSStatus 		err;
	OTMPEndpointRef mpEP;
	OpenParam   	openParam;

	assert(cfig != NULL);
	
	MPLog1(kOTMPAPILogID, kOTMPOpenMagic, cfig);

	// Initialise our parameter block, allocate a new OTMPProvider 
	// structure with the MP allocator, fill out the structure, and 
	// then do an MPRemoteCall to open the endpoint.

	err = MPAllocateQ(sizeof(*mpEP), (void **) &mpEP);
	if (err == noErr) {
		mpEP->magic 		= kOTMPProviderMagic;
		mpEP->otEPCreated 	= false;
		mpEP->otEP 			= kOTInvalidProviderRef;
		mpEP->waitRecords.fHead = NULL;
		mpEP->cancelErr		= noErr;

		openParam.magic = kOTMPOpenMagic;
		openParam.mpEP 	= mpEP;
		openParam.cfig 	= cfig;
		openParam.oflag = oflag;
		openParam.info  = info;
		openParam.clientContext = clientContext;

		err = (OSStatus) MPRemoteCall(OpenRemoteProc, &openParam, kMPAnyRemoteContext);
		
		mpEP->otEPCreated = (err == noErr);
	}

	// Clean up after an error.
	
	if (err != noErr && mpEP != NULL) {
	
		// If we successfully created the endpoint, we wouldn't be failing,
		// so just make sure that's true.
		
		assert( ! mpEP->otEPCreated );
		assert( mpEP->otEP == kOTInvalidProviderRef );
		
		mpEP->magic = 'bad!';		// just in case someone accidentally tries to reuse this memory
		
		MPFree(mpEP);
		
		mpEP = NULL;
	}
	
	if (errPtr != NULL) {
		*errPtr = err;
	}
	
	if (err == noErr) {
		#if MORE_DEBUG
			(void) OTAtomicAdd32(1, &gOTMPProviderCount);
		#endif
	}

	// Post condition.
	
	assert((err != noErr) || ValidOTMPProviderRef(mpEP, false));
	
	MPLog2(kOTMPAPILogID, MPLogTagReturn(kOTMPOpenMagic), (void *) err, mpEP);
	
	return mpEP;
}

/////////////////////////////////////////////////////////////////
#pragma mark **** Close *****

enum {
	kOTMPCloseMagic = '>Clo'
};

struct CloseParam {
	OSType			magic;				// must be kOTMPCloseMagic
	OTMPWaitRecord  waitRecord;
	OTMPProviderRef mpEP;
};
typedef struct CloseParam CloseParam;

static OSStatus CloseCore(OTMPProviderRef mpEP)
	// This routine has to be completely reentrant because it's 
	// called by CloseAction (deferred task time) and the 
	// kOTProvideWillClose handling in the BlueNotifier (system 
	// task time).  So we use a bunch of atomic operations.
{
	OSStatus 	 err;
	ProviderRef  otEP;
	OTLink * 	 thisLink;
	OTLink * 	 nextLink;

	// First close the underlying endpoint.
	
	// If a kOTProviderWillClose or kOTProviderIsClosed notification 
	// happens then it's possible for the underlying OT endpoint to be 
	// closed at this point.  If we call OTCloseProvider with a closed 
	// endpoint it will return an error, which we don't want.
	
	err = noErr;
	otEP = mpEP->otEP;
	if ( OTCompareAndSwapPtr((void *) otEP, (void *) kOTInvalidProviderRef, (void **) &mpEP->otEP) ) {
		if (otEP != kOTInvalidProviderRef) {
			err = OTCloseProvider(otEP);
		}
	}
	assert(err == noErr);			// Getting an error from CloseProvider is very strange.

	// Grab the entire list of wait records and then unblock each element.

	do {
		thisLink = mpEP->waitRecords.fHead;
	} while ( ! OTCompareAndSwapPtr(thisLink, NULL, (void **) &mpEP->waitRecords.fHead) );
	
	while ( thisLink != NULL ) {
		OTMPWaitRecord *thisWaitRecord;
		
		// Skip to the next element now because as soon as we call 
		// CompleteWaitRecord the wait record may be gone.
		
		nextLink = thisLink->fNext;
		
		thisWaitRecord = OTGetLinkObject(thisLink, OTMPWaitRecord, link);
		assert(thisWaitRecord->magic == kOTMPWaitRecordMagic);

		CompleteWaitRecord(thisWaitRecord, kOTCanceledErr);
		
		thisLink = nextLink;
	}

	return err;
}

static pascal void CloseAction(MoreBlueAction *thisTask)
	// The action callback for OTMPCloseProvider.
{
	OSStatus 	err;
	CloseParam *closeParam;
	
	assert( ! MPTaskIsPreemptive(kInvalidID) );
	assert( (TaskLevel() & kInDeferredTaskMask) == kInDeferredTaskMask );

	closeParam = OTGetLinkObject(thisTask, CloseParam, waitRecord.blue);
	assert(closeParam->magic == kOTMPCloseMagic);
	
	err = CloseCore(closeParam->mpEP);

	CompleteWaitRecord(&closeParam->waitRecord, err);
}

extern pascal OSStatus OTMPCloseProvider(OTMPProviderRef ref)
	// OTMP API entry point.
{
	OSStatus 	err;
	CloseParam 	closeParam;
	
	assert(ValidOTMPProviderRef(ref, true));
	
	MPLog1(kOTMPAPILogID, kOTMPCloseMagic, ref);
	
	err = InitWaitRecord(&closeParam.waitRecord, CloseAction, 0, NULL);
	if (err == noErr) {
		closeParam.magic    = kOTMPCloseMagic;
		closeParam.mpEP		= ref;
	
		// Deliberately invalidate the endpoint data structure so that
		// any future calls will assert immediately.
		
		ref->magic = 'bad!';

		err = QueueBlueAndWait(&closeParam.waitRecord, false);
		
		MPFree(ref);
		
		#if MORE_DEBUG
			(void) OTAtomicAdd32(-1, &gOTMPProviderCount);
			assert(gOTMPProviderCount >= 0);
		#endif
	}

	MPLog1(kOTMPAPILogID, MPLogTagReturn(kOTMPCloseMagic), (void *) err);
	return err;
}

enum {
	kOTMPCancelMagic = '>Can'
};

struct CancelParam {
	OSType			magic;				// must be kOTMPCancelMagic
	OTMPWaitRecord  waitRecord;
	OTMPProviderRef mpEP;
	OSStatus		errParam;
};
typedef struct CancelParam CancelParam;

static pascal void CancelAction(MoreBlueAction *thisTask)
	// The action callback for OTMPCancelSynchronousCall.
{
	OSStatus 		err;
	CancelParam *	cancelParam;
	Boolean			entered;
	
	assert( ! MPTaskIsPreemptive(kInvalidID) );
	assert( (TaskLevel() & kInDeferredTaskMask) == kInDeferredTaskMask );

	cancelParam = OTGetLinkObject(thisTask, CancelParam, waitRecord.blue);
	assert(cancelParam->magic == kOTMPCancelMagic);

	err = noErr;
	
	// We are running at deferred task time and hence have exclusive access 
	// to the wait record list.  But hey, I enter the notifier anyway, 
	// just to be sure.  This assert might fire (in the same cases where 
	// the assert that was previouly in StdAction used to fire) but it's 
	// probably not a bug because the wait record list is protected by 
	// always being accessed from a deferred task.
	
	entered = OTEnterNotifier(cancelParam->mpEP->otEP);
	assert(entered);

	// Mark the endpoint as cancelled so that any future operations will 
	// fail immediately.
	
	cancelParam->mpEP->cancelErr = cancelParam->errParam;

	// Cancel each blocked synchronous operation that's waiting on this 
	// endpoint.
	
	while ( cancelParam->mpEP->waitRecords.fHead != NULL ) {
		OTMPWaitRecord *thisWaitRecord;
		
		thisWaitRecord = OTGetLinkObject( OTRemoveFirst(&cancelParam->mpEP->waitRecords) , OTMPWaitRecord, link);
		assert(thisWaitRecord->magic == kOTMPWaitRecordMagic);
		
		CompleteWaitRecord(thisWaitRecord, kOTCanceledErr);
	}

	// Leave the notifier and unblock our caller.
	
	if (entered) {
		OTLeaveNotifier(cancelParam->mpEP->otEP);
	}
	CompleteWaitRecord(&cancelParam->waitRecord, err);
}

extern pascal OSStatus OTMPCancelSynchronousCalls(
								 OTMPProviderRef        ref,
                                 OSStatus               errParam)
	// OTMP API entry point.
{
	OSStatus 	err;
	CancelParam cancelParam;
	
	assert(ValidOTMPProviderRef(ref, false));
	assert(errParam < noErr);					// can't uncancel (which differs from OT)

	MPLog1(kOTMPAPILogID, kOTMPCloseMagic, ref);

	err = InitWaitRecord(&cancelParam.waitRecord, CancelAction, 0, NULL);
	if (err == noErr) {
		cancelParam.magic    = kOTMPCancelMagic;
		cancelParam.mpEP 	 = ref;
		cancelParam.errParam = errParam;
		err = QueueBlueAndWait(&cancelParam.waitRecord, false);
	}

	MPLog1(kOTMPAPILogID, MPLogTagReturn(kOTMPCancelMagic), (void *) err);
	return err;
}

/////////////////////////////////////////////////////////////////
#pragma mark **** General Stuff *****

// OTMPIoctl in the first OTMP API call we implement that follows 
// the standard pattern.  Standard API calls are divided into four 
// components.
//
// 1. The operation parameter block.  This embeds a wait record 
//    (or possibly a standard action structure, which itself embeds 
//    a wait record) and also includes other parameters for that 
//    operation.
//
// 2. The API routine itself.  This sets up the operation parameter 
//    block and installs a Blue action to run the action routine.
//
// 3. The action routine, executed at deferred task time via a Blue action.
//    This starts the operation asynchronously.  This may be a standard 
//    action routine, in which case it has a different prototype and 
//    is always executed within the context of the provider's notifier.
//
// 4. The notifier callback, called when the operation is run 
//    asynchronously and the asynchronous completion event is called.
//    This routine always runs at deferred task time in the context of the 
// 	  provide'rs notifier.  An operation that always runs synchronously, for 
//	  example OTLook, does not have a notifier routine.

enum {
	kOTMPIoctlMagic = '>Ioc'
};

struct IoctlParam {
	StdParam    stdParam;			// kOTMPIoctlMagic
	UInt32 		cmd;
	void * 		data;
};
typedef struct IoctlParam IoctlParam;

static pascal Boolean IoctlStdAction(StdParam *thisParam, OSStatus *errPtr)
	// The action callback for OTMPIoctl.
{
	OTResult 	err;
	IoctlParam *ioctlParam;
	
	assert( ! MPTaskIsPreemptive(kInvalidID) );
	assert( (TaskLevel() & kInDeferredTaskMask) == kInDeferredTaskMask );

	ioctlParam = OTGetLinkObject(thisParam, IoctlParam, stdParam);
	assert(ioctlParam->stdParam.magic == kOTMPIoctlMagic);
	
	err = OTIoctl(ioctlParam->stdParam.mpEP->otEP, ioctlParam->cmd, ioctlParam->data);

	// For async ioctl commands, the possibly positive ioctl result should
	// come back via the notifier, not via the function result.  The function
	// result should merely indicate whether the ioctl command was queued
	// correctly.  However, I'm a paranoid guy, so I normalise the result
	// here, and DebugStr if I seed the unexpected case.
	
	assert(err <= noErr);
	if (err > noErr) {
		err = noErr;
	}
	
	*errPtr = err;
	return (err != noErr);
}

extern pascal SInt32 OTMPIoctl(  OTMPProviderRef        ref,
                                 UInt32                 cmd,
                                 void *                 data)
	// OTMP API entry point.
{
	OSStatus 	err;
	IoctlParam	ioctlParam;

	assert(ValidOTMPProviderRef(ref, false));

	MPLog3(kOTMPAPILogID, kOTMPIoctlMagic, ref, (void *) cmd, data);
	
	err = InitStdParam(&ioctlParam.stdParam, kOTMPIoctlMagic, IoctlStdAction, ref, kStreamIoctlEvent, NULL);
	if (err == noErr) {
		ioctlParam.cmd  = cmd;
		ioctlParam.data = data;	
		err = QueueBlueAndWait(&ioctlParam.stdParam.waitRecord, true);
	}
	
	MPLog1(kOTMPAPILogID, MPLogTagReturn(kOTMPIoctlMagic), (void *) err);
	return err;
}

enum {
	kOTMPGetEndpointInfoMagic = '>Inf'
};

struct GetEndpointInfoParam {
	StdParam    	stdParam;			// kOTMPGetEndpointInfoMagic
	TEndpointInfo * info;
};
typedef struct GetEndpointInfoParam GetEndpointInfoParam;

static pascal Boolean GetEndpointInfoStdAction(StdParam *thisParam, OSStatus *errPtr)
	// The action callback for OTMPGetEndpointInfo.
{
	OTResult 				err;
	GetEndpointInfoParam *	getEndpointInfoParam;
	
	assert( ! MPTaskIsPreemptive(kInvalidID) );
	assert( (TaskLevel() & kInDeferredTaskMask) == kInDeferredTaskMask );

	getEndpointInfoParam = OTGetLinkObject(thisParam, GetEndpointInfoParam, stdParam);
	assert(getEndpointInfoParam->stdParam.magic == kOTMPGetEndpointInfoMagic);
	
	err = OTGetEndpointInfo( (EndpointRef)getEndpointInfoParam->stdParam.mpEP->otEP, getEndpointInfoParam->info);

	*errPtr = err;
	return (err != noErr);
}

extern pascal OSStatus OTMPGetEndpointInfo(
								 OTMPEndpointRef        ref,
                                 TEndpointInfo *        info)
	// OTMP API entry point.
{
	OSStatus 	   			err;
	GetEndpointInfoParam 	getEndpointInfoParam;

	assert(ValidOTMPProviderRef(ref, false));

	MPLog2(kOTMPAPILogID, kOTMPGetEndpointInfoMagic, ref, info);
	
	err = InitStdParam(&getEndpointInfoParam.stdParam, kOTMPGetEndpointInfoMagic, GetEndpointInfoStdAction, ref, T_GETINFOCOMPLETE, NULL);
	if (err == noErr) {
		getEndpointInfoParam.info = info;
		err = QueueBlueAndWait(&getEndpointInfoParam.stdParam.waitRecord, true);
	}
	
	MPLog1(kOTMPAPILogID, MPLogTagReturn(kOTMPGetEndpointInfoMagic), (void *) err);
	return err;
}

enum {
	kOTMPGetEndpointStateMagic = '>Sta'
};

struct GetEndpointStateParam {
	OSType			magic;				// must be kOTMPGetEndpointStateMagic
	OTMPWaitRecord  waitRecord;
	OTMPEndpointRef mpEP;
};
typedef struct GetEndpointStateParam GetEndpointStateParam;

static pascal void GetEndpointStateAction(MoreBlueAction *thisTask)
	// The action callback for OTMPGetEndpointState.
{
	OTResult 				err;
	GetEndpointStateParam *	getEndpointStateParam;
	
	assert( ! MPTaskIsPreemptive(kInvalidID) );
	assert( (TaskLevel() & kInDeferredTaskMask) == kInDeferredTaskMask );

	getEndpointStateParam = OTGetLinkObject(thisTask, GetEndpointStateParam, waitRecord.blue);
	assert(getEndpointStateParam->magic == kOTMPGetEndpointStateMagic);

	err = getEndpointStateParam->mpEP->cancelErr;
	if (err == noErr) {
		err = OTGetEndpointState((EndpointRef)getEndpointStateParam->mpEP->otEP);
	}
	
	CompleteWaitRecord(&getEndpointStateParam->waitRecord, err);
}

extern pascal OTResult OTMPGetEndpointState(OTMPEndpointRef ref)
	// OTMP API entry point.
{
	OSStatus 				err;
	GetEndpointStateParam 	getEndpointStateParam;
	
	assert(ValidOTMPProviderRef(ref, false));
	MPLog1(kOTMPAPILogID, kOTMPGetEndpointStateMagic, ref);
	
	err = InitWaitRecord(&getEndpointStateParam.waitRecord, GetEndpointStateAction, 0, NULL);
	if (err == noErr) {
		getEndpointStateParam.magic	= kOTMPGetEndpointStateMagic;
		getEndpointStateParam.mpEP	= ref;
		err = QueueBlueAndWait(&getEndpointStateParam.waitRecord, false);
	}

	MPLog1(kOTMPAPILogID, MPLogTagReturn(kOTMPGetEndpointStateMagic), (void *) err);
	return err;
}

enum {
	kOTMPLookMagic = '>Lok'					// forward declared for debugging hackery
};

struct LookParam {
	OSType			magic;				// must be kOTMPLookMagic
	OTMPWaitRecord  waitRecord;
	OTMPEndpointRef mpEP;
};
typedef struct LookParam LookParam;

static pascal void LookAction(MoreBlueAction *thisTask)
	// The action callback for OTMPLook.
{
	OTResult 	err;
	LookParam *	lookParam;
	
	assert( ! MPTaskIsPreemptive(kInvalidID) );
	assert( (TaskLevel() & kInDeferredTaskMask) == kInDeferredTaskMask );

	lookParam = OTGetLinkObject(thisTask, LookParam, waitRecord.blue);
	assert(lookParam->magic == kOTMPLookMagic);

	// If OTLook returns a T_GODATA or T_GOEXDATA event to you, 
	// OT decides that you know about this particular event and won't 
	// call your notifier with the same information.  [The fact that 
	// Look consumes T_GODATA and T_GOEXDATA is mandated by XTI.]  In our 
	// case this would be bad, because we have MP tasks blocked waiting for 
	// those events.  I solve this problem by calling our notifier directly 
	// if we get one of these events.
	
	err = lookParam->mpEP->cancelErr;
	if (err == noErr) {
		err = OTLook((EndpointRef)lookParam->mpEP->otEP);
	}
	if (err == T_GODATA || err == T_GOEXDATA) {

		// I tried really hard to exercise this code path but my most valiant 
		// attempts all failed.  If you see this DebugStr and the code worked, 
		// let me know and I'll take it out.
		
		#if MORE_DEBUG
			DebugStr("\pLookAction: Consumed T_GODATA so must call notifier manually");
		#endif
		
		// Note here that err does double duty as the operation result and 
		// as the code parameter to the notifier.  Also, the cookie parameter 
		// to a T_GODATA event is the raw OT EndpointRef (although our notifier 
		// doesn't actually need that info because it works entirely from the 
		// contextPtr.
		
	    InvokeOTNotifyUPP(lookParam->mpEP, err, noErr, lookParam->mpEP->otEP, gNotifierToInstall);
	}
	
	CompleteWaitRecord(&lookParam->waitRecord, err);
}

extern pascal OTResult OTMPLook(OTMPEndpointRef ref)
	// OTMP API entry point.
{
	OSStatus 	err;
	LookParam 	lookParam;
	
	assert(ValidOTMPProviderRef(ref, false));
	// I commented out the MPLog's for OTMPLook because one of my test 
	// programs (the one trying to exercise the special case in 
	// LookAction) was calling this routine about 50,000 times a second!
	// You should feel free to add this log point back in if you need it.
	// MPLog1(kOTMPAPILogID, kOTMPLookMagic, ref);
	
	err = InitWaitRecord(&lookParam.waitRecord, LookAction, 0, NULL);
	if (err == noErr) {
		lookParam.magic	= kOTMPLookMagic;
		lookParam.mpEP	= ref;
		err = QueueBlueAndWait(&lookParam.waitRecord, false);
	}

	// MPLog1(kOTMPAPILogID, MPLogTagReturn(kOTMPLookMagic), (void *) err);
	return err;
}

enum {
	kOTMPCountDataBytesMagic = '>Cnt'
};

struct CountDataBytesParam {
	OSType			magic;				// must be kOTMPCountDataBytesMagic
	OTMPWaitRecord  waitRecord;
	OTMPEndpointRef mpEP;
	OTByteCount *   countPtr;
};
typedef struct CountDataBytesParam CountDataBytesParam;

static pascal void CountDataBytesAction(MoreBlueAction *thisTask)
	// The action callback for OTMPCountDataBytes.
{
	OTResult 				err;
	CountDataBytesParam *	countDataBytesParam;
	
	assert( ! MPTaskIsPreemptive(kInvalidID) );
	assert( (TaskLevel() & kInDeferredTaskMask) == kInDeferredTaskMask );

	countDataBytesParam = OTGetLinkObject(thisTask, CountDataBytesParam, waitRecord.blue);
	assert(countDataBytesParam->magic == kOTMPCountDataBytesMagic);

	err = countDataBytesParam->mpEP->cancelErr;
	if (err == noErr) {
		err = OTCountDataBytes((EndpointRef)countDataBytesParam->mpEP->otEP, countDataBytesParam->countPtr);
	}
	
	CompleteWaitRecord(&countDataBytesParam->waitRecord, err);
}

extern pascal OTResult OTMPCountDataBytes(
								 OTMPEndpointRef        ref,
                                 OTByteCount *          countPtr)
	// OTMP API entry point.
{
	OSStatus 			err;
	CountDataBytesParam countDataBytesParam;
	
	assert(ValidOTMPProviderRef(ref, false));
	MPLog2(kOTMPAPILogID, kOTMPCountDataBytesMagic, ref, countPtr);
	
	err = InitWaitRecord(&countDataBytesParam.waitRecord, CountDataBytesAction, 0, NULL);
	if (err == noErr) {
		countDataBytesParam.magic		= kOTMPCountDataBytesMagic;
		countDataBytesParam.mpEP		= ref;
		countDataBytesParam.countPtr	= countPtr;
		err = QueueBlueAndWait(&countDataBytesParam.waitRecord, false);
	}

	MPLog1(kOTMPAPILogID, MPLogTagReturn(kOTMPCountDataBytesMagic), (void *) err);
	return err;
}

enum {
	kOTMPGetProtAddressMagic = '>Prt'
};

struct GetProtAddressParam {
	StdParam	stdParam;				// kOTMPGetProtAddressMagic
	TBind *		boundAddr;
	TBind *		peerAddr;
};
typedef struct GetProtAddressParam GetProtAddressParam;

static pascal Boolean GetProtAddressStdAction(StdParam *thisParam, OSStatus *errPtr)
	// The action callback for OTMPGetProAddress.
{
	OTResult 				err;
	GetProtAddressParam *	getProtAddressParam;
	
	assert( ! MPTaskIsPreemptive(kInvalidID) );
	assert( (TaskLevel() & kInDeferredTaskMask) == kInDeferredTaskMask );

	getProtAddressParam = OTGetLinkObject(thisParam, GetProtAddressParam, stdParam);
	assert(getProtAddressParam->stdParam.magic == kOTMPGetProtAddressMagic);
	
	err = OTGetProtAddress((EndpointRef)getProtAddressParam->stdParam.mpEP->otEP, getProtAddressParam->boundAddr, getProtAddressParam->peerAddr);

	*errPtr = err;
	return (err != noErr);
}

extern pascal OSStatus OTMPGetProtAddress(
								 OTMPEndpointRef        ref,
                                 TBind *                boundAddr,
                                 TBind *                peerAddr)
	// OTMP API entry point.
{
	OSStatus 	   		err;
	GetProtAddressParam getProtAddressParam;

	assert(ValidOTMPProviderRef(ref, false));
	assert( (boundAddr != NULL) || (peerAddr != NULL) );
	MPLog3(kOTMPAPILogID, kOTMPGetProtAddressMagic, ref, boundAddr, peerAddr);
	
	err = InitStdParam(&getProtAddressParam.stdParam, kOTMPGetProtAddressMagic, GetProtAddressStdAction, ref, T_GETPROTADDRCOMPLETE, NULL);
	if (err == noErr) {
		getProtAddressParam.boundAddr = boundAddr;
		getProtAddressParam.peerAddr  = peerAddr;
		err = QueueBlueAndWait(&getProtAddressParam.stdParam.waitRecord, true);
	}
	
	MPLog1(kOTMPAPILogID, MPLogTagReturn(kOTMPGetProtAddressMagic), (void *) err);
	return err;
}

enum {
	kOTMPResolveAddressMagic = '>Rlv'
};

struct ResolveAddressParam {
	StdParam    stdParam;				// kOTMPResolveAddressMagic
	TBind *   	reqAddr;
	TBind *     retAddr;
	OTTimeout   timeOut;
};
typedef struct ResolveAddressParam ResolveAddressParam;

static pascal Boolean ResolveAddressStdAction(StdParam *thisParam, OSStatus *errPtr)
	// The action callback for OTMPResolveAddress.
{
	OTResult 				err;
	ResolveAddressParam *	getProtAddressParam;
	
	assert( ! MPTaskIsPreemptive(kInvalidID) );
	assert( (TaskLevel() & kInDeferredTaskMask) == kInDeferredTaskMask );

	getProtAddressParam = OTGetLinkObject(thisParam, ResolveAddressParam, stdParam);
	assert(getProtAddressParam->stdParam.magic == kOTMPResolveAddressMagic);
	
	err = OTResolveAddress((EndpointRef)getProtAddressParam->stdParam.mpEP->otEP, 
							getProtAddressParam->reqAddr, getProtAddressParam->retAddr,
							getProtAddressParam->timeOut);

	*errPtr = err;
	return (err != noErr);
}

extern pascal OSStatus OTMPResolveAddress(
								 OTMPEndpointRef        ref,
                                 TBind *                reqAddr,
                                 TBind *                retAddr,
                                 OTTimeout              timeOut)
	// OTMP API entry point.
{
	OSStatus 	   		err;
	ResolveAddressParam getProtAddressParam;

	assert(ValidOTMPProviderRef(ref, false));
	assert(reqAddr != NULL);
	assert(retAddr != NULL);
	MPLog4(kOTMPAPILogID, kOTMPResolveAddressMagic, ref, reqAddr, retAddr, (void *) timeOut);
	
	err = InitStdParam(&getProtAddressParam.stdParam, kOTMPResolveAddressMagic, ResolveAddressStdAction, ref, T_RESOLVEADDRCOMPLETE, NULL);
	if (err == noErr) {
		getProtAddressParam.reqAddr = reqAddr;
		getProtAddressParam.retAddr = retAddr;
		getProtAddressParam.timeOut = timeOut;
		err = QueueBlueAndWait(&getProtAddressParam.stdParam.waitRecord, true);
	}
	
	MPLog1(kOTMPAPILogID, MPLogTagReturn(kOTMPResolveAddressMagic), (void *) err);
	return err;
}

enum {
	kOTMPOptionManagementMagic = '>Opt'
};

struct OptionManagementParam {
	StdParam    stdParam;				// kOTMPOptionManagementMagic
	TOptMgmt *	req;
	TOptMgmt *	ret;	
};
typedef struct OptionManagementParam OptionManagementParam;

static pascal Boolean OptionManagementStdAction(StdParam *thisParam, OSStatus *errPtr)
	// The action callback for OTMPOptionManagement.
{
	OTResult err;
	OptionManagementParam *optionManagementParam;
	
	assert( ! MPTaskIsPreemptive(kInvalidID) );
	assert( (TaskLevel() & kInDeferredTaskMask) == kInDeferredTaskMask );

	optionManagementParam = OTGetLinkObject(thisParam, OptionManagementParam, stdParam);
	assert(optionManagementParam->stdParam.magic == kOTMPOptionManagementMagic);
	
	err = OTOptionManagement((EndpointRef)optionManagementParam->stdParam.mpEP->otEP, optionManagementParam->req, optionManagementParam->ret);

	// Can OTOptionManagement return a positive result?  I don't think so, but I 
	// want to be certain so I assert it here.
	
	assert(err <= noErr);
	
	*errPtr = err;
	return (err != noErr);
}

extern pascal OSStatus OTMPOptionManagement(
								 OTMPEndpointRef        ref,
                                 TOptMgmt *             req,
                                 TOptMgmt *             ret)
	// OTMP API entry point.
{
	OSStatus 	   			err;
	OptionManagementParam 	optionManagementParam;

	assert(ValidOTMPProviderRef(ref, false));
	assert(req != NULL);
	MPLog3(kOTMPAPILogID, kOTMPOptionManagementMagic, ref, req, ret);
	
	err = InitStdParam(&optionManagementParam.stdParam, kOTMPOptionManagementMagic, OptionManagementStdAction, ref, T_OPTMGMTCOMPLETE, NULL);
	if (err == noErr) {
		optionManagementParam.req  = req;
		optionManagementParam.ret  = ret;	
		err = QueueBlueAndWait(&optionManagementParam.stdParam.waitRecord, true);
	}
	
	MPLog1(kOTMPAPILogID, MPLogTagReturn(kOTMPOptionManagementMagic), (void *) err);
	return err;
}

/////////////////////////////////////////////////////////////////
#pragma mark **** Bind/Unbind *****

enum {
	kOTMPBindMagic = '>Bnd'
};

struct BindParam {
	StdParam    stdParam;			// kOTMPBindMagic
	TBind *		reqAddr;
	TBind *		retAddr;
};
typedef struct BindParam BindParam;

static pascal Boolean BindStdAction(StdParam *thisParam, OSStatus *errPtr)
	// The action callback for OTMPBind.
{
	OTResult 	err;
	BindParam *	bindParam;
	
	assert( ! MPTaskIsPreemptive(kInvalidID) );
	assert( (TaskLevel() & kInDeferredTaskMask) == kInDeferredTaskMask );

	bindParam = OTGetLinkObject(thisParam, BindParam, stdParam);
	assert(bindParam->stdParam.magic == kOTMPBindMagic);
	
	err = OTBind((EndpointRef)bindParam->stdParam.mpEP->otEP, bindParam->reqAddr, bindParam->retAddr);

	*errPtr = err;
	return (err != noErr);
}

extern pascal OSStatus OTMPBind( OTMPEndpointRef        ref,
                                 TBind *                reqAddr, /* can be NULL */
                                 TBind *                retAddr) /* can be NULL */
	// OTMP API entry point.
{
	OSStatus	err;
	BindParam	bindParam;

	assert(ValidOTMPProviderRef(ref, false));
	MPLog3(kOTMPAPILogID, kOTMPBindMagic, ref, reqAddr, retAddr);
	
	err = InitStdParam(&bindParam.stdParam, kOTMPBindMagic, BindStdAction, ref, T_BINDCOMPLETE, NULL);
	if (err == noErr) {
		bindParam.reqAddr = reqAddr;
		bindParam.retAddr = retAddr;
		err = QueueBlueAndWait(&bindParam.stdParam.waitRecord, true);
	}
	
	MPLog1(kOTMPAPILogID, MPLogTagReturn(kOTMPBindMagic), (void *) err);
	return err;
}

enum {
	kOTMPUnbindMagic = '>Unb'
};

struct UnbindParam {
	StdParam    stdParam;			// kOTMPUnbindMagic
};
typedef struct UnbindParam UnbindParam;

static pascal Boolean UnbindStdAction(StdParam *thisParam, OSStatus *errPtr)
	// The action callback for OTMPUnbind.
{
	OTResult 		err;
	UnbindParam *	unbindParam;
	
	assert( ! MPTaskIsPreemptive(kInvalidID) );
	assert( (TaskLevel() & kInDeferredTaskMask) == kInDeferredTaskMask );

	unbindParam = OTGetLinkObject(thisParam, UnbindParam, stdParam);
	assert(unbindParam->stdParam.magic == kOTMPUnbindMagic);
	
	err = OTUnbind((EndpointRef)unbindParam->stdParam.mpEP->otEP);

	*errPtr = err;
	return (err != noErr);
}

extern pascal OSStatus OTMPUnbind(OTMPEndpointRef ref)
	// OTMP API entry point.
{
	OSStatus 		err;
	UnbindParam 	unbindParam;

	assert(ValidOTMPProviderRef(ref, false));
	MPLog1(kOTMPAPILogID, kOTMPUnbindMagic, ref);
	
	err = InitStdParam(&unbindParam.stdParam, kOTMPUnbindMagic, UnbindStdAction, ref, T_UNBINDCOMPLETE, NULL);
	if (err == noErr) {
		err = QueueBlueAndWait(&unbindParam.stdParam.waitRecord, true);
	}
	
	MPLog1(kOTMPAPILogID, MPLogTagReturn(kOTMPUnbindMagic), (void *) err);
	return err;
}

/////////////////////////////////////////////////////////////////
#pragma mark **** Connect/Disconnect *****

enum {
	kOTMPConnectMagic = '>Con'
};

struct ConnectParam {
	StdParam    stdParam;				// kOTMPConnectMagic
	TCall *		sndCall;
	TCall *		rcvCall;
};
typedef struct ConnectParam ConnectParam;

static pascal Boolean ConnectStdAction(StdParam *thisParam, OSStatus *errPtr)
	// The action callback for OTMPConnect.
{
	OTResult 		err;
	ConnectParam *	connectParam;
	
	assert( ! MPTaskIsPreemptive(kInvalidID) );
	assert( (TaskLevel() & kInDeferredTaskMask) == kInDeferredTaskMask );

	connectParam = OTGetLinkObject(thisParam, ConnectParam, stdParam);
	assert(connectParam->stdParam.magic == kOTMPConnectMagic);
	
	err = OTConnect((EndpointRef)connectParam->stdParam.mpEP->otEP, connectParam->sndCall, connectParam->rcvCall);
	if (err == kOTNoDataErr) {
		err = noErr;				// kOTNoDataErr means async connect is in progress
	} else if (err == noErr) {
		assert(false);			// doesn't make sense for async OTConnect to return noErr
	}

	*errPtr = err;
	return (err != noErr);
}

static pascal void ConnectNotifier(OTMPWaitRecord *waitRec, OTEventCode code, OTResult result, void *cookie)
	// The notification callback for OTMPConnect.
	//
	// This is the first example of a notification callback that 
	// might complete with an event other than the event specified in 
	// the waitingFor field of the wait record.  In this case, the 
	// real completion event is T_CONNECT, but the notifier can be 
	// called with either a T_CONNECT or a T_DISCONNECT.  [This fuzzy 
	// matching logic is implemented in WaitRecordSearchProc.]
	// This routine has to handle being called with either of those events. 
	// If the event is a T_CONNECT, the right thing has happened and we 
	// receive the connection data into the user's buffer.  If the 
	// event is T_DISCONNECT the routine completes the client's 
	// OTMPConnect request with a kOTLookErr.
{
	ConnectParam * connectParam;
	#pragma unused(cookie)

	assert( ! MPTaskIsPreemptive(kInvalidID) );
	assert( (TaskLevel() & kInDeferredTaskMask) == kInDeferredTaskMask );

	assert( (code == T_CONNECT) || (code == T_DISCONNECT) );
	
	connectParam = OTGetLinkObject(waitRec, ConnectParam, stdParam.waitRecord);
	assert(connectParam->stdParam.magic == kOTMPConnectMagic);

	switch (code) {
		case T_CONNECT:
			assert(result == noErr);		// error would have caused T_DISCONNECT
			result = OTRcvConnect((EndpointRef)connectParam->stdParam.mpEP->otEP, connectParam->rcvCall);
			
			// The call to OTRcvConnect can itself fail with a look error if a disconnect
			// arrives between the time when the T_CONNECT message was sent to us and
			// the time we call OTRcvConnect.  This code handles that case correctly.
			// The look error is returned as the result of the OTMPConnect, as expected.
			// The client can then call OTMPLook to see the reason and continue, without
			// really knowing that the connection was ever in place.
			
			break;
		case T_DISCONNECT:
			assert(result == noErr);		// connect error causes T_DISCONNECT
												// client must OTMPLook then OTMPRcvDisconnect
												// to clear state and get error
			result = kOTLookErr;
			break;
		default:
			assert(false);
			break;
	}
	
	CompleteWaitRecord(waitRec, result);
}

extern pascal OSStatus OTMPConnect(OTMPEndpointRef      ref,
                                 TCall *                sndCall,
                                 TCall *                rcvCall)
	// OTMP API entry point.
{
	OSStatus 		err;
	ConnectParam	connectParam;

	assert(ValidOTMPProviderRef(ref, false));
	assert(sndCall != NULL);
	MPLog3(kOTMPAPILogID, kOTMPConnectMagic, ref, sndCall, rcvCall);

	err = InitStdParam(&connectParam.stdParam, kOTMPConnectMagic, ConnectStdAction, ref, T_CONNECT, ConnectNotifier);
	if (err == noErr) {
		connectParam.sndCall = sndCall;
		connectParam.rcvCall = rcvCall;
		err = QueueBlueAndWait(&connectParam.stdParam.waitRecord, true);
	}
	
	MPLog1(kOTMPAPILogID, MPLogTagReturn(kOTMPConnectMagic), (void *) err);
	return err;
}

enum {
	kOTMPRcvDisconnectMagic = '>Rdi'
};

struct RcvDisconnectParam {
	OSType			magic;				// must be kOTMPRcvDisconnectMagic
	OTMPWaitRecord  waitRecord;
	OTMPEndpointRef mpEP;
	TDiscon *		discon;
};
typedef struct RcvDisconnectParam RcvDisconnectParam;

static pascal void RcvDisconnectAction(MoreBlueAction *thisTask)
	// The action callback for OTMPRcvDisconnect.
{
	OTResult 			err;
	RcvDisconnectParam *rcvDisconnectParam;
	
	assert( ! MPTaskIsPreemptive(kInvalidID) );
	assert( (TaskLevel() & kInDeferredTaskMask) == kInDeferredTaskMask );

	rcvDisconnectParam = OTGetLinkObject(thisTask, RcvDisconnectParam, waitRecord.blue);
	assert(rcvDisconnectParam->magic == kOTMPRcvDisconnectMagic);
	
	err = rcvDisconnectParam->mpEP->cancelErr;
	if (err == noErr) {
		err = OTRcvDisconnect((EndpointRef)rcvDisconnectParam->mpEP->otEP, rcvDisconnectParam->discon);
	}
	
	CompleteWaitRecord(&rcvDisconnectParam->waitRecord, err);
}

extern pascal OSStatus OTMPRcvDisconnect(
								 OTMPEndpointRef        ref,
                                 TDiscon *              discon) /* can be NULL */
	// OTMP API entry point.
{
	OSStatus 			err;
	RcvDisconnectParam 	rcvDisconnectParam;
	
	assert(ValidOTMPProviderRef(ref, false));
	MPLog2(kOTMPAPILogID, kOTMPRcvDisconnectMagic, ref, discon);
	
	err = InitWaitRecord(&rcvDisconnectParam.waitRecord, RcvDisconnectAction, 0, NULL);
	if (err == noErr) {
		rcvDisconnectParam.magic	= kOTMPRcvDisconnectMagic;
		rcvDisconnectParam.mpEP		= ref;
		rcvDisconnectParam.discon	= discon;
		err = QueueBlueAndWait(&rcvDisconnectParam.waitRecord, false);
	}

	MPLog1(kOTMPAPILogID, MPLogTagReturn(kOTMPRcvDisconnectMagic), (void *) err);
	return err;
}

enum {
	kOTMPSndDisconnectMagic = '>Sdi'
};

struct SndDisconnectParam {
	StdParam    stdParam;			// kOTMPSndDisconnectMagic
	TCall *		call;
};
typedef struct SndDisconnectParam SndDisconnectParam;

static pascal Boolean SndDisconnectStdAction(StdParam *thisParam, OSStatus *errPtr)
	// The action callback for OTMPSndDisconnect.
{
	OTResult 			err;
	SndDisconnectParam *sndDisconnectParam;
	
	assert( ! MPTaskIsPreemptive(kInvalidID) );
	assert( (TaskLevel() & kInDeferredTaskMask) == kInDeferredTaskMask );

	sndDisconnectParam = OTGetLinkObject(thisParam, SndDisconnectParam, stdParam);
	assert(sndDisconnectParam->stdParam.magic == kOTMPSndDisconnectMagic);
	
	err = OTSndDisconnect((EndpointRef)sndDisconnectParam->stdParam.mpEP->otEP, sndDisconnectParam->call);

	*errPtr = err;
	return (err != noErr);
}

extern pascal OSStatus OTMPSndDisconnect(
								 OTMPEndpointRef        ref,
                                 TCall *                call) /* can be NULL */
	// OTMP API entry point.
{
	OSStatus 	   		err;
	SndDisconnectParam 	sndDisconnectParam;

	assert(ValidOTMPProviderRef(ref, false));
	MPLog2(kOTMPAPILogID, kOTMPSndDisconnectMagic, ref, call);
	
	err = InitStdParam(&sndDisconnectParam.stdParam, kOTMPSndDisconnectMagic, SndDisconnectStdAction, ref, T_DISCONNECTCOMPLETE, NULL);
	if (err == noErr) {
		sndDisconnectParam.call = call;
		err = QueueBlueAndWait(&sndDisconnectParam.stdParam.waitRecord, true);
	}
	
	MPLog1(kOTMPAPILogID, MPLogTagReturn(kOTMPSndDisconnectMagic), (void *) err);
	return err;
}

/////////////////////////////////////////////////////////////////
#pragma mark **** Send/Receive *****

// Send Retry Profiling
// --------------------
// Under high load OT can fail a send request (OTSnd, OTSndUData) with 
// a kOTOutOfMemoryErr error.  OT could not allocate enough memory to 
// start the send, but the stream itself isn't flow controlled (so you 
// won't get a T_GODATA event to continue the send).  The correct response 
// when you get this error is to delay for some short period of time and 
// then retry the send.
// 
// This raises the question, what is the correct amount of time to delay? 
// I used the following code to work out a delay that seems to work well. 
// I really haven't explored this issue in great depth, but this 
// infrastructure should allow someone (either folks using this code or 
// me, when I get more time) 

enum {
	kOTMPSndRetrySystemTaskThreshold = 6,		// number of times to retry with binary backoff 
												// before going to system task code
	kOTMPSndRetryMax = 10						// total number of times to retry
};

#if MORE_DEBUG

	static SInt32 gSndFlowCounter;
		// Total number of times we've handled a kOTFlowErr result from a 
		// send operation.

	// Normally I don't want to include any standard C I/O libraries 
	// in my library code, but in the case of this debugging facility 
	// it is necessary.
	
	#include <stdio.h>
	
	static SInt32 gSndRetryFreqDist[kOTMPSndRetryMax + 1];
		// A frequency distribution of number of times we've retried 
		// at a particular retry count.
	
	extern void OTMPPrintSndRetryFreqDist(void)
		// See comment in header file.
	{
		SInt32 i;
		
		printf("PrintSndRetryFreqDist\n");
		for (i = 0; i < kOTMPSndRetryMax + 1; i++) {
			printf("[%ld] = %ld", i, gSndRetryFreqDist[i]);
			if (i == kOTMPSndRetrySystemTaskThreshold) {
				printf(" <-- kOTMPSndRetrySystemTaskThreshold");
			}
			printf("\n");
			gSndRetryFreqDist[i] = 0;
		}
		printf("gSndFlowCounter = %ld\n", gSndFlowCounter);
		gSndFlowCounter = 0;
	}

#endif

static void* SndRetrySystemTask(void *parameter)
	// This routine is called via MPRemoteCall in the event 
	// of too many send retries (ie too many kOTOutOfMemoryErr 
	// errors being returned by OTSnd or OTSndUData).  It doesn't 
	// actually do anything except delay the calling MP task until 
	// the next time that SystemTask is called.  OT hooks into 
	// SystemTask and uses that to grow the OT memory pools, thereby 
	// hopefully overcoming the memory crisis that prompted the error.
{
	#pragma unused(parameter)

	assert( ! MPTaskIsPreemptive(kInvalidID) );
	assert( TaskLevel() == 0 );
		
	#if MORE_DEBUG
		// DebugStr("\pSndSystemTask");
	#endif

	// This call to OTIdle is completely unnecessary.  The main purpose 
	// of this routine is to delay the task calling OTMPSnd until SystemTask 
	// has been called.  OT should grow the OT kernel pool at system task time 
	// and hence relieve the out of memory condition.  I just call call OTIdle 
	// so that the routine isn't entirely empty (-:
	
	OTIdle();
	
	return NULL;
}

static OSStatus SndRetryDelay(UInt32 retryCount, AbsoluteTime *currentDelay)
	// This is common code factored out of OTSnd and OTSndUData.  
	// Given a retry count (ie the number of times we've retried) 
	// this routine delays for the appropriate amount of time. 
	// It also updates currentDelay for the next time that it is 
	// called.  [By doubling current delay in this routine and recording 
	// the current delay, we can avoid the complex maths required to 
	// generate currentDelay from retryCount.]
{
	OSStatus err;

	// Record the retry in our statistics array.
	
	#if MORE_DEBUG
		(void) OTAtomicAdd32(1, &gSndRetryFreqDist[retryCount]);
	#endif
	
	if (retryCount >= kOTMPSndRetrySystemTaskThreshold) {
		// If we've retried more than a certain threshold,
		// schedule an RPC, which effectively delays us until the next 
		// time that SystemTask is called.
		(void) MPRemoteCall(SndRetrySystemTask, NULL, kMPAnyRemoteContext);
		// Don't MPDelayUntil or adjust currentDelay once we've switched 
		// to "system task" mode.
		err = noErr;
	} else {
		AbsoluteTime timeToWakeUp;

		// Otherwise just delay the specified amount and then double the 
		// delay for the next time we're called.  Voila, exponential backoff.
				
		timeToWakeUp = AddAbsoluteToAbsolute(UpTime(), *currentDelay);
		err = MPDelayUntil(&timeToWakeUp);
		if (err == noErr) {
			// Double the delay for next time around.
			*currentDelay = AddAbsoluteToAbsolute(*currentDelay, *currentDelay);
		}
	}
	return err;
}

enum {
	kOTMPSndMagic = '>Snd',
	kOTMPSndMemoryRetry = '*Snd'
};

struct SndParam {
	StdParam	stdParam;			// kOTMPSndMagic
	void *		buf;
	OTByteCount	nbytes;
	OTFlags		flags;
	UInt32		memoryRetries;
};
typedef struct SndParam SndParam;

static pascal Boolean SndStdAction(StdParam *thisParam, OSStatus *errPtr)
	// The action callback for OTMPSnd.
{
	OSStatus 	err;
	Boolean  	done;
	SndParam *	sndParam;
	
	assert( ! MPTaskIsPreemptive(kInvalidID) );
	assert( (TaskLevel() & kInDeferredTaskMask) == kInDeferredTaskMask );

	sndParam = OTGetLinkObject(thisParam, SndParam, stdParam);
	assert(sndParam->stdParam.magic == kOTMPSndMagic);

	// Must be called from within context of notifier. 
	// The easy way to test this is by calling OTEnterNotifier, 
	// expecting it to return false (ie we couldn't enter the 
	// notifier because we're already inside).
	
	assert( !OTEnterNotifier(sndParam->stdParam.mpEP->otEP) );
	
	do {	
		// Initialise done to a bogus Boolean to detect flawed control logic.

		#if MORE_DEBUG
			done = 66;
		#endif
		
		// On the last memory retry, drop into the debugger so that we can 
		// trace through OTSnd and figure out why it keeps returning the 
		// kOTOutOfMemoryErr error.
		
		#if MORE_DEBUG
			if (sndParam->memoryRetries == (kOTMPSndRetryMax - 1)) {
				DebugStr("\pOTMPSnd: Last retry");
			}
		#endif

		// Attempt the send.
		
		err = OTSnd((EndpointRef)sndParam->stdParam.mpEP->otEP, sndParam->buf, sndParam->nbytes, sndParam->flags);
		
		// There are 6 possible results of this call.
		//
		// 1. success (err == nbytes)
		// 2. partial success (0 < err < nybytes)
		// 3. flow control (err = kOTFlowErr)
		// 4. weird memory stall (err = kOTOutOfMemoryErr)
		// 5. look
		// 6. other error
		
		if (err == sndParam->nbytes) {
			// Case 1
			err = noErr;
			done = true;
		} else if ( (err > 0) && (err < sndParam->nbytes) ) {
			// Case 2
			
			// If you haven't set T_MORE and OT only accepts one part of the data 
			// then things get really weird.  We need to go back in time and 
			// set T_MORE on the previous call to OTSnd.  We can't do that, so
			// instead we let you know when this happens.
			//
			// This is only a problem for endpoints that preserve TSDU boundaries.
			// If this assert fires too often (because the primary use of this 
			// module is TCP and TCP does not preserve boundaries) we should comment 
			// it out.  Ideally, the long term solution is for the OTMPEndpoint to 
			// record whether the endpoint's provider preserves TSDU boundaries and 
			// only fire this assert for providers that do.
			//
			// Note that the handle of the other flag, T_EXPEDITED, is correct.
			
			assert( (sndParam->flags & T_MORE) == T_MORE );
			
			((char *) sndParam->buf) += err;
			sndParam->nbytes -= err;
			err = noErr;
			done = false;
		} else if (err == kOTFlowErr) {
			// Case 3
			#if MORE_DEBUG
				(void) OTAtomicAdd32(1, &gSndFlowCounter);
			#endif
			done = true;
		} else if (err == kOTOutOfMemoryErr) {
			// Case 4
			// Return the error back to the MP task.  It will delay 
			// and retry.
			#if MORE_DEBUG
				if (sndParam->memoryRetries == (kOTMPSndRetryMax - 1)) {
					DebugStr("\pOTMPSnd: Retried too many times");
				}
			#endif
			done = true;
		} else {
			// Case 5, 6
			// Return the error back to the MP task.  It will return
			// the error to the caller.
			assert(err != noErr);
			done = true;
		}
		
		// All branches of the above if statement must set the control variable.
		
		assert(done == true          || done == false         );
	} while ( ! done );

	if (err == kOTFlowErr) {
		*errPtr = noErr;
		return false;
	} else {
		*errPtr = err;
		return true;
	}
}

static pascal void SndNotifier(OTMPWaitRecord *waitRec, OTEventCode code, OTResult result, void *cookie)
	// The notification callback for OTMPSnd.
	//
	// This is another example, like ConnectNotifier, of a notifier that 
	// can be called with a code different from the code that's expected.
{
	OSStatus 	err;
	SndParam *	sndParam;
	Boolean 	completeIt;
	#pragma unused(result)
	#pragma unused(cookie)

	assert( ! MPTaskIsPreemptive(kInvalidID) );
	assert( (TaskLevel() & kInDeferredTaskMask) == kInDeferredTaskMask );

	assert( (code == T_GODATA) || (code == T_DISCONNECT) || (code == T_ORDREL) );
	
	sndParam = OTGetLinkObject(waitRec, SndParam, stdParam.waitRecord);
	assert(sndParam->stdParam.magic == kOTMPSndMagic);

	assert(waitRec == &sndParam->stdParam.waitRecord);

	switch (code) {	
		case T_GODATA:
			completeIt = SndStdAction(&sndParam->stdParam, &err);
			break;
		case T_DISCONNECT:
		case T_ORDREL:
			completeIt = true;
			err = kOTLookErr;
			break;
		default:
			assert(false);
			break;
	}

	if (completeIt) {
		CompleteWaitRecord(waitRec, err);
	} else {
		assert(waitRec->link.fNext == NULL);
		OTAddFirst(&sndParam->stdParam.mpEP->waitRecords, &waitRec->link);
	}
}

extern pascal OTResult OTMPSnd(	 OTMPEndpointRef        ref,
                                 void *                 buf,
                                 OTByteCount            nbytes,
                                 OTFlags                flags)
	// OTMP API entry point.
{
	OSStatus 		err;
	SndParam 		sndParam;
	AbsoluteTime	currentDelay;
	
	assert(ValidOTMPProviderRef(ref, false));
	assert(buf != NULL);
	assert(nbytes < 0x01000000);		// arbitrary limit to catch wacky errors
	MPLog4(kOTMPAPILogID, kOTMPSndMagic, ref, buf, (void *) nbytes, (void *) flags);
	
	err = InitStdParam(&sndParam.stdParam, kOTMPSndMagic, SndStdAction, ref, T_GODATA, SndNotifier);
	if (err == noErr) {
		sndParam.buf 	= buf;
		sndParam.nbytes = nbytes;
		sndParam.flags 	= flags;
	
		// In some cases, OTSnd will return a mysterious kOTOutOfMemoryErr, 
		// which indicates that the OT client libraries didn't have enough 
		// memory to send a request to the kernel even though the stream 
		// wasn't flowed controlled.  The accepted response to this is to 
		// delay for some short period of time before retrying.  We do this 
		// on the MP side rather than the Blue side because it's much 
		// easier to delay an MP task using MPDelayUntil than it is to 
		// retries the Blue side operation using a Time Manager task.
		//
		// Some interesting questions include how long should we delay 
		// and how many times should we retry.  I have chosen a binary 
		// exponential backoff algorithm which allows for very short 
		// retry times (to catch times when a network data consumer is 
		// freeing memory in parallel with us consuming it) that expands 
		// to very long retry times (to allow OT to grow the memory 
		// pool).  Also, after a certain number of retries we issue an 
		// MPRemoteCall, which should trigger a system task which should 
		// grow the OT memory pool.
		//
		// This algorithm is an obvious place to look when performance tuning. 
		// I've done a limited amount of testing to choose reasonable numbers, 
		// but I make no guarantees.
		//
		// Retry#   Delay   Total Delay
		// ------   -----   -----------
		//	1		  0.5			0.5
		//	2		  1				1.5
		//	3		  2				3.5
		//	4		  4				7.5
		//	5		  8			   15.5
		//	6		 16			   31.5
		//	7		 32			   63.5
		//	8		 64			  127.5
		//	9		128			  255.5
		//	10		256			  511.5

		sndParam.memoryRetries = 0;
		currentDelay = DurationToAbsolute(-500);		// Start delay at 500us.
		do {
			// Restore the wait record's magic.  QueueBlueAndWait sets the 
			// magic to a bad value on the way out to detect accidental wait 
			// record reuse.  However, in this case we know what we're doing 
			// so we just reset the magic.  We could just call InitStdParam 
			// again, but that would be needlessly inefficient.
			#if MORE_DEBUG
				sndParam.stdParam.waitRecord.magic = kOTMPWaitRecordMagic;
			#endif

			err = QueueBlueAndWait(&sndParam.stdParam.waitRecord, true);
			if (err == kOTOutOfMemoryErr) {
				sndParam.memoryRetries += 1;
				MPLog1(kOTMPRetriesLogID, kOTMPSndMemoryRetry, (void *) sndParam.memoryRetries);

				err = SndRetryDelay(sndParam.memoryRetries, &currentDelay);
				if (err == noErr) {
					err = kOTOutOfMemoryErr;
				}
			}
		} while (err == kOTOutOfMemoryErr && sndParam.memoryRetries < kOTMPSndRetryMax);
		
		// If we completed successfully, return nbytes as the function 
		// result.  Because we're effectively running in sync/blocking 
		// mode, all other results are errors.
		
		if (err == noErr) {
			err = nbytes;
		}
	}

	MPLog1(kOTMPAPILogID, MPLogTagReturn(kOTMPSndMagic), (void *) err);
	return err;
}

enum {
	kOTMPRcvMagic = '>Rcv'
};

struct RcvParam {
	StdParam	stdParam;			// kOTMPRcvMagic
	void *		buf;
	OTByteCount	nbytes;
	OTFlags *	flags;
};
typedef struct RcvParam RcvParam;

static pascal Boolean RcvStdAction(StdParam *thisTask, OSStatus *errPtr)
	// The action callback for OTMPRcv.
{
	OSStatus 	err;
	Boolean  	done;
	OTByteCount bytesReceivedSoFar;
	OTByteCount bytesAvailable;
	RcvParam *	rcvParam;

	assert( ! MPTaskIsPreemptive(kInvalidID) );
	assert( (TaskLevel() & kInDeferredTaskMask) == kInDeferredTaskMask );

	rcvParam = OTGetLinkObject(thisTask, RcvParam, stdParam);
	assert(rcvParam->stdParam.magic == kOTMPRcvMagic);

	// Must be called from within context of notifier. 
	
	assert( !OTEnterNotifier(rcvParam->stdParam.mpEP->otEP) );
	
	bytesReceivedSoFar = 0;
	do {	
		// Initialise done to a bogus Boolean to detect flawed control logic.

		#if MORE_DEBUG
			done = 66;
		#endif

		// Attempt to received the data.
		
		err = OTRcv((EndpointRef)rcvParam->stdParam.mpEP->otEP, rcvParam->buf, rcvParam->nbytes, rcvParam->flags);
		
		// There are 5 possible results of this call.
		//
		// 1. success (err == nbytes)
		// 2. partial success (0 < err < nybytes)
		// 3. no data (kOTNoDataErr)
		// 4. look
		// 5. other error
		
		if (err == rcvParam->nbytes) {
			// Case 1
			bytesReceivedSoFar += err;
			err = noErr;
			done = true;
		} else if ( (err > 0) && (err < rcvParam->nbytes) && (*(rcvParam->flags) == T_MORE) ) {
			// Case 2

			// A note on flags.  We can only try another data receive if the T_MORE 
			// flag is set.  If it isn't set then we should return this chunk of 
			// the data to the client as is, thereby preserve the TSDU boundary.
			
			bytesReceivedSoFar += err;
			err = noErr;
			
			((char *) rcvParam->buf) += bytesReceivedSoFar;
			rcvParam->nbytes -= bytesReceivedSoFar;

			// As an optimisation, if new data has arrived between the OTRcv
			// and here, we loop to get the new data.  Otherwise we return the
			// data that we have.  Note that this optimisation will cause 
			// problems if expedited data is used.  This library isn't really 
			// qualified to handle expedited data.
			
			if ( (OTCountDataBytes((EndpointRef)rcvParam->stdParam.mpEP->otEP, &bytesAvailable) == noErr)
						&& (bytesAvailable >= rcvParam->nbytes) ) {
				done = false;
			} else {
				done = true;
			}
		} else if (err == kOTNoDataErr) {
			// Case 3
			done = true;
		} else {
			// Case 4, 5
			// Return the error back to the MP task.  It will return
			// the error to the caller.
			assert(err != noErr);
			done = true;
		}
		
		// All branches of the above if statement must set the control variable.
		
		assert(done == true          || done == false         );
	} while ( ! done );

	if (err == kOTNoDataErr) {
		*errPtr = noErr;
		return false;
	} else {
		if (err == noErr) {
			err = bytesReceivedSoFar;
		}
		*errPtr = err;
		return true;
	}
}

static pascal void RcvNotifier(OTMPWaitRecord *waitRec, OTEventCode code, OTResult result, void *cookie)
	// The notification callback for OTMPRcv.
	//
	// This is another example, like ConnectNotifier, of a notifier that 
	// can be called with a code different from the code that's expected.
{
	OSStatus	err;
	RcvParam *	rcvParam;
	Boolean		completeIt;
	#pragma unused(result)
	#pragma unused(cookie)

	assert( ! MPTaskIsPreemptive(kInvalidID) );
	assert( (TaskLevel() & kInDeferredTaskMask) == kInDeferredTaskMask );

	assert( (code == T_DATA) || (code == T_DISCONNECT) || (code == T_ORDREL) );
	
	rcvParam = OTGetLinkObject(waitRec, RcvParam, stdParam.waitRecord);
	assert(rcvParam->stdParam.magic == kOTMPRcvMagic);

	assert(waitRec == &rcvParam->stdParam.waitRecord);

	switch (code) {	
		case T_DATA:
			completeIt = RcvStdAction(&rcvParam->stdParam, &err);
			break;
		case T_DISCONNECT:
		case T_ORDREL:
			completeIt = true;
			err = kOTLookErr;
			break;
		default:
			assert(false);
			break;
	}

	if (completeIt) {
		CompleteWaitRecord(waitRec, err);
	} else {
		assert(waitRec->link.fNext == NULL);
		OTAddFirst(&rcvParam->stdParam.mpEP->waitRecords, &waitRec->link);
	}
}

extern pascal OTResult OTMPRcv(	 OTMPEndpointRef        ref,
                                 void *                 buf,
                                 OTByteCount            nbytes,
                                 OTFlags *              flags)
	// OTMP API entry point.
{
	OSStatus 	err;
	RcvParam 	rcvParam;
	
	assert(ValidOTMPProviderRef(ref, false));
	assert(buf != NULL);
	assert(nbytes < 0x01000000);		// arbitrary limit to catch wacky errors
	assert(flags != NULL);
	MPLog4(kOTMPAPILogID, kOTMPRcvMagic, ref, buf, (void *) nbytes, flags);
	
	err = InitStdParam(&rcvParam.stdParam, kOTMPRcvMagic, RcvStdAction, ref, T_DATA, RcvNotifier);
	if (err == noErr) {
		rcvParam.buf 	= buf;
		rcvParam.nbytes = nbytes;
		rcvParam.flags 	= flags;
		err = QueueBlueAndWait(&rcvParam.stdParam.waitRecord, true);
	}

	MPLog2(kOTMPAPILogID, MPLogTagReturn(kOTMPRcvMagic), (void *) err, (void *) *flags);
	return err;
}

/////////////////////////////////////////////////////////////////
#pragma mark **** Listen/Accept *****

enum {
	kOTMPListenMagic = '>Lsn'
};

struct ListenParam {
	StdParam	stdParam;			// kOTMPListenMagic
	TCall * 	call;
};
typedef struct ListenParam ListenParam;

static pascal Boolean ListenStdAction(StdParam *thisTask, OSStatus *errPtr)
	// The action callback for OMPListenn.
{
	OSStatus 		err;
	Boolean  		completeIt;
	ListenParam *	listenParam;

	assert( ! MPTaskIsPreemptive(kInvalidID) );
	assert( (TaskLevel() & kInDeferredTaskMask) == kInDeferredTaskMask );

	listenParam = OTGetLinkObject(thisTask, ListenParam, stdParam);
	assert(listenParam->stdParam.magic == kOTMPListenMagic);

	// Must be called from within context of notifier. 
	
	assert( !OTEnterNotifier(listenParam->stdParam.mpEP->otEP) );

	err = OTListen((EndpointRef)listenParam->stdParam.mpEP->otEP, listenParam->call);
	if (err == kOTNoDataErr) {
		err = noErr;
		completeIt = false;
	} else {
		completeIt = true;
	}
	*errPtr = err;
	return completeIt;
}

static pascal void ListenNotifier(OTMPWaitRecord *waitRec, OTEventCode code, OTResult result, void *cookie)
	// The notification callback for OTMPListen.
	//
	// This is another example, like ConnectNotifier, of a notifier that 
	// can be called with a code different from the code that's expected.
{
	OSStatus		err;
	ListenParam *	listenParam;
	Boolean			completeIt;
	#pragma unused(result)
	#pragma unused(cookie)

	assert( ! MPTaskIsPreemptive(kInvalidID) );
	assert( (TaskLevel() & kInDeferredTaskMask) == kInDeferredTaskMask );

	assert( (code == T_LISTEN) || (code == T_DISCONNECT) );
	
	listenParam = OTGetLinkObject(waitRec, ListenParam, stdParam.waitRecord);
	assert(listenParam->stdParam.magic == kOTMPListenMagic);

	assert(waitRec == &listenParam->stdParam.waitRecord);

	switch (code) {
		case T_LISTEN:
			completeIt = ListenStdAction(&listenParam->stdParam, &err);
			break;
		case T_DISCONNECT:
			completeIt = true;
			err = kOTLookErr;
			break;
		default:
			assert(false);
			break;
	}

	if (completeIt) {
		CompleteWaitRecord(waitRec, err);
	} else {
		assert(waitRec->link.fNext == NULL);
		OTAddFirst(&listenParam->stdParam.mpEP->waitRecords, &waitRec->link);
	}
}

extern pascal OSStatus OTMPListen(OTMPEndpointRef       ref,
                                 TCall *                call)
	// OTMP API entry point.
{
	OSStatus 	err;
	ListenParam listenParam;
	
	assert(ValidOTMPProviderRef(ref, false));
	assert(call != NULL);
	MPLog2(kOTMPAPILogID, kOTMPListenMagic, ref, call);
	
	err = InitStdParam(&listenParam.stdParam, kOTMPListenMagic, ListenStdAction, ref, T_LISTEN, ListenNotifier);
	if (err == noErr) {
		listenParam.call 	= call;
		err = QueueBlueAndWait(&listenParam.stdParam.waitRecord, true);
	}

	MPLog1(kOTMPAPILogID, MPLogTagReturn(kOTMPListenMagic), (void *) err);
	return err;
}

enum {
	kOTMPAcceptMagic = '>Acp'
};

struct AcceptParam {
	StdParam    	stdParam;			// kOTMPAcceptMagic
	OTMPWaitRecord  passConnWaitRec;
	OTMPEndpointRef worker;
	TCall *         call;
	SInt32			waiterCount;
};
typedef struct AcceptParam AcceptParam;

static pascal Boolean AcceptStdAction(StdParam *thisParam, OSStatus *errPtr)
	// The action callback for OTMPAccept.
{
	OTResult 		err;
	AcceptParam *	acceptParam;
	Boolean  		enteredWorker;
	
	assert( ! MPTaskIsPreemptive(kInvalidID) );
	assert( (TaskLevel() & kInDeferredTaskMask) == kInDeferredTaskMask );

	acceptParam = OTGetLinkObject(thisParam, AcceptParam, stdParam);
	assert(acceptParam->stdParam.magic == kOTMPAcceptMagic);

	// If we're accepting on the same endpoint as we're listening, 
	// we don't need to enter the notifier because we're already in it.
	
	enteredWorker = false;
	if (acceptParam->worker != acceptParam->stdParam.mpEP) {
		enteredWorker = OTEnterNotifier(acceptParam->worker->otEP);
		assert(enteredWorker);
		// There's no real guarantee that the above assert will never fire. 
		// However, the fact that we're running from a deferred task 
		// means that we won't have interrupted OT or another, so it's 
		// should not happen on traditional Mac OS implementations.  Rather 
		// than complicate the code by addressing this problem I've decided 
		// to ignore it and see if it ever happens in practice.
	}

	// Things you learn while programming OT...  It turns out that 
	// T_PASSCON is always delivered immediately to the worker endpoint, 
	// regardless of whether the worker is inside its notifier.  So calling 
	// OTEnterNotifier will not defer the T_PASSCON.  So we can't take 
	// our normal approach of:
	//
	// 1. call OTEnterNotifier
	// 2. call the OT routine
	// 3. if there was no error add the wait record to the list
	// 4. call OTLeaveNotifier, which unblocks notifications, including 
	//    the notification which will remove the wait record from the list.
	//
	// This doesn't work because the T_PASSCON is delivered immediately, 
	// so the wait record isn't on the list when the notifier runs.  We work 
	// around this by putting the wait record on the list in advance and then, 
	// if the call fails, removing it by hand.
	//
	// Note that this T_PASSCON reentrancy is bad for the global 
	// synchronisation model.  Specifically, if a T_PASSCON interrupts 
	// some unrelated operation on the worker endpoint (for example, an 
	// OTIoctl), there's a possibility that the non-atomic operations on 
	// the wait record list on the worker might corrupt the list.  Long term, 
	// we have two choices: 1) update the global synchronisation model (OTGate?), 
	// or 2) outlaw async operations on the worker while OTMPAccept is in 
	// progress.  For the moment I'm going to go with option 2 because the cost 
	// of changing the global synchronisation model is very high and it only 
	// affects a relatively small edge case.  If I later find other problems with 
	// the global synchronisation model I may revisit this.  Of course, option 
	// 1 still doesn't fix the problem noted above with OTEnterNotifier.
	
	assert(acceptParam->passConnWaitRec.link.fNext == NULL);
	OTAddFirst(&acceptParam->worker->waitRecords, &acceptParam->passConnWaitRec.link);

	err = OTAccept((EndpointRef)acceptParam->stdParam.mpEP->otEP, (EndpointRef)acceptParam->worker->otEP, acceptParam->call);

	if (err != noErr) {
		Boolean junkBool;
		
		junkBool = OTRemoveLink(&acceptParam->worker->waitRecords, &acceptParam->passConnWaitRec.link);
		assert(junkBool);

		acceptParam->passConnWaitRec.link.fNext = NULL;				// only needed for debugging
	}

	if (enteredWorker) {
		OTLeaveNotifier(acceptParam->worker->otEP);
	}

	*errPtr = err;
	return (err != noErr);
}

static pascal void AcceptNotifier(OTMPWaitRecord *waitRec, OTEventCode code, OTResult result, void *cookie)
	// The notification callback listener endpoint for OTMPAccept.
{
	AcceptParam * acceptParam;
	#pragma unused(code)
	#pragma unused(cookie)

	assert( ! MPTaskIsPreemptive(kInvalidID) );
	assert( (TaskLevel() & kInDeferredTaskMask) == kInDeferredTaskMask );

	assert(code == T_ACCEPTCOMPLETE);
	
	acceptParam = OTGetLinkObject(waitRec, AcceptParam, stdParam.waitRecord);
	assert(acceptParam->stdParam.magic == kOTMPAcceptMagic);
	
	if ( OTAtomicAdd32(-1, &acceptParam->waiterCount) == 0) {
		CompleteWaitRecord(&acceptParam->stdParam.waitRecord, result);
	}
}

static pascal void PassConNotifier(OTMPWaitRecord *waitRec, OTEventCode code, OTResult result, void *cookie)
	// The notification callback for worker endpoint for OTMPAccept.
{
	AcceptParam * acceptParam;
	#pragma unused(code)
	#pragma unused(cookie)

	assert( ! MPTaskIsPreemptive(kInvalidID) );
	assert( (TaskLevel() & kInDeferredTaskMask) == kInDeferredTaskMask );

	assert(code == T_PASSCON);

	acceptParam = OTGetLinkObject(waitRec, AcceptParam, passConnWaitRec);
	assert(acceptParam->stdParam.magic == kOTMPAcceptMagic);

	if ( OTAtomicAdd32(-1, &acceptParam->waiterCount) == 0) {
		CompleteWaitRecord(&acceptParam->stdParam.waitRecord, result);
	}
}

extern pascal OSStatus OTMPAccept(OTMPEndpointRef       listener,
                                 OTMPEndpointRef        worker,
                                 TCall *                call)
	// OTMP API entry point.
	//
	// As always, Accept is an ugly special case operation.  Specifically, 
	// we have to coordinate behaviour between two endpoints.  This yields 
	// a number of special cases.
	//
	// o We have two wait records inside AcceptParam, one embedded within 
	//   the stdParam and the other called passConnWaitRec.  The first is 
	//   used to kick off the accept operation and to wait for the 
	//   T_ACCEPTCOMPLETE event.  The second isn't used to kick off any 
	//   operation (the Blue action within the wait record is unusued --
	//   a special case in itself) but is used to wait for the T_PASSCON 
	//   event.
	// 
	// o A counter inside the AcceptParam counts down from 2 each time 
	//   one of the notifier callbacks is called.  When it hits zero 
	//   the accept operation is complete and we unblock the MP task.
	// 
	// o AcceptStdAction has two gotchas, one associated with entering 
	//   the worker endpoint and the other with the immediate nature 
	//   of T_PASSCON.  See the comments in that routine for more details.
{
	OSStatus	err;
	AcceptParam acceptParam;

	assert(ValidOTMPProviderRef(listener, false));
	assert(ValidOTMPProviderRef(worker, false));
	assert(call != NULL);
	MPLog3(kOTMPAPILogID, kOTMPAcceptMagic, listener, worker, call);

	err = InitStdParam(&acceptParam.stdParam, kOTMPAcceptMagic, AcceptStdAction, listener, T_ACCEPTCOMPLETE, AcceptNotifier);
	if (err == noErr) {
		err = InitWaitRecord(&acceptParam.passConnWaitRec, NULL, T_PASSCON, PassConNotifier);
	}
	if (err == noErr) {
		acceptParam.worker = worker;
		acceptParam.call = call;
		acceptParam.waiterCount = 2;

		err = QueueBlueAndWait(&acceptParam.stdParam.waitRecord, true);
	}
	
	MPLog1(kOTMPAPILogID, MPLogTagReturn(kOTMPAcceptMagic), (void *) err);
	return err;
}

/////////////////////////////////////////////////////////////////
#pragma mark **** Orderly Disconnect *****

enum {
	kOTMPSndOrderlyDisconnectMagic = '>Sor'
};

struct SndOrderlyDisconnectParam {
	OSType			magic;				// must be kOTMPSndOrderlyDisconnectMagic
	OTMPWaitRecord  waitRecord;
	OTMPEndpointRef mpEP;
};
typedef struct SndOrderlyDisconnectParam SndOrderlyDisconnectParam;

static pascal void SndOrderlyDisconnectAction(MoreBlueAction *thisTask)
	// The action callback for OTMPSndOrderlyDisconnect.
{
	OTResult 					err;
	SndOrderlyDisconnectParam *	sndOrderlyDisconnectParam;
	
	assert( ! MPTaskIsPreemptive(kInvalidID) );
	assert( (TaskLevel() & kInDeferredTaskMask) == kInDeferredTaskMask );

	sndOrderlyDisconnectParam = OTGetLinkObject(thisTask, SndOrderlyDisconnectParam, waitRecord.blue);
	assert(sndOrderlyDisconnectParam->magic == kOTMPSndOrderlyDisconnectMagic);

	err = sndOrderlyDisconnectParam->mpEP->cancelErr;
	if (err == noErr) {
		err = OTSndOrderlyDisconnect((EndpointRef)sndOrderlyDisconnectParam->mpEP->otEP);
	}

	CompleteWaitRecord(&sndOrderlyDisconnectParam->waitRecord, err);
}

extern pascal OSStatus OTMPSndOrderlyDisconnect(OTMPEndpointRef ref)
	// OTMP API entry point.
{
	OSStatus 					err;
	SndOrderlyDisconnectParam 	sndOrderlyDisconnectParam;
	
	assert(ValidOTMPProviderRef(ref, false));
	MPLog1(kOTMPAPILogID, kOTMPSndOrderlyDisconnectMagic, ref);
	
	err = InitWaitRecord(&sndOrderlyDisconnectParam.waitRecord, SndOrderlyDisconnectAction, 0, NULL);
	if (err == noErr) {
		sndOrderlyDisconnectParam.magic = kOTMPSndOrderlyDisconnectMagic;
		sndOrderlyDisconnectParam.mpEP  = ref;
		err = QueueBlueAndWait(&sndOrderlyDisconnectParam.waitRecord, false);
	}

	MPLog1(kOTMPAPILogID, MPLogTagReturn(kOTMPSndOrderlyDisconnectMagic), (void *) err);
	return err;
}

enum {
	kOTMPRcvOrderlyDisconnectMagic = '>Ror'
};

struct RcvOrderlyDisconnectParam {
	OSType			magic;				// must be kOTMPRcvOrderlyDisconnectMagic
	OTMPWaitRecord  waitRecord;
	OTMPEndpointRef mpEP;
};
typedef struct RcvOrderlyDisconnectParam RcvOrderlyDisconnectParam;

static pascal void RcvOrderlyDisconnectAction(MoreBlueAction *thisTask)
	// The action callback for OTMPRcvOrderlyDisconnect.
{
	OTResult 					err;
	RcvOrderlyDisconnectParam *	rcvOrderlyDisconnectParam;
	
	assert( ! MPTaskIsPreemptive(kInvalidID) );
	assert( (TaskLevel() & kInDeferredTaskMask) == kInDeferredTaskMask );

	rcvOrderlyDisconnectParam = OTGetLinkObject(thisTask, RcvOrderlyDisconnectParam, waitRecord.blue);
	assert(rcvOrderlyDisconnectParam->magic == kOTMPRcvOrderlyDisconnectMagic);

	err = rcvOrderlyDisconnectParam->mpEP->cancelErr;
	if (err == noErr) {
		err = OTRcvOrderlyDisconnect((EndpointRef)rcvOrderlyDisconnectParam->mpEP->otEP);
	}

	CompleteWaitRecord(&rcvOrderlyDisconnectParam->waitRecord, err);
}

extern pascal OSStatus OTMPRcvOrderlyDisconnect(OTMPEndpointRef ref)
	// OTMP API entry point.
{
	OSStatus 					err;
	RcvOrderlyDisconnectParam 	rcvOrderlyDisconnectParam;
	
	assert(ValidOTMPProviderRef(ref, false));
	MPLog1(kOTMPAPILogID, kOTMPRcvOrderlyDisconnectMagic, ref);
	
	err = InitWaitRecord(&rcvOrderlyDisconnectParam.waitRecord, RcvOrderlyDisconnectAction, 0, NULL);
	if (err == noErr) {
		rcvOrderlyDisconnectParam.magic = kOTMPRcvOrderlyDisconnectMagic;
		rcvOrderlyDisconnectParam.mpEP  = ref;
		err = QueueBlueAndWait(&rcvOrderlyDisconnectParam.waitRecord, false);
	}

	MPLog1(kOTMPAPILogID, MPLogTagReturn(kOTMPRcvOrderlyDisconnectMagic), (void *) err);
	return err;
}

/////////////////////////////////////////////////////////////////
#pragma mark **** Unit Data *****

enum {
	kOTMPSndUDataMagic       = '>SnU',
	kOTMPSndUDataMemoryRetry = '*SnU'
};

struct SndUDataParam {
	StdParam	stdParam;			// kOTMPSndUDataMagic
	TUnitData * udata;
	UInt32		memoryRetries;
};
typedef struct SndUDataParam SndUDataParam;

static pascal Boolean SndUDataStdAction(StdParam *thisParam, OSStatus *errPtr)
	// The action callback for OTMPSndUData.
{
	OSStatus 		err;
	SndUDataParam *	sndUDataParam;
	
	assert( ! MPTaskIsPreemptive(kInvalidID) );
	assert( (TaskLevel() & kInDeferredTaskMask) == kInDeferredTaskMask );

	sndUDataParam = OTGetLinkObject(thisParam, SndUDataParam, stdParam);
	assert(sndUDataParam->stdParam.magic == kOTMPSndUDataMagic);

	// Must be called from within context of notifier. 
	
	assert( !OTEnterNotifier(sndUDataParam->stdParam.mpEP->otEP) );
	
	err = OTSndUData((EndpointRef)sndUDataParam->stdParam.mpEP->otEP, sndUDataParam->udata);
	
	if (err == kOTFlowErr) {
		#if MORE_DEBUG
			(void) OTAtomicAdd32(1, &gSndFlowCounter);
		#endif
		*errPtr = noErr;
		return false;
	} else {
		*errPtr = err;
		return true;
	}
}

static pascal void SndUDataNotifier(OTMPWaitRecord *waitRec, OTEventCode code, OTResult result, void *cookie)
	// The notification callback for OTMPSndUData.
	//
	// This is another example, like ConnectNotifier, of a notifier that 
	// can be called with a code different from the code that's expected.
{
	OSStatus 		err;
	SndUDataParam *	sndUDataParam;
	Boolean 		completeIt;
	#pragma unused(result)
	#pragma unused(cookie)

	assert( ! MPTaskIsPreemptive(kInvalidID) );
	assert( (TaskLevel() & kInDeferredTaskMask) == kInDeferredTaskMask );

	assert( (code == T_GODATA) || (code == T_UDERR) );
	
	sndUDataParam = OTGetLinkObject(waitRec, SndUDataParam, stdParam.waitRecord);
	assert(sndUDataParam->stdParam.magic == kOTMPSndUDataMagic);

	assert(waitRec == &sndUDataParam->stdParam.waitRecord);

	switch (code) {	
		case T_GODATA:
			completeIt = SndUDataStdAction(&sndUDataParam->stdParam, &err);
			break;
		case T_UDERR:
			completeIt = true;
			err = kOTLookErr;
			break;
		default:
			assert(false);
			break;
	}

	if (completeIt) {
		CompleteWaitRecord(waitRec, err);
	} else {
		assert(waitRec->link.fNext == NULL);
		OTAddFirst(&sndUDataParam->stdParam.mpEP->waitRecords, &waitRec->link);
	}
}

extern pascal OSStatus OTMPSndUData(
								 OTMPEndpointRef        ref,
                                 TUnitData *            udata)
	// OTMP API entry point.
{
	OSStatus 		err;
	SndUDataParam 	sndUDataParam;
	AbsoluteTime	currentDelay;
	
	assert(ValidOTMPProviderRef(ref, false));
	assert(udata != NULL);
	MPLog2(kOTMPAPILogID, kOTMPSndUDataMagic, ref, udata);
	
	err = InitStdParam(&sndUDataParam.stdParam, kOTMPSndUDataMagic, SndUDataStdAction, ref, T_GODATA, SndUDataNotifier);
	if (err == noErr) {
		sndUDataParam.udata = udata;
	
		// See OTSnd for description of why this "do" loop is necessary.
		
		sndUDataParam.memoryRetries = 0;		
		currentDelay = DurationToAbsolute(-500);		// Start delay at 500us.
		do {
			#if MORE_DEBUG
				// Restore the wait record's magic.  See OTMPSnd for why.
				sndUDataParam.stdParam.waitRecord.magic = kOTMPWaitRecordMagic;
			#endif

			err = QueueBlueAndWait(&sndUDataParam.stdParam.waitRecord, true);
			if (err == kOTOutOfMemoryErr) {
				sndUDataParam.memoryRetries += 1;
				MPLog1(kOTMPRetriesLogID, kOTMPSndUDataMemoryRetry, (void *) sndUDataParam.memoryRetries);

				err = SndRetryDelay(sndUDataParam.memoryRetries, &currentDelay);
				if (err == noErr) {
					err = kOTOutOfMemoryErr;
				}
			}
		} while (err == kOTOutOfMemoryErr && sndUDataParam.memoryRetries < kOTMPSndRetryMax);
	}

	MPLog1(kOTMPAPILogID, MPLogTagReturn(kOTMPSndUDataMagic), (void *) err);
	return err;
}

enum {
	kOTMPRcvUDataMagic = '>RcU'
};

struct RcvUDataParam {
	StdParam	stdParam;			// kOTMPRcvUDataMagic
	TUnitData *	udata;
	OTFlags *	flags;
};
typedef struct RcvUDataParam RcvUDataParam;

static pascal Boolean RcvUDataStdAction(StdParam *thisTask, OSStatus *errPtr)
	// The action callback for OTMPRcvUData.
{
	OSStatus 		err;
	RcvUDataParam *	rcvUDataParam;

	assert( ! MPTaskIsPreemptive(kInvalidID) );
	assert( (TaskLevel() & kInDeferredTaskMask) == kInDeferredTaskMask );

	rcvUDataParam = OTGetLinkObject(thisTask, RcvUDataParam, stdParam);
	assert(rcvUDataParam->stdParam.magic == kOTMPRcvUDataMagic);

	// Must be called from within context of notifier. 
	
	assert( !OTEnterNotifier(rcvUDataParam->stdParam.mpEP->otEP) );
	
	err = OTRcvUData((EndpointRef)rcvUDataParam->stdParam.mpEP->otEP, rcvUDataParam->udata, rcvUDataParam->flags);
	
	if (err == kOTNoDataErr) {
		*errPtr = noErr;
		return false;
	} else {
		*errPtr = err;
		return true;
	}
}

static pascal void RcvUDataNotifier(OTMPWaitRecord *waitRec, OTEventCode code, OTResult result, void *cookie)
	// The notification callback for OTMPRcvUData.
	//
	// This is another example, like ConnectNotifier, of a notifier that 
	// can be called with a code different from the code that's expected.
{
	OSStatus		err;
	RcvUDataParam *	rcvUDataParam;
	Boolean			completeIt;
	#pragma unused(result)
	#pragma unused(cookie)

	assert( ! MPTaskIsPreemptive(kInvalidID) );
	assert( (TaskLevel() & kInDeferredTaskMask) == kInDeferredTaskMask );

	assert( (code == T_DATA) || (code == T_UDERR) );
	
	rcvUDataParam = OTGetLinkObject(waitRec, RcvUDataParam, stdParam.waitRecord);
	assert(rcvUDataParam->stdParam.magic == kOTMPRcvUDataMagic);

	assert(waitRec == &rcvUDataParam->stdParam.waitRecord);

	switch (code) {	
		case T_DATA:
			completeIt = RcvUDataStdAction(&rcvUDataParam->stdParam, &err);
			break;
		case T_UDERR:
			completeIt = true;
			err = kOTLookErr;
			break;
		default:
			assert(false);
			break;
	}

	if (completeIt) {
		CompleteWaitRecord(waitRec, err);
	} else {
		assert(waitRec->link.fNext == NULL);
		OTAddFirst(&rcvUDataParam->stdParam.mpEP->waitRecords, &waitRec->link);
	}
}

extern pascal OSStatus OTMPRcvUData(
								 OTMPEndpointRef        ref,
                                 TUnitData *            udata,
                                 OTFlags *              flags)
	// OTMP API entry point.
{
	OSStatus 			err;
	RcvUDataParam 	rcvUDataParam;
	
	assert(ValidOTMPProviderRef(ref, false));
	assert(udata != NULL);
	assert(flags != NULL);
	MPLog3(kOTMPAPILogID, kOTMPRcvUDataMagic, ref, udata, flags);
	
	err = InitStdParam(&rcvUDataParam.stdParam, kOTMPRcvUDataMagic, RcvUDataStdAction, ref, T_DATA, RcvUDataNotifier);
	if (err == noErr) {
		rcvUDataParam.udata 	= udata;
		rcvUDataParam.flags 	= flags;
		err = QueueBlueAndWait(&rcvUDataParam.stdParam.waitRecord, true);
	}

	MPLog2(kOTMPAPILogID, MPLogTagReturn(kOTMPRcvUDataMagic), (void *) err, (void *) *flags);
	return err;
}

enum {
	kOTMPRcvUDErrMagic = '>RuE'
};

struct RcvUDErrParam {
	OSType			magic;				// must be kOTMPRcvUDErrMagic
	OTMPWaitRecord  waitRecord;
	OTMPEndpointRef mpEP;
	TUDErr *        uderr;
};
typedef struct RcvUDErrParam RcvUDErrParam;

static pascal void RcvUDErrAction(MoreBlueAction *thisTask)
	// The action callback for OTMPRcvUDErr.
{
	OTResult 		err;
	RcvUDErrParam *	rcvUDErrParam;
	
	assert( ! MPTaskIsPreemptive(kInvalidID) );
	assert( (TaskLevel() & kInDeferredTaskMask) == kInDeferredTaskMask );

	rcvUDErrParam = OTGetLinkObject(thisTask, RcvUDErrParam, waitRecord.blue);
	assert(rcvUDErrParam->magic == kOTMPRcvUDErrMagic);

	err = rcvUDErrParam->mpEP->cancelErr;
	if (err == noErr) {
		err = OTRcvUDErr((EndpointRef)rcvUDErrParam->mpEP->otEP, rcvUDErrParam->uderr);
	}

	CompleteWaitRecord(&rcvUDErrParam->waitRecord, err);
}

extern pascal OSStatus OTMPRcvUDErr(
								 OTMPEndpointRef        ref,
                                 TUDErr *               uderr)
	// OTMP API entry point.
{
	OSStatus 		err;
	RcvUDErrParam 	rcvUDErrParam;
	
	assert(ValidOTMPProviderRef(ref, false));
	MPLog2(kOTMPAPILogID, kOTMPRcvUDErrMagic, ref, uderr);
	
	err = InitWaitRecord(&rcvUDErrParam.waitRecord, RcvUDErrAction, 0, NULL);
	if (err == noErr) {
		rcvUDErrParam.magic = kOTMPRcvUDErrMagic;
		rcvUDErrParam.mpEP  = ref;
		rcvUDErrParam.uderr = uderr;
		err = QueueBlueAndWait(&rcvUDErrParam.waitRecord, false);
	}

	MPLog1(kOTMPAPILogID, MPLogTagReturn(kOTMPRcvUDErrMagic), (void *) err);
	return err;
}

/////////////////////////////////////////////////////////////////
#pragma mark **** Raw Get/Put *****

#if !TARGET_API_MAC_CARBON

enum {
	kOTMPPutMessageMagic       = '>Put',
	kOTMPPutMessageMemoryRetry = '*Put'
};

struct PutMessageParam {
	StdParam		stdParam;			// kOTMPPutMessageMagic
	const strbuf *	ctlbuf;
	const strbuf *  databuf;
	OTFlags         flags;
	UInt32			memoryRetries;
};
typedef struct PutMessageParam PutMessageParam;

static pascal Boolean PutMessageStdAction(StdParam *thisParam, OSStatus *errPtr)
	// The action callback for OTMPPutMessage.
{
	OSStatus 			err;
	PutMessageParam *	putMessageParam;
	
	assert( ! MPTaskIsPreemptive(kInvalidID) );
	assert( (TaskLevel() & kInDeferredTaskMask) == kInDeferredTaskMask );

	putMessageParam = OTGetLinkObject(thisParam, PutMessageParam, stdParam);
	assert(putMessageParam->stdParam.magic == kOTMPPutMessageMagic);

	// Must be called from within context of notifier. 
	
	assert( !OTEnterNotifier(putMessageParam->stdParam.mpEP->otEP) );
	
	err = OTPutMessage(putMessageParam->stdParam.mpEP->otEP, putMessageParam->ctlbuf, putMessageParam->databuf, putMessageParam->flags);
	
	if (err == kEAGAINErr) {
		#if MORE_DEBUG
			(void) OTAtomicAdd32(1, &gSndFlowCounter);
		#endif
		*errPtr = noErr;
		return false;
	} else {
		*errPtr = err;
		return true;
	}
}

static pascal void PutMessageNotifier(OTMPWaitRecord *waitRec, OTEventCode code, OTResult result, void *cookie)
	// The notification callback for OTMPPutMessage.
{
	OSStatus 			err;
	PutMessageParam *	putMessageParam;
	Boolean 			completeIt;
	#pragma unused(result)
	#pragma unused(cookie)

	assert( ! MPTaskIsPreemptive(kInvalidID) );
	assert( (TaskLevel() & kInDeferredTaskMask) == kInDeferredTaskMask );

	assert( code == T_GODATA );
	
	putMessageParam = OTGetLinkObject(waitRec, PutMessageParam, stdParam.waitRecord);
	assert(putMessageParam->stdParam.magic == kOTMPPutMessageMagic);

	assert(waitRec == &putMessageParam->stdParam.waitRecord);

	completeIt = PutMessageStdAction(&putMessageParam->stdParam, &err);

	if (completeIt) {
		CompleteWaitRecord(waitRec, err);
	} else {
		assert(waitRec->link.fNext == NULL);
		OTAddFirst(&putMessageParam->stdParam.mpEP->waitRecords, &waitRec->link);
	}
}

extern pascal OSStatus OTMPPutMessage(
                                 OTMPProviderRef        ref,
                                 const strbuf *         ctlbuf,
                                 const strbuf *         databuf,
                                 OTFlags                flags)
	// OTMP API entry point.
{
	OSStatus 		err;
	PutMessageParam 	putMessageParam;
	AbsoluteTime	currentDelay;
	
	// IMPORTANT:
	// This routine is a preliminary implementation and has 
	// not yet been tested properly.  Specifically, I have not 
	// tested the flow control execution path.  Use it at your 
	// own risk.
	
	assert(false);
	
	assert(ValidOTMPProviderRef(ref, false));
	assert( (flags == 0) || (flags == RS_HIPRI) );
	MPLog4(kOTMPAPILogID, kOTMPPutMessageMagic, ref, ctlbuf, databuf, (void *) flags);
	
	err = InitStdParam(&putMessageParam.stdParam, kOTMPPutMessageMagic, PutMessageStdAction, ref, T_GODATA, PutMessageNotifier);
	if (err == noErr) {
		putMessageParam.ctlbuf  = ctlbuf;
		putMessageParam.databuf = databuf;
		putMessageParam.flags   = flags;
	
		// See OTSnd for description of why this "do" loop is necessary.
		
		putMessageParam.memoryRetries = 0;		
		currentDelay = DurationToAbsolute(-500);		// Start delay at 500us.
		do {
			#if MORE_DEBUG
				// Restore the wait record's magic.  See OTMPSnd for why.
				putMessageParam.stdParam.waitRecord.magic = kOTMPWaitRecordMagic;
			#endif

			err = QueueBlueAndWait(&putMessageParam.stdParam.waitRecord, true);
			if (err == kOTOutOfMemoryErr) {
				putMessageParam.memoryRetries += 1;
				MPLog1(kOTMPRetriesLogID, kOTMPPutMessageMemoryRetry, (void *) putMessageParam.memoryRetries);

				err = SndRetryDelay(putMessageParam.memoryRetries, &currentDelay);
				if (err == noErr) {
					err = kOTOutOfMemoryErr;
				}
			}
		} while (err == kOTOutOfMemoryErr && putMessageParam.memoryRetries < kOTMPSndRetryMax);
	}

	MPLog1(kOTMPAPILogID, MPLogTagReturn(kOTMPPutMessageMagic), (void *) err);
	return err;
}

enum {
	kOTMPGetMessageMagic = '>Get'
};

struct GetMessageParam {
	StdParam	stdParam;			// kOTMPGetMessageMagic
	strbuf *	ctlbuf;
	strbuf *	databuf;
	OTFlags *	flagsPtr;
};
typedef struct GetMessageParam GetMessageParam;

static pascal Boolean GetMessageStdAction(StdParam *thisTask, OSStatus *errPtr)
	// The action callback for OTMPGetMessage.
{
	OSStatus 			err;
	GetMessageParam *	getMessageParam;

	assert( ! MPTaskIsPreemptive(kInvalidID) );
	assert( (TaskLevel() & kInDeferredTaskMask) == kInDeferredTaskMask );

	getMessageParam = OTGetLinkObject(thisTask, GetMessageParam, stdParam);
	assert(getMessageParam->stdParam.magic == kOTMPGetMessageMagic);

	// Must be called from within context of notifier. 
	
	assert( !OTEnterNotifier(getMessageParam->stdParam.mpEP->otEP) );
	
	err = OTGetMessage(getMessageParam->stdParam.mpEP->otEP, getMessageParam->ctlbuf, getMessageParam->databuf, getMessageParam->flagsPtr);
	
	if (err == kEAGAINErr) {
		*errPtr = noErr;
		return false;
	} else {
		*errPtr = err;
		return true;
	}
}

static pascal void GetMessageNotifier(OTMPWaitRecord *waitRec, OTEventCode code, OTResult result, void *cookie)
	// The notification callback for OTMPGetMessage.
{
	OSStatus			err;
	GetMessageParam *	getMessageParam;
	Boolean				completeIt;
	#pragma unused(result)
	#pragma unused(cookie)

	assert( ! MPTaskIsPreemptive(kInvalidID) );
	assert( (TaskLevel() & kInDeferredTaskMask) == kInDeferredTaskMask );

	assert( code == kGetmsgEvent );
	
	getMessageParam = OTGetLinkObject(waitRec, GetMessageParam, stdParam.waitRecord);
	assert(getMessageParam->stdParam.magic == kOTMPGetMessageMagic);

	assert(waitRec == &getMessageParam->stdParam.waitRecord);

	// Are we actually in the notifier at this point???
	//
	// I don't think we are, so I added the following test code just to be 
	// sure.  If it fires, I'll have to reexamine the issue 
	// (which will probably be really tricky).
	
	assert( !OTEnterNotifier(getMessageParam->stdParam.mpEP->otEP) );

	completeIt = GetMessageStdAction(&getMessageParam->stdParam, &err);

	if (completeIt) {
		CompleteWaitRecord(waitRec, err);
	} else {
		assert(waitRec->link.fNext == NULL);
		OTAddFirst(&getMessageParam->stdParam.mpEP->waitRecords, &waitRec->link);
	}
}

extern pascal OTResult OTMPGetMessage(
								 OTMPProviderRef        ref,
                                 strbuf *               ctlbuf,
                                 strbuf *               databuf,
                                 OTFlags *              flagsPtr)
	// OTMP API entry point.
{
	OSStatus 			err;
	GetMessageParam 	getMessageParam;
	
	// IMPORTANT:
	// This routine is a preliminary implementation and has 
	// not yet been tested properly.  Specifically, I have not 
	// tested the flow control execution path.  Use it at your 
	// own risk.
	
	assert(false);

	assert(ValidOTMPProviderRef(ref, false));
	assert(flagsPtr != NULL);
	MPLog4(kOTMPAPILogID, kOTMPGetMessageMagic, ref, ctlbuf, databuf, flagsPtr);
	
	err = InitStdParam(&getMessageParam.stdParam, kOTMPGetMessageMagic, GetMessageStdAction, ref, T_DATA, GetMessageNotifier);
	if (err == noErr) {
		getMessageParam.ctlbuf 		= ctlbuf;
		getMessageParam.databuf 	= databuf;
		getMessageParam.flagsPtr	= flagsPtr;
		err = QueueBlueAndWait(&getMessageParam.stdParam.waitRecord, true);
	}

	MPLog2(kOTMPAPILogID, MPLogTagReturn(kOTMPGetMessageMagic), (void *) err, (void *) *flagsPtr);
	return err;
}

#endif

/////////////////////////////////////////////////////////////////
#pragma mark **** Init/Term *****

static SInt32 gOTInitedCount = 0;

extern pascal OSStatus InitOpenTransportMPInContext(
                                 OTInitializationFlags  flags,
                                 OTClientContextPtr *	clientContext)
	// See comment in header file.
{
	OSStatus err;
	
	assert( ! MPTaskIsPreemptive(kInvalidID) );
	assert( TaskLevel() == 0 );

	err = noErr;
	assert(gOTInitedCount >= 0);
	gOTInitedCount += 1;
	if (gOTInitedCount == 1) {
		InitOTCanLoadLibrariesProc();

		if (gSystemVersion == 0) {
			err = Gestalt(gestaltSystemVersion, (SInt32 *) &gSystemVersion);
		}
		
		if (err == noErr) {
			err = InitMoreBlueActions();
		}

		if (err == noErr) {
			err = MPAllocateTaskStorageIndex(&gOTMPTaskStorageIndex);
		}

		// Set up various UPPs.  We can always allocate these in the 
		// current zone because there's never a case where a 
		// RoutineDescriptor is allocated in the system heap.
		// Look at the four cases in turn.
		//
		// o Non-Carbon, Application -- UPP allocators just return parameter.
		//
		// o Non-Carbon, Extension -- UPP allocators just return parameter.
		// 
		// o Carbon, Application -- UPP allocators may allocate memory but 
		//   allocating in the application heap is fine.
		//
		// o Carbon, Extension --  UPP allocators may allocate memory but 
		//   allocating in the application heap is fine.  You can't use 
		//   Carbon for building real system extensions.  Any Carbon 
		//   shared library must be loaded in the context of a host 
		//   Carbon application.
		
		assert(gQueuingBlueNotifierUPP  == NULL);
		assert(gWaitRecordSearchProcUPP == NULL);
		assert(gTrueBlueNotifierUPP     == NULL);
		assert(gNotifierToInstall       == NULL);
		if (err == noErr) {
			gLocalDeferredTaskUPP    = NewDeferredTaskUPP(LocalDeferredTask);
			gTrueBlueNotifierUPP 	 = NewOTNotifyUPP(TrueBlueNotifier);
			gWaitRecordSearchProcUPP = NewOTListSearchUPP(WaitRecordSearchProc);
			if (gLocalDeferredTaskUPP == NULL || gTrueBlueNotifierUPP == NULL || gWaitRecordSearchProcUPP == NULL) {
				err = memFullErr;
			}
			#if MORE_DEBUG
				gQueuingBlueNotifierUPP  = NewOTNotifyUPP(QueuingBlueNotifier);
				if (gQueuingBlueNotifierUPP == NULL) {
					err = memFullErr;
				}
			#endif
		}
		if (err == noErr) {
			gNotifierToInstall = gTrueBlueNotifierUPP;
		}
	}
	if (err == noErr) {
		err = InitOpenTransportInContext(flags, clientContext);
	}

	// Clean up after failure.
	
	if (err != noErr) {
		// InitOpenTransportInContext can't have run successfully, 
		// so there's no need to call CloseOpenTransportInContext for 
		// this particular connection.  So, we call 
		// CloseMPOpenTransportInContext only if we're the first client 
		// to start up, and pass it -1 to indicate that there's no 
		// OT context to shut down.  In that case it simply shuts down 
		// all the UPPs etc.
		if (gOTInitedCount == 1) {
			CloseOpenTransportMPInContext( (OTClientContextPtr) -1 );
		}
	}
	
	return err;
}

extern pascal void CloseOpenTransportMPInContext(OTClientContextPtr clientContext)
	// See comment in header file.
{
	OSStatus junk;
	
	assert( ! MPTaskIsPreemptive(kInvalidID) );
	assert( TaskLevel() == 0 );

	// Close the client's connection to OT, unless we're handling 
	// the special case of InitOpenTransportMPInContext failing (see above).
	
	if (clientContext != (OTClientContextPtr) -1) {
		CloseOpenTransportInContext(clientContext);
	}

	gOTInitedCount -= 1;
	assert(gOTInitedCount >= 0);
	if (gOTInitedCount == 0) {
		// I don't track and clean up all of the OTMPProviderRefs because there's
		// not much point.  The critical data, the OT endpoints, will be shut down
		// by CloseOpenTransport.  The only remaining stuff is the memory used by
		// the OTMPEndpoints, which isn't a huge amount.  However, it's worth noting
		// the failure to close all your endpoints in the debug build, so to help
		// flush out these errors.  Plus, it's one way we deviate from the OT API.
		
		assert(gOTMPProviderCount == 0);
		
		if (gOTMPTaskStorageIndex != 0) {
			junk = MPDeallocateTaskStorageIndex(gOTMPTaskStorageIndex);
			assert(junk == noErr);
			gOTMPTaskStorageIndex = 0;
		}

		if (gTrueBlueNotifierUPP != NULL) {
			DisposeOTNotifyUPP(gTrueBlueNotifierUPP);
			gTrueBlueNotifierUPP = NULL;
		}
		#if MORE_DEBUG
			if (gQueuingBlueNotifierUPP != NULL) {
				DisposeOTNotifyUPP(gQueuingBlueNotifierUPP);
				gQueuingBlueNotifierUPP = NULL;
			}
		#endif
		gNotifierToInstall = NULL;
		if (gWaitRecordSearchProcUPP != NULL) {
			DisposeOTListSearchUPP(gWaitRecordSearchProcUPP);
			gWaitRecordSearchProcUPP = NULL;
		}
		if (gLocalDeferredTaskUPP != NULL) {
			DisposeDeferredTaskUPP(gLocalDeferredTaskUPP);
			gLocalDeferredTaskUPP = NULL;
		}

		assert(gOTMPPreparedTaskCount == 0);
		
		TermMoreBlueActions();
	}
}

/////////////////////////////////////////////////////////////////
#pragma mark ----- OTMPX -----

extern pascal OSStatus InitOpenTransportMPXInContext(
                                 OTInitializationFlags  flags,
                                 OTClientContextPtr *   clientContext)
	// OTMPX API entry point.
{
	OSStatus err;
	
	err = noErr;
	if (gSystemVersion == 0) {
		err = Gestalt(gestaltSystemVersion, (SInt32 *) &gSystemVersion);
	}
	if (err == noErr) {
		if (gSystemVersion >= kMacOSXSystemVersion) {
			err = InitOpenTransportInContext(flags, clientContext);
		} else {
			err = InitOpenTransportMPInContext(flags, clientContext);
		}
	}
	return err;
}

extern pascal void CloseOpenTransportMPXInContext(OTClientContextPtr clientContext)
	// OTMPX API entry point.
{
	if (gSystemVersion >= kMacOSXSystemVersion) {
		CloseOpenTransportInContext(clientContext);
	} else {
		CloseOpenTransportMPInContext(clientContext);
	}
}

extern pascal OSStatus OTMPXPrepareThisTask(void)
	// OTMPX API entry point.
{
	if (gSystemVersion >= kMacOSXSystemVersion) {
		return noErr;
	} else {
		return OTMPPrepareThisTask();
	}
}

extern pascal void OTMPXUnprepareThisTask(void)
	// OTMPX API entry point.
{
	if (gSystemVersion >= kMacOSXSystemVersion) {
		// do nothing
	} else {
		OTMPUnprepareThisTask();
	}
}

extern pascal OTMPEndpointRef
OTMPXOpenEndpointQInContext     (const char *           cfig,
                                 OTOpenFlags            oflag,
                                 TEndpointInfo *        info,
                                 OSStatus *             errPtr,
                                 OTClientContextPtr     clientContext)
	// OTMPX API entry point.
{
	if (gSystemVersion >= kMacOSXSystemVersion) {
		EndpointRef ep;
		OSStatus    junk;
		
		ep = OTOpenEndpointInContext(OTCreateConfiguration(cfig), oflag, info, errPtr, clientContext);
		
		// By default providers do not block.  Our model expects blocking 
		// providers, so we set the endpoint to blocking before we return it.
		
		if (ep != kOTInvalidEndpointRef) {
			junk = OTSetBlocking(ep);
			assert(junk == noErr);
		}
		return (OTMPEndpointRef)ep;
	} else {
		return OTMPOpenEndpointQInContext(cfig, oflag, info, errPtr, clientContext);
	}
}
	
extern pascal OSStatus
OTMPXCloseProvider              (OTMPProviderRef        ref)
	// OTMPX API entry point.
{
	if (gSystemVersion >= kMacOSXSystemVersion) {
		return OTCloseProvider((ProviderRef) ref);
	} else {
		return OTMPCloseProvider(ref);
	}
}

extern pascal SInt32
OTMPXIoctl                      (OTMPProviderRef        ref,
                                 UInt32                 cmd,
                                 void *                 data)
	// OTMPX API entry point.
{
	if (gSystemVersion >= kMacOSXSystemVersion) {
		return OTIoctl((ProviderRef) ref, cmd, data);
	} else {
		return OTMPIoctl(ref, cmd, data);
	}
}

extern pascal OSStatus
OTMPXCancelSynchronousCalls     (OTMPProviderRef        ref,
                                 OSStatus               errParam)
	// OTMPX API entry point.
{
	if (gSystemVersion >= kMacOSXSystemVersion) {
		return OTCancelSynchronousCalls((ProviderRef) ref, errParam);
	} else {
		return OTMPCancelSynchronousCalls(ref, errParam);
	}
}

extern pascal OSStatus
OTMPXGetEndpointInfo            (OTMPEndpointRef        ref,
                                 TEndpointInfo *        info)
	// OTMPX API entry point.
{
	if (gSystemVersion >= kMacOSXSystemVersion) {
		return OTGetEndpointInfo((EndpointRef) ref, info);
	} else {
		return OTMPGetEndpointInfo(ref, info);
	}
}

extern pascal OTResult
OTMPXGetEndpointState           (OTMPEndpointRef        ref)
	// OTMPX API entry point.
{
	if (gSystemVersion >= kMacOSXSystemVersion) {
		return OTGetEndpointState((EndpointRef) ref);
	} else {
		return OTMPGetEndpointState(ref);
	}
}

extern pascal OTResult
OTMPXLook                       (OTMPEndpointRef        ref)
	// OTMPX API entry point.
{
	if (gSystemVersion >= kMacOSXSystemVersion) {
		return OTLook((EndpointRef) ref);
	} else {
		return OTMPLook(ref);
	}
}

extern pascal OTResult
OTMPXCountDataBytes             (OTMPEndpointRef        ref,
                                 OTByteCount *          countPtr)
	// OTMPX API entry point.
{
	if (gSystemVersion >= kMacOSXSystemVersion) {
		return OTCountDataBytes((EndpointRef) ref, countPtr);
	} else {
		return OTMPCountDataBytes(ref, countPtr);
	}
}

extern pascal OSStatus
OTMPXGetProtAddress             (OTMPEndpointRef        ref,
                                 TBind *                boundAddr, /* can be NULL */
                                 TBind *                peerAddr) /* can be NULL */
	// OTMPX API entry point.
{
	if (gSystemVersion >= kMacOSXSystemVersion) {
		return OTGetProtAddress((EndpointRef) ref, boundAddr, peerAddr);
	} else {
		return OTMPGetProtAddress(ref, boundAddr, peerAddr);
	}
}

extern pascal OSStatus
OTMPXResolveAddress             (OTMPEndpointRef        ref,
                                 TBind *                reqAddr,
                                 TBind *                retAddr,
                                 OTTimeout              timeOut)
	// OTMPX API entry point.
{
	if (gSystemVersion >= kMacOSXSystemVersion) {
		return OTResolveAddress((EndpointRef) ref, reqAddr, retAddr, timeOut);
	} else {
		return OTMPResolveAddress(ref, reqAddr, retAddr, timeOut);
	}
}

extern pascal OSStatus
OTMPXBind                       (OTMPEndpointRef        ref,
                                 TBind *                reqAddr, /* can be NULL */
                                 TBind *                retAddr) /* can be NULL */
	// OTMPX API entry point.
{
	if (gSystemVersion >= kMacOSXSystemVersion) {
		return OTBind((EndpointRef) ref, reqAddr, retAddr);
	} else {
		return OTMPBind(ref, reqAddr, retAddr);
	}
}

extern pascal OSStatus
OTMPXUnbind                     (OTMPEndpointRef        ref)
	// OTMPX API entry point.
{
	if (gSystemVersion >= kMacOSXSystemVersion) {
		return OTUnbind((EndpointRef) ref);
	} else {
		return OTMPUnbind(ref);
	}
}

extern pascal OSStatus
OTMPXConnect                    (OTMPEndpointRef        ref,
                                 TCall *                sndCall,
                                 TCall *                rcvCall) /* can be NULL */
	// OTMPX API entry point.
{
	if (gSystemVersion >= kMacOSXSystemVersion) {
		return OTConnect((EndpointRef) ref, sndCall, rcvCall);
	} else {
		return OTMPConnect(ref, sndCall, rcvCall);
	}
}

extern pascal OSStatus
OTMPXListen                     (OTMPEndpointRef        ref,
                                 TCall *                call)
	// OTMPX API entry point.
{
	if (gSystemVersion >= kMacOSXSystemVersion) {
		return OTListen((EndpointRef) ref, call);
	} else {
		return OTMPListen(ref, call);
	}
}

extern pascal OSStatus
OTMPXAccept                     (OTMPEndpointRef        listener,
                                 OTMPEndpointRef        worker,
                                 TCall *                call)
	// OTMPX API entry point.
{
	if (gSystemVersion >= kMacOSXSystemVersion) {
		return OTAccept((EndpointRef) listener, (EndpointRef)worker, call);
	} else {
		return OTMPAccept(listener, worker, call);
	}
}

extern pascal OSStatus
OTMPXSndDisconnect              (OTMPEndpointRef        ref,
                                 TCall *                call) /* can be NULL */
	// OTMPX API entry point.
{
	if (gSystemVersion >= kMacOSXSystemVersion) {
		return OTSndDisconnect((EndpointRef) ref, call);
	} else {
		return OTMPSndDisconnect(ref, call);
	}
}

extern pascal OSStatus
OTMPXRcvDisconnect              (OTMPEndpointRef        ref,
                                 TDiscon *              discon) /* can be NULL */
	// OTMPX API entry point.
{
	if (gSystemVersion >= kMacOSXSystemVersion) {
		return OTRcvDisconnect((EndpointRef) ref, discon);
	} else {
		return OTMPRcvDisconnect(ref, discon);
	}
}

extern pascal OSStatus
OTMPXSndOrderlyDisconnect       (OTMPEndpointRef        ref)
	// OTMPX API entry point.
{
	if (gSystemVersion >= kMacOSXSystemVersion) {
		return OTSndOrderlyDisconnect((EndpointRef) ref);
	} else {
		return OTMPSndOrderlyDisconnect(ref);
	}
}

extern pascal OSStatus
OTMPXRcvOrderlyDisconnect       (OTMPEndpointRef        ref)
	// OTMPX API entry point.
{
	if (gSystemVersion >= kMacOSXSystemVersion) {
		return OTRcvOrderlyDisconnect((EndpointRef) ref);
	} else {
		return OTMPRcvOrderlyDisconnect(ref);
	}
}

extern pascal OSStatus
OTMPXOptionManagement           (OTMPEndpointRef        ref,
                                 TOptMgmt *             req,
                                 TOptMgmt *             ret)
	// OTMPX API entry point.
{
	if (gSystemVersion >= kMacOSXSystemVersion) {
		return OTOptionManagement((EndpointRef) ref, req, ret);
	} else {
		return OTMPOptionManagement(ref, req, ret);
	}
}

extern pascal OTResult
OTMPXRcv                        (OTMPEndpointRef        ref,
                                 void *                 buf,
                                 OTByteCount            nbytes,
                                 OTFlags *              flags)
	// OTMPX API entry point.
{
	if (gSystemVersion >= kMacOSXSystemVersion) {
		return OTRcv((EndpointRef) ref, buf, nbytes, flags);
	} else {
		return OTMPRcv(ref, buf, nbytes, flags);
	}
}

extern pascal OTResult
OTMPXSnd                        (OTMPEndpointRef        ref,
                                 void *                 buf,
                                 OTByteCount            nbytes,
                                 OTFlags                flags)
	// OTMPX API entry point.
{
	if (gSystemVersion >= kMacOSXSystemVersion) {
		return OTSnd((EndpointRef) ref, buf, nbytes, flags);
	} else {
		return OTMPSnd(ref, buf, nbytes, flags);
	}
}

extern pascal OSStatus
OTMPXSndUData                   (OTMPEndpointRef        ref,
                                 TUnitData *            udata)
	// OTMPX API entry point.
{
	if (gSystemVersion >= kMacOSXSystemVersion) {
		return OTSndUData((EndpointRef) ref, udata);
	} else {
		return OTMPSndUData(ref, udata);
	}
}

extern pascal OSStatus
OTMPXRcvUData                   (OTMPEndpointRef        ref,
                                 TUnitData *            udata,
                                 OTFlags *              flags)
	// OTMPX API entry point.
{
	if (gSystemVersion >= kMacOSXSystemVersion) {
		return OTRcvUData((EndpointRef) ref, udata, flags);
	} else {
		return OTMPRcvUData(ref, udata, flags);
	}
}

extern pascal OSStatus
OTMPXRcvUDErr                   (OTMPEndpointRef        ref,
                                 TUDErr *               uderr)
	// OTMPX API entry point.
{
	if (gSystemVersion >= kMacOSXSystemVersion) {
		return OTRcvUDErr((EndpointRef) ref, uderr);
	} else {
		return OTMPRcvUDErr(ref, uderr);
	}
}
