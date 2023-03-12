#ifndef __AUDIODEFS__
#define __AUDIODEFS__

#if PRAGMA_STRUCT_ALIGN
	#pragma options align=mac68k
#elif PRAGMA_STRUCT_PACKPUSH
	#pragma pack(push, 2)
#elif PRAGMA_STRUCT_PACK
	#pragma pack(2)
#endif

typedef struct {
    AudioFormatAtom		formatData;
    AudioEndianAtom		endianData;
    AudioTerminatorAtom		terminatorData;
} AudioCompressionAtom, *AudioCompressionAtomPtr, **AudioCompressionAtomHandle;

#if PRAGMA_STRUCT_ALIGN
	#pragma options align=reset
#elif PRAGMA_STRUCT_PACKPUSH
	#pragma pack(pop)
#elif PRAGMA_STRUCT_PACK
	#pragma pack()
#endif

#define BailErr(x) {err = x; if(err != noErr) goto bail;}

enum {kMaxOutputBuffer = 64 * 1024}; // max size of output buffer

typedef enum { kFirstBuffer, kSecondBuffer } eBufferNumber;

typedef struct {
	ExtendedSoundComponentData	compData;
	Handle				hSource;			// source media buffer
	Media				sourceMedia;		// sound media identifier
	TimeValue			getMediaAtThisTime;
	TimeValue			sourceDuration;
	UInt32				maxBufferSize;
	Boolean				isThereMoreSource;
	Boolean				isSourceVBR;
} SCFillBufferData, *SCFillBufferDataPtr;

// functions
OSErr PlaySound(const FSSpec *inFileToPlay);

#endif // __AUDIODEFS__