/*
*	File:		HICustomView.c of HICustomView_Tester (4_Custom_Tracking)
* 
*	Contains:	A custom HIView to test
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

#include "HICustomView.h"

//****************************************************
#pragma mark -
#pragma mark * typedef's, struct's, enums, defines, etc. *

typedef struct
	{
	HIViewRef view;                             // our view

	Boolean trackOn;                            // tracking information
	HIPoint trackSpot;
	}
HICustomViewData;

//****************************************************
#pragma mark -
#pragma mark * local (static) function prototypes *

static pascal OSStatus Internal_HICustomViewHandler(EventHandlerCallRef inHandlerCallRef, EventRef inEvent, void *inUserData);
static OSStatus Internal_HICustomViewTrack(HICustomViewData * myData, EventRef inEvent);

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
* HICreateCustomView(boundsRect, outHICustomView) 
*
* Purpose:  Create a HICustomView with minimum parameter list,
*           the HICustomView can be set up later using other APIs
*
* Inputs:   boundsRect          - the HIRect for our view
*           outHICustomView     - returning the created HIView
*
* Returns:  OSStatus            - error code (0 == no error) 
*/
OSStatus HICreateCustomView(
		const HIRect * boundsRect,					// can be NULL
		HIViewRef * outHICustomView)				// cannot be NULL
	{
	OSStatus status;
	HIObjectRef hiObject;
	
	require_action(outHICustomView != NULL, exitCreation, status = paramErr);
	*outHICustomView = NULL;

	// create the view
	status = HIObjectCreate(GetHICustomViewClass(), 0, &hiObject);
	require_noerr(status, exitCreation);
		
	// position the view
	if (boundsRect != NULL)
		{
		status = HIViewSetFrame((HIViewRef)hiObject, boundsRect);
		require_noerr(status, exitCreation);
		}

	// return the view
	*outHICustomView = (HIViewRef)hiObject;

exitCreation:
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
CFStringRef GetHICustomViewClass(void)
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
				{ kEventClassControl, kEventControlDraw },
				{ kEventClassControl, kEventControlValueFieldChanged },
				{ kEventClassControl, kEventControlHiliteChanged },
				{ kEventClassControl, kEventControlHitTest },
				{ kEventClassControl, kEventControlTrack }
			};
		
		OSStatus status = HIObjectRegisterSubclass(kHICustomViewClass, kHIViewClassID, 0, Internal_HICustomViewHandler,
								  GetEventTypeCount(kFactoryEvents), kFactoryEvents, 0, &theClass);
		verify_noerr(status);
		}
	
	return kHICustomViewClass;
	}   // GetHICustomViewClass

//****************************************************
#pragma mark -
#pragma mark * local (static) function implementations *

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
#pragma mark *   kEventControlDraw
				case kEventControlDraw:
					{
					CGContextRef context;
					status = GetEventParameter(inEvent, kEventParamCGContextRef, typeCGContextRef, NULL, sizeof(context), NULL, &context);
					require_noerr(status, ControlDrawExit);

					HIRect bounds, viewBounds;
					HIViewGetBounds(myData->view, &viewBounds);

					// setting our colors according to state: IsControlEnabled, IsControlActive, IsControlHilited
					if (!IsControlEnabled(myData->view))
						{
						CGContextSetRGBFillColor(context, 0.3, 0.3, 0.3, 0.8);
						CGContextSetRGBStrokeColor(context, 0.5, 0.5, 0.5, 0.8);
						}
					else if (!IsControlActive(myData->view))
						{
						CGContextSetRGBFillColor(context, 0.7, 0.7, 0.7, 0.8);
						CGContextSetRGBStrokeColor(context, 0.8, 0.8, 0.8, 0.8);
						}
					else if (!IsControlHilited(myData->view))
						{
						CGContextSetRGBFillColor(context, 1, 0, 0, 0.8);
						CGContextSetRGBStrokeColor(context, 0, 0, 1, 0.8);
						}
					else
						{
						CGContextSetRGBFillColor(context, 0.7, 0, 0, 0.8);
						CGContextSetRGBStrokeColor(context, 0, 0, 0.7, 0.8);
						}

					// using a line thickness of 3
					CGContextSetLineWidth(context, 3);
					bounds = CGRectInset(viewBounds, 3, 3);
					float minDim = (bounds.size.height < bounds.size.width) ? bounds.size.height / 2 : bounds.size.width / 2;
					float cx = bounds.origin.x + minDim, cy = bounds.origin.y + minDim;
					UInt32 i, n = GetControl32BitValue(myData->view);

					// having some fun with geometric shapes based on the value of the custom view
					CGContextBeginPath(context);
					switch (n)
						{
						case 0: CGContextAddArc(context, cx, cy, minDim, 0, 2 * pi, true); break;
						case 1: CGContextAddEllipseInRect(context, CGRectInset(bounds, bounds.size.width * 0.4, 0)); break;
						default:
							{
							float deltangle = pi / n, angle = 0, r = minDim / 2;
							CGContextMoveToPoint(context, cx + minDim, cy);
							for (i = 0; i < n; i++)
								{
								angle += deltangle;
								CGContextAddLineToPoint(context, cx + r * cos(angle), cy + r * sin(angle));
								angle += deltangle;
								CGContextAddLineToPoint(context, cx + minDim * cos(angle), cy + minDim * sin(angle));
								}
							CGContextAddLineToPoint(context, cx + minDim, cy);
							}
						}
					CGContextClosePath(context);
					CGContextDrawPath(context, kCGPathFillStroke);
					
					// if we're tracking then we also draw the track spot
					if (myData->trackOn)
						{
						CGContextBeginPath(context);
						CGContextAddArc(context, myData->trackSpot.x, myData->trackSpot.y, 10, 0, 2 * pi, 1);
						CGContextClosePath(context);
						CGContextSetRGBFillColor(context, 0.7, 0, 0.7, 0.8);
						CGContextSetRGBStrokeColor(context, 0, 0.7, 0.7, 0.8);
						CGContextDrawPath(context, kCGPathFillStroke);
						}

					status = noErr;
ControlDrawExit:
					break;
					}

#pragma mark *   kEventControl___Changed
				case kEventControlValueFieldChanged:
				case kEventControlHiliteChanged:
					{
					// just asking for a refresh
					HIViewSetNeedsDisplay(myData->view, true);
					break;
					}

#pragma mark *   kEventControlHitTest
				case kEventControlHitTest:
					{
					// we need to implement the kEventControlHitTest handler so that either our kEventControlTrack handler
					// or our kEventControlHiliteChanged handler will be called
					HIPoint mouseLoc;
					status = GetEventParameter(inEvent, kEventParamMouseLocation, typeHIPoint, NULL, sizeof(mouseLoc), NULL, &mouseLoc);
					require_noerr(status, ControlHitTestExit);

					HIRect hiBounds;
					status = HIViewGetBounds(myData->view, &hiBounds);
					require_noerr(status, ControlHitTestExit);

					ControlPartCode partCode = ( CGRectContainsPoint(hiBounds, mouseLoc) ) ? kControlButtonPart : kControlNoPart;
					status = SetEventParameter(inEvent, kEventParamControlPart, typeControlPartCode, sizeof(partCode), &partCode);
					require_noerr(status, ControlHitTestExit);

ControlHitTestExit:
  					break;
					}

#pragma mark *   kEventControlTrack
				case kEventControlTrack:
					{
					// handling the mouse tracking
					status = Internal_HICustomViewTrack(myData, inEvent);
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

/*****************************************************
*
* Internal_HICustomViewTrack(myData, inEvent) 
*
* Purpose:  Track Event handler
*
* Inputs:   myData              - our custom view data
*			inEvent             - the event
*
* Returns:  OSStatus            - error code (0 == no error) 
*/
static OSStatus Internal_HICustomViewTrack(HICustomViewData * myData, EventRef inEvent)
	{
	// Here we are having some fun with a time-controlled value setting
	// The value is going to be incremented each time we move within our bounds
	// or incremented every 1/3 second if we don't move but are within our bounds
	// If we reach the maximum, we go back to the minimum
	OSStatus status;
	
	// Getting our bounds and locations
	HIRect hiViewBounds;
	status = HIViewGetBounds(myData->view, &hiViewBounds);
	require_noerr(status, HIViewGetBounds);
	
	Rect qdWindowBounds;
	status = GetWindowBounds(GetControlOwner(myData->view), kWindowStructureRgn, &qdWindowBounds);
	require_noerr(status, GetWindowBounds);

	HIPoint hiPoint;
	status = GetEventParameter(inEvent, kEventParamMouseLocation, typeHIPoint, NULL, sizeof(hiPoint), NULL, &hiPoint);
	require_noerr(status, GetEventParameter);

	// handle the first mouseDown before moving
	MouseTrackingResult mouseStatus = kMouseTrackingMouseDown;
	while (mouseStatus != kMouseTrackingMouseUp)
		{
		if (CGRectContainsPoint(hiViewBounds, hiPoint))
			{
			if (hiPoint.x < hiViewBounds.origin.x+4) hiPoint.x = hiViewBounds.origin.x+4;
			if (hiPoint.x > hiViewBounds.origin.x+hiViewBounds.size.width-4) hiPoint.x = hiViewBounds.origin.x+hiViewBounds.size.width-4;
			if (hiPoint.y < hiViewBounds.origin.y+4) hiPoint.y = hiViewBounds.origin.y+4;
			if (hiPoint.y > hiViewBounds.origin.y+hiViewBounds.size.height-4) hiPoint.y = hiViewBounds.origin.y+hiViewBounds.size.height-4;
			myData->trackSpot = hiPoint;

			// setting the new value
			SInt32 currentValue = GetControl32BitValue(myData->view);
			UInt32 newValue = currentValue + 1;
			newValue %= (GetControl32BitMaximum(myData->view) + 1);
			SetControl32BitValue(myData->view, newValue);
			
			myData->trackOn = true;
			HiliteControl(myData->view, 1);
			}
		else if (myData->trackOn)
			{
			myData->trackOn = false;
			HiliteControl(myData->view, 0);
			}

		// a -1 GrafPtr to TrackMouseLocationWithOptions yields global coordinates
		Point qdPoint;
		status = TrackMouseLocationWithOptions((GrafPtr)-1L, 0, kEventDurationSecond / 3, &qdPoint, NULL, &mouseStatus);						
		require_noerr(status, TrackMouseLocation);

		// convert to window-relative coordinates
		hiPoint.x = qdPoint.h - qdWindowBounds.left;
		hiPoint.y = qdPoint.v - qdWindowBounds.top;
		
		// convert to view-relative coordinates
		HIViewConvertPoint(&hiPoint, NULL, myData->view);
		}

SendEventToEventTarget:
GetWindowEventTarget:
SetEventParameter:
CreateEvent:
TrackMouseLocation:
GetEventParameter:
GetWindowBounds:
HIViewGetBounds:

	myData->trackOn = false;
	HiliteControl(myData->view, 0);

	return status;
	}
