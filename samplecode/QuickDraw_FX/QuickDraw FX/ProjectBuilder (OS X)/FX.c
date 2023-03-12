/*
	File:		FX.c

	Contains:	

	Written by: 	EL	

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
                                                        
        7/13/1999	KG				Updated for Metrowerks Codewarror Pro 2.1
				

*/

#include "FX.h"


long drawFXImage()
{
	int		exampleNumber, exampleItem;
	Rect	rect, rect2;
	Rect	outlineRect;
	long	startTicks, endTicks;
	Point	point = { 0, 0 };
	Rect	frame;
	Rect	windowRect, gWorldRect;

	GetPortBounds(GetWindowPort(gWindow), &windowRect);
	GetPortBounds(gGWorld, &gWorldRect);
	SetRect( &rect, windowRect.right - gWorldRect.right - 20, 37,
				windowRect.right - 20, 37 + gWorldRect.bottom );
	
	outlineRect = rect;
	InsetRect( &outlineRect, -5, -5 );
	drawDeepBox( &outlineRect );
	
	frame = rect;
	
	if (settings.bItem == 1)
	{
		ForeColor( whiteColor );
		PaintRect( &rect );
	}
	else if (settings.bItem == 2)
	{
		ForeColor( blackColor );
		PaintRect( &rect );
	}
	else
	{
		rect2 = rect;
		
		rect2.right = (rect2.right - rect2.left) / 2 + rect2.left;
		ForeColor( whiteColor );
		PaintRect( &rect2 );
		
		rect2.left = rect2.right;
		rect2.right = rect.right;
		ForeColor( blackColor );
		PaintRect( &rect2 );
	}

	ForeColor( blackColor );
	BackColor( whiteColor );
	
	exampleNumber = gCurrentExample / 10;
	exampleItem = gCurrentExample % 10;
	
	startTicks = TickCount();
	
	if (exampleNumber == 1)
		transferExample( &rect, exampleItem );
	else if (exampleNumber == 2)
		arithmeticExample( &rect, exampleItem );
	else if (exampleNumber == 3)
		ditherExample( &rect, exampleItem );
	else if (exampleNumber == 4)
		colorizationExample( &rect, exampleItem );
	else if (exampleNumber == 5)
		mappingExample( &rect, exampleItem );
	else if (exampleNumber == 6)
		paintBucketExample( &rect, exampleItem, point );
	else if (exampleNumber == 7)
		lassoToolExample( &rect, exampleItem, &frame );
	else if (exampleNumber == 8)
		pixelAverageExample( &rect, exampleItem );
	else if (exampleNumber == 9)
		customExample( &rect, exampleItem );
		
	endTicks = TickCount();
	
	return (endTicks - startTicks);
}

void transferExample( Rect *rect, int item )
{
	int		transferMode;
	
	resetItems();
	settings.tItem = item;
	
	transferMode = setTransferMode( item );
	
	CopyBits( (BitMap *)(*(GetPortPixMap(gGWorld))), GetPortBitMapForCopyBits(GetWindowPort(gWindow)),
				&((**(GetPortPixMap(gGWorld))).bounds), rect, transferMode, nil );
}

int setTransferMode(int	 item )
{
	item--;
	return item;
}

void arithmeticExample( Rect *rect, int item )
{
	int			arithmeticMode;
	RGBColor	color;
	
	resetItems();
	settings.aItem = item;
	
	arithmeticMode = setArithmeticMode( item );
	
	CopyBits( (BitMap *)(*(GetPortPixMap(gGWorld))), GetPortBitMapForCopyBits(GetWindowPort(gWindow)),
				&((**(GetPortPixMap(gGWorld))).bounds), rect, arithmeticMode, nil );
				
	color.red = color.green = color.blue = 0;
	OpColor( &color );
}

int setArithmeticMode(int item)
{
	RGBColor	color;
	
	if (item <= 4)
		item = 31 + item;
	else if (item == 9)
		item = transparent;
	else
		item = 32 + item;
	
	color.red = color.green = color.blue = 0xffff / 2;
	OpColor( &color );
	
	return item;
}

void colorizationExample(Rect *rect, int item )
{
	int			i;
	Rect		srcRect, dstRect;
	RGBColor	color;
	
	resetItems();
	settings.cItem = item;
	
	srcRect = (**(GetPortPixMap(gGWorld))).bounds;
	srcRect.right = srcRect.left + (srcRect.right - srcRect.left) / 3;
	
	dstRect = *rect;
	dstRect.right = dstRect.left + (dstRect.right - dstRect.left) / 3;
	
	for (i = 0; i < 3; i++)
	{
		if (item == 1)
		{
			ForeColor( blackColor );
			BackColor( whiteColor );
		}	
		else if (item == 2)
		{
			ForeColor( whiteColor );
			BackColor( blackColor );
		}
		else
		{
			color.red = color.green = color.blue = 0;
		
			if (i == 0)
				color.red = 0xffff;
			else if (i == 1)
				color.green = 0xffff;
			else if (i == 2)
				color.blue = 0xffff;
			
			if (item == 3)
			{
				RGBForeColor( &color );
				BackColor( whiteColor );
			}
			else
			{
				RGBBackColor( &color );
				ForeColor( blackColor );
			}
		}
				
		CopyBits( (BitMap *)(*(GetPortPixMap(gGWorld))), GetPortBitMapForCopyBits(GetWindowPort(gWindow)),
				&srcRect, &dstRect, srcCopy, nil );
				
		OffsetRect( &srcRect, srcRect.right - srcRect.left, 0 );
		OffsetRect( &dstRect, dstRect.right - dstRect.left, 0 );
	}
}

void ditherExample(Rect	* rect,int item )
{
	#pragma unused(rect)
	int		ditherMode;
	
	/* This function shows an example of using copybits with the dithering. */
	
	resetItems();
	settings.dItem = item;
	
	ditherMode = setDitherMode( item );
	
	CopyBits( (BitMap *)(*(GetPortPixMap(gGWorld))), GetPortBitMapForCopyBits(GetWindowPort(gWindow)),
				&(**(GetPortPixMap(gGWorld))).bounds, rect, srcCopy + ditherMode, nil );
}

int setDitherMode(int item)
{
	if (item == 1)
		return 0;
	else
		return ditherCopy;
}

void mappingExample(Rect * rect, int	item )
{
	int			i;
	GWorldPtr	gworld;
	CTabHandle	ictable;
	Rect		tempRect1;
	
	resetItems();
	settings.mItem = item;
	
	if (item == 1)
		CopyBits( (BitMap *)(*(GetPortPixMap(gGWorld))), GetPortBitMapForCopyBits(GetWindowPort(gWindow)),
				&((**(GetPortPixMap(gGWorld))).bounds), rect, srcCopy, nil );
	else if (item == 2)
	{
		ColorSearchUPP searchProcUPP = NewColorSearchUPP(searchProc);
		AddSearch(searchProcUPP);
		
		CopyBits( (BitMap *)(*(GetPortPixMap(gGWorld))), GetPortBitMapForCopyBits(GetWindowPort(gWindow)),
				&((**(GetPortPixMap(gGWorld))).bounds), rect, srcCopy, nil );
		
		DelSearch(searchProcUPP);
	}
	else if (item == 3)
	{
		NewGWorld( &gworld, 8, GetPortBounds(gGWorld, &tempRect1), GetCTable( 8 + 64 ), nil, 0 );
		
		CopyBits( (BitMap *)(*(GetPortPixMap(gGWorld))), (BitMap *)(*(GetPortPixMap(gworld))),
					&((**(GetPortPixMap(gGWorld))).bounds), &((**(GetPortPixMap(gworld))).bounds), 
					srcCopy, nil );
		
		ictable = GetCTable( 8 );
		
		for (i = 0; i <= (**(**(GetPortPixMap(gworld))).pmTable).ctSize; i++)
			(**(**(GetPortPixMap(gworld))).pmTable).ctTable[i].rgb =
					(**ictable).ctTable[255 - i].rgb;
		
		(**(**(GetPortPixMap(gworld))).pmTable).ctSeed = 67;
		
		CopyBits( (BitMap *)(*(GetPortPixMap(gworld))), GetPortBitMapForCopyBits(GetWindowPort(gWindow)),
			&(**(GetPortPixMap(gworld))).bounds, rect, srcCopy, nil );
		
		DisposeGWorld( gworld );
	}
}

pascal Boolean searchProc( RGBColor	* color, long		*position )
{
	#pragma unused(position)
	(*color).red = 0;
	
	return false;
}

void paintBucketExample(Rect *rect, int exampleItem, Point point )
{
	#pragma unused(exampleItem)
	GrafPtr     mask;
	Point		newPoint;
	GWorldPtr	gworld;
	Rect		tempRect1;
	
	resetItems();

	newPoint.h = point.h - 20;
	newPoint.v = point.v - 37;

	tempRect1 = (**(GetPortPixMap(gGWorld))).bounds;
	if (newPoint.h < 0 || newPoint.v < 0 ||
		newPoint.h > tempRect1.right - tempRect1.left ||
		newPoint.v > tempRect1.bottom - tempRect1.top)
		return;
	mask = CreateGrafPort(GetPortBounds(gGWorld, &tempRect1));
	
	ForeColor( redColor );
	MoveTo( point.h - 2, point.v );
	LineTo( point.h + 2, point.v );
	MoveTo( point.h, point.v - 2 );
	LineTo( point.h, point.v + 2 );

	ForeColor( blackColor );

	NewGWorld( &gworld, 8, GetPortBounds(gGWorld, &tempRect1), GetCTable( 8 + 64 ), nil, 0 );
		
	CopyBits( (BitMap *)(*(GetPortPixMap(gGWorld))), (BitMap *)(*(GetPortPixMap(gworld))),
					&((**(GetPortPixMap(gGWorld))).bounds), &((**(GetPortPixMap(gworld))).bounds), 
					srcCopy, nil );
	
	SeedCFill( (BitMap *)(*(GetPortPixMap(gworld))), GetPortBitMapForCopyBits(mask),
				&((**(GetPortPixMap(gworld))).bounds), GetPortBounds(mask, &tempRect1),
				newPoint.h, newPoint.v, nil, 0 );

	CopyMask( (BitMap *)(*(GetPortPixMap(gworld))), GetPortBitMapForCopyBits(mask), GetPortBitMapForCopyBits(GetWindowPort(gWindow)),
				&(**(GetPortPixMap(gworld))).bounds, GetPortBounds(mask, &tempRect1), rect );
	
	DisposeGrafPort( mask );
}

void lassoToolExample(Rect	* rect, int	exampleItem,Rect *frame )
{
	#pragma unused(exampleItem,frame)
	GrafPtr     mask;
	RGBColor	color;
	CGrafPtr	currentPort;
	GDHandle	currentDevice;
	Rect		tempRect1;
	
	pascal Boolean matchProc();
	
	resetItems();

	mask = CreateGrafPort(GetPortBounds(gGWorld, &tempRect1));

	GetGWorld( &currentPort, &currentDevice );
	SetGWorld( gGWorld, nil );
	
	color.red = color.green = color.blue = 0;
	
	CalcCMask( (BitMap *)(*(GetPortPixMap(gGWorld))),GetPortBitMapForCopyBits(mask),
				&(**(GetPortPixMap(gGWorld))).bounds, GetPortBounds(mask, &tempRect1),
				&color, NewColorSearchUPP(matchProc), 0 );
	
	
	SetGWorld( currentPort, currentDevice );
	
	CopyMask( (BitMap *)(*(GetPortPixMap(gGWorld))), GetPortBitMapForCopyBits(mask), GetPortBitMapForCopyBits(GetWindowPort(gWindow)),
				&(**(GetPortPixMap(gGWorld))).bounds, GetPortBounds(mask, &tempRect1), rect );
	
	DisposeGrafPort( mask );
}

pascal Boolean matchProc(RGBColor *color, long		*position )
{	
	GDHandle	currDevice;
	MatchRec	*matchInfo;
	
	currDevice = GetGDevice();
	matchInfo = (MatchRec *)(**currDevice).gdRefCon;

	if (matchInfo->red == color->red && matchInfo->green == color->green &&
		matchInfo->blue == color->blue)
		*position = 0;
	else
		*position = 1;
		
	return true;
}

void pixelAverageExample(Rect	*rect, int	 item )
{
	#pragma unused(item)
	Rect		bounds;
	GWorldPtr	gworld;
	GWorldPtr	source;
	Rect		tempRect1;
	
	resetItems();
	
	/* Create 8-bit gworld from 32-bit original. */
	
	NewGWorld( &source, 8, GetPortBounds(gGWorld, &tempRect1), GetCTable( 8 + 64 ), nil, 0 );
	CopyBits( (BitMap *)(*(GetPortPixMap(gGWorld))), (BitMap *)(*(GetPortPixMap(source))),
					&((**(GetPortPixMap(gGWorld))).bounds), &((**(GetPortPixMap(source))).bounds), 
					srcCopy, nil );
	
	/* Create another 8-bit gworld at quarter size of original. */
				
	GetPortBounds(gGWorld, &tempRect1);
	SetRect( &bounds, 0, 0, tempRect1.right / 4,
				tempRect1.bottom / 4);
	
	NewGWorld( &gworld, 8, &bounds, GetCTable( 8 + 64 ), nil, 0 );

	/* Copy the 8-bit original to 8-bit quarter image without pixel averaging. */
    
	GetPortBounds(source, &bounds);
	
	CopyBits( (BitMap *)(*(GetPortPixMap(source))), (BitMap *)(*(GetPortPixMap(gworld))),
					&((**(GetPortPixMap(source))).bounds), &((**(GetPortPixMap(gworld))).bounds), 
					srcCopy, nil );
	
	/* Copy the quarter image to screen. */
	
	OffsetRect( &bounds, 20, 37 );
	CopyBits( (BitMap *)(*(GetPortPixMap(gworld))), GetPortBitMapForCopyBits(GetWindowPort(gWindow)),
			&(**(GetPortPixMap(gworld))).bounds, &bounds, srcCopy, nil );
	
	/* Now, copy the 8-bit original to 8-bit quarter image with pixel averaging. */
	
	GetPortBounds(source, &bounds);
	
	CopyBits( (BitMap *)(*(GetPortPixMap(source))), (BitMap *)(*(GetPortPixMap(gworld))),
					&((**(GetPortPixMap(source))).bounds), &((**(GetPortPixMap(gworld))).bounds), 
					srcCopy + ditherCopy, nil );
	
	/* Finally, copy pixel averaged quarter image to screen. */
			
	OffsetRect( &bounds, (*rect).left, (*rect).top );
	CopyBits( (BitMap *)(*(GetPortPixMap(gworld))), GetPortBitMapForCopyBits(GetWindowPort(gWindow)),
			&(**(GetPortPixMap(gworld))).bounds, &bounds, srcCopy, nil );
	
	DisposeGWorld( source );
	DisposeGWorld( gworld );
}

void customExample(Rect	* rect,int	item )
{
	#pragma unused(item)
	int			transferMode;
	int			ditherMode;
	RGBColor	color;
	ColorSearchUPP searchProcUPP = NewColorSearchUPP(searchProc);
		
	transferMode = setTransferMode( settings.tItem );
	ditherMode = setDitherMode( settings.dItem );
	
	if (settings.cItem == 1)
	{
		ForeColor( blackColor );
		BackColor( whiteColor );
	}
	else if (settings.cItem == 2)
	{
		ForeColor( whiteColor );
		BackColor( blackColor );
	}
	else if (settings.cItem == 3)
	{
		color.green = 0xffff;
		color.red = color.blue = 0;
		RGBForeColor( &color );
		BackColor( whiteColor );
	}
	else if (settings.cItem == 4)
	{
		color.green = 0xffff;
		color.red = color.blue = 0;
		ForeColor( blackColor );
		RGBBackColor( &color );
	}
	
	
	if (settings.mItem == 2)
		AddSearch(searchProcUPP);
	
	CopyBits( (BitMap *)(*(GetPortPixMap(gGWorld))), GetPortBitMapForCopyBits(GetWindowPort(gWindow)),
				&(**(GetPortPixMap(gGWorld))).bounds, rect,
				transferMode + ditherMode, nil );
				
	if (settings.mItem == 2)
		DelSearch(searchProcUPP);
}

GrafPtr CreateGrafPort( Rect *bounds )	/* CreateGrafPort originally written by Forrest Tanaka. */
{
	GrafPtr	savedPort;		/* Saved GrafPtr for later restore. */
	GrafPtr	newPort;		/* New GrafPort. */
	Rect	localBounds;	/* Local copy of bounds. */
	RgnHandle		rgnHandle = NewRgn();	// Used for carbonization

	GetPort( &savedPort );

	localBounds = *bounds;
	OffsetRect( &localBounds, -bounds->left, -bounds->top );

	newPort = CreateNewPort();
	
	if (newPort != nil)
	{
		SetPort( newPort);

		(*(GetPortPixMap(newPort)))->bounds = localBounds;
		(*(GetPortPixMap(newPort)))->rowBytes = ((localBounds.right + 15) >> 4) <<1;
		(*(GetPortPixMap(newPort)))->baseAddr = NewPtrClear(GetPortBitMapForCopyBits(newPort)->rowBytes *
														(long)localBounds.bottom );
		
		if (GetPortBitMapForCopyBits(newPort)->baseAddr != nil)
		{
			SetPortBounds(newPort, &localBounds);
			ClipRect( &localBounds );
			RectRgn( GetPortVisibleRegion(newPort, rgnHandle), &localBounds );
			EraseRect( &localBounds );
		}
		else
		{
			DisposePort( newPort );
			newPort = nil;
		}
	}
	
	DisposeRgn(rgnHandle);
	SetPort( savedPort );
	return newPort;
}

void DisposeGrafPort( GrafPtr doomedPort )	/* DisposeGrafPort originally written by Forrest Tanaka. */
{
	DisposePtr( GetPortBitMapForCopyBits(doomedPort)->baseAddr );
	DisposePort( doomedPort );
}