/*
*	File:			StarMenu.cpp of StarMenu
* 
*	Contains:	This file contains the implementation for a custom subclass (in the
*					HIObject sense) of kHIMenuViewClassID.  The subclass draws a lovely
*					star shaped menu and allows the user to select a color from it.
*
*					This file also contains implementations of the utility routines for
*					changing the items in the menu as declared in StarMenu.h
*
*					Of particular interest is the handling of the kEventMenuCreateFrameView
*					event which creates a custom frame view for our star-shaped menu.  That way
*					we don't end up with a stellar menu in a square frame.
*	
*	Version:		1.0.1
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

#include "StarMenu.h"
#include "StarMenuFrame.h"

const EventParamName kEventParamStarMenuRadius = 'RAD ';
const OSType kStarMenuCreator  = 'CrcM';
const OSType kStarMenuColor  = 'Colr';

// HIView instance data.
struct StarMenuData
{
	MenuRef				menu;
	HIViewRef			hiSelf;
	EventHandlerRef		rootHandler;
	float				radius;
};

// These are the events handled by the Menu View.
static EventTypeSpec gStarMenuHandledEvents[] =
{
	{ kEventClassHIObject, kEventHIObjectConstruct } ,
	{ kEventClassHIObject, kEventHIObjectInitialize },
	{ kEventClassHIObject, kEventHIObjectDestruct },

	{ kEventClassControl, kEventControlHitTest },
	{ kEventClassControl, kEventControlGetPartRegion },
	{ kEventClassControl, kEventControlDraw },
	{ kEventClassControl, kEventControlGetOptimalBounds },

	{ kEventClassMenu, kEventMenuCreateFrameView },

	{ kEventClassMenu, kEventMenuBecomeScrollable },
	{ kEventClassMenu, kEventMenuCeaseToBeScrollable },

	// we don't know that scrollable is strictly necesssary
	// in this case but we'll add it anyway
	{ kEventClassScrollable, kEventScrollableGetInfo },
};

static  HIObjectClassRef gStarMenuClassRef = NULL;

static OSStatus StarMenuEventHandler(EventHandlerCallRef inCallRef, EventRef inEvent, void *refcon);
static OSStatus HandleObjectEvents(EventHandlerCallRef inCallRef, EventRef inEvent, StarMenuData* data);
static OSStatus HandleControlEvents(EventHandlerCallRef inCallRef, EventRef inEvent, StarMenuData* data);
static OSStatus HandleMenuEvents(EventHandlerCallRef inCallRef, EventRef inEvent, StarMenuData* data);
static OSStatus HandleScrollableEvents(EventHandlerCallRef inCallRef,  EventRef event, StarMenuData* menuData );

static void DrawStarMenu(EventRef drawEvent, StarMenuData *menuData);
static bool GetStarMenuPartRegion( EventRef inEvent, StarMenuData* data);
static void HitTestStarMenu( EventRef inEvent, StarMenuData* data);
static CGPathRef CreatePathForStarMenuItem(StarMenuData *menuData, MenuItemIndex whichItem);
static CGPathRef CreatePathForEntireStarMenu(StarMenuData *menuData);


/* ------------------------------------------------------- CreateStarMenu */
/*
	Public routine for creating an instance of our star-shaped menu
*/
OSStatus CreateStarMenu(MenuID inMenuID, float radius, MenuRef *outMenu)
{
	EventRef	initEvent = NULL;
	OSStatus	errStatus = noErr;

	// If my custom menu view type is not yet registered then
	// register it as a sublcass of MenuView (kHIMenuViewClassID)
	static bool isClassRegistered = false;
	if(!isClassRegistered) {
		verify_noerr( HIObjectRegisterSubclass (
							kStarMenuClassID,
							kHIMenuViewClassID,
							kNilOptions,
							StarMenuEventHandler,
							GetEventTypeCount( gStarMenuHandledEvents ),
							gStarMenuHandledEvents,
							NULL,
							&gStarMenuClassRef));

		isClassRegistered = true;
	}

	// We're going to create an instance of our custom menu type.  To do that we
	// need to tell the system how to create one of our menus.  We do that with an
	// MenuDefSpec.

	// An HIObject based menu view needs to have an initialization event
	errStatus = CreateEvent( NULL, kEventClassHIObject, kEventHIObjectInitialize, 0, 0, &initEvent );
	if(noErr == errStatus && initEvent) {
		MenuDefSpec customMenuDef;

		// Pass the init parameters that are specific to our class.  The operating system
		// will add it's own initialization parameters in CreateCustomMenu
		SetEventParameter(initEvent, kEventParamStarMenuRadius, typeFloat, sizeof(radius), &radius);

		// setup the menu def function specification that we'll pass to CreateCustomMenu
		customMenuDef.defType = kMenuDefClassID;
		customMenuDef.u.view.classID = kStarMenuClassID;
		customMenuDef.u.view.initEvent = initEvent;

		// Finally, we can create the menu
		errStatus = CreateCustomMenu( &customMenuDef, inMenuID, 0, outMenu );

		// We're done with our custom event
		ReleaseEvent(initEvent);
		initEvent = nil;
	}

	return errStatus;
}


/* ------------------------------------------------- SetStarMenuItemColor */
/*
	Public routine that establishes the color of a given menu item
*/
OSStatus SetStarMenuItemColor(
	MenuRef starMenu,
	MenuItemIndex whichItem,
	const RGBColor newColor)
{
	return SetMenuItemProperty( starMenu, whichItem, kStarMenuCreator, kStarMenuColor, sizeof(RGBColor), &newColor);
}


/* ------------------------------------------------- GetStarMenuItemColor */
/*
	Public routine that returns the color of a given menu item
*/
OSStatus GetStarMenuItemColor(
	MenuRef starMenu,
	MenuItemIndex whichItem,
	RGBColor *outItemColor)
{
	return GetMenuItemProperty(starMenu, whichItem, kStarMenuCreator, kStarMenuColor, sizeof(RGBColor), NULL, outItemColor);
}


/* ------------------------------------------------- StarMenuEventHandler */
/*
	Main event handler that defines the behavior of the star menu view.

	This is little more than a dispatcher to more specialized handlers
*/
OSStatus StarMenuEventHandler(EventHandlerCallRef inCallRef, EventRef inEvent, void *inUserData)
{
	OSStatus retVal = eventNotHandledErr;

	switch(GetEventClass(inEvent)) {
		case kEventClassHIObject :
			retVal = HandleObjectEvents(inCallRef, inEvent, (StarMenuData *) inUserData );
		break;

		case kEventClassControl :
			retVal = HandleControlEvents(inCallRef, inEvent, (StarMenuData *) inUserData );
		break;

		case kEventClassMenu :
			retVal = HandleMenuEvents(inCallRef, inEvent, (StarMenuData *) inUserData);
		break;

		case kEventClassScrollable :
			retVal = HandleScrollableEvents(inCallRef, inEvent, (StarMenuData *) inUserData);
		break;
	}

	return retVal;
}


/* ------------------------------------------------------ HandleControlEvents */
/*
	Handle controls of kEventClassControl that are passed to our Menu view
*/
OSStatus HandleControlEvents(EventHandlerCallRef inCallRef, EventRef inEvent, StarMenuData* data)
{
	OSStatus retVal = eventNotHandledErr;

	switch(GetEventKind(inEvent)) {
		  case kEventControlInitialize :
				retVal = CallNextEventHandler( inCallRef, inEvent );
		  break;

		  case kEventControlHitTest :
				HitTestStarMenu(inEvent, data);
				retVal = noErr;
		  break;

		  case kEventControlGetPartRegion :
				if(GetStarMenuPartRegion(inEvent, data)) {
					 retVal = noErr;
				}
		  break;

		  case kEventControlDraw :
				DrawStarMenu(inEvent, data);
				retVal = noErr;
		  break;

		  case kEventControlGetOptimalBounds : {
				HIRect bounds;

				bounds.origin.x = bounds.origin.y = 0;
				bounds.size.width = bounds.size.height = 2 * data->radius;

				SetEventParameter(inEvent, kEventParamControlOptimalBounds, typeHIRect, sizeof( bounds ), &bounds );
				retVal = noErr;
		  } break;

		default:
		break;
	}

	return retVal;
}


/* ------------------------------------------------------- HandleObjectEvents */
/*
	Handle the Carbon Event messages necessary to implement a custom subclass
	of some HIObject-based class.

	<file:///Developer/Documentation/Carbon/Reference/HIObjectReference/index.html>
*/
OSStatus HandleObjectEvents(
	EventHandlerCallRef inCallRef,
	EventRef inEvent,
	StarMenuData* menuData)
{
	OSStatus err = eventNotHandledErr;

	switch ( GetEventKind( inEvent ) ) {
		case kEventHIObjectConstruct:
			menuData = (StarMenuData *) calloc(1, sizeof(StarMenuData));
			menuData->menu = NULL;

			// save off a pointer to myself as an HIViewRef
			GetEventParameter(inEvent, kEventParamHIObjectInstance, typeHIObjectRef, NULL, sizeof( menuData->hiSelf ), NULL, &menuData->hiSelf );

			// This important step actually associates our data with the HIObject that is being created
			SetEventParameter(inEvent, kEventParamHIObjectInstance, typeVoidPtr, sizeof( menuData ), &menuData );

			err = noErr;
		break;

		case kEventHIObjectInitialize:
			// extract the parameters passed in the initialization events and
			// save them into our instance data.
			err = CallNextEventHandler( inCallRef, inEvent );
			if ( err == noErr ) {
				// The kEventParamMenuRef is placed in the initialization event by the operating system
				// when creating an instance of kHIMenuClassID.
				GetEventParameter(inEvent, kEventParamMenuRef, typeMenuRef, NULL, sizeof(menuData->menu), NULL, &menuData->menu);

				// This is the parameter that we added to the event in CreateStarMenu
				GetEventParameter(inEvent, kEventParamStarMenuRadius, typeFloat, NULL, sizeof(menuData->radius), NULL, &menuData->radius);
			}
		break;

		case kEventHIObjectDestruct:
			free( (void*) menuData );
			err = noErr;
		break;

		/* No need to override the "kEventHIObjectIsEqual" event... the default behavior should work fine */
		/* We also ignore the "kEventHIObjectPrintDebugInfo" event... we're evil!" */

		default:
		break;
	}

	return err;
}


/* --------------------------------------------------------- HandleMenuEvents */
/*
	Handle the kEventMenuCreateFrameView event to create a custom
	menu frame.
*/
OSStatus HandleMenuEvents(
	EventHandlerCallRef inCallRef,
	EventRef inEvent,
	StarMenuData* data)
{
	OSStatus retVal = noErr;

	switch(GetEventKind(inEvent)) {
		// This handler creates our custom star frame for our
		// stellar menu.
		case kEventMenuBecomeScrollable :
		case kEventMenuCeaseToBeScrollable :
			break;
		case kEventMenuCreateFrameView : {
			ThemeMenuType menuType;
			HIViewRef   frameView;

			verify_noerr(GetEventParameter(inEvent, kEventParamMenuType, typeThemeMenuType, NULL, sizeof(menuType), NULL, &menuType));

			retVal = CreateStarFrame(data->menu, menuType, &frameView);
			if(noErr == retVal && frameView) {
				verify_noerr(SetEventParameter (inEvent, kEventParamMenuFrameView, typeControlRef, sizeof(frameView), &frameView));
				retVal = noErr;
			}
		} break;
	}

	return retVal;
}


/* --------------------------------------------------- HandleScrollableEvents */
/*
	Our menu view should impelement the scrollable protocol as a matter of
	course, but that may not make sense for our star menu.  As a result,
	this is a pretty bare-bones implementation of the scrollable protocol.
*/
OSStatus HandleScrollableEvents(EventHandlerCallRef inCallref,  EventRef event, StarMenuData* menuData )
{
	OSStatus err = eventNotHandledErr;

	switch (GetEventKind(event)) {
		case kEventScrollableGetInfo: {
			HISize size;
			HIPoint origin = { 0, 0 };

			size.width = 2 * menuData->radius;
			size.height = 2 * menuData->radius;

			SetEventParameter(event, kEventParamImageSize, typeHISize, sizeof( size ), &size );
			SetEventParameter(event, kEventParamViewSize, typeHISize, sizeof( size ), &size );
			SetEventParameter(event, kEventParamOrigin, typeHIPoint, sizeof( origin ), &origin );

			// line size is 1/10th total size
			size.width /= 10;
			size.height /= 10;

			SetEventParameter(event, kEventParamLineSize, typeHISize, sizeof( size ), &size );

			err = noErr;
		} break;

		default:
		break;
	}

	return err;
}


/* --------------------------------------------------------- DrawStarMenu */
/*
	Implementation routine that draws the lovely star menu content
	with Core Graphics.
*/
void DrawStarMenu(EventRef drawEvent, StarMenuData *menuData)
{
	HIViewPartCode	focusPart;
	CGContextRef cgContext;
	RgnHandle rgnToDraw;

	GetEventParameter(drawEvent, kEventParamCGContextRef, typeCGContextRef, NULL, sizeof(cgContext), NULL, &cgContext );
	GetEventParameter(drawEvent, kEventParamRgnHandle, typeQDRgnHandle, NULL, sizeof(rgnToDraw), NULL, &rgnToDraw );

	CGContextSaveGState(cgContext);

	// Flip the coordinate system back over the way God and Euclid intended it to be.
	CGContextTranslateCTM(cgContext, 0, 2 * menuData->radius);
	CGContextScaleCTM(cgContext, 1.0, -1.0);

	// translate the origin of the context to the center of our star;
	CGContextTranslateCTM(cgContext, menuData->radius, menuData->radius);

	// We don't try to optimize the drawing of the menu contents very much.
	// Better behaved code would use the rgnToDraw in order to draw only those
	// items that needed updating.
	//
	// You should do better in your own code :-)
	MenuItemIndex numItems = CountMenuItems(menuData->menu);
	if(numItems > 0) {
		for(short ctr = 1; ctr <= numItems; ctr++) {
			RGBColor itemColor;

			CGPathRef itemPath = CreatePathForStarMenuItem(menuData, ctr);
			GetStarMenuItemColor(menuData->menu, ctr, &itemColor);

			CGContextAddPath(cgContext, itemPath);
			CGPathRelease(itemPath);
			itemPath = NULL;

			CGContextSetRGBFillColor( cgContext,
				(float) itemColor.red / 65535.0,
				(float) itemColor.green / 65535.0,
				(float) itemColor.blue / 65535.0, 1.0);
			CGContextFillPath(cgContext);
		}
	}

	 // Draw a nice line around the star.
   CGPathRef entirePath = CreatePathForEntireStarMenu(menuData);
   CGContextAddPath(cgContext, entirePath);
   CGContextSetRGBStrokeColor( cgContext, 0.5, 0.5, 0.5, 1.0);
   CGContextSetLineWidth( cgContext, 0.5);
   CGContextStrokePath(cgContext);

	// Draw a little arrow to show which item is currently selected
	HIViewGetFocusPart( menuData->hiSelf, &focusPart );
	if(focusPart) {
		float anglePerItem = 2 * pi / (float)numItems;   // in radians naturally
		float startAngle = (anglePerItem * (focusPart - 1));
		float endAngle = (anglePerItem * focusPart) ;
		float midAngle = (startAngle + endAngle) / 2;

		// pick some values for the arrow that look groovy.
		float farRadius = (3.0 * menuData->radius) / 4.0;
		float nearRadius = (3.0 * farRadius) / 5.0;

		CGContextSaveGState(cgContext);
		  // It's much easier to rotate the context instead of trying to rotate
		  // the points that make up the arrow... Core Graphics is Cool!
		  CGContextRotateCTM(cgContext, midAngle);
		  CGContextMoveToPoint(cgContext, 0, 0);
		  CGContextAddLineToPoint(cgContext, nearRadius, -nearRadius * sin(anglePerItem / 2) * 0.75);
		  CGContextAddLineToPoint(cgContext, farRadius, 0);
		  CGContextAddLineToPoint(cgContext, nearRadius, nearRadius * sin(anglePerItem / 2) * 0.75);
		  CGContextClosePath(cgContext);

		  CGContextSetRGBFillColor(cgContext, 1.0, 1.0, 1.0, 0.8);
		  CGContextSetRGBStrokeColor(cgContext, 0, 0, 0, 1.0);
		  CGContextDrawPath(cgContext, kCGPathFillStroke);
		CGContextRestoreGState(cgContext);
	}
}


/* ------------------------------------------------ GetStarMenuPartRegion */
/*
	We want to generate regions for our wedges, but regions are a
	QuickDraw artifact and our wedges are drawn with Core Graphics.

	Unfortunately it seems that the the only way to get an accurate region
	from a Core Graphics um... graphic is to draw the Graphic then use
	BitMapToRegion.

	Moreover it is unfortunate that CoreGraphics doesn't like to draw
	in a 1 Bit-per-pixel GWorld.  So we draw the Core Graphics wedge
	in full 16 bit color then downsample the pixmap to create something
	suitable for BitMapToRegion
*/
bool GetStarMenuPartRegion( EventRef inEvent, StarMenuData *menuData)
{
	Rect qdRect;
	bool retVal = false;
	RgnHandle outRegion = NULL;
	GWorldPtr offscreenWorld = NULL;
	ControlPartCode whichItem;

	MenuItemIndex numItems = CountMenuItems(menuData->menu);
	if(numItems == 0) {
		return false;
	}

	verify_noerr(GetEventParameter(inEvent, kEventParamControlPart, typeControlPartCode, NULL, sizeof(whichItem), NULL, &whichItem));
	verify_noerr(GetEventParameter(inEvent, kEventParamControlRegion, typeQDRgnHandle, NULL, sizeof(outRegion), NULL, &outRegion));

	// for all the meta part codes we return the region for the entire control
	if(whichItem < 0) {
		whichItem = kControlEntireControl;
	}

	SetRect(&qdRect, 0, 0, (short)(menuData->radius * 2), (short)(menuData->radius * 2));

	// Create the GWorld at 8 bits per pixel - grayLevels - for Core Graphics.
	// CoreGraphics uses the gray levels 0...255 in the opposite direction as QD;
	// so we need to fill with black (instead of erase) and invert the result, before
	// processing it into a region.
	OSStatus errStatus = NewGWorld(&offscreenWorld, 8, &qdRect, GetCTable(32 + 8), NULL, 0);
	if(noErr == errStatus && nil != offscreenWorld) {
		GWorldPtr savePort;
		bool swapped = QDSwapPort(offscreenWorld, &savePort);
		PaintRect(&qdRect);

		CGContextRef cgContext = NULL;
		errStatus = QDBeginCGContext(offscreenWorld, &cgContext);
		if(noErr == errStatus && NULL != cgContext) {
			CGContextTranslateCTM(cgContext, menuData->radius, menuData->radius);

			// Create the path for the item we're interested in
			// and make it the current path in the context
			CGPathRef itemPath = NULL;
			if(whichItem != kControlEntireControl) {
			   itemPath = CreatePathForStarMenuItem(menuData, whichItem);
			} else {
			   itemPath = CreatePathForEntireStarMenu(menuData);
			}

			CGContextAddPath(cgContext, itemPath);
			CGPathRelease(itemPath);
			itemPath = NULL;

			// We add a bit of slop to our region by adding a stroke
			// to the path that generates it.
			CGContextSetRGBFillColor(cgContext, 0, 0, 0, 1.0);
			CGContextSetLineWidth(cgContext, 2.0);
			CGContextSetRGBStrokeColor(cgContext, 0, 0, 0, 1.0);
			CGContextDrawPath(cgContext, kCGPathFillStroke);

			QDEndCGContext(offscreenWorld, &cgContext);
			
			// Now bring the gray levels back to the QD convention
			InvertRect(&qdRect);

			// Downsample the image to 1 bit-per-pixel
			UpdateGWorld(&offscreenWorld, 1, &qdRect, NULL, NULL, clipPix);

			// Extract the pixmap from the port and the (hopefully) star-shaped region
			// from the pixmap.
			PixMapHandle portPixMap = GetPortPixMap(offscreenWorld);
			if(LockPixels(portPixMap)) {
				verify_noerr(BitMapToRegion(outRegion, (BitMap *) *portPixMap));
				retVal = true;
				UnlockPixels(portPixMap);
			}
		}

		if(swapped)
			QDSwapPort(savePort, NULL);

		DisposeGWorld(offscreenWorld);
		offscreenWorld = NULL;
	}

	return retVal;
}


/* ------------------------------------------------------ HitTestStarMenu */
/*
	Remember all that trigonometry you learned?  This is why you learned it :-)
*/
void HitTestStarMenu( EventRef inEvent, StarMenuData* menuData)
{
	HIPoint mouseLoc;
	ControlPartCode partHit = kControlNoPart;

	MenuItemIndex numItems = CountMenuItems(menuData->menu);
	if(numItems) {
		float anglePerItem = 2 * pi / (float)numItems;   // in radians naturally

		verify_noerr(GetEventParameter(inEvent, kEventParamMouseLocation, typeHIPoint, NULL, sizeof( mouseLoc ), NULL, &mouseLoc ) );

		// Calculate a vector from the center of our view to the mouse location.
		// Because the mouse point is in flipped coordinates this calculation looks a bit off,
		// but is technically correct.  Check to see if the mouse is within our star radius
		HIPoint vectorToMouse = { mouseLoc.x - menuData->radius, menuData->radius - mouseLoc.y};
		bool inCircle = (vectorToMouse.x * vectorToMouse.x + vectorToMouse.y * vectorToMouse.y) <= (menuData->radius * menuData->radius);
		if(inCircle) {
			// find out what angle the mouse makes with the horizontal axis
			// use "man atan2" from the terminal to figure out what it's about
			float mouseAngle = atan2(vectorToMouse.y, vectorToMouse.x);

			// If we got negative mouse angle (i.e. below the
			// horizontal axis) make it positive.
			if(mouseAngle < 0) {
				mouseAngle += 2 * pi;
			}

			// Now the item should come out through simple division
			partHit = (int) (mouseAngle / anglePerItem) + 1;
		}
	}

	SetEventParameter(inEvent, kEventParamControlPart, typeControlPartCode, sizeof( partHit ), &partHit );
}


/* -------------------------------------------- CreatePathForStarMenuItem */
/*
	Create a path for the given item.

	In true Core Foundation style, this is a CreateXXX routine and the
	caller is responsible for freeing the path that is returned.
*/
CGPathRef CreatePathForStarMenuItem(
	StarMenuData *menuData,
	MenuItemIndex whichItem)
{
   CGMutablePathRef retVal = CGPathCreateMutable();
   MenuItemIndex numItems = CountMenuItems(menuData->menu);

   if(numItems > 0) {
	  const CGPoint fullRadiusPoint = { menuData->radius, 0 };
	  const CGPoint halfRadiusPoint = { menuData->radius / 2.0, 0 };

	  float anglePerItem = 2 * pi / (float)numItems;   // in radians naturally
	  float halfAngle = anglePerItem / 2.0;

	  float startAngle = (anglePerItem * (whichItem - 1));
	  float midAngle = (anglePerItem * (whichItem - 1)) + halfAngle;
	  float endAngle = (anglePerItem * whichItem);

	  CGAffineTransform startRotate = CGAffineTransformMakeRotation(startAngle);
	  CGAffineTransform midRotate = CGAffineTransformMakeRotation(midAngle);
	  CGAffineTransform endRotate = CGAffineTransformMakeRotation(endAngle);

	  CGPoint startPoint = CGPointApplyAffineTransform(halfRadiusPoint, startRotate);
	  CGPoint midPoint = CGPointApplyAffineTransform(fullRadiusPoint, midRotate);
	  CGPoint endPoint = CGPointApplyAffineTransform(halfRadiusPoint, endRotate);

	  CGPathMoveToPoint(retVal, NULL, 0, 0);
	  CGPathAddLineToPoint(retVal, NULL, startPoint.x, startPoint.y);
	  CGPathAddLineToPoint(retVal, NULL, midPoint.x, midPoint.y);
	  CGPathAddLineToPoint(retVal, NULL, endPoint.x, endPoint.y);

	  CGPathCloseSubpath(retVal);
   }

   return retVal;
}


/* ------------------------------------------ CreatePathForEntireStarMenu */
/*
	Create a path for entire star menu

	In true Core Foundation style, this is a CreateXXX routine and the
	caller is responsible for freeing the path that is returned.
*/
CGPathRef   CreatePathForEntireStarMenu(StarMenuData *menuData)
{
   CGMutablePathRef retVal = CGPathCreateMutable();
   MenuItemIndex numItems = CountMenuItems(menuData->menu);

   if(numItems > 0) {
	  const CGPoint fullRadiusPoint = { menuData->radius, 0 };
	  const CGPoint halfRadiusPoint = { menuData->radius / 2.0, 0 };

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
