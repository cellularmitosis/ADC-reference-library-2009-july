/*
	File:		MoreSystemMenus.h

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

$Log: MoreSystemMenus.h,v $
Revision 1.5  2003/03/28 16:15:32         
Removed a bunch of #pragma import/export directives.  IIRC PCG added them for reasons I don't fully understand.  He may have meant #pragma internal.  Regardless, there presence was causing lots of MIB symbols to be CFM exported, which is wrong.  MIB is not meant to ship as a shared library.

Revision 1.4  2002/11/08 23:35:53         
Improve compile time environment check.

Revision 1.3  2001/11/07 15:55:40         
Tidy up headers, add CVS logs, update copyright.


         <2>     21/9/01    Quinn   Changes for CWPro7 Mach-O build.
         <1>     25/8/99    Quinn   First checked in.
*/

#pragma once

/////////////////////////////////////////////////////////////////

// MIB Setup

#include "MoreSetup.h"

#if TARGET_API_MAC_CARBON
	// The architecture used by MoreSystemMenus relies on the fact
	// that system menus can be created by a system extension (INIT).
	// As Carbon does not support the building of system extensions,
	// this module can not be used by Carbon code.

	#error MoreSystemMenus does not support Carbon development.
	
	// This also precludes Mach-O development.
#endif

// Mac OS Interfaces

#if ! MORE_FRAMEWORK_INCLUDES
	#include <MacTypes.h>
	#include <Menus.h>
	#include <Icons.h>
#endif

/////////////////////////////////////////////////////////////////

#ifdef __cplusplus
	extern "C" {
#endif

/////////////////////////////////////////////////////////////////

/*	Overall Comments
	----------------
	
	o System menus are *bad* human interface design!  Apple
	  recognises this fundamental truth, and tries to minimise
	  its use of system menus.  For example, earlier versions of
	  the system included a system menu to choose the current
	  printer; that function has moved to the control strip
	  in current system software.

	o System menus are *bad* engineering.  To implement a system
	  menu, you must patch traps and muck with undocumented low-memory
	  globals.  Doing this will introduce a compatibility liability
	  in your product.  Specifically, using a system menu will
	  guarantee that your product is *not* Carbon compatible.
	
	o Apple will not introduce an API for system menus in Carbon
	  (or any other system) because system menus are bad HI design.
	
	o This code exists because:
	
		o We (DTS) recognise that you (third party developers) are going
		  to implement system menus regardless of how much we tell you 
		  not to, often because your marketting organisation demands them.
		  Furthermore, users will install your products without knowing
		  that they are based on unsupport technology.  If you implement
		  system menus badly, you will compromise the stability of user
		  systems, which is not in Apple's interest.
		
		o It's in Apple's interest for all system menus to be implemented
		  in the same way.  This makes it easier for our engineering 
		  organisation to maintain compatibility between releases.
		  This is no guarantee of compatibility, of course, but it's
		  always safer in a big herd than a small herd.
	
	o The routines described below are *SAMPLE CODE*, not an
	  Apple-sanctioned programming interface.  Read the disclaimer at
	  the top of this file for more details of what this means.
	
	o All routines must be called at system task time.

	o Be wary of menu IDs shifting out from underneath you.
	  See the comments for InsertSystemMenu and InsertSystemSubMenu
	  for details.

	o Note on Recursion -- When writing system menu code, you have
      to be very careful to avoid dying because of recursion.
      The MoreSystemMenus calls your callback from a patch on
      MenuSelect.  If your callback calls MenuSelect (either
      directly, or indirectly, for example by calling ModalDialog
      or something that calls ModalDialog), then MenuSelect
      is being called recursively.
      
	  There are two distinct cases.  MoreSystemMenus handles against
	  re-entrancy from your kSystemMenuActOnChosen callback.  For
	  example, if your kSystemMenuActOnChosen callback poses a modal
	  dialog, which in turn calls MenuSelect, all will be well.
	  
	  However, MoreSystemMenus doesn't handle recursion from your
	  kSystemMenuPreSelectAdjust and kSystemMenuPostSelectAdjust
	  callbacks.  If these callbacks call MenuSelect, MoreSystemMenus
	  will not crash, but it will also not handle any selections
	  on your menus.  You should avoid making calls to MenuSelect
	  from these callbacks.

	  Also, your own code must guard against recursion, even in
	  the kSystemMenuActOnChosen.  For example, if you have a 
	  system menu whose callback poses a modal dialog, you should 
	  be aware that the callback might be called again from
	  within ModalDialog.  If you don't want this to happen,
	  you should set a global variable (before calling ModalDialog)
	  that causes your kSystemMenuPreSelectAdjust to disable the
	  menu item for menu operations while the modal dialog is up.

    o Command Keys -- MoreSystemMenus does not handle command keys
      in system menus.  Extending it to do this would be relatively
      easy (it would need to patch more traps, specifically MenuKey
      and MenuEvent) but I didn't do this because the human interface
      of putting command keys on system menus is even more questionable
      than the HI of system menus themselves (-:
*/

/////////////////////////////////////////////////////////////////

// General utility routines

extern pascal MenuRef GetNewSystemMenu(SInt16 menuResourceID);
	// This routine creates a new menu in the system heap based
	// on the 'MENU' resource with the given ID.
	//
	// The menu is detached from the resource file, so you can
	// safely close the resource file containing the 'MENU' resource.
	// Also, this means you must DisposeMenu the menu if you don't
	// insert it into the menu bar.
	//
	// Calling this routine twice will yield two distinct menus,
	// so the semantics are somewhat different from the classic
	// GetMenu (but close to the Carbon semantics).

extern pascal MenuRef NewSystemMenu(ConstStr255Param title);
	// This routine creates a new, empty menu in the system heap.
	// It's a close analogue to NewMenu.

extern pascal OSStatus GetDetachedIconSuite(IconSuiteRef *theIconSuite, SInt16 theResID, IconSelectorValue selector);
	// This routine is a direct analogue of GetIconSuite except
	// that the icons in the suite are detached from their parent
	// resource file.

extern pascal void IconSuiteToSystemMenuTitle(IconSuiteRef theIconSuite, StringPtr menuTitle);
	// This routine sets menuTitle such that the title will draw
	// theIconSuite instead of text.  This only takes 6 bytes of
	// storage, so we pass in a StringPtr rather than a Str255.

extern pascal OSStatus SetMenuTitleToIconSuite(MenuRef theMenu, IconSuiteRef theIconSuite);
	// This routine sets the title of theMenu to the icon given
	// in theIconSuite.  The current title of theMenu must be exactly 5
	// characters long (otherwise you get a paramErr), although the
	// contents of the current title are ignored.
	//
	// theIconSuite is *not* owned by theMenu after this call.
	// If you dispose of the menu, you must subsequently dispose
	// of theIconSuite.

// Startup

extern pascal OSStatus InitMoreSystemMenus(void);
	// This routine initialises the MoreSystemMenus module.  It
	// must be called at system startup time.  It must be called
	// before any of the other routines in this module.  If it returns
	// an error, you must not call any other routines in this module,
	// although you are guaranteed that this module has no hooks into
	// the system and you can unload the module's code.

// Core system menu routines

typedef pascal OSStatus (*SystemMenuCallbackProc)(OSType selector,
													MenuRef theMenu,
													void *param, 
													void *refcon);
	// When adding a system menu, you must supply a pointer
	// to a callback routine with the above prototype.
	// When the system calls you back, selector is one of the messages
	// (described) below and refcon is the refcon you supplied when
	// you added the menu.  The other parameters are message-dependent.
	//
	// The callback is always made at system task time, although
	// not in any particular application's context (it will probably
	// be the context of the frontmost application).  68K code must
	// set up its global world register (A4 for CodeWarrior code resources
	// A5 for MPW code resources) before accessing any global variables.
	//
	// This is not a UPP because you get the source code to the
	// MoreSystemMenus module; making it a UPP would complicate things
	// without any real benefit.

enum {
	kSystemMenuPreSelectAdjust  = 'prsl',
		// MoreSystemMenus calls your SystemMenuCallbackProc routine
		// with this selector to tell you to adjust the
		// specified root system menu prior to the user
		// being able to browse the menus (ie from a head patch
		// on MenuSelect).  Typically this involves enabling
		// or disabling items, but you can also add sub-menus
		// using InsertSystemSubMenu.  If you add a sub-menu,
		// you are also responsible for adjusting it before
		// returning from this call.
		//
		// o theMenu is the MenuHandle of the root system menu
		// o param is not defined
		//
		// Your implementation of this selector should not
		// call MenuSelect (directly or indirectly).
		// See "Notes on Recursion" (above) for more details.

	kSystemMenuPostSelectAdjust = 'posl',
		// MoreSystemMenus calls your SystemMenuCallbackProc routine
		// with this selector after the user has finished
		// choosing a menu item (ie a tail patch
		// on MenuSelect).  You can use this to 'de-adjust'
		// your system menus (if necessary).
		//
		// o theMenu is the MenuHandle of the root system menu
		// o param is not defined
		//
		// Your implementation of this selector should not
		// call MenuSelect (directly or indirectly).
		// See "Notes on Recursion" (above) for more details.

	kSystemMenuActOnChosen      = 'act!'
		// MoreSystemMenus calls your SystemMenuCallbackProc routine
		// with this selector when one of your menu items
		// has been chosen by the user.  You must act on that
		// item, by doing whatever your extension does.
		//
		// o theMenu is MenuHandle of the chosen menu.
		//   This may be the root system menu or one
		//   of your hierarchical sub-menus added by
		//   InsertSystemSubMenu.
		// o param is the item index of the chosen item
		//   within theMenu
		//
		// Your implementation of this selector *may*
		// call MenuSelect (directly or indirectly).
		// Specifically, you can call ModalDialog while
		// handling this selector.
		// See "Notes on Recursion" (above) for more details.
};

extern pascal OSStatus InsertSystemMenu(MenuRef theMenu,
										SystemMenuCallbackProc callback, void *refcon);
	// You can call this routine to add a menu to the system
	// part of the menu bar.  [This is known as a "root" menu.]
	// theMenu is a reference to the menu to add.  It must be a
	// memory handle (as opposed to a resource handle) in the
	// system heap.  Other utility routines in this module
	// (GetNewSystemMenu and NewSystemMenu) create menus that
	// way.  callback must be a pointer to your callback routine,
	// as described above.  refcon is passed to your callback routine
	// without interpretation.
	//
	// This routine must be called during startup time, ie before
	// the Process Manager is launched.  Any attempt to call it after
	// will not be effective (debug builds will DebugStr).
	//
	// The menu you add will not appear in the menu bar until
	// the Process Manager is launched.  Moreover, the menu ID
	// of the menu will not be decided until it is added to the
	// menu bar.  Any time you need to operate on the menu,
	// use the MenuRef.  If you must use the menu ID (because an
	// API requires it), extract it from the MenuRef immediately
	// before using it and don't cache it.
	//
	// Menus appear in the menu bar in the order in which you add
	// them, with the first menu appearing on the left.  Also,
	// if two distinct extensions that both use this module are
	// installed, the first one's menus will appear to the left
	// of the second's.

extern pascal OSStatus InsertSystemSubMenu(MenuRef rootMenu,
										MenuRef parentMenu, UInt16 itemInParent, MenuRef childMenu);
	// You can call this routine to add a hierarchical sub-menu
	// to the menu bar.  You must call this routine in the context
	// of a kSystemMenuPreSelectAdjust.  You must pass in rootMenu 
	// the MenuRef of a system menu you registered using InsertSystemMenu.
	// When an item on your sub-menu is chosen, the callback for
	// the rootMenu will be called.  parentMenu is the menu in which
	// the sub-menu is to appear.  This is typically rootMenu, unless
	// you have sufficient bad taste to use multi-level
	// hierarchical menus.  itemInParent is the item number where
	// the sub-menu is to appear in the parent.  This must be between
	// 1 and CountMenuItems(parentMenu), inclusive.  childMenu is the
	// sub-menu itself.
	//
	// MoreSystemMenus will add childMenu to the menu bar with a
	// temporary menu ID and set itemInParent of parentMenu to reference
	// that temporary menu ID.  Before returning from its patch on MenuSelect,
	// MoreSystemMenus will remove any these sub-menus from the menu
	// bar (so that they don't conflict with application menus).
	// Therefore, your callback must respond to kSystemMenuPreSelectAdjust 
	// by inserting its sub-menus each time it is called.  Also, you must not
	// rely on the menu ID of the sub-menu being constant, as the menu
	// is dynamically renumbered each time it's added to the menu bar.

/////////////////////////////////////////////////////////////////

#ifdef __cplusplus
	}
#endif
