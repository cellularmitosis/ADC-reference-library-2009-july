/*
	File:		main.c
 
	Abstract:	This source demonstrates how to use NSCursor from a Carbon based application.
	We provide some wrapper functions CreateCocoaCursor, ReleaseCocoaCursor, and SetCocoaCursor
	to abstract the underlying Cocoa calls to manage the cursor.  NSCursor provides support for
	large cursors, alpha, and hot spots.
	
 
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
#include <Cocoa/Cocoa.h>
#include "NSCursorWrappers.h"

static OSStatus DoNewWindow( WindowRef *outWindow );
static OSStatus CommandProcessEventHandler( EventHandlerCallRef nextHandler, EventRef inEvent, void *userData );
static OSStatus HIViewFindBySigAndID( HIViewRef inStartView, OSType signature, SInt32 id, HIViewRef *outControl );
static OSStatus CursorEventHandler( EventHandlerCallRef nextHandler, EventRef inEvent, void *inUserData );
static	OSStatus	MainWindowEventHandler( EventHandlerCallRef nextHandler, EventRef inEvent, void *inUserData );
CursorRef SetCursorFromCFURL( CFURLRef urlToImage );

IBNibRef				gMainNibRef;

struct CustomCursorStruct {
	CursorRef						*cursor;
};
typedef struct CustomCursorStruct CustomCursorStruct;


int	main( int argc, char *argv[] )
{
	OSStatus				status;
	const   EventTypeSpec   commandProcessEvents[]		= { { kEventClassCommand, kEventCommandProcess } };
	
	status	= CreateNibReference( CFSTR("main"), &gMainNibRef );
	require_noerr( status, CantGetNibRef );
	
	//	Once the nib reference is created, set the menu bar. "MainMenu" is the name of the menu bar object. This name is set in InterfaceBuilder when the nib is created.
	status	= SetMenuBarFromNib( gMainNibRef, CFSTR("MenuBar") );
	require_noerr( status, CantSetMenuBar );
	
	InstallApplicationEventHandler( NewEventHandlerUPP(CommandProcessEventHandler), GetEventTypeCount(commandProcessEvents), commandProcessEvents, NULL, NULL );
	
	NSApplicationLoad();												//	Needed for Carbon based applications which call into Cocoa
	
	//	As of Mac OS X 10.4 RunApplicationEventLoop handles the autorelease pool for us.  But since we are invoking Obj-C Cocoa code from within
	//	routines called by DoNewWindow, and we haven't yet called RAEL, we need to allocate and release the autorelease pool ourselves.
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
	DoNewWindow( NULL );
	[pool release];
	
	RunApplicationEventLoop();											//	Call the event loop
	
CantSetMenuBar:
CantGetNibRef:
		return( 0 );
}


//------------------------------------------------------------------------------
//	GetPasteboardFromDragEvent
//------------------------------------------------------------------------------
//
static PasteboardRef GetPasteboardFromDragEvent(
	EventRef			inEvent )
{
	DragRef				drag;
	PasteboardRef		pasteboard = NULL;
	
	// get the drag pasteboard, if it exists
	require_noerr( GetEventParameter( inEvent, kEventParamDragRef, typeDragRef, NULL, sizeof( DragRef ),
										NULL, &drag ), CantGetDragRefFromEvent );

	GetDragPasteboard( drag, &pasteboard );
	
CantGetDragRefFromEvent:

	return pasteboard;
}

// Creates a CGImageRef from a URL.  Caller is responsible for disposing of it.
CGImageRef CGImageCreateFromURL( CFURLRef urlToImage )
{
	CGImageSourceRef	imageSource = CGImageSourceCreateWithURL( urlToImage, NULL );
	CGImageRef			theImage = nil;
	
	require( imageSource != nil, CantCreateImageSource );

	theImage = CGImageSourceCreateImageAtIndex( imageSource, 0, NULL );
	require( theImage != nil, CantGetImage );
	
	CFRelease( imageSource );

CantCreateImageSource:
CantGetImage:
	return( theImage );
}


//------------------------------------------------------------------------------
//	CreateFileURLFromPasteboard
//------------------------------------------------------------------------------
//
static CFURLRef CreateFileURLFromPasteboard(
	PasteboardRef		inPasteboard,
	CFIndex				inIndex )
{
	PasteboardItemID		item;
	CFDataRef				fileURLData;
	CFURLRef				fileURL = NULL;
	LSItemInfoRecord		info;
	CFStringRef				uti = NULL;
	bool					isSupported = false;

	require_noerr( PasteboardGetItemIdentifier( inPasteboard, inIndex, &item ), CantGetPasteboardIdentifier );
	
	require_noerr_quiet( PasteboardCopyItemFlavorData( inPasteboard, item, kUTTypeFileURL, &fileURLData ),
							CantCopyFileURLFromPasteboard );
	
	// create the file URL with the dragged data
	fileURL = CFURLCreateWithBytes( kCFAllocatorDefault, CFDataGetBytePtr( fileURLData ), CFDataGetLength( fileURLData ),
									kCFStringEncodingMacRoman, NULL );
	
	// get the UTI for the dragged file
	require_noerr( LSCopyItemInfoForURL( fileURL, kLSRequestExtension | kLSRequestTypeCreator, &info ), CantCopyItemInfo );
	
	if ( info.extension != NULL )
	{
		uti = UTTypeCreatePreferredIdentifierForTag( kUTTagClassFilenameExtension, info.extension, kUTTypeData );
					
		CFRelease( info.extension );
	}
	
	if ( uti == NULL )
	{
		CFStringRef typeString = UTCreateStringForOSType( info.filetype );
		
		if ( typeString != NULL )
		{
			uti = UTTypeCreatePreferredIdentifierForTag( kUTTagClassOSType, typeString, kUTTypeData );
			
			CFRelease( typeString );
		}
	}
	
	require( uti != NULL, CantCreateFileUTI );
	
	// verify we're dealing with a file that ImageIO can understand
	{
	CFArrayRef	supportedTypes = CGImageSourceCopyTypeIdentifiers();
	CFIndex		i, typeCount = CFArrayGetCount( supportedTypes );
	
	for( i = 0; i < typeCount; i++ )
	{
		if ( UTTypeConformsTo( uti, (CFStringRef)CFArrayGetValueAtIndex( supportedTypes, i ) ) )
		{
			isSupported = true;
			break;
		}
	}
	
	CFRelease( supportedTypes );
	}
	
	CFRelease( uti );
	
CantCreateFileUTI:
CantCopyItemInfo:
	
	if ( !isSupported )
	{
		CFRelease( fileURL );
		fileURL = NULL;
	}
	CFRelease( fileURLData );
	
CantCopyFileURLFromPasteboard:
CantGetPasteboardIdentifier:

	return fileURL;
}

//------------------------------------------------------------------------------
//	CreateFirstFileURLFromDragEvent
//------------------------------------------------------------------------------
//
static CFURLRef CreateFirstFileURLFromDragEvent(
	EventRef			inEvent )
{
	PasteboardRef		pasteboard;
	CFURLRef			fileURL = NULL;
	
	pasteboard = GetPasteboardFromDragEvent( inEvent );
	require( pasteboard != NULL, CantGetDragPasteboard );
	
	fileURL = CreateFileURLFromPasteboard( pasteboard, 1 );
	
CantGetDragPasteboard:

	return fileURL;
}


//------------------------------------------------------------------------------
//	ImageViewDragEnter
//------------------------------------------------------------------------------
//
static OSStatus ImageViewDragEnter(
	EventRef				inEvent)
{
	CFURLRef				fileURL = CreateFirstFileURLFromDragEvent( inEvent );
	Boolean					wouldAccept = (fileURL != NULL); // accept the drag if it contains an image file URL
	
	// let the HIView drag apparatus know that we want to continue receiving drag messages
	verify_noerr( SetEventParameter( inEvent, kEventParamControlWouldAcceptDrop, typeBoolean, sizeof( Boolean ), &wouldAccept ) );
	
	if ( fileURL != NULL )
		CFRelease( fileURL );
	
	return noErr;
}

OSStatus ImageViewDragReceive( EventRef event, CustomCursorStruct *customCursor, HIViewRef imageView )
{
	PasteboardRef		pasteboard;
	CFURLRef			fileURL = NULL;
	CursorRef			newCursor;

	pasteboard = GetPasteboardFromDragEvent( event );
	require( pasteboard != NULL, CantGetDragPasteboard );
	
	fileURL = CreateFirstFileURLFromDragEvent( event  );
	if ( fileURL )
		{
			CGImageRef theImage;
			
			newCursor = SetCursorFromCFURL( fileURL );
			if (newCursor)
			{
				ReleaseCocoaCursor( customCursor->cursor );
				customCursor->cursor = newCursor;
				SetCocoaCursor( newCursor );
				theImage = CGImageCreateFromURL( fileURL );
				HIImageViewSetImage( imageView, theImage );
				
				if ( theImage )
					CFRelease( theImage );
			}

			CFRelease( fileURL );
		}
	
CantGetDragPasteboard:
	return noErr;
}



CursorRef SetCursorFromCFURL( CFURLRef urlToImage )
{
	CGImageRef cgImageRef = CGImageCreateFromURL( urlToImage );
	CursorRef newCursor = NULL;
	
	require( cgImageRef != NULL, CantCreateImage );
	newCursor = CreateCocoaCursor( cgImageRef, 48, 16 );
	if ( cgImageRef )
		CFRelease( cgImageRef );

CantCreateImage:		
	return( (CursorRef)newCursor );
}


static OSStatus CommandProcessEventHandler( EventHandlerCallRef nextHandler, EventRef inEvent, void *userData )
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

//	The tracking handler to manage the cursor as it rolls over our HIImageView.
static OSStatus HIImageViewTrackingHandler( EventHandlerCallRef nextHandler, EventRef inEvent, void *inUserData )
{
	CustomCursorStruct	*customCursor;
	OSStatus			status			= eventNotHandledErr;
	UInt32				eventKind		= GetEventKind( inEvent );
	UInt32				eventClass		= GetEventClass( inEvent );
	
	status	= GetControlProperty( (ControlRef) inUserData, 'Curs', 'Cstm', sizeof(CustomCursorStruct*), NULL, &customCursor );	//	For convenience we store the pointer to the cursor
	require_noerr( status, Bail );
	
	switch ( eventClass )
	{
		case kEventClassControl:
			switch ( eventKind )
			{
				case kEventControlTrackingAreaEntered:
					SetCocoaCursor( customCursor->cursor );
					break;
				case kEventControlTrackingAreaExited:
					SetThemeCursor( kThemeArrowCursor );
					break;
				case kEventControlDispose:
					ReleaseCocoaCursor( customCursor->cursor );
					DisposePtr( (Ptr) customCursor );
					break;
				case kEventControlDragEnter:
					status = ImageViewDragEnter( inEvent );
					break;
				case kEventControlDragReceive:
					status = ImageViewDragReceive( inEvent, customCursor, (ControlRef) inUserData );
					break;
			}
			break;
	}
Bail:
    return( status );
}




static OSStatus DoNewWindow( WindowRef *outWindow )
{
	OSStatus					status;
	CustomCursorStruct			*customCursor;
	static	EventHandlerUPP		windowEventHandlerUPP;
	const	EventTypeSpec		windowEvents[]		=	{ { kEventClassCommand, kEventCommandProcess } };
	WindowRef					window				= NULL;

	//	Create a window. "MainWindow" is the name of the window object. This name is set in	InterfaceBuilder when the nib is created.
	status	= CreateWindowFromNib( gMainNibRef, CFSTR("MainWindow"), &window );
	require_noerr( status, CantCreateWindow );
	
	
	if ( windowEventHandlerUPP == NULL ) windowEventHandlerUPP	= NewEventHandlerUPP( MainWindowEventHandler );
	status	= InstallWindowEventHandler( window, windowEventHandlerUPP, GetEventTypeCount(windowEvents), windowEvents, window, NULL );
	require_noerr( status, CantInstallWindowEventHandler );

	{
		HIViewRef					hiImageView;
		HIViewTrackingAreaRef		dummyTrackingArea;      //	The tracking area is disposed automatically with the view, so we don't need to keep it in this case
		CGImageRef					cgImageRef			= NULL;
		const	EventTypeSpec		ourEvents[] = {	{ kEventClassControl, kEventControlTrackingAreaEntered },
															{ kEventClassControl, kEventControlTrackingAreaExited },
															{ kEventClassControl, kEventControlDispose },
															{ kEventClassControl, kEventControlDragReceive },
															{ kEventClassControl, kEventControlDragEnter },
														};
		CFURLRef	cfURL		= CFBundleCopyResourcesDirectoryURL( CFBundleGetMainBundle() );
		CFURLRef	fullUrlRef	= CFURLCreateCopyAppendingPathComponent( NULL, cfURL, CFSTR("BlueArrow.png"), false );
		CFRelease( cfURL );
		
		cgImageRef = CGImageCreateFromURL( fullUrlRef );
		status	= HIViewFindBySigAndID( HIViewGetRoot(window), 'HIiv', 0, &hiImageView );	require_noerr( status, Bail );		//	Get the HIImageView
		status	= HIImageViewSetImage( hiImageView, cgImageRef );		require_noerr( status, Bail );							//	Set the image of the HIImageView
		status	= HIImageViewSetOpaque( hiImageView, false );			require_noerr( status, Bail );							//	Image can have transparency
	//	status	= HIImageViewSetScaleToFit( hiImageView, true );		require_noerr( status, Bail );							//	Don't scale the image, it's embeded in an HIScrollView so scaling doesn't make much sense
		status	= HIViewSetVisible( hiImageView, true );				require_noerr( status, Bail );							//	Make it visible
		
		customCursor	= (CustomCursorStruct*) NewPtrClear( sizeof(CustomCursorStruct) );
		SetControlProperty( hiImageView,'Curs','Cstm', sizeof(CustomCursorStruct*), &customCursor );	//	For convenience we store the pointer to the cursor in the window's RefCon
		status  = HIViewNewTrackingArea( hiImageView, NULL, 0, &dummyTrackingArea);
		customCursor->cursor	= SetCursorFromCFURL( fullUrlRef );
		CFRelease( fullUrlRef );
		status	= InstallControlEventHandler( hiImageView, HIImageViewTrackingHandler, GetEventTypeCount(ourEvents), ourEvents, hiImageView, NULL );
		status	= SetAutomaticControlDragTrackingEnabledForWindow( window, true );
		status	= SetControlDragTrackingEnabled( hiImageView, true );
		
	Bail:
		if ( cgImageRef != NULL )		CFRelease( cgImageRef );					else	status  = -1;
	}
	
	ShowWindow( window );

CantInstallWindowEventHandler:
CantCreateWindow:
	if ( outWindow != NULL )	*outWindow	= window;
	return( status );
}


static	OSStatus	MainWindowEventHandler( EventHandlerCallRef nextHandler, EventRef inEvent, void *inUserData )
{
#pragma unused ( inCallRef )
	HICommand		command;
	HIViewRef		myButton;
	Boolean			isButtonDefault;
	WindowRef		window			= (WindowRef) inUserData;
	OSStatus		status			= eventNotHandledErr;
	UInt32			eventKind		= GetEventKind( inEvent );
	UInt32			eventClass		= GetEventClass( inEvent );
	
	switch ( eventClass )
	{
		case kEventClassCommand:
			if ( eventKind == kEventCommandProcess )
			{
				GetEventParameter( inEvent, kEventParamDirectObject, typeHICommand, NULL, sizeof(HICommand), NULL, &command );
				if ( command.commandID == 'Curs' )		//	Our button was clicked
				{
					status	= HIViewFindBySigAndID( HIViewGetRoot(window), 'Curs', 0, &myButton );	require_noerr( status, Bail );
					status = GetControlData( myButton, kControlEntireControl, kControlPushButtonDefaultTag, sizeof(Boolean), &isButtonDefault, NULL );		require_noerr( status, Bail );
					
					if ( isButtonDefault )
					{
						// If the button was default, show the cursor and make the button non-default
						ShowCocoaCursor();
						SetControlTitleWithCFString( myButton, CFSTR( "Hide Cursor" ));
						SetWindowDefaultButton( window, NULL );
					}
					else
					{
						//	We hit the hide cursor button.  Hide the cursor and make the button default so that the user can hit return
						//	to show the cursor again.  This is only useful as sample code, it's not good UI to hide the cursor generally.
						HideCocoaCursor();
						SetControlTitleWithCFString( myButton, CFSTR( "Show Cursor" ));
						SetWindowDefaultButton( window, myButton );
					}
				}
			}
			break;
	}
    
Bail:
	return( status );
}


static	OSStatus	HIViewFindBySigAndID( HIViewRef inStartView, OSType signature, SInt32 id, HIViewRef *outControl )
{
	HIViewID	hiViewID = { signature, id };
	return( HIViewFindByID( inStartView, hiViewID, outControl ) );
}

