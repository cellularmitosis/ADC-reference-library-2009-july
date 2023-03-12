/*
    File:	MouseTrackingView-CG.c
    
    Version:	1.0

    Disclaimer:	IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc.
		("Apple") in consideration of your agreement to the following terms, and your
		use, installation, modification or redistribution of this Apple software
		constitutes acceptance of these terms.  If you do not agree with these terms,
		please do not use, install, modify or redistribute this Apple software.

		In consideration of your agreement to abide by the following terms, and subject
		to these terms, Apple grants you a personal, non-exclusive license, under Apple‚Äôs
		copyrights in this original Apple software (the "Apple Software"), to use,
		reproduce, modify and redistribute the Apple Software, with or without
		modifications, in source and/or binary forms; provided that if you redistribute
		the Apple Software in its entirety and without modifications, you must retain
		this notice and the following text and disclaimers in all such redistributions of
		the Apple Software.  Neither the name, trademarks, service marks or logos of
		Apple Computer, Inc. may be used to endorse or promote products derived from the
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

	Copyright ¬© 2005 Apple Computer, Inc., All Rights Reserved
*/


/****************************************************************************************************
 * Here we replace the QD polygon tracking and drawing by a tracking and drawing a CGPath.
 * And, while we are at it, we add a background picture for decoration ...
 * We also want to experiment again with using either the QD-like HIView coordinates, or the
 * right-handed CG coordinates with the origin at the bottom-left.
 ****************************************************************************************************/

#include "MouseTrackingView.h"

extern CGImageRef LoadImageFromMainBundle(CFStringRef imageName);     // implementation is in main.c

#define CG_COORDINATES	1

// A pointer to this per-view storage gets passed to the EventHandler as "userData".
struct MTViewData {
    HIViewRef		theView;
    CGImageRef		theImage;
    CGMutablePathRef    thePath;
};
typedef struct MTViewData   MTViewData;

#if CG_COORDINATES
static void TransformHIViewToCG(CGContextRef ctx, HIViewRef theView)
{
    // Undo the HIView coordinate flipping
    HIRect bounds;
    HIViewGetBounds(theView, &bounds);
    CGContextConcatCTM(ctx, CGAffineTransformMake(1, 0, 0, -1, 0, bounds.size.height));
}
#endif

static float FlipHIViewYCoordinate(HIViewRef theView, float y)
{
    // Undo the HIView coordinate flipping
    HIRect bounds;
    HIViewGetBounds(theView, &bounds);
    return bounds.size.height - y;
}

//-----------------------------------------------------------------------------------
// Called for each CGPathElement when scanning the CGPath in CGPathApply()
static void MyCGPathApplier(void* info, const CGPathElement* element)
{
    CGContextRef ctx = (CGContextRef)info;
    switch (element->type)
    {
	case kCGPathElementMoveToPoint:
	    CGContextMoveToPoint(ctx, element->points[0].x, element->points[0].y);
	    break;
		
	case kCGPathElementAddLineToPoint:
	    CGContextAddLineToPoint(ctx, element->points[0].x, element->points[0].y);
	    break;
	
	default:	// we know our path only contains line segments
	    break;
    }
}

//-----------------------------------------------------------------------------------
static void DrawTheMTView(CGContextRef ctx, MTViewData* data)
{
    CGRect dstRect;

#if CG_COORDINATES
    TransformHIViewToCG(ctx, data->theView);
#endif

    // Draw the image first, before stroking the path; otherwise the path gets overwritten
    if (data->theImage != NULL)
    {
	dstRect = CGRectMake(0, 0, CGImageGetWidth(data->theImage), CGImageGetHeight(data->theImage));
#if CG_COORDINATES
	CGContextDrawImage(ctx, dstRect, data->theImage);
#else
	HIViewDrawCGImage(ctx, &dstRect, data->theImage);
#endif
    }
    
    if (data->thePath != NULL)
    {
	CGPathApply(data->thePath, (void*)ctx, MyCGPathApplier);
	CGContextStrokePath(ctx);
    }
}   // DrawTheMTView

// -----------------------------------------------------------------------------
static HIPoint  
QDGlobalToHIViewLocal( const Point inGlobalPoint, const HIViewRef inDestView)
{                               
    HIPoint viewPoint = CGPointMake(inGlobalPoint.h, inGlobalPoint.v);

    HIPointConvert(&viewPoint, kHICoordSpace72DPIGlobal, NULL, kHICoordSpaceView, inDestView);
    return viewPoint;
}

//-----------------------------------------------------------------------------------
static void DoTheTracking(EventRef inEvent, MTViewData* data)
{
    MouseTrackingResult	mouseResult;
    ControlPartCode	part;
    Point		qdPt;
    HIPoint		where;
    
    // Extract the mouse location (local coordinates!)
    GetEventParameter(inEvent, kEventParamMouseLocation, typeHIPoint, NULL, sizeof(HIPoint), NULL, &where);

    CGAffineTransform m = CGAffineTransformIdentity;
    // Reset the path
    if (data->thePath != NULL)
	CGPathRelease(data->thePath);
    
    data->thePath = CGPathCreateMutable();
    
#if CG_COORDINATES
    where.y = FlipHIViewYCoordinate(data->theView, where.y);
#endif

    CGPathMoveToPoint(data->thePath, &m, where.x, where.y);
//  fprintf(stderr, "StartPt:  (%g, %g\n", where.x, where.y);
    
    while (true)
    {
	// Watch the mouse for change: qdPt comes back in global coordinates!
	TrackMouseLocation((GrafPtr)(-1), &qdPt, &mouseResult);

	// Bail out when the mouse is released
	if ( mouseResult == kMouseTrackingMouseReleased )
	{
	    HIViewSetNeedsDisplay(data->theView, true);
	    break;
	}
	
	// Need to convert from global
	where = QDGlobalToHIViewLocal(qdPt, data->theView);
#if CG_COORDINATES
	where.y = FlipHIViewYCoordinate(data->theView, where.y);
#endif
	
	CGPathAddLineToPoint(data->thePath, &m, where.x, where.y);
//	fprintf(stderr, "TrackPt:  (%g, %g\n", where.x, where.y);

	part = 0;
	SetEventParameter(inEvent, kEventParamControlPart, typeControlPartCode, sizeof(ControlPartCode), &part); 

	HIViewSetNeedsDisplay(data->theView, true);
    }
    
    // Send back the part upon which the mouse was released
    part = kControlEntireControl;
    SetEventParameter(inEvent, kEventParamControlPart, typeControlPartCode, sizeof(ControlPartCode), &part); 
}

//-----------------------------------------------------------------------------------
static OSStatus
MTViewHandler( EventHandlerCallRef inCallRef, EventRef inEvent, void* inUserData )
{
    OSStatus	    err = eventNotHandledErr;
    HIPoint	    where;
    ControlPartCode part;
    UInt32	    eventClass = GetEventClass( inEvent );
    UInt32	    eventKind = GetEventKind( inEvent );
    MTViewData*	    data = (MTViewData*)inUserData;		// pointless for the kEventHIObjectConstruct event
	    
    switch ( eventClass )
    {
	case kEventClassHIObject:
	switch ( eventKind )
	{
	    case kEventHIObjectConstruct:
	    {
		data = (MTViewData*)malloc(sizeof(MTViewData));

		err = GetEventParameter( inEvent, kEventParamHIObjectInstance, typeHIObjectRef, NULL, sizeof(HIObjectRef*), NULL, &data->theView );
		require_noerr( err, ParameterMissing );

		SetEventParameter( inEvent, kEventParamHIObjectInstance, typeVoidPtr, sizeof(MTViewData), &data ); 
	    }
	    break;
	    
	    case kEventHIObjectInitialize:
	    err = CallNextEventHandler( inCallRef, inEvent );
	    if ( err == noErr )
	    {
		Rect bounds;
		// Extract the bounds from the initialization event that we set up
		err = GetEventParameter(inEvent, kCanvasBoundsParam, typeQDRectangle, NULL, sizeof(Rect), NULL, &bounds);
		require_noerr(err, ParameterMissing);
		SetControlBounds(data->theView, &bounds);
		// Also initialize our MTViewData
		data->theImage = LoadImageFromMainBundle(CFSTR("Peter.png"));
		data->thePath = NULL;
	    }
	    break;
	    
	    case kEventHIObjectDestruct:
		free(inUserData);
	    break;
	}
	break;	// kEventClassHIObject
	

	case kEventClassControl:
	switch ( eventKind )
	{
	    case kEventControlDraw:
	    {
		CGContextRef ctx;
		GetEventParameter( inEvent, kEventParamCGContextRef, typeCGContextRef, NULL, sizeof(CGContextRef), NULL, &ctx );

//		CallNextEventHandler(inCallRef, inEvent);	// Erase old content
		DrawTheMTView(ctx, data);
		err = noErr;
	    }
	    break;
	    
	    case kEventControlHitTest:
			GetEventParameter(inEvent, kEventParamMouseLocation, typeHIPoint, NULL, sizeof(HIPoint), NULL, &where);
			part = kControlContentMetaPart;
			SetEventParameter(inEvent, kEventParamControlPart, typeControlPartCode, sizeof(ControlPartCode), &part);
			err = noErr;
			break;

	    case kEventControlTrack:
			DoTheTracking(inEvent, data);
			break;
	}
	break;	// kEventClassControl
    }

ParameterMissing:
	return err;
}


static OSStatus MyMTViewRegister(CFStringRef myClassID)
{
    OSStatus                err = noErr;
    static HIObjectClassRef sMyViewClassRef = NULL;
	
    if ( sMyViewClassRef == NULL )
    {
        EventTypeSpec eventList[] = {
            { kEventClassHIObject, kEventHIObjectConstruct },
            { kEventClassHIObject, kEventHIObjectInitialize },
            { kEventClassHIObject, kEventHIObjectDestruct },

            { kEventClassControl, kEventControlDraw },
            { kEventClassControl, kEventControlHitTest },
	    { kEventClassControl, kEventControlTrack },
	};
		
        err = HIObjectRegisterSubclass( myClassID,
					kHIViewClassID,					// base class ID
					0,						// option bits
					MTViewHandler,					// construct proc
					GetEventTypeCount( eventList ),
					eventList,
					NULL,						// construct data
					&sMyViewClassRef );
    }
    return err;
}

OSStatus 
CreateMouseTrackingView(HIViewRef parentView, const Rect* inBounds, HIViewID* inViewID)
{
    #define kCanvasClassID	CFSTR( "com.apple.sample.canvasview" )

    OSStatus	err;
    EventRef	event;
    HIViewRef	theView;
	
    // Register this class
    err = MyMTViewRegister(kCanvasClassID); 
    require_noerr( err, CantRegister );
	
    // Make an initialization event
    err = CreateEvent( NULL, kEventClassHIObject, kEventHIObjectInitialize, GetCurrentEventTime(), 0, &event ); 
    require_noerr( err, CantCreateEvent );
        
    // If bounds were specified, push them into the initialization event
    // so that they can be used in the initialization handler.
    if ( inBounds != NULL )
    {
        err = SetEventParameter(event, kCanvasBoundsParam, typeQDRectangle, sizeof(Rect), inBounds);
        require_noerr( err, CantSetParameter );
    }
    err = HIObjectCreate(kCanvasClassID, event, (HIObjectRef*)&theView);
    require_noerr(err, CantCreate);
	
    if (parentView != NULL) 
    {
        err = HIViewAddSubview(parentView, theView);
    }

    SetControlID(theView, inViewID);	// useful if a handler needs to call GetControlByID()
    HIViewSetVisible(theView, true);
	
CantCreate:
CantSetParameter:
CantCreateEvent:
    ReleaseEvent( event );
	
CantRegister:
    return err;
}


