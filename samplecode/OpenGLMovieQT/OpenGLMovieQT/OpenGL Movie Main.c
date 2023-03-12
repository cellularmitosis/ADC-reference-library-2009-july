/*
	File:		OpenGL Moov Main.c

	Contains:	Main event handling and setup for gl moov demo

	Copyright:	2000 Apple Computer, Inc., All Rights Reserved

	Change History (most recent first):
        <5+>     2/24/01    ggs     fix window handling for full screen
         <5>     2/24/01    ggs     New quit event handler
         <4>     12/4/00    ggs     Fixed Quit Event Handler
         <3>    11/25/00    ggs     fixed non-Carbon parts
         <2>    11/25/00    ggs     Completed options and dialog
         <1>    10/29/00    ggs     Name change

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


// system includes ----------------------------------------------------------

#ifdef __APPLE_CC__
    #include "Carbon Include.h"
    #include <Carbon/Carbon.h>
    
    #include <AGL/agl.h>
    #include <OpenGL/gl.h>
#else
    #include <Devices.h>
    #include <Dialogs.h>
    #include <DriverServices.h>
    #include <Events.h>
    #include <LowMem.h>
    #include <TextEdit.h>
    #include <ToolUtils.h>
    #include <Windows.h>
    
    // profile
    #if __profile__
    #include <Profiler.h>
    #endif

    #include <agl.h>
    #include <gl.h>
#endif

#include <math.h>
#include <stdio.h>
//#include <string.h>

// project includes ---------------------------------------------------------

#include "OpenGLMovie.h"

// prototypes ---------------------------------------------------------------

void InitToolbox(void);
Boolean SetUp (void);
void DoMenu (SInt32 menuResult);
void DoKey (SInt8 theKey, SInt8 theCode);
void DoUpdate (void);

#if TARGET_API_MAC_CARBON
pascal void IdleTimer (EventLoopTimerRef inTimer, void* userData);
EventLoopTimerUPP GetTimerUPP (void);
#endif

void DoButton (void);
void DoEvent (void);
void CleanUp (void);

// profile wrappers
Boolean WaitNextEventWrapper (short eventMask, EventRecord *theEvent, unsigned long sleep,RgnHandle mouseRgn);
void UpdateWrapper (EventRecord *theEvent);


// statics/globals (internal only) ------------------------------------------

// Menu defs
enum 
{
	kNumRes = 10,
	kNumFreqs = 11,

	kMenuApple = 128,
	kMenuFile = 129,
	
	kAppleAbout = 1,
	kFileQuit = 1
};

#if TARGET_API_MAC_CARBON
enum 
{
	kForegroundSleep = 10,
	kBackgroundSleep = 10000
};
EventLoopTimerRef gTimer = NULL;
#else
enum 
{
	kForegroundSleep = 0,
	kBackgroundSleep = 100
};
#endif

SInt32 gSleepTime = kForegroundSleep;
Boolean gDone = false, gfFrontProcess = true;

// profile
#if __profile__
OSErr gProfErr = noErr;
#endif	

// functions (internal/private) ---------------------------------------------

static pascal OSErr QuitAppleEventHandler( const AppleEvent *appleEvt, AppleEvent* reply, SInt32 refcon )
{
	#pragma unused (appleEvt, reply, refcon)
	gDone =  true;
	return false;
}

// --------------------------------------------------------------------------

void InitToolbox(void)
{
	OSErr err;
	long response;
	MenuHandle menu;
	
#if !TARGET_API_MAC_CARBON
	MaxApplZone ();

	InitGraf((Ptr) &qd.thePort);
	InitFonts();
	InitWindows();
	InitMenus();
	TEInit();
	InitDialogs(nil);
#endif

	InitCursor();
	
// profile
#if __profile__
// 	prototype:
//		ProfilerInit (collection method, time base, num funcs, stack depth)
	// default call
	gProfErr = ProfilerInit (collectDetailed, bestTimeBase, 20, 5); // set up profiling
	// something that you may need to do (may need more memory)
//	gProfErr = ProfilerInit (collectSummary, bestTimeBase, 1000, 100); // set up profiling
// Note: application will likely run slower, but still should be useful info
	if (noErr == gProfErr)
		ProfilerSetStatus(FALSE);
#endif	
	
#if !TARGET_API_MAC_CARBON
	qd.randSeed =  TickCount(); // ? Random()
#endif

	// Init Menus
	menu = NewMenu (kMenuApple, "\p\024");			// new  apple menu
	InsertMenu (menu, 0);							// add menu to end

#if !TARGET_API_MAC_CARBON
	AppendResMenu(menu, 'DRVR');
#endif
	
	// add quit if not under Mac OS X
	err = Gestalt (gestaltMenuMgrAttr, &response);
	if ((err == noErr) && !(response & gestaltMenuMgrAquaLayoutMask))
	{
			menu = NewMenu (kMenuFile, "\pFile");			// new menu
			InsertMenu (menu, 0);							// add menu to end
			AppendMenu (menu, "\pQuit/Q"); 					// add quit
	}

	DrawMenuBar();
	err = AEInstallEventHandler( kCoreEventClass, kAEQuitApplication, NewAEEventHandlerUPP(QuitAppleEventHandler), 0, false );
	if (err != noErr)
		ExitToShell();
		
	InitCursor ();
} 

// --------------------------------------------------------------------------

Boolean SetUp (void)
{
	InitToolbox ();

// profile
#if __profile__
	if (noErr == gProfErr)
		ProfilerSetStatus (TRUE); // turn on profiling
#endif	

	GetSettings ();
	if (noErr == SetupGLMovie ())
		StartGLMovie ();
	else
		return false;

#if TARGET_API_MAC_CARBON
	// now that we are set up go ahead and start drawing
	InstallEventLoopTimer (GetCurrentEventLoop(), 0, 0.000001, GetTimerUPP (), 0, &gTimer);
#endif	

// profile
#if __profile__
	if (noErr == gProfErr)
		ProfilerSetStatus (FALSE); // turn on profiling
#endif	
	
	return true;
}

// --------------------------------------------------------------------------

void DoMenu (SInt32 menuResult)
{
#if !TARGET_API_MAC_CARBON
	Str255 daName;
#endif
	SInt16 theMenu;
	SInt16 theItem;
	MenuRef theMenuHandle;
		
	theMenu = HiWord(menuResult);
	theItem = LoWord(menuResult);
	theMenuHandle = GetMenuHandle(theMenu);

	switch (theMenu)
	{
		case kMenuApple:
			switch (theItem)
			{
				case kAppleAbout:
					break;
				default:
#if !TARGET_API_MAC_CARBON
					GetMenuItemText (theMenuHandle, theItem, daName);
					OpenDeskAcc(daName);
#endif
					break;
			}
			break;
		case kMenuFile:
			switch (theItem)
			{
				case kFileQuit:
					gDone = true;
					break;
			}
			break;
	}
	HiliteMenu(0);
	DrawMenuBar();
}

// --------------------------------------------------------------------------

void DoKey (SInt8 theKey, SInt8 theCode)
{
	#pragma unused (theCode, theKey)
	// do nothing
}

// --------------------------------------------------------------------------

void DoUpdate (void)
{
	DrawGLMovieFrame ();
	if (GLMovieDone ())
		EndGLMovie ();
}

// --------------------------------------------------------------------------

void UpdateWrapper (EventRecord *theEvent)
{
	WindowRef whichWindow;
	GrafPtr pGrafSave;
	
	whichWindow = (WindowRef) theEvent->message;
	GetPort (&pGrafSave);
	SetPort((GrafPtr) GetWindowPort(whichWindow));
	BeginUpdate(whichWindow);
	DoUpdate();
	SetPort((GrafPtr) GetWindowPort(whichWindow));
	EndUpdate(whichWindow);
	SetPort (pGrafSave);
}

// --------------------------------------------------------------------------

#if TARGET_API_MAC_CARBON
pascal void IdleTimer (EventLoopTimerRef inTimer, void* userData)
{
	#pragma unused (inTimer, userData)
	DoUpdate ();
}

// --------------------------------------------------------------------------

EventLoopTimerUPP GetTimerUPP (void)
{
	static EventLoopTimerUPP	sTimerUPP = NULL;
	
	if (sTimerUPP == NULL)
		sTimerUPP = NewEventLoopTimerUPP (IdleTimer);
	
	return sTimerUPP;
}
#endif

// --------------------------------------------------------------------------

void DoButton (void)
{
	DrawGLMovieFrame ();
	if (GLMovieDone () || Button ())
	{
		gDone = true;
		EndGLMovie ();
	}
}

// --------------------------------------------------------------------------

Boolean WaitNextEventWrapper (short eventMask, EventRecord *theEvent, unsigned long sleep,RgnHandle mouseRgn)
{
	return WaitNextEvent (eventMask, theEvent, sleep, mouseRgn);
}

// --------------------------------------------------------------------------

void DoEvent (void)
{
	EventRecord theEvent;
	Rect rectGrow;
	SInt32 menuResult;
	WindowRef whichWindow = NULL;
	long grow;
	SInt16 whatPart;
	SInt8 theKey;
	SInt8 theCode;
	Boolean fProcessed = false;
	
	if (WaitNextEventWrapper (everyEvent, &theEvent, gSleepTime, NULL))
	{
		fProcessed = ProcessFullScreenEvent (&theEvent);
		if (!fProcessed)
		{
			switch (theEvent.what)
			{
				case mouseDown:
					whatPart = FindWindow(theEvent.where, &whichWindow);
//					if (whichWindow != NULL)
//						SelectWindow (whichWindow);
					switch (whatPart)
					{
						case inGoAway:
							break;
						case inMenuBar:
							DrawMenuBar();
							menuResult = MenuSelect(theEvent.where);
							if (HiWord(menuResult) != 0)
								DoMenu(menuResult);
							break;
						case inDrag:
						{
#if !TARGET_API_MAC_CARBON
								DragWindow (whichWindow, theEvent.where, &(**LMGetGrayRgn()).rgnBBox);
#else
							{
								BitMap	screenBits;
								DragWindow(whichWindow, theEvent.where, &GetQDGlobalsScreenBits(&screenBits)->bounds);
							}
#endif
						}
							break;
						case inGrow:
						{
							SetRect (&rectGrow, 100, 100, 20000, 20000);
							grow = GrowWindow (whichWindow, theEvent.where, &rectGrow);
							if (grow)
							{
								SizeWindow (whichWindow, grow & 0x0000FFFF, grow >> 16, true);
								// do content stuff here
								SetPort((GrafPtr) GetWindowPort(whichWindow));
#if !TARGET_API_MAC_CARBON
								InvalRect (&whichWindow->portRect);				// redraw all
#else
								{
									Rect tempRect;
									InvalWindowRect(whichWindow, GetWindowPortBounds(whichWindow, &tempRect));
								}		
#endif
								UpdateGLMovieForGrow (grow & 0x0000FFFF, grow >> 16);
							}
							break;
						}
						case inSysWindow:
#if !TARGET_API_MAC_CARBON
							SystemClick(&theEvent, whichWindow);
#endif
							break;
					}
					break;
				case keyDown:
				case autoKey:
					theKey = theEvent.message & charCodeMask;
					theCode = (theEvent.message & keyCodeMask) >> 8;
					if ((theEvent.modifiers & cmdKey) != 0)
					{
						menuResult = MenuKey(theKey);
						if (HiWord(menuResult) != 0)
							DoMenu (menuResult);
					}
					else
						DoKey (theKey, theCode);
					break;
				case updateEvt:
					UpdateWrapper (&theEvent);
					break;
				case diskEvt:
					break;
				case osEvt:
					if (theEvent.message & 0x01000000)		//	Suspend/resume event
					{
						if (theEvent.message & 0x00000001)	//	Resume
						{
							gSleepTime = kForegroundSleep;
/*							if (IsFullScreen ())
								ResumeFullScreenGLMovie ();
							else
								ResumeGLMovie ();
							DoUpdate ();
*/							gfFrontProcess = true;
						}
						else
						{
							gSleepTime = kBackgroundSleep;	//	Suspend
/*							if (IsFullScreen ())
								SuspendFullScreenGLMovie ();
							else
								SuspendGLMovie ();
*/							gfFrontProcess = false;
						}
					}
					break;

				case kHighLevelEvent:
					AEProcessAppleEvent(&theEvent);
					break;
			}
		}
	}
#if !TARGET_API_MAC_CARBON
	else
		DoUpdate ();
#endif // !TARGET_API_MAC_CARBON
}

// --------------------------------------------------------------------------

void CleanUp (void)
{
	MenuHandle hMenu;

#if TARGET_API_MAC_CARBON
	if (gTimer)
	{
		RemoveEventLoopTimer(gTimer);
		gTimer = NULL;
	}
#endif

	CleanupGLMovie ();

	hMenu = GetMenuHandle (kMenuFile);
	DeleteMenu (kMenuFile);
	DisposeMenu (hMenu);

	hMenu = GetMenuHandle (kMenuApple);
	DeleteMenu (kMenuApple);
	DisposeMenu (hMenu);
	
// profile
#if __profile__
	if (noErr == gProfErr)
	{
		ProfilerDump("\pMoovGL.prof");
		ProfilerTerm();
	}
#endif	
}

// --------------------------------------------------------------------------

int main (void)
{
	if (SetUp ())	
		while (!gDone) 
		{
// profile
#if __profile__
			if (noErr == gProfErr)
				ProfilerSetStatus(TRUE); // turn on profiling
#endif	
			if (UseWaitNextEvent ())
				DoEvent ();
			else
				DoButton ();
// profile
#if __profile__
			if (noErr == gProfErr)
				ProfilerSetStatus(FALSE); // turn profiling off again
#endif	
		}
	CleanUp ();
	return 0;
}