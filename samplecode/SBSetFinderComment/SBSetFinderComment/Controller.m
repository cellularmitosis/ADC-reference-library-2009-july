/*

File: Controller.m

Abstract: Main Controller file for the SBSetFinderComment sample.
All of the interesting functionality for this sample is in this file.

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


#import "Controller.h"
#import "Finder.h"

@implementation Controller


@synthesize selectedFinderItem;



	/* -changeFinderComment:forFileURL: returns YES if it is able to change
	the finder comment (aka Spotlight comment) for an item referenced by the
	file url.  Returns NO if an error occurs during processing.  */ 
- (BOOL) changeFinderComment:(NSString*) comment forFileURL:(NSURL*) theFileURL {
	BOOL result;

	@try {
			/* retrieve the Finder application Scripting Bridge object. */
		FinderApplication* finder = [SBApplication applicationWithBundleIdentifier:@"com.apple.finder"];
				
			/* retrieve a reference to our finder item asking for it by location */
		FinderItem * theItem = [[finder items] objectAtLocation: theFileURL];

			/* attempt to set the comment for the Finder item.  */
		theItem.comment = comment;
		
			/* successful result */
		result = YES;
	}
	@catch(NSException *e) {
		result = NO;
	}

		/* return YES on success */
	return result;
}



	/* -finderCommentForFileURL: returns an NSString containing the referenced
	item's finder comment (aka Spotlight comment) an item referenced by the
	file url.  Returns nil if an error occurs during processing.  */ 
- (NSString*) finderCommentForFileURL:(NSURL*) theFileURL {
	NSString* result;

	@try {
			/* retrieve the Finder application Scripting Bridge object. */
		FinderApplication* finder = [SBApplication applicationWithBundleIdentifier:@"com.apple.finder"];
		
			/* retrieve a reference to our finder item asking for it by location */
		FinderItem * theItem = [[finder items] objectAtLocation: theFileURL];
		
			/* set the result.  */
		result = theItem.comment;
	}
	@catch(NSException *e) {
		result = nil;
	}
	
		/* return the comment (or nil on error). */
	return result;
}




	/* -finderRevealFileURL: returns true if it is able to reveal the indicated item
	in a Finder window.  The Finder will be switched into the forground during the
	processing of this call.  Returns NO if an error occurs during processing.  */ 
- (BOOL) finderRevealFileURL:(NSURL*) theFileURL {
	BOOL result;

	@try {
			/* retrieve the Finder application Scripting Bridge object. */
		FinderApplication* finder = [SBApplication applicationWithBundleIdentifier:@"com.apple.finder"];
		
			/* retrieve a reference to our finder item asking for it by location */
		FinderItem * theItem = [[finder items] objectAtLocation: theFileURL];
		
			/* display the item  */
		[theItem reveal];
		
			/* activate the Finder application  */
		[finder activate];

			/* successful result  */
		result = YES;
	}
	@catch(NSException *e) {
		result = NO;
	}
	
		/* return YES on success */
	return result;
}




- (void)awakeFromNib {
		
		/* start with no file selection */
	self.selectedFinderItem = nil;

}



	/* we set ourself to the NSApplication's delegate in the .nib file.  Adding
	this method is a minor convenience so the sample will quit when the
	window is closed. */
- (BOOL)applicationShouldTerminateAfterLastWindowClosed:(NSApplication *)sender {
	return YES;
}


	/* Convenience for displaying error messages. */
- (void) showErrorMessage:(NSString*) message withTitle: (NSString*) title {
	[[NSAlert alertWithMessageText:title defaultButton:@"OK" alternateButton:nil otherButton:nil
			informativeTextWithFormat: @"%@", message] runModal];
}


	/* IB action called when the 'Reveal in Finder' button is clicked. We use bindings
	to enable the button whenever self.selectedFinderItem is not nil, so there's no need
	to check if a file is selected here.  We just go ahead and process the command.  */
- (IBAction)revealInFinder:(id)sender {

	if ( ! [self finderRevealFileURL: self.selectedFinderItem] ) {
	
		[self showErrorMessage: @"Unable to reveal item in Finder.  Please re-select the file and try again."
			withTitle: @"Error revealing item"];			
	}
}



	/* IB action called when the 'Set Comment' button is clicked. We use bindings
	to enable the button whenever self.selectedFinderItem is not nil, so there's no need
	to check if a file is selected here.  We just go ahead and process the command.  */
- (IBAction)changeComment:(id)sender {
		
		/* retrieve the comment text from the window */
	NSString *commentText = [[commentField textStorage] string];

		/* Verify that the comment is of a suitable length.
		Radar rdar://problem/4857955 states that Finder comments are limited
		to 750 Unicode characters. This is the current recommendation at
		the time of this writing.  */
	if ( [commentText length] > 750 ) {
	
		[self showErrorMessage:
			[NSString stringWithFormat:
				@"Comments are limited to 750 characters.  The comment you entered is %d characters long.",
				[commentText length]]
			withTitle: @"Comment too long"];

	} else {
	
			/* Attempt to change the finder comment. */
		if ( ! [self changeFinderComment:commentText forFileURL: self.selectedFinderItem] ) {
			
			[self showErrorMessage: @"Unable to set the finder comment.  Please re-select the file and try again."
				withTitle: @"Error setting comment"];			
		}
	}
}



	/* applicationDidBecomeActive: is an NSApplication delegate method
	that is called whenever our application is switched into the forground.
	We take this opportunity to refresh the comment field in case the comment
	was changed while our process was in the background.  */
- (void) applicationDidBecomeActive: (NSNotification *) notification {

		/* if we have selected a file in the finder */
	if ( self.selectedFinderItem ) {
	
		NSString *theComment = [self finderCommentForFileURL: self.selectedFinderItem ];
		if ( theComment ) {
		
				/* set the path in the display */
			[fileNameField setStringValue: [self.selectedFinderItem path]];
			
				/* retrieve the finder comment */
			NSUInteger p = [[[commentField textStorage] string] length];
			[commentField setSelectedRange:NSMakeRange(0, p)];
			[commentField insertText: theComment];
			
		} else {
				
			[self showErrorMessage: @"Unable to update the finder comment for the selected item."
				withTitle: @"Error getting comment"];
		}
	}
}




	/* IB action called when the 'Select...' button beside the file/directory field
	is clicked.  */
- (IBAction)selectFileForComment:(id)sender {
	NSOpenPanel *theOpenPanel;
	NSInteger opResult;
	
		/* create an open panel */
	theOpenPanel= [NSOpenPanel openPanel];
    [theOpenPanel setDelegate:self];
	
		/* set the prompt and title */
    [theOpenPanel setMessage:@"Select a file or folder for comment editing."];
    [theOpenPanel setTitle:@"Choose File or Folder"];

		/* directories okay, only one at a time */
    [theOpenPanel setCanChooseDirectories:YES];
    [theOpenPanel setAllowsMultipleSelection:NO];
	
		/* run the panel */
	opResult = [theOpenPanel runModalForDirectory:NSHomeDirectory() file:nil types:nil];
    if ( NSOKButton == opResult ) {
		
			/* get and save the path */
		self.selectedFinderItem = [[theOpenPanel URLs] objectAtIndex:0];
		
			/* attempt to retrieve the comment */
		NSString *theComment = [self finderCommentForFileURL: self.selectedFinderItem ];
		if ( theComment ) {
		
				/* set the path in the display */
			[fileNameField setStringValue: [self.selectedFinderItem path]];
			
				/* retrieve the finder comment */
			NSUInteger p = [[[commentField textStorage] string] length];
			[commentField setSelectedRange:NSMakeRange(0, p)];
			[commentField insertText: theComment];
			
		} else {
			
			[self showErrorMessage: @"Unable to retrieve the finder comment for that item."
				withTitle: @"Error getting comment"];
		}
    }
}


@end
