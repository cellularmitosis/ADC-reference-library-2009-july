/*
	File: 		THexEditorScrollableView.h
	
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

#pragma once

#include <Carbon/Carbon.h>
#include "TView.h"

// View defined part codes
enum
{
	kHexEditorOffsetColumnPart			= 100,
	kHexEditorOffsetSeparatorPart,
	kHexEditorHexColumnPart,
	kHexEditorHexColumnSeparatorPart,
	kHexEditorCharColumnPart,	
};

// register for this event if you want notification that the selection has changed
enum
{
	kEventClassScrollableHexEditorView	= 'ScHx',
	
	kEventHexEditorSelectionChanged		= 1000
};

//--------------------------------------------------------------------

class THexEditorScrollableView : public TView
{
	public:
		static OSStatus						Create(
												CFDataRef		inData,
												HIViewRef*		outControl );
		static void							RegisterClass(); // this is a one-shot

		static OSStatus						GetDataSize(
												HIViewRef	inView,
												long*		outSize );
		static OSStatus						GetDataSelection(
												HIViewRef	inView,
												long*		outSelectionStart,
												long*		outSelectionEnd );
		static OSStatus						SetCFData(
												HIViewRef	inView,
												CFDataRef	inData );
		static OSStatus						GetCFData(
												HIViewRef	inView,
												CFDataRef*	outData );
		
		virtual void						ScrollTo( const HIPoint& inWhere );
		virtual HISize						ImageSize();
		virtual HIPoint						ImageOrigin();
		virtual HISize						ViewSize();
		virtual HISize						LineSize();
		
		CFDataRef							GetCFData();
		CFDataRef							CopyCFData();
		void								SetCFData( CFDataRef inData );
		long								GetDataLength();
		void								GetSelection(
												long*	outSelectionStart,
												long*	outSelectionEnd );
		
														
	protected:	
		virtual OSStatus					Initialize( TCarbonEvent& inEvent );

		// Handlers
		virtual void						Draw( RgnHandle limitRgn, CGContextRef inContext );
		virtual ControlPartCode				HitTest( const HIPoint& where );
		virtual	OSStatus					Track(
												TCarbonEvent&		inEvent,
												ControlPartCode*	outPartHit );
		virtual OSStatus					ActiveStateChanged();
		virtual OSStatus					BoundsChanged(
												UInt32				inOptions,
												const HIRect&		inOriginalBounds,
												const HIRect&		inCurrentBounds,
												RgnHandle			inInvalRgn );
		virtual OSStatus					GetRegion(
												ControlPartCode		inPart,
												RgnHandle			outRgn );
		virtual ControlKind					GetKind();
		virtual UInt32						GetBehaviors();
		virtual OSStatus					SetFocusPart(
												ControlPartCode		inDesiredFocus,
												RgnHandle			inInvalidRgn,
												Boolean				inFocusEverything,
												ControlPartCode*	outActualFocus );
		virtual OSStatus					TextInput( TCarbonEvent& inEvent );

			THexEditorScrollableView( ControlRef inControl );
		virtual ~THexEditorScrollableView();
		
	private:
	
		enum HexEditorDataColumn
		{
			kDataColumnNone				= -1,
			kDataColumnOffset			= 0,
			kDataColumnHex,
			kDataColumnChar,
			
			kDataColumnCount
		};
		
		enum CaretState
		{
			kCaretStateOff,				// the caret is not blinking
			kCaretStateShown,			// the caret is blinking and is visible
			kCaretStateHidden			// the caret is blinking and is not visible
		};
		
		enum ScrollAction
		{
			kScrollHome,
			kScrollEnd,
			kScrollPageUp,
			kScrollPageDown
		};
		
		enum AdjustSelectionAction
		{
			kAdjustSelectionLeft,
			kAdjustSelectionRight,
			kAdjustSelectionUp,
			kAdjustSelectionDown
		};
		
		ControlPartCode						FindPart( const HIPoint& where );
		void								Scroll( ScrollAction inAction );
		void								Scroll( float inVScroll );
		void								AdjustSelection( AdjustSelectionAction inAction );
		void								SendViewInfoChangedNotification();
		
		//
		// metric calculations
		//
		
		long								NumDrawableRows();
		long								NumVisibleRows();
		long								NumHexColumns();
		float								GetColumnOffset( HexEditorDataColumn inColumn );
		long								VisibleVirtualBytes();
		long								BytesPerRow();
		long								RowsToDraw();
		long								DrawableRowBytes( long inOffset );
		void								GetOffsetStringAndOffset(
												long	inByteOffset,
												char*	outOffsetString,
												float*	outHorizontalOffset );
		void								CalculateDrawOffsets(
												long*	outByteOffset,			// can be NULL
												float*	outVerticalOffset );	// can be NULL
		long								ByteOffsetOfViewPoint( const HIPoint& inViewPoint );
		
		// row related calculations
		
		long								ByteOffsetOfRow( long inRow );
		long								RowForByteOffset( long inByteOffset );
		void								RowForYOffset(
												float 	inYOffset,
												long*	outRow,
												float*	outTextStartOffset );	// can be NULL
		float								VerticalOffsetOfRow( long inRow );
		void								CalculateRowHighlightSelection(
												long	inRow,
												long*	outSelectionStart,
												long*	outSelectionEnd );
		
		// column related calculations
		
		long								ByteOffsetOfHexColumnXOffset( float inHexColumnXOffset );
		long								ByteOffsetOfCharColumnXOffset( float inCharColumnXOffset );
		float								HexColumnXOffsetOfByteOffset( long inByteOffset );
		float								CharColumnXOffsetOfByteOffset( long inByteOffset );
		
		void								HexColumnForXOffset(
												float	inHexColumnXOffset,
												long*	outColumn,
												float*	outTextStartOffset );	// can be NULL
		
		// selection
		
		Boolean								IsEmptySelection();
		void								InvalidateSelection();
		Boolean								IsCaretVisible();
		void								BlinkCaret( Boolean inBlinking );
		void								UpdateCaret();
		TRect								GetCaretRect();
		
		Boolean								IsValidHexInputChar( char inKeyChar );
		UInt8								ByteForChar( char inKeyChar );
		Boolean								IsSpecialCharacter( char inKeyChar );
		OSStatus							HandleSpecialCharacter( char inKeyChar );
		OSStatus							HandleHexColumnInput( char inKeyChar );
		OSStatus							HandleCharColumnInput( char inKeyChar );
		void								HandleDelete();
		void								HandleTab();
		void								SelectionChanged( long inChangedSelection ); // follow this new selection point
		void								InvalidateContainerView();
		
		//	drawing

		void								PrepContext( CGContextRef inContext );
		
		void								DrawBackground( CGContextRef inContext );
		void								DrawColumnSeparators( CGContextRef inContext );
		void								DrawRowHighlight(
												CGContextRef	inContext,
												float			inVOffset,
												float			inSelectionStartPosition,
												float			inSelectionEndPosition );
		void								DrawHexBytes(
												CGContextRef	inContext,
												void*			inBuffer,
												long			inBufferSize,
												float			inVOffset,
												float			inHOffset );
		void								DrawCharBytes(
												CGContextRef	inContext,
												void*			inBuffer,
												long			inBufferSize,
												float			inVOffset,
												float			inHOffset );
		void								DrawCaret( CGContextRef inContext );
		void								ResetContextFillColor( CGContextRef inContext );
		
		// utilities
		void								ConvertGlobalQDPointToViewHIPoint(
												const Point& 	inWinPoint,
												HIPoint*		outViewPoint );
		void								ResetData();
												
		// class methods
	
		static OSStatus						Construct( ControlRef inControl, TView** outView );		
		static pascal OSStatus				EventHandler( EventHandlerCallRef inCallRef, EventRef inEvent, void* inUserData );
		static pascal void					CaretTimerProc( EventLoopTimerRef inTimer, void* inUserData );
		
		// data
		
		CFMutableDataRef					fData;
		long								fSelectionStart;
		long								fSelectionEnd;
		CaretState							fCaretState;
		ControlPartCode						fCaretColumn;	// which column is the caret in?
		Boolean								fHalfByteInsertion;
		float								fDataColumnWidths[ kDataColumnCount ];
		
		EventLoopTimerRef					fCaretTimer;
		EventHandlerRef						fHandler;
		HIPoint								fImageOrigin;
};