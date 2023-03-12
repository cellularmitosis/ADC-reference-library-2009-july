/*
*	File:		CustomTransparentWindow.c of CarbonTransparentWindow
* 
*	Contains:	1 custom transparent window.
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

#include "CustomTransparentWindow.h"

//****************************************************
#pragma mark -
#pragma mark * typedef's, struct's, enums, defines, etc. *

#define kHICustomViewClass	CFSTR("com.apple.sample.dts.HICustomView")

typedef struct
	{
	HIViewRef view;                             // our view
	}
HICustomViewData;

//****************************************************
#pragma mark -
#pragma mark * local (static) function prototypes *

static CFStringRef GetHICustomViewClass(void);
static OSStatus HICreateCustomView(HIViewRef * outHICustomView);
static pascal OSStatus Internal_HICustomViewHandler(EventHandlerCallRef inHandlerCallRef, EventRef inEvent, void *inUserData);

//****************************************************
#pragma mark -
#pragma mark * exported globals *

//****************************************************
#pragma mark -
#pragma mark * local (static) globals *

//****************************************************
#pragma mark -
#pragma mark * exported function implementations *

/*****************************************************
*
* Do_NewCustomCompositingWindow() 
*
* Purpose:  Create a custom compositing transparent window
*
* Inputs:   none
*
* Returns:  OSStatus            - error code (0 == no error) 
*/
OSStatus Do_NewCustomCompositingWindow()
{
	OSStatus status;

	// create a custom View
	HIViewRef customView;
	status = HICreateCustomView(&customView);
	require_noerr(status, HICreateCustomView);
	
	// create a custom window
	Rect bounds = {0, 0, 360, 480};
	WindowDefSpec def = {kWindowDefHIView, (void *)customView};
	WindowRef aWindowRef;
	status = CreateCustomWindow(&def, kPlainWindowClass, kWindowStandardHandlerAttribute | kWindowCompositingAttribute, &bounds, &aWindowRef);
	require_noerr(status, CreateCustomWindow);
	require(aWindowRef != NULL, CreateCustomWindow);

	// telling the HIToolbox that our window is not opaque
	UInt32 features;
	status = GetWindowFeatures(aWindowRef, &features);
	require_noerr(status, GetWindowFeatures);
	if ( ( features & kWindowIsOpaque ) != 0 )
	{
		status = HIWindowChangeFeatures(aWindowRef, 0, kWindowIsOpaque);
		require_noerr(status, HIWindowChangeFeatures);
	}

	// adding some content
	Rect buttonBounds = { 50, 50, 70, 350 };
	HIViewRef button;
	status = CreatePushButtonControl(NULL, &buttonBounds, CFSTR("Large Text Push Button"), &button);
	require_noerr(status, CreatePushButtonControl);
	
	status = HIViewAddSubview(customView, button);
	require_noerr(status, HIViewAddSubview);

	MoveWindow(aWindowRef, 100, 100, true);

	ShowWindow(aWindowRef);

HIViewAddSubview:
HIViewFindByID:
CreatePushButtonControl:
ReshapeCustomWindow:
HIWindowChangeFeatures:
GetWindowFeatures:
CreateCustomWindow:
HICreateCustomView:

	return status;
}   // Do_NewCustomCompositingWindow

//****************************************************
#pragma mark -
#pragma mark * local (static) function implementations *

/*****************************************************
*
* HICreateCustomView(outHICustomView) 
*
* Purpose:  Create a HICustomView
*
* Inputs:   outHICustomView     - returning the created HIView
*
* Returns:  OSStatus            - error code (0 == no error) 
*/
static OSStatus HICreateCustomView(HIViewRef * outHICustomView)
{
	OSStatus status = paramErr;
	require(outHICustomView != NULL, paramErr);
	*outHICustomView = NULL;

	// create the view
	HIObjectRef hiObject;
	status = HIObjectCreate(GetHICustomViewClass(), 0, &hiObject);
	require_noerr(status, HIObjectCreate);

	// return the view
	*outHICustomView = (HIViewRef)hiObject;

HIObjectCreate:
paramErr:

	return status;
}   // HICreateCustomView

/*****************************************************
*
* GetHICustomViewClass() 
*
* Purpose:  Registers our custom HICustomView view class and installs the appropriate handlers
*
* Inputs:   none
*
* Returns:  our class ID as a CFStringRef
*/
static CFStringRef GetHICustomViewClass(void)
{
	// following code is pretty much boiler plate.
	static HIObjectClassRef	theClass;
	
	if (theClass == NULL)
	{
		static EventTypeSpec kFactoryEvents[] =
		{
			{ kEventClassHIObject, kEventHIObjectConstruct },
			{ kEventClassHIObject, kEventHIObjectInitialize },
			{ kEventClassHIObject, kEventHIObjectDestruct },
			{ kEventClassControl, kEventControlInitialize },
			{ kEventClassControl, kEventControlDraw },
			{ kEventClassControl, kEventControlHitTest },
			{ kEventClassControl, kEventControlTrack }
		};
		
		HIObjectRegisterSubclass(kHICustomViewClass, kHIViewClassID, 0, Internal_HICustomViewHandler,
								  GetEventTypeCount(kFactoryEvents), kFactoryEvents, 0, &theClass);
	}
	
	return kHICustomViewClass;
}   // GetHICustomViewClass

/*****************************************************
*
* Internal_HICustomViewHandler(inHandlerCallRef, inEvent, inUserData) 
*
* Purpose:  Event handler that implements our HICustomView custom view
*
* Inputs:   inHandlerCallRef    - reference to the current handler call chain
*           inEvent             - the event
*           inUserData          - app-specified data you passed in the call to InstallEventHandler
*
* Returns:  OSStatus            - error code (0 == no error) 
*/
static pascal OSStatus Internal_HICustomViewHandler(EventHandlerCallRef inHandlerCallRef, EventRef inEvent, void * inUserData)
{
	OSStatus status = eventNotHandledErr;
	HICustomViewData * myData = (HICustomViewData *)inUserData;

	switch (GetEventClass(inEvent))
	{
		case kEventClassHIObject:
			switch (GetEventKind(inEvent))
			{
				case kEventHIObjectConstruct:
				{
					// allocate some instance data
					myData = (HICustomViewData *) calloc(1, sizeof(HICustomViewData));
					require_action(myData != NULL, ConstructExit, status = memFullErr);
					
					// get our superclass instance
					HIViewRef epView;
					status = GetEventParameter(inEvent, kEventParamHIObjectInstance, typeHIObjectRef, NULL, sizeof(epView), NULL, &epView);
					require_noerr(status, ConstructExit);
					
					// remember our superclass in our instance data
					myData->view = epView;
					// calloc fills our structure with all zeroes which is what we want a default value for all our fields
					// so we don't need to set them to NULL or 0 one by one
					
					// store our instance data into the event
					status = SetEventParameter(inEvent, kEventParamHIObjectInstance, typeVoidPtr, sizeof(myData), &myData);
					require_noerr(status, ConstructExit);
ConstructExit:
					break;
				}
					
#pragma mark *   kEventHIObjectInitialize
				case kEventHIObjectInitialize:
				{
					// always begin kEventHIObjectInitialize by calling through to the previous handler
					status = CallNextEventHandler(inHandlerCallRef, inEvent);
					require_noerr(status, InitializeExit);
					
					// if that succeeded, do our own initialization

					// in this sample code, there is nothing to do

InitializeExit:
					break;
				}
					
				case kEventHIObjectDestruct:
				{
					// freeing our storage
					if (myData != NULL) free(myData);
					status = noErr;
					break;
				}
				
				default:
					break;
			}
			break;

		case kEventClassControl:
			switch (GetEventKind(inEvent))
			{
#pragma mark *   kEventControlInitialize
				case kEventControlInitialize:
					{
					status = CallNextEventHandler(inHandlerCallRef, inEvent);
					require_noerr(status, ControlInitializeExit);

					// we need to be an embedder HIView so that others HIViews can be added in our custom window
					UInt32 features = 0;
					status = GetEventParameter(inEvent, kEventParamControlFeatures, typeUInt32, NULL, sizeof(features), NULL, &features);
					if (status == noErr)
						features |= kControlSupportsEmbedding;
					else
						features = kControlSupportsEmbedding;

					status = SetEventParameter(inEvent, kEventParamControlFeatures, typeUInt32, sizeof features, &features);
					require_noerr(status, ControlInitializeExit);

ControlInitializeExit:
					break;
					}

#pragma mark *   kEventControlDraw
				case kEventControlDraw:
				{
					CGContextRef context;
					status = GetEventParameter(inEvent, kEventParamCGContextRef, typeCGContextRef, NULL, sizeof(context), NULL, &context);
					require_noerr(status, ControlDrawExit);

					HIRect bounds, viewBounds;
					HIViewGetBounds(myData->view, &viewBounds);
					
					// this will make the entire view totally transparent
					CGContextClearRect(context, viewBounds);

					// and now we draw with partially opaque colors only where we want
					CGContextSetRGBFillColor(context, 0.7, 0, 0, 0.8);
					CGContextSetRGBStrokeColor(context, 0, 0, 0.7, 0.8);

					// using a line thickness of 3
					CGContextSetLineWidth(context, 3);
					bounds = CGRectInset(viewBounds, 3, 3);
					float minDim = (bounds.size.height < bounds.size.width) ? bounds.size.height / 2 : bounds.size.width / 2;
					float cx = bounds.origin.x + minDim, cy = bounds.origin.y + minDim;

					// drawing an ellipse
					CGContextBeginPath(context);
					CGContextAddEllipseInRect(context, CGRectInset(bounds, bounds.size.width * 0.4, 0));
					CGContextClosePath(context);
					CGContextDrawPath(context, kCGPathFillStroke);

					status = noErr;
ControlDrawExit:
					break;
				}

				// we want our custom window to be draggable as if it were a metal window, ie. any click-and-drag in
				// the content other than a subview will move the window around
				// we just need to handle kEventControlHitTest and kEventControlTrack for that

#pragma mark *   kEventControlHitTest
				case kEventControlHitTest:
				{
					// the point parameter is in view-local coords.
					HIPoint	pt;
					status = GetEventParameter(inEvent, kEventParamMouseLocation, typeHIPoint, NULL, sizeof(pt), NULL, &pt);
					require_noerr(status, ControlHitTestExit);

					HIRect bounds;
					status = HIViewGetBounds(myData->view, &bounds);
					require_noerr(status, ControlHitTestExit);
					
					if (CGRectContainsPoint(bounds, pt))
					{
						ControlPartCode part = kControlButtonPart;
						SetEventParameter(inEvent, kEventParamControlPart, typeControlPartCode, sizeof(part), &part);
						status = noErr;
					}
					else status = eventNotHandledErr;
ControlHitTestExit:
					break;
				}

#pragma mark *   kEventControlTrack
				case kEventControlTrack:
				{
					Point pt;
					status = GetEventParameter(inEvent, kEventParamMouseLocation, typeQDPoint, NULL, sizeof(pt), NULL, &pt);
					require_noerr(status, ControlTrackExit);

					WindowRef window = GetControlOwner(myData->view);

					//
					// Convert point from view coordinates to global coordinates. On Tiger and later,
					// you could also use HIPointConvert to do this.
					//
					Rect windowBounds;
					GetWindowBounds(window, kWindowStructureRgn, &windowBounds );
					pt.h += windowBounds.left;
					pt.v += windowBounds.top;

					DragWindow(window, pt, NULL);
ControlTrackExit:
					break;
				}

				default:
					break;
			}
			break;
			
		default:
			break;
	}
	
	return status;
}   // Internal_HICustomViewHandler
