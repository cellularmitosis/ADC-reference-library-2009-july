/*
	File:		main.c

	Abstract:	A skeleton of modern nib-based and Carbon Events-based Carbon application.
				AppleScriptRunner contains a folder within its application bundle, "AppleScripts"
				which contains a number of AppleScripts.  AppleScriptRunner builds a popup menu
				populated with the AppleScript names.  When "Execute" is clicked, the selected
				AppleScript is executed passing it one Text parameter, the contents of out HITextView.
				In this example all AppleScripts must take one, and only one, text parameter. An
				advisable method of adding functionallity to an application to send email is to 
				create an AppleScript to send email, and then envoke the script from the application.
				In this example "Entourage" and "Mail" each send email, provided the email clients are
				correctly configured.  The format of the HITextView to send mail should be:
				"Santa@NorthPole.com Dear Santa, My chimney ..."
				Because this sample requires only one input parameter, the AppleScrips themselves parse
				the "To" email address out as the first word, and assume the rest is the message body.
				The main entry to the "interesting" portion of the code is the routine: RunAppleScript().
				
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
#include	<sys/param.h>

struct  GlobalAppInfo							//  Application globals
{
	IBNibRef				mainNibRef;
};
typedef struct GlobalAppInfo GlobalAppInfo;

static	void		SendCommandProcessEvent( UInt32 commandID );
static  OSStatus	GetControlBySigAndID( WindowRef window, OSType  signature, SInt32 id, ControlRef *control );
static	void		InstallAppleEventHandlers( void );
static	void		DoPreferences( void );
static  OSStatus	DoSomething();
static	OSStatus	NavOpenDocument();
static	pascal		Boolean NavOpenFilterProc( AEDesc *theItem, void *info, NavCallBackUserData callBackUD, NavFilterModes filterMode );
static	OSStatus	HIViewFindBySigAndID( HIViewRef inStartView, OSType signature, SInt32 id, HIViewRef *outControl );
static	TXNObject	GetTXNObjectBySigAndID( WindowRef window, OSType signature, SInt32 id );
static	void		Wprintf( WindowRef window, char const *fmt, ... );
static  OSStatus	DisplayTextFileInWindow( WindowRef window, FSRef *fsRef );
static	pascal  OSStatus	CommandProcessEventHandler( EventHandlerCallRef nextHandler, EventRef inEvent, void *userData );	//  Added kHICommandRevert, kHICommandPageSetup, and kHICommandPrint
static	OSStatus	OpenDocuments( AEDescList docList );	//  Open Text files
static  OSStatus	RunAppleScript( FSRef *scriptFSRef, char *textParameter );
static  OSStatus	GetAppleScriptsFolderFSRef( FSRef *fsRef);
static  OSStatus	CreateMessageEvent( AppleEvent *theEvent, char *parameter );
static	pascal	OSStatus	MainWindowEventHandler( EventHandlerCallRef nextHandler, EventRef inEvent, void *userData );
static	OSStatus	DoNewWindow( WindowRef *window );		//  Install Event Handler for Text view


GlobalAppInfo		g;


#pragma mark -
#pragma mark ¥ Main ¥

int	main( int argc, char *argv[] )
{
	OSStatus				status;
	long					response;
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
	HICommand		command;
	UInt32			secs;
	DateTimeRec		date;
	OSStatus		status			= eventNotHandledErr;
	TXNObject		txnObject		= GetTXNObjectBySigAndID( GetFrontWindowOfClass(kDocumentWindowClass, true), 'HITv', 0 );

	GetEventParameter( inEvent, kEventParamDirectObject, typeHICommand, NULL, sizeof(HICommand), NULL, &command );

	switch ( command.commandID )
	{
		case kHICommandNew:
			status  = DoNewWindow( NULL );
			GetDateTime( &secs );												//  Demonstrate how to use utility routine Wprintf
			SecondsToDate( secs, &date );
			Wprintf( GetFrontWindowOfClass(kDocumentWindowClass, true), "Todays Date: %d/%d/%d\n", date.month, date.day, date.year );
			break;
		
		case kHICommandOpen:
			status	= NavOpenDocument();
			break;
			
		case kHICommandPreferences:
			DoPreferences();
			break;

		case kHICommandRevert:
			if ( txnObject != NULL ) status = TXNRevert( txnObject );
			break;
		
		case kHICommandPageSetup:
			if ( txnObject != NULL ) status = TXNPageSetup( txnObject );
			status  = noErr;
			break;
		
		case kHICommandPrint:
			if ( txnObject != NULL ) status = TXNPrint( txnObject );
			status  = noErr;
			break;
		
		case 'DoIt':															//  Our own menu to hook in and Do Something...
			status  = DoSomething();
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
					FSRef						parentFSRef;
					FSRef						fsRef;
					TXNObject					txnObject;
					CFStringRef					cfString;
					HFSUniStr255				appleScriptName;
					ControlRef					control;
					SInt32						controlValue;
					MenuRef						menuRef;
					Handle						txnDataHandle		= NULL;
					
					//  Get the text to pass as a parameter to the AppleScript
					txnObject	= GetTXNObjectBySigAndID( window, 'HITv', 0 );								//	Get the TXNObject
					status		= TXNGetDataEncoded( txnObject, kTXNStartOffset, kTXNEndOffset, &txnDataHandle, kTXNTextData );		//	Retrieve the text. kTXNTextData specifies Text, not Unicode or Rich Text

					//  Find the name of the chosen menu item, which is the name of our AppleScript to run
					status  = GetControlBySigAndID( window, 'PopM', 0, &control );							//	Get the popup menu control
					controlValue	= GetControl32BitValue( control );										//	Get the control value, which is the selected menu item
					menuRef	= GetControlPopupMenuHandle( control );											//	Get the MenuRef from the control
					CopyMenuItemTextAsCFString( menuRef, controlValue, &cfString );							//	Copy the selected item text into a CFString
					
					//	Convert our CFString to an HFSUniStr255 and create an FSRef to our chosen AppleScript
					appleScriptName.length = CFStringGetBytes( cfString, CFRangeMake(0, MIN(CFStringGetLength(cfString), 255)), kCFStringEncodingUnicode, 0, false, (UInt8 *)(appleScriptName.unicode), 255, NULL );
					status	= GetAppleScriptsFolderFSRef( &parentFSRef );									//	Get FSRef to AppleScripts folder
					status	= FSMakeFSRefUnicode( &parentFSRef, appleScriptName.length, appleScriptName.unicode, kTextEncodingUnknown, &fsRef );   //	FSMakeFSRefUnicode to make FSRef to the selected AppleScript

					//	Run the AppleScript passing in our text as a parameter
					status  = RunAppleScript( &fsRef, *txnDataHandle );
					
					//	Release what we have allocated
					if ( cfString != NULL )	CFRelease( cfString );
					if ( txnDataHandle != NULL )	DisposeHandle( txnDataHandle );
				}
			}
			break;
	}
    
    return( status );
}

#pragma mark -
#pragma mark ¥ Windows ¥

static	void	DoPreferences( void )
{																				//  Entry point for a preferences window
	DialogRef		theAlert;
	CreateStandardAlert( kAlertStopAlert, CFSTR("No Preferences yet!"), NULL, NULL, &theAlert );
	RunStandardAlert( theAlert, NULL, NULL );
}


static	OSStatus	DoNewWindow( WindowRef *outWindow )
{
	OSStatus					status;
	static	EventHandlerUPP		windowEventHandlerUPP;
	const	EventTypeSpec		windowEvents[]		=	{	{ kEventClassCommand, kEventCommandProcess }, { kEventClassWindow, kEventWindowClose }	};
	WindowRef					window				= NULL;

	//	Create a window. "MainWindow" is the name of the window object. This name is set in	InterfaceBuilder when the nib is created.
	status	= CreateWindowFromNib( g.mainNibRef, CFSTR("MainWindow"), &window );
	require_noerr( status, CantCreateWindow );

	if ( windowEventHandlerUPP == NULL ) windowEventHandlerUPP	= NewEventHandlerUPP( MainWindowEventHandler );
	status	= InstallWindowEventHandler( window, windowEventHandlerUPP, GetEventTypeCount(windowEvents), windowEvents, window, NULL );
	require_noerr( status, CantInstallWindowEventHandler );

	{
		FSIterator					iterator;
		ItemCount					actualObjects;
		FSRef						fsRefs[32];											//	Allow up to 32 FSRefs, in this case 32 AppleScripts
		FSRef						fsRef;
		CFStringRef					cfString;
		ControlRef					control;
		int							i;
		MenuRef						menuRef;

		status  = GetAppleScriptsFolderFSRef( &fsRef );									//	Locate our AppleScripts folder within our application bundle
		status  = FSOpenIterator( &fsRef, kFSIterateFlat, &iterator );					//	Read the first 32 items of the "AppleScripts" folder using FSOpenIterator-FSGetCatalogInfoBulk-FSCloseIterator
		require_noerr( status, FileSystemError );
		status  = FSGetCatalogInfoBulk( iterator, 32, &actualObjects, NULL, 0, NULL, fsRefs, NULL, NULL );
		status  = FSCloseIterator( iterator );
		if ( actualObjects > 0 )														//	If we found some items
		{
			status  = GetControlBySigAndID( window, 'PopM', 0, &control );				//	Get the popup menu control
			menuRef	= GetControlPopupMenuHandle( control );								//	Get the MenuRef from the control
			SetControl32BitMaximum( control, actualObjects );							//	Need to SetControl32BitMaximum for popups to function as expected
			DeleteMenuItem( menuRef, 1 );												//	Delete the first menuItem titled: "No AppleScripts Found"
			for ( i = 0 ; i < actualObjects ; i++ )										//	Itterate over the files we found
			{
				status	= LSCopyDisplayNameForRef( &fsRefs[i], &cfString );				//	Get the name of the file the Finder displays (may not include file extension)
				CFShow( cfString );
				
				if ( (status == noErr) && ( ( CFStringHasPrefix( cfString, CFSTR(".") ) ) == false ) )  //  Don't add ".DS_Store", or any other file starting with a "."
					AppendMenuItemTextWithCFString( menuRef, cfString, 0, 0, NULL );	//	Append the CFString of the AppleScript to out popup menu
			}
		}
	}

	//	The window was created hidden so show it if the window parameter is NULL, if it's not, it will be the responsibility of the caller to show it.
	if ( outWindow == NULL )	ShowWindow( window );

	SetWindowModified( window, false );
	
FileSystemError:
CantInstallWindowEventHandler:
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


//	Generic Navigation Services filter proc described in TechNote 2017, "Using Launch Services"
//	This filter querries LaunchServices to ask what types of documents this application can open.
//	Openable document types are defined within the applications info.plist file.
static	pascal	Boolean	NavOpenFilterProc( AEDesc *theItem, void *info, NavCallBackUserData callBackUD, NavFilterModes filterMode )
{
	LSItemInfoRecord		lsInfoRec;
	FSRef					fsRef;
	OSStatus				status;
	static	Boolean			applicationFSRefInitialized;
	static	FSRef			applicationFSRef;
	Boolean					canViewItem				= false;
	
	if ( theItem->descriptorType == typeFSRef )
	{
		status	= AEGetDescData( theItem, &fsRef, sizeof(fsRef) );
		require_noerr( status, CantGetFSRef );
		
		status	= LSCopyItemInfoForRef( &fsRef, kLSRequestAllInfo, &lsInfoRec );	//	Ask LaunchServices for information about the item
		require( (status == noErr) || (status == kLSApplicationNotFoundErr), LaunchServicesError );
		
		if ( applicationFSRefInitialized == false )									//  First time this routine ir run, we locate our application bundle
		{
			ProcessSerialNumber		psn			= { 0, kCurrentProcess };
			GetProcessBundleLocation( &psn, &applicationFSRef );					//  Save the reference in a static
			applicationFSRefInitialized	= true;
		}
		
		if ( (lsInfoRec.flags & kLSItemInfoIsContainer) != 0 )						//  If it's a folder, make it selectable
			canViewItem	= true;
		else
        	status	= LSCanRefAcceptItem( &fsRef, &applicationFSRef, kLSRolesViewer, kLSAcceptDefault, &canViewItem );		//  Can this application "view" this file?
	}

LaunchServicesError:
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
			
			status  = DisplayTextFileInWindow( window, &fsRef );
			if ( status == noErr )
				ShowWindow( window );												//	The window was created hidden so show it
			else
				DisposeWindow( window );
		}
	}
	
CantGetName:
CantCreateWindow:
CantGetCount:
	return( status );
}

#pragma mark -
#pragma mark ¥ Utilities ¥

//	Utility routine to synchronously send a basic "kEventClassCommand / kEventCommandProcess" CarbonEvent to the application target
static  void	SendCommandProcessEvent( UInt32 commandID )
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


static  OSStatus	GetControlBySigAndID( WindowRef window, OSType  signature, SInt32 id, ControlRef *control )
{
	ControlID	controlID	= { signature, id };
	return( GetControlByID( window, &controlID, control ) );
}

static	OSStatus	HIViewFindBySigAndID( HIViewRef inStartView, OSType signature, SInt32 id, HIViewRef *outControl )
{
	HIViewID	hiViewID	= { signature, id };
	return( HIViewFindByID( inStartView, hiViewID, outControl ) );
}


static	TXNObject	GetTXNObjectBySigAndID( WindowRef window, OSType signature, SInt32 id )
{
	HIViewRef		hiTextView;
	TXNObject		txnObject		= NULL;
	
	if ( window != NULL )
		if ( HIViewFindBySigAndID( HIViewGetRoot(window), signature, id, &hiTextView ) == noErr )
			if ( hiTextView != NULL )
				txnObject	= HITextViewGetTXNObject( hiTextView );
	
	return( txnObject );
}

/*****************************************************
*
* ExecuteCompiledAppleScriptEvent( AEDesc *scriptData, AppleEvent *theEvent, AEDesc *resultData )
*
* Purpose:  Generic routine to execute our AppleScriptEvent, passing parameters to an
*			AppleScript running inside my application
*
* Notes:	http://developer.apple.com/qa/qa2001/qa1111.html
*
* Inputs:   scriptData		- Reference to the AppleScript to be executed
*			theEvent		- text parameter to our AppleScript as an AppleEvent
*			resultData		- result from script 
*
* Returns:  OSStatus		- error code (0 == no error) 
*/
OSStatus	ExecuteCompiledAppleScriptEvent( AEDesc *scriptData, AppleEvent *theEvent, AEDesc *resultData )
{
	OSStatus			err;
	ComponentInstance	theComponent	= NULL;
	OSAID				contextID		= kOSANullScript;
	OSAID				resultID		= kOSANullScript;

	theComponent	= OpenDefaultComponent( kOSAComponentType, typeAppleScript );	//	Open the scripting component
	if ( theComponent == NULL )	{ err = paramErr; goto Bail; }

	err	= OSALoad( theComponent, scriptData, kOSAModeNull, &contextID );			//	Compile the script into a new context
	require_noerr( err, Bail );

	err	= OSAExecuteEvent( theComponent, theEvent, contextID, kOSAModeNull, &resultID );	//	Run the script

	if ( resultData != NULL )														//	Collect the results - if any
	{
		AECreateDesc( typeNull, NULL, 0, resultData );
		if ( err == errOSAScriptError )
			OSAScriptError( theComponent, kOSAErrorMessage, typeChar, resultData );
		else if ( (err == noErr) && (resultID != kOSANullScript) )
			OSADisplay(theComponent, resultID, typeChar, kOSAModeNull, resultData);
	}

Bail:
	if ( contextID != kOSANullScript )	OSADispose( theComponent, contextID );
	if ( resultID != kOSANullScript )	OSADispose( theComponent, resultID );
	if ( theComponent != NULL )			CloseComponent( theComponent );
	return( err );
}

#pragma mark -
#pragma mark ¥ Specialized Routines ¥

static  OSStatus	DoSomething()
{
	DebugStr( "\pDoSomething Was Called\n" );
	return( noErr );
}

//	This utility routine takes printf style arguments and puts the resulting text in the HITextView ( 'HITv', 0 ) within the passed in window.
static	void	Wprintf( WindowRef window, char const *fmt, ... )
{
	char			s[4096];
	va_list			args;
	TXNObject		txnObject;
	
	if ( window == NULL )	return;
	
	va_start( args, fmt );
	(void) vsprintf( s, fmt, args );
	va_end( args );
	
	txnObject	= GetTXNObjectBySigAndID( window, 'HITv', 0 );
	TXNSetData( txnObject, kTXNTextData, s, strlen(s), kTXNUseCurrentSelection, kTXNUseCurrentSelection );
	return;
}

//	Return the FSRef to the AppleScripts directory within our bundle
static  OSStatus	GetAppleScriptsFolderFSRef( FSRef *fsRef)
{
	Boolean						successful			= false;
	CFURLRef					urlRef1				= NULL;
	CFURLRef					urlRef2				= NULL;
	
	urlRef1 = CFBundleCopyResourcesDirectoryURL( CFBundleGetMainBundle() );									//	Get URL to our bundles "Resources" directory
	require( urlRef1 != NULL , Bail );
	urlRef2 = CFURLCreateCopyAppendingPathComponent( NULL, urlRef1, CFSTR("AppleScripts"), true );			//	Append "AppleScripts" onto the URL, location of our AppleScripts
	require( urlRef1 != NULL , Bail );
	successful  = CFURLGetFSRef( urlRef2, fsRef );															//	Convert the CFURL into an FSRef to be returned
	if ( urlRef1 != NULL )  CFRelease( urlRef1 );
	if ( urlRef2 != NULL )  CFRelease( urlRef2 );

Bail:	
	return( successful == false );
}


static  OSStatus	DisplayTextFileInWindow( WindowRef window, FSRef *fsRef )
{
	OSStatus					status;
	TXNObject					txnObject;
	FSSpec						fsSpec;
	CFURLRef					cfURL				= NULL;
	
	require( (fsRef != NULL) && (window != NULL), Bail );

	txnObject   = GetTXNObjectBySigAndID( window, 'HITv', 0 );
	require( txnObject != NULL, CantGetTXNObject );
	
	cfURL	= CFURLCreateFromFSRef( NULL, fsRef );											//  Create a CFURL from the passed in FSRef
	require( cfURL != NULL, CantCreateURL );
	status	= TXNSetDataFromCFURLRef( txnObject, cfURL, kTXNStartOffset, kTXNEndOffset );	//	url will be stored by MLTE
	require_noerr( status, CantSetTXN );

	status	= FSGetCatalogInfo( fsRef, kFSCatInfoNone, NULL, NULL, &fsSpec, NULL );			//  Need the FSSpec, since SetWindowProxy requires it
	require_noerr( status, CantGetFSSpec );
	SetWindowProxyFSSpec( window, &fsSpec );
	
	// The following lines deal with current shortcomings of the HITextView
	TXNRevert( txnObject );																	//  The change count is not set to 0 as it should,  a call to TXNRevert workarounds the problem
	TXNSetSelection( txnObject, kTXNEndOffset, kTXNEndOffset );
	TXNShowSelection( txnObject, true );													//  The HIScrollView scroll bars don't reflect the current size of the HITextView, a couple of calls to Set/Show selection workaround the problem
	TXNSetSelection( txnObject, kTXNStartOffset, kTXNStartOffset );
	TXNShowSelection( txnObject, true );													//  The HIScrollView scroll bars don't reflect the current size of the HITextView, a couple of calls to Set/Show selection workaround the problem

CantGetFSSpec:
CantSetTXN:
CantCreateURL:
	if ( cfURL != NULL )		CFRelease( cfURL );
CantGetTXNObject:
Bail:
	return( status );
}


/*****************************************************
*
* RunAppleScript( FSRef *scriptFSRef, char *textParameter )
*
* Purpose:  Runs an AppleScript with one text parameter as input.
*			CreateMessageEvent, and therefore RunAppleScript, assumes the AppleScript has a
*			subroutine entry titled "applescriptentry" and accepts one TEXT parameter.
*
* Inputs:   scriptFSRef		- FSRef to our AppleScript
*			textParameter	- text parameter to our AppleScript 
*
* Returns:  OSStatus		- error code (0 == no error) 
*/
static  OSStatus	RunAppleScript( FSRef *scriptFSRef, char *textParameter )
{
	OSStatus		err;
	AppleEvent		aeParameter;
	AEDesc			scriptData;
	short			refNum;
	FSCatalogInfo   catalogInfo;
	Handle			h				= NULL;
	
	refNum  = FSOpenResFile( scriptFSRef, fsRdPerm );									//  Older (Mac OS 8/9) scripts store their data in the 'scpt' (1) resource
	if ( refNum != -1 )
	{
		h		= Get1IndResource( 'scpt', 1 );
		if( h != NULL )	DetachResource( h );											//	Detach the handle before closing the resource
		CloseResFile( refNum );
	}
	if ( h == NULL )
	{
		err = FSGetCatalogInfo( scriptFSRef, kFSCatInfoDataSizes, &catalogInfo, NULL, NULL, NULL ); //  Get the size of the script
		require_noerr( err, Bail );

		err = FSOpenFork( scriptFSRef, 0, NULL, fsRdPerm, &refNum );					//  Open the data fork read only
		require_noerr( err, Bail );

		h	= NewHandle( catalogInfo.dataLogicalSize );
		err	= FSReadFork( refNum, fsFromStart, 0, catalogInfo.dataLogicalSize, *h, NULL );	//  Read the script into our handle
		(void) FSCloseFork( refNum );													//	Close the file reference
	}

	err	= CreateMessageEvent( &aeParameter, textParameter );								//  Create the AppleEvent, and use the Apple event to call the script's subroutine
	require_noerr( err, Bail );

	err = AECreateDesc( typeOSAGenericStorage, *h, GetHandleSize(h), &scriptData );		//	Load the compiled script into an AEDesc of type typeOSAGenericStorage
	require_noerr( err, Bail );
                        
	err	= ExecuteCompiledAppleScriptEvent( &scriptData, &aeParameter, NULL );			//  "Generic" routine to execute our AppleScript

Bail:
	if ( h != NULL ) DisposeHandle( h );
	return( err );
}


//	http://developer.apple.com/qa/qa2001/qa1111.html
//	Creates an AppleEvent with one text parameter.  We leave it up to the AppleScript
//	to further parse the text parameter into potentially more parameters.
static  OSStatus	CreateMessageEvent( AppleEvent *theEvent, char *parameter )
{
    OSStatus				err;
    ProcessSerialNumber 	psn		= {0, kCurrentProcess};

	err	= AEBuildAppleEvent( 'ascr', kASSubroutineEvent, typeProcessSerialNumber, (Ptr) &psn, sizeof(psn), kAutoGenerateReturnID, kAnyTransactionID,
        theEvent,
        NULL,
        "'----':[TEXT(@)],"				//	One TEXT pointer parameter
        "'snam':TEXT(@)",				//	The keyASSubroutineName ('snam') parameter must contain the name of the subroutine that is being called with every letter converted to lowercase. For example, if name of the subroutine in your script is "GetDocumentSize", then the string provided in the keyASSubroutineName parameter should be "getdocumentsize".
        parameter, "applescriptentry");	//  The entry routine whithin the AppleScript

    return( err );
}



