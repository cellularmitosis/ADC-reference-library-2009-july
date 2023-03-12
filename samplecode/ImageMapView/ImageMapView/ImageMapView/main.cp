/*
	File:		main.cp
	
	Project:	ImageMapView
	
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

	Copyright © 2005 Apple Computer, Inc., All Rights Reserved
*/

#include <Carbon/Carbon.h>

#include "ImageMapView.h"
#include "ImageMap.h"

// -----------------------------------------------------------------------------
//	constants
// -----------------------------------------------------------------------------
//
enum
{
	kImageMapAndLabelGroupingPane	= 'Grpr',
	kImageMapView					= 'IMVi',
	kImageMapLabel					= 'IMLa',
	kModeSelector					= 'Sele',
	kSelectorLabel					= 'SelL',

	kCommandModeInvisibleHotSpots	= 'HSIn',
	kCommandModeVisibleHotSpots		= 'HSVi',
	kCommandModeRollovereHotSpots	= 'HSRo',
};

const ControlID	kImageMapAndLabelGroupingPaneID = { kImageMapAndLabelGroupingPane, 0 };
const ControlID	kImageMapViewID = { kImageMapView, 0 };
const ControlID	kModeSelectorID = { kModeSelector, 0 };
const ControlID	kSelectorLabelID = { kSelectorLabel, 0 };
const ControlID	kImageMapLabelID = { kImageMapLabel, 0 };

const int kSelectorCount = 3;

const CFStringRef kFoodPyramidWindow = CFSTR( "FoodPyramidWindow" );
const CFStringRef kShapesWindow = CFSTR( "ShapesWindow" );

// -----------------------------------------------------------------------------
//	prototypes
// -----------------------------------------------------------------------------
//
OSStatus	AppEventHandler( EventHandlerCallRef inHandlerCallRef, EventRef inEvent, void* inUserData );
OSStatus	WindowEventHandler( EventHandlerCallRef inHandlerCallRef, EventRef inEvent, void* inUserData );
OSStatus	CreateWindow( CFStringRef inWindowName );

DEFINE_ONE_SHOT_HANDLER_GETTER( AppEventHandler );
DEFINE_ONE_SHOT_HANDLER_GETTER( WindowEventHandler );

CGImageRef	GetSelectorImage( int inIndex );
void		ChangeHotSpots( WindowRef inWindow, ImageMapHotSpots inHotSpotsKind );

// -----------------------------------------------------------------------------
//	main
// -----------------------------------------------------------------------------
//
int
main(
	int				argc,
	char*			argv[] )
{
	IBNibRef 		nibRef;
	static const
	EventTypeSpec	kAppEventList[] = {
						{ kEventClassCommand, kEventProcessCommand } };
	OSStatus		err;

	ImageMapViewRegister();

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
	err = InstallApplicationEventHandler( GetAppEventHandlerUPP(),
			GetEventTypeCount( kAppEventList ), kAppEventList, NULL, NULL );
	require_noerr( err, CantInstallAppEventHandler );

	err = CreateWindow( kFoodPyramidWindow );
	require_noerr( err, CantCreateWindow );

	// Call the event loop
	RunApplicationEventLoop();

CantCreateWindow:
CantInstallAppEventHandler:
CantSetMenuBar:
CantGetNibRef:
	return err;
}

// -----------------------------------------------------------------------------
//	AppEventHandler
// -----------------------------------------------------------------------------
//
OSStatus
AppEventHandler(
	EventHandlerCallRef	inHandlerCallRef,
	EventRef			inEvent,
	void*				inUserData )
{
	HICommand			hiCommand;
	OSStatus			err = eventNotHandledErr;

	// UInt32			eventClass = GetEventClass( inEvent );
	// UInt32			eventKind = GetEventKind( inEvent );

	// if ( eventClass == kEventClassCommand && eventKind == kEventProcessCommand )
	//{
		err = GetEventParameter( inEvent, kEventParamDirectObject, typeHICommand,
			NULL, sizeof( HICommand ), NULL, &hiCommand );
		require_noerr( err, CantGetEventParameter );

		switch ( hiCommand.commandID )
		{
			case kHICommandNew:
				err = CreateWindow( kFoodPyramidWindow );
				break;

			case 'New2':
				err = CreateWindow( kShapesWindow );
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
//	ChangeHotSpots
// -----------------------------------------------------------------------------
//
void
ChangeHotSpots(
	WindowRef				inWindow,
	ImageMapHotSpots		inHotSpotsKind )
{
	HIViewRef		imageMapView;

	verify_noerr( GetControlByID( inWindow, &kImageMapViewID, &imageMapView ) );
	ImageMapViewSetHotSpots( imageMapView, inHotSpotsKind );
}

// -----------------------------------------------------------------------------
//	CreateWindow
// -----------------------------------------------------------------------------
//
OSStatus
CreateWindow(
	CFStringRef		inWindowName )
{
	IBNibRef 		nibRef;
	WindowRef		window;
	static const
	EventTypeSpec	kWindowEventList[] = {
							{ kEventClassCommand, kEventProcessCommand }
						};
	OSStatus		err;
	HIViewRef		modeSelector, imageMapView;
	int				i;
	Rect			windowBounds;
	HIRect			frame, optimalBounds;
	HIViewRef		view;
	AXUIElementRef	uiElement;
	CFArrayRef		uiElements;

	// Create a Nib reference passing the name of the nib file (without the .nib extension)
	// CreateNibReference only searches into the application bundle.
	err = CreateNibReference( CFSTR( "main" ), &nibRef );
	require_noerr( err, CantGetNibRef );

	// Then create a window. "MainWindow" is the name of the window object. This name is set in
	// InterfaceBuilder when the nib is created.
	err = CreateWindowFromNib( nibRef, inWindowName, &window );
	require_noerr( err, CantCreateWindow );

	// We don't need the nib reference anymore.
	DisposeNibReference( nibRef );

	// Install the window event handler
	err = InstallWindowEventHandler( window, GetWindowEventHandlerUPP(),
			GetEventTypeCount( kWindowEventList ), kWindowEventList, window, NULL );
	require_noerr( err, CantInstallWindowEventHandler );

	// Set the images on the segmented view (our mode selector)
	err = GetControlByID( window, &kModeSelectorID, &modeSelector );
	require_noerr( err, CantGetControlByID );
	HIViewSetValue( modeSelector, 1 );
	for ( i = 0; i < kSelectorCount; i++ )
	{
		HIViewImageContentInfo	contentInfo = { kHIViewContentCGImageRef };
		contentInfo.u.imageRef = GetSelectorImage( i );
		verify_noerr( HISegmentedViewSetSegmentImage( modeSelector, i + 1, &contentInfo ) );
		verify_noerr( HISegmentedViewSetSegmentContentWidth( modeSelector, i + 1, true, 0 ) );
	}

	// Since the segmented view is auto-sized, it needs to be centered.
	HIViewGetOptimalBounds( modeSelector, &frame, NULL );
	GetWindowBounds( window, kWindowContentRgn, &windowBounds );
	frame.origin.x = floorf( ( windowBounds.right - windowBounds.left - frame.size.width ) / 2 + 0.5 );
	HIViewSetFrame( modeSelector, &frame );

	// Size the image map view to its optimal size (likely the size of its image)
	GetControlByID( window, &kImageMapViewID, &imageMapView );
	HIViewGetOptimalBounds( imageMapView, &optimalBounds, NULL );
	HIViewGetFrame( imageMapView, &frame );
	frame.size = optimalBounds.size;
	HIViewSetFrame( imageMapView, &frame );

	// *** Set the accessibility description name of the image map view
	// - This is done external to the view implementation just to show that any host can set
	//	 the description of a contained HIObject. Setting the description attribute could just
	//	 as easily be done in the view implementation
	HIObjectSetAuxiliaryAccessibilityAttribute( (HIObjectRef) imageMapView, 0, kAXDescriptionAttribute, ImageMapViewGetMapName( imageMapView ) );

	// *** Mark an element as the title element of another for accessibility. In this case, a
	// a static text control is the title element of our mode selector segemented view.
	GetControlByID( window, &kSelectorLabelID, &view );
	uiElement = AXUIElementCreateWithHIObjectAndIdentifier( (HIObjectRef) view, 0 );
	HIObjectSetAuxiliaryAccessibilityAttribute( (HIObjectRef) modeSelector, 0, kAXTitleUIElementAttribute, uiElement );
	CFRelease( uiElement );

	// *** Let the static text item know that it is serving that purpose.
	uiElement = AXUIElementCreateWithHIObjectAndIdentifier( (HIObjectRef) modeSelector, 0 );
	uiElements = CFArrayCreate( kCFAllocatorDefault, (const void**) &uiElement, 1, &kCFTypeArrayCallBacks );
	HIObjectSetAuxiliaryAccessibilityAttribute( (HIObjectRef) view, 0, kAXServesAsTitleForUIElementsAttribute, uiElements );
	CFRelease( uiElements );
	CFRelease( uiElement );

	// *** Set the accessibility descriptions of the sub-elements (the segments) of a segmented view.
	HIObjectSetAuxiliaryAccessibilityAttribute( (HIObjectRef) modeSelector, 1, kAXDescriptionAttribute, CFSTR( "Invisible Hot Spots" ) );
	HIObjectSetAuxiliaryAccessibilityAttribute( (HIObjectRef) modeSelector, 2, kAXDescriptionAttribute, CFSTR( "Visible Hot Spots" ) );
	HIObjectSetAuxiliaryAccessibilityAttribute( (HIObjectRef) modeSelector, 3, kAXDescriptionAttribute, CFSTR( "Rollover Hot Spots" ) );

	// *** There is a User Pane used to group the image map view and the image map label together.
	// Mark it as ignored for accessibility.
	GetControlByID( window, &kImageMapAndLabelGroupingPaneID, &view );
	HIObjectSetAccessibilityIgnored( (HIObjectRef) view, true );
	
	// The window was created hidden so show it.
	ShowWindow( window );

CantGetControlByID:
CantInstallWindowEventHandler:
CantCreateWindow:
CantGetNibRef:
	return err;
}

// -----------------------------------------------------------------------------
//	GetSelectorImage
// -----------------------------------------------------------------------------
//
CGImageRef
GetSelectorImage(
	int				inIndex )
{
	static
	CGImageRef		sSelectorImages[ kSelectorCount ] = { NULL };

	check( inIndex < kSelectorCount );

	// Lazy initialization. Use ImageIO to load the images.
	if ( sSelectorImages[ 0 ] == NULL )
	{
		CGImageSourceRef		imageSource;
		CFURLRef				imageUrl;
		CFStringRef				selectorImageNames[ kSelectorCount ] = {
										CFSTR( "InvisibleHotSpots.gif" ),
										CFSTR( "VisibleHotSpots.gif" ),
										CFSTR( "RolloverHighlighting.gif" ),
									};
		int						i;
					
		for ( i = 0; i < kSelectorCount; i++ )
		{
			imageUrl = ::CFBundleCopyResourceURL( CFBundleGetMainBundle(), selectorImageNames[ i ], NULL, NULL );
			check( imageUrl != NULL );

			imageSource = ::CGImageSourceCreateWithURL( imageUrl, 0 );
			check( imageSource != NULL );
			sSelectorImages[ i ] = ::CGImageSourceCreateImageAtIndex( imageSource, 0, 0 );
			check( sSelectorImages[ i ] != NULL );
			::CFRelease( imageSource );
			::CFRelease( imageUrl );
		}
	}
	
	return sSelectorImages[ inIndex ];
}

// -----------------------------------------------------------------------------
//	WindowEventHandler
// -----------------------------------------------------------------------------
//
OSStatus
WindowEventHandler(
	EventHandlerCallRef	inHandlerCallRef,
	EventRef			inEvent,
	void*				inUserData )
{
	HICommandExtended	hiCommand;
	OSStatus			err = eventNotHandledErr;
	UInt32				eventClass = GetEventClass( inEvent );
	UInt32				eventKind = GetEventKind( inEvent );
	WindowRef			window = (WindowRef) inUserData;

	if ( eventClass == kEventClassCommand && eventKind == kEventProcessCommand )
	{
		err = GetEventParameter( inEvent, kEventParamDirectObject, typeHICommand,
			NULL, sizeof( HICommand ), NULL, &hiCommand );
		require_noerr( err, CantGetEventParameter );

		switch ( hiCommand.commandID )
		{
			case kCommandModeInvisibleHotSpots:
				ChangeHotSpots( window, kImageMapHotSpotsInvisible );
				break;

			case kCommandModeVisibleHotSpots:
				ChangeHotSpots( window, kImageMapHotSpotsVisible );
				break;

			case kCommandModeRollovereHotSpots:
				ChangeHotSpots( window, kImageMapHotSpotsRollover );
				break;

			case kImageMapView:
				{
					HIViewRef		imageMapLabel;

					// Set the label of the selected image map part, using the value
					// of the image map view to select from an array of labels
					verify_noerr( GetControlByID( window, &kImageMapLabelID, &imageMapLabel ) );
					HIViewSetText( imageMapLabel, ImageMapViewGetSelectedName( hiCommand.source.control ) );
				}
				break;

			default:
				err = eventNotHandledErr;
				break;
		}
	}

CantGetEventParameter:
	return err;
}
