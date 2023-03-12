/*
	File:		main.c
 
	Abstract:	Our main application code, written in Carbon which calls DisplayCocoaWebWindow
	to display a Cocoa based window.  Intended to run on Mac OS X 10.4 and greater.
 
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

static	OSStatus	DoNewWindow( WindowRef *outWindow );
static	OSStatus	CommandProcessEventHandler( EventHandlerCallRef nextHandler, EventRef inEvent, void *userData );
static	OSStatus	MainWindowEventHandler( EventHandlerCallRef nextHandler, EventRef inEvent, void *inUserData );
static	OSStatus	HIViewFindBySigAndID( HIViewRef inStartView, OSType signature, SInt32 id, HIViewRef *outControl );
OSStatus	DisplayCocoaWebWindow( CFURLRef urlRef );

IBNibRef				gMainNibRef;

//--------------------------------------------------------------------------------------------
int	main( int argc, char *argv[] )
{
	OSStatus				status;
	const   EventTypeSpec   commandProcessEvents[]		= { { kEventClassCommand, kEventCommandProcess } };
	
	status	= CreateNibReference( CFSTR("main"), &gMainNibRef );
	require_noerr( status, CantGetNibRef );
	
	
	//	IMPORTANT: Carbon applications must call NSApplicationLoad() to initialize the Cocoa environment.
	NSApplicationLoad();	//	Make sure that cocoa is initialized properly

	
	//	Once the nib reference is created, set the menu bar. "MainMenu" is the name of the menu bar object. This name is set in InterfaceBuilder when the nib is created.
	status	= SetMenuBarFromNib( gMainNibRef, CFSTR("MenuBar") );
	require_noerr( status, CantSetMenuBar );
	
	InstallApplicationEventHandler( NewEventHandlerUPP(CommandProcessEventHandler), GetEventTypeCount(commandProcessEvents), commandProcessEvents, NULL, NULL );
	
	DoNewWindow( NULL );
	
	RunApplicationEventLoop();											//	Call the event loop
	
CantSetMenuBar:
CantGetNibRef:
		return( status );
}

static	OSStatus	CommandProcessEventHandler( EventHandlerCallRef nextHandler, EventRef inEvent, void *userData )
{
	HICommand		command;
	OSStatus		status			= eventNotHandledErr;
	
	GetEventParameter( inEvent, kEventParamDirectObject, typeHICommand, NULL, sizeof(HICommand), NULL, &command );
	
	switch ( command.commandID )
	{
		case kHICommandNew:
			status  = DoNewWindow( NULL );
			break;
	}
	
	return( status );
}


static	OSStatus	DoNewWindow( WindowRef *outWindow )
{
	OSStatus					status;
	static	EventHandlerUPP		windowEventHandlerUPP;
	WindowRef					window				= NULL;
	const	EventTypeSpec		windowEvents[]		=	{ { kEventClassCommand, kEventCommandProcess } };
	
	//	Create a window. "MainWindow" is the name of the window object. This name is set in	InterfaceBuilder when the nib is created.
	status	= CreateWindowFromNib( gMainNibRef, CFSTR("MainWindow"), &window );
	require_noerr( status, CantCreateWindow );
	
	if ( windowEventHandlerUPP == NULL ) windowEventHandlerUPP	= NewEventHandlerUPP( MainWindowEventHandler );
	status	= InstallWindowEventHandler( window, windowEventHandlerUPP, GetEventTypeCount(windowEvents), windowEvents, window, NULL );
	require_noerr( status, CantInstallWindowEventHandler );
	
	//	The window was created hidden so show it if the window parameter is NULL, if it's not, it will be the responsibility of the caller to show it.
	if ( outWindow == NULL )	ShowWindow( window );
	
CantInstallWindowEventHandler:
CantCreateWindow:
		if ( outWindow != NULL )	*outWindow	= window;
	return( status );
}


static	OSStatus	MainWindowEventHandler( EventHandlerCallRef nextHandler, EventRef inEvent, void *inUserData )
{
#pragma unused ( inCallRef )
	HICommand		command;
	HIViewRef		editTextField;
	CFURLRef		urlRef			= NULL;
	CFStringRef		urlString		= NULL;
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
				if ( command.commandID == 'Broz' )												//	Open url as web page
				{
					status	= HIViewFindBySigAndID( HIViewGetRoot(window), 'ETxt', 0, &editTextField );
					status	= GetControlData( editTextField, kControlEditTextPart, kControlEditTextCFStringTag, sizeof( CFStringRef ), &urlString, NULL );	//	Get the current text as a CFURL from the Combo Box
					urlRef  = CFURLCreateWithString( kCFAllocatorDefault, urlString, NULL );		//	Create a CFURL from the CFString
					
					status	= DisplayCocoaWebWindow( urlRef );
				}
			}
			break;
	}
    
    return( status );
}

static	OSStatus	HIViewFindBySigAndID( HIViewRef inStartView, OSType signature, SInt32 id, HIViewRef *outControl )
{
	HIViewID	hiViewID = { signature, id };
	return( HIViewFindByID( inStartView, hiViewID, outControl ) );
}

