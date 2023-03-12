/*
	File:		MorePreferences.h

	Contains:	implements collection-based preferences container;
				supports both memory and disk

	Written by:	Pete Gontier

	Copyright:	Copyright (c) 1998-2001 by Apple Computer, Inc., All Rights Reserved.

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

$Log: MorePreferences.h,v $
Revision 1.8  2003/03/28 16:15:18         
Removed a bunch of #pragma import/export directives.  IIRC PCG added them for reasons I don't fully understand.  He may have meant #pragma internal.  Regardless, there presence was causing lots of MIB symbols to be CFM exported, which is wrong.  MIB is not meant to ship as a shared library.

Revision 1.7  2002/11/08 23:55:11         
When using framework includes, explicitly include the frameworks we need. Convert nil to NULL.

Revision 1.6  2001/11/07 15:54:44         
Tidy up headers, add CVS logs, update copyright.


         <5>     21/9/01    Quinn   Changes for CWPro7 Mach-O build.
         <4>    22/11/00    Quinn   Relatively minor API additions and changes.
         <3>     1/22/99    PCG     dunno
         <2>    11/11/98    PCG     fix headers
         <1>    11/10/98    PCG     first big re-org at behest of Quinn

	Old Change History (most recent first):

         <6>    10/11/98    Quinn   Convert "MorePrefix.h" to "MoreSetup.h".
         <5>     11/9/98    PCG     fix comment 4
         <4>     11/9/98    PCG     add copyright blurb and OpenPrefsResourceFile
         <3>      9/9/98    PCG     re-work import and export pragmas
         <2>     7/24/98    PCG	    coddle linker (C++, CFM-68K)
         <1>     6/16/98    PCG     initial checkin
*/

#pragma once

#include "MoreSetup.h"

#if MORE_FRAMEWORK_INCLUDES
	#include <ApplicationServices/ApplicationServices.h>
#else
	#include <Files.h>
#endif

enum
{
	kPrefsReservedType		= 'prrt',
	kPrefsFileType			= 'pref'
};

typedef FourCharCode	PrefsType;
typedef UInt32			PrefsID;

typedef struct OpaquePrefsCollection *PrefsCollection;

#ifdef __cplusplus
	extern "C" {
#endif

	//
	// Below are optional functions to call before reading or writing preferences files.
	//

pascal OSStatus GetPrefsFileCreator    (OSType *);
pascal OSStatus SetPrefsFileCreator    (OSType);
pascal OSStatus SetPrefsFolderName     (ConstStringPtr folderName);
pascal OSStatus SetPrefsFileName       (ConstStringPtr fileName);

	//
	// Below are functions for reading preferences collections; you probably want to use
	// one or the other of these functions; use of both should be relatively uncommon.
	//
	// If the fileName passed to GetNewPrefsCollection is NULL, MorePreferences uses the
	// name last passed to SetPrefsFileName. In this case, if no file name has been
	// passed to SetPrefsFileName, GetNewPrefsCollection returns paramErr.
	//

pascal OSStatus GetNewPrefsCollection         (PrefsCollection *, ConstStringPtr fileName);
pascal OSStatus GetNewPrefsCollectionFromFile (PrefsCollection *, const FSSpec *);

	//
	// Below are functions for writing preferences collections; you probably want to use
	// one or the other of these functions; use of both should be relatively uncommon.
	//
	// If the last parameter is NULL, the function operates on a preferences collection
	// whose lifetime is maintained internally by MorePreferences.
	//
	// If the fileName passed to WritePrefsCollection is NULL, MorePreferences uses the
	// name last passed to SetPrefsFileName. In this case, if no file name has been
	// passed to SetPrefsFileName, GetNewPrefsCollection returns paramErr.
	//

pascal OSStatus WritePrefsCollection        (PrefsCollection, ConstStringPtr fileName);
pascal OSStatus WritePrefsCollectionToFile  (PrefsCollection, const FSSpec *);

	//
	// Below are functions for creating and destroying prefs collections.
	// Passing NULL produces an error.
	//

pascal OSStatus NewPrefsCollection       (PrefsCollection *);
pascal OSStatus DisposePrefsCollection   (PrefsCollection);

	//
	// Below are functions for manipulating preferences collections.
	//
	// If the last parameter is NULL, the function operates on a preferences collection whose
	// lifetime is maintained internally by MorePreferences. If you choose to take advantage
	// of this short-hand, remmeber to write the prefs when your app quits. 
	//

pascal OSStatus GetPreferenceSize (PrefsCollection, PrefsType, PrefsID, Size *);
pascal OSStatus GetPreference     (PrefsCollection, PrefsType, PrefsID, Size, Size *actualSize, void *);
pascal OSStatus SetPreference     (PrefsCollection, PrefsType, PrefsID, Size, void *);

	//
	// Below are preference utility functions.
	//

pascal OSStatus PrefsAreSame     (PrefsCollection, PrefsCollection, Boolean *);

#ifdef __cplusplus
	}
#endif
