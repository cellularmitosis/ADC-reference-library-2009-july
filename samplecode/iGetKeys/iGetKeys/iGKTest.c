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
        © Copyright 2000 Apple Computer, Inc. All rights reserved.
    
    Disclaimer:
        IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc.
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
        Fri, Jan 28, 2000 -- created
*/



#ifdef __APPLE_CC__
#include <Carbon/Carbon.h>
#else
#include <Carbon.h>
#endif

#include "iGKTest.h"
#include "iGetKeys.h"
#include "MapDialog.h"


Boolean gRunning = true;
Boolean gTesting = false;

typedef struct {
	short itemNo;
	short keycode;
	short state;
	ControlHandle itemControl;
	char* name;
	short namelen;
} ItemKeyCode;

ItemKeyCode gItemKeyCheckboxes[] = {
	{ 4, kVirtualCapsLockKey, 0, NULL, "Caps Lock", 9},
	{ 5, kVirtualShiftKey, 0, NULL, "Shift", 5},
	{ 6, kVirtualControlKey, 0, NULL, "Control", 7},
	{ 7, kVirtualOptionKey, 0, NULL, "Option", 6},
	{ 8, kVirtualCommandKey, 0, NULL, "Command", 7},
	{ 9, kVirtualHelpKey, 0, NULL, "Help", 4},
	{ 10, kVirtualDeleteKey, 0, NULL, "Delete", 6},
	{ 11, kVirtualTabKey, 0, NULL, "Tab", 3},
	{ 12, kVirtualEnterKey, 0, NULL, "Enter", 5},
	{ 13, kVirtualReturnKey, 0, NULL, "Return", 6},
	{ 14, kVirtualEscapeKey, 0, NULL, "Escape", 6},
	{ 15, kVirtualForwardDeleteKey, 0, NULL, "FwdDelete", 6},
	{ 16, kVirtualHomeKey, 0, NULL, "Home", 4},
	{ 17, kVirtualEndKey, 0, NULL, "End", 3},
	{ 18, kVirtualPageUpKey, 0, NULL, "Page Up", 7},
	{ 19, kVirtualPageDownKey, 0, NULL, "Page Down", 9},
	{ 20, kVirtualLeftArrowKey, 0, NULL, "Arrow Left", 10},
	{ 21, kVirtualRightArrowKey, 0, NULL, "Arrow Right", 11},
	{ 22, kVirtualUpArrowKey, 0, NULL, "Arrow Up", 8},
	{ 23, kVirtualDownArrowKey, 0, NULL, "Arrow Down", 10}
};
enum {
	kGKCharacterListItem = 3,
	kNumCheckBoxes = (sizeof(gItemKeyCheckboxes)/sizeof(ItemKeyCode))
};

DialogPtr gGKDialog = NULL;
DialogMapRecPtr gItemMap = NULL;



enum {
	kTestDialogID = 130,
	kCancelTestItem = 1,
	kTestListItem = 2
};
DialogPtr gTestDialog = NULL;
DialogMapRecPtr gTestMap = NULL;

void ProcessNextEvent(void);

typedef struct {
	Boolean asciiEntry;
	Boolean state;
	short code;
} KeyEntryTable;

static void BeginTestingSequence(void) {
	ItemKeyCode *itemp;
	short itemt, i;
	Handle itemh;
	Rect itemb;
	Str255 s;
	Size outActualSize;
	ControlHandle listbox;
	Ascii2KeyCodeTable ttable;
	Cell theCell;
	ListHandle gTestList = NULL;
	Boolean isDown, product, changed, tableChanged;
	
	KeyEntryTable keyTable[64], *entryp;
	short keyTableSize;
	
		/* get the test dialog, separate the list control */
	gTestDialog = GetNewDialog(kTestDialogID, NULL, (WindowPtr)(-1));
	NewDialogItemMap(gTestDialog, &gTestMap);
	GetDialogItemAsControl(gTestDialog, kTestListItem, &listbox);
	GetControlData(listbox, kControlEntireControl, kControlListBoxListHandleTag, sizeof(gTestList), &gTestList, &outActualSize);
	SetListSelectionFlags(gTestList, 0);
	
		/* list of ascii codes to test */
	GetDialogItem( gGKDialog, kGKCharacterListItem, &itemt, &itemh, &itemb );
	GetDialogItemText( itemh, s );
	
		/* set up the ascii translation table */
	InitAscii2KeyCodeTable( &ttable );

		/* set up our local translation table */
	keyTableSize = 0;
	entryp = keyTable;
		/* gather letters typed in table */
	for (i=0; i<s[0]; i++) {
			/* display in list */
		SetPt(&theCell, 0, LAddRow(1, 3000, gTestList));
		LSetCell(&s[i+1], 1, theCell, gTestList);
			/* add new key table entry */
		entryp->asciiEntry = true;
		entryp->code = s[i+1];
		entryp->state = false;
		keyTableSize++;
		entryp++;
	}
		/* gather checkboxes into table */
	for (i=0, itemp=gItemKeyCheckboxes; i<kNumCheckBoxes; itemp++, i++)
		if (itemp->state) {
				/* display in list */
			SetPt(&theCell, 0, LAddRow(1, 3000, gTestList));
			LSetCell(itemp->name, itemp->namelen, theCell, gTestList);
				/* add new key table entry */
			entryp->asciiEntry = false;
			entryp->code = itemp->keycode;
			entryp->state = false;
			keyTableSize++;
			entryp++;
		}
		/* display the window */
	ShowWindow(GetDialogWindow(gTestDialog));
		/* while the window is visible, re-display the keys */
	gTesting = true;
	while (gTesting) {
			/* validate the ascii translation table */
		ValidateAscii2KeyCodeTable( &ttable, &tableChanged);
			/* set the drawing environment */
		SetPortWindowPort(GetDialogWindow(gTestDialog));
		product = true;
		changed = false;
		for (i = 0, entryp=keyTable; i < keyTableSize; entryp++, i++) {
				/* search the table for key transitions */
			if (entryp->asciiEntry)
				isDown = TestForAsciiKeyDown( &ttable, entryp->code );
			else isDown = TestForKeyDown(entryp->code);
			if (isDown != entryp->state) {
				entryp->state = isDown;
				SetPt(&theCell, 0, i);
				LSetSelect(isDown, theCell, gTestList);
				changed = true;
			}
			if ( ! isDown ) product = false;
		}
			/* redraw the list if it changed */
		if (changed) Draw1Control(listbox);
			/* beep and exit if all the keys are down */
		if ( product ) {
			SysBeep(1);
			gTesting = false;
		}
			/* process the next event */
		ProcessNextEvent();
	}
		/* close and return to the caller */
	DisposeDialog(gTestDialog);
	gTestDialog = NULL;
	gTestList = NULL;
}


static void GKKeyTestDialog(EventRecord *ev, DialogPtr theDialog, short itemHit) {
	if (itemHit == kCancelTestItem)
		gTesting = false;
}





static void GKConfigDialog(EventRecord *ev, DialogPtr theDialog, short itemHit) {
	long i;
	ItemKeyCode *itemp;
		/* check box clicks handled first */
	for (i=0, itemp=gItemKeyCheckboxes; i<kNumCheckBoxes; itemp++, i++)
		if (itemp->itemNo == itemHit) {
			itemp->state = ( (itemp->state + 1) & 1 );
			SetControlValue(itemp->itemControl, itemp->state);
			return;
		}
		/* button clicks */
	if (itemHit == 1) {
		gRunning = false;
	} else if (itemHit == 2) {
		BeginTestingSequence();
	}
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


void ProcessNextEvent(void) {
	EventRecord ev;
	DialogPtr theDialog;
	WindowPtr theWindow;
	short itemHit, partCode;
		/* get the next event */
	if ( ! WaitNextEvent(everyEvent, &ev, GetCaretTime(), NULL) )
		ev.what = nullEvent;
		/* mouse events */
	if ( ev.what == mouseDown ) {
		partCode = FindWindow(ev.where, &theWindow);
		switch (partCode) {
			case inGrow:
				if (theWindow == GetDialogWindow(gGKDialog)) {
					GrowMappedDialog(ev.where, gGKDialog, gItemMap);
				} else if (theWindow == GetDialogWindow(gTestDialog)) {
					GrowMappedDialog(ev.where, gTestDialog, gTestMap);
				}
				break;
			case inGoAway:
				if (theWindow == GetDialogWindow(gGKDialog)) {
					if (TrackGoAway(theWindow, ev.where))
						gRunning = false;
				}
				break;
			case inDrag:
				{	Rect boundsRect = {0,0, 32000, 32000};
					DragWindow(theWindow, ev.where, &boundsRect);
				}
				break;
		}
		if (partCode == inGrow || partCode == inGoAway || partCode == inDrag)
			ev.what = nullEvent;
	}
		/* apple events */
	if ( ev.what == kHighLevelEvent ) {
		AEProcessAppleEvent(&ev);
		ev.what = nullEvent;
	}
		/* key downs during testing... */
	if (gTesting && (ev.what == keyDown || ev.what == autoKey))
		ev.what = nullEvent;
	
		/* handle dialog events */
	if (IsDialogEvent(&ev))
		if ( DialogSelect(&ev, &theDialog, &itemHit))
			if (theDialog == gGKDialog)
				GKConfigDialog(&ev, theDialog, itemHit);
			else if (theDialog == gTestDialog)
				GKKeyTestDialog(&ev, theDialog, itemHit);
}

	/* the main program */
int main(void) {
	long i;
	ItemKeyCode *itemp;
		/* set up */
	InitCursor();
	AEInstallEventHandler(kCoreEventClass, kAEQuitApplication, NewAEEventHandlerUPP(QuitAppleEventHandler), 0, false);
	
		/* set up the menu bar */
	SetMenuBar(GetNewMBar(kMenuBarID));
	DrawMenuBar();

		/* open the dialog window */
	gGKDialog = GetNewDialog(kMainDialogBox, NULL, (WindowPtr)(-1));
	NewDialogItemMap(gGKDialog, &gItemMap);
	for (i=0, itemp=gItemKeyCheckboxes; i<kNumCheckBoxes; itemp++, i++) {
		GetDialogItemAsControl(gGKDialog, itemp->itemNo, &itemp->itemControl);
	}
	ShowWindow(GetDialogWindow(gGKDialog));

		/* loop processing events until.... */
	while ( gRunning ) 
		ProcessNextEvent();
		/* close the dialog. */
	DisposeDialog(gGKDialog);
	DeleteDialogItemMap(gItemMap);
		/* done */
	ExitToShell();
	return 0;
}