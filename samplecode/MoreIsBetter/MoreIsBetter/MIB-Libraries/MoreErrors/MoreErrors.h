/*
	File:		MoreErrors.h

	Contains:	Error handling utilities.

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

$Log: MoreErrors.h,v $
Revision 1.5  2003/02/26 12:11:37         
Allocated an error range for MoreSCF.

Revision 1.4  2002/11/08 23:31:15         
Explicitly document the error code ranges used by the various MoreIsBetter modules. When using framework includes, explicitly include the frameworks we need.

Revision 1.3  2001/11/07 15:52:27         
Tidy up headers, add CVS logs, update copyright.


         <2>     21/9/01    Quinn   Changes for CWPro7 Mach-O build.
         <1>    22/12/00    Quinn   First checked in.
*/

#pragma once

/////////////////////////////////////////////////////////////////

// MIB Setup

#include "MoreSetup.h"

// Mac OS Interfaces

#if MORE_FRAMEWORK_INCLUDES
	#include <CoreServices/CoreServices.h>
#else
	#include <MacTypes.h>
#endif

/////////////////////////////////////////////////////////////////

#ifdef __cplusplus
extern "C" {
#endif

/////////////////////////////////////////////////////////////////

// Error Code Allocation
// ---------------------
// MoreIsBetter allocates error codes in blocks of 20 in the range 
// from 5300 and up.  This is in accordance with Apple guidelines 
// about application error code allocation.
// 
// <http://developer.apple.com/qa/ov/ov02.html>
//
// Current allocations are:
//
// 5300..5319 - MoreOSL
// 5320..5339 - MoreCFMPatches
// 5340..5359 - MoreNetworkSetup
// 5360..5379 - MoreSecurity
// 5380..5399 - MoreSCF
// 5400..5419 - MoreCFQ
// 

/////////////////////////////////////////////////////////////////

extern pascal void MoreLookupError(SInt16 errorResourceID, OSStatus errNum, Str255 errStr);
	/*  Return the string associated with errNum.  The 'µErr' resource
		template is given in "MoreErrors.r"
		
		There must be a terminating entry with error number of 0 that contains
		the default error message.
	*/

#ifdef __cplusplus
}
#endif
