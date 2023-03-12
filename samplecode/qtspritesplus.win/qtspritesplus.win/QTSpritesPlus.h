//////////
//
//	File:		QTSpritesPlus.c
//
//	Contains:	QuickTime sprites support for QuickTime movies.
//				This file is used for BOTH MacOS and Windows.
//
//	Written by:	Tim Monroe
//
//	Copyright:	© 2001 by Apple Computer, Inc., all rights reserved.
//
//	Change History (most recent first):
//
//	   <1>	 	01/18/01	rtm		first file; based on existing QTSprites sample code
//	   
//////////

//////////
//
// header files
//
//////////

#include <Endian.h>
#include <FixMath.h>
#include <Fonts.h>
#include <Movies.h>
#include <Processes.h>
#include <QuickTimeComponents.h>
#include <Resources.h>
#include <Script.h>

#ifndef _SPRITEUTILITIES_
#include "SpriteUtilities.h"
#endif

#ifndef __ENDIANUTILITIES__
#include "EndianUtilities.h"
#endif

#include "ComApplication.h"

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

#define kIconDimension						32

// sizes of the sprite tracks
#define kIconSpriteTrackWidth				294
#define kIconSpriteTrackHeight				100

#define kPenguinSpriteTrackWidth			152
#define kPenguinSpriteTrackHeight			202

#define kPowerBookSpriteTrackWidth			346
#define kPowerBookSpriteTrackHeight			234

// PICT resource IDs
#define kOldQTIconID						200
#define kPenguinPictID						128
#define kTitaniumPowerBookID				130

// sprite atom IDs	
#define kQTIconSpriteAtomID					1
#define kPenguinSpriteAtomID				1

// image indices
#define kOldQTIconImageIndex				1
#define kPowerBookImageIndex				2
#define kPenguinImageIndex					1

#define kSpriteMediaTimeScale				600
#define kSpriteMediaFrameDurationIcon		1000
#define kSpriteMediaFrameDurationPowerBook	18000
#define kSpriteMediaFrameDurationPenguin	6000

#define kNumRotations						5

#define kSpriteSavePrompt					"Save New Sprite Movie As:"
#define kSpriteSaveMovieFileName			"Sprite.mov"


//////////
//
// function prototypes
//
//////////

ApplicationDataHdl				QTSprites_InitWindowData (WindowObject theWindowObject);
void							QTSprites_DumpWindowData (WindowObject theWindowObject);
OSErr							QTSprites_CreateSpritesMovie (UInt16 theMenuItem);
void							QTSprites_GetMovieSize (UInt16 theMenuItem, Fixed *theHeight, Fixed *theWidth);
void							QTSprites_AddIconMovieSamplesToMedia (Media theMedia);
void							QTSprites_AddPowerBookMovieSamplesToMedia (Media theMedia);
void							QTSprites_AddPenguinMovieSamplesToMedia (Media theMedia);
void							QTSprites_AddSpaceMovieSamplesToMedia (Media theMedia);
void							QTSprites_SetTrackProperties(Media theMedia, UInt16 theMenuItem);
Boolean							QTSprites_HitTestSprites (WindowObject theWindowObject, EventRecord *theEvent);
OSErr							QTSprites_AddVideoOverrideTrack (Movie theSpriteMovie, Track theSpriteTrack);
OSErr							QTSprites_AddVideoEntryToInputMap (QTAtomContainer theInputMap, long theRefIndex, long theID, OSType theType, char *theName);
void							QTSprites_AddTweenOverrideTrack (Movie theMovie, Track theTargetTrack, UInt32 theTweenType);
OSErr							QTSprites_AddTweenEntryToInputMap (QTAtomContainer theInputMap, long theRefIndex, long theID, OSType theType, char *theName);
OSErr							QTSprites_SetTweenEntryDuration (QTAtomContainer theSample, QTAtomID theID, TimeValue theDuration);
OSErr							QTSprites_SetTweenEntryStartOffset (QTAtomContainer theSample, QTAtomID theID, TimeValue theOffset);
OSErr							QTSprites_ImportVideoTrack (Movie theSrcMovie, Movie theDstMovie, Track *theTrack);
