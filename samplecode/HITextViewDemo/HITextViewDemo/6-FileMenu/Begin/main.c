/*

File: main.c

Abstract:  Event handling & main entry point for HITextViewDemo sample project.

Version: <1.0>

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
#include "window_transparency.h"


// Forward declaration for our event handler
OSStatus MyHICommandEventHandler(EventHandlerCallRef nextHandler, EventRef theEvent, void* userData);


//---------------------------------------------------------------------
// Main entry point. Creates first window, runs overall app event loop.
//
int main(int argc, char* argv[])
{
    IBNibRef nibRef;
	EventTypeSpec eventType;
    
	// Install the menubar
    verify_noerr( CreateNibReference(CFSTR("main"), &nibRef) );
	verify_noerr( SetMenuBarFromNib(nibRef, CFSTR("MenuBar")) );
    DisposeNibReference(nibRef);
	
	// Install an event handler for HICommand events
	eventType.eventClass = kEventClassCommand;
	eventType.eventKind = kEventCommandProcess;
	verify_noerr( InstallEventHandler(GetApplicationEventTarget(), NewEventHandlerUPP(MyHICommandEventHandler), 1, &eventType, NULL, NULL) );

	// Create the first document window
	(void)MyCreateNewDocumentWindow();
    
    // Main event loop
    RunApplicationEventLoop();

	return 0;
}


//---------------------------------------------------------------------
// Application-wide handler for HICommand events.
//
OSStatus MyHICommandEventHandler(EventHandlerCallRef nextHandler, EventRef theEvent, void* userData)
{
	HICommand command;
	CFURLRef file = NULL;
	CFDictionaryRef dataOptions = NULL;
	WindowRef window = NULL;
	OSStatus status = noErr;

	status = GetEventParameter(theEvent, kEventParamDirectObject, typeHICommand, NULL, sizeof(HICommand), NULL, &command);
	require_noerr( status, CantGetEventParameter );
      
	switch (command.commandID)
	{
		// **** NEW
		case kHICommandNew:
			(void)MyCreateNewDocumentWindow();
			break;

		// **** OPEN
		case kHICommandOpen:
			file = GetOpenFileFromUser();
			if (file == NULL) break;
			window = MyCreateNewDocumentWindow();
			if (window == NULL) break;

			// <<< Insert file open code here >>>

			TXNClearUndo(HITextViewGetTXNObject(GetTextViewFromWindow(window)));
			verify_noerr( SetWindowModified(window, false) );
			SetWindowTitleAndProxyFromCFURL(window, file);
			CFRelease(file);
			break;

		// **** SAVE and SAVE AS
		case kHICommandSave:
		case kHICommandSaveAs:
			window = GetFrontWindowOfClass(kDocumentWindowClass, true);
			if ( window == NULL ) break;
			if ( command.commandID == kHICommandSave )
				file = GetWindowProxyFileCFURL(window);
			if (file == NULL) file = GetSaveFileFromUser(window);
			if (file == NULL) break;

			// <<< Insert file save code here >>>

			TXNClearUndo(HITextViewGetTXNObject(GetTextViewFromWindow(window)));
			verify_noerr( SetWindowModified(window, false) );
			SetWindowTitleAndProxyFromCFURL(window, file);
			CFRelease(file);
			break;
		
		// **** REVERT
		case kHICommandRevert:
			window = GetFrontWindowOfClass(kDocumentWindowClass, true);
			if ( window == NULL ) break;
			TXNRevert(HITextViewGetTXNObject(GetTextViewFromWindow(window)));
			verify_noerr( SetWindowModified(window, false) );
			break;

		// Page setup and print do not work because this comes from an HITextView, see MLTE documentation
		case kHICommandPageSetup:
			//verify_noerr( TXNPageSetup(HITextViewGetTXNObject(GetTextViewFromWindow(GetFrontWindowOfClass(kDocumentWindowClass, true)))) );
			break;
		case kHICommandPrint:
			//verify_noerr( TXNPrint(HITextViewGetTXNObject(GetTextViewFromWindow(GetFrontWindowOfClass(kDocumentWindowClass, true)))) );
			break;

		default:
			status = eventNotHandledErr;
			break;
	}

CantGetEventParameter:
	return status;
}
