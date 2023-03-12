/*
	File:		TestHIView.cp

	Contains:	Minimal application to test HIViews.

	Version:	1.0.2

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

	Copyright © 2002 Apple Computer, Inc., All Rights Reserved
*/

#include <Carbon/Carbon.h>
#include "HISimpleList.h"

// Common stuff to all samples

void InitMenuBar(void);
pascal OSErr HandlePref(const AppleEvent *theAppleEvent, AppleEvent *reply, long handlerRefcon);
pascal OSErr HandleOapp(const AppleEvent *theAppleEvent, AppleEvent *reply, long handlerRefcon);
pascal OSErr HandleOdoc(const AppleEvent *theAppleEvent, AppleEvent *reply, long handlerRefcon);
pascal OSErr HandlePdoc(const AppleEvent *theAppleEvent, AppleEvent *reply, long handlerRefcon);
void InstallAppleEventHandlers(void);
pascal OSStatus CommandProcess(EventHandlerCallRef nextHandler, EventRef theEvent, void* userData);
void InstallApplicationCarbonEventHandlers(void);
void DoCloseWindow(WindowRef theWind);
void RecursiveCloseWindow(WindowRef theWind);
void CloseAllWindows();
void InitApplication(void);
void TermApplication(void);

// End of common stuff

void DoAboutBox(void);
void DoPreferences(void);
void DoNewWindow(void);
void DoOtherNewWindow(void);

enum
	{
	kHICommandNew2 = 'NEW2'
	};

int main(void)
	{
	InitApplication();
	RunApplicationEventLoop();
	TermApplication();
   return 0;
	}

//------------------------------- Initializations ------------------------------

void InitApplication(void)
	{
	long response;
	OSErr err;

   // Can we run this particular demo application?
	err = Gestalt(gestaltSystemVersion, &response);
	Boolean ok = ((err == noErr) && (response >= 0x00001020));
   if (!ok)
      {
		StandardAlert(kAlertStopAlert, "\pMac OS X 10.2 (minimum) is required for this application", "\p", NULL, NULL);
      ExitToShell();
      }

	InstallAppleEventHandlers();
	InstallApplicationCarbonEventHandlers();

	InitMenuBar();

	DoNewWindow();
	DoOtherNewWindow();
	}

void InitMenuBar(void)
	{
	// Getting our menu bar
	Handle aHand = GetNewMBar(128);
	SetMenuBar(aHand);

	// Enabling Preferences menu item
	EnableMenuCommand(NULL, kHICommandPreferences);

	// Adding some menu item commands
	MenuHandle menu = GetMenuHandle(128);
	SetMenuItemCommandID(menu, 1, kHICommandNew);
	SetMenuItemCommandID(menu, 2, kHICommandNew2);
	SetMenuItemCommandID(menu, 3, kHICommandClose);
	SetMenuItemCommandID(GetMenuHandle(1),  1, kHICommandAbout);

	// Adding a standard Window menu
	CreateStandardWindowMenu(0, &menu);
	InsertMenu(menu, 0);

	DrawMenuBar();
	DisposeHandle(aHand);
	}

//------------------------------- Apple Events ---------------------------------

void InstallAppleEventHandlers(void)
	{
	OSErr	err;
	err = AEInstallEventHandler(kCoreEventClass, kAEShowPreferences, NewAEEventHandlerUPP(HandlePref), 0, false);
	if (err) DebugStr("\pAEInstallEventHandler failed for kAEOpenApplication");
	err = AEInstallEventHandler(kCoreEventClass, kAEOpenApplication, NewAEEventHandlerUPP(HandleOapp), 0, false);
	if (err) DebugStr("\pAEInstallEventHandler failed for kAEOpenApplication");
	err = AEInstallEventHandler(kCoreEventClass, kAEOpenDocuments,   NewAEEventHandlerUPP(HandleOdoc), 0, false);
	if (err) DebugStr("\pAEInstallEventHandler failed for kAEOpenDocuments");
	err = AEInstallEventHandler(kCoreEventClass, kAEPrintDocuments,  NewAEEventHandlerUPP(HandlePdoc), 0, false);
	if (err) DebugStr("\pAEInstallEventHandler failed for kAEPrintDocuments");
	
	// Note: Since RunApplicationEventLoop installs a Quit AE Handler, we no longer have to do it here.
	}

pascal OSErr HandlePref(const AppleEvent *theAppleEvent, AppleEvent *reply, long handlerRefcon)
	{
#pragma unused (theAppleEvent, reply, handlerRefcon)
	return errAEEventNotHandled;
	}

pascal OSErr HandleOapp(const AppleEvent *theAppleEvent, AppleEvent *reply, long handlerRefcon)
	{
#pragma unused (theAppleEvent, reply, handlerRefcon)
	return noErr;
	}

pascal OSErr HandleOdoc(const AppleEvent *theAppleEvent, AppleEvent *reply, long handlerRefcon)
	{
#pragma unused (theAppleEvent, reply, handlerRefcon)
	return errAEEventNotHandled;
	}

pascal OSErr HandlePdoc(const AppleEvent *theAppleEvent, AppleEvent *reply, long handlerRefcon)
	{
#pragma unused (theAppleEvent, reply, handlerRefcon)
	return errAEEventNotHandled;
	}

//------------------------------- Carbon Events --------------------------------

void InstallApplicationCarbonEventHandlers(void)
	{
	EventTypeSpec eventType = {kEventClassCommand, kEventCommandProcess};
	InstallEventHandler(GetApplicationEventTarget(), NewEventHandlerUPP(CommandProcess), 1, &eventType, NULL, NULL);
	}

pascal OSStatus CommandProcess(EventHandlerCallRef nextHandler, EventRef theEvent, void* userData)
	{
#pragma unused (nextHandler, userData)
	HICommand aCommand;
	OSStatus status = noErr;

	GetEventParameter(theEvent, kEventParamDirectObject, typeHICommand, NULL, sizeof(HICommand), NULL, &aCommand);
      
	switch (aCommand.commandID)
		{
		case kHICommandAbout:
			DoAboutBox();
			break;
		case kHICommandPreferences:
			DoPreferences();
			break;
		case kHICommandNew:
			DoNewWindow();
			break;
		case kHICommandNew2:
			DoOtherNewWindow();
			break;
		case kHICommandClose:
			DoCloseWindow(FrontNonFloatingWindow());
			break;
		default:
			status = eventNotHandledErr;
			break;
		}
	return status;
	}

//------------------------------- Terminating ----------------------------------

void TermApplication(void)
	{
	CloseAllWindows();
	}

void CloseAllWindows()
	{
	WindowRef theWind = FrontWindow();
	if (theWind)
		{
		RecursiveCloseWindow(theWind);
		}
	}

void RecursiveCloseWindow(WindowRef theWind)
	{
	WindowRef nextWind = GetNextWindow(theWind);
	if (nextWind) RecursiveCloseWindow(nextWind);
	DoCloseWindow(theWind);
	}

void DoCloseWindow(WindowRef theWind)
	{
	EventRef theEvent;
	CreateEvent(NULL, kEventClassWindow, kEventWindowClose, 0, 0, &theEvent);
	SetEventParameter(theEvent, kEventParamDirectObject, typeWindowRef, sizeof(WindowRef), &theWind);
	SendEventToEventTarget(theEvent, GetWindowEventTarget(theWind));
	}

//------------------------------- End of common stuff --------------------------

void DoAboutBox(void)
	{
	StandardAlert(kAlertNoteAlert, "\pHISimpleList 1.2.3 by DTS, Apple © 2006", "\p", NULL, NULL);
	}

void DoPreferences(void)
	{
	StandardAlert(kAlertNoteAlert, "\pNo Preferences yet!", "\p", NULL, NULL);
	}

#define kSimpleListViewHeader 40
#define kViewMargin 10
#define kSetTo0 'SZER'
#define kSetTo1 'SONE'
#define kSetTo10 'STEN'
#define kSetTo100 'SHUN'
#define kSetTo1000 'SKIL'

pascal OSStatus WindCommandHandler(EventHandlerCallRef nextHandler, EventRef theEvent, void* userData)
	{
	// we got a successful on one of the buttons
	// we retrieve the scrollview reference and call SetControlData + kSetNumberOfLinesCommand
	
	OSStatus status = eventNotHandledErr;

	HICommand aCommand;
	GetEventParameter(theEvent, kEventParamDirectObject, typeHICommand, NULL, sizeof(aCommand), NULL, &aCommand);
	
	SInt32 newNbLines = -1;
	switch (aCommand.commandID)
		{
		case kSetTo0:		newNbLines =   0; break;
		case kSetTo1:		newNbLines =   1; break;
		case kSetTo10:		newNbLines =   10; break;
		case kSetTo100:	newNbLines =  100; break;
		case kSetTo1000:	newNbLines = 1000; break;
		}

	if ((newNbLines != -1) && ((aCommand.attributes & kHICommandFromControl) != 0))
		{
		HICommandExtended * extCom = (HICommandExtended *)&aCommand;
		ControlRef theControl = extCom->source.control;
		ControlRef theAssociatedControl = (ControlRef)GetControlReference(theControl);
		SetControlData(theAssociatedControl, kControlEntireControl, kSetNumberOfLinesCommand, sizeof(newNbLines), &newNbLines);
		status = noErr;
		}

	return status;
	}

pascal OSStatus WindBoundsHandler(EventHandlerCallRef nextHandler, EventRef theEvent, void* userData)
	{
	// the bottom and right sides of our list will stick to the window edges
	WindowRef theWind;
	GetEventParameter(theEvent, kEventParamDirectObject, typeWindowRef, NULL, sizeof(theWind), NULL, &theWind);
	HIViewRef contentView;
	HIViewFindByID(HIViewGetRoot(theWind), kHIViewWindowContentID, &contentView);
	HIRect boundsRect;
	HIViewGetBounds(contentView, &boundsRect);
	boundsRect.origin.y += kSimpleListViewHeader;
	boundsRect.size.height -= kSimpleListViewHeader;
	
	// for simplicity, we set up our list view as the first view. Else, we could find it by ID.
	// see the end of DoNewWindow()
	HIViewSetFrame(HIViewGetFirstSubview(contentView), &boundsRect);
	return eventNotHandledErr;
	}

void DoNewWindow(void)
	{
	WindowRef theWind;
	Rect bounds = {50, 50, 550, 550};
	OSStatus theStatus = CreateNewWindow(
									kDocumentWindowClass,
									kWindowStandardDocumentAttributes | kWindowStandardHandlerAttribute | kWindowCompositingAttribute,
									&bounds, &theWind);
	if ((theStatus != noErr) || (theWind == NULL)) {DebugStr("\pCreateNewWindow failed!"); return;}
	
	SetWindowTitleWithCFString(theWind, CFSTR("HISimpleList"));

	// we could have both following handlers grouped in 1 function but for the sake or readibility, let's keep them separate.
	EventTypeSpec event1 =	{kEventClassWindow, kEventWindowBoundsChanged};
	InstallEventHandler(GetWindowEventTarget(theWind), NewEventHandlerUPP(WindBoundsHandler), 1, &event1, NULL, NULL);

	EventTypeSpec event2 = {kEventClassCommand, kEventCommandProcess};
	InstallEventHandler(GetWindowEventTarget(theWind), NewEventHandlerUPP(WindCommandHandler), 1, &event2, NULL, NULL);
	
	HIViewRef contentView;
	HIViewFindByID(HIViewGetRoot(theWind), kHIViewWindowContentID, &contentView);
	
	//let's create the scrolling view
	HIViewRef listView;
	OSStatus status = HICreateSimpleList(&listView);
	if (status != noErr) {DebugStr("\p HICreateSimpleList failed"); return;}
	
	// and retrieve the view being scrolled inside
	HIViewRef scrolledView;
	HIViewFindByID(listView, kSimpleListViewID, &scrolledView);

	HIRect boundsRect;
	status = HIViewGetBounds(contentView, &boundsRect);
	
	// let's add 3 buttons to set a different number of lines in our list
	SInt32 top = (int) boundsRect.origin.y;
	SInt32 left = (int) boundsRect.origin.x;
	Rect buttonRect = {top + kViewMargin, left + kViewMargin, top + kViewMargin + 20, left + 100};
	ControlRef button;

	CreatePushButtonControl(NULL, &buttonRect, CFSTR("Empty List"), &button);
	status = HIViewAddSubview(contentView, button);
	SetControlCommandID(button, kSetTo0);
	SetControlReference(button, (SInt32)scrolledView);
	HIViewSetVisible(button, true);

	buttonRect.left = buttonRect.right + kViewMargin; buttonRect.right = buttonRect.left + 60;

	CreatePushButtonControl(NULL, &buttonRect, CFSTR("1 Line"), &button);
	status = HIViewAddSubview(contentView, button);
	SetControlCommandID(button, kSetTo1);
	SetControlReference(button, (SInt32)scrolledView);
	HIViewSetVisible(button, true);

	buttonRect.left = buttonRect.right + kViewMargin; buttonRect.right = buttonRect.left + 80;

	CreatePushButtonControl(NULL, &buttonRect, CFSTR("10 Lines"), &button);
	status = HIViewAddSubview(contentView, button);
	SetControlCommandID(button, kSetTo10);
	SetControlReference(button, (SInt32)scrolledView);
	HIViewSetVisible(button, true);

	buttonRect.left = buttonRect.right + kViewMargin; buttonRect.right = buttonRect.left + 100;

	CreatePushButtonControl(NULL, &buttonRect, CFSTR("100 Lines"), &button);
	status = HIViewAddSubview(contentView, button);
	SetControlCommandID(button, kSetTo100);
	SetControlReference(button, (SInt32)scrolledView);
	HIViewSetVisible(button, true);

	buttonRect.left = buttonRect.right + kViewMargin; buttonRect.right = buttonRect.left + 100;

	CreatePushButtonControl(NULL, &buttonRect, CFSTR("1000 Lines"), &button);
	status = HIViewAddSubview(contentView, button);
	SetControlCommandID(button, kSetTo1000);
	SetControlReference(button, (SInt32)scrolledView);
	HIViewSetVisible(button, true);

	// the list view has to be added last to be the first in the list (or we could use HIViewSetZOrder...)
	// see WindBoundsHandler()
	status = HIViewAddSubview(contentView, listView);
	boundsRect.origin.y += kSimpleListViewHeader;
	boundsRect.size.height -= kSimpleListViewHeader;
	status = HIViewSetFrame(listView, &boundsRect);

	ShowWindow(theWind);
	}

pascal OSStatus OtherWindCommandHandler(EventHandlerCallRef nextHandler, EventRef theEvent, void* userData)
	{
	// we got a successful on one of the buttons
	// we retrieve the scrollview reference and call SetControl32BitMaximum
	
	OSStatus status = eventNotHandledErr;

	HICommand aCommand;
	GetEventParameter(theEvent, kEventParamDirectObject, typeHICommand, NULL, sizeof(aCommand), NULL, &aCommand);
	
	SInt32 newNbLines = -1;
	switch (aCommand.commandID)
		{
		case kSetTo10:		newNbLines =   10; break;
		case kSetTo100:	newNbLines =  100; break;
		case kSetTo1000:	newNbLines = 1000; break;
		}

	if ((newNbLines != -1) && ((aCommand.attributes & kHICommandFromControl) != 0))
		{
		HICommandExtended * extCom = (HICommandExtended *)&aCommand;
		ControlRef theControl = extCom->source.control;
		ControlRef theAssociatedControl = (ControlRef)GetControlReference(theControl);
		SetControl32BitMaximum(theAssociatedControl, newNbLines);
		status = noErr;
		}

	return status;
	}

void DoOtherNewWindow(void)
	{
	WindowRef theWind;
	Rect bounds = {50, 600, 550, 1100};
	OSStatus theStatus = CreateNewWindow(
									kDocumentWindowClass,
									kWindowStandardDocumentAttributes | kWindowStandardHandlerAttribute | kWindowCompositingAttribute,
									&bounds, &theWind);
	if ((theStatus != noErr) || (theWind == NULL)) {DebugStr("\pCreateNewWindow failed!"); return;}
	
	SetWindowTitleWithCFString(theWind, CFSTR("HINotSoSimpleList"));

	// we could have both following handlers grouped in 1 function but for the sake or readibility, let's keep them separate.
	EventTypeSpec event1 =	{kEventClassWindow, kEventWindowBoundsChanged};
	InstallEventHandler(GetWindowEventTarget(theWind), NewEventHandlerUPP(WindBoundsHandler), 1, &event1, NULL, NULL);

	EventTypeSpec event2 = {kEventClassCommand, kEventCommandProcess};
	InstallEventHandler(GetWindowEventTarget(theWind), NewEventHandlerUPP(OtherWindCommandHandler), 1, &event2, NULL, NULL);
	
	HIViewRef contentView;
	HIViewFindByID(HIViewGetRoot(theWind), kHIViewWindowContentID, &contentView);
	
	//let's create the scrolling view
	HIViewRef listView;
	OSStatus status = HICreateNotSoSimpleList(5, 300, &listView);
	if (status != noErr) {DebugStr("\p HICreateNotSoSimpleList failed"); return;}
	
	// and retrieve the view being scrolled inside
	HIViewRef scrolledView;
	HIViewFindByID(listView, kNotSoSimpleListViewID, &scrolledView);

	HIRect boundsRect;
	status = HIViewGetBounds(contentView, &boundsRect);
	
	// let's add 3 buttons to set a different number of lines in our list
	SInt32 top = (int) boundsRect.origin.y;
	SInt32 left = (int) boundsRect.origin.x;
	Rect buttonRect = {top + kViewMargin, left + kViewMargin, top + kViewMargin + 20, left + 100};
	ControlRef button;

	CreatePushButtonControl(NULL, &buttonRect, CFSTR("10 Lines"), &button);
	status = HIViewAddSubview(contentView, button);
	SetControlCommandID(button, kSetTo10);
	SetControlReference(button, (SInt32)scrolledView);
	HIViewSetVisible(button, true);

	buttonRect.left = buttonRect.right + kViewMargin; buttonRect.right = buttonRect.left + 100;

	CreatePushButtonControl(NULL, &buttonRect, CFSTR("100 Lines"), &button);
	status = HIViewAddSubview(contentView, button);
	SetControlCommandID(button, kSetTo100);
	SetControlReference(button, (SInt32)scrolledView);
	HIViewSetVisible(button, true);

	buttonRect.left = buttonRect.right + kViewMargin; buttonRect.right = buttonRect.left + 100;

	CreatePushButtonControl(NULL, &buttonRect, CFSTR("1000 Lines"), &button);
	status = HIViewAddSubview(contentView, button);
	SetControlCommandID(button, kSetTo1000);
	SetControlReference(button, (SInt32)scrolledView);
	HIViewSetVisible(button, true);

	Rect textRect = buttonRect;
	textRect.left = textRect.right + kViewMargin; textRect.right = textRect.left + 150; textRect.top += 2; textRect.bottom -= 3;

	ControlRef textControl;
	CreateEditUnicodeTextControl(NULL, &textRect, CFSTR("Type Something!"), false, NULL, &textControl);
	status = HIViewAddSubview(contentView, textControl);
	HIViewSetVisible(textControl, true);

	// the list view has to be added last to be the first in the list (or we could use HIViewSetZOrder...)
	// see WindBoundsHandler()
	status = HIViewAddSubview(contentView, listView);
	boundsRect.origin.y += kSimpleListViewHeader;
	boundsRect.size.height -= kSimpleListViewHeader;
	status = HIViewSetFrame(listView, &boundsRect);
	
	SetKeyboardFocus(theWind, scrolledView, kControlFocusNextPart);

	ShowWindow(theWind);
	}
