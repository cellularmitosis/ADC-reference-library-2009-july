//////////
//
//	File:		VRMovies.h
//
//	Contains:	Support for QuickTime movie playback in VR nodes.
//
//	Written by:	Tim Monroe
//
//	Copyright:	© 1997 by Apple Computer, Inc., all rights reserved.
//
//	Change History (most recent first):
//
//	   <1>	 	12/11/96	rtm		first file 
//	   
//////////

#pragma once


//////////
//
// header files
//	   
//////////

#ifndef __QDOFFSCREEN__
#include <QDOffscreen.h>
#endif

#include "ComApplication.h"
#include "VRScript.h"

#if TARGET_OS_MAC
#include "MacFramework.h"
#endif

#if TARGET_OS_WIN32
#include "WinFramework.h"
#endif


//////////
//
// compiler macros
//	   
//////////

#define RECT_WIDTH(rect)			((rect).right-(rect).left)
#define RECT_HEIGHT(rect)			((rect).bottom-(rect).top)


//////////
//
// constants
//	   
//////////

#define kVRMoov_HideHotSpotType		FOUR_CHAR_CODE('hide')


//////////
//
// function prototypes
//	   
//////////

void								VRMoov_Init (void);
void								VRMoov_Stop (void);
void								VRMoov_StartMovie (Movie theMovie);
void								VRMoov_PlayMovie (WindowObject theWindowObject, UInt32 theNodeID, UInt32 theEntryID, float thePanAngle, float theTiltAngle, float theScale, float theWidth, UInt32 theKeyRed, UInt32 theKeyGreen, UInt32 theKeyBlue, Boolean theUseBuffer, Boolean theUseCenter, Boolean theUseKey, Boolean theUseHide, Boolean theUseDir, Boolean theRotate, float theVolAngle, UInt32 theMode, UInt32 theOptions, char *thePathName);
void								VRMoov_PlayTransitionMovie (WindowObject theWindowObject, UInt32 theOptions, char *thePathName);
Boolean								VRMoov_LoadEmbeddedMovie (char *thePathName, WindowObject theWindowObject, VRScriptMoviePtr theEntry);
void								VRMoov_LoopEmbeddedMovie (Movie theMovie, Boolean isPalindrome);
Boolean								VRMoov_DoIdle (WindowObject theWindowObject);
void								VRMoov_DumpNodeMovies (WindowObject theWindowObject);
void								VRMoov_DumpSceneMovies (WindowObject theWindowObject);
void								VRMoov_DumpSelectedMovies (WindowObject theWindowObject, UInt32 theOptions);
VRScriptMoviePtr					VRMoov_GetEmbeddedVideo (WindowObject theWindowObject);
float								VRMoov_GetEmbeddedMovieWidth (WindowObject theWindowObject);
void								VRMoov_SetEmbeddedMovieWidth (WindowObject theWindowObject, float theWidth);
void								VRMoov_GetEmbeddedMovieCenter (WindowObject theWindowObject, QTVRFloatPoint *theCenter);
void								VRMoov_SetEmbeddedMovieCenter (WindowObject theWindowObject, const QTVRFloatPoint *theCenter);
float								VRMoov_GetEmbeddedMovieScale (WindowObject theWindowObject);
void								VRMoov_SetEmbeddedMovieScale (WindowObject theWindowObject, float theScale);
void								VRMoov_GetEmbeddedMovieRect (WindowObject theWindowObject, Rect *theRect);
void								VRMoov_SetEmbeddedMovieRect (WindowObject theWindowObject, const Rect *theRect);
void								VRMoov_SetAllBalanceAndVolume (WindowObject theWindowObject, float thePan, float theTilt);
void								VRMoov_SetOneBalanceAndVolume (Movie theMovie, MediaHandler theMediaHandler, float thePan, float theTilt, float theMoviePan, float theVolAngle);
OSErr								VRMoov_CalcImagingMatrix (WindowObject theWindowObject, Rect *theBBufRect);
OSErr								VRMoov_SetupDecompSeq (VRScriptMoviePtr theEntry, GWorldPtr theDestGWorld);
OSErr								VRMoov_RemoveDecompSeq (VRScriptMoviePtr theEntry);
PASCAL_RTN OSErr					VRMoov_BackBufferImagingProc (QTVRInstance theInstance, Rect *theRect, UInt16 theAreaIndex, UInt32 theFlagsIn, UInt32 *theFlagsOut, SInt32 theRefCon);
PASCAL_RTN OSErr					VRMoov_CoverProc (Movie theMovie, RgnHandle theRegion, SInt32 theRefCon);
void								VRMoov_SetVideoGraphicsMode (Movie theMovie, VRScriptMoviePtr theEntry, Boolean theSetVGM);
VRScriptMoviePtr					VRMoov_GetFinishedMovie (WindowObject theWindowObject);
void								VRMoov_CheckForCompletedMovies (WindowObject theWindowObject);
void								VRMoov_DumpEntryMem (VRScriptMoviePtr theEntry);
