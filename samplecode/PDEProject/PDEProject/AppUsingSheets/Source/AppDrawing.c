/*
	File:		AppDrawing.c
	
	Contains:	Drawing code for our sample document

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
#include "AppDrawing.h"
#include "UIHandling.h"

#define	kNumDocumentPages 	10

/********** our data types	***********/
	
typedef struct DataToDraw{
    PMPageFormat pageFormat;		// the page format for this document
    PMPrintSettings printSettings;	// the print settings for a given print job
}DataToDraw, *DataToDrawPtr;

/**** our private prototypes ***/

static PageDrawProc MyPageDrawProc;

/*** the routines ***/

OSStatus CopyDocumentName(void *documentDataP, CFStringRef *stringRefP)
/*
    This routine normally return a reference (owned by the caller)
    for the name of the document which may or may not be the window
    title of the window being printed. In our sample code we'll just
    get a constant from our localized strings.
*/
{
#pragma unused (documentDataP)
    OSStatus err = noErr;
    *stringRefP = CFCopyLocalizedString(kMyDocumentTitleKey, NULL);
    if(!*stringRefP)
        err = kCantGetDocumentLocalizedStringErr;
        
    return err;
}


UInt32 MyGetDocumentNumPagesInDoc(const void *ourDataP)
{
#pragma unused (ourDataP)
    // more realistic code would use the document's page format to determine the number of pages
    // in our document
    return kNumDocumentPages;		// we always have kNumDocumentPages page per document
}

PageDrawProc *GetMyDrawPageProc(const void *ourDataP)
{
#pragma unused (ourDataP)
    return MyPageDrawProc;
}

OSStatus SetPageFormatOnPrivateData(void *ourDataP, PMPageFormat pageFormat)
/*
    SetPageFormatOnPrivateData takes the passed in PMPageFormat object and stores it on our
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
    if(ourDataP){
        if(pageFormat){
            err = PMRetain(pageFormat);		// we'll retain the format we put on our private data
        }
        if(!err){
            if( ((DataToDrawPtr)ourDataP)->pageFormat )
                err = PMRelease(((DataToDrawPtr)ourDataP)->pageFormat);
                
            ((DataToDrawPtr)ourDataP)->pageFormat = pageFormat;
        }
    }
        
    return err;
}

OSStatus SetPrintSettingsOnPrivateData(void *ourDataP, PMPrintSettings printSettings)
/*
    SetPrintSettingsOnPrivateData takes the passed in PMPrintSettings object and stores
    it on our private data. In doing so it does a PMRetain so that the caller of this
    routine can safely release its reference to the object. In addition this
    routine releases any previous page settings that was stored with our private
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
    if(ourDataP){
        if(printSettings){
            err = PMRetain(printSettings);// we'll retain the print settings we put on our private data
        }
        if(!err){
            if( ((DataToDrawPtr)ourDataP)->printSettings )
                err = PMRelease(((DataToDrawPtr)ourDataP)->printSettings);
                
            ((DataToDrawPtr)ourDataP)->printSettings = printSettings;
        }
    }
        
    return err;
}

PMPageFormat GetPageFormatFromPrivateData(const void *ourDataP)
/*
    This routine has 'Get' semantics, that is the caller does
    not own the reference returned and should not release it. 
*/
{
    return ourDataP ? ((DataToDrawPtr)ourDataP)->pageFormat : NULL;
}


PMPrintSettings GetPrintSettingsFromPrivateData(const void *ourDataP)
/*
    This routine has 'Get' semantics, that is the caller does
    not own the reference returned and should not release it. 
*/
{
    return ourDataP ? ((DataToDrawPtr)ourDataP)->printSettings : NULL;
}

OSStatus DisposeWindowPrivateData(void *ourDataP)
{
    OSStatus err = noErr, tempErr;
    DataToDrawPtr ourDataToDrawP = (DataToDrawPtr)ourDataP;
    if(ourDataToDrawP != NULL){
        // release our page format and print settings if they exist.
        // Setting them to NULL causes any data already stored
        // there to be released
        err = SetPageFormatOnPrivateData(ourDataToDrawP, NULL);		
        tempErr = SetPrintSettingsOnPrivateData(ourDataToDrawP, NULL);
        if(!err)
            err = tempErr;
        
        free(ourDataToDrawP);	// free the outer structure
    }
    return err;
}

OSStatus MakeNewDocument(void)
{
    OSStatus err = noErr;
    // calloc sets all our struct fields to 0
    DataToDraw *ourDataP = (DataToDraw *)calloc(1, sizeof(DataToDraw));	
    
    if(!ourDataP)
        err = memFullErr;
       
    if(!err){
	/* This sample code models an application where the
	   the page format provides needed information for formatting
	   a document. Either that page format comes from that stored
	   with an already existing document or is newly created
	   for each new document. Here each document is new so
	   we create a page format for that document and keep
	   it with our document's data.
	*/
	err = CreateDefaultPageFormatForDocument(ourDataP);
    }
       
    if (!err){
	WindowRef qdWindow = MakeWindow(ourDataP);	// this will be our Window 

	if(qdWindow){
            Rect bounds;
	    SetPortWindowPort(qdWindow);
	    GetWindowPortBounds(qdWindow, &bounds);
	    ClipRect(&bounds);
	    InvalWindowRect(qdWindow, &bounds); 
            ShowWindow(qdWindow);
	}else
	    err = kCantCreateWindow;
    }


    return err;	
 }

static OSStatus MyPageDrawProc(const void *refCon, const Rect *drawingRectP, UInt32 pageNumber)
{
#pragma unused (refCon, drawingRectP)
    OSStatus err = noErr;
    Str255 pageNumberString;
    //	In this sample code we do some very simple text drawing. More realistic code
    // would use our refCon document data to draw a given page
        
    MoveTo(72,72);
    TextFont(kFontIDHelvetica);
    TextSize(24);
    DrawString("\pDrawing Page Number ");
    NumToString(pageNumber, pageNumberString);
    DrawString(pageNumberString);

    return err;
}	//	MyPageDrawProc
