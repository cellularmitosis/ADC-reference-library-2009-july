/*

File: ExamplePrefs.c

Abstract: Demo program showing how the custom Icon List Definition function might 
          be used in creating a Preferences window/dialog.

Version: 1.0

Change History:
	
	<1.0>	first release version, this is totally Mac OS 8.6-savvy
			it's kind of cheating but we decided to use Carbon Event 
			HI Command IDs even though we're not using Carbon Events 
			(or Carbon for that matter)

© Copyright 2005 Apple Computer, Inc. All rights reserved.

IMPORTANT:  This Apple software is supplied to 
you by Apple Computer, Inc. ("Apple") in 
consideration of your agreement to the following 
terms, and your use, installation, modification 
or redistribution of this Apple software 
constitutes acceptance of these terms.  If you do 
not agree with these terms, please do not use, 
install, modify or redistribute this Apple 
software.

In consideration of your agreement to abide by 
the following terms, and subject to these terms, 
Apple grants you a personal, non-exclusive 
license, under Apple's copyrights in this 
original Apple software (the "Apple Software"), 
to use, reproduce, modify and redistribute the 
Apple Software, with or without modifications, in 
source and/or binary forms; provided that if you 
redistribute the Apple Software in its entirety 
and without modifications, you must retain this 
notice and the following text and disclaimers in 
all such redistributions of the Apple Software. 
Neither the name, trademarks, service marks or 
logos of Apple Computer, Inc. may be used to 
endorse or promote products derived from the 
Apple Software without specific prior written 
permission from Apple.  Except as expressly 
stated in this notice, no other rights or 
licenses, express or implied, are granted by 
Apple herein, including but not limited to any 
patent rights that may be infringed by your 
derivative works or by other works in which the 
Apple Software may be incorporated.

The Apple Software is provided by Apple on an "AS 
IS" basis.  APPLE MAKES NO WARRANTIES, EXPRESS OR 
IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED 
WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY 
AND FITNESS FOR A PARTICULAR PURPOSE, REGARDING 
THE APPLE SOFTWARE OR ITS USE AND OPERATION ALONE 
OR IN COMBINATION WITH YOUR PRODUCTS.

IN NO EVENT SHALL APPLE BE LIABLE FOR ANY 
SPECIAL, INDIRECT, INCIDENTAL OR CONSEQUENTIAL 
DAMAGES (INCLUDING, BUT NOT LIMITED TO, 
PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS 
OF USE, DATA, OR PROFITS; OR BUSINESS 
INTERRUPTION) ARISING IN ANY WAY OUT OF THE USE, 
REPRODUCTION, MODIFICATION AND/OR DISTRIBUTION OF 
THE APPLE SOFTWARE, HOWEVER CAUSED AND WHETHER 
UNDER THEORY OF CONTRACT, TORT (INCLUDING 
NEGLIGENCE), STRICT LIABILITY OR OTHERWISE, EVEN 
IF APPLE HAS BEEN ADVISED OF THE POSSIBILITY OF 
SUCH DAMAGE.

*/

#include "ExamplePrefs.h"
#include "Help.h"
#include "IconListUtilities.h"
#include "PrefsDialog.h"
#include "PrefsWindow.h"

#include <AEDataModel.h>
#include <AEInteraction.h>
#include <Appearance.h>
#include <AppleEvents.h>
#include <Balloons.h>
#include <CarbonEvents.h>
#include <Controls.h>
#include <Devices.h>
#include <Dialogs.h>
#include <DiskInit.h>
#include <Events.h>
#include <Fonts.h>
#include <MacErrors.h>
#include <MacMemory.h>
#include <MacWindows.h>
#include <Processes.h>
#include <Quickdraw.h>
#include <Sound.h>
#include <TextEdit.h>
#include <ToolUtils.h>

#include <limits.h>

static AEEventHandlerUPP gOpenAppAEHandler;
static AEEventHandlerUPP gQuitAppAEHandler;
static AEEventHandlerUPP gViewsFontChangedAEHandler;
static Boolean gDone;

static void initialize(void);
static void handleEvents(EventRecord *event);
static void doIdleProcessing(void);
static pascal OSErr openApplicationAEHandler(const AppleEvent *appleEvent, AppleEvent *reply, 
												long refcon);
static pascal OSErr quitApplicationAEHandler(const AppleEvent *appleEvent, AppleEvent *reply, 
												long refcon);
static pascal OSErr viewsFontChangedAEHandler(const AppleEvent *appleEvent, 
												AppleEvent *reply, long refcon);


// --------------------------------------------------------------------------------------
int main(void)
{
	WindowRef window;
	
	initialize();		// initialization

	gDone = false;
	while (!gDone)		// application event loop
	{
		EventRecord event;
		
		if (WaitNextEvent(everyEvent, &event, ULONG_MAX, NULL))
		{
			if (IsDialogEvent(&event))
				HandleDialogEvents(&event);
			else
				handleEvents(&event);
		}
		else
			doIdleProcessing();
	}
									// finalization
	for (window = FrontNonFloatingWindow(); window != NULL; window = FrontNonFloatingWindow())
	{
		if (GetWindowKind(window) == kDialogWindowKind)
			ClosePrefsDialog(GetDialogFromWindow(window));
		else	// kApplicationWindowKind
			ClosePrefsWindow(window);
	}
	
	DisposeAEEventHandlerUPP(gOpenAppAEHandler);
	DisposeAEEventHandlerUPP(gQuitAppAEHandler);
	DisposeAEEventHandlerUPP(gViewsFontChangedAEHandler);
	
	return 0;
}

// --------------------------------------------------------------------------------------
static void initialize(void)
{
	MenuBarHandle menuBar;
	OSErr error;
	
	MoreMasters();				// I doubt we actually need any extra master pointers 
	InitGraf(&qd.thePort);		// but I left the calls here as a reminder for where they 
	InitFonts();				// belong if they are needed
	InitWindows();
	InitMenus();
	TEInit();
	InitDialogs(NULL);
	
	InitCursor();
	RegisterAppearanceClient();
	
	menuBar = GetNewMBar(rMenuBar);		// draw the menu bar as soon as possible
	if (menuBar == NULL)
		ExitToShell();
	SetMenuBar(menuBar);
	DrawMenuBar();
	
		// do non time sensitive initialization after we get the application event loop going
	gOpenAppAEHandler = NewAEEventHandlerUPP(openApplicationAEHandler);
	error = AEInstallEventHandler(kCoreEventClass, kAEOpenApplication, gOpenAppAEHandler, 0, 
									false);
	if (error != noErr)
		ExitToShell();
	
		/* If we supported them, we would install open documents and print documents 
		   handlers here and we would do most of the same initialization stuff that we 
		   do in the open application handler. */
	
	FlushEvents(everyEvent, 0);
}

// --------------------------------------------------------------------------------------
static void handleEvents(EventRecord *event)
{
	WindowRef window;
	Boolean activate;
	WindowPartCode partCode;
	OSErr error;
	Rect tempRect, newSize;
	long menuChoice;
	MenuCommand commandID;
	RgnHandle grayRegion;
	
	switch (event->what)	// handle events according to the priority 
	{						// determined by the system
		case activateEvt:
			window = (WindowRef)event->message;
			activate = (event->modifiers & activeFlag) != 0;
			HandleActivate(window, activate);
			break;
		
		case mouseDown:
			partCode = FindWindow(event->where, &window);
			
			switch(partCode)
			{
				case inMenuBar:
					menuChoice = MenuSelect(event->where);
					error = GetMenuItemCommandID(GetMenuRef(HiWord(menuChoice)), 
 													LoWord(menuChoice), &commandID);
					if (error == noErr)
					{
						if (commandID == 0)		// if the menu item clicked on does not have 
							commandID = (MenuCommand)menuChoice;	// a command ID
						HandleMenuChoice(commandID);
					}
					break;
				
				case inSysWindow:
					if (window != NULL)
						SystemClick(event, window);
					break;
				
				case inContent:		// the following window part codes will only be returned 
					if (window != FrontWindow())				// for the preferences window
						SelectWindow(window);
					else
						HandleContentClick(window, event->where, event->modifiers);
					break;
				
				case inDrag:
					grayRegion = GetGrayRgn();
					DragWindow(window, event->where, &((*grayRegion)->rgnBBox));
					break;
				
				case inGrow:	
					SetRect(&tempRect, kPrefsWindowPlatinumWidth, kPrefsWindowPlatinumHeight, 
							SHRT_MAX, SHRT_MAX);
					ResizeWindow(window, event->where, &tempRect, &newSize);
					AdjustControls(window);
					break;
				
				case inGoAway:
					ClosePrefsWindow(window);
					break;
			}
			break;
		
		case keyDown:
		case autoKey:	// a separate auto key handler would go after disk events
			if ((event->modifiers & cmdKey) != 0)
			{
				UInt32 keyMenuChoice;
				
				keyMenuChoice = MenuEvent(event);
				error = GetMenuItemCommandID(GetMenuRef(HiWord(keyMenuChoice)), 
												LoWord(keyMenuChoice), &commandID);
				if (error == noErr)
				{
					if (commandID == 0)		// if the menu item chosen does not have a 
						commandID = (MenuCommand)keyMenuChoice;		// command ID 
					HandleMenuChoice(commandID);					// (but they all should)
				}
			}
			else
			{
				window = FrontNonFloatingWindow();
				if (window != NULL)
				{
					char keyCode = (event->message & keyCodeMask) >> 8;
					
					HandleKeyDown(keyCode, window);
				}
			}
			break;
		
		case diskEvt:
			if (HiWord(event->message) != noErr) 
			{
				Point where;
			
				SetPt(&where, 70, 50);
				ShowCursor();
				DIBadMount(where, event->message);
			}		
			break;
		
		case updateEvt:
			window = (WindowRef)event->message;
			SetPortWindowPort(window);
			
			BeginUpdate(window);
			HandleDrawContent(window);
			EndUpdate(window);
			break;
		
		case kHighLevelEvent:		// an OS Event handler would go before high level events
			AEProcessAppleEvent(event);
			break;
	}
} // handleEvents

// --------------------------------------------------------------------------------------
static void doIdleProcessing(void)
{
	WindowRef frontWindow;
	
	frontWindow = FrontNonFloatingWindow();
	if (frontWindow != NULL)
	{
		IdleControls(frontWindow);
		if ( (GetWindowKind(frontWindow) == kApplicationWindowKind) && HMGetBalloons() )
			DisplayHelpBalloons(frontWindow);
	}
}

// --------------------------------------------------------------------------------------
void HandleMenuChoice(MenuCommand commandID)
{
	MenuID menuID;
	MenuItemIndex menuItem;
	Str255 itemName;
	short daDriverRefNum;
	
	switch (commandID)
	{
		case kHICommandAbout:
			SysBeep(30);
			break;
		
		case kHICommandClose:
			ClosePrefsWindow(FrontNonFloatingWindow());
			break;
		
		case kHICommandQuit:
			gDone = true;
			break;
		
		case kCommandPrWin:
			OpenPrefsWindow();
			break;
		
		case kCommandPrDlg:
			OpenPrefsDialog();
			break;
		
		default:	// there isn't a command ID; the commandID parameter is actually a 
			menuID = HiWord((long)commandID);		// MenuID:MenItemIndex structure
			menuItem = LoWord((long)commandID);
			
			switch (menuID)
			{
				case mApple:
					GetMenuItemText(GetMenuRef(menuID), menuItem, itemName);
					daDriverRefNum = OpenDeskAcc(itemName);
					break;
			}
	}
	
	HiliteMenu(0);
}

#pragma mark -
// --------------------------------------------------------------------------------------
static pascal OSErr openApplicationAEHandler(const AppleEvent *appleEvent, AppleEvent *reply, 
												long refcon)
{
#pragma unused (reply, refcon)
	
	OSErr error;
	DescType returnedType;
	Size actualSize;
	
	error = AEGetAttributePtr(appleEvent, keyMissedKeywordAttr, typeWildCard, &returnedType,
								NULL, 0, &actualSize);
	if (error == noErr)
		error = errAEParamMissed;
	else if (error == errAEDescNotFound)
	{
		MenuRef menu;
		
		gQuitAppAEHandler = NewAEEventHandlerUPP(quitApplicationAEHandler);
		error = AEInstallEventHandler(kCoreEventClass, kAEQuitApplication, gQuitAppAEHandler, 
										0, false);
		if (error != noErr)		// if we can't allow the user a mechanism to quit
		{						// we'd better just quit right now
			DisposeAEEventHandlerUPP(gOpenAppAEHandler);
			DisposeAEEventHandlerUPP(gQuitAppAEHandler);
			
			ExitToShell();
		}
		gViewsFontChangedAEHandler = NewAEEventHandlerUPP(viewsFontChangedAEHandler);
		error = AEInstallEventHandler(kAppearanceEventClass, kAEViewsFontChanged, 
										gViewsFontChangedAEHandler, 0, false);
		
		menu = GetMenuRef(mApple);
		AppendResMenu(menu,'DRVR');
		
		ClearIconsAreRegistered();
		
		error = noErr;
	}
	
	return error;
}

// --------------------------------------------------------------------------------------
static pascal OSErr quitApplicationAEHandler(const AppleEvent *appleEvent, AppleEvent *reply, 
												long refcon)
{
#pragma unused (reply, refcon)

	OSErr error;
	DescType returnedType;
	Size actualSize;

	error = AEGetAttributePtr(appleEvent, keyMissedKeywordAttr, typeWildCard, &returnedType, 
								NULL, 0, &actualSize);

	if (error == noErr)
		error = errAEParamMissed;
	else if (error == errAEDescNotFound)
	{
		gDone = true;
		error = noErr;
	} 
	
	return error;
}

// --------------------------------------------------------------------------------------
static pascal OSErr viewsFontChangedAEHandler(const AppleEvent *appleEvent, 
												AppleEvent *reply, long refcon)
{
#pragma unused (reply, refcon)

	OSErr error;
	DescType returnedType;
	Size actualSize;

	error = AEGetAttributePtr(appleEvent, keyMissedKeywordAttr, typeWildCard, &returnedType, 
								NULL, 0, &actualSize);

	if (error == noErr)
		error = errAEParamMissed;
	else if (error == errAEDescNotFound)
	{
		WindowRef window;
		
		for (window = FrontNonFloatingWindow(); window != NULL; 
				window = GetNextWindow(window))
		{
			if (GetWindowKind(window) == kDialogWindowKind)
				RedrawPrefsDialogList(GetDialogFromWindow(window));
			else	// kApplicationWindowKind
				RedrawPrefsWindowList(window);
		}
		
		error = noErr;
	} 
	
	return error;
}

#pragma mark -
// --------------------------------------------------------------------------------------
void GetWindowDeviceDepthAndColor(WindowRef inWindow, short *outPixelSize, 
									Boolean *outIsColorDevice)
{
	Rect contentRect;
	GDHandle device;
	
	GetWindowBounds(inWindow, kWindowContentRgn, &contentRect);
	
	device = GetMaxDevice(&contentRect);
	
	*outPixelSize = (*(*device)->gdPMap)->pixelSize;
	
	if (BitTst(&(*device)->gdFlags, gdDevType))
		*outIsColorDevice = true;
	else
		*outIsColorDevice = false;
}