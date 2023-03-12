/*
	File:		MoreWindows.h

	Contains:	Window Manager utilities.

	Written by:	Pete Gontier

	Copyright:	Copyright (c) 1998-2001 by Apple Computer, Inc., All Rights Reserved.

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

$Log: MoreWindows.h,v $
Revision 1.12  2003/03/28 16:15:42         
Removed a bunch of #pragma import/export directives.  IIRC PCG added them for reasons I don't fully understand.  He may have meant #pragma internal.  Regardless, there presence was causing lots of MIB symbols to be CFM exported, which is wrong.  MIB is not meant to ship as a shared library.

Revision 1.11  2002/11/09 00:15:38         
When using framework includes, explicitly include the frameworks we need.

Revision 1.10  2001/11/07 15:56:08         
Tidy up headers, add CVS logs, update copyright.


         <9>     21/9/01    Quinn   Get rid of wacky Finder label.
         <8>     21/9/01    Quinn   Changes for CWPro7 Mach-O build.
         <7>     20/9/01    Quinn   Upgrade to CWPro7.
         <6>    22/12/00    Quinn   Added a bunch of new APIs.
         <5>     20/3/00    Quinn   Added GetWindowList for non-Carbon clients.
         <4>     1/25/99    PCG     TARGET_CARBON
         <3>    11/11/98    PCG     fix headers
         <2>    11/11/98    PCG     fix header
         <1>    11/10/98    PCG     first big re-org at behest of Quinn

	Old Change History (most recent first):

         <4>    10/11/98    Quinn   Convert "MorePrefix.h" to "MoreSetup.h".
         <3>      9/9/98    PCG     re-work import and export pragmas
         <2>     7/24/98    PCG	    coddle linker (C++, CFM-68K)
         <1>     6/16/98    PCG     initial checkin
*/

#pragma once

#include "MoreSetup.h"

#if MORE_FRAMEWORK_INCLUDES
	#include <Carbon/Carbon.h>
#else
	#include <MacWindows.h>
#endif

#ifdef __cplusplus
	extern "C" {
#endif

pascal OSStatus		MoveWindowToAlertPosition	(WindowRef);

pascal OSErr		MoreNewWindow				(	const Rect *			boundsRect,
													ConstStr255Param 		title,
													short 					theProc,
													Boolean 				goAwayFlag,
													long 					refCon,
													WindowRef				*window				);

#if !TARGET_API_MAC_CARBON

	extern pascal WindowRef GetWindowList(void);

#endif

extern pascal Boolean WindowHasEmbeddingHierarchy(WindowPtr window);
	// Returns true if the window has a control embedding hierarchy.

extern pascal OSStatus MoreGetWindowRegion(WindowRef window, WindowRegionCode inRegionCode, RgnHandle ioWinRgn);
	// A version of MoreGetWindowRegion that supports an inRegionCode 
	// of kWindowStructureRgn or kWindowContentRgn on all platforms, 
	// not just 8.5 and higher.

extern pascal RgnHandle MoreGetWindowContentRegion  (WindowRef window);
extern pascal RgnHandle MoreGetWindowStructureRegion(WindowRef window);
extern pascal RgnHandle MoreGetWindowVisibleRegion  (WindowRef window);
	// These routines all return a region handle for one of the windows 
	// well known regions.  These routines are not reentrant.  Each routine 
	// has a global variable in which it returns the handle, and thus calling 
	// the routine a second time will invalidate the results of the first call.  
	// You are expected to either stop using the handle immediately or 
	// make a copy of it for your own purposes.
	//
	// While this API is a little cheesy it makes it a lot easier to 
	// port code that was just looking at the regions in the WindowRecord.
	// The client does not have to allocate memory, or dispose memory 
	// that this module allocated.

extern pascal Boolean MoreTitleBarOnScreen(WindowRef window);
	// This routine returns true if the window�s title bar region 
	// in on the screen (any screen).  It is useful when restoring 
	// a window�s position from a save preference file.  If the title 
	// bar isn�t somewhere on screen you have to reposition the window 
	// so that the user can at least see it.

extern pascal void MoreGetWindowGlobalRect(WindowRef theWindow, Rect *windowRect);
	// This routine returns, in windowRect, the bounds of theWindow 
	// context region in global coordinates.  It is useful when saving 
	// the window position in a preferences file.

#ifdef __cplusplus
	}
#endif
