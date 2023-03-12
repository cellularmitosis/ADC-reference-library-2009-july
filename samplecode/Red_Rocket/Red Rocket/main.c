/*
 *  main.c
 *  Red Rocket
 *
 *  Created by ggs on Fri May 11 2001.

	Copyright:	Copyright © 2001 Apple Computer, Inc., All Rights Reserved

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
 *
 */

#include <Carbon/Carbon.h>

#include "Sprite_Window.h"

#include "HID_Utilities_External.h"

// ==================================

// HID Input
enum
{
    kActionXAxis,
	kActionRotLeft,
	kActionRotRight,
    kActionThrust,
    kActionZin,
    kActionZout,
	kNumActions
};

struct actionRec
{
	UInt32 keyCode;
	char charCode;
	Boolean keyState;
    pRecElement pElement;
    pRecDevice pDevice;
    long value;
};
typedef struct actionRec actionRec;
typedef actionRec * pActionRec;

actionRec actionArray[kNumActions];
Boolean gfDevice = true;

// ---------------------

enum
{
    kTextureSizeMenuNum = 4,
    kTextureScaleMenuNum = 5,
    kMainMenu = 500,
    kTextureScaleMenu = 1000,
    kTextureSizeMenu = 2000
};

EventHandlerUPP gEvtHandler;			// main event handler
EventHandlerUPP gWinEvtHandler;			// window event handler
EventHandlerUPP gConfigEvtHandler;
EventLoopTimerRef gTimer = NULL;

WindowRef gWindow = NULL;
WindowRef gConfigWindow = NULL;

short gFileRefNum = 0;
recImage * gImage = NULL;
IBNibRef nibRef = NULL;

// ==================================

    static void HandleWindowUpdate(WindowRef window);
    static pascal OSStatus myWindowEvtHndlr (EventHandlerCallRef myHandler, EventRef event, void* userData);
    pascal void IdleTimer (EventLoopTimerRef inTimer, void* userData);
    EventLoopTimerUPP GetTimerUPP (void);
    static pascal OSStatus myEvtHndlr (EventHandlerCallRef myHandler, EventRef event, void* userData);

// ==================================

static OSStatus SaveConfig (void)
{
    FILE * fileRef;
    short a;
    fileRef = fopen ("RR config", "wb");
    if (fileRef)  {
		for (a = 0; a < kNumActions; a++) {
			fwrite ((void *) &(actionArray [a].keyCode), sizeof (UInt32), 1, fileRef);
			fwrite ((void *) &(actionArray [a].charCode), sizeof (char), 1, fileRef);
		}
		for (a = 0; a < kNumActions; a++)
				HIDSaveElementConfig (fileRef, actionArray [a].pDevice, actionArray [a].pElement, a);
		fclose (fileRef);
		return noErr;
    }
    return paramErr;
}

// ---------------------------------

static OSStatus LoadConfig (void)
{
    FILE * fileRef;
    short a;
	for (a = 0; a < kNumActions; a++) {
		actionArray [a].keyCode = 0;
		actionArray [a].charCode = 0;
		actionArray [a].keyState = 0;
		actionArray [a].pElement = NULL;
		actionArray [a].pDevice = NULL;
		actionArray [a].value = 0;
	}
    fileRef = fopen ("RR config", "rb");
    if (fileRef)  {
		for (a = 0; a < kNumActions; a++) { // read keys
			fread ((void *) &(actionArray [a].keyCode), 1, sizeof (UInt32), fileRef);
			fread ((void *) &(actionArray [a].charCode), 1, sizeof (char), fileRef);
		}
		for (a = 0; a < kNumActions; a++) {
			pRecDevice pDevice = NULL;
			pRecElement pElement = NULL;
			long action = HIDRestoreElementConfig (fileRef, &pDevice, &pElement);
			if (pDevice && pElement) { // if valid device and element
				if ((action >= 0) && (action < kNumActions)) { // if in range
					actionArray [action].pDevice = pDevice;
					actionArray [action].pElement = pElement;
					if (action == kActionXAxis) {
						pElement->userMin = -127;
						pElement->userMax = 127;
					} else if (action == kActionRotRight) {
						pElement->userMin = 0;
						pElement->userMax = 127;
					} else if (action == kActionRotLeft) {
						pElement->userMin = 0;
						pElement->userMax = -127;
					} else if (action == kActionThrust) {
						pElement->userMin = 0;
						pElement->userMax = 255;
					} else {
						pElement->userMin = 0;
						pElement->userMax = 1;
					}
				}
			}
		}
		fclose (fileRef);
		return noErr;
    }
	else //clear input
	
    return paramErr;
}

// ---------------------------------

static void GetKeyInput (EventRef event, Boolean keyDown)
{
    short a;
	UInt32 keyCode;
	GetEventParameter (event, kEventParamKeyCode, typeUInt32, NULL, sizeof(UInt32), NULL, &keyCode);
    for (a = 0; a < kNumActions; a++) // check each action
    {
		if ((actionArray [a].keyCode || actionArray [a].charCode) &&
		    (keyCode == actionArray [a].keyCode))
			actionArray [a].keyState = keyDown; // if match then record state
    }
}

// ---------------------------------

static void GetInput (void)
{
    short a;
    for (a = 0; a < kNumActions; a++)
    {
		actionArray [a].value = 0;
		if (actionArray [a].pDevice && actionArray [a].pElement) { // handle device input
			actionArray [a].value = HIDGetElementValue (actionArray [a].pDevice, actionArray [a].pElement);
			if ((a == kActionXAxis) || (a == kActionThrust) || (a == kActionRotRight) || (a == kActionRotLeft))
				actionArray [a].value = HIDScaleValue (HIDCalibrateValue (actionArray [a].value, actionArray [a].pElement), actionArray [a].pElement);
			if ((a == kActionRotRight) || (a == kActionRotLeft)) // map trun left and right to rotation values 
				actionArray [kActionXAxis].value += actionArray [a].value;
		}
		if (actionArray [a].keyCode || actionArray [a].charCode) { // handle key presses
			if (actionArray [a].keyState) {
					if (a == kActionThrust)
						actionArray [a].value = 255;
					else if (a == kActionRotRight)
						actionArray [kActionXAxis].value += 127; // map trun left and right to rotation values 
					else if (a == kActionRotLeft)
						actionArray [kActionXAxis].value -= 127;
					else
						actionArray [a].value = 1;
			}
		}
			
    }
	// limit turning
	if (actionArray [kActionXAxis].value > 127)
		actionArray [kActionXAxis].value = 127;
	if (actionArray [kActionXAxis].value < -127)
		actionArray [kActionXAxis].value = -127;
}

// ---------------------------------

static void GetDeviceElementNameString (pRecDevice pDevice, pRecElement pElement, char * cstr)
{
	char cstrElement[256] = "----", cstrDevice[256] = "----";

	if (*(pElement->name))
		sprintf (cstrElement, "%s", pElement->name);
	else { // if no name
		HIDGetUsageName (pElement->usagePage, pElement->usage, cstrElement);
		if (!*cstrElement) // if not usage
			sprintf (cstrElement, "Element");
	}
	
	if (*(pDevice->product))
		sprintf (cstrDevice, "%s", pDevice->product);
	else {
	    HIDGetUsageName (pDevice->usagePage, pDevice->usage, cstrDevice);
		if (!*cstrDevice) // if usage
			sprintf (cstrDevice, "Device");
	}
	sprintf (cstr, "%s: %s",cstrDevice, cstrElement);
}

// ---------------------------------

static void SetControlText (WindowRef window, OSType sig, SInt32 id, char * text)
{
    ControlID idControl;
    ControlRef control;
    
    idControl.signature = sig;
    idControl.id = id; 
    GetControlByID (window, &idControl, &control);
    SetControlData (control, kControlNoPart, kControlStaticTextTextTag, strlen (text), text);
}

// ---------------------------------

static void SetControlNames (WindowRef window)
{
    char cstr [256];
    short a;
    for (a = 0; a < kNumActions; a++)
    {
		if (actionArray [a].pDevice && actionArray [a].pElement)
			GetDeviceElementNameString (actionArray [a].pDevice, actionArray [a].pElement, cstr);
		else
			sprintf (cstr, "---");
		SetControlText (window, 'hidc', a + 1, cstr);
		if (a > 0) // ignore keyboard x-axis
			if (actionArray [a].charCode || actionArray [a].keyCode)
				sprintf (cstr, "'%c'", actionArray [a].charCode);
			else
				sprintf (cstr, "--");
		SetControlText (window, 'keyc', a + 1, cstr);
    }
    DrawControls (window);
}

// ---------------------------------

static void SetControlRadio (WindowRef window, SInt32 id, Boolean value)
{
    ControlID idControl;
    ControlRef control;
    
    idControl.signature = 'ReRo';
    idControl.id = id; 
    GetControlByID (window, &idControl, &control);
    SetControlValue (control, value);
	// set second radio
    idControl.id = 1 - id; 
    GetControlByID (window, &idControl, &control);
    SetControlValue (control, 1 - value);
}

// ---------------------------------

static Boolean GetControlRadio (WindowRef window, SInt32 id)
{
    ControlID idControl;
    ControlRef control;
    
    idControl.signature = 'ReRo';
    idControl.id = id; 
    GetControlByID (window, &idControl, &control);
    return (Boolean) GetControlValue (control);
}

// ---------------------------------

static pascal OSStatus ConfigEvtHndlr (EventHandlerCallRef myHandler, EventRef event, void* userData)
{
#pragma unused (myHandler)
	static short actionToConfigure = -1;
    OSStatus result = eventNotHandledErr;
    pRecDevice pDevice = NULL;
    pRecElement pElement = NULL;
    UInt32 class = GetEventClass (event);
    UInt32 kind = GetEventKind (event);
	
	switch (class) {
		case kEventClassWindow:
			switch (kind) {
				case kEventWindowShown:
					SetControlNames ((WindowRef) userData);
					if ((WindowRef) userData == FrontWindow ())
						SetUserFocusWindow ((WindowRef) userData);
					break;
			}
			break;
		case kEventClassKeyboard:
			switch (kind) {
				case kEventRawKeyDown:
					if (actionToConfigure > -1)
					{
						char myCharCode;
						UInt32 myKeyCode;
						short i;
						GetEventParameter (event, kEventParamKeyMacCharCodes, typeChar, NULL, sizeof(char), NULL, &myCharCode);
						GetEventParameter (event, kEventParamKeyCode, typeUInt32, NULL, sizeof(UInt32), NULL, &myKeyCode);
						actionArray [actionToConfigure].keyCode = myKeyCode;
						actionArray [actionToConfigure].charCode = myCharCode;
						for (i = 0; i < kNumActions; i++)
							if (i != actionToConfigure)
								if (actionArray [i].keyCode == actionArray [actionToConfigure].keyCode) {
									actionArray [i].keyCode = 0;
									actionArray [i].charCode = 0;
								}
						actionToConfigure = -1;	
						SetControlNames ((WindowRef) userData);
						SetUserFocusWindow ((WindowRef) userData);
						result = noErr;
					}
					break;
			}
			break;
		case kEventClassCommand:
			switch (kind) {
				case kEventProcessCommand:
					{
						HICommand command;
						GetEventParameter (event, kEventParamDirectObject, kEventParamHICommand, NULL, sizeof(command), NULL, &command);
						if (command.commandID == 'cKey') {
							SetControlRadio ((WindowRef) userData, 0, 1); // set key config on
							result = noErr;
						}
						else if (command.commandID == 'cDev') {
							actionToConfigure = -1;  // reset key configure
							SetControlRadio ((WindowRef) userData, 1, 1); // set device config on
							result = noErr;
						}
						else if (command.commandID == 'xaxs')
						{
							if (GetControlRadio ((WindowRef) userData, 0)) { // if key is selected
								actionToConfigure = kActionXAxis;
							} else {
								if (HIDConfigureAction (&pDevice, &pElement, 10.0)) // timeout ticks
								{	
									if (abs (pElement->max - pElement->min) > 1) // it it is a axis/slider type element
									{
										actionArray [kActionXAxis].pDevice = pDevice;
										actionArray [kActionXAxis].pElement = pElement;
										pElement->userMin = -127;
										pElement->userMax = 127;
										SetControlNames ((WindowRef) userData);
									}
									else
										SysBeep (30);
								}
							}
							result = noErr;
						}
						else if (command.commandID == 'rLft')
						{
							if (GetControlRadio ((WindowRef) userData, 0)) { // if key is selected
								actionToConfigure = kActionRotLeft;
							} else {
								if (HIDConfigureAction (&pDevice, &pElement, 10.0)) // timeout seconds
								{	
									actionArray [kActionRotLeft].pDevice = pDevice;
									actionArray [kActionRotLeft].pElement = pElement;
									pElement->userMin = 0;
									pElement->userMax = -127;
									SetControlNames ((WindowRef) userData);
								}
							}
							result = noErr;
						}
						else if (command.commandID == 'rRgt')
						{
							if (GetControlRadio ((WindowRef) userData, 0)) { // if key is selected
								actionToConfigure = kActionRotRight;
							} else {
								if (HIDConfigureAction (&pDevice, &pElement, 10.0)) // timeout seconds
								{	
									actionArray [kActionRotRight].pDevice = pDevice;
									actionArray [kActionRotRight].pElement = pElement;
									pElement->userMin = 0;
									pElement->userMax = 127;
									SetControlNames ((WindowRef) userData);
								}
							}
							result = noErr;
						}
						else if (command.commandID == 'thrs')
						{
							if (GetControlRadio ((WindowRef) userData, 0)) { // if key is selected
								actionToConfigure = kActionThrust;
							} else {
								if (HIDConfigureAction (&pDevice, &pElement, 10.0)) // timeout ticks
								{	
									actionArray [kActionThrust].pDevice = pDevice;
									actionArray [kActionThrust].pElement = pElement;
									pElement->userMin = 0;
									pElement->userMax = 255;
									SetControlNames ((WindowRef) userData);
								}
							}
							result = noErr;
						}
						else if (command.commandID == 'czin')
						{
							if (GetControlRadio ((WindowRef) userData, 0)) { // if key is selected
								actionToConfigure = kActionZin;
							} else {
								if (HIDConfigureAction (&pDevice, &pElement, 10.0)) // timeout ticks
								{	
									actionArray [kActionZin].pDevice = pDevice;
									actionArray [kActionZin].pElement = pElement;
									pElement->userMin = 0;
									pElement->userMax = 1;
									SetControlNames ((WindowRef) userData);
								}
							}
							result = noErr;
						}
						else if (command.commandID == 'czot')
						{
							if (GetControlRadio ((WindowRef) userData, 0)) { // if key is selected
								actionToConfigure = kActionZout;
							} else {
								if (HIDConfigureAction (&pDevice, &pElement, 10.0)) // timeout ticks
								{	
									actionArray [kActionZout].pDevice = pDevice;
									actionArray [kActionZout].pElement = pElement;
									pElement->userMin = 0;
									pElement->userMax = 1;
									SetControlNames ((WindowRef) userData);
								}
							}
							result = noErr;
						}
						else if (command.commandID == kHICommandOK)
						{
							HideWindow ((WindowRef)userData);
							result = noErr;
						}
					}
					break;
			}
			break;
	}
    return result;
}

// ---------------------------------

static void HandleWindowUpdate(WindowRef window)
{
    GrafPtr portSave;
    GetPort (&portSave);
    SetPort (GetWindowPort (window));
    DrawGL (window); // draw content
    SetPort (portSave);
}

// ---------------------------------

static pascal OSStatus myWindowEvtHndlr (EventHandlerCallRef myHandler, EventRef event, void* userData)
{
#pragma unused (myHandler, userData)
    WindowRef			window;
    OSStatus			result = eventNotHandledErr;
    UInt32 class = GetEventClass (event);
    UInt32 kind = GetEventKind (event);

    GetEventParameter(event, kEventParamDirectObject, typeWindowRef, NULL, sizeof(window), NULL, &window);
	switch (class) {
		case kEventClassKeyboard:
			switch (kind) {
				case kEventRawKeyDown:
					GetKeyInput  (event, true);
					break;
				case kEventRawKeyUp:
					GetKeyInput  (event, false);
					break;
			}
			break;
		case kEventClassWindow:
			switch (kind) {
				case kEventWindowActivated:
					if (window == FrontWindow ())
						SetUserFocusWindow (window);
				case kEventWindowDrawContent:
					HandleWindowUpdate(window);
					break;
				case kEventWindowClose:
					DisposeWindow(window);
					break;
				case kEventWindowShown:
					BuildGLForWindow (window);
					if (window == FrontWindow ())
						SetUserFocusWindow (window);
					break;
				case kEventWindowBoundsChanged:
					ResizeGLWindow (window);
					HandleWindowUpdate (window);
					break;
				case kEventWindowResizeCompleted:
					ResizeGLWindow (window);
					break;
				case kEventWindowZoomed:
					ResizeGLWindow (window);
					break;
			}
			break;
	}
    return result;
}

// ---------------------------------

pascal void IdleTimer (EventLoopTimerRef inTimer, void* userData)
{
	#pragma unused (inTimer)
    WindowRef window = (WindowRef) userData;
    GetInput ();
    IdleWindow (window, actionArray [kActionXAxis].value, actionArray [kActionThrust].value, 
		        actionArray [kActionZin].value, actionArray [kActionZout].value);
    DrawGL (window);
}

// ---------------------------------

EventLoopTimerUPP GetTimerUPP (void)
{
	static EventLoopTimerUPP	sTimerUPP = NULL;
	
	if (sTimerUPP == NULL)
		sTimerUPP = NewEventLoopTimerUPP (IdleTimer);
	
	return sTimerUPP;
}

// ---------------------------------

static OSStatus OpenSpriteFile (void)
{
	OSStatus result = eventNotHandledErr;
	pRecWindow pWindowInfo = NULL;
	EventHandlerRef	ref;
	EventTypeSpec	list[] = { 	{ kEventClassWindow, kEventWindowShown },
								{ kEventClassWindow, kEventWindowActivated },
								{ kEventClassWindow, kEventWindowClose },
								{ kEventClassWindow, kEventWindowDrawContent },
								{ kEventClassWindow, kEventWindowResizeCompleted },
								{ kEventClassWindow, kEventWindowBoundsChanged },
								{ kEventClassWindow, kEventWindowZoomed },
								{ kEventClassKeyboard, kEventRawKeyDown },
								{ kEventClassKeyboard, kEventRawKeyUp } };
	
	pWindowInfo = (pRecWindow) NewPtrClear (sizeof (recWindow)); // memory for window record
	if (LoadDataFile (pWindowInfo)) // open sprite file and read sprites
	{
		WindowRef window = NULL;
		result = CreateWindowFromNib(nibRef, CFSTR("MainWindow"), &window); // build window
		if (window)
		{
			SetWRefCon (window, (long) pWindowInfo); // point to the window record in the ref con of the window
			InstallWindowEventHandler (window, gWinEvtHandler, GetEventTypeCount (list), list, (void*)window, &ref); // add event handler
			ShowWindow (window);
		}
	}
	return result;
}

// ---------------------------------

static pascal OSStatus myEvtHndlr (EventHandlerCallRef myHandler, EventRef event, void* userData)
{
#pragma unused (myHandler, userData)
    OSStatus result = eventNotHandledErr;
    Rect rectPort;
    pRecWindow pWindowInfo = NULL;
    WindowRef window = FrontWindow ();
    UInt32 class = GetEventClass (event);
    UInt32 kind = GetEventKind (event);
	HICommand command;

    if (window) { // do we have a window?
        GetWindowPortBounds (window, &rectPort); // get bounds for later inval
        pWindowInfo = (pRecWindow) GetWRefCon (window); // get window info
    }
	
	switch (class) {
		case kEventClassMenu:
			switch (kind) {
				case kEventMenuOpening:
					if (pWindowInfo) {
						CheckMenuItem(GetMenuHandle (500), 15, pWindowInfo->lines);
						CheckMenuItem(GetMenuHandle (500), 16, pWindowInfo->info);
						CheckMenuItem(GetMenuHandle (500), 17, pWindowInfo->grid);
					}
					break;
			}
			break;
		case kEventClassCommand:
			switch (kind) {
				case kEventProcessCommand:
					GetEventParameter (event, kEventParamDirectObject, kEventParamHICommand, NULL, sizeof(command), NULL, &command); // get command
					switch (command.commandID) {
						case 'conf':
							if (!gConfigWindow) {
								
								EventHandlerRef	ref;
								EventTypeSpec	list[] = { { kEventClassWindow, kEventWindowShown},
															{ kEventClassCommand, kEventProcessCommand },
															{ kEventClassKeyboard, kEventRawKeyDown } };
								
								result = CreateWindowFromNib(nibRef, CFSTR("ConfigWindow"), &gConfigWindow);
								gConfigEvtHandler = NewEventHandlerUPP (ConfigEvtHndlr);
								InstallWindowEventHandler (gConfigWindow, gConfigEvtHandler,  GetEventTypeCount (list), list, (void *)gConfigWindow, &ref);
								if (!gfDevice) {
									ControlID idControl;
									ControlRef control;
									idControl.signature = 'ReRo';
									idControl.id = 1; 
									GetControlByID (window, &idControl, &control);
									DisableControl (control);
								}
								SetControlRadio (gConfigWindow, 0, 1); // set key config on
							}
							ShowWindow( gConfigWindow );
							SetUserFocusWindow (gConfigWindow);
							break;
						case 'save':
							result = SaveConfig ();
							break;
						case 'load':
							result = LoadConfig ();
							break;
						case 'opnf':
							result = OpenSpriteFile ();
							break;
						case 'clsf':
							if (window) {
								DisposeGLForWindow (window);  // dump our sturctures
								DisposeWindow (window); // if it exists dump it
							}
							break;
						case 'quit':
							while (FrontWindow())
							{
								DisposeGLForWindow (FrontWindow());  // dump our sturctures
								DisposeWindow (FrontWindow()); // if it exists dump it
							}
							break;
					}
					if (pWindowInfo)  // have window info
						switch (command.commandID) {
							case 'gogo':
								pWindowInfo->go = true;
								if (!gTimer)
									InstallEventLoopTimer (GetCurrentEventLoop(), 0, 0.0001, GetTimerUPP (), (void *) window, &gTimer);
								break;
							case 'stop':
								pWindowInfo->go = false;
								if (gTimer)
									RemoveEventLoopTimer(gTimer);
								gTimer = NULL;
								break;
							case 'roro':
								gRocketRotate = 1 - gRocketRotate;
								break;
							case 'zmin':
								pWindowInfo->zoom *= 1.5;
								GetWindowPortBounds (window, &rectPort);
								InvalWindowRect (window, &rectPort);
								result = noErr;
								break;
							case 'zout':
								pWindowInfo->zoom /= 1.5;
								if (pWindowInfo->zoom < 0.10)
									pWindowInfo->zoom = 0.10;
								GetWindowPortBounds (window, &rectPort);
								InvalWindowRect (window, &rectPort);
								result = noErr;
								break;
							case 'zt11':
								pWindowInfo->zoom = 1.0;
								GetWindowPortBounds (window, &rectPort);
								InvalWindowRect (window, &rectPort);
								result = noErr;
								break;
							case 'tlin':
								pWindowInfo->lines = 1 - pWindowInfo->lines;
								GetWindowPortBounds (window, &rectPort);
								InvalWindowRect (window, &rectPort);
								result = noErr;
								break;
							case 'tinf':
								pWindowInfo->info = 1 - pWindowInfo->info;
								GetWindowPortBounds (window, &rectPort);
								InvalWindowRect (window, &rectPort);
								result = noErr;
								break;
							case 'grid':
								pWindowInfo->grid = 1 - pWindowInfo->grid;
								GetWindowPortBounds (window, &rectPort);
								InvalWindowRect (window, &rectPort);
								result = noErr;
								break;
						}
					break;
			}
			break;
	}
    return result;
}

// ==================================

int main(int argc, char* argv[])
{
    OSStatus		err;
    EventHandlerRef	ref;
    EventTypeSpec	list[] = { { kEventClassCommand,  kEventProcessCommand },
                               { kEventClassMenu,  kEventMenuOpening } };
		
    gMaxTextureSize = 256;

    // Create a Nib reference passing the name of the nib file (without the .nib extension)
    // CreateNibReference only searches into the application bundle.
    err = CreateNibReference(CFSTR("main"), &nibRef);
    require_noerr( err, CantGetNibRef );
    err = SetMenuBarFromNib(nibRef, CFSTR("MainMenu"));
    require_noerr( err, CantSetMenuBar );
	gEvtHandler = NewEventHandlerUPP(myEvtHndlr);
    InstallApplicationEventHandler (gEvtHandler, GetEventTypeCount (list), list, 0, &ref);
    gWinEvtHandler = NewEventHandlerUPP(myWindowEvtHndlr);

    
	OpenSpriteFile ();
    if (HIDBuildDeviceList (0, 0))
		gfDevice = true;
	else
		gfDevice = false;	
    LoadConfig ();
	
    // Call the event loop
    RunApplicationEventLoop();

//CantCreateWindow:
CantSetMenuBar:
CantGetNibRef:

	SaveConfig ();
	if (gfDevice)
		HIDReleaseDeviceList ();
	if (gConfigWindow)
		DisposeWindow (gConfigWindow);
    // We don't need the nib reference anymore.
    DisposeNibReference (nibRef);

	return err;
}

