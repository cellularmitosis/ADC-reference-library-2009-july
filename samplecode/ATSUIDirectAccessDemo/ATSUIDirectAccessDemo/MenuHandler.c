/*
	File:		ManuHandler.c
	
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
// This code will run on Mac OS X 10.2 (or later) ONLY!!!
#include <Carbon/Carbon.h>

#include "Window.h"
#include "HIElements.h"
#include "MenuHandler.h"
#include "DirectAccessCallbacks.h"

// ------------------------------------------------------------------------------
// Global Variables
// ------------------------------------------------------------------------------

static const char gInitialFont[] = "Geneva\0";

ATSUStyle 	gGlobalStyle = NULL;
SInt16		gOptionsMenuSelection = 0;
SInt16		gDemoMenuSelection = 0;
UInt16		gFontSize = 0;

// ------------------------------------------------------------------------------
// Function Prototypes
// ------------------------------------------------------------------------------

static OSStatus InitializeFontsMenu( MenuRef fontMenuRef, MenuRef fontSizeRef );

static SInt16 GetMenuSelection( MenuRef theMenu );

static OSStatus MenuHandler( EventHandlerCallRef inCallRef, EventRef inEvent,
	void *inUserData );
	
static OSStatus FontMenuHandler( EventHandlerCallRef inCallRef,
	EventRef inEvent, void *inUserData );
	
static OSStatus FontSizeMenuHandler( EventHandlerCallRef inCallRef,
	EventRef inEvent, void *inUserData );
	
static OSStatus SetGlobalFontFromMenuSelections( MenuRef fontMenuRef,
	UInt16 selection );
	
static OSStatus SetGlobalFontSizeFromMenuSelections( MenuRef sizeMenuRef,
	UInt16 selection );

// ------------------------------------------------------------------------------
// InitializeDemoMenus													[EXPORT]
// ------------------------------------------------------------------------------

extern 
OSStatus InitializeDemoMenus( void )
{
	static const EventTypeSpec	menuEvents[] = 
	{
		{ kEventClassCommand, kEventCommandProcess }
	};

	OSStatus 	err;
	MenuRef		demoMenuRef;
	MenuRef		optionsMenuRef;
	MenuRef		fontMenuRef;
	MenuRef		fontSizeRef;
	
    // "MainMenu" is the name of the menu bar object. This name is set in 
	// InterfaceBuilder when the nib is created.
    err = SetMenuBarFromNib(gMainNibRef, CFSTR("MenuBar"));
    require_noerr( err, InitializeDemoMenus_err );
	
	// grab the newly instantiated menu handles
	demoMenuRef = GetMenuHandle( kDemoMenuID );
	optionsMenuRef = GetMenuHandle( kOptionsMenuID );
	fontMenuRef = GetMenuHandle( kFontMenuID );
	fontSizeRef = GetMenuHandle( kFontSizeMenuID );
	
	// get the selected item for the style menu
	gDemoMenuSelection = GetMenuSelection( demoMenuRef ); 
	
	// get the selected item for the demo menu
	gOptionsMenuSelection = GetMenuSelection( optionsMenuRef );
	
	// create the standard font menu
	err = InitializeFontsMenu( fontMenuRef, fontSizeRef );
	require_noerr( err, InitializeDemoMenus_err );
	
	// now, we need to install some carbon event handlers to grab
	// the selection when it occurs so that we can re-draw the window and
	// update our global selection
	
	// Install one for the Demo Menu
	err = InstallMenuEventHandler( demoMenuRef, MenuHandler,
		GetEventTypeCount( menuEvents ), menuEvents, &gDemoMenuSelection,
		NULL );
	require_noerr( err, InitializeDemoMenus_err );
		
	// Install one for the Options Menu
	err = InstallMenuEventHandler( optionsMenuRef, MenuHandler,
		GetEventTypeCount( menuEvents ), menuEvents, &gOptionsMenuSelection,
		NULL );
	require_noerr( err, InitializeDemoMenus_err );
	
	// Install one for the Font menu
	err = InstallMenuEventHandler( fontMenuRef, FontMenuHandler,
		GetEventTypeCount( menuEvents ), menuEvents, fontMenuRef, NULL );
	require_noerr( err, InitializeDemoMenus_err );
	
	// Install one for the Font Size menu
	err = InstallMenuEventHandler( fontSizeRef, FontSizeMenuHandler,
		GetEventTypeCount( menuEvents ), menuEvents, fontSizeRef, NULL );
	require_noerr( err, InitializeDemoMenus_err );	
		
InitializeDemoMenus_err:

	return err;
	
}

// ------------------------------------------------------------------------------
// InitializeFontsMenu												[INTERNAL]
// ------------------------------------------------------------------------------

static
OSStatus InitializeFontsMenu(
	MenuRef fontMenuRef,
	MenuRef	fontSizeRef )
{
	OSStatus 		err;
	UInt16	 		i;
	UInt16			numItems;
	Str255			menuString;
	int				compareResult;
	UInt16			sizeMenuSelection;
	
	// use the Carbon-ish font menu creation API to actually create the
	// fonts menu. This is way cooler than the AppendResMenu( 'FONT' ) thing.
	err = CreateStandardFontMenu( fontMenuRef, 0, 0, 0, NULL );
	require_noerr( err, InitializeFontsMenu_err );
	
	// grab the number of items in the menu
	numItems = CountMenuItems( fontMenuRef );
	
	// loop through and look for Lucida Grande
	for ( i = 0; i < numItems; i++ )
	{
		// get the menu item text
		GetMenuItemText( fontMenuRef, i, menuString );
		
		// compare the menu item with the intial font
		compareResult = strncmp( gInitialFont, &menuString[1], 
			strlen( gInitialFont ) );
			
		// if they're equal, then we've got our font
		if ( compareResult == 0 )
		{
			break;
		}
		
	}
		
	// if i is the same as numItems, then we need to pick something else,
	// since we don't want to get the last number
	if ( i == numItems )
	{
		i = 0;
	}
	
	// create the global style
	err = ATSUCreateStyle( &gGlobalStyle );
	require_noerr( err, InitializeFontsMenu_err );
	
	// set the global style for the font menu settings
	err = SetGlobalFontFromMenuSelections( fontMenuRef, i );
	require_noerr( err, InitializeFontsMenu_err );
	
	// grab the currently checked menu item
	sizeMenuSelection = GetMenuSelection( fontSizeRef );
	
	// set the global font size for font size menu settings
	err = SetGlobalFontSizeFromMenuSelections( fontSizeRef, sizeMenuSelection );
	require_noerr( err, InitializeFontsMenu_err );
	
	// set the mark on the currently selected font. The size menu should already
	// have the check mark on it, since it's defined in the .nib file.
	MacCheckMenuItem( fontMenuRef, i, true );
	
InitializeFontsMenu_err:

	return err;

}

// ------------------------------------------------------------------------------
// SetGlobalFontFromMenuSelections									[INTERNAL]
// ------------------------------------------------------------------------------

static
OSStatus SetGlobalFontFromMenuSelections(
	MenuRef		fontMenuRef,
	UInt16		selection )
{
	OSStatus				err;
	FMFontFamily			fontFamily;
	FMFontStyle				fontStyle;
	ATSUFontID				fontID;
	ATSUAttributeTag		tag = kATSUFontTag;
	ByteCount				valueSize = sizeof( ATSUFontID );
	ATSUAttributeValuePtr	valuePtr = &fontID;
	
	// get the font family and style from the font menu selection
	err = GetFontFamilyFromMenuSelection( fontMenuRef, selection, &fontFamily,
		&fontStyle );
	require_noerr( err, SetGlobalFontFromMenuSelections_err );
	
	// get the ATSUFontID from the font family and style that was selected
	err = ATSUFONDtoFontID( fontFamily, fontStyle, &fontID );
	require_noerr( err, SetGlobalFontFromMenuSelections_err );	
		
	// now, we just need to set this info in the global style
	err = ATSUSetAttributes( gGlobalStyle, 1, &tag,  &valueSize, &valuePtr );
	require_noerr( err, SetGlobalFontFromMenuSelections_err );
	
	// update any of the callback contexts based on the new menu selections
	UpdateCallbackContexts();
	
SetGlobalFontFromMenuSelections_err:

	return err;
	
}

// ------------------------------------------------------------------------------
// SetGlobalFontSizeFromMenuSelections								[INTERNAL]
// ------------------------------------------------------------------------------

static
OSStatus SetGlobalFontSizeFromMenuSelections(
	MenuRef		sizeMenuRef,
	UInt16		selection )
{
	OSStatus				err;
	Fixed					fontSize;
	ATSUAttributeTag		tag = kATSUSizeTag;
	ByteCount				valueSize = sizeof( Fixed );
	ATSUAttributeValuePtr	valuePtr = &fontSize;
	
	// calculate the bit size of the font
	gFontSize = selection * kFontSizeMinimum;
	
	// Make sure that the font size is converted to a Fixed.
	fontSize = gFontSize << 16;
	
	// now, we just need to set this info in the global style
	err = ATSUSetAttributes( gGlobalStyle, 1, &tag,  &valueSize, &valuePtr );
	require_noerr( err, SetGlobalFontSizeFromMenuSelections_err );
	
	// update any of the callback contexts based on the new menu selections
	UpdateCallbackContexts();
	
SetGlobalFontSizeFromMenuSelections_err:

	return err;

}	

// ------------------------------------------------------------------------------
// ProcessMenuSelection												[INTERNAL]
// ------------------------------------------------------------------------------

static
OSStatus ProcessMenuSelection(
	EventRef	inEvent,
	SInt16		*currentItem,
	Boolean		*needToInvalidate )
{
	OSStatus	err;
	HICommand 	command;	

	// assume that everything's fine
	*needToInvalidate = false;

	// grab the menu item that was hit
	err = GetEventParameter( inEvent, kEventParamDirectObject,
		typeHICommand, NULL, sizeof( HICommand ), NULL,
		&command );
	require_noerr( err, ProcessMenuSelection_err );
	
	// check to see if the old item and the new item are the same. We don't
	// need to do anything if the user simply selected the same thing
	if ( *currentItem != command.menu.menuItemIndex )
	{	
		// uncheck the old item
		MacCheckMenuItem( command.menu.menuRef, *currentItem, false );
		
		// check the menu item that was hit
		MacCheckMenuItem( command.menu.menuRef, command.menu.menuItemIndex,
			true );
			
		// set the current item to the new item
		*currentItem = command.menu.menuItemIndex;
		
		// we need to invalidate, so go ahead and return that
		*needToInvalidate = true;
	}
	
ProcessMenuSelection_err:
		
	return err;
}


// ------------------------------------------------------------------------------
// MenuHandler														[INTERNAL]
// ------------------------------------------------------------------------------

static
OSStatus MenuHandler(
	EventHandlerCallRef inCallRef,
	EventRef 			inEvent,
	void				*inUserData )
{

	OSStatus 	err = eventNotHandledErr;
	Boolean		needToInvalidate = false;
	UInt16		*currentSelection;
	
	// cast away the user data, since it's just the current selection
	currentSelection = (SInt16 *) inUserData;

	// make sure that we're processing a command event
	if ( GetEventClass( inEvent ) == kEventClassCommand )
	{
		switch ( GetEventKind( inEvent ) )
		{
		
			// see if this is a process event
			case kEventCommandProcess:
			
				// It is, so process the event
				err = ProcessMenuSelection( inEvent, currentSelection,
					&needToInvalidate );
				require_noerr( err, DemoMenuHandler_err );
				break;
			
			// the default case is to simply fall through and return
			// eventNotHandledErr.
			default:
				break;
			
		}
		
	}
	
	
	// check to see if we need to invalidate. If so, then invalidate and draw
	// all of the windows
	if ( needToInvalidate == true )
	{
		// redraw the windows
		InvalidateAndRedrawWindows();
	}

DemoMenuHandler_err:	

	return err;
}

// ------------------------------------------------------------------------------
// FontMenuHandler													[INTERNAL]
// ------------------------------------------------------------------------------

static
OSStatus FontMenuHandler(
	EventHandlerCallRef	inCallRef,
	EventRef 			inEvent,
	void 				*inUserData )
{
	OSStatus	err = eventNotHandledErr;
	MenuRef		menuRef;
	SInt16		menuSelection;
	Boolean		needToInvalidate = false;
	
	// cast away the user data, as it's just a menuRef
	menuRef = (MenuRef) inUserData;
	
	// grab the old selection from the menu
	menuSelection = GetMenuSelection( menuRef );
		
	// make sure that we're processing a command event
	if ( GetEventClass( inEvent ) == kEventClassCommand )
	{
		switch ( GetEventKind( inEvent ) )
		{
		
			// see if this is a process event
			case kEventCommandProcess:
			
				// It is, so process the event
				err = ProcessMenuSelection( inEvent, &menuSelection,
					&needToInvalidate );
				require_noerr( err, FontMenuHandler_err );
				break;
			
			// the default case is to simply fall through and return
			// eventNotHandledErr.
			default:
				break;
			
		}
		
	}

	// if we need to invalidate, then we need to reset the font in the global
	// style and redraw all of the windows.
	if ( needToInvalidate == true )
	{	
		// reset the font in the global style
		err = SetGlobalFontFromMenuSelections( menuRef, menuSelection );
		require_noerr( err, FontMenuHandler_err );
		
		// redraw the windows
		RedrawWindows();
	}
	
FontMenuHandler_err:

	return err;

}

// ------------------------------------------------------------------------------
// FontSizeMenuHandler												[INTERNAL]
// ------------------------------------------------------------------------------

static
OSStatus FontSizeMenuHandler(
	EventHandlerCallRef	inCallRef,
	EventRef 			inEvent,
	void 				*inUserData )
{
	OSStatus	err = eventNotHandledErr;
	MenuRef		menuRef;
	SInt16		menuSelection;
	Boolean 	needToInvalidate = false;
	
	// cast away the user data, as it's just a menuRef
	menuRef = (MenuRef) inUserData;
	
	// grab the old selection from the menu
	menuSelection = GetMenuSelection( menuRef );
	
	// make sure that we're processing a command event
	if ( GetEventClass( inEvent ) == kEventClassCommand )
	{
		switch ( GetEventKind( inEvent ) )
		{
		
			// see if this is a process event
			case kEventCommandProcess:
			
				// It is, so process the event
				err = ProcessMenuSelection( inEvent, &menuSelection,
					&needToInvalidate );
				require_noerr( err, FontSizeMenuHandler_err );
				break;
			
			// the default case is to simply fall through and return
			// eventNotHandledErr.
			default:
				break;
			
		}
		
	}

	// if we need to invalidate, then we need to reset the font in the global
	// style and redraw all of the windows.
	if ( needToInvalidate == true )
	{
		// reset the font in the global style
		err = SetGlobalFontSizeFromMenuSelections( menuRef, menuSelection );
		require_noerr( err, FontSizeMenuHandler_err );
		
		// redraw the windows
		RedrawWindows();
	}
	
FontSizeMenuHandler_err:

	return err;

}


// ------------------------------------------------------------------------------
// GetMenuSelection													[INTERNAL]
// ------------------------------------------------------------------------------

static
SInt16 GetMenuSelection(
	MenuRef theMenu )
{
	UInt16			menuItemCount;
	SInt16			i;
	CharParameter	markChar;
	
	// count the number of items in the menu
	menuItemCount = CountMenuItems( theMenu );
	
	// loop through and find the checked item
	for ( i = 0; i < menuItemCount; i++ )
	{
		// if there's a mark on the menu item, then it's been checked. So,
		// get the mark
		GetItemMark( theMenu, i, &markChar );
		
		// check to see if it's not zero. If that's the case, then we've
		// found the marked item
		if ( markChar != 0 )
		{
			break;
		}
		
	}

	return i;
	
}
	
	
	