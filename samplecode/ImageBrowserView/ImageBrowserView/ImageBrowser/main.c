/*
    File:		main.c
    
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

#if USE_HIFRAMEWORK
	#include "TImageBrowserView.h"
#else
	#include "ImageBrowserView.h"
#endif // USE_HIFRAMEWORK

//------------------------------------------------------------------------------
//	constants
//------------------------------------------------------------------------------
//
// HIArchive keys used in an ImageBrower document archive
#define kImageBrowserWindowArchiveKey	CFSTR("ImageBrowserWindowKey")

const HIViewID	kImageBrowserViewID	=	{ 'IMGB', 0 };

//------------------------------------------------------------------------------
//	prototypes
//------------------------------------------------------------------------------
//
static OSStatus CreateNewImageBrowserDocument();
static OSStatus SaveImageBrowserDocument(
	WindowRef			inDocumentWindow );
static OSStatus OpenImageBrowserDocument();
static OSStatus ApplicationEventHandler(
	EventHandlerCallRef	inCallRef,
	EventRef			inEvent,
	void*				inUserData );

//------------------------------------------------------------------------------
//	main
//------------------------------------------------------------------------------
//
int main( int argc, char* argv[] )
{
	OSStatus		err;
    IBNibRef		nibRef;
	EventTypeSpec	appEvents[] = { { kEventClassCommand, kEventCommandProcess } };
	
	// register our custom HIView before it is instantiated from the nib
#if USE_HIFRAMEWORK
	err = TImageBrowserView::RegisterClass();
#else
	err = ImageBrowserViewRegister();
#endif // USE_HIFRAMEWORK

	require_noerr( err, CantRegisterImageBrowserView );
	
	// install an application event handler for new, open and save file commands
	err = InstallApplicationEventHandler( ApplicationEventHandler, GetEventTypeCount( appEvents ), appEvents, NULL, NULL );
	require_noerr( err, CantInstallAppEventHandler );
	
    err = CreateNibReference( CFSTR("main"), &nibRef );
    require_noerr( err, CantGetNibRef );
    
    err = SetMenuBarFromNib( nibRef, CFSTR("MenuBar") );
    require_noerr( err, CantSetMenuBar );
    
    err = CreateNewImageBrowserDocument();
    require_noerr( err, CantCreateWindow );
    
    RunApplicationEventLoop();
	
CantCreateWindow:
CantSetMenuBar:
	
	DisposeNibReference( nibRef );
	
CantGetNibRef:
CantInstallAppEventHandler:
CantRegisterImageBrowserView:
	
	return err;
}

//------------------------------------------------------------------------------
//	CreateNewImageBrowserDocument
//------------------------------------------------------------------------------
//
static OSStatus CreateNewImageBrowserDocument()
{
	OSStatus			err;
	IBNibRef			nibRef;
	WindowRef			window;
	HIViewRef			view;
	CFIndex				count, index;
	CFMutableArrayRef	array;
	const CFStringRef	kImages[] =
						{
							CFSTR( "/Library/Desktop Pictures/Nature/Evening Reflections.jpg" ),
							CFSTR( "/Library/Desktop Pictures/Nature/Clown Fish.jpg" ),
							CFSTR( "/Library/Desktop Pictures/Nature/Flowing Rock.jpg" ),
							CFSTR( "/Library/Desktop Pictures/Nature/Gentle Rapids.jpg" ),
							CFSTR( "/Library/Desktop Pictures/Nature/Sweeping Current.jpg" )
						};
	
	err = CreateNibReference( CFSTR("main"), &nibRef );
	require_noerr( err, CantGetNibRef );
	
	err = CreateWindowFromNib( nibRef, CFSTR("MainWindow"), &window );
	require_noerr( err, CantCreateWindow );
	
	// allow the window to be archived
	err = HIObjectSetArchivingIgnored( (HIObjectRef)window, false );
	require_noerr( err, CantSetArchivingIgnored );
	
	// create an array of images
	array = CFArrayCreateMutable( kCFAllocatorDefault, 0, &kCFTypeArrayCallBacks );
	require_action( array != NULL, CantCreateArray, err = memFullErr );

	count = sizeof( kImages ) / sizeof( CFStringRef );
	for ( index = 0; index < count; index++ )
	{
		CFURLRef	url = CFURLCreateWithFileSystemPath( kCFAllocatorDefault, kImages[ index ], kCFURLPOSIXPathStyle, false );
		
		if ( url )
		{
			CFArrayAppendValue( array, url );
			CFRelease( url );
		}
	}

	// get the image browser view in this window
	verify_noerr( HIViewFindByID( HIViewGetRoot( window ), kImageBrowserViewID, &view ) );
	
#if USE_HIFRAMEWORK
	verify_noerr( TImageBrowserView::AddImages( view, array ) );
#else
	verify_noerr( ImageBrowserViewAddImages( view, array ) );
#endif // USE_HIFRAMEWORK
	
	CFRelease( array );
	
CantCreateArray:

	ShowWindow( window );
	
CantSetArchivingIgnored:
CantCreateWindow:
	
	DisposeNibReference( nibRef );
	
CantGetNibRef:
	
	return err;
}

//------------------------------------------------------------------------------
//	SaveImageBrowserDocument
//------------------------------------------------------------------------------
//
static OSStatus SaveImageBrowserDocument(
	WindowRef					inDocumentWindow )
{
	OSStatus					err = noErr;
	NavDialogCreationOptions	options;
	NavDialogRef				navDialog;
	NavReplyRecord				navReply;
	
	require( inDocumentWindow != NULL, NoWindowToArchive );
	
	err = NavGetDefaultDialogCreationOptions( &options );
	require_noerr( err, CantGetDefaultDialogOptions );
	
	options.saveFileName = CFSTR("Untitled.imagebrowser");
	options.optionFlags |= kNavPreserveSaveFileExtension;
	
	// find a place to put our archive
	err = NavCreatePutFileDialog( &options, kNavGenericSignature, kNavGenericSignature, NULL, NULL, &navDialog );
	require_noerr( err, CantCreatePutFileDialog );
	
	err = NavDialogRun( navDialog );
	require_noerr( err, CantRunSaveDialog );
	
	err = NavDialogGetReply( navDialog, &navReply );
	require( err == userCanceledErr || err == noErr, CantGetNavReply );
	
	if ( navReply.validRecord && err != userCanceledErr )
	{
		FSRef			fileRef;
		CFURLRef		directoryURL, fileURL;
		HIArchiveRef	encoder;
		CFDataRef		encodedData;
		
		// ask the nav reply where the file should go
		err = AEGetNthPtr( &navReply.selection, 1, typeFSRef, NULL, NULL, &fileRef, sizeof( FSRef ), NULL );
		require_noerr( err, CantGetFSRefFromSaveFileReplyDesc );
		
		directoryURL = CFURLCreateFromFSRef( kCFAllocatorDefault, &fileRef );
		require_action( directoryURL != NULL, CantCreateDirectoryURL, err = coreFoundationUnknownErr );
		
		fileURL = CFURLCreateCopyAppendingPathComponent( kCFAllocatorDefault, directoryURL, navReply.saveFileName, false );
		require_action( fileURL != NULL, CantCreateOpenFileURL, err = coreFoundationUnknownErr );
	
		err = HIArchiveCreateForEncoding( &encoder );
		require_noerr( err, CantCreateEncoder );
		
		// write the window into the archive
		err = HIArchiveEncodeCFType( encoder, kImageBrowserWindowArchiveKey, inDocumentWindow );
		require_noerr( err, CantEncodeWindow );
		
		err = HIArchiveCopyEncodedData( encoder, &encodedData );
		require_noerr( err, CantCopyEncodedData );
		
		// write the archived data out to disk
		require_action( CFURLWriteDataAndPropertiesToResource( fileURL, encodedData, NULL, NULL ),
							CantWriteDataToFile, err = coreFoundationUnknownErr );
		
CantWriteDataToFile:
		
		CFRelease( encodedData );
		
CantCopyEncodedData:
CantEncodeWindow:

		CFRelease( encoder );
		
CantCreateEncoder:
		
		CFRelease( fileURL );
		
CantCreateOpenFileURL:
		
		CFRelease( directoryURL );
	}
	
CantCreateDirectoryURL:
CantGetFSRefFromSaveFileReplyDesc:
CantGetNavReply:
	
	NavDialogDispose( navDialog );
	
CantRunSaveDialog:
CantCreatePutFileDialog:
CantGetDefaultDialogOptions:
NoWindowToArchive:
	
	return err;
}

//------------------------------------------------------------------------------
//	OpenImageBrowserDocument
//------------------------------------------------------------------------------
//
static OSStatus OpenImageBrowserDocument()
{
	OSStatus					err;
	NavDialogCreationOptions	options;
	NavDialogRef				navDialog;
	CFMutableArrayRef			identifiers;
	CFStringRef					imageBrowserUTI;
	NavReplyRecord				navReply;
	
	err = NavGetDefaultDialogCreationOptions( &options );
	require_noerr( err, CantGetDefaultChooseFileOptions );
	
	err = NavCreateChooseFileDialog( &options, NULL, NULL, NULL, NULL, NULL, &navDialog );
	require_noerr( err, CantCreateChooseFile );
	
	// Image Browser can open files with the ".imagebrowser" extension
	identifiers = CFArrayCreateMutable( kCFAllocatorDefault, 0, &kCFTypeArrayCallBacks );
	require_action( identifiers != NULL, CantCreateChooseFileFilterIdentifiers, err = coreFoundationUnknownErr );
	
	// create the image browser UTI conforming to "public.data" because it's a data file rather than a bundle 
	imageBrowserUTI = UTTypeCreatePreferredIdentifierForTag( kUTTagClassFilenameExtension, CFSTR("imagebrowser"),
																kUTTypeData );
	require_action( imageBrowserUTI != NULL, CantCreateImageBrowserUTI, err = coreFoundationUnknownErr );
	
	CFArrayAppendValue( identifiers, imageBrowserUTI );
	
	// filter by image browser UTI
	err = NavDialogSetFilterTypeIdentifiers( navDialog, identifiers );
	require_noerr( err, CantSetChooseFileFilterIdentifiers );
	
	// choose the file
	err = NavDialogRun( navDialog );
	require_noerr( err, CantRunChooseFileDialog );
	
	err = NavDialogGetReply( navDialog, &navReply );
	require( err == userCanceledErr || err == noErr, CantGetChooseFileReply );
	
	// create url to chosen file
	if ( navReply.validRecord && err != userCanceledErr )
	{
		FSRef			fileRef;
		CFURLRef		fileURL;
		CFDataRef		fileData = NULL;
		HIArchiveRef	decoder;
		WindowRef		window;
		
		err = AEGetNthPtr( &navReply.selection, 1, typeFSRef, NULL, NULL, &fileRef, sizeof( FSRef ), NULL );
		require_noerr( err, CantGetFSRefFromChooseFileReplyDesc );
				
		fileURL = CFURLCreateFromFSRef( kCFAllocatorDefault, &fileRef );
		require_action( fileURL != NULL, CantCreateOpenFileURL, err = coreFoundationUnknownErr );
		
		// load data from URL
		CFURLCreateDataAndPropertiesFromResource( kCFAllocatorDefault, fileURL, &fileData, NULL, NULL, NULL );
		require_action( fileData != NULL, CantCreateDataFromFile, err = coreFoundationUnknownErr );
		
		// unarchive the loaded data
		err = HIArchiveCreateForDecoding( fileData, 0, &decoder );
		require_noerr( err, CantCreateDecoder );
		
		// unarchive the image browser "document" window
		err = HIArchiveCopyDecodedCFType( decoder, kImageBrowserWindowArchiveKey, (CFTypeRef*)&window );
		require_noerr( err, CantDecodeWindow );
		
		ShowWindow( window );
		
		// don't release the created window, it will be released when closed
		
CantDecodeWindow:
		
		CFRelease( decoder );
		
CantCreateDecoder:
		
		CFRelease( fileData );
		
CantCreateDataFromFile:
		
		CFRelease( fileURL );
	}
	
	NavDisposeReply( &navReply );
	
CantCreateOpenFileURL:
CantGetFSRefFromChooseFileReplyDesc:
CantGetChooseFileReply:
CantRunChooseFileDialog:
CantSetChooseFileFilterIdentifiers:
	
	CFRelease( imageBrowserUTI );
	
CantCreateImageBrowserUTI:
	
	CFRelease( identifiers );
	
CantCreateChooseFileFilterIdentifiers:
	
	NavDialogDispose( navDialog );
	
CantCreateChooseFile:
CantGetDefaultChooseFileOptions:
	
	return err;
}

//------------------------------------------------------------------------------
//	ApplicationEventHandler
//------------------------------------------------------------------------------
//
static OSStatus ApplicationEventHandler(
	EventHandlerCallRef	inCallRef,
	EventRef			inEvent,
	void*				inUserData )
{
	OSStatus			err = eventNotHandledErr;
	
	if ( GetEventClass( inEvent ) == kEventClassCommand && GetEventKind( inEvent ) == kEventCommandProcess )
	{
		HICommandExtended command;
		
		// grab the command from the event
		require_noerr( GetEventParameter( inEvent, kEventParamDirectObject, typeHICommand, NULL, sizeof( HICommand ),
						NULL, &command ), CantGetHICommandParameter );
		
		if ( command.commandID == kHICommandNew ) // load a new image browser window from the nib
		{
			err = CreateNewImageBrowserDocument();
		}
		else if ( command.commandID == kHICommandSave ) // archive an image browser window to disk
		{
			// archive out the active window
			err = SaveImageBrowserDocument( ActiveNonFloatingWindow() );
		}
		else if ( command.commandID == kHICommandOpen ) // unarchive an image browser window from disk
		{
			err = OpenImageBrowserDocument();
		}
	}
	
CantGetHICommandParameter:
	
	return err;
}
