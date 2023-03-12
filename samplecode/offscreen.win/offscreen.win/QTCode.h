/*
	File:		QTCode.h

	Contains:	QuickTime 3.0 Offscreen sample application.

	Written by:	Scott Kuechle

	Copyright:	© 1998 by Apple Computer, Inc., all rights reserved.

	Change History (most recent first):

	   <1>	 	4/24/98		srk		first file
	    
 
 NOTES:

 
 TO DO:

*/


#include "Application.h"


void		QTCode_PositionMovieRectInClientWindow(Movie theMovie, HWND	hwnd);
TimeValue	QTCode_DoGetMovieNextInterestingTime(TimeValue	currentTime, Movie	theMovie, OSType	mediaType);
GrafPtr		QTCode_DoCreatePortAssociation(void *theWnd, Ptr storage, long 	flags);
void		QTCode_DisposeMovieAndController(Movie theMovie, short movRefNum);
void		QTCode_DoDestroyPortAssociation(HWND hwnd);
BOOL		QTCode_DoQTInit();
void		QTCode_QTCleanUp();
OSErr		QTCode_NewMovieFromWinPathname(char	*pathName, Movie *theMovie, short *movieRefNum, short *movieResId);
OSErr		QTCode_GetStartPointOfFirstVideoSample(Movie theMovie, TimeValue *startPoint);
QDErr		QTCode_CreateOffscreen(GWorldPtr *offscreenGWorld, CTabHandle cTable, GDHandle aGDevice, GWorldFlags flags, HBITMAP	newHBITMAP, HDC newHDC);
void		QTCode_DoDestroyOffscreen(GWorldPtr offscreen, HBITMAP	hBitmap);
void		QTCode_SetMovieGWorld(Movie theMovie, GWorldPtr offscreen);
void		QTCode_ForceMovieRedraw(Movie theMovie);
