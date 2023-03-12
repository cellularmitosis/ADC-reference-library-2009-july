/*
    File:		TImageBrowserView.cp
    
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


#include "TImageBrowserView.h"
#include "ImageBrowserView.h"

//------------------------------------------------------------------------------
//	constants
//------------------------------------------------------------------------------
//
const float kImageMargin	= 2;
const float kButtonSize		= 24;
const float kButtonMargin	= 32;
const float kButtonPad		= 4;

enum // view parts
{
	kImagePart			= 1,
	kBrowseBackPart,
	kBrowseForwardPart,
	kDeleteImagePart
};

#define kImageURLsArchiveKey	CFSTR("ImageURLsKey")
#define kImageIndexArchiveKey	CFSTR("ImageIndexKey")

#pragma mark -
#pragma mark Client API

//-----------------------------------------------------------------------------------
//	RegisterClass
//-----------------------------------------------------------------------------------
//
OSStatus
TImageBrowserView::RegisterClass()
{
	static bool sRegistered;
	
	if ( !sRegistered )
	{
		TView::RegisterSubclass( kImageBrowserViewClassID, Construct );
		sRegistered = true;
	}
	
	return noErr;
}

//------------------------------------------------------------------------------
//	ImageBrowserViewCreate
//------------------------------------------------------------------------------
// creates a TImageBrowserView instance
//
OSStatus TImageBrowserView::Create(
	WindowRef			inWindow,
	const HIRect*		inBounds,
	CFArrayRef			inImageURLs,
	HIViewRef*			outView )
{
	OSStatus			err;
	HIViewRef			content;
	EventRef			event;
	
	// make sure the ImageBrowserView class is registered
	err = ImageBrowserViewRegister();
	require_noerr( err, CantRegister );

	// create an initialization event
	err = CreateEvent( NULL, kEventClassHIObject, kEventHIObjectInitialize, GetCurrentEventTime(), 0, &event );
	require_noerr( err, CantCreateEvent );
	
	// if bounds were provided, add them to the initialization event
	if ( inBounds != NULL )
	{
		err = SetEventParameter( event, kEventParamBounds, typeHIRect, sizeof( HIRect ), inBounds );
		require_noerr( err, CantSetParameter );
	}
	
	// if image URLs were provided, add them to the initialization event
	if ( inImageURLs != NULL )
	{
		err = SetEventParameter( event, kEventParamImageURLArray, typeCFTypeRef, sizeof( CFArrayRef ), inImageURLs );
		require_noerr( err, CantSetParameter );
	}

	// create a new instance of this class
	err = HIObjectCreate( kImageBrowserViewClassID, event, (HIObjectRef*)outView );
	require_noerr( err, CantCreate );

	// if a parent window was provided, place the new view into the parent window
	if ( inWindow != NULL )
	{
		verify_noerr( HIViewFindByID( HIViewGetRoot( inWindow ), kHIViewWindowContentID, &content ) );
		verify_noerr( HIViewAddSubview( content, *outView ) );
	}

CantCreate:
CantSetParameter:
CantCreateEvent:
	
	ReleaseEvent( event );
	
CantRegister:
	
	return err;
}

//------------------------------------------------------------------------------
//	ImageBrowserViewAddImages
//------------------------------------------------------------------------------
//
OSStatus TImageBrowserView::AddImages(
	HIViewRef				inView,
	CFArrayRef				inImageURLs )
{
	OSStatus				err = noErr;
	TImageBrowserView*		data;
	
	data = (TImageBrowserView*)HIObjectDynamicCast( (HIObjectRef)inView, kImageBrowserViewClassID );
	require_action( data != NULL, CantGetInstanceData, err = paramErr );
	
	CFArrayAppendArray( data->fImageURLs, inImageURLs, CFRangeMake( 0, CFArrayGetCount( inImageURLs ) ) );
	
	verify_noerr( data->Invalidate() );
	
CantGetInstanceData:

	return err;
}

#pragma mark -
#pragma mark Methods

// -----------------------------------------------------------------------------
//	TImageBrowserView constructor
// -----------------------------------------------------------------------------
//
TImageBrowserView::TImageBrowserView(
	HIViewRef			inView )
	: TView( inView )
{
	// turn on the TObject and TView accessibility, drag and drop, and keyboard focus interfaces
	verify_noerr( ActivateInterface( kAccessibility ) );
	verify_noerr( ActivateInterface( kDragAndDrop ) );
	verify_noerr( ActivateInterface( kKeyboardFocus ) );
	
	// register for the various Carbon Events that cannot be implicitly turned on via ActivateInterface
	static const EventTypeSpec kImageBrowserViewEvents[] =
	{
		{ kEventClassHIObject, kEventHIObjectEncode },
		
		{ kEventClassControl, kEventControlGetFocusPart },
		{ kEventClassControl, kEventControlTrackingAreaEntered },
		{ kEventClassControl, kEventControlTrackingAreaExited }
	};
	
	verify_noerr( ::AddEventTypesToHandler( GetEventHandler(), GetEventTypeCount( kImageBrowserViewEvents ),
										kImageBrowserViewEvents ) );
}

// -----------------------------------------------------------------------------
//	TImageBrowserView destructor
// -----------------------------------------------------------------------------
//
TImageBrowserView::~TImageBrowserView()
{
	if ( fImageURLs != NULL )
		::CFRelease( fImageURLs );
	
	if ( fTrackingArea != NULL )
		verify_noerr( ::HIViewDisposeTrackingArea( fTrackingArea ) );
	
	InvalidateImageCache();
}

// -----------------------------------------------------------------------------
//	GetKind
// -----------------------------------------------------------------------------
//
ControlKind
TImageBrowserView::GetKind()
{
	const ControlKind kMyKind = { 'TImB', 'TImB' };
	
	return kMyKind;
}

//-----------------------------------------------------------------------------------
//	Construct
//-----------------------------------------------------------------------------------
//
OSStatus
TImageBrowserView::Construct(
	HIObjectRef			inObjectRef,
	TObject**			outObject )
{
	*outObject = new TImageBrowserView( (HIViewRef)inObjectRef );
	
	return noErr;
}

//------------------------------------------------------------------------------
//	IInitialize
//------------------------------------------------------------------------------
//
OSStatus TImageBrowserView::Initialize(
	TCarbonEvent&				inEvent )
{
	OSStatus				err = noErr;
	HIArchiveRef			decoder = NULL;
	HIViewRef				view = GetViewRef();
	CFArrayRef				imageURLs = NULL;
	CFStringRef				imagePath;
	
	// look for the optional HIArchive decoder in the event
	inEvent.GetParameter( kEventParamHIArchive, typeCFTypeRef, sizeof( HIArchiveRef ), &decoder );
	
	// defult to the first image
	fImageIndex = 0;
		
	if ( decoder != NULL ) // if we're unarchiving, initialize from the decoder ...
	{
		// decode our persistant state
		err = ::HIArchiveCopyDecodedCFType( decoder, kImageURLsArchiveKey, (CFTypeRef*)&imageURLs );
		require_noerr( err, CantDecodeImageURLs );
		
		err = ::HIArchiveDecodeNumber( decoder, kImageIndexArchiveKey, kCFNumberCFIndexType, &fImageIndex );
		require_noerr( err, CantDecodeImageIndex );
	}
	else // ... otherwise initialize as normal
	{
		// extract the optional image URLs from the initialization event
		inEvent.GetParameter( kEventParamImageURLArray, typeCFTypeRef, sizeof( CFArrayRef ), &imageURLs );
	}
	
	// create a mutable version of the image URLs array for later modification
	if ( imageURLs != NULL )
		fImageURLs = ::CFArrayCreateMutableCopy( kCFAllocatorDefault, 0, imageURLs );
	else
		fImageURLs = ::CFArrayCreateMutable( kCFAllocatorDefault, 0, &kCFTypeArrayCallBacks );
	
	require_action( fImageURLs != NULL, CantCreateImageURLArray, err = coreFoundationUnknownErr );
	
	// extract the optional image path from the initialization event
	if ( inEvent.GetParameter( kEventParamImageURL, typeCFStringRef, sizeof( CFStringRef ), &imagePath ) == noErr )
	{
		CFURLRef imageURL = ::CFURLCreateWithFileSystemPath( kCFAllocatorDefault, imagePath, kCFURLPOSIXPathStyle, false );

		require_action( imageURL != NULL, CantCreateImageURL, err = coreFoundationUnknownErr );
		
		::CFArrayAppendValue( fImageURLs, imageURL );		
		::CFRelease( imageURL );
	}
	
	// we're not tracking any drags by default
	fTrackingDrag = false;
	
	// install a tracking area on the entire view to implement a mouse over effect
	verify_noerr( ::HIViewNewTrackingArea( view, NULL, 0, &fTrackingArea ) );
	
	// the mouse is not in the view by default
	fMouseInView = false;
	
	// default to a non-focused state
	fCurrentFocusPart = kHIViewFocusNoPart;
	
	// we have no cached image
	fImageCache = NULL;
	fImageCacheDisabled = NULL;
	fImageCacheIndex = -1;
	
	// allow the view to be archived
	verify_noerr( HIObjectSetArchivingIgnored( (HIObjectRef)GetViewRef(), false ) );
	
	// let HIToolbox know that we're an opaque view for performance
	verify_noerr( ::HIViewChangeFeatures( view, kHIViewFeatureIsOpaque, 0 ) );
	
	// always invalidate on enable and activate changes
	ChangeAutoInvalidateFlags( kAutoInvalidateOnEnable | kAutoInvalidateOnActivate, 0 );
	
CantCreateImageURL:
CantCreateImageURLArray:
CantDecodeImageIndex:
CantDecodeImageURLs:
InitBoundsParameterMissing:
CantInitializeSuperclass:
	
	return err;
}

//------------------------------------------------------------------------------
//	Encode
//------------------------------------------------------------------------------
//
OSStatus TImageBrowserView::Encode(
	HIArchiveRef			inEncoder )
{
	OSStatus err;
	
	// encode our persistant state
	err = HIArchiveEncodeCFType( inEncoder, kImageURLsArchiveKey, fImageURLs );
	require_noerr( err, CantEncodeImageURLs );
	
	err = HIArchiveEncodeNumber( inEncoder, kImageIndexArchiveKey, kCFNumberCFIndexType, &fImageIndex );
	require_noerr( err, CantEncodeImageIndex );
	
CantEncodeImageIndex:
CantEncodeImageURLs:
CantEncodeSuperclass:
	
	return err;
}

//-----------------------------------------------------------------------------------
//	Draw
//-----------------------------------------------------------------------------------
//
void TImageBrowserView::Draw(
	RgnHandle			inLimitRgn,
	CGContextRef		inContext )
{	
	OSStatus				err = noErr;
	HIRect					bounds = Bounds();
	CFIndex					imageURLCount = CFArrayGetCount( fImageURLs );
	
	// draw a black background/frame
	::CGContextSetRGBFillColor( inContext, 0, 0, 0, 1 );
	::CGContextFillRect( inContext, bounds );
	
	// draw the current image if it exists
	if ( imageURLCount > 0 )
	{
		CGImageSourceRef	imageSource;
		HIRect				insetBounds = ::CGRectInset( bounds, kImageMargin, kImageMargin );
		HIRect				imageBounds = insetBounds;
		float				scale;
		CGImageRef			image;
		
		if ( fImageCacheIndex != fImageIndex )
			InvalidateImageCache();
		
		// don't rerender the image if the cache is still valid
		if ( fImageCache == NULL )
		{
			CFURLRef imageURL = (CFURLRef)::CFArrayGetValueAtIndex( fImageURLs, fImageIndex );
			
			// dynamically load and cache the image from the URL with ImageIO as necessary
			imageSource = ::CGImageSourceCreateWithURL( imageURL, NULL );
			require_action( imageSource != NULL, CantCreateImageSource, err = coreFoundationUnknownErr );
			
			fImageCache = ::CGImageSourceCreateImageAtIndex( imageSource, 0, NULL );
			CFRelease( imageSource );
			require_action( fImageCache != NULL, CantCreateImage, err = coreFoundationUnknownErr );
			
			// cache the image data in a bitmap context to improve drawing performance
			ConvertImageToBitmapImage( &fImageCache );
			
			// create a disabled version of the image
			::HICreateTransformedCGImage( fImageCache, kHITransformDisabled, &fImageCacheDisabled );
			
			fImageCacheIndex = fImageIndex;
		}
		
		imageBounds.size.width = ::CGImageGetWidth( fImageCache );
		imageBounds.size.height = ::CGImageGetHeight( fImageCache );
		
		// determine image scale factor based on relative sizes of view and image bounds
		if ( imageBounds.size.width/insetBounds.size.width > imageBounds.size.height/insetBounds.size.height )
			scale = insetBounds.size.width/imageBounds.size.width;
		else
			scale = insetBounds.size.height/imageBounds.size.height;
		
		// scale and center the image
		imageBounds.size.width *= scale;
		imageBounds.size.height *= scale;
		imageBounds.origin.x += (insetBounds.size.width - imageBounds.size.width )/2;
		imageBounds.origin.y += (insetBounds.size.height - imageBounds.size.height )/2;
				
		// determine whether to use the regular or disabled image
		if ( !(IsEnabled() && IsActive()) && fImageCacheDisabled != NULL )
			image = fImageCacheDisabled;
		else
			image = fImageCache;
		
		// draw the image
		verify_noerr( ::HIViewDrawCGImage( inContext, &imageBounds, image ) );
		
CantCreateImageSource:
CantCreateImage:
		;
	}
		
	// draw a drag hilight if necessary
	if ( fTrackingDrag )
	{
		RGBColor dragColor;
		
		// draw with the appropriate theme brush
		verify_noerr( ::GetThemeBrushAsColor( kThemeBrushDragHilite, 32, true, &dragColor ) );
		
		::CGContextSetRGBStrokeColor( inContext, (float)dragColor.red/65535, (float)dragColor.green/65535,
									(float)dragColor.blue/65535, 1 );
		::CGContextSetLineWidth( inContext, 5 );
		::CGContextStrokeRect( inContext, GetPartFrame( kImagePart ) );
	}
	
	// draw the currently focused area if necessary
	if ( fCurrentFocusPart != kHIViewFocusNoPart && IsEnabled() && IsActive() )
	{
		RGBColor	hiliteColor;
		HIRect		focusFrame = GetPartFrame( fCurrentFocusPart );
		
		// draw with the appropriate theme brush
		verify_noerr( ::GetThemeBrushAsColor( kThemeBrushFocusHighlight, 32, true, &hiliteColor ) );
		
		::CGContextSetRGBStrokeColor( inContext, (float)hiliteColor.red/65535, (float)hiliteColor.green/65535,
									(float)hiliteColor.blue/65535, 1 );
		::CGContextSetLineWidth( inContext, (fCurrentFocusPart == kImagePart) ? 5 : 3 ); // thicker for the image part
		
		::CGContextStrokeRect( inContext, focusFrame );
	}
	
	// only draw the browse buttons when we're enable, active and the mouse button is inside the view or they're focused
	if ( IsEnabled() && IsActive() && ( fMouseInView || fCurrentFocusPart == kBrowseBackPart ||
			 fCurrentFocusPart == kBrowseForwardPart || fCurrentFocusPart == kDeleteImagePart) )
	{
		HISize			shadowOffset = { 0, -2 };
		HIRect			frame;
		HIViewPartCode	hilite = GetHilite();
		
		// add a little shadow to the buttons for effect
		::CGContextSetShadow( inContext, shadowOffset, 2 );
		
		// draw the browse back button
		frame = GetPartFrame( kBrowseBackPart );
		
		::CGContextBeginPath( inContext );
		::CGContextMoveToPoint( inContext, frame.origin.x + frame.size.width - kButtonPad, frame.origin.y );
		::CGContextAddLineToPoint( inContext, frame.origin.x, frame.origin.y + kButtonSize/2 );
		::CGContextAddLineToPoint( inContext, frame.origin.x + frame.size.width - kButtonPad, frame.origin.y + kButtonSize );
		::CGContextAddLineToPoint( inContext, frame.origin.x + frame.size.width - kButtonPad, frame.origin.y );
		::CGContextClosePath( inContext );
		
		// display hilited or disabled if necessary
		if ( hilite == kBrowseBackPart )
			::CGContextSetRGBFillColor( inContext, .66, .66, .66, 1 );
		else
			::CGContextSetRGBFillColor( inContext, 1, 1, 1, IsPartAvailable( kBrowseBackPart ) ? 1 : .33 );
		::CGContextFillPath( inContext );
		
		// draw the browse forward button (appearing disabled if necessary)
		frame = GetPartFrame( kBrowseForwardPart );
		
		::CGContextBeginPath( inContext );
		::CGContextMoveToPoint( inContext, frame.origin.x + kButtonPad, frame.origin.y );
		::CGContextAddLineToPoint( inContext, frame.origin.x + frame.size.width, frame.origin.y + kButtonSize/2 );
		::CGContextAddLineToPoint( inContext, frame.origin.x + kButtonPad, frame.origin.y + kButtonSize );
		::CGContextAddLineToPoint( inContext, frame.origin.x + kButtonPad, frame.origin.y );
		::CGContextClosePath( inContext );
		
		// display hilited or disabled if necessary
		if ( hilite == kBrowseForwardPart )
			::CGContextSetRGBFillColor( inContext, .66, .66, .66, 1 );
		else
			::CGContextSetRGBFillColor( inContext, 1, 1, 1, IsPartAvailable( kBrowseForwardPart ) ? 1 : .33 );
		::CGContextFillPath( inContext );
		
		// draw the delete image button (appearing disabled if necessary)
		frame = ::CGRectInset( GetPartFrame( kDeleteImagePart ), 2, 2 );
		
		::CGContextBeginPath( inContext );
		::CGContextMoveToPoint( inContext, frame.origin.x, frame.origin.y );
		::CGContextAddLineToPoint( inContext, frame.origin.x + frame.size.width, frame.origin.y + frame.size.height );
		::CGContextMoveToPoint( inContext, frame.origin.x, frame.origin.y + frame.size.height );
		::CGContextAddLineToPoint( inContext, frame.origin.x + frame.size.width, frame.origin.y );
		::CGContextClosePath( inContext );
		
		// display hilited or disabled if necessary
		if ( hilite == kDeleteImagePart )
			::CGContextSetRGBStrokeColor( inContext, .66, .66, .66, 1 );
		else
			::CGContextSetRGBStrokeColor( inContext, 1, 1, 1, IsPartAvailable( kDeleteImagePart ) ? 1 : .33 );
		
		::CGContextSetLineWidth( inContext, 3 );
		::CGContextStrokePath( inContext );
	}
}

//-----------------------------------------------------------------------------------
//	HitTest
//-----------------------------------------------------------------------------------
//
ControlPartCode
TImageBrowserView::HitTest(
	const HIPoint&		inWhere )
{
	HIViewPartCode			part;
	
	// find the part that was hit, and make sure that part is currently available
	part = GetViewPart( inWhere );
	
	if ( !IsPartAvailable( part ) )
		part = kControlNoPart;
	
	return part;
}

//-----------------------------------------------------------------------------------
//	GetRegion
//-----------------------------------------------------------------------------------
//
OSStatus
TImageBrowserView::GetRegion(
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
	
		::RectRgn( outRgn, &qdBounds );
	}
	
	return err;
}

//------------------------------------------------------------------------------
//	Track
//------------------------------------------------------------------------------
//
OSStatus TImageBrowserView::Track(
	TCarbonEvent&			inEvent,
	ControlPartCode*		outPartHit )
{
	OSStatus				err = eventNotHandledErr;
	HIPoint					mouse;
	HIViewPartCode			part;
	
	// get the mouse location in view coordinates
	verify_noerr( inEvent.GetParameter( kEventParamMouseLocation, typeHIPoint, sizeof( HIPoint ), &mouse ) );
	
	part = GetViewPart( mouse );
	
	if ( part == kImagePart )
		err = noErr;
	
	// return the part that was tracked
	*outPartHit = part;
	
	return err;
}

//------------------------------------------------------------------------------
//	Hit
//------------------------------------------------------------------------------
//
OSStatus TImageBrowserView::ControlHit(
	ControlPartCode		inPart,
	UInt32				inModifiers )
{
	OSStatus err = noErr;
	
	// don't handle the Hit event if this part isn't currently available
	if ( !IsPartAvailable( inPart ) )
		return eventNotHandledErr;
		
	switch ( inPart )
	{
		case kBrowseBackPart:
			PreviousImage();
		break;
		
		case kBrowseForwardPart:
			NextImage();
		break;
		
		case kDeleteImagePart:
			DeleteCurrentImage();
		break;
		
		default:
			err = eventNotHandledErr;
		break;
	}
	
	return err;
}

#pragma mark -
#pragma mark Keyboard Focus

//------------------------------------------------------------------------------
//	SetFocusPart
//------------------------------------------------------------------------------
//
OSStatus TImageBrowserView::SetFocusPart(
	ControlPartCode			inDesiredFocus,
	Boolean					inFocusEverything,
	ControlPartCode*		outActualFocus )
{
	OSStatus				err = noErr;
	HIViewPartCode			oldFocusPart = fCurrentFocusPart;
	HIViewPartCode			newFocusPart = oldFocusPart;
	
	switch ( inDesiredFocus )
	{
		case kControlFocusNextPart:
			// focus the next part, if there is one that is available
			if ( !inFocusEverything )
			{
				if ( oldFocusPart != kImagePart )
					newFocusPart = kImagePart;
				else
					newFocusPart = kHIViewFocusNoPart;
			}
			else
			{
				while ( ++newFocusPart >= kImagePart && newFocusPart <= kDeleteImagePart )
				{
					if ( IsPartAvailable( newFocusPart ) )
						break;
				}
				if ( newFocusPart > kDeleteImagePart )
					newFocusPart = kHIViewFocusNoPart;
			}
		break;
		
		case kControlFocusPrevPart:
			if ( !inFocusEverything )
			{
				if ( oldFocusPart != kImagePart )
					newFocusPart = kImagePart;
				else
					newFocusPart = kHIViewFocusNoPart;
			}
			else
			{
				// if no focus, set up so that we move to the DeleteImage part first
				if ( newFocusPart == kHIViewFocusNoPart )
					newFocusPart = kDeleteImagePart + 1;
					
				// focus the previous part, if there is one that is available
				while ( --newFocusPart >= kImagePart && newFocusPart <= kDeleteImagePart )
				{
					if ( IsPartAvailable( newFocusPart ) )
						break;
				}
				if ( newFocusPart < kImagePart )
					newFocusPart = kHIViewFocusNoPart;
			}
		break;
		
		default:
			if ( IsPartAvailable( inDesiredFocus ) )
				newFocusPart = inDesiredFocus;
			else
				newFocusPart = inDesiredFocus + (inDesiredFocus - oldFocusPart); // request the next part if necessary
		break;
	}
		
	fCurrentFocusPart = newFocusPart;
	*outActualFocus = newFocusPart;
	
	// invalidate ourselves if the focus changed
	if ( fCurrentFocusPart != oldFocusPart )
	{
		// if focus was changing to or from the image part, invalidate the entire view ...
		if ( fCurrentFocusPart == kImagePart || oldFocusPart == kImagePart )
		{
			verify_noerr( Invalidate() );
		}
		else // ... otherwise, only invalidate the subparts for better performance
		{
			HIViewRef	view = GetViewRef();
			HIRect		focusUpdateFrame = ::CGRectUnion( GetPartFrame( (HIViewPartCode)kBrowseBackPart ),
															GetPartFrame( (HIViewPartCode)kDeleteImagePart ) );
			
			// make sure to invalidate the entire focus area
			focusUpdateFrame = ::CGRectInset( focusUpdateFrame, -2, -2 );
			
			verify_noerr( ::HIViewSetNeedsDisplayInRect( view, &focusUpdateFrame, true ) );
		}
	}
	
	return err;
}

//------------------------------------------------------------------------------
//	GetFocusPart
//------------------------------------------------------------------------------
//
OSStatus TImageBrowserView::GetFocusPart(
	HIViewPartCode*			outCurrentFocusPart )
{
	*outCurrentFocusPart = fCurrentFocusPart;
	
	return noErr;
}

#pragma mark -
#pragma mark Drag and Drop

//------------------------------------------------------------------------------
//	DragEnter
//------------------------------------------------------------------------------
//
bool TImageBrowserView::DragEnter(
	DragRef					inDrag )
{
	PasteboardRef	pasteboard;
	CFURLRef		fileURL = NULL;
	Boolean			wouldAccept = false;
	
	verify_noerr( ::GetDragPasteboard( inDrag, &pasteboard ) );
	
	fileURL = CreateFileURLFromPasteboard( pasteboard, 1 );
	
	wouldAccept = (fileURL != NULL); // accept the drag if it contains an image file URL
		
	if ( wouldAccept )
	{
		// note that we're tracking an acceptable drag
		fTrackingDrag = true;
		
		// invalidate ourseleves so we can display drag feedback
		verify_noerr( Invalidate() );
	}
	
	if ( fileURL != NULL )
		::CFRelease( fileURL );
	
	return wouldAccept;
}

//------------------------------------------------------------------------------
//	DragLeave
//------------------------------------------------------------------------------
//
bool TImageBrowserView::DragLeave(
	DragRef					inDrag )
{
	// note that we are no longer tracking a drag
	fTrackingDrag = false;
	
	// invalidate ourseleves so we can turn off our drag feedback
	verify_noerr( Invalidate() );
	
	return true;
}

//------------------------------------------------------------------------------
//	DragReceive
//------------------------------------------------------------------------------
//
OSStatus TImageBrowserView::DragReceive(
	DragRef					inDrag )
{
	PasteboardRef pasteboard;
	
	verify_noerr( ::GetDragPasteboard( inDrag, &pasteboard ) );
	
	if ( pasteboard != NULL )
	{
		ItemCount		itemCount;
		CFIndex			itemIndex;
		
		verify_noerr( ::PasteboardGetItemCount( pasteboard, &itemCount ) );
		
		for ( itemIndex = 1; itemIndex <= itemCount; itemIndex++ )
		{
			CFURLRef fileURL = CreateFileURLFromPasteboard( pasteboard, itemIndex );
			
			// insert the image file URL after the current index and increment
			if ( fileURL != NULL )
			{
				AddImage( fileURL );
				::CFRelease( fileURL );
			}
		}
	}
	
	return noErr;
}

//------------------------------------------------------------------------------
//	TextInput
//------------------------------------------------------------------------------
//
OSStatus TImageBrowserView::TextInput(
	TCarbonEvent&			inEvent )
{
	OSStatus	err = noErr;
	UniChar		uniChar;
	HIViewRef	view = GetViewRef();
	
	err = inEvent.GetParameter( kEventParamTextInputSendText, typeUnicodeText, sizeof( UniChar ), &uniChar );
	require_noerr( err, CantGetTextInputParameter );
	
	switch ( uniChar )
	{
		case kSpaceCharCode:
			if ( fCurrentFocusPart == kBrowseBackPart || fCurrentFocusPart == kBrowseForwardPart ||
					fCurrentFocusPart == kDeleteImagePart )
				err = ::HIViewSimulateClick( view, fCurrentFocusPart, 0, NULL );
			else
				err = eventNotHandledErr;  // otherwise let someone else deal with the keystroke
		break;
		
		case kBackspaceCharCode:
		case kDeleteCharCode:
		case kClearCharCode:
			if ( fCurrentFocusPart == kImagePart )
				err = ::HIViewSimulateClick( view, kDeleteImagePart, 0, NULL );
		break;
		
		case kLeftArrowCharCode:
		case kRightArrowCharCode:
			if ( fCurrentFocusPart == kImagePart )
			{
				if ( uniChar == kLeftArrowCharCode )
					PreviousImage();
				else
					NextImage();
			}
			else
			{
				Boolean			foundFocusablePart = false;
				HIViewPartCode	focusPart = fCurrentFocusPart;
				
				// loop forward or backward through the focusable subparts
				while ( !foundFocusablePart )
				{
					focusPart += (uniChar == kLeftArrowCharCode) ? -1 : 1;
					
					// cycle through the subparts
					if ( focusPart == kBrowseBackPart - 1 )
						focusPart = kDeleteImagePart;
					
					if ( focusPart == kDeleteImagePart + 1 )
						focusPart = kBrowseBackPart;
					
					// if the part is enabled we've got it
					if ( IsPartAvailable( focusPart ) )
					{
						foundFocusablePart = true;
						break;
					}
					
					// check to see if we've cycled all the way around looking for an enabled part
					if ( focusPart == fCurrentFocusPart )
					{
						focusPart = kImagePart;
						foundFocusablePart = true;
						break;
					}
				}
				
				err = ::SetKeyboardFocus( GetWindowRef(), view, focusPart );
			}
		break;
		
		default:
			err = eventNotHandledErr; // otherwise let someone else deal with the keystroke
		break;
	}
	
CantGetTextInputParameter:
	
	return err;
}

#pragma mark -
#pragma mark Accessibility

//------------------------------------------------------------------------------
//	CopyAccessibleChildAtPoint
//------------------------------------------------------------------------------
//
OSStatus TImageBrowserView::CopyAccessibleChildAtPoint(
	EventHandlerCallRef		inCallRef,
	EventRef				inEvent,
	AXUIElementRef			inElement,
	UInt64					inIdentifier,
	const HIPoint&			inWhere,
	CFTypeRef*				outChild )
{
	OSStatus				result = eventNotHandledErr;
	HIViewPartCode			part;
	HIPoint					localWhere;
	
	// Only handle events for the basic target/identifier pair. Anything else is custom.
	require_quiet( inIdentifier == 0, NotTheBasicElement );

	// convert to view coordinates
	ConvertGlobalToLocalPoint( inWhere, &localWhere );

	// get the part that is at this point
	part = GetViewPart( localWhere );
	
	// if the part is the image part, we consider that to be "the control" for
	// purposes of accessibility
	if ( part == kImagePart )
		part = kControlNoPart;

	if ( part != kControlNoPart )
	{		
		// create a AXUIElementRef that represents this part
		CFTypeRef child = AXUIElementCreateWithHIObjectAndIdentifier( GetObjectRef(), part );
		require_action( child != NULL, CantCreateChild, result = memFullErr );
		
		*outChild = child;
		
		result = noErr;
	}
		
CantCreateChild:
NotTheBasicElement:

	return result;
}

//------------------------------------------------------------------------------
//	CopyAccessibleFocusedChild
//------------------------------------------------------------------------------
//
OSStatus TImageBrowserView::CopyAccessibleFocusedChild(
	EventHandlerCallRef		inCallRef,
	EventRef				inEvent,
	AXUIElementRef			inElement,
	UInt64					inIdentifier,
	CFTypeRef*				outChild )
{
	OSStatus				result = eventNotHandledErr;
	HIViewPartCode			part;
	
	part = fCurrentFocusPart;
	
	// Only handle events for the basic target/identifier pair. Anything else is custom.
	require_quiet( inIdentifier == 0, NotTheBasicElement );
	
	// for the purposes of accessibility, the image part is the control
	if ( part == kImagePart )
		part = kControlNoPart;
	
	if ( fCurrentFocusPart != kControlNoPart )
	{
		CFTypeRef child = AXUIElementCreateWithHIObjectAndIdentifier( GetObjectRef(), fCurrentFocusPart );
		require_action( child != NULL, CantCreateChild, result = memFullErr );
		
		*outChild = child;
		
		result = noErr;
	}
	
CantCreateChild:
NotTheBasicElement:

	return result;
}

//------------------------------------------------------------------------------
//	GetAccessibleAttributeNames
//------------------------------------------------------------------------------
//
OSStatus TImageBrowserView::GetAccessibleAttributeNames(
	EventHandlerCallRef		inCallRef,
	EventRef				inEvent,
	AXUIElementRef			inElement,
	UInt64					inIdentifier,
	CFMutableArrayRef		outNames )
{
	OSStatus				result = eventNotHandledErr;
	
	// If this request is for the entire control
	if ( inIdentifier == 0 )
	{
		// Have the base class report its attributes and report its error
		result = CallNextEventHandler( inCallRef, inEvent );
		
		CFArrayAppendValue( outNames, kAXFocusedAttribute );
		CFArrayAppendValue( outNames, kAXChildrenAttribute );
	}
	else
	{
		CFArrayAppendValue( outNames, kAXFocusedAttribute );
		CFArrayAppendValue( outNames, kAXEnabledAttribute );
		CFArrayAppendValue( outNames, kAXSizeAttribute );
		CFArrayAppendValue( outNames, kAXPositionAttribute );
	}
	
	return result;
}

//------------------------------------------------------------------------------
//	GetAccessibleNamedAttribute
//------------------------------------------------------------------------------
//
OSStatus TImageBrowserView::GetAccessibleNamedAttribute(
	EventHandlerCallRef		inCallRef,
	EventRef				inEvent,
	AXUIElementRef			inElement,
	UInt64					inIdentifier,
	CFStringRef				inAttribute )
{
	OSStatus result = eventNotHandledErr;
	
	if ( inIdentifier == 0 )
	{
		if ( CFStringCompare( inAttribute, kAXRoleAttribute, 0 ) == kCFCompareEqualTo )
		{
			CFStringRef		role = CFSTR( "ImageBrowser" );
			
			result = SetEventParameter( inEvent, kEventParamAccessibleAttributeValue,
							typeCFStringRef, sizeof( role ), &role );
		}
		else if ( CFStringCompare( inAttribute, kAXRoleDescriptionAttribute, 0 ) == kCFCompareEqualTo )
		{
			CFStringRef		roleDescription = CFSTR( "Image Browser" );
			
			result = SetEventParameter( inEvent, kEventParamAccessibleAttributeValue,
							typeCFStringRef, sizeof( roleDescription ), &roleDescription );
		}
		else if ( CFStringCompare( inAttribute, kAXFocusedAttribute, 0 ) == kCFCompareEqualTo )
		{
			Boolean	focused = fCurrentFocusPart == (HIViewPartCode)kImagePart;
			
			result = SetEventParameter( inEvent, kEventParamAccessibleAttributeValue,
							typeBoolean, sizeof( focused ), &focused );
		}
		else if ( CFStringCompare( inAttribute, kAXChildrenAttribute, 0 ) == kCFCompareEqualTo )
		{
			AXUIElementRef		child;
			CFMutableArrayRef	array = NULL;
			OSStatus			err;
			
			// get the array from the event
			err = GetEventParameter( inEvent, kEventParamAccessibleAttributeValue, typeCFTypeRef,
						NULL, sizeof( array ), NULL, &array );
			
			// if the array hasn't yet been created, create it here
			if ( err != noErr && array == NULL )
				array = CFArrayCreateMutable( kCFAllocatorDefault, 0, &kCFTypeArrayCallBacks );
			
			// must report back all of the parts of the view			
			child = AXUIElementCreateWithHIObjectAndIdentifier( GetObjectRef(), kBrowseBackPart );
			require_action( child != NULL, CantCreateChild, result = memFullErr );
			CFArrayAppendValue( array, child );
			CFRelease( child );
			
			child = AXUIElementCreateWithHIObjectAndIdentifier( GetObjectRef(), kBrowseForwardPart );
			require_action( child != NULL, CantCreateChild, result = memFullErr );
			CFArrayAppendValue( array, child );
			CFRelease( child );
			
			child = AXUIElementCreateWithHIObjectAndIdentifier( GetObjectRef(), kDeleteImagePart );
			require_action( child != NULL, CantCreateChild, result = memFullErr );
			CFArrayAppendValue( array, child );
			CFRelease( child );
			
			result = noErr;
			
			// if the array hadn't been created, we need to stuff it into the event
			if ( err != noErr )
			{
				verify_noerr( SetEventParameter( inEvent, kEventParamAccessibleAttributeValue, typeCFTypeRef,
									sizeof( array ), &array ) );
				CFRelease( array );
			}
		}
	}
	else
	{
		// this request is for the navigation buttons
		if ( CFStringCompare( inAttribute, kAXRoleAttribute, 0 ) == kCFCompareEqualTo )
		{
			CFStringRef	role = kAXButtonRole;
			
			result = SetEventParameter( inEvent, kEventParamAccessibleAttributeValue,
						typeCFStringRef, sizeof( role ), &role );
		}
		else if ( CFStringCompare( inAttribute, kAXRoleDescriptionAttribute, 0 ) == kCFCompareEqualTo )
		{
			CFStringRef	description = HICopyAccessibilityRoleDescription( kAXButtonRole, NULL );
			
			if ( description != NULL )
			{
				result = SetEventParameter( inEvent, kEventParamAccessibleAttributeValue,
							typeCFStringRef, sizeof( description ), &description );
							
				CFRelease( description );
			}
		}
		else if ( CFStringCompare( inAttribute, kAXFocusedAttribute, 0 ) == kCFCompareEqualTo )
		{
			Boolean	focused = fCurrentFocusPart == (HIViewPartCode)inIdentifier;
			
			result = SetEventParameter( inEvent, kEventParamAccessibleAttributeValue,
							typeBoolean, sizeof( focused ), &focused );
		}
		else if ( CFStringCompare( inAttribute, kAXEnabledAttribute, 0 ) == kCFCompareEqualTo )
		{
			Boolean	enabled = IsPartAvailable( (HIViewPartCode)inIdentifier );
			
			result = SetEventParameter( inEvent, kEventParamAccessibleAttributeValue,
							typeBoolean, sizeof( enabled ), &enabled );
		}
		else if ( CFStringCompare( inAttribute, kAXParentAttribute, 0 ) == kCFCompareEqualTo )
		{
			AXUIElementRef	parent = AXUIElementCreateWithHIObjectAndIdentifier( GetObjectRef(), 0 );
			
			require_action( parent != NULL, CantCreateParent, result = memFullErr );
			
			result = SetEventParameter( inEvent, kEventParamAccessibleAttributeValue,
							typeCFTypeRef, sizeof( parent ), &parent );
			CFRelease( parent );
		}
		else if ( CFStringCompare( inAttribute, kAXSizeAttribute, 0 ) == kCFCompareEqualTo )
		{
			HIRect	frame = GetPartFrame( (HIViewPartCode)inIdentifier );
			
			result = SetEventParameter( inEvent, kEventParamAccessibleAttributeValue,
							typeHISize, sizeof( frame.size ), &frame.size );
		}
		else if ( CFStringCompare( inAttribute, kAXPositionAttribute, 0 ) == kCFCompareEqualTo )
		{
			HIRect	frame = GetPartFrame( (HIViewPartCode)inIdentifier );
			
			ConvertLocalToGlobalPoint( &frame.origin, &frame.origin );
			
			result = SetEventParameter( inEvent, kEventParamAccessibleAttributeValue,
							typeHIPoint, sizeof( frame.origin ), &frame.origin );
		}
	}

	// If we didn't handle it, pass it off to the base class
	if ( result == eventNotHandledErr )
		result = CallNextEventHandler( inCallRef, inEvent );

CantCreateParent:
CantCreateChild:
NotTheBasicElement:

	return result;
}

//------------------------------------------------------------------------------
//	GetAccessibleActionNames
//------------------------------------------------------------------------------
//
OSStatus TImageBrowserView::GetAccessibleActionNames(
	EventHandlerCallRef		inCallRef,
	EventRef				inEvent,
	AXUIElementRef			inElement,
	UInt64					inIdentifier,
	CFMutableArrayRef		outNames )
{
	OSStatus				result = eventNotHandledErr;
	
	// If the control is being asked this, pass it to the base class, otherwise
	// provide the actions that the children attributes handle
	if ( inIdentifier == 0 )
	{
		result = CallNextEventHandler( inCallRef, inEvent );
	}
	else
	{
		// the buttons handle the pressed action		
		CFArrayAppendValue( outNames, kAXPressAction );
		
		result = noErr;
	}
	
	return result;
}

//------------------------------------------------------------------------------
//	PerformAccessibleNamedAction
//------------------------------------------------------------------------------
//
OSStatus TImageBrowserView::PerformAccessibleNamedAction(
	EventHandlerCallRef		inCallRef,
	EventRef				inEvent,
	AXUIElementRef			inElement,
	UInt64					inIdentifier,
	CFStringRef				inName )
{
	OSStatus				result = eventNotHandledErr;
	
	// handle the action for the children attributes
	if ( inIdentifier != 0 )
	{
		if ( CFStringCompare( inName, kAXPressAction, 0 ) == kCFCompareEqualTo )
		{
			// Simulate a click in the part
			if ( IsPartAvailable( (HIViewPartCode)inIdentifier ) )
				::HIViewSimulateClick( GetViewRef(), (HIViewPartCode)inIdentifier, 0, NULL );
			
			result = noErr;
		}
	}
	
	return result;
}

//------------------------------------------------------------------------------
//	CopyAccessibleNamedActionDescription
//------------------------------------------------------------------------------
//
OSStatus TImageBrowserView::CopyAccessibleNamedActionDescription(
	EventHandlerCallRef		inCallRef,
	EventRef				inEvent,
	AXUIElementRef			inElement,
	UInt64					inIdentifier,
	CFStringRef				inName,
	CFStringRef*			outDescription )
{
	OSStatus				result = eventNotHandledErr;
	
	// We only handle pressed events, so if this is for a pressed action,
	// we need to determine which child the request is for and supply the 
	// appropriate string
	if ( CFStringCompare( inName, kAXPressAction, 0 ) == kCFCompareEqualTo )
	{
		switch ( inIdentifier )
		{
			case kBrowseBackPart:
			{
				*outDescription = (CFStringRef)::CFRetain( CFSTR( "Go to the previous image" ) );
				result = noErr;
			}
			break;
			
			case kBrowseForwardPart:
			{
				*outDescription = (CFStringRef)::CFRetain( CFSTR( "Go to the next image" ) );
				result = noErr;
			}
			break;
			
			case kDeleteImagePart:
			{
				*outDescription = (CFStringRef)::CFRetain( CFSTR( "Delete the current image" ) );
				result = noErr;
			}
			break;
		}
	}
	
	return result;
}

#pragma mark -
#pragma mark Notifications

//------------------------------------------------------------------------------
//	ActiveStateChanged
//------------------------------------------------------------------------------
//
void TImageBrowserView::ActiveStateChanged()
{
	Boolean invalidate = false;
	
	// invalidate on activation/deactivation when the view is focused
	if ( fCurrentFocusPart != kHIViewFocusNoPart )
		invalidate = true;
		
	if ( IsActive() )
	{
		Point mouse;
		::GetGlobalMouse( &mouse );
		
		HIPoint pt = { mouse.h, mouse.v };
		ConvertGlobalToLocalPoint( pt, &pt );
		
		if ( GetViewPart( pt ) != 0 )
		{
			fMouseInView = true;
			invalidate = true;
		}
	}
	else if ( fMouseInView )
	{
		// invalidate on deactivation if the mouse is inside a part that shows rollover behavior
		fMouseInView = false;
		invalidate = true;
	}
	
	if ( invalidate )
		verify_noerr( Invalidate() );
}

//------------------------------------------------------------------------------
//	HiliteChanged
//------------------------------------------------------------------------------
//
void TImageBrowserView::HiliteChanged(
	ControlPartCode		inOriginalPart,
	ControlPartCode		inCurrentPart )
{
	HIViewRef	view = GetViewRef();
	
	// get bounds of old and new hilited parts, and also invalidate shadow and focus
	HIRect		originalFrame = CGRectInset( GetPartFrame( inOriginalPart ), -2, -2 );
	HIRect		currentFrame = CGRectInset( GetPartFrame( inCurrentPart ), -2, -2 );
	
	if ( inOriginalPart != kHIViewNoPart )
		verify_noerr( ::HIViewSetNeedsDisplayInRect( view, &originalFrame, true ) );
	
	if ( inCurrentPart != kHIViewNoPart )
		verify_noerr( ::HIViewSetNeedsDisplayInRect( view, &currentFrame, true ) );
}

//------------------------------------------------------------------------------
//	OwningWindowChanged
//------------------------------------------------------------------------------
//
void TImageBrowserView::OwningWindowChanged(
	WindowRef				oldWindow,
	WindowRef				newWindow )
{
	// turn on drag tracking for the window and view
	verify_noerr( SetAutomaticControlDragTrackingEnabledForWindow( newWindow, true ) );
	verify_noerr( SetControlDragTrackingEnabled( GetViewRef(), true ) );
}

//------------------------------------------------------------------------------
//	TrackingAreaEvent
//------------------------------------------------------------------------------
//
OSStatus TImageBrowserView::TrackingAreaEvent(
	bool					inIsEnterEvent )
{	
	// support rollover subpart hide/show
	if ( IsEnabled() && IsActive() )
	{
		HIViewRef view = GetViewRef();
		
		fMouseInView = inIsEnterEvent;
		
		HIRect trackingUpdateFrame = ::CGRectUnion( GetPartFrame( (HIViewPartCode)kBrowseBackPart ),
													GetPartFrame( (HIViewPartCode)kDeleteImagePart ) );
		
		// also invalidate shadow and focus areas
		trackingUpdateFrame = CGRectInset( trackingUpdateFrame, -2, -2 );
		
		verify_noerr( ::HIViewSetNeedsDisplayInRect( view, &trackingUpdateFrame, true ) );
	}
	
	return noErr;
}

#pragma mark -
#pragma mark Generic Event Handling

//------------------------------------------------------------------------------
//	HandleEvent
//------------------------------------------------------------------------------
//
OSStatus TImageBrowserView::HandleEvent(
	EventHandlerCallRef		inCallRef,
	TCarbonEvent&			inEvent )
{
	OSStatus err = eventNotHandledErr;
	
	switch( inEvent.GetClass() )
	{
		case kEventClassHIObject:
		{
			switch ( inEvent.GetKind() )
			{
				case kEventHIObjectEncode:
				{
					HIArchiveRef encoder;
					
					// allow the superclass to encode itself
					err = CallNextEventHandler( inCallRef, inEvent );
					require_noerr( err, CantEncodeSuperclass );
					
					// get the HIArchive encoder from the event
					verify_noerr( inEvent.GetParameter( kEventParamHIArchive, typeCFTypeRef,
															sizeof( HIArchiveRef ), &encoder ) );
					
					err = Encode( encoder );
				}
				break;
			}
			break;
			
			case kEventClassControl:
			{
				switch ( inEvent.GetKind() )
				{
					case kEventControlGetFocusPart:
					{
						HIViewPartCode part;
						
						err = GetFocusPart( &part );
						
						return inEvent.SetParameter( kEventParamControlPart, part );
					}
					break;
					
					case kEventControlTrackingAreaEntered:
					case kEventControlTrackingAreaExited:
						err = TrackingAreaEvent( inEvent.GetKind() == kEventControlTrackingAreaEntered );
					break;
				}
			}
		}
		break;
	}
	
	if ( err == eventNotHandledErr )
		err = TView::HandleEvent( inCallRef, inEvent );
	
CantEncodeSuperclass:
	
	return err;
}

#pragma mark -
#pragma mark Helper Routines

//------------------------------------------------------------------------------
//	GetPartFrame
//------------------------------------------------------------------------------
//
HIRect TImageBrowserView::GetPartFrame(
	HIViewPartCode	inPart )
{
	HIRect			viewBounds = Bounds(), partFrame;
	
	// generate the frame for the requested part
	switch ( inPart )
	{
		case kBrowseBackPart:
			partFrame.origin.x = viewBounds.size.width/2 - kButtonSize;
			partFrame.origin.y = viewBounds.size.height - kButtonSize - kButtonMargin;
			partFrame.size.width = partFrame.size.height = kButtonSize;
		break;
		
		case kBrowseForwardPart:
			partFrame.origin.x = viewBounds.size.width/2;
			partFrame.origin.y = viewBounds.size.height - kButtonSize - kButtonMargin;
			partFrame.size.width = partFrame.size.height = kButtonSize;
		break;
		
		case kDeleteImagePart:
			partFrame.origin.x = viewBounds.size.width - kButtonSize - kButtonMargin;
			partFrame.origin.y = viewBounds.size.height - kButtonSize - kButtonMargin;
			partFrame.size.width = partFrame.size.height = kButtonSize;
		break;
		
		case kImagePart:
			partFrame = ::CGRectInset( viewBounds, 1, 1 );
		break;
	}
	
	return partFrame;
}

//------------------------------------------------------------------------------
//	IsPartAvailable
//------------------------------------------------------------------------------
//	Returns whether a particular part is currently available, based on the
//	contents of the view.
//
bool
TImageBrowserView::IsPartAvailable(
	HIViewPartCode			inPart )
{
	bool enabled = true;
	
	switch ( inPart )
	{
		case kImagePart:
			enabled = IsEnabled();
		break;
		
		case kBrowseBackPart:
			enabled = fImageIndex > 0;
		break;
		
		case kBrowseForwardPart:
			enabled = fImageIndex < ( ::CFArrayGetCount( fImageURLs ) - 1 );
		break;
		
		case kDeleteImagePart:
			enabled = ::CFArrayGetCount( fImageURLs ) > 0;
		break;
	}
	
	return enabled;
}

//------------------------------------------------------------------------------
//	CreateFileURLFromPasteboard
//------------------------------------------------------------------------------
//
CFURLRef TImageBrowserView::CreateFileURLFromPasteboard(
	PasteboardRef		inPasteboard,
	CFIndex				inIndex )
{
	PasteboardItemID		item;
	CFDataRef				fileURLData;
	CFURLRef				fileURL = NULL;
	LSItemInfoRecord		info;
	CFStringRef				uti = NULL;
	bool					isSupported = false;

	require_noerr( ::PasteboardGetItemIdentifier( inPasteboard, inIndex, &item ), CantGetPasteboardIdentifier );
	
	require_noerr_quiet( ::PasteboardCopyItemFlavorData( inPasteboard, item, kUTTypeFileURL, &fileURLData ),
							CantCopyFileURLFromPasteboard );
	
	// create the file URL with the dragged data
	fileURL = ::CFURLCreateWithBytes( kCFAllocatorDefault, CFDataGetBytePtr( fileURLData ), CFDataGetLength( fileURLData ),
									kCFStringEncodingMacRoman, NULL );
	
	// get the UTI for the dragged file
	require_noerr( ::LSCopyItemInfoForURL( fileURL, kLSRequestExtension | kLSRequestTypeCreator, &info ), CantCopyItemInfo );
	
	if ( info.extension != NULL )
	{
		uti = ::UTTypeCreatePreferredIdentifierForTag( kUTTagClassFilenameExtension, info.extension, kUTTypeData );
					
		::CFRelease( info.extension );
	}
	
	if ( uti == NULL )
	{
		CFStringRef typeString = ::UTCreateStringForOSType( info.filetype );
		
		if ( typeString != NULL )
		{
			uti = ::UTTypeCreatePreferredIdentifierForTag( kUTTagClassOSType, typeString, kUTTypeData );
			
			::CFRelease( typeString );
		}
	}
	
	require( uti != NULL, CantCreateFileUTI );
	
	// verify we're dealing with a file that ImageIO can understand
	{
	CFArrayRef	supportedTypes = ::CGImageSourceCopyTypeIdentifiers();
	CFIndex		i, typeCount = ::CFArrayGetCount( supportedTypes );
	
	for( i = 0; i < typeCount; i++ )
	{
		if ( ::UTTypeConformsTo( uti, (CFStringRef)::CFArrayGetValueAtIndex( supportedTypes, i ) ) )
		{
			isSupported = true;
			break;
		}
	}
	
	::CFRelease( supportedTypes );
	}
	
	::CFRelease( uti );
	
CantCreateFileUTI:
CantCopyItemInfo:
	
	if ( !isSupported )
	{
		::CFRelease( fileURL );
		fileURL = NULL;
	}
	::CFRelease( fileURLData );
	
CantCopyFileURLFromPasteboard:
CantGetPasteboardIdentifier:

	return fileURL;
}

//------------------------------------------------------------------------------
//	InvalidateImageCache
//------------------------------------------------------------------------------
//
void TImageBrowserView::InvalidateImageCache( void )
{
	if ( fImageCache != NULL )
	{
		::CFRelease( fImageCache );
		fImageCache = NULL;
	}
	
	if ( fImageCacheDisabled != NULL )
	{
		::CFRelease( fImageCacheDisabled );
		fImageCacheDisabled = NULL;
	}
}

//------------------------------------------------------------------------------
//	PreviousImage
//------------------------------------------------------------------------------
//
void TImageBrowserView::PreviousImage( void )
{
	if ( fImageIndex > 0 )
	{
		fImageIndex--; // decrement the image index if possible
		
		// if the focused part is no longer available, advance the focus to the next part
		if ( !IsPartAvailable( fCurrentFocusPart ) )
			::SetKeyboardFocus( GetWindowRef(), GetViewRef(), kControlFocusNextPart );
			
		// invalidate due to index change
		verify_noerr( Invalidate() );
	}
}

//------------------------------------------------------------------------------
//	NextImage
//------------------------------------------------------------------------------
//
void TImageBrowserView::NextImage( void )
{
	if ( fImageIndex < ::CFArrayGetCount( fImageURLs ) - 1 )
	{
		fImageIndex++; // increment the image index if possible
		
		// if the focused part is no longer available, advance the focus to the previous part
		if ( !IsPartAvailable( fCurrentFocusPart ) )
			::SetKeyboardFocus( GetWindowRef(), GetViewRef(), kControlFocusPrevPart );
			
		// invalidate due to index change
		verify_noerr( Invalidate() );
	}
}

//------------------------------------------------------------------------------
//	AddImage
//------------------------------------------------------------------------------
//
void TImageBrowserView::AddImage(
	CFURLRef				inFileURL )
{
	//
	// If we already have images in the array, then put our new image after the current image index.
	// If we have no images in the array, then imageIndex is zero, and that's the index for the new image.
	//
	if ( CFArrayGetCount( fImageURLs ) > 0 )
		fImageIndex++;
	
	CFArrayInsertValueAtIndex( fImageURLs, fImageIndex, inFileURL );
	
	// invalidate due to index change
	InvalidateImageCache();
	verify_noerr( Invalidate() );
}

//------------------------------------------------------------------------------
//	DeleteCurrentImage
//------------------------------------------------------------------------------
//
void TImageBrowserView::DeleteCurrentImage( void )
{
	if ( ::CFArrayGetCount( fImageURLs ) > 0 )
	{
		::CFArrayRemoveValueAtIndex( fImageURLs, fImageIndex ); 
		if ( fImageIndex > 0 )
			fImageIndex--;
		
		// if the focused part is no longer available, set focus to the image part
		if ( !IsPartAvailable( fCurrentFocusPart ) )
			::SetKeyboardFocus( GetWindowRef(), GetViewRef(), kImagePart );
		
		// invalidate due to index change
		InvalidateImageCache();
		verify_noerr( Invalidate() );
	}
}

//------------------------------------------------------------------------------
//	ReleaseImageBits
//------------------------------------------------------------------------------
//
static void 
ReleaseImageBits( void *info, const void *data, size_t size )
{
	free( (void*) data );
}

//-----------------------------------------------------------------------------
//	ConvertImageToBitmapImage
//-----------------------------------------------------------------------------
//	Draws an image into a bitmap context and replaces the original image with
//	a new image created from the bitmap. This greatly improves performance when
//	the original image potentially needs to be decoded from source data in order
//	to be drawn.
//
void TImageBrowserView::ConvertImageToBitmapImage(
	CGImageRef*				ioImage )
{
	HIRect				bounds;
	CGImageRef			image = NULL;
	size_t				bytesPerRow;
	size_t				size;
	size_t				height, width;
	void *				ptr;
	CGContextRef		context;
	CGColorSpaceRef		colorSpace;
	CGDataProviderRef	provider;
	
	bounds.origin.x = bounds.origin.y = 0;
	bounds.size.width = ::CGImageGetWidth( *ioImage );
	bounds.size.height = ::CGImageGetHeight( *ioImage );

	height = (size_t)::ceil( bounds.size.height );
	width = (size_t)::ceil( bounds.size.width );

	bytesPerRow = ( ( width * 8 * 4 + 7 ) / 8 );
	size = bytesPerRow * height;
	
	ptr = ::calloc( 1, size );
	require( ptr != NULL, CantAllocateImageTemp );

	colorSpace = ::CGColorSpaceCreateDeviceRGB();
	context = ::CGBitmapContextCreate( ptr, width, height, 8, bytesPerRow, colorSpace, kCGImageAlphaPremultipliedFirst );
	require( context != NULL, CouldntCreateContext );
		
	::CGContextDrawImage( context, bounds, *ioImage );

	::CGContextRelease( context );

	// Create a CGImage from our offscreen.
	provider = ::CGDataProviderCreateWithData( 0, ptr, size, ReleaseImageBits );
	
	image = ::CGImageCreate( width, height, 8, 32, bytesPerRow, colorSpace, kCGImageAlphaPremultipliedFirst,
							provider, NULL, false, kCGRenderingIntentDefault );

	::CGColorSpaceRelease( colorSpace );
	::CGDataProviderRelease( provider );

	::CFRelease( *ioImage );
	*ioImage = image;
	
CantAllocateImageTemp:
CouldntCreateContext:
	return;
}

//------------------------------------------------------------------------------
//	ConvertGlobalToLocalPoint
//------------------------------------------------------------------------------
//
HIPoint* TImageBrowserView::ConvertGlobalToLocalPoint(
	const HIPoint&				inPoint,
	HIPoint*					outPoint )
{
	HIPoint					hiPoint = inPoint;
	HIViewRef				view = GetViewRef();
	Rect					bounds;
	
	check( outPoint != NULL );
	
	// use HIPointConvert if available
	if ( ::HIPointConvert != NULL )
	{
		*outPoint = inPoint;
		::HIPointConvert( outPoint, kHICoordSpace72DPIGlobal, NULL, kHICoordSpaceView, view );
	}
	else
	{
		// default
		outPoint->x = outPoint->y = 0;
		
		// convert global to local window coordinates
		::GetWindowBounds( GetWindowRef(), kWindowStructureRgn, &bounds );
		hiPoint.x -= bounds.left;
		hiPoint.y -= bounds.top;
		
		// convert from window to view
		if ( ::HIViewConvertPoint( &hiPoint, NULL, view ) == noErr )
			*outPoint = hiPoint;
	}
	
	return outPoint;
}

//------------------------------------------------------------------------------
//	ConvertLocalToGlobalPoint
//------------------------------------------------------------------------------
//
HIPoint* TImageBrowserView::ConvertLocalToGlobalPoint(
	const HIPoint*			inPoint,
	HIPoint*				outPoint )
{
	HIPoint					hiPoint = *inPoint;
	HIViewRef				view = GetViewRef();
	
	check( inPoint != NULL );
	check( outPoint != NULL );
	
	// use HIPointConvert if available
	if ( ::HIPointConvert != NULL )
	{
		*outPoint = *inPoint;
		::HIPointConvert( outPoint, kHICoordSpaceView, view, kHICoordSpace72DPIGlobal, NULL );
	}
	else
	{
		// default
		outPoint->x = outPoint->y = 0;
		
		// convert view to window
		if ( ::HIViewConvertPoint( &hiPoint, view, NULL ) == noErr )
		{
			Rect	bounds;
			
			::GetWindowBounds( GetWindowRef(), kWindowStructureRgn, &bounds );
			
			// convert to global
			outPoint->x = hiPoint.x + bounds.left;
			outPoint->y = hiPoint.y + bounds.top;
		}
	}
	
	return outPoint;
}

//------------------------------------------------------------------------------
//	GetViewPart
//------------------------------------------------------------------------------
//	Returns the part corresponding to the given point in local coordinates
//
HIViewPartCode TImageBrowserView::GetViewPart(
	const HIPoint&			inPoint )
{
	HIViewPartCode			part = kDeleteImagePart;

	do // test which part the mouse is within cycling backwards from smallest to largest
	{
		if ( ::CGRectContainsPoint( GetPartFrame( part ), inPoint ) )
			break;
	}
	while ( part-- > kImagePart );
	
	return part;
}
