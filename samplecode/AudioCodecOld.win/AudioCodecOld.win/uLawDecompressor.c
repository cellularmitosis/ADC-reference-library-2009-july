/*
	File:		uLawDecompressor.c

	Contains	µLaw 2:1 Decompression Sound Component

	Written by:	Jim Reekes

	Copyright:	© 1991-1998 by Apple Computer, Inc., all rights reserved.

*/
#if defined(_MSC_VER) && !defined(__MWERKS__) 
#pragma warning(disable:4229)		// ignore anachronism used: modifiers on data are ignored
#endif

#include <MacTypes.h>
#include <Errors.h>
#include <Endian.h>
#include <MacMemory.h>
#include <Resources.h>
#include <Components.h>
#include <Sound.h>
#include <MoviesFormat.h>

#if !TARGET_OS_MAC
#include "QTML.h"
#endif

#include "uLawCodec.h"


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//			Constants
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

#define kInputFormat		kCodecFormat				/* required input format */
#define kOutputFormat		k16BitNativeEndianFormat	/* decompressors must output native samples */

enum {
	kOutputSampleSize		= 16,							/* size of output samples */
	kOutputBufferBytes		= (kMaxOutputSamples * (kOutputSampleSize/8) * 2)		/* room for 16-bit stereo data */
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
	ComponentInstance	sourceComponent;	/* who to call for more source data */
	SoundComponentDataPtr sourceDataPtr;	/* pointer to source data descriptor */
	SoundComponentData	outputData;			/* local data descriptor record */
	SoundSource			sourceID;
	long				maxFrames;			/* max frames we can output, considering stereo */
	long				outputBufferSamples; /* no. samples in output buffer */
	CompressionInfo		compInfo;			/* info about compressor */
	Handle				ulawTableHandle;	/* resource handle to ulaw table */
	short				*ulawTable;			/* pointer to ulaw table */
	Byte				buffer[kOutputBufferBytes];	/* buffer space */

#if !TARGET_OS_MAC
	QTMLMutex			interruptMutex;		/* prevent (if necessary) the app and interrupt killing eachother */
#endif
} ULawDecompGlobals, *ULawDecompGlobalsPtr;

typedef struct {
	AudioFormatAtom		formatData;
	AudioEndianAtom		endianData;
	AudioTerminatorAtom	terminatorData;
} AudioDecompressionAtom, *AudioDecompressionAtomPtr, **AudioDecompressionAtomHandle;

#if PRAGMA_STRUCT_ALIGN
	#pragma options align=reset
#elif PRAGMA_STRUCT_PACKPUSH
	#pragma pack(pop)
#elif PRAGMA_STRUCT_PACK
	#pragma pack()
#endif

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//			prototypes
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

static OSErr				GetDecompressionParams(ULawDecompGlobalsPtr globals, AudioDecompressionAtomHandle *params);
static OSErr				SetDecompressionParams(ULawDecompGlobalsPtr globals, UserDataAtom *atom);
static ComponentResult		PrimeSource(ULawDecompGlobalsPtr globals);
static unsigned long		SamplesToBytes(unsigned long sampleCount,  CompressionInfoPtr compInfo);
static unsigned long		BytesToSamples(unsigned long byteCount,  CompressionInfoPtr compInfo);
static unsigned long		SamplesToFrames(unsigned long sampleCount,  CompressionInfoPtr compInfo);
static unsigned long		FramesToSamples(unsigned long framesCount,  CompressionInfoPtr compInfo);

static void					InitializeCompressionInfo(ULawDecompGlobalsPtr globals);
static OSErr				DecompressorGetInfo(ULawDecompGlobalsPtr globals, OSType selector, void *infoPtr);
static OSErr				DecompressorSetInfo(ULawDecompGlobalsPtr globals, OSType selector, void *infoPtr);
static OSErr				SetCompressorInfo(ULawDecompGlobalsPtr globals, CompressionInfoPtr cp);
static void					ExpandULaw(Byte *inbuf, short *outbuf, short *ulawTable, long framesToConvert, short numChannels, short whichChannel, short sampleSize);

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//			Component Dispatcher
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

#if TARGET_CPU_68K
	#define COMPONENT_C_DISPATCHER
	#define COMPONENT_DISPATCH_MAIN
#endif

#define CALLCOMPONENT_BASENAME() 		__uLawDecomp
#define CALLCOMPONENT_GLOBALS() 		ULawDecompGlobalsPtr storage

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
static pascal ComponentResult __uLawDecompOpen(ULawDecompGlobalsPtr globals, ComponentInstance self)
{
	OSErr						result;

	globals = (ULawDecompGlobalsPtr)NewPtrSysClear(sizeof(ULawDecompGlobals));
	FailWithAction(globals == nil, result = MemError(), Failure);

	result = GetComponentResource((Component)self, k16BitTableResType, kSoundDecompressorResID, &globals->ulawTableHandle);
	FailIf(result != noErr, NoResource);
	
	HLock(globals->ulawTableHandle);
	globals->ulawTable = (short *)StripAddress(*globals->ulawTableHandle);

#if TARGET_RT_LITTLE_ENDIAN
	// The ulawTable values are stored in the resource fork as 16-bit big-endian data
	// (Rez assumes any integers in a .r file are big-endian, for compatibility).
	// Make them little-endian, so we can do math with them on this little-endian machine.
	// There are ways to get the Resource Manager to do this for you by registering
	// a callback for the resource type, but that's beyond the scope of this example.
	{
		long numElements = GetHandleSize(globals->ulawTableHandle) / 2;
		long i;
		for (i = 0; i < numElements; i++) {
			globals->ulawTable[i] = EndianU16_BtoN(globals->ulawTable[i]);
		}
	}
#endif

	globals->outputData.format = kOutputFormat;			// output format
	globals->outputData.sampleSize = kOutputSampleSize;	// output sample size

	globals->outputBufferSamples = kMaxOutputSamples;	// will be set to requested->sampleCount later

	InitializeCompressionInfo(globals);

	// set our storage pointer to our globals
	SetComponentInstanceStorage(self, (Handle) globals);

#if !TARGET_OS_MAC
	globals->interruptMutex = QTMLCreateMutex();
	FailWithAction(globals->interruptMutex == nil, result = memFullErr, NoMutex);
#endif

	return (noErr);
	
NoMutex:
	DisposeHandle(globals->ulawTableHandle);
NoResource:
	DisposePtr((Ptr)globals);
Failure:
	return (result);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
static pascal ComponentResult __uLawDecompClose(ULawDecompGlobalsPtr globals, ComponentInstance self)
{
	ComponentResult		result;

	self; // suppress "unused variable" warning for all compilers

	if (globals != nil)											// we have some globals
	{
		if (globals->sourceComponent)
		{
			result = CloseComponent(globals->sourceComponent);	// torch source component
			FailMessage(result != noErr);
		}
		globals->outputData.sampleCount = 0;					// nothing in our buffer now

#if !TARGET_OS_MAC
		if (globals->interruptMutex != nil) 
		{
			QTMLDestroyMutex(globals->interruptMutex);
			globals->interruptMutex = nil;
		}
#endif
		if (globals->ulawTableHandle != nil)
			DisposeHandle(globals->ulawTableHandle);
		DisposePtr((Ptr)globals);								// torch them
	}

	return (noErr);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
pascal ComponentResult __uLawDecompVersion(ULawDecompGlobalsPtr globals)
{
	globals; // suppress "unused variable" warning for all compilers

	return (kSoundDecompressorVersion);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Sound Component Methods
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
static pascal ComponentResult __uLawDecompSetSource(ULawDecompGlobalsPtr globals, SoundSource sourceID, ComponentInstance source)
{
	SoundComponentDataPtr	sourceData;

	globals->sourceID = sourceID;
	globals->sourceComponent = source;					// our food source
	globals->sourceDataPtr = nil;						// nothing read from source yet

	// make sure we can get the source we need
	return (SoundComponentSetOutput(source, &globals->outputData, &sourceData));
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
static pascal ComponentResult __uLawDecompSetOutput(ULawDecompGlobalsPtr globals, SoundComponentDataPtr requested, SoundComponentDataPtr *actual)
{
	globals->outputBufferSamples = requested->sampleCount;				// no. samples to output
	if (globals->outputBufferSamples > kMaxOutputSamples)				// too much for our buffer
		globals->outputBufferSamples = kMaxOutputSamples;

	// must be one of the output formats we support
	FailIf((requested->sampleSize == 8) && (requested->format != k8BitOffsetBinaryFormat), Failure);

    // decompressors output to native endian by definition
    FailIf((requested->sampleSize == 16) && (requested->format != k16BitNativeEndianFormat), Failure);

	globals->outputData = *requested;
	return (noErr);

Failure:
	// here's what we can do
	*actual = &globals->outputData;
	return (paramErr);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
static pascal ComponentResult __uLawDecompGetInfo(ULawDecompGlobalsPtr globals, SoundSource sourceID, OSType selector, void *infoPtr)
{
	ComponentResult		result;

	result = noErr;
	switch (selector)
	{
		case siCompressionFactor:
			*(CompressionInfoPtr)infoPtr = globals->compInfo;
			break;

		case siDecompressionParams:
			result = GetDecompressionParams(globals, (AudioDecompressionAtomHandle *)infoPtr);
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
static pascal ComponentResult __uLawDecompSetInfo(ULawDecompGlobalsPtr globals, SoundSource sourceID, OSType selector, void *infoPtr)
{
	ComponentResult		result;

	result = noErr;
	switch (selector)
	{
		case siDecompressionParams:
			result = SetDecompressionParams(globals, (UserDataAtom *)infoPtr);
			break;

		default:
			if (globals->sourceComponent == nil)
				result = siUnknownInfoType;
			else
				result = SoundComponentSetInfo(globals->sourceComponent, sourceID, selector, infoPtr);
	}
	return (result);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
static pascal ComponentResult __uLawDecompPlaySourceBuffer(ULawDecompGlobalsPtr globals, SoundSource sourceID, SoundParamBlockPtr pb, long actions)
{
	globals->sourceDataPtr = nil;						// no source yet
	globals->outputData.sampleCount = 0;				// our buffer is empty

	return (SoundComponentPlaySourceBuffer(globals->sourceComponent, sourceID, pb, actions));
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
static pascal ComponentResult __uLawDecompStopSource(ULawDecompGlobalsPtr globals, short count, SoundSource *sources)
{
	globals->sourceDataPtr = nil;						// assume our source is gone
	return (SoundComponentStopSource(globals->sourceComponent, count, sources));
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
static pascal ComponentResult __uLawDecompGetSourceData(ULawDecompGlobalsPtr globals, SoundComponentDataPtr *resultData)
{
	SoundComponentDataPtr sourceDataPtr;
	long				samplesConverted;
	long				framesToConvert;
	ComponentResult		result = noErr;
	Byte				*inputBuffer;

	if (globals->sourceDataPtr == nil)						// no source pointer so...
	{
		result = PrimeSource(globals);						// get data from our source
		FailIf(result != noErr, Exit);
	}

	sourceDataPtr = globals->sourceDataPtr;					// get pointer to source sound component
	if (sourceDataPtr->sampleCount == 0)					// source is out of bytes
	{														// get some more source
		result = SoundComponentGetSourceData(globals->sourceComponent, &globals->sourceDataPtr);
		FailIf(result != noErr, Exit);
		sourceDataPtr = globals->sourceDataPtr;				// get pointer to source sound component
	}

	if ((sourceDataPtr->format == globals->outputData.format) ||	// input and output are same
		(sourceDataPtr->buffer == nil) ||							// or no source buffer
		(sourceDataPtr->sampleCount == 0))							// or no source buffer
	{
		globals->sourceDataPtr = nil;						// get new source next time
		*resultData = sourceDataPtr;						// pass source on down
	}
	else
	{
		framesToConvert = SamplesToFrames(sourceDataPtr->sampleCount, &globals->compInfo);

		if (framesToConvert)								// source has some data for us
		{
			if (framesToConvert > globals->maxFrames)
				framesToConvert = globals->maxFrames;		// limited to size of output
			samplesConverted = FramesToSamples(framesToConvert, &globals->compInfo);

			inputBuffer = sourceDataPtr->buffer;			// point at input buffer
			sourceDataPtr->buffer += SamplesToBytes(samplesConverted, &globals->compInfo);	// update input buffer

			sourceDataPtr->sampleCount -= samplesConverted;

			if (sourceDataPtr->numChannels == 1)
			{
				ExpandULaw(inputBuffer, (short *) globals->buffer, globals->ulawTable,
							framesToConvert, 1, 1, globals->outputData.sampleSize);
			}
			else
			{
				// do the left samples
				ExpandULaw(inputBuffer, (short *) globals->buffer, globals->ulawTable,
							framesToConvert, 2, 1, globals->outputData.sampleSize);

				// do the right samples
				ExpandULaw(inputBuffer, (short *) globals->buffer, globals->ulawTable,
							framesToConvert, 2, 2, globals->outputData.sampleSize);
			}
		}
		else
			samplesConverted = 0;

		globals->outputData.buffer = globals->buffer;		// data in this buffer
		globals->outputData.sampleCount = samplesConverted;	// return num. samples converted

		*resultData = &globals->outputData;						// tell them what we made
	}

Exit:
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
static OSErr GetDecompressionParams(ULawDecompGlobalsPtr globals, AudioDecompressionAtomHandle *params)
{
	AudioDecompressionAtomHandle	atom;
	AudioDecompressionAtomPtr		atomPtr;
	OSErr							result;

	globals; // suppress "unused variable" warning for all compilers

	result = noErr;
	atom = (AudioDecompressionAtomHandle)NewHandle(sizeof(AudioDecompressionAtom));
	FailWithAction(atom == nil, result = MemError(), Exit);

	atomPtr = *atom;
	atomPtr->formatData.size = EndianU32_NtoB(sizeof(AudioFormatAtom));
	atomPtr->formatData.atomType = EndianU32_NtoB(kAudioFormatAtomType);
	atomPtr->formatData.format = EndianU32_NtoB(kInputFormat);

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
static OSErr SetDecompressionParams(ULawDecompGlobalsPtr globals, UserDataAtom *atom)
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
				FailWithAction(((AudioFormatAtom *)atom)->format != EndianU32_NtoB(kInputFormat), result = badFormat, Exit);
				break;

			case kAudioEndianAtomType:
#if !TARGET_OS_MAC
				// Don't do this during an interrupt
				QTMLGrabMutex(globals->interruptMutex);
#endif
				littleEndian = EndianU16_BtoN(((AudioEndianAtom *)atom)->littleEndian);
#if !TARGET_OS_MAC
				QTMLReturnMutex(globals->interruptMutex);
#endif
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
static ComponentResult PrimeSource(ULawDecompGlobalsPtr globals)
{
	ComponentResult			result = noErr;
	SoundComponentDataPtr	sourceDataPtr;

	result = SoundComponentGetSourceData(globals->sourceComponent, &globals->sourceDataPtr);
	FailIf(result != noErr, Exit);
	FailWithAction(globals->sourceDataPtr == nil, result = paramErr, Exit);

	sourceDataPtr = globals->sourceDataPtr;
	globals->outputData.flags = sourceDataPtr->flags;				// copy flags unchanged
	globals->outputData.sampleRate = sourceDataPtr->sampleRate;		// copy sample rate unchanged
	globals->outputData.numChannels = sourceDataPtr->numChannels;	// copy numchannels unchanged

	// update bytesPerFrame according to number of channels
	globals->compInfo.bytesPerFrame = globals->compInfo.bytesPerPacket * sourceDataPtr->numChannels;
	globals->maxFrames = SamplesToFrames(globals->outputBufferSamples, &globals->compInfo);

Exit:
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
static void InitializeCompressionInfo(ULawDecompGlobalsPtr globals)
{
	globals->compInfo.recordSize = sizeof(CompressionInfo);
	globals->compInfo.format = kInputFormat;
	globals->compInfo.compressionID = fixedCompression;
	globals->compInfo.samplesPerPacket = kULawBlockSamples;
	globals->compInfo.bytesPerPacket = kULawBlockBytes;
	globals->compInfo.bytesPerSample = kULawBytesPerSample;
	globals->compInfo.bytesPerFrame = kULawBlockBytes;
	globals->compInfo.futureUse1 = 0;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//			µLaw 2:1 Decompression
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#if TARGET_RT_BIG_ENDIAN
	#define GetEndianHighByte(p) (*(Byte *)(&p))
#else
	#define GetEndianHighByte(p) (*((Byte *)(&p)+1))
#endif

void ExpandULaw(Byte *inbuf, short *outbuf, short *ulawTable, 
				long framesToConvert, short numChannels, short whichChannel, short sampleSize)
{
	short		i;

	whichChannel -= 1;
	inbuf += whichChannel;

	if (sampleSize == 8)
	{
		Byte	*outbuf8 = (Byte *) outbuf;
		Byte	b, toggle = 0x80;

		if (numChannels == 1)
		{
			for (i = framesToConvert - 1; i >= 0; --i)
			{
				b = GetEndianHighByte(ulawTable[*inbuf++]);
				b ^= toggle;
				*outbuf8++ = b;
			}
		}
		else
		{
			outbuf8 += whichChannel;

			for (i = framesToConvert - 1; i >= 0; --i)
			{
				b = GetEndianHighByte(ulawTable[*inbuf]);
				b ^= toggle;
				*outbuf8 = b;
				inbuf += 2;
				outbuf8 += 2;
			}
		}
	}
	else
	{
		if (numChannels == 1)
		{
			for (i = framesToConvert - 1; i >= 0; --i)
			{
				*outbuf++ = ulawTable[*inbuf++];
			}
		}
		else
		{
			outbuf += whichChannel;

			for (i = framesToConvert - 1; i >= 0; --i)
			{
				*outbuf = ulawTable[*inbuf];
				inbuf += 2;
				outbuf += 2;
			}
		}
	}
}
