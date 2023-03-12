/*
 File:  QTCode.c
 
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

static void QuickTimeInit (void);
static void Utils_DisplayMsg(char *msg);
static void CheckError(OSErr error, char *msg);


/************************************************************
*                                                           *
*    FUNCTION:  RenderIntoHBITMAPExample                    *
*                                                           *
*    PURPOSE:   The sample code first creates a GWorld in a *
*               Win32 compatible pixel format (in our       *
*               example code we use k32BGRAPixelFormat).    *
*               Once created the GWorld's HDC and HBITMAP   *
*               are retrieved using GetPortHDC and          *
*               GetPortHBITMAP. The active port is selected *
*               by calling SetGWorld, and graphics rendered *
*               using the Quickdraw APIs RGBForeColor and   *
*               PaintRect. Additionally, native Win32 GDI   *
*               calls are used to render graphics into the  *
*               same GWorld. Finally, the contents of the   *
*               port is copied to a secondary HDC via       *
*               ScaleBlt, using the GWorld's HDC as a       *
*               source. Note that the HBITMAP and HDC are   *
*               owned and managed by the GWorld, and are not*
*               disposed of.                                *
*                                                           *
*               (code taken from Ice Floe Dispatch #16      *
*               from the QuickTime web site)                *
*                                                           *
*************************************************************/

OSErr RenderIntoHBITMAPExample(HDC hdcDest, RECT *rectDest)
{
	GWorldPtr gw = nil;
	CGrafPtr savedPort;
	GDHandle savedGD;
	HDC hdcSrc;
	HBITMAP hbitmapSrc;
	Rect bounds;
	OSErr result = noErr;
   
   	
	// first perform QuickTime initialization
	QuickTimeInit();

	// Create a 256 x 256 32BGRA GWorld
	bounds.top = bounds.left = 0;
	bounds.bottom = bounds.right = 256;
 
	result = QTNewGWorld(&gw,k32BGRAPixelFormat,&bounds,NULL,NULL,0);
   
	// check for errors
	if (result != noErr)
		goto bail;
   
	// retrieve the associated HDC and HBITMAP
	hdcSrc = GetPortHDC((GrafPtr)gw);
	hbitmapSrc = GetPortHBITMAP((GrafPtr)gw);
   
	// bail if DIB allocation failed.
	if ((hdcSrc == 0) || (hbitmapSrc == 0)) {
		result = memFullErr;
		goto bail;
	}
	
	// save current port and GDevice, set current port to new GWorld
	GetGWorld(&savedPort,&savedGD);
	SetGWorld(gw,NULL);

	// Render graphics into GWorld
	{
		Rect macRect;
		RGBColor color;
		RECT winRect;
		HBRUSH hBrush, hBrushOld;
 
		// red
		color.red = 0xffff; color.green = 0; color.blue = 0;
		RGBForeColor(&color);
		macRect = bounds;
		PaintRect(&macRect);
 
		// green
		color.red = 0; color.green = 0xffff; color.blue = 0;
		RGBForeColor(&color);
		MacInsetRect(&macRect, 20, 20);
		PaintRect(&macRect);
 
		// blue. just for kicks lets use GDI to render graphics into the same GWorld
		MacInsetRect(&macRect, 20, 20);
		winRect.top = macRect.top;
		winRect.left = macRect.left;
		winRect.bottom = macRect.bottom;
		winRect.right = macRect.right;
		hBrush = CreateSolidBrush(RGB(0,0,0xff));
		hBrushOld = SelectObject(hdcSrc, hBrush);
		FillRect(hdcSrc, &winRect, hBrush);
		GdiFlush();
		DeleteObject(SelectObject(hdcSrc, hBrushOld));
	}
    
	// copy contents of GWorld to dstHDC.
	StretchBlt(hdcDest,rectDest->left,rectDest->top,rectDest->right-rectDest->left,
		rectDest->bottom-rectDest->top,hdcSrc,bounds.left,bounds.top,
		bounds.right-bounds.left,bounds.bottom-bounds.top,SRCCOPY);
	
	// reset port
	SetGWorld(savedPort,savedGD);
   
bail:
	// dispose of gworld
	if( gw ) DisposeGWorld(gw);
	
	return result;
}


/************************************************************
*                                                           *
*    FUNCTION:  QuickTimeInit                               *
*                                                           *
*    PURPOSE:   Contains QuickTime initialization code      *
*                                                           *
*************************************************************/

static void QuickTimeInit (void)
{
	OSErr err;

	err = InitializeQTML(0L);
	CheckError (err, "InitializeQTML error" );

	err = EnterMovies ();
	CheckError (err, "EnterMovies error" );

}


/************************************************************
*                                                           *
*    FUNCTION:  Utils_Win32_DisplayMsg                      *
*                                                           *
*    PURPOSE:   Displays error messages for Win95/NT sample *
*               code                                        *
*                                                           *
*************************************************************/

static void Utils_DisplayMsg(char *msg)
{
	MessageBox(NULL, msg, "", MB_OK);
}


/************************************************************
*                                                           *
*    FUNCTION:  CheckError                                  *
*                                                           *
*    PURPOSE:   Displays error messages for Win95/NT sample *
*               code                                        *
*                                                           *
*************************************************************/

static void CheckError(OSErr error, char *msg)
{
	if (error == noErr)
	{
		return;
	}
	if (strlen(msg) > 0)
	{
		Utils_DisplayMsg(msg);
	
		ExitToShell();
	}
}
