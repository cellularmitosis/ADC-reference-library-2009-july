/*
File: AppDrawing.c

Abstract: Drawing code for our sample document

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

#include "AppDrawing.h"
#include "UIHandling.h"

#define	kNumDocumentPages	5

/********** our data types	***********/
	
typedef struct DrawData {
	CFDataRef		flatPageFormat;		// a flattened representation of the page format
    PMPageFormat	pageFormat;			// the page format for this document
    PMPrintSettings printSettings;		// the print settings for a given print job
} DrawData;

/*** the routines ***/

/*	This routine normally return a reference (owned by the caller)
    for the name of the document which may or may not be the window
    title of the window being printed. In our sample code we'll just
    get a constant from our localized strings.
*/
OSStatus CopyDocumentName(DrawDataRef ourData, CFStringRef *stringRefP)
{
#pragma unused (ourData)
    OSStatus err = noErr;
    *stringRefP = CFCopyLocalizedString(kMyDocumentTitleKey, NULL);
    if(!*stringRefP)
        err = kCantGetDocumentLocalizedStringErr;
        
    return err;
}

UInt32 GetMyDocumentNumPagesInDoc(const DrawDataRef ourData)
{
#pragma unused (ourData)
	// more realistic code would use the document's page format
	// to determine the number of pages in our document
	// we always have kNumDocumentPages page per document
    return kNumDocumentPages;
}

OSStatus SetPageFormatOnPrivateData(DrawDataRef ourData, PMPageFormat pageFormat)
/*	SetPageFormatOnPrivateData takes the passed in PMPageFormat object and stores it on our
    private data. In doing so it does a PMRetain so that the caller of this
    routine can safely release its reference to the object. In addition this
    routine releases any previous page format that was stored with our private
    data.
    
    Couple of specific issues:
    a) we check that the passed in pageFormat is not NULL before we try
        to retain it.
    b) we check that the format that is stored on our private data is not
        NULL before we try to release it.
    c) we first retain the object passed in before we release the data already
        stored on our private data so that if they are the SAME PMPageFormat
        object we don't release it before retaining it since that release
        could cause the object to be deallocated.
*/
{
    OSStatus err = noErr;
    if(ourData) {
        if(pageFormat) {
            err = PMRetain(pageFormat);		// we'll retain the format we put on our private data
        }
        if(!err) {
            if( ourData->pageFormat )
                err = PMRelease(ourData->pageFormat);
                
            ourData->pageFormat = pageFormat;
        }
    }
        
    return err;
}

OSStatus SetPrintSettingsOnPrivateData(DrawDataRef ourData, PMPrintSettings printSettings)
/*
    SetPrintSettingsOnPrivateData takes the passed in PMPrintSettings object and stores
    it on our private data. In doing so it does a PMRetain so that the caller of this
    routine can safely release its reference to the object. In addition this
    routine releases any previous page settings that were stored with our private
    data.
    
    Couple of specific issues:
    a) we check that the passed in printSettings is not NULL before we try
        to retain it.
    b) we check that the printSettings that is stored on our private data is not
        NULL before we try to release it.
    c) we first retain the object passed in before we release the data already
        stored on our private data so that if they are the SAME PMPrintSettings
        object we don't release it before retaining it since that release
        could cause the object to be deallocated.
*/
{
    OSStatus err = noErr;
    if(ourData) {
        if(printSettings) {
            err = PMRetain(printSettings);// we'll retain the print settings we put on our private data
        }
        if(!err) {
            if( ourData->printSettings )
                err = PMRelease(ourData->printSettings);
                
            ourData->printSettings = printSettings;
        }
    }
        
    return err;
}

PMPageFormat GetPageFormatFromPrivateData(const DrawDataRef ourData)
/*
    This routine has 'Get' semantics, that is the caller does
    not own the reference returned and should not release it. 
*/
{
    return ourData ? ourData->pageFormat : NULL;
}


PMPrintSettings GetPrintSettingsFromPrivateData(const DrawDataRef ourData)
/*
    This routine has 'Get' semantics, that is the caller does
    not own the reference returned and should not release it. 
*/
{
    return ourData ? ourData->printSettings : NULL;
}


/*------------------------------------------------------------------------------
	Function:
		FlattenAndSavePageFormat
	
	Parameters:
		pageFormat	-	a PageFormat object
	
	Description:
		Flattens a PageFormat object so it can be saved with the document.
		Assumes caller passes a validated PageFormat object.
		
------------------------------------------------------------------------------*/
OSStatus FlattenAndSavePageFormat(DrawDataRef ourData)
{
    OSStatus	status;
    CFDataRef	flatFormatCFData = NULL;
	
    //	Flatten the PageFormat object to memory.
    status = PMFlattenPageFormatToCFData( GetPageFormatFromPrivateData(ourData), &flatFormatCFData);

    if((status == noErr) && (ourData != NULL)) {
        //	Write the PageFormat data to the "document" file.
        //	In this sample code we simply associate it with the document data.
		
	    //  If there is already existing page format data, we'll release it.
		if(ourData->flatPageFormat)
			CFRelease(ourData->flatPageFormat);
			
       ourData->flatPageFormat = flatFormatCFData;
    }

    return status;
}	//	FlattenAndSavePageFormat



/*------------------------------------------------------------------------------
    Function:	LoadAndUnflattenPageFormat
	
    Parameters:
        pageFormat	- PageFormat object read from document file
	
    Description:
        Gets flattened PageFormat data from the document and returns a PageFormat
        object.
        The function is not called in this sample code but your application
        will need to retrieve PageFormat data saved with documents.
		
------------------------------------------------------------------------------*/
OSStatus	LoadAndUnflattenPageFormat(DrawDataRef ourData)
{
    OSStatus	status = noErr;
    CFDataRef	flatFormatCFData = NULL;

    if(ourData != NULL) {
	   //	Read the PageFormat flattened data from file.
		//	In this sample code we simply copy it from a global.
		flatFormatCFData = ourData->flatPageFormat;
		if(flatFormatCFData) {
			//	Convert the PageFormat flattened data into a PageFormat object.
			status = PMUnflattenPageFormatWithCFData(flatFormatCFData, &ourData->pageFormat);
		}
	}
    return status;
}	//	LoadAndUnflattenPageFormat

OSStatus DisposeWindowPrivateData(DrawDataRef ourData)
{
    OSStatus err = noErr, tempErr;

    if(ourData != NULL) {
        // release our page format and print settings if they exist.
        // Setting them to NULL causes any data already stored
        // there to be released
        err = SetPageFormatOnPrivateData(ourData, NULL);		
        tempErr = SetPrintSettingsOnPrivateData(ourData, NULL);
        if(!err)
            err = tempErr;
			
		// Release any flattened page format data we have.	
		if(ourData->flatPageFormat)
			CFRelease(ourData->flatPageFormat);
        
        free(ourData); // free the outer structure
    }
    return err;
}

OSStatus MakeNewDocument(void)
{
    OSStatus err = noErr;
    // calloc sets all our struct fields to 0
    DrawDataRef ourData = (DrawDataRef)calloc(1, sizeof(DrawData));	
    
    if(!ourData)
        err = memFullErr;
       
    if(!err) {
		/* This sample code models an application where the
		   the page format provides needed information for formatting
		   a document. Either that page format comes from that stored
		   with an already existing document or is newly created
		   for each new document. Here each document is new so
		   we create a page format for that document and keep
		   it with our document's data.
		*/
		err = CreateDefaultPageFormatForDocument(ourData);
    }
       
    if (!err) {
		WindowRef window = MakeWindow(ourData);	// this will be our Window 

		if(window) {
			ShowWindow(window);
		} else {
			err = kCantCreateWindow;
		}
    }

    return err;	
}  // MakeNewDocument

void DrawPage(CGContextRef context, const CGRect *drawingRectP, UInt32 pageNumber, Boolean drawTitle, const DrawDataRef ourData)
{
    static const char *drawString = "Drawing Page Number ";
    static const char *titleString = "Carbon Printing Sample";
    char scratch[20];
	
	require(pageNumber <= GetMyDocumentNumPagesInDoc(ourData), bail);
    
	// In this sample code we do some very simple text drawing. More realistic code
	// would use document data to draw a given page.
    
    CGContextSelectFont(context, "Helvetica", 36, kCGEncodingMacRoman);
    // This starts the string 72 units in from left edge of drawing canvas and
    // 72 units down from the top of the drawing canvas.
    CGContextShowTextAtPoint(context, 72, drawingRectP->size.height - 72, drawString, strlen(drawString));
    // Compute the string representing the page number.
    int strlength = snprintf(scratch, sizeof(scratch), "%d", (int)pageNumber);
    CGContextShowText(context, scratch, strlength);
	if(drawTitle) {
		CGContextSelectFont(context, "Helvetica", 18, kCGEncodingMacRoman);
		CGContextShowTextAtPoint(context, 36, drawingRectP->size.height - 36, titleString, strlen(titleString));
	}
	
bail:
	return;
}