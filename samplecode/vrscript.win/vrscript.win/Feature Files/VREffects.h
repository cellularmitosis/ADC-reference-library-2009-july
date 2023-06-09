//////////
//
//	File:		VREffects.h
//
//	Contains:	QuickTime video effects support for QuickTime VR movies.
//
//	Written by:	Tim Monroe
//
//	Copyright:	� 1997 by Apple Computer, Inc., all rights reserved.
//
//	Change History (most recent first):
//
//	   <1>	 	12/13/96	rtm		first file
//	   
//////////

#pragma once

//////////
//
// header files
//
//////////

#include <ImageCompression.h>
#include <ImageCodec.h>

#include "ComApplication.h"

#if TARGET_OS_MAC
#include "MacFramework.h"
#endif

#if TARGET_OS_WIN32
#include "WinFramework.h"
#endif

#include "VRScript.h"


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

#define kSourceOneName					FOUR_CHAR_CODE('srcA')
#define kSourceTwoName					FOUR_CHAR_CODE('srcB')
#define kSourceNoneName					FOUR_CHAR_CODE('srcZ')

#define kDefaultNumSteps				50		// the number of steps in a transition
#define kDoIdleStep						10		// the number of steps we take before giving scene-wide sound-only movies some idle time


//////////
//
// function prototypes
//
//////////

void						VREffects_InitWindowData (WindowObject theWindowObject);
void						VREffects_DumpWindowData (WindowObject theWindowObject);
Boolean						VREffects_DoIdle (WindowObject theWindowObject);
VRScriptTransitionPtr		VREffects_GetTransitionEffect (WindowObject theWindowObject, UInt32 fromNodeID, UInt32 toNodeID);
QTAtomContainer				VREffects_MakeEffectDescription (OSType theEffectType, long theEffectNum, OSType theSourceName1, OSType theSourceName2);
ImageDescriptionHandle		VREffects_MakeSampleDescription (OSType theEffectType, short theWidth, short theHeight);
OSErr						VREffects_SetupTransitionEffect (WindowObject theWindowObject, UInt32 fromNodeID, UInt32 toNodeID);
OSErr						VREffects_RunTransitionEffect (WindowObject theWindowObject);
void						VREffects_DumpEntryMem (VRScriptTransitionPtr theEntry);

