//////////
//
//	File:		SoundConverter.h
//
//	Contains:	Sound format conversion sample code.
//
//	Written by:	Bob Aron
//	Revised by:	Tim Monroe
//
//	Copyright:	© 1999 by Apple Computer, Inc., all rights reserved.
//
//	Change History (most recent first):
//
//	   <1>	 	07/01/99	rtm		first file from Bob Aron; conversion to personal coding style; updated to latest headers
//	   
//////////

//////////
//
// header files
//
//////////

#ifndef __MACERRORS__
#include <MacErrors.h>
#endif

#ifndef __COMPONENTS__
#include <Components.h>
#endif

#ifndef __FIXMATH__
#include <FixMath.h>
#endif

#ifndef __MOVIES__
#include <Movies.h>
#endif

#ifndef __MOVIESFORMAT__
#include <MoviesFormat.h>
#endif

#ifndef __QUICKTIMECOMPONENTS__
#include <QuickTimeComponents.h>
#endif

#ifndef __SOUND__
#include <Sound.h>
#endif

#ifndef __STRINGS__
#include <Strings.h>
#endif

#ifndef _STRING_H
#include <string.h>
#endif

#ifndef __QTUtilities__
#include "QTUtilities.h"
#endif

#if TARGET_OS_MAC
#include "MacFramework.h"
#endif

#if TARGET_OS_WIN32
#include "WinFramework.h"
#endif

#if TARGET_OS_WIN32
#include <math.h>
#define double_t		double
#endif

#if TARGET_OS_MAC
	#ifndef __FP__
	#include <fp.h>
	#endif
#endif


//////////
//
// compiler macros
//
//////////

#define FailIf(cond, handler)								\
	if (cond) {												\
		goto handler;										\
	}

#define FailWithAction(cond, action, handler)				\
	if (cond) {												\
		{ action; }											\
		goto handler;										\
	}


//////////
//
// constants
//
//////////

#define kMaxBufferSize				(20*1024) 									// the upper limit for the in and out conversion buffers
#define kSaveSoundPrompt			"Save sound movie file as:"
#define kSaveSoundFileName			"Sound.mov"
#define kConcertA					440


//////////
//
// function prototypes
//
//////////

void								SndConv_DriveAudioConversion (void);

OSErr								SndConv_ConvertSomeUncompressedAudio (	
												Handle theSourceHandle,
												SoundComponentData theSourceInfo,
												unsigned long theSourceTotalFrames, 
												Handle theDestHandle,
												SoundComponentData theDestInfo,
												unsigned long *theDestFramesMoved, 
												CompressionInfo *theDestCompInfo,
												Handle *theDestCompParams);

OSErr								SndConv_CreateSoundMovie (	
												Handle theDestAudioData,
												short theMovieRefNum, 
												Movie theMovie,
												SoundComponentData theDestInfo, 
												Handle *theDestCompParams,
												CompressionInfo theDestCompInfo, 
												unsigned long theDestFrameCount);

OSErr								SndConv_UncompressedSineWaveToHandle (
												Handle theData,
												SoundComponentData *theCompInfo,
												unsigned long *theTotalFrames);
