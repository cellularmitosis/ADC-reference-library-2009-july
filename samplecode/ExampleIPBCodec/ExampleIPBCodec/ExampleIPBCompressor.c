/*

File: ExampleIPBCompressor.c, part of ExampleIPBCodec

Abstract: Example Image Compressor using new IPB-capable component interface in QuickTime 7.

Version: 1.0

© Copyright 2005 Apple Computer, Inc. All rights reserved.

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

*/


#if __APPLE_CC__
    #include <QuickTime/QuickTime.h>
#else
    #include <ConditionalMacros.h>
    #include <Endian.h>
    #include <ImageCodec.h>
#endif

#include "ExampleIPBCodecVersion.h"
#include "NaiveEncoder.h"
#include "NaiveDecoder.h"

enum {
	kMaxQueuedFrames = 10	// maximum number of source frames we'll hold at a time
};

typedef struct {
	ComponentInstance				self;
	ComponentInstance				target;
	
	ICMCompressorSessionRef 		session; // NOTE: we do not need to retain or release this
	ICMCompressionSessionOptionsRef	sessionOptions;
	
	long							width;
	long							height;
	size_t							maxEncodedDataSize;
	int								nextDecodeNumber;
	
	// We maintain a queue of source frames that we have yet to encode, so that we can reorder them.
	// This can also be used as a lookahead queue to guide rate control and encoding tricks.
	// You can use CFArray or STL or whatever you like to hold these; this example uses a simple array.
	// Note that the compressor retains the source frames when adding them and releases them when removing them.
	ICMCompressorSourceFrameRef		queuedFrames[ kMaxQueuedFrames ];
	Boolean							queuedFrameKeyFrame[ kMaxQueuedFrames ];
	int								queuedFrameCount;
	int								keyFrameCountDown;
	
	// We also track the decoder state so that we know what the decoder can use for prediction.
	struct InternalPixelBuffer		storedFrameArray[kMaxStoredFrames];
	Boolean							storedFrameAvailable[kMaxStoredFrames]; 
	int								nextStorageIndex;
	
	struct InternalPixelBuffer		currentFrame; // holds the frame currently being encoded
} ExampleIPBCompressorGlobalsRecord, *ExampleIPBCompressorGlobals;

// Setup required for ComponentDispatchHelper.c
#define IMAGECODEC_BASENAME() 		ExampleIPB_C
#define IMAGECODEC_GLOBALS() 		ExampleIPBCompressorGlobals storage

#define CALLCOMPONENT_BASENAME()	IMAGECODEC_BASENAME()
#define	CALLCOMPONENT_GLOBALS()		IMAGECODEC_GLOBALS()

#define QT_BASENAME()				CALLCOMPONENT_BASENAME()
#define QT_GLOBALS()				CALLCOMPONENT_GLOBALS()

#define COMPONENT_UPP_PREFIX()		uppImageCodec
#define COMPONENT_DISPATCH_FILE		"ExampleIPBCompressorDispatch.h"
#define COMPONENT_SELECT_PREFIX()  	kImageCodec

#if __APPLE_CC__
#include <CoreServices/Components.k.h>
#include <QuickTime/ImageCodec.k.h>
#include <QuickTime/ImageCompression.k.h>
#include <QuickTime/ComponentDispatchHelper.c>
#else
#include <Components.k.h>
#include <ImageCodec.k.h>
#include <ImageCompression.k.h>
#include <ComponentDispatchHelper.c>
#endif

// Prototypes for local utility functions defined later:
static ComponentResult addFrameToQueue( ExampleIPBCompressorGlobals glob, ICMCompressorSourceFrameRef sourceFrame, Boolean forceKeyFrame );
static Boolean isFrameInQueue( ExampleIPBCompressorGlobals glob, ICMCompressorSourceFrameRef sourceFrame );
static void removeFrameFromQueue( ExampleIPBCompressorGlobals glob, ICMCompressorSourceFrameRef sourceFrame );
static void emptyQueue( ExampleIPBCompressorGlobals glob );
static ComponentResult encodeSomeSourceFrames( ExampleIPBCompressorGlobals glob );
static ComponentResult encodeThisSourceFrame( ExampleIPBCompressorGlobals glob, ICMCompressorSourceFrameRef sourceFrame, ICMFrameType frameType, Boolean keyFrame, Boolean partialSyncFrame, Boolean droppableFrame );

// Open a new instance of the component.
// Allocate component instance storage ("globals") and associate it with the new instance so that other
// calls will receive it.  
// Note that "one-shot" component calls like CallComponentVersion and ImageCodecGetCodecInfo work by opening
// an instance, making that call and then closing the instance, so you should avoid performing very expensive
// initialization operations in a component's Open function.
ComponentResult 
ExampleIPB_COpen(
  ExampleIPBCompressorGlobals glob, 
  ComponentInstance self )
{
	ComponentResult err = noErr;
	
	glob = calloc( sizeof( ExampleIPBCompressorGlobalsRecord ), 1 );
	if( ! glob ) {
		err = memFullErr;
		goto bail;
	}

	SetComponentInstanceStorage( self, (Handle)glob );

	glob->self = self;
	glob->target = self;
	
	glob->nextDecodeNumber = 1;
	
bail:
	return err;
}

// Closes the instance of the component.
// Release all storage associated with this instance.
// Note that if a component's Open function fails with an error, its Close function will still be called.
ComponentResult 
ExampleIPB_CClose(
  ExampleIPBCompressorGlobals glob, 
  ComponentInstance self )
{
	if( glob ) {
		int frameIndex;
		
		// There should be no queued frames left, but:
		emptyQueue( glob );
		
		for( frameIndex = 0; frameIndex < kMaxStoredFrames; frameIndex++ ) {
			freeInternalPixelBuffer( &glob->storedFrameArray[frameIndex] );
		}
		freeInternalPixelBuffer( &glob->currentFrame );
		
		ICMCompressionSessionOptionsRelease( glob->sessionOptions );
		glob->sessionOptions = NULL;
		
		free( glob );
	}
	
	return noErr;
}

// Return the version of the component.
// This does not need to correspond in any way to the human-readable version numbers found in
// Get Info windows, etc.  
// The principal use of component version numbers is to choose between multiple installed versions
// of the same component: if the component manager sees two components with the same type, subtype
// and manufacturer codes and either has the componentDoAutoVersion registration flag set,
// it will deregister the one with the older version.  (If componentAutoVersionIncludeFlags is also 
// set, it only does this when the component flags also match.)
// By convention, the high short of the component version is the interface version, which Apple
// bumps when there is a major change in the interface.  
// We recommend bumping the low short of the component version every time you ship a release of a component.
// The version number in the 'thng' resource should be the same as the number returned by this function.
ComponentResult 
ExampleIPB_CVersion(ExampleIPBCompressorGlobals glob)
{
	return kExampleIPBCompressorVersion;
}

// Sets the target for a component instance.
// When a component wants to make a component call on itself, it should make that call on its target.
// This allows other components to delegate to the component.
// By default, a component's target is itself -- see the Open function.
ComponentResult 
ExampleIPB_CTarget(ExampleIPBCompressorGlobals glob, ComponentInstance target)
{
	glob->target = target;
	return noErr;
}

// Your component receives the ImageCodecGetCodecInfo request whenever an application calls the Image Compression Manager's GetCodecInfo function.
// Your component should return a formatted compressor information structure defining its capabilities.
// Both compressors and decompressors may receive this request.
ComponentResult 
ExampleIPB_CGetCodecInfo(ExampleIPBCompressorGlobals glob, CodecInfo *info)
{
	OSErr err = noErr;

	if (info == NULL) {
		err = paramErr;
	}
	else {
		CodecInfo **tempCodecInfo;

		err = GetComponentResource((Component)glob->self, codecInfoResourceType, 256, (Handle *)&tempCodecInfo);
		if (err == noErr) {
			*info = **tempCodecInfo;
			DisposeHandle((Handle)tempCodecInfo);
		}
	}

	return err;
}

// Return the maximum size of compressed data for the image in bytes.
// Note that this function is only used when the ICM client is using a compression sequence
// (created with CompressSequenceBegin, not ICMCompressionSessionCreate).
// Nevertheless, it's important to implement it because such clients need to know how much 
// memory to allocate for compressed frame buffers.
ComponentResult 
ExampleIPB_CGetMaxCompressionSize(
	ExampleIPBCompressorGlobals glob, 
	PixMapHandle        src,
	const Rect *        srcRect,
	short               depth,
	CodecQ              quality,
	long *              size)
{
	ComponentResult err = noErr;
	size_t maxBytes = 0;
	
	if( ! size )
		return paramErr;

	NaiveEncoder_GetMaxEncodedDataSize(
			srcRect->right - srcRect->left,
			srcRect->bottom - srcRect->top,
			&maxBytes );
	*size = maxBytes;
	
bail:
	return err;
}

// Utility to add an SInt32 to a CFMutableDictionary.
static void
addNumberToDictionary( CFMutableDictionaryRef dictionary, CFStringRef key, SInt32 numberSInt32 )
{
	CFNumberRef number = CFNumberCreate( NULL, kCFNumberSInt32Type, &numberSInt32 );
	if( ! number ) 
		return;
	CFDictionaryAddValue( dictionary, key, number );
	CFRelease( number );
}

// Utility to add a double to a CFMutableDictionary.
static void
addDoubleToDictionary( CFMutableDictionaryRef dictionary, CFStringRef key, double numberDouble )
{
	CFNumberRef number = CFNumberCreate( NULL, kCFNumberDoubleType, &numberDouble );
	if( ! number ) 
		return;
	CFDictionaryAddValue( dictionary, key, number );
	CFRelease( number );
}

// Utility to round up to a multiple of 16.
static int 
roundUpToMultipleOf16( int n )
{
	if( 0 != ( n & 15 ) )
		n = ( n + 15 ) & ~15;
	return n;
}

// Create a dictionary that describes the kinds of pixel buffers that we want to receive.
// The important keys to add are kCVPixelBufferPixelFormatTypeKey, 
// kCVPixelBufferWidthKey and kCVPixelBufferHeightKey.
// Many compressors will also want to set kCVPixelBufferExtendedPixels, 
// kCVPixelBufferBytesPerRowAlignmentKey, kCVImageBufferGammaLevelKey and kCVImageBufferYCbCrMatrixKey.
static OSStatus 
createPixelBufferAttributesDictionary( SInt32 width, SInt32 height,
		const OSType *pixelFormatList, int pixelFormatCount,
		CFMutableDictionaryRef *pixelBufferAttributesOut )
{
	OSStatus err = memFullErr;
	int i;
	CFMutableDictionaryRef pixelBufferAttributes = NULL;
	CFNumberRef number = NULL;
	CFMutableArrayRef array = NULL;
	SInt32 widthRoundedUp, heightRoundedUp, extendRight, extendBottom;
	
	pixelBufferAttributes = CFDictionaryCreateMutable( 
			NULL, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks );
	if( ! pixelBufferAttributes ) goto bail;
	
	array = CFArrayCreateMutable( NULL, 0, &kCFTypeArrayCallBacks );
	if( ! array ) goto bail;
	
	// Under kCVPixelBufferPixelFormatTypeKey, add the list of source pixel formats.
	// This can be a CFNumber or a CFArray of CFNumbers.
	for( i = 0; i < pixelFormatCount; i++ ) {
		number = CFNumberCreate( NULL, kCFNumberSInt32Type, &pixelFormatList[i] );
		if( ! number ) goto bail;
		
		CFArrayAppendValue( array, number );
		
		CFRelease( number );
		number = NULL;
	}
	
	CFDictionaryAddValue( pixelBufferAttributes, kCVPixelBufferPixelFormatTypeKey, array );
	CFRelease( array );
	array = NULL;
	
	// Add kCVPixelBufferWidthKey and kCVPixelBufferHeightKey to specify the dimensions
	// of the source pixel buffers.  Normally this is the same as the compression target dimensions.
	addNumberToDictionary( pixelBufferAttributes, kCVPixelBufferWidthKey, width );
	addNumberToDictionary( pixelBufferAttributes, kCVPixelBufferHeightKey, height );
	
	// If you want to require that extra scratch pixels be allocated on the edges of source pixel buffers,
	// add the kCVPixelBufferExtendedPixels{Left,Top,Right,Bottom}Keys to indicate how much.
	// Internally our encoded can only support multiples of 16x16 macroblocks; 
	// we will round the compression dimensions up to a multiple of 16x16 and encode that size.  
	// (Note that if your compressor needs to copy the pixels anyhow (eg, in order to convert to a different 
	// format) you may get better performance if your copy routine does not require extended pixels.)
	widthRoundedUp = roundUpToMultipleOf16( width );
	heightRoundedUp = roundUpToMultipleOf16( height );
	extendRight = widthRoundedUp - width;
	extendBottom = heightRoundedUp - height;
	if( extendRight || extendBottom ) {
		addNumberToDictionary( pixelBufferAttributes, kCVPixelBufferExtendedPixelsRightKey, extendRight );
		addNumberToDictionary( pixelBufferAttributes, kCVPixelBufferExtendedPixelsBottomKey, extendBottom );
	}
	
	// Altivec code is most efficient reading data aligned at addresses that are multiples of 16.
	// Pretending that we have some altivec code, we set kCVPixelBufferBytesPerRowAlignmentKey to
	// ensure that each row of pixels starts at a 16-byte-aligned address.
	addNumberToDictionary( pixelBufferAttributes, kCVPixelBufferBytesPerRowAlignmentKey, 16 );
	
	// This codec accepts YCbCr input in the form of '2vuy' format pixel buffers.
	// We recommend explicitly defining the gamma level and YCbCr matrix that should be used.
	addDoubleToDictionary( pixelBufferAttributes, kCVImageBufferGammaLevelKey, 2.2 );
	CFDictionaryAddValue( pixelBufferAttributes, kCVImageBufferYCbCrMatrixKey, kCVImageBufferYCbCrMatrix_ITU_R_601_4 );
	
	err = noErr;
	*pixelBufferAttributesOut = pixelBufferAttributes;
	pixelBufferAttributes = NULL;
	
bail:
	if( pixelBufferAttributes ) CFRelease( pixelBufferAttributes );
	if( number ) CFRelease( number );
	if( array ) CFRelease( array );
	return err;
}

// Prepare to compress frames.
// Compressor should record session and sessionOptions for use in later calls.
// Compressor may modify imageDescription at this point.
// Compressor may create and return pixel buffer options.
ComponentResult 
ExampleIPB_CPrepareToCompressFrames(
  ExampleIPBCompressorGlobals glob,
  ICMCompressorSessionRef session,
  ICMCompressionSessionOptionsRef sessionOptions,
  ImageDescriptionHandle imageDescription,
  void *reserved,
  CFDictionaryRef *compressorPixelBufferAttributesOut)
{
	ComponentResult err = noErr;
	CFMutableDictionaryRef compressorPixelBufferAttributes = NULL;
	OSType pixelFormatList[] = { k422YpCbCr8PixelFormat }; // also known as '2vuy'
	Fixed gammaLevel;
	int frameIndex;
	SInt32 widthRoundedUp, heightRoundedUp;
	
	// Record the compressor session for later calls to the ICM.
	// Note: this is NOT a CF type and should NOT be CFRetained or CFReleased.
	glob->session = session;
	
	// Retain the session options for later use.
	ICMCompressionSessionOptionsRelease( glob->sessionOptions );
	glob->sessionOptions = sessionOptions;
	ICMCompressionSessionOptionsRetain( glob->sessionOptions );
	
	// Modify imageDescription here if needed.
	// We'll set the image description gamma level to say "2.2".
	gammaLevel = kQTCCIR601VideoGammaLevel;
	err = ICMImageDescriptionSetProperty( imageDescription,
			kQTPropertyClass_ImageDescription,
			kICMImageDescriptionPropertyID_GammaLevel,
			sizeof(gammaLevel),
			&gammaLevel );
	if( err )
		goto bail;
	
	// Record the dimensions from the image description.
	glob->width = (*imageDescription)->width;
	glob->height = (*imageDescription)->height;
	
	// Create a pixel buffer attributes dictionary.
	err = createPixelBufferAttributesDictionary( glob->width, glob->height, 
			pixelFormatList, sizeof(pixelFormatList) / sizeof(OSType),
			&compressorPixelBufferAttributes );
	if( err ) 
		goto bail;

	*compressorPixelBufferAttributesOut = compressorPixelBufferAttributes;
	compressorPixelBufferAttributes = NULL;
	
	// Work out the upper bound on encoded frame data size -- we'll allocate buffers of this size.
	NaiveEncoder_GetMaxEncodedDataSize( glob->width, glob->height, &glob->maxEncodedDataSize );
	
	glob->keyFrameCountDown = ICMCompressionSessionOptionsGetMaxKeyFrameInterval( glob->sessionOptions );
	
	// Create internal pixel buffers for the encoder and decoder.
	widthRoundedUp = roundUpToMultipleOf16( glob->width );
	heightRoundedUp = roundUpToMultipleOf16( glob->height );
	
	for( frameIndex = 0; frameIndex < kMaxStoredFrames; frameIndex++ ) {
		err = callocInternalPixelBuffer( widthRoundedUp, heightRoundedUp, &glob->storedFrameArray[frameIndex] );
		if( err )
			goto bail;
	}
	err = callocInternalPixelBuffer( widthRoundedUp, heightRoundedUp, &glob->currentFrame );
	if( err )
		goto bail;
	
bail:
	if( compressorPixelBufferAttributes ) CFRelease( compressorPixelBufferAttributes );
	
	return err;
}

// Presents the compressor with a frame to encode.
// The compressor may encode the frame immediately or queue it for later encoding.
// If the compressor queues the frame for later decode, it must retain it (by calling ICMCompressorSourceFrameRetain)
// and release it when it is done with it (by calling ICMCompressorSourceFrameRelease).
// Pixel buffers are guaranteed to conform to the pixel buffer options returned by ImageCodecPrepareToCompressFrames.
ComponentResult 
ExampleIPB_CEncodeFrame(
  ExampleIPBCompressorGlobals glob,
  ICMCompressorSourceFrameRef sourceFrame,
  UInt32 flags )
{
	ComponentResult err = noErr;
	ICMCompressionFrameOptionsRef frameOptions;
	Boolean forceKeyFrame;
	
	// Determine whether this frame must be a key frame.
	// If the session options do not allow temporal compression, all frames must be key frames.
	// Frame options can be used to force specific frames to be key frames.
	// We also count down from the "maximum key frame interval" to determine when it is time
	// to generate a regular key frame.  
	// The compressor is also allowed to decide to generate more key frames than required, but not fewer.
	frameOptions = ICMCompressorSourceFrameGetFrameOptions( sourceFrame );
	forceKeyFrame = 
			( ! ICMCompressionSessionOptionsGetAllowTemporalCompression( glob->sessionOptions ) )
		 || ICMCompressionFrameOptionsGetForceKeyFrame( frameOptions )
		 || ( 0 == glob->keyFrameCountDown );
	
	if( forceKeyFrame )
		glob->keyFrameCountDown = ICMCompressionSessionOptionsGetMaxKeyFrameInterval( glob->sessionOptions );
	else
		glob->keyFrameCountDown -= 1;
	
	// Put this frame on the end of the queue.
	err = addFrameToQueue( glob, sourceFrame, forceKeyFrame );
	if( err ) goto bail;
	
	// If the queue is full, we have to complete a frame.
	if( glob->queuedFrameCount >= kMaxQueuedFrames ) {
		err = encodeSomeSourceFrames( glob );
		if( err ) goto bail;
	}

bail:
	return err;
}

// Directs the compressor to finish with a queued source frame, either emitting or dropping it.
// This frame does not necessarily need to be the first or only source frame emitted or dropped
// during this call, but the compressor must call either ICMCompressorSessionDropFrame or 
// ICMCompressorSessionEmitEncodedFrame with this frame before returning.
// The ICM will call this function to force frames to be encoded for the following reasons:
//   - the maximum frame delay count or maximum frame delay time in the sessionOptions
//     does not permit more frames to be queued
//   - the client has called ICMCompressionSessionCompleteFrames.
ComponentResult 
ExampleIPB_CCompleteFrame( 
  ExampleIPBCompressorGlobals glob,
  ICMCompressorSourceFrameRef sourceFrame,
  UInt32 flags )
{
	ComponentResult err = noErr;
	
	// Encode frames until sourceFrame is no longer in the queue.
	while( isFrameInQueue( glob, sourceFrame ) ) {
		err = encodeSomeSourceFrames( glob );
		if( err ) goto bail;
	}
	
bail:
	return err;
}

// Put this frame on the end of the queue.
static ComponentResult
addFrameToQueue( ExampleIPBCompressorGlobals glob, ICMCompressorSourceFrameRef sourceFrame, Boolean forceKeyFrame )
{
	if( kMaxQueuedFrames == glob->queuedFrameCount )
		return paramErr; // already full
	
	ICMCompressorSourceFrameRetain( sourceFrame );
	glob->queuedFrames[ glob->queuedFrameCount ] = sourceFrame;
	glob->queuedFrameKeyFrame[ glob->queuedFrameCount ] = forceKeyFrame;
	glob->queuedFrameCount++;
	
	return noErr;
}

// Test whether this source frame is in our queue.
static Boolean
isFrameInQueue( ExampleIPBCompressorGlobals glob, ICMCompressorSourceFrameRef sourceFrame )
{
	int index;
	for( index = 0; index < glob->queuedFrameCount; index++ ) {
		if( glob->queuedFrames[ index ] == sourceFrame )
			return true;
	}
	return false;
}

// Remove a single frame from the queue.
static void
removeFrameFromQueue( ExampleIPBCompressorGlobals glob, ICMCompressorSourceFrameRef sourceFrame )
{
	int index;
	for( index = 0; index < glob->queuedFrameCount; index++ ) {
		if( glob->queuedFrames[ index ] == sourceFrame ) {
			ICMCompressorSourceFrameRelease( glob->queuedFrames[ index ] );
			memmove( &glob->queuedFrames[ index ], 
					 &glob->queuedFrames[ index+1 ], 
					 ( glob->queuedFrameCount - index - 1 ) * sizeof( ICMCompressorSourceFrameRef ) );
			memmove( &glob->queuedFrameKeyFrame[ index ], 
					 &glob->queuedFrameKeyFrame[ index+1 ], 
					 ( glob->queuedFrameCount - index - 1 ) * sizeof( Boolean ) );
			glob->queuedFrames[ glob->queuedFrameCount-1 ] = NULL;
			glob->queuedFrameCount--;
			return;
		}
	}
}

// Release all frames in the queue.
static void
emptyQueue( ExampleIPBCompressorGlobals glob )
{
	int index;
	for( index = 0; index < glob->queuedFrameCount; index++ ) {
		ICMCompressorSourceFrameRelease( glob->queuedFrames[ index ] );
		glob->queuedFrames[ index ] = NULL;
	}
	glob->queuedFrameCount = 0;
}

// Test whether any source frames in the queue have earlier display numbers than displayNumber.
static Boolean
doesQueueContainEarlierDisplayNumbers( ExampleIPBCompressorGlobals glob, long displayNumber )
{
	int index;
	for( index = 0; index < glob->queuedFrameCount; index++ ) {
		if( ICMCompressorSourceFrameGetDisplayNumber( glob->queuedFrames[ index ] ) < displayNumber )
			return true;
	}
	return false;
}

static ICMFrameType
getRequestedFrameType( ICMCompressorSourceFrameRef sourceFrame )
{
	ICMCompressionFrameOptionsRef frameOptions = ICMCompressorSourceFrameGetFrameOptions( sourceFrame );
	ICMFrameType requestedFrameType = frameOptions ? ICMCompressionFrameOptionsGetFrameType( frameOptions ) : kICMFrameType_Unknown;
	return requestedFrameType;
}

// Select one or more frames, encode them and remove them from the source frame queue.
// The source frame queue contains frames in display order, and the order in which we 
// encode them will become the decode order, so this is where the frame reordering pattern 
// is defined.  
//
// If frame reordering has not been enabled by the client, then decode order must be the same 
// as display order, so we always encode the first frame in the source frame queue next.
// Otherwise, we look ahead for the a frame to encode as the next I or P frame,
// encode it, and then encode all the earlier frames as B frames.  
// Clients may request particular frames be I/P/B frames through frame options; we honor those 
// requests if possible.  
// Given no other guidance, we pick a regular B frame spacing.  
// A smarter codec could pick good I/P frames based on analysis of the image data.
//
// Note that although this example basically follows the MPEG-2 IPB reordering patterns,
// compressors are allowed to use more free-form frame reordering if they like.  
static ComponentResult
encodeSomeSourceFrames( ExampleIPBCompressorGlobals glob )
{
	ComponentResult err = noErr;
	ICMCompressorSourceFrameRef sourceFrame = NULL;
	int nextFrameIndex;
	ICMFrameType frameType;
	Boolean keyFrame;
	Boolean partialSyncFrame;
	Boolean droppableFrame;
	int i;
	
	// This function should not be called when there are no queued source frames.
	if( 0 == glob->queuedFrameCount ) {
		err = paramErr;
		goto bail;
	}
	
	// If frame reordering has not been enabled by the client, then decode order must be the same 
	// as display order, so we always encode the first frame in the source frame queue next.
	if( ! ICMCompressionSessionOptionsGetAllowFrameReordering( glob->sessionOptions ) ) {
		ICMFrameType requestedFrameType = getRequestedFrameType( glob->queuedFrames[ 0 ] );
		nextFrameIndex = 0;
		if( glob->queuedFrameKeyFrame[ 0 ]
		 || ( kICMFrameType_I == requestedFrameType ) ) {
			// Encode this frame as a key frame.
			frameType = kICMFrameType_I;
			keyFrame = true;
			partialSyncFrame = false;
			droppableFrame = false;
		}
		else {
			// Encode this frame as a P frame.
			frameType = kICMFrameType_P;
			keyFrame = false;
			partialSyncFrame = false;
			droppableFrame = false;
		}
	}
	else {
		const int kDefaultMaxConsecutiveBFrames = 2;
		
		// Otherwise, we look ahead for the a frame to encode as the next P frame (or non-key I frame),
		// encode it, and then encode all the earlier frames as B frames.  
		
		for( i = 0; i < glob->queuedFrameCount; i++ ) {
			ICMFrameType requestedFrameType = getRequestedFrameType( glob->queuedFrames[ i ] );
			if( glob->queuedFrameKeyFrame[ i ] ) {
				// Encode this frame as a key frame.
				nextFrameIndex = i;
				frameType = kICMFrameType_I;
				keyFrame = true;
				partialSyncFrame = false;
				droppableFrame = false;
				break;
			}
			else if( kICMFrameType_P == requestedFrameType ) {
				// OK, generate a P frame.
				nextFrameIndex = i;
				frameType = kICMFrameType_P;
				keyFrame = false;
				partialSyncFrame = false;
				droppableFrame = false;
				break;
			}
			else if( kICMFrameType_I == requestedFrameType ) {
				// Generate an I frame...
				nextFrameIndex = i;
				frameType = kICMFrameType_I;
				if( i > 0 ) {
					// If we're going to encode B frames with earlier display times, 
					// we can choose to make this an open-GOP I frame.
					keyFrame = false;
					partialSyncFrame = true;
				}
				else {
					keyFrame = true;
					partialSyncFrame = false;
				}
				droppableFrame = false;
				break;
			}
			else if( kICMFrameType_B == requestedFrameType ) {
				// OK, we'll encode this as a B frame after we've encoded the next I or P frame.
			}
			else if( ( i >= kDefaultMaxConsecutiveBFrames ) || ( i+1 == glob->queuedFrameCount ) ) {
				// Generate a regular P frame here.
				nextFrameIndex = i;
				frameType = kICMFrameType_P;
				keyFrame = false;
				partialSyncFrame = false;
				droppableFrame = false;
				break;
			}
		}
	}
	
	if( 1 == glob->nextDecodeNumber ) {
		// The first frame we encode must be a key frame.
		frameType = kICMFrameType_I;
		keyFrame = true;
		partialSyncFrame = false;
		droppableFrame = false;
	}
	glob->nextDecodeNumber += 1;
	
	sourceFrame = glob->queuedFrames[ nextFrameIndex ];
	err = encodeThisSourceFrame( glob, sourceFrame, frameType, keyFrame, partialSyncFrame, droppableFrame );
	if( err )
		goto bail;
	
	// Take the source frame out of the queue and release it.
	removeFrameFromQueue( glob, sourceFrame );
	
	// Encode the frames before that one as B frames.
	frameType = kICMFrameType_B;
	keyFrame = false;
	partialSyncFrame = false;
	droppableFrame = true;
	for( i = 0; i < nextFrameIndex; i++ ) {
		sourceFrame = glob->queuedFrames[ 0 ];
		
		err = encodeThisSourceFrame( glob, sourceFrame, frameType, keyFrame, partialSyncFrame, droppableFrame );
		if( err )
			goto bail;
		
		// Take the source frame out of the queue and release it.
		removeFrameFromQueue( glob, sourceFrame );
	}
	
bail:
	return err;
}

// Decide upon a byte budget for this frame.
// We'll use the data rate settings if we can; otherwise, we'll use the compression quality.
// This is simplistic -- a sophisticated compressor would average bit rate allocation over many frames.
// In such a compressor it would be a good idea to also support the hard data rate limits in the session options
// (see kICMCompressionSessionOptionsPropertyID_DataRateLimits in ImageCompression.h).
static ComponentResult
getByteBudget( 
	ExampleIPBCompressorGlobals glob, 
	ICMCompressorSourceFrameRef sourceFrame,
	size_t *byteBudgetOut )
{
	ComponentResult err = noErr;
	TimeValue64 frameDisplayDuration = 0;
	TimeScale timescale = 0;
	ICMValidTimeFlags validTimeFlags = 0;
	SInt32 averageDataRate = 0;
	CodecQ quality = codecNormalQuality;
	size_t maxEncodedDataSize, minEncodedDataSize;
	
	// If we have a known frame duration and an average data rate, use them to guide the byte budget.
	err = ICMCompressorSourceFrameGetDisplayTimeStampAndDuration( sourceFrame,
			NULL, &frameDisplayDuration, &timescale, &validTimeFlags );
	if( err )
		goto bail;
	
	if( glob->sessionOptions
	 && ( validTimeFlags & kICMValidTime_DisplayDurationIsValid )
	 && ( frameDisplayDuration != 0 )
	 && ( timescale != 0 ) ) {
		err = ICMCompressionSessionOptionsGetProperty( glob->sessionOptions,
				kQTPropertyClass_ICMCompressionSessionOptions,
				kICMCompressionSessionOptionsPropertyID_AverageDataRate,
				sizeof( averageDataRate ),
				&averageDataRate,
				NULL );
		if( err )
			goto bail;
		
		if( averageDataRate ) {
			// bytes = (bytes per second) * (seconds)
			*byteBudgetOut = averageDataRate * frameDisplayDuration / timescale;
			goto bail;
		}
	}
	
	// Otherwise, use the compression quality setting to guide this: in interests of simplicity, 
	// let's use the compression quality to interpolate linearly between the best-case and worst-case compression sizes.
	if( glob->sessionOptions ) {
		err = ICMCompressionSessionOptionsGetProperty( glob->sessionOptions,
				kQTPropertyClass_ICMCompressionSessionOptions,
				kICMCompressionSessionOptionsPropertyID_Quality,
				sizeof( quality ),
				&quality,
				NULL );
		if( err )
			goto bail;
	}
	
	NaiveEncoder_GetMaxEncodedDataSize( glob->width, glob->height, &maxEncodedDataSize );
	NaiveEncoder_GetMinEncodedDataSize( glob->width, glob->height, &minEncodedDataSize );
	
	*byteBudgetOut = minEncodedDataSize + ((double)( maxEncodedDataSize - minEncodedDataSize )) * quality / codecLosslessQuality;
	
bail:
	return err;
}

static ComponentResult
encodeThisSourceFrame( 
	ExampleIPBCompressorGlobals glob, 
	ICMCompressorSourceFrameRef sourceFrame,
	ICMFrameType frameType,
	Boolean keyFrame,
	Boolean partialSyncFrame,
	Boolean droppableFrame )
{
	ComponentResult err = noErr;
	ICMMutableEncodedFrameRef encodedFrame = NULL;
	UInt8 *dataPtr;
	const UInt8 *decoderDataPtr;
	size_t dataSize = 0;
	MediaSampleFlags mediaSampleFlags;
	CVPixelBufferRef sourcePixelBuffer = NULL;
	size_t byteBudget;
	int storageIndex = 0;
	
	// Create the buffer for the encoded frame.
	err = ICMEncodedFrameCreateMutable( glob->session, sourceFrame, glob->maxEncodedDataSize, &encodedFrame );
	if( err )
		goto bail;
	
	// Decide how many bytes we're allowed to use encoding this frame.
	err = getByteBudget( glob, sourceFrame, &byteBudget );
	if( err )
		goto bail;
	
	// For non-droppable frames, cycle between the storage buffers.
	if( ! droppableFrame ) {
		storageIndex = glob->nextStorageIndex;
		glob->nextStorageIndex = ( glob->nextStorageIndex + 1 ) % kMaxStoredFrames;
	}
	
	dataPtr = ICMEncodedFrameGetDataPtr( encodedFrame );
	
	// Transfer the source frame into glob->currentFrame, converting it from chunky YUV422 to planar YUV420.
	sourcePixelBuffer = ICMCompressorSourceFrameGetPixelBuffer( sourceFrame );
	CVPixelBufferLockBaseAddress( sourcePixelBuffer, 0 );
	err = CopyChunkyYUV422ToPlanarYUV420( glob->width, glob->height,
			CVPixelBufferGetBaseAddress( sourcePixelBuffer ), 
			CVPixelBufferGetBytesPerRow( sourcePixelBuffer ),
			glob->currentFrame.planeArray[0].planeBaseAddr,
			glob->currentFrame.planeArray[0].planeRowBytes,
			glob->currentFrame.planeArray[1].planeBaseAddr,
			glob->currentFrame.planeArray[1].planeRowBytes,
			glob->currentFrame.planeArray[2].planeBaseAddr,
			glob->currentFrame.planeArray[2].planeRowBytes );
	CVPixelBufferUnlockBaseAddress( sourcePixelBuffer, 0 );
	if( err )
		goto bail;
	
	err = NaiveEncoder_EncodeFrame( 
			glob->width, glob->height, &glob->currentFrame, dataPtr, &dataSize, byteBudget,
			glob->storedFrameAvailable, glob->storedFrameArray, keyFrame, droppableFrame, storageIndex );
	if( err )
		goto bail;
	
	if( ! droppableFrame ) {
		// Non-droppable frames modify the state of the decoder, so we need to decode the frame
		// we just encoded so that we can accurately model the decoder's state.  
		decoderDataPtr = dataPtr;
		err = NaiveDecoder_DecodeFramePayload( glob->width, glob->height, &decoderDataPtr, NULL, dataSize, 
				glob->storedFrameArray, &glob->storedFrameArray[ storageIndex ] );
		if( err )
			goto bail;
		
		glob->storedFrameAvailable[ storageIndex ] = true;
	}
	
	// Update the encoded frame to reflect the actual frame size, sample flags and frame type.
	err = ICMEncodedFrameSetDataSize( encodedFrame, dataSize );
	if( err ) goto bail;
	
	mediaSampleFlags = 0;
	if( ! keyFrame ) {
		mediaSampleFlags |= mediaSampleNotSync;
		if( droppableFrame )
			mediaSampleFlags |= mediaSampleDroppable;
		if( partialSyncFrame )
			mediaSampleFlags |= mediaSamplePartialSync;
	}
	if( kICMFrameType_I == frameType )
		mediaSampleFlags |= mediaSampleDoesNotDependOnOthers;
	if( doesQueueContainEarlierDisplayNumbers( glob, 
				ICMCompressorSourceFrameGetDisplayNumber( sourceFrame ) ) )
		mediaSampleFlags |= mediaSampleEarlierDisplayTimesAllowed;
	
	err = ICMEncodedFrameSetMediaSampleFlags( encodedFrame, mediaSampleFlags );
	if( err )
		goto bail;
	
	err = ICMEncodedFrameSetFrameType( encodedFrame, frameType );
	if( err )
		goto bail;
	
	// Output the encoded frame.
	err = ICMCompressorSessionEmitEncodedFrame( glob->session, encodedFrame, 1, &sourceFrame );
	if( err )
		goto bail;
	
bail:
	// Since we created this, we must also release it.
	ICMEncodedFrameRelease( encodedFrame );
	
	return err;
}


