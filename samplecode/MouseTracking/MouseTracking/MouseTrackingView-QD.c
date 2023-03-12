/*
    File:	MouseTrackingView-QD.c
    
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
 * The starting point of our MouseTrackingDemo: use a QD polygon structure to demo freehand sketching.
 ****************************************************************************************************/

#include "MouseTrackingView.h"


// A pointer to this per-view storage gets passed to the EventHandler as "userData".
struct MTViewData {
    HIViewRef		theView;
    PolyHandle		thePoly;
};
typedef struct MTViewData   MTViewData;



//-----------------------------------------------------------------------------------
static void DrawTheMTView(CGContextRef ctx, MTViewData* data)
{
    if (data->thePoly != NULL)
	FramePoly(data->thePoly);
}

// -----------------------------------------------------------------------------
static HIPoint  ConvertGlobalQDPointToViewHIPoint( const Point inGlobalPoint, const HIViewRef inDestView )
{                               
    Point	localPoint = inGlobalPoint;
    HIPoint	viewPoint;
    HIViewRef	contentView; 

    QDGlobalToLocalPoint( GetWindowPort(GetControlOwner(inDestView)), &localPoint );

    // convert the QD point to an HIPoint
    viewPoint = CGPointMake(localPoint.h, localPoint.v);
						    
    // get the content view (which is what the local point is relative to)
    HIViewFindByID(HIViewGetRoot(GetControlOwner(inDestView)), kHIViewWindowContentID, &contentView);

    // convert to view coordinates
    HIViewConvertPoint( &viewPoint, contentView, inDestView );
    return viewPoint;
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
	//--------------------------- some boilerplate HIObject business
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
		data->thePoly = NULL;
	    }
	    break;
	    
	    case kEventHIObjectDestruct:
		free(inUserData);
	    break;
	}
	break;	// kEventClassHIObject
	
	//--------------------------- kEventControlDraw, kEventControlHitTest and kEventControlTrack:
	case kEventClassControl:
	switch ( eventKind )
	{
	    case kEventControlDraw:
	    {
		CGContextRef ctx;
		GetEventParameter( inEvent, kEventParamCGContextRef, typeCGContextRef, NULL, sizeof(CGContextRef), NULL, &ctx );

		CallNextEventHandler(inCallRef, inEvent);	// Erase old content

		DrawTheMTView(ctx, data);
		err = noErr;
	    }
	    break;
	    
	    case kEventControlHitTest:
	    {
		GetEventParameter(inEvent, kEventParamMouseLocation, typeHIPoint, NULL, sizeof(HIPoint), NULL, &where);
		part = kControlContentMetaPart;
		SetEventParameter(inEvent, kEventParamControlPart, typeControlPartCode, sizeof(ControlPartCode), &part);
		err = noErr;
	    }
	    break;

	    case kEventControlTrack:
	    {
		MouseTrackingResult	mouseResult;
		Point			qdPt;

		// Extract the mouse location (local coordinates!)
		GetEventParameter(inEvent, kEventParamMouseLocation, typeHIPoint, NULL, sizeof(HIPoint), NULL, &where);

		// Reset the polygon
		if (data->thePoly != NULL)
		    KillPoly(data->thePoly);
		data->thePoly = OpenPoly();
		MoveTo(where.x, where.y);
		// also need to call ShowPen, or we don't see our line drawing
		ShowPen();
		
		while (true)
		{
		    // Watch the mouse for change: qdPt comes back in global coordinates!
		    err = TrackMouseLocation((GrafPtr)(-1), &qdPt, &mouseResult);

		    // Bail out when the mouse is released
		    if ( mouseResult == kMouseTrackingMouseReleased )
		    {
			// Time to close the polygon, and balance or extra ShowPen call
			HidePen();
			ClosePoly();
			break;
		    }
		    
		    // Need to convert from global
		    where = ConvertGlobalQDPointToViewHIPoint(qdPt, data->theView);
		    LineTo(where.x, where.y);
		    part = 0;
		    SetEventParameter(inEvent, kEventParamControlPart, typeControlPartCode, sizeof(ControlPartCode), &part); 

		    HIViewSetNeedsDisplay(data->theView, true);
		}
		
		// Send back the part upon which the mouse was released
		part = kControlEntireControl;
		SetEventParameter(inEvent, kEventParamControlPart, typeControlPartCode, sizeof(ControlPartCode), &part); 
	    }
	    break;
	}
	break;	// kEventClassControl
    }

ParameterMissing:
	return err;
}

//----------------------------------------------------------------------------------- (more boilerplate stuff)
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
OSStatus CreateMouseTrackingView(HIViewRef parentView, const Rect* inBounds, HIViewID* inViewID)
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


