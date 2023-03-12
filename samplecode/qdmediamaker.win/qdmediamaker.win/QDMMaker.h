//////////
//
//	File:		QDMMaker.h
//
//	Contains:	Code to create movies that use the derived QuickDraw media handler.
//
//	Written by:	Tim Monroe
//				Based on MyMakeMediaMovies code written by John Wang.
//
//	Copyright:	� 1993-1999 by Apple Computer, Inc., all rights reserved.
//
//	Change History (most recent first):
//
//	   <2>	 	01/14/99	rtm		conversion to personal coding style
//	   <1>	 	02/25/93	jw		first file
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

#ifndef __MEDIAHANDLERS__
#include <MediaHandlers.h>
#endif

#ifndef __ENDIAN__
#include <Endian.h>
#endif

#ifndef __SCRIPT__
#include <Script.h>
#endif

#ifndef __SOUND__
#include <Sound.h>
#endif

#ifndef __FIXMATH__
#include <FixMath.h>
#endif

#ifndef __QDOFFSCREEN__
#include <QDOffscreen.h>
#endif

#ifndef __QDMEDIACOMMON__
#include "QDMediaCommon.h"
#endif

#ifndef __QTUtilities__
#include "QTUtilities.h"
#endif

#include "ComResource.h"
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

#define kQDTrackWidth				400
#define kQDTrackHeight				300
#define kQDMediaTimeScale			600

#define kSaveQDMoviePrompt			"Save movie file as:"
#define kSaveQDMovieFileName		"Untitled.mov"

#define kWindowTitle				"Display Graphics"


//////////
//
// function prototypes
//
//////////

static OSErr						QDMM_AddRowsSamples (Media theMedia, short theWidth, short theHeight, QDrawDescriptionHandle theQDDesc);
static OSErr						QDMM_AddLinesSamples (Media theMedia, short theWidth, short theHeight, QDrawDescriptionHandle theQDDesc);
static OSErr						QDMM_AddBoxesSamples (Media theMedia, short theWidth, short theHeight, QDrawDescriptionHandle theQDDesc);
static OSErr						QDMM_AddBallSamples (Media theMedia, short theWidth, short theHeight, QDrawDescriptionHandle theQDDesc);
void								QDMM_MakeQDMovie (UInt16 theMenuItem, short theWidth, short theHeight);


