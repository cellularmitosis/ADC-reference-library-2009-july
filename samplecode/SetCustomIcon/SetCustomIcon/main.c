/*
	File:		main.c

	Abstract:	A skeleton of modern nib-based and Carbon Events-based Carbon application.
				SetCustomIcon demonstrates how to programmatically set a custom icon to either
				a file or folder.  A typical example may be to create an icon representation of
				a photo.  This example uses QuickTime's graphics importers to draw the selected
				image onscreen, and also in the process of converting the image to a PicHandle and
				eventually to a IconFamilyHandle.  The utility routine SaveCustomIcon() is
				responsible for saving the IconFamilyHandle data as a resource in the appropriate
				format.

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
#include	<sys/param.h>

struct  GlobalAppInfo											//  Application globals
{
	IBNibRef				mainNibRef;
};
typedef struct GlobalAppInfo GlobalAppInfo;

struct  CustomIconWindowInfo									//	Additional information to be stored with each window
{
	WindowRef					window;
	GraphicsImportComponent		graphicsImportComponent;		//	QuickTime Graphics importer to draw image
};
typedef struct CustomIconWindowInfo CustomIconWindowInfo;

int	main( int argc, char *argv[] );								//  Added call to EnterMovies()
static	void		InstallAppleEventHandlers( void );
static	OSStatus	DoNewWindow( WindowRef *window );
static	void		DoPreferences( void );
static	OSStatus	NavOpenDocument( OSType command );
static	pascal	Boolean NavOpenFilterProc( AEDesc *theItem, void *info, NavCallBackUserData callBackUD, NavFilterModes filterMode );
OSStatus	GetControlBySigAndID( WindowRef window, OSType  signature, SInt32 id, ControlRef *control );
static	OSStatus	OpenDocument( AEDescList docList, OSType command );
static	pascal	OSStatus	CommandProcessEventHandler( EventHandlerCallRef nextHandler, EventRef inEvent, void *userData );	//  Change kHICommandNew to behave same as kHICommandOpen
static	pascal	OSStatus	CreateCustomIconWindowEventHandler( EventHandlerCallRef inCallRef, EventRef inEvent, void* inUserData );
static  void	SendCommandProcessEvent( UInt32 commandID );
static  OSStatus	DisplayImageInWindow( WindowRef window, FSRef *fsRef );
void	ScaleInRect( Rect *boundsRect, Rect *origRect, Rect *destRect );
void	SetControlCString( WindowRef window, OSType signature, SInt32 id, char *cString );
static  OSStatus	DisplayCustomIconDestinationPath( WindowRef window, FSRef *fsRef );
char	*GetControlCString( WindowRef window, OSType signature, SInt32 id, char *cString );
OSErr	CreateCustomIconFromGraphicsComponent( GraphicsImportComponent graphicsImportComponent, FSSpec *targetSpec );
OSStatus	SetCustomIcon( CustomIconWindowInfo *w );
PicHandle	CreatePictHandleFromGraphicsComponent( GraphicsImportComponent graphicsImportComponent );
OSErr	SaveCustomIcon( FSSpec *targetSpec, IconFamilyHandle icnsH );
static	pascal	OSStatus	UserPaneControlHandler( EventHandlerCallRef inCallRef, EventRef inEvent, void* inUserData );
OSErr	SendFinderAppleEvent( AliasHandle aliasH, AEEventID appleEventID );

GlobalAppInfo		g;


#pragma mark -
#pragma mark ¥ Main ¥

int	main( int argc, char *argv[] )
{
	OSStatus				status;
	const   EventTypeSpec   commandProcessEvents[]		= { { kEventClassCommand, kEventCommandProcess } };

	BlockZero( &g, sizeof(g) );											//	Initialize our globals

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
	SendCommandProcessEvent( kHICommandNew );							// Instantiate an empty window at the beginning so the User sees something
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

	err	= OpenDocument( docList, 'GImg' );
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
	OSStatus				status					= eventNotHandledErr;

	GetEventParameter( inEvent, kEventParamDirectObject, typeHICommand, NULL, sizeof(HICommand), NULL, &command );

	switch ( command.commandID )
	{
		case kHICommandNew:
			status  = DoNewWindow( NULL );
			break;
		
		case kHICommandOpen:
			status	= NavOpenDocument( 'GImg' );								//	File->Open assumes the user wants to select a source image
			break;
			
		case kHICommandPreferences:
			DoPreferences();
			break;

		case kHICommandRevert:													//  Place holders, not handled yet
		case kHICommandPageSetup:
		case kHICommandPrint:
			break;
		}

	return( status );
}


#pragma mark -
#pragma mark ¥ Windows ¥

static	void	DoPreferences( void )
{
	DialogRef		alertDialog;
	CreateStandardAlert( kAlertStopAlert, CFSTR("No Preferences yet!"), NULL, NULL, &alertDialog );
	RunStandardAlert( alertDialog, NULL, NULL );
}


//	Create a new window, install Event Handlers, initialize window storage 
static	OSStatus	DoNewWindow( WindowRef *outWindow )
{
	OSStatus					status;
	CustomIconWindowInfo		*w;
	Rect						r;
	static	EventHandlerUPP		createCustomIconWindowEventHandlerUPP;
	HISize						minWindowSize;			//	width, height
	const EventTypeSpec			windowEvents[]		= {	{ kEventClassCommand, kEventCommandProcess }, { kEventClassWindow, kEventWindowClose }	};

	w   = (CustomIconWindowInfo*) NewPtrClear( sizeof(CustomIconWindowInfo) );
	status	= CreateWindowFromNib( g.mainNibRef, CFSTR("MainWindow"), &w->window );	//	Create a window. "MainWindow" is the name of the window object. This name is set in	InterfaceBuilder when the nib is created.
	require_noerr( status, CantCreateWindow );
	SetWRefCon( w->window, (long) w );

	if ( createCustomIconWindowEventHandlerUPP == NULL ) createCustomIconWindowEventHandlerUPP	= NewEventHandlerUPP( CreateCustomIconWindowEventHandler );
	(void) InstallWindowEventHandler( w->window, createCustomIconWindowEventHandlerUPP, GetEventTypeCount(windowEvents), windowEvents, w, NULL );

	GetWindowBounds( w->window, kWindowContentRgn, &r );
	minWindowSize.width		= r.right - r.left;
	minWindowSize.height	= r.bottom - r.top;
	(void) SetWindowResizeLimits( w->window, &minWindowSize, NULL );				//	Sets the minimum window size to the initial window size.  This prevents controls from overlapping

	if ( outWindow == NULL )	ShowWindow( w->window );							//	The window was created hidden so show it if the window parameter is NULL, if it's not, it will be the responsibility of the caller to show it.
	
CantCreateWindow:
	if ( outWindow != NULL )	*outWindow	= w->window;
	return( status );
}

//	Main window event handler.
static	pascal	OSStatus	CreateCustomIconWindowEventHandler( EventHandlerCallRef inCallRef, EventRef inEvent, void* inUserData )
{
	#pragma unused ( inCallRef )
	HICommand				command;
	ControlRef				control;
	char					path[MAXPATHLEN];
	OSStatus				err				= eventNotHandledErr;
	UInt32					eventKind		= GetEventKind( inEvent );
	UInt32					eventClass		= GetEventClass( inEvent );
	CustomIconWindowInfo	*w				= (CustomIconWindowInfo *) inUserData;

	switch ( eventClass )
	{
		case kEventClassWindow:
			if ( eventKind == kEventWindowClose )										//	Free up extra window storage
			{
				if ( w->graphicsImportComponent != NULL )   (void) CloseComponent( w->graphicsImportComponent );	//	Close the Graphics Importer
				DisposePtr( (Ptr) w );													//	Dispose window storage
			}
			break;

		case kEventClassCommand:
			if ( eventKind == kEventCommandProcess )
			{
				GetEventParameter( inEvent, kEventParamDirectObject, typeHICommand, NULL, sizeof(HICommand), NULL, &command );
				if ( (command.commandID == 'GImg') || (command.commandID == 'Targ') )
				{
					err	= NavOpenDocument( command.commandID );							//	User to select source image or target path
					
					GetControlCString( w->window, 'STxt', 0, path );
					if ( (w->graphicsImportComponent != NULL) && (strlen(path) > 0) )	//	If source image and target path have both been set
					{
						GetControlBySigAndID( w->window, 'Butn', 0, &control );
						(void) EnableControl( control );								//	Enable the "Set Custom Icon" button
					}
				}
				else if ( command.commandID == kHICommandOK )
				{
					SetCustomIcon( w );													//	Set the custom icon
				}
			}
		break;
	}
    
    return( err );
}


#pragma mark -
#pragma mark ¥ Save/Open Document ¥

static	OSStatus	NavOpenDocument( OSType command )
{
	OSStatus					status;
	NavDialogCreationOptions	options;
	NavReplyRecord				navReply;
	static	NavObjectFilterUPP	navFilterUPP;
	NavDialogRef				navDialog		= NULL;

	status	= NavGetDefaultDialogCreationOptions( &options );
	require_noerr( status, CantGetDefaultOptions );

	if ( navFilterUPP == NULL )	navFilterUPP = NewNavObjectFilterUPP( NavOpenFilterProc );		//	Filter only documents we can open

	if ( command == 'Targ' )	status	= NavCreateChooseObjectDialog( &options, NULL, NULL, NULL, NULL, &navDialog );	//	Select file, folder, anything as target
	else						status	= NavCreateChooseFileDialog( &options, NULL, NULL, NULL, navFilterUPP, NULL, &navDialog );	//	Select source image
	require_noerr( status, CantCreateDialog );
	
	status	= NavDialogRun( navDialog );
	require_noerr( status, CantRunDialog );
	
	status	= NavDialogGetReply( navDialog, &navReply );
	require( (status == noErr) || (status == userCanceledErr), CantGetReply );

	if ( navReply.validRecord ) status	= OpenDocument( navReply.selection, command );			//	Handle the image/target path by displaying it in the window
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
//	This filter is used to select source images which Graphics Importers can draw.
static	pascal	Boolean	NavOpenFilterProc( AEDesc *theItem, void *info, NavCallBackUserData callBackUD, NavFilterModes filterMode )
{
	FSRef					fsRef;
	OSStatus				status;
	FSSpec					fsSpec;
	FSCatalogInfo			fsCatalogInfo;
	Boolean					canOpen;
	Boolean					canViewItem			= false;
	
	if ( theItem->descriptorType == typeFSRef )
	{
		if ( ((NavFileOrFolderInfo*)info)->isFolder == true )	return( true );									//	Allow user to recurse into all directories
		
		status	= AEGetDescData( theItem, &fsRef, sizeof(fsRef) );												//	Get the FSRef
		require_noerr( status, CantGetFSRef );
		
		status	= FSGetCatalogInfo( &fsRef, kFSCatInfoFinderInfo, &fsCatalogInfo, NULL, &fsSpec, NULL );		//	Get the FSSpec from FSRef
		require_noerr( status, CantGetFSRef );

		status	= CanQuickTimeOpenFile( &fsSpec, ((FileInfo *)&fsCatalogInfo.finderInfo)->fileType, 0, NULL, &canOpen, NULL,  kQTDontLookForMovieImporterIfGraphicsImporterFound );
		if ( (status == noErr) && (canOpen==true) )	canViewItem	= true;
	}

CantGetFSRef:
	return( canViewItem );
}

//	Bottleneck routine responsible for opening files.  "command" specified weather the specified file should be opened 
//	as a source image or as a target to receive the custom icon.
static	OSStatus	OpenDocument( AEDescList docList, OSType command )
{
	FSRef				fsRef;
	WindowRef			window;
	OSStatus			status;

	if ( (status = AEGetNthPtr( &docList, 1, typeFSRef, NULL, NULL, &fsRef, sizeof(FSRef), NULL) ) == noErr )
	{
		window  = GetFrontWindowOfClass( kDocumentWindowClass, true );								//	If no window is open yet
		if ( window == NULL )
		{
			status	= DoNewWindow( &window );														//	Create a new (Hidden) window
			require_noerr( status, CantCreateWindow );
		}

		if ( command == 'GImg' )	status  = DisplayImageInWindow( window, &fsRef );				//	Display the selected source image in the window
		else						status  = DisplayCustomIconDestinationPath( window, &fsRef );	//	Display the target path
		ShowWindow( window );																		//	The window was created hidden so show it
	}
	
CantCreateWindow:
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

void	SetControlCString( WindowRef window, OSType signature, SInt32 id, char *cString )
{
	ControlRef		control;

	GetControlBySigAndID( window, signature, id, &control );
	if ( control == NULL ) goto Bail;
	
	(void) SetControlData( control, 0, kControlStaticTextTextTag, strlen(cString), cString );
	
Bail:
	return;
}

char	*GetControlCString( WindowRef window, OSType signature, SInt32 id, char *cString )
{
	ControlRef		control;
	SInt32			dataSize;

	cString[0]			= 0;	
	GetControlBySigAndID( window, signature, id, &control );
	if ( control == NULL ) goto Bail;
	
	(void)GetControlData( control, 0, kControlStaticTextTextTag, 4096, cString, &dataSize );
	cString[dataSize]	= 0;
Bail:
	return( cString );
}

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

OSErr	FSpGetCatInfo( FSSpec *spec, CInfoPBRec *cpb )
{
	cpb->hFileInfo.ioFDirIndex	= 0;
	cpb->hFileInfo.ioNamePtr	= spec->name;
	cpb->hFileInfo.ioVRefNum	= spec->vRefNum;
	cpb->hFileInfo.ioDirID		= spec->parID;
	
	return( PBGetCatInfoSync( cpb ) );
}

//	Given two rectangles, boundsRect and origRect, ScaleInRect will create the largest possible rectangle to fit in 
//	boundsRect, while maintaining the aspect ratio specified by origRect.  The new rectangle is centered and returned
//	in destRect
void	ScaleInRect( Rect *boundsRect, Rect *origRect, Rect *destRect )
{
	SInt16		width;
	SInt16		height;
	SInt16		hSlop;
	SInt16		vSlop;
	float		oRatio  = ((float)(origRect->right - origRect->left)) / ((float)(origRect->bottom - origRect->top));
	
	if ( ((boundsRect->bottom - boundsRect->top) * oRatio) > (boundsRect->right - boundsRect->left) )
	{
		width   = boundsRect->right - boundsRect->left;
		height  = width * (1.0/oRatio);
	}
	else
	{
		height  = boundsRect->bottom - boundsRect->top;
		width   = height * oRatio;
	}
	hSlop   = (boundsRect->right - boundsRect->left - width) / 2;
	vSlop   = (boundsRect->bottom - boundsRect->top - height) / 2;
	SetRect( destRect, boundsRect->left + hSlop, boundsRect->top + vSlop, boundsRect->left + width + hSlop, boundsRect->top + height + vSlop );
}

//	Utility routine to send the Finder an event within its kAEFinderSuite.  We use it to send a kAESync AppleEvent which forces the Finder
//	to immediately display the new icon.
OSErr	SendFinderAppleEvent( AliasHandle aliasH, AEEventID appleEventID )
{
	ProcessInfoRec			processInfo;
	OSErr					err;
	AppleEvent				appleEvent		= { typeNull, NULL };
	AEDesc					aeDesc			= { typeNull, NULL };
	ProcessSerialNumber		psn				= { kNoProcess, kNoProcess };
	AppleEvent				aeReply			= { typeNull, NULL };
	
	BlockZero( (Ptr)&processInfo, sizeof(processInfo) );
	processInfo.processInfoLength	= sizeof( processInfo );
	
	for ( err = GetNextProcess( &psn ) ; err == noErr ; err = GetNextProcess( &psn ) )
	{
		err	= GetProcessInformation( &psn, &processInfo );
		if ( (processInfo.processSignature == 'MACS') && (processInfo.processType == 'FNDR') )			//	Find the Finders PSN by searching all running processes
				break;
	}
	require_noerr( err, Bail );
	
	
	err	= AECreateDesc( typeProcessSerialNumber, &psn, sizeof( psn ), &aeDesc );
	require_noerr( err, Bail );
	err	= AECreateAppleEvent( kAEFinderSuite, appleEventID, &aeDesc, kAutoGenerateReturnID, kAnyTransactionID, &appleEvent );	//	Create AppleEvent (kAEFinderSuite, appleEventID)
	(void) AEDisposeDesc( &aeDesc );
	require_noerr( err, Bail );

	err	= AECreateDesc( typeAlias, *aliasH, GetHandleSize( (Handle)aliasH ), &aeDesc );
	require_noerr( err, Bail );

	err	= AEPutParamDesc( &appleEvent, keyDirectObject, &aeDesc );
	(void) AEDisposeDesc( &aeDesc );
	require_noerr( err, Bail );

	err = AESend( &appleEvent, &aeReply, kAENoReply, kAENormalPriority, kNoTimeOut, NULL, NULL );		//	Send the AppleEvent to the Finder
	(void) AEDisposeDesc( &aeReply );
	require_noerr( err, Bail );

	(void) AEDisposeDesc( &appleEvent );
	
Bail:	
	return( err );
}


#pragma mark -
#pragma mark ¥ Specialized Routines ¥

//	Displays the selected source image within our User Pane, ( 'UPan', 0 ).  We use QuickTime's Graphics importers to draw
//	the image whose maximum size is the bounds of the user pane, 128 X 128, which is coincidentally the maximum size the 
//	Finder will display an icon.
static  OSStatus	DisplayImageInWindow( WindowRef window, FSRef *fsRef )
{
	ControlRef					control;
	ControlRef					rootControl;
	Rect						boundsRect;
	Rect						r;
	CustomIconWindowInfo		*w;
	FSSpec						fsSpec;
	static	EventHandlerUPP		userPaneControlHandlerUPP;			//	Drawing is done within the user pane event handler draw message
	OSStatus					status						= -1;
	const EventTypeSpec			controlEvents[]				=   {   { kEventClassControl, kEventControlDraw }   };
	
	require( (fsRef != NULL) || (window != NULL), Bail );

	w   = (CustomIconWindowInfo *) GetWRefCon( window );			//	Get our window storage
	SetPortWindowPort( window );
	
	GetControlBySigAndID( window, 'UPan', 0, &control );			//	Get the user pane control
	
	if ( userPaneControlHandlerUPP == NULL ) userPaneControlHandlerUPP	= NewEventHandlerUPP( UserPaneControlHandler );
	(void) InstallControlEventHandler( control, userPaneControlHandlerUPP, GetEventTypeCount(controlEvents), controlEvents, w, NULL );

	GetControlBounds( control, &boundsRect );						//  Get the bounds of the User Pane within the window.  Because it is embedded, we offset it by its parents bounds.
	GetRootControl( w->window, &rootControl );
	for ( GetSuperControl( control, &control ) ; control != rootControl ; GetSuperControl( control, &control ) )
	{
		GetControlBounds( control, &r );
		OffsetRect( &boundsRect, r.left, r.top );
	}

	FSGetCatalogInfo( fsRef, kFSCatInfoNone, NULL, NULL, &fsSpec, NULL );
	if ( w->graphicsImportComponent != NULL )   (void) CloseComponent( w->graphicsImportComponent );
	status	= GetGraphicsImporterForFile( &fsSpec, &w->graphicsImportComponent );		//	Get the appropriate Graphics Importer for this file
	
	status	= GraphicsImportGetNaturalBounds( w->graphicsImportComponent, &r );
	ScaleInRect( &boundsRect, &r, &boundsRect );
	status	= GraphicsImportSetBoundsRect( w->graphicsImportComponent, &boundsRect );	//	Set the rectangle to draw to

Bail:	
	return( status );
}

//	Event Handler simply draws the image whenever the kEventControlDraw is received for our User pane control.
static	pascal	OSStatus	UserPaneControlHandler( EventHandlerCallRef inCallRef, EventRef inEvent, void* inUserData )
{
#pragma unused ( inCallRef )
	CustomIconWindowInfo		*w				= (CustomIconWindowInfo*) inUserData;
	UInt32						eventClass		= GetEventClass( inEvent );
	UInt32						eventKind		= GetEventKind( inEvent );
	OSStatus					err				= eventNotHandledErr;

	switch ( eventClass )
	{			
		case kEventClassControl:
			if ( eventKind == kEventControlDraw )
				err	= GraphicsImportDraw( w->graphicsImportComponent );					//	Graphics Importer draws the image
			break;
	}

	return( err );
}

//	Display the destination path selected by the user as a full path.
static  OSStatus	DisplayCustomIconDestinationPath( WindowRef window, FSRef *fsRef )
{
	char						path[MAXPATHLEN];
	OSStatus					status				= -1;
	
	require( (fsRef != NULL) || (window != NULL), Bail );

	status  = FSRefMakePath( fsRef, (UInt8*)path, MAXPATHLEN );									//	Create a full path
	SetControlCString( window, 'STxt', 0, path );										//	Display the path within the window

Bail:	
	return( status );
}


OSStatus	SetCustomIcon( CustomIconWindowInfo *w )
{
	OSStatus		status;
	char			path[MAXPATHLEN];
	Boolean			isDirectory;
	FSRef			fsRef;
	FSSpec			fsSpec;
	
	GetControlCString( w->window, 'STxt', 0, path );
	status  = FSPathMakeRef( (UInt8*)path, &fsRef, &isDirectory );									//	Create an FSRef from the destination path
	require_noerr( status, Bail );
	status  = FSGetCatalogInfo( &fsRef, kFSCatInfoNone, NULL, NULL, &fsSpec, NULL );		//	Create an FSSpec from the FSRef
	require_noerr( status, Bail );
	
	status  = CreateCustomIconFromGraphicsComponent( w->graphicsImportComponent, &fsSpec );	//	Set the custom icon of fsSpec!
	require_noerr( status, Bail );
	
Bail:	
	return( status );
}


//	This utility routine will set the custom icon in the file/folder targetSpec with an image described by the Graphics Importer.
//	It does this by creating a PicHandle, then creating an icon from the Picture, and finally saving the custom icon to disk.
OSErr	CreateCustomIconFromGraphicsComponent( GraphicsImportComponent graphicsImportComponent, FSSpec *targetSpec )
{
	PicHandle					pictH;
	IconFamilyHandle			icnsH		= NULL;
	OSErr						err			= -1;
	
	icnsH	= (IconFamilyHandle) NewHandle( 8 );								//	IconFamilyResource.resourceType + IconFamilyResource.resourceSize
	if ( icnsH == NULL ) return( memFullErr );
	(**icnsH).resourceType			= kIconFamilyType;
	(**icnsH).resourceSize			= 8;

	pictH	= CreatePictHandleFromGraphicsComponent( graphicsImportComponent );	//	Create the PicHandle
	if ( pictH == NULL )	goto Bail;

	err	= SetIconFamilyData( icnsH, 'PICT', (Handle)pictH );					//	Create the icon from the Picture
	require_noerr( err, Bail );
	
	SaveCustomIcon( targetSpec, icnsH );										//	Save the custom icon to disk

Bail:	
	if ( icnsH != NULL ) DisposeHandle( (Handle) icnsH );
	if ( pictH != NULL ) KillPicture( pictH );
	return( err );
}


//	Utility routine to create a PicHandle from a Graphics Importer.
PicHandle	CreatePictHandleFromGraphicsComponent( GraphicsImportComponent graphicsImportComponent )
{
	Rect						imageRect;
	OSStatus					status;
	PicHandle					pictH		= NULL;

	require( graphicsImportComponent != NULL , Bail );
	status = GraphicsImportGetBoundsRect( graphicsImportComponent, &imageRect );		//	Get the rectangle of the picture
	require_noerr( status, Bail );

	if ( (imageRect.bottom > imageRect.top) && (imageRect.right > imageRect.left) )		//	Sanity check to make sure the picture is not corrupt
	{
		ClipRect( &imageRect );
		
		pictH	= OpenPicture( &imageRect );											//	Create the PicHandle OpenPicture -> Draw -> ClosePicture
		status	= GraphicsImportDraw( graphicsImportComponent );
		ClosePicture();
	}
Bail:
	return( pictH );
}

//	Saves the custom icon to disk
//	A file with a custom icon must contain a resource ( 'icns', -16455 ) and have its kHasCustomIcon bit set.
//	A folder with a custom icon must contain a file named "\pIcon\r" within it with a resource ( 'icns', -16455 ) and have the folders kHasCustomIcon bit set.
OSErr	SaveCustomIcon( FSSpec *targetSpec, IconFamilyHandle icnsH )
{
	short				refNum;
	OSErr				err;
	Handle				h;
	CInfoPBRec			cpb;
	AliasHandle			aliasH;
	
	err = FSpGetCatInfo( targetSpec, &cpb );
	require_noerr( err, Bail );
	
	if ( cpb.hFileInfo.ioFlAttrib & kioFlAttribDirMask )									//	If the target is a directory
	{
		CInfoPBRec	iconCPB;
		FSSpec		fsSpec;
		err = FSMakeFSSpec( cpb.dirInfo.ioVRefNum, cpb.dirInfo.ioDrDirID, "\pIcon\r", &fsSpec );//	Create an FSSpec to the "\pIcon\r" file
		HCreateResFile( fsSpec.vRefNum, fsSpec.parID, "\pIcon\r" );							//	Create the resource forked file "\pIcon\r"
		refNum = FSpOpenResFile( &fsSpec, fsRdWrPerm );										//	Open it with write permissions
		
		(void) FSpGetCatInfo( &fsSpec, &iconCPB );
		iconCPB.hFileInfo.ioFlFndrInfo.fdFlags	|=	kIsInvisible;							//	Set the kIsInvisible flag of the "Icon\r" file
		iconCPB.hFileInfo.ioDirID   = iconCPB.dirInfo.ioDrParID;
		(void) PBSetCatInfoSync( &iconCPB );
	}
	else
	{
		HCreateResFile( targetSpec->vRefNum, targetSpec->parID, targetSpec->name );			//	Create the resource forked file "\pIcon\r"
		refNum = FSpOpenResFile( targetSpec, fsCurPerm );									//	Open it with write permissions
	}
	
	if ( refNum == -1 )	goto Bail;
	
	UseResFile( refNum );
	h	= Get1Resource( 'icns', kCustomIconResource );
	if ( h != NULL )																		//	If it already has a custom icon
	{
		RemoveResource( h );																//	Remove the existing custom icon
		DisposeHandle( h );
	}
	AddResource( (Handle) icnsH, 'icns', kCustomIconResource, nil );						//	Create a new resource ( 'icns', -16455 )
	err	= ResError(); if ( err != noErr ) DebugStr("\p AddResource Failed" );
	WriteResource( (Handle) icnsH );														//	Save the 'icns' resource
	err	= ResError(); if ( err != noErr ) DebugStr("\p WriteResource Failed" );
	DetachResource( (Handle) icnsH );
	err	= ResError(); if ( err != noErr ) DebugStr("\p DetachResource Failed" );
	CloseResFile( refNum );																	//	Close the resource fork
	
	if ( targetSpec->parID == fsRtParID )													//	As of Mac OS X 10.4, the disk icon requires it be stored in ".VolumeIcon.icns"
	{
		FSSpec		volumeIconSpec;
		long		ioCount;
		refNum	= 0;
		err	= FSMakeFSSpec( targetSpec->vRefNum, fsRtDirID, "\p.VolumeIcon.icns", &volumeIconSpec );	//	"." files are always invisible
		if ( err == noErr )	(void) FSpDelete( &volumeIconSpec );
		err	= FSpCreate( &volumeIconSpec, 0, 0, smSystemScript );			if ( err != noErr )	goto BailOnTenFour;
		err	= FSpOpenDF( &volumeIconSpec, fsWrPerm, &refNum );				if ( err != noErr )	goto BailOnTenFour;
		ioCount = GetHandleSize( (Handle) icnsH );
		err	= FSWrite( refNum, &ioCount, *icnsH );							if ( err != noErr )	goto BailOnTenFour;	//	Write the icns file
BailOnTenFour:
		if ( refNum != 0 )	FSClose( refNum );
	}
	
	cpb.hFileInfo.ioFlFndrInfo.fdFlags	|=	kHasCustomIcon;									//	Set the kHasCustomIcon flag
	cpb.hFileInfo.ioDirID   = cpb.dirInfo.ioDrParID;
	err	= PBSetCatInfoSync( &cpb );															//	Set the Custom Icon bit on disk

	err	= NewAliasMinimal( targetSpec, &aliasH );											//	Create an alias to our target
	require_noerr( err, Bail );
	err	= SendFinderAppleEvent( aliasH, kAESync );											//	Send the Finder a kAESync AppleEvent to force it to update the icon immediately
	DisposeHandle( (Handle) aliasH );

Bail:
	return( err );
}







