//////////
//
//	File:		EffectsUtilities.h
//
//	Contains:	Some utilities for working with QuickTime video effects.
//
//	Written by:	Tim Monroe
//
//	Copyright:	© 2001 by Apple Computer, Inc., all rights reserved.
//
//	Change History (most recent first):
//
//	   <1>	 	07/10/01	rtm		first file
//	   
//////////

#ifndef __EFFECTSUTILITIES__
#define __EFFECTSUTILITIES__


//////////
//
// header files
//
//////////

#ifndef __IMAGECODEC__
#include <ImageCodec.h>
#endif

#ifndef __RESOURCES__
#include <Resources.h>
#endif

#ifndef __SOUND__
#include <Sound.h>
#endif


//////////
//
// compiler flags
//
//////////

#define USES_MAKE_IMAGE_DESC_FOR_EFFECT	1		// use MakeImageDescriptionForEffect (QT 4.0 and later)


//////////
//
// constants
//
//////////

#define kMaxNumSources					3		// the maximum number of input tracks we will collect

#define kSourceOneName					FOUR_CHAR_CODE('srcA')
#define kSourceTwoName					FOUR_CHAR_CODE('srcB')
#define kSourceThreeName				FOUR_CHAR_CODE('srcC')
#define kSourceNoneName					FOUR_CHAR_CODE('srcZ')

#define kVideoTrackTimeScale			600


//////////
//
// function prototypes
//
//////////

QTAtomContainer							EffectsUtils_CreateEffectDescription (OSType theEffectType, OSType theSourceName1, OSType theSourceName2, OSType theSourceName3);
OSErr									EffectsUtils_GetTypeFromEffectDescription (QTAtomContainer theEffectDesc, OSType *theEffectType);
OSErr									EffectsUtils_AddTrackReferenceToInputMap (QTAtomContainer theInputMap, Track theTrack, Track theSrcTrack, OSType theSrcName);
ImageDescriptionHandle					EffectsUtils_MakeSampleDescription (OSType theEffectType, short theWidth, short theHeight);
QTAtomContainer							EffectsUtils_GetEffectDescFromQFXFile (FSSpec *theFSSpec);

OSErr									EffectsUtils_GetPictResourceAsGWorld (short theResID, short theWidth, short theHeight, short theDepth, GWorldPtr *theGW);
OSErr									EffectsUtils_AddVideoTrackFromGWorld (Movie *theMovie, GWorldPtr theGW, Track *theSourceTrack, long theStartTime, TimeValue theDuration, short theWidth, short theHeight);
short									EffectsUtils_GetFrontmostTrackLayer (Movie theMovie, OSType theTrackType);


#endif	// ifndef __EFFECTSUTILITIES__
