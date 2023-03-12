/*
	File:		Window.c
	
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
// This code will run on Mac OS X 10.2 (or later) ONLY!!!
#include <Carbon/Carbon.h>

#include "Window.h"
#include "MenuHandler.h"
#include "HIElements.h"
#include "DirectAccessCallbacks.h"

#define BUGWORKAROUNDS 1

// ------------------------------------------------------------------------------
// Constants
// ------------------------------------------------------------------------------

enum {
	kDefaultUnicodeBufferSize = 20,
	kGlyphBurstLines = 20
};

// ------------------------------------------------------------------------------
// Data Structures
// ------------------------------------------------------------------------------

typedef struct WindowListItem {

	WindowRef				windowRef;
	struct WindowListItem	*nextItem;
	
} WindowListItem;

// ------------------------------------------------------------------------------
// Global Variables
// ------------------------------------------------------------------------------

IBNibRef gMainNibRef = NULL;

static WindowListItem *gWindowList = NULL;

// ------------------------------------------------------------------------------
// Function Prototypes
// ------------------------------------------------------------------------------

static OSStatus InstallWindowEventHandlers( WindowRef windowRef );

static OSStatus HandleKeyEvent( EventHandlerCallRef inHandlerCallRef,
	EventRef inEvent, void *inUserData );
	
static OSStatus HandleWindowEvent( EventHandlerCallRef inHandlerCallRef,
	EventRef inEvent, void *inUserData );
	
static OSStatus AddNewTextLayoutToContext( DrawContextStruct *context );
	
static OSStatus DrawWindow( WindowRef windowRef );

static void AddWindowToList( WindowRef windowRef );

static OSStatus DrawSingleLine( DrawContextStruct *context, Rect windowBounds );

static OSStatus DrawGlyphBurst( DrawContextStruct *context, Rect windowBounds );

static OSStatus ResizeDemoWindow( EventRef inEvent, WindowRef windowRef );

static void CloseDemoWindow( WindowRef windowRef );

// ------------------------------------------------------------------------------
// NewDemoWindow														[EXPORT]
// ------------------------------------------------------------------------------

extern
OSStatus NewDemoWindow( void )
{
	OSStatus	err;
	WindowRef 	windowRef;

	// create the window from the interface builder NIB file
	err = CreateWindowFromNib(gMainNibRef, CFSTR("MainWindow"), &windowRef);
    require_noerr( err, NewDemoWindow_err );
	
	// install the standard event handlers for the window
	err = InstallWindowEventHandlers( windowRef );
	require_noerr( err, NewDemoWindowEvent_err );
	
	// add the window to the window list
	AddWindowToList( windowRef );
    
    // The window was created hidden so show it
    ShowWindow( windowRef );
	
NewDemoWindow_err:	

	return err;

NewDemoWindowEvent_err:

	// dispose the window on error
	DisposeWindow( windowRef );

	return err;
	
}

// ------------------------------------------------------------------------------
// RedrawWindows														[EXPORT]
// ------------------------------------------------------------------------------

extern
OSStatus RedrawWindows( void )
{
	OSStatus			err = noErr;
	WindowListItem 		*currentItem = gWindowList;
	
	// search through the window list
	while( currentItem != NULL )
	{
		// draw the window
		err = DrawWindow( currentItem->windowRef );
		require_noerr( err, RedrawWindows_err );
		
		// grab the next context
		currentItem = currentItem->nextItem;
		
	}

RedrawWindows_err:

	return err;

}

// ------------------------------------------------------------------------------
// CloseAllWindows														[EXPORT]
// ------------------------------------------------------------------------------

extern
OSStatus CloseAllWindows( void )
{
	WindowListItem	*currentItem = gWindowList;

	// run through the window list
	while( currentItem != NULL )
	{
		// close the window
		CloseDemoWindow( currentItem->windowRef );
		
		// move on to the next item
		currentItem = currentItem->nextItem;
	}

	return noErr;
}

// ------------------------------------------------------------------------------
// InvalidateAndRedrawWindows											[EXPORT]
// ------------------------------------------------------------------------------

extern
OSStatus InvalidateAndRedrawWindows( void )
{
	OSStatus			err = noErr;
	WindowListItem 		*currentItem = gWindowList;
	DrawContextStruct	*windowContext;
	
	// search through the window list
	while( currentItem != NULL )
	{
		// get the draw context struct from the window's refCon
		windowContext = (DrawContextStruct *) GetWRefCon( 
			currentItem->windowRef );
		
		// make sure that we have a context
		if ( windowContext != NULL )
		{	
			// if we have a text layout, then make sure that we get rid of it.
			// This really is invalidation!
			if ( windowContext->layoutObject != NULL )
			{
				err = ATSUDisposeTextLayout( windowContext->layoutObject );
				require_noerr( err, InvalidateAndRedrawWindows_err );
				
				windowContext->layoutObject = NULL;
			}
		
			// nifty. Now, we need to redraw the window
			err = DrawWindow( currentItem->windowRef  );
			require_noerr( err, InvalidateAndRedrawWindows_err );
		}
		
		// grab the next context
		currentItem = currentItem->nextItem;
		
	}

InvalidateAndRedrawWindows_err:

	return err;

}

// ------------------------------------------------------------------------------
// CloseDemoWindow													[INTERNAL]
// ------------------------------------------------------------------------------

static
void CloseDemoWindow(
	WindowRef windowRef )
{
	WindowListItem 		*currentItem = gWindowList;
	WindowListItem 		*previousItem = NULL;
	DrawContextStruct	*windowContext;

	// search through the window list, looking for the windowRef
	while( currentItem != NULL )
	{
		// check to see if the current item is the one that we're looking for
		if ( currentItem->windowRef == windowRef )
		{
			// we've got a match, so set the previous item's next item pointer
			// to the current item's next item pointer
			if ( previousItem != NULL )
			{
				previousItem->nextItem = currentItem->nextItem;
			}
			else
			{
				gWindowList = currentItem->nextItem;
			}
			
			
			// free up the current item
			free( currentItem );
			break;
		}
		
		// move on to the next item
		previousItem = currentItem;
		currentItem = currentItem->nextItem;
	}
	
	// get the draw context struct from the window's refCon
	windowContext = (DrawContextStruct *) GetWRefCon( windowRef );
	
	// if we have a context, then we need to make sure to dispose of everything
	// that's in the context before we free the context.
	if ( windowContext != NULL )
	{
	
		// stop the CGContext for the port
		if ( windowContext->cgContext != NULL )
		{	
			CGrafPtr	windowPort;
		
			// get the window port for the window
			windowPort = GetWindowPort( windowRef );
					
			verify_noerr( 
				QDEndCGContext( windowPort, &windowContext->cgContext ) );
		}
		
		// dispose of the text layout object
		if ( windowContext->layoutObject != NULL ) 
		{
			verify_noerr( 
				ATSUDisposeTextLayout( windowContext->layoutObject ) );
		}
		
		// free the text buffer, if there is one
		if( windowContext->textBuffer != NULL )
		{
			free( windowContext->textBuffer );
		}
		
		// free the context itself
		free( windowContext );
		
	}

}

// ------------------------------------------------------------------------------
// AddWindowToList													[INTERNAL]
// ------------------------------------------------------------------------------

static
void AddWindowToList(
	WindowRef windowRef )
{	
	WindowListItem *newItem;
	
	// allocate a new item
	newItem = (WindowListItem *) malloc( sizeof( WindowListItem ) );
	require( newItem != NULL, AddWindowToList_err );
	
	// add the new item to the head of the list
	newItem->windowRef = windowRef;
	newItem->nextItem = gWindowList;
	gWindowList = newItem;

AddWindowToList_err:

	return;
}

// ------------------------------------------------------------------------------
// ResizeDemoWindow													[INTERNAL]
// ------------------------------------------------------------------------------

static
OSStatus ResizeDemoWindow(
	EventRef 	inEvent, 
	WindowRef 	windowRef )
{
	OSStatus			err;
	Rect				newBounds;
	Rect				oldBounds;
	short				newWidth;
	short				newLength;
	
	// get the event parameter which will tell us the new size
	err = GetEventParameter( inEvent, kEventParamCurrentBounds, typeQDRectangle,
		NULL, sizeof( Rect ), NULL, &newBounds );
	require_noerr( err, ResizeDemoWindow_err );

	// get the old bounds, just to see if we don't need to draw anything
	err = GetEventParameter( inEvent, kEventParamPreviousBounds,  typeQDRectangle,
		NULL, sizeof( Rect ), NULL, &oldBounds );
	require_noerr( err, ResizeDemoWindow_err );		
	
	// calculate the new width and length
	newWidth = newBounds.right - newBounds.left;
	newLength = newBounds.bottom - newBounds.top;
	
	// check to see if we need to redraw. If this is a simple translation,
	// then we don't.
	if ( ( newLength != ( oldBounds.bottom - oldBounds.top ) ) ||
		 ( newWidth !=	( oldBounds.right - oldBounds.left ) ) )
	{
		DrawContextStruct	*windowContext;
		CGrafPtr			windowPort;
	
		// get the draw context struct from the window's refCon
		windowContext = (DrawContextStruct *) GetWRefCon( windowRef );

#if BUGWORKAROUNDS
		
		// get the window port
		windowPort = GetWindowPort( windowRef );
			
		// okay, so this is really, really lame. I'm going to get rid of
		// the current cg context and get a new one. This is because I
		// don't have the time right now to figure out how to synchronize the
		// clipping regions and such for the window and the context. This
		// is really bad, beacuse the text layout needs to have the new
		// cg context tag set in it, which will invalidate the layout.
		
		// I only need to dispose of the context each time until I figure
		// out what's going wrong with the Quickdraw clipping regions.
		
		// dispose of the context
		err = QDEndCGContext( windowPort, &windowContext->cgContext );
		require_noerr( err, ResizeDemoWindow_err );
				
		// re-grab the cgContext from the port
		err = QDBeginCGContext( windowPort, &windowContext->cgContext );
		require_noerr( err, ResizeDemoWindow_err );
		
		// make sure that the context is re-set in the ATSUI object
		if ( windowContext->layoutObject != NULL )
		{
			ATSUAttributeTag tag = kATSUCGContextTag;
			ByteCount valueSize = sizeof( CGContextRef );
			ATSUAttributeValuePtr valuePtr = &windowContext->cgContext;
			
			err = ATSUSetLayoutControls( windowContext->layoutObject, 1, &tag,
				&valueSize, &valuePtr );
			require_noerr( err, ResizeDemoWindow_err );
			
		}
#endif
		
		// redraw the context
		err = DrawWindow( windowRef );
		require_noerr( err, ResizeDemoWindow_err );
		
	}

ResizeDemoWindow_err:

	return eventNotHandledErr;	

}

// ------------------------------------------------------------------------------
// InstallWindowEventHandlers										[INTERNAL]
// ------------------------------------------------------------------------------

static
OSStatus InstallWindowEventHandlers(
	WindowRef windowRef )
{
	static const EventTypeSpec 	inputEventSpec[] = 
		{ 	{ kEventClassTextInput, kEventTextInputUnicodeForKeyEvent } };
		
	static const EventTypeSpec	windowEventSpec[] =
		{	{ kEventClassWindow, kEventWindowClosed },
			{ kEventClassWindow, kEventWindowBoundsChanged },
			{ kEventClassWindow, kEventWindowDrawContent } };
		
	OSStatus 			err;
	DrawContextStruct	*newContext;
	CGrafPtr			windowPort;
	
	// allocate a new draw context
	newContext = calloc( 1, sizeof( DrawContextStruct ) );
	require_action( newContext != NULL, InstallWindowEventHandlers_err,
		err = paramErr );
		
	// get the window port for the window
	windowPort = GetWindowPort( windowRef );
		
	// grab the cgContext from the port
	err = QDBeginCGContext( windowPort, &newContext->cgContext );
	require_noerr( err, InstallWindowEventHandlersEvent_err );
	
	// install a key event handler
	err = InstallWindowEventHandler( windowRef,
		NewEventHandlerUPP( HandleKeyEvent ), GetEventTypeCount( inputEventSpec ), 
		inputEventSpec, (void *) windowRef, NULL );
	require_noerr( err, InstallWindowEventHandlersEvent_err );
	
	// install a general window event handler
	err = InstallWindowEventHandler( windowRef,
		NewEventHandlerUPP( HandleWindowEvent ), GetEventTypeCount( windowEventSpec ), 
		windowEventSpec, (void *) windowRef, NULL );
	require_noerr( err, InstallWindowEventHandlersEvent_err );
	
	// also, set the context as the window refcon
	SetWRefCon( windowRef, (long) newContext );

	return noErr;
	
InstallWindowEventHandlersEvent_err:

	// make sure that if we're bailing to get rid of the allocated buffer
	free( newContext );

InstallWindowEventHandlers_err:
	
	return err;

}

// ------------------------------------------------------------------------------
// HandleWindowEvent												[INTERNAL]
// ------------------------------------------------------------------------------

static
OSStatus HandleWindowEvent(
	EventHandlerCallRef inHandlerCallRef,
	EventRef 			inEvent, 
	void 				*inUserData )
{
#pragma unused( inHandlerCallRef )

	OSStatus 	err = eventNotHandledErr;
	WindowRef	windowRef;

	// cast away the user data to the context for easier handling
	windowRef = (WindowRef) inUserData;
	check( windowRef != NULL );
	
	// pass the control off to our handlers based on event kind
	switch ( GetEventKind( inEvent ) )
	{
		case kEventWindowClosed:		
			CloseDemoWindow( windowRef );
			break;
			
		case kEventWindowBoundsChanged:
			ResizeDemoWindow( inEvent, windowRef );
			break;
			
		case kEventWindowDrawContent:
		{					
			// redraw  the context
			DrawWindow( windowRef );
			break;
		}
			
		default:
			break;
	
	};
	
	return err;	

}


// ------------------------------------------------------------------------------
// HandleKeyEvent													[INTERNAL]
// ------------------------------------------------------------------------------

static
OSStatus HandleKeyEvent(
	EventHandlerCallRef inHandlerCallRef,
	EventRef 			inEvent, 
	void 				*inUserData )
{
#pragma unused( inHandlerCallRef )

	OSStatus			err;
	EventRef			keyboardEvent;
	UInt32				dataSize;
	UniCharCount		numNewCharacters;
	WindowRef			windowRef;
	DrawContextStruct	*context;
	
	// cast away the user data to the context for easier handling
	windowRef = (WindowRef) inUserData;
	
	// get the context from windowRef		
	context = (DrawContextStruct *) GetWRefCon( windowRef );
	check( context != NULL );
	
	// get the keyboard event
	err = GetEventParameter( inEvent, kEventParamTextInputSendKeyboardEvent,
		typeEventRef, NULL, sizeof( EventRef ), NULL, &keyboardEvent );
	require_noerr( err, HandleKeyEvent_err );
	
	// get the data size of the event
	err = GetEventParameter( keyboardEvent, kEventParamKeyUnicodes,
		typeUnicodeText, NULL, 0, &dataSize, NULL );
	require_noerr( err, HandleKeyEvent_err );
	
	// calculate the number of characters based on the data size
	numNewCharacters = dataSize / sizeof( UniChar );
	
	// check to see if the buffer is large enough to fit the key
	if ( context->textBufferSize < ( context->characterCount + 
		numNewCharacters ) )
	{
		ByteCount newSize;
	
		// nope. We need to resize the buffer. First, we need to pick a
		// size.
		if ( context->textBufferSize == 0 )
		{
			// the buffer has never been allocated. So, set it to a default
			// size plus whatever we got
			newSize = dataSize + kDefaultUnicodeBufferSize;
		}
		else
		{
			// the buffer has been allocated, so just double it's current
			// size. This is actually one of the most efficient ways to
			// use realloc
			newSize = dataSize + (context->textBufferSize * 2);
		}
		
		// allocate the new buffer
		context->textBuffer = realloc( context->textBuffer, newSize );
		require_action( context->textBuffer != NULL, HandleKeyEvent_err,
			err = memFullErr );
			
		// reset the text pointer, if there is one
		if ( context->layoutObject != NULL ) {
			err = ATSUSetTextPointerLocation( context->layoutObject, 
				context->textBuffer, kATSUFromTextBeginning, 
				kATSUToTextEnd, context->characterCount );
			require_noerr( err, HandleKeyEvent_err );
		}
			
		// set the new size in the buffer size
		context->textBufferSize = newSize / sizeof( UniChar );
	}
	
	// add the keys to the buffer from the event
	err = GetEventParameter( keyboardEvent, kEventParamKeyUnicodes,
		typeUnicodeText, NULL, dataSize, NULL,
		(void*) ( context->textBuffer + context->characterCount ) );
	require_noerr( err, HandleKeyEvent_err );
		
	// if the layout's already been allocated, then add the characters
	// to it. Otherwise, do nothing and let the draw function handle
	// everything
	if ( context->layoutObject != NULL ) {
		
		// insert the text into the layout
		err = ATSUTextInserted( context->layoutObject,
			context->characterCount, numNewCharacters );
		require_noerr( err, HandleKeyEvent_err );
		
	}
	
	// increment the character count stored in the context
	context->characterCount += numNewCharacters;

	// draw the context
	err = DrawWindow( windowRef );
	require_noerr( err, HandleKeyEvent_err );

HandleKeyEvent_err:

	return err;

}

// ------------------------------------------------------------------------------
// AddNewTextLayoutToContext										[INTERNAL]
// ------------------------------------------------------------------------------

static
OSStatus AddNewTextLayoutToContext( 
	DrawContextStruct	*context )
{
	OSStatus err;
	ATSUAttributeTag tag;
	ByteCount valueSize;
	ATSUAttributeValuePtr valuePtr;
	UniCharCount runLength = kATSUToTextEnd;
	
	// create a text layout object
	err = ATSUCreateTextLayoutWithTextPtr( context->textBuffer,
		kATSUFromTextBeginning, kATSUToTextEnd, context->characterCount, 1,
		&runLength, &gGlobalStyle, &context->layoutObject );
	require_noerr( err, AddNewTextLayoutToContext_err );
	
	// add the cgContext to the text layout
	tag = kATSUCGContextTag;
	valueSize = sizeof( CGContextRef );
	valuePtr = &context->cgContext;
	err = ATSUSetLayoutControls( context->layoutObject, 1, &tag, &valueSize,
		&valuePtr );
	require_noerr( err, AddNewTextLayoutToContext_err );
	
	// set font substitution for the new layout	
	err = ATSUSetTransientFontMatching( context->layoutObject, true );
	require_noerr( err, AddNewTextLayoutToContext_err );
	
	// now, check the menu and see if we need to install a callback
	switch ( gDemoMenuSelection )
	{
		case kDemoMenuItemStretch:
			err = InstallStrechyGlyphCallback( context->layoutObject );
			require_noerr( err, AddNewTextLayoutToContext_err );
			break;
			
		case kDemoMenuItemShrink:
			err = InstallShrinkyGlyphCallback( context->layoutObject );
			require_noerr( err, AddNewTextLayoutToContext_err );
			break;
		
		case kDemoMenuItemWhitespaceReplace:		
			err = InstallGlyphReplacementCallback( context->layoutObject );
			require_noerr( err, AddNewTextLayoutToContext_err );
			break;
			
		case kDemoMenuItemSineWave:
			err = InstallGlyphWaveCallback( context->layoutObject );
			require_noerr( err, AddNewTextLayoutToContext_err );
			break;
		
		case kDemoMenuItemNone:
		default:		
			break;
			
	}


AddNewTextLayoutToContext_err:
	
	return err;
}

// ------------------------------------------------------------------------------
// DrawWindow														[INTERNAL]
// ------------------------------------------------------------------------------

static
OSStatus DrawWindow(
	WindowRef windowRef )
{
	OSStatus 				err = noErr;
	DrawContextStruct		*context;
	
	// get the context from windowRef		
	context = (DrawContextStruct *) GetWRefCon( windowRef );
	check( context != NULL );
	
	// we only need to do this if there are characters in the buffer. If there
	// aren't any characters, then there's nothing to draw.
	if ( context->characterCount > 0 )
	{	
		CGRect 		cgRect;
		CGrafPtr	windowPort;
		Rect		windowBounds;
		
		// get the window port for the window
		windowPort = GetWindowPort( windowRef );
	
		// sync the origins
		SyncCGContextOriginWithPort( context->cgContext, windowPort );
		
		// get the bounds for the port
		GetPortBounds( windowPort, &windowBounds );
			
		// now, we need to set the clipping region for the document. Let's 
		cgRect = CGRectMake( windowBounds.left, windowBounds.top,
			windowBounds.right, windowBounds.bottom );
		
#if BUGWORKAROUNDS
		// reset the clipping region for the document. This should stop that
		// silly bug which is causing stuff to draw into the window tilebar
		CGContextClipToRect( context->cgContext, cgRect );
#endif
			
		// save the current state
		CGContextSaveGState(context->cgContext);
		
		// set a fill color in the CGContext. Make it white.
		CGContextSetRGBFillColor( context->cgContext, 1.0, 1.0, 1.0, 1.0 );
		
		// fill the window rect to clear it out
		CGContextFillRect( context->cgContext, cgRect );
		
		// restore the old state
		CGContextRestoreGState( context->cgContext );
		
		
		// if there isn't a layout object, then we need to add one to the
		// context
		if ( context->layoutObject == NULL )
		{
			err = AddNewTextLayoutToContext( context );
			require_noerr( err, DrawContext_err );
		}
		
		// based on the current options selection, either just draw a single
		// line, or do the fancy glyph rotate stuff.
		switch ( gOptionsMenuSelection )
		{
			case kOptionsMenuTextBurst:
				err = DrawGlyphBurst( context, windowBounds );
				require_noerr( err, DrawContext_err );
				break;
			
			case kOptionsMenuItemNone:
			default:
				err = DrawSingleLine( context, windowBounds );
				require_noerr( err, DrawContext_err );
				break;
		}

		// flush the CGContext
		CGContextFlush( context->cgContext );

	}
		
DrawContext_err:

	return err;

}

// ------------------------------------------------------------------------------
// DrawSingleLine													[INTERNAL]
// ------------------------------------------------------------------------------

static
OSStatus DrawSingleLine(
	DrawContextStruct *context,
	Rect			  windowBounds )
{
	OSStatus				err;
	ATSTrapezoid			glyphBounds;
	ItemCount				numGlyphBounds;
	Fixed					lineHeight;
	ATSUTextMeasurement		xPosition;
	ATSUTextMeasurement		yPosition;
	
	// set the xPosition and the yPosition as the boundries. 
	xPosition = windowBounds.left << 16;
	yPosition = windowBounds.bottom << 16;
	
	// we need to calculate where the line should start, so get the height of the
	// line.
	err = ATSUGetGlyphBounds( context->layoutObject, 0, 0, kATSUFromTextBeginning,
		kATSUToTextEnd, kATSUseCaretOrigins, 1, &glyphBounds, &numGlyphBounds );
	require_noerr( err, DrawSingleLine_err );
	
	lineHeight = glyphBounds.lowerRight.y - glyphBounds.upperRight.y;
	
	// adjust the line position based on the line height
	yPosition -= lineHeight;
	
	// draw!
	err = ATSUDrawText( context->layoutObject, kATSUFromTextBeginning,
		kATSUToTextEnd, xPosition, yPosition );
	check_noerr( err );
	
DrawSingleLine_err:
	
	return err;
}

// ------------------------------------------------------------------------------
// DrawGlyphBurst													[INTERNAL]
// ------------------------------------------------------------------------------

static
OSStatus DrawGlyphBurst(
	DrawContextStruct 	*context,
	Rect				windowBounds )
{
	OSStatus				err;
	ATSUTextMeasurement		xPosition;
	ATSUTextMeasurement		yPosition;
	Fixed					numDegreesPerLine;
	Fixed					currentDegree;
	ATSUAttributeTag		tag = kATSULineRotationTag;
	ByteCount				valueSize = sizeof( Fixed );
	ATSUAttributeValuePtr	valuePtr = &currentDegree;
	
	// initialize the current degree to 360 degrees
	currentDegree = (360 << 16);
	
	// calculate the number of degrees for each glyph burst
	numDegreesPerLine = currentDegree / kGlyphBurstLines;
	
	// set the xPosition and the yPosition at the center of the window
	xPosition = ( ( windowBounds.right - windowBounds.left ) / 2 ) << 16;
	yPosition = ( ( windowBounds.bottom - windowBounds.top ) / 2 ) << 16;
		
	// loop through until all lines are drawn
	while ( currentDegree > 0 )
	{
		// set the current rotation degree in the text layout object
		err = ATSUSetLayoutControls( context->layoutObject, 1, &tag,
			&valueSize, &valuePtr );
		require_noerr( err, DrawGlyphBurst_err );
		
		// draw!
		err = ATSUDrawText( context->layoutObject, kATSUFromTextBeginning,
			kATSUToTextEnd, xPosition, yPosition );
			
		// decrement the current degree
		currentDegree -= numDegreesPerLine;
	}
	
	// set the current degree back to zero, in case it wasn't
	currentDegree = 0;
	err = ATSUSetLayoutControls( context->layoutObject, 1, &tag,
		&valueSize, &valuePtr );
	require_noerr( err, DrawGlyphBurst_err );	
		
DrawGlyphBurst_err:
		
	return err;	

}