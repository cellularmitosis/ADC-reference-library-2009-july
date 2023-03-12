 /*

File: QTKitPlayerAppDelegate.m

Abstract: QTKitPlayer Application Delegate to handle opening of
               movies from URLs and movie Inspector windows.

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

Copyright (C) 2004 - 2007 Apple Inc. All Rights Reserved.

*/

#import "QTKit/QTKit.h"
#import "QTKitPlayerAppDelegate.h"
#import "MovieDocument.h"

enum
{
    kQTKitPlayerOpenAsURL = 0,
	kQTKitPlayerOpenAsData
  
};

@implementation QTKitPlayerAppDelegate

- (void) dealloc
{
	[mInspectorController release];
	[super dealloc];
}

#pragma mark --- NSMenu validation protocol ---
- (BOOL)validateMenuItem:(NSMenuItem *)menuItem
{
    BOOL	valid = NO;
    SEL		action;

    // init
    action = [menuItem action];

    // validate
    if (action == @selector(doOpenURL:))
        valid = YES;
    else if (action == @selector(doOpenURLData:))
        valid = YES;
    else if((action == @selector(showInspectorWindow:)))
    {
    	valid = YES;
    }
    else
        valid = [[NSDocumentController sharedDocumentController] validateMenuItem:menuItem];

    return valid;
}

#pragma mark --- OpenURLPanel delegates ---
- (void)openURLPanelDidEnd:(OpenURLPanel *)openURLPanel returnCode:(int)returnCode contextInfo:(void *)contextInfo
{
    BOOL closePanel = YES;

    // create the movie document
    if (returnCode == NSOKButton)
        closePanel = [self createMovieDocumentWithURL:[openURLPanel url] asData:((long)contextInfo == kQTKitPlayerOpenAsData)];

    if (closePanel)
        [openURLPanel close];
}

#pragma mark --- actions ---

- (IBAction)doOpenURL:(id)sender
{
    [[OpenURLPanel openURLPanel] beginSheetWithWindow:nil delegate:self didEndSelector:@selector(openURLPanelDidEnd:returnCode:contextInfo:) contextInfo:((void *)kQTKitPlayerOpenAsURL)];
}

- (IBAction)doOpenURLData:(id)sender
{
    [[OpenURLPanel openURLPanel] beginSheetWithWindow:nil delegate:self didEndSelector:@selector(openURLPanelDidEnd:returnCode:contextInfo:) contextInfo:((void *)kQTKitPlayerOpenAsData)];
}

// Create and present a Movie Inspector window
- (IBAction)showInspectorWindow:(id)sender
{
	// Create Inspector window 
	if (!mInspectorController)
	{
		mInspectorController = [[MovieInspectorController alloc] init];
	}
		
	// Show the window
	[mInspectorController  doShowWindow];

	// Display info. in the window for the current movie document
	[mInspectorController updateInspectorWindowForCurrentMovie];	
}


#pragma mark --- methods ---
- (BOOL)createMovieDocumentWithURL:(NSURL *)url asData:(BOOL)asData
{
    NSDocument		*movieDocument = nil;
    NSDocumentController	*documentController;
    BOOL			success = YES;

    // init
    documentController = [NSDocumentController sharedDocumentController];

    // try to create the document from the URL
    if (url)
    {
        if (asData)
            movieDocument = [documentController makeDocumentWithContentsOfURL:url ofType:@"MovieDocumentData" error:nil];
        else
            movieDocument = [documentController makeDocumentWithContentsOfURL:url ofType:@"MovieDocument" error:nil];
    }

    // add the document
    if (movieDocument)
    {
        [documentController addDocument:movieDocument];

        // setup
        [movieDocument makeWindowControllers];
        [movieDocument updateChangeCount:NSChangeCleared];
        [movieDocument showWindows];
    }
    else
    {
        NSRunAlertPanel(@"Invalid movie", @"The url is not a valid movie.", nil, nil, nil);
        success = NO;
    }

    return success;
}

// Getter:
// returns the MovieInspectorController instance
-(MovieInspectorController *)inspectorWindowController
{
    return [[mInspectorController retain] autorelease];
}

// The inspector controller will tell us when it 
// closes it's inspector window so we can release
// our reference to it
-(void)inspectorWindowHasClosed
{

	if (mInspectorController)
	{
		// Free the MovieInspectorController instance
		[mInspectorController release];
		mInspectorController = nil;
	}
}

#pragma mark --- delegates ---

// don't want an untitled document opened upon program launch
// so return NO here
- (BOOL)applicationShouldOpenUntitledFile:(NSApplication *)sender 
{ 
	return NO; 
}

@end
