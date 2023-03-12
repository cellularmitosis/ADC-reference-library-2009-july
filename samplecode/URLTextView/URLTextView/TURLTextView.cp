/*
    File:		TURLTextView.cp
    
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

#include "TRect.h"
#include "TURLTextView.h"

//-----------------------------------------------------------------------------------
//	macros
//-----------------------------------------------------------------------------------
//
#define kTViewClassID	CFSTR( "com.apple.sample.URLTextView" )

//-----------------------------------------------------------------------------------
//	constants
//-----------------------------------------------------------------------------------
//
enum
{
	kEventParamURL	= 'URL ',		// typeCFURLRef
	kEventParamText = 'TEXT',		// typeCFStringRef
	
	typeCFURLRef	= 'cfur'
};

//-----------------------------------------------------------------------------------
//	HIURLTextViewCreate
//-----------------------------------------------------------------------------------
//
OSStatus HIURLTextViewCreate( const HIRect* inBounds, CFURLRef inURL, CFStringRef inText, HIViewRef* outURLTextView )
{
	return TURLTextView::Create( outURLTextView, inBounds, inURL, inText );
}

//-----------------------------------------------------------------------------------
//	HIURLTextViewSetURL
//-----------------------------------------------------------------------------------
//
OSStatus HIURLTextViewSetURL( HIViewRef inURLTextView, CFURLRef inURL )
{
	return SetControlData( inURLTextView, kControlEntireControl, kControlURLTag, sizeof( inURL ), &inURL );
}

//-----------------------------------------------------------------------------------
//	HIURLTextViewCopyURL
//-----------------------------------------------------------------------------------
//
OSStatus HIURLTextViewCopyURL( HIViewRef inURLTextView, CFURLRef* outURL )
{
	return GetControlData( inURLTextView, kControlEntireControl, kControlURLTag, sizeof( *outURL ), outURL, NULL );
}

//-----------------------------------------------------------------------------------
//	HIURLTextViewSetText
//-----------------------------------------------------------------------------------
//
OSStatus HIURLTextViewSetText( HIViewRef inURLTextView, CFStringRef inText )
{
	return SetControlData( inURLTextView, kControlEntireControl, kControlURLTextTag, sizeof( inText ), &inText );
}

//-----------------------------------------------------------------------------------
//	HIURLTextViewCopyText
//-----------------------------------------------------------------------------------
//
OSStatus HIURLTextViewCopyText( HIViewRef inURLTextView, CFStringRef* outText )
{
	return GetControlData( inURLTextView, kControlEntireControl, kControlURLTextTag, sizeof( *outText ), outText, NULL );
}

//-----------------------------------------------------------------------------------
//	TURLTextView constructor
//-----------------------------------------------------------------------------------
//
TURLTextView::TURLTextView(
	HIViewRef			inControl )
	:	TView( inControl ),
		fURL( NULL ),
		fText( NULL )
{
	ActivateInterface( kKeyboardFocus );
	ChangeAutoInvalidateFlags( kAutoInvalidateOnActivate | kAutoInvalidateOnHilite, 0 );
}

//-----------------------------------------------------------------------------------
//	TURLTextView destructor
//-----------------------------------------------------------------------------------
//	Clean up after yourself.
//
TURLTextView::~TURLTextView()
{
	if ( fURL != NULL )
		CFRelease( fURL );
	if ( fText != NULL )
		CFRelease( fText );
}

//-----------------------------------------------------------------------------------
//	CreateInitializationEvent
//-----------------------------------------------------------------------------------
// Creates an init event for the URLTextView HIObject.
//
EventRef TURLTextView::CreateInitializationEvent( CFURLRef inURL, CFStringRef inText )
{
	EventRef event = TView::CreateInitializationEvent();
	if ( event != NULL )
	{
		if ( inURL != NULL )
			SetEventParameter( event, kEventParamURL, typeCFURLRef, sizeof( inURL ), &inURL );
		if ( inText != NULL )
			SetEventParameter( event, kEventParamText, typeCFStringRef, sizeof( inText ), &inText );
	}
		
	return event;
}

//-----------------------------------------------------------------------------------
//	GetKind
//-----------------------------------------------------------------------------------
//
ControlKind TURLTextView::GetKind()
{
	static const ControlKind kMyKind = { 'TurV', 'TurV' };
	return kMyKind;
}

//-----------------------------------------------------------------------------------
//	Create
//-----------------------------------------------------------------------------------
//
OSStatus TURLTextView::Create(
	HIViewRef*			outControl,
	const HIRect*		inBounds,
	CFURLRef			inURL,
	CFStringRef			inText )
{
	OSStatus			err;
	EventRef			event = CreateInitializationEvent( inURL, inText );
	
	RegisterClass();

	err = HIObjectCreate( kTViewClassID, event, (HIObjectRef*) outControl );
	
	ReleaseEvent( event );

	if ( err == noErr )
		HIViewSetFrame( *outControl, inBounds );
	
	return err;
}

//-----------------------------------------------------------------------------------
//	RegisterClass
//-----------------------------------------------------------------------------------
//
void TURLTextView::RegisterClass()
{
	static bool sRegistered;
	
	if ( !sRegistered )
	{
		TView::RegisterSubclass( kTViewClassID, Construct );
		sRegistered = true;
	}
}

//-----------------------------------------------------------------------------------
//	Construct
//-----------------------------------------------------------------------------------
//
OSStatus TURLTextView::Construct(
	ControlRef		inControl,
	TView**			outView )
{
	*outView = new TURLTextView( inControl );
	return noErr;
}

//-----------------------------------------------------------------------------------
//	Initialize
//-----------------------------------------------------------------------------------
//	The control is set up.  Do the last minute stuff that needs to be done like
//
OSStatus TURLTextView::Initialize(
	TCarbonEvent&		inEvent )
{
	OSStatus			err = noErr;
	
	GetEventParameter( inEvent, kEventParamURL, typeCFURLRef, NULL, sizeof( fURL ), NULL, &fURL );
	GetEventParameter( inEvent, kEventParamText, typeCFStringRef, NULL, sizeof( fText ), NULL, &fText );
	
	if ( fURL != NULL )
		CFRetain( fURL );
	if ( fText != NULL )
		CFRetain( fText );
	
	return err;
}

//-----------------------------------------------------------------------------------
//	SetFocusPart
//-----------------------------------------------------------------------------------
//	Focus ourselves.
//
OSStatus TURLTextView::SetFocusPart(
	ControlPartCode		inDesiredFocus,
	RgnHandle			inInvalidRgn,
	Boolean				inFocusEverything,
	ControlPartCode*	outActualFocus )
{
#pragma unused( inInvalidRgn )

	ControlPartCode currentFocus;
	HIViewGetFocusPart( GetViewRef(), &currentFocus );
	
	//
	// We only support focusing if inFocusEverything is true. Otherwise, we are
	// using traditional focusing rules (focus only on edit fields, etc.) and we
	// do not have any traditionally focusable parts.
	//
	// We can focus ourselves if we are currently not focused. If we are focused,
	// then since we have only a single part, moving to the next or previous part
	// will remove the focus from us and move it to the next control.
	//
	switch ( inDesiredFocus )
	{
		case kControlFocusNextPart:
		case kControlFocusPrevPart: // toggle between focused or not
			if ( ( currentFocus == kControlNoPart ) && inFocusEverything )
				inDesiredFocus = kControlLabelPart;
			else
				inDesiredFocus = kControlNoPart;
			break;
		
		default:  // we don't focus anything but the label
			if( inDesiredFocus != kControlLabelPart )
				inDesiredFocus = kControlNoPart;
			break;
	}
	
	// if the focus changes, make sure we redraw
	if ( currentFocus != inDesiredFocus )
		Invalidate();

	*outActualFocus = inDesiredFocus;
	return noErr;
}

//-----------------------------------------------------------------------------------
//	TextInput
//-----------------------------------------------------------------------------------
//	Handles text input.
//
OSStatus TURLTextView::TextInput(
	TCarbonEvent&		inEvent )
{
	OSStatus	result = eventNotHandledErr;
	
	if ( inEvent.GetKind() == kEventTextInputUnicodeForKeyEvent )
	{
		UniChar uch = 0;
		inEvent.GetParameter<UniChar>( kEventParamTextInputSendText, typeUnicodeText, &uch );
		if ( uch == kSpaceCharCode )
		{
			ControlPartCode part;
			HIViewSimulateClick( GetViewRef(), kControlLabelPart, 0, &part );
			result = noErr;
		}
	}
	
	return result;
}

//-----------------------------------------------------------------------------------
//	Draw
//-----------------------------------------------------------------------------------
//	The fun part of the control
//
void TURLTextView::Draw(
	RgnHandle				inLimitRgn,
	CGContextRef			inContext )
{
#pragma unused( inLimitRgn )

	CFStringRef text = CopyText();
	if ( text != NULL )
	{
		RGBColor		rgb = { 0, 0, 0xFFFF };	// default color is pure blue
		TRect			bounds( Bounds() );
		Rect			qdBounds;
		SInt16			baseline;
		float			yUnderline;
		Boolean			showFocus = false;
		ControlPartCode	focusPart;
	
		// Determine the color for disabled text. Since there is no SetThemeTextColor for
		// a CGContext, we get the RGB color and then manually place it into the context
		if ( !IsActive() || !IsEnabled() )
			GetThemeTextColor( kThemeTextColorDialogInactive, 32, true, &rgb );
		
		// set an appropriate color for the text
		CGContextSetRGBFillColor( inContext,
								  (float) rgb.red / 0xFFFF,
								  (float) rgb.green / 0xFFFF,
								  (float) rgb.blue / 0xFFFF,
								  1.0 );
		
		// draw the text using the control's entire bounds
		qdBounds = bounds;
		DrawThemeTextBox( text, kThemeSystemFont, kThemeStateActive, false, &qdBounds, teFlushDefault, inContext );
		
		// switch to the bounds of the text itself
		bounds.Set( TextBounds( &baseline ) );

		// draw the underline
		yUnderline = bounds.MaxY() + baseline + 2.5;
		CGContextBeginPath( inContext );
		CGContextMoveToPoint( inContext, bounds.MinX(), yUnderline );
		CGContextAddLineToPoint( inContext, bounds.MaxX(), yUnderline );
		CGContextClosePath( inContext );
		CGContextSetRGBStrokeColor( inContext,
									(float) rgb.red / 0xFFFF,
									(float) rgb.green / 0xFFFF,
									(float) rgb.blue / 0xFFFF,
									1.0 );
		CGContextStrokePath( inContext );
		
		// we are showing focus if the control is hilited (during mouse tracking) or has keyboard focus
		if ( GetHilite() == kControlLabelPart )
			showFocus = true;
		else if ( HIViewGetFocusPart( GetViewRef(), &focusPart ) == noErr && focusPart == kControlLabelPart )
			showFocus = true;
			
		// draw focus (but only if we're active and enabled)
		if ( showFocus && IsActive() && IsEnabled() )
		{
			qdBounds = bounds;
			DrawThemeFocusRect( &qdBounds, true );
		}
			
		CFRelease( text );
	}
}

//-----------------------------------------------------------------------------------
//	ControlHit
//-----------------------------------------------------------------------------------
//	In response to a click on the control, we use LaunchServices to open the URL.
//
OSStatus TURLTextView::ControlHit(
	ControlPartCode		inPart,
	UInt32				inKeyModifiers )
{
#pragma unused( inPart, inKeyModifiers )

	OSStatus result = eventNotHandledErr;
	
	// we only support one part code
	check( inPart == kControlLabelPart );
	
	if ( fURL != NULL )
		result = LSOpenCFURLRef( fURL, NULL );
	
	return result;
}

//-----------------------------------------------------------------------------------
//	HitTest
//-----------------------------------------------------------------------------------
//	Determine whether a click was in the control or not
//
ControlPartCode TURLTextView::HitTest(
	const HIPoint&		inWhere )
{
	ControlPartCode		part = kControlNoPart;

	if (	IsVisible()
		 && IsEnabled()
		 && IsActive()
		 && CGRectContainsPoint( TextBounds( NULL ), inWhere ) )
	{
		part = kControlLabelPart;
	}

	return part;
}

//-----------------------------------------------------------------------------------
//	GetRegion
//-----------------------------------------------------------------------------------
//
OSStatus TURLTextView::GetRegion(
	ControlPartCode		inPart,
	RgnHandle			outRgn )
{
	OSStatus			err = noErr;
	
	if ( inPart == kControlContentMetaPart
			|| inPart == kControlStructureMetaPart
		 /* || inPart == kControlOpaqueRegionMetaPart */ )
	{
		TRect	bounds( Bounds() );
		Rect	qdBounds = bounds;
	
		// leave room for the focus rect
		// ••• how do we know how much room to leave?
		InsetRect( &qdBounds, -3, -3 );
		
		RectRgn( outRgn, &qdBounds );
	}
	
	return err;
}

//-----------------------------------------------------------------------------------
//	GetOptimalSize
//-----------------------------------------------------------------------------------
//	Returns the optimal size of the view, based on its content.
//
OSStatus TURLTextView::GetOptimalSize(
	HISize*				outSize,
	float*				outBaseLine )
{
	SInt16 baseline;
	HIRect bounds = TextBounds( &baseline );
	
	if ( outSize != NULL )
	{
		outSize->width = bounds.size.width;
		outSize->height = bounds.size.height;
	}
	
	if ( outBaseLine != NULL )
		*outBaseLine = baseline;
		
	return noErr;
}

//-----------------------------------------------------------------------------------
//	GetData
//-----------------------------------------------------------------------------------
//	Implements GetControlData on the view.
//
OSStatus TURLTextView::GetData(
	OSType				inTag,
	ControlPartCode		inPart,
	Size				inSize,
	Size*				outSize,
	void*				inPtr )
{
	// TView handles kControlKindTag for us
	OSStatus err = TView::GetData( inTag, inPart, inSize, outSize, inPtr );
	if ( err == noErr )
		return noErr;
	
	// assume that we'll handle this request
	err = noErr;
	
	switch( inTag )
	{
		case kControlURLTag:
			if ( inPtr != NULL )
			{
				if ( inSize != sizeof( CFURLRef ) )
				{
					err = errDataSizeMismatch;
				}
				else
				{
					*(CFURLRef*) inPtr = fURL;
					if ( fURL != NULL )
						CFRetain( fURL );
				}
			}
			*outSize = sizeof( CFURLRef );
			break;
		
		case kControlURLTextTag:
			if ( inPtr != NULL )
			{
				if ( inSize != sizeof( CFStringRef ) )
				{
					err = errDataSizeMismatch;
				}
				else
				{
					*(CFStringRef*) inPtr = fText;
					if ( fText != NULL )
						CFRetain( fText );
				}
			}
			*outSize = sizeof( CFStringRef );
			break;
		
		default:
			err = eventNotHandledErr;
			break;
	}
	
	return err;
}

//-----------------------------------------------------------------------------------
//	SetData
//-----------------------------------------------------------------------------------
//	Implements SetControlData on the view.
//
OSStatus TURLTextView::SetData(
	OSType				inTag,
	ControlPartCode		inPart,
	Size				inSize,
	const void*			inPtr )
{
#pragma unused( inPart )

	// assume that we'll handle this request
	OSStatus err = noErr;
	
	switch( inTag )
	{
		case kControlURLTag:
			if ( inPtr != NULL )
			{
				if ( inSize != sizeof( CFURLRef ) )
				{
					err = errDataSizeMismatch;
				}
				else
				{
					if ( fURL != NULL )
						CFRelease( fURL );
					fURL = *(CFURLRef*) inPtr;
					if ( fURL != NULL )
						CFRetain( fURL );
					
					err = Invalidate();
				}
			}
			break;
	
		case kControlURLTextTag:
			if ( inPtr != NULL )
			{
				if ( inSize != sizeof( CFStringRef ) )
				{
					err = errDataSizeMismatch;
				}
				else
				{
					if ( fText != NULL )
						CFRelease( fText );
					fText = *(CFStringRef*) inPtr;
					if ( fText != NULL )
						CFRetain( fText );
					
					err = Invalidate();
				}
			}
			break;
	
		default:
			err = eventNotHandledErr;
			break;
	}
	
	return err;
}

//-----------------------------------------------------------------------------------
//	CopyText
//-----------------------------------------------------------------------------------
//	Returns the text to be drawn by the view.
//
CFStringRef TURLTextView::CopyText()
{
	CFStringRef	text = NULL;
	
	if ( fText != NULL )
	{
		text = (CFStringRef) CFRetain( fText );
	}
	else if ( fURL != NULL )
	{
		// if LSCopyDisplayName returns an error, or returns empty text,
		// then we just use a string derived from the URL itself
		if ( LSCopyDisplayNameForURL( fURL, &text ) != noErr || CFStringGetLength( text ) == 0 )
		{
			if ( text != NULL )
			{
				CFRelease( text );
				text = NULL;
			}
			
			// if we have a valid URL, try using its net location and path
			if ( CFURLCanBeDecomposed( fURL ) )
			{
				CFStringRef netLocation = CFURLCopyNetLocation( fURL );
				CFStringRef path = CFURLCopyPath( fURL );
				
				if ( netLocation != NULL && path != NULL )
				{
					text = CFStringCreateMutableCopy( NULL, 0, netLocation );
					CFStringAppend( (CFMutableStringRef) text, path );
				}
				
				if ( netLocation != NULL )
					CFRelease( netLocation );
				if ( path != NULL )
					CFRelease( path );
			}
			
			// if all else fails, just use the text of the URL itself
			if ( text == NULL )
				text = (CFStringRef) CFRetain( CFURLGetString( fURL ) );
		}
	}
		
	return text;
}

//-----------------------------------------------------------------------------------
//	TextBounds
//-----------------------------------------------------------------------------------
//	Determines the bounds of the text displayed by the view.
//
HIRect TURLTextView::TextBounds( SInt16* outBaseline )
{
	HIRect		bounds = Bounds();
	CFStringRef text = CopyText();
	
	if ( text != NULL )
	{
		Point	qdSize;
		SInt16	baseline;
		
		GetThemeTextDimensions( text, kThemeSystemFont, kThemeStateActive, false, &qdSize, &baseline );
		
		bounds.size.width = qdSize.h;
		bounds.size.height = qdSize.v;
		
		if ( outBaseline != NULL )
			*outBaseline = baseline;
		
		CFRelease( text );
	}
	
	return bounds;
}

/*
	To do:
	
	x	draw text in gray when inactive
	x	hit-testing should not return a hit if the click isn't in the bounds of the text
	x	keyboard focus
	x	use LSCopyDisplayNameForURL to get a display name if no display text is specified
	x	specify URL using CFURLRef instead of CFStringRef? Or perhaps allow either?
	x	add cover APIs
	x	GetOptimalSize
	ControlData tags for getting and setting URL/text
	URL underline presence or absence
	can URL text be selected/copied
	ControlFontStyle
	whether we should be focusable even in absence of full keyboard focusing
	accessibility
	drag support - allow dragging URL to desktop to get a .webloc file or something similar
	allow specifying LSLaunchURLSpec that's used when opening the URL
*/

