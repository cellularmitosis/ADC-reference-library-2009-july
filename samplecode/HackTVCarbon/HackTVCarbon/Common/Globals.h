/*
	File:		Globals.c
	
	Description: HackTV application globals

	Author:		QuickTime Engineering

	Copyright: 	© Copyright 1992-2001 Apple Computer, Inc. All rights reserved.
	
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
*/

#ifndef _APP_GLOBALS_
#define _APP_GLOBALS_

#if (TARGET_OS_MAC && TARGET_API_MAC_CARBON)
	#if __APPLE_CC__
		#include <Carbon/Carbon.h>
        #include <QuickTime/QuickTime.h>
	#elif __MWERKS__
		#include <Carbon.h>
		#include <QuickTimeComponents.h>
		#include <Printing.h>
	#else
        #error "I'm confused!"
    #endif	
#else
	#include <QTML.h>
	#include <Menus.h>
	#include <Printing.h>
	#include <QuickTimeComponents.h>
#endif

//----------------------------------------------------------------------
// Defines

// Dialog IDs
enum
{
	kAboutDLOGID = 128,
	kMonitorDLOGID,
	kMovieHasBeenRecordedAlertID
};

//-----------------------------------------------------------------------
// Globals
extern Boolean					gQuitFlag;
extern SeqGrabComponent			gSeqGrabber;
extern SGChannel				gVideoChannel;
extern SGChannel				gSoundChannel;
extern DialogPtr				gDialog;
extern WindowPtr				gMonitor;
extern Rect						gActiveVideoRect;
extern PicHandle				gMonitorPICT;
extern Boolean					gFullSize;
extern Boolean					gHalfSize;
extern Boolean					gQuarterSize;
#if  __MWERKS__
extern THPrint					gPrintRec;
#endif
extern ICMAlignmentProcRecord	gSeqGrabberAlignProc;
extern Boolean					gRecordVideo;
extern Boolean					gRecordSound;
extern Boolean					gSplitTracks;
extern Boolean					gLowLatency;
extern Boolean					gUseTimeBase;
extern Boolean					gUseSoundClock;
extern short					gChosenSizeItem;
extern SGOutput					gOutput;
extern Boolean 					gCurrentlyRecording;
extern ComponentDescription 	gSGClockComponentDescription;
#endif