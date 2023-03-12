/*
*	File:			main.c of StarMenu
* 
*	Contains:	This file contains the "main" routine for the Star Menu sample code.  This
*					code aguments the code from the generic Carbon project by loading a nib file
*					that contains a popup menu view, creating and populating a star menu, then
*					setting the menu for the popup menu view to be that star menu.
*	
*	Version:		1.0
*
*	Disclaimer:	IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc.
*					("Apple") in consideration of your agreement to the following terms, and your
*					use, installation, modification or redistribution of this Apple software
*					constitutes acceptance of these terms.  If you do not agree with these terms,
*					please do not use, install, modify or redistribute this Apple software.
*
*					In consideration of your agreement to abide by the following terms, and subject
*					to these terms, Apple grants you a personal, non-exclusive license, under AppleÍs
*					copyrights in this original Apple software (the "Apple Software"), to use,
*					reproduce, modify and redistribute the Apple Software, with or without
*					modifications, in source and/or binary forms; provided that if you redistribute
*					the Apple Software in its entirety and without modifications, you must retain
*					this notice and the following text and disclaimers in all such redistributions of
*					the Apple Software.  Neither the name, trademarks, service marks or logos of
*					Apple Computer, Inc. may be used to endorse or promote products derived from the
*					Apple Software without specific prior written permission from Apple.  Except as
*					expressly stated in this notice, no other rights or licenses, express or implied,
*					are granted by Apple herein, including but not limited to any patent rights that
*					may be infringed by your derivative works or by other works in which the Apple
*					Software may be incorporated.
*
*					The Apple Software is provided by Apple on an "AS IS" basis.  APPLE MAKES NO
*					WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED
*					WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS FOR A PARTICULAR
*					PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND OPERATION ALONE OR IN
*					COMBINATION WITH YOUR PRODUCTS.
*
*					IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL OR
*					CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
*					GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
*					ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION, MODIFICATION AND/OR DISTRIBUTION
*					OF THE APPLE SOFTWARE, HOWEVER CAUSED AND WHETHER UNDER THEORY OF CONTRACT, TORT
*					(INCLUDING NEGLIGENCE), STRICT LIABILITY OR OTHERWISE, EVEN IF APPLE HAS BEEN
*					ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*
*	Copyright:  Copyright © 2005 Apple Computer, Inc, All Rights Reserved
*/

#include <Carbon/Carbon.h>
#include "StarMenu.h"

OSStatus WindowEventHandler(EventHandlerCallRef inCallRef, EventRef inEvent, void *userData);

int main(int argc, char* argv[])
{
	IBNibRef nibRef = NULL;
	WindowRef window = NULL;
	MenuRef starMenu = NULL;
	OSStatus err = noErr;

	EventHandlerUPP windowHandler = NewEventHandlerUPP(WindowEventHandler);
	static const EventTypeSpec windowEventsHandled[] = {{ kEventClassWindow, kEventWindowClosed }};

	// Create a Nib reference passing the name of the nib file (without the .nib extension)
	// CreateNibReference only searches into the application bundle.
	err = CreateNibReference(CFSTR("main"), &nibRef);
	require_noerr( err, CantGetNibRef );

	// Once the nib reference is created, set the menu bar. "MainMenu" is the name of the menu bar
	// object. This name is set in InterfaceBuilder when the nib is created.
	err = SetMenuBarFromNib(nibRef, CFSTR("MenuBar"));
	require_noerr( err, CantSetMenuBar );

	// Then create a window. "MainWindow" is the name of the window object. This name is set in
	// InterfaceBuilder when the nib is created.
	err = CreateWindowFromNib(nibRef, CFSTR("MainWindow"), &window);
	require_noerr( err, CantCreateWindow );

	InstallWindowEventHandler(
		window,
		windowHandler,
		GetEventTypeCount(windowEventsHandled),
		windowEventsHandled,
		NULL,
		NULL);

	// Call a utility routine to create a star menu.
	err = CreateStarMenu(4242, 50, &starMenu);
	if(NULL != starMenu) {
		short ctr;
		HIViewRef popupControl = NULL;
		ControlID controlToFind = { 'PopU', 4242 };

		CFStringRef itemNames[] = {
			CFSTR("Red"),
			CFSTR("Orange"),
			CFSTR("Yellow"),
			CFSTR("Green"),
			CFSTR("Blue"),
			CFSTR("Indigo"),
			CFSTR("Violet")
		};

		RGBColor itemColor[] = {
			{ 0xFFFF, 0x0000, 0x0000},
			{ 0xFFFF, 0x8888, 0x0000},
			{ 0xFFFF, 0xFFFF, 0x0000},
			{ 0x0000, 0xFFFF, 0x0000},
			{ 0x0000, 0x0000, 0xFFFF},
			{ 0x8888, 0x0000, 0xFFFF},
			{ 0x8888, 0x0000, 0x8888},
		};

		// Use the static arrays above to populate the star menu with
		// items.
		MenuItemIndex numItems = (sizeof(itemNames) / sizeof(CFStringRef));
		for(ctr = 0; ctr < numItems ; ctr++) {
			MenuItemIndex newItem;
			AppendMenuItemTextWithCFString( starMenu, itemNames[ctr], 0, 0, &newItem);
			SetStarMenuItemColor(starMenu, newItem, itemColor[ctr]);
		}

		// Locate the popup control on the window that was loaded from the
		// nib file and change its menu to be our star menu
		err = HIViewFindByID(HIViewGetRoot(window), controlToFind, &popupControl);
		if(noErr == err && NULL != popupControl) {
			SetControlData(
				popupControl,
				kControlEntireControl,
				kControlPopupButtonMenuRefTag,
				sizeof(MenuRef),
				&starMenu);

			SetControl32BitMaximum(popupControl, numItems);
		}
	}

	require_noerr( err, CantCreateMenu );

	// We don't need the nib reference anymore.
	DisposeNibReference(nibRef);

	// The window was created hidden so show it.
	ShowWindow( window );

	// Call the event loop
	RunApplicationEventLoop();

CantCreateMenu:
CantCreateWindow:
CantSetMenuBar:
CantGetNibRef:
	return err;
}

/* ------------------------------------------------------- WindowEventHandler */
/*
	Simple event handler that quits the application when we close the window.
*/
OSStatus WindowEventHandler(
	EventHandlerCallRef inCallRef,
	EventRef inEvent,
	void *userData)
{
	OSStatus retVal = eventNotHandledErr;

	if(GetEventKind(inEvent) == kEventWindowClosed) {
		static const HICommand quitCommand = { 0, kHICommandQuit, {0, 0} };
		retVal = ProcessHICommand(&quitCommand);
	}

	return retVal;
}