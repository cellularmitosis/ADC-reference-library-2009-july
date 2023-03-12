//////////
//
//	File:		VRActions.h
//
//	Contains:	Support for reacting to QuickTime wired actions in VR nodes.
//
//	Written by:	Tim Monroe
//
//	Copyright:	© 1999 by Apple Computer, Inc., all rights reserved.
//
//	Change History (most recent first):
//
//	   <1>	 	08/05/99	rtm		first file
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

// special value for fEventType field of wired action commands
#define kVRAnyEvent			(OSType)-1

// special value for fID field of wired action commands
#define kVRAnyItemID		(UInt32)-1


//////////
//
// function prototypes
//	   
//////////

void						VRActions_InitWindowData (WindowObject theWindowObject);
void						VRActions_DumpWindowData (WindowObject theWindowObject);

