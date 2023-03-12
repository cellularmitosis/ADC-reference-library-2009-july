/*
*	File:		main.c of DialogsToHIViews
* 
*	Contains:	Shows how to convert from an old-style resource-based dialog to
*	            a modern nib-based window with HIViews.
*
*  Note:		The project is set up so that the DEBUG macro is set to one when the "Development"
*				build style is chosen and not at all when the "Deployment" build style is chosen.
*				Thus, all the require asserts "fire" only in "Development".
*	
*	Version:	1.0
* 
*	Created:	March 22nd, 2004
*
*	Disclaimer:	IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc.
*				("Apple") in consideration of your agreement to the following terms, and your
*				use, installation, modification or redistribution of this Apple software
*				constitutes acceptance of these terms.  If you do not agree with these terms,
*				please do not use, install, modify or redistribute this Apple software.
*
*				In consideration of your agreement to abide by the following terms, and subject
*				to these terms, Apple grants you a personal, non-exclusive license, under AppleÍs
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
*	Copyright:  Copyright © 2004 Apple Computer, Inc, All Rights Reserved
*/
// ****************************************************
#pragma mark * compilation directives *
// ----------------------------------------------------
// ****************************************************
#pragma mark -
#pragma mark * includes & imports *
// ----------------------------------------------------
#include <Carbon/Carbon.h>
// ****************************************************
#pragma mark -
#pragma mark * typedef's, struct's, enums, defines, etc. *
// ----------------------------------------------------

// ****************************************************
#pragma mark -
#pragma mark * local (static) function prototypes *
// ----------------------------------------------------

static pascal OSStatus CommandProcess(EventHandlerCallRef inHandlerCallRef, EventRef inEvent, void* inUserData);
static void StartTheDialogs(void);

static DialogRef DrawDialogTheSystem6or7Way(void);
static DialogRef DrawDialogTheMacOS8or9Way(void);
static WindowRef DrawDialogTheMacOSXWay(void);
static void RunDialogTheSystem6or7Way(DialogRef theDialog);
static void RunDialogTheMacOS8or9Way(DialogRef theDialog);
static void RunDialogTheMacOSXWay(WindowRef window);
// ****************************************************
#pragma mark -
#pragma mark * exported globals *
// ----------------------------------------------------

// ****************************************************
#pragma mark -
#pragma mark * local (static) globals *
// ----------------------------------------------------
static SInt16 gUserH, gUserV;
// ****************************************************
#pragma mark -
#pragma mark * exported function implementations *
// ----------------------------------------------------

/*****************************************************
*
* Routine:	main (argc, argv)*
* Purpose:	main program entry point
*
* Inputs:	argc	 - the number of elements in the argv array
*			argv	 - an array of pointers to the parameters to this application
*
* Returns:	int		 - error code (0 == no error)
*/
int main(int argc, char* argv[])
{
	IBNibRef nibRef;
	OSStatus err;
	
	// Create a Nib reference passing the name of the nib file (without the .nib extension)
	// CreateNibReference only searches into the application bundle.
	err = CreateNibReference(CFSTR("main"), &nibRef);
	require_noerr(err, CantGetNibRef);
	
	// Once the nib reference is created, set the menu bar. "MainMenu" is the name of the menu bar
	// object. This name is set in InterfaceBuilder when the nib is created.
	err = SetMenuBarFromNib(nibRef, CFSTR("MenuBar"));
	require_noerr(err, CantSetMenuBar);
	
	// A bit of an explanation...
	WindowRef window;
	CreateWindowFromNib(nibRef, CFSTR("Explain"), &window);
	ShowWindow(window);
	
	// We don't need the nib reference anymore.
	DisposeNibReference(nibRef);
	
	// Let's react to User's commands.
	EventTypeSpec eventTypeCP = {kEventClassCommand, kEventCommandProcess};
	InstallEventHandler(GetApplicationEventTarget(), CommandProcess, 1, &eventTypeCP, NULL, NULL);
	
	// And start our dialogs
	StartTheDialogs();
	
	// Call the event loop
	RunApplicationEventLoop();
	
CantSetMenuBar:
CantGetNibRef:
		return err;
}

// ****************************************************
#pragma mark -
#pragma mark * local (static) function implementations *
// ----------------------------------------------------

/*****************************************************
*
* Routine:	CommandProcess(inHandlerCallRef, inEvent, inUserData)*
* Purpose:	called to process commands from Carbon events
*
* Inputs:	inHandlerCallRef	- reference to the current handler call chain
*			inEvent				- the event
*			inUserData			- app-specified data you passed in the call to InstallApplicationEventHandler
*
* Returns:	OSStatus			- noErr indicates the event was handled
*								  eventNotHandledErr indicates the event was not handled and the Toolbox should take over
*/
static pascal OSStatus CommandProcess(EventHandlerCallRef inHandlerCallRef, EventRef inEvent, void* inUserData)
{
	HICommand aCommand;
	OSStatus status = eventNotHandledErr;
	
	GetEventParameter(inEvent, kEventParamDirectObject, typeHICommand, NULL, sizeof(HICommand), NULL, &aCommand);
	
	switch (aCommand.commandID)
	{
		case kHICommandNew:
			StartTheDialogs();
			status = noErr;
			break;
	}
	return status;
}


static void StartTheDialogs(void)
{
	WindowRef macOSXWindow = DrawDialogTheMacOSXWay();
	DialogRef macOS8or9Dialog = DrawDialogTheMacOS8or9Way();
	DialogRef macOS6or7Dialog = DrawDialogTheSystem6or7Way();

	RunDialogTheSystem6or7Way(macOS6or7Dialog);
	RunDialogTheMacOS8or9Way(macOS8or9Dialog);
	RunDialogTheMacOSXWay(macOSXWindow);
}

#pragma mark -
#pragma mark ¥ DialogTheSystem6or7Way ¥

/*
*
 *
 * ---------------- DialogTheSystem6or7Way -----------------------------------
 *
 *
 */

static Boolean MySystem6or7DialogFilter(DialogRef theDialog, EventRecord *inEvent, DialogItemIndex *itemHit)
{
	if ((inEvent->what == keyDown) || (inEvent->what == autoKey))
	{
		char c = (inEvent->message & charCodeMask);
		
		// return or enter key?
		if ((c == kReturnCharCode) || (c == kEnterCharCode))
		{
			*itemHit = 1;
			return true;
		}
		
		// tab key or arrow keys?
		if (c == kTabCharCode) return false;
		if (c == kLeftArrowCharCode) return false;
		if (c == kRightArrowCharCode) return false;
		if (c == kUpArrowCharCode) return false;
		if (c == kDownArrowCharCode) return false;
		
		// digits only for edittext box item #9 ?
		// pre-Carbon, this would have been: ((DialogPeek)theDialog)->editField+1 == 9
		if (GetDialogKeyboardFocusItem(theDialog) == 9)
		{
			if ((c < '0') || (c > '9'))
			{
				SysBeep(1);
				return true;
			}
		}
	}
	
	// we got a click!
	if (inEvent->what == mouseDown)
	{
		DialogItemType itemType;
		Handle itemHandle;
		Rect itemBox;
		GetDialogItem(theDialog, 13, &itemType, &itemHandle, &itemBox);
		
		// is the user item enabled?
		if (!(itemType & itemDisable))
		{
			CGrafPtr savePort;
			GetPort(&savePort);
			SetPortDialogPort(theDialog);
			Point thePoint = inEvent->where;
			GlobalToLocal(&thePoint);
			Boolean inside = PtInRect(thePoint, &itemBox);
			
			// is the click inside the user item?
			if (inside)
			{
				// let's constrain and move the spot!
				// it's possible to track the spot here but it's complex
				// so we just move on the click and don't track.
				// that's typical of dialog's user items of that era.
				Rect userRect1 = {gUserV-4, gUserH-4, gUserV+4, gUserH+4};
				EraseRect(&userRect1);
				InvalWindowRect(GetDialogWindow(theDialog), &userRect1);
				gUserH = thePoint.h;
				gUserV = thePoint.v;
				if (gUserH < itemBox.left+4) gUserH = itemBox.left+4;
				if (gUserH > itemBox.right-4) gUserH = itemBox.right-4;
				if (gUserV < itemBox.top+4) gUserV = itemBox.top+4;
				if (gUserV > itemBox.bottom-4) gUserV = itemBox.bottom-4;
				Rect userRect2 = {gUserV-4, gUserH-4, gUserV+4, gUserH+4};
				InvalWindowRect(GetDialogWindow(theDialog), &userRect2);
			}
			SetPort(savePort);
		}
	}
	
	return false;
}

static void ScrollBarActionProc(ControlRef theControl, ControlPartCode partCode)
{
	SInt16 oldValue = GetControlValue(theControl);
	switch (partCode)
	{
		case kControlUpButtonPart:		SetControlValue(theControl, oldValue - 1); break;
		case kControlDownButtonPart:  SetControlValue(theControl, oldValue + 1); break;
		case kControlPageUpPart:		SetControlValue(theControl, oldValue - 10); break;
		case kControlPageDownPart:		SetControlValue(theControl, oldValue + 10); break;
	}
}

static void MyDrawUserItem(DialogRef theDialog, DialogItemIndex itemNo)
{
	DialogItemType itemType;
	Handle itemHandle;
	Rect itemBox;
	GetDialogItem(theDialog, itemNo, &itemType, &itemHandle, &itemBox);
	
	CGrafPtr savePort;
	GetPort(&savePort);
	SetPortDialogPort(theDialog);
	
	PenState penState;
	GetPenState(&penState);
	
	PenSize(3, 3);
	if (itemType & itemDisable)
	{
		Pattern gray;
		PenPat(GetQDGlobalsGray(&gray));
	}
	FrameRect(&itemBox);
	Rect userRect = {gUserV-4, gUserH-4, gUserV+4, gUserH+4};
	PaintRect(&userRect);
	
	SetPenState(&penState);
	SetPort(savePort);
}

static DialogRef DrawDialogTheSystem6or7Way(void)
{
	DialogItemType itemType;
	Handle itemHandle;
	Rect itemBox;
	
	DialogRef theDialog = GetNewDialog(256, NULL, (WindowRef)-1L);
	if (theDialog == NULL) return(NULL);
	
	// Move it!
	MoveWindow(GetDialogWindow(theDialog), 10, 271, false);
	
	// Setting the check box
	GetDialogItem(theDialog, 2, &itemType, &itemHandle, &itemBox);
	SetControlValue((ControlHandle)itemHandle, 1);
	
	// Setting a radio button
	GetDialogItem(theDialog, 3, &itemType, &itemHandle, &itemBox);
	SetControlValue((ControlHandle)itemHandle, 1);
	
	// Setting the draw proc for the user item
	GetDialogItem(theDialog, 13, &itemType, &itemHandle, &itemBox);
	gUserH = (itemBox.left + itemBox.right) / 2;
	gUserV = (itemBox.top + itemBox.bottom) / 2;
	SetDialogItem(theDialog, 13, itemType, (Handle)&MyDrawUserItem, &itemBox);
	
	// Setting the action proc for the scroll bar so that the PageUp/PageDown/Up/Down buttons work
	GetDialogItem(theDialog, 14, &itemType, &itemHandle, &itemBox);
	SetControlAction((ControlHandle)itemHandle, ScrollBarActionProc);
	
	ShowWindow(GetDialogWindow(theDialog));
	
	return(theDialog);
}

static void RunDialogTheSystem6or7Way(DialogRef theDialog)
{
	SInt16 itemHit;
	DialogItemType itemType;
	Handle itemHandle;
	Rect itemBox;
	
	BringToFront(GetDialogWindow(theDialog));
	
	do {
		ModalDialog(MySystem6or7DialogFilter, &itemHit);
		switch (itemHit)
		{
			case 2:
			{
				// we enable or disable the user item depending on whether the box is checked or not
				GetDialogItem(theDialog, itemHit, &itemType, &itemHandle, &itemBox);
				SInt16 enable = GetControlValue((ControlHandle)itemHandle);
				SetControlValue((ControlHandle)itemHandle, 1 - enable);
				GetDialogItem(theDialog, 13, &itemType, &itemHandle, &itemBox);
				SetDialogItem(theDialog, 13, enable?userItem+itemDisable:userItem, itemHandle, &itemBox);
				HideDialogItem(theDialog, 13);
				ShowDialogItem(theDialog, 13);
			}
				break;
			case 3: case 4: case 5: case 6: case 7:
			{
				// one radio button was chosen, let's adjust them all (we could also remember the last one...)
				int i;
				for (i = 3; i <= 7; i++)
				{
					GetDialogItem(theDialog, i, &itemType, &itemHandle, &itemBox);
					SetControlValue((ControlHandle)itemHandle, (i == itemHit)?1:0);
				}
			}
				break;
			case 14:
			{
				// the indicator of the scroll bar was moved so let's display the value in the first edit box
				// this is System 6 or 7 style so the controls can only handle 16 bits value (hence a max of 32767)
				GetDialogItem(theDialog, itemHit, &itemType, &itemHandle, &itemBox);
				SInt16 newValue = GetControlValue((ControlHandle)itemHandle);
				Str255 theStr;
				NumToString(newValue, theStr);
				GetDialogItem(theDialog, 9, &itemType, &itemHandle, &itemBox);
				SetDialogItemText(itemHandle, theStr);
				SelectDialogItemText(theDialog, 9, 0, 32767);
			}
				break;
		}
	} while (!(itemHit == ok));
	
	DisposeDialog(theDialog);
}

#pragma mark -
#pragma mark ¥ DialogTheMacOS8or9Way ¥

/*
*
 *
 * ---------------- DialogTheMacOS8or9Way -----------------------------------
 *
 *
 */

static void ScrollBar32BitActionProc(ControlRef theControl, ControlPartCode partCode)
{
	// this is Mac OS 8 or 9 style so the controls can now handle 32 bits value (hence a max of 2147483647)
	SInt32 oldValue = GetControl32BitValue(theControl);
	switch (partCode)
	{
		case kControlUpButtonPart:		SetControl32BitValue(theControl, oldValue - 1); break;
		case kControlDownButtonPart:  SetControl32BitValue(theControl, oldValue + 1); break;
		case kControlPageUpPart:		SetControl32BitValue(theControl, oldValue - 10); break;
		case kControlPageDownPart:		SetControl32BitValue(theControl, oldValue + 10); break;
	}
	SInt32 newValue = GetControl32BitValue(theControl);
	
	// let's display the value in the edit text control associated with this control
	theControl = (ControlRef)GetControlReference(theControl);
	Str255 theStr;
	NumToString(newValue, theStr);
	SetControlData(theControl, kControlEntireControl, kControlEditTextTextTag, theStr[0], &theStr[1]);
}

static Boolean MyMacOS8or9DialogFilter(DialogRef theDialog, EventRecord *inEvent, DialogItemIndex *itemHit)
{
	// this ModalFilterProc is much simpler than its System 6 or 7 ancestor
	// the controls used instead of the dialog items are a bit smarter and more self-standing
	if ((inEvent->what == keyDown) || (inEvent->what == autoKey))
	{
		char c = (inEvent->message & charCodeMask);
		
		// return or enter key?
		if ((c == kReturnCharCode) || (c == kEnterCharCode)) { *itemHit = 1; return true; }
		
		// tab key?
		if (c == kTabCharCode) { AdvanceKeyboardFocus(GetDialogWindow(theDialog)); return true; }
	}
	
	return false;
}

static ControlKeyFilterResult MyEditKeyFilter(ControlRef theControl, SInt16 *keyCode, SInt16 *charCode, EventModifiers *modifiers)
{
	// the edit text control can filter keys on his own
	if ((*charCode < '0') || (*charCode > '9'))
	{
		SysBeep(1);
		return kControlKeyFilterBlockKey;
	}
	return kControlKeyFilterPassKey;
}

static void MyUserPaneDrawProc(ControlRef control, SInt16 part)
{
	// we now use a User Pane Control instead of a dialog user item
	// the draw, hit test, and track are more separated
	Rect bounds;
	GetControlBounds(control, &bounds);
	
	PenSize(3, 3);
	if (!IsControlActive(control))
	{
		RGBColor gray = {32767, 32767, 32767};
		RGBForeColor(&gray);
	}
	FrameRect(&bounds);
	Rect userRect = {gUserV-4, gUserH-4, gUserV+4, gUserH+4};
	PaintRect(&userRect);
}

static ControlPartCode MyUserPaneHitTestProc(ControlRef control, Point where)
{
	Rect bounds;
	GetControlBounds(control, &bounds);
	ControlPartCode partCode = (PtInRect(where, &bounds))?kControlButtonPart:kControlNoPart;
	return partCode;
}

static void MoveSpotActionProc(ControlRef theControl, ControlPartCode partCode)
{
	Rect bounds;
	GetControlBounds(theControl, &bounds);
	Point thePoint;
	if (partCode == kControlButtonPart)
	{
		GetMouse(&thePoint);
		gUserH = thePoint.h;
		gUserV = thePoint.v;
		if (gUserH < bounds.left+4) gUserH = bounds.left+4;
		if (gUserH > bounds.right-4) gUserH = bounds.right-4;
		if (gUserV < bounds.top+4) gUserV = bounds.top+4;
		if (gUserV > bounds.bottom-4) gUserV = bounds.bottom-4;
		
		// the erasing/drawing management is very complex to do properly
		// unless we just Hide and Show. This technique is OK if the control
		// is not too big on the screen and the machine is fast enough to
		// prevent flicker. If not then a better tracking would require
		// clipping management and calling the User Pane Draw Proc.
		HideControl(theControl);
		ShowControl(theControl);
	}
}

static DialogRef DrawDialogTheMacOS8or9Way(void)
{
	short		i;
	ControlRef theControl;
	
	DialogRef theDialog = GetNewDialog(257, NULL, (WindowRef)-1L);
	if (theDialog == NULL) return(NULL);
	
	// Let's get a pulsing blue default button!
	GetDialogItemAsControl(theDialog, 1, &theControl);
	SetWindowDefaultButton(GetDialogWindow(theDialog), theControl);
	
	// Setting the check box
	GetDialogItemAsControl(theDialog, 2, &theControl);
	SetControl32BitValue(theControl, 1);
	
	// We need to autoembed our radio buttons in the radio group
	// so that they work automatically
	for (i = 4; i <= 8; i++)
	{
		GetDialogItemAsControl(theDialog, i, &theControl);
		AutoEmbedControl(theControl, GetDialogWindow(theDialog));
	}
	
	// we assign a key filter on our edit text box so that only digits can be entered
	ControlRef theTextControl;
	ControlKeyFilterUPP keyFilter = MyEditKeyFilter;
	GetDialogItemAsControl(theDialog, 9, &theTextControl);
	SetKeyboardFocus(GetDialogWindow(theDialog), theTextControl, kControlFocusNextPart);
	SetControlData(theTextControl, kControlEntireControl, kControlEditTextKeyFilterTag, sizeof(keyFilter), &keyFilter);
	
	// Setting the action proc for the scroll bar so that the PageUp/PageDown/Up/Down buttons work
	// We also associate the previous edit text box with the scroll bar so it gets updated
	GetDialogItemAsControl(theDialog, 14, &theControl);
	SetControlAction(theControl, ScrollBar32BitActionProc);
	SetControl32BitMaximum(theControl, 0x7fffffff);
	SetControlReference(theControl, (SInt32)theTextControl);
	
	// The static text control is created as a resource but we could only set its title and
	// not its content. We set the content now!
	GetDialogItemAsControl(theDialog, 15, &theControl);
	Str255 theTitle;
	GetControlTitle(theControl, theTitle);
	SetControlData(theControl, kControlEntireControl, kControlStaticTextTextTag, theTitle[0], &theTitle[1]);
	
	// We set up our User Pane Control with the draw, hit test, and track (actually action) procs
	GetDialogItemAsControl(theDialog, 13, &theControl);
	Rect bounds;
	GetControlBounds(theControl, &bounds);
	gUserH = (bounds.left + bounds.right) / 2;
	gUserV = (bounds.top + bounds.bottom) / 2;	
	ControlUserPaneDrawUPP userPaneDraw = MyUserPaneDrawProc;
	SetControlData(theControl, kControlEntireControl, kControlUserPaneDrawProcTag, sizeof(userPaneDraw), &userPaneDraw);
	ControlUserPaneHitTestUPP userPaneHitTest = MyUserPaneHitTestProc;
	SetControlData(theControl, kControlEntireControl, kControlUserPaneHitTestProcTag, sizeof(userPaneHitTest), &userPaneHitTest);
	SetControlAction(theControl, MoveSpotActionProc);
	
	ShowWindow(GetDialogWindow(theDialog));
	
	return(theDialog);
}

static void RunDialogTheMacOS8or9Way(DialogRef theDialog)
{
	SInt16 itemHit;
	ControlRef theControl;
	ControlRef  theTextControl;
	
	BringToFront(GetDialogWindow(theDialog));
	
	do {
		ModalDialog(MyMacOS8or9DialogFilter, &itemHit);
		switch (itemHit)
		{
			case 2:
			{
				// we still enable or disable the user pane depending on whether the box is checked or not
				GetDialogItemAsControl(theDialog, itemHit, &theControl);
				SInt32 enable = GetControl32BitValue(theControl);
				SetControl32BitValue(theControl, 1 - enable);
				GetDialogItemAsControl(theDialog, 13, &theControl);
				if (!enable)
					ActivateControl(theControl);
				else
					DeactivateControl(theControl);
			}
				break;
			case 9: case 10:
			{
				// we got a click in an edit text control, if didn't have the focus, let's set it
				GetDialogItemAsControl(theDialog, itemHit, &theTextControl);
				ControlRef currentFocus;
				GetKeyboardFocus(GetDialogWindow(theDialog), &currentFocus);
				if (currentFocus != theTextControl)
					SetKeyboardFocus(GetDialogWindow(theDialog), theTextControl, kControlFocusNextPart);
			}
				break;
		}
	} while (!(itemHit == ok));
	
	DisposeDialog(theDialog);
}

#pragma mark -
#pragma mark ¥ DialogTheMacOSXWay ¥

/*
*
 *
 * ---------------- DialogTheMacOSXWay -----------------------------------
 *
 *
 */

static pascal OSStatus MacOSXDialogCommandProcess(EventHandlerCallRef inHandlerCallRef, EventRef inEvent, void* inUserData)
{
	HICommand aCommand;
	OSStatus status = eventNotHandledErr;
	
	GetEventParameter(inEvent, kEventParamDirectObject, typeHICommand, NULL, sizeof(HICommand), NULL, &aCommand);
	
	switch (aCommand.commandID)
	{
		case kHICommandOK:
			// we got a valid click on the OK button so let's quit our local run loop
			QuitAppModalLoopForWindow((WindowRef) inUserData);
			break;
		case 'CBED':
		{
			// we still enable or disable the custom spot view depending on whether the box is checked or not
			HIViewRef checkBox = ((HICommandExtended *)&aCommand)->source.control;
			SInt32 enable = GetControl32BitValue(checkBox);
			HIViewID hidcsv = {0, 13};
			HIViewRef customSpotView;
			HIViewFindByID(HIViewGetRoot(GetControlOwner(checkBox)), hidcsv, &customSpotView);
			if (enable)
				ActivateControl(customSpotView);
			else
				DeactivateControl(customSpotView);
			HIViewSetNeedsDisplay(customSpotView, true);
		}
			break;
	}
	
	return status;
}

//
// We could still have used the previous UserPane control, slightly modified to
// handle the change in the coordinate system due to the fact that the window
// is now compositing, but we take the opportunity to replace it by a custom
// HIView so that we can take advantage of the CarbonEvents handling.
// As it is, we now have a custom spot view which is not "erasing" anymore, which
// can be used on top of any kind of background, and whose tracking is more
// flicker-free thanks to the delayed updating.
//
// This custom spot view necessitates a bit more code than the User Pane control
// required but most of it is common to all custom HIViews so the actual code to
// write is all in the kEventControlDraw, kEventControlBoundsChanged,
// kEventControlHitTest, and kEventControlTrack handlers.
//

#define kCustomSpotViewClassID CFSTR("com.apple.sample.dts.HICustomSpotView")

typedef struct {
	HIViewRef	view;
	HIPoint		spot;
} CustomSpotViewData;

static pascal OSStatus CustomSpotViewHandler(EventHandlerCallRef inCaller, EventRef inEvent, void* inRefcon)
{
	OSStatus result = eventNotHandledErr;
	CustomSpotViewData* myData = (CustomSpotViewData*)inRefcon;
	
	switch (GetEventClass(inEvent))
	{
		case kEventClassHIObject:
			switch (GetEventKind(inEvent))
			{
				case kEventHIObjectConstruct:
				{
					myData = (CustomSpotViewData*) calloc(1, sizeof(CustomSpotViewData));
					GetEventParameter(inEvent, kEventParamHIObjectInstance, typeHIObjectRef, NULL, sizeof(myData->view), NULL, &myData->view);
					result = SetEventParameter(inEvent, kEventParamHIObjectInstance, typeVoidPtr, sizeof(myData), &myData);
					break;
				}
					
				case kEventHIObjectInitialize:
				{
					HIRect bounds;
					GetEventParameter(inEvent, kEventParamBounds, typeHIRect, NULL, sizeof(bounds), NULL, &bounds);
					myData->spot.x = CGRectGetMidX(bounds) - CGRectGetMinX(bounds);
					myData->spot.y = CGRectGetMidY(bounds) - CGRectGetMinY(bounds);
					HIViewSetVisible(myData->view, true);
					break;
				}
					
				case kEventHIObjectDestruct:
				{
					free(myData);
					result = noErr;
					break;
				}
					
				default:
					break;
			}
			break;
			
		case kEventClassControl:
			switch (GetEventKind(inEvent))
			{
				case kEventControlDraw:
				{
					CGContextRef	context;
					HIRect			bounds;
					result = GetEventParameter(inEvent, kEventParamCGContextRef, typeCGContextRef, NULL, sizeof(context), NULL, &context);
					HIViewGetBounds(myData->view, &bounds);
					
					if (!IsControlActive(myData->view))
					{
						CGContextSetGrayStrokeColor(context, 0.5, 0.3);
						CGContextSetGrayFillColor(context, 0.5, 0.3);
					}
					else
					{
						CGContextSetRGBStrokeColor(context, 0.0, 0.0, 0.0, 0.7);
						CGContextSetRGBFillColor(context, 0.0, 0.0, 0.0, 0.7);
					}
					
					CGContextSetLineWidth(context, 3.0);
					CGContextStrokeRect(context, bounds);
					
					HIRect spot = { {myData->spot.x - 4.0, myData->spot.y - 4.0}, {8.0, 8.0} };
					CGContextFillRect(context, spot);
					
					result = noErr;
					break;
				}
					
				case kEventControlBoundsChanged:
				{
					HIRect newHIBounds;
					GetEventParameter(inEvent, kEventParamCurrentBounds, typeHIRect, NULL, sizeof(newHIBounds), NULL, &newHIBounds);
					myData->spot.x = CGRectGetMidX(newHIBounds) - CGRectGetMinX(newHIBounds);
					myData->spot.y = CGRectGetMidY(newHIBounds) - CGRectGetMinY(newHIBounds);
					break;
				}
					
				case kEventControlHitTest:
				{
					HIPoint	pt;
					HIRect	bounds;
					GetEventParameter(inEvent, kEventParamMouseLocation, typeHIPoint, NULL, sizeof(pt), NULL, &pt);
					HIViewGetBounds(myData->view, &bounds);
					ControlPartCode part = (CGRectContainsPoint(bounds, pt))?kControlButtonPart:kControlNoPart;
					result = SetEventParameter(inEvent, kEventParamControlPart, typeControlPartCode, sizeof(part), &part);
					break;
				}
					
				case kEventControlTrack:
				{
					Point qdPoint;
					Rect qdWindowBounds;
					HIPoint hiPoint;
					HIRect hiViewBounds;
					MouseTrackingResult mouseStatus = kMouseTrackingMouseDown;
					
					HIViewGetBounds(myData->view, &hiViewBounds);
					GetWindowBounds(GetControlOwner(myData->view), kWindowStructureRgn, &qdWindowBounds);
					
					// handle the first mouseDown before moving
					GetEventParameter(inEvent, kEventParamMouseLocation, typeHIPoint, NULL, sizeof(hiPoint), NULL, &hiPoint);
					
					while (mouseStatus != kMouseTrackingMouseUp)
					{
						if (CGRectContainsPoint(hiViewBounds, hiPoint))
						{
							if (hiPoint.x < hiViewBounds.origin.x+4) hiPoint.x = hiViewBounds.origin.x+4;
							if (hiPoint.x > hiViewBounds.origin.x+hiViewBounds.size.width-4) hiPoint.x = hiViewBounds.origin.x+hiViewBounds.size.width-4;
							if (hiPoint.y < hiViewBounds.origin.y+4) hiPoint.y = hiViewBounds.origin.y+4;
							if (hiPoint.y > hiViewBounds.origin.y+hiViewBounds.size.height-4) hiPoint.y = hiViewBounds.origin.y+hiViewBounds.size.height-4;
							myData->spot = hiPoint;
							HIViewSetNeedsDisplay(myData->view, true);
						}
						
						// a -1 GrafPtr to TrackMouseLocation yields global coordinates
						TrackMouseLocation((GrafPtr)-1L, &qdPoint, &mouseStatus);						
						
						// convert to window-relative coordinates
						hiPoint.x = qdPoint.h - qdWindowBounds.left;
						hiPoint.y = qdPoint.v - qdWindowBounds.top;
						
						// convert to view-relative coordinates
						HIViewConvertPoint(&hiPoint, NULL, myData->view);
					}
					break;
				}
					
					
				default:
					break;
			}
			break;
			
		default:
			break;
	}
	
	return result;
}

static WindowRef DrawDialogTheMacOSXWay(void)
{
	// Create a window. "DLOG:257" is the name of the window object. This name is set in 
	// InterfaceBuilder when the resource file is imported.
	
	OSStatus status = noErr;
	IBNibRef nibRef;
	WindowRef window  = NULL;
	
	static HIObjectClassRef	theClass;
	if (theClass == NULL)
	{
		static EventTypeSpec kFactoryEvents[] =
	{
				{ kEventClassHIObject, kEventHIObjectConstruct },
				{ kEventClassHIObject, kEventHIObjectInitialize },
				{ kEventClassHIObject, kEventHIObjectDestruct },
				{ kEventClassControl, kEventControlHitTest },
				{ kEventClassControl, kEventControlTrack },
				{ kEventClassControl, kEventControlBoundsChanged },
				{ kEventClassControl, kEventControlDraw }
	};
		HIObjectRegisterSubclass(kCustomSpotViewClassID, kHIViewClassID, 0, CustomSpotViewHandler, GetEventTypeCount(kFactoryEvents), kFactoryEvents, 0, &theClass);
	}
	
	status = CreateNibReference(CFSTR("main"), &nibRef);
	require_noerr(status, CantGetNibRef);
	
	status = CreateWindowFromNib(nibRef, CFSTR("DLOG:257"), &window);
	require_noerr(status, CantCreateWindow);
	
	// Let's react to User's commands.
	EventTypeSpec eventTypeCP = {kEventClassCommand, kEventCommandProcess};
	InstallEventHandler(GetWindowEventTarget(window), MacOSXDialogCommandProcess, 1, &eventTypeCP, window, NULL);
	
	// we still assign the key filter on our edit text box so that only digits can be entered
	HIViewID hidnst = {0, 9};
	HIViewRef numEditText;
	HIViewFindByID(HIViewGetRoot(window), hidnst, &numEditText);
	ControlKeyFilterUPP keyFilter = MyEditKeyFilter;
	SetControlData(numEditText, kControlEntireControl, kControlEditTextKeyFilterTag, sizeof(keyFilter), &keyFilter);
	SetKeyboardFocus(window, numEditText, kControlFocusNextPart);
	
	// we still set the action proc for the scroll bar so that the PageUp/PageDown/Up/Down buttons work
	// and still associate the previous edit text box with the scroll bar so it gets updated
	HIViewID hidsb = {0, 14};
	HIViewRef scrollBar;
	HIViewFindByID(HIViewGetRoot(window), hidsb, &scrollBar);
	SetControlAction(scrollBar, ScrollBar32BitActionProc);
	SetControlReference(scrollBar, (SInt32)numEditText);
	
	// Move it!
	MoveWindow(window, 854, 271, false);
	
	// and use the replacement for ModalDialog
	// one good thing is that our behaviors are no longer half-done at the dialog level
	// and half-done at the control level, they are all handled by the view handlers
	ShowWindow(window);
	
CantCreateWindow:
CantGetNibRef:
		return (window);
}

static void RunDialogTheMacOSXWay(WindowRef window)
{
	BringToFront(window);
	RunAppModalLoopForWindow(window);
	DisposeWindow(window);
	
	return;
}

