/*
	File:		main.c

	Abstract:	iTunesControllermakes use of the RegisterEventHotKey API to retrieve and process
				hotkeys, then calling through to an AppleScript to control iTunes.
				RegisterEventHotKey  allows the application to get hotkey events while in the
				background or foregroud.  The PostHotKeyKeyboardEvent function demonstrates how to
				turn off hotkeys, post keys with CGPostKeyboardEvent, and reenable them to prevent
				a circular dependency.

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

	Copyright © 2006 Apple Computer, Inc., All Rights Reserved
*/


#include <Carbon/Carbon.h>
#include <sys/sysctl.h>


static	WindowRef	DisplayITunesControlWindow();
OSStatus	DrawTextHandler(EventHandlerCallRef nextHandler, EventRef theEvent, void* userData);
void	PeriodicTimerAction (EventLoopTimerRef  theTimer, void* userData);
Boolean	FindiTunes();
Boolean	RestorePrefs( WindowRef window );
static	void	SavePrefs( WindowRef window );
static	OSStatus	ITunesControlWindowEventHandlerProc( EventHandlerCallRef inCallRef, EventRef inEvent, void *inUserData );
static	OSStatus	HotKeyEventHandlerProc( EventHandlerCallRef inCallRef, EventRef inEvent, void* inUserData );
static	OSStatus	ApplicationEventHandlerProc( EventHandlerCallRef inCallRef, EventRef inEvent, void *inUserData );
void	DisplayAboutBox();
void	MakeMeTheFrontProcess();
void	RegisterHotKeys( Boolean registerKeys );
void	PostHotKeyKeyboardEvent( CGKeyCode virtualKey );
void	SetControlCString( WindowRef window, OSType signature, SInt32 id, char *cString );
OSStatus	GetControlBySigAndID( WindowRef window, OSType  signature, SInt32 id, ControlRef *control );


struct GlobalAppInfo	
{
	CFBundleRef				mainBundle;
    IBNibRef				nibRef;
	CFMutableStringRef		titleString;
	CFMutableStringRef		artistAlbumString;
};
typedef struct GlobalAppInfo GlobalAppInfo;

GlobalAppInfo		g;


int	main( int argc, char* argv[] )
{
    WindowRef				window;
	HIViewRef				hiViewRef;
    OSStatus				err;
    const	EventTypeSpec	applicationEvents[]	= {	{ kEventClassApplication, kEventAppFrontSwitched }, { kEventClassCommand, kEventCommandProcess }	};
    const	EventTypeSpec	hotKeyEvents[]		= { { kEventClassKeyboard, kEventHotKeyPressed }, { kEventClassKeyboard, kEventHotKeyReleased }	};

    g.titleString		= CFStringCreateMutable( kCFAllocatorDefault, 0 );			//	Initialize globals
    g.artistAlbumString	= CFStringCreateMutable( kCFAllocatorDefault, 0 );
	g.mainBundle		= CFBundleGetMainBundle();

    err	= CreateNibReference( CFSTR("main"), &g.nibRef );					if ( err != noErr )	goto Bail;
    err	= SetMenuBarFromNib( g.nibRef, CFSTR("MenuBar") );					if ( err != noErr )	goto Bail;
    
    window	= DisplayITunesControlWindow();									if ( window == NULL )	goto Bail;
    
	//	Install a timer, PeriodicTimerAction(), which fires 2 times a second to get latest song, album, artist, information from iTunes
	GetControlBySigAndID( window, 'hivw', 100, &hiViewRef);
    InstallEventLoopTimer( GetMainEventLoop(), 0,  kEventDurationSecond / 2, NewEventLoopTimerUPP(PeriodicTimerAction), hiViewRef, NULL );
	
    InstallApplicationEventHandler( NewEventHandlerUPP(HotKeyEventHandlerProc), GetEventTypeCount(hotKeyEvents), hotKeyEvents, 0, NULL );
    InstallApplicationEventHandler( NewEventHandlerUPP(ApplicationEventHandlerProc), GetEventTypeCount(applicationEvents), applicationEvents, 0, NULL );

	RegisterHotKeys( true );														//	Register to receive our HotKeys
		
    RunApplicationEventLoop();

    SavePrefs( GetFrontWindowOfClass( kUtilityWindowClass, true ) );				//	Save window preferences on quit

Bail:
	return( 0 );
}


//	Creates and displays our floating iTunes status window.
static	WindowRef	DisplayITunesControlWindow()
{
	HIViewRef				hiViewRef;
	WindowRef				window;
    OSStatus				err;
    HISize					minWindowSize	= { 192, 32 };
    const   EventTypeSpec	controlEvents[]	= { { kEventClassControl, kEventControlDraw }, { kEventClassControl, kEventControlClick } };
    const   EventTypeSpec	windowEvents[]	= { { kEventClassWindow, kEventWindowClose } };
                                                    
    err	= CreateWindowFromNib( g.nibRef, CFSTR("MainWindow"), &window );	if ( err != noErr )	return( NULL );

    (void) SetWindowResizeLimits( window, &minWindowSize, NULL );					//	Set the minimum allowable size of the window
    err	= InstallWindowEventHandler( window, NewEventHandlerUPP( ITunesControlWindowEventHandlerProc ), GetEventTypeCount(windowEvents), windowEvents, window, NULL );

	GetControlBySigAndID( window, 'hivw', 100, &hiViewRef);
    (void) InstallControlEventHandler( hiViewRef, NewEventHandlerUPP(DrawTextHandler), GetEventTypeCount(controlEvents), controlEvents, hiViewRef, NULL );

	if ( RestorePrefs( window ) == false )	DisplayAboutBox();						//	If run the first time, show the About Box first for instructions

	ShowWindow( window );
	
	return( window );
}


static	OSStatus	ApplicationEventHandlerProc( EventHandlerCallRef inCallRef, EventRef inEvent, void *inUserData )
{
	#pragma unused ( inCallRef, inUserData )
	WindowRef					window;
	HICommand					command;
	OSStatus 					err					= eventNotHandledErr;
	UInt32						eventClass			= GetEventClass( inEvent );
	UInt32						eventKind			= GetEventKind(inEvent);
	
	if ( eventClass == kEventClassApplication )
	{
		//	If iTunes is the frontmost application, unregister our hotkeys since they match iTunes.  Otherwise we would generate double events
		if ( eventKind == kEventAppFrontSwitched )
		{
			ProcessSerialNumber		psn;
			ProcessInfoRec			processInfo	= { 0 };
			processInfo.processInfoLength		= sizeof( processInfo );
			GetFrontProcess( &psn );
			GetProcessInformation( &psn, &processInfo );
			if ( processInfo.processSignature == 'hook' )	RegisterHotKeys( false );			//	Unregister our hotkeys, iTunes is frontmost
			else											RegisterHotKeys( true );			//	Make sure our hotkeys are registered
		}
	}
	else if ( eventClass == kEventClassCommand )
	{
		GetEventParameter( inEvent, kEventParamDirectObject, typeHICommand, NULL, sizeof(HICommand), NULL, &command );
		if ( command.commandID == kHICommandNew )
		{
			window	= GetFrontWindowOfClass( kUtilityWindowClass, false );
			if ( window != NULL )	ShowWindow(window);											//	New simply shows the window, a close will hide it, never disposing the window
			err	= noErr;
		}
		else if ( command.commandID == kHICommandAbout )
		{
			DisplayAboutBox();
			err	= noErr;
		}
	}

    return( err );
}


void	DisplayAboutBox()
{
	OSErr					err;
	NumVersion				version;
	WindowRef				window;
	Str255					s;
    IBNibRef				nibRef;
	
    err	= CreateNibReference( CFSTR("main"), &nibRef );					if ( err != noErr )	goto Bail;
 	err	= CreateWindowFromNib( nibRef, CFSTR("AboutBox"), &window );	if ( err != noErr )	goto Bail;
    DisposeNibReference( nibRef );	

	*((UInt32*)&version)	= CFBundleGetVersionNumber( g.mainBundle );							//	Display the version number in our About Box
	if ( (version.minorAndBugRev & 0x0F) != 0 )	sprintf( (char*)s, "%d.%d.%d", version.majorRev, version.minorAndBugRev >> 4, version.minorAndBugRev & 0x0F );
	else										sprintf( (char*)s, "%d.%d", version.majorRev, version.minorAndBugRev >> 4 );
	SetControlCString( window, 'STxt', 1, (char*)s );

	ShowWindow( window );

Bail:
	return;
}


//	RegisterHotKeys is responsible for registering for ther hotkeys we are interested in, as well as unregistering for those keys.
void	RegisterHotKeys( Boolean registerKeys )
{
	static	Boolean				hotKeysRegistered;												//	Static to remember if HotKeys are registered
	static	EventHotKeyRef		eventHotKeyRef[5];

	if ( registerKeys == true )
	{
		if ( hotKeysRegistered == false )
		{
			EventHotKeyID			rightArrowID		= { 'Arow', 1 };						//	rightArrowID is defined as our EventHotKeyID 'Arow' 1
			EventHotKeyID			leftArrowID			= { 'Arow', 2 };
			EventHotKeyID			upArrowID			= { 'Arow', 3 };
			EventHotKeyID			downArrowID			= { 'Arow', 4 };
			EventHotKeyID			numberPadZeroID		= { 'Arow', 5 };
			RegisterEventHotKey( 124, cmdKey, rightArrowID, GetApplicationEventTarget(), 0, &eventHotKeyRef[0] );	//	<command><right arrow>, right arrow has a virtual key code of 124
			RegisterEventHotKey( 123, cmdKey, leftArrowID, GetApplicationEventTarget(), 0, &eventHotKeyRef[1] );
			RegisterEventHotKey( 126, cmdKey, upArrowID, GetApplicationEventTarget(), 0, &eventHotKeyRef[2] );
			RegisterEventHotKey( 125, cmdKey, downArrowID, GetApplicationEventTarget(), 0, &eventHotKeyRef[3] );
			RegisterEventHotKey( 82, cmdKey, numberPadZeroID, GetApplicationEventTarget(), 0, &eventHotKeyRef[4] );
			hotKeysRegistered	= true;
		}
	}
	else if ( registerKeys == false )
	{
		if ( hotKeysRegistered == true )
		{
			UnregisterEventHotKey( eventHotKeyRef[0] );											//	Unregister our hot keys
			UnregisterEventHotKey( eventHotKeyRef[1] );
			UnregisterEventHotKey( eventHotKeyRef[2] );
			UnregisterEventHotKey( eventHotKeyRef[3] );
			UnregisterEventHotKey( eventHotKeyRef[4] );
			hotKeysRegistered	= false;
		}
	}
	
	return;
}


//	We execute our AppleScript code to communicate with iTunes via the command line tool osascript.
#define NEXT_TRACK "osascript -e 'tell application \"iTunes\"' -e 'next track' -e 'end tell'"
#define BACK_TRACK "osascript -e 'tell application \"iTunes\"' -e 'back track' -e 'end tell'"			//	back track - go to beginning of current track, or previous track if at beginning of current.
#define PREVIOUS_TRACK "osascript -e 'tell application \"iTunes\"' -e 'previous track' -e 'end tell'"	//	previous track - start playing previous track
#define PAUSE "osascript -e 'tell application \"iTunes\"' -e 'pause' -e 'end tell'"
#define PLAY "osascript -e 'tell application \"iTunes\"' -e 'play' -e 'end tell'"
#define PLAYPAUSE "osascript -e 'tell application \"iTunes\"' -e 'playpause' -e 'end tell'"				//	playpause - toggle play/pause of current track, a.k.a. space bar
#define VOLUME_UP "osascript -e 'tell application \"iTunes\"' -e 'set currentVolume to sound volume' -e 'if currentVolume is greater than 90 then' -e 'set currentVolume to 100' -e 'end if' -e 'set sound volume to \(currentVolume + 10\)' -e 'end tell'"
#define VOLUME_DOWN "osascript -e 'tell application \"iTunes\"' -e 'set currentVolume to sound volume' -e 'if currentVolume is less than 10 then' -e 'set currentVolume to 0' -e 'end if' -e 'set sound volume to \(currentVolume - 10\)' -e 'end tell'"

static	OSStatus	HotKeyEventHandlerProc( EventHandlerCallRef inCallRef, EventRef inEvent, void* inUserData )
{
	#pragma unused ( inCallRef, inUserData )
	EventHotKeyID			hotKeyID;
	OSStatus				err			= eventNotHandledErr;
	UInt32					eventClass	= GetEventClass( inEvent );
	UInt32					eventKind	= GetEventKind( inEvent );
	
	switch ( eventClass )
	{
		case kEventClassKeyboard:
			if ( eventKind == kEventHotKeyPressed )
			{
				GetEventParameter( inEvent, kEventParamDirectObject, typeEventHotKeyID, NULL, sizeof(EventHotKeyID), NULL, &hotKeyID );
				if ( hotKeyID.signature == 'Arow' )
				{
					switch ( hotKeyID.id )
					{						
						case 1:																	//	1-5 values defined in RegisterHotKeys(), 'Arow' (1), etc.
							system( NEXT_TRACK );												//	system() executes command line arguments, executable from the Terminal
							PostHotKeyKeyboardEvent( (CGKeyCode) 124 );							//	124 = virtual key code for the "right arrow" also used in RegisterHotKeys()
							break;
						case 2:
							system( BACK_TRACK );
							PostHotKeyKeyboardEvent( (CGKeyCode) 123 );
							break;
						case 3:
							system( VOLUME_UP );
							PostHotKeyKeyboardEvent( (CGKeyCode) 126 );
							break;
						case 4:
							system( VOLUME_DOWN );
							PostHotKeyKeyboardEvent( (CGKeyCode) 125 );
							break;
						case 5:
							system( PLAYPAUSE );
							PostHotKeyKeyboardEvent( (CGKeyCode) 82 );
							break;
					}
				}
			}
			break;
	}		

	return( err );
}

//	Let's say we register for <command>-P events.  If we're the background application we *may* want to forward those events onto the frontmost
//	application, so that they can process the event and print.  To do this, we have to first unregister for the event, post the event using
//	CGPostKeyboardEvent, and then reregister.  If we don't unregister first, we will end up posting the event, and catching it ourselves again and again.
void	PostHotKeyKeyboardEvent( CGKeyCode virtualKey )
{
	RegisterHotKeys( false );										//	Unregister the hot keys, so that we don't catch our own posted key events
	CGPostKeyboardEvent( (CGCharCode)0, (CGKeyCode)55, true );		//	command down
	CGPostKeyboardEvent( (CGCharCode)0, virtualKey, true );			//	key down
	CGPostKeyboardEvent( (CGCharCode)0, virtualKey, false );		//	key up
	CGPostKeyboardEvent( (CGCharCode)0, (CGKeyCode)55, false );		//	command up
	RegisterHotKeys( true );										//	Re-register for the hotkeys
}


OSStatus DrawTextHandler( EventHandlerCallRef nextHandler, EventRef theEvent, void* userData )
{
    HIRect					windowBounds;
    HIRect					titleBounds;
    HIRect					artistAlbumBounds;
    CGContextRef			context;
    OSStatus				err					= noErr;
    float					titleHeight			= 0;
    UInt32					eventKind			= GetEventKind(theEvent);
	HIViewRef				hiViewRef			= (HIViewRef) userData;
	HIThemeTextInfo			titleTextInfo		= { 0, kThemeStateActive, kThemeSmallEmphasizedSystemFont, kHIThemeTextHorizontalFlushCenter, kHIThemeTextVerticalFlushCenter, kHIThemeTextBoxOptionStronglyVertical, kHIThemeTextTruncationNone, 0, false };
	HIThemeTextInfo			artistAlbumTextInfo	= { 0, kThemeStateActive, kThemeSmallSystemFont, kHIThemeTextHorizontalFlushCenter, kHIThemeTextVerticalFlushTop, kHIThemeTextBoxOptionStronglyVertical, kHIThemeTextTruncationNone, 0, false };

    switch ( eventKind )
    {
		case kEventControlDraw:    
			GetEventParameter( theEvent, kEventParamCGContextRef, typeCGContextRef, NULL, sizeof(context), NULL, &context );
			err	= HIViewGetBounds ( hiViewRef, &windowBounds );
			
			err	= HIViewGetBounds ( hiViewRef, &titleBounds );
			err	= HIThemeGetTextDimensions( g.titleString, 0, &titleTextInfo, NULL, &titleHeight, NULL );
			titleBounds.size.height = titleHeight + 4;

			err		= HIViewGetBounds ( hiViewRef, &artistAlbumBounds );
			artistAlbumBounds.origin.y	= titleBounds.size.height;
			artistAlbumBounds.size.height	= windowBounds.size.height - titleBounds.size.height;

			if ( g.titleString )		HIThemeDrawTextBox( g.titleString, &titleBounds, &titleTextInfo, context, kHIThemeOrientationNormal );
			if ( g.artistAlbumString )	HIThemeDrawTextBox( g.artistAlbumString, &artistAlbumBounds, &artistAlbumTextInfo, context, kHIThemeOrientationNormal );
			break;

		case kEventControlClick:    
			MakeMeTheFrontProcess();							//	Bring the application to the front when our floating window is clicked
			break;
			
		default:
			break;
    }
	
    return( err );
}

void	PeriodicTimerAction( EventLoopTimerRef theTimer, void* userData )
{
    CFMutableStringRef		artistAlbumReplacement;
    CFMutableStringRef		tempAlbum;
    FILE					*osascript_pipe;
    char					fileBuffer[4096];
	CFArrayRef				stringArray;
	CFStringRef				playerState;
	CFURLRef				appleScriptURL;
	CFStringRef				appleScriptPath;
	CFStringRef				bufferString		= NULL;
	HIViewRef				hiViewRef			= (HIViewRef) userData;
    static	char			theCommand[4096]	= "osascript ";
    size_t					bytesRead			= 0;
    
    if ( FindiTunes() == false )
    {
        CFStringReplaceAll( g.titleString, CFSTR("iTunes not running...") );
        CFStringReplaceAll( g.artistAlbumString, CFSTR("") );
        HIViewSetNeedsDisplay( hiViewRef, true );
        return;
    }

	if ( strlen( theCommand ) < 12 ) 
	{
		appleScriptURL		= CFBundleCopyResourceURL( g.mainBundle, CFSTR("GetiTunesInfo.applescript"), NULL, NULL );	if ( appleScriptURL == NULL )	goto Bail;
		appleScriptPath		= CFURLCopyPath( appleScriptURL );
		strcat( theCommand, CFStringGetCStringPtr( appleScriptPath, CFStringGetSystemEncoding() ) );
		CFRelease( appleScriptPath );	CFRelease( appleScriptURL );
	}
    
	//	popen executes command line arguments, and receives stdout stream in "osascript_pipe"
	osascript_pipe	= popen( theCommand, "r" );		if ( osascript_pipe == NULL )	goto Bail;
    
    bytesRead	= fread( fileBuffer, sizeof(char), sizeof(fileBuffer), osascript_pipe );	//	Read the output of the popen() command into fileBuffer
    fileBuffer[bytesRead-1] = '\0';
    
    pclose( osascript_pipe );																//	Close the pipe opened by popen
    
    //	Parse strings out of returned AppleScript data
    bufferString	= CFStringCreateWithCString( kCFAllocatorDefault, fileBuffer, CFStringGetSystemEncoding());			if ( bufferString == NULL ) goto Bail;
    stringArray		= CFStringCreateArrayBySeparatingStrings( kCFAllocatorDefault, bufferString, CFSTR ("**"));			if ( stringArray == NULL ) goto Bail;
    playerState		= CFArrayGetValueAtIndex( stringArray, 0 );
    
    //	If playing, look for stream or track data
    if ( CFStringCompare( playerState, CFSTR("playing"), kCFCompareCaseInsensitive ) == 0 )
    {
        // Retrieve the rest of the strings so everything will get released correctly
        CFStringRef streamTitle	= CFArrayGetValueAtIndex(stringArray, 1);
        CFStringRef trackTitle	= CFArrayGetValueAtIndex(stringArray, 2);
        CFStringRef trackArtist	= CFArrayGetValueAtIndex(stringArray, 3);
        CFStringRef trackAlbum	= CFArrayGetValueAtIndex(stringArray, 4);

        // If no stream title, we're playing a local track
        if ( CFStringCompare( streamTitle, CFSTR("missing value"), kCFCompareCaseInsensitive ) == 0 )
        {
            CFStringReplaceAll( g.titleString, trackTitle);

            artistAlbumReplacement	= CFStringCreateMutableCopy( kCFAllocatorDefault, 0,  trackArtist );
            CFStringAppend( artistAlbumReplacement, CFSTR("\n") );
            tempAlbum	= CFStringCreateMutableCopy(kCFAllocatorDefault, 0,  trackAlbum );
            CFStringAppend( tempAlbum, CFSTR("\"") );
            CFStringInsert( tempAlbum, 0, CFSTR("\"") );
            CFStringAppend( artistAlbumReplacement, tempAlbum );
            CFStringReplaceAll( g.artistAlbumString, artistAlbumReplacement );
            CFRelease( tempAlbum );
            CFRelease( artistAlbumReplacement );
        }
        else
        {
            CFStringReplaceAll( g.titleString, streamTitle );
            artistAlbumReplacement	= CFStringCreateMutableCopy(kCFAllocatorDefault, 0,  trackTitle );
            CFStringReplaceAll( g.artistAlbumString, trackTitle );
            CFRelease( artistAlbumReplacement );
        }
    }
    else if ( CFStringCompare( playerState, CFSTR("paused"), kCFCompareCaseInsensitive ) == 0 )
    {
		CFStringReplaceAll( g.titleString, CFSTR("iTunes is paused...") );
		CFStringReplaceAll( g.artistAlbumString, CFSTR("") );
    }
    else if ( CFStringCompare( playerState, CFSTR("stopped"), kCFCompareCaseInsensitive ) == 0 )
    {
		CFStringReplaceAll( g.titleString, CFSTR("iTunes is stopped...") );
		CFStringReplaceAll( g.artistAlbumString, CFSTR("") );
    }

    if ( stringArray != NULL )	CFRelease( stringArray );
    if ( bufferString != NULL )	CFRelease( bufferString );
    
    HIViewSetNeedsDisplay( hiViewRef, true );

    return;
        
Bail:
    CFStringReplaceAll( g.titleString, CFSTR("Error communicating with iTunes...") );
    CFStringReplaceAll( g.artistAlbumString, CFSTR("") );
    if ( bufferString != NULL )	CFRelease( bufferString );
    HIViewSetNeedsDisplay( hiViewRef, true );
    return;
}

//	Return "true" if iTunes is running.
Boolean FindiTunes()
{
	OSErr					err;
	ProcessInfoRec			processInfo	= { 0 };
    ProcessSerialNumber		psn			= { kNoProcess, kNoProcess };
	
	processInfo.processInfoLength		= sizeof( processInfo );

	for ( ; ; )
	{
		err	= GetNextProcess( &psn );							if ( err != noErr )	goto Bail;
		err	= GetProcessInformation( &psn, &processInfo );		if ( err != noErr )	goto Bail;
		if ( processInfo.processSignature == 'hook' )									//	iTunes Signature
			return( true );
	}
	
Bail:
	return( false );
}



static	OSStatus	ITunesControlWindowEventHandlerProc( EventHandlerCallRef inCallRef, EventRef inEvent, void *inUserData )
{
    #pragma unused ( inCallRef )
    OSStatus		err				= eventNotHandledErr;
    WindowRef		window			= (WindowRef) inUserData;
    UInt32			eventClass		= GetEventClass( inEvent );
    UInt32			eventKind		= GetEventKind(inEvent);
    
    switch ( eventClass )
    {			
		case kEventClassWindow:
			if ( eventKind == kEventWindowClose )
			{
				HideWindow( window );													//	Rather than dispose the window, we just hide it, and return noErr
				err	= noErr;
			}
			break;
    }

    return( err );
}


void	SavePrefs( WindowRef window )
{
    OSStatus			err;
    Rect				windowBounds;
    CFDataRef			windowBoundsDataRef;

	if ( window == NULL )	goto Bail;
	
    err	= GetWindowBounds( window, kWindowStructureRgn, &windowBounds );		if ( err != noErr )	goto Bail;

	windowBoundsDataRef	= CFDataCreate( kCFAllocatorDefault, (UInt8 *)&windowBounds, sizeof(Rect) );
	CFPreferencesSetAppValue( CFSTR("WindowSize"), windowBoundsDataRef, kCFPreferencesCurrentApplication );		//	Save the window bounds as CFData
	CFPreferencesAppSynchronize( kCFPreferencesCurrentApplication );
	CFRelease( windowBoundsDataRef );

Bail:
	return;
}

Boolean	RestorePrefs( WindowRef window )
{
    CFDataRef			windowBoundsDataRef;
    Rect				windowBounds;

    windowBoundsDataRef	= CFPreferencesCopyAppValue( CFSTR("WindowSize"), kCFPreferencesCurrentApplication );
    if ( windowBoundsDataRef == NULL )	goto Bail;

	CFDataGetBytes( windowBoundsDataRef, CFRangeMake( 0, CFDataGetLength(windowBoundsDataRef) ), (UInt8 *)&windowBounds );
	SetWindowBounds( window, kWindowStructureRgn, &windowBounds );												//	Restore the window bounds
	CFRelease( windowBoundsDataRef );
	return( true );

Bail:
	return( false );
}

OSStatus	GetControlBySigAndID( WindowRef window, OSType  signature, SInt32 id, ControlRef *control )
{
	ControlID	controlID	= { signature, id };
	
	return( GetControlByID( window, &controlID, control ) );
}


void	SetControlCString( WindowRef window, OSType signature, SInt32 id, char *cString )
{
	ControlRef		control;

	GetControlBySigAndID( window, signature, id, &control );
	if ( control == NULL ) goto Bail;
	
	(void) SetControlData( control, 0, kControlStaticTextTextTag, strlen(cString), cString );
	
Bail:
	return;
}

//	Bring THIS process to the front
void	MakeMeTheFrontProcess()
{
	ProcessSerialNumber	psn;
	OSErr				err;
	
	err	= GetCurrentProcess( &psn );
	if ( err == noErr )
		(void) SetFrontProcess( &psn );
}


