/*
	File:		CopyDeepMask.c

	Contains:	This snippet demonstrates the use of CopyDeepMask using 2 PICTS;
 				one a photograph and the other a triangular mask. 	It uses 2 offscreen
 				gworlds to hold the source and mask pixmaps.  CopyDeepMask is then used
 				to create the masked image and display it in the application window.  The
 				source, mask, and destination rectangles are all the same size in order
 				avoid altering pixel sizes.

	Written by: KM	

	Copyright:	Copyright © 1993-1999 by Apple Computer, Inc., All Rights Reserved.

				You may incorporate this Apple sample source code into your program(s) without
				restriction. This Apple sample source code has been provided "AS IS" and the
				responsibility for its operation is yours. You are not permitted to redistribute
				this Apple sample source code as "Apple sample source code" after having made
				changes. If you're going to re-distribute the source, we require that you make
				it clear in the source that the code was descended from Apple sample source
				code, but that you've made changes.

	Change History (most recent first):
				08/2000		JM				Carbonized, non-Carbon code is commented out
											for demonstration purposes.
				7/9/1999	KG				Updated for Metrowerks Codewarror Pro 2.1
				12/14/94 	KM				Fixed a bug in doEventLoop that was causing a bus error using 
								 			EvenBetterBusError (bad things happen when you attempt to use an 
								 			unitialized EventRecord!)
				11/22/93	KM				Created		
				

*/

#ifdef __MWERKS__

// includes for MetroWerks CodeWarrior

#include "CarbonPrefix.h"
#include <Quickdraw.h>
#include <QDOffscreen.h>
#include <Windows.h>
#include <Dialogs.h>
#include <Menus.h>
#include <Types.h>
#include <Memory.h>
#include <Fonts.h>
//#include <OSEvents.h>
#include <Events.h>
#include <ToolUtils.h>

#else

#ifdef __APPLE_CC__

// includes for ProjectBuilder

#include <Carbon/Carbon.h>

#else

// includes for MPW

#include <Carbon.h>

#endif
#endif

#define windID 128
#define inFront -1
#define sleepTime 30

#define	 srcPicID	130
#define	 maskPicID	131

Boolean					continueThis;
WindowPtr				mainWindow;

void InitToolbox(void);
void doEventLoop(void);
void doDrag (WindowPtr winPtr,Point mouseLoc);
void doUpdateEvent(EventRecord *event);


void main(void)
{
	
	InitToolbox();
		
	mainWindow = GetNewCWindow(windID,nil,(WindowPtr)inFront);
	
	continueThis = true;
	
	doEventLoop();
}

void InitToolbox(void)
{
	/*MoreMasters();
	MoreMasters();
	MoreMasters();
	
	MaxApplZone();

    InitGraf((Ptr)&qd.thePort);
	InitFonts();
	InitWindows();
	InitMenus();
	//InitDialogs((long)nil);
	TEInit();*/
	
	MoreMasterPointers(3);
	
	InitCursor();

	FlushEvents(everyEvent, 0);
	
}

void doEventLoop(void)
{
	EventRecord	event;
	WindowPtr	window;
	short		hitArea;
		
	
	while (continueThis)
	{	
		if (WaitNextEvent(everyEvent,&event,sleepTime,nil))
			
			if (event.what == updateEvt)	
				doUpdateEvent(&event);
				
			else if (event.what == mouseDown)
			{
				hitArea = FindWindow(event.where,&window);
				
				if (hitArea == inDrag)
					doDrag(window,event.where);
					
				else if (hitArea == inGoAway)
					if (TrackGoAway (window,event.where))
						return;
			}
			
	} 
}
		
void doDrag (WindowPtr winPtr,Point mouseLoc)
{
	Rect 	dragRect;
	Rect	bndsRect;
	BitMap	bitMap;
	
	//bndsRect = qd.screenBits.bounds; /*screenBits is a QuickDraw global variable with the same structure as portBits (BitMap)*/
	bndsRect = GetQDGlobalsScreenBits(&bitMap)->bounds;	//must use accessor functions to get at the global Quickdraw variable in Carbon
	InsetRect(&dragRect,10,10);
	DragWindow(winPtr,mouseLoc,&bndsRect);
	
}

void doUpdateEvent(EventRecord *event)
{
#pragma unused (event)

	Rect			bndsRectSrc,bndsRectMask,srcRect,maskRect,destRect;
	QDErr			gWorldErr;
	GWorldPtr		offscreenGWorldSource;
	GWorldPtr		offscreenGWorldMask;
	short			pixelDepthSource = 32;
	short			pixelDepthMask = 1;
	GWorldFlags		flags = 0;
	GDHandle		currGDevice;
	GWorldPtr		currGWorldPort;
	PicHandle		srcPicHdl;
	PicHandle		maskPicHdl;
	PixMapHandle	srcPMHdl;
	PixMapHandle	maskPMHdl;
	Boolean			pmLock= false;
	GrafPtr			oldPort;
	
	GetPort(&oldPort);

	
	//Set up the bounds rectangle for the source and mask gWorlds	
	bndsRectSrc.top = 32;
	bndsRectSrc.left = 64;
	bndsRectSrc.bottom = 160;
	bndsRectSrc.right = 192;
	
	bndsRectMask = bndsRectSrc;
	
	//Make the source,mask and destination rectangles the same 
	// size as the associated gWorlds
	srcRect = bndsRectSrc;
	maskRect = bndsRectSrc;
	destRect = bndsRectSrc;
	
	//Fetch the current port and gdevice and save them for later
	GetGWorld(&currGWorldPort,&currGDevice);
		
	//Set up our source and mask gWorlds
	gWorldErr = NewGWorld(&offscreenGWorldSource,pixelDepthSource,&bndsRectSrc,0,nil,flags);
	if(gWorldErr != noErr)
		DebugStr("\pthe first NewGWorld call failed");
	else 
	{
		//lock the offscreen buffer
		srcPMHdl = GetGWorldPixMap(offscreenGWorldSource);
		pmLock = LockPixels(srcPMHdl);
		if (!pmLock)
			DebugStr("\pthe first LockPixels call failed");
	}
		
	gWorldErr = NewGWorld(&offscreenGWorldMask,pixelDepthMask,&bndsRectMask,0,nil,flags);
	if(gWorldErr != noErr)
		DebugStr("\pthe second NewGWorld call failed"); 
	else 
	{
		//lock the offscreen buffer
		maskPMHdl = GetGWorldPixMap(offscreenGWorldMask);
		pmLock = LockPixels(maskPMHdl);
		if (!pmLock)
			DebugStr("\pthe second LockPixels call failed");
	}
		
	//Set the current graphics world to my offscreen source and draw into it
	SetGWorld((CGrafPtr) offscreenGWorldSource,nil);
	
	srcPicHdl = GetPicture(srcPicID);
	if (srcPicHdl==nil)
		DebugStr("\pthe first call to GetPicture failed");
	EraseRect(&srcRect);
	HLock((Handle)srcPicHdl);
	DrawPicture(srcPicHdl,&srcRect);
	HUnlock((Handle) srcPicHdl);
	
	//Set the current graphics world to my offscreen mask and draw into it
	SetGWorld((CGrafPtr)offscreenGWorldMask,nil);
	GetPicture(maskPicID);
	maskPicHdl = GetPicture(maskPicID);
	if (maskPicHdl==nil)
		DebugStr("\pthe second call to GetPicture failed");
	EraseRect(&maskRect);
	HLock((Handle)maskPicHdl);
	DrawPicture(maskPicHdl,&maskRect);
	HUnlock((Handle)maskPicHdl);

	//Set the current graphics port to my application window for drawing into
	SetGWorld(GetWindowPort(mainWindow),nil);
	BeginUpdate(mainWindow);

	//Now for the whole point of this. . .call CopyDeepMask
	/*CopyDeepMask(
				(BitMap *) &(offscreenGWorldSource->portPixMap),
				(BitMap *) &(offscreenGWorldMask->portPixMap),
				&(mainWindow->portBits),
				&srcRect,
				&maskRect,
				&destRect,
				srcCopy,
				nil);*/
				
	CopyDeepMask(
				(BitMap *) *(GetPortPixMap(offscreenGWorldSource)),
				(BitMap *) *(GetPortPixMap(offscreenGWorldMask)),
				GetPortBitMapForCopyBits(GetWindowPort(mainWindow)),
				&srcRect,
				&maskRect,
				&destRect,
				srcCopy,
				nil);
				
	EndUpdate(mainWindow);
	
	//Unlock the offscreen buffer
	UnlockPixels(srcPMHdl);
	UnlockPixels(maskPMHdl);
	
	//Restore the saved port and gdevice
	SetGWorld(currGWorldPort,currGDevice);
		
	//Dispose of the gWorlds
	DisposeGWorld(offscreenGWorldSource);
	DisposeGWorld(offscreenGWorldMask);
	SetPort(oldPort);

}