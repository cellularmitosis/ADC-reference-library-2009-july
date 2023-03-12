//////////
//
//	File:		WinFramework.h
//
//	Contains:	Basic functions for windows, menus, and similar things.
//
//	Written by:	Tim Monroe
//
//	Copyright:	© 1999 by Apple Computer, Inc., all rights reserved.
//
//	Change History (most recent first):
//	   
//	   <1>	 	11/05/99	rtm		first file
//	   
//////////

#pragma once


//////////
//	   
// header files
//	   
//////////

#ifndef __Prefix_File__
#include "WinPrefix.h"
#endif

#ifndef _WINDOWS_
#include <windows.h>
#endif

#ifndef __FIXMATH__
#include <FixMath.h>
#endif

#ifndef __QTML__
#include <QTML.h>
#endif

#ifndef __SCRAP__
#include <Scrap.h>
#endif

#ifndef _STDLIB_H
#include <stdlib.h>
#endif

#ifndef __malloc_h__ 
#include <malloc.h>
#endif

#include "ComFramework.h"
#include "ComResource.h"


//////////
//
// constants
//
//////////

#define	WM_PUMPMOVIE				(WM_USER+0)
#define	WM_OPENDROPPEDFILES			(WM_USER+1)
#define USEEXPLORERSTYLE			(LOBYTE(LOWORD(GetVersion()))>=4)
#define kOpenDialogCustomData		11						// an arbitrary value that allows our dialog proc to detect the Open File dialog box
#define kAlertMessageMaxLength		256						// maximum length of a message in the QTFrame_ShowCautionAlert message box

#define kWinFilePathSeparator		(char)'\\'				// on Windows, the file path separator is '\\'


//////////
//
// function prototypes
//	   
//////////

LRESULT CALLBACK 			QTFrame_FrameWndProc (HWND theWnd, UINT theMessage, UINT wParam, LONG lParam);
LRESULT CALLBACK 			QTFrame_MovieWndProc (HWND theWnd, UINT theMessage, UINT wParam, LONG lParam);
void						QTFrame_OpenCommandLineMovies (LPSTR theCmdLine);
int							QTFrame_ShowCautionAlert (HWND theWnd, UINT theID, UINT theIconStyle, UINT theButtonStyle, LPSTR theTitle, LPSTR theArgument);
static UINT APIENTRY		QTFrame_DialogProcedure (HWND theDialog, UINT theMessage, WPARAM wParam, LPARAM lParam);
static void					QTFrame_CalcWindowMinMaxInfo (HWND theWnd, LPMINMAXINFO lpMinMax);
