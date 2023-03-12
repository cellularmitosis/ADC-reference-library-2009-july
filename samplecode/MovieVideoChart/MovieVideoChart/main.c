/*

File: main.c

Abstract: Main source file for MovieVideoChart

Version: 1.0

© Copyright 2005 Apple Computer, Inc. All rights reserved.

IMPORTANT:  This Apple software is supplied to 
you by Apple Computer, Inc. ("Apple") in 
consideration of your agreement to the following 
terms, and your use, installation, modification 
or redistribution of this Apple software 
constitutes acceptance of these terms.  If you do 
not agree with these terms, please do not use, 
install, modify or redistribute this Apple 
software.

In consideration of your agreement to abide by 
the following terms, and subject to these terms, 
Apple grants you a personal, non-exclusive 
license, under Apple's copyrights in this 
original Apple software (the "Apple Software"), 
to use, reproduce, modify and redistribute the 
Apple Software, with or without modifications, in 
source and/or binary forms; provided that if you 
redistribute the Apple Software in its entirety 
and without modifications, you must retain this 
notice and the following text and disclaimers in 
all such redistributions of the Apple Software. 
Neither the name, trademarks, service marks or 
logos of Apple Computer, Inc. may be used to 
endorse or promote products derived from the 
Apple Software without specific prior written 
permission from Apple.  Except as expressly 
stated in this notice, no other rights or 
licenses, express or implied, are granted by 
Apple herein, including but not limited to any 
patent rights that may be infringed by your 
derivative works or by other works in which the 
Apple Software may be incorporated.

The Apple Software is provided by Apple on an "AS 
IS" basis.  APPLE MAKES NO WARRANTIES, EXPRESS OR 
IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED 
WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY 
AND FITNESS FOR A PARTICULAR PURPOSE, REGARDING 
THE APPLE SOFTWARE OR ITS USE AND OPERATION ALONE 
OR IN COMBINATION WITH YOUR PRODUCTS.

IN NO EVENT SHALL APPLE BE LIABLE FOR ANY 
SPECIAL, INDIRECT, INCIDENTAL OR CONSEQUENTIAL 
DAMAGES (INCLUDING, BUT NOT LIMITED TO, 
PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS 
OF USE, DATA, OR PROFITS; OR BUSINESS 
INTERRUPTION) ARISING IN ANY WAY OUT OF THE USE, 
REPRODUCTION, MODIFICATION AND/OR DISTRIBUTION OF 
THE APPLE SOFTWARE, HOWEVER CAUSED AND WHETHER 
UNDER THEORY OF CONTRACT, TORT (INCLUDING 
NEGLIGENCE), STRICT LIABILITY OR OTHERWISE, EVEN 
IF APPLE HAS BEEN ADVISED OF THE POSSIBILITY OF 
SUCH DAMAGE.

*/

#include <Carbon/Carbon.h>

#include "ChartView.h"
#include "ChartWindow.h"

static OSStatus openWindowsForAEDescItems( AEDescList descList )
{
	OSStatus err = noErr;
	FSRef fileRef;
	long index;
	long itemsInList = 0;
	AEKeyword keyword;
	DescType actualType;
	Size actualSize;
	
	err = AECountItems( &descList, &itemsInList );
	if (err) goto bail;
	
	for( index = 1; index <= itemsInList; index++ ) {
		err = AEGetNthPtr( &descList, index, typeFSRef, &keyword, &actualType, &fileRef,
								sizeof(FSRef), &actualSize );
		if (err) goto bail;
		
		CreateChartWindowForFile( &fileRef );
		if (err) goto bail;
	}
	
bail:
	return err;
}

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

static void promptForAndOpenFiles(void)
{
	OSStatus err = noErr;
	NavDialogCreationOptions navDialogOptions = {0};
	NavDialogRef navDialog = NULL;
	NavReplyRecord navReply = {0};
	
	err = NavGetDefaultDialogCreationOptions( &navDialogOptions );
	require_noerr( err, bail );
	
	navDialogOptions.optionFlags |= kNavAllowMultipleFiles;

	err = NavCreateGetFileDialog( &navDialogOptions, NULL, NULL, NULL, NULL, NULL, &navDialog );
	require_noerr( err, bail );

	err = NavDialogRun( navDialog );
	require_noerr( err, bail );
	
	err = NavDialogGetReply( navDialog, &navReply );
	require_noerr( err, bail );
	
	if( !err && ( navReply.validRecord == false ) )
		err = userCanceledErr;
	
	if( !err )
		err = openWindowsForAEDescItems( navReply.selection );

bail:	
	NavDisposeReply( &navReply );
	NavDialogDispose( navDialog );
}

static OSStatus handleCommand( HICommand hiCommand )
{
	OSStatus err;
	
	switch ( hiCommand.commandID )
	{
		case kHICommandOpen:
			promptForAndOpenFiles();
			err = noErr;
			break;

		default:
			err = eventNotHandledErr;
			break;
	}
	return err;
}

static pascal OSStatus ApplicationEventHandler(
	EventHandlerCallRef	inHandlerCallRef,
	EventRef			inEvent,
	void*				inUserData )
{
#pragma unused( inHandlerCallRef, inUserData )
	OSStatus		err = eventNotHandledErr;
	UInt32			eventClass = GetEventClass( inEvent );
	UInt32			eventKind = GetEventKind( inEvent );
	HICommand		hiCommand;

	if ( eventClass == kEventClassCommand && eventKind == kEventProcessCommand )
	{
		err = GetEventParameter( inEvent, kEventParamDirectObject, typeHICommand,
			NULL, sizeof( HICommand ), NULL, &hiCommand );
		require_noerr( err, CantGetEventParameter );
		
		err = handleCommand( hiCommand );
	}
	
CantGetEventParameter:
	return err;
}

DEFINE_ONE_SHOT_HANDLER_GETTER( ApplicationEventHandler );

static void
InitAppleEvents(void)
{
	const long noRefCon = 0;
	
	AEInstallEventHandler( kCoreEventClass, kAEOpenDocuments, NewAEEventHandlerUPP(DoAEOpenDocument), noRefCon, false );
}

int main(int argc, char* argv[])
{
    IBNibRef 		nibRef;
    OSStatus		err;
    EventTypeSpec	appEventList[] = { { kEventClassCommand, kEventProcessCommand } };
	
	// Register our custom HIView subclass.
	err = ChartView_Register();
    require_noerr( err, CantRegisterChartView );

	InitAppleEvents();
	EnterMovies();

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
    
    // Install the application event handler
	err = InstallApplicationEventHandler( GetApplicationEventHandlerUPP(),
			GetEventTypeCount( appEventList ), appEventList, NULL, NULL );
	require_noerr( err, CantInstallAppEventHandler );
    
    // Call the event loop
    RunApplicationEventLoop();

CantInstallAppEventHandler:
CantCreateWindow:
CantSetMenuBar:
CantGetNibRef:
CantRegisterChartView:
	return err;
}

