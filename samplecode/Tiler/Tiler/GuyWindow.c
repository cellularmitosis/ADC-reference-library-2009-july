/*
	File:		GuyWindow.c
	
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
**	typedefs
*******************************************************************************/
typedef struct
{
	SInt16		currentFrame;
	SInt16		finalFrame;
	SInt16		increment;
	WindowRef	window;
	Boolean		continuous;
} GuyConversionData;

/*******************************************************************************
**	prototypes
*******************************************************************************/
OSStatus GuyWindowEventHandler(
		EventHandlerCallRef		inHandlerCallRef,
		EventRef				inEvent,
		void*					inUserData );
void DrawGuyWindowContent( WindowRef inWindow );
OSStatus ConvertGuy( WindowRef	inWindow);
void ConvertGuyProc(
		EventLoopTimerRef		inTimer,
		void*					inUserData );
void StopConvertingGuy(
		EventLoopTimerRef		inTimer,
		GuyConversionData*		inData );
void DoTheGuyProcessing(
		EventLoopTimerRef		inTimer,
		GuyConversionData*		inData );
void DoThePostGuyProcessing(
		EventLoopTimerRef		inTimer,
		GuyConversionData*		inData );

/*******************************************************************************
**	NewGuyWindow
*******************************************************************************/
void NewGuyWindow()
{
	IBNibRef 			theNibRef;
	WindowRef 			theWindow;
	OSStatus			theErr;
	EventHandlerUPP		theWindowEventHandlerUPP;

	// These are the events that we want to handle
	EventTypeSpec		theEvents[] = {
							{ kEventClassCommand, kEventProcessCommand },
							{ kEventClassWindow, kEventWindowDrawContent } };

    // Create a Nib reference passing the name of the nib file (without the .nib
	// extension). CreateNibReference only searches into the application bundle.
    theErr = CreateNibReference(CFSTR("main"), &theNibRef);
    require_noerr( theErr, CantGetNibRef );

    // Then create a window. "MainWindow" is the name of the window object. This
	// name is set in InterfaceBuilder when the nib is created.
    theErr = CreateWindowFromNib(theNibRef, CFSTR("GuyWindow"), &theWindow);
    require_noerr( theErr, CantCreateWindow );

    // We don't need the nib reference anymore.
    DisposeNibReference( theNibRef );

	// Install the command handler so we can see our button clicks
	theWindowEventHandlerUPP = NewEventHandlerUPP( GuyWindowEventHandler );
	require( theWindowEventHandlerUPP != NULL, CantCreateEventHandlerUPP );
	theErr = InstallEventHandler( GetWindowEventTarget( theWindow ),
			theWindowEventHandlerUPP, GetEventTypeCount( theEvents ),
			theEvents, theWindow, NULL);
    require_noerr( theErr, CantInstallEventHandler );

    // Tell the window which picture to draw
	SetWRefCon( theWindow, kDarthGuy );

	// Cascade
	RepositionWindow( theWindow, NULL, kWindowCascadeOnMainScreen );

	// The window was created hidden so show it.
    ShowWindow( theWindow );

CantGetNibRef:
CantCreateWindow:
CantCreateEventHandlerUPP:
CantInstallEventHandler:
	;
}

/*******************************************************************************
**	GuyWindowEventHandler
*******************************************************************************/
OSStatus GuyWindowEventHandler(
	EventHandlerCallRef		inHandlerCallRef,
	EventRef				inEvent,
	void*					inUserData )
{
	OSStatus		theErr;
	HICommand		theHICommand;

	// Handle HICommands
	if ( GetEventClass( inEvent ) == kEventClassCommand &&
		 GetEventKind( inEvent ) == kEventProcessCommand )
	{
		// Extract enough info to glean the commandID
		theErr = GetEventParameter( inEvent, kEventParamDirectObject,
			typeHICommand, NULL, sizeof( HICommand ), NULL,
			&theHICommand);
		require_noerr( theErr, CantGetEventParameter );

		// Handle Tiler commands
		switch ( theHICommand.commandID )
		{
			case kCommandConvertGuy:
				ConvertGuy( ( WindowRef ) inUserData );
				theErr = noErr;
				break;

			default:
				// Let the default handler handle everything else
				theErr = eventNotHandledErr;
		}
	}
	else if ( GetEventClass( inEvent ) == kEventClassWindow &&
			  GetEventKind( inEvent) == kEventWindowDrawContent )
	{
		WindowRef	theWindow;

		// Extract enough info to glean the commandID
		theErr = GetEventParameter(
			inEvent, kEventParamDirectObject, typeWindowRef,
			NULL, sizeof( WindowRef ), NULL, &theWindow);
		require_noerr( theErr, CantGetEventParameter );

		DrawGuyWindowContent( theWindow );
	}
	else
	{
		// Let the default handler handle everything else
		theErr = eventNotHandledErr;
	}

CantGetEventParameter:
	return theErr;
}

/*******************************************************************************
**	DrawGuyWindowContent
*******************************************************************************/
void DrawGuyWindowContent( WindowRef inWindow )
{
	PicHandle	thePicture;
	Rect		theDestRect = { 13, 36, 141, 164 };

	thePicture = GetPicture( GetWRefCon( inWindow ) );
	if ( thePicture == NULL )
		return;

	DrawPicture( thePicture, &theDestRect );

	ReleaseResource( (Handle) thePicture );
}

/*******************************************************************************
**	ConvertGuy
*******************************************************************************/
OSStatus ConvertGuy( WindowRef inWindow )
{
    OSStatus				theErr;
	GuyConversionData*		theData;
	ControlID				theConvertButtonID = { kCommandConvertGuy, 0 };
	ControlRef				theControl;

	// Make the data
	theData = ( GuyConversionData* ) malloc( sizeof( GuyConversionData ) );
	if ( theData == NULL )
		return memFullErr;

	// Set up the data
	theData->currentFrame = kDarthGuy;
	theData->finalFrame = kGuyFull;
	theData->increment = kConvertForwards;
	theData->window = inWindow;
	theData->continuous = false;

	// While "processing", you can't click the button.  Disable it.
	theErr = GetControlByID( inWindow, &theConvertButtonID, &theControl );
	if ( theErr == noErr )
		DisableControl( theControl );

	// Install a timer to simulate the processing of the image
	theErr = InstallEventLoopTimer(
		GetCurrentEventLoop(), kTimerFrequency, kTimerFrequency, ConvertGuyProc,
		theData, NULL );

	return theErr;
}

/*******************************************************************************
**	StopConvertingGuy
*******************************************************************************/
void StopConvertingGuy(
	EventLoopTimerRef		inTimer,
	GuyConversionData*		inData )
{
	ControlID				theConvertButtonID = { kCommandConvertGuy, 0 };
	OSStatus				theErr;
	ControlRef				theControl;

	// Enable the convert control
	theErr = GetControlByID( inData->window, &theConvertButtonID, &theControl );
	if ( theErr == noErr )
		EnableControl( theControl );

	// Dispose of the timer
	RemoveEventLoopTimer( inTimer );

	// Update the tile to assure that it matches the final state
	UpdateCollapsedWindowDockTile( inData->window );

	// We're done with the user data, too
	free( inData );
}

/*******************************************************************************
**	DoTheGuyProcessing
*******************************************************************************/
void DoTheGuyProcessing(
	EventLoopTimerRef		inTimer,
	GuyConversionData*		inData )
{
	Rect					theDestRect = { 13, 36, 141, 164 };
	GrafPtr					theSavedPort;

	// If the animation has been cancelled
	if ( GetWRefCon( inData->window ) == 0 )
	{
		StopConvertingGuy( inTimer, inData );
		return;
	}

	// Next image
	inData->currentFrame += inData->increment;

	GetPort( &theSavedPort );
	SetPort( GetWindowPort( inData->window ) );

	// Let the window know which image it needs to draw
	SetWRefCon( inData->window, inData->currentFrame );
	
	// Mark for redrawing
	InvalWindowRect( inData->window, &theDestRect );

	SetPort( theSavedPort );
}

/*******************************************************************************
**	DoThePostGuyProcessing
*******************************************************************************/
void DoThePostGuyProcessing(
	EventLoopTimerRef		inTimer,
	GuyConversionData*		inData )
{
	// If this is the last one
	if ( inData->currentFrame == inData->finalFrame )
	{
		if ( inData->continuous == true )
		{
			// Reverse the animation
			inData->increment = - inData->increment;
			if ( inData->finalFrame == kDarthGuy )
				inData->finalFrame = kGuyFull;
			else
				inData->finalFrame = kDarthGuy;
		}
		else
		{
			// All done!
			StopConvertingGuy( inTimer, inData );
		}
	}
}

/*******************************************************************************
**	ConvertGuyProc
*******************************************************************************/
void ConvertGuyProc(
	EventLoopTimerRef		inTimer,
	void*					inUserData )
{
	GuyConversionData*		theData = ( GuyConversionData* ) inUserData;

	DoTheGuyProcessing( inTimer, theData );

	// ***
	//
	// Tell the collapsed window tile to look like the window
	//
	// ***
	UpdateCollapsedWindowDockTile( theData->window );

	DoThePostGuyProcessing( inTimer, theData );
}
