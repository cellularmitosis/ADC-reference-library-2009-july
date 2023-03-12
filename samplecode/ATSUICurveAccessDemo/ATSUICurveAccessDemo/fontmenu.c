/*
	File:		fontmenu.c

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

                Copyright © 2002 Apple Computer, Inc., All Rights Reserved
*/

#include "globals.h"
#include "fontmenu.h"


// Globals
//
static MenuID                   gFontMenuID;
static MenuRef                  gFontMenuCurrentMenuRef;
static MenuItemIndex            gFontMenuCurrentMenuItem;
static MenuItemIndex            *gFontMenuHierarchicalItems;
static ItemCount                gNumHierarchicalItems;


// Builds and attaches the font menu in the specified menu
//
OSStatus InstallFontMenu(MenuID menuID)
{
    OSStatus status;

    // Initialize globals
    gFontMenuID = menuID;
    gFontMenuCurrentMenuRef = GetMenuRef(gFontMenuID);
    gFontMenuCurrentMenuItem = (MenuItemIndex)-1; 

    // Create the standard font menu
    status = CreateStandardFontMenu(GetMenuRef(gFontMenuID), 0, (gFontMenuID+1), kHierarchicalFontMenuOption, &gNumHierarchicalItems);
    if ( status != noErr) return status;
   
    // Remember where all the submenus are
    BuildFontMenuParentItemArray();
    
    return status;
}


// Creates a global array containing information about the submenus in the hierarchical font menu.
// This information is then used by GetFontMenuParentItem.
//
void BuildFontMenuParentItemArray(void)
{
    ItemCount               numItems;
    int                     i, currentIndex;
    MenuRef                 theSubMenu;

    // Build an array of item indexes which have hierarchical menus -- used in GetFontMenuParentItem (see below)
    gFontMenuHierarchicalItems = (MenuItemIndex *) malloc(sizeof(MenuItemIndex) * gNumHierarchicalItems);

    currentIndex = 0;
    numItems = CountMenuItems(GetMenuRef(gFontMenuID));

    for (i=1; i <= numItems; i++) {

        verify_noerr( GetMenuItemHierarchicalMenu(GetMenuRef(gFontMenuID), i, &theSubMenu) );

        if ( theSubMenu != NULL ) {
            gFontMenuHierarchicalItems[currentIndex++] = i;
        }
    }

    check( currentIndex == gNumHierarchicalItems );     // Make sure we found the right number
}


// Gets the parent item of a font in a submenu.  Returns zero if not found.
//
MenuItemIndex GetFontMenuParentItem(MenuRef inMenu)
{
    MenuRef                 theSubMenu;
    int                     i;
    
    for(i=0; i < gNumHierarchicalItems; i++) {
        verify_noerr( GetMenuItemHierarchicalMenu(GetMenuRef(gFontMenuID), gFontMenuHierarchicalItems[i], &theSubMenu) );
        if ( theSubMenu == inMenu ) {
            return gFontMenuHierarchicalItems[i];
        }
    }
    return 0;
}


// Handles changes to the font menu
//
FMFont SelectAndGetFont(MenuRef theMenu, MenuItemIndex theItem)
{
    MenuRef                 parentMenuRef;
    MenuItemIndex           parentMenuItem;
    FMFontFamily            theFMFontFamily;
    FMFontStyle             theFMFontStyle;
    FMFont                  theFont;

    // Uncheck the previous item (if any)
    if (gFontMenuCurrentMenuItem != (MenuItemIndex)-1) {
        CheckMenuItem(gFontMenuCurrentMenuRef, gFontMenuCurrentMenuItem, false);
        if ( GetMenuID(gFontMenuCurrentMenuRef) > gFontMenuID ) {		// Sub-menus will have MenuIDs starting at gFontMenuID + 1
            parentMenuRef = GetMenuRef(gFontMenuID);
            parentMenuItem = GetFontMenuParentItem(gFontMenuCurrentMenuRef);
            if ( parentMenuItem > 0 ) {
                SetItemMark(parentMenuRef, parentMenuItem, kMenuNoMark);
            }
        }
    }

    // Check the new item
    CheckMenuItem(theMenu, theItem, true);
    if ( GetMenuID(theMenu) > gFontMenuID ) {							// Sub-menus will have MenuIDs starting at gFontMenuID + 1
        parentMenuRef = GetMenuRef(gFontMenuID);
        parentMenuItem = GetFontMenuParentItem(theMenu);
        if ( parentMenuItem > 0 ) {
            SetItemMark(parentMenuRef, parentMenuItem, kMenuDashMark);
        }
    }
    
    // Store the current item in the globals for future reference
    gFontMenuCurrentMenuRef = theMenu;
    gFontMenuCurrentMenuItem = theItem;

    // Return the proper font
    verify_noerr( GetFontFamilyFromMenuSelection(gFontMenuCurrentMenuRef, gFontMenuCurrentMenuItem, &theFMFontFamily, &theFMFontStyle) );
    verify_noerr( FMGetFontFromFontFamilyInstance(theFMFontFamily, theFMFontStyle, &theFont, NULL) );
    return theFont;
}


// Walks the font menu, finds and selects the specified font
// Returns false if the font could not be found
//
Boolean FindAndSelectFont(FMFont iFont)
{
    FMFontFamily            theFMFontFamily;
    FMFontStyle             theFMFontStyle;
    FMFont					currentFont;
    ItemCount               numItems, numSubItems;
    MenuRef                 theSubMenu = NULL;
    MenuItemIndex			parentMenuItem, i, j;
    Boolean					found = false;


    // Loop over all the parent menu items (outer loop)
    numItems = CountMenuItems(GetMenuRef(gFontMenuID));
    for (i=1; i <= numItems; i++) {

        // Check to see if this item is hierarchical
        verify_noerr( GetMenuItemHierarchicalMenu(GetMenuRef(gFontMenuID), i, &theSubMenu) );
        if ( theSubMenu != NULL ) {

            // Loop over all the submenu items (inner loop)
            numSubItems = CountMenuItems(theSubMenu);
            for (j=1; j <= numSubItems; j++) {
                verify_noerr( GetFontFamilyFromMenuSelection(theSubMenu, j, &theFMFontFamily, &theFMFontStyle) );
                verify_noerr( FMGetFontFromFontFamilyInstance(theFMFontFamily, theFMFontStyle, &currentFont, NULL) );
                
                if (currentFont == iFont) {
                    found = true;
                    break;
                }
            } // End of inner loop
        }
        else {
            verify_noerr( GetFontFamilyFromMenuSelection(GetMenuRef(gFontMenuID), i, &theFMFontFamily, &theFMFontStyle) );
            verify_noerr( FMGetFontFromFontFamilyInstance(theFMFontFamily, theFMFontStyle, &currentFont, NULL) );
            
            if (currentFont == iFont) {
                found = true;
            }
        }

        if (found) break;
    } // End of outer loop
    
    
    // If the font was found, check its menu item (uncheck the previous one, if there is one)
    //
    if (found) {
        // Uncheck the previous item (if any)
        if (gFontMenuCurrentMenuItem != (MenuItemIndex)-1) {
            CheckMenuItem(gFontMenuCurrentMenuRef, gFontMenuCurrentMenuItem, false);
            if ( GetMenuID(gFontMenuCurrentMenuRef) > gFontMenuID ) {		// Sub-menus will have MenuIDs starting at gFontMenuID + 1
                parentMenuItem = GetFontMenuParentItem(gFontMenuCurrentMenuRef);
                if ( parentMenuItem > 0 ) {
                    SetItemMark(GetMenuRef(gFontMenuID), parentMenuItem, kMenuNoMark);
                }
            }
        }
    
        // Check the new item
        if (theSubMenu != NULL) {
            gFontMenuCurrentMenuRef = theSubMenu;
            gFontMenuCurrentMenuItem = j;
            SetItemMark(GetMenuRef(gFontMenuID), i, kMenuDashMark);
        }
        else {
            gFontMenuCurrentMenuRef = GetMenuRef(gFontMenuID);
            gFontMenuCurrentMenuItem = i;
        }
        CheckMenuItem(gFontMenuCurrentMenuRef, gFontMenuCurrentMenuItem, true);
    }

    return found;
}