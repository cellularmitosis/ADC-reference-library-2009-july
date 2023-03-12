/*
    File:		CalendarView.c
    
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

#include "CalendarView.h"
#include "ShadeRect.h"

// -----------------------------------------------------------------------------
//	constants
// -----------------------------------------------------------------------------
//
enum
{
	kCalendarMonthPart = 100,
	kCalendarPreviousYearPart,
	kCalendarPreviousMonthPart,
	kCalendarNextMonthPart,
	kCalendarNextYearPart,
	kCalendarSundayNamePart = 200,
	kCalendarMondayNamePart,
	kCalendarTuesdayNamePart,
	kCalendarWednesdayNamePart,
	kCalendarThursdayNamePart,
	kCalendarFridayNamePart,
	kCalendarSaturdayNamePart
};

#define kCalendarViewClassID	CFSTR( "com.apple.CalendarView" )

const ControlPartCode	kControlOpaqueRegionMetaPart = -3;

// -----------------------------------------------------------------------------
//	types
// -----------------------------------------------------------------------------
//
typedef struct
{
	HIViewRef			view;

	// Geometry
	float				titleRowRatio;
	float				dayNameRowRatio;
	float				dayRowRatio;
	
	// Date stuff
	CFGregorianDate		date;
	CFTimeZoneRef		timeZone;
	UInt8				firstDay;
	UInt8				daysInMonth;
	
	// Proc stuff
	CalendarDrawUPP		drawProc;
	CalendarDrawUPP		labelProc;
	
} CalendarViewData;

// -----------------------------------------------------------------------------
//	prototypes
// -----------------------------------------------------------------------------
//
OSStatus CalendarViewRegister();
pascal OSStatus CalendarViewHandler(
	EventHandlerCallRef		inCallRef,
	EventRef				inEvent,
	void*					inUserData );
OSStatus CalendarViewConstruct(
	EventRef				inEvent );
OSStatus CalendarViewInitialize(
	EventHandlerCallRef		inCallRef,
	EventRef				inEvent,
	CalendarViewData*		inData );
OSStatus CalendarViewDestruct(
	EventRef				inEvent,
	CalendarViewData*		inData );
OSStatus CalendarViewDraw(
	EventRef				inEvent,
	CalendarViewData*		inData );
OSStatus CalendarViewHitTest(
	EventRef				inEvent,
	CalendarViewData*		inData );
OSStatus CalendarViewTrack(
	EventRef				inEvent,
	CalendarViewData*		inData );
OSStatus CalendarViewChanged(
	EventRef				inEvent,
	CalendarViewData*		inData );
OSStatus CalendarViewGetData(
	EventRef				inEvent,
	CalendarViewData*		inData );
OSStatus CalendarViewSetData(
	EventRef				inEvent,
	CalendarViewData*		inData );
OSStatus CalendarViewGetRegion(
	EventRef				inEvent,
	CalendarViewData*		inData );
ControlPartCode FindPart(
	const HIRect*			inBounds,
	const HIPoint*			inWhere,
	CalendarViewData*		inData );
pascal void DefaultDrawPart(
	ControlPartCode			inPart,
	const HIRect*			inPartRect,
	const CalendarDrawData*	inData );
pascal void DefaultDrawPartLabel(
	ControlPartCode			inPart,
	const HIRect*			inPartRect,
	const CalendarDrawData*	inData );
void SetUpDateData(
	CalendarViewData*		inData );

// -----------------------------------------------------------------------------
//	globals
// -----------------------------------------------------------------------------
//

// -----------------------------------------------------------------------------
//	CalendarViewCreate
// -----------------------------------------------------------------------------
//
OSStatus CalendarViewCreate(
	WindowRef			inWindow,
	const Rect*			inBounds,
	ControlRef*			outControl )
{
	OSStatus			err;
	ControlRef			root;
	EventRef			event;
	
	require_action( inWindow != NULL, BadParent, err = paramErr );
	require_action( inBounds != NULL, BadBounds, err = paramErr );

	// Make sure this type of view is registered
	err = CalendarViewRegister();
	require_noerr( err, CantRegister );

	// Make the initialization event
	err = CreateEvent( NULL, kEventClassHIObject, kEventHIObjectInitialize,
			GetCurrentEventTime(), 0, &event );
	require_noerr( err, CantCreateEvent );
	
	// Set the bounds into the event
	err = SetEventParameter( event, 'Boun', typeQDRectangle,
			sizeof( Rect ), inBounds );
	require_noerr( err, CantSetParameter );

	err = HIObjectCreate( kCalendarViewClassID, event, (HIObjectRef*) outControl );
	require_noerr( err, CantCreate );

	// Get the content root
	err = GetRootControl( inWindow, &root );
	require_noerr( err, CantGetRootControl );
	
	// And stick this view into it
	err = HIViewAddSubview( root, *outControl );
	
CantCreate:
CantGetRootControl:
CantSetParameter:
CantCreateEvent:
	ReleaseEvent( event );

CantRegister:
BadBounds:
BadParent:
	return err;
}

// -----------------------------------------------------------------------------
//	CalendarViewRegister
// -----------------------------------------------------------------------------
//
OSStatus CalendarViewRegister()
{
	OSStatus				err = noErr;
	static HIObjectClassRef	sCalendarViewClassRef = NULL;

	if ( sCalendarViewClassRef == NULL )
	{
		EventTypeSpec		eventList[] = {
			{ kEventClassHIObject, kEventHIObjectConstruct },
			{ kEventClassHIObject, kEventHIObjectInitialize },
			{ kEventClassHIObject, kEventHIObjectDestruct },
			{ kEventClassControl, kEventControlInitialize },
			{ kEventClassControl, kEventControlDraw },
			{ kEventClassControl, kEventControlHitTest },
			{ kEventClassControl, kEventControlTrack },
			{ kEventClassControl, kEventControlValueFieldChanged },
			{ kEventClassControl, kEventControlHiliteChanged },
			{ kEventClassControl, kEventControlGetData },
			{ kEventClassControl, kEventControlSetData },
			{ kEventClassControl, kEventControlGetPartRegion }};

		err = HIObjectRegisterSubclass(
			kCalendarViewClassID,		// class ID
			kHIViewClassID,				// base class ID
			NULL,						// option bits
			CalendarViewHandler,		// construct proc
			GetEventTypeCount( eventList ),
			eventList,
			NULL,						// construct data,
			&sCalendarViewClassRef );
	}
	
	return err;
}

// -----------------------------------------------------------------------------
//	CalendarViewHandler
// -----------------------------------------------------------------------------
//	This is the bottleneck for incoming events
//
pascal OSStatus CalendarViewHandler(
	EventHandlerCallRef		inCallRef,
	EventRef				inEvent,
	void*					inUserData )
{
	OSStatus				err = eventNotHandledErr;
	UInt32					eventClass = GetEventClass( inEvent );
	UInt32					eventKind = GetEventKind( inEvent );
	CalendarViewData*		data = (CalendarViewData*) inUserData;

	switch ( eventClass )
	{
		case kEventClassHIObject:
		{
			switch ( eventKind )
			{
				case kEventHIObjectConstruct:
					err = CalendarViewConstruct( inEvent );
					break;

				case kEventHIObjectInitialize:
					err = CalendarViewInitialize( inCallRef, inEvent, data );
					break;

				case kEventHIObjectDestruct:
					// don't CallNextEventHandler!
					err = CalendarViewDestruct( inEvent, data );
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
					err = CalendarViewDraw( inEvent, data );
					break;
				
				case kEventControlHitTest:
					err = CalendarViewHitTest( inEvent, data );
					break;
				
				case kEventControlTrack:
					err = CalendarViewTrack( inEvent, data );
					break;
				
				case kEventControlValueFieldChanged:
				case kEventControlHiliteChanged:
					err = CalendarViewChanged( inEvent, data );
					break;

				case kEventControlGetData:
					err = CalendarViewGetData( inEvent, data );
					break;

				case kEventControlSetData:
					err = CalendarViewSetData( inEvent, data );
					break;
				
				case kEventControlGetPartRegion:
					err = CalendarViewGetRegion( inEvent, data );
					break;
			}
		}
		break;
	}
	
	return err;
}

// -----------------------------------------------------------------------------
//	CalendarViewConstruct
// -----------------------------------------------------------------------------
//
OSStatus CalendarViewConstruct(
	EventRef			inEvent )
{
	OSStatus			err;
	CalendarViewData*	data;
	
	// don't CallNextEventHandler!
	data = (CalendarViewData*) malloc( sizeof( CalendarViewData ) );
	require_action( data != NULL, CantMalloc, err = memFullErr );

	// Set up the row height ratios
	data->titleRowRatio = 0.5;		// half a row
	data->dayNameRowRatio = 0.5;	// half a row
	data->dayRowRatio = 1.0;		// full row

	// Set up the current timezone
	data->timeZone = CFTimeZoneCopySystem();
	require_action( data->timeZone != NULL, CantGetTimeZone, err = memFullErr );

	// Set up the current month
	data->date = CFAbsoluteTimeGetGregorianDate( CFAbsoluteTimeGetCurrent(), data->timeZone );
	data->date.day = 1;
	data->date.hour = 0;
	data->date.minute = 0;
	data->date.second = 0;
	SetUpDateData( data );

	// Set up the default drawing callbacks
	data->drawProc = DefaultDrawPart;
	data->labelProc = DefaultDrawPartLabel;
	
	// Keep a copy of the created HIViewRef
	err = GetEventParameter( inEvent, kEventParamHIObjectInstance, typeHIObjectRef,
			NULL, sizeof( HIObjectRef ), NULL, (HIObjectRef*) &data->view );
	require_noerr( err, ParameterMissing );
	
	// Set the userData that will be used with all subsequent eventHandler calls
	err = SetEventParameter( inEvent, kEventParamHIObjectInstance, typeVoidPtr,
			sizeof( CalendarViewData* ), &data ); 

ParameterMissing:
CantGetTimeZone:
	if ( err != noErr )
		free( data );

CantMalloc:
	return err;
}

// -----------------------------------------------------------------------------
//	CalendarViewDestruct
// -----------------------------------------------------------------------------
//
OSStatus CalendarViewDestruct(
	EventRef			inEvent,
	CalendarViewData*	inData )
{
#pragma unused( inEvent )
	// Clean up any allocated data
	CFRelease( inData->timeZone );
	free( inData );

	return noErr;
}

// -----------------------------------------------------------------------------
//	CalendarViewInitialize
// -----------------------------------------------------------------------------
//
OSStatus CalendarViewInitialize(
	EventHandlerCallRef	inCallRef,
	EventRef			inEvent,
	CalendarViewData*	inData )
{
	OSStatus			err;
	Rect				bounds;
	UInt32				features = kControlSupportsDataAccess;

	// Let the base class initialization occur
	err = CallNextEventHandler( inCallRef, inEvent );
	require_noerr( err, TroubleInSuperClass );

	// Extract the initial view bounds from the event
	err = GetEventParameter( inEvent, 'Boun', typeQDRectangle,
			NULL, sizeof( Rect ), NULL, &bounds );
	require_noerr( err, ParameterMissing );
	
	// Set up this view's feature bits
	err = SetEventParameter( inEvent, kEventParamControlFeatures, typeUInt32,
			sizeof( UInt32 ), &features );

	SetControlBounds( inData->view, &bounds );
	
ParameterMissing:
TroubleInSuperClass:
	return err;
}

// -----------------------------------------------------------------------------
//	CalendarViewDraw
// -----------------------------------------------------------------------------
//
OSStatus CalendarViewDraw(
	EventRef			inEvent,
	CalendarViewData*	inData )
{
	OSStatus			err;
	HIRect				bounds;
	float				rowHeight;
	float				colWidth;
	int					cols;
	float				rows = 0;
	HIRect				drawRect;
	ControlPartCode		part;
	UInt16				dayCount = 0;
	float				rowCount = 6 * inData->dayRowRatio + inData->dayNameRowRatio + inData->titleRowRatio;
	CalendarDrawData	drawData;
	
	// Get ready to do the CG drawing boogaloo!
	err = GetEventParameter( inEvent, kEventParamCGContextRef, typeCGContextRef,
			NULL, sizeof( CGContextRef ), NULL, &drawData.context );
	require_noerr( err, ParameterMissing );

	err = HIViewGetBounds( inData->view, &bounds );
	
	drawData.hilitePart = GetControlHilite( inData->view );
	drawData.daysInMonth = inData->daysInMonth;
	drawData.date = inData->date;

	// Figure out how tall a row should be
	rowHeight = bounds.size.height / rowCount;
	colWidth = (int) ( bounds.size.width / 14 );	// round here instead of over and over

	drawRect = bounds;
	drawRect.size.width = colWidth * 14;
	InvokeCalendarDrawUPP( kControlStructureMetaPart, &drawRect, &drawData, inData->drawProc );
	
	drawRect.origin = bounds.origin;
	drawRect.size.height = (int) ( rowHeight * inData->titleRowRatio );

	drawRect.size.width = colWidth;
	InvokeCalendarDrawUPP( kCalendarPreviousYearPart, &drawRect, &drawData, inData->drawProc );
	InvokeCalendarDrawUPP( kCalendarPreviousYearPart, &drawRect, &drawData, inData->labelProc );
	drawRect.origin.x += drawRect.size.width;

	InvokeCalendarDrawUPP( kCalendarPreviousMonthPart, &drawRect, &drawData, inData->drawProc );
	InvokeCalendarDrawUPP( kCalendarPreviousMonthPart, &drawRect, &drawData, inData->labelProc );
	drawRect.origin.x += drawRect.size.width;

	// Draw the month
	drawRect.size.width = 10 * colWidth;
	InvokeCalendarDrawUPP( kCalendarMonthPart, &drawRect, &drawData, inData->drawProc );
	InvokeCalendarDrawUPP( kCalendarMonthPart, &drawRect, &drawData, inData->labelProc );
	drawRect.origin.x += drawRect.size.width;

	drawRect.size.width = colWidth;
	InvokeCalendarDrawUPP( kCalendarNextMonthPart, &drawRect, &drawData, inData->drawProc );
	InvokeCalendarDrawUPP( kCalendarNextMonthPart, &drawRect, &drawData, inData->labelProc );
	drawRect.origin.x += drawRect.size.width;

	InvokeCalendarDrawUPP( kCalendarNextYearPart, &drawRect, &drawData, inData->drawProc );
	InvokeCalendarDrawUPP( kCalendarNextYearPart, &drawRect, &drawData, inData->labelProc );

	drawRect.origin.y += drawRect.size.height;	// on to the next row!
	rows += inData->titleRowRatio;
	drawRect.origin.x = bounds.origin.x;	// reset to leftmost

	// Optionally draw the week day names
	if ( inData->dayNameRowRatio != 0 )
	{
		// Draw the weekdays
		drawRect.size.width = 2 * colWidth;
		drawRect.size.height = (int) ( rowHeight * inData->dayNameRowRatio );
		part = kCalendarSundayNamePart;
		for ( cols = 0; cols < 7; cols++ )
		{
			InvokeCalendarDrawUPP( part, &drawRect, &drawData, inData->drawProc );
			InvokeCalendarDrawUPP( part, &drawRect, &drawData, inData->labelProc );
			drawRect.origin.x += drawRect.size.width;	// on to the next col!
			part++;
		}
		drawRect.origin.y += drawRect.size.height;	// on to the next row!
		rows += inData->dayNameRowRatio;
	}

	// Set up to draw the date rows
	drawRect.origin.x = bounds.origin.x;	// reset to leftmost
	drawRect.size.width = 2 * colWidth;
	drawRect.size.height = (int) ( rowHeight * inData->dayRowRatio );

	part = 0;
	for ( ; rows < rowCount; rows += inData->dayRowRatio )
	{
		for ( cols = 0; cols < 7; cols++ )
		{
			if ( dayCount++ >= inData->firstDay )
				part++;
			InvokeCalendarDrawUPP( part > inData->daysInMonth ? 0 : part, &drawRect, &drawData, inData->drawProc );
			InvokeCalendarDrawUPP( part > inData->daysInMonth ? 0 : part, &drawRect, &drawData, inData->labelProc );
			drawRect.origin.x += drawRect.size.width;	// on to the next col!
		}
		drawRect.origin.x = bounds.origin.x;	// reset to leftmost
		drawRect.origin.y += drawRect.size.height;	// on to the next row!
	}

ParameterMissing:
	return err;
}

// -----------------------------------------------------------------------------
//	CalendarViewHitTest
// -----------------------------------------------------------------------------
//
OSStatus CalendarViewHitTest(
	EventRef			inEvent,
	CalendarViewData*	inData )
{
	OSStatus			err;
	HIRect				bounds;
	HIPoint				where;
	ControlPartCode		part;

	err = GetEventParameter( inEvent, kEventParamMouseLocation, typeHIPoint,
			NULL, sizeof( HIPoint ), NULL, &where );
	require_noerr( err, ParameterMissing );

	err = HIViewGetBounds( inData->view, &bounds );

	part = FindPart( &bounds, &where, inData );

	err = SetEventParameter( inEvent, kEventParamControlPart, typeControlPartCode,
			sizeof( ControlPartCode ), &part ); 

ParameterMissing:
	return err;
}

// -----------------------------------------------------------------------------
//	CalendarViewTrack
// -----------------------------------------------------------------------------
//
OSStatus CalendarViewTrack(
	EventRef			inEvent,
	CalendarViewData*	inData )
{
	OSStatus			err;
	HIRect				bounds;
	HIPoint				where;
	ControlPartCode		part;
	ControlPartCode		lastPart;
	ControlPartCode		startHilite;
	Point				qdPt;
	MouseTrackingResult	mouseResult;
	PixMapHandle		portPixMap;

	err = GetEventParameter( inEvent, kEventParamMouseLocation, typeHIPoint,
			NULL, sizeof( HIPoint ), NULL, &where );
	require_noerr( err, ParameterMissing );

	err = HIViewGetBounds( inData->view, &bounds );
	
	startHilite = GetControlHilite( inData->view );
	
	lastPart = FindPart( &bounds, &where, inData );
	HiliteControl( inData->view, lastPart );
	
	// Need the port's pixMap's bounds to convert the point
	portPixMap = GetPortPixMap( GetWindowPort( GetControlOwner( inData->view ) ) );

	while ( true )
	{
		part = FindPart( &bounds, &where, inData );
		if ( lastPart != part )
			HiliteControl( inData->view, part );
		lastPart = part;

		err = TrackMouseLocation( (GrafPtr)-1L, &qdPt, &mouseResult );

		// Need to convert from global
		QDGlobalToLocalPoint( GetWindowPort( GetControlOwner( inData->view ) ), &qdPt );
		where.x = qdPt.h - (**portPixMap).bounds.left;
		where.y = qdPt.v - (**portPixMap).bounds.top;
		HIViewConvertPoint( &where, NULL, inData->view );

		// bail out when the mouse is released
		if ( mouseResult == kMouseTrackingMouseReleased )
			break;
	}
	
	// If a day wasn't clicked, revert the highlight to the last highlit day
	if ( lastPart < 1 || lastPart > inData->daysInMonth )
		HiliteControl( inData->view, startHilite );
	
	if ( lastPart >= kCalendarPreviousYearPart && lastPart <= kCalendarNextYearPart )
	{
		CFAbsoluteTime		tempTime;
		CFGregorianUnits	dateChange = { 0, 0, 0, 0, 0, 0 };

		tempTime = CFGregorianDateGetAbsoluteTime( inData->date, inData->timeZone );

		switch( lastPart )
		{
			case kCalendarPreviousYearPart:
				dateChange.years = -1;
				break;
			case kCalendarPreviousMonthPart:
				dateChange.months = -1;
				break;
			case kCalendarNextMonthPart:
				dateChange.months = 1;
				break;
			case kCalendarNextYearPart:
				dateChange.years = 1;
				break;
		}

		tempTime = CFAbsoluteTimeAddGregorianUnits( tempTime, inData->timeZone, dateChange );
		inData->date = CFAbsoluteTimeGetGregorianDate( tempTime, inData->timeZone );
		
		SetUpDateData( inData );
	}
	
	err = SetEventParameter( inEvent, kEventParamControlPart, typeControlPartCode,
			sizeof( ControlPartCode ), &part ); 

ParameterMissing:
	return err;
}

// -----------------------------------------------------------------------------
//	CalendarViewChanged
// -----------------------------------------------------------------------------
//
OSStatus CalendarViewChanged(
	EventRef			inEvent,
	CalendarViewData*	inData )
{
#pragma unused( inEvent )
	OSStatus			err = noErr;

	HIViewSetNeedsDisplay( inData->view, true );

	return err;
}

// -----------------------------------------------------------------------------
//	CalendarViewGetData
// -----------------------------------------------------------------------------
//
OSStatus CalendarViewGetData(
	EventRef				inEvent,
	CalendarViewData*		inData )
{
	OSStatus				err;
	ControlPartCode			part;
	OSType					tag;
	Ptr						ptr;
	Size					size;
	Size					outSize;
	
	err = GetEventParameter( inEvent, kEventParamControlPart, typeControlPartCode,
			NULL, sizeof( ControlPartCode ), NULL, &part );
	require_noerr( err, ParameterMissing );

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
		case kControlCalendarTitleRatioTag:
			if ( size == sizeof( float ) )
				*( (float*) ptr ) = inData->titleRowRatio;
			else
				err = errDataSizeMismatch;
			outSize = sizeof( float );
			break;
			
		case kControlCalendarDayNameRatioTag:
			if ( size == sizeof( float ) )
				*( (float*) ptr ) = inData->dayNameRowRatio;
			else
				err = errDataSizeMismatch;
			outSize = sizeof( float );
			break;

		case kControlCalendarDayRatioTag:
			if ( size == sizeof( float ) )
				*( (float*) ptr ) = inData->dayRowRatio;
			else
				err = errDataSizeMismatch;
			outSize = sizeof( float );
			break;

		case kControlCalendarDateTag:
			if ( size == sizeof( CFGregorianDate ) )
				*( (CFGregorianDate*) ptr ) = inData->date;
			else
				err = errDataSizeMismatch;
			outSize = sizeof( CFGregorianDate );
			break;
		
		case kControlCalendarDrawProcTag:
			if ( size == sizeof( CalendarDrawUPP ) )
				*( (CalendarDrawUPP*) ptr ) = inData->drawProc;
			else
				err = errDataSizeMismatch;
			outSize = sizeof( CalendarDrawUPP );
			break;

		case kControlCalendarLabelProcTag:
			if ( size == sizeof( CalendarDrawUPP ) )
				*( (CalendarDrawUPP*) ptr ) = inData->labelProc;
			else
				err = errDataSizeMismatch;
			outSize = sizeof( CalendarDrawUPP );
			break;

		default:
			err = errDataNotSupported;
			outSize = 0;
			break;
	}

	if ( err == noErr )
		err = SetEventParameter( inEvent, kEventParamControlDataBufferSize, typeLongInteger,
				sizeof( Size ), &outSize );

ParameterMissing:
	return err;
}

// -----------------------------------------------------------------------------
//	CalendarViewSetData
// -----------------------------------------------------------------------------
//
OSStatus CalendarViewSetData(
	EventRef				inEvent,
	CalendarViewData*		inData )
{
	OSStatus				err;
	ControlPartCode			part;
	OSType					tag;
	Ptr						ptr;
	Size					size;
	
	err = GetEventParameter( inEvent, kEventParamControlPart, typeControlPartCode,
			NULL, sizeof( ControlPartCode ), NULL, &part );
	require_noerr( err, ParameterMissing );

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
		case kControlCalendarTitleRatioTag:
			if ( size == sizeof( float ) )
				 inData->titleRowRatio = *( (float*) ptr );
			else
				err = errDataSizeMismatch;
			break;
			
		case kControlCalendarDayNameRatioTag:
			if ( size == sizeof( float ) )
				inData->dayNameRowRatio = *( (float*) ptr );
			else
				 err = errDataSizeMismatch;
			break;

		case kControlCalendarDayRatioTag:
			if ( size == sizeof( float ) )
				inData->dayRowRatio = *( (float*) ptr );
			else
				err = errDataSizeMismatch;
			break;

		case kControlCalendarDateTag:
			if ( size == sizeof( CFGregorianDate ) )
			{
				inData->date.year = ( (CFGregorianDate*) ptr )->year;
				inData->date.month = ( (CFGregorianDate*) ptr )->month;
				SetUpDateData( inData );
				HIViewSetNeedsDisplay( inData->view, true );
			}
			else
			{
				err = errDataSizeMismatch;
			}
			break;
		
		case kControlCalendarDrawProcTag:
			if ( size == sizeof( CalendarDrawUPP ) )
				inData->drawProc = *( (CalendarDrawUPP*) ptr );
			else
				err = errDataSizeMismatch;
			break;

		case kControlCalendarLabelProcTag:
			if ( size == sizeof( CalendarDrawUPP ) )
				inData->labelProc = *( (CalendarDrawUPP*) ptr );
			else
				err = errDataSizeMismatch;
			break;

		default:
			err = errDataNotSupported;
			break;
	}

ParameterMissing:
	return err;
}

// -----------------------------------------------------------------------------
//	CalendarViewGetRegion
// -----------------------------------------------------------------------------
//
OSStatus CalendarViewGetRegion(
	EventRef				inEvent,
	CalendarViewData*		inData )
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
		HIViewGetBounds( inData->view, &bounds );
		qdBounds.top = (SInt16) CGRectGetMinY( bounds );
		qdBounds.left = (SInt16) CGRectGetMinX( bounds );
		qdBounds.bottom = (SInt16) CGRectGetMaxY( bounds );
		qdBounds.right = (SInt16) CGRectGetMaxX( bounds );
	
		RectRgn( outRegion, &qdBounds );
	}
	
ParameterMissing:
	return err;
}

// -----------------------------------------------------------------------------
//	FindPart
// -----------------------------------------------------------------------------
//
ControlPartCode FindPart(
	const HIRect*		inBounds,
	const HIPoint*		inWhere,
	CalendarViewData*	inData )
{
	ControlPartCode		part;
	HIRect				testRect;
	float				rowHeight, colWidth;
	float				rows = 0;
	int					cols;
	UInt16				dayCount = 0;
	float				rowCount = 6 * inData->dayRowRatio + inData->dayNameRowRatio + inData->titleRowRatio;

	rowHeight = inBounds->size.height / rowCount;
	colWidth = (int) ( inBounds->size.width / 14 );

	// Part is the month?
	testRect.origin = inBounds->origin;
	testRect.size.height = (int) ( rowHeight * inData->titleRowRatio );

	testRect.size.width = colWidth;
	require_action_quiet( !CGRectContainsPoint( testRect, *inWhere ), GotPart, part = kCalendarPreviousYearPart );
	testRect.origin.x += testRect.size.width;

	require_action_quiet( !CGRectContainsPoint( testRect, *inWhere ), GotPart, part = kCalendarPreviousMonthPart );
	testRect.origin.x += testRect.size.width;

	testRect.size.width = 10 * colWidth;
	require_action_quiet( !CGRectContainsPoint( testRect, *inWhere ), GotPart, part = kCalendarMonthPart );
	testRect.origin.x += testRect.size.width;

	testRect.size.width = colWidth;
	require_action_quiet( !CGRectContainsPoint( testRect, *inWhere ), GotPart, part = kCalendarNextMonthPart );
	testRect.origin.x += testRect.size.width;

	testRect.size.width = colWidth;
	require_action_quiet( !CGRectContainsPoint( testRect, *inWhere ), GotPart, part = kCalendarNextYearPart );

	testRect.origin.y += testRect.size.height;
	rows += inData->titleRowRatio;
	testRect.origin.x = inBounds->origin.x;
	
	if ( inData->dayNameRowRatio != 0 )
	{
		// Part is a weekday?
		testRect.size.height = (int) ( rowHeight * inData->dayNameRowRatio );
		testRect.size.width = inBounds->size.width;

		if ( CGRectContainsPoint( testRect, *inWhere ) )
		{
			part = kCalendarSundayNamePart;
			testRect.size.width = 2 * colWidth;
			for ( cols = 0; cols < 7; cols++ )
			{
				require_quiet( !CGRectContainsPoint( testRect, *inWhere ), GotPart );
				testRect.origin.x += testRect.size.width;
				part++;
			}
		}
		testRect.origin.y += testRect.size.height;
		rows += inData->dayNameRowRatio;
	}

	testRect.origin.x = inBounds->origin.x;
	testRect.size.height = (int) ( rowHeight * inData->dayRowRatio );

	// Part is a calendar square?
	part = 0;
	for ( ; rows < rowCount; rows += inData->dayRowRatio )
	{
		testRect.size.width = inBounds->size.width;
		if ( CGRectContainsPoint( testRect, *inWhere ) )
		{
			testRect.size.width = 2 * colWidth;
			for ( cols = 0; cols < 7; cols++ )
			{
				if ( dayCount++ >= inData->firstDay )
					part++;
				require_quiet( part <= inData->daysInMonth, PostDate );
				require_quiet( !CGRectContainsPoint( testRect, *inWhere ), GotPart );
				testRect.origin.x += testRect.size.width;
			}
		}
		if ( dayCount == 0 )
			part += 7 - inData->firstDay;
		else
			part += 7;
		dayCount += 7;
		testRect.origin.y += testRect.size.height;
		testRect.origin.x = inBounds->origin.x;
	}
	
PostDate:
	part = kControlNoPart;

GotPart:
	return part;
}

// -----------------------------------------------------------------------------
//	DefaultDrawPart
// -----------------------------------------------------------------------------
//
pascal void DefaultDrawPart(
	ControlPartCode			inPart,
	const HIRect*			inPartRect,
	const CalendarDrawData*	inData )
{
	switch ( inPart )
	{
		case kControlStructureMetaPart:
			#if DONT_USE_SHADING
				CGContextSetRGBFillColor( inData->context, .95, .95, .95, 1 );
				CGContextFillRect( inData->context, *inPartRect );
			#elif 1
				ShadeRect( .8, .95, inPartRect, inData->context );
			#else
			{
				CGRGB	start = { 1, 0, 0 };
				CGRGB	end = { 0, 0, 1 };
				ShadeRectColor( &start, &end, inPartRect, inData->context );
			}
			#endif
			break;
		
		case kCalendarMonthPart:
		case kCalendarPreviousMonthPart:
		case kCalendarNextMonthPart:
		case kCalendarPreviousYearPart:
		case kCalendarNextYearPart:
		default:
			if ( inData->hilitePart && inData->hilitePart == inPart )
			{
				RGBColor	color;
				GetThemeBrushAsColor( kThemeBrushPrimaryHighlightColor, 32, true, &color );
				CGContextSetRGBFillColor( inData->context, (float) color.red / 65536,
						(float) color.green / 65536, (float) color.blue / 65536, 1 );
				CGContextFillRect( inData->context, *inPartRect );
			}
			break;
	}
}

// -----------------------------------------------------------------------------
//	DefaultDrawPartLabel
// -----------------------------------------------------------------------------
//
pascal void DefaultDrawPartLabel(
	ControlPartCode			inPart,
	const HIRect*			inPartRect,
	const CalendarDrawData*	inData )
{
	Rect					qdBounds;
	CFStringRef				string;
	
	// Set up a quickdraw rectangle for DrawThemeTextBox
	qdBounds.top = (SInt16) inPartRect->origin.y;
	qdBounds.left = (SInt16) inPartRect->origin.x;
	qdBounds.bottom = qdBounds.top + (SInt16) inPartRect->size.height;
	qdBounds.right = qdBounds.left + (SInt16) inPartRect->size.width;

	CGContextSetRGBFillColor( inData->context, 0, 0, 0, 1 );
	
	switch ( inPart )
	{
		case kCalendarMonthPart:
			{
			CFStringRef	months[] = {
					CFSTR( "January" ),
					CFSTR( "February" ),
					CFSTR( "March" ),
					CFSTR( "April" ),
					CFSTR( "May" ),
					CFSTR( "June" ),
					CFSTR( "July" ),
					CFSTR( "August" ),
					CFSTR( "September" ),
					CFSTR( "October" ),
					CFSTR( "November" ),
					CFSTR( "December" ) };
			CFMutableStringRef	dateString;
			
			dateString = CFStringCreateMutableCopy( NULL, 16, months[ inData->date.month-1 ] );
			CFStringAppend( dateString, CFSTR( " " ) );
			CFStringAppendFormat( dateString, NULL, CFSTR( "%d" ), inData->date.year );

			DrawThemeTextBox( dateString, kThemeSystemFont,
					kThemeStateActive, false, &qdBounds, teJustCenter, inData->context );
			
			CFRelease( dateString );
			}
			break;

		case kCalendarPreviousYearPart:
			DrawThemeTextBox( CFSTR( "<<" ), kThemeEmphasizedSystemFont,
					kThemeStateActive, false, &qdBounds, teJustCenter, inData->context );
			break;

		case kCalendarPreviousMonthPart:
			DrawThemeTextBox( CFSTR( "<" ), kThemeSystemFont,
					kThemeStateActive, false, &qdBounds, teJustCenter, inData->context );
			break;

		case kCalendarNextMonthPart:
			DrawThemeTextBox( CFSTR( ">" ), kThemeSystemFont,
					kThemeStateActive, false, &qdBounds, teJustCenter, inData->context );
			break;

		case kCalendarNextYearPart:
			DrawThemeTextBox( CFSTR( ">>" ), kThemeEmphasizedSystemFont,
					kThemeStateActive, false, &qdBounds, teJustCenter, inData->context );
			break;

		case kCalendarSundayNamePart:
		case kCalendarMondayNamePart:
		case kCalendarTuesdayNamePart:
		case kCalendarWednesdayNamePart:
		case kCalendarThursdayNamePart:
		case kCalendarFridayNamePart:
		case kCalendarSaturdayNamePart:
			{
				CFStringRef	dow[][7] = {
						{ CFSTR( "SUN" ), CFSTR( "MON" ), CFSTR( "TUE" ), CFSTR( "WED" ), CFSTR( "THU" ), CFSTR( "FRI" ), CFSTR( "SAT" ) },
						{ CFSTR( "Su" ), CFSTR( "Mo" ), CFSTR( "Tu" ), CFSTR( "We" ), CFSTR( "Th" ), CFSTR( "Fr" ), CFSTR( "Sa" ) },
						{ CFSTR( "S" ), CFSTR( "M" ), CFSTR( "T" ), CFSTR( "W" ), CFSTR( "T" ), CFSTR( "F" ), CFSTR( "S" ) } };
				
				DrawThemeTextBox( dow[ 2 ][ inPart % kCalendarSundayNamePart ], kThemeSystemFont,
						kThemeStatePressed, false, &qdBounds, teJustCenter, inData->context );
			}
			break;
		
		default:
			if ( inPart >= 1 && inPart <= inData->daysInMonth )
			{
				string = CFStringCreateWithFormat( NULL, NULL, CFSTR( "%d" ), inPart );
		
				DrawThemeTextBox( string, kThemeSystemFont,
						kThemeStatePressed, false, &qdBounds, teJustRight, inData->context );
				
				CFRelease( string );
			}
	}
}

// -----------------------------------------------------------------------------
//	SetUpDateData
// -----------------------------------------------------------------------------
//
void SetUpDateData(
	CalendarViewData*	inData )
{
	CFGregorianDate		tempDate;
	CFAbsoluteTime		tempTime;
	CFGregorianUnits	nextMonthMinusADay = { 0, 1, -1, 0, 0, 0 };

	// What is the first day of this month?
	tempTime = CFGregorianDateGetAbsoluteTime( inData->date, inData->timeZone );
	inData->firstDay = CFAbsoluteTimeGetDayOfWeek( tempTime, inData->timeZone ) % 7;

	// How many days in this month?
	tempTime = CFAbsoluteTimeAddGregorianUnits( tempTime, inData->timeZone, nextMonthMinusADay );
	tempDate = CFAbsoluteTimeGetGregorianDate( tempTime, inData->timeZone );
	inData->daysInMonth = tempDate.day;
}

// -----------------------------------------------------------------------------
//	NewCalendarDrawUPP
// -----------------------------------------------------------------------------
//
CalendarDrawUPP NewCalendarDrawUPP(
	CalendarDrawProcPtr		inRoutine)
{
	/* Just do a magic cast since UPP == ProcPtr on X */
	return (CalendarDrawUPP) inRoutine;
}

// -----------------------------------------------------------------------------
//	DisposeCalendarDrawUPP
// -----------------------------------------------------------------------------
//
void DisposeCalendarDrawUPP(
	CalendarDrawUPP			inUPP)
{
#pragma unused( inUPP )
	/* do nothing since UPP == ProcPtr on X */
}

// -----------------------------------------------------------------------------
//	InvokeCalendarDrawUPP
// -----------------------------------------------------------------------------
//
void InvokeCalendarDrawUPP(
	ControlPartCode			inPart,
	const HIRect*			inPartRect,
	const CalendarDrawData*	inData,
	CalendarDrawUPP			inUPP)
{
	/* Just do a magic cast since UPP == ProcPtr on X */
	( *( (CalendarDrawProcPtr) inUPP) )( inPart, inPartRect, inData );
}
