/*
	File:		MoreSCFCCLScanner.h

	Contains:	Code to generate list of modem script on Mac OS X.

	Written by:	Quinn

	Copyright:	Copyright (c) 2002 by Apple Computer, Inc., All Rights Reserved.

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

$Log: MoreSCFCCLScanner.h,v $
Revision 1.4  2002/12/12 15:24:03         
Correct the file name in the header comment.

Revision 1.3  2002/11/25 16:48:17         
Added MoreSCSetDefaultCCL. Also made changes to support building with CFM, even on Mac OS 9.  The algorithm to scan for CCLs is the same on 9 vs X, so we might as well use the same code.

Revision 1.2  2002/11/09 00:00:52         
Added compile time environment check. When using framework includes, explicitly include the frameworks we need. Convert nil to NULL.

Revision 1.1  2002/01/16 22:52:26         
First checked in.


*/

#pragma once

/////////////////////////////////////////////////////////////////

// MoreIsBetter Setup

#include "MoreSetup.h"

// System prototypes

#if MORE_FRAMEWORK_INCLUDES
	#include <CoreServices/CoreServices.h>
#else
	#include <CFArray.h>
#endif

/////////////////////////////////////////////////////////////////

#ifdef __cplusplus
extern "C" {
#endif

/////////////////////////////////////////////////////////////////

extern pascal OSStatus MoreSCCreateCCLArray(CFArrayRef *result, CFIndex *indexOfDefaultCCL);
	// Creates a CFArray containing a sorted list of CCL (modem script) 
	// names.  If indexOfDefaultCCL is not NULL then, on return, 
	// *indexOfDefaultCCL will be the index of the default CCL in 
	// result array.
	//
	// IMPORTANT: If all of the Modem Scripts folders are empty, this 
	// routine will return an empty array.  This is definitely a weird 
	// edge case.  If you call this function directly you should handle 
	// it appropriately.
	//
	// result must not be NULL.
	// On input, *result must not be NULL.
	// On error, *result is always NULL.
	// On success, *result is an array of CFStrings.
	// indexOfDefaultCCL may be NULL.  If it isn't NULL, on success 
	// *indexOfDefaultCCL is set to the index in *result of the 
	// default CCL.  

extern pascal void MoreSCSetDefaultCCL(CFStringRef cclName);
	// NULL is OK, clears previously set default.

/////////////////////////////////////////////////////////////////

#ifdef __cplusplus
}
#endif
