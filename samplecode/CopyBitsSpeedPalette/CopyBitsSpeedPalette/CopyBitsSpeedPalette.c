/*
	File:		CopyBitsSpeedPalette.c

	Contains:	This program demostrates ways to increase copybits speed when using palettes.	

	Written by: JW	

	Copyright:	Copyright © 1991-1999 by Apple Computer, Inc., All Rights Reserved.

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
				

*/


#ifdef __MWERKS__

// includes for MetroWerks CodeWarrior

#include	"CarbonPrefix.h"
#include 	<Dialogs.h>
#include	<Resources.h>
#include	<Menus.h>
#include	<Devices.h>
#include	<ToolUtils.h>
//#include 	<GestaltEqu.h>
#include	<Gestalt.h>
#include	<Palettes.h>
#include	<QDOffscreen.h>
#include 	<Processes.h>
#include	<OSUtils.h>

#else

#ifdef __APPLE_CC__

// includes for ProjectBuilder

#include <Carbon/Carbon.h>

#else

// includes for MPW

#include <Carbon.h>

#endif
#endif

#define Gestalttest		0xA1AD
#define NoTrap			0xA89F

#define	appleID			128			
#define	appleMenu		0
#define	aboutMeCommand	1

#define	fileID			129
#define	quitCommand 	1

#define	PictID			128
#define	clutID			150

#define	aboutMeDLOG		128
#define	okButton		1

#ifndef topLeft
#define topLeft(r)              		(((Point *) &(r))[0])
#endif

#ifndef botRight
#define botRight(r)             		(((Point *) &(r))[1])
#endif

/*------------------------------------------------------*/
/*	Global Variables.									*/
/*------------------------------------------------------*/

Rect				TotalRect, minRect, WinMinusScroll, InitWindowSize;
WindowPtr			myWindow;
CTabHandle			mycolors;
PaletteHandle		srcPalette;
Boolean				DoneFlag;
MenuHandle			mymenu0, mymenu1;
PicHandle			ThePict;
GWorldPtr			offscreenGWorld;
//enum PointSelector {topLeft, botRight};

void draw();
void doCommand(long mResult);
void showAboutMeDialog();
void init();


/*------------------------------------------------------*/
/*	showAboutMeDialog().								*/
/*------------------------------------------------------*/

void showAboutMeDialog()
{
	GrafPtr 	savePort;
	DialogPtr	theDialog;
	short		itemHit;

	GetPort(&savePort);
	theDialog = GetNewDialog(aboutMeDLOG, nil, (WindowPtr) -1);
	//SetPort(theDialog);
	SetPortDialogPort(theDialog);

	do {
		ModalDialog(nil, &itemHit);
	} while (itemHit != okButton);

	//CloseDialog(theDialog);
	DisposeDialog(theDialog);

	SetPort(savePort);
	return;
}

/*------------------------------------------------------*/
/*	init().												*/
/*------------------------------------------------------*/

void init()
{
	RgnHandle			tempRgn;
	Rect				BaseRect;
	OSErr				err;
	long				QDfeature/*, OSfeature*/;
	GDHandle			SaveGD;
	CGrafPtr			SavePort;

	/*	Initialize Managaer.	*/
	//InitGraf(&qd.thePort);
	//InitWindows();
	//InitDialogs(nil);
	InitCursor();
	FlushEvents(everyEvent, 0);
	
	/*	Set up menus.	*/
	mymenu0 = GetMenu(appleID);
	//AppendResMenu(mymenu0, 'DRVR');
	InsertMenu(mymenu0,0);
	mymenu1 = GetMenu(fileID);
	InsertMenu(mymenu1,0);
	DrawMenuBar();
	DoneFlag = false;
	ThePict = GetPicture(PictID);
	if (ThePict == nil)
		DoneFlag = true;

	/*	Use Gestalt to find is QuickDraw is avaiable.	*/
	/*if ((GetOSTrapAddress(Gestalttest) != GetOSTrapAddress(NoTrap))) {
		err = Gestalt(gestaltQuickdrawVersion, &QDfeature);
		if (err)
			DoneFlag = true;
		err = Gestalt(gestaltSystemVersion, &OSfeature);
		if (err)
			DoneFlag = true;
		if (!DoneFlag && (QDfeature & 0x0f00) != 0x0200 && OSfeature < 0x0605)
			DoneFlag = true;
		}
	else
		DoneFlag = true;*/
	
	err = Gestalt(gestaltQuickdrawVersion, &QDfeature);
	if (err != noErr || QDfeature < gestalt32BitQD)
		DoneFlag = true;

	/*	Set Rects.	*/
	SetRect(&BaseRect, 40, 60, 472, 282);
	SetRect(&WinMinusScroll, BaseRect.left-40, BaseRect.top-60, BaseRect.right-60, 
				BaseRect.bottom - 80);
	SetRect(&InitWindowSize, WinMinusScroll.left, WinMinusScroll.top, 
							WinMinusScroll.right, WinMinusScroll.bottom);
	tempRgn = GetGrayRgn();
	HLock ((Handle) tempRgn);
	//TotalRect = (**tempRgn).rgnBBox;
	GetRegionBounds(tempRgn, &TotalRect);
	/*SetRect(&minRect, 80, 80, (**tempRgn).rgnBBox.right - 40, 
				(**tempRgn).rgnBBox.bottom - 40);*/
	SetRect(&minRect, 80, 80, TotalRect.right - 40, TotalRect.bottom - 40);
	HUnlock ((Handle) tempRgn);

	/*	Open window and set up picture.	*/
	GetGWorld (&SavePort, &SaveGD);
	mycolors = GetCTable (clutID);
	(*mycolors)->ctFlags |= 0x4000;

	myWindow = NewCWindow(nil, &BaseRect, (ConstStr255Param)"", true, zoomDocProc, 
							(WindowPtr) -1, true, 150);
	SetGWorld(GetWindowPort(myWindow), SaveGD);
	DrawGrowIcon (myWindow);

	srcPalette = NewPalette (((**mycolors).ctSize)+1, mycolors,
			pmTolerant + pmExplicit + pmAnimated, 0);
	SetPalette ((WindowPtr) myWindow, srcPalette, true);
	
	GetGWorld (&SavePort, &SaveGD);
	err = NewGWorld (&offscreenGWorld, 8, &InitWindowSize, mycolors, nil, 0);
	if (err)
		Debugger();
	SetGWorld (offscreenGWorld, nil);
	EraseRect (&InitWindowSize);
	DrawPicture (ThePict, &InitWindowSize);
	SetGWorld (SavePort, SaveGD);
}

/*------------------------------------------------------*/
/*	doCommand().										*/
/*------------------------------------------------------*/

void doCommand(mResult)
	long	mResult;
{
	int 					theMenu, theItem;
	//Str255					daName;
	//GrafPtr 				savePort;

	theItem = LoWord(mResult);
	theMenu = HiWord(mResult);
	
	switch (theMenu) {
		case appleID:
			if (theItem == aboutMeCommand)
				showAboutMeDialog();
			else {
				/*GetMenuItemText(mymenu0, theItem, daName);
				GetPort(&savePort);
				(void) OpenDeskAcc((ConstStr255Param)daName);
				SetPort(savePort);*/
			}
			break;

		case fileID:
			switch (theItem) {
				case quitCommand:
					DoneFlag = true;
					break;
				default:
					break;
				}
			break;
	}
	HiliteMenu(0);
	return;
}

/*------------------------------------------------------*/
/*	draw.												*/
/*------------------------------------------------------*/

void draw()
{
	RGBColor		black = {0,0,0}, White = {65535,65535,65535};
	long			before, delay;
	//char theString[255];
	GDHandle		screensDevice;
	Rect			area;
		
	RGBForeColor (&black);
	RGBBackColor (&White);
	
	SetPortWindowPort(myWindow);
	
	EraseRect(&WinMinusScroll);
	
	/* This is the only change made to support a faster copybits on one screen.
		ctFlags is still set above. */
	area = WinMinusScroll;
	/*LocalToGlobal(topLeft);
	LocalToGlobal((Point*)botRight);*/
	LocalToGlobal(&topLeft(area));
	LocalToGlobal(&botRight(area));
	screensDevice = GetMaxDevice(&area);
	if (screensDevice != nil)
		//(**(**offscreenGWorld->portPixMap).pmTable).ctSeed = (**(**(**screensDevice).gdPMap).pmTable).ctSeed;
		(**(**(GetPortPixMap(offscreenGWorld))).pmTable).ctSeed = (**(**(**screensDevice).gdPMap).pmTable).ctSeed;
	
	/*CopyBits ((BitMap *) *offscreenGWorld->portPixMap, &myWindow->portBits, 
					&InitWindowSize, &WinMinusScroll, srcCopy, nil);*/
	CopyBits ((BitMap *) *(GetPortPixMap(offscreenGWorld)), GetPortBitMapForCopyBits(GetWindowPort(myWindow)), 
					&InitWindowSize, &WinMinusScroll, srcCopy, nil);
	EraseRect(&WinMinusScroll);
	/*CopyBits ((BitMap *) *offscreenGWorld->portPixMap, &myWindow->portBits, 
					&InitWindowSize, &WinMinusScroll, srcCopy, nil);*/
	CopyBits ((BitMap *) *(GetPortPixMap(offscreenGWorld)), GetPortBitMapForCopyBits(GetWindowPort(myWindow)), 
					&InitWindowSize, &WinMinusScroll, srcCopy, nil);
	EraseRect(&WinMinusScroll);
	/*CopyBits ((BitMap *) *offscreenGWorld->portPixMap, &myWindow->portBits, 
					&InitWindowSize, &WinMinusScroll, srcCopy, nil);*/
	CopyBits ((BitMap *) *(GetPortPixMap(offscreenGWorld)), GetPortBitMapForCopyBits(GetWindowPort(myWindow)), 
					&InitWindowSize, &WinMinusScroll, srcCopy, nil);
	EraseRect(&WinMinusScroll);
	/*CopyBits ((BitMap *) *offscreenGWorld->portPixMap, &myWindow->portBits, 
					&InitWindowSize, &WinMinusScroll, srcCopy, nil);*/
	CopyBits ((BitMap *) *(GetPortPixMap(offscreenGWorld)), GetPortBitMapForCopyBits(GetWindowPort(myWindow)), 
					&InitWindowSize, &WinMinusScroll, srcCopy, nil);
	EraseRect(&WinMinusScroll);
	before = TickCount();
	/*CopyBits ((BitMap *) *offscreenGWorld->portPixMap, &myWindow->portBits, 
					&InitWindowSize, &WinMinusScroll, srcCopy, nil);*/
	CopyBits ((BitMap *) *(GetPortPixMap(offscreenGWorld)), GetPortBitMapForCopyBits(GetWindowPort(myWindow)), 
					&InitWindowSize, &WinMinusScroll, srcCopy, nil);
	delay = TickCount() - before;
/*	DebugStr("\PIt's me!!!");	*/
}

/*------------------------------------------------------*/
/*	main().												*/
/*------------------------------------------------------*/

main()
{
	char			key;
	Boolean			track;
	long			growResult;
	EventRecord 	myEvent;
	WindowPtr		whichWindow;
	int				yieldTime;
	Rect			tempRect1;


	init();
	yieldTime = 0;
	for ( ;; ) {
		if (DoneFlag)
			ExitToShell();
			
		if (WaitNextEvent(everyEvent, &myEvent, yieldTime, nil)) {
			switch (myEvent.what) {
				case mouseDown:
					switch (FindWindow(myEvent.where, &whichWindow)) {
						case inSysWindow:
							//SystemClick(&myEvent, whichWindow);
							break;
						case inMenuBar:
							doCommand(MenuSelect(myEvent.where));
							break;
						case inContent:
							break;
						case inDrag:
							DragWindow (whichWindow, myEvent.where, &TotalRect);
							draw();
							DrawGrowIcon (whichWindow);
							break;
						case inGrow:
							growResult = GrowWindow (whichWindow, myEvent.where,
													&minRect);
							SizeWindow(whichWindow, LoWord(growResult), 
									HiWord(growResult), true);
							/*EraseRect(&whichWindow->portRect);
							SetRect(&WinMinusScroll, whichWindow->portRect.left, 
									whichWindow->portRect.top, 
									whichWindow->portRect.right-20, 
									whichWindow->portRect.bottom - 20);*/
							EraseRect(GetPortBounds(GetWindowPort(whichWindow), &tempRect1));
							SetRect(&WinMinusScroll, tempRect1.left, 
									tempRect1.top, 
									tempRect1.right-20, 
									tempRect1.bottom - 20);
							draw();
							DrawGrowIcon (whichWindow);
							break;
						case inGoAway:
							track = TrackGoAway (whichWindow, myEvent.where);
							if (track) {
								//CloseWindow (whichWindow);
								DoneFlag = true;
								}
							break;
						case inZoomIn:
							track = TrackBox (whichWindow, myEvent.where, inZoomIn);
							if (track) {
								ZoomWindow (whichWindow, inZoomIn, true);
								/*EraseRect(&whichWindow->portRect);
								SetRect(&WinMinusScroll, whichWindow->portRect.left, 
										whichWindow->portRect.top, 
										whichWindow->portRect.right-20, 
										whichWindow->portRect.bottom - 20);*/
								EraseRect(GetPortBounds(GetWindowPort(whichWindow), &tempRect1));
								SetRect(&WinMinusScroll, tempRect1.left, 
										tempRect1.top, 
										tempRect1.right-20, 
										tempRect1.bottom - 20);
								draw();
								DrawGrowIcon (whichWindow);
								}
							break;
						case inZoomOut:
							track = TrackBox (whichWindow, myEvent.where, inZoomOut);
							if (track) {
								ZoomWindow (whichWindow, inZoomOut, true);
								/*EraseRect(&whichWindow->portRect);
								SetRect(&WinMinusScroll, whichWindow->portRect.left, 
										whichWindow->portRect.top, 
										whichWindow->portRect.right-20, 
										whichWindow->portRect.bottom - 20);*/
								EraseRect(GetPortBounds(GetWindowPort(whichWindow), &tempRect1));
								SetRect(&WinMinusScroll, tempRect1.left, 
										tempRect1.top, 
										tempRect1.right-20, 
										tempRect1.bottom - 20);
								draw();
								DrawGrowIcon (whichWindow);
								}
							break;
						default:
							break;
						}
					break;
				case keyDown:
				case autoKey:
					key = myEvent.message & charCodeMask;
					if ( myEvent.modifiers & cmdKey )
						if ( myEvent.what == keyDown )
							doCommand(MenuKey(key));
					break;
				case updateEvt:
					if ((WindowPtr) myEvent.message == myWindow) {
						BeginUpdate((WindowPtr) myWindow);
						EndUpdate((WindowPtr) myWindow);
						draw();
						}
					break;
				case diskEvt:
					break;
				case activateEvt:
					break;
				case 15:
					if ((myEvent.message << 31) == 0) {
						yieldTime = 30;
						}
					else {
						yieldTime = 0;
						//SetPort((WindowPtr) myWindow);
						SetPortWindowPort(myWindow);
						}
					break; 
				default:
					break;
				}
			}
		}
}