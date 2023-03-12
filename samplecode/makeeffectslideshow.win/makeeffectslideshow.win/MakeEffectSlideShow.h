//////////
//
//	File:		MakeEffectSlideShow.h
//
//	Contains:	QuickTime video effect support for QuickTime movies.
//				This file is used for BOTH MacOS and Windows.
//
//	Written by:	Tim Monroe
//				Based (heavily!) on the MakeEffectSlideShow code written by Sam Bushell.
//
//	Copyright:	© 1994-1997 by Apple Computer, Inc., all rights reserved.
//
//	Change History (most recent first):
//
//	   <1>	 	11/06/97	rtm		first file; integrated existing code with shell framework
//	   
//////////

//////////
//
// header files
//
//////////

#include "ComApplication.h"

#include <ImageCodec.h>

#if TARGET_OS_MAC
#include "MacFramework.h"
#endif

#if TARGET_OS_WIN32
#include "WinFramework.h"
#endif


//////////
//
// compiler flags and macros
//
//////////

#define USES_MAKE_IMAGE_DESC_FOR_EFFECT	1		// use MakeImageDescriptionForEffect (QT 4.0 and later)

#define	BailNil(n)						if (!n) { if (!myErr) myErr = -1; goto bail; }
#define	BailError(n)					if (n) { if (!myErr) myErr = -1; goto bail; }

#define MAKE_STILL_SECTIONS				0		// should we bother with the non-effect parts of the movie?


//////////
//
// constants
//
//////////

#define kMaxNumSources					100		// the maximum number of input tracks we will collect
#define kTimeScale						600

#if MAKE_STILL_SECTIONS
#define	kStillDuration					(2*kTimeScale)
#define	kEffectDuration					(1*kTimeScale)
#define kMinimumDuration				(kEffectDuration + kStillDuration + kEffectDuration)
#else
#define	kStillDuration					0
#define	kEffectDuration					(2*kTimeScale)
#define kMinimumDuration				(kEffectDuration + kStillDuration + kEffectDuration)
#endif

#define kNoSourceName					FOUR_CHAR_CODE('none')
#define kSourceOneName					FOUR_CHAR_CODE('srca')
#define kSourceTwoName					FOUR_CHAR_CODE('srcb')

// constants to pick out portions of a track; use these as parameters to QTEffects_CopyPortionOfTrackToTrack
enum {
	eStartPortion			= 1,		// the first kEffectDuration of a track
	eMiddlePortion			= 2,		// the part between eStartPortion and eFinishPortion
	eFinishPortion			= 4			// the last kEffectDuration of a track
};

#define kEffectsSaveMoviePrompt			"Save Effect Movie File As:"
#define kEffectsSaveMovieFileName		"untitled.mov"

//////////
//
// function prototypes
//
//////////

OSErr								QTEffects_GetFirstVideoTrackInMovie (Movie theMovie, Track *theTrack);
PicHandle							QTEffects_GetPosterPictFromFirstVideoTrackInMovieFile (FSSpec *theSpec);
OSErr								QTEffects_CopyPortionOfTrackToTrack (Track theSourceTrack, UInt16 theSourcePortions, Track theDestTrack, TimeValue theDestStartTime, TimeValue *theDestDuration);
OSErr								QTEffects_DisplayDialogForSources (FSSpec *theSpecList, UInt16 theSpecCount);
void								QTEffects_RespondToDialogSelection (OSErr theErr);
# if TARGET_OS_WIN32
static void							QTEffects_EffectsDialogCallback (EventRecord *theEvent, DialogRef theDialog, DialogItemIndex theItemHit);
LRESULT CALLBACK					QTEffects_CustomDialogWndProc (HWND theWnd, UINT theMessage, UINT wParam, LONG lParam);
#endif
Boolean								QTEffects_HandleEffectsDialogEvents (EventRecord *theEvent, DialogItemIndex theItemHit);
void								QTEffects_PromptUserForFilesAndMakeEffect (void);
