/*

File: ChartView.c

Abstract: Implements an HIView that draws a chart of the internal structure of a movie's video track.

Version: 1.0.1

© Copyright 2005-2006 Apple Computer, Inc. All rights reserved.

IMPORTANT:  This Apple software is supplied to 
you by Apple Computer, Inc. ("Apple") in 
consideration of your agreement to the following 
terms, and your use, installation, modification 
or redistribution of this Apple software 
constitutes acceptance of these terms.  If you do 
not agree with these terms, please do not use, 
install, modify or redistribute this Apple 
software.

In consideration of your agreement to abide by 
the following terms, and subject to these terms, 
Apple grants you a personal, non-exclusive 
license, under Apple's copyrights in this 
original Apple software (the "Apple Software"), 
to use, reproduce, modify and redistribute the 
Apple Software, with or without modifications, in 
source and/or binary forms; provided that if you 
redistribute the Apple Software in its entirety 
and without modifications, you must retain this 
notice and the following text and disclaimers in 
all such redistributions of the Apple Software. 
Neither the name, trademarks, service marks or 
logos of Apple Computer, Inc. may be used to 
endorse or promote products derived from the 
Apple Software without specific prior written 
permission from Apple.  Except as expressly 
stated in this notice, no other rights or 
licenses, express or implied, are granted by 
Apple herein, including but not limited to any 
patent rights that may be infringed by your 
derivative works or by other works in which the 
Apple Software may be incorporated.

The Apple Software is provided by Apple on an "AS 
IS" basis.  APPLE MAKES NO WARRANTIES, EXPRESS OR 
IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED 
WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY 
AND FITNESS FOR A PARTICULAR PURPOSE, REGARDING 
THE APPLE SOFTWARE OR ITS USE AND OPERATION ALONE 
OR IN COMBINATION WITH YOUR PRODUCTS.

IN NO EVENT SHALL APPLE BE LIABLE FOR ANY 
SPECIAL, INDIRECT, INCIDENTAL OR CONSEQUENTIAL 
DAMAGES (INCLUDING, BUT NOT LIMITED TO, 
PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS 
OF USE, DATA, OR PROFITS; OR BUSINESS 
INTERRUPTION) ARISING IN ANY WAY OUT OF THE USE, 
REPRODUCTION, MODIFICATION AND/OR DISTRIBUTION OF 
THE APPLE SOFTWARE, HOWEVER CAUSED AND WHETHER 
UNDER THEORY OF CONTRACT, TORT (INCLUDING 
NEGLIGENCE), STRICT LIABILITY OR OTHERWISE, EVEN 
IF APPLE HAS BEEN ADVISED OF THE POSSIBILITY OF 
SUCH DAMAGE.

Revision History:
    <2> 07/10/2006 dts correct edit label enumeration
    <1> WWDC 2005  qte initial release

*/

#include "ChartView.h"
#include "ChartWindow.h"

typedef struct
{
	// bounds of the controls
	HIRect framesInTrackRect;
	HIRect sampleNumberTrackRect;
	HIRect trackTimeRect;
	HIRect displayTimeRect;
	HIRect framesInMediaDisplayRect;
	HIRect sampleNumberDisplayRect;
	HIRect decodeTimeRect;
	HIRect framesInMediaDecodeRect;
	HIRect sampleNumberDecodeRect;
	HIRect dataSizeRect;
	HIRect sampleFlagsRect;
} ChartRowRects;

typedef struct
{
	HIViewRef view;
	HIRect viewBounds; // Bounds of the view -- cached while drawing.
	ChartRowRects rowRects; // Bounds of the caption views -- we use these for vertically positioning the rows we draw.
	CGSize thumbnailSize; // Size of thumbnail frames.
	
	Track videoTrack;
	float pixelsPerSecond; // the scale
	float startTimeInSeconds; // the time of the left edge of the visible portion of the chart
	int units; // kChartViewUnit_Timescale or kChartViewUnit_Seconds
	
	ICMDecompressionSessionRef decompressionSession; // Used to decode frames as thumbnails.
	long lastDecompressedSampleDescIndex;
	SInt64 lastDecompressedSampleNumber;
	OSStatus lastDecompressionError;
	CGImageRef lastDecompressedImage; // The most recently decoded thumbnail image.
	CFMutableDictionaryRef thumbnailCache; // Cache of frame thumbnail CGImageRefs -- the key is the sample number.
	CGImageRef proxyThumbnailImage; // A transparent grey image to use in place of decoding a thumbnail.
} ChartViewData;

enum {
	kChartView_VideoTrack = 'VITR',
	kChartView_Scale = 'SCAL',
	kChartView_StartTime = 'STAR',
	kChartView_Units = 'UNIT',
	kChartView_ThumbnailWidth = 'TWID',
};

static OSStatus ChartView_Construct (EventRef inEvent)
{
    OSStatus err = noErr;
    ChartViewData *data = NULL;
    
	// don't CallNextEventHandler!
	data = calloc( sizeof( ChartViewData ), 1 );
    require_action( data != NULL, CantMalloc, err = memFullErr );
	
    err = GetEventParameter( inEvent, kEventParamHIObjectInstance, 
                            typeHIObjectRef, NULL, sizeof( HIObjectRef ),
                            NULL, (HIObjectRef*) &data->view );
    require_noerr( err, ParameterMissing );
    
	// Set the userData that will be used with all subsequent eventHandler calls
	err = SetEventParameter( inEvent, kEventParamHIObjectInstance, typeVoidPtr,
			sizeof( ChartViewData * ), &data ); 

ParameterMissing:
    if ( err != noErr )
        free( data );

CantMalloc:
    return err;
}

static OSStatus ChartView_Destruct (EventRef inEvent, 
                                ChartViewData* data)
{
	if( data ) {
		ICMDecompressionSessionRelease( data->decompressionSession );
		data->decompressionSession = NULL;
		
		CGImageRelease( data->lastDecompressedImage );
		
		if( data->thumbnailCache ) {
			CFRelease( data->thumbnailCache );
			data->thumbnailCache = NULL;
		}
		
		CGImageRelease( data->proxyThumbnailImage );
		
		free( data );
	}
    return noErr;
}

static OSStatus ChartView_Initialize (EventHandlerCallRef inCallRef,
                            EventRef                inEvent,
                            ChartViewData*       inData )
{
    OSStatus                err;

    // Let any parent classes have a chance at initialization
    err = CallNextEventHandler( inCallRef, inEvent );
    require_noerr( err, TroubleInSuperClass );
        
TroubleInSuperClass:
    return err;
}

// Get the bounds of the HIView with captionID, in our view's coordinate system.
static OSStatus getOneCaptionRect( ChartViewData* inData, ControlID captionID, HIRect *captionBoundsOut )
{
	OSStatus err = noErr;
	HIViewRef contentView = HIViewGetSuperview( inData->view );
	HIViewRef captionView = NULL;
	HIRect captionBounds;
	
	// Find the caption view.
	err = HIViewFindByID( contentView, captionID, &captionView );
	require_noerr( err, Failed );

	// Get its bounds in its own coordinate system.
	err = HIViewGetBounds( captionView, &captionBounds );
	require_noerr( err, Failed );
	
	// Translate them over to our coordinate system.
	err = HIViewConvertRect( &captionBounds, captionView, inData->view );
	require_noerr( err, Failed );
	
	*captionBoundsOut = captionBounds;
	
Failed:
	return err;
}

// Grab the bounds of each caption view.
static OSStatus getAllCaptionRects( ChartViewData* inData )
{
	OSStatus err = noErr;
	ChartRowRects *rowRects = &inData->rowRects;
	
	err = getOneCaptionRect( inData, kFramesInTrackCaption,        &rowRects->framesInTrackRect );
	require_noerr( err, Failed );
	
	err = getOneCaptionRect( inData, kSampleNumberTrackCaption,    &rowRects->sampleNumberTrackRect );
	require_noerr( err, Failed );
	
	err = getOneCaptionRect( inData, kTrackTimeCaption,            &rowRects->trackTimeRect );
	require_noerr( err, Failed );
	
	err = getOneCaptionRect( inData, kFramesInMediaDisplayCaption, &rowRects->framesInMediaDisplayRect );
	require_noerr( err, Failed );
	
	err = getOneCaptionRect( inData, kSampleNumberDisplayCaption,  &rowRects->sampleNumberDisplayRect );
	require_noerr( err, Failed );
	
	err = getOneCaptionRect( inData, kDisplayTimeCaption,          &rowRects->displayTimeRect );
	require_noerr( err, Failed );
	
	err = getOneCaptionRect( inData, kDecodeTimeCaption,           &rowRects->decodeTimeRect );
	require_noerr( err, Failed );
	
	err = getOneCaptionRect( inData, kFramesInMediaDecodeCaption,  &rowRects->framesInMediaDecodeRect );
	require_noerr( err, Failed );
	
	err = getOneCaptionRect( inData, kSampleNumberDecodeCaption,   &rowRects->sampleNumberDecodeRect );
	require_noerr( err, Failed );
	
	err = getOneCaptionRect( inData, kDataSizeCaption,             &rowRects->dataSizeRect );
	require_noerr( err, Failed );
	
	err = getOneCaptionRect( inData, kSampleFlagsCaption,          &rowRects->sampleFlagsRect );
	require_noerr( err, Failed );
	
Failed:
	return err;
}

// Create a solid image for use as a proxy when we don't want to bother decoding a frame.
static CGImageRef createSolidImage( CGSize size, float red, float green, float blue, float alpha )
{
	CGColorSpaceRef deviceRGB = CGColorSpaceCreateDeviceRGB();
	size_t width = ceil( size.width );
	size_t height = ceil( size.height );
	size_t bytesPerRow = ( 4*width + 15 ) & ~15;
	void *buffer = calloc( bytesPerRow, height );
	CGContextRef bc = NULL;
	CGImageRef image = NULL;
	
	bc = CGBitmapContextCreate( buffer, width, height, 8, bytesPerRow, deviceRGB, kCGImageAlphaPremultipliedFirst );
	
	CGContextClear( bc, CGRectMake( 0, 0, width, height ) );
	CGContextSetRGBFillColor( bc, red, green, blue, alpha );
	CGContextFillRect( bc, CGRectMake( 0, 0, width, height ) );
	
	image = CGBitmapContextCreateImage( bc );
	
	CGColorSpaceRelease( deviceRGB );
	CGContextRelease( bc );
	free( buffer );
	return image;
}

// Utility to add an SInt32 to a CFMutableDictionary.
static void
addSInt32ToDictionary( CFMutableDictionaryRef dictionary, CFStringRef key, SInt32 numberSInt32 )
{
	CFNumberRef number = CFNumberCreate( NULL, kCFNumberSInt32Type, &numberSInt32 );
	if( ! number ) 
		return;
	CFDictionaryAddValue( dictionary, key, number );
	CFRelease( number );
}

// Utility to create a CGImage from a CVPixelBuffer.

static void releaseAndUnlockThis( void *info, const void *data, size_t size )
{
	CVPixelBufferRef pixelBuffer = info;
	
	CVPixelBufferUnlockBaseAddress( pixelBuffer, 0 );
	CVBufferRelease( pixelBuffer );
}
static CGImageRef createCGImageFrom32XRGBCVPixelBuffer( CVPixelBufferRef pixelBuffer )
{
	size_t width, height, rowBytes;
	void *baseAddr = NULL;
	CGColorSpaceRef colorspace = NULL;
	CGDataProviderRef provider = NULL;
	CGImageRef image = NULL;
	
	CVPixelBufferLockBaseAddress( pixelBuffer, 0 );
	
	rowBytes = CVPixelBufferGetBytesPerRow( pixelBuffer );
	baseAddr = CVPixelBufferGetBaseAddress( pixelBuffer );
	width = CVPixelBufferGetWidth( pixelBuffer );
	height = CVPixelBufferGetHeight( pixelBuffer );
	
	colorspace = CGColorSpaceCreateDeviceRGB();
	CVBufferRetain( pixelBuffer );
	provider = CGDataProviderCreateWithData( pixelBuffer, baseAddr, rowBytes * height, releaseAndUnlockThis );
	image = CGImageCreate( width, height, 8, 32, rowBytes, colorspace, 
						   kCGImageAlphaNoneSkipFirst, provider, NULL, true, kCGRenderingIntentDefault );
	
bail:
	if( provider ) CGDataProviderRelease( provider );
	if( colorspace ) CGColorSpaceRelease( colorspace );
	return image;
}

static void trackFrame( void *decompressionTrackingRefCon, OSStatus result, ICMDecompressionTrackingFlags decompressionTrackingFlags, CVPixelBufferRef pixelBuffer, TimeValue64 displayTime, TimeValue64 displayDuration, ICMValidTimeFlags validTimeFlags, void *reserved, void *sourceFrameRefCon );

// Create a decompression session that will render frames at thumbnail dimensions and call trackFrame for each frame.
static OSStatus createDecompressionSession( ChartViewData *data, long mediaSampleDescIndex )
{
	OSStatus err = noErr;
	ImageDescriptionHandle imageDesc = NULL;
	SInt32 width = data->thumbnailSize.width;
	SInt32 height = data->thumbnailSize.height;
	OSType pixelFormat = k32ARGBPixelFormat;
	CFMutableDictionaryRef pixelBufferAttributes = NULL;
	ICMDecompressionSessionOptionsRef sessionOptions = NULL;
	ICMDecompressionTrackingCallbackRecord trackingCallbackRecord;
	
	// Read the image description from the media.
	imageDesc = (ImageDescriptionHandle)NewHandle(0);
	require_action( imageDesc != NULL, CantNewHandle, err = memFullErr );
	GetMediaSampleDescription( GetTrackMedia( data->videoTrack ), mediaSampleDescIndex, (SampleDescriptionHandle)imageDesc );
	err = GetMoviesError();
	require_noerr( err, CantGetMediaSampleDescription );
	
	// Create a dictionary describing the pixel buffers we want to get back.
	pixelBufferAttributes = CFDictionaryCreateMutable( NULL, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks );
	
	addSInt32ToDictionary( pixelBufferAttributes, kCVPixelBufferWidthKey, width );
	addSInt32ToDictionary( pixelBufferAttributes, kCVPixelBufferHeightKey, height );
	addSInt32ToDictionary( pixelBufferAttributes, kCVPixelBufferPixelFormatTypeKey, pixelFormat );
	
	trackingCallbackRecord.decompressionTrackingCallback = trackFrame;
	trackingCallbackRecord.decompressionTrackingRefCon = data;
	
	err = ICMDecompressionSessionCreate( NULL, imageDesc, sessionOptions, 
				pixelBufferAttributes, &trackingCallbackRecord, &data->decompressionSession );
	
CantGetMediaSampleDescription:
CantNewHandle:
	if( pixelBufferAttributes )
		CFRelease( pixelBufferAttributes );
	DisposeHandle( (Handle)imageDesc );
	return err;
}

// The "tracking callback" for our decompression session.
// It releases source frame buffers when no longer needed, 
// and grabs emitted pixel buffers and wraps them in CGImages, 
// which are saved in our private storage.
static void trackFrame( void *decompressionTrackingRefCon,
						OSStatus result,
						ICMDecompressionTrackingFlags decompressionTrackingFlags,
						CVPixelBufferRef pixelBuffer,
						TimeValue64 displayTime,
						TimeValue64 displayDuration,
						ICMValidTimeFlags validTimeFlags,
						void *reserved,
						void *sourceFrameRefCon )
{
	ChartViewData *data = decompressionTrackingRefCon;
	data->lastDecompressionError = result;
	
	if( kICMDecompressionTracking_ReleaseSourceData & decompressionTrackingFlags ) {
		// Our sourceFrameRefCons are the malloced frame data buffers.
		free( sourceFrameRefCon );
	}
	if( ( kICMDecompressionTracking_EmittingFrame & decompressionTrackingFlags ) && pixelBuffer ) {
		CGImageRef image = createCGImageFrom32XRGBCVPixelBuffer( pixelBuffer );
		CGImageRelease( data->lastDecompressedImage );
		data->lastDecompressedImage = image;
	}
}

// Walk the media in decode order, starting with either the next frame from last time or from a key frame,
// pushing frames into the decompression session.  This is unusual in that we actually want the frames back
// in decode order even if it's not the same as display order.  If we wanted frames back in display order,
// we would use "non-scheduled display time" to index the frames we wanted to pull out of the session.
static OSStatus decompressFramesUpTo( ChartViewData *data, SInt64 targetSampleNumber, CGImageRef *thumbnailOut )
{
	OSStatus err = noErr;
	Media videoMedia = GetTrackMedia( data->videoTrack );
	SInt64 syncSampleNumber, nextSampleNumber;
	TimeValue64 targetDecodeTime, syncDecodeTime;
	
	// Find the last key frame (sync sample) at or before the target sample number.
	SampleNumToMediaDecodeTime( videoMedia, targetSampleNumber, &targetDecodeTime, NULL );
	err = GetMoviesError();
	require_noerr( err, Failure );
	
	GetMediaNextInterestingDecodeTime( videoMedia,
								 nextTimeSyncSample | nextTimeEdgeOK,
								 targetDecodeTime,
								 -fixed1,
								 &syncDecodeTime,
								 NULL );
	err = GetMoviesError();
	require_noerr( err, Failure );
	
	MediaDecodeTimeToSampleNum( videoMedia, syncDecodeTime, &syncSampleNumber, NULL, NULL );
	err = GetMoviesError();
	require_noerr( err, Failure );
	
	// Pick the starting point.
	if( ( data->lastDecompressedSampleNumber + 1 <= targetSampleNumber )
	 && ( syncSampleNumber < data->lastDecompressedSampleNumber + 1 ) )
		nextSampleNumber = data->lastDecompressedSampleNumber + 1;
	else
		nextSampleNumber = syncSampleNumber;
	
	// Walk forward in decode order.
	for( ; nextSampleNumber <= targetSampleNumber; nextSampleNumber++ ) {
		TimeValue64 sampleDecodeTime;
		ByteCount sampleDataSize = 0;
		MediaSampleFlags sampleFlags = 0;
		UInt8 *sampleData = NULL;
		ICMFrameTimeRecord frameTime = {0};
		
		// Get the frame's data size and sample flags.  
		SampleNumToMediaDecodeTime( videoMedia, nextSampleNumber, &sampleDecodeTime, NULL );
		err = GetMediaSample2( videoMedia, NULL, 0, &sampleDataSize, sampleDecodeTime,
							   NULL, NULL, NULL, NULL, NULL, 1, NULL, &sampleFlags );
		require_noerr( err, Failure );
		
		// We can skip droppable frames before the target.
		if( ( nextSampleNumber != targetSampleNumber ) && ( mediaSampleDroppable & sampleFlags ) )
			continue;
		
		// Load the frame.
		sampleData = malloc( sampleDataSize );
		err = GetMediaSample2( videoMedia, sampleData, sampleDataSize, NULL, sampleDecodeTime,
							   NULL, NULL, NULL, NULL, NULL, 1, NULL, NULL );
		require_noerr( err, Failure );
		
		// Set up an immediate decode request -- we don't care about frame times, we just need to pass a flag.
		frameTime.recordSize = sizeof(ICMFrameTimeRecord);
		*(TimeValue64 *)&frameTime.value = sampleDecodeTime;
		frameTime.scale = GetMediaTimeScale( videoMedia );
		frameTime.rate = fixed1;
		frameTime.frameNumber = nextSampleNumber;
		
		// If we haven't reached the target sample, tell the session not to emit the frame.
		if( nextSampleNumber != targetSampleNumber )
			frameTime.flags = icmFrameTimeDoNotDisplay;
		
		// Decode the frame. 
		err = ICMDecompressionSessionDecodeFrame( data->decompressionSession, 
				sampleData, sampleDataSize, NULL, &frameTime, sampleData );
		require_noerr( err, Failure );
		
		// Note: trackFrame will release the sampleData buffer.
		
		data->lastDecompressedSampleNumber = nextSampleNumber;
	}
	
Success:
	*thumbnailOut = data->lastDecompressedImage;
	
Failure:
	return err;
}

// Returns a thumbnail of the given frame as a CGImage.  
// These are cached in a dictionary because it's likely we'll need them again.
// It would be smart to keep a limit on the number that we cache at a time to avoid tying up too much memory.
static OSStatus copyFrameThumbnail( 
	ChartViewData *data, 
	SInt64 sampleNumber, 
	long mediaSampleDescIndex, 
	Boolean onlyIfCached,
	CGImageRef *thumbnailOut )
{
	OSStatus err = noErr;
	CGImageRef thumbnail = NULL;
	
	*thumbnailOut = NULL;
	
	// If the thumbnail is already cached, pull it from the cache and retain and return it.
	if( data->thumbnailCache ) {
		thumbnail = (CGImageRef)CFDictionaryGetValue( data->thumbnailCache, (void *)(long)sampleNumber );
		if( thumbnail )
			goto Success;
	}
	
	// If we've been told not to grab anything not already in the cache, return a grey proxy instead.
	if( onlyIfCached ) {
		if( ! data->proxyThumbnailImage )
			data->proxyThumbnailImage = createSolidImage( data->thumbnailSize, 0.2, 0.2, 0.2, 0.8 );
		thumbnail = data->proxyThumbnailImage;
		goto Success;
	}
	
	// Otherwise, advance our decompression session to draw this sample, cache and return it.
	
	// If we switch sample descriptions, make sure we rebuild the decompression session.
	if( data->lastDecompressedSampleDescIndex != mediaSampleDescIndex ) {
		ICMDecompressionSessionRelease( data->decompressionSession );
		data->decompressionSession = NULL;
	}
	// Create the decompression session if necessary.
	if( ! data->decompressionSession ) {
		err = createDecompressionSession( data, mediaSampleDescIndex );
		require_noerr( err, Failure );
		
		data->lastDecompressedSampleNumber = 0;
		data->lastDecompressedSampleDescIndex = mediaSampleDescIndex;
	}
	// Decode enough frames.
	if( data->decompressionSession ) {
		err = decompressFramesUpTo( data, sampleNumber, &thumbnail );
		require_noerr( err, Failure );
	}
	
	// Cache the thumbnail -- create the cache directory if necessary.
	if( ! data->thumbnailCache )
		data->thumbnailCache = CFDictionaryCreateMutable( NULL, 0, NULL, &kCFTypeDictionaryValueCallBacks );
	if( data->thumbnailCache )
		CFDictionaryAddValue( data->thumbnailCache, (void *)(long)sampleNumber, thumbnail );
	
Success:
	CGImageRetain( thumbnail );
	*thumbnailOut = thumbnail;
	
Failure:
	return err;
}

// Some silly calculations designed to pick distinct colours for edits.
static void getSpacedOutFraction( int i, int *numerOut, int *denomOut )
{
	int numer, denom;
	denom = 1;
	while( denom && ( denom <= i ) )
		denom = denom << 1;
	numer = 1 + ( 2 * i - denom );
	*numerOut = numer;
	*denomOut = denom;
}

static void getSpacedOutSixth( int i, int *numerOut, int *denomOut )
{
	int slot = ( ( i - 1 ) % 6 );
	int isixth = ( ( i - 1 ) / 6 ) + 1;
	int numer, denom;
	getSpacedOutFraction( isixth, &numer, &denom );
	// n/6d + slot/6
	*numerOut = numer + slot * denom;
	*denomOut = denom * 6;
}

static float getWrappedComponent( float f )
{
  // 0/6 -> 1.0
  // 1/6 -> 1.0
  // 2/6 -> 1.0
  // 3/6 -> 0.0
  // 4/6 -> 0.0
  // 5/6 -> 0.0
  // 6/6 -> 1.0
  f = f - floor(f);
  if( f <= 2.0/6.0 ) {
    return 1;
  }
  else if( f <= 3.0/6.0 ) {
    return 3 - f * 6;
  }
  else if( f <= 5.0/6.0 ) {
    return 0;
  }
  else {
    return f * 6 - 5;
  }
}

// Pick a distinctive color for the nth edit.
static void setRGBFillColorForEdit( CGContextRef c, int editIndex )
{
	int numer, denom;
	float angle, red, green, blue;
	getSpacedOutSixth( editIndex, &numer, &denom );
	angle = (float)numer/(float)denom;
	red   = getWrappedComponent( angle + 0/3.0 );
	green = getWrappedComponent( angle + 1/3.0 );
	blue  = getWrappedComponent( angle + 2/3.0 );
	CGContextSetRGBFillColor( c, red, green, blue, 0.5 );
}

// Convert a duration to a width in pixels.
static float convertSecondsToWidth( ChartViewData *data, double time, double timescale )
{
	return ( time / timescale ) * data->pixelsPerSecond;
}

// Convert a timestamp to an x-coordinate in pixels.
static float convertSecondsToXCoordinate( ChartViewData *data, double time, double timescale )
{
	return 5 + data->viewBounds.origin.x + ( time / timescale - data->startTimeInSeconds ) * data->pixelsPerSecond;
}

// Create a string for a timestamp.
static CFStringRef createStringForTime( int units, TimeValue64 time, TimeScale timescale, Boolean showMinutes )
{
	switch( units ) {
		case kChartViewUnit_Timescale:
			return CFStringCreateWithFormat( NULL, NULL, CFSTR("%lld"), (long long)time );
			
		case kChartViewUnit_Seconds:
		{
			Boolean showHours = false;
			TimeValue64 seconds = time / timescale;
			TimeValue64 partialSeconds = time % timescale;
			CFMutableStringRef str = NULL;
			str = CFStringCreateMutable( NULL, 0 );
			if( seconds >= 60 * 60 )
				showHours = true;
			if( showHours || ( seconds >= 60 ) )
				showMinutes = true;
			// hours
			if( showHours )
				CFStringAppendFormat( str, NULL, CFSTR("%d:"), (int)( seconds / ( 60 * 60 ) ) );
			// minutes
			if( showMinutes )
				CFStringAppendFormat( str, NULL, showHours ? CFSTR("%02d:") : CFSTR("%d:"), (int)( seconds / 60 ) );
			// seconds
			CFStringAppendFormat( str, NULL, showMinutes ? CFSTR("%02d") : CFSTR("%d"), (int)( seconds % 60 ) );
			// milliseconds
			if( partialSeconds != 0 )
				CFStringAppendFormat( str, NULL, CFSTR(".%03d"), (int)( partialSeconds * 1000 / timescale ) );
			return str;
		}
	}
	return NULL;
}

// Draw a circle corresponding to a type of sample (based on sample flags).
static void drawBlob( CGContextRef c, float x, float y, float radius, MediaSampleFlags sampleFlags )
{
	Boolean stroke = false;
	CGContextSaveGState( c );
	CGContextSetLineWidth( c, 1.0 );
	if( 0 == ( mediaSampleNotSync & sampleFlags ) ) {
		CGContextSetRGBFillColor( c, 0.8, 0.0, 0.0, 1.0 ); // sync: red
	}
	else if( mediaSamplePartialSync & sampleFlags ) {
		CGContextSetRGBFillColor( c, 1.0, 0.5, 0.0, 1.0 ); // partial sync: orange
	}
	else if( mediaSampleDroppable & sampleFlags ) {
		CGContextSetRGBStrokeColor( c, 0.0, 0.7, 0.0, 1.0 ); // droppable: green outline
		stroke = true;
		CGContextSetRGBFillColor( c, 1.0, 1.0, 1.0, 1.0 ); // droppable: white centre
	}
	else {
		CGContextSetRGBFillColor( c, 0.0, 0.5, 1.0, 1.0 ); // difference: blue
	}
	CGContextBeginPath( c );
	CGContextAddArc( c, x, y, radius, 0, 2*pi, true );
	CGContextFillPath( c );
	if( stroke ) {
		CGContextAddArc( c, x, y, radius-0.5, 0, 2*pi, true );
		CGContextStrokePath( c );
	}
	CGContextRestoreGState( c );
}

// Create an image used for masking, with a soft right edge.
static CGImageRef createImageWithSoftRightEdge( int width, int height, int edge )
{
	CGColorSpaceRef deviceGray = CGColorSpaceCreateDeviceGray();
	size_t bytesPerRow = ( width + 15 ) & ~15;
	void *buffer = calloc( bytesPerRow, height );
	CGContextRef bc = NULL;
	CGImageRef image = NULL;
	int i;
	
	if( edge > width / 2 )
		edge = width / 2;
	
	bc = CGBitmapContextCreate( buffer, width, height, 8, bytesPerRow, deviceGray, kCGImageAlphaNone );
	
	CGContextSetGrayFillColor( bc, 1.0, 1.0 );
	CGContextFillRect( bc, CGRectMake( 0, 0, width - edge, height ) );
	for( i = 0; i < edge; i++ ) {
		CGContextSetGrayFillColor( bc, (float)(edge - i) / (float)edge, 1.0 );
		CGContextFillRect( bc, CGRectMake( width - edge + i, 0, 1, height ) );
	}
	
	image = CGBitmapContextCreateImage( bc );
	
	CGColorSpaceRelease( deviceGray );
	CGContextRelease( bc );
	free( buffer );
	return image;
}

// Create a string that describes interesting sample flags.
static CFStringRef createStringForMediaSampleFlags( MediaSampleFlags sampleFlags )
{
	CFMutableStringRef str = NULL;
	str = CFStringCreateMutable( NULL, 0 );
	
	if( 0 == ( mediaSampleNotSync & sampleFlags ) )
		CFStringAppend( str, CFSTR("sync\r") );
	if( mediaSampleDroppable & sampleFlags )
		CFStringAppend( str, CFSTR("droppable\r") );
	if( mediaSamplePartialSync & sampleFlags )
		CFStringAppend( str, CFSTR("partial sync\r") );
	
	// There are other flags that may also be interesting.
	
	return str;
}

// Draw alternating white and pale blue bands every second.
// Label the seconds at the bottom of the window, but throttle the label frequency to a power of 10 (seconds) so that 
// we don't draw more than 15 labels.
static OSStatus drawTimeMarks(
	ChartViewData* data,
	CGContextRef c,
	float minimumTime,
	float maximumTime )
{
	HIThemeTextInfo labelTextInfo = { 0, kThemeStateActive, kThemeSystemFont, kHIThemeTextHorizontalFlushLeft, kHIThemeTextVerticalFlushTop, kHIThemeTextBoxOptionStronglyVertical, kHIThemeTextTruncationNone, 0, false };
	float bandTop = CGRectGetMinY( data->viewBounds );
	float bandBottom = CGRectGetMaxY( data->viewBounds );
	float tickTime, tickFrequency, tickX, tickWidth;
	CGRect tickLabelRect;
	CFStringRef tickLabelString = NULL;
	
	CGContextSaveGState( c );
	
	// Draw a pale blue band for every even second.
	CGContextSetRGBFillColor( c, 0.80, 0.80, 0.90, 1.0 ); // very pale blue
	CGContextBeginPath( c );
	for( tickTime = 2 * floor( minimumTime / 2 ); tickTime < maximumTime; tickTime += 2.0 ) {
		tickX = convertSecondsToXCoordinate( data, tickTime, 1 );
		tickWidth = convertSecondsToWidth( data, 1, 1 );
		CGContextAddRect( c, 
				CGRectMake( tickX, bandTop, 
							tickWidth, bandBottom - bandTop ) );
	}
	CGContextFillPath( c );
	
	// Label the seconds at the bottom of the window, but throttle the label frequency to a power of 10 (seconds) 
	// so that we don't draw more than 15 labels.
	tickFrequency = 1.0;
	while( ( maximumTime - minimumTime ) / tickFrequency > 15 )
		tickFrequency = tickFrequency * 10;
	CGContextSetRGBFillColor( c, 0, 0, 0, 1 ); // opaque black
	for( tickTime = floor( minimumTime ); tickTime < maximumTime; tickTime += tickFrequency ) {
		tickX = convertSecondsToXCoordinate( data, tickTime, 1 );
		
		tickLabelRect = CGRectMake( tickX, CGRectGetMaxY( data->viewBounds ) - 16, kLabelWidth, 16 );
		tickLabelString = createStringForTime( kChartViewUnit_Seconds, tickTime, 1, true );
		if( tickLabelString ) {
			HIThemeDrawTextBox( tickLabelString, &tickLabelRect, &labelTextInfo, c, kHIThemeOrientationNormal );
			CFRelease( tickLabelString );
		}
	}
	
	CGContextRestoreGState( c );
	return noErr;
}

// Draw the edit list, which is a mapping from media display time ranges to track time ranges.
// Label the track times when edits start.
static OSStatus drawEditList(
	ChartViewData* data,
	CGContextRef c,
	float minimumTime,
	float maximumTime )
{
	Track videoTrack = data->videoTrack;
	TimeScale trackTimeScale = GetMovieTimeScale( GetTrackMovie( videoTrack ) );
	TimeScale mediaTimeScale = GetMediaTimeScale( GetTrackMedia( videoTrack ) );
	TimeValue editTrackStart, editTrackDuration, editTrackEnd;
	TimeValue64 editDisplayStart, editDisplayDuration, editDisplayEnd;
	float trackAxisY, displayAxisY;
	float midTrackAxisY, midDisplayAxisY;
	int editIndex = 0;
	HIThemeTextInfo labelTextInfo = { 0, kThemeStateActive, kThemeSystemFont, kHIThemeTextHorizontalFlushLeft, kHIThemeTextVerticalFlushTop, kHIThemeTextBoxOptionStronglyVertical, kHIThemeTextTruncationNone, 0, false };
	
	// Calculate Y coordinates that we will need.
	trackAxisY = CGRectGetMidY( data->rowRects.trackTimeRect );
	displayAxisY = CGRectGetMinY( data->rowRects.framesInMediaDisplayRect );
	midTrackAxisY = ( trackAxisY + displayAxisY ) / 2;
	midDisplayAxisY = ( trackAxisY + displayAxisY ) / 2;
	
	CGContextSaveGState( c );
	
	// Find the first edit, skipping empty edits.
	// Each edit has a starting track timestamp, a duration in track time, a starting display timestamp and a rate.
	// We'll use the rate to calculate the ending display timestamp.
	GetTrackNextInterestingTime( videoTrack, 
								 nextTimeTrackEdit | nextTimeEdgeOK,
								 0,
								 fixed1,
								 &editTrackStart,
								 &editTrackDuration );
	while( ( editTrackStart >= 0 ) && ( editTrackDuration > 0 ) ) {
		// Get the media display time corresponding to the start of this edit.
		editDisplayStart = TrackTimeToMediaDisplayTime( editTrackStart, videoTrack );
		if( editDisplayStart >= 0 ) {
			float editTrackX1, editTrackX2, editDisplayX1, editDisplayX2;
			Fixed rate;
			HIRect editLabelRect;
			CFStringRef editLabelString = NULL, editStartString = NULL, editEndString = NULL;
			// Get the edit rate for this edit and use it to find when the edit ends in display time.
			rate = GetTrackEditRate( videoTrack, editTrackStart );
			editDisplayDuration = editTrackDuration * Fix2X( rate ) * mediaTimeScale / trackTimeScale;
			editTrackEnd = editTrackStart + editTrackDuration;
			editDisplayEnd = editDisplayStart + editDisplayDuration;
			
			editTrackX1 = convertSecondsToXCoordinate( data, editTrackStart, trackTimeScale );
			editTrackX2 = convertSecondsToXCoordinate( data, editTrackEnd, trackTimeScale );
			editDisplayX1 = convertSecondsToXCoordinate( data, editDisplayStart, mediaTimeScale );
			editDisplayX2 = convertSecondsToXCoordinate( data, editDisplayEnd, mediaTimeScale );
			
			// Pick a different color for each edit.  
			// By filling with 50% opacity we'll be able to identify overlapping edits.
            editIndex++;
			setRGBFillColorForEdit( c, editIndex );
			
			// Draw the edit.
			CGContextBeginPath( c );
			CGContextMoveToPoint( c, 
					editTrackX1, trackAxisY );
			CGContextAddCurveToPoint( c, 
					editTrackX1, midTrackAxisY,
					editDisplayX1, midDisplayAxisY,
					editDisplayX1, displayAxisY );
			CGContextAddLineToPoint( c, 
					editDisplayX2, displayAxisY );
			CGContextAddCurveToPoint( c, 
					editDisplayX2, midDisplayAxisY,
					editTrackX2, midTrackAxisY,
					editTrackX2, trackAxisY );
			CGContextFillPath( c );
			
			// Label the edit with the track time range.
			CGContextSetRGBFillColor( c, 0, 0, 0, 1 ); // opaque black
			if( editTrackX1 < 0 )
				editTrackX1 = 0;
			editLabelRect = CGRectMake( editTrackX1, data->rowRects.trackTimeRect.origin.y, editTrackX2 - editTrackX1, data->rowRects.trackTimeRect.size.height );
			editStartString = createStringForTime( data->units, editTrackStart, trackTimeScale, true );
			editEndString = createStringForTime( data->units, editTrackEnd, trackTimeScale, true );
			editLabelString = CFStringCreateWithFormat( NULL, NULL, CFSTR("edit #%d from %@ to %@"), editIndex, editStartString, editEndString );
			if( editLabelString ) {
				HIThemeDrawTextBox( editLabelString, &editLabelRect, &labelTextInfo, c, kHIThemeOrientationNormal );
				CFRelease( editLabelString );
				CFRelease( editStartString );
				CFRelease( editEndString );
			}
		}
		
		// Find the next edit, skipping empty edits.
		GetTrackNextInterestingTime( videoTrack, 
									 nextTimeTrackEdit,
									 editTrackStart,
									 fixed1,
									 &editTrackStart,
									 &editTrackDuration );
	}
	
	CGContextRestoreGState( c );
	return noErr;
}

// Draw the samples and their offsets from media decode time to media display time.
// Label each sample with its decode time, display time, sample number, data size and sample flags
// in the appropriate areas.
static OSStatus drawSampleInfo(
	ChartViewData* data,
	CGContextRef c,
	float minimumTime,
	float maximumTime )
{
	OSStatus err = noErr;
	Track videoTrack = data->videoTrack;
	Media videoMedia = GetTrackMedia( videoTrack );
	TimeScale mediaTimeScale = GetMediaTimeScale( videoMedia );
	TimeValue64 startDecodeTime, maxDecodeDuration;
	QTMutableSampleTableRef sampleTable = NULL;
	float decodeAxisY, displayAxisY;
	HIThemeTextInfo labelTextInfo = { 0, kThemeStateActive, kThemeSystemFont, kHIThemeTextHorizontalFlushLeft, kHIThemeTextVerticalFlushTop, kHIThemeTextBoxOptionStronglyVertical, kHIThemeTextTruncationNone, 0, false };
	HIThemeTextInfo labelSmallTextInfo = { 0, kThemeStateActive, kThemeSmallSystemFont, kHIThemeTextHorizontalFlushLeft, kHIThemeTextVerticalFlushTop, kHIThemeTextBoxOptionStronglyVertical, kHIThemeTextTruncationNone, 0, false };
	float maskWidth;
	CGImageRef maskImage = NULL;
	
	// Calculate Y coordinates that we will need.
	decodeAxisY = CGRectGetMidY( data->rowRects.decodeTimeRect );
	displayAxisY = CGRectGetMidY( data->rowRects.displayTimeRect );
	
	CGContextSaveGState( c );
	CGContextSetLineCap( c, kCGLineCapRound );
	CGContextSetLineWidth( c, 1.0 );
	
	// Convert minimumTime into media decode time; this will be our starting decode time.
	startDecodeTime = minimumTime * mediaTimeScale;
	maxDecodeDuration = ( maximumTime - minimumTime ) * mediaTimeScale;
	
	while( ( startDecodeTime < maximumTime * mediaTimeScale ) && ( startDecodeTime < GetMediaDecodeDuration( videoMedia ) ) )  {
		TimeValue64 sampleDecodeTime;
		TimeValue64 sampleTableStartDecodeTime = 0, sampleTableTotalDecodeDuration = 0;
		SInt64 sampleCount = 0, sampleIndex, sampleNumber;
		
		// Extract some samples as a new, mutable QTSampleTableRef.
		// QuickTime may return fewer than the max duration, so we may need to call this multiple times.
		err = CopyMediaMutableSampleTable( videoMedia,
				startDecodeTime, &sampleTableStartDecodeTime, 0, maxDecodeDuration, &sampleTable );
		require_noerr( err, CantCopyMediaMutableSampleTable );
	
		sampleCount = QTSampleTableGetNumberOfSamples( sampleTable );
		sampleDecodeTime = sampleTableStartDecodeTime;
		MediaDecodeTimeToSampleNum( videoMedia, sampleTableStartDecodeTime, &sampleNumber, NULL, NULL );
		
		for( sampleIndex = 1; 
					( sampleIndex <= sampleCount )
				 && ( sampleDecodeTime < startDecodeTime + maxDecodeDuration ); 
				sampleIndex++, sampleNumber++ ) {
			float decodeX, displayX, decodeWidth, displayWidth;
			CGRect maskRect;
			CGImageRef frameThumbnail = NULL;
			HIRect labelRect;
			CFStringRef labelString = NULL;
			Boolean onlyIfCached = false;
			
			// Extract information about this sample from the sample table.
			TimeValue64 decodeDuration = QTSampleTableGetDecodeDuration( sampleTable, sampleIndex );
			TimeValue64 displayOffset = QTSampleTableGetDisplayOffset( sampleTable, sampleIndex );
			TimeValue64 displayDuration;
			ByteCount dataSize = QTSampleTableGetDataSizePerSample( sampleTable, sampleIndex );
			MediaSampleFlags sampleFlags = QTSampleTableGetSampleFlags( sampleTable, sampleIndex );
			QTSampleDescriptionID sampleDescID = QTSampleTableGetSampleDescriptionID( sampleTable, sampleIndex );
			long mediaSampleDescIndex = 0;
			QTSampleTableCopySampleDescription( sampleTable, sampleDescID, &mediaSampleDescIndex, NULL );
			
			// The display duration isn't held in the sample table, so we'll grab that from the media.
			SampleNumToMediaDisplayTime( videoMedia, sampleNumber, NULL, &displayDuration );
			
			decodeX = convertSecondsToXCoordinate( data, sampleDecodeTime, mediaTimeScale );
			// The display time is the decode time plus the decode offset.
			displayX = convertSecondsToXCoordinate( data, sampleDecodeTime + displayOffset, mediaTimeScale );
			
			// Draw a circle to represent the sample.
			drawBlob( c, decodeX, decodeAxisY, 4, sampleFlags );
			
			// Draw the line from decode time stamp to display time stamp.
			if( mediaSampleEarlierDisplayTimesAllowed & sampleFlags )
				CGContextSetRGBStrokeColor( c, 0.0, 0.0, 0.5, 1.0 ); // navy
			else
				CGContextSetRGBStrokeColor( c, 0.0, 0.0, 0.0, 1.0 ); // black
			CGContextBeginPath( c );
			CGContextMoveToPoint( c, decodeX, decodeAxisY );
			CGContextAddLineToPoint( c, displayX, displayAxisY );
			CGContextStrokePath( c );
			
			decodeWidth = floor( convertSecondsToWidth( data, decodeDuration, mediaTimeScale ) );
			displayWidth = floor( convertSecondsToWidth( data, displayDuration, mediaTimeScale ) );

			// Draw decode-time-related rows.
			if( decodeWidth > 2 ) {
				CGContextSaveGState( c );

				// Clip the next few items using a rectangular mask with a soft right edge.
				if( ( NULL == maskImage ) || ( maskWidth != decodeWidth ) ) {
					CGImageRelease( maskImage );
					maskImage = createImageWithSoftRightEdge( decodeWidth, 1, 5 );
					maskWidth = decodeWidth;
				}
				maskRect = CGRectUnion( CGRectMake( floor(decodeX), data->rowRects.decodeTimeRect.origin.y,          floor(decodeWidth), data->rowRects.decodeTimeRect.size.height ),
						   CGRectUnion( CGRectMake( floor(decodeX), data->rowRects.framesInMediaDecodeRect.origin.y, floor(decodeWidth), data->rowRects.framesInMediaDecodeRect.size.height ),
						   CGRectUnion( CGRectMake( floor(decodeX), data->rowRects.sampleNumberDecodeRect.origin.y,  floor(decodeWidth), data->rowRects.sampleNumberDecodeRect.size.height ),
						   CGRectUnion( CGRectMake( floor(decodeX), data->rowRects.dataSizeRect.origin.y,            floor(decodeWidth), data->rowRects.dataSizeRect.size.height ),
										CGRectMake( floor(decodeX), data->rowRects.sampleFlagsRect.origin.y,         floor(decodeWidth), data->rowRects.sampleFlagsRect.size.height ) ) ) ) );
				CGContextClipToMask( c, maskRect, maskImage );
				
				// Write the decode time.
				labelString = createStringForTime( data->units, sampleDecodeTime, mediaTimeScale, false );
				if( labelString ) {
					labelRect = CGRectMake( floor(decodeX+2), data->rowRects.decodeTimeRect.origin.y, kLabelWidth, data->rowRects.decodeTimeRect.size.height );
					HIThemeDrawTextBox( labelString, &labelRect, &labelTextInfo, c, kHIThemeOrientationNormal );
					CFRelease( labelString );
				}
				
				// Draw the frame thumbnail.
				if( decodeWidth <= 10 )
					onlyIfCached = true;
				copyFrameThumbnail( data, sampleNumber, mediaSampleDescIndex, onlyIfCached, &frameThumbnail );
				if( frameThumbnail ) {
					CGRect thumbnailRect = CGRectMake( floor(decodeX), data->rowRects.framesInMediaDecodeRect.origin.y, data->thumbnailSize.width, data->rowRects.framesInMediaDecodeRect.size.height );
					HIViewDrawCGImage( c, &thumbnailRect, frameThumbnail ); // like CGContextDrawImage, but handles flip
					CGImageRelease( frameThumbnail );
				}
			
				// Write the sample number.
				labelString = CFStringCreateWithFormat( NULL, NULL, CFSTR("%lld"), (long long)sampleNumber );
				if( labelString ) {
					labelRect = CGRectMake( floor(decodeX), data->rowRects.sampleNumberDecodeRect.origin.y, kLabelWidth, data->rowRects.sampleNumberDecodeRect.size.height );
					HIThemeDrawTextBox( labelString, &labelRect, &labelTextInfo, c, kHIThemeOrientationNormal );
					CFRelease( labelString );
				}
				
				// Write the data size.
				labelString = CFStringCreateWithFormat( NULL, NULL, CFSTR("%ld bytes"), (long)dataSize );
				if( labelString ) {
					labelRect = CGRectMake( decodeX, data->rowRects.dataSizeRect.origin.y, kLabelWidth, data->rowRects.dataSizeRect.size.height );
					HIThemeDrawTextBox( labelString, &labelRect, &labelSmallTextInfo, c, kHIThemeOrientationNormal );
					CFRelease( labelString );
				}

				// Write the media sample flags.
				labelString = createStringForMediaSampleFlags( sampleFlags );
				if( labelString ) {
					labelRect = CGRectMake( decodeX, data->rowRects.sampleFlagsRect.origin.y, kLabelWidth, data->rowRects.sampleFlagsRect.size.height );
					HIThemeDrawTextBox( labelString, &labelRect, &labelSmallTextInfo, c, kHIThemeOrientationNormal );
					CFRelease( labelString );
				}
				
				CGContextRestoreGState( c );
			}
			
			// Draw display-time-related rows.
			if( displayWidth > 2 ) {
				CGContextSaveGState( c );

				// Clip the next few items using a rectangular mask with a soft right edge.
				if( ( NULL == maskImage ) || ( maskWidth != displayWidth ) ) {
					CGImageRelease( maskImage );
					maskImage = createImageWithSoftRightEdge( displayWidth, 1, 5 );
					maskWidth = displayWidth;
				}
				maskRect = CGRectUnion( CGRectMake( floor(displayX), data->rowRects.framesInMediaDisplayRect.origin.y, floor(displayWidth), data->rowRects.framesInMediaDisplayRect.size.height ),
						   CGRectUnion( CGRectMake( floor(displayX), data->rowRects.sampleNumberDisplayRect.origin.y, floor(displayWidth), data->rowRects.sampleNumberDisplayRect.size.height ),
						                CGRectMake( floor(displayX), data->rowRects.displayTimeRect.origin.y, floor(displayWidth), data->rowRects.displayTimeRect.size.height ) ) );
				CGContextClipToMask( c, maskRect, maskImage );
				
				
				// Draw the frame thumbnail.
				if( displayWidth <= 10 )
					onlyIfCached = true;
				copyFrameThumbnail( data, sampleNumber, mediaSampleDescIndex, onlyIfCached, &frameThumbnail );
				if( frameThumbnail ) {
					CGRect thumbnailRect = CGRectMake( floor(displayX), data->rowRects.framesInMediaDisplayRect.origin.y, data->thumbnailSize.width, data->rowRects.framesInMediaDisplayRect.size.height );
					HIViewDrawCGImage( c, &thumbnailRect, frameThumbnail );
					CGImageRelease( frameThumbnail );
				}
			
				// Write the sample number.
				labelString = CFStringCreateWithFormat( NULL, NULL, CFSTR("%lld"), (long long)sampleNumber );
				if( labelString ) {
					labelRect = CGRectMake( floor(displayX), data->rowRects.sampleNumberDisplayRect.origin.y, kLabelWidth, data->rowRects.sampleNumberDisplayRect.size.height );
					HIThemeDrawTextBox( labelString, &labelRect, &labelTextInfo, c, kHIThemeOrientationNormal );
					CFRelease( labelString );
				}
				
				// Write the display time.
				labelString = createStringForTime( data->units, sampleDecodeTime + displayOffset, mediaTimeScale, false );
				if( labelString ) {
					labelRect = CGRectMake( floor(displayX+2), data->rowRects.displayTimeRect.origin.y, kLabelWidth, data->rowRects.displayTimeRect.size.height );
					HIThemeDrawTextBox( labelString, &labelRect, &labelTextInfo, c, kHIThemeOrientationNormal );
					CFRelease( labelString );
				}
				
				CGContextRestoreGState( c );
			}
			
			sampleDecodeTime += decodeDuration;
		}
		
		// Get the total decode duration so we can find out where to start the next sample table.
		err = QTSampleTableGetProperty( sampleTable,
				kQTPropertyClass_SampleTable,
				kQTSampleTablePropertyID_TotalDecodeDuration,
				sizeof( sampleTableTotalDecodeDuration ),
				&sampleTableTotalDecodeDuration,
				NULL );
		require_noerr( err, CantGetSampleTableProperty );
		
		startDecodeTime += sampleTableTotalDecodeDuration;
		QTSampleTableRelease( sampleTable );
		sampleTable = NULL;
	}
	
CantGetSampleTableProperty:
CantCopyMediaMutableSampleTable:
	CGContextRestoreGState( c );
	QTSampleTableRelease( sampleTable );
	CGImageRelease( maskImage );
	return err;
}

// Draw thumbnails of frames in track order.
static OSStatus drawTrackThumbnails(
	ChartViewData* data,
	CGContextRef c,
	float minimumTime,
	float maximumTime )
{
	Track videoTrack = data->videoTrack;
	Media videoMedia = GetTrackMedia( videoTrack );
	TimeScale trackTimeScale = GetMovieTimeScale( GetTrackMovie( videoTrack ) );
	TimeValue trackSampleTime, trackSampleDuration;
	HIThemeTextInfo labelTextInfo = { 0, kThemeStateActive, kThemeSystemFont, kHIThemeTextHorizontalFlushLeft, kHIThemeTextVerticalFlushTop, kHIThemeTextBoxOptionStronglyVertical, kHIThemeTextTruncationNone, 0, false };
	float maskWidth;
	CGImageRef maskImage = NULL;
		
	CGContextSaveGState( c );
	
	// Walk the track, looking for frame times in range.
	GetTrackNextInterestingTime( videoTrack, 
								 nextTimeMediaSample | nextTimeEdgeOK,
								 minimumTime * trackTimeScale,
								 fixed1,
								 &trackSampleTime,
								 &trackSampleDuration );
	while( ( trackSampleTime >= 0 ) && ( trackSampleDuration > 0 ) && ( trackSampleTime < maximumTime * trackTimeScale ) ) {
		float showX, showWidth;
		CGRect maskRect;
		TimeValue64 sampleDecodeTime;
		ItemCount mediaSampleDescIndex = 0;
		CGImageRef frameThumbnail = NULL;
		Boolean onlyIfCached = false;
		TimeValue64 displayTime = TrackTimeToMediaDisplayTime( trackSampleTime, videoTrack );
		SInt64 sampleNumber = 0;
		MediaDisplayTimeToSampleNum( videoMedia, displayTime, &sampleNumber, NULL, NULL );
		CFStringRef labelString;
		
		CGContextSaveGState( c );
		
		showX = floor( convertSecondsToXCoordinate( data, trackSampleTime, trackTimeScale ) );
		showWidth = floor( convertSecondsToWidth( data, trackSampleDuration, trackTimeScale ) );
		
		if( showWidth > 2 ) {
			if( ( NULL == maskImage ) || ( maskWidth != showWidth ) ) {
				CGImageRelease( maskImage );
				maskImage = createImageWithSoftRightEdge( showWidth, 1, 5 );
				maskWidth = showWidth;
			}
			maskRect = CGRectUnion( CGRectMake( showX, data->rowRects.framesInTrackRect.origin.y,     showWidth, data->rowRects.framesInTrackRect.size.height ),
			                        CGRectMake( showX, data->rowRects.sampleNumberTrackRect.origin.y, showWidth, data->rowRects.sampleNumberTrackRect.size.height ) );
			CGContextClipToMask( c, maskRect, maskImage );
			
			// Get the media sample description index.
			SampleNumToMediaDecodeTime( videoMedia, sampleNumber, &sampleDecodeTime, NULL );
			GetMediaSample2( videoMedia, NULL, 0, NULL, sampleDecodeTime,
							 NULL, NULL, NULL, NULL, &mediaSampleDescIndex, 1, NULL, NULL );
			
			// Draw the frame thumbnail.
			if( showWidth <= 10 )
				onlyIfCached = true;
			copyFrameThumbnail( data, sampleNumber, mediaSampleDescIndex, onlyIfCached, &frameThumbnail );
			if( frameThumbnail ) {
				CGRect thumbnailRect = CGRectMake( showX, data->rowRects.framesInTrackRect.origin.y, data->thumbnailSize.width, data->rowRects.framesInTrackRect.size.height );
				HIViewDrawCGImage( c, &thumbnailRect, frameThumbnail ); // like CGContextDrawImage, but handles flip
				CFRelease( frameThumbnail );
			}

			// Write the sample number.
			labelString = CFStringCreateWithFormat( NULL, NULL, CFSTR("%lld"), (long long)sampleNumber );
			if( labelString ) {
				CGRect labelRect = CGRectMake( showX, data->rowRects.sampleNumberTrackRect.origin.y, kLabelWidth, data->rowRects.sampleNumberTrackRect.size.height );
				HIThemeDrawTextBox( labelString, &labelRect, &labelTextInfo, c, kHIThemeOrientationNormal );
				CFRelease( labelString );
			}
		}
		
		CGContextRestoreGState( c );
		
		// Find the next frame.
		GetTrackNextInterestingTime( videoTrack, 
									 nextTimeMediaSample,
									 trackSampleTime,
									 fixed1,
									 &trackSampleTime,
									 &trackSampleDuration );
	}
	
	CGContextRestoreGState( c );
	CGImageRelease( maskImage );
	return noErr;
}

// Draw the chart.
static OSStatus ChartView_Draw (
	EventRef inEvent, 
	ChartViewData* inData )
{
	OSStatus err = noErr;
	CGContextRef context = NULL;
	Media videoMedia = GetTrackMedia( inData->videoTrack );
	float minimumTime, maximumTime, extraTime;
	
	err = GetEventParameter( inEvent, kEventParamCGContextRef, typeCGContextRef,
			NULL, sizeof( CGContextRef ), NULL, &context );
	require_noerr( err, ParameterMissing );
	
	err = HIViewGetBounds( inData->view, &inData->viewBounds );
	require_noerr( err, Failed );
	
	err = getAllCaptionRects( inData );
	require_noerr( err, Failed );
	
	// Work out the time range for which we need to display information.
	extraTime = GetMediaAdvanceDecodeTime( videoMedia ) / (float)GetMediaTimeScale( videoMedia );
	
	minimumTime = inData->startTimeInSeconds - kFrameWidth / inData->pixelsPerSecond - extraTime;
	if( minimumTime < 0 )
		minimumTime = 0;
	maximumTime = inData->startTimeInSeconds + inData->viewBounds.size.width / inData->pixelsPerSecond + extraTime;
	
	// Draw alternating white and pale blue bands every second.
	// Label the seconds at the bottom of the window, but throttle the label frequency to a power of 10 (seconds) so that 
	// we don't draw more than 15 labels.
	drawTimeMarks( inData, context, minimumTime, maximumTime );
	
	// Draw the edit list, which is a mapping from media display time ranges to track time ranges.
	// Label the track times when edits start.
	drawEditList( inData, context, minimumTime, maximumTime );
	
	// Draw the samples and their offsets from media decode time to media display time.
	// Label each sample with its decode time, display time, sample number, data size and sample flags
	// in the appropriate areas.
	// Draw thumbnails of frames in decode order.
	drawSampleInfo( inData, context, minimumTime, maximumTime );
	
	// Draw thumbnails of frames in track order.
	drawTrackThumbnails( inData, context, minimumTime, maximumTime );
	
Failed:
ParameterMissing:
	return err;
}

// Called when the video track is set.
static void updateForNewVideoTrack( ChartViewData *data )
{
	HIRect framesInTrackRect;
	Fixed trackWidth, trackHeight;
	
	// Update the thumbnail dimensions by scaling the video track to fit the vertical space we have.
	GetTrackDimensions( data->videoTrack, &trackWidth, &trackHeight );
	getOneCaptionRect( data, kFramesInTrackCaption, &framesInTrackRect );
	data->thumbnailSize.height = framesInTrackRect.size.height;
	data->thumbnailSize.width = ceil( Fix2X( trackWidth ) / Fix2X( trackHeight ) * data->thumbnailSize.height );
}

// Receive a value set using SetControlData.
static OSStatus ChartView_SetData( 
	EventRef                event,
	ChartViewData *         data )
{
	OSStatus err = noErr;
	OSType tag = 0;
	Ptr buffer = NULL;
	long size = 0;

	err = GetEventParameter( event, kEventParamControlDataTag, typeEnumeration, NULL, sizeof(OSType), NULL, &tag );
	require_noerr(err, bail);
	err = GetEventParameter( event, kEventParamControlDataBuffer, typePtr, NULL, sizeof(Ptr), NULL, &buffer );
	require_noerr(err, bail);
	err = GetEventParameter( event, kEventParamControlDataBufferSize, typeLongInteger, NULL, sizeof(long), NULL, &size );
	require_noerr(err, bail);
	
	switch( tag ) {
		case kChartView_VideoTrack:
			if( size == sizeof(Track) )
				data->videoTrack = *(Track *)buffer;
			else
				err = paramErr;
			updateForNewVideoTrack( data );
			break;
			
		case kChartView_Scale:
			if( size == sizeof(float) )
				data->pixelsPerSecond = *(float *)buffer;
			else
				err = paramErr;
			break;
			
		case kChartView_StartTime:
			if( size == sizeof(float) )
				data->startTimeInSeconds = *(float *)buffer;
			else
				err = paramErr;
			break;
			
		case kChartView_Units:
			if( size == sizeof(int) )
				data->units = *(int *)buffer;
			else
				err = paramErr;
			break;
			
		default:
			err = paramErr;
	}
	
	if( noErr == err )
		HIViewSetNeedsDisplay( data->view, true );
	
bail:
	return err;
}

// Return a value requested using GetControlData.
static OSStatus ChartView_GetData( 
	EventRef                event,
	ChartViewData *         data )
{
	OSStatus err = noErr;
	OSType tag = 0;
	Ptr buffer = NULL;
	long size = 0;

	err = GetEventParameter( event, kEventParamControlDataTag, typeEnumeration, NULL, sizeof(OSType), NULL, &tag );
	require_noerr(err, bail);
	err = GetEventParameter( event, kEventParamControlDataBuffer, typePtr, NULL, sizeof(Ptr), NULL, &buffer );
	require_noerr(err, bail);
	err = GetEventParameter( event, kEventParamControlDataBufferSize, typeLongInteger, NULL, sizeof(long), NULL, &size );
	require_noerr(err, bail);
	
	switch( tag ) {
		case kChartView_ThumbnailWidth:
			if( size == sizeof(int) )
				*(int *)buffer = (int)data->thumbnailSize.width;
			else
				err = paramErr;
			break;
			
		default:
			err = paramErr;
	}
	
bail:
	return err;
}

// Event handler for our view.
static OSStatus ChartView_HandleEvent(
    EventHandlerCallRef     inCallRef,
    EventRef                inEvent,
    void*                   inUserData )
{
    OSStatus                err = eventNotHandledErr;
    UInt32                  eventClass = GetEventClass( inEvent );
    UInt32                  eventKind = GetEventKind( inEvent );
    ChartViewData *         data = (ChartViewData*) inUserData;
    switch ( eventClass )
    {
        case kEventClassHIObject:
        {
            switch ( eventKind )
            {
                case kEventHIObjectConstruct:
                    err = ChartView_Construct( inEvent );
                    break;
                case kEventHIObjectInitialize:
                    err = ChartView_Initialize( inCallRef, inEvent, data );
                    break;
                case kEventHIObjectDestruct:
                    err = ChartView_Destruct( inEvent, data );
                    break;
            }
        }
        break;
                
        case kEventClassControl:
        {
            switch ( eventKind )
            {
                case kEventControlInitialize:
                    err = noErr;
                    break;
                case kEventControlDraw:
                    err = ChartView_Draw( inEvent, data );
                    break;
				case kEventControlSetData:
					err = ChartView_SetData( inEvent, data );
					break;		
				case kEventControlGetData:
					err = ChartView_GetData( inEvent, data );
					break;		
            }
        }
        break;
    }
        
    return err;
}

// Setter utilities so that other files don't have to mess with SetControlData.

extern OSStatus ChartView_SetVideoTrack( HIViewRef view, Track videoTrack )
{
	return SetControlData( view, kControlEntireControl, kChartView_VideoTrack, sizeof( videoTrack ), &videoTrack );
}

extern OSStatus ChartView_SetScale( HIViewRef view, float pixelsPerSecond )
{
	return SetControlData( view, kControlEntireControl, kChartView_Scale, sizeof( pixelsPerSecond ), &pixelsPerSecond );
}

extern OSStatus ChartView_SetStartTime( HIViewRef view, float startTimeInSeconds )
{
	return SetControlData( view, kControlEntireControl, kChartView_StartTime, sizeof( startTimeInSeconds ), &startTimeInSeconds );
}

extern OSStatus ChartView_SetUnits( HIViewRef view, int units )
{
	return SetControlData( view, kControlEntireControl, kChartView_Units, sizeof( units ), &units );
}

extern OSStatus ChartView_GetThumbnailWidth( HIViewRef view, int *thumbnailWidth )
{
	return GetControlData( view, kControlEntireControl, kChartView_ThumbnailWidth, sizeof( int ), thumbnailWidth, NULL );
}

// Register our HIView subclass.
extern OSStatus ChartView_Register(void)
{
    OSStatus                err = noErr;
    static HIObjectClassRef sChartViewClassRef = NULL;
    if ( sChartViewClassRef == NULL )
    {
        EventTypeSpec       eventList[] = {
            { kEventClassHIObject, kEventHIObjectConstruct },
            { kEventClassHIObject, kEventHIObjectInitialize },
            { kEventClassHIObject, kEventHIObjectDestruct },
            { kEventClassControl, kEventControlInitialize },
            { kEventClassControl, kEventControlDraw },
			{ kEventClassControl, kEventControlGetData },
			{ kEventClassControl, kEventControlSetData } };
        err = HIObjectRegisterSubclass(
            kChartViewClassID,          // class ID
            kHIViewClassID,             // base class ID
            0,                          // option bits
            ChartView_HandleEvent,      // construct proc
            GetEventTypeCount( eventList ),
            eventList,
            NULL,                       // construct data,
            &sChartViewClassRef );
    }
        
    return err;
}

