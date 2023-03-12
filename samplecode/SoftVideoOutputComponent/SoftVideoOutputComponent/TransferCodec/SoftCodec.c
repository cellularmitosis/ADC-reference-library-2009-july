/*
	File:		SoftCodec.c
	
	Description: This is an example of a Transfer Codec - it is dedicated to be used with SoftVOut
				 a fake software implementation of a video output 'vout' component.

	Author:		QuickTime Engineering, QuickTime DTS

	Copyright: 	© Copyright 1998 - 2005 Apple Computer, Inc. All rights reserved.
	
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
				
	Change History (most recent first): <4> dts 08/04/05 Updated to build universal binary
                                        <3> dts 02/17/04 Added the ability to support echo port for 'raw ' source
	                                                    through shared vout globals and added simple scaling
	                                                    Updated to CW 9 and added Xcode project
                                        <2> dts 10/28/03 QuickTime 6.4 changed the 'raw ' case to now recieve
													    a NewImageGWorld call:
													      Implemented NewImageGWorld
													      Fixed BeginBand to calculate srcRowBytes correctly
													      correcting "skewed pixel" issue when using transfer codec 
                                        <1> 12/20/01 Initial Release
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
#else
    #include <ConditionalMacros.h>
    #include <Endian.h>
    #include <ImageCodec.h>
    #include <Resources.h>
    #include <QuickTimeComponents.h>
#endif

// Our Private Pixel Format
// To register a new fourCC please send mail to qtfourCC@apple.com.
// Include an email address for future correspondence, the fourCC you
// would like to register, and a brief description of the fourCC format.
// For more information refer to IceFloe #20 - QuickTime Pixel Format FourCCs
// http://developer.apple.com/quicktime/icefloe/dispatch020.html
enum {
	kSoftPixelFormat = FOUR_CHAR_CODE('soft')
};

// Constants
const UInt8 kNumPixelFormatsSupported = 2;
const UInt8 kCodecInfoResNum = 129;

// Because this is a custom codec which implements a custom pixel format, you have to provide
// all codec functionality, for example Masking(Clipping) and Scaling, yourself. QuickTime
// isn't going to do any of this work for you.
// NOTE: This example implements scaling in the ScaleDraw function.
const long kCodecCapabilitiesFlags = codecCanMask | codecCanScale;

// Per frame globals
typedef struct {
	UInt32	width;
	UInt32	height;
    UInt32  destWidth;
    UInt32  destHeight;
	UInt32	depth;
	UInt32	maxBytesPerRow;
	UInt32	maxRows;
    UInt32	srcRowBytes;
} SoftCodecDecompressRecord;

// VOut component shared globals
typedef struct {
	CGrafPtr echoPort;
	Ptr      baseAddr;
	long	 rowBytes;
	short	 width;
	short	 height;
	Boolean  isEchoPortOn;
} SharedGlobalsRecord, *SharedGlobalsPtr;

// Per instance globals
typedef struct {
	ComponentInstance		self;
	ComponentInstance		target;
	ComponentInstance		baseCodec;
	SharedGlobalsPtr		sharedGlobals;
	OSType					codecType;
	OSType**				wantedDestinationPixelTypeH;
	ImageCodecMPDrawBandUPP drawBandUPP;
} SoftCodecGlobalsRecord, *SoftCodecGlobals;

static ComponentResult GetVOutSharedGlobals(SoftCodecGlobals storage);
static void ScaleDraw(UInt32 *pSrc, UInt32 *pDest, UInt32 srcHeight, UInt32 srcWidth, UInt32 dstHeight, UInt32 dstWidth, UInt32 srcRowBytes, UInt32 dtsRowBytes);

/************************************************************************************/
// Setup required for ComponentDispatchHelper.c

#define IMAGECODEC_BASENAME()		SoftCodec_
#define IMAGECODEC_GLOBALS()		SoftCodecGlobals storage

#define CALLCOMPONENT_BASENAME()	IMAGECODEC_BASENAME()
#define CALLCOMPONENT_GLOBALS()		IMAGECODEC_GLOBALS()

#define COMPONENT_DISPATCH_FILE		"SoftCodecDispatch.h"
#define	GET_DELEGATE_COMPONENT()	(storage->baseCodec)
#define COMPONENT_UPP_SELECT_ROOT()	ImageCodec

#if __APPLE_CC__ || __MACH__
	#include <CoreServices/Components.k.h>			// Standard .k.h
	#include <QuickTime/ImageCodec.k.h>
	#include <QuickTime/ComponentDispatchHelper.c>	// Make the dispatcher and canDo
#else
	#include "Components.k.h"				// Standard .k.h
	#include "ImageCodec.k.h"
	#include "ComponentDispatchHelper.c"	// Make the dispatcher and canDo
#endif

#pragma mark-

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
pascal ComponentResult SoftCodec_Open(SoftCodecGlobals glob, ComponentInstance self)
{
    ComponentDescription cd;
	GDHandle 			 mainGD;
	OSType				 voutPixelFormat;
	
	ComponentResult		 err;

	// Allocate memory for our globals, set them up and inform the component
	// manager that we've done so
	glob = (SoftCodecGlobals)NewPtrClear(sizeof(SoftCodecGlobalsRecord));
	if ((err = MemError())) goto bail;
	
	SetComponentInstanceStorage(self, (Handle)glob);
	glob->self	 = self;
	glob->target = self;
	glob->sharedGlobals = NULL;
	glob->drawBandUPP = NULL;
	
	// Open and target an instance of the base decompressor
	err	= OpenADefaultComponent(decompressorComponentType, kBaseCodecType, &glob->baseCodec);
	if (err) goto bail;

	// Set us as the base component's target
	CallComponentTarget(glob->baseCodec, self);
	
    // Get globals shared with VideoOutput Component
	err = GetVOutSharedGlobals(glob);
	if (err) goto bail;
	
	// Record our codecType for posterity
	err = GetComponentInfo((Component)self, &cd, NULL, NULL, NULL);
	if (err) goto bail;
	
	glob->codecType	= cd.componentSubType;

	// NOTE: YOU DON'T NEED TO DO THIS
	// Check to see if the codec type matches the pixel format of the main device.
	// This is done here because SoftVOut is using the main display to simulate hardware
    // which requires a bitdepth of 32. This will either be ARGB or BGRA (in the case of an Intel-based Mac)
    // and if the main display is not set up for this depth we bail
	mainGD = GetMainDevice();
	voutPixelFormat	= GETPIXMAPPIXELFORMAT(*mainGD[0]->gdPMap);
	if (glob->codecType == kRawCodecType && (voutPixelFormat == k32ARGBPixelFormat || voutPixelFormat == k32BGRAPixelFormat)) {
		voutPixelFormat	= kRawCodecType;
	}

	if (glob->codecType == voutPixelFormat) {
		// Allocate memory for our wantedDestinationPixelType list, we fill it in during the Preflight call.
		glob->wantedDestinationPixelTypeH = (OSType **)NewHandle(sizeof(OSType) * (kNumPixelFormatsSupported + 1));
	} else {
		err = codecConditionErr;
	}

bail:
	return err;
}

// Component Close Request - Required
pascal ComponentResult SoftCodec_Close(SoftCodecGlobals glob, ComponentInstance self)
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
pascal ComponentResult SoftCodec_Version(SoftCodecGlobals glob)
{
#pragma unused(glob)

	return (codecInterfaceVersion << 2) + 1;
}

// Component Register Request
pascal ComponentResult SoftCodec_Register(SoftCodecGlobals glob)
{
#pragma unused(glob)

	// Always register
	return noErr;
}

// Component Target Request
// 		Allows another component to "target" you i.e., you call another component whenever
// you would call yourself (as a result of your component being used by another component).
pascal ComponentResult SoftCodec_Target(SoftCodecGlobals glob, ComponentInstance target)
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

// Component GetMPWorkFunction Request
//		Allows your image decompressor component to perform asynchronous decompression
// in a single MP task by taking advantage of the Base Decompressor. If you implement
// this selector, your DrawBand function must be MP-safe. MP safety means not
// calling routines that may move or purge memory and not calling any routines which
// might cause 68K code to be executed. Ideally, your DrawBand function should not make
// any API calls whatsoever. Obviously don't implement this if you're building a 68k component.
#if !TARGET_CPU_68K
pascal ComponentResult SoftCodec_GetMPWorkFunction(SoftCodecGlobals glob, ComponentMPWorkFunctionUPP *workFunction, void **refCon)
{
	if (NULL == glob->drawBandUPP)
		#if !TARGET_API_MAC_CARBON
			glob->drawBandUPP = NewImageCodecMPDrawBandProc(SoftCodec_DrawBand);
		#else
			glob->drawBandUPP = NewImageCodecMPDrawBandUPP((ImageCodecMPDrawBandProcPtr)SoftCodec_DrawBand);
		#endif
		
	return ImageCodecGetBaseMPWorkFunction(glob->baseCodec, workFunction, refCon, glob->drawBandUPP, glob);
}
#else
	#error "You're not really building a 68K Component are you?"
#endif // !TARGET_CPU_68K

#pragma mark-
/************************************************************************************/
// Base Component Calls

// ImageCodecInitialize
//		The first function call that your image decompressor component receives from the base image
// decompressor is always a call to ImageCodecInitialize . In response to this call, your image decompressor
// component returns an ImageSubCodecDecompressCapabilities structure that specifies its capabilities.
pascal ComponentResult SoftCodec_Initialize(SoftCodecGlobals glob, ImageSubCodecDecompressCapabilities *cap)
{
#pragma unused(glob)

	cap->decompressRecordSize = sizeof(SoftCodecDecompressRecord);
	cap->canAsync			  = true;

	return noErr;
}

// ImageCodecPreflight
// 		The base image decompressor gets additional information about the capabilities of your image
// decompressor component by calling ImageCodecPreflight. The base image decompressor uses this
// information when responding to a call to the ImageCodecPredecompress function,
// which the ICM makes before decompressing an image. You are required only to provide values for
// the wantedDestinationPixelSize and wantedDestinationPixelTypes fields and can also modify other
// fields if necessary.
pascal ComponentResult SoftCodec_Preflight(SoftCodecGlobals glob, CodecDecompressParams *p)
{
	CodecCapabilities *capabilities = p->capabilities;
	OSTypePtr		  formats = *glob->wantedDestinationPixelTypeH;
	
	// Fill in formats for wantedDestinationPixelTypeH
	// Terminate with an OSType value 0  - see IceFloe #7
	// http://developer.apple.com/quicktime/icefloe/dispatch007.html
	*formats++	= kSoftPixelFormat;
	*formats++  = k32ARGBPixelFormat;
	*formats++	= 0;
	
	// Reject anything other than 32
	if ((**p->imageDescription).depth != 32)
		return codecConditionErr;
	
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
pascal ComponentResult SoftCodec_BeginBand(SoftCodecGlobals glob, CodecDecompressParams *p, ImageSubCodecDecompressRecord *drp, long flags)
{
#pragma unused(glob, flags)

	SoftCodecDecompressRecord *myDrp = (SoftCodecDecompressRecord *)drp->userDecompressRecord;
	long					  condition = (unsigned short)p->conditionFlags;
	ICMProgressProcRecord 	  progressProcRecord;
	ICMDataProcRecord		  dataProcRecord;

	// Save the height and width in our private decompress
	// record so we'll have it available in DrawBand.
    myDrp->width = p->srcRect.right - p->srcRect.left;
	myDrp->height = p->srcRect.bottom - p->srcRect.top;
    myDrp->destWidth = p->dstRect.right - p->dstRect.left;
    myDrp->destHeight = p->dstRect.bottom - p->dstRect.top;
    
	// We also need these max values for DrawBand.
	myDrp->maxBytesPerRow = (p->dstPixMap.bounds.right - p->dstPixMap.bounds.left) * 4;
	myDrp->maxBytesPerRow -= p->dstRect.left * 4;
	myDrp->srcRowBytes = p->bufferSize / myDrp->height;
	
	myDrp->maxRows = (p->dstPixMap.bounds.bottom - p->dstPixMap.bounds.top);
	myDrp->maxRows -= p->dstRect.top;

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
pascal ComponentResult SoftCodec_DrawBand(SoftCodecGlobals glob, ImageSubCodecDecompressRecord *drp)
{
#pragma unused(glob)

	SoftCodecDecompressRecord *myDrp = (SoftCodecDecompressRecord *)drp->userDecompressRecord;
	
	OSErr err = noErr;
    
    // When the echo port is on, your transfer codec is responsible for drawing
    // to both the Video Output hardware and to the regular destination given to your codec
    // In this sample we're using shared globals to keep both Video Output and Transfer Codec
    // in sync with each other
	
	if (glob->sharedGlobals->isEchoPortOn) {
		// ******* ECHO PORT ON - draw to the VideoOutput destination first...
		Ptr src			   = drp->codecData;
		Ptr dest		   = glob->sharedGlobals->baseAddr;
		UInt32 srcRowBytes = myDrp->srcRowBytes;
		UInt32 dstRowBytes = glob->sharedGlobals->rowBytes;
		UInt32 bytesPerRow;
		UInt32 height;
		
		ScaleDraw((UInt32 *)src, (UInt32 *)dest,
                  myDrp->height, myDrp->width,
			      glob->sharedGlobals->height, glob->sharedGlobals->width,
			      srcRowBytes, dstRowBytes);
		
		// ...Now draw to the echo port
		dest	 	= drp->baseAddr;
		dstRowBytes = drp->rowBytes;
		srcRowBytes = myDrp->srcRowBytes;
		bytesPerRow = myDrp->width * 4;
		height 	 	= myDrp->height;
        
        ScaleDraw((UInt32 *)src, (UInt32 *)dest,
                  myDrp->height, myDrp->width,
			      myDrp->destHeight, myDrp->destWidth,
                  srcRowBytes, dstRowBytes);
	
	} else {
		// ******* ECHO PORT OFF - just draw to the VideoOutput destination
		Ptr src		 	   = drp->codecData;
		Ptr dest	 	   = drp->baseAddr;
		UInt32 srcRowBytes = myDrp->srcRowBytes;
		UInt32 dstRowBytes = drp->rowBytes;
		UInt32 bytesPerRow = myDrp->width * 4;
		UInt32 height 	   = myDrp->height;
		
		ScaleDraw((UInt32 *)src, (UInt32 *)dest,
                  myDrp->height, myDrp->width,
                  glob->sharedGlobals->height, glob->sharedGlobals->width,
                  srcRowBytes, dstRowBytes);
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
pascal ComponentResult SoftCodec_EndBand(SoftCodecGlobals glob, ImageSubCodecDecompressRecord *drp, OSErr result, long flags)
{
#pragma unused(glob, drp, result, flags)

	return noErr;
}

// See Q&A 1157 http://developer.apple.com/qa/qa2001/qa1157.html regarding the next two selectors and how they relate to supporting
// supports asynchronous scheduled decompression 

// ImageCodecQueueStarting
// 		If your component supports asynchronous scheduled decompression, the base image decompressor calls your image decompressor component's
// ImageCodecQueueStarting function before decompressing the frames in the queue. Your component is not required to implement this function.
// It can implement the function if it needs to perform any tasks at this time, such as locking data structures.
// The base image decompressor never calls the ImageCodecQueueStarting function at interrupt time.
pascal ComponentResult SoftCodec_QueueStarting(SoftCodecGlobals glob)
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
pascal ComponentResult SoftCodec_QueueStopping(SoftCodecGlobals glob)
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
pascal ComponentResult SoftCodec_GetCodecInfo(SoftCodecGlobals glob, CodecInfo *info)
{
	ComponentInstance self = glob->self;
	CodecInfo**		  hInfo;
	
	OSErr			  err = noErr;

	if (!info) return paramErr;
	
	// Use component refcon (shared by all codec instance) to cache
	// codec info resource.
	hInfo = (CodecInfo **)GetComponentRefcon((Component)self);
	if (NULL == hInfo  ||  NULL == *hInfo) {
		
		if (NULL != hInfo) {
			DisposeHandle((Handle) hInfo);
			SetComponentRefcon((Component)self, 0);
		}
		
		#if !TARGET_API_MAC_CARBON
		{
			THz	saveZone;
			saveZone = GetZone();
			SetZone(SystemZone());
		#endif
		err = GetComponentResource((Component) self, codecInfoResourceType, kCodecInfoResNum, (Handle *)&hInfo);
		if (noErr == err && ((err = ResError()) == noErr)) {
			DetachResource((Handle)hInfo);
			SetComponentRefcon((Component)self, (long)hInfo);
		}

		#if !TARGET_API_MAC_CARBON
			SetZone(saveZone);
		}
		#endif
	}

	if (noErr == err)
	{
		BlockMoveData((Ptr)*hInfo, (Ptr)info, sizeof(CodecInfo));
	}

	return err;
}

// ImageCodecNewImageGWorld
// 		Allows your transfer codec to allocate memory for an offscreen buffer for your destination.
pascal ComponentResult SoftCodec_NewImageGWorld(SoftCodecGlobals glob, CodecDecompressParams *p, GWorldPtr *newGW, long flags)
{
#pragma unused(glob, flags)

	PixMapPtr portPixMap;
  	Rect	  theRect;
  	
  	OSErr	 err = codecConditionErr;

	// Make sure the destination port is our VOut. This is done by checking
	// the pixelFormat of the destination image. If the pixel format isn't ours,
	// (kSoftPixelFormat) it is a bogus usage of the transfer codec.
	#if !TARGET_API_MAC_CARBON
		portPixMap = *p->port->portPixMap;
	#else
		portPixMap = *(GetPortPixMap(p->port));
	#endif
	if (GETPIXMAPPIXELFORMAT(portPixMap) == kSoftPixelFormat) {
		theRect.top	= theRect.left = 0;
        theRect.right	= (*p->imageDescription)->width;
		theRect.bottom	= (*p->imageDescription)->height;
		
		err = QTNewGWorld(newGW, k32ARGBPixelFormat, &theRect, NULL, NULL, 0);
	}

	return err;
}

// ImageCodecDisposeImageGWorld
pascal ComponentResult SoftCodec_DisposeImageGWorld(SoftCodecGlobals glob, GWorldPtr theGW)
{
#pragma unused(glob)

	DisposeGWorld(theGW);

	return noErr;
}

#pragma mark-
/************************************************************************************/
// Local Component Calls

// Get globals shared with VideoOutput Component
static ComponentResult GetVOutSharedGlobals(SoftCodecGlobals storage)
{
	Component c = 0;
	ComponentDescription cd = { QTVideoOutputComponentType,
								FOUR_CHAR_CODE('soft'),
								FOUR_CHAR_CODE('appl'),
								0, 0 };
	
	ComponentResult err = badComponentType;
		
	c = FindNextComponent(c, &cd);
	
	if (c) {
		storage->sharedGlobals = (SharedGlobalsPtr)GetComponentRefcon((Component)c);
		if (storage->sharedGlobals) err = noErr;
	}

	return err;
}

// Implementation of "Fast Anamorphic Image Scaling"
// as described in Graphics Gems II [p.78]Dale A. Schumacher
// While not particularly accurate it works well in the context of this sample
static void ScaleDraw(UInt32 *pSrc,       UInt32 *pDest,
                      UInt32 srcHeight,   UInt32 srcWidth,
                      UInt32 dstHeight,   UInt32 dstWidth,
                      UInt32 srcRowBytes, UInt32 dstRowBytes)
{
    UInt32 dX, dY, sX, sY;
    UInt32 *dstScan;
    UInt32 *srcScan;
    
    UInt32 srcRow = srcRowBytes / 4;
    UInt32 dstRow = dstRowBytes / 4;
        
    for (dY = 0; dY < dstHeight; dY++) {
        sY = ((dY * srcHeight) / dstHeight);
      	srcScan = pSrc + (sY * srcRow);
        dstScan = pDest + (dY * dstRow);

        for (dX = 0; dX < dstWidth; dX++){
            sX = ((dX * srcWidth) / dstWidth);
            
            dstScan[dX] = srcScan[sX];
        }
    }
}