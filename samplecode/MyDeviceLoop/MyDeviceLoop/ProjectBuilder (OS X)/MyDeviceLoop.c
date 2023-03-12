/*
	File:		MyDeviceLoop.c

	Contains:	This snippet shows how to write a device loop that			
				works under System 7 and pre-7.0 systems.  As				
				described on pages 21-23 and 21-24 of Inside Mac			
				volume VI, a device loop procedure searches all				
				active screen devices, calling a drawing procedure			
				whenever it encounters a screen that intersects			
				the drawing region.  In this app the drawing				
				region is the app's window bounds and the chosen			
				drawing procedure simply displays the screen's				
				colors for every device the window bounds intersect.		

	Written by: EL	

	Copyright:	Copyright © 1992-1999 by Apple Computer, Inc., All Rights Reserved.

	Disclaimer:	IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc.
				("Apple") in consideration of your agreement to the following terms, and your
				use, installation, modification or redistribution of this Apple software
				constitutes acceptance of these terms.  If you do not agree with these terms,
				please do not use, install, modify or redistribute this Apple software.

				In consideration of your agreement to abide by the following terms, and subject
				to these terms, Apple grants you a personal, non-exclusive license, under AppleÕs
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
        6/2003		MK				Updated for ProjectBuilder
        
        08/2000		JM				Carbonized, non-Carbon code is commented out
                                                        for demonstration purposes.
                                                        
        7/12/1999	KG				Updated for Metrowerks Codewarror Pro 2.1
				

*/
#include "CarbonPrefix.h"

/* Constant Declarations */

#define	WWIDTH		400
#define	WHEIGHT		256

//#define WLEFT		(((qd.screenBits.bounds.right - qd.screenBits.bounds.left) - WWIDTH) / 2)
//#define WTOP		(((qd.screenBits.bounds.bottom - qd.screenBits.bounds.top) - WHEIGHT) / 2)


enum PointSelector {topLeft,
			botRight};
/* Global Variable Definitions */

WindowPtr	gWindow;

void initMac();
void createWindow();
void doMyDeviceLoop();
void doDraw();
void doEventLoop();

int main()
{
	initMac();
	
	createWindow();

	doEventLoop();
        
        return 0;
}

void initMac()
{
	/*MaxApplZone();

	InitGraf( &qd.thePort );
	InitFonts();
	InitWindows();
	InitMenus();
	TEInit();
	InitDialogs( nil );*/
	InitCursor();
	FlushEvents( 0, everyEvent );
}

void createWindow()
{
	Rect rect;
	
	BitMap	bitMap;
	int	top, left;
	
	GetQDGlobalsScreenBits(&bitMap);
	
	top = (((bitMap.bounds.bottom - bitMap.bounds.top) - WHEIGHT) / 2);
	left = (((bitMap.bounds.right - bitMap.bounds.left) - WWIDTH) / 2);
	
	//SetRect( &rect, WLEFT, WTOP, WLEFT + WWIDTH, WTOP + WHEIGHT );
	SetRect( &rect, left, top, left + WWIDTH, top + WHEIGHT );
	
	gWindow = NewCWindow( 0L, &rect, "\pMyDeviceLoop", true, documentProc,
							(WindowPtr)-1L, true, 0L );						
	//SetPort( gWindow );
	SetPortWindowPort( gWindow );
	
	TextFont( kFontIDTimes );
	TextSize( 48 );
	TextMode( srcCopy );
}


// Old version of doMyDeviceLoop, new version capable of supporting multiple monitors
/*
void doMyDeviceLoop()
{
	int			depth;
	Rect		gDeviceRect;
	Rect		intersectingRect;
	GDHandle	gDevice;
	//Point		point;
	Rect		tempRect1;
	
	// Get the handle to the first device in the list. 
	gDevice = GetDeviceList();
	
	// Loop through all the devices in the list. 
	while (gDevice != nil)
	{
		// Get the device's gdRect and convert it to local coordinates. 
		gDeviceRect = (**gDevice).gdRect;
		depth = (**(**gDevice).gdPMap).pixelSize;
			
		GlobalToLocal( topLeft );
		GlobalToLocal( (Point *)botRight );
		
		// Check if the app's window rect intersects the device's, and if it 
		//	does, set the clip region's rect to the intersection, then DRAW! 
		
		//if (SectRect( &gWindow->portRect, &gDeviceRect, &intersectingRect ))
		if (SectRect( GetPortBounds(GetWindowPort(gWindow), &tempRect1), &gDeviceRect, &intersectingRect ))
		{
			ClipRect( &intersectingRect );
			doDraw( depth, &intersectingRect );
		}
		
		// Get the next device in the list. 
		gDevice = GetNextDevice( gDevice );
	}
}
*/

// New doMyDeviceLoop, now works with multiple monitors
void doMyDeviceLoop()
{
	int				depth;
	Rect			gDeviceRect;
	Rect			intersectingRect;
	GDHandle		gDevice;
	//WindowRecord	*windowRec = (WindowRecord *)gWindow;
	WindowPtr		windowRec = gWindow;
	Rect			windowRect; //= (**windowRec->contRgn).rgnBBox;
	RgnHandle		rgnHandle = NewRgn();
	
	GetWindowRegion(windowRec, kWindowContentRgn, rgnHandle);
	GetRegionBounds(rgnHandle, &windowRect);

	// Get the handle to the first device in the list. 
	gDevice = GetDeviceList();

	// Loop through all the devices in the list. 
	while (gDevice != nil)
	{
		// Get the device's gdRect  */
		gDeviceRect = (**gDevice).gdRect;
		depth = (**(**gDevice).gdPMap).pixelSize;

		// Check if the app's window rect intersects the device's, and if it 
		// does, set the clip region's rect to the intersection, then DRAW! 
		if (SectRect( &windowRect, &gDeviceRect, &intersectingRect ))
		{
			// The intersectingRect is in global coords. Convert to local 
			GlobalToLocal((Point *)&intersectingRect.top);
			GlobalToLocal((Point *)&intersectingRect.bottom);

			ClipRect( &intersectingRect );
			doDraw( depth, &intersectingRect );
		}

		// Get the next device in the list. 
		gDevice = GetNextDevice( gDevice );
	}
	
	DisposeRgn(rgnHandle);
}


void doDraw( depth)
int		depth;
{
	int				i;
	int				totalColors;
	int				penThickness;
	//RGBColor		color;
	CTabHandle		ctable;
	Str255			string = "\pDirect Colors";
	Rect			tempRect1;
	
	if (depth > 8)
	{
		BackColor( blackColor );
		ForeColor( blueColor );
		
		/* Draw text for direct colors mode. */
		//EraseRect( &gWindow->portRect );
		EraseRect( GetPortBounds(GetWindowPort(gWindow), &tempRect1));
		//MoveTo( (gWindow->portRect.right - StringWidth( string )) / 2, 130 );
		MoveTo((GetPortBounds(GetWindowPort(gWindow), &tempRect1)->right - StringWidth(string)) / 2, 130);
		DrawString( string );
	}
	else
	{
		/* Get the colortable at this depth. */
		ctable = GetCTable( depth );
		
		/* Set the line thickness to a fraction of the window height. */
		totalColors = (2 << depth) / 2;
		//penThickness = gWindow->portRect.bottom / totalColors;
		penThickness = GetPortBounds(GetWindowPort(gWindow), &tempRect1)->bottom / totalColors;
		PenSize( 1, penThickness );
	
		/* Now draw the colors at this depth. */
		for (i = 0; i < totalColors; i++)
		{
			RGBForeColor( &(**ctable).ctTable[i].rgb );
			MoveTo( 0, i * penThickness );
			//LineTo( gWindow->portRect.right, i * penThickness );
			LineTo(GetPortBounds(GetWindowPort(gWindow), &tempRect1)->right, i * penThickness);
		}
		
		/* Release the colortable memory. */
		DisposeCTable( ctable );
	}
}

void doEventLoop()
{
	EventRecord event;
	WindowPtr   window;
	short       clickArea;
	Rect        screenRect;

	for (;;)
	{
		if (WaitNextEvent( everyEvent, &event, 0, nil ))
		{
			if (event.what == mouseDown)
			{
				clickArea = FindWindow( event.where, &window );
				
				if (clickArea == inDrag)
				{
					//screenRect = (**GetGrayRgn()).rgnBBox;
					GetRegionBounds(GetGrayRgn(), &screenRect);
					DragWindow( window, event.where, &screenRect );
				}
				else if (clickArea == inContent)
				{
					if (window != FrontWindow())
						SelectWindow( window );
				}
				else if (clickArea == inGoAway)
					if (TrackGoAway( window, event.where ))
						return;
			}
			else if (event.what == updateEvt)
			{
				window = (WindowPtr)event.message;	
				//SetPort( window );
				SetPortWindowPort( window );
				
				BeginUpdate( window );
				doMyDeviceLoop();
				EndUpdate( window );
			}
		}
	}
}