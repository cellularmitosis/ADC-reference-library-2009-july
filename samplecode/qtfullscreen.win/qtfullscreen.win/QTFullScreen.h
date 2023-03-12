//////////
//
//	File:		QTFullScreen.h
//
//	Contains:	Functions to display full-screen QuickTime movies.
//
//	Written by:	Tim Monroe
//
//	Copyright:	© 1997 by Apple Computer, Inc., all rights reserved.
//
//	Change History (most recent first):
//
//	   <1>	 	12/22/97	rtm		first file
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

#ifndef __QTML__
#include <QTML.h>
#endif


//////////
//
// function prototypes
//
//////////

OSErr						QTFullScreen_PlayOnFullScreen (FSSpecPtr theFSSpecPtr);
OSErr						QTFullScreen_RestoreScreen (void);
OSErr						QTFullScreen_EventLoopAction (EventRecord *theEvent);

#if TARGET_OS_WIN32
LRESULT CALLBACK			QTFullScreen_HandleMessages (HWND theWnd, UINT theMessage, UINT wParam, LONG lParam);
#endif

PASCAL_RTN void				QTFullScreen_MoviePrePrerollCompleteProc (Movie theMovie, OSErr thePrerollErr, void *theRefcon);
OSErr						QTFullScreen_PlayMovieOnFullScreen (Movie theMovie);

