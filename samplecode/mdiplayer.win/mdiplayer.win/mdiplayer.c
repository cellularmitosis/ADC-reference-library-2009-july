//---------------------------------------------------------------------------------------------------
//	MDIPlayer.c
//
//	A rudimentary MDI movie player application
//  Created Aug 5, 1996 by Brian S. Friedkin
//	Copyright 1996 Apple Computer Inc.
//---------------------------------------------------------------------------------------------------
//
//	Change History (most recent first):
//
//	   <2>	 	03/19/98	rtm		added !IsIconic(hwnd) before call to MCIsPlayerEvent in MovieWndProc
//									to fix drawing problems when arranging icons
//	   <1>	 	03/18/98	rtm		added !IsIconic(hwnd) to test in WM_SIZE message processing in MovieWndProc
//									to fix sizing problems when minimized; added WM_WINDOWPOSCHANGED processing to
//									MovieWndProc to stop movies from playing in minimized windows


// Macintosh headers
#include "QTML.h"
#include "Movies.h"
#include "Scrap.h"

// Windows headers
#include <windows.h>
#include "resource.h"

#define kMovieControllerHeight	16
#define	RECT_WIDTH(r)	(r.right-r.left)
#define	RECT_HEIGHT(r)	(r.bottom-r.top)
#define	WM_PUMPMOVIE	(WM_USER+0)

long FAR PASCAL FrameWndProc  (HWND, UINT, UINT, LONG) ;
long FAR PASCAL MovieWndProc  (HWND, UINT, UINT, LONG) ;
BOOL			DoOpenMovie();
int				DoIdleMenus(HWND hWnd, HMENU hMenu);
Boolean			MyPlayerFilter(MovieController theController, short action, void *params, long refCon);
void			SizeWindowToMovie(HWND hWnd);
static long		GetWindowBorderWidth(HWND hWnd);
static long		GetWindowMenuHeight(HWND hWnd);
static void		DoCut(HWND);
static void		DoCopy(HWND);
static void		DoPaste(HWND);
static void		DoClear(HWND);
static void		DoUndo(HWND);
static void		DoAbout(void);
static void		CalcWindowMinMaxInfo(HWND theWnd, LPMINMAXINFO lpMinMax);

// Data held in each child movie window
typedef struct {
	Movie			m;
	MovieController	mc;
	HWND			hWnd;
	short			resID;
	short			refNum;
} ChildWindowRecord, **ChildWindowHand;

HANDLE	ghInst;
HWND	ghWnd;
HWND	ghWndMDIClient;
char	szAppName[20];
char	szChildName[] = "MdiChild";
Rect	gMCResizeBounds;
int		cOpen = 0;
BOOL	gWeAreSizingWindow = 0;
BOOL	gWeAreCreatingWindow = 0;
BOOL	gShuttingDown = FALSE;

/* ------------------------------------------------------------- */

int PASCAL WinMain (HANDLE hInstance, HANDLE hPrevInstance,
                    LPSTR lpszCmdLine, int nCmdShow)
{
	HANDLE		hAccel ;
	HWND		hwndFrame;
	MSG			msg ;
    WNDCLASSEX	wc;

	ghInst = hInstance ;

	if (!hPrevInstance) 
	{
		LoadString(hInstance, IDS_APPNAME, szAppName, sizeof(szAppName));
		
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
		wc.lpszMenuName  = szAppName;
		wc.lpszClassName = szAppName;
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

		// Register the Movie child window class
		wc.cbSize        = sizeof(WNDCLASSEX);
		wc.style         = 0;
		wc.lpfnWndProc   = (WNDPROC)MovieWndProc;
		wc.cbClsExtra    = 0;
		wc.cbWndExtra    = 0;
		wc.hInstance     = hInstance;
		wc.hIcon         = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_CHILDICON));
		wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
		wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
		wc.lpszMenuName  = NULL;
		wc.lpszClassName = szChildName;
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
	hAccel = LoadAccelerators (hInstance, szAppName);

	// Initialize QuickTime Media Layer
	InitializeQTML(0);

	// Initialize QuickTime
	EnterMovies();

	// Create the main frame window
	ghWnd = hwndFrame = CreateWindow (szAppName, szAppName,
                               WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN,
                               CW_USEDEFAULT, CW_USEDEFAULT,
                               CW_USEDEFAULT, CW_USEDEFAULT,
                               NULL, NULL, hInstance, NULL) ;

	// Show the window
	ShowWindow(hwndFrame, nCmdShow);
	UpdateWindow(hwndFrame);

	// Process messages
    while (GetMessage(&msg, NULL, 0, 0))
    {
        if (!TranslateMDISysAccel(ghWndMDIClient, &msg))
            if (!TranslateAccelerator(hwndFrame, hAccel, &msg))
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
    }

	// Clean up
	ExitMovies();
	TerminateQTML();

	return msg.wParam;
}

/* ------------------------------------------------------------- */

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
						SendMessage (ghWndMDIClient, WM_MDIDESTROY, (WPARAM)hwndChild, 0L) ;
					return 0 ;

				case IDM_EXIT:
					gShuttingDown = TRUE;
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

/* ------------------------------------------------------------- */

long FAR PASCAL MovieWndProc (HWND hwnd, UINT message, UINT wParam, LONG lParam)
{
	WPARAM			nWidth, nHeight;
	MovieController	mc = 0;
	Movie			m = 0;
	ChildWindowHand	hStorage;
	MSG				msg = {0};

	if (hStorage = (ChildWindowHand)GetWindowLong(hwnd, GWL_USERDATA))
	{
		mc = (**hStorage).mc;
		m = (**hStorage).m;
	}

	// Give the movie controller this message first
	if (!gShuttingDown && mc)
	{
		EventRecord	macEvent;

		msg.hwnd = hwnd;
		msg.message = message;
		msg.wParam = wParam;
		msg.lParam = lParam;
		msg.time = 0;
		msg.pt.x = 0;
		msg.pt.y = 0;

		// tranlate a windows event to a mac event
		WinEventToMacEvent(&msg, &macEvent);

		// pass in a mac event
		if (!IsIconic(hwnd))
			MCIsPlayerEvent(mc, (EventRecord*)&macEvent);
	}

	switch (message)
	{
		case WM_CREATE:
			// Tuck away some private storage
			hStorage = (ChildWindowHand)NewHandleClear(sizeof(ChildWindowRecord));
			(**hStorage).hWnd = hwnd;
			SetWindowLong(hwnd, GWL_USERDATA, (LPARAM)hStorage);

			// Associate a GrafPort with this window
			CreatePortAssociation(hwnd, NULL, 0);
			break;

		case WM_WINDOWPOSCHANGING:
			// Don't show the window until we have created a movie and therefore
			// can properly size the window to contain the movie.
			if (gWeAreCreatingWindow)
			{
				WINDOWPOS	*lpWindowPos = (WINDOWPOS*)lParam;
				lpWindowPos->flags &= ~SWP_SHOWWINDOW;
				return 0;
			}
			break;

		case WM_WINDOWPOSCHANGED:
			// If a movie window has become minimized, stop the movie.
			if (IsIconic(hwnd))
				StopMovie(m);
			break;

		case WM_SIZE:
			// Resize the movie and controller to fit the window
			nWidth = LOWORD(lParam);
			nHeight = HIWORD(lParam);
			if (!gWeAreSizingWindow && mc && !IsIconic(hwnd))
			{
				Rect	r = {0};
				GrafPtr	port = (GrafPtr)MCGetControllerPort(mc);
				r.right = nWidth;
				r.bottom = nHeight;
				MCSetControllerBoundsRect(mc, &r);
			}
			break;

		case WM_PUMPMOVIE:					// We receive this message only to idle the movie
			break;

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

		case WM_GETMINMAXINFO:
			CalcWindowMinMaxInfo(hwnd, (LPMINMAXINFO)lParam);
			return 0;

		case WM_DESTROY:
			// One less movie open
			--cOpen;

			// Dispose movie and controller
			hStorage = (ChildWindowHand)GetWindowLong(hwnd, GWL_USERDATA);
			if ((**hStorage).refNum) CloseMovieFile((**hStorage).refNum);

			// Set a player filter proc to nil before closing it because
			// in some instances it gets called after we shut down.
			if (mc) {
				MCSetActionFilterWithRefCon(mc, MyPlayerFilter, 0);
				DisposeMovieController(mc);
			}
			if (m) DisposeMovie(m);
			DisposeHandle((Handle)GetWindowLong(hwnd, GWL_USERDATA));
			SetWindowLong(hwnd, GWL_USERDATA, 0);

			// Destroy the port association
			DestroyPortAssociation((CGrafPtr)GetHWNDPort(hwnd));
			return 0;
	}

	return DefMDIChildProc (hwnd, message, wParam, lParam);
}

/* ------------------------------------------------------------- */

BOOL DoOpenMovie()
{
    DWORD			dwVersion;
	FSSpec			fileSpec;
	OSErr			err;
	long			mcFlags;
	short			len, movieRefNum, movieResId;
    char            szPathName[256], szFileName[256];
	Rect			movieBounds;
	ChildWindowHand	hStorage;
	RECT			rcWindow;
	HWND			hWndDesktop;
    HWND			hwndChild = 0;
	Movie			theMovie=0;
	MovieController	theMC=0;
    OPENFILENAME    ofn = {0};

	// Present the dialog...
	*szPathName = 0;
	ofn.lStructSize = sizeof(OPENFILENAME);
    ofn.hwndOwner = ghWnd;
    ofn.lpstrFile = (LPSTR)szPathName;
    ofn.nMaxFile  = sizeof(szPathName);
	ofn.lpstrFilter  = "QuickTime Movies (*.mov) \0 *.mov\0All Files (*.*) \0 *.*\0";
    ofn.nFilterIndex = 1;
    ofn.lpstrInitialDir = NULL;
    ofn.Flags        = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
    if (!GetOpenFileName(&ofn)) goto bail;

	// Create the child movie window
	dwVersion = GetVersion();
	gWeAreCreatingWindow = TRUE;
	if ((dwVersion < 0x80000000) || (LOBYTE(LOWORD(dwVersion)) < 4))
	{
		// This is Windows NT or Win32s, so use the WM_MDICREATE message
		MDICREATESTRUCT mcs;

		mcs.szClass = szChildName;
		mcs.szTitle = szFileName; 
		mcs.hOwner  = ghInst;
		mcs.x       = CW_USEDEFAULT;
		mcs.y       = CW_USEDEFAULT;
		mcs.cx      = CW_USEDEFAULT;
		mcs.cy      = CW_USEDEFAULT;
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
								   szChildName,
								   szFileName,
								   0,
								   CW_USEDEFAULT,
								   CW_USEDEFAULT,
								   CW_USEDEFAULT,
								   CW_USEDEFAULT,
								   ghWndMDIClient, 
								   NULL,
								   ghInst,
								   0);
	}
	gWeAreCreatingWindow = FALSE;
	if (!hwndChild) goto bail;

	MacSetPort(GetHWNDPort(hwndChild));
	hStorage = (ChildWindowHand)GetWindowLong(hwndChild, GWL_USERDATA);

	// Set the window title to the movie filename
	len = strlen(szPathName);
	while (len--)
	{
		char	c = szPathName[len];
		if (c == 0x5c || c == '/')
		{
			strcpy(szFileName, &szPathName[len+1]);
			break;
		}
	}
	SetWindowText(hwndChild, szFileName);

	// Open up the movie file...

	// Fill in a FSSpec.  Currently the vRefNum and parID are not used under Windows.
	// Note the name must be a full pathname P-string!
	fileSpec.vRefNum = 0;
	fileSpec.parID = 0;
	strcpy((char*)&fileSpec.name[1], szPathName);
	fileSpec.name[0] = strlen(szPathName);
	err = OpenMovieFile(&fileSpec, &movieRefNum, fsRdWrPerm);
	if (err)
		err = OpenMovieFile(&fileSpec, &movieRefNum, fsRdPerm);
	if (!err)
	{
		movieResId = 0;
		err = NewMovieFromFile(&theMovie, movieRefNum, &movieResId, NULL,newMovieActive,NULL);
	}
	if (err) goto bail;

	// Offset the movie box to (0,0)
	GetMovieBox(theMovie, &movieBounds);
	movieBounds.right = RECT_WIDTH(movieBounds);
	movieBounds.bottom = RECT_HEIGHT(movieBounds);
	movieBounds.left = movieBounds.top = 0;
	
	// Audio only movies need some width for the controller!
	if (!movieBounds.right) movieBounds.right = 320;
	SetMovieBox(theMovie, &movieBounds);

	// Create and initialize the movie controller
	movieBounds.bottom += kMovieControllerHeight;
	theMC = NewMovieController(theMovie, &movieBounds, mcTopLeftMovie);
	if (!theMC) goto bail;

	MCDoAction(theMC, mcActionGetFlags, &mcFlags);
	mcFlags |= (mcFlagSuppressMovieFrame | mcFlagsUseWindowPalette);
	MCDoAction(theMC, mcActionSetFlags, (void*)mcFlags);
	MCEnableEditing(theMC, true);

	// Make a growbox
	// Set the mc resize bounds
	hWndDesktop = GetDesktopWindow();
	GetWindowRect(hWndDesktop, &rcWindow);
	gMCResizeBounds.right = (short)rcWindow.right;
	gMCResizeBounds.bottom = (short)rcWindow.bottom;
	gMCResizeBounds.top = 48+16;			// Min height + mc
	gMCResizeBounds.left = (16+16+16);	// Speaker+mc+grow icon widths
	if (gMCResizeBounds.left < GetSystemMetrics(SM_CXMINTRACK))
		gMCResizeBounds.left = GetSystemMetrics(SM_CXMINTRACK);
	if (gMCResizeBounds.top < GetSystemMetrics(SM_CYMINTRACK))
		gMCResizeBounds.top = GetSystemMetrics(SM_CYMINTRACK);
	MCDoAction(theMC, mcActionSetGrowBoxBounds, &gMCResizeBounds);

	// Allow the controller to accept keyboard events
	MCDoAction(theMC, mcActionSetKeysEnabled, (void*)1);

	// Store movie info in private window record
	(**hStorage).m = theMovie;
	(**hStorage).mc = theMC;
	(**hStorage).resID = movieResId;
	(**hStorage).refNum = movieRefNum;

	// Set a player filter proc to trap certain events
	MCSetActionFilterWithRefCon(theMC, MyPlayerFilter, (long)hStorage);

	// Size the window to fit the movie and controller
	SizeWindowToMovie(hwndChild);

	// Show the window
	ShowWindow(hwndChild, SW_SHOW);
	UpdateWindow(hwndChild);
	SetFocus(hwndChild);


	// One more window has been opened
	cOpen += 1;

	return TRUE;

bail:
	if (hwndChild) SendMessage(ghWndMDIClient, WM_MDIDESTROY, (WPARAM)hwndChild, 0L);
	if (theMovie) DisposeMovie(theMovie);
	if (theMC) DisposeMovieController(theMC);
	return FALSE;
}

/* ------------------------------------------------------------- */

static void DoCut(HWND hwnd)
{
	MovieController	mc = 0;
	ChildWindowHand	hStorage = (ChildWindowHand)GetWindowLong(hwnd, GWL_USERDATA);

	if (hStorage)
	{
		if (mc = (**hStorage).mc)
		{
			// Cut the segment
			Movie	scrapMovie = MCCut(mc);

			// Place the segment into the scrap
			if (scrapMovie)
			{
				PutMovieOnScrap(scrapMovie, 0L);
				DisposeMovie(scrapMovie);
			}
		}
	}
}

/* ------------------------------------------------------------- */

static void DoCopy(HWND hwnd)
{
	MovieController	mc = 0;
	ChildWindowHand	hStorage = (ChildWindowHand)GetWindowLong(hwnd, GWL_USERDATA);

	if (hStorage)
	{
		if (mc = (**hStorage).mc)
		{
			// Copy the segment
			Movie	scrapMovie = MCCopy(mc);

			// Place the segment into the scrap
			if (scrapMovie)
			{
				PutMovieOnScrap(scrapMovie, 0L);
				DisposeMovie(scrapMovie);
			}
		}
	}
}

/* ------------------------------------------------------------- */

static void DoPaste(HWND hwnd)
{
	MovieController	mc = 0;
	ChildWindowHand	hStorage = (ChildWindowHand)GetWindowLong(hwnd, GWL_USERDATA);

	if (hStorage)
	{
		if (mc = (**hStorage).mc)
			MCPaste(mc, nil);
	}
}

/* ------------------------------------------------------------- */

static void DoClear(HWND hwnd)
{
	MovieController	mc = 0;
	ChildWindowHand	hStorage = (ChildWindowHand)GetWindowLong(hwnd, GWL_USERDATA);

	if (hStorage)
	{
		if (mc = (**hStorage).mc)
			MCClear(mc);
	}
}

/* ------------------------------------------------------------- */

static void DoUndo(HWND hwnd)
{
	MovieController	mc = 0;
	ChildWindowHand	hStorage = (ChildWindowHand)GetWindowLong(hwnd, GWL_USERDATA);

	if (hStorage)
	{
		if (mc = (**hStorage).mc)
			MCUndo(mc);
	}
}

/* ------------------------------------------------------------- */

// Size the HWND to exactly fit the movie and controller
void SizeWindowToMovie(HWND hWnd)
{
	ChildWindowHand	hStorage = (ChildWindowHand)GetWindowLong(hWnd, GWL_USERDATA);
	Movie		theMovie = (**hStorage).m;
	MovieController	mc = (**hStorage).mc;
	CGrafPtr	wPort = (CGrafPtr)GetHWNDPort(hWnd);
	Rect		movieBounds;

	gWeAreSizingWindow = TRUE;

	MCGetControllerBoundsRect(mc, &movieBounds);
	if (!RECT_WIDTH(movieBounds))
	{
		movieBounds.left = 0;
		movieBounds.right = 320;
	}

	// Size our window
	SizeWindow((WindowPtr)wPort, movieBounds.right, movieBounds.bottom, FALSE);

	gWeAreSizingWindow = FALSE;

	MacSetPort((GrafPtr)wPort);
}

/* ------------------------------------------------------------- */

Boolean MyPlayerFilter(MovieController theController, short action, void *params, long refCon)
{
#define mcActionControllerSizeChanged 26
	ChildWindowHand	hStorage;

	switch (action) {
	case  mcActionControllerSizeChanged:
		if ( (hStorage = (ChildWindowHand)refCon) ) {
			SizeWindowToMovie((**hStorage).hWnd);
		}
		break;
	}
	return 0;
}

/* ------------------------------------------------------------- */

int DoIdleMenus(HWND hWnd, HMENU hMenu)
{
	ChildWindowHand	hStorage = (ChildWindowHand)GetWindowLong(hWnd, GWL_USERDATA);
	MovieController	mc = 0;

	if (hStorage) mc = (**hStorage).mc;

	// Enable the close item if there are any movie windows opened
	EnableMenuItem(hMenu, IDM_FILECLOSE, (cOpen) ? MF_ENABLED : MF_GRAYED);

	// Idle the edit menu
	if (mc)
	{
		long	controllerFlags;

		MCGetControllerInfo(mc,&controllerFlags);

		EnableMenuItem(hMenu, IDM_EDITUNDO, controllerFlags & mcInfoUndoAvailable ? MF_ENABLED : MF_GRAYED);
		EnableMenuItem(hMenu, IDM_EDITCUT, controllerFlags & mcInfoCutAvailable ? MF_ENABLED : MF_GRAYED);
		EnableMenuItem(hMenu, IDM_EDITCOPY, controllerFlags & mcInfoCopyAvailable ? MF_ENABLED : MF_GRAYED);
		EnableMenuItem(hMenu, IDM_EDITPASTE, controllerFlags & mcInfoPasteAvailable ? MF_ENABLED : MF_GRAYED);
		EnableMenuItem(hMenu, IDM_EDITCLEAR, controllerFlags & mcInfoClearAvailable ? MF_ENABLED : MF_GRAYED);
	}

	return 0;
}

/* ------------------------------------------------------------- */

static LRESULT CALLBACK DialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message) {
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

static void DoAbout()
{
	DialogBox(ghInst, MAKEINTRESOURCE(IDD_ABOUT), ghWnd, (DLGPROC)DialogProc);
}

/* ------------------------------------------------------------- */

// Get width of the window (vertical) borders
static long GetWindowBorderWidth(HWND hWnd)
{
	RECT	rcWindow, rcClient;
	
	GetWindowRect(hWnd, &rcWindow);
	GetClientRect(hWnd, &rcClient);

	return RECT_WIDTH(rcWindow) - RECT_WIDTH(rcClient);
}

/* ------------------------------------------------------------- */

// Get height of the menubar contained by this window
static long GetWindowMenuHeight(HWND hWnd)
{
	RECT	rcWindow, rcClient;
	
	GetWindowRect(hWnd, &rcWindow);
	GetClientRect(hWnd, &rcClient);

	// Our MDI child windows have no menu
	return (RECT_HEIGHT(rcWindow) - RECT_HEIGHT(rcClient)) - GetSystemMetrics(SM_CYCAPTION);
}

/* ------------------------------------------------------------- */

static void CalcWindowMinMaxInfo(HWND theWnd, LPMINMAXINFO lpMinMax)
{
	ChildWindowHand	hStorage = (ChildWindowHand)GetWindowLong(theWnd, GWL_USERDATA);
	MovieController	mc = 0;
	Movie	m = 0;

	if (hStorage)
	{
		mc = (**hStorage).mc;
		m = (**hStorage).m;
	}

	if (mc && m)
	{
		Rect	movieBox;

		lpMinMax->ptMinTrackSize.x = gMCResizeBounds.left + (2 * GetSystemMetrics(SM_CXFRAME));
		GetMovieBox(m, &movieBox);
		if (RECT_HEIGHT(movieBox))
			lpMinMax->ptMinTrackSize.y = gMCResizeBounds.top +	// growbounds height +
				(2 * GetSystemMetrics(SM_CXFRAME)) +			// frame wthickness +
				GetSystemMetrics(SM_CYCAPTION) +				// caption height +
				-1 +											// fudge factor
				kMovieControllerHeight;							// movie controller height
		else
			lpMinMax->ptMaxSize.y =
			lpMinMax->ptMaxTrackSize.y =
			lpMinMax->ptMinTrackSize.y = 0 +					// height of audio only movie +
				(2 * GetSystemMetrics(SM_CXFRAME)) +			// frame wthickness +
				GetSystemMetrics(SM_CYCAPTION) +				// caption height +
				-1 +											// fudge factor
				kMovieControllerHeight;							// movie controller height
	}
}
