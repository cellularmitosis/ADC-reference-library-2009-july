/*
	File:		softVdig.c

	Contains:	Software video digitzer routines

	Written by:	Peter Hoddie (mostly) and Casey King and Gary Woodcock
	
				Refer to develop Issue 14, "Video Digitizing Under QuickTime",
				for details on this code.
				
				This code requires QuickTime 1.5.

	Copyright:	© 1993 by Apple Computer, Inc.

*/

//-----------------------------------------------------------------------
// includes

#include "softVdig.h"
#include <FixMath.h>
#include <MacErrors.h>
#include <Packages.h>
#include <Fonts.h>
#include <MacMemory.h>
#include <ToolUtils.h>
#include <NumberFormatting.h>

//-----------------------------------------------------------------------

#if TARGET_OS_WIN32
	#pragma warning (disable: 4761)		// ignore integral size mismatch in argument
#endif

#if defined(DEBUG_IT) || !TARGET_OS_MAC
// Use this declaration when we're running linked (for debugging)
pascal ComponentResult softVdig (ComponentParameters *params, Handle storage)

#else

// Use this declaration when we're a standalone component
pascal ComponentResult main(ComponentParameters *params, Handle storage)

#endif DEBUG_IT

{
	// This routine is the main dispatcher for the softVdig
	
	ComponentRoutineUPP	vdigProc = 0;
	ComponentResult err = 0;

	if (params->what < 0) {
		switch(params->what) {
			case kComponentOpenSelect:				vdigProc = (ComponentRoutineUPP)vdigOpen; break;
			case kComponentCloseSelect:				vdigProc = (ComponentRoutineUPP)vdigClose; break;
			case kComponentCanDoSelect:				vdigProc = (ComponentRoutineUPP)vdigCanDo; break;
			case kComponentVersionSelect:			vdigProc = (ComponentRoutineUPP)vdigVersion; break;
		}
	}
	else {
		switch (params->what) {
			case kVDGetMaxSrcRectSelect:			vdigProc = (ComponentRoutineUPP)vdigGetMaxSrcRect; break;
			case kVDGetActiveSrcRectSelect:			vdigProc = (ComponentRoutineUPP)vdigGetActiveSrcRect; break;
			case kVDSetDigitizerRectSelect:			vdigProc = (ComponentRoutineUPP)vdigSetDigitizerRect; break;
			case kVDGetDigitizerRectSelect:			vdigProc = (ComponentRoutineUPP)vdigGetDigitizerRect; break;
			case kVDUseThisCLUTSelect:				vdigProc = (ComponentRoutineUPP)vdigUseThisCLUT; break;
			case kVDGrabOneFrameSelect:				vdigProc = (ComponentRoutineUPP)vdigGrabOneFrame; break;
			case kVDGetMaxAuxBufferSelect:			vdigProc = (ComponentRoutineUPP)vdigGetMaxAuxBuffer; break;
			case kVDGetDigitizerInfoSelect:			vdigProc = (ComponentRoutineUPP)vdigGetDigitizerInfo; break;
			case kVDGetCurrentFlagsSelect :			vdigProc = (ComponentRoutineUPP)vdigGetCurrentFlags; break;
			case kVDSetPlayThruDestinationSelect:	vdigProc = (ComponentRoutineUPP)vdigSetPlayThruDestination; break;
			case kVDSetBrightnessSelect :			vdigProc = (ComponentRoutineUPP)vdigSetBrightness; break;
			case kVDGetBrightnessSelect :			vdigProc = (ComponentRoutineUPP)vdigGetBrightness; break;	
			case kVDSetContrastSelect :				vdigProc = (ComponentRoutineUPP)vdigSetContrast; break;			
			case kVDSetHueSelect :					vdigProc = (ComponentRoutineUPP)vdigSetHue; break;
			case kVDSetSaturationSelect :			vdigProc = (ComponentRoutineUPP)vdigSetSaturation; break;
			case kVDSetSharpnessSelect :			vdigProc = (ComponentRoutineUPP)vdigSetSharpness; break;
			case kVDGetContrastSelect :				vdigProc = (ComponentRoutineUPP)vdigGetContrast; break;
			case kVDGetHueSelect :					vdigProc = (ComponentRoutineUPP)vdigGetHue; break;
			case kVDGetSaturationSelect :			vdigProc = (ComponentRoutineUPP)vdigGetSaturation; break;
			case kVDGetSharpnessSelect :			vdigProc = (ComponentRoutineUPP)vdigGetSharpness; break;
			case kVDSetBlackLevelValueSelect :		vdigProc = (ComponentRoutineUPP)vdigSetBlackLevel; break;
			case kVDGetBlackLevelValueSelect :		vdigProc = (ComponentRoutineUPP)vdigGetBlackLevel; break;
			case kVDSetWhiteLevelValueSelect :		vdigProc = (ComponentRoutineUPP)vdigSetWhiteLevel; break;
			case kVDGetWhiteLevelValueSelect :		vdigProc = (ComponentRoutineUPP)vdigGetWhiteLevel; break;
			case kVDGetVideoDefaultsSelect :		vdigProc = (ComponentRoutineUPP)vdigGetVideoDefaults; break;	
			case kVDSetPlayThruOnOffSelect:			vdigProc = (ComponentRoutineUPP)vdigSetPlayThruOnOff; break;
			case kVDPreflightDestinationSelect:		vdigProc = (ComponentRoutineUPP)vdigPreflightDestination; break;
			case kVDSetupBuffersSelect:				vdigProc = (ComponentRoutineUPP)vdigSetupBuffers; break;
			case kVDGrabOneFrameAsyncSelect:		vdigProc = (ComponentRoutineUPP)vdigGrabOneFrameAsync; break;
			case kVDDoneSelect:						vdigProc = (ComponentRoutineUPP)vdigDone; break;
			case kVDGetNumberOfInputsSelect:		vdigProc = (ComponentRoutineUPP)vdigGetNumberOfInputs; break;
			case kVDGetInputFormatSelect:			vdigProc = (ComponentRoutineUPP)vdigGetInputFormat; break;
			case kVDSetInputSelect:					vdigProc = (ComponentRoutineUPP)vdigSetInput; break;
			case kVDGetInputSelect:					vdigProc = (ComponentRoutineUPP)vdigGetInput; break;
			case kVDSetCompressionSelect:			vdigProc = (ComponentRoutineUPP)vdigSetCompression; break;
			case kVDCompressOneFrameAsyncSelect:	vdigProc = (ComponentRoutineUPP)vdigCompressOneFrameAsync; break;
			case kVDCompressDoneSelect:				vdigProc = (ComponentRoutineUPP)vdigCompressDone; break;
			case kVDReleaseCompressBufferSelect:	vdigProc = (ComponentRoutineUPP)vdigReleaseCompressBuffer; break;
			case kVDGetImageDescriptionSelect:		vdigProc = (ComponentRoutineUPP)vdigGetImageDescription; break;
			case kVDResetCompressSequenceSelect:	vdigProc = (ComponentRoutineUPP)vdigResetCompressSequence; break;
			case kVDSetCompressionOnOffSelect:		vdigProc = (ComponentRoutineUPP)vdigSetCompressionOnOff; break;
			case kVDGetCompressionTypesSelect:		vdigProc = (ComponentRoutineUPP)vdigGetCompressionTypes; break;
			case kVDSetTimeBaseSelect:				vdigProc = (ComponentRoutineUPP)vdigSetTimeBase; break;
			case kVDSetFrameRateSelect:				vdigProc = (ComponentRoutineUPP)vdigSetFrameRate; break;
			case kVDGetDMADepthsSelect:				vdigProc = (ComponentRoutineUPP)vdigGetDMADepths; break;
			case kVDGetPreferredTimeScaleSelect:	vdigProc = (ComponentRoutineUPP)vdigGetPreferredTimeScale; break;

			default:
						err = digiUnimpErr;
						break;
		}
	}

	if (vdigProc) {
		err = CallComponentFunctionWithStorage((Handle)storage, params, vdigProc);
		
	// kck - for debug only to catch calls that fail
	if (((params->what != kVDSetFrameRateSelect) && 
			(params->what != kVDCompressDoneSelect) && 
			(params->what != kVDGetMaxAuxBufferSelect) &&
			(params->what != kVDUseThisCLUTSelect) && 
			(params->what != kVDSetCompressionOnOffSelect)
			) && err ) {
			return err;
		}
		return err;
	}

	return err;
}

#ifdef DEBUG_IT
Component RegisterSoftVdig(void)
{
	ComponentDescription foo;
	Component fooey;
	Handle name;

  	foo.componentType = videoDigitizerComponentType;
  	foo.componentSubType = 'soft';
  	foo.componentManufacturer = 'jph ';
  	foo.componentFlags = 0L;
  	foo.componentFlagsMask = 0L;

	PtrToHand ((Ptr)"\psoftVdig (Linked)", &name, 18);
	fooey = RegisterComponent (&foo, (void *)softVdig, 0, name, 0, 0);
	DisposHandle (name);
	SetDefaultComponent (fooey, defaultComponentAnyFlagsAnyManufacturerAnySubType);

	return fooey;
}
#endif DEBUG_IT

pascal ComponentResult vdigOpen(vdigGlobals storage, ComponentInstance self)
{
	ComponentResult err;

	if (CountComponentInstances((Component)self) > (short)gMaxSoftVdigCount)
		return(-1);

	storage = (vdigGlobals)NewHandleClear(sizeof(vdigGlobalsRecord));
	if (err = MemError()) goto bail;
	(**storage).self = self;
	SetComponentInstanceStorage(self, (Handle)storage);
#if TARGET_OS_MAC
	SetComponentInstanceA5(self, *(long *)0x904);
#endif

	// Initialize some of the global storage members
	err = vdigGetMaxSrcRect(storage, ntscIn, &(**storage).maxSrcRect);
	(**storage).digiRect = (**storage).maxSrcRect;
	(**storage).pendingAsyncBuffer = -1;
	err = vdigGetVideoDefaults(storage,
							 &(**storage).blackLevel, &(**storage).whiteLevel,
							&(**storage).brightness, &(**storage).hue, &(**storage).saturation,
							&(**storage).contrast, &(**storage).sharpness);

// Initialize two additional softvdig configurable items
	(**storage).gAttachedGDevice = 0;
	MacSetRect(&(**storage).gAuxBufferRect,0,0,300,300);


bail:
	return(err);
}

pascal ComponentResult vdigClose(vdigGlobals storage, ComponentInstance self)
{
	if (storage) {
		if ((**storage).tempPort)
			DisposeGWorld((**storage).tempPort);

		if ((**storage).auxBuffer)
			DisposeGWorld((**storage).auxBuffer);

		if ((**storage).bufferList)
			DisposeHandle((Handle)(**storage).bufferList);

		if ((**storage).clipRgn)
			DisposeRgn((**storage).clipRgn);

		tossCompressStuff(storage);

		DisposeHandle((Handle)storage);
	}

	return noErr;
}

pascal ComponentResult vdigCanDo(vdigGlobals storage, short ftnNumber)
{
#define kvdigSelectors kVDSetPreferredPacketSizeSelect
	return (ftnNumber <= kvdigSelectors) && (ftnNumber >= kComponentVersionSelect);
}

pascal ComponentResult vdigVersion(vdigGlobals storage)
{
	return (vdigInterfaceRev<<16) | 1;
}

pascal VideoDigitizerError vdigGetMaxSrcRect(vdigGlobals storage, short inputStd, Rect *maxSrcRect)
{
long	error = noErr;

	if (inputStd == ntscIn)			// softvdig supports only NTSC
		MacSetRect(maxSrcRect, 0, 0, kMaxHorNTSCIn, kMaxVerNTSCIn + kVerBlank);
	else
		error = paramErr;
	if (!error) (**storage).maxSrcRect = *maxSrcRect;

	return error;
}

pascal VideoDigitizerError vdigGetActiveSrcRect(vdigGlobals storage, short inputStd, Rect *activeSrcRect)
{
	*activeSrcRect = (**storage).maxSrcRect;

	return noErr;
}

pascal VideoDigitizerError vdigSetDigitizerRect(vdigGlobals storage, Rect *digiRect)
{
	Rect tempR;

	// can't be empty
	if (!digiRect || EmptyRect(digiRect))
		return(paramErr);

	// they must intersect
	if (!SectRect(digiRect, &(**storage).maxSrcRect, &tempR))
		return(paramErr);

	// completely...
	if (!MacEqualRect(digiRect, &tempR))
		return(paramErr);

	(**storage).digiRect = *digiRect;

	return noErr;
}

pascal VideoDigitizerError vdigGetDigitizerRect(vdigGlobals storage, Rect *digiRect)
{
	*digiRect = (**storage).digiRect;

	return noErr;
}

pascal VideoDigitizerError vdigUseThisCLUT(vdigGlobals storage, CTabHandle colorTableHandle)
{
	return digiUnimpErr;
}

void makeOffscreen(vdigGlobals storage)
{
	OSErr err;
	GWorldPtr tempPort;
	Rect r;

	tossOffscreen(storage);

	if (!(**storage).playThruPixMap) return;

	MacSetRect(&r, 0, 0, 4, 4);
	err = NewGWorld(&tempPort, (**(**storage).playThruPixMap).pixelSize, &r, 0, 0, 0);
	if (err) return;

	(**storage).tempPort = tempPort;
	MacSetRectRgn(tempPort->visRgn, -32000, -32000, 32000, 32000);
}

void tossOffscreen(vdigGlobals storage)
{
	if ((**storage).tempPort) {
		DisposeGWorld((**storage).tempPort);
		(**storage).tempPort = 0;
	}
}

void drawVideoFrame(vdigGlobals storage, Point where, PixMapHandle destPixMap)
{
	Rect destRect;
	CGrafPtr savePort;
	GDHandle saveGD;
	PixMapHandle savePortPix;
	Rect pictRect;
	Rect clipRect;
	Str255 tempStr;
	Point offsetClip;
	RgnHandle clipRgn;
	FontInfo	fontInfo;
	short		stringWidth, textSize;

	if (!(**storage).tempPort && !(**storage).compressGW) {
		makeOffscreen(storage);
		if (!(**storage).tempPort)
			return;
	}

	destRect = (**storage).digiRect;
	TransformRect(&(**storage).matrix, &destRect, 0);
	MacOffsetRect(&destRect, -destRect.left + where.h, -destRect.top + where.v);

	GetGWorld(&savePort, &saveGD);
	if ((**storage).compressGW)
		SetGWorld((**storage).compressGW, 0);
	else {
		SetGWorld((**storage).tempPort, 0);
		savePortPix = (**storage).tempPort->portPixMap;
		SetPortPix(destPixMap);
	}

	clipRect = (**storage).digiRect;
	ClipRect(&clipRect);

	if (clipRgn = (**storage).clipRgn) {
		offsetClip.h = where.h - (**storage).clipOrigin.h;
		offsetClip.v = where.v - (**storage).clipOrigin.v;
		MacOffsetRgn(clipRgn, offsetClip.h, offsetClip.v);
		SetClip(clipRgn);
	}
	else {
		MacSetRect(&clipRect, -32000, -32000, 32000, 32000);
		ClipRect(&clipRect);
	}

	clipRect = (**storage).maxSrcRect;
	MapRect(&clipRect, &(**storage).digiRect, &destRect);

	pictRect = clipRect;
	EraseRect(&pictRect);
	MacFrameRect(&pictRect);
	MacInsetRect(&pictRect, 10, 10);
	MacFrameRect(&pictRect);
	MacInsetRect(&pictRect, 10, 10);
	TextFont(kFontIDHelvetica);
	textSize = (pictRect.bottom - pictRect.top) >> 1;
	TextSize(textSize);
	NumToString((**storage).frameNumber, tempStr);
	stringWidth = StringWidth(tempStr);
	while (stringWidth > (pictRect.right - pictRect.left))
	{
 		textSize /= 2;
		TextSize(textSize);
		NumToString((**storage).frameNumber, tempStr);
		stringWidth = StringWidth(tempStr);
	}
	GetFontInfo(&fontInfo);
	MoveTo((pictRect.left + pictRect.right)/2 - stringWidth/2,
		(pictRect.top + pictRect.bottom)/2 - (fontInfo.ascent+fontInfo.descent)/2 + fontInfo.ascent);
	DrawString(tempStr);

	(**storage).frameNumber++;

	if (clipRgn) {
		MacOffsetRgn(clipRgn, -offsetClip.h, -offsetClip.v);
		MacSetRect(&clipRect, -32000, -32000, 32000, 32000);
		ClipRect(&clipRect);
	}

	if (!(**storage).compressGW)
		SetPortPix(savePortPix);
	SetGWorld(savePort, saveGD);
}

pascal VideoDigitizerError vdigGrabOneFrame(vdigGlobals storage)
{
	if (!(**storage).playThruPixMap)
		return badCallOrderErr;

	drawVideoFrame(storage, *(Point *)&((**storage).playThruRect), (**storage).playThruPixMap);

	return noErr;
}
pascal VideoDigitizerError vdigSetContrast(vdigGlobals storage, unsigned short *contrast)
{
	(**storage).contrast = *contrast;
	return noErr;
}

pascal VideoDigitizerError vdigGetContrast(vdigGlobals storage, unsigned short *contrast)
{
	*contrast = (**storage).contrast;
	return noErr;
}

pascal VideoDigitizerError vdigSetHue(vdigGlobals storage, unsigned short *hue) {
	(**storage).hue = *hue;
	return noErr;
}
pascal VideoDigitizerError vdigGetHue(vdigGlobals storage, unsigned short *hue)
{
	*hue = (**storage).hue;
	return noErr;
}

pascal VideoDigitizerError vdigSetBrightness(vdigGlobals storage, unsigned short *brightness)
{
	(**storage).brightness = *brightness;
	return noErr;
}
pascal VideoDigitizerError vdigGetBrightness(vdigGlobals storage, unsigned short *brightness)
{
	*brightness = (**storage).brightness;
	return noErr;
}

pascal VideoDigitizerError vdigSetSaturation(vdigGlobals storage, unsigned short *saturation)
{
	(**storage).saturation = *saturation;
	return noErr;
}

pascal VideoDigitizerError vdigGetSaturation(vdigGlobals storage, unsigned short *saturation)
{
	*saturation = (**storage).saturation;
	return noErr;
}
pascal VideoDigitizerError vdigSetSharpness(vdigGlobals storage, unsigned short *sharpness)
{
	(**storage).sharpness = *sharpness;
	return noErr;
}

pascal VideoDigitizerError vdigGetSharpness(vdigGlobals storage, unsigned short *sharpness)
{
	*sharpness = (**storage).sharpness;
	return noErr;
}

pascal VideoDigitizerError vdigSetBlackLevel(vdigGlobals storage, unsigned short *blackLevel)
{
	(**storage).blackLevel = *blackLevel;
	return noErr;
}
pascal VideoDigitizerError vdigGetBlackLevel(vdigGlobals storage, unsigned short *blackLevel)
{
	*blackLevel = (**storage).blackLevel;
	return noErr;
}
pascal VideoDigitizerError vdigSetWhiteLevel(vdigGlobals storage, unsigned short *whiteLevel)
{
	(**storage).whiteLevel = *whiteLevel;
	return noErr;
}
pascal VideoDigitizerError vdigGetWhiteLevel(vdigGlobals storage, unsigned short *whiteLevel){
	*whiteLevel = (**storage).whiteLevel;
	return noErr;
}
pascal VideoDigitizerError vdigGetVideoDefaults(vdigGlobals storage,
							unsigned short *blackLevel, unsigned short *whiteLevel,
							unsigned short *brightness, unsigned short *hue, unsigned short *saturation,
							unsigned short *contrast, unsigned short *sharpness)
{
	(**storage).blackLevel 	= 0;
	(**storage).whiteLevel 	= 60000;
	(**storage).brightness 	= 20000;
	(**storage).hue 		= 30000;
	(**storage).saturation 	= 40000;
	(**storage).contrast 	= 50000;
	(**storage).sharpness 	= 60000;
	return noErr;
}

pascal VideoDigitizerError vdigGetMaxAuxBuffer(vdigGlobals storage, PixMapHandle *pm, Rect *r)
{
	OSErr 	err = noErr;

	if (!gHasAuxBuffer)
		return digiUnimpErr;

	if (!(**storage).auxBuffer) {
		GWorldPtr tempGW;

		err = NewGWorld(&tempGW, gAuxDepth, &(**storage).gAuxBufferRect, 0, 0, 0);
		if (err) goto bail;

		LockPixels(tempGW->portPixMap);

		(**storage).auxBuffer = tempGW;
	}

	if (pm) *pm = (**storage).auxBuffer->portPixMap;
	if (r) *r = (**(**storage).auxBuffer->portPixMap).bounds;

bail:
	return err;
}

pascal VideoDigitizerError vdigGetDigitizerInfo(vdigGlobals storage, DigitizerInfo *info)
{
	
	info->vdigType = gCanClip ? vdTypeMask : vdTypeBasic;
	info->inputCapabilityFlags = digiInDoesNTSC | digiInDoesComposite | digiInDoesColor;
	info->outputCapabilityFlags = gDoesDepths |
								(gCanScale ? digiOutDoesStretch | digiOutDoesShrink : digiOutDoesQuarter | digiOutDoesSixteenth) | 
								(gCanClip ? digiOutDoesMask : 0) |
								(gCanDMA ? digiOutDoesHW_DMA : 0) |
								(gDoesPlaythru ? digiOutDoesHWPlayThru : 0) |
								(gCanAsync ? digiOutDoesAsyncGrabs : 0) |
								(gDoesCompress ? digiOutDoesCompress : 0) |
								((gDoesCompress && gOnlyDoesCompress) ? digiOutDoesCompressOnly : 0)
								;

	info->inputCurrentFlags = info->inputCapabilityFlags;
	info->inputCurrentFlags |= digiInSignalLock;
	info->outputCurrentFlags = 0;
	if ((**storage).gAttachedGDevice)
		info->outputCurrentFlags = (**(**(**storage).gAttachedGDevice).gdPMap).pixelSize;
	else {

	}

	info->slot = 0;								// we have no slot
	info->gdh = (**storage).gAttachedGDevice;
	info->maskgdh = 0;							
	info->minDestHeight = gMinHeight;
	info->minDestWidth = gMinWidth;
	info->maxDestHeight = gMaxHeight;
	info->maxDestWidth = gMaxWidth;
	info->blendLevels = 0;
	info->reserved = 0;
	
	(**storage).inputCurrentFlags  = info->inputCurrentFlags;
	(**storage).outputCurrentFlags = info->outputCurrentFlags;
	
	return noErr;
}
pascal VideoDigitizerError	vdigGetCurrentFlags(vdigGlobals storage, long *inputCurrentFlag, long *outputCurrentFlag) 
{
	*inputCurrentFlag = (**storage).inputCurrentFlags;
	*outputCurrentFlag = (**storage).outputCurrentFlags;
	return noErr;
}

pascal VideoDigitizerError vdigSetPlayThruOnOff(vdigGlobals storage, short state)
{
	if (!gDoesPlaythru)
		return digiUnimpErr;

	(**storage).playThruOn = (state != 0);
	return noErr;
}

pascal VideoDigitizerError vdigSetPlayThruDestination(vdigGlobals storage, PixMapHandle dest, Rect *dr, MatrixRecord *m, RgnHandle mask)
{
	OSErr err;
	short width, height;
	Rect destRect;

	if (!dest || (!dr && !m))
		return paramErr;

	if (dr)
		destRect = *dr;
	else {
		if (GetMatrixType(m) > scaleTranslateMatrixType)
			return matrixErr;
		destRect = (**storage).digiRect;
		TransformRect(m, &destRect, nil);
	}

	// make sure the rectangle is ok
	if (EmptyRect(&destRect))
		return paramErr;

	if (mask && !gCanClip)
		return paramErr;

	if (!validatePixMap(storage, dest))
		return paramErr;

	if ((**storage).clipRgn) {
		DisposeRgn((**storage).clipRgn);
		(**storage).clipRgn = 0;
	}

	if (mask) {
		HandToHand((Handle *)&mask);
		if (err = MemError()) return err;
	}
	(**storage).clipRgn = mask;
	(**storage).clipOrigin = *(Point *)&destRect;

	{
	Rect tempRect;

	tempRect = destRect;
	MapRect(&tempRect, &(**storage).digiRect, &(**storage).maxSrcRect);			// convert to full frame
	RectMatrix(&(**storage).matrix, &(**storage).maxSrcRect, &tempRect);		// make a matrix

	tempRect = (**storage).maxSrcRect;
	TransformRect(&(**storage).matrix, &tempRect, 0);
	width = tempRect.right - tempRect.left;
	height = tempRect.bottom - tempRect.top;
	}

	if (width < gMinWidth || width > gMaxWidth || height < gMinHeight || height > gMaxHeight)
		return paramErr;

#if 0
	if (width != gMaxWidth || height != gMaxHeight)
		return paramErr;
#endif

	if (!gCanScale) {
		short i = 3;
		short tempWidth = gMaxWidth;
		short tempHeight = gMaxHeight;
		Boolean ok;

		while (i--) {
			ok = (width == tempWidth) && (height == tempHeight);
			if (ok) break;
			tempWidth >>= 1;
			tempHeight >>= 1;
		}
		if (!ok)
			return paramErr;
	}

	if ((**storage).tempPort) {
		if ((**dest).pixelSize != (**(**storage).tempPort->portPixMap).pixelSize)
			tossOffscreen(storage);
	}

	(**storage).playThruRect = destRect;
	(**storage).playThruPixMap = dest;

	return noErr;
}

// better checks for pixmap baseAddr matching could be done
Boolean validatePixMap(vdigGlobals storage, PixMapHandle p)
{
	if ( !((**p).pixelSize & gDoesDepths) ) {
		if (gCanDMA && ((**p).pixelSize & gDMADepths))
			;
		else
			return false;
	}

	if (gCanDMA)						// any other place is OK
		return true;

	if ((**storage).gAttachedGDevice) {
		// see if pixels are on the device
		if ((**p).baseAddr == (**(**(**storage).gAttachedGDevice).gdPMap).baseAddr)
			return true;
	}

	if ((**storage).auxBuffer) {
		if ((**p).baseAddr == (**(**storage).auxBuffer->portPixMap).baseAddr)
			return true;
	}

	if (!(**storage).gAttachedGDevice) return true;

	return false;
}

pascal VideoDigitizerError vdigPreflightDestination(vdigGlobals storage, Rect *digitizerRect, PixMap **dest, Rect *destRect, MatrixRecord *m)
{
	//¥¥  real code is required here in your vdig!!

	return noErr;
}

pascal VideoDigitizerError vdigSetupBuffers(vdigGlobals storage, VdigBufferRecListHandle bufferList)
{
	OSErr err;
	MatrixRecord matrix;
	short i;
	RgnHandle clipRgn;

	if (!bufferList)
		return paramErr;

	if (!(**bufferList).count)
		return paramErr;

	if ((**storage).bufferList) {
		DisposeHandle((Handle)(**storage).bufferList);
		(**storage).bufferList = 0;
	}

	if ((**storage).clipRgn) {
		DisposeRgn((**storage).clipRgn);
		(**storage).clipRgn = 0;
	}

	if (!gCanClip && (**bufferList).mask)
		return paramErr;

	if ((**bufferList).matrix)
		matrix = *(**bufferList).matrix;

	// make a local copy of the buffer list
	HandToHand((Handle *)&bufferList);
	if (err = MemError()) return err;

	if (clipRgn = (**bufferList).mask) {
		HandToHand((Handle *)&clipRgn);
		if (err = MemError()) return err;
		(**storage).clipOrigin = (**bufferList).list[0].location;
	}

	(**storage).bufferList = bufferList;
	(**storage).clipRgn = clipRgn;
	(**storage).pendingAsyncBuffer = -1;
	(**storage).matrix = matrix;

	for (i=0; i < (**bufferList).count; i++) {	
		if (!validatePixMap(storage, (**bufferList).list[i].dest))
			return paramErr;
		// should check the point here too
	}

	return noErr;
}

pascal VideoDigitizerError vdigGrabOneFrameAsync(vdigGlobals storage, short buffer)
{
	VdigBufferRecListHandle bufferList;

	if (!gCanAsync)
		return digiUnimpErr;

	if (!(bufferList = (**storage).bufferList))
		return badCallOrderErr;

	if (buffer > (**bufferList).count)
		return paramErr;

	if ((**storage).pendingAsyncBuffer != -1) {
		short aBuf = (**storage).pendingAsyncBuffer;

#if HAVE_DEBUGSTR
		if (aBuf == buffer)
			DebugStr("\pasync grab into incomplete buffer");
#endif

		drawVideoFrame(storage, (**bufferList).list[aBuf].location, (**bufferList).list[aBuf].dest);
	}

	(**storage).pendingAsyncBuffer = buffer;

	return noErr;
}

pascal long	vdigDone(vdigGlobals storage, short buffer)
{
	vdigGlobalsPtr store = *storage;
	VdigBufferRecListHandle bufferList;

	if (!gCanAsync)
		return 0;

	if (!(bufferList = store->bufferList))
		return 0;

	if (buffer > (**bufferList).count)
		return 0;

	if (store->frameTimeStep && GetTimeBaseRate(store->timeBase)) {
		if (GetTimeBaseTime(store->timeBase, gVideoTimeScale ? gVideoTimeScale : 600, nil) >= store->nextFrameTime)
			store->nextFrameTime += store->frameTimeStep;
		else
			return false;
	}

	if (store->pendingAsyncBuffer == buffer) {
		drawVideoFrame(storage, (**bufferList).list[buffer].location, (**bufferList).list[buffer].dest);
		store->pendingAsyncBuffer = -1;
	}

	return true;
}

/*
	inputs garbage
*/

pascal VideoDigitizerError vdigGetNumberOfInputs(vdigGlobals storage, short *inputs)
{
	*inputs = 0;

	return noErr;
}

pascal VideoDigitizerError vdigGetInputFormat(vdigGlobals storage, short input, short *format)
{
	if (input == 0) {
		*format = ntscIn;
		return noErr;
	}
	else
		return paramErr;
}

pascal VideoDigitizerError vdigSetInput(vdigGlobals storage, short input)
{
	if (input == 0)
		return noErr;
	else
		return paramErr;
}

pascal VideoDigitizerError vdigGetInput(vdigGlobals storage, short *input)
{
	*input = 0;

	return noErr;
}

/*
	compressed source stuff
*/

void tossCompressStuff(vdigGlobals storage)
{
	if ((**storage).compressSeq) {
		CDSequenceEnd((**storage).compressSeq);
		(**storage).compressSeq = 0;
	}

	if ((**storage).compressGW) {
		DisposeGWorld((**storage).compressGW);
		(**storage).compressGW = 0;
	}

	if ((**storage).compressBuffer) {
		DisposePtr((**storage).compressBuffer);
		(**storage).compressBuffer = 0;
	}

	if ((**storage).desc) {
		DisposeHandle((Handle)(**storage).desc);
		(**storage).desc = 0;
	}
}


pascal VideoDigitizerError vdigSetCompression(vdigGlobals storage, OSType compressType, short depth, Rect *bounds,
			CodecQ spatialQuality, CodecQ temporalQuality, long keyFrameRate)
{
	OSErr err;
	GWorldPtr gw = 0;
	long bufSize;
	Ptr p;
	ImageDescription ** tempImageDesc;
	ImageSequence tempSeqId;

	if (!gDoesCompress)
		return digiUnimpErr;

	tossCompressStuff(storage);

	if (compressType == 'vpza')
		compressType = 'rpza';

	// see if we handle this compression
	if (compressType && gOnlyCompressType && 
		(compressType != gOnlyCompressType))
		return noCodecErr;

	if (!compressType) {
		compressType = gOnlyCompressType ? gOnlyCompressType : 'rpza';
		spatialQuality = codecNormalQuality;
		temporalQuality = 0;
		keyFrameRate = 0;
	}

	(**storage).compressType = compressType;
	(**storage).compressDepth = depth ? depth : 16;
	(**storage).compressRect = *bounds;
	(**storage).spatialQuality = spatialQuality;
	(**storage).temporalQuality = temporalQuality;
	(**storage).keyFrameRate = keyFrameRate;

	if (compressType == -1)
		return noErr;

	{
	Rect tempRect;

	tempRect = *bounds;
	MapRect(&tempRect, &(**storage).digiRect, &(**storage).maxSrcRect);			// convert to full frame
	RectMatrix(&(**storage).matrix, &(**storage).maxSrcRect, &tempRect);		// make a matrix
	}

	err = NewGWorld(&gw, depth, bounds, 0, 0, 0);
	if (err) goto bail;
	(**storage).compressGW = gw;
	LockPixels(gw->portPixMap);

	err = GetMaxCompressionSize(gw->portPixMap, bounds, depth, spatialQuality, compressType, bestSpeedCodec, &bufSize);
	if (err) goto bail;

	p = NewPtr(bufSize);
	if (err = MemError())  goto bail;
	(**storage).compressBuffer = p;

	tempImageDesc = (ImageDescription **)NewHandle(sizeof(ImageDescription));
	if (err = MemError()) goto bail;
	(**storage).desc = tempImageDesc;

	// fill this out for now.. mark will help us out later
	(**tempImageDesc).temporalQuality = temporalQuality;
	(**tempImageDesc).spatialQuality = spatialQuality;
	(**tempImageDesc).width = bounds->right - bounds->left;
	(**tempImageDesc).height = bounds->bottom - bounds->top;
	(**tempImageDesc).depth = (compressType == 'rpza') ? 16 : depth;
	(**tempImageDesc).cType = compressType;
	(**tempImageDesc).hRes = 0x00480000;
	(**tempImageDesc).vRes = 0x00480000;

	err = CompressSequenceBegin(&tempSeqId,
					gw->portPixMap, (PixMap **)0,
					bounds, (Rect *)0,
					depth, compressType, bestSpeedCodec, spatialQuality, 
					temporalQuality, keyFrameRate, (CTabHandle )0,
					codecFlagUpdatePrevious, tempImageDesc );
	if (err) goto bail;

	(**storage).compressSeq = tempSeqId;

bail:
	if (err)
		tossCompressStuff(storage);

	return err;
}

pascal VideoDigitizerError vdigCompressOneFrameAsync(vdigGlobals storage)
{
	if (!gDoesCompress)
		return digiUnimpErr;

	if (!(**storage).compressSeq)
		return badCallOrderErr;

	// draw the offscreen now, compress later...
	drawVideoFrame(storage, *(Point *)&(**storage).compressRect, (**storage).compressGW->portPixMap);

	return noErr;
}

pascal VideoDigitizerError vdigCompressDone(vdigGlobals storage, Boolean *done, Ptr *theData, long *dataSize, unsigned char *similarity, TimeRecord *tr)
{
	vdigGlobalsPtr store = *storage;
	OSErr err;
	Rect r;

	*done = true;

	if (!gDoesCompress)
		return digiUnimpErr;

	if (!store->compressSeq)
		return badCallOrderErr;

	if (store->frameTimeStep && GetTimeBaseRate(store->timeBase)) {
		if (GetTimeBaseTime(store->timeBase, gVideoTimeScale ? gVideoTimeScale : 600, nil) >= store->nextFrameTime)
			store->nextFrameTime += store->frameTimeStep;
		else {
			*done = false;
			return noErr;
		}
	}

	r = store->compressRect;
/*
extern pascal OSErr CompressSequenceFrame(ImageSequence seqID, PixMapHandle src, const Rect *srcRect, 
										  CodecFlags flags, Ptr data, long *dataSize, UInt8 *similarity, 
										  ICMCompletionProcRecordPtr asyncCompletionProc)
*/
	err = (OSErr)CompressSequenceFrame(store->compressSeq, store->compressGW->portPixMap, &r,
							codecFlagUpdatePrevious | (store->forceKeyFrame ? codecFlagForceKeyFrame : 0),
							store->compressBuffer, 
							dataSize, 
							similarity, 
							(ICMCompletionProcRecordPtr)0);
	store = *storage;
	*theData = store->compressBuffer;

	store->forceKeyFrame = false;

	GetTimeBaseTime(store->timeBase, 30, tr);

	return err;
}

pascal VideoDigitizerError vdigReleaseCompressBuffer(vdigGlobals storage, Ptr theBufffer)
{
	if (!gDoesCompress)
		return digiUnimpErr;

	if (!(**storage).compressSeq)
		return badCallOrderErr;

	//¥¥ could actually track this...
	//¥¥Êcould pound part of buffer to 0 or something

	return noErr;
}


pascal VideoDigitizerError vdigGetImageDescription(vdigGlobals storage, ImageDescriptionHandle desc)
{
	OSErr err = noErr;

	if (!gDoesCompress)
		return digiUnimpErr;

	if ((**storage).desc) {
		long dataSize;

		SetHandleSize((Handle)desc, dataSize = GetHandleSize((Handle)(**storage).desc));
		if (err = MemError()) goto bail;
		BlockMove((Ptr)*(**storage).desc, (Ptr)*desc, dataSize);
	}
	else
		err = badCallOrderErr;

bail:
	return err;
}

pascal VideoDigitizerError vdigResetCompressSequence(vdigGlobals storage)
{
	OSErr err = noErr;

	if (!gDoesCompress)
		return digiUnimpErr;

	(**storage).forceKeyFrame = true;

	return err;
}

pascal VideoDigitizerError vdigSetCompressionOnOff(vdigGlobals storage, Boolean newState)
{
	OSErr err = noErr;

	if (!gDoesCompress)
		return digiUnimpErr;

	if (newState != (**storage).compressOn) {
		(**storage).compressOn = newState;
		if (!(**storage).compressOn)
			tossCompressStuff(storage);
	}

	return err;
}

pascal VideoDigitizerError vdigGetCompressionTypes(vdigGlobals storage, VDCompressionListHandle h)
{
	CodecInfo info;
	VDCompressionListPtr p;

	GetCodecInfo(&info, 'rpza',  0);
	SetHandleSize((Handle)h, sizeof(VDCompressionList));
	p = &(*h)[0];
	BlockMove(info.typeName, p->typeName, 32);
	p->typeName[++p->typeName[0]] = ' ';
	p->typeName[++p->typeName[0]] = 'V';
	p->typeName[++p->typeName[0]] = 'D';
	p->typeName[++p->typeName[0]] = 'I';
	p->typeName[++p->typeName[0]] = 'G';
	BlockMove(p->typeName, p->name, 64);
	p->codec = 0;
	p->cType = 'rpza';
	p->formatFlags = info.formatFlags;
	p->compressFlags = info.compressFlags;

	return noErr;
}

pascal VideoDigitizerError vdigSetTimeBase(vdigGlobals storage, TimeBase t)
{
	(**storage).timeBase = t;
	return noErr;
}

pascal VideoDigitizerError vdigSetFrameRate(vdigGlobals storage, Fixed framesPerSecond)
{
	vdigGlobalsPtr store = *storage;

	if (!gDoesFrameRate)
		return digiUnimpErr;

	if (framesPerSecond < 0)
		return paramErr;

	store->nextFrameTime = store->frameTimeStep = 0;
	if (framesPerSecond)
		store->frameTimeStep = FixDiv(gVideoTimeScale ? gVideoTimeScale : 600, framesPerSecond);

	return noErr;
}

pascal VideoDigitizerError vdigGetDMADepths(vdigGlobals storage, long *depthArray, long *preferredDepth)
{
	if (!gCanDMA)
		return paramErr;

	*depthArray = gDMADepths;
	*preferredDepth = 40;

	return noErr;
}

pascal VideoDigitizerError vdigGetPreferredTimeScale(vdigGlobals storage, TimeScale *preferred)
{
	if (gVideoTimeScale) {
		*preferred = gVideoTimeScale;
		return noErr;
	}
	else
		return badComponentSelector;
}

