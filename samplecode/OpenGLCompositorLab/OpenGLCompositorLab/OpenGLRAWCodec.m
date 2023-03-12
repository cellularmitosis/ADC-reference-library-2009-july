/*
	File:		OpenGLRAWCodec.c
	
	Description: This is an example of a Transfer Codec - it is dedicated to be used with SoftVOut
				 a fake software implementation of a video output 'vout' component.

	Author:		QuickTime Engineering

	Copyright: 	© Copyright 1998 - 2001 Apple Computer, Inc. All rights reserved.
	
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
				
	Change History (most recent first): <1> 12/20/01 Initial Release
*/

/*

If your video output device cannot display a pixel format defined by QuickTime, you should include
a special decompressor, known as a transfer codec, that converts one of the supported QuickTime pixel
formats (preferably 32-bit RGB) to data that the device can display. When this transfer codec is
available, the QuickTime Image Compression Manager automatically uses it together with its built-in
decompressors. This, in turn, lets applications or other software draw any QuickTime video directly
to the video output component's graphics world.

A transfer codec is a specialized image decompressor component, and should be based on the Base Image
Decompressor.

*/

#if __APPLE_CC__ || __MACH__
    #include <Carbon/Carbon.h>
    #include <QuickTime/QuickTime.h>
#import <Foundation/Foundation.h>
#else
    #include <ConditionalMacros.h>
    #include <Endian.h>
    #include <ImageCodec.h>
    #include <Resources.h>
#endif

#import "Layer.h"

// Our Private Pixel Format
// To register a new fourCC please send mail to qtfourCC@apple.com.
// Include an email address for future correspondence, the fourCC you
// would like to register, and a brief description of the fourCC format.
// For more information refer to IceFloe #20 - QuickTime Pixel Format FourCCs
// http://developer.apple.com/quicktime/icefloe/dispatch020.html
enum {
	kOpenGLRAWPixelFormat = FOUR_CHAR_CODE('OGLR')
};

// Constants
static const UInt8 kNumPixelFormatsSupported = 1;
static const UInt8 kCodecInfoResNum = 129;

// Because this is a custom codec which implements a custom pixel format, you have to provide
// all codec functionality, for example Masking(Clipping) and Scaling, yourself. QuickTime
// isn't going to do any of this work for you.
// NOTE: This example does not actually scale, but we say we do because it's expected of us.
static const long kCodecCapabilitiesFlags = codecCanMask | codecCanScale | codecCanTransform | codecCanRemapColor | codecCanRemapResolution;

// Per frame globals
typedef struct {
	long	width;
	long	height;
	long	depth;
	long	maxBytesPerRow;
	long	maxRows;
} OpenGLRAWCodecDecompressRecord;

// Per instance globals
typedef struct {
	ComponentInstance		self;
	ComponentInstance		target;
	ComponentInstance		baseCodec;
	OSType					codecType;
	OSType**				wantedDestinationPixelTypeH;
	ImageCodecMPDrawBandUPP drawBandUPP;
	unsigned char			*imageBuffer;
	unsigned long			imageBufferRowBytes;
	id				owner;
	
} OpenGLRAWCodecGlobalsRecord, *OpenGLRAWCodecGlobals;

/************************************************************************************/
// Setup required for ComponentDispatchHelper.c

pascal ComponentResult
OpenGLRAWCodec_LockBits(OpenGLRAWCodecGlobals storage, CGrafPtr port);
pascal ComponentResult
OpenGLRAWCodec_UnlockBits(OpenGLRAWCodecGlobals storage, CGrafPtr port);

enum {
    kImageCodecLockBitsSelect		= 0x0026,
    kImageCodecUnlockBitsSelect 	= 0x0027
};

enum {
    uppImageCodecLockBitsProcInfo	= 0x000003F0,
    uppImageCodecUnlockBitsProcInfo	= 0x000003F0
};

#define IMAGECODEC_BASENAME()		OpenGLRAWCodec_
#define IMAGECODEC_GLOBALS()		OpenGLRAWCodecGlobals storage

#define CALLCOMPONENT_BASENAME()	IMAGECODEC_BASENAME()
#define CALLCOMPONENT_GLOBALS()		IMAGECODEC_GLOBALS()

#define COMPONENT_DISPATCH_FILE		"OpenGLRAWCodecDispatch.h"
#define	GET_DELEGATE_COMPONENT()	(storage->baseCodec)
#define COMPONENT_UPP_SELECT_ROOT()	ImageCodec

#include "Components.k.h"				// Standard .k.h
#include "ImageCodec.k.h"
#include "ComponentDispatchHelper.c"	// Make the dispatcher and canDo

#pragma mark-

Component OpenGLRawComponent;

void OpenGLRAWCodec_DoRegister()
{
	ComponentRoutineUPP glComponentUPP;
	ComponentDescription cd;
	
	cd.componentType = decompressorComponentType;
	cd.componentSubType = 'raw ';
	cd.componentManufacturer = 'WWDC';
	cd.componentFlags =     codecInfoDoes32 |
				codecInfoDoes16	 |
				codecInfoDoesStretch |
				codecInfoDoesDouble  |
				codecInfoDoesMask	 |
				codecInfoDoesQuad;		// component flags
	cd.componentFlagsMask = 0;
	
	glComponentUPP = NewComponentRoutineUPP((ComponentRoutineProcPtr)OpenGLRAWCodec_ComponentDispatch);
	
	OpenGLRawComponent = RegisterComponent(&cd, glComponentUPP, 0, NULL, NULL, NULL);	
}

/* -- This Image Decompressor User the Base Image Decompressor Component --
	The base image decompressor is an Apple-supplied component
	that makes it easier for developers to create new decompressors.
	The base image decompressor does most of the housekeeping and
	interface functions required for a QuickTime decompressor component,
	including scheduling for asynchronous decompression.
*/

/************************************************************************************/
// Component Manager Calls

// Component Open Request - Required
pascal ComponentResult OpenGLRAWCodec_Open(OpenGLRAWCodecGlobals glob, ComponentInstance self)
{
    ComponentDescription cd;
	
	ComponentResult		 err;

	// Allocate memory for our globals, set them up and inform the component
	// manager that we've done so
	glob = (OpenGLRAWCodecGlobals)NewPtrClear(sizeof(OpenGLRAWCodecGlobalsRecord));
	if ((err = MemError())) goto bail;
	
	SetComponentInstanceStorage(self, (Handle)glob);
	glob->self	 = self;
	glob->target = self;
	glob->drawBandUPP = NULL;
	
	// Open and target an instance of the base decompressor
	err	= OpenADefaultComponent(decompressorComponentType, kBaseCodecType, &glob->baseCodec);
	if (err) goto bail;

	// Set us as the base component's target
	CallComponentTarget(glob->baseCodec, self);
	
	// Record our codecType for posterity
	err = GetComponentInfo((Component)self, &cd, NULL, NULL, NULL);
	if (err) goto bail;
	
	glob->codecType	= cd.componentSubType;

	// Allocate memory for our wantedDestinationPixelType list, we fill it in during the Preflight call.
	glob->wantedDestinationPixelTypeH = (OSType **)NewHandle(sizeof(OSType) * (kNumPixelFormatsSupported + 1));

bail:
	return err;
}

// Component Close Request - Required
pascal ComponentResult OpenGLRAWCodec_Close(OpenGLRAWCodecGlobals glob, ComponentInstance self)
{
	if (glob) {
		if (glob->baseCodec)
			CloseComponent(glob->baseCodec);
		if (glob->wantedDestinationPixelTypeH)
			DisposeHandle((Handle)glob->wantedDestinationPixelTypeH);	
		if (glob->drawBandUPP) {
			DisposeImageCodecMPDrawBandUPP(glob->drawBandUPP);
		}
		DisposePtr((Ptr)glob);
	}

	return noErr;
}

// Component Version Request - Required
pascal ComponentResult OpenGLRAWCodec_Version(OpenGLRAWCodecGlobals glob)
{
#pragma unused(glob)

	return (codecInterfaceVersion << 2) + 1;
}

// Component Register Request
pascal ComponentResult OpenGLRAWCodec_Register(OpenGLRAWCodecGlobals glob)
{
#pragma unused(glob)

	// Always register
	return noErr;
}

// Component Target Request
// 		Allows another component to "target" you i.e., you call another component whenever
// you would call yourself (as a result of your component being used by another component).
pascal ComponentResult OpenGLRAWCodec_Target(OpenGLRAWCodecGlobals glob, ComponentInstance target)
{
	ComponentResult	err;

	// Tell the base component to target the instance
	err	= CallComponentTarget(glob->baseCodec, target);
	if (err) goto bail;

	// Remember our target
	glob->target = target;

bail:
	return err;
}

#pragma mark-
/************************************************************************************/
// Base Component Calls

// ImageCodecInitialize
//		The first function call that your image decompressor component receives from the base image
// decompressor is always a call to ImageCodecInitialize . In response to this call, your image decompressor
// component returns an ImageSubCodecDecompressCapabilities structure that specifies its capabilities.
pascal ComponentResult OpenGLRAWCodec_Initialize(OpenGLRAWCodecGlobals glob, ImageSubCodecDecompressCapabilities *cap)
{
#pragma unused(glob)

	cap->decompressRecordSize = sizeof(OpenGLRAWCodecDecompressRecord);
	cap->canAsync			  = false;
	
	return noErr;
}

// ImageCodecPreflight
// 		The base image decompressor gets additional information about the capabilities of your image
// decompressor component by calling ImageCodecPreflight. The base image decompressor uses this
// information when responding to a call to the ImageCodecPredecompress function,
// which the ICM makes before decompressing an image. You are required only to provide values for
// the wantedDestinationPixelSize and wantedDestinationPixelTypes fields and can also modify other
// fields if necessary.
pascal ComponentResult OpenGLRAWCodec_Preflight(OpenGLRAWCodecGlobals glob, CodecDecompressParams *p)
{
	CodecCapabilities *capabilities = p->capabilities;
	OSTypePtr		  formats = *glob->wantedDestinationPixelTypeH;
	
	// Fill in formats for wantedDestinationPixelTypeH
	// Terminate with an OSType value 0  - see IceFloe #7
	// http://developer.apple.com/quicktime/icefloe/dispatch007.html
	*formats++	= kOpenGLRAWPixelFormat;
	*formats++	= 0;

	// The base codec adds some flags, so OR in our own flags as well
	capabilities->flags = capabilities->flags | kCodecCapabilitiesFlags;
	
	capabilities->wantedPixelSize  = 0; 	
	p->wantedDestinationPixelTypes = glob->wantedDestinationPixelTypeH;
	
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
pascal ComponentResult OpenGLRAWCodec_BeginBand(OpenGLRAWCodecGlobals glob, CodecDecompressParams *p, ImageSubCodecDecompressRecord *drp, long flags)
{
#pragma unused(glob, flags)

	OpenGLRAWCodecDecompressRecord *myDrp = (OpenGLRAWCodecDecompressRecord *)drp->userDecompressRecord;
	//long					  condition = (unsigned short)p->conditionFlags;
	ICMProgressProcRecord 	  progressProcRecord;
	ICMDataProcRecord		  dataProcRecord;

	// Save the height and width in our private decompress
	// record so we'll have it available in DrawBand.
	myDrp->height = p->srcRect.bottom - p->srcRect.top;
	myDrp->width = p->srcRect.right - p->srcRect.left;
	
	// We also need these max values for DrawBand.
	myDrp->maxBytesPerRow = (p->dstPixMap.bounds.right - p->dstPixMap.bounds.left) * 4;
	myDrp->maxBytesPerRow -= p->dstRect.left * 4;
	
	myDrp->maxRows = (p->dstPixMap.bounds.bottom - p->dstPixMap.bounds.top);
	myDrp->maxRows -= p->dstRect.top;

	myDrp->depth = (*p->imageDescription)->depth;
	
	progressProcRecord = p->progressProcRecord;
	dataProcRecord	   = p->dataProcRecord;
	
	return noErr;
}

// ImageCodecDrawBand
//		The base image decompressor calls your image decompressor component's ImageCodecDrawBand function
// to decompress a band or frame. Your component must implement this function. If the ImageSubCodecDecompressRecord
// structure specifies a progress function or data-loading function, the base image decompressor will never call ImageCodecDrawBand
// at interrupt time. If the ImageSubCodecDecompressRecord structure specifies a progress function, the base image decompressor
// handles codecProgressOpen and codecProgressClose calls, and your image decompressor component must not implement these functions.
// You can however optionally implement the codecProgressUpdatePercent function to provide progress information during lengthy
// decompression operations.
// If the ImageSubCodecDecompressRecord structure does not specify a progress function the base image decompressor may call the
// ImageCodecDrawBand function at interrupt time.
// When the base image decompressor calls your ImageCodecDrawBand function, your component must perform the decompression specified
// by the fields of the ImageSubCodecDecompressRecord structure. The structure includes any changes your component made to it
// when performing the ImageCodecBeginBand function. If your component supports asynchronous scheduled decompression,
// it may receive more than one ImageCodecBeginBand call before receiving an ImageCodecDrawBand call.
pascal ComponentResult OpenGLRAWCodec_DrawBand(OpenGLRAWCodecGlobals glob, ImageSubCodecDecompressRecord *drp)
{
#pragma unused(glob)
	OpenGLRAWCodecDecompressRecord *myDrp = (OpenGLRAWCodecDecompressRecord *)drp->userDecompressRecord;
	
	Ptr	 src		 = drp->codecData;
	Ptr	 dest		 = drp->baseAddr;
	long dstRowBytes = drp->rowBytes;
	long srcRowBytes = (myDrp->width * myDrp->depth) >> 3;
	long bytesPerRow = myDrp->width * 4;
	long height 	 = myDrp->height;
	long row;
	
	// Make sure we limit our blit to 320x240 if the source is larger.
	if (bytesPerRow > myDrp->maxBytesPerRow)
		bytesPerRow = myDrp->maxBytesPerRow;
	
	if (height > myDrp->maxRows)
		height = myDrp->maxRows;	

	switch(myDrp->depth)
	{
		case 32:
			// Should be reasonably fast.
			for(row = 0; row < height; row++)
			{
				memcpy(dest,src,srcRowBytes);
				dest += dstRowBytes;
				src += srcRowBytes;
			}
			break;
			
		case 24:
			// This is probably really slow.
			for(row = 0; row < height; row++)
			{
				unsigned char r, g, b;
				unsigned char *s;
				unsigned long *d;
				unsigned long x;
				s = src + row*srcRowBytes;
				d = (unsigned long *)(dest + row*dstRowBytes);

				// We sould unroll this to do 4 source pixels at once (12 bytes),
				// which would let us do 32-bit reads and writes.
				for(x = 0; x < myDrp->width; x++)
				{
					r = s[0];
					g = s[1];
					b = s[2];
					s += 3;
					*d = (255 << 24) | (r << 16) | (g << 8) | b;
					d++;
				}
				
			}
			break;
			
		default:
		case 16:
			// Not handled yet.
			break;
		
	}
	return noErr;
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
pascal ComponentResult OpenGLRAWCodec_EndBand(OpenGLRAWCodecGlobals glob, ImageSubCodecDecompressRecord *drp, OSErr result, long flags)
{
#pragma unused(glob, drp, result, flags)

	return noErr;
}

// ImageCodecQueueStarting
// 		If your component supports asynchronous scheduled decompression, the base image decompressor calls your image decompressor component's
// ImageCodecQueueStarting function before decompressing the frames in the queue. Your component is not required to implement this function.
// It can implement the function if it needs to perform any tasks at this time, such as locking data structures.
// The base image decompressor never calls the ImageCodecQueueStarting function at interrupt time.
pascal ComponentResult OpenGLRAWCodec_QueueStarting(OpenGLRAWCodecGlobals glob)
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
pascal ComponentResult OpenGLRAWCodec_QueueStopping(OpenGLRAWCodecGlobals glob)
{
#pragma unused(glob)

	return noErr;
}

#pragma mark-
/************************************************************************************/
// Codec Component Calls

// ImageCodecGetCodecInfo
//		Your component receives the ImageCodecGetCodecInfo request whenever an application calls the Image Compression Manager's GetCodecInfo
// function. Your component should return a formatted compressor information structure defining its capabilities.
// Both compressors and decompressors may receive this request.
pascal ComponentResult OpenGLRAWCodec_GetCodecInfo(OpenGLRAWCodecGlobals glob, CodecInfo *info)
{
	if (!info) return paramErr;

	c2pstrcpy((unsigned char *)info->typeName,"OpenGL Output Codec");
	info->version = 0x0001;
	info->revisionLevel = 0x0001;
	info->vendor = 'WWDC';
	info->decompressFlags = codecInfoDoes32 |
				codecInfoDoes16	 |
				codecInfoDoesStretch |
				codecInfoDoesDouble  |
				codecInfoDoesMask	 |
				codecInfoDoesQuad;		// component flags
	info->compressFlags = 0,
	info->formatFlags = codecInfoDepth16|codecInfoDepth32 | codecInfoDoesLossless;
	info->compressionAccuracy = 100;
	info->decompressionAccuracy = 100;
	info->compressionSpeed = 1;
	info->decompressionSpeed = 1;
	info->compressionLevel = 0;
	info->resvd = 0;
	info->minimumHeight = 2;
	info->minimumWidth = 2;
	info->decompressPipelineLatency = 0;
	info->compressPipelineLatency = 0;
	info->privateData = 0;
	
	return noErr;
}

