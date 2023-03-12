/*
 File:  Print.c
 
 Description: QuickTime sample code showing how to use QuickTime routines to draw
	            into a standard Win32 printing device context

 Copyright:    Copyright 2003 Apple Computer, Inc. All rights reserved.
 
 Disclaimer: IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc.
    ("Apple") in consideration of your agreement to the following terms, and your
    use, installation, modification or redistribution of this Apple software
    constitutes acceptance of these terms.  If you do not agree with these terms,
    please do not use, install, modify or redistribute this Apple software.

    In consideration of your agreement to abide by the following terms, and subject
    to these terms, Apple grants you a personal, non-exclusive license, under Apple's
    copyrights in this original Apple software (the "Apple Software"), to use,
    reproduce, modify and redistribute the Apple Software, with or without
    modifications, in source and/or binary forms; provided that if you redistribute
    the Apple Software in its entirety and without modifications, you must retain
    this notice and the following text and disclaimers in all such redistributions of
    the Apple Software.  Neither the name, trademarks, service marks or logos of
    Apple Computer, Inc. may be used to endorse or promote products derived from the
    Apple Software without specific prior written permission from Apple.  Except as
    expressly stated in this notice, no other rights or licenses, express or implied,
    are granted by Apple herein, including but not limited to any patent rights that
    may be infringed by your derivative works or by other works in which the Apple
    Software may be incorporated.

    The Apple Software is provided by Apple on an "AS IS" basis.  APPLE MAKES NO
    WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED
    WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS FOR A PARTICULAR
    PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND OPERATION ALONE OR IN
    COMBINATION WITH YOUR PRODUCTS.

    IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL OR
    CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
    GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
    ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION, MODIFICATION AND/OR DISTRIBUTION
    OF THE APPLE SOFTWARE, HOWEVER CAUSED AND WHETHER UNDER THEORY OF CONTRACT, TORT
    (INCLUDING NEGLIGENCE), STRICT LIABILITY OR OTHERWISE, EVEN IF APPLE HAS BEEN
    ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
    
 Change History (most recent first): 
 
	<1> 9/1/03 srk	initial release

*/

/************************************************************
*                                                           *
*    INCLUDE FILES                                          *
*                                                           *
*************************************************************/

#include "ConditionalMacros.h"

	// Windows headers
#define	STRICT
#include <windows.h>
	// QTML stuff
#include "QTML.h"
#include "MacTypes.h"
#include "MacMemory.h"
#include "Errors.h"
#include "Fonts.h"
#include "QuickDraw.h"
#include "Resources.h"
#include "Gestalt.h"
#include "FixMath.h"
#include "Sound.h"
#include "string.h"
#include "Movies.h"
#include "ImageCompression.h"
#include "Script.h"
#include "TextUtils.h"
#include "Processes.h"

#include "QTCode.h"


/************************************************************
*                                                           *
*    FUNCTION PROTOTYPES                                    *
*                                                           *
*************************************************************/

HDC  GetPrinterDC (void) ;
void PageGDICalls (HDC, int, int) ;
static BOOL PrintAPage ();
static void ShowWin32Error();
static HDC SelectPrinter(HWND hWnd);

/************************************************************
*                                                           *
*    FUNCTION:  WinMain                                     *
*                                                           *
*    PURPOSE:   Standard Win32 main routine                 *
*                                                           *
*************************************************************/

int WINAPI WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance,
                    PSTR szCmdLine, int iCmdShow)
{
	BOOL printErr = PrintAPage();

	return 0;
}

/************************************************************
*                                                           *
*    FUNCTION:  PrintAPage                                 *
*                                                           *
*    PURPOSE:   Contains code to find and print to the      *
*               default printer. However, we use QuickTime  *
*               QuickDraw drawing functions to actually     *
*               draw into the printer device context        *
*                                                           *
*                                                           *
*************************************************************/

static BOOL PrintAPage ()
{
	static DOCINFO docInfo = { sizeof (DOCINFO), "Print Document", NULL } ;
	BOOL           printError = FALSE ;
	HDC            printerHDC ;
	int            xPage, yPage ;
	OSErr          err;
	RECT           rectDest = {0,0,0,0};

	// get printer device context for selecte printer -
	// we'll put up the dialog to let the user choose
	if (NULL == (printerHDC = SelectPrinter (NULL)))

	// use this code if you want to "silently" enumerate
	// the list of printers
//	if (NULL == (printerHDC = GetPrinterDC ()))
	  return TRUE ;

	xPage = GetDeviceCaps (printerHDC, HORZRES) ;
	yPage = GetDeviceCaps (printerHDC, VERTRES) ;

	// start print job
	if (StartDoc (printerHDC, &docInfo) > 0)
	{
		// prepare the printer driver to accept data
		if (StartPage (printerHDC) > 0)
		{
			rectDest.right = xPage;
			rectDest.bottom = yPage;
				
			/* draw into the printer device context using
				QuickTime QuickDraw drawing routines */
			err = RenderIntoHBITMAPExample(printerHDC,
										&rectDest);

			if (EndPage (printerHDC) > 0)
			{
				EndDoc (printerHDC) ;
			}
			else
			{
				printError = TRUE ;
			}
		}
	}
	else
	{
	  printError = TRUE ;
	}

	DeleteDC (printerHDC);

	return printError ;
}


/************************************************************
*                                                           *
*    FUNCTION:  GetPrinterDC                                *
*                                                           *
*    PURPOSE:   Contains code to retrieve the default       *
*               printer, and create a printer device        *
*               context from that printer                   *
*                                                           *
*************************************************************/


static HDC GetPrinterDC (void)
{
	PRINTER_INFO_5 pinfo5[10] ; 
	DWORD          dwNeeded, dwReturned ;

	if (EnumPrinters (PRINTER_ENUM_DEFAULT, NULL, 5, (LPBYTE) pinfo5,
				   sizeof (pinfo5), &dwNeeded, &dwReturned))
	{
	  return CreateDC (NULL, pinfo5[0].pPrinterName, NULL, NULL) ;
	}
	else
	{
		ShowWin32Error();     // show error
		return 0 ;            // return null HDC
	}

}

/************************************************************
*                                                           *
*    FUNCTION:  SelectPrinter                               *
*                                                           *
*    PURPOSE:   Contains code to retrieve the printer		*
*				device context for the selected printer     *
*               by displaying the dialog box and letting    *
*               the user choose                             *
*************************************************************/

static HDC SelectPrinter(HWND hWnd) 
{ 
	// Local variables 
	BOOL bPrintDlg; // Return code from PrintDlg function 
	PRINTDLG pd; // Printer dialog structure 
	DWORD dwError; 
	HDC hDC; // DC to printer 

	// Initialize variables 
	hDC = NULL; 

	/* Initialize the PRINTDLG members. */ 

	pd.lStructSize = sizeof(PRINTDLG); 
	pd.hwndOwner = hWnd; 
	pd.Flags = PD_RETURNDC | PD_PRINTSETUP; 
	pd.hDevMode = (HANDLE) NULL; 
	pd.hDevNames = (HANDLE) NULL; 
	pd.hDC = (HDC) NULL; 
	pd.nFromPage = 1; 
	pd.nToPage = 1; 
	pd.nMinPage = 0; 
	pd.nMaxPage = 0; 
	pd.nCopies = 1; 
	pd.hInstance = (HANDLE) NULL; 
	pd.lCustData = 0L; 
	pd.lpfnPrintHook = (LPPRINTHOOKPROC) NULL; 
	pd.lpfnSetupHook = (LPSETUPHOOKPROC) NULL; 
	pd.lpPrintTemplateName = (LPTSTR) NULL; 
	pd.lpSetupTemplateName = (LPTSTR) NULL; 
	pd.hPrintTemplate = (HANDLE) NULL; 
	pd.hSetupTemplate = (HANDLE) NULL; 

	// Display the PRINT dialog box. 
	bPrintDlg = PrintDlg(&pd); 
	if (!bPrintDlg) // Either no changes, or a common dialog error occured 
	{ 
		dwError = CommDlgExtendedError(); 
		if (dwError != 0) 
		{ 
			return(NULL); 
		} 
	} 
	else // Passed call, set up DC 
	{ 
		hDC = pd.hDC; 
	} 
	return(hDC); 
} // End of SelectPrinter 


/************************************************************
*                                                           *
*    FUNCTION:  ShowWin32Error                              *
*                                                           *
*    PURPOSE:   Displays Win32 function error messages      *
*                                                           *
*************************************************************/

static void ShowWin32Error()
{
	LPVOID lpMsgBuf;

	FormatMessage( 
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL,
		GetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
		(LPTSTR) &lpMsgBuf,
		0,
		NULL 
	);

	// Display the string.
	MessageBox( NULL, lpMsgBuf, "GetLastError", MB_OK|MB_ICONINFORMATION );

	// Free the buffer.
	LocalFree( lpMsgBuf );

}
