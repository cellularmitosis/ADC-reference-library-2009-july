/*
	File:		MacShell.c
	
	Description: Simple mac shell which calls each sample

	Author:		QuickTime Engineering, DTS

	Copyright: 	© Copyright 1999 - 2005 Apple Computer, Inc. All rights reserved.
	
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
				
	Change History (most recent first): <2> 4/22/02 released as dts sample code
										<1> 4/1/99 initial release

*/

#include "MacShell.h"

#ifndef __APPLE_CC__
    #include <SIOUX.h>
#endif

// globals
// ------------------------
BitMap screenBits;
Boolean gDone = false;
WindowPtr window = NULL;

//
//	doAEQuit
//
static pascal OSErr doAEQuit(const AppleEvent *message, AppleEvent *reply, long refcon)
{
#pragma unused(message, reply, refcon)
	gDone = true;
	return noErr;
}

static void doMenu( long menuSelection )
{
	short whichMenu = HiWord(menuSelection);
	short whichMenuItem = LoWord(menuSelection);
	
	switch (whichMenu) {
	case kAppleMenuID:
		switch (whichMenuItem) {
		case kAppleMenuAbout:
			Alert(128, NULL);
			break;

		default:
			break;
		}
		break;

	case kFileMenuID:
		switch (whichMenuItem) {
		case kFileMenuQuit:
			gDone = true;
			break;
		}
		break;
	
	case kDemoMenuID:
		if (window) { DisposeWindow( window ); window = NULL; };
		switch (whichMenuItem) {
		case kDemoMenuDraw: DrawImage(); break;
		case kDemoMenuScaleRotate: ScaleAndRotate(); break;
		case kDemoMenuAlpha: AlphaComposite(); break;
		case kDemoMenuMoreInfo: GetMoreInfo(); break;
		case kDemoMenuMultipleImage: MultipleImage(); break;
		case kDemoMenuURLImage: ImageFromURL(); break;
		case kDemoMenuFiltersExport: FilterExport(); break;
		case kDemoMenuMovieImage: MovieToImage(); break;
		case kDemoMenuDeepImages: DeepImages(); break;
        case kDemoMenuDrawCMYK: DrawCMYK(); break;
        case kDemoMenuDrawUsingCGImage: DrawUsingCGImage(); break;
        case kDemoMenuExportFromCGBitmapContext: ExportFromCGBitmapContext(); break;
		default:
			break;
		} // switch
	}
	
	HiliteMenu(0);
}

void pause( void )
{
	EventRecord event;
	Str255 saveTitle;
	
	if( FrontWindow() ) {
		GetWTitle( FrontWindow(), saveTitle );
		SetWTitle( FrontWindow(), "\p(paused)" );
	}
	
	do {
		// wait
	} while( false == WaitNextEvent( mDownMask + keyDownMask, &event, 0, NULL ) );
	
	if( FrontWindow() )
		SetWTitle( FrontWindow(), saveTitle );
}

Boolean IsQuickTimeInstalled(void) 
{
	OSErr	err;
	long	lResult;

	err = Gestalt(gestaltQuickTime, &lResult);
	return (err == noErr);
}

int main( void )
{
	long	result = 0;
	
	// initialize for carbon & QuickTime
	InitCursor();
	if ( IsQuickTimeInstalled() )
		EnterMovies();
	else
		goto bail;
		
	GetQDGlobalsScreenBits( &screenBits );
	
	Gestalt(gestaltMenuMgrAttr, &result);
	if (result & gestaltMenuMgrAquaLayoutMask) {
		// Mmmmmm...X
		SetMenuBar(GetNewMBar(129));
		if (AEInstallEventHandler( kCoreEventClass, kAEQuitApplication, NewAEEventHandlerUPP(doAEQuit), 0, false))
			goto bail;
	} else {
		SetMenuBar(GetNewMBar(128));
	}
	DrawMenuBar();
	
	while (gDone == false) {
		EventRecord theEvent;
		WindowPtr pWhichWindow;		
		short windowPart;
        Boolean SIOUXHandledEvent = false;
		
		WaitNextEvent(everyEvent, &theEvent, 0, NULL);
		
        #ifndef __MACH__
            SIOUXHandledEvent = SIOUXHandleOneEvent(&theEvent);
        #endif
        
		if ( !SIOUXHandledEvent ) {
		
			switch (theEvent.what) {
				case updateEvt:
					pWhichWindow = (WindowPtr)theEvent.message;
					
					// we don't do anything for this simple sample
					BeginUpdate(pWhichWindow);
					EndUpdate(pWhichWindow);
					break;
				
				case keyDown:
					if (theEvent.modifiers & cmdKey) {
						doMenu(MenuKey(theEvent.message & charCodeMask));
					}
					break;
				
				case mouseDown:
					windowPart = FindWindow(theEvent.where, &pWhichWindow);

					switch (windowPart) {
						case inDrag:
							DragWindow(pWhichWindow, theEvent.where, &screenBits.bounds);
							break;

						case inGoAway:
							if (TrackGoAway(pWhichWindow, theEvent.where))
								DisposeWindow( pWhichWindow );
								//gDone = true;
							break;

						case inContent:
							SelectWindow(pWhichWindow);
							break;

						case inMenuBar:
							doMenu(MenuSelect(theEvent.where));
							break;
					}
					break;
			}
		}
	}
	
bail:

	return 0;	
}
