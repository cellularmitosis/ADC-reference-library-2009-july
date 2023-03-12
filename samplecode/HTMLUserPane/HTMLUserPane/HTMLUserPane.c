/*
    File: MLTEUserPane.c
    
    Description:
        This file contains the main application program for the MLTEUserPane
	example.  This application creates a dialog window and installs a scrolling
	text user pane in the dialog.  You will notice that since the implementation
	of these scrolling text fields is based on the user pane control manager
	api, this program makes very few calls for maintaining the edit
	fields.  those calls are made by the control manager. 
	
	Routines in this file are responsible for handling events directed
	at the application.
	
	Where calls are explicitly made to routines defined in the
	file mUPControl.h, I have added comments beginning with
	the phrase:
		Call to mUPControl.h

    Copyright:
        � Copyright 2000 Apple Computer, Inc. All rights reserved.
    
    Disclaimer:
        IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc.
        ("Apple") in consideration of your agreement to the following terms, and your
        use, installation, modification or redistribution of this Apple software
        constitutes acceptance of these terms.  If you do not agree with these terms,
        please do not use, install, modify or redistribute this Apple software.

        In consideration of your agreement to abide by the following terms, and subject
        to these terms, Apple grants you a personal, non-exclusive license, under Apple�s
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
        Fri, Jan 28, 2000 -- created
*/


#include "HTMLControl.h"

#include "HTMLUserPane.h"

#ifdef __APPLE_CC__
#include <Carbon/Carbon.h>
#else
#include <Carbon.h>
#endif

#include <StdIO.h>
#include <StdArg.h>
#include <string.h>
#include <StdDef.h>


Boolean gRunning = true;







typedef struct {
	ControlHandle theItem;
	Rect bounds, ibounds;
} DialogMapItem;

typedef struct {
	Rect bounds;
	long count;
	DialogMapItem itemb[1];
} DialogMapRecord, *DialogMapRecPtr;

static OSStatus NewDialogItemMap(DialogPtr theDialog, DialogMapRecPtr *theItems) {
	long i, n;
	DialogMapRecPtr items;
	DialogMapItem *itemip;
	OSStatus err;
	short itemt;
	Handle itemh;
		/* known state */
	items = NULL;
	SetPortWindowPort(GetDialogWindow(theDialog));
		/* allocate our list */
	n = CountDITL(theDialog);
	items = (DialogMapRecPtr) NewPtr(offsetof(DialogMapRecord, itemb) + sizeof(DialogMapItem) * n);
	if (items == NULL) { err = memFullErr; goto bail; }
		/* bounds */
	GetPortBounds(GetWindowPort(GetDialogWindow(theDialog)), &items->bounds);
		/* count */
	items->count = n;
		/* items */
	for (i=1, itemip = items->itemb; i<=n; itemip++, i++) {
		err = GetDialogItemAsControl( theDialog, i, &itemip->theItem);
		if (err != noErr) goto bail;
		GetControlBounds(itemip->theItem, &itemip->bounds);
		GetDialogItem( theDialog, i, &itemt, &itemh, &itemip->ibounds);
	}
		/* save result */
	*theItems = items;
	return noErr;
bail:
	if (items != NULL) DisposePtr((Ptr) items);
	return err;
}

static void DeleteDialogItemMap(DialogMapRecPtr theItems) {
	DisposePtr((Ptr) theItems);
} 

static OSStatus RemapDialog(DialogPtr theDialog, DialogMapRecPtr theItems) {
	long i;
	DialogMapItem *itemip;
	short itemt;
	Handle itemh;
	Rect newbounds, temp, theemptyrect = {0, 0, 0, 0};
	RgnHandle clipSave;
	
	SetPortWindowPort(GetDialogWindow(theDialog));
	GetPortBounds(GetWindowPort(GetDialogWindow(theDialog)), &newbounds);

	GetClip((clipSave = NewRgn()));
	ClipRect(&theemptyrect);
		/* items */
	for (i=1, itemip = theItems->itemb; i <= theItems->count; itemip++, i++) {
			/* adjust control rectangle */
		temp = itemip->bounds;
		MapRect(&temp, &theItems->bounds, &newbounds);
		MoveControl(itemip->theItem, temp.left, temp.top);
		SizeControl(itemip->theItem, temp.right - temp.left, temp.bottom - temp.top);
			/* adjust item rectangle */
		GetDialogItem( theDialog, i, &itemt, &itemh, &temp);
		temp = itemip->ibounds;
		MapRect(&temp, &theItems->bounds, &newbounds);
		SetDialogItem( theDialog, i, itemt, itemh, &temp);
	}
	SetClip(clipSave);
	DisposeRgn(clipSave);
	EraseRect(&newbounds);
	InvalWindowRect(GetDialogWindow(theDialog), &newbounds);
	
	return noErr;
}




static Boolean CFURLToFSSpec(CFURLRef theURL, FSSpec *theSpec) {
	FSRef fsRef;
	if (CFURLGetFSRef( theURL, &fsRef) )
		if (FSGetCatalogInfo( &fsRef, kFSCatInfoNone, NULL, NULL, theSpec, NULL) == noErr)
			return true;
	return false;
}

		   

typedef struct {
	DialogPtr dialog;
	ControlHandle field1, field2, field3;
	DialogMapRecPtr itemsMap;
} MUDialogRec, *MUDialogRecPtr;

static DialogPtr CreateNewMUDialog(short h, short v, short field1, short field2, short field3) {
	MUDialogRecPtr murec;
	CFURLRef mainp;
	FSSpec  theSpec;
	
		/* create our storage */
	murec = (MUDialogRecPtr) NewPtr(sizeof(MUDialogRec));
	if (murec == NULL) return NULL;
		/* open the dialog window */
	murec->dialog = GetNewDialog(kMainDialogBox, NULL, (WindowPtr)(-1));
		/* get the user pane controls from the dialog record */
	GetDialogItemAsControl(murec->dialog, kEditItemOne, &murec->field1);
	GetDialogItemAsControl(murec->dialog, kEditItemTwo, &murec->field2);
	GetDialogItemAsControl(murec->dialog, kEditItemThree, &murec->field3);
		/* initialize the user pane controls for use as scrolling text fields */
	HTMLOpenControl(murec->field1);		/* Call to mUPControl.h */
	HTMLOpenControl(murec->field2);
	HTMLOpenControl(murec->field3);
		/* get the data */

	mainp = CFBundleCopyResourceURL( CFBundleGetMainBundle(), CFSTR("index"), CFSTR("html"), CFSTR("docs"));
	
	if (CFURLToFSSpec(mainp, &theSpec)) {
		HTMLDisplayFile(murec->field1, &theSpec);
		HTMLDisplayFile(murec->field2, &theSpec);
		HTMLDisplayFile(murec->field3, &theSpec);
	}
	CFRelease(mainp);
		

		/* map the items */
	NewDialogItemMap(murec->dialog, &murec->itemsMap);
		/* save the info */
	SetWRefCon(GetDialogWindow(murec->dialog), (long) murec);
		/* show the window */
	MoveWindow(GetDialogWindow(murec->dialog), h, v, true);
	ShowWindow(GetDialogWindow(murec->dialog));
		/* done */
	return murec->dialog;
}

static void ResizeMUDialog(DialogPtr dialog) {
	MUDialogRecPtr murec;
	murec = (MUDialogRecPtr) GetWRefCon(GetDialogWindow(dialog));
	RemapDialog(murec->dialog, murec->itemsMap);
}


static void CloseMUDialog(DialogPtr dialog) {
	MUDialogRecPtr murec;
	
	murec = (MUDialogRecPtr) GetWRefCon(GetDialogWindow(dialog));
	
	HTMLCloseControl(murec->field1);	/* Call to mUPControl.h */
	HTMLCloseControl(murec->field2);
	HTMLCloseControl(murec->field3);
	DisposeDialog(murec->dialog);
	DeleteDialogItemMap(murec->itemsMap);
	DisposePtr((Ptr) murec);
}






/* DoDialogSelect is called whenever DialogSelect indicates that an event
	has occured in our dialog.  In this routine, we only look for clicks
	in the ok button.  All of the other processing necessary for maintaining
	the appearance and behavior of the scrolling edit fields is taken care of
	by the control manager. */
static void DoDialogSelect(EventRecord *ev, DialogPtr theDialog, short itemHit) {
	if (itemHit == ok) gRunning = false;
}



/* ResetMenus is called immediately before all calls to
	MenuSelect or MenuKey.  In this routine, we re-build
	or enable the menus as appropriate depending on the
	current environment */
static void ResetMenus(void) {
	Boolean doEnableEdit;
	MenuHandle theEditMenu;
	WindowPtr target;
	ControlHandle theFocus;
	static Boolean gEditMenuEnabled = false;
		/* set up locals */
	doEnableEdit = false;
		/* if the frontmost window's keyboard focus is a
		mUP control, then we enable the edit menu. */
	if ((target = FrontWindow()) != NULL)
		if (GetKeyboardFocus(target, &theFocus) == noErr) {
			if (IsHTMLControl(theFocus)) {		/* Call to mUPControl.h */
				doEnableEdit = true;
			}
		}
		/* enable/disable the edit menu and re-draw it
		only if the enable state has changed. */
	if (doEnableEdit != gEditMenuEnabled) {
		gEditMenuEnabled = doEnableEdit;
		theEditMenu = GetMenuHandle(mEdit);
		if (gEditMenuEnabled)
			EnableMenuItem(theEditMenu, 0);
		else DisableMenuItem(theEditMenu, 0);
		DrawMenuBar();
	}
}


/* DoMenuCommand is called in response to MenuKey
	or MenuSelect.  Here, we dispatch the menu command
	to its appropriate handler, or if it's a small action
	we do it here. */
static void DoMenuCommand(long rawMenuSelectResult) {
	short menu, item;
		/* decode the MenuSelect result */
	menu = (rawMenuSelectResult >> 16);
	if (menu == 0) return;
	item = (rawMenuSelectResult & 0x0000FFFF);
		/* dispatch on result */
	switch (menu) {
	
			/* apple menu commands */
		case mApple:
			if (item == iAbout)
				ParamAlert(kAboutBoxAlert, NULL, NULL);
			break;
			
				/* file menu commands */
		case mFile:
			if (item == iQuit)
				gRunning = false;
			break;
			
				/* edit menu commands.  If the current keyboard focus
				in the frontmost window is a mUP control, then we send
				the edit command to the control.  */
		case mEdit:
			{	WindowPtr target;
				ControlHandle theFocus;
					/* get the front window */
				target = FrontWindow();
				if (target != NULL) {
						/* find the focus  and check to see if it's
						a mUP control */
					if (GetKeyboardFocus(target, &theFocus) == noErr) {
						if (IsHTMLControl(theFocus)) {		/* Call to mUPControl.h */
								/* pass the editing command along to the mUP control */
							/* no editing commands at this time */
						}
					}
				}
			}
			break;
	}
		/* unhilite the menu bar */
	HiliteMenu(0);
}


/* HandleMouseDown is called for mouse down events.  Processing of
	mouse down events in the HTML rendering area of windows is 
	handled by the HTMLRenderinLib, but clicks in the controls and
	other parts of the windows are handled here. */
static void HandleMouseDown(EventRecord *ev) {
	WindowPtr theWindow;
	short partcode;
	partcode = FindWindow(ev->where, &theWindow);
	switch (partcode) {
			/* inside the window's content area */
		case inContent:
			if (theWindow != FrontWindow()) {
					/* if it's not the frontmost window,
					then make it the frontmost window. */
				SelectWindow(theWindow);
			} else {
					/* otherwise, if it's a rendering window,
					pass the click along to the window. */
				Point where;
				SetPort(GetWindowPort(theWindow));
				where = ev->where;
				GlobalToLocal(&where);
			}
			break;
			
			/* menu bar clicks */
		case inMenuBar:
			ResetMenus();
			DoMenuCommand(MenuSelect(ev->where));
			break;
			
			/* track clicks in the close box */
		case inGoAway:
			if (TrackGoAway(theWindow, ev->where)) {
				gRunning = false;
			}
			break;
			
			/* allow window drags */
		case inDrag:
			{	Rect boundsRect = {0,0, 32000, 32000};
				DragWindow(theWindow, ev->where, &boundsRect);
			}
			break;
			
			/* allow window drags */
		case inGrow:
			{	Rect sizerect;
				long grow_result;
				SetRect(&sizerect, 300, 150, 32767, 32767);
				grow_result = GrowWindow(theWindow, ev->where, &sizerect);
				if (grow_result != 0) {
					MUDialogRecPtr murec;
					SizeWindow(theWindow, LoWord(grow_result), HiWord(grow_result), true);
					murec = (MUDialogRecPtr) GetWRefCon(theWindow);
					ResizeMUDialog(murec->dialog);
				}
			}
			break;
			
			/* zoom box clicks.  NOTE:  since the rendering window
			always sets the standard rectangle to the 'best size' for
			displaying the current HTML window, the inZoomOut partcode
			will zoom the window to that size rather than the entire screen.*/
		case inZoomIn:
		case inZoomOut:
			if (TrackBox(theWindow, ev->where, partcode)) {
				Rect r;
				CGrafPtr gp;
				gp = GetWindowPort(theWindow);
				GetPortBounds(gp, &r);
				SetPort(gp);
				EraseRect(&r);
				ZoomWindow(theWindow, partcode, true);
			}
			break;
			
	}
}



/* HandleEvent is the main event handling routine for the
	application.  ev points to an event record returned by
	WaitNextEvent. */
void HandleEvent(EventRecord *ev) {
	WindowPtr target;
		/* redraw the edit menu if needed... */
	ResetMenus();
		/* process menu key events */
	if (((ev->what == keyDown) || (ev->what == autoKey)) && ((ev->modifiers & cmdKey) != 0)) {
		ResetMenus();
		DoMenuCommand(MenuKey((char) (ev->message & charCodeMask)));
		ev->what = nullEvent;
	}
	
		/* for dialog events, we simply pass the event along to
		our dialog event handler.  NOTE:  because we defined the editable
		text fields as user pane items, the control manager takes care
		of calling all of the routines necessary for maintaining the controls. */
	if (IsDialogEvent(ev)) {
		DialogPtr theDialog;
		short itemHit;
		if (DialogSelect(ev, &theDialog, &itemHit))
			DoDialogSelect(ev, theDialog, itemHit);
	}

		/* process other event types */
	switch (ev->what) {
	
			/* for null events, track the cursor. */
		case nullEvent:
			break;
			
			/* the application may be switching in to the forground
			or into the background.  Either way, we need to activate
			the frontmost window accordingly. */
		case osEvt:
				
			if (((ev->message >> 24) & 255) == mouseMovedMessage) {
				/* do some cursor checking */
			} else if (((ev->message >> 24) & 255) == suspendResumeMessage) {
				Boolean switchingIn;
				switchingIn = ((ev->message & resumeFlag) != 0);
					/* send an activate event to the frontmost window */
				target = FrontWindow();
			}
			break;
			
			/* for activate events we call the window's activate event
			handler. */
		case activateEvt:
			target = (WindowPtr) ev->message;
			break;
			
			/* for update events we call the window's update event
			handler. if the window is of an unknown type, we ignore the
			event. */
		case updateEvt:
			target = (WindowPtr) ev->message;
			BeginUpdate(target);
			EndUpdate(target);
			break;	
			
			/* for mouse events we call the the HandleMouseDown routine
			defined above. */
		case mouseDown:
			HandleMouseDown(ev);
			break;
		
			/* for key down events we call the window's key down event
			handler. */
		case keyDown:
		case autoKey:
			target = FrontWindow();
			break;
		
			/* Apple events. */
		case kHighLevelEvent:
			AEProcessAppleEvent(ev);
			break;
	}
}


/* MyIdleInteractProc is the idle procedure called by AEInteractWithUser while we are waiting
	for the application to be pulled into the forground.  It simply passes the event along
	to HandleNextEvent */
static pascal Boolean MyIdleInteractProc(EventRecord *theEvent, long *sleepTime, RgnHandle *mouseRgn) {
	HandleEvent(theEvent);
	return ( ! gRunning ); /* quit waiting if we're not running */
}


/* ParamAlert is a general alert handling routine.  If Apple events exist, then it
	calls AEInteractWithUser to ensure the application is in the forground, and then
	it displays an alert after passing the s1 and s2 parameters to ParamText. */
short ParamAlert(short alertID, StringPtr s1, StringPtr s2) {
	AEIdleUPP aeIdleProc;
	OSStatus err;
	aeIdleProc = NewAEIdleUPP(MyIdleInteractProc);
	if (aeIdleProc == NULL) { err = memFullErr; goto bail; }
	err = AEInteractWithUser(kNoTimeOut, NULL, aeIdleProc);
	if (err != noErr) goto bail;
	ParamText(s1, s2, NULL, NULL);
	err = Alert(alertID, NULL);
	DisposeAEIdleUPP(aeIdleProc);
	return err;
bail:
	if (aeIdleProc != NULL) DisposeAEIdleUPP(aeIdleProc);
	return err;
}





/* QuitAppleEventHandler is our quit Apple event handler.  this routine
	is called when a quit Apple event is sent to our application.  Here,
	we set the gRunning flag to false. NOTE:  it is not appropriate to
	call ExitToShell here.  Instead, by setting the flag to false we
	fall through the bottom of our main event loop.  */
static pascal OSErr QuitAppleEventHandler(const AppleEvent *appleEvt, AppleEvent* reply, long refcon) {

	gRunning = false;	
			
	return noErr;
}


	/* the main program */

int main(void) {
	DialogPtr dialoga, dialogb;
		/* set up the cursor, register */
	InitCursor();
	
	TXNInitTextension(NULL,  0, (kTXNWantMoviesMask | kTXNWantSoundMask | kTXNWantGraphicsMask));

		/* install our event handlers */
	AEInstallEventHandler(kCoreEventClass, kAEQuitApplication, NewAEEventHandlerUPP(QuitAppleEventHandler), 0, false);

		/* set up the menu bar */
	SetMenuBar(GetNewMBar(kMenuBarID));
	DrawMenuBar();

	dialoga = CreateNewMUDialog(100, 100, 128, 129, 130);
	dialogb = CreateNewMUDialog(200, 200, 131, 132, 133);
	
		/* loop processing events until.... */
	do {	EventRecord ev;
			/* process the next event */
		if ( ! WaitNextEvent(everyEvent, &ev, GetCaretTime(), NULL) ) ev.what = nullEvent;
		HandleEvent(&ev);
	} while (gRunning);
		/* free up our scrolling text items.  NOTE:  we only clean up
		the stuff we allocated.  We don't actually dispose of the user
		pane controls themselves because they are owned by the
		dialog manager. */
	CloseMUDialog(dialoga);
	CloseMUDialog(dialogb);
		/* done */
	ExitToShell();
	return 0;
}