/*
	File:		Effect.c
	
	Description: QuickTime effect component framework

	Author:		QuickTime Engineering

	Copyright: 	© Copyright 1997-2002 Apple Computer, Inc. All rights reserved.
	
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
	   <1>	 	05/11/02	era		updated for X
*/

/*
	Dimmer2 Effect - a sample QuickTime video effect.
	
	This effect uses two sources as input, and renders that source with
	a dim value that starts at full on, and ramps to full off. This isn't very
	useful as a real effect, but shows instead how to create an effect.
	
	READ THIS PARAGRAPH BEFORE DOING ANYTHING ELSE IN THIS FILE:
	To find things to change to implement your effect, seach for *** CHANGE ***.  
	Code that will require modification is bracketted by CHANGE/END CHANGE.
	
	There are eleven places to change code, and one place where you write new
	code that implements your actual effect.  MAKE SURE YOU LOOK AT ALL OF THEM.
	
	*IMPORTANT*
	
	You MUST also ensure that the Effect.r file is kept in sync with this code.
	In particular it is very important that you update the 'atms' resource
	description so that the parameters of your effect are correctly described.
	Full details of the format of the 'atms' resource can be found in the Effects
	chapter of the QuickTime documentation
	
	ALSO PLEASE NOTE THAT THE LINK WARNING ABOUT THE COMPONENT DISPATCH ENTRY POINT
	NOT BEING A ROUTINE DESCRIPTOR IS NORMAL AND CAN BE IGNORED WHEN BUILDING THE PPC
	TARGET.
	
	written by Tom Dowdy and Dan Crow
	© Copyright 1997-2002 Apple Computer, Inc. All rights reserved.
*/

// --------------------------------------------------------------------------------------
// INCLUDES
// --------------------------------------------------------------------------------------

#if (__APPLE_CC__ || __MACH__)
    #include <Carbon/Carbon.h>
    #include <QuickTime/QuickTime.h>
#elif TARGET_API_MAC_CARBON
	#include <Carbon.h>
	#include <QuickTime.h>
#else
    #include <ConditionalMacros.h>
	#include <ImageCodec.h>
	#include <QuickDraw.h>
#endif

#include "EffectDefinitions.h"

/*#if PRAGMA_STRUCT_ALIGN
	#pragma options align=mac68k
#endif*/

#include "EffectUtilities.h"

// --------------------------------------------------------------------------------------
// INTERNAL DEFINES
// --------------------------------------------------------------------------------------

// *** CHANGE *** This defines the number of frames that can be queued for
// asynchronous rendering by this effect.  The value 0 indicates the effect
// runs synchronously.
#define kMaxAsyncFrames		0
// *** END CHANGE ***

// *** CHANGE *** Change if your effect accepts multiple sources.  This value
// is the maximum number of sources this effect operates on.
#define	kMaxSources			2
// *** END CHANGE ***

// --------------------------------------------------------------------------------------
// INTERNAL TYPEDEFS
// --------------------------------------------------------------------------------------

// Structure used to store information about each source
typedef struct {						
	CDSequenceDataSourcePtr	src;
	void					*srcBaseAddr;
	long					srcRowBytes;
} SourceRecord;

// This is the structure used to store information for drawing a single frame of the effect
typedef struct {							
	SourceRecord		sources[kMaxSources];	// inputs

	void				*dstBaseAddr;			// output base address
	long				dstRowBytes;			// output row bytes
	long				height;					// output height
	long				width;					// output width
	short				destDepth;				// output depth
	OSType				dstPixelFormat;			// output pixel format
	
	// *** CHANGE *** Here is where we store values relating to a single frame
	// of the effect.  These are the parameters to the effect, modified by the
	// percentage into the effect which we are.
	
	long				direction;
	short				dimValue;				// dimming value, from 0 to 255
	
	// *** END CHANGE ***
} BlitGlobals;

// global data per instance. This holds data for the entire effect as it runs its course.
typedef struct {
	ComponentInstance	self;		// ourselves
	ComponentInstance	target;		// top of the calling chain
	ComponentInstance	delegate;	// if we can't handle an effect, this one can
	
	BlitGlobals			blitter;	// information for drawing the data
	OSType				**wantedDestinationPixelTypeH;
	
	#if kMaxAsyncFrames > 0
		volatile short	asyncCount;	// number of outstanding frames we have		
	#endif
	
	// parameter/source/dest seed tracking
	long						initialized;
	long						frameNumber;			
	long						virtualDuration;
	long						majorSourceChangeSeed;
	
	TweenGlobals				tweenGlobals;

	// *** CHANGE *** PLACE PARAMETERS FOR YOUR EFFECT HERE
	
	long						direction;	// direction of blit.  0 for dim to bright, 255 for bright to dim
		
	TweenContainerRecord		percentage;
	
	// *** END CHANGE ***
} EffectGlobals;

// --------------------------------------------------------------------------------------
// DISPATCHER
// --------------------------------------------------------------------------------------
/************************************************************************************ 
 *	This is the main dispatcher for our codec. All calls from the codec manager
 *	will come through here, with a unique selector and corresponding parameter block.
 *
 *	This routine must be first in the code segment of the codec component.
 *
 *	We use the normal dispatcher rather than the codec dispatcher as we need to
 *	implement the extra effects routines on top of the codec ones.
 */
/************************************************************************************/
// Begin Dispatch Stuff

// Used by Component Dispatch Helper to call our routines
#define CALLCOMPONENT_BASENAME()		EffectsFrame
#define	CALLCOMPONENT_GLOBALS()			EffectGlobals * storage

// Used by Type's .k.h to create prototypes for our routines
#define	IMAGECODEC_BASENAME()			CALLCOMPONENT_BASENAME()
#define	IMAGECODEC_GLOBALS()			CALLCOMPONENT_GLOBALS()

// Used by SubType's .k.h to create prototypes for our routines
#define	IMAGECODECEFFECT_BASENAME()		CALLCOMPONENT_BASENAME()
#define	IMAGECODECEFFECT_GLOBALS()		CALLCOMPONENT_GLOBALS()

// Other defines for Component Dispatch Helper
#define COMPONENT_DISPATCH_FILE			"EffectDispatch.h"	// describes what to dispatch
#define	GET_DELEGATE_COMPONENT()		(storage->delegate)	// how to find delegate component

#define COMPONENT_UPP_SELECT_ROOT()		ImageCodec			// root for Type's UPP_PREFIX and SELECT_PREFIX		

#include "Components.k.h"				// StdComponent's .k.h
#include "ImageCodec.k.h"				// Type's .k.h
#include "ComponentDispatchHelper.c"	// make our dispatcher and cando

// End Dispatch Stuff
/************************************************************************************/

// --------------------------------------------------------------------------------------
// EFFECT CODE - MULTIPLE BIT-DEPTH AND PIXEL FORMATS
// 		For clarity and reuse sake, the drawing code is being kept separate from the
//		component interface code.
// --------------------------------------------------------------------------------------

// 16BE555 - k16BE555PixelFormat /* 16 bit BE rgb 555 (Mac)*/
#define EffectFilter16 EffectFilter16BE555
#define srcIs16BE555 1
#define dstIs16BE555 1
#include "EffectFilter16.c"
#undef EffectFilter16
#undef srcIs16BE555
#undef dstIs16BE555

#if NON_MAC_PIXEL_FORMATS
// 16LE555 - k16LE555PixelFormat /* 16 bit LE rgb 555 (PC)*/
#define EffectFilter16 EffectFilter16LE555
#define srcIs16LE555 1
#define dstIs16LE555 1
#include "EffectFilter16.c"
#undef EffectFilter16
#undef srcIs16LE555
#undef dstIs16LE555

// 16LE565 - k16LE565PixelFormat  /* 16 bit LE rgb 565*/
#define EffectFilter16 EffectFilter16LE565
#define srcIs16LE565 1
#define dstIs16LE565 1
#include "EffectFilter16.c"
#undef EffectFilter16
#undef srcIs16LE565
#undef dstIs16LE565
#endif

// 32ARGB - k32ARGBPixelFormat /* 32 bit argb    (Mac)*/
#define EffectFilter32 EffectFilter32ARGB
#define srcIs32ARGB 1
#define dstIs32ARGB 1
#include "EffectFilter32.c"
#undef EffectFilter32
#undef srcIs32ARGB
#undef dstIs32ARGB

#if NON_MAC_PIXEL_FORMATS
// 32BGRA - k32BGRAPixelFormat /* 32 bit bgra    (Matrox)*/
#define EffectFilter32 EffectFilter32BGRA
#define srcIs32BGRA 1
#define dstIs32BGRA 1
#include "EffectFilter32.c"
#undef EffectFilter32
#undef srcIs32BGRA
#undef dstIs32BGRA
// 32RGBA - k32RGBAPixelFormat /* 32 bit rgba    */
#define EffectFilter32 EffectFilter32RGBA
#define srcIs32RGBA 1
#define dstIs32RGBA 1
#include "EffectFilter32.c"
#undef EffectFilter32
#undef srcIs32RGBA
#undef dstIs32RGBA
// 32ABGR - k32ABGRPixelFormat /* 32 bit abgr    */
#define EffectFilter32 EffectFilter32ABGR
#define srcIs32ABGR 1
#define dstIs32ABGR 1
#include "EffectFilter32.c"
#undef EffectFilter32
#undef srcIs32ABGR
#undef dstIs32ABGR
#endif

// --------------------------------------------------------------------------------------
// INTERNAL ROUTINES
//		For clarity and reuse sake, the drawing code is being kept separate from the
//		component interface code.
// --------------------------------------------------------------------------------------

// RequestImageFormat
//		If the data is already in the requested height and depth, returns.
//	Otherwise, calls decompression to get it into the format we can handle
// --------------------------------------------------------------------------------------
static OSErr RequestImageFormat(EffectGlobals  *glob, 		// input: globals for rendering
								EffectSourcePtr source, 	// input: source to potentially convert
								short			width,		// input: desired width
								short			height,		// input: desired height
								OSType			pixelFormat)// input: desired pixel format (depth & format)
{
	OSErr					err = noErr;
	CDSequenceDataSourcePtr	sourceData = source->source.image;
	ImageDescriptionHandle	curDesc = (ImageDescriptionHandle)sourceData->dataDescription;
	ImageDescriptionHandle	newDesc = nil;
	ImageDescriptionPtr		dp;

	dp = *curDesc;
	if ((source->effectType == kEffectRawSource) && (((dp->cType == kRawCodecType) && (dp->depth == (short)pixelFormat)) || 
		(dp->cType == pixelFormat)) && (dp->width == width) && (dp->height == height))
	{
		/* already got what we need */
		return noErr;
	}

	// otherwise, call the ICM to convert to desired data format
	newDesc = (ImageDescriptionHandle)NewHandleClear(sizeof(ImageDescription));
	err = MemError();

	if (err == noErr) {
		short pixelSize = QTGetPixelSize(pixelFormat);
		dp = *newDesc;
		
#if !NON_MAC_PIXEL_FORMATS

		if ((pixelFormat >> 24) != 0) {

			// non-Mac format
			dp->cType = pixelFormat;
			dp->depth = pixelSize;

		} else {

			dp->cType = kRawCodecType;
			dp->depth = pixelFormat;
		}
#else
		dp->cType = pixelFormat;
		dp->depth = pixelSize;
#endif
		dp->width  = width;
		dp->height = height;
		dp->clutID = -1;
		
		/* the source is a stacked effect - or one in a format we can't handle. */
		/* pass it off to the Generic Effect to convert */
		/* it to a normal source */
		err = ImageCodecEffectConvertEffectSourceToFormat(glob->target, source, newDesc);

		if (newDesc) {
			DisposeHandle((Handle) newDesc);
		}
	}

	return err;
}

// BlitterPreflight
// --------------------------------------------------------------------------------------
static long BlitterPreflight(BlitGlobals *glob, 	// input: globals for rendering
							 short		 width, 	// input: width of data
							 short		 height, // input: height of data
							 long		 *depth)	// input/output: depth of data
{
	// *** CHANGE ***
	// If your effect handles different bit depths, change this code to return
	// what bit depths you want.
	
	// our blitter can handle 16 and 32 bit deep -- otherwise, we request a change to 16 bit
	switch (*depth) {
		case 16:
		case 32:
			break;
		
		default:
			*depth = 16;
			break;
	}
	// *** END CHANGE ***

	// save away the actual depth we are running at
	glob->width     = width;
	glob->height    = height;	
	glob->destDepth = *depth;
	
	return noErr;
}

// BlitterSetSource
// --------------------------------------------------------------------------------------
static long BlitterSetSource(EffectGlobals	 *glob,			// input: our globals
							 long 			 sourceNumber,	// input: source index to set
							 EffectSourcePtr source)		// input: source value
{
	OSErr	err = noErr;

	if (sourceNumber >= kMaxSources) {
		// too many sources for us to handle
		err = -1;
	} else { 
		// a source we can handle, save it away
		err = RequestImageFormat(glob, source, glob->blitter.width, glob->blitter.height, glob->blitter.dstPixelFormat);
		if (err == noErr) {
			glob->blitter.sources[sourceNumber].src = source->source.image;
		} else {
			glob->blitter.sources[sourceNumber].src = nil;
		}
	}
	
	return err;
}

// BlitterSetDest
// --------------------------------------------------------------------------------------
static long BlitterSetDest(BlitGlobals	*glob, 		// input: our globals
						   PixMap		*dstPixMap, // input: pixels we will draw into
						   Rect			*dstRect)	// input: area of pixels we will draw into
{
	OSErr	result = noErr;
	long	offsetH,offsetV;
	char	*baseAddr;
	OSType	dstPixelFormat;

	dstPixelFormat = GETPIXMAPPIXELFORMAT(dstPixMap);

	glob->dstPixelFormat = dstPixelFormat;

	// *** CHANGE ***
	// If your effect handles different bit depths, change this code adjust
	// the destination baseaddress to be at the beginning of the desired rect.
	offsetH = (dstRect->left - dstPixMap->bounds.left);
	if (dstPixMap->pixelSize == 16) {
		offsetH <<= 1;						/* 1 pixel = 2 bytes */
	} else {
		if (dstPixMap->pixelSize == 32) {
			offsetH <<= 2;					/* 1 pixel = 4 bytes */
		} else {
			result = -1;					/* a data format we can't handle */
		}
	}
	// *** END CHANGE ***

	offsetV = (dstRect->top - dstPixMap->bounds.top) * QTGetPixMapPtrRowBytes(dstPixMap);
	baseAddr = dstPixMap->baseAddr + offsetH + offsetV;

	glob->dstBaseAddr = baseAddr;
	glob->dstRowBytes = QTGetPixMapPtrRowBytes(dstPixMap);

	return result;	
}

// BlitterRenderFrame
// --------------------------------------------------------------------------------------
static long BlitterRenderFrame(BlitGlobals	*glob) // input: our globals
{
	//SInt8 mmuMode;

	// render with proper memory mode
	//mmuMode = true32b;
	//SwapMMUMode(&mmuMode);
	
	// convert data into base/size
	{
		short	i;
		
		for (i = 0; i < kMaxSources; ++i) {
			if (glob->sources[i].src) {
				glob->sources[i].srcBaseAddr = glob->sources[i].src->dataPtr;
				glob->sources[i].srcRowBytes = glob->sources[i].src->dataSize / glob->height;
			}
		}
	}
		
	// *** CHANGE ***
	// If your effect handles different bit depths, write other bit depth
	// routines and call them from here.
	
	// do the actual render
	switch (glob->dstPixelFormat) {
		case k32ARGBPixelFormat:
			EffectFilter32ARGB(glob);
			break;
#if NON_MAC_PIXEL_FORMATS
		case k32ABGRPixelFormat:
			EffectFilter32ABGR(glob);
			break;
		case k32BGRAPixelFormat:	 // we know how to do these pixel formats
			EffectFilter32BGRA(glob);
			break;
		case k32RGBAPixelFormat:
			EffectFilter32RGBA(glob);
			break;
#endif
		case k16BE555PixelFormat:
			EffectFilter16BE555(glob);
			break;
#if NON_MAC_PIXEL_FORMATS
		case k16LE565PixelFormat:
			EffectFilter16LE565(glob);
			break;
		case k16LE555PixelFormat:
			EffectFilter16LE555(glob);
			break;
#endif
	}
	// *** END CHANGE ***

	//SwapMMUMode(&mmuMode);
	
	return noErr;
}

#pragma mark-
// --------------------------------------------------------------------------------------
// COMPONENT ENTRY POINTS - Standard Component Calls
// --------------------------------------------------------------------------------------

// The number of supported pixel formats
#define kNumPixelFormatsSupported 0x20

/* -- This Effect Component uses the Generic Effect Component --
	The Generic Effect Component is an Apple-supplied component
	that makes it easier for developers to create new Effects.
	This component implements many of the "housekeeping" functions
	that all components must perform. In most cases, these default
	implementations are appropriate for your effect, and you simply
	delegate these functions to the generic effect component.
*/

// Component Open Request - Required
// This is called once per instance of our component.  Allocate our storage at
// this point.  If we have any shared storage, we would check here to make sure
// it exists, else create it.
// input/output: our globals
// input: reference to ourself
pascal ComponentResult EffectsFrameOpen(EffectGlobals		*glob,
									    ComponentInstance	self)
{
	ComponentResult	result;
	
	result = noErr;
		
	// first, allocate our local storage
	if ((glob = (EffectGlobals*) NewPtrClear(sizeof(EffectGlobals))) == nil)
	{
		result = MemError();
		goto bail;
	}
	
	SetComponentInstanceStorage(self, (Handle) glob);
	
	// we are ourselves, and the current top of chain is us
	glob->self = self;
	glob->target = self;
	glob->wantedDestinationPixelTypeH = (OSType **)NewHandleClear(sizeof(OSType) * (kNumPixelFormatsSupported + 1));
	
	// open the generic effect, this will handle effects we can't handle ourselves
	result = OpenADefaultComponent(decompressorComponentType, kEffectGenericType, &glob->delegate);
	if (result) goto bail;
		
	// set up the target for the components below us
	ComponentSetTarget(glob->delegate, self);

bail:
	return result;
}

// Component Close Request - Required
// Called each time an instance of our component is going away.  Toss anything we allocated.
// input: our globals
// input: reference to ourself
pascal ComponentResult EffectsFrameClose(EffectGlobals		*glob,
										 ComponentInstance	self)
{
#pragma unused (self)

	if (glob)
	{
		/* 	*** CHANGE *** DISPOSE OF YOUR TWEENERS */
		DisposeTweenRecord(&glob->percentage);

		/* *** END CHANGE *** */

		DisposeTweenGlobals(&glob->tweenGlobals);

		CloseComponent(glob->delegate);

		DisposeHandle((Handle) glob->wantedDestinationPixelTypeH);

		DisposePtr((Ptr) glob);
	}
	
	return noErr;
}

// Component Target Request
// 		Allows another component to "target" you i.e., you call another component whenever
// you would call yourself (as a result of your component being used by another component).
// input: our globals
// input: reference to new top of chain
pascal ComponentResult EffectsFrameTarget(EffectGlobals		*glob,
										  ComponentInstance target)
{
	// remember who is top of chain
	glob->target = target;
	
	// and tell folks below us, too.
	ComponentSetTarget(glob->delegate, target);
	
	return noErr;
}

// Component Version Request - Required
// 		Called to obtain the version of our component.
//	input: our globals
pascal ComponentResult EffectsFrameVersion(EffectGlobals	*glob)
{
#pragma unused (glob)
	
	return kDimmerEffectVersion;
}

#pragma mark-
// --------------------------------------------------------------------------------------
// COMPONENT ENTRY POINTS - Image Codec Calls
// --------------------------------------------------------------------------------------

// ImageCodecGetCodecInfo
//		Your component receives the ImageCodecGetCodecInfo request whenever an application calls the
// 	Image Compression Manager's GetCodecInfo function. Your component should return a formatted compressor
// 	information structure defining its capabilities.
// 	The info is stored as a resource in our component.
//
//		input: our globals
//		output: our codec info
// ----------------------------------------------------------------------------------------
pascal ComponentResult EffectsFrameGetCodecInfo(EffectGlobals	*glob,
												CodecInfo		*info)
{
	OSErr err = noErr;

	if (info == nil)
	{
		err = paramErr;
	}
	else
	{
		CodecInfo **tempCodecInfo;

		err = GetComponentResource((Component) glob->self,
								   codecInfoResourceType,
								   kEffect_cdci_ResID,
								   (Handle *)&tempCodecInfo);
		if (err == noErr)
		{
			*info = **tempCodecInfo;
			DisposeHandle((Handle)tempCodecInfo);
		}
	}

	return err;
}

// ImageCodecGetParameterListHandle
//		Returns a parameter description atom container, as described in the QuickTime Effect Documentaion
// 	section "Supplying Parameter Description Information".
//  http://developer.apple.com/techpubs/quicktime/qtdevdocs/REF/refEffects.28.htm
// 	This function can use the GetComponentResource function to retrieve an 'atms' resource that stores this
//	information if you have provided one in your component.
//
//		input: our globals
// 		output: the parameter description for this effect
// ----------------------------------------------------------------------------------------
pascal ComponentResult EffectsFrameGetParameterListHandle(EffectGlobals	*glob,
														  Handle		*theHandle)
{
	OSErr err = noErr;
	
	err = GetComponentResource((Component)glob->self,
							   FOUR_CHAR_CODE('atms'),
							   kEffect_atms_ResID,
							   theHandle);
	return err;
}

// ImageCodecValidateParameters - Optional
//		If your effect implements this optional function, the Component Manager calls it whenever the user
//	changes a parameter value in the standard parameter dialog box, or attempts to dismiss the dialog by clicking OK.
//	The purpose of this function is to allow your effect to validate its parameters. The current parameter values
//	are passed to the effect in parameters . If all of these values are valid, this function should return noErr.
//	Otherwise, you should return a paramErr and put an explanatory message in the errorString parameter.
//
//		input: our globals
//		input: the current parameter values for this effect
//		input: flags that indicate whether a parameter value has changed or the user is
//			   dismissing the standard parameter dialog box.
//
//				kParameterValidationNoFlags - This value indicates that a standard validation should take place.
//											  This function is being called because the user has changed the
//											  value of a parameter in the standard parameters dialog box. 
//				kParameterValidationFinalValidation - This value indicates that this validation is the final validation
//													  before the standard parameters dialog box is dismissed. This is useful
//													  if you want to perform a single validation of the parameters just after
//													  the user clicks the OK button to dismiss the dialog box. 
//
// 		output: a StringPtr that is contains an error string explaining to the user why
//				the validation has failed.
// ----------------------------------------------------------------------------------------  
pascal ComponentResult EffectsFrameValidateParameters(EffectGlobals *glob,
													  QTAtomContainer parameters,
                     								  QTParameterValidationOptions validationFlags,
                     								  StringPtr errorString)
{
#pragma unused (glob, parameters, validationFlags, errorString)

	return noErr;
}

#pragma mark-
// --------------------------------------------------------------------------------------
// COMPONENT ENTRY POINTS - Effect Codec Calls
// --------------------------------------------------------------------------------------

// ImageCodecEffectSetup
// 		Called once before a sequence of frames are rendered. This gives your effect the chance
// to set up variables that will alter their value during the execution of a sequence of frames.
// Your component should examine the capabilities field of the decompressParams data structure to
// ensure that it can meet the requirements for executing this sequence. In particular, it should
// check the bit depth and pixel format requirements of the sequence. If the sequence requires a
// bit depth and pixel format combination that your component does not support, this function should
// return the nearest supported combination in the decompressParams->capabilities field.
// In this case, QuickTime will redirect all source and destination bitmaps through offscreen graphics
// worlds that have the bit depth and pixel format characteristics that you specify.
// 
//		input: our globals
// 		input: information about the thing being decompressed
//
//	Return in p->capabilities anything in particular your effect requires, such as 
//	limitations on bitdepth.
// --------------------------------------------------------------------------------------
pascal long EffectsFrameEffectSetup(EffectGlobals			*glob,
							 		CodecDecompressParams	*p)
{
	CodecCapabilities 	*capabilities = p->capabilities;
	OSErr				err;
	OSType 				*formats = *glob->wantedDestinationPixelTypeH;
	long				wantedPixelSize = capabilities->wantedPixelSize;
	OSType				dstPixelFormat;
	
	dstPixelFormat = GETPIXMAPPIXELFORMAT(&p->dstPixMap);

	switch (dstPixelFormat)
	{
		case k32ARGBPixelFormat:
		case k16BE555PixelFormat:
#if NON_MAC_PIXEL_FORMATS
		case k32BGRAPixelFormat:	 // we know how to do these pixel formats
		case k32ABGRPixelFormat:
		case k32RGBAPixelFormat:
		case k16LE565PixelFormat:
		case k16LE555PixelFormat:
#endif
			*formats++ = dstPixelFormat;
			break;
		default:					// we prefer 16 over 32
#if NON_MAC_PIXEL_FORMATS
			*formats++ = k16LE555PixelFormat;
			*formats++ = k16LE565PixelFormat;
			*formats++ = k32BGRAPixelFormat;
			*formats++ = k32RGBAPixelFormat;
			*formats++ = k32ABGRPixelFormat;
#endif
			*formats++ = k16BE555PixelFormat;
			*formats++ = k32ARGBPixelFormat;
			break;
	}
	
	// NOTE: 0 marks the end of the format list
	*formats++ = 0;

	/* set up our blitter */
	err = BlitterPreflight(&glob->blitter,
						   (*p->imageDescription)->width,
						   (*p->imageDescription)->height,
						   &wantedPixelSize);
	
	capabilities->wantedPixelSize = 0;

	p->wantedDestinationPixelTypes = glob->wantedDestinationPixelTypeH;

	return err;
}

// ImageCodecEffectBegin
//  	This function is called immediately before your EffectRenderFrame function.
//  The EffectBegin function should ensure that the information it holds about the current source
//	and destination buffers and the parameter values for the effect are valid. If any of these have
//	changed since the last call to EffectBegin, the new values should be read from the appropriate
//	data structures. This function is guaranteed to be called synchronously. In particular, this means
//	you can allocate and move memory, and can call functions that allocate or move memory.
//
// 		input: our globals
// 		input: info about frame being drawn
// 		input: info about this effect frame
// --------------------------------------------------------------------------------------
pascal long EffectsFrameEffectBegin(EffectGlobals			*glob,
							 		CodecDecompressParams	*p,
							 		EffectsFrameParamsPtr	effect)
{
	OSErr			err = noErr;
	EffectSourcePtr	source;
	long			numSources = 0;
	wide			percentage;

	#if kMaxAsyncFrames > 0
		/* we can go async if we don't already have an effect scheduled */
		if (glob->asyncCount < kMaxAsyncFrames)
		{
			glob->asyncCount++;
			effect->doAsync = true;
		}
	#endif

	// dest changed? 
	if (p->conditionFlags & (codecConditionNewClut+codecConditionFirstFrame+codecConditionNewDepth+
							 codecConditionNewDestination+codecConditionNewTransform))
	{
		// re-scan the sources
		glob->majorSourceChangeSeed = 0;

		// re-read the parameters
		glob->frameNumber = 0;

		err = BlitterSetDest(&glob->blitter, &p->dstPixMap, &p->dstRect);
		if (err != noErr) goto bail;
	}

	// new sources?  make note of them!
	if (p->majorSourceChangeSeed != glob->majorSourceChangeSeed)
	{
		// grab start of input chain for this effect
		source = effect->source;

		/* we can play with up to kMaxSources sources, so go get them */
		while (source != nil && numSources < kMaxSources)
		{		
			/* now give that source to our blitter */
			err = BlitterSetSource(glob, numSources, source);
			if (err != noErr) goto bail;
				
			source = source->next;
			++numSources;
		}
		
		glob->majorSourceChangeSeed = p->majorSourceChangeSeed;
	}
	
	/* if this is a new frame, or the same frame with a new length, get rid of our old parameters & tweeners */	
	if ((effect->frameTime.frameNumber != glob->frameNumber) || (effect->frameTime.virtualDuration != glob->virtualDuration) )
	{
		/* 	*** CHANGE *** DISPOSE OF YOUR TWEENERS */
		DisposeTweenRecord(&glob->percentage);
		
		/* *** END CHANGE *** */

		DisposeTweenGlobals(&glob->tweenGlobals);

		glob->initialized = false;
		glob->frameNumber = effect->frameTime.frameNumber;
		glob->virtualDuration = effect->frameTime.virtualDuration;
	}

	// Read in effect parameters
	if (!glob->initialized)
	{
		Ptr 					data = p->data;
		OSErr					err;
		long					index = 1;
		
		err = InitializeTweenGlobals(&glob->tweenGlobals, p);
		if (err!=noErr)
			goto bail;
			
		/* 	*** CHANGE *** TIME TO READ IN PARAMETERS TO YOUR EFFECT:
		
			You can read any number of atoms you wish from the container.  For example, you might have a 
			'star' (start) and 'end ' (end) value.  They might be expressed as percentages, numbers, or other values.
			Or, you might have multiple atoms of type 'para' (param) which would be read in by calling
			QTFindChildByIndex with various index values.  If you want to know how many parameters of a given
			type there are, call QTCountChildrenOfType.  These parameters are specific to your particular effect,
			and will need to be placed there by whoever is authoring the title.
			
			If you require parameters, and they aren't there, return an error.  If you can default the values
			if they are missing, do so and continue.  In general, I'd recommend having a default case unless
			you really are unable to implement it for a technical reason.
			
			These parameters are those that apply to the effect itself.  Later, we'll translate some of these parameters
			into how they relate to *where* in the effect we are.  For example, an effect that runs from a starting
			percentage of 10 to an ending percentage of 90 will have 10 and 90 as parameters here.
		*/
		
		/* make our tweener, return if we already have it */
		err = CreateTweenRecord(&glob->tweenGlobals, &glob->percentage, 
								kParameterUsagePercent, 1, sizeof(Fixed),
								kTweenTypeFixed, (void*)0, (void*)fixed1,  
								effect->frameTime.virtualDuration);
		if (err != noErr) goto bail;

		/* *** END CHANGE *** */

		glob->initialized = true;
	}
	
	/* determine the amount we are along the tween via the current time - start time */
	percentage = effect->frameTime.value;
	CompSub(&effect->frameTime.virtualStartTime, &percentage);
	
	/* 	Tween our parameters and get the current value for this frame, prepare to render
		it when the RenderFrame happens. */
	if (err == noErr)
	{
		Fixed	thePercentage;
		
		/* ***** CHANGE TO TWEEN YOUR EFFECTS PARAMETERS */
		
		if (glob->percentage.tween)
			QTDoTween(glob->percentage.tween, percentage.lo, glob->percentage.tweenData, nil, nil, nil );
		
		thePercentage = **(Fixed**)(glob->percentage.tweenData);
		// If we are before the half-way point of this transition, we should
		// be fading the first source to black
		if (thePercentage < fixed1 / 2)
		{
			(glob->blitter).direction = 1;
			(glob->blitter).dimValue = Fix2Long(FixMul(Long2Fix(255*2), thePercentage));
		}
		// Otherwise, we are fading up onto the new source
		else
		{
			(glob->blitter).direction = 0;
			(glob->blitter).dimValue = Fix2Long(FixMul(Long2Fix(255*2), thePercentage)) - 255;
		}

		/* ***** END CHANGE */
	}

	if (glob->tweenGlobals.atLeastOneTweener)
	{
		// this effect runs constantly
		p->needUpdateOnTimeChange = true;
		p->needUpdateOnSourceChange = false;
	}
	else
	{
		// this effect only needs to run when the sources actually change
		p->needUpdateOnTimeChange = false;
		p->needUpdateOnSourceChange = true;
	}
	
		
// EXCEPTION HANDLING
bail:
	#if kMaxAsyncFrames > 0
		// if we didn't queue the frame, then remove it from used list
		if (err != noErr)
			glob->asyncCount--;
	#endif
	
	return err;
}

// ImageCodecEffectRenderFrame
//		Called to render a frame. Because this function can be called asynchronously, it is not safe
//	to perform operations that may move memory during this call. This function contains the implementation
//	of your effect.
//
//		input: our global
// 		input: effect frame to be rendered
// --------------------------------------------------------------------------------------
pascal long EffectsFrameEffectRenderFrame(EffectGlobals 		*glob,
										  EffectsFrameParamsPtr	effect)
{
#pragma unused (effect)

	/* render the frame */
	BlitterRenderFrame(&glob->blitter);
	
	#if kMaxAsyncFrames > 0
		glob->asyncCount--;
	#endif

	return noErr;
	
}

// ImageCodecEffectCancel
// 		Cancel the rendering of a sequence of frames before the last frame has been rendered. If your
//	component supports asynchronous operation, this function can be called while a frame is being rendered.
//	If your component is running synchronously, it should simply return noErr - no further calls to your
//	EffectRenderFrame function will be made for this sequence. If your component is running asynchronously,
//	this function should dequeue all outstanding render frame requests, then return noErr.
//
// 		input: our globals
// 		input: effect frame to be canceled
// --------------------------------------------------------------------------------------
pascal long EffectsFrameEffectCancel(EffectGlobals			*glob,
									EffectsFrameParamsPtr	effect)
{
#pragma unused (effect)
#pragma unused (glob)

	#if kMaxAsyncFrames > 0
		glob->asyncCount--;
	#endif

	return noErr;	
}

// ImageCodecEffectGetSpeed
//		Returns the approximate number of frames per second that your effect is capable of transforming.
//	This function should return a Fixed value in pFPS , which represents the rendering speed in frames-per-second
//	of the effect. If your effect can render in real time, it should return a value of effectIsRealtime.
//	Otherwise, you should return an estimate of the number of frames your effect can render per second.
//	Because rendering speeds are hardware-dependent, effect authors can choose to measure actual rendering
//	speeds in this function. Alternatively, effect authors can choose to return a single value for all hardware
//	configurations, estimating the value for a reference hardware platform.
//	Apple recommends that the values returned are rounded down to the nearest common frames-per-second value,
//	such as 15, 24 or 30.
//
//		input: our globals
//		input: the current parameter values for this effect
// 		output: a Fixed value that will contain the rendering speed of this effect on exit
// ----------------------------------------------------------------------------------------
pascal long EffectsFrameEffectGetSpeed(EffectGlobals * glob,
									   QTAtomContainer parameters,
									   Fixed *pFPS)
{
#pragma unused (glob, parameters)

	if (pFPS)
		*pFPS = Long2Fix(30);
		
	return noErr;
}