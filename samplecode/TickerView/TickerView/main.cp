#include <Carbon/Carbon.h>

#include "TTickerView.h"


int main(int argc, char* argv[])
{
#pragma unused( argc, argv )
	IBNibRef 		nibRef;
	WindowRef 		window;
	HIViewRef		view;
	
	OSStatus		err;
	
	// Workaround: Unfortunately, at the time we call RegisterClass below,
	// the HIView base class isn't registered. It's supposed to be automatically
	// registered, but something is going wrong in HIToolbox. We can force
	// it to register by creating any arbitrary view. Here, we simply create
	// and release a scroll view. That's enough to make sure the HIView base
	// class is registered. Sorry folks.
	HIScrollViewCreate( kHIScrollViewOptionsVertScroll, &view );
	CFRelease( view );
	
	// Register our ticker view subclass 
	TTickerView::RegisterClass();
	
	// Create a Nib reference passing the name of the nib file (without the .nib extension)
	// CreateNibReference only searches into the application bundle.
	err = CreateNibReference( CFSTR( "main" ), &nibRef );
	require_noerr( err, CantGetNibRef );
	
	// Once the nib reference is created, set the menu bar. "MainMenu" is the name of the menu bar
	// object. This name is set in InterfaceBuilder when the nib is created.
	err = SetMenuBarFromNib( nibRef, CFSTR( "MenuBar" ) );
	require_noerr( err, CantSetMenuBar );
	
	// Then create a window. "MainWindow" is the name of the window object. This name is set in 
	// InterfaceBuilder when the nib is created.
	err = CreateWindowFromNib( nibRef, CFSTR( "MainWindow" ), &window );
	require_noerr( err, CantCreateWindow );
	
	// We don't need the nib reference anymore.
	DisposeNibReference( nibRef );
	
	// The window was created hidden so show it.
	ShowWindow( window );
	
	// Call the event loop
	RunApplicationEventLoop();
	
CantCreateWindow:
CantSetMenuBar:
CantGetNibRef:
	
	return err;
}

