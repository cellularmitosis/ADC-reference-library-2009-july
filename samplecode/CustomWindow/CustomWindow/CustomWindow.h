/*
	File:		CustomWindow.h	

	Contains:	The header file for the custom window sample

	Written by: 	Karl Groethe

	Copyright:	Copyright © 2000 by Apple Computer, Inc., All Rights Reserved.

			You may incorporate this Apple sample source code into your program(s) without
			restriction. This Apple sample source code has been provided "AS IS" and the
			responsibility for its operation is yours. You are not permitted to redistribute
			this Apple sample source code as "Apple sample source code" after having made
			changes. If you're going to re-distribute the source, we require that you make
			it clear in the source that the code was descended from Apple sample source
			code, but that you've made changes.

	Change History (most recent first):
                        6/28/00 	created
				

*/
#ifndef CUSTOM_WINDOW_H
#define CUSTOM_WINDOW_H

#include <Carbon/Carbon.h>

SInt32 myWindowDef(short varCode,WindowRef window, SInt16 message, SInt32 param);
SInt32 myWindowDraw(WindowRef window,SInt32 param);
RgnHandle getWindowCloseBoxRegion(WindowRef window,RgnHandle closeBoxRegion);
RgnHandle getWindowZoomBoxRegion(WindowRef window,RgnHandle zoomBoxRegion);
RgnHandle getWindowCollapseBoxRegion(WindowRef window,RgnHandle collapseBoxRegion);
RgnHandle getWindowTitleBarRegion(WindowRef window,RgnHandle titleBarRegion);
RgnHandle getWindowTitleTextRegion(WindowRef window,RgnHandle titleTextRegion);
RgnHandle getWindowGlobalPortRegion(WindowRef window, RgnHandle globalPortRegion);
RgnHandle getWindowContentRegion(WindowRef window,RgnHandle contentRegion);
RgnHandle getWindowStructureRegion(WindowRef window, RgnHandle structureRegion);
RgnHandle getWindowDragRegion(WindowRef window, RgnHandle dragRegion);
RgnHandle getWindowUpdateRegion(WindowRef window,RgnHandle updateRegion);
void drawWindowFrame(WindowRef window);
Boolean drawWindowCloseBox(WindowRef window, Boolean hilite);
Boolean drawWindowZoomBox(WindowRef window, Boolean hilite);
Boolean drawWindowCollapseBox(WindowRef window, Boolean hilite);
SInt32 myWindowHitTest(WindowRef window,SInt32 param);
SInt32 myWindowInitialize(WindowRef window,SInt32 param);
SInt32 myWindowCleanUp(WindowRef window,SInt32 param);
SInt32 myWindowDrawGrowBox(WindowRef window,SInt32 param);
SInt32 myWindowHitTest(WindowRef window,SInt32 param);
SInt32 myWindowGetFeatures(WindowRef window,SInt32 param);
SInt32 myWindowGetRegion(WindowRef window,SInt32 param);
SInt32 myWindowDragHilite(WindowRef window,SInt32 param);
SInt32 myWindowModified(WindowRef window,SInt32 param);
SInt32 myWindowDrawInCurrentPort(WindowRef window,SInt32 param);
SInt32 myWindowStateChanged(WindowRef window,SInt32 param);
SInt32 myWindowGetGrowImageRegion(WindowRef window,SInt32 param);
SInt32 myWindowDrawGrowOutline(WindowRef window, SInt32 param);

void getCurrentPortBounds(Rect* inRect);

#endif
