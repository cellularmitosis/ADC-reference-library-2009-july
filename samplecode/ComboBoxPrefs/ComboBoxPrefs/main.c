/*
	File:		main.c

	Abstract:	A skeleton of modern nib-based and Carbon Events-based Carbon application.
				ComboBoxPrefs maintains a list of recently accessed pictures within its combo box.
				It is a chronological list (CFArray) stored via CFPreferences.  We also override 
				the standard behavior of the combo box to make it more like an address bar seen in 
				web browsers.  When an item from the ComboBox list is double-clicked that item is 
				loaded. ComboBoxPrefs also demonstrates how to create a CGImageRef from a URL pointing
				to a picture.  We use QuickTime Graphics Importers, QTNewDataReferenceFromCFURL, 
				for automatic support of a wide variety of image types.
				
	Version:	1.0

	Disclaimer:	IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc.
				("Apple") in consideration of your agreement to the following terms, and your
				use, installation, modification or redistribution of this Apple software
				constitutes acceptance of these terms.  If you do not agree with these terms,
				please do not use, install, modify or redistribute this Apple software.

				In consideration of your agreement to abide by the following terms, and subject
				to these terms, Apple grants you a personal, non-exclusive license, under Apple's
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

	Copyright © 2005 Apple Computer, Inc., All Rights Reserved
*/

#include	<Carbon/Carbon.h>
#include	<Quicktime/Quicktime.h>
#include	<sys/param.h>

struct  GlobalAppInfo							//  Application globals
{
	IBNibRef				mainNibRef;
	SInt32					maxHistory;
};
typedef struct GlobalAppInfo GlobalAppInfo;

void	SendCommandProcessEvent( UInt32 commandID );
void	SendWindowCloseEvent( WindowRef window );
static  OSStatus	GetControlBySigAndID( WindowRef window, OSType  signature, SInt32 id, ControlRef *control );
static	void		InstallAppleEventHandlers( void );
static	void		DisplayPreferencesWindow( void );
static	pascal  OSStatus	CommandProcessEventHandler( EventHandlerCallRef nextHandler, EventRef inEvent, void *userData );	//  Added kHICommandRevert, kHICommandPageSetup, and kHICommandPrint
static	pascal	OSStatus	MainWindowEventHandler( EventHandlerCallRef nextHandler, EventRef inEvent, void *userData );
static	OSStatus	DoNewWindow( WindowRef *window );		//  Install Event Handler for Text view
static	pascal	OSStatus	PreferencesWindowEventHandlerProc( EventHandlerCallRef inCallRef, EventRef inEvent, void* inUserData );
SInt16	GetControlValueByID( WindowRef window, OSType signature, SInt32 id );
void	SetControlValueByID( WindowRef window, OSType signature, SInt32 id, SInt32 value );
static  OSStatus	DisplayImageInWindow( WindowRef window, CFURLRef cfURL );
static  pascal  void	LiveAlphaSliderActionProc( ControlRef control, ControlPartCode partCode );
static	OSStatus	HIViewFindBySigAndID( HIViewRef inStartView, OSType signature, SInt32 id, HIViewRef *outControl );
static	pascal	OSStatus	ComboBoxEventHandler( EventHandlerCallRef inCallRef, EventRef inEvent, void *inUserData );
void	SaveComboBoxValues( HIViewRef comboBoxView );
void	RestoreComboBoxValues( HIViewRef comboBoxView );


GlobalAppInfo		g;


#pragma mark -
#pragma mark ¥ Main ¥

int	main( int argc, char *argv[] )
{
	OSStatus				status;
	long					response;
	CFNumberRef				cfNumber					= NULL;
	const   EventTypeSpec   commandProcessEvents[]		= { { kEventClassCommand, kEventCommandProcess } };

	BlockZero( &g, sizeof(g) );

	status	= Gestalt( gestaltSystemVersion, &response );
	if ( ! ((status == noErr) && (response >= 0x00001030)) )			//	We require Mac OS X 10.3 or greater to run
	{
		DialogRef	alertDialod;
		CreateStandardAlert( kAlertStopAlert, CFSTR("Mac OS X 10.3 (minimum) is required for this application"), NULL, NULL, &alertDialod );
		RunStandardAlert( alertDialod, NULL, NULL );
		ExitToShell();
	}

	EnterMovies();														//	Needed for use with QuickTimes Graphics Importers
	
	//	Create a Nib reference passing the name of the nib file (without the .nib extension) CreateNibReference only searches into the application bundle.
	status	= CreateNibReference( CFSTR("main"), &g.mainNibRef );
	require_noerr( status, CantGetNibRef );

	//	Read in our preferences, maximum urls to display in our combo box
	cfNumber	= (CFNumberRef) CFPreferencesCopyAppValue( CFSTR("MaxHistory"), kCFPreferencesCurrentApplication );
	if ( cfNumber != NULL )
	{
		(void) CFNumberGetValue( cfNumber, kCFNumberSInt32Type, &g.maxHistory );
		CFRelease( cfNumber );
	}
	else
	{
		g.maxHistory	= 10;
	}

	//	Once the nib reference is created, set the menu bar. "MainMenu" is the name of the menu bar object. This name is set in InterfaceBuilder when the nib is created.
	status	= SetMenuBarFromNib( g.mainNibRef, CFSTR("MenuBar") );
	require_noerr( status, CantSetMenuBar );

	EnableMenuCommand( NULL, kHICommandPreferences );					//	Enable Preferences menu item

	InstallAppleEventHandlers();
	InstallApplicationEventHandler( NewEventHandlerUPP(CommandProcessEventHandler), GetEventTypeCount(commandProcessEvents), commandProcessEvents, NULL, NULL );
	
	RunApplicationEventLoop();											//	Call the event loop
	
CantSetMenuBar:
CantGetNibRef:
	return( status );
}


#pragma mark -
#pragma mark ¥ AppleEvent Handlers ¥

static	pascal	OSErr	HandleAppleEventOapp( const AppleEvent *theAppleEvent, AppleEvent *reply, long handlerRefcon )
{
	SendCommandProcessEvent( kHICommandNew );							// Instantiate an empty window at the beginning so the User sees something
	return( noErr );
}


static	pascal	OSErr	HandleAppleEventRapp( const AppleEvent *theAppleEvent, AppleEvent *reply, long handlerRefcon )
{
	WindowRef	window	= GetFrontWindowOfClass( kDocumentWindowClass, true );
	if ( window == NULL )
		SendCommandProcessEvent( kHICommandNew );						//	We were already running but with no windows so we create an empty one.
	return( noErr );
}

static	pascal	OSErr	HandleAppleEventOdoc( const AppleEvent *theAppleEvent, AppleEvent *reply, long handlerRefcon )
{
	return( errAEEventNotHandled );
}

static	pascal	OSErr	HandleAppleEventPdoc( const AppleEvent *theAppleEvent, AppleEvent *reply, long handlerRefcon )
{
	return( errAEEventNotHandled );
}

static	void	InstallAppleEventHandlers( void )
{
	OSErr		status;
	
	status	= AEInstallEventHandler( kCoreEventClass, kAEOpenApplication, NewAEEventHandlerUPP(HandleAppleEventOapp), 0, false );	require_noerr( status, CantInstallAppleEventHandler );
	status	= AEInstallEventHandler( kCoreEventClass, kAEReopenApplication, NewAEEventHandlerUPP(HandleAppleEventRapp), 0, false );	require_noerr( status, CantInstallAppleEventHandler );
	status	= AEInstallEventHandler( kCoreEventClass, kAEOpenDocuments, NewAEEventHandlerUPP(HandleAppleEventOdoc), 0, false );		require_noerr( status, CantInstallAppleEventHandler );
	status	= AEInstallEventHandler( kCoreEventClass, kAEPrintDocuments, NewAEEventHandlerUPP(HandleAppleEventPdoc), 0, false );	require_noerr( status, CantInstallAppleEventHandler );
	//	Note: Since RunApplicationEventLoop installs a Quit AE Handler, there is no need to do it here.

CantInstallAppleEventHandler:
	return;
}


#pragma mark -
#pragma mark ¥ CarbonEvent Handlers ¥

static	pascal	OSStatus	CommandProcessEventHandler( EventHandlerCallRef nextHandler, EventRef inEvent, void *userData )
{
	HICommand		command;
	OSStatus		status			= eventNotHandledErr;

	GetEventParameter( inEvent, kEventParamDirectObject, typeHICommand, NULL, sizeof(HICommand), NULL, &command );

	switch ( command.commandID )
	{
		case kHICommandNew:
			status  = DoNewWindow( NULL );
			break;
		
		case kHICommandPreferences:
			DisplayPreferencesWindow();
			break;
		}

	return( status );
}

static	pascal	OSStatus	MainWindowEventHandler( EventHandlerCallRef nextHandler, EventRef inEvent, void *inUserData )
{
	#pragma unused ( inCallRef )
	HICommand		command;
	OSStatus		status			= eventNotHandledErr;
	UInt32			eventKind		= GetEventKind( inEvent );
	UInt32			eventClass		= GetEventClass( inEvent );
	WindowRef		window			= (WindowRef) inUserData;

	switch ( eventClass )
	{
		case kEventClassWindow:
			//if ( eventKind == kEventWindowClose )								//  Dispose extra window storage here
			break;
			
		case kEventClassCommand:
			if ( eventKind == kEventCommandProcess )
			{
				GetEventParameter( inEvent, kEventParamDirectObject, typeHICommand, NULL, sizeof(HICommand), NULL, &command );
				if ( command.commandID == kHICommandOK )						//	OK (Execute) Button was clicked in window
				{
					HIViewRef		comboBoxView;
					CFURLRef		urlRef			= NULL;
					CFStringRef		urlString		= NULL;
					
					status	= HIViewFindBySigAndID( HIViewGetRoot(window), 'Cmbo', 0, &comboBoxView );
					status	= GetControlData( comboBoxView, kControlEditTextPart, kControlEditTextCFStringTag, sizeof( CFStringRef ), &urlString, NULL );	//	Get the current text as a CFURL from the Combo Box
					urlRef  = CFURLCreateWithString( kCFAllocatorDefault, urlString, NULL );					//	Create a CFURL from the CFString
					
					SaveComboBoxValues( comboBoxView );
					status	= DisplayImageInWindow( window, urlRef );
					
					if ( urlString != NULL )	CFRelease( urlString );
					if ( urlRef != NULL )		CFRelease( urlRef );
				}
			}
			break;
	}
    
    return( status );
}



#pragma mark -
#pragma mark ¥ Windows ¥

static	void	DisplayPreferencesWindow( void )
{																				//  Entry point for a preferences window
	WindowRef					window;
	OSErr						err;
	static	EventHandlerUPP		preferencesWindowEventHandlerUPP;
	const EventTypeSpec			windowEvents[]		=	{	{ kEventClassCommand, kEventCommandProcess }, { kEventClassWindow, kEventWindowClose }	};

	err	= CreateWindowFromNib( g.mainNibRef, CFSTR("PreferencesWindow"), &window );
	if ( err != noErr ) goto Bail;

	SetControlValueByID( window, 'PopM', 0, (g.maxHistory / 5) + 1 );			//	We use a 5X algorithm.  0, 5, 10, 15, ...
	
	if ( preferencesWindowEventHandlerUPP == NULL )	preferencesWindowEventHandlerUPP	= NewEventHandlerUPP( PreferencesWindowEventHandlerProc );
	err	= InstallWindowEventHandler( window, preferencesWindowEventHandlerUPP, GetEventTypeCount(windowEvents), windowEvents, window, NULL );
	if ( err == noErr )
	{
		ShowWindow( window );
		(void) RunAppModalLoopForWindow( window );								//	Run the preferences window as application modal
	}
Bail:
	return;
}

static	pascal	OSStatus	PreferencesWindowEventHandlerProc( EventHandlerCallRef inCallRef, EventRef inEvent, void* inUserData )
{
#pragma unused ( inCallRef )
	HICommand				command;
	CFNumberRef				cfNumber		= NULL;
	WindowRef				window			= (WindowRef) inUserData;
	UInt32					eventClass		= GetEventClass( inEvent );
	UInt32					eventKind		= GetEventKind( inEvent );

	switch ( eventClass )
	{
		case kEventClassWindow:
			if ( eventKind == kEventWindowClose )
			{
				(void) QuitAppModalLoopForWindow( window );
			}
			break;

		case kEventClassCommand:
			if ( eventKind == kEventCommandProcess )
			{
				GetEventParameter( inEvent, kEventParamDirectObject, typeHICommand, NULL, sizeof(HICommand), NULL, &command );
				if ( command.commandID == kHICommandOK )
				{
					g.maxHistory	= ( (GetControlValueByID( window, 'PopM', 0 ) - 1) * 5 );			//	We use a 5X algorithm.  0, 5, 10, 15, ...

					cfNumber	= CFNumberCreate( NULL, kCFNumberSInt32Type, &g.maxHistory );
					CFPreferencesSetAppValue( CFSTR("MaxHistory"), cfNumber, kCFPreferencesCurrentApplication );
					if ( cfNumber != NULL )	CFRelease( cfNumber );
					(void) CFPreferencesAppSynchronize( kCFPreferencesCurrentApplication );				//	Write prefs to disk!
				}
				if ( (command.commandID == kHICommandOK) || (command.commandID == kHICommandCancel) )
				{
					SendWindowCloseEvent( window );														//	Send ourselves a kEventWindowClose event
				}
			}
			break;
	}

    return( eventNotHandledErr );
}


static	OSStatus	DoNewWindow( WindowRef *outWindow )
{
	OSStatus					status;
	HIViewRef					comboBoxView;
	static	EventHandlerUPP		windowEventHandlerUPP;
	static	EventHandlerUPP		comboBoxEventHandlerUPP;
	WindowRef					window				= NULL;
	const	EventTypeSpec		windowEvents[]		=	{	{ kEventClassCommand, kEventCommandProcess }, { kEventClassWindow, kEventWindowClose }	};
	const	EventTypeSpec		comboBoxEvents[]	=	{	{ kEventClassControl, kEventControlHiliteChanged } };

	//	Create a window. "MainWindow" is the name of the window object. This name is set in	InterfaceBuilder when the nib is created.
	status	= CreateWindowFromNib( g.mainNibRef, CFSTR("MainWindow"), &window );
	require_noerr( status, CantCreateWindow );

	if ( windowEventHandlerUPP == NULL ) windowEventHandlerUPP	= NewEventHandlerUPP( MainWindowEventHandler );
	status	= InstallWindowEventHandler( window, windowEventHandlerUPP, GetEventTypeCount(windowEvents), windowEvents, window, NULL );
	require_noerr( status, CantInstallWindowEventHandler );

	status	= HIViewFindBySigAndID( HIViewGetRoot(window), 'Cmbo', 0, &comboBoxView );
	if ( comboBoxEventHandlerUPP == NULL ) comboBoxEventHandlerUPP	= NewEventHandlerUPP( ComboBoxEventHandler );
	status	= InstallControlEventHandler( comboBoxView, comboBoxEventHandlerUPP, GetEventTypeCount(comboBoxEvents), comboBoxEvents, comboBoxView, NULL );
	require_noerr( status, CantInstallWindowEventHandler );

	RestoreComboBoxValues( comboBoxView );				//	Load the saved locations into our ComboBox control list

	//	The window was created hidden so show it if the window parameter is NULL, if it's not, it will be the responsibility of the caller to show it.
	if ( outWindow == NULL )	ShowWindow( window );

	SetWindowModified( window, false );
	
CantInstallWindowEventHandler:
CantCreateWindow:
	if ( outWindow != NULL )	*outWindow	= window;
	return( status );
}

//	We modify the behavior of the combo box by making it more interactive, similar to the way the Safari address bar behaves.
//	When an item is double clicked from the combo box list it is automatically loaded.
static	pascal	OSStatus	ComboBoxEventHandler( EventHandlerCallRef inCallRef, EventRef inEvent, void *inUserData )
{
	ControlPartCode				previousPartCode;
	HIViewRef					comboBoxView		= (HIViewRef) inUserData;
	UInt32						eventClass			= GetEventClass( inEvent );
	UInt32						eventKind			= GetEventKind( inEvent );
	OSStatus					status				= eventNotHandledErr;
	
	switch ( eventClass )
	{
		case kEventClassControl:
			if ( eventKind == kEventControlHiliteChanged )
			{
				CFURLRef			urlRef			= NULL;
				CFStringRef			currentCFString	= NULL;
				CFStringRef			urlCFString		= NULL;
				
				(void) GetControlProperty( comboBoxView, 'Crnt', 'URL_', sizeof(CFStringRef), NULL, &currentCFString );	//	We store the most recent url as a control property
				(void) GetControlData( comboBoxView, kControlEditTextPart, kControlEditTextCFStringTag, sizeof( CFStringRef ), &urlCFString, NULL );
				
				if ( (currentCFString == NULL) || (CFStringCompare( currentCFString, urlCFString, 0 ) != kCFCompareEqualTo) )	//	Has the url changed?
				{
					(void) GetEventParameter( inEvent, kEventParamControlPreviousPart, typeControlPartCode, NULL, sizeof(ControlPartCode), NULL, &previousPartCode );
				
					if ( previousPartCode == kHIComboBoxDisclosurePart )												//	Tells us an item has been selected from the list
					{
						CFRetain( urlCFString );
						SetControlProperty( comboBoxView, 'Crnt', 'URL_', sizeof(CFStringRef), &urlCFString );			//	Save the url as a control property for checking later
						
						SaveComboBoxValues( comboBoxView );

						urlRef  = CFURLCreateWithString( kCFAllocatorDefault, urlCFString, NULL );
						(void) DisplayImageInWindow( GetControlOwner(comboBoxView), urlRef );							//	Display the image pointed to by the url in the window
						
						if ( urlRef != NULL )			CFRelease( urlRef );
						if ( urlCFString != NULL )		CFRelease( urlCFString );
					}
				}
			}
			break;
	}
	return( status );
}



#pragma mark -
#pragma mark ¥ Save/Open Document ¥


#pragma mark -
#pragma mark ¥ Utilities ¥

//	Utility routine to synchronously send a basic "kEventClassCommand / kEventCommandProcess" CarbonEvent to the application target
void	SendCommandProcessEvent( UInt32 commandID )
{
	HICommand				command;
	EventRef				event;

	BlockZero( &command, sizeof(command) );
	command.commandID	= commandID;
	
	(void) CreateEvent( NULL,  kEventClassCommand, kEventCommandProcess, GetCurrentEventTime(), kEventAttributeUserEvent, &event );
	(void) SetEventParameter( event, kEventParamDirectObject, typeHICommand, sizeof(command), &command );
	(void) SendEventToApplication( event );
	(void) ReleaseEvent( event );
}

//	Utility routine to synchronously send a basic "kEventClassWindow / kEventWindowClose" CarbonEvent to the window target
void	SendWindowCloseEvent( WindowRef window )
{
	EventRef				event;
	
	(void) CreateEvent( NULL,  kEventClassWindow, kEventWindowClose, GetCurrentEventTime(), kEventAttributeUserEvent, &event );
	(void) SetEventParameter( event, kEventParamDirectObject, typeWindowRef, sizeof(window), &window );
	(void) SendEventToWindow( event, window );
	(void) ReleaseEvent( event );
}

static	OSStatus	HIViewFindBySigAndID( HIViewRef inStartView, OSType signature, SInt32 id, HIViewRef *outControl )
{
	HIViewID	hiViewID = { signature, id };
	return( HIViewFindByID( inStartView, hiViewID, outControl ) );
}

static  OSStatus	GetControlBySigAndID( WindowRef window, OSType  signature, SInt32 id, ControlRef *control )
{
	ControlID	controlID	= { signature, id };
	return( GetControlByID( window, &controlID, control ) );
}

void	SetControlValueByID( WindowRef window, OSType signature, SInt32 id, SInt32 value )
{
	ControlRef		control;

	GetControlBySigAndID( window, signature, id, &control );
	if ( control == NULL ) goto Bail;

	SetControlValue( control, value );
	
	Draw1Control( control );
Bail:
	return;
}

SInt16	GetControlValueByID( WindowRef window, OSType signature, SInt32 id )
{
	ControlRef		control;

	GetControlBySigAndID( window, signature, id, &control );
	if ( control == NULL ) return( 0 );

	return( GetControlValue(control) );
}


#pragma mark -
#pragma mark ¥ Specialized Routines ¥

//	Given a URL to an image, display it in our HIImageView which is embeded in an HIScrollView.
static  OSStatus	DisplayImageInWindow( WindowRef window, CFURLRef cfURL )
{
	OSStatus					status;
	HIViewRef					hiImageView;
	ControlRef					control;
	OSType						dataRefType;
	static  ControlActionUPP	sliderControlActionUPP;
	GraphicsImportComponent		ci					= NULL;
	CGImageRef					cgImageRef			= NULL;
	Handle						dataRef				= NULL;
	
	require( (cfURL != NULL) && (window != NULL), Bail );

	//	Install a slider action proc to modify the image alpha live
	status	= GetControlBySigAndID( window, 'Sldr', 1, &control );	require_noerr( status, Bail );	
	if ( sliderControlActionUPP == NULL )		sliderControlActionUPP  = NewControlActionUPP( LiveAlphaSliderActionProc );
	SetControlAction( control, sliderControlActionUPP );

	//	We use the powerful combination of QTNewDataReferenceFromCFURL, GetGraphicsImporterForDataRef, and GraphicsImportCreateCGImage to create a CGImage
	//	From a url which points to any media type supported by QuickTime Graphics Importers.
	status	= QTNewDataReferenceFromCFURL( cfURL, 0, &dataRef, &dataRefType );												//	Create the DataRef from a URL
	status	= GetGraphicsImporterForDataRef( dataRef, dataRefType, &ci );		require( ci != NULL, Bail );				//	Get the QT GraphicsImporter for the data ref
	status	= GraphicsImportCreateCGImage( ci, &cgImageRef, kGraphicsImportCreateCGImageUsingCurrentSettings );				//	Use the GraphicsImporter to create a CGImage
	
	//	Set up the image default characteristics
	status	= HIViewFindBySigAndID( HIViewGetRoot(window), 'HIiv', 1, &hiImageView );	require_noerr( status, Bail );		//	Get the HIImageView
	status	= HIImageViewSetImage( hiImageView, cgImageRef );		require_noerr( status, Bail );							//	Set the image of the HIImageView
	status	= HIImageViewSetOpaque( hiImageView, false );			require_noerr( status, Bail );							//	Image can have transparency
	status	= HIImageViewSetAlpha( hiImageView, GetControl32BitValue(control) / 100.0 );	require_noerr( status, Bail );	//	Set the image alpha
	status	= HIImageViewSetScaleToFit( hiImageView, false );		require_noerr( status, Bail );							//	Don't scale the image, it's embeded in an HIScrollView so scaling doesn't make much sense
	status	= HIViewSetVisible( hiImageView, true );				require_noerr( status, Bail );							//	Make it visible

Bail:
	if ( ci != NULL )				CloseComponent( ci );						else	status  = -1;
	if ( dataRef != NULL )			DisposeHandle( dataRef );					else	status  = -1;
	if ( cgImageRef != NULL )		CFRelease( cgImageRef );					else	status  = -1;
	
	return( status );
}

//	ControlActionProc for slider to adjust image alpha live. ControlActionProc's are needed for live slider feedback.
static  pascal  void	LiveAlphaSliderActionProc( ControlRef control, ControlPartCode partCode )
{
	OSStatus					status;
	HIViewRef					hiImageView;
	float						alpha			= GetControl32BitValue(control) / 100.0;

	status	= HIViewFindBySigAndID( HIViewGetRoot(GetControlOwner(control)), 'HIiv', 1, &hiImageView );	require_noerr( status, Bail );
	status	= HIImageViewSetAlpha( hiImageView, alpha );								require_noerr( status, Bail );
	
Bail:
	return;
}

//	Given a combo box control, keep it sorted chronologically, and store its current list values in "HistoryEntries" appliocation preference
//	1) Moves the current combo box entry to the top of its CFArray list
//	2) Truncates array to contain no more than g.maxHistory entries
//	3) Saves a CFArray of the list in its "HistoryEntries" preference
//	4) Updates the combo box entries for display
void	SaveComboBoxValues( HIViewRef comboBoxView )
{
	OSStatus				status;
	CFArrayRef				cfArray;
	CFIndex					count;
	CFStringRef				cfString;
	CFMutableArrayRef		cfMutableArray	= NULL;
	CFStringRef				urlString		= NULL;
	
	status	= GetControlData( comboBoxView, kHIComboBoxDisclosurePart, kHIComboBoxListTag, sizeof(cfArray), (Ptr)&cfArray, NULL );			//	Get the combo box list entries
	cfMutableArray	= CFArrayCreateMutableCopy( NULL, g.maxHistory, cfArray );																//	Make a mutable copy for modification
	status	= GetControlData( comboBoxView, kControlEditTextPart, kControlEditTextCFStringTag, sizeof( CFStringRef ), &urlString, NULL );	//	Get the current entry in the text portion
	
	for ( count = CFArrayGetCount( cfMutableArray ) - 1 ; count >= 0 ; count-- )						//	If the current url is in the list, delete it, to add it to the top of the list
	{
		cfString	= (CFStringRef) CFArrayGetValueAtIndex( cfMutableArray, count );
		if ( CFStringCompare( cfString, urlString, kCFCompareCaseInsensitive ) == kCFCompareEqualTo )	//	Compare each list entry to the current text entry
			CFArrayRemoveValueAtIndex( cfMutableArray, count );											//	Remove any duplicates
	}
	for ( count = CFArrayGetCount( cfMutableArray ) - 1 ; count >= g.maxHistory - 1 ; count-- )			//	Remove extra array values beyond g.maxHistory
		CFArrayRemoveValueAtIndex( cfMutableArray, count );
	
	CFArrayInsertValueAtIndex( cfMutableArray, 0, (const void *) urlString );							//	Insert our new entry at the top of the list

	CFPreferencesSetAppValue( CFSTR("HistoryEntries"), cfMutableArray, kCFPreferencesCurrentApplication );
	(void) CFPreferencesAppSynchronize( kCFPreferencesCurrentApplication );								//	Save the modified list to our application "HistoryEntries" preferences
	
	RestoreComboBoxValues( comboBoxView );																//	Since the list now contains the text entry and modifications, refresh the combo box list

	if ( cfMutableArray != NULL )	CFRelease( cfMutableArray );
}



//	Read in the combo box preferences and populate the combo box with those "HistoryEntries" preferences.
void	RestoreComboBoxValues( HIViewRef comboBoxView )
{
	CFArrayRef				cfArray					= NULL;

	cfArray	= (CFArrayRef) CFPreferencesCopyAppValue( CFSTR("HistoryEntries"), kCFPreferencesCurrentApplication );
	if ( cfArray != NULL )
	{
		(void) SetControlData( comboBoxView, kHIComboBoxDisclosurePart, kHIComboBoxListTag, sizeof(cfArray), &cfArray );
		CFRelease( cfArray );
	}
}








