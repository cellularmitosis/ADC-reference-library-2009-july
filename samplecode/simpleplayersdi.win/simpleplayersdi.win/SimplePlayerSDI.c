 /*
	File:		SimplePlayerSDI.c

	Written by: Keith Gurganus

	SimplePlayerSDI.exe is a SDI application that plays a movie with QuickTime.This is a part 
	of the QuickTime sample source code and is provided as is.

  	Copyright:	© 1997 by Apple Computer, Inc., all rights reserved.
*/

#include "QTML.h"
#include "Movies.h"
#include "stdwin.h"
#include "SimplePlayerSDI.h"
#include "resource.h"
#include "QTime.h"

#define APPNAME "SimplePlayerSDI"

// Makes it easier to determine appropriate code paths:
#if defined (WIN32)
	#define IS_WIN32 TRUE
#else
	#define IS_WIN32 FALSE
#endif
#define IS_NT      IS_WIN32 && (BOOL)(GetVersion() < 0x80000000)
#define IS_WIN32S  IS_WIN32 && (BOOL)(!(IS_NT) && (LOBYTE(LOWORD(GetVersion()))<4))
#define IS_WIN95 (BOOL)(!(IS_NT) && !(IS_WIN32S)) && IS_WIN32


// Global Variables:
HINSTANCE			hInst;					// Current instance
char				szAppName[] = APPNAME;	// The name of this application
char				szTitle[]   = APPNAME;	// The title bar text
MovieStuff			gMovieStuff;			// Movie Structure
WINDOWPOS			gOldWindowPos = {nil, nil, 0, 0, 0, 0, 0};

// Prototypes
ATOM MyRegisterClass(CONST WNDCLASS*);
BOOL InitApplication(HINSTANCE);
BOOL InitInstance(HINSTANCE, int);
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK About(HWND, UINT, WPARAM, LPARAM);


/*
	WinMain
*/
int CALLBACK WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	MSG msg;
	HANDLE hAccelTable;

	if (!hPrevInstance) {
		// Perform instance initialization:
		if (!InitApplication(hInstance)) {
			return (FALSE);
		}
	}

	// Initialize QuickTime Media Layer
	InitializeQTML(0);

	// Initialize QuickTime
	EnterMovies();

	// Perform application initialization:
	if (!InitInstance(hInstance, nCmdShow)) {
		return (FALSE);
	}

	hAccelTable = LoadAccelerators (hInstance, MAKEINTRESOURCE(IDR_ACCELSIMPLESDI));

	// Main message loop:
	while (GetMessage(&msg, NULL, 0, 0)) {
		if (!TranslateAccelerator (msg.hwnd, hAccelTable, &msg)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	// Deinitialize QuickTime Media Layer
	ExitMovies();

	// Deinitialize QuickTime Media Layer
	TerminateQTML();
	
	return (msg.wParam);

	lpCmdLine; // This will prevent 'unused formal parameter' warnings
}


/*
	MyRegisterClass
*/
ATOM MyRegisterClass(CONST WNDCLASS *lpwc)
{
	HANDLE  hMod;
	FARPROC proc;
	WNDCLASSEX wcex;

	hMod = GetModuleHandle ("USER32");
	if (hMod != NULL) {

#if defined (UNICODE)
		proc = GetProcAddress (hMod, "RegisterClassExW");
#else
		proc = GetProcAddress (hMod, "RegisterClassExA");
#endif

		if (proc != NULL) {

			wcex.style         = lpwc->style;
			wcex.lpfnWndProc   = lpwc->lpfnWndProc;
			wcex.cbClsExtra    = lpwc->cbClsExtra;
			wcex.cbWndExtra    = lpwc->cbWndExtra;
			wcex.hInstance     = lpwc->hInstance;
			wcex.hIcon         = lpwc->hIcon;
			wcex.hCursor       = lpwc->hCursor;
			wcex.hbrBackground = lpwc->hbrBackground;
			wcex.lpszMenuName  = lpwc->lpszMenuName;
			wcex.lpszClassName = lpwc->lpszClassName;

			// Added elements for Windows 95:
			wcex.cbSize = sizeof(WNDCLASSEX);
			wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDR_SMALL));
			
			return (*proc)(&wcex);//return RegisterClassEx(&wcex);
		}
	}
	return (RegisterClass(lpwc));
}


/*
	InitApplication
*/
BOOL InitApplication(HINSTANCE hInstance)
{
    WNDCLASS  wc;
    HWND      hwnd;

    // Win32 will always set hPrevInstance to NULL, so lets check
    // things a little closer. This is because we only want a single
    // version of this app to run at a time
    hwnd = FindWindow (szAppName, NULL);
    if (hwnd) {
        // We found another version of ourself. Lets defer to it:
        if (IsIconic(hwnd)) {
            ShowWindow(hwnd, SW_RESTORE);
        }
        SetForegroundWindow (hwnd);

        return FALSE;
	}

    // Fill in window class structure with parameters that describe
    // the main window.
    wc.style         = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc   = (WNDPROC)WndProc;
    wc.cbClsExtra    = 0;
    wc.cbWndExtra    = 0;
    wc.hInstance     = hInstance;
    wc.hIcon         = LoadIcon (hInstance, MAKEINTRESOURCE(IDI_BIG));
    wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
	wc.lpszMenuName  = MAKEINTRESOURCE(IDR_SIMPLESDI);
    wc.lpszClassName = szAppName;

    // Register the window class and return success/failure code.
    if (IS_WIN95) {
		return MyRegisterClass(&wc);
    } else {
		return RegisterClass(&wc);
    }
}

/*
	InitInstance
*/
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	HWND hWnd;
	
	hInst = hInstance; // Store instance handle in our global variable

	// Create our window
	hWnd = CreateWindow(szAppName, szTitle, WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, 0, CW_USEDEFAULT, 0,
		NULL, NULL, hInstance, NULL);

	if (!hWnd) {
		return (FALSE);
	}

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	return (TRUE);
}

/*
	WndProc
*/
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
		int			wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC			hdc;

	if(GetNativeWindowPort(hWnd)){
		MSG	msg;
		EventRecord	macEvent;
		LONG thePoints = GetMessagePos();

		msg.hwnd = hWnd;
		msg.message = message;
		msg.wParam = wParam;
		msg.lParam = lParam;
		msg.time = GetMessageTime();
		msg.pt.x = LOWORD(thePoints);
		msg.pt.y = HIWORD(thePoints);
		WinEventToMacEvent(&msg, &macEvent);  // Convert the message to a QTML event

		// if we have a Movie Controller, pass the QTML event
		if(gMovieStuff.theMC)
			MCIsPlayerEvent(gMovieStuff.theMC,(const EventRecord *)&macEvent);
	}	

	switch (message) { 
		case WM_CREATE:
			memset(&gMovieStuff, 0, sizeof(MovieStuff));
			
			// Register this HWND with QTML
			CreatePortAssociation(hWnd, NULL, 0L);	
			gMovieStuff.theHwnd = hWnd;
			break;

		case WM_COMMAND:
			wmId    = LOWORD(wParam);
			wmEvent = HIWORD(wParam);

			//Parse the menu selections:
			switch (wmId) {
	
				case IDM_ABOUT:
					DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, (DLGPROC)About);
					break;

				case IDM_EXIT:
					CloseMovie(&gMovieStuff);
					DestroyPortAssociation((CGrafPort *)GetNativeWindowPort(hWnd));
					DestroyWindow(hWnd);
					break;

				case IDM_OPEN:
					// Open a movie file
					if(GetFile(gMovieStuff.filename)){
						// Close any open movie
						CloseMovie(&gMovieStuff);
						OpenMovie(hWnd, &gMovieStuff);  // open the movie and size the window
					}
					break;
			}
			break;

		case WM_PAINT:
			hdc = BeginPaint (hWnd, &ps);
			// Add any additional drawing code here...
			EndPaint (hWnd, &ps);
			break;
			
		case WM_CLOSE:
			// Unregister the HWND with QTML
			CloseMovie(&gMovieStuff);
			DestroyPortAssociation((CGrafPtr)GetNativeWindowPort(hWnd));
			break;

		case WM_DESTROY:
			PostQuitMessage(0);
			break;

		case WM_ENTERSIZEMOVE:
			{
				RECT	clientRect;

				GetClientRect(hWnd, & clientRect );

				gOldWindowPos.cx = clientRect.right - clientRect.left;
				gOldWindowPos.cy = clientRect.bottom - clientRect.top;
			}
			break;
		case WM_EXITSIZEMOVE:
			if ( gMovieStuff.theMovie )
			{
				RECT		clientRect;
				long		widthAdjust = 0, heightAdjust = 0;

				GetClientRect(hWnd, &clientRect );
				widthAdjust = (clientRect.right - clientRect.left) - gOldWindowPos.cx;
				heightAdjust = (clientRect.bottom - clientRect.top) - gOldWindowPos.cy;
				if ( widthAdjust || heightAdjust )
				{
					Rect		controllerBox;

					MCGetControllerBoundsRect(gMovieStuff.theMC, &controllerBox);
					controllerBox.right += (short)widthAdjust;
					controllerBox.bottom += (short)heightAdjust;
					MCSetControllerBoundsRect(gMovieStuff.theMC, &controllerBox);
				}	
			}
			break;
	}
	return (DefWindowProc(hWnd, message, wParam, lParam));
}

/*
	About
*/
LRESULT CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message) {
		case WM_COMMAND:
			if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL) {
				EndDialog(hDlg, TRUE);
				return (TRUE);
			}
			break;
	}
    return FALSE;
}



