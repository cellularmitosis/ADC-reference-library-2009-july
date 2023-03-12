/*
	File:		ZoomRecter.c

	Contains:	This snippet shows how to do "Finder" style zooming between two rectangles.
                The boolean flag "kZoomLarger" controls the proportional direction of the
                zooming.

                To get the two rectangles, you drag them out rubberbanded, and the zoom
                occurs between them.  To quit, click the close box.

                If you want to do zooms between windows, open up a port with the dimensions
                of the desktop (from GetGrayRgn()).

                DON'T use this as a sample of how to do rubberband drawing!!!  It's sort of
                hacked together bypassing the event mechanism and just using Button().

	Written by: SF	

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
                                    for demonstration purposes.  Changed basic code
                                    significantly so it would simply work better.....
                                    
        7/14/1999	KG				Updated for Metrowerks Codewarror Pro 2.1
				

*/
#include "CarbonPrefix.h"

#define	kNumSteps		14
#define	kRectsVisible	4
#define	kZoomRatio		.7
#define	kDelayTicks		1

#define	kZoomLarger		true		// change this to zoom "inward"


void InitStuff(void);
void ZoomRect(Boolean zoomOut,Rect *smallRect, Rect *bigRect);
void CalcRect(Rect *theRect,Rect *smallRect,Rect *bigRect,double stepValue);
//Boolean GetRects(Rect *zoomFrom,Rect *zoomTo);
void FixRect(Rect *theRect,Rect *rightRect);
void doEventLoop();
void GetRect(Rect *zoomFrom, Rect *zoomTo);

Boolean gDone = false;
WindowPtr gWindow;

int main()
{
	
	Rect bounds = {44,12,330,500}/*,zoomFrom,zoomTo*/;
	//Rect	tempRect1;	//used for carbonization
		
	InitStuff();
	gWindow = NewWindow(nil,&bounds,"\pDrag Two Rects to Zoom",true,documentProc,(WindowPtr)-1L,true,0);
	//SetPort(window);
	SetPortWindowPort(gWindow);
	
	/*do {
		if (GetRects(&zoomFrom,&zoomTo))
			ZoomRect(kZoomLarger,&zoomFrom,&zoomTo);
		//EraseRect(&window->portRect);
		EraseRect(GetPortBounds(GetWindowPort(window), &tempRect1));
	}
	while (!gDone);*/
	
	doEventLoop();
	
	FlushEvents(everyEvent,0);
        
        return 0;
}

	
void InitStuff(void)
{
	//InitGraf(&qd.thePort);
	//InitFonts();
	//InitWindows();
	//InitMenus();
	//TEInit();
	//InitDialogs(nil);
	InitCursor();
	FlushEvents(everyEvent,0);
}


void ZoomRect(Boolean zoomLarger,Rect *smallRect, Rect *bigRect)
{
	double firstStep,stepValue,trailer,zoomRatio;
	short i,step;
	Rect curRect;
	unsigned long ticks;
	Pattern	grayPattern; //used in carbonization
	RgnHandle rgnHandle = NewRgn();
	GrafPtr oldPort;
	Rect	tempRect1;
	
	GetPort(&oldPort);
	SetPort(GetWindowPort(gWindow));
	
	//PenPat(&qd.gray);
	PenPat(GetQDGlobalsGray(&grayPattern));
	PenMode(patXor);
	
	
	firstStep=kZoomRatio;
	for (i=0; i<kNumSteps; i++) {
		firstStep *= kZoomRatio;
	}

	if (!zoomLarger) {
		zoomRatio = 1.0/kZoomRatio;
		firstStep = 1.0-firstStep;
	}
	else
		zoomRatio = kZoomRatio;
		
	trailer = firstStep;
	stepValue = firstStep;
	for (step=0; step<(kNumSteps+kRectsVisible); step++) {
	
		// draw new frame
		
		if (step<kNumSteps) {
			stepValue /= zoomRatio;
			CalcRect(&curRect,smallRect,bigRect,stepValue);
			FrameRect(&curRect);
		}
		
		// erase old frame
		
		if (step>=kRectsVisible) {
			trailer /= zoomRatio;
			CalcRect(&curRect,smallRect,bigRect,trailer);
			FrameRect(&curRect);
		}
		QDFlushPortBuffer(GetWindowPort(gWindow), GetPortVisibleRegion(GetWindowPort(gWindow), rgnHandle));
		Delay(kDelayTicks,&ticks);
		
	}

	PenNormal();
	DisposeRgn(rgnHandle);
	smallRect->top = bigRect->top = -1;
	EraseRect(GetPortBounds(GetWindowPort(gWindow), &tempRect1));
	
	SetPort(oldPort);
}


void CalcRect(Rect *theRect,Rect *smallRect,Rect *bigRect,double stepValue)
{
	theRect->left = smallRect->left + (short)((double)(bigRect->left-smallRect->left)*stepValue);
	theRect->top = smallRect->top + (short)((double)(bigRect->top-smallRect->top)*stepValue);
	theRect->right = smallRect->right + (short)((double)(bigRect->right-smallRect->right)*stepValue);
	theRect->bottom = smallRect->bottom + (short)((double)(bigRect->bottom-smallRect->bottom)*stepValue);
}

// Old version, would require that you drag 2 rects before letting the event loop continue
/*Boolean GetRects(Rect *zoomFrom,Rect *zoomTo)
{
	short numRects = 0;
	//EventRecord ev;
	Rect theRect,drawRect;
	Point firstPt,curPt,oldPt,globalPt;
	//KeyMap theKeys;
	WindowPtr window;
	Boolean result = true;
	RgnHandle rgnHandle = NewRgn();
	
	PenMode(patXor);
	
	do {
		while (!Button()) {
		
			GetMouse(&globalPt);
			LocalToGlobal(&globalPt);
			if ((FindWindow(globalPt,&window)==inGoAway) && window==FrontWindow()) {
				gDone = true;
				result = false;
			}
		
			GetMouse(&firstPt);
			oldPt = firstPt;
			SetRect(&theRect,firstPt.h,firstPt.v,firstPt.h,firstPt.v);
		}
		
		if (gDone)
			break;
		
		while (Button()) {
			GetMouse(&curPt);
			if (!EqualPt(curPt,oldPt)) {
				FixRect(&theRect,&drawRect);
				FrameRect(&drawRect);
				oldPt = curPt;
				theRect.right = curPt.h;
				theRect.bottom = curPt.v;
				FixRect(&theRect,&drawRect);
				FrameRect(&drawRect);
			}
		}
		
		FixRect(&theRect,&drawRect);
		if (numRects==0)
			*zoomFrom = drawRect;
		else
			*zoomTo = drawRect;
			
		numRects++;
		
		QDFlushPortBuffer(GetWindowPort(window), GetPortVisibleRegion(GetWindowPort(window), rgnHandle));

	} while (numRects<2);
	
	PenNormal();
	DisposeRgn(rgnHandle);
	
	return result;
}*/

// New Version of GetRect(s), modified from original to only handle one dragged rect at a time.

void GetRect(Rect *zoomFrom, Rect *zoomTo)
{
	static short numRects = 0;
	Rect theRect,drawRect;
	Point firstPt,curPt,oldPt;
	RgnHandle rgnHandle = NewRgn();
	GrafPtr	oldPort;
	
	GetPort(&oldPort);
	SetPort(GetWindowPort(gWindow));
	
	PenMode(patXor);
	
	GetMouse(&firstPt);
	oldPt = firstPt;
	SetRect(&theRect,firstPt.h,firstPt.v,firstPt.h,firstPt.v);
	
	while (Button()) {
		GetMouse(&curPt);
		if (!EqualPt(curPt,oldPt)) {
			FixRect(&theRect,&drawRect);
			FrameRect(&drawRect);
			oldPt = curPt;
			theRect.right = curPt.h;
			theRect.bottom = curPt.v;
			FixRect(&theRect,&drawRect);
			FrameRect(&drawRect);
			QDFlushPortBuffer(GetWindowPort(gWindow), GetPortVisibleRegion(GetWindowPort(gWindow), rgnHandle));
		}
	}
		
	FixRect(&theRect,&drawRect);
	if (numRects==0)
		*zoomFrom = drawRect;
	else
		*zoomTo = drawRect;
			
	numRects++;
		
	QDFlushPortBuffer(GetWindowPort(gWindow), GetPortVisibleRegion(GetWindowPort(gWindow), rgnHandle));
	
	if (numRects >= 2) {
		ZoomRect(kZoomLarger, zoomFrom, zoomTo);
		numRects = 0;
	}
	
	PenNormal();
	DisposeRgn(rgnHandle);
	SetPort(oldPort);

}


void FixRect(Rect *theRect,Rect *rightRect)
{
	if (theRect->right > theRect->left) {
		rightRect->right = theRect->right;
		rightRect->left = theRect->left;
	}
	else {
		rightRect->right = theRect->left;
		rightRect->left = theRect->right;
	}
	
	if (theRect->bottom > theRect->top) {
		rightRect->bottom = theRect->bottom;
		rightRect->top = theRect->top;
	}
	else {
		rightRect->bottom = theRect->top;
		rightRect->top = theRect->bottom;
	}
	
}

void doEventLoop()
{
	EventRecord anEvent;
	WindowPtr   evtWind;
	short       clickArea;
	Rect        screenRect;
	Point		thePoint;
	Rect		zoomFrom, zoomTo;
	
	zoomFrom.top = zoomTo.top = -1;

	while (!gDone)
	{
		if (WaitNextEvent( everyEvent, &anEvent, 0, nil ))
		{
			if (anEvent.what == mouseDown)
			{
				clickArea = FindWindow( anEvent.where, &evtWind );
				
				if (clickArea == inDrag)
				{
					GetRegionBounds( GetGrayRgn(), &screenRect );
					DragWindow( evtWind, anEvent.where, &screenRect );
				}
				else if (clickArea == inContent)
				{
					if (evtWind != FrontWindow())
						SelectWindow( evtWind );
					else
					{
						thePoint = anEvent.where;
						GlobalToLocal( &thePoint );
						//Handle click in window content here
						GetRect(&zoomFrom, &zoomTo);
					}
				}
				else if (clickArea == inGoAway)
					if (TrackGoAway( evtWind, anEvent.where ))
						gDone = true;
			}
			else if (anEvent.what == updateEvt)
			{
				evtWind = (WindowPtr)anEvent.message;	
				SetPortWindowPort( evtWind );
				
				BeginUpdate( evtWind );
				//Call Draw Function here....
				if (zoomFrom.top != -1)
					FrameRect(&zoomFrom);
				if (zoomTo.top != -1)
					FrameRect(&zoomTo);
				EndUpdate (evtWind);
			}
		}
	}
}