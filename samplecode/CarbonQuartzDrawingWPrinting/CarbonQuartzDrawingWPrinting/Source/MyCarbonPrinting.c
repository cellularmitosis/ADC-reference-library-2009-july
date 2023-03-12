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

// this setting determines whether there will be a printing status dialog when
// saving to PDF
#define NO_SAVE_STATUS_DIALOG	1

// the following typedefs are defined so that our PrintingProcs structure
// definition can be defined and used.
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


static PMSheetDoneUPP gMyPageSetupDoneProc;
static PMSheetDoneUPP gMyPrintDialogDoneProc;

static pascal void MyPageSetupDoneProc(PMPrintSession printSession, WindowRef documentWindow, Boolean accepted);
static pascal void MyPrintDialogDoneProc(PMPrintSession printSession, WindowRef documentWindow, Boolean accepted);

static OSStatus MySetupPageFormatForPrinting(PMPrintSession printSession, 
						void *docDataP, PMPageFormat *pageFormatP);

static OSStatus MyDoPrintLoop(PMPrintSession printSession, PMPageFormat pageFormat, 
				    PMPrintSettings printSettings, const void *docDataP,
                                    const PrintingProcs *printingProcsP);
                      
/**** Global Data ****/
static PrintingProcs gMyPrintingProcsNoStatusDialog = {PMSessionBeginDocumentNoDialog, 
						PMSessionBeginPageNoDialog, 
						PMSessionEndPageNoDialog,
						PMSessionEndDocumentNoDialog 
                                                };

static PrintingProcs gMyPrintingProcsWithStatusDialog = {PMSessionBeginDocument, 
						PMSessionBeginPage, 
						PMSessionEndPage,
                                                PMSessionEndDocument
                                                };


// --------------------------------------------------------------------------------------------------------------
OSStatus CreateSheetDoneProcs(void)
{
    OSStatus err = noErr;
    gMyPageSetupDoneProc = NewPMSheetDoneUPP(MyPageSetupDoneProc);
    if(!gMyPageSetupDoneProc)
        err = memFullErr;

    if(!err){
	gMyPrintDialogDoneProc = NewPMSheetDoneUPP(MyPrintDialogDoneProc);
	if(!gMyPrintDialogDoneProc)
	    err = memFullErr;
    }
        
    return err;
}

// --------------------------------------------------------------------------------------------------------------
static OSStatus MySetupPageFormatForPrinting(PMPrintSession printSession, void *docDataP, PMPageFormat *pageFormatP)
{
    OSStatus err = noErr;
    PMPageFormat pageFormat = GetPageFormatFromPrivateData(docDataP);
    if (pageFormat == NULL)
    {
        err = PMCreatePageFormat(&pageFormat);
        if(err == noErr)
        {
            err = PMSessionDefaultPageFormat(printSession, pageFormat);
            if (err == noErr)
                SetPageFormatOnPrivateData(docDataP, pageFormat);
            else{
                (void)PMRelease(pageFormat);
                pageFormat = NULL;
            }
        }
    }else{
	// we already have a page format so we'll validate it
        err = PMSessionValidatePageFormat(printSession, pageFormat,
                                        kPMDontWantBoolean);
        if(err){	// if validate failed!
	    SetPageFormatOnPrivateData(docDataP, NULL);
            (void)PMRelease(pageFormat);
            pageFormat = NULL;
        }
    }
    
    *pageFormatP = pageFormat;
    return err;
}

// --------------------------------------------------------------------------------------------------------------
OSStatus DoPageSetup(WindowRef window, void *docDataP)
{
    OSStatus		err = noErr;
    if(docDataP){
        PMPrintSession printSession;
        PMPageFormat pageFormat = NULL;
        err = PMCreateSession(&printSession);
        if(!err){
            Boolean accepted;
            err = MySetupPageFormatForPrinting(printSession, docDataP, &pageFormat);
            if(!err){
		Boolean sheetsAreAvailable = true;
                err = PMSessionUseSheets(printSession, window, gMyPageSetupDoneProc);             
		if(err == kPMNotImplemented){
		    // sheets are not available (aka, Mac OS 8/9)
		    err = noErr;
		    sheetsAreAvailable = false;
		}
                if(!err){
                    err = PMSessionPageSetupDialog(printSession, pageFormat, &accepted);
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
            if(err){	// only if there is an error do we release the session
                        // otherwise we'll do that in our sheet done proc
                (void)PMRelease(printSession);
            }
	}
    }
    DoErrorAlert(err, kMyPrintErrorFormatStrKey);
    return err;
} // DoPageSetup

// --------------------------------------------------------------------------------------------------------------
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


// --------------------------------------------------------------------------------------------------------------
OSStatus DoPrint(WindowRef parentWindow, void *documentDataP, Boolean printOne)
{
    OSStatus err = noErr;
    PMPrintSettings printSettings = NULL;
    PMPageFormat pageFormat = NULL;

    UInt32 minPage = 1, maxPage;
    PMPrintSession printSession;
    err = PMCreateSession(&printSession);
    if(err == noErr){
	err = MySetupPageFormatForPrinting(printSession, documentDataP, &pageFormat);
        if (err == noErr)
        {
            err = PMCreatePrintSettings(&printSettings);
            if(err == noErr)
            {
                err = PMSessionDefaultPrintSettings(printSession, printSettings);
                if(err == noErr){
                    CFStringRef windowTitleRef;
                    err = CopyWindowTitleAsCFString(parentWindow, &windowTitleRef);
                    if(err == noErr)
                    {
                        // set the job name before displaying the print dialog
                        err = PMSetJobNameCFString (printSettings, windowTitleRef);
                        CFRelease(windowTitleRef);
                    }
                }
            }
            if (err == noErr)
            {
                maxPage = MyGetDocumentNumPagesInDoc(documentDataP);
                err = PMSetPageRange(printSettings, minPage, maxPage);
            }

            if (err == noErr)
            {
                Boolean accepted;
		Boolean sheetsAreAvailable = true;
                SetPrintSettingsOnPrivateData(documentDataP, printSettings);
                err = PMSessionUseSheets(printSession, parentWindow, gMyPrintDialogDoneProc);
		if(err == kPMNotImplemented){
		    // we get here if sheets are not available, i.e. Mac OS 8/9
		    err = noErr;
		    sheetsAreAvailable = false;
		}		
                if(err == noErr && !printOne){
                    err = PMSessionPrintDialog(printSession, printSettings, 
                                    pageFormat,
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
                        MyPrintDialogDoneProc(printSession, parentWindow, true);
                }
            }
        }
        if(err != noErr){
            // if we got an error our dialog done proc will not be called so we need to clean up
            if(printSettings){
                SetPrintSettingsOnPrivateData(documentDataP, NULL);
                (void)PMRelease(printSettings);	// ignoring error since we already have one
            }
            (void)PMRelease(printSession);   // ignoring error since we already have one 
        }
    }
    DoErrorAlert(err, kMyPrintErrorFormatStrKey);
    return err;
}

// --------------------------------------------------------------------------------------------------------------
static pascal void MyPrintDialogDoneProc(PMPrintSession printSession,
                            WindowRef documentWindow, Boolean accepted)
{
    OSStatus err = noErr, tempErr;
    void *ourDataP = GetOurWindowProperty(documentWindow);
    if(ourDataP)
    {
        PMPrintSettings printSettings = GetPrintSettingsFromPrivateData(ourDataP);
	// only run the print loop if the user accepted the print dialog.
        if(accepted){
            err = MyDoPrintLoop(printSession, 
                        GetPageFormatFromPrivateData(ourDataP),
                        printSettings, ourDataP, &gMyPrintingProcsWithStatusDialog);
	}

        SetPrintSettingsOnPrivateData(ourDataP, NULL);
        tempErr = PMRelease(printSettings);
        if(err == noErr)
            err = tempErr;
    }// else Assert(...);
    
    // now we release the session we created to do the Print dialog
    tempErr = PMRelease(printSession);
    if(err == noErr)
        err = tempErr;
    
    DoErrorAlert(err, kMyPrintErrorFormatStrKey);
}

// --------------------------------------------------------------------------------------------------------------
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
        err = PMSetLastPage(printSettings, lastPage, false);

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
                        err = drawProc(ourDataP, &pageRect, pageNumber);	// image the correct page
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
    OSStatus err = noErr, tempErr;
    PMPrintSettings printSettings = NULL;
    PMPageFormat pageFormat = NULL;
    PMPrintSession printSession;
    err = PMCreateSession(&printSession);
    if(err == noErr){
	err = MySetupPageFormatForPrinting(printSession, documentDataP, &pageFormat);
        if (err == noErr)
        {
            err = PMCreatePrintSettings(&printSettings);
            if(err == noErr)
            {
                err = PMSessionDefaultPrintSettings(printSession, printSettings);
                if(err == noErr){
                    CFStringRef windowTitleRef;
                    err = CopyWindowTitleAsCFString(parentWindow, &windowTitleRef);
                    if(err == noErr)
                    {
                        // set the job name
                        err = PMSetJobNameCFString (printSettings, windowTitleRef);
                        CFRelease(windowTitleRef);
                    }
                }

                if (err == noErr)
                    err = PMSetPageRange(printSettings, 1, MyGetDocumentNumPagesInDoc(documentDataP));
                
                // set our destination to be our target URL and the format to be PDF
                if(err == noErr){
                    // this API exists ONLY in Mac OS X 10.1 and later only
                    err = PMSessionSetDestination(printSession, printSettings,
                                    kPMDestinationFile, kPMDocumentFormatPDF, 
                                    saveURL);
                }
#if !NO_SAVE_STATUS_DIALOG
                if(err == noErr){
                    err = PMSessionUseSheets(printSession, parentWindow, NULL);             
                }
#endif
            }

            if (err == noErr)
            {
                err = MyDoPrintLoop(printSession, 
                            pageFormat,
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

