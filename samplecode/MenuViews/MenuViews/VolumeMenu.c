/*
	File:		VolumeMenu.c

    Version:	Mac OS X

	Disclaimer:	IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc.
				("Apple") in consideration of your agreement to the following terms, and your
				use, installation, modification or redistribution of this Apple software
				constitutes acceptance of these terms.  If you do not agree with these terms,
				please do not use, install, modify or redistribute this Apple software.

				In consideration of your agreement to abide by the following terms, and subject
				to these terms, Apple grants you a personal, non-exclusive license, under AppleÕs
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

#include "VolumeMenu.h"


#define kVolumeViewClassID	CFSTR("com.apple.sample.kVolumeViewClassID")


enum
{
	kHICommandVolumeChanged		= 'VOLC',
	kHICommandMuteChanged		= 'MUTC',
	kHICommandOpenSoundPrefs	= 'PREF'
};


typedef struct
{
	HIViewRef		view;
	HIViewRef		slider;
	HIViewRef		checkbox;
	HIViewRef		button;
}
VolumeData;


static pascal OSStatus	AppHandler( EventHandlerCallRef inCaller, EventRef inEvent, void* inRefcon );
static CFStringRef		GetVolumeMenuClassID();
static pascal OSStatus	VolumeHandler( EventHandlerCallRef inCaller, EventRef inEvent, void* inRefcon );
static pascal void		SliderAction(ControlRef theControl, ControlPartCode partCode);
static pascal void		CloseMenuTimer( EventLoopTimerRef inTimer, void* inRefcon );
static OSStatus			SetMute( Boolean inMute );
static OSStatus			OpenSoundPrefs();


static const EventTypeSpec	kAppEvents[] =
{
	{ kEventClassMenu, kEventMenuBeginTracking }
};

static const EventTypeSpec	kVolumeEvents[] =
{
	{ kEventClassHIObject, kEventHIObjectConstruct },
	{ kEventClassHIObject, kEventHIObjectDestruct },
	{ kEventClassControl, kEventControlInitialize },
	{ kEventClassControl, kEventControlBoundsChanged },
	{ kEventClassControl, kEventControlGetOptimalBounds },
	{ kEventClassScrollable, kEventScrollableGetInfo },
	{ kEventClassCommand, kEventCommandProcess }
};

static MenuRef		sCurrentRootMenu;


//ÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑ
//	¥ VolumeMenuCreate
//	Creates a new volume menu.
//ÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑ
OSStatus
VolumeMenuCreate( MenuRef* outMenu )
{
	static Boolean	sAppHandlerInstalled;
	MenuDefSpec		defSpec;
	OSStatus		err;
	
	if ( !sAppHandlerInstalled )
	{
		InstallApplicationEventHandler( NewEventHandlerUPP( AppHandler ), GetEventTypeCount( kAppEvents ),
										kAppEvents, 0, NULL );
		sAppHandlerInstalled = true;
	}
	
	defSpec.defType = kMenuDefClassID;
	defSpec.u.view.classID = GetVolumeMenuClassID();
	defSpec.u.view.initEvent = NULL;
	err = CreateCustomMenu( &defSpec, 0, 0, outMenu );
	if ( err == noErr )
		SetMenuTitleWithCFString( *outMenu, CFSTR("Volume") );
	return err;
}

//ÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑ
//	¥ÊAppHandler
//	Event handler for kEventMenuBeginTracking that records the current root menu,
//	so we can pass it to CancelMenuTracking from CloseMenuTimer.
//ÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑ
static pascal OSStatus
AppHandler( EventHandlerCallRef inCaller, EventRef inEvent, void* inRefcon )
{
	check( GetEventClass( inEvent ) == kEventClassMenu );
	check( GetEventKind( inEvent ) == kEventMenuBeginTracking );
	
	GetEventParameter( inEvent, kEventParamDirectObject, typeMenuRef, NULL, sizeof( sCurrentRootMenu ), NULL, &sCurrentRootMenu );
	return noErr;
}

//ÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑ
//	¥ÊGetVolumeMenuClassID
//	Registers the HIView subclass used for the volume menu, and returns its class ID.
//ÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑ
static CFStringRef
GetVolumeMenuClassID()
{
	static HIObjectClassRef	sClassRef;
	
	if ( sClassRef == NULL )
	{
		verify_noerr( HIObjectRegisterSubclass( kVolumeViewClassID, kHIMenuViewClassID, kNilOptions, VolumeHandler,
												GetEventTypeCount( kVolumeEvents ), kVolumeEvents, NULL, &sClassRef ) );
	}

	return kVolumeViewClassID;
}

//ÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑ
//	¥ VolumeHandler
//	Handles events for the volume menu.
//ÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑ
static pascal OSStatus
VolumeHandler( EventHandlerCallRef inCaller, EventRef inEvent, void* inRefcon )
{
	OSStatus	err = eventNotHandledErr;
	VolumeData*	data = (VolumeData*) inRefcon;
	
	switch ( GetEventClass( inEvent ) )
	{
		case kEventClassHIObject:
			switch ( GetEventKind( inEvent ) )
			{
				case kEventHIObjectConstruct:
				{
					data = (VolumeData*) calloc( 1, sizeof( VolumeData ) );
					require_action( data != NULL, exception_VolumeHandler_CouldntAllocData, err = memFullErr );
					
					verify_noerr( GetEventParameter( inEvent, kEventParamHIObjectInstance, typeHIObjectRef, NULL,
													 sizeof( data->view ), NULL, &data->view ) );
					verify_noerr( SetEventParameter( inEvent, kEventParamHIObjectInstance, typeVoidPtr, sizeof( data ), &data ) );
					err = noErr;
					break;
				}
					
				case kEventHIObjectDestruct:
					// our controls will be freed automatically as the view hierarchy is released
					free( (void*) data );
					break;
			}
			break;

		case kEventClassCommand:
			switch ( GetEventKind( inEvent ) )
			{
				case kEventCommandProcess:
				{
					HICommand cmd;
					verify_noerr( GetEventParameter( inEvent, kEventParamDirectObject, typeHICommand, NULL,
													 sizeof( cmd ), NULL, &cmd ) );
					switch ( cmd.commandID )
					{
						case kHICommandMuteChanged:
							err = SetMute( GetControlValue( data->checkbox ) != 0 );
							
							//
							// We want to close the menu after the user has clicked on the mute button,
							// but we also want the user to be able to see the mute button change state,
							// which doesn't happen until the event loop runs, the button redraws, and
							// the window is flushed. To do this we use a timer that runs a short time
							// after the click occurs and cancels menu tracking.
							//
							InstallEventLoopTimer( GetMainEventLoop(), 0.15, 0, CloseMenuTimer, 0, NULL );
							break;
						
						case kHICommandOpenSoundPrefs:
							err = OpenSoundPrefs();
							break;
							
						default:
							break;
					}
					break;
				}
			}
			break;

		case kEventClassControl:
			switch ( GetEventKind( inEvent ) ) 
			{
				case kEventControlInitialize:
				{
					Rect		bounds = { 0, 0, 0, 0 };
					UInt32		features = kControlSupportsEmbedding;
					
					verify_noerr( SetEventParameter( inEvent, kEventParamControlFeatures, typeUInt32, sizeof( features ), &features ) );

					err = CreateSliderControl( NULL, &bounds, 0, 0, 100, kControlSliderDoesNotPoint,
											   0, true, SliderAction, &data->slider );
					require_noerr( err, exception_VolumeHandler_CouldntCreateSlider );
					verify_noerr( HIViewAddSubview( data->view, data->slider ) );
					SetControlCommandID( data->slider, kHICommandVolumeChanged );
					SetControlReference( data->slider, (SInt32) data );
												
					err = CreateCheckBoxControl( NULL, &bounds, CFSTR("Mute"), 0, true, &data->checkbox );
					require_noerr( err, exception_VolumeHandler_CouldntCreateCheckbox );
					verify_noerr( HIViewAddSubview( data->view, data->checkbox ) );
					SetControlCommandID( data->checkbox, kHICommandMuteChanged );

					err = CreatePushButtonControl( NULL, &bounds, CFSTR("Prefs"), &data->button );
					require_noerr( err, exception_VolumeHandler_CouldntCreateButton );
					verify_noerr( HIViewAddSubview( data->view, data->button ) );
					SetControlCommandID( data->button, kHICommandOpenSoundPrefs );
					
					err = noErr;
					break;
				}
				
				case kEventControlBoundsChanged:
				{
					SInt32 sliderWidth;
					GetThemeMetric( kThemeMetricVSliderWidth, &sliderWidth );
					
					HIRect bounds;
					HIViewGetBounds( data->view, &bounds );
					
					HIRect frame;
					frame.origin.x = bounds.size.width / 2 - ( sliderWidth / 2 );
					frame.size.width = sliderWidth;
					frame.origin.y = 10;
					frame.size.height = 300;
					HIViewSetFrame( data->slider, &frame );
					
					frame.origin.x = 10;
					frame.size.width = bounds.size.width - 20;
					frame.origin.y = frame.origin.y + frame.size.height + 10;
					frame.size.height = 20;
					HIViewSetFrame( data->checkbox, &frame );
					
					frame.origin.y = frame.origin.y + frame.size.height + 10;
					HIViewSetFrame( data->button, &frame );
					break;
				}
				
				case kEventControlGetOptimalBounds:
				{
					HIRect bounds = { { 0, 0 }, { 90, 380 } };
					err = SetEventParameter( inEvent, kEventParamControlOptimalBounds, typeHIRect, sizeof( bounds ), &bounds );
					// we don't return a baseline offset; it doesn't apply for menus
					break;
				}
			}
			break;
		
		case kEventClassScrollable:
			switch ( GetEventKind( inEvent ) )
			{
				case kEventScrollableGetInfo:
				{
					HIRect bounds;
					HIPoint origin = { 0, 0 };
					HIViewGetBounds( data->view, &bounds );
					verify_noerr( SetEventParameter( inEvent, kEventParamImageSize, typeHISize, sizeof( bounds.size ), &bounds.size ) );
					verify_noerr( SetEventParameter( inEvent, kEventParamViewSize, typeHISize, sizeof( bounds.size ), &bounds.size ) );
					
					bounds.size.height = 20;	// arbitrary, doesn't really matter
					verify_noerr( SetEventParameter( inEvent, kEventParamLineSize, typeHISize, sizeof( bounds.size ), &bounds.size ) );
					
					verify_noerr( SetEventParameter( inEvent, kEventParamOrigin, typeHIPoint, sizeof( origin ), &origin ) );
					err = noErr;
					break;
				}
			}
			break;
	}

exception_VolumeHandler_CouldntCreateButton:
exception_VolumeHandler_CouldntCreateCheckbox:
exception_VolumeHandler_CouldntCreateSlider:
exception_VolumeHandler_CouldntAllocData:
	return err;
}

//ÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑ
//	¥ÊSliderAction
//	Live-scrolling action proc for the volume slider.
//ÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑ
static pascal void
SliderAction( ControlRef theControl, ControlPartCode partCode )
{
	VolumeData* data = (VolumeData*) GetControlReference( theControl );
	if ( GetControlValue( theControl ) == 0 )
		SetControlValue( data->checkbox, 1 );
	else
		SetControlValue( data->checkbox, 0 );
}

//ÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑ
//	¥ÊCloseMenuTimer
//	A timer that cancels menu tracking.
//ÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑ
static pascal void
CloseMenuTimer( EventLoopTimerRef inTimer, void* inRefcon )
{
	CancelMenuTracking( sCurrentRootMenu, true, kHIMenuDismissedBySelection );
	RemoveEventLoopTimer( inTimer );
}

//ÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑ
//	¥ÊSetMute
//	Enables or disables muting.
//ÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑ
static OSStatus
SetMute( Boolean inMute )
{
	return noErr;
}

//ÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑ
//	¥ÊOpenSoundPrefs
//	Opens the Sound preferences pane.
//ÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑ
static OSStatus
OpenSoundPrefs()
{
	OSStatus			err = noErr;
	FSRef				folderRef, itemRef;
	HFSUniStr255		theItemName;
	CFIndex				used;
	LSLaunchFSRefSpec	spec;
	CFStringRef			fullName = CFSTR("Sound.prefPane");
	
	// Get the System Prefs folder, where preferences panes _must_ live.
	err = FSFindFolder( kSystemDomain, kSystemPreferencesFolderType, false, &folderRef );
	require_noerr( err, OpenSoundPrefs_CantFindAppsFolder );
	
	// Get the name in Unicode for our call into the File system
	theItemName.length = CFStringGetBytes( 	fullName, 
											CFRangeMake( 0, CFStringGetLength( fullName ) ), 
											kCFStringEncodingUnicode, 
											0, 
											false, 
											(UInt8*) theItemName.unicode,
											sizeof( theItemName.unicode ), 
											&used );

	// Now get the ref to the pref pane itself
	err = FSMakeFSRefUnicode( &folderRef, theItemName.length, theItemName.unicode, kCFStringEncodingUnicode, &itemRef );
	require_noerr( err, OpenSoundPrefs_CantFindPrefPath );

	spec.appRef 		= NULL;
	spec.numDocs 		= 1;
	spec.itemRefs 		= &itemRef;
	spec.passThruParams = NULL;
	spec.launchFlags 	= (kLSLaunchAsync | kLSLaunchDontAddToRecents);
	spec.asyncRefCon 	= 0;
	
	err = LSOpenFromRefSpec( &spec, NULL );
	check_noerr( err );

OpenSoundPrefs_CantFindPrefPath:
OpenSoundPrefs_CantFindAppsFolder:
	return err;
}
