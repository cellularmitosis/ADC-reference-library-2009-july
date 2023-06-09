//////////
//
//	File:		VRPicture.h
//
//	Contains:	Headers for drawing pictures into the prescreen buffer.
//
//	Written by:	Tim Monroe
//
//	Copyright:	� 1994-1997 by Apple Computer, Inc., all rights reserved.
//
//	Change History (most recent first):
//
//	   <1>	 	01/28/97	rtm		first file
//	   
//////////

#pragma once

// header files
#include <QuickTimeVR.h>

#include "ComApplication.h"

#if TARGET_OS_MAC
#include "MacFramework.h"
#endif

#if TARGET_OS_WIN32
#include "WinFramework.h"
#endif

// function prototypes
void								VRPicture_ShowPicture (WindowObject theWindowObject, UInt32 theResID, UInt32 theEntryID, UInt32 theHeight, UInt32 theWidth, UInt32 thePegSides, UInt32 theOffset, UInt32 theOptions);
void								VRPicture_DrawNodePictures (WindowObject theWindowObject);
void								VRPicture_DumpNodePictures (WindowObject theWindowObject);
void								VRPicture_DumpEntryMem (VRScriptPicturePtr theEntry);
