/*
	File:		MoreOSLHelpers.h

	Contains:	Property getters and setters for standard system objects.

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

$Log: MoreOSLHelpers.h,v $
Revision 1.6  2002/11/08 23:48:50         
Include our prototype early to flush out any missing dependencies.

Revision 1.5  2001/11/07 15:54:17         
Tidy up headers, add CVS logs, update copyright.


         <4>     24/9/01    Quinn   Fixes to compile with C++ activated.
         <3>     21/9/01    Quinn   Changes for CWPro7 Mach-O build.
         <2>     20/3/00    Quinn   Added comments.
         <1>      6/3/00    Quinn   First checked in.
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
	#include <MacWindows.h>
#endif

/////////////////////////////////////////////////////////////////

#ifdef __cplusplus
extern "C" {
#endif

// Big Picture
// -----------
// The goal of this module is to provide property getters and setters for
// standard properties of standard system objects (currently the application
// and windows).  A complex application will probably want to reimplement
// most of these routines for itself, but I had to write the code for
// TestMOSL, so I thought I’d share it.

// IMPORTANT:
// These routines assume that they are running on Mac OS 8.5 or later and that
// you’re application is using the Mac OS 8.5 toolbox.  If these are not both
// true, bad things will happen.

extern pascal OSStatus MOSLCApplicationGetPropHelper(DescType propName, AEDesc *result);
	// Provides support for standard application properties.
	//
	// 	  o	keyAEBestType	-- always cApplication
	// 	  o	pName			-- using Process Manager
	// 	  o	pIsFrontProcess	-- using Process Manager
	// 	  o	pVersion		-- from your 'vers' ID=1 resource
	//						   (your resource file must be CurResFile)

extern pascal OSStatus MOSLCWindowGetPropHelper(WindowRef window, DescType propName, 
												Point idealSize,  AEDesc *result);
	// Provides support for getting standard window properties.
	//
	//	  o	pName			-- using classic Window Manager
	//	  o	pIndex			-- ditto
	//	  o	kAESetPosition	-- using Mac OS 8.5 Window Manager (GetWindowBounds)
	//	  o	keyAEBounds		-- ditto
	//	  o	pIsResizable	-- using Mac OS 8.5 Window Manager (GetWindowAttributes)
	//	  o	pIsZoomable		-- ditto
	//	  o	pHasCloseBox	-- ditto
	//	  o	pIsFloating		-- using Mac OS 8.5 Window Manager (GetWindowClass)
	//	  o	pIsModal		-- ditto
	//	  o	pHasTitleBar	-- using Appearance Window Manager (GetWindowRegion)
	//	  o	pIsZoomed		-- using Mac OS 8.5 Window Manager (GetWindowAttributes, IsWindowInStandardState)
	//	  o	pVisible		-- using classic Window Manager
	//	  o	pIsFrontProcess	-- ditto
	//
	// If you use this routine to support the zooming pIsZoomed property,
	// you must pass in the window’s idealSize as you would to ZoomWindowIdeal.
	//
	// Using pIsFrontProcess is a bit of sleaze, but it appears to be a commonly
	// accepted sleaze.
	
extern pascal OSStatus MOSLCWindowSetPropHelper(WindowRef window, DescType propName, 
												Point idealSize,  const AEDesc *data);
	// Provides support for setting standard window properties.
	//
	//	  o	pName			-- using classic Window Manager
	//	  o	kAESetPosition	-- using Mac OS 8.5 Window Manager (Get/SetWindowBounds)
	//	  o	keyAEBounds		-- ditto (and GetWindowAttributes)
	//	  o	pIsZoomed		-- using Mac OS 8.5 Window Manager (GetWindowAttributes, IsWindowInStandardState, ZoomWindowIdeal)
	//	  o	pVisible		-- using classic Window Manager
	//	  o	pIsFrontProcess	-- ditto
	//
	// If you use this routine to support the zooming pIsZoomed property,
	// you must pass in the window’s idealSize as you would to ZoomWindowIdeal.
	
#ifdef __cplusplus
}
#endif
