/*
	File:		main.c
	
	Contains:	This sample demonstrates a variety of techniques and features of Carbon.
				• How to display sheets on Mac OS X, & Alert windows through CarbonLib
				• How to install the default event handlers without really using RunApplicationEventLoop()
				• How to efficiently set up and use cooperative threads from a CarbonEvent based application
				• Uses CarbonEvents on the Application, window, and control targets
				• Uses EventLoopTimers
				• How to bundle your CFM based application for use in CodeWarrior
				• How to use Nib's within your application
					- How to set up command IDs within InterfaceBuilder
					- How to automatically set up help tags for Mac OS X within InterfaceBuilder
				• Tab controls
				• How to enable the Preferences menu
				• CFPreferences
				• How to add a help book
					- How to add a menu item to the Help menu
				• 128 Bit Icon

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

	Copyright © 2001 Apple Computer, Inc., All Rights Reserved
*/


#ifdef __APPLE_CC__
#include <Carbon/Carbon.h>
#else
#include <Carbon.h>
#endif

#include	"Main.h"

static	pascal	OSStatus MyTabEventHandlerProc( EventHandlerCallRef inCallRef, EventRef inEvent, void* inUserData );
static	pascal	OSStatus AppEventEventHandlerProc( EventHandlerCallRef inCallRef, EventRef inEvent, void* inUserData );
static	pascal	OSStatus MainWindowEventHandlerProc( EventHandlerCallRef inCallRef, EventRef inEvent, void* inUserData );
static	pascal	void MainRunLoopForThreadedApps( EventLoopTimerRef inTimer, void *inUserData );
static	pascal	OSErr QuitAppleEventHandler( const AppleEvent *appleEvt, AppleEvent* reply, long refcon );
static	pascal	OSStatus PrefsWindowEventHandlerProc( EventHandlerCallRef inCallRef, EventRef inEvent, void* inUserData );
static	pascal	OSStatus AboutBoxWindowEventHandlerProc( EventHandlerCallRef inCallRef, EventRef inEvent, void* inUserData );
static	void	InstallMainWindowEventHandlers( WindowRef window );
static	void	InitApplication( void );
static	void	MySwitchItemOfTabControl( WindowRef window, ControlRef tabControl, short currentTabIndex );
static	void	SaveMainWindowPrefs( WindowRef window );
static	void	SetMainWindowAttributesFromPrefs( WindowRef window );
static	void	SaveApplicationPrefs( WindowRef window );
static	void	GetApplicationPrefs( void );
static	OSStatus	RegisterMyHelpBook( void );
static	OSStatus	GoToMyHelpAnchor( CFStringRef anchorName );
static	CFStringRef	GetMachineNameAsCFString();

void	DisplayAlertWindow( WindowRef parentWindow, StringPtr alertS );
static pascal OSStatus AlertWindowEventHandlerProc( EventHandlerCallRef inCallRef, EventRef inEvent, void *inUserData );


GlobalsStruct	g;							//	Globals


//	Main entry point
int	main( int argc, char* argv[] )
{
	HICommand		newCommand		= { 0, kHICommandNew };
	
    InitApplication();
    
	(void) InstallEventLoopTimer( GetCurrentEventLoop(), 0, 0, NewEventLoopTimerUPP( MainRunLoopForThreadedApps ), NULL, &g.mainRunLoopTimerRef );

	ProcessHICommand( &newCommand );		//	Open a new window
	
	//	We really only call RunApplicationEventLoop() to install the default CarbonEvent handlers.  Once the timer installed above
	//	fires, we remain in the MainRunLoopForThreadedApps() routine which is designed to yield to other cooperative threads.
    RunApplicationEventLoop();

    // We don't need the nib reference anymore.
    DisposeNibReference( g.mainNibRef );

	return( noErr );
}


//	Q&A 1061 discusses how to handle cooperative threads from a CarbonEvent based application
//	http://developer.apple.com/qa/qa2001/qa1061.html
//	Main Event Loop
static	pascal	void	MainRunLoopForThreadedApps( EventLoopTimerRef inTimer, void *inUserData )
{
	OSStatus		err;
	EventRef		theEvent;
	EventTimeout	timeToWaitForEvent;
	EventTargetRef	theTarget			= GetEventDispatcherTarget();
	
	do
	{
	    if ( g.numberOfRunningThreads == 0 )
	        timeToWaitForEvent	= kEventDurationForever;
	    else
	        timeToWaitForEvent	= kEventDurationNoWait;
	    
	    err = ReceiveNextEvent( 0, NULL, timeToWaitForEvent, true, &theEvent );
	    
	    if ( err == noErr )
	    {
	        (void) SendEventToEventTarget( theEvent, theTarget );
	        ReleaseEvent( theEvent );
	    }
	    else if ( err == eventLoopTimedOutErr )
	    {
	        err = noErr;
	    }
	    if ( g.numberOfRunningThreads > 0 )
	        (void) YieldToAnyThread();
												//	eventLoopQuitErr may be sent to wake up the queue and does not necessarily
	} while ( g.done == false );				//	mean 'quit', we handle the Quit case from our AppleEvent handler
}


static	Boolean	RunningOnCarbonX( void )
{
    UInt32 response;
    
    return( Gestalt(gestaltSystemVersion, 
                    (SInt32 *) &response) == noErr)
                && (response >= 0x01000 );
}

static	Boolean	RunningOnCarbonTenPointOneOrHigher( void )
{
    UInt32 response;
    
     return( Gestalt(gestaltSystemVersion, 
                    (SInt32 *) &response) == noErr)
                && (response >= 0x01010 );
}

static	void	InitApplication( )
{
    long				response;
    OSStatus			err;
	MenuRef				menuHandle;
	MenuItemIndex		customItemIndex;
	Rect				rect;
	static const EventTypeSpec	sAppEvents[] =
	{
		{ kEventClassCommand, kEventCommandProcess },
		{ kEventClassApplication, kEventAppActivated }
	};
    
	InitCursor();

    // Create a Nib reference passing the name of the nib file (without the .nib extension)
    // CreateNibReference only searches into the application bundle.
    err	= CreateNibReference( CFSTR("main"), &g.mainNibRef );
    if ( err != noErr ) goto Bail;
    
    // Once the nib reference is created, set the menu bar. "MainMenu" is the name of the menu bar
    // object. This name is set in InterfaceBuilder when the nib is created.
	err = Gestalt( gestaltMenuMgrAttr, &response );
    if ( err != noErr ) goto Bail;
	if ( response & gestaltMenuMgrAquaLayoutMask )
		err = SetMenuBarFromNib( g.mainNibRef, CFSTR("MenuBar") );
	else
		err = SetMenuBarFromNib( g.mainNibRef, CFSTR("MenuBar9") );
    if ( err != noErr ) goto Bail;
        
	err	= RegisterMyHelpBook();
	if ( err == noErr )									//	Add custom Help Menu items
	{
		err	= HMGetHelpMenu( &menuHandle, &customItemIndex );
		if ( err == noErr )
		{
			if ( RunningOnCarbonX() == false )			//	Mac OS 8/9 bundled applications must manually add and handle their help menu
				(void) SetMenuItemCommandKey( menuHandle, customItemIndex, true, 0x2C );	//	Set <command> '/' to the menu
		
			InsertMenuItem( menuHandle, "\pGo To The Images Help", customItemIndex - 1 );		//	Insert our extra Help item
			(void) SetMenuItemCommandID( menuHandle, customItemIndex, 'XHlp' );
		}
	}

	//	Create & initialize our "Large Cursor" window
    SetRect( &rect, 50, 50, 114, 114 );					//	left, top, right, bottom
    err = CreateNewWindow( kOverlayWindowClass, kWindowHideOnSuspendAttribute, &rect, &g.cursorWindow );
	//	if ( err == errUnrecognizedWindowClass )		//	Disabled on Mac OS 8/9

	InstallApplicationEventHandler( NewEventHandlerUPP(AppEventEventHandlerProc), GetEventTypeCount(sAppEvents), sAppEvents, 0, NULL );

    (void) AEInstallEventHandler( kCoreEventClass, kAEQuitApplication, NewAEEventHandlerUPP(QuitAppleEventHandler), 0, false );
	
	g.numberOfRunningThreads	= 0;					//	Initialize globals
	g.done						= false;
	g.largeCursorActive			= false;
	g.largeCursorEventHandler	= NewEventHandlerUPP( LargeCursorEventHandler );
	GetApplicationPrefs();
	
	EnableMenuCommand( NULL, kHICommandPreferences );	//	Enable the preferences menu

Bail:
	return;
}


static	void	InstallMainWindowEventHandlers( WindowRef window )
{
    ControlRef	control;
    OSStatus	err;

    static const EventTypeSpec	tabControlEvents[] =
    {
		{ kEventClassControl, kEventControlClick },		//	Sent before the tab control switches its control value
		{ kEventClassControl, kEventControlHit },
		{ kEventClassCommand, kEventCommandProcess }
    };
    
    static const EventTypeSpec	windowEvents[] =
    {
		{ kEventClassCommand, kEventCommandProcess },
		{ kEventClassWindow, kEventWindowClose }
    };
    
    g.tabControlID.signature	= 'AAPL';
    g.tabControlID.id			= 128;
    GetControlByID( window, &g.tabControlID, &control );

	err	= InstallControlEventHandler( control, NewEventHandlerUPP( MyTabEventHandlerProc ), GetEventTypeCount(tabControlEvents), tabControlEvents, window, NULL );
	if ( err != noErr )	SysBeep(0);
		
	SetMainWindowAttributesFromPrefs( window );

	//	We pass "window" as the userData to make it available from our event handler
	err	= InstallWindowEventHandler( window, NewEventHandlerUPP( MainWindowEventHandlerProc ), GetEventTypeCount(windowEvents), windowEvents, window, NULL );
	if ( err != noErr )	SysBeep(0);
}


static pascal OSStatus AppEventEventHandlerProc( EventHandlerCallRef inCallRef, EventRef inEvent, void* inUserData )
{
	#pragma unused ( inCallRef, inUserData )
	HICommand		command;
	WindowRef		window;
	OSStatus 		err			= eventNotHandledErr;
	UInt32			eventClass	= GetEventClass( inEvent );
	UInt32			eventKind	= GetEventKind(inEvent);
	
	switch ( eventClass )
	{
		case kEventClassCommand:
			GetEventParameter( inEvent, kEventParamDirectObject, typeHICommand, NULL, sizeof(HICommand), NULL, &command );
			if ( eventKind == kEventCommandProcess )
			{
				if ( command.commandID == kHICommandNew )
				{
				    err	= CreateWindowFromNib( g.mainNibRef, CFSTR("MainWindow"), &window );
					if ( err == noErr )
					{
						WindowinfoStruct	*windowinfo	= (WindowinfoStruct*)NewPtrClear( sizeof(WindowinfoStruct) );
						windowinfo->windowidentifier	= kMainTabWindowType;
						SetWRefCon( window, (long) windowinfo );
						InstallMainWindowEventHandlers( window );
						//CreateCGContextForPort( GetWindowPort(window), &windowinfo->cgContext );
						CreateMyDataBrowserControl( window );
						ShowWindow( window );
						SetPortWindowPort( window );
					}
					
				}
				else if ( command.commandID == kHICommandPreferences )		//	Handle selection of the preferences menu
				{
				    err	= CreateWindowFromNib( g.mainNibRef, CFSTR("PreferencesWindow"), &window );
					if ( err == noErr )
					{
						ControlRef		control;
   						ControlID		controlID	= { 'PChk', 1 };
					    static const EventTypeSpec	windowEvents[] =
					    {	{ kEventClassCommand, kEventCommandProcess },
							{ kEventClassWindow, kEventWindowClose }
						};
						err	= InstallWindowEventHandler( window, NewEventHandlerUPP( PrefsWindowEventHandlerProc ), GetEventTypeCount(windowEvents), windowEvents, window, NULL );
						
						DisableMenuCommand( NULL, kHICommandPreferences );
						
						GetControlByID( window, &controlID, &control );
					    SetControlValue( control, g.saveOnClose );
						
						ShowWindow( window );
					}
				}
				else if ( command.commandID == kHICommandAbout )			//	Handle selection of the About Box menu
				{
				    err	= CreateWindowFromNib( g.mainNibRef, CFSTR("AboutBox"), &window );
					if ( err == noErr )
					{
					    const EventTypeSpec	windowEvents[]	= { { kEventClassMouse, kEventMouseDown } };
						err	= InstallWindowEventHandler( window, NewEventHandlerUPP( AboutBoxWindowEventHandlerProc ), GetEventTypeCount(windowEvents), windowEvents, window, NULL );
						
						DisableMenuCommand( NULL, kHICommandAbout );
						
						ShowWindow( window );
					}
				}
				else if ( command.commandID == 'GHlp' )				//	Mac OS 8/9 must handle Help menu selection
				{
					(void) GoToMyHelpAnchor( NULL );				//	Pass NULL to go to main page
				}
				else if ( command.commandID == 'XHlp' )				//	Our extra help menu item
				{
					(void) GoToMyHelpAnchor( CFSTR("ImagesAnchor") );
				}
			}
			break;
			
		case kEventClassApplication:
			if ( eventKind == kEventAppActivated )
				SetThemeCursor( kThemeArrowCursor );
			break;
	}
	
    return( eventNotHandledErr );
}




static pascal OSStatus MainWindowEventHandlerProc( EventHandlerCallRef inCallRef, EventRef inEvent, void *inUserData )
{
	HICommand		command;
    ControlRef		control;
    SInt32			dataSize;
	EventRef		event;
	ThreadID		id;
	OSErr			err;
    OSStatus		result			= eventNotHandledErr;
	WindowRef		window			= (WindowRef) inUserData;
	UInt32			eventClass		= GetEventClass( inEvent );
	UInt32			eventKind		= GetEventKind( inEvent );
	
	switch ( eventClass )
	{
		case kEventClassCommand:
			if ( eventKind == kEventCommandProcess )
			{
				GetEventParameter( inEvent, kEventParamDirectObject, typeHICommand, NULL, sizeof(HICommand), NULL, &command );
				if ( command.commandID == 'SCrs' )					//	Popup menu "Small Cursor"
				{
					if ( g.cursorWindow != NULL )
					{
						if ( g.largeCursorActive == true )
						{
							RemoveEventHandler( g.largeCursorEventHandlerRef );
			    			HideWindow( g.cursorWindow );
		    				g.largeCursorActive	= false;
		    			}
		    		}
		    		else
		    		{
						DisplayAlertWindow( window, "\pOverlay windows are not supported in this configuration." );
		    		}
				}
				else if ( command.commandID == 'LCrs' )				//	Popup menu "Large Cursor"
				{
					if ( g.cursorWindow != NULL )
					{
						if ( g.largeCursorActive == false )
						{
							const EventTypeSpec	largeCursorAppEvents[]	= { 
											{ kEventClassMouse, kEventMouseMoved }, 
											{ kEventClassMouse, kEventMouseDragged },
											{ kEventClassWindow, kEventWindowBoundsChanging }
							};
		    				ShowWindow( g.cursorWindow );
							InstallApplicationEventHandler( g.largeCursorEventHandler, GetEventTypeCount(largeCursorAppEvents), largeCursorAppEvents, (void*)GetWRefCon(window), &g.largeCursorEventHandlerRef );
		    				g.largeCursorActive	= true;
						}
					}
					else
					{
						DisplayAlertWindow( window, "\pOverlay windows are not supported in this configuration." );
					}
				}
				else if ( command.commandID == 'MNme' )				//	Machine Name button
				{
					CFStringRef	machineNameCF	= GetMachineNameAsCFString();	//	Get the FileSharing machine name
					if ( machineNameCF != NULL )
					{
    					ControlID		controlID	= { 'MNme', 1 };
						GetControlByID( window, &controlID, &control );
						err	= SetControlData( control, 0, kControlStaticTextCFStringTag, sizeof(CFStringRef), &machineNameCF );
						if ( err == errDataNotSupported )
						{
							ConstStringPtr	pString	= CFStringGetPascalStringPtr( machineNameCF, kCFStringEncodingMacRoman );
							(void) SetControlData( control, 0, kControlStaticTextTextTag, pString[0], pString+1 );
						}
						Draw1Control( control );
						CFRelease( machineNameCF );
					}
				}
				else if ( command.commandID == 'Srch' )				//	Launch the "Search Thread Window"
				{
					CFStringRef		cfString;
    				ControlID		controlID	= { 'Extn', 0 };

					GetControlByID( window, &controlID, &control );
					err	= GetControlData( control, 0, kControlStaticTextCFStringTag, sizeof(CFStringRef), (Ptr)&cfString, &dataSize );
					if ( err != noErr )
					{
    					char		name[64];
						err	= GetControlData( control, 0, kControlStaticTextTextTag, 64, name, &dataSize );
						name[dataSize]	= '\0';
						cfString	= CFStringCreateWithCString( NULL, name, CFStringGetSystemEncoding() );
					}

					result = NewThread( kCooperativeThread, NewThreadEntryUPP((ThreadEntryProcPtr) ProgressThread), (void *)cfString, 0, kCreateIfNeeded, nil, &id );
					
					if ( result == noErr )
						g.numberOfRunningThreads++;					//	Increment the total number of threads
				}
				else if ( command.commandID == 'IOwr' )
				{
					OpenGrabBagSharedFileWithWriteAccess( window );
				}
				else if ( command.commandID == 'IOrd' )
				{
					DisplayDataFromGrabBagSharedFile( window );
				}
				else if ( command.commandID == 'IOcl' )
				{
					CloseGrabBagSharedFile( window );
				}
				else if ( command.commandID == 'IOsv' )
				{
					SaveDataToGrabBagSharedFile( window );
				}
				else if ( command.commandID == kHICommandClose )	//	<command>-W sends this command
				{													//	We turn around and send it as a kEventWindowClose 
					(void) CreateEvent( NULL,  kEventClassWindow, kEventWindowClose, GetCurrentEventTime(), kEventAttributeUserEvent, &event );
					(void) SetEventParameter( event, kEventParamDirectObject, typeWindowRef, sizeof(window), &window );
					(void) SendEventToWindow( event, GetUserFocusWindow() );
				}
			}
			break;
		
		case kEventClassWindow:
				if ( eventKind == kEventWindowClose )
				{
					if ( g.saveOnClose == true )
						SaveMainWindowPrefs( window );				//	Save Prefs
				}
			break;
	}
    
    return( eventNotHandledErr );									//	Use default handlers for things like kHICommandBringAllToFront
}


//	Event handler for the Preferences window
static pascal OSStatus PrefsWindowEventHandlerProc( EventHandlerCallRef inCallRef, EventRef inEvent, void* inUserData )
{
	HICommand		command;
	EventRef		event;
	WindowRef		window			= (WindowRef) inUserData;
	UInt32			eventClass		= GetEventClass( inEvent );
	UInt32			eventKind		= GetEventKind( inEvent );

	switch ( eventClass )
	{
		case kEventClassCommand:
			if ( eventKind == kEventCommandProcess )
			{
				GetEventParameter( inEvent, kEventParamDirectObject, typeHICommand, NULL, sizeof(HICommand), NULL, &command );
				if ( command.commandID == kHICommandClose )			//	We turn around and send it as a kEventWindowClose
				{
					(void) CreateEvent( NULL,  kEventClassWindow, kEventWindowClose, GetCurrentEventTime(), kEventAttributeUserEvent, &event );
					(void) SetEventParameter( event, kEventParamDirectObject, typeWindowRef, sizeof(window), &window );
					(void) SendEventToWindow( event, GetUserFocusWindow() );
				}
			}
			break;
		
		case kEventClassWindow:
				if ( eventKind == kEventWindowClose )
				{
					SaveApplicationPrefs( window );						//	Save Prefs
					EnableMenuCommand( NULL, kHICommandPreferences );
				}
			break;
	}
    
    return( eventNotHandledErr );
}

//	We only subscribe to the MouseDown event for the window, so we just send ourselves a kEventWindowClose event, 
//	and let thew default handler dispose it.
static pascal OSStatus AboutBoxWindowEventHandlerProc( EventHandlerCallRef inCallRef, EventRef inEvent, void* inUserData )
{
	HICommand		command;
	EventRef		event;
	WindowRef		window			= (WindowRef) inUserData;

	GetEventParameter( inEvent, kEventParamDirectObject, typeHICommand, NULL, sizeof(HICommand), NULL, &command );
	
	(void) CreateEvent( NULL,  kEventClassWindow, kEventWindowClose, GetCurrentEventTime(), kEventAttributeUserEvent, &event );
	(void) SetEventParameter( event, kEventParamDirectObject, typeWindowRef, sizeof(window), &window );
	(void) SendEventToWindow( event, GetUserFocusWindow() );
    
	EnableMenuCommand( NULL, kHICommandAbout );
    
    return( eventNotHandledErr );
}


//	Event handler for the Tab control
//	Derrived from sample in O'Reilly's "Learning Carbon" book, page 298
static pascal OSStatus MyTabEventHandlerProc( EventHandlerCallRef inCallRef, EventRef inEvent, void* inUserData )
{
    ControlRef		tabControl;
    static	short	lastTabIndex;
    short			controlValue;
    WindowRef		window			= (WindowRef)inUserData;

	GetControlByID( window, &g.tabControlID, &tabControl );
    controlValue	= GetControlValue( tabControl );
    if ( controlValue != lastTabIndex )
    {
    	MySwitchItemOfTabControl( window, tabControl, controlValue );
    	lastTabIndex	= controlValue;
    }
    
    return( eventNotHandledErr );
}


//	We must manually display the correct Tab information
static	void	MySwitchItemOfTabControl( WindowRef window, ControlRef tabControl, short currentTabIndex )
{
    ControlID		controlID;
    ControlRef		userPaneControl;
    short			i;
    ControlRef		selectedPaneControl	= NULL;
 	int				tabList[]			= {5, 1, 2, 3, 4, 5};				//	Information about this tab, # of tabs, and IDs
   
    controlID.signature	= 'AAPL';
    
    for ( i=1 ; i<tabList[0]+1 ; i++ )
    {
    	controlID.id	= tabList[i];
   	 	GetControlByID( window, &controlID, &userPaneControl );
   	 	
   	 	if ( i == currentTabIndex )
   	 		selectedPaneControl	= userPaneControl;
   	 	else
   	 		SetControlVisibility( userPaneControl, false, true );
    }
	if ( selectedPaneControl != NULL )
	{
		(void) ClearKeyboardFocus( window );
		SetControlVisibility( selectedPaneControl, true, true );
	}
	
	Draw1Control( tabControl );										//	Redraw the Tab control itself
}


static pascal OSErr QuitAppleEventHandler( const AppleEvent *appleEvt, AppleEvent* reply, long refcon )
{
#pragma unused ( appleEvt, reply, refcon )
	(void) XFSClose( &g.sharedFileRefNum );								//	Close shared file, if it's open
	g.done	= true;													//	Quit our event loop
	QuitApplicationEventLoop();
	return( noErr );
}


//	Demonstrates how to use CFPreferences to store simple preferences
static	void	SaveApplicationPrefs( WindowRef window )
{
	CFNumberRef		saveOnCloseRef;
	ControlRef		control;
	ControlID		controlID	= { 'PChk', 1 };
	
	GetControlByID( window, &controlID, &control );
    g.saveOnClose	= (Boolean) GetControlValue( control );

	saveOnCloseRef	= CFNumberCreate( NULL, kCFNumberSInt8Type, &g.saveOnClose );
	CFPreferencesSetAppValue( CFSTR("AppPrefsSaveOnClose"), saveOnCloseRef, kCFPreferencesCurrentApplication );

	CFRelease( saveOnCloseRef );
	
	(void) CFPreferencesAppSynchronize( kCFPreferencesCurrentApplication );		//	Write them to disk!
}

static	void	GetApplicationPrefs( void )
{
	CFNumberRef		saveOnCloseRef;
	
	saveOnCloseRef		= CFPreferencesCopyAppValue( CFSTR("AppPrefsSaveOnClose"), kCFPreferencesCurrentApplication );
	if ( saveOnCloseRef != NULL )
	{
		(void) CFNumberGetValue( saveOnCloseRef, kCFNumberSInt8Type, &g.saveOnClose );
		CFRelease( saveOnCloseRef );
	}
	else
	{
		g.saveOnClose	= false;
	}
}

//	Demonstrates how to use CFPreferences to store preferences saved as a CFArray
static	void	SaveMainWindowPrefs( WindowRef window )
{
	Rect			rect;
	CFNumberRef		selectedTab;
	CFNumberRef		bounds[4];
	CFArrayRef		boundsArray;
	ControlRef		control;
	SInt16			controlValue;
	
	//	Save the window bounds
	SetPortWindowPort( window );
	GetWindowPortBounds( window, &rect );
	
	LocalToGlobal( (Point*) &(rect.top) );
	LocalToGlobal( (Point*) &(rect.bottom) );
	
	bounds[0]	= CFNumberCreate( NULL, kCFNumberSInt16Type, &(rect.top) );
	bounds[1]	= CFNumberCreate( NULL, kCFNumberSInt16Type, &(rect.left) );
	bounds[2]	= CFNumberCreate( NULL, kCFNumberSInt16Type, &(rect.bottom) );
	bounds[3]	= CFNumberCreate( NULL, kCFNumberSInt16Type, &(rect.right) );

	boundsArray	= CFArrayCreate( NULL, (const void **)bounds, 4, &kCFTypeArrayCallBacks );
	CFPreferencesSetAppValue( CFSTR( "MainWindowBounds" ), boundsArray, kCFPreferencesCurrentApplication );
	
	CFRelease( boundsArray );
  	CFRelease( bounds[0] );
  	CFRelease( bounds[1] );
  	CFRelease( bounds[2] );
  	CFRelease( bounds[3] );
  	
  	//	Save the default Tab
	GetControlByID( window, &g.tabControlID, &control );
	controlValue	= GetControlValue( control );
	selectedTab		= CFNumberCreate( NULL, kCFNumberSInt16Type, &controlValue );
	
	CFPreferencesSetAppValue( CFSTR("DefaultTab"), selectedTab, kCFPreferencesCurrentApplication );

	CFRelease( selectedTab );
	
	(void) CFPreferencesAppSynchronize( kCFPreferencesCurrentApplication );		//	Write them to disk!
}


static	void	SetMainWindowAttributesFromPrefs( WindowRef window )
{
	Rect			rect;
	CFNumberRef		selectedTab;
	CFNumberRef		bounds[4];
	CFArrayRef		boundsArray;
	ControlRef		control;
	SInt16			controlValue;
	Boolean			lossyConversion;

	boundsArray	= CFPreferencesCopyAppValue( CFSTR( "MainWindowBounds" ), kCFPreferencesCurrentApplication );
	if ( boundsArray != NULL )
	{
		CFRange range = { 0, 4 };
		CFArrayGetValues( boundsArray, range, (const void **)&bounds );

		(void) CFNumberGetValue( bounds[0], kCFNumberSInt16Type, &(rect.top) );
		(void) CFNumberGetValue( bounds[1], kCFNumberSInt16Type, &(rect.left) );
		(void) CFNumberGetValue( bounds[2], kCFNumberSInt16Type, &(rect.bottom) );
		(void) CFNumberGetValue( bounds[3], kCFNumberSInt16Type, &(rect.right) );
		CFRelease( boundsArray );

		MoveWindow( window, rect.left, rect.top, true );
		SizeWindow( window, rect.right - rect.left, rect.bottom - rect.top, true );
	}

	GetControlByID( window, &g.tabControlID, &control );
	selectedTab		= CFPreferencesCopyAppValue( CFSTR("DefaultTab"), kCFPreferencesCurrentApplication );
	if ( selectedTab != NULL )
	{
		lossyConversion	= CFNumberGetValue( selectedTab, kCFNumberSInt16Type, &controlValue );
		CFRelease( selectedTab );

		SetControlValue( control, controlValue );
	}
	MySwitchItemOfTabControl( window, control, GetControlValue( control ) );
}



//	http://developer.apple.com/qa/qa2001/qa1022.html
static	OSStatus	RegisterMyHelpBook( void )
{
	FSRef		myBundleRef;
	OSStatus	err;
	CFBundleRef	myAppsBundle	= NULL;
	CFURLRef	myBundleURL		= NULL;
    
    myAppsBundle	= CFBundleGetMainBundle();					//	Get our application's main bundle from Core Foundation
    if ( myAppsBundle == NULL ) { err = fnfErr; goto Bail; }
    
    myBundleURL	= CFBundleCopyBundleURL( myAppsBundle );		//	Retrieve the URL to our bundle
    if ( myBundleURL == nil ) { err = fnfErr; goto Bail; }
    
    if ( ! CFURLGetFSRef( myBundleURL, &myBundleRef ) ) { err = fnfErr; goto Bail; }	//	Convert the URL to a FSRef
    
    err	= AHRegisterHelpBook(&myBundleRef);						//	Register our application's help book
    
Bail:
    if ( myBundleURL != NULL ) CFRelease( myBundleURL );
    return( err );
}


//	http://developer.apple.com/qa/qa2001/qa1006.html
static	OSStatus	GoToMyHelpAnchor( CFStringRef anchorName )
{
	OSStatus	err;
	CFBundleRef	myAppsBundle	= NULL;
	CFTypeRef	myBookName		= NULL;

	myAppsBundle	= CFBundleGetMainBundle();						//	Get our application's main bundle from Core Foundation
	if ( myAppsBundle == NULL ) { err = fnfErr; goto Bail; }

	myBookName	= CFBundleGetValueForInfoDictionaryKey( myAppsBundle, CFSTR("CFBundleHelpBookName") );	//	Get the help book's name
	if ( myAppsBundle == NULL ) { err = fnfErr; goto Bail; }

	if( CFGetTypeID( myBookName ) != CFStringGetTypeID() ) { err = paramErr; goto Bail; }	//	Verify the data type returned

	if ( anchorName != NULL )
		err	= AHLookupAnchor( myBookName, anchorName );				//	Fing and go to the anchor
	else
		err	= AHGotoPage( myBookName, NULL, NULL );					//	Go to main page

Bail:
    return( err );
}


//	This routine gets the file sharing name of the machine regardless of OS version.  On Mac OS 8/9 it gets the name
//	from a resource, on Mac OS X (<10.1) it gets it from the SystemConfiguration/preferences.xml XML file, and On Mac OS X (>=10.1)
//	It calls the Mach-O routine CSCopyMachineNameProc() to retrieve the name.

typedef pascal CFStringRef (*CSCopyMachineNameProc)();			//	Mach-O function deffinition

static	CFStringRef	GetMachineNameAsCFString()
{
	CFStringRef	machineNameCF	= NULL;
    
	if ( RunningOnCarbonX() )
	{
		if ( RunningOnCarbonTenPointOneOrHigher() )
		{
			CSCopyMachineNameProc	CSCopyMachineNameFunc;
			
			CFBundleRef bundle	= CFBundleGetBundleWithIdentifier( CFSTR( "com.apple.Carbon" ) );
			if ( bundle == NULL ) goto Bail;

			CSCopyMachineNameFunc	= (CSCopyMachineNameProc) CFBundleGetFunctionPointerForName( bundle, CFSTR("CSCopyMachineName") );
			
			if ( CSCopyMachineNameFunc != NULL )
				machineNameCF	= CSCopyMachineNameFunc();
		}
		else						//	A bug in Mac OS X 10.0 returns "localhost" rather than the file sharing name 
		{							//	For Mac OS X 10.0, the answer is to read the name directly out of the System Configuration preferences file.
			CFURLRef        url;
			CFDataRef       data;
			CFDictionaryRef dict;
			CFDictionaryRef systemDict;
			CFDictionaryRef system2Dict;
			Boolean			success;
			    
			url		= CFURLCreateWithFileSystemPath( kCFAllocatorDefault, CFSTR("/var/db/SystemConfiguration/preferences.xml"), kCFURLPOSIXPathStyle, FALSE );
			if ( url == NULL ) goto Bail;
    
			success	= CFURLCreateDataAndPropertiesFromResource( kCFAllocatorDefault, url, &data, NULL, NULL, NULL );
			if ( (success==true) && (data != NULL) )
			{
            	dict	= CFPropertyListCreateFromXMLData( kCFAllocatorSystemDefault, data, kCFPropertyListImmutable, NULL ); 
				if ( dict != NULL )
				{
					systemDict	= CFDictionaryGetValue( dict, CFSTR("System") );
					if ( systemDict != NULL )
					{
						system2Dict	= CFDictionaryGetValue( systemDict, CFSTR("System") );
                    	if ( system2Dict != NULL )
                    	{
							machineNameCF	= CFDictionaryGetValue( system2Dict, CFSTR("ComputerName") );
                        }
                    }
					CFRelease( dict );
				}
				CFRelease( data );
			}
			CFRelease( url );
		}
	}
    else																//	Running on Mac OS 8/9
    {
		#define			kMachineNameID		-16413
		SInt16			saveResFile;
		StringHandle	stringH;
        
		saveResFile	= CurResFile();										//	Save current resource file
        UseResFile( kSystemResFile );									//	Make system resource current
               
		stringH	= GetString( kMachineNameID );
		if ( stringH == NULL ) goto Bail;
                
		machineNameCF	= CFStringCreateWithPascalString( kCFAllocatorSystemDefault, *stringH, kCFStringEncodingMacRoman );
        
		UseResFile( saveResFile );
    }
    
Bail:
	return( machineNameCF );
}


void	DisplayAlertWindow( WindowRef parentWindow, StringPtr alertS )
{
	OSErr				err;
	WindowRef			window;
	ControlRef			control;
    ControlID			controlID		= { 'Alrt', 0 };
    EventTypeSpec		windowEvents[]	= { { kEventClassControl, kEventControlClick } };

	err	= CreateWindowFromNib( g.mainNibRef, CFSTR("AlertWindow"), &window );
	if ( err != noErr )	goto Bail;
	
	GetControlByID( window, &controlID, &control );
	(void) SetControlData( control, 0, kControlStaticTextTextTag, alertS[0], alertS+1 );

	err	= InstallWindowEventHandler( window, NewEventHandlerUPP( AlertWindowEventHandlerProc ), GetEventTypeCount(windowEvents), windowEvents, window, NULL );
	if ( err != noErr )	goto Bail;

	err	= ShowSheetWindow( window, parentWindow );
	
Bail:
	if ( err != noErr )	SysBeep(0);
	return;
}


static pascal OSStatus AlertWindowEventHandlerProc( EventHandlerCallRef inCallRef, EventRef inEvent, void *inUserData )
{
	return( HideSheetWindow( (WindowRef) inUserData ) );
}









