/*
    File:		HITestView.c
    
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

	Copyright © 2002 Apple Computer, Inc., All Rights Reserved
*/

#include <Carbon/Carbon.h>

#include "HITestView.h"

// -----------------------------------------------------------------------------
//	types
// -----------------------------------------------------------------------------
//
typedef struct
{
	ControlRef			control;
} HITestViewData;

// -----------------------------------------------------------------------------
//	prototypes
// -----------------------------------------------------------------------------
//
OSStatus HITestViewRegister();
pascal OSStatus HITestViewHandler(
	EventHandlerCallRef		inCallRef,
	EventRef				inEvent,
	void*					inUserData );
OSStatus HITestViewConstruct(
	EventRef				inEvent );
OSStatus HITestViewInitialize(
	EventHandlerCallRef		inCallRef,
	EventRef				inEvent,
	HITestViewData*			inData );
OSStatus HITestViewDestruct(
	EventRef				inEvent,
	HITestViewData*			inData );
OSStatus HITestViewDraw(
	EventRef				inEvent,
	HITestViewData*			inData );
OSStatus HITestViewHitTest(
	EventRef				inEvent,
	HITestViewData*			inData );
#if CUSTOM_TRACK
OSStatus HITestViewTrack(
	EventRef				inEvent,
	HITestViewData*			inData );
#endif
OSStatus HITestViewChanged(
	EventRef				inEvent,
	HITestViewData*			inData );
OSStatus HITestViewGetData(
	EventRef				inEvent,
	HITestViewData*			inData );
OSStatus HITestViewSetData(
	EventRef				inEvent,
	HITestViewData*			inData );
OSStatus HITestViewGetRegion(
	EventRef				inEvent,
	HITestViewData*			inData );

// -----------------------------------------------------------------------------
//	constants
// -----------------------------------------------------------------------------
//
#define kHITestViewClassID		CFSTR( "com.apple.sample.HITestView" )

const ControlPartCode	kControlOpaqueRegionMetaPart = -3;

// -----------------------------------------------------------------------------
//	HITestViewCreate
// -----------------------------------------------------------------------------
//
OSStatus HITestViewCreate(
	WindowRef			inWindow,
	const Rect*			inBounds,
	ControlRef*			outControl )
{
	OSStatus			err;
	ControlRef			root;
	EventRef			event;
	
	// Register this class
	err = HITestViewRegister();
	require_noerr( err, CantRegister );

	// Make an initialization event
	err = CreateEvent( NULL, kEventClassHIObject, kEventHIObjectInitialize,
			GetCurrentEventTime(), 0, &event );
	require_noerr( err, CantCreateEvent );
	
	// If bounds were specified, push the them into the initialization event
	// so that they can be used in the initialization handler.
	if ( inBounds != NULL )
	{
		err = SetEventParameter( event, 'Boun', typeQDRectangle,
				sizeof( Rect ), inBounds );
		require_noerr( err, CantSetParameter );
	}

	// Make a new instantiation of this class
	err = HIObjectCreate( kHITestViewClassID, event, (HIObjectRef*) outControl );
	require_noerr( err, CantCreate );

	// If a parent window was specified, place the new view into the
	// parent window.
	if ( inWindow != NULL )
	{
		err = GetRootControl( inWindow, &root );
		require_noerr( err, CantGetRootControl );
		
		err = HIViewAddSubview( root, *outControl );
	}

CantCreate:
CantGetRootControl:
CantSetParameter:
CantCreateEvent:
	ReleaseEvent( event );

CantRegister:
	return err;
}

// -----------------------------------------------------------------------------
//	HITestViewRegister
// -----------------------------------------------------------------------------
//	Register this class with the HIObject registry, notifying it of which
//	events we will be interested.
//
//	This API can be called multiple times, but will only register once.
//
OSStatus HITestViewRegister()
{
	OSStatus				err = noErr;
	static HIObjectClassRef	sHITestViewClassRef = NULL;

	if ( sHITestViewClassRef == NULL )
	{
		EventTypeSpec		eventList[] = {
			{ kEventClassHIObject, kEventHIObjectConstruct },
			{ kEventClassHIObject, kEventHIObjectInitialize },
			{ kEventClassHIObject, kEventHIObjectDestruct },
			{ kEventClassControl, kEventControlInitialize },
			{ kEventClassControl, kEventControlDraw },
			{ kEventClassControl, kEventControlHitTest },
#if CUSTOM_TRACK
			{ kEventClassControl, kEventControlTrack },
#endif
			{ kEventClassControl, kEventControlValueFieldChanged },
			{ kEventClassControl, kEventControlHiliteChanged } };

		err = HIObjectRegisterSubclass(
			kHITestViewClassID,			// class ID
			kHIViewClassID,				// base class ID
			NULL,						// option bits
			HITestViewHandler,			// construct proc
			GetEventTypeCount( eventList ),
			eventList,
			NULL,						// construct data,
			&sHITestViewClassRef );
	}
	
	return err;
}

// -----------------------------------------------------------------------------
//	HITestViewHandler
// -----------------------------------------------------------------------------
//	This is the event bottleneck through which all of the incoming events are
//	dispatched.
//
pascal OSStatus HITestViewHandler(
	EventHandlerCallRef		inCallRef,
	EventRef				inEvent,
	void*					inUserData )
{
	OSStatus				err = eventNotHandledErr;
	UInt32					eventClass = GetEventClass( inEvent );
	UInt32					eventKind = GetEventKind( inEvent );
	HITestViewData*			data = (HITestViewData*) inUserData;

	switch ( eventClass )
	{
		case kEventClassHIObject:
		{
			switch ( eventKind )
			{
				case kEventHIObjectConstruct:
					// don't CallNextEventHandler
					err = HITestViewConstruct( inEvent );
					break;

				case kEventHIObjectInitialize:
					err = HITestViewInitialize( inCallRef, inEvent, data );
					break;

				case kEventHIObjectDestruct:
					// don't CallNextEventHandler
					err = HITestViewDestruct( inEvent, data );
					break;
			}
		}
		break;
		
		case kEventClassControl:
		{
			switch ( eventKind )
			{
				case kEventControlInitialize:
					err = noErr;
					break;

				case kEventControlDraw:
					err = HITestViewDraw( inEvent, data );
					break;
				
				case kEventControlHitTest:
					err = HITestViewHitTest( inEvent, data );
					break;
				
#if CUSTOM_TRACK
				case kEventControlTrack:
					err = HITestViewTrack( inEvent, data );
					break;
#endif				
				case kEventControlValueFieldChanged:
				case kEventControlHiliteChanged:
					err = HITestViewChanged( inEvent, data );
					break;

				case kEventControlGetData:
					err = HITestViewGetData( inEvent, data );
					break;

				case kEventControlSetData:
					err = HITestViewSetData( inEvent, data );
					break;
				
				case kEventControlGetPartRegion:
					err = HITestViewGetRegion( inEvent, data );
					break;
			}
		}
		break;
	}
	
	return err;
}


// -----------------------------------------------------------------------------
//	HITestViewConstruct
// -----------------------------------------------------------------------------
//	Do any instatiation-time preparation for the view.
//
OSStatus HITestViewConstruct(
	EventRef				inEvent )
{
	OSStatus				err;
	HITestViewData*			data;

	// don't CallNextEventHandler!
	data = (HITestViewData*) malloc( sizeof( HITestViewData ) );
	require_action( data != NULL, CantMalloc, err = memFullErr );

	err = GetEventParameter( inEvent, kEventParamHIObjectInstance, typeHIObjectRef,
			NULL, sizeof( HIObjectRef ), NULL, (HIObjectRef*) &data->control );
	require_noerr( err, ParameterMissing );
	
	// Set the userData that will be used with all subsequent eventHandler calls
	err = SetEventParameter( inEvent, kEventParamHIObjectInstance, typeVoidPtr,
			sizeof( HITestViewData* ), &data ); 

ParameterMissing:
	if ( err != noErr )
		free( data );

CantMalloc:
	return err;
}

// -----------------------------------------------------------------------------
//	HITestViewDestruct
// -----------------------------------------------------------------------------
//
OSStatus HITestViewDestruct(
	EventRef				inEvent,
	HITestViewData*			inData )
{
#pragma unused( inEvent, inData )
	return noErr;
}

// -----------------------------------------------------------------------------
//	HITestViewInitialize
// -----------------------------------------------------------------------------
//	The view is contstructed.  Do anything necessary to initialize it.
//
OSStatus HITestViewInitialize(
	EventHandlerCallRef		inCallRef,
	EventRef				inEvent,
	HITestViewData*			inData )
{
	OSStatus				err;
	Rect					bounds;

	// Let any parent classes have a chance at initialization
	err = CallNextEventHandler( inCallRef, inEvent );
	require_noerr( err, TroubleInSuperClass );

	// Extract the bounds from the initialization event
	err = GetEventParameter( inEvent, 'Boun', typeQDRectangle,
			NULL, sizeof( Rect ), NULL, &bounds );
	require_noerr( err, ParameterMissing );
	
	// Resize the view
	SetControlBounds( inData->control, &bounds );
	
ParameterMissing:
TroubleInSuperClass:
	return err;
}

// -----------------------------------------------------------------------------
//	HITestViewDraw
// -----------------------------------------------------------------------------
//	Here's the fun stuff.  Draw a red box, unless highlit, then draw a blue box.
//
OSStatus HITestViewDraw(
	EventRef				inEvent,
	HITestViewData*			inData )
{
	OSStatus				err;
	HIRect					bounds;
	CGContextRef			context;

	err = GetEventParameter( inEvent, kEventParamCGContextRef, typeCGContextRef,
			NULL, sizeof( CGContextRef ), NULL, &context );
	require_noerr( err, ParameterMissing );

	err = HIViewGetBounds( inData->control, &bounds );
	
	switch ( GetControlHilite( inData->control ) )
	{
		case kControlNoPart:
			CGContextSetRGBFillColor( context, 1, 0, 0, 0.25 );
			CGContextSetRGBStrokeColor( context, 1, 0, 0, 1 );
			break;
		default:
			CGContextSetRGBFillColor( context, 0, 0, 1, 0.25 );
			CGContextSetRGBStrokeColor( context, 0, 0, 1, 1 );
			break;
	}
	CGContextFillRect( context, bounds );
	CGContextStrokeRect( context, bounds );

ParameterMissing:
	return err;
}

// -----------------------------------------------------------------------------
//	HITestViewHitTest
// -----------------------------------------------------------------------------
//	Check to see if a point hits the view
//
OSStatus HITestViewHitTest(
	EventRef				inEvent,
	HITestViewData*			inData )
{
	OSStatus				err;
	HIRect					bounds;
	HIPoint					where;
	ControlPartCode			part;

	// Extract the mouse location
	err = GetEventParameter( inEvent, kEventParamMouseLocation, typeHIPoint,
			NULL, sizeof( HIPoint ), NULL, &where );
	require_noerr( err, ParameterMissing );

	// Is the mouse in the view?
	err = HIViewGetBounds( inData->control, &bounds );
	if ( CGRectContainsPoint( bounds, where ) )
		part = 1;
	else
		part = kControlNoPart;

	// Send back the value of the hit part
	err = SetEventParameter( inEvent, kEventParamControlPart, typeControlPartCode,
			sizeof( ControlPartCode ), &part ); 

ParameterMissing:
	return err;
}

#if CUSTOM_TRACK
// -----------------------------------------------------------------------------
//	HITestViewTrack
// -----------------------------------------------------------------------------
//	This is overkill, and probably #ifdef'd out, but is here as an example of
//	a custom tracking handler.
//
OSStatus HITestViewTrack(
	EventRef				inEvent,
	HITestViewData*			inData )
{
	OSStatus				err;
	HIRect					bounds;
	HIPoint					where;
	ControlPartCode			part;
	Boolean					inside;
	Boolean					wasInside;
	Point					qdPt;
	MouseTrackingResult		mouseResult;
	PixMapHandle			portPixMap;

	// Extract the mouse location
	err = GetEventParameter( inEvent, kEventParamMouseLocation, typeHIPoint,
			NULL, sizeof( HIPoint ), NULL, &where );
	require_noerr( err, ParameterMissing );

	// Is the mouse location in the view?
	err = HIViewGetBounds( inData->control, &bounds );
	if ( CGRectContainsPoint( bounds, where ) )
		part = 1;
	else
		part = kControlNoPart;
	HiliteControl( inData->control, part );
	
	wasInside = true;
	
	// Need the port's pixMap's bounds to convert the mouse location
	portPixMap = GetPortPixMap( GetWindowPort( GetControlOwner( inData->control ) ) );

	// The tracking loop
	while ( true )
	{
		// Check again to see if the mouse is in the view
		if ( CGRectContainsPoint( bounds, where ) )
			part = 1;
		else
			part = kControlNoPart;
		inside = ( part != kControlNoPart );
		
		// If that changed, update
		if ( inside != wasInside )
			HiliteControl( inData->control, part );
		wasInside = inside;

		// Watch the mouse for change
		err = TrackMouseLocation( (GrafPtr)-1L, &qdPt, &mouseResult );

		// Need to convert from global
		QDGlobalToLocalPoint( GetWindowPort( GetControlOwner( inData->control ) ), &qdPt );
		where.x = qdPt.h - (**portPixMap).bounds.left;
		where.y = qdPt.v - (**portPixMap).bounds.top;
		HIViewConvertPoint( &where, NULL, inData->control );

		// Bail out when the mouse is released
		if ( mouseResult == kMouseTrackingMouseReleased )
			break;
	}
	
	// Restore the original highlight
	HiliteControl( inData->control, kControlNoPart );

	// Send back the part upon which the mouse was released
	err = SetEventParameter( inEvent, kEventParamControlPart, typeControlPartCode,
			sizeof( ControlPartCode ), &part ); 

ParameterMissing:
	return err;
}
#endif

// -----------------------------------------------------------------------------
//	HITestViewChanged
// -----------------------------------------------------------------------------
//	Handler for bounds or hilite changed events
//
OSStatus HITestViewChanged(
	EventRef				inEvent,
	HITestViewData*			inData )
{
#pragma unused( inEvent )
	OSStatus				err = noErr;

	// Due to the change, the view needs to redraw
	HIViewSetNeedsDisplay( inData->control, true );

	return err;
}

// -----------------------------------------------------------------------------
//	HITestViewGetData
// -----------------------------------------------------------------------------
//
OSStatus HITestViewGetData(
	EventRef				inEvent,
	HITestViewData*			inData )
{
#pragma unused( inData )
	OSStatus				err;
	ControlPartCode			part;
	OSType					tag;
	Ptr						ptr;
	Size					size;
	Size					outSize;
	
	// Extract the part -- we don't use it here, but it might be important
	// in a non-trivial view
	err = GetEventParameter( inEvent, kEventParamControlPart, typeControlPartCode,
			NULL, sizeof( ControlPartCode ), NULL, &part );
	require_noerr( err, ParameterMissing );

	// Extract the rest of the info needs for data handling
	err = GetEventParameter( inEvent, kEventParamControlDataTag, typeEnumeration,
			NULL, sizeof( OSType ), NULL, &tag );
	require_noerr( err, ParameterMissing );

	err = GetEventParameter( inEvent, kEventParamControlDataBuffer, typePtr,
			NULL, sizeof( Ptr ), NULL, &ptr );
	require_noerr( err, ParameterMissing );

	err = GetEventParameter( inEvent, kEventParamControlDataBufferSize, typeLongInteger,
			NULL, sizeof( Size ), NULL, &size );
	require_noerr( err, ParameterMissing );

	switch ( tag )
	{
/*
		case kControlHITest_YourTagHere_Tag:
			if ( size == sizeof( HITest_YourTypeHere ) )
				*( (HITest_YourTypeHere*) ptr ) = inData->HITest_YourDataMemberHere;
			else
				err = errDataSizeMismatch;

			// Set the outgoing size.  In addition to indicating how much of a buffer
			// gets used, this is how clients can call in and check to see how big a
			// buffer they need to pass in.
			outSize = sizeof( HITest_YourTypeHere );
			break;
*/

		default:
			err = errDataNotSupported;
			outSize = 0;
			break;
	}

	if ( err == noErr )
		// Send back the outSize
		err = SetEventParameter( inEvent, kEventParamControlDataBufferSize, typeLongInteger,
				sizeof( Size ), &outSize );

ParameterMissing:
	return err;
}

// -----------------------------------------------------------------------------
//	HITestViewSetData
// -----------------------------------------------------------------------------
//
OSStatus HITestViewSetData(
	EventRef				inEvent,
	HITestViewData*			inData )
{
#pragma unused( inData )
	OSStatus				err;
	ControlPartCode			part;
	OSType					tag;
	Ptr						ptr;
	Size					size;
	
	// Extract the part -- we don't use it here, but it might be important
	// in a non-trivial view
	err = GetEventParameter( inEvent, kEventParamControlPart, typeControlPartCode,
			NULL, sizeof( ControlPartCode ), NULL, &part );
	require_noerr( err, ParameterMissing );

	// Extract the rest of the info needs for data handling
	err = GetEventParameter( inEvent, kEventParamControlDataTag, typeEnumeration,
			NULL, sizeof( OSType ), NULL, &tag );
	require_noerr( err, ParameterMissing );

	err = GetEventParameter( inEvent, kEventParamControlDataBuffer, typePtr,
			NULL, sizeof( Ptr ), NULL, &ptr );
	require_noerr( err, ParameterMissing );

	err = GetEventParameter( inEvent, kEventParamControlDataBufferSize, typeLongInteger,
			NULL, sizeof( Size ), NULL, &size );
	require_noerr( err, ParameterMissing );

	switch ( tag )
	{
/*
		case kControlHITest_YourTagHere_Tag:
			if ( size == sizeof( HITest_YourTypeHere ) )
				inData->HITest_YourDataMemberHere = *( (HITest_YourTypeHere*) ptr );
			else
				err = errDataSizeMismatch;
			break;
*/
		default:
			err = errDataNotSupported;
			break;
	}

ParameterMissing:
	return err;
}

// -----------------------------------------------------------------------------
//	HITestViewGetRegion
// -----------------------------------------------------------------------------
//
OSStatus HITestViewGetRegion(
	EventRef				inEvent,
	HITestViewData*			inData )
{
	OSStatus				err;
	ControlPartCode			part;
	RgnHandle				outRegion;
	HIRect					bounds;
	Rect					qdBounds;
	
	err = GetEventParameter( inEvent, kEventParamControlPart, typeControlPartCode,
			NULL, sizeof( ControlPartCode ), NULL, &part );
	require_noerr( err, ParameterMissing );

	err = GetEventParameter( inEvent, kEventParamControlRegion, typeQDRgnHandle,
			NULL, sizeof( RgnHandle ), NULL, &outRegion );

	if ( part == kControlContentMetaPart
			|| part == kControlStructureMetaPart
			/* || part == kControlOpaqueRegionMetaPart */ )
	{
		HIViewGetBounds( inData->control, &bounds );
		qdBounds.top = (SInt16) CGRectGetMinY( bounds );
		qdBounds.left = (SInt16) CGRectGetMinX( bounds );
		qdBounds.bottom = (SInt16) CGRectGetMaxY( bounds );
		qdBounds.right = (SInt16) CGRectGetMaxX( bounds );
	
		RectRgn( outRegion, &qdBounds );
	}
	
ParameterMissing:
	return err;
}
