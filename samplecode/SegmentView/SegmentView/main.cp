/*
    File:		main.cp
    
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

	Copyright © 2002 Apple Computer, Inc., All Rights Reserved
*/

#include <Carbon/Carbon.h>

#include "TSegmentView.h"

// -----------------------------------------------------------------------------
//	constants
// -----------------------------------------------------------------------------
//
enum
{
	kSegementView	= 'segm'
};

const ControlID	kSegmentViewID = { kSegementView, 0 };

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
    EventTypeSpec	windowEventList[] = { { kEventClassCommand, kEventProcessCommand } };
    OSStatus		err;
	ControlRef		control;
	HIRect			hiBounds = { { 40, 40 }, { 200, 40 } };

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
	
	err = TSegmentView::Create( &control, &hiBounds, window );
	if ( err == noErr )
	{
		ShowControl( control );

		// Give it an ID, so we can find it later
		SetControlID( control, &kSegmentViewID );

		CFStringRef			iconNames[] = {
								CFSTR( "IconView.png" ),
								CFSTR( "ListView.png" ),
								CFSTR( "ColumnView.png" ),
								CFSTR( "AlbumView.png" ),
								CFSTR( "DetailView.png" ) };

		CFURLRef			url;
		CGDataProviderRef	provider;
		int					i;
		CGImageRef			icons[ 5 ];
		SegmentIconData		iconData = { 5, icons };

		// Load up the art work
		for ( i = 0; i < 5; i++ )
		{
			url = CFBundleCopyResourceURL( CFBundleGetMainBundle(), iconNames[ i ], NULL, NULL );
			require_action( url != NULL, CantGetURL, err = paramErr );
			
			provider = CGDataProviderCreateWithURL( url );
			
			icons[ i ] = CGImageCreateWithPNGDataProvider( provider, NULL, false, kCGRenderingIntentDefault );
			require_action( icons[ i ] != NULL, CantMakeImage, err = memFullErr );
			
			CGDataProviderRelease( provider );
		
			CFRelease( url );
		}
		
		err = SetControlData( control, 0, kControlSegmentViewIconsTag, sizeof( iconData ), &iconData );
	}

	// The window was created hidden so show it.
    ShowWindow( window );
    
CantMakeImage:
CantGetURL:
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
	HICommand			hiCommand;
	OSStatus			err = eventNotHandledErr;
	
	// UInt32			eventClass = GetEventClass( inEvent );
	// UInt32			eventKind = GetEventKind( inEvent );

	// We are only registered for the kEventClassCommand/kEventProcessCommand pair

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
	// UInt32			eventClass = GetEventClass( inEvent );
	// UInt32			eventKind = GetEventKind( inEvent );
	WindowRef			window = (WindowRef) inUserData;
	SInt32				max;
	ControlRef			control;
	
	// We are only registered for the kEventClassCommand/kEventProcessCommand pair

	// if ( eventClass == kEventClassCommand && eventKind == kEventProcessCommand )
	//{
		err = GetEventParameter( inEvent, kEventParamDirectObject, typeHICommand,
			NULL, sizeof( HICommand ), NULL, &hiCommand );
		require_noerr( err, CantGetEventParameter );
		
		switch ( hiCommand.commandID )
		{
			case 'segm':
				// Get the segement view
				err = GetControlByID( window, &kSegmentViewID, &control );
				require_noerr( err, CantGetControlByID );
				
				// Get the maximum
				max = GetControlMaximum( control );

				// Change it
				SetControlMaximum( control, max == 3 ? 5 : 3 );
				
				HIViewSetNeedsDisplay( control, true );
				break;

			default:
				err = eventNotHandledErr;
				break;
		}
	//}
	
CantGetControlByID:
CantGetEventParameter:
	return err;
}
