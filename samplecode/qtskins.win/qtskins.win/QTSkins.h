//////////
//
//	File:		QTSkins.h
//
//	Contains:	Sample code for using QuickTime's skins.
//
//	Written by:	Tim Monroe
//
//	Copyright:	� 2000 by Apple Computer, Inc., all rights reserved.
//
//	Change History (most recent first):
//
//	   <1>	 	11/15/00	rtm		first file 
//	   
//////////

//////////
//	   
// header files
//	   
//////////

#ifndef __MOVIES__
#include <Movies.h>
#endif

#ifndef __QUICKTIMECOMPONENTS__
#include <QuickTimeComponents.h>
#endif

#ifndef __MEDIAHANDLERS__
#include <MediaHandlers.h>
#endif

#ifndef __FIXMATH__
#include <FixMath.h>
#endif

#include "ComApplication.h"


//////////
//	   
// compiler macros
//	   
//////////

#if TARGET_OS_WIN32
#define HiWord						HIWORD
#define LoWord						LOWORD
#define GetPortPixMap(port)			((port)->portPixMap)
#endif


//////////
//	   
// constants
//	   
//////////


//////////
//	   
// function prototypes
//	   
//////////

OSErr								QTSkin_AddSkinTrack (Movie theMovie);
PicHandle							QTSkin_GetPicHandleFromFile (void);
#if TARGET_OS_MAC
PASCAL_RTN Boolean					QTSkin_FileFilterFunction (AEDesc *theItem, void *theInfo, void *theCallBackUD, NavFilterModes theFilterMode);
#endif
#if TARGET_OS_WIN32
PASCAL_RTN Boolean					QTSkin_FileFilterFunction (CInfoPBPtr thePBPtr);
#endif

void								QTSkin_Init (void);
void								QTSkin_Stop (void);
static PASCAL_RTN long				QTSkin_SkinWindowDef (short theVarCode, WindowRef theWindow, short theMessage, long theParam);
WindowReference						QTSkin_CreateSkinsWindow (void);
OSErr								QTSkin_ConvertPictureToRegion (PicHandle thePicture, RgnHandle *theRegionPtr);
ApplicationDataHdl					QTSkin_InitWindowData (WindowObject theWindowObject);
void								QTSkin_DumpWindowData (WindowObject theWindowObject);
#if TARGET_OS_WIN32
Boolean								QTSkin_IsDragClick (WindowObject theWindowObject, LONG lParam);
#endif

Boolean								QTSkin_IsSkinnedMovie (Movie theMovie) ;