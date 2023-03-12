/*
	File:		THexEditorView.cp
	
	Version:	1.0

	Disclaimer:	IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc.
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

#include "THexEditorView.h"
#include "THexEditorScrollableView.h"

// This is the class ID that we've assigned to the HexEditorView
#define kHexEditorClassID	CFSTR("com.apple.sample.hexeditorview")

//--------------------------------------------------------------------
//	CONSTANTS
//--------------------------------------------------------------------
// const data has internal linkage, so these will not pollute the
// global namespace
//
const ControlKind 	kHexEditorViewKind	= { 'Samp', 'HxVw' };

const short			kHeaderTextOffset 	= 8;
const float			kHeaderHeight		= 20.0;

// View defined Carbon Events that are used internally to this view
enum
{
	kEventParamDataBuffer	= 'Data',		// typeCFDataRef
	
	typeCFDataRef			= 'CFDt'
};

// Events that this view is interested in
const EventTypeSpec	kEvents[] =
{
	{ kEventClassScrollableHexEditorView, kEventHexEditorSelectionChanged }
};

//--------------------------------------------------------------------
//	THexEditorView
//--------------------------------------------------------------------
//
THexEditorView::THexEditorView( ControlRef inControl )
	:
	TView( inControl ),
	fScrollView( NULL ),
	fScrollableHexView( NULL ),
	fEventHandler( NULL )
{
}

//--------------------------------------------------------------------
//	THexEditorView::THexEditorView
//--------------------------------------------------------------------
//	Clean up
//
THexEditorView::~THexEditorView()
{
	if ( fScrollableHexView != NULL )
		CFRelease( fScrollableHexView );
		
	if ( fScrollView != NULL )
		CFRelease( fScrollView );
		
	if ( fEventHandler != NULL )
		RemoveEventHandler( fEventHandler );
}

//--------------------------------------------------------------------
//	Create												API
//--------------------------------------------------------------------
//	This is the main creation API to create an instance of this
//	class.
//
OSStatus
THexEditorView::Create(
	CFDataRef		inData,
	HIViewRef*		outControl )
{
	OSStatus		err;
	EventRef		event = CreateInitializationEvent();
	
	// Register this class with the HIView system
	RegisterClass();
	
	verify_noerr( SetEventParameter(
						event,
						kEventParamDataBuffer,
						typeCFDataRef,
						sizeof( CFDataRef ),
						&inData ) );

	err = _HIObjectCreate( kHexEditorClassID, event, (HIObjectRef*)outControl );
	
	ReleaseEvent( event );

	return err;
}

//--------------------------------------------------------------------
//	SetCFData													API
//--------------------------------------------------------------------
//	Update the hex editor view to display this new chunk of data
//
OSStatus
THexEditorView::SetCFData(
	HIViewRef		inView,
	CFDataRef		inData )
{
	OSStatus		err;
	THexEditorView*	view;
	
	// verify that we have valid parameters
	require_action( inView != NULL, SetData_BadParam_NoView, err = paramErr );
	
	// cast the given view to an instance of this class so we can call its
	// methods
	view = (THexEditorView*)::HIObjectDynamicCast(
									(HIObjectRef)inView,
									kHexEditorClassID );
	require_action( view != NULL, SetData_NoView, err = paramErr );
	
	err = view->SetCFData( inData );
	
SetData_NoView:
SetData_BadParam_NoView:
	return err;
}

//--------------------------------------------------------------------
//	GetCFData												API
//--------------------------------------------------------------------
//	This is the API to get the CFData from this view
//
OSStatus
THexEditorView::GetCFData(
	HIViewRef		inView,
	CFDataRef*		outData )
{
	OSStatus		err;
	THexEditorView*	view;
	
	// verify that we have valid parameters
	require_action( inView != NULL, GetData_BadParam_NoView, err = paramErr );
	require_action( outData != NULL, GetData_BadParam_NoOutData, err = paramErr );
	
	// cast the given view to an instance of this class so we can call its
	// methods
	view = (THexEditorView*)::HIObjectDynamicCast(
								(HIObjectRef)inView,
								kHexEditorClassID );
	require_action( view != NULL, GetData_CantGetView, err = paramErr );
	
	err = view->GetCFData( outData );
	
GetData_CantGetView:
GetData_BadParam_NoOutData:
GetData_BadParam_NoView:
	return err;
}

//--------------------------------------------------------------------
//	RegisterClass
//--------------------------------------------------------------------
//
void
THexEditorView::RegisterClass()
{
	static	Boolean sRegistered;
	
	if ( not sRegistered )
	{
		TView::RegisterSubclass( kHexEditorClassID, Construct );
		sRegistered = true;
	}
}

//--------------------------------------------------------------------
//	SetCFData
//--------------------------------------------------------------------
//
OSStatus
THexEditorView::SetCFData( CFDataRef inData )
{
	OSStatus		err;
	
	require_action( fScrollableHexView != NULL, SetData_NoScrollableView, err = paramErr );
	
	err = THexEditorScrollableView::SetCFData( fScrollableHexView, inData );
	
SetData_NoScrollableView:
	return err;
}

//--------------------------------------------------------------------
//	GetCFData
//--------------------------------------------------------------------
//
OSStatus
THexEditorView::GetCFData( CFDataRef* outData )
{
	OSStatus	err;
	
	check( outData != NULL );
	require_action( fScrollableHexView != NULL, GetData_NoScrollableView, err = paramErr );
	
	err = THexEditorScrollableView::GetCFData( fScrollableHexView, outData );
	
GetData_NoScrollableView:
	return err;
}

//--------------------------------------------------------------------
//	Initialize
//--------------------------------------------------------------------
//	Create the scrollable sub view and embed it in this view
//
OSStatus
THexEditorView::Initialize( TCarbonEvent& inEvent )
{
	OSStatus	err 			= noErr;
	CFDataRef	data 			= NULL;
	
	// Install a handler for the events that this view is interested
	// in
	verify_noerr( InstallEventHandler(
						GetEventTarget(),
						EventHandler,
						GetEventTypeCount( kEvents ),
						kEvents,
						this,
						&fEventHandler ) );
	
	// It's okay if this parameter doesn't exist, it will leave data untouched
	inEvent.GetParameter( kEventParamDataBuffer, typeCFDataRef, sizeof( CFDataRef ), &data );
	
	// Now create a scroll view that we will embed the scrollable view in
	err = HIScrollViewCreate( kHIScrollViewOptionsVertScroll, &fScrollView );
	require_noerr( err, Initialize_CantCreateScrollView );
	
	// Embed the scroll view in our view
	verify_noerr( HIViewAddSubview( GetViewRef(), fScrollView ) );
	
	// This class doesn't actually deal with the data, it just passes it along to 
	// the subordinate scroller class, which we create here
	err = THexEditorScrollableView::Create( data, &fScrollableHexView );
	require_noerr( err, Initialize_CantCreateScrollableHexView );
	
	// Embed the scrollable view in the scroll view and make it visible
	verify_noerr( HIViewAddSubview( fScrollView, fScrollableHexView ) );
	HIViewSetVisible( fScrollableHexView, true );
	HIViewSetVisible( fScrollView, true );
	
Initialize_CantCreateScrollableHexView:
Initialize_CantCreateScrollView:
	return err;
}
								
//--------------------------------------------------------------------
//	Draw
//--------------------------------------------------------------------
//
void
THexEditorView::Draw(
	RgnHandle 			limitRgn,
	CGContextRef		inContext )
{
	TRect				bounds = Bounds();
	Rect				textBox = bounds;
	CFStringRef			dataSizeString;
	CFStringRef			dataSelectionString;
	Point				textBounds;
	SInt16				textBaseline;
	
#pragma unused( limitRgn )

	check( inContext != NULL );
	
	DrawHeaderBackground( inContext );
	
	// get the strings for the data size and selection
	dataSizeString = CreateDataSizeString();
	require( dataSizeString != NULL, Draw_CantGetDataString );
	
	dataSelectionString = CreateDataSelectionString();
	require( dataSelectionString != NULL, Draw_CantGetSelectionString );

	// get the dimensions of the data size string
	verify_noerr( GetThemeTextDimensions(
					dataSizeString,
					kThemeSmallSystemFont,
					kThemeStateActive,
					false,
					&textBounds,
					&textBaseline ) );

	textBox.left += kHeaderTextOffset;
	
	// draw the dataSize string into the header area
	verify_noerr( DrawThemeTextBox(
					dataSizeString,
					kThemeSmallSystemFont,
					kThemeStateActive,
					false,
					&textBox,
					teFlushDefault,
					inContext ) );

	// move the box past the end of the data size string (plus a little padding)
	textBox.left += (textBounds.h + kHeaderTextOffset);
	
	// now draw the data selection string into this box
	verify_noerr( DrawThemeTextBox(
					dataSelectionString,
					kThemeSmallSystemFont,
					kThemeStateActive,
					false,
					&textBox,
					teFlushDefault,
					inContext ) );

	// release our references to these strings, we're done with them
	CFRelease( dataSelectionString );

Draw_CantGetSelectionString:

	CFRelease( dataSizeString );
	
Draw_CantGetDataString:

	// Draw a rectangle around the content (or scrollable view)
	CGContextMoveToPoint( inContext, bounds.MinX(), bounds.MinY() );
	CGContextAddLineToPoint( inContext, bounds.MinX(), bounds.MaxY() );
	CGContextAddLineToPoint( inContext, bounds.MaxX(), bounds.MaxY() );
	CGContextAddLineToPoint( inContext, bounds.MaxX(), bounds.MinY() );
	CGContextAddLineToPoint( inContext, bounds.MinX(), bounds.MinY() );
	CGContextMoveToPoint( inContext, bounds.MinX(), bounds.MinY() + GetHeaderHeight() );
	CGContextAddLineToPoint( inContext, bounds.MaxX(), bounds.MinY() + GetHeaderHeight() );
	CGContextStrokePath( inContext );
}


//--------------------------------------------------------------------
//	HitTest
//--------------------------------------------------------------------
//
ControlPartCode
THexEditorView::HitTest( const HIPoint& where )
{
	if ( CGRectContainsPoint( Bounds(), where ) )
		return 1;
	else
		return kControlNoPart;
}

//--------------------------------------------------------------------
//	GetBehaviors
//--------------------------------------------------------------------
//	We must respond to this and indicate that this control supports
//	embedding because we want to embed the scroll view within this view
//
UInt32
THexEditorView::GetBehaviors()
{
	return TView::GetBehaviors() | kControlSupportsEmbedding;
}

#pragma mark --- HANDLERS ---

//--------------------------------------------------------------------
//	BoundsChanged
//--------------------------------------------------------------------
//	Called when the view's bounds have changed
//
OSStatus
THexEditorView::BoundsChanged(
	UInt32				inOptions,
	const HIRect&		inOriginalBounds,
	const HIRect&		inCurrentBounds,
	RgnHandle			inInvalRgn )
{
	OSStatus			err;
	TRect				bounds;
	float				headerHeight = GetHeaderHeight();
	
#pragma unused( inOptions, inOriginalBounds, inInvalRgn )

	check( fScrollView != NULL );
	
	// leave enough room for our header
	bounds.Set(
		inCurrentBounds.origin.x,
		inCurrentBounds.origin.y + headerHeight,
		inCurrentBounds.size.width,
		inCurrentBounds.size.height - headerHeight );
	
	// embed the scroll view _within_ this view. Inset by a pixel.
	bounds.Inset( 1.0, 1.0 );
	
	// set the scroll view to be this "content" rect
	err = HIViewSetFrame( fScrollView, &bounds );
	require_noerr( err, BoundsChanged_CantSetFrame );
		
BoundsChanged_CantSetFrame:
	return err;
}

//--------------------------------------------------------------------
//	GetKind
//--------------------------------------------------------------------
//
ControlKind
THexEditorView::GetKind()
{
	return kHexEditorViewKind;
}

#pragma mark --- PRIVATE METHODS ---

//--------------------------------------------------------------------
//	GetHeaderHeight
//--------------------------------------------------------------------
//
float
THexEditorView::GetHeaderHeight()
{
	return kHeaderHeight;
}

//--------------------------------------------------------------------
//	CreateDataSizeString
//--------------------------------------------------------------------
//	Create a string that contains the length of the contained data
//
CFStringRef
THexEditorView::CreateDataSizeString()
{
	OSStatus		err;
	CFStringRef		sizeString = NULL;
	long			dataSize;
	
	require_quiet( fScrollableHexView != NULL, CreateDataSizeString_NoScrollableView );

	// get the data size from the scrollable hex view instance
	err = THexEditorScrollableView::GetDataSize(
				fScrollableHexView,
				&dataSize );
	require_noerr( err, CreateDataSizeString_CantGetSize );
	
	// create a string that contains this information
	sizeString = CFStringCreateWithFormat( kCFAllocatorDefault, NULL, CFSTR("Data size: %ld"), dataSize );
	
CreateDataSizeString_CantGetSize:
CreateDataSizeString_NoScrollableView:
	
	return sizeString;
}

//--------------------------------------------------------------------
//	CreateDataSelectionString
//--------------------------------------------------------------------
//	Create a string that specifies the current selection
//
CFStringRef
THexEditorView::CreateDataSelectionString()
{
	OSStatus		err;
	CFStringRef		selectionString = NULL;
	long			selectionStart;
	long			selectionEnd;
	
	require_quiet( fScrollableHexView != NULL, CreateSelectionString_NoScrollableView );
	
	// get the selection from the scrollable hex view instance
	err = THexEditorScrollableView::GetDataSelection(
				fScrollableHexView,
				&selectionStart,
				&selectionEnd );
	require_noerr( err, CreateSelectionString_CantGetSelection );

	// if it is an empty selection indicate that the cursor is set up
	// for insertion, otherwise indicate what the selection is
	if ( selectionStart == selectionEnd )
	{
		selectionString = CFStringCreateWithFormat(
								kCFAllocatorDefault,
								NULL,
								CFSTR("Insertion: %ld"),
								selectionStart );
	}
	else
	{
		selectionString = CFStringCreateWithFormat(
								kCFAllocatorDefault,
								NULL,
								CFSTR("Selection: [ %ld, %ld )  %ld bytes"),
								selectionStart,
								selectionEnd,
								selectionEnd - selectionStart );
	}

CreateSelectionString_CantGetSelection:
CreateSelectionString_NoScrollableView:

	return selectionString;
}

//--------------------------------------------------------------------
//	DrawHeaderBackground
//--------------------------------------------------------------------
//
void
THexEditorView::DrawHeaderBackground( CGContextRef inContext )
{
	TRect	headerBounds = Bounds();
	
	headerBounds.SetHeight( GetHeaderHeight() );
	
	// Fill in with gray
	::CGContextSetRGBFillColor( 
			inContext,
			0.9,
			0.9,
			0.9,
			1.0 );
			
	::CGContextFillRect( inContext, headerBounds );
	
	// Restore with black
	::CGContextSetRGBFillColor(
			inContext,
			0.0,
			0.0,
			0.0,
			1.0 );
}

#pragma mark --- CLASS HANDLERS ---

//--------------------------------------------------------------------
//	Construct
//--------------------------------------------------------------------
//		
OSStatus
THexEditorView::Construct(
	ControlRef		inControl,
	TView**			outView )
{
	*outView = new THexEditorView( inControl );
	
	if ( *outView != NULL )
		return noErr;
	else
		return memFullErr;	// could be a lie
}

//--------------------------------------------------------------------
//	EventHandler
//--------------------------------------------------------------------
//
pascal OSStatus
THexEditorView::EventHandler(
	EventHandlerCallRef	inCallRef,
	EventRef			inEvent,
	void*				inUserData )
{
	OSStatus			err = eventNotHandledErr;
	THexEditorView*		view = (THexEditorView*)inUserData;
	
#pragma unused( inCallRef )

	// The only event registered for the hex editor scrollable class
	// is the notification that the selection has changed. This requires
	// an update of the view. Just invalidate. We can be smarter if
	// we wanted to be.
	if ( GetEventClass( inEvent ) == kEventClassScrollableHexEditorView )
	{
		view->Invalidate();
		err = noErr;
	}
	
	return err;
}
