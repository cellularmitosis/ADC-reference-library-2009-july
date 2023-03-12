/*

File: MyController.m

Author: QuickTime DTS

Change History (most recent first):
        
        <2> 03/24/06 ensure the movie is fully loaded before extraction can start Q&A 1469
        <1> 11/10/05 initial release

© Copyright 2005 - 2006 Apple Computer, Inc. All rights reserved.

IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc. ("Apple") in
consideration of your agreement to the following terms, and your use, installation,
modification or redistribution of this Apple software constitutes acceptance of these
terms.  If you do not agree with these terms, please do not use, install, modify or
redistribute this Apple software.

In consideration of your agreement to abide by the following terms, and subject to these
terms, Apple grants you a personal, non-exclusive license, under Apple's copyrights in
this original Apple software (the "Apple Software"), to use, reproduce, modify and
redistribute the Apple Software, with or without modifications, in source and/or binary
forms; provided that if you redistribute the Apple Software in its entirety and without
modifications, you must retain this notice and the following text and disclaimers in all
such redistributions of the Apple Software. Neither the name, trademarks, service marks
or logos of Apple Computer, Inc. may be used to endorse or promote products derived from
the Apple Software without specific prior written permission from Apple.  Except as
expressly stated in this notice, no other rights or licenses, express or implied, are
granted by Apple herein, including but not limited to any patent rights that may be
infringed by your derivative works or by other works in which the Apple Software may be
incorporated.

The Apple Software is provided by Apple on an "AS IS" basis.  APPLE MAKES NO WARRANTIES,
EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED WARRANTIES OF
NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE, REGARDING THE
APPLE SOFTWARE OR ITS USE AND OPERATION ALONE OR IN COMBINATION WITH YOUR PRODUCTS.

IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) ARISING IN ANY WAY OUT OF THE
USE, REPRODUCTION, MODIFICATION AND/OR DISTRIBUTION OF THE APPLE SOFTWARE, HOWEVER
CAUSED AND WHETHER UNDER THEORY OF CONTRACT, TORT (INCLUDING NEGLIGENCE), STRICT
LIABILITY OR OTHERWISE, EVEN IF APPLE HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/

#import "MyController.h"
#import "AIFFWriter.h"

@implementation MyController

- (void)awakeFromNib
{
    NSNotificationCenter *nc = [NSNotificationCenter defaultCenter];
     
	// create the extraction object and set the
    // delegate so it will call the progress callback
	mAIFFWriter = [[AIFFWriter alloc] init];
	[mAIFFWriter setDelegate:self];
    
    [mExportButton setEnabled:FALSE];
    
    [nc addObserver:self selector:@selector(movieLoadStateDidChange:) name:@"QTMovieLoadStateDidChangeNotification" object:nil];
}

#pragma mark ---- panel callbacks ----

// movie opening panel
- (void)openPanelDidEnd:(NSOpenPanel *)sheet returnCode:(int)returnCode contextInfo:(void *)contextInfo
{
    if (NSOKButton == returnCode) {
        NSString *theFilename = [[sheet filenames] objectAtIndex:0];
        [sheet close];
    
        [self openMovie:theFilename];
    }
}

// movie extraction panel
- (void)savePanelDidEnd:(NSOpenPanel*)sheet returnCode:(int)returnCode contextInfo:(void*)contextInfo
{    
    if (NSOKButton == returnCode) {
    	[sheet close];

        [mAIFFWriter exportFromMovie:mMovie toFile:[sheet filename]];
    }
}

// error alert sheet
- (void)alertDidEnd:(NSAlert *)alert returnCode:(int)returnCode contextInfo:(void *)contextInfo
{
    // if applicationShouldTerminate was called during export we don't quit - we
    // set the isQuitting flag then wait for the extaction to finish
    if (YES == mAppIsQuitting) {
        [[NSApplication sharedApplication] replyToApplicationShouldTerminate:YES];
    }
}

#pragma mark ---- open movie/extract ----

// select a file to open
- (IBAction)doOpen:(id)sender
{
	[[NSOpenPanel openPanel] beginSheetForDirectory:nil
                             file:nil
                             types:nil
                             modalForWindow:[self window]
                             modalDelegate:self
                             didEndSelector:@selector(openPanelDidEnd:returnCode:contextInfo:)
                             contextInfo:nil];
}

// called when the extraction button is pressed
- (IBAction)doExportToAIFF:(id)sender
{
    if (nil == mMovie) return;
    
    NSSavePanel *savePanel = [NSSavePanel savePanel];
	
    [savePanel setRequiredFileType:@"aif"];
	
    // open a save panel to get a target file specification
	[savePanel beginSheetForDirectory:nil
                                file:@"ExtractedAudio"
                                modalForWindow:[self window]
                                modalDelegate:self
                                didEndSelector:@selector(savePanelDidEnd:returnCode:contextInfo:)
                                contextInfo:nil];
}

// open the QTMovie and set it in the view replacing the movie that was there
// the AIFFWriter object retains the movie during export
- (void)openMovie:(NSString *)inFile
{
    NSAlert *alert = nil;
    NSError *error = nil;
    
	if (mMovie != nil) {
        [mMovieView pause:self];
		[mMovieView setMovie:nil];
	}
    
    if (![mAIFFWriter isExporting] ) {
        [mExportButton setTitle:@"Loading Movie..."];
    }
    
    mMovie = [QTMovie movieWithFile:inFile error:&error];
    
    if (nil == error) {

        [[self window] setTitle:[mMovie attributeForKey:QTMovieDisplayNameAttribute]];
    
        [mMovieView setMovie:mMovie];
        [mMovieView setNeedsDisplay:TRUE];
        
        // if a Movie loads really fast QTKit will not send a LoadStateDidChange notification so we do it here
        [[NSNotificationCenter defaultCenter] postNotification:[NSNotification notificationWithName:@"QTMovieLoadStateDidChangeNotification" object:nil]];
    } else {
    	if (error) {
 			alert = [NSAlert alertWithError:error];
        }
        
		[alert runModal];
    }

}

#pragma mark ---- getters ----

- (NSWindow *)window
{
    return mWindow;
}

#pragma mark ---- progress callback delegate ----

// progress callback funtion used to drive the progress indicator, UI and allows
// the client app to get completion status errors from the AIFFWriter object
- (BOOL)shouldContinueOperationWithProgressInfo:(AIFFWriterProgressInfo *)inProgressInfo
{
    AIFFWriterExportOperationPhase phase = [inProgressInfo phase];
    
	switch (phase) {
    case AIFFWriterExportBegin:
    	[mProgressBar startAnimation:self];
        [mExportButton setTitle:@"Extracting..."];
        [mExportButton setEnabled:FALSE];
        
        break;
    case AIFFWriterExportPercent:
    	if ([mProgressBar isIndeterminate]) {
        	[mProgressBar stopAnimation:self];
        	[mProgressBar setIndeterminate:FALSE];
        }
        
    	[mProgressBar setDoubleValue:[[inProgressInfo progressValue] doubleValue]];
        
    	break;
    case AIFFWriterExportEnd:
    {
        NSError *status = [inProgressInfo exportStatus];
        long movieLoadState = [[mMovie attributeForKey:QTMovieLoadStateAttribute] longValue];
        
    	[mProgressBar setIndeterminate:TRUE];
        [mProgressBar setDoubleValue:0];
        [mProgressBar stopAnimation:self];
        
        if (kMovieLoadStateComplete == movieLoadState) {
            [mExportButton setTitle:@"Extract"];
        }
        
        if (nil == status) {
        
            // if applicationShouldTerminate was called during export we don't quit - we
            // set the isQuitting flag then wait for the extaction to finish
            if (YES == mAppIsQuitting) {
                [[NSApplication sharedApplication] replyToApplicationShouldTerminate:YES];
            }
        } else {
        
            NSAlert *alert = [NSAlert alertWithMessageText:@"AIFFWriter Error!"
                                      defaultButton:@"OK"
                                      alternateButton:nil
                                      otherButton:nil
                                      informativeTextWithFormat:@"Extraction failed with %d error.", [status code]];
                                      
        	[alert beginSheetModalForWindow:[self window]
                   modalDelegate:self
                   didEndSelector:@selector(alertDidEnd:returnCode:contextInfo:)
                   contextInfo:NULL];
        }
        
        if (kMovieLoadStateComplete == movieLoadState && (TRUE == [[mMovie attributeForKey:QTMovieHasAudioAttribute] boolValue])) {
            [mExportButton setEnabled:TRUE];
        }
    }
        break;
    default:
    	break;
    }
    
    return YES;	// return NO if you want to cancel the export op.
}

#pragma mark ---- notifications ----

// movieLoadStateDidChange is called for QTMovieLoadStateDidChangeNotification notifications.
- (void)movieLoadStateDidChange:(NSNotification *)notification
{
    if (kMovieLoadStateComplete == [[mMovie attributeForKey:QTMovieLoadStateAttribute] longValue]) {
        if (TRUE == [[mMovie attributeForKey:QTMovieHasAudioAttribute] boolValue]) {
        	if (![mAIFFWriter isExporting]) {
                [mExportButton setTitle:@"Extract"];
            	[mExportButton setEnabled:TRUE];
            }
        } else {
            NSAlert *alert = [NSAlert alertWithMessageText:@"No Sound Track!"
                                      defaultButton:@"OK"
                                      alternateButton:nil
                                      otherButton:nil
                                      informativeTextWithFormat:@"Open some media that contains audio if you're planing to perform audio extraction."];
            
            [mExportButton setTitle:@"Extract"];
            [mExportButton setEnabled:FALSE];
            [alert runModal];
        }
    } else {
        [mExportButton setEnabled:FALSE];
    }
}

#pragma mark ---- window delegates ----

// don't close the window right in the middle of writing the AIFF file
- (BOOL)windowShouldClose:(id)sender
{
    if ([mAIFFWriter isExporting]) return NO;
    
    return YES;
}

#pragma mark ---- application delegates ----

// split when window is closed
- (BOOL)applicationShouldTerminateAfterLastWindowClosed:(NSApplication *)sender
{
    return YES;
}

// handle the situation when a user may quit the application right in the middle
// of an extraction operation -- we wait for completion then quit
- (NSApplicationTerminateReply)applicationShouldTerminate:(NSApplication *)sender;
{
    if ([mAIFFWriter isExporting]) {
        mAppIsQuitting = YES;
        return NSTerminateLater;
    } else {
        return NSTerminateNow;
    }
}

@end