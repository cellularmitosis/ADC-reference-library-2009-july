/*
    File:		TSegmentView.cp
    
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

#include "TSegmentView.h"

// -----------------------------------------------------------------------------
//	constants
// -----------------------------------------------------------------------------
//
#define kTViewClassID CFSTR( "com.apple.sample.TSegmentView" )

const UInt32	kDefaultSegmentCount	= 3;
const UInt32	kSegmentHeight			= 26;
const UInt32	kLeftCapWidth			= 10;
const UInt32	kRightCapWidth			= kLeftCapWidth;
const UInt32	kLeftFillWidth			= 17;
const UInt32	kRightFillWidth			= 17;
const UInt32	kFillWidth				= 21;
const UInt32	kSeparatorWidth			= 1;
const UInt32	kContentWidth			= 17;
const UInt32	kContentHeight			= kContentWidth;
const UInt32	kContentTopOffset		= 4;

enum
{
	kImageSegmentFill = 0,
	kImageSegmentFillDisabled,
	kImageSegmentFillOn,
	kImageSegmentFillOnDisabled,
	kImageSegmentFillPressed,
	kImageSegmentFillSelected,
	kImageSegmentLeft,
	kImageSegmentLeftDisabled,
	kImageSegmentLeftOn,
	kImageSegmentLeftOnDisabled,
	kImageSegmentLeftPressed,
	kImageSegmentLeftSelected,
	kImageSegmentRight,
	kImageSegmentRightDisabled,
	kImageSegmentRightOn,
	kImageSegmentRightOnDisabled,
	kImageSegmentRightPressed,
	kImageSegmentRightSelected,
	kImageSegmentSeparator,
	kImageSegmentSeparatorDisabled,
	kImageSegmentSeparatorOn,
	kImageSegmentSeparatorOnDisabled,
	kImageSegmentSeparatorPressed,
	kImageSegmentSeparatorSelected,

	// This has to always be last!
	kImageCount,
	
	kImageNormalOffset = 0,
	kImageDisabledOffset,
	kImageOnOffset,
	kImageOnDisabledOffset,
	kImagePressedOffset,
	kImageSelectedOffset,
	
};


// -----------------------------------------------------------------------------
//	statics
// -----------------------------------------------------------------------------
//
CGImageRef*	TSegmentView::fStructureImages = NULL;
UInt32		TSegmentView::fStructureImageClientRefCount = 0;

// -----------------------------------------------------------------------------
//	TSegmentView constructor
// -----------------------------------------------------------------------------
//
TSegmentView::TSegmentView(
	HIViewRef			inControl )
	:	TView( inControl ),
		fIconCount( 0 ),
		fIcons( NULL )
{
	ChangeAutoInvalidateFlags( kAutoInvalidateOnActivate
			| kAutoInvalidateOnEnable
			| kAutoInvalidateOnHilite, 0 );
}

// -----------------------------------------------------------------------------
//	TSegmentView destructor
// -----------------------------------------------------------------------------
//
TSegmentView::~TSegmentView()
{
	int				i;
	
	// Without this instance, there is one less image client
	fStructureImageClientRefCount--;
	
	// If there are no image clients, the images can be released
	if ( fStructureImageClientRefCount == 0 && fStructureImages != NULL )
	{
		for ( i = 0; i < kImageCount; i++ )
			CGImageRelease( fStructureImages[ i ] );
		
		delete fStructureImages;

		// Reset the static fStructureImages ptr so it can be reinitialized if neccessary
		fStructureImages = NULL;
	}
}

// -----------------------------------------------------------------------------
//	GetKind
// -----------------------------------------------------------------------------
//
ControlKind TSegmentView::GetKind()
{
	const ControlKind kMyKind = { 'TtsV', 'TtsV' };
	
	return kMyKind;
}

//-----------------------------------------------------------------------------------
//	Create
//-----------------------------------------------------------------------------------
//
OSStatus TSegmentView::Create(
	HIViewRef*			outControl,
	const HIRect*		inBounds,
	WindowRef			inWindow )
{
	OSStatus			err;
	EventRef			event = CreateInitializationEvent();
	ControlRef			root;
	
	// Register this class
	RegisterClass();

	// Make a new instantiation of this class
	err = HIObjectCreate( kTViewClassID, event, (HIObjectRef*) outControl );
	
	ReleaseEvent( event );

	if ( err == noErr )
	{
		if ( inWindow != NULL )
		{
			GetRootControl( inWindow, &root );
			HIViewAddSubview( root, *outControl );
		}

		HIViewSetFrame( *outControl, inBounds );
	}
	
	return err;
}

//-----------------------------------------------------------------------------------
//	RegisterClass
//-----------------------------------------------------------------------------------
//	Register this class with the HIObject registry.
//
//	This API can be called multiple times, but will only register once.
//
void TSegmentView::RegisterClass()
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
OSStatus TSegmentView::Construct(
	ControlRef		inControl,
	TView**			outView )
{
	*outView = new TSegmentView( inControl );
	
	SetControlMaximum( inControl, kDefaultSegmentCount );
	SetControlValue( inControl, 1 );
	
	return noErr;
}

//-----------------------------------------------------------------------------------
//	Initialize
//-----------------------------------------------------------------------------------
//	The control is set up.  Do the last minute stuff that needs to be done like
//	setting up the images.
//
OSStatus TSegmentView::Initialize(
	TCarbonEvent&		inEvent )
{
	OSStatus			err;
	
	err = TView::Initialize( inEvent );
	require_noerr( err, CantInitializeParent );

	// Load the images if they aren't already
	if ( fStructureImages == NULL )
	{
		CFStringRef			imageNames[ kImageCount ] = {
								CFSTR( "SegmentFillN.png" ),
								CFSTR( "SegmentFillD.png" ),
								CFSTR( "SegmentFillO.png" ),
								CFSTR( "SegmentFillOD.png" ),
								CFSTR( "SegmentFillP.png" ),
								CFSTR( "SegmentFillS.png" ),
								CFSTR( "SegmentLeftN.png" ),
								CFSTR( "SegmentLeftD.png" ),
								CFSTR( "SegmentLeftO.png" ),
								CFSTR( "SegmentLeftOD.png" ),
								CFSTR( "SegmentLeftP.png" ),
								CFSTR( "SegmentLeftS.png" ),
								CFSTR( "SegmentRightN.png" ),
								CFSTR( "SegmentRightD.png" ),
								CFSTR( "SegmentRightO.png" ),
								CFSTR( "SegmentRightOD.png" ),
								CFSTR( "SegmentRightP.png" ),
								CFSTR( "SegmentRightS.png" ),
								CFSTR( "SegmentSeparatorN.png" ),
								CFSTR( "SegmentSeparatorD.png" ),
								CFSTR( "SegmentSeparatorO.png" ),
								CFSTR( "SegmentSeparatorOD.png" ),
								CFSTR( "SegmentSeparatorP.png" ),
								CFSTR( "SegmentSeparatorS.png" ) };

		CFURLRef			url;
		CGDataProviderRef	provider;
		int					i;

		fStructureImages = new CGImageRef[ kImageCount ];
		require_action( fStructureImages != NULL, CantMakeImageArray, err = memFullErr );
		
		// Load up the art work
		for ( i = 0; i < kImageCount; i++ )
		{
			url = CFBundleCopyResourceURL( CFBundleGetMainBundle(), imageNames[ i ], NULL, NULL );
			require_action( url != NULL, CantGetURL, err = paramErr );
			
			provider = CGDataProviderCreateWithURL( url );
			
			fStructureImages[ i ] = CGImageCreateWithPNGDataProvider( provider, NULL, false, kCGRenderingIntentDefault );
			require_action( fStructureImages[ i ] != NULL, CantMakeImage, err = memFullErr );
			
			CGDataProviderRelease( provider );
		
			CFRelease( url );
		}
	}
	
	// Add 1 to the client count for these images
	fStructureImageClientRefCount++;
	
CantMakeImage:
CantMakeImageArray:
CantGetURL:
CantInitializeParent:
	return err;
}

//-----------------------------------------------------------------------------------
//	Draw
//-----------------------------------------------------------------------------------
//	Here's the fun stuff.  Draw a green box, unless highlit, then draw a purple box.
//
void TSegmentView::Draw(
	RgnHandle			inLimitRgn,
	CGContextRef		inContext )
{
#pragma unused( inLimitRgn )
	SInt16				i;
	TRect				rect( Bounds() );
	CGImageRef			image;
	TRect				contentRect;
	
	rect.SetHeight( kSegmentHeight );

	// Draw the left segment
	image = FetchStructureImage( 1, kImageSegmentLeft );
	rect.SetWidth( kLeftCapWidth );
	HIViewDrawCGImage( inContext, rect, image );
	rect.MoveBy( rect.Width(), 0 );
	image = FetchStructureImage( 1, kImageSegmentFill );
	rect.SetWidth( kLeftFillWidth );
	HIViewDrawCGImage( inContext, rect, image );

	// Draw the left content
	if ( fIconCount > 0 )
	{
		contentRect.Set( rect.MaxX() - 2 - kContentWidth,
				rect.MinY() + kContentTopOffset, kContentWidth, kContentHeight );
		HIViewDrawCGImage( inContext, contentRect, fIcons[ 0 ] );
	}
	
	rect.MoveBy( rect.Width(), 0 );

	// Draw the middle segments
	for ( i = 2; i < GetMaximum(); i++ )
	{
		// Precede with separator
		rect.SetWidth( kSeparatorWidth );
		image = FetchStructureImage( i, kImageSegmentSeparator );
		HIViewDrawCGImage( inContext, rect, image );
		rect.MoveBy( rect.Width(), 0 );

		// Fill
		image = FetchStructureImage( i, kImageSegmentFill );
		rect.SetWidth( kFillWidth );
		HIViewDrawCGImage( inContext, rect, image );
		
		// Draw the content
		if ( i-1 < fIconCount )
		{
			contentRect.Set( rect.MinX() + 2,
					rect.MinY() + kContentTopOffset, kContentWidth, kContentHeight );
			HIViewDrawCGImage( inContext, contentRect, fIcons[ i-1 ] );
		}

		rect.MoveBy( rect.Width(), 0 );
	}

	// Precede with separator
	rect.SetWidth( kSeparatorWidth );
	image = FetchStructureImage( GetMaximum(), kImageSegmentSeparator );
	HIViewDrawCGImage( inContext, rect, image );
	rect.MoveBy( rect.Width(), 0 );

	// Draw the last segment
	rect.SetWidth( kRightFillWidth );
	image = FetchStructureImage( GetMaximum(), kImageSegmentFill );
	HIViewDrawCGImage( inContext, rect, image );
	contentRect.Set( rect.MinX() + 2,
			rect.MinY() + kContentTopOffset, kContentWidth, kContentHeight );
	rect.MoveBy( rect.Width(), 0 );
	image = FetchStructureImage( GetMaximum(), kImageSegmentRight );
	rect.SetWidth( kRightCapWidth );
	HIViewDrawCGImage( inContext, rect, image );

	// Draw the content
	if ( GetMaximum()-1 < fIconCount )
		HIViewDrawCGImage( inContext, contentRect, fIcons[ i-1 ] );
}

//-----------------------------------------------------------------------------------
//	HitTest
//-----------------------------------------------------------------------------------
//	Check to see if a point hits the view
//
ControlPartCode TSegmentView::HitTest(
	const HIPoint&		inWhere )
{
	ControlPartCode		part = kControlNoPart;
	TRect				bounds( Bounds() );
	TRect				segmentRect( bounds );
	SInt16				i;
	
	// Is the mouse in the view?
	if ( bounds.Contains( inWhere ) )
	{
		segmentRect.SetWidth( kLeftCapWidth + kLeftFillWidth );
		segmentRect.SetHeight( kSegmentHeight );
		if ( segmentRect.Contains( inWhere ) )
			part = 1;
		segmentRect.MoveBy( segmentRect.Width() + kSeparatorWidth, 0 );

		segmentRect.SetWidth( kFillWidth );
		for ( i = 2; i < GetMaximum() && part == kControlNoPart; i++ )
		{
			if ( segmentRect.Contains( inWhere ) )
				part = i;
			else
				segmentRect.MoveBy( segmentRect.Width() + kSeparatorWidth, 0 );
		}
		
		if ( part == kControlNoPart )
		{
			segmentRect.SetWidth( kRightFillWidth + kRightCapWidth );
			if ( segmentRect.Contains( inWhere ) )
				part = GetMaximum();
		}
	}
	
	return part;
}

//-----------------------------------------------------------------------------------
//	HiliteChanged
//-----------------------------------------------------------------------------------
//	Handler for hilite changed events
//
OSStatus TSegmentView::HiliteChanged(
	ControlPartCode		inOriginalPart,
	ControlPartCode		inCurrentPart,
	RgnHandle			inInvalRgn )
{
#pragma unused( inOriginalPart, inCurrentPart, inInvalRgn )
	// Due to the change, the view needs to redraw
	return HIViewSetNeedsDisplay( GetViewRef(), true );
}

//-----------------------------------------------------------------------------------
//	BoundsChanged
//-----------------------------------------------------------------------------------
//	Handler for bounds changed events
//
OSStatus TSegmentView::BoundsChanged(
	UInt32 				inOptions,
	const HIRect& 		inOriginalBounds,
	const HIRect& 		inCurrentBounds,
	RgnHandle 			inInvalRgn )
{
#pragma unused( inOptions, inOriginalBounds, inCurrentBounds, inInvalRgn )
	// Due to the change, the view needs to redraw
	return HIViewSetNeedsDisplay( GetViewRef(), true );
}

//-----------------------------------------------------------------------------------
//	GetRegion
//-----------------------------------------------------------------------------------
//
OSStatus TSegmentView::GetRegion(
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
	
		RectRgn( outRgn, &qdBounds );
	}
	
	return err;
}

//-----------------------------------------------------------------------------------
//	ControlHit
//-----------------------------------------------------------------------------------
//
OSStatus TSegmentView::ControlHit(
	ControlPartCode		inPart,
	UInt32				inModifiers )
{
#pragma unused( inModifiers )
	SetValue( inPart );
	
	return noErr;
}

//-----------------------------------------------------------------------------------
//	FetchStructureImage
//-----------------------------------------------------------------------------------
//
CGImageRef TSegmentView::FetchStructureImage(
	ControlPartCode		inPart,
	UInt32				inBaseImageID )
{
	UInt32				offset;
	
	if ( !IsEnabled() )
	{
		if ( GetValue() == inPart )
			offset = kImageOnDisabledOffset;
		else
			offset = kImageDisabledOffset;
	}
	else if ( !IsActive() )
	{
		if ( GetValue() == inPart )
			offset = kImageOnOffset;
		else
			offset = kImageNormalOffset;
	}
	else
	{
		if ( GetHilite() == inPart )
			offset = kImagePressedOffset;
		else if ( GetValue() == inPart )
			offset = kImageOnOffset;
		else
			offset = kImageNormalOffset;
	}
	
	
	return fStructureImages[ inBaseImageID + offset ];
}

//-----------------------------------------------------------------------------------
//	ReleaseIcons
//-----------------------------------------------------------------------------------
//
void TSegmentView::ReleaseIcons()
{
	if ( fIconCount > 0 && fIcons != NULL )
	{
		UInt8		i;
		for ( i = 0; i < fIconCount; i++ )
			CGImageRelease( fIcons[ i ] );
		
		delete fIcons;
		fIcons = NULL;
		
		fIconCount = 0;
	}
}

//-----------------------------------------------------------------------------------
//	SetData
//-----------------------------------------------------------------------------------
//
OSStatus TSegmentView::SetData(
	OSType				inTag,
	ControlPartCode		inPart,
	Size				inSize,
	const void*			inPtr )
{
	OSStatus			err = noErr;
	
	switch( inTag )
	{
		case kControlSegmentViewIconsTag:
			if ( inPtr )
			{
				if ( inSize != sizeof( SegmentIconData ) )
				{
					err = errDataSizeMismatch;
				}
				else
				{
					UInt8			i;
					
					ReleaseIcons();
					
					fIcons = new CGImageRef[ ( (SegmentIconData*) inPtr )->count ];
					require_action( fIcons != NULL, CantAllocateIconArray, err = memFullErr );
					fIconCount = ( (SegmentIconData*) inPtr )->count;

					for ( i = 0; i < fIconCount; i++ )
					{
						fIcons[ i ] = ( (SegmentIconData*) inPtr )->icons[ i ];
						CGImageRetain( fIcons[ i ] );
					}
				}
			}
			break;

		default:
			err = TView::SetData( inTag, inPart, inSize, inPtr );
			break;
	}
	
CantAllocateIconArray:
	return err;
}

