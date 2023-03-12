/*
	File:		MyCarbonPrinting.c
	
	Contains:	Routines needed to perform printing. This example uses sheets.

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
#include "CGDrawPicture.h"
#include "UIHandling.h"

static PMSheetDoneUPP gMyPageSetupDoneProc;
static PMSheetDoneUPP gMyPrintDialogDoneProc;

static pascal void MyPageSetupDoneProc(PMPrintSession printSession, WindowRef documentWindow, Boolean accepted);
static pascal void MyPrintDialogDoneProc(PMPrintSession printSession, WindowRef documentWindow, Boolean accepted);

static OSStatus MySetupPageFormatForPrinting(PMPrintSession printSession, 
						void *docDataP, PMPageFormat *pageFormatP);

static OSStatus MyDoPrintLoop(PMPrintSession printSession, PMPageFormat pageFormat, 
				    PMPrintSettings printSettings, const void *docDataP);
                            

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
    OSStatus status = noErr;
    PMPageFormat pageFormat = GetPageFormatFromPrivateData(docDataP);
    if (!pageFormat)
    {
        status = PMCreatePageFormat(&pageFormat);
        if(status == noErr)
        {
            status = PMSessionDefaultPageFormat(printSession, pageFormat);
	    if(status == noErr){
		PMResolution appDrawingResolution;
		
		// for illustrative purposes, this sample will draw at 300,300 dpi resolution
		appDrawingResolution.hRes = appDrawingResolution.vRes = 300.;
		status = PMSetResolution(pageFormat, &appDrawingResolution);
	    }
            if (status == noErr)
                SetPageFormatOnPrivateData(docDataP, pageFormat);
            else{
                (void)PMRelease(pageFormat);
                pageFormat = NULL;
            }
        }
    }else{
	// we already have a page format so we'll validate it
        status = PMSessionValidatePageFormat(printSession, pageFormat,
                                        kPMDontWantBoolean);
        if(status){	// if validate failed!
            SetPageFormatOnPrivateData(docDataP, NULL);
            (void)PMRelease(pageFormat);
            pageFormat = NULL;
        }
    }
    
    DoErrorAlert(status, kMyPrintErrorFormatStrKey);
    
    *pageFormatP = pageFormat;
    return status;
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
                Cursor arrow;
                SetCursor(GetQDGlobalsArrow(&arrow));
                err = PMSessionUseSheets(printSession, window, gMyPageSetupDoneProc);             
                if(!err)
                    err = PMSessionPageSetupDialog(printSession, pageFormat, &accepted);
		// when using sheets, the value of accepted returned here is irrelevant
		// since it is our sheet done proc that is called when the dialog is dismissed
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
    // this sample doesn't do anything with the page format after page setup is done
    
    // now we release the session we created to do the page setup dialog
    OSStatus err = PMRelease(printSession);	
    if(err)
        DoErrorAlert(err, kMyPrintErrorFormatStrKey);
    return;
}


// --------------------------------------------------------------------------------------------------------------
OSStatus DoPrint(WindowRef parentWindow, void *ourDataP)
{
    OSStatus status = noErr;
    PMPrintSettings printSettings = NULL;
    PMPageFormat pageFormat = NULL;

    UInt32 minPage = 1, maxPage;
    PMPrintSession printSession;
    status = PMCreateSession(&printSession);
    if(status == noErr){
	status = MySetupPageFormatForPrinting(printSession, ourDataP, &pageFormat);
        if (status == noErr)
        {
            status = PMCreatePrintSettings(&printSettings);
            if(status == noErr)
            {
                status = PMSessionDefaultPrintSettings(printSession, printSettings);
                if(status == noErr){
                    CFStringRef windowTitleRef;
                    status = CopyWindowTitleAsCFString(parentWindow, &windowTitleRef);
                    if(status == noErr)
                    {
                        // set the job name before displaying the print dialog
                        status = PMSetJobNameCFString (printSettings, windowTitleRef);
                        CFRelease (windowTitleRef);
                    }
                }
            }
            if (status == noErr)
            {
                maxPage = MyGetDocumentNumPagesInDoc(ourDataP);
                status = PMSetPageRange(printSettings, minPage, maxPage);
            }
            if (status == noErr)
            {
                Boolean accepted;
                SetPrintSettingsOnPrivateData(ourDataP, printSettings);
                status = PMSessionUseSheets(printSession, parentWindow, gMyPrintDialogDoneProc);
                if(status == noErr)
                    status = PMSessionPrintDialog(printSession, printSettings, 
                                    pageFormat,
                                    &accepted);
		// when using sheets, the value of 'accepted' returned here is irrelevant
		// since it is our sheet done proc that is called when the dialog is dismissed
            }
        }
        if(status != noErr){
            // if we got an error our dialog done proc will not be called so we need to clean up
            if(printSettings){
                SetPrintSettingsOnPrivateData(ourDataP, NULL);
                (void)PMRelease(printSettings);
            }
            (void)PMRelease(printSession);    
        }
    }
    DoErrorAlert(status, kMyPrintErrorFormatStrKey);
    return status;
}

// --------------------------------------------------------------------------------------------------------------
static pascal void MyPrintDialogDoneProc(PMPrintSession printSession,
                            WindowRef documentWindow, Boolean accepted)
{
    OSStatus status = noErr, tempErr;
    void *ourDataP = GetOurWindowProperty(documentWindow);
    if(ourDataP)
    {
        PMPrintSettings printSettings = GetPrintSettingsFromPrivateData(ourDataP);
	// only run the print loop if the user accepted the print dialog.
        if(accepted)
            status = MyDoPrintLoop(printSession, 
                        GetPageFormatFromPrivateData(ourDataP),
                        printSettings, ourDataP);

        SetPrintSettingsOnPrivateData(ourDataP, NULL);
        tempErr = PMRelease(printSettings);
        if(status == noErr)
            status = tempErr;
    }
    // now we release the session we created to do the Print dialog
    tempErr = PMRelease(printSession);
    if(status == noErr)
        status = tempErr;
    
    DoErrorAlert(status, kMyPrintErrorFormatStrKey);
}

// --------------------------------------------------------------------------------------------------------------
static OSStatus MyDoPrintLoop(PMPrintSession printSession, PMPageFormat pageFormat, 
				PMPrintSettings printSettings, const void *ourDataP)
{
    OSStatus err = noErr;
    OSStatus tempErr = noErr;
    UInt32 firstPage, lastPage, totalDocPages = MyGetDocumentNumPagesInDoc(ourDataP);
    PMResolution res;
    float ourAppScaling = 1.;
    
    err = PMGetResolution(pageFormat, &res);
    if(!err){
	ourAppScaling = res.hRes/72.;	// the scale factor we are applying
    }

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
        err = PMSessionBeginDocument(printSession, printSettings, pageFormat);
        if (!err){
	    UInt32 pageNumber;
            for(pageNumber = firstPage; 
                err == noErr && (err = PMSessionError(printSession)) == noErr && pageNumber <= lastPage; 
                pageNumber++
            ){
                err = PMSessionBeginPage(printSession, pageFormat, NULL);
                if (!err){
                    GrafPtr oldPort = NULL;
                    void *printingContext = NULL;
                    GetPort(&oldPort);	// preserve the existing port
                
                    err = PMSessionGetGraphicsContext(printSession, kPMGraphicsContextQuickdraw,
                                                    (void **)&printingContext);
                    if(!err){
                        SetPort((CGrafPtr)printingContext);
                        err = drawProc(ourDataP, ourAppScaling);	// supply app drawing resolution
                        SetPort(oldPort);	// restore the prior port
                    }
		    tempErr = PMSessionEndPage(printSession);
		    if(!err)err = tempErr;
                }
            }	// end for loop
	    tempErr = PMSessionEndDocument(printSession);
	    if(!err)err = tempErr;
        }
    }
    return err;
}
