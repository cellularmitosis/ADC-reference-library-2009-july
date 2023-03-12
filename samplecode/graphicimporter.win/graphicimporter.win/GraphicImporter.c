 /*
	File:		GraphicImporter.c

	Written by: Keith Gurganus

	GraphicImporter.exe is a MDI application that demonstates QuickTime graphic import 
	functions. This is a part of the QuickTime sample source code and is provided as is.

  	Copyright:	© 1997 by Apple Computer, Inc., all rights reserved.
*/

#include "QTML.h"
#include "Movies.h"
#include "MacWindows.h"
#include "Resources.h"
#include "ImageCompression.h"
#include "stdwin.h"
#include "resource.h"
#include "TextUtils.h"

// Globals
HINSTANCE	ghModule;
HWND		ghwndMain   = NULL;
HWND		ghwndClient = NULL;
HMENU		hMenu,      hMenuWindow;
HMENU		hChildMenu, hChildMenuWindow; 
int			gMDICount=1;


// Instance data for each MDI child window
typedef struct _PerWndInfo {
    HWND					hParent;
	HWND					hwndChildWindow;
    RECT					rcClient;
	FSSpec					spec;
	OPENFILENAME			ofn;
	GraphicsImportComponent	gi;
	Rect					imageRect;
} INFO, *PINFO;

// Defines
#define USEEXPLORERSTYLE (LOBYTE(LOWORD(GetVersion()))>=4)

// Prototypes
LRESULT CALLBACK MainWndProc     (HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK MDIWndProc      (HWND, UINT, WPARAM, LPARAM);
BOOL	CALLBACK About           (HWND, UINT, WPARAM, LPARAM);
BOOL	InitializeApp(void);
BOOL	DoOpen(PINFO pInfo);
BOOL	DoGraphicsImport(PINFO pInfo);
static	UINT APIENTRY GenericHook(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
void	ParseCmdLinePriv(LPSTR cmdLine);
BOOL	DoFileOpen(char* fullPathName);

/*
	WinMain
*/
int CALLBACK WinMain ( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
    MSG    msg;
    HANDLE hAccel;

	// Store our module handle
    ghModule = GetModuleHandle(NULL);

	// Initialize QuickTime Media Layer
	InitializeQTML(0);

	// Initialize QuickTime
	EnterMovies();

	// Initialize the app
    if (!InitializeApp()) 
    {
        MessageBox(ghwndMain, "MDI: InitializeApp failure!", "Error", MB_OK);
        return 0;
    }

	// Load our accelerator keys
    if (!(hAccel = LoadAccelerators (ghModule, MAKEINTRESOURCE(ACCEL_ID))))
        MessageBox(ghwndMain, "MDI: Load Accel failure!", "Error", MB_OK);

	// Parse the command line for drag and drop
	ParseCmdLinePriv(NULL);

	// Loop for messages
    while (GetMessage(&msg, NULL, 0, 0)) {
        if (!TranslateAccelerator( ghwndMain, hAccel, &msg) &&
            !TranslateMDISysAccel(  ghwndClient, &msg)          ) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

	// Deinitialize QuickTime
	ExitMovies();

	// Deinitialize QTML
	TerminateQTML();

    return TRUE;
}


/*
	InitializeApp
*/
BOOL InitializeApp(void)
{

    WNDCLASS wc;
	RECT	rcDesktop;

    wc.style            = CS_OWNDC;
    wc.lpfnWndProc      = (WNDPROC) MainWndProc;
    wc.cbClsExtra       = 0;
    wc.cbWndExtra       = sizeof(LONG);
    wc.hInstance        = ghModule;
    wc.hIcon            = LoadIcon(NULL, MAKEINTRESOURCE(IDI_APP));
    wc.hCursor          = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground    = (HBRUSH)(COLOR_APPWORKSPACE);
    wc.lpszMenuName     = MAKEINTRESOURCE(IDR_MAINMENU);
    wc.lpszClassName    = "MDIDemoClass";

    if (!RegisterClass(&wc))
        return FALSE;

    wc.lpfnWndProc      = (WNDPROC) MDIWndProc;
    wc.hIcon            = LoadIcon(NULL, MAKEINTRESOURCE(IDI_APP));
    wc.lpszMenuName     = NULL;
    wc.lpszClassName    = "MDIClass";

    if (!RegisterClass(&wc))
        return FALSE;


    hMenu       = LoadMenu(ghModule, MAKEINTRESOURCE(IDR_MAINMENU));
    hChildMenu  = LoadMenu(ghModule, MAKEINTRESOURCE(IDR_CHILDMENU));
    hMenuWindow      = GetSubMenu(hMenu, 1);
    hChildMenuWindow = GetSubMenu(hChildMenu, 2);

	// Get the size for our window
	GetWindowRect(GetDesktopWindow(), &rcDesktop);
    ghwndMain = CreateWindowEx(0L, "MDIDemoClass", "Graphic Importer",
            WS_OVERLAPPED   | WS_CAPTION     | WS_BORDER       |
            WS_THICKFRAME   | WS_MAXIMIZEBOX | WS_MINIMIZEBOX  |
            WS_CLIPCHILDREN | WS_VISIBLE     | WS_SYSMENU,
            0, 0, rcDesktop.right - rcDesktop.left, rcDesktop.bottom - rcDesktop.top,
            NULL, hMenu, ghModule, NULL);

    if (ghwndMain == NULL)
        return FALSE;

	// Store our data
    SetWindowLong(ghwndMain, GWL_USERDATA, 0L);

	// Set initial focus 
    SetFocus(ghwndMain);

    return TRUE;
}

/*
	MainWndProc
*/
LRESULT CALLBACK MainWndProc (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{

    CLIENTCREATESTRUCT	clientcreate;
	DWORD				dwStyle;	

    switch (message) 
    {

      case WM_CREATE:
        SetWindowLong(hwnd, 0, (LONG)NULL);
        clientcreate.hWindowMenu  = hMenuWindow;
        clientcreate.idFirstChild = 1;
	
		dwStyle = WS_CHILD | WS_CLIPCHILDREN | WS_VISIBLE;			

		// Create our child window
		ghwndClient = CreateWindow("MDICLIENT", NULL,
										dwStyle,
										0,0,0,0,
										hwnd, NULL, ghModule, (LPVOID)&clientcreate);

        return 0L;

      case WM_DESTROY: 
		PostQuitMessage(0);
        return 0L;

      case WM_SYSCOMMAND:
		if ( LOWORD(wParam) == MM_ABOUT)   {
			SendMessage(hwnd,WM_COMMAND,MM_ABOUT,0);
			return 0;
		} else
              return DefWindowProc (hwnd, message, wParam, lParam);
		break;

      case WM_COMMAND:

        switch (LOWORD(wParam)) 
        {
            
			// File Menu
			case ID_FILE_OPEN: 
				if(DoFileOpen(NULL))
					return 0L;
				break;
			
			case ID_FILE_CLOSE:
				DestroyWindow((HWND)SendMessage(ghwndClient, WM_MDIGETACTIVE, 0L, FALSE));
				break;

			case ID_FILE_EXIT:
	            PostMessage(hwnd, WM_SYSCOMMAND, SC_CLOSE, 0L);
	            break;
			
			// Window Menu
            case IDM_TILE:
                SendMessage(ghwndClient, WM_MDITILE, 0L, 0L);
                return 0L;
            
			case IDM_CASCADE:
                SendMessage(ghwndClient, WM_MDICASCADE, 0L, 0L);
                return 0L;
            
			case IDM_ARRANGE:
                SendMessage(ghwndClient, WM_MDIICONARRANGE, 0L, 0L);
                return 0L;

            case ID_WINDOW_ABOUT:
                if (DialogBox(ghModule, "AboutBox", ghwndMain, (DLGPROC)About) == -1){
					MessageBox(ghwndMain, "About Dialog Creation Error!", "Error", MB_OK);
				}
				return 0L;

            default:
                return DefFrameProc(hwnd,  ghwndClient, message, wParam, lParam);
        }

    default:

        return DefFrameProc(hwnd,  ghwndClient, message, wParam, lParam);
    }
}

/*
	MDIWndProc
*/
LRESULT CALLBACK MDIWndProc( HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    HANDLE      hInfo;
	PINFO       pInfo;
	MSG			msg;
	EventRecord	macEvent;
	LONG		thePoints = GetMessagePos();
	PAINTSTRUCT	ps;

	msg.hwnd = hwnd;
	msg.message = message;
	msg.wParam = wParam;
	msg.lParam = lParam;
	msg.time = GetMessageTime();
	msg.pt.x = LOWORD(thePoints);
	msg.pt.y = HIWORD(thePoints);
	WinEventToMacEvent(&msg, &macEvent);           

    switch (message) 
    {
		case WM_CREATE:
		case WM_MDICREATE:
			break;
		case WM_DESTROY:
			hInfo = (HANDLE)GetWindowLong(hwnd, GWL_USERDATA);
            if (hInfo) 
            {
                if ((pInfo = (PINFO)LocalLock(hInfo)) != NULL){
					if (pInfo->gi) 
						// close the graphic import component
						CloseComponent(pInfo->gi);

						// Destroy our port association
						DestroyPortAssociation((CGrafPort *)GetHWNDPort(pInfo->hwndChildWindow));
				}
                LocalUnlock(hInfo);
			}
			break;

		// Draw our graphic
		case WM_PAINT:
			BeginPaint(hwnd, &ps); 
			hInfo = (HANDLE)GetWindowLong(hwnd, GWL_USERDATA);
            if (hInfo) 
            {
                if ((pInfo = (PINFO)LocalLock(hInfo)) != NULL)
					GraphicsImportDraw(pInfo->gi);
			
                LocalUnlock(hInfo);
			}
			EndPaint(hwnd, &ps);
			break;

        default:
            return DefMDIChildProc(hwnd, message, wParam, lParam);

    }
    return DefMDIChildProc(hwnd, message, wParam, lParam);
}

/*
	About
*/
BOOL CALLBACK About (HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message) 
    {
    case WM_INITDIALOG:
        return TRUE;

    case WM_COMMAND:
        if (wParam == IDOK)
            EndDialog(hDlg, wParam);
        break;
    }

    return FALSE;
}

/*
	DoGraphicsImport
*/
BOOL DoGraphicsImport(PINFO pInfo)
{
	long	err;

	// Get the graphics importer component
	c2pstr(pInfo->spec.name);
	err = GetGraphicsImporterForFile(&pInfo->spec, &pInfo->gi);
	
	if (err) 
		goto bail;

	// Get the pixel bounds of this image
	err = GraphicsImportGetNaturalBounds(pInfo->gi, &pInfo->imageRect);

bail:
	if (err)
		return FALSE;
	else
		return TRUE;
}

/*
	DoOpen
*/
BOOL DoOpen(PINFO pInfo)
{
    pInfo->ofn.lStructSize = sizeof(OPENFILENAME);
	pInfo->ofn.lpstrTitle = "Select an Image File";
	pInfo->ofn.lpstrFile = pInfo->spec.name;
	pInfo->ofn.nMaxFile = 256;
    pInfo->ofn.lpstrFilter = "All Files (*.*) \0 *.*\0";
	if(USEEXPLORERSTYLE)
		pInfo->ofn.Flags |= OFN_ENABLEHOOK | OFN_EXPLORER;
	else
		pInfo->ofn.Flags |= OFN_ENABLEHOOK;
	pInfo->ofn.lpfnHook = GenericHook;

	// Open the file...
	return GetOpenFileName(&pInfo->ofn);
}

/*
	GenericHook
*/
static UINT APIENTRY GenericHook(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
		case WM_INITDIALOG:		// Center window
		{
			Point	ptTopLeft;
			RECT	rcWindow;
			BOOL	retValue;
			HWND	theWnd = hWnd;
			RECT	rcDesktopWindow;
			long	width;
			long	height;

			// if we are using 95 or NT 4.0 use the new explorer style
			if(USEEXPLORERSTYLE)
				theWnd = GetParent(hWnd);

			GetWindowRect(theWnd, &rcWindow);

			width = rcWindow.right - rcWindow.left;
			height = rcWindow.bottom - rcWindow.top;
			GetWindowRect(GetDesktopWindow(), &rcDesktopWindow);
			ptTopLeft.h = (short)((rcDesktopWindow.right + rcDesktopWindow.left)/2 - width/2);
			ptTopLeft.v = (short)((rcDesktopWindow.top + rcDesktopWindow.bottom)/3 - height/3);
		
			retValue = SetWindowPos(theWnd, 0, ptTopLeft.h, ptTopLeft.v, 0, 0, SWP_NOZORDER|SWP_NOSIZE);
	
			return true;
		}
	}
	return 0;
}

/*
	DoFileOpen
*/
BOOL DoFileOpen(char* fullPathName)
{
	HANDLE			hInfo;
    PINFO			pInfo;
    MDICREATESTRUCT mdicreate;
	CGrafPtr		port;
	GDHandle		gd;

	// Allocate memory for INFO to be associated with the
	hInfo = LocalAlloc(LHND, (WORD) sizeof(INFO));
	if (hInfo) 
	{
		if ((pInfo = (PINFO)LocalLock(hInfo)) == NULL)
			MessageBox(ghwndMain, "Failed in LocalLock", "Error", MB_OK);

		// if we do not have a fullPathName, display an open dialog
		if(fullPathName == NULL) {
			// Display an open dialog
			if(	!DoOpen(pInfo)){
				LocalFree(hInfo);
				return FALSE;
			}
		} else {
			// Else we open the file
			pInfo->ofn.lpstrFile = fullPathName;
			strcpy(pInfo->spec.name, fullPathName);
			pInfo->ofn.nMaxFile = 256;
		}

		// Can we import this file?
		if(!DoGraphicsImport(pInfo)){
			LocalFree(hInfo);
			return FALSE;
		} else
			gMDICount++;

		pInfo->hParent    = ghwndClient;
		mdicreate.szClass = "MDIClass";
		mdicreate.szTitle = (LPTSTR)((*pInfo).ofn).lpstrFile;
		mdicreate.hOwner  = ghModule;
		mdicreate.x       = CW_USEDEFAULT;
		mdicreate.y       = CW_USEDEFAULT;
		mdicreate.cx      = pInfo->imageRect.right - pInfo->imageRect.left;
		mdicreate.cy      = pInfo->imageRect.bottom - pInfo->imageRect.top + GetSystemMetrics(SM_CYCAPTION);
		mdicreate.style   = 0L;
		// pass the handle of the per MDI child INFO to the child MDI window for storage
		mdicreate.lParam  = (LONG) hInfo;

		// Create Child Window
		pInfo->hwndChildWindow =
							(HANDLE) SendMessage(ghwndClient, WM_MDICREATE,
							0L,
							(LONG)(LPMDICREATESTRUCT)&mdicreate);

		if (pInfo->hwndChildWindow == NULL) 
		{
			MessageBox(ghwndMain, "Failed in Creating Child Window", "Error", MB_OK);
			return FALSE;
		}

		// Create our port association && set the port
		MacSetPort(CreatePortAssociation(pInfo->hwndChildWindow, NULL, 0));
	
		// These lines required to get the image to draw within the window
		GetGWorld(&port, &gd);
		GraphicsImportSetGWorld(pInfo->gi, port, gd);
		
		// Store our private data
		SetWindowLong(pInfo->hwndChildWindow, GWL_USERDATA, (LONG)hInfo);

		return TRUE;
    } else 
		MessageBox(ghwndMain, "Failed to Allocate INFO data!", "Error", MB_OK);

	return FALSE;
}

/*
	ParseCmdLinePriv

	Parse the command line when the applcation first starts.
*/
void ParseCmdLinePriv(LPSTR cmdLine)
{
	LPSTR lpCmdLine, lpT;

	if(cmdLine == NULL)
		lpCmdLine = GetCommandLine();
	else
		lpCmdLine = cmdLine;

	if (*lpCmdLine) {
		lpT = strchr(lpCmdLine, ' ');   // skip self name
		if (lpT) {
			lpCmdLine = lpT;
			while (*lpCmdLine == ' ') {
				lpCmdLine++;            // skip spaces to end or first cmd
			}

			while (*lpCmdLine != '\0'){
				char buff[255];
				int i;
				for (i = 0; (*lpCmdLine != ' ') && (*lpCmdLine != '\0'); i++, lpCmdLine++) {
					buff[i] = *lpCmdLine;
				}

				// Null terminate
				if(*lpCmdLine != '\0')
					lpCmdLine++;
				
				buff[i] = '\0';

				// Open the file
				DoFileOpen(buff);	
			}

		} else
			lpCmdLine += strlen(lpCmdLine);   // point to NULL
		
	}
}  



