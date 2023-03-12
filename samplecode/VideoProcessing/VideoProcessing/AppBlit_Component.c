/*
	File:		AppBlit_Component.c
	
	Description: Custom decompressor component. We'll use this component to overlay
                images on top of our data, or to perform color-clamping of the data.

	Copyright: 	© Copyright 2003 Apple Computer, Inc. All rights reserved.
	
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

*/

#include "AppBlit_Component.h"
#include "VideoProcessing.h"

// ---------------------------------------------------------------------------
//		D I S P A T C H   H E L P E R   S T U F F
// ---------------------------------------------------------------------------

#define IMAGECODEC_BASENAME()		AppBlit_
#define IMAGECODEC_GLOBALS()		AppBlitGlobalsPtr storage

#define COMPONENT_DISPATCH_FILE		"AppBlitComponentDispatch.h"

#define GET_DELEGATE_COMPONENT()	(storage->delegateComponent)

#define CALLCOMPONENT_BASENAME()	IMAGECODEC_BASENAME()
#define	CALLCOMPONENT_GLOBALS()		IMAGECODEC_GLOBALS()

#define COMPONENT_UPP_PREFIX()		uppImageCodec
#define COMPONENT_SELECT_PREFIX()  	kImageCodec
#define COMPONENT_C_DISPATCHER		(1)

#define DECO_BUILD					(1)

#if __MACH__
	#include <CoreServices/Components.k.h>
	#include <QuickTime/ImageCodec.k.h>
	#include <QuickTime/ComponentDispatchHelper.c>
#else
	#include <Components.k.h>
	#include <ImageCodec.k.h>
	#include <ComponentDispatchHelper.c>
#endif


//////////
//
// module variables
//
//////////

static Boolean mInstalled = false;

// ---------------------------------------------------------------------------
// COMPONENT ENTRY POINTS
// ---------------------------------------------------------------------------

//////////
//
// AppBlit_Open
//
//////////

pascal ComponentResult AppBlit_Open(AppBlitGlobalsPtr glob, ComponentInstance self)
{
	ComponentResult err;

	glob = (AppBlitGlobalsPtr)NewPtrClear(sizeof(AppBlitGlobalsRecord));
	if ( glob == NULL ) goto bail;

	SetComponentInstanceStorage(self, (Handle)glob);

	glob->self 		= self;
	glob->target 	= self;

	err = OpenADefaultComponent(decompressorComponentType, kBaseCodecType, &glob->delegateComponent);
	if (err != noErr ) goto bail;

	ComponentSetTarget(glob->delegateComponent, self);

	glob->wantedDestinationPixelTypeH = (OSType **)NewHandle(sizeof(OSType) * (kNumPixelFormatsSupported + 1));
	err = MemError();
	
bail:
	return err;
}

//////////
//
// AppBlit_Close
//
//////////

pascal ComponentResult AppBlit_Close(AppBlitGlobalsPtr glob, ComponentInstance self)
{
#pragma unused (self)

	if (glob != NULL) 
		{
		if (glob->delegateComponent)
			CloseComponent(glob->delegateComponent);
		DisposeHandle((Handle)glob->wantedDestinationPixelTypeH);
		DisposePtr((Ptr)glob);
		}
	return noErr;
}


//////////
//
// AppBlit_Target
//
//////////

pascal ComponentResult AppBlit_Target(AppBlitGlobalsPtr glob, ComponentInstance target)
{
	glob->target = target;

	return noErr;
}


//////////
//
// AppBlit_Version
//
//////////

pascal ComponentResult AppBlit_Version(AppBlitGlobalsPtr glob)
{
#pragma unused(glob)
	return 0x00010000;
}


//////////
//
// AppBlit_Initialize
//
//////////

pascal ComponentResult AppBlit_Initialize(AppBlitGlobalsPtr glob, ImageSubCodecDecompressCapabilities *cap)
{
#pragma unused(glob)

	cap->decompressRecordSize = sizeof(AppBlitDecompressRecord); 
	cap->canAsync = false;
	
	return noErr;
}


//////////
//
// AppBlit_Preflight
//
//////////

pascal ComponentResult AppBlit_Preflight(AppBlitGlobalsPtr glob, CodecDecompressParams *p)
{
	CodecCapabilities  	*capabilities = p->capabilities;
 	OSErr				err = noErr;
	OSType 				*formats = *glob->wantedDestinationPixelTypeH;

	// list of output pixel formats we support	
	*formats++ = k32ARGBPixelFormat;
	*formats++ = 0;
	p->capabilities->wantedPixelSize = 0; 	
	p->wantedDestinationPixelTypes = glob->wantedDestinationPixelTypeH;

	capabilities->bandMin 		= (**p->imageDescription).height;
	capabilities->bandInc 		= (**p->imageDescription).height;
	capabilities->extendWidth  	= 0;
	capabilities->extendHeight 	= 0;
	
   	if( p->imageDescription == NULL )
   		return paramErr;

	return err;
}


//////////
//
// AppBlit_BeginBand
//
//////////

pascal ComponentResult AppBlit_BeginBand(AppBlitGlobalsPtr glob, CodecDecompressParams *p, ImageSubCodecDecompressRecord *drp, long flags)
{
#pragma unused(glob, flags)
	AppBlitDecompressRecord 	*myDrp = (AppBlitDecompressRecord *)drp->userDecompressRecord;
	ComponentResult				err = noErr;
    PixMap						dstPixMap = p->dstPixMap;
    Rect						dstRect = p->dstRect;
	long						offsetH,offsetV;
	PixMapHandle				pix;
	
	offsetH = (dstRect.left - dstPixMap.bounds.left);
	if (dstPixMap.pixelSize == 16)
		{
		offsetH <<= 1;					/* 1 pixel = 2 bytes */
		}
	else
		{
		if (dstPixMap.pixelSize == 32)
			{
			offsetH <<= 2;					/* 1 pixel = 4 bytes */
			}
		}
    
    // store away info about the place we draw
    myDrp->dstRect		= dstRect;
	myDrp->width 		= dstRect.right - dstRect.left;
	myDrp->height 		= dstRect.bottom - dstRect.top;
	myDrp->pixelFormat 	= GETPIXMAPPIXELFORMAT(&p->dstPixMap);
	myDrp->rowBytes		= QTGetPixMapPtrRowBytes(&dstPixMap);

	offsetV = (dstRect.top - dstPixMap.bounds.top) * myDrp->rowBytes;
	myDrp->baseAddr = dstPixMap.baseAddr + offsetH + offsetV;
			
	pix = GetGWorldPixMap(((GWorldPtr*)p->data)[0]);
	myDrp->source.baseAddr = GetPixBaseAddr(pix);
	myDrp->source.rowBytes = GetPixRowBytes(pix);
	myDrp->source.layerBounds = (**pix).bounds;

	pix = GetGWorldPixMap(((GWorldPtr*)p->data)[1]);
	if (pix)
		{
		myDrp->overlay.baseAddr = GetPixBaseAddr(pix);
		myDrp->overlay.rowBytes = GetPixRowBytes(pix);
		myDrp->overlay.layerBounds = (**pix).bounds;
		}
	else
		myDrp->overlay.baseAddr = nil;
	myDrp->redCount = ((UInt32**)p->data)[2];
	myDrp->greenCount = ((UInt32**)p->data)[3];
	myDrp->blueCount = ((UInt32**)p->data)[4];
	
	myDrp->redMin = ((long*)p->data)[5];
	myDrp->redMax = ((long*)p->data)[6];
	myDrp->greenMin = ((long*)p->data)[7];
	myDrp->greenMax = ((long*)p->data)[8];
	myDrp->blueMin = ((long*)p->data)[9];
	myDrp->blueMax = ((long*)p->data)[10];

	return err;
}


//////////
//
// Overlay32
//
//////////

static void Overlay32(
			LayerPtr overlayLayer, 	// input: layer to float on top
			LayerPtr sourceLayer, 	// input: layer to be under the overlay
			LayerPtr destLayer)		// output: layer to render into
{
	long	rows = destLayer->layerBounds.bottom - destLayer->layerBounds.top;
	Ptr		rowDestBase, rowSourceBase;
	Ptr		rowOverlayBase;
	static 	Boolean tablesBuilt = false;
	static 	UInt8 	alphaT[256][256], notalphaT[256][256];

	rowDestBase = destLayer->baseAddr;
	rowSourceBase = sourceLayer->baseAddr;
	rowOverlayBase = overlayLayer->baseAddr;

	// build blend tables
	if (!tablesBuilt)
		{
		int i, j;
		
		for (i = 0; i < 256; ++i)
			for (j = 0; j < 256; ++j)
				{
				alphaT[i][j] 	= i*j/255;
				notalphaT[i][j] = i*(255-j)/255;
				}
		tablesBuilt = true;
		}

	do
		{
		long		cols = destLayer->layerBounds.right - destLayer->layerBounds.left;
		UInt32  	*destBaseAddr;
		UInt32		*overlayBaseAddr;
		UInt32		*sourceBaseAddr;
		
		destBaseAddr = (UInt32*)rowDestBase;
		rowDestBase += destLayer->rowBytes;

		sourceBaseAddr = (UInt32*)rowSourceBase;
		rowSourceBase += sourceLayer->rowBytes;
		overlayBaseAddr = (UInt32*)rowOverlayBase;
		rowOverlayBase += overlayLayer->rowBytes;
		do
			{
			UInt32	overlayPixel;
			UInt8	alpha;
			
			if (rowOverlayBase)
				{
				overlayPixel = *overlayBaseAddr++;
				alpha = (overlayPixel >> 24) & 0xff;
				}
			else
				alpha = 0;
				
			if (alpha == 0)
				overlayPixel = *sourceBaseAddr;
			else
			if (alpha == 0xff)
				{
				//overlayPixel = overlayerPixe;
				}
			else
				{
				UInt32	sourcePixel;
				
				sourcePixel = *sourceBaseAddr;
				overlayPixel = 
					( (UInt32)(alphaT[(overlayPixel >> 16) & 0xff][alpha] 	+ notalphaT[(sourcePixel >> 16) & 0xff][alpha]) << 16) |
					( (UInt32)(alphaT[(overlayPixel >> 8) & 0xff][alpha] 	+ notalphaT[(sourcePixel >> 8) & 0xff][alpha]) << 8) |
					( (UInt32)(alphaT[overlayPixel & 0xff][alpha] 			+ notalphaT[sourcePixel & 0xff][alpha]) );
				}
				
			overlayPixel |= 0xFF000000;
			*destBaseAddr++ = overlayPixel;

			sourceBaseAddr++;
			} while (--cols);
		} while (--rows);
		
} // Overlay32

//////////
//
// CalcClamp
//
//////////

static UInt32 CalcClamp(int i, long min, long max)
{
	if (min < max)
		return min + (max - min) * i / 255;
	else
		{
		if (min > max)
			{
			UInt32 midPoint;
			
			midPoint = (min - max) / 2 + max;
			if (i <= midPoint)
				return i * max / (midPoint - 0);
			else
				return min + (i - midPoint) * (255-min) / (255 - midPoint);
			}
		}
	return 0;
}

//////////
//
// Clamp32
//
//////////

static void Clamp32(
			LayerPtr sourceLayer, 	// input: layer to be draw
			LayerPtr destLayer,		// output: layer to render into
			long minRed, long maxRed, // input: clamp to betwen, or outside of these values
			long minGreen, long maxGreen,
			long minBlue, long maxBlue,
			UInt32 *redCount, UInt32 *greenCount, UInt32 *blueCount)
{
	long	rows = destLayer->layerBounds.bottom - destLayer->layerBounds.top;
	Ptr		rowDestBase, rowSourceBase;
	UInt32	clampRed[256], clampGreen[256], clampBlue[256];
	int		i;
	
	rowDestBase = destLayer->baseAddr;
	rowSourceBase = sourceLayer->baseAddr;

	// build clamp tables
	for (i = 0; i < 256; ++i)
		{		
		redCount[i] = 0;
		greenCount[i] = 0;
		blueCount[i] = 0;
		
		clampRed[i] = CalcClamp(i, minRed, maxRed);
		clampGreen[i] = CalcClamp(i, minGreen, maxGreen);
		clampBlue[i] = CalcClamp(i, minBlue, maxBlue);			
		}
		
	do
		{
		long		cols = destLayer->layerBounds.right - destLayer->layerBounds.left;
		UInt32  	*destBaseAddr;
		UInt32		*sourceBaseAddr;
		
		destBaseAddr = (UInt32*)rowDestBase;
		rowDestBase += destLayer->rowBytes;

		sourceBaseAddr = (UInt32*)rowSourceBase;
		rowSourceBase += sourceLayer->rowBytes;
		do
			{
			UInt32 	overlayPixel;
			UInt32	sourcePixel;
			UInt32	red, green, blue;
			
			sourcePixel = *sourceBaseAddr++;
			red = clampRed[(sourcePixel >> 16) & 0xff];
			green = clampGreen[(sourcePixel >> 8) & 0xff];
			blue = clampBlue[(sourcePixel >> 0) & 0xff];
			overlayPixel = 0xFF000000 | ( ( red << 16 )  | (green << 8) | (blue << 0));
			
			redCount[red]++;
			greenCount[green]++;
			blueCount[blue]++;
			
			*destBaseAddr++ = overlayPixel;
			} while (--cols);
		} while (--rows);
		
} // Clamp32

//////////
//
// AppBlit_DrawBand
//
//////////

pascal ComponentResult AppBlit_DrawBand(AppBlitGlobalsPtr glob, ImageSubCodecDecompressRecord *drp)
{
#pragma unused (glob)

	OSErr					err = noErr;
	AppBlitDecompressRecord *myDrp = (AppBlitDecompressRecord *)(drp->userDecompressRecord);
	

	myDrp->dest.layerBounds = myDrp->dstRect;
	myDrp->dest.baseAddr = myDrp->baseAddr;
	myDrp->dest.rowBytes = myDrp->rowBytes;
				
		
	if (myDrp->overlay.baseAddr)
		Overlay32(			&myDrp->overlay,
							&myDrp->source,
							&myDrp->dest);
	else
		Clamp32(&myDrp->source,
				&myDrp->dest, 
				myDrp->redMin, myDrp->redMax,
				myDrp->greenMin, myDrp->greenMax,
				myDrp->blueMin, myDrp->blueMax,
				myDrp->redCount, myDrp->greenCount, myDrp->blueCount);

	return err;
}

//////////
//
// AppBlit_EndBand
//
//////////

pascal ComponentResult AppBlit_EndBand(AppBlitGlobalsPtr glob, ImageSubCodecDecompressRecord *drp, OSErr result, long flags)
{
#pragma unused(glob, drp,result, flags)
	
	return noErr;
}

//////////
//
// AppBlit_QueueStarting
//
//////////

pascal ComponentResult AppBlit_QueueStarting(AppBlitGlobalsPtr glob)
{
#pragma unused(glob)
	return noErr;
}

//////////
//
// AppBlit_QueueStopping
//
//////////

pascal ComponentResult AppBlit_QueueStopping(AppBlitGlobalsPtr glob)
{
#pragma unused(glob)
	return noErr;
}


//////////
//
// AppBlit_GetCompressedImageSize
//
//////////

pascal ComponentResult AppBlit_GetCompressedImageSize(AppBlitGlobalsPtr glob, ImageDescriptionHandle desc, Ptr data, long dataSize, ICMDataProcRecordPtr dataProc, long *size)
{
#pragma	unused(glob,dataSize,desc,data, dataProc)

	if (size == NULL) 
		return paramErr;

	return noErr;
}


//////////
//
// AppBlit_GetCodecInfo
//
//////////

pascal ComponentResult AppBlit_GetCodecInfo(AppBlitGlobalsPtr glob, CodecInfo *info)
{
#pragma unused (glob)
	OSErr err = noErr;

	if (info)
		{
		BlockMoveData("WWDC", &info->typeName[1], 4);
		info->typeName[0] = 4;
		info->version = 1;
		info->revisionLevel = 0;
		info->vendor = 'appl';
		info->decompressFlags = codecInfoDoes32 | codecInfoDoes16 |
					codecInfoDoesSpool | codecInfoDoesMask | 
					codecInfoDoesStretch | codecInfoDoesShrink | 
					codecInfoDoesDouble | codecInfoDoesQuad | codecInfoDoesHalf | codecInfoDoesQuarter;
		info->compressFlags = 0;
		info->formatFlags = codecInfoDepth32 | codecInfoDepth16;
		info->compressionAccuracy = 255;
		info->decompressionAccuracy = 255;
		info->compressionSpeed = 0;
		info->decompressionSpeed = 200;
		info->compressionLevel = 0;
		info->minimumHeight = 1;
		info->minimumWidth = 1;
		info->decompressPipelineLatency = 0;
		info->compressPipelineLatency = 0;
		}
	else
		err = paramErr;
		
	return err;
}


//////////
//
// InstallAppBlitComponentCodec
//
//////////

void InstallAppBlitComponentCodec(void)
{
	ComponentDescription td;
	Component	c = nil;
	Handle	dname;
		
	if (mInstalled)
		return;
			
	if (c == nil)
    {
        dname = NewHandle(12);
        td.componentType = decompressorComponentType;
        td.componentSubType = kCustomDecompressorType;
        td.componentManufacturer = kCustomDecompressorType;
        td.componentFlags = codecInfoDoes32;
        td.componentFlagsMask = 0;
        
        BlockMoveData("\pApplication",*dname,12);
        c= RegisterComponent(&td,NewComponentRoutineUPP((ComponentRoutineProcPtr) AppBlit_ComponentDispatch), 0,dname,nil, nil);
        if (dname)
            DisposeHandle(dname);
    }
	
	// if we got any version, we're fine
	if (c)
    {	
		mInstalled = true;
    }

		
} // InstallAppBlitComponentCodec

