/*
	File:		Magnify.cp

	Contains:	Sample application which implements a magnifying lens tool.

	Version:	Mac OS X

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

	Copyright © 1998-2001 Apple Computer, Inc., All Rights Reserved
*/

#ifdef __APPLE_CC__
#include <Carbon/Carbon.h>
#else
#include <Carbon.h>
#endif

enum
{
	kCmdMagnificationChanged	= 'MChg',
	kCmdShowMouseLocation		= 'MLoc',
	kCmdFloat					= 'Floa'
};

struct PrefInfo
{
	UInt8			origMagFactor;
	Boolean			origShowInfo;
	Boolean			origFloats;
	WindowRef		window;
};
typedef struct PrefInfo PrefInfo;

static pascal OSStatus			WindowHandler( EventHandlerCallRef inHandler, EventRef inEvent, void* userData );
static pascal OSStatus			AppCommandHandler( EventHandlerCallRef inHandler, EventRef inEvent, void* userData );
static pascal OSStatus			PrefHandler( EventHandlerCallRef inHandler, EventRef inEvent, void* userData );

pascal void				MagnifyEventLoopTimer(EventLoopTimerRef /*inTimer*/, void *inUserData);
static pascal OSErr		QuitAppleEventHandler(const AppleEvent *appleEvt, AppleEvent* reply, long refcon);

static void				DoPreferences();
static void				AdjustWindowFloating();

static GrafPtr			sGlobalPort = NULL;
static UInt8			sMagFactor = 8;
static Boolean			sShowInfo = true;
static Boolean			sFloats = false;
static Boolean			sForceUpdate = false;
static WindowRef		sMagnifyWindow;

static const ControlID	kMagSlider 		= { 'PREF', 1 };
static const ControlID	kShowInfoCheck 	= { 'PREF', 2 };
static const ControlID	kFloatCheck 	= { 'PREF', 3 };

#define kPreferencesKey	CFSTR( "com.apple.carbon.examples.magnify" )

//----------------------------------------------------------------------------
//	• MagnifyEventLoopTimer
//
//	Our magnification timer. Ideally we should be event based, but we can't
//	detect every event that might cause a screen update (window move, etc.)
//	so we resort to this.
//----------------------------------------------------------------------------

pascal void
MagnifyEventLoopTimer(EventLoopTimerRef /*inTimer*/, void *inUserData)
{
	WindowRef	window = (WindowRef)inUserData;
	Point		mouse;
	GrafPtr		windowPort;
	GrafPtr		savePort;
	Rect		sourceRect;
	Rect		destRect;
	Rect		portBounds;
	SInt16		distance;
    static Point	sLastMouse = { -1, -1};
    
	GetPort( &savePort );
	windowPort = GetWindowPort(window);

	GetPortBounds( windowPort, &portBounds );

	GetGlobalMouse( &mouse );

    if ( !EqualPt( mouse, sLastMouse ) || sForceUpdate )
    {
        Rect	monitorBounds;
		SInt32	temp;
        
        sLastMouse = mouse;
        SetPort(windowPort);

        distance = SInt16((portBounds.right - portBounds.left) / (sMagFactor * 2));

        // Pin the mouse position to the main monitor
        monitorBounds = (**GetMainDevice()).gdRect;

        InsetRect( &monitorBounds, distance, distance );

        temp = PinRect( &monitorBounds, mouse );
        mouse = *(Point*)&temp;
        
        sourceRect.left = mouse.h - distance;
        sourceRect.right = mouse.h + distance;
        sourceRect.top = mouse.v - distance;
        sourceRect.bottom = mouse.v + distance;

        destRect = portBounds;

        ForeColor(blackColor);
        BackColor(whiteColor);
		PenMode( srcCopy );

        CopyBits( *(BitMap **)GetPortPixMap(sGlobalPort), *(BitMap **)GetPortPixMap(windowPort), &sourceRect, &destRect, srcCopy, NULL );

		if ( sShowInfo )
		{
			CFStringRef		string;
			
			string = CFStringCreateWithFormat( NULL, NULL, CFSTR( "(%d,%d), %dx" ), sLastMouse.h, sLastMouse.v, sMagFactor );

			if ( string )
			{
				Rect			box;
				Point			size;
				SInt16			baseLine;
				const RGBColor	kWhite = { 0xFFFF, 0xFFFF, 0xFFFF };
				const RGBColor	kBlack = { 0, 0, 0 };

				TextFont( 0 );
				TextSize( 12 );
								
				GetThemeTextDimensions( string, kThemeCurrentPortFont, kThemeStateActive,
						false, &size, &baseLine );
				
				box.left = portBounds.left;
				box.bottom = portBounds.bottom;
				box.right = box.left + size.h;
				box.top = box.bottom - size.v;
				
				RGBForeColor( &kBlack );
				PaintRect( &box );
				
				RGBForeColor( &kWhite );
				DrawThemeTextBox( string, kThemeCurrentPortFont, kThemeStateActive,
						false, &box, teCenter, NULL );
				
				CFRelease( string );
			}
		}

			
		ForeColor( blackColor );
		BackColor( whiteColor );
		PenMode( patXor );

		MoveTo( ( portBounds.right / 2 ) - 4, portBounds.bottom / 2 );
		Line( 8, 0 );
		MoveTo( portBounds.right / 2, ( portBounds.bottom / 2 ) - 4 );
		Line( 0, 8 );
		
        SetPort( savePort );
    }
    
    sForceUpdate = false;
}

//----------------------------------------------------------------------------
//	• CreateMagnifyWindow
//
//	Create our magnification window.
//----------------------------------------------------------------------------

static OSStatus
CreateMagnifyWindow( IBNibRef nibRef )
{
	WindowRef			window;
	EventLoopTimerUPP	eventLoopTimer;
	EventHandlerRef		eventHandlerRef;
	EventHandlerUPP		handlerProc;
	EventTypeSpec		events[] = { { kEventClassWindow, kEventWindowBoundsChanging } };
	OSStatus			err;

	err = CreateWindowFromNib( nibRef, CFSTR( "MainWindow" ), &window );
	require_noerr( err, CantCreateWindow );

	// Now install our own handler so we can intercept window size changes
	// For this handler, plus our timer below, we create a UPP, but we never
	// dispose of it. This is because our window stays around until we quit.
	// If we had a window that came and went, we would need to clean up
	// properly and Dispose the EventHandlerUPP and the EventLoopTimerUPP for
	// the window.

	handlerProc = NewEventHandlerUPP( WindowHandler );
	verify_noerr( InstallWindowEventHandler( window, handlerProc, GetEventTypeCount( events ),
				events, window, &eventHandlerRef ) );

	// Install our timer, which we use to update the window.
	
    eventLoopTimer = NewEventLoopTimerUPP(MagnifyEventLoopTimer);
	
	verify_noerr( InstallEventLoopTimer( GetCurrentEventLoop(), kEventDurationSecond,
				kEventDurationSecond / 20, eventLoopTimer, (void *)window, NULL ) );
	
	// Make this window such that it does not hide when we are suspended.
	// Normally, floating windows have the hide-on-suspend attribute set
	// so that they hide when switching out and show when resumed later.
	
	ChangeWindowAttributes( window, 0, kWindowHideOnSuspendAttribute );

	sMagnifyWindow = window;
	
	// Adjust our float-ness as per our current settings
	
	AdjustWindowFloating();
	
	// And it's always nice to see the windows we create.
	
	ShowWindow(window);
	
CantCreateWindow:
	return err;
}

//----------------------------------------------------------------------------
//	• WindowHandler
//
//	Constrain our resizing to a perfect square.
//----------------------------------------------------------------------------

static pascal OSStatus
WindowHandler( EventHandlerCallRef inHandler, EventRef inEvent, void* userData )
{
	Rect		bounds;
	SInt16		height, width;
	UInt32		attributes;
	OSStatus	result = eventNotHandledErr;
	
	GetEventParameter( inEvent, kEventParamAttributes, typeUInt32, NULL, sizeof( UInt32 ), NULL, &attributes );
	
	// We only constrain on size changes. There is no point constraining if
	// the position (origin) is changing.
	
	if ( (attributes & kWindowBoundsChangeSizeChanged) != 0 )
	{
		// Extract the current bounds. This is the paramter you get to modify to 
		// alter the window position or size during a window resizing.
		GetEventParameter( inEvent, kEventParamCurrentBounds, typeQDRectangle, NULL, sizeof( bounds ), NULL, &bounds );
	
		// Make the bounds square
		height = bounds.bottom - bounds.top;
		width = bounds.right - bounds.left;
	
		if ( width > height )
		{
			bounds.bottom = bounds.top + width;
		}
		else
		{
			bounds.right = bounds.left + height;
		}
	
		// Set the current bounds parameter to our adjusted bounds. Return
		// noErr to indicate we handled this event.
		SetEventParameter( inEvent, kEventParamCurrentBounds, typeQDRectangle, sizeof( bounds ), &bounds );
		result = noErr;
	}
	return result;
}

static pascal OSErr QuitAppleEventHandler(const AppleEvent *appleEvt, AppleEvent* reply, long refcon)
{
#pragma unused (appleEvt, reply, refcon)
	QuitApplicationEventLoop();
	return(noErr);
}

//----------------------------------------------------------------------------
//	• main
//
//	Where it all begins...
//----------------------------------------------------------------------------

int main( int argc, char* argv[] )
{
	long 				response;
	IBNibRef			nibRef;
	Boolean				test;
	OSStatus			err			= noErr;
	EventTypeSpec		cmdEvent	= { kEventClassCommand, kEventCommandProcess };
	
	// Make sure the arrow cursor is displayed

	InitCursor();

    err = AEInstallEventHandler(kCoreEventClass, kAEQuitApplication, NewAEEventHandlerUPP(QuitAppleEventHandler), 0, false);
	if (err != noErr)
		ExitToShell();

	// Open our nib and set up the menu bar from it

	err = CreateNibReference( CFSTR( "Magnify" ), &nibRef );
	require_noerr( err, CantOpenNib );

	err = Gestalt(gestaltMenuMgrAttr, &response);
	if ((err == noErr) && (response & gestaltMenuMgrAquaLayoutMask))
		err = SetMenuBarFromNib( nibRef, CFSTR( "MainMenuX" ) );
	else
		err = SetMenuBarFromNib( nibRef, CFSTR( "MainMenu9" ) );
	require_noerr( err, CantSetMenuBar );

	
	// We have a preference dialog, so we need to activate
	// the Preferences menu item in our Application menu.
	// The way to do that is by command ID. Normally, this
	// menu item is disabled.

	EnableMenuCommand( NULL, kHICommandPreferences );
	
	// Create a global port so we can copy bits from the screen

	sGlobalPort = CreateNewPort();
	require_noerr( QDError(), NewPortFailed );

	// Let's see the menu bar, please.
	
	DrawMenuBar();

	// Load any prefs. We use CFPreferences to store our preferences.
	// When doing this, you need to create a key for your application.
	// Normally this is the bundle ID of your app.

	sShowInfo = CFPreferencesGetAppBooleanValue( CFSTR( "ShowInfo" ), kPreferencesKey, &test );
	if ( !test ) sShowInfo = true;

	sMagFactor = CFPreferencesGetAppIntegerValue( CFSTR( "MagFactor" ), kPreferencesKey, &test );
	if ( !test ) sMagFactor = 8;
	
	sFloats = CFPreferencesGetAppBooleanValue( CFSTR( "Float" ), kPreferencesKey, &test );
	if ( !test ) sFloats = false;
	
	// Now create our window and dispose the nib ref - we're done with it.
	
	CreateMagnifyWindow( nibRef );
	DisposeNibReference( nibRef );

	// Because we want to respond to the preferences menu item, we need to
	// install a command handler to handle the menu selection. Here we are
	// creating the UPP on the fly and not disposing it, since it will get
	// destroyed when the app terminates.

	InstallApplicationEventHandler( NewEventHandlerUPP( AppCommandHandler ), 1, &cmdEvent, 0, NULL );

	// Run Lola, Run!

	RunApplicationEventLoop();

	// Clean up and exit.

	DisposePort( sGlobalPort );

	return noErr;
	
NewPortFailed:
CantSetMenuBar:
	DisposeNibReference( nibRef );

CantOpenNib:
	return err;
}

//----------------------------------------------------------------------------
//	• AppCommandHandler
//
//	Look for the preferences menu item and call DoPreferences when we
//	receive it.
//----------------------------------------------------------------------------

static pascal OSStatus
AppCommandHandler( EventHandlerCallRef inHandler, EventRef inEvent, void* inUserData )
{
	#pragma unused( inHandler, inUserData )
	
	HICommand			cmd;
	OSStatus			result = eventNotHandledErr;
	
	GetEventParameter( inEvent, kEventParamDirectObject, typeHICommand, NULL, sizeof( cmd ), NULL, &cmd );
	if ( cmd.commandID == kHICommandPreferences )
	{
		DoPreferences();
		result = noErr;
	}
	
	return result;
}

//----------------------------------------------------------------------------
//	• DoPreferences
//
//	Open our preferences dialog and run it modally.
//----------------------------------------------------------------------------

static void
DoPreferences()
{
	IBNibRef			nibRef;
	EventTypeSpec		cmdEvent = { kEventClassCommand, kEventCommandProcess };
	WindowRef			window;
	OSStatus			err;
	ControlRef			control;
	PrefInfo			info;
	EventHandlerUPP		handler;
	
	// Open our nib, create the window, and close the nib.
	err = CreateNibReference( CFSTR( "Magnify" ), &nibRef );
	require_noerr( err, CantOpenNib );
	
	err = CreateWindowFromNib( nibRef, CFSTR( "Preferences" ), &window );
	require_noerr( err, CantCreateWindow );

	DisposeNibReference( nibRef );

	// For each control of interest, set its value as appropriate
	// for the settings we currently have.

	GetControlByID( window, &kMagSlider, &control );
	SetControlValue( control, (sMagFactor >> 2) + 1 );

	GetControlByID( window, &kShowInfoCheck, &control );
	SetControlValue( control, sShowInfo );
	
	GetControlByID( window, &kFloatCheck, &control );
	SetControlValue( control, sFloats );
	
	// Build up a structure to pass to the window handler we are about
	// to install. We store the window itself, as well as the original
	// states of our settings. We use this to revert if the user clicks
	// the cancel button.

	info.window 		= window;
	info.origMagFactor 	= sMagFactor;
	info.origShowInfo 	= sShowInfo;
	info.origFloats 	= sFloats;

	// Now create our UPP and install the handler.
	
	handler = NewEventHandlerUPP( PrefHandler );
	InstallWindowEventHandler( window, handler, 1, &cmdEvent, &info, NULL );
	
	// Position and show the window
	
	RepositionWindow( window, NULL, kWindowAlertPositionOnMainScreen );
	ShowWindow( window );
	
	// Now we run modally. We will remain here until the PrefHandler
	// calls QuitAppModalLoopForWindow if the user clicks OK or
	// Cancel.

	RunAppModalLoopForWindow( window );

	// OK, we're done. Dispose of our window and our UPP.
	// We do the UPP last because DisposeWindow can send out
	// CarbonEvents, and we haven't explicitly removed our
	// handler. If we disposed the UPP, the Toolbox might try
	// to call it. That would be bad.

	DisposeWindow( window );
	DisposeEventHandlerUPP( handler );

	return;

CantCreateWindow:
	DisposeNibReference( nibRef );
	
CantOpenNib:
	return;
}

//----------------------------------------------------------------------------
//	• PrefHandler
//
//	Our command handler for the Preferences dialog.
//----------------------------------------------------------------------------

static pascal OSStatus
PrefHandler( EventHandlerCallRef inHandler, EventRef inEvent, void* inUserData )
{
	#pragma unused( inHandler )
	
	HICommand			cmd;
	OSStatus			result = eventNotHandledErr;
	PrefInfo*			info = (PrefInfo*)inUserData;
	ControlRef			control;
	CFNumberRef 		number;
	WindowRef			window = info->window;

	// The direct object for a 'process commmand' event is the HICommand.
	// Extract it here and switch off the command ID.

	GetEventParameter( inEvent, kEventParamDirectObject, typeHICommand, NULL, sizeof( cmd ), NULL, &cmd );

	switch ( cmd.commandID )
	{
		case kHICommandOK:
			number = CFNumberCreate( NULL, kCFNumberSInt8Type, &sMagFactor );
			CFPreferencesSetAppValue( CFSTR( "MagFactor" ), number, kPreferencesKey );
			CFPreferencesSetAppValue( CFSTR( "ShowInfo" ), sShowInfo ? kCFBooleanTrue : kCFBooleanFalse, kPreferencesKey );
			CFPreferencesSetAppValue( CFSTR( "Float" ), sFloats ? kCFBooleanTrue : kCFBooleanFalse, kPreferencesKey );
			CFPreferencesAppSynchronize( kPreferencesKey );
			CFRelease( number );

			QuitAppModalLoopForWindow( window );
			result = noErr;
			break;
		
		case kHICommandCancel:
			sMagFactor = info->origMagFactor;
			sShowInfo = info->origShowInfo;
			sFloats = info->origFloats;
			AdjustWindowFloating();
			sForceUpdate = true;
			QuitAppModalLoopForWindow( window );
			result = noErr;
			break;
		
		case kCmdMagnificationChanged:
			GetControlByID( window, &kMagSlider, &control );
			sMagFactor = 2 << (GetControlValue( control ) - 1);
			sForceUpdate = true;
			result = noErr;
			break;
		
		case kCmdShowMouseLocation:
			GetControlByID( window, &kShowInfoCheck, &control );
			sShowInfo = GetControlValue( control );
			sForceUpdate = true;
			result = noErr;
			break;

		case kCmdFloat:
			GetControlByID( window, &kFloatCheck, &control );
			sFloats = GetControlValue( control );
			AdjustWindowFloating();
			result = noErr;
			break;
	}	
	return result;
}

//----------------------------------------------------------------------------
//	• AdjustWindowFloating
//
//	Adjust our window's floating state based on the current setting. We can
//	either float in our application's layer, or above all applications. We
//	accomplish this magic by moving the window between the appropriate window
//	groups.
//----------------------------------------------------------------------------

static void
AdjustWindowFloating()
{
	check( sMagnifyWindow != NULL );
	
	if ( sFloats )
		SetWindowGroup( sMagnifyWindow, GetWindowGroupOfClass( kUtilityWindowClass ) );
	else
		SetWindowGroup( sMagnifyWindow, GetWindowGroupOfClass( kFloatingWindowClass ) );
}
