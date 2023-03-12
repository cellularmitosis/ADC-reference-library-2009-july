/*
	File:		MoreAutoPush.h

	Contains:	A simple framework for doing autopush properly.

	Written by: Quinn "The Eskimo!"

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

$Log: MoreAutoPush.h,v $
Revision 1.4  2002/11/08 23:40:42         
Compile time environment check now in header.

Revision 1.3  2001/11/07 15:51:11         
Tidy up headers, add CVS logs, update copyright.


         <2>     21/9/01    Quinn   Changes for CWPro7 Mach-O build.
         <1>     5/10/00    Quinn   First checked in.
*/

#pragma once

/////////////////////////////////////////////////////////////////////

// MIB Setup

#include "MoreSetup.h"

#if TARGET_API_MAC_CARBON
	// MoreAutoPush doesn’t make sense for Carbon development because 
	// there’s no guarantee that you’re running on a system with STREAMS. 
	// Specifically, Mac OS X does not include a STREAMS environment.

	#error MoreAutoPush does not support Carbon development.
#endif

// Mac OS Interfaces

#if ! MORE_FRAMEWORK_INCLUDES
	#include <OpenTransportProtocol.h>
#endif

/////////////////////////////////////////////////////////////////////

#ifdef __cplusplus
extern "C" {
#endif

extern pascal OSStatus ValidateModuleExists(const char *moduleName);
	// Verify that the specified module exists within the
	// OT environment.	We do this by calling the "sad"
	// module and asking it to check.  Returns kOTNotFoundErr
	// if the module does not exist.

extern pascal OSStatus GetAutoPushList(const char *driverName, OTAutopushInfo *autoPushInfo);
	// This is a high-level entry point into the framework.
	// This routine returns the autopush information associated
	// with the given driver.
	
extern pascal OSStatus AddModuleToAutoPushList(const char *driverName, const char *moduleName);
	// This is a high-level entry point into the framework.
	// This routine adds the given module to the autopush
	// list associated with the given driver.
	//
	// Some important notes:
	// 1. You must supply the driver name, not the port name.
	//	  For example, supply "mace", not "enet0".
	// 2. The module is added to the end of the autopush list.
	//	  If you need any other position, you'll have to crack
	//	  open this abstraction layer.
	// 3. A result of kOTAddressBusyErr means that too many
	//	  autopushes have been specified for the driver.
	//	  The limit is kOTAutopushMax.
	// 4. A result of kEEXISTErr means that the module is already
	//	  in the autopush list for this driver.
	
extern pascal OSStatus RemoveModuleDriverAutoPushList(const char *driverName, const char *moduleName);
	// This is a high-level entry point into the framework.
	// This routine removes the given module from the autopush
	// list associated with the driver.	 The order of the remaining
	// modules is preserved.
	//
	// Some important notes:
	// 1. A result of kENOENTErr means that the module is not
	//	  in the autopush list for the driver.

#ifdef __cplusplus
}
#endif
