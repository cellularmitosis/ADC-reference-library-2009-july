//
//	File:		main.c of RecentItems
// 
//	Contains:	A skeleton of modern nib-based and Carbon Events-based Carbon application.
//	
//	Version:	1.0
// 
//	Created:	5 / 15 / 06
//	
//	Copyright:	Copyright 2006 Apple Computer, Inc., All Rights Reserved
// 
//	Disclaimer: IMPORTANT:	This Apple software is supplied to you by Apple Computer, Inc. ( "Apple" ) in 
//				consideration of your agreement to the following terms, and your use, installation, modification 
//				or redistribution of this Apple software constitutes acceptance of these terms.	 If you do 
//				not agree with these terms, please do not use, install, modify or redistribute this Apple 
//				software.
//
//				In consideration of your agreement to abide by the following terms, and subject to these terms,
//				Apple grants you a personal, non-exclusive license, under Apple's copyrights in this 
//				original Apple software ( the "Apple Software" ), to use, reproduce, modify and redistribute the 
//				Apple Software, with or without modifications, in source and / or binary forms; provided that if you 
//				redistribute the Apple Software in its entirety and without modifications, you must retain this 
//				notice and the following text and disclaimers in all such redistributions of the Apple Software. 
//				Neither the name, trademarks, service marks or logos of Apple Computer, Inc. may be used to 
//				endorse or promote products derived from the Apple Software without specific prior written 
//				permission from Apple.	Except as expressly stated in this notice, no other rights or 
//				licenses, express or implied, are granted by Apple herein, including but not limited to any 
//				patent rights that may be infringed by your derivative works or by other works in which the 
//				Apple Software may be incorporated.
//
//				The Apple Software is provided by Apple on an "AS IS" basis. APPLE MAKES NO WARRANTIES, EXPRESS OR 
//				IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY 
//				AND FITNESS FOR A PARTICULAR PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND OPERATION ALONE 
//				OR IN COMBINATION WITH YOUR PRODUCTS.
//
//				IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL OR CONSEQUENTIAL 
//				DAMAGES ( INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS 
//				OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION ) ARISING IN ANY WAY OUT OF THE USE,
//				REPRODUCTION, MODIFICATION AND / OR DISTRIBUTION OF THE APPLE SOFTWARE, HOWEVER CAUSED AND WHETHER 
//				UNDER THEORY OF CONTRACT, TORT ( INCLUDING NEGLIGENCE ), STRICT LIABILITY OR OTHERWISE, EVEN 
//				IF APPLE HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
//****************************************************
#pragma mark * complation directives * 
#define USE_DRAG_N_DROP	TRUE

//****************************************************
#pragma mark -
#pragma mark * includes & imports * 
//----------------------------------------------------

#include <Carbon/Carbon.h> 
#include "RecentItems.h"

//****************************************************
#pragma mark -
#pragma mark * typedef's, struct's, enums, defines, etc. *
//----------------------------------------------------

#if DEBUG_ASSERT_PRODUCTION_CODE
#define require_orelse_continue( assertion )						\
{																	\
	if ( !( assertion ) )											\
		continue;													\
}
#else
#define require_orelse_continue( assertion ) {						\
	if ( !( assertion ) ) {											\
		DEBUG_ASSERT_MESSAGE( DEBUG_ASSERT_COMPONENT_NAME_STRING,	\
								#assertion,							\
								0,									\
								0,									\
								__FILE__,							\
								__LINE__,							\
								0 );								\
								continue;							\
								}									\
}
#endif

#define require_orelse_continue_quiet( assertion )					\
{																	\
	if ( !( assertion ) )											\
		continue;													\
}

#if USE_DRAG_N_DROP
typedef struct dnd_data_struct {
	HIViewRef	fHIViewRef;
	HIPoint		fHIPoint;
	Boolean		fDragging;
} dnd_data_rec, *dnd_data_ptr;
#endif USE_DRAG_N_DROP

// Application preference keys

#define kOpenFolderContentsPrefKey			CFSTR( "Open Folders?" )
#define kOpenFolderRecursivePrefKey			CFSTR( "Open Folders Recursive?" )

#define kResolveAliasesContentsPrefKey		CFSTR( "Resolve Aliases?" )
#define kResolveAliasesChainsPrefKey		CFSTR( "Resolve Aliases Chains?" )

#define kRecentAppsPrefKey					CFSTR( "Recent Apps" )
#define kRecentFoldersPrefKey				CFSTR( "Recent Folders" )
#define kRecentDocsPrefKey					CFSTR( "Recent Docs" )

// Preferences window's command ID's

#define kOpenFoldersHICommand				'OFld'
#define kOpenFoldersRecursiveHICommand		'OFRc'

#define kResolveAliasesHICommand			'RAls'
#define kResolveAliasesChainsHICommand		'RARc'

// The commands send by our recent menu items
#define	kRecentAppsMenuCMD		'RecA'
#define	kRecentFoldersMenuCMD	'RecF'
#define	kRecentDocsMenuCMD		'RecD'
#define	kRecentClearMenuCMD		'RecC'

//****************************************************
#pragma mark -
#pragma mark * local ( static ) function prototypes *
//----------------------------------------------------

static pascal	OSErr		Handle_OpenApplicationAE( const AppleEvent * inAppleEvent, AppleEvent * outAppleEvent, long inHandlerRefcon );
static pascal	OSErr		Handle_ReopenApplicationAE( const AppleEvent * inAppleEvent, AppleEvent * outAppleEvent, long inHandlerRefcon );
static pascal	OSErr		Handle_OpenDocumentsAE( const AppleEvent * inAppleEvent, AppleEvent * outAppleEvent, long inHandlerRefcon );
static pascal	OSErr		Handle_PrintDocumentsAE( const AppleEvent * inAppleEvent, AppleEvent * outAppleEvent, long inHandlerRefcon );
static			OSErr		Install_AppleEventHandlers( void );

static			OSStatus	Handle_AppEvents( EventHandlerCallRef inCaller, EventRef inEvent, void * inRefcon );
static			OSStatus	Handle_NewWindow( void );
static			OSStatus	Handle_WindowEvents( EventHandlerCallRef inCaller, EventRef inEvent, void * inRefcon );
#if USE_DRAG_N_DROP
static			OSStatus	Handle_DragEvents( EventHandlerCallRef inCaller, EventRef inEvent, void * inRefcon );
#endif USE_DRAG_N_DROP

static			void		Handle_NewPrefWindow( void );
static pascal	OSStatus	Handle_PrefCommandProcess( EventHandlerCallRef inHandlerCallRef, EventRef inEvent, void * inUserData );
static pascal	OSStatus	Handle_PrefWindowIsAboutToClose( EventHandlerCallRef inHandlerCallRef, EventRef inEvent, void * inUserData );
static			void		Get_Preferences( void );
static			OSStatus	Set_Preferences( void );

static			OSStatus	Do_OpenDocs( AEDescList inDocumentsList );
static			void		Append_ContainerItemsToAEDescList( const FSRef * inFSRef, AEDescList inDocumentsList );
static			OSErr		Get_ContainerItems( const FSRef * inContainerFSRef, FSRef * **outFSRefHandle, ItemCount * outNumRefs, Boolean * outContainerChanged );

//****************************************************
#pragma mark -
#pragma mark * exported globals *
//----------------------------------------------------

//****************************************************
#pragma mark -
#pragma mark * local ( static ) globals *
//----------------------------------------------------

static FSRef		gApplicationBundleFSRef;
static IBNibRef		gMainIBNibRef = NULL;
static Boolean		gAutoQuit = FALSE;		// if this is TRUE we auto-quit after a drag-n-drop ( 'odoc' ) launch

// our preferences
static WindowRef gPreferencesWindow = NULL;
static Boolean gOpenFolderContents = TRUE;
static Boolean gOpenFolderRecursive = TRUE;
static Boolean gResolveAliases = TRUE;
static Boolean gResolveAliasesChains = TRUE;

// Preferences window's checkboxes' IDs
static const HIViewID kOpenFoldersHID = { kOpenFoldersHICommand, 100 };
static const HIViewID kOpenFoldersRecursiveHID = { kOpenFoldersRecursiveHICommand, 100 };
static const HIViewID kResolveAliasesHID = { kResolveAliasesHICommand, 100 };
static const HIViewID kResolveAliasesChainsHID = { kResolveAliasesChainsHICommand, 100 };

#if USE_DRAG_N_DROP
// Our Drag-N-Drop HIView ( user pane )
static const HIViewID kDragNDropHID = { 'DnDv', 0 };
#endif USE_DRAG_N_DROP

static MenuRef	gRecentAppsMenuRef = NULL;
static MenuRef	gRecentFoldersMenuRef = NULL;
static MenuRef	gRecentDocsMenuRef = NULL;

//****************************************************
#pragma mark -
#pragma mark * exported function implementations *
//----------------------------------------------------

/*****************************************************
*
* Routine:	main ( argc, argv ) 
*
* Purpose:	main program entry point
*
* Inputs:	argc - the number of elements in the argv array
*			argv - an array of pointers to the parameters to this application
*
* Returns:	int - error code ( 0 == no error ) 
*
*/
int main( int argc, char * argv[] )
{
    OSStatus result;
	
	Get_Preferences( );	// load user preferences
	
	ProcessSerialNumber psn = { 0, kCurrentProcess };
	result = GetProcessBundleLocation( &psn, &gApplicationBundleFSRef );
	require_noerr( result, Oops );
	
    // Create a Nib reference, passing the name of the nib file ( without the .nib extension ).
    // CreateNibReference only searches into the application bundle.
    result = CreateNibReference( CFSTR( "main" ), &gMainIBNibRef );
    require_noerr( result, Oops );
    
    // Once the nib reference is created, set the menu bar. "MainMenu" is the name of the menu bar
    // object. This name is set in InterfaceBuilder when the nib is created.
    result = SetMenuBarFromNib( gMainIBNibRef, CFSTR( "MenuBar" ) );
    require_noerr( result, Oops );
	
	// Install the Apple Event handlers
	Install_AppleEventHandlers( );
    
    // Install our handler for common commands on the application target
    const EventTypeSpec    kAppEvents[] = {
	{ kEventClassCommand, kEventCommandProcess }, 	{ kEventClassCommand, kEventCommandUpdateStatus },     };
    InstallApplicationEventHandler( NewEventHandlerUPP( Handle_AppEvents ), GetEventTypeCount( kAppEvents ), kAppEvents, 0, NULL );

	// setup the recent menus
	MenuRef tMenuRef;
	MenuItemIndex tMenuItemIndex;
	// Applications
	do {
		result = GetIndMenuItemWithCommandID( NULL, kRecentAppsMenuCMD, 1, &tMenuRef, &tMenuItemIndex );
		if ( noErr != result ) break;

		result = GetMenuItemHierarchicalMenu( tMenuRef, tMenuItemIndex, &gRecentAppsMenuRef );
		if ( noErr != result ) break;

		result = RecentItems_SetMaxItems( gRecentAppsMenuRef, 10 );
		if ( noErr != result ) break;

		result = RecentItems_PopulateMenu( gRecentAppsMenuRef, kRecentAppsPrefKey, kRecentAppsMenuCMD );
		if ( noErr != result ) break;
	} while ( FALSE );

	// Folders
	do {
		result = GetIndMenuItemWithCommandID( NULL, kRecentFoldersMenuCMD, 1, &tMenuRef, &tMenuItemIndex );
		if ( noErr != result ) break;
		
		result = GetMenuItemHierarchicalMenu( tMenuRef, tMenuItemIndex, &gRecentFoldersMenuRef );
		if ( noErr != result ) break;
		
		result = RecentItems_SetMaxItems( gRecentFoldersMenuRef, 20 );
		if ( noErr != result ) break;
		
		result = RecentItems_PopulateMenu( gRecentFoldersMenuRef, kRecentFoldersPrefKey, kRecentFoldersMenuCMD );
		if ( noErr != result ) break;
	} while ( FALSE );

	// Documents
	do {
		result = GetIndMenuItemWithCommandID( NULL, kRecentDocsMenuCMD, 1, &tMenuRef, &tMenuItemIndex );
		if ( noErr != result ) break;
		
		result = GetMenuItemHierarchicalMenu( tMenuRef, tMenuItemIndex, &gRecentDocsMenuRef );
		if ( noErr != result ) break;
		
		result = RecentItems_SetMaxItems( gRecentDocsMenuRef, 30 );
		if ( noErr != result ) break;
		
		result = RecentItems_PopulateMenu( gRecentDocsMenuRef, kRecentDocsPrefKey, kRecentDocsMenuCMD );
		if ( noErr != result ) break;

		// result = RecentItems_DisableOpenItems( gRecentDocsMenuRef );
	} while ( FALSE );

	result = noErr;
	
    // Run the event loop
    RunApplicationEventLoop( );
	
Oops:	;
	return result;
}	// main

/*****************************************************/
#pragma mark -
#pragma mark * local ( static ) function implementations *
//----------------------------------------------------

#pragma mark * AppleEvent Handlers * 
/*****************************************************
*
* Routine:	Handle_OpenApplicationAE( inAppleEvent, reply, inHandlerRefcon ) 
*
* Purpose:	AppleEvent handler for the kAEOpenApplication event
*
* Inputs:	inAppleEvent - the Apple event
*			reply - our reply to the Apple event
*			inHandlerRefcon - refcon passed to AEInstallEventHandler when this hander was installed
*
* Returns:	OSErr - error code ( 0 == no error ) 
*
*/
static pascal OSErr Handle_OpenApplicationAE( const AppleEvent * inAppleEvent, AppleEvent * outAppleEvent, long inHandlerRefcon )
{
	gAutoQuit = FALSE;	// this is NOT a drag-n-drop launch; disable auto-quit
	
	// if no windows are open then...
	WindowRef tWindowRef = GetFrontWindowOfClass( kDocumentWindowClass, TRUE );
	if ( !tWindowRef )
		Handle_NewWindow( ); // create an empty window
	return noErr;
}	// Handle_OpenApplicationAE

/*****************************************************
*
* Routine:	Handle_ReopenApplicationAE( inAppleEvent, reply, inHandlerRefcon ) 
*
* Purpose:	AppleEvent handler for the kAEReopenApplication event
*
* Inputs:	inAppleEvent - the Apple event
*			reply - our reply to the Apple event
*			inHandlerRefcon - refcon passed to AEInstallEventHandler when this hander was installed
*
* Returns:	OSErr - error code ( 0 == no error ) 
*
*/
static pascal OSErr Handle_ReopenApplicationAE( const AppleEvent * inAppleEvent, AppleEvent * outAppleEvent, long inHandlerRefcon )
{
	// We were already running but with no windows so we create an empty one.
	WindowRef tWindowRef = GetFrontWindowOfClass( kDocumentWindowClass, TRUE );
	if ( !tWindowRef )
		Handle_NewWindow( );
	return noErr;
}	// Handle_ReopenApplicationAE

/*****************************************************
*
* Routine:	Handle_OpenDocumentsAE( inAppleEvent, reply, inHandlerRefcon ) 
*
* Purpose:	AppleEvent handler for the kAEOpenDocuments event
*
* Inputs:	inAppleEvent - the Apple event
*			reply - our reply to the Apple event
*			inHandlerRefcon - refcon passed to AEInstallEventHandler when this hander was installed
*
* Returns:	OSErr - error code ( 0 == no error ) 
*
*/
static pascal OSErr Handle_OpenDocumentsAE( const AppleEvent * inAppleEvent, AppleEvent * outAppleEvent, long inHandlerRefcon )
{
	AEDescList documentsList;
	OSErr err = AEGetParamDesc( inAppleEvent, keyDirectObject, typeAEList, &documentsList );
	require_noerr( err, Oops );
	
	err = Do_OpenDocs( documentsList );
	
	AEDisposeDesc( &documentsList );
	
Oops:	;
	
	if ( gAutoQuit ) {
		printf( "Handle_OpenDocumentsAE, gAutoQuit.\n" );
		QuitApplicationEventLoop( );
	}
	
	return err;
}	// Handle_OpenDocumentsAE

/*****************************************************
*
* Routine:	Handle_PrintDocumentsAE( inAppleEvent, reply, inHandlerRefcon ) 
*
* Purpose:	AppleEvent handler for the kAEPrintDocuments event
*
* Inputs:	inAppleEvent - the Apple event
*			reply - our reply to the Apple event
*			inHandlerRefcon - refcon passed to AEInstallEventHandler when this hander was installed
*
* Returns:	OSErr - error code ( 0 == no error ) 
*
*/
static pascal OSErr Handle_PrintDocumentsAE( const AppleEvent * inAppleEvent, AppleEvent * outAppleEvent, long inHandlerRefcon )
{
	return errAEEventNotHandled;
}	// Handle_PrintDocumentsAE

/*****************************************************
*
* Routine:	Install_AppleEventHandlers( void ) 
*
* Purpose:	installs the AppleEvent handlers
*
* Inputs:	none
*
* Returns:	OSErr - error code ( 0 == no error ) 
*
*/
static OSErr Install_AppleEventHandlers( void )
{
	OSErr	result;
	
	result = AEInstallEventHandler( kCoreEventClass, kAEOpenApplication, Handle_OpenApplicationAE, 0, FALSE );
	require_noerr( result, Oops );
	
	result = AEInstallEventHandler( kCoreEventClass, kAEReopenApplication, Handle_ReopenApplicationAE, 0, FALSE );
	require_noerr( result, Oops );
	
	result = AEInstallEventHandler( kCoreEventClass, kAEOpenDocuments, Handle_OpenDocumentsAE, 0, FALSE );
	require_noerr( result, Oops );
	
	result = AEInstallEventHandler( kCoreEventClass, kAEPrintDocuments, Handle_PrintDocumentsAE, 0, FALSE );
	require_noerr( result, Oops );
	
	// Note: Since RunApplicationEventLoop installs a Quit AE Handler, there is no need to do it here.
	
Oops:	;
	
	return result;
}	// Install_AppleEventHandlers

#pragma mark -
#pragma mark * CarbonEvent Handlers *
//----------------------------------------------------


/*****************************************************
*
* Routine:	Handle_AppEvents( inCaller, inEvent, inRefcon ) 
*
* Purpose:	called to process commands from Carbon events
*
* Inputs:	inCaller - reference to the current handler call chain
*			inEvent - the event
*			inRefcon - app - specified data you passed in the call to InstallApplicationEventHandler ( NULL )
*
* Returns:	OSStatus - noErr indicates the event was handled
*								 eventNotHandledErr indicates the event was not handled and the Toolbox should take over
*
*/
static OSStatus Handle_AppEvents( EventHandlerCallRef inCaller, EventRef inEvent, void * inRefcon )
{
    OSStatus    result = eventNotHandledErr;

    UInt32 eventClass = GetEventClass( inEvent );
    UInt32 eventKind = GetEventKind( inEvent );
    
    switch ( eventClass ) {
        case kEventClassCommand: {
            HICommand tHICommand;
            verify_noerr( GetEventParameter( inEvent, kEventParamDirectObject, typeHICommand, NULL, sizeof( tHICommand ), NULL, &tHICommand ) );
			
            switch ( eventKind ) {
                case kEventCommandProcess: {
                    switch ( tHICommand.commandID ) {
						case kHICommandPreferences:
							Handle_NewPrefWindow( );
							break;
                        case kHICommandNew: {
                            result = Handle_NewWindow( );
                            break;
						}
						case kRecentAppsMenuCMD:
						case kRecentFoldersMenuCMD:
						case kRecentDocsMenuCMD: {
							if ( kHICommandFromMenu & tHICommand.attributes ) {
								FSRef tFSRef;
								OSStatus status = RecentItems_GetMenuItemFile( tHICommand.menu.menuRef, tHICommand.menu.menuItemIndex, &tFSRef );
								if ( noErr == status ) {
									HFSUniStr255 tHFSUniStr255;
									FSGetCatalogInfo( &tFSRef, kFSCatInfoNone, NULL, &tHFSUniStr255, NULL, NULL );
									
									CFStringRef tCFStringRef = CFStringCreateWithCharacters( kCFAllocatorDefault, tHFSUniStr255.unicode, tHFSUniStr255.length );
									if ( tCFStringRef ) {
										char buffer[256];
										CFStringGetCString( tCFStringRef, buffer, 256, kCFStringEncodingMacRoman );
										
										switch ( tHICommand.commandID ) {
											case kRecentAppsMenuCMD: {
												printf( "Recent app menu item #%d: <%s > .\n", tHICommand.menu.menuItemIndex, buffer );
												break;
											}
											case kRecentFoldersMenuCMD: {
												printf( "Recent folder menu item #%d: <%s > .\n", tHICommand.menu.menuItemIndex, buffer );
												break;
											}
											case kRecentDocsMenuCMD: {
												printf( "Recent file menu item #%d: <%s > .\n", tHICommand.menu.menuItemIndex, buffer );
												break;
											}
										}
										CFRelease( tCFStringRef );
										result = noErr;
									}
								}
							}
                            break;
						}
#if TRUE
						case kRecentClearMenuCMD: {
							// clear all our recent items ( ignoring errors )
							( void ) RecentItems_Clear( kRecentAppsPrefKey );
							( void ) RecentItems_Clear( kRecentFoldersPrefKey );
							( void ) RecentItems_Clear( kRecentDocsPrefKey );
							// after we've cleared all our prefs we repopulate our menus ( ignoring errors )
							( void ) RecentItems_PopulateMenu( gRecentAppsMenuRef, kRecentAppsPrefKey, kRecentAppsMenuCMD );
							( void ) RecentItems_PopulateMenu( gRecentFoldersMenuRef, kRecentFoldersPrefKey, kRecentFoldersMenuCMD );
							( void ) RecentItems_PopulateMenu( gRecentDocsMenuRef, kRecentDocsPrefKey, kRecentDocsMenuCMD );
							break;
						}
#endif
						case kHICommandAbout:
						case kHICommandOpen:
						case kHICommandQuit:
						{
							// these are just here to prevent them from logging the "unhandled commandID" error below
							break;
						}
						// Add your own command handling cases here
							
						default: {
							if ( tHICommand.commandID < '    ' ) {
								fprintf( stderr, "%s: Unhandled commandID: %ld ( 0x % 08lX ).\n", __PRETTY_FUNCTION__, tHICommand.commandID, tHICommand.commandID );
							} else {
								fprintf( stderr, "%s: Unhandled commandID: '%.4s'.\n", __PRETTY_FUNCTION__, &tHICommand.commandID );
							}
							break;
						}
                    }
                    break;
				}
				case kEventCommandUpdateStatus: {
					// Note: You'll only get here if there aren't any windows open OR 
					// if Handle_WindowEvents returns eventNotHandledErr.
					switch ( tHICommand.commandID ) {
						case kHICommandAbout:
						case kHICommandPreferences:
						{
							EnableMenuItem( tHICommand.menu.menuRef, tHICommand.menu.menuItemIndex );
							break;
						}
						case kHICommandClose:
						case kHICommandSave:
						case kHICommandSaveAs:
						case kHICommandRevert:
						case kHICommandPageSetup:
						case kHICommandPrint:
						case kHICommandUndo:
						case kHICommandRedo:
						case kHICommandCut:
						case kHICommandCopy:
						case kHICommandPaste:
						case kHICommandClear:
						case kHICommandSelectAll:
						{
							DisableMenuItem( tHICommand.menu.menuRef, tHICommand.menu.menuItemIndex );
							break;
						}
					}
					break;
				}
				default: {
					fprintf( stderr, "%s: Unhandled event { class: '%.4s', kind: '%.4s' }.\n", __PRETTY_FUNCTION__, &eventClass, &eventKind );
					break;
				}
            }
            break;
        }
        default: {
			fprintf( stderr, "%s: Unhandled event { class: '%.4s', kind: '%.4s' }.\n", __PRETTY_FUNCTION__, &eventClass, &eventKind );
            break;
		}
    }
    return result;
}	// Handle_AppEvents

#pragma mark -
#pragma mark * Windows *
//----------------------------------------------------

/*****************************************************
*
* Routine:	Handle_NewWindow( ) 
*
* Purpose:	called create a new window
*
* Inputs:	none
*
* Returns:	OSErr - error code ( 0 == no error ) 
*
*/
static OSStatus Handle_NewWindow( void )
{
    OSStatus result;

    const EventTypeSpec    kWindowEvents[] = {
		{ kEventClassCommand, kEventCommandProcess }, 		{ kEventClassCommand, kEventCommandUpdateStatus },     };
	static EventHandlerUPP sWindowEventHandlerUPP = NULL;
	if ( !sWindowEventHandlerUPP ) {
		sWindowEventHandlerUPP = NewEventHandlerUPP( Handle_WindowEvents );
		assert( sWindowEventHandlerUPP );
	}
    
    // Create a window. "MainWindow" is the name of the window object. This name is set in 
    // InterfaceBuilder when the nib is created.
    WindowRef tWindowRef;
    result = CreateWindowFromNib( gMainIBNibRef, CFSTR( "MainWindow" ), &tWindowRef );
    require_noerr( result, Oops );
	
    // Install a command handler on the window. We don't use this handler yet, but nearly all
    // Carbon apps will need to handle commands, so this saves everyone a little typing.
    result = InstallWindowEventHandler( tWindowRef, sWindowEventHandlerUPP,     									GetEventTypeCount( kWindowEvents ), kWindowEvents,     									tWindowRef, NULL );
    require_noerr( result, Oops );

#if USE_DRAG_N_DROP
	OSStatus status = noErr;

	do {	// basicly a TRY block
		HIViewRef tHIViewRef;
		status = HIViewFindByID( HIViewGetRoot( tWindowRef ), kDragNDropHID, &tHIViewRef );
		if ( noErr != status ) break;

		dnd_data_ptr dnd_ptr = ( dnd_data_ptr ) malloc( sizeof( dnd_data_rec ) );
		if ( !dnd_ptr ) break;

		SetControlReference( tHIViewRef, ( SInt32 ) dnd_ptr );
		
		EventTypeSpec tDragEvents[] = {
		{ kEventClassControl, kEventControlDraw }, 		{ kEventClassControl, kEventControlDragEnter }, 			// { kEventClassControl, kEventControlDragWithin }, 		{ kEventClassControl, kEventControlDragLeave }, 		{ kEventClassControl, kEventControlDragReceive }
		};
		status = HIViewInstallEventHandler( tHIViewRef, Handle_DragEvents, GetEventTypeCount( tDragEvents ), tDragEvents, tHIViewRef, NULL );
		if ( noErr != status ) break;

		status = SetControlDragTrackingEnabled( tHIViewRef, TRUE );
		if ( noErr != status ) break;

	} while ( FALSE );

	// We want window and thus our custom view to be able to respond to drops
	status = SetAutomaticControlDragTrackingEnabledForWindow( tWindowRef, TRUE );
	require_noerr( status, Oops );
#endif USE_DRAG_N_DROP
	
    // Position new windows in a staggered arrangement on the main screen
    RepositionWindow( tWindowRef, NULL, kWindowCascadeOnMainScreen );

	// mark window as not modified
	SetWindowModified( tWindowRef, FALSE );

    // The window was created hidden, so show it
    ShowWindow( tWindowRef );
    
Oops:	;
	return result;
}

/*****************************************************
*
* Routine:	Handle_WindowEvents( inCaller, inEvent, inRefcon ) 
*
* Purpose:	called to process our window events
*
* Inputs:	inCaller - reference to the current handler call chain
*			inEvent - the event
*			inRefcon - app - specified data you passed in the call to InstallApplicationEventHandler ( NULL )
*
* Returns:	OSStatus - noErr indicates the event was handled
*						  eventNotHandledErr indicates the event was not handled and the Toolbox should take over
*
*/
static OSStatus Handle_WindowEvents( EventHandlerCallRef inCaller, EventRef inEvent, void * inRefcon )
{
	WindowRef	tWindowRef = ( WindowRef ) inRefcon;
    OSStatus    result = eventNotHandledErr;

    UInt32 eventClass = GetEventClass( inEvent );
    UInt32 eventKind = GetEventKind( inEvent );
	
    switch ( eventClass ) {
        case kEventClassCommand: {
            HICommand tHICommand;
            verify_noerr( GetEventParameter( inEvent, kEventParamDirectObject, typeHICommand, NULL, sizeof( tHICommand ), NULL, &tHICommand ) );

			// Note: any command class events that return eventNotHandledErr here will be proprogated to Handle_AppEvents

			switch ( eventKind ) {
                case kEventCommandProcess: {
                    switch ( tHICommand.commandID ) {
                        // Add your own command handling cases here
						case kHICommandAbout:
						case kHICommandPreferences:
                        case kHICommandSelectWindow:
						case kHICommandOpen:
						case kHICommandClose:
						case kHICommandQuit:
						{
							// these are just here to prevent them from logging the "unhandled commandID" error below
							break;
						}
						default: {
							if ( tHICommand.commandID < '    ' ) {
								fprintf( stderr, "%s: Unhandled commandID: %ld ( 0x % 08lX ).\n", __PRETTY_FUNCTION__, tHICommand.commandID, tHICommand.commandID );
							} else {
								fprintf( stderr, "%s: Unhandled commandID: '%.4s'.\n", __PRETTY_FUNCTION__, &tHICommand.commandID );
							}
							break;
						}
                    }
                    break;
				}
				case kEventCommandUpdateStatus: {
					switch ( tHICommand.commandID )	{
						case kHICommandClose:
						case kHICommandSaveAs:
						case kHICommandPageSetup:
						case kHICommandPrint:
						{
							EnableMenuItem( tHICommand.menu.menuRef, tHICommand.menu.menuItemIndex );
							result = noErr;
							break;
						}
						case kHICommandSave:
						case kHICommandRevert:
						{
							if ( IsWindowModified( tWindowRef ) ) {
								EnableMenuItem( tHICommand.menu.menuRef, tHICommand.menu.menuItemIndex );
							} else {
								DisableMenuItem( tHICommand.menu.menuRef, tHICommand.menu.menuItemIndex );
							}
							result = noErr;
							break;
						}
						case kHICommandUndo:
						case kHICommandRedo:
						{
							if ( FALSE ) {
								EnableMenuItem( tHICommand.menu.menuRef, tHICommand.menu.menuItemIndex );
							} else {
								DisableMenuItem( tHICommand.menu.menuRef, tHICommand.menu.menuItemIndex );
							}
							result = noErr;
							break;
						}
					}		
					// FYI: the Edit menu is handled automatically by MLTE
					break;
				}
				default: {
					fprintf( stderr, "%s: Unhandled event { class: '%.4s', kind: '%.4s' }.\n", __PRETTY_FUNCTION__, &eventClass, &eventKind );
					break;
				}
            }
            break;
        }
        default: {
			fprintf( stderr, "%s: Unhandled event { class: '%.4s', kind: '%.4s' }.\n", __PRETTY_FUNCTION__, &eventClass, &eventKind );
            break;
		}
    }
    return result;
}	// Handle_WindowEvents

#if USE_DRAG_N_DROP
/*****************************************************
*
* Routine:	Handle_DragEvents( inCaller, inEvent, inRefcon ) 
*
* Purpose:	called to process commands from Carbon events
*
* Inputs:	inCaller - reference to the current handler call chain
*			inEvent - the event
*			inRefcon - data you passed in the call to InstallApplicationEventHandler ( our HIViewRef )
*
* Returns:	OSStatus - noErr indicates the event was handled
*						  eventNotHandledErr indicates the event was not handled and the Toolbox should take over
*
*/
static OSStatus Handle_DragEvents( EventHandlerCallRef inCaller, EventRef inEvent, void * inRefcon )
{
	HIViewRef	tHIViewRef = ( HIViewRef ) inRefcon;
	
    OSStatus    result = eventNotHandledErr, status;
	
	dnd_data_ptr dnd_ptr = ( dnd_data_ptr ) GetControlReference( tHIViewRef );
	
	HIRect bounds;
	HIViewGetBounds( tHIViewRef, &bounds );
	
    UInt32 eventClass = GetEventClass( inEvent );
    UInt32 eventKind = GetEventKind( inEvent );
	
    switch ( eventClass ) {
        case kEventClassControl: {
			switch ( eventKind ) {
				case kEventControlDraw: {
					CGContextRef tCGContextRef;
					status = GetEventParameter( inEvent, kEventParamCGContextRef, typeCGContextRef, NULL, sizeof( tCGContextRef ), NULL, &tCGContextRef );
					require_noerr( status, Oops );
					
					if ( dnd_ptr->fDragging ) {
						CGContextSetRGBFillColor( tCGContextRef, 0.875f, 1.0f, 0.875f, 1.0f );
						CGContextFillRect( tCGContextRef, bounds );

						HIRect focusHIRect = CGRectInset( bounds, 2, 2 );
						HIThemeDrawFocusRect( &focusHIRect, TRUE, tCGContextRef, kHIThemeOrientationNormal );
					} else {
						CGContextSetRGBFillColor( tCGContextRef, 1.0f, 1.0f, 0.875f, 1.0f );
						CGContextFillRect( tCGContextRef, bounds );
					}
					result = noErr;
					break;
				}	// case kEventControlDraw:
				case kEventControlDragEnter: {
					printf( "kEventControlDragEnter\n" );
					
					Boolean accept = TRUE;
					status = SetEventParameter( inEvent, kEventParamControlWouldAcceptDrop, typeBoolean, sizeof( accept ), &accept );
					require_noerr( status, Oops );
					
					HIViewSetNeedsDisplay( tHIViewRef, TRUE );
					
					dnd_ptr->fDragging = TRUE;
					result = noErr;
					break;
				}	// case kEventControlDragEnter:
				case kEventControlDragWithin: {
					printf( "kEventControlDragWithin\n" );
					
					// the drag is being moved around within our custom view bounds
					// we determine the location in the view's local coordinate system and
					// ask for a refresh
					DragRef dragRef;
					status = GetEventParameter( inEvent, kEventParamDragRef, typeDragRef, NULL, sizeof( dragRef ), NULL, &dragRef );
					require_noerr( status, Oops );
					
					Point mouse, globalMouse;
					status = GetDragMouse( dragRef, &mouse, &globalMouse );
					require_noerr( status, Oops );

					dnd_ptr->fHIPoint = CGPointMake( globalMouse.h, globalMouse.v );
					HIPointConvert( &dnd_ptr->fHIPoint, kHICoordSpace72DPIGlobal, NULL, kHICoordSpaceView, tHIViewRef );

					if ( CGRectContainsPoint( bounds, dnd_ptr->fHIPoint ) ) {
						// myData->dragSpot = hiPoint;
						HIViewSetNeedsDisplay( tHIViewRef, true );
					}
					result = noErr;
					break;
				}	// case kEventControlDragWithin:
				case kEventControlDragLeave: {
					printf( "kEventControlDragLeave\n" );
					dnd_ptr->fDragging = FALSE;
					HIViewSetNeedsDisplay( tHIViewRef, TRUE );
					result = noErr;
					break;
				}	// case kEventControlDragLeave:
				case kEventControlDragReceive: {
					printf( "kEventControlDragReceive\n" );
					HIViewSetNeedsDisplay( tHIViewRef, TRUE );
					
					DragRef dragRef;
					status = GetEventParameter( inEvent, kEventParamDragRef, typeDragRef, NULL, sizeof( dragRef ), NULL, &dragRef );
					require_noerr( status, Oops );
					
					PasteboardRef pasteBoard;
					status = GetDragPasteboard( dragRef, &pasteBoard );
					require_noerr( status, Oops );
					
					// how many items in the pasteboard?
					ItemCount i, itemCount;
					status = PasteboardGetItemCount( pasteBoard, &itemCount );
					require_noerr( status, Oops );
					
					// create an empty list
					AEDescList itemAEDescList;
					status = AECreateList( NULL, 0, FALSE, &itemAEDescList );
					require_noerr( status, Oops );

					// let's loop on the items...
					for ( i = 1 ; i <= itemCount ; i++ ) {
						PasteboardItemID itemID;
						CFArrayRef flavorTypeArray = NULL;
						CFIndex j, flavorCount = 0;
						
						status = PasteboardGetItemIdentifier( pasteBoard, i, &itemID );
						require_noerr( status, Oops );
						
						// how many flavors in this item?
						status = PasteboardCopyItemFlavors( pasteBoard, itemID, &flavorTypeArray );
						require_noerr( status, Oops );
						
						if ( flavorTypeArray ) {
							flavorCount = CFArrayGetCount( flavorTypeArray );
							
							// let's loop on the flavors
							for( j = 0;  j < flavorCount ; j++ ) {
								CFDataRef flavorData;
								CFStringRef flavorType = ( CFStringRef )CFArrayGetValueAtIndex( flavorTypeArray, j );
								require_orelse_continue( flavorType );
								
								// CFShow( flavorType );
								require_orelse_continue(  UTTypeConformsTo( flavorType, kUTTypeFileURL )  ); // this is a file
								require_orelse_continue( PasteboardCopyItemFlavorData( pasteBoard, itemID, flavorType, &flavorData ) == noErr );
								
								CFIndex flavorDataSize = CFDataGetLength( flavorData );
								// CFShow( flavorData );
								CFURLRef tCFURLRef = CFURLCreateWithBytes( kCFAllocatorDefault, CFDataGetBytePtr( flavorData ), flavorDataSize, kCFStringEncodingMacRoman, NULL );
								CFRelease( flavorData );
								if ( tCFURLRef ) {
									FSRef tFSRef;
									if ( CFURLGetFSRef( tCFURLRef, &tFSRef ) ) {
										// Add this FSRef to the end of our list
										status = AEPutPtr( &itemAEDescList, 0, typeFSRef, &tFSRef, sizeof( FSRef ) );
									}
									CFRelease( tCFURLRef );
								}
							}
							CFRelease( flavorTypeArray );
						}
					}
					status = Do_OpenDocs( itemAEDescList );
					AEDisposeDesc( &itemAEDescList );

					result = noErr;
					break;
				}	// case kEventControlDragReceive:
					
				default: {
					fprintf( stderr, "%s: Unhandled event { class: '%.4s', kind: '%.4s' }.\n", __PRETTY_FUNCTION__, &eventClass, &eventKind );
					break;
				}
			}
			break;
        }	// case kEventClassControl:
		default: {
			fprintf( stderr, "%s: Unhandled event { class: '%.4s', kind: '%.4s' }.\n", __PRETTY_FUNCTION__, &eventClass, &eventKind );
			break;
		}	// default:
    }	// switch ( eventClass )
Oops:	;
	return result;
}	// Handle_DragEvents
#endif USE_DRAG_N_DROP

/*****************************************************/
#pragma mark -
#pragma mark * Preferences *
//----------------------------------------------------

/*****************************************************
*
* Routine:	Handle_NewPrefWindow( void ) 
*
* Purpose:	routine to display dialog to set our applications preferences
*
* Inputs:	none
*
* Returns:	none
*
*/
static void Handle_NewPrefWindow( void )
{
	// If the Preferences window is already open then just select it to make it front else
	// create a window. "PrefWindow" is the name of the window object. This name is set in 
	// InterfaceBuilder when the nib is created.
	if ( gPreferencesWindow ) {
		SelectWindow( gPreferencesWindow );
		return;
	}
	
	OSStatus status = CreateWindowFromNib( gMainIBNibRef, CFSTR( "PrefWindow" ), &gPreferencesWindow );
	require_noerr( status, Oops );
	
	EventTypeSpec eventType1[] = {{kEventClassWindow, kEventWindowClose}};
	status = InstallWindowEventHandler( gPreferencesWindow, Handle_PrefWindowIsAboutToClose, GetEventTypeCount( eventType1 ), eventType1, NULL, NULL );
	require_noerr( status, Oops );
	
	EventTypeSpec eventType2[] = {{kEventClassCommand, kEventCommandProcess}};
	status = InstallWindowEventHandler( gPreferencesWindow, Handle_PrefCommandProcess, GetEventTypeCount( eventType2 ), eventType2, NULL, NULL );
	require_noerr( status, Oops );
	
	HIViewRef tHIViewRef;

	status = HIViewFindByID( HIViewGetRoot( gPreferencesWindow ), kOpenFoldersHID, &tHIViewRef );
	require_noerr( status, Oops );
	SetControl32BitValue( tHIViewRef, gOpenFolderContents ? 1 : 0 );
	
	status = HIViewFindByID( HIViewGetRoot( gPreferencesWindow ), kOpenFoldersRecursiveHID, &tHIViewRef );
	require_noerr( status, Oops );
	SetControl32BitValue( tHIViewRef, gOpenFolderRecursive ? 1 : 0 );
	if ( gOpenFolderContents ) EnableControl( tHIViewRef ); else DisableControl( tHIViewRef );
	
	status = HIViewFindByID( HIViewGetRoot( gPreferencesWindow ), kResolveAliasesHID, &tHIViewRef );
	require_noerr( status, Oops );
	SetControl32BitValue( tHIViewRef, gResolveAliases ? 1 : 0 );
	
	status = HIViewFindByID( HIViewGetRoot( gPreferencesWindow ), kResolveAliasesChainsHID, &tHIViewRef );
	require_noerr( status, Oops );
	SetControl32BitValue( tHIViewRef, gResolveAliasesChains ? 1 : 0 );
	if ( gResolveAliases ) EnableControl( tHIViewRef ); else DisableControl( tHIViewRef );
	
	ShowWindow( gPreferencesWindow );
	
Oops:	;
}	// Handle_NewPrefWindow

/*****************************************************
*
* Routine:	Handle_PrefCommandProcess( inHandlerCallRef, inEvent, inUserData ) 
*
* Purpose:	called to process commands from the preferences window check boxes
*
* Inputs:	inHandlerCallRef - reference to the current handler call chain
*			inEvent - the event
*			inUserData - app - specified data you passed in the call to InstallWindowEventHandler ( NULL )
*
* Returns:	OSStatus - noErr indicates the event was handled
*								 eventNotHandledErr indicates the event was not handled and the Toolbox should take over
*
*/
static pascal OSStatus Handle_PrefCommandProcess( EventHandlerCallRef inHandlerCallRef, EventRef inEvent, void * inUserData )
{
	OSStatus result = noErr;
	HIViewRef tHIViewRef;
	HICommand aCommand; 
	GetEventParameter( inEvent, kEventParamDirectObject, typeHICommand, NULL, sizeof( HICommand ), NULL, &aCommand );
	
	switch ( aCommand.commandID ) {
		case kOpenFoldersHICommand:
			gOpenFolderContents = !gOpenFolderContents;
			result = HIViewFindByID( HIViewGetRoot( gPreferencesWindow ), kOpenFoldersRecursiveHID, &tHIViewRef );
			if ( noErr != result ) break;
				if ( gOpenFolderContents ) EnableControl( tHIViewRef ); else DisableControl( tHIViewRef );
			break;
		case kOpenFoldersRecursiveHICommand:
			gOpenFolderRecursive = !gOpenFolderRecursive;
			break;
		case kResolveAliasesHICommand:
			gResolveAliases = !gResolveAliases;
			result = HIViewFindByID( HIViewGetRoot( gPreferencesWindow ), kResolveAliasesChainsHID, &tHIViewRef );
			if ( noErr != result ) break;
				if ( gResolveAliases ) EnableControl( tHIViewRef ); else DisableControl( tHIViewRef );
			break;
		case kResolveAliasesChainsHICommand:
			gResolveAliasesChains = !gResolveAliasesChains;
			break;
		default:
			result = eventNotHandledErr;
			break;
	}
	if ( noErr == result ) {
		( void ) Set_Preferences( );
	}
	
	return result;
}	// Handle_PrefCommandProcess

/*****************************************************
*
* Routine:	Handle_PrefWindowIsAboutToClose( inHandlerCallRef, inEvent, inUserData ) 
*
* Purpose:	called as notification that the preferences window is going to close
*
* Inputs:	inHandlerCallRef - reference to the current handler call chain
*			inEvent - the event
*			inUserData - app - specified data you passed in the call to InstallWindowEventHandler ( NULL )
*
* Returns:	OSStatus - noErr indicates the event was handled
*								 eventNotHandledErr indicates the event was not handled and the Toolbox should take over
*
*/
static pascal OSStatus Handle_PrefWindowIsAboutToClose( EventHandlerCallRef inHandlerCallRef, EventRef inEvent, void * inUserData )
{
	gPreferencesWindow = NULL;
	
	// by returning eventNotHandledErr, we continue with the normal closing of the window
	return eventNotHandledErr;
}	// Handle_PrefWindowIsAboutToClose
/*****************************************************
*
* Routine:	Get_Preferences( ) 
*
* Purpose:	get's the users preferences
*
* Inputs:	none
*
* Returns:	none
*
*/
static void Get_Preferences( void )
{
	Boolean keyExistsAndHasValidFormat, tBoolean;
	
	tBoolean = CFPreferencesGetAppBooleanValue( kOpenFolderContentsPrefKey, kCFPreferencesCurrentApplication, &keyExistsAndHasValidFormat );
	if ( keyExistsAndHasValidFormat ) {
		gOpenFolderContents = tBoolean;
	}
	
	tBoolean = CFPreferencesGetAppBooleanValue( kOpenFolderRecursivePrefKey, kCFPreferencesCurrentApplication, &keyExistsAndHasValidFormat );
	if ( keyExistsAndHasValidFormat ) {
		gOpenFolderRecursive = tBoolean;
	}
	
	tBoolean = CFPreferencesGetAppBooleanValue( kResolveAliasesContentsPrefKey, kCFPreferencesCurrentApplication, &keyExistsAndHasValidFormat );
	if ( keyExistsAndHasValidFormat ) {
		gResolveAliases = tBoolean;
	}
	
	tBoolean = CFPreferencesGetAppBooleanValue( kResolveAliasesChainsPrefKey, kCFPreferencesCurrentApplication, &keyExistsAndHasValidFormat );
	if ( keyExistsAndHasValidFormat ) {
		gResolveAliasesChains = tBoolean;
	}
} // Get_Preferences
/*****************************************************
*
* Routine:	Set_Preferences( ) 
*
* Purpose:	Set's the users preferences
*
* Inputs:	none
*
* Returns:	OSErr - error code ( 0 == no error ) 
*
*/
static OSStatus Set_Preferences( void )
{
	OSStatus result = coreFoundationUnknownErr;
	CFPreferencesSetAppValue( kOpenFolderContentsPrefKey, gOpenFolderContents ? kCFBooleanTrue : kCFBooleanFalse, kCFPreferencesCurrentApplication );
	CFPreferencesSetAppValue( kOpenFolderRecursivePrefKey, gOpenFolderRecursive ? kCFBooleanTrue : kCFBooleanFalse, kCFPreferencesCurrentApplication );
	CFPreferencesSetAppValue( kResolveAliasesContentsPrefKey, gResolveAliases ? kCFBooleanTrue : kCFBooleanFalse, kCFPreferencesCurrentApplication );
	CFPreferencesSetAppValue( kResolveAliasesChainsPrefKey, gResolveAliasesChains ? kCFBooleanTrue : kCFBooleanFalse, kCFPreferencesCurrentApplication );

	// sync to disk
	if ( CFPreferencesAppSynchronize( kCFPreferencesCurrentApplication ) ) {
		result = noErr;
	}
	return result;
}

//****************************************************
#pragma mark -
#pragma mark * open document routines *
//----------------------------------------------------

/*****************************************************
*
* Routine:	Do_OpenDocs( inDocumentsList ) 
*
* Purpose:	open docs in inDocumentsList
*
* Notes:	called by Do_OpenWindows( ) ( "File / Open" menu item ) & Handle_OpenDocuments( ) ( 'odoc' AppleEvent ) 
*
* Inputs:	inDocumentsList - list of AEObjects ( files ) 
*
* Returns:	OSStatus - error code ( 0 == no error ) 
*
*/
static OSStatus Do_OpenDocs( AEDescList inDocumentsList )
{
	long	firstCount;
	OSStatus result = AECountItems( &inDocumentsList, &firstCount );
	require_noerr( result, Oops );
	
	long idx, cnt = firstCount;
	for ( idx = 1; idx <= cnt; idx++ ) {
		FSRef tFSRef;
		
		result = AEGetNthPtr( &inDocumentsList, idx, typeFSRef, NULL, NULL, &tFSRef, sizeof( FSRef ), NULL );
		require_orelse_continue( noErr == result );

		Boolean aliasFlag = FALSE;
		Boolean appFlag = FALSE;
		Boolean containerFlag = FALSE;

		do {
			LSItemInfoRecord tLSItemInfoRecord;
			result = LSCopyItemInfoForRef( &tFSRef, kLSRequestAllFlags, &tLSItemInfoRecord );
			require_orelse_continue( noErr == result );

			aliasFlag = appFlag = containerFlag = FALSE;

			if ( 0 == ( kLSItemInfoIsInvisible & tLSItemInfoRecord.flags ) ) {	// not invisible
				if ( kLSItemInfoIsAliasFile & tLSItemInfoRecord.flags ) {	// is an alias file
					if ( gResolveAliases ) {
						result = FSResolveAliasFileWithMountFlags( &tFSRef, gResolveAliasesChains, &containerFlag, &aliasFlag, kResolveAliasFileNoUI );
						if ( noErr != result ) break;
						// TO-DO: test if tFSRef is container above the orginal alias file? ( to prevent infinite recurision )
						aliasFlag = TRUE;
					}
				} else if ( kLSItemInfoIsApplication & tLSItemInfoRecord.flags ) {	// is an application
					appFlag = TRUE;
				} else if ( kLSItemInfoIsContainer & tLSItemInfoRecord.flags ) {	// is a container
					containerFlag = TRUE;
				}
			}
		} while ( gResolveAliases && aliasFlag );

#if TRUE // set true to printf items as we iterate over them
		HFSUniStr255 tHFSUniStr255;
		FSGetCatalogInfo( &tFSRef, kFSCatInfoNone, NULL, &tHFSUniStr255, NULL, NULL );

		CFStringRef tCFStringRef = CFStringCreateWithCharacters( kCFAllocatorDefault, tHFSUniStr255.unicode, tHFSUniStr255.length );
		if ( tCFStringRef ) {
			char buffer[256];
			CFStringGetCString( tCFStringRef, buffer, 256, kCFStringEncodingMacRoman );

			if ( appFlag ) {
				printf( "#%6d of #%6d, App: <%s> .\n", idx, cnt, buffer );
			} else if ( containerFlag ) {
				printf( "#%6d of #%6d, Folder: <%s> .\n", idx, cnt, buffer );
			} else {
				printf( "#%6d of #%6d, File: <%s> .\n", idx, cnt, buffer );
			}
			CFRelease( tCFStringRef );
		}
#endif		
		
		if ( appFlag ) {
			result = RecentItems_Update( kRecentAppsPrefKey, &tFSRef, idx == cnt );
			require_orelse_continue( noErr == result );
		} else if ( containerFlag ) {
			result = RecentItems_Update( kRecentFoldersPrefKey, &tFSRef, idx == cnt );
			require_orelse_continue( noErr == result );
			
			if ( gOpenFolderContents ) {
				if ( gOpenFolderRecursive || ( idx <= firstCount ) ) {
					Append_ContainerItemsToAEDescList( &tFSRef, inDocumentsList );
					result = AECountItems( &inDocumentsList, &cnt );
					require_orelse_continue( noErr == result );
				}
			}
		} else {
			result = RecentItems_Update( kRecentDocsPrefKey, &tFSRef, idx == cnt );
			require_orelse_continue( noErr == result );
		}
	}

	// after we've updated all our prefs we repopulate our menus ( Ignore errors )
	( void ) RecentItems_PopulateMenu( gRecentAppsMenuRef, kRecentAppsPrefKey, kRecentAppsMenuCMD );
	( void ) RecentItems_PopulateMenu( gRecentFoldersMenuRef, kRecentFoldersPrefKey, kRecentFoldersMenuCMD );
	( void ) RecentItems_PopulateMenu( gRecentDocsMenuRef, kRecentDocsPrefKey, kRecentDocsMenuCMD );
	
Oops:	;
	return result;
}	// Do_OpenDocs

/*****************************************************
*
* Routine:	Append_ContainerItemsToAEDescList( inFSRef, inDocumentsList ) 
*
* Purpose:	append all container items to AE desciptor list
*
* Inputs:	inFSRef - FSRef to container
*			inDocumentsList - AEDescList to append container items to
*
* Returns:	none
*
*/
static void Append_ContainerItemsToAEDescList( const FSRef * inFSRef, AEDescList inDocumentsList )
{
	long listCount;
	OSStatus status = AECountItems( &inDocumentsList, &listCount );
	require_noerr( status, Oops );
	
	FSRef * * tFSRefHdl;
	ItemCount idx, cnt;
	Boolean containerChanged;
	status = Get_ContainerItems( inFSRef, &tFSRefHdl, &cnt, &containerChanged );
	require_noerr( status, Oops );

	for ( idx = 0; idx < cnt; idx++ ) {
		FSRef tFSRef = ( *tFSRefHdl )[idx];
		( void ) AEPutPtr( &inDocumentsList, ++listCount, typeFSRef, &tFSRef, sizeof( FSRef ) );
	}
Oops:	;
	return;
}	// Append_ContainerItemsToAEDescList

/*****************************************************
*
* Routine:	Get_ContainerItems( inContainerFSRef, outFSRefHandle, outNumRefs, outContainerChanged ) 
*
* Purpose:	create a handle of FSRef's for all the items in the provided container FSRef
*
* Inputs:	inContainerFSRef - FSRef for the container
*			outFSRefHandle - address of handle of array of FSRef's
*			outNumRefs - number of FSRef's in output array ( handle )
*			outContainerChanged - Boolean, TRUE if container changes while being iterated
*
* Returns:	OSErr - error code ( 0 == no error ) 
*
*/
static OSErr Get_ContainerItems( const FSRef * inContainerFSRef, FSRef * **outFSRefHandle, ItemCount * outNumRefs, Boolean * outContainerChanged )
{
	// Grab items 16 at a time.
	enum { kMaxItemsPerBulkCall = 16 };
	
	OSErr		result;
	OSErr		memResult;
	FSIterator	iterator;
	FSRef		refs[kMaxItemsPerBulkCall];
	ItemCount	actualObjects;
	Boolean		changed;
	
	// check parameters
	require_action( ( outFSRefHandle ) && ( outNumRefs ) && ( outContainerChanged ), Oops, result = paramErr );
	
	*outNumRefs = 0;
	*outContainerChanged = FALSE;
	*outFSRefHandle = ( FSRef * * ) NewHandle( 0 );
	require_action( *outFSRefHandle, Oops, result = memFullErr );
	
	// open an FSIterator
	result = FSOpenIterator( inContainerFSRef, kFSIterateFlat, &iterator );
	require_noerr( result, Oops );
	
	// Call FSGetCatalogInfoBulk in loop to get all items in the container
	do {
		result = FSGetCatalogInfoBulk( iterator, kMaxItemsPerBulkCall, &actualObjects, 									   &changed, kFSCatInfoNone, NULL, refs, NULL, NULL );
		
		// if the container changed, set outContainerChanged for output, but keep going
		if ( changed ) {
			*outContainerChanged = changed;
		}
		
		// any result other than noErr and errFSNoMoreItems is serious
		require( ( noErr == result ) || ( errFSNoMoreItems == result ), Oops );
		
		// add objects to output array and count
		if ( actualObjects ) {
			// concatenate the FSRefs to the end of the	 handle
			PtrAndHand( refs, ( Handle )*outFSRefHandle, actualObjects * sizeof( FSRef ) );
			memResult = MemError( );
			require_noerr_action( memResult, Oops, result = memResult );
			
			*outNumRefs += actualObjects;
		}
	} while ( noErr == result );
	
	verify_noerr( FSCloseIterator( iterator ) ); // closing an open iterator should never fail, but...
	
	return ( noErr );
	
	//===============================================
	
Oops:	;
	
	// close the iterator
	verify_noerr( FSCloseIterator( iterator ) );
	
	// dispose of handle if already allocated and clear the outputs
	if ( *outFSRefHandle ) {
		DisposeHandle( ( Handle )*outFSRefHandle );
		*outFSRefHandle = NULL;
	}
	*outNumRefs = 0;
	
	return result;
}	// Get_ContainerItems
