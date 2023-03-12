/*
	File:		ImageMapView.cp
	
	Project:	ImageMapView
	
	Abstract:	ImageMapView custom HIView subclass for displaying HTML-like Image Maps,
				using the map areas as view parts.

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

#include <Carbon/Carbon.h>

#include "TView.h"
#include "ImageMapView.h"
#include "ImageMap.h"

// -----------------------------------------------------------------------------
//	constants
// -----------------------------------------------------------------------------
//
const CFStringRef kImageMapViewClassID = CFSTR( "com.apple.hitoolbox.sample.imagemapview" );

// -----------------------------------------------------------------------------
//	TImageMapView declaration
// -----------------------------------------------------------------------------
//
class TImageMapView
	: public TView
{
public:
	static OSStatus			Create(
								HIViewRef*			outView );
	static void 			RegisterClass();

	CFStringRef				GetMapName() const
								{ return fImageMap->GetMapName(); }
	CFStringRef				GetSelectedName()
								{ return fImageMap->GetName( GetValue() - 1 ); }
	OSStatus				SetHotSpots(
								ImageMapHotSpots	inHotSpots );

protected:
	// Constructor/Destructor
							TImageMapView(
								HIViewRef			inView );
	virtual					~TImageMapView();

	virtual OSStatus		ControlHit(
								ControlPartCode		inPart,
								UInt32				inModifiers );
	virtual void			Draw(
								RgnHandle			inLimitRgn,
								CGContextRef		inContext );
	virtual ControlKind		GetKind();
	virtual OSStatus		GetRegion(
								ControlPartCode		inPart,
								RgnHandle			outRgn );
	virtual ControlPartCode	HitTest(
								const HIPoint&		inWhere );
	virtual OSStatus		Initialize(
								TCarbonEvent&		inEvent );
	virtual OSStatus		GetOptimalSizeSelf(
								HISize*				outSize,
								float*				outBaseLine );
	virtual OSStatus		HandleEvent(
								EventHandlerCallRef	inCallRef,
								TCarbonEvent&		inEvent );
	virtual OSStatus		SetFocusPart(
								ControlPartCode		inDesiredFocus,
								Boolean				inFocusEverything,
								ControlPartCode*	outActualFocus );
	virtual OSStatus		TextInput(
								TCarbonEvent&		inEvent );

	// *** These are the method declarations for overriding the accessibility
	// methods.
	virtual OSStatus		GetAccessibleAttributeNames(
								EventHandlerCallRef	inCallRef,
								EventRef			inEvent,
								AXUIElementRef		inElement,
								UInt64				inIdentifier,
								CFMutableArrayRef	outNames );
	virtual OSStatus		GetAccessibleNamedAttribute(
								EventHandlerCallRef	inCallRef,
								EventRef			inEvent,
								AXUIElementRef		inElement,
								UInt64				inIdentifier,
								CFStringRef			inAttribute );
	virtual OSStatus		IsAccessibleNamedAttributeSettable(
								EventHandlerCallRef	inCallRef,
								EventRef			inEvent,
								AXUIElementRef		inElement,
								UInt64				inIdentifier,
								CFStringRef			inName,
								Boolean*			outIsSettable );
	virtual OSStatus		SetAccessibleNamedAttribute(
								EventHandlerCallRef	inCallRef,
								EventRef			inEvent,
								AXUIElementRef		inElement,
								UInt64				inIdentifier,
								CFStringRef			inAttribute );
	virtual OSStatus		CopyAccessibleChildAtPoint(
								EventHandlerCallRef	inCallRef,
								EventRef			inEvent,
								AXUIElementRef		inElement,
								UInt64				inIdentifier,
								const HIPoint&		inWhere,
								CFTypeRef*			outChild );
	virtual OSStatus		GetAccessibleActionNames(
								EventHandlerCallRef	inCallRef,
								EventRef			inEvent,
								AXUIElementRef		inElement,
								UInt64				inIdentifier,
								CFMutableArrayRef	outNames );
	virtual OSStatus		CopyAccessibleNamedActionDescription(
								EventHandlerCallRef	inCallRef,
								EventRef			inEvent,
								AXUIElementRef		inElement,
								UInt64				inIdentifier,
								CFStringRef			inName,
								CFStringRef*		outDescription );
	virtual OSStatus		PerformAccessibleNamedAction(
								EventHandlerCallRef	inCallRef,
								EventRef			inEvent,
								AXUIElementRef		inElement,
								UInt64				inIdentifier,
								CFStringRef			inName );

private:
	static OSStatus			Construct(
								HIObjectRef			inObjectRef,
								TObject**			outObject );
	CGColorRef				GetHiliteFill();
	CGColorRef				GetRolloverFill();

	CGImageRef				GetImage() const
								{ return fImageMap->GetImage(); }
	CFIndex					GetPartCount() const
								{ return fImageMap->GetCount(); }
	CGRect					GetPartBounds(
								CFIndex				inIndex ) const
								{ return fImageMap->GetPathBounds( inIndex ); }
	CFStringRef				GetPartName(
								CFIndex				inIndex ) const
								{ return fImageMap->GetName( inIndex ); }
	CGPathRef				GetPartPath(
								CFIndex				inIndex ) const
								{ return fImageMap->GetPath( inIndex ); }

	ImageMapHotSpots		fHotSpots;
	CFIndex					fCurrentHotSpot;

	ImageMap				*fImageMap;
};

// -----------------------------------------------------------------------------
//	TImageMapView constructor
// -----------------------------------------------------------------------------
//
TImageMapView::TImageMapView(
	HIViewRef			inView )
	: TView( inView )
{
	ChangeAutoInvalidateFlags( kAutoInvalidateOnValueChange
			| kAutoInvalidateOnHilite
			| kAutoInvalidateOnActivate, 0 );

	fImageMap = NULL;
	fHotSpots = kImageMapHotSpotsInvisible;
	fCurrentHotSpot = 0;
}

// -----------------------------------------------------------------------------
//	TImageMapView destructor
// -----------------------------------------------------------------------------
//
TImageMapView::~TImageMapView()
{
	delete fImageMap;
}

// -----------------------------------------------------------------------------
//	Construct
// -----------------------------------------------------------------------------
//
OSStatus
TImageMapView::Construct(
	HIObjectRef			inObject,
	TObject**			outObject )
{
	*outObject = new TImageMapView( (HIViewRef) inObject );
	
	return noErr;
}

// -----------------------------------------------------------------------------
//	ControlHit
// -----------------------------------------------------------------------------
//
OSStatus
TImageMapView::ControlHit(
	ControlPartCode		inPart,
	UInt32				inModifiers )
{
	SetValue( inPart );
	return eventNotHandledErr;
}

// -----------------------------------------------------------------------------
//	Create
// -----------------------------------------------------------------------------
//
OSStatus
TImageMapView::Create(
	HIViewRef*			outView )
{
	OSStatus			err;
	EventRef			event = CreateInitializationEvent();
	
	// Register this class
	RegisterClass();

	// Make a new instantiation of this class
	err = ::HIObjectCreate( kImageMapViewClassID, event, (HIObjectRef*) outView );
	
	ReleaseEvent( event );

	return err;
}

// -----------------------------------------------------------------------------
//	Draw
// -----------------------------------------------------------------------------
//	Here's the fun stuff.
//
void
TImageMapView::Draw(
	RgnHandle			inLimitRgn,
	CGContextRef		inContext )
{
	TRect				bounds = Bounds();
	ControlPartCode		focusedPart;
	CFIndex				i;
	CGColorRef			fillColor;

	// This code assumes that the size of the view and the size of the image are the same.
	::HIViewDrawCGImage( inContext, bounds, GetImage() );
	::HIViewGetFocusPart( GetViewRef(), &focusedPart );

	for ( i = 0; i < GetPartCount(); i++ )
	{
		// Draw hilite, rollover and normal differently
		if ( (CFIndex) GetHilite() == i + 1 )
			fillColor = GetHiliteFill();
		else if ( fHotSpots == kImageMapHotSpotsVisible
			|| ( fHotSpots == kImageMapHotSpotsRollover && fCurrentHotSpot == i + 1 ) )
			fillColor = GetRolloverFill();
		else
			fillColor = NULL;
		
		if ( fillColor != NULL )
		{
			::CGContextSetFillColorWithColor( inContext, fillColor );
			::CGContextAddPath( inContext, GetPartPath( i ) );
			::CGContextFillPath( inContext );
		}

		// Draw the focus if the part if focused and the view is active
		if ( (CFIndex) focusedPart == i + 1 && IsActive() )
		{
			HIRect	partBounds = GetPartBounds( i );
			::HIThemeDrawFocusRect( &partBounds, true, inContext, kHIThemeOrientationNormal );
		}
	}
}

// -----------------------------------------------------------------------------
//	GetKind
// -----------------------------------------------------------------------------
//
ControlKind
TImageMapView::GetKind()
{
	const ControlKind kMyKind = { 'IMap', 'IMap' };
	
	return kMyKind;
}

// -----------------------------------------------------------------------------
//	GetHiliteFill
// -----------------------------------------------------------------------------
//
CGColorRef
TImageMapView::GetHiliteFill()
{
	static CGColorRef	sHiliteFill = NULL;
	if ( sHiliteFill == NULL )
	{
		CGColorSpaceRef	colorSpace = ::CGColorSpaceCreateDeviceRGB();
		float			hiliteComponents[] = { 1, 1, 1, 0.75 };
		sHiliteFill = ::CGColorCreate( colorSpace, hiliteComponents );
		::CFRelease( colorSpace );
	}
	return sHiliteFill;
}

// -----------------------------------------------------------------------------
//	GetRolloverFill
// -----------------------------------------------------------------------------
//
CGColorRef
TImageMapView::GetRolloverFill()
{
	static CGColorRef	sRolloverFill = NULL;
	if ( sRolloverFill == NULL )
	{
		CGColorSpaceRef	colorSpace = ::CGColorSpaceCreateDeviceRGB();
		float			rolloverComponents[] = { 1, 1, 1, 0.25 };
		sRolloverFill = ::CGColorCreate( colorSpace, rolloverComponents );
		::CFRelease( colorSpace );
	}
	return sRolloverFill;
}

// -----------------------------------------------------------------------------
//	GetRegion
// -----------------------------------------------------------------------------
//
OSStatus
TImageMapView::GetRegion(
	ControlPartCode		inPart,
	RgnHandle			outRgn )
{
	OSStatus			err = noErr;
	TRect				bounds;
	Rect				qdBounds;
	
	if ( inPart == kControlContentMetaPart
			|| inPart == kControlStructureMetaPart
			/* || inPart == kControlOpaqueRegionMetaPart */ )
	{
		bounds = Bounds();
		qdBounds = bounds;

		// Leave room for focus drawing
		::InsetRect( &qdBounds, -3, -3 );
	
		::RectRgn( outRgn, &qdBounds );
	}
	
	return err;
}

// -----------------------------------------------------------------------------
//	GetOptimalSizeSelf
// -----------------------------------------------------------------------------
//
OSStatus
TImageMapView::GetOptimalSizeSelf(
	HISize*				outSize,
	float*				outBaseLine )
{
#pragma unused( outBaseLine )
	if ( outSize )
		*outSize = CGSizeMake( ::CGImageGetWidth( GetImage() ), ::CGImageGetHeight( GetImage() ) );
	return noErr;
}

// -----------------------------------------------------------------------------
//	HandleEvent
// -----------------------------------------------------------------------------
//
OSStatus
TImageMapView::HandleEvent(
	EventHandlerCallRef	inCallRef,
	TCarbonEvent&		inEvent )
{
	#pragma unused( inCallRef )

	OSStatus			err = eventNotHandledErr;
	
	if ( inEvent.GetClass() == kEventClassControl )
	{
		HIPoint					where;
		HIViewTrackingAreaRef	trackingArea;
		HIViewTrackingAreaID	id;

		switch ( inEvent.GetKind() )
		{
			case kEventControlTrackingAreaEntered:
				inEvent.GetParameter<HIViewTrackingAreaRef>( kEventParamHIViewTrackingArea, typeHIViewTrackingAreaRef, &trackingArea );
				inEvent.GetParameter<HIPoint>( kEventParamMouseLocation, typeHIPoint, &where );
				verify_noerr( ::HIViewGetTrackingAreaID( trackingArea, &id ) );
				fCurrentHotSpot = id;
				Invalidate();
				break;

			case kEventControlTrackingAreaExited:
				inEvent.GetParameter<HIViewTrackingAreaRef>( kEventParamHIViewTrackingArea, typeHIViewTrackingAreaRef, &trackingArea );
				inEvent.GetParameter<HIPoint>( kEventParamMouseLocation, typeHIPoint, &where );
				verify_noerr( ::HIViewGetTrackingAreaID( trackingArea, &id ) );
				if ( fCurrentHotSpot == (CFIndex) id )
					fCurrentHotSpot = 0;
				Invalidate();
				break;
		}
	}
	
	// Not handled, let the parent class have a go at the event
	if ( err == eventNotHandledErr )
		err = TView::HandleEvent( inCallRef, inEvent );

	return err;
}

// -----------------------------------------------------------------------------
//	HitTest
// -----------------------------------------------------------------------------
//	Check to see if a point hits the view
//
ControlPartCode
TImageMapView::HitTest(
	const HIPoint&		inWhere )
{
	ControlPartCode		part = kControlNoPart;
	TRect				bounds( Bounds() );

	if ( bounds.Contains( inWhere ) )
	{
		HIRect				frame = Frame();
		CFIndex				i;

		for ( i = 0; part == kControlNoPart && i < GetPartCount(); i++ )
		{
			if ( ::CGPathContainsPoint( GetPartPath( i ), NULL, inWhere, true ) )
				part = i + 1;
		}
	}

	return part;
}

// -----------------------------------------------------------------------------
//	Initialize
// -----------------------------------------------------------------------------
//	The control is set up.  Do the last minute stuff that needs to be done like
//	setting up the images.
//
OSStatus
TImageMapView::Initialize(
	TCarbonEvent&		inEvent )
{
	OSStatus			err;
	CFStringRef			imageName, imageMapName;
	CFURLRef			imageUrl, mapUrl;
	const EventTypeSpec kMouseTrackingEvents[] = {
								{ kEventClassControl, kEventControlTrackingAreaEntered },
								{ kEventClassControl, kEventControlTrackingAreaExited },
							};
	CFIndex				i;

	err = TView::Initialize( inEvent );
	require_noerr( err, CantInitializeParent );

	// Extract the image and image map names
	inEvent.GetParameter( kEventParamImageMapImageName, typeCFStringRef, &imageName );
	inEvent.GetParameter( kEventParamImageMapMapName, typeCFStringRef, &imageMapName );

	// Make them into resource URLs
	imageUrl = ::CFBundleCopyResourceURL( CFBundleGetMainBundle(), imageName, NULL, NULL );
	check( imageUrl != NULL );
	mapUrl = ::CFBundleCopyResourceURL( CFBundleGetMainBundle(), imageMapName, NULL, NULL );
	check( imageUrl != NULL );
	
	// Make a new image map object with those URLs
	fImageMap = new ImageMap( imageUrl, mapUrl );
	
	::CFRelease( imageUrl );
	::CFRelease( mapUrl );
	
	// Do view-specific initialization here
	SetMaximum( GetPartCount() );
	
	// Set up the hot spot tracking areas
	for ( i = 0; i < GetPartCount(); i++ )
	{
		HIRect					pathBounds = GetPartBounds( i );
		HIShapeRef				shape;
		HIViewTrackingAreaRef	trackingArea;

		shape = ::HIShapeCreateWithRect( &pathBounds );
		verify_noerr( ::HIViewNewTrackingArea( GetViewRef(), shape, i + 1, &trackingArea ) );
		::CFRelease( shape );
	}
	
	// Start listening to mouse tracking events
	verify_noerr( AddEventTypesToHandler( GetEventHandler(), GetEventTypeCount( kMouseTrackingEvents ),
			kMouseTrackingEvents ) );
	
	ActivateInterface( kKeyboardFocus );

	// *** Let HIFramework know that we are interested in having the accessibility
	// methods called.
	ActivateInterface( kAccessibility );

CantInitializeParent:
	return err;
}

// -----------------------------------------------------------------------------
//	RegisterClass
// -----------------------------------------------------------------------------
//	Register this class with the HIObject registry.
//
//	This API can be called multiple times, but will only register once.
//
void
TImageMapView::RegisterClass()
{
	static bool sRegistered;
	
	if ( !sRegistered )
	{
		TView::RegisterSubclass( kImageMapViewClassID, Construct );
		sRegistered = true;
	}
}

// -----------------------------------------------------------------------------
//	SetHotSpots
// -----------------------------------------------------------------------------
//
OSStatus
TImageMapView::SetHotSpots(
	ImageMapHotSpots	inHotSpots )
{
	fHotSpots = inHotSpots;
	Invalidate();
	return noErr;
}

// -----------------------------------------------------------------------------
//	SetFocusPart
// -----------------------------------------------------------------------------
//
OSStatus
TImageMapView::SetFocusPart(
	ControlPartCode		inDesiredFocus,
	Boolean				inFocusEverything,
	ControlPartCode*	outActualFocus )
{
	ControlPartCode		currentFocus;
	OSStatus			err = noErr;

	HIViewGetFocusPart( GetViewRef(), &currentFocus );
	
	if ( inFocusEverything )
	{
		switch ( inDesiredFocus )
		{
			case kControlFocusNextPart:
				if ( currentFocus == kControlNoPart )
					inDesiredFocus = 1;
				else if ( (CFIndex) currentFocus == GetPartCount() )
					inDesiredFocus = kControlNoPart;
				else	
					inDesiredFocus = currentFocus + 1;
				break;

			case kControlFocusPrevPart:
				if ( currentFocus == kControlNoPart )
					inDesiredFocus = GetPartCount();
				else if ( currentFocus == 1 )
					inDesiredFocus = kControlNoPart;
				else
					inDesiredFocus = currentFocus - 1;
				break;
			
			// default:
				// Focus part is being set manually
		}

		// if the focus changes, make sure we redraw
		if ( currentFocus != inDesiredFocus )
			Invalidate();

		*outActualFocus = inDesiredFocus;
	}
	
	return err;
}

// -----------------------------------------------------------------------------
//	TextInput
// -----------------------------------------------------------------------------
//
OSStatus
TImageMapView::TextInput(
	TCarbonEvent&		inEvent )
{
	OSStatus	err = eventNotHandledErr;
	
	if ( inEvent.GetKind() == kEventTextInputUnicodeForKeyEvent )
	{
		UniChar				uch = 0;

		inEvent.GetParameter<UniChar>( kEventParamTextInputSendText, typeUnicodeText, &uch );

		if ( uch ==  kSpaceCharCode )
		{
			ControlPartCode		currentFocus;
			::HIViewGetFocusPart( GetViewRef(), &currentFocus );
			::HIViewSimulateClick( GetViewRef(), currentFocus, 0, NULL );
			err = noErr;
		}
	}
	
	return err;
}

// *** The implementations of the overridden accessibility methods.
// (please note that there is limited error checking in the code to simplify it)

// -----------------------------------------------------------------------------
//	GetAccessibleAttributeNames
// -----------------------------------------------------------------------------
//	Read about kEventAccessibleGetAllAttributeNames in CarbonEvents.h to
//	understand what the appended values mean for this implementation.
//
OSStatus
TImageMapView::GetAccessibleAttributeNames(
	EventHandlerCallRef	inCallRef,
	EventRef			inEvent,
	AXUIElementRef		inElement,
	UInt64				inIdentifier,
	CFMutableArrayRef	outNames )
{
	OSStatus			err = eventNotHandledErr;

	if ( inIdentifier == 0 )
	{
		// Identifier 0 means "the whole view".

		CallNextEventHandler( inCallRef, inEvent );

		// Let accessibility know that this view has children and can
		// return a list of them.
		CFArrayAppendValue( outNames, kAXChildrenAttribute );
		err = noErr;
	}
	else if ( (CFIndex) inIdentifier <= GetPartCount() )
	{
		// Identifier > 0 means "sub elements of the view" -- in this case, parts.

		CallNextEventHandler( inCallRef, inEvent );

		// Let accessibility know that this view's children can return description,
		// size, position, parent window, top level element and isFocused attributes.
		CFArrayAppendValue( outNames, kAXDescriptionAttribute );
		CFArrayAppendValue( outNames, kAXSizeAttribute );
		CFArrayAppendValue( outNames, kAXPositionAttribute );
		CFArrayAppendValue( outNames, kAXWindowAttribute );
		CFArrayAppendValue( outNames, kAXTopLevelUIElementAttribute );
		CFArrayAppendValue( outNames, kAXFocusedAttribute );
		err = noErr;
	}
	
	return err;
}

// -----------------------------------------------------------------------------
//	GetAccessibleNamedAttribute
// -----------------------------------------------------------------------------
//	Read about kEventAccessibleGetNamedAttribute in CarbonEvents.h to
//	understand what the output event parameters mean for this implementation.
//
OSStatus
TImageMapView::GetAccessibleNamedAttribute(
	EventHandlerCallRef	inCallRef,
	EventRef			inEvent,
	AXUIElementRef		inElement,
	UInt64				inIdentifier,
	CFStringRef			inAttribute )
{
	OSStatus			err = eventNotHandledErr;

	if ( inIdentifier == 0 )
	{
		// String compare the incoming attribute name and return the appropriate accessibility
		// information as an event parameter.
		
		if ( CFStringCompare( inAttribute, kAXChildrenAttribute, 0 ) == kCFCompareEqualTo )
		{
			// Create and return an array of AXUIElements describing the children of this view.
			CFMutableArrayRef	children;
			AXUIElementRef		child;
			CFIndex				i;

			children = CFArrayCreateMutable( kCFAllocatorDefault, GetPartCount(), &kCFTypeArrayCallBacks );
			for ( i = 0; i < GetPartCount(); i++ )
			{
				child = AXUIElementCreateWithHIObjectAndIdentifier( GetObjectRef(), i + 1 );

				CFArrayAppendValue( children, child );
				CFRelease( child );
			}
			SetEventParameter( inEvent, kEventParamAccessibleAttributeValue, typeCFTypeRef, sizeof( children ), &children );
			CFRelease( children );

			err = noErr;
		}
		else if ( CFStringCompare( inAttribute, kAXRoleAttribute, 0 ) == kCFCompareEqualTo )
		{
			// Return a string indicating the role of this view. Use the system group role.
			CFStringRef		role = kAXGroupRole;
			SetEventParameter( inEvent, kEventParamAccessibleAttributeValue, typeCFTypeRef, sizeof( role ), &role );
			err = noErr;
		}
		else if ( CFStringCompare( inAttribute, kAXRoleDescriptionAttribute, 0 ) == kCFCompareEqualTo )
		{
			// Return a string describing the role of this view. Use the system group role description.
			CFStringRef		roleDesc = HICopyAccessibilityRoleDescription( kAXGroupRole, NULL );
			SetEventParameter( inEvent, kEventParamAccessibleAttributeValue, typeCFTypeRef, sizeof( roleDesc ), &roleDesc );
			CFRelease( roleDesc );
			err = noErr;
		}
	}
	else if ( (CFIndex) inIdentifier <= GetPartCount() )
	{
		if ( CFStringCompare( inAttribute, kAXDescriptionAttribute, 0 ) == kCFCompareEqualTo )
		{
			// Return a string describing this part of the view. Use the part name retrieved from the image map.
			CFStringRef		description = GetPartName( inIdentifier - 1 );
			SetEventParameter( inEvent, kEventParamAccessibleAttributeValue, typeCFTypeRef, sizeof( description ), &description );
			err = noErr;
		}
		else if ( CFStringCompare( inAttribute, kAXParentAttribute, 0 ) == kCFCompareEqualTo )
		{
			// Return the parent of this part. It's always the view.
			AXUIElementRef		parent;
			parent = AXUIElementCreateWithHIObjectAndIdentifier( GetObjectRef(), 0 );
			SetEventParameter( inEvent, kEventParamAccessibleAttributeValue, typeCFTypeRef, sizeof( parent ), &parent );
			CFRelease( parent );
			err = noErr;
		}
		else if ( CFStringCompare( inAttribute, kAXRoleAttribute, 0 ) == kCFCompareEqualTo )
		{
			// Return a string indicating the role of this part. The parts of the view behave like
			// buttons, so use that system role.
			CFStringRef		role = kAXButtonRole;
			SetEventParameter( inEvent, kEventParamAccessibleAttributeValue, typeCFTypeRef, sizeof( role ), &role );
			err = noErr;
		}
		else if ( CFStringCompare( inAttribute, kAXRoleDescriptionAttribute, 0 ) == kCFCompareEqualTo )
		{
			// Return a string describing the role of this part. Use the system description.
			CFStringRef		roleDesc = HICopyAccessibilityRoleDescription( kAXButtonRole, NULL );
			SetEventParameter( inEvent, kEventParamAccessibleAttributeValue, typeCFTypeRef, sizeof( roleDesc ), &roleDesc );
			CFRelease( roleDesc );
			err = noErr;
		}
		else if ( CFStringCompare( inAttribute, kAXSizeAttribute, 0 ) == kCFCompareEqualTo )
		{
			// Return the size of this part as an HISize.
			HIRect			partBounds = GetPartBounds( inIdentifier - 1 );
			SetEventParameter( inEvent, kEventParamAccessibleAttributeValue, typeHISize, sizeof( partBounds.size ), &partBounds.size );
			err = noErr;
		}
		else if ( CFStringCompare( inAttribute, kAXPositionAttribute, 0 ) == kCFCompareEqualTo )
		{
			// Return the position of this part as an HIPoint.
			HIRect			partBounds = GetPartBounds( inIdentifier - 1 );
			HIPointConvert( &partBounds.origin, kHICoordSpaceView, GetViewRef(), kHICoordSpace72DPIGlobal, NULL );
			SetEventParameter( inEvent, kEventParamAccessibleAttributeValue, typeHIPoint, sizeof( partBounds.origin ), &partBounds.origin );
			err = noErr;
		}
		else if ( CFStringCompare( inAttribute, kAXWindowAttribute, 0 ) == kCFCompareEqualTo
				|| CFStringCompare( inAttribute, kAXTopLevelUIElementAttribute, 0 ) == kCFCompareEqualTo )
		{
			// Return the window or top level ui element for this part. They are both the same so re-use the code.
			AXUIElementRef		windOrTopUI;
			windOrTopUI = AXUIElementCreateWithHIObjectAndIdentifier( (HIObjectRef) GetOwner(), 0 );
			SetEventParameter( inEvent, kEventParamAccessibleAttributeValue, typeCFTypeRef, sizeof( windOrTopUI ), &windOrTopUI );
			CFRelease( windOrTopUI );
			err = noErr;
		}
		else if ( CFStringCompare( inAttribute, kAXFocusedAttribute, 0 ) == kCFCompareEqualTo )
		{
			// Return whether or not this part is focused.
			ControlPartCode		focusedPart;
			Boolean				focused;
			HIViewGetFocusPart( GetViewRef(), &focusedPart );
			focused = ( focusedPart == (ControlPartCode) inIdentifier );
			SetEventParameter( inEvent, kEventParamAccessibleAttributeValue, typeBoolean, sizeof( focused ), &focused );
			err = noErr;
		}
	}
	
	return err;
}

// -----------------------------------------------------------------------------
//	IsAccessibleNamedAttributeSettable
// -----------------------------------------------------------------------------
//	Read about kEventAccessibleIsNamedAttributeSettable in CarbonEvents.h to
//	understand what return values mean for this implementation.
//
OSStatus
TImageMapView::IsAccessibleNamedAttributeSettable(
	EventHandlerCallRef	inCallRef,
	EventRef			inEvent,
	AXUIElementRef		inElement,
	UInt64				inIdentifier,
	CFStringRef			inName,
	Boolean*			outIsSettable )
{
	OSStatus			err = eventNotHandledErr;

	// The focused attribute is the only settable attribute for this view,
	// and it can only be set on part (or subelements), not the whole view.
	if ( inIdentifier > 0 && (CFIndex) inIdentifier <= GetPartCount() )
	{
		if ( CFStringCompare( inName, kAXFocusedAttribute, 0 ) == kCFCompareEqualTo )
		{
			*outIsSettable = true;
			err = noErr;
		}
	}

	return err;
}

// -----------------------------------------------------------------------------
//	SetAccessibleNamedAttribute
// -----------------------------------------------------------------------------
//	Read about kEventAccessibleSetNamedAttribute in CarbonEvents.h to understand
//  what this implementation should do.
//
OSStatus
TImageMapView::SetAccessibleNamedAttribute(
	EventHandlerCallRef	inCallRef,
	EventRef			inEvent,
	AXUIElementRef		inElement,
	UInt64				inIdentifier,
	CFStringRef			inAttribute )
{
	OSStatus			err = eventNotHandledErr;

	if ( inIdentifier > 0 && (CFIndex) inIdentifier <= GetPartCount() )
	{
		if ( CFStringCompare( inAttribute, kAXFocusedAttribute, 0 ) == kCFCompareEqualTo )
		{
			Boolean		focused;
			GetEventParameter( inEvent, kEventParamAccessibleAttributeValue, typeBoolean, NULL, sizeof( focused ), NULL, &focused );
			// Don't allow unfocusing
			if ( focused == true )
			{
				SetKeyboardFocus( GetWindowRef(), GetViewRef(),(ControlPartCode) inIdentifier );
				err = noErr;
			}
			else
			{
				err = kAXErrorIllegalArgument;
			}
		}
	}
	
	return err;
}

// -----------------------------------------------------------------------------
//	CopyAccessibleChildAtPoint
// -----------------------------------------------------------------------------
//	Read about kEventAccessibleGetChildAtPoint in CarbonEvents.h to
//	understand what gets set to outChild in this implementation.
//
OSStatus
TImageMapView::CopyAccessibleChildAtPoint(
	EventHandlerCallRef	inCallRef,
	EventRef			inEvent,
	AXUIElementRef		inElement,
	UInt64				inIdentifier,
	const HIPoint&		inWhere,
	CFTypeRef*			outChild )
{
	OSStatus			err = eventNotHandledErr;
	
	// Only the whole view can be tested since the parts don't have sub-parts.
	if ( inIdentifier == 0 )
	{
		// This is like hit testing for accessibility. So much so, in fact, that
		// it is a wrapper around this view's HitTest method.

		ControlPartCode		part;
		HIPoint				localPoint = inWhere;
		HIPointConvert( &localPoint, kHICoordSpace72DPIGlobal, NULL, kHICoordSpaceView, GetViewRef() );
		part = HitTest( localPoint );

		if ( part != kControlNoPart )
		{
			*outChild = AXUIElementCreateWithHIObjectAndIdentifier( GetObjectRef(), part );
			err = noErr;
		}
	}

	return err;
}

// -----------------------------------------------------------------------------
//	GetAccessibleActionNames
// -----------------------------------------------------------------------------
//	Read about kEventAccessibleGetAllActionNames in CarbonEvents.h to
//	understand what the appended values mean for this implementation.
//
OSStatus
TImageMapView::GetAccessibleActionNames(
	EventHandlerCallRef	inCallRef,
	EventRef			inEvent,
	AXUIElementRef		inElement,
	UInt64				inIdentifier,
	CFMutableArrayRef	outNames )
{
	OSStatus			err = eventNotHandledErr;

	// Only the parts have actions.
	if ( inIdentifier > 0 && (CFIndex) inIdentifier <= GetPartCount() )
	{
		// Let accessibility know that the parts can be pressed like buttons.
		CFArrayAppendValue( outNames, kAXPressAction );
		err = noErr;
	}

	return err;
}

// -----------------------------------------------------------------------------
//	CopyAccessibleNamedActionDescription
// -----------------------------------------------------------------------------
//	Read about kEventAccessibleGetNamedActionDescription in CarbonEvents.h to
//	understand what the returned values mean for this implementation.
//
OSStatus
TImageMapView::CopyAccessibleNamedActionDescription(
	EventHandlerCallRef	inCallRef,
	EventRef			inEvent,
	AXUIElementRef		inElement,
	UInt64				inIdentifier,
	CFStringRef			inName,
	CFStringRef*		outDescription )
{
	OSStatus			err = eventNotHandledErr;

	// Only the parts have actions.
	if ( inIdentifier > 0 && (CFIndex) inIdentifier <= GetPartCount() )
	{
		// Return the system description for the action.
		*outDescription = HICopyAccessibilityActionDescription( inName );
		err = noErr;
	}

	return err;
}

// -----------------------------------------------------------------------------
//	PerformAccessibleNamedAction
// -----------------------------------------------------------------------------
//	Read about kEventAccessiblePerformNamedAction in CarbonEvents.h to
//	understand what should occur in this implementation.
//
OSStatus
TImageMapView::PerformAccessibleNamedAction(
	EventHandlerCallRef	inCallRef,
	EventRef			inEvent,
	AXUIElementRef		inElement,
	UInt64				inIdentifier,
	CFStringRef			inName )
{
	OSStatus			err = eventNotHandledErr;

	// Only the parts have actions.
	if ( inIdentifier > 0 && (CFIndex) inIdentifier <= GetPartCount() )
	{
		// Pressing a part is like clicking it. Let's pretend that is happening.
		HIViewSimulateClick( GetViewRef(), inIdentifier, 0, NULL );
		err = noErr;
	}

	return err;
}

// =============================================================================
//	Public APIs
// =============================================================================

// -----------------------------------------------------------------------------
//	ImageMapViewCreate
// -----------------------------------------------------------------------------
//	Public API
//
OSStatus
ImageMapViewCreate(
	HIViewRef*			outView )
{
	OSStatus			err;
	
	err = TImageMapView::Create( outView );
	require_noerr( err, CantCreateView );

CantCreateView:
	return err;
}

// -----------------------------------------------------------------------------
//	ImageMapViewRegister
// -----------------------------------------------------------------------------
//	Public API
//
void
ImageMapViewRegister()
{
	TImageMapView::RegisterClass();
}

// -----------------------------------------------------------------------------
//	ImageMapViewSetHotSpots
// -----------------------------------------------------------------------------
//	Public API
//
OSStatus
ImageMapViewSetHotSpots(
	HIViewRef				inView,
	ImageMapHotSpots		inHotSpots )
{
	OSStatus		err;
	TImageMapView*	view;
	
	view = (TImageMapView*) HIObjectDynamicCast( (HIObjectRef) inView, kImageMapViewClassID );

	require_action( view != NULL, BadParam, err = paramErr );
	
	err = view->SetHotSpots( inHotSpots );

BadParam:
	return err;
}

// -----------------------------------------------------------------------------
//	ImageMapViewGetSelectedName
// -----------------------------------------------------------------------------
//	Public API
//
CFStringRef
ImageMapViewGetMapName(
	HIViewRef				inView )
{
	CFStringRef		outName;
	TImageMapView*	view;
	
	view = (TImageMapView*) HIObjectDynamicCast( (HIObjectRef) inView, kImageMapViewClassID );

	require_action( view != NULL, BadParam, outName = NULL );
	
	outName = view->GetMapName();

BadParam:
	return outName;
}

// -----------------------------------------------------------------------------
//	ImageMapViewGetSelectedName
// -----------------------------------------------------------------------------
//	Public API
//
CFStringRef
ImageMapViewGetSelectedName(
	HIViewRef				inView )
{
	CFStringRef		outName;
	TImageMapView*	view;
	
	view = (TImageMapView*) HIObjectDynamicCast( (HIObjectRef) inView, kImageMapViewClassID );

	require_action( view != NULL, BadParam, outName = NULL );
	
	outName = view->GetSelectedName();

BadParam:
	return outName;
}
