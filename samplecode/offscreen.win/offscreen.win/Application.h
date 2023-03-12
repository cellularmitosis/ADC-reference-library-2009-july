/*
	File:		Application.h

	Contains:	QuickTime 3.0 Offscreen sample application.

	Written by:	Scott Kuechle
				based on MDIPlayer code by Brian S. Friedkin

	Copyright:	© 1998 by Apple Computer, Inc., all rights reserved.

	Change History (most recent first):

	   <1>	 	4/24/98		srk		first file
	    
 
 NOTES:

 
 TO DO:

*/

#pragma once

// Macintosh headers
#include "MacTypes.h"
#include "QTML.h"
#include "Movies.h"
#include "Scrap.h"
#include "FixMath.h"
#include "NumberFormatting.h"
#include "TextUtils.h"
#include "Resources.h"
#include "quickdraw.h"
#include "files.h"
#include "PictUtils.h"

#include <string.h>

// Windows headers
#define	STRICT
#include <windows.h>
#include "resource.h"

// Program headers
#include "ApplicationPrivate.h"


#define	RECT_WIDTH(r)	(r.right-r.left)
#define	RECT_HEIGHT(r)	(r.bottom-r.top)

#define	kWinSpacer	200

void		DrawBackgroundBitmap(HDC hDC, HWND hwnd, HANDLE hBitmap, RECT *updateRect);
void		DrawHelpMessage(HDC	hDC, HWND hwnd);
WORD		GetDCBitDepth(HDC hDC);
HPALETTE	UseCustomPalette(HDC hDC, HPALETTE	newPalette);
void		DoDrawFrameInfo(HDC	hMemDC, HWND hwnd, Rect *movieBounds, TimeValue theTime);
HBITMAP		DoCreateDIBSection(HDC dc, LPRGBQUAD srcRgbQuadArray,WORD depth,LONG width,LONG	height);
HDC			DoCreateMemoryDC(HWND	hwnd);
void		CenterMovieRectInWindow(HWND hwnd, int movWidth, int movHeight, Rect *centeredRect);

