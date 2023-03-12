/*
	File:		MoreOSLStringCompare.h

	Contains:	String comparison utilities for MoreOSL.

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

$Log: MoreOSLStringCompare.h,v $
Revision 1.7  2002/11/08 23:48:53         
Include our prototype early to flush out any missing dependencies.

Revision 1.6  2001/11/07 15:54:26         
Tidy up headers, add CVS logs, update copyright.


         <5>     24/9/01    Quinn   Fixes to compile with C++ activated.
         <4>     21/9/01    Quinn   Changes for CWPro7 Mach-O build.
         <3>     27/3/00    Quinn   Change "Pre-Carbon" -> "Non-Carbon" to be consisent with rest of
                                    MOSL.
         <2>     20/3/00    Quinn   Lots of comments.
         <1>      9/3/00    Quinn   First checked in.
*/

#pragma once

/////////////////////////////////////////////////////////////////

// MoreIsBetter Setup

#include "MoreSetup.h"

// Mac OS Interfaces

#if MORE_FRAMEWORK_INCLUDES
	#include <Carbon/Carbon.h>
#else
	#include <AEDataModel.h>
	#if TARGET_API_MAC_CARBON
		#include <CFString.h>
	#endif
#endif

// MIB Prototypes

/////////////////////////////////////////////////////////////////

#ifdef __cplusplus
extern "C" {
#endif

/////////////////////////////////////////////////////////////////

// String Comparison and AppleScript
// ---------------------------------
// Implementing support for AppleScript’s string comparison operators is
// easy to do badly, but very hard to do right.  The challenges include:
//
//   1.	The system provides no international friendly way of implementing 
//		the "contains" operator that works 100% correctly.  The CFString
//		mechanism works pretty well, but it does have it’s problems (see below).
//
//	 2. Any mechanism provided by the system still wouldn’t be the same as
//		AppleScript’s built-in string operators because AppleScript carries
//		around its own string canonicalisation tables.
//
//	 3. There is no documented way of getting at the script’s current
//		"considering" and "ignoring" state.  This is a known limitation,
//		documented in the AppleScript Language Guide, but it’s still lame.
//
// Some of these difficulties are evident even in the Roman script system.
// For example, assuming document 1 is called "ﬁnd" (the first character
// is the fi ligature (option-shift-5)), the script:
//
//  	tell application "X"
//		    name of document 1 = "find"
//		end tell
//
// might produce different results from:
//
//  	tell application "X"
//  		get name of document 1
//    		result = "find"
//  	end tell
//
// because the application implements string comparison in terms of IdenticalText
// or CFStringCompare, which break down the ligature, but AppleScript’s built-in
// string tables doesn't (even if you turn on expansion).
//
// Moreover, while testing this against CFString I discovered a bug [2442526] that
// causes it to break the identity:
//
// 		(a equals b) implies ((a contains b) and (b contains a))
//
// While this probably won’t be a huge problem in practice, it is annoying.
//
// While casting around for a solution (especially for the "contains" operator)
// I looked at how some other applications did this.  That was a depressing
// exercise.  The Finder’s implementation of "contains", for example, does not
// even attempt to be two-byte friendly, nor does it address the possibility
// that IdenticalText may return true for text of different length (for example,
// "ﬁnd" vs "find").
//
// Despite these obstacles, an application /must/ implement string comparison
// in order to support the simplest AppleScript constructs (such as formName).
// So I had to come up with a solution.  I chose two different paths depending
// on the target environment.
//
// 	 1.	Carbon -- For Carbon applications, CFString was the obvious solution,
//		despite the niggling bug described above.
//
//	 2. Non-Carbon -- For non-Carbon applications, there is no obvious, clean
//		solution.  Eventually I resorted to loading an AppleScript (using OSA)
//		and forcing it to do the comparison (using OSADoEvent).  Sleasy, but
//		functionaly.
//
// After this much pain, I thought it was important to file a bug [2444555] asking
// for a better solution.

extern pascal OSStatus MOSLStringCompare(DescType oper, const AEDesc *data1, const AEDesc *data2, Boolean *result);
	// This routine is the core string compare functionality provided
	// by MOSL.  It is called by both MOSL itself, and is available for your
	// general use.  It implements the following operators:
	//
	//		kAEEquals
	//		kAEGreaterThanEquals
	//		kAEGreaterThan
	//		kAELessThan
	//		kAELessThanEquals
	//		kAEBeginsWith
	//		kAEEndsWith
	//		kAEContains
	//
	// For Carbon code, comparison is done using CFString with the following
	// options:  kCFCompareCaseInsensitive, kCFCompareNonliteral, kCFCompareLocalized.
	//
	// For non-Carbon code, comparison is done using an embedded AppleScript,
	// using the default AppleScript comparison options.
	//
	// IMPORTANT:
	// Non-Carbon implementations must have the 'scpt' ID=5300 resource from
	// "MoreOSLStringCompare.rsrc" available in CurResFile before calling 
	// this routine.

// The following routines are provided for use by your "accessByName"
// object primitive.

extern pascal OSStatus MOSLStringEqualsPStringDesc(ConstStr255Param myObjectName, const AEDesc *comparisonName, Boolean *result);
	// This routine is a helper wrapper function that allows you to
	// test a Pascal string for equality with an AEDesc.

extern pascal OSStatus MOSLStringEqualsCStringDesc(const char *myObjectName, const AEDesc *comparisonName, Boolean *result);
	// This routine is a helper wrapper function that allows you to
	// test a C string for equality with an AEDesc.

extern pascal OSStatus MOSLStringEqualsIntlTextDesc(ScriptCode script, LangCode lang, const void *textBuf, Size textBufLen, const AEDesc *comparisonName, Boolean *result);
	// This routine is a helper wrapper function that allows you to
	// test the guts of an typeIntlText AEDesc for equality with an AEDesc.

#if TARGET_API_MAC_CARBON

	extern pascal OSStatus MOSLStringEqualsCFStringDesc(CFStringRef myObjectName, const AEDesc *comparisonName, Boolean *result);
		// This routine is a helper wrapper function that allows you to
		// test a CFString for equality with an AEDesc.

#endif

/////////////////////////////////////////////////////////////////

#ifdef __cplusplus
}
#endif
