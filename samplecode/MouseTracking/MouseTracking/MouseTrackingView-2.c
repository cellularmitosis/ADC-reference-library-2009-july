/*
    File:	MouseTrackingView-2.c
    
    Version:	1.0

    Disclaimer:	IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc.
		("Apple") in consideration of your agreement to the following terms, and your
		use, installation, modification or redistribution of this Apple software
		constitutes acceptance of these terms.  If you do not agree with these terms,
		please do not use, install, modify or redistribute this Apple software.

		In consideration of your agreement to abide by the following terms, and subject
		to these terms, Apple grants you a personal, non-exclusive license, under Apple’s
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

	Copyright © 2005 Apple Computer, Inc., All Rights Reserved
*/

/****************************************************************************************************
 * And this is the version with the "bells and whistles", to illustrate the following points:
 * - Use the sketched path as clip path (vs. the QD clipRgn)
 * - Use a 1x1 bitmap context for hit testing on the clipped portion (vs. PtInRgn)
 * - Drag the clipped image portion around, with transparency (vs. GWorlds, CopyBits, CopyDeepMask etc.)
 * - Use a maskedImage to remove the white background (vs. CopyBits, SearchProcs, BitmapToRegion etc.)
 ****************************************************************************************************/

#include "MouseTrackingView.h"

extern CGImageRef LoadImageFromMainBundle(CFStringRef imageName);


// A pointer to this per-view storage gets passed to the EventHandler as "userData".
struct MTViewData {
    HIViewRef		theView;
    CGImageRef		theImage;
    CGImageRef		maskedImage;
    CGMutablePathRef    thePath;
    HIPoint		selectionOffset;
    Boolean		pathBuildingDone;
    Boolean		hitSelection;
    Boolean		selectionMoveDone;
};
typedef struct MTViewData   MTViewData;

//-----------------------------------------------------------------------------------
// Two alternatives for "CreateMaskedImage", both available in 10.4 and later only:
// a) Use CGImageCreateWithMaskingColors
// b) Draw image into grayLevel bitmap context to be used as imageMask
//    and use CGImageCreateWithMask.
// Prior to 10.4, the mask would have to be worked into the alpha channel of the image.
 
#if 0
static CGImageRef CreateMaskedImage(CGImageRef image)
{
    const float maskingMinMax[] = { 220, 255, 220, 255, 220, 255 };
    CGImageRef maskedImage = CGImageCreateWithMaskingColors(image, maskingMinMax);
    return maskedImage;
}

#else

static void FreePixelData(void *info, const void *data, size_t size)
{
    free((void*)data);
}

static void FixOpaqueness(uint8_t* p, size_t n)
{
    const uint8_t kCutOff = 220;
    while (n-- > 0)
    {
	uint8_t v = *p;
	if (v < kCutOff)
	    v = 0;
	*p++ = v;
    }
}

static CGImageRef CreateMaskedImage(CGImageRef image)
{
    // Draw the image into a grayLevel bitmap context to produce a mask
    size_t width = CGImageGetWidth(image);
    size_t height = CGImageGetHeight(image);
    size_t dataSize = width * height;	    // 1 byte per pixel
    void* data = malloc(dataSize);
    CGColorSpaceRef colorspace = CGColorSpaceCreateDeviceGray();
    // Ironically, due to a bug in CGImageCreateWithMask, you cannot use 
    // CGColorSpaceCreateWithName(kCGColorSpaceGenericGray) at this point!
    
    CGContextRef ctx = CGBitmapContextCreate(data, width, height, 8, width, colorspace, kCGImageAlphaNone);
    CGContextDrawImage(ctx, CGRectMake(0, 0, width, height), image);
    CGContextRelease(ctx);
    FixOpaqueness(data, dataSize);
    CGDataProviderRef provider = CGDataProviderCreateWithData(NULL, data, dataSize, FreePixelData);
    const float decode[] = { 1.0, 0.0 };    // need to inverse gray levels
    CGImageRef imageMask = CGImageCreate(width, height, 8, 8, width, colorspace, kCGImageAlphaNone, provider, decode, false, kCGRenderingIntentDefault);
    CGColorSpaceRelease(colorspace);
    CGDataProviderRelease(provider);

    CGImageRef maskedImage = CGImageCreateWithMask(image, imageMask);
    CGImageRelease(imageMask);
    
    return maskedImage;
}
#endif

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

    if ((data->theImage == NULL) || (data->maskedImage == NULL))
	return;
	
    // Fill background
#if 1
    HIViewGetBounds(data->theView, &dstRect);
    CGContextSetRGBFillColor(ctx, 0.5, 1, 0.5, 1);
    CGContextFillRect(ctx, dstRect);
#endif

    // Draw the image first, before stroking the path; otherwise the path gets overwritten

    dstRect = CGRectMake(0, 0, CGImageGetWidth(data->theImage), CGImageGetHeight(data->theImage));
    HIViewDrawCGImage(ctx, &dstRect, data->maskedImage);
    
    // If we already hit the selection, remove the selection from the original image.
    // Because we apply a clip, save and restore the GState.
    CGContextSaveGState(ctx);
    if (data->hitSelection)
    {
	CGPathApply(data->thePath, ctx, MyCGPathApplier);
	CGContextClip(ctx);
	CGContextSetRGBFillColor(ctx, 1, 1, 1, 1);
	CGContextFillRect(ctx, dstRect);
    }
    CGContextRestoreGState(ctx);
    
    // Draw our selection path only as long as we have not hit the selection
    if (!data->hitSelection && (data->thePath != NULL))
    {
	CGPathApply(data->thePath, ctx, MyCGPathApplier);
	CGContextStrokePath(ctx);
    }
   
    if (data->pathBuildingDone)
    {
//	CGContextSaveGState(ctx); -- not necessary here: the HIView system saves and restores the GState from the outside
	CGContextTranslateCTM(ctx, data->selectionOffset.x, data->selectionOffset.y);
	CGPathApply(data->thePath, ctx, MyCGPathApplier);
	CGContextClip(ctx);
	if (!data->selectionMoveDone)	// while we are dragging, use some transparency for the image drawing
	    CGContextSetAlpha(ctx, 0.5);
	HIViewDrawCGImage(ctx, &dstRect, data->maskedImage);
//	CGContextRestoreGState(ctx);
    }
}   // DrawTheMTView


// -----------------------------------------------------------------------------
static HIPoint QDGlobalToHIViewLocal( const Point inGlobalPoint, const HIViewRef inTheView)
{                               
    HIPoint viewPoint = CGPointMake(inGlobalPoint.h, inGlobalPoint.v);

    HIPointConvert(&viewPoint, kHICoordSpace72DPIGlobal, NULL, kHICoordSpaceView, inTheView);
    return viewPoint;
}

// -----------------------------------------------------------------------------
static OSStatus	DoTheHitTest(EventRef inEvent, MTViewData* data)
{
    HIPoint where;
    ControlPartCode part = kControlContentMetaPart;
    
    GetEventParameter(inEvent, kEventParamMouseLocation, typeHIPoint, NULL, sizeof(HIPoint), NULL, &where);
    SetEventParameter(inEvent, kEventParamControlPart, typeControlPartCode, sizeof(ControlPartCode), &part);
    
    // If we hit a previously clipped image portion described by data->thePath, indicate it so we can start 
    // dragging it around inside "DoTheTracking"
    
    if (data->pathBuildingDone)	// i.e. we have finished data->thePath to use it as grab selection
    {
	// Create 1x1 BitmapContext
	CGColorSpaceRef colorspace = CGColorSpaceCreateWithName(kCGColorSpaceGenericRGB);
	CGBitmapInfo bitmapInfo = kCGImageAlphaNoneSkipFirst;
	UInt32	thePixel = 0;
	CGContextRef bmpCtx = CGBitmapContextCreate(&thePixel, 1, 1, 8, 4, colorspace, bitmapInfo);

	// Move it to "where"
	CGContextTranslateCTM(bmpCtx, -where.x, -where.y);

	// Fill the path and ...
	CGPathApply(data->thePath, bmpCtx, MyCGPathApplier);
	CGContextFillPath(bmpCtx);
	
	// ... check whether we have touched thePixel
	data->hitSelection = (thePixel != 0);
	
	CGColorSpaceRelease(colorspace);
	CGContextRelease(bmpCtx);
    }
    
    return noErr;
}   // DoTheHitTest

// -----------------------------------------------------------------------------
static void DragImageSelectionAround(EventRef inEvent, MTViewData* data)
{
    HIPoint startPt;
    
    // Extract the mouse location (local coordinates!)
    GetEventParameter(inEvent, kEventParamMouseLocation, typeHIPoint, NULL, sizeof(HIPoint), NULL, &startPt);
    
    data->selectionMoveDone = false;

    while (true)
    {
	MouseTrackingResult mouseResult;
	Point		    qdPt;
	
	// Watch the mouse for change: qdPt comes back in global coordinates!
	TrackMouseLocation((GrafPtr)(-1), &qdPt, &mouseResult);

	// Bail out when the mouse is released
	if ( mouseResult == kMouseTrackingMouseReleased )
	{
	    data->selectionMoveDone = true;
	    break;
	}
	
	// Need to convert from global
	HIPoint where = QDGlobalToHIViewLocal(qdPt, data->theView);
	data->selectionOffset.x = where.x - startPt.x;
	data->selectionOffset.y = where.y - startPt.y;

	ControlPartCode part = 0;
	SetEventParameter(inEvent, kEventParamControlPart, typeControlPartCode, sizeof(ControlPartCode), &part); 

	HIViewSetNeedsDisplay(data->theView, true);
    }
}   // DragImageSelectionAround

// -----------------------------------------------------------------------------
static void BuildPolygonPath(EventRef inEvent, MTViewData* data)
{
    MouseTrackingResult	mouseResult;
    Point		qdPt;
    HIPoint		where;
    CGAffineTransform m = CGAffineTransformIdentity;
    // Reset the path
    if (data->thePath != NULL)
	CGPathRelease(data->thePath);
    
    data->thePath = CGPathCreateMutable();
    data->pathBuildingDone = false;
    data->hitSelection = false;
    data->selectionOffset.x = data->selectionOffset.y = 0;
    
    // Extract the mouse location (local coordinates!)
    GetEventParameter(inEvent, kEventParamMouseLocation, typeHIPoint, NULL, sizeof(HIPoint), NULL, &where);
    CGPathMoveToPoint(data->thePath, &m, where.x, where.y);
    
    while (true)
    {
	// Watch the mouse for change: qdPt comes back in global coordinates!
	TrackMouseLocation((GrafPtr)(-1), &qdPt, &mouseResult);

	// Bail out when the mouse is released
	if ( mouseResult == kMouseTrackingMouseReleased )
	{
	    data->pathBuildingDone = true;
	    break;
	}
	
	// Need to convert from global
	where = QDGlobalToHIViewLocal(qdPt, data->theView);
	CGPathAddLineToPoint(data->thePath, &m, where.x, where.y);

	ControlPartCode part = 0;
	SetEventParameter(inEvent, kEventParamControlPart, typeControlPartCode, sizeof(ControlPartCode), &part); 

	HIViewSetNeedsDisplay(data->theView, true);
    }
}   // BuildPolygonPath

//-----------------------------------------------------------------------------------
static OSStatus DoTheTracking(EventRef inEvent, MTViewData* data)
{
    // If we hit an image selection, drag it around, else build a path
    if (data->hitSelection)
	DragImageSelectionAround(inEvent, data);
    else
	BuildPolygonPath(inEvent, data);

    // One final redisplay
    HIViewSetNeedsDisplay(data->theView, true);
    
    // Send back the part upon which the mouse was released
    ControlPartCode part = kControlEntireControl;
    SetEventParameter(inEvent, kEventParamControlPart, typeControlPartCode, sizeof(ControlPartCode), &part);
    return noErr; 
}

//-----------------------------------------------------------------------------------
static OSStatus DoHIObjectBusiness(EventHandlerCallRef inCallRef, EventRef inEvent, void* inUserData)
{
    MTViewData* data;
    OSStatus	err = eventNotHandledErr;
    switch ( GetEventKind(inEvent) )
    {
	case kEventHIObjectConstruct:
	{
	    data = (MTViewData*)malloc(sizeof(MTViewData));
	    // Follow the usual mechanism of setting our data as EventParameter, after recovering data->theView.
	    err = GetEventParameter( inEvent, kEventParamHIObjectInstance, typeHIObjectRef, NULL, sizeof(HIObjectRef*), NULL, &data->theView );
	    require_noerr( err, ParameterMissing );

	    SetEventParameter( inEvent, kEventParamHIObjectInstance, typeVoidPtr, sizeof(MTViewData), &data ); 
	}
	break;
	
	case kEventHIObjectInitialize:
	err = CallNextEventHandler( inCallRef, inEvent );
	if ( err == noErr )
	{
	    data = (MTViewData*)inUserData;
	    Rect bounds;
	    // Extract the bounds from the initialization event that we set up in CreateMouseTrackingView
	    err = GetEventParameter(inEvent, kCanvasBoundsParam, typeQDRectangle, NULL, sizeof(Rect), NULL, &bounds);
	    require_noerr(err, ParameterMissing);
	    SetControlBounds(data->theView, &bounds);
	    
	    // Also initialize our MTViewData
	    data->theImage = LoadImageFromMainBundle(CFSTR("Peter.png"));
	    data->maskedImage = CreateMaskedImage(data->theImage);
	    data->thePath = NULL;
	    data->selectionOffset.x = data->selectionOffset.y = 0;
	    data->pathBuildingDone = false;
	    data->hitSelection = false;
	    data->selectionMoveDone = false;
	}
	break;
	
	case kEventHIObjectDestruct:
	{
	    CGImageRelease(data->theImage);
	    CGImageRelease(data->maskedImage);
	    CGPathRelease(data->thePath);
	    free(inUserData);
	}
	break;
    }
    
ParameterMissing:
    return err;
}   // DoHIObjectBusiness

//-----------------------------------------------------------------------------------
static OSStatus
MTViewHandler( EventHandlerCallRef inCallRef, EventRef inEvent, void* inUserData )
{
    OSStatus	    err = eventNotHandledErr;
	    
    switch ( GetEventClass(inEvent) )
    {
	case kEventClassHIObject:
	    err = DoHIObjectBusiness(inCallRef, inEvent, inUserData);
	    break;

	case kEventClassControl:
	{
	    MTViewData* data = (MTViewData*)inUserData;
	    switch ( GetEventKind(inEvent) )
	    {
		case kEventControlDraw:
		{
		    CGContextRef ctx;
		    GetEventParameter( inEvent, kEventParamCGContextRef, typeCGContextRef, NULL, sizeof(CGContextRef), NULL, &ctx );

		    CallNextEventHandler(inCallRef, inEvent);	// not really needed, here
		    DrawTheMTView(ctx, data);
		    err = noErr;
		}
		break;
		
		case kEventControlHitTest:
		    err = DoTheHitTest(inEvent, data);
		    break;

		case kEventControlTrack:
		    err = DoTheTracking(inEvent, data);
		    break;
	    }	// switch
	}
	break;	// kEventClassControl
    }	// switch
    return err;
}   // MTViewHandler


//----------------------------------------------------------------------------------- the usual boilerplate stuff
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

//-----------------------------------------------------------------------------------
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
}   // CreateMouseTrackingView


