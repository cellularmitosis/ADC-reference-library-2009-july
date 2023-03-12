/*

File: EatClosedCaption.h

Abstract: Basic implementation of a QuickTime Movie Import component

Version: 1.0

Disclaimer: IMPORTANT:  This Apple software is supplied to you by 
Apple Inc. ("Apple") in consideration of your agreement to the
following terms, and your use, installation, modification or
redistribution of this Apple software constitutes acceptance of these
terms.  If you do not agree with these terms, please do not use,
install, modify or redistribute this Apple software.

In consideration of your agreement to abide by the following terms, and
subject to these terms, Apple grants you a personal, non-exclusive
license, under Apple's copyrights in this original Apple software (the
"Apple Software"), to use, reproduce, modify and redistribute the Apple
Software, with or without modifications, in source and/or binary forms;
provided that if you redistribute the Apple Software in its entirety and
without modifications, you must retain this notice and the following
text and disclaimers in all such redistributions of the Apple Software. 
Neither the name, trademarks, service marks or logos of Apple Inc. 
may be used to endorse or promote products derived from the Apple
Software without specific prior written permission from Apple.  Except
as expressly stated in this notice, no other rights or licenses, express
or implied, are granted by Apple herein, including but not limited to
any patent rights that may be infringed by your derivative works or by
other works in which the Apple Software may be incorporated.

The Apple Software is provided by Apple on an "AS IS" basis.  APPLE
MAKES NO WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION
THE IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS
FOR A PARTICULAR PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND
OPERATION ALONE OR IN COMBINATION WITH YOUR PRODUCTS.

IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL
OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION,
MODIFICATION AND/OR DISTRIBUTION OF THE APPLE SOFTWARE, HOWEVER CAUSED
AND WHETHER UNDER THEORY OF CONTRACT, TORT (INCLUDING NEGLIGENCE),
STRICT LIABILITY OR OTHERWISE, EVEN IF APPLE HAS BEEN ADVISED OF THE
POSSIBILITY OF SUCH DAMAGE.

Copyright (C) 2007 Apple Inc. All Rights Reserved.

*/


#ifndef _EatClosedCaption_
#define _EatClosedCaption_

#if PRAGMA_ONCE
	#pragma once
#endif

#include <QuickTime/QuickTime.h>

#ifdef __cplusplus
	extern "C" {
#endif
	
#if PRAGMA_IMPORT
	#pragma import on
#endif
	
#if PRAGMA_STRUCT_ALIGN
	#pragma options align=mac68k
#elif PRAGMA_STRUCT_PACKPUSH
	#pragma pack(push, 2)
#elif PRAGMA_STRUCT_PACK
	#pragma pack(2)
#endif
	

// Component globals
typedef struct
{
	ComponentInstance	self;
	
	Fixed				width;				// Width of new track
	Fixed				height;				// Height of new track
	TimeScale			timescale;			// Timescale of new media
	
	DataHandler			dataHandler;		// For the source file
	
	Movie				movie;				// Import target movie
	Handle				targetDataRef;
	OSType				targetDataRefType;
	
	Track				closedCaptionTrack;	// Import target track
	Track				timeCodeTrack;		// Associated timecode (if supported by source format)
	
	Media				closedCaptionMedia;	// Import target media
	Media				timeCodeMedia;		// Associated timecode
	
	Boolean				editingClosedCaption;	// Have we called BeginMediaEdits?
	Boolean				editingTimeCode;		// Have we called BeginMediaEdits?
	
	void				*parserGlobals;		// Source format specific globals
}
EatClosedCaptionGlobalsRecord, *EatClosedCaptionGlobals;


#if PRAGMA_STRUCT_ALIGN
	#pragma options align=reset
#elif PRAGMA_STRUCT_PACKPUSH
	#pragma pack(pop)
#elif PRAGMA_STRUCT_PACK
	#pragma pack()
#endif

#ifdef PRAGMA_IMPORT_OFF
	#pragma import off
#elif PRAGMA_IMPORT
	#pragma import reset
#endif

#ifdef __cplusplus
	}
#endif

#endif // _EatClosedCaption_