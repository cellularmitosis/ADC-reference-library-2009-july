/*
    File:		ImageBrowserView.c
    
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

#include "ImageBrowserView.h"

/*
	To do:
	
		implement copy/paste
		implement kEventCommandUpdateStatus if control is focused
		in ImageBrowserViewTrack, if mouse is in image, use WaitMouseMoved or TrackMouseLocation and start a drag if appropriate.
*/

//------------------------------------------------------------------------------
//	types
//------------------------------------------------------------------------------
//
typedef struct
{
	HIViewRef				view;
	CFMutableArrayRef		imageURLs;
	CFIndex					imageIndex;
	bool					trackingDrag;
	bool					mouseInView;
	HIViewTrackingAreaRef	trackingArea;
	HIViewPartCode			currentFocusPart;
	CGImageRef				imageCache;
	CGImageRef				imageCacheDisabled;
	CFIndex					imageCacheIndex;
} ImageBrowserViewData;

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
#pragma mark Helper Routines

//------------------------------------------------------------------------------
//	GetPartFrame
//------------------------------------------------------------------------------
//
static HIRect GetPartFrame(
	HIViewRef		inView,
	HIViewPartCode	inPart )
{
	HIRect			viewBounds, partFrame;
	
	// get the bounds for the entire view
	verify_noerr( HIViewGetBounds( inView, &viewBounds ) );
	
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
			partFrame = CGRectInset( viewBounds, 1, 1 );
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
static Boolean
IsPartAvailable(
	ImageBrowserViewData*	inData,
	HIViewPartCode			inPart )
{
	Boolean enabled = true;
	
	switch ( inPart )
	{
		case kImagePart:
			enabled = HIViewIsEnabled( inData->view, NULL );
		break;
		
		case kBrowseBackPart:
			enabled = inData->imageIndex > 0;
		break;
		
		case kBrowseForwardPart:
			enabled = inData->imageIndex < ( CFArrayGetCount( inData->imageURLs ) - 1 );
		break;
		
		case kDeleteImagePart:
			enabled = CFArrayGetCount( inData->imageURLs ) > 0;
		break;
	}
	
	return enabled;
}

//------------------------------------------------------------------------------
//	CreateFileURLFromPasteboard
//------------------------------------------------------------------------------
//
static CFURLRef CreateFileURLFromPasteboard(
	PasteboardRef		inPasteboard,
	CFIndex				inIndex )
{
	PasteboardItemID		item;
	CFDataRef				fileURLData;
	CFURLRef				fileURL = NULL;
	LSItemInfoRecord		info;
	CFStringRef				uti = NULL;
	bool					isSupported = false;

	require_noerr( PasteboardGetItemIdentifier( inPasteboard, inIndex, &item ), CantGetPasteboardIdentifier );
	
	require_noerr_quiet( PasteboardCopyItemFlavorData( inPasteboard, item, kUTTypeFileURL, &fileURLData ),
							CantCopyFileURLFromPasteboard );
	
	// create the file URL with the dragged data
	fileURL = CFURLCreateWithBytes( kCFAllocatorDefault, CFDataGetBytePtr( fileURLData ), CFDataGetLength( fileURLData ),
									kCFStringEncodingMacRoman, NULL );
	
	// get the UTI for the dragged file
	require_noerr( LSCopyItemInfoForURL( fileURL, kLSRequestExtension | kLSRequestTypeCreator, &info ), CantCopyItemInfo );
	
	if ( info.extension != NULL )
	{
		uti = UTTypeCreatePreferredIdentifierForTag( kUTTagClassFilenameExtension, info.extension, kUTTypeData );
					
		CFRelease( info.extension );
	}
	
	if ( uti == NULL )
	{
		CFStringRef typeString = UTCreateStringForOSType( info.filetype );
		
		if ( typeString != NULL )
		{
			uti = UTTypeCreatePreferredIdentifierForTag( kUTTagClassOSType, typeString, kUTTypeData );
			
			CFRelease( typeString );
		}
	}
	
	require( uti != NULL, CantCreateFileUTI );
	
	// verify we're dealing with a file that ImageIO can understand
	{
	CFArrayRef	supportedTypes = CGImageSourceCopyTypeIdentifiers();
	CFIndex		i, typeCount = CFArrayGetCount( supportedTypes );
	
	for( i = 0; i < typeCount; i++ )
	{
		if ( UTTypeConformsTo( uti, (CFStringRef)CFArrayGetValueAtIndex( supportedTypes, i ) ) )
		{
			isSupported = true;
			break;
		}
	}
	
	CFRelease( supportedTypes );
	}
	
	CFRelease( uti );
	
CantCreateFileUTI:
CantCopyItemInfo:
	
	if ( !isSupported )
	{
		CFRelease( fileURL );
		fileURL = NULL;
	}
	CFRelease( fileURLData );
	
CantCopyFileURLFromPasteboard:
CantGetPasteboardIdentifier:

	return fileURL;
}

//------------------------------------------------------------------------------
//	GetPasteboardFromDragEvent
//------------------------------------------------------------------------------
//
static PasteboardRef GetPasteboardFromDragEvent(
	EventRef			inEvent )
{
	DragRef				drag;
	PasteboardRef		pasteboard = NULL;
	
	// get the drag pasteboard, if it exists
	require_noerr( GetEventParameter( inEvent, kEventParamDragRef, typeDragRef, NULL, sizeof( DragRef ),
										NULL, &drag ), CantGetDragRefFromEvent );

	GetDragPasteboard( drag, &pasteboard );
	
CantGetDragRefFromEvent:

	return pasteboard;
}

//------------------------------------------------------------------------------
//	CreateFirstFileURLFromDragEvent
//------------------------------------------------------------------------------
//
static CFURLRef CreateFirstFileURLFromDragEvent(
	EventRef			inEvent )
{
	PasteboardRef		pasteboard;
	CFURLRef			fileURL = NULL;
	
	pasteboard = GetPasteboardFromDragEvent( inEvent );
	require( pasteboard != NULL, CantGetDragPasteboard );
	
	fileURL = CreateFileURLFromPasteboard( pasteboard, 1 );
	
CantGetDragPasteboard:

	return fileURL;
}

//------------------------------------------------------------------------------
//	InvalidateImageCache
//------------------------------------------------------------------------------
//
static void
InvalidateImageCache(
	ImageBrowserViewData*	inData )
{
	if ( inData->imageCache != NULL )
	{
		CFRelease( inData->imageCache );
		inData->imageCache = NULL;
	}
	
	if ( inData->imageCacheDisabled != NULL )
	{
		CFRelease( inData->imageCacheDisabled );
		inData->imageCacheDisabled = NULL;
	}
}

//------------------------------------------------------------------------------
//	PreviousImage
//------------------------------------------------------------------------------
//
static void PreviousImage(
	ImageBrowserViewData*	inData )
{
	if ( inData->imageIndex > 0 )
	{
		inData->imageIndex--; // decrement the image index if possible
		
		// if the focused part is no longer available, advance the focus to the next part
		if ( !IsPartAvailable( inData, inData->currentFocusPart ) )
			SetKeyboardFocus( HIViewGetWindow( inData->view ), inData->view, kControlFocusNextPart );
			
		// invalidate due to index change
		verify_noerr( HIViewSetNeedsDisplay( inData->view, true ) );
	}
}

//------------------------------------------------------------------------------
//	NextImage
//------------------------------------------------------------------------------
//
static void NextImage(
	ImageBrowserViewData*	inData )
{
	if ( inData->imageIndex < CFArrayGetCount( inData->imageURLs ) - 1 )
	{
		inData->imageIndex++; // increment the image index if possible
		
		// if the focused part is no longer available, advance the focus to the previous part
		if ( !IsPartAvailable( inData, inData->currentFocusPart ) )
			SetKeyboardFocus( HIViewGetWindow( inData->view ), inData->view, kControlFocusPrevPart );
			
		// invalidate due to index change
		verify_noerr( HIViewSetNeedsDisplay( inData->view, true ) );
	}
}

//------------------------------------------------------------------------------
//	AddImage
//------------------------------------------------------------------------------
//
static void AddImage(
	CFURLRef				inFileURL,
	ImageBrowserViewData*	inData )
{
	//
	// If we already have images in the array, then put our new image after the current image index.
	// If we have no images in the array, then imageIndex is zero, and that's the index for the new image.
	//
	if ( CFArrayGetCount( inData->imageURLs ) > 0 )
		inData->imageIndex++;
	
	CFArrayInsertValueAtIndex( inData->imageURLs, inData->imageIndex, inFileURL );
	
	// invalidate due to index change
	InvalidateImageCache( inData );
	verify_noerr( HIViewSetNeedsDisplay( inData->view, true ) );
}

//------------------------------------------------------------------------------
//	DeleteCurrentImage
//------------------------------------------------------------------------------
//
static void DeleteCurrentImage(
	ImageBrowserViewData*	inData )
{
	if ( CFArrayGetCount( inData->imageURLs ) > 0 )
	{
		CFArrayRemoveValueAtIndex( inData->imageURLs, inData->imageIndex ); 
		if ( inData->imageIndex > 0 )
			inData->imageIndex--;
		
		// if the focused part is no longer available, set focus to the image part
		if ( !IsPartAvailable( inData, inData->currentFocusPart ) )
			SetKeyboardFocus( HIViewGetWindow( inData->view ), inData->view, kImagePart );
		
		// invalidate due to index change
		InvalidateImageCache( inData );
		verify_noerr( HIViewSetNeedsDisplay( inData->view, true ) );
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
static void
ConvertImageToBitmapImage( CGImageRef* ioImage )
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
	bounds.size.width = CGImageGetWidth( *ioImage );
	bounds.size.height = CGImageGetHeight( *ioImage );

	height = (size_t)ceil( bounds.size.height );
	width = (size_t)ceil( bounds.size.width );

	bytesPerRow = ( ( width * 8 * 4 + 7 ) / 8 );
	size = bytesPerRow * height;
	
	ptr = calloc( 1, size );
	require( ptr != NULL, CantAllocateImageTemp );

	colorSpace = CGColorSpaceCreateDeviceRGB();
	context = CGBitmapContextCreate( ptr, width, height, 8, bytesPerRow, colorSpace, kCGImageAlphaPremultipliedFirst );
	require( context != NULL, CouldntCreateContext );
		
	CGContextDrawImage( context, bounds, *ioImage );

	CGContextRelease( context );

	// Create a CGImage from our offscreen.

	provider = CGDataProviderCreateWithData( 0, ptr, size, ReleaseImageBits );
	
	image = CGImageCreate( width, height, 8, 32, bytesPerRow, colorSpace, kCGImageAlphaPremultipliedFirst,
							provider, NULL, false, kCGRenderingIntentDefault );

	CGColorSpaceRelease( colorSpace );
	CGDataProviderRelease( provider );

	CFRelease( *ioImage );
	*ioImage = image;
	
CantAllocateImageTemp:
CouldntCreateContext:
	return;
}

//------------------------------------------------------------------------------
//	ConvertGlobalToLocalPoint
//------------------------------------------------------------------------------
//
static HIPoint* ConvertGlobalToLocalPoint(
	ImageBrowserViewData*	inData,
	const HIPoint*			inPoint,
	HIPoint*				outPoint )
{
	HIPoint					hiPoint = *inPoint;
	Rect					bounds;
	
	check( inData != NULL );
	check( inData->view != NULL );
	check( inPoint != NULL );
	check( outPoint != NULL );
	
	// use HIPointConvert if available
	if ( HIPointConvert != NULL )
	{
		*outPoint = *inPoint;
		HIPointConvert( outPoint, kHICoordSpace72DPIGlobal, NULL, kHICoordSpaceView, inData->view );
	}
	else
	{
		// default
		outPoint->x = outPoint->y = 0;
		
		// convert global to local window coordinates
		GetWindowBounds( HIViewGetWindow( inData->view ), kWindowStructureRgn, &bounds );
		hiPoint.x -= bounds.left;
		hiPoint.y -= bounds.top;
		
		// convert from window to view
		if ( HIViewConvertPoint( &hiPoint, NULL, inData->view ) == noErr )
			*outPoint = hiPoint;
	}
	
	return outPoint;
}

//------------------------------------------------------------------------------
//	ConvertLocalToGlobalPoint
//------------------------------------------------------------------------------
//
static HIPoint* ConvertLocalToGlobalPoint(
	ImageBrowserViewData*	inData,
	const HIPoint*			inPoint,
	HIPoint*				outPoint )
{
	HIPoint					hiPoint = *inPoint;
	
	check( inData != NULL );
	check( inData->view != NULL );
	check( inPoint != NULL );
	check( outPoint != NULL );
	
	// use HIPointConvert if available
	if ( HIPointConvert != NULL )
	{
		*outPoint = *inPoint;
		HIPointConvert( outPoint, kHICoordSpaceView, inData->view, kHICoordSpace72DPIGlobal, NULL );
	}
	else
	{
		// default
		outPoint->x = outPoint->y = 0;
		
		// convert view to window
		if ( HIViewConvertPoint( &hiPoint, inData->view, NULL ) == noErr )
		{
			Rect	bounds;
			
			GetWindowBounds( HIViewGetWindow( inData->view ), kWindowStructureRgn, &bounds );
			
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
static HIViewPartCode GetViewPart(
	ImageBrowserViewData*	inData,
	HIPoint*				inPoint )
{
	HIViewPartCode			part = kDeleteImagePart;

	do // test which part the mouse is within cycling backwards from smallest to largest
	{
		if ( CGRectContainsPoint( GetPartFrame( inData->view, part ), *inPoint ) )
			break;
	}
	while ( part-- > kImagePart );
	
	return part;
}

#pragma mark -
#pragma mark Event Handlers

//------------------------------------------------------------------------------
//	ImageBrowserViewConstruct
//------------------------------------------------------------------------------
//
static OSStatus ImageBrowserViewConstruct(
	EventRef				inEvent )
{
	OSStatus				err = noErr;
	ImageBrowserViewData*	data;
	
	// don't CallNextEventHandler!
	data = (ImageBrowserViewData*)malloc( sizeof( ImageBrowserViewData ) );
	require_action( data != NULL, CantMalloc, err = memFullErr );
	
	verify_noerr( GetEventParameter( inEvent, kEventParamHIObjectInstance, typeHIObjectRef, NULL, sizeof( HIObjectRef ),
								NULL, (HIObjectRef*)&data->view ) );
	
	// set the userData that will be used with all subsequent eventHandler calls
	verify_noerr( SetEventParameter( inEvent, kEventParamHIObjectInstance, typeVoidPtr, sizeof( ImageBrowserViewData* ), &data ) );
	
CantMalloc:
	
	return err;
}

//------------------------------------------------------------------------------
//	ImageBrowserViewDestruct
//------------------------------------------------------------------------------
//
static OSStatus ImageBrowserViewDestruct(
	EventRef				inEvent,
	ImageBrowserViewData*	inData )
{
	// free the instance data
	if ( inData != NULL )
	{
		if ( inData->imageURLs != NULL )
			CFRelease( inData->imageURLs );
		
		if ( inData->trackingArea != NULL )
			verify_noerr( HIViewDisposeTrackingArea( inData->trackingArea ) );
		
		InvalidateImageCache( inData );
		
		free( inData );
	}
	
	return noErr;
}

//------------------------------------------------------------------------------
//	ImageBrowserViewInitialize
//------------------------------------------------------------------------------
//
static OSStatus ImageBrowserViewInitialize(
	EventHandlerCallRef		inCallRef,
	EventRef				inEvent,
	ImageBrowserViewData*	inData )
{
	OSStatus				err = noErr;
	HIArchiveRef			decoder = NULL;
	CFArrayRef				imageURLs = NULL;
	CFStringRef				imagePath;
	
	// allow the superclass to initialize itself
	err = CallNextEventHandler( inCallRef, inEvent );
	require_noerr( err, CantInitializeSuperclass );
	
	// look for the optional HIArchive decoder in the event
	GetEventParameter( inEvent, kEventParamHIArchive, typeCFTypeRef, NULL, sizeof( HIArchiveRef ), NULL, &decoder );
	
	// defult to the first image
	inData->imageIndex = 0;
	
	if ( decoder != NULL ) // if we're unarchiving, initialize from the decoder ...
	{
		// decode our persistant state
		err = HIArchiveCopyDecodedCFType( decoder, kImageURLsArchiveKey, (CFTypeRef*)&imageURLs );
		require_noerr( err, CantDecodeImageURLs );
		
		err = HIArchiveDecodeNumber( decoder, kImageIndexArchiveKey, kCFNumberCFIndexType, &(inData->imageIndex) );
		require_noerr( err, CantDecodeImageIndex );
	}
	else // ... otherwise initialize as normal
	{
		// extract the optional image URLs from the initialization event
		GetEventParameter( inEvent, kEventParamImageURLArray, typeCFTypeRef, NULL, sizeof( CFArrayRef ), NULL, &imageURLs );
	}
	
	// create a mutable version of the image URLs array for later modification
	if ( imageURLs != NULL )
		inData->imageURLs = CFArrayCreateMutableCopy( kCFAllocatorDefault, 0, imageURLs );
	else
		inData->imageURLs = CFArrayCreateMutable( kCFAllocatorDefault, 0, &kCFTypeArrayCallBacks );
	
	require_action( inData->imageURLs != NULL, CantCreateImageURLArray, err = coreFoundationUnknownErr );
	
	// extract the optional image path from the initialization event
	if ( GetEventParameter( inEvent, kEventParamImageURL, typeCFStringRef, NULL, sizeof( CFStringRef ), NULL, &imagePath ) == noErr )
	{
		CFURLRef	imageURL = CFURLCreateWithFileSystemPath( kCFAllocatorDefault, imagePath, kCFURLPOSIXPathStyle, false );

		require_action( imageURL != NULL, CantCreateImageURL, err = coreFoundationUnknownErr );
		
		CFArrayAppendValue( inData->imageURLs, imageURL );		
		CFRelease( imageURL );
	}
	
	// we're not tracking any drags by default
	inData->trackingDrag = false;
	
	// install a tracking area on the entire view to implement a mouse over effect
	verify_noerr( HIViewNewTrackingArea( inData->view, NULL, 0, &(inData->trackingArea) ) );
	
	// the mouse is not in the view by default
	inData->mouseInView = false;
	
	// default to a non-focused state
	inData->currentFocusPart = kHIViewFocusNoPart;
	
	// we have no cached image
	inData->imageCache = NULL;
	inData->imageCacheDisabled = NULL;
	inData->imageCacheIndex = -1;
	
	// allow the view to be archived
	verify_noerr( HIObjectSetArchivingIgnored( (HIObjectRef)inData->view, false ) );
	
	// let HIToolbox know that we're an opaque view for performance
	verify_noerr( HIViewChangeFeatures( inData->view, kHIViewFeatureIsOpaque, 0 ) );
	
CantCreateImageURL:
CantCreateImageURLArray:
CantDecodeImageIndex:
CantDecodeImageURLs:
InitBoundsParameterMissing:
CantInitializeSuperclass:
	
	return err;
}

//------------------------------------------------------------------------------
//	ImageBrowserViewEncode
//------------------------------------------------------------------------------
//
static OSStatus ImageBrowserViewEncode(
	EventHandlerCallRef		inCallRef,
	EventRef				inEvent,
	ImageBrowserViewData*	inData )
{
	OSStatus				err;
	HIArchiveRef			encoder;
	
	// allow the superclass to encode itself
	err = CallNextEventHandler( inCallRef, inEvent );
	require_noerr( err, CantEncodeSuperclass );
	
	// get the HIArchive encoder from the event
	verify_noerr( GetEventParameter( inEvent, kEventParamHIArchive, typeCFTypeRef, NULL, sizeof( HIArchiveRef ), NULL, &encoder ) );
	
	// encode our persistant state
	err = HIArchiveEncodeCFType( encoder, kImageURLsArchiveKey, inData->imageURLs );
	require_noerr( err, CantEncodeImageURLs );
	
	err = HIArchiveEncodeNumber( encoder, kImageIndexArchiveKey, kCFNumberCFIndexType, &(inData->imageIndex) );
	require_noerr( err, CantEncodeImageIndex );
	
CantEncodeImageIndex:
CantEncodeImageURLs:
CantEncodeSuperclass:
	
	return err;
}

//------------------------------------------------------------------------------
//	ImageBrowserViewDraw
//------------------------------------------------------------------------------
//
static OSStatus ImageBrowserViewDraw(
	EventRef				inEvent,
	ImageBrowserViewData*	inData )
{
	OSStatus				err = noErr;
	HIRect					bounds;
	CGContextRef			context;
	CFIndex					imageURLCount = CFArrayGetCount( inData->imageURLs );
	
	verify_noerr( GetEventParameter( inEvent, kEventParamCGContextRef, typeCGContextRef, NULL, sizeof( CGContextRef ),
								NULL, &context ) );

	verify_noerr( HIViewGetBounds( inData->view, &bounds ) );
	
	// draw a black background/frame
	CGContextSetRGBFillColor( context, 0, 0, 0, 1 );
	CGContextFillRect( context, bounds );
	
	// draw the current image if it exists
	if ( imageURLCount > 0 )
	{
		CGImageSourceRef	imageSource;
		HIRect				insetBounds = CGRectInset( bounds, kImageMargin, kImageMargin );
		HIRect				imageBounds = insetBounds;
		float				scale;
		CGImageRef			image;
		
		if ( inData->imageCacheIndex != inData->imageIndex )
			InvalidateImageCache( inData );
		
		// don't rerender the image if the cache is still valid
		if ( inData->imageCache == NULL )
		{
			CFURLRef imageURL = (CFURLRef)CFArrayGetValueAtIndex( inData->imageURLs, inData->imageIndex );
			
			// dynamically load and cache the image from the URL with ImageIO as necessary
			imageSource = CGImageSourceCreateWithURL( imageURL, NULL );
			require_action( imageSource != NULL, CantCreateImageSource, err = coreFoundationUnknownErr );
			
			inData->imageCache = CGImageSourceCreateImageAtIndex( imageSource, 0, NULL );
			CFRelease( imageSource );
			require_action( inData->imageCache != NULL, CantCreateImage, err = coreFoundationUnknownErr );
			
			// cache the image data in a bitmap context to improve drawing performance
			ConvertImageToBitmapImage( &inData->imageCache );
			
			// create a disabled version of the image
			HICreateTransformedCGImage( inData->imageCache, kHITransformDisabled, &inData->imageCacheDisabled );
			
			inData->imageCacheIndex = inData->imageIndex;
		}
		
		imageBounds.size.width = CGImageGetWidth( inData->imageCache );
		imageBounds.size.height = CGImageGetHeight( inData->imageCache );
		
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
		if ( !(HIViewIsEnabled( inData->view, NULL ) && HIViewIsActive( inData->view, NULL )) &&
				inData->imageCacheDisabled != NULL )
			image = inData->imageCacheDisabled;
		else
			image = inData->imageCache;
		
		// draw the image
		verify_noerr( HIViewDrawCGImage( context, &imageBounds, image ) );
		
CantCreateImageSource:
CantCreateImage:
		;
	}
		
	// draw a drag hilight if necessary
	if ( inData->trackingDrag )
	{
		RGBColor dragColor;
		
		// draw with the appropriate theme brush
		verify_noerr( GetThemeBrushAsColor( kThemeBrushDragHilite, 32, true, &dragColor ) );
		
		CGContextSetRGBStrokeColor( context, (float)dragColor.red/65535, (float)dragColor.green/65535,
									(float)dragColor.blue/65535, 1 );
		CGContextSetLineWidth( context, 5 );
		CGContextStrokeRect( context, GetPartFrame( inData->view, kImagePart ) );
	}
	
	// draw the currently focused area if necessary
	if ( inData->currentFocusPart != kHIViewFocusNoPart && HIViewIsEnabled( inData->view, NULL ) &&
			HIViewIsActive( inData->view, NULL )  )
	{
		RGBColor	hiliteColor;
		HIRect		focusFrame = GetPartFrame( inData->view, inData->currentFocusPart );
		
		// draw with the appropriate theme brush
		verify_noerr( GetThemeBrushAsColor( kThemeBrushFocusHighlight, 32, true, &hiliteColor ) );
		
		CGContextSetRGBStrokeColor( context, (float)hiliteColor.red/65535, (float)hiliteColor.green/65535,
									(float)hiliteColor.blue/65535, 1 );
		CGContextSetLineWidth( context, (inData->currentFocusPart == kImagePart) ? 5 : 3 ); // thicker for the image part
		
		CGContextStrokeRect( context, focusFrame );
	}
	
	// only draw the browse buttons when we're enablde, active and the mouse button is inside the view or they're focused
	if ( HIViewIsEnabled( inData->view, NULL ) && HIViewIsActive( inData->view, NULL ) &&
			(inData->mouseInView || inData->currentFocusPart == kBrowseBackPart ||
			 inData->currentFocusPart == kBrowseForwardPart || inData->currentFocusPart == kDeleteImagePart) )
	{
		HISize			shadowOffset = { 0, -2 };
		HIRect			frame;
		HIViewPartCode	hilite = GetControlHilite( inData->view );
		
		// add a little shadow to the buttons for effect
		CGContextSetShadow( context, shadowOffset, 2 );
		
		// draw the browse back button
		frame = GetPartFrame( inData->view, kBrowseBackPart );
		
		CGContextBeginPath( context );
		CGContextMoveToPoint( context, frame.origin.x + frame.size.width - kButtonPad, frame.origin.y );
		CGContextAddLineToPoint( context, frame.origin.x, frame.origin.y + kButtonSize/2 );
		CGContextAddLineToPoint( context, frame.origin.x + frame.size.width - kButtonPad, frame.origin.y + kButtonSize );
		CGContextAddLineToPoint( context, frame.origin.x + frame.size.width - kButtonPad, frame.origin.y );
		CGContextClosePath( context );
		
		// display hilited or disabled if necessary
		if ( hilite == kBrowseBackPart )
			CGContextSetRGBFillColor( context, .66, .66, .66, 1 );
		else
			CGContextSetRGBFillColor( context, 1, 1, 1, IsPartAvailable( inData, kBrowseBackPart ) ? 1 : .33 );
		CGContextFillPath( context );
		
		// draw the browse forward button (appearing disabled if necessary)
		frame = GetPartFrame( inData->view, kBrowseForwardPart );
		
		CGContextBeginPath( context );
		CGContextMoveToPoint( context, frame.origin.x + kButtonPad, frame.origin.y );
		CGContextAddLineToPoint( context, frame.origin.x + frame.size.width, frame.origin.y + kButtonSize/2 );
		CGContextAddLineToPoint( context, frame.origin.x + kButtonPad, frame.origin.y + kButtonSize );
		CGContextAddLineToPoint( context, frame.origin.x + kButtonPad, frame.origin.y );
		CGContextClosePath( context );
		
		// display hilited or disabled if necessary
		if ( hilite == kBrowseForwardPart )
			CGContextSetRGBFillColor( context, .66, .66, .66, 1 );
		else
			CGContextSetRGBFillColor( context, 1, 1, 1, IsPartAvailable( inData, kBrowseForwardPart ) ? 1 : .33 );
		CGContextFillPath( context );
		
		// draw the delete image button (appearing disabled if necessary)
		frame = CGRectInset( GetPartFrame( inData->view, kDeleteImagePart ), 2, 2 );
		
		CGContextBeginPath( context );
		CGContextMoveToPoint( context, frame.origin.x, frame.origin.y );
		CGContextAddLineToPoint( context, frame.origin.x + frame.size.width, frame.origin.y + frame.size.height );
		CGContextMoveToPoint( context, frame.origin.x, frame.origin.y + frame.size.height );
		CGContextAddLineToPoint( context, frame.origin.x + frame.size.width, frame.origin.y );
		CGContextClosePath( context );
		
		// display hilited or disabled if necessary
		if ( hilite == kDeleteImagePart )
			CGContextSetRGBStrokeColor( context, .66, .66, .66, 1 );
		else
			CGContextSetRGBStrokeColor( context, 1, 1, 1, IsPartAvailable( inData, kDeleteImagePart ) ? 1 : .33 );
			
		CGContextSetLineWidth( context, 3 );
		CGContextStrokePath( context );
	}
	
	return err;
}

//------------------------------------------------------------------------------
//	ImageBrowserViewHitTest
//------------------------------------------------------------------------------
//
static OSStatus ImageBrowserViewHitTest(
	EventRef				inEvent,
	ImageBrowserViewData*	inData )
{
	HIViewPartCode			part;
	HIPoint					mouse;
	
	// get the mouse location in view coordinates
	verify_noerr( GetEventParameter( inEvent, kEventParamMouseLocation, typeHIPoint, NULL, sizeof( HIPoint ), NULL, &mouse ) );
	
	// find the part that was hit, and make sure that part is currently available
	part = GetViewPart( inData, &mouse );
	if ( !IsPartAvailable( inData, part ) )
		part = kControlNoPart;
	
	// return the part code so we receive track messages
	return SetEventParameter( inEvent, kEventParamControlPart, typeControlPartCode, sizeof( HIViewPartCode ), &part );
}

//------------------------------------------------------------------------------
//	ImageBrowserViewTrack
//------------------------------------------------------------------------------
//
static OSStatus ImageBrowserViewTrack(
	EventRef				inEvent,
	ImageBrowserViewData*	inData )
{
	OSStatus				err = eventNotHandledErr;
	HIPoint					mouse;
	HIViewPartCode			part;
	
	// get the mouse location in view coordinates
	verify_noerr( GetEventParameter( inEvent, kEventParamMouseLocation, typeHIPoint, NULL, sizeof( HIPoint ), NULL, &mouse ) );
	
	part = GetViewPart( inData, &mouse );
	
	if ( part == kImagePart )
	{
		err = noErr;
	}
	
	// return the part that was tracked
	verify_noerr( SetEventParameter( inEvent, kEventParamControlPart, typeControlPartCode, sizeof( HIViewPartCode ), &part ) );
	
	return err;
}

//------------------------------------------------------------------------------
//	ImageBrowserViewHit
//------------------------------------------------------------------------------
//
static OSStatus ImageBrowserViewHit(
	EventRef				inEvent,
	ImageBrowserViewData*	inData )
{
	OSStatus				err = noErr;
	HIViewPartCode			part;
	
	verify_noerr( GetEventParameter( inEvent, kEventParamControlPart, typeControlPartCode, NULL, sizeof( part ), NULL, &part ) );
	
	// don't handle the Hit event if this part isn't currently available
	if ( !IsPartAvailable( inData, part ) )
		return eventNotHandledErr;
		
	switch ( part )
	{
		case kBrowseBackPart:
			PreviousImage( inData );
		break;
		
		case kBrowseForwardPart:
			NextImage( inData );
		break;
		
		case kDeleteImagePart:
			DeleteCurrentImage( inData );
		break;
		
		default:
			err = eventNotHandledErr;
		break;
	}
	
	return err;
}

//------------------------------------------------------------------------------
//	ImageBrowserViewSetFocusPart
//------------------------------------------------------------------------------
//
static OSStatus ImageBrowserViewSetFocusPart(
	EventRef				inEvent,
	ImageBrowserViewData*	inData )
{
	OSStatus				err = noErr;
	Boolean					focusAll = false;
	HIViewPartCode			inPart;
	HIViewPartCode			oldFocusPart = inData->currentFocusPart;
	HIViewPartCode			newFocusPart = oldFocusPart;
	
	// the FocusEverything parameter may not always be present; default to false if it's missing
	(void)GetEventParameter( inEvent, kEventParamControlFocusEverything, typeBoolean, NULL, sizeof( Boolean ), NULL, &focusAll );
	
	(void)GetEventParameter( inEvent, kEventParamControlPart, typeControlPartCode, NULL, sizeof( HIViewPartCode ), NULL, &inPart );
	
	switch ( inPart )
	{
		case kControlFocusNextPart:
			// focus the next part, if there is one that is available
			if ( !focusAll )
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
					if ( IsPartAvailable( inData, newFocusPart ) )
						break;
				}
				if ( newFocusPart > kDeleteImagePart )
					newFocusPart = kHIViewFocusNoPart;
			}
		break;
		
		case kControlFocusPrevPart:
			if ( !focusAll )
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
					if ( IsPartAvailable( inData, newFocusPart ) )
						break;
				}
				if ( newFocusPart < kImagePart )
					newFocusPart = kHIViewFocusNoPart;
			}
		break;
		
		default:
			if ( IsPartAvailable( inData, inPart ) )
				newFocusPart = inPart;
			else
				newFocusPart = inPart + (inPart - oldFocusPart); // request the next part if necessary
		break;
	}
		
	inData->currentFocusPart = newFocusPart;
	verify_noerr( SetEventParameter( inEvent, kEventParamControlPart, typeControlPartCode, sizeof( newFocusPart ), &newFocusPart ) );
	
	// invalidate ourselves if the focus changed
	if ( inData->currentFocusPart != oldFocusPart )
	{
		// if focus was changing to or from the image part, invalidate the entire view ...
		if ( inData->currentFocusPart == kImagePart || oldFocusPart == kImagePart )
		{
			verify_noerr( HIViewSetNeedsDisplay( inData->view, true ) );
		}
		else // ... otherwise, only invalidate the subparts for better performance
		{
			HIRect focusUpdateFrame = CGRectUnion( GetPartFrame( inData->view, (HIViewPartCode)kBrowseBackPart ),
													GetPartFrame( inData->view, (HIViewPartCode)kDeleteImagePart ) );
			
			// make sure to invalidate the entire focus area
			focusUpdateFrame = CGRectInset( focusUpdateFrame, -2, -2 );
			
			verify_noerr( HIViewSetNeedsDisplayInRect( inData->view, &focusUpdateFrame, true ) );
		}
	}
	
CantGetFocusAllParameter:
	
	return err;
}

//------------------------------------------------------------------------------
//	ImageBrowserViewGetFocusPart
//------------------------------------------------------------------------------
//
static OSStatus ImageBrowserViewGetFocusPart(
	EventRef				inEvent,
	ImageBrowserViewData*	inData )
{
	return SetEventParameter( inEvent, kEventParamControlPart, typeControlPartCode, sizeof( HIViewPartCode ),
								&(inData->currentFocusPart) );
}

//------------------------------------------------------------------------------
//	ImageBrowserViewChangeActivation
//------------------------------------------------------------------------------
//
static OSStatus ImageBrowserViewChangeActivation(
	Boolean					inActivate,
	ImageBrowserViewData*	inData )
{
	if ( inActivate )
	{
		Point mouse;
		GetGlobalMouse( &mouse );
		
		HIPoint pt = { mouse.h, mouse.v };
		ConvertGlobalToLocalPoint( inData, &pt, &pt );
		
		if ( GetViewPart( inData, &pt ) != 0 )
			inData->mouseInView = true;
	}
	else if ( inData->mouseInView )
	{
		inData->mouseInView = false;
	}
	
	// always invalidate the entire view to draw the en/disabled state
	verify_noerr( HIViewSetNeedsDisplay( inData->view, true ) );
		
	return noErr;
}

//------------------------------------------------------------------------------
//	ImageBrowserViewHiliteChanged
//------------------------------------------------------------------------------
//
static OSStatus ImageBrowserViewHiliteChanged(
	EventRef				inEvent,
	ImageBrowserViewData*	inData )

{
	HIViewPartCode previousHilitePart, currentHilitePart;
	
	verify_noerr( GetEventParameter( inEvent, kEventParamControlPreviousPart, typeControlPartCode, NULL, sizeof( HIViewPartCode ),
										NULL, &previousHilitePart ) );
	
	verify_noerr( GetEventParameter( inEvent, kEventParamControlCurrentPart, typeControlPartCode, NULL, sizeof( HIViewPartCode ),
										NULL, &currentHilitePart ) );

	// get bounds of old and new hilited parts, and also invalidate shadow and focus
	HIRect previousHiliteFrame = CGRectInset( GetPartFrame( inData->view, (HIViewPartCode)previousHilitePart ), -2, -2 );
	HIRect currentHiliteFrame = CGRectInset( GetPartFrame( inData->view, (HIViewPartCode)currentHilitePart ), -2, -2 );
	
	if ( previousHilitePart != kHIViewNoPart )
		verify_noerr( HIViewSetNeedsDisplayInRect( inData->view, &previousHiliteFrame, true ) );
	
	if ( currentHilitePart != kHIViewNoPart )
		verify_noerr( HIViewSetNeedsDisplayInRect( inData->view, &currentHiliteFrame, true ) );
	
	return noErr;
}

//------------------------------------------------------------------------------
//	ImageBrowserViewDragEnter
//------------------------------------------------------------------------------
//
static OSStatus ImageBrowserViewDragEnter(
	EventRef				inEvent,
	ImageBrowserViewData*	inData )
{
	CFURLRef				fileURL = CreateFirstFileURLFromDragEvent( inEvent );
	Boolean					wouldAccept = (fileURL != NULL); // accept the drag if it contains an image file URL
	
	// let the HIView drag apparatus know that we want to continue receiving drag messages
	verify_noerr( SetEventParameter( inEvent, kEventParamControlWouldAcceptDrop, typeBoolean, sizeof( Boolean ), &wouldAccept ) );
	
	if ( wouldAccept )
	{
		// note that we're tracking an acceptable drag
		inData->trackingDrag = true;
		
		// invalidate ourseleves so we can display drag feedback
		verify_noerr( HIViewSetNeedsDisplay( inData->view, true ) );
	}
	
	if ( fileURL != NULL )
		CFRelease( fileURL );
	
	return noErr;
}

//------------------------------------------------------------------------------
//	ImageBrowserViewDragLeave
//------------------------------------------------------------------------------
//
static OSStatus ImageBrowserViewDragLeave(
	EventRef				inEvent,
	ImageBrowserViewData*	inData )
{
	// note that we are no longer tracking a drag
	inData->trackingDrag = false;
	
	// invalidate ourseleves so we can turn off our drag feedback
	verify_noerr( HIViewSetNeedsDisplay( inData->view, true ) );
	
	return noErr;
}

//------------------------------------------------------------------------------
//	ImageBrowserViewDragReceive
//------------------------------------------------------------------------------
//
static OSStatus ImageBrowserViewDragReceive(
	EventRef				inEvent,
	ImageBrowserViewData*	inData )
{
	PasteboardRef			pasteboard = GetPasteboardFromDragEvent( inEvent );
	
	if ( pasteboard != NULL )
	{
		ItemCount		itemCount;
		CFIndex			itemIndex;
		
		verify_noerr( PasteboardGetItemCount( pasteboard, &itemCount ) );
		
		for ( itemIndex = 1; itemIndex <= itemCount; itemIndex++ )
		{
			CFURLRef	fileURL = CreateFileURLFromPasteboard( pasteboard, itemIndex );
			
			// insert the image file URL after the current index and increment
			if ( fileURL != NULL )
			{
				AddImage( fileURL, inData );
				CFRelease( fileURL );
			}
		}
	}
	
	return noErr;
}

//------------------------------------------------------------------------------
//	ImageBrowserViewTrackingAreaEvent
//------------------------------------------------------------------------------
//
static OSStatus ImageBrowserViewTrackingAreaEvent(
	Boolean					inEnteredArea,
	ImageBrowserViewData*	inData )
{
	// support rollover subpart hide/show
	if ( HIViewIsEnabled( inData->view, NULL ) && HIViewIsActive( inData->view, NULL ) )
	{
		inData->mouseInView = inEnteredArea;
		
		HIRect trackingUpdateFrame = CGRectUnion( GetPartFrame( inData->view, (HIViewPartCode)kBrowseBackPart ),
								GetPartFrame( inData->view, (HIViewPartCode)kDeleteImagePart ) );
								
		// also invalidate shadow and focus areas
		trackingUpdateFrame = CGRectInset( trackingUpdateFrame, -2, -2 );
		
		verify_noerr( HIViewSetNeedsDisplayInRect( inData->view, &trackingUpdateFrame, true ) );
	}
	
	return noErr;
}

//------------------------------------------------------------------------------
//	ImageBrowserViewTextInput
//------------------------------------------------------------------------------
//
static OSStatus ImageBrowserViewTextInput(
	EventRef				inEvent,
	ImageBrowserViewData*	inData )
{
	OSStatus	err = noErr;
	UniChar		uniChar;
	
	err = GetEventParameter( inEvent, kEventParamTextInputSendText, typeUnicodeText, NULL, sizeof( UniChar ), NULL, &uniChar );
	require_noerr( err, CantGetTextInputParameter );
	
	switch ( uniChar )
	{
		case kSpaceCharCode:
			if ( inData->currentFocusPart == kBrowseBackPart || inData->currentFocusPart == kBrowseForwardPart ||
					inData->currentFocusPart == kDeleteImagePart )
				err = HIViewSimulateClick( inData->view, inData->currentFocusPart, 0, NULL );
			else
				err = eventNotHandledErr;  // otherwise let someone else deal with the keystroke
		break;
		
		case kBackspaceCharCode:
		case kDeleteCharCode:
		case kClearCharCode:
			if ( inData->currentFocusPart == kImagePart )
				err = HIViewSimulateClick( inData->view, kDeleteImagePart, 0, NULL );
		break;
		
		case kLeftArrowCharCode:
		case kRightArrowCharCode:
			if ( inData->currentFocusPart == kImagePart )
			{
				if ( uniChar == kLeftArrowCharCode )
					PreviousImage( inData );
				else
					NextImage( inData );
			}
			else
			{
				Boolean			foundFocusablePart = false;
				HIViewPartCode	focusPart = inData->currentFocusPart;
				
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
					if ( IsPartAvailable( inData, focusPart ) )
					{
						foundFocusablePart = true;
						break;
					}
					
					// check to see if we've cycled all the way around looking for an enabled part
					if ( focusPart == inData->currentFocusPart )
					{
						focusPart = kImagePart;
						foundFocusablePart = true;
						break;
					}
				}
				
				err = SetKeyboardFocus( HIViewGetWindow( inData->view ), inData->view, focusPart );
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
//	ImageBrowserAccessibilityGetChildAtPoint
//------------------------------------------------------------------------------
//
static OSStatus ImageBrowserAccessibilityGetChildAtPoint(
	AXUIElementRef			inElement,
	UInt64					inIdentifier,
	EventHandlerCallRef		inCallRef,
	EventRef				inEvent,
	ImageBrowserViewData*	inData )
{
	OSStatus				result = eventNotHandledErr;
	CFTypeRef				child = NULL;
	HIPoint					where;
	HIViewPartCode			part;

	check( inData != NULL );
	
	// Only handle events for the basic target/identifier pair. Anything else is custom.
	require_quiet( inIdentifier == 0, NotTheBasicElement );
	
	verify_noerr( GetEventParameter( inEvent, kEventParamMouseLocation, typeHIPoint,
					NULL, sizeof( where ), NULL, &where ) );

	// convert to view coordinates
	ConvertGlobalToLocalPoint( inData, &where, &where );

	// get the part that is at this point
	part = GetViewPart( inData, &where );
	
	// if the part is the image part, we consider that to be "the control" for
	// purposes of accessibility
	if ( part == kImagePart )
		part = kControlNoPart;

	if ( part != kControlNoPart )
	{
		// create a AXUIElementRef that represents this part
		child = AXUIElementCreateWithHIObjectAndIdentifier( (HIObjectRef)inData->view, part );
		require_action( child != NULL, CantCreateChild, result = memFullErr );
		
		result = SetEventParameter( inEvent, kEventParamAccessibleChild, typeCFTypeRef,
						sizeof( child ), &child );
		CFRelease( child );
	}
		
CantCreateChild:
NotTheBasicElement:

	return result;
}

//------------------------------------------------------------------------------
//	ImageBrowserAccessibilityGetFocusedChild
//------------------------------------------------------------------------------
//
static OSStatus ImageBrowserAccessibilityGetFocusedChild(
	AXUIElementRef			inElement,
	UInt64					inIdentifier,
	EventHandlerCallRef		inCallRef,
	EventRef				inEvent,
	ImageBrowserViewData*	inData )
{
	OSStatus				result = eventNotHandledErr;
	CFTypeRef				child = NULL;
	HIViewPartCode			part;

	check( inData != NULL );
	
	part = inData->currentFocusPart;
	
	// Only handle events for the basic target/identifier pair. Anything else is custom.
	require_quiet( inIdentifier == 0, NotTheBasicElement );
	
	// for the purposes of accessibility, the image part is the control
	if ( part == kImagePart )
		part = kControlNoPart;
	
	if ( inData->currentFocusPart != kControlNoPart )
	{
		child = AXUIElementCreateWithHIObjectAndIdentifier( (HIObjectRef)inData->view, inData->currentFocusPart );
		require_action( child != NULL, CantCreateChild, result = memFullErr );
		
		result = SetEventParameter( inEvent, kEventParamAccessibleChild, typeCFTypeRef,
						sizeof( child ), &child );
		CFRelease( child );
	}
	
CantCreateChild:
NotTheBasicElement:

	return result;
}

//------------------------------------------------------------------------------
//	ImageBrowserAccessibilityGetAllAttributeNames
//------------------------------------------------------------------------------
//
static OSStatus ImageBrowserAccessibilityGetAllAttributeNames(
	AXUIElementRef			inElement,
	UInt64					inIdentifier,
	EventHandlerCallRef		inCallRef,
	EventRef				inEvent,
	ImageBrowserViewData*	inData )
{
	OSStatus				result = eventNotHandledErr;
	CFMutableArrayRef		namesArray;
	
	verify_noerr( GetEventParameter( inEvent, kEventParamAccessibleAttributeNames,
				typeCFMutableArrayRef, NULL, sizeof( namesArray ), NULL, &namesArray ) );
	
	// If this request is for the entire control
	if ( inIdentifier == 0 )
	{
		// Have the base class report its attributes and report its error
		result = CallNextEventHandler( inCallRef, inEvent );
		
		CFArrayAppendValue( namesArray, kAXFocusedAttribute );
		CFArrayAppendValue( namesArray, kAXChildrenAttribute );
	}
	else
	{
		CFArrayAppendValue( namesArray, kAXFocusedAttribute );
		CFArrayAppendValue( namesArray, kAXEnabledAttribute );
		CFArrayAppendValue( namesArray, kAXSizeAttribute );
		CFArrayAppendValue( namesArray, kAXPositionAttribute );
	}
	
	return result;
}

//------------------------------------------------------------------------------
//	ImageBrowserAccessibilityGetNamedAttribute
//------------------------------------------------------------------------------
//
static OSStatus ImageBrowserAccessibilityGetNamedAttribute(
	AXUIElementRef			inElement,
	UInt64					inIdentifier,
	EventHandlerCallRef		inCallRef,
	EventRef				inEvent,
	ImageBrowserViewData*	inData )
{
	OSStatus				result = eventNotHandledErr;
	CFStringRef				attribute;

	check( inData != NULL );
	
	verify_noerr( GetEventParameter( inEvent, kEventParamAccessibleAttributeName, typeCFStringRef,
				NULL, sizeof( attribute ), NULL, &attribute ) );

	if ( inIdentifier == 0 )
	{
		if ( CFStringCompare( attribute, kAXRoleAttribute, 0 ) == kCFCompareEqualTo )
		{
			CFStringRef		role = CFSTR( "ImageBrowser" );
			
			result = SetEventParameter( inEvent, kEventParamAccessibleAttributeValue,
							typeCFStringRef, sizeof( role ), &role );
		}
		else if ( CFStringCompare( attribute, kAXRoleDescriptionAttribute, 0 ) == kCFCompareEqualTo )
		{
			CFStringRef		roleDescription = CFSTR( "Image Browser" );
			
			result = SetEventParameter( inEvent, kEventParamAccessibleAttributeValue,
							typeCFStringRef, sizeof( roleDescription ), &roleDescription );
		}
		else if ( CFStringCompare( attribute, kAXFocusedAttribute, 0 ) == kCFCompareEqualTo )
		{
			Boolean	focused = inData->currentFocusPart == (HIViewPartCode)kImagePart;
			
			result = SetEventParameter( inEvent, kEventParamAccessibleAttributeValue,
							typeBoolean, sizeof( focused ), &focused );
		}
		else if ( CFStringCompare( attribute, kAXChildrenAttribute, 0 ) == kCFCompareEqualTo )
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
			child = AXUIElementCreateWithHIObjectAndIdentifier( (HIObjectRef)inData->view, kBrowseBackPart );
			require_action( child != NULL, CantCreateChild, result = memFullErr );
			CFArrayAppendValue( array, child );
			CFRelease( child );
			
			child = AXUIElementCreateWithHIObjectAndIdentifier( (HIObjectRef)inData->view, kBrowseForwardPart );
			require_action( child != NULL, CantCreateChild, result = memFullErr );
			CFArrayAppendValue( array, child );
			CFRelease( child );
			
			child = AXUIElementCreateWithHIObjectAndIdentifier( (HIObjectRef)inData->view, kDeleteImagePart );
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
		if ( CFStringCompare( attribute, kAXRoleAttribute, 0 ) == kCFCompareEqualTo )
		{
			CFStringRef	role = kAXButtonRole;
			
			result = SetEventParameter( inEvent, kEventParamAccessibleAttributeValue,
						typeCFStringRef, sizeof( role ), &role );
		}
		else if ( CFStringCompare( attribute, kAXRoleDescriptionAttribute, 0 ) == kCFCompareEqualTo )
		{
			CFStringRef	description = HICopyAccessibilityRoleDescription( kAXButtonRole, NULL );
			if ( description != NULL )
			{
				result = SetEventParameter( inEvent, kEventParamAccessibleAttributeValue,
							typeCFStringRef, sizeof( description ), &description );
							
				CFRelease( description );
			}
		}
		else if ( CFStringCompare( attribute, kAXFocusedAttribute, 0 ) == kCFCompareEqualTo )
		{
			Boolean	focused = inData->currentFocusPart == (HIViewPartCode)inIdentifier;
			
			result = SetEventParameter( inEvent, kEventParamAccessibleAttributeValue,
							typeBoolean, sizeof( focused ), &focused );
		}
		else if ( CFStringCompare( attribute, kAXEnabledAttribute, 0 ) == kCFCompareEqualTo )
		{
			Boolean	enabled = IsPartAvailable( inData, (HIViewPartCode)inIdentifier );
			
			result = SetEventParameter( inEvent, kEventParamAccessibleAttributeValue,
							typeBoolean, sizeof( enabled ), &enabled );
		}
		else if ( CFStringCompare( attribute, kAXParentAttribute, 0 ) == kCFCompareEqualTo )
		{
			AXUIElementRef	parent = AXUIElementCreateWithHIObjectAndIdentifier( (HIObjectRef)inData->view, 0 );
			
			require_action( parent != NULL, CantCreateParent, result = memFullErr );
			
			result = SetEventParameter( inEvent, kEventParamAccessibleAttributeValue,
							typeCFTypeRef, sizeof( parent ), &parent );
			CFRelease( parent );
		}
		else if ( CFStringCompare( attribute, kAXSizeAttribute, 0 ) == kCFCompareEqualTo )
		{
			HIRect	frame = GetPartFrame( inData->view, (HIViewPartCode)inIdentifier );
			
			result = SetEventParameter( inEvent, kEventParamAccessibleAttributeValue,
							typeHISize, sizeof( frame.size ), &frame.size );
		}
		else if ( CFStringCompare( attribute, kAXPositionAttribute, 0 ) == kCFCompareEqualTo )
		{
			HIRect	frame = GetPartFrame( inData->view, (HIViewPartCode)inIdentifier );
			
			ConvertLocalToGlobalPoint( inData, &frame.origin, &frame.origin );
			
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
//	ImageBrowserAccessibilityGetAllActionNames
//------------------------------------------------------------------------------
//
static OSStatus ImageBrowserAccessibilityGetAllActionNames(
	AXUIElementRef			inElement,
	UInt64					inIdentifier,
	EventHandlerCallRef		inCallRef,
	EventRef				inEvent,
	ImageBrowserViewData*	inData )
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
		CFMutableArrayRef		actionArray;
		
		// the buttons handle the pressed action
		verify_noerr( GetEventParameter( inEvent, kEventParamAccessibleActionNames,
				typeCFMutableArrayRef, NULL, sizeof( actionArray ), NULL, &actionArray ) );
		
		CFArrayAppendValue( actionArray, kAXPressAction );
		
		result = noErr;
	}
	
	return result;
}

//------------------------------------------------------------------------------
//	ImageBrowserAccessibilityPerformNamedAction
//------------------------------------------------------------------------------
//
static OSStatus ImageBrowserAccessibilityPerformNamedAction(
	AXUIElementRef			inElement,
	UInt64					inIdentifier,
	EventHandlerCallRef		inCallRef,
	EventRef				inEvent,
	ImageBrowserViewData*	inData )
{
	OSStatus				result = eventNotHandledErr;
	
	// handle the action for the children attributes
	if ( inIdentifier != 0 )
	{
		CFStringRef		action;
	
		check( inData != NULL );
	
		verify_noerr( GetEventParameter( inEvent, kEventParamAccessibleActionName,
					typeCFStringRef, NULL, sizeof( action ), NULL, &action ) );
		
		if ( CFStringCompare( action, kAXPressAction, 0 ) == kCFCompareEqualTo )
		{
			// Simulate a click in the part
			if ( IsPartAvailable( inData, (HIViewPartCode)inIdentifier ) )
				HIViewSimulateClick( inData->view, (HIViewPartCode)inIdentifier, 0, NULL );
			
			result = noErr;
		}
	}
	
	return result;
}

//------------------------------------------------------------------------------
//	ImageBrowserAccessibilityGetNamedActionDescription
//------------------------------------------------------------------------------
//
static OSStatus ImageBrowserAccessibilityGetNamedActionDescription(
	AXUIElementRef			inElement,
	UInt64					inIdentifier,
	EventHandlerCallRef		inCallRef,
	EventRef				inEvent,
	ImageBrowserViewData*	inData )
{
	OSStatus				result = eventNotHandledErr;
	CFStringRef				action;
	CFMutableStringRef		desc;
	
	verify_noerr( GetEventParameter( inEvent, kEventParamAccessibleActionName, typeCFStringRef,
				NULL, sizeof( action ), NULL, &action ) );
	
	verify_noerr( GetEventParameter( inEvent, kEventParamAccessibleActionDescription,
				typeCFMutableStringRef, NULL, sizeof( desc ), NULL, &desc ) );
	
	// We only handle pressed events, so if this is for a pressed action,
	// we need to determine which child the request is for and supply the 
	// appropriate string
	if ( CFStringCompare( action, kAXPressAction, 0 ) == kCFCompareEqualTo )
	{
		switch ( inIdentifier )
		{
			case kBrowseBackPart:
			{
				CFStringReplaceAll( desc, CFSTR( "Go to the previous image" ) );
				result = noErr;
			}
			break;
			
			case kBrowseForwardPart:
			{
				CFStringReplaceAll( desc, CFSTR( "Go to the next image" ) );
				result = noErr;
			}
			break;
			
			case kDeleteImagePart:
			{
				CFStringReplaceAll( desc, CFSTR( "Delete the current image" ) );
				result = noErr;
			}
			break;
		}
	}
	
	return result;
}

#pragma mark -

//------------------------------------------------------------------------------
//	ImageBrowserEventHandler
//------------------------------------------------------------------------------
//
static OSStatus ImageBrowserEventHandler(
	EventHandlerCallRef		inCallRef,
	EventRef				inEvent,
	void*					inUserData )
{
	OSStatus				err = eventNotHandledErr;
	UInt32					eventClass = GetEventClass( inEvent );
	UInt32					eventKind = GetEventKind( inEvent );
	ImageBrowserViewData*	data = (ImageBrowserViewData*)inUserData;
	
	switch ( eventClass )
	{
		case kEventClassHIObject:
		{
			switch ( eventKind )
			{
				case kEventHIObjectConstruct:
					// don't CallNextEventHandler
					err = ImageBrowserViewConstruct( inEvent );
				break;

				case kEventHIObjectInitialize:
					err = ImageBrowserViewInitialize( inCallRef, inEvent, data );
				break;
				
				case kEventHIObjectDestruct:
					// don't CallNextEventHandler
					err = ImageBrowserViewDestruct( inEvent, data );
				break;
				
				case kEventHIObjectEncode:
					err = ImageBrowserViewEncode( inCallRef, inEvent, data );
				break;
			}
		}
		break;
		
		case kEventClassControl:
		{
			switch ( eventKind )
			{
				case kEventControlDraw:
					err = ImageBrowserViewDraw( inEvent, data );
				break;
				
				case kEventControlHitTest:
					err = ImageBrowserViewHitTest( inEvent, data );
				break;
				
				case kEventControlTrack:
					err = ImageBrowserViewTrack( inEvent, data );
				break;
				
				case kEventControlHit:
					err = ImageBrowserViewHit( inEvent, data );
				break;
				
				case kEventControlSetFocusPart:
					err = ImageBrowserViewSetFocusPart( inEvent, data );
				break;
				
				case kEventControlGetFocusPart:
					err = ImageBrowserViewGetFocusPart( inEvent, data );
				break;
				
				case kEventControlActivate:
				case kEventControlDeactivate:
					err = ImageBrowserViewChangeActivation( eventKind == kEventControlActivate, data );
				break;
				
				case kEventControlHiliteChanged:
					err = ImageBrowserViewHiliteChanged( inEvent, data );
				break;
				
				case kEventControlEnabledStateChanged:
					HIViewSetNeedsDisplay( data->view, true );
				break;
				
				case kEventControlOwningWindowChanged:
					// turn on drag tracking for the window and view
					verify_noerr( SetAutomaticControlDragTrackingEnabledForWindow( HIViewGetWindow( data->view ), true ) );
					verify_noerr( SetControlDragTrackingEnabled( data->view, true ) );
				break;
				
				case kEventControlDragEnter:
					err = ImageBrowserViewDragEnter( inEvent, data );
				break;
				
				case kEventControlDragLeave:
					err = ImageBrowserViewDragLeave( inEvent, data );
				break;
				
				case kEventControlDragReceive:
					err = ImageBrowserViewDragReceive( inEvent, data );
				break;
				
				case kEventControlBoundsChanged:
					// update our tracking area to the new view bounds
					err = HIViewChangeTrackingArea( data->trackingArea, NULL );
				break;
				
				case kEventControlTrackingAreaEntered:
				case kEventControlTrackingAreaExited:
					err = ImageBrowserViewTrackingAreaEvent( eventKind == kEventControlTrackingAreaEntered, data );
				break;
			}
		}
		break;
		
		case kEventClassTextInput:
		{
			switch ( eventKind )
			{
				case kEventTextInputUnicodeForKeyEvent:
					err = ImageBrowserViewTextInput( inEvent, data );
				break;
			}
		}
		break;
		
		case kEventClassAccessibility:
		{
			AXUIElementRef		element;
			UInt64				identifier;
				
			verify_noerr( GetEventParameter( inEvent, kEventParamAccessibleObject, typeCFTypeRef,
						NULL, sizeof( element ), NULL, &element ) );
			
			AXUIElementGetIdentifier( element, &identifier );
			
			switch ( eventKind )
			{
				case kEventAccessibleGetChildAtPoint:
					err = ImageBrowserAccessibilityGetChildAtPoint( element, identifier, inCallRef, inEvent, data );
				break;
				
				case kEventAccessibleGetFocusedChild:
					err = ImageBrowserAccessibilityGetFocusedChild( element, identifier, inCallRef, inEvent, data );
				break;
				
				case kEventAccessibleGetAllAttributeNames:
					err = ImageBrowserAccessibilityGetAllAttributeNames( element, identifier, inCallRef, inEvent, data );
				break;
				
				case kEventAccessibleGetNamedAttribute:
					err = ImageBrowserAccessibilityGetNamedAttribute( element, identifier, inCallRef, inEvent, data );
				break;
				
				case kEventAccessibleGetAllActionNames:
					err = ImageBrowserAccessibilityGetAllActionNames( element, identifier, inCallRef, inEvent, data );
				break;
				
				case kEventAccessiblePerformNamedAction:
					err = ImageBrowserAccessibilityPerformNamedAction( element, identifier, inCallRef, inEvent, data );
				break;
				
				case kEventAccessibleGetNamedActionDescription:
					err = ImageBrowserAccessibilityGetNamedActionDescription( element, identifier, inCallRef, inEvent, data );
				break;
			}
		}
		break;
	}
	
	return err;
}

#pragma mark -
#pragma mark Client API

//------------------------------------------------------------------------------
//	ImageBrowserViewRegister
//------------------------------------------------------------------------------
// registers the custom ImageBrowserView class
//
OSStatus ImageBrowserViewRegister( void )
{
	OSStatus				err = noErr;
	static HIObjectClassRef	sImageBrowserViewClassRef = NULL;
	
	// only register the ImageBrowserView subclass once
	if ( sImageBrowserViewClassRef == NULL )
	{
		const EventTypeSpec kImageBrowserEvents[] =
					{ { kEventClassHIObject, kEventHIObjectConstruct },
					{ kEventClassHIObject, kEventHIObjectInitialize },
					{ kEventClassHIObject, kEventHIObjectDestruct },
					{ kEventClassHIObject, kEventHIObjectEncode },
					
					{ kEventClassControl, kEventControlDraw },
					{ kEventClassControl, kEventControlHitTest },
					{ kEventClassControl, kEventControlTrack },
					{ kEventClassControl, kEventControlHit },
					{ kEventClassControl, kEventControlSetFocusPart },
					{ kEventClassControl, kEventControlGetFocusPart },
					{ kEventClassControl, kEventControlActivate },
					{ kEventClassControl, kEventControlDeactivate },
					{ kEventClassControl, kEventControlHiliteChanged },
					{ kEventClassControl, kEventControlEnabledStateChanged },
					{ kEventClassControl, kEventControlOwningWindowChanged },
					{ kEventClassControl, kEventControlDragEnter },
					{ kEventClassControl, kEventControlDragLeave },
					{ kEventClassControl, kEventControlDragReceive },
					{ kEventClassControl, kEventControlBoundsChanged },
					{ kEventClassControl, kEventControlTrackingAreaEntered },
					{ kEventClassControl, kEventControlTrackingAreaExited },
					
					{ kEventClassTextInput, kEventTextInputUnicodeForKeyEvent },

					{ kEventClassAccessibility, kEventAccessibleGetChildAtPoint },
					{ kEventClassAccessibility, kEventAccessibleGetFocusedChild },
					{ kEventClassAccessibility, kEventAccessibleGetAllAttributeNames },
					{ kEventClassAccessibility, kEventAccessibleGetNamedAttribute },
					{ kEventClassAccessibility, kEventAccessibleGetAllActionNames },
					{ kEventClassAccessibility, kEventAccessiblePerformNamedAction },
					{ kEventClassAccessibility, kEventAccessibleGetNamedActionDescription } };
		
		// subclass from HIView
		err = HIObjectRegisterSubclass( kImageBrowserViewClassID, kHIViewClassID, 0, ImageBrowserEventHandler,
										GetEventTypeCount( kImageBrowserEvents ), kImageBrowserEvents, NULL,
										&sImageBrowserViewClassRef );
	}
	
	return err;
}

//------------------------------------------------------------------------------
//	ImageBrowserViewCreate
//------------------------------------------------------------------------------
// creates an ImageBrowserView instance
//
OSStatus ImageBrowserViewCreate(
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
OSStatus
ImageBrowserViewAddImages(
	HIViewRef				inView,
	CFArrayRef				inImageURLs )
{
	OSStatus				err = noErr;
	ImageBrowserViewData*	data;
	
	data = HIObjectDynamicCast( (HIObjectRef)inView, kImageBrowserViewClassID );
	require_action( data != NULL, CantGetInstanceData, err = paramErr );
	
	CFArrayAppendArray( data->imageURLs, inImageURLs, CFRangeMake( 0, CFArrayGetCount( inImageURLs ) ) );
	
	verify_noerr( HIViewSetNeedsDisplay( inView, true ) );
	
CantGetInstanceData:

	return err;
}
	
