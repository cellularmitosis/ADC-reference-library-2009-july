//////////
//
//	File:		QTMusic.h
//
//	Contains:	QuickTime Music Architecture sample code.
//
//	Written by:	Tim Monroe
//				based largely on QTMusic code by David Van Brink (see develop, issue 23).
//
//	Copyright:	© 1997-1998 by Apple Computer, Inc., all rights reserved.
//
//	Change History (most recent first):
//
//	   <1>	 	11/14/97	rtm		first file; conversion to personal coding style
//	   
//////////

#include "ComApplication.h"
#include <QuickTimeMusic.h>

#if TARGET_OS_MAC
#include "MacFramework.h"
#endif

#if TARGET_OS_WIN32
#include "WinFramework.h"
#endif

//////////
//
// constants
//
//////////

#define kNoteRequestEventLength				((sizeof(NoteRequest)/sizeof(long)) + 2) 	// number of (32-bit) long words in a note request event
#define kMarkerEventLength					(1) 										// number of (32-bit) long words in a marker event
#define kGeneralEventLength					(2) 										// number of (32-bit) long words in a general event, minus its data
#define kNoteDuration						240											// in 600ths of a second
#define kRestDuration						300											// in 600ths of a second; tempo will be 120 bpm

// some constants for the instruments we'll use; a complete list is found in develop, issue 23, p. 9
#define kInst_AcousticGrandPiano			1
#define kInst_BrightAcousticPiano			2
#define kInst_ElectricGrandPiano			3
#define kInst_HammondOrgan					17
#define kInst_Violin						41
#define kInst_Viola							42
#define kInst_Custom						1											// number of our custom instrument

#define kCustomInstAtomID					11											// arbitrary ID of our custom instrument atom
#define kCustomInstPICTID					128											// ID of our picture for our custom instrument

#define kMIDINoteValue_Lowest				0
#define kMIDINoteValue_Highest				128

#define kMusicSavePrompt					"Save New Music Movie As:"
#define kMusicSaveMovieFileName				"music.mov"


//////////
//
// structures
//
//////////

typedef struct {
	NoteAllocator			fNoteAllocator;
	NoteChannel				fNoteChannel;
} MIDIInputExample;

// sound header data structure
typedef union {
	SoundHeader				s;								// plain sound header
	CmpSoundHeader			c;								// compressed sound header
	ExtSoundHeader			e;								// extended sound header
} CommonSoundHeader, *CommonSoundHeaderPtr;


//////////
//
// function prototypes
//
//////////

void						QTMusic_PlayRisingNotesOnChannel (NoteAllocator theNoteAllocator, NoteChannel theNoteChannel);
void						QTMusic_PlaySomeNotes (void);
void						QTMusic_PlayShepardMelody (void);
void						QTMusic_PickInstThenPlaySomeNotes (void);
void						QTMusic_PlaySomeBentNotes (void);
unsigned long *				QTMusic_BuildTuneHeader (long *theCount, AtomicInstrument theInstrument);
Handle						QTMusic_BuildTuneSequence (long *theDuration);
void						QTMusic_BuildSequenceAndPlay (void);
PASCAL_RTN ComponentResult	QTMusic_ReadHook (MusicMIDIPacket *theMIDIPacket, long theRefCon);
void						QTMusic_UseMIDIInput (void);
void						QTMusic_CreateMusicMovie (AtomicInstrument theInstrument);
AtomicInstrument			QTMusic_CreateAtomicInstFromResource (Handle theHandle);
void						QTMusic_PickCustomInstThenPlaySomeNotes (void);
void						QTMusic_AddCustomInstrumentToMovie (Movie theMovie);
Handle						QTMusic_PickSndResource (void);
short						QTMusic_GetSndResourceID (short theRefNum);
OSErr						QTMusic_ParseSndResource (Handle theHandle, SoundComponentData *theSndInfo, CompressionInfo *theCompInfo, unsigned long *theHeaderOffset, unsigned long *theDataOffset, long *theBaseFreq);
#if TARGET_OS_MAC
PASCAL_RTN Boolean			QTMusic_FilterFiles (AEDesc *theItem, void *theInfo, void *theCallBackUD, NavFilterModes theFilterMode);
#endif
#if TARGET_OS_WIN32
PASCAL_RTN Boolean			QTMusic_FilterFiles (CInfoPBPtr thePBPtr);
#endif
