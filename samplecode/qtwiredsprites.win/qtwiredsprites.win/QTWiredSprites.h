//////////
//
//	File:		QTWiredSprites.c
//
//	Contains:	QuickTime wired sprites support for QuickTime movies.
//
//	Written by:	Sean Allen
//	Revised by:	Chris Flick and Tim Monroe
//				Based (heavily!) on the existing MakeActionSpriteMovie.c code written by Sean Allen.
//
//	Copyright:	� 1997-1998 by Apple Computer, Inc., all rights reserved.
//
//	Change History (most recent first):
//
//	   <1>	 	03/25/97	rtm		first file; integrated existing code with shell framework
//	   
//////////

//////////
//
// header files
//
//////////

#ifndef __ENDIAN__
#include <Endian.h>
#endif

#ifndef __FIXMATH__
#include <FixMath.h>
#endif

#ifndef __MOVIES__
#include <Movies.h>
#endif

#ifndef _WIREDSPRITEUTILITIES_
#include "WiredSpriteUtilities.h"
#endif

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

#define kSpriteTrackWidth					320
#define kSpriteTrackHeight					240

#define kPenguinWidth						92
#define kPenguinHeight						102
#define kStartEndButtonWidth				36
#define kStartEndButtonHeight				19
#define kNextPrevButtonWidth				28
#define kNextPrevButtonHeight				19

#define kGoToBeginningButtonUp				10000
#define kGoToBeginningButtonDown			10001
#define kGoToEndButtonUp					10002
#define kGoToEndButtonDown					10003
#define kGoToPrevButtonUp					10004
#define kGoToPrevButtonDown					10005
#define kGoToNextButtonUp					10006
#define kGoToNextButtonDown					10007
#define kPenguinForward						20000
#define kPenguinLeft						20001
#define kPenguinRight						20002
#define kPenguinClosed						20003
#define kWalkDownRightCycleStart			20004
#define kWalkDownRightCycleEnd				20015
#define kPenguinWalkCycleLength				(kWalkDownRightCycleEnd - kWalkDownRightCycleStart + 1)
#define kPenguinFlapCycleStart				30000
#define kPenguinFlapCycleEnd				30005
#define kPenguinFlapCycleLength				(kPenguinFlapCycleEnd - kPenguinFlapCycleStart + 1)

#define kGoToBeginningButtonUpIndex			1
#define kGoToBeginningButtonDownIndex		2
#define kGoToEndButtonUpIndex				3
#define kGoToEndButtonDownIndex				4
#define kGoToPrevButtonUpIndex				5
#define kGoToPrevButtonDownIndex			6
#define kGoToNextButtonUpIndex				7
#define kGoToNextButtonDownIndex			8
#define kPenguinForwardIndex				9
#define kPenguinLeftIndex					10
#define kPenguinRightIndex					11
#define kPenguinClosedIndex					12
#define kPenguinDownRightCycleStartIndex	13
#define kPenguinDownRightCycleEndIndex		(kPenguinDownRightCycleStartIndex + kPenguinWalkCycleLength - 1)

#define kSpriteMediaTimeScale				600
#define kSpriteMediaFrameDuration			20
#define kSpriteMediaFramesPerSecond			(kSpriteMediaTimeScale / kSpriteMediaFrameDuration)

#define kGoToBeginningSpriteID				1
#define kGoToPrevSpriteID					2
#define kGoToNextSpriteID					3
#define kGoToEndSpriteID					4
#define kPenguinOneSpriteID					5
#define kPenguinTwoSpriteID					6

#define kNumOverrideSamples					(2 * kPenguinWalkCycleLength)

#define kPenguinStateVariableID				1
#define kButtonStateOne						0
#define kButtonStateTwo						1

#define kWiredSavePrompt					"Save Wired Sprite Movie As:"
#define kWiredSaveFileName					"WiredSprite.mov"

#define kMouseStateVariableID				2


//////////
//
// function prototypes
//
//////////

OSErr							QTWired_CreateWiredSpritesMovie (void);
OSErr							QTWired_AddPenguinTwoConditionalActions (QTAtomContainer theContainer, QTAtom theEventAtom);
OSErr							QTWired_AddWraparoundMatrixOnIdle (QTAtomContainer theContainer);
OSErr							QTWired_AddCursorChangeOnMouseOver (QTAtomContainer theContainer, QTAtomID theID);
OSErr							QTWired_MakeSpriteDraggable (QTAtomContainer theContainer, QTAtomID theID);
