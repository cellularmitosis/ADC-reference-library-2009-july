/*
*	File:		ScrollAndZoom.c of ScrollAndZoom
* 
*	Contains:	An illustration of the use of the Context Transformation Matrix (CTM) with HIViews.
*
*	Version:	1.0
* 
*	Created:	October 7th, 2004
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
*	Copyright:  Copyright © 2004 Apple Computer, Inc, All Rights Reserved
*/
//****************************************************
#pragma mark * compilation directives *

//****************************************************
#pragma mark -
#pragma mark * includes & imports *

#include "ScrollAndZoom.h"

//****************************************************
#pragma mark -
#pragma mark * typedef's, struct's, enums, defines, etc. *

//****************************************************
#pragma mark -
#pragma mark * local (static) function prototypes *

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
* myLiveSliderAction(theControl, partCode) 
*
* Purpose:  Necessary action proc to have a "live" slider, even if it does nothing
*
* Inputs:   theControl   - the slider control
*				partCode     - the part being actionned
*
* Returns:  none
*/
void pascal myLiveSliderAction(ControlRef theControl, ControlPartCode partCode)
	{
	}

/*****************************************************
*
* SanityCheck(where, myData) 
*
* Purpose:  Makes sure that we always scroll in a position such that we don't display out-of-bounds content
*
* Inputs:   where   - the point where we are being asked to scroll to
*				myData  - all the data necessary to check the bounds, the origin, and the scale factor
*
* Returns:  the point where it is safe to scroll to
*/
HIPoint SanityCheck(HIPoint where, ScrollAndZoomData* myData)
	{
	HIRect bounds;
	HIViewGetBounds(myData->view, &bounds);
	HISize imageSize = myData->imageSize;	

	bounds.size.height /= myData->zoomFactor;
	bounds.size.width /= myData->zoomFactor;

	if (where.y + bounds.size.height > imageSize.height) 
		where.y = imageSize.height - bounds.size.height;
	if (where.y < 0) where.y = 0;

	if (where.x + bounds.size.width > imageSize.width) 
		where.x = imageSize.width - bounds.size.width;
	if (where.x < 0) where.x = 0;
	
	return where;
	}

/*****************************************************
*
* ScrollAndZoomHandler(inHandlerCallRef, inEvent, inUserData) 
*
* Purpose:  Event handler that implements our HIScrollAndZoom custom view
*
* Inputs:   inHandlerCallRef    - reference to the current handler call chain
*				inEvent             - the event
*           inUserData          - app-specified data you passed in the call to InstallEventHandler
*
* Returns:  OSStatus            - error code (0 == no error) 
*/
pascal OSStatus ScrollAndZoomHandler(EventHandlerCallRef inHandlerCallRef, EventRef inEvent, void *inUserData)
	{
	OSStatus result = eventNotHandledErr;
	ScrollAndZoomData* myData = (ScrollAndZoomData*)inUserData;

	switch (GetEventClass(inEvent))
		{
		case kEventClassHIObject:
			switch (GetEventKind(inEvent))
				{
				case kEventHIObjectConstruct:
					{
					// allocate some instance data
					myData = (ScrollAndZoomData*) calloc(1, sizeof(ScrollAndZoomData));
					
					// get our superclass instance
					HIViewRef epView;
					result = GetEventParameter(inEvent, kEventParamHIObjectInstance, typeHIObjectRef, NULL, sizeof(epView), NULL, &epView);
					
					// remember our superclass in our instance data
					myData->view = epView;
					
					// store our instance data into the event
					result = SetEventParameter(inEvent, kEventParamHIObjectInstance, typeVoidPtr, sizeof(myData), &myData);
					break;
					}
					
				case kEventHIObjectInitialize:
					{
					// always begin kEventHIObjectInitialize by calling through to the previous handler
					result = CallNextEventHandler(inHandlerCallRef, inEvent);
					
					// if that succeeded, do our own initialization
					if (result == noErr)
						{
						myData->originPoint.x = 0;
						myData->originPoint.y = 0;
						myData->imageSize.width = 3000;
						myData->imageSize.height = 2000;
						myData->zoomFactor = 1;

						// Make ourselves opaque to optimize the composite drawing
						HIViewChangeFeatures(myData->view, kHIViewIsOpaque, 0);
						}
					break;
					}
					
				case kEventHIObjectDestruct:
					{
					if (myData != NULL)
						free(myData);
					result = noErr;
					break;
					}
				
				default:
					break;
				}
			break;

		case kEventClassScrollable:
			switch (GetEventKind(inEvent))
				{
				case kEventScrollableGetInfo:
					{
					// we're being asked to return information about the scrolled view that we set as Event Parameters
					HIRect bounds;
					HIViewGetBounds(myData->view, &bounds);
					HISize imageSize = myData->imageSize;
					HISize lineSize = { kLineSize, kLineSize };

					// From our scrollable view perspective, our bounds are reduced when the zoom factor increases
					bounds.size.height /= myData->zoomFactor;
					bounds.size.width /= myData->zoomFactor;

					SetEventParameter(inEvent, kEventParamViewSize, typeHISize, sizeof(bounds.size), &bounds.size);
					SetEventParameter(inEvent, kEventParamImageSize, typeHISize, sizeof(imageSize), &imageSize);
					SetEventParameter(inEvent, kEventParamLineSize, typeHISize, sizeof(lineSize), &lineSize);
					SetEventParameter(inEvent, kEventParamOrigin, typeHIPoint, sizeof(myData->originPoint), &myData->originPoint);
					result = noErr;
					break;
					}

				case kEventScrollableScrollTo:
					{
					// we're being asked to scroll, we just do a sanity check and ask for a redraw if the location is different
					HIPoint where;
					GetEventParameter(inEvent, kEventParamOrigin, typeHIPoint, NULL, sizeof(where), NULL, &where);

					where = SanityCheck(where, myData);

					if ((myData->originPoint.y != where.y) || (myData->originPoint.x != where.x))
						{
						myData->originPoint = where;
					
						HIViewSetNeedsDisplay(myData->view, true);
						}
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
					CGContextRef context;
					result = GetEventParameter(inEvent, kEventParamCGContextRef, typeCGContextRef, NULL, sizeof(context), NULL, &context);

					HIRect bounds;
					HIViewGetBounds(myData->view, &bounds);

					// Setting our background color to light blue
					CGContextSetRGBFillColor(context, 0.8, 1.0, 1.0, 1.0);
					CGContextFillRect(context, bounds);

					// Adjust the transform so the text doesn't draw upside down
					// Look at the HIView documentation for more details but basically:
					//     The HIView coordinate system is set up to match QuickDraw's coordinate system,
					//     origin at the top left, and positive Y going down.
					//     But the Core Graphics operations assume that the origin is botton left
					//     and Positive Y going up.
					//     To let developers draw the "QuickDraw" way, to simplify the porting of their
					//     legacy code, the CGContext we're being passed has already been transformed so that
					//     the Y axis is reversed.
					//     But the Core Graphics Text operations assume that the origin is botton left
					//     and Positive Y going up. If we don't reverse the Text transform (thus cancelling
					//     the previous transformation), then we would get our text upside down.

					CGContextSaveGState(context);
					CGAffineTransform transform = CGAffineTransformIdentity;
					transform = CGAffineTransformScale(transform, 1, -1);
					CGContextSetTextMatrix(context, transform);
					
					// Furthermore, adjust the Context Transformation Matrix to scale and translate
					// to reflect the zooming and scrolling values of the slider and scroll bar controls
					//     OK, this is geometry...
					//     As far as our drawing data is concerned, its size or position never change, its the
					//     "visible" rectangle manipulated by the User which moves and changes size.
					//     Thus, when the User moves this viewable rectangle to, let's say, position 100, 200,
					//     in the coordinate system of our data, in order to draw the "viewable" portion of our data, 
					//     we would have to "move" our data to a position of -100, -200, to draw the correct data
					//     in the coordinate system of the custom view.
					//     
					//     Instead of applying the translation to each and every CG drawing operation
					//     which would give us code like:
					//         CGContextMoveToPoint(context, i - myData->originPoint.x, -myData->originPoint.y);
					//         CGContextAddLineToPoint(context, myData->imageSize.height + i - myData->originPoint.x, myData->imageSize.height - myData->originPoint.y);
					//         CGContextMoveToPoint(context, i - myData->originPoint.x, -myData->originPoint.y);
					//         CGContextAddLineToPoint(context, -myData->originPoint.x, i - myData->originPoint.y);
					//     We apply instead once and for all the translation to the Transformation Matrix
					//     with CGContextTranslateCTM in order to get code like:
					//         CGContextMoveToPoint(context, i, 0);
					//         CGContextAddLineToPoint(context, myData->imageSize.height + i, myData->imageSize.height);
					//         CGContextMoveToPoint(context, i, 0);
					//         CGContextAddLineToPoint(context, 0, i);
					//     And we also apply the scaling once and for all with CGContextScaleCTM for the same reason.
					
					CGContextScaleCTM(context, myData->zoomFactor, myData->zoomFactor);
					CGContextTranslateCTM(context, -myData->originPoint.x, -myData->originPoint.y);

					// Drawing the entire grid, letting the CGClip do the magic
					CGContextBeginPath(context);
					UInt32 i, j;
					for (i = 0; i < myData->imageSize.width; i += 50)
						{
						CGContextMoveToPoint(context, i, 0);
						CGContextAddLineToPoint(context, myData->imageSize.height + i, myData->imageSize.height);
						CGContextMoveToPoint(context, i, 0);
						CGContextAddLineToPoint(context, 0, i);
						}
					for (i = 0; i < myData->imageSize.height; i += 50)
						{
						CGContextMoveToPoint(context, 0, i);
						CGContextAddLineToPoint(context, myData->imageSize.height - i, myData->imageSize.height);
						CGContextMoveToPoint(context, myData->imageSize.width, i);
						CGContextAddLineToPoint(context, myData->imageSize.width - myData->imageSize.height + i, myData->imageSize.height);
						}
					CGContextClosePath(context);
					CGContextSetRGBStrokeColor(context, 0.0, 0.0, 0.0, 1.0);
					CGContextStrokePath(context);
					
					// Drawing __ONLY__ the items which bounding box intersect our bounds (translated and scaled)
					// to optimize the drawing
					// In order to know when our items intersect the bounds of our "viewable" rectangle, we need
					// to apply a reverse transformation on those bounds.
					// The best way to explain this is with an example:
					//     When we zoom in, it's as if the size of our drawing data increases
					//     But in the coordinate system of our drawing data, it's as if the "viewable" rectangle
					//     gets smaller.

					bounds = CGRectOffset(bounds, myData->originPoint.x, myData->originPoint.y);
					bounds.size.width /= myData->zoomFactor;
					bounds.size.height /= myData->zoomFactor;
					
					CGContextSetRGBFillColor(context, 0.0, 0.0, 0.0, 1.0);
					CGContextSelectFont(context, "Helvetica-Bold", 72.0, kCGEncodingMacRoman);

					for (i = 0; i < myData->imageSize.width; i += 100)
						for (j = 0; j < myData->imageSize.height; j += 100)
							{
							HIRect charRect = { {i, j}, {100, 100} };
							if (CGRectIntersectsRect(charRect, bounds))
								{
								char s[] = { 'A' + i / 100 + j / 100 };
								CGContextShowTextAtPoint(context, charRect.origin.x + 10, charRect.origin.y + 90, s, 1);
								}
							}

					// restoring our context the way we found it when we started
					CGContextRestoreGState(context);

					result = noErr;
					break;
					}

				case kEventControlValueFieldChanged:
					{
					// The user moved the zoom slider!
					ControlRef theControl;
					GetEventParameter(inEvent, kEventParamDirectObject, typeControlRef, NULL, sizeof(theControl), NULL, &theControl);

					// Checking the new value
					if (myData->zoomFactor != GetControl32BitValue(theControl))
						{
						myData->zoomFactor = GetControl32BitValue(theControl);
						
						// Let's make sure that we still display valid content
						myData->originPoint = SanityCheck(myData->originPoint, myData);

						// Sending an event to the HIScrollView to let it know its scrollable view has changed
						EventRef theEvent;
						CreateEvent(NULL, kEventClassScrollable, kEventScrollableInfoChanged, GetCurrentEventTime(), kEventAttributeUserEvent, &theEvent);
						SendEventToEventTarget(theEvent, GetControlEventTarget(HIViewGetSuperview(myData->view)));
						ReleaseEvent(theEvent);
						HIViewSetNeedsDisplay(myData->view, true);
						}

					result = eventNotHandledErr;
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

/*****************************************************
*
* GetScrollAndZoomClass(where, myData) 
*
* Purpose:  Registers our custom HIScrollAndZoom view class and installs the appropriate handlers
*
* Inputs:   none
*
* Returns:  our class ID as a CFStringRef
*/
CFStringRef GetScrollAndZoomClass()
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
				{ kEventClassScrollable, kEventScrollableGetInfo },
				{ kEventClassScrollable, kEventScrollableScrollTo },
				{ kEventClassControl, kEventControlValueFieldChanged },
				{ kEventClassControl, kEventControlDraw }
			};
		
		HIObjectRegisterSubclass(kHIScrollAndZoomClass, kHIViewClassID, 0, ScrollAndZoomHandler,
								  GetEventTypeCount(kFactoryEvents), kFactoryEvents, 0, &theClass);
		}
	
	return kHIScrollAndZoomClass;
	}

/*****************************************************
*
* Do_NewWindow(outWindow) 
*
* Purpose:  called to create a new window, each other window will be created from APIs and the other one from Interface Builder
*
* Notes:    called by Handle_CommandProcess() ("File/New" menu item), Handle_OpenApplication(). Handle_ReopenApplication()
*
* Inputs:   outWindow   - if not NULL, the address where to return the WindowRef
*								- if not NULL, the callee will have to ShowWindow
*
* Returns:  OSErr			- error code (0 == no error) 
*/
OSStatus Do_NewWindow(WindowRef * outWindow)
	{
	WindowRef aWindowRef = NULL;
	OSStatus status;
	
	// Create a window, title and position it
	Rect bounds = {0, 0, 360, 480};
	status = CreateNewWindow(kDocumentWindowClass, kWindowStandardDocumentAttributes | kWindowLiveResizeAttribute | kWindowStandardHandlerAttribute | kWindowCompositingAttribute, &bounds, &aWindowRef);
	require_noerr(status, CantCreateWindow);
	require(NULL != aWindowRef, CantCreateWindow);
	
	status = SetWindowTitleWithCFString(aWindowRef, CFSTR("Scroll And Zoom"));
	require_noerr(status, CantCreateWindow);
	
	status = RepositionWindow(aWindowRef, NULL, kWindowCascadeOnMainScreen);
	require_noerr(status, CantCreateWindow);
	
	// Get the Content view
	HIViewRef contentView;
	status = HIViewFindByID(HIViewGetRoot(aWindowRef), kHIViewWindowContentID, &contentView);
	require_noerr(status, CantCreateWindow);
	
	// Create the HIScrollView, the scrollable view (our HIScrollAndZoom custom view), and the zoom slider control
	HIViewRef scrollView;
	status = HIScrollViewCreate(kHIScrollViewValidOptions, &scrollView);
	require_noerr(status, CantCreateScrollView);
	
	HIViewRef scrollAndZoomView;
	status = HIObjectCreate(GetScrollAndZoomClass(), NULL, (HIObjectRef *)&scrollAndZoomView);
	require_noerr(status, CantCreateScrollAndZoomView);
	
	Rect sliderRect = {0, 0, 20, 100};
	HIViewRef slider;
	status = CreateSliderControl(NULL, &sliderRect, 1, 1, 100, kControlSliderDoesNotPoint, 0, true, myLiveSliderAction, &slider);
	require_noerr(status, CantCreateSlider);
	
	// Embed each control in its appropriate parent and make it visible
	status = HIViewAddSubview(scrollView, scrollAndZoomView);
	require_noerr(status, CantSetUpViews);
	status = HIViewSetVisible(scrollAndZoomView, true);
	require_noerr(status, CantSetUpViews);

	status = HIViewAddSubview(contentView, scrollView);
	require_noerr(status, CantSetUpViews);
	status = HIViewSetVisible(scrollView, true);
	require_noerr(status, CantSetUpViews);
	
	status = HIViewAddSubview(contentView, slider);
	require_noerr(status, CantSetUpViews);
	status = HIViewSetVisible(slider, true);
	require_noerr(status, CantSetUpViews);
	
	// Set the frame of each view
	HIRect hiFrame;
	status = HIViewGetBounds(contentView, &hiFrame);
	require_noerr(status, CantSetUpViews);
	hiFrame = CGRectInset(hiFrame, 16, 16);
	status = HIViewSetFrame(scrollView, &hiFrame);
	require_noerr(status, CantSetUpViews);
	
	hiFrame.origin.x += hiFrame.size.width - 130;
	hiFrame.origin.y += hiFrame.size.height - 40;
	hiFrame.size.width = 100;
	hiFrame.size.height = 20;
	status = HIViewSetFrame(slider, &hiFrame);
	require_noerr(status, CantSetUpViews);

	// Set the layout info of each view for automated frame adjustment when resized
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
	status = HIViewSetLayoutInfo(scrollView, &layout);
	require_noerr(status, CantSetUpViews);

	HILayoutInfo layout2 = {
		kHILayoutInfoVersionZero,
		{
			{ NULL, kHILayoutBindNone },
			{ NULL, kHILayoutBindNone },
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
	status = HIViewSetLayoutInfo(slider, &layout2);
	require_noerr(status, CantSetUpViews);
	
	// Install the kEventControlValueFieldChanged handler on the zoom slider control to handle the "live" zooming
	// We pass (ScrollAndZoomData*)HIObjectDynamicCast((HIObjectRef)scrollAndZoomView, kHIScrollAndZoomClass) as inUserData
	// so that the ScrollAndZoomHandler receives the same inUserData as it receives when the other events are received.
	EventTypeSpec eventTypeVC = {kEventClassControl, kEventControlValueFieldChanged};
	InstallEventHandler(GetControlEventTarget(slider), ScrollAndZoomHandler, 1, &eventTypeVC, (ScrollAndZoomData*)HIObjectDynamicCast((HIObjectRef)scrollAndZoomView, kHIScrollAndZoomClass), NULL);
		
	// The window was created hidden so show it if the outWindow parameter is NULL, 
	// if it's not, it will be the responsibility of the caller to show it.
	if (NULL == outWindow)
		ShowWindow(aWindowRef);
	
	SetWindowModified(aWindowRef, false);
	
CantInstallEventHandler:
CantSetTitle:
CantSetUpViews:
CantCreateSlider:
CantCreateScrollAndZoomView:
CantCreateScrollView:
CantCreateWindow:
	
	if (NULL != outWindow)
		*outWindow = aWindowRef;
	
	return status;
	}   // Do_NewWindow
