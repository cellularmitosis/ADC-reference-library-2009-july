/*
	File:		MoreOSUtils.h

	Contains:	Interface to the OS utilities library.

	Written by:	Quinn

	Copyright:	Copyright (c) 1999-2001 by Apple Computer, Inc., All Rights Reserved.

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

$Log: MoreOSUtils.h,v $
Revision 1.10  2003/04/09 19:45:07         
Moved a bunch of low-level Gestalt routines from MoreToolbox to MoreOSUtils.

Revision 1.9  2002/11/08 23:52:12         
When using framework includes, explicitly include the frameworks we need. Only include <CFString.h> when building Carbon.

Revision 1.8  2001/11/07 15:54:38         
Tidy up headers, add CVS logs, update copyright.


         <7>     28/9/01    Quinn   Added MoreCSCopyUserName and MoreCSCopyMachineName.
         <6>     21/9/01    Quinn   Changes for CWPro7 Mach-O build.
         <5>    24/11/00    Quinn   Make interrupt mask handling stuff CALL_NOT_IN_CARBON.
         <4>    24/11/00    Quinn   Correct routine name in previous checkin comment.
         <3>    24/11/00    Quinn   Carbon does not support 68K code generation, so I've made
                                    MakeData68KExecutable CALL_NOT_IN_CARBON. I could have made
                                    this work, at least on Mac OS 9, but I don't see the need right
                                    now.
         <2>     16/3/99    Quinn   Rolled in InterfaceDisableLib from DTS Technote 1137.
         <1>      1/3/99    Quinn   First checked in.
*/

#pragma once

/////////////////////////////////////////////////////////////////

// MoreIsBetter Setup

#include "MoreSetup.h"

// Standard Mac OS interfaces

#if MORE_FRAMEWORK_INCLUDES
	#include <CoreServices/CoreServices.h>
#else
	#include <MacTypes.h>
	#if TARGET_API_MAC_CARBON
		#include <CFString.h>
	#endif
#endif

#ifdef __cplusplus
extern "C" {
#endif

/////////////////////////////////////////////////////////////////
// OS-Level Gestalt Tests

extern pascal UInt32 MoreGetSystemVersion(void);
	// Returns gestaltSystemVersion.

extern pascal Boolean MoreRunningOnMacOSX(void);
	// True if running on Mac OS X.

extern pascal Boolean MoreRunningOnClassic(void);
	// True if running within Classic compatibility environment on Mac OS X.

// The following are wrappers that keeps code that relies 
// on the old names working.

pascal UInt32		GetSystemVersion	(void);
	// This is a compatibility wrappers for MoreGetSystemVersion 
	// that keeps old code compiling.
	
pascal Boolean		HavePowerManager	(void);

/////////////////////////////////////////////////////////////////
// Instruction Cache Flushing

extern pascal OSStatus MakeDataPowerPCExecutable(void *address, ByteCount count);
	// Make the memory range described by address and count
	// executable as PowerPC code.  This routine can only return
	// an error when called from classic 68K code.

#if CALL_NOT_IN_CARBON

extern pascal OSStatus MakeData68KExecutable(void *address, ByteCount count);
	// Make the memory range described by address and count
	// executable as 68K code.  This routine can never return an
	// error.  It has an error result purely for symmetry with
	// MakeDataPowerPCExecutable.

#endif

#if TARGET_CPU_68K && !TARGET_RT_MAC_CFM

	// IMPORTANT:
	// This routine is only necessary if you're compiling
	// classic 68K which calls MakeDataPowerPCExecutable.

	extern pascal void TermMoreOSUtils(void);
		// A termination routine for this library.  Call it before
		// you unload from memory.

#endif

/////////////////////////////////////////////////////////////////
// Interrupt Enable and Disable

// See DTS Technote 1xxx "Disabling Interrupts on the Traditional Mac OS"
// for details on how this works, why you should avoid it, and what
// alternatives there are.

#if CALL_NOT_IN_CARBON

extern pascal UInt16 GetInterruptMask(void);
	// Returns the current interrupt mask, as a value
	// from 0 to 7.
	
extern pascal UInt16 SetInterruptMask(UInt16 newMask);
	// Sets the current interrupt mask, as a value
	// from 0 to 7.

#endif

/////////////////////////////////////////////////////////////////
// Machine and User Name Compatibility

// These are only available to Carbon clients (because they're the only 
// ones who can use a CFStringRef.

#if TARGET_API_MAC_CARBON

	// Works around a bug in Mac OS 10.0 and 10.0.1 whereby the 
	// system wasn't retaining these strings. [2665708]
	//
	// Also works around a problem in all systems prior to 
	// Mac OS X 10.1 where MoreCSCopyMachineName would return the 
	// BSD host name rather than the computer name as set in the 
	// Sharing panel of System Preferences. [2650897]

	extern pascal CFStringRef MoreCSCopyUserName(Boolean useShortName);

	extern pascal CFStringRef MoreCSCopyMachineName(void);

#endif

#ifdef __cplusplus
}
#endif
