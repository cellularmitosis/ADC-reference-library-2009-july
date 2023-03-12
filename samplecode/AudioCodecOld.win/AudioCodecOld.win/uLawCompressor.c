/*
	File:		uLawCompressor.c

	Contains:	uLaw Compression Sound Component

	Written by:	Jim Reekes

	Copyright:	© 1991-1998 by Apple Computer, Inc., all rights reserved.

*/
#if defined(_MSC_VER) && !defined(__MWERKS__) 
#pragma warning(disable:4229)		// ignore anachronism used: modifiers on data are ignored
#endif

#include <MacTypes.h>
#include <Errors.h>
#include <Endian.h>
#include <Components.h>
#include <MacMemory.h>
#include <Resources.h>
#include <Controls.h>
#include <Dialogs.h>
#include <Sound.h>
#include <MoviesFormat.h>

#include "uLawCodec.h"


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//			Constants
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

#define kInputFormat		k16BitNativeEndianFormat		/* required input format */
#define kOutputFormat		kCodecFormat					/* only formats we output */

#define ZEROTRAP	1										/* turn on the trap as per the MIL-STD */

enum {
	kOutputSampleSize		= 16,								/* size of output samples */
	kOutputSamples			= ((kMaxOutputSamples / kULawBlockSamples) * kULawBlockSamples),	/* max actual samples in output buffer */
	kOutputBufferBytes		= ((kMaxOutputSamples / kULawBlockSamples) * kULawBlockBytes * 2),	/* room for ULaw stereo data */
	kLeftoverBufferBytes	= (kULawBlockSamples * 2 * 2),		/* room for leftover stereo 16-bit samples */

	kInputSampleSize		= 16,

	BIAS					= 0x84,							/* define the add-in bias for 16 bit samples */
	CLIP					= 32635,

	kOptionCheckBox			= 4
};

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//			types
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

#if PRAGMA_STRUCT_ALIGN
	#pragma options align=mac68k
#elif PRAGMA_STRUCT_PACKPUSH
	#pragma pack(push, 2)
#elif PRAGMA_STRUCT_PACK
	#pragma pack(2)
#endif

// Keep your data long word aligned for best performance

typedef struct {
	ComponentInstance		self;								/* our instance */
	ComponentInstance		sourceComponent;					/* who to call for more source data */
	SoundComponentDataPtr	sourceDataPtr;						/* pointer to source data descriptor */
	SoundComponentData		outputData;							/* local data descriptor record */
	SoundSource				sourceID;
	long					outputBufferSamples;				/* no. samples in output buffer */
	CompressionInfo			sourceCompInfo;						/* info about source data format */
	CompressionInfo			destCompInfo;						/* info about destination data format */
	Handle					ulawTableHandle;					/* resource handle to ulaw table */
	Byte					*ulawTable;							/* pointer to ulaw table */
	unsigned long			leftOverSamples;					/* no. samples in leftover buffer */
	Byte					leftOverBuffer[kLeftoverBufferBytes];				/* space for leftover samples */
	Byte					buffer[kOutputBufferBytes + kLeftoverBufferBytes];	/* buffer space */
} ULawCompGlobals, *ULawCompGlobalsPtr;

typedef struct {
	AudioFormatAtom			formatData;
	AudioEndianAtom			endianData;
	AudioTerminatorAtom		terminatorData;
} AudioCompressionAtom, *AudioCompressionAtomPtr, **AudioCompressionAtomHandle;

#if PRAGMA_STRUCT_ALIGN
	#pragma options align=reset
#elif PRAGMA_STRUCT_PACKPUSH
	#pragma pack(pop)
#elif PRAGMA_STRUCT_PACK
	#pragma pack()
#endif

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//			Prototypes
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

static OSErr				DisplayOptionsDialog(ULawCompGlobalsPtr globals);
static void					SetButtonState(DialogPtr dialog, short item, Boolean state);
static short				GetButtonState(DialogPtr dialog, short item);
static OSErr				GetCompressionParams(ULawCompGlobalsPtr globals, AudioCompressionAtomHandle *params);
static OSErr				SetCompressionParams(ULawCompGlobalsPtr globals, UserDataAtom *atom);
static ComponentResult		PrimeSource(ULawCompGlobalsPtr globals);
static unsigned long		SamplesToBytes(unsigned long sampleCount,  CompressionInfoPtr compInfo);
static unsigned long		BytesToSamples(unsigned long byteCount,  CompressionInfoPtr compInfo);
static unsigned long		SamplesToFrames(unsigned long sampleCount,  CompressionInfoPtr compInfo);
static unsigned long		FramesToSamples(unsigned long framesCount,  CompressionInfoPtr compInfo);
static void					CompressThisMess(Byte *inputBuffer, Byte *outputBuffer, Byte *ulawTable, long samplesToConvert, short numChannels);
static void					GetCompressorInfo(CompressionInfoPtr cp);
static void					CompressULaw(SInt16 *inPtr, Byte *outPtr, Byte *ulawTable, long sampleCount, short numChannels, short whichChannel);

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//			Component Dispatcher
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

#if TARGET_CPU_68K
	#define COMPONENT_C_DISPATCHER
	#define COMPONENT_DISPATCH_MAIN
#endif

#define CALLCOMPONENT_BASENAME() 		__uLawComp
#define CALLCOMPONENT_GLOBALS() 		ULawCompGlobalsPtr storage

#define SOUNDCOMPONENT_BASENAME() 		CALLCOMPONENT_BASENAME()
#define SOUNDCOMPONENT_GLOBALS() 		CALLCOMPONENT_GLOBALS()

#define COMPONENT_UPP_SELECT_ROOT()		SoundComponent
#define COMPONENT_DISPATCH_FILE			"uLawCodecDispatch.h"

#define	GET_DELEGATE_COMPONENT()		(storage->sourceComponent)

#include "Components.k.h"
#include "Sound.k.h"
#include "ComponentDispatchHelper.c"


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//			Component Routines
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
static pascal ComponentResult __uLawCompOpen(ULawCompGlobalsPtr globals, ComponentInstance self)
{
	OSErr				result;

	globals = (ULawCompGlobalsPtr)NewPtrSysClear(sizeof(ULawCompGlobals));
	FailWithAction(globals == nil, result = MemError(), Failure);
	
	result = GetComponentResource((Component)self, k8BitTableResType, kSoundCompressorResID, &globals->ulawTableHandle);
	FailIf(result != noErr, NoResource);
	
	HLock(globals->ulawTableHandle);
	globals->ulawTable = (Byte *)StripAddress(*globals->ulawTableHandle);

	globals->self = self;
	globals->outputData.format = kOutputFormat;			// output format
	globals->outputData.sampleSize = kOutputSampleSize;	// output sample size

	globals->outputBufferSamples = kOutputSamples;		// will be set to requested->sampleCount later

	// set our storage pointer to our globals
	SetComponentInstanceStorage(self, (Handle) globals);
	return (noErr);

NoResource:
	DisposePtr((Ptr)globals);
Failure:
	return (result);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
static pascal ComponentResult __uLawCompClose(ULawCompGlobalsPtr globals, ComponentInstance self)
{
	ComponentResult		result;

	self; // suppress "unused variable" warning for all compilers

	if (globals != nil)									// we have some globals
	{
		if (globals->sourceComponent)
		{
			result = CloseComponent(globals->sourceComponent);	// torch source component
			FailMessage(result != noErr);
		}
		globals->outputData.sampleCount = 0;			// nothing in our buffer now

		if (globals->ulawTableHandle != nil)
			DisposeHandle(globals->ulawTableHandle);
		DisposePtr((Ptr)globals);						// torch them
	}

	return (noErr);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
pascal ComponentResult __uLawCompVersion(ULawCompGlobalsPtr globals)
{
	globals; // suppress "unused variable" warning for all compilers

	return (kSoundCompressorVersion);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Sound Component Methods
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
static pascal ComponentResult __uLawCompSetSource(ULawCompGlobalsPtr globals, SoundSource sourceID, ComponentInstance source)
{
	SoundComponentData		sourceData;
	short					i;
	Byte					*bPtr;

	globals->sourceID = sourceID;
	globals->sourceComponent = source;					// our food source
	globals->sourceDataPtr = nil;						// nothing read from source yet

	bPtr = (Byte *)&sourceData;							// zero out struct
	for (i = 0; i < sizeof(SoundComponentData); i++)
		bPtr = 0;
		
	sourceData.format = kInputFormat;					// our source must give us this format
	sourceData.sampleSize = kInputSampleSize;
	sourceData.sampleCount = globals->outputBufferSamples;

	// make sure we can get the source we need
    return (noErr);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
static pascal ComponentResult __uLawCompSetOutput(ULawCompGlobalsPtr globals, SoundComponentDataPtr requested, SoundComponentDataPtr *actual)
{
	globals->outputBufferSamples = requested->sampleCount;				// no. samples to output
	if (globals->outputBufferSamples > kOutputSamples)					// too much for our buffer
		globals->outputBufferSamples = kOutputSamples;

	// must be one of the output formats we support
	if (requested->format != kOutputFormat)
		goto Failure;

	globals->outputData = *requested;
	return (noErr);

Failure:
	// here's what we can do
	*actual = &globals->outputData;
	return (paramErr);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
static pascal ComponentResult __uLawCompGetInfo(ULawCompGlobalsPtr globals, SoundSource sourceID, OSType selector, void *infoPtr)
{
	ComponentResult		result;

	result = noErr;
	switch (selector)
	{
		case siCompressionFactor:
			GetCompressorInfo(infoPtr);
			break;

		case siCompressionParams:
			result = GetCompressionParams(globals, infoPtr);
			break;

		case siOptionsDialog:
			*(short *)infoPtr = true;
			break;

		default:
			if (globals->sourceComponent == nil)
				result = siUnknownInfoType;
			else
				result = SoundComponentGetInfo(globals->sourceComponent, sourceID, selector, infoPtr);
			break;
	}
	return (result);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
static pascal ComponentResult __uLawCompSetInfo(ULawCompGlobalsPtr globals, SoundSource sourceID, OSType selector, void *infoPtr)
{
	ComponentResult		result;

	result = noErr;
	switch (selector)
	{
		case siOptionsDialog:
			DisplayOptionsDialog(globals);
			break;

		case siCompressionParams:
			result = SetCompressionParams(globals, infoPtr);
			break;

		default:
			if (globals->sourceComponent == nil)
				result = siUnknownInfoType;
			else
				result = SoundComponentSetInfo(globals->sourceComponent, sourceID, selector, infoPtr);
			break;
	}

	return (result);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
static pascal ComponentResult __uLawCompPlaySourceBuffer(ULawCompGlobalsPtr globals, SoundSource sourceID, SoundParamBlockPtr pb, long actions)
{
	globals->sourceDataPtr = nil;						// no source yet
	globals->outputData.sampleCount = 0;				// our buffer is empty

	return (SoundComponentPlaySourceBuffer(globals->sourceComponent, sourceID, pb, actions));
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
static pascal ComponentResult __uLawCompStopSource(ULawCompGlobalsPtr globals, short count, SoundSource *sources)
{
	globals->sourceDataPtr = nil;						// assume our source is gone
	globals->leftOverSamples = 0;						// clear anything in leftover buffer
	return (SoundComponentStopSource(globals->sourceComponent, count, sources));
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
static pascal ComponentResult __uLawCompGetSourceData(ULawCompGlobalsPtr globals, SoundComponentDataPtr *resultData)
{
	SoundComponentDataPtr	sourceDataPtr;
	long					samplesToCopy;
	unsigned long			bytesToCopy, framesToCopy, byteOffset;
	ComponentResult			result = noErr;

	if (globals->sourceDataPtr == nil)							// no source pointer so...
	{
		result = PrimeSource(globals);							// get data from our source
		FailIf(result != noErr, Failure);
	}

	sourceDataPtr = globals->sourceDataPtr;						// get pointer to source sound component
	if ((sourceDataPtr->format == globals->outputData.format) ||		// input and output are same
		(sourceDataPtr->buffer == nil))									// or no source buffer
	{
		globals->sourceDataPtr = nil;							// get new source next time
		*resultData = sourceDataPtr;							// pass source on down
		return (noErr);											// get out
	}

	globals->outputData.buffer = globals->buffer;				// initialize output buffer
	globals->outputData.sampleCount = 0;

	while ((sourceDataPtr->sampleCount < kULawBlockSamples)	||	// we don't have enough source for at least one block
		   (globals->leftOverSamples))							// or we have samples in the leftover buffer
	{
		if (sourceDataPtr->sampleCount == 0)					// used up all the source
		{
			result = SoundComponentGetSourceData(globals->sourceComponent, &globals->sourceDataPtr);	// get more source
			FailIf(result != noErr, Failure);
			sourceDataPtr = globals->sourceDataPtr;				// get pointer to source sound component
			if (sourceDataPtr->sampleCount == 0)				// still no source samples - all done
				break;
		}

		samplesToCopy = kULawBlockSamples - globals->leftOverSamples; // compute samples needed to fill leftover buffer
		if (sourceDataPtr->sampleCount < samplesToCopy)				// not enough source to fill it, so do what we can
			samplesToCopy = sourceDataPtr->sampleCount;

		// copy from source into leftover buffer
		bytesToCopy = SamplesToBytes(samplesToCopy, &globals->sourceCompInfo);
		byteOffset = SamplesToBytes(globals->leftOverSamples, &globals->sourceCompInfo);
		BlockMoveData(sourceDataPtr->buffer, globals->leftOverBuffer + byteOffset, bytesToCopy);

		sourceDataPtr->buffer += bytesToCopy;					// samples removed from source
		sourceDataPtr->sampleCount -= samplesToCopy;
		globals->leftOverSamples += samplesToCopy;				// keep track off samples in leftover buffer

		if (globals->leftOverSamples == kULawBlockSamples)		// leftover buffer is full
		{
			CompressThisMess(globals->leftOverBuffer, globals->outputData.buffer, globals->ulawTable, kULawBlockSamples, sourceDataPtr->numChannels);

			globals->outputData.buffer += SamplesToBytes(kULawBlockSamples, &globals->destCompInfo);
			globals->outputData.sampleCount += kULawBlockSamples;
			globals->leftOverSamples = 0;

			if (globals->outputData.sampleCount == kOutputSamples)	// output buffer is full
				break;											// stop converting
		}
	}

	samplesToCopy = kOutputSamples - globals->outputData.sampleCount;	// find amount available in output buffer
	if (samplesToCopy > sourceDataPtr->sampleCount)						// don't copy more than we have
		samplesToCopy = sourceDataPtr->sampleCount;

	framesToCopy = SamplesToFrames(samplesToCopy, &globals->destCompInfo);

	if (framesToCopy)											// source has some data for us
	{
		samplesToCopy = FramesToSamples(framesToCopy, &globals->destCompInfo);

		CompressThisMess(sourceDataPtr->buffer, globals->outputData.buffer, globals->ulawTable, samplesToCopy, sourceDataPtr->numChannels);

		sourceDataPtr->buffer += SamplesToBytes(samplesToCopy, &globals->sourceCompInfo);	// update input buffer
		sourceDataPtr->sampleCount -= samplesToCopy;

		globals->outputData.buffer += SamplesToBytes(samplesToCopy, &globals->destCompInfo);
		globals->outputData.sampleCount += samplesToCopy;
	}

	globals->outputData.buffer = globals->buffer;				// reset to beginning of buffer
	*resultData = &globals->outputData;							// tell them what we made
	return (noErr);

Failure:
	return (result);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//		sub routines    _
//				   	  _|_
//		o		_____|   |_____
//		  o o  +______________/
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
static OSErr DisplayOptionsDialog(ULawCompGlobalsPtr globals)
{
	DialogPtr		dialog;
	short			resFile;
	short			currentResFile;
	short			itemHit;
	OSErr			result;

	result = noErr;
	currentResFile = CurResFile();
	resFile = OpenComponentResFile((Component)globals->self);
	FailWithAction(resFile == kResFileNotOpened, result = ResError(), Exit);

	dialog = GetNewDialog(kSoundCompressorResID, nil, (WindowPtr)-1);
	FailWithAction(dialog == nil, result = resNotFound, NoDialog);

	MacSetPort(dialog);
	SetButtonState(dialog, kOptionCheckBox, true);
	SetDialogDefaultItem(dialog, kStdOkItemIndex);
	SetDialogCancelItem(dialog, kStdCancelItemIndex);
	MacShowWindow(dialog);

	do
	{
		ModalDialog(nil, &itemHit);
		switch (itemHit)
		{
			case kOptionCheckBox:
				if (GetButtonState(dialog, kOptionCheckBox))
					SetButtonState(dialog, kOptionCheckBox, false);
				else
					SetButtonState(dialog, kOptionCheckBox, true);
				break;
				
			case kStdOkItemIndex:
				GetButtonState(dialog, kOptionCheckBox);
				// accept changes according to values set in dialog
				break;
		}
	} while ( (itemHit != kStdOkItemIndex) && (itemHit != kStdCancelItemIndex) );

	DisposeDialog(dialog);

NoDialog:
	CloseResFile(resFile);
Exit:
	UseResFile(currentResFile);
	return (result);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
static void SetButtonState(DialogPtr dialog, short item, Boolean state)
{
	short		iType;
	Handle		iHandle;
	Rect		iRect;

	GetDialogItem(dialog, item, &iType, &iHandle, &iRect);
	if (iHandle != nil)
		SetControlValue((ControlHandle) iHandle, (state) ? kControlRadioButtonCheckedValue : kControlRadioButtonUncheckedValue);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
static short GetButtonState(DialogPtr dialog, short item)
{
	short		iType;
	Handle		iHandle;
	Rect		iRect;

	GetDialogItem(dialog, item, &iType, &iHandle, &iRect);
	return (GetControlValue((ControlHandle) iHandle));
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
static OSErr GetCompressionParams(ULawCompGlobalsPtr globals, AudioCompressionAtomHandle *params)
{
	AudioCompressionAtomHandle		atom;
	AudioCompressionAtomPtr			atomPtr;
	OSErr							result;

	globals; // suppress "unused variable" warning for all compilers

	result = noErr;
	atom = (AudioCompressionAtomHandle)NewHandle(sizeof(AudioCompressionAtom));
	FailWithAction(atom == nil, result = MemError(), Exit);

	atomPtr = *atom;
	atomPtr->formatData.size = EndianU32_NtoB(sizeof(AudioFormatAtom));
	atomPtr->formatData.atomType = EndianU32_NtoB(kAudioFormatAtomType);
	atomPtr->formatData.format = EndianU32_NtoB(kOutputFormat);

	atomPtr->endianData.size = EndianU32_NtoB(sizeof(AudioEndianAtom));
	atomPtr->endianData.atomType = EndianU32_NtoB(kAudioEndianAtomType);
	atomPtr->endianData.littleEndian = EndianU16_NtoB(false);		// µlaw is not little endian

	atomPtr->terminatorData.size = EndianU32_NtoB(sizeof(AudioTerminatorAtom));
	atomPtr->terminatorData.atomType = EndianU32_NtoB(kAudioTerminatorAtomType);

	*params = atom;

Exit:
	return (result);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
static OSErr SetCompressionParams(ULawCompGlobalsPtr globals, UserDataAtom *atom)
{
	OSErr			result;
	short			littleEndian;
	Boolean			moreAtoms;

	globals; // suppress "unused variable" warning for all compilers

	result = noErr;
	moreAtoms = true;
	littleEndian = false;
	do
	{
		long atomSize = EndianS32_BtoN(atom->size);
		FailWithAction(atomSize < 8, result = invalidAtomErr, Exit);
		switch (EndianU32_BtoN(atom->atomType))
		{
			case kAudioFormatAtomType:
				FailWithAction(((AudioFormatAtom *)atom)->format != EndianU32_NtoB(kOutputFormat), result = badFormat, Exit);
				break;

			case kAudioEndianAtomType:
				littleEndian = EndianU16_BtoN(((AudioEndianAtom *)atom)->littleEndian);
				break;

			case kAudioTerminatorAtomType:
				moreAtoms = false;
				break;

			default:	// unknown atom type
				break;
		}
		atom = (UserDataAtom *)((long)atom + atomSize);
	} while (moreAtoms);
	if (littleEndian)
		result = paramErr;						// we do not do little endian

Exit:
	return (result);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
static ComponentResult PrimeSource(ULawCompGlobalsPtr globals)
{
	ComponentResult			result = noErr;
	SoundComponentDataPtr	sourceDataPtr;

	result = SoundComponentGetSourceData(globals->sourceComponent, &globals->sourceDataPtr);
	FailIf(result != noErr, Failure);
	FailWithAction(globals->sourceDataPtr == nil, result = paramErr, Failure);

	sourceDataPtr = globals->sourceDataPtr;
	globals->outputData.flags = sourceDataPtr->flags;				// copy flags unchanged
	globals->outputData.sampleRate = sourceDataPtr->sampleRate;		// copy sample rate unchanged
	globals->outputData.numChannels = sourceDataPtr->numChannels;	// copy numchannels unchanged

	globals->sourceCompInfo.recordSize = sizeof(CompressionInfo);	// get source compression info
	result = GetCompressionInfo(fixedCompression, sourceDataPtr->format,
								sourceDataPtr->numChannels, sourceDataPtr->sampleSize,
								&globals->sourceCompInfo);
	FailIf(result != noErr, Failure);

	globals->destCompInfo.recordSize = sizeof(CompressionInfo);
	GetCompressorInfo(&globals->destCompInfo);					// get dest compression info
	globals->destCompInfo.bytesPerFrame = globals->destCompInfo.bytesPerPacket * sourceDataPtr->numChannels;

Failure:
	return (result);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
static unsigned long SamplesToBytes(unsigned long sampleCount, CompressionInfoPtr compInfo)
{
	return ((sampleCount / compInfo->samplesPerPacket) * compInfo->bytesPerFrame);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
static unsigned long BytesToSamples(unsigned long byteCount, CompressionInfoPtr compInfo)
{
	return ((byteCount / compInfo->bytesPerFrame) * compInfo->samplesPerPacket);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
static unsigned long SamplesToFrames(unsigned long sampleCount, CompressionInfoPtr compInfo)
{
	return (sampleCount / compInfo->samplesPerPacket);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
static unsigned long FramesToSamples(unsigned long frameCount, CompressionInfoPtr compInfo)
{
	return (frameCount * compInfo->samplesPerPacket);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
static void CompressThisMess(Byte *inputBuffer, Byte *outputBuffer, Byte *ulawTable, long samplesToConvert, short numChannels)
{
	if (numChannels == 1)
	{
		CompressULaw((SInt16 *) inputBuffer, outputBuffer, ulawTable, samplesToConvert, 1, 1);
	}
	else
	{
		CompressULaw((SInt16 *) inputBuffer, outputBuffer, ulawTable, samplesToConvert, 2, 1);
		CompressULaw((SInt16 *) inputBuffer, outputBuffer, ulawTable, samplesToConvert, 2, 2);
	}
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
static void GetCompressorInfo(CompressionInfoPtr cp)
{
	if (cp->recordSize > sizeof(CompressionInfo))				// limit amount we return
		cp->recordSize = sizeof(CompressionInfo);

	cp->compressionID = fixedCompression;
	cp->format = kOutputFormat;
	cp->samplesPerPacket = kULawBlockSamples;
	cp->bytesPerPacket = kULawBlockBytes;
	cp->bytesPerSample = kULawBytesPerSample;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
static void CompressULaw(SInt16 *inPtr, Byte *outPtr, Byte *ulawTable, long sampleCount,
				  short numChannels, short whichChannel)
{
	long			sampnum;
    int				sample, sign, exponent, mantissa;
    unsigned char	ulawbyte;

	whichChannel -= 1;
	inPtr += whichChannel;						/* skip to first sample */
	outPtr += whichChannel;

  	for (sampnum = 0; sampnum < sampleCount; sampnum++)
  	{
		sample = *inPtr;
		inPtr += numChannels;					/* skip to next sample */

	    /* Get the sample into sign-magnitude. */
	    sign = (sample >> 8) & 0x80;			/* set aside the sign */
	    if (sign != 0)
	    	sample = -sample;					/* get magnitude */
	    if (sample > CLIP)
	    	sample = CLIP;						/* clip the magnitude */

	    /* Convert from 16 bit linear to ulaw. */
	    sample = sample + BIAS;
	    exponent = ulawTable[(sample >> 7) & 0xFF];
	    mantissa = (sample >> (exponent + 3)) & 0x0F;
		ulawbyte = ~(sign | (exponent << 4) | mantissa);
#ifdef ZEROTRAP
	    if (ulawbyte == 0)
	    	ulawbyte = 0x02;					/* optional CCITT trap */
#endif
		*outPtr = ulawbyte;						/* output compressed sample */
		outPtr += numChannels;
	}
}
