/*	File:		SoundUnit.h	Contains:		Written by: Jim Reekes		Copyright:	Copyright � 1994-1999 by Apple Computer, Inc., All Rights Reserved.				You may incorporate this Apple sample source code into your program(s) without				restriction. This Apple sample source code has been provided "AS IS" and the				responsibility for its operation is yours. You are not permitted to redistribute				this Apple sample source code as "Apple sample source code" after having made				changes. If you're going to re-distribute the source, we require that you make				it clear in the source that the code was descended from Apple sample source				code, but that you've made changes.	Change History (most recent first):				7/29/1999	Karl Groethe	Updated for Metrowerks Codewarror Pro 2.1				*///~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~#ifndef __SOUNDUNIT__#define __SOUNDUNIT__//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~// constants//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//refer to the SoundAppSnds.r file for documentation on these valuesenum {	kOctave1	= 0,					//octaves of MIDI values	kOctave2	= 12,	kOctave3	= 24,	kOctave4	= 36,	kOctave5	= 48,	kOctave6	= 60,	kOctave7	= 72,	kOctave8	= 84,	kOctave9	= 96,	kOctave10	= 108,	kOctave11	= 120,	Akey		= -3,					//the key A	Bbkey		= -2,					//the key B flat	Bkey		= -1,					//the key B	Ckey		= 0,					//the key C	Dbkey		= 1,					//the key D flat	Dkey		= 2,					//the key D	Ebkey		= 3,					//the key D flat	Ekey		= 4,					//the key E	Fkey		= 5,					//the key F	Gbkey		= 6,					//the key G flat	Gkey		= 7,					//the key G	Abkey		= 8,					//the key A flat//These are other constants used in the SoundUnit	kInitNone	= 0,					//no init options	kWait		= false,				//wait for the channel	kSMAsynch	= true,					//asynchronous Sound Manager call	kWaveSize	= 512,					//standard size of wave table	kSyncID		= 0x12345678,			//identifier used in syncCmd	kOneSecond	= 2000,					//one second frequency duration	kNoSynth	= 0						//no synth is specified};//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//types//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~typedef SndCommand *SndCmdPtr;	// Ptr to a sound command, for type coersion//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~// prototypes//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~/*The routines below are for public consumption in this SoundUnit.  Note thatthese all return standard sound channels to allow easy of modifications.I can change the structure of my own sound channels and the code withoutforcing a change in the application that uses this SoundUnit.VERSION 1.1: No longer putting the SANELib into a seperate segment, which thenneeds to be unloaded.  Instead, I merge it into the Main segment.  Refer tothe Make file for more information.*/pascal void _SoundUnit(void);pascal OSErr InitSoundUnit(void);pascal Boolean HasMACE(void);pascal Boolean HasSoundInput(void);pascal Boolean HasStereoSupport(void);pascal short GetSoundMgrVersion(void);pascal Boolean HasSoundCompleted(void);pascal Boolean HasChannelOpen(void);pascal OSErr SendQuiet(SndChannelPtr chan, Boolean immediate);pascal void DoSoundComplete(void);pascal void FreeAllChans(void);pascal void FreeSoundUnit(void);pascal OSErr SoundComplete(SndChannelPtr chan);pascal OSErr HoldSnd(SndListHandle sndHandle);pascal OSErr PlaySong(SndChannelPtr chan, SndListHandle sndSong);pascal OSErr SetSquareWaveTimbre(SndChannelPtr squareChan, short timbre, Boolean immediate);pascal OSErr SendNote(SndChannelPtr chan, short duration, long note);pascal OSErr SendRest(SndChannelPtr chan, short duration);pascal OSErr GetSquareWaveChan(SndChannelPtr *squareChan, short timbre);pascal OSErr GetNoSynthChan(SndChannelPtr *chan);pascal OSErr GetSampleChan(SndChannelPtr *sampleChan, long init, SndListHandle sndInstrument);pascal OSErr Get4SampleInstruments(SndChannelPtr *chan1, SndChannelPtr *chan2, SndChannelPtr *chan3, SndChannelPtr *chan4, SndListHandle sndInstrument1, SndListHandle sndInstrument2, SndListHandle sndInstrument3, SndListHandle sndInstrument4);pascal Boolean HasWorkingWaveTables(void);pascal OSErr GetWaveChans(SndChannelPtr *waveChan1, SndChannelPtr *waveChan2, SndChannelPtr *waveChan3, SndChannelPtr *waveChan4);pascal OSErr InstallWave(SndChannelPtr waveChan, Ptr aWavePtr, short waveLength);pascal OSErr Play4ChanSongs(SndChannelPtr chan1, SndChannelPtr chan2, SndChannelPtr chan3, SndChannelPtr chan4, SndListHandle song1, SndListHandle song2, SndListHandle song3, SndListHandle song4);pascal long GetSndDataOffset(SndListHandle sndHandle, short *dataType, short *waveLength);pascal OSErr HyperSndPlay(SndListHandle sndHandle);pascal OSErr AsynchSndPlay(SndListHandle sndHandle);#endif