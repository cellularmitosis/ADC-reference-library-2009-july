/*
*	File:		main.c of KillEveryOneButMe
* 
*	Contains:	This sample gives developers a simple demonstration of how to quit other running processes
*				on the system.  This code is intended to be used in kiosks or for demonstrations, etc.
*				Do not abuse the power that this simple application demonstrates.
*
*				This application can be run within the Xcode IDE.
*				However, when Xcode itself is quit by this application then your application
*				will quit as well (not giving the expected results).
*				You can set the MACRO DONTKILLXCODE to 1 to prevent Xcode from being killed.
*
*				This sample lists the running processes on the system and offers two menu items.
*				One allows the user to kill all other running programs, the other will kill all other
*				running programs but sparing the Finder.
*
*				On Mac OS X, some processes such as Loginwindow will not be quit because they are required
*				for Mac OS X to operate properly.
*
*				This application in no way forces applications to quit.
*				It simply offers them an Apple Event telling them to quit.
*				They can remain running (for example if they have unsaved documents) by ignoring the Apple Event.
*
*  Note:		The project is set up so that the DEBUG macro is set to one when the "Development"
*				build style is chosen and not at all when the "Deployment" build style is chosen.
*				Thus, all the require asserts "fire" only in "Development".
*	
*	Version:	2.0
* 
*	Created:	8/18/05
*
*	Disclaimer:	IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc.
*				("Apple") in consideration of your agreement to the following terms, and your
*				use, installation, modification or redistribution of this Apple software
*				constitutes acceptance of these terms.  If you do not agree with these terms,
*				please do not use, install, modify or redistribute this Apple software.
*
*				In consideration of your agreement to abide by the following terms, and subject
*				to these terms, Apple grants you a personal, non-exclusive license, under AppleÕs
*				copyrights in this original Apple software (the "Apple Software"), to use,
*				reproduce, modify and redistribute the Apple Software, with or without
*				modifications, in source and/or binary forms; provided that if you redistribute
*				the Apple Software in its entirety and without modifications, you must retain
*				this notice and the following text and disclaimers in all such redistributions of
*				the Apple Software.  Neither the name, trademarks, service marks or logos of
*				Apple Computer, Inc. may be used to endorse or promote products derived from the
*				Apple Software without specific prior written permission from Apple.  Except as
*				expressly stated in this notice, no other rights or licenses, express or implied,
*				are granted by Apple herein, including but not limited to any patent rights that
*				may be infringed by your derivative works or by other works in which the Apple
*				Software may be incorporated.
*
*				The Apple Software is provided by Apple on an "AS IS" basis.  APPLE MAKES NO
*				WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED
*				WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS FOR A PARTICULAR
*				PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND OPERATION ALONE OR IN
*				COMBINATION WITH YOUR PRODUCTS.
*
*				IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL OR
*				CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
*				GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
*				ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION, MODIFICATION AND/OR DISTRIBUTION
*				OF THE APPLE SOFTWARE, HOWEVER CAUSED AND WHETHER UNDER THEORY OF CONTRACT, TORT
*				(INCLUDING NEGLIGENCE), STRICT LIABILITY OR OTHERWISE, EVEN IF APPLE HAS BEEN
*				ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*
*	Copyright:  Copyright © 2005 Apple Computer, Inc, All Rights Reserved
*/
//****************************************************
#pragma mark * compilation directives *

//****************************************************
#pragma mark -
#pragma mark * includes & imports *

#include <Carbon/Carbon.h>
#include "KillEveryOneButMe.h"

//****************************************************
#pragma mark -
#pragma mark * typedef's, struct's, enums, defines, etc. *

//****************************************************
#pragma mark -
#pragma mark * local (static) function prototypes *

static pascal OSErr Handle_OpenApplication(const AppleEvent *inAppleEvent, AppleEvent *outAppleEvent, long inHandlerRefcon);
static pascal OSErr Handle_ReopenApplication(const AppleEvent *inAppleEvent, AppleEvent *outAppleEvent, long inHandlerRefcon);
static pascal OSErr Handle_OpenDocuments(const AppleEvent *inAppleEvent, AppleEvent *outAppleEvent, long inHandlerRefcon);
static pascal OSErr Handle_PrintDocuments(const AppleEvent *inAppleEvent, AppleEvent *outAppleEvent, long inHandlerRefcon);
static void Install_AppleEventHandlers(void);

static pascal OSStatus Handle_CommandUpdateStatus(EventHandlerCallRef inHandlerCallRef, EventRef inEvent, void *inUserData);
static pascal OSStatus Handle_CommandProcess(EventHandlerCallRef inHandlerCallRef, EventRef inEvent, void *inUserData);

static pascal void ClockTask(EventLoopTimerRef inTimer, void *inUserData);

static void Do_Preferences(void);
static OSStatus Do_NewWindow(void);

static OSStatus Do_CleanUp(void);

//****************************************************
#pragma mark -
#pragma mark * exported globals *

//****************************************************
#pragma mark -
#pragma mark * local (static) globals *

static IBNibRef gIBNibRef;
static UInt32 gWindowCount = 0;

//****************************************************
#pragma mark -
#pragma mark * exported function implementations *

/*****************************************************
*
* main (argc, argv) 
*
* Purpose:  main program entry point
*
* Notes:	   You might want to change this to something more verbose
*
* Inputs:   argc     - the number of elements in the argv array
*			argv     - an array of pointers to the parameters to this application
*
* Returns:  int      - error code (0 == no error) 
*/
int main(int argc, char* argv[])
	{
	OSStatus status;
	
	// Can we run this particular demo application?
	long response;
	status = Gestalt(gestaltSystemVersion, &response);
	Boolean ok = ((noErr == status) && (response >= 0x00001040));
	if (!ok)
		{
		DialogRef theAlert;
		CreateStandardAlert(kAlertStopAlert, CFSTR("Mac OS X 10.4 (minimum) is required for this application"), NULL, NULL, &theAlert);
		RunStandardAlert(theAlert, NULL, NULL);
		ExitToShell();
		}
	
	// Create a Nib reference passing the name of the nib file (without the .nib extension)
	// CreateNibReference only searches into the application bundle.
	status = CreateNibReference(CFSTR("main"), &gIBNibRef);
	require_noerr(status, CantGetNibRef);
	
	// Once the nib reference is created, set the menu bar. "MainMenu" is the name of the menu bar
	// object. This name is set in InterfaceBuilder when the nib is created.
	status = SetMenuBarFromNib(gIBNibRef, CFSTR("MenuBar"));
	require_noerr(status, CantSetMenuBar);
	
	// Enabling Preferences menu item
	EnableMenuCommand(NULL, kHICommandPreferences);
	
	// Let's react to User's commands.
	Install_AppleEventHandlers();
	
	EventTypeSpec eventTypeCP = {kEventClassCommand, kEventCommandProcess};
	InstallEventHandler(GetApplicationEventTarget(), Handle_CommandProcess, 1, &eventTypeCP, NULL, NULL);
	
	EventTypeSpec eventTypeCUS = {kEventClassCommand, kEventCommandUpdateStatus};
	InstallEventHandler(GetApplicationEventTarget(), Handle_CommandUpdateStatus, 1, &eventTypeCUS, NULL, NULL);
	
    EventLoopTimerRef timerRef;
	status = InstallEventLoopTimer(GetCurrentEventLoop(), ((EventTime)2.0), ((EventTime)0.5), ClockTask, 0, &timerRef);
	require_noerr(status, InstallEventLoopTimer);

	// Call the event loop
	RunApplicationEventLoop();
	
InstallEventLoopTimer:
CantSetMenuBar:
CantGetNibRef:
	return status;
	}   // main

/*****************************************************/
#pragma mark -
#pragma mark * local (static) function implementations *
#pragma mark * AppleEvent Handlers *

/*****************************************************
*
* Handle_OpenApplication(inAppleEvent, reply, inHandlerRefcon) 
*
* Purpose:  AppleEvent handler for the kAEOpenApplication event
*
* Inputs:   inAppleEvent    - the Apple event
*           reply            - our reply to the Apple event
*           inHandlerRefcon    - refcon passed to AEInstallEventHandler when this hander was installed
*
* Returns:  OSErr            - error code (0 == no error) 
*/
static pascal OSErr Handle_OpenApplication(const AppleEvent *inAppleEvent, AppleEvent *outAppleEvent, long inHandlerRefcon)
	{
	return Do_NewWindow(); // create an empty window
	}   // Handle_OpenApplication

/*****************************************************
*
* Handle_ReopenApplication(inAppleEvent, reply, inHandlerRefcon) 
*
* Purpose:  AppleEvent handler for the kAEReopenApplication event
*
* Inputs:   inAppleEvent    - the Apple event
*           reply            - our reply to the Apple event
*           inHandlerRefcon    - refcon passed to AEInstallEventHandler when this hander was installed
*
* Returns:  OSErr            - error code (0 == no error) 
*/
static pascal OSErr Handle_ReopenApplication(const AppleEvent *inAppleEvent, AppleEvent *outAppleEvent, long inHandlerRefcon)
	{
	// We were already running but with no windows so we create an empty one.
	WindowRef theWindow = GetFrontWindowOfClass(kDocumentWindowClass, true);
	if (theWindow == NULL)
		return Do_NewWindow();
	else
		return noErr;
	}   // Handle_ReopenApplication

/*****************************************************
*
* Handle_OpenDocuments(inAppleEvent, reply, inHandlerRefcon) 
*
* Purpose:  AppleEvent handler for the kAEOpenDocuments event
*
* Inputs:   inAppleEvent    - the Apple event
*           reply            - our reply to the Apple event
*           inHandlerRefcon    - refcon passed to AEInstallEventHandler when this hander was installed
*
* Returns:  OSErr            - error code (0 == no error) 
*/
static pascal OSErr Handle_OpenDocuments(const AppleEvent *inAppleEvent, AppleEvent *outAppleEvent, long inHandlerRefcon)
	{
	return errAEEventNotHandled;
	}   // Handle_OpenDocuments

/*****************************************************
*
* Handle_PrintDocuments(inAppleEvent, reply, inHandlerRefcon) 
*
* Purpose:  AppleEvent handler for the kAEPrintDocuments event
*
* Inputs:   inAppleEvent	- the Apple event
*           reply           - our reply to the Apple event
*           inHandlerRefcon - refcon passed to AEInstallEventHandler when this hander was installed
*
* Returns:  OSErr			- error code (0 == no error) 
*/
static pascal OSErr Handle_PrintDocuments(const AppleEvent *inAppleEvent, AppleEvent *outAppleEvent, long inHandlerRefcon)
	{
	return errAEEventNotHandled;
	}   // Handle_PrintDocuments

/*****************************************************
*
* Install_AppleEventHandlers(void) 
*
* Purpose:  installs the AppleEvent handlers
*
* Inputs:   none
*
* Returns:  none
*/
static void Install_AppleEventHandlers(void)
	{
	OSErr	status;
	status = AEInstallEventHandler(kCoreEventClass, kAEOpenApplication, Handle_OpenApplication, 0, false);
	require_noerr(status, CantInstallAppleEventHandlerOpenAppl);
	
	status = AEInstallEventHandler(kCoreEventClass, kAEReopenApplication, Handle_ReopenApplication, 0, false);
	require_noerr(status, CantInstallAppleEventHandlerReOpenAppl);
	
	status = AEInstallEventHandler(kCoreEventClass, kAEOpenDocuments, Handle_OpenDocuments, 0, false);
	require_noerr(status, CantInstallAppleEventHandlerOpenDocs);
	
	status = AEInstallEventHandler(kCoreEventClass, kAEPrintDocuments, Handle_PrintDocuments, 0, false);
	require_noerr(status, CantInstallAppleEventHandlerPrintDocs);
	
	// Note: Since RunApplicationEventLoop installs a Quit AE Handler, there is no need to do it here.
	
CantInstallAppleEventHandlerOpenAppl:
CantInstallAppleEventHandlerReOpenAppl:
CantInstallAppleEventHandlerOpenDocs:
CantInstallAppleEventHandlerPrintDocs:
	return;
	}   // Install_AppleEventHandlers

#pragma mark -
#pragma mark * CarbonEvent Handlers and Timers *

/*****************************************************
*
* ClockTask(inTimer, inUserData) 
*
* Purpose:  called to regularly the frontmost process list window
*
* Inputs:   inTimer             - reference to the carbon event timer
*           inUserData          - app-specified data you passed in the call to InstallEventLoopTimer
*
* Returns:  none
*/
static pascal void ClockTask(EventLoopTimerRef inTimer, void *inUserData)
{
	// we only update the frontmost window even if there are more.
	WindowRef aWindowRef = GetFrontWindowOfClass(kDocumentWindowClass, true);
	if (aWindowRef != NULL)
	{
		HIViewID textViewID = {'HITV', 100};
		HIViewRef textView;
		HIViewFindByID(HIViewGetRoot(aWindowRef), textViewID, &textView);
		if (textView != NULL)
		{
			CFStringRef processList = CFStringCreateWithProcessList();
			if (processList != NULL)
			{
				HIViewSetText(textView, processList);
				CFRelease(processList);
			}
		}
	}
}   // ClockTask

/*****************************************************
*
* Handle_CommandUpdateStatus(inHandlerCallRef, inEvent, inUserData) 
*
* Purpose:  called to update status of the commands, enabling or disabling the menu items
*
* Inputs:   inHandlerCallRef    - reference to the current handler call chain
*				inEvent             - the event
*           inUserData          - app-specified data you passed in the call to InstallEventHandler
*
* Returns:  OSStatus            - noErr indicates the event was handled
*                                 eventNotHandledErr indicates the event was not handled and the Toolbox should take over
*/
static pascal OSStatus Handle_CommandUpdateStatus(EventHandlerCallRef inHandlerCallRef, EventRef inEvent, void *inUserData)
	{
	OSStatus status = eventNotHandledErr;
	
	HICommand aCommand;
	GetEventParameter(inEvent, kEventParamDirectObject, typeHICommand, NULL, sizeof(HICommand), NULL, &aCommand);
	
	WindowRef aWindowRef = GetFrontWindowOfClass(kDocumentWindowClass, true);
		
	if (aWindowRef == NULL)
		{
		switch (aCommand.commandID)
			{
			case kHICommandClose:
				DisableMenuItem(aCommand.menu.menuRef, aCommand.menu.menuItemIndex);
				break;
			}
		}
	else
		{
		switch (aCommand.commandID)
			{
			case kHICommandClose:
				EnableMenuItem(aCommand.menu.menuRef, aCommand.menu.menuItemIndex);
				break;
			}
		}

	return status;
	}   // Handle_CommandUpdateStatus

/*****************************************************
*
* Handle_CommandProcess(inHandlerCallRef, inEvent, inUserData) 
*
* Purpose:  called to process commands from Carbon events
*
* Inputs:   inHandlerCallRef    - reference to the current handler call chain
*			inEvent             - the event
*           inUserData          - app-specified data you passed in the call to InstallEventHandler
*
* Returns:  OSStatus            - noErr indicates the event was handled
*                                 eventNotHandledErr indicates the event was not handled and the Toolbox should take over
*/
static pascal OSStatus Handle_CommandProcess(EventHandlerCallRef inHandlerCallRef, EventRef inEvent, void *inUserData)
	{
	HICommand aCommand;
	OSStatus status = eventNotHandledErr;
	
	GetEventParameter(inEvent, kEventParamDirectObject, typeHICommand, NULL, sizeof(HICommand), NULL, &aCommand);
	
	switch (aCommand.commandID)
		{
		case kHICommandPreferences:
			Do_Preferences();
			break;
		case kHICommandNew:
			status = Do_NewWindow();
			break;
		case kHICommandQuit:
			status = Do_CleanUp();
			break;
		case 'KEBM':
			status = KillEveryone(true);
			break;
		case 'KEAF':
			status = KillEveryone(false);
			break;
		}
	return status;
	}   // Handle_CommandProcess

#pragma mark -
#pragma mark * Windows *

/*****************************************************
*
* Do_Preferences(void) 
*
* Purpose:  routine to display dialog to set our applications preferences
*
* Inputs:   none
*
* Returns:  none
*/
static void Do_Preferences(void)
	{
	DialogRef theAlert;
	CreateStandardAlert(kAlertStopAlert, CFSTR("No Preferences yet!"), NULL, NULL, &theAlert);
	RunStandardAlert(theAlert, NULL, NULL);
	}   // Do_Preferences

/*****************************************************
*
* Do_NewWindow(outWindow) 
*
* Purpose:  called to create a new window, each other window will be created from APIs and the other one from Interface Builder
*
* Notes:    called by Handle_CommandProcess() ("File/New" menu item), Handle_OpenApplication(). Handle_ReopenApplication()
*
* Inputs:   none
*
* Returns:  OSStatus			- error code (0 == no error) 
*/
static OSStatus Do_NewWindow(void)
	{
	WindowRef aWindowRef = NULL;
	CFStringRef theTitle = NULL;
	CFMutableStringRef theNewTitle = NULL;
	CFStringRef processList = NULL;
	OSStatus status;
	
	// Create a window. "MainWindow" is the name of the window object. This name is set in 
	// InterfaceBuilder when the nib is created.
	status = CreateWindowFromNib(gIBNibRef, CFSTR("MainWindow"), &aWindowRef);
	require_noerr(status, CantCreateWindow);
	require(aWindowRef != NULL, CantCreateWindow);
	
	status = CopyWindowTitleAsCFString(aWindowRef, &theTitle);
	require_noerr(status, CantGetSetTitle);
	
	theNewTitle = CFStringCreateMutableCopy(NULL, 0, theTitle);
	require(theNewTitle != NULL, CantGetSetTitle);
	
	CFStringAppendFormat(theNewTitle, NULL, CFSTR(" %ld"), ++gWindowCount);
	status = SetWindowTitleWithCFString(aWindowRef, theNewTitle);
	require_noerr(status, CantGetSetTitle);
	
	// Get the process list as a CFString and insert it in the HITextView
	processList = CFStringCreateWithProcessList();
	require(processList != NULL, CFStringCreateWithProcessList);
	
	HIViewID textViewID = {'HITV', 100};
	HIViewRef textView;
	status = HIViewFindByID(HIViewGetRoot(aWindowRef), textViewID, &textView);
	require_noerr(status, HIViewFindByID);
	require(textView != NULL, HIViewFindByID);

	status = HIViewSetText(textView, processList);
	require_noerr(status, HIViewSetText);

	ShowWindow(aWindowRef);
	
	SetWindowModified(aWindowRef, false);

HIViewSetText:
HIViewFindByID:
	if (processList != NULL)
		CFRelease(processList);
CFStringCreateWithProcessList:
CantGetSetTitle:
	if (theTitle != NULL)
		CFRelease(theTitle);
	if (theNewTitle != NULL)
		CFRelease(theNewTitle);
CantAllocateWindowData:
CantCreateWindow:
	
	return status;
	}   // Do_NewWindow

/*****************************************************
*
* Do_CleanUp(void) 
*
* Purpose:  called when we get the quit event, closes all the windows.
*
* Inputs:   none
*
* Returns:  OSStatus   - eventNotHandledErr indicates that the quit process can continue
*/
static OSStatus Do_CleanUp(void)
	{
	WindowRef windowToDispose, aWindowRef = GetFrontWindowOfClass(kDocumentWindowClass, true);

	for ( ; aWindowRef != NULL; )
	{
		windowToDispose = aWindowRef;
		aWindowRef = GetNextWindowOfClass(aWindowRef, kDocumentWindowClass, true);
		
		DisposeWindow(windowToDispose);
	}
	
	return eventNotHandledErr;
	}   // Do_CleanUp
