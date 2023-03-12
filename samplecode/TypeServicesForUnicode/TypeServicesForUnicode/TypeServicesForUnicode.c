/*
*	File:		TypeServicesForUnicode.c
* 
*	Contains:	This is a sample program that demonstrates how to use
*					the Apple Text Services For Unicode Imaging (ATSUI) technology
*					in a Carbon Events based HIView architectured application.
*
*					The TypeForServices code handles:
*						- showing how to set the text
*						- showing how to set the text and styles
*						- showing how to rotate
*						- showing how to handle font variants
*						- showing how to handle font selection through the standard Font menu
*						- showing how to handle font selection through the standard Font panel
*						- showing how to instantiate the HIATSUIView from a nib
*						- showing how to read a text file and set the HIATSUIView content
*						- showing how to handle the clipboard
*						- showing how to handle background pictures
*
*  Note:		The project is set up so that the DEBUG macro is set to one when the "Development"
*				build style is chosen and not at all when the "Deployment" build style is chosen.
*				Thus, all the require asserts "fire" only in "Development".
*	
*	Version:	2.0
* 
*	Created:	11/4/04
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

#include <Carbon/Carbon.h>
#include "HIATSUIView.h"
#include "MoreATSUnicode.h"
#include "TypeServicesForUnicode.h"

//****************************************************
#pragma mark -
#pragma mark * typedef's, struct's, enums, defines, etc. *

const HIViewID kAtsuiID = { 'HAtV', 100 };

//****************************************************
#pragma mark -
#pragma mark * local (static) function prototypes *

static OSStatus Create_WindowWithTitleAndCustomView(HISize theSize, CFStringRef theTitle, HIViewRef theHIView, WindowRef * outWindow);
static pascal OSStatus Handle_FontSelection(EventHandlerCallRef inHandlerCallRef, EventRef inEvent, void *inUserData);
static pascal OSStatus Handle_WindowSetFont(EventHandlerCallRef inHandlerCallRef, EventRef inEvent, void *inUserData);
static OSStatus ConvenienceSetUp_MultipleStyles(ItemCount * outNumberOfRuns, UniCharCount ** outRunLengths, ATSUStyle ** outStyles);
static pascal OSStatus Handle_RotateCommandProcess(EventHandlerCallRef inHandlerCallRef, EventRef inEvent, void *inUserData);
static pascal OSStatus Handle_SmallerBiggerCommandProcess(EventHandlerCallRef inHandlerCallRef, EventRef inEvent, void *inUserData);
static OSStatus AnotherConvenienceSetUp_MultipleStyles(ItemCount * outNumberOfRuns, UniCharCount ** outRunLengths, ATSUStyle ** outStyles, Boolean bigOrSmall, UniCharArrayOffset textLength);
static pascal OSStatus Handle_MoveHilightingCommandProcess(EventHandlerCallRef inHandlerCallRef, EventRef inEvent, void *inUserData);
static OSStatus Signal_ScrollViewContentChanged(HIViewRef theEmbeddedView);
static void MyScrollingTextTimeProc(EventLoopTimerRef inTimer, void *inUserData);
static OSStatus StartStopAutoScroll(HIViewRef theHIATSUIView, Boolean start);
static pascal OSStatus Handle_FrameWrapCommandProcess(EventHandlerCallRef inHandlerCallRef, EventRef inEvent, void *inUserData);
static pascal void MyEmptyActionProc(ControlRef theControl, ControlPartCode partCode);
static pascal OSStatus Handle_XLocationChanged(EventHandlerCallRef inHandlerCallRef, EventRef inEvent, void *inUserData);
static pascal OSStatus Handle_JustificationChanged(EventHandlerCallRef inHandlerCallRef, EventRef inEvent, void *inUserData);
static OSStatus Create_CGImageFromResource(CFStringRef name, CGImageRef * outCGImageRef);
static OSStatus Install_ImageInBackground(WindowRef theWindow, CFStringRef name, HIViewRef * imageView);
static OSStatus Set_FontAndSizeToFirstStyle(HIViewRef atsuiView, ATSUFontID atsuFontID, Fixed atsuSize, ATSUStyle * atsuStyle);
static OSStatus Populate_FeatureWindow(WindowRef featureWindow, ATSUFontID atsuFontID, ATSUStyle atsuStyle);
static OSStatus Beautify_FeatureWindow(WindowRef featureWindow, SInt16 * maxWidth, SInt16 * maxHeight);
static CFStringRef atsuGetFontNameCodeAsCFString(ATSUFontID inFontID);
static CFStringRef atsuGetFontFeatureNameCodeAsCFString(ATSUFontID inFontID, ATSUFontFeatureType inType, ATSUFontFeatureSelector inSelector);
static CFStringRef atsuGetFontVariationNameCodeAsCFString(ATSUFontID inFontID, ATSUFontVariationAxis inAxis);
static pascal OSStatus Handle_ScrollingWindowIsClosing(EventHandlerCallRef inHandlerCallRef, EventRef inEvent, void *inUserData);
static pascal OSStatus Handle_FeatureCommandProcess(EventHandlerCallRef inHandlerCallRef, EventRef inEvent, void *inUserData);
static pascal OSStatus Handle_VariationValueChanged(EventHandlerCallRef inHandlerCallRef, EventRef inEvent, void *inUserData);
static pascal void MyNavEventProc(NavEventCallbackMessage callBackSelector, NavCBRecPtr callBackParms, void *callBackUD);
static OSStatus ChooseOneFile(FSRef * theFSRef);
static CFStringRef GetTextFromFile(FSRef * theFSRef);
static pascal OSStatus Handle_UserPaneDraw(EventHandlerCallRef inHandlerCallRef, EventRef inEvent, void *inUserData);
static pascal OSStatus Handle_TextBoxParameterChanged(EventHandlerCallRef inHandlerCallRef, EventRef inEvent, void *inUserData);
static pascal OSStatus Handle_ClipboardWindowIsClosing(EventHandlerCallRef inHandlerCallRef, EventRef inEvent, void *inUserData);
static OSStatus Update_ClipboardWindow(void);
static OSStatus Extract_TextAndStyleFromPasteboard(CFStringRef * oTextString, void ** oStyleBuffer, ByteCount * oStyleBufferSize);
static OSStatus Extract_RunLengthsAndStylesFromStyleBuffer(void * styleBuffer, ByteCount styleBufferSize, ItemCount * oNumberOfRuns, UniCharCount ** oRunLengths, ATSUStyle ** oStyles);
static Boolean UTTypeConformsToTag(CFStringRef inTypeIdentifier, OSType tag);

//****************************************************
#pragma mark -
#pragma mark * exported globals *

//****************************************************
#pragma mark -
#pragma mark * local (static) globals *

// window count for the window cascade (see Create_WindowWithTitleAndCustomView)
static int gWindowCount = 0;

// keeping our location around for the Navigation Services dialogs
static FSRef gApplicationBundleFSRef;

// there is only 1 clipboard so we need a few globals to deal with this unicity
static PasteboardRef gClipboard = NULL;
static WindowRef gClipboardWindow = NULL;
static Boolean gClipboardChanged;

//****************************************************
#pragma mark -
#pragma mark * exported function implementations *

/*****************************************************
*
* Do_NewSingleLineSingleStyleWindow(outWindow) 
*
* Purpose:  called to create a new window which shows a single ATSUI line in a single style
*
* Notes:    called by Handle_CommandProcess(), Handle_OpenApplication(). Handle_ReopenApplication()
*
* Inputs:   none
*
* Returns:  OSStatus			- error code (0 == no error) 
*/
OSStatus Do_NewSingleLineSingleStyleWindow(void)
	{
	WindowRef aWindowRef = NULL;
	CFStringRef theTitle = NULL;
	HIViewRef atsuiView = NULL;
	CFStringRef hello = NULL;
	OSStatus status;
	
	// creating the window title
	theTitle = CFStringCreateWithFormat(NULL, NULL, CFSTR("Single Line Single Style #%ld"), ++gWindowCount);
	require(theTitle != NULL, CantSetTitle);
	
	// creating the HIATSUIView
	HIRect hiRect = { { 20, 20 }, { 300, 200 } };
	status = HICreateATSUIView(&hiRect, &atsuiView);
	require_noerr(status, CantCreateATSUIView);
	require(atsuiView != NULL, CantCreateATSUIView);
	SetControlID(atsuiView, &kAtsuiID);
	
	// creating the window with that title and HIATSUIView
	HISize theSize = { hiRect.size.width + 2 * hiRect.origin.x, hiRect.size.height + 2 * hiRect.origin.y };
	status = Create_WindowWithTitleAndCustomView(theSize, theTitle, atsuiView, &aWindowRef);
	require_noerr(status, CantCreateWindow);
	
	// grabbing the text from our resources and setting it in the HIATSUIView
	hello = CFCopyLocalizedString( CFSTR("Hello World!"), CFSTR("") );
	require(hello != NULL, CantSetATSUIView);

	status = HIATSUIViewSetText(atsuiView, hello, 0, NULL, NULL);
	require_noerr(status, CantSetATSUIView);
	
	// moving the text within the HIATSUIView
	status = HIATSUIViewSetPosition(atsuiView, IntToFixed(10), IntToFixed(50));
	require_noerr(status, CantSetATSUIView);
	
	ShowWindow(aWindowRef);
	
CantSetATSUIView:
CantCreateWindow:
CantCreateATSUIView:
CantSetTitle:

	// let's cleanup
	if (hello != NULL) CFRelease(hello);
	if (theTitle != NULL) CFRelease(theTitle);
	
	return status;
	}   // Do_NewSingleLineSingleStyleWindow

/*****************************************************
*
* Do_NewSingleLineMultipleStyleWindow(outWindow) 
*
* Purpose:  called to create a new window which shows a single ATSUI line with multiple styles
*
* Notes:    called by Handle_CommandProcess()
*
* Inputs:   none
*
* Returns:  OSStatus			- error code (0 == no error) 
*/
OSStatus Do_NewSingleLineMultipleStyleWindow(void)
	{
	WindowRef aWindowRef = NULL;
	CFStringRef theTitle = NULL;
	HIViewRef atsuiView = NULL;
	CFStringRef demoText2 = NULL;
	UniCharCount * runLengths = NULL;
	ATSUStyle * styles = NULL;
	ItemCount numberOfRuns;
	OSStatus status;
	
	// creating the window title
	theTitle = CFStringCreateWithFormat(NULL, NULL, CFSTR("Single Line Multiple Styles #%ld"), ++gWindowCount);
	require(theTitle != NULL, CantSetTitle);
	
	// creating the HIATSUIView
	HIRect hiRect = { { 20, 20 }, { 800, 200 } };
	status = HICreateATSUIView(&hiRect, &atsuiView);
	require_noerr(status, CantCreateATSUIView);
	require(atsuiView != NULL, CantCreateATSUIView);
	SetControlID(atsuiView, &kAtsuiID);
	
	// creating the window with that title and HIATSUIView
	HISize theSize = { hiRect.size.width + 2 * hiRect.origin.x, hiRect.size.height + 2 * hiRect.origin.y };
	status = Create_WindowWithTitleAndCustomView(theSize, theTitle, atsuiView, &aWindowRef);
	require_noerr(status, CantCreateWindow);
	
	// using a convenience function to set up our styles
	status = ConvenienceSetUp_MultipleStyles(&numberOfRuns, &runLengths, &styles);
	require_noerr(status, CantSetStyles);

	// grabbing the text from our resources and setting it in the HIATSUIView
	demoText2 = CFCopyLocalizedString( CFSTR("demoText2"), CFSTR("") );
	require(demoText2 != NULL, CantSetLayout);

	status = HIATSUIViewSetText(atsuiView, demoText2, numberOfRuns, runLengths, styles);
	require_noerr(status, CantSetLayout);
	
	ShowWindow(aWindowRef);
	
CantSetLayout:
CantSetStyles:
CantCreateWindow:
CantCreateATSUIView:
CantSetTitle:

	// let's cleanup
	if (demoText2 != NULL) CFRelease(demoText2);
	if (styles != NULL) free (styles);
	if (runLengths != NULL) free(runLengths);
	if (theTitle != NULL) CFRelease(theTitle);

	return status;
	}   // Do_NewSingleLineMultipleStyleWindow

/*****************************************************
*
* Do_NewSingleVerticalLineMultipleStyleWindow(outWindow) 
*
* Purpose:  called to create a new window which shows a single vertical ATSUI line with multiple styles
*
* Notes:    called by Handle_CommandProcess()
*
* Inputs:   none
*
* Returns:  OSStatus			- error code (0 == no error) 
*/
OSStatus Do_NewSingleVerticalLineMultipleStyleWindow(void)
	{
	WindowRef aWindowRef = NULL;
	CFStringRef theTitle = NULL;
	HIViewRef atsuiView = NULL;
	CFStringRef demoText2 = NULL;
	UniCharCount * runLengths = NULL;
	ATSUStyle * styles = NULL;
	ItemCount numberOfRuns;
	OSStatus status;
	
	// creating the window title
	theTitle = CFStringCreateWithFormat(NULL, NULL, CFSTR("Single Vertical Line Multiple Styles #%ld"), ++gWindowCount);
	require(theTitle != NULL, CantSetTitle);
	
	// creating the HIATSUIView
	HIRect hiRect = { { 20, 60 }, { 800, 800 } };
	status = HICreateATSUIView(&hiRect, &atsuiView);
	require_noerr(status, CantCreateATSUIView);
	require(atsuiView != NULL, CantCreateATSUIView);
	SetControlID(atsuiView, &kAtsuiID);
	
	// creating the window with that title and HIATSUIView
	HISize theSize = { hiRect.size.width + 2 * hiRect.origin.x, hiRect.size.height + 2 * hiRect.origin.y };
	status = Create_WindowWithTitleAndCustomView(theSize, theTitle, atsuiView, &aWindowRef);
	require_noerr(status, CantCreateWindow);

	// using a convenience function to set up our styles
	status = ConvenienceSetUp_MultipleStyles(&numberOfRuns, &runLengths, &styles);
	require_noerr(status, CantSetStyles);

	// grabbing the text from our resources and setting it in the HIATSUIView
	demoText2 = CFCopyLocalizedString( CFSTR("demoText2"), CFSTR("") );
	require(demoText2 != NULL, CantSetLayout);

	status = HIATSUIViewSetText(atsuiView, demoText2, numberOfRuns, runLengths, styles);
	require_noerr(status, CantSetLayout);
	HIATSUIViewSetPosition(atsuiView, IntToFixed(30), IntToFixed(30));
	
	// let's rotate the line
	status = atsuSetLineRotation(HIATSUIViewGetTextLayout(atsuiView), IntToFixed(0), IntToFixed(-90));
	require_noerr(status, CantRotate);
	
	// let's add a button to let the user ask for the animated rotation again
	HIViewRef button;
	Rect bounds = { 20, 20, 40, 100 };
	status = CreatePushButtonControl(aWindowRef, &bounds, CFSTR("Rotate"), &button);
	require_noerr(status, CantCreateButton);
	SetControlCommandID(button, 'Rott');
	
	// let's add a handler for when that button is clicked
	EventTypeSpec eventTypeCP = {kEventClassCommand, kEventCommandProcess};
	status = InstallWindowEventHandler(aWindowRef, Handle_RotateCommandProcess, 1, &eventTypeCP, (void *)atsuiView, NULL);
	require_noerr(status, CantInstallEventHandler);
	
	ShowWindow(aWindowRef);
	
CantInstallEventHandler:
CantCreateButton:
CantRotate:
CantSetLayout:
CantSetStyles:
CantCreateWindow:
CantCreateATSUIView:
CantSetTitle:

	// let's cleanup
	if (demoText2 != NULL) CFRelease(demoText2);
	if (styles != NULL) free (styles);
	if (runLengths != NULL) free(runLengths);
	if (theTitle != NULL) CFRelease(theTitle);
	
	return status;
	}   // Do_NewSingleVerticalLineMultipleStyleWindow

/*****************************************************
*
* Do_NewParagraphsWindow(outWindow) 
*
* Purpose:  called to create a new window which shows how to break text into paragraphs
*
* Notes:    called by Handle_CommandProcess()
*
* Inputs:   none
*
* Returns:  OSStatus			- error code (0 == no error) 
*/
OSStatus Do_NewParagraphsWindow(void)
	{
	WindowRef aWindowRef = NULL;
	CFStringRef theTitle = NULL;
	HIViewRef atsuiView = NULL;
	CFStringRef demoText4 = NULL;
	OSStatus status;
	
	// creating the window title
	theTitle = CFStringCreateWithFormat(NULL, NULL, CFSTR("Paragraphs #%ld"), ++gWindowCount);
	require(theTitle != NULL, CantSetTitle);
	
	// creating the HIATSUIView
	HIRect hiRect = { { 20, 60 }, { 500, 400 } };
	status = HICreateATSUIView(&hiRect, &atsuiView);
	require_noerr(status, CantCreateATSUIView);
	require(atsuiView != NULL, CantCreateATSUIView);
	SetControlID(atsuiView, &kAtsuiID);
	
	// creating the window with that title and HIATSUIView
	HISize theSize = { hiRect.size.width + 2 * hiRect.origin.x, hiRect.size.height + hiRect.origin.x + hiRect.origin.y };
	status = Create_WindowWithTitleAndCustomView(theSize, theTitle, atsuiView, &aWindowRef);
	require_noerr(status, CantCreateWindow);

	// grabbing the text from our resources and setting it in the HIATSUIView
	demoText4 = CFCopyLocalizedString( CFSTR("demoText4"), CFSTR("") );
	require(demoText4 != NULL, CantSetLayout);

	status = HIATSUIViewSetText(atsuiView, demoText4, 0, NULL, NULL);
	require_noerr(status, CantSetLayout);

	// let's add 2 buttons to let the user change the size of the text so that the text is properly broken in lines
	HIViewRef button;
	Rect bounds = { 20, 20, 40, 100 };
	status = CreatePushButtonControl(aWindowRef, &bounds, CFSTR("Smaller"), &button);
	require_noerr(status, CantCreateButton);
	SetControlCommandID(button, 'Smlr');
	OffsetRect(&bounds, 100, 0);
	status = CreatePushButtonControl(aWindowRef, &bounds, CFSTR("Bigger"), &button);
	require_noerr(status, CantCreateButton);
	SetControlCommandID(button, 'Bggr');
	
	// let's add a handler for when those buttons are clicked
	EventTypeSpec eventTypeCP = {kEventClassCommand, kEventCommandProcess};
	status = InstallWindowEventHandler(aWindowRef, Handle_SmallerBiggerCommandProcess, 1, &eventTypeCP, (void *)atsuiView, NULL);
	require_noerr(status, CantInstallEventHandler);
	
	ShowWindow(aWindowRef);
	
CantInstallEventHandler:
CantCreateButton:
CantSetLayout:
CantCreateWindow:
CantCreateATSUIView:

	// let's cleanup
	if (demoText4 != NULL) CFRelease(demoText4);
	if (theTitle != NULL) CFRelease(theTitle);

CantSetTitle:
	
	return status;
	}   // Do_NewParagraphsWindow

/*****************************************************
*
* Do_NewHighlightingWindow(outWindow) 
*
* Purpose:  called to create a new window which shows how to highlight text and move the caret
*
* Notes:    called by Handle_CommandProcess()
*
* Inputs:   none
*
* Returns:  OSStatus			- error code (0 == no error) 
*/
OSStatus Do_NewHighlightingWindow(void)
	{
	WindowRef aWindowRef = NULL;
	CFStringRef theTitle = NULL;
	HIViewRef atsuiView = NULL;
	CFStringRef demoText5 = NULL;
	UniCharCount * runLengths = NULL;
	ATSUStyle * styles = NULL;
	ItemCount numberOfRuns;
	OSStatus status;
	
	// creating the window title
	theTitle = CFStringCreateWithFormat(NULL, NULL, CFSTR("Highlighting, Carets & Cursor Movements #%ld"), ++gWindowCount);
	require(theTitle != NULL, CantSetTitle);
	
	// creating the HIATSUIView
	HIRect hiRect = { { 20, 60 }, { 540, 700 } };
	status = HICreateATSUIView(&hiRect, &atsuiView);
	require_noerr(status, CantCreateATSUIView);
	require(atsuiView != NULL, CantCreateATSUIView);
	SetControlID(atsuiView, &kAtsuiID);
	
	// creating the window with that title and HIATSUIView
	HISize theSize = { hiRect.size.width + 2 * hiRect.origin.x, hiRect.size.height + hiRect.origin.x + hiRect.origin.y };
	status = Create_WindowWithTitleAndCustomView(theSize, theTitle, atsuiView, &aWindowRef);
	require_noerr(status, CantCreateWindow);

	// grabbing the text from our resources
	demoText5 = CFCopyLocalizedString( CFSTR("demoText5"), CFSTR("") );
	require(demoText5 != NULL, CantGetText);

	// using a convenience function to set up our styles
	status = AnotherConvenienceSetUp_MultipleStyles(&numberOfRuns, &runLengths, &styles, false, CFStringGetLength(demoText5));
	require_noerr(status, CantSetStyles);

	// setting our text and styles
	status = HIATSUIViewSetText(atsuiView, demoText5, numberOfRuns, runLengths, styles);
	require_noerr(status, CantSetLayout);

	// let's add 4 buttons to let the user change the location and size of the selection
	HIViewRef button;
	Rect bounds = { 20, 20, 40, 80 };
	status = CreatePushButtonControl(aWindowRef, &bounds, CFSTR("Home"), &button);
	require_noerr(status, CantCreateButton);
	SetControlCommandID(button, 'Home');
	OffsetRect(&bounds, 70, 0); bounds.right += 50;
	status = CreatePushButtonControl(aWindowRef, &bounds, CFSTR("Previous Word"), &button);
	require_noerr(status, CantCreateButton);
	SetControlCommandID(button, 'Prev');
	OffsetRect(&bounds, 120, 0); bounds.right -= 20;
	status = CreatePushButtonControl(aWindowRef, &bounds, CFSTR("Next Word"), &button);
	require_noerr(status, CantCreateButton);
	SetControlCommandID(button, 'Next');
	OffsetRect(&bounds, 100, 0); bounds.right -= 10;
	status = CreatePushButtonControl(aWindowRef, &bounds, CFSTR("Next 30"), &button);
	require_noerr(status, CantCreateButton);
	SetControlCommandID(button, 'Nx30');
	
	// let's add a handler for when those buttons are clicked
	EventTypeSpec eventTypeCP = {kEventClassCommand, kEventCommandProcess};
	status = InstallWindowEventHandler(aWindowRef, Handle_MoveHilightingCommandProcess, 1, &eventTypeCP, (void *)atsuiView, NULL);
	require_noerr(status, CantInstallEventHandler);
	
	ShowWindow(aWindowRef);
	
CantInstallEventHandler:
CantCreateButton:
CantSetLayout:
CantSetStyles:
CantGetText:
CantCreateWindow:
CantCreateATSUIView:
CantSetTitle:

	// let's cleanup
	if (demoText5 != NULL) CFRelease(demoText5);
	if (styles != NULL) free (styles);
	if (runLengths != NULL) free(runLengths);
	if (theTitle != NULL) CFRelease(theTitle);
	
	return status;
	}   // Do_NewHighlightingWindow

/*****************************************************
*
* Do_NewHitTestingWindow(outWindow) 
*
* Purpose:  called to create a new window which shows how to handle hit testing
*
* Notes:    called by Handle_CommandProcess()
*
* Inputs:   none
*
* Returns:  OSStatus			- error code (0 == no error) 
*/
OSStatus Do_NewHitTestingWindow(void)
	{
	WindowRef aWindowRef = NULL;
	CFStringRef theTitle = NULL;
	HIViewRef atsuiView = NULL;
	CFStringRef demoText6 = NULL;
	UniCharCount * runLengths = NULL;
	ATSUStyle * styles = NULL;
	ItemCount numberOfRuns;
	OSStatus status;
	
	// creating the window title
	theTitle = CFStringCreateWithFormat(NULL, NULL, CFSTR("Hit Testing #%ld"), ++gWindowCount);
	require(theTitle != NULL, CantSetTitle);
	
	// creating the HIATSUIView
	HIRect hiRect = { { 20, 20 }, { 540, 700 } };
	status = HICreateATSUIView(&hiRect, &atsuiView);
	require_noerr(status, CantCreateATSUIView);
	require(atsuiView != NULL, CantCreateATSUIView);
	SetControlID(atsuiView, &kAtsuiID);
	
	// creating the window with that title and HIATSUIView
	HISize theSize = { hiRect.size.width + 2 * hiRect.origin.x, hiRect.size.height + hiRect.origin.x + hiRect.origin.y };
	status = Create_WindowWithTitleAndCustomView(theSize, theTitle, atsuiView, &aWindowRef);
	require_noerr(status, CantCreateWindow);

	// grabbing the text from our resources
	demoText6 = CFCopyLocalizedString( CFSTR("demoText6"), CFSTR("") );
	require(atsuiView != NULL, CantGetText);

	// using a convenience function to set up our styles
	status = AnotherConvenienceSetUp_MultipleStyles(&numberOfRuns, &runLengths, &styles, false, CFStringGetLength(demoText6));
	require_noerr(status, CantSetStyles);

	// setting our text and styles
	status = HIATSUIViewSetText(atsuiView, demoText6, numberOfRuns, runLengths, styles);
	require_noerr(status, CantSetLayout);
	
	ShowWindow(aWindowRef);
	
CantInstallEventHandler:
CantCreateButton:
CantSetLayout:
CantSetStyles:
CantGetText:
CantCreateWindow:
CantCreateATSUIView:
CantSetTitle:

	// let's cleanup
	if (demoText6 != NULL) CFRelease(demoText6);
	if (styles != NULL) free (styles);
	if (runLengths != NULL) free(runLengths);
	if (theTitle != NULL) CFRelease(theTitle);
	
	return status;
	}   // Do_NewHitTestingWindow

/*****************************************************
*
* Handle_AppActivated(inHandlerCallRef, inEvent, inUserData) 
*
* Purpose:  called when the application becomes frontmost
*
* Inputs:   inHandlerCallRef    - reference to the current handler call chain
*				inEvent             - the event
*           inUserData          - app-specified data you passed in the call to InstallEventHandler
*
* Returns:  OSStatus            - noErr indicates the event was handled
*                                 eventNotHandledErr indicates the event was not handled and the Toolbox should take over
*/
pascal OSStatus Handle_AppActivated(EventHandlerCallRef inHandlerCallRef, EventRef inEvent, void *inUserData)
	{
	OSStatus status = eventNotHandledErr;
	
	// if that's the first time we get in here, then the application has just been launched and
	// we need to create and keep a reference to our clipboard
	if (gClipboard == NULL)
		{
		status = PasteboardCreate(kPasteboardClipboard, &gClipboard);
		require_noerr(status, ExitActivated);
		gClipboardChanged = true;
		}

	// checking if the Pasteboard has been modified
	// if it has and the "Show Clipboard" window is visible then we update its contents right away
	// else we just memorize the fact that the clipboard has changed and we'll update the contents
	// of the "Show Clipboard" window when it gets visible (this is to improve performance by not
	// doing needless work).
	if (PasteboardSynchronize(gClipboard) & kPasteboardModified)
		{
		gClipboardChanged = true;
		if (gClipboardWindow != NULL)
			if (IsWindowVisible(gClipboardWindow))
				{
				status = Update_ClipboardWindow();
				require_noerr(status, ExitActivated);
				}
		}

ExitActivated:

	return status;
	}   // Handle_AppActivated

/*****************************************************
*
* Do_ShowClipboardWindow(outWindow) 
*
* Purpose:  called to create a new window which shows how to use the clipboard
*
* Notes:    called by Handle_CommandProcess()
*
* Inputs:   none
*
* Returns:  OSStatus			- error code (0 == no error) 
*/
OSStatus Do_ShowClipboardWindow(void)
	{
	HIViewRef atsuiView = NULL;
	OSStatus status;

	// if that's the first time we get in here, then the application has just been launched and
	// we need to create and keep a reference to our "Show Clipboard" window
	if (gClipboardWindow == NULL)
		{
		// creating the HIATSUIView
		HIRect hiRect = { { 20, 20 }, { 300, 200 } };
		status = HICreateATSUIView(&hiRect, &atsuiView);
		require_noerr(status, CantCreateATSUIView);
		require(atsuiView != NULL, CantCreateATSUIView);
		SetControlID(atsuiView, &kAtsuiID);

		// creating a scroll view
		HIViewRef scrollView;
		status = HIScrollViewCreate(kHIScrollViewOptionsVertScroll, &scrollView);
		require_noerr(status, CantCreateScrollView);
		
		// embedding our HIATSUIView in the scroll view
		status = HIViewAddSubview(scrollView, atsuiView);
		require_noerr(status, CantCreateScrollView);
		
		status = HIViewSetFrame(scrollView, &hiRect);
		require_noerr(status, CantCreateScrollView);

		HIViewSetVisible(atsuiView, true);
		require_noerr(status, CantCreateScrollView);
		
		// creating the window with that scrollview
		HISize theSize = { hiRect.size.width + 2 * hiRect.origin.x, hiRect.size.height + 2 * hiRect.origin.y };
		status = Create_WindowWithTitleAndCustomView(theSize, CFSTR("Show Clipboard"), scrollView, &gClipboardWindow);
		require_noerr(status, CantCreateWindow);

		// changing the window class so it doesn't interfere with the other ones
		status = HIWindowChangeClass(gClipboardWindow, kFloatingWindowClass);
		require_noerr(status, CantSetClass);
		
		// when the user closes the "Show Clipboard" window, we actually want only to hide it so we need to handle that event
		EventTypeSpec eventTypeWC = {kEventClassWindow, kEventWindowClose};
		status = InstallWindowEventHandler(gClipboardWindow, Handle_ClipboardWindowIsClosing, 1, &eventTypeWC, (void *)gClipboardWindow, NULL);
		require_noerr(status, CantInstallEventHandler);
		}

	// if there has been a previous clipboard content change then we update the contents
	// of the "Show Clipboard" window before making it visible or front
	if (gClipboardChanged)
		{
		status = Update_ClipboardWindow();
		require_noerr(status, CantUpdateClipboard);
		}

	ShowWindow(gClipboardWindow);
	SelectWindow(gClipboardWindow);

CantUpdateClipboard:
CantInstallEventHandler:
CantSetClass:
CantCreateWindow:
CantCreateScrollView:
CantCreateATSUIView:
	
	return status;
	}   // Do_ShowClipboardWindow

/*****************************************************
*
* Do_NewTextBoxWindow(outWindow) 
*
* Purpose:  called to create a new window which shows how to use the atsuTextBox() API
*
* Notes:    called by Handle_CommandProcess()
*
* Inputs:   none
*
* Returns:  OSStatus			- error code (0 == no error) 
*/
OSStatus Do_NewTextBoxWindow(IBNibRef nibRef)
	{
	WindowRef aWindowRef = NULL;
	CFStringRef theTitle = NULL;
	OSStatus status;
	
	// creating that window from our nib
	status = CreateWindowFromNib(nibRef, CFSTR("TextBoxWindow"), &aWindowRef);
	require_noerr(status, CantCreateWindow);
	require(NULL != aWindowRef, CantCreateWindow);
	
	// finding our user pane where the text will be drawn
	HIViewRef windRoot = HIViewGetRoot(aWindowRef);
	HIViewID hiViewID = { 'Usrp', 100 };
	HIViewRef userPane;
	status = HIViewFindByID(windRoot, hiViewID, &userPane);
	require_noerr(status, CantSetUpUserPane);

	EventTypeSpec eventTypeCD = {kEventClassControl, kEventControlDraw};
	InstallControlEventHandler(userPane, Handle_UserPaneDraw, 1, &eventTypeCD, (void *)userPane, NULL);

	// we set up all our controls so that any change will trigger a call to Handle_TextBoxParameterChanged
	// which, in turn, invalidates our user pane for later redraw
	EventTypeSpec eventTypeCVFC = {kEventClassControl, kEventControlValueFieldChanged};
	OSType controlSigs[] = { 'Alig', 'Ornt', 'Just' };
	HIViewRef theView;
	int i;
	for (i=0; i<3; i++)
		{
		HIViewID hiViewID = { controlSigs[i], 100 };
		status = HIViewFindByID(windRoot, hiViewID, &theView);
		require_noerr(status, CantSetUpControls);
		status = InstallControlEventHandler(theView, Handle_TextBoxParameterChanged, 1, &eventTypeCVFC, (void *)userPane, NULL);
		require_noerr(status, CantSetUpControls);
		}
	// in addition, the last view is the slider which needs the following action proc in order to be live
	SetControlAction(theView, MyEmptyActionProc);

	// changing the title
	theTitle = CFStringCreateWithFormat(NULL, NULL, CFSTR("ATSU Text Box #%ld"), ++gWindowCount);
	require(theTitle != NULL, CantSetTitle);
	status = SetWindowTitleWithCFString(aWindowRef, theTitle);
	require_noerr(status, CantSetTitle);
	
	ShowWindow(aWindowRef);
	
CantSetTitle:
CantSetUpControls:
CantSetUpUserPane:
CantCreateWindow:

	// let's cleanup
	if (theTitle != NULL) CFRelease(theTitle);
	
	return status;
	}   // Do_NewTextBoxWindow

/*****************************************************
*
* Do_NewUsingFontVariantsWindow(outWindow) 
*
* Purpose:  called to create a new window which shows how to handle Font variants when present
*
* Notes:    called by Handle_CommandProcess()
*
* Inputs:   none
*
* Returns:  OSStatus			- error code (0 == no error) 
*/
OSStatus Do_NewUsingFontVariantsWindow(void)
	{
	WindowRef aWindowRef = NULL;
	CFStringRef theTitle = NULL;
	HIViewRef atsuiView = NULL;
	CFStringRef demoText4 = NULL;
	OSStatus status;
	
	// creating the window title
	theTitle = CFStringCreateWithFormat(NULL, NULL, CFSTR("Using Font Variants #%ld"), ++gWindowCount);
	require(theTitle != NULL, CantSetTitle);
	
	// creating the HIATSUIView
	HIRect hiRect = { { 20, 20 }, { 500, 400 } };
	status = HICreateATSUIView(&hiRect, &atsuiView);
	require_noerr(status, CantCreateATSUIView);
	require(atsuiView != NULL, CantCreateATSUIView);
	SetControlID(atsuiView, &kAtsuiID);
	
	// creating the window with that title and HIATSUIView
	HISize theSize = { hiRect.size.width + 2 * hiRect.origin.x, hiRect.size.height + hiRect.origin.x + hiRect.origin.y };
	status = Create_WindowWithTitleAndCustomView(theSize, theTitle, atsuiView, &aWindowRef);
	require_noerr(status, CantCreateWindow);

	// grabbing the text from our resources and setting it in the HIATSUIView
	demoText4 = CFCopyLocalizedString( CFSTR("demoText4"), CFSTR("") );
	require(demoText4 != NULL, CantSetLayout);

	status = HIATSUIViewSetText(atsuiView, demoText4, 0, NULL, NULL);
	require_noerr(status, CantSetLayout);

	// let's have our window reacting to changes in the Standard Font panel
	EventTypeSpec eventTypeFS = {kEventClassFont, kEventFontSelection};
	status = InstallEventHandler(GetWindowEventTarget(aWindowRef), Handle_FontSelection, 1, &eventTypeFS, (void *)aWindowRef, NULL);
	require_noerr(status, CantInstallEventHandler);

	// setting the main (first) style of our text to Geneva 36
	ATSUFontID atsuFontID;
	char fontName[] = "Geneva";
	status = ATSUFindFontFromName(fontName, strlen(fontName), kFontFamilyName, kFontMacintoshPlatform, kFontRomanScript, kFontNoLanguageCode, &atsuFontID);
	verify_noerr_string(status, "ATSUFindFontFromName failed.");

	ATSUStyle style;
	status = Set_FontAndSizeToFirstStyle(atsuiView, atsuFontID, IntToFixed(36), &style);
	require_noerr(status, CantSetStyle);

	// and let's use that style to preset the Standard Font panel
#ifdef MAC_OS_X_VERSION_10_4
	status = SetFontInfoForSelection(kFontSelectionATSUIType, 1, &style, GetWindowEventTarget(aWindowRef));
#else
	// there was a small typecast bug in the pre-10.4 headers
	status = SetFontInfoForSelection(kFontSelectionATSUIType, 1, &style, (HIObjectRef)GetWindowEventTarget(aWindowRef));
#endif
	verify_noerr_string(status, "SetFontInfoForSelection failed.");

	// adding a drawer where we will show the font variants if present
	Rect globalBounds = {0, 0, 0, 0};
	WindowRef featureWindow = NULL;
	status = CreateNewWindow(kDrawerWindowClass, kWindowResizableAttribute | kWindowStandardHandlerAttribute | kWindowMetalAttribute | kWindowCompositingAttribute, &globalBounds, &featureWindow);
	require_noerr(status, CantSetFeatureWindow);
	
	// linking our window and its drawer together for convenience
	SetWRefCon(aWindowRef, (long)featureWindow);
	SetWRefCon(featureWindow, (long)aWindowRef);

	// populating our drawer with the font variants of our main style
	status = Populate_FeatureWindow(featureWindow, atsuFontID, style);
	require_noerr(status, CantSetFeatureWindow);

	// handling the user's choices in the drawer
	EventTypeSpec eventTypeCP = {kEventClassCommand, kEventCommandProcess};
	InstallEventHandler(GetWindowEventTarget(featureWindow), Handle_FeatureCommandProcess, 1, &eventTypeCP, NULL, NULL);

	status = SetDrawerParent(featureWindow, aWindowRef);
	require_noerr(status, CantSetFeatureWindow);
	
	ShowWindow(aWindowRef);
	status = ToggleDrawer(featureWindow);
	require_noerr(status, CantSetFeatureWindow);

CantSetFeatureWindow:
CantSetStyle:
CantInstallEventHandler:
CantSetLayout:
CantCreateWindow:
CantCreateATSUIView:

	// let's cleanup
	if (demoText4 != NULL) CFRelease(demoText4);
	if (theTitle != NULL) CFRelease(theTitle);

CantSetTitle:
	
	return status;
	}   // Do_NewUsingFontVariantsWindow

/*****************************************************
*
* Do_NewScrollingWindow(outWindow) 
*
* Purpose:  called to create a new window which shows how to handle scrolling withing a HIScrollView
*
* Notes:    called by Handle_CommandProcess()
*
* Inputs:   none
*
* Returns:  OSStatus			- error code (0 == no error) 
*/
OSStatus Do_NewScrollingWindow(void)
	{
	WindowRef aWindowRef = NULL;
	CFStringRef theTitle = NULL;
	HIViewRef atsuiView = NULL;
	CFStringRef demoText6 = NULL;
	UniCharCount * runLengths = NULL;
	ATSUStyle * styles = NULL;
	ItemCount numberOfRuns;
	OSStatus status;
	
	// creating the window title
	theTitle = CFStringCreateWithFormat(NULL, NULL, CFSTR("Scrolling #%ld"), ++gWindowCount);
	require(theTitle != NULL, CantSetTitle);
	
	// creating the HIATSUIView
	HIRect hiRect = { { 20, 60 }, { 690, 700 } };
	status = HICreateATSUIView(&hiRect, &atsuiView);
	require_noerr(status, CantCreateATSUIView);
	require(atsuiView != NULL, CantCreateATSUIView);
	SetControlID(atsuiView, &kAtsuiID);
	
	// creating a scroll view
	HIViewRef scrollView;
	status = HIScrollViewCreate(kHIScrollViewOptionsVertScroll, &scrollView);
	require_noerr(status, CantCreateScrollView);
	
	// embedding our HIATSUIView in the scroll view
	status = HIViewAddSubview(scrollView, atsuiView);
	require_noerr(status, CantCreateScrollView);
	
	status = HIViewSetFrame(scrollView, &hiRect);
	require_noerr(status, CantCreateScrollView);

	HIViewSetVisible(atsuiView, true);
	require_noerr(status, CantCreateScrollView);
	
	// creating the window with that title and scroll view
	HISize theSize = { hiRect.size.width + 2 * hiRect.origin.x, hiRect.size.height + hiRect.origin.x + hiRect.origin.y };
	status = Create_WindowWithTitleAndCustomView(theSize, theTitle, scrollView, &aWindowRef);
	require_noerr(status, CantCreateWindow);

	// grabbing the text from our resources
	demoText6 = CFCopyLocalizedString( CFSTR("demoText6"), CFSTR("") );
	require(demoText6 != NULL, CantGetText);

	// using a convenience function to set up our styles
	status = AnotherConvenienceSetUp_MultipleStyles(&numberOfRuns, &runLengths, &styles, true, CFStringGetLength(demoText6));
	require_noerr(status, CantSetStyles);

	// setting our text and styles
	status = HIATSUIViewSetText(atsuiView, demoText6, numberOfRuns, runLengths, styles);
	require_noerr(status, CantSetLayout);
	
	// adding a background image
	HIViewRef imageView;
	status = Install_ImageInBackground(aWindowRef, CFSTR("Pale Faux Fur.png"), &imageView);
	require_noerr(status, CantSetImageBackground);

	// adding controls to let the user choose the framing, wrapping, transparency,
	// background image, text position, justification, and auto-scrolling options
	// adding also the various event handlers needed to react to the user's choices
	HIViewRef aView, slider;
	Rect bounds = { 10, 20, 30, 110 };
	status = CreateCheckBoxControl(aWindowRef, &bounds, CFSTR("With Frame"), true, true, &aView);
	require_noerr(status, CantCreateControl);
	SetControlCommandID(aView, 'Frme');
	OffsetRect(&bounds, 0, 24);
	status = CreateCheckBoxControl(aWindowRef, &bounds, CFSTR("Wrap Text"), true, true, &aView);
	require_noerr(status, CantCreateControl);
	SetControlCommandID(aView, 'Wrap');
	OffsetRect(&bounds, 110, -24); bounds.right += 50;
	status = CreateCheckBoxControl(aWindowRef, &bounds, CFSTR("With Background"), true, true, &aView);
	require_noerr(status, CantCreateControl);
	SetControlCommandID(aView, 'Bckg');
	SetControlReference(aView, (SInt32)imageView);
	OffsetRect(&bounds, 0, 24);
	status = CreateCheckBoxControl(aWindowRef, &bounds, CFSTR("With Transparence"), false, true, &aView);
	require_noerr(status, CantCreateControl);
	SetControlCommandID(aView, 'Trns');
	OffsetRect(&bounds, 160, -24); bounds.right -= 40;
	status = CreateStaticTextControl(aWindowRef, &bounds, CFSTR("xLocation:"), NULL, &aView);
	OffsetRect(&bounds, 0, 18);
	status = CreateSliderControl(aWindowRef, &bounds, 3, 3, 70, kControlSliderDoesNotPoint, 0, true, MyEmptyActionProc, &slider);
	require_noerr(status, CantCreateControl);
	
	EventTypeSpec eventTypeCVFC = {kEventClassControl, kEventControlValueFieldChanged};
	status = InstallControlEventHandler(slider, Handle_XLocationChanged, 1, &eventTypeCVFC, (void *)atsuiView, NULL);
	require_noerr(status, CantInstallEventHandler);

	OffsetRect(&bounds, 120, -23); bounds.bottom = bounds.top + 60; bounds.right -= 40;
	HIViewRef radioGroup;
	status = CreateRadioGroupControl(aWindowRef, &bounds, &radioGroup);
	require_noerr(status, CantCreateControl);
	Rect rBounds = { 0, 0, 18, 100 };
	status = CreateRadioButtonControl(NULL, &rBounds, CFSTR("Left"), 0, true, &aView);
	require_noerr(status, CantCreateControl);
	SetControlCommandID(aView, 'Left');
	HIViewAddSubview(radioGroup, aView);
	OffsetRect(&rBounds, 0, 18);
	status = CreateRadioButtonControl(NULL, &rBounds, CFSTR("Center"), 0, true, &aView);
	require_noerr(status, CantCreateControl);
	SetControlCommandID(aView, 'Cntr');
	HIViewAddSubview(radioGroup, aView);
	OffsetRect(&rBounds, 0, 18);
	status = CreateRadioButtonControl(NULL, &rBounds, CFSTR("Right"), 0, true, &aView);
	require_noerr(status, CantCreateControl);
	SetControlCommandID(aView, 'Rght');
	HIViewAddSubview(radioGroup, aView);
	OffsetRect(&bounds, 80, 5); bounds.bottom = bounds.top + 20; bounds.right = bounds.left + 100;
	status = CreateStaticTextControl(aWindowRef, &bounds, CFSTR("Justification:"), NULL, &aView);
	OffsetRect(&bounds, 0, 18);
	status = CreateSliderControl(aWindowRef, &bounds, (UInt32)kATSUNoJustification, (UInt32)kATSUNoJustification, (UInt32)kATSUFullJustification, kControlSliderDoesNotPoint, 0, true, MyEmptyActionProc, &slider);
	require_noerr(status, CantCreateControl);
	OffsetRect(&bounds, 120, 0);
	status = CreateCheckBoxControl(aWindowRef, &bounds, CFSTR("Auto-Scroll"), false, true, &aView);
	require_noerr(status, CantCreateControl);
	SetControlCommandID(aView, 'Ascr');
	
	status = InstallControlEventHandler(slider, Handle_JustificationChanged, 1, &eventTypeCVFC, (void *)atsuiView, NULL);
	require_noerr(status, CantInstallEventHandler);
	
	EventTypeSpec eventTypeCP = {kEventClassCommand, kEventCommandProcess};
	status = InstallWindowEventHandler(aWindowRef, Handle_FrameWrapCommandProcess, 1, &eventTypeCP, (void *)atsuiView, NULL);
	require_noerr(status, CantInstallEventHandler);
	
	EventTypeSpec eventTypeWC = {kEventClassWindow, kEventWindowClosed};
	status = InstallWindowEventHandler(aWindowRef, Handle_ScrollingWindowIsClosing, 1, &eventTypeWC, (void *)aWindowRef, NULL);
	require_noerr(status, CantInstallEventHandler);

	// we keep the Carbon Event Timer reference for auto-scrolling in our Window Refcon
	// for convenience so let's initialize it with NULL for sure
	SetWRefCon(aWindowRef, 0);

	// dealing with the Standard Font panel
	EventTypeSpec eventTypeFS = {kEventClassFont, kEventFontSelection};
	status = InstallWindowEventHandler(aWindowRef, Handle_FontSelection, 1, &eventTypeFS, (void *)aWindowRef, NULL);
	require_noerr(status, CantInstallEventHandler);

	ATSUFontID atsuFontID;
	char fontName[] = "Geneva";
	status = ATSUFindFontFromName(fontName, strlen(fontName), kFontFamilyName, kFontMacintoshPlatform, kFontRomanScript, kFontNoLanguageCode, &atsuFontID);
	verify_noerr_string(status, "ATSUFindFontFromName failed.");

	ATSUStyle style;
	status = Set_FontAndSizeToFirstStyle(atsuiView, atsuFontID, IntToFixed(36), &style);
	require_noerr(status, CantSetStyle);

#ifdef MAC_OS_X_VERSION_10_4
	status = SetFontInfoForSelection(kFontSelectionATSUIType, 1, &style, GetWindowEventTarget(aWindowRef));
#else
	// there was a small typecast bug in the pre-10.4 headers
	status = SetFontInfoForSelection(kFontSelectionATSUIType, 1, &style, (HIObjectRef)GetWindowEventTarget(aWindowRef));
#endif
	verify_noerr_string(status, "SetFontInfoForSelection failed.");

	Signal_ScrollViewContentChanged(atsuiView);

	ShowWindow(aWindowRef);
	
CantSetStyle:
CantInstallEventHandler:
CantCreateControl:
CantSetImageBackground:
CantSetLayout:
CantSetStyles:
CantGetText:
CantCreateWindow:
CantCreateScrollView:
CantCreateATSUIView:
CantSetTitle:

	// let's cleanup
	if (demoText6 != NULL) CFRelease(demoText6);
	if (styles != NULL) free (styles);
	if (runLengths != NULL) free(runLengths);
	if (theTitle != NULL) CFRelease(theTitle);
	
	return status;
	}   // Do_NewScrollingWindow

/*****************************************************
*
* Handle_FontMenuSelection(hiCommand) 
*
* Purpose:  called to respond to a menu item selection in the Standard Font Menu
*
* Inputs:   hiCommand     - the command we received
*
* Returns:  OSStatus		  - error code (0 == no error, means we dealt with the command) 
*/
OSStatus Handle_FontMenuSelection(const HICommand * hiCommand)
	{
	OSStatus status = eventNotHandledErr;

	// do we really have a command coming from the Standard Font Menu?
	if ((hiCommand->commandID == 0) && (GetMenuID(hiCommand->menu.menuRef) == 3))
		{
		// looking for the frontmost window containing a HIATSUIView
		WindowRef frontWindow;
		HIViewRef atsuiView;
		Get_FrontWindowAndATSUIView(&frontWindow, &atsuiView, true);
		require(frontWindow != NULL, FontMenuSelectionExit);

		// getting font information from the Standard Font Menu
		FMFontFamily fmFamily;
		FMFontStyle fmStyle;
		status = GetFontFamilyFromMenuSelection(hiCommand->menu.menuRef, hiCommand->menu.menuItemIndex, &fmFamily, &fmStyle);
		require_noerr(status, FontMenuSelectionExit);

		// converting that font information into an ATSUFontID
		ATSUFontID atsuFont;
		status = FMGetFontFromFontFamilyInstance(fmFamily, fmStyle, &atsuFont, NULL);
		require_noerr(status, FontMenuSelectionExit);
		
		// creating and sending a custom event for our HIATSUIView so that it changes the font of the main (first) style
		EventRef theEvent;
		status = CreateEvent(NULL, kEventClassATSUI, kEventATSUISetFont, GetCurrentEventTime(), kEventAttributeUserEvent, &theEvent);
		require_noerr(status, FontMenuSelectionExit);

		status = SetEventParameter(theEvent, kEventParamATSUFontID, typeATSUFontID, sizeof(atsuFont), &atsuFont);
		require_noerr(status, FontMenuSelectionExit);

		SendEventToEventTarget(theEvent, GetWindowEventTarget(frontWindow));
		ReleaseEvent(theEvent);
		}

FontMenuSelectionExit:

	return status;
	}   // Handle_FontMenuSelection

/*****************************************************
*
* Handle_SelectAll() 
*
* Purpose:  called to respond to the Select All command
*
* Inputs:   none
*
* Returns:  OSStatus		  - error code (0 == no error, means we dealt with the command) 
*/
OSStatus Handle_SelectAll(void)
	{
	OSStatus status = eventNotHandledErr;

	// looking for the frontmost window containing a HIATSUIView
	WindowRef frontWindow;
	HIViewRef atsuiView;
	Get_FrontWindowAndATSUIView(&frontWindow, &atsuiView, false);
	require(atsuiView != NULL, SelectAllExit);
	
	status = HIATSUIViewSetSelection(atsuiView, kATSUFromTextBeginning, kATSUToTextEnd);
	HIViewSetNeedsDisplay(atsuiView, true);
	require_noerr(status, SelectAllExit);

SelectAllExit:

	return status;
	}   // Handle_SelectAll

/*****************************************************
*
* Handle_Copy() 
*
* Purpose:  called to respond to the Copy command
*
* Inputs:   none
*
* Returns:  OSStatus		  - error code (0 == no error, means we dealt with the command) 
*/
OSStatus Handle_Copy(void)
	{
	OSStatus status = eventNotHandledErr;

	// looking for the frontmost window containing a HIATSUIView
	WindowRef frontWindow;
	HIViewRef atsuiView;
	Get_FrontWindowAndATSUIView(&frontWindow, &atsuiView, false);
	require(atsuiView != NULL, CopyExit);
	
	// getting the selection range
	UniCharArrayOffset start, end;
	status = HIATSUIViewGetSelection(atsuiView, &start, &end);
	require_noerr(status, CantGetSelection);
	require(start != end, CantGetSelection);

	// do the copy
	status = HIATSUIViewPutContentInPasteboard(atsuiView, gClipboard, start, end);
	require_noerr(status, CantPutContentInPasteboard);

	// update the "Show Clipboard" window
	gClipboardChanged = true;
	if (gClipboardWindow != NULL)
		if (IsWindowVisible(gClipboardWindow))
			{
			status = Update_ClipboardWindow();
			require_noerr(status, CopyExit);
			}

CantPutContentInPasteboard:
CantGetSelection:
CopyExit:

	return status;
	}   // Handle_Copy

/*****************************************************
*
* Do_CreateWithTextWindow(outWindow) 
*
* Purpose:  called to create a new window which shows how to use the HICreateATSUIViewWithText API
*
* Notes:    called by Handle_CommandProcess()
*
* Inputs:   none
*
* Returns:  OSStatus			- error code (0 == no error) 
*/
OSStatus Do_CreateWithTextWindow(void)
	{
	WindowRef aWindowRef = NULL;
	CFStringRef theTitle = NULL;
	HIViewRef atsuiView = NULL;
	CFStringRef demoText6 = NULL;
	UniCharCount * runLengths = NULL;
	ATSUStyle * styles = NULL;
	ItemCount numberOfRuns;
	OSStatus status;
	
	// creating the window title
	theTitle = CFStringCreateWithFormat(NULL, NULL, CFSTR("Using HICreateATSUIViewWithText #%ld"), ++gWindowCount);
	require(theTitle != NULL, CantSetTitle);

	// grabbing the text from our resources
	demoText6 = CFCopyLocalizedString( CFSTR("demoText6"), CFSTR("") );
	require(demoText6 != NULL, CantCreateATSUIView);

	// using a convenience function to set up our styles
	status = AnotherConvenienceSetUp_MultipleStyles(&numberOfRuns, &runLengths, &styles, (gWindowCount % 2 == 1), CFStringGetLength(demoText6));
	require_noerr(status, CantCreateATSUIView);
	
	// creating the HIATSUIView with a frame and no wrapping of text
	HIRect hiRect = { { 20, 20 }, { 500, 700 } };
	HISize hiSize = { 800, 0 };
	status = HICreateATSUIViewWithText(&hiRect, demoText6, 0, 0, numberOfRuns, runLengths, styles, true, false, &hiSize, &atsuiView);
	require_noerr(status, CantCreateATSUIView);
	require(atsuiView != NULL, CantCreateATSUIView);
	SetControlID(atsuiView, &kAtsuiID);
	
	// creating the window with that title and HIATSUIView
	HISize theSize = { hiRect.size.width + 2 * hiRect.origin.x, hiRect.size.height + 2 * hiRect.origin.y };
	status = Create_WindowWithTitleAndCustomView(theSize, theTitle, atsuiView, &aWindowRef);
	require_noerr(status, CantCreateWindow);
	
	ShowWindow(aWindowRef);
	
CantCreateWindow:
CantCreateATSUIView:
CantSetTitle:

	// let's cleanup
	if (demoText6 != NULL) CFRelease(demoText6);
	if (styles != NULL) free(styles);
	if (runLengths != NULL) free(runLengths);
	if (theTitle != NULL) CFRelease(theTitle);
	
	return status;
	}   // Do_CreateWithTextWindow

/*****************************************************
*
* Do_CreateFromNibWindow(outWindow) 
*
* Purpose:  called to create a new window which shows how to set up the HIATSUIView from a nib
*
* Notes:    called by Handle_CommandProcess()
*
* Inputs:   none
*
* Returns:  OSStatus			- error code (0 == no error) 
*/
OSStatus Do_CreateFromNibWindow(IBNibRef nibRef)
	{
	WindowRef aWindowRef = NULL;
	CFStringRef theTitle = NULL;
	OSStatus status;
	
	// we need to register our custom HIATSUIView class if it has not been done already
	GetHIATSUIViewClass();

	// creating that window from our nib
	status = CreateWindowFromNib(nibRef, CFSTR("NibTextWindow"), &aWindowRef);
	require_noerr(status, CantCreateWindow);
	require(NULL != aWindowRef, CantCreateWindow);

	// changing the title
	theTitle = CFStringCreateWithFormat(NULL, NULL, CFSTR("Using Nib #%ld"), ++gWindowCount);
	require(theTitle != NULL, CantSetTitle);
	status = SetWindowTitleWithCFString(aWindowRef, theTitle);
	require_noerr(status, CantSetTitle);
	
	ShowWindow(aWindowRef);
	
CantSetTitle:
CantCreateWindow:

	// let's cleanup
	if (theTitle != NULL) CFRelease(theTitle);
	
	return status;
	}   // Do_CreateFromNibWindow

/*****************************************************
*
* Do_OpenATextFile() 
*
* Purpose:  called to create a new window and display the contents of a text file
*
* Notes:    called by Handle_CommandProcess()
*
* Inputs:   none
*
* Returns:  OSStatus			- error code (0 == no error) 
*/
OSStatus Do_OpenATextFile(void)
	{
	WindowRef aWindowRef = NULL;
	CFStringRef theTitle = NULL;
	CFStringRef theText = NULL;
	HIViewRef atsuiView = NULL;
	OSStatus status;
	
	// asking the user to choose a text file
	FSRef theFSRef;
	status = ChooseOneFile(&theFSRef);
	if (status == userCanceledErr) goto DidntGetAFile;
	require_noerr(status, DidntGetAFile);
	
	// creating the window title
	theTitle = CFStringCreateWithFormat(NULL, NULL, CFSTR("Text File #%ld"), ++gWindowCount);
	require(theTitle != NULL, CantSetTitle);
	
	// creating the HIATSUIView
	HIRect hiRect = { { 20, 20 }, { 500, 700 } };
	status = HICreateATSUIView(&hiRect, &atsuiView);
	require_noerr(status, CantCreateATSUIView);
	require(atsuiView != NULL, CantCreateATSUIView);
	SetControlID(atsuiView, &kAtsuiID);
	
	// creating a scroll view
	HIViewRef scrollView;
	status = HIScrollViewCreate(kHIScrollViewOptionsVertScroll, &scrollView);
	require_noerr(status, CantCreateScrollView);
	
	// embedding our HIATSUIView in the scroll view
	status = HIViewAddSubview(scrollView, atsuiView);
	require_noerr(status, CantCreateScrollView);
	
	status = HIViewSetFrame(scrollView, &hiRect);
	require_noerr(status, CantCreateScrollView);

	HIViewSetVisible(atsuiView, true);
	require_noerr(status, CantCreateScrollView);
	
	// creating the window with that title and scroll view
	HISize theSize = { hiRect.size.width + 2 * hiRect.origin.x, hiRect.size.height + hiRect.origin.x + hiRect.origin.y };
	status = Create_WindowWithTitleAndCustomView(theSize, theTitle, scrollView, &aWindowRef);
	require_noerr(status, CantCreateWindow);

	// getting the text from the text file
	theText = GetTextFromFile(&theFSRef);
	require(theText != NULL, CantGetText);

	// creating a default style
	UniCharCount runLength = CFStringGetLength(theText);
	ATSUStyle style;

	Fixed thePointSize = IntToFixed(14);
	ATSUFontID atsuFontID;
	char fontName[] = "Geneva";
	status = ATSUFindFontFromName(fontName, strlen(fontName), kFontFamilyName, kFontMacintoshPlatform, kFontRomanScript, kFontNoLanguageCode, &atsuFontID);
	verify_noerr_string(status, "ATSUFindFontFromName failed.");
	ItemCount numberOfValidTags = (status == noErr) ? 2 : 1;

	status = ATSUCreateStyle(&style);
	require_noerr(status, CantSetStyle);

	ATSUAttributeTag tags[] = {kATSUSizeTag, kATSUFontTag};
	ByteCount sizes[] = {sizeof(Fixed), sizeof(ATSUFontID)};
	ATSUAttributeValuePtr values[] = {&thePointSize, &atsuFontID};
	status = ATSUSetAttributes(style, numberOfValidTags, tags, sizes, values);
	require_noerr(status, CantSetStyle);
	
	// setting our text and styles
	status = HIATSUIViewSetText(atsuiView, theText, 1, &runLength, &style);
	require_noerr(status, CantSetLayout);

	// dealing with the Standard Font panel
	EventTypeSpec eventTypeFS = {kEventClassFont, kEventFontSelection};
	status = InstallWindowEventHandler(aWindowRef, Handle_FontSelection, 1, &eventTypeFS, (void *)aWindowRef, NULL);
	require_noerr(status, CantInstallEventHandler);

#ifdef MAC_OS_X_VERSION_10_4
	status = SetFontInfoForSelection(kFontSelectionATSUIType, 1, &style, GetWindowEventTarget(aWindowRef));
#else
	// there was a small typecast bug in the pre-10.4 headers
	status = SetFontInfoForSelection(kFontSelectionATSUIType, 1, &style, (HIObjectRef)GetWindowEventTarget(aWindowRef));
#endif
	verify_noerr_string(status, "SetFontInfoForSelection failed.");

	Signal_ScrollViewContentChanged(atsuiView);

	ShowWindow(aWindowRef);
	
CantInstallEventHandler:
CantSetLayout:
CantSetStyle:
CantGetText:
CantCreateWindow:
CantCreateScrollView:
CantCreateATSUIView:
CantSetTitle:
DidntGetAFile:

	// let's cleanup
	if (theText != NULL) CFRelease(theText);
	if (theTitle != NULL) CFRelease(theTitle);
	
	return status;
	}   // Do_OpenATextFile

/*****************************************************
*
* Get_FrontWindowAndATSUIView(outWindow, outATSUIView, deep) 
*
* Purpose:  returns the front window and the HIATSUIView if any in that window
*
* Inputs:   outWindow           - the front window or NULL if there is no front window containing a HIATSUIView
*				outATSUIView        - the HIATSUIView of the front window if any or NULL if not
*				deep                - indicates if we just stop looking after the first window, or if we continue to search deeper
*                                 the first window which contains a HIATSUIView
*
* Returns:  none 
*/
void Get_FrontWindowAndATSUIView(WindowRef * outWindow, HIViewRef * outATSUIView, Boolean deep)
	{
	WindowRef frontWindow = GetFrontWindowOfClass(kDocumentWindowClass, true);
	HIViewRef atsuiView = NULL;
	if (frontWindow != NULL)
		HIViewFindByID(HIViewGetRoot(frontWindow), kAtsuiID, &atsuiView);
	
	if (deep)
		{
		while ( (frontWindow != NULL) & (atsuiView == NULL) )
			{
			frontWindow = GetNextWindowOfClass(frontWindow, kDocumentWindowClass, true);
			if (frontWindow != NULL)
				HIViewFindByID(HIViewGetRoot(frontWindow), kAtsuiID, &atsuiView);
			}
		}

	*outWindow = frontWindow;
	*outATSUIView = atsuiView;
	}   // Get_FrontWindowAndATSUIView

//****************************************************
#pragma mark -
#pragma mark * local (static) function implementations *

/*****************************************************
*
* Create_WindowWithTitleAndCustomView(theSize, inTitle, inCustomView, outWindow)
*
* Purpose:  convenience function to create all windows identically but for the title and its custom view
*
* Inputs:   theSize             - the size we desire for the window
*           inTitle             - CFStringRef containing the title
*				inCustomView        - the custom HIView to add to the content view
*				outWindow           - returns the created window (or NULL if creation failed)
*
* Returns:  OSStatus            - noErr indicates the window was created
*
*/
static OSStatus Create_WindowWithTitleAndCustomView(HISize theSize, CFStringRef theTitle, HIViewRef theHIView, WindowRef * outWindow)
	{
	WindowRef aWindowRef = NULL;
	OSStatus status;
	
	require_action(outWindow != NULL, CantCreateWindow, status = paramErr);
	
	// Create a window with adequate attributes
	Rect bounds = {0, 0, theSize.height, theSize.width};
	status = CreateNewWindow(kDocumentWindowClass, kWindowStandardFloatingAttributes | kWindowResizableAttribute | kWindowLiveResizeAttribute | kWindowStandardHandlerAttribute | kWindowCompositingAttribute, &bounds, &aWindowRef);
	require_noerr(status, CantCreateWindow);
	require(aWindowRef != NULL, CantCreateWindow);
	
	status = RepositionWindow(aWindowRef, NULL, kWindowCascadeOnMainScreen);
	require_noerr(status, CantCreateWindow);
	
	// add our custom HIView
	HIViewRef contentView;
	status = HIViewFindByID(HIViewGetRoot(aWindowRef), kHIViewWindowContentID, &contentView);
	require_noerr(status, CantCreateWindow);
	
	HIViewAddSubview(contentView, theHIView);
	HIViewSetVisible(theHIView, true);

	// set the layout info for our view for automated frame adjustment when resized
	HILayoutInfo layout = {
		kHILayoutInfoVersionZero,
		{
			{ NULL, kHILayoutBindTop },
			{ NULL, kHILayoutBindLeft },
			{ NULL, kHILayoutBindBottom },
			{ NULL, kHILayoutBindRight }
		},
		{
			{ NULL, 0.0 },
			{ NULL, 0.0 }
		},
		{
			{ NULL, kHILayoutPositionNone },
			{ NULL, kHILayoutPositionNone }
		}
	};
	status = HIViewSetLayoutInfo(theHIView, &layout);
	require_noerr(status, CantSetUpView);
	
	// set the title
	status = SetWindowTitleWithCFString(aWindowRef, theTitle);
	require_noerr(status, CantSetTitle);
	
	// add a handler to deal with the Standard Font Menu
	EventTypeSpec eventTypeASF = {kEventClassATSUI, kEventATSUISetFont};
	status = InstallWindowEventHandler(aWindowRef, Handle_WindowSetFont, 1, &eventTypeASF, (void *)aWindowRef, NULL);
	require_noerr(status, CantInstallEventHandler);
	
CantInstallEventHandler:
CantSetTitle:
CantAllocateWindowData:
CantSetUpView:
CantCreateWindow:
	
	if (outWindow != NULL)
		*outWindow = aWindowRef;
	
	return status;
	}   // Create_WindowWithTitleAndCustomView

/*****************************************************
*
* Handle_FontSelection(inHandlerCallRef, inEvent, inUserData) 
*
* Purpose:  called when the user changes the settings in the System Font Panel
*
* Inputs:   inHandlerCallRef    - reference to the current handler call chain
*				inEvent             - the event
*           inUserData          - app-specified data you passed in the call to InstallEventHandler
*
* Returns:  OSStatus            - noErr indicates the event was handled
*                                 eventNotHandledErr indicates the event was not handled and the Toolbox should take over
*/
static pascal OSStatus Handle_FontSelection(EventHandlerCallRef inHandlerCallRef, EventRef inEvent, void *inUserData)
	{
	OSStatus status = noErr;
	WindowRef aWindowRef = (WindowRef) inUserData;

	// finding our HIATSUIView
	HIViewRef atsuiView;
	status = HIViewFindByID(HIViewGetRoot(aWindowRef), kAtsuiID, &atsuiView);
	require_noerr(status, FontSelectionExit);
	require(atsuiView != NULL, FontSelectionExit);
	
	// retrieving the font id and size from the event's parameters
	ATSUFontID atsuFontID;
	status = GetEventParameter(inEvent, kEventParamATSUFontID, typeATSUFontID, NULL, sizeof(atsuFontID), NULL, &atsuFontID);
	require_noerr(status, FontSelectionExit);
	
	Fixed atsuSize;
	status = GetEventParameter(inEvent, kEventParamATSUFontSize, typeATSUSize, NULL, sizeof(atsuSize), NULL, &atsuSize);
	require_noerr(status, FontSelectionExit);

	// setting the main (first) style of our HIATSUIView
	ATSUStyle style;
	status = Set_FontAndSizeToFirstStyle(atsuiView, atsuFontID, atsuSize, &style);
	require_noerr(status, FontSelectionExit);
	
	// if we have a font variant features drawer then update it with the new font
	WindowRef featureWindow = (WindowRef)GetWRefCon(aWindowRef);
	if (featureWindow != NULL)
		Populate_FeatureWindow(featureWindow, atsuFontID, style);

FontSelectionExit:

	// if there are no errors then we need to return eventNotHandledErr
	// so that the HIToolbox gets a chance to do what it has to do.
	if (status == noErr) status = eventNotHandledErr;

	return status;
	}   // Handle_FontSelection

/*****************************************************
*
* Handle_WindowSetFont(inHandlerCallRef, inEvent, inUserData) 
*
* Purpose:  called when the user selects a font in the Standard Font menu
*
* Inputs:   inHandlerCallRef    - reference to the current handler call chain
*				inEvent             - the event
*           inUserData          - app-specified data you passed in the call to InstallEventHandler
*
* Returns:  OSStatus            - noErr indicates the event was handled
*                                 eventNotHandledErr indicates the event was not handled and the Toolbox should take over
*/
static pascal OSStatus Handle_WindowSetFont(EventHandlerCallRef inHandlerCallRef, EventRef inEvent, void *inUserData)
	{
	OSStatus status = noErr;
	WindowRef aWindowRef = (WindowRef) inUserData;

	// finding our HIATSUIView
	HIViewRef atsuiView;
	status = HIViewFindByID(HIViewGetRoot(aWindowRef), kAtsuiID, &atsuiView);
	require_noerr(status, SetFontExit);
	require(atsuiView != NULL, SetFontExit);

	// retrieving the font id from the event's parameters
	ATSUFontID atsuFontID;
	status = GetEventParameter(inEvent, kEventParamATSUFontID, typeATSUFontID, NULL, sizeof(atsuFontID), NULL, &atsuFontID);
	require_noerr(status, SetFontExit);
	
	// setting the main (first) style of our HIATSUIView
	ATSUStyle style;
	status = Set_FontAndSizeToFirstStyle(atsuiView, atsuFontID, 0, &style);
	require_noerr(status, SetFontExit);

	// updating the System Font Panel information if present
#ifdef MAC_OS_X_VERSION_10_4
	status = SetFontInfoForSelection(kFontSelectionATSUIType, 1, &style, GetWindowEventTarget(aWindowRef));
#else
	// there was a small typecast bug in the pre-10.4 headers
	status = SetFontInfoForSelection(kFontSelectionATSUIType, 1, &style, (HIObjectRef)GetWindowEventTarget(aWindowRef));
#endif
	
	// if we have a font variant features drawer then update it with the new font
	WindowRef featureWindow = (WindowRef)GetWRefCon(aWindowRef);
	if (featureWindow != NULL)
		Populate_FeatureWindow(featureWindow, atsuFontID, style);
	
SetFontExit:
	return eventNotHandledErr;
	}   // Handle_WindowSetFont

/*****************************************************
*
* ConvenienceSetUp_MultipleStyles(outNumberOfRuns, outRunLengths, outStyles) 
*
* Purpose:  Setup arrays as a convenience for the 2 functions Do_NewSingleLineMultipleStyleWindow and Do_NewSingleVerticalLineMultipleStyleWindow
*
* Outputs:  outNumberOfRuns     - returning the number of runs
*           outRunLengths       - returning the run lengths array
*				outStyles           - returning the styles arrays
*
* Returns:  OSStatus		        - error code (0 == no error) 
*/
static OSStatus ConvenienceSetUp_MultipleStyles(ItemCount * outNumberOfRuns, UniCharCount ** outRunLengths, ATSUStyle ** outStyles)
	{
	UniCharCount * runLengths = NULL;
	ATSUStyle * styles = NULL;
	OSStatus status = noErr;
	
	require_action(outNumberOfRuns != NULL, ExitSetup, status = paramErr);
	require_action(outRunLengths != NULL, ExitSetup, status = paramErr);
	require_action(outStyles != NULL, ExitSetup, status = paramErr);

	// let's have 5 runs, setting the 4th word "much" and the last word "styles" in a different style
	ItemCount numberOfRuns = 5;
	runLengths = (UniCharCount *)malloc(numberOfRuns * sizeof(UniCharCount));
	require(runLengths != NULL, CantAllocateMemory);
	runLengths[0] = 10;
	runLengths[1] = 4;
	runLengths[2] = 39;
	runLengths[3] = 6;
	runLengths[4] = 1;

	// we only use 3 styles, the 3rd and 5th runs use the same one as the first
	// but we still have to dimension the styles array the same as the runLengths array
	styles = (ATSUStyle *)malloc(numberOfRuns * sizeof(ATSUStyle));
	require(styles != NULL, CantAllocateMemory);

	ATSUStyle tempS;
	ATSUFontID atsuFont;
	ItemCount numberOfValidTags;

	// Main style is just default style in size 16
	status = ATSUCreateStyle(&tempS);
	require_noerr(status, CantSetStyle);

	Fixed thePointSize = IntToFixed(16);
	ATSUAttributeTag theTag = kATSUSizeTag;
	ByteCount theTagSize = sizeof(Fixed);
	ATSUAttributeValuePtr theValuePtr = &thePointSize;
	status = ATSUSetAttributes(tempS, 1, &theTag, &theTagSize, &theValuePtr);
	require_noerr(status, CantSetStyle);

	styles[0] = styles[2] = styles[4] = tempS;

	// Second style is vertical italic blue Helvetica in size 48
	status = ATSUCreateStyle(&tempS);
	require_noerr(status, CantSetStyle);

	thePointSize = IntToFixed(48);
	RGBColor blueColor = {0, 0, 0xFFFF};
	Boolean italic = true;
	ATSUVerticalCharacterType direction = kATSUStronglyVertical;
	char fontName1[] = "Helvetica";
	status = ATSUFindFontFromName(fontName1, strlen(fontName1), kFontFamilyName, kFontMacintoshPlatform, kFontRomanScript, kFontNoLanguageCode, &atsuFont);
	verify_noerr_string(status, "ATSUFindFontFromName failed.");
	numberOfValidTags = (status == noErr) ? 5 : 4;
	ATSUAttributeTag tags[] = {kATSUSizeTag, kATSUColorTag, kATSUQDItalicTag, kATSUVerticalCharacterTag, kATSUFontTag};
	ByteCount sizes[] = {sizeof(Fixed), sizeof(RGBColor), sizeof(Boolean), sizeof(ATSUVerticalCharacterType), sizeof(ATSUFontID)};
	ATSUAttributeValuePtr values[] = {&thePointSize, &blueColor, &italic, &direction, &atsuFont};
	status = ATSUSetAttributes(tempS, numberOfValidTags, tags, sizes, values);
	require_noerr(status, CantSetStyle);

	styles[1] = tempS;
	
	// Third and last style is underlined Courier in size 60
	status = ATSUCreateStyle(&tempS);
	require_noerr(status, CantSetStyle);

	thePointSize = IntToFixed(60);
	Boolean underline = true;
	char fontName2[] = "Courier";
	status = ATSUFindFontFromName(fontName2, strlen(fontName2), kFontFamilyName, kFontMacintoshPlatform, kFontRomanScript, kFontNoLanguageCode, &atsuFont);
	verify_noerr_string(status, "ATSUFindFontFromName failed.");
	numberOfValidTags = (status == noErr) ? 3 : 2;
	ATSUAttributeTag tags2[] = {kATSUSizeTag, kATSUQDUnderlineTag, kATSUFontTag};
	ByteCount sizes2[] = {sizeof(Fixed), sizeof(Boolean), sizeof(ATSUFontID)};
	ATSUAttributeValuePtr values2[] = {&thePointSize, &underline, &atsuFont};
	status = ATSUSetAttributes(tempS, numberOfValidTags, tags2, sizes2, values2);
	require_noerr(status, CantSetStyle);
	
	styles[3] = tempS;

CantSetStyle:
CantAllocateMemory:

	*outNumberOfRuns = numberOfRuns;
	*outRunLengths = runLengths;
	*outStyles = styles;

ExitSetup:

	return status;
	}   // ConvenienceSetUp_MultipleStyles

/*****************************************************
*
* Handle_RotateCommandProcess(inHandlerCallRef, inEvent, inUserData) 
*
* Purpose:  called when the user clicks on the "Rotate" button of the Single Vertical Line Multiple Style Window
*
* Inputs:   inHandlerCallRef    - reference to the current handler call chain
*				inEvent             - the event
*           inUserData          - app-specified data you passed in the call to InstallEventHandler
*
* Returns:  OSStatus            - noErr indicates the event was handled
*                                 eventNotHandledErr indicates the event was not handled and the Toolbox should take over
*/
static pascal OSStatus Handle_RotateCommandProcess(EventHandlerCallRef inHandlerCallRef, EventRef inEvent, void *inUserData)
	{
	OSStatus status = eventNotHandledErr;
	HIViewRef atsuiView = (HIViewRef)inUserData;

	// getting the command
	HICommand aCommand;	
	GetEventParameter(inEvent, kEventParamDirectObject, typeHICommand, NULL, sizeof(HICommand), NULL, &aCommand);
	
	switch (aCommand.commandID)
		{
		case 'Rott':
			{
			// small animation of rotated text
			long i;
			for (i = 0; i >= -90; i -= 5)
				{
				atsuSetLineRotation(HIATSUIViewGetTextLayout(atsuiView), IntToFixed(0), IntToFixed(i));
				HIViewSetNeedsDisplay(atsuiView, true);
				HIWindowFlush(GetControlOwner(atsuiView));
				}

			status = noErr;
			break;
			}
		}

	return status;
	}   // Handle_RotateCommandProcess

/*****************************************************
*
* Handle_SmallerBiggerCommandProcess(inHandlerCallRef, inEvent, inUserData) 
*
* Purpose:  called when the user clicks on the "Smaller" or "Bigger" button of the Paragraphs Window
*
* Inputs:   inHandlerCallRef    - reference to the current handler call chain
*				inEvent             - the event
*           inUserData          - app-specified data you passed in the call to InstallEventHandler
*
* Returns:  OSStatus            - noErr indicates the event was handled
*                                 eventNotHandledErr indicates the event was not handled and the Toolbox should take over
*/
static pascal OSStatus Handle_SmallerBiggerCommandProcess(EventHandlerCallRef inHandlerCallRef, EventRef inEvent, void *inUserData)
	{
	OSStatus status = eventNotHandledErr;
	HIViewRef atsuiView = (HIViewRef)inUserData;
	Fixed thePointSize, variation = 0;

	// getting the command
	HICommand aCommand;
	GetEventParameter(inEvent, kEventParamDirectObject, typeHICommand, NULL, sizeof(HICommand), NULL, &aCommand);
	switch (aCommand.commandID)
		{
		case 'Smlr': variation = IntToFixed(-6); break;
		case 'Bggr': variation = IntToFixed( 6); break;
		}

	// applying the variation to all styles of the text to make bigger or smaller
	// since 1 ATSUStyle might be shared by more than 1 run, we need to be careful
	// to not apply the variation twice or more to the same ATSUStyle.
	if (variation != 0)
		{
		ItemCount i, numberOfRuns;
		long j;
		ATSUStyle * styles = HIATSUIViewGetStyles(atsuiView, &numberOfRuns);
		for (i = 0; i < numberOfRuns; i++)
			{
			ATSUStyle tempS = styles[i];
			Boolean seenIt = false;
			
			// did we change this style already?
			if (i >= 1) for (j = i-1; (j >= 0) && (!seenIt); j--) seenIt = (styles[j] == tempS);
			
			if (!seenIt)
				{
				// get the current font size
				status = atsuGetSize(tempS, &thePointSize);

				// add the variation
				thePointSize += variation;
				
				// let's have reasonable values:
				if (variation > 0) if (thePointSize > IntToFixed(84)) thePointSize = IntToFixed(84);
				if (variation < 0) if (thePointSize < IntToFixed( 4)) thePointSize = IntToFixed( 4);

				// set the new font size in the style
				status = atsuSetSize(tempS, thePointSize);
				}
			}

		// we must break the text layout again since we changed the styles
		// and ask for a refresh
		HIATSUIViewBreakLines(atsuiView);
		HIViewSetNeedsDisplay(atsuiView, true);
		status = noErr;
		}

	return status;
	}   // Handle_SmallerBiggerCommandProcess

/*****************************************************
*
* AnotherConvenienceSetUp_MultipleStyles(outNumberOfRuns, outRunLengths, outStyles, bigOrSmall, textLength) 
*
* Purpose:  Setup arrays as a convenience for the 4 functions:
*				Do_NewHilightingWindow, Do_NewHitTestingWindow, Do_NewScrollingWindow, and Do_CreateWithTextWindow
*
* Outputs:  outNumberOfRuns     - returning the number of runs
*           outRunLengths       - returning the run lengths array
*				outStyles           - returning the styles arrays
*				bigOrSmall          - detemines if we want big text or small text
*				textLength          - length of the text
*
* Returns:  OSStatus		        - error code (0 == no error) 
*/
static OSStatus AnotherConvenienceSetUp_MultipleStyles(ItemCount * outNumberOfRuns, UniCharCount ** outRunLengths, ATSUStyle ** outStyles, Boolean bigOrSmall, UniCharArrayOffset textLength)
	{
	UniCharCount * runLengths = NULL;
	ATSUStyle * styles = NULL;
	OSStatus status = noErr;
	
	require_action(outNumberOfRuns != NULL, ExitSetup, status = paramErr);
	require_action(outRunLengths != NULL, ExitSetup, status = paramErr);
	require_action(outStyles != NULL, ExitSetup, status = paramErr);

	// let's have 5 runs, setting the 4th word "much" and the last word "styles" in a different style
	ItemCount numberOfRuns = 5;
	runLengths = (UniCharCount *)malloc(numberOfRuns * sizeof(UniCharCount));
	require(runLengths != NULL, CantAllocateMemory);
	runLengths[0] = 228;
	runLengths[1] = 36;
	runLengths[2] = 135;
	runLengths[3] = 44;
	runLengths[4] = textLength - 228 - 36 - 135 - 44;

	// we only use 3 styles, the 3rd and 5th runs use the same one as the first
	// but we still have to dimension the styles array the same as the runLengths array
	styles = (ATSUStyle *)malloc(numberOfRuns * sizeof(ATSUStyle));
	require(styles != NULL, CantAllocateMemory);

	ATSUStyle tempS;
	ATSUFontID atsuFont;
	ItemCount numberOfValidTags;

	// Main style is just default style in size 16 (or 32 if bigOrSmall is true)
	status = ATSUCreateStyle(&tempS);
	require_noerr(status, CantSetStyle);

	Fixed thePointSize = ( bigOrSmall ) ? IntToFixed(32) : IntToFixed(16);
	ATSUAttributeTag theTag = kATSUSizeTag;
	ByteCount theTagSize = sizeof(Fixed);
	ATSUAttributeValuePtr theValuePtr = &thePointSize;
	status = ATSUSetAttributes(tempS, 1, &theTag, &theTagSize, &theValuePtr);
	require_noerr(status, CantSetStyle);

	styles[0] = styles[2] = styles[4] = tempS;

	// Second style is bold underlined Helvetica in size 36 (or 72 if bigOrSmall is true)
	status = ATSUCreateStyle(&tempS);
	require_noerr(status, CantSetStyle);

	thePointSize = ( bigOrSmall ) ? IntToFixed(72) : IntToFixed(36);
	Boolean bold = true;
	Boolean underline = true;
	char fontName1[] = "Helvetica";
	status = ATSUFindFontFromName(fontName1, strlen(fontName1), kFontFamilyName, kFontMacintoshPlatform, kFontRomanScript, kFontNoLanguageCode, &atsuFont);
	verify_noerr_string(status, "ATSUFindFontFromName failed.");
	numberOfValidTags = (status == noErr) ? 4 : 3;
	ATSUAttributeTag tags[] = {kATSUSizeTag, kATSUQDBoldfaceTag, kATSUQDUnderlineTag, kATSUFontTag};
	ByteCount sizes[] = {sizeof(Fixed), sizeof(Boolean), sizeof(Boolean), sizeof(ATSUFontID)};
	ATSUAttributeValuePtr values[] = {&thePointSize, &bold, &underline, &atsuFont};
	status = ATSUSetAttributes(tempS, numberOfValidTags, tags, sizes, values);
	require_noerr(status, CantSetStyle);

	styles[1] = tempS;
	
	// Third and last style is bold italic underlined Courier in size 20
	status = ATSUCreateStyle(&tempS);
	require_noerr(status, CantSetStyle);

	Boolean italic = true;
	char fontName2[] = "Courier";
	status = ATSUFindFontFromName(fontName2, strlen(fontName2), kFontFamilyName, kFontMacintoshPlatform, kFontRomanScript, kFontNoLanguageCode, &atsuFont);
	verify_noerr_string(status, "ATSUFindFontFromName failed.");
	numberOfValidTags = (status == noErr) ? 5 : 4;
	ATSUAttributeTag tags2[] = {kATSUSizeTag, kATSUQDBoldfaceTag, kATSUQDItalicTag, kATSUQDUnderlineTag, kATSUFontTag};
	ByteCount sizes2[] = {sizeof(Fixed), sizeof(Boolean), sizeof(Boolean), sizeof(Boolean), sizeof(ATSUFontID)};
	ATSUAttributeValuePtr values2[] = {&thePointSize, &bold, &italic, &underline, &atsuFont};
	status = ATSUSetAttributes(tempS, numberOfValidTags, tags2, sizes2, values2);
	require_noerr(status, CantSetStyle);
	
	styles[3] = tempS;

CantSetStyle:
CantAllocateMemory:

	*outNumberOfRuns = numberOfRuns;
	*outRunLengths = runLengths;
	*outStyles = styles;

ExitSetup:

	return status;
	}   // AnotherConvenienceSetUp_MultipleStyles

/*****************************************************
*
* Handle_MoveHilightingCommandProcess(inHandlerCallRef, inEvent, inUserData) 
*
* Purpose:  called when the user clicks on the "Home", "Previous Word", "Next Word", or "Next 30" button of the Hilighting Window
*
* Inputs:   inHandlerCallRef    - reference to the current handler call chain
*				inEvent             - the event
*           inUserData          - app-specified data you passed in the call to InstallEventHandler
*
* Returns:  OSStatus            - noErr indicates the event was handled
*                                 eventNotHandledErr indicates the event was not handled and the Toolbox should take over
*/
static pascal OSStatus Handle_MoveHilightingCommandProcess(EventHandlerCallRef inHandlerCallRef, EventRef inEvent, void *inUserData)
	{
	OSStatus status = eventNotHandledErr;
	HIViewRef atsuiView = (HIViewRef)inUserData;
	
	// getting the current selection
	UniCharArrayOffset oldStart, oldEnd;
	status = HIATSUIViewGetSelection(atsuiView, &oldStart, &oldEnd);
	require_noerr(status, ExitHandler);
	UniCharArrayOffset newStart = oldStart, newEnd = oldEnd;

	// getting the command and moving the offsets
	HICommand aCommand;	
	GetEventParameter(inEvent, kEventParamDirectObject, typeHICommand, NULL, sizeof(HICommand), NULL, &aCommand);
	switch (aCommand.commandID)
		{
		case 'Home': newStart = newEnd = 0; break;
		case 'Nx30': newStart = oldEnd; newEnd = newStart + 30; break;
		case 'Prev':
			{
			status = ATSUPreviousCursorPosition(HIATSUIViewGetTextLayout(atsuiView), oldStart, kATSUByWord, &newStart);
			require_noerr(status, ExitHandler);
			status = ATSUNextCursorPosition(HIATSUIViewGetTextLayout(atsuiView), newStart, kATSUByWord, &newEnd);
			require_noerr(status, ExitHandler);
			break;
			}
		case 'Next':
			{
			status = ATSUNextCursorPosition(HIATSUIViewGetTextLayout(atsuiView), oldStart, kATSUByWord, &newStart);
			require_noerr(status, ExitHandler);
			status = ATSUNextCursorPosition(HIATSUIViewGetTextLayout(atsuiView), newStart, kATSUByWord, &newEnd);
			require_noerr(status, ExitHandler);
			break;
			}
		default: status = eventNotHandledErr;
		}
	
	// if the selection changed then update the HIATSUIView
	// only in this case, do we set status to noErr
	// so that the command does not propagate
	if ((newStart != oldStart) || (newEnd != oldEnd))
		{
		HIATSUIViewSetSelection(atsuiView, newStart, newEnd);
		HIViewSetNeedsDisplay(atsuiView, true);
		status = noErr;
		}

ExitHandler:
	return status;
	}   // Handle_MoveHilightingCommandProcess

/*****************************************************
*
* Signal_ScrollViewContentChanged(theEmbeddedView) 
*
* Purpose:  called when we want to make the HIScrollView aware that the content (the HIATSUIView)
*				changed and that the scroll bars should probably be updated to reflect the change
*
* Inputs:   theEmbeddedView     - reference to the view being scrolled
*
* Returns:  OSStatus			     - error code (0 == no error) 
*/
static OSStatus Signal_ScrollViewContentChanged(HIViewRef theEmbeddedView)
	{
	OSStatus status = noErr;

	// creating the event
	EventRef theEvent = NULL;
	status = CreateEvent(NULL, kEventClassScrollable, kEventScrollableInfoChanged, GetCurrentEventTime(), kEventAttributeUserEvent, &theEvent);
	require_noerr(status, CantCreateEvent);

	// sending the event
	status = SendEventToEventTarget(theEvent, GetControlEventTarget(HIViewGetSuperview(theEmbeddedView)));
	require_noerr(status, CantSendEvent);

CantSendEvent:
CantCreateEvent:

	// releasing the event
	if (theEvent != NULL) ReleaseEvent(theEvent);

	return status;
	}   // Signal_ScrollViewContentChanged

/*****************************************************
*
* MyScrollingTextTimeProc(inTimer, inUserData) 
*
* Purpose:  Carbon Event Timer Proc for our autoscroll
*
* Inputs:   inTimer             - reference to the EventLoopTimerRef
*           inUserData          - app-specified data you passed in the call to InstallEventLoopTimer
*
* Returns:  none 
*/
static void MyScrollingTextTimeProc(EventLoopTimerRef inTimer, void *inUserData)
	{
	HIATSUIViewData* myData = (HIATSUIViewData*)inUserData;
	
	// let's scroll by 1 pixel each iteration
	HIPoint where = { 0, myData->originPoint.y + 1 };
	
	// If we reached the end of our text, let's start again
	if (where.y >= myData->textSize.height - 10.0) where.y = 0.0;

	// creating our ScrollTo event
	OSStatus status = noErr;
	EventRef theEvent = NULL;
	status = CreateEvent(NULL, kEventClassScrollable, kEventScrollableScrollTo, GetCurrentEventTime(), kEventAttributeUserEvent, &theEvent);
	require_noerr(status, ExitTimeProc);

	// adding the point to our parameters
	status = SetEventParameter(theEvent, kEventParamOrigin, typeHIPoint, sizeof(where), &where);
	require_noerr(status, ExitTimeProc);

	// sending the event
	status = SendEventToEventTarget(theEvent, GetControlEventTarget(myData->view));
	require_noerr(status, ExitTimeProc);

ExitTimeProc:

	// releasing the event
	if (theEvent != NULL) ReleaseEvent(theEvent);

	return;
	}   // MyScrollingTextTimeProc

/*****************************************************
*
* StartStopAutoScroll(theHIATSUIView, start) 
*
* Purpose:  start or stop the auto-scroll, we need to remove/add the HIScrollView so that the User is not
*				tempted to "fight" with the auto-scroll
*
* Inputs:   theHIATSUIView      - reference to the HIATSUIView
*           start               - Boolean, if true we start auto-scrolling, if false we stop
*
* Returns:  none 
*/
static OSStatus StartStopAutoScroll(HIViewRef theHIATSUIView, Boolean start)
	{
	OSStatus status = noErr;
	
	// getting our private data
	HIATSUIViewData* myData = (HIATSUIViewData*)HIObjectDynamicCast((HIObjectRef)theHIATSUIView, kHIATSUIViewClass);
	require(myData != NULL, CantGetData);

	// we keep the Carbon Event Timer reference in the refcon of our window so we need to know the window
	WindowRef aWindowRef = GetControlOwner(theHIATSUIView);
	require(aWindowRef != NULL, CantSwitchViews);

	HIViewRef scrollView, parentView;
	HIRect frame;

	if (start)
		{
		// getting the HIScrollView which is our parent and that we need to remove
		scrollView = HIViewGetSuperview(myData->view);
		require(scrollView != NULL, CantSwitchViews);

		// getting its frame so that we can set ourselves in the same place
		status = HIViewGetFrame(scrollView, &frame);
		require_noerr(status, CantSwitchViews);
		
		// getting the HIScollView's parent that we will make our own parent
		parentView = HIViewGetSuperview(scrollView);
		require(parentView != NULL, CantSwitchViews);

		// removing ourselves from the HIScrollView
		status = HIViewRemoveFromSuperview(myData->view);
		require_noerr(status, CantSwitchViews);
		
		// removing the HIScrollView
		status = HIViewRemoveFromSuperview(scrollView);
		require_noerr(status, CantSwitchViews);
		
		// Adding ourselves to the parent
		status = HIViewAddSubview(parentView, myData->view);
		require_noerr(status, CantSwitchViews);
		
		// setting our frame
		status = HIViewSetFrame(myData->view, &frame);
		require_noerr(status, CantSwitchViews);
		
		// releasing the HIScrollView which is no longer useful
		CFRelease(scrollView);

		// resetting our scrolling data
		myData->originPoint.x = myData->originPoint.y = 0;
		
		// installing the Carbon Event Timer which drives our auto-scrolling and storing its reference as our window's refCon
		EventLoopTimerRef theTimer;
		status = InstallEventLoopTimer(GetCurrentEventLoop(), TicksToEventTime(1), TicksToEventTime(2), MyScrollingTextTimeProc, myData, &theTimer);
		require_noerr(status, CantSwitchViews);

		SetWRefCon(aWindowRef, (long)theTimer);
		}
	else
		{
		// getting our Carbon Event Timer and cancelling it
		EventLoopTimerRef theTimer = (EventLoopTimerRef)GetWRefCon(aWindowRef);
		if (theTimer != NULL) RemoveEventLoopTimer(theTimer);
		SetWRefCon(aWindowRef, 0);
		
		// resetting our scrolling data
		myData->originPoint.x = myData->originPoint.y = 0;

		// getting our parent
		parentView = HIViewGetSuperview(myData->view);
		require(parentView != NULL, CantSwitchViews);

		// getting our frame
		status = HIViewGetFrame(myData->view, &frame);
		require_noerr(status, CantSwitchViews);

		// creating the HIScrollView
		status = HIScrollViewCreate(kHIScrollViewOptionsVertScroll, &scrollView);
		require_noerr(status, CantSwitchViews);

		// setting the HIScrollView's frame to our own
		status = HIViewSetFrame(scrollView, &frame);
		require_noerr(status, CantSwitchViews);

		// removing ourselves from our parent
		status = HIViewRemoveFromSuperview(myData->view);
		require_noerr(status, CantSwitchViews);

		// and adding ourselves inside the HIScrollView
		status = HIViewAddSubview(scrollView, myData->view);
		require_noerr(status, CantSwitchViews);

		// adding the HIScrollView to our previous parent
		status = HIViewAddSubview(parentView, scrollView);
		require_noerr(status, CantSwitchViews);

		// and making the HIScrollView visible
		status = HIViewSetVisible(scrollView, true);
		require_noerr(status, CantSwitchViews);
		}

CantSwitchViews:
CantGetData:

	return status;
	}   // StartStopAutoScroll

/*****************************************************
*
* Handle_FrameWrapCommandProcess(inHandlerCallRef, inEvent, inUserData) 
*
* Purpose:  called when the user clicks on the "With Frame", "Wrap Text", "With Background", "With Transparence",
*				"Left", "Center", "Right", and "Auto-Scroll" buttons of the Scrolling Window
*
* Inputs:   inHandlerCallRef    - reference to the current handler call chain
*				inEvent             - the event
*           inUserData          - app-specified data you passed in the call to InstallEventHandler
*
* Returns:  OSStatus            - noErr indicates the event was handled
*                                 eventNotHandledErr indicates the event was not handled and the Toolbox should take over
*/
static pascal OSStatus Handle_FrameWrapCommandProcess(EventHandlerCallRef inHandlerCallRef, EventRef inEvent, void *inUserData)
	{
	OSStatus status;
	HIViewRef atsuiView = (HIViewRef)inUserData;

	// getting the command and making sure it comes from a control
	HICommandExtended aCommand;	
	status = GetEventParameter(inEvent, kEventParamDirectObject, typeHICommand, NULL, sizeof(HICommandExtended), NULL, &aCommand);
	require_noerr(status, ExitHandler);
	status = eventNotHandledErr;
	if ( ! (aCommand.attributes & kHICommandFromControl) ) goto ExitHandler;
	status = noErr;
	
	// getting the control's value
	SInt32 controlValue = GetControl32BitValue(aCommand.source.control);
	HISize hardCodedSize = { 800, 0 };
	
	switch (aCommand.commandID)
		{
		case 'Frme':
			// setting the frame, yes or no, and updating the HIATSUIView
			status = HIATSUIViewSetWithFrame(atsuiView, controlValue == 1);
			require_noerr(status, ExitHandler);

			HIViewSetNeedsDisplay(atsuiView, true);
			break;
		case 'Wrap':
			// setting the wrap, yes or no, and if no, then use a hard-coded value for the text bounds
			status = HIATSUIViewSetWrap(atsuiView, controlValue == 1, &hardCodedSize);
			require_noerr(status, ExitHandler);
			
			// Needs to send a kEventScrollableInfoChanged to atsuiView parent so that the ScrollView knows about the internal change
			// We must not signal if we are auto-scrolling because our parent is not a HIScrollView in this case
			if (HIObjectIsOfClass((HIObjectRef)HIViewGetSuperview(atsuiView), kHIScrollViewClassID))
				Signal_ScrollViewContentChanged(atsuiView);
			
			HIViewSetNeedsDisplay(atsuiView, true);
			break;
		case 'Bckg':
			// the refCon of the HIATSUIView contains the HIViewRef of the Background Image control
			// so we just have to make it visible or not
			HIViewSetVisible((HIViewRef)GetControlReference(aCommand.source.control), (controlValue == 1));
			break;
		case 'Trns':
			// setting the text and background colors and updating the HIATSUIView
			status = HIATSUIViewSetColor(atsuiView, (controlValue == 1), 0.0, 0.0, 1.0, 0.4);
			require_noerr(status, ExitHandler);
			
			status = HIATSUIViewSetBackColor(atsuiView, (controlValue == 1), 1.0, 0.0, 0.0, 0.4);
			require_noerr(status, ExitHandler);
			
			HIViewSetNeedsDisplay(atsuiView, true);
			break;
		case 'Left':
			// setting the required alignment and updating the HIATSUIView
			status = HIATSUIViewSetAlignment(atsuiView, kATSUStartAlignment);
			require_noerr(status, ExitHandler);
			
			HIViewSetNeedsDisplay(atsuiView, true);
			break;
		case 'Cntr':
			// setting the required alignment and updating the HIATSUIView
			status = HIATSUIViewSetAlignment(atsuiView, kATSUCenterAlignment);
			require_noerr(status, ExitHandler);
			
			HIViewSetNeedsDisplay(atsuiView, true);
			break;
		case 'Rght':
			// setting the required alignment and updating the HIATSUIView
			status = HIATSUIViewSetAlignment(atsuiView, kATSUEndAlignment);
			require_noerr(status, ExitHandler);
			
			HIViewSetNeedsDisplay(atsuiView, true);
			break;
		case 'Ascr':
			// starting or stopping the Auto-scrolling of the HIATSUIView
			status = StartStopAutoScroll(atsuiView, (controlValue == 1));
			require_noerr(status, ExitHandler);
			
			HIViewSetNeedsDisplay(atsuiView, true);
			break;
		}

ExitHandler:
	return status;
	}   // Handle_FrameWrapCommandProcess

/*****************************************************
*
* MyEmptyActionProc(theControl, partCode) 
*
* Purpose:  This empty action proc is necessary to get a live slider but the actual handling is done
*				in the slider kEventControlValueFieldChanged handler such as:
*						- Handle_XLocationChanged
*						- Handle_JustificationChanged
*						- Handle_VariationValueChanged
*
* Inputs:   theControl          - ignored
*				partCode            - ignored
*
* Returns:  none
*/
static pascal void MyEmptyActionProc(ControlRef theControl, ControlPartCode partCode)
	{
	}   // MyEmptyActionProc

/*****************************************************
*
* Handle_XLocationChanged(inHandlerCallRef, inEvent, inUserData) 
*
* Purpose:  called when the user moves the xLocation slider indicator of the Scrolling Window
*
* Inputs:   inHandlerCallRef    - reference to the current handler call chain
*				inEvent             - the event
*           inUserData          - app-specified data you passed in the call to InstallEventHandler
*
* Returns:  OSStatus            - noErr indicates the event was handled
*                                 eventNotHandledErr indicates the event was not handled and the Toolbox should take over
*/
static pascal OSStatus Handle_XLocationChanged(EventHandlerCallRef inHandlerCallRef, EventRef inEvent, void *inUserData)
	{
	OSStatus status = eventNotHandledErr;
	HIViewRef atsuiView = (HIViewRef)inUserData;

	// getting the slider
	HIViewRef slider;	
	status = GetEventParameter(inEvent, kEventParamDirectObject, typeControlRef, NULL, sizeof(slider), NULL, &slider);
	require_noerr(status, ExitHandler);

	// setting the xLocation of the HIATSUIView to the slider control value (Fixed Format)
	status = HIATSUIViewSetPosition(atsuiView, IntToFixed(GetControl32BitValue(slider)), 0);
	require_noerr(status, ExitHandler);

	HIViewSetNeedsDisplay(atsuiView, true);

ExitHandler:

	// letting the HIToolBox handling the indicator movement
	if (status == noErr) status = eventNotHandledErr;
	return status;
	}   // Handle_XLocationChanged

/*****************************************************
*
* Handle_JustificationChanged(inHandlerCallRef, inEvent, inUserData) 
*
* Purpose:  called when the user moves the Justification slider indicator of the Scrolling Window
*
* Inputs:   inHandlerCallRef    - reference to the current handler call chain
*				inEvent             - the event
*           inUserData          - app-specified data you passed in the call to InstallEventHandler
*
* Returns:  OSStatus            - noErr indicates the event was handled
*                                 eventNotHandledErr indicates the event was not handled and the Toolbox should take over
*/
static pascal OSStatus Handle_JustificationChanged(EventHandlerCallRef inHandlerCallRef, EventRef inEvent, void *inUserData)
	{
	OSStatus status = eventNotHandledErr;
	HIViewRef atsuiView = (HIViewRef)inUserData;

	// getting the slider
	HIViewRef slider;	
	status = GetEventParameter(inEvent, kEventParamDirectObject, typeControlRef, NULL, sizeof(slider), NULL, &slider);
	require_noerr(status, ExitHandler);

	// setting the justification of the HIATSUIView to the slider control value (Fixed Format)
	status = HIATSUIViewSetJustification(atsuiView, (Fract)GetControl32BitValue(slider));
	require_noerr(status, ExitHandler);

	HIViewSetNeedsDisplay(atsuiView, true);

ExitHandler:

	// letting the HIToolBox handling the indicator movement
	if (status == noErr) status = eventNotHandledErr;
	return status;
	}   // Handle_JustificationChanged

/*****************************************************
*
* Create_CGImageFromResource(name, outCGImageRef) 
*
* Purpose:  called to create a CGImageRef from a PNG file
*
* Inputs:   name			  - name of the image file to use
*           outCGImageRef - returns the reference of the CGImage created
*
* Returns:  OSStatus      - error code (0 == no error) 
*/
static OSStatus Create_CGImageFromResource(CFStringRef name, CGImageRef * outCGImageRef)
	{
	OSStatus status = noErr;
	CFURLRef url = NULL;
	CGDataProviderRef provider = NULL;

	// getting the image file from our application's resources
	url = CFBundleCopyResourceURL(CFBundleGetMainBundle(), name, NULL, NULL);
	require_action(NULL != url, CantGetURL, status = fnfErr);
	
	// creating the Data Provider from that file
	provider = CGDataProviderCreateWithURL(url);
	require_action(NULL != provider, CantCreateProvider, status = fnfErr);

	// creating the CGImage from that Data Provider
	*outCGImageRef = CGImageCreateWithPNGDataProvider(provider, NULL, false, kCGRenderingIntentDefault);
	require_action(NULL != outCGImageRef, CantCreateImage, status = badImageErr);

CantCreateImage:
CantCreateProvider:
CantGetURL:

	// let's cleanup
	if (provider != NULL) CGDataProviderRelease(provider);
	if (url != NULL) CFRelease(url);

	return status;
	}   // Create_CGImageFromResource

/*****************************************************
*
* Install_ImageInBackground(theWindow, name, imageView) 
*
* Purpose:  called to install an image background HIImageView in the window
*
* Inputs:   theWindow     - WindowRef of the window
*				name			  - name of the image file to use
*           imageView     - returns the newly created HIImageView
*
* Returns:  OSErr			  - error code (0 == no error) 
*/
static OSStatus Install_ImageInBackground(WindowRef theWindow, CFStringRef name, HIViewRef * imageView)
	{
	OSStatus status = noErr;

	// creating the background image from our PNG file in the application's resources
	CGImageRef backgroundImage = NULL;
	status = Create_CGImageFromResource(name, &backgroundImage);
	require_noerr(status, CantGetImage);

	// creating a HImageView with that image
	status = HIImageViewCreate(backgroundImage, imageView);
	require_noerr(status, CantCreateImageView);

	HIViewRef contentView;
	status = HIViewFindByID(HIViewGetRoot(theWindow), kHIViewWindowContentID, &contentView);
	require_noerr(status, CantGetContentView);

	// adding that HIImageView to the content view of our window
	status = HIViewAddSubview(contentView, *imageView);
	require_noerr(status, CantAddSubview);

	// making sure this image is at the bottom so that it draws first
	status = HIViewSetZOrder(*imageView, kHIViewZOrderBelow, NULL);
	require_noerr(status, CantSetZOrder);

	// adjusting the bounds, scaling, and layout info
	HIRect bounds;
	status = HIViewGetBounds(contentView, &bounds);
	require_noerr(status, CantGetBounds);

	status = HIViewSetFrame(*imageView, &bounds);
	require_noerr(status, CantSetFrame);

	status = HIImageViewSetScaleToFit(*imageView, true);
	require_noerr(status, CantSetScale);

	HILayoutInfo layout = {
		kHILayoutInfoVersionZero,
		{ { NULL, kHILayoutBindTop }, { NULL, kHILayoutBindLeft }, { NULL, kHILayoutBindBottom }, { NULL, kHILayoutBindRight } },
		{ { NULL, 0.0 }, { NULL, 0.0 } },
		{ { NULL, kHILayoutPositionNone }, { NULL, kHILayoutPositionNone } }
	};
	status = HIViewSetLayoutInfo(*imageView, &layout);
	require_noerr(status, CantSetLayout);

	status = HIViewSetVisible(*imageView, true);
	require_noerr(status, CantSetVisible);

CantSetVisible:
CantSetLayout:
CantSetScale:
CantSetFrame:
CantGetBounds:
CantSetZOrder:
CantAddSubview:
CantGetContentView:
CantCreateImageView:
CantGetImage:

	// let's cleanup
	if (backgroundImage != NULL) CGImageRelease(backgroundImage);

	return status;
	}   // Install_ImageInBackground

/*****************************************************
*
* Set_FontAndSizeToFirstStyle(atsuiView, atsuFontID, atsuSize, atsuStyle) 
*
* Purpose:  setting the font and size of the first style of the layout
*
*				This is an ultra simplistic approach to font handling. We just set the font of the first style
*				If that's the only style for the layout then the whole text will change.
*				Else only the first style, which can be shared by many runs, will change. 
*
* Inputs:   atsuiView           - the HIATSUView to set
*				atsuFontID          - the font to set
*				atsuSize            - the size to set in Fixed format, 0 means no change
*				atsuStyle           - returns a pointer to the first style
*
* Returns:  OSStatus			     - error code (0 == no error) 
*/
static OSStatus Set_FontAndSizeToFirstStyle(HIViewRef atsuiView, ATSUFontID atsuFontID, Fixed atsuSize, ATSUStyle * atsuStyle)
	{
	OSStatus status = noErr;

	// getting our text layout
	ATSUTextLayout aTL = HIATSUIViewGetTextLayout(atsuiView);
	require(aTL != NULL, FontAndSizeExit);
	
	// getting the first style
	UniCharArrayOffset start;
	UniCharCount length;
	status = ATSUGetRunStyle(aTL, 0, atsuStyle, &start, &length);
	require_noerr(status, FontAndSizeExit);

	// setting the font and size of that style
	status = atsuSetFont(*atsuStyle, atsuFontID);
	require_noerr(status, FontAndSizeExit);

	if (atsuSize != 0)
		{
		status = atsuSetSize(*atsuStyle, atsuSize);
		require_noerr(status, FontAndSizeExit);
		}
	
	// and updating the HIATSUIView after re-breaking the lines
	status = HIATSUIViewBreakLines(atsuiView);
	require_noerr(status, FontAndSizeExit);
	
	HIViewSetNeedsDisplay(atsuiView, true);

FontAndSizeExit:

	return status;
	}   // Set_FontAndSizeToFirstStyle

/*****************************************************
*
* Populate_FeatureWindow(featureWindow, atsuFontID, atsuStyle) 
*
* Purpose:  populating the window with controls associated with each feature
*
* Inputs:   featureWindow       - the window to populate with the controls for each feature
*				atsuFontID          - the font to extract features from
*				atsuStyle           - the style to extract variation from
*
* Returns:  OSStatus			     - error code (0 == no error) 
*/
static OSStatus Populate_FeatureWindow(WindowRef featureWindow, ATSUFontID atsuFontID, ATSUStyle atsuStyle)
	{
	OSStatus status = noErr;
	
	// changing the title of the main window to insert the name of the chosen font
	WindowRef mainWindow = (WindowRef)GetWRefCon(featureWindow);
	require_action(mainWindow != NULL, FeatureExit, status = errInvalidWindowRef);

	CFStringRef currentTitle = NULL;
	CFStringRef nbString = NULL;
	CFStringRef fontName = NULL;
	CFMutableStringRef mutTitle = NULL;

	// getting the current title
	status = CopyWindowTitleAsCFString(mainWindow, &currentTitle);
	require_action(currentTitle != NULL, FeatureExit, status = errWindowPropertyNotFound);
	require_noerr(status, FeatureExit);
	
	// getting the number at the end of the title
	CFRange nbRange = CFStringFind(currentTitle, CFSTR("#"), kCFCompareBackwards);
	nbRange.length = CFStringGetLength(currentTitle) - nbRange.location;
	nbString = CFStringCreateWithSubstring(NULL, currentTitle, nbRange);
	require_action(nbString != NULL, FeatureExit, status = memFullErr);

	// getting the name of the font
	fontName = atsuGetFontNameCodeAsCFString(atsuFontID);
	if (fontName != NULL)
		{
		mutTitle = CFStringCreateMutableCopy(NULL, 0, CFSTR("Using Font Variants of "));
		CFStringAppend(mutTitle, fontName);
		CFStringAppend(mutTitle, CFSTR(" "));
		}
	else mutTitle = CFStringCreateMutableCopy(NULL, 0, CFSTR("Using Font Variants "));
	require_action(mutTitle != NULL, FeatureExit, status = memFullErr);

	// appending the number and setting the new title
	CFStringAppend(mutTitle, nbString);
	SetWindowTitleWithCFString(mainWindow, mutTitle);
	
	// first let's clean all previous controls
	// for convenience we put everything in a single user pane
	// the user pane will not be there yet when we populate for the first time
	// so we don't block if we can't find it.
	HIViewRef userPane = NULL;
	HIViewID userPaneID = { 'User', 100 };
	HIViewFindByID(HIViewGetRoot(featureWindow), userPaneID, &userPane);
	if (userPane != NULL) DisposeControl(userPane);
	
	// and we recreate a brand new clean user pane
	Rect bounds = { 0, 0, 4000, 4000 };
	status = CreateUserPaneControl(featureWindow, &bounds, kControlSupportsEmbedding, &userPane);
	require_noerr(status, FeatureExit);
	SetControlID(userPane, &userPaneID);
	
	// allocating selectors and defaults arrays
	SInt16 maxWidth = 100, y, maxHeight = 0;
	ATSUFontFeatureType * types = NULL;
	ATSUFontFeatureSelector * sels = NULL;
	Boolean * defs = NULL;
	ItemCount maxSels = 10;
	sels = (ATSUFontFeatureSelector *) malloc( maxSels * sizeof(ATSUFontFeatureSelector));
	defs = (Boolean *) malloc( maxSels * sizeof(Boolean));
	require(sels != NULL, FeatureExit);
	require(defs != NULL, FeatureExit);
	
	// how many features?
	ItemCount i, j, typeCount, selCount;
	status = ATSUCountFontFeatureTypes(atsuFontID, &typeCount);
	require_noerr(status, FeatureExit);

	if (typeCount > 0)
		{
		// allocating and getting the feature types
		types = (ATSUFontFeatureType *) malloc( typeCount * sizeof(ATSUFontFeatureType));
		require(types != NULL, FeatureExit);

		status = ATSUGetFontFeatureTypes(atsuFontID, typeCount, types, NULL);
		require_noerr(status, FeatureExit);
		
		for (i=0; i<typeCount; i++)
			{
			// get the feature name to set as title of the group control
			CFStringRef theSetName = atsuGetFontFeatureNameCodeAsCFString(atsuFontID, types[i], kATSUNoSelector);
			require(theSetName != NULL, FeatureExit);
			
			HIViewRef groupBox, currentParent;
			Rect groupBounds = { maxHeight, 0, maxHeight+1, 300 };
			status = CreateGroupBoxControl(NULL, &groupBounds, theSetName, true, &groupBox);
			CFRelease(theSetName);
			require_noerr(status, FeatureExit);
			maxHeight += 20;

			// how many selectors for that feature?
			status = ATSUCountFontFeatureSelectors(atsuFontID, types[i], &selCount);
			require_noerr(status, FeatureExit);
			
			// do we need to reallocated our arrays bigger?
			if (selCount > maxSels)
				{
				maxSels = selCount;
				sels = (ATSUFontFeatureSelector *) realloc( sels, maxSels * sizeof(ATSUFontFeatureSelector));
				defs = (Boolean *) realloc( defs, maxSels * sizeof(Boolean));
				require(sels != NULL, FeatureExit);
				require(defs != NULL, FeatureExit);
				}
			
			// getting the selectors, we also know if the selectors are exclusive (radio buttons)
			// or not (check boxes) and which are the default options.
			// Note: some fonts have bugs in their tables and offer exclusive selectors as non-exclusice and vice-versa.
			Boolean exclusive;
			status = ATSUGetFontFeatureSelectors(atsuFontID, types[i], selCount, sels, defs, NULL, &exclusive);
			require_noerr(status, FeatureExit);
			
			// to make the code easier, we create an embedding user pane for our check boxes, it is not necessary
			// but makes the code more symetric to balance the radio group when we have radios.
			Rect embdBounds = { 20, 0, 21, 1000 };
			if (exclusive)
				status = CreateRadioGroupControl(NULL, &embdBounds, &currentParent);
			else
				status = CreateUserPaneControl(NULL, &embdBounds, kControlSupportsEmbedding, &currentParent);
			require_noerr(status, FeatureExit);
			HIViewAddSubview(groupBox, currentParent);
			
			y = 0;
			for (j=0; j<selCount; j++)
				{
				// getting the selector name
				CFStringRef theCFName = atsuGetFontFeatureNameCodeAsCFString(atsuFontID, types[i], sels[j]);
				require(theCFName != NULL, FeatureExit);

				HIViewRef featureControl;
				Rect featureRect = { y, 0, y + 20, 1000 };

				// creating the correct control with that name
				if (exclusive)
					status = CreateRadioButtonControl(NULL, &featureRect, theCFName, 0, true, &featureControl);
				else
					status = CreateCheckBoxControl(NULL, &featureRect, theCFName, defs[j] ? 1 : 0, true, &featureControl);
				CFRelease(theCFName);
				require_noerr(status, FeatureExit);
				SetControlReference(featureControl, (SInt32)((types[i] << 16) | sels[j]));
				SetControlCommandID(featureControl, 'Feat');
				
				// managing dimensions
				SInt16 baseLine;
				Rect bestRect = {0, 0, 0, 0};
				GetBestControlRect(featureControl, &bestRect, &baseLine);
				if ((bestRect.right - bestRect.left) > maxWidth) maxWidth = bestRect.right - bestRect.left;
				
				HIViewAddSubview(currentParent, featureControl);
				if (exclusive) if (defs[j]) SetControl32BitValue(currentParent, j+1);

				maxHeight += 20;
				y += 20;
				}
			
			// managing dimensions
			HIRect frame;
			HIViewGetFrame(currentParent, &frame);
			frame.size.height = y+20;
			HIViewSetFrame(currentParent, &frame);
			HIViewGetFrame(groupBox, &frame);
			frame.size.height = y+26;
			HIViewSetFrame(groupBox, &frame);
			HIViewAddSubview(userPane, groupBox);
			maxHeight += 6;
			}
		}

	// and now, we do the same for the variations
	ItemCount varCount;
	status = ATSUCountFontVariations(atsuFontID, &varCount);
	require_noerr(status, FeatureExit);

	for (i=0; i<varCount; i++)
		{
		ATSUFontVariationAxis axis;
		ATSUFontVariationValue minV, maxV, defV, curV;

		// getting the variation
		status = ATSUGetIndFontVariation(atsuFontID, i, &axis, &minV, &maxV, &defV);
		require_noerr(status, FeatureExit);
		
		// getting the default value
		status = ATSUGetFontVariationValue(atsuStyle, axis, &curV);
		if (status != noErr) curV = defV;

		// getting the name of the variation
		CFStringRef theCFName = atsuGetFontVariationNameCodeAsCFString(atsuFontID, axis);
		require(theCFName != NULL, FeatureExit);
		
		// creating the slider for that variation, embedded in a group box to display the name of the variation
		HIViewRef groupBox, slider;
		Rect groupBounds = { maxHeight, 0, maxHeight+46, 1000 };
		status = CreateGroupBoxControl(NULL, &groupBounds, theCFName, true, &groupBox);
		CFRelease(theCFName);
		require_noerr(status, FeatureExit);
		maxHeight += 20;
		
		Rect embdBounds = { 20, 4, 40, maxWidth-4 };
		status = CreateSliderControl(NULL, &embdBounds, curV, minV, maxV, kControlSliderDoesNotPoint, 0, true, MyEmptyActionProc, &slider);
		require_noerr(status, FeatureExit);
	
		EventTypeSpec eventTypeCVFC = {kEventClassControl, kEventControlValueFieldChanged};
		status = InstallControlEventHandler(slider, Handle_VariationValueChanged, 1, &eventTypeCVFC, (void *)axis, NULL);
		require_noerr(status, FeatureExit);
		
		HIViewAddSubview(groupBox, slider);
		HIViewAddSubview(userPane, groupBox);
		HIViewSetVisible(groupBox, true);
		maxHeight += 26;
		}
	
	// and now, we make the drawer window a bit nicer, respecting the Aqua theme margins
	maxWidth += 8;
	Beautify_FeatureWindow(featureWindow, &maxWidth, &maxHeight);

	GetWindowBounds(featureWindow, kWindowContentRgn, &bounds);
	SizeWindow(featureWindow, maxWidth+16, bounds.bottom - bounds.top, true);

FeatureExit:

	// let's cleanup
	if (types != NULL) free(types);
	if (sels != NULL) free(sels);
	if (defs != NULL) free(defs);
	if (currentTitle != NULL) CFRelease(currentTitle);
	if (nbString != NULL) CFRelease(nbString);
	if (fontName != NULL) CFRelease(fontName);
	if (mutTitle != NULL) CFRelease(mutTitle);

	return status;
	}   // Populate_FeatureWindow

/*****************************************************
*
* Beautify_FeatureWindow(featureWindow, maxWidth, maxHeight) 
*
* Purpose:  Makes the feature window look better by rearranging the contents
*
* Inputs:   featureWindow       - the window to populate with the controls for each feature
*				maxWidth            - the computed width
*				maxHeight           - the computed height
*
* Returns:  OSStatus			     - error code (0 == no error) 
*/
static OSStatus Beautify_FeatureWindow(WindowRef featureWindow, SInt16 * maxWidth, SInt16 * maxHeight)
	{
	OSStatus status = noErr;
	HIRect frame;
	HIViewRef subView, userPane = NULL;
	
	// getting the user pane
	HIViewID userPaneID = { 'User', 100 };
	status = HIViewFindByID(HIViewGetRoot(featureWindow), userPaneID, &userPane);
	require_noerr(status, BeautifyExit);
	
	// Sanity check
	if (*maxWidth < 200) *maxWidth = 200;

	// making all subviews the same width
	subView = HIViewGetFirstSubview(userPane);
	while (subView != NULL)
		{
		HIViewGetFrame(subView, &frame);
		frame.size.width = *maxWidth;
		HIViewSetFrame(subView, &frame);
		subView = HIViewGetNextView(subView);
		}
	
	// if the height is too much, let's split in 2 columns
	if ((*maxHeight) > 500)
		{
		SInt16 y1 = 0, y2 = 0;
		subView = HIViewGetLastSubview(userPane);
		while (subView != NULL)
			{
			HIViewGetFrame(subView, &frame);
			frame.origin.x = (y1 <= y2) ? 0 : (*maxWidth + 8);
			frame.origin.y = (y1 <= y2) ? y1 : y2;
			if (y1 <= y2)
				y1 += frame.size.height;
			else
				y2 += frame.size.height;
			HIViewSetFrame(subView, &frame);
			subView = HIViewGetPreviousView(subView);
			}
		*maxHeight = (y1 > y2) ? y1 : y2;
		*maxWidth = 2 * (*maxWidth) + 8;
		}
	
	*maxWidth += 8;
	HIViewMoveBy(userPane, 16, 0);

BeautifyExit:

	return status;
	}   // Beautify_FeatureWindow

/*****************************************************
*
* atsuGetFontNameCodeAsCFString(inFontID, inType, inSelector) 
*
* Purpose:  convenience utility function returning the name of a font
*
* Inputs:   inFontID            - the font
*
* Returns:  CFStringRef         - the name of the font feature as a CFString, or NULL if something went wrong
*/
static CFStringRef atsuGetFontNameCodeAsCFString(ATSUFontID inFontID)
	{
	OSStatus status = noErr;
	CFStringRef theCFName = NULL;
	char * cStrfontName = NULL;
	
	// do we find the name in any table?
	ByteCount actualLength;
	status = ATSUFindFontName(inFontID, kFontFamilyName, kFontNoPlatform, kFontNoScript, kFontNoLanguage, 0, NULL, &actualLength, NULL);
	require_noerr(status, FontNameExit);

	actualLength += 1;
	cStrfontName = malloc(actualLength);
	require(cStrfontName != NULL, FontNameExit);

	// trying to get the Unicode name first
	status = ATSUFindFontName(inFontID, kFontFamilyName, kFontUnicodePlatform, kFontNoScript, kFontNoLanguage, actualLength, cStrfontName, NULL, NULL);
	if (status == noErr)
		theCFName = CFStringCreateWithCharacters(NULL, (UniChar *)cStrfontName, (actualLength-1) >> 1);
	else
		{
		// else let's get the first name we can get
		status = ATSUFindFontName(inFontID, kFontFamilyName, kFontNoPlatform, kFontNoScript, kFontNoLanguage, actualLength, cStrfontName, NULL, NULL);
		require_noerr(status, FontNameExit);
		cStrfontName[actualLength-1] = 0;
		theCFName = CFStringCreateWithCString(NULL, cStrfontName, kCFStringEncodingMacRoman);
		}
	require(theCFName != NULL, FontNameExit);

FontNameExit:
	
	if (cStrfontName != NULL) free(cStrfontName);

	return theCFName;
	}   // atsuGetFontNameCodeAsCFString

/*****************************************************
*
* atsuGetFontFeatureNameCodeAsCFString(inFontID, inType, inSelector) 
*
* Purpose:  convenience utility function returning the name of a font feature
*
* Inputs:   inFontID            - the font
*				inType              - the font feature
*           inSelector          - the font feature selector
*
* Returns:  CFStringRef         - the name of the font feature as a CFString, or NULL if something went wrong
*/
static CFStringRef atsuGetFontFeatureNameCodeAsCFString(ATSUFontID inFontID, ATSUFontFeatureType inType, ATSUFontFeatureSelector inSelector)
	{
	OSStatus status = noErr;
	CFStringRef theCFName = NULL;
	char * cStrfontNameCode = NULL;

	// getting the code
	FontNameCode fontNameCode;
	status = ATSUGetFontFeatureNameCode(inFontID, inType, inSelector, &fontNameCode);
	require_noerr(status, FeatureNameExit);
	
	// calling ATSUFindFontName a first time to get the length
	ByteCount actualLength;
	status = ATSUFindFontName(inFontID, fontNameCode, kFontNoPlatform, kFontNoScript, kFontNoLanguage, 0, NULL, &actualLength, NULL);
	require_noerr(status, FeatureNameExit);

	// allocating the buffer
	actualLength += 1;
	cStrfontNameCode = malloc(actualLength);
	require(cStrfontNameCode != NULL, FeatureNameExit);

	// calling ATSUFindFontName a second time to get the characters
	status = ATSUFindFontName(inFontID, fontNameCode, kFontNoPlatform, kFontNoScript, kFontNoLanguage, actualLength, cStrfontNameCode, NULL, NULL);
	require_noerr(status, FeatureNameExit);
	cStrfontNameCode[actualLength-1] = 0;
	
	// creating the CFString
	theCFName = CFStringCreateWithCString(NULL, cStrfontNameCode, kCFStringEncodingMacRoman);
	require(theCFName != NULL, FeatureNameExit);

FeatureNameExit:
	
	if (cStrfontNameCode != NULL) free(cStrfontNameCode);

	return theCFName;
	}   // atsuGetFontFeatureNameCodeAsCFString

/*****************************************************
*
* atsuGetFontVariationNameCodeAsCFString(inFontID, inAxis) 
*
* Purpose:  convenience utility function returning the name of a font feature
*
* Inputs:   inFontID            - the font
*				inAxis              - the variation
*
* Returns:  CFStringRef         - the name of the font variation as a CFString, or NULL if something went wrong
*/
static CFStringRef atsuGetFontVariationNameCodeAsCFString(ATSUFontID inFontID, ATSUFontVariationAxis inAxis)
	{
	OSStatus status = noErr;
	CFStringRef theCFName = NULL;
	char * cStrfontNameCode = NULL;

	// getting the code
	FontNameCode fontNameCode;
	status = ATSUGetFontVariationNameCode(inFontID, inAxis, &fontNameCode);
	require_noerr(status, FeatureNameExit);
	
	// calling ATSUFindFontName a first time to get the length
	ByteCount actualLength;
	status = ATSUFindFontName(inFontID, fontNameCode, kFontNoPlatform, kFontNoScript, kFontNoLanguage, 0, NULL, &actualLength, NULL);
	require_noerr(status, FeatureNameExit);

	// allocating the buffer
	actualLength += 1;
	cStrfontNameCode = malloc(actualLength);
	require(cStrfontNameCode != NULL, FeatureNameExit);

	// calling ATSUFindFontName a second time to get the characters
	status = ATSUFindFontName(inFontID, fontNameCode, kFontNoPlatform, kFontNoScript, kFontNoLanguage, actualLength, cStrfontNameCode, NULL, NULL);
	require_noerr(status, FeatureNameExit);
	cStrfontNameCode[actualLength-1] = 0;
	
	// creating the CFString
	theCFName = CFStringCreateWithCString(NULL, cStrfontNameCode, kCFStringEncodingMacRoman);
	require(theCFName != NULL, FeatureNameExit);

FeatureNameExit:
	
	if (cStrfontNameCode != NULL) free(cStrfontNameCode);

	return theCFName;
	}   // atsuGetFontVariationNameCodeAsCFString

/*****************************************************
*
* Handle_ScrollingWindowIsClosing(inHandlerCallRef, inEvent, inUserData) 
*
* Purpose:  called as notification that a Scrolling window is being destroyed, we need to remove the Carbon Event Timer if any
*
* Inputs:   inHandlerCallRef    - reference to the current handler call chain
*				inEvent             - the event
*           inUserData          - app-specified data you passed in the call to InstallEventHandler
*
* Returns:  OSStatus            - noErr indicates the event was handled
*                                 eventNotHandledErr indicates the event was not handled and the Toolbox should take over
*/
static pascal OSStatus Handle_ScrollingWindowIsClosing(EventHandlerCallRef inHandlerCallRef, EventRef inEvent, void *inUserData)
	{
	WindowRef aWindowRef = (WindowRef)inUserData;

	EventLoopTimerRef theTimer = (EventLoopTimerRef)GetWRefCon(aWindowRef);
	if (theTimer != NULL)
		{
		RemoveEventLoopTimer(theTimer);
	
		// since the window is closing, the following is probably unnecessary
		// but on the off-chance that this handler is called twice...
		SetWRefCon(aWindowRef, 0);
		}
	
	return eventNotHandledErr;
	}   // Handle_ScrollingWindowIsClosing

/*****************************************************
*
* Handle_FeatureCommandProcess(inHandlerCallRef, inEvent, inUserData) 
*
* Purpose:  called to process commands from the controls in the Feature Window drawer
*
* Inputs:   inHandlerCallRef    - reference to the current handler call chain
*				inEvent             - the event
*           inUserData          - app-specified data you passed in the call to InstallEventHandler
*
* Returns:  OSStatus            - noErr indicates the event was handled
*                                 eventNotHandledErr indicates the event was not handled and the Toolbox should take over
*/
static pascal OSStatus Handle_FeatureCommandProcess(EventHandlerCallRef inHandlerCallRef, EventRef inEvent, void *inUserData)
	{
	OSStatus status;
	HICommandExtended aCommand;
	
	// getting the command
	status = GetEventParameter(inEvent, kEventParamDirectObject, typeHICommand, NULL, sizeof(HICommandExtended), NULL, &aCommand);
	require_noerr(status, ExitHandler);
	
	// this handler only deals with the check or radio buttons of the feature window
	status = eventNotHandledErr;
	if (!(aCommand.attributes & kHICommandFromControl)) goto ExitHandler;
	require(aCommand.commandID == 'Feat', ExitHandler);
	
	// getting the main window
	HIViewRef theControl = aCommand.source.control;
	WindowRef theWindow = (WindowRef)GetWRefCon(GetControlOwner(theControl));
	require(theWindow != NULL, ExitHandler);

	// getting the HIATSUIView
	HIViewRef atsuiView;
	status = HIViewFindByID(HIViewGetRoot(theWindow), kAtsuiID, &atsuiView);
	require_noerr(status, ExitHandler);
	require(atsuiView != NULL, ExitHandler);

	// getting the text layout of that HIATSUIView
	ATSUTextLayout aTL = HIATSUIViewGetTextLayout(atsuiView);
	require(aTL != NULL, ExitHandler);
	
	// getting the first style of that text layout
	UniCharArrayOffset start;
	UniCharCount length;
	ATSUStyle atsuStyle;
	status = ATSUGetRunStyle(aTL, 0, &atsuStyle, &start, &length);
	require_noerr(status, ExitHandler);
	
	// which feature, which selector, on or off?
	ATSUFontFeatureType type = GetControlReference(theControl) >> 16;
	ATSUFontFeatureSelector sel = GetControlReference(theControl) & 0x0FFFF;
	SInt32 value = GetControl32BitValue(theControl);
	
	// exclusive?
	ControlKind kind;
	GetControlKind(theControl, &kind);
	Boolean exclusive = (kind.kind == kControlKindRadioButton);
	
	if (!exclusive)
		{
		// setting or clearing when non-exclusive (check box)
		if (value == 1)
			status = ATSUSetFontFeatures(atsuStyle, 1, &type, &sel);
		else
			status = ATSUClearFontFeatures(atsuStyle, 1, &type, &sel);
		}
	else
		{
		// the feature is exclusive, we only need to set it
		status = ATSUSetFontFeatures(atsuStyle, 1, &type, &sel);
		}
	require_noerr(status, ExitHandler);

	// updating the HIATSUIView
	status = HIATSUIViewBreakLines(atsuiView);
	require_noerr(status, ExitHandler);
	
	HIViewSetNeedsDisplay(atsuiView, true);

ExitHandler:

	return status;
	}   // Handle_FeatureCommandProcess

/*****************************************************
*
* Handle_VariationValueChanged(inHandlerCallRef, inEvent, inUserData) 
*
* Purpose:  called when the user moves the variation slider indicator (if any) of the Feature Window
*
* Inputs:   inHandlerCallRef    - reference to the current handler call chain
*				inEvent             - the event
*           inUserData          - app-specified data you passed in the call to InstallEventHandler
*
* Returns:  OSStatus            - noErr indicates the event was handled
*                                 eventNotHandledErr indicates the event was not handled and the Toolbox should take over
*/
static pascal OSStatus Handle_VariationValueChanged(EventHandlerCallRef inHandlerCallRef, EventRef inEvent, void *inUserData)
	{
	OSStatus status = eventNotHandledErr;
	ATSUFontVariationAxis axis = (ATSUFontVariationAxis)inUserData;

	// getting the slider
	HIViewRef slider;
	status = GetEventParameter(inEvent, kEventParamDirectObject, typeControlRef, NULL, sizeof(slider), NULL, &slider);
	require_noerr(status, ExitHandler);

	// getting the main window
	WindowRef theWindow = (WindowRef)GetWRefCon(GetControlOwner(slider));
	require(theWindow != NULL, ExitHandler);

	// getting the HIATSUIView
	HIViewRef atsuiView;
	status = HIViewFindByID(HIViewGetRoot(theWindow), kAtsuiID, &atsuiView);
	require_noerr(status, ExitHandler);
	require(atsuiView != NULL, ExitHandler);

	// getting the text layout of that HIATSUIView
	ATSUTextLayout aTL = HIATSUIViewGetTextLayout(atsuiView);
	require(aTL != NULL, ExitHandler);
	
	// getting the first style of that text layout
	UniCharArrayOffset start;
	UniCharCount length;
	ATSUStyle atsuStyle;
	status = ATSUGetRunStyle(aTL, 0, &atsuStyle, &start, &length);
	require_noerr(status, ExitHandler);
	
	// setting the variation
	Fixed value = (Fixed)GetControl32BitValue(slider);
	status = ATSUSetVariations(atsuStyle, 1, &axis, &value);

	// updating the HIATSUIView
	status = HIATSUIViewBreakLines(atsuiView);
	require_noerr(status, ExitHandler);

	HIViewSetNeedsDisplay(atsuiView, true);

ExitHandler:

	// letting the HIToolBox handling the indicator movement
	if (status == noErr) status = eventNotHandledErr;
	return status;
	}   // Handle_VariationValueChanged

/*****************************************************
*
* MyNavEventProc(callBackSelector, callBackParms, callBackUD) 
*
* Purpose:  This empty NavEventProc is necessary to get the default behaviors in the Navigation Services dialog
*				such as handling drag-and-drop locations and files
*
* Inputs:   callBackSelector    - ignored
*				callBackParms       - ignored
*           callBackUD          - ignored
*
* Returns:  none
*/
static pascal void MyNavEventProc(NavEventCallbackMessage callBackSelector, NavCBRecPtr callBackParms, void *callBackUD)
	{
	}   // MyNavEventProc

/*****************************************************
*
* Handle_NavFilter(theItem, info, callBackUD, filterMode) 
*
* Purpose:  This filter proc will retain only TEXT, utxt, and txtn files based on the Info.plist content
*
* Inputs:   theItem             - A pointer to an Apple event descriptor structure (AEDesc)
*				info                - A pointer to a NavFileOrFolderInfo structure
*           callBackUD          - A pointer to a value set by your application when it calls a Navigation Services dialog creation function
*				filterMode          - A value representing which list of objects is currently being filtered
*
* Returns:  A Boolean value.
*				If your application returns true, Navigation Services displays the object.
*				If your application returns false, Navigation Services does not display the object.
*/
static pascal Boolean Handle_NavFilter(AEDesc *theItem, void *info, void *callBackUD, NavFilterModes filterMode)
	{
	LSItemInfoRecord lsInfoRec;
	FSRef fsRef;
	OSStatus status;
	Boolean canViewItem = false;
	
	if (theItem->descriptorType == typeFSRef)
		{
		status = AEGetDescData(theItem, &fsRef, sizeof(fsRef));
		require_noerr(status, CantGetFSRef);
		
		//	Ask LaunchServices for information about the item
		status = LSCopyItemInfoForRef(&fsRef, kLSRequestAllInfo, &lsInfoRec);
		require((noErr == status) || (kLSApplicationNotFoundErr == status), LaunchServicesError);
		
		if (0 != (lsInfoRec.flags & kLSItemInfoIsContainer))
			canViewItem	= true;
		else
			status = LSCanRefAcceptItem(&fsRef, &gApplicationBundleFSRef, kLSRolesViewer, kLSAcceptDefault, &canViewItem);		
		}
	
LaunchServicesError:
CantGetFSRef:

	return(canViewItem);
	}   // Handle_NavFilter

/*****************************************************
*
* ChooseOneFile(theFSRef) 
*
* Purpose:  called to choose a text file using Navigation Services
*
* Output:   theFSRef            - reference to the chosen file
*
* Returns:  OSStatus            - error code (0 == no error) 
*/
static OSStatus ChooseOneFile(FSRef * theFSRef)
	{
	OSStatus status = noErr;
	
	// setting the default options
	NavDialogCreationOptions options;
	status = NavGetDefaultDialogCreationOptions(&options);
	require_noerr(status, ExitChoose);
	options.optionFlags &= ~kNavAllowMultipleFiles;
	
	// creating the dialog
	NavDialogRef theDialog;
	status = NavCreateChooseFileDialog(&options, NULL, MyNavEventProc, NULL, Handle_NavFilter, NULL, &theDialog);
	require_noerr(status, ExitChoose);

	// accessing our bundle to get the Info.plist settings
	ProcessSerialNumber psn = {0, kCurrentProcess};
	status = GetProcessBundleLocation(&psn, &gApplicationBundleFSRef);
	require_noerr(status, ExitChoose);

	// running the dialog
	status = NavDialogRun(theDialog);
	require_noerr(status, ExitChoose);
	
	// getting the User's reply
	NavReplyRecord theReply;
	status = NavDialogGetReply(theDialog, &theReply);
	if (status == userCanceledErr) goto ExitChoose;
	require_noerr(status, ExitChoose);
	
	// getting the chosen file
	status = -1;
	if (theReply.validRecord)
		if (NavDialogGetUserAction(theDialog) == kNavUserActionChoose)
			status = AEGetNthPtr(&theReply.selection, 1, typeFSRef, NULL, NULL, theFSRef, sizeof(FSRef), NULL);
	require_noerr(status, ExitChoose);
	
ExitChoose:

	return status;
	}   // ChooseOneFile

/*****************************************************
*
* GetTextFromFile(theFSRef) 
*
* Purpose:  called to extract the text from a file and return it as a CFString
*				
*				Note: A lot of unsupported assumptions are used in this code
*						- that a 'TEXT' file is encoded in Mac Roman
*						- that a 'utxt' file will begin with a 2-byte BOM
*						- that a 'txtn' file contains a 'utxt' at offset 4
*					This was achieved by examining file contents and is just provided here
*					to show how a Text file can be opened and displayed. This code is not
*					an endorsement of those undocumented file formats.
*
* Output:   theFSRef            - reference to the chosen file
*
* Returns:  OSStatus            - error code (0 == no error) 
*/
static CFStringRef GetTextFromFile(FSRef * theFSRef)
	{
	OSStatus status = noErr;
	CFStringRef theText = NULL;
	SInt16 refNum = 0;
	char * buffer = NULL;

	// getting the type of the file
	FSCatalogInfo catalogInfo;
	HFSUniStr255 theName;
	status = FSGetCatalogInfo(theFSRef, kFSCatInfoFinderInfo, &catalogInfo, &theName, NULL, NULL);
	require_noerr(status, ExitGetText);
	FInfo * fInfo = (FInfo *)catalogInfo.finderInfo;
	int kind = 0;
	
	// let's check first if we have a type
	switch (fInfo->fdType)
		{
		case 'TEXT': kind = 1; break;
		case 'utxt': kind = 2; break;
		case 'txtn': kind = 3; break;
		}
	// if not, let's check if we have an extension
	if (kind == 0)
		{
		UniCharCount there;
		status = LSGetExtensionInfo(theName.length, theName.unicode, &there);
		if ((status == noErr) && (there != kLSInvalidExtensionIndex))
			{
			if ((theName.unicode[there] == 't') && (theName.unicode[there+1] == 'x') && (theName.unicode[there+2] == 't') && (theName.length == there+3)) kind = 1;
			else if ((theName.unicode[there] == 't') && (theName.unicode[there+1] == 'e') && (theName.unicode[there+2] == 'x') && (theName.unicode[there+3] == 't') && (theName.length == there+4)) kind = 1;
			else if ((theName.unicode[there] == 'T') && (theName.unicode[there+1] == 'E') && (theName.unicode[there+2] == 'X') && (theName.unicode[there+3] == 'T') && (theName.length == there+4)) kind = 1;
			else if ((theName.unicode[there] == 'T') && (theName.unicode[there+1] == 'X') && (theName.unicode[there+2] == 'T') && (theName.length == there+3)) kind = 1;
			else if ((theName.unicode[there] == 'u') && (theName.unicode[there+1] == 't') && (theName.unicode[there+2] == 'x') && (theName.unicode[there+3] == 't') && (theName.length == there+4)) kind = 2;
			else if ((theName.unicode[there] == 't') && (theName.unicode[there+1] == 'x') && (theName.unicode[there+2] == 't') && (theName.unicode[there+3] == 'n') && (theName.length == there+4)) kind = 3;
			}
		}
	require(kind != 0, ExitGetText);
	
	// opening the file
	status = FSOpenFork(theFSRef, 0, NULL, fsRdPerm, &refNum);
	require_noerr(status, ExitGetText);
	
	SInt64 forkSize;
	status = FSGetForkSize(refNum, &forkSize);
	require_noerr(status, ExitGetText);

	// allocating the buffer and reading the file
	// we truncate the operation if the file is too big
	UInt32 theLength = (UInt32)forkSize;
	if (theLength > 8000) theLength = 8000;
	buffer = malloc(theLength+1);
	require(buffer != NULL, ExitGetText);

	status = FSReadFork(refNum, fsFromStart+noCacheMask, 0, theLength, buffer, NULL);
	require_noerr(status, ExitGetText);
	
	switch (kind)
		{
		case 1:
			buffer[theLength] = 0;
			theText = CFStringCreateWithCString(NULL, buffer, kCFStringEncodingMacRoman);
			break;
		case 2:
			if (*((UniChar *)buffer) == kUnicodeSwappedByteOrderMark)
				{
				// need to reverse the lo-byte / hi-bte
				}
			theText = CFStringCreateWithCharacters(NULL, (UniChar *)(buffer+2), (theLength-2) >> 1);
			break;
		case 3:
			if (*((OSType *)(buffer+4)) == 'utxt')
				{
				theLength = *((UInt32 *)(buffer+8));
				theText = CFStringCreateWithCharacters(NULL, (UniChar *)(buffer+12), theLength >> 1);
				}
			break;
		}
	
ExitGetText:

	if (buffer != NULL) free(buffer);
	if (refNum != 0) FSCloseFork(refNum);

	return theText;
	}   // GetTextFromFile

/*****************************************************
*
* Handle_UserPaneDraw(inHandlerCallRef, inEvent, inUserData) 
*
* Purpose:  called when the user modifies the controls of the ATSU Text Box Window
*
* Inputs:   inHandlerCallRef    - reference to the current handler call chain
*				inEvent             - the event
*           inUserData          - app-specified data you passed in the call to InstallEventHandler
*
* Returns:  OSStatus            - noErr indicates the event was handled
*                                 eventNotHandledErr indicates the event was not handled and the Toolbox should take over
*/
static pascal OSStatus Handle_UserPaneDraw(EventHandlerCallRef inHandlerCallRef, EventRef inEvent, void *inUserData)
	{
	OSStatus status;
	HIViewRef userPane = (HIViewRef)inUserData;

	// getting the CGContext
	CGContextRef context;
	status = GetEventParameter(inEvent, kEventParamCGContextRef, typeCGContextRef, NULL, sizeof(context), NULL, &context);
	require_noerr(status, UserPaneDrawExit);

	Rect bounds;
	GetControlBounds(userPane, &bounds);
	// we need to work in our local view coordinates
	OffsetRect(&bounds, -bounds.left, -bounds.top);

	// we need to reverse the y-axis in order for the text to not be upside-down
	// and thus we also need to translate by the height so that we draw in the visible bounds
	CGContextSaveGState(context);
	CGContextScaleCTM(context, 1, -1);
	CGContextTranslateCTM(context, 0, -bounds.bottom);

	// getting our default font ID
	CGContextSetRGBFillColor(context, 0.2, 0.2, 0.9, 0.8);
	ATSUFontID atsuFontID;
	char fontName[] = "Geneva";
	status = ATSUFindFontFromName(fontName, strlen(fontName), kFontFamilyName, kFontMacintoshPlatform, kFontRomanScript, kFontNoLanguageCode, &atsuFontID);
	verify_noerr_string(status, "ATSUFindFontFromName failed.");

	HIViewRef aView, windRoot = HIViewGetRoot(GetControlOwner(userPane));

	// getting our alignment, orientation, and justification values from the controls
	short alignment = 0, orientation = 0;
	Fract justification = 0;
	
	HIViewID ida = { 'Alig', 100 };
	HIViewFindByID(windRoot, ida, &aView);
	short alignments[] = { teFlushLeft, teCenter, teFlushRight };
	if (aView != NULL) alignment = alignments[GetControl32BitValue(aView) - 1];

	HIViewID ido = { 'Ornt', 100 };
	HIViewFindByID(windRoot, ido, &aView);
	if (aView != NULL) orientation = GetControl32BitValue(aView) - 1; // katsuHorizontalText is 0, katsuVerticalText is 1

	HIViewID idj = { 'Just', 100 };
	HIViewFindByID(windRoot, idj, &aView);
	if (aView != NULL) justification = (Fract)GetControl32BitValue(aView);

	// getting our text from our resources and drawing it with all our parameters
	CFStringRef demoText4 = CFCopyLocalizedString( CFSTR("demoText4"), CFSTR("") );
	atsuCompositedTextBox(CFStringGetCharactersPtr(demoText4), CFStringGetLength(demoText4), &bounds, alignment, justification, orientation, true, context, atsuFontID, IntToFixed(16));
	CFRelease(demoText4);

	// restoring our context the way we found it when we started
	CGContextRestoreGState(context);

UserPaneDrawExit:
	if (status == noErr) status = eventNotHandledErr;
	return status;
	}   // Handle_UserPaneDraw

/*****************************************************
*
* Handle_TextBoxParameterChanged(inHandlerCallRef, inEvent, inUserData) 
*
* Purpose:  called when the user modifies the controls of the ATSU Text Box Window
*
* Inputs:   inHandlerCallRef    - reference to the current handler call chain
*				inEvent             - the event
*           inUserData          - app-specified data you passed in the call to InstallEventHandler
*
* Returns:  OSStatus            - noErr indicates the event was handled
*                                 eventNotHandledErr indicates the event was not handled and the Toolbox should take over
*/
static pascal OSStatus Handle_TextBoxParameterChanged(EventHandlerCallRef inHandlerCallRef, EventRef inEvent, void *inUserData)
	{
	// for any change in any control, we just ask for a subsequent refresh, the actual drawing will be done in Handle_UserPaneDraw
	HIViewSetNeedsDisplay((HIViewRef)inUserData, true);

	return eventNotHandledErr;
	}   // Handle_TextBoxParameterChanged

/*****************************************************
*
* Handle_ClipboardWindowIsClosing(inHandlerCallRef, inEvent, inUserData) 
*
* Purpose:  called as notification that the user wants to close the "Show Clipboard" window. Let's just hide it instead.
*
* Inputs:   inHandlerCallRef    - reference to the current handler call chain
*				inEvent             - the event
*           inUserData          - app-specified data you passed in the call to InstallEventHandler
*
* Returns:  OSStatus            - noErr indicates the event was handled
*                                 eventNotHandledErr indicates the event was not handled and the Toolbox should take over
*/
static pascal OSStatus Handle_ClipboardWindowIsClosing(EventHandlerCallRef inHandlerCallRef, EventRef inEvent, void *inUserData)
	{
	HideWindow((WindowRef)inUserData);
	
	return noErr;
	}   // Handle_ClipboardWindowIsClosing

/*****************************************************
*
* Update_ClipboardWindow() 
*
* Purpose:  called to update the "Show Clipboard" window when the clipboard content has changed.
*
* Inputs:   none
*
* Returns:  OSErr			  - error code (0 == no error) 
*/
static OSStatus Update_ClipboardWindow(void)
	{
	OSStatus status = noErr;
	CFStringRef textString = NULL;
	void * styleBuffer = NULL;
	UniCharCount * runLengths = NULL;
	ATSUStyle * styles = NULL;

	// getting the HIATSUIView of the "Show Clipboard" window
	HIViewRef atsuiView;
	status = HIViewFindByID(HIViewGetRoot(gClipboardWindow), kAtsuiID, &atsuiView);
	require_noerr(status, UpdateClipboardExit);

	// Making sure we are synchronized
	PasteboardSynchronize(gClipboard);
	
	// Extracting the information from the pasteboard to get our text and styles. The styles are flattened in a block of bytes.
	ByteCount styleBufferSize;
	status = Extract_TextAndStyleFromPasteboard(&textString, &styleBuffer, &styleBufferSize);
	require_noerr(status, UpdateClipboardExit);
	require(textString != NULL, UpdateClipboardExit);
	
	// Extracting the runs and styles from the flattened block of bytes
	ItemCount numberOfRuns;
	status = Extract_RunLengthsAndStylesFromStyleBuffer(styleBuffer, styleBufferSize, &numberOfRuns, &runLengths, &styles);
	require_noerr(status, UpdateClipboardExit);

	// setting oiur HIATSUIView with those text and styles
	status = HIATSUIViewSetText(atsuiView, textString, numberOfRuns, runLengths, styles);
	require_noerr(status, CantSetText);

	// updating and signaling a change of content to our parent
	HIViewSetNeedsDisplay(atsuiView, true);
	Signal_ScrollViewContentChanged(atsuiView);

	// our content is now up-to-date so let's remember
	gClipboardChanged = false;
	
CantSetText:
UpdateClipboardExit:

	// let's cleanup
	if (textString != NULL) CFRelease(textString);
	if (styleBuffer != NULL) free(styleBuffer);
	if (runLengths != NULL) free(runLengths);
	if (styles != NULL) free(styles);

	return status;
	}   // Update_ClipboardWindow

/*****************************************************
*
* Extract_TextAndStyleFromPasteboard(oTextString, oStyleBuffer, oStyleBufferSize) 
*
* Purpose:  Extracts the text ('utxt') and the style ('ustl') from the Pasteboard
*
* Inputs:   oTextString         - the text to return as CFStringRef
*				oStyleBuffer        - the style to return as a memory block
*           oStyleBufferSize    - returning the size of that memory block
*
* Returns:  OSErr			  - error code (0 == no error) 
*/
static OSStatus Extract_TextAndStyleFromPasteboard(CFStringRef * oTextString, void ** oStyleBuffer, ByteCount * oStyleBufferSize)
	{
	OSStatus status = noErr;

	require_action(oTextString != NULL, ExitExtractTextAndStyle, status = paramErr);
	require_action(oStyleBuffer != NULL, ExitExtractTextAndStyle, status = paramErr);
	require_action(oStyleBufferSize != NULL, ExitExtractTextAndStyle, status = paramErr);
	
	CFStringRef textString = NULL;
	void * styleBuffer = NULL;
	ByteCount styleBufferSize = 0;

	// how many items in the pasteboard?
	ItemCount i, itemCount;
	status = PasteboardGetItemCount(gClipboard, &itemCount);
	require_noerr(status, CantGetPasteboardItemCount);

	// as long as we didn't get our text and style, let's loop on the items. We stop as soon as we get them
	for (i = 1; (i <= itemCount) && ( (textString == NULL) || (styleBuffer == NULL) ); i++)
		{
		PasteboardItemID itemID;
		CFArrayRef flavorTypeArray = NULL;
		CFIndex j, flavorCount = 0;

		status = PasteboardGetItemIdentifier(gClipboard, i, &itemID);
		require_noerr(status, CantGetPasteboardItemIdentifier);

		// how many flavors in this item?
		status = PasteboardCopyItemFlavors(gClipboard, itemID, &flavorTypeArray);
		require_noerr(status, CantCopyPasteboardItemFlavors);

		if (flavorTypeArray != NULL)
			flavorCount = CFArrayGetCount(flavorTypeArray);

		// as long as we didn't get our text and style, let's loop on the flavors. We stop as soon as we get them
		for(j = 0; (j < flavorCount) && ( (textString == NULL) || (styleBuffer == NULL) ); j++)
			{
			CFDataRef flavorData;
			CFStringRef flavorType = (CFStringRef)CFArrayGetValueAtIndex(flavorTypeArray, j);
			if (flavorType != NULL)
				if (UTTypeConformsTo(flavorType, CFSTR("public.utf16-plain-text"))) // this is 'utxt'
					{
					if (PasteboardCopyItemFlavorData(gClipboard, itemID, flavorType, &flavorData) == noErr)
						{
						CFIndex flavorDataSize = CFDataGetLength(flavorData);
						
						// getting the text
						textString = CFStringCreateWithCharacters(NULL, (UniChar *)CFDataGetBytePtr(flavorData), flavorDataSize >> 1);
						CFRelease(flavorData);
						}
					}
				else if (UTTypeConformsToTag(flavorType, 'ustl')) // and this is 'ustl'
					{
					if (PasteboardCopyItemFlavorData(gClipboard, itemID, flavorType, &flavorData) == noErr)
						{
						styleBufferSize = CFDataGetLength(flavorData);
						styleBuffer = malloc(styleBufferSize);
						
						// getting the styles
						memcpy(styleBuffer, CFDataGetBytePtr(flavorData), styleBufferSize);
						CFRelease(flavorData);
						}
					}
			}
		if (flavorTypeArray != NULL) CFRelease(flavorTypeArray);
		}

CantCopyPasteboardItemFlavors:
CantGetPasteboardItemIdentifier:
CantGetPasteboardItemCount:

	*oTextString = textString;
	*oStyleBuffer = styleBuffer;
	*oStyleBufferSize = styleBufferSize;

ExitExtractTextAndStyle:

	return status;
	}   // Extract_TextAndStyleFromPasteboard

/*****************************************************
*
* Extract_RunLengthsAndStylesFromStyleBuffer(styleBuffer, styleBufferSize, oNumberOfRuns, oRunLengths, oStyles) 
*
* Purpose:  Extracts the number of runs, lengths of each run, and style of each run from the flattened style buffer obtained from the Pasteboard
*
* Inputs:   styleBuffer         - the flattened style buffer
*				styleBufferSize     - the size of that style buffer
*				oNumberOfRuns       - the number of runs to return
*				oRunLengths         - the array of run lengths to return
*           oStyles             - the array of styles to return
*
* Returns:  OSErr			  - error code (0 == no error) 
*/
static OSStatus Extract_RunLengthsAndStylesFromStyleBuffer(void * styleBuffer, ByteCount styleBufferSize, ItemCount * oNumberOfRuns, UniCharCount ** oRunLengths, ATSUStyle ** oStyles)
	{
	OSStatus status = noErr;

	require_action(oNumberOfRuns != NULL, ExitExtractRunAndStyle, status = paramErr);
	require_action(oRunLengths != NULL, ExitExtractRunAndStyle, status = paramErr);
	require_action(oStyles != NULL, ExitExtractRunAndStyle, status = paramErr);
	
	ItemCount numberOfRuns = 0;
	UniCharCount * runLengths = NULL;
	ATSUStyle * styles = NULL;
	
	ATSUStyleRunInfo * runInfoArray = NULL;
	ATSUStyle * styleArray = NULL;
	
	// if we don't have a style buffer, there's not much we can do
	if (styleBuffer == NULL) goto CantUnflattenStyles;

	// read the comments in QD/ATSUnicodeFlattening.h for more details on the following
	ATSFlatDataMainHeaderBlock * ustlHeader = (ATSFlatDataMainHeaderBlock *)styleBuffer;
	require(ustlHeader->version == kATSFlatDataUstlVersion2, CantUnflattenStyles);

	ATSFlatDataStyleRunDataHeader * runHeader = (ATSFlatDataStyleRunDataHeader *)((UInt32)ustlHeader + ustlHeader->offsetToStyleRuns);
	numberOfRuns = runHeader->numberOfStyleRuns;
	runLengths = (UniCharCount *)malloc(numberOfRuns * sizeof(UniCharCount));
	require(runLengths != NULL, CantUnflattenStyles);
	styles = (ATSUStyle *)malloc(numberOfRuns * sizeof(ATSUStyle));
	require(styles != NULL, CantUnflattenStyles);

	ATSFlatDataStyleListHeader * styleHeader = (ATSFlatDataStyleListHeader *)((UInt32)ustlHeader + ustlHeader->offsetToStyleList);
	ItemCount numberOfStyles = styleHeader->numberOfStyles;

	runInfoArray = (ATSUStyleRunInfo * )malloc(numberOfRuns * sizeof(ATSUStyleRunInfo));
	require(runInfoArray != NULL, CantUnflattenStyles);
	styleArray = (ATSUStyle * )malloc(numberOfStyles * sizeof(ATSUStyle));
	require(styleArray != NULL, CantUnflattenStyles);

	status = ATSUUnflattenStyleRunsFromStream(
						kATSUDataStreamUnicodeStyledText,
						kATSUUnFlattenOptionNoOptionsMask,
						styleBufferSize,
						styleBuffer,
						numberOfRuns,
						numberOfStyles,
						runInfoArray,
						styleArray,
						NULL,
						NULL);
	require_noerr(status, CantUnflattenStyles);

	ItemCount i;
	for (i = 0; i < numberOfRuns; i++)
		{
		runLengths[i] = runInfoArray[i].runLength;
		styles[i] = styleArray[runInfoArray[i].styleObjectIndex];
		}

CantUnflattenStyles:

	if (runInfoArray != NULL) free(runInfoArray);
	if (styleArray != NULL) free(styleArray);
	*oNumberOfRuns = numberOfRuns;
	*oRunLengths = runLengths;
	*oStyles = styles;

ExitExtractRunAndStyle:

	return status;
	}   // Extract_RunLengthsAndStylesFromStyleBuffer

/*****************************************************
*
* UTTypeConformsToTag(inTypeIdentifier, tag) 
*
* Purpose:  Convenience utility function to parallel the UTTypeConformsTo API
*
* Inputs:   inTypeIdentifier    - the uniform type identifier to test
*				tag                 - the OSType tag against which to test conformance
*
* Returns:  Boolean             - true if conforms, false if not
*/
static Boolean UTTypeConformsToTag(CFStringRef inTypeIdentifier, OSType tag)
	{
	Boolean result = false;
	CFStringRef osTypeFlavorType = UTTypeCopyPreferredTagWithClass(inTypeIdentifier, kUTTagClassOSType);
	if (osTypeFlavorType != NULL)
		{
		result = (UTGetOSTypeFromString(osTypeFlavorType) == tag);
		CFRelease(osTypeFlavorType);
		}
	return result;
	}   // UTTypeConformsToTag
