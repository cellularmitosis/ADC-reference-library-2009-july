/*

File: nav.c

Abstract:  Helper routines for file open & save dialogs.

© Copyright 2005 Apple Computer, Inc. All rights reserved.

IMPORTANT:  This Apple software is supplied to you by Apple Computer,
Inc. ("Apple") in consideration of your agreement to the following
terms, and your use, installation, modification or redistribution of
this Apple software constitutes acceptance of these terms.  If you do
not agree with these terms, please do not use, install, modify or
redistribute this Apple software.

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

*/

#include "nav.h"
#include "document_window.h"


//---------------------------------------------------------------------
// Gets a file to open from the user. Caller must release the CFURLRef.
//
CFURLRef GetOpenFileFromUser(void)
{
	NavDialogCreationOptions dialogOptions;
	NavDialogRef dialog;
	NavReplyRecord replyRecord;
	CFURLRef fileAsCFURLRef = NULL;
	FSRef fileAsFSRef;
	OSStatus status;

	// Get the standard set of defaults
	status = NavGetDefaultDialogCreationOptions(&dialogOptions);
	require_noerr( status, CantGetNavOptions );

	// Make the window app-wide modal
	dialogOptions.modality = kWindowModalityAppModal;

	// Create the dialog
	status = NavCreateGetFileDialog(&dialogOptions, NULL, NULL, NULL, NULL, NULL, &dialog);
	require_noerr( status, CantCreateDialog );

	// Show it
	status = NavDialogRun(dialog);
	require_noerr( status, CantRunDialog );
	
	// Get the reply
	status = NavDialogGetReply(dialog, &replyRecord);
	require( ((status == noErr) || (status == userCanceledErr)), CantGetReply );

	// If the user clicked "Cancel", just bail
	if ( status == userCanceledErr ) goto UserCanceled;

	// Get the file
	status = AEGetNthPtr(&(replyRecord.selection), 1, typeFSRef, NULL, NULL, &fileAsFSRef, sizeof(FSRef), NULL);
	require_noerr( status, CantExtractFSRef );
	
	// Convert it to a CFURL
	fileAsCFURLRef = CFURLCreateFromFSRef(NULL, &fileAsFSRef);

	// Cleanup
CantExtractFSRef:
UserCanceled:
	verify_noerr( NavDisposeReply(&replyRecord) );
CantGetReply:
CantRunDialog:
	NavDialogDispose(dialog);
CantCreateDialog:
CantGetNavOptions:
    return fileAsCFURLRef;
}


//---------------------------------------------------------------------
// Gets a file to save from the user. Caller must release the CFURLRef.
//
CFURLRef GetSaveFileFromUser(WindowRef window)
{
	CFURLRef previousFile;
	CFStringRef saveFileName;
	UniChar *chars = NULL;
    CFIndex length;
	NavDialogCreationOptions dialogOptions;
	NavDialogRef dialog;
	NavReplyRecord replyRecord;
	CFURLRef fileAsCFURLRef = NULL;
	FSRef fileAsFSRef;
	FSRef parentDirectory;
	OSStatus status;

	if ( window == NULL ) return NULL;

	// Get the standard set of defaults
	status = NavGetDefaultDialogCreationOptions(&dialogOptions);
	require_noerr( status, CantGetNavOptions );

	// Change a few things (app-wide modal, show the extension)
	dialogOptions.modality = kWindowModalityAppModal;
	dialogOptions.parentWindow = window;
	dialogOptions.optionFlags = kNavDefaultNavDlogOptions | kNavPreserveSaveFileExtension;

	// Set up the default save name
	previousFile = GetWindowProxyFileCFURL(window);
	if (previousFile == NULL)
		dialogOptions.saveFileName = CFStringCreateWithCString(NULL, "Untitled.rtf", kCFStringEncodingASCII);
	else	
		dialogOptions.saveFileName = CFURLCopyLastPathComponent(previousFile);

	// Create the dialog
	status = NavCreatePutFileDialog(&dialogOptions, kUnknownType, kUnknownType, NULL, NULL, &dialog);
	require_noerr( status, CantCreateDialog );

	// Show it
	status = NavDialogRun(dialog);
	require_noerr( status, CantRunDialog );
	
	// Get the reply
	status = NavDialogGetReply(dialog, &replyRecord);
	require( ((status == noErr) || (status == userCanceledErr)), CantGetReply );

	// If the user clicked "Cancel", just bail
	if ( status == userCanceledErr ) goto UserCanceled;

	// Get the file's location and name
	status = AEGetNthPtr(&(replyRecord.selection), 1, typeFSRef, NULL, NULL, &parentDirectory, sizeof(FSRef), NULL);
	require_noerr( status, CantExtractFSRef );

	saveFileName = replyRecord.saveFileName;
    length = CFStringGetLength(saveFileName);
	chars = malloc(length * sizeof(UniChar));
	CFStringGetCharacters(saveFileName, CFRangeMake(0, length), chars);

    // If we are replacing a file, erase the previous one
    if ( replyRecord.replacing ) {

		status = FSMakeFSRefUnicode(&parentDirectory, length, chars, kTextEncodingUnknown, &fileAsFSRef);
		require_noerr( status, CantMakeFSRef );

        status = FSDeleteObject(&fileAsFSRef);
		require_noerr( status, CantDeletePreviousFile );
    }

    // Create the file
	status = FSCreateFileUnicode(&parentDirectory, length, chars, kFSCatInfoNone, NULL, &fileAsFSRef, NULL);
	require_noerr( status, CantCreateSaveFile );

	// Convert the reference to the file to a CFURL
	fileAsCFURLRef = CFURLCreateFromFSRef(NULL, &fileAsFSRef);

	// Cleanup
CantCreateSaveFile:
CantDeletePreviousFile:
CantMakeFSRef:
	if ( chars != NULL ) free(chars);
CantExtractFSRef:
UserCanceled:
	verify_noerr( NavDisposeReply(&replyRecord) );
CantGetReply:
CantRunDialog:
	NavDialogDispose(dialog);
CantCreateDialog:
	if (previousFile) CFRelease(previousFile);
	CFRelease(dialogOptions.saveFileName);
CantGetNavOptions:
    return fileAsCFURLRef;
}
