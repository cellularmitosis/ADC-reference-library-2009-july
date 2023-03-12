/*
    File:		main.c
    
    Version:	1.0

	Disclaimer:	IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc.
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

	Copyright � 2002 Apple Computer, Inc., All Rights Reserved
*/

#include <Carbon/Carbon.h>

#include "CalendarView.h"

// -----------------------------------------------------------------------------
//	constants
// -----------------------------------------------------------------------------
//
enum
{
	kCalendar	= 'CalC'
};

const ControlID	kCalendarID = { kCalendar, 0 };

// -----------------------------------------------------------------------------
//	types
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//	prototypes
// -----------------------------------------------------------------------------
//
pascal OSStatus MyAppEventHandler(
	EventHandlerCallRef	inHandlerCallRef,
	EventRef			inEvent,
	void*				inUserData);
pascal OSStatus MyWindowEventHandler(
	EventHandlerCallRef	inHandlerCallRef,
	EventRef			inEvent,
	void*				inUserData);
OSStatus MyCreateWindow();

DEFINE_ONE_SHOT_HANDLER_GETTER( MyAppEventHandler );
DEFINE_ONE_SHOT_HANDLER_GETTER( MyWindowEventHandler );

// -----------------------------------------------------------------------------
//	globals
// -----------------------------------------------------------------------------

//	globals suck too, but sometimes you need them

// -----------------------------------------------------------------------------
//	main
// -----------------------------------------------------------------------------
//
int main(
	int				argc,
	char*			argv[] )
{
#pragma unused( argc, argv )
    IBNibRef 		nibRef;
    EventTypeSpec	appEventList[] = { { kEventClassCommand, kEventProcessCommand } };
    OSStatus		err;

    // Create a Nib reference passing the name of the nib file (without the .nib extension)
    // CreateNibReference only searches into the application bundle.
    err = CreateNibReference( CFSTR( "main" ), &nibRef );
    require_noerr( err, CantGetNibRef );
    
    // Once the nib reference is created, set the menu bar. "MainMenu" is the name of the menu bar
    // object. This name is set in InterfaceBuilder when the nib is created.
    err = SetMenuBarFromNib( nibRef, CFSTR( "MenuBar" ) );
    require_noerr( err, CantSetMenuBar );
    
    // We don't need the nib reference anymore.
    DisposeNibReference( nibRef );
    
    // Install the applcation event handler
	err = InstallApplicationEventHandler( GetMyAppEventHandlerUPP(),
			GetEventTypeCount( appEventList ), appEventList, NULL, NULL );
	require_noerr( err, CantInstallAppEventHandler );
	
	err = MyCreateWindow();
	require_noerr( err, CantMyCreateWindow );
	
	// Call the event loop
    RunApplicationEventLoop();

CantMyCreateWindow:
CantInstallAppEventHandler:
CantSetMenuBar:
CantGetNibRef:
	return err;
}

// -----------------------------------------------------------------------------
//	MyCreateWindow
// -----------------------------------------------------------------------------
//
OSStatus MyCreateWindow()
{
    IBNibRef 		nibRef;
    WindowRef 		window;
    EventTypeSpec	windowEventList[] = {
							{ kEventClassCommand, kEventProcessCommand },
							{ kEventClassWindow, kEventWindowBoundsChanged } };
    OSStatus		err;
	ControlRef		control;
	Rect			bounds = { 10, 10, 10+320, 10+420 };

    // Create a Nib reference passing the name of the nib file (without the .nib extension)
    // CreateNibReference only searches into the application bundle.
    err = CreateNibReference( CFSTR( "main" ), &nibRef );
    require_noerr( err, CantGetNibRef );
    
    // Then create a window. "MainWindow" is the name of the window object. This name is set in 
    // InterfaceBuilder when the nib is created.
    err = CreateWindowFromNib( nibRef, CFSTR( "MainWindow" ), &window );
    require_noerr( err, CantCreateWindow );

    // We don't need the nib reference anymore.
    DisposeNibReference( nibRef );
    
    // Install the window event handler
	err = InstallWindowEventHandler( window, GetMyWindowEventHandlerUPP(),
			GetEventTypeCount( windowEventList ), windowEventList, window, NULL );
	require_noerr( err, CantInstallWindowEventHandler );
	
	err = CalendarViewCreate( window, &bounds, &control );
	if ( err == noErr )
	{
		float			ratio;

		SetControlID( control, &kCalendarID );
		ratio = 0;
		err = SetControlData( control, 0, kControlCalendarDayNameRatioTag, sizeof( float ), &ratio );
		ratio = (float) 1;
		err = SetControlData( control, 0, kControlCalendarTitleRatioTag, sizeof( float ), &ratio );
		ShowControl( control );
	}
	
	// The window was created hidden so show it.
    ShowWindow( window );
    
CantInstallWindowEventHandler:
CantCreateWindow:
CantGetNibRef:
	return err;
}

// -----------------------------------------------------------------------------
//	MyAppEventHandler
// -----------------------------------------------------------------------------
//
pascal OSStatus MyAppEventHandler(
	EventHandlerCallRef	inHandlerCallRef,
	EventRef			inEvent,
	void*				inUserData )
{
#pragma unused( inHandlerCallRef, inUserData )
	HICommand	hiCommand;
	OSStatus	err = eventNotHandledErr;
	
	// UInt32		eventClass = GetEventClass( inEvent );
	// UInt32		eventKind = GetEventKind( inEvent );

	// if ( eventClass == kEventClassCommand && eventKind == kEventProcessCommand )
	//{
		err = GetEventParameter( inEvent, kEventParamDirectObject, typeHICommand,
			NULL, sizeof( HICommand ), NULL, &hiCommand );
		require_noerr( err, CantGetEventParameter );
		
		switch ( hiCommand.commandID )
		{
			case kHICommandNew:
				err = MyCreateWindow();
				break;
	
			default:
				err = eventNotHandledErr;
				break;
		}
	//}
	
CantGetEventParameter:
	return err;
}

// -----------------------------------------------------------------------------
//	MyWindowEventHandler
// -----------------------------------------------------------------------------
//
pascal OSStatus MyWindowEventHandler(
	EventHandlerCallRef	inHandlerCallRef,
	EventRef			inEvent,
	void*				inUserData )
{
#pragma unused( inHandlerCallRef )
	HICommand			hiCommand;
	OSStatus			err;
	UInt32				eventClass = GetEventClass( inEvent );
	UInt32				eventKind = GetEventKind( inEvent );
	WindowRef			window = (WindowRef) inUserData;
	ControlRef			control;
	CFGregorianDate		date;

	if ( eventClass == kEventClassCommand && eventKind == kEventProcessCommand )
	{
		err = GetEventParameter( inEvent, kEventParamDirectObject, typeHICommand,
			NULL, sizeof( HICommand ), NULL, &hiCommand );
		require_noerr( err, CantGetEventParameter );
		
		switch ( hiCommand.commandID )
		{
			case 'oc02':
				err = GetControlByID( window, &kCalendarID, &control );
				require_noerr( err, CantGetControlByID );
				
				date.year = 2002;
				date.month = 10;

				err = SetControlData( control, 0, kControlCalendarDateTag, sizeof( CFGregorianDate ), &date );
				require_noerr( err, CantSetControlData );
				break;

			case 'ja04':
				err = GetControlByID( window, &kCalendarID, &control );
				require_noerr( err, CantGetControlByID );
				
				date.year = 2004;
				date.month = 1;

				err = SetControlData( control, 0, kControlCalendarDateTag, sizeof( CFGregorianDate ), &date );
				require_noerr( err, CantSetControlData );
				break;
			
			default:
				err = eventNotHandledErr;
				break;
		}
	}
	else if ( eventClass == kEventClassWindow && eventKind == kEventWindowBoundsChanged )
	{
		UInt32		attributes;
		Rect		origBounds;
		Rect		currBounds;
		HIRect		bounds;
		ControlRef	control;
		
		err = GetEventParameter( inEvent, kEventParamAttributes, typeUInt32,
			NULL, sizeof( UInt32 ), NULL, &attributes );
		require_noerr( err, CantGetEventParameter );
		
		if ( attributes & kWindowBoundsChangeSizeChanged )
		{
			// Resize the user control accordingly
			err = GetEventParameter( inEvent, kEventParamOriginalBounds, typeQDRectangle,
				NULL, sizeof( Rect ), NULL, &origBounds );
			require_noerr( err, CantGetEventParameter );
			
			err = GetEventParameter( inEvent, kEventParamCurrentBounds, typeQDRectangle,
				NULL, sizeof( Rect ), NULL, &currBounds );
			require_noerr( err, CantGetEventParameter );

			err = GetControlByID( window, &kCalendarID, &control );
			require_noerr( err, CantGetControlByID );
			
			HIViewSetVisible( control, false );
			HIViewGetFrame( control, &bounds );
			bounds.size.width += currBounds.right - origBounds.right;
			bounds.size.height += currBounds.bottom - origBounds.bottom;
			HIViewSetFrame( control, &bounds );
			HIViewSetVisible( control, true );
		}
	}
	
CantSetControlData:
CantGetControlByID:
CantGetEventParameter:
	return err;
}