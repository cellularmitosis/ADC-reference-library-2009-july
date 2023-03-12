/*
	File:		main.h
	
	Description:Main Program for the QTMP3Player sample.
				The QTMP3Player sample shows how to make a "one-page" MP3 player using QuickTime.
                Note that this isn't the most efficient way to play MP3s.  For more in-depth information,
                see the other DTS sample MP3Player that shows how to use QuickTime and the Carbon
                Sound Manager to play VBR MP3 files.
                                
	Author:		DH

	Copyright: 	© Copyright 2001 Apple Computer, Inc. All rights reserved.
	
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
	
		6/22/2001		DH		First release of WWDC 2001 sample code

*/


#include <Carbon/Carbon.h>
#include <Quicktime/Quicktime.h>

#include "main.h"

void Initialize(void);	/* function prototypes */
void EventLoop(void);
void MakeWindow(void);
void MakeMenu(void);
void DoEvent(EventRecord *event);
void DoMenuCommand(long menuResult);
void DoAboutBox(void);
void DrawWindow(WindowRef window);
static OSErr QuitAppleEventHandler(const AppleEvent *appleEvt, AppleEvent* reply, UInt32 refcon);
void PlaySound ( FSSpec* fileToPlay );
static OSErr GetSoundToPlay(FSSpec *fileToPlay);

static pascal OSErr HandleOApp(const AppleEvent *theAppleEvent, AppleEvent *reply, long handlerRefcon);
static pascal OSErr HandlePDoc(const AppleEvent *theAppleEvent, AppleEvent *reply, long handlerRefcon);
static pascal OSErr HandleQuit(const AppleEvent *theAppleEvent, AppleEvent *reply, long handlerRefcon);
static pascal OSErr HandleODoc(const AppleEvent *theAppleEvent, AppleEvent *reply, long handlerRefcon);

Boolean		gQuitFlag;	/* global */
Movie		gMovie;
WindowRef 	gWindow;
		



static pascal OSErr HandleOApp(const AppleEvent *theAppleEvent, AppleEvent *reply, long handlerRefcon)
{
#pragma unused(theAppleEvent, reply, handlerRefcon)

	return noErr;	// We're up and running
}

static pascal OSErr HandlePDoc(const AppleEvent *theAppleEvent, AppleEvent *reply, long handlerRefcon)
{
#pragma unused(theAppleEvent, reply, handlerRefcon)

	return noErr;
}

static pascal OSErr HandleQuit(const AppleEvent *theAppleEvent, AppleEvent *reply, long handlerRefcon)
{
#pragma unused(theAppleEvent, reply, handlerRefcon)

	gQuitFlag = true;
	return noErr;
}

static pascal OSErr HandleODoc(const AppleEvent *theAppleEvent, AppleEvent *reply, long handlerRefcon)
{
#pragma unused(reply, handlerRefcon)

	AEDescList	docList;
	long		index = 1, itemsInList;
	Size		actualSize;
	AEKeyword	keywd;
	DescType	returnedType;
	
	OSErr		err;

	err = AEGetParamDesc(theAppleEvent, keyDirectObject, typeAEList, &docList);
	if (err == noErr) {
		err = AECountItems(&docList, &itemsInList);
	}

	if (err == noErr) {
		FSSpecPtr fileSpecPtr;

		do {
			fileSpecPtr = (FSSpecPtr)NewPtr(sizeof(FSSpec));
			err = MemError ();

			if (err == noErr) {
				err = AEGetNthPtr(&docList, index, typeFSS, &keywd, &returnedType, fileSpecPtr, sizeof(FSSpec), &actualSize);
			}

			if (err == noErr) {
				HParamBlockRec pb;

				pb.fileParam.ioCompletion = NULL;
				pb.fileParam.ioNamePtr = fileSpecPtr->name;
				pb.fileParam.ioVRefNum = fileSpecPtr->vRefNum;
				pb.fileParam.ioDirID = fileSpecPtr->parID;
				pb.fileParam.ioFDirIndex = 0;

				err = PBHGetFInfoSync(&pb);
				if (err == noErr && pb.fileParam.ioFlFndrInfo.fdType != kPreferencesFolderType) {
					PlaySound(fileSpecPtr);	// get to the meat of the issue, play the damn file already!
					DisposePtr((Ptr)fileSpecPtr);
				}
			}

			index++;
		} while (err == noErr);

		// The last time through the loop we allocate a pointer we don't need.
		DisposePtr((Ptr)fileSpecPtr);
	}

	AEDisposeDesc(&docList);

	return err;
}

static OSErr InstallRequiredAppleEvents(void)
{
	OSErr err;

	err = AEInstallEventHandler(kCoreEventClass, kAEOpenApplication, NewAEEventHandlerUPP(HandleOApp), 0, false);

	if (err == noErr)
		err = AEInstallEventHandler(kCoreEventClass, kAEOpenDocuments, NewAEEventHandlerUPP(HandleODoc), 0, false);

	if (err == noErr)
		err = AEInstallEventHandler(kCoreEventClass, kAEPrintDocuments, NewAEEventHandlerUPP(HandlePDoc), 0, false);

	if (err == noErr)
		err = AEInstallEventHandler(kCoreEventClass, kAEQuitApplication, NewAEEventHandlerUPP(HandleQuit), 0, false);

	return err;
}



void PlaySound ( FSSpec* fileToPlay )
{
	short		myRefNum;
	
	// elicit a file from the user
    if ( fileToPlay != NULL )
    {
		OpenMovieFile(fileToPlay, &myRefNum, fsRdPerm);
		NewMovieFromFile(&gMovie, myRefNum, NULL, (StringPtr)NULL, newMovieActive, NULL);
		if (myRefNum != 0)
			CloseMovieFile(myRefNum);
			
		// play the movie once thru
		StartMovie(gMovie);
	}
}



static OSErr GetSoundToPlay(FSSpec *fileToPlay)
{	
	OSErr err = noErr;

#ifndef TARGET_API_MAC_CARBON
	if (NavServicesAvailable() == true) {
#else
#pragma unused(fileToPlay)
#endif
		NavReplyRecord		navReply;
		NavDialogOptions	dialogOptions;

		err = NavGetDefaultDialogOptions(&dialogOptions);
		if (err == noErr) {
			dialogOptions.dialogOptionFlags = kNavAllFilesInPopup;
			dialogOptions.dialogOptionFlags |= kNavAllowMultipleFiles;
		}

		if (err == noErr) {
			err = NavGetFile(NULL, &navReply, &dialogOptions, NULL, NULL, NULL, NULL, NULL);
		}

		if (navReply.validRecord && err == noErr) {
			ProcessSerialNumber processSN = {0, kCurrentProcess};
			AEAddressDesc		targetAddress = {typeNull, NULL};
			AppleEvent			theODOC = {typeNull, NULL},
								theReply = {typeNull, NULL};

			// Create an Apple Event to ourselves.
			err = AECreateDesc(typeProcessSerialNumber, &processSN, sizeof(ProcessSerialNumber), &targetAddress);

			if (err == noErr) {
				// Create the open document event.
				err = AECreateAppleEvent(kCoreEventClass, kAEOpenDocuments, &targetAddress, kAutoGenerateReturnID, kAnyTransactionID, &theODOC);
				AEDisposeDesc(&targetAddress);
			}

			if (err == noErr) {
				// Put the list of files into the open document event Apple Event.
				err = AEPutParamDesc(&theODOC, keyDirectObject, &(navReply.selection));
			}

			if (err == noErr) {
				// Send the open document event to ourselves.
				err = AESend(&theODOC, &theReply, kAENoReply, kAENormalPriority, kAEDefaultTimeout, NULL, NULL);
				AEDisposeDesc(&theODOC);
				AEDisposeDesc(&theReply);
			}

		}
		
		NavDisposeReply (&navReply);

#ifndef TARGET_API_MAC_CARBON		
	} else {
		SFTypeList			typeList = {'AIFF', 'AIFC', 0, 0};
		StandardFileReply	sfReply;
		
		StandardGetFile(nil, 2, typeList, &sfReply);

		if (sfReply.sfGood == true) {
			*fileToPlay = sfReply.sfFile;
			err = PlaySound(fileToPlay);
		} else {
			err = userCanceledErr;
		}
	}
#endif

	return err;
}


int main(int argc, char *argv[])
{
	Initialize();
    InstallRequiredAppleEvents();
    MakeWindow();
	MakeMenu();
    
    GetSoundToPlay( NULL );
    
	EventLoop();

	return 0;
}
 
void Initialize()	/* Initialize some managers */
{
    OSErr	err;
        
    InitCursor();

    err = AEInstallEventHandler( kCoreEventClass, kAEQuitApplication, NewAEEventHandlerUPP((AEEventHandlerProcPtr)QuitAppleEventHandler), 0, false );
    if (err != noErr)
        ExitToShell();
}

static OSErr QuitAppleEventHandler( const AppleEvent *appleEvt, AppleEvent* reply, UInt32 refcon )
{
    gQuitFlag =  true;
    
    return noErr;
}

void EventLoop()
{
    Boolean	gotEvent;
    EventRecord	event;
        
    gQuitFlag = false;
	
    do
    {
        gotEvent = WaitNextEvent(everyEvent,&event,32767,nil);
        if (gotEvent)
            DoEvent(&event);

		if ( !IsMovieDone(gMovie) )
        {
			MoviesTask(gMovie, 0);
        }
        
    } while (!gQuitFlag);
    
    DisposeMovie(gMovie);

    ExitToShell();					
}

void MakeWindow()	/* Put up a window */
{
    Rect	wRect;
    WindowRef	myWindow;
    
    SetRect(&wRect,50,50,600,200); /* left, top, right, bottom */
    myWindow = NewCWindow(nil, &wRect, "\pHello", true, zoomNoGrow, (WindowRef) -1, true, 0);
    
    if (myWindow != nil)
        SetPort(GetWindowPort(myWindow));  /* set port to new window */
    else
        ExitToShell();	
        
    gWindow = myWindow;
}

void MakeMenu()		/* Put up a menu */
{
    Handle	menuBar;
    MenuRef	menu;
    long	response;
    OSErr	err;
	
    menuBar = GetNewMBar(rMenuBar);	/* read menus into menu bar */
    if ( menuBar != nil )
    {
        SetMenuBar(menuBar);	/* install menus */
        // AppendResMenu(GetMenuHandle(mApple), 'DRVR');
        
        err = Gestalt(gestaltMenuMgrAttr, &response);
	if ((err == noErr) && (response & gestaltMenuMgrAquaLayoutMask))
        {
            menu = GetMenuHandle( mFile );
            DeleteMenuItem( menu, iQuit );
            DeleteMenuItem( menu, iQuitSeparator );
        }
        
        DrawMenuBar();
    }
    else
    	ExitToShell();
}

void DoEvent(EventRecord *event)
{
    short	part;
    Boolean	hit;
    char	key;
    Rect	tempRect;
    WindowRef	whichWindow;
        
    switch (event->what) 
    {
        case mouseDown:
            part = FindWindow(event->where, &whichWindow);
            switch (part)
            {
                case inMenuBar:  /* process a moused menu command */
                    DoMenuCommand(MenuSelect(event->where));
                    break;
                    
                case inSysWindow:
                    break;
                
                case inContent:
                    if (whichWindow != FrontWindow()) 
                        SelectWindow(whichWindow);
                    break;
                
                case inDrag:	/* pass screenBits.bounds */
                    GetRegionBounds(GetGrayRgn(), &tempRect);
                    DragWindow(whichWindow, event->where, &tempRect);
                    break;
                    
                case inGrow:
                    break;
                    
                case inGoAway:
                    DisposeWindow(whichWindow);
                    ExitToShell();
                    break;
                    
                case inZoomIn:
                case inZoomOut:
                    hit = TrackBox(whichWindow, event->where, part);
                    if (hit) 
                    {
                        SetPort(GetWindowPort(whichWindow));   // window must be current port
                        EraseRect(GetWindowPortBounds(whichWindow, &tempRect));   // inval/erase because of ZoomWindow bug
                        ZoomWindow(whichWindow, part, true);
                        InvalWindowRect(whichWindow, GetWindowPortBounds(whichWindow, &tempRect));	
                    }
                    break;
                }
                break;
		
                case keyDown:
		case autoKey:
                    key = event->message & charCodeMask;
                    if (event->modifiers & cmdKey)
                        if (event->what == keyDown)
                            DoMenuCommand(MenuKey(key));
		case activateEvt:	       /* if you needed to do something special */
                    break;
                    
                case updateEvt:
			DrawWindow((WindowRef) event->message);
			break;
                        
                case kHighLevelEvent:
			AEProcessAppleEvent( event );
			break;
		
                case diskEvt:
			break;
	}
}

void DoMenuCommand(long menuResult)
{
    short	menuID;		/* the resource ID of the selected menu */
    short	menuItem;	/* the item number of the selected menu */
	
    menuID = HiWord(menuResult);    /* use macros to get item & menu number */
    menuItem = LoWord(menuResult);
	
    switch (menuID) 
    {
        case mApple:
            switch (menuItem) 
            {
                case iAbout:
                    DoAboutBox();
                    break;
                    
                case iQuit:
                    ExitToShell();
                    break;
				
                default:
                    break;
            }
            break;
        
        case mFile:
            break;
		
        case mEdit:
            break;
    }
    HiliteMenu(0);	/* unhighlight what MenuSelect (or MenuKey) hilited */
}

void DrawWindow(WindowRef window)
{
    Rect		tempRect;
    GrafPtr		curPort;
	
    GetPort(&curPort);
    SetPort(GetWindowPort(window));
    BeginUpdate(window);
    EraseRect(GetWindowPortBounds(window, &tempRect));
    DrawControls(window);
    DrawGrowIcon(window);
    EndUpdate(window);
    SetPort(curPort);
}

void DoAboutBox(void)
{
   (void) Alert(kAboutBox, nil);  // simple alert dialog box
}
