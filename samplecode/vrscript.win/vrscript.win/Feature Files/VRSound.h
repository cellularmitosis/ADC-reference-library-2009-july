//////////
//
//	File:		VRSound.h
//
//	Contains:	Sound support for QuickTime VR movies.
//
//	Written by:	Tim Monroe
//
//	Copyright:	© 1996-1997 by Apple Computer, Inc., all rights reserved.
//
//	Change History (most recent first):
//
//	   <3>	 	03/13/98	rtm		removed for chunk-based sound files (AIFF and WAVE)
//	   <2>	 	08/11/97	rtm		added support for chunk-based sound files (AIFF and WAVE)
//	   <1>	 	12/09/96	rtm		first file
//	   
//////////

#pragma once

// header files
#include <math.h>
#include <Endian.h>

#include "ComApplication.h"

#if TARGET_OS_MAC
#include "MacFramework.h"
#endif

#if TARGET_OS_WIN32
#include "WinFramework.h"
#endif

#include "VRScript.h"

// constants

// values that we pass in param1 of a callBackCmd
enum {
	kVRSound_Complete	= 1				// the sound has finished playing
};

#define kVRSound_NumFadeSteps	8		// number of steps while fading a sound to silence
#define kVRSound_FadeStepWait	5		// number of ticks to wait on each fade step
#define kVRSound_NumCmdsInQueue	4		// number of commands in a sound channel queue
#define kVRSound_SpeakerAngle	15.0	// angular distance (in degrees) of left or right speaker from viewer vector


// function prototypes
Boolean						VRSound_CheckVersionNumber (const NumVersion *theVersion, UInt8 theMajor, UInt8 theMinor, UInt8 theBug);
SoundHeaderPtr				VRSound_GetSoundHeader (Handle theSndHandle);
long						VRSound_GetSndBaseFrequency (Handle theSndHandle);
OSErr						VRSound_GetVolume (SndChannelPtr theChannel, unsigned short *theLeftVol, unsigned short *theRightVol);
OSErr						VRSound_SetVolume (SndChannelPtr theChannel, unsigned short theLeftVol, unsigned short theRightVol);
SndChannelPtr				VRSound_CreateSoundChannel (Boolean theSoundIsLocalized);
SSpSourceReference			VRSound_CreateLocalizedSource (void);
void						VRSound_SetLocation (WindowObject theWindowObject, UInt32 theEntryID, float theX, float theY, float theZ, UInt32 theOptions);
short						VRSound_GetSndResourceID (short theRefNum);
void						VRSound_Update3DSoundEnv (WindowObject theWindowObject);
void						VRSound_SetBalanceAndVolume (WindowObject theWindowObject, float thePan, float theTilt);
void						VRSound_Init (void);
void						VRSound_Stop (void);
void						VRSound_InitWindowData (WindowObject theWindowObject);
void						VRSound_DumpWindowData (WindowObject theWindowObject);
void						VRSound_FadeNodeSounds (WindowObject theWindowObject);
void						VRSound_DumpNodeSounds (WindowObject theWindowObject);
void						VRSound_DumpSceneSounds (WindowObject theWindowObject);
void						VRSound_DumpSelectedSounds (WindowObject theWindowObject, UInt32 theOptions);
void						VRSound_FadeSilence (WindowObject theWindowObject, SndChannelPtr theChannel);
void						VRSound_PlaySilence (WindowObject theWindowObject, SndChannelPtr theChannel);
void						VRSound_PlayResource (WindowObject theWindowObject, SndChannelPtr theChannel, SndListHandle theResHandle, UInt32 theOptions);
void						VRSound_PlaySound (WindowObject theWindowObject, UInt32 theNodeID, UInt32 theResID, UInt32 theEntryID, float theX, float theY, float theZ, float theProjAngle, UInt32 theSourceMode, UInt32 theMode, UInt32 theFade, UInt32 theOptions);
void						VRSound_InstallCallbackMessage (VRScriptSoundPtr theEntry, short theMessage);
VRScriptSoundPtr			VRSound_GetFinishedSound (WindowObject theWindowObject);
PASCAL_RTN void				VRSound_CallbackProc (SndChannelPtr theChannel, SndCommand *theCommand);
void						VRSound_CheckForCompletedSounds (WindowObject theWindowObject);
void						VRSound_DumpEntryMem (VRScriptSoundPtr theEntry);

