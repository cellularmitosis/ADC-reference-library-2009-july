//////////
//
//	File:		QTBigScreen.c
//
//	Contains:	Code for playing QuickTime movies fullscreen.
//
//	Written by:	Tim Monroe
//
//	Copyright:	� 2002 by Apple Computer, Inc., all rights reserved.
//
//	Change History (most recent first):
//
//	   <1>	 	07/01/02	rtm		first file
//
//////////

//////////
//
// header files
//
//////////

#include "QTBigScreen.h"


//////////
//
// global variables
//
//////////

WindowObject					gFullScreenWindowObject = NULL;
Boolean							gEndingFullScreen = false;


//////////
//
// QTBig_InitWindowData
// Do any fullscreen-specific initialization for the specified window.
//
//////////

ApplicationDataHdl QTBig_InitWindowData (WindowObject theWindowObject)
{
	ApplicationDataHdl		myAppData = NULL;

	// if we already have some window data, dump it
	myAppData = (ApplicationDataHdl)QTFrame_GetAppDataFromWindowObject(theWindowObject);
 	if (myAppData != NULL)
		QTBig_DumpWindowData(theWindowObject);

	// allocate a new application data handle
	myAppData = (ApplicationDataHdl)NewHandleClear(sizeof(ApplicationDataRecord));
		
	return(myAppData);
}


//////////
//
// QTBig_DumpWindowData
// Do any fullscreen-specific tear-down for the specified window.
//
//////////

void QTBig_DumpWindowData (WindowObject theWindowObject)
{
	ApplicationDataHdl		myAppData = NULL;
		
	myAppData = (ApplicationDataHdl)QTFrame_GetAppDataFromWindowObject(theWindowObject);
	if (myAppData != NULL) {
		if ((**myAppData).fFullScreenWindow != NULL)
			QTBig_StopFullscreen(theWindowObject);
		
		DisposeHandle((Handle)myAppData);
		(**theWindowObject).fAppData = NULL;
	}
}


//////////
//
// QTBig_StartFullscreen
// Start fullscreen playback.
//
//////////

OSErr QTBig_StartFullscreen (WindowObject theWindowObject)
{
	MovieController 		myMC = NULL;
	Movie			 		myMovie = NULL;
	ApplicationDataHdl		myAppData = NULL;
	long					myFlags = fullScreenAllowEvents;
	OSErr					myErr = paramErr;
		
	if (theWindowObject == NULL)
		goto bail;

	myAppData = (ApplicationDataHdl)QTFrame_GetAppDataFromWindowObject(theWindowObject);
	myMovie = (**theWindowObject).fMovie;
	myMC = (**theWindowObject).fController;
	
	if ((myAppData == NULL) || (myMovie == NULL) || (myMC == NULL))
		goto bail;
	
	if ((**myAppData).fFullScreenWindow == NULL) {
		short				myOrigScreenWidth = 0;
		short				myOrigScreenHeight = 0;
		short				myNewScreenWidth = 0;
		short				myNewScreenHeight = 0;
		short				myScreenWidth = 0;
		short				myScreenHeight = 0;
		short				myMovieWidth = 0;
		short				myMovieHeight = 0;
		Fixed				myScreenRatio;
		Fixed				myMovieRatio;
		Rect				myRect;
		RGBColor 			myColor = {0x0000, 0x0000, 0x0000};	// black
			
		// remember some of the current state
		GetMovieGWorld(myMovie, &(**myAppData).fOrigMovieGWorld, NULL);
		GetMovieBox(myMovie, &(**myAppData).fOrigMovieRect);
		MCGetControllerBoundsRect(myMC, &(**myAppData).fOrigControllerRect);
		(**myAppData).fOrigControllerVis = MCGetVisible(myMC);
		(**myAppData).fOrigControllerAttached = MCIsControllerAttached(myMC);
		(**myAppData).fOrigWindow = (**theWindowObject).fWindow;
		
		// get the current screen resolution
		myErr = BeginFullScreen(&(**myAppData).fRestoreState, NULL, &myScreenWidth, &myScreenHeight, NULL, NULL, fullScreenPreflightSize);
		if (myErr != noErr)
			goto bail;
		
		// keep track of the original screen resolution
		myOrigScreenHeight = myScreenHeight;
		myOrigScreenWidth = myScreenWidth;
		
		// calculate the destination rectangle
		GetMovieNaturalBoundsRect(myMovie, &myRect);
		MacOffsetRect(&myRect, -myRect.left, -myRect.top);
		
		myMovieWidth = myRect.right;
		myMovieHeight = myRect.bottom;
		
		myMovieRatio = FixRatio(myMovieWidth, myMovieHeight);
		myScreenRatio = FixRatio(myScreenWidth, myScreenHeight);

		// scale the movie rectangle to fit the screen ratio
		if (myMovieRatio > myScreenRatio) {
			myMovieHeight = (myScreenWidth * myMovieHeight) / myMovieWidth;
			myMovieWidth = myScreenWidth;
		} else {
			myMovieWidth = (myScreenHeight * myMovieWidth) / myMovieHeight;
			myMovieHeight = myScreenHeight;
		}
		
		MacSetRect(&myRect, 0, 0, myMovieWidth, myMovieHeight);

		myScreenWidth = myMovieWidth;
		myScreenHeight = myMovieHeight;

		// begin full-screen display
		myErr = BeginFullScreen(&(**myAppData).fRestoreState, NULL, &myScreenWidth, &myScreenHeight, &(**myAppData).fFullScreenWindow, &myColor, myFlags); 
		if (myErr != noErr)
			goto bail;

		// determine whether the resolution changed; if it has changed, we must have passed in a supported resolution,
		// so we want the movie to fill the screen; otherwise, we need to offset the movie to center it in the screen
		if ((myScreenWidth == myOrigScreenWidth) && (myScreenHeight == myOrigScreenHeight))
			MacOffsetRect(&myRect, (myScreenWidth - myMovieWidth) / 2, (myScreenHeight - myMovieHeight) / 2);
			
#if TARGET_OS_WIN32
		// on Windows, set a window procedure for the new window
		QTMLSetWindowWndProc((**myAppData).fFullScreenWindow, QTBig_HandleMessages);
#endif

		// hide the original window
		QTFrame_SetWindowVisState(theWindowObject, false);
		
		// attach the existing window object to the new window
#if TARGET_OS_MAC
		SetWRefCon((**myAppData).fFullScreenWindow, (long)theWindowObject);
#endif
#if TARGET_OS_WIN32
		SetWindowLong(GetPortNativeWindow((GrafPtr)(**myAppData).fFullScreenWindow), GWL_USERDATA, (LPARAM)theWindowObject);
#endif
		
		// set the movie and movie controller state to the new window and rectangle 
		SetGWorld(GetWindowPort((**myAppData).fFullScreenWindow), NULL);
		SetMovieGWorld(myMovie, GetWindowPort((**myAppData).fFullScreenWindow), NULL);
		MCSetControllerPort(myMC, GetWindowPort((**myAppData).fFullScreenWindow));
		MCSetControllerAttached(myMC, false);
		MCSetControllerBoundsRect(myMC, &myRect);
		
		SetMovieBox(myMovie, &myRect);
		MCSetVisible(myMC, false);
		MCActivate(myMC, (**myAppData).fFullScreenWindow, true);
		
		(**theWindowObject).fWindow = QTFrame_GetWindowReferenceFromWindow((**myAppData).fFullScreenWindow);

#if TARGET_API_MAC_CARBON
		HiliteWindow((**myAppData).fFullScreenWindow, true);
#endif

#if END_FULLSCREEN_AT_MOVIE_END
		// install a callback procedure to return linear, non-looping movies
		// to normal mode at the end of the movie
		if (QTBig_MovieIsStoppable(myMC))
			QTBig_InstallCallBack(theWindowObject);
#endif
	}
	
	gFullScreenWindowObject = theWindowObject;
	
bail:
	return(myErr);
}


//////////
//
// QTBig_StopFullscreen
// Stop fullscreen playback.
//
//////////

OSErr QTBig_StopFullscreen (WindowObject theWindowObject)
{
	ApplicationDataHdl		myAppData = NULL;
	OSErr					myErr = paramErr;
		
	if (theWindowObject == NULL)
		goto bail;

	myAppData = (ApplicationDataHdl)QTFrame_GetAppDataFromWindowObject(theWindowObject);
	if (myAppData != NULL) {
		if ((**myAppData).fFullScreenWindow != NULL) {

			// restore the original settings
			(**theWindowObject).fWindow = (**myAppData).fOrigWindow;

			MacSetPort(QTFrame_GetPortFromWindowReference((**theWindowObject).fWindow));
			
			SetMovieGWorld((**theWindowObject).fMovie, (CGrafPtr)(**myAppData).fOrigMovieGWorld, GetGWorldDevice((CGrafPtr)(**myAppData).fOrigMovieGWorld));
			SetMovieBox((**theWindowObject).fMovie, &(**myAppData).fOrigMovieRect);

			MCSetControllerPort((**theWindowObject).fController, (CGrafPtr)(**myAppData).fOrigMovieGWorld);
			MCSetControllerAttached((**theWindowObject).fController, (**myAppData).fOrigControllerAttached);
			MCSetVisible((**theWindowObject).fController, (**myAppData).fOrigControllerVis);
			MCSetControllerBoundsRect((**theWindowObject).fController, &(**myAppData).fOrigControllerRect);

			gEndingFullScreen = true;

			// end fullscreen playback
			myErr = EndFullScreen((**myAppData).fRestoreState, 0L);
			
			gEndingFullScreen = false;

			// empty out the data structures and global variables
			(**myAppData).fOrigWindow = NULL;
			(**myAppData).fFullScreenWindow = NULL;
			(**myAppData).fRestoreState = NULL;
			(**myAppData).fOrigMovieGWorld = NULL;
			
			gFullScreenWindowObject = NULL;
						
#if END_FULLSCREEN_AT_MOVIE_END
			// dispose of the CallMeWhen callback and the callback UPP
			if ((**myAppData).fCallBack != NULL)
				DisposeCallBack((**myAppData).fCallBack);
				
			if ((**myAppData).fCallBackUPP != NULL)
				DisposeQTCallBackUPP((**myAppData).fCallBackUPP);
				
			(**myAppData).fCallBack = NULL;
			(**myAppData).fCallBackUPP = NULL;
#endif
				
			// make sure the movie window is the correct size and then show it again
			QTFrame_SizeWindowToMovie(theWindowObject);
			QTFrame_SetWindowVisState(theWindowObject, true);
		}
	}

bail:
	return(myErr);
}


#if END_FULLSCREEN_AT_MOVIE_END
//////////
//
// QTBig_MovieIsStoppable
// Is the specified movie non-looping and non-interactive?
//
//////////

Boolean QTBig_MovieIsStoppable (MovieController theMC)
{
	long		myFlags = 0L;

	MCGetControllerInfo(theMC, &myFlags);
	
	if ((myFlags & mcInfoIsLooping) || (myFlags & mcInfoMovieIsInteractive))
		return(false);
	else
		return(true);
}


//////////
//
// QTBig_InstallCallBack
// Install a CallMeWhen callback procedure.
//
//////////

void QTBig_InstallCallBack (WindowObject theWindowObject)
{
	ApplicationDataHdl		myAppData = NULL;
	QTCallBack				myCallBack = NULL;
		
	if (theWindowObject == NULL)
		return;

	if ((**theWindowObject).fMovie == NULL)
		return;
		
	myAppData = (ApplicationDataHdl)QTFrame_GetAppDataFromWindowObject(theWindowObject);
	if (myAppData == NULL)
		return;
	
	myCallBack = NewCallBack(GetMovieTimeBase((**theWindowObject).fMovie), callBackAtExtremes);
	if (myCallBack != NULL) {
		(**myAppData).fCallBack = myCallBack;
		(**myAppData).fCallBackUPP = NewQTCallBackUPP(QTBig_FullscreenCallBack);
		CallMeWhen(myCallBack, (**myAppData).fCallBackUPP, (long)theWindowObject, triggerAtStop, 0, 0);
	}
}


//////////
//
// QTBig_FullscreenCallBack
// A CallMeWhen callback procedure that ends fullscreen playback.
//
// The theRefCon parameter is assumed to be a WindowObject.
//
//////////

PASCAL_RTN void QTBig_FullscreenCallBack (QTCallBack theCallBack, long theRefCon)
{
	WindowObject 			myWindowObject = (WindowObject)theRefCon;
	ApplicationDataHdl		myAppData = NULL;
	QTCallBack				myCallBack = NULL;
		
	if (myWindowObject == NULL)
		return;

	myAppData = (ApplicationDataHdl)QTFrame_GetAppDataFromWindowObject(myWindowObject);
	if (myAppData == NULL)
		return;

	if ((**myAppData).fCallBack != theCallBack)
		return;

	// mark this window for ending fullscreen mode
	(**myAppData).fEndFullscreenNeeded = true;

	// clean up the callback stuff
	if ((**myAppData).fCallBack != NULL)
		DisposeCallBack((**myAppData).fCallBack);
		
	if ((**myAppData).fCallBackUPP != NULL)
		DisposeQTCallBackUPP((**myAppData).fCallBackUPP);
		
	(**myAppData).fCallBack = NULL;
	(**myAppData).fCallBackUPP = NULL;
}
#endif	// END_FULLSCREEN_AT_MOVIE_END


#if TARGET_OS_WIN32
//////////
//
// QTBig_HandleMessages
// Handle Windows messages for the full-screen window.
// 
//////////

LRESULT CALLBACK QTBig_HandleMessages (HWND theWnd, UINT theMessage, UINT wParam, LONG lParam)
{
	MovieController			myMC = NULL;
	Movie					myMovie = NULL;
	WindowObject			myWindowObject = NULL;
	MSG						myMsg = {0};
	EventRecord				myMacEvent;
	LONG					myPoints = GetMessagePos();
	BOOL					myIsHandled = false;

	if (gFullScreenWindowObject == NULL)
		goto bail;

	// make sure we don't get called while the movie is returning to normal state
	if (gEndingFullScreen)
		goto bail;
	
	// get the window object, movie, and movie controller for this window
	myWindowObject = gFullScreenWindowObject;		
	if (myWindowObject != NULL) {
		myMC = (**myWindowObject).fController;
		myMovie = (**myWindowObject).fMovie;
	}
		
	// give the movie controller this message first
	if (myMC != NULL) {
		LONG			myPoints = GetMessagePos();

		myMsg.hwnd = theWnd;
		myMsg.message = theMessage;
		myMsg.wParam = wParam;
		myMsg.lParam = lParam;
		myMsg.time = GetMessageTime();
		myMsg.pt.x = LOWORD(myPoints);
		myMsg.pt.y = HIWORD(myPoints);

		// translate a Windows event to a Mac event
		WinEventToMacEvent(&myMsg, &myMacEvent);

		// let the application-specific code have a chance to intercept the event
		myIsHandled = QTApp_HandleEvent(&myMacEvent);
	}

	switch (theMessage) {
		case WM_CHAR:
			// do any application-specific key press handling
			myIsHandled = QTApp_HandleKeyPress((char)wParam);
			break;
	}
	
bail:

	return(DefWindowProc(theWnd, theMessage, wParam, lParam));
}
#endif	// TARGET_OS_WIN32


//////////
//
// QTFrame_SetWindowVisState
// Show or hide the window associated with the specified window object.
//
//////////

void QTFrame_SetWindowVisState (WindowObject theWindowObject, Boolean theState)
{
	// make sure we have a non-NULL window object and window
	if (theWindowObject == NULL)
		return;
		
	if ((**theWindowObject).fWindow == NULL)
		return;
		
	// set the visibility state of the window
#if TARGET_OS_MAC
	if (theState)
		MacShowWindow((**theWindowObject).fWindow);
	else
		HideWindow((**theWindowObject).fWindow);
#endif
#if TARGET_OS_WIN32
	ShowWindow((**theWindowObject).fWindow, theState);		
#endif
}