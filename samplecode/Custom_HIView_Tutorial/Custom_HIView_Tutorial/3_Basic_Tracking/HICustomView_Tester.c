/*
*	File:		HICustomView_Tester.c of HICustomView_Tester
* 
*	Contains:	A window designed to help write custom HIViews.
*	
*	Version:	1.0
* 
*	Created:	2/14/05
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
*	Copyright:  Copyright © 2005 Apple Computer, Inc, All Rights Reserved
*/
//****************************************************
#pragma mark * compilation directives *

//****************************************************
#pragma mark -
#pragma mark * includes & imports *

#include "HICustomView.h"
#include "HICustomView_Tester.h"

//****************************************************
#pragma mark -
#pragma mark * typedef's, struct's, enums, defines, etc. *

const HIViewID kCustomID = { 'HCst', 100 };

const HIViewID kGroupBoxID = { 'GrpB', 100 };
const HIViewID kSetValue0ID = { 'SV00', 100 };
const HIViewID kSetValue1ID = { 'SV01', 100 };
const HIViewID kSetValue17ID = { 'SV17', 100 };
const HIViewID kSetValue1000ID = { 'SVTH', 100 };
const HIViewID kSetValueEditID = { 'SVET', 100 };
const HIViewID kCheckHiliteID = { 'CHlt', 100 };
const HIViewID kCheckEnableID = { 'CEnb', 100 };
const HIViewID kCheckActiveID = { 'CAct', 100 };
const HIViewID kCheckTestInScrollID = { 'CTiS', 100 };
const HIViewID kCheckTestAsScrollID = { 'CTaS', 100 };
const HIViewID kStaticTextID = { 'Stxt', 100 };

const HILayoutInfo kBindToParentLayout = {
	kHILayoutInfoVersionZero,
	{ { NULL, kHILayoutBindTop }, { NULL, kHILayoutBindLeft }, { NULL, kHILayoutBindBottom }, { NULL, kHILayoutBindRight } },
	{ { NULL, 0.0 }, { NULL, 0.0 } },
	{ { NULL, kHILayoutPositionNone }, { NULL, kHILayoutPositionNone } }
};

const HILayoutInfo kNoBindLayout = {
	kHILayoutInfoVersionZero,
	{ { NULL, kHILayoutBindNone }, { NULL, kHILayoutBindNone }, { NULL, kHILayoutBindNone }, { NULL, kHILayoutBindNone } },
	{ { NULL, 0.0 }, { NULL, 0.0 } },
	{ { NULL, kHILayoutPositionNone }, { NULL, kHILayoutPositionNone } }
};

//****************************************************
#pragma mark -
#pragma mark * local (static) function prototypes *

static pascal OSStatus Handle_WindowCommandProcess(EventHandlerCallRef inHandlerCallRef, EventRef inEvent, void *inUserData);

static pascal void MyValidationProc(ControlRef control);
static pascal OSStatus Handle_TextInputEvent(EventHandlerCallRef inHandlerCallRef, EventRef inEvent, void *inUserData);
static pascal OSStatus Handle_TextShouldChangeInRange(EventHandlerCallRef inHandlerCallRef, EventRef inEvent, void *inUserData);

static pascal OSStatus UserPaneHandler(EventHandlerCallRef inHandlerCallRef, EventRef inEvent, void *inUserData);

static pascal OSStatus Handle_ControlValueFieldOrHiliteChanged(EventHandlerCallRef inHandlerCallRef, EventRef inEvent, void *inUserData);

//****************************************************
#pragma mark -
#pragma mark * exported globals *

//****************************************************
#pragma mark -
#pragma mark * local (static) globals *

// window count for the window cascade (see Do_NewWindow)
static UInt32 gWindowCount = 0;

//****************************************************
#pragma mark -
#pragma mark * exported function implementations *

/*****************************************************
*
* Do_NewWindow() 
*
* Purpose:  called to create a new window that has been constructed with Interface Builder
*
* Inputs:   none
*
* Returns:  OSStatus			- error code (0 == no error) 
*/
OSStatus Do_NewWindow(void)
	{
	OSStatus status;
	static IBNibRef gIBNibRef = NULL;
	WindowRef aWindowRef = NULL;
	CFStringRef theTitle = NULL;
	CFMutableStringRef theNewTitle = NULL;
	
	if (gIBNibRef == NULL)
		{
		// Create a Nib reference passing the name of the nib file (without the .nib extension)
		// CreateNibReference only searches into the application bundle.
		status = CreateNibReference(CFSTR("main"), &gIBNibRef);
		require_noerr(status, CantGetNibRef);
		}
	require(gIBNibRef != NULL, CantGetNibRef);
	
	// Create a window. "MainWindow" is the name of the window object. This name is set in 
	// InterfaceBuilder when the nib is created.
	status = CreateWindowFromNib(gIBNibRef, CFSTR("MainWindow"), &aWindowRef);
	require_noerr(status, CantCreateWindow);
	require(NULL != aWindowRef, CantCreateWindow);
	
	// Grab the title of the window and add the window count to it
	status = CopyWindowTitleAsCFString(aWindowRef, &theTitle);
	require_noerr(status, CantGetSetTitle);
	
	theNewTitle = CFStringCreateMutableCopy(NULL, 0, theTitle);
	require(NULL != theNewTitle, CantGetSetTitle);
	
	CFStringAppendFormat(theNewTitle, NULL, CFSTR(" %ld"), ++gWindowCount);
	status = SetWindowTitleWithCFString(aWindowRef, theNewTitle);
	require_noerr(status, CantGetSetTitle);
	
	// Create the custom view to be tested and embed it in our group box control 
	HIViewRef groupBox;
	status = HIViewFindByID(HIViewGetRoot(aWindowRef), kGroupBoxID, &groupBox);
	require_noerr(status, CantFindGroupBox);
	require(groupBox != NULL, CantFindGroupBox);
	
	HIViewRef customView;
	status = HICreateCustomView(NULL, &customView);
	require_noerr(status, CantCreateCustom);
	require(customView != NULL, CantCreateCustom);
	
	HIRect groupBoxBounds;
	HIViewGetBounds(groupBox, &groupBoxBounds);
	groupBoxBounds.origin.x += 20;
	groupBoxBounds.origin.y += 34;
	groupBoxBounds.size.width -= 40;
	groupBoxBounds.size.height -= 54;
	HIViewSetFrame(customView, &groupBoxBounds);
	
	HIViewSetLayoutInfo(customView, &kBindToParentLayout);
	
	status = HIViewAddSubview(groupBox, customView);
	require_noerr(status, CantAddSubview);

	HIViewSetVisible(customView, true);
	
	// Let's react to User's commands.
	EventTypeSpec eventTypeCP = {kEventClassCommand, kEventCommandProcess};
	status = InstallWindowEventHandler(aWindowRef, Handle_WindowCommandProcess, 1, &eventTypeCP, (void *)customView, NULL);
	require_noerr(status, CantInstallEventHandler);
	
	// Let's update our static text field whenever the value or hilite of our tested custom view changes
	EventTypeSpec eventTypeCVFC[] =
		{
			{kEventClassControl, kEventControlValueFieldChanged},
			{kEventClassControl, kEventControlHiliteChanged}
		};
	status = InstallControlEventHandler(customView, Handle_ControlValueFieldOrHiliteChanged, 2, eventTypeCVFC, (void *)customView, NULL);
	require_noerr(status, CantInstallEventHandler);

	// We accept only numbers in our Edit text control
	HIViewRef editText;
	HIViewFindByID(HIViewGetRoot(GetControlOwner(customView)), kSetValueEditID, &editText);

#ifdef MAC_OS_X_VERSION_10_4
	// in Tiger, only 1 event handler is necessary for the key filtering
	EventTypeSpec eventTypeTSCIR = {kEventClassTextField, kEventTextShouldChangeInRange};
	status = InstallControlEventHandler(editText, Handle_TextShouldChangeInRange, 1, &eventTypeTSCIR, (void *)editText, NULL);
	require_noerr(status, CantInstallEventHandler);
#else
	// pre-Tiger, we need a different event handler and a validation proc to handle pastes and drops.
	ControlEditTextValidationUPP textValidation = MyValidationProc;
	SetControlData(editText, kControlEntireControl, kControlEditTextValidationProcTag, sizeof(textValidation), &textValidation);

	EventTypeSpec eventTypeTIUFKE = {kEventClassTextInput, kEventTextInputUnicodeForKeyEvent};
	status = InstallControlEventHandler(editText, Handle_TextInputEvent, 1, &eventTypeTIUFKE, (void *)editText, NULL);
	require_noerr(status, CantInstallEventHandler);
#endif

	// Finishing the custom view setup
	SetControl32BitMinimum(customView, 0);
	SetControl32BitMaximum(customView, 36);
	SetControl32BitValue(customView, 2);

	// We want window and thus our custom view to be able to respond to drops
	status = SetAutomaticControlDragTrackingEnabledForWindow(aWindowRef, true);
	require_noerr(status, SetAutomaticControlDragTrackingEnabledForWindow);

	ShowWindow(aWindowRef);
	SetWindowModified(aWindowRef, false);

SetAutomaticControlDragTrackingEnabledForWindow:
SetControlDragTrackingEnabled:
CantAddSubview:
CantCreateCustom:
CantFindGroupBox:
CantInstallEventHandler:
CantGetSetTitle:
CantAllocateWindowData:
CantCreateWindow:
CantGetNibRef:

	if (theTitle != NULL) CFRelease(theTitle);
	if (theNewTitle != NULL) CFRelease(theNewTitle);
	
	return status;
	}   // Do_NewWindow

//****************************************************
#pragma mark -
#pragma mark * local (static) function implementations *

/*****************************************************
*
* Handle_WindowCommandProcess(inHandlerCallRef, inEvent, inUserData) 
*
* Purpose:  called to handle of the events generated by the various controls of the HICustomView_Tester window
*
* Inputs:   inHandlerCallRef    - reference to the current handler call chain
*           inEvent             - the event
*           inUserData          - app-specified data you passed in the call to InstallEventHandler
*
* Returns:  OSStatus            - noErr indicates the event was handled
*                                 eventNotHandledErr indicates the event was not handled and the Toolbox should take over
*/
static pascal OSStatus Handle_WindowCommandProcess(EventHandlerCallRef inHandlerCallRef, EventRef inEvent, void *inUserData)
	{
	OSStatus status;
	HIViewRef customView = (HIViewRef)inUserData;
	
	// getting the command
	HICommandExtended aCommand;
	status = GetEventParameter(inEvent, kEventParamDirectObject, typeHICommand, NULL, sizeof(aCommand), NULL, &aCommand);
	require_noerr(status, ExitCommandProcess);
	status = eventNotHandledErr;

	// cheking that the command came from a control
	if ( ! (aCommand.attributes & kHICommandFromControl) ) goto ExitCommandProcess;

	switch (aCommand.commandID)
		{
		//
		// Asking for a refresh of the custom view
		//
		case 'SNDt':
			HIViewSetNeedsDisplay(customView, true);
			status = noErr;
			break;

		//
		// Setting the control value of the custom view
		//
		case 'SV00':
			SetControl32BitValue(customView, 0);
			status = noErr;
			break;
		case 'SV01':
			SetControl32BitValue(customView, 1);
			status = noErr;
			break;
		case 'SV17':
			SetControl32BitValue(customView, 17);
			status = noErr;
			break;
		case 'SVTH':
			SetControl32BitValue(customView, 1000);
			status = noErr;
			break;
		case 'SVet':
			{
			HIViewRef editText;
			HIViewFindByID(HIViewGetRoot(GetControlOwner(customView)), kSetValueEditID, &editText);
			char buffer[11];
			Size actualSize;
			GetControlData(editText, kControlEntireControl, kControlEditTextTextTag, 10, buffer, &actualSize);
			if (actualSize > 10) actualSize = 10;
			buffer[actualSize] = 0;
			SetControl32BitValue(customView, atoi(buffer));
			}
			status = noErr;
			break;

		//
		// Setting the state of the custom view
		//
		case 'CHlt':
			// setting the hilite to non-0 also stomps the previous hilite state if any
			// and we don't want that in our testing
			if (GetControl32BitValue(aCommand.source.control) == 1)
				HiliteControl(customView, 1);
			else
				HiliteControl(customView, 0);
			status = noErr;
			break;
		case 'CEnb':
			{
			HIViewRef hiliteControl;
			HIViewFindByID(HIViewGetRoot(GetControlOwner(customView)), kCheckHiliteID, &hiliteControl);
			if (GetControl32BitValue(aCommand.source.control) == 1)
				EnableControl(customView);
			else
				DisableControl(customView);
			UInt16 prevHilite = GetControlHilite(customView);
			if ((prevHilite == kControlInactivePart) || (prevHilite == kControlDisabledPart))
				DisableControl(hiliteControl);
			else
				EnableControl(hiliteControl);
			HIViewSetNeedsDisplay(customView, true);
			}
			status = noErr;
			break;
		case 'CAct':
			{
			HIViewRef hiliteControl;
			HIViewFindByID(HIViewGetRoot(GetControlOwner(customView)), kCheckHiliteID, &hiliteControl);
			if (GetControl32BitValue(aCommand.source.control) == 1)
				ActivateControl(customView);
			else
				DeactivateControl(customView);
			UInt16 prevHilite = GetControlHilite(customView);
			if ((prevHilite == kControlInactivePart) || (prevHilite == kControlDisabledPart))
				DisableControl(hiliteControl);
			else
				EnableControl(hiliteControl);
			HIViewSetNeedsDisplay(customView, true);
			}
			status = noErr;
			break;

		//
		// Testing the custom view in or as a scroller in a HIScrollView
		//
		case 'CTiS':
		case 'CTaS':
			if (GetControl32BitValue(aCommand.source.control) == 1)
				{
				// create a HIScrollView and install it where and as the custom view was
				HIViewRef scrollView;
				status = HIScrollViewCreate(kHIScrollViewValidOptions, &scrollView);
				require_noerr(status, ExitCommandProcess);
				
				HIRect frame;
				status = HIViewGetFrame(customView, &frame);
				require_noerr(status, ExitCommandProcess);
				status = HIViewSetFrame(scrollView, &frame);
				require_noerr(status, ExitCommandProcess);

				HIViewSetLayoutInfo(scrollView, &kBindToParentLayout);
				HIViewSetLayoutInfo(customView, &kNoBindLayout);
				
				status = HIViewAddSubview(HIViewGetSuperview(customView), scrollView);
				require_noerr(status, ExitCommandProcess);
				
				if (aCommand.commandID == 'CTiS')
					{
					// if we are testing the custom view in a scroller, we embed it in a scrolling User Pane
					// that we embed in the HIScrollView
					Rect boundsRect = {0, 0, 1000, 1000};
					HIViewRef userPane;
					status = CreateUserPaneControl(NULL, &boundsRect, kControlSupportsEmbedding, &userPane);
					require_noerr(status, ExitCommandProcess);
					
					EventTypeSpec userPaneEvents[] =
						{
							{kEventClassScrollable, kEventScrollableGetInfo},
							{kEventClassScrollable, kEventScrollableScrollTo}
						};
					InstallControlEventHandler(userPane, UserPaneHandler, 2, userPaneEvents, userPane, NULL);

					status = HIViewAddSubview(scrollView, userPane);
					require_noerr(status, ExitCommandProcess);
					status = HIViewAddSubview(userPane, customView);
					require_noerr(status, ExitCommandProcess);

					HIViewSetVisible(userPane, true);
					}
				else
					{
					// else we just embed the custom view directly in the HIScrollView
					status = HIViewAddSubview(scrollView, customView);
					require_noerr(status, ExitCommandProcess);
					}
				
				HIViewSetVisible(scrollView, true);
				
				// the 2 modes are not compatible so we disable the other check box
				HIViewRef otherCheckToDisable;
				if (aCommand.commandID == 'CTiS')
					HIViewFindByID(HIViewGetRoot(GetControlOwner(customView)), kCheckTestAsScrollID, &otherCheckToDisable);
				else
					HIViewFindByID(HIViewGetRoot(GetControlOwner(customView)), kCheckTestInScrollID, &otherCheckToDisable);
				require_noerr(status, ExitCommandProcess);
				DisableControl(otherCheckToDisable);
				// if we reach here, status is already set to noErr so we don't set it again
				}
			else
				{
				// we remove the HIScrollView and set the custom view back to where and as it was
				HIViewRef scrollView;
				if (aCommand.commandID == 'CTiS')
					scrollView = HIViewGetSuperview(HIViewGetSuperview(customView));
				else
					scrollView = HIViewGetSuperview(customView);

				status = HIViewAddSubview(HIViewGetSuperview(scrollView), customView);
				require_noerr(status, ExitCommandProcess);

				HIRect frame;
				status = HIViewGetFrame(scrollView, &frame);
				require_noerr(status, ExitCommandProcess);
				status = HIViewSetFrame(customView, &frame);
				require_noerr(status, ExitCommandProcess);

				HIViewSetLayoutInfo(customView, &kBindToParentLayout);

				// by releasing the HIScrollView, we also release the scrolling User Pane if any
				// which was embedded inside
				CFRelease(scrollView);
				
				// we renable the other check box
				HIViewRef otherCheckToEnable;
				if (aCommand.commandID == 'CTiS')
					HIViewFindByID(HIViewGetRoot(GetControlOwner(customView)), kCheckTestAsScrollID, &otherCheckToEnable);
				else
					HIViewFindByID(HIViewGetRoot(GetControlOwner(customView)), kCheckTestInScrollID, &otherCheckToEnable);
				require_noerr(status, ExitCommandProcess);
				EnableControl(otherCheckToEnable);
				// if we reach here, status is already set to noErr so we don't set it again
				}
			break;
		}

ExitCommandProcess:

	return status;
	}   // Handle_WindowCommandProcess

/*****************************************************
*
* UserPaneHandler(inHandlerCallRef, inEvent, inUserData) 
*
* Purpose:  called to handle the UserPane used as an embedder in the HIScrollView
*
* Inputs:   inHandlerCallRef    - reference to the current handler call chain
*           inEvent             - the event
*           inUserData          - app-specified data you passed in the call to InstallEventHandler
*
* Returns:  OSStatus            - noErr indicates the event was handled
*                                 eventNotHandledErr indicates the event was not handled and the Toolbox should take over
*/
static pascal OSStatus UserPaneHandler(EventHandlerCallRef inHandlerCallRef, EventRef inEvent, void * inUserData)
	{
	OSStatus status = eventNotHandledErr;
	HIViewRef userPane = (HIViewRef)inUserData;
	HIRect bounds;
	HIViewGetBounds(userPane, &bounds);
	
	// the following is a very straightforward simple yet complete implementation of the
	// kEventClassScrollable protocol which is enough to make the User Pane scrolls along its embedded content
	switch (GetEventKind(inEvent))
		{
#pragma mark *   kEventScrollableGetInfo
		case kEventScrollableGetInfo:
			{
			// we're being asked to return information about the scrolled view that we set as Event Parameters
			HISize lineSize = { 1, 1 }, imageSize = {3000, 2000};

			SetEventParameter(inEvent, kEventParamViewSize, typeHISize, sizeof(bounds.size), &bounds.size);
			SetEventParameter(inEvent, kEventParamImageSize, typeHISize, sizeof(imageSize), &imageSize);
			SetEventParameter(inEvent, kEventParamLineSize, typeHISize, sizeof(lineSize), &lineSize);
			SetEventParameter(inEvent, kEventParamOrigin, typeHIPoint, sizeof(bounds.origin), &bounds.origin);

			status = noErr;
			break;
			}

#pragma mark *   kEventScrollableScrollTo
		case kEventScrollableScrollTo:
			{
			// we're being asked to scroll, we just do a sanity check and ask for a redraw if the location is different
			HIPoint where;
			GetEventParameter(inEvent, kEventParamOrigin, typeHIPoint, NULL, sizeof(where), NULL, &where);

			if ((bounds.origin.y != where.y) || (bounds.origin.x != where.x))
				{
				HIViewSetBoundsOrigin(userPane, where.x, where.y);
				HIViewSetNeedsDisplay(userPane, true);
				}

			status = noErr;
			break;
			}
		}
	}   // UserPaneHandler

/*****************************************************
*
* MyValidationProc(theControl)
*
* Purpose:  called when a key filter can't be: after cut, paste, etc
*
* Inputs:   theControl          - the control to validate
*
* Returns:  none
*/
static pascal void MyValidationProc(ControlRef theControl)
	{
	OSStatus status;
	CFStringRef theCFString = NULL;
	UniChar *buffer = NULL;

	// Getting the text content of the control
	status = GetControlData(theControl, kControlEntireControl, kControlEditTextCFStringTag, sizeof(theCFString), &theCFString, NULL);
	require_noerr(status, ExitValidation);
	require(theCFString != NULL, ExitValidation);
	
	CFIndex i, j, len = CFStringGetLength(theCFString);
	if (len == 0) goto ExitValidation; // there's nothing to validate
	
	// Grabbing the characters as Unicode chars
	buffer = (UniChar *)malloc(len * sizeof(UniChar));
	require(buffer != NULL, ExitValidation);
	
	// Checking to see if we have only digits
	CFStringGetCharacters(theCFString, CFRangeMake(0, len), buffer);
	Boolean ok = true;
	for (i = 0; (i < len) && ok; i++)
		ok = (buffer[i] >= '0') && (buffer[i] <= '9');
	
	if (!ok)
		{
		// if not, we remove the offending characters
		// we also make sure that we restore the insertion point to the correct location.
		ControlEditTextSelectionRec textSelection;
		status = GetControlData(theControl, kControlEntireControl, kControlEditTextSelectionTag, sizeof(textSelection), &textSelection, NULL);
		require_noerr(status, ExitValidation);

		for (i = 0, j = 0; i < len; i++)
			if ((buffer[i] >= '0') && (buffer[i] <= '9')) buffer[j++] = buffer[i];

		CFRelease(theCFString);
		theCFString = CFStringCreateWithCharacters(NULL, buffer, j);
		require(theCFString != NULL, ExitValidation);
		
		status = SetControlData(theControl, kControlEntireControl, kControlEditTextCFStringTag, sizeof(theCFString), &theCFString);
		require_noerr(status, ExitValidation);
		
		textSelection.selEnd -= (len - j); textSelection.selStart = textSelection.selEnd;
		status = SetControlData(theControl, kControlEntireControl, kControlEditTextSelectionTag, sizeof(textSelection), &textSelection);
		require_noerr(status, ExitValidation);
		}

ExitValidation:

	if (buffer != NULL) free(buffer);
	if (theCFString != NULL) CFRelease(theCFString);
	
	return;
	}   // MyValidationProc

/*****************************************************
*
* Handle_TextInputEvent(inHandlerCallRef, inEvent, inUserData) 
*
* Purpose:  called to intercept keystrokes which are destined for a control
*
* Inputs:   inHandlerCallRef    - reference to the current handler call chain
*           inEvent             - the event
*           inUserData          - app-specified data you passed in the call to InstallEventHandler
*
* Returns:  OSStatus            - noErr indicates the event was handled
*                                 eventNotHandledErr indicates the event was not handled and the Toolbox should take over
*/
static pascal OSStatus Handle_TextInputEvent(EventHandlerCallRef inHandlerCallRef, EventRef inEvent, void *inUserData)
	{
	OSStatus status;
	
	UniChar ch = 0;
	status = GetEventParameter(inEvent, kEventParamTextInputSendText, typeUnicodeText, NULL, sizeof(ch), NULL, &ch);
	require_noerr(status, ExitTextInput);
	
	if ((ch == kReturnCharCode) || (ch == kEnterCharCode))
		{
		//
		// we got return or enter so we validate the edit text control by sending a command
		//
		EventRef theEvent;
		CreateEvent(NULL, kEventClassCommand, kEventCommandProcess, GetCurrentEventTime(), kEventAttributeUserEvent, &theEvent);
		HICommandExtended theCommand;
		theCommand.attributes = kHICommandFromControl;
		theCommand.commandID = 'SVet';
		theCommand.source.control = (ControlRef)inUserData;
		SetEventParameter(theEvent, kEventParamDirectObject, typeHICommand, sizeof(theCommand), &theCommand);
		SendEventToEventTarget(theEvent, GetWindowEventTarget(GetControlOwner((ControlRef)inUserData)));
		ReleaseEvent(theEvent);
		status = noErr;
		}
	if (	((ch >= '0') && (ch <= '9'))	||
			(ch == kBackspaceCharCode)		||
			(ch == kLeftArrowCharCode)		||
			(ch == kRightArrowCharCode)		||
			(ch == kUpArrowCharCode)		||
			(ch == kDownArrowCharCode)		)
		status = eventNotHandledErr;
	else
		status = noErr;

ExitTextInput:

	return status;
	}   // Handle_TextInputEvent

/*****************************************************
*
* Handle_TextShouldChangeInRange(inHandlerCallRef, inEvent, inUserData) 
*
* Purpose:  called to intercept text changes which are destined for a text control
*
* Inputs:   inHandlerCallRef    - reference to the current handler call chain
*           inEvent             - the event
*           inUserData          - app-specified data you passed in the call to InstallEventHandler
*
* Returns:  OSStatus            - noErr indicates the event was handled
*                                 eventNotHandledErr indicates the event was not handled and the Toolbox should take over
*/
static pascal OSStatus Handle_TextShouldChangeInRange(EventHandlerCallRef inHandlerCallRef, EventRef inEvent, void *inUserData)
	{
	OSStatus status;

	CFStringRef theCFString = NULL;
	status = GetEventParameter(inEvent, kEventParamCandidateText, typeCFStringRef, NULL, sizeof(theCFString), NULL, &theCFString);
	require_noerr(status, ExitShouldChange);
	require_action(theCFString != NULL, ExitShouldChange, status = userCanceledErr);
	
	UniChar *buffer = NULL;
	CFIndex i, j, len = CFStringGetLength(theCFString);
	if (len == 0) goto ExitShouldChange; // there's nothing to filter
	
	// Grabbing the characters as Unicode chars
	buffer = (UniChar *)malloc(len * sizeof(UniChar));
	require(buffer != NULL, ExitShouldChange);
	CFStringGetCharacters(theCFString, CFRangeMake(0, len), buffer);
	
	// Checking if we just have the return code
	if ((len == 1) && (buffer[0] == kReturnCharCode))
		{
		EventRef theEvent;
		CreateEvent(NULL, kEventClassCommand, kEventCommandProcess, GetCurrentEventTime(), kEventAttributeUserEvent, &theEvent);
		HICommandExtended theCommand;
		theCommand.attributes = kHICommandFromControl;
		theCommand.commandID = 'SVet';
		theCommand.source.control = (ControlRef)inUserData;
		SetEventParameter(theEvent, kEventParamDirectObject, typeHICommand, sizeof(theCommand), &theCommand);
		SendEventToEventTarget(theEvent, GetWindowEventTarget(GetControlOwner((ControlRef)inUserData)));
		ReleaseEvent(theEvent);
		// we don't want the return character to be added to the text so we abort the input
		status = userCanceledErr;
		}
	else
		{
		// Checking to see if we have only digits
		Boolean ok = true;
		for (i = 0; (i < len) && ok; i++)
			ok = (buffer[i] >= '0') && (buffer[i] <= '9');
		
		if (!ok)
			{
			// if not, we remove the offending characters
			for (i = 0, j = 0; i < len; i++)
				if ((buffer[i] >= '0') && (buffer[i] <= '9')) buffer[j++] = buffer[i];
			
			if (j == 0)
				// not a single digit in the candidate text, we abort the inout
				status = userCanceledErr;
			else
				{
				theCFString = CFStringCreateWithCharacters(NULL, buffer, j);
				require_action(theCFString != NULL, ExitShouldChange, status = userCanceledErr);

				status = SetEventParameter(inEvent, kEventParamReplacementText, typeCFStringRef, sizeof(theCFString), &theCFString);
				require_noerr(status, ExitShouldChange);
				// if we reach here, status is already set to noErr so we don't set it again
				}
			}
		else
			// only digits, we just let the HIToolbox do its job
			status = eventNotHandledErr;
		}

ExitShouldChange:

	if (buffer != NULL) free(buffer);

	return status;
	}   // Handle_TextShouldChangeInRange

/*****************************************************
*
* Handle_ControlValueFieldOrHiliteChanged(inHandlerCallRef, inEvent, inUserData) 
*
* Purpose:  called to handle the change of the value or hilite of our custom view, we update the static text field
*
* Inputs:   inHandlerCallRef    - reference to the current handler call chain
*           inEvent             - the event
*           inUserData          - app-specified data you passed in the call to InstallEventHandler
*
* Returns:  OSStatus            - noErr indicates the event was handled
*                                 eventNotHandledErr indicates the event was not handled and the Toolbox should take over
*/
static pascal OSStatus Handle_ControlValueFieldOrHiliteChanged(EventHandlerCallRef inHandlerCallRef, EventRef inEvent, void *inUserData)
	{
	OSStatus status;
	HIViewRef customView = (HIViewRef)inUserData;
	
	// Finding our static text control
	HIViewRef statText;
	status = HIViewFindByID(HIViewGetRoot(GetControlOwner(customView)), kStaticTextID, &statText);
	require_noerr(status, ExitValueFieldChanged);

	// Grabbing the fields that we are interested in
	CFStringRef theCFString = CFStringCreateWithFormat(NULL, NULL, CFSTR("Value: %ld, Min: %ld, Max: %ld, Hilite: %d"), GetControl32BitValue(customView), GetControl32BitMinimum(customView), GetControl32BitMaximum(customView), GetControlHilite(customView));
	require(theCFString != NULL, ExitValueFieldChanged);

	// Setting the text in the control
#ifdef MAC_OS_X_VERSION_10_4
	status = HIViewSetText(statText, theCFString);
#else
	status = SetControlData(statText, kControlEntireControl, kControlStaticTextCFStringTag, sizeof(theCFString), &theCFString);
#endif
	require_noerr(status, ExitValueFieldChanged);

	CFRelease(theCFString);

ExitValueFieldChanged:

	if (status == noErr) status = eventNotHandledErr;
	return status;
	}   // Handle_ControlValueFieldOrHiliteChanged
