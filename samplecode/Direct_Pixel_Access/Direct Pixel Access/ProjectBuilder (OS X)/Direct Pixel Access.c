/*
	File:		Direct Pixel Access.c

	Contains:	This snippet shows one example of how to directly			
                        change the pixel values stored in a pixel image.			
                        The original pixel image is obtained from a 'icl8'			
                        resource.  Only the first 20 columns of the first		
                        20 rows of the 'icl8' image is used.	

	Written by: EL	

	Copyright:	Copyright © 1992-1999 by Apple Computer, Inc., All Rights Reserved.

	Disclaimer:	IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc.
				("Apple") in consideration of your agreement to the following terms, and your
				use, installation, modification or redistribution of this Apple software
				constitutes acceptance of these terms.  If you do not agree with these terms,
				please do not use, install, modify or redistribute this Apple software.

				In consideration of your agreement to abide by the following terms, and subject
				to these terms, Apple grants you a personal, non-exclusive license, under Apple’s
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
        6/2003		MK				Updated for Project Builder
        
        08/2000		JM				Carbonized, non-Carbon code is commented out
                                                                for demonstration purposes.
        7/9/1999	KG				Updated for Metrowerks Codewarror Pro 2.1
				

*/
#include "CarbonPrefix.h"

/* Constant Declarations */

#define	WWIDTH	600
#define	WHEIGHT	400

//#define WLEFT	(((qd.screenBits.bounds.right - qd.screenBits.bounds.left) - WWIDTH) / 2)
//#define WTOP	(((qd.screenBits.bounds.bottom - qd.screenBits.bounds.top) - WHEIGHT) / 2)

/* Global Variable Definitions */

WindowPtr		gWindow;
PixMapHandle	gPixmap;

void initMac();

void createWindow();
void createOffscreen();
void drawPixelImageData();
void doEventLoop();

int main(void)
{
	initMac();
	
	createWindow();
	createOffscreen();
	
	doEventLoop();
    
    return 0;
}

void initMac()
{
	//MaxApplZone();

	/*InitGraf( &qd.thePort );
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
	Rect		rect;
	BitMap		bitMap;
	Rect		tempRect1;
	int			top, left;
	
	GetQDGlobalsScreenBits(&bitMap);
	tempRect1 = bitMap.bounds;
	
	left = (((tempRect1.right - tempRect1.left) - WWIDTH) / 2);
	top = (((tempRect1.bottom - tempRect1.top) - WHEIGHT) / 2);

	//SetRect( &rect, WLEFT, WTOP, WLEFT + WWIDTH, WTOP + WHEIGHT );
	SetRect( &rect, left, top, left + WWIDTH, top + WHEIGHT );
	gWindow = NewCWindow( 0L, &rect, "\pDirect Pixel Access", true, documentProc,
							(WindowPtr)-1L, true, 0L );						
	//SetPort( gWindow );
	SetPortWindowPort( gWindow );
	
	TextFont( kFontIDGeneva );
	TextSize( 9 );
	TextMode( srcXor );
}

void createOffscreen()
{
	Rect		rect;
	Handle		iclHandle;
	char		*image;
	int			row, col, index, value;
	
	
	SetRect( &rect, 0, 0, 32, 32 );
	
	/* Create offscreen pixmap image using an 'icl8' icon resource. */

	iclHandle = GetResource( 'icl8', 129 );
    HandToHand( &iclHandle );
    HLock( iclHandle );
	HNoPurge( iclHandle );
	
	//gPixmap = (PixMapHandle)NewHandle( sizeof( PixMap ) );
	gPixmap = NewPixMap();
	
	(**gPixmap).baseAddr = *iclHandle;
	(**gPixmap).rowBytes = ((32 * 8) / 8) | 0x8000;
	(**gPixmap).bounds = rect;
	(**gPixmap).pmVersion = 0;
	(**gPixmap).packType = 0;
	(**gPixmap).packSize = 0;
	(**gPixmap).hRes = 72;
	(**gPixmap).vRes = 72;
	(**gPixmap).pixelSize = 8;
	//(**gPixmap).planeBytes = 0;
	//(**gPixmap).pmReserved = 0;
	(**gPixmap).pixelType = 0;
	(**gPixmap).cmpCount = 1;
	(**gPixmap).cmpSize = 8;
	(**gPixmap).pmTable = GetCTable( 8 );
	(**gPixmap).pixelFormat = 0;
	
	/* Give a unique seed for the pixmap's colortable. */
	(**(**gPixmap).pmTable).ctSeed = GetCTSeed();
	
	SetRect( &rect, 0, 0, 20, 20 );
	
	/* Set the pointer to the beginning of the pixel image. */
	image = GetPixBaseAddr( gPixmap );
	
	/*****************************************************************/
	/* For this example, let's set the pixel values of the left half */
	/*   of the image to half their original values.				 */
	/*****************************************************************/
	
	for (row = 0; row < rect.bottom; row++)
	{
		// Loop through the first 10 columns of the pixel image. 
		for (index = 0, col = 0; col < rect.right / 2; col++)
		{
			// Set the value at this index to half its value. 
			value = (unsigned char)*(image + index);
			*(image + index) = value / 2;
			
			index++;
		}
		
		// Increment the pointer to the next row of the pixel image. 
		image += ((**gPixmap).rowBytes & 0x7fff);
	}

}

void drawPixelImageData()
{
	int				row, col;
	Rect			rect;
	unsigned char	value;
	char			*image;
	int				index = 0;
	Str255			string;
	RGBColor		color = { 32000, 32000, 32000 };
	//Byte			mode;
	Rect			tempRect1;
	
	ForeColor( blackColor );
	
	SetRect( &rect, 0, 0, 20, 20 );
	
	/* For this example, let's just use only the upper left corner of the image. */
	
	// Draw the offscreen image to the screen to see what it looks like.
	//CopyBits( (BitMap *)*gPixmap, &gWindow->portBits, &rect,
	//		&gWindow->portRect, srcCopy, 0 );
	//(**gPixmap).rowBytes ^= 0x8000;
	CopyBits( (BitMap *)*gPixmap, GetPortBitMapForCopyBits(GetWindowPort(gWindow)), &rect,
			GetPortBounds(GetWindowPort(gWindow), &tempRect1), srcCopy, 0 );
	//(**gPixmap).rowBytes ^= 0x8000;
	
	RGBForeColor( &color );
	
	// Again, set the pointer to the beginning of the pixel image.
	image = GetPixBaseAddr( gPixmap );
	
	/***************************************************************/
	/* Finally let's display the pixel values on top of the image. */
	/***************************************************************/
	
	/* Loop through the first 20 rows of the pixel image. */
	for (row = 0; row < rect.bottom; row++)
	{
		// Loop through the first 20 columns of the pixel image. 
		for (index = 0, col = 0; col < rect.right; col++)
		{
			// Get the value at this index into the pixel image. 
			value = (unsigned char)*(image + index);
			
			MoveTo( col * 30, row * 20 );
			LineTo( col * 30, (row + 1) * 20 );
			LineTo( (col + 1) * 30, (row + 1) * 20 );
			
			MoveTo( (col * 30) + 6, (row * 20) + 14 );
			NumToString( (long)value, string );
			DrawString( string );
			
			index++;
		}
		
		// Increment the pointer to the next row of the pixel image. 
		image += ((**gPixmap).rowBytes & 0x7fff);
	}
}

void doEventLoop()
{
	EventRecord event;
	WindowPtr   window;
	short       clickArea;
	Rect        screenRect;
	RgnHandle	rgnHandle = NewRgn();

	for (;;)
	{
		if (WaitNextEvent( everyEvent, &event, 0, nil ))
		{
			if (event.what == mouseDown)
			{
				clickArea = FindWindow( event.where, &window );
				
				if (clickArea == inDrag)
				{
					//screenRect = (**GetGrayRgn ()).rgnBBox;
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
				drawPixelImageData();
				EndUpdate( window );
				QDFlushPortBuffer(GetWindowPort(window), GetPortVisibleRegion(GetWindowPort(window), rgnHandle));
			}
			else if (event.what == activateEvt) 
			{
				/*if (event.modifiers & activeFlag) {
					window = (WindowPtr)event.message;
					SetPortWindowPort(window);
					drawPixelImageData();
					QDFlushPortBuffer(GetWindowPort(window), GetPortVisibleRegion(GetWindowPort(window), rgnHandle));
				}*/
				/*if (event.modifiers & activeFlag)
					PostEvent(updateEvt, (unsigned long)gWindow);*/
			}
		}
	}
	
	DisposeRgn(rgnHandle);
}