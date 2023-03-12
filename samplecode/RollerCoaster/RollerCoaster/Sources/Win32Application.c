/*
	File:		Win32Application.c

	Contains:	Standard Win32 event loop & window creation code

	Written by:	Scott Kuechle

	Copyright:	© 1998 by Apple Computer, Inc., all rights reserved.

	Change History (most recent first):

		<2>		9/28/98		rtm		made changes for Metrowerks compiler
		<1>	 	9/01/98		srk		first file
	    
 
 NOTES:

 
 TO DO:

*/

/************************************************************
*                                                           *
*    INCLUDE FILES                                          *
*                                                           *
*************************************************************/

#include "Win32Application.h"

/************************************************************
*                                                           *
*    GLOBAL VARIABLES                                       *
*                                                           *
*************************************************************/

HINSTANCE			ghInst;          	/* current instance */
DocumentRec			gDocument;
UINT				gTimer;
char szAppName[] = "RollerCoaster";		/* The name of this application */
char szTitle[]   = "RollerCoaster";		/* The title bar text */
COLORREF			customColors[16] = { 0L };			/* for use by choosecolor dialog */

/************************************************************
*                                                           *
*    CONSTANTS                                              *
*                                                           *
*************************************************************/

#define kMillsPerTick 10  /* milliseconds per timer tick */


/************************************************************
*                                                           *
*    FUNCTION PROTOTYPES                                    *
*                                                           *
*************************************************************/

static BOOL InitApplication(HINSTANCE hInstance);
static BOOL InitInstance( HINSTANCE hInstance, int nCmdShow);
static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM uParam, LPARAM lParam);
BOOL CenterWindow (HWND hwndChild, HWND hwndParent);
static LRESULT CALLBACK About(
								HWND hDlg,           /* window handle of the dialog box */
								UINT message,        /* type of message */
								WPARAM uParam,       /* message-specific information */
								LPARAM lParam);
void		UpdateFrame(void);
TQ3Status	GetToolBarPosition(DocumentPtr theDocument, unsigned long *x, unsigned long *y);
TQ3Status	GetToolBarSize(DocumentPtr theDocument, unsigned long *width, unsigned long *height);

static void StartTimer( void );
static void StopTimer( void );
VOID CALLBACK TimerProc(
						HWND hwnd,	/* handle of window for timer messages */
						UINT uMsg,	/* WM_TIMER message */
						UINT idEvent,	/* timer identifier */
						DWORD dwTime 	/* current system time */
					   );

/************************************************************
*                                                           *
*    FUNCTION:  WinMain                                     *
*                                                           *
*    PURPOSE:   Standard Windows WinMain routine            *
*                                                           *
*************************************************************/

int CALLBACK WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
					LPSTR lpCmdLine, int nCmdShow)
{
    MSG msg;
    HANDLE hAccelTable;

		if (!InitApplication(hInstance)) 
		{
			return (FALSE);    
		}

		if (!InitInstance(hInstance, nCmdShow)) 
		{
			return (FALSE);
		}

		StartTimer();

		hAccelTable = LoadAccelerators (hInstance, MAKEINTRESOURCE(IDR_GENERIC));

		while (GetMessage(&msg, NULL, 0, 0))
		{
			if (!TranslateAccelerator (msg.hwnd, hAccelTable, &msg))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}

		StopTimer();

		return (msg.wParam); /* Returns the value from PostQuitMessage */
}

/************************************************************
*                                                           *
*    FUNCTION:  UpdateFrame                                 *
*                                                           *
*    PURPOSE:   Invalidates our window so it will be        *
*               redrawn                                     *
*                                                           *
*************************************************************/

void UpdateFrame(void)
{
	RECT			aWinRect;
	BOOL			aResult;

		aResult = GetClientRect(gDocument.fMainWindow, (LPRECT)&aWinRect);
		aResult = InvalidateRect(gDocument.fMainWindow, &aWinRect, FALSE);	
}


/************************************************************
*                                                           *
*    FUNCTION:  InitApplication                             *
*                                                           *
*    PURPOSE:   Invalidates our window so it will be        *
*               redrawn                                     *
*                                                           *
*************************************************************/

BOOL InitApplication(HINSTANCE hInstance)
{
	WNDCLASS  wc;

			/* CS_OWNDC is required to support the Win32DC Draw context */
        wc.style         = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;

        wc.lpfnWndProc   = (WNDPROC)WndProc;       
        wc.cbClsExtra    = 0;                      
        wc.cbWndExtra    = 0;                     
        wc.hInstance     = hInstance;             
        wc.hIcon         = LoadIcon (hInstance, MAKEINTRESOURCE(IDI_APP)); 
        wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
        wc.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
        wc.lpszMenuName  = MAKEINTRESOURCE(IDR_GENERIC); 
        wc.lpszClassName = szAppName;              

        return (RegisterClass(&wc));
}

/************************************************************
*                                                           *
*    FUNCTION:  InitInstance                                *
*                                                           *
*    PURPOSE:   Creates our window                          *
*                                                           *
*                                                           *
*************************************************************/

BOOL InitInstance(
					HINSTANCE       hInstance,
					int             nCmdShow)
{
	HWND  hWnd; 

        ghInst = hInstance; 
        hWnd = CreateWindowEx(
				WS_EX_WINDOWEDGE | WS_EX_CLIENTEDGE,
                szAppName,          
                szTitle,             
                WS_OVERLAPPEDWINDOW, 
                25, 25, 640, 530, 	 /* fixed size windows */
                NULL,                
                NULL,                
                hInstance,           
                NULL                 
        );

        if (!hWnd) 
		{
            return (FALSE);
        }


        ShowWindow(hWnd, nCmdShow); 
        UpdateWindow(hWnd);        

        return (TRUE);             

}

/************************************************************
*                                                           *
*    FUNCTION:  WndProc                                     *
*                                                           *
*    PURPOSE:   Our window procedure                        *
*                                                           *
*                                                           *
*************************************************************/

LRESULT CALLBACK WndProc(
						HWND hWnd,        
						UINT message,      
						WPARAM uParam,     
						LPARAM lParam
						)   
{
    int wmId, wmEvent;
	PAINTSTRUCT PaintStruct;

        switch (message) 
		{
            case WM_COMMAND:

                wmId    = LOWORD(uParam);
                wmEvent = HIWORD(uParam);

                switch (wmId) 
				{
                    case IDM_ABOUT:
                        DialogBox(ghInst,          
                                MAKEINTRESOURCE(IDD_ABOUTBOX),
                                hWnd,                 
                                (DLGPROC)About);
                        break;

                    case IDM_EXIT:
                        DestroyWindow (hWnd);
                        break;

                    default:
                        return (DefWindowProc(hWnd, message, uParam, lParam));
                }
                break;

			case WM_CREATE:
				QD3DSupport_InitDoc3DData( hWnd, &gDocument );
				break;

            case WM_DESTROY:  /* message: window being destroyed */
                PostQuitMessage(0);
				QD3DSupport_DisposeDoc3DData( &gDocument );
                break;

			case WM_PAINT:
				BeginPaint(gDocument.fMainWindow, &PaintStruct);				
				QD3DSupport_DocDraw3DData( &gDocument );
				EndPaint(gDocument.fMainWindow, &PaintStruct);
				break;

			case WM_SIZE:
				{
				unsigned long width = LOWORD(lParam);  /* width of client area */
				unsigned long height = HIWORD(lParam); /* height of client area */

				break;
				}

			case WM_QUERYENDSESSION:
				return (TRUE);

            default:          /* Passes it on if unproccessed */
                return (DefWindowProc(hWnd, message, uParam, lParam));
        }
        return (0);
}


/************************************************************
*                                                           *
*    FUNCTION:  CenterWindow                                *
*                                                           *
*    PURPOSE:   Place our window on the screen              *
*                                                           *
*                                                           *
*************************************************************/

BOOL CenterWindow (HWND hwndChild, HWND hwndParent)
{
	RECT    rChild, rParent;
	int     wChild, hChild, wParent, hParent;
	int     wScreen, hScreen, xNew, yNew;
	HDC     hdc;

        /* Get the Height and Width of the child window */
        GetWindowRect (hwndChild, &rChild);
        wChild = rChild.right - rChild.left;
        hChild = rChild.bottom - rChild.top;

        /* Get the Height and Width of the parent window */
        GetWindowRect (hwndParent, &rParent);
        wParent = rParent.right - rParent.left;
        hParent = rParent.bottom - rParent.top;

        /* Get the display limits */
        hdc = GetDC (hwndChild);
        wScreen = GetDeviceCaps (hdc, HORZRES);
        hScreen = GetDeviceCaps (hdc, VERTRES);
        ReleaseDC (hwndChild, hdc);

        /* Calculate new X position, then adjust for screen */
        xNew = rParent.left + ((wParent - wChild) /2);
        if (xNew < 0)
		{
            xNew = 0;
        }
		else if ((xNew+wChild) > wScreen)
		{
            xNew = wScreen - wChild;
        }

        /* Calculate new Y position, then adjust for screen */
        yNew = rParent.top  + ((hParent - hChild) /2);
        if (yNew < 0)
		{
            yNew = 0;
        }
		else if ((yNew+hChild) > hScreen)
		{
            yNew = hScreen - hChild;
        }

        /* Set it, and return */
        return SetWindowPos (hwndChild, NULL,
                xNew, yNew, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
}

/************************************************************
*                                                           *
*    FUNCTION:  About                                       *
*                                                           *
*    PURPOSE:   About box callback                          *
*                                                           *
*                                                           *
*************************************************************/

LRESULT CALLBACK About(
					HWND hDlg,           /* window handle of the dialog box */
					UINT message,        /* type of message */
					WPARAM uParam,       /* message-specific information */
					LPARAM lParam)
{
    static  HFONT hfontDlg;
    LPSTR   lpVersion;
    DWORD   dwVerInfoSize;
    DWORD   dwVerHnd;
    UINT    uVersionLen;
    WORD    wRootLen;
    BOOL    bRetCode;
    int     i;
    char    szFullPath[256];
    char    szResult[256];
    char    szGetName[256];

        switch (message)
		{
                case WM_INITDIALOG:  /* message: initialize dialog box */
                        return (TRUE);

                case WM_COMMAND:                      /* message: received a command */
                        if (LOWORD(uParam) == IDOK        /* "OK" box selected? */
                        || LOWORD(uParam) == IDCANCEL) {  /* System menu close command? */
                                EndDialog(hDlg, TRUE);        /* Exit the dialog */
                                DeleteObject (hfontDlg);
                                return (TRUE);
                        }
                        break;
        }
        return (FALSE); /* Didn't process the message */
}

/************************************************************
*                                                           *
*    FUNCTION:  StartTimer                                  *
*                                                           *
*    PURPOSE:   Create a timer which will be used to        *
*               periodically call an update routine for     *
*               our window.                                 *
*                                                           *
*                                                           *
*************************************************************/

static void StartTimer( void )
{
	gTimer = SetTimer( NULL, 0, kMillsPerTick, (TIMERPROC) TimerProc );
}

/************************************************************
*                                                           *
*    FUNCTION:  StopTimer                                   *
*                                                           *
*    PURPOSE:   Stops our timer                             *
*                                                           *
*                                                           *
*************************************************************/

static void StopTimer( void )
{
	KillTimer( NULL, gTimer );
}

/************************************************************
*                                                           *
*    FUNCTION:  TimerProc                                   *
*                                                           *
*    PURPOSE:   Our timer procedure, which calls and        *
*               update routine for our window               *
*                                                           *
*                                                           *
*************************************************************/

VOID CALLBACK TimerProc(
						HWND hwnd,	/* handle of window for timer messages  */
						UINT uMsg,	/* WM_TIMER message */
						UINT idEvent,	/* timer identifier */
						DWORD dwTime 	/* current system time */
					   )	
{
	UpdateFrame();
}
