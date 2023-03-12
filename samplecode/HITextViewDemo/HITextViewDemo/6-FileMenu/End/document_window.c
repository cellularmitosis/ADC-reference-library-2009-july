/*

File: document_window.c

Abstract:  Functions to create and manipulate document windows.

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

#include "document_window.h"

// All of our document windows contain an HITextView with this HIViewID
static HIViewID gTextViewID = { 'MTXT', 0 };


//---------------------------------------------------------------------
// Creates one of our simple document windows and returns a ref to it.
//
WindowRef MyCreateNewDocumentWindow(void)
{
    IBNibRef nib;
    WindowRef window = NULL;
	OSStatus status;

    // Create one of our special main document windows
    status = CreateNibReference(CFSTR("main"), &nib);
	require_noerr( status, CantGetNIBRef );
    status = CreateWindowFromNib(nib, CFSTR("MainDocumentWindow"), &window);
	require_noerr( status, CantCreateWindow );

	// Set its default options
	status = MySetTextViewOptions(GetTextViewFromWindow(window));
	check_noerr( status );

	// Make the window transparent
	status = MakeWindowTransparent(window);
	check_noerr( status );

	// Make the TextView partially transparent
	status = TextViewSetAlpha(GetTextViewFromWindow(window), 0.25);
	check_noerr( status );

	// Initialize the window title and proxy icon
	status = SetWindowTitleWithCFString(window, CFSTR("Untitled"));
	check_noerr( status );
	status = SetWindowProxyCreatorAndType(window, kUnknownType, kUnknownType, kOnSystemDisk);
	check_noerr( status );

	// Show the window
    ShowWindow(window);

	// Cleanup
CantGetViewRef:
CantCreateWindow:
    DisposeNibReference(nib);
CantGetNIBRef:
	return window;
}


//---------------------------------------------------------------------
// Sets the alpha of an HITextView to the specified value
//
OSStatus TextViewSetAlpha(HIViewRef textView, float alpha)
{
	CGColorRef prevColor, newColor;
	OSStatus status;
	
	status = HITextViewCopyBackgroundColor(textView, &prevColor);
	require_noerr( status, CantGetBackgroundColor );
	
	newColor = CGColorCreateCopyWithAlpha(prevColor, alpha);
	require( (newColor != NULL), CantCreateNewColor );
	
	status = HITextViewSetBackgroundColor(textView, newColor);
	check_noerr( status );

	CGColorRelease(newColor);
CantCreateNewColor:
	CGColorRelease(prevColor);
CantGetBackgroundColor:
	return status;
}


//---------------------------------------------------------------------
// Sets some default options in our main text view.
//
OSStatus MySetTextViewOptions(HIViewRef textView)
{
	TXNCommandEventSupportOptions options;
	TXNObject txnObj;
	OSStatus status;

	// We are setting options that must be set in the
	// underlying MLTE object, so get a reference to it
	txnObj = HITextViewGetTXNObject(textView);
	require( (txnObj != NULL), CantGetTXNObject );

	// Get the previous options
	status = TXNGetCommandEventSupport(txnObj, &options);
	require_noerr( status, CantGetCommandOptions );

	// Add the font panel support options
	options |= kTXNSupportFontCommandProcessing;
	options |= kTXNSupportFontCommandUpdating;

	// Add the spelling panel support options
	options |= kTXNSupportSpellCheckCommandProcessing;
	options |= kTXNSupportSpellCheckCommandUpdating;

	// Apply above changes to the object
	status = TXNSetCommandEventSupport(txnObj, options);
	require_noerr( status, CantSetCommandOptions );

CantSetCommandOptions:
CantGetCommandOptions:
CantGetTXNObject:
	return status;
}


//---------------------------------------------------------------------
// Returns the HITextView ref from one of our windows.
//
HIViewRef GetTextViewFromWindow(WindowRef window)
{
	HIViewRef textView = NULL;

	if( window != NULL )
		verify_noerr( HIViewFindByID(HIViewGetRoot(window), gTextViewID, &textView) );

	return textView;
}


//---------------------------------------------------------------------
// Sets up the window's title and proxy icon based on the given file.
//
void SetWindowTitleAndProxyFromCFURL(WindowRef window, CFURLRef file)
{
	CFStringRef title;
	FSRef fileAsFSRef;
	
	require( (CFURLGetFSRef(file, &fileAsFSRef) != false), CantGetFSRef );
	verify_noerr( HIWindowSetProxyFSRef(window, &fileAsFSRef) );

	title = CFURLCopyLastPathComponent(file);
	verify_noerr( SetWindowTitleWithCFString(window, title) );
	CFRelease(title);

CantGetFSRef:
	return;
}


//---------------------------------------------------------------------
// Gets a window's proxy file CFURL. Caller must release the CFURLRef.
//
CFURLRef GetWindowProxyFileCFURL(WindowRef window)
{
	FSRef fileAsFSRef;
	CFURLRef fileAsCFURL = NULL;
	OSStatus status;

	status = HIWindowGetProxyFSRef(window, &fileAsFSRef);

	if ( status == noErr ) // errWindowDoesNotHaveProxy could be special cased here
		fileAsCFURL = CFURLCreateFromFSRef(NULL, &fileAsFSRef);
		
	return fileAsCFURL;
}
