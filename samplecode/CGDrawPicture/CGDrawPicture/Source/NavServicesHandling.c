/*
	File:		NavServicesHandling.c
	
	Contains:	Code to handle the open and save dialogs for our sample application.

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

	Copyright © 1999-2001 Apple Computer, Inc., All Rights Reserved
*/

#ifdef __APPLE_CC__
#include <Carbon/Carbon.h>
#else
#include <Carbon.h>
#endif

#include "NavServicesHandling.h"
#include "CGDrawPicture.h"
#include "UIHandling.h"


#define	kFileCreator			'prvw'	// whatever app should own the PDF files we create
#define kFileTypePICT			'PICT'
#define kFileTypePDF			'PDF '

#define kFileTypePDFCFStr		CFSTR("%@.pdf")		// our format string for making the save as file name

// these are values are used to keep "unique" set of preferences for each Nav dialog,
// for example: using more than one NavChooseObject style dialog in one application:
#define kOpenPrefKey			1
#define kSavePrefKey			2


/****  Typedefs ********/

// the structure we're going to give to the save dialog to hang our data off of
typedef struct OurSaveDialogData{	
    NavDialogRef	dialogRef;
    Boolean		isOpenDialog;
    const void 		*documentDataP;
}OurSaveDialogData;

/**** Prototypes ****/

static OSStatus DoFSRefSave(const OurSaveDialogData *dialogDataP, NavReplyRecord* reply, 
                                                                    AEDesc *actualDescP);
static OSStatus GetFSSpecInfo( AEDesc* fileObject, FSSpec* returnSpec );

/****	Global Data ****/

OSStatus DoSaveAsPDF(WindowRef w, const void *ourDataP)
{
    OSStatus 			err = noErr;
    NavDialogCreationOptions	dialogOptions;
    
    if (( err = NavGetDefaultDialogCreationOptions( &dialogOptions )) == noErr )
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
	    dialogDataP->isOpenDialog = false;
	    dialogDataP->documentDataP = ourDataP;
	    err = NavCreatePutFileDialog(&dialogOptions, kFileTypePDF, kFileCreator,
						    gNavEventProc, dialogDataP,
						    &dialogDataP->dialogRef);
	    if (!err && dialogDataP->dialogRef != NULL)
	    {
		err = NavDialogRun( dialogDataP->dialogRef );
		if (err != noErr )
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

static OSStatus DoFSRefSave(const OurSaveDialogData *dialogDataP, NavReplyRecord* reply, AEDesc *actualDescP)
{
    OSStatus 	err = noErr;
    FSRef 	fileRefParent;
	    
    if ((err = AECoerceDesc( &reply->selection, typeFSRef, actualDescP )) == noErr)
    {
	if ((err = AEGetDescData( actualDescP, &fileRefParent, sizeof( FSRef ) )) == noErr )
	{
	    // get the name data and its length:	
	    HFSUniStr255	nameBuffer;
	    UniCharCount 	sourceLength = 0;
	    
	    sourceLength = (UniCharCount)CFStringGetLength( reply->saveFileName );
	    
	    CFStringGetCharacters( reply->saveFileName, CFRangeMake( 0, sourceLength ), (UniChar*)&nameBuffer.unicode );
	    
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
			    if (( err = MakePDFDocument( &newFileSpec, dialogDataP->documentDataP)) == noErr )
				err = NavCompleteSave( reply, kNavTranslateInPlace );
			    else{
				// an error ocurred saving the file, so delete the copy left over:
				(void)FSpDelete( &newFileSpec );
				DoErrorAlert(err, kMyWriteErrorFormatStrKey);
			    }
			}
		    }
		}
	    }
	}
    }
    return err;
}

// *****************************************************************************
// *
// *	GetFSSpecInfo( )
// *
// *	Given a generic AEDesc, this routine returns the FSSpec of that object.
// *	Otherwise it returns an error.
// *	
// *****************************************************************************
static OSStatus GetFSSpecInfo( AEDesc* fileObject, FSSpec* returnSpec )
{
    OSStatus 	theErr = noErr;
    AEDesc		theDesc;
    
    if ((theErr = AECoerceDesc( fileObject, typeFSS, &theDesc )) == noErr)
    {
	theErr = AEGetDescData( &theDesc, returnSpec, sizeof ( FSSpec ) );
	AEDisposeDesc( &theDesc );
    }
    else
    {
	if ((theErr = AECoerceDesc( fileObject, typeFSRef, &theDesc )) == noErr)
	{
	    FSRef ref;
	    if ((theErr = AEGetDescData( &theDesc, &ref, sizeof( FSRef ) )) == noErr)
		theErr = FSGetCatalogInfo( &ref, kFSCatInfoGettableInfo, NULL, NULL, returnSpec, NULL );
	    AEDisposeDesc( &theDesc );
	}
    }

    return theErr;
}

OSStatus DoOpenDocument(void)
{
    OSStatus 	err = noErr;
    NavDialogCreationOptions	dialogOptions;
    OurSaveDialogData *dialogDataP = NULL;

    // while our open dialog is up we'll disable our Open command, else
    // we might end up with more than one open dialog. Yuk
    DisableMenuCommand(NULL, kHICommandOpen);

    dialogDataP = (OurSaveDialogData *)calloc(1, sizeof(OurSaveDialogData));
    if(dialogDataP){
	dialogDataP->isOpenDialog = true;
    }else
	err = memFullErr;
	
    if (!err && ( err = NavGetDefaultDialogCreationOptions( &dialogOptions )) == noErr )
    {
	NavTypeListHandle	openListH = NULL;
	int			numTypes = 1;
	
	openListH = (NavTypeListHandle)NewHandle( sizeof(NavTypeList) + 
						numTypes * sizeof(OSType) );
	(*openListH)->componentSignature = kNavGenericSignature;
	(*openListH)->reserved = 0;
	(*openListH)->osTypeCount = numTypes;
	(*openListH)->osType[0] = kFileTypePICT;

	dialogOptions.preferenceKey = kOpenPrefKey;
	
	dialogOptions.modality = kWindowModalityNone;	// make it modeless
	
	if ((err = NavCreateGetFileDialog( 	&dialogOptions,
						openListH,
						gNavEventProc,
						NULL,		// no custom previews
						NULL,		// filter proc is NULL
						dialogDataP,
						&dialogDataP->dialogRef )) == noErr)
	    {
		if (( err = NavDialogRun( dialogDataP->dialogRef )) != noErr){
		    if ( dialogDataP->dialogRef != NULL ){
			NavDialogDispose( dialogDataP->dialogRef );
			dialogDataP->dialogRef = NULL;
			free(dialogDataP);
		    }
		}
	    }
	if ( openListH != NULL )
	    DisposeHandle( (Handle)openListH );
	
	if(err == userCanceledErr)
	    err = noErr;
	    
    }

    return err;
}

pascal void NavEventProc( NavEventCallbackMessage callBackSelector, NavCBRecPtr callBackParms, void* callBackUD )
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
			    if ((err = AECoerceDesc( &reply.selection, typeFSRef, &actualDesc )) == noErr)
			    {	
				// the coercion succeeded as an FSRef, so use HFS+ APIs to save the file:
				err = DoFSRefSave( dialogDataP, &reply, &actualDesc);
				AEDisposeDesc( &actualDesc );
			    }else{
				// the coercion failed as an FSRef, so get the FSSpec and save the file
                                /*
                                    FSRef's don't exist on systems prior to MacOS 9 so there it is
                                    necessary to have a different approach as shown in the Nav Services
                                    sample in the CarbonSDK. Since this code is MacOS X based only,
                                    this is not an issue.
                                */
				// assert(...)
			    }
			}
			break;
		    }


		    case kNavUserActionOpen:
		    {        
			if (dialogDataP != NULL )
			{	
			    long count = 0;
			    short index;
			    err = AECountItems( &reply.selection, &count );
			    for ( index = 1; index <= count; index++ )
			    {
				AEKeyword 	keyWord;
				AEDesc		theDesc;
				if (( err = AEGetNthDesc( &reply.selection, index, typeWildCard, &keyWord, &theDesc )) == noErr )
				{
				    FSSpec 	fileSpec;
				    FInfo	fileInfo;
				    
				    if (( err = GetFSSpecInfo( &theDesc, &fileSpec )) == noErr ){
					// decide if the doc we are opening is a 'PICT':
					if (( err = FSpGetFInfo( &fileSpec, &fileInfo)) == noErr )
					{
					    if ( fileInfo.fdType == kFileTypePICT )
						(void)doTheFile( &fileSpec );
					    else
					    {
						// error:
						// if we got this far, the document is a type we can't open and
						// (most likely) built-in translation was turned off.
						// You can alert the user that this returned selection or file spec
						// needs translation.
					    }
					}
				    }
				    AEDisposeDesc( &theDesc );
				}
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
		// re-enable open if needed
		if(dialogDataP->isOpenDialog)
		    EnableMenuCommand(NULL, kHICommandOpen);

		free(dialogDataP);
            }
		
        }
	break;
    }
}
