//////////
//
//	File:		AIFFWriter.c
//
//	Contains:	Header for dispatching a sound hardware output ('sdev') component.
//
//	Written by:	Mark Cookson, Apple Developer Technical Support
//				Based on code by Kip Olson.
//	Revised by: Tim Monroe
//
//	Copyright:	� 1993-1999 by Apple Computer, Inc., all rights reserved.
//
//	Change History (most recent first):
//
//	   <2>	 	03/05/99	rtm		further work to bring into line with existing component sample code
//	   <1>	 	02/25/99	rtm		first inherited file; changed some compiler conditionals
//									to support Windows compiles
//
//	You may incorporate this sample code into your applications without
//	restriction, though the sample code has been provided "AS IS" and the
//	responsibility for its operation is 100% yours. However, what you are
//	not permitted to do is to redistribute the source as "Apple Sample
//	Code" after having made changes. If you're going to re-distribute the
//	source, we require that you make it clear in the source that the code
//	was descended from Apple Sample Code, but that you've made changes.
//
//////////

#ifndef __AIFFWRITER__
#define __AIFFWRITER__


//////////
//
// header files
//
//////////

#ifndef __COMPONENTS__
#include <Components.h>
#endif

#ifndef __MACMEMORY__
#include <MacMemory.h>
#endif

#ifndef __MACERRORS__
#include <MacErrors.h>
#endif

#ifndef __SOUND__
#include <Sound.h>
#endif

#ifndef __TIMER__
#include <Timer.h>
#endif

#ifndef __FIXMATH__
#include <FixMath.h>
#endif

#ifndef __SCRIPT__
#include <Script.h>
#endif


//////////
//
// compiler flags
//
//////////

#define FakeInterrupts					1


//////////
//
// constants
//
//////////

#define kSoundComponentVersion			0x00010000				// version for this component
#define kSndFileName					"\pAIFF Writer output"	// name for sound file data is written to
#define kAIFFWriterName					"\pAIFF Writer"

#define kSecondsInIOBuffer				1						// number of seconds of audio data in each I/O buffer

#define kAIFFWriterSifterID				128
#define kAIFFWriterPanelID				130
#define kAIFFWriterSubType				'hack'					// OS Type for component
#define kAIFFCreator					FOUR_CHAR_CODE('TVOD')
#define kAIFFFileType					FOUR_CHAR_CODE('AIFF')

enum {
	kHardwareVolumeSteps 				= 16,							// levels of volume adjustments
	kHardwareSampleCount 				= 1024,							// number of samples in hardware buffer
	kHardwareByteSize 					= kHardwareSampleCount * 2 * 2	// enough for 16 bit stereo
};


//////////
//
// data types
//
//////////

#if PRAGMA_STRUCT_ALIGN
	#pragma options align=mac68k
#elif PRAGMA_STRUCT_PACKPUSH
	#pragma pack(push, 2)
#elif PRAGMA_STRUCT_PACK
	#pragma pack(2)
#endif

typedef struct myTMTask myTMTask;
typedef struct myTMTask *myTMTaskPtr;

typedef struct SoundOutputGlobals SoundOutputGlobals;
typedef struct SoundOutputGlobals *SoundOutputGlobalsPtr;

// a Time Manager task record that includes space for my globals at the end
struct myTMTask {
	TMTask							task;
	SoundOutputGlobalsPtr			globals;
};

// i/o buffer structure
typedef struct IOBuffer {
	ParamBlockRec					iopb;						// i/o parameter block
	long							byteCount;					// no. bytes in buffer
	Ptr								buffer;						// buffer for data
} IOBuffer, *IOBufferPtr;

typedef struct HardwarePreferences {
	long							size;						// size of current preferences
	Fixed							version;					// 16.16 value for version
	unsigned long					volume;						// current hardware volume
	UnsignedFixed					sampleRate;					// current sample rate
	short							sampleSize;					// current sample size
	short							numChannels;				// current num channels
	short							muteState;					// current hardware mute state
	short							sampleCount;				// size of interrupt buffer in samples
} HardwarePreferences, *HardwarePrefsPtr, **HardwarePrefsHandle;

struct SoundOutputGlobals {
	// component stuff
	ComponentInstance				sourceComponent;			// component to call when hardware needs more data
	SoundComponentDataPtr			sourceDataPtr;				// pointer to source data structure
	SoundComponentData				outputData;
	ComponentInstance				self;						// our component's instance

	// fields specific to this component implementation
	HardwarePrefsHandle				prefs;						// user's preferences
	Boolean							prefsChanged;				// true if prefs need to be saved to disk
	Boolean							hardwareOn;					// true if hardware is on, false if it is off
	Boolean							inAppHeap;					// true if loaded into an app's heap
	Boolean							padding1;					// padding to align struct
	unsigned long					ioBufferSize;				// size of i/o buffer in bytes
	long							interruptInterval;			// time in microseconds between interrupts
	myTMTask						tmTask;						// time manager task record used to trigger interrupt
	long							headerLen;					// length of AIFF header written to beginning of file
	short							fRefNum;					// fref of file to save data into
	short							currentIndex;				// current buffer to save data into
	IOBuffer						ioBuffers[2];				// buffer descriptions

#ifdef FakeInterrupts
	unsigned long					nextTime;					// next time to interrupt
#endif
};

#if PRAGMA_STRUCT_ALIGN
	#pragma options align=reset
#elif PRAGMA_STRUCT_PACKPUSH
	#pragma pack(pop)
#elif PRAGMA_STRUCT_PACK
	#pragma pack()
#endif


//////////
//
// function prototypes
//
//////////

OSErr					SetupHardware (SoundOutputGlobalsPtr globals);
void					ReleaseHardware (SoundOutputGlobalsPtr globals);
void					StartHardware (SoundOutputGlobalsPtr globals);
void					StopHardware (SoundOutputGlobalsPtr globals);
void					ResumeHardware (SoundOutputGlobalsPtr globals);
void					CopySamplesToHardware (SoundOutputGlobalsPtr globals, SoundComponentDataPtr siftPtr);
void					OutputToFile (SoundComponentDataPtr siftPtr, void *dest, long sampleCount);
Boolean					FakeInterrupt (void);
void					InterruptRoutine (SoundOutputGlobalsPtr globals);
SoundComponentDataPtr	GetMoreSource (SoundOutputGlobalsPtr globals);
OSErr					SetupOutputFile (SoundOutputGlobalsPtr globals);
OSErr					CloseOutputFile (SoundOutputGlobalsPtr globals);
OSErr					GetPreferences (SoundOutputGlobalsPtr globals);
OSErr					InitFromPreferences (SoundOutputGlobalsPtr globals);

//#if TARGET_CPU_68K
//void					TMInterrupt (myTMTaskPtr taskPtr:__a1);
//#else
void					TMInterrupt (myTMTaskPtr taskPtr);
//#endif

unsigned long			MicroSeconds (void) ONEWORDINLINE(0xA193);
extern Ptr				GetRegisterA1 (void) ONEWORDINLINE(0x2009);	// MOVE.L	A1,D0


//////////
//
// failure handling macros
//
//////////

/*
	Some macros used to check for errors and also to allow for
	handling them by using a goto statement.  This makes the source
	code easier to read.  It will break into the debugger with a
	message showing the condition that caused the failure.  In some
	of the macros the debug message is removed but goto remains.  In
	other macros all of it is removed when doing a non-debug build.

    Note that these macros use the "\p" construct for creating
	Pascal strings at compile time.  Most non-Mac compilers do
	not recognize this, give a warning, and put 'p' as the first
	character of the string.  You can ignore the warning because
	the non-Mac version of DebugStr deals with this just fine.
	For Microsoft's Visual C++, we suppress the warning below.
*/

#if defined(_MSC_VER) && !defined(__MWERKS__)
	// Visual C++ from Microsoft
	#pragma warning(disable:4129) 							// unrecognized character escape sequence
#endif

// This checks for the exception, and if true then goto handler
#ifdef _DEBUG
#define FailIf(cond, handler)								\
	if (cond) {												\
		DebugStr((ConstStr255Param)"\p"#cond " goto " #handler);	\
		goto handler;										\
	} else 0
#else
#define FailIf(cond, handler)								\
	if (cond) {												\
		goto handler;										\
	} else 0
#endif

// This checks for the exception, and if true do the action and goto handler
#ifdef _DEBUG
#define FailWithAction(cond, action, handler)				\
	if (cond) {												\
		DebugStr((ConstStr255Param)"\p"#cond " goto " #handler);	\
		{ action; }											\
		goto handler;										\
	} else 0
#else
#define FailWithAction(cond, action, handler)				\
	if (cond) {												\
		{ action; }											\
		goto handler;										\
	} else 0
#endif

// This will insert debugging code in the application to check conditions
// and displays the condition in the debugger if true.  This code is
// completely removed in non-debug builds.
#ifdef _DEBUG
#define FailMessage(cond)									\
	if (cond)												\
		DebugStr((ConstStr255Param)"\p"#cond);				\
	else 0
#else
#define FailMessage(cond)
#endif

// This allows you to test for the result of a condition (i.e. CloseComponent)
// and break if it returns a non zero result, otherwise it ignores the result.
// When a non-debug build is done, the result is ignored.
#ifdef _DEBUG
#define ErrorMessage(cond)									\
	if (cond)												\
		DebugStr((ConstStr255Param)"\p"#cond);				\
	else 0
#else
#define ErrorMessage(cond)		cond
#endif

// This will display a given message in the debugger; this code is completely
// removed in non-debug builds.
#ifdef _DEBUG
#define DebugMessage(s)			DebugString((ConstStr255Param)s)
#else
#define DebugMessage(s)
#endif


#endif // __AIFFWRITER__
