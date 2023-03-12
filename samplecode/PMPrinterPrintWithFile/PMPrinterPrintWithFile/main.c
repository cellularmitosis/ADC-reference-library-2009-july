/*

File:main.c

Abstract: Shows how to send files directly to the printer, including postscript and PDF

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

Copyright © 2006 Apple Computer, Inc., All Rights Reserved

*/ 

#include <Carbon/Carbon.h>

static OSStatus        AppEventHandler( EventHandlerCallRef inCaller, EventRef inEvent, void* inRefcon );
static OSStatus        HandleNew();
static OSStatus		   HandlePrint(CFURLRef fileURL, CFStringRef mimeType, PMPageFormat * pageFormat);
static OSStatus		   HandlePageSetup();
static OSStatus        WindowEventHandler( EventHandlerCallRef inCaller, EventRef inEvent, void* inRefcon );
static OSStatus		   DoPageSetupDialog(PMPrintSession printSession, PMPageFormat* pageFormat);

static IBNibRef        sNibRef;

CFStringRef psMimeType;
CFURLRef    psFileURL = NULL;
static PMPageFormat savedPageFormat = NULL;


//--------------------------------------------------------------------------------------------
int main(int argc, char* argv[])
{
    OSStatus                    err;
	CFBundleRef					mainBundle;
	
    static const EventTypeSpec    kAppEvents[] =
    {
        { kEventClassCommand, kEventCommandProcess }
    };

    // Create a Nib reference, passing the name of the nib file (without the .nib extension).
    // CreateNibReference only searches into the application bundle.
    err = CreateNibReference( CFSTR("main"), &sNibRef );
    require_noerr( err, CantGetNibRef );
    
    // Once the nib reference is created, set the menu bar. "MainMenu" is the name of the menu bar
    // object. This name is set in InterfaceBuilder when the nib is created.
    err = SetMenuBarFromNib( sNibRef, CFSTR("MenuBar") );
    require_noerr( err, CantSetMenuBar );
    
	mainBundle = CFBundleGetMainBundle();
	psFileURL = CFBundleCopyResourceURL(mainBundle, CFSTR("testps.ps"), NULL, NULL);
	
    // Install our handler for common commands on the application target
    InstallApplicationEventHandler( NewEventHandlerUPP( AppEventHandler ),
                                    GetEventTypeCount( kAppEvents ), kAppEvents,
                                    0, NULL );
    
	psMimeType = CFSTR("application/postscript");
    // Create a new window. A full-fledged application would do this from an AppleEvent handler
    // for kAEOpenApplication.
    HandleNew();
    
    // Run the event loop
    RunApplicationEventLoop();

CantSetMenuBar:
CantGetNibRef:
    return err;
}

//--------------------------------------------------------------------------------------------
static OSStatus
AppEventHandler( EventHandlerCallRef inCaller, EventRef inEvent, void* inRefcon )
{
    OSStatus    result = eventNotHandledErr;
    
    switch ( GetEventClass( inEvent ) )
    {
        case kEventClassCommand:
        {
            HICommandExtended cmd;
            verify_noerr( GetEventParameter( inEvent, kEventParamDirectObject, typeHICommand, NULL, sizeof( cmd ), NULL, &cmd ) );
            
            switch ( GetEventKind( inEvent ) )
            {
                case kEventCommandProcess:
                    switch ( cmd.commandID )
                    {
                        case kHICommandNew:
                            result = HandleNew();
                            break;
                            
                        // Add your own command-handling cases here
                        case kHICommandPageSetup:
							result = HandlePageSetup(&savedPageFormat);
							break;
							
						case kHICommandPrint:
							result = HandlePrint(psFileURL, psMimeType,&savedPageFormat);
							break;
							
                        default:
                            break;
                    }
                    break;
            }
            break;
        }
            
        default:
            break;
    }
    
    return result;
}

static OSStatus HandlePageSetup(PMPageFormat * pageFormat)
{
    OSStatus err = noErr;
    PMPrintSession  printSession = NULL;
    
    if ( PMCreateSession(&printSession) == noErr )
	{
     err = DoPageSetupDialog(printSession, pageFormat);
     PMRelease(printSession);
    }
	
	return err;
}

static OSStatus MyCreatePageFormat(PMPrintSession printSession, PMPageFormat *pageFormat)
{
        OSStatus status = PMCreatePageFormat(pageFormat);
            
        //  Note that PMPageFormat is not session-specific, but calling
        //  PMSessionDefaultPageFormat assigns values specific to the printer
        //  associated with the current printing session.
        if ((status == noErr) && (*pageFormat != kPMNoPageFormat))
            status = PMSessionDefaultPageFormat(printSession, *pageFormat);
      
  return status;
}

//-----------------------------------------------------------------------------------------------------------------------
// (Borrowed from /Developer/Examples/Printing/App/)
static OSStatus DoPageSetupDialog(PMPrintSession printSession, PMPageFormat* pageFormat)
{
    OSStatus  status = noErr;
    
    if (*pageFormat == kPMNoPageFormat)    // Set up a valid PageFormat object
    {
    MyCreatePageFormat(printSession, pageFormat);
    }
    else
    {
        status = PMSessionValidatePageFormat(printSession, *pageFormat, kPMDontWantBoolean);
    }

    if (status == noErr)            //  Display the Page Setup dialog
    {
        Boolean accepted;
        status = PMSessionPageSetupDialog(printSession, *pageFormat, &accepted);
        if (status == noErr && !accepted)
            status = kPMCancel;    // user clicked Cancel button
    }  
                                
    return status;
} // DoPageSetupDialog

//-----------------------------------------------------------------------------------------------------------------------
// (Borrowed from /Developer/Examples/Printing/App/)
static OSStatus DoPrintDialog(PMPrintSession printSession, PMPageFormat *pageFormat, PMPrintSettings *printSettings)
{
    OSStatus	status = noErr;
    Boolean     accepted;
    
    //	In this sample code the caller provides a valid PageFormat reference but in
    //	your application you may want to load and unflatten the PageFormat object
    //	that was saved at PageSetup time.  See LoadAndUnflattenPageFormat below.
    
    //	Set up a valid PrintSettings object.
    if (*printSettings == kPMNoPrintSettings)
    {
		status = PMCreatePrintSettings(printSettings);	
        check(status == noErr);
        // Note that PMPrintSettings is not session-specific, but calling
        // PMSessionDefaultPrintSettings assigns values specific to the printer
        // associated with the current printing session.
        if ((status == noErr) && (*printSettings != kPMNoPrintSettings))
            status = PMSessionDefaultPrintSettings(printSession, *printSettings);
        check(status == noErr);
    }
    else
    {
        status = PMSessionValidatePrintSettings(printSession, *printSettings, kPMDontWantBoolean);
        check(status == noErr);
    }

    //	Display the Print dialog.
    if (status == noErr)
    {
        status = PMSessionPrintDialog(printSession, *printSettings, *pageFormat, &accepted);
        check(status == noErr);
        if (status == noErr && !accepted)
            status = kPMCancel;		// user clicked Cancel button
    }

    return status;
	
}   // DoPrintDialog
  
static OSStatus HandlePrint(CFURLRef fileURL, CFStringRef mimeType, PMPageFormat * pageFormat)
{
    OSStatus status = noErr;
    PMPrintSession  printSession = NULL;
	PMPrintSettings printSettings = NULL;
	PMPrinter       currentPrinter = NULL;
	
	if ( PMCreateSession(&printSession) == noErr )
	{
		status = PMCreatePrintSettings(&printSettings);  
        if(status == noErr)
		{
			// Note that PMPrintSettings are not session-specific, but calling
			// PMSessionDefaultPrintSettings assigns values specific to the printer
			// associated with the current printing session.
			if ((status == noErr) && (printSettings != kPMNoPrintSettings))
				status = PMSessionDefaultPrintSettings(printSession, printSettings);
			if(status == noErr)
			{
				if (*pageFormat == NULL)
				  {
					status = MyCreatePageFormat(printSession, pageFormat);
				  }
				
				if (status == noErr && DoPrintDialog(printSession, pageFormat, &printSettings) == noErr)
							{
								PMDestinationType printDestination;
								
								// Verify that the destination is the printer
								status = PMSessionGetDestinationType(printSession,printSettings,&printDestination);
								if(status == noErr && printDestination == kPMDestinationPrinter)
								{
									status = PMSessionGetCurrentPrinter(printSession,&currentPrinter);
									if(status == noErr)
									{
										 /*    One reason this function (PMPrinterPrintWithFile) may fail is if the specified 
										 *    printer can not handle the file's mime type. Use PMPrinterGetMimeTypes() to
										 *    check whether a mime type is supported. 
										  Mime Type Examples 
										–application/postscript // We instert the PS for the PMPrintSettings into the PostScript stream.
										–application/vnd.cups-postscript // Raw postscript/finished postscript.  We don't insert anything
										–application/pdf // PDF document
										–image/gif, image/jpeg, image/tiff // Images
										–text/plain, text/rtf, text/html // Text
										–application/vnd.cups-raw //Raw printer commands and escape codes, mainly for the printer venders
										–And more...
										 */
										 CFArrayRef mimeTypes;
										 status = PMPrinterGetMimeTypes(currentPrinter,printSettings,&mimeTypes);
										 
										 if(status == noErr && mimeTypes != NULL)
										 {
											if(CFArrayContainsValue(mimeTypes,CFRangeMake(0, CFArrayGetCount(mimeTypes)),mimeType))
											{
											 status = PMPrinterPrintWithFile(currentPrinter,printSettings,*pageFormat,mimeType, fileURL);
											 }
											else {
												#define StringMax 256
												char mimeTypeCString[StringMax];
												
												if(CFStringGetCString(mimeType,mimeTypeCString,StringMax,kCFStringEncodingUTF8))
													printf("Mime type %s isn't supported by the printer\n",mimeTypeCString);
												else
												printf("Mime type wasn't supported by the printer\n");
											}
										 }
									}
								}
								
							}
			}
		}
	}
	return status;
}

//--------------------------------------------------------------------------------------------
DEFINE_ONE_SHOT_HANDLER_GETTER( WindowEventHandler )

//--------------------------------------------------------------------------------------------

static OSStatus
HandleNew()
{
    OSStatus                    err;
    WindowRef                    window;
    static const EventTypeSpec    kWindowEvents[] =
    {
        { kEventClassCommand, kEventCommandProcess }
    };
    
    // Create a window. "MainWindow" is the name of the window object. This name is set in 
    // InterfaceBuilder when the nib is created.
    err = CreateWindowFromNib( sNibRef, CFSTR("MainWindow"), &window );
    require_noerr( err, CantCreateWindow );

    // Install a command handler on the window. We don't use this handler yet, but nearly all
    // Carbon apps will need to handle commands, so this saves everyone a little typing.
    InstallWindowEventHandler( window, GetWindowEventHandlerUPP(),
                               GetEventTypeCount( kWindowEvents ), kWindowEvents,
                               window, NULL );
    
    // Position new windows in a staggered arrangement on the main screen
    RepositionWindow( window, NULL, kWindowCascadeOnMainScreen );
    
    // The window was created hidden, so show it
    ShowWindow( window );
    
CantCreateWindow:
    return err;
}

//--------------------------------------------------------------------------------------------
static OSStatus
WindowEventHandler( EventHandlerCallRef inCaller, EventRef inEvent, void* inRefcon )
{
    OSStatus    err = eventNotHandledErr;
    
    switch ( GetEventClass( inEvent ) )
    {
        case kEventClassCommand:
        {
            HICommandExtended cmd;
            verify_noerr( GetEventParameter( inEvent, kEventParamDirectObject, typeHICommand, NULL, sizeof( cmd ), NULL, &cmd ) );
            
            switch ( GetEventKind( inEvent ) )
            {
                case kEventCommandProcess:
                    switch ( cmd.commandID )
                    {
                        // Add your own command-handling cases here
                        
                        default:
                            break;
                    }
                    break;
            }
            break;
        }
            
        default:
            break;
    }
    
    return err;
}
