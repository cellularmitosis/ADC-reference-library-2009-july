//////////
//
//	File:		QTFullScreen.c
//
//	Contains:	Functions to display full-screen QuickTime movies.
//
//	Written by:	Tim Monroe
//
//	Copyright:	� 1998 by Apple Computer, Inc., all rights reserved.
//
//	Change History (most recent first):
//
//	   <4>	 	03/21/00	rtm		changed QTFullScreen_PlayOnFullScreen to use FSSpec parameter
//	   <3>	 	04/30/99	rtm		added QTFullScreen_PlayMovieOnFullScreen, to play an existing
//									movie full screen
//	   <2>	 	03/17/98	rtm		finally got back to this; now it's working on Mac and Windows
//	   <1>	 	12/22/97	rtm		first file
//
//	This file contains functions that illustrate how to play QuickTime movies full screen. The
//	key elements to displaying full screen movies are the calls BeginFullScreen and EndFullScreen,
//	introduced in QuickTime 2.5. In the function QTFullScreen_PlayOnFullScreen, we open a movie
//  file, get the movie from it, configure it to play full screen, associate a movie controller,
//	and then let the controller handle events. Your application should call the function
//	QTFullScreen_EventLoopAction in its event loop (on MacOS) or when it gets idle events (on Windows).
//
//	In the function QTFullScreen_PlayMovieOnFullScreen, we take an existing movie and play it full
//	screen; in this function, we use the Movie Toolbox to start the movie and to give it processor
//	time.
// 
//////////

#include "QTFullScreen.h"


// gloabl variables
WindowPtr					gFullScreenWindow = NULL;		// the full-screen window
MovieController				gMC = NULL;						// movie controller for the full-screen window
Ptr							gRestoreState = NULL;			// restore state; used when closing the full-screen window


//////////
//
// QTFullScreen_PlayOnFullScreen
// Prompt the user for a movie and play it full screen.
//
//////////

OSErr QTFullScreen_PlayOnFullScreen (FSSpecPtr theFSSpecPtr)
{
	Movie				myMovie = NULL;
	short				myRefNum = 0;
	long				myFlags = fullScreenDontChangeMenuBar | fullScreenAllowEvents;
	OSErr				myErr = noErr;
	
	myErr = OpenMovieFile(theFSSpecPtr, &myRefNum, fsRdPerm);
	if (myErr != noErr)
		goto bail;

	// now fetch the first movie from the file
	myErr = NewMovieFromFile(&myMovie, myRefNum, NULL, NULL, newMovieActive, NULL);
	if (myErr != noErr)
		goto bail;
	
	CloseMovieFile(myRefNum);

	// set up for full-screen display
	myErr = BeginFullScreen(&gRestoreState, NULL, 0, 0, &gFullScreenWindow, NULL, myFlags); 

#if TARGET_OS_WIN32
	// on Windows, set a window procedure for the new window and associate a port with that window
	QTMLSetWindowWndProc(gFullScreenWindow, QTFullScreen_HandleMessages);
	CreatePortAssociation(GetPortNativeWindow(gFullScreenWindow), NULL, 0L);
#endif
	
	SetMovieGWorld(myMovie, (CGrafPtr)gFullScreenWindow, GetGWorldDevice((CGrafPtr)gFullScreenWindow));
	SetMovieBox(myMovie, &gFullScreenWindow->portRect);

	// create the movie controller
	gMC = NewMovieController(myMovie, &gFullScreenWindow->portRect, 0);

bail:
	return(myErr);
}


//////////
//
// QTFullScreen_RestoreScreen
//
//////////

OSErr QTFullScreen_RestoreScreen (void)
{
	OSErr		myErr = noErr;
	
#if TARGET_OS_WIN32	
	DestroyPortAssociation((CGrafPtr)gFullScreenWindow);
#endif

	DisposeMovieController(gMC);
	myErr = EndFullScreen(gRestoreState, 0L); 
	
	return(myErr);
}


//////////
//
// QTFullScreen_EventLoopAction
// Do any required event loop action processing.
//
//////////

OSErr QTFullScreen_EventLoopAction (EventRecord *theEvent)
{
	return(MCIsPlayerEvent(gMC, theEvent));
}


#if TARGET_OS_WIN32
//////////
//
// QTFullScreen_HandleMessages
// Handle Windows messages for the full-screen window.
// 
//////////

LRESULT CALLBACK QTFullScreen_HandleMessages (HWND theWnd, UINT theMessage, UINT wParam, LONG lParam)
{
	MSG				myMsg = {0};
	EventRecord		myMacEvent;
	LONG			myPoints = GetMessagePos();

	myMsg.hwnd = theWnd;
	myMsg.message = theMessage;
	myMsg.wParam = wParam;
	myMsg.lParam = lParam;
	myMsg.time = GetMessageTime();
	myMsg.pt.x = LOWORD(myPoints);
	myMsg.pt.y = HIWORD(myPoints);

	// translate the Windows message to a Mac event
	WinEventToMacEvent(&myMsg, &myMacEvent);

	// pass the Mac event to the movie controller
	QTFullScreen_EventLoopAction(&myMacEvent);
		
	return(DefWindowProc(theWnd, theMessage, wParam, lParam));
}
#endif	// TARGET_OS_WIN32


//////////
//
// QTFullScreen_MoviePrePrerollCompleteProc
// A completion procedure for pre-prerolling movies.
//
//////////

PASCAL_RTN void QTFullScreen_MoviePrePrerollCompleteProc (Movie theMovie, OSErr thePrerollErr, void *theRefcon)
{
#pragma unused(thePrerollErr, theRefcon)
	StartMovie(theMovie);
}


//////////
//
// QTFullScreen_PlayMovieOnFullScreen
// Play the specified movie full screen (without a movie controller).
//
//////////

OSErr QTFullScreen_PlayMovieOnFullScreen (Movie theMovie)
{
	long				myFlags = fullScreenAllowEvents;
	GWorldPtr			myOrigGWorld = NULL;
	Rect				myRect;
	OSErr				myErr = noErr;
	
	StopMovie(theMovie);
	
	// set up for full-screen display
	myErr = BeginFullScreen(&gRestoreState, NULL, 0, 0, &gFullScreenWindow, NULL, myFlags); 

#if TARGET_OS_WIN32
	// on Windows, set a window procedure for the new window and associate a port with that window
	QTMLSetWindowWndProc(gFullScreenWindow, QTFullScreen_HandleMessages);
	CreatePortAssociation(GetPortNativeWindow(gFullScreenWindow), NULL, 0L);
#endif
	
	GetMovieBox(theMovie, &myRect);
	GetMovieGWorld(theMovie, &myOrigGWorld, NULL);
	SetMovieGWorld(theMovie, (CGrafPtr)gFullScreenWindow, GetGWorldDevice((CGrafPtr)gFullScreenWindow));
	SetMovieBox(theMovie, &gFullScreenWindow->portRect);
			
	PrePrerollMovie(theMovie, GetMovieTime(theMovie, NULL), GetMoviePreferredRate(theMovie), NewMoviePrePrerollCompleteProc(QTFullScreen_MoviePrePrerollCompleteProc), (void *)0L);	

	// play the movie through until the end; of course, a real application would probably want
	// to call MoviesTask in its idle routine instead of just looping mindlessly here; this is
	// left as an exercise for the reader
	while (!IsMovieDone(theMovie))
		MoviesTask(theMovie, 0L);

	StopMovie(theMovie);
	SetMovieGWorld(theMovie, myOrigGWorld, NULL);
	
	myErr = EndFullScreen(gRestoreState, 0L); 
	
	SetMovieBox(theMovie, &myRect);

bail:
	return(myErr);
}

