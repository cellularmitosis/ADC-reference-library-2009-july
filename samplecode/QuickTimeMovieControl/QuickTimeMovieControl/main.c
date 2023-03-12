/*
	File:		main.c

	Abstract:	A skeleton of modern nib-based and Carbon Events-based Carbon application.
				QuickTimeMovieControl demonstrates how to easily add QuickTime movie playback
				to an application.  The meat of the sample is the use of the API CreateMovieControl()
				within the routine DisplayMovieInWindow().
				With little code, using the "Open URL" menu choice, this sample is able to play 
				networked QuickTime media such as streaming mp3s, movies, and more.  Or you can 
				choose to open a file locally for playback.  The NavServices filter should make 
				only QuickTime media selectable by taking advantage of CanQuickTimeOpenFile().

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

	Copyright © 2004 Apple Computer, Inc., All Rights Reserved
*/

#include	<Carbon/Carbon.h>
#include	<QuickTime/QuickTime.h>

#define	kDistanceFromBottomOfWindow	16			//	We move the movie control up 16 pixels so the window resize thumb doesn't cover the control, this space can also be used to display status, etc.

struct  GlobalAppInfo							//  Application globals
{
	IBNibRef				mainNibRef;
};
typedef struct GlobalAppInfo GlobalAppInfo;

static	void		InstallAppleEventHandlers( void );
static	OSStatus	DoNewWindow( WindowRef *window );
static	void		DoPreferences( void );
static  OSStatus	DoSomething();
static	OSStatus	NavOpenDocument();
static	pascal	Boolean NavOpenFilterProc( AEDesc *theItem, void *info, NavCallBackUserData callBackUD, NavFilterModes filterMode );
CFStringRef	GetControlCFString( WindowRef window, OSType signature, SInt32 id );
static	pascal	OSStatus	URLWindowEventHandlerProc( EventHandlerCallRef inCallRef, EventRef inEvent, void* inUserData );
void	SendWindowCloseEvent( WindowRef window );
OSStatus	SetWindowProxyFSRef( WindowRef window, FSRef *fsRef );
OSStatus	GetControlBySigAndID( WindowRef window, OSType  signature, SInt32 id, ControlRef *control );
static  OSStatus	DisplayMovieInWindow( WindowRef window, FSRef *fsRef, CFStringRef urlCFString );
static	pascal	OSStatus	ParentMovieControlWindowEventHandler( EventHandlerCallRef inCallRef, EventRef inEvent, void* inUserData );
static	pascal	OSStatus	MovieControlEventHandler( EventHandlerCallRef inCallRef, EventRef inEvent, void* inUserData );
static	OSStatus	OpenDocuments( AEDescList docList );		//  Add call to DisplayMovieInWindow()
static	pascal	OSStatus	CommandProcessEventHandler( EventHandlerCallRef nextHandler, EventRef inEvent, void *userData );	//  Change kHICommandNew to behave same as kHICommandOpen
int	main( int argc, char *argv[] );								//  Added call to EnterMovies()

GlobalAppInfo		g;


#pragma mark -
#pragma mark ¥ Main ¥

int	main( int argc, char *argv[] )
{
	OSStatus				status;
	long					response;
	const   EventTypeSpec   commandProcessEvents[]		= { { kEventClassCommand, kEventCommandProcess } };

	BlockZero( &g, sizeof(g) );											//	Initialize our globals

	status	= Gestalt( gestaltSystemVersion, &response );
	if ( ! ((status == noErr) && (response >= 0x00001030)) )			//	We require Mac OS X 10.3 or greater to run
	{
		DialogRef	alertDialog;
		CreateStandardAlert( kAlertStopAlert, CFSTR("Mac OS X 10.3 (minimum) is required for this application"), NULL, NULL, &alertDialog );
		RunStandardAlert( alertDialog, NULL, NULL );
		ExitToShell();
	}

	status	= EnterMovies();											//	Initialize QuickTime
	require_noerr( status, CantInitializeQuickTime );

	//	Create a Nib reference passing the name of the nib file (without the .nib extension) CreateNibReference only searches into the application bundle.
	status	= CreateNibReference( CFSTR("main"), &g.mainNibRef );
	require_noerr( status, CantGetNibRef );

	//	Once the nib reference is created, set the menu bar. "MainMenu" is the name of the menu bar object. This name is set in InterfaceBuilder when the nib is created.
	status	= SetMenuBarFromNib( g.mainNibRef, CFSTR("MenuBar") );
	require_noerr( status, CantSetMenuBar );

	EnableMenuCommand( NULL, kHICommandPreferences );					//	Enable Preferences menu item

	InstallAppleEventHandlers();
	InstallApplicationEventHandler( NewEventHandlerUPP(CommandProcessEventHandler), GetEventTypeCount(commandProcessEvents), commandProcessEvents, NULL, NULL );

	RunApplicationEventLoop();											//	Call the event loop
	
CantSetMenuBar:
CantGetNibRef:
CantInitializeQuickTime:
	return( status );
}


#pragma mark -
#pragma mark ¥ AppleEvent Handlers ¥

static	pascal	OSErr	HandleAppleEventOapp( const AppleEvent *theAppleEvent, AppleEvent *reply, long handlerRefcon )
{
	return( noErr );
}

static	pascal	OSErr	HandleAppleEventRapp( const AppleEvent *theAppleEvent, AppleEvent *reply, long handlerRefcon )
{
	return( noErr );
}

static	pascal	OSErr	HandleAppleEventOdoc( const AppleEvent *theAppleEvent, AppleEvent *reply, long handlerRefcon )
{
	AEDescList		docList;
	OSErr			err			= AEGetParamDesc( theAppleEvent, keyDirectObject, typeAEList, &docList );
	require_noerr( err, CantGetDocList );

	err	= OpenDocuments( docList );
	AEDisposeDesc( &docList );

CantGetDocList:
  	return( err );
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
	HICommand				command;
	WindowRef				window;
	static EventHandlerUPP	urlWindowEventHandlerUPP;
	OSStatus				status					= eventNotHandledErr;
	const EventTypeSpec		windowEvents[]			= {	{ kEventClassCommand, kEventCommandProcess }	};

	GetEventParameter( inEvent, kEventParamDirectObject, typeHICommand, NULL, sizeof(HICommand), NULL, &command );

	switch ( command.commandID )
	{
		case kHICommandNew:
			//status  = DoNewWindow( NULL );									//  We don't want to create a new window without an image in it
			//break;															//  So, we'll just fall through to the kHICommandOpen case
		
		case kHICommandOpen:
			status	= NavOpenDocument();
			break;
			
		case kHICommandPreferences:
			DoPreferences();
			break;

		case kHICommandRevert:													//  Place holders, not handled yet
		case kHICommandPageSetup:
		case kHICommandPrint:
			break;
		
		case 'NURL':															//  Open from Network URL
			status	= CreateWindowFromNib( g.mainNibRef, CFSTR("URLWindow"), &window );
			if ( urlWindowEventHandlerUPP == NULL )	urlWindowEventHandlerUPP	= NewEventHandlerUPP( URLWindowEventHandlerProc );
			status	= InstallWindowEventHandler( window, urlWindowEventHandlerUPP, GetEventTypeCount(windowEvents), windowEvents, window, NULL );
			ShowWindow( window );

			break;

		case 'DoIt':															//  Our own menu to hook in and Do Something...
			status  = DoSomething();
			break;
		}

	return( status );
}

static	pascal	OSStatus	URLWindowEventHandlerProc( EventHandlerCallRef inCallRef, EventRef inEvent, void* inUserData )
{
	#pragma unused ( inCallRef )
	HICommand		command;
	CFStringRef		urlCFString;
	OSStatus		status			= eventNotHandledErr;
	UInt32			eventKind		= GetEventKind( inEvent );
	UInt32			eventClass		= GetEventClass( inEvent );
	WindowRef		window			= (WindowRef) inUserData;

	switch ( eventClass )
	{
		case kEventClassCommand:
			if ( eventKind == kEventCommandProcess )
			{
				GetEventParameter( inEvent, kEventParamDirectObject, typeHICommand, NULL, sizeof(HICommand), NULL, &command );
				if ( command.commandID == kHICommandOK )							//	OK Button was clicked in window
				{
					urlCFString	= GetControlCFString( window, 'ETxt', 0 );
					
					SendWindowCloseEvent( window );									//	Close this URL window

					status	= DoNewWindow( &window );								//	Create a new (Hidden) window
					status  = DisplayMovieInWindow( window, NULL, urlCFString );
					SetWindowTitleWithCFString( window, urlCFString );			//	Set the Window title
					
					if ( status == noErr )	ShowWindow( window );
					else					DisposeWindow( window );

					if ( urlCFString != NULL )	CFRelease( urlCFString );
				}
				else if ( command.commandID == kHICommandCancel )
				{
					SendWindowCloseEvent( window );									//	Close this URL window
				}
			}
			break;
	}
	return( status );
}


#pragma mark -
#pragma mark ¥ Windows ¥

static	void	DoPreferences( void )
{																					//  Some very basic CFPreferences code would be good here
	DialogRef		alertDialog;
	CreateStandardAlert( kAlertStopAlert, CFSTR("No Preferences yet!"), NULL, NULL, &alertDialog );
	RunStandardAlert( alertDialog, NULL, NULL );
}


static	OSStatus	DoNewWindow( WindowRef *outWindow )
{
	OSStatus					status;
	WindowRef					window				= NULL;

	status	= CreateWindowFromNib( g.mainNibRef, CFSTR("MainWindow"), &window );	//	Create a window. "MainWindow" is the name of the window object. This name is set in	InterfaceBuilder when the nib is created.
	require_noerr( status, CantCreateWindow );

	if ( outWindow == NULL )	ShowWindow( window );								//	The window was created hidden so show it if the window parameter is NULL, if it's not, it will be the responsibility of the caller to show it.

	SetWindowModified( window, false );
	
CantCreateWindow:
	if ( outWindow != NULL )	*outWindow	= window;
	return( status );
}


#pragma mark -
#pragma mark ¥ Save/Open Document ¥

static	OSStatus	NavOpenDocument()
{
	OSStatus					status;
	NavDialogCreationOptions	options;
	NavReplyRecord				navReply;
	static	NavObjectFilterUPP	navFilterUPP;
	NavDialogRef				navDialog		= NULL;

	status	= NavGetDefaultDialogCreationOptions( &options );
	require_noerr( status, CantGetDefaultOptions );

	if ( navFilterUPP == NULL )	navFilterUPP = NewNavObjectFilterUPP( NavOpenFilterProc );		//	Filter only documents we can open

	status	= NavCreateChooseFileDialog( &options, NULL, NULL, NULL, navFilterUPP, NULL, &navDialog );
	require_noerr( status, CantCreateDialog );
	
	status	= NavDialogRun( navDialog );
	require_noerr( status, CantRunDialog );
	
	status	= NavDialogGetReply( navDialog, &navReply );
	require( (status == noErr) || (status == userCanceledErr), CantGetReply );

	if ( navReply.validRecord )	status	= OpenDocuments( navReply.selection );
	else						status	= userCanceledErr;

	NavDisposeReply( &navReply );

CantGetReply:
CantRunDialog:
	if ( navDialog != NULL )	NavDialogDispose( navDialog );
CantCreateDialog:
CantGetDefaultOptions:
	return( status );
}


//	We use CanQuickTimeOpenFile to determine which files are displayed in ther NavServices open file dialog.
static	pascal	Boolean	NavOpenFilterProc( AEDesc *theItem, void *info, NavCallBackUserData callBackUD, NavFilterModes filterMode )
{
	FSRef					fsRef;
	OSStatus				status;
	FSSpec					fsSpec;
	FSCatalogInfo			fsCatalogInfo;
	Boolean					outCanOpenAsMovie;
	Boolean					canViewItem			= false;
	
	if ( theItem->descriptorType == typeFSRef )
	{
		if ( ((NavFileOrFolderInfo*)info)->isFolder == true )	return( true );
		
		status	= AEGetDescData( theItem, &fsRef, sizeof(fsRef) );
		require_noerr( status, CantGetFSRef );
		
		status	= FSGetCatalogInfo( &fsRef, kFSCatInfoFinderInfo, &fsCatalogInfo, NULL, &fsSpec, NULL );
		require_noerr( status, CantGetFSRef );

		status	= CanQuickTimeOpenFile( &fsSpec, ((FileInfo *)&fsCatalogInfo.finderInfo)->fileType, 0, NULL, &outCanOpenAsMovie, NULL, kQTDontUseDataToFindImporter | kQTAllowOpeningStillImagesAsMovies | kQTAllowAggressiveImporters );
		if ( (status == noErr) && (outCanOpenAsMovie==true) )	canViewItem	= true;
	}

CantGetFSRef:
	return( canViewItem );
}

static	OSStatus	OpenDocuments( AEDescList docList )
{
	long				index;
	FSRef				fsRef;
	CFStringRef			fileName;
	WindowRef			window			= NULL;
	long				count			= 0;
	OSStatus			status			= AECountItems( &docList, &count );
	require_noerr( status, CantGetCount );

	for( index = 1; index <= count; index++ )
	{
		if ( (status = AEGetNthPtr( &docList, index, typeFSRef, NULL, NULL, &fsRef, sizeof(FSRef), NULL) ) == noErr )
		{
			status	= DoNewWindow( &window );										//	Create a new (Hidden) window
			require_noerr( status, CantCreateWindow );

			status	= LSCopyDisplayNameForRef( &fsRef, &fileName );					//	Get the name of the file the Finder displays (may not include file extension)
			require_noerr( status, CantGetName );
			SetWindowTitleWithCFString( window, fileName );							//	Set the Window title
			
			status  = DisplayMovieInWindow( window, &fsRef, NULL );
			if ( status == noErr )	ShowWindow( window );												//	The window was created hidden so show it
			else					DisposeWindow( window );

		}
	}
	
CantGetName:
CantCreateWindow:
CantGetCount:
	return( status );
}

#pragma mark -
#pragma mark ¥ Utilities ¥

OSStatus	GetControlBySigAndID( WindowRef window, OSType  signature, SInt32 id, ControlRef *control )
{
	ControlID	controlID;
	
	controlID.id		= id;
	controlID.signature	= signature;

	return( GetControlByID( window, &controlID, control ) );
}

CFStringRef	GetControlCFString( WindowRef window, OSType signature, SInt32 id )
{
	ControlRef		control;
	SInt32			dataSize;
	OSStatus		err;
	CFStringRef		cfString		= NULL;
	
	GetControlBySigAndID( window, signature, id, &control );
	if ( control == NULL ) goto Bail;
	
	err	= GetControlData( control, 0, kControlStaticTextCFStringTag, sizeof(CFStringRef), (Ptr)&cfString, &dataSize );
	if ( err != noErr ) goto Bail;
	
	return( cfString );
	
Bail:
	if ( cfString != NULL ) CFRelease( cfString );
	return( NULL );
}


OSStatus	SetWindowProxyFSRef( WindowRef window, FSRef *fsRef )
{
	FSSpec		fsSpec;
	OSStatus	status	= fnfErr;

	require( (fsRef != NULL), Bail );
	
	status	= FSGetCatalogInfo( fsRef, kFSCatInfoNone, NULL, NULL, &fsSpec, NULL );
	require_noerr( status, Bail );
	status	= SetWindowProxyFSSpec( window, &fsSpec );
	
Bail:
	return( status );
}

void	SendWindowCloseEvent( WindowRef window )
{
	EventRef				event;
	
	(void) CreateEvent( NULL,  kEventClassWindow, kEventWindowClose, GetCurrentEventTime(), kEventAttributeUserEvent, &event );
	(void) SetEventParameter( event, kEventParamDirectObject, typeWindowRef, sizeof(window), &window );
	(void) SendEventToWindow( event, window );
	(void) ReleaseEvent( event );
}


#pragma mark -
#pragma mark ¥ Specialized Routines ¥

static  OSStatus	DoSomething()
{
	DebugStr( "\pDoSomething Was Called\n" );
	return( noErr );
}


static  OSStatus	DisplayMovieInWindow( WindowRef window, FSRef *fsRef, CFStringRef urlCFString )
{
	OSStatus					status;
	ControlRef					control;
	Rect						rect;
	Movie						movie;
	OSType						dataRefType;
	Handle						dataRef				= NULL;
	
	require( ((fsRef != NULL) || (urlCFString != NULL)) && (window != NULL), Bail );

	
	if ( urlCFString != NULL )	status	= QTNewDataReferenceFromURLCFString( urlCFString, 0, &dataRef, &dataRefType );
	else						status  = QTNewDataReferenceFromFSRef( fsRef, 0, &dataRef, &dataRefType );		//  http://developer.apple.com/documentation/QuickTime/WhatsNewQT6_4/Chap1/chapter_1_section_6.html
	require_noerr( status, Bail );
	status  = NewMovieFromDataRef( &movie, newMovieActive, 0, dataRef, dataRefType );
	require_noerr( status, Bail );
	
	GetMovieBox( movie, &rect );													//	Get the natural bounds of the movie
	if ( rect.right - rect.left < 150 )	rect.right	= rect.left + 150;				//	Just in case the rect is too small
	if ( rect.bottom - rect.top < (80 + kDistanceFromBottomOfWindow) )	rect.bottom	= rect.top + (80 + kDistanceFromBottomOfWindow);	//	Grow it to 80 X 150
	SizeWindow( window, rect.right - rect.left, rect.bottom - rect.top + kDistanceFromBottomOfWindow, true );		//	Start off the window the size the movie was meant for
	
	status  = CreateMovieControl( window, &rect, movie, kMovieControlOptionSetKeysEnabled | kMovieControlOptionLocateTopLeft | kMovieControlOptionEnableEditing | kMovieControlOptionHandleEditingHI, &control );   //  http://developer.apple.com/documentation/QuickTime/QT6WhatsNew/Chap1/chapter_1_section_25.html

	//  Enabling this code will grow the Movie control with the window.
	//  The trick is in growing the control in a flicker-free way. We do this by installing the event handler for the kEventControlBoundsChanged
	//  event on the parent control.  This is how control redrawing should be done.  You may be tempted to install an event on the window target
	//  for the kEventWindowBoundsChanging event, but this does cause flicker as the parent control (window) is erased and then the movie frame is drawn.
	//	In addition we get notified when the Control is disposed via the kEventControlDispose event at which time we stop the move from playing.  This
	//	isn't neccesary, but without it the buffers will play through even though the Control was disposed.
	if ( true )
	{
		ControlRef					parentControl;
		static	EventHandlerUPP		parentControlEventHandlerUPP;
		static	EventHandlerUPP		movieControlEventHandlerUPP;
		const	EventTypeSpec		parentControlEvents[]		=	{	{ kEventClassControl, kEventControlBoundsChanged  } };
		const	EventTypeSpec		movieControlEvents[]		=	{	{ kEventClassControl, kEventControlDispose  } };

		SizeControl( control, rect.right - rect.left, rect.bottom - rect.top );
		
		GetSuperControl( control, &parentControl );
		if ( parentControlEventHandlerUPP == NULL ) parentControlEventHandlerUPP	= NewEventHandlerUPP( ParentMovieControlWindowEventHandler );
		status	= InstallControlEventHandler( parentControl, parentControlEventHandlerUPP, GetEventTypeCount(parentControlEvents), parentControlEvents, control, NULL );

		if ( movieControlEventHandlerUPP == NULL ) movieControlEventHandlerUPP	= NewEventHandlerUPP( MovieControlEventHandler );
		status	= InstallControlEventHandler( control, movieControlEventHandlerUPP, GetEventTypeCount(movieControlEvents), movieControlEvents, control, NULL );
	}

	SetWindowProxyFSRef( window, fsRef );

Bail:	
	if ( dataRef != NULL )	DisposeHandle( dataRef );
	return( status );
}

//	Resize the movie control when the controls parent is resized, in this case the windows root control.
static	pascal	OSStatus	ParentMovieControlWindowEventHandler( EventHandlerCallRef inCallRef, EventRef inEvent, void* inUserData )
{
	Rect						rect;
	ControlRef					control			= (ControlRef) inUserData;
	UInt32						eventClass		= GetEventClass( inEvent );
	UInt32						eventKind		= GetEventKind( inEvent );
	OSStatus					status			= eventNotHandledErr;
	
	switch ( eventClass )
	{
		case kEventClassControl:
			if ( eventKind == kEventControlBoundsChanged )
			{
				(void) GetEventParameter( inEvent, kEventParamCurrentBounds, typeQDRectangle, NULL, sizeof(Rect), NULL, &rect );
				rect.bottom	-= kDistanceFromBottomOfWindow;
				SizeControl( control, rect.right - rect.left, rect.bottom - rect.top );
			}
			break;
	}
	return( status );
}

//	Call StopMovie() when the control is disposed.  Without calling StopMovie, the audio may continue to play for one or 
//	two seconds after the control is disposed.
static	pascal	OSStatus	MovieControlEventHandler( EventHandlerCallRef inCallRef, EventRef inEvent, void *inUserData )
{
	OSStatus					err;
	SInt32						dataSize;
	Movie						movie;
	ControlRef					control			= (ControlRef) inUserData;
	UInt32						eventClass		= GetEventClass( inEvent );
	UInt32						eventKind		= GetEventKind( inEvent );
	OSStatus					status			= eventNotHandledErr;
	
	switch ( eventClass )
	{
		case kEventClassControl:
			if ( eventKind == kEventControlDispose )
			{
				err	= GetControlData( control, 0, kMovieControlDataMovie, sizeof(Movie), (Ptr)&movie, &dataSize );	//	kMovieControlDataMovie tag allows us to get the Movie from the Control
				if ( err == noErr )
				{
					CallNextEventHandler( inCallRef, inEvent );
					DisposeMovie( movie );
					status	= noErr;								//	We handled the event
				}
			}
			break;
	}
	return( status );
}












