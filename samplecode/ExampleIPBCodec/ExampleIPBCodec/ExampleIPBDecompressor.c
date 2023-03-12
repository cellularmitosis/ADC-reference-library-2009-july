/*

File: ExampleIPBDecompressor.c, part of ExampleIPBCodec

Abstract: Example Image Decompressor component supporting IPB frame patterns under QuickTime 7.

Version: 1.0

� Copyright 2005 Apple Computer, Inc. All rights reserved.

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
#include "NaiveDecoder.h"

// Data structures
typedef struct	{
	ComponentInstance			self;
	ComponentInstance			delegateComponent;
	ComponentInstance			target;
	long						width;
	long						height;
	struct InternalPixelBuffer	storedFrameArray[kMaxStoredFrames];
	struct InternalPixelBuffer	immediateFrame;
	Handle						wantedDestinationPixelTypes;
} ExampleIPBDecompressorGlobalsRecord, *ExampleIPBDecompressorGlobals;

typedef struct {
	long		width;
	long		height;
	size_t		dataSize;
	int			storageIndex; // index in storedFrameArray of where this frame will go, if applicable
	Boolean		willBeStored; // if true, frame will go in storedFrameArray[storageIndex]; if false, immediateFrame.
	Boolean		decoded;
	char		pad[2];
} ExampleIPBDecompressRecord;

// Setup required for ComponentDispatchHelper.c
#define IMAGECODEC_BASENAME() 		ExampleIPB_D
#define IMAGECODEC_GLOBALS() 		ExampleIPBDecompressorGlobals storage

#define CALLCOMPONENT_BASENAME()	IMAGECODEC_BASENAME()
#define	CALLCOMPONENT_GLOBALS()		IMAGECODEC_GLOBALS()

#define QT_BASENAME()				CALLCOMPONENT_BASENAME()
#define QT_GLOBALS()				CALLCOMPONENT_GLOBALS()

#define COMPONENT_UPP_PREFIX()		uppImageCodec
#define COMPONENT_DISPATCH_FILE		"ExampleIPBDecompressorDispatch.h"
#define COMPONENT_SELECT_PREFIX()  	kImageCodec

#define	GET_DELEGATE_COMPONENT()	(storage->delegateComponent)

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

/* -- This Image Decompressor User the Base Image Decompressor Component --
	The base image decompressor is an Apple-supplied component
	that makes it easier for developers to create new decompressors.
	The base image decompressor does most of the housekeeping and
	interface functions required for a QuickTime decompressor component,
	including scheduling for asynchronous decompression.
*/

// Component Open Request - Required
pascal ComponentResult ExampleIPB_DOpen(ExampleIPBDecompressorGlobals glob, ComponentInstance self)
{
	ComponentResult err;

	// Allocate memory for our globals, set them up and inform the component manager that we've done so
	glob = calloc( sizeof( ExampleIPBDecompressorGlobalsRecord ), 1 );
	if( ! glob ) {
		err = memFullErr;
		goto bail;
	}

	SetComponentInstanceStorage(self, (Handle)glob);

	glob->self = self;
	glob->target = self;
	
	// Open and target an instance of the base decompressor as we delegate
	// most of our calls to the base decompressor instance
	err = OpenADefaultComponent(decompressorComponentType, kBaseCodecType, &glob->delegateComponent);
	if( err )
		goto bail;

	ComponentSetTarget(glob->delegateComponent, self);

bail:
	return err;
}

// Component Close Request - Required
pascal ComponentResult ExampleIPB_DClose(ExampleIPBDecompressorGlobals glob, ComponentInstance self)
{
	// Make sure to close the base component and dealocate our storage
	if (glob) {
		int frameIndex;
		
		if (glob->delegateComponent) {
			CloseComponent(glob->delegateComponent);
		}
		
		for( frameIndex = 0; frameIndex < kMaxStoredFrames; frameIndex++ ) {
			freeInternalPixelBuffer( &glob->storedFrameArray[frameIndex] );
		}
		freeInternalPixelBuffer( &glob->immediateFrame );

		DisposeHandle( glob->wantedDestinationPixelTypes );
		glob->wantedDestinationPixelTypes = NULL;
		
		free( glob );
	}

	return noErr;
}

// Component Version Request - Required
pascal ComponentResult ExampleIPB_DVersion(ExampleIPBDecompressorGlobals glob)
{
#pragma unused(glob)
	return kExampleIPBDecompressorVersion;
}

// Component Target Request
// 		Allows another component to "target" you i.e., you call another component whenever
// you would call yourself (as a result of your component being used by another component)
pascal ComponentResult ExampleIPB_DTarget(ExampleIPBDecompressorGlobals glob, ComponentInstance target)
{
	glob->target = target;
	return noErr;
}

#pragma mark-

// ImageCodecInitialize
//		The first function call that your image decompressor component receives from the base image
// decompressor is always a call to ImageCodecInitialize. In response to this call, your image decompressor
// component returns an ImageSubCodecDecompressCapabilities structure that specifies its capabilities.
pascal ComponentResult ExampleIPB_DInitialize(ExampleIPBDecompressorGlobals glob, ImageSubCodecDecompressCapabilities *cap)
{
#pragma unused(glob)

	// Secifies the size of the ImageSubCodecDecompressRecord structure
	// and say we can support asyncronous decompression
	// With the help of the base image decompressor, any image decompressor
	// that uses only interrupt-safe calls for decompression operations can
	// support asynchronous decompression.
	cap->decompressRecordSize = sizeof(ExampleIPBDecompressRecord);
	cap->canAsync = true;

	// These fields were added in QuickTime 7.  Be safe.
	if( cap->recordSize > offsetof(ImageSubCodecDecompressCapabilities, baseCodecShouldCallDecodeBandForAllFrames) ) {
		// Tell the base codec that we are "multi-buffer aware".
		// This promises that we always draw using the ImageSubCodecDecompressRecord.baseAddr/rowBytes 
		// passed to our ImageCodecDrawBand function, and that we always overwrite every pixel in the buffer.
		// It is important to set this in order to get optimal performance when playing through CoreVideo.
		cap->subCodecIsMultiBufferAware = true;
	
		// Tell the base codec that we support "out-of-order display times".
		// This is the same as saying that we support B frames, or frame reordering.
		// It is important to set this or the ICM will assume we do not support B frames,
		// and attempts to schedule frames in a display order that's different from their 
		// decode order will fail.
		cap->subCodecSupportsOutOfOrderDisplayTimes = true;
		
		// Ask the base codec to call our ImageCodecDecodeBand function for every frame.
		// If you do not set this, then your ImageCodecDrawBand function must
		// manually ensure that the frame is decoded before drawing it.
		cap->baseCodecShouldCallDecodeBandForAllFrames = true;
	}

	return noErr;
}

// ImageCodecPreflight
// 		The base image decompressor gets additional information about the capabilities of your image
// decompressor component by calling ImageCodecPreflight. The base image decompressor uses this
// information when responding to a call to the ImageCodecPredecompress function,
// which the ICM makes before decompressing an image. You are required only to provide values for
// the wantedDestinationPixelSize and wantedDestinationPixelTypes fields and can also modify other
// fields if necessary.
pascal ComponentResult ExampleIPB_DPreflight(ExampleIPBDecompressorGlobals glob, CodecDecompressParams *p)
{
	OSStatus err = noErr;
	CodecCapabilities *capabilities = p->capabilities;
	int widthRoundedUp, heightRoundedUp;
	int frameIndex;

	// Specify the minimum image band height supported by the component
	// bandInc specifies a common factor of supported image band heights - 
	// if your component supports only image bands that are an even
    // multiple of some number of pixels high report this common factor in bandInc
	capabilities->bandMin = (**p->imageDescription).height;
	capabilities->bandInc = capabilities->bandMin;

	// Indicate the pixel depth the component can use with the specified image
	capabilities->wantedPixelSize = 0; // set this to zero when using wantedDestinationPixelTypes
	if( NULL == glob->wantedDestinationPixelTypes ) {
		glob->wantedDestinationPixelTypes = NewHandleClear( 2 * sizeof(OSType) );
		if( NULL == glob->wantedDestinationPixelTypes )
			return memFullErr;
	}
	p->wantedDestinationPixelTypes = (OSType **)glob->wantedDestinationPixelTypes;
	(*p->wantedDestinationPixelTypes)[0] = k422YpCbCr8PixelFormat; // also known as '2vuy'
	(*p->wantedDestinationPixelTypes)[1] = 0;

	// Specify the number of pixels the image must be extended in width and height if
	// the component cannot accommodate the image at its given width and height.
	// This codec must have output buffers that are rounded up to multiples of 16x16.
	glob->width = (**p->imageDescription).width;
	glob->height = (**p->imageDescription).height;
	widthRoundedUp = glob->width;
	heightRoundedUp = glob->height;
	if( 0 != ( widthRoundedUp & 15 ) )
		widthRoundedUp = ( widthRoundedUp + 15 ) & ~15;
	if( 0 != ( heightRoundedUp & 15 ) )
		heightRoundedUp = ( heightRoundedUp + 15 ) & ~15;
	capabilities->extendWidth = widthRoundedUp - glob->width;
	capabilities->extendHeight = heightRoundedUp - glob->height;

	// Allocate our buffers.
	for( frameIndex = 0; frameIndex < kMaxStoredFrames; frameIndex++ ) {
		err = callocInternalPixelBuffer( widthRoundedUp, heightRoundedUp, &glob->storedFrameArray[frameIndex] );
		if( err )
			goto bail;
	}
	callocInternalPixelBuffer( widthRoundedUp, heightRoundedUp, &glob->immediateFrame );
	if( err )
		goto bail;

bail:
	return err;
}

// ImageCodecBeginBand
// 		The ImageCodecBeginBand function allows your image decompressor component to save information about
// a band before decompressing it. This function is never called at interrupt time. The base image decompressor
// preserves any changes your component makes to any of the fields in the ImageSubCodecDecompressRecord
// or CodecDecompressParams structures. If your component supports asynchronous scheduled decompression, it
// may receive more than one ImageCodecBeginBand call before receiving an ImageCodecDrawBand call.
pascal ComponentResult ExampleIPB_DBeginBand(ExampleIPBDecompressorGlobals glob, CodecDecompressParams *p, ImageSubCodecDecompressRecord *drp, long flags)
{
#pragma unused(glob)
	OSStatus err = noErr;
	ExampleIPBDecompressRecord *myDrp = (ExampleIPBDecompressRecord *)drp->userDecompressRecord;
	Boolean keyFrame, differenceFrame, droppableFrame;
	int storageIndex;

	myDrp->width = (**p->imageDescription).width;
	myDrp->height = (**p->imageDescription).height;
	
	// Unfortunately, the image decompressor API can not quite guarantee to tell the decompressor
	// how much data is available, because the deprecated API DecompressSequenceFrame does not take 
	// a dataSize argument.  (That's why you should call DecompressSequenceFrameS instead.)
	// Here's the best effort we can make: if there's a data-loading proc, use the dataSize from the 
	// image description; otherwise, use the bufferSize.
	if( drp->dataProcRecord.dataProc )
		myDrp->dataSize = (**p->imageDescription).dataSize;
	else
		myDrp->dataSize = p->bufferSize;
	
	// In some cases, a frame will be decoded and ready for display, but the display will be cancelled.  
	// QuickTime's video media handler will remember that the frame has already been decoded,
	// and if appropriate, will schedule that frame for display without redecoding by using the 
	// icmFrameAlreadyDecoded flag.  
	// In that case, we should simply retrieve the frame from whichever buffer we put it in.
	myDrp->decoded = p->frameTime ? (0 != (p->frameTime->flags & icmFrameAlreadyDecoded)) : false;
    
	err = NaiveDecoder_DecodeFrameHeader( (const UInt8 *)drp->codecData, &keyFrame, &differenceFrame, &droppableFrame, &storageIndex );
	if( err )
		goto bail;
	
	// Remember which internal buffer we're going to put this frame in when we decode it.
	myDrp->storageIndex = storageIndex;
	myDrp->willBeStored = ! droppableFrame;
	
    // Classify the frame so that the base codec can do the right thing.
	// It is very important to do this so that the base codec will know
	// which frames it can drop if we're in danger of falling behind.
	
	if( keyFrame ) {
		// Key frames are resynchronization points in the sequence of frames.
		// No frames following a key frame can depend on information from frames before a key frame.
		// Note that the I frame at the start of an open GOP in MPEG-1/2 frame patterns is *not* a key frame.
		drp->frameType = kCodecFrameTypeKey;
	}
	else if( droppableFrame ) {
		// Droppable frames are not stored; no later frames depend on information in them.
		// We decode them into the immediateFrame buffer, which is available for reuse
		// as soon as the frame is drawn.
		drp->frameType = kCodecFrameTypeDroppableDifference;
	}
	else {
    	// Other frames are difference frames.  
		drp->frameType = kCodecFrameTypeDifference;
    }
	
bail:
	return err;
}

pascal ComponentResult ExampleIPB_DDecodeBand(ExampleIPBDecompressorGlobals glob, ImageSubCodecDecompressRecord *drp, unsigned long flags)
{
	OSErr err = noErr;
	ExampleIPBDecompressRecord *myDrp = (ExampleIPBDecompressRecord *)drp->userDecompressRecord;
	ICMDataProcRecordPtr dataProc = drp->dataProcRecord.dataProc ? &drp->dataProcRecord : NULL;
	struct InternalPixelBuffer *destBuffer;
	
	if( myDrp->willBeStored ) {
		destBuffer = &glob->storedFrameArray[ myDrp->storageIndex ];
	}
	else {
		destBuffer = &glob->immediateFrame;
	}
	
	err = NaiveDecoder_DecodeFramePayload( myDrp->width, myDrp->height, (const UInt8 **)&drp->codecData, dataProc, myDrp->dataSize,
			glob->storedFrameArray, destBuffer );
	
	myDrp->decoded = true;
	
bail:
	return err;
}

// ImageCodecDrawBand
//		The base image decompressor calls your image decompressor component's ImageCodecDrawBand function
// to decompress a band or frame. Your component must implement this function. If the ImageSubCodecDecompressRecord
// structure specifies a progress function or data-loading function, the base image decompressor will never call ImageCodecDrawBand
// at interrupt time. If the ImageSubCodecDecompressRecord structure specifies a progress function, the base image decompressor
// handles codecProgressOpen and codecProgressClose calls, and your image decompressor component must not implement these functions.
// If not, the base image decompressor may call the ImageCodecDrawBand function at interrupt time.
// When the base image decompressor calls your ImageCodecDrawBand function, your component must perform the decompression specified
// by the fields of the ImageSubCodecDecompressRecord structure. The structure includes any changes your component made to it
// when performing the ImageCodecBeginBand function. If your component supports asynchronous scheduled decompression,
// it may receive more than one ImageCodecBeginBand call before receiving an ImageCodecDrawBand call.
pascal ComponentResult ExampleIPB_DDrawBand(ExampleIPBDecompressorGlobals glob, ImageSubCodecDecompressRecord *drp)
{
#pragma unused(glob)
	OSErr err = noErr;
	
	ExampleIPBDecompressRecord *myDrp = (ExampleIPBDecompressRecord *)drp->userDecompressRecord;
	struct InternalPixelBuffer *destBuffer;

	if( ! myDrp->decoded ) {
		// If you don't set the baseCodecShouldCallDecodeBandForAllFrames flag, or if you 
		// need QuickTime 6 compatibility, you should double-check that the frame has been decoded here,
		// and decode if necessary:
		
		err = ExampleIPB_DDecodeBand( glob, drp, 0 );
		if( err ) goto bail;
	}
	
	if( myDrp->willBeStored ) {
		destBuffer = &glob->storedFrameArray[ myDrp->storageIndex ];
	}
	else {
		destBuffer = &glob->immediateFrame;
	}
	
	err = CopyPlanarYUV420ToChunkyYUV422( 
			myDrp->width, myDrp->height, 
			destBuffer->planeArray[0].planeBaseAddr, destBuffer->planeArray[0].planeRowBytes,
			destBuffer->planeArray[1].planeBaseAddr, destBuffer->planeArray[1].planeRowBytes,
			destBuffer->planeArray[2].planeBaseAddr, destBuffer->planeArray[2].planeRowBytes,
			drp->baseAddr, drp->rowBytes );
	
bail:
	return err;
}

// ImageCodecEndBand
//		The ImageCodecEndBand function notifies your image decompressor component that decompression of a band has finished or
// that it was terminated by the Image Compression Manager. Your image decompressor component is not required to implement
// the ImageCodecEndBand function. The base image decompressor may call the ImageCodecEndBand function at interrupt time.
// After your image decompressor component handles an ImageCodecEndBand call, it can perform any tasks that are required
// when decompression is finished, such as disposing of data structures that are no longer needed. Because this function
// can be called at interrupt time, your component cannot use this function to dispose of data structures; this
// must occur after handling the function. The value of the result parameter should be set to noErr if the band or frame was
// drawn successfully. If it is any other value, the band or frame was not drawn.
pascal ComponentResult ExampleIPB_DEndBand(ExampleIPBDecompressorGlobals glob, ImageSubCodecDecompressRecord *drp, OSErr result, long flags)
{
#pragma unused(glob, drp,result, flags)
	
	return noErr;
}

// ImageCodecQueueStarting
// 		If your component supports asynchronous scheduled decompression, the base image decompressor calls your image decompressor component's
// ImageCodecQueueStarting function before decompressing the frames in the queue. Your component is not required to implement this function.
// It can implement the function if it needs to perform any tasks at this time, such as locking data structures.
// The base image decompressor never calls the ImageCodecQueueStarting function at interrupt time.
pascal ComponentResult ExampleIPB_DQueueStarting(ExampleIPBDecompressorGlobals glob)
{
#pragma unused(glob)
	
	return noErr;
}

// ImageCodecQueueStopping
//		 If your image decompressor component supports asynchronous scheduled decompression, the ImageCodecQueueStopping function notifies
// your component that the frames in the queue have been decompressed. Your component is not required to implement this function.
// After your image decompressor component handles an ImageCodecQueueStopping call, it can perform any tasks that are required when decompression
// of the frames is finished, such as disposing of data structures that are no longer needed. 
// The base image decompressor never calls the ImageCodecQueueStopping function at interrupt time.
pascal ComponentResult ExampleIPB_DQueueStopping(ExampleIPBDecompressorGlobals glob)
{
#pragma unused(glob)
	
	return noErr;
}

// ImageCodecGetCompressedImageSize
// 		Your component receives the ImageCodecGetCompressedImageSize request whenever an application calls the ICM's GetCompressedImageSize function.
// You can use the ImageCodecGetCompressedImageSize function when you are extracting a single image from a sequence; therefore, you don't have an
// image description structure and don't know the exact size of one frame. In this case, the Image Compression Manager calls the component to determine
// the size of the data. Your component should return a long integer indicating the number of bytes of data in the compressed image. You may want to store
// the image size somewhere in the image description structure, so that you can respond to this request quickly. Only decompressors receive this request.
pascal ComponentResult ExampleIPB_DGetCompressedImageSize(ExampleIPBDecompressorGlobals glob, ImageDescriptionHandle desc, Ptr data, long dataSize, ICMDataProcRecordPtr dataProc, long *size)
{
#pragma	unused(glob,desc,dataSize,dataProc)
	if (size == NULL) 
		return paramErr;
	
	//••
	
	return unimpErr;
}

// ImageCodecGetCodecInfo
//		Your component receives the ImageCodecGetCodecInfo request whenever an application calls the Image Compression Manager's GetCodecInfo function.
// Your component should return a formatted compressor information structure defining its capabilities.
// Both compressors and decompressors may receive this request.
pascal ComponentResult ExampleIPB_DGetCodecInfo(ExampleIPBDecompressorGlobals glob, CodecInfo *info)
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

#pragma mark-

// When building the *Application Version Only* make our component available for use by applications (or other clients).
// Once the Component Manager has registered a component, applications can find and open the component using standard
// Component Manager routines.
#if !STAND_ALONE && !TARGET_OS_WIN32
void RegisterExampleIPBDecompressor(void);
void RegisterExampleIPBDecompressor(void)
{
	ComponentDescription td;
	
	td.componentType = decompressorComponentType;
	td.componentSubType = FOUR_CHAR_CODE('EIPB');
	td.componentManufacturer = kAppleManufacturer;
	td.componentFlags = cmpThreadSafe;
	td.componentFlagsMask = 0;

	RegisterComponent(&td,(ComponentRoutineUPP)ExampleIPB_DComponentDispatch, 0, NULL, NULL, NULL);
}
#endif // !STAND_ALONE && TARGET_OS_WIN32
