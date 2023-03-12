/*
	File:		main.c
	
	Description: Basic RunApplicationEventLoop-based sample code shell, Carbon API

	Copyright: 	© Copyright 2003 Apple Computer, Inc. All rights reserved.
	
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

*/


#ifdef __APPLE_CC__
#include <Carbon/Carbon.h>
#else
#include <Carbon.h>
#endif

#include "main.h"
#include "Utilities.h"
#include "QTUtilities.h"
#include "AppBlit_Component.h"
#include "PlayMovieFile.h"
#include "MungData.h"
#include "MiniMung.h"
#include "ConvertToMovieJr.h"
#include "DoubleMung.h"

#define BailErr(err) {if(err != noErr) goto bail;}

#define kAboutBox				200				// About... alert

#define mFile 					129
#define mDemo					131

#define	iQuitSeperator			10
#define iQuit 					11

#define iMotionDetectSeqGrab	1
#define iAlphaOverlaySeqGrab	2
#define iColorClampSeqGrab		3
#define iFilmNoiseSeqGrab		4
#define iBlendedSeqGrab			5
#define iAlphaOverlayMovieConv	7
#define iColorClampMoviePlayb	9
#define iAlphaOverlayMoviePlayb	10

static const EventTypeSpec	MyAppEventTypes[] =
{
	{ kEventClassCommand, kEventCommandProcess },
	{ kEventClassKeyboard, kEventRawKeyUp }
};

static const EventTypeSpec MyWindowEventTypes[] = 
{ 	
    {kEventClassKeyboard, kEventRawKeyUp},
    {kEventClassWindow, kEventWindowUpdate},
    {kEventClassWindow, kEventWindowDrawContent},
    {kEventClassWindow, kEventWindowActivated},
    {kEventClassWindow, kEventWindowDeactivated},
    {kEventClassWindow, kEventWindowHandleContentClick},
    {kEventClassWindow, kEventWindowClose}
};

//////////
//
// Module variables
//
//////////

static EventHandlerUPP mAppCommandProcess;  	// Command-process event handler
static EventHandlerUPP mWinEvtHandler;			// window event handler
static UInt32		   mCurrentMode;			// current mode of operation
static UInt32		   mPreviousMode;			// previous mode of operation
//////////
//
// Prototypes
//
//////////

static void Initialize();
static void MakeMenu(void);
static void MakeWindow();
static void InstallAppEvents(void);
static pascal OSErr QuitAppleEventHandler(const AppleEvent *appleEvt, AppleEvent* reply, long refcon);
static pascal OSStatus myWinEvtHndlr(EventHandlerCallRef theCallRef, EventRef theEvent, void *theRefCon);
static void DoAboutBox(void);
static pascal OSStatus DoAppCommandProcess(EventHandlerCallRef nextHandler, EventRef theEvent, void* userData);

static void HandleWindowUpdate(WindowRef window, EventRef theEvent);
static void HandleWindowClose();

static void DoPreviousModeCleanup();
static void EraseFrontWindow();

#ifdef CAN_SET_CLAMP_VALUES
static void DrawHelpStringsForColorClampPlayback(Rect movieBounds);
static void DrawHelpStringsForRegularPlayback(Rect movieBounds);
#endif

#ifdef MINI_MUNG
static void DrawHelpStringsForMotionDetectPlayback(Rect movieBounds);
static void DrawDoubleMungHelpStrings();
static void DrawMiniMungHelpStrings(Boolean withOverlay, Boolean withClamp, Boolean withEffect);
#endif

#ifdef PLAY_MOVIE
static void DrawHelpStringsForMoviePlayback(Boolean overlayPlayback);
#endif


//////////
//
// main
//
//////////

int main(void)
{	
    Initialize();
    MakeWindow();
    MakeMenu();
    InstallAppEvents();
    RunApplicationEventLoop();
    return 0;
}

//////////
//
// Initialize
// Basic Initialization stuff.
//
//////////

static void Initialize()
{
	OSErr	err;

	InitCursor();
    
	EnterMovies();
	
	mCurrentMode = 0;
	mPreviousMode = 0;
	
    err = AEInstallEventHandler(kCoreEventClass, 
                                kAEQuitApplication, 
                                NewAEEventHandlerUPP(QuitAppleEventHandler), 
                                0, 
                                false);
	if (err != noErr)
		ExitToShell();
}


//////////
//
// MakeWindow
// Create our window, install window handlers.
//
//////////

static void MakeWindow()
{
    Rect			wRect;
    WindowRef		window = NULL;
    OSStatus		err;
    EventHandlerRef	ref;

    SetRect(&wRect,50,50,50+320 + 520 ,50+480 + 60);
    err = CreateNewWindow(kDocumentWindowClass, 
        (kWindowStandardDocumentAttributes | kWindowStandardHandlerAttribute | kWindowCloseBoxAttribute),
                        &wRect,
                        &window);
    if (window)
    {
        // install Carbon event handlers for this window
        InstallStandardEventHandler(GetWindowEventTarget(window));
    
        mWinEvtHandler = NewEventHandlerUPP(myWinEvtHndlr);
        InstallWindowEventHandler(window, mWinEvtHandler, GetEventTypeCount(MyWindowEventTypes), MyWindowEventTypes, 0, &ref);
        
        ShowWindow(window);
        SetPortWindowPort(window);
        
        ForeColor(blackColor);
        BackColor(whiteColor);
    }
}


//////////
//
// MakeMenu
// Basic menu setup.
//
//////////

static void MakeMenu(void)
{
	Handle		menuBar;
	MenuRef 	menu;
	long 		response;
	OSStatus	err = noErr;

	menuBar = GetNewMBar(128);
	if (menuBar)
	{
		SetMenuBar(menuBar);

		// see if we should modify quit in accordance with the Aqua HI guidelines
		err = Gestalt(gestaltMenuMgrAttr, &response);
		if ((err == noErr) && (response & gestaltMenuMgrAquaLayoutMask))
		{
			menu = GetMenuHandle(mFile);
			DeleteMenuItem(menu, iQuit);
			DeleteMenuItem(menu, iQuitSeperator);
		}
		DrawMenuBar();
	}
	else
    {
		DebugStr("\p MakeMenu failed");
    }

    menu = GetMenuHandle(mDemo);

    // first disable all menu items
    DisableAllMenuItems(menu);
    
#ifdef PLAY_MOVIE
    EnableMenuItem (menu, iColorClampMoviePlayb);
    EnableMenuItem (menu, iAlphaOverlayMoviePlayb);
#endif

#ifdef CONVERT_TO_MOVIE_JR
    EnableMenuItem (menu,iAlphaOverlayMovieConv);
#endif

#ifdef MINI_MUNG
    EnableMenuItem (menu,iMotionDetectSeqGrab);
    EnableMenuItem (menu,iAlphaOverlaySeqGrab);
    EnableMenuItem (menu,iColorClampSeqGrab);
    EnableMenuItem (menu,iFilmNoiseSeqGrab);
    EnableMenuItem (menu,iBlendedSeqGrab);
#endif

#ifdef VIDEOPROCESSING
    // enable _all_ menu items
    EnableAllMenuItems(menu);
#endif
}


//////////
//
// InstallAppEvents
// Install handler for our app events.
//
//////////

static void InstallAppEvents(void)
{
    mAppCommandProcess = NewEventHandlerUPP(DoAppCommandProcess);

    InstallApplicationEventHandler(mAppCommandProcess, GetEventTypeCount(MyAppEventTypes), MyAppEventTypes, (void *)NULL, NULL);
}

//////////
//
// HandleKeyEvent
// Basic key handling.
//
//////////

static void HandleKeyEvent(EventRef theEvent)
{
    char theCharPressed;
    
    GetEventParameter(theEvent, 
                    kEventParamKeyMacCharCodes, 
                    typeChar,
                    NULL, 
                    sizeof(char), 
                    NULL, 
                    &theCharPressed);

    switch (theCharPressed)
    {
#ifdef CAN_SET_CLAMP_VALUES
        case 'r':
        case 'R':
            SetMungDataColorDefaults();
            break;
#endif
        case 'Q':
        case 'q':
        case 'S':
        case 's':
            {
                Rect winBounds;
                
                HandleWindowClose();

                SetPortWindowPort(FrontWindow());
                /* erase window */
                GetPortBounds(GetWindowPort(FrontWindow()), &winBounds);
                EraseRect(&winBounds);
            }
            break;
#ifdef CAN_SET_CLAMP_VALUES
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
            SetCurrentClamp((theCharPressed - '0') - 1);
        break;
        
        case '+':
        case '=':
            IncrementCurrentClamp();
            break;
        case '-':
        case '_':
            DecrementCurrentClamp();
            break;
#endif
        default:

            break;
    }

}

//////////
//
// HandleWindowContentClick
// Handling of window clicks.
//
//////////

static void HandleWindowContentClick(EventRef theEvent)
{
    OSStatus 	myErr;
    Point 		wheresMyMouse;
    
    myErr = GetEventParameter(theEvent, 
                            kEventParamMouseLocation, 
                            typeQDPoint,
                            NULL, 
                            sizeof(Point), 
                            NULL, 
                            &wheresMyMouse);

    GlobalToLocal(&wheresMyMouse);
    if (myErr == noErr)
    {
#ifdef PLAY_MOVIE
        AdjustColorClampEndpoints(wheresMyMouse.h);
#endif

#ifdef MINI_MUNG
        AdjustColorClampEndpoints(wheresMyMouse.h);
#endif

    }
}

//////////
//
// HandleWindowClose
//
//////////

static void HandleWindowClose()
{
#ifdef PLAY_MOVIE
    DoKillMovie();
#endif

#ifdef MINI_MUNG
    KillMiniMungGrab();
	KillDoubleMungGrab();
#endif
}

//////////
//
// myWinEvtHndlr
// Event handler code for our window
//
//////////

static pascal OSStatus myWinEvtHndlr(EventHandlerCallRef theCallRef, EventRef theEvent, void *theRefCon)
{
    #pragma unused(theCallRef,theRefCon)
    UInt32				myClass, myKind;
    WindowRef			window = NULL;
    OSStatus			myErr = eventNotHandledErr;

    GetEventParameter(theEvent, kEventParamDirectObject, typeWindowRef, NULL, sizeof(window), NULL, &window);
    
    myClass = GetEventClass(theEvent);
    myKind = GetEventKind(theEvent);
    
    switch (myClass) 
    {
        case kEventClassKeyboard:
            switch (myKind) 
            {
                case kEventRawKeyUp:
                        HandleKeyEvent(theEvent);
                        myErr = noErr;
                    break;
            }
            break;
            
        case kEventClassWindow:
            switch (myKind) 
            {
                case kEventWindowDrawContent:
                break;
				
                case kEventWindowBoundsChanged:
                {
                    Rect bounds;
                    InvalWindowRect(window, GetWindowPortBounds(window, &bounds));
                    myErr = noErr;
                }
                break;

                case kEventWindowUpdate:
                    HandleWindowUpdate(window, theEvent);
                break;
				
                case kEventWindowActivated:
                case kEventWindowDeactivated:
                    break;
                    
                case kEventWindowHandleContentClick:
                case kEventWindowClickContentRgn:
                    HandleWindowContentClick(theEvent);
                    break;

                case kEventWindowClose:
                        HandleWindowClose();
                
                        DisposeEventHandlerUPP(mWinEvtHandler);
                        DisposeWindow(window);
                                
                        myErr = noErr;
                    break;
                
                default:
                    break;
        }
        break;
    }
    
    return(myErr);
}

//////////
//
// EraseFrontWindow
// Clear out the front window contents
//
//////////

static void EraseFrontWindow()
{
	WindowRef	frontWindowRef = FrontWindow();
	Rect		winRect;
	
	if (frontWindowRef)
	{
		GetWindowPortBounds (frontWindowRef, &winRect);
		EraseRect(&winRect);
	}
}

//////////
//
// DoPreviousModeCleanup
// Before running a new demo mode we must first
// perform cleanup for any previous modes
//
//////////

static void DoPreviousModeCleanup()
{
	switch (mPreviousMode)
	{
#ifdef MINI_MUNG
		case 'this':	// motion detect grab
		case 'thi2':	// alpha overlay grab
		case 'thi3':	// color clamp grab
		case 'thi4':	// film noise grab
				EraseFrontWindow();
				KillMiniMungGrab();
			break;

		case 'thi5':	// blended grab
				EraseFrontWindow();
				KillDoubleMungGrab();
			break;
#endif

#ifdef CONVERT_TO_MOVIE_JR
		case 'that':
				EraseFrontWindow();
			break;
#endif

#ifdef PLAY_MOVIE			
		case 'othe':	/* color clamped movie playback */
		case 'oth2':	/* alpha overlay movie playback */
				EraseFrontWindow();
				DoKillMovie();
			break;
#endif
            
		default:
			break;
	}
	
	mPreviousMode = mCurrentMode;
}

//////////
//
// DoAppCommandProcess
// Handle command-process events at the application level
//
//////////

static pascal OSStatus DoAppCommandProcess(EventHandlerCallRef nextHandler, EventRef theEvent, void* userData)
{
#pragma unused (nextHandler, userData)
	HICommand  aCommand;
	OSStatus   result;

	GetEventParameter(theEvent, kEventParamDirectObject, typeHICommand, NULL, sizeof(HICommand), NULL, &aCommand);

	switch (aCommand.commandID)
	{
		case 'abou':
			DoAboutBox();
			result = noErr; 
			break;
            
#ifdef MINI_MUNG
		case 'this':	// motion detect grab
			mCurrentMode = aCommand.commandID;
			DoPreviousModeCleanup();
			if (noErr == MiniMung(FrontWindow(), false, false, false))
			{
				DrawMiniMungHelpStrings(false, false, false);
			}
			result = noErr; 
			break;

		case 'thi2':	// alpha overlay grab
			mCurrentMode = aCommand.commandID;
			DoPreviousModeCleanup();
			if (noErr == MiniMung(FrontWindow(), true, false, false))
			{
				DrawMiniMungHelpStrings(true, false, false);
			}
			result = noErr; 
			break;

		case 'thi3':	// color clamp grab
			mCurrentMode = aCommand.commandID;
			DoPreviousModeCleanup();
			if (noErr == MiniMung(FrontWindow(), false, true, false))
			{
				DrawMiniMungHelpStrings(false, true, false);
			}
			result = noErr; 
			break;

		case 'thi4':	// film noise grab
			mCurrentMode = aCommand.commandID;
			DoPreviousModeCleanup();
			if (noErr == MiniMung(FrontWindow(), false, false, true))
			{
				DrawMiniMungHelpStrings(false, false, true);
			}
			result = noErr; 
			break;

		case 'thi5':	// blended grab
			mCurrentMode = aCommand.commandID;
			DoPreviousModeCleanup();
			if (noErr == DoubleMung(FrontWindow()))
			{
				DrawDoubleMungHelpStrings();
			}
			result = noErr; 
			break;
#endif

#ifdef CONVERT_TO_MOVIE_JR
		case 'that':
			mCurrentMode = aCommand.commandID;
			DoPreviousModeCleanup();
			ConvertToMovieJr(FrontWindow());
			result = noErr;

			break;
#endif

#ifdef PLAY_MOVIE			
		case 'othe':	/* color clamped movie playback */
		case 'oth2':	/* alpha overlay movie playback */
            {
                Boolean useOverlay = (aCommand.commandID == 'oth2');
				mCurrentMode = aCommand.commandID;
				DoPreviousModeCleanup();
                
                if (noErr == DoPlayMovie(useOverlay))
				{
					DrawHelpStringsForMoviePlayback(useOverlay);
				}
            }
			result = noErr;

			break;
#endif
		case kHICommandQuit:
			QuitApplicationEventLoop();
			result = noErr;
			break;
            
		default:
			result = eventNotHandledErr;
			break;
	}

      HiliteMenu(0);
      return result;
}

//////////
//
// HandleWindowUpdate
//
//////////

static void HandleWindowUpdate(WindowRef window, EventRef theEvent)
{
    GrafPtr 		mySavedPort = NULL;
    GrafPtr 		myWindowPort = NULL;
    WindowPtr		myWindow = NULL;
    Rect			myRect;
    EventRecord 	myEvent;

    GetPort(&mySavedPort);
    
    myWindowPort = (GrafPtr)GetWindowPort(window);
    myWindow = window;
    
    if (myWindowPort != NULL)
    {
        MacSetPort(myWindowPort);
    
        ConvertEventRefToEventRecord(theEvent, &myEvent);
        GetPortBounds(myWindowPort, &myRect);
        BeginUpdate(myWindow);
        
        // ***insert application-specific drawing here***
            
        EndUpdate(myWindow);
        MacSetPort(mySavedPort);
    }
 }

//////////
//
// QuitAppleEventHandler
//
//////////

static pascal OSErr QuitAppleEventHandler(const AppleEvent *appleEvt, AppleEvent* reply, long refcon)
{
#pragma unused (appleEvt, reply, refcon)

	QuitApplicationEventLoop();
    
	return noErr;
}

//////////
//
// DoAboutBox
//
//////////

static void DoAboutBox(void)
{
	Alert(kAboutBox, nil);  // simple alert dialog box
}

#ifdef MINI_MUNG
//////////
//
// DrawMiniMungHelpStrings
//
//////////

static void DrawMiniMungHelpStrings(Boolean withOverlay, Boolean withClamp, Boolean withEffect)
{
    Rect rect;
    
    GetMungDataBoundsRect(&rect);
    
	if (!withOverlay)
    {
        if (withClamp)
        {
            DrawHelpStringsForColorClampPlayback(rect);
        }
        else
        {
            if (withEffect)
            {
                DrawHelpStringsForRegularPlayback(rect);
            }
            else
            {
                DrawHelpStringsForMotionDetectPlayback(rect);
            }
        }
    }
    else
    {
        DrawHelpStringsForRegularPlayback(rect);
    }

}

//////////
//
// DrawDoubleMungHelpStrings
//
//////////

static void DrawDoubleMungHelpStrings()
{
    DrawHelpStringsForRegularPlayback(gMungRect);
}
#endif

#ifdef CAN_SET_CLAMP_VALUES
//////////
//
// DrawHelpStringsForColorClampPlayback
//
//////////

static void DrawHelpStringsForColorClampPlayback(Rect movieBounds)
{
    MoveTo(10, movieBounds.bottom + 20);
    DrawString("\p1-6 = select endpoint");
    MoveTo(10, movieBounds.bottom + 40);
    DrawString("\p<space>, mouse = change endpoint");
    MoveTo(10, movieBounds.bottom + 60);
    DrawString("\pr, R = reset endpoints to default");
    MoveTo(10, movieBounds.bottom + 80);
    DrawString("\pS,Q = stop demo");
}
#endif

#ifdef CAN_SET_CLAMP_VALUES
//////////
//
// DrawHelpStringsForRegularPlayback
//
//////////

static void DrawHelpStringsForRegularPlayback(Rect movieBounds)
{
    MoveTo(10, movieBounds.bottom + 20);
    DrawString("\pS, Q = stop demo");
}
#endif

#ifdef MINI_MUNG
//////////
//
// DrawHelpStringsForMotionDetectPlayback
//
//////////

static void DrawHelpStringsForMotionDetectPlayback(Rect movieBounds)
{
    MoveTo(10, movieBounds.bottom+20);
    DrawString("\pNow scanning for lobsters…");
    MoveTo(10, movieBounds.bottom+40);
    DrawString("\pS,Q = halt search.");
}
#endif

#ifdef PLAY_MOVIE
//////////
//
// DrawHelpStringsForMoviePlayback
//
//////////

static void DrawHelpStringsForMoviePlayback(Boolean overlayPlayback)
{
    Rect movieBounds;
    GetMungDataBoundsRect(&movieBounds);

    if (!overlayPlayback)	// do color clamp playback
    {
        DrawHelpStringsForColorClampPlayback(movieBounds);
    }
    else
    {
        DrawHelpStringsForRegularPlayback(movieBounds);
    }

}
#endif
