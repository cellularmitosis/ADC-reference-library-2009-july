/*
	File:		main.c

	Contains:	

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
        6/2003		MK				Updated for Project Builder
        08/2000		JM				Carbonized, non-Carbon code is commented out
                                                        for demonstration purposes.
        7/13/1999	KG				Updated for Metrowerks Codewarror Pro 2.1
				

*/
#include "FX.h"

/* Global Variable Definitions */

WindowPtr		gWindow;
GWorldPtr		gGWorld = nil;
int				gCurrentExample = 11;

itemType	tItem[numTItems];
itemType	aItem[numAItems];
itemType	cItem[numCItems];
itemType	dItem[numDItems];
itemType	mItem[numMItems];
itemType	bItem[numBItems];
itemType	pItem[numPItems];
itemType	lItem[numLItems];

itemSettings	settings;

void main()
{
	initMac();
	
	createOffscreen( 1 );
	createWindow();

	resetItems();
	defineItems();

	eventLoop();
}

void initMac()
{
	InitCursor();
	FlushEvents( 0, everyEvent );
	
	setUpMenus();
}

void createOffscreen(int pictItem)
{
	PicHandle	pict;
	Rect		rect;
	CGrafPtr	currentPort;
	GDHandle	currentDevice;
	
	if (gGWorld != nil)
		DisposeGWorld( gGWorld );
	
	pict = (PicHandle)GetResource( 'PICT', pictItem + 127 );
	
	rect = (**pict).picFrame;
	
	GetGWorld( &currentPort, &currentDevice );
	NewGWorld( &gGWorld, 32, &rect, nil, nil, 0 );
		
	LockPixels( GetPortPixMap(gGWorld));
	SetGWorld( gGWorld, nil );

	DrawPicture( pict, &rect );
	
	SetGWorld( currentPort, currentDevice );
	
	ReleaseResource( (Handle)pict );
}

void createWindow()
{
	int		width, height;
	int		left, top;
	Rect	rect;
	Rect	tempRect1;
	BitMap	bitMap;
	
	GetPortBounds(gGWorld, &tempRect1);
	
	width = 58 + (tempRect1.right - tempRect1.left) * 2;
	height = 230 + tempRect1.bottom - tempRect1.top;
	
	
	tempRect1 = GetQDGlobalsScreenBits(&bitMap)->bounds;
	
	left = (((tempRect1.right - tempRect1.left) - width) / 2);
	top = (((tempRect1.bottom - tempRect1.top) - height) / 2) + 20;

	SetRect( &rect, left, top, left + width, top + height );
	gWindow = NewCWindow( 0L, &rect, "\pQuickDraw Special Effects", true, noGrowDocProc,
							(WindowPtr)-1L, true, 0 );
							
	SetPortWindowPort( gWindow );
}

void resetItems()
{
	settings.tItem = 1;
	settings.aItem = 1;
	settings.cItem = 1;
	settings.mItem = 1;
	settings.dItem = 1;
	settings.pItem = 1;
	settings.lItem = 1;
}

void defineItems()
{
	int		i;
	int		col, row;
	
	char	tNames[numTItems][15] = {	"srcCopy", "srcOr", "srcXor", "srcBic",
										"notSrcCopy", "notSrcOr", "notSrcXor", "notSrcBic"	};
	char	aNames[numAItems][15] = {	"blend", "addPin", "addOver", "subPin",
										"adMax", "subMin", "adMin", "transparent"	};
	char	cNames[numCItems][15] = {	"None", "Inverse", "FG Only", "BG Only"	};
	char	dNames[numDItems][15] = {	"Dithering OFF", "Dithering ON"	};
	char	mNames[numMItems][15] = {	"None", "Search Proc", "ColorTable"	};
	char	bNames[numBItems][15] = {	"White", "Black", "White&Black"	};
	char	pNames[numPItems][15] = {	"SeedCFill", "SeedFill"	};
	char	lNames[numLItems][15] = {	"CalcCMask", "CalcMask"	};
	Rect	tempRect1;
	
	settings.bItem = 1;
	
	col = 15;
	row = GetPortBounds(GetWindowPort(gWindow), &tempRect1)->bottom - 157;
	
	for (i = 0; i < numTItems; i++)
	{
		strcpy(P2CStr(tItem[i].label),tNames[i]);
		C2PStr((char *)tItem[i].label);
		SetRect( &tItem[i].rect, col + 40, row + (i * 15),
					col + 40 + 90, row + ((i + 1) * 15) );
	}
	
	for (i = 0; i < numAItems; i++)
	{
		strcpy(P2CStr(aItem[i].label),aNames[i]);
		C2PStr((char *)aItem[i].label);
		SetRect( &aItem[i].rect, col + 140, row + (i * 15),
					col + 140 + 90, row + ((i + 1) * 15) );
	}
	
	for (i = 0; i < numCItems; i++)
	{
		strcpy(P2CStr(cItem[i].label),cNames[i]);
		C2PStr((char *)cItem[i].label);
		SetRect( &cItem[i].rect, col + 240, row + (i * 15),
					col + 240 + 90, row + ((i + 1) * 15) );
	}
	
	for (i = 0; i < numDItems; i++)
	{
		strcpy(P2CStr(dItem[i].label),dNames[i]);
		C2PStr((char *)dItem[i].label);
		SetRect( &dItem[i].rect, col + 240, row + 90 + (i * 15),
					col + 240 + 90, row + 90 + ((i + 1) * 15) );
	}
		
	for (i = 0; i < numMItems; i++)
	{
		strcpy(P2CStr(mItem[i].label),mNames[i]);
		C2PStr((char *)mItem[i].label);
		SetRect( &mItem[i].rect, col + 340, row + (i * 15),
					col + 340 + 90, row + ((i + 1) * 15) );
	}
	
	for (i = 0; i < numBItems; i++)
	{
		strcpy(P2CStr(bItem[i].label),bNames[i]);
		C2PStr((char *)bItem[i].label);
		SetRect( &bItem[i].rect, col + 340, row + 75 + (i * 15),
					col + 340 + 90, row + 75 + ((i + 1) * 15) );
	}
	
	for (i = 0; i < numPItems; i++)
	{
		strcpy(P2CStr(pItem[i].label),pNames[i]);
		C2PStr((char *)pItem[i].label);
		SetRect( &pItem[i].rect, col + 440, row + (i * 15),
					col + 440 + 90, row + ((i + 1) * 15) );
	}
	
	for (i = 0; i < numLItems; i++)
	{
		strcpy(P2CStr(lItem[i].label),lNames[i]);
		C2PStr((char *)lItem[i].label);
		SetRect( &lItem[i].rect, col + 440, row + 60 + (i * 15),
					col + 440 + 90, row + 60 + ((i + 1) * 15) );
	}
}

void updateWindow()
{
	long	ticks;
	
	drawBackground();
	drawAllItems();
	drawExampleName();
	drawSourceImage();
	
	ticks = drawFXImage();
	drawTime( ticks );
}

void drawBackground()
{
	Rect		rect;
	RGBColor	color;
	Rect		tempRect1;
	
	color.red = color.green = color.blue = 8700;
	
	RGBForeColor( &color );
	PaintRect( GetPortBounds(GetWindowPort(gWindow), &tempRect1));
	
	TextFont( kFontIDTimes );
	TextMode( srcOr );
	TextSize( 24 );
	
	drawName( 85, 22, "\pSource Image" );
	drawName( GetPortBounds(GetWindowPort(gWindow), &tempRect1)->right - 215, 22, "\pNew Image" );
	
	GetPortBounds(GetWindowPort(gWindow), &tempRect1);
	SetRect( &rect, 15, tempRect1.bottom - 180, tempRect1.right - 15, tempRect1.bottom - 30);
	drawDeepBox( &rect );
	
	TextSize( 12 );
	
	drawName( tItem[0].rect.left, tItem[0].rect.top - 8, "\pTransfer Mode" );
	drawName( aItem[0].rect.left, aItem[0].rect.top - 8, "\pArithmetic Mode" );
	drawName( cItem[0].rect.left, cItem[0].rect.top - 8, "\pColorization" );
	drawName( dItem[0].rect.left, dItem[0].rect.top - 8, "\pDither" );
	drawName( mItem[0].rect.left, mItem[0].rect.top - 8, "\pColor Mapping" );
	drawName( bItem[0].rect.left, bItem[0].rect.top - 8, "\pDestination" );
	drawName( pItem[0].rect.left, pItem[0].rect.top - 8, "\pPaint Bucket" );
	drawName( lItem[0].rect.left, lItem[0].rect.top - 8, "\pLasso Tool" );
}

void drawAllItems()
{
	drawTransferItems();
	drawArithmeticItems();
	drawColorizeItems();
	drawDitherItems();
	drawMappingItems();
	drawDestinationItems();
	drawPaintBucketItems();
	drawLassoToolItems();
}

void drawTransferItems()
{
	int			i;
	Boolean		listEnabled;

	listEnabled = (gCurrentExample / 10 == transferID || gCurrentExample / 10 == customID);
	
	for (i = 0; i < numTItems; i++)
		drawItem( tItem[i].rect.left, tItem[i].rect.top, &tItem[i].label,
					listEnabled, settings.tItem == i + 1 );
}

void drawArithmeticItems()
{
	int			i;
	Boolean		listEnabled;
	
	listEnabled = (gCurrentExample / 10 == arithmeticID);
	
	for (i = 0; i < numAItems; i++)
		drawItem( aItem[i].rect.left, aItem[i].rect.top, &aItem[i].label,
					listEnabled, settings.aItem == i + 1 );
}

void drawColorizeItems()
{
	int			i;
	Boolean		listEnabled;
	
	listEnabled = (gCurrentExample / 10 == colorizationID || gCurrentExample / 10 == customID);
		
	for (i = 0; i < numCItems; i++)
		drawItem( cItem[i].rect.left, cItem[i].rect.top, &cItem[i].label,
					listEnabled, settings.cItem == i + 1 );
}

void drawDitherItems()
{
	int			i;
	Boolean		listEnabled;
	
	listEnabled = (gCurrentExample / 10 == ditherID || gCurrentExample / 10 == customID);
		
	for (i = 0; i < numDItems; i++)
		drawItem( dItem[i].rect.left, dItem[i].rect.top, &dItem[i].label,
					listEnabled, settings.dItem == i + 1 );
}

void drawMappingItems()
{
	int			i;
	Boolean		listEnabled;
	
	listEnabled = (gCurrentExample / 10 == searchProcID || gCurrentExample / 10 == customID);
	
	for (i = 0; i < numMItems; i++)
		drawItem( mItem[i].rect.left, mItem[i].rect.top, &mItem[i].label,
					listEnabled, settings.mItem == i + 1 );
}

void drawDestinationItems()
{
	int			i;
	
	for (i = 0; i < numBItems; i++)
		drawItem( bItem[i].rect.left, bItem[i].rect.top, &bItem[i].label,
					true, settings.bItem == i + 1 );
}

void drawPaintBucketItems()
{
	int			i;
	Boolean		listEnabled;

	listEnabled = false;
	
	for (i = 0; i < numPItems; i++)
		drawItem( pItem[i].rect.left, pItem[i].rect.top, &pItem[i].label,
					listEnabled, settings.pItem == i + 1 );
}

void drawLassoToolItems()
{
	int			i;
	Boolean		listEnabled;

	listEnabled = false;
	
	for (i = 0; i < numLItems; i++)
		drawItem( lItem[i].rect.left, lItem[i].rect.top, &lItem[i].label,
					listEnabled, settings.lItem == i + 1 );
}

void drawName(int left, int top, Str255 name )
{
	RGBColor	color;
	
	ForeColor( blackColor );
	MoveTo( left + 1, top + 1 );
	DrawString( name );
	
	color.red = color.green = 0xffff;
	color.blue = 40000;
	RGBForeColor( &color );
	
	MoveTo( left, top );
	DrawString( name );
}

void drawItem(int left, int top, Str255 *label, Boolean listEnabled, Boolean itemEnabled )
{
	Rect		rect;
	RGBColor	color, markColor;
	
	if (listEnabled)
		color.red = color.green = color.blue = 0xffff;
	else
		color.red = color.green = color.blue = 0xffff / 2;
		
	SetRect( &rect, left, top + 2, left + 12, top + 14 );
	ForeColor( blackColor );
	FrameOval( &rect );
	
	OffsetRect( &rect, -1, -1 );
	RGBForeColor( &color );
	FrameOval( &rect );
				
	if (listEnabled && itemEnabled)
		markColor.red = markColor.green = markColor.blue = 0xffff;
	else if (itemEnabled)
		markColor.red = markColor.green = markColor.blue = 0xffff / 2;
	else
		markColor.red = markColor.green = markColor.blue = 8700;
	
	InsetRect( &rect, 3, 3 );
	RGBForeColor( &markColor );
	PaintOval( &rect );
	
	ForeColor( blackColor );
	MoveTo( left + 16, top + 11 );
	DrawString( (unsigned char*)label );
	
	RGBForeColor( &color );
	MoveTo( left + 15, top + 10 );
	DrawString( (unsigned char*)label );
}

void drawDeepBox( Rect *rect )
{
	Rect		box;
	
	box = *rect;
	box.right += 2;
	box.bottom += 2;
	
	ForeColor( blackColor );
	FrameRect( &box );
	
	OffsetRect( &box, -1, -1 );
	ForeColor( whiteColor );
	FrameRect( &box );
}

void drawExampleName()
{
	int			left, top;
	Rect		rect;
	Str255		label = "\pCurrent Example:  ";
	Str255		name[9] = {	"\p TRANSFER MODES", "\pARITHMETIC MODES", "\p   DITHERING",
							"\p  COLORIZATION", "\p  COLOR MAPPING", "\p  PAINT BUCKET",
							"\p   LASSO TOOL", "\pPIXEL AVERAGING", "\p    CUSTOM"	};
	Rect		tempRect1;
	
	left = 15;
	top = GetPortBounds(GetWindowPort(gWindow), &tempRect1)->bottom - 13;
	
	drawName( left, top, label );
	left += StringWidth( label );
	
	SetRect( &rect, left - 2, top - 12, left + 130, top + 4 );
	eraseRect( &rect );
	
	drawName( left, top, name[(gCurrentExample / 10) - 1] );
}

void drawTime( long ticks )
{
	Rect	rect;
	int		left, top;
	float	seconds;
	char	cString[40];
	Rect	tempRect1;
	
	GetPortBounds(GetWindowPort(gWindow), &tempRect1);
	left = tempRect1.right - 130;
	top = tempRect1.bottom - 13;
	
	
	SetRect( &rect, left - 2, top - 12, left + 90, top + 4 );
	eraseRect( &rect );
	
	seconds = (float)ticks / 60.0;
	sprintf( &cString[0], "Draw Time: %03.03f secs", seconds );
	
	drawName( left, top, C2PStr( cString ) );
}

void eraseRect( Rect *rect )
{
	RGBColor	color;
	
	color.red = color.green = color.blue = 8700;
	
	RGBForeColor( &color );
	PaintRect( rect );
}

void drawSourceImage()
{
	Rect	rect;
	Rect	outlineRect;
	Rect	tempRect1;

	GetPortBounds(gGWorld, &tempRect1);		
	SetRect( &rect, 20, 37, 20 + tempRect1.right, 37 + tempRect1.bottom);

	outlineRect = rect;
	InsetRect( &outlineRect, -5, -5 );
	drawDeepBox( &outlineRect );
	
	ForeColor( blackColor );
	BackColor( whiteColor );
	
	CopyBits( (BitMap *)(*(GetPortPixMap(gGWorld))), GetPortBitMapForCopyBits(GetWindowPort(gWindow)),
				&((**(GetPortPixMap(gGWorld))).bounds), &rect, srcCopy, nil );
}