//////////
//
//	File:		VRSprites.h
//
//	Contains:	Support for QuickTime sprite tracks in VR nodes.
//
//	Written by:	Tim Monroe
//
//	Copyright:	� 1998 by Apple Computer, Inc., all rights reserved.
//
//	Change History (most recent first):
//
//	   <1>	 	06/19/98	rtm		first file
//	   
//////////

#pragma once


//////////
//
// header files
//	   
//////////

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
// constants
//	   
//////////

// special value for Options field of sprite click commands
#define kVRAnySprite		(UInt32)-1


//////////
//
// function prototypes
//	   
//////////

void						VRSprites_InitWindowData (WindowObject theWindowObject);
void						VRSprites_DumpWindowData (WindowObject theWindowObject);
void						VRSprites_SetVisibleState (WindowObject theWindowObject, QTAtomID theSpriteID, Boolean theState, UInt32 theOptions);
void						VRSprites_SetLayer (WindowObject theWindowObject, QTAtomID theSpriteID, short theLayer, UInt32 theOptions);
void						VRSprites_SetGraphicsMode (WindowObject theWindowObject, QTAtomID theSpriteID, long theMode, UInt32 theOptions);
void						VRSprites_SetImageIndex (WindowObject theWindowObject, QTAtomID theSpriteID, short theIndex, UInt32 theOptions);
void						VRSprites_SetMatrix (WindowObject theWindowObject, QTAtomID theSpriteID, MatrixRecord *theMatrix, UInt32 theOptions);
void						VRSprites_SetLocation (WindowObject theWindowObject, QTAtomID theSpriteID, Point *thePoint, UInt32 theOptions);

