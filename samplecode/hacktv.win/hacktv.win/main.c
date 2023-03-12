/*
	File:		Main.c

	Contains:	HackTV Windows application

	Written by: Brian Friedkin
			
	Copyright:	© 1992-1998 by Apple Computer, Inc.
*/

// Macintosh headers
#include "QTML.h"
#include "Movies.h"
#include "Scrap.h"
#include "Resources.h"
#include "Common.h"
#include "Globals.h"

// Windows headers
#include <windows.h>
#include "resource.h"

long FAR PASCAL FrameWndProc(HWND, UINT, UINT, LONG);
long FAR PASCAL MonitorWndProc(HWND, UINT, UINT, LONG);
int				DoIdleMenus(HWND hWnd, HMENU hMenu);

HANDLE	ghInst;
HWND	ghWnd;
HWND	ghWndMDIClient;
char	szAppName[20];
char	szChildName[] = "MdiChild";
BOOL	gShuttingDown = FALSE;

/* ------------------------------------------------------------- */

int PASCAL WinMain (HANDLE hInstance, HANDLE hPrevInstance,
                    LPSTR lpszCmdLine, int nCmdShow)
{
	HANDLE		hAccel ;
	HWND		hwndFrame;
	MSG			msg ;
	char		szAppPathName[_MAX_PATH];
	FSSpec		fsp;
	short		appResID;
    WNDCLASS	wc={0};

	ghInst = hInstance ;

	if (!hPrevInstance) 
	{
		LoadString(hInstance, IDS_APPNAME, szAppName, sizeof(szAppName));
		
		// Register the frame window class
		wc.style         = CS_HREDRAW | CS_VREDRAW;
		wc.lpfnWndProc   = (WNDPROC)FrameWndProc;
		wc.cbClsExtra    = 0;
		wc.cbWndExtra    = 0;
		wc.hInstance     = hInstance;
		wc.hIcon         = LoadIcon(0, IDI_APPLICATION);
		wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
		wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
		wc.lpszMenuName  = szAppName;
		wc.lpszClassName = szAppName;
		if (!RegisterClass(&wc))
        	return FALSE;

		// Register the Monitor child window class
		wc.style         = 0;
		wc.lpfnWndProc   = (WNDPROC)MonitorWndProc;
		wc.cbClsExtra    = 0;
		wc.cbWndExtra    = 0;
		wc.hInstance     = hInstance;
		wc.hIcon         = LoadIcon(0, IDI_APPLICATION);
		wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
		wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
		wc.lpszMenuName  = NULL;
		wc.lpszClassName = szChildName;
		if (!RegisterClass(&wc))
        	return FALSE;
	}

	// Load accelerators
	hAccel = LoadAccelerators(hInstance, szAppName);

	// Initialize QuickTime Media Layer
	InitializeQTML(0);

	// Initialize QuickTime
	EnterMovies();

	// Open up our resource file
	GetModuleFileName(hInstance, szAppPathName, 256);
	NativePathNameToFSSpec(szAppPathName, &fsp, 0);
	appResID = FSpOpenResFile(&fsp, fsRdPerm);
	UseResFile(appResID);

	// Create the main frame window
	ghWnd = hwndFrame = CreateWindow (szAppName, szAppName,
                               WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN,
                               CW_USEDEFAULT, CW_USEDEFAULT,
                               CW_USEDEFAULT, CW_USEDEFAULT,
                               NULL, NULL, hInstance, NULL) ;

	// Show the window
	ShowWindow(hwndFrame, nCmdShow);
	UpdateWindow(hwndFrame);

	// Fire up the sequence grabber
	InitializeSequenceGrabber();
	SendMessage(ghWndMDIClient, WM_MDIACTIVATE, (WPARAM)GetPortNativeWindow(gMonitor), 0);  

	// Process messages
	while (!gShuttingDown)
	{
		if (PeekMessage(&msg, 0, 0, 0, PM_NOREMOVE))
		{
			GetMessage(&msg, NULL, 0, 0);
			{
				if (!TranslateMDISysAccel(ghWndMDIClient, &msg))
					if (!TranslateAccelerator(hwndFrame, hAccel, &msg))
					{
						TranslateMessage(&msg);
						DispatchMessage(&msg);
					}
			}
		}
		else if (gSeqGrabber)
			SGIdle(gSeqGrabber);
	}

	// Clean up
	if (gSeqGrabber != 0L)
	{
		CloseComponent(gSeqGrabber);
		gSeqGrabber = 0L;
	}	
	
	if (gMonitor != nil)
		DestroyWindow(GetPortNativeWindow(gMonitor));
	
	// Set quit flag
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

			// Create the MDI client filling the client area
			ghWndMDIClient = CreateWindow("mdiclient",
										 NULL,
										 WS_CHILD | WS_CLIPCHILDREN | WS_VSCROLL |
										 WS_HSCROLL | MDIS_ALLCHILDSTYLES,
										 0, 0, 0, 0,
										 hwnd,
										 (HMENU)0xCAC,
										 ghInst,
										 (LPVOID)&ccs);

			ShowWindow(ghWndMDIClient, SW_SHOW);
		}
		return 0;

		case WM_COMMAND:
			switch (LOWORD(wParam))
			{
				case IDM_EXIT:
					gShuttingDown = TRUE;
					SendMessage (hwnd, WM_CLOSE, 0, 0L) ;
					return 0 ;

				case IDM_ABOUT:
					DoAboutDialog();
					return 0;

				case IDM_PAGE_SETUP:
					DoPageSetup();
					break;

				case IDM_PRINT:
					DoPrint();
					break;

				case IDM_CUT:
				case IDM_COPY:
					DoCopyToClipboard();
					break;

				case IDM_VIDEO_SETTINGS:
					DoVideoSettings();
					break;

				case IDM_SOUND_SETTINGS:
					DoSoundSettings();
					break;

				case IDM_RECORD_VIDEO:
					gRecordVideo = !gRecordVideo;
					break;

				case IDM_RECORD_SOUND:
					gRecordSound = !gRecordSound;
					break;

				case IDM_SPLIT_TRACKS:
					gSplitTracks = !gSplitTracks;
					break;

				case IDM_QUARTER_SIZE:
					DoResize(4);
					break;

				case IDM_HALF_SIZE:
					DoResize(2);
					break;

				case IDM_FULL_SIZE:
					DoResize(1);
					break;

				case IDM_RECORD:
					DoRecord();
					break;

				default:            // Pass to active child
					hwndChild = (HWND)SendMessage (ghWndMDIClient, WM_MDIGETACTIVE, 0, 0L) ;
					if (IsWindow (ghWndMDIClient))
						SendMessage (hwndChild, WM_COMMAND, wParam, lParam) ;

					break;        // and then to DefFrameProc
			}
			break;

		case WM_INITMENU:
			if (GetMenu(hwnd) == (HMENU)wParam)
				return DoIdleMenus((HWND)SendMessage(ghWndMDIClient, WM_MDIGETACTIVE, 0, 0), (HMENU)wParam);
			return 1;

		case WM_DESTROY :
			gShuttingDown = TRUE;
			PostQuitMessage(0);
			return 0 ;
	}

	return DefFrameProc(hwnd, ghWndMDIClient, message, wParam, lParam);
}

/* ------------------------------------------------------------- */

// Create the playthru monitor window
void CreateMonitorWindow()
{
	DWORD	dwVersion;
	HWND	hWnd;

	// Create the monitor window
	dwVersion = GetVersion();
	if ((dwVersion < 0x80000000) || (LOBYTE(LOWORD(dwVersion)) < 4))
	{
		// This is Windows NT or Win32s, so use the WM_MDICREATE message
		MDICREATESTRUCT mcs;

		mcs.szClass = szChildName;
		mcs.szTitle = "Monitor"; 
		mcs.hOwner  = ghInst;
		mcs.x       = CW_USEDEFAULT;
		mcs.y       = CW_USEDEFAULT;
		mcs.cx      = 100;
		mcs.cy      = 100;
		mcs.style   = WS_CHILD | WS_CAPTION;
		mcs.lParam  = 0;

		hWnd = (HWND) SendMessage(ghWndMDIClient,
									   WM_MDICREATE,
									   0,
									   (LPARAM)(LPMDICREATESTRUCT) &mcs);
	}
	else
	{
		// This method will only work with Windows 95, not Windows NT or Win32s
		hWnd = CreateWindowEx(WS_EX_MDICHILD | WS_CHILD | WS_CAPTION,
								   szChildName,
								   "Monitor",
								   0,
								   CW_USEDEFAULT,
								   CW_USEDEFAULT,
								   100,
								   100,
								   ghWndMDIClient, 
								   NULL,
								   ghInst,
								   0);
	}
	if (hWnd)
		gMonitor = GetNativeWindowPort(hWnd);
}

/* ------------------------------------------------------------- */

// Destroy the monitor window
void DestroyMonitorWindow(void)
{
	HWND	hWnd;

	if (hWnd = GetPortNativeWindow(gMonitor))
	{
		SendMessage(hWnd, WM_CLOSE, 0, 0);
	}
	gMonitor = 0;
}

/* ------------------------------------------------------------- */

long FAR PASCAL MonitorWndProc (HWND hWnd, UINT message, UINT wParam, LONG lParam)
{
	switch(message)
	{
		case WM_CREATE:
			// Associate a GrafPort with this window
			CreatePortAssociation(hWnd, NULL, 0);
			break;

		case WM_DESTROY:
			// Destroy the port association
			DestroyPortAssociation((CGrafPtr)gMonitor);
			gMonitor = 0;
			return 0;

		case WM_PAINT:
			if (gSeqGrabber)
			{
				PAINTSTRUCT	ps;
				HRGN	hRgnUpdate;

				// Get the native update region
				if (hRgnUpdate = CreateRectRgn(0,0,0,0))
					GetUpdateRgn(hWnd, hRgnUpdate, 0);

				UpdatePort(gMonitor);
				BeginPaint(hWnd, &ps);
				EndPaint(hWnd, &ps);
				if (hRgnUpdate)
				{
					RgnHandle	updateRgn;

					if (updateRgn = NativeRegionToMacRegion(hRgnUpdate))
					{
						SGUpdate(gSeqGrabber, updateRgn);
						DisposeRgn(updateRgn);
					}
					DeleteObject(hRgnUpdate);
				}
				BeginUpdate(gMonitor);
				EndUpdate(gMonitor);
			}
			break;

		case WM_ENTERSIZEMOVE:
			if (gSeqGrabber)
				SGPause(gSeqGrabber, true);
			break;

		case WM_EXITSIZEMOVE:
			// This is necessary, for now, to get the grab to start again afer the
			// dialog goes away.  For some reason the video destRect never gets reset to point
			// back to the monitor window.
			if (gSeqGrabber)
			{
				Rect	r;
				GrafPtr	oldPort, port = GetNativeWindowPort(hWnd);

				GetPort(&oldPort);

				// Make sure the port structures are up to date
				MacSetPort(port);
				UpdatePort(GetNativeWindowPort(hWnd));
				r = port->portRect;
				LocalToGlobal((Point*)&r.top);
				LocalToGlobal((Point*)&r.bottom);

				// Call the grabber alignment proc.  This would have been called continuously
				// during DragAlignedWindow, but since we don't go though that API, we just
				// call it when the window is done moving.
				(*gSeqGrabberAlignProc.alignmentProc)(&r, gSeqGrabberAlignProc.alignmentRefCon);

				MacSetPort(oldPort);

				// Start 'er up
				SGPause(gSeqGrabber, false);
			}
			break;
	}

	return DefMDIChildProc (hWnd, message, wParam, lParam);
}

/* ------------------------------------------------------------- */

int DoIdleMenus(HWND hWnd, HMENU hMenu)
{
	// File menu
	EnableMenuItem(hMenu, IDM_PRINT, (gVideoChannel != 0L ? MF_ENABLED : MF_GRAYED));

	// Edit menu
	EnableMenuItem(hMenu, IDM_UNDO, MF_GRAYED);
	EnableMenuItem(hMenu, IDM_CUT, (gVideoChannel != 0L) ? MF_ENABLED : MF_GRAYED);
	EnableMenuItem(hMenu, IDM_COPY, (gVideoChannel != 0L) ? MF_ENABLED : MF_GRAYED);
	EnableMenuItem(hMenu, IDM_PASTE, MF_GRAYED);
	EnableMenuItem(hMenu, IDM_DELETE, MF_GRAYED);
	
	// Monitor menu
	EnableMenuItem(hMenu, IDM_VIDEO_SETTINGS, (gVideoChannel != 0L) ? MF_ENABLED : MF_GRAYED);
	EnableMenuItem(hMenu, IDM_SOUND_SETTINGS, (gSoundChannel != 0L) ? MF_ENABLED : MF_GRAYED);
	EnableMenuItem(hMenu, IDM_RECORD_VIDEO, (gVideoChannel != 0L) ? MF_ENABLED : MF_GRAYED);
	CheckMenuItem(hMenu, IDM_RECORD_VIDEO, (gVideoChannel && gRecordVideo) ? MF_CHECKED : MF_UNCHECKED); 
	EnableMenuItem(hMenu, IDM_RECORD_SOUND, (gSoundChannel != 0L) ? MF_ENABLED : MF_GRAYED);
	CheckMenuItem(hMenu, IDM_RECORD_SOUND, (gSoundChannel && gRecordSound) ? MF_CHECKED : MF_UNCHECKED); 
	EnableMenuItem(hMenu, IDM_SPLIT_TRACKS, (gSoundChannel && gRecordSound && gVideoChannel && gRecordVideo) ? MF_ENABLED : MF_GRAYED);
	CheckMenuItem(hMenu, IDM_SPLIT_TRACKS, gSplitTracks ? MF_CHECKED : MF_UNCHECKED); 
	EnableMenuItem(hMenu, IDM_HALF_SIZE, (gVideoChannel != 0L) ? MF_ENABLED : MF_GRAYED);
	CheckMenuItem(hMenu, IDM_HALF_SIZE, gHalfSize ? MF_CHECKED : MF_UNCHECKED); 
	EnableMenuItem(hMenu, IDM_FULL_SIZE, (gVideoChannel != 0L) ? MF_ENABLED : MF_GRAYED);
	CheckMenuItem(hMenu, IDM_FULL_SIZE, gFullSize ? MF_CHECKED : MF_UNCHECKED); 
	EnableMenuItem(hMenu, IDM_RECORD, ((gSoundChannel && gRecordSound) || (gVideoChannel && gRecordVideo)) ? MF_ENABLED : MF_GRAYED);

	return 0;
}

/* ------------------------------------------------------------- */
