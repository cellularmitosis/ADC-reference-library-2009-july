/*
	File:		InterruptSafeDebug.h

	Contains:	Library for logging directly to the screen.

	Written by:	Quinn "The Eskimo!"

	Copyright:	© 1998 by Apple Computer, Inc., all rights reserved.

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

$Log: InterruptSafeDebug.h,v $
Revision 1.5  2002/11/08 23:21:41         
When using framework includes, explicitly include the frameworks we need.

Revision 1.4  2001/11/07 15:50:02         
Tidy up headers, add CVS logs, update copyright.


         <3>     24/9/01    Quinn   Fixes to compile with C++ activated.
         <2>     21/9/01    Quinn   Changes for CWPro7 Mach-O build.
         <1>    23/11/98    Quinn   First checked in.
*/

#pragma once

/////////////////////////////////////////////////////////////////////

#include "MoreSetup.h"

#if MORE_FRAMEWORK_INCLUDES
	#include <ApplicationServices/ApplicationServices.h>
#else
	#include <MacTypes.h>
#endif

/////////////////////////////////////////////////////////////////////

#ifdef __cplusplus
	extern "C" {
#endif

/////////////////////////////////////////////////////////////////////

// WARNING
// Once initialised, this library has a cached copy of the screen
// base, bit depth, and row bytes.  If the screen geometry
// (ie resolution or bit depth) changes, this module is likely to
// start writing over random bits of memory.  You must be sure to
// reinitialise the module before logging any text if the screen
// geometry changes.

/////////////////////////////////////////////////////////////////////

// The following parameter can be twiddled to suit your particular
// environment.

enum {

	kISDebugNumberOfColumns = 64,
	kISDebugNumberOfRows = 32,
		// These values were chosen to fit on the smallest
		// Mac display.  If you want, you can increase the size.
		// Be sure to always make them a power of 2, otherwise
		// the MOD of gCurrentCharIndex by kMaxCharIndex will do
		// weird things.
	
	kISDebugMaxCharIndex = kISDebugNumberOfColumns * kISDebugNumberOfRows,

	kISDebugXPixelOffset = 0,
	kISDebugYPixelOffset = 0
		// You can use these constants to position the debugging
		// output in an appropriate place on the screen for your
		// tests.
};

/////////////////////////////////////////////////////////////////////

extern OSStatus InitInterruptSafeDebug(void);
	// You must call this routine before calling any of the following.
	// You may call this routine more than once, which is useful
	// is the display geometry has changed.
	
extern void ISDebugChar(UInt8 ch);
extern void ISDebugText(const UInt8 *text, ByteCount len);
extern void ISDebugStr (ConstStr255Param str);
extern void ISdebugstr (const char *str);
	// These routines write the supplied parameter directly to
	// the screen, bypassing QuickDraw and the rest of the OS.
	// The routines are completely interrupt-safe, including
	// when paging is unsafe (as long as you can guarantee that 
	// the code and data sections are held resident).

/////////////////////////////////////////////////////////////////////

#ifdef __cplusplus
}
#endif
