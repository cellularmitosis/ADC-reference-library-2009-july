/*
	File:		ColorSwatch.cp

    Version:	Mac OS X

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

#include "ColorSwatch.h"

// -----------------------------------------------------------------------------
//	constants
// -----------------------------------------------------------------------------
//
#define kTViewColorSwatchClassID	CFSTR( "com.apple.colorswatch" )

#define kEventParamSwatchColor		'sClr'

// This constant was accidentally left private for Jaguar.
// It is to be used with the drag tracking events to indicate that a control
// can accept the incoming drag.
//
// Use it with a typeBoolean
const UInt32 kEventParamControlLikesDrag = 'cldg';

// -----------------------------------------------------------------------------
//	HIColorSwatchCreate
// -----------------------------------------------------------------------------
//	High-level API for creating a color swatch control.
//
OSStatus HIColorSwatchCreate(
	WindowRef		inWindow,
	const HIRect*	inBounds,
	RGBColor		inSwatchColor,
	ControlRef*		outControl )
{
	return ColorSwatch::Create( inWindow, inBounds, inSwatchColor, outControl );
}

// -----------------------------------------------------------------------------
//	HIColorSwatchGetColor
// -----------------------------------------------------------------------------
//	High-level APIs for getting and setting the swatch color.
//
OSStatus HIColorSwatchGetColor(
	ControlRef	inControl,
	RGBColor*	inColor )
{
	return GetControlData( inControl, kControlEntireControl,
			kControlSwatchColorTag, sizeof( RGBColor ), inColor, NULL );
}

// -----------------------------------------------------------------------------
//	HIColorSwatchSetColor
// -----------------------------------------------------------------------------
//
OSStatus HIColorSwatchSetColor(
	ControlRef		inControl,
	RGBColor		inColor )
{
	return SetControlData( inControl, kControlEntireControl,
			kControlSwatchColorTag, sizeof( RGBColor ), &inColor );
}

// -----------------------------------------------------------------------------
//	ColorSwatch constructor
// -----------------------------------------------------------------------------
//
ColorSwatch::ColorSwatch(
	HIViewRef			inControl )
	:	TView( inControl ),
		fCanAcceptDrag( false ),
		fCurrentFocusPart( kControlNoPart )
{
	// turn on drag tracking for this view
	SetControlDragTrackingEnabled( GetViewRef(), true );
	
	// add autoinvalidation
	ChangeAutoInvalidateFlags(	kAutoInvalidateOnHilite   |
								kAutoInvalidateOnActivate |
								kAutoInvalidateOnEnable,
								NULL );
}

// -----------------------------------------------------------------------------
//	ColorSwatch destructor
// -----------------------------------------------------------------------------
//
ColorSwatch::~ColorSwatch()
{
}

// -----------------------------------------------------------------------------
//	Create
// -----------------------------------------------------------------------------
//	Public class method for creating a ColorSwatch
//
OSStatus ColorSwatch::Create(
	WindowRef		inWindow,
	const HIRect*	inBounds,
	RGBColor		inSwatchColor,
	ControlRef*		outControl )
{
	OSStatus	err = noErr;
	ControlRef	root;
	EventRef	event = TView::CreateInitializationEvent(); // create initialization event
	
	require_action( event != NULL, CantCreateEvent, err = eventInternalErr );
	
	// register the subclass
	static bool sRegistered = false;
	if( !sRegistered )
	{
		TView::RegisterSubclass( kTViewColorSwatchClassID, ColorSwatch::Construct );
		sRegistered = true;
	}
	
	// add swatch color parameter
	err = SetEventParameter( event, kEventParamSwatchColor, typeRGBColor,
		sizeof( inSwatchColor ), &inSwatchColor );
	require_noerr( err, CantAddParameter );
	
	// instantiate the object
	err = HIObjectCreate( kTViewColorSwatchClassID, event, (HIObjectRef*) outControl );
	
	// add to color swatch view to parent view
	if( err == noErr )
	{
		if( inWindow != NULL )
		{
			GetRootControl( inWindow, &root );
			HIViewAddSubview( root, *outControl );
		}
		
		HIViewSetFrame( *outControl, inBounds );
	}

CantAddParameter:
	
	ReleaseEvent( event );

CantCreateEvent:
	
	return err;
}

// -----------------------------------------------------------------------------
//	GetKind
// -----------------------------------------------------------------------------
//
ControlKind ColorSwatch::GetKind()
{
	const ControlKind kColorSwatchKind = { 'cSwh', 'cSwh' };
	
	return kColorSwatchKind;
}

// -----------------------------------------------------------------------------
//	Construct
// -----------------------------------------------------------------------------
//
OSStatus ColorSwatch::Construct(
	ControlRef		inControl,
	TView**			outView )
{
	*outView = new ColorSwatch( inControl );
	
	return noErr;
}

// -----------------------------------------------------------------------------
//	Initialize
// -----------------------------------------------------------------------------
//	Called during HIObject construction, this is your subclasses' chance to
//	extract any parameters it might have added to the initialization event
//	passed into the HIObjectCreate call.
//
OSStatus ColorSwatch::Initialize(
	TCarbonEvent&		inEvent )
{
	OSStatus	err = noErr;
	
	err = TView::Initialize( inEvent );
	require_noerr( err, CantInitBaseClass );
	
	// look for the color
	err = GetEventParameter( inEvent, kEventParamSwatchColor, typeRGBColor,
			NULL, sizeof( fSwatchColor ), NULL, &fSwatchColor );
	
	// add drag & drop and focusing behaviors
	ActivateInterface( kDragAndDrop );
	ActivateInterface( kKeyboardFocus );
	
CantInitBaseClass:

	return err;
}

// -----------------------------------------------------------------------------
//	GetBehaviors
// -----------------------------------------------------------------------------
//	Returns our behaviors.
//
UInt32 ColorSwatch::GetBehaviors()
{
	return TView::GetBehaviors() | kControlSupportsDragAndDrop |
			kControlSupportsFocus;
}

// -----------------------------------------------------------------------------
//	Draw
// -----------------------------------------------------------------------------
//	Draw your view. You should draw based on VIEW coordinates, not frame
//	coordinates.
//
void ColorSwatch::Draw(
	RgnHandle			inLimitRgn,
	CGContextRef		inContext )
{
#pragma unused( inLimitRgn, inContext )

	ThemeDrawState			state = kThemeStateDisabled;
	ThemeButtonAdornment	adornment = ( fCurrentFocusPart == kControlButtonPart ) ?
										(ThemeButtonAdornment)kThemeFocusAdornment :
										(ThemeButtonAdornment)kThemeAdornmentNone;
	// set up button draw info
	if( IsActive() )
		state |= kThemeStateActive;
	
	if( GetControlHilite( GetViewRef() ) == kControlButtonPart )
		state |= kThemeStatePressed;
	
	ThemeButtonDrawInfo	info = { state, 0, adornment };
	
	RgnHandle drawRgn = NewRgn();
	Rect qdBounds;
	
	GetRegion( kControlButtonPart, drawRgn );
	GetRegionBounds( drawRgn, &qdBounds );
	
	// draw the bevel button background
	DrawThemeButton( &qdBounds, kThemeSmallBevelButton, &info, NULL, NULL, NULL, NULL );
	
	GetRegion( kControlImageWellPart, drawRgn );
	
	// draw the swatch color
	RGBForeColor( &fSwatchColor );
	PaintRgn( drawRgn );
	
	// draw the drag highlight if the view is capable of accepting it
	if( fCanAcceptDrag )
	{
		InsetRect( &qdBounds, 2, 2 );
		DrawThemeFocusRect( &qdBounds, true );
	}
	
	DisposeRgn( drawRgn );
}

// -----------------------------------------------------------------------------
//	HitTest
// -----------------------------------------------------------------------------
//	Asks your view to return what part of itself (if any) is hit by the point
//	given to it. The point is in VIEW coordinates, so you should get the view
//	rect to do bounds checking.
//
ControlPartCode ColorSwatch::HitTest(
	const HIPoint&		inWhere )
{
	ControlPartCode		part;
	
	// is the mouse on the button?
	if( CGRectContainsPoint( Bounds(), inWhere ) )
		part = kControlButtonPart;
	else
		part = kControlNoPart;
	
	return part;
}

// -----------------------------------------------------------------------------
//	GetRegion
// -----------------------------------------------------------------------------
//	This is called when someone wants to know certain metrics regarding this
//	view. The base class does nothing. Subclasses should handle their own parts,
//	such as the content region by overriding this method. The structure region
//	is, by default, the view's bounds. If a subclass does not have a region for
//	a given part, it  should always call the inherited method.
//
OSStatus ColorSwatch::GetRegion(
	ControlPartCode		inPart,
	RgnHandle			outRgn )
{
	OSStatus			err = noErr;
	HIRect				bounds = Bounds();
	Rect				qdBounds;
	
	qdBounds.top = (short)CGRectGetMinY( bounds );
	qdBounds.left = (short)CGRectGetMinX( bounds );
	qdBounds.bottom = (short)CGRectGetMaxY( bounds );
	qdBounds.right = (short)CGRectGetMaxX( bounds );
	
	// handle regions for these parts
	if( inPart == kControlContentMetaPart || inPart == kControlStructureMetaPart ||
		inPart == kControlButtonPart || inPart == kControlImageWellPart )
	{
		if( inPart == kControlImageWellPart ) // inset for the swatch
			InsetRect( &qdBounds, 5, 5 );
		
		RectRgn( outRgn, &qdBounds );
	}
	else
		err = errInvalidPartCode;
	
	return err;
}

// -----------------------------------------------------------------------------
//	GetData
// -----------------------------------------------------------------------------
//	Gets some data from our view.
//
OSStatus ColorSwatch::GetData(
	OSType				inTag,
	ControlPartCode		inPart,
	Size				inSize,
	Size*				outSize,
	void*				inPtr )
{
#pragma unused( inPart )

	OSStatus			err = noErr;
	
	switch( inTag )
	{
		case kControlKindTag:
			if ( inPtr )
			{
				if ( inSize != sizeof( ControlKind ) )
					err = errDataSizeMismatch;
				else
					(*(ControlKind*) inPtr) = GetKind();
			}
			*outSize = sizeof( ControlKind );
			break;
		
		case kControlSwatchColorTag:
			if ( inPtr )
			{
				if ( inSize != sizeof( RGBColor ) )
					err = errDataSizeMismatch;
				else
				{
					(*(RGBColor*) inPtr).red = fSwatchColor.red;
					(*(RGBColor*) inPtr).green = fSwatchColor.green;
					(*(RGBColor*) inPtr).blue = fSwatchColor.blue;
				}
			}
			*outSize = sizeof( RGBColor );
			break;
		
		default:
			err = eventNotHandledErr;
			break;
	}
	
	return err;
}

// -----------------------------------------------------------------------------
//	SetData
// -----------------------------------------------------------------------------
//	Sets some data on our control.
//
OSStatus ColorSwatch::SetData(
	OSType				inTag,
	ControlPartCode		inPart,
	Size				inSize,
	const void*			inPtr )
{
#pragma unused( inPart )

	OSStatus err = noErr;
	
	switch( inTag )
	{
		case kControlSwatchColorTag:
			if ( inPtr )
			{
				if ( inSize != sizeof( RGBColor ) )
					err = errDataSizeMismatch;
				else
				{
					fSwatchColor.red = (*(RGBColor*) inPtr).red;
					fSwatchColor.green = (*(RGBColor*) inPtr).green;
					fSwatchColor.blue = (*(RGBColor*) inPtr).blue;
					
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

// -----------------------------------------------------------------------------
//	GetOptimalSize
// -----------------------------------------------------------------------------
//	Someone wants to know this view's optimal size and text baseline, probably
//	to help do some type of layout.
//
OSStatus ColorSwatch::GetOptimalSize(
	HISize*				outSize,
	float*				outBaseLine )
{
#pragma unused( outBaseLine )

	if( outSize )
	{
		outSize->width = 30;
		outSize->height = 20;
	}
	
	return noErr;
}

// -----------------------------------------------------------------------------
//	GetSizeConstraints
// -----------------------------------------------------------------------------
//	Someone wants to know this view's minimum and maximum sizes, probably to
//	help do some type of layout.
//
OSStatus ColorSwatch::GetSizeConstraints(
	HISize*				outMin,
	HISize*				outMax )
{
#pragma unused( outMax )

	if( outMin )
	{
		outMin->width = 20;
		outMin->height = 20;
	}
	
	return noErr;
}

// -----------------------------------------------------------------------------
//	BoundsChanged
// -----------------------------------------------------------------------------
//	The bounds of our view have changed.
//
OSStatus ColorSwatch::BoundsChanged(
	UInt32 				inOptions,
	const HIRect& 		inOriginalBounds,
	const HIRect& 		inCurrentBounds,
	RgnHandle 			inInvalRgn )
{
#pragma unused( inOptions, inOriginalBounds, inCurrentBounds, inInvalRgn )

	// force redraw with new bounds
	return Invalidate();
}

// -----------------------------------------------------------------------------
//	DragEnter
// -----------------------------------------------------------------------------
//	A drag has entered our bounds.
//
bool ColorSwatch::DragEnter(
	DragRef				inDrag )
{
	RGBColor tempColor;
	
	fCanAcceptDrag = GetColorFromDrag( inDrag, &tempColor );
	
	// allow the view to redraw with the drag highlight
	if( fCanAcceptDrag )
	{
		// add event parameter to make HIView dragging work
		SetEventParameter( GetCurrentEvent(), kEventParamControlLikesDrag,
				typeBoolean, sizeof( fCanAcceptDrag ), &fCanAcceptDrag );
		
		Invalidate();
	}
	
	return fCanAcceptDrag;
}

// -----------------------------------------------------------------------------
//	DragLeave
// -----------------------------------------------------------------------------
//	A drag has left. Deal with it.
//
bool ColorSwatch::DragLeave(
	DragRef				inDrag )
{
	RGBColor tempColor;
	
	// if the drag was acceptable, redraw without the drag highlight
	if( fCanAcceptDrag )
	{
		Invalidate();
		fCanAcceptDrag = false;
	}
	
	return GetColorFromDrag( inDrag, &tempColor );
}

// -----------------------------------------------------------------------------
//	DragReceive
// -----------------------------------------------------------------------------
//	Deal with receiving a drag.
//
OSStatus ColorSwatch::DragReceive(
	DragRef				inDrag )
{	
	// grab the color from the drag and if it exists, cause the view to redraw
	if( GetColorFromDrag( inDrag, &fSwatchColor ) )
	{
		Invalidate();
		
		SendColorChangedCommandEvent(); // messge that the color changed
		
		return noErr;
	}
	
	return dragNotAcceptedErr;
}

// -----------------------------------------------------------------------------
//	Hit
// -----------------------------------------------------------------------------
//	Default hit method.
//
OSStatus ColorSwatch::ControlHit(
	ControlPartCode		inPart,
	UInt32				inModifiers )
{
#pragma unused( inPart, inModifiers )

	OSStatus	err = noErr;
	Point		p = { -1, -1 };
	Str255		prompt = "\pChoose a color.";
	
	// choose a new color when the button is hit
	if( GetColor( p, prompt, &fSwatchColor, &fSwatchColor ) )
	{
		err = Invalidate();
		
		SendColorChangedCommandEvent(); // messge that the color changed
	}
	
	return err;
}

// -----------------------------------------------------------------------------
//	Track
// -----------------------------------------------------------------------------
//	Default tracking method.
//	
OSStatus ColorSwatch::Track(
	TCarbonEvent&		inEvent,
	ControlPartCode*	outPart )
{
	OSStatus	err = noErr;
	RgnHandle	tempRgn = NewRgn();
	Point		qdPoint;
	
	err = GetRegion( kControlImageWellPart, tempRgn );
	require_noerr( err, CantGetRegion );
	
	// grab the point from the carbon event
	err = inEvent.GetParameter( kEventParamMouseLocation, &qdPoint );
	require_noerr( err, CantGetParameter );
	
	// test if the point was in the swatch region
	if( PtInRgn( qdPoint, tempRgn ) && err == noErr )
	{
		Hilite( kControlButtonPart );
		
		// if it was, see if a drag should be initiated
		if( WaitMouseMoved( qdPoint ) )
		{
			Hilite( kControlNoPart );
			
			PerformColorDrag( inEvent );
			
			*outPart = kControlNoPart;
			
			return noErr;
		}
	}
	
	*outPart = kControlButtonPart;
	
CantGetParameter:
CantGetRegion:

	DisposeRgn( tempRgn );
	
	return eventNotHandledErr;
}

// -----------------------------------------------------------------------------
//	SetFocusPart
// -----------------------------------------------------------------------------
//	Handle focusing.
//
OSStatus ColorSwatch::SetFocusPart(
	ControlPartCode		inDesiredFocus,
	RgnHandle			inInvalidRgn,
	Boolean				inFocusEverything,
	ControlPartCode*	outActualFocus )
{
#pragma unused( inInvalidRgn )

	OSStatus err = noErr;
	
	// resolve the desired focus
	switch( inDesiredFocus )
	{		
		case kControlFocusNextPart:
		case kControlFocusPrevPart: // toggle between focused or not
			if( (fCurrentFocusPart == kControlNoPart) && inFocusEverything )
				inDesiredFocus = kControlButtonPart;
			else
				inDesiredFocus = kControlNoPart;
			break;
		
		default:  // we don't focus anything but the button
			if( inDesiredFocus != kControlButtonPart )
				inDesiredFocus = kControlNoPart;
			break;
	}
	
	// handle invalidation if focus changed
	if( fCurrentFocusPart != inDesiredFocus )
	{
		fCurrentFocusPart = inDesiredFocus;
		
		err = Invalidate();
	}
	
	if( err == noErr )
		*outActualFocus = fCurrentFocusPart;
	
	return err;
}

// -----------------------------------------------------------------------------
//	TextInput
// -----------------------------------------------------------------------------
//	Handles text input.
//
OSStatus ColorSwatch::TextInput(
	TCarbonEvent&		inEvent )
{
	if( inEvent.GetKind() == kEventTextInputUnicodeForKeyEvent )
	{
		UniChar uch = 0;
		inEvent.GetParameter<UniChar>( kEventParamTextInputSendText,
				typeUnicodeText, &uch );
		
		// if space was hit, simulate a click in the button for keyboard nav support
		if( uch == kSpaceCharCode )
		{
			ControlPartCode	part;
			
			HIViewSimulateClick( GetViewRef(), kControlButtonPart, 0, &part );
			
			return noErr;
		}
	}
	
	return eventNotHandledErr;
}

// -----------------------------------------------------------------------------
//	PerformColorDrag
// -----------------------------------------------------------------------------
//	Create a drag with a flavor containing the color swatch color.
//
void ColorSwatch::PerformColorDrag( EventRef event )
{
	OSStatus	err = noErr;
	DragRef		dragRef;
	EventRecord	convertedEvent;
	
	ConvertEventRefToEventRecord( event, &convertedEvent );
	
	// create the drag
	err = NewDrag( &dragRef );
	require_noerr( err, CantCreateDrag );
	
	// add the color swatch flavor
	err = AddDragItemFlavor( dragRef, 1, kFlavorTypeColor, &fSwatchColor,
			sizeof( fSwatchColor ), NULL );
	require_noerr( err, CantAddFlavor );
	
	// create the drag image
	GWorldPtr world;
	
	{
		GrafPtr		save;
		Rect		bounds = { 0, 0, 10, 10 };
		RGBColor	black = { 0, 0, 0 };
		
		err = NewGWorld( &world, 16, &bounds, NULL, NULL, 0 );
		require_noerr( err, CantCreateGWorld );
		
		GetPort( &save );
		SetPort( world );
		
		RGBForeColor( &fSwatchColor );
		PaintRect( &bounds );
		
		RGBForeColor( &black );
		FrameRect( &bounds );
		
		SetPort( save );
	}
	
	// add the drag image
	{
		Point dragPoint = { convertedEvent.where.v - 5, convertedEvent.where.h - 5 };
		
		err = SetDragImage( dragRef, GetGWorldPixMap( world ), NULL, dragPoint,
				kDragOpaqueTranslucency );
		require_noerr( err, CantSetDragImage );
	}
	
	// perform the drag
	{
		RgnHandle tempRgn = NewRgn();
		
		err = TrackDrag( dragRef, &convertedEvent, tempRgn );
		
		DisposeRgn( tempRgn );
	}
	
CantSetDragImage:

	DisposeGWorld( world );
	
CantCreateGWorld:
CantAddFlavor:
CantCreateDrag:

	return;
}

// -----------------------------------------------------------------------------
//	GetColorFromDrag
// -----------------------------------------------------------------------------
//	Grab the color from the drag.  If the color flavor doesn't exist, return false.
//
bool ColorSwatch::GetColorFromDrag(
	DragRef			dragRef,
	RGBColor*		color )
{
	OSStatus		err = noErr;
	UInt16			itemCount;
	static DragRef	lastDragRef = 0;
	static bool		lastResponse;
	static RGBColor	lastColor;
	
	// return cached responses if this is the same drag
	if( dragRef == lastDragRef )
	{
		color->red = lastColor.red;
		color->green = lastColor.green;
		color->blue = lastColor.blue;
		
		return lastResponse;
	}
	
	lastDragRef = dragRef; // cache drag reference
	
	err = CountDragItems( dragRef, &itemCount );
	require_noerr( err, CantGetDragItemCount );
	
	for( UInt16 i = 1; i <= itemCount; i++ )
	{
		DragItemRef	itemRef;
		UInt16		flavorCount;
		
		err = GetDragItemReferenceNumber( dragRef, i, &itemRef );
		require_noerr( err, CantGetDragItemReferenceNumber );
		
		err = CountDragItemFlavors( dragRef, itemRef, &flavorCount );
		require_noerr( err, CantGetDragItemFlavorCount );
		
		for( UInt16 f = 1; f <= flavorCount; f++ )
		{
			FlavorType	flavor;
			Size		size;
			RGBColor	tempColor;
			
			err = GetFlavorType( dragRef, itemRef, f, &flavor );
			require_noerr( err, CantGetDragFlavor );
			
			if( flavor == kFlavorTypeColor )
			{
				err = GetFlavorDataSize( dragRef, itemRef, flavor, &size );
				require_noerr( err, CantGetDragFlavorDataSize );
				
				if( size == sizeof( tempColor ) )
				{
					err = GetFlavorData( dragRef, itemRef, flavor, &tempColor, &size, 0 );
					require_noerr( err, CantGetDragFlavorData );
					
					color->red = tempColor.red;
					color->green = tempColor.green;
					color->blue = tempColor.blue;
					
					// cache responses
					lastResponse = true;
					
					lastColor.red = tempColor.red;
					lastColor.green = tempColor.green;
					lastColor.blue = tempColor.blue;
					
					return true;
				}
			}
		}
	}

CantGetDragFlavorData:
CantGetDragFlavorDataSize:
CantGetDragFlavor:
CantGetDragItemFlavorCount:
CantGetDragItemReferenceNumber:
CantGetDragItemCount:

	lastResponse = false; // cache response

	return false;
}

// -----------------------------------------------------------------------------
//	SendColorChangedCommandEvent
// -----------------------------------------------------------------------------
//
OSStatus ColorSwatch::SendColorChangedCommandEvent()
{
	OSStatus	err = noErr;
	EventRef	commandEvent;
	HICommand	theCommand;
	err = CreateEvent( NULL, kEventClassCommand, kEventProcessCommand,
			GetCurrentEventTime(), kEventAttributeNone, &commandEvent );
	require_noerr( err, CantCreateEvent );
	
	err = GetControlCommandID( GetViewRef(), &theCommand.commandID );
	require_noerr( err, CantGetCommandID );
	
	err = SetEventParameter( commandEvent, kEventParamDirectObject,
			typeHICommand, sizeof( HICommand ), &theCommand );
	require_noerr( err, CantSetEventParameter );
	
	err = SendEventToEventTarget( commandEvent, GetApplicationEventTarget() );

CantSetEventParameter:
CantGetCommandID:
CantCreateEvent:

	return err;
}
