/*
	File:		rotateSampleMain.c

	Contains:	

	Written by: RT and BS	

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
                                                        
        7/14/1999	KG				Updated for Metrowerks Codewarror Pro 2.1
				

*/
#include "CarbonPrefix.h"
#include "RotateString.h"

#define	NIL_POINTER			0L
#define REMOVE_ALL_EVENTS	0
#define VISIBLE				true
#define	MOVE_TO_FRONT		(Ptr)-1
#define HAS_GOAWAY			true
int	main()
{
	WindowPtr	window1,window2;
	BitMap		destMap;
	Rect		windowRect;
	RgnHandle	rgnHandle = NewRgn();
	
	/*InitGraf( &qd.thePort);
	InitFonts();
	InitWindows();
	InitMenus();
	TEInit();
	InitDialogs( NIL_POINTER);*/
	InitCursor();
	FlushEvents( everyEvent, REMOVE_ALL_EVENTS);
	
	SetRect( &windowRect, 100, 100, 200, 200);
	
	window1 = NewWindow( NIL_POINTER, &windowRect, "\pCounter CW", VISIBLE,
				noGrowDocProc,(WindowPtr)-1, HAS_GOAWAY, NIL_POINTER);

	//SetPort(window1);
	SetPortWindowPort(window1);
	TextFont(4);
	//TextFace(bold+italic);
	TextSize(96);
	
	RotateString( "\pRotatey", &destMap, counterClockWise);
	SizeWindow(window1,destMap.bounds.right,destMap.bounds.bottom,false);

	/*CopyBits( &destMap, &window1->portBits, &destMap.bounds,
			 &destMap.bounds, srcCopy, NIL_POINTER);*/
	CopyBits( &destMap, GetPortBitMapForCopyBits(GetWindowPort(window1)), &destMap.bounds,
			 &destMap.bounds, srcCopy, NIL_POINTER);
	QDFlushPortBuffer(GetWindowPort(window1), GetPortVisibleRegion(GetWindowPort(window1), rgnHandle));
	
	OffsetRect(&windowRect,(windowRect.right - windowRect.left ) + 40,0);
	window2 = NewWindow( NIL_POINTER, &windowRect, "\pClock Wise", VISIBLE,
				noGrowDocProc,(WindowPtr) MOVE_TO_FRONT, HAS_GOAWAY, NIL_POINTER);

	//SetPort(window2);
	SetPortWindowPort(window2);
	TextFont(4);
	//TextFace(bold+italic);
	TextSize(96);
	
	RotateString( "\pRotate", &destMap, clockWise);
	SizeWindow(window2,destMap.bounds.right,destMap.bounds.bottom,false);

	/*CopyBits( &destMap, &window2->portBits, &destMap.bounds,
			 &destMap.bounds, srcCopy, NIL_POINTER);*/
	CopyBits( &destMap, GetPortBitMapForCopyBits(GetWindowPort(window2)), &destMap.bounds,
			 &destMap.bounds, srcCopy, NIL_POINTER);
	QDFlushPortBuffer(GetWindowPort(window2), GetPortVisibleRegion(GetWindowPort(window2), rgnHandle));

	while( !Button());
	
	DisposeRgn(rgnHandle);
        
        return 0;
}