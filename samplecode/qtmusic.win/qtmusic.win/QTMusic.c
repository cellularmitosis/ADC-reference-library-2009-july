//////////
//
//	File:		QTMusic.c
//
//	Contains:	QuickTime Music Architecture sample code.
//
//	Written by:	Tim Monroe
//				based largely on QTMusic code by David van Brink (see develop, issue 23).
//
//	Copyright:	© 1995-1998 by Apple Computer, Inc., all rights reserved.
//
//	Change History (most recent first):
//
//	   <13>	 	03/17/00	rtm		made changes to get things running under CarbonLib
//	   <12>	 	10/14/98	rtm		fixed bug in QTMusic_UseMIDIInput (duplicate call to NewMusicMIDIReadHookProc)
//	   <11>	 	09/30/98	rtm		tweaked call to AddMovieResource to create single-fork movies
//	   <10>	 	06/19/98	rtm		minor clean-up
//	   <9>	 	06/16/98	rtm		finished support for custom instruments in movies
//	   <8>	 	06/12/98	rtm		further work on custom instruments
//	   <7>	 	06/10/98	rtm		begun support for custom instruments; see NOTE (5) below
//	   <6>	 	02/04/98	rtm		changed QTMusic_PlayShepardMelody to stop after 30 notes
//									(previously, it required a button click to stop)
//	   <5>	 	11/20/97	rtm		minor clean-up; added some error checking
//	   <4>	 	11/19/97	rtm		fixed crashing bugs in QTMusic_BuildTuneSequence (do NOT use ++ operator
//									inside of the _Stuff macros, please...?); factored out QTMusic_PlayRisingNotesOnChannel
//	   <3>	 	11/18/97	rtm		reworked endian macros and #if's; see NOTE (2) below
//	   <2>	 	11/17/97	rtm		added endian macros and #if's; now compiles for and runs on Windows;
//									simplified QTMusic_CreateMusicMovie
//	   <1>	 	11/14/97	rtm		first file; conversion to personal coding style; updated to latest headers
//	 
//
// NOTES:
//
// *** (1) ***
// This code is based heavily on the code provided with the develop article on the QTMA by David van Brink
// (issue 23). Make sure to read that article for complete details on the techniques employed here. I have
// taken the liberty of reworking that code as necessary to make it run on Windows platforms and to bring it 
// into line with the other QuickTime code samples.
//
// *** (2) ***
// In several locations, we need to assign values to the fields of a NoteRequest structure. The header file
// defines those fields differently on big- and little-endian machines (mainly to keep you out of the debugger).
// We can assign values like this, in an endian-neutral manner:
//
//		*(short *)(&myNoteRequest.info.polyphony) = EndianS16_NtoB(2);				// simultaneous tones
//		*(Fixed *)(&myNoteRequest.info.typicalPolyphony) = EndianU32_NtoB(0x00010000);
//
// For the weak of heart (like me!), these two lines can be replaced by the following seven lines:
//
//	#if TARGET_RT_BIG_ENDIAN
//		myNoteRequest.info.polyphony = 2;											// simultaneous tones
//		myNoteRequest.info.typicalPolyphony = 0x00010000;
//	#else
//		myNoteRequest.info.polyphony.bigEndianValue = EndianS16_NtoB(2);			// simultaneous tones
//		myNoteRequest.info.typicalPolyphony.bigEndianValue = EndianU32_NtoB(0x00010000);
//	#endif
//
// *** (3) ***
// All data in tune headers and tune sequences must be big-endian. If you use macros like qtma_StuffNoteEvent
// to build a tune sequence, the data you pass to those macros is automatically converted to big-endian form.
// If you stuff data directly into a tune header or tune sequence, you must make sure you stuff in big-endian
// data. For that, you can use macros like EndianU32_NtoB.
//
// *** (4) ***
// The qtma_Stuff* macros do not allow increment or decrement operators to be used on arguments.
// You should NOT do this:
//
// 		qtma_StuffNoteEvent(*myPos++, 1, 60, 100, kNoteDuration);
//
// Instead, you can do this:
//
// 		qtma_StuffNoteEvent(*myPos, 1, 60, 100, kNoteDuration);
//		*myPos++;
//
// *** (5) ***
// QuickTime version 2.2 added the ability to create new instruments from sampled sounds. You can embed a
// custom instrument in a specific QuickTime movie, install it in the system folder (as a component of type
// 'inst'), or register it with QuickTime. This sample code illustrates how to add a custom instrument to a
// movie containing a music track and how to play a sequence of notes using a custom instrument.
// 
//////////

#include <FixMath.h>
#include <Movies.h>
#include <QuickTimeComponents.h>
#include <Script.h>
#include <Sound.h>
#include <StandardFile.h>
#include <stddef.h>

#include "QTMusic.h"


//////////
//
// QTMusic_PlayRisingNotesOnChannel
// Play some rising tones on the specified note channel.
//
//////////

void QTMusic_PlayRisingNotesOnChannel (NoteAllocator theNoteAllocator, NoteChannel theNoteChannel)
{
	unsigned long			myDelay;
	long					myIndex;
	
	if ((theNoteAllocator == NULL) || (theNoteChannel == NULL))					// just makin' sure....
		return;
		
	NAPlayNote(theNoteAllocator, theNoteChannel, kMiddleC, 80);					// middle C at velocity 80
	Delay(40, &myDelay);														// delay 2/3 of a second
	NAPlayNote(theNoteAllocator, theNoteChannel, kMiddleC, 0);					// middle C at velocity 0: end note
	Delay(40, &myDelay);														// delay 2/3 of a second

	// obligatory loop of rising tones
	for (myIndex = kMiddleC; myIndex <= 94; myIndex++) {
		NAPlayNote(theNoteAllocator, theNoteChannel, myIndex, 80);				// pitch myIndex at velocity 80
		NAPlayNote(theNoteAllocator, theNoteChannel, myIndex + 7, 80);			// pitch myIndex + 7 (musical fifth) at velocity 80
		Delay(10, &myDelay);													// delay 1/6 of a second
		NAPlayNote(theNoteAllocator, theNoteChannel, myIndex, 0);				// pitch myIndex at velocity 0: end note
		NAPlayNote(theNoteAllocator, theNoteChannel, myIndex + 7, 0);			// pitch myIndex + 7 at velocity 0: end note
	}
}


//////////
//
// QTMusic_PlaySomeNotes
// Play some notes using the note allocator component.
//
//////////

void QTMusic_PlaySomeNotes (void)
{
	NoteAllocator			myNoteAllocator;
	NoteChannel				myNoteChannel;
	NoteRequest				myNoteRequest;
	ComponentResult			myErr = noErr;

	myNoteAllocator = NULL;
	myNoteChannel = NULL;

	// open the note allocator component
	myNoteAllocator = OpenDefaultComponent(kNoteAllocatorComponentType, 0);
	if (myNoteAllocator == NULL)
		goto bail;

	// fill out a note request, using NAStuffToneDescription
	myNoteRequest.info.flags = 0;
	myNoteRequest.info.reserved = 0;
	*(short *)(&myNoteRequest.info.polyphony) = EndianS16_NtoB(2);				// simultaneous tones
	*(Fixed *)(&myNoteRequest.info.typicalPolyphony) = EndianU32_NtoB(0x00010000);

	myErr = NAStuffToneDescription(myNoteAllocator, kInst_AcousticGrandPiano, &myNoteRequest.tone);
	if (myErr != noErr)
		goto bail;
		
	// allocate a note channel
	myErr = NANewNoteChannel(myNoteAllocator, &myNoteRequest, &myNoteChannel);
	if ((myErr != noErr) || (myNoteChannel == NULL))
		goto bail;

	// if we've gotten this far, it's OK to play some musical notes; lovely
	QTMusic_PlayRisingNotesOnChannel(myNoteAllocator, myNoteChannel);
		
bail:
	if (myNoteChannel != NULL)
		NADisposeNoteChannel(myNoteAllocator, myNoteChannel);
		
	if (myNoteAllocator != NULL)
		CloseComponent(myNoteAllocator);
}


//////////
//
// QTMusic_PlayShepardMelody
// Play Roger Shepard's melody.
//
//////////

void QTMusic_PlayShepardMelody (void)
{
	NoteAllocator			myNoteAllocator;
	NoteChannel				myNoteChannel;
	NoteRequest				myNoteRequest;
	unsigned long			myDelay;
	long					myIndex1, myIndex2, myVeloc;
	ComponentResult			myErr = noErr;

	myNoteAllocator = NULL;
	myNoteChannel = NULL;

	// open the note allocator component
	myNoteAllocator = OpenDefaultComponent(kNoteAllocatorComponentType, 0);
	if (myNoteAllocator == NULL)
		goto bail;

	// fill out a note request, using NAStuffToneDescription
	myNoteRequest.info.flags = 0;
	myNoteRequest.info.reserved = 0;
	*(short *)(&myNoteRequest.info.polyphony) = EndianS16_NtoB(3);				// simultaneous tones
	*(Fixed *)(&myNoteRequest.info.typicalPolyphony) = EndianU32_NtoB(0x00010000);

	myErr = NAStuffToneDescription(myNoteAllocator, kInst_AcousticGrandPiano, &myNoteRequest.tone);
	if (myErr != noErr)
		goto bail;
		
	// allocate a note channel
	myErr = NANewNoteChannel(myNoteAllocator, &myNoteRequest, &myNoteChannel);
	if ((myErr != noErr) || (myNoteChannel == NULL))
		goto bail;
		
	// play Roger Shepard's melody
	myIndex1 = 0;
	while (myIndex1 < 30) {
		for (myIndex2 = myIndex1 % 13; myIndex2 < 128; myIndex2 += 13) {
			myVeloc = myIndex2 < 64 ? myIndex2 * 2 : (127 - myIndex2) * 2;
			NAPlayNote(myNoteAllocator, myNoteChannel, myIndex2, myVeloc);
		}
		
		Delay(13, &myDelay);
		
		for (myIndex2 = myIndex1 % 13; myIndex2 < 128; myIndex2 += 13)
			NAPlayNote(myNoteAllocator, myNoteChannel, myIndex2, 0);
			
		myIndex1++;
	}
		
bail:
	if (myNoteChannel != NULL)
		NADisposeNoteChannel(myNoteAllocator, myNoteChannel);
		
	if (myNoteAllocator != NULL)
		CloseComponent(myNoteAllocator);
}


//////////
//
// QTMusic_PickInstThenPlaySomeNotes
// Let the user pick an instrument, then play some notes.
//
//////////

void QTMusic_PickInstThenPlaySomeNotes (void)
{
	NoteAllocator			myNoteAllocator;
	NoteChannel				myNoteChannel;
	NoteRequest				myNoteRequest;
	ComponentResult			myErr = noErr;

	myNoteAllocator = NULL;
	myNoteChannel = NULL;

	// open the note allocator component
	myNoteAllocator = OpenDefaultComponent(kNoteAllocatorComponentType, 0);
	if (myNoteAllocator == NULL)
		goto bail;

	// fill out a note request, using NAStuffToneDescription
	myNoteRequest.info.flags = 0;
	myNoteRequest.info.reserved = 0;
	*(short *)(&myNoteRequest.info.polyphony) = EndianS16_NtoB(2);				// simultaneous tones
	*(Fixed *)(&myNoteRequest.info.typicalPolyphony) = EndianU32_NtoB(0x00010000);

	myErr = NAStuffToneDescription(myNoteAllocator, kInst_AcousticGrandPiano, &myNoteRequest.tone);
	if (myErr != noErr)
		goto bail;
		
	// display the instrument picker dialog box to elicit an instrument from the user
	myErr = NAPickInstrument(myNoteAllocator, NULL, "\pPick An Instrument:", &myNoteRequest.tone, 0, 0, 0, 0);
	if (myErr != noErr)
		goto bail;

	// allocate a note channel
	myErr = NANewNoteChannel(myNoteAllocator, &myNoteRequest, &myNoteChannel);
	if ((myErr != noErr) || (myNoteChannel == NULL))
		goto bail;

	// if we've gotten this far, it's OK to play some musical notes; lovely
	QTMusic_PlayRisingNotesOnChannel(myNoteAllocator, myNoteChannel);
			
bail:
	if (myNoteChannel != NULL)
		NADisposeNoteChannel(myNoteAllocator, myNoteChannel);
		
	if (myNoteAllocator != NULL)
		CloseComponent(myNoteAllocator);
}


//////////
//
// QTMusic_PlaySomeBentNotes
// Play some bent notes.
//
//////////

void QTMusic_PlaySomeBentNotes (void)
{
	NoteAllocator			myNoteAllocator;
	NoteChannel				myNoteChannel;
	NoteRequest				myNoteRequest;
	unsigned long			myDelay;
	long					myIndex;
	ComponentResult			myErr = noErr;

	myNoteAllocator = NULL;
	myNoteChannel = NULL;

	// open the note allocator component
	myNoteAllocator = OpenDefaultComponent(kNoteAllocatorComponentType, 0);
	if (myNoteAllocator == NULL)
		goto bail;

	// fill out a note request, using NAStuffToneDescription
	myNoteRequest.info.flags = 0;
	myNoteRequest.info.reserved = 0;
	*(short *)(&myNoteRequest.info.polyphony) = EndianS16_NtoB(2);				// simultaneous tones
	*(Fixed *)(&myNoteRequest.info.typicalPolyphony) = EndianU32_NtoB(0x00010000);

	myErr = NAStuffToneDescription(myNoteAllocator, kInst_HammondOrgan, &myNoteRequest.tone);
	if (myErr != noErr)
		goto bail;
		
	// allocate a note channel
	myErr = NANewNoteChannel(myNoteAllocator, &myNoteRequest, &myNoteChannel);
	if ((myErr != noErr) || (myNoteChannel == NULL))
		goto bail;

	Delay(30, &myDelay);														// delay 1/2 of a second

	// if we've gotten this far, it's OK to play some musical notes; lovely
	NAPlayNote(myNoteAllocator, myNoteChannel, kMiddleC, 80);					// middle C at velocity 80
	NAPlayNote(myNoteAllocator, myNoteChannel, 67, 60);							// G at velocity 60

	Delay(30, &myDelay);														// delay 1/2 of a second

	// loop through differing pitch bendings
	for (myIndex = 0; myIndex <= 0x300; myIndex += 10) {						// bend 3 semitones
		NASetController(myNoteAllocator, myNoteChannel, kControllerPitchBend, myIndex);
		Delay(1, &myDelay);
	}
	
	Delay(30, &myDelay);														// delay 1/2 of a second
	
	for (myIndex = 768; myIndex >= 0; myIndex -= 10) {							// bend back to normal
		NASetController(myNoteAllocator, myNoteChannel, kControllerPitchBend, myIndex);
		Delay(1, &myDelay);
	}
	
	Delay(30, &myDelay);														// delay 1/2 of a second

	NAPlayNote(myNoteAllocator, myNoteChannel, 60, 0);							// middle C off
	NAPlayNote(myNoteAllocator, myNoteChannel, 67, 0);							// G off

bail:
	if (myNoteChannel != NULL)
		NADisposeNoteChannel(myNoteAllocator, myNoteChannel);
		
	if (myNoteAllocator != NULL)
		CloseComponent(myNoteAllocator);
}


//////////
//
// QTMusic_BuildTuneHeader
// Build a tune header and return, in the theCount parameter, the number of long words
// in the new tune header (*including* the marker event). The caller is responsible for
// disposing of the tune header (by calling DisposePtr).
//
// A tune header is a block of memory containing one or more music events, followed by a
// 32-bit marker event (whose value is the constant kEndMarkerValue). A note request event
// is simply a NoteRequest structure, sandwiched between two long words. The first long word
// is of the form 0xfnnn0017, where nnn is a part number. The last long word is 0xc0010017.
// (0x0017 is the length, in long words, of the NoteRequest structure and the first and last
// message long words.)
//
// If the theInstrument parameter is non-NULL, stuff that custom instrument in the generated
// tune header instead of the second note request event. Here we'll use an atomic instrument
// general event, which is the instrument's atom container sandwiched between two long words.
// The first long word is of the form 0xfnnnmmmm, where nnn is a part number and mmmm is the
// number of long words in the entire general event. The last long word is of the form 0xc006mmmm.
//
// You can use the macro qtma_StuffGeneralEvent (defined in QuickTimeMusic.h) to assist you
// in building a tune header.
//
// Remember that all data in a tune header must be big-endian.
//
//////////

unsigned long *QTMusic_BuildTuneHeader (long *theCount, AtomicInstrument theInstrument)
{
	unsigned long			*myHeader;
	unsigned long			*myPos1, *myPos2;		// pointers to the head and tail long words of a music event
	NoteRequest				*myNoteRequest;
	NoteAllocator			myNoteAllocator;		// for the NAStuffToneDescription call
	long					myLongsInInst = 0L;		// the number of long words occupied by the atomic instrument
	ComponentResult			myErr = noErr;

	myHeader = NULL;
	myNoteAllocator = NULL;

	// open the note allocator component
	myNoteAllocator = OpenDefaultComponent(kNoteAllocatorComponentType, 0);
	if (myNoteAllocator == NULL)
		goto bail;

	// if we have an atomic instrument to install in the tune header,
	// figure out how many full (32-bit) long words its data occupies
	if (theInstrument != NULL) {
		myLongsInInst = GetHandleSize(theInstrument) / sizeof(long);
		if ((GetHandleSize(theInstrument) % sizeof(long)) != 0)
			myLongsInInst++;
	}
	
	// allocate space for the tune header;
	// in this example, we allocate space for *two* note request events and the terminating marker event
	// (if theInstrument is NULL) or we allocate space for *one* note request event, *one* general event,
	// and the terminating marker event (if theInstrument is non-NULL);
	if (theInstrument == NULL)
		// *two* note request events and the terminating marker event
		myHeader = (unsigned long *)NewPtrClear(((2 * kNoteRequestEventLength) + kMarkerEventLength) * sizeof(long));
	else
		// *one* note request event, the terminating marker event, and a general event that contains theInstrument
		myHeader = (unsigned long *)NewPtrClear((kNoteRequestEventLength + kMarkerEventLength + kGeneralEventLength + myLongsInInst) * sizeof(long));
	
	if (myHeader == NULL)
		goto bail;

	myPos1 = myHeader;

	// stuff request for piano, polyphony 4
	myPos2 = myPos1 + (kNoteRequestEventLength - 1);							// last longword of general event
	qtma_StuffGeneralEvent(*myPos1, *myPos2, 1, kGeneralEventNoteRequest, kNoteRequestEventLength);
	myNoteRequest = (NoteRequest *)(myPos1 + 1);

	myNoteRequest->info.flags = 0;
	myNoteRequest->info.reserved = 0;
	*(short *)(&myNoteRequest->info.polyphony) = EndianS16_NtoB(4);				// simultaneous tones
	*(Fixed *)(&myNoteRequest->info.typicalPolyphony) = EndianU32_NtoB(0x00010000);

	myErr = NAStuffToneDescription(myNoteAllocator, kInst_AcousticGrandPiano, &myNoteRequest->tone);
	if (myErr != noErr)
		goto bail;
		
	// move pointer to beginning of next event
	myPos1 += kNoteRequestEventLength;

	if (theInstrument == NULL) {
		// stuff request for violin, polyphony 3
		myPos2 = myPos1 + (kNoteRequestEventLength - 1);						// last longword of general event
		qtma_StuffGeneralEvent(*myPos1, *myPos2, 2, kGeneralEventNoteRequest, kNoteRequestEventLength);
		myNoteRequest = (NoteRequest *)(myPos1 + 1);

		myNoteRequest->info.flags = 0;
		myNoteRequest->info.reserved = 0;
		*(short *)(&myNoteRequest->info.polyphony) = EndianS16_NtoB(3);			// simultaneous tones
		*(Fixed *)(&myNoteRequest->info.typicalPolyphony) = EndianU32_NtoB(0x00010000);

		myErr = NAStuffToneDescription(myNoteAllocator, kInst_Violin, &myNoteRequest->tone);
		if (myErr != noErr)
			goto bail;
			
		// move pointer to beginning of marker event
		myPos1 += kNoteRequestEventLength;
	} else {
		// stuff the atomic instrument info
		myPos2 = myPos1 + (myLongsInInst + kGeneralEventLength - 1);			// last longword of general event
		qtma_StuffGeneralEvent(*myPos1, *myPos2, 2, kGeneralEventAtomicInstrument, kGeneralEventLength + myLongsInInst);
		
		// copy the instrument data into the tune header
		BlockMove(*theInstrument, myPos1 + 1, GetHandleSize(theInstrument));
		
		// move pointer to beginning of marker event
		myPos1 += myLongsInInst + kGeneralEventLength;
	}
		
	// set value of marker event
	*myPos1 = kEndMarkerValue;													// end of header marker event

bail:
	if (myNoteAllocator != NULL)
		CloseComponent(myNoteAllocator);

	if (theCount != NULL) {
		if (theInstrument != NULL)
			*theCount = kNoteRequestEventLength + (myLongsInInst + kGeneralEventLength) + kMarkerEventLength;
		else
			*theCount = (2 * kNoteRequestEventLength) + kMarkerEventLength;
	}

	// if we encountered an error, dispose of the storage we allocated and return NULL
	if (myErr != noErr) {
		DisposePtr((Ptr)myHeader);
		myHeader = NULL;
	}
	
	return(myHeader);
}


//////////
//
// QTMusic_BuildTuneSequence
// Build a tune sequence. The caller is responsible for disposing of the tune sequence
// (by calling DisposeHandle).
//
// A tune sequence is a block of memory containing note events and rest events,
// followed by a 32-bit marker event (whose value is the constant kEndMarkerValue).
//
// Remember that all data in a tune sequence must be big-endian.
//
//////////

Handle QTMusic_BuildTuneSequence (long *theDuration)
{
	unsigned long 			*mySequence;
	unsigned long 			*myPos;
	Handle					myHandle;

	// allocate space for the tune header
	myHandle = NewHandleClear(22 * sizeof(long));
	if (myHandle == NULL)
		goto bail;
		
	HLock(myHandle);
	mySequence = (unsigned long *)*myHandle;

	myPos = mySequence;

	// *** do NOT attempt to do the following:
	// ***
	// ***	qtma_StuffNoteEvent(*myPos++, 1, 60, 100, kNoteDuration);
	// ***
	// *** your application will die a horrible death if you do....
	
	qtma_StuffNoteEvent(*myPos, 1, 60, 100, kNoteDuration);						// piano C
	myPos++;
	qtma_StuffRestEvent(*myPos, kRestDuration);
	myPos++;
	qtma_StuffNoteEvent(*myPos, 2, 60, 100, kNoteDuration);						// violin C
	myPos++;
	qtma_StuffRestEvent(*myPos, kRestDuration);
	myPos++;

	qtma_StuffNoteEvent(*myPos, 1, 63, 100, kNoteDuration);						// piano
	myPos++;
	qtma_StuffRestEvent(*myPos, kRestDuration);
	myPos++;
	qtma_StuffNoteEvent(*myPos, 2, 64, 100, kNoteDuration);						// violin
	myPos++;
	qtma_StuffRestEvent(*myPos, kRestDuration);
	myPos++;

	// make the 5th and 6th notes much softer, just for fun
	qtma_StuffNoteEvent(*myPos, 1, 67, 60, kNoteDuration);						// piano
	myPos++;
	qtma_StuffRestEvent(*myPos, kRestDuration);
	myPos++;
	qtma_StuffNoteEvent(*myPos, 2, 66, 60, kNoteDuration);						// violin
	myPos++;
	qtma_StuffRestEvent(*myPos, kRestDuration);
	myPos++;

	qtma_StuffNoteEvent(*myPos, 1, 72, 100, kNoteDuration);						// piano
	myPos++;
	qtma_StuffRestEvent(*myPos, kRestDuration);
	myPos++;
	qtma_StuffNoteEvent(*myPos, 2, 73, 100, kNoteDuration);						// violin
	myPos++;
	qtma_StuffRestEvent(*myPos, kRestDuration);
	myPos++;

	qtma_StuffNoteEvent(*myPos, 1, 60, 100, kNoteDuration);						// piano
	myPos++;
	qtma_StuffNoteEvent(*myPos, 1, 67, 100, kNoteDuration);						// piano
	myPos++;
	qtma_StuffNoteEvent(*myPos, 2, 63, 100, kNoteDuration);						// violin
	myPos++;
	qtma_StuffNoteEvent(*myPos, 2, 72, 100, kNoteDuration);						// violin
	myPos++;
	qtma_StuffRestEvent(*myPos, kRestDuration);
	myPos++;

	*myPos = kEndMarkerValue;													// end marker

bail:
	if (theDuration)
		*theDuration = 9 * kRestDuration;
		
	return(myHandle);
}


//////////
//
// QTMusic_BuildSequenceAndPlay
// Build a tune sequence and play it.
//
//////////

void QTMusic_BuildSequenceAndPlay (void)
{
	Handle					mySequence;
	unsigned long			*myHeader;
	TunePlayer				myTunePlayer;
	TuneStatus				myTuneStatus;
	unsigned long			myDelay;
	ComponentResult			myErr = noErr;

	myTunePlayer = NULL;

	myHeader = QTMusic_BuildTuneHeader(NULL, NULL);
	mySequence = QTMusic_BuildTuneSequence(NULL);

	if ((mySequence == NULL) || (myHeader == NULL))
		goto bail;

	// open the tune player component
	myTunePlayer = OpenDefaultComponent(kTunePlayerComponentType, 0);
	if (myTunePlayer == NULL)
		goto bail;

	myErr = TuneSetHeader(myTunePlayer, myHeader);
	if (myErr != noErr)
		goto bail;
		
	Delay(10, &myDelay);
	
	myErr = TuneQueue(myTunePlayer, (unsigned long *)*mySequence, 0x00010000, 0, 0x7fffffff, 0, 0, 0);
	if (myErr != noErr)
		goto bail;

	// wait until the sequence finishes playing or the user clicks the mouse
spin:
	TuneGetStatus(myTunePlayer, &myTuneStatus);
	if (myTuneStatus.queueTime && !Button())
		goto spin;			// I like to use goto's primarily to shock the children

bail:
	if (myTunePlayer != NULL)
		CloseComponent(myTunePlayer);
		
	if (myHeader != NULL)
		DisposePtr((Ptr)myHeader);
		
	if (mySequence != NULL)
		DisposeHandle(mySequence);
}


//////////
//
// QTMusic_ReadHook
// Process a MIDI message.
//
//////////

PASCAL_RTN ComponentResult QTMusic_ReadHook (MusicMIDIPacket *theMIDIPacket, long theRefCon)
{
	MIDIInputExample		*myMIE;
	Boolean					isMajor;
	short					myStatus, myPitch, myVeloc;

	if (theRefCon == 0L)
		return(paramErr);

	myMIE = (MIDIInputExample *)theRefCon;

	if (theMIDIPacket->reserved == kMusicPacketPortLost)							// port gone? make channel quiet
		NASetNoteChannelVolume(myMIE->fNoteAllocator, myMIE->fNoteChannel, 0);
	else if (theMIDIPacket->reserved == kMusicPacketPortFound)						// port back? raise volume
		NASetNoteChannelVolume(myMIE->fNoteAllocator, myMIE->fNoteChannel, 0x00010000);
	else if (theMIDIPacket->length == 3) {
		myStatus = theMIDIPacket->data[0] & 0xF0;
		myPitch = theMIDIPacket->data[1];
		myVeloc = theMIDIPacket->data[2];
		
		switch (myStatus) {
			case 0x80:
				myVeloc = 0;
				// falls thru into case 0x90; almost as fun as a goto, eh mom?
			case 0x90:
				isMajor = (myPitch % 5 == 0);
				NAPlayNote(myMIE->fNoteAllocator, myMIE->fNoteChannel, myPitch, myVeloc);
				NAPlayNote(myMIE->fNoteAllocator, myMIE->fNoteChannel, myPitch + 3 + isMajor, myVeloc);
				NAPlayNote(myMIE->fNoteAllocator, myMIE->fNoteChannel, myPitch + 7, myVeloc);
				break;
		}
	}
		
	return(noErr);
}


//////////
//
// QTMusic_UseMIDIInput
// Read input from the default MIDI input device.
//
//////////

void QTMusic_UseMIDIInput (void)
{
	MIDIInputExample		myMIE;
	NoteRequest				myNoteRequest;
	MusicMIDIReadHookUPP	myReadHookUPP = NULL;
	ComponentResult			myErr = noErr;

	// open the note allocator component
	myMIE.fNoteAllocator = OpenDefaultComponent(kNoteAllocatorComponentType, 0);
	if (myMIE.fNoteAllocator == NULL)
		goto bail;

	myNoteRequest.info.flags = 0;
	myNoteRequest.info.reserved = 0;
	*(short *)(&myNoteRequest.info.polyphony) = EndianS16_NtoB(2);				// simultaneous tones
	*(Fixed *)(&myNoteRequest.info.typicalPolyphony) = EndianU32_NtoB(0x00010000);

	myErr = NAStuffToneDescription(myMIE.fNoteAllocator, kInst_AcousticGrandPiano, &myNoteRequest.tone);
	if (myErr != noErr)
		goto bail;
		
	// allocate a note channel
	myErr = NANewNoteChannel(myMIE.fNoteAllocator, &myNoteRequest, &myMIE.fNoteChannel);
	if (myErr != noErr)
		goto bail;
		
	myReadHookUPP = NewMusicMIDIReadHookUPP(QTMusic_ReadHook);
	NAUseDefaultMIDIInput(myMIE.fNoteAllocator, myReadHookUPP, (long)&myMIE, 0L);
	while (!Button())
		;
	NALoseDefaultMIDIInput(myMIE.fNoteAllocator);

bail:
	if (myReadHookUPP != NULL)
		DisposeMusicMIDIReadHookUPP(myReadHookUPP);
		
	if (myMIE.fNoteAllocator != NULL)
		CloseComponent(myMIE.fNoteAllocator);									// disposes note channel too
}


//////////
//
// QTMusic_CreateMusicMovie
// Create a movie containing the notes defined by QTMusic_BuildTuneSequence.
//
// Building a QuickTime music movie is just like building any QuickTime movie;
// the sample description is the tune header information, and the sample data
// is the tune sequence.
//
// If the theInstrument parameter is non-NULL, use that custom instrument in the
// generated movie. 
//
//////////

void QTMusic_CreateMusicMovie (AtomicInstrument theInstrument)
{
	Handle					myHandle = NULL;
	MusicDescriptionHandle	mySampleDesc = NULL;
	short					myResRefNum = 0;
	short					myResID = movieInDataForkResID;
	Movie					myMovie = NULL;
	Track					myTrack;
	Media					myMedia;
	FSSpec					myFile;
	Boolean					myIsSelected = false;
	Boolean					myIsReplacing = false;	
	StringPtr 				myPrompt = QTUtils_ConvertCToPascalString(kMusicSavePrompt);
	StringPtr 				myFileName = QTUtils_ConvertCToPascalString(kMusicSaveMovieFileName);
	long					myDuration, myHeaderLength;
	long					myFlags = createMovieFileDeleteCurFile | createMovieFileDontCreateResFile;
	unsigned long			*myHeader;
	ComponentResult			myErr = noErr;

	// create a music sample description
	myHeader = QTMusic_BuildTuneHeader(&myHeaderLength, theInstrument);
	if (myHeader == NULL)
		goto bail;

	mySampleDesc = (MusicDescriptionHandle)NewHandleClear(sizeof(MusicDescription) + (myHeaderLength * sizeof(long)));
	if (mySampleDesc == NULL)
		goto bail;

	// fill in the fields of the sample description
	(**mySampleDesc).descSize = GetHandleSize((Handle)mySampleDesc);
	(**mySampleDesc).dataFormat = kMusicComponentType;

	// copy the tune header into the sample description
	BlockMove(myHeader, (**mySampleDesc).headerData, myHeaderLength * sizeof(long));

	// get a tune
	myHandle = QTMusic_BuildTuneSequence(&myDuration);
	
	// prompt user for new file name
	QTFrame_PutFile(myPrompt, myFileName, &myFile, &myIsSelected, &myIsReplacing);
	if (!myIsSelected)
		goto bail;

	// create a movie file for the destination movie
	myErr = CreateMovieFile(&myFile, FOUR_CHAR_CODE('TVOD'), smCurrentScript, myFlags, &myResRefNum, &myMovie);
	if (myErr != noErr)
		goto bail;

	// create the movie track and media
	myTrack = NewMovieTrack(myMovie, 0, 0, kFullVolume);
	myMedia = NewTrackMedia(myTrack, MusicMediaType, 600, NULL, 0);

	// create the media sample
	BeginMediaEdits(myMedia);

	myErr = AddMediaSample(myMedia, myHandle, 0, GetHandleSize(myHandle), myDuration, (SampleDescriptionHandle)mySampleDesc, 1, 0, NULL);
	if (myErr != noErr)
		goto bail;

	EndMediaEdits(myMedia);
	
	// add the media to the track
	InsertMediaIntoTrack(myTrack, 0, 0, myDuration, fixed1);	
	AddMovieResource(myMovie, myResRefNum, &myResID, NULL);

bail:
	free(myPrompt);
	free(myFileName);

	if (mySampleDesc != NULL)
		DisposeHandle((Handle)mySampleDesc);
	
	if (myResRefNum != 0)
		CloseMovieFile(myResRefNum);

	if (myHeader != NULL)
		DisposePtr((Ptr)myHeader);
		
	if (myHandle != NULL)
		DisposeHandle(myHandle);

	if (myMovie != NULL)
		DisposeMovie(myMovie);
}


//////////
//
// QTMusic_CreateAtomicInstFromResource
// Create a custom atomic instrument using the sampled sound data in the specified resource handle.
//
// A custom instrument is defined by a QTAtom structure containing appropriate atoms (hence the name
// "atomic" instrument). See the QT3.0 book "QuickTime Music Architecture" for information about the
// structure of an atomic instrument. Page 15 has a good picture of the required structure.
//
//////////

AtomicInstrument QTMusic_CreateAtomicInstFromResource (Handle theHandle)
{
	SoundComponentData		mySndInfo;
	CompressionInfo			myCompInfo;
	unsigned long 			myHeaderOffset, myDataOffset;
	long					myBaseFreq = kMiddleC;		
	unsigned long			mySampleDataSize;
	unsigned long			mySampleDataPtr;
	AtomicInstrument		myInstrument = NULL;
	ToneDescription			myToneDesc;
	NoteRequestInfo			myNoteInfo;
	InstKnobList			myKnobList;
	QTAtom					myKeyRangeInfoAtom = 0;
	QTAtom					mySampleInfoAtom = 0;
	QTAtom					myInstInfoAtom = 0;
	Str31					myInstName = "\pMy custom instrument";
	OSErr					myErr = noErr;

	//////////
	//
	// get information about the sound; we'll use this later to construct a sample description atom
	//
	//////////
	
	myErr = QTMusic_ParseSndResource(theHandle, &mySndInfo, &myCompInfo, &myHeaderOffset, &myDataOffset, &myBaseFreq);
	if (myErr != noErr)
		goto bail;

	//////////
	//
	// create an atom container with atoms that describe the custom instrument
	//
	//////////
	
	// create a new, empty atom container
	myErr = QTNewAtomContainer(&myInstrument);
	if (myErr != noErr)
		goto bail;

	// insert a tone description atom, which contains a tone description structure
	*(OSType *)(&myToneDesc.synthesizerType) = EndianU32_NtoB(kSoftSynthComponentSubType);
	myToneDesc.synthesizerName[0] = 0;
	BlockMove(&myInstName, myToneDesc.instrumentName, myInstName[0] + 1);
	*(long *)(&myToneDesc.instrumentNumber) = EndianU32_NtoB(kInst_Custom);
	*(long *)(&myToneDesc.gmNumber) = 0;
	
	myErr = QTInsertChild(myInstrument, kParentAtomIsContainer, kaiToneDescType, 1, 1, sizeof(myToneDesc), &myToneDesc, NULL);
	if (myErr != noErr)
		goto bail;

	// insert a note request atom, which contains a note request structure;
	// this atom is optional; if it's not present, QTMA assumes some reasonable values
	myNoteInfo.flags = 0;
	myNoteInfo.reserved = 0;
	*(short *)(&myNoteInfo.polyphony) = EndianS16_NtoB(1);
	*(Fixed *)(&myNoteInfo.typicalPolyphony) = EndianU32_NtoB(0x00010000);
	
	myErr = QTInsertChild(myInstrument, kParentAtomIsContainer, kaiNoteRequestInfoType, 1, 1, sizeof(myNoteInfo), &myNoteInfo, NULL);
	if (myErr != noErr)
		goto bail;

	// insert a knob list atom;
	// this atom is optional; if it's not present, QTMA assumes some reasonable values
	*(long *)(&myKnobList.knobCount) = EndianU32_NtoB(kInstKnobMissingDefault);
	*(long *)(&myKnobList.knobFlags) = EndianU32_NtoB(1L);
	*(long *)(&myKnobList.knob[0].number) = EndianU32_NtoB(1L);
	*(long *)(&myKnobList.knob[0].value)  = EndianU32_NtoB(1L);
	
	myErr = QTInsertChild(myInstrument, kParentAtomIsContainer, kaiKnobListType, 1, 1, sizeof(myKnobList), &myKnobList, NULL);
	if (myErr != noErr)
		goto bail;

	// insert a key range information atom; this is the parent of the sample description atom
	myErr = QTInsertChild(myInstrument, kParentAtomIsContainer, kaiKeyRangeInfoType, 1, 1, 0, NULL, &myKeyRangeInfoAtom);
	if (myErr != noErr)
		goto bail;

	if (myKeyRangeInfoAtom != 0) {
		InstSampleDescRec		mySampleDescRec;
		
		// define the characteristics of the sampled sound	
		*(OSType *)(&mySampleDescRec.dataFormat) = EndianU32_NtoB(mySndInfo.format);
		*(short *)(&mySampleDescRec.numChannels) = EndianU16_NtoB(mySndInfo.numChannels);
		*(short *)(&mySampleDescRec.sampleSize) = EndianU16_NtoB(mySndInfo.sampleSize);
		*(UnsignedFixed *)(&mySampleDescRec.sampleRate) = EndianU32_NtoB(mySndInfo.sampleRate);
		*(short *)(&mySampleDescRec.sampleDataID) = EndianU16_NtoB(kCustomInstAtomID);
		*(long *)(&mySampleDescRec.offset) = 0L;
		*(long *)(&mySampleDescRec.numSamples) = EndianU32_NtoB(mySndInfo.sampleCount);

		*(long *)(&mySampleDescRec.loopType) = EndianU32_NtoB(kMusicLoopTypeNormal);
		*(long *)(&mySampleDescRec.loopStart) = 0L;
		*(long *)(&mySampleDescRec.loopEnd) = EndianU32_NtoB(mySndInfo.sampleCount);

		*(long *)(&mySampleDescRec.pitchNormal) = EndianU32_NtoB(myBaseFreq);
		*(long *)(&mySampleDescRec.pitchLow) = EndianU32_NtoB(kMIDINoteValue_Lowest);
		*(long *)(&mySampleDescRec.pitchHigh) = EndianU32_NtoB(kMIDINoteValue_Highest);
		
		// insert the sample description atom
		myErr = QTInsertChild(myInstrument, myKeyRangeInfoAtom, kaiSampleDescType, 1, 1, sizeof(mySampleDescRec), &mySampleDescRec, NULL);
		if (myErr != noErr)
			goto bail;		
	}
	
	// insert a sample information atom; this is the parent of the sample data atom and must have
	// the same atom ID specified by the sampleDataID field of the instrument sample description 
	myErr = QTInsertChild(myInstrument, kParentAtomIsContainer, kaiSampleInfoType, kCustomInstAtomID, 0, 0, NULL, &mySampleInfoAtom);
	if (myErr != noErr)
		goto bail;

	if (mySampleInfoAtom != 0) {
		// insert a sample data atom into the sample information atom; this atom contains the actual
		// sample data that defines the custom instrument; the format of the sample data is defined
		// by the corresponding sample description atom
		mySampleDataSize = mySndInfo.sampleCount * myCompInfo.bytesPerSample;
		mySampleDataPtr = (unsigned long)(*theHandle + myDataOffset);
		
		myErr = QTInsertChild(myInstrument, mySampleInfoAtom, kaiSampleDataType, 1, 0, mySampleDataSize, (void *)mySampleDataPtr, NULL);
		if (myErr != noErr)
			goto bail;
	}

	// insert an instrument info atom; this is a parent atom
	myErr = QTInsertChild(myInstrument, kParentAtomIsContainer, kaiInstInfoType, 1, 0, 0, NULL, &myInstInfoAtom);
	if (myErr != noErr)
		goto bail;

	if (myInstInfoAtom != 0) {
		PicHandle			myPictHandle = NULL;
		Str63				myWriter = "This space for rent!";
		Str63				myRights = "Copyright 1998 by Apple Computer, Inc.";
		Str63				myOthers = "Custom atomic instrument.";
		
		// insert a picture atom into the instrument info atom
		myPictHandle = GetPicture(kCustomInstPICTID);
		if (myPictHandle != NULL) {
			HLock((Handle)myPictHandle);
			QTInsertChild(myInstrument, myInstInfoAtom, kaiPictType, 1, 0, GetHandleSize((Handle)myPictHandle), *myPictHandle, NULL);
			HUnlock((Handle)myPictHandle);
			ReleaseResource((Handle)myPictHandle);
		}

		// insert a writer atom into the instrument info atom
		QTInsertChild(myInstrument, myInstInfoAtom, kaiWriterType, 1, 0, myWriter[0], &myWriter[1], NULL);
		
		// insert a copyright atom into the instrument info atom
		QTInsertChild(myInstrument, myInstInfoAtom, kaiCopyrightType, 1, 0, myRights[0], &myRights[1], NULL);
		
		// insert an other info atom into the instrument info atom
		QTInsertChild(myInstrument, myInstInfoAtom, kaiOtherStrType, 1, 0, myOthers[0], &myOthers[1], NULL);
	}
		
bail:	
	return(myInstrument);
}


//////////
//
// QTMusic_PickCustomInstThenPlaySomeNotes
// Let the user select a sound file for a custom instrument; then play some notes.
//
//////////

void QTMusic_PickCustomInstThenPlaySomeNotes (void)
{
	Handle					myHandle = NULL;
	AtomicInstrument		myInstrument = NULL;
	NoteAllocator			myNoteAllocator = NULL;
	NoteChannel				myNoteChannel = NULL;
	OSErr					myErr = noErr;

	// open a file of type 'sfil' and get the single 'snd ' resource contained in it
	myHandle = QTMusic_PickSndResource();
	if (myHandle == NULL)
		goto bail;
		
	// create the new instrument from the resource data
	myInstrument = QTMusic_CreateAtomicInstFromResource(myHandle);
	if (myInstrument == NULL)
		goto bail;
	
	// open the note allocator component
	myNoteAllocator = OpenDefaultComponent(kNoteAllocatorComponentType, 0);
	if (myNoteAllocator == NULL)
		goto bail;

	QTLockContainer(myInstrument);
	myErr = NANewNoteChannelFromAtomicInstrument(myNoteAllocator, *myInstrument, 0, &myNoteChannel);
	QTUnlockContainer(myInstrument);
	if ((myErr != noErr) || (myNoteChannel == NULL))
		goto bail;

	// if we've gotten this far, it's OK to play some musical notes; lovely
	QTMusic_PlayRisingNotesOnChannel(myNoteAllocator, myNoteChannel);

bail:
	if (myInstrument != NULL)
		QTDisposeAtomContainer(myInstrument);
		
	if (myNoteChannel != NULL)
		NADisposeNoteChannel(myNoteAllocator, myNoteChannel);
		
	if (myNoteAllocator != NULL)
		CloseComponent(myNoteAllocator);
}
			

//////////
//
// QTMusic_PickSndResource
// Let the user select a sound resource file (of type 'sfil'); return a handle to the resource data.
//
//////////

Handle QTMusic_PickSndResource (void)
{
	FSSpec					myFSSpec;
	OSType 					myTypeList[] = {kQTFileTypeSystemSevenSound};
	short					myNumTypes = 1;
	QTFrameFileFilterUPP	myFileFilterUPP = NULL;
	Handle					myHandle = NULL;
	short					myRefNum = -1;
	short					myResID;
	OSErr					myErr = noErr;

#if TARGET_OS_MAC
	myNumTypes = 0;
#endif

	// prompt the user to select a file of type 'sfil'
	
	myFileFilterUPP = QTFrame_GetFileFilterUPP((ProcPtr)QTMusic_FilterFiles);

	myErr = QTFrame_GetOneFileWithPreview(myNumTypes, (QTFrameTypeListPtr)myTypeList, &myFSSpec, myFileFilterUPP);

	if (myFileFilterUPP != NULL)
		DisposeNavObjectFilterUPP(myFileFilterUPP);

	if (myErr != noErr)
		goto bail;
	
	// open the file and extract the single 'snd ' resource contained in it
	myRefNum = FSpOpenResFile(&myFSSpec, fsRdPerm);
	if (myRefNum == -1)
		goto bail;
	
	myResID = QTMusic_GetSndResourceID(myRefNum);
	myHandle = GetResource(soundListRsrc, myResID);

bail:
	return(myHandle);
}


//////////
//
// QTMusic_GetSndResourceID
// Get the resource ID of the first 'snd ' resource in the specified resource file.
//
//////////

short QTMusic_GetSndResourceID (short theRefNum)
{
	Handle		myResource = NULL;
	short		myResID = 0;
	short		myCurRefNum;
	ResType		myResType;
	Str255		myResName;
	
	// make sure the specified file is the current resource file
	myCurRefNum = CurResFile();
	if (theRefNum != myCurRefNum)
		UseResFile(theRefNum);
	
	if (theRefNum > 0) {
		// find the resource ID of the single 'snd ' resource in the resource file;
		// we do this by loading the resource and then getting its info
		myResource = Get1IndResource(soundListRsrc, 1);
		if (myResource != NULL)
			GetResInfo(myResource, &myResID, &myResType, myResName);
	}
	
	// restore the current resource file
	if (theRefNum != myCurRefNum)
		UseResFile(myCurRefNum);
	
	return(myResID);
}


//////////
//
// QTMusic_ParseSndResource
// Parse the specified 'snd ' resource and return information about the sound format,
// as well as the offsets into the source resource of the sound header and the start
// of the audio sample data; also return the base frequency of the sampled sound.
//
// Based on the ParseSnd function by Kip Olson; see "Sound Secrets" in develop issue 24.
//
//////////

OSErr QTMusic_ParseSndResource (Handle theHandle,
								SoundComponentData *theSndInfo, CompressionInfo *theCompInfo,
								unsigned long *theHeaderOffset, unsigned long *theDataOffset, long *theBaseFreq)
{
	CommonSoundHeaderPtr	mySndHeader = NULL;
	unsigned long			myHeaderOffset = 0L;
	unsigned long			myDataOffset = 0L;
	UInt8					myBaseFreq = kMiddleC;
	short					myCompID;
	OSErr					myErr = noErr;

	// use GetSoundHeaderOffset to find the offset of the sound header
	// from the beginning of the sound resource handle
	myErr = GetSoundHeaderOffset((SndListHandle)theHandle, (long *)&myHeaderOffset);
	if (myErr != noErr)
		return(myErr);

	// get a pointer to the sound header using this offset
	mySndHeader = (CommonSoundHeaderPtr)(*theHandle + myHeaderOffset);
	myDataOffset = myHeaderOffset;
	myBaseFreq = mySndHeader->s.baseFrequency;

	// extract the sound info, based on audio data encoding type
	switch (mySndHeader->s.encode) {
		case stdSH:
			theSndInfo->sampleCount = mySndHeader->s.length;
			theSndInfo->sampleRate = mySndHeader->s.sampleRate;
			theSndInfo->sampleSize = 8;
			theSndInfo->numChannels = 1;
			myDataOffset += offsetof(SoundHeader, sampleArea);
			myCompID = notCompressed;
			break;

		case extSH:
			theSndInfo->sampleCount = mySndHeader->e.numFrames;
			theSndInfo->sampleRate = mySndHeader->e.sampleRate;
			theSndInfo->sampleSize = mySndHeader->e.sampleSize;
			theSndInfo->numChannels = mySndHeader->e.numChannels;
			myDataOffset += offsetof(ExtSoundHeader, sampleArea);
			myCompID = notCompressed;
			break;

		case cmpSH:
			theSndInfo->sampleCount = mySndHeader->c.numFrames;
			theSndInfo->sampleRate = mySndHeader->c.sampleRate;
			theSndInfo->sampleSize = mySndHeader->c.sampleSize;
			theSndInfo->numChannels = mySndHeader->c.numChannels;
			myDataOffset += offsetof(CmpSoundHeader, sampleArea);
			myCompID = mySndHeader->c.compressionID;
			theSndInfo->format = mySndHeader->c.format;
			break;

		default:
			return(badFormat);
			break;
	}
	
	// use GetCompressionInfo to get the data format of the sound and the compression information
	theCompInfo->recordSize = sizeof(CompressionInfo);
	myErr = GetCompressionInfo(myCompID, theSndInfo->format, theSndInfo->numChannels, theSndInfo->sampleSize, theCompInfo);
	if (myErr != noErr)
		return(myErr);

	// store the sound data format and convert frames to samples
	theSndInfo->format = theCompInfo->format;
	theSndInfo->sampleCount *= theCompInfo->samplesPerPacket;

	// return the offsets of the sound header and the audio data, and the base frequency of the sampled sound
	*theHeaderOffset = myHeaderOffset;
	*theDataOffset = myDataOffset;
	*theBaseFreq = (long)myBaseFreq;
	
	return(myErr);
}


//////////
//
// QTMusic_FilterFiles
// Filter files for a file-opening dialog box.
//
//////////

#if TARGET_OS_MAC
PASCAL_RTN Boolean QTMusic_FilterFiles (AEDesc *theItem, void *theInfo, void *theCallBackUD, NavFilterModes theFilterMode)
{
#pragma unused(theCallBackUD, theFilterMode)
	NavFileOrFolderInfo		*myInfo = (NavFileOrFolderInfo *)theInfo;
	
	if (theItem->descriptorType == typeFSS) {
		if (!myInfo->isFolder) {
			OSType			myType = myInfo->fileAndFolder.fileInfo.finderInfo.fdType;
			
			if (myType == kQTFileTypeSystemSevenSound)
				return(true);
			
			// if we got to here, it's a file we cannot open
			return(false);		
		}
	}
	
	// if we got to here, it's a folder or non-HFS object
	return(true);
}
#endif
#if TARGET_OS_WIN32
PASCAL_RTN Boolean QTMusic_FilterFiles (CInfoPBPtr thePBPtr)
{
#pragma unused(thePBPtr)
	return(false);
}
#endif
