/*
	File:		print.c

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

                Copyright © 2002 Apple Computer, Inc., All Rights Reserved
*/

#include "atsui.h"
#include "print.h"

// Globals (for this source file only)
//
static  Handle                  gflatPageFormat = NULL;         // used in FlattenAndSavePageFormat
static  PMPageFormat            gPageFormat = kPMNoPageFormat;
static  PMPrintSettings         gPrintSettings = kPMNoPrintSettings;
static  PMPrintSession          gPrintSession;

/*------------------------------------------------------------------------------

    Function:   InitializePrinting
    
    Parameters:
        none
    
    Description:
        Creates a print session to be used throughout the application.  Also
        creates a default page format, which allows the user to choose "Print"
        from the menu without having done "Page Setup" first.
    
------------------------------------------------------------------------------*/
OSStatus InitializePrinting(void)
{
    OSStatus    status;

    // Initialize the print session
    //
    status = PMCreateSession(&gPrintSession);

    // Create the page format
    //
    status = PMCreatePageFormat(&gPageFormat);
    
    //  Note that PMPageFormat is not session-specific, but calling
    //  PMSessionDefaultPageFormat assigns values specific to the printer
    //  associated with the current printing session.
    //
    if ( (status == noErr) && (gPageFormat != kPMNoPageFormat) ) {
        status = PMSessionDefaultPageFormat(gPrintSession, gPageFormat);
    }

    return status;
}


/*------------------------------------------------------------------------------

    Function:   DoPageSetupDialog
    
    Parameters:
        printSession    -   current printing session
        pageFormat      -   a PageFormat object addr
    
    Description:
        If the caller passes in an empty PageFormat object, DoPageSetupDialog
        creates a new one, otherwise it validates the one provided by the caller.
        It then invokes the Page Setup dialog and checks for Cancel. Finally it
        flattens the PageFormat object so it can be saved with the document.
        Note that the PageFormat object is modified by this function.
    
------------------------------------------------------------------------------*/
OSStatus DoPageSetupDialog(void)
{
    OSStatus    status;
    Boolean     accepted;
    
    //  Set up a valid PageFormat object.
    //
    if ( gPageFormat == kPMNoPageFormat ) {
        status = PMCreatePageFormat(&gPageFormat);
    
        //  Note that PMPageFormat is not session-specific, but calling
        //  PMSessionDefaultPageFormat assigns values specific to the printer
        //  associated with the current printing session.
        if ( (status == noErr) && (gPageFormat != kPMNoPageFormat) ) {
            status = PMSessionDefaultPageFormat(gPrintSession, gPageFormat);
        }
    }
    else {
        status = PMSessionValidatePageFormat(gPrintSession, gPageFormat, kPMDontWantBoolean);
    }

    //  Display the Page Setup dialog.
    //
    if ( status == noErr ) {
        status = PMSessionPageSetupDialog(gPrintSession, gPageFormat, &accepted);
        if ( ! accepted ) {
            status = kPMCancel;     // user clicked Cancel button
        }
    }
                
    //  If the user did not cancel, flatten and save the PageFormat object
    //  with our document.
    //
    if  (status == noErr ) {
        status = FlattenAndSavePageFormat(gPageFormat);
    }

    return status;
    
}   //  DoPageSetupDialog



/*------------------------------------------------------------------------------
    Function:   DoPrintDialog
        
    Parameters:
        printSession    -   current printing session
        pageFormat      -   a PageFormat object addr
        printSettings   -   a PrintSettings object addr
            
    Description:
        If the caller passes an empty PrintSettings object, DoPrintDialog creates
        a new one, otherwise it validates the one provided by the caller.
        It then invokes the Print dialog and checks for Cancel.
        Note that the PrintSettings object is modified by this function.
        
------------------------------------------------------------------------------*/
OSStatus DoPrintDialog(void)
{
    OSStatus    status;
    Boolean     accepted;
    UInt32      realNumberOfPagesinDoc;
    
    //  In this sample code the caller provides a valid PageFormat reference but in
    //  your application you may want to load and unflatten the PageFormat object
    //  that was saved at PageSetup time.  See LoadAndUnflattenPageFormat below.
    
    //  Set up a valid PrintSettings object.
    if (gPrintSettings == kPMNoPrintSettings) {
        status = PMCreatePrintSettings(&gPrintSettings);    

        //  Note that PMPrintSettings is not session-specific, but calling
        //  PMSessionDefaultPrintSettings assigns values specific to the printer
        //  associated with the current printing session.
        if ((status == noErr) && (gPrintSettings != kPMNoPrintSettings)) {
            status = PMSessionDefaultPrintSettings(gPrintSession, gPrintSettings);
        }
    }
    else {
        status = PMSessionValidatePrintSettings(gPrintSession, gPrintSettings, kPMDontWantBoolean);
    }

    //  Before displaying the Print dialog, we calculate the number of pages in the
    //  document.  On Mac OS X this is useful because we can prime the Print dialog
    //  with the actual page range of the document and prevent the user from entering
    //  out-of-range numbers.  This is not possible on Mac OS 8 and 9 because the driver,
    //  not the printing manager, controls the page range fields in the Print dialog.

    //  Calculate the number of pages required to print the entire document.
    if (status == noErr) {
        status = DetermineNumberOfPagesInDoc(gPageFormat, &realNumberOfPagesinDoc);
    }

    //  Set a valid page range before displaying the Print dialog
    if (status == noErr) {
        status = PMSetPageRange(gPrintSettings, 1, realNumberOfPagesinDoc);
    }

    //  Display the Print dialog.
    if (status == noErr) {
        status = PMSessionPrintDialog(gPrintSession, gPrintSettings, gPageFormat, &accepted);
        if (!accepted) {
            status = kPMCancel;     // user clicked Cancel button
        }
    }
        
    return status;
    
}   //  DoPrintDialog


/*------------------------------------------------------------------------------
    Function:
        DoPrintLoop
    
    Parameters:
        printSession    -   current printing session
        pageFormat      -   a PageFormat object addr
        printSettings   -   a PrintSettings object addr
    
    Description:
        DoPrintLoop calculates which pages to print and executes the print
        loop, calling DrawPage for each page.
                
------------------------------------------------------------------------------*/
void DoPrintLoop(void)
{
    OSStatus    status,
                printError;
    GrafPtr     currPort, printingPort;
    UInt32      realNumberOfPagesinDoc,
                pageNumber,
                firstPage,
                lastPage;
    CFStringRef jobName = CFSTR("ATSUITestApp");

    //  Since this sample code doesn't have a window, give the spool file a name.
    status = PMSetJobNameCFString(gPrintSettings, jobName);

    //  Get the user's Print dialog selection for first and last pages to print.
    if (status == noErr) {
        status = PMGetFirstPage(gPrintSettings, &firstPage);
        if (status == noErr) {
            status = PMGetLastPage(gPrintSettings, &lastPage);
        }
    }

    //  Check that the selected page range does not exceed the actual number of
    //  pages in the document.
    if (status == noErr) {
        status = DetermineNumberOfPagesInDoc(gPageFormat, &realNumberOfPagesinDoc);
        if (realNumberOfPagesinDoc < lastPage) {
            lastPage = realNumberOfPagesinDoc;
        }
    }

    //  Before executing the print loop, tell the Carbon Printing Manager which pages
    //  will be spooled so that the progress dialog can reflect an accurate page count.
    //  This is recommended on Mac OS X.  On Mac OS 8 and 9, we have no control over
    //  what the printer driver displays.
    
    if (status == noErr) {
        status = PMSetFirstPage(gPrintSettings, firstPage, false);
    }
    if (status == noErr) {
        status = PMSetLastPage(gPrintSettings, lastPage, false);
    }

    //  Note, we don't have to worry about the number of copies.  The printing
    //  manager handles this.  So we just iterate through the document from the
    //  first page to be printed, to the last.
    if (status == noErr) {
        //  Establish a graphics context for drawing the document's pages.
        status = PMSessionBeginDocument(gPrintSession, gPrintSettings, gPageFormat);
        if (status == noErr) {
            //  Print the selected range of pages in the document.      
            pageNumber = firstPage;
            while ((pageNumber <= lastPage) && (PMSessionError(gPrintSession) == noErr)) {
                //  Note, we don't have to deal with the classic Printing Manager's
                //  128-page boundary limit.
            
                //  Set up a page for printing.  Under the classic Printing Manager, applications
                //  could provide a page rect different from the one in the print record to achieve
                //  scaling. This is no longer recommended and on Mac OS X, the PageRect argument
                //  is ignored.
                status = PMSessionBeginPage(gPrintSession, gPageFormat, NULL);
                if (status != noErr) {
                    break;
                }
                
                //  Save the current QD grafport.
                GetPort(&currPort);
                
                //  Get the current graphics context, in this case a Quickdraw grafPort,
                //  for drawing the page.
                status = PMSessionGetGraphicsContext(gPrintSession, kPMGraphicsContextQuickdraw, (void**) &printingPort);
                if (status == noErr) {
                    //  Set the printing port before drawing the page.
                    SetPort((GrafPtr) printingPort);
                
                    //  Draw the page.
                    //DrawPage(gPrintSession, pageNumber, addPostScript);
                    DrawATSUIStuff((GrafPtr) printingPort);
                    
                    //  Restore the QD grafport.
                    SetPort(currPort);
                }
            
                //  Close the page.
                status = PMSessionEndPage(gPrintSession);
                if (status != noErr) {
                    break;
                }
                
                //  And loop.
                pageNumber++;
            }
        
            // Close the printing port.  This dismisses the progress dialog on Mac OS X.
            (void)PMSessionEndDocument(gPrintSession);
        }
    }
        
    //  Only report a printing error once we have completed the print loop. This
    //  ensures that every PMBeginXXX call is followed by a matching PMEndXXX
    //  call, so the Printing Manager can release all temporary memory and close
    //  properly.
    printError = PMSessionError(gPrintSession);
    if (printError != noErr && printError != kPMCancel) {
        PostPrintingErrors(printError);
    }
        
}   //  DoPrintLoop



/*------------------------------------------------------------------------------
    Function:
        FlattenAndSavePageFormat
    
    Parameters:
        pageFormat  -   a PageFormat object
    
    Description:
        Flattens a PageFormat object so it can be saved with the document.
        Assumes caller passes a validated PageFormat object.
        
------------------------------------------------------------------------------*/
OSStatus FlattenAndSavePageFormat(PMPageFormat pageFormat)
{
    OSStatus    status;
    Handle  flatFormatHandle = NULL;
    
    //  Flatten the PageFormat object to memory.
    status = PMFlattenPageFormat(pageFormat, &flatFormatHandle);
    
    //  Write the PageFormat data to file.
    //  In this sample code we simply copy it to a global.  
    gflatPageFormat = flatFormatHandle;

    return status;
}   //  FlattenAndSavePageFormat



/*------------------------------------------------------------------------------
    Function:   LoadAndUnflattenPageFormat
    
    Parameters:
        pageFormat  - PageFormat object read from document file
    
    Description:
        Gets flattened PageFormat data from the document and returns a PageFormat
        object.
        The function is not called in this sample code but your application
        will need to retrieve PageFormat data saved with documents.
        
------------------------------------------------------------------------------*/
OSStatus    LoadAndUnflattenPageFormat(PMPageFormat* pageFormat)
{
    OSStatus    status;
    Handle  flatFormatHandle = NULL;

    //  Read the PageFormat flattened data from file.
    //  In this sample code we simply copy it from a global.
    flatFormatHandle = gflatPageFormat;

    //  Convert the PageFormat flattened data into a PageFormat object.
    status = PMUnflattenPageFormat(flatFormatHandle, pageFormat);
    
    return status;
}   //  LoadAndUnflattenPageFormat



/*------------------------------------------------------------------------------
    Function:   DetermineNumberOfPagesInDoc
    
    Parameters:
        pageFormat  - a PageFormat object addr
        numPages    - on return, the size of the document in pages
            
    Description:
        Calculates the number of pages needed to print the entire document.
        
------------------------------------------------------------------------------*/
OSStatus    DetermineNumberOfPagesInDoc(PMPageFormat pageFormat, UInt32* numPages)
{
    OSStatus    status;
    PMRect      pageRect;

    //  PMGetAdjustedPageRect returns the page size taking into account rotation,
    //  resolution and scaling settings.
    status = PMGetAdjustedPageRect(pageFormat, &pageRect);

    //  In this sample code we simply return a hard coded number.  In your application,
    //  you will need to figure out how many page rects are needed to image the
    //  current document.
    *numPages = 1;

    return status;
    
}   //  DetermineNumberOfPagesinDoc



/*------------------------------------------------------------------------------
    Function:   PostPrintingErrors
    
    Parameters:
        status  -   error code
    
    Description:
        This is where we could post an alert to report any problem reported
        by the Printing Manager.
        
------------------------------------------------------------------------------*/
void    PostPrintingErrors(OSStatus status)
{
#pragma unused (status) 
}   //  PostPrintingErrors