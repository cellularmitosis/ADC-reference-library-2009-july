/*
	File:		softVdig.c

	Contains:	Software video digitzer header file (constants, structs, prototypes)

	Written by:	Peter Hoddie (mostly) and Casey King and Gary Woodcock
	
				Refer to develop Issue 14, "Video Digitizing Under QuickTime",
				for details on this code.
				
				This code requires QuickTime 1.5.

	Copyright:	© 1993 by Apple Computer, Inc.

	Change History (most recent first):

*/

//-----------------------------------------------------------------------
// includes

#ifndef	_SOFTVDIG_
#define	_SOFTVDIG_

#include <Components.h>
#include "QuickTimeComponents.h"
#include <QDOffscreen.h>

// Comment out this line if you are building a standalone component
//#define DEBUG_IT

#ifdef DEBUG_IT
pascal ComponentResult softVdig (ComponentParameters *params, Handle storage);
Component RegisterSoftVdig(void);
#endif DEBUG_IT


// constants which define the characteristics of this vdig
//	there are 2 other global controls which are initialized in vdigOpen
//	these global controls are gAttachedGDevice and gAuxBufferRect

#define		kMaxHorNTSCIn 		320				// maps to the MaxSrcRect width
#define		kMaxVerNTSCIn 		240				// maps to the MaxSrcRect height
#define		kVerBlank			0				// would map to VerBlankRect, but unused here
#define		gMaxSoftVdigCount  	1				// how many can be open at once
#define		gDoesDepths  		0				// what output depth can the device output to
#define		gDoesPlaythru  		false			// will respond to VDSetPlayThruOnOff
#define		gHasAuxBuffer  		false			// simulates local buffering											
#define		gAuxDepth  			16				// for use in simulating local buffering							
#define		gCanAsync 			true			// simulates async grab capability				
#define		gCanClip  			true			// clipping can be applied
#define		gCanScale  			true			// supports stretch & shrink
#define		gCanDMA  			true			// simulates the "go anywhere" vdig						
#define		gDMADepths  		16 | 8			// depths that DMA works				
#define		gMaxHeight  		240				// maximum scalable height
#define		gMaxWidth  			320				// maximum scalable width
#define		gMinHeight  		10				// minimum scalable height
#define		gMinWidth  			10				// minimum scalable width
#define		gDoesFrameRate  	false			// simulates controllable frame rate grabs
#define		gVideoTimeScale  	2997			
#define		gDoesCompress  		false			// capable of producing compressed data			
#define		gOnlyDoesCompress  	false			// can't provide image data	
#define		gOnlyCompressType  	'rpza'			// set to zero to support all compression types

// softvdig global variables

typedef struct {
	ComponentInstance	self;					// this instance
	long				frameNumber;			// frame counter
	GWorldPtr			tempPort;				// use for drawing simulated video frames
	Rect				maxSrcRect;				// for VDGetMaxSrcRect
	Rect				digiRect;				// part of frame to use
	Boolean				playThruOn;				// draw during idle
	Rect 				playThruRect;
	PixMapHandle		playThruPixMap;
	GWorldPtr			auxBuffer;				// the auxbuffer, if using one
	MatrixRecord		matrix;					// display matrix
	RgnHandle			clipRgn;				// current clip
	Point				clipOrigin;				// top left of display when clip set
	VdigBufferRecListHandle bufferList;			// buffer list for async stuff
	short				pendingAsyncBuffer;		// pending grab request... -1 if none.

	// compressed source stuff
	OSType				compressType;
	short				compressDepth;
	Rect				compressRect;
	CodecQ				spatialQuality;
	CodecQ				temporalQuality;
	long				keyFrameRate;
	ImageDescriptionHandle	desc;
	Ptr					compressBuffer;
	GWorldPtr			compressGW;
	Boolean				forceKeyFrame;
	Boolean				compressOn;
	ImageSequence		compressSeq;
	TimeBase			timeBase;
	TimeValue			nextFrameTime;			// next time if doing frame rate limiting
	TimeValue			frameTimeStep;			// increment for stepping, non-zero means do limiting

	// more globals
	long				inputCurrentFlags;
	long				outputCurrentFlags;	
	unsigned short		blackLevel;
	unsigned short		whiteLevel;
	unsigned short		brightness;
	unsigned short		hue;
	unsigned short		saturation;
	unsigned short		contrast;
	unsigned short		sharpness;
	Rect				gAuxBufferRect;
	GDHandle			gAttachedGDevice;

} vdigGlobalsRecord, *vdigGlobalsPtr, **vdigGlobals;

// prototypes that map to VD API calls
pascal ComponentResult vdigOpen(vdigGlobals storage, ComponentInstance self);
pascal ComponentResult vdigClose(vdigGlobals storage, ComponentInstance self);
pascal ComponentResult vdigCanDo(vdigGlobals storage, short ftnNumber);
pascal ComponentResult vdigVersion(vdigGlobals storage);

pascal VideoDigitizerError vdigGetMaxSrcRect(vdigGlobals storage, short inputStd, Rect *maxSrcRect);
pascal VideoDigitizerError vdigGetActiveSrcRect(vdigGlobals storage, short inputStd, Rect *activeSrcRect);
pascal VideoDigitizerError vdigSetDigitizerRect(vdigGlobals storage, Rect *digiRect);
pascal VideoDigitizerError vdigGetDigitizerRect(vdigGlobals storage, Rect *digiRect);
pascal VideoDigitizerError vdigUseThisCLUT(vdigGlobals storage, CTabHandle colorTableHandle);
pascal VideoDigitizerError vdigGrabOneFrame(vdigGlobals storage);
pascal VideoDigitizerError vdigSetContrast(vdigGlobals storage, unsigned short *contrast);
pascal VideoDigitizerError vdigSetHue(vdigGlobals storage, unsigned short *hue);
pascal VideoDigitizerError vdigSetBrightness(vdigGlobals storage, unsigned short *brightness);
pascal VideoDigitizerError vdigSetSaturation(vdigGlobals storage, unsigned short *saturation);
pascal VideoDigitizerError vdigSetSharpness(vdigGlobals storage, unsigned short *sharpness);
pascal VideoDigitizerError vdigSetBlackLevel(vdigGlobals storage, unsigned short *blackLevel);
pascal VideoDigitizerError vdigSetWhiteLevel(vdigGlobals storage, unsigned short *whiteLevel);
pascal VideoDigitizerError vdigGetContrast(vdigGlobals storage, unsigned short *contrast);
pascal VideoDigitizerError vdigGetHue(vdigGlobals storage, unsigned short *hue);
pascal VideoDigitizerError vdigGetBrightness(vdigGlobals storage, unsigned short *brightness);
pascal VideoDigitizerError vdigGetSaturation(vdigGlobals storage, unsigned short *saturation);
pascal VideoDigitizerError vdigGetSharpness(vdigGlobals storage, unsigned short *sharpness);
pascal VideoDigitizerError vdigGetBlackLevel(vdigGlobals storage, unsigned short *blackLevel);
pascal VideoDigitizerError vdigGetWhiteLevel(vdigGlobals storage, unsigned short *whiteLevel);
pascal VideoDigitizerError vdigGetVideoDefaults(vdigGlobals storage,
							unsigned short *blackLevel, unsigned short *whiteLevel,
							unsigned short *brightness, unsigned short *hue, unsigned short *saturation,
							unsigned short *contrast, unsigned short *sharpness);
pascal VideoDigitizerError vdigGetMaxAuxBuffer(vdigGlobals storage, PixMapHandle *pm, Rect *r);
pascal VideoDigitizerError vdigGetDigitizerInfo(vdigGlobals storage, DigitizerInfo *info);
pascal VideoDigitizerError vdigGetCurrentFlags(vdigGlobals storage, long *inputCurrentFlag, long *outputCurrentFlag);
pascal VideoDigitizerError vdigSetPlayThruOnOff(vdigGlobals storage, short state);
pascal VideoDigitizerError vdigSetPlayThruDestination(vdigGlobals storage, PixMapHandle dest, Rect *destRect, MatrixRecord *m, RgnHandle mask);
pascal VideoDigitizerError vdigSetupBuffers(vdigGlobals storage, VdigBufferRecListHandle bufferList);
pascal VideoDigitizerError vdigPreflightDestination(vdigGlobals storage, Rect *digitizerRect, PixMap **dest, Rect *destRect, MatrixRecord *m);
pascal VideoDigitizerError vdigGrabOneFrameAsync(vdigGlobals storage, short buffer);
pascal long	vdigDone(vdigGlobals storage, short buffer);
pascal VideoDigitizerError vdigGetNumberOfInputs(vdigGlobals storage, short *inputs);
pascal VideoDigitizerError vdigGetInputFormat(vdigGlobals storage, short input, short *format);
pascal VideoDigitizerError vdigSetInput(vdigGlobals storage, short input);
pascal VideoDigitizerError vdigGetInput(vdigGlobals storage, short *input);
pascal VideoDigitizerError vdigSetCompression(vdigGlobals storage, OSType compressType, short depth, Rect *bounds,
			CodecQ spatialQuality, CodecQ temporalQuality, long keyFrameRate);
pascal VideoDigitizerError vdigCompressOneFrameAsync(vdigGlobals storage);
pascal VideoDigitizerError vdigCompressDone(vdigGlobals storage, Boolean *done, Ptr *theData, long *dataSize, unsigned char *similarity, TimeRecord *tr);
pascal VideoDigitizerError vdigReleaseCompressBuffer(vdigGlobals storage, Ptr theBufffer);
pascal VideoDigitizerError vdigGetImageDescription(vdigGlobals storage, ImageDescriptionHandle desc);
pascal VideoDigitizerError vdigResetCompressSequence(vdigGlobals storage);
pascal VideoDigitizerError vdigSetCompressionOnOff(vdigGlobals storage, Boolean newState);
pascal VideoDigitizerError vdigGetCompressionTypes(vdigGlobals storage, VDCompressionListHandle h);
pascal VideoDigitizerError vdigSetTimeBase(vdigGlobals storage, TimeBase t);
pascal VideoDigitizerError vdigSetFrameRate(vdigGlobals storage, Fixed framesPerSecond);
pascal VideoDigitizerError vdigGetDMADepths(vdigGlobals storage, long *depthArray, long *preferredDepth);
pascal VideoDigitizerError vdigGetPreferredTimeScale(vdigGlobals storage, TimeScale *preferred);

// prototypes for utility functions
void makeOffscreen(vdigGlobals storage);
void tossOffscreen(vdigGlobals storage);
void drawVideoFrame(vdigGlobals storage, Point where, PixMapHandle destPixMap);
Boolean validatePixMap(vdigGlobals storage, PixMapHandle p);
void tossCompressStuff(vdigGlobals storage);

#endif _MATHCOMPONENT_
