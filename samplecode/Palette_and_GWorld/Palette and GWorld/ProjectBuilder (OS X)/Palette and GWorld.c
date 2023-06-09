/*
	File:		Palette and GWorld.c

	Contains:	This app copies from two offscreen GWorlds into the left and right halves of
                        the window. The contents of the GWorlds are vertical lines that show all the
                        entries in the color tables associated with the GWorlds. The GWorlds were created
                        as described below. 

                        One commonly asked question is how to use a palette when drawing into a GWorld.
                        The trick is understanding that while setting a palette to a GWorld is permitted,
                        doing so does not change the GWorld�s color table. 

                        The solution is to make a palette from the color table ( or the color table from
                        a palette) and to use that color table to create or update the GWorld. After then
                        doing a SetGWorld you can either draw with Index2Color and RGBForeColor, or you
                        can set the palette to the GWorld and draw with PmForeColor. These techniques are
                        shown in the procedures createRGBForeColorImage and createPmForeColorImage in
                        Palette and GWorld.c. 


	Written by: 	

	Copyright:	Copyright � 1999 by Apple Computer, Inc., All Rights Reserved.

	Disclaimer:	IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc.
				("Apple") in consideration of your agreement to the following terms, and your
				use, installation, modification or redistribution of this Apple software
				constitutes acceptance of these terms.  If you do not agree with these terms,
				please do not use, install, modify or redistribute this Apple software.

				In consideration of your agreement to abide by the following terms, and subject
				to these terms, Apple grants you a personal, non-exclusive license, under Apple�s
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
                                                        
        7/13/1999	KG				Updated for Metrowerks Codewarror Pro 2.1
				

*//*  Includes  */
#include "CarbonPrefix.h"

/*  Defines  */
static 	BitMap	globalBitMap;					//declared here to use in a #define (for carbonization)
#define	TOTALCOLORS	256
#define	WWIDTH		(TOTALCOLORS * 2)
#define	WHALFWIDTH	TOTALCOLORS
#define	WHEIGHT		TOTALCOLORS
//#define WLEFT		(((qd.screenBits.bounds.right - qd.screenBits.bounds.left) - WWIDTH) / 2)
//#define WTOP		(((qd.screenBits.bounds.bottom - qd.screenBits.bounds.top) - WHEIGHT) / 2)
#define WLEFT		(((GetQDGlobalsScreenBits(&globalBitMap)->bounds.right - GetQDGlobalsScreenBits(&globalBitMap)->bounds.left) - WWIDTH) / 2)
#define WTOP		(((GetQDGlobalsScreenBits(&globalBitMap)->bounds.bottom - GetQDGlobalsScreenBits(&globalBitMap)->bounds.top) - WHEIGHT) / 2)

/*  Global Variable Definitions  */
/*#ifdef	powerc
	QDGlobals 			qd;
#endif*/
WindowPtr			gWindow;
PaletteHandle		gPalette;
GWorldPtr			gRGBGWorld;
GWorldPtr			gPmGWorld;
PixMapHandle		gRGBPixMap;
PixMapHandle		gPmPixMap;
CTabHandle			gClut;
Rect				gRGBRect = { 0, 0, WHEIGHT, WHALFWIDTH };
Rect				gPmRect  = { 0, WHALFWIDTH, WHEIGHT, WWIDTH };
short				gMode    = srcOr;
short				gText    = kFontIDTimes;
short				gSize    = 24;

/*  Procedure Prototypes  */
void initMac( void );
void makePalette( void );
void createWindow( void );
void createOffscreens( void );
void createRGBForeColorImage( void );
void createPmForeColorImage( void );
void drawWindow( void );
void doEventLoop( void );


/*             */
/*  Main loop. */
/*             */

int main()
{
	initMac();
	
	makePalette();
	createWindow();	
	createOffscreens();
	createRGBForeColorImage();
	createPmForeColorImage();

	doEventLoop();
        
        return 0;
}


/*                      */
/*  Mac initialization. */
/*                      */

void initMac( void )
{
	/*MaxApplZone();
	MoreMasters();
	MoreMasters();
	InitGraf( &qd.thePort );
	InitFonts();
	InitWindows();
	InitMenus();
	TEInit();
	InitDialogs( (long)nil );*/
	MoreMasterPointers(2);
	InitCursor();
	FlushEvents( 0, everyEvent );
}


/*                                    */
/*  Create the palette from the clut. */
/*                                    */

void makePalette( void )
{	
	short	plttID = 128;
	
	/*  Get the color table from the .rsrc file. */
	gClut = GetCTable( plttID );
	
	/*  Create the palette from the color table.                                                  */
	/*  pmExplicit says we want the colors in the same order as the color table.                  */
	/*  pmTolerant with a tolerance of 0 says we want exactly the same colors as the color table. */
	gPalette = NewPalette( TOTALCOLORS, gClut, pmExplicit + pmTolerant, 0 );
}


/*                                                                     */
/*  Create the window to display the images and set the palette to it. */
/*                                                                     */

void createWindow( void )
{
	Rect	bounds;
	
	SetRect( &bounds, WLEFT, WTOP, WLEFT + WWIDTH, WTOP + WHEIGHT );
	
	gWindow = NewCWindow( 0L, &bounds, "\pPalette&GWorld", true, documentProc,
							(WindowPtr)-1L, true, 0L );						
	//SetPort( gWindow );
	SetPortWindowPort( gWindow );
	
	/*  Attach the palette to the window. */
	NSetPalette( gWindow, gPalette, pmAllUpdates );
}


/*                                                           */
/*  Create the GWorlds based on the clut to hold the images. */
/*                                                           */

void createOffscreens( void )
{
	GDHandle	screensDevice;

	short		depth = 8;

	/*  Set the color table's ctSeed equal to that of the main screen's GDevice. */ 
	/*  This tells QuickDraw not to do color mapping.                            */ 
	/*  Color mapping is not needed because the palette matches the color table. */ 
	screensDevice = GetMainDevice();
	if (screensDevice != nil)
		(**gClut).ctSeed = (**(**(**screensDevice).gdPMap).pmTable).ctSeed;
		
	/*  Create the RGBForeColor offscreen world using the colortable. */
	NewGWorld( &gRGBGWorld, depth, &gRGBRect, gClut, nil, 0 );
	gRGBPixMap = GetGWorldPixMap( gRGBGWorld );	

	/*  Create the PmForeColor offscreen world using the colortable. */
	NewGWorld( &gPmGWorld, depth, &gPmRect, gClut, nil, 0 );
	gPmPixMap = GetGWorldPixMap( gPmGWorld );	
}


/*                                                              */
/*  Create the GWorld image using Index2Color and RGBForeColor. */
/*                                                              */

void createRGBForeColorImage( void )
{
	int			i;
	CGrafPtr	currentPort;
	GDHandle	currentDevice;
	RGBColor	color;
	FontInfo	info;
	Str255		string = "\pRGBForeColor";
	
	GetGWorld( &currentPort, &currentDevice );
	
	SetGWorld( gRGBGWorld, nil );
	PenSize ( 1, 1 );
	LockPixels( gRGBPixMap );
	
	/*  Show all the colors in the background using Index2Color and RGBForeColor. */
	for (i = 0; i < TOTALCOLORS; i++){
		Index2Color( i, &color );
		RGBForeColor( &color );
		MoveTo( i, 0 );
		Line( 0, WHEIGHT );
	}
	
	/*  Draw label; RGBForeColor should now be black. */
	TextMode( gMode );
	TextFont( gText );
	TextSize( gSize );		
    GetFontInfo( &info );
	MoveTo( (WHALFWIDTH - StringWidth(string))/2, WHEIGHT/2 - info.ascent/3 );
	DrawString( string );
	
	UnlockPixels( gRGBPixMap );
	SetGWorld( currentPort, currentDevice );
}


/*                                                                                      */
/*  Create the GWorld image by setting the palette to the GWorld and using PmForeColor. */
/*                                                                                      */

void createPmForeColorImage( void )
{
	int			i;
	CGrafPtr	currentPort;
	GDHandle	currentDevice;
	FontInfo	info;
	Str255		string = "\pPmForeColor";
	
	GetGWorld( &currentPort, &currentDevice );
	
	SetGWorld( gPmGWorld, nil );
	NSetPalette( (WindowPtr)gPmGWorld, gPalette, pmNoUpdates );
	PenSize ( 1, 1 );
	LockPixels( gPmPixMap );
	
	/*  Show all the colors in the background using PmForeColor. */
	for (i = 0; i < TOTALCOLORS; i++){
		PmForeColor( i );
		MoveTo( (i + WHALFWIDTH), 0 );
		Line( 0, WHEIGHT );
	}
					
	/*  Draw label; PmForeColor should now be black. */
	TextMode( gMode );
	TextFont( gText );
	TextSize( gSize );		
    GetFontInfo( &info );
	MoveTo( ((WHALFWIDTH - StringWidth(string))/2) + WHALFWIDTH, WHEIGHT/2 - info.ascent/3 );
	DrawString( string );
	
	UnlockPixels( gPmPixMap );
	SetGWorld( currentPort, currentDevice );
}


/*                                      */
/*  CopyBits both images to the screen. */
/*                                      */

void drawWindow( void )
{	
	/*CopyBits( (BitMap*)*gRGBPixMap, &gWindow->portBits, &(**gRGBPixMap).bounds,
				&gRGBRect, srcCopy, 0l );
				
	CopyBits( (BitMap*)*gPmPixMap, &gWindow->portBits, &(**gPmPixMap).bounds,
				&gPmRect, srcCopy, 0l );*/
	CopyBits( (BitMap*)*gRGBPixMap, GetPortBitMapForCopyBits(GetWindowPort(gWindow)), &(**gRGBPixMap).bounds,
				&gRGBRect, srcCopy, 0l );
				
	CopyBits( (BitMap*)*gPmPixMap, GetPortBitMapForCopyBits(GetWindowPort(gWindow)), &(**gPmPixMap).bounds,
				&gPmRect, srcCopy, 0l );
}


/*              */
/*  Event loop. */
/*              */

void doEventLoop( void )
{
	EventRecord event;
	WindowPtr   window;
	short       clickArea;
	Rect        screenRect;

	for (;;){
		if (WaitNextEvent( everyEvent, &event, 0, nil )){
			if (event.what == mouseDown){
				clickArea = FindWindow( event.where, &window );
				
				if (clickArea == inDrag){
					//screenRect = (**GetGrayRgn()).rgnBBox;
					GetRegionBounds(GetGrayRgn(), &screenRect);
					DragWindow( window, event.where, &screenRect );
				}
				else if (clickArea == inContent){
					if (window != FrontWindow())
						SelectWindow( window );
				}
				else if (clickArea == inGoAway)
					if (TrackGoAway( window, event.where ))
						return;
			}
			else if (event.what == updateEvt){
				window = (WindowPtr)event.message;	
				//SetPort( window );	
				SetPortWindowPort( window );			
				BeginUpdate( window );
				drawWindow();
				EndUpdate( window );
			}
		}
	}
}