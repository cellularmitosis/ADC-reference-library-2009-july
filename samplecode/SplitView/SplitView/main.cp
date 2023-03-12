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
	Created by dam on Thu Jul 25 2002
*/

#include <Carbon/Carbon.h>

#include "SplitView.h"
#include "TClockView.h"

// -----------------------------------------------------------------------------
//	constants
// -----------------------------------------------------------------------------
//
enum
{
	kViewPane	= 'splV',
	kClockAPane	= 'clkA',
	kClockBPane	= 'clkB'
};

const ControlID	kViewPaneID = { kViewPane, 0 };
const ControlID	kClockBPaneID = { kClockBPane, 0 };

// -----------------------------------------------------------------------------
//	macros
// -----------------------------------------------------------------------------
//
#define QDRECTWIDTH(R)	((R).right - (R).left)
#define QDRECTHEIGHT(R)	((R).bottom - (R).top)

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
    EventTypeSpec	windowEventList[] = {
							{ kEventClassCommand, kEventProcessCommand },
							{ kEventClassWindow, kEventWindowBoundsChanged } };
    OSStatus		err;
	ControlRef		splitViewA;
	ControlRef		splitViewB;
	HIRect			hiBounds;
	Rect			bounds;
	ControlRef		clock;
	float			ratio = 0.5;

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
	
	// Base the clock control size on the start window size
	GetWindowBounds( window, kWindowContentRgn, &bounds );
	hiBounds = CGRectMake( 0, 0, QDRECTWIDTH( bounds ), QDRECTHEIGHT( bounds ) );

	// Make the main split view
	err = SplitView::Create( &splitViewA, &hiBounds, window );
	if ( err == noErr )
	{
		// Set the split view to be split in half
		err = SetControlData( splitViewA, 0, kControlSplitViewSplitRatioTag, sizeof( float ), &ratio );
		require_noerr( err, CantSetControlData );

		ShowControl( splitViewA );
		
		// Give it an ID, so we can find it later
		SetControlID( splitViewA, &kViewPaneID );
	}
	
	// Make the split view that's going into the top of the main split view
	err = SplitView::Create( &splitViewB, &hiBounds, window );
	if ( err == noErr )
	{
		Boolean		isVertical = true;
		
		// Set the split view to be split in half
		err = SetControlData( splitViewB, 0, kControlSplitViewSplitRatioTag, sizeof( float ), &ratio );
		require_noerr( err, CantSetControlData );

		// Make it a vertical split
		err = SetControlData( splitViewB, 0, kControlSplitViewOrientationTag, sizeof( Boolean ), &isVertical );
		require_noerr( err, CantSetControlData );

		ShowControl( splitViewB );
		
		// Set this as the A view of the main split view
		err = SetControlData( splitViewA, 0, kControlSplitViewSubViewA, sizeof( HIViewRef ), &splitViewB ); 
		require_noerr( err, CantSetControlData );
	}
	
	// Make a new clock control
	err = TClockView::Create( &clock, &hiBounds, window );
	if ( err == noErr )
	{
		ShowControl( clock );
		
		// Set this as the A view of the secondary split view
		err = SetControlData( splitViewB, 0, kControlSplitViewSubViewA, sizeof( HIViewRef ), &clock ); 
		require_noerr( err, CantSetControlData );
	}
	
	// Make another new clock control
	err = TClockView::Create( &clock, &hiBounds, window );
	if ( err == noErr )
	{
		ShowControl( clock );
		
		// Set this as the B view of the secondary split view
		err = SetControlData( splitViewB, 0, kControlSplitViewSubViewB, sizeof( HIViewRef ), &clock ); 
		require_noerr( err, CantSetControlData );
	}
	
	// Make a third new clock control
	err = TClockView::Create( &clock, &hiBounds, window );
	if ( err == noErr )
	{
		ShowControl( clock );
		
		// Set this as the B view of the main split view
		err = SetControlData( splitViewA, 0, kControlSplitViewSubViewB, sizeof( HIViewRef ), &clock ); 
		require_noerr( err, CantSetControlData );
	}
	
	// The window was created hidden so show it.
    ShowWindow( window );
    
CantSetControlData:
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
	UInt32				eventClass = GetEventClass( inEvent );
	UInt32				eventKind = GetEventKind( inEvent );
	WindowRef			window = (WindowRef) inUserData;
	Boolean				isVertical;
	ControlRef			control;
	
	if ( eventClass == kEventClassCommand && eventKind == kEventProcessCommand )
	{
		err = GetEventParameter( inEvent, kEventParamDirectObject, typeHICommand,
			NULL, sizeof( HICommand ), NULL, &hiCommand );
		require_noerr( err, CantGetEventParameter );
		
		switch ( hiCommand.commandID )
		{
			case 'revo':
				// Get the split view
				err = GetControlByID( window, &kViewPaneID, &control );
				require_noerr( err, CantGetControlByID );
				
				// Get orientation
				err = GetControlData( control, 0, kControlSplitViewOrientationTag, sizeof( Boolean ), &isVertical, NULL );
				require_noerr( err, CantGetControlData );
				
				// Reverse it
				isVertical = !isVertical;
				
				// Set orientation
				err = SetControlData( control, 0, kControlSplitViewOrientationTag, sizeof( Boolean ), &isVertical );
				require_noerr( err, CantSetControlData );
				
				HIViewSetNeedsDisplay( control, true );
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
		
		// Get the change attributes
		err = GetEventParameter( inEvent, kEventParamAttributes, typeUInt32,
			NULL, sizeof( UInt32 ), NULL, &attributes );
		require_noerr( err, CantGetEventParameter );
		
		// If the window size changed
		if ( attributes & kWindowBoundsChangeSizeChanged )
		{
			// Resize the clock control accordingly
			err = GetEventParameter( inEvent, kEventParamOriginalBounds, typeQDRectangle,
				NULL, sizeof( Rect ), NULL, &origBounds );
			require_noerr( err, CantGetEventParameter );
			
			err = GetEventParameter( inEvent, kEventParamCurrentBounds, typeQDRectangle,
				NULL, sizeof( Rect ), NULL, &currBounds );
			require_noerr( err, CantGetEventParameter );

			// Get the splitview
			err = GetControlByID( window, &kViewPaneID, &control );
			require_noerr( err, CantGetControlByID );
			
			// Hide it (to avoid remnants when downsizing)
			HIViewSetVisible( control, false );
			
			// Resize it
			HIViewGetFrame( control, &bounds );
			bounds.size.width += QDRECTWIDTH( currBounds ) - QDRECTWIDTH( origBounds );
			bounds.size.height += QDRECTHEIGHT( currBounds ) - QDRECTHEIGHT( origBounds );
			HIViewSetFrame( control, &bounds );
			
			// Show it
			HIViewSetVisible( control, true );
		}
	}
	
CantGetControlData:
CantSetControlData:
CantGetControlByID:
CantGetEventParameter:
	return err;
}
