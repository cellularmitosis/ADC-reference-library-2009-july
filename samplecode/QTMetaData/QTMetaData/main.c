/*

	File:		main.c

	Contains:	High-level code to display QuickTime MetaData for a QuickTime
				movie.

	Version:	1.0

Disclaimer: IMPORTANT:  This Apple software is supplied to you by Apple
Computer, Inc. ("Apple") in consideration of your agreement to the
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

Copyright © 2005 Apple Computer, Inc., All Rights Reserved

*/ 

#include <Carbon/Carbon.h>
#include <QuickTime/QuickTime.h>

#include "MetaDataUtils.h"
#include "QTUtils.h"
#include "DumpMetaData.h"

pascal OSStatus CommandProcess(EventHandlerCallRef nextHandler, EventRef theEvent, void* userData);
void DoHITextViewFromAPIs(void);
OSStatus GetHITextViewForWindow(WindowRef window, HIViewRef *outTextView);
void DrawStringToFrontWindow(CFStringRef stringRef);
void DrawUTF8DataAsString(QTPropertyValuePtr keyValuePtr);
void CreateNewMetaDataWindow(void);


enum 
{
	kMyCreator = ' DTS',
	kMyPropTag = 'CUST'
};

struct MyWinCustomData 
{
	HIViewRef theTextView;	// Text View for window
};
typedef struct MyWinCustomData MyWinCustomData;


//////////
//
// GetTextViewForFrontWindow
//
// Retrieve the Text View for the frontmost window
//
//////////

HIViewRef GetTextViewForFrontWindow()
{
	WindowRef frontWindow = GetFrontWindowOfClass(kDocumentWindowClass, true);
	require(frontWindow != nil, CantGetFrontWindow);

	UInt32 actualSize;
	MyWinCustomData	winData;

	// get our custom structure with the text view info.
	GetWindowProperty(frontWindow,
					  kMyCreator,
					  kMyPropTag,
					  sizeof (struct MyWinCustomData),
					  &actualSize,            /* can be NULL */
					  (void *)&winData);
					  
	return (winData.theTextView);
	
CantGetFrontWindow:
	return nil;
}


//////////
//
// AddTextToTextView
//
// Set text data to the specified text view
//
//////////

void AddTextToTextView(HIViewRef textView, UniChar *buffer, ByteCount size)
{
	TXNOffset oStartOffset, oEndOffset;
	TXNGetSelection (HITextViewGetTXNObject(textView),&oStartOffset,&oEndOffset);
	TXNSetData(HITextViewGetTXNObject(textView), kTXNUnicodeTextData, buffer, size, oEndOffset, oEndOffset);
}

//////////
//
// DrawCFStringToTextView
//
// Draw the CFString to the specified Text View
//
//////////

void DrawCFStringToTextView(CFStringRef stringRef, HIViewRef textView)
{
	CFIndex size = CFStringGetLength (stringRef);
	CFRange range = CFRangeMake(0, size);
	UniChar *buffer = (UniChar *)malloc(range.length  * sizeof(UniChar));
	require(buffer != nil, CantMakeBuffer);

	CFStringGetCharacters(stringRef, range, buffer);
	
	AddTextToTextView(textView, buffer, range.length  * sizeof(UniChar));
	
	free((void *)buffer);
		
CantMakeBuffer:
	return;
}

//////////
//
// DrawStringToFrontWindow
//
// Draw the CFString to the Text View for the frontmost window
//
//////////

void DrawStringToFrontWindow(CFStringRef stringRef)
{
	DrawCFStringToTextView(stringRef, GetTextViewForFrontWindow());
}


//////////
//
// DrawStorageCountStrToWindow
//
// Draw our specially formatted string for the storage format containing :
//		- the storage format type, plus
//		- count of the number of metadata items for that storage format
//
//////////

void DrawStorageCountStrToWindow(char *storageFormatString, ItemCount outCount)
{
	CFStringRef countStr = NULL;
	
	countStr = CFStringCreateWithFormat( kCFAllocatorDefault, 
										NULL, 
										CFSTR("%s storage format \n    item count = %d \n"), 
										storageFormatString,
										outCount );
	if (countStr)
	{
		DrawCFStringToTextView(countStr, GetTextViewForFrontWindow());
		CFRelease(countStr);
	}
}


//////////
//
// CreateNewMetaDataWindow
//
// Create a window to display all the metadata for a given movie
//
//////////

void CreateNewMetaDataWindow(void)
{
	IBNibRef 		nibRef;
	OSStatus		err;

	// Create a Nib reference passing the name of the nib file (without the .nib extension)
	// CreateNibReference only searches into the application bundle.
	err = CreateNibReference(CFSTR("main"), &nibRef);
	require_noerr(err, CantGetNibRef);
	
	WindowRef 		window;

	// Then create a window. "MainWindow" is the name of the window object. This name is set in 
	// InterfaceBuilder when the nib is created.
	err = CreateWindowFromNib(nibRef, CFSTR("MainWindow"), &window);
	require_noerr(err, CantCreateWindow);

	// We don't need the nib reference anymore.
	DisposeNibReference(nibRef);
	
	ControlID id = { 'xyzt', 1 }; // set with Interface Builder in the Inspector
	HIViewRef theTextView = nil;
	err = HIViewFindByID(HIViewGetRoot(window), id, &theTextView);
	MyWinCustomData	winData;
	winData.theTextView = theTextView;
	
	// save the HIView TextView for our window so we can easily access it
	err = SetWindowProperty (  window,
							   kMyCreator,
							   kMyPropTag,
							   sizeof(struct MyWinCustomData),
							   &winData );

	// The window was created hidden so show it.
	ShowWindow(window);	

	// get a movie and display its metadata
	CFStringRef movieName = NULL;
	Movie aMovie = GetMovieAndMovieName(&movieName);
	require(aMovie != nil, CantGetAMovie);
	
	// draw movie name for window title
	err = SetWindowTitleWithCFString(window, movieName);
	
	// show the movie's metadata
	DumpMovieMetaData(aMovie);
	
	EventTypeSpec eventType = {kEventClassCommand, kEventCommandProcess};
	InstallEventHandler(GetApplicationEventTarget(), NewEventHandlerUPP(CommandProcess), 1, &eventType, NULL, NULL);

	return;

CantGetAMovie:
	DisposeWindow(window);

	EventTypeSpec eventType2 = {kEventClassCommand, kEventCommandProcess};
	InstallEventHandler(GetApplicationEventTarget(), NewEventHandlerUPP(CommandProcess), 1, &eventType2, NULL, NULL);

CantCreateWindow:
CantGetNibRef:

	return;
}

//////////
//
// CommandProcess
//
// Standard Event Handler
//
//////////


pascal OSStatus CommandProcess(EventHandlerCallRef nextHandler, EventRef theEvent, void* userData)
{
	#pragma unused (nextHandler, userData)
	HICommand aCommand;
	OSStatus status = noErr;

	GetEventParameter(theEvent, kEventParamDirectObject, typeHICommand, NULL, sizeof(HICommand), NULL, &aCommand);
	  
	switch (aCommand.commandID)
	{
		case kHICommandNew:
			CreateNewMetaDataWindow();	// create new QT MetaData window
			break;
			
		default:
			status = eventNotHandledErr;
			break;
	}
	return status;
}

//////////
//
// main
//
// The main function for this application.
//
//////////

int main(int argc, char* argv[])
{
	OSStatus	err = noErr;

   // Can we run this particular demo application?
	long response;
	err = Gestalt(gestaltSystemVersion, &response);
	Boolean ok = ((err == noErr) && (response >= 0x00001030));
	if (!ok)
	{
		StandardAlert(kAlertStopAlert, "\pMac OS X 10.3 (minimum) is required for this application", "\p", NULL, NULL);
		ExitToShell();
	}

	// Initialize QuickTime
	err = EnterMovies();
	require_noerr(err, CantInitQuickTime);

	IBNibRef 		nibRef;

	// Create a Nib reference passing the name of the nib file (without the .nib extension)
	// CreateNibReference only searches into the application bundle.
	err = CreateNibReference(CFSTR("main"), &nibRef);
	require_noerr(err, CantGetNibRef);
	
	// Once the nib reference is created, set the menu bar. "MainMenu" is the name of the menu bar
	// object. This name is set in InterfaceBuilder when the nib is created.
	err = SetMenuBarFromNib(nibRef, CFSTR("MenuBar"));
	require_noerr(err, CantSetMenuBar);

	// We don't need the nib reference anymore.
	DisposeNibReference(nibRef);

	// create a new window to display metadata items for a QuickTime movie
	CreateNewMetaDataWindow();

	// Call the event loop
	RunApplicationEventLoop();

	return err;
	
CantSetMenuBar:
	DisposeNibReference(nibRef);
CantGetNibRef:
CantInitQuickTime:
	return err;
}

