/*
	File:		HIFleetingControlsView.cp

	Contains:	Custom Fleeting Controls view displaying a temprary set of sub controls like QuickTime Player in full screen view or iPhoto in Slideshow.

	Version:	1.0

	Disclaimer:	IMPORTANT:  This Apple software is supplied to you by Apple, Inc.
				("Apple") in consideration of your agreement to the following terms, and your
				use, installation, modification or redistribution of this Apple software
				constitutes acceptance of these terms.  If you do not agree with these terms,
				please do not use, install, modify or redistribute this Apple software.

				In consideration of your agreement to abide by the following terms, and subject
				to these terms, Apple grants you a personal, non-exclusive license, under AppleÕs
				copyrights in this original Apple software (the "Apple Software"), to use,
				reproduce, modify and redistribute the Apple Software, with or without
				modifications, in source and/or binary forms; provided that if you redistribute
				the Apple Software in its entirety and without modifications, you must retain
				this notice and the following text and disclaimers in all such redistributions of
				the Apple Software.  Neither the name, trademarks, service marks or logos of
				Apple, Inc. may be used to endorse or promote products derived from the
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

	Copyright © 2007 Apple, Inc., All Rights Reserved
*/

//****************************************************
#pragma mark -
#pragma mark * includes & imports *

#include "HIFleetingControlsView.h"

//****************************************************
#pragma mark -
#pragma mark * typedef's, struct's, enums, defines, etc. *

typedef struct
	{
	HIViewRef			view;
	UInt32				alpha;
	EventLoopTimerRef	shouldFadeTimer;
	EventLoopTimerRef	fadingTimer;
	} FleetingControlsViewData;

const UInt32 kVisibleFleetingAlpha = 50;

#define kEventDurationFleetingDelay ((EventTime)(kEventDurationSecond*3))

//****************************************************
#pragma mark -
#pragma mark * local (static) function prototypes *

static OSStatus FleetingControlsViewDrawing(CGContextRef context, const HIRect * bounds, const FleetingControlsViewData * myData);
static pascal OSStatus FleetingControlsViewHandler(EventHandlerCallRef inCaller, EventRef inEvent, void * inRefcon);
static pascal void ShouldFadeTimer(EventLoopTimerRef inTimer, EventLoopIdleTimerMessage inState, void *inUserData);
static pascal void FadingTimer(EventLoopTimerRef inTimer, void *inUserData);

//****************************************************
#pragma mark -
#pragma mark * exported function implementations *

/*
 *  HIFleetingControlsViewCreate(outFleetingControls)
 *  
 *  Summary:
 *    Creates a FleetingControls custom HIView.
 *  
 *  Parameters:
 *    
 *    outFleetingControls:
 *      On exit, contains the new control.
 */
extern OSStatus HIFleetingControlsViewCreate(HIViewRef * outFleetingControls)
	{
	OSStatus status = noErr;

	HIObjectRef hiObject;
	status = HIObjectCreate(GetFleetingControlsViewClass(), NULL, &hiObject);
  	require_noerr(status, HIObjectCreate);

  	*outFleetingControls = (HIViewRef)hiObject;

HIObjectCreate:

	return status;
	}

/*
 *  HIFleetingControlsViewSetHotZone(inFleetingControls, inHotZone)
 *  
 *  Summary:
 *    Associates a FleetingControls with a hotZone HIView.
 */
extern OSStatus HIFleetingControlsViewSetHotZone(HIViewRef inFleetingControls, HIViewRef inHotZone)
	{
	OSStatus status = noErr;

	EventTypeSpec eventTypeTracking[] =
	{
		{ kEventClassControl, kEventControlTrackingAreaEntered },
		{ kEventClassControl, kEventControlTrackingAreaExited }
	};

	FleetingControlsViewData * myData = (FleetingControlsViewData *)HIObjectDynamicCast((HIObjectRef)inFleetingControls, kFleetingControlsViewClassID);
	require_action(myData != NULL, HIObjectDynamicCast, status = coreFoundationUnknownErr);

	status = HIViewInstallEventHandler(inHotZone, FleetingControlsViewHandler, GetEventTypeCount(eventTypeTracking), eventTypeTracking, (void *)myData, NULL);
	require_noerr(status, CantInstallEventHandler);
	
	status = HIViewNewTrackingArea(inHotZone, NULL, 0, NULL);
	require_noerr(status, HIViewNewTrackingArea);

HIViewNewTrackingArea:
CantInstallEventHandler:
HIObjectDynamicCast:

	return status;
	}

/*
 *  HIFleetingControlViewGetAlphaIfEmbedded(embeddedView)
 *  
 *  Summary:
 *    Let a view, embedded in the fleeting controls view, get access to the alpha value necessary to its redrawing.
 *
 *    If the view is not embedded in any fleeting controls view, then an alpha value of 1.0 is returned, which is benign
 *    since this value should be used as a multiplicator and anything multiplied by 1 is unchanged...
 *  
 *  Parameters:
 *    
 *    embeddedView:
 *      The view embedded in the fleeting controls view.
 *
 *    Returns the alpha value as a float.
 */
extern float HIFleetingControlViewGetAlphaIfEmbedded(HIViewRef embeddedView)
{
	float alpha = 1.0;
	HIViewRef view = HIViewGetSuperview(embeddedView);
	
	while (view != NULL)
	{
		if (HIObjectIsOfClass((HIObjectRef)view, kFleetingControlsViewClassID))
		{
			FleetingControlsViewData * myData = (FleetingControlsViewData *)HIObjectDynamicCast((HIObjectRef)view, kFleetingControlsViewClassID);
			require(myData != NULL, HIObjectDynamicCast);

			alpha = myData->alpha / 100.0;
			view = NULL;
		}
		else
		{
			view = HIViewGetSuperview(view);
		}
	}

HIObjectDynamicCast:

	return alpha;
}

/*
 *  GetFleetingControlsViewClass()
 *  
 *  Summary:
 *    Registers and returns the class ID string.
 */
extern CFStringRef GetFleetingControlsViewClass()
	{
	static HIObjectClassRef	theClass;
	
	if (theClass == NULL)
		{
		static EventTypeSpec kFactoryEvents[] =
			{
				{ kEventClassHIObject, kEventHIObjectConstruct },
				{ kEventClassHIObject, kEventHIObjectInitialize },
				{ kEventClassHIObject, kEventHIObjectDestruct },
				{ kEventClassControl, kEventControlInitialize },
				{ kEventClassControl, kEventControlDraw }
			};
		
		HIObjectRegisterSubclass(kFleetingControlsViewClassID, kHIViewClassID, 0, FleetingControlsViewHandler,
								  GetEventTypeCount(kFactoryEvents), kFactoryEvents, 0, &theClass);
		}
	
	return kFleetingControlsViewClassID;
	}

/*****************************************************/
#pragma mark -
#pragma mark * local (static) function implementations *

/*
 *  FleetingControlsViewViewHandler()
 *  
 *  Summary:
 *    Event handler that implements our FleetingControls view.
 */
static pascal OSStatus FleetingControlsViewHandler(EventHandlerCallRef inCaller, EventRef inEvent, void * inRefcon)
	{
	OSStatus status = eventNotHandledErr;
	FleetingControlsViewData * myData = (FleetingControlsViewData *)inRefcon;

	switch (GetEventClass(inEvent))
		{
		case kEventClassHIObject:
			switch (GetEventKind(inEvent))
				{
#pragma mark *   kEventHIObjectConstruct
				case kEventHIObjectConstruct:
					{
					myData = (FleetingControlsViewData *) calloc(1, sizeof(FleetingControlsViewData));
					require(myData != NULL, calloc);
					
					HIViewRef epView;
					status = GetEventParameter(inEvent, kEventParamHIObjectInstance, typeHIObjectRef, NULL, sizeof(epView), NULL, &epView);
					require_noerr(status, GetEventParameter);
					
					myData->view = epView;
					myData->alpha = 0;
					myData->shouldFadeTimer = NULL;
					myData->fadingTimer = NULL;
					
					status = SetEventParameter(inEvent, kEventParamHIObjectInstance, typeVoidPtr, sizeof(myData), &myData);
					require_noerr(status, SetEventParameter);

					break;
					}
					
				case kEventHIObjectDestruct:
					{
					if (myData->fadingTimer != NULL) RemoveEventLoopTimer(myData->fadingTimer);
					if (myData->shouldFadeTimer != NULL) RemoveEventLoopTimer(myData->shouldFadeTimer);
					free(myData);
					status = noErr;
					break;
					}

#pragma mark *   kEventHIObjectInitialize
				case kEventHIObjectInitialize:
					{
					status = CallNextEventHandler(inCaller, inEvent);
					require_noerr(status, CallNextEventHandler);

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
					status = CallNextEventHandler(inCaller, inEvent);
					require_noerr(status, CallNextEventHandler);

					UInt32 features = 0;
					status = GetEventParameter(inEvent, kEventParamControlFeatures, typeUInt32, NULL, sizeof(features), NULL, &features);
					if (status == noErr)
						features |= kHIViewFeatureAllowsSubviews;
					else
						features = kHIViewFeatureAllowsSubviews;

					status = SetEventParameter(inEvent, kEventParamControlFeatures, typeUInt32, sizeof features, &features);
					require_noerr(status, SetEventParameter);

					break;
				}

#pragma mark *   kEventControlDraw
				case kEventControlDraw:
				{
					CGContextRef context;
					status = GetEventParameter(inEvent, kEventParamCGContextRef, typeCGContextRef, NULL, sizeof(context), NULL, &context);
					require_noerr(status, GetEventParameter);

					HIRect bounds;
					status = HIViewGetBounds(myData->view, &bounds);
					require_noerr(status, HIViewGetBounds);
 					
					// now that the proper parameters and configurations have been dealt with, let's draw
					status = FleetingControlsViewDrawing(context, &bounds, myData);
					require_noerr(status, FleetingControlsViewDrawing);

					break;
				}

#pragma mark *   kEventControlTrackingAreaEntered
				case kEventControlTrackingAreaEntered:
				{
					//
					// mouse entered the hot zone, we draw our fleeting controls and we start the idle timer to know when the user is not doing anything.
					//
					status = InstallEventLoopIdleTimer(GetMainEventLoop(), kEventDurationFleetingDelay, 0, ShouldFadeTimer, myData, &myData->shouldFadeTimer);
					require_noerr(status, InstallEventLoopIdleTimer);
					require_action(myData->shouldFadeTimer != NULL, InstallEventLoopIdleTimer, status = coreFoundationUnknownErr);
					
					myData->alpha = kVisibleFleetingAlpha;
					HIViewSetNeedsDisplay(myData->view, true);

					break;
				}
				case kEventControlTrackingAreaExited:
				{
					//
					// mouse exited the hot zone, we hide our fleeting controls and remove the idle timer.
					//
					if (myData->shouldFadeTimer != NULL)
					{
						RemoveEventLoopTimer(myData->shouldFadeTimer);
						myData->shouldFadeTimer = NULL;
					}
					myData->alpha = 0;
					HIViewSetNeedsDisplay(myData->view, true);

					break;
				}
				
				default:
					break;
				}
			break;
			
		default:
			break;
		}

CreatePushButtonControl:
HIViewAddSubview:

InstallEventLoopIdleTimer:

SetEventParameter:
GetEventParameter:
calloc:
CallNextEventHandler:
HIViewGetBounds:
GetWindowBounds:
FleetingControlsViewDrawing:
HIViewFindByID:
InstallControlEventHandler:
GetWindowProperty:
	
	return status;
	}

/*
 *  FleetingControlsViewDrawing(context, bounds, myData)
 *  
 *  Summary:
 *    Draws the contents of the view.
 *  
 *  Parameters:
 *    
 *    context:
 *      The CGContext of our view.
 *    bounds:
 *      The HIRect/CGRect to draw in.
 *    myData:
 *      The private data of our view.
 */
static OSStatus FleetingControlsViewDrawing(CGContextRef context, const HIRect * bounds, const FleetingControlsViewData * myData)
	{
	CGContextSetRGBFillColor(context, 1, 1, 1, (float)myData->alpha / 100.0);
	CGContextFillEllipseInRect(context, *bounds);

	return noErr;
	}

/*
 *  ShouldFadeTimer(EventLoopTimerRef inTimer, EventLoopIdleTimerMessage inState, void *inUserData)
 *  
 *  Summary:
 *    Called after the idle delay has expired or an event occurred.
 *    After examination of the situation (location of the mouse, are we already fading, etc.), we
 *    determine if fading should start, stop, or be left alone.
 *  
 *  Parameters:
 *    
 *    inTimer:
 *      ignored.
 *    inState:
 *      ignored.
 *    inUserData:
 *      The private data of our view.
 */
static pascal void ShouldFadeTimer(EventLoopTimerRef inTimer, EventLoopIdleTimerMessage inState, void *inUserData)
{
	OSStatus status = noErr;
	FleetingControlsViewData * myData = (FleetingControlsViewData *)inUserData;

	// fading already?
	Boolean fadingOrFaded = (myData->alpha != kVisibleFleetingAlpha);
	
	// is the cursor in the HIFleetingControlsView itself?
	Point globalMouse;
	GetGlobalMouse(&globalMouse);
	
	HIPoint localMouse = { globalMouse.h, globalMouse.v };
	HIPointConvert(&localMouse, kHICoordSpace72DPIGlobal, NULL, kHICoordSpaceView, myData->view);

	HIRect bounds;
	HIViewGetBounds(myData->view, &bounds);
	
	Boolean inFleetingControl = CGRectContainsPoint(bounds, localMouse);
	
	// should we fade, stop fading, or do nothing?
	Boolean shouldFade = (!fadingOrFaded) && (!inFleetingControl);
	Boolean shouldReappear = (fadingOrFaded) || (inFleetingControl);
	
	if (shouldFade)
	{
		// we start the fading timer
		status = InstallEventLoopTimer(GetMainEventLoop(), kEventDurationSecond / 25, kEventDurationSecond / 25, FadingTimer, myData, &myData->fadingTimer);
		require_noerr(status, InstallEventLoopTimer);
		require_action(myData->fadingTimer != NULL, InstallEventLoopTimer, status = coreFoundationUnknownErr);
	}
	if (shouldReappear)
	{
		// we stop the fading timer and redraw the fleeting controls
		if (myData->alpha != 0)
		{
			RemoveEventLoopTimer(myData->fadingTimer);
			myData->fadingTimer = NULL;
		}
		myData->alpha = kVisibleFleetingAlpha;
		HIViewSetNeedsDisplay(myData->view, true);
	}

InstallEventLoopTimer:
	;
}

static pascal void FadingTimer(EventLoopTimerRef inTimer, void *inUserData)
{
	FleetingControlsViewData * myData = (FleetingControlsViewData *)inUserData;
	
	// finish fading ?
	if (myData->alpha == 0)
	{
		// removing the fading timer
		RemoveEventLoopTimer(myData->fadingTimer);
		myData->fadingTimer = NULL;
	}
	else
	{
		// decrement alpha and force a redraw of the view and its subviews
		myData->alpha -= 1;
		HIViewSetNeedsDisplay(myData->view, true);
	}
}
