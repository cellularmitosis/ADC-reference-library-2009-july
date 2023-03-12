/*
	File:		MoreSystemMenus.c

	Contains:	Module for adding iconic menus to the right of the menu bar.

	Written by:	Quinn, Pete Gontier

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

$Log: MoreSystemMenus.c,v $
Revision 1.7  2002/11/08 23:36:40         
Include our prototype early to flush out any missing dependencies. Convert nil to NULL. Convert MoreAssertQ to assert. The compile time environment checking moved to the header.

Revision 1.6  2001/11/07 15:55:36         
Tidy up headers, add CVS logs, update copyright.


         <5>     21/9/01    Quinn   Changes for CWPro7 Mach-O build.
         <4>     20/9/01    Quinn   Upgrade to CWPro7.
         <3>    24/11/00    Quinn   Complain if compiled for Carbon.
         <2>     7/12/99    Quinn   Fix bug where sticky menu timeout was doing the current menu
                                    operation instead of cancelling.
         <1>     25/8/99    Quinn   First checked in.
*/

// To Do Items
//
// √ Ask Eric about icon suite titles and menu bar offscreen always being 8 bits deep
//   • Eric says it should be fixed in Sonata, but I can't test because I don't have
//     a 1 bit capable machine that can run Sonata!
// • get rid of NOPs in PPC callers
//
// Testing (perfunctorary, boot to Finder, test system menu, test that other system menus still work)
//
// - System 7.1 (fails with bus error at startup, GetMenu needs InitGraf)
// √ System 7.5.5
// • Mac OS 7.6.1
// √ Mac OS 8.1
// √ Mac OS 8.6
// √ Mac OS 9.0
// √ third party system menus
//   √ Timbuktu
//   √ MOM
// √ 68K clients
// √ PowerPC clients
// √ multiple clients in different extensions
// √ multiple clients in same extension
// √ keyboard menu
// √ JLK, especially Kotereri's menu
// √ sub-menus
// • MenuEvent
// • modifying system menus while in the application context

/////////////////////////////////////////////////////////////////

// Our prototypes

#include "MoreSystemMenus.h"

// Mac OS Interfaces

#if ! MORE_FRAMEWORK_INCLUDES
	#include <MacTypes.h>
	#include <Menus.h>
	#include <LowMem.h>
	#include <MixedMode.h>
	#include <Traps.h>
#endif

// Wacky Metrowerks Interfaces

#if TARGET_CPU_68K
	#include <A4Stuff.h>
	#include <SetUpA4.h>
#else
	#define SetUpA4(param) 0
	#define SetA4(param) 0
	#define RememberA4(param)
#endif

// ANSI C Interfaces (just for "offsetof" macro)

#include <stddef.h>

/////////////////////////////////////////////////////////////////
#pragma mark ----- Comments -----

/*	Some Overall Notes:

	  o A4 Handling -- We remember A4 in our primary entry point
	  	and reset it whenever any other entry point is called.
	  	There are two other types of entry points, published routines
	  	and trap patches.  You can spot the former by searching
	  	for "extern pascal" and the latter by searching for "static pascal".
	  	Note that we don't set up A4 on some of the trivial utility
	  	routines because they access no globals.
	  
	  o Patching SystemMenu --
	  
		Reviewer comment:
		
		One design tweak to consider: instead of using
		MenuDisable to figure out which item was selected, you
		could also patch SystemMenu, which is called to handle
		selections from (you guessed it!) system menus as well
		as DAs. This has a couple advantages:

		- you don't need your heuristic code for figuring out
		when to ignore the contents of MenuDisable

		- MenuDisable is updated by the MDEF, not the Menu
		Manager, and if someone wrote a custom MDEF for a system
		menu, they might not know or bother to update
		MenuDisable. But SystemMenu would still be called with
		the proper info.

		Quinn's response:
		
		I thought about this for a while and decided against it.
		The reasons:

		The more patches you have, the trickier the interaction
		between them.  Specifically, the most obvious algorithm
		for this would be...

			  on menuSelect
			    insert system menus
			    call old menuSelect
			    remove system menus
			  end menuSelect
			
			  on systemMenu
			    find and call client
			  end systemMenu

		but this is bad if the client callback recursively calls
		MenuSelect.  I carefully avoid this in my current code
		using...

			  on menuSelect
			    insert system menus
			    call old menuSelect
			    remove system menus
			    find and call client
			  end menuSelect

		but it's hard to do this from two patches.

		There are two immediately obvious alternate designs,
		shown below, but neither are particularly
		satisfactorary.

		a)
			  on menuSelect
			    insert system menus
			    call old menuSelect
			    if system menus still in menu bar
			      remove system menus
			    end-if
			  end menuSelect
			
			  on systemMenu
			    remove system menus
			    find and call client
			  end systemMenu

		I didn't implement this because it involves removing
		menus from the menu bar in the middle of MenuSelect's
		call to SystemMenu, and I was worried about how Menu
		Manager (past, present and future) might react to this.

		b)
			  on menuSelect
			    insert system menus
			    call old menuSelect
			    remove system menus
			    get global which holds menuResult from systemMenu
			    find and call client
			  end menuSelect
			
			  on systemMenu
			    store menuResult in global
			  end systemMenu

		I didn't implement this because it involves more global
		variables, which meant that I'd have to do yet another
		recursion analysis.
		
		Besides, my current code works (-:
*/

/////////////////////////////////////////////////////////////////
#pragma mark ----- Globals -----

// Simple static globals

static IconActionUPP 	gDetachIconActionUPP;		// points to DetachIconAction

static UniversalProcPtr gMenuSelectOldUPP;
static UniversalProcPtr gInsertMenuOldUPP;

// List of root menus

struct RootSysMenuEntry {
	MenuRef 				 theMenu;				// the menu to insert
	SystemMenuCallbackProc 	 callback;				// the client callback
	void 					*refcon;				// and its refcon
};
typedef struct RootSysMenuEntry RootSysMenuEntry;
typedef RootSysMenuEntry *RootSysMenuPtr;
typedef RootSysMenuPtr   *RootSysMenuHandle;

static RootSysMenuHandle gRootSysMenus = NULL;		// an unbounded array of RootSysMenuEntry

static Boolean gHaveInsertedRootSysMenus = false;

static MCTableHandle gRootSysMenuCInfo = NULL;

// List of sub-menus (built and torn down during the MenuSelect patch)

struct SubSysMenuEntry {
	ItemCount	rootMenuIndex;						// index into gRootSysMenus
	MenuRef 	parentMenu;							// reference to parent menu
	SInt16		itemInParent;						// and item
	MenuRef 	childMenu;							// reference to child menu
};
typedef struct SubSysMenuEntry SubSysMenuEntry;
typedef SubSysMenuEntry *SubSysMenuPtr;
typedef SubSysMenuPtr *SubSysMenuHandle;

static SubSysMenuHandle gSubSysMenus = NULL;			// an unbounded array of SubSysMenuEntry

// State associated with the MenuSelect patch

typedef enum {
	kMenuSelectStateNil  = 0,						// outside of patch
	kMenuSelectStatePre  = 1,						// inside head part of patch
	kMenuSelectStatePost = 2						// inside of tail part of patch
} MenuSelectState;

static MenuSelectState gMenuSelectState;

/////////////////////////////////////////////////////////////////
#pragma mark ----- General Utility Routines -----

static Boolean ValidateSystemMenuRef(MenuRef theMenu)
	// Returns true if theMenu is a valid system menu handle.
{
	Boolean result;
	
	// First check for NULL and NULL master pointer.
	
	result = (theMenu != NULL) && (*theMenu != NULL);
	
	// Then check that both handle and pointer are in the system heap.
	// This will trigger falsely if you use the pageable system heap for menu
	// handles, but third party developers shouldn't be doing that.
	
	if (result) {
		THz sysZone;
		char *lowerBound;
		char *upperBound;
		
		sysZone = SystemZone();
		lowerBound = (char *) sysZone;
		upperBound = (char *) sysZone->bkLim;
		
		if (   ((char *)  theMenu) < lowerBound 
			|| ((char *)  theMenu) > upperBound
			|| ((char *) *theMenu) < lowerBound
			|| ((char *) *theMenu) > upperBound ) {
			result = false;
		}
	}
	
	// Now check the handle size.
	
	if (result) {
		Size size;

		size = GetHandleSize( (Handle) theMenu);
		if (MemError() != noErr || size < offsetof(MenuInfo, menuData)) {
			result = false;
		}
	}
	
	// Now check that neither resource nor purgeable bits are set.
	
	if (result) {
		SInt8 s;
		
		s = HGetState( (Handle) theMenu );
		if (MemError() != noErr || (s & 0x060) != 0) {
			result = false;
		}
	}
	
	return result;
}

static THz SetSystemZone(void)
	// Sets the current zone to the system zone, returning
	// the previous value.
{
	THz result;
	
	result = GetZone();
	SetZone(SystemZone());
	return result;
}

static ItemCount CountRootSysMenus(void)
	// Returns the number of root system menu entries in gRootSysMenus.
{
	ItemCount result;
	assert(gRootSysMenus != NULL);
	
	result = GetHandleSize( (Handle) gRootSysMenus ) / sizeof(RootSysMenuEntry);
	assert(MemError() == noErr);
	return result;
}

static ItemCount CountSubSysMenus(SubSysMenuHandle subSysMenus)
	// Returns the number of entries in the subSysMenus sub-menu list.
	// Note that this handles the case where subSysMenus is NULL and returns
	// zero, which is unlike CountRootSysMenus.  I did this
	// to fix the case where HandleMenuSelect is called with no
	// sub-menus installed.
{
	ItemCount result;
	
	if (subSysMenus == NULL) {
		result = 0;
	} else {
		result = GetHandleSize( (Handle) subSysMenus ) / sizeof(SubSysMenuEntry);
	}
	return result;
}

enum {
	kHighestGlobalMenuID = -16385,
	kLowestGlobalMenuID  = -20480
};

static Boolean IsSystemMenuID(SInt16 menuID)
	// Returns true if menuID is in the system menu range,
	// as defined by the above constants.
	//
	// Don't blame me for the backwards logic in this routine.
	// I just stole the code from PeteG (-:
{
	return !(menuID < kLowestGlobalMenuID || menuID > kHighestGlobalMenuID);
}

static Handle LMGetSystemMenuList(void)
	// Returns the low-memory global SystemMenuList,
	// which is like MenuList but it holds the list
	// of system menus.
{
	return *((Handle *) 0x0286);
}

static MenuHandle GetSystemMenuHandle(SInt16 menuID)
	// An analogue of GetMenuHandle, this routine
	// returns a handle to an installed system menu
	// in the system menu list.
{
	MenuHandle result;
	Handle saveMenuList;
	
	// Switch MenuList to SystemMenuList, call
	// GetMenuHandle and then switch it back.
	
	saveMenuList = LMGetMenuList();
	LMSetMenuList(LMGetSystemMenuList());
	
	result = GetMenuHandle(menuID);

	LMSetMenuList(saveMenuList);

	return result;
}

static SInt16 FindUniqueSystemMenuID(void)
	// Returns the next available system menu ID.
	// It works by starting at the top of the system
	// menu ID range and calling GetSystemMenuHandle on
	// each sequential ID to see whether that ID is already
	// used.
	//
	// We call this routine when we insert our system
	// menus into the system menu list at Process Manager
	// launch time.
{
	SInt16 result;
	
	result = kHighestGlobalMenuID;
	while ( GetSystemMenuHandle(result) != NULL ) {
		result -= 1;
	}
	
	// The following assert fires if a) the system menu list
	// is full, ie we have more than 4000 system menus,
	// ie *very* unlikely, or b) something bad has happened.
	// Either way, we want to know.  Because the only 
	// valid failure case is so unlikely, the higher
	// level ignore that possibility, so we don't have
	// an error return from this routine.
	
	assert(result >= kLowestGlobalMenuID);

	return result;
}

static SInt16 FindUniqueSubMenuID(void)
	// Returns the next available menu ID for a hierarchical
	// (or sub-) menu.  Prior to Mac OS 8.0, the sub-menu
	// ID for a menu item was encoded in the item mark field
	// of the menu, which meant that sub-menus were required
	// to use an ID that fit into a byte.  This routine will
	// search for such an ID and return it.
	//
	// We call this routine when we insert sub-menus into
	// the menu bar, namely on our head patch to MenuSelect.
	// Note that sub-menus go into the normal menu bar
	// (because we remove them on the tail patch to MenuSelect)
	// so this routine calls the standard GetMenuHandle to test
	// for uniqueness.
{
	SInt16 result;
	
	result = 235;
	while ( GetMenuHandle(result) != NULL) {
		result -= 1;
	}
	
	// The following assert fires if a) the application's menu list
	// is full, ie we have more than 250 application menus,
	// ie *very* unlikely, or b) something bad has happened.
	// Either way, we want to know.  Because the only 
	// valid failure case is so unlikely, the higher
	// level ignore that possibility, so we don't have
	// an error return from this routine.

	assert(result > 0);

	return result;
}

static Boolean IsSystemStartup(void)
	// Returns true if we're still at system startup time.
	// We do this in the traditional way, by testing the first
	// byte of CurApName.  We only use this for debugging.
{
	return LMGetCurApName()[0] == 0xFF;
}

extern pascal MenuRef GetNewSystemMenu(SInt16 menuResourceID)
	// See comment in interface part.
{
	MenuRef result;
	THz oldZone;
	
	oldZone = SetSystemZone();
	result = GetMenu(menuResourceID);
	if (result != NULL) {
		DetachResource( (Handle) result );
		assert(ResError() == noErr);
		
		assert(ValidateSystemMenuRef(result));
	}
	SetZone(oldZone);
	return result;
}

extern pascal MenuRef NewSystemMenu(ConstStr255Param title)
	// See comment in interface part.
{
	MenuRef result;
	THz oldZone;
	
	assert(title != NULL);
	
	oldZone = SetSystemZone();
	result = NewMenu(0, title);
	assert(result == NULL || ValidateSystemMenuRef(result));
	SetZone(oldZone);
	return result;
}

static pascal OSErr DetachIconAction(ResType theType, Handle *theIcon, void *yourDataPtr)
	// GetDetachedIconSuite passes this routine to
	// ForEachIconDo to detach each icon icon in the
	// suite from the resource file from which it came.
{
	assert(theIcon != NULL);
	#pragma unused(theType)
	#pragma unused(yourDataPtr)
	
	if (*theIcon != NULL) {
		DetachResource(*theIcon);

		// Errors from DetachResource probably aren't fatal
		// (it probably means that the icon handle isn't a resource),
		// but we certainly want to know about them.

		assert(ResError() == noErr);
	}
	
	return noErr;
}

extern pascal OSStatus GetDetachedIconSuite(IconSuiteRef *theIconSuite, SInt16 theResID, IconSelectorValue selector)
	// See comment in interface part.
{
	long oldA4;
	OSStatus err;
	THz oldZone;
	
	oldA4 = SetUpA4();
	
	assert(theIconSuite != NULL);
	
	oldZone = SetSystemZone();

	// The algorithm here is perfectly simple.  First get the icon suite,
	// then iterate over its contents detaching each handle.  I looked
	// at the Icon Services code and this seems like a perfectly reasonable
	// strategy.  It also disassembled SBGetDetachIconSuite and it works
	// in the same way.
	
	err = GetIconSuite(theIconSuite, theResID, selector);
	if (err == noErr) {
		(void) ForEachIconDo(*theIconSuite, kSelectorAllAvailableData, gDetachIconActionUPP, NULL);
	}
	
	SetZone(oldZone);
	
	(void) SetA4(oldA4);
	
	return err;
}

extern pascal void IconSuiteToSystemMenuTitle(IconSuiteRef theIconSuite, StringPtr menuTitle)
	// See comment in interface part.
	//
	// It's vital that this routine not more or purge memory unless you
	// want to rewrite SetMenuTitleToIconSuite to lock the menu handle.
{
	assert(theIconSuite != NULL);
	assert(menuTitle != NULL);

	// The following 2 are byte accesses, so they'll work
	// on original 68000.
	
	menuTitle[0] = 5;
	menuTitle[1] = 1;
	
	// The next is a 4-byte access, but it's 2-byte aligned
	// so it will also work on the original 68000.
	
	*((IconSuiteRef *) &menuTitle[2]) = theIconSuite;
}

extern pascal OSStatus SetMenuTitleToIconSuite(MenuRef theMenu, IconSuiteRef theIconSuite)
	// See comment in interface part.
{
	OSStatus err;
	
	assert(ValidateSystemMenuRef(theMenu));
	assert(theIconSuite != NULL);
	
	if ( (**theMenu).menuData[0] == 5 ) {
		IconSuiteToSystemMenuTitle(theIconSuite, (**theMenu).menuData);
		err = noErr;
	} else {
		err = paramErr;
	}
	
	return err;
}

/////////////////////////////////////////////////////////////////
#pragma mark ----- Core System Menu -----

extern pascal OSStatus InsertSystemMenu(MenuRef theMenu,
										SystemMenuCallbackProc callback,
										void *refcon)
	// See comment in interface part.
{
	long oldA4;
	OSStatus err;
	RootSysMenuEntry newEntry;
	
	oldA4 = SetUpA4();

	assert(ValidateSystemMenuRef(theMenu));
	assert(callback != NULL);

	// If either of these fire, you're too late to call this routine,
	// either because system startup is complete or because someone
	// has already inserted a system menu.

	assert( IsSystemStartup() );
	assert( ! gHaveInsertedRootSysMenus );

	// Fill out the new entry.
	
	newEntry.theMenu  = theMenu;
	newEntry.callback = callback;
	newEntry.refcon   = refcon;
	
	// Add the entry on to the system menu list.  Later on, in the
	// InsertMenuPatch, we'll add it to the menu bar.
	
	if (theMenu == NULL || callback == NULL) {
		err = paramErr;
	} else {
		err = PtrAndHand(&newEntry, (Handle) gRootSysMenus, sizeof(newEntry));
	}

	(void) SetA4(oldA4);
	
	return err;
}

// kRootMenuNotFound is used by FindRootMenuByID and FindRootMenuByRef
// to indicate that no menu was found.  Note that those routines return
// an ItemCount, which is unsigned, to returning -1 gets very confusing
// very quickly (-:

enum {
	kRootMenuNotFound = 0xFFFFFFFF
};

static ItemCount FindRootMenuByID(SInt16 menuID)
	// Search the list of root system menus (gRootSysMenus)
	// looking for one with the given menu ID.  Returns
	// kRootMenuNotFound if there is none.
{
	ItemCount result;
	ItemCount systemMenuCount;
	ItemCount systemMenuIndex;

	result = kRootMenuNotFound;
	systemMenuCount = CountRootSysMenus();
	for (systemMenuIndex = 0; systemMenuIndex < systemMenuCount; systemMenuIndex++) {
		if (menuID == (**(*gRootSysMenus)[systemMenuIndex].theMenu).menuID) {
			result = systemMenuIndex;
			break;
		}
	}
	return result;
}

static ItemCount FindRootMenuByRef(MenuRef menuRef)
	// Search the list of root system menus (gRootSysMenus)
	// looking for one with the given menu ID.  Returns
	// kRootMenuNotFound if there is none.
{
	ItemCount result;
	ItemCount systemMenuCount;
	ItemCount systemMenuIndex;

	assert(ValidateSystemMenuRef(menuRef));
	
	result = kRootMenuNotFound;
	systemMenuCount = CountRootSysMenus();
	for (systemMenuIndex = 0; systemMenuIndex < systemMenuCount; systemMenuIndex++) {
		if (menuRef == (*gRootSysMenus)[systemMenuIndex].theMenu) {
			result = systemMenuIndex;
			break;
		}
	}
	return result;
}

extern pascal OSStatus InsertSystemSubMenu(MenuRef rootMenu,
										MenuRef parentMenu, UInt16 itemInParent, MenuRef childMenu)
	// See comment in interface part.
{
	long oldA4;
	OSStatus err;
	SubSysMenuEntry newEntry;
	SInt16 newIDForChildMenu;
	
	oldA4 = SetUpA4();

	assert(ValidateSystemMenuRef(rootMenu  ));
	assert(ValidateSystemMenuRef(parentMenu));
	assert(ValidateSystemMenuRef(childMenu ));
	
	assert(gMenuSelectState = kMenuSelectStatePre);
	
	// Validate parameters
	
	err = noErr;
	if ((rootMenu == NULL) || (parentMenu == NULL) || (childMenu == NULL)) {
		err = paramErr;
	}
	if (err == noErr && ((itemInParent == 0) || (itemInParent > CountMenuItems(parentMenu)))) {
		err = paramErr;
	}

	// Make sure rootMenu references a current system menu.
	// If it does, record the index of the root menu for
	// later use by HandleMenuSelect.  Also fill out the rest
	// of the fields of the sub-menu entry.

	if (err == noErr) {
		newEntry.rootMenuIndex = FindRootMenuByRef(rootMenu);
		newEntry.parentMenu    = parentMenu;
		newEntry.itemInParent  = itemInParent;
		newEntry.childMenu     = childMenu;
		
		if (newEntry.rootMenuIndex == kRootMenuNotFound) {
			err = paramErr;
		}
	}
	
	// Add newEntry to the sub-menu list, creating the sub-menu list if necessary.

	if (err == noErr) {
		if (gSubSysMenus == NULL) {
			gSubSysMenus = (SubSysMenuHandle) NewHandleSys(0);
			err = MemError();
		}
	}
	if (err == noErr) {
		err = PtrAndHand(&newEntry, (Handle) gSubSysMenus, sizeof(newEntry));
	}
	
	// Finally, insert the menu with a unique ID into the menu bar,
	// and set the parent item to reference its ID.
	
	if (err == noErr) {
		newIDForChildMenu = FindUniqueSubMenuID();
		(**childMenu).menuID = newIDForChildMenu;
		InsertMenu(childMenu, hierMenu);

		SetItemCmd(parentMenu, itemInParent, hMenuCmd);
		SetItemMark(parentMenu, itemInParent, newIDForChildMenu);
		
		// I added this after testing revealed that adding a sub-menu didn't 
		// force the menu size to be recalculated, so the item text of the
		// hierarchical item in the sub-menu was being truncated.  This
		// only appears to happen on Mac OS 8.5, but the fix is sufficiently
		// benign to be employed on all systems.
		
		CalcMenuSize(parentMenu);
	}
	
	(void) SetA4(oldA4);
	
	return err;
}

static void DeleteSubMenus(SubSysMenuHandle subSysMenus)
	// Remove any sub-menus that we added to the menu bar.
	// Basically this just walks subSysMenus (backwards, which
	// isn't strictly necessary, but reassures me that the menu
	// bar is somewhat consistent at each step), deleting each
	// menu from the menu bar and reseting its parent to reference
	// menu ID 0.
{
	ItemCount entryCount;
	ItemCount entryIndex;
	SubSysMenuEntry thisEntry;
	
	// Have to handle both NULL and non-NULL case.
	// This expression always evaluates to true,
	// but it captures the semantics of what this
	// routine must do.
	
	assert(subSysMenus != NULL || subSysMenus == NULL);
	
	entryCount = CountSubSysMenus(subSysMenus);
	for (entryIndex = 0; entryIndex < entryCount; entryIndex++) {
		thisEntry = (*subSysMenus)[entryCount - entryIndex - 1];
		DeleteMenu( (**(thisEntry.childMenu)).menuID );
		SetItemMark( thisEntry.parentMenu, thisEntry.itemInParent, 0);

		// Recalculate the parent menu size, for consistency with
		// the similar code in InsertSystemSubMenu.
		
		CalcMenuSize(thisEntry.parentMenu);
	}
}
	
static void CallSingleClient(ItemCount systemMenuIndex, OSType selector, MenuRef theMenu, void *param)
	// Call the client's callback associated with the root menu
	// whose index is systemMenuIndex with the supplied parameters.
	// Note that the MenuRef is a parameter to this routine because,
	// in the case of a sub-menu, it's the MenuRef of the sub-menu,
	// not the MenuRef of the root menu.
{
	OSStatus junk;
	RootSysMenuEntry *entryToCall;
	
	assert(ValidateSystemMenuRef(theMenu));
	
	entryToCall = &(*gRootSysMenus)[systemMenuIndex];
	
	junk = (entryToCall->callback)(selector, theMenu, param, entryToCall->refcon);
	
	// If the client returns an error, that's a problem because there
	// are currently no selectors where the return result is used.
	
	assert(junk == noErr);
}

static void CallAllClients(OSType selector, void *param)
	// Call the callbacks associated with all root menus
	// with the supplied parameters.  This routine is used
	// for broadcast-type messages, for example kSystemMenuPreSelectAdjust.
	// There's no MenuRef passed to this routine because the client
	// is expected to adjust their sub-menus when they get called
	// for their root menu.
{
	ItemCount systemMenuCount;
	ItemCount systemMenuIndex;

	systemMenuCount = CountRootSysMenus();
	for (systemMenuIndex = 0; systemMenuIndex < systemMenuCount; systemMenuIndex++) {
		CallSingleClient(systemMenuIndex, selector, (*gRootSysMenus)[systemMenuIndex].theMenu, param);
	}
}

static SInt32 HandleMenuSelect(SInt32 menuAndItem, SubSysMenuHandle subSysMenus)
	// This routine is called from the tail patch on MenuSelect
	// to handle an actual menu command being returned.
	// menuAndItem was the result from the original MenuSelect, and
	// the result of this routine will be returned from our MenuSelect
	// patch.
	//
	// The basic idea is that, if the menu command was on
	// a system menu belong to us, we call the appropriate client
	// callback routine and return 0 so that the client sees
	// a null selection.  If the menu command doesn't belong to us,
	// we just return the value passed in.
{
	SInt32 result;
	SInt16 hitMenuID;
	UInt16 hitItem;
	ItemCount foundIndex;
	MenuRef foundMenuRef;
	
	// The system implementation of MenuSelect will return 0 if the hit item
	// is in a system menu.  Fortunately, the real hit item is still around
	// in the low memory global MenuDisable.  So we use if menuAndItem is zero.
	// 
	// One weird edge case is cancelling sticky menus.  If the user brings
	// down a sticky menu and never chooses anything, the sticky menu
	// will eventually time out.  But LMGetMenuDisable is still set to
	// the menu and item of the item over which the mouse was last moved.
	// So, we conditionalise our access to LMGetMenuDisable with a
	// call to LMGetTheMenu.  If LMGetTheMenu is 0, the operation was
	// cancelled and we should bail out without choosing the item
	// in LMGetMenuDisable.
	//
	// Also note that we assign result before doing this, so we only use
	// MenuDisable if it relates to a system menu and never return it from
	// this routine, and hence from our patch on MenuSelect.
	
	result = menuAndItem;	
	if (menuAndItem == 0) {
		if ( LMGetTheMenu() != 0 ) {
			menuAndItem = LMGetMenuDisable();
		}
	}
	
	hitMenuID = menuAndItem >> 16;
	hitItem   = menuAndItem & 0x0FFFF;
	
	// Search our list of system menus looking for a matching menu ID.
	// Start by looking for a root menu and, if that fails, look for
	// one of the sub-menus we attached to the root menus.
	//
	// If we find a matching menu, call the corresponding client with a
	// kSystemMenuActOnChosen message, and also set menuAndItem to 0 so
	// that the application never sees the menu choice.
	
	foundMenuRef = NULL;
	
	foundIndex = FindRootMenuByID(hitMenuID);
	if (foundIndex != kRootMenuNotFound) {
		foundMenuRef = (*gRootSysMenus)[foundIndex].theMenu;
	} else {
		ItemCount entryCount;
		ItemCount entryIndex;
		
		entryCount = CountSubSysMenus(subSysMenus);
		for (entryIndex = 0; entryIndex < entryCount; entryIndex++) {
			if (hitMenuID == (**(*subSysMenus)[entryIndex].childMenu).menuID) {
				foundIndex = (*subSysMenus)[entryIndex].rootMenuIndex;
				foundMenuRef = (*subSysMenus)[entryIndex].childMenu;
				assert(foundIndex < CountRootSysMenus());
				break;
			}
		}
	}
	if (foundIndex != kRootMenuNotFound) {
		Boolean enabled;
		CharParameter cmdOfHitItem;

		assert(ValidateSystemMenuRef(foundMenuRef));
		
		// Because we're using MenuDisable to get the actual item hit,
		// we'll often get false positives, hits on the menu title or
		// when the menu item is disabled.  Make sure we don't call the
		// client in these cases.

		// Item 0 is the title and is never enabled.  Items from 1 to 31
		// are controlled by the enableFlags field of the menu.  Items
		// above 31 are always assumed to be enabled.
		//
		// Note that I don't use IsMenuItemEnabled (introduced in Mac OS 8.5)
		// to access this information because it's a CFM-only routine.  It's
		// only relevant for items beyond 31, which is a corner case IMHO.

		// Also, items that are the root of sub-menus (ie their cmd is
		// hMenuCmd) can not be chosen.
		
		if (hitItem == 0) {
			enabled = false;
		} else if (hitItem <= 31) {
			enabled = (((**foundMenuRef).enableFlags & (1L << hitItem)) != 0);
		} else {
			enabled = true;
		}
		if (enabled) {
			GetItemCmd(foundMenuRef, hitItem, &cmdOfHitItem);
			enabled = (cmdOfHitItem != hMenuCmd);
		}
		if (enabled) {
			CallSingleClient(foundIndex, kSystemMenuActOnChosen, foundMenuRef, (void *) hitItem);
		}
		result = 0;
	}
	
	return result;
}

/////////////////////////////////////////////////////////////////
#pragma mark ----- Trap Patches -----

enum {
	uppMenuSelectProcInfo = 0x00F0,			// Extracted from InterfaceLib
	uppInsertMenuProcInfo = 0x02C0			// Extracted from InterfaceLib
};

typedef pascal SInt32 (*MenuSelectProcPtr)(Point startPt);
typedef pascal void   (*InsertMenuProcPtr)(MenuHandle theMenu, SInt16 beforeID);

#if TARGET_RT_MAC_CFM
	#define CallMenuSelectProc(userRoutine, startPt)  \
			CALL_ONE_PARAMETER_UPP((userRoutine), uppMenuSelectProcInfo, (startPt))
	#define CallInsertMenuProc(userRoutine, theMenu, beforeID)  \
			CALL_TWO_PARAMETER_UPP((userRoutine), uppInsertMenuProcInfo, (theMenu), (beforeID))
#else
	#define CallMenuSelectProc(userRoutine, startPt)  \
			((MenuSelectProcPtr)(userRoutine))((startPt))
	#define CallInsertMenuProc(userRoutine, theMenu, beforeID)  \
			((InsertMenuProcPtr)(userRoutine))((theMenu), (beforeID))
#endif

static pascal SInt32 MenuSelectPatch(Point startPt)
	// This is our patch on MenuSelect.  The basic idea is to
	// call our clients to let them adjust their menus (and install
	// sub-menus by calling InsertSystemMenu), call through to
	// the old trap, call our clients to un-adjust their menus,
	// and then call HandleMenuSelect to see whether the chosen
	// menu was one of our system menus.  If it was, HandleMenuSelect
	// calls the client to tell them that their menu was chosen and
	// returns 0 so that we tell the host application that nothing
	// happened.
	//
	// Note that MenuSelect is called by MenuEvent, so this patch
	// covers that case as well.
{
	long oldA4;
	SInt32 result;
	SubSysMenuHandle tempSubSysMenus;
	
	oldA4 = SetUpA4();
	
	// Our patch should only be installed if InitMoreSystemMenus was successful.
	assert(gRootSysMenus != NULL);

	// Only do stuff if we're actually up and running.
	// If gHaveInsertedRootSysMenus is false, it means that
	// someone is calling MenuSelect before our InsertMenu patch
	// has run, ie before anyone has inserted any system menus,
	// ie before the Process Manager has launched.  I don't know
	// whether this ever happens, I'm use being cautious.
	
	// Also, if gMenuSelectState is not NULL then we're getting recursion
	// where we're not expecting it (probably from the kSystemMenuPreSelectAdjust
	// or kSystemMenuPostSelectAdjust callbacks, but possibly from another
	// trap patch), so we just call through to the system in that case.
	
	// We want debug versions to tell the client that they're recursing
	// in an unsupport fashion.
	
	assert(gMenuSelectState == kMenuSelectStateNil);
	
	if ( gHaveInsertedRootSysMenus && gMenuSelectState == kMenuSelectStateNil) {
	
		// OK, so we're up and running, so let's do our stuff.
		
		// First tell all clients that we're about to do a MenuSelect
		// (they can adjust their menus and insert sub-menus)...
		
		assert(gSubSysMenus == NULL);
		gMenuSelectState = kMenuSelectStatePre;
		CallAllClients(kSystemMenuPreSelectAdjust, NULL);
		
		// ... then call through to the old trap address...
		
		result = CallMenuSelectProc(gMenuSelectOldUPP, startPt);
		
		// ... and then tell all clients that we're done...
		
		gMenuSelectState = kMenuSelectStatePost;
		CallAllClients(kSystemMenuPostSelectAdjust, NULL);
		
		// Things get tricky here.  In order to allow the client
		// to post modal dialogs from its kSystemMenuActOnChosen callback,
		// we must be re-entrant at the time we call HandleMenuSelect
		// (because the ModalDialog routine will call MenuSelect).
		// But we want to avoid having sub-menus in the menu bar when
		// we're recursively called because we're going to call the
		// client's kSystemMenuPreSelectAdjust callback, which will
		// attempt to re-insert those menus.
		//
		// So, we remove our sub-menus now, before calling the
		// kSystemMenuActOnChosen callback, then we make the callback,
		// and then we dispose of the memory we used to track the
		// sub-menus.
		
		tempSubSysMenus = gSubSysMenus;
		gSubSysMenus = NULL;
		
		DeleteSubMenus(tempSubSysMenus);

		gMenuSelectState = kMenuSelectStateNil;

		// At this point we're prepared to accept recursion.
		// Call HandleMenuSelect to process the results
		// from the menu operation and call the appropriate
		// client if it was one of our menus.
		
		result = HandleMenuSelect(result, tempSubSysMenus);

		// Clean up.  If we created any sub-menu info,
		// dispose of it.
		
		if (tempSubSysMenus != NULL) {
			DisposeHandle( (Handle) tempSubSysMenus);
			assert(MemError() == noErr);
		}
	} else {
		result = CallMenuSelectProc(gMenuSelectOldUPP, startPt);
	}

	assert(gMenuSelectState == kMenuSelectStateNil);

	(void) SetA4(oldA4);
	
	return result;
}

static void InsertMySystemMenus(SInt16 whereToInsert)
	// This routine is called by InsertMenuPatch to actually
	// insert our list of system menus (stored in gRootSysMenus)
	// into the system menu list.  It does this by iterating
	// through the list and, for each element, assigning the menu
	// a unique ID in the system menu range and then calling through
	// to the old InsertMenu trap to actually put it in the system menu
	// list.
{
	ItemCount systemMenuCount;
	ItemCount systemMenuIndex;
	SInt16 newSystemMenuID;
	MenuRef thisMenu;
	
	// For each menu in our list of system menus...
	
	systemMenuCount = CountRootSysMenus();
	for (systemMenuIndex = 0; systemMenuIndex < systemMenuCount; systemMenuIndex++) {
	
		// ... find a unique menu ID in the system menu range...
		
		newSystemMenuID = FindUniqueSystemMenuID();

		// ... fix up the menu ID in the MenuRef we're about to insert...

		thisMenu = (*gRootSysMenus)[systemMenuIndex].theMenu;
		assert(ValidateSystemMenuRef(thisMenu));
		(**thisMenu).menuID = newSystemMenuID;
		
		// ... and then insert the menu into the menu list.
		
		CallInsertMenuProc(gInsertMenuOldUPP, thisMenu, whereToInsert);
	}
}

static pascal void InsertMenuPatch(MenuHandle theMenu, SInt16 beforeID)
	// This is our patch on InsertMenu.  The only goal of this
	// patch is to watch for the first person to insert a system
	// menu and then, when they do that, turn around and insert
	// our system menus along with it.  This patch typically
	// executes at Process Manager startup time as the Help menu
	// is inserted.  The nice thing about this is that we don't
	// have to monkey with the system state so that our menus
	// go into the system menu list; the system has already done
	// that, our menus just tag along for the ride.
{
	long oldA4;
	
	oldA4 = SetUpA4();
	
	// Our patch should only be installed if InitMoreSystemMenus was successful.
	assert(gRootSysMenus != NULL);
	
	CallInsertMenuProc(gInsertMenuOldUPP, theMenu, beforeID);
	
	if ( !gHaveInsertedRootSysMenus && IsSystemMenuID( (**theMenu).menuID ) ) {

		// Remember the system's MenuCInfo in gRootSysMenuCInfo
		// so that we can set it back later.  We currently
		// don't use this information but I have a feeling
		// we'll need it in the future.
		
		gRootSysMenuCInfo = LMGetMenuCInfo();
		
		// Insert our menus into the system menu list.
		
		InsertMySystemMenus((**theMenu).menuID);
		
		// We only try to do this once.  If we fail, too bad,
		// there's no one to report an error to.
		
		gHaveInsertedRootSysMenus = true;
	}

	(void) SetA4(oldA4);
}

/////////////////////////////////////////////////////////////////
#pragma mark ----- Startup -----

extern pascal OSStatus InitMoreSystemMenus(void)
	// See comment in interface part.
{
	OSStatus err;
	UniversalProcPtr gMenuSelectPatchUPP;		// points to MenuSelectPatch
	UniversalProcPtr gInsertMenuPatchUPP;		// points to InsertMenuPatch
	
	// We need to use the callback routine SetUpA4 to set up
	// A4 in our trap patches.  In preparation for that, we
	// need to remember A4 in some PC-relative place.  The client
	// may have already done this, but doing it twice doesn't hurt.
	//
	// Note that this translates to a NOP for the CFM build.
	
	RememberA4();
	
	assert(IsSystemStartup());
	
	// To avoid allocating extra pointer blocks for each of our
	// routine descriptors, we just declare them statically
	// here, and then setup the gFooUPP pointers to point to
	// them.
	//
	// Of course, for the classic 68K case, we can just use
	// assign them directly.
	
	// Build our routine descriptors.
	
	#if TARGET_RT_MAC_CFM
		{
			static RoutineDescriptor gDetachIconActionRD = BUILD_ROUTINE_DESCRIPTOR(uppIconActionProcInfo, DetachIconAction);
			static RoutineDescriptor gMenuSelectPatchRD = BUILD_ROUTINE_DESCRIPTOR(uppMenuSelectProcInfo, MenuSelectPatch);
			static RoutineDescriptor gInsertMenuPatchRD = BUILD_ROUTINE_DESCRIPTOR(uppInsertMenuProcInfo, InsertMenuPatch);
			gDetachIconActionUPP = &gDetachIconActionRD;
			gMenuSelectPatchUPP = &gMenuSelectPatchRD;
			gInsertMenuPatchUPP = &gInsertMenuPatchRD;
		}
	#else
		gDetachIconActionUPP = (IconActionUPP)    DetachIconAction;
		gMenuSelectPatchUPP  = (UniversalProcPtr) MenuSelectPatch;
		gInsertMenuPatchUPP  = (UniversalProcPtr) InsertMenuPatch;
	#endif
	
	// Setup our global state.
	
	gMenuSelectState = kMenuSelectStateNil;
	gRootSysMenus = (RootSysMenuHandle) NewHandleSys(0);
	err = MemError();

	// If all goes well, install our patches.
	
	if (err == noErr) {
		gMenuSelectOldUPP = GetToolboxTrapAddress(_MenuSelect);
		gInsertMenuOldUPP = GetToolboxTrapAddress(_InsertMenu);

		SetToolboxTrapAddress(gMenuSelectPatchUPP, _MenuSelect);
		SetToolboxTrapAddress(gInsertMenuPatchUPP, _InsertMenu);
	}
	
	return err;
}

/////////////////////////////////////////////////////////////////
