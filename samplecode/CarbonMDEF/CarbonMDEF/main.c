/*
	File:		main.c

	Contains:	Test application for a custom MDEF.

	Version:	1.0

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

	Copyright © 2000-2001 Apple Computer, Inc., All Rights Reserved
*/

#define TARGET_API_MAC_CARBON 1
#include <Carbon.h>

#include "Sample.h"

static void		AddSampleItems( MenuRef menu );
static void		AddShellItems( MenuRef menu );

int main()
{
    IBNibRef 		nibRef;
    WindowRef 		window;
    OSStatus		err;
	MenuDefSpec		defSpec;
	MenuRef			menu;
	SInt32			gestaltResult;

    // Create a Nib reference passing the name of the nib file (without the .nib extension)
    // CreateNibReference only searches into the application bundle.
    err = CreateNibReference(CFSTR("main"), &nibRef);
    require_noerr( err, CantGetNibRef );
    
    // Once the nib reference is created, set the menu bar. "MainMenu" is the name of the menu bar
    // object. This name is set in InterfaceBuilder when the nib is created.
    err = SetMenuBarFromNib(nibRef, CFSTR("MenuBar"));
    require_noerr( err, CantSetMenuBar );
    
    // Add a Quit item if we're not running on X
    Gestalt( gestaltMenuMgrAttr, &gestaltResult );
    if ( ( gestaltResult & gestaltMenuMgrAquaLayoutMask ) == 0 )
    {
    	MenuRef rootMenu = AcquireRootMenu();
    	MenuItemIndex item;
    	
    	GetMenuItemHierarchicalMenu( rootMenu, 2, &menu );
    	AppendMenuItemTextWithCFString( menu, CFSTR("Quit"), 0, kHICommandQuit, &item );
    	SetMenuItemCommandKey( menu, item, false, 'Q' );
    	
    	ReleaseMenu( rootMenu );
    }
    
	defSpec.defType = kMenuDefProcPtr;
	defSpec.u.defProc = NewMenuDefUPP( SampleMDEF );
	
	// Create a standard menu
	CreateNewMenu( 200, 0, &menu );
	SetMenuTitleWithCFString( menu, CFSTR("Sample") );
	InsertMenu( menu, 0 );
	AddSampleItems( menu );
	
	// Create a custom menu
	CreateCustomMenu( &defSpec, 201, 0, &menu );
	SetMenuTitleWithCFString( menu, CFSTR("Sample [Custom]") );
	InsertMenu( menu, 0 );
	AddSampleItems( menu );
	
	// Create a standard menu
	CreateNewMenu( 202, 0, &menu );
	SetMenuTitleWithCFString( menu, CFSTR("Shell") );
	InsertMenu( menu, 0 );
	AddShellItems( menu );
	
	// Create a custom menu
	CreateCustomMenu( &defSpec, 203, 0, &menu );
	SetMenuTitleWithCFString( menu, CFSTR("Shell [Custom]") );
	InsertMenu( menu, 0 );
	AddShellItems( menu );
	
	// Create a standard menu
	CreateNewMenu( 204, 0, &menu );
	SetMenuTitleWithCFString( menu, CFSTR("Fonts") );
	InsertMenu( menu, 0 );
	CreateStandardFontMenu( menu, 0, 0, 0, NULL );
	
	// Create a custom menu
	CreateCustomMenu( &defSpec, 205, 0, &menu );
	SetMenuTitleWithCFString( menu, CFSTR("Fonts [Custom]") );
	InsertMenu( menu, 0 );
	CreateStandardFontMenu( menu, 0, 0, 0, NULL );
	
    // Then create a window. "MainWindow" is the name of the window object. This name is set in 
    // InterfaceBuilder when the nib is created.
    err = CreateWindowFromNib(nibRef, CFSTR("MainWindow"), &window);
    require_noerr( err, CantCreateWindow );

    // We don't need the nib reference anymore.
    DisposeNibReference(nibRef);
    
    // The window was created hidden so show it.
    ShowWindow( window );
    
    // Call the event loop
    RunApplicationEventLoop();

CantCreateWindow:
CantSetMenuBar:
CantGetNibRef:
	return err;
}

// add some interesting sample items
static void
AddSampleItems( MenuRef menu )
{
	MenuItemIndex	item;
	
	AppendMenuItemTextWithCFString( menu, CFSTR("Checkmark"), 0, 0, &item );
	CheckMenuItem( menu, item, true );
	AppendMenuItemTextWithCFString( menu, CFSTR("Dash"), 0, 0, &item );
	SetItemMark( menu, item, '-' );
	AppendMenuItemTextWithCFString( menu, CFSTR("Diamond"), 0, 0, &item );
	SetItemMark( menu, item, kDiamondCharCode );
	AppendMenuItemTextWithCFString( menu, CFSTR("Bullet"), 0, 0, &item );
	SetItemMark( menu, item, kBulletCharCode );
	AppendMenuItemTextWithCFString( menu, NULL, kMenuItemAttrSeparator, 0, NULL );
	AppendMenuItemTextWithCFString( menu, CFSTR("Section Header"), kMenuItemAttrSectionHeader, 0, NULL );
	AppendMenuItemTextWithCFString( menu, CFSTR("Indented item 1"), 0, 0, &item );
	SetMenuItemIndent( menu, item, 1 );
	AppendMenuItemTextWithCFString( menu, CFSTR("Indented item 2"), 0, 0, &item );
	SetMenuItemIndent( menu, item, 1 );
}

// create the Shell menu in Terminal
static void
AddShellItems( MenuRef menu )
{
	MenuItemIndex	item;
	
	AppendMenuItemTextWithCFString( menu, CFSTR("New"), 0, 0, &item );
	SetItemCmd( menu, item, 'N' );
	AppendMenuItemTextWithCFString( menu, CFSTR("Open..."), 0, 0, &item );
	SetItemCmd( menu, item, 'O' );
	AppendMenuItemTextWithCFString( menu, CFSTR("Library"), kMenuItemAttrDisabled, 0, &item );
	SetMenuItemHierarchicalID( menu, item, 201 );
	AppendMenuItemTextWithCFString( menu, NULL, kMenuItemAttrSeparator, 0, NULL );
	AppendMenuItemTextWithCFString( menu, CFSTR("Save"), 0, 0, &item );
	SetItemCmd( menu, item, 'S' );
	AppendMenuItemTextWithCFString( menu, CFSTR("Save As..."), 0, 0, &item );
	SetItemCmd( menu, item, 'S' );
	SetMenuItemModifiers( menu, item, kMenuShiftModifier );
	AppendMenuItemTextWithCFString( menu, CFSTR("Save Text As..."), 0, 0, &item );
	SetItemCmd( menu, item, 'S' );
	SetMenuItemModifiers( menu, item, kMenuOptionModifier );
	AppendMenuItemTextWithCFString( menu, CFSTR("Save Selected Text As..."), 0, 0, &item );
	SetItemCmd( menu, item, 'S' );
	SetMenuItemModifiers( menu, item, kMenuShiftModifier | kMenuOptionModifier );
	AppendMenuItemTextWithCFString( menu, NULL, kMenuItemAttrSeparator, 0, NULL );
	AppendMenuItemTextWithCFString( menu, CFSTR("Run Command..."), 0, 0, &item );
	SetItemCmd( menu, item, 'N' );
	SetMenuItemModifiers( menu, item, kMenuShiftModifier );
	AppendMenuItemTextWithCFString( menu, CFSTR("Set Title..."), 0, 0, &item );
	SetItemCmd( menu, item, 'T' );
	SetMenuItemModifiers( menu, item, kMenuShiftModifier );
	AppendMenuItemTextWithCFString( menu, CFSTR("Inspector..."), 0, 0, &item );
	SetItemCmd( menu, item, 'I' );
	AppendMenuItemTextWithCFString( menu, NULL, kMenuItemAttrSeparator, 0, NULL );
	AppendMenuItemTextWithCFString( menu, CFSTR("Page Setup..."), 0, 0, &item );
	SetItemCmd( menu, item, 'P' );
	SetMenuItemModifiers( menu, item, kMenuShiftModifier );
	AppendMenuItemTextWithCFString( menu, CFSTR("Print..."), 0, 0, &item );
	SetItemCmd( menu, item, 'P' );
}
