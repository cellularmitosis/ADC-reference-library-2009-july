/*
*	File:		main.c of LittleArrowsShowcase
* 
*	Contains:	The LittleArrows control does not increment or decrement automatically its value
*               when the user clicks on the up or down arrows.
*               The recommended way to deal with the incrementing or decrementing is to install
*               a ControlAction on the control.
*               This sample code shows how to proceed when the control is instantiated from a nib
*               or instantiated programmatically.
*
*   Note:		The project is set up so that the DEBUG macro is set to one when the "Development"
*				build style is chosen and not at all when the "Deployment" build style is chosen.
*				Thus, all the require asserts "fire" only in "Development".
*	
*	Version:	1.0
* 
*	Created:	10/9/06
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
*	Copyright:  Copyright © 2006 Apple Computer, Inc, All Rights Reserved
*/
//****************************************************
#pragma mark * compilation directives *

//****************************************************
#pragma mark -
#pragma mark * includes & imports *

#include <Carbon/Carbon.h>

//****************************************************
#pragma mark -
#pragma mark * typedef's, struct's, enums, defines, etc. *

typedef struct
{
	UInt32 dummyPlaceHolder;
} WindowDataRec, * WindowDataPtr;

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
static pascal OSStatus Handle_WindowCommandProcess(EventHandlerCallRef inHandlerCallRef, EventRef inEvent, void *inUserData);
static pascal OSStatus Handle_WindowIsClosing(EventHandlerCallRef inHandlerCallRef, EventRef inEvent, void *inUserData);

static void Do_Preferences(void);
static OSStatus Do_NewWindow(WindowRef *outWindow);
static OSStatus Do_NewWindowFromIB(WindowRef *outWindow);
static OSStatus Do_NewWindowFromAPI(WindowRef *outWindow);

static void LittleArrowsControlAction(ControlRef theControl, ControlPartCode partCode);

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
* Notes:	You might want to change this to something more verbose
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
	Boolean ok = ((status == noErr) && (response >= 0x00001040));
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
	require_noerr(status, CreateNibReference);
	
	// Once the nib reference is created, set the menu bar. "MainMenu" is the name of the menu bar
	// object. This name is set in InterfaceBuilder when the nib is created.
	status = SetMenuBarFromNib(gIBNibRef, CFSTR("MenuBar"));
	require_noerr(status, SetMenuBarFromNib);
	
	// Enabling Preferences menu item
	EnableMenuCommand(NULL, kHICommandPreferences);
	
	// Let's react to User's commands.
	Install_AppleEventHandlers();
	
	EventTypeSpec eventTypeCP = {kEventClassCommand, kEventCommandProcess};
	status = InstallEventHandler(GetApplicationEventTarget(), Handle_CommandProcess, 1, &eventTypeCP, NULL, NULL);
	require_noerr(status, InstallEventHandler);
	
	EventTypeSpec eventTypeCUS = {kEventClassCommand, kEventCommandUpdateStatus};
	status = InstallEventHandler(GetApplicationEventTarget(), Handle_CommandUpdateStatus, 1, &eventTypeCUS, NULL, NULL);
	require_noerr(status, InstallEventHandler);
	
	// Call the event loop
	RunApplicationEventLoop();
	
InstallEventHandler:
SetMenuBarFromNib:
CreateNibReference:

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
* Inputs:   inAppleEvent     - the Apple event
*           reply            - our reply to the Apple event
*           inHandlerRefcon  - refcon passed to AEInstallEventHandler when this hander was installed
*
* Returns:  OSErr            - error code (0 == no error) 
*/
static pascal OSErr Handle_OpenApplication(const AppleEvent *inAppleEvent, AppleEvent *outAppleEvent, long inHandlerRefcon)
{
	return Do_NewWindow(NULL); // create an empty window
}   // Handle_OpenApplication

/*****************************************************
*
* Handle_ReopenApplication(inAppleEvent, reply, inHandlerRefcon) 
*
* Purpose:  AppleEvent handler for the kAEReopenApplication event
*
* Inputs:   inAppleEvent     - the Apple event
*           reply            - our reply to the Apple event
*           inHandlerRefcon  - refcon passed to AEInstallEventHandler when this hander was installed
*
* Returns:  OSErr            - error code (0 == no error) 
*/
static pascal OSErr Handle_ReopenApplication(const AppleEvent *inAppleEvent, AppleEvent *outAppleEvent, long inHandlerRefcon)
{
	// We were already running but with no windows so we create an empty one.
	WindowRef theWindow = GetFrontWindowOfClass(kDocumentWindowClass, true);
	if (theWindow == NULL)
		return Do_NewWindow(NULL);
	else
		return noErr;
}   // Handle_ReopenApplication

/*****************************************************
*
* Handle_OpenDocuments(inAppleEvent, reply, inHandlerRefcon) 
*
* Purpose:  AppleEvent handler for the kAEOpenDocuments event
*
* Inputs:   inAppleEvent     - the Apple event
*           reply            - our reply to the Apple event
*           inHandlerRefcon  - refcon passed to AEInstallEventHandler when this hander was installed
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
* Inputs:   inAppleEvent     - the Apple event
*           reply            - our reply to the Apple event
*           inHandlerRefcon  - refcon passed to AEInstallEventHandler when this hander was installed
*
* Returns:  OSErr            - error code (0 == no error) 
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
#pragma mark * CarbonEvent Handlers *

/*****************************************************
*
* Handle_CommandUpdateStatus(inHandlerCallRef, inEvent, inUserData) 
*
* Purpose:  called to update status of the commands, enabling or disabling the menu items
*
* Inputs:   inHandlerCallRef    - reference to the current handler call chain
*			inEvent             - the event
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
	OSStatus status = eventNotHandledErr;
	
	HICommand aCommand;
	GetEventParameter(inEvent, kEventParamDirectObject, typeHICommand, NULL, sizeof(HICommand), NULL, &aCommand);
	
	switch (aCommand.commandID)
	{
		case kHICommandPreferences:
			Do_Preferences();
			break;
		case kHICommandNew:
			status = Do_NewWindow(NULL);
			break;
		case kHICommandQuit:
			status = Do_CleanUp();
			break;
	}
	return status;
}   // Handle_CommandProcess

/*****************************************************
*
* Handle_WindowCommandProcess(inHandlerCallRef, inEvent, inUserData) 
*
* Purpose:  called to process commands from the window controls
*
* Inputs:   inHandlerCallRef    - reference to the current handler call chain
*			inEvent             - the event
*           inUserData          - app-specified data you passed in the call to InstallEventHandler
*
* Returns:  OSStatus            - noErr indicates the event was handled
*                                 eventNotHandledErr indicates the event was not handled and the Toolbox should take over
*/
static pascal OSStatus Handle_WindowCommandProcess(EventHandlerCallRef inHandlerCallRef, EventRef inEvent, void *inUserData)
{
	OSStatus status = eventNotHandledErr;
	WindowRef aWindowRef = (WindowRef)inUserData;

	HICommand aCommand;	
	GetEventParameter(inEvent, kEventParamDirectObject, typeHICommand, NULL, sizeof(HICommand), NULL, &aCommand);
	
	switch (aCommand.commandID)
	{
		case kHICommandOK:
			DisposeWindow(aWindowRef);
			status = noErr;
			break;
	}

	return status;
}   // Handle_WindowCommandProcess

/*****************************************************
*
* Handle_PostLittleArrowsClick(inHandlerCallRef, inEvent, inUserData) 
*
* Purpose:  called to update the static text with the current value of the little arrows control
*
* Inputs:   inHandlerCallRef    - reference to the current handler call chain
*			inEvent             - the event
*           inUserData          - app-specified data you passed in the call to InstallEventHandler
*
* Returns:  OSStatus            - noErr indicates the event was handled
*                                 eventNotHandledErr indicates the event was not handled and the Toolbox should take over
*/
static pascal OSStatus Handle_PostLittleArrowsClick(EventHandlerCallRef inHandlerCallRef, EventRef inEvent, void *inUserData)
{
	OSStatus status = eventNotHandledErr;
	ControlRef littleArrows = (ControlRef)inUserData;

	SInt32 value = GetControl32BitValue(littleArrows);
	
	HIViewID staticTextID = { 'STTC', 100 };
	HIViewRef staticText;
	status = HIViewFindByID(HIViewGetRoot(GetControlOwner(littleArrows)), staticTextID, &staticText);
	require_noerr(status, HIViewFindByID);
	require(littleArrows != NULL, HIViewFindByID);
	
	CFStringRef theValueStr = CFStringCreateWithFormat(NULL, NULL, CFSTR("%ld"), value);
	require(theValueStr != NULL, CFStringCreateWithFormat);

	HIViewSetText(staticText, theValueStr);
	CFRelease(theValueStr);

CFStringCreateWithFormat:
HIViewFindByID:

	if (status == noErr)
		status = eventNotHandledErr;

	return status;
}   // Handle_PostLittleArrowsClick

/*****************************************************
*
* Handle_WindowIsClosing(inHandlerCallRef, inEvent, inUserData) 
*
* Purpose:  called as notification that a window is being destroyed
*
* Inputs:   inHandlerCallRef    - reference to the current handler call chain
*			inEvent             - the event
*           inUserData          - app-specified data you passed in the call to InstallEventHandler
*
* Returns:  OSStatus            - noErr indicates the event was handled
*                                 eventNotHandledErr indicates the event was not handled and the Toolbox should take over
*/
static pascal OSStatus Handle_WindowIsClosing(EventHandlerCallRef inHandlerCallRef, EventRef inEvent, void *inUserData)
{
	OSStatus status = eventNotHandledErr;
	WindowRef aWindowRef = (WindowRef)inUserData;

	// freeing the private data associated with the window
	WindowDataPtr wdr = (WindowDataPtr)GetWRefCon(aWindowRef);
	require(wdr != NULL, CantGetData);
	
	free(wdr);
	
CantGetData:

	return status;
}   // Handle_WindowIsClosing

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
* Inputs:   outWindow   - if not NULL, the address where to return the WindowRef
*                       - if not NULL, the callee will have to ShowWindow
*
* Returns:  OSStatus    - error code (0 == no error) 
*/
static OSStatus Do_NewWindow(WindowRef * outWindow)
{
	if ((gWindowCount % 2) == 0)
		return Do_NewWindowFromIB(outWindow);
	else
		return Do_NewWindowFromAPI(outWindow);
}   // Do_NewWindow

/*****************************************************
*
* Do_NewWindowFromIB(outWindow) 
*
* Purpose:  called to create a new window that has been constructed with Interface Builder
*
* Notes:    called by Do_NewWindow()
*
* Inputs:   outWindow   - if not NULL, the address where to return the WindowRef
*                       - if not NULL, the callee will have to ShowWindow
*
* Returns:  OSStatus    - error code (0 == no error) 
*/
static OSStatus Do_NewWindowFromIB(WindowRef * outWindow)
{
	OSStatus status;
	WindowRef aWindowRef = NULL;
	CFStringRef theTitle = NULL;
	CFMutableStringRef theNewTitle = NULL;
	
	// Create a window. "MainWindow" is the name of the window object. This name is set in 
	// InterfaceBuilder when the nib is created.
	status = CreateWindowFromNib(gIBNibRef, CFSTR("MainWindow"), &aWindowRef);
	require_noerr(status, CreateWindowFromNib);
	require(aWindowRef != NULL, CreateWindowFromNib);
	
	WindowDataPtr wdr = (WindowDataPtr)calloc(1, sizeof(WindowDataRec));
	require(wdr != NULL, CantAllocateWindowData);

	SetWRefCon(aWindowRef, (long)wdr);
	
	status = CopyWindowTitleAsCFString(aWindowRef, &theTitle);
	require_noerr(status, CopyWindowTitleAsCFString);
	
	theNewTitle = CFStringCreateMutableCopy(NULL, 0, theTitle);
	require(theNewTitle != NULL, CFStringCreateMutableCopy);
	
	CFStringAppendFormat(theNewTitle, NULL, CFSTR(" %ld"), ++gWindowCount);
	status = SetWindowTitleWithCFString(aWindowRef, theNewTitle);
	require_noerr(status, SetWindowTitleWithCFString);
	
	EventTypeSpec eventTypeCP = {kEventClassCommand, kEventCommandProcess};
	status = InstallWindowEventHandler(aWindowRef, Handle_WindowCommandProcess, 1, &eventTypeCP, (void *)aWindowRef, NULL);
	require_noerr(status, CantInstallEventHandler);
	
	EventTypeSpec eventTypeWC = {kEventClassWindow, kEventWindowClosed};
	status = InstallWindowEventHandler(aWindowRef, Handle_WindowIsClosing, 1, &eventTypeWC, (void *)aWindowRef, NULL);
	require_noerr(status, CantInstallEventHandler);
	
	HIViewID littleArrowsId = { 'LARC', 100 };
	HIViewRef littleArrows;
	status = HIViewFindByID(HIViewGetRoot(aWindowRef), littleArrowsId, &littleArrows);
	require_noerr(status, HIViewFindByID);
	require(littleArrows != NULL, HIViewFindByID);

	SetControlAction(littleArrows, LittleArrowsControlAction);

	EventTypeSpec eventTypeCVFC = {kEventClassControl, kEventControlValueFieldChanged};
	status = HIViewInstallEventHandler(littleArrows, Handle_PostLittleArrowsClick, 1, &eventTypeCVFC, (void *)littleArrows, NULL);
	require_noerr(status, CantInstallEventHandler);

	// The window was created hidden so show it if the outWindow parameter is NULL, 
	// if it's not, it will be the responsibility of the caller to show it.
	if (outWindow == NULL)
		ShowWindow(aWindowRef);
	
	SetWindowModified(aWindowRef, false);

HIViewFindByID:
CantInstallEventHandler:
SetWindowTitleWithCFString:
CFStringCreateMutableCopy:
CopyWindowTitleAsCFString:

	if (theTitle != NULL)
		CFRelease(theTitle);
	if (theNewTitle != NULL)
		CFRelease(theNewTitle);

CantAllocateWindowData:
CreateWindowFromNib:
	
	if (outWindow != NULL)
		*outWindow = aWindowRef;
	
	return status;
}   // Do_NewWindowFromIB

/*****************************************************
*
* Do_NewWindowFromAPI(outWindow) 
*
* Purpose:  called to create a new window using only API calls fron MacWindows.h, Controls.h, and HIView.h
*
* Notes:    called by Do_NewWindow()
*
* Inputs:   outWindow   - if not NULL, the address where to return the WindowRef
*                       - if not NULL, the callee will have to ShowWindow
*
* Returns:  OSStatus    - error code (0 == no error) 
*/
static OSStatus Do_NewWindowFromAPI(WindowRef * outWindow)
{
	WindowRef aWindowRef = NULL;
	CFStringRef theTitle = NULL;
	OSStatus status;
	
	// Create a window
	Rect bounds = {0, 0, 360, 480};
	status = CreateNewWindow(kDocumentWindowClass, kWindowStandardFloatingAttributes | kWindowStandardHandlerAttribute | kWindowCompositingAttribute | kWindowMetalAttribute, &bounds, &aWindowRef);
	require_noerr(status, CreateNewWindow);
	require(aWindowRef != NULL, CreateNewWindow);
	
	status = RepositionWindow(aWindowRef, NULL, kWindowCascadeOnMainScreen);
	require_noerr(status, RepositionWindow);
	
	HIViewRef contentView;
	status = HIViewFindByID(HIViewGetRoot(aWindowRef), kHIViewWindowContentID, &contentView);
	require_noerr(status, HIViewFindByID);
	
	ControlRef aControlRef;
	Rect statBounds = {113, 20, 177, 460};
	ControlFontStyleRec style = {kControlUseJustMask, 0, 0, 0, 0, teJustCenter};
	status = CreateStaticTextControl(NULL, &statBounds, CFSTR("Click in the LittleArrows control above."), &style, &aControlRef);
	require_noerr(status, CreateStaticTextControl);
	HIViewID staticTextID = { 'STTC', 100 };
	HIViewSetID(aControlRef, staticTextID);
	status = HIViewAddSubview(contentView, aControlRef);
	require_noerr(status, HIViewAddSubview);
	
	Rect littleArrowBounds = {51, 234, 73, 247};
	status = CreateLittleArrowsControl(NULL, &littleArrowBounds, 0, 0, 10, 1, &aControlRef);
	require_noerr(status, CreateLittleArrowsControl);
	status = HIViewAddSubview(contentView, aControlRef);
	require_noerr(status, HIViewAddSubview);

	SetControlAction(aControlRef, LittleArrowsControlAction);

	EventTypeSpec eventTypeCVFC = {kEventClassControl, kEventControlValueFieldChanged};
	status = HIViewInstallEventHandler(aControlRef, Handle_PostLittleArrowsClick, 1, &eventTypeCVFC, (void *)aControlRef, NULL);
	require_noerr(status, CantInstallEventHandler);

	Rect buttonBounds = {320, 390, 340, 460};
	status = CreatePushButtonControl(NULL, &buttonBounds, CFSTR("OK"), &aControlRef);
	require_noerr(status, CreatePushButtonControl);
	status = SetControlCommandID(aControlRef, kHICommandOK);
	require_noerr(status, SetControlCommandID);
	status = HIViewAddSubview(contentView, aControlRef);
	require_noerr(status, HIViewAddSubview);
	status = SetWindowDefaultButton(aWindowRef, aControlRef);
	require_noerr(status, SetWindowDefaultButton);
	
	WindowDataPtr wdr = (WindowDataPtr)calloc(1, sizeof(WindowDataRec));
	require(wdr != NULL, CantAllocateWindowData);

	SetWRefCon(aWindowRef, (long)wdr);
	
	theTitle = CFStringCreateWithFormat(NULL, NULL, CFSTR("LittleArrowsShowcase Window From API #%ld"), ++gWindowCount);
	require(theTitle != NULL, CFStringCreateWithFormat);
	
	status = SetWindowTitleWithCFString(aWindowRef, theTitle);
	require_noerr(status, SetWindowTitleWithCFString);
	
	EventTypeSpec eventTypeCP = {kEventClassCommand, kEventCommandProcess};
	status = InstallWindowEventHandler(aWindowRef, Handle_WindowCommandProcess, 1, &eventTypeCP, (void *)aWindowRef, NULL);
	require_noerr(status, CantInstallEventHandler);
	
	EventTypeSpec eventTypeWC = {kEventClassWindow, kEventWindowClosed};
	status = InstallWindowEventHandler(aWindowRef, Handle_WindowIsClosing, 1, &eventTypeWC, (void *)aWindowRef, NULL);
	require_noerr(status, CantInstallEventHandler);
		
	// The window was created hidden so show it if the outWindow parameter is NULL, 
	// if it's not, it will be the responsibility of the caller to show it.
	if (outWindow == NULL)
		ShowWindow(aWindowRef);
	
	SetWindowModified(aWindowRef, false);
	
CantInstallEventHandler:
SetWindowTitleWithCFString:
CFStringCreateWithFormat:

	if (theTitle != NULL)
		CFRelease(theTitle);

CantAllocateWindowData:
SetWindowDefaultButton:
SetControlCommandID:
CreatePushButtonControl:
CreateLittleArrowsControl:
HIViewAddSubview:
CreateStaticTextControl:
HIViewFindByID:
RepositionWindow:
CreateNewWindow:
	
	if (outWindow != NULL)
		*outWindow = aWindowRef;
	
	return status;
}   // Do_NewWindowFromAPI

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

/*****************************************************
*
* LittleArrowsControlAction(theControl, partCode) 
*
* Purpose:  called to handle the little arrows control increment and decrement.
*
* Inputs:   theControl          - the littleArrows control
*           partCode            - part which was clicked
*
* Returns:  none
*/
static void LittleArrowsControlAction(ControlRef theControl, ControlPartCode partCode)
{
	// Grabbing the increment set up by Interface Builder or CreateLittleArrowsControl.
	// Defaulting to 1 if not found.
	SInt32 increment;
	OSStatus status = GetControlData(theControl, kControlEntireControl, kControlLittleArrowsIncrementValueTag, sizeof(increment), &increment, NULL);
	if (status != noErr)
		increment = 1;
	
	SInt32 value = GetControl32BitValue(theControl);

	switch (partCode)
	{
		case kControlUpButtonPart:
			SetControl32BitValue(theControl, value + increment);
			break;

		case kControlDownButtonPart:
			SetControl32BitValue(theControl, value - increment);
			break;
	}

}   // LittleArrowsControlAction
