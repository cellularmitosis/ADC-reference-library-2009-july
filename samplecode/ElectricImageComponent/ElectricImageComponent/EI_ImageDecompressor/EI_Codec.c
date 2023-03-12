/*
	File:		EI_Codec.c
	
	Description: Compressor component sample for Electric Image files

	Author:		QuickTime Engineering

	Copyright: 	© Copyright 2001 - 2003 Apple Computer, Inc. All rights reserved.
	
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
				
	Change History (most recent first):
	   <4>      05/05/03    dts         added frameType and wantedDestinationPixelType
	   <3>      06/07/01    dts         added ImageCodecGetMPWorkFunction QT5
	   <2>		01/19/01	dts			updated for X
	   <1>	 	11/28/99	QTE			first file
*/

#if TARGET_OS_WIN32
    #include "EIComponentWindowsPrefix.h"
#endif

#if __MACH__
    #include <Carbon/Carbon.h>
    #include <QuickTime/QuickTime.h>
#else
    #include <ConditionalMacros.h>
    #include <Endian.h>
    #include <ImageCodec.h>
#endif

#include "EI_CodecVersion.h"
#include "EI_Image.h"

// Constants
const UInt8 kNumPixelFormatsSupported = 1;

// Data structures
typedef struct	{
	ComponentInstance		self;
	ComponentInstance		delegateComponent;
	ComponentInstance		target;
	OSType**				wantedDestinationPixelTypeH;
	ImageCodecMPDrawBandUPP drawBandUPP;
} EI_GlobalsRecord, *EI_Globals;

typedef struct {
	long		width;
	long		height;
	long		depth;
} EI_DecompressRecord;

// Prototypes
static OSErr EI_Decompress1RLE(UInt8 *dataPtr, ICMDataProcRecordPtr dataProc, UInt8 *baseAddr, long rowBump, long width, long height);
static OSErr EI_Decompress8RLE(UInt8 *dataPtr, ICMDataProcRecordPtr dataProc, UInt8 *baseAddr, long rowBump, long width, long height);
static OSErr EI_Decompress16RLE(UInt8 *dataPtr, ICMDataProcRecordPtr dataProc, UInt8 *baseAddr, long rowBump, long width, long height);
static OSErr EI_Decompress24RLE(UInt8 *dataPtr, ICMDataProcRecordPtr dataProc, UInt8 *baseAddr, long rowBump, long width, long height);
static OSErr EI_Decompress32RLE(UInt8 *dataPtr, ICMDataProcRecordPtr dataProc, UInt8 *baseAddr, long rowBump, long width, long height);

// Setup required for ComponentDispatchHelper.c
#define IMAGECODEC_BASENAME() 		EI_ImageCodec
#define IMAGECODEC_GLOBALS() 		EI_Globals storage

#define CALLCOMPONENT_BASENAME()	IMAGECODEC_BASENAME()
#define	CALLCOMPONENT_GLOBALS()		IMAGECODEC_GLOBALS()

#define COMPONENT_UPP_PREFIX()		uppImageCodec
#define COMPONENT_DISPATCH_FILE		"EI_CodecDispatch.h"
#define COMPONENT_SELECT_PREFIX()  	kImageCodec

#define	GET_DELEGATE_COMPONENT()	(storage->delegateComponent)

#if __MACH__
	#include <CoreServices/Components.k.h>
	#include <QuickTime/ImageCodec.k.h>
	#include <QuickTime/ComponentDispatchHelper.c>
#else
	#include <Components.k.h>
	#include <ImageCodec.k.h>
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
pascal ComponentResult EI_ImageCodecOpen(EI_Globals glob, ComponentInstance self)
{
	ComponentResult err;

	// Allocate memory for our globals, set them up and inform the component manager that we've done so
	glob = (EI_Globals)NewPtrClear(sizeof(EI_GlobalsRecord));
	if (err = MemError()) goto bail;

	SetComponentInstanceStorage(self, (Handle)glob);

	glob->self = self;
	glob->target = self;
	glob->wantedDestinationPixelTypeH = (OSType **)NewHandle(sizeof(OSType) * (kNumPixelFormatsSupported + 1));
	if (err = MemError()) goto bail;
	glob->drawBandUPP = NULL;
	
	// Open and target an instance of the base decompressor as we delegate
	// most of our calls to the base decompressor instance
	err = OpenADefaultComponent(decompressorComponentType, kBaseCodecType, &glob->delegateComponent);
	if (err) goto bail;

	ComponentSetTarget(glob->delegateComponent, self);

bail:
	return err;
}

// Component Close Request - Required
pascal ComponentResult EI_ImageCodecClose(EI_Globals glob, ComponentInstance self)
{
	// Make sure to close the base component and dealocate our storage
	if (glob) {
		if (glob->delegateComponent) {
			CloseComponent(glob->delegateComponent);
		}
		if (glob->wantedDestinationPixelTypeH) {
			DisposeHandle((Handle)glob->wantedDestinationPixelTypeH);
		}
		if (glob->drawBandUPP) {
			DisposeImageCodecMPDrawBandUPP(glob->drawBandUPP);
		}

		DisposePtr((Ptr)glob);
	}

	return noErr;
}

// Component Version Request - Required
pascal ComponentResult EI_ImageCodecVersion(EI_Globals glob)
{
#pragma unused(glob)
	
    return kEI_CodecVersion;
}

// Component Target Request
// 		Allows another component to "target" you i.e., you call another component whenever
// you would call yourself (as a result of your component being used by another component)
pascal ComponentResult EI_ImageCodecTarget(EI_Globals glob, ComponentInstance target)
{
	glob->target = target;
	return noErr;
}

// Component GetMPWorkFunction Request
//		Allows your image decompressor component to perform asynchronous decompression
// in a single MP task by taking advantage of the Base Decompressor. If you implement
// this selector, your DrawBand function must be MP-safe. MP safety means not
// calling routines that may move or purge memory and not calling any routines which
// might cause 68K code to be executed. Ideally, your DrawBand function should not make
// any API calls whatsoever. Obviously don't implement this if you're building a 68k component.
#if !TARGET_CPU_68K
pascal ComponentResult EI_ImageCodecGetMPWorkFunction(EI_Globals glob, ComponentMPWorkFunctionUPP *workFunction, void **refCon)
{
	if (NULL == glob->drawBandUPP)
		#if !TARGET_API_MAC_CARBON
			glob->drawBandUPP = NewImageCodecMPDrawBandProc(EI_ImageCodecDrawBand);
		#else
			glob->drawBandUPP = NewImageCodecMPDrawBandUPP((ImageCodecMPDrawBandProcPtr)EI_ImageCodecDrawBand);
		#endif
		
	return ImageCodecGetBaseMPWorkFunction(glob->delegateComponent, workFunction, refCon, glob->drawBandUPP, glob);
}
#endif // !TARGET_CPU_68K

#pragma mark-

// ImageCodecInitialize
//		The first function call that your image decompressor component receives from the base image
// decompressor is always a call to ImageCodecInitialize . In response to this call, your image decompressor
// component returns an ImageSubCodecDecompressCapabilities structure that specifies its capabilities.
pascal ComponentResult EI_ImageCodecInitialize(EI_Globals glob, ImageSubCodecDecompressCapabilities *cap)
{
#pragma unused(glob)

	// Secifies the size of the ImageSubCodecDecompressRecord structure
	// and say we can support asyncronous decompression
	// With the help of the base image decompressor, any image decompressor
	// that uses only interrupt-safe calls for decompression operations can
	// support asynchronous decompression.
	cap->decompressRecordSize = sizeof(EI_DecompressRecord);
	cap->canAsync = true;

	return noErr;
}

// ImageCodecPreflight
// 		The base image decompressor gets additional information about the capabilities of your image
// decompressor component by calling ImageCodecPreflight. The base image decompressor uses this
// information when responding to a call to the ImageCodecPredecompress function,
// which the ICM makes before decompressing an image. You are required only to provide values for
// the wantedDestinationPixelSize and wantedDestinationPixelTypes fields and can also modify other
// fields if necessary.
pascal ComponentResult EI_ImageCodecPreflight(EI_Globals glob, CodecDecompressParams *p)
{
#pragma unused(glob)

	CodecCapabilities *capabilities = p->capabilities;
	OSTypePtr         formats = *glob->wantedDestinationPixelTypeH;
	UInt8             depth = (**p->imageDescription).depth;

	// Fill in formats for wantedDestinationPixelTypeH
	// Terminate with an OSType value 0  - see IceFloe #7
	// http://developer.apple.com/quicktime/icefloe/dispatch007.html
	
    if (depth == 24) depth = 32;
	
    *formats++	= depth;
	*formats++	= 0;
	
	// Specify the minimum image band height supported by the component
	// bandInc specifies a common factor of supported image band heights - 
	// if your component supports only image bands that are an even
    // multiple of some number of pixels high report this common factor in bandInc
	capabilities->bandMin = (**p->imageDescription).height;
	capabilities->bandInc = capabilities->bandMin;

	// Indicate the wanted destination using the wantedDestinationPixelTypeH previously set up
	capabilities->wantedPixelSize  = 0; 	
	p->wantedDestinationPixelTypes = glob->wantedDestinationPixelTypeH;

	// Specify the number of pixels the image must be extended in width and height if
	// the component cannot accommodate the image at its given width and height
	capabilities->extendWidth = 0;
	capabilities->extendHeight = 0;

	return noErr;
}

// ImageCodecBeginBand
// 		The ImageCodecBeginBand function allows your image decompressor component to save information about
// a band before decompressing it. This function is never called at interrupt time. The base image decompressor
// preserves any changes your component makes to any of the fields in the ImageSubCodecDecompressRecord
// or CodecDecompressParams structures. If your component supports asynchronous scheduled decompression, it
// may receive more than one ImageCodecBeginBand call before receiving an ImageCodecDrawBand call.
pascal ComponentResult EI_ImageCodecBeginBand(EI_Globals glob, CodecDecompressParams *p, ImageSubCodecDecompressRecord *drp, long flags)
{
#pragma unused(glob,flags)
	
	long offsetH, offsetV;
	EI_DecompressRecord *myDrp = (EI_DecompressRecord *)drp->userDecompressRecord;

	offsetH = (long)(p->dstRect.left - p->dstPixMap.bounds.left) * (long)(p->dstPixMap.pixelSize >> 3);
	offsetV = (long)(p->dstRect.top - p->dstPixMap.bounds.top) * (long)drp->rowBytes;

	drp->baseAddr = p->dstPixMap.baseAddr + offsetH + offsetV;
	
	// Let base codec know that all our frames are key frames (a.k.a., sync samples)
	// This allows the base codec to perform frame dropping on our behalf if needed 
    drp->frameType = kCodecFrameTypeKey;

	myDrp->width = (**p->imageDescription).width;
	myDrp->height = (**p->imageDescription).height;
	myDrp->depth = (**p->imageDescription).depth;

	return noErr;
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
pascal ComponentResult EI_ImageCodecDrawBand(EI_Globals glob, ImageSubCodecDecompressRecord *drp)
{
#pragma unused(glob)
	OSErr err = noErr;
	
	EI_DecompressRecord *myDrp = (EI_DecompressRecord *)drp->userDecompressRecord;
	unsigned char *dataPtr = (unsigned char *)drp->codecData;
	ICMDataProcRecordPtr dataProc = drp->dataProcRecord.dataProc ? &drp->dataProcRecord : NULL;
	ImageFramePtr framePtr = (ImageFramePtr)dataPtr;

	// skip the Image Frame Header and palette
	dataPtr += sizeof(ImageFrame);
	dataPtr += EndianU16_BtoN(framePtr->framePalettes) * 3;

	if (framePtr->framePacking == 0) {
		// uncompressed data
	} else if (framePtr->framePacking == 1) {
		// decompress appropriately
		switch (myDrp->depth) {
			case 1:
				err = EI_Decompress1RLE(dataPtr, dataProc, (UInt8 *)drp->baseAddr, drp->rowBytes - ((myDrp->width + 7) / 8), myDrp->width, myDrp->height);
				break;

			case 8:
				err = EI_Decompress8RLE(dataPtr, dataProc, (UInt8 *)drp->baseAddr, drp->rowBytes - (myDrp->width * 1), myDrp->width, myDrp->height);
				break;

			case 16:
				err = EI_Decompress16RLE(dataPtr, dataProc, (UInt8 *)drp->baseAddr, drp->rowBytes - (myDrp->width * 2), myDrp->width, myDrp->height);
				break;

			case 24:
				err = EI_Decompress24RLE(dataPtr, dataProc, (UInt8 *)drp->baseAddr, drp->rowBytes - (myDrp->width * 4), myDrp->width, myDrp->height);
				break;

			case 32:
				err = EI_Decompress32RLE(dataPtr, dataProc, (UInt8 *)drp->baseAddr, drp->rowBytes - (myDrp->width * 4), myDrp->width, myDrp->height);
				break;
		}
	}

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
pascal ComponentResult EI_ImageCodecEndBand(EI_Globals glob, ImageSubCodecDecompressRecord *drp, OSErr result, long flags)
{
#pragma unused(glob, drp,result, flags)
	
	return noErr;
}

// ImageCodecQueueStarting
// 		If your component supports asynchronous scheduled decompression, the base image decompressor calls your image decompressor component's
// ImageCodecQueueStarting function before decompressing the frames in the queue. Your component is not required to implement this function.
// It can implement the function if it needs to perform any tasks at this time, such as locking data structures.
// The base image decompressor never calls the ImageCodecQueueStarting function at interrupt time.
pascal ComponentResult EI_ImageCodecQueueStarting(EI_Globals glob)
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
pascal ComponentResult EI_ImageCodecQueueStopping(EI_Globals glob)
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
pascal ComponentResult EI_ImageCodecGetCompressedImageSize(EI_Globals glob, ImageDescriptionHandle desc, Ptr data, long dataSize, ICMDataProcRecordPtr dataProc, long *size)
{
#pragma	unused(glob,dataSize,dataProc,desc)
	ImageFramePtr framePtr = (ImageFramePtr)data;

	if (size == NULL) 
		return paramErr;

	*size = EndianU32_BtoN(framePtr->frameSize) + sizeof(ImageFrame);

	return noErr;
}

// ImageCodecGetCodecInfo
//		Your component receives the ImageCodecGetCodecInfo request whenever an application calls the Image Compression Manager's GetCodecInfo function.
// Your component should return a formatted compressor information structure defining its capabilities.
// Both compressors and decompressors may receive this request.
pascal ComponentResult EI_ImageCodecGetCodecInfo(EI_Globals glob, CodecInfo *info)
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

//****** RLE Decompression Routines ******

// Someday you may find yourself on an architecture where misaligned reads
// are ultra-expensive. On that day you can cheerfully adjust these macros to compensate.

#define Get32(x)		(*(long*)(x))
#define GetU32(x)		(*(unsigned long*)(x))
#define Set32(x,y)		(*(long*)(x)) = ((long)(y))

#define Get16(x)		(*(short*)(x))
#define GetU16(x)		(*(unsigned short*)(x))
#define Set16(x,y)		(*(short*)(x)) = ((short)(y))


#define kSpoolChunkSize (16384)
#define kInfiniteDataSize (0x7fffffff)

OSErr EI_Decompress32RLE(UInt8 *dataPtr, ICMDataProcRecordPtr dataProc, UInt8 *baseAddr, long rowBump, long width, long height)
{
	OSErr err = noErr;
	UInt8 *endOfScanLine;
	long availableData = dataProc ? codecMinimumDataSize : kInfiniteDataSize;

	endOfScanLine = baseAddr + (width * 4);

	while (height) {
		UInt16 opcode;

		if (availableData < kSpoolChunkSize) {	
			// get some more source data
			err = InvokeICMDataUPP((Ptr *)&dataPtr, codecMinimumDataSize, dataProc->dataRefCon, dataProc->dataProc);
			if (err == eofErr) err = noErr;
			if (err) goto bail;

			availableData = codecMinimumDataSize;
		}

		opcode = *dataPtr++;
		if (opcode <= 127) {
			// repeat
			UInt32 pixel = GetU32(dataPtr);
			dataPtr += sizeof(UInt32);
			opcode++;
			availableData -= 5;
			while (opcode--) {
				Set32(baseAddr, pixel);
				baseAddr += sizeof(long);
				if (baseAddr == endOfScanLine) {
					baseAddr += rowBump;
					endOfScanLine = baseAddr + (width * 4);
					height--;
				}
			}
		}
		else {
			// quote
			opcode -= 127;
			availableData -= (1 + (opcode * 4));
			while (opcode--) {
				Set32(baseAddr, GetU32(dataPtr));
				baseAddr += sizeof(long);
				dataPtr += sizeof(long);
				if (baseAddr == endOfScanLine) {
					baseAddr += rowBump;
					endOfScanLine = baseAddr + (width * 4);
					height--;
				}
			}
		}
	}

bail:
	return err;
}

OSErr EI_Decompress24RLE(UInt8 *dataPtr, ICMDataProcRecordPtr dataProc, UInt8 *baseAddr, long rowBump, long width, long height)
{
	OSErr err = noErr;
	UInt8 *endOfScanLine;
	long availableData = dataProc ? codecMinimumDataSize : kInfiniteDataSize;

	endOfScanLine = baseAddr + (width * 4);

	while (height) {
		UInt16 opcode;

		if (availableData < kSpoolChunkSize) {
			// get some more source data
			err = InvokeICMDataUPP((Ptr *)&dataPtr, codecMinimumDataSize, dataProc->dataRefCon, dataProc->dataProc);
			if (err == eofErr) err = noErr;
			if (err) goto bail;

			availableData = codecMinimumDataSize;
		}

		opcode = *dataPtr++;
		if (opcode <= 127) {
			// repeat
			#if TARGET_RT_BIG_ENDIAN
				UInt32 pixel = ((GetU32(dataPtr) >> 8) | 0xFF000000);	// make sure we're not transparent on X
			#else
				UInt32 pixel = ((GetU32(dataPtr) << 8) | 0x000000FF);
			#endif
			dataPtr += 3;
			opcode++;
			availableData -= 4;
			while (opcode--) {
				Set32(baseAddr, pixel);
				baseAddr += sizeof(long);
				if (baseAddr == endOfScanLine) {
					baseAddr += rowBump;
					endOfScanLine = baseAddr + (width * 4);
					height--;
				}
			}
		}
		else {
			// quote
			opcode -= 127;
			availableData -= (1 + (opcode * 3));
			while (opcode--) {
				#if TARGET_RT_BIG_ENDIAN
					UInt32 pixel = ((GetU32(dataPtr) >> 8) | 0xFF000000);	// make sure we're not transparent on X
				#else
					UInt32 pixel = ((GetU32(dataPtr) << 8) | 0x000000FF);
				#endif
				Set32(baseAddr, pixel);
				baseAddr += sizeof(UInt32);
				dataPtr += 3;
				if (baseAddr == endOfScanLine) {
					baseAddr += rowBump;
					endOfScanLine = baseAddr + (width * 4);
					height--;
				}
			}
		}
	}

bail:
	return err;
}

OSErr EI_Decompress16RLE(UInt8 *dataPtr, ICMDataProcRecordPtr dataProc, UInt8 *baseAddr, long rowBump, long width, long height)
{
	OSErr err = noErr;
	UInt8 *endOfScanLine;
	long availableData = dataProc ? codecMinimumDataSize : kInfiniteDataSize;

	endOfScanLine = baseAddr + (width * 2);

	while (height) {
		UInt16 opcode;

		if (availableData < kSpoolChunkSize) {
			// get some more source data
			err = InvokeICMDataUPP((Ptr *)&dataPtr, codecMinimumDataSize, dataProc->dataRefCon, dataProc->dataProc);
			if (err == eofErr) err = noErr;
			if (err) goto bail;

			availableData = codecMinimumDataSize;
		}

		opcode = *dataPtr++;
		if (opcode <= 127) {
			// repeat
			UInt16 pixel = GetU16(dataPtr);
			dataPtr += sizeof(UInt16);
			opcode++;
			availableData -= 3;
			while (opcode--) {
				Set16(baseAddr, pixel);
				baseAddr += sizeof(UInt16);
				if (baseAddr == endOfScanLine) {
					baseAddr += rowBump;
					endOfScanLine = baseAddr + (width * 2);
					height--;
				}
			}
		}
		else {
			// quote
			opcode -= 127;
			availableData -= (1 + (opcode * 2));
			while (opcode--) {
				Set16(baseAddr, GetU16(dataPtr));
				baseAddr += sizeof(UInt16);
				dataPtr += sizeof(UInt16);
				if (baseAddr == endOfScanLine) {
					baseAddr += rowBump;
					endOfScanLine = baseAddr + (width * 2);
					height--;
				}
			}
		}
	}

bail:
	return err;
}

OSErr EI_Decompress8RLE(UInt8 *dataPtr, ICMDataProcRecordPtr dataProc, UInt8 *baseAddr, long rowBump, long width, long height)
{
	OSErr err = noErr;
	UInt8 *endOfScanLine;
	long availableData = dataProc ? codecMinimumDataSize : kInfiniteDataSize;

	endOfScanLine = baseAddr + (width * 1);

	while (height) {
		UInt16 opcode;

		if (availableData < kSpoolChunkSize) {
			// get some more source data
			err = InvokeICMDataUPP((Ptr *)&dataPtr, codecMinimumDataSize, dataProc->dataRefCon, dataProc->dataProc);
			if (err == eofErr) err = noErr;
			if (err) goto bail;

			availableData = codecMinimumDataSize;
		}

		opcode = *dataPtr++;
		if (opcode <= 127) {
			// repeat
			UInt8 pixel = *(UInt8 *)dataPtr;
			dataPtr += sizeof(UInt8);
			opcode++;
			availableData -= 2;
			while (opcode--) {
				*(UInt8 *)baseAddr = pixel;
				baseAddr += sizeof(UInt8);
				if (baseAddr == endOfScanLine) {
					baseAddr += rowBump;
					endOfScanLine = baseAddr + (width * 1);
					height--;
				}
			}
		}
		else {
			// quote
			opcode -= 127;
			availableData -= (1 + (opcode * 1));
			while (opcode--) {
				*(UInt8 *)baseAddr = *(UInt8 *)dataPtr;
				baseAddr += sizeof(UInt8);
				dataPtr += sizeof(UInt8);
				if (baseAddr == endOfScanLine) {
					baseAddr += rowBump;
					endOfScanLine = baseAddr + (width * 1);
					height--;
				}
			}
		}
	}

bail:
	return err;
}

OSErr EI_Decompress1RLE(UInt8 *dataPtr, ICMDataProcRecordPtr dataProc, UInt8 *baseAddr, long rowBump, long width, long height)
{
	OSErr err = noErr;
	UInt8 *endOfScanLine;
	long availableData = dataProc ? codecMinimumDataSize : kInfiniteDataSize;

	endOfScanLine = baseAddr + ((width + 7) / 8);

	while (height) {
		UInt16 opcode;

		if (availableData < kSpoolChunkSize) {
			// get some more source data
			err = InvokeICMDataUPP((Ptr *)&dataPtr, codecMinimumDataSize, dataProc->dataRefCon, dataProc->dataProc);
			if (err == eofErr) err = noErr;
			if (err) goto bail;

			availableData = codecMinimumDataSize;
		}

		opcode = *dataPtr++;
		if (opcode <= 127) {
			// repeat
			UInt8 pixel = *(UInt8 *)dataPtr;
			dataPtr += sizeof(UInt8);
			opcode++;
			availableData -= 2;
			while (opcode--) {
				*(UInt8 *)baseAddr = pixel;
				baseAddr += sizeof(UInt8);
				if (baseAddr == endOfScanLine) {
					baseAddr += rowBump;
					endOfScanLine = baseAddr + ((width + 7) / 8);
					height--;
				}
			}
		}
		else {
			// quote
			opcode -= 127;
			availableData -= (1 + (opcode * 1));
			while (opcode--) {
				*(UInt8 *)baseAddr = *(UInt8 *)dataPtr;
				baseAddr += sizeof(UInt8);
				dataPtr += sizeof(UInt8);
				if (baseAddr == endOfScanLine) {
					baseAddr += rowBump;
					endOfScanLine = baseAddr + ((width + 7) / 8);
					height--;
				}
			}
		}
	}

bail:
	return err;
}

#pragma mark-

// When building the *Application Version Only* make our component available for use by applications (or other clients).
// Once the Component Manager has registered a component, applications can find and open the component using standard
// Component Manager routines.
#if !STAND_ALONE && !TARGET_OS_WIN32
void EI_CodecRegister(void);
void EI_CodecRegister(void)
{
	ComponentDescription td;
	#if !TARGET_API_MAC_CARBON
		ComponentRoutineUPP componentEntryPoint = NewComponentRoutineProc(EI_ImageCodecComponentDispatch);
	#else
		ComponentRoutineUPP componentEntryPoint = NewComponentRoutineUPP((ComponentRoutineProcPtr)EI_ImageCodecComponentDispatch);
	#endif

	td.componentType = decompressorComponentType;
	td.componentSubType = FOUR_CHAR_CODE('EIDI');
	td.componentManufacturer = kAppleManufacturer;
	td.componentFlags = codecInfoDoes32;
	td.componentFlagsMask = 0;

	RegisterComponent(&td,componentEntryPoint, 0, NULL, NULL, NULL);
}
#endif // !STAND_ALONE && TARGET_OS_WIN32