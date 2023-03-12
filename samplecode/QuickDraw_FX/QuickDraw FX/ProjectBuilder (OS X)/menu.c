/*
	File:		menu.c

	Contains:	

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
        7/13/1999	KG				Updated for Metrowerks Codewarror Pro 2.1
				

*/

#include "FX.h"

enum {
	appleID = 128,
	fileID = 129,
	exID = 130,
	srcID = 131
};

enum {
	quitItem = 1
};

#define	WWIDTH		420
#define	WHEIGHT		200

void setUpMenus()
{
	Handle		menuHandle;
	
	if ((menuHandle = GetNewMBar( 128 )) == nil)
		ExitToShell();
		
	SetMenuBar( menuHandle );
	AppendResMenu( GetMenu( appleID ), 'DRVR' );
		
	DrawMenuBar();
	ReleaseResource( menuHandle );
}

void adjustMenus()
{
	int		i;

	//EnableItem( GetMenuHandle( fileID ), quitItem );
	EnableMenuItem( GetMenuHandle( fileID ), quitItem );
	
	for (i = 1; i <= pixelAverageID; i++)
		setMenuItem( GetMenuHandle( exID ), i, (i == gCurrentExample / 10) );
	
	setMenuItem( GetMenuHandle( exID ), customID + 1, (customID == gCurrentExample / 10) );
}

void setMenuItem( MenuHandle menu, int itemNum, Boolean enabled )
{
	/*if (enabled)
		//CheckItem( menu, itemNum, true );
		CheckMenuItem( menu, itemNum, true );
	else
		//CheckItem( menu, itemNum, false );
		CheckMenuItem( menu, itemNum, false );*/
	CheckMenuItem( menu, itemNum, enabled );
}

void handleMenu( long mSelect )

{
	int			menuID = HiWord( mSelect );
	int			menuItem = LoWord( mSelect );
	//GrafPtr		savePort;
	//Str255		name;
	long		ticks;

	switch (menuID)
	{
		case appleID:
			if (menuItem == 1)
				doAboutBox();
			else
			{
				/*GetPort( &savePort );
				GetMenuItemText( GetMenu( appleID ), menuItem, name );
				OpenDeskAcc( name );
				SetPort( savePort );*/
			}
			break;
			
		case fileID:
			if (menuItem)
				ExitToShell();
			break;
			
		case exID:
			if (menuItem == 10)
				menuItem--;

			if (menuItem == gCurrentExample / 10)
				break;
			
			if (menuItem == 9)
				resetItems();
				
			gCurrentExample = (menuItem * 10) + 1;
			drawExampleName();
			drawSourceImage();
			
			ticks = drawFXImage();
			drawTime( ticks );
			drawAllItems();
			break;
			
		case srcID:
			createOffscreen( menuItem );
			drawSourceImage();
			
			ticks = drawFXImage();
			drawTime( ticks );
			break;
	}
	
	HiliteMenu( 0 );
}

void doAboutBox()
{
	int				col, row;
	int				width, height;
	WindowPtr		window;
	CIconHandle		cicn;
	Rect			rect;
	RGBColor		color;
	BitMap			bitMap;
	int				left, top;

	cicn = GetCIcon( 128 );
	HPurge( (Handle)cicn );
	
	GetQDGlobalsScreenBits(&bitMap);
	left = (((bitMap.bounds.right - bitMap.bounds.left) - WWIDTH) / 2);
	top = (((bitMap.bounds.bottom - bitMap.bounds.top) - WHEIGHT) / 2);

	//SetRect( &rect, WLEFT, WTOP, WLEFT + WWIDTH, WTOP + WHEIGHT );
	SetRect( &rect, left, top, left + WWIDTH, top + WHEIGHT );
	window = NewCWindow( 0L, &rect, "\p", true, plainDBox, (WindowPtr)-1L, false, 0L );							
	//SetPort( window );
	SetPortWindowPort( window );
		
	TextFont( kFontIDGeneva );
	TextMode( srcOr );
	
	color.red = color.green = color.blue = 8700;
	RGBForeColor( &color );
		
	//rect = window->portRect;
	GetPortBounds(GetWindowPort(window), &rect);
	InsetRect( &rect, 1, 1 );
	PaintRect( &rect );
	
	width = 32 * 6;
	height = width;
	
	SetRect( &rect, 3, 3, width + 3, height + 3 );
	PlotCIcon( &rect, cicn );

	ForeColor( blackColor );

	for (row = 6; row < height; row += 6)
	{
		MoveTo( rect.left, rect.top + row );
		LineTo( rect.left + width, rect.top + row );
	}
	
	for (col = 6; col < width; col += 6)
	{
		MoveTo( rect.left + col, rect.top );
		LineTo( rect.left + col, rect.top + height );
	}
	
	col = width + 15;
	row = 35;
	
	TextFont( kFontIDTimes );
	TextSize( 36 );
	
	color.blue = 0xffff;
	color.red = color.green = 0;
	RGBForeColor( &color );	
	drawMyString( col, &row, 30, "\pQuickDraw™" );
	
	color.blue = 0x9fff;
	RGBForeColor( &color );
	drawMyString( col + 45, &row, -5, "\pFX" );
	
	TextFont( kFontIDGeneva );
	TextSize( 9 );
	ForeColor( whiteColor );
	
	col += 10;
	
	drawMyString( col + 115, &row, 25, "\pVersion 1.0" );
	drawMyString( col, &row, 20, "\pBrought to you by Edgar Lee." );
	
	drawMyString( col, &row, 13, "\pFor any suggestions or comments," );
	drawMyString( col, &row, 13, "\pplease write to edgar@apple.com" );
	drawMyString( col, &row, 20, "\por appleLink EDGAR." );
	
	drawMyString( col, &row, 15, "\p© 1992-1999 Apple Computer, Inc." );
	drawMyString( col, &row, 25, "\pAll rights reserved." );
	
	TextFont( kFontIDTimes );
	TextSize( 36 );
	
	color.green = 0x8fff;
	color.red = color.blue = 0;
	RGBForeColor( &color );
	drawMyString( col + 125, &row, 0, "\pDTS" );
	
	while (!Button());
	
	DisposeWindow( window );
}

void drawMyString( int col, int *row, int increment, Str255 string)
{
	RGBColor	color;
	
	GetForeColor( &color );
	
	ForeColor( blackColor );
	MoveTo( col + 2, *row + 2 );
	DrawString( string );
	
	RGBForeColor( &color );
	MoveTo( col, *row );
	DrawString( string );
	
	*row += increment;
}