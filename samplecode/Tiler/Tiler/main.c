/*
	File:		Main.c
	
	Contains:	Source demonstrating how to set dock tiles and badges

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

	Copyright © 1998-2000 Apple Computer, Inc., All Rights Reserved
*/


#include <Carbon/Carbon.h>
#include "Tiler.h"

/*******************************************************************************
**	prototypes
*******************************************************************************/
OSStatus TilerWindowEventHandler(
		EventHandlerCallRef		inHandlerCallRef,
		EventRef				inEvent,
		void*					inUserData );
OSStatus TilerAppEventHandler(
		EventHandlerCallRef		inHandlerCallRef,
		EventRef				inEvent,
		void*					inUserData );

/*******************************************************************************
**	main
*******************************************************************************/
int main(int argc, char* argv[])
{
    IBNibRef 			theNibRef;
    WindowRef 			theWindow;
    OSStatus			theErr;
	EventHandlerUPP		theTilerWindowEventHandlerUPP;
	EventHandlerUPP		theTilerAppEventHandlerUPP;

	// These are the events that we want to handle
	EventTypeSpec	theEvents[] = { { kEventClassCommand, kEventProcessCommand } };

    // Create a Nib reference passing the name of the nib file (without the .nib extension)
    // CreateNibReference only searches into the application bundle.
    theErr = CreateNibReference(CFSTR("main"), &theNibRef);
    require_noerr( theErr, CantGetNibRef );

    // Once the nib reference is created, set the menu bar. "MainMenu" is the name of the menu bar
    // object. This name is set in InterfaceBuilder when the nib is created.
    theErr = SetMenuBarFromNib(theNibRef, CFSTR("MenuBar"));
    require_noerr( theErr, CantSetMenuBar );

    // Then create a window. "MainWindow" is the name of the window object. This name is set in
    // InterfaceBuilder when the nib is created.
    theErr = CreateWindowFromNib(theNibRef, CFSTR("MainWindow"), &theWindow);
    require_noerr( theErr, CantCreateWindow );

    // We don't need the nib reference anymore.
    DisposeNibReference( theNibRef );

	// Install a window event handler so we can see our button clicks
	theTilerWindowEventHandlerUPP = NewEventHandlerUPP( TilerWindowEventHandler );
	require( theTilerWindowEventHandlerUPP != NULL, CantCreateEventHandlerUPP );
	theErr = InstallEventHandler(
			GetWindowEventTarget( theWindow ), theTilerWindowEventHandlerUPP,
			GetEventTypeCount( theEvents ), theEvents, NULL, NULL);
	require_noerr( theErr, CantInstallEventHandler );

	// Install an app command handler so we can handle our menu commands
	theTilerAppEventHandlerUPP = NewEventHandlerUPP( TilerAppEventHandler );
	require( theTilerAppEventHandlerUPP != NULL, CantCreateEventHandlerUPP );
	theErr = InstallEventHandler(
			GetApplicationEventTarget(), theTilerAppEventHandlerUPP,
			GetEventTypeCount( theEvents ), theEvents, NULL, NULL);
    require_noerr( theErr, CantInstallEventHandler );

    // The window was created hidden so show it.
    ShowWindow( theWindow );

    // Set up the timer
    require_noerr( theErr, CantCreateTimer );

	// Call the event loop
    RunApplicationEventLoop();

	DisposeEventHandlerUPP( theTilerWindowEventHandlerUPP );
	DisposeEventHandlerUPP( theTilerAppEventHandlerUPP );

CantCreateWindow:
CantSetMenuBar:
CantGetNibRef:
CantCreateTimer:
CantCreateEventHandlerUPP:
CantInstallEventHandler:
	return theErr;
}

/*******************************************************************************
**	TilerWindowEventHandler
*******************************************************************************/
OSStatus TilerWindowEventHandler(
		EventHandlerCallRef		inHandlerCallRef,
		EventRef				inEvent,
		void*					inUserData )
{
	OSStatus		theErr;
	HICommand		theHICommand;
	static Boolean	cleanFirst = true;

	// I assume we only get called when we asked for it
	assert( GetEventClass( inEvent ) == kEventClassCommand );
	assert( GetEventKind( inEvent ) == kEventProcessCommand );

	// Extract enough info to glean the commandID
	theErr = GetEventParameter(
		inEvent,
		kEventParamDirectObject,	// event param name
		typeHICommand,   			// event param type
		NULL,						// actual EventParamType
		sizeof( HICommand ),		// buffer size
		NULL,						// actual size
		&theHICommand);				// the buffer
	require_noerr( theErr, CantGetEventParameter );

	// Handle Tiler commands
	switch ( theHICommand.commandID )
	{
		case kCommandTileAnimationQuickdraw:
			TileAnimationQuickdraw( cleanFirst );
			theErr = noErr;
			break;

		case kCommandTileAnimationQuartz:
			TileAnimationQuartz( cleanFirst );
			theErr = noErr;
			break;

		case kCommandTileBadge:
			TileBadge( cleanFirst );
			theErr = noErr;
			break;

		case kCommandRestoreTile:
			TileRestore();
			theErr = noErr;
			break;

		case kCommandToggleClean:
			// Keep track of whether the clean CheckBox is checked
			cleanFirst = !cleanFirst;
			break;

		case kHICommandQuit:
			// ***
			//
			// PITFALL!
			//
			// If you change the tile, don't quit your app without cleaning up!
			//
			// ***
			if ( cleanFirst )
				TileRestore();

			// Let the default handler continue to handle this event
			theErr = eventNotHandledErr;
			break;

		default:
			// Don't eat anyone else's commands
			theErr = eventNotHandledErr;
	}

CantGetEventParameter:
	return theErr;
}

/*******************************************************************************
**	TilerAppEventHandler
*******************************************************************************/
OSStatus TilerAppEventHandler(
		EventHandlerCallRef		inHandlerCallRef,
		EventRef				inEvent,
		void*					inUserData )
{
	OSStatus		theErr;
	HICommand		theHICommand;

	// Extract enough info to glean the commandID
	theErr = GetEventParameter(
		inEvent,
		kEventParamDirectObject,	// event param name
		typeHICommand,   			// event param type
		NULL,						// actual EventParamType
		sizeof( HICommand ),		// buffer size
		NULL,						// actual size
		&theHICommand);				// the buffer
	require_noerr( theErr, CantGetEventParameter );

	// Handle Tiler commands
	switch ( theHICommand.commandID )
	{
		case kCommandMenuConvertGuy:
			NewGuyWindow();
			theErr = noErr;
			break;

		default:
			// Don't eat anyone else's commands
			theErr = eventNotHandledErr;
	}

CantGetEventParameter:
	return theErr;
}
