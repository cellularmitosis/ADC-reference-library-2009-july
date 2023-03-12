/*
	File:		THexEditorScrollableView.cp
	
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

#include <ctype.h>	// for toascii
#include "THexEditorScrollableView.h"

#if __MWERKS__
	#warning Metrowerks doesnt implement toascii. Bypassing it.
	#define toascii(c) (c)
#endif

// This is the class ID that we've assigned to this view
#define kHexEditorScrollClassID	CFSTR("com.apple.sample.hexeditorscrollview")

//--------------------------------------------------------------------
//	CONSTANTS
//--------------------------------------------------------------------
// const data has internal linkage, so these will not pollute the
// global namespace
//
const ControlKind kHexEditorScrollableViewKind = { 'Samp', 'HxSV' };

static const EventTypeSpec	kEvents[] =
{
	{ kEventClassScrollable, kEventScrollableGetInfo },
	{ kEventClassScrollable, kEventScrollableScrollTo }
};

// Some of the character metric constants below are just guesses of the actual metrics
// that work in this case.
// You wouldn't want to do this in shipping software -- you would instead want to get
// the metrics from ATS for the Courier font and use that.
const float kCharacterWidth			= 7.0;				// character width (just a guess)
const float	kCharacterHeight	 	= 12.0;				// the character height (just a guess)
const char	kFontName[]				= "Courier";		// font to draw with
const int	kBytesPerColumn			= 2;				// number of bytes per hex column
const int	kCharactersPerByte		= 2;				// characters required per byte in the hex column
const float	kColumnSeparatorPad		= 10.0;				// padding required for the column separators
const float kHexColumnPad			= 5.0;				// padding for the hex column entries
const float	kRowPad					= 2.0;				// vertical padding for each row
const float kFontSize				= 12.0;				// the font size to draw the font in
const float kColumnOffsetMaxChars	= 10;				// max number of characters to draw in the offsest column
const long	kNoSelection			= -1;				// indicate that there is no selection

const float	kRowHeight				= kCharacterHeight + kRowPad;
const float	kHexColumnWidth			= ( kCharacterWidth * ( kBytesPerColumn * kCharactersPerByte ) ) + kHexColumnPad;
const float	kColumnOffsetWidth		= kCharacterWidth * kColumnOffsetMaxChars;

const EventTimerInterval kCaretBlinkFrequency	= kEventDurationSecond / 2.0;

// View defined Carbon Event types which are used in this translation unit
enum
{
	kEventParamDataBuffer	= 'Data',		// typeCFDataRef
	
	typeCFDataRef			= 'CFDt'
};

//--------------------------------------------------------------------
//	THexEditorScrollableView
//--------------------------------------------------------------------
//
THexEditorScrollableView::THexEditorScrollableView( ControlRef inControl )
	:
	TView( inControl ),
	fData( NULL ),
	fSelectionStart( kNoSelection ),
	fSelectionEnd( kNoSelection ),
	fCaretState( kCaretStateOff ),
	fCaretColumn( kControlNoPart ),
	fHalfByteInsertion( false ),
	fCaretTimer( NULL ),
	fHandler( NULL )
{
	fImageOrigin.x = fImageOrigin.y = 0;
	
	for ( int index = 0; index < kDataColumnCount; index++ )
		fDataColumnWidths[ index ] = 0;
	
	verify_noerr( ::InstallEventHandler(
						GetEventTarget(),
						EventHandler,
						GetEventTypeCount( kEvents ),
						kEvents,
						this,
						&fHandler ) );	
}

//--------------------------------------------------------------------
//	THexEditorScrollableView::THexEditorScrollableView
//--------------------------------------------------------------------
//
THexEditorScrollableView::~THexEditorScrollableView()
{
	if ( fHandler != NULL )
		verify_noerr( ::RemoveEventHandler( fHandler ) );
		
	if ( fCaretTimer != NULL )
		verify_noerr( ::RemoveEventLoopTimer( fCaretTimer ) );
}

//--------------------------------------------------------------------
//	Create													API
//--------------------------------------------------------------------
//	This is the creation API to get an instance of this class
//
OSStatus
THexEditorScrollableView::Create(
	CFDataRef			inData,
	HIViewRef*	 		outControl )
{
	OSStatus			err;
	EventRef			event = CreateInitializationEvent();
	
	RegisterClass();

	// store the given data in the creation event
	verify_noerr( ::SetEventParameter(
						event,
						kEventParamDataBuffer,
						typeCFDataRef,
						sizeof( CFMutableDataRef ),
						&inData ) );

	err = ::_HIObjectCreate( kHexEditorScrollClassID, event, (HIObjectRef*)outControl );
	
	::ReleaseEvent( event );

	return err;
}

//--------------------------------------------------------------------
//	RegisterClass
//--------------------------------------------------------------------
//
void
THexEditorScrollableView::RegisterClass()
{
	static	Boolean sRegistered;
	
	if ( not sRegistered )
	{
		TView::RegisterSubclass( kHexEditorScrollClassID, Construct );
		sRegistered = true;
	}
}

//--------------------------------------------------------------------
//	GetDataSize												API
//--------------------------------------------------------------------
//	API to return the data size of the given instance
//
OSStatus
THexEditorScrollableView::GetDataSize(
	HIViewRef					inView,
	long*						outSize )
{
	OSStatus					err = noErr;
	THexEditorScrollableView*	view;
	
	// verify that there are valid parameters
	require_action( inView != NULL, GetDataSize_BadParam_NoView, err = paramErr );
	require_action( outSize != NULL, GetDataSize_BadParam_NoSize, err = paramErr );
	
	// cast the given view to an instance of this class so we can call
	// its methods
	view = (THexEditorScrollableView*)::HIObjectDynamicCast(
											(HIObjectRef) inView,
											kHexEditorScrollClassID );
	require_action( view != NULL, GetDataSize_NoView, err = paramErr );

	// get the data size
	*outSize = view->GetDataLength();

GetDataSize_NoView:
GetDataSize_BadParam_NoSize:	
GetDataSize_BadParam_NoView:

	return err;
}

//--------------------------------------------------------------------
//	GetDataSelection									API
//--------------------------------------------------------------------
//	API to return the current selection of this instance
//
OSStatus
THexEditorScrollableView::GetDataSelection(
	HIViewRef					inView,
	long*						outSelectionStart,
	long*						outSelectionEnd )
{
	OSStatus					err = noErr;
	THexEditorScrollableView*	view;
	
	// verify the parameters
	require_action( inView != NULL, GetDataSelection_NoView, err = paramErr );
	require_action( outSelectionStart != NULL, GetDataSelection_NoSelection, err = paramErr );
	require_action( outSelectionEnd != NULL, GetDataSelection_NoSelection, err = paramErr );
	
	// cast the view to an instance of this class so we can call its 
	// methods
	view = (THexEditorScrollableView*)::HIObjectDynamicCast(
											(HIObjectRef) inView,
											kHexEditorScrollClassID );
	require_action( view != NULL, GetDataSelection_NoView, err = paramErr );
	
	// get the selection
	view->GetSelection( outSelectionStart, outSelectionEnd );
	
GetDataSelection_NoSelection:
GetDataSelection_NoView:

	return err;
}

//--------------------------------------------------------------------
//	SetCFData												API
//--------------------------------------------------------------------
//	API to get the data from this view and return it to the caller
//
OSStatus
THexEditorScrollableView::SetCFData(
	HIViewRef					inView,
	CFDataRef					inData )
{
	OSStatus					err = noErr;
	THexEditorScrollableView*	view;
	
	// verify the parameters
	require_action( inView != NULL, SetData_NoView, err = paramErr );
	require_action( inData != NULL, SetData_NoOutData, err = paramErr );
	
	// cast the view to an instance of this class so we can call
	// its methods
	view = (THexEditorScrollableView*)::HIObjectDynamicCast(
											(HIObjectRef) inView,
											kHexEditorScrollClassID );
	require_action( view != NULL, SetData_NoView, err = paramErr );
	
	// copy the data and return it to the caller
	view->SetCFData( inData );
	
SetData_NoOutData:
SetData_NoView:
	return err;
}

//--------------------------------------------------------------------
//	GetCFData												API
//--------------------------------------------------------------------
//	API to get the data from this view and return it to the caller
//
OSStatus
THexEditorScrollableView::GetCFData(
	HIViewRef					inView,
	CFDataRef*					outData )
{
	OSStatus					err = noErr;
	THexEditorScrollableView*	view;
	
	// verify the parameters
	require_action( inView != NULL, GetData_NoView, err = paramErr );
	require_action( outData != NULL, GetData_NoOutData, err = paramErr );
	
	// cast the view to an instance of this class so we can call
	// its methods
	view = (THexEditorScrollableView*)::HIObjectDynamicCast(
											(HIObjectRef) inView,
											kHexEditorScrollClassID );
	require_action( view != NULL, GetData_NoView, err = paramErr );
	
	// copy the data and return it to the caller
	*outData = view->GetCFData();
	
GetData_NoOutData:
GetData_NoView:
	return err;
}

//--------------------------------------------------------------------
//	ScrollTo
//--------------------------------------------------------------------
//	Scroll to the given point. Validate the given point to make sure
//	we aren't scrolling to an invalid point -- for sanity
//
void
THexEditorScrollableView::ScrollTo( const HIPoint& where )
{
	float		dX, dY;
	
	if ( where.x < 0.0 )
	{
		dX = -fImageOrigin.x;
		fImageOrigin.x = 0.0;
	}
	else
	{
		dX = fImageOrigin.x - where.x;
		fImageOrigin.x = where.x;
	}
	
	if ( where.y < 0.0 )
	{
		dY = -fImageOrigin.y;
		fImageOrigin.y = 0;
	}
	else
	{
		dY = fImageOrigin.y - where.y;
		fImageOrigin.y = where.y;
	}

	::HIViewScrollRect( GetViewRef(), NULL, dX, dY );
}

//--------------------------------------------------------------------
//	ImageSize
//--------------------------------------------------------------------
//	Return the ideal size of this view
//
HISize
THexEditorScrollableView::ImageSize()
{	
	HISize		size = {0, 0};
	TRect		bounds = Bounds();
	long		virtualRowCount;
	
	check( BytesPerRow() > 0 );
	
	// the ideal width will fit the width of the number of columns
	size.width = fDataColumnWidths[ kDataColumnOffset ] + fDataColumnWidths[ kDataColumnHex ] + fDataColumnWidths[ kDataColumnChar ];
	
	// the ideal height will fit the number of rows required to fit all the bytes
	virtualRowCount = (long)ceil( GetDataLength() / (float)BytesPerRow() );
	size.height = (virtualRowCount * kRowHeight) + kRowPad;
	
	return size;
}

//--------------------------------------------------------------------
//	ImageOrigin
//--------------------------------------------------------------------
//
HIPoint
THexEditorScrollableView::ImageOrigin()
{	
	return fImageOrigin;
}

//--------------------------------------------------------------------
//	ViewSize
//--------------------------------------------------------------------
//
HISize
THexEditorScrollableView::ViewSize()
{
	return Bounds().size;
}

//--------------------------------------------------------------------
//	LineSize
//--------------------------------------------------------------------
//
HISize
THexEditorScrollableView::LineSize()
{
	HISize		size = { TRect( Bounds() ).Width(), kRowHeight };
	
	return size;
}

//--------------------------------------------------------------------
//	GetCFData
//--------------------------------------------------------------------
//
CFDataRef
THexEditorScrollableView::GetCFData()
{
	return fData;
}

//--------------------------------------------------------------------
//	CopyCFData
//--------------------------------------------------------------------
//	Create a copy of the CFData from this instance and return it
//
CFDataRef
THexEditorScrollableView::CopyCFData()
{
	CFDataRef	data = NULL;
	
	if ( fData != NULL )
		data = ::CFDataCreateCopy( kCFAllocatorDefault, fData );
	
	return data;
}

//--------------------------------------------------------------------
//	SetCFData
//--------------------------------------------------------------------
//
void
THexEditorScrollableView::SetCFData( CFDataRef inData )
{
	// out with the old
	if ( fData )
		CFRelease( fData );
	
	// in with the new
	fData = ::CFDataCreateMutableCopy( kCFAllocatorDefault, 0, inData );
	
	// Reset our member data
	ResetData();
	
	// we'll need to redraw
	SendViewInfoChangedNotification();
	Invalidate();
}

//--------------------------------------------------------------------
//	GetDataLength
//--------------------------------------------------------------------
//	Get the size of the data
//
long
THexEditorScrollableView::GetDataLength()
{
	return ( fData == NULL ) ? 0 : ::CFDataGetLength( fData );
}

//--------------------------------------------------------------------
//	GetSelection
//--------------------------------------------------------------------
//	Get the current selection
//
void
THexEditorScrollableView::GetSelection(
	long*		outSelectionStart,
	long*		outSelectionEnd )
{
	check( outSelectionStart != NULL );
	check( outSelectionEnd != NULL );

	*outSelectionStart = fSelectionStart == kNoSelection ? 0 : fSelectionStart;
	*outSelectionEnd = fSelectionEnd == kNoSelection ? 0 : fSelectionEnd;
}

//--------------------------------------------------------------------
//	Initialize
//--------------------------------------------------------------------
//
OSStatus
THexEditorScrollableView::Initialize( TCarbonEvent& inEvent )
{
	CFMutableDataRef	data = NULL;
	
	// It's okay if this parameter doesn't exist, it will leave data untouched
	inEvent.GetParameter( kEventParamDataBuffer, typeCFDataRef, sizeof( CFMutableDataRef ), &data );
	
	// Create a mutable copy for our own use. This way the client can dispose their 
	// copy and we can allow editing
	if ( data != NULL )
		fData = ::CFDataCreateMutableCopy( kCFAllocatorDefault, 0, data );
	
	// We also activate the keyboard interface so we get text input events
	return ActivateInterface( kKeyboardFocus );	
}

#pragma mark --- HANDLERS ---

//--------------------------------------------------------------------
//	Draw
//--------------------------------------------------------------------
//
void
THexEditorScrollableView::Draw(
	RgnHandle 			limitRgn,
	CGContextRef		inContext )
{
	long				byteOffset = 0;
	long				finalRow;
	long				rowsToDraw;
	float				verticalOffset;
	UInt8*				byteHead;
	long				bytesPerRow = BytesPerRow();
	Boolean				createdBuffer = false;
	
#pragma unused( limitRgn )

	check( inContext != NULL );
	
	// save the context
	::CGContextSaveGState( inContext );
	
	// prep the context
	PrepContext( inContext );
	
	// erase the background
	DrawBackground( inContext );
	
	// draw the separators between each column
	DrawColumnSeparators( inContext );
	
	// don't draw anything if there is nothing to draw
	require_quiet( fData != NULL, Draw_NothingToDraw );
	
	// figure out the draw starting point
	CalculateDrawOffsets( &byteOffset, &verticalOffset );

	// Determine how many rows we need to draw
	rowsToDraw = RowsToDraw();
	
	byteHead = (UInt8*)::CFDataGetBytePtr( fData );
	
	// If we got a pointer to the bytes, it's pointing to the start of the data,
	// so offset it by the offset count, if it's NULL, we need to get the data out into
	// a buffer.
	if ( byteHead != NULL )
	{
		byteHead += byteOffset;
	}
	else
	{
		byteHead = (UInt8*)malloc( bytesPerRow );
		if ( byteHead != NULL )
		{
			createdBuffer = true;
			::CFDataGetBytes( fData, ::CFRangeMake( byteOffset, bytesPerRow ), byteHead );
		}
	}
	require( byteHead != NULL, Draw_CantGetRowBuffer );
	
	finalRow = RowForByteOffset( byteOffset ) + rowsToDraw;
	
	// Draw all the visible rows
	for ( long row = RowForByteOffset( byteOffset ); row < finalRow; row++ )
	{
		char	offsetString[ 16 ];
		float	offsetStringOffset;
		long	bytesToDraw = DrawableRowBytes( byteOffset );
				
		// Get the offset string and drawing offset for this string
		GetOffsetStringAndOffset( byteOffset, offsetString, &offsetStringOffset );
		
		// Draw the highlight
		if ( not IsEmptySelection() )
		{
			long	highlightStart;
			long	highlightEnd;
			float	startOffset;
			float	endOffset;
			
			// Calculate the highlight region for this row
			CalculateRowHighlightSelection( row, &highlightStart, &highlightEnd );
			
			// Draw the highlight for the hex column
			startOffset = HexColumnXOffsetOfByteOffset( highlightStart ) + GetColumnOffset( kDataColumnHex );
			endOffset = HexColumnXOffsetOfByteOffset( highlightEnd ) + GetColumnOffset( kDataColumnHex );
			DrawRowHighlight( inContext, verticalOffset, startOffset, endOffset );
			
			// Draw the highlight for the char column
			startOffset = CharColumnXOffsetOfByteOffset( highlightStart ) + GetColumnOffset( kDataColumnChar );
			endOffset = CharColumnXOffsetOfByteOffset( highlightEnd ) + GetColumnOffset( kDataColumnChar );
			DrawRowHighlight( inContext, verticalOffset, startOffset, endOffset );
		}
		
		// Draw the offset column
		DrawCharBytes(
			inContext,
			offsetString,
			strlen( offsetString ),
			verticalOffset,
			GetColumnOffset( kDataColumnOffset ) + offsetStringOffset );
		
		// Draw the hex column
		DrawHexBytes( inContext, byteHead, bytesToDraw, verticalOffset, GetColumnOffset( kDataColumnHex ) );
		
		// Draw the char column
		DrawCharBytes( inContext, byteHead, bytesToDraw, verticalOffset, GetColumnOffset( kDataColumnChar ) );

		// Bump the offset to the next row
		byteOffset += bytesPerRow;
		
		// If we didn't have to create a buffer to hold the data, advance the data ptr, 
		// otherwise, get the next row of bytes
		if ( !createdBuffer )
			byteHead += bytesPerRow;
		else
			::CFDataGetBytes( fData, CFRangeMake( byteOffset, bytesToDraw ), byteHead );

		// advance the vertical offset to the next row
		verticalOffset += kRowHeight;
	}

	DrawCaret( inContext );

	// if we created the buffer, dispose of it properly so we don't leak
	if ( createdBuffer )
		free( byteHead );
		
Draw_CantGetRowBuffer:
Draw_NothingToDraw:

	// restore our context
	::CGContextRestoreGState( inContext );
}

//--------------------------------------------------------------------
//	HitTest
//--------------------------------------------------------------------
//
ControlPartCode
THexEditorScrollableView::HitTest( const HIPoint& where )
{
	ControlPartCode		partHit = kControlNoPart;
	TRect				bounds = Bounds();
	
	if ( ::CGRectContainsPoint( bounds, where ) )
		partHit = FindPart( where );
	
	return partHit;
}

//--------------------------------------------------------------------
//	Track
//--------------------------------------------------------------------
//	Track the control
//
OSStatus
THexEditorScrollableView::Track(
	TCarbonEvent&		inEvent,
	ControlPartCode*	outPartHit )
{
	OSStatus			err = eventNotHandledErr;
	HIPoint				startPt;
	HIPoint				trackingPt;
	ControlPartCode		startPart;
	MouseTrackingResult	trackingResult;
	long				trackingStartOffset;
	UInt32				modifiers;
	
	// get the starting mouse location and modifier keys
	verify_noerr( inEvent.GetParameter( kEventParamMouseLocation, &startPt ) );
	verify_noerr( inEvent.GetParameter( kEventParamKeyModifiers, &modifiers ) );
	
	// find what part was hit
	startPart = FindPart( startPt );
	
	// make sure it is in a valid column
	require_quiet( (startPart == kHexEditorHexColumnPart)
					|| (startPart == kHexEditorCharColumnPart), Track_InvalidTrackingClick );
	
	// we need to cache what column part the click was in to blink
	// the cursor in the right place
	fCaretColumn = startPart;
	
	trackingStartOffset = ByteOffsetOfViewPoint( startPt );
	
	if ( ( modifiers & shiftKey ) == 0 )
	{
		// the mouse down was here
		fSelectionStart = fSelectionEnd = trackingStartOffset;
	}
	else
	{
		fSelectionEnd = trackingStartOffset;
		trackingStartOffset = fSelectionStart;
	}
	
	// turn off any caret blinking
	BlinkCaret( false );
	
	// loop while tracking the mouse. Verify that the mouse is still down before getting
	// into this loop. It is quite possible that the mouse up has left the queue already
	// if the machine is under a heavy load.
	if ( ::StillDown() )
	{
		while ( true )
		{
			Point			mousePt;
			ControlPartCode	trackingPart;
			long			trackingSelection;
			
			verify_noerr( ::TrackMouseLocation( (GrafPtr)-1 /*global coords*/, &mousePt, &trackingResult ) );
			
			// convert it to this view's coordinate space
			ConvertGlobalQDPointToViewHIPoint( mousePt, &trackingPt );
			
			trackingPart = FindPart( trackingPt );
			
			// If the user is tracking the mouse in the same part that they started in
			// update visuals, otherwise, ignore
			if ( trackingPart == startPart )
			{
				trackingSelection = ByteOffsetOfViewPoint( trackingPt );
				
				// update the selection offsets to properly reflect "start" and "end"
				if ( trackingSelection < trackingStartOffset )
				{
					fSelectionStart = trackingSelection;
					fSelectionEnd = trackingStartOffset;
				}
				else
				{
					fSelectionStart = trackingStartOffset;
					fSelectionEnd = trackingSelection;
				}
				
				SelectionChanged( trackingSelection );
				InvalidateSelection();
			}
			
			// break out of the tracking loop if we get a mouse up
			if ( trackingResult == kMouseTrackingMouseUp )
				break;
		}
	}
	
	// if there is an empty selection, we want to blink the caret again
	if ( IsEmptySelection() )
		BlinkCaret( true );
	
	fHalfByteInsertion = false;
	
	*outPartHit = startPart;
	
Track_InvalidTrackingClick:
	return err;
}

//--------------------------------------------------------------------
//	ActiveStateChanged
//--------------------------------------------------------------------
//	Adjust whether the cursor is blinking on activation/deactivation
//
OSStatus
THexEditorScrollableView::ActiveStateChanged()
{	
	if ( IsActive() && IsEmptySelection() )
		BlinkCaret( true );
	else
		BlinkCaret( false );
	
	Invalidate();
	
	return noErr;
}

//--------------------------------------------------------------------
//	BoundsChanged
//--------------------------------------------------------------------
//
OSStatus
THexEditorScrollableView::BoundsChanged(
	UInt32				inOptions,
	const HIRect&		inOriginalBounds,
	const HIRect&		inCurrentBounds,
	RgnHandle			inInvalRgn )
{
	TRect				bounds = Bounds();
	float				workableWidth;
	float				hexSpacesPerByte;
	float				charSpacesPerByte;
	float				proportion;
	
#pragma unused( inOptions, inOriginalBounds, inCurrentBounds, inInvalRgn )

	//
	// recalculate the column widths
	//
	
	workableWidth = bounds.Width();
	
	// First, we know that the offset column is of a fixed width
	fDataColumnWidths[ kDataColumnOffset ] = kColumnOffsetWidth;
	workableWidth -= kColumnOffsetWidth;
	
	// Second, we know the available width must take into account the 2 separators
	// between the offset and hex columns and between the hex and char columns
	workableWidth -= ( 2 * kColumnSeparatorPad );
	
	// Next, we calculate the ratio required for the hex column to the char column
	// in terms of character spaces per byte

	// first figure the hex proportion
	hexSpacesPerByte = ( ( kBytesPerColumn * kCharactersPerByte ) + 1 ) / kCharactersPerByte;
	
	// now figure the char proportion (which is 1 character per 1 byte )
	charSpacesPerByte = 1;
	
	// now figure the proportion that we need to figure for the availableWidth
	proportion = hexSpacesPerByte + charSpacesPerByte;
	
	check( proportion != 0 );
	proportion = workableWidth / proportion;
	
	// Now we can determine the width of each column
	fDataColumnWidths[ kDataColumnHex ] = proportion * hexSpacesPerByte;
	fDataColumnWidths[ kDataColumnChar ] = proportion * charSpacesPerByte;
	
	// verify that the widths are about right, giving room for roundoff error.
	check( ::abs( bounds.Width() - ( fDataColumnWidths[ kDataColumnOffset ]
				+ fDataColumnWidths[ kDataColumnHex ]
				+ fDataColumnWidths[ kDataColumnChar ] ) ) < 1.0 );
	
	return noErr;
}

//--------------------------------------------------------------------
//	GetRegion
//--------------------------------------------------------------------
//
OSStatus
THexEditorScrollableView::GetRegion(
	ControlPartCode		inPart,
	RgnHandle 			outRgn )
{
	OSStatus			err = noErr;
	TRect				bounds = Bounds();
	TRect				regionBounds;
	Rect				qdBounds;

	check( outRgn != NULL );
	
	switch ( inPart )
	{
		case kHexEditorOffsetColumnPart:
			{
				regionBounds.Set(
						GetColumnOffset( kDataColumnOffset ),
						bounds.MinY(),
						fDataColumnWidths[ kDataColumnOffset ],
						bounds.Height() );
				qdBounds = regionBounds;
				
				RectRgn( outRgn, &qdBounds );
			}
			break;
			
		case kHexEditorOffsetSeparatorPart:
			{
				regionBounds.Set(
						GetColumnOffset( kDataColumnOffset ) + fDataColumnWidths[ kDataColumnOffset ],
						bounds.MinY(),
						kColumnSeparatorPad,
						bounds.Height() );
				qdBounds = regionBounds;
				
				RectRgn( outRgn, &qdBounds );
			}
			break;
			
		case kHexEditorHexColumnPart:
			{
				regionBounds.Set(
						GetColumnOffset( kDataColumnHex ),
						bounds.MinY(),
						fDataColumnWidths[ kDataColumnHex ],
						bounds.Height() );
				qdBounds = regionBounds;
				
				RectRgn( outRgn, &qdBounds );
			}
			break;
			
		case kHexEditorHexColumnSeparatorPart:
			{
				regionBounds.Set(
						GetColumnOffset( kDataColumnHex ) + fDataColumnWidths[ kDataColumnHex ],
						bounds.MinY(),
						kColumnSeparatorPad,
						bounds.Height() );
				qdBounds = regionBounds;
				
				RectRgn( outRgn, &qdBounds );
			}
			break;
			
		case kHexEditorCharColumnPart:
			{
				regionBounds.Set(
						GetColumnOffset( kDataColumnChar ),
						bounds.MinY(),
						fDataColumnWidths[ kDataColumnChar ],
						bounds.Height() );
				qdBounds = regionBounds;
				
				RectRgn( outRgn, &qdBounds );
			}
			break;
		
		default:
			err = TView::GetRegion( inPart, outRgn );
			break;
	}
	
	return err;
}

//--------------------------------------------------------------------
//	GetKind
//--------------------------------------------------------------------
//
ControlKind
THexEditorScrollableView::GetKind()
{	
	return kHexEditorScrollableViewKind;
}

//--------------------------------------------------------------------
//	GetBehaviors
//--------------------------------------------------------------------
//
UInt32
THexEditorScrollableView::GetBehaviors()
{
	return TView::GetBehaviors() | kControlGetsFocusOnClick;
}

//--------------------------------------------------------------------
//	SetFocusPart
//--------------------------------------------------------------------
//	We claim to accept focus, and we need this to support text input
//
OSStatus
THexEditorScrollableView::SetFocusPart(
	ControlPartCode		inDesiredFocus,
	RgnHandle			inInvalidRgn,
	Boolean				inFocusEverything,
	ControlPartCode*	outActualFocus )
{	

#pragma unused( inInvalidRgn, inFocusEverything )

	check( outActualFocus != NULL );
	
	if ( ( inDesiredFocus == kHexEditorHexColumnPart )
		|| ( inDesiredFocus == kHexEditorCharColumnPart ) )
	{
		fCaretColumn = inDesiredFocus;
		BlinkCaret( true );
		*outActualFocus = fCaretColumn;
	}
	else
	{
		fCaretColumn = kControlNoPart;
		BlinkCaret( false );
		*outActualFocus = kControlNoPart;
	}
	
	return noErr;
}

//--------------------------------------------------------------------
//	TextInput
//--------------------------------------------------------------------
//	Handle text input here
//
OSStatus
THexEditorScrollableView::TextInput( TCarbonEvent& inEvent )
{
	OSStatus	result = eventNotHandledErr;
	OSStatus	err;
	char		keyChar;
	EventRef	rawKeyboardEvent;
	
	require_quiet( fSelectionStart != kNoSelection, TextInput_NoSelection );
	
	// We just want raw characters since we're not doing any fancy 
	// unicode editing or such, so retrieve the raw key event from this event
	err = inEvent.GetParameter(
				kEventParamTextInputSendKeyboardEvent,
				typeEventRef,
				sizeof( EventRef ),
				&rawKeyboardEvent );
	require_noerr( err, TextInput_CantGetOriginalRawKeyEvent );
	check( rawKeyboardEvent != NULL );
	
	// then get the character code from the raw key event
	verify_noerr( TCarbonEvent( rawKeyboardEvent ).GetParameter(
						kEventParamKeyMacCharCodes,
						typeChar,
						sizeof( char ),
						&keyChar ) );
	
	// if the key is a special character for the control, handle it in the 
	// central place otherwise pass the key to the individual column that is
	// accepting the input
	if ( IsSpecialCharacter( keyChar ) )
	{
		err = HandleSpecialCharacter( keyChar );
	}
	else
	{
		switch ( fCaretColumn )
		{
			case kHexEditorHexColumnPart:
				err = HandleHexColumnInput( keyChar );
				break;
				
			case kHexEditorCharColumnPart:
				err = HandleCharColumnInput( keyChar );
				break;
			
			default:
				// do nothing
				break;
		}
	}

TextInput_CantGetOriginalRawKeyEvent:
TextInput_NoSelection:
	return result;
}

//--------------------------------------------------------------------
//	FindPart
//--------------------------------------------------------------------
//	Find the part code associated with the current point
//	expects where to be in view coordinates
//
ControlPartCode
THexEditorScrollableView::FindPart( const HIPoint& where )
{
	TRect		bounds = Bounds();
	TRect		partRect;
		
	//
	// find where the click is
	//
	
	// is it in the offset field?
	partRect.Set( bounds.MinX(), bounds.MinY(), fDataColumnWidths[ kDataColumnOffset ], bounds.Height() );
	
	if ( partRect.Contains( where ) )
		return kHexEditorOffsetColumnPart;
	
	// is it in the offset separator?
	partRect.MoveBy( partRect.Width(), 0 );
	partRect.SetWidth( kColumnSeparatorPad );
	if ( partRect.Contains( where ) )
		return kHexEditorOffsetSeparatorPart;
	
	// is it the hex field?
	partRect.MoveBy( partRect.Width(), 0 );
	partRect.SetWidth( fDataColumnWidths[ kDataColumnHex ] );
	if ( partRect.Contains( where ) )
		return kHexEditorHexColumnPart;
	
	// is it the hex separator?
	partRect.MoveBy( partRect.Width(), 0 );
	partRect.SetWidth( kColumnSeparatorPad );
	if ( partRect.Contains( where ) )
		return kHexEditorHexColumnSeparatorPart;
	
	// is it the character column
	partRect.MoveBy( partRect.Width(), 0 );
	partRect.SetWidth( fDataColumnWidths[ kDataColumnChar ] );
	if ( partRect.Contains( where ) )
		return kHexEditorCharColumnPart;
		
	return kControlNoPart;
}

//--------------------------------------------------------------------
//	Scroll
//--------------------------------------------------------------------
//	Scroll the view with the specified action. This just generates
//	a delta and passes that delta to the more generic scroll method
//
void
THexEditorScrollableView::Scroll( ScrollAction inAction )
{
	switch ( inAction )
	{
		case kScrollHome:
				Scroll( -fImageOrigin.y );
			break;
			
		case kScrollEnd:
				Scroll( ImageSize().height - fImageOrigin.y );
			break;
			
		case kScrollPageUp:
				Scroll( -ViewSize().height + LineSize().height );
			break;
			
		case kScrollPageDown:
				Scroll( ViewSize().height - LineSize().height );
			break;
	}
}

//--------------------------------------------------------------------
//	Scroll
//--------------------------------------------------------------------
//	Scroll the view by the specified vertical delta
//
void
THexEditorScrollableView::Scroll( float inVDelta )
{
	float			newOffset = fImageOrigin.y + inVDelta;
	float			maxOffset;
	
	check( parentView != NULL );
	
	// make sure we didn't over shoot the boundaries of the view
	newOffset = newOffset < 0 ? 0 : newOffset;
	
	maxOffset = ImageSize().height - ViewSize().height;
	
	fImageOrigin.y = newOffset > maxOffset ? maxOffset : newOffset;
	
	// Notify the parent view that our state has changed
	SendViewInfoChangedNotification();
	
	// force a redraw of my view and my parent
	Invalidate();
	InvalidateContainerView();	
}

//--------------------------------------------------------------------
//	AdjustSelection
//--------------------------------------------------------------------
//	Adjust the current selection with the given action
//
void
THexEditorScrollableView::AdjustSelection( AdjustSelectionAction inAction )
{
	long	newSelectionPoint = 0;
	long	dataSize = GetDataLength();
	long	tempPoint;
		
	switch ( inAction )
	{
		case kAdjustSelectionLeft:
			tempPoint = fSelectionStart - 1;
			newSelectionPoint = tempPoint < 0 ? 0 : tempPoint;
			break;
		
		case kAdjustSelectionRight:
			tempPoint = fSelectionEnd + 1;
			newSelectionPoint = tempPoint > dataSize ? dataSize : tempPoint;
			break;
			
		case kAdjustSelectionUp:
			tempPoint = fSelectionStart - BytesPerRow();
			newSelectionPoint = tempPoint < 0 ? 0 : tempPoint;
			break;
		
		case kAdjustSelectionDown:
			tempPoint = fSelectionEnd + BytesPerRow();
			newSelectionPoint = tempPoint > dataSize ? dataSize : tempPoint;
			break;
		
		default:
			debug_string( "Unknown adjustment action" );
			break;
	}
	
	fSelectionStart = fSelectionEnd = newSelectionPoint;

	SelectionChanged( newSelectionPoint );
	
	// Update the caret, so we can see that it moved immediately
	BlinkCaret( true );
}

//--------------------------------------------------------------------
//	SendViewInfoChangedNotification
//--------------------------------------------------------------------
//	Send a notification that our internal state has changed so the
//	parent view can get updated appropriately
//
void
THexEditorScrollableView::SendViewInfoChangedNotification()
{
	TCarbonEvent	event( kEventClassScrollable, kEventScrollableInfoChanged );
	
	// Tell the containing view that my info has just changed, this event
	// will get propogated to my parent
	::SendEventToEventTarget( event, GetEventTarget() );
}


#pragma mark --- METRIC CALCULATIONS ---

//--------------------------------------------------------------------
//	NumDrawableRows
//--------------------------------------------------------------------
//	Get the count of the number of rows that are drawable within 
//	our bounds which can be displayed (not the absolute value that
//	this view can display)
//
long
THexEditorScrollableView::NumDrawableRows()
{
	TRect	bounds = Bounds();	
	
	return (long) ceil( bounds.Height() / kRowHeight );
}

//--------------------------------------------------------------------
//	NumVisibleRows
//--------------------------------------------------------------------
//	Calculate the number of user visible rows that can be displayed. There
// 	is a fudge here because a partial row may be visible.
//
long
THexEditorScrollableView::NumVisibleRows()
{
	TRect	bounds = Bounds();
	
	return (long) ( bounds.Height() / kRowHeight );
}

//--------------------------------------------------------------------
//	NumHexColumns
//--------------------------------------------------------------------
//	Get the count of the number of visible hex columns based off of the
//	hex dimensions
//
long
THexEditorScrollableView::NumHexColumns()
{
	TRect	bounds = Bounds();
	
	return (long)(fDataColumnWidths[ kDataColumnHex ] / kHexColumnWidth);
}

//--------------------------------------------------------------------
//	GetColumnOffset
//--------------------------------------------------------------------
//	Calculate the offset of the given column type
//
float
THexEditorScrollableView::GetColumnOffset( HexEditorDataColumn inColumn )
{
	switch ( inColumn )
	{
		case kDataColumnOffset:
			return 0;
			break;
		
		case kDataColumnHex:
			return fDataColumnWidths[ kDataColumnOffset ] + kColumnSeparatorPad;
			break;
		
		case kDataColumnChar:
			return fDataColumnWidths[ kDataColumnOffset ] + fDataColumnWidths[ kDataColumnHex ] + ( 2 * kColumnSeparatorPad );
			break;
		
		default:
			debug_string( "Unknown DataColumn type" );
			return 0;
	}
}

//--------------------------------------------------------------------
//	VisibleVirtualBytes
//--------------------------------------------------------------------
//
long
THexEditorScrollableView::VisibleVirtualBytes()
{
	return NumDrawableRows() * BytesPerRow();
}

//--------------------------------------------------------------------
//	BytesPerRow
//--------------------------------------------------------------------
//
long
THexEditorScrollableView::BytesPerRow()
{
	return NumHexColumns() * kBytesPerColumn;
}

//--------------------------------------------------------------------
//	RowsToDraw
//--------------------------------------------------------------------
//	If the buffer size is greater than the visible bytes on the screen
//	then draw the entire visible rows, otherwise find the subset required
//
long
THexEditorScrollableView::RowsToDraw()
{
	long		topLeftRow;
	long		rows;
	long		visibleBytes;
	
	RowForYOffset( fImageOrigin.y, &topLeftRow, NULL );
	
	visibleBytes = GetDataLength() - ByteOffsetOfRow( topLeftRow );
	
	// If there are more bytes to draw than will fit in the view, 
	// draw the number of visible rows
	if ( visibleBytes >= VisibleVirtualBytes() )
		rows = NumDrawableRows();
	else
		rows = (long)ceil( (float)visibleBytes / (float)BytesPerRow() );
	
	return rows;
}

//--------------------------------------------------------------------
//	DrawableRowBytes
//--------------------------------------------------------------------
//	Calculates the number of bytes that can be drawn for the row
//	that begins at the given offset
//
long
THexEditorScrollableView::DrawableRowBytes( long inOffset )
{
	CFIndex		bufferSize;
	
	bufferSize = GetDataLength();
	
	// if the number of bytes left in the buffer is greater than the bytes
	// that can be drawn in the row, draw the entire row, otherwise find the 
	// number of bytes required
	if ( ( bufferSize - inOffset ) >= BytesPerRow() )
		return BytesPerRow();
	else
		return ( bufferSize - inOffset ) % BytesPerRow();
}

//--------------------------------------------------------------------
//	GetOffsetStringAndOffset
//--------------------------------------------------------------------
//	Get the offset string from the given offset, and determine the
//	offset required to draw it in a pretty location.
//
void
THexEditorScrollableView::GetOffsetStringAndOffset(
	long	inByteOffset,
	char*	outOffsetString,
	float*	outHorizontalOffset )
{
	int		chars;
	
	check( outOffsetString != NULL );
	check( outHorizontalOffset != NULL );
	
	// fill in the offset string
	chars = ::sprintf( outOffsetString, "%ld", inByteOffset );
	
	// calculate the offset within this column to make the text appear
	// right justified
	*outHorizontalOffset = kColumnOffsetWidth - ( chars * kCharacterWidth );
}

//--------------------------------------------------------------------
//	CalculateDrawOffsets
//--------------------------------------------------------------------
//	Calculate the byte offset position and the drawable offsets
//	to begin drawing at the image origin. Any parameter can be NULL
//
void
THexEditorScrollableView::CalculateDrawOffsets(
	long*	outByteOffset,
	float*	outVerticalOffset )
{
	long	row;
	
	// Make sure we're doing work for something
	check( outByteOffset || outVerticalOffset );

	RowForYOffset( fImageOrigin.y, &row, outVerticalOffset );
	
	if ( outByteOffset != NULL )
		*outByteOffset = ByteOffsetOfRow( row );
}

//--------------------------------------------------------------------
//	ByteOffsetOfViewPoint
//--------------------------------------------------------------------
//	Calculate the byte offset of the given point in view coordinates
//
long
THexEditorScrollableView::ByteOffsetOfViewPoint( const HIPoint& inPoint )
{
	HIPoint			point;
	long			row;
	long			byteOffset;
	ControlPartCode	part;
	
	// find what part this is
	part = FindPart( inPoint );
	
	// translate the point to virtual view coordinates (offset by origin)
	point = inPoint;
	point.x += fImageOrigin.x;
	point.y += fImageOrigin.y;
	
	// find the row that was clicked in
	RowForYOffset( point.y, &row, NULL );
	
	// find this byte offset
	byteOffset = ByteOffsetOfRow( row );
	
	switch ( part )
	{
		case kHexEditorOffsetColumnPart:
			// invalid, no offset available here
			break;
		
		case kHexEditorHexColumnPart:
			byteOffset += ByteOffsetOfHexColumnXOffset( inPoint.x - GetColumnOffset( kDataColumnHex ) );
			break;
		
		case kHexEditorCharColumnPart:
			byteOffset += ByteOffsetOfCharColumnXOffset( inPoint.x - GetColumnOffset( kDataColumnChar ) );
			break;
	}
	
	// the offset can't be greater than the size of the data
	return byteOffset > GetDataLength() ? GetDataLength() : byteOffset;
}


#pragma mark -- ROW CALCULATIONS --

//--------------------------------------------------------------------
//	ByteOffsetOfRow
//--------------------------------------------------------------------
//	Find the byte offset for the given row, i.e. what byte corresponds
//	to the beginning of this row
//
long
THexEditorScrollableView::ByteOffsetOfRow( long inRow )
{
	return inRow * BytesPerRow();
}

//--------------------------------------------------------------------
//	RowForByteOffset
//--------------------------------------------------------------------
//	Determine what row the given byte offset is in
//
long
THexEditorScrollableView::RowForByteOffset( long inByteOffset )
{
	check( inByteOffset < GetDataLength() );
	
	return inByteOffset / BytesPerRow();
}

//--------------------------------------------------------------------
//	RowForYOffset
//--------------------------------------------------------------------
//	Determine the row number for the offset, and what the offset is
//	to the bottom of the visible text within that row.
//
void
THexEditorScrollableView::RowForYOffset(
	float	inYOffset,
	long*	outRow,
	float*	outTextStartOffset )	// can be NULL
{
	check( outRow != NULL );

	// the vertical origin may be in the middle of a line. If this is the case,
	// we need to draw the row above this one as well
	*outRow = (long)floor( inYOffset / kRowHeight );
	
	// The origin may be within a row, if this is the case, find the offset
	// within the row so we can determine where the pen position should be.
	if ( outTextStartOffset != NULL )
		*outTextStartOffset = kRowHeight - ( inYOffset - ( *outRow * kRowHeight ) );
}

//--------------------------------------------------------------------
//	VerticalOffsetOfRow
//--------------------------------------------------------------------
//	Determine the appropriate vertical offset to the top of the given row
//
float
THexEditorScrollableView::VerticalOffsetOfRow( long inRow )
{
	return inRow * kRowHeight;
}

//--------------------------------------------------------------------
//	CalculateRowHighlightSelection
//--------------------------------------------------------------------
//	Calculates the byte offsets in the given row to which the highlight
//	should be applied if there is a selection in that row. This is the 
//	relative offset within this row, not the absolute byte offset.
//
void
THexEditorScrollableView::CalculateRowHighlightSelection(
	long	inRow,
	long*	outSelectionStart,
	long*	outSelectionEnd )
{
	long	rowByteOffset;
	long	rowSelectionStart;
	long	rowSelectionEnd;
	
	check( outSelectionStart != NULL );
	check( outSelectionEnd != NULL );
	
	// default to something reasonable
	*outSelectionStart = *outSelectionEnd = 0;
	
	// if there is no selection, nothing to do
	require_quiet( not IsEmptySelection(), CalculateRowHighlight_NoSelection );

	// figure out what rows the selections begin on
	rowSelectionStart	= RowForByteOffset( fSelectionStart );
	rowSelectionEnd 	= RowForByteOffset( fSelectionEnd );
	
	// determine the byte offset at the beginning of this row
	rowByteOffset = ByteOffsetOfRow( inRow );
	
	// skip operations if the entire selection is not in this row
	require_quiet( ( rowSelectionStart <= inRow )
					&& ( rowSelectionEnd >= inRow ), CalculateRowHighlight_SelectionBeyondRow );
					
	// if the selection start is before this row, the byte offset for this row is 0, 
	// which it is already initialized to be, so the only thing we need to check
	// is if it is within the current row
	if ( rowSelectionStart == inRow )
		*outSelectionStart = fSelectionStart - rowByteOffset;
	
	// now determine the end offset
	if ( rowSelectionEnd > inRow )
		*outSelectionEnd = BytesPerRow();
	else
		*outSelectionEnd = fSelectionEnd - rowByteOffset;

CalculateRowHighlight_SelectionBeyondRow:
CalculateRowHighlight_NoSelection:
	;	
}

#pragma mark -- COLUMN CALCULATIONS --

//--------------------------------------------------------------------
//	ByteOffsetOfHexColumnXOffset
//--------------------------------------------------------------------
//	Find the byte offset for the x offset in the hex column's frame
//
long
THexEditorScrollableView::ByteOffsetOfHexColumnXOffset( float inHexColumnXOffset )
{
	float	clusterColumn = inHexColumnXOffset / kHexColumnWidth;
	float	clusterOffset;
	short	clusterByteOffset = 0;
	long	byteOffset;
	
	// We know the cluster column that this offset is in, now we need to determine 
	// where the offset is within this cluster
	clusterOffset = clusterColumn - floor( clusterColumn );
	
	// Now that we know the offset within the cluster, we need to determine
	// if the point is closer to the first or second byte of the cluster
	// if it is within a reasonable distance of the center, then assume 
	// the user was clicking on the second byte
	if ( clusterOffset > 0.8 )
		clusterByteOffset = 2;
	else if ( clusterOffset > 0.4 )
		clusterByteOffset = 1;
	
	// the byte offset is the offset of the cluster that we're in plus any cluster offset
	byteOffset = (long)(floor( clusterColumn ) * kBytesPerColumn ) + clusterByteOffset;
	
	return byteOffset;
}

//--------------------------------------------------------------------
//	ByteOffsetOfCharColumnXOffset
//--------------------------------------------------------------------
//	Find the byte offset for the x offset in the char column's frame
//
long
THexEditorScrollableView::ByteOffsetOfCharColumnXOffset( float inCharColumnXOffset )
{
	float byteOffset = inCharColumnXOffset / kCharacterWidth;
	
	if ( byteOffset < BytesPerRow() )
		return (long) byteOffset;
	else
		return BytesPerRow();
}

//--------------------------------------------------------------------
//	HexColumnXOffsetOfByteOffset
//--------------------------------------------------------------------
//	Determine the x offset within the hex column of the byte offset
//	within the row (not the absolute byte offset)
//
float
THexEditorScrollableView::HexColumnXOffsetOfByteOffset( long inByteOffset )
{
	long	numHexColumns = inByteOffset / kBytesPerColumn;
	
	check_string( inByteOffset < BytesPerRow(), "Takes row byte offset, not absolute byte offset" );
	
	// If there are an even number of bytes, then the point offset is to
	// the beginning of the nth hex column. If there are an odd number of bytes
	// it is within the hex column cluster
	if ( ( inByteOffset % kBytesPerColumn ) == 0 )
		return numHexColumns * kHexColumnWidth;
	else
		return ( numHexColumns * kHexColumnWidth ) + ( kCharactersPerByte * kCharacterWidth );	
}

//--------------------------------------------------------------------
//	CharColumnXOffsetOfByteOffset
//--------------------------------------------------------------------
//	Calculate the x offset within the character column of the given 
//	byte offset within that row (not the absolute byte offset)
//
float
THexEditorScrollableView::CharColumnXOffsetOfByteOffset( long inByteOffset )
{
	return inByteOffset * kCharacterWidth;
}

//--------------------------------------------------------------------
//	HexColumnForXOffset
//--------------------------------------------------------------------
//	Determine the hex column number for the offset, and what the offset
// 	is to the beginning of the visible text within that column
//
void
THexEditorScrollableView::HexColumnForXOffset(
	float	inHexColumnXOffset,
	long*	outColumn,
	float*	outTextStartOffset )	// can be NULL
{
	float 	column = inHexColumnXOffset / kHexColumnWidth;
	
	check( outColumn != NULL );

	// The column number will be the column that contains the horizontal offset
	*outColumn = (long)floor( column );
	
	// Now return the pixel offset with respect to where the cluster should draw
	if ( outTextStartOffset != NULL )
		*outTextStartOffset = kHexColumnPad - ( ( ceil( column ) * kHexColumnWidth ) - inHexColumnXOffset);
}

#pragma mark --- SELECTION ---

//--------------------------------------------------------------------
//	IsEmptySelection
//--------------------------------------------------------------------
//
Boolean
THexEditorScrollableView::IsEmptySelection()
{
	return fSelectionStart == fSelectionEnd;
}

//--------------------------------------------------------------------
//	InvalidateSelection
//--------------------------------------------------------------------
//	***This could be optimized to only invalidate the selection region
//	rather than the whole thing
//
void
THexEditorScrollableView::InvalidateSelection()
{
	::HIViewSetNeedsDisplay( GetViewRef(), true );
}

//--------------------------------------------------------------------
//	IsCaretVisible
//--------------------------------------------------------------------
//
Boolean
THexEditorScrollableView::IsCaretVisible()
{
	Boolean	visible = false;
	long	topLeftByteOffset;
	long	visibleBytes;
	
	// the caret will only be blinking if there is an empty selection
	require_quiet( IsEmptySelection(), IsCaretVisible_NotEmptySelection );
	
	CalculateDrawOffsets( &topLeftByteOffset, NULL );
	visibleBytes = VisibleVirtualBytes();
	
	if ( ( fSelectionStart >= topLeftByteOffset)
		&& ( fSelectionStart < ( topLeftByteOffset + visibleBytes ) ) )
	{
		visible = true;
	}
	
IsCaretVisible_NotEmptySelection:
	return visible;	
}

//--------------------------------------------------------------------
//	BlinkCaret
//--------------------------------------------------------------------
//
void
THexEditorScrollableView::BlinkCaret( Boolean inIsBlinking )
{
	if ( inIsBlinking )
	{
		// fire up that timer if it's not already going; if it's going, have
		// it update immediately
		if ( fCaretTimer == NULL )
		{
			verify_noerr( ::InstallEventLoopTimer(
								::GetCurrentEventLoop(),
								0,							// no delay
								kCaretBlinkFrequency,		// interval
								CaretTimerProc,
								this,
								&fCaretTimer ) );
		}
		else
		{
			::SetEventLoopTimerNextFireTime( fCaretTimer, 0 );
		}
		
		// set it to hidden, so when it gets updated it will correctly draw shown. This doesn't
		// make much sense unless you look at the method below
		fCaretState = kCaretStateHidden;
	}
	else
	{
		if ( fCaretTimer != NULL )
		{
			::RemoveEventLoopTimer( fCaretTimer );
			fCaretTimer = NULL;
		}
		
		fCaretState = kCaretStateOff;
	}
}

//--------------------------------------------------------------------
//	UpdateCaret
//--------------------------------------------------------------------
//	Update the caret to the other state
//
void
THexEditorScrollableView::UpdateCaret()
{
	fCaretState = fCaretState == kCaretStateShown ? kCaretStateHidden : kCaretStateShown;
	
	if ( IsCaretVisible() )
	{
		TRect		caretRect 	= GetCaretRect();
		Rect		qdCaretRect = caretRect;
		RgnHandle	caretRgn 	= ::NewRgn();
		
		// get the caret region
		::RectRgn( caretRgn, &qdCaretRect );
		
		// now tell the HIView system to update this region
		::HIViewSetNeedsDisplay( GetViewRef(), true );
		::DisposeRgn( caretRgn );
	}
	
}

//--------------------------------------------------------------------
//	GetCaretRect
//--------------------------------------------------------------------
//	Calculate the caret rect for this view
//
TRect
THexEditorScrollableView::GetCaretRect()
{
	long	row;
	TRect	rect = CGRectZero;
	HIPoint	topLeft;
	
	require( IsEmptySelection(), GetCaretRect_NotEmptySelection );
	
	row = RowForByteOffset( fSelectionStart );

	// the top left of the visible rect is the coordinates of the caret adjusted
	// for the origin
	topLeft.y = VerticalOffsetOfRow( row ) + kRowPad - fImageOrigin.y;
	
	// the horizontal offset is different depending on which column it is in
	if ( fCaretColumn == kHexEditorHexColumnPart )
	{
		topLeft.x = HexColumnXOffsetOfByteOffset( fSelectionStart - ByteOffsetOfRow( row ) )
						- fImageOrigin.x 
						+ GetColumnOffset( kDataColumnHex );
	}
	else if ( fCaretColumn == kHexEditorCharColumnPart )
	{
		topLeft.x = CharColumnXOffsetOfByteOffset( fSelectionStart - ByteOffsetOfRow( row ) )
						- fImageOrigin.x
						+ GetColumnOffset( kDataColumnChar );
	}
	
	rect.SetOrigin( topLeft );
	rect.SetWidth( 1.0 );
	rect.SetHeight( kCharacterHeight + kRowPad );
	
GetCaretRect_NotEmptySelection:
	return rect;
}

//--------------------------------------------------------------------
//	IsValidHexInputChar
//--------------------------------------------------------------------
//
Boolean
THexEditorScrollableView::IsValidHexInputChar( char inKeyChar )
{
	return isxdigit( inKeyChar ) || ( inKeyChar == kSpaceCharCode );
}

//--------------------------------------------------------------------
//	ByteForChar
//--------------------------------------------------------------------
//
UInt8
THexEditorScrollableView::ByteForChar( char inKeyChar )
{
	UInt8	byte = 0;
	
	// translate to lower case (for ease)
	inKeyChar = tolower( inKeyChar );
	
	// This is derived from an ASCII table. We want to get the
	// byte that corresponds to the ASCII character
	if ( isdigit( inKeyChar ) )
		byte = inKeyChar - 48;
	else
		byte = inKeyChar - 87;
		
	return byte;
}

//--------------------------------------------------------------------
//	IsSpecialCharacter
//--------------------------------------------------------------------
//	Can the given character be handled by this view rather than
//	the columns?
//
Boolean
THexEditorScrollableView::IsSpecialCharacter( char inKeyChar )
{
	Boolean	special;
	
	switch ( inKeyChar )
	{
		case kHomeCharCode:
		case kEndCharCode:
		case kTabCharCode:
		case kPageUpCharCode:
		case kPageDownCharCode:
		case kLeftArrowCharCode:
		case kRightArrowCharCode:
		case kUpArrowCharCode:
		case kDownArrowCharCode:
		case kBackspaceCharCode:
		case kDeleteCharCode:
			special = true;
			break;
			
		default:
			special = false;
			break;
	}
	
	return special;
}

//--------------------------------------------------------------------
//	HandleSpecialCharacter
//--------------------------------------------------------------------
//	Is this a special character that should be handled by the view
//	itself, or should it be passed to the appropriate insertion column?
//
OSStatus
THexEditorScrollableView::HandleSpecialCharacter( char inKeyChar )
{
	OSStatus	err = noErr;
	
	switch ( inKeyChar )
	{
		case kHomeCharCode:
			Scroll( kScrollHome );
			break;
			
		case kEndCharCode:
			Scroll( kScrollEnd );
			break;
			
		case kTabCharCode:
			HandleTab();
			break;
			
		case kPageUpCharCode:
			Scroll( kScrollPageUp );
			break;
		
		case kPageDownCharCode:
			Scroll( kScrollPageDown );
			break;
			
		case kLeftArrowCharCode:
			AdjustSelection( kAdjustSelectionLeft );
			break;
			
		case kRightArrowCharCode:
			AdjustSelection( kAdjustSelectionRight );
			break;
			
		case kUpArrowCharCode:
			AdjustSelection( kAdjustSelectionUp );
			break;
			
		case kDownArrowCharCode:
			AdjustSelection( kAdjustSelectionDown );
			break;
			
		case kBackspaceCharCode:
			HandleDelete();
			break;
		
		default:
			err = eventNotHandledErr;
	}
	
	return err;
}

//--------------------------------------------------------------------
//	HandleHexColumnInput
//--------------------------------------------------------------------
//	This will handle input of hexadecimal numbers in the hex column.
//	Since a byte requires 2 characters for input, this keeps track of
//	whether the input is at the beginning of the byte or at the end, and
//	attempts to do what the user would expect.
//
OSStatus
THexEditorScrollableView::HandleHexColumnInput( char inKeyChar )
{
	OSStatus	err = eventNotHandledErr;
	
	if ( IsValidHexInputChar( inKeyChar ) )
	{
		HISize	initialSize = ImageSize();
		UInt8	byte;
		
		check( fData != NULL );
		
		// the space bar will get out of a half byte insertion mode
		if ( inKeyChar == kSpaceCharCode )
		{
			fHalfByteInsertion = false;	
		}
		else if ( not fHalfByteInsertion )
		{
			byte = ByteForChar( inKeyChar );
			
			// Insert this new byte into the data
			::CFDataReplaceBytes(
					fData,
					::CFRangeMake( fSelectionStart, fSelectionEnd - fSelectionStart ),
					&byte,
					sizeof( byte ) );
			
			// Now move the insertion point over
			fSelectionEnd = ++fSelectionStart;
			
			// this could potentially be only a half byte insertion
			fHalfByteInsertion = true;
		}
		else
		{
			check( IsEmptySelection() );	// can't have half insertions if there is a selection
			check( fSelectionStart > 0 );
			
			::CFDataGetBytes( fData, ::CFRangeMake( fSelectionStart - 1, sizeof( UInt8 ) ), &byte );
			
			check( (byte & 0xf0) == 0 );
			
			byte = ( byte << 4 ) | ByteForChar( inKeyChar );

			// Insert this new byte into the data
			::CFDataReplaceBytes(
				fData,
				CFRangeMake( fSelectionStart - 1, 1 ),
				&byte,
				sizeof( byte ) );
			
			// this is now a full byte that was inserted
			fHalfByteInsertion = false;
		}
	
		// Flag that the selection has changed so changes get updated
		SelectionChanged( fSelectionStart );
		
		// Update the view if there were any visible changes
		if ( initialSize.height != ImageSize().height )
			SendViewInfoChangedNotification();
			
		err = noErr;
	}
	else
	{
		SysBeep(1);
	}
	
	return err;
}

//--------------------------------------------------------------------
//	HandleCharColumnInput
//--------------------------------------------------------------------
//
OSStatus
THexEditorScrollableView::HandleCharColumnInput( char inKeyChar )
{
	UInt8		byte = inKeyChar;
	HISize		initialSize = ImageSize();
	
	check( fData != NULL );

	// Insert this new byte into the data
	::CFDataReplaceBytes(
		fData,
		CFRangeMake( fSelectionStart, fSelectionEnd - fSelectionStart ),
		&byte,
		sizeof( byte ) );
	
	// Now move the insertion point over
	fSelectionEnd = ++fSelectionStart;
	
	if ( initialSize.height != ImageSize().height )
		SendViewInfoChangedNotification();
	
	// And invalidate the selection so it redraws
	SelectionChanged( fSelectionStart );
	
	return noErr;
}

//--------------------------------------------------------------------
//	HandleDelete
//--------------------------------------------------------------------
//	Handle a deletion
//
void
THexEditorScrollableView::HandleDelete()
{
	long	selectionStart;
	
	check( fData != NULL );
	
	selectionStart = IsEmptySelection() ? fSelectionStart - 1 : fSelectionStart;
	
	// Only delete if there is something prior to this selection to delete
	if ( selectionStart > 0 )
	{
		HISize		initialSize = ImageSize();
		
		::CFDataDeleteBytes( fData, CFRangeMake( selectionStart, fSelectionEnd - selectionStart ) );
		fSelectionEnd = fSelectionStart = selectionStart;
		
		// Tell our view that info has changed if we've changed the image size
		if ( initialSize.height != ImageSize().height )
			SendViewInfoChangedNotification();
		
		SelectionChanged( fSelectionStart );
		
		// the selection is wiped out, therefore so is the insertion state
		fHalfByteInsertion = false;
		
		// since the selection is now empty start the caret blinking again
		BlinkCaret( true );
	}
}

//--------------------------------------------------------------------
//	HandleTab
//--------------------------------------------------------------------
//	Handle a tab character to go to the next focusable part
//
void
THexEditorScrollableView::HandleTab()
{
	::AdvanceKeyboardFocus( GetOwner() );
}

//--------------------------------------------------------------------
//	SelectionChanged
//--------------------------------------------------------------------
//	Check whether the selection is visible in the current view and update
//	the view if it is not.
//
void
THexEditorScrollableView::SelectionChanged( long inChangedSelection )
{
	long	topLeft;
	long	row;
	
	RowForYOffset( fImageOrigin.y, &topLeft, NULL );
	row = RowForByteOffset( inChangedSelection );
	
	if ( row < topLeft )
	{
		Scroll ( -( topLeft - row ) * kRowHeight );
	}
	else if ( row > ( topLeft + NumVisibleRows() ) )
	{
		Scroll( ( row - topLeft ) * kRowHeight );
	}
	
	Invalidate();
	InvalidateContainerView();
}

//--------------------------------------------------------------------
//	InvalidateContainerView
//--------------------------------------------------------------------
//
void
THexEditorScrollableView::InvalidateContainerView()
{
	TCarbonEvent	event( kEventClassScrollableHexEditorView, kEventHexEditorSelectionChanged );
	
	::SendEventToEventTarget( event, GetEventTarget() );
}

#pragma mark --- DRAWING ---

//--------------------------------------------------------------------
//	PrepContext
//--------------------------------------------------------------------
//	Prepare the context for our drawing operations
//
void
THexEditorScrollableView::PrepContext( CGContextRef inContext )
{
	CGAffineTransform		transform = CGAffineTransformIdentity;
	
	// adjust the transform so the text doesn't draw upside down
	transform = CGAffineTransformScale( transform, 1, -1 );

	// select the appropriate font, drawing mode and aliasing features
	::CGContextSelectFont( inContext, kFontName, kFontSize, kCGEncodingMacRoman );
	::CGContextSetTextDrawingMode( inContext, kCGTextFill );
	::CGContextSetShouldAntialias( inContext, false );
	::CGContextSetTextMatrix( inContext, transform );
	
	ResetContextFillColor( inContext );
}

//--------------------------------------------------------------------
//	DrawBackground
//--------------------------------------------------------------------
//
void
THexEditorScrollableView::DrawBackground( CGContextRef inContext )
{
	TRect	bounds = Bounds();
	
	::CGContextSetRGBFillColor(
			inContext,
			1.0,
			1.0,
			1.0,
			1.0 );
	
	::CGContextFillRect( inContext, bounds );
	
	ResetContextFillColor( inContext );
}

//--------------------------------------------------------------------
//	DrawColumnSeparators
//--------------------------------------------------------------------
//
void
THexEditorScrollableView::DrawColumnSeparators( CGContextRef inContext )
{
	TRect	bounds = Bounds();
	float	horizontalOffset;
	
	check( inContext != NULL );
	
	horizontalOffset = fDataColumnWidths[ kDataColumnOffset ] + ( kColumnSeparatorPad / 2 );
	
	::CGContextMoveToPoint( inContext, horizontalOffset, 0 );
	::CGContextAddLineToPoint( inContext, horizontalOffset, bounds.Height() );
	
	horizontalOffset += fDataColumnWidths[ kDataColumnHex ] + kColumnSeparatorPad - 0.5;

	::CGContextMoveToPoint( inContext, horizontalOffset, 0 );
	::CGContextAddLineToPoint( inContext, horizontalOffset, bounds.Height() );
	::CGContextStrokePath( inContext );
}

//--------------------------------------------------------------------
//	DrawRowHighlight
//--------------------------------------------------------------------
//	Draw the highlight from the first x offset to the second within
//	this row. Also vary the alpha depending on this control's active
//	state for visual clarity.
//
void
THexEditorScrollableView::DrawRowHighlight(
	CGContextRef		inContext,
	float				inVOffset,
	float				inSelectionStartOffset,
	float				inSelectionEndOffset )
{
	RGBColor			highlight;
	TRect				highlightRect(
							inSelectionStartOffset,
							inVOffset - kRowHeight + kRowPad,
							inSelectionEndOffset - inSelectionStartOffset,
							kRowHeight );
	float				alpha = IsActive() ? 1.0 : 0.60;
	
	// set up the appropriate highlight color in the context
	::LMGetHiliteRGB( &highlight );
	::CGContextSetRGBFillColor(
			inContext,
			(float)highlight.red / 65535.0,
			(float)highlight.green / 65535.0,
			(float)highlight.blue / 65535.0,
			alpha );
	::CGContextFillRect( inContext, highlightRect );
	
	ResetContextFillColor( inContext );
}

//--------------------------------------------------------------------
//	DrawHexBytes
//--------------------------------------------------------------------
//	Draw the given bytes as hex
//
void
THexEditorScrollableView::DrawHexBytes(
	CGContextRef		inContext,
	void*				inBuffer,
	long				inBufferSize,
	float				inVertOffset,
	float				inHorizontalOffset )
{
	char				hexBuffer[ 10 ]; //hexBuffer[ (kBytesPerColumn * kCharactersPerByte) + 1 ];
	float				horizPenLoc;
	size_t				bytesToDraw;
	char*				byteHead;
	
	// prep the horizontal pen location
	horizPenLoc = inHorizontalOffset;
	
	// point to the head of the bytes to draw
	byteHead = (char*)inBuffer;
	
	// now draw the bytes in clumps of 2
	for ( long index = 0; index < inBufferSize; index += kBytesPerColumn )
	{
		// Recall that sprintf requires a conversion code with optional parameters
		// of the form: %fnc
		// where 
		//		* 'f' is an optional flag. In this case we're using the flag 
		//			0 which means to pad a number on the left with zeros
		//		* 'n' is an optional positive number which is the minimum 
		//			width of the datum. In this case, we want a minimum of
		//			2 characters to be displayed
		//		* 'c' is the code which specifies how the output is converted,
		//			in this case we want the characters to be hexidecimal
		//
		//	These parameters were chosen such that there is padding of a minimum width
		//	such that "0" will be printed as "00".
		//
		//	We are also masking the byte with 0xFF so that the lowest byte is printed.
		//	This is needed to print the low order bits. For example, (int)'-1' will be interpreted as 
		//	0xFFFFFFFF instead of the desired 0xFF.

		// If there are 2 bytes to draw cluster them together
		if ( ( inBufferSize - index ) >= kBytesPerColumn )
		{
			::sprintf( hexBuffer, "%02X%02X", byteHead[0] & 0xFF, byteHead[1] & 0xFF );	
			bytesToDraw = kBytesPerColumn * kCharactersPerByte;
		}
		else	// there is only 1 byte to draw
		{
			::sprintf( hexBuffer, "%02X", byteHead[0] & 0xFF );
			bytesToDraw = 1 * kCharactersPerByte;
		}
		
		// Now draw the bytes as text
		::CGContextShowTextAtPoint( inContext, horizPenLoc, inVertOffset, hexBuffer, bytesToDraw );
		
		// update the pen location and the byteHead ptr
		horizPenLoc	+= kHexColumnWidth;
		byteHead	+= kBytesPerColumn;
	}
}

//--------------------------------------------------------------------
//	DrawCharBytes
//--------------------------------------------------------------------
//	Draw the given bytes as chars
//
void
THexEditorScrollableView::DrawCharBytes(
	CGContextRef	inContext,
	void*			inBuffer,
	long			inBufferSize,
	float			inVertOffset,
	float			inHorizontalOffset )
{
	UInt8*			buffer = (UInt8*)inBuffer;
	char*			characterBuffer;
	
	characterBuffer = new char[ inBufferSize ];
	
	// Fill a buffer with the printable characters that represent the given bytes
	for ( long index = 0; index < inBufferSize; index++ )
		characterBuffer[ index ] = toascii( buffer[ index ] );
		
	::CGContextShowTextAtPoint( inContext, inHorizontalOffset, inVertOffset, characterBuffer, inBufferSize );
	
	delete [] characterBuffer;
}

//--------------------------------------------------------------------
//	DrawCaret
//--------------------------------------------------------------------
//	Draw the caret in the appropriate highlight color
//
void
THexEditorScrollableView::DrawCaret( CGContextRef inContext )
{
	TRect	caretRect = GetCaretRect();
	
	require_quiet( IsCaretVisible(), DrawCaret_CaretNotVisible );
	
	if ( fCaretState == kCaretStateShown )
		::CGContextSetRGBFillColor( inContext, 0.0, 0.0, 0.0, 1.0 );
	else
		::CGContextSetRGBFillColor( inContext, 1.0, 1.0, 1.0, 1.0 );
		
	::CGContextFillRect( inContext, caretRect );

	ResetContextFillColor( inContext );
	
DrawCaret_CaretNotVisible:
	;
}

//--------------------------------------------------------------------
//	ResetContextFillColor
//--------------------------------------------------------------------
//
void
THexEditorScrollableView::ResetContextFillColor( CGContextRef inContext )
{
	check( inContext != NULL );
	
	::CGContextSetRGBFillColor( inContext, 0.0, 0.0, 0.0, 1.0 );
}

//--------------------------------------------------------------------
//	ResetData
//--------------------------------------------------------------------
//
void
THexEditorScrollableView::ResetData()
{
	fSelectionStart 	= fSelectionEnd = kNoSelection;
	fCaretState 		= kCaretStateOff;
	fCaretColumn		= kControlNoPart;
	fHalfByteInsertion	= false;
	
	BlinkCaret( false );
}

#pragma mark --- UTILITIES ---

//--------------------------------------------------------------------
//	ConvertGlobalQDPointToViewHIPoint
//--------------------------------------------------------------------
//
void
THexEditorScrollableView::ConvertGlobalQDPointToViewHIPoint(
	const Point& 	inGlobalPoint,
	HIPoint*		outViewPoint )
{
	Point			localPoint;
	HIViewRef		contentView;
	
	check( GetOwner() != NULL );
	check( outViewPoint != NULL );
	
	localPoint = inGlobalPoint;
	::QDGlobalToLocalPoint( ::GetWindowPort( GetOwner() ), &localPoint );
	
	// convert the QD point to an HIPoint
	outViewPoint->x = localPoint.h;
	outViewPoint->y = localPoint.v;
	
	// get the content view (which is what the local point is relative to)
	verify_noerr( ::HIViewFindByID( ::HIViewGetRoot( GetOwner() ), kHIViewWindowContentID, &contentView ) );
	
	// convert to view coordinates
	::HIViewConvertPoint( outViewPoint, contentView, GetViewRef() );
}
												
#pragma mark --- CLASS METHODS ---

//--------------------------------------------------------------------
//	Construct
//--------------------------------------------------------------------
//		
OSStatus
THexEditorScrollableView::Construct(
	ControlRef		inControl,
	TView**			outView )
{
	*outView = new THexEditorScrollableView( inControl );
	
	return noErr;
}

//--------------------------------------------------------------------
//	EventHandler
//--------------------------------------------------------------------
//
pascal OSStatus
THexEditorScrollableView::EventHandler(
	EventHandlerCallRef	inCallRef,
	EventRef			inEvent,
	void*				inUserData )
{
	THexEditorScrollableView*	view = (THexEditorScrollableView*)inUserData;
	OSStatus					err = eventNotHandledErr;
	TCarbonEvent				event( inEvent );
	
#pragma unused( inCallRef )

	switch ( event.GetClass() )
	{
		case kEventClassScrollable:
			switch( event.GetKind() )
			{
				case kEventScrollableGetInfo:
					{
						event.SetParameter( kEventParamImageSize, view->ImageSize() );
						event.SetParameter( kEventParamViewSize, view->ViewSize() );
						event.SetParameter( kEventParamOrigin, view->ImageOrigin() );
						event.SetParameter( kEventParamLineSize, view->LineSize() );
						
						err = noErr;
					}
					break;
				
				case kEventScrollableScrollTo:
					{
						HIPoint		location;
						
						event.GetParameter( kEventParamOrigin, &location );

						view->ScrollTo( location );
						err = noErr;
					}
					break;
		}
	}
	
	return err;
}

//--------------------------------------------------------------------
//	CaretTimerProc
//--------------------------------------------------------------------
//
pascal void
THexEditorScrollableView::CaretTimerProc(
	EventLoopTimerRef			inTimer,
	void*						inUserData )
{
	THexEditorScrollableView*	view = (THexEditorScrollableView*)inUserData;
	
#pragma unused( inTimer )

	view->UpdateCaret();
}
