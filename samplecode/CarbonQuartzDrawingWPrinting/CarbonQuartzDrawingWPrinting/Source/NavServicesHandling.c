/*
	File:		NavServicesHandling.c
	
	Contains:	Code to handle the save dialog for our sample application.

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

#include "NavServicesHandling.h"
#include "AppDrawing.h"
#include "UIHandling.h"

#include "MyCarbonPrinting.h"

#define	kFileCreator			'prvw'
#define kFileTypePDF			'PDF '

#define kFileTypePDFCFStr		CFSTR("%@.pdf")		// our format string for making the save as file name

// these are values are used to keep "unique" set of preferences for each Nav dialog,
// for example: using more than one NavChooseObject style dialog in one application:
#define kSavePrefKey			1

/****  Typedefs ********/

// the structure we're going to give to the save dialog to hang our data off of
typedef struct OurSaveDialogData{	
    NavDialogRef	dialogRef;
    WindowRef		parentWindow;
    void 		*documentDataP;
}OurSaveDialogData;

/**** Prototypes ****/

static OSStatus DoFSRefSave(const OurSaveDialogData *dialogDataP, NavReplyRecord* reply, 
                                                                    AEDesc *actualDescP);

static pascal void NavEventProc( NavEventCallbackMessage callBackSelector, 
                                NavCBRecPtr callBackParms, void* callBackUD );

// this code originates from the NavServices sample code in the CarbonLib SDK
OSStatus DoSaveAsPDF(WindowRef w, void *ourDataP)
{
    OSStatus 			err = noErr;
    static NavEventUPP		gNavEventProc = NULL;		// event proc for our Nav Dialogs 
    NavDialogCreationOptions	dialogOptions;
    if(!gNavEventProc){
        gNavEventProc = NewNavEventUPP(NavEventProc);
        if(!gNavEventProc)
            err = memFullErr;
    }

    if(!err && (( err = NavGetDefaultDialogCreationOptions( &dialogOptions )) == noErr ))
    {
	OurSaveDialogData *dialogDataP = NULL;
	CFStringRef	tempString;
	CopyWindowTitleAsCFString(w, &tempString);

        dialogOptions.saveFileName = CFStringCreateWithFormat(kCFAllocatorDefault, NULL, 
                                                kFileTypePDFCFStr, tempString);
	
	CFRelease(tempString);
	
	// make the dialog modal to our parent doc, AKA sheets
        dialogOptions.parentWindow = w;
	dialogOptions.modality = kWindowModalityWindowModal;
	
	dialogDataP = (OurSaveDialogData *)malloc(sizeof(OurSaveDialogData));
	if(dialogDataP){
	    dialogDataP->dialogRef = NULL;
	    dialogDataP->parentWindow = w;
	    dialogDataP->documentDataP = ourDataP;
	    err = NavCreatePutFileDialog(&dialogOptions, kFileTypePDF, kFileCreator,
						    gNavEventProc, dialogDataP,
						    &dialogDataP->dialogRef);
	    if (!err && dialogDataP->dialogRef != NULL)
	    {
		err = NavDialogRun( dialogDataP->dialogRef );
		if (err != noErr)
		{
		    NavDialogDispose( dialogDataP->dialogRef );
		    dialogDataP->dialogRef = NULL;
		    free(dialogDataP);
		}
	    }
	}else
	    err = memFullErr;
       	
     	if ( dialogOptions.saveFileName != NULL )
	    CFRelease( dialogOptions.saveFileName );

    }
    return err;
}

static pascal void NavEventProc( NavEventCallbackMessage callBackSelector, 
                                NavCBRecPtr callBackParms, void* callBackUD )
{
    OurSaveDialogData *dialogDataP = (OurSaveDialogData*)callBackUD;
    OSStatus 	err = noErr;		        
	
    switch( callBackSelector )
    {
	case kNavCBUserAction:
	{
	    NavReplyRecord 	reply;
	    NavUserAction 	userAction = 0;
	    
	    if ((err = NavDialogGetReply( callBackParms->context, &reply )) == noErr )
	    {
		OSStatus tempErr;
		userAction = NavDialogGetUserAction( callBackParms->context );
				
		switch( userAction )
		{
		    case kNavUserActionSaveAs:
		    {
			if ( dialogDataP != NULL ){
			    AEDesc	actualDesc;
			    if (
                                (err = AECoerceDesc( &reply.selection, typeFSRef, &actualDesc )) 
                                        == noErr
                            )
			    {	
				// the coercion succeeded as an FSRef, 
                                // so use HFS+ APIs to save the file:
				err = DoFSRefSave( dialogDataP, &reply, &actualDesc);
				AEDisposeDesc( &actualDesc );
			    }else{
                                // the coercion failed as an FSRef, so get the FSSpec and 
                                // save the file
                                /*
                                    FSRef's don't exist on systems prior to MacOS 9 so 
                                    there it is necessary to have a different approach as shown in 
                                    the Nav Services sample in the CarbonSDK. Since this code is 
                                    used on MacOS X only, this is not an issue.
                                */
				// assert(...)
			    }
			}
			break;
		    }
				    				    
		    case kNavUserActionCancel:
			    //..
			    break;
					    
		    case kNavUserActionNewFolder:
			    //..
			    break;
		}
	          
		tempErr = NavDisposeReply( &reply );
		if(!err)
		    err = tempErr;
            }
            break;
	}
			
	case kNavCBTerminate:
	{
	    if( dialogDataP != NULL )
	    {
            	if(dialogDataP->dialogRef)
		    NavDialogDispose(dialogDataP->dialogRef );
		    
            	dialogDataP->dialogRef = NULL;
		free(dialogDataP);
            }
		
        }
	break;
    }
}

static OSStatus DoFSRefSave(const OurSaveDialogData *dialogDataP, 
                                    NavReplyRecord* reply, AEDesc *actualDescP)
{
    OSStatus 	err = noErr;
    FSRef 	fileRefParent;
	    
    if ((err = AEGetDescData( actualDescP, &fileRefParent, sizeof( FSRef ) )) == noErr )
    {
        // get the name data and its length:	
        HFSUniStr255	nameBuffer;
        UniCharCount 	sourceLength = 0;
        
        sourceLength = (UniCharCount)CFStringGetLength( reply->saveFileName );
        
        CFStringGetCharacters( reply->saveFileName, CFRangeMake( 0, sourceLength ), 
                                                        (UniChar*)&nameBuffer.unicode );
        
        if ( sourceLength > 0 )
        {	
            if ( reply->replacing )
            {
                // delete the file we are replacing:
                FSRef fileToDelete;
                if ((err = FSMakeFSRefUnicode( &fileRefParent, sourceLength, nameBuffer.unicode, 
                                    kTextEncodingUnicodeDefault, &fileToDelete )) == noErr )
                {
                    err = FSDeleteObject( &fileToDelete );
                    if ( err == fBsyErr ){
                        DoErrorAlert(fBsyErr, kMyDeleteErrorFormatStrKey);
                    }
                }
            }
                            
            if ( err == noErr )
            {
                // create the file based on Unicode, but we can write the file's data with an FSSpec:
                FSSpec newFileSpec;

                // get the FSSpec back so we can write the file's data
                if ((err = FSCreateFileUnicode( &fileRefParent, sourceLength, 
                                                    nameBuffer.unicode,
                                                    kFSCatInfoNone,
                                                    NULL,
                                                    NULL,	
                                                    &newFileSpec)) == noErr)
                {
                    FInfo fileInfo;
                    fileInfo.fdType = kFileTypePDF;
                    fileInfo.fdCreator = kFileCreator;
                    err = FSpSetFInfo( &newFileSpec, &fileInfo );
                    // now that we have the FSSpec, we can proceed with the save operation:
                    if(!err){
                        FSRef fsRef;
                        err = FSpMakeFSRef(&newFileSpec, &fsRef);	// make an FSRef
                        if(!err){
                            CFURLRef saveURL = CFURLCreateFromFSRef(NULL, &fsRef);
                            if(saveURL){
                                // delete the file we just made for making the FSRef
                                err = FSpDelete(&newFileSpec);	
                                if(!err)
                                    err = MakePDFDocument(dialogDataP->parentWindow, 
                                                            dialogDataP->documentDataP, 
                                                            saveURL);
                                if(!err)
                                    err = NavCompleteSave( reply, kNavTranslateInPlace );
                                
                                if(err){
                                    // an error ocurred saving the file, so delete the copy 
                                    // left over:
                                    (void)FSpDelete( &newFileSpec );
                                    DoErrorAlert(err, kMyWriteErrorFormatStrKey);
                                }
                                CFRelease(saveURL);
                            }else{
                                // delete the file we just made for making the FSRe
                               (void)FSpDelete(&newFileSpec);
                                err = kCantCreateSaveURL;
                                DoErrorAlert(err, kMyCreateURLErrorFormatStrKey);
                            }
                        }
                    }
                }
            }
        }
    }
    return err;
}
