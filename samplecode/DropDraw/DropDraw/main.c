/*

		File: DropDraw/main.c

		Abstract: A Carbon application to demonstrate automatic ColorSync matching in 
				QuickTime Graphics Importer components.

		Version: 1.0

		Disclaimer: IMPORTANT:  This Apple software is supplied to you by Apple
		Computer, Inc. ("Apple") in consideration of your agreement to the
		following terms, and your use, installation, modification or
		redistribution of this Apple software constitutes acceptance of these
		terms.  If you do not agree with these terms, please do not use,
		install, modify or redistribute this Apple software.

		In consideration of your agreement to abide by the following terms, and
		subject to these terms, Apple grants you a personal, non-exclusive
		license, under Apple's copyrights in this original Apple software (the
		"Apple Software"), to use, reproduce, modify and redistribute the Apple
		Software, with or without modifications, in source and/or binary forms;
		provided that if you redistribute the Apple Software in its entirety and
		without modifications, you must retain this notice and the following
		text and disclaimers in all such redistributions of the Apple Software. 
		Neither the name, trademarks, service marks or logos of Apple Computer,
		Inc. may be used to endorse or promote products derived from the Apple
		Software without specific prior written permission from Apple.  Except
		as expressly stated in this notice, no other rights or licenses, express
		or implied, are granted by Apple herein, including but not limited to
		any patent rights that may be infringed by your derivative works or by
		other works in which the Apple Software may be incorporated.

		The Apple Software is provided by Apple on an "AS IS" basis.  APPLE
		MAKES NO WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION
		THE IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS
		FOR A PARTICULAR PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND
		OPERATION ALONE OR IN COMBINATION WITH YOUR PRODUCTS.

		IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL
		OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
		SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
		INTERRUPTION) ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION,
		MODIFICATION AND/OR DISTRIBUTION OF THE APPLE SOFTWARE, HOWEVER CAUSED
		AND WHETHER UNDER THEORY OF CONTRACT, TORT (INCLUDING NEGLIGENCE),
		STRICT LIABILITY OR OTHERWISE, EVEN IF APPLE HAS BEEN ADVISED OF THE
		POSSIBILITY OF SUCH DAMAGE.

		Copyright © 2004 Apple Computer, Inc., All Rights Reserved

*/


#include <Carbon/Carbon.h>
#include <QuickTime/QuickTime.h>
#include <math.h>
#include <stdio.h>

Boolean gUseColorSyncMatching = true;
Boolean gUseSourceExtraction = false;
Boolean gClipUsingClipRegion = false;
Boolean gRotate30Degrees = false;


//////////
//
// openWindowForFile
// 
// Use the QuickTime Graphics Importer to open and draw our image, and use
// GraphicsImportSetFlags to specify whether or not to use automatic 
// ColorSync color-matching when drawing
//
//////////

static void openWindowForFile( FSSpec *fileSpec )
{
	GraphicsImportComponent gi = 0;
    OSErr err = noErr;
    WindowRef window = NULL;
    Rect naturalBounds, boundsRect, windowBounds;
	RgnHandle clipRgn = NULL;
	FSRef fileRef;
	HFSUniStr255 unicodeFileName;
	CFStringRef fileName = NULL;
	CFMutableStringRef windowTitle = NULL;

	// find a QuickTime graphics importer to open our file
    err = GetGraphicsImporterForFile( fileSpec, &gi );
    if( err ) goto bail;
	
	// rotate the image 30 degrees if the user has set the 
	// "Apply 30 Rotation Matrix" check box in the window 
	// options
	if( gRotate30Degrees ) {
		MatrixRecord matrix;
		SetIdentityMatrix( &matrix );
		RotateMatrix( &matrix, 30L<<16, 0, 0 );
		GraphicsImportSetMatrix( gi, &matrix );
		GraphicsImportGetBoundsRect( gi, &boundsRect );
		TranslateMatrix( &matrix, ((Fixed)-boundsRect.left)<<16, ((Fixed)-boundsRect.top)<<16 );
		GraphicsImportSetMatrix( gi, &matrix );
		if( err ) goto bail;
	}
    
	// get the bounding rectangle of the image
	err = GraphicsImportGetNaturalBounds( gi, &naturalBounds );
    if( err ) goto bail;
    
	// get the bounding rectangle for _drawing_ the image,
	// this takes into account any transformation matrix
	// used to draw the image
	err = GraphicsImportGetBoundsRect( gi, &boundsRect );
    if( err ) goto bail;
    
    windowBounds = boundsRect;
    
	// create a window (the size of the image bounds rect)
	// to draw our image
    err = CreateNewWindow( kDocumentWindowClass, 
        kWindowCloseBoxAttribute | kWindowAsyncDragAttribute | kWindowCollapseBoxAttribute | 
        kWindowNoUpdatesAttribute | kWindowStandardHandlerAttribute, 
        &windowBounds, &window );
    if( err ) goto bail;
    
    err = RepositionWindow( window, NULL, kWindowCascadeOnMainScreen );
    if( err ) goto bail;
    
	err = FSpMakeFSRef( fileSpec, &fileRef );
	if( err ) goto bail;
	
	// we need the file name to display in our window
	err = FSGetCatalogInfo( &fileRef, kFSCatInfoNone, NULL, &unicodeFileName, NULL, NULL );
	if( err ) goto bail;
	
	// set the window title to show the drawing options concatenated 
	// together
	fileName = CFStringCreateWithCharacters( NULL, unicodeFileName.unicode, unicodeFileName.length );
	windowTitle = CFStringCreateMutableCopy( NULL, 0, fileName );
	CFStringAppendFormat( windowTitle, NULL, CFSTR( "%s%s%s%s" ), 
			gUseColorSyncMatching ? " + ColorSync Matching" : " (ColorSync Disabled)",
			gUseSourceExtraction ? " + Source Extraction" : "",
			gClipUsingClipRegion ? " + Clip Region" : "",
			gRotate30Degrees ? " + Rotated" : "" );
	SetWindowTitleWithCFString( window, windowTitle );
    SetWindowProxyFSSpec( window, fileSpec );
    SetWindowModified( window, false );
    
    MacShowWindow( window );
	SetPortWindowPort( window );
    
	// use source extraction when drawing (if the check box is
	// set by the user) - this means well just draw a specific 
	// portion of the image
    if( gUseSourceExtraction ) {
        Rect sourceRect = naturalBounds;
        
		if( naturalBounds.right == 600 && naturalBounds.bottom == 400 ) {
			sourceRect.left += 70;
			sourceRect.top += 90;
			sourceRect.right -= 90;
			sourceRect.bottom -= 110;
		}
		else {
			InsetRect( &sourceRect, 40, 40 );
		}
        
        err = GraphicsImportSetSourceRect( gi, &sourceRect );
        if( err ) goto bail;
    }
    
	// ColorSync color-matching is turned on by default in Panther for
	// QuickTime drawing via GraphicsImportDraw -- if you don't want this,
	// you must tell the graphics importer by calling GraphicsImportSetFlags
    if( ! gUseColorSyncMatching ) {
        err = GraphicsImportSetFlags( gi, kGraphicsImporterDontUseColorMatching );
        if( err ) goto bail;
    }
	
	// if the user has set the "Clip Using Clip Region" check box
	// then well build a clip region using some fancy mathematics
	// and finally call GraphicsImportSetClip to specify the clip
	// region to use when drawing
	if( gClipUsingClipRegion ) {
		int i, count = 200;
		float xfreq = 5.0, yfreq = 3.0, xphase = 0.2;
		clipRgn = NewRgn();
		OpenRgn();
		FrameRect( &boundsRect );
		MoveTo( boundsRect.right  * (1.0 + cos(xphase)) / 2.0, 
				boundsRect.bottom * (1.0 + sin(0)) / 2.0 );
		for( i = 0; i <= count; i++ ) {
			float t = (float)i * 2 * pi / (float)count;
			LineTo( boundsRect.right  * (1.0 + cos(xfreq * t + xphase)) / 2.0, 
					boundsRect.bottom * (1.0 + sin(yfreq * t)) / 2.0 );
		}
		CloseRgn( clipRgn );
		err = GraphicsImportSetClip( gi, clipRgn );
		if( err ) goto bail;
	}
	
	// tell the graphics importer to draw into our window
	err = GraphicsImportSetGWorld( gi, GetWindowPort( window ), NULL );
    if( err ) goto bail;
    
	// draw the image!
    err = GraphicsImportDraw( gi );
    if( err ) goto bail;
    
bail:
	// clean-up
    CloseComponent( gi );
	DisposeRgn( clipRgn );
	if( windowTitle ) CFRelease( windowTitle );
	if( fileName ) CFRelease( fileName );
    return;
}

//////////
//
// openWindowsForAEDescItems
// 
// Opens a window for each AE typeFSS descriptor record
//
//////////

static OSErr openWindowsForAEDescItems( AEDescList descList )
{
	OSErr err = noErr;
	FSSpec fileSpec;
	long index;
	long itemsInList = 0;
	AEKeyword keyword;
	DescType actualType;
	Size actualSize;
	
	err = AECountItems( &descList, &itemsInList );
	if (err) goto bail;
	
	for( index = 1; index <= itemsInList; index++ ) {
		err = AEGetNthPtr( &descList, index, typeFSS, &keyword, &actualType, &fileSpec,
								sizeof(fileSpec), &actualSize );
		if (err) goto bail;
		
		openWindowForFile( &fileSpec );
		if (err) goto bail;
	}
	
bail:
	return err;
}

//////////
//
// DoAEOpenDocument
// 
// Our AppleEvent handler
//
//////////

static pascal OSErr 
DoAEOpenDocument(
	const AppleEvent *message, 
	AppleEvent *reply, 
	long refcon )
{
	OSErr       err;
	AEDescList  docList;

	docList.dataHandle = nil;

	err = AEGetParamDesc( message, keyDirectObject, typeAEList, &docList );
	if( err ) goto bail;
	
	err = openWindowsForAEDescItems( docList );
	if( err ) goto bail;

bail:
	if( docList.dataHandle )
		AEDisposeDesc( &docList );
	
	return err;
}

//////////
//
// promptForAndOpenFiles
// 
//////////

static void promptForAndOpenFiles(void)
{
	OSErr err = noErr;
	NavReplyRecord reply = {0};
	NavDialogOptions options;
	NavObjectFilterUPP objectFilterUPP = nil;

	NavGetDefaultDialogOptions( &options );

	err = NavGetFile( NULL, &reply, &options, NULL, NULL, objectFilterUPP, NULL, NULL );

	if( !err && ( reply.validRecord == false ) )
		err = userCanceledErr;
	
	if( !err )
		err = openWindowsForAEDescItems( reply.selection );
	
	NavDisposeReply( &reply );
}

enum {
	kMyCommandUseColorSyncMatching = 'Matc',
	kMyCommandUseSourceExtraction = 'Crop',
	kMyCommandClipUsingClipRegion = 'Clip',
	kMyCommandRotate30Degrees = 'Ro30'
};

WindowRef gOptionsWindow = NULL;

//////////
//
// updateOptionsWindowCheckBoxes
// 
// Update the options window check boxes based on global settings
//
//////////

static void updateOptionsWindowCheckBoxes(void)
{
	ControlRef	control;
	OSStatus	err;
	ControlID	kMyCommandUseColorSyncMatchingID = { kMyCommandUseColorSyncMatching, 0 };
	ControlID	kMyCommandUseSourceExtractionID = { kMyCommandUseSourceExtraction, 0 };
	ControlID	kMyCommandClipUsingClipRegionID = { kMyCommandClipUsingClipRegion, 0 };
	ControlID	kMyCommandRotate30DegreesID = { kMyCommandRotate30Degrees, 0 };
	
	err = GetControlByID( gOptionsWindow, &kMyCommandUseColorSyncMatchingID, &control );
	require_noerr( err, CantGetControlByID );
	SetControlValue( control, gUseColorSyncMatching );
	
	err = GetControlByID( gOptionsWindow, &kMyCommandUseSourceExtractionID, &control );
	require_noerr( err, CantGetControlByID );
	SetControlValue( control, gUseSourceExtraction );
	
	err = GetControlByID( gOptionsWindow, &kMyCommandClipUsingClipRegionID, &control );
	require_noerr( err, CantGetControlByID );
	SetControlValue( control, gClipUsingClipRegion );
	
	err = GetControlByID( gOptionsWindow, &kMyCommandRotate30DegreesID, &control );
	require_noerr( err, CantGetControlByID );
	SetControlValue( control, gRotate30Degrees );
	
CantGetControlByID:
	return;
}

//////////
//
// handleCommand
// 
// Handle command events
//
//////////

static OSStatus handleCommand( HICommand hiCommand )
{
	OSStatus err = noErr;
	
	switch ( hiCommand.commandID )
	{
		case kHICommandOpen:
			promptForAndOpenFiles();
			err = noErr;
			break;

		case kMyCommandUseColorSyncMatching:
			gUseColorSyncMatching = ! gUseColorSyncMatching;
			updateOptionsWindowCheckBoxes();
			break;

		case kMyCommandUseSourceExtraction:
			gUseSourceExtraction = ! gUseSourceExtraction;
			updateOptionsWindowCheckBoxes();
			break;

		case kMyCommandClipUsingClipRegion:
			gClipUsingClipRegion = ! gClipUsingClipRegion;
			updateOptionsWindowCheckBoxes();
			break;

		case kMyCommandRotate30Degrees:	
			gRotate30Degrees = ! gRotate30Degrees;
			updateOptionsWindowCheckBoxes();
			break;

		default:
			err = eventNotHandledErr;
			break;
	}
	return err;
}

//////////
//
// applicationEventHandler
// 
//////////

static pascal OSStatus applicationEventHandler(
	EventHandlerCallRef	inHandlerCallRef,
	EventRef			inEvent,
	void*				inUserData )
{
#pragma unused( inHandlerCallRef, inUserData )
	OSStatus		err = eventNotHandledErr;
	UInt32			eventClass = GetEventClass( inEvent );
	UInt32			eventKind = GetEventKind( inEvent );
	HICommand		hiCommand;

	// We are only registered for the kEventClassCommand/kEventProcessCommand pair

	if ( eventClass == kEventClassCommand && eventKind == kEventProcessCommand )
	{
		err = GetEventParameter( inEvent, kEventParamDirectObject, typeHICommand,
			NULL, sizeof( HICommand ), NULL, &hiCommand );
		require_noerr( err, CantGetEventParameter );
		
		err = handleCommand( hiCommand );
	}
	else if ( eventClass == kEventClassCommand && eventKind == kEventCommandUpdateStatus )
	{
		err = GetEventParameter( inEvent, kEventParamDirectObject, typeHICommand,
			NULL, sizeof( HICommand ), NULL, &hiCommand );
		require_noerr( err, CantGetEventParameter );
		
		switch ( hiCommand.commandID )
		{
			case kMyCommandUseColorSyncMatching:
				SetMenuCommandMark( NULL, kMyCommandUseColorSyncMatching, gUseColorSyncMatching ? kMenuCheckmarkGlyph : 0 );
				break;
	
			case kMyCommandUseSourceExtraction:
				SetMenuCommandMark( NULL, kMyCommandUseSourceExtraction, gUseSourceExtraction ? kMenuCheckmarkGlyph : 0 );
				break;
	
			case kMyCommandClipUsingClipRegion:
				SetMenuCommandMark( NULL, kMyCommandClipUsingClipRegion, gClipUsingClipRegion ? kMenuCheckmarkGlyph : 0 );
				break;
	
			case kMyCommandRotate30Degrees:	
				SetMenuCommandMark( NULL, kMyCommandRotate30Degrees, gRotate30Degrees ? kMenuCheckmarkGlyph : 0 );
				break;
	
			default:
				err = eventNotHandledErr;
				break;
		}
	}
	
CantGetEventParameter:
	return err;
}

DEFINE_ONE_SHOT_HANDLER_GETTER( applicationEventHandler );

//////////
//
// optionsWindowEventHandler
// 
//////////

pascal OSStatus optionsWindowEventHandler(
	EventHandlerCallRef	inHandlerCallRef,
	EventRef			inEvent,
	void*				inUserData )
{
#pragma unused( inHandlerCallRef )
	HICommand			hiCommand;
	OSStatus			err = noErr;
	UInt32				eventClass = GetEventClass( inEvent );
	UInt32				eventKind = GetEventKind( inEvent );
	WindowRef			window = (WindowRef) inUserData;

	if ( window != gOptionsWindow ) goto WrongWindow;
	
	if ( eventClass == kEventClassCommand && eventKind == kEventProcessCommand )
	{
		err = GetEventParameter( inEvent, kEventParamDirectObject, typeHICommand,
			NULL, sizeof( HICommand ), NULL, &hiCommand );
		require_noerr( err, CantGetEventParameter );
		
		err = handleCommand( hiCommand );
	}
	
CantGetEventParameter:
WrongWindow:
	return err;
}

DEFINE_ONE_SHOT_HANDLER_GETTER( optionsWindowEventHandler );

//////////
//
// InitAppleEvents
// 
//////////

static void
InitAppleEvents(void)
{
	const long noRefCon = 0;
	OSErr aevtErr;
	
	aevtErr = AEInstallEventHandler( kCoreEventClass, kAEOpenDocuments,   NewAEEventHandlerUPP(DoAEOpenDocument), noRefCon, false );
}

//////////
//
// main
// 
//////////

int main(int argc, char* argv[])
{
    IBNibRef 		nibRef;
    EventTypeSpec	appEventList[] = { { kEventClassCommand, kEventProcessCommand }, 
									   { kEventClassCommand, kEventCommandUpdateStatus } };
    EventTypeSpec	windowEventList[] = { { kEventClassCommand, kEventProcessCommand } };
	OSStatus		err;

    // Create a Nib reference passing the name of the nib file (without the .nib extension)
    // CreateNibReference only searches into the application bundle.
    err = CreateNibReference(CFSTR("main"), &nibRef);
    require_noerr( err, CantGetNibRef );
    
    // Once the nib reference is created, set the menu bar. "MainMenu" is the name of the menu bar
    // object. This name is set in InterfaceBuilder when the nib is created.
    err = SetMenuBarFromNib(nibRef, CFSTR("MenuBar"));
    require_noerr( err, CantSetMenuBar );
	
    // Create an options window.
    err = CreateWindowFromNib( nibRef, CFSTR( "OptionsWindow" ), &gOptionsWindow );
    require_noerr( err, CantCreateWindow );

    InitAppleEvents();
    
    // We don't need the nib reference anymore.
    DisposeNibReference(nibRef);
    
    // Install the application event handler
	err = InstallApplicationEventHandler( GetapplicationEventHandlerUPP(),
			GetEventTypeCount( appEventList ), appEventList, NULL, NULL );
	require_noerr( err, CantInstallAppEventHandler );
    
    // Install the window event handler
	err = InstallWindowEventHandler( gOptionsWindow, GetoptionsWindowEventHandlerUPP(),
			GetEventTypeCount( windowEventList ), windowEventList, gOptionsWindow, NULL );
	require_noerr( err, CantInstallWindowEventHandler );
    
	updateOptionsWindowCheckBoxes();
	ShowWindow( gOptionsWindow );
	
    // Call the event loop
    RunApplicationEventLoop();

CantSetMenuBar:
CantGetNibRef:
CantCreateWindow:
CantInstallAppEventHandler:
CantInstallWindowEventHandler:
	return err;
}

