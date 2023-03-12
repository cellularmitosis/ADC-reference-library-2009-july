/*
	File:		QTCode.c

	Contains:	QuickTime 3.0 Offscreen sample application.

	Written by:	Scott Kuechle

	Copyright:	© 1998 by Apple Computer, Inc., all rights reserved.

	Change History (most recent first):

	   <2>	 	5/18/98		rtm		changed QTCode_NewMovieFromWinPathname to use NativePathNameToFSSpec
	   <1>	 	4/24/98		srk		first file
	    
 
 NOTES:

 
 TO DO:

*/


#include "QTCode.h"


void QTCode_ReMapMovieBounds(Movie theMovie);




/* ---------------------------------------------------------------- */
/*                                                                  */
/*    QTCode_CreateOffscreen()                                      */
/*                                                                  */
/*                                                                  */
/*    Creates a Mac offscreen GWorld using the NewGWorldFromHBITMAP */
/*    function                                                      */
/*                                                                  */
/*                                                                  */
/* ---------------------------------------------------------------- */

QDErr QTCode_CreateOffscreen(GWorldPtr		*offscreenGWorld,
							CTabHandle 		cTable,
							GDHandle 		aGDevice,
							GWorldFlags 	flags,
							HBITMAP			newHBITMAP,	/* a pointer to a valid bitmap or NULL */
							HDC				newHDC)		/* a pointer to a valid device context or NULL */
{
	QDErr err;

	/* pre-flight our input parameters */
	if ( ((newHBITMAP == NULL) && (newHDC != NULL)) ||
			((newHBITMAP != NULL) && (newHDC == NULL)) )
	{
		return paramErr;
	}
	else
	{
		err = NewGWorldFromHBITMAP(offscreenGWorld,
									cTable,			/*	CTabHandle 		cTable		*/
									aGDevice,		/*	GDHandle 		aGDevice	*/
									flags,			/*	GWorldFlags		flags		*/
									newHBITMAP,
									newHDC);
		return (err);
	}

}

/* ---------------------------------------------------------------- */
/*                                                                  */
/*    QTCode_ForceMovieRedraw()                                     */
/*                                                                  */
/*                                                                  */
/*    Invalidates a movies display state so subsequent calls to     */
/*    MoviesTask will force the movie toolbox to redraw the movie   */
/*                                                                  */
/*                                                                  */
/* ---------------------------------------------------------------- */

void QTCode_ForceMovieRedraw(Movie theMovie) 
{
	OSErr 		err = noErr;
	Rect		movieRect;
	RgnHandle	clipRegion = NULL;

	
		if (theMovie == NULL) goto bail;
	
		clipRegion = NewRgn();
		if (clipRegion == NULL) goto bail;
		
		GetClip(clipRegion);
		GetMovieBox(theMovie, &movieRect); 
		ClipRect(&movieRect);
	
		UpdateMovie(theMovie);
		MoviesTask(theMovie, 0);
	
		SetClip(clipRegion);
	
			/* Closure. Clean up if we have handles. */
	bail:	
	
		if	(clipRegion != NULL)
		{
			DisposeRgn(clipRegion);
		}
}

/* ---------------------------------------------------------------- */
/*                                                                  */
/*    QTCode_SetMovieGWorld()                                       */
/*                                                                  */
/*                                                                  */
/*    Sets the graphics world for displaying a movie                */
/*                                                                  */
/*                                                                  */
/*                                                                  */
/* ---------------------------------------------------------------- */

void QTCode_SetMovieGWorld(Movie theMovie, GWorldPtr offscreen) 
{
	SetMovieGWorld(theMovie, offscreen, GetGWorldDevice(offscreen));
}

/* ---------------------------------------------------------------- */
/*                                                                  */
/*    QTCode_GetStartPointOfFirstVideoSample()                      */
/*                                                                  */
/*                                                                  */
/*    Return the time value of the first video sample found in the  */
/*    movie in the startpoint parameter. If the function fails,     */
/*    startPoint will contain -1 and the OSErr is also returned     */
/*                                                                  */
/* ---------------------------------------------------------------- */

OSErr QTCode_GetStartPointOfFirstVideoSample(Movie		theMovie,
											TimeValue	*startPoint) 
{
	OSErr	anErr = noErr;
	OSType	media = VideoMediaType;
	
	*startPoint = -1;

	GetMovieNextInterestingTime(theMovie, nextTimeMediaSample+nextTimeEdgeOK, (TimeValue)1, &media, 0, 
								fixed1, startPoint, NULL);
	anErr = GetMoviesError();

	return anErr;
}

/* ---------------------------------------------------------------- */
/*                                                                  */
/*    QTCode_NewMovieFromWinPathname()                              */
/*                                                                  */
/*                                                                  */
/*    Creates a new movie, given a windows pathname to the desired  */
/*    movie file location                                           */
/*                                                                  */
/*                                                                  */
/* ---------------------------------------------------------------- */

OSErr QTCode_NewMovieFromWinPathname(char	*pathName,		/* c-string path to movie */
									Movie	*theMovie,		/* Movie returned here */
									short	*movieRefNum,	/* Movie RefNum returned here */
									short	*movieResId)	/* Movie Resource ID returned here */
{
	FSSpec	fileSpec;
	OSErr	err;


	*theMovie = NULL;
	*movieRefNum = 0;
	*movieResId = 0;

	// Open up the movie file...

	// Fill in an FSSpec.
	NativePathNameToFSSpec(pathName, &fileSpec, 0L);

	err = OpenMovieFile(&fileSpec, movieRefNum, fsRdWrPerm);
	if (err)
	{
		err = OpenMovieFile(&fileSpec, movieRefNum, fsRdPerm);
	}

	if (!err)
	{
		err = NewMovieFromFile(theMovie, *movieRefNum, movieResId, NULL, newMovieActive, NULL);
	}

	if (err)
	{
		if (*theMovie)
		{
			DisposeMovie(*theMovie);
		}
	}

	return err;
}

/* ---------------------------------------------------------------- */
/*                                                                  */
/*    QTCode_DoGetMovieNextInterestingTime()                        */
/*                                                                  */
/*                                                                  */
/*    Gets the next time of interest in a given movie               */
/*                                                                  */
/*                                                                  */
/*                                                                  */
/* ---------------------------------------------------------------- */

TimeValue QTCode_DoGetMovieNextInterestingTime(TimeValue	currentTime,
												Movie		theMovie,
												OSType		mediaType)
{
	TimeValue nextTime = 0;

	GetMovieNextInterestingTime(theMovie, nextTimeMediaSample, 1, &mediaType, currentTime, 0, &nextTime, NULL);
		/* have we reached the end of the movie? */
	if(nextTime == -1)
	{
			/* end of movie, so go back to beginning */
		GetMovieNextInterestingTime(theMovie, nextTimeMediaSample, 1, &mediaType, 1, 0, &nextTime, NULL);
	}

	return nextTime;
}

/* ---------------------------------------------------------------- */
/*                                                                  */
/*    QTCode_DoCreatePortAssociation()                              */
/*                                                                  */
/*                                                                  */
/*    Create a mac graphics port and associate it with a given      */
/*    window so QTML can draw into it                               */
/*                                                                  */
/*                                                                  */
/* ---------------------------------------------------------------- */

GrafPtr QTCode_DoCreatePortAssociation(void *	theWnd,
										Ptr 	storage,
										long 	flags)
{
		/* Associate a GrafPort with this window */
	return(CreatePortAssociation(theWnd, storage, flags));
}

/* ---------------------------------------------------------------- */
/*                                                                  */
/*    QTCode_DoDestroyPortAssociation()                             */
/*                                                                  */
/*                                                                  */
/*    De-register a mac graphics port and its associated            */
/*    window                                                        */
/*                                                                  */
/*                                                                  */
/* ---------------------------------------------------------------- */

void QTCode_DoDestroyPortAssociation(HWND hwnd)
{
		/* Destroy the port association */
	DestroyPortAssociation((CGrafPtr)GetHWNDPort(hwnd));
}

/* ---------------------------------------------------------------- */
/*                                                                  */
/*    QTCode_DoDestroyOffscreen()                                   */
/*                                                                  */
/*                                                                  */
/*    Dispose the data structures associated with a mac offscreen   */
/*    created with NewGWorlDFromHBITMAP                             */
/*                                                                  */
/*                                                                  */
/* ---------------------------------------------------------------- */

void QTCode_DoDestroyOffscreen(GWorldPtr offscreen, HBITMAP	hBitmap)
{
	BOOL success;

		/* Destroy the offscreen GWorld and related objects */
	DisposeGWorld(offscreen);
	success = DeleteObject(hBitmap);
}

/* ---------------------------------------------------------------- */
/*                                                                  */
/*    QTCode_DisposeMovieAndController()                            */
/*                                                                  */
/*                                                                  */
/*    Disposes a movie and its associated controller                */
/*                                                                  */
/*                                                                  */
/*                                                                  */
/* ---------------------------------------------------------------- */

void QTCode_DisposeMovieAndController(Movie	theMovie,
									short	movRefNum)
{
		// Dispose movie and controller
	if (movRefNum)
	{
		CloseMovieFile(movRefNum);
	}

	if (theMovie)
	{
		DisposeMovie(theMovie);
	}
}

/* ---------------------------------------------------------------- */
/*                                                                  */
/*    QTCode_DoQTInit()                                             */
/*                                                                  */
/*                                                                  */
/*    Initialize Quicktime                                          */
/*                                                                  */
/*                                                                  */
/*                                                                  */
/* ---------------------------------------------------------------- */

BOOL QTCode_DoQTInit()
{
		/* Initialize QuickTime Media Layer */
	if (InitializeQTML(0L) != 0)
	{
		return (FALSE);
	}

		/* Initialize QuickTime */
	if (EnterMovies() != 0)
	{
		return (FALSE);
	}

	return (TRUE);
}

/* ---------------------------------------------------------------- */
/*                                                                  */
/*    QTCode_QTCleanUp()                                            */
/*                                                                  */
/*                                                                  */
/*    De-initialize Quicktime                                       */
/*                                                                  */
/*                                                                  */
/*                                                                  */
/* ---------------------------------------------------------------- */

void QTCode_QTCleanUp()
{
		/* Clean up */
	ExitMovies();
	TerminateQTML();
}

/* ---------------------------------------------------------------- */
/*                                                                  */
/*    QTCode_PositionMovieRectInClientWindow()                      */
/*                                                                  */
/*                                                                  */
/*    Adjusts the movie bounds of a given movie so the movie is     */
/*    centered in the window                                        */
/*                                                                  */
/*                                                                  */
/* ---------------------------------------------------------------- */

void QTCode_PositionMovieRectInClientWindow(Movie	theMovie,
											HWND	hwnd)
{
	Rect movieBounds;

	GetMovieBox(theMovie, &movieBounds);

	CenterMovieRectInWindow(hwnd,					/* window where we are placing the image */
							RECT_WIDTH(movieBounds), RECT_HEIGHT(movieBounds),	/* width, height, of the image */
							&movieBounds			/* on output, a Mac Rect centered in the window */
							);

	SetMovieBox(theMovie, &movieBounds);
}
