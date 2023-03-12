/*
	File:		Offscreen.c

	Contains:	QuickTime 3.0 Offscreen sample application.

	Written by:	Scott Kuechle
				based on MDIPlayer code by Brian S. Friedkin

	Copyright:	© 1998 by Apple Computer, Inc., all rights reserved.

	Change History (most recent first):

	   <2>	 	5/18/98		rtm		minor tweaks to GetBitDepthOfBitmap and DrawBackgroundBitmap to get this code
	   								working with the Metrowerks compiler
	   <1>	 	4/24/98		srk		first file; revised to personal coding style
	    
 
 NOTES:

 
 TO DO:

*/


#include "Application.h"
#include "QTCode.h"


long FAR PASCAL FrameWndProc  (HWND, UINT, UINT, LONG) ;
long FAR PASCAL MovieWndProc  (HWND, UINT, UINT, LONG) ;
BOOL			DoOpenMovie();
int				DoIdleMenus(HWND hWnd, HMENU hMenu);
static void		DoCut(HWND);
static void		DoCopy(HWND);
static void		DoPaste(HWND);
static void		DoClear(HWND);
static void		DoUndo(HWND);
static void		DoAbout(void);
HPALETTE		DoCreatePaletteFromBitMap(HMODULE appInstance, WORD bitMapID);
void			SetMovieFrameTimeValue(Movie theMovie, TimeValue thisPoint);
WORD			GetBitDepthOfBitmap(HMODULE appInstance, WORD bitMapID);
void			GetRBGColorsFromBitmap(HDC	hDC, HANDLE	hInst, WORD	bitMapID, LPRGBQUAD	*srcRgbQuadArray, HPALETTE	*hPalette);
void			DoEraseRect(HDC	hDC, RECT *theRect);
int				GetLineHeight(HDC hDC);
void			DrawMovieAndBackground(HGLOBAL hWinStorage, HDC hDC, HWND hwnd);


	/* globals for this module */
HANDLE		ghInst					= NULL;
HWND		ghWndFrame				= NULL;
HWND		ghWndMDIClient			= NULL;
BOOL		gWeAreSizingWindow		= 0;
BOOL		gWeAreCreatingWindow	= 0;
int			gOpenMovieCount			= 0;
char		gAppName[20];
char		gChildName[20];
LPRGBQUAD	gSrcRgbQuadArray		= NULL;		/* rbg color values for our window background bitmap */
HPALETTE	gHBackgroundPalette		= NULL;		/* color palette for our window background */
HANDLE		ghBkgrndBitMap			= NULL;		/* bitmap we paint on the background of our client windows */

/* ---------------------------------------------------------------- */
/*                                                                  */
/*    WinMain()                                                     */
/*                                                                  */
/*                                                                  */
/*    The main function for this application                        */
/*                                                                  */
/*                                                                  */
/*                                                                  */
/* ---------------------------------------------------------------- */

int WINAPI WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance,
					LPSTR lpszCmdLine, int nCmdShow)
    {
	HANDLE		hAccel ;
	HWND		hwndFrame;
	MSG			msg ;
    WNDCLASSEX	wc;
	BOOL		success;

		ghInst = hInstance ;

		if (!hPrevInstance) 
		{
			LoadString(hInstance, IDS_APPNAME, gAppName, sizeof(gAppName));
			
			// Register the frame window class
			wc.cbSize        = sizeof(WNDCLASSEX);
			wc.style         = CS_HREDRAW | CS_VREDRAW;
			wc.lpfnWndProc   = (WNDPROC)FrameWndProc;
			wc.cbClsExtra    = 0;
			wc.cbWndExtra    = 0;
			wc.hInstance     = hInstance;
			wc.hIcon         = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_APPICON));
			wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
			wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
			wc.lpszMenuName  = gAppName;
			wc.lpszClassName = gAppName;
			wc.hIconSm       = LoadImage(hInstance,
										 MAKEINTRESOURCE(IDI_APPICON),
										 IMAGE_ICON,
										 16, 16,
										 0);
			if (!RegisterClassEx(&wc))
			{
				if (!RegisterClass((LPWNDCLASS)&wc.style))
        			return FALSE;
			}

			LoadString(hInstance, IDS_MDICHILD, gChildName, sizeof(gChildName));

			// Register the Movie child window class
			wc.cbSize        = sizeof(WNDCLASSEX);
			wc.style         = CS_HREDRAW | CS_VREDRAW;
			wc.lpfnWndProc   = (WNDPROC)MovieWndProc;
			wc.cbClsExtra    = 0;
			wc.cbWndExtra    = 0;
			wc.hInstance     = hInstance;
			wc.hIcon         = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_CHILDICON));
			wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
			wc.hbrBackground = NULL	/* we want to handle erasing of the background ourselves */;
			wc.lpszMenuName  = NULL;
			wc.lpszClassName = gChildName;
			wc.hIconSm       = LoadImage(hInstance,
										 MAKEINTRESOURCE(IDI_CHILDICON),
										 IMAGE_ICON,
										 16, 16,
										 0);
			if (!RegisterClassEx(&wc))
			{
				if (!RegisterClass((LPWNDCLASS)&wc.style))
        			return FALSE;
			}
		}

		// Load accelerators
		hAccel = LoadAccelerators (hInstance, gAppName);

		// Create the main frame window
		ghWndFrame = hwndFrame = CreateWindow (gAppName, gAppName,
											   WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN,
											   CW_USEDEFAULT, CW_USEDEFAULT,
											   CW_USEDEFAULT, CW_USEDEFAULT,
											   NULL, NULL, hInstance, NULL) ;

			/* get background bitmap for use with our client windows */
		ghBkgrndBitMap = LoadResource(ghInst, FindResource(ghInst, MAKEINTRESOURCE(IDB_BITMAP2), RT_BITMAP) );
 
			/* Show the window */
		ShowWindow(hwndFrame, nCmdShow);
		UpdateWindow(hwndFrame);

			/* Initialize QuickTime */
		success = QTCode_DoQTInit();
		if (success)
		{
				/* Process messages */
			while (GetMessage(&msg, NULL, 0, 0))
			{
				if (!TranslateMDISysAccel(ghWndMDIClient, &msg))
					if (!TranslateAccelerator(hwndFrame, hAccel, &msg))
					{
						TranslateMessage(&msg);
						DispatchMessage(&msg);
					}
			}
		}
		else
		{
			MessageBox(hwndFrame, "Quicktime 3.0 not available", "", MB_OK);
		}

			/* QuickTime clean up */
		QTCode_QTCleanUp();

		return msg.wParam;
}

/* ---------------------------------------------------------------- */
/*                                                                  */
/*    FrameWndProc()                                                */
/*                                                                  */
/*                                                                  */
/*    The window procedure for the MDI frame window                 */
/*                                                                  */
/*                                                                  */
/*                                                                  */
/* ---------------------------------------------------------------- */

long FAR PASCAL FrameWndProc (HWND hwnd, UINT message, UINT wParam, LONG lParam)
{
	HWND               hwndChild ;

	switch (message)
	{
		case WM_CREATE:          // Create the client window
		{
			CLIENTCREATESTRUCT ccs = {0};

			ccs.hWindowMenu  = GetSubMenu(GetMenu(hwnd), WINDOWMENU);
			ccs.idFirstChild = IDM_WINDOWCHILD;

			// Create the MDI client filling the client area
			ghWndMDIClient = CreateWindow("mdiclient",
										 NULL,
										 WS_CHILD | WS_CLIPCHILDREN | WS_VSCROLL |
										 WS_HSCROLL,
										 0, 0, 0, 0,
										 hwnd,
										 (HMENU)0xCAC,
										 ghInst,
										 (LPVOID)&ccs);

			ShowWindow(ghWndMDIClient, SW_SHOW);
		}
		return 0 ;

		case WM_COMMAND:
			switch (LOWORD(wParam))
			{
				case IDM_FILEOPEN:
					DoOpenMovie();
					return 0 ;

				case IDM_FILECLOSE:
					hwndChild = (HWND)SendMessage(ghWndMDIClient, WM_MDIGETACTIVE, 0, 0L) ;
					if (SendMessage (hwndChild, WM_QUERYENDSESSION, 0, 0L))
					{
						SendMessage (ghWndMDIClient, WM_MDIDESTROY, (WPARAM)hwndChild, 0L) ;
					}
					return 0 ;

				case IDM_EXIT:
					SendMessage (hwnd, WM_CLOSE, 0, 0L) ;
					return 0 ;

				case IDM_WINDOWTILE:
					SendMessage (ghWndMDIClient, WM_MDITILE, 0, 0L) ;
					return 0 ;

				case IDM_WINDOWCASCADE:
					SendMessage (ghWndMDIClient, WM_MDICASCADE, 0, 0L) ;
					return 0 ;

				case IDM_WINDOWICONS:
					SendMessage (ghWndMDIClient, WM_MDIICONARRANGE, 0, 0L) ;
					return 0 ;

				case IDM_WINDOWCLOSEALL:
					{
						HWND	hwndT;
			
						while (hwndT = GetWindow(ghWndMDIClient, GW_CHILD))
						{
							// Skip the icon and title windows
							while (hwndT && GetWindow(hwndT, GW_OWNER))
								hwndT = GetWindow(hwndT, GW_HWNDNEXT);

							if (!hwndT) break;

							SendMessage(ghWndMDIClient, WM_MDIDESTROY, (WPARAM)hwndT, 0L);
						}
					}
					return 0;

				case IDM_ABOUT:
					DoAbout();
					return 0;

				default:            // Pass to active child
					hwndChild = (HWND)SendMessage (ghWndMDIClient, WM_MDIGETACTIVE, 0, 0L) ;
					if (IsWindow (ghWndMDIClient))
						SendMessage (hwndChild, WM_COMMAND, wParam, lParam) ;

					break ;        // and then to DefFrameProc
			}
			break ;

		case WM_INITMENU:
			if (GetMenu(hwnd) == (HMENU)wParam)
				return DoIdleMenus((HWND)SendMessage(ghWndMDIClient, WM_MDIGETACTIVE, 0, 0), (HMENU)wParam);
			return 1;

		case WM_DESTROY :
			PostQuitMessage (0) ;
			return 0 ;
	}

	return DefFrameProc (hwnd, ghWndMDIClient, message, wParam, lParam) ;
}

/* ---------------------------------------------------------------- */
/*                                                                  */
/*    MovieWndProc()                                                */
/*                                                                  */
/*                                                                  */
/*    The window procedure for our movie window                     */
/*                                                                  */
/*                                                                  */
/*                                                                  */
/* ---------------------------------------------------------------- */

long FAR PASCAL MovieWndProc (HWND hwnd, UINT message, UINT wParam, LONG lParam)
{
	HGLOBAL			hWinStorage = NULL;
	ChildWindowPtr	childWinPtr = NULL;
	MSG				msg = {0};


	hWinStorage = (HGLOBAL)GetWindowLong(hwnd, GWL_USERDATA);

	switch (message)
	{
		case WM_CREATE:
			{
				GrafPtr gp;

						/* Tuck away some private storage */
					hWinStorage = GlobalAlloc(GMEM_MOVEABLE + GMEM_ZEROINIT, sizeof(ChildWindowRecord));
					SetWindowLong(hwnd, GWL_USERDATA, (LPARAM)hWinStorage);

						/* Associate a GrafPort with this window */
					gp = QTCode_DoCreatePortAssociation(hwnd, NULL, 0);

			}
			return 0;

		case WM_GETMINMAXINFO:
			{				
				if (hWinStorage != NULL)
				{
					childWinPtr = (ChildWindowPtr)GlobalLock(hWinStorage);
					if (childWinPtr != NULL)
					{
						LPMINMAXINFO lpmmi;

							lpmmi = (LPMINMAXINFO) lParam;
								/* we restrict the grow size of our window */
							lpmmi->ptMaxSize.x = childWinPtr->maxWindowWidth;
							lpmmi->ptMaxSize.y = childWinPtr->maxWindowHeight;
							lpmmi->ptMaxTrackSize.x = childWinPtr->maxWindowWidth;
							lpmmi->ptMaxTrackSize.y = childWinPtr->maxWindowHeight;

							GlobalUnlock(hWinStorage);
					}
				}
			}
			return 0;

		case WM_PAINT:
			{
				PAINTSTRUCT	lPaint;
				HDC			hDC;

					hDC = BeginPaint(hwnd, &lPaint);
					if (hDC != NULL)
					{
						DrawMovieAndBackground(hWinStorage, hDC, hwnd);
					}
					EndPaint(hwnd, &lPaint);
			}
			return 0;

		case WM_WINDOWPOSCHANGING:
			// Don't show the window until we have created a movie and therefore
			// can properly size the window to contain the movie.
			if (gWeAreCreatingWindow)
			{
				WINDOWPOS	*lpWindowPos = (WINDOWPOS*)lParam;
				lpWindowPos->flags &= ~SWP_SHOWWINDOW;
				return 0;
			}


			return 0;
	
		case WM_KEYDOWN:
			if (wParam == VK_RETURN)	/* return key was pressed */
			{
				HDC		hDC			= NULL;
				OSType	mediaType	= VideoMediaType;

					if (hWinStorage != NULL)
					{
						childWinPtr = (ChildWindowPtr)GlobalLock(hWinStorage);
						if (childWinPtr != NULL)
						{
								/* move to next interesting movie time to display */
							childWinPtr->currentTime = 
								QTCode_DoGetMovieNextInterestingTime(childWinPtr->currentTime, childWinPtr->movie,
																		VideoMediaType);
							GlobalUnlock(hWinStorage);

							hDC = GetDC(hwnd);
							if (hDC != NULL)
							{
								DrawMovieAndBackground(hWinStorage, hDC, hwnd);
								ReleaseDC(hwnd, hDC);
							}
						}
					}
			}

			return 0;

		case WM_SIZE:
			{				
				if (hWinStorage != NULL)
				{
					childWinPtr = (ChildWindowPtr)GlobalLock(hWinStorage);
					if (childWinPtr != NULL)
					{
						if (childWinPtr->movie)
						{
							QTCode_PositionMovieRectInClientWindow(childWinPtr->movie, hwnd);
						}
						GlobalUnlock(hWinStorage);
					}
				}
			}
			return 0;

		case WM_ERASEBKGND:
			{
				/* we don't want the background erased when the window is
					re-sized, so we'll trap calls here for the WM_ERASEBKGND
					event */
			}
			return (1);	/* tell GDI we've handled erasing ourselves */

		case WM_COMMAND:
			{
				switch(LOWORD(wParam))		// Undo, Cut, Copy, Paste and Clear
				{
					case IDM_EDITUNDO:
						DoUndo(hwnd);
						break;

					case IDM_EDITCUT:
						DoCut(hwnd);
						break;

					case IDM_EDITCOPY:
						DoCopy(hwnd);
						break;

					case IDM_EDITPASTE:
						DoPaste(hwnd);
						break;

					case IDM_EDITCLEAR:
						DoClear(hwnd);
						break;
				}
			}
			break;

		case WM_DESTROY:
			{		
				if (hWinStorage != NULL)
				{
					childWinPtr = (ChildWindowPtr)GlobalLock(hWinStorage);
					if (childWinPtr != NULL)
					{
							/* One less movie open */
						--gOpenMovieCount;

						QTCode_DisposeMovieAndController(childWinPtr->movie, childWinPtr->refNum);
							/* Destroy the port association */
						QTCode_DoDestroyPortAssociation(hwnd);
							/* clean up offscreen GWorld */
						QTCode_DoDestroyOffscreen(childWinPtr->offscreenGWorld, childWinPtr->hBitmap);

						SetWindowLong(hwnd, GWL_USERDATA, 0);
						GlobalUnlock(hWinStorage);
						GlobalFree((HGLOBAL)GetWindowLong(hwnd, GWL_USERDATA));
					}
				}
			}
			return 0;
	}

	return DefMDIChildProc (hwnd, message, wParam, lParam);
}

/* ---------------------------------------------------------------- */
/*                                                                  */
/*    DoOpenMovie()                                                 */
/*                                                                  */
/*                                                                  */
/*    Code to open a given movie and prepare it for display in a    */
/*    window                                                        */
/*                                                                  */
/*                                                                  */
/* ---------------------------------------------------------------- */

BOOL DoOpenMovie()
{
    DWORD			dwVersion;
	short			len;
    char            szPathName[256], szFileName[256];
    HWND			hwndChild = 0;
    OPENFILENAME    ofn = {0};
	HDC				hDC;
	HGLOBAL			hWinStorage = NULL;
	OSErr			err;
	Movie			movie = NULL;
	short			movieRefNum;
	short			movieResId;
	int				movWidth, movHeight;

		// Present the dialog...
		*szPathName = 0;
		ofn.lStructSize = sizeof(OPENFILENAME);
		ofn.hwndOwner = ghWndFrame;
		ofn.lpstrFile = (LPSTR)szPathName;
		ofn.nMaxFile  = sizeof(szPathName);
		ofn.lpstrFilter  = "QuickTime Movies (*.mov) \0 *.mov\0All Files (*.*) \0 *.*\0";
		ofn.nFilterIndex = 1;
		ofn.lpstrInitialDir = NULL;
		ofn.Flags        = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
		if (!GetOpenFileName(&ofn)) goto bail;

		err = QTCode_NewMovieFromWinPathname(szPathName,	/* c-string path to movie */
											&movie,			/* Movie returned here */
											&movieRefNum,	/* Movie RefNum returned here */
											&movieResId);	/* Movie Resource ID returned here */
		if (err == noErr)
		{
			Rect	movieBounds;

				GetMovieBox(movie, &movieBounds);
				movWidth = RECT_WIDTH(movieBounds);
				movHeight = RECT_HEIGHT(movieBounds);
		}
		// Create the child movie window
		dwVersion = GetVersion();
		gWeAreCreatingWindow = TRUE;
		if ((dwVersion < 0x80000000) || (LOBYTE(LOWORD(dwVersion)) < 4))
		{
			// This is Windows NT or Win32s, so use the WM_MDICREATE message
			MDICREATESTRUCT mcs;

			mcs.szClass = gChildName;
			mcs.szTitle = szFileName; 
			mcs.hOwner  = ghInst;
			mcs.x       = CW_USEDEFAULT;
			mcs.y       = CW_USEDEFAULT;
			mcs.cx      = movWidth + kWinSpacer;	/* set an appropriate width for our movie window */
			mcs.cy      = movHeight + kWinSpacer;	/* set an appropriate height for our movie window */
			mcs.style   = 0;
			mcs.lParam  = 0;

			hwndChild = (HWND) SendMessage(ghWndMDIClient,
										   WM_MDICREATE,
										   0,
										   (LPARAM)(LPMDICREATESTRUCT) &mcs);
		}
		else
		{
			// This method will only work with Windows 95, not Windows NT or Win32s
			hwndChild = CreateWindowEx(WS_EX_MDICHILD,
									   gChildName,
									   szFileName,
									   0,
									   CW_USEDEFAULT,
									   CW_USEDEFAULT,
									   movWidth + kWinSpacer,	/* set an appropriate width for our movie window */
									   movHeight + kWinSpacer,	/* set an appropriate height for our movie window */
									   ghWndMDIClient, 
									   NULL,
									   ghInst,
									   0);
		}
		gWeAreCreatingWindow = FALSE;
		if (!hwndChild) goto bail;

		// Set the window title to the movie filename
		len = strlen(szPathName);
		while (len--)
		{
			char c = szPathName[len];
			if (c == 0x5c || c == '/')
			{
				strcpy(szFileName, &szPathName[len+1]);
				break;
			}
		}
		SetWindowText(hwndChild, szFileName);

			// Store movie info in private window record
		hWinStorage = (HGLOBAL)GetWindowLong(hwndChild, GWL_USERDATA);
		if (hWinStorage != NULL)
		{
			ChildWindowPtr	childWinPtr = NULL;

			childWinPtr = (ChildWindowPtr)GlobalLock(hWinStorage);
			if (childWinPtr != NULL)
			{

				childWinPtr->ghInst	= ghInst;

				if (movie != NULL)
				{
					childWinPtr->movie = movie;
					childWinPtr->refNum = movieRefNum;
					childWinPtr->hBkgrndBitmap = ghBkgrndBitMap;
					childWinPtr->maxWindowWidth = movWidth + kWinSpacer;
					childWinPtr->maxWindowHeight = movHeight + kWinSpacer;


					QTCode_PositionMovieRectInClientWindow(childWinPtr->movie, hwndChild);
					err = QTCode_GetStartPointOfFirstVideoSample(childWinPtr->movie, &childWinPtr->currentTime);
				}


				hDC = GetDC(hwndChild);
				if (hDC != NULL)
				{
					WORD bitDepthDC;
					RECT windowRect;
					QDErr err;

						GetRBGColorsFromBitmap(	hDC,
												ghInst,
												IDB_BITMAP2,		/* ID of background bitmap we want to use */
												&childWinPtr->srcRgbQuadArray,	/* on return, a pointer to a rgbquad
																				structure for this bitmap */
												&childWinPtr->hBackgroundPalette);	/* if bitmap bit depth <= 8, a custom palette */
						bitDepthDC = GetDCBitDepth(hDC);
						GetWindowRect(hwndChild, &windowRect);
						childWinPtr->hBitmap = DoCreateDIBSection(hDC,				/* the device context for this HWND */
																	childWinPtr->srcRgbQuadArray,		/* pointer to RGBQUAD array to use when creating
																											DIB - NULL if bit depth > 8 */
																	bitDepthDC,				/* screen depth setting */
																	RECT_WIDTH(windowRect),	/* window width */
																	RECT_HEIGHT(windowRect)	/* window height */
																	);
						childWinPtr->hMemDC = DoCreateMemoryDC(hwndChild);
						SelectObject(childWinPtr->hMemDC, childWinPtr->hBitmap);
						err = QTCode_CreateOffscreen(&childWinPtr->offscreenGWorld,
													NULL,
													NULL,
													0,
													childWinPtr->hBitmap,/* a pointer to a valid bitmap or NULL */
													childWinPtr->hMemDC);	/* a pointer to a valid device context or NULL */
						QTCode_SetMovieGWorld(childWinPtr->movie, childWinPtr->offscreenGWorld);

						ReleaseDC(hwndChild, hDC);
				}

				GlobalUnlock(hWinStorage);
			}
		}
		SetWindowLong(hwndChild, GWL_USERDATA, (LPARAM)hWinStorage);

		// Show the window
		ShowWindow(hwndChild, SW_SHOW);
		UpdateWindow(hwndChild);
		SetFocus(hwndChild);

		// One more window has been opened
		gOpenMovieCount += 1;

		return TRUE;

		bail:
			if (hwndChild) SendMessage(ghWndMDIClient, WM_MDIDESTROY, (WPARAM)hwndChild, 0L);

		return FALSE;
}


/* ---------------------------------------------------------------- */
/*                                                                  */
/*    DrawMovieAndBackground()                                      */
/*                                                                  */
/*                                                                  */
/*    Code to display a movie and background image in a given       */
/*    window                                                        */
/*                                                                  */
/*                                                                  */
/* ---------------------------------------------------------------- */

void DrawMovieAndBackground(HGLOBAL hWinStorage, HDC hDC, HWND hwnd)
{
	HPALETTE		previousPalette	= NULL;
	ChildWindowPtr	childWinPtr;

		if ((childWinPtr = (ChildWindowPtr)GlobalLock(hWinStorage)) != NULL)
			{
				RECT	clientRect, windowRect;
				Rect	movieBounds;
				
					previousPalette = UseCustomPalette(hDC, childWinPtr->hBackgroundPalette);

					GetClientRect(hwnd, &clientRect);
					GetWindowRect(hwnd, &windowRect);

					DrawBackgroundBitmap(childWinPtr->hMemDC, hwnd, childWinPtr->hBkgrndBitmap, &clientRect);
					DrawHelpMessage(childWinPtr->hMemDC, hwnd);

					GetMovieBox(childWinPtr->movie, &movieBounds);
					DoDrawFrameInfo(childWinPtr->hMemDC, hwnd, &movieBounds, childWinPtr->currentTime);

					SetMovieTimeValue(childWinPtr->movie, childWinPtr->currentTime);
					QTCode_ForceMovieRedraw(childWinPtr->movie);

						/* copy our memory DC image to the display DC */
					BitBlt(hDC, 0, 0, RECT_WIDTH(windowRect), RECT_HEIGHT(windowRect), childWinPtr->hMemDC,
							0, 0, SRCCOPY);

					GlobalUnlock(hWinStorage);
			}

}

/* ---------------------------------------------------------------- */
/*                                                                  */
/*    DoCut()                                                       */
/*                                                                  */
/*                                                                  */
/*    Handles clipboard "cut" operations                            */
/*                                                                  */
/*                                                                  */
/*                                                                  */
/* ---------------------------------------------------------------- */

static void DoCut(HWND hwnd)
{
}

/* ---------------------------------------------------------------- */
/*                                                                  */
/*    DoCopy()                                                      */
/*                                                                  */
/*                                                                  */
/*    Handles clipboard "copy" operations                           */
/*                                                                  */
/*                                                                  */
/*                                                                  */
/* ---------------------------------------------------------------- */

static void DoCopy(HWND hwnd)
{
}

/* ---------------------------------------------------------------- */
/*                                                                  */
/*    DoPaste()                                                     */
/*                                                                  */
/*                                                                  */
/*    Handles clipboard "paste" operations                          */
/*                                                                  */
/*                                                                  */
/*                                                                  */
/* ---------------------------------------------------------------- */

static void DoPaste(HWND hwnd)
{
}

/* ---------------------------------------------------------------- */
/*                                                                  */
/*    DoClear()                                                     */
/*                                                                  */
/*                                                                  */
/*    Handles clipboard "clear" operations                          */
/*                                                                  */
/*                                                                  */
/*                                                                  */
/* ---------------------------------------------------------------- */

static void DoClear(HWND hwnd)
{
}

/* ---------------------------------------------------------------- */
/*                                                                  */
/*    DoUndo()                                                      */
/*                                                                  */
/*                                                                  */
/*    Handles clipboard "undo" operations                           */
/*                                                                  */
/*                                                                  */
/*                                                                  */
/* ---------------------------------------------------------------- */

static void DoUndo(HWND hwnd)
{
}


/* ---------------------------------------------------------------- */
/*                                                                  */
/*    DoIdleMenus()                                                 */
/*                                                                  */
/*                                                                  */
/*    Enable the close item if there are any movie windows opened   */
/*                                                                  */
/*                                                                  */
/*                                                                  */
/* ---------------------------------------------------------------- */

int DoIdleMenus(HWND hWnd, HMENU hMenu)
{
		/* Enable the close item if there are any movie windows opened */
	EnableMenuItem(hMenu, IDM_FILECLOSE, (gOpenMovieCount) ? MF_ENABLED : MF_GRAYED);

	return 0;
}

/* ---------------------------------------------------------------- */
/*                                                                  */
/*    DialogProc()                                                  */
/*                                                                  */
/*                                                                  */
/*    Callback for our dialog box                                   */
/*                                                                  */
/*                                                                  */
/*                                                                  */
/* ---------------------------------------------------------------- */

static LRESULT CALLBACK DialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
		case WM_COMMAND:
			switch (LOWORD(wParam))
			{
				case IDOK:
					EndDialog(hDlg, IDOK);
					break;
			}
			break;
	}

    return 0;
}

/* ---------------------------------------------------------------- */
/*                                                                  */
/*    DoAbout()                                                     */
/*                                                                  */
/*                                                                  */
/*    Displays a dialog box with information about our program      */
/*                                                                  */
/*                                                                  */
/*                                                                  */
/* ---------------------------------------------------------------- */

static void DoAbout()
{
	DialogBox(ghInst, MAKEINTRESOURCE(IDD_ABOUT), ghWndFrame, (DLGPROC)DialogProc);
}

/* ---------------------------------------------------------------- */
/*                                                                  */
/*    DrawBackgroundBitmap()                                        */
/*                                                                  */
/*                                                                  */
/*    Sets the pixels in the target device context to those from    */
/*    a given DIB                                                   */
/*                                                                  */
/*                                                                  */
/* ---------------------------------------------------------------- */

void DrawBackgroundBitmap(	HDC		hDC,
							HWND	hwnd,
							HANDLE	hBitMap,
							RECT	*updateRect)
{
	if ((hBitMap != NULL) && (hDC != NULL) && (hwnd != NULL))
	{
		LPBITMAPINFO	lpBi;
		LPTSTR			lpBits;
		int 			x, y, width, height;
		int				success = FALSE;
		RECT			clientRect;
		int				childHeight, childWidth;

			GetClientRect(hwnd, &clientRect);
			childWidth = RECT_WIDTH(clientRect);
			childHeight = RECT_HEIGHT(clientRect);

			lpBi = (LPBITMAPINFO)LockResource(hBitMap);

			lpBits = (LPSTR)lpBi;
			lpBits += lpBi->bmiHeader.biSize + ((1 << lpBi->bmiHeader.biBitCount) * sizeof(RGBQUAD));
			width = lpBi->bmiHeader.biWidth;
			height = lpBi->bmiHeader.biHeight;

			for (y = 0; y <= childHeight; y += lpBi->bmiHeader.biHeight)
			{
				for (x = 0; x <= childWidth; x += lpBi->bmiHeader.biWidth)
				{
					BOOL	intersectRect = FALSE;
					RECT	bitMapRect;
					RECT	destRect;
					
					SetRect(&bitMapRect, x, y, x+lpBi->bmiHeader.biWidth, y+lpBi->bmiHeader.biHeight);
					
						intersectRect = IntersectRect( &destRect, // address of structure for intersection 
														updateRect, // address of structure with first rectangle 
														&bitMapRect // address of structure with second rectangle 
													); 
							/* does the current rectangle intersect with the update
								rectangle? If so, draw it, otherwise, don't */
						if (intersectRect)
						{
							success = SetDIBitsToDevice(hDC, x, y,
														lpBi->bmiHeader.biWidth,
														lpBi->bmiHeader.biHeight,
														0,0,
														0, lpBi->bmiHeader.biHeight,
														lpBits, lpBi,
														DIB_RGB_COLORS);
						}
				}
			}
	}

}

/* ---------------------------------------------------------------- */
/*                                                                  */
/*    DoCreatePaletteFromBitMap()                                   */
/*                                                                  */
/*                                                                  */
/*    Builds a palette from a given bitmap                          */
/*                                                                  */
/*                                                                  */
/*                                                                  */
/* ---------------------------------------------------------------- */

HPALETTE DoCreatePaletteFromBitMap(	HMODULE		appInstance,
									WORD		bitMapID)
{
	HANDLE hBitMap;
	HPALETTE hPalette = NULL;


		hBitMap = LoadResource(appInstance,
								FindResource(appInstance, MAKEINTRESOURCE(bitMapID), RT_BITMAP) );
		if (hBitMap != NULL)
		{
			LPBITMAPINFO	lpBi;
			HANDLE			hPal;
			LPLOGPALETTE	lpPal;
			int				i, nColorData;

				lpBi = (LPBITMAPINFO)LockResource(hBitMap);

				if (lpBi->bmiHeader.biClrUsed != 0)
				{
					nColorData = lpBi->bmiHeader.biClrUsed;
				}
				else
				{
					switch( lpBi->bmiHeader.biBitCount)
					{
						case 1 :
							nColorData = 2;
						break;
					
						case 4 :
							nColorData = 16;
						break;
					
						case 8 :
							nColorData = 256;
						break;
					
						case 24 :
						case 32 : 
							nColorData = 0;
						break;
					}
				}

				hPal = GlobalAlloc( GHND, sizeof (LOGPALETTE) + (nColorData * sizeof (PALETTEENTRY)) );
				lpPal = (LPLOGPALETTE)GlobalLock(hPal);

				lpPal->palVersion = 0x300;
				lpPal->palNumEntries = nColorData;

				for (i = 0; i < nColorData; i++)
				{
					lpPal->palPalEntry[i].peRed		= lpBi->bmiColors[i].rgbRed;
					lpPal->palPalEntry[i].peGreen	= lpBi->bmiColors[i].rgbGreen;
					lpPal->palPalEntry[i].peBlue	= lpBi->bmiColors[i].rgbBlue;

				}

				hPalette = CreatePalette( lpPal );

				GlobalUnlock(hPal);
				GlobalFree(hPal);

		}

		return hPalette;
}


/* ---------------------------------------------------------------- */
/*                                                                  */
/*    DoEraseRect()                                                 */
/*                                                                  */
/*                                                                  */
/*    Code to erase a given windows rectangle                       */
/*                                                                  */
/*                                                                  */
/*                                                                  */
/* ---------------------------------------------------------------- */

void DoEraseRect(HDC	hDC,
				RECT	*theRect)
{
	if (hDC != NULL)
	{
		HBRUSH		aBrush;
			
				/* erase rect. for string before redraw */
			aBrush = CreateSolidBrush(RGB(255,255,255));
			if (aBrush != NULL)
			{
				FillRect(hDC, theRect, aBrush);
				DeleteObject(aBrush);
			}

	}
}

/* ---------------------------------------------------------------- */
/*                                                                  */
/*    GetLineHeight()                                               */
/*                                                                  */
/*                                                                  */
/*    Returns the line height for a given device context            */
/*                                                                  */
/*                                                                  */
/*                                                                  */
/* ---------------------------------------------------------------- */

int GetLineHeight(HDC	hDC)
{
	TEXTMETRIC	tm;

		GetTextMetrics(hDC, &tm);
		return(tm.tmExternalLeading + tm.tmHeight);
}

/* ---------------------------------------------------------------- */
/*                                                                  */
/*    DrawHelpMessage()                                             */
/*                                                                  */
/*                                                                  */
/*    Draws our help text in a window                               */
/*                                                                  */
/*                                                                  */
/*                                                                  */
/* ---------------------------------------------------------------- */

void DrawHelpMessage(	HDC		hDC,
						HWND	hwnd)
{

	if (hDC != NULL)
	{
		RECT		clientRect, targetRect;
		char		helpText[100] = "Press <enter> to advance to next movie frame";
		TEXTMETRIC	tm;
		int			nLineHeight, strWidth;

			GetClientRect(hwnd, &clientRect);
			GetTextMetrics(hDC, &tm);
			nLineHeight = tm.tmExternalLeading + tm.tmHeight;

			strWidth =  strlen( (LPCTSTR)&helpText ) * tm.tmAveCharWidth;
			SetRect(&targetRect,	clientRect.left + ((RECT_WIDTH(clientRect) - strWidth)/2),
									clientRect.bottom - nLineHeight,
									clientRect.left + ((RECT_WIDTH(clientRect) - strWidth)/2) + strWidth,
									clientRect.bottom);
			DoEraseRect(hDC, &targetRect);

				/* draw help message */
			DrawText(hDC, (LPCTSTR)&helpText, -1, &targetRect, DT_CENTER);
	}
}


/* ---------------------------------------------------------------- */
/*                                                                  */
/*    DoCreateDIBSection()                                          */
/*                                                                  */
/*                                                                  */
/*    Creates a device independent bitmap using CreateDIBSection    */
/*                                                                  */
/*                                                                  */
/*                                                                  */
/* ---------------------------------------------------------------- */

HBITMAP DoCreateDIBSection(	HDC				dc,
							LPRGBQUAD		srcRgbQuadArray,	/* if (depth > 8), this param ignored */
							WORD			depth,
							LONG			width,
							LONG			height
							)
{
	UINT	usage;
	LPTSTR	bits = NULL;
   	
		// a struct large enough to hold the largest bitmapinfo ...
	struct BITMAPINFO_256
    {   
        BITMAPINFOHEADER bmiHeader;
        long bmiColors[256];
    };
	typedef struct BITMAPINFO_256 BITMAPINFO_256;

    BITMAPINFO_256 bmi;


		memset(&bmi, 0, sizeof(bmi));

		bmi.bmiHeader.biSize			= sizeof(BITMAPINFOHEADER);
		bmi.bmiHeader.biWidth			= width;
			/* important!!! - we specify a top-down DIB (by passing a negative height value)
				with origin in the upper left, otherwise our movie image will show up as inverted */
		bmi.bmiHeader.biHeight			= -1 * height;
		bmi.bmiHeader.biPlanes			= 1;
		bmi.bmiHeader.biBitCount		= depth;
		bmi.bmiHeader.biSizeImage		= 0;
		bmi.bmiHeader.biClrUsed			= 0;
		bmi.bmiHeader.biClrImportant	= 0;


		switch ( depth )
		{
			case 4:
			case 8:
				{
					long		num_colors = (1 << depth),
								i;
					LPRGBQUAD	lpQuadDest, lpQuadSource;


						bmi.bmiHeader.biCompression = BI_RGB;
						usage = DIB_RGB_COLORS;

						lpQuadSource = srcRgbQuadArray;
						
						for (i=0; i<num_colors; i++)
						{
							lpQuadDest = (LPRGBQUAD)&bmi.bmiColors[i];

							lpQuadDest->rgbBlue		= lpQuadSource->rgbBlue;
							lpQuadDest->rgbRed		= lpQuadSource->rgbRed;
							lpQuadDest->rgbGreen	= lpQuadSource->rgbGreen;
							lpQuadDest->rgbReserved	= lpQuadSource->rgbReserved;

							++lpQuadSource;
						}
				}
				break;

			case 16:
			case 24:
			case 32:
				{					
					bmi.bmiHeader.biCompression = BI_RGB;
					usage = DIB_RGB_COLORS;
        
					bmi.bmiColors[0] = 0;
					bmi.bmiColors[1] = 0;
					bmi.bmiColors[2] = 0;
				}
				break;
   
			default:
				break;
		}

		return CreateDIBSection(dc, (BITMAPINFO *) &bmi, usage, &bits, NULL, 0);
}


/* ---------------------------------------------------------------- */
/*                                                                  */
/*    GetDCBitDepth()                                               */
/*                                                                  */
/*                                                                  */
/*    Returns the bit depth of a device context                     */
/*                                                                  */
/*                                                                  */
/*                                                                  */
/* ---------------------------------------------------------------- */

WORD	GetDCBitDepth(HDC	hDC)
{
	return(GetDeviceCaps( hDC, BITSPIXEL)); 
}

/* ---------------------------------------------------------------- */
/*                                                                  */
/*    GetBitDepthOfBitmap()                                         */
/*                                                                  */
/*                                                                  */
/*    Returns the bit depth of a bitmap                             */
/*                                                                  */
/*                                                                  */
/*                                                                  */
/* ---------------------------------------------------------------- */

WORD	GetBitDepthOfBitmap(HMODULE appInstance,	/* application instance */
							WORD bitMapID			/* resource ID of bit map */
							)
{
	HBITMAP hBitMap;

		hBitMap = LoadResource(appInstance, FindResource(appInstance, MAKEINTRESOURCE(bitMapID), RT_BITMAP) );
		if (hBitMap != NULL)
		{
			LPBITMAPINFO	lpBi;

				lpBi = (LPBITMAPINFO)LockResource(hBitMap);

				return (lpBi->bmiHeader.biBitCount);
		}
		else
		{
			return 0;
		}

}


/* ---------------------------------------------------------------- */
/*                                                                  */
/*    GetRBGColorsFromBitmap()                                      */
/*                                                                  */
/*                                                                  */
/*    Returns the RGBQUAD array for a bitmap                        */
/*                                                                  */
/*                                                                  */
/*                                                                  */
/* ---------------------------------------------------------------- */

void GetRBGColorsFromBitmap(HDC			hDC,
							HANDLE		hInst,
							WORD		bitMapID,
							LPRGBQUAD	*srcRgbQuadArray,	/* on return, a pointer to a rgbquad
															structure for this bitmap */
							HPALETTE	*hPalette)	/* if bitmap bit depth <= 8, a custom palette
													for the bitmap will be returned here */
{
	WORD			bitDepthDC, bitDepthBITMAP;
	LPBITMAPINFO	srcBITMAPINFO;
	HANDLE			hBitMapTemp;

		bitDepthDC		=	GetDCBitDepth(hDC);
		bitDepthBITMAP	=	GetBitDepthOfBitmap(hInst,		/* application instance */
												bitMapID	/* resource ID of bit map */
												);
		srcBITMAPINFO = NULL;
		*srcRgbQuadArray = NULL;
		*hPalette = NULL;

			/* does the device context have a bit depth <= 8? */
		if (bitDepthDC <= 8)
		{
				/* does our bitmap have a depth <= 8? */
			if (bitDepthBITMAP <= 8)
			{
					/* create custom palette for use with this bitmap */
				*hPalette = DoCreatePaletteFromBitMap(hInst, bitMapID);

					/* get color data to use when calling CreateDIBSection */
				hBitMapTemp	= LoadResource(hInst,
											FindResource(hInst, MAKEINTRESOURCE(bitMapID),
											RT_BITMAP) );
				if (hBitMapTemp != NULL)
				{
						/* get bitmapinfo data */
					srcBITMAPINFO = (LPBITMAPINFO)LockResource(hBitMapTemp);
						/* get pointer to rgbquad array for this bitmap */
					*srcRgbQuadArray = &srcBITMAPINFO->bmiColors[0];
				}
			}
		}

}

/* ---------------------------------------------------------------- */
/*                                                                  */
/*    DoCreateMemoryDC()                                            */
/*                                                                  */
/*                                                                  */
/*    Creates a memory device context                               */
/*                                                                  */
/*                                                                  */
/*                                                                  */
/* ---------------------------------------------------------------- */

HDC	DoCreateMemoryDC(HWND	hwnd)
{
	HDC	hMemDC, hDC;

		hDC = GetDC(hwnd);
		if (hDC)
		{
			hMemDC = CreateCompatibleDC(hDC);
			SetMapMode(hMemDC, GetMapMode(hDC));
			ReleaseDC(hwnd, hDC);

			return hMemDC;
		}

		return NULL;
}



/* ---------------------------------------------------------------- */
/*                                                                  */
/*    UseCustomPalette()                                            */
/*                                                                  */
/*                                                                  */
/*    Selects/Realizes a logical palette                            */
/*                                                                  */
/*                                                                  */
/*                                                                  */
/* ---------------------------------------------------------------- */

HPALETTE UseCustomPalette(HDC	hDC, HPALETTE	newPalette)
{
	HPALETTE	previousPalette;
	UINT		nEntries;

			/* use a custom palette if need be */
		if (newPalette)
		{
			previousPalette = SelectPalette(hDC, newPalette, FALSE);
			nEntries = RealizePalette(hDC);
		}

		return previousPalette;
}

/* ---------------------------------------------------------------- */
/*                                                                  */
/*    DoDrawFrameInfo()                                             */
/*                                                                  */
/*                                                                  */
/*    Draws our help text messages into a window                    */
/*                                                                  */
/*                                                                  */
/*                                                                  */
/* ---------------------------------------------------------------- */

void DoDrawFrameInfo(HDC		hMemDC,
					 HWND		hwnd,
					 Rect		*movieBounds,
					 TimeValue	theTime)
{
	RECT		destRect;
	int			nLineHeight;
	Str255 		theString;
	char		frameText[25] = "Frame ";
	Rect		centeredRect;


		nLineHeight = GetLineHeight(hMemDC);

		CenterMovieRectInWindow(hwnd,					/* window where we are placing the image */
							  (movieBounds->right - movieBounds->left),
							  (movieBounds->bottom - movieBounds->top),	/* width, height, of the movie */
							  &centeredRect			/* on output, a Mac Rect centered in the window */
							  );
		SetRect(&destRect,	(int)centeredRect.left - nLineHeight,
							(int)centeredRect.top - nLineHeight /*- menuSize*/,
							(int)centeredRect.right + nLineHeight,
							(int)centeredRect.bottom + nLineHeight /*- menuSize*/);
		DoEraseRect(hMemDC, &destRect);

		SetRect(&destRect,	(int)centeredRect.left,
							(int)centeredRect.bottom /*- menuSize*/,
							(int)centeredRect.right,
							(int)centeredRect.bottom + nLineHeight /*- menuSize*/);
		DoEraseRect(hMemDC, &destRect);

			/* draw new frame # */
		NumToString((long)theTime, theString);
		p2cstr(theString);
		lstrcat((LPSTR)frameText, (LPCSTR)&theString);
		DrawText(hMemDC, (LPCTSTR)&frameText, -1, &destRect, DT_CENTER);

}


/* ---------------------------------------------------------------- */
/*                                                                  */
/*    CenterMovieRectInWindow()                                     */
/*                                                                  */
/*                                                                  */
/*    Given height/width values for a movie, returns a Mac rect     */
/*    for the movie centered in a window                            */
/*                                                                  */
/*                                                                  */
/* ---------------------------------------------------------------- */

void CenterMovieRectInWindow(HWND	hwnd,					/* window where we are placing the image */
							  int	movWidth, int movHeight,	/* width, height, of the movie */
							  Rect	*centeredRect			/* on output, a Mac Rect centered in the window */
							  )
{
	RECT	clientRect;
	int		availSpace;
	BOOL	success;

		success = GetClientRect(hwnd, // handle of window 
								&clientRect// address of structure for client coordinates 
								);	

		availSpace = RECT_WIDTH(clientRect) - movWidth;
		if (availSpace < 0)
		{
			/* movie is bigger than client space? */
		}
		centeredRect->left = clientRect.left + availSpace/2;

		availSpace = RECT_HEIGHT(clientRect) - movHeight;
		if (availSpace < 0)
		{
			/* movie is bigger than client space? */
		}
		centeredRect->top = clientRect.top + availSpace/2;
		centeredRect->right = centeredRect->left + movWidth;
		centeredRect->bottom = centeredRect->top + movHeight;

}
