/*
	File:		MoreBlueActions.h

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

$Log: MoreBlueActions.h,v $
Revision 1.4  2002/11/08 23:37:27         
When using framework includes, explicitly include the frameworks we need.

Revision 1.3  2001/11/07 15:51:28         
Tidy up headers, add CVS logs, update copyright.


         <2>     21/9/01    Quinn   Changes for CWPro7 Mach-O build.
         <1>     7/11/00    Quinn   First checked in.
*/

#pragma once

/////////////////////////////////////////////////////////////////

// MoreIsBetter Setup

#include "MoreSetup.h"

// Mac OS Interfaces

#if MORE_FRAMEWORK_INCLUDES
	#include <CoreServices/CoreServices.h>
#else
	#include <OpenTransport.h>
#endif

/////////////////////////////////////////////////////////////////

#ifdef __cplusplus
extern "C" {
#endif

// Overview
// --------
// This module provides an efficient mechanism for scheduling deferred 
// task from an MP task.  Rather than use one deferred task for each 
// MP to Blue transition, we use our own deferred task equivalent 
// (see the MoreBlueAction structure, below), maintain a list of these 
// structures, and use a single deferred task to execute the entire list.

// This module logs its activities to MoreMPLog.  You can control which 
// bit in the log mask it uses (see "MoreMPLog.h" for a discussion of the 
// log mask) by changing this constant.

enum {
	kMoreBlueActionsLogID = 0
};

extern pascal OSStatus InitMoreBlueActions(void);
	// Initialises this module.
	// Must be called at system task time.
	
extern pascal void     TermMoreBlueActions(void);
	// Shuts down this module.
	// Must be called at system task time.

/////////////////////////////////////////////////////////////////
// MoreBlueAction structure

// Forward declare to break mutual dependency.

typedef struct MoreBlueAction MoreBlueAction;

typedef pascal void (*MoreBlueActionProc)(MoreBlueAction *thisTask);

enum {
	kMoreBlueActionMagic = 'BTsk'
};

struct MoreBlueAction {
	OSType 			 	magic;			// must be kMoreBlueActionMagic
	OTLink 			 	link;			// next MoreBlueAction in the list
	MoreBlueActionProc	action;		// routine to callback
	void *			 	refCon;		// for client's use
};

extern pascal void MoreBlueActionInstall(MoreBlueAction *thisTask);
	// A safe and efficient way for MP tasks to schedule code to run 
	// at deferred task time in the 'Blue' environment.
	// Like DTInstall, the MoreBlueAction structure must continue to be 
	// valid until the callback is called.
	//
	// You must set thisTask->magic before installing the task.
	//
	// Legal to call from non-MP tasks.

#if MORE_DEBUG

	// When debugging, it's often useful to run all of your code at 
	// system task time.  These routines let you do just that.

	extern pascal void MoreBTRunAsSystemTasks(Boolean asSystemTasks);
		// If asSystemTasks is true, the module will run blue tasks at 
		// system task time.  You should call this routine before 
		// scheduling any blue tasks.  Once you've set blue tasks to run 
		// at system task time you must then call MoreBTRunNow to actually 
		// run the queued tasks.
		// Must be called at system task time.
	
	extern pascal void MoreBTRunNow(void);
		// Runs any queued blue tasks.  Only necessary if you've set 
		// blue tasks to run at system task time using the above routine.
		// Must be called at system task time.
	
#endif

#ifdef __cplusplus
}
#endif
