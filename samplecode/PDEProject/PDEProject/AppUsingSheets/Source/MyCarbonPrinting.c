/*
	File:		MyCarbonPrinting.c
	
	Contains:	Routines needed to perform printing. This example uses sheets and provides for
                        save as PDF.

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

	Copyright © 1999-2001 Apple Computer, Inc., All Rights Reserved
*/

#include "MyCarbonPrinting.h"
#include "AppDrawing.h"
#include "UIHandling.h"
#include "PDEHandling.h"


// set to 0 if you want printing status narration during save to PDF.
#define NO_SAVE_STATUS_DIALOG		1

#ifndef __APPLE_CC__
#define BUILD_FOR_CFM	1	// default for building with Metrowerks. 
                                // Should really be keyed off a MACH_O define of some sort
#endif

// the following typedefs are defined so that our PrintingProcs structure
// definition can be defined and used and so that we can call
// these routines from CFM code if needed
typedef CALLBACK_API(OSStatus, PMSessionBeginDocumentProcPtr)
				(PMPrintSession printSession,
                                 PMPrintSettings        printSettings,
                                 PMPageFormat           pageFormat);
typedef CALLBACK_API(OSStatus, PMSessionBeginPageProcPtr)
				(PMPrintSession printSession,
                                 PMPageFormat pageFormat,
                                 const PMRect *pageFrame);
typedef CALLBACK_API(OSStatus, PMSessionEndPageProcPtr)(PMPrintSession printSession);
typedef CALLBACK_API(OSStatus, PMSessionEndDocumentProcPtr)(PMPrintSession printSession);

// used so we can lookup a procptr for the PMSessionSetDestination call from CFM
typedef CALLBACK_API(OSStatus, PMSessionSetDestinationProcPtr)
					(PMPrintSession      printSession,
					PMPrintSettings     printSettings,
					PMDestinationType   destType,
					CFStringRef         destFormat,
					CFURLRef            destLocation);

typedef struct PrintingProcs{
    PMSessionBeginDocumentProcPtr	BeginDocumentProc;
    PMSessionBeginPageProcPtr		BeginPageProc;
    PMSessionEndPageProcPtr 		EndPageProc;
    PMSessionEndDocumentProcPtr 	EndDocumentProc;
}PrintingProcs;

/*
    The routines PMSessionBeginDocumentNoDialog, PMSessionEndDocumentNoDialog,
        PMSessionBeginPageNoDialog, PMSessionEndPageNoDialog are incorrectly
        missing from the public header file PMCore.h and consequently also
        from ApplicationServices.h. These routines have
        the following availability:
        
    Availability:
      Mac OS X:         in version 10.0 and later in ApplicationServices.framework
      CarbonLib:        not available
      Non-Carbon CFM:   not available

    These routines are public and will appear in the PMCore.h header file in
    a future release of Mac OS X.
    
    The functional difference between these routines and the matching routines
    without the 'NoDialog' suffix is that there is not window or status narration
    performed when these routines are used. This makes them suitable for our
    save to PDF situation where we may want either no status narration or
    an application might want to perform its own status narration.
*/
EXTERN_API( OSStatus )
PMSessionBeginDocumentNoDialog  (PMPrintSession         printSession,
                                 PMPrintSettings        printSettings,
                                 PMPageFormat           pageFormat);

EXTERN_API( OSStatus )
PMSessionEndDocumentNoDialog    (PMPrintSession         printSession);

EXTERN_API( OSStatus )
PMSessionBeginPageNoDialog      (PMPrintSession         printSession,
                                 PMPageFormat           pageFormat,
                                 const PMRect *         pageFrame);

EXTERN_API( OSStatus )
PMSessionEndPageNoDialog        (PMPrintSession         printSession);


/*** Local Function Prototypes ***/

static pascal void MyPageSetupDoneProc(PMPrintSession printSession, 
                                            WindowRef documentWindow, Boolean accepted);
static pascal void MyPrintDialogDoneProc(PMPrintSession printSession, 
                                            WindowRef documentWindow, Boolean accepted);

static OSStatus MyDoPrintLoop(PMPrintSession printSession, PMPageFormat pageFormat, 
				    PMPrintSettings printSettings, const void *docDataP,
                                    const PrintingProcs *printingProcsP);

/*
    For CFM apps we have to call through proc pointers we find. For 
    mach-o apps we can link to and call the actual routine
*/
#if BUILD_FOR_CFM
static OSStatus LoadFrameworkBundle(CFStringRef framework, CFBundleRef *bundlePtr);
static CFBundleRef getCorePrintingBundle();
static pascal OSStatus Call_PMSessionBeginDocumentNoDialog(PMPrintSession printSession,
                                 PMPrintSettings        printSettings,
                                 PMPageFormat           pageFormat);
static pascal OSStatus Call_PMSessionEndDocumentNoDialog(PMPrintSession printSession);
static pascal OSStatus Call_PMSessionBeginPageNoDialog(PMPrintSession printSession,
                                 PMPageFormat pageFormat,
                                 const PMRect *pageFrame);
static pascal OSStatus Call_PMSessionEndPageNoDialog(PMPrintSession printSession);
static pascal OSStatus Call_PMSessionSetDestination(PMPrintSession printSession,
					PMPrintSettings     printSettings,
					PMDestinationType   destType,
					CFStringRef         destFormat,
					CFURLRef            destLocation);
#else
#define  Call_PMSessionBeginDocumentNoDialog	PMSessionBeginDocumentNoDialog
#define  Call_PMSessionEndDocumentNoDialog	PMSessionEndDocumentNoDialog
#define  Call_PMSessionBeginPageNoDialog	PMSessionBeginPageNoDialog
#define  Call_PMSessionEndPageNoDialog		PMSessionEndPageNoDialog
#define	 Call_PMSessionSetDestination 		PMSessionSetDestination
#endif

/**** Global Data ****/
static const PrintingProcs gMyPrintingProcsWithStatusDialog = {
                                                PMSessionBeginDocument, 
						PMSessionBeginPage, 
						PMSessionEndPage,
                                                PMSessionEndDocument
					    };
#if NO_SAVE_STATUS_DIALOG
static const PrintingProcs gMyPrintingProcsNoStatusDialog = {
                                                Call_PMSessionBeginDocumentNoDialog, 
						Call_PMSessionBeginPageNoDialog, 
						Call_PMSessionEndPageNoDialog,
                                                Call_PMSessionEndDocumentNoDialog
					    };
#endif

// -----------------------------------------------------------------------
OSStatus CreateDefaultPageFormatForDocument(void *docDataP)
{
    OSStatus err = noErr, tempErr;
    PMPrintSession printSession;
    err = PMCreateSession(&printSession);
    if(!err){
	PMPageFormat pageFormat;
	err = PMCreatePageFormat(&pageFormat);	// we own a reference to this page format
	if(err == noErr)
	{
	    err = PMSessionDefaultPageFormat(printSession, pageFormat);
	    if (err == noErr){	
		// this routine will keep its own reference to the pageFormat
		err = SetPageFormatOnPrivateData(docDataP, pageFormat);	
	    }
	    tempErr = PMRelease(pageFormat);// release our reference obtained from PMCreatePageFormat
	    if(!err)err = tempErr;
	}
	tempErr = PMRelease(printSession);
	if(!err)err = tempErr;
    }
    return err;
}

// -----------------------------------------------------------------
OSStatus DoPageSetup(WindowRef window, void *docDataP)
{
    OSStatus		err = noErr;
    if(docDataP){
        PMPrintSession printSession;
        err = PMCreateSession(&printSession);
        if(!err){
            Boolean accepted;
            static PMSheetDoneUPP myPageSetupDoneProc = NULL;
            if(!myPageSetupDoneProc){
                myPageSetupDoneProc = NewPMSheetDoneUPP(MyPageSetupDoneProc);
                if(!myPageSetupDoneProc){
                    err = memFullErr;
                }
            }
            
            if(!err) 	// validate the page format we're going to pass to the dialog code
		err = PMSessionValidatePageFormat(printSession, GetPageFormatFromPrivateData(docDataP),
								    kPMDontWantBoolean);
            if(!err){
		Boolean sheetsAreAvailable = TRUE;
                err = PMSessionUseSheets(printSession, window, myPageSetupDoneProc);             
		if(err == kPMNotImplemented){
		    // sheets are not available (aka, Mac OS 8/9)
		    err = noErr;
		    sheetsAreAvailable = FALSE;
		}
                if(!err){
                    err = PMSessionPageSetupDialog(printSession, GetPageFormatFromPrivateData(docDataP), &accepted);
                    /*  when using sheets, the value of 'accepted' returned here is irrelevant
			since it is our sheet done proc that is called when the dialog is dismissed.
			Our dialog done proc will be called when the sheet is dismissed.
		    
			If sheets are NOT implemented then WE call our DialogDone proc here 
			to complete the dialog.
		    */
		    if(err == noErr && !sheetsAreAvailable)
                        MyPageSetupDoneProc(printSession, window, accepted);
		}
            }
            if(err){	// only if there is an error do we release the session,
                        // otherwise we'll do that in our sheet done proc
                (void)PMRelease(printSession);
            }
	}
    }
    DoErrorAlert(err, kMyPrintErrorFormatStrKey);
    return err;
} // DoPageSetup

// -------------------------------------------------------------------------------
static pascal void MyPageSetupDoneProc(PMPrintSession printSession,
                        WindowRef documentWindow,
                        Boolean accepted)
{
#pragma unused(documentWindow, accepted)
    // this sample doesn't do anything with the page format after page setup is done
    
    // now we release the session we created to do the page setup dialog
    OSStatus err = PMRelease(printSession);	
    if(err)
        DoErrorAlert(err, kMyPrintErrorFormatStrKey);
    return;
}


// -------------------------------------------------------------------------------
OSStatus DoPrint(WindowRef parentWindow, void *documentDataP, Boolean printOne)
{
    OSStatus err = noErr;
    PMPrintSettings printSettings = NULL;

    UInt32 minPage = 1, maxPage;
    PMPrintSession printSession;
    err = PMCreateSession(&printSession);
    if(err == noErr){
	// validate the page format we're going to use
	err = PMSessionValidatePageFormat(printSession, 
			    GetPageFormatFromPrivateData(documentDataP),
			    kPMDontWantBoolean);
        if (err == noErr)
        {
            err = PMCreatePrintSettings(&printSettings);
            if(err == noErr)
            {
                err = PMSessionDefaultPrintSettings(printSession, printSettings);
                if(err == noErr){
                    CFStringRef myDocumentNameRef;
                    err = CopyDocumentName(documentDataP, &myDocumentNameRef);
                    if(err == noErr)
                    {
                        // set the job name before displaying the print dialog
                        err = PMSetJobNameCFString (printSettings, myDocumentNameRef);
                        CFRelease(myDocumentNameRef);	// release our reference to the document name
                    }
                }
            }
            if (err == noErr)
            {
                /*
                    On Mac OS X, calling PMSetPageRange has the following benefits:
                    (a) sets the From/To settings in the print dialog to the range of pages 
                        in the document 
                        
                        AND 
                        
                    (b) the print dialog code enforces this so that users can't enter 
                        values outside this range.
                */
                maxPage = MyGetDocumentNumPagesInDoc(documentDataP);
                err = PMSetPageRange(printSettings, minPage, maxPage);
            }

            // register our custom PDE if we haven't already
            if (err == noErr){
                static Boolean initedPDE = false;
                if(!initedPDE){
                    err = MyRegisterPDE(kMyPDEName);
                    initedPDE = true;
                }
            }

            if (err == noErr)
            {
                Boolean accepted;
                static PMSheetDoneUPP myPrintDialogDoneProc = NULL;
                if(!myPrintDialogDoneProc){
                    myPrintDialogDoneProc = NewPMSheetDoneUPP(MyPrintDialogDoneProc);
                    if(!myPrintDialogDoneProc)
                        err = memFullErr;
                }

                if(!err){
                    Boolean sheetsAreAvailable = TRUE;
                    err = SetPrintSettingsOnPrivateData(documentDataP, printSettings);
                    if(!err){
                        err = PMSessionUseSheets(printSession, parentWindow, myPrintDialogDoneProc);
                        if(err == kPMNotImplemented){
                            // we get here if sheets are not available, i.e. Mac OS 8/9
                            err = noErr;
                            sheetsAreAvailable = FALSE;
                        }		
                        if(err == noErr && !printOne){
                            err = PMSessionPrintDialog(printSession, printSettings, 
                                            GetPageFormatFromPrivateData(documentDataP),
                                            &accepted);
                            /*  when using sheets, the value of 'accepted' returned here is irrelevant
                                since it is our sheet done proc that is called when the dialog is dismissed.
                                Our dialog done proc will be called when the sheet is dismissed.
                            
                                If sheets are NOT implemented then WE call our DialogDone proc here 
                                to complete the dialog.
                            */
                            if(err == noErr && !sheetsAreAvailable)
                                MyPrintDialogDoneProc(printSession, parentWindow, accepted);
                        }else{
                            // if we are doing print one we have no dialog, therefore
                            // we have to call our dialog done proc since there is no
                            // dialog to do so for us
                            if(err == noErr)
                                MyPrintDialogDoneProc(printSession, parentWindow, TRUE);
                        }
                    }
                }
            }
        }
        if(err != noErr){
            /* normally the printSettings and printSession would be released by our dialog 
		done proc but if we got an error and therefore got to this point in our code, 
		the dialog done proc was NOT called and therefore we need to release the
		printSession here
            */
	    if(printSettings){
                // this releases any print settings already stored on our private data
                (void)SetPrintSettingsOnPrivateData(documentDataP, NULL);
            }
            (void)PMRelease(printSession);   // ignoring error since we already have one 
        }
    }
    
    DoErrorAlert(err, kMyPrintErrorFormatStrKey);
    return err;
}

// ---------------------------------------------------------------------------------------------
static pascal void MyPrintDialogDoneProc(PMPrintSession printSession,
                            WindowRef documentWindow, Boolean accepted)
{
    OSStatus err = noErr, tempErr;
    void *ourDataP = GetOurWindowProperty(documentWindow);

    if (ourDataP)
    {
        PMPrintSettings printSettings = GetPrintSettingsFromPrivateData (ourDataP);

		// only run the print loop if the user accepted the print dialog

        if (accepted)
        {
            //	Additional code that demonstrates how to gain access to the extended 
            //	data stored in the print settings ticket by your application-hosted PDE.
 
            MySettings extendedData;
            
            err = MyGetPDEData (printSettings, &extendedData);
            
            if (err == noErr)
            {
                    char message[80];
                    sprintf (message, "AppUsingSheets: custom feature %s", 
                            extendedData.checkbox ? "selected" : "not selected");
                    MyDebugMessage (message, extendedData.checkbox);
            }

            err = MyDoPrintLoop(
				printSession, 
				GetPageFormatFromPrivateData (ourDataP),
				printSettings, 
				ourDataP, 
				&gMyPrintingProcsWithStatusDialog
			);

		}

        // We're done with the print settings on our private data so
        // set them to NULL which causes the print settings to be released
        tempErr = SetPrintSettingsOnPrivateData(ourDataP, NULL);
        if(err == noErr)
            err = tempErr;
    } // else Assert(...);
    
    // now we release the session we created to do the Print dialog
    tempErr = PMRelease(printSession);
    if(err == noErr)
        err = tempErr;
    
    DoErrorAlert(err, kMyPrintErrorFormatStrKey);
}

// --------------------------------------------------------------------------------------
static OSStatus MyDoPrintLoop(PMPrintSession printSession, PMPageFormat pageFormat, 
				PMPrintSettings printSettings, const void *ourDataP,
                                const PrintingProcs *printingProcsP)
{
    OSStatus err = noErr;
    OSStatus tempErr = noErr;
    UInt32 firstPage, lastPage, totalDocPages = MyGetDocumentNumPagesInDoc(ourDataP);
    
    if(!err)
	err = PMGetFirstPage(printSettings, &firstPage);
	
    if (!err)
        err = PMGetLastPage(printSettings, &lastPage);

    if(!err && lastPage > totalDocPages){
        // don't draw more than the number of pages in our document
        lastPage = totalDocPages;
    }

    if (!err)		// tell the printing system the number of pages we are going to print
        err = PMSetLastPage(printSettings, lastPage, FALSE);

    if (!err)
    {
        PageDrawProc *drawProc = GetMyDrawPageProc(ourDataP);
        err = printingProcsP->BeginDocumentProc(printSession, printSettings, pageFormat);
        if (!err){
	    UInt32 pageNumber = firstPage;
	    // need to check errors from our print loop and errors from the session for each
	    // time around our print loop before calling our BeginPageProc
            while(pageNumber <= lastPage && err == noErr && PMSessionError(printSession) == noErr)
            {
                err = printingProcsP->BeginPageProc(printSession, pageFormat, NULL);
                if (!err){
                    GrafPtr oldPort = NULL;
                    void *printingContext = NULL;
                    GetPort(&oldPort);	// preserve the existing port
                
                    err = PMSessionGetGraphicsContext(printSession, kPMGraphicsContextQuickdraw,
                                                    (void **)&printingContext);
                    if(!err){
                        Rect pageRect;
                        SetPort((CGrafPtr)printingContext);
                        GetPortBounds(printingContext, &pageRect);
                        err = drawProc(ourDataP, &pageRect, pageNumber); // image the correct page
                        SetPort(oldPort);	// restore the prior port
                    }
                    // we must call EndPage if BeginPage returned noErr
		    tempErr = printingProcsP->EndPageProc(printSession);
                        
		    if(!err)err = tempErr;
                }
		pageNumber++;
            }	// end while loop
            
            // we must call EndDocument if BeginDocument returned noErr
	    tempErr = printingProcsP->EndDocumentProc(printSession);

	    if(!err)err = tempErr;
	    if(!err)
		err = PMSessionError(printSession);
        }
    }
    return err;
}

OSStatus MakePDFDocument(WindowRef parentWindow, void *documentDataP, CFURLRef saveURL)
{
#pragma unused (parentWindow)
    OSStatus err = noErr, tempErr;
    PMPrintSettings printSettings = NULL;
    PMPrintSession printSession;
    err = PMCreateSession(&printSession);
    if(err == noErr){
	// validate the page format we're going to use
	err = PMSessionValidatePageFormat(printSession, 
			    GetPageFormatFromPrivateData(documentDataP),
			    kPMDontWantBoolean);
        if (err == noErr)
        {
            err = PMCreatePrintSettings(&printSettings);
            if(err == noErr)
            {
                err = PMSessionDefaultPrintSettings(printSession, printSettings);
                if(err == noErr){
                    CFStringRef myDocumentNameRef;
                    err = CopyDocumentName(documentDataP, &myDocumentNameRef);
                    if(err == noErr)
                    {
                        // set the job name
                        err = PMSetJobNameCFString (printSettings, myDocumentNameRef);
                        CFRelease(myDocumentNameRef);	// release our reference to the document name
                    }
                }

                if (err == noErr)
                    err = PMSetPageRange(printSettings, 1, MyGetDocumentNumPagesInDoc(documentDataP));
                
                // set our destination to be our target URL and the format to be PDF
                if(err == noErr){
                    // this API exists ONLY in MacOS X 10.1 and later only
                    if(!err)
                        err = Call_PMSessionSetDestination(printSession, printSettings,
                                    kPMDestinationFile, kPMDocumentFormatPDF, 
                                    saveURL);
                }
#if !NO_SAVE_STATUS_DIALOG
                /*
                    It looks a bit odd to call PMSessionUseSheets when we aren't even
                    using the print dialog BUT the reason we are doing so is so that
                    the printing status narration will use sheets and not a modal
                    dialog. NOTE: this is only called when this code is built with
                    !NO_SAVE_STATUS_DIALOG is true, i.e. we want status narration
                    of the save to PDF process.
                */
                if(err == noErr){
                    err = PMSessionUseSheets(printSession, parentWindow, NULL);             
                }
#endif
            }

            if (err == noErr)
            {
                err = MyDoPrintLoop(printSession, 
                            GetPageFormatFromPrivateData(documentDataP),
                            printSettings, 
                            documentDataP, 
#if NO_SAVE_STATUS_DIALOG
			    &gMyPrintingProcsNoStatusDialog
#else
                            &gMyPrintingProcsWithStatusDialog
#endif
                        );
            }
        }
        if(printSettings){
            tempErr = PMRelease(printSettings);
            if(err == noErr)
                err = tempErr;
        }

        tempErr = PMRelease(printSession);    
        if(err == noErr)
            err = tempErr;
    }
    DoErrorAlert(err, kMySaveAsPDFErrorFormatStrKey);
    return err;
}

#if BUILD_FOR_CFM	// only if we are building for CFM do we need these routines
static OSStatus LoadFrameworkBundle(CFStringRef framework, CFBundleRef *bundlePtr)
{
	OSStatus 	err;
	FSRef 		frameworksFolderRef;
	CFURLRef	baseURL;
	CFURLRef	bundleURL;
	
	//MoreAssertQ(bundlePtr != nil);
	
	*bundlePtr = nil;
	
	baseURL = nil;
	bundleURL = nil;
	
	err = FSFindFolder(kOnAppropriateDisk, kFrameworksFolderType, TRUE, &frameworksFolderRef);
	if (err == noErr) {
		baseURL = CFURLCreateFromFSRef(kCFAllocatorSystemDefault, &frameworksFolderRef);
		if (baseURL == nil) {
			err = coreFoundationUnknownErr;
		}
	}
	if (err == noErr) {
		bundleURL = CFURLCreateCopyAppendingPathComponent(kCFAllocatorSystemDefault, 
                                                                    baseURL, framework, FALSE);
		if (bundleURL == nil) {
			err = coreFoundationUnknownErr;
		}
	}
	if (err == noErr) {
		*bundlePtr = CFBundleCreate(kCFAllocatorSystemDefault, bundleURL);
		if (*bundlePtr == nil) {
			err = coreFoundationUnknownErr;
		}
	}
	if (err == noErr) {
	    if ( ! CFBundleLoadExecutable( *bundlePtr ) ) {
			err = coreFoundationUnknownErr;
	    }
	}

	// Clean up.
	
	if (err != noErr && *bundlePtr != nil) {
		CFRelease(*bundlePtr);
		*bundlePtr = nil;
	}
	if (bundleURL != nil) {
		CFRelease(bundleURL);
	}	
	if (baseURL != nil) {
		CFRelease(baseURL);
	}	
	
	return err;
}

static CFBundleRef getCorePrintingBundle()
{
    static CFBundleRef printingBundle = NULL;
    // first time this is called we need to load the framework bundle
    if(!printingBundle){
	OSStatus err = LoadFrameworkBundle(CFSTR("ApplicationServices.framework"), &printingBundle);
        if(err)
            printingBundle = NULL;
    }
    return printingBundle;
}

static pascal OSStatus Call_PMSessionBeginDocumentNoDialog(PMPrintSession printSession,
                                 PMPrintSettings        printSettings,
                                 PMPageFormat           pageFormat)
{
    OSStatus err = kCantGetPrintingProcPtrErr;	// returned if we can't call our proc
    static PMSessionBeginDocumentProcPtr ourProcPtr = NULL;
    // first time this is called we need to get our procPtr
    if(!ourProcPtr){
        CFBundleRef printingBundle = getCorePrintingBundle();
        if(printingBundle){
            ourProcPtr = (PMSessionBeginDocumentProcPtr)CFBundleGetFunctionPointerForName(printingBundle, 
							CFSTR("PMSessionBeginDocumentNoDialog"));
        }
    }
    
    if(ourProcPtr){
        err = ourProcPtr(printSession, printSettings, pageFormat);
    }
    return err;
}

static pascal OSStatus Call_PMSessionEndDocumentNoDialog(PMPrintSession printSession)
{
    OSStatus err = kCantGetPrintingProcPtrErr;	// returned if we can't call our proc
    static PMSessionEndDocumentProcPtr ourProcPtr = NULL;
    // first time this is called we need to get our procPtr
    if(!ourProcPtr){
        CFBundleRef printingBundle = getCorePrintingBundle();
        if(printingBundle){
            ourProcPtr = (PMSessionEndDocumentProcPtr)CFBundleGetFunctionPointerForName(printingBundle, 
							CFSTR("PMSessionEndDocumentNoDialog"));
        }
    }
    
    if(ourProcPtr){
        err = ourProcPtr(printSession);
    }
    return err;
}

static pascal OSStatus Call_PMSessionBeginPageNoDialog(PMPrintSession printSession,
                                 PMPageFormat pageFormat,
                                 const PMRect *pageFrame)
{
    OSStatus err = kCantGetPrintingProcPtrErr;	// returned if we can't call our proc
    static PMSessionBeginPageProcPtr ourProcPtr = NULL;
    // first time this is called we need to get our procPtr
    if(!ourProcPtr){
        CFBundleRef printingBundle = getCorePrintingBundle();
        if(printingBundle){
            ourProcPtr = (PMSessionBeginPageProcPtr)CFBundleGetFunctionPointerForName(printingBundle, 
							CFSTR("PMSessionBeginPageNoDialog"));
        }
    }
    
    if(ourProcPtr){
        err = ourProcPtr(printSession, pageFormat, pageFrame);
    }
    return err;
}

static pascal OSStatus Call_PMSessionEndPageNoDialog(PMPrintSession printSession)
{
    OSStatus err = kCantGetPrintingProcPtrErr;	// returned if we can't call our proc
    static PMSessionEndPageProcPtr ourProcPtr = NULL;
    // first time this is called we need to get our procPtr
    if(!ourProcPtr){
        CFBundleRef printingBundle = getCorePrintingBundle();
        if(printingBundle){
            ourProcPtr = (PMSessionEndPageProcPtr)CFBundleGetFunctionPointerForName(printingBundle, 
							CFSTR("PMSessionEndPageNoDialog"));
        }
    }
    
    if(ourProcPtr){
        err = ourProcPtr(printSession);
    }
    return err;
}

static pascal OSStatus Call_PMSessionSetDestination(PMPrintSession printSession,
					PMPrintSettings     printSettings,
					PMDestinationType   destType,
					CFStringRef         destFormat,
					CFURLRef            destLocation)
{
    OSStatus err = kCantGetPrintingProcPtrErr;	// returned if we can't call our proc
    static PMSessionSetDestinationProcPtr ourProcPtr = NULL;
    // first time this is called we need to get our procPtr
    if(!ourProcPtr){
        CFBundleRef printingBundle = getCorePrintingBundle();
        if(printingBundle){
            ourProcPtr = (PMSessionSetDestinationProcPtr)CFBundleGetFunctionPointerForName(printingBundle, 
							CFSTR("PMSessionSetDestination"));
        }
    }
    
    if(ourProcPtr){
        err = ourProcPtr(printSession, printSettings, destType, destFormat, destLocation);
    }
    return err;
}
#endif
