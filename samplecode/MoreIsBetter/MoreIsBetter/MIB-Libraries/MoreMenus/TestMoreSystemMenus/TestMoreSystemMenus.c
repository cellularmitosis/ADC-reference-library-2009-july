/*
	File:		TestMoreSystemMenus.c

	Contains:	System extension to test the MoreSystemMenus module.

	Written by:	Quinn

	Copyright:	Copyright (c) 1999-2001 by Apple Computer, Inc., All Rights Reserved.

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

$Log: TestMoreSystemMenus.c,v $
Revision 1.4  2002/11/08 23:37:08         
Convert nil to NULL. Convert MoreAssertQ to assert.

Revision 1.3  2001/11/07 15:58:38         
Tidy up headers, add CVS logs, update copyright.


         <2>     20/9/01    Quinn   Upgrade to CWPro7.
         <1>     27/8/99    Quinn   First checked in.
*/

/////////////////////////////////////////////////////////////////

// MIB Setup

#include "MoreSetup.h"

// Mac OS Interfaces

#include <Resources.h>
#include <Menus.h>
#include <TextUtils.h>
#include <Sound.h>
#include <Dialogs.h>

// Wacky Metrowerks Interfaces

#if TARGET_CPU_68K
	#include <A4Stuff.h>
	#include <SetUpA4.h>
#else
	#define SetUpA4(param) 0
	#define SetA4(param) 0
	#define RememberA4(param)
	#define SetCurrentA4() 0
#endif

// MIB Interfaces

#include "MoreSystemMenus.h"

/////////////////////////////////////////////////////////////////

static FSSpec gOurExtensionFile;

static void DoModalDialog(ConstStr255Param str)
{
	OSStatus err;
	SInt16 refNum;
	DialogPtr dlg;
	DialogItemType junkType;
	Handle itemH;
	Rect junkRect;
	DialogItemIndex itemHit;
	
	// When opening resource files in a weird context, use
	// fsRdPerm avoid being bitten by an annoying Resource Manager
	// misfeature.  See TN 1120 for details.
	//
	// <http://developer.apple.com/technotes/tn/tn1120.html>
	
	refNum = FSpOpenResFile(&gOurExtensionFile, fsRdPerm);
	err = ResError();
	if (err == noErr) {
		dlg = GetNewDialog(128, NULL, (WindowPtr) -1);
		if (dlg == NULL) {
			err = memFullErr;
		}
		
		if (err == noErr) {
			GetDialogItem(dlg, 3, &junkType, &itemH, &junkRect);
			SetDialogItemText(itemH, str);
			SetDialogDefaultItem(dlg, 1);
			ShowWindow(dlg);
			do {
				ModalDialog(NULL, &itemHit);
			} while (itemHit != 1);
			
			DisposeDialog(dlg);
		}
		
		CloseResFile(refNum);
		assert(ResError() == noErr);
	}
	
	if (err != noErr) {
		DebugStr("\pDoModalDialog: Got an error.");
	}
}

static MenuRef gSysMenu = NULL;
static MenuRef gSubMenu1 = NULL;
static MenuRef gSubMenu2 = NULL;

static pascal OSStatus MySysMenuCallback(OSType selector,
											MenuRef theMenu,
											void *param, 
											void *refcon)
{
	long oldA4;
	OSStatus err;
	Str255 itemStr;
	UInt16 hitItem;
	EventRecord event;
	#pragma unused(refcon)
	
	oldA4 = SetUpA4();

	switch (selector) {
		case kSystemMenuActOnChosen:
			hitItem = (UInt16) param;
			GetMenuItemText(theMenu, hitItem, itemStr);
			if (hitItem == 1) {
				DoModalDialog(itemStr);
			} else {
				DebugStr(itemStr);
			}
			break;
		case kSystemMenuPreSelectAdjust:
			err = InsertSystemSubMenu(gSysMenu, gSysMenu, 4, gSubMenu1);
			if (err == noErr) {
				err = InsertSystemSubMenu(gSysMenu, gSubMenu1, 2, gSubMenu2);
			}
			if (err == noErr) {
				(void) OSEventAvail(0, &event);
				if (event.modifiers & optionKey) {
					DoModalDialog("\pkSystemMenuPreSelectAdjust");
				}
			}
			assert(err == noErr);
			break;
		default:
			// do nothing
			break;
	}

	(void) SetA4(oldA4);
	return noErr;
}

static OSStatus BasicSystemMenuTest(void)
{
	OSStatus err;
	OSStatus junk;
	IconSuiteRef iconSuite;
	
	// Initialize our system menu.
	
	iconSuite = NULL;
	gSysMenu = NULL;
	
	gSysMenu  = GetNewSystemMenu(128);
	gSubMenu1 = GetNewSystemMenu(129);
	gSubMenu2 = GetNewSystemMenu(130);
	if (gSysMenu == NULL || gSubMenu1 == NULL || gSubMenu2 == NULL) {
		err = memFullErr;
	}
	if (err == noErr) {
		err = GetDetachedIconSuite(&iconSuite, 128, kSelectorAllSmallData);
	}
	if (err == noErr) {
		err = SetMenuTitleToIconSuite(gSysMenu, iconSuite);
	}
	if (err == noErr) {
		err = InsertSystemMenu(gSysMenu, MySysMenuCallback, NULL);
	}
	
	// Clean up.

	if (err != noErr) {
		if (gSysMenu != NULL) {
			DisposeMenu(gSysMenu);
			gSysMenu = NULL;
		}
		if (gSubMenu1 != NULL) {
			DisposeMenu(gSubMenu1);
			gSubMenu1 = NULL;
		}
		if (gSubMenu2 != NULL) {
			DisposeMenu(gSubMenu2);
			gSubMenu2 = NULL;
		}
		if (iconSuite != NULL) {
			junk = DisposeIconSuite(iconSuite, true);
			assert(junk == noErr);
		}
	}
	
	return err;
}

/////////////////////////////////////////////////////////////////

static MenuRef gSoundMenu = NULL;

static pascal OSStatus SoundSysMenuCallback(OSType selector,
											MenuRef theMenu,
											void *param, 
											void *refcon)
{
	long oldA4;
	OSStatus junk;
	SndListHandle sndH;
	Str255 itemStr;
	#pragma unused(param)
	#pragma unused(refcon)
	
	oldA4 = SetUpA4();

	switch (selector) {
		case kSystemMenuActOnChosen:
			assert(theMenu == gSoundMenu);
			
			GetMenuItemText(theMenu, (UInt16) param, itemStr);
			sndH = (SndListHandle) GetNamedResource('snd ', itemStr);
			if (sndH != NULL) {
				junk = SndPlay(NULL, sndH, false);
				assert(junk == noErr);
			} else {
				assert(false);
			}
			break;
		default:
			// do nothing
			break;
	}

	(void) SetA4(oldA4);
	return noErr;
}

static OSStatus CreateSoundSystemMenu(void)
{
	OSStatus err;
	OSStatus junk;
	IconSuiteRef soundIcon;
	Str255 soundMenuTitle;
	
	err = GetDetachedIconSuite(&soundIcon, 256, kSelectorAllSmallData);
	if (err == noErr) {
		IconSuiteToSystemMenuTitle(soundIcon, soundMenuTitle);

		gSoundMenu = NewSystemMenu(soundMenuTitle);
		if (gSoundMenu == NULL) {
			err = memFullErr;
		}
	}
	if (err == noErr) {
		AppendResMenu(gSoundMenu, 'snd ');
		err = InsertSystemMenu(gSoundMenu, SoundSysMenuCallback, NULL);
	}
	
	// Clean up.
	
	if (err != noErr) {
		if (gSoundMenu != NULL) {
			DisposeMenu(gSoundMenu);
			gSoundMenu = NULL;
		}
		if (soundIcon != NULL) {
			junk = DisposeIconSuite(soundIcon, true);
			assert(junk == noErr);
		}
	}
	
	return err;
}


/////////////////////////////////////////////////////////////////

static OSErr RefNumToFSSpec(SInt16 refNum, FSSpec *fss)
{
	OSErr err;
	FCBPBRec fcbPB;
	
    fcbPB.ioNamePtr = fss->name;
    fcbPB.ioVRefNum = 0;
    fcbPB.ioRefNum = refNum;
    fcbPB.ioFCBIndx = 0;
    err = PBGetFCBInfoSync(&fcbPB);
	if (err == noErr) {
		fss->vRefNum = fcbPB.ioFCBVRefNum;
		fss->parID = fcbPB.ioFCBParID;
	}
	return err;
}

extern void main(void)
{
	long oldA4;
	OSStatus err;
	Handle ourCodeResource;
	Boolean shouldDetach;
	
	#if MORE_DEBUG
		Debugger();
	#endif
	
	oldA4 = SetCurrentA4();
	RememberA4();
	
	// Find, lock and detach our code.
	
	ourCodeResource = Get1Resource('INIT', 128);
	assert(ourCodeResource != NULL);

	HLock(ourCodeResource);
	assert(MemError() == noErr);
	
	shouldDetach = false;
	
	err = RefNumToFSSpec(CurResFile(), &gOurExtensionFile);
	if (err == noErr) {
		err = InitMoreSystemMenus();
		
		// Once InitMoreSystemMenus returns noErr, we're patched into
		// the system in such a way that we can't unload our code.
		
		shouldDetach = (err == noErr);
	}
	if (err == noErr) {
		err = BasicSystemMenuTest();
	}
	if (err == noErr) {
		err = CreateSoundSystemMenu();
	}
	if (err != noErr) {
		DebugStr("\pTestMoreSystemMenus: Failed to start up");
	}

	if (shouldDetach) {
		DetachResource(ourCodeResource);
		assert(ResError() == noErr);
	}
	
	(void) SetA4(oldA4);
}