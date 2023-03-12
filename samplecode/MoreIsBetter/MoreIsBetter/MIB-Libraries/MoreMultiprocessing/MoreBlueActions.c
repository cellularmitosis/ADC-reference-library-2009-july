/*
	File:		MoreBlueActions.c

	Contains:	Library for efficiently scheduling deferred task actions
				from MP tasks.

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

$Log: MoreBlueActions.c,v $
Revision 1.6  2002/11/08 23:37:44         
Include our prototype early to flush out any missing dependencies. Convert nil to NULL. Convert MoreAssertQ to assert.

Revision 1.5  2001/11/07 15:51:23         
Tidy up headers, add CVS logs, update copyright.


         <4>     21/9/01    Quinn   Changes for CWPro7 Mach-O build.
         <3>    22/11/00    Quinn   Don't allow classic 68K builds for the reasons explained in the
                                    comments.
         <2>    10/11/00    Quinn   Added more MPLog points. I used this to help track down a
                                    CarbonLib bug, but they are generally useful so I thought I'd
                                    keep them around.
         <1>     7/11/00    Quinn   First checked in.
*/

/////////////////////////////////////////////////////////////////

// Our prototypes

#include "MoreBlueActions.h"

// Mac OS Interfaces

#if ! MORE_FRAMEWORK_INCLUDES
	#include <OSUtils.h>
	#include <Multiprocessing.h>
#endif

// MIB interfaces

#include "MoreMPLog.h"

/////////////////////////////////////////////////////////////////

static pascal void MoreBlueActionsRun(long dtParam);		// Forward declare to allow globals to be grouped before code.

static OTLIFO gMoreBlueActionList = {NULL};				// List of MoreBlueAction's chained through link.

// I want to statically allocate my RoutineDescriptor (because it avoids 
// possible memory allocation failures) but Carbon won't let me do that.
// *sigh*
// 
// gMoreBlueActionsRun is the single deferred task that is used to run all 
// of our Blue actions.  The MoreBlueActionInstall routine allocates it by using
// an atomic test and set on gMoreBlueActionsRunInUse.


#if TARGET_API_MAC_CARBON
	static DeferredTask gMoreBlueActionsRun = {NULL, dtQType, 0, NULL, 0, 0};
#else
	static RoutineDescriptor gMoreBlueActionsRunRD = BUILD_ROUTINE_DESCRIPTOR(uppDeferredTaskProcInfo, MoreBlueActionsRun);

	// We can’t use this module for classic 68K code because &gMoreBlueActionsRunRD 
	// is not a valid UPP for the 68K build.  I originally tried to work around this 
	// by conditionally declaring this structure to use MoreBlueActionsRun for 
	// the 68K case.  However, this doesn't really work properly because the 
	// callback has to use register calling conventions.  I could fix this with 
	// enough messing around with "#pragma parameter" and :__d0, but I decided 
	// it wasn’t worth the effort because this module is specifically targetted 
	// for MP code, and you can’t run 68K code in an MP task.
	
	#if !TARGET_RT_MAC_CFM
		#error This module can not be used by 68K code.
	#endif
	
	static DeferredTask gMoreBlueActionsRun = {NULL, dtQType, 0, &gMoreBlueActionsRunRD, 0, 0};
#endif

static OTLock gMoreBlueActionsRunInUse = 0;	// Set if gMoreBlueActionsRun is currently in the
											// deferred task queue.

// This global controls whether blue actions are run at deferred task time 
// or system task time.  See the header file comments for a discussion of 
// this facility.

#if MORE_DEBUG

	static Boolean gRunAsSystemTasks = false;
	
	static Boolean gModuleInitialised;

#endif

extern pascal OSStatus InitMoreBlueActions(void)
	// See comment in header file.
{
	OSStatus err;

	assert( ! MPTaskIsPreemptive(kInvalidID) );
	assert( ! gModuleInitialised );

	err = noErr;
	#if TARGET_API_MAC_CARBON
		// Carbon makes me do this silliness!
		gMoreBlueActionsRun.dtAddr = NewDeferredTaskUPP(MoreBlueActionsRun);
		if (gMoreBlueActionsRun.dtAddr == NULL) {
			err = memFullErr;
		}
	#endif
	#if MORE_DEBUG
		gModuleInitialised = (err == noErr);
	#endif
	return err;
}

extern pascal void     TermMoreBlueActions(void)
	// See comment in header file.
{
	assert( ! MPTaskIsPreemptive(kInvalidID) );
	assert( gModuleInitialised );

	#if TARGET_API_MAC_CARBON
		if (gMoreBlueActionsRun.dtAddr != NULL) {
			DisposeDeferredTaskUPP(gMoreBlueActionsRun.dtAddr);
			gMoreBlueActionsRun.dtAddr = NULL;
		}
	#endif
	#if MORE_DEBUG
		gModuleInitialised = false;
	#endif
}

static pascal void MoreBlueActionsRun(long dtParam)
	// This routine is run in two possible ways.  If blue actions 
	// are being run as deferred tasks (which typically is true), 
	// this routine is called by the Deferred Task Manager.
	// If blue actions are being run as system tasks (during debugging),
	// this routine is run by MoreBTRunNow, which is called by the client
	// at system task time.
	//
	// Regardless of how it's call, the routine does the same thing.
	// It first clears gMoreBlueActionsRunInUse, which ensures that any 
	// future blue actions being installed will schedule the deferred task 
	// again.  It then grabs the list of blue actions and runs them one 
	// at a time.
{
	OTLink *taskList;
	
	MPLog(kMoreBlueActionsLogID, '>Blu');
	
	#pragma unused(dtParam)

	OTClearLock(&gMoreBlueActionsRunInUse);
		
	taskList = OTReverseList(OTLIFOStealList(&gMoreBlueActionList));
	while ( taskList != NULL ) {
		MoreBlueAction *thisTask;
		
		thisTask = OTGetLinkObject(taskList, MoreBlueAction, link);
		assert(thisTask->magic == kMoreBlueActionMagic);
		
		// Move taskList along to next element before calling action proc
		// so that action proc can 'dispose' of thisTask in some way.
		
		taskList = taskList->fNext;
		
		assert(thisTask->action != NULL);

		MPLog1(kMoreBlueActionsLogID, 'BluD', thisTask);
		(thisTask->action)(thisTask);
	}

	MPLog(kMoreBlueActionsLogID, '<Blu');
}

extern pascal void MoreBlueActionInstall(MoreBlueAction *thisTask)
	// See comment in header file.
{
	OSStatus junk;

	assert(thisTask->magic  == kMoreBlueActionMagic);
	assert(thisTask->action != NULL);
	assert( gModuleInitialised );
	
	MPLog1(kMoreBlueActionsLogID, '>BlQ', thisTask);

	// First put the task on the list of tasks to run.  Then 
	// see whether the deferred task is already installed or not. 
	// If it isn't, install it.  If it is, just return.  The 
	// deferred task will handle /all/ of the requests on the list.
	
	OTLIFOEnqueue(&gMoreBlueActionList, &thisTask->link);
	
	if ( OTAcquireLock(&gMoreBlueActionsRunInUse) ) {
		#if MORE_DEBUG
			if (gRunAsSystemTasks) {
				// Blue actions will be run when client calls MoreBlueActionsRunNow.
				MPLog1(kMoreBlueActionsLogID, 'BlQ1', thisTask);
				junk = noErr;
			} else {
				MPLog1(kMoreBlueActionsLogID, 'BlQ2', thisTask);
				junk = DTInstall(&gMoreBlueActionsRun);
			}
		#else
			junk = DTInstall(&gMoreBlueActionsRun);
		#endif
		
		assert(junk == noErr);
	} else {
		MPLog1(kMoreBlueActionsLogID, 'BlQ3', thisTask);
	}

	MPLog(kMoreBlueActionsLogID, '<BlQ');
}

#if MORE_DEBUG

	extern pascal void MoreBTRunAsSystemTasks(Boolean asSystemTasks)
		// See comment in header file.
	{
		assert( ! MPTaskIsPreemptive(kInvalidID) );
		gRunAsSystemTasks = asSystemTasks;
	}
	
	extern pascal void MoreBTRunNow(void)
		// See comment in header file.
	{
		assert( ! MPTaskIsPreemptive(kInvalidID) );
		if ( gMoreBlueActionsRunInUse ) {
			MoreBlueActionsRun(0);
		}
	}
	
#endif
