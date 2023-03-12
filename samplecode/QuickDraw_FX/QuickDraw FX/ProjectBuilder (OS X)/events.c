/*
	File:		events.c

	Contains:	

	Written by: Edgar Lee		

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
        6/2003		MK				Updated for Project Builders
        7/13/1999	Karl Groethe	Updated for Metrowerks Codewarror Pro 2.1
				

*/
#include "FX.h"

void handleMouseDown();

void eventLoop()
{
	EventRecord 	event;
	WindowPtr		window;
	short			clickArea;
	Rect 			screenRect;
	long			sleep = 30;
	Point			point;
	
	for (;;)
	{		
		if (WaitNextEvent( everyEvent, &event, sleep, (RgnHandle)nil ))
		{
			if (event.what == mouseDown)
			{
				clickArea = FindWindow( event.where, &window );
				
				if (clickArea == inDrag)
				{
					//screenRect = (**GetGrayRgn()).rgnBBox;
					GetRegionBounds(GetGrayRgn(), &screenRect);
					DragWindow(window, event.where, &screenRect );
				}
				else if (clickArea == inGoAway)
				{
					if (TrackGoAway( window , event.where ))
						ExitToShell();
				}
				else if (clickArea == inMenuBar)
				{
					adjustMenus();
					handleMenu( MenuSelect( event.where ) );
				}
				else if (clickArea == inContent)
				{
					if (window != FrontWindow())
						SelectWindow( window );
					else
					{
						point = event.where;
						GlobalToLocal( &point );
						handleMouseDown( point );
					}
				}
			}
			else if (event.what == updateEvt)
			{
				window = (WindowPtr)event.message;
				//SetPort( window );
				SetPortWindowPort( window );
				
				BeginUpdate( window );
				updateWindow();
				EndUpdate( window );
			}
			else if (event.what ==  keyDown || event.what == autoKey)
			{
				if ((event.modifiers & cmdKey) != 0)
				{
					adjustMenus();
					handleMenu( MenuKey( (char)(event.message & charCodeMask) ) );
				}
			}
		}
	}
}

void handleMouseDown( Point point )
{
	int		i;
	long	ticks;
	int		itemNum = -1;
	Rect	tempRect1, tempRect2;

	for (i = 0; i < numBItems; i++)
		if (PtInRect( point, &bItem[i].rect ))
		{
			if (i == settings.bItem - 1)
				return;
				
			drawItem( bItem[settings.bItem - 1].rect.left, bItem[settings.bItem - 1].rect.top,
								&bItem[settings.bItem - 1].label, true, false );
			drawItem( bItem[i].rect.left, bItem[i].rect.top,
								&bItem[i].label, true, true );

			itemNum = 0;
			settings.bItem = i + 1;
			break;
		}				

	if (gCurrentExample / 10 == transferID || gCurrentExample / 10 == customID)
		for (i = 0; i < numTItems && itemNum == -1; i++)
			if (PtInRect( point, &tItem[i].rect ))
			{
				if (gCurrentExample % 10 != i + 1 || gCurrentExample / 10 == customID)
				{
					drawItem( tItem[settings.tItem - 1].rect.left, tItem[settings.tItem - 1].rect.top,
								&tItem[settings.tItem - 1].label, true, false );
					drawItem( tItem[i].rect.left, tItem[i].rect.top,
								&tItem[i].label, true, true );
					
					itemNum = i + 1;
					settings.tItem = itemNum;
				}				
				break;
			}
		
	if (gCurrentExample / 10 == arithmeticID || gCurrentExample / 10 == customID)			
		for (i = 0; i < numAItems && itemNum == -1; i++)
			if (PtInRect( point, &aItem[i].rect ))
			{
				if (gCurrentExample % 10 != i + 1 || gCurrentExample / 10 == customID)
				{
					drawItem( aItem[settings.aItem - 1].rect.left, aItem[settings.aItem - 1].rect.top,
								&aItem[settings.aItem - 1].label, true, false );
					drawItem( aItem[i].rect.left, aItem[i].rect.top,
								&aItem[i].label, true, true );
					
					itemNum = i + 1;
					settings.aItem = itemNum;
				}		
				break;
			}
	
	if (gCurrentExample / 10 == colorizationID || gCurrentExample / 10 == customID)	
		for (i = 0; i < numCItems && itemNum == -1; i++)
			if (PtInRect( point, &cItem[i].rect ))
			{
				if (gCurrentExample % 10 != i + 1 || gCurrentExample / 10 == customID)
				{
					drawItem( cItem[settings.cItem - 1].rect.left, cItem[settings.cItem - 1].rect.top,
								&cItem[settings.cItem - 1].label, true, false );
					drawItem( cItem[i].rect.left, cItem[i].rect.top,
								&cItem[i].label, true, true );
					
					itemNum = i + 1;
					settings.cItem = itemNum;
				}
				break;
			}
		
	if (gCurrentExample / 10 == ditherID || gCurrentExample / 10 == customID)
		for (i = 0; i < numDItems && itemNum == -1; i++)
			if (PtInRect( point, &dItem[i].rect ))
			{
				if (gCurrentExample % 10 != i + 1 || gCurrentExample / 10 == customID)
				{
					drawItem( dItem[settings.dItem - 1].rect.left, dItem[settings.dItem - 1].rect.top,
								&dItem[settings.dItem - 1].label, true, false );
					drawItem( dItem[i].rect.left, dItem[i].rect.top,
								&dItem[i].label, true, true );
					
					itemNum = i + 1;
					settings.dItem = itemNum;
				}
				break;
			}
	
	if (gCurrentExample / 10 == searchProcID || gCurrentExample / 10 == customID)
		for (i = 0; i < numMItems && itemNum == -1; i++)
			if (PtInRect( point, &mItem[i].rect ))
			{
				if (gCurrentExample % 10 != i + 1 || gCurrentExample / 10 == customID)
				{
					drawItem( mItem[settings.mItem - 1].rect.left, mItem[settings.mItem - 1].rect.top,
								&mItem[settings.mItem - 1].label, true, false );
					drawItem( mItem[i].rect.left, mItem[i].rect.top,
								&mItem[i].label, true, true );
					
					itemNum = i + 1;
					settings.mItem = itemNum;
				}	
				break;
			}

	if (gCurrentExample / 10 == paintBucketID)
	{
		Rect	rect;
		void	paintBucketExample();
		
		/*SetRect( &rect, (*gWindow).portRect.right - (*gGWorld).portRect.right - 20, 37,
				(*gWindow).portRect.right - 20, 37 + (*gGWorld).portRect.bottom );*/
		
		GetPortBounds(GetWindowPort(gWindow), &tempRect1);
		GetPortBounds(gGWorld, &tempRect2);
		SetRect( &rect, tempRect1.right - tempRect2.right - 20, 37,
				tempRect1.right - 20, 37 + tempRect2.bottom);

		paintBucketExample( &rect, 0, point );
	}
	if (itemNum != -1)
	{
		if (itemNum == 0)
		{
			drawSourceImage();
			ticks = drawFXImage();
			drawTime( ticks );
		}
		else
		{
			if (gCurrentExample / 10 == customID)
				itemNum = gCurrentExample;
			else
				itemNum = ((gCurrentExample / 10) * 10) + itemNum;
				
			if (itemNum != gCurrentExample || itemNum / 10 == customID)
			{
				gCurrentExample = itemNum;
				
				drawSourceImage();
				ticks = drawFXImage();
				drawTime( ticks );
			}
		}
	}
}