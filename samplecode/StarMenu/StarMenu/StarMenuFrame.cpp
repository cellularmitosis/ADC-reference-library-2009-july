/*
*	File:			StarMenuFrame.cpp of StarMenu
* 
*	Contains:	This file implements a custom subclass of HIView that puts a frame
*					around a star-shaped menu

*					The Frame View creates at least one subview, the Content View.
*					Both the Frame View and Content View have some specific responsibilities they
*					must accept if they are to properly host a Menu View :
*
*						The Frame View must support embedding as it contains at least one other view, the Content View
*						The Frame View creates the Content View in its kEventControlInitialize handler.
*						The ID of the Content View must be kHIViewWindowContentID and the content view must be visible.
*						The Content View must support embedding as it will contain the Menu View
*						The Content View is inserted as a subview of of the Frame View.
*						The Content View handles the kEventControlBoundsChanged and
*							kEventControlOwningWindowChanged events to keep the QuickDraw port of the
*							Content View in the correct place
*						The Content View responds to kEventControlGetFrameMetrics to set the offsets from the
*							Frame View to the contents of the Menu View
*						The Frame View responds to kEventControlBoundsChanged by changing the position of the
*							Content View (taking the Content View's frame metrics into account)
*						The Frame View implements kEventControlDraw to draw menu frame
*	
*	Version:		1.0
*
*	Disclaimer:	IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc.
*					("Apple") in consideration of your agreement to the following terms, and your
*					use, installation, modification or redistribution of this Apple software
*					constitutes acceptance of these terms.  If you do not agree with these terms,
*					please do not use, install, modify or redistribute this Apple software.
*
*					In consideration of your agreement to abide by the following terms, and subject
*					to these terms, Apple grants you a personal, non-exclusive license, under AppleÕs
*					copyrights in this original Apple software (the "Apple Software"), to use,
*					reproduce, modify and redistribute the Apple Software, with or without
*					modifications, in source and/or binary forms; provided that if you redistribute
*					the Apple Software in its entirety and without modifications, you must retain
*					this notice and the following text and disclaimers in all such redistributions of
*					the Apple Software.  Neither the name, trademarks, service marks or logos of
*					Apple Computer, Inc. may be used to endorse or promote products derived from the
*					Apple Software without specific prior written permission from Apple.  Except as
*					expressly stated in this notice, no other rights or licenses, express or implied,
*					are granted by Apple herein, including but not limited to any patent rights that
*					may be infringed by your derivative works or by other works in which the Apple
*					Software may be incorporated.
*
*					The Apple Software is provided by Apple on an "AS IS" basis.  APPLE MAKES NO
*					WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED
*					WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS FOR A PARTICULAR
*					PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND OPERATION ALONE OR IN
*					COMBINATION WITH YOUR PRODUCTS.
*
*					IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL OR
*					CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
*					GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
*					ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION, MODIFICATION AND/OR DISTRIBUTION
*					OF THE APPLE SOFTWARE, HOWEVER CAUSED AND WHETHER UNDER THEORY OF CONTRACT, TORT
*					(INCLUDING NEGLIGENCE), STRICT LIABILITY OR OTHERWISE, EVEN IF APPLE HAS BEEN
*					ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*
*	Copyright:  Copyright © 2005 Apple Computer, Inc, All Rights Reserved
*/

#include "StarMenuFrame.h"

const EventParamName kEventParamStarFrameRadius = 'fRAD';
const unsigned int kFrameOffset = 8;

struct StarFrameData
{
	MenuRef			 menu;
	HIViewRef		 hiSelf;
	EventHandlerRef  rootHandler;
	ThemeMenuType	 menuType;
};

static  HIObjectClassRef gStarFrameClassRef;

// These are the events handled by our custom HIView sublcass
// that implements the star menu frame.
EventTypeSpec gStarFrameHandledEvents[] = {
	{ kEventClassHIObject, kEventHIObjectConstruct } ,
	{ kEventClassHIObject, kEventHIObjectInitialize },
	{ kEventClassHIObject, kEventHIObjectDestruct },

	{ kEventClassControl, kEventControlInitialize },
	{ kEventClassControl, kEventControlBoundsChanged },
	{ kEventClassControl, kEventControlDraw },
	{ kEventClassControl, kEventControlOwningWindowChanged },

	// Mac OS X v10.4 introduced a Window Manager bug.
	// The workaround is to implement the kEventControlGetFrameMetrics handler.
	// Even after the bug is fixed, the workaround will not be harmful.
	{ kEventClassControl, kEventControlGetFrameMetrics }
};

// Our menu frame view has a subview that is the "Content View"
// The Content View, in turn, will own the Menu View that draws the menu
//
// That content view is responsible for a few things... most notably
// it handles the positioning relationship between the frame and the menu
// view (by responding to kEventControlGetFrameMetrics).  It also has to
// establish the size of the menu view when it is added as a child.  Finally,
// it maintains the location of the content QuickDraw port (using
// MovePortTo and PortSize)
EventTypeSpec gContentViewHandledEvents[] = {
	{ kEventClassMenu, kEventMenuBecomeScrollable },
	{ kEventClassMenu, kEventMenuCeaseToBeScrollable },

	{ kEventClassControl, kEventControlAddedSubControl },
	{ kEventClassControl, kEventControlBoundsChanged },
	{ kEventClassControl, kEventControlOwningWindowChanged },
	{ kEventClassControl, kEventControlGetFrameMetrics }
};

static OSStatus StarFrameEventHandler(EventHandlerCallRef inCallRef, EventRef inEvent, void *refcon);
static OSStatus HandleStarFrameObjectEvents(EventHandlerCallRef inCallRef, EventRef inEvent, StarFrameData* frameData);
static OSStatus HandleStarFrameControlEvents(EventHandlerCallRef inCallRef, EventRef inEvent, StarFrameData* frameData);
static OSStatus HandleStarFrameInitialize(EventHandlerCallRef inCallRef, EventRef inEvent, StarFrameData* frameData);
static OSStatus HandleStarFrameBoundsChanged(EventHandlerCallRef inCallRef, EventRef inEvent, StarFrameData* frameData);
static OSStatus PositionContentViewWithMetrics(HIViewRef frame, HIViewRef content);

static OSStatus ContentViewEventHandler(EventHandlerCallRef inCallRef, EventRef inEvent, void *refcon);

static CGPathRef CreatePathForStarFrame(StarFrameData *menuData, float radius);

// The UPP used to handle the events for the content view
static EventHandlerUPP gContentViewHandlerUPP = NewEventHandlerUPP(ContentViewEventHandler);

/* ------------------------------------------------------ CreateStarFrame */
/*
	The public routine exported from this file that creates a star-shaped frame
	view suitable for the given menuType.
*/
OSStatus CreateStarFrame(MenuRef inMenu, ThemeMenuType menuType, HIViewRef *outFrameView)
{
	OSStatus errStatus = noErr;
	static bool isRegistered = false;

	// Register the StarMenuFrame subclass if necessary
	if(!isRegistered) {
		verify_noerr( HIObjectRegisterSubclass (
							kStarFrameClassID,
							kHIViewClassID,
							kNilOptions,
							StarFrameEventHandler,
							GetEventTypeCount( gStarFrameHandledEvents ),
							gStarFrameHandledEvents,
							NULL,
							&gStarFrameClassRef));
		isRegistered = true;
	}

	// Create an instance of the StarMenuFrame
	EventRef initEvent = NULL;
	errStatus = CreateEvent(NULL, kEventClassHIObject, kEventHIObjectInitialize, 0, 0, &initEvent );
	if(noErr == errStatus && initEvent) {

		// store the initialization parameters in the event
		SetEventParameter( initEvent, kEventParamMenuRef, typeMenuRef, sizeof(inMenu), &inMenu);
		SetEventParameter( initEvent, kEventParamMenuType, typeThemeMenuType, sizeof(menuType), &menuType);

		verify_noerr(errStatus = HIObjectCreate(kStarFrameClassID, initEvent, (HIObjectRef *) outFrameView));

		ReleaseEvent(initEvent);
		initEvent = nil;
	}

	return errStatus;
}

/* ------------------------------------------------ StarFrameEventHandler */
/*
	The main event handler for the star-shaped frame.  Mostly this just dispatches
	events to more specialized handler routines.
*/
OSStatus StarFrameEventHandler(
	EventHandlerCallRef inCallRef,
	EventRef inEvent,
	void *inUserData)
{
	OSStatus retVal = eventNotHandledErr;

	switch(GetEventClass(inEvent)) {
		case kEventClassHIObject :
			retVal = HandleStarFrameObjectEvents(inCallRef, inEvent, (StarFrameData *) inUserData );
		break;

		case kEventClassControl :
			retVal = HandleStarFrameControlEvents(inCallRef, inEvent, (StarFrameData *) inUserData );
		break;
	}

	return retVal;
}

/* ----------------------------------------- HandleStarFrameControlEvents */
/*
	Handle events of kEventClassControl that get sent to the Frame
*/
OSStatus HandleStarFrameControlEvents(
	EventHandlerCallRef inCallRef,
	EventRef inEvent,
	StarFrameData* frameData)
{
	OSStatus retVal = eventNotHandledErr;

	switch(GetEventKind(inEvent)) {
		case kEventControlInitialize :
			retVal = HandleStarFrameInitialize(inCallRef, inEvent, frameData);
		break;

		case kEventControlOwningWindowChanged : {
			// We only want the star-shaped opaque area of our frame view to
			// draw.  Everything else should be transparent.  To accomplish that
			// we change the features of the owning window so that only the
			// content we draw shows up on screen
			WindowRef newWindow = GetControlOwner(frameData->hiSelf);
			HIWindowChangeFeatures(newWindow, 0, kWindowIsOpaque);
		} break;

		case kEventControlBoundsChanged : {
			retVal = HandleStarFrameBoundsChanged(inCallRef, inEvent, frameData);
		} break;

        case kEventControlDraw : {
			HIRect			bounds;
			CGContextRef	cgContext;

			HIViewGetBounds(frameData->hiSelf, &bounds);
			float radius = fmin(CGRectGetWidth(bounds) / 2.0, CGRectGetHeight(bounds) / 2.0);

			GetEventParameter(inEvent, kEventParamCGContextRef, typeCGContextRef, NULL, sizeof(cgContext), NULL, &cgContext );
			if(NULL != cgContext) {
				HIThemeMenuDrawInfo drawInfo;
				CGPathRef starPath = CreatePathForStarFrame(frameData, radius);

				drawInfo.version = 0;
				drawInfo.menuType = frameData->menuType;

				// HIThemeDrawMenuBackground is designed to draw the pin striped background
				// of standard menus.  Our menu is a star and so HIThemeDrawMenuBackground may not be
				// appropriate in this case.  Nevertheless, we'll draw the standard menu background for
				// this menu and clip it to a star.
				CGContextClearRect(cgContext, bounds);
				CGContextSaveGState(cgContext);
				CGContextTranslateCTM(cgContext, radius, radius);
				CGContextAddPath(cgContext, starPath);
				CGContextClip(cgContext);
				CGContextTranslateCTM(cgContext, -radius, -radius);
				HIThemeDrawMenuBackground(&bounds, &drawInfo, cgContext, kHIThemeOrientationNormal);
				CGContextRestoreGState(cgContext);

				// The pin striping looks a bit odd sort of floating out by itself.  We'll also add
				// a lovely gray line to help emphasize the boundary
				CGContextTranslateCTM(cgContext, radius, radius);
				CGContextAddPath(cgContext, starPath);
				CGContextSetRGBStrokeColor(cgContext, 0.8, 0.8, 0.8, 1.0);
				CGContextSetLineWidth(cgContext, 1.0);
				CGContextStrokePath(cgContext);
				
				CGPathRelease(starPath);
				starPath = NULL;
			}

			retVal = noErr;
		} break;

		// Mac OS X v10.4 introduced a Window Manager bug.
		// The workaround is to implement the kEventControlGetFrameMetrics handler.
		// Even after the bug is fixed, the workaround will not be harmful.
		case kEventControlGetFrameMetrics: {
			HIViewRef contentView = NULL;

			// If we can find our content view, ask it for our metrics
			verify_noerr(HIViewFindByID(frameData->hiSelf, kHIViewWindowContentID, &contentView));
			if(NULL != contentView) {
				retVal = SendEventToEventTargetWithOptions( inEvent, GetControlEventTarget( contentView ), kEventTargetDontPropagate );
			}
		} break;

		default:
		break;
	}

	return retVal;
}

/* -------------------------------------------- HandleStarFrameInitialize */
/*
	Handle the initialization event for a star-shaped frame.  This routine
	creates the content view, installs its event handler, and sets its
	initial location.
*/
OSStatus HandleStarFrameInitialize(
	EventHandlerCallRef inCallRef,
	EventRef inEvent,
	StarFrameData *frameData)
{
	OSStatus retVal = eventNotHandledErr;
	HIViewRef contentView;

	// The frame view must support embedding
	HIViewChangeFeatures(frameData->hiSelf, kHIViewAllowsSubviews, kHIViewIsOpaque);

	// Create the content view for the frame.
	retVal = HIObjectCreate(kHIViewClassID, NULL, (HIObjectRef *)&contentView);
	if(noErr == retVal && NULL != contentView) {
		HIViewChangeFeatures(contentView, kHIViewAllowsSubviews, kHIViewIsOpaque);
		SetControlID(contentView, &kHIViewWindowContentID);

		// content view must be visible.
		verify_noerr(HIViewSetVisible(contentView, true));

		// Make sure the content view is handling events
		verify_noerr(InstallControlEventHandler(
								contentView,
								gContentViewHandlerUPP,
								GetEventTypeCount(gContentViewHandledEvents),
								gContentViewHandledEvents,
								NULL,
								NULL));

		// set the location of the content view appropriately
		PositionContentViewWithMetrics(frameData->hiSelf, contentView);

		// add the content view as a subview of me.
		verify_noerr(HIViewAddSubview(frameData->hiSelf, contentView));

		retVal = noErr;
	}

	return retVal;
}

/* ----------------------------------------- HandleStarFrameBoundsChanged */
/*
	Ask the content view for its frame metrics then position the content
	view based on those metrics.
*/
OSStatus HandleStarFrameBoundsChanged(
	EventHandlerCallRef inCallRef,
	EventRef inEvent,
	StarFrameData* frameData)
{
	OSStatus retVal = eventNotHandledErr;
	HIViewRef contentView = NULL;

	// If we can find our content view change it to match the new bounds
	verify_noerr(HIViewFindByID(frameData->hiSelf, kHIViewWindowContentID, &contentView));
	if(NULL != contentView) {
		retVal = PositionContentViewWithMetrics(frameData->hiSelf, contentView);
	}

	return retVal;
}

/* ------------------------------------------- PositionContentViewWithMetrics */
/*
	Position the contentView (presumed to be a subview of the frameView) by
	asking the content view for its metrics and positioning it appropriately.
*/
OSStatus PositionContentViewWithMetrics(HIViewRef frameView, HIViewRef contentView)
{
	HIViewFrameMetrics metrics = { 0, 0, 0, 0 };
	EventRef getMetricsEvent = NULL;

	// First we check the frame metrics of the content view by asking it (politely) for the
	// metrics it wants
	verify_noerr(CreateEvent(NULL, kEventClassControl, kEventControlGetFrameMetrics, GetCurrentEventTime(), 0, &getMetricsEvent));
	if(NULL != getMetricsEvent) {
		SetEventParameter(getMetricsEvent, kEventParamDirectObject, typeControlRef, sizeof(contentView), &contentView);
		SetEventParameter(getMetricsEvent, kEventParamControlFrameMetrics, typeControlFrameMetrics, sizeof(metrics), &metrics);

		OSStatus result = SendEventToEventTarget(getMetricsEvent, HIObjectGetEventTarget((HIObjectRef)contentView));
		if(result == noErr) {
			verify_noerr(GetEventParameter(getMetricsEvent, kEventParamControlFrameMetrics, typeControlFrameMetrics, NULL, sizeof(metrics), NULL, &metrics));
		}

		ReleaseEvent(getMetricsEvent);
		getMetricsEvent = NULL;
	}

	// Now we reposition the content view based on the metrics we got from it.
	HIRect bounds, contentRect;
	HIViewGetBounds(frameView, &bounds);

	contentRect.origin.x = metrics.left;
	contentRect.origin.y = metrics.top;
	contentRect.size.width = bounds.size.width - (metrics.left + metrics.right);
	contentRect.size.height = bounds.size.height - (metrics.top + metrics.bottom);

	HIViewSetFrame(contentView, &contentRect);

	return noErr;
}

/* ------------------------------------------ HandleStarFrameObjectEvents */
/*
	Handle the messages necessary for creating the custom StarMenuFame
	class.
*/
OSStatus HandleStarFrameObjectEvents(
	EventHandlerCallRef inCallRef,
	EventRef inEvent,
	StarFrameData* frameData)
{
	OSStatus err = eventNotHandledErr;

	switch ( GetEventKind( inEvent ) ) {
		case kEventHIObjectConstruct:
			frameData = (StarFrameData *) calloc(1, sizeof(StarFrameData));
			frameData->menu = NULL;

			GetEventParameter(inEvent, kEventParamHIObjectInstance, typeHIObjectRef, NULL, sizeof( frameData->hiSelf ), NULL, &frameData->hiSelf );

			// This important step actually associates our frameData with the view being initialized
			SetEventParameter(inEvent, kEventParamHIObjectInstance, typeVoidPtr, sizeof( frameData ), &frameData );

			err = noErr;
		break;

		case kEventHIObjectInitialize:
			err = CallNextEventHandler( inCallRef, inEvent );
			if ( err == noErr ) {
				GetEventParameter(inEvent, kEventParamMenuRef, typeMenuRef, NULL, sizeof(frameData->menu), NULL, &frameData->menu);
				GetEventParameter(inEvent, kEventParamMenuType, typeThemeMenuType, NULL, sizeof( frameData->menuType ), NULL, &frameData->menuType );
			}
			err = noErr;
		break;

		case kEventHIObjectDestruct:
			free( (void*) frameData );
			err = noErr;
		break;

		default:
		break;
	}

	return err;
}


/* -------------------------------------------------- ContentViewEventHandler */
/*
	Event handler for the content view that gets attached to the menu frame.

	The content view will (eventually) contain the menu view.
*/
OSStatus ContentViewEventHandler(
	EventHandlerCallRef inCallRef,
	EventRef inEvent,
	void *refcon)
{
	OSStatus retVal = eventNotHandledErr;
	if(GetEventClass(inEvent) == kEventClassMenu) {
		return noErr;
	} else
	if(GetEventClass(inEvent) == kEventClassControl) {
		HIViewRef hiSelf = NULL;
		verify_noerr(GetEventParameter(inEvent, kEventParamDirectObject, typeControlRef, NULL, sizeof(hiSelf), NULL, &hiSelf));

		if(hiSelf) {
			HIRect frame;
			HIViewGetFrame(hiSelf, &frame);

			switch(GetEventKind(inEvent)) {
				case kEventControlAddedSubControl : {
					HIViewRef subControl;
					ControlID subControlID;

					GetEventParameter(inEvent, kEventParamControlSubControl, typeControlRef, NULL, sizeof(subControl), NULL, &subControl );
					GetControlID(subControl, &subControlID);

					// This should be comparing against kHIViewMenuContentID as shown inside the
					// #if 0. At the time of this writing, however, using that constant causes a
					// linker error (and a crash if you use ZeroLink).  I extracted the signature
					// and id by determining the value at run-time the value I compare against.
#if 0
					if( kHIViewMenuContentID.signature == subControlID.signature && kHIViewMenuContentID.id == subControlID.id ) {
#else
					if( 'menu' == subControlID.signature && 0 == subControlID.id ) {
#endif
						// If we have the menu content view then set up some view bindings for it.
						HIRect bounds;
						HIViewGetBounds(hiSelf, &bounds);
						HIViewSetFrame(subControl, &bounds);

						HILayoutInfo contentLayout = {
							kHILayoutInfoVersionZero,
							{
								{ NULL, kHILayoutBindTop },
								{ NULL, kHILayoutBindLeft },
								{ NULL, kHILayoutBindBottom },
								{ NULL, kHILayoutBindRight }
							},
							{
								{ NULL, kHILayoutScaleAbsolute, 0 },
								{ NULL, kHILayoutScaleAbsolute, 0 }
							},
							{
								{ NULL, kHILayoutPositionTop, 0 },
								{ NULL, kHILayoutPositionLeft, 0 }
							}
						};

						verify_noerr(HIViewSetLayoutInfo(subControl, &contentLayout));
					}

					retVal = noErr;
				} break;

				case kEventControlGetFrameMetrics :
					HIViewFrameMetrics metrics;

					// The offset from the frame view to the content view is 
					// given by the kFrameOffset constant
					metrics.top = kFrameOffset;
					metrics.left = kFrameOffset;
					metrics.right = kFrameOffset;
					metrics.bottom = kFrameOffset;

					verify_noerr(SetEventParameter(inEvent, kEventParamControlFrameMetrics, typeControlFrameMetrics, sizeof(metrics), &metrics));

					retVal = noErr;
				break;

				case kEventControlBoundsChanged :
				case kEventControlOwningWindowChanged : {
					// Maintain the QuickDraw port by changing its position to
					// match that of the content view.
					CGrafPtr windowPort = NULL;
					WindowRef window = GetControlOwner(hiSelf);

					if(window && (windowPort = GetWindowPort(window))) {
						CGrafPtr savePort;
						bool swapped = QDSwapPort(windowPort, &savePort);

						MovePortTo((short) frame.origin.x, (short) frame.origin.y);
						PortSize((short) frame.size.width, (short) frame.size.height);

						if(swapped) {
							QDSwapPort(savePort, NULL);
						}
					}

					retVal = noErr;
				} break;
			} // switch
		} // if (hiSelf)
	}

	return retVal;
}


/* ------------------------------------------ CreatePathForEntireStarMenu */
/*
	Create a path shape for the star frame.
	
	This looks an awful lot like CreatePathForEntireStarMenu in
	StarMenu.cpp but takes the radius to use as a parameter and
	then takes into account the kFrameOffest when creating the path.

	In true Core Foundation style, this is a CreateXXX routine and the
	caller is responsible for freeing the path that is returned.
*/
CGPathRef CreatePathForStarFrame(StarFrameData *menuData, float radius)
{
   CGMutablePathRef retVal = CGPathCreateMutable();
   MenuItemIndex numItems = CountMenuItems(menuData->menu);

   if(numItems > 0) {
	  const CGPoint fullRadiusPoint = { radius, 0 };
	  const CGPoint halfRadiusPoint = { ((radius - kFrameOffset) / 2.0) + kFrameOffset , 0 };

	  float   anglePerItem = 2 * pi / (float)numItems;   // in radians naturally
	  float   halfAngle = anglePerItem / 2.0;

	  CGPoint startPoint = halfRadiusPoint;
	  CGAffineTransform midRotate = CGAffineTransformMakeRotation(halfAngle);
	  CGPoint midPoint = CGPointApplyAffineTransform(fullRadiusPoint, midRotate);

	  CGAffineTransform rotateToNext = CGAffineTransformMakeRotation(anglePerItem);

	  CGPathMoveToPoint(retVal, NULL, startPoint.x, startPoint.y);
	  CGPathAddLineToPoint(retVal, NULL, midPoint.x, midPoint.y);

	  for(short ctr = 0; ctr < numItems; ctr++) {
		 startPoint = CGPointApplyAffineTransform(startPoint, rotateToNext);
		 midPoint = CGPointApplyAffineTransform(midPoint, rotateToNext);

		 CGPathAddLineToPoint(retVal, NULL, startPoint.x, startPoint.y);
		 CGPathAddLineToPoint(retVal, NULL, midPoint.x, midPoint.y);
	  }

	  CGPathCloseSubpath(retVal);
   }

   return retVal;
}
