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
				
	Change History (most recent first):

	Revision 1.6	4/16/2001	updated for UI3.4 and X DTS
				  	2000/11/20	Add gCurrentlyRecording global
	Revision 1.5  	2000/03/06 	Added gOutput to facilitate testing SGOutput APIs	
	Revision 1.4  	2000/03/01  Add more sizes & recording on Idle
	Revision 1.3  	1999/12/15	carbonized
	Revision 1.2  	Original	QTE
*/

#if (TARGET_OS_MAC && TARGET_API_MAC_CARBON)
	#if __APPLE_CC__
		#include <Carbon/Carbon.h>
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

//-----------------------------------------------------------------------
// Globals

Boolean					gQuitFlag = 0;
SeqGrabComponent		gSeqGrabber=0;
SGChannel				gVideoChannel=0;
SGChannel				gSoundChannel=0;
DialogPtr				gDialog=0;
WindowPtr				gMonitor=0;
Rect					gActiveVideoRect;
PicHandle				gMonitorPICT=0;
Boolean					gFullSize;
Boolean					gHalfSize;
Boolean					gQuarterSize;
#if  __MWERKS__
THPrint					gPrintRec;
#endif
ICMAlignmentProcRecord	gSeqGrabberAlignProc;
Boolean					gRecordVideo = 1;
Boolean					gRecordSound = 1;
Boolean					gSplitTracks = 0;
Boolean					gLowLatency = 0;
Boolean					gUseTimeBase = 0;
Boolean					gUseSoundClock = 0;
short					gChosenSizeItem=0;
SGOutput				gOutput=0;
Boolean					gCurrentlyRecording = false;
ComponentDescription 	gSGClockComponentDescription = {0};