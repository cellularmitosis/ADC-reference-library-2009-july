//////////
//
//	File:		SoundSnippets.h
//
//	Contains:	Code snippets showing how to perform a few typical sound-related operations.
//
//	Written by:	Tim Monroe
//				Some routines based on code by Jim Reekes.
//
//	Copyright:	© 1998 by Apple Computer, Inc., all rights reserved.
//
//	Change History (most recent first):
//
//	   <1>	 	04/17/98	rtm		first file
//	 
//////////

#ifndef __MACTYPES__
#include <MacTypes.h>
#endif

#ifndef __cmath__
#include <Math.h>
#endif

#ifndef __FIXMATH__
#include <FixMath.h>
#endif

#ifndef __MOVIES__
#include <Movies.h>
#endif

#ifndef __QUICKTIMECOMPONENTS__
#include <QuickTimeComponents.h>
#endif

#ifndef __RESOURCES__
#include <Resources.h>
#endif

#ifndef __SOUND__
#include <Sound.h>
#endif

#ifndef __SCRIPT__
#include <Script.h>
#endif

// constants
#define kNoWait						true
#define kWait						false
#define kNumberOfBufferChunks		10
#define kNumberOfCmdsInQueue		8
#define kSampleResourceID			2222

#define kNumberOfTargetBytes		(40*1024L)

#define kSaveSoundPrompt			"Save sound file as:"
#define kSaveSoundWaveName			"Untitled.wav"
#define kSaveSoundFileName			"Untitled"


// function prototypes
void						SndSnip_SaveSoundMovieAsWAVEFile (Movie theMovie);
void						SndSnip_SaveSoundMovieAsAnyTypeFile (Movie theMovie);
void						SndSnip_SaveSoundTrackAsAnyTypeFile (Movie theMovie);
void						SndSnip_ExtractSoundTrackIntoHandle (Movie theMovie);
void						SndSnip_PlayWAVEFileWithQuickTime (void);
void						SndSnip_SetVolumeOfSoundTrack (Movie theMovie, short theVolume);
OSErr						SndSnip_PlaySoundResourceUsingBufferCmds (void);
PASCAL_RTN void				SndSnip_CallbackProc (SndChannelPtr theChannel, SndCommand *theCommand);
void						SndSnip_CheckBuffers (void);
static OSErr				SndSnip_InstallBufferCmd (SndChannelPtr theChannel, SoundHeaderPtr theHeaderPtr);
static OSErr				SndSnip_InstallCallbackCmd (SndChannelPtr theChannel, short theParam1, long theParam2);
SoundHeaderPtr				SndSnip_GetSoundHeader (Handle theSndHandle);
long						SndSnip_GetSndBaseFrequency (Handle theSndHandle);
OSErr						SndSnip_GetHardwareSettings (SndChannelPtr theChannel, SoundComponentData *theInfo);
OSErr						SndSnip_GetAudioSettings (Movie theMovie, SoundComponentData *theInfo);
Boolean						SndSnip_HasSoundManager3_1 (void);
Boolean						SndSnip_CheckVersionNumber (const NumVersion *theVersion, UInt8 theMajor, UInt8 theMinor, UInt8 theBug);
OSErr						SndSnip_GetVolume (SndChannelPtr theChannel, unsigned short *theLeftVol, unsigned short *theRightVol);
OSErr						SndSnip_SetVolume (SndChannelPtr theChannel, unsigned short theLeftVol, unsigned short theRightVol);
OSErr						SndSnip_ConvertWAVEFormats (Movie theMovie, FSSpec *theFile);
void						SndSnip_PromptUserForAudioFileAndCompress (void);
void						SndSnip_PromptUserForDiskFileAndSaveCompressed (short theSrcRefNum, SoundComponentData *theSrcInfo, SoundComponentData *theDstInfo, unsigned long theSrcDataOffset, unsigned long theSrcNumFrames);
