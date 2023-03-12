/*
	File:		main.cp

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

	Copyright © 2003 Apple Computer, Inc., All Rights Reserved
*/

#include "TCarbonEvent.h"
#include "GridMenu.h"
#include "VolumeMenu.h"
#include "LabelItemView.h"


#define kLabelViewID				CFSTR("com.apple.sample.MenuViews.LabelView")
#define kLabelToolbarItemClassID	CFSTR("com.apple.sample.MenuViews.LabelToolbarItem")


static void				SetupWindowToolbar( WindowRef inWindow );
static pascal OSStatus	ToolbarHandler( EventHandlerCallRef inCaller, EventRef inEvent, void* inRefcon );
static OSStatus			CreateLabelToolbarItem( HIToolbarItemRef* outItem );
static pascal OSStatus	LabelToolbarItemHandler( EventHandlerCallRef inCaller, EventRef inEvent, void* inRefcon );
static void				AddMenuItems( MenuRef menu );
static void				CreateSubmenus( MenuRef topMenu, int count );
static OSStatus			WindowEventHandler( EventHandlerCallRef caller, EventRef event, void* refcon );
static OSStatus			PictureHandler( EventHandlerCallRef caller, EventRef event, void* refcon );


static const EventTypeSpec	kWindowEvents[] =
							{
								{ kEventClassWindow, kEventWindowContextualMenuSelect }
							};

static const EventTypeSpec	kPictureEvents[] =
							{
								{ kEventClassControl, kEventControlOwningWindowChanged }
							};

static MenuRef	sFileInfoMenu;


//---------------------------------------------------------------------------------------------------------
//	• main
//	main function for the MenuViews sample application.
//---------------------------------------------------------------------------------------------------------
int main(int argc, char* argv[])
{
    IBNibRef 			nibRef;
    WindowRef 			window;
    OSStatus			err;
	MenuRef				menu;
	RGBColor			minRGB = { 0, 0, 0 };
	RGBColor			maxRGB = { 0xFFFF, 0xFFFF, 0xFFFF };
	ControlID			ctlID = { 'POPB', 0 };
	ControlRef			ctl;
	HIViewRef			view;
	HIViewRef			subview;
	
    // Create a Nib reference passing the name of the nib file (without the .nib extension)
    // CreateNibReference only searches into the application bundle.
    err = CreateNibReference(CFSTR("main"), &nibRef);
    require_noerr( err, CantGetNibRef );
    
    // Once the nib reference is created, set the menu bar. "MainMenu" is the name of the menu bar
    // object. This name is set in InterfaceBuilder when the nib is created.
    err = SetMenuBarFromNib(nibRef, CFSTR("MenuBar"));
    require_noerr( err, CantSetMenuBar );
    
    // Then create a window. "MainWindow" is the name of the window object. This name is set in 
    // InterfaceBuilder when the nib is created.
    err = CreateWindowFromNib(nibRef, CFSTR("MainWindow"), &window);
    require_noerr( err, CantCreateWindow );

	GetControlByID( window, &ctlID, &ctl );
	SetControlMinimum( ctl, 0 );
	SetControlValue( ctl, 0 );
	
	InstallWindowEventHandler( window, WindowEventHandler, GetEventTypeCount( kWindowEvents ), kWindowEvents, window, NULL );
	
	SetupWindowToolbar( window );
	
    // The window was created hidden so show it.
    ShowWindow( window );
    
	CreateNewMenu( 201, 0, &menu );
	SetMenuTitleWithCFString( menu, CFSTR("Standard") );
	AddMenuItems( menu );
	InsertMenu( menu, 0 );

	// create a menu that will have a picture behind the items
	CreateNewMenu( 199, 0, &menu );
	SetMenuTitleWithCFString( menu, CFSTR("Picture") );
	AddMenuItems( menu );
	InsertMenu( menu, 0 );
	
	// demonstrate placing a picture behind the items of a standard menu
	HIMenuGetContentView( menu, kThemeMenuTypePullDown, &view );
	InstallControlEventHandler( view, PictureHandler, GetEventTypeCount( kPictureEvents ), kPictureEvents, 0, NULL );
	
	// create a menu that will have a label view embedded in it
	CreateMenuFromNib( nibRef, CFSTR("FileMenu"), &sFileInfoMenu );
	InsertMenu( sFileInfoMenu, 0 );
	
	// demonstrate placing a custom menu item view in the menu
	HIMenuGetContentView( sFileInfoMenu, kThemeMenuTypePullDown, &view );
	HILabelViewCreate( sFileInfoMenu, CountMenuItems( sFileInfoMenu ) + 1, view, &subview );
	HIViewSetVisible( subview, true );
	
	if ( GridMenuCreate( 202, 20, 20, minRGB, maxRGB, &menu ) == noErr )
	{
		SetMenuTitleWithCFString( menu, CFSTR("ColorGrid") );
		InsertMenu( menu, 0 );
	}

	if ( VolumeMenuCreate( &menu ) == noErr )
		InsertMenu( menu, 0 );
	
	DrawMenuBar();

    // Call the event loop
    RunApplicationEventLoop();

CantCreateWindow:
CantSetMenuBar:
CantGetNibRef:
	return err;
}

//---------------------------------------------------------------------------------------------------------
//	• SetupWindowToolbar
//	Creates a toolbar for our sample window.
//---------------------------------------------------------------------------------------------------------
static void
SetupWindowToolbar( WindowRef inWindow )
{
	HIToolbarRef				tb;
	static const EventTypeSpec	kTBEvents[] =
								{
									{ kEventClassToolbar, kEventToolbarGetDefaultIdentifiers },
									{ kEventClassToolbar, kEventToolbarGetAllowedIdentifiers },
									{ kEventClassToolbar, kEventToolbarCreateItemWithIdentifier }
								};
	
	HIToolbarCreate( CFSTR("com.apple.sample.MenuViews.Toolbar"), kHIToolbarNoAttributes, &tb );
	SetWindowToolbar( inWindow, tb );
	InstallEventHandler( HIObjectGetEventTarget( tb ), ToolbarHandler, GetEventTypeCount( kTBEvents ), kTBEvents, 0, NULL );
}

//---------------------------------------------------------------------------------------------------------
//	• ToolbarHandler
//	Event handler for our window's toolbar.
//---------------------------------------------------------------------------------------------------------
static pascal OSStatus
ToolbarHandler( EventHandlerCallRef inCaller, EventRef inEvent, void* inRefcon )
{
	OSStatus	result = eventNotHandledErr;
	
	switch ( GetEventKind( inEvent ) )
	{
		case kEventToolbarGetDefaultIdentifiers:
		case kEventToolbarGetAllowedIdentifiers:
		{
			TCarbonEvent		event( inEvent );
			CFMutableArrayRef	array;
			
			event.GetParameter<CFMutableArrayRef>( kEventParamMutableArray, typeCFMutableArrayRef, &array );
			CFArrayAppendValue( array, kLabelViewID );
			result = noErr;
			break;
		}
			
		case kEventToolbarCreateItemWithIdentifier:
		{
			TCarbonEvent	event( inEvent );
			CFStringRef		itemID;
			
			event.GetParameter<CFStringRef>( kEventParamToolbarItemIdentifier, typeCFStringRef, &itemID );
			if ( CFStringCompare( itemID, kLabelViewID, 0 ) == kCFCompareEqualTo )
			{
				HIToolbarItemRef item;
				result = CreateLabelToolbarItem( &item );
				if ( result == noErr )
					event.SetParameter<HIToolbarItemRef>( kEventParamToolbarItem, typeHIToolbarItemRef, item );
			}
			else
			{
				result = eventNotHandledErr;
			}
			break;
		}
			
		default:
			break;
	}
	
	return result;
}

//---------------------------------------------------------------------------------------------------------
//	• CreateLabelToolbarItem
//	Creates a toolbar item for the label view.
//---------------------------------------------------------------------------------------------------------
static OSStatus
CreateLabelToolbarItem( HIToolbarItemRef* outItem )
{
	static const EventTypeSpec	kLabelItemEvents[] =
								{
									{ kEventClassHIObject, kEventHIObjectConstruct },
									{ kEventClassHIObject, kEventHIObjectDestruct },
									{ kEventClassToolbarItem, kEventToolbarItemCreateCustomView }
								};
								
	static Boolean				sRegistered;
	
	if ( !sRegistered )
	{
		HIObjectRegisterSubclass( kLabelToolbarItemClassID, kHIToolbarItemClassID, 0,
								  LabelToolbarItemHandler, GetEventTypeCount( kLabelItemEvents ),
								  kLabelItemEvents, 0, NULL );
		sRegistered = true;
	}
	
	EventRef event;
	if ( CreateEvent( NULL, kEventClassHIObject, kEventHIObjectInitialize, 0, 0, &event ) == noErr )
	{
		CFStringRef itemID = kLabelToolbarItemClassID;
		OptionBits	options = 0;
		
		SetEventParameter( event, kEventParamToolbarItemIdentifier, typeCFStringRef, sizeof( CFStringRef ), &itemID );
		SetEventParameter( event, kEventParamAttributes, typeUInt32, sizeof( UInt32 ), &options );
		return HIObjectCreate( itemID, event, (HIObjectRef*) outItem );
	}
	
	return memFullErr;
}

//---------------------------------------------------------------------------------------------------------
//	• LabelToolbarItemHandler
//	Event handler for label toolbar items.
//---------------------------------------------------------------------------------------------------------
static pascal OSStatus
LabelToolbarItemHandler( EventHandlerCallRef inCaller, EventRef inEvent, void* inRefcon )
{
	TCarbonEvent	event( inEvent );
	OSStatus		result = eventNotHandledErr;
	
	switch ( event.GetClass() )
	{
		case kEventClassHIObject:
			switch ( event.GetKind() )
			{
				case kEventHIObjectConstruct:
				{
					HIObjectRef super;
					// we don't currently allocate any data for our toolbar item; just use the input HIObject as our view data
					event.GetParameter<HIObjectRef>( kEventParamHIObjectInstance, typeHIObjectRef, &super );
					event.SetParameter<HIObjectRef>( kEventParamHIObjectInstance, typeVoidPtr, super );
					result = noErr;
					break;
				}
					
				case kEventHIObjectDestruct:
					// we don't currently have any data to free for our toolbar item
					result = noErr;
					break;
			}
			break;
			
		case kEventClassToolbarItem:
			switch ( event.GetKind() )
			{
				case kEventToolbarItemCreateCustomView:
				{
					HIViewRef view;
					HILabelViewCreate( NULL, NULL, NULL, &view );
					HIViewSetVisible( view, true );
					event.SetParameter( kEventParamControlRef, view );
					result = noErr;
					break;
				}
			}
			break;
			
		default:
			break;
	}
	
	return result;
}

//---------------------------------------------------------------------------------------------------------
//	• AddMenuItems
//	Adds some sample menu items to a menu.
//---------------------------------------------------------------------------------------------------------
static void AddMenuItems( MenuRef menu )
{
	static MenuRef sampleMenu;
	if ( sampleMenu == NULL )
	{
		IBNibRef		nibRef;
		MenuItemIndex	i;
		short			fid;
		
		CreateNibReference(CFSTR("main"), &nibRef);
		CreateMenuFromNib( nibRef, CFSTR("SampleMenu"), &sampleMenu );
		DisposeNibReference( nibRef );
		
		// set item features that IB doesn't support
		SetItemMark( sampleMenu, 1, '•' );
		SetItemMark( sampleMenu, 3, '-' );
		SetItemMark( sampleMenu, 4, 'a' );
		for ( i = 20; i <= 27; i++ )
		{
			UInt8	modifiers;
			GetMenuItemModifiers( sampleMenu, i, &modifiers );
			SetMenuItemModifiers( sampleMenu, i, modifiers | kMenuNoCommandModifier );
		}
		
		GetFNum( "\pCourier", &fid );
		SetMenuItemFontID( sampleMenu, 1, fid );
	}
	
	CopyMenuItems( sampleMenu, 1, CountMenuItems( sampleMenu ), menu, CountMenuItems( menu ) );
	
	CreateSubmenus( menu, 6 );
}

//---------------------------------------------------------------------------------------------------------
//	• CreateSubmenus
//	Creates some sample submenus.
//---------------------------------------------------------------------------------------------------------
static void CreateSubmenus( MenuRef topMenu, int count )
{
	int		i;
	MenuRef	menu = topMenu;
	
	for ( i = 1; i <= count; i++ )
	{
		MenuRef submenu;
		
		DuplicateMenu( topMenu, &submenu );
		SetMenuItemHierarchicalMenu( menu, 4, submenu );
		menu = submenu;
	}
	
	// clear the submenu link in the bottom-most menu (which otherwise will point to the first submenu)
	SetMenuItemHierarchicalMenu( menu, 4, NULL );
}

//---------------------------------------------------------------------------------------------------------
//	• WindowEventHandler
//	Handles window-specific events.
//---------------------------------------------------------------------------------------------------------
static OSStatus
WindowEventHandler( EventHandlerCallRef caller, EventRef event, void* refcon )
{
	switch ( GetEventClass( event ) )
	{
		case kEventClassWindow:
			switch ( GetEventKind( event ) )
			{
				case kEventWindowContextualMenuSelect:
				{
					Point			mouse;
					UInt32			selType;
					MenuID			menuID;
					MenuItemIndex	item;
					
					GetEventParameter( event, kEventParamMouseLocation, typeQDPoint, NULL, sizeof( mouse ), NULL, &mouse );
					ContextualMenuSelect( sFileInfoMenu, mouse, false, kCMHelpItemNoHelp, NULL, NULL, &selType, &menuID, &item );

					return noErr;
				}
			}
			break;
		
		default:
			break;
	}
	
	return eventNotHandledErr;
}

//---------------------------------------------------------------------------------------------------------
//	• PictureHandler
//	An event handler that installs an image view behind the items view of a standard menu.
//---------------------------------------------------------------------------------------------------------
static OSStatus
PictureHandler( EventHandlerCallRef caller, EventRef event, void* refcon )
{
	OSStatus	err = eventNotHandledErr;
	WindowRef	owner;
	
	GetEventParameter( event, kEventParamControlCurrentOwningWindow, typeWindowRef, NULL, sizeof( owner ), NULL, &owner );
	if ( owner != NULL )
	{
		// find the content view
		HIViewRef content;
		HIViewFindByID( HIViewGetRoot( owner ), kHIViewWindowContentID, &content );
		
		// create a data provider for our image
		CFURLRef url = CFBundleCopyResourceURL( CFBundleGetMainBundle(), CFSTR("GoldenGate"), CFSTR("jpg"), NULL );
		CGDataProviderRef data = CGDataProviderCreateWithURL( url );
		CFRelease( url );
		
		// create our image
		CGImageRef image = CGImageCreateWithJPEGDataProvider( data, NULL, true, kCGRenderingIntentDefault );
		CFRelease( data );
		
		// create our image view
		HIViewRef imageView;
		HIImageViewCreate( image, &imageView );
		HIImageViewSetOpaque( imageView, false );
		HIImageViewSetAlpha( imageView, 0.3 );
		CFRelease( image );
		
		// position our image view at the bottom of the content view's children
		HIViewAddSubview( content, imageView );
		HIViewSetZOrder( imageView, kHIViewZOrderBelow, NULL );
		HIViewSetVisible( imageView, true );
		
		// size our image view to match the content view
		HIRect bounds;
		HIViewGetBounds( content, &bounds );
		HIViewSetFrame( imageView, &bounds );
	}
	return err;
}

