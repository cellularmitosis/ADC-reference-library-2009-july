/*
*	File:		CarbonTransparentWindow.c of CarbonTransparentWindow
* 
*	Contains:	2 Carbon transparent windows.
*	
*	Version:	1.0
* 
*	Created:	6/20/05
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
//****************************************************
#pragma mark * compilation directives *

//****************************************************
#pragma mark -
#pragma mark * includes & imports *

#include "CarbonTransparentWindow.h"

//****************************************************
#pragma mark -
#pragma mark * typedef's, struct's, enums, defines, etc. *

//****************************************************
#pragma mark -
#pragma mark * local (static) function prototypes *

static pascal OSStatus UserPaneCompDraw(EventHandlerCallRef inHandlerCallRef, EventRef inEvent, void *inUserData);
static pascal OSStatus UserPaneNonCompDraw(EventHandlerCallRef inHandlerCallRef, EventRef inEvent, void *inUserData);
static OSStatus MakeWindowTransparent(WindowRef aWindowRef);
static pascal OSStatus TransparentWindowHandler(EventHandlerCallRef inHandlerCallRef, EventRef inEvent, void *inUserData);

//****************************************************
#pragma mark -
#pragma mark * exported globals *

//****************************************************
#pragma mark -
#pragma mark * local (static) globals *

//****************************************************
#pragma mark -
#pragma mark * exported function implementations *


// the code for Do_NewCompositingWindow and Do_NewNonCompositingWindow is
// actually the same except for the name of the nib resource (CompWindow vs. NonCompWindow)
// and the user pane draw handler which uses QuickDraw in the Non-Compositing window and
// Quartz in the Compositing window

// in the nib, both windows are also the same except that one has the compositing attribute
// and the other one has not.


/*****************************************************
*
* Do_NewCompositingWindow() 
*
* Purpose:  Create a compositing transparent window
*
* Inputs:   none
*
* Returns:  OSStatus            - error code (0 == no error) 
*/
OSStatus Do_NewCompositingWindow()
{
	WindowRef aWindowRef = NULL;
	OSStatus status;

	// create a Nib reference passing the name of the nib file (without the .nib extension)
	// CreateNibReference only searches into the application bundle.
	IBNibRef anIBNibRef;
	status = CreateNibReference(CFSTR("main"), &anIBNibRef);
	require_noerr(status, CantGetNibRef);
	
	// create a window. "CompWindow" is the name of the window object. This name is set in 
	// InterfaceBuilder when the nib is created.
	status = CreateWindowFromNib(anIBNibRef, CFSTR("CompWindow"), &aWindowRef);
	require_noerr(status, CantCreateWindow);
	require(aWindowRef != NULL, CantCreateWindow);
	
	// we no longer need the nib reference
	DisposeNibReference(anIBNibRef);
	
	// looking for our user pane
	HIViewID userPaneID = { 'UsrP', 100 };
	HIViewRef userPane;
	status = HIViewFindByID(HIViewGetRoot(aWindowRef), userPaneID, &userPane);
	require_noerr(status, CantCreateWindow);
	require(userPane != NULL, CantCreateWindow);
	
	// so that we can install its custom drawing handler
	EventTypeSpec eventCD = { kEventClassControl, kEventControlDraw };
	status = InstallControlEventHandler(userPane, UserPaneCompDraw, 1, &eventCD, userPane, NULL);
	require_noerr(status, CantCreateWindow);
	
	// make this window transparent
	status = MakeWindowTransparent(aWindowRef);
	require_noerr(status, MakeWindowTransparent);
		
	ShowWindow(aWindowRef);

MakeWindowTransparent:
CantCreateWindow:
CantGetNibRef:
	
	return status;
}   // Do_NewCompositingWindow

/*****************************************************
*
* Do_NewNonCompositingWindow() 
*
* Purpose:  Create a non-compositing transparent window
*
* Inputs:   none
*
* Returns:  OSStatus            - error code (0 == no error) 
*/
OSStatus Do_NewNonCompositingWindow()
{
	WindowRef aWindowRef = NULL;
	OSStatus status;

	// create a Nib reference passing the name of the nib file (without the .nib extension)
	// CreateNibReference only searches into the application bundle.
	IBNibRef anIBNibRef;
	status = CreateNibReference(CFSTR("main"), &anIBNibRef);
	require_noerr(status, CantGetNibRef);
	
	// create a window. "NonCompWindow" is the name of the window object. This name is set in 
	// InterfaceBuilder when the nib is created.
	status = CreateWindowFromNib(anIBNibRef, CFSTR("NonCompWindow"), &aWindowRef);
	require_noerr(status, CantCreateWindow);
	require(aWindowRef != NULL, CantCreateWindow);
	
	// we no longer need the nib reference
	DisposeNibReference(anIBNibRef);
	
	// looking for our user pane
	HIViewID userPaneID = { 'UsrP', 100 };
	HIViewRef userPane;
	status = HIViewFindByID(HIViewGetRoot(aWindowRef), userPaneID, &userPane);
	require_noerr(status, CantCreateWindow);
	require(userPane != NULL, CantCreateWindow);
	
	// so that we can install its custom drawing handler
	EventTypeSpec eventCD = { kEventClassControl, kEventControlDraw };
	status = InstallControlEventHandler(userPane, UserPaneNonCompDraw, 1, &eventCD, userPane, NULL);
	require_noerr(status, CantCreateWindow);
	
	// make this window transparent
	status = MakeWindowTransparent(aWindowRef);
	require_noerr(status, MakeWindowTransparent);
		
	ShowWindow(aWindowRef);

MakeWindowTransparent:
CantCreateWindow:
CantGetNibRef:
	
	return status;
}   // Do_NewNonCompositingWindow

//****************************************************
#pragma mark -
#pragma mark * local (static) function implementations *

/*****************************************************
*
* UserPaneCompDraw(inHandlerCallRef, inEvent, inUserData) 
*
* Purpose:  The draw handler for our user pane in the Compositing window, we just paint 2 rectangles in 2 different transparent colors with Quartz
*
* Inputs:   inHandlerCallRef    - reference to the current handler call chain
*			inEvent             - the event
*           inUserData          - app-specified data you passed in the call to InstallEventHandler
*
* Returns:  OSStatus            - error code (0 == no error) 
*/
static pascal OSStatus UserPaneCompDraw(EventHandlerCallRef inHandlerCallRef, EventRef inEvent, void *inUserData)
{
	OSStatus status;

	CGContextRef context;
	status = GetEventParameter(inEvent, kEventParamCGContextRef, typeCGContextRef, NULL, sizeof(context), NULL, &context);
	require_noerr(status, GetEventParameter);

	HIRect bounds;
	status = HIViewGetBounds((HIViewRef)inUserData, &bounds);
	require_noerr(status, HIViewGetBounds);

	CGContextSetRGBFillColor(context, 1, 0, 0, 0.5);
	CGContextFillRect(context, bounds);

	CGContextSetRGBFillColor(context, 1, 1, 0, 0.5);
	bounds = CGRectInset(bounds, 30, 30);
	CGContextFillRect(context, bounds);
	
GetEventParameter:
HIViewGetBounds:

	return status;
}   // UserPaneCompDraw

/*****************************************************
*
* UserPaneNonCompDraw(inHandlerCallRef, inEvent, inUserData) 
*
* Purpose:  The draw handler for our user pane in the Non-Compositing window, we just paint 2 rectangles in 2 different colors with QuickDraw
*
* Inputs:   inHandlerCallRef    - reference to the current handler call chain
*			inEvent             - the event
*           inUserData          - app-specified data you passed in the call to InstallEventHandler
*
* Returns:  OSStatus            - error code (0 == no error) 
*/
static pascal OSStatus UserPaneNonCompDraw(EventHandlerCallRef inHandlerCallRef, EventRef inEvent, void *inUserData)
{
	OSStatus status = noErr;
	
	Rect bounds;
	GetControlBounds((HIViewRef)inUserData, &bounds);

	RGBColor redColor = { 65535, 0, 0 };
	RGBForeColor(&redColor);
	PaintRect(&bounds);

	RGBColor yellowColor = { 65535, 65535, 0 };
	RGBForeColor(&yellowColor);
	InsetRect(&bounds, 30, 30);
	PaintRect(&bounds);

	return status;
}   // UserPaneNonCompDraw

/*****************************************************
*
* MakeWindowTransparent(aWindowRef) 
*
* Purpose:  Makes a window (Non-Compositing or Compositing) transparent using custom handlers
*
* Inputs:   aWindowRef          - the window to make transparent
*
* Returns:  OSStatus            - error code (0 == no error) 
*/
static OSStatus MakeWindowTransparent(WindowRef aWindowRef)
{
	OSStatus status = paramErr;
	require(aWindowRef != NULL, paramErr);
	
	// is the window compositing or not?
	WindowAttributes attributes;
	status = GetWindowAttributes(aWindowRef, &attributes);
	require_noerr(status, GetWindowAttributes);
	
	if (attributes & kWindowCompositingAttribute)
	{
		// it is compositing so we intercept the kEventWindowGetRegion event to be able to specify an empty opaque region
		EventTypeSpec wCompositingEvents = { kEventClassWindow, kEventWindowGetRegion };
		status = InstallWindowEventHandler(aWindowRef, TransparentWindowHandler, 1, &wCompositingEvents, aWindowRef, NULL);
		require_noerr(status, InstallWindowEventHandler);
		
		HIViewRef contentView;
		status = HIViewFindByID(HIViewGetRoot(aWindowRef), kHIViewWindowContentID, &contentView);
		require_noerr(status, HIViewFindByID);

		// and we intercept the kEventControlDraw event of our content view so that we can make it transparent
		EventTypeSpec cCompositingEvents = { kEventClassControl, kEventControlDraw };
		status = InstallControlEventHandler(contentView, TransparentWindowHandler, 1, &cCompositingEvents, contentView, NULL);
		require_noerr(status, InstallControlEventHandler);
	}
	else
	{
		// it is non-compositing so we intercept the kEventWindowGetRegion event to be able to specify an empty opaque region
		// and we intercept the kEventWindowDrawContent event of our window so that we can make it transparent
		EventTypeSpec wNonCompositingEvents[] =
		{
			{ kEventClassWindow, kEventWindowGetRegion },
			{ kEventClassWindow, kEventWindowDrawContent }
		};
		status = InstallWindowEventHandler(aWindowRef, TransparentWindowHandler, GetEventTypeCount(wNonCompositingEvents), wNonCompositingEvents, aWindowRef, NULL);
		require_noerr(status, InstallWindowEventHandler);
	}

	// telling the HIToolbox that our window is not opaque so that we will be asked for the opaque region
	UInt32 features;
	status = GetWindowFeatures(aWindowRef, &features);
	require_noerr(status, GetWindowFeatures);
	if ( ( features & kWindowIsOpaque ) != 0 )
	{
		status = HIWindowChangeFeatures(aWindowRef, 0, kWindowIsOpaque);
		require_noerr(status, HIWindowChangeFeatures);
	}
		
	// force opaque shape to be recalculated
	status = ReshapeCustomWindow(aWindowRef);
	require_noerr(status, ReshapeCustomWindow);

	// ensure that HIToolbox doesn't use standard shadow style, which defeats custom opaque shape
	status = SetWindowAlpha(aWindowRef, 0.999);
	require_noerr(status, SetWindowAlpha);

SetWindowAlpha:
ReshapeCustomWindow:
HIWindowChangeFeatures:
GetWindowFeatures:
InstallControlEventHandler:
HIViewFindByID:
InstallWindowEventHandler:
GetWindowAttributes:
paramErr:

	return status;
}   // MakeWindowTransparent

/*****************************************************
*
* TransparentWindowHandler(inHandlerCallRef, inEvent, inUserData) 
*
* Purpose:  The handlers for the 3 events that we intercept in MakeWindowTransparent
*
* Inputs:   inHandlerCallRef    - reference to the current handler call chain
*			inEvent             - the event
*           inUserData          - app-specified data you passed in the call to InstallEventHandler
*
* Returns:  OSStatus            - error code (0 == no error) 
*/
static pascal OSStatus TransparentWindowHandler(EventHandlerCallRef inHandlerCallRef, EventRef inEvent, void *inUserData)
{
	OSStatus status = eventNotHandledErr;
	
	switch(GetEventKind(inEvent))
	{
		case kEventWindowGetRegion:
		{
			WindowRegionCode code;
			RgnHandle rgn;
			
			// which region code is being queried?
			GetEventParameter(inEvent, kEventParamWindowRegionCode, typeWindowRegionCode, NULL, sizeof(code), NULL, &code);

			// if it is the opaque region code then set the region to Empty and return noErr to stop the propagation
			if (code == kWindowOpaqueRgn)
			{
				GetEventParameter(inEvent, kEventParamRgnHandle, typeQDRgnHandle, NULL, sizeof(rgn), NULL, &rgn);
				SetEmptyRgn(rgn);
				status = noErr;
			}
			break;
		}

		case kEventWindowDrawContent:
		{
			GrafPtr port;
			CGContextRef context;
			Rect portBounds;
			HIRect bounds;
			
			GetPort(&port);
			GetPortBounds(port, &portBounds);
			
			// we need a Quartz context so that we can use transparency
			QDBeginCGContext(port, &context);
			
			// make the whole content transparent
			bounds = CGRectMake(0, 0, portBounds.right, portBounds.bottom);
			CGContextClearRect(context, bounds);
			
			QDEndCGContext(port, &context);
			
			// we need to let the HIToolbox and the regular kEventWindowDrawContent handler do their work,
			// mainly draw the subviews, so we return eventNotHandledErr to propagate.
			status = eventNotHandledErr;
			break;
		}

		case kEventControlDraw:
		{
			CGContextRef context;
			HIRect bounds;

			GetEventParameter(inEvent, kEventParamCGContextRef, typeCGContextRef, NULL, sizeof(context), NULL, &context);
			HIViewGetBounds((HIViewRef)inUserData, &bounds);

			// make the whole content transparent
			CGContextClearRect(context, bounds);

			// we must not let the default draw handler of the content view be called (it would draw the default opaque theme)
			// so we return noErr to stop the propagation.
			status = noErr;
			break;
		}
	}
	
	return status;
}   // TransparentWindowHandler
