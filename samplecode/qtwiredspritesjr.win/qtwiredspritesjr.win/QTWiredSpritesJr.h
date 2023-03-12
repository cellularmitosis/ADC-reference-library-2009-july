//////////
//
//	File:		QTWiredSpritesJr.c
//
//	Contains:	QuickTime wired sprites support for QuickTime movies.
//
//	Written by:	Tim Monroe
//
//	Copyright:	© 2001 by Apple Computer, Inc., all rights reserved.
//
//	Change History (most recent first):
//
//	   <1>	 	01/18/01	rtm		first file; based on existing QTSprites and QTWiredSprites sample code
//	   
//////////

//////////
//
// header files
//
//////////

#ifndef _SPRITEUTILITIES_
#include "SpriteUtilities.h"
#endif

#ifndef _WIREDSPRITEUTILITIES_
#include "WiredSpriteUtilities.h"
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
// compiler directives
//
//////////

#define USE_WIRED_UTILITIES					1


//////////
//
// constants for controller buttons sprites
//
//////////

// controller image PICT resource IDs
#define kPauseUpID							131
#define kPauseDownID						132
#define kPlayUpID							133
#define kPlayDownID							134
#define kToBeginUpID						135
#define kToBeginDownID						136
#define kToEndUpID							137
#define kToEndDownID						138
#define kBackStepUpID						139
#define kBackStepDownID						140
#define kAheadStepUpID						141
#define kAheadStepDownID					142

// image indices
#define kPauseUpIndex						1
#define kPauseDownIndex						2
#define kPlayUpIndex						3
#define kPlayDownIndex						4
#define kToBeginUpIndex						5
#define kToBeginDownIndex					6
#define kToEndUpIndex						7
#define kToEndDownIndex						8
#define kBackStepUpIndex					9
#define kBackStepDownIndex					10
#define kAheadStepUpIndex					11
#define kAheadStepDownIndex					12

#define kNumControllerButtons				6
#define kNumControllerImages				kAheadStepDownIndex
#define kFirstControllerImageID				kPauseUpID

// sprite atom IDs	
#define kPauseSpriteAtomID					1
#define kPlaySpriteAtomID					2
#define kToBeginSpriteAtomID				3
#define kToEndSpriteAtomID					4
#define kBackStepSpriteAtomID				5
#define kAheadStepSpriteAtomID				6

// position indices	
#define kToBeginSpritePosition				0
#define kBackStepSpritePosition				1
#define kPlaySpritePosition					2
#define kPauseSpritePosition				3
#define kAheadStepSpritePosition			4
#define kToEndSpritePosition				5

#define kButtonHeight						17
#define kButtonWidth						17


//////////
//
// constants for QuickTime icon sprite and its movie
//
//////////

#define kIconDimension						32

// sizes of the sprite tracks
#define kIconSpriteTrackWidth				294
#define kIconSpriteTrackHeight				100

// PICT resource IDs
#define kNewQTIconID						201
#define kNewQTIconDownID					202

// sprite atom IDs	
#define kQTIconSpriteAtomID					1

// image indices
#define kNewQTIconImageIndex				1
#define kNewQTIconDownImageIndex			2

#define kSpriteMediaTimeScale				600
#define kSpriteMediaFrameDurationIcon		1000

#define kMouseStateVariableID				2


//////////
//
// miscellaneous constants
//
//////////

#define kMaxLayerNumber						0x7fff			// maximum layer number

#define kSpriteSavePrompt					"Save New Sprite Movie As:"
#define kSpriteSaveMovieFileName			"WiredSprite.mov"


//////////
//
// function prototypes
//
//////////

OSErr							QTWired_CreateDraggableIconMovie (void);
OSErr							QTWired_AddIconMovieSamplesToMedia (Media theMedia);
void							QTWired_SetTrackProperties (Media theMedia, UInt32 theIdleFrequency);
void							QTWired_AddControllerButtonSamplesToMedia (Media theMedia, long theTrackWidth, long theTrackHeight, TimeValue theDuration);
OSErr							QTWired_AddSpriteControllerTrack (Movie theMovie);
OSErr							QTWired_MakeSpriteDraggable (QTAtomContainer theContainer, QTAtomID theID);
OSErr							QTWired_AddCursorChangeToSprite (QTAtomContainer theContainer, QTAtomID theID);
short							QTWired_GetLowestLayerInMovie (Movie theMovie);


