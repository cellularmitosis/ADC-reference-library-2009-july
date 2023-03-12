/*
File: UIHandling.c

Abstract: Basic RunApplicationEventLoop-based UI handling, Carbon API.

Version: 1.0

Disclaimer: IMPORTANT:  This Apple software is supplied to you by 
Apple Inc. ("Apple") in consideration of your agreement to the
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
Neither the name, trademarks, service marks or logos of Apple Inc. 
may be used to endorse or promote products derived from the Apple
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

Copyright (C) 2007 Apple Inc. All Rights Reserved.

*/

#include "MyCarbonPrinting.h"
#include "AppDrawing.h"
#include "NavServicesHandling.h"
#include "UIHandling.h"

#define kMyApplicationSignature		'DGPT'
#define kMyPrintInfoProperty		'CGDT'

static OSStatus Initialize(void);
static OSErr QuitAppleEventHandler(const AppleEvent *appleEvt, AppleEvent* reply, long refcon);
static OSErr PrintDocumentAppleEventHandler( const AppleEvent *inputEvent, AppleEvent *outputEvent, SInt32 handlerRefCon);
static OSStatus InstallAppEvents(void);

static OSStatus MyDrawEventHandler(EventHandlerCallRef nextHandler, EventRef event, void *userData);
static OSStatus MyWindowEventHandler(EventHandlerCallRef nextHandler, EventRef event, void* userData);
static OSStatus DoAppCommandProcess(EventHandlerCallRef nextHandler, EventRef theEvent, void* userData);

static void setUseSheets(Boolean doSheets);
static void setShowStatusDialogOnSave(Boolean doStatus);
static IBNibRef getOurNibRef(void);

int main(int argc, char* argv[])
{
#pragma unused (argc, argv)
    OSStatus		err = noErr;
    IBNibRef		nibRef = getOurNibRef();
	require( nibRef, CantGetNib );

	// Once the nib reference is created, set the menu bar. "MainMenu" is the name of the menu bar
	// object. This name is set in InterfaceBuilder when the nib is created.
	err = SetMenuBarFromNib(nibRef, CFSTR("MenuBar"));
	require_noerr( err, CantSetMenuBar );

	SetMenuCommandMark(NULL, kHICommandUseSheets, useSheets() ? checkMark : noMark);
	SetMenuCommandMark(NULL, kHICommandShowStatusDialogOnSave, showStatusDialogOnSave() ? checkMark : noMark);
	
	// Install our apple event handlers
	err = Initialize();
	require_noerr( err, CantInitialize );

	// open a document
	err = MakeNewDocument();
	require_noerr( err, CantOpenDocument );
	
	// Call the event loop
	RunApplicationEventLoop();

CantGetNib:
CantSetMenuBar:
CantInitialize:
CantOpenDocument:

	return err;
}

static IBNibRef getOurNibRef(void)
{
    /* the nibRef is a static local global and is only created the first
	time this routine is called but is needed repeatedly during the life
	of our application. getOurNibRef is an accessor for that data.
    */
    static IBNibRef nibRef = NULL;
    if(nibRef == NULL) {
        /* Create a Nib reference passing the name of the nib file (without the .nib extension)
            CreateNibReference only searches into the application bundle.
            
            We need this nib for the life of our application
        */
        OSStatus err = CreateNibReference(CFSTR("main"), &nibRef);
        if(err)
            nibRef = NULL;
    }
    return nibRef;
}

static OSStatus Initialize()
{
    OSErr err = noErr;
    
 // install AE handlers
    if(!err)
		err = AEInstallEventHandler(kCoreEventClass, kAEPrintDocuments, NewAEEventHandlerUPP(PrintDocumentAppleEventHandler), 0, false);
    if(!err)
		err = AEInstallEventHandler(kCoreEventClass, kAEQuitApplication, NewAEEventHandlerUPP(QuitAppleEventHandler), 0, false);
    if (!err)
        err = InstallAppEvents();
                
    return err;
}

// Declare the signature and field ID for our HIView. These must
// match what you assigned to the HIView in Interface Builder.
#define kMyHIViewSignature  'vFpd'
#define kMyHIViewFieldID     135

WindowRef MakeWindow(const DrawDataRef ourData)
{
	OSStatus err = noErr;
	WindowRef	window = NULL;	// initialize to NULL
	HIViewRef	myView;
	static EventHandlerUPP windowEventHandler = NULL;
	static EventHandlerUPP viewEventHandler = NULL;
	
	if( !windowEventHandler)
		windowEventHandler = NewEventHandlerUPP(MyWindowEventHandler);
		
	if( !viewEventHandler )
		viewEventHandler = NewEventHandlerUPP(MyDrawEventHandler);

	static const HIViewID		myViewID = { kMyHIViewSignature, kMyHIViewFieldID };
	// Declare the event class and kind for the Carbon events of interest.
	static const EventTypeSpec	kMyViewEvents[] = {{ kEventClassControl, kEventControlDraw }};					
	static const EventTypeSpec	myWindowEvents[] = {{ kEventClassWindow, kEventWindowClose }};
	// window event handler
	// view event handler

	if(!err)
	    err = CreateWindowFromNib(getOurNibRef(), CFSTR("Window"), &window);

	if(!err) {
	    EventHandlerRef	ref;
	    err = InstallWindowEventHandler(window, windowEventHandler, GetEventTypeCount(myWindowEvents), myWindowEvents, NULL, &ref);
	    if(!err) {
			// Get the HIView of the requested ID associated with the window.
			HIViewFindByID( HIViewGetRoot( window ), myViewID, &myView );
		
			// Install the event handler for the HIView.															
			err = HIViewInstallEventHandler(myView, viewEventHandler, GetEventTypeCount(kMyViewEvents), kMyViewEvents, ourData, NULL);
	    }

	    // attach our document data to the window
	    if(!err)
			err = SetWindowProperty(window, kMyApplicationSignature, kMyPrintInfoProperty, sizeof(ourData), &ourData);

	    if(err) {
			DisposeWindow(window);
			window = NULL;
	    }
	}
    return window;
}

static OSStatus InstallAppEvents(void)
{
	EventTypeSpec	eventType = { kEventClassCommand, kEventCommandProcess };
	OSStatus		err = noErr;
	// Command-process event handler
	EventHandlerUPP appCommandProcess = NewEventHandlerUPP(DoAppCommandProcess);  	
	err = InstallApplicationEventHandler(appCommandProcess, 1, &eventType, NULL, NULL);

	return err;
}

static OSStatus MyWindowEventHandler(EventHandlerCallRef nextHandler, 
                                                EventRef event, void* userData)
{
#pragma unused (nextHandler, userData)
    WindowRef	window;
    OSStatus	result = eventNotHandledErr;
    
    GetEventParameter(event, kEventParamDirectObject, typeWindowRef, NULL, sizeof(window), NULL, &window);

    switch(GetEventKind(event))
	{
        case kEventWindowClose:
            (void)DisposeWindowPrivateData(GetMyWindowProperty(window));
            DisposeWindow(window);
            result = noErr;
            break;
	    
		default:
			break;
    }
    return result;
}

// Handle command-process events at the application level
static OSStatus DoAppCommandProcess(EventHandlerCallRef nextHandler, 
                                            EventRef theEvent, void* userData)
{
#pragma unused (nextHandler, userData)
    HICommand  	aCommand;
    OSStatus	result = eventNotHandledErr;
	WindowRef	w;
	void		*ourDataP;

    GetEventParameter(theEvent, kEventParamDirectObject, typeHICommand, NULL, 
				    sizeof(HICommand), NULL, &aCommand);
    
    switch (aCommand.commandID)
    {
		case kHICommandNew:
			(void)MakeNewDocument();
			result = noErr;
			break;

		case kHICommandSave:
			{
				w = ActiveNonFloatingWindow();
				ourDataP = GetMyWindowProperty(w);
				if(ourDataP) {
					// Save as PDF
					(void)DoSaveAsFile(w, ourDataP, false );
					result = noErr;
				}
			}
			break;

		case kHICommandSaveAsPostScript:
			{
				w = ActiveNonFloatingWindow();
				ourDataP = GetMyWindowProperty(w);
				if(ourDataP) {
					// Save as PDF
					(void)DoSaveAsFile(w, ourDataP, true );
					result = noErr;
				}
			}
			break;

		case kHICommandPageSetup:
		{
			w = ActiveNonFloatingWindow();
			ourDataP = GetMyWindowProperty(w);
			if(ourDataP) {
				(void)DoPageSetup(w, ourDataP);
				result = noErr;
			}
		}
			break;

		case kHICommandPrint:
		{
			w = ActiveNonFloatingWindow();
			ourDataP = GetMyWindowProperty(w);
			if(ourDataP) {
				(void)DoPrint(w, ourDataP, kDoPrintWithPrintDialog);
				result = noErr;
			}
		}
			break;

		case kMyMenuCommandPrintOne:
		{
			w = ActiveNonFloatingWindow();
			ourDataP = GetMyWindowProperty(w);
			if(ourDataP) {
				(void)DoPrint(w, ourDataP, kDoPrintOne);
				result = noErr;
			}
		}
			break;


		case kHICommandUseSheets:
		{
			Boolean doSheets = useSheets();
			doSheets = !doSheets;
			setUseSheets(doSheets);
			SetMenuCommandMark(NULL, kHICommandUseSheets, doSheets ? checkMark : noMark);
		}
			break;
			
		case kHICommandShowStatusDialogOnSave:
		{
			Boolean doStatus = showStatusDialogOnSave();
			doStatus = !doStatus;
			setShowStatusDialogOnSave(doStatus);
			SetMenuCommandMark(NULL, kHICommandShowStatusDialogOnSave, doStatus ? checkMark : noMark);
		}
			break;

		case kHICommandQuit:
			QuitApplicationEventLoop();
			result = noErr;
			break;
		
		default:
			break;
    }
    return result;
}

static OSStatus MyDrawEventHandler(EventHandlerCallRef nextHandler, EventRef event, void *userData)
{
#pragma unused (nextHandler)
	OSStatus status = noErr;
	CGContextRef myContext;
	
	if(userData) {
	    HIRect  bounds;
	    // Get the bounding rectangle
	    HIViewRef theView;
		status = GetEventParameter( event, kEventParamDirectObject, typeControlRef, NULL, sizeof(theView), NULL, &theView );
		require_noerr( status, CantGetView );

		HIViewGetBounds (theView, &bounds);
	    
	    // Get the CGContextRef for the view.
	    if(status == noErr)
			status = GetEventParameter(event, kEventParamCGContextRef, typeCGContextRef, NULL, sizeof (CGContextRef), NULL, &myContext );		

	    if(status != noErr) {
			fprintf(stderr, "Got error %d getting the context!\n", (int)status);
			return status;
	    }
	    
	    // HIView's have a 'flipped' coordinate system. We'll unflip to get correct
	    // text.
	    
	    // Flip the coordinates by translating and scaling. This produces a
	    // coordinate system that matches the Quartz default coordinate system
	    // with the origin in the lower-left corner with the +y axis pointing up.
	    CGContextTranslateCTM(myContext, 0, bounds.size.height);
	    CGContextScaleCTM(myContext, 1.0, -1.0);

	    // this code always draws page 1 on screen
	    DrawPage(myContext, &bounds, 1, false, (DrawDataRef)userData);
	}
	
CantGetView:
	return status;   
}

/*------------------------------------------------------------------------------

    Function:	PrintDocumentAppleEventHandler
    
    Parameters:
        inputEvent	-	Apple Event to process
        outputEvent	-	returned event
        handlerRefCon - ref con
    Description:
       Process the high level kAEPrintDocuments Apple Event. A Print Settings is
       extracted from the Apple Event and coerced to a PMPrintSettings and a PMPrinter. 
       The coerced PMPrintSettings is used to print each document in the document list.
       The PMPrinter, if any, represents the desired target printer. We set the sessions
       printer to be that printer.
       
       This sample is limited to processing only the first 1024 bytes of a text file.

------------------------------------------------------------------------------*/
static OSErr
PrintDocumentAppleEventHandler( const AppleEvent *inputEvent, AppleEvent *outputEvent, SInt32 handlerRefCon)
{
#pragma unused (outputEvent, handlerRefCon)

    OSStatus	err = noErr;
    AEDesc		printSettingsDesc = {};
    AEDesc		printerDesc = {};
    AEDesc		aePrintSettingsDesc = {};
    PMPrintSettings	coercePrintSettings = kPMNoPrintSettings;
    PMPrinter	coercePrinter = NULL;
    SInt16		fileRefNum = 0;
    Boolean		showPrintDialog = false;
    AEDesc		booleanDesc = {};

    // See if we need to show the print dialog.
    err = AEGetParamDesc(inputEvent, kPMShowPrintDialogAEType, typeBoolean, &booleanDesc);
    if( !err )
        err = AEGetDescData( &booleanDesc, &showPrintDialog, sizeof( Boolean));

    // Get the AE print record.
    err = AEGetParamDesc(inputEvent, keyAEPropData, typeAERecord, &aePrintSettingsDesc);

    // Coerce it to a PMPrintSettings.
    if(!err)
        err = AECoerceDesc(&aePrintSettingsDesc, kPMPrintSettingsAEType, &printSettingsDesc);

    // Retrieve the coerced PMPrintSettings from the AEDesc.
    if(!err) {
        err = AEGetDescData( &printSettingsDesc, &coercePrintSettings, sizeof( void*));
        // We can now get rid of the AEDesc.
        err = AEDisposeDesc(&printSettingsDesc);
    }

    // Coerce it to a PMPrinter.
    if(!err) {
        err = AECoerceDesc(&aePrintSettingsDesc, kPMPrinterAEType, &printerDesc);

        // They may not have requested a target printer. But thats ok, so reset the error .
        if( !err ) {
            err = AEGetDescData( &printerDesc, &coercePrinter, sizeof( void*));
            // We can now get rid of the AEDesc.
            err = AEDisposeDesc(&printerDesc);
        } else
            err = noErr;
    }

    if( !err ) {
        long		index, itemsInList = 0;
        AEDescList	docList;
        char		buffer[96];
        ByteCount	readCount, count=sizeof(buffer);
        PMPrintSession session = NULL;
        PMPageFormat pageFormat = kPMNoPageFormat;

        // Create the session we'll use to print.
        PMCreateSession( &session );
        
        // Set the output to the target printer.
        if( coercePrinter )
            PMSessionSetCurrentPMPrinter( session, coercePrinter );

        // Create a default pageformat.
        PMCreatePageFormat(&pageFormat);
        PMSessionDefaultPageFormat(session, pageFormat);
            
         // Get the file list.
        err = AEGetParamDesc( inputEvent, keyDirectObject, typeAEList, &docList);
		if(!err)
			err = AECountItems( &docList, &itemsInList);			// how many files passed in
        
       // Walk the list of files.
        for (index = 1; !err && index <= itemsInList; index++)
        {
            FSRef			fileRef;
            AEKeyword		keywd;
            DescType		returnedType;
            Size			actualSize;
            CFStringRef		fileNameRef=NULL;
            Boolean			printIt = true;
            
            // Get the file ref.
            err = AEGetNthPtr( &docList, index, typeFSRef, &keywd, &returnedType, &fileRef, sizeof( fileRef ), &actualSize );

            // Get the file name to use in the window's title.
			if( !err )
				err = LSCopyDisplayNameForRef( &fileRef, &fileNameRef );

            // Open the file for reading.
            if( !err )
            {
                err = FSOpenFork(&fileRef, 0, NULL, fsRdPerm, &fileRefNum);
                if( !err )
                {
					count = sizeof(buffer);
                    // read the data (1024 max)
					FSReadFork(fileRefNum, fsFromStart, 0, count, &buffer[0], &readCount);
                    // Close file.
                    FSCloseFork(fileRefNum);
                
                    // Show the print dialog?
                    printIt = true;
                    if( showPrintDialog )
                        PMSessionPrintDialog( session, coercePrintSettings, pageFormat, &printIt );
        
                    // Print the file.
                    if( printIt )
                    {
                        CGContextRef printContext = NULL;
						PMSessionBeginCGDocument(session, coercePrintSettings, pageFormat);
						PMSessionBeginPage(session, pageFormat, NULL);
					
						PMSessionGetCGGraphicsContext(session, &printContext);
						CGContextSelectFont(printContext, "Helvetica", 20, kCGEncodingMacRoman);
						CGContextShowTextAtPoint(printContext, 72.0, 792 - 72, &buffer[0], count);
			
						PMSessionEndPage(session);
                        PMSessionEndDocument(session);
                    }
                }
            }
        }
       
        // Clean up.
        if( pageFormat != kPMNoPageFormat )
            PMRelease( pageFormat );
        if( session != NULL )
            PMRelease( session );
    }

    // We're done so get rid of everything.
    if( coercePrintSettings != kPMNoPrintSettings )
        PMRelease( coercePrintSettings );
    if( coercePrinter != NULL )
        PMRelease( coercePrinter );
    return err;
}

static OSErr QuitAppleEventHandler(const AppleEvent *appleEvt, AppleEvent* reply, long refcon)
{
#pragma unused (appleEvt, reply, refcon)
	QuitApplicationEventLoop();
	return noErr;
}

void *GetMyWindowProperty(WindowRef window)
{
    void *ourDataP = NULL;
    if(window) {
        UInt32 infoSize = sizeof(void *);
		OSStatus err = GetWindowProperty(window, kMyApplicationSignature, kMyPrintInfoProperty, infoSize, NULL, &ourDataP);
        if( err ) {
            ourDataP = NULL;
        }
    }
    return ourDataP;
}

void DoErrorAlert(OSStatus status, CFStringRef errorFormatString)
{	
    CFStringRef formatStr = NULL, printErrorMsg = NULL;
    SInt16      alertItemHit = 0;

	if ((status != noErr) && (status != kPMCancel))           
	{
		formatStr =  CFCopyLocalizedString(errorFormatString, NULL);	
		if (formatStr != NULL)
		{
			printErrorMsg = CFStringCreateWithFormat(NULL, NULL, formatStr, status);
			if (printErrorMsg != NULL)
			{
				DialogRef alert;
				AlertStdCFStringAlertParamRec alertParams = { 0 };
				alertParams.version = kStdCFStringAlertVersionOne;
				alertParams.movable = true;
				alertParams.defaultText = (CFStringRef)kAlertDefaultOKText;
				alertParams.defaultButton = kAlertStdAlertOKButton;
				alertParams.position = kWindowDefaultPosition;
				OSStatus err = CreateStandardAlert(kAlertStopAlert, printErrorMsg, NULL, &alertParams, &alert);
				if( !err )
					RunStandardAlert( alert, NULL, &alertItemHit );
				CFRelease (printErrorMsg);                     
			}
			CFRelease (formatStr);                             
		}
	}
}

static Boolean gDoSheets = false;

Boolean useSheets(void) {
    return gDoSheets;
}

static void setUseSheets(Boolean doSheets) {
    gDoSheets = doSheets;
}

static Boolean gDoStatusSaveDialog = false;

Boolean showStatusDialogOnSave(void) {
    return gDoStatusSaveDialog;
}

static void setShowStatusDialogOnSave(Boolean doStatus) {
    gDoStatusSaveDialog = doStatus;
}
