/*
	File:		MoreScheduledExec.h

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

$Log: MoreScheduledExec.h,v $
Revision 1.4  2002/11/08 23:57:03         
Move compile time environment check to header. Convert nil to NULL.

Revision 1.3  2001/11/07 15:55:12         
Tidy up headers, add CVS logs, update copyright.


         <2>     21/9/01    Quinn   Changes for CWPro7 Mach-O build.
         <1>      6/3/00    Quinn   First checked in.
*/

#pragma once

/////////////////////////////////////////////////////////////////

// MoreIsBetter Setup

#include "MoreSetup.h"

#if TARGET_API_MAC_CARBON
	// We can’t work for Carbon software because we rely on features of
	// traditional Mac OS that will never be Carbon-compatible.
	//
	// o We need to allocate memory in the system heap.
	//
	// o We need Time Manager tasks installed in the system heap to persist
	//   when the application quits.
	//
	// o We need to execute 68K machine code.
	//
	// Carbon code on traditional Mac OS can use this module by putting it
	// in a plug-in and linking to the plug-in at runtime using GetSharedLibrary
	// (or perhaps using CFMLateImport, part of MoreIsBetter).  Carbon code
	// on Mac OS X has no explicit alternative to this code, although the cost
	// of background processes on Mac OS X is substantially less.

	#error MoreScheduledExec: This module can not be used by Carbon programs.
#endif

// Standard Mac OS interfaces

#if ! MORE_FRAMEWORK_INCLUDES
	#include <MacTypes.h>
	#include <Files.h>
	#include <AEDataModel.h>
#endif

/////////////////////////////////////////////////////////////////

#ifdef __cplusplus
extern "C" {
#endif

// This module is designed to allow you to quit your application and have
// it be relaunched at some time in the future.  By using this module, you can
// avoid running a background-only application whose sole purpose is to wake
// up at a specific time and do stuff.  Eliminating a superfluous background-only
// application is good for the system because it reduces the system memory footprint
// and it shortens the Process Manager scheduling lists.
//
// This module stores its persistent state in a pointer block in the system heap,
// which it tracks using Gestalt.  Therefore you can call these routines from any 
// global variable context (for example, an 'INIT' and an application).  The only
// requirement is that you use a unique, registered, signature for each situation
// in which you use this module.
//
// There are two common usage scenarios for this module:
//
// o If you have an 'appe' that sleeps for long periods of time, you can install
//   it the Extensions folder.  At startup time, the system will launch your
//   'appe'.  Your 'appe' can do its job and decide how long it needs to sleep
//   for.  It can then call MoreDelayedLaunchApplication and quit.  After the
//   appropriate delay, this module will relanch your application.
//
// o If you have a an 'INIT' containing an application, you can call
//   MoreDelayedLaunchApplication from your 'INIT', with some arbitrary
//   delay.  After the system starts up, this module will launch your application.
//   Your application can do its job and, if necessary, reschedule itself,
//   by calling MoreDelayedLaunchApplication again, and quit.
//
// You can maintain persistent state between executions of your application
// (without using a preferences file) by means of the appParams parameter to 
// MoreDelayedLaunchApplication.  You can package your persistent state into an 
// AppleEvent, coerce it a descriptor of typeAppParameters, and pass it to 
// MoreDelayedLaunchApplication.  When the module relaunches your application, it 
// will receive this persistent state as an AppleEvent

extern pascal OSStatus MoreDelayedLaunchApplication(
									OSType signature,
									const FSSpec *appToLaunch,		// may be NULL
									UInt32 appLaunchFlags,
									const AEDesc *appParams,		// may be NULL
									Duration launchDelay);
	// Launch the application specified by appToLaunch after the 
	// delay specified by launchDelay.
	//
	// You must provide a unique signature; this does not have to be
	// the creator of your application but it must still be 
	// registered with DTS.  [The signature is used as a Gestalt selector
	// to hold the global state for your application.]  If a application
	// of this signature is already scheduled to be launched, the routine
	// will return paramErr.
	//
	// appToLaunch may be NULL if a) your application is currently running,
	// and b) signature is your application’s creator code.  In this case,
	// the routine derives the FSSpec of the application to launch from
	// the FSSpec of the running application.
	//
	// appLaunchFlags is a bit mask that is OR’d into the launchControlFlags
	// field of the LaunchParamBlockRec.  The most likely flag that you might
	// want to specify is launchUseMinimum or launchDontSwitch.
	//
	// appParams is most commonly NULL, in which case the application is
	// just launched in the normal way.  If it isn’t NULL, it must be
	// a AEDesc of typeAppParameters.  The routine will launch the
	// application and pass it appParams as its launch parameters
	// (ie in the launchAppParameters field of the LaunchParamBlockRec).
	// The module makes a copy of the appParams; you still own the AEDesc
	// you pass in.
	//
	// launchDelay is either a positive value (in milliseconds) or a negative
	// value (in microseconds).  You should specify a sufficient delay to allow
	// your application time to quit.

extern pascal OSStatus MoreCancelDelayedLaunchApplication(OSType signature);
	// This routine cancels the scheduled launch of an application
	// which was set up by the previous call.  signature should match
	// the signature supplied when the scheduled launch was set up.

#ifdef __cplusplus
}
#endif
