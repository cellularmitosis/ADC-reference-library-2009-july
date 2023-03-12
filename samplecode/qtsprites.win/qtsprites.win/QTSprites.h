//////////
//
//	File:		QTSprites.c
//
//	Contains:	QuickTime sprites support for QuickTime movies.
//				This file is used for BOTH MacOS and Windows.
//
//	Written by:	???
//	Revised by:	Tim Monroe and Deeje Cooley
//				Based (heavily!) on the existing MakeSpriteMovie.c code written by ???.
//
//	Copyright:	� 1997-1998 by Apple Computer, Inc., all rights reserved.
//
//	Change History (most recent first):
//
//	   <1>	 	04/02/98	rtm		first file; integrated existing code with shell framework
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

// sizes of the three sprite tracks
#define kIconSpriteTrackWidth				294
#define kIconSpriteTrackHeight				100

#define kPenguinSpriteTrackWidth			152
#define kPenguinSpriteTrackHeight			202

#define kSpaceSpriteTrackWidth				640
#define kSpaceSpriteTrackHeight				480

// PICT resource IDs
#define kOldQTIconID						200
#define kNewQTIconID						201

#define kPenguinPictID						128

#define kIconPictID							129
#define kWorldPictID						130
#define kBackgroundPictID					158
#define kFirstSpaceShipPictID				(kBackgroundPictID + 1)

// sprite atom IDs	
#define kQTIconSpriteAtomID					1

#define kPenguinSpriteAtomID				1

#define kBackgroundSpriteAtomID				1
#define kSpaceShipSpriteAtomID				2
#define kWorldSpriteAtomID					3
#define kIconSpriteAtomID					4

// image indices
#define kQTIconImageIndex					1

#define kPenguinImageIndex					1

#define kIconImageIndex						1
#define kWorldImageIndex					2
#define kBackgroundImageIndex				3
#define kFirstSpaceShipImageIndex			4
#define kNumSpaceShipImages					24
#define kLastSpaceShipImageIndex			(kFirstSpaceShipImageIndex + kNumSpaceShipImages - 1)

#define kSpriteMediaTimeScale				600
#define kSpriteMediaFrameDurationIcon		kSpriteMediaTimeScale/50			// each frame is 1/50 second
#define kSpriteMediaFrameDurationPenguin	kSpriteMediaTimeScale/10			// each frame is 1/10 second
#define kSpriteMediaFrameDurationSpace		8

#define kNumOverrideSamples					99
#define kNumOverrideSamplesSpace			199

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
void							QTSprites_GetBackgroundColor (UInt16 theMenuItem, RGBColor *theColor);
void							QTSprites_AddIconMovieSamplesToMedia (Media theMedia);
void							QTSprites_AddPenguinMovieSamplesToMedia (Media theMedia);
void							QTSprites_AddSpaceMovieSamplesToMedia (Media theMedia);
void							QTSprites_SetTrackProperties(Media theMedia, UInt16 theMenuItem);
Boolean							QTSprites_HitTestSprites (WindowObject theWindowObject, EventRecord *theEvent);
