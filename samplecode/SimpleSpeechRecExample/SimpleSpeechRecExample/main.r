/*
SimpleSpeechRecExample
A Simple Carbon Application Using Speech Recognition
	
The SimpleSpeechRecExample program demonstrates how to use the Speech Recognition Manager in a simple application.  This program listens for four phrases: "Hi", "Goodbye", "What time is it", and "What day is it".  When it recognizes one of the phrases, it displays the phrase in the dialog.

The Speech Recognition code is contained in the four routines at the end of the file: InitAndStartSpeechRecognition, MakeALanguageModel, HandleSpeechDoneAppleEvent, and CleanupSpeechRecognitionStuff. The rest this file contains classic Macintosh event and dialog handling code to support these Speech Recognition routines.
	
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

	Copyright © 1994-2001 Apple Computer, Inc., All Rights Reserved


*/

#include "Carbon.r"

/* These define's are used in the MENU resources to disable specific
   menu items. */
#define AllItems	0b1111111111111111111111111111111	/* 31 flags */
#define NoItems		0b000000
#define MenuItem1	0b000001
#define MenuItem2	0b000010
#define MenuItem3	0b000100
#define MenuItem4	0b001000
#define MenuItem5	0b010000
#define MenuItem6	0b100000


resource 'MENU' (1000, "Apple", preload) {
	1000, textMenuProc,
	AllItems & ~MenuItem2,	/* Disable item #2 */
	enabled, apple,
	{
		"About SimpleSpeechRecExample",
			noicon, nokey, nomark, plain;
		"-",
			noicon, nokey, nomark, plain
	}
};


resource 'DLOG' (1000, "AboutBox") {
	{0, 0, 150, 400},
	dBoxProc, visible, noGoAway, 0x0, 1000, "", centerMainScreen
};

resource 'DITL' (1000) {
	 {
/* 1 */ {118, 330, 138, 388},
		button {
			enabled,
			"OK"
		};
/* 2 */ {12, 12, 110, 388},				/* SourceLanguage Item */
		staticText {
			disabled,
			"Sample Carbon Application using Speech Recognition.\n\n"
			"Version 1.1"
		}
	}
};


/* The SIZE resource */

resource 'SIZE' (-1) {
	saveScreen,
	acceptSuspendResumeEvents,
	disableOptionSwitch,
	canBackground,
	needsActivateOnFGSwitch,	// this says we do our own activate/deactivate; don't fake us out 
	backgroundAndForeground,	// this is definitely note a background-only application! 
	dontGetFrontClicks,			// change this is if you want "do first click" behavior like the Finder
	ignoreAppDiedEvents,		// essentially, I'm not a debugger (sub-launching)
	is32BitCompatible,			// this app should not be run in 32-bit address space
	isHighLevelEventAware,
	onlyLocalHLEvents,
	notStationeryAware,
	dontUseTextEditServices,
	notDisplayManagerAware,
	reserved,
	reserved,
	1512 * 1024,	// Preferred Size												
	1365 * 1024		// Minimum Size					
								
};
