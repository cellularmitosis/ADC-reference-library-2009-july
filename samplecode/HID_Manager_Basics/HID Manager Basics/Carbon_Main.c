/*
	File:		Carbon Main.c

	Contains:	Main event handling and setup
				This code is designed to supplement the HID Manager Documentation, with minor chnages 
				for readability.  It should be used with Mac OS X only.
    
	DRI: George Warner

	Copyright:	Copyright © 2002 Apple Computer, Inc., All Rights Reserved

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
	#include <Carbon/Carbon.h>
#else
	#include "Carbon_Include.h"
    #include <MacTypes.h>
    #include <CarbonEvents.h>
    #include <Gestalt.h>
    #include <ToolUtils.h>
    
    // profile
    #if __profile__
        #include <Profiler.h>
    #endif
#endif

#include <math.h>
#include <stdio.h>

// project includes ---------------------------------------------------------

#include "HID_Manager_Test.h"

// prototypes ---------------------------------------------------------------

void InitToolbox(void);
Boolean SetUp (void);
void DoMenu (SInt32 menuResult);
void DoKey (SInt8 theKey, SInt8 theCode);
void DoUpdate (void);

pascal void IdleTimer (EventLoopTimerRef inTimer, void* userData);
EventLoopTimerUPP GetTimerUPP (void);

void DoButton (void);
void DoEvent (void);
void CleanUp (void);

// profile wrappers
Boolean WaitNextEventWrapper (EventMask eventMask, EventRecord *theEvent, unsigned long sleep,RgnHandle mouseRgn);
void UpdateWrapper (EventRecord *theEvent);


// statics/globals (internal only) ------------------------------------------

// Menu defs
enum 
{
	kMenuApple = 128,
	kMenuFile = 129,
	
	kAppleAbout = 1,
    KFileTestHID = 1,
	kFileQuit
};

enum 
{
	kForegroundSleep = 10,
	kBackgroundSleep = 10000
};
EventLoopTimerRef gTimer = NULL;

UInt32 gSleepTime = kForegroundSleep;
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
	
	InitCursor();
	
// profile
#if __profile__
	gProfErr = ProfilerInit (collectDetailed, bestTimeBase, 20, 5); // set up profiling
	if (noErr == gProfErr)
		ProfilerSetStatus(FALSE);
#endif	
	
	// Init Menus
	menu = NewMenu (kMenuApple, "\p\024");			// new  apple menu
	InsertMenu (menu, 0);							// add menu to end
    menu = NewMenu (kMenuFile, "\pFile");			// new menu
    InsertMenu (menu, 0);							// add menu to end
    AppendMenu (menu, "\pTest HID/T"); 				// add HID Menu

	// add quit if not under Mac OS X
	err = Gestalt (gestaltMenuMgrAttr, &response);
	if ((err == noErr) && !(response & gestaltMenuMgrAquaLayoutMask))
	{
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

	// Do setup here
	
	// now that we are set up go ahead and start drawing
	InstallEventLoopTimer (GetCurrentEventLoop(), 0, 0.000001, GetTimerUPP (), 0, &gTimer);

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
					break;
			}
			break;
		case kMenuFile:
			switch (theItem)
			{
				case KFileTestHID:
					MyStartHIDDeviceInterfaceTest ();
					break;
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
	// do window drawing
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

// --------------------------------------------------------------------------

void DoButton (void)
{

}

// --------------------------------------------------------------------------

Boolean WaitNextEventWrapper (EventMask eventMask, EventRecord *theEvent, UInt32 sleep, RgnHandle mouseRgn)
{
	return WaitNextEvent (eventMask, theEvent, sleep, mouseRgn);
}

// --------------------------------------------------------------------------

void DoEvent (void)
{
	EventRecord theEvent;
	Rect rectWork;
	SInt32 menuResult;
	WindowRef whichWindow = NULL;
	long grow;
	SInt16 whatPart;
	SInt8 theKey;
	SInt8 theCode;
//	Boolean fProcessed = false;
	
	if (WaitNextEventWrapper (everyEvent, &theEvent, gSleepTime, NULL))
	{
		switch (theEvent.what)
		{
			case mouseDown:
				whatPart = FindWindow(theEvent.where, &whichWindow);
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
						BitMap	screenBits;
						DragWindow(whichWindow, theEvent.where, &GetQDGlobalsScreenBits(&screenBits)->bounds);
					}
						break;
					case inGrow:
					{
						SetRect (&rectWork, 100, 100, 20000, 20000);
						grow = GrowWindow (whichWindow, theEvent.where, &rectWork);
						if (grow)
						{
							SizeWindow (whichWindow, grow & 0x0000FFFF, grow >> 16, true);
							SetPort((GrafPtr) GetWindowPort(whichWindow));
							InvalWindowRect(whichWindow, GetWindowPortBounds(whichWindow, &rectWork));
						}
						break;
					}
					case inSysWindow:
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
						DoUpdate ();
						gfFrontProcess = true;
					}
					else
					{
						gSleepTime = kBackgroundSleep;	//	Suspend
						gfFrontProcess = false;
					}
				}
				break;

			case kHighLevelEvent:
				AEProcessAppleEvent(&theEvent);
				break;
		}
	}
}

// --------------------------------------------------------------------------

void CleanUp (void)
{
	MenuHandle hMenu;

	if (gTimer)
	{
		RemoveEventLoopTimer(gTimer);
		gTimer = NULL;
	}

	// do clean up here

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
				DoEvent ();
// profile
#if __profile__
			if (noErr == gProfErr)
				ProfilerSetStatus(FALSE); // turn profiling off again
#endif	
		}
	CleanUp ();
	return 0;
}