//////////
//
//	File:		QTText.c
//
//	Contains:	QuickTime text media handler sample code.
//
//	Written by:	Tim Monroe
//				parts based on QTTextSample code by Nick Thompson (see develop, issue 20).
//
//	Copyright:	© 1995-1998 by Apple Computer, Inc., all rights reserved.
//
//	Change History (most recent first):
//
//	   <1>	 	11/14/97	rtm		first file; conversion to personal coding style
//	   
//////////

//////////
//
// header files
//
//////////

#ifndef __FIXMATH__
#include <FixMath.h>
#endif

#ifndef __MOVIES__
#include <Movies.h>
#endif

#ifndef __SCRIPT__
#include <Script.h>
#endif

#ifndef __STANDARDFILE__
#include <StandardFile.h>
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
// compiler flags
//
//////////

#define USE_MOVIESEARCHTEXT		1		// do we use MovieSearchText or TextMediaFindNextText to find text?
#define USE_ADDMEDIASAMPLE		0		// do we use AddMediaSample or TextMediaAddTextSample to add a text track?


//////////
//
// constants
//
//////////

#define kAllTextTracks			0		// special index value to be passed to QTText_RemoveIndTextTrack
#define kTextTrackHeight		20		// default height (in pixels) of the new text track

#define kSearchText				"QuickTime"
#define kSampleText				""

#define kBogusStartingTime		-1			// an invalid starting time

#define kHREFTrackName			"HREFTrack"
#define kNonHREFTrackName		"Text Track"


//////////
//
// function prototypes
//
//////////

ApplicationDataHdl			QTText_InitWindowData (WindowObject theWindowObject);
void						QTText_DumpWindowData (WindowObject theWindowObject);
void						QTText_SyncWindowData (WindowObject theWindowObject);
void						QTText_SetSearchText (void);
void						QTText_FindText (WindowObject theWindowObject, Str255 theText);
void						QTText_EditText (WindowObject theWindowObject);
PASCAL_RTN OSErr			QTText_TextProc (Handle theText, Movie theMovie, short *theDisplayFlag, long theRefCon);
Track						QTText_AddTextTrack (Movie theMovie, char *theStrings[], short theFrames[], short theNumFrames, OSType theType, Boolean isChapterTrack);
OSErr						QTText_RemoveIndTextTrack (WindowObject theWindowObject, short theIndex);

OSErr						QTText_SetTextTrackAsChapterTrack (WindowObject theWindowObject, OSType theType, Boolean isChapterTrack);
Boolean						QTText_TrackTypeHasAChapterTrack (Movie theMovie, OSType theType);
Boolean						QTText_TrackHasAChapterTrack (Track theTrack);
Boolean						QTText_MovieHasAChapterTrack (Movie theMovie);
Track						QTText_GetChapterTrackForTrack (Track theTrack);
Track						QTText_GetChapterTrackForMovie (Movie theMovie);
Boolean						QTText_IsChapterTrack (Track theTrack);
OSErr						QTText_GetFirstChapterTime (Track theChapterTrack, TimeValue *theTime);
OSErr						QTText_GetNextChapterTime (Track theChapterTrack, TimeValue *theTime);
TimeValue					QTText_GetIndChapterTime (Track theChapterTrack, long theIndex);
char *						QTText_GetIndChapterText (Track theChapterTrack, long theIndex);
long						QTText_GetChapterCount (Track theChapterTrack);

OSErr						QTText_SetTextTrackAsHREFTrack (Track theTrack, Boolean isHREFTrack);
Boolean						QTText_IsHREFTrack (Track theTrack);

void						QTText_CopyCStringToPascal (const char *theSrc, Str255 theDst);
