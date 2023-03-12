/*
	File:		SeedCFill.c

	Contains:	This snippet shows how to use the SeedCFill routine			
                to create a mask given a source image. As decribed			
                on page 71 of Inside Mac volume V, this routine				
                computes a destination bitmap image with 1's only			
                in the pixels where paint can not leak from the				
                starting seed point. This is similar to the paint-			
                bucket tool.		

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
        6/2003		MK				Updated for ProjectBuilder
        
        08/2000		JM				Carbonized, non-Carbon code is commented out
                                    for demonstration purposes.
                                    
        7/14/1999	KG				Updated for Metrowerks Codewarror Pro 2.1
				

*/
#include "CarbonPrefix.h"

/* Constant Declarations */

#define	WWIDTH		(176 * 2)
#define	WHEIGHT		(106 * 2)

//#define WLEFT		(((qd.screenBits.bounds.right - qd.screenBits.bounds.left) - WWIDTH) / 2)
//#define WTOP		(((qd.screenBits.bounds.bottom - qd.screenBits.bounds.top) - WHEIGHT) / 2)

/* Global Variable Definitions */

WindowPtr	gWindow;

void initMac();
void createWindow();
void doSeedCFillExample();

void DisposeGrafPort(GrafPtr);

void doEventLoop();

int main(void)
{
	initMac();
	createWindow();
	doEventLoop();
        
        return 0;
}

void initMac()
{
	//MaxApplZone();
	
	//InitGraf( &qd.thePort );
	//InitFonts();
	//InitWindows();
	//InitMenus();
	//TEInit();
	//InitDialogs( nil );
	InitCursor();
	FlushEvents( 0, everyEvent );
}

void createWindow()
{
	Rect			rect;
	PaletteHandle	palette;
	BitMap			bitMap;
	int				left, top;
	
	GetQDGlobalsScreenBits(&bitMap);
	left = (((bitMap.bounds.right - bitMap.bounds.left) - WWIDTH) / 2);
	top = (((bitMap.bounds.bottom - bitMap.bounds.top) - WHEIGHT) / 2);
	
	//SetRect( &rect, WLEFT, WTOP, WLEFT + WWIDTH, WTOP + WHEIGHT );
	SetRect( &rect, left, top, left + WWIDTH, top + WHEIGHT );
	
	gWindow = NewCWindow( 0L, &rect, "\pClick anywhere in left image.", true, documentProc,
							(WindowPtr)-1L, true, 0L );
							
	//SetPort( gWindow );
	SetPortWindowPort( gWindow );

	/****************************************************************************/	
	/* Attach a palette of the default colortable to the window.				*/
	/*  WARNING: SeedCFill may not work if the current gDevice's colortable is 	*/
	/*	different than that of the offscreen's colortable.  This offscreen is 	*/
	/*	the one passed into SeedCFill.  To eliminate this problem, we attach a	*/
	/*	palette contaning the same colors used by the offscreen to the window.	*/
	/****************************************************************************/
	
	palette = NewPalette( 256, GetCTable( 8 ), pmTolerant, 0 );
	SetPalette( gWindow, palette, true );
}

void doSeedCFillExample( point )
Point	point;
{
	PicHandle		pict;				/* Pict used for the source. */
	GWorldPtr		source;				/* Gworld used to store the pict. */
	PixMapHandle	sourcePixMap;		/* Handle to the source Gworld. */
	//GrafPtr			mask;			/* BitMap used for creating the mask. */
	int				seedH, seedV;		/* Pixel position used to determine the mask. */
	Rect			rect;				/* Bounding rect of mask and source. */
	CGrafPtr		currentPort;		/* Saved CGrafPtr for later restore. */
	GDHandle		currentDevice;		/* Saved device for later restore. */
	Rect			tempRect1;
	GrafPtr			oldPort;
	GWorldPtr		newMask;			// new mask, use a GWorld as opposed to a new port
	
	GetPort(&oldPort);
	SetPortWindowPort( gWindow );
	
	/* Load the pict resource to be used for the source. */
	pict = (PicHandle)GetResource( 'PICT', 128 );

	/* Define the bounding rect for the source and mask. */
	rect = (**pict).picFrame;
	OffsetRect( &rect, -rect.left, -rect.top );
	rect.right *= 2;
	rect.bottom *= 2;
	
	/* Draw the source image in the window to see what the mask will be created from. */
	DrawPicture( pict, &rect );
	
	/* Return if mouse click was not within the source rect. */
	if (point.h < rect.left || point.h > rect.right ||
		point.v < rect.top || point.v > rect.bottom)
			return;

	/* Create a gworld to store the pict. */
	NewGWorld( &source, 8, &rect, GetCTable( 8 ), nil, 0 );
	LockPixels( GetGWorldPixMap( source ) );
	sourcePixMap = GetGWorldPixMap( source );
	
	/* Draw the pict into the gworld. */
	GetGWorld( &currentPort, &currentDevice );
	SetGWorld( source, nil );
	DrawPicture( pict, &rect );
	
		
	/* Allocate a bitmap for the mask. */
	NewGWorld( &newMask, 1, &(**sourcePixMap).bounds, nil, nil, 0 );
	SetGWorld( newMask, nil);
	EraseRect( &(**sourcePixMap).bounds );

	SetGWorld( currentPort, currentDevice );
	/* Use the mouse click as the starting seed position. */
	seedH = point.h;
	seedV = point.v;
	
	/* Create a mask from the source, starting from the seed point. */
	/*SeedCFill( (BitMap *)*sourcePixMap, &(*mask).portBits, &(**sourcePixMap).bounds,
				&(*mask).portRect, seedH, seedV, nil, 0 );*/
	LockPixels(GetGWorldPixMap(newMask));
	SeedCFill( (BitMap *)*sourcePixMap, (BitMap *) *(GetGWorldPixMap(newMask)), &(**sourcePixMap).bounds,
				GetPortBounds(newMask, &tempRect1), seedH, seedV, nil, 0 );
		
	/* Draw a red cross where the mouse was last clicked. */
	ForeColor( redColor );
	MoveTo( rect.left + seedH - 2, seedV );
	LineTo( rect.left + seedH + 2, seedV );
	MoveTo( rect.left + seedH, seedV - 2 );
	LineTo( rect.left + seedH, seedV + 2 );
	ForeColor( blackColor );
	BackColor( whiteColor );
	
	OffsetRect( &rect, (**sourcePixMap).bounds.right, 0 );
	EraseRect( &rect );
	
	/* Copy the source to the window while applying the mask. */
	/*CopyMask( (BitMap *)*sourcePixMap, &((GrafPtr)mask)->portBits, &gWindow->portBits,
				&(**sourcePixMap).bounds, &((GrafPtr)mask)->portRect, &rect );*/
	CopyMask( (BitMap *)*sourcePixMap, (BitMap *) *(GetGWorldPixMap(newMask)), GetPortBitMapForCopyBits(GetWindowPort(gWindow)),
				&(**sourcePixMap).bounds, GetPortBounds(newMask, &tempRect1), &rect );
	
	/* Release the used memory. */
	//DisposeGrafPort( (GrafPtr)mask );
	UnlockPixels(GetGWorldPixMap(newMask));
	UnlockPixels(GetGWorldPixMap(source));
	DisposeGWorld( newMask );
	DisposeGWorld( source );
	HPurge( (Handle)pict );
	SetPort(oldPort); //restore port
}

void DisposeGrafPort( GrafPtr doomedPort )	/* DisposeGrafPort originally written by Forrest Tanaka. */
{
	//ClosePort( doomedPort );
	//DisposePtr( doomedPort->portBits.baseAddr );
	DisposePtr( GetPortBitMapForCopyBits(doomedPort)->baseAddr );
	//DisposePtr( (Ptr)doomedPort );
	DisposePort( doomedPort );
}

void doEventLoop()
{
	EventRecord		event;
	WindowPtr		window;
	short			clickArea;
	Rect      		screenRect;
	static Point	point = { -1, -1 };
	
        
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
					else
					{
						point = event.where;
						GlobalToLocal( &point );
						doSeedCFillExample( point );
					}
				}
				else if (clickArea == inGoAway)
                                {
					if (TrackGoAway( window, event.where ))
                                        {
						return;
                                        }
                                }
			}
			else if (event.what == updateEvt)
			{
				window = (WindowPtr)event.message;	
				//SetPort( window );
				
				BeginUpdate( window );
				doSeedCFillExample( point );
				EndUpdate( window );
			}
		}
	}
}