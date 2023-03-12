/*
    File:		main.cp
    
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

	Copyright © 2002 Apple Computer, Inc., All Rights Reserved
*/

#include <Carbon/Carbon.h>

#include "TURLTextView.h"

static pascal OSStatus	WindowEventHandler( EventHandlerCallRef inCaller, EventRef inEvent, void* inRefcon );
static OSStatus			MyCreateWindow( WindowRef* outWindow );

DEFINE_ONE_SHOT_HANDLER_GETTER( WindowEventHandler );

int main(int argc, char* argv[])
{
#pragma unused( argc, argv )

    IBNibRef 		nibRef;
    WindowRef 		window;
    OSStatus		err;

    // Create a Nib reference passing the name of the nib file (without the .nib extension)
    // CreateNibReference only searches into the application bundle.
    err = CreateNibReference(CFSTR("main"), &nibRef);
    require_noerr( err, CantGetNibRef );
    
    // Once the nib reference is created, set the menu bar. "MainMenu" is the name of the menu bar
    // object. This name is set in InterfaceBuilder when the nib is created.
    err = SetMenuBarFromNib(nibRef, CFSTR("MenuBar"));
    require_noerr( err, CantSetMenuBar );
    
    // We don't need the nib reference anymore.
    DisposeNibReference(nibRef);
    
	// Create the About box
	err = MyCreateWindow( &window );
	require_noerr( err, CantCreateWindow );
	
	// Place the About box in the standard alert position
	RepositionWindow( window, NULL, kWindowAlertPositionOnMainScreen );
	
    // The window was created hidden so show it.
    ShowWindow( window );
    
    // Call the event loop
	RunAppModalLoopForWindow( window );

CantCreateWindow:
CantSetMenuBar:
CantGetNibRef:
	return err;
}

static OSStatus
MyCreateWindow( WindowRef* outWindow )
{
	static const EventTypeSpec	kWindowEvents[] =	{
														{ kEventClassCommand, kEventCommandProcess }
													};
	static const ControlID		kIconCtlID = { 'ICON', 0 };
	static const ControlID		kURLCtlID = { 'URLV', 0 };
	
    IBNibRef 					nibRef;
    OSStatus					err;
	ControlRef					ctl;
	Rect						bounds;
	ControlImageContentInfo		content = { kControlContentIconRef };
	ProcessSerialNumber			psn = { 0, kCurrentProcess };
	FSRef						appLocation;
	HIRect						hiBounds;
	ControlRef					root;

    // Create a Nib reference passing the name of the nib file (without the .nib extension)
    // CreateNibReference only searches into the application bundle.
    err = CreateNibReference(CFSTR("main"), &nibRef);
    require_noerr( err, CantGetNibRef );
    
    // Then create a window. "MainWindow" is the name of the window object. This name is set in 
    // InterfaceBuilder when the nib is created.
    err = CreateWindowFromNib(nibRef, CFSTR("MainWindow"), outWindow );
    require_noerr( err, CantCreateWindow );

    // We don't need the nib reference anymore.
    DisposeNibReference(nibRef);
	
	GetProcessBundleLocation( &psn, &appLocation );
	GetIconRefFromFileInfo( &appLocation, 0, NULL, 0, NULL, kIconServicesNormalUsageFlag, &content.u.iconRef, NULL );
	
	GetControlByID( *outWindow, &kIconCtlID, &ctl );
	GetControlBounds( ctl, &bounds );
	DisposeControl( ctl );
	
	CreateIconControl( *outWindow, &bounds, &content, true, &ctl );
	
	GetControlByID( *outWindow, &kURLCtlID, &ctl );
	GetControlBounds( ctl, &bounds );
	DisposeControl( ctl );
	
	hiBounds.origin.x = bounds.left;
	hiBounds.origin.y = bounds.top;
	hiBounds.size.width = bounds.right - bounds.left;
	hiBounds.size.height = bounds.bottom - bounds.top;
	
#if 1
	err = HIURLTextViewCreate( &hiBounds, CFURLCreateWithString( NULL, CFSTR("http://www.apple.com"), NULL ),
							   NULL, &ctl );
#elif 0
	err = HIURLTextViewCreate( &hiBounds, NULL, CFSTR("Apple"), &ctl );
	HIURLTextViewSetURL( ctl, CFURLCreateWithString( NULL, CFSTR("http://www.apple.com/macosx"), NULL ) );
	HIURLTextViewSetText( ctl, CFSTR("Mac OS X") );

	// testing
	{
		CFURLRef					url;
		CFStringRef					text;
		HIURLTextViewCopyURL( ctl, &url );
		CFShow( url );
		CFRelease( url );
		HIURLTextViewCopyText( ctl, &text );
		CFShow( text );
		CFRelease( text );
	}
#else
	err = HIURLTextViewCreate( &hiBounds,
							   CFURLCreateWithString( NULL, CFSTR("file:///Developer/Documentation/ReleaseNotes/HIToolbox.html"), NULL ),
							   CFSTR("HIToolbox Release Notes"), &ctl );
#endif

	GetRootControl( *outWindow, &root );
	HIViewAddSubview( root, ctl );

	if ( err == noErr )
		ShowControl( ctl );
	
	// Install our event handlers
	InstallWindowEventHandler( *outWindow, GetWindowEventHandlerUPP(),
							   GetEventTypeCount( kWindowEvents ), kWindowEvents,
							   *outWindow, NULL );
	
CantCreateWindow:
CantGetNibRef:
	return err;
}

static pascal OSStatus
WindowEventHandler( EventHandlerCallRef inCaller, EventRef inEvent, void* inRefcon )
{
#pragma unused( inCaller )

	OSStatus	result = eventNotHandledErr;
	
	switch ( GetEventClass( inEvent ) )
	{
		case kEventClassCommand:
		{
			HICommand	cmd;
			
			GetEventParameter( inEvent, kEventParamDirectObject, typeHICommand, NULL, sizeof( cmd ), NULL, &cmd );
			switch ( GetEventKind( inEvent ) )
			{
				case kEventCommandProcess:
					switch ( cmd.commandID )
					{
						case kHICommandOK:
							QuitAppModalLoopForWindow( (WindowRef) inRefcon );
							result = noErr;
							break;
						
						default:
							break;
					}
					break;
				
				default:
					break;
			}
			break;
		}
			
		default:
			break;
	}
	
	return result;
}
