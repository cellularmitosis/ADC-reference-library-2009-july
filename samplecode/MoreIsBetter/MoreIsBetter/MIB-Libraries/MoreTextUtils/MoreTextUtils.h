/*
	File:		MoreTextUtils.h

	Contains:	Simple utilities for operating on text.

	Written by:	Pete Gontier

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

$Log: MoreTextUtils.h,v $
Revision 1.11  2003/03/28 16:15:35         
Removed a bunch of #pragma import/export directives.  IIRC PCG added them for reasons I don't fully understand.  He may have meant #pragma internal.  Regardless, there presence was causing lots of MIB symbols to be CFM exported, which is wrong.  MIB is not meant to ship as a shared library.

Revision 1.10  2002/11/09 00:11:59         
When using framework includes, explicitly include the frameworks we need. Convert nil to NULL.

Revision 1.9  2001/11/07 15:55:48         
Tidy up headers, add CVS logs, update copyright.


         <8>     21/9/01    Quinn   Project Builder compatibility.
         <7>     21/9/01    Quinn   Changes for CWPro7 Mach-O build.
         <6>     15/2/01    Quinn   Changed tStringList type to accomodate Project Builder’s
                                    (rightful) strictness. Deprecated GetPascalStringFromLongDouble
                                    (see comments for reasons).
         <5>     11/4/00    Quinn   Added more 'STR#' routines with a new, consistent naming
                                    convention.  Provided compatibility for the old routine names. 
                                    Updated header comment (blergh!).
         <4>     20/3/00    Quinn   Added MoreReplaceText and its support routines.  Added
                                    MoreStrLen.  Tidied up a little.
         <3>     21/4/99    Quinn   Added ValidStringListHandle.
         <2>    11/11/98    PCG     fix headers
         <1>    11/10/98    PCG     first big re-org at behest of Quinn

	Old Change History (most recent first):

         <5>    10/11/98    Quinn   Convert "MorePrefix.h" to "MoreSetup.h".
         <4>    10/23/98    PCG     add GetPascalStringFromLongDouble
         <3>      9/9/98    PCG     re-work import and export pragmas
         <2>     7/24/98    PCG	    coddle linker (C++, CFM-68K)
         <1>     6/16/98    PCG     initial checkin
*/

#pragma once

/////////////////////////////////////////////////////////////////

// MoreIsBetter Setup

#include "MoreSetup.h"

// Mac OS Interfaces

#if MORE_FRAMEWORK_INCLUDES
	#include <ApplicationServices/ApplicationServices.h>
#else
	#include <TextUtils.h>
	#include <fp.h>
#endif

// Standard C interfaces

/////////////////////////////////////////////////////////////////

#ifdef __cplusplus
	extern "C" {
#endif

// Pete Gontier’s own string list routines.

struct tStringList
{
	UInt16				count;
	ConstStringPtr		list[kVariableLengthArray];

	// followed by packed strings from 'STR#' resource
};
typedef struct tStringList tStringList, *tStringListP, **tStringListH;

#if 0

// I have removed this routine from MIB because it uses long double. 
// Using long double is almost always a mistake because CW and MrC 
// have radically different ideas how long a long double is (CW PPC 
// treats long double as a IEEE-754 "double", CW 68K treats long double 
// as an extended (either or 80 or 96 depending on the 68881 setting), 
// and MrC has a weird 'two bonded IEEE-754 "double"s' format for long 
// double.
// -- Quinn, 8 Feb 2001

pascal OSErr	GetPascalStringFromLongDouble	(long double, SInt8 precision, StringPtr);

#endif

pascal OSErr	GetNewStringList				(short resID, tStringListP *newStringList);

pascal OSErr	NewStringPtr					(ConstStr255Param, UInt8 maxSize, StringPtr *result);

/////////////////////////////////////////////////////////////////////

// Routines to operate on standard string lists (ie handles in 'STR#' format).
// We provide all the common semantics, including count, get, set, delete,
// insert, and append.  We also provide a validate for cases where you’re
// reading a handle from a resource and want to make sure that it hasn’t
// been corrupted.
//
// In most cases you can use these routines on both memory and resource handles
// (the obvious exception being MoreStrListNew).  Therefore, these routines
// must (and do) work properly with purgeable handles.  If the handle has
// been purged, the routines will call LoadResource to attempt to reload
// the data, returning the appropriate Resource Manager if the attempt
// fails (typically resNotFound is returned if the handle is a memory handle).
// Also, the routines temporarily mark the handles are being non-purgeable while
// they work on them, and restore the handle status before returning.

extern pascal OSStatus MoreStrListNew(Handle *strList);
	// Creates a new string list handle and returns it in
	// *strList.  You can dispose of this by calling DisposeHandle.
	// On error, returns an error and *strList is NULL.

extern pascal OSStatus MoreStrListEmpty(Handle strList);
	// Converts the specified handle into an empty string list.
	// Throws away the current contents of the handle without
	// looking at them.

extern pascal SInt16   MoreStrListCount(Handle strList);
	// Returns the number of strings in the stringList handle (STR# format).

extern pascal OSStatus MoreStrListGetIndexed(Handle strList, SInt16 index, Str255 str);
	// Returns the index'th string in the string list.
	// index is one-based.
	// Returns paramErr if index is not in the range [1..count].

extern pascal OSStatus MoreStrListSetIndexed(Handle strList, SInt16 index, ConstStr255Param str);
	// Sets the index'th string in the string list.
	// index is one-based.
	// Returns paramErr if index is not in the range [1..count].

extern pascal OSStatus MoreStrListDeleteIndexed(Handle strList, SInt16 index);
	// Deletes the index'th string in the string list, moving all
	// the subsequent strings down.
	// index is one-based.
	// Returns paramErr if index is not in the range [1..count].

extern pascal OSStatus MoreStrListInsertAtIndex(Handle strList, SInt16 index, ConstStr255Param str);
	// Inserts str at the index'th position in the string list;
	// subsequent strings are moved up one index position.
	// After the call, str is at position index and the string
	// previously at position index has been moved to index+1.
	// Pass an index of 1 to insert at the beginning of the string
	// list, and an index of count+1 to append.

extern pascal OSStatus MoreStrListAppend(Handle strList, ConstStr255Param str);
	// Appends the string at the end of the string list.

extern pascal Boolean  MoreStrListValidate(Handle strList);
	// Return true if stringList is a valid string list (STR#).
	// This is useful when you get a string list out of a resource
	// (such as a preferences file) and you want to make sure that
	// you can safely operate on it with the other string list routines
	// in this file.

// These macros provide compatibility for the various ad hoc 'STR#' 
// handling routines previously exported by this module.  The new names
// are more regular (they all use <noun><verb> structure), are in the
// correct namespace (start with "More"), and are shorter ("StrList"
// rather than "StringListHandle).

#if 1
	#define NewStringListHandle MoreStrListNew
	#define AppendStringToListHandle(str, strList) MoreStrListAppend((strList), (str))
	#define ValidStringListHandle MoreStrListValidate
	#define CountStringListHandle MoreStrListCount
#endif

/////////////////////////////////////////////////////////////////////

// Other text utilities.

extern pascal OSStatus InitMoreTextUtils(void);
	// Initialises this module.  Calling this routine is not strictly speaking
	// necessary because each entry point will perform whatever initialisation
	// it requires, but it can be useful to detect any initialisation errors.
	
extern pascal void TermMoreTextUtils(void);
	// Terminates this module.  This is not necessary for application clients,
	// but very important for non-application (code resources, shared libraries,
	// etc) clients.

extern pascal void MoreReplaceText(Str255 baseText, ConstStr255Param substitutionText, ConstStr15Param key);
	// Works like the Mac OS system call ReplaceText, except that:
	// 
	// o You don’t have to set up thePort before calling this routine.  The
	//   routine will automatically switch to a port configured to use
	//   the system font.  Thus the baseText is assumed to be in the system
	//   script.
	//
	// o For convenience, the parameters are strings rather than handles.

extern pascal ByteCount MoreStrLen(const char *str);
	// strlen for those whose don’t do StdCLib.

#ifdef __cplusplus
	}
#endif