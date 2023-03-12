/*

File: MyController.m

Author: QuickTime DTS

Change History (most recent first):
        
      <1> 09/28/07 initial release
      
© Copyright 2007 Apple Inc. All rights reserved.

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

@interface MyController (Private)

- (void)openMovie:(NSString *)inFile;

- (void)exportCompleted:(ExportItem *)inExportEntry;
- (void)performExportSelector:(SEL)aSelector withObject:(MovieEntry *)argument;
- (BOOL)checkEntryCompletionState;
- (void)doExportToPod:(ExportItem *)inExportEntry;
- (void)doExportToATV:(ExportItem *)inExportEntry;
- (void)doExportToPhone:(ExportItem *)inExportEntry;
- (void)doExportToPhoneCell:(ExportItem *)inExportEntry;
- (void)openPanelDidEnd:(NSOpenPanel *)sheet returnCode:(int)returnCode contextInfo:(void *)contextInfo;
- (NSTableView *)tableView;
- (NSWindow *)window;

@end

@implementation MyController

#pragma mark ---- initialization ----

- (void)awakeFromNib
{
	// set up some initial UI state
    [self setCanAdd:YES];
    [self setCanRemove:NO];
    [self setCanExport:NO];
    [self setIsExporting:NO];
    
    // add our arrangedObjects observer
    [mMovieArrayController addObserver:self forKeyPath:@"arrangedObjects" options:0 context:NULL];
	
	// open the Shared Folder which is the default output destination
	NSAppleScript *openSharedFolderScript = [[NSAppleScript alloc] initWithSource:@"tell application \"Finder\" \n make new Finder window to folder \"Shared\" of folder \"Users\" of startup disk \n end tell"];
	[openSharedFolderScript executeAndReturnError:nil];
	[openSharedFolderScript release];
}

#pragma mark ---- user interface ----

// observing values from our model object which then control enabling/disabling buttons though bindings
- (void)observeValueForKeyPath:(NSString *)keyPath ofObject:(id)object change:(NSDictionary *)change context:(void *)context
{
    if([keyPath isEqualToString:@"arrangedObjects"]) {
        int count = [[object arrangedObjects] count];
        
        if (count >= 4) {
            [self setCanAdd:NO];
        } else {
            [self setCanAdd:YES];
        }
        
        if (count == 0) {
            [self setCanRemove:NO];
            [self setCanExport:NO];
        } else {
            [self setCanRemove:YES];
            [self setCanExport:YES];
        }
    }
}

- (void)setCanAdd:(BOOL)input
{
	if (NO == input) {
		// disable dragging
		[[self tableView] unregisterDraggedTypes];
	} else {
		// enable dragging
		[[self tableView] registerForDraggedTypes:[NSArray arrayWithObjects: NSFilenamesPboardType, nil]];
	}
	
    canAdd = input;
}

- (void)setCanRemove:(BOOL)input
{
    canRemove = input;
}

- (void)setCanExport:(BOOL)input
{
    canExport = input;
}

- (BOOL)isExporting
{
	return mIsExporting;
}

- (void)setIsExporting:(BOOL)input
{
	if (NO == input && canAdd) {
		// enable draging
		[[self tableView] registerForDraggedTypes:[NSArray arrayWithObjects: NSFilenamesPboardType, nil]];
	} else {
		// disable dragging
		[[self tableView] unregisterDraggedTypes];
	}
	
	mIsExporting = input;
}

#pragma mark ---- add / remove / export ----

// select a file to open
- (IBAction)addMovie:(id)sender
{
#pragma unused(sender)

	[[NSOpenPanel openPanel] beginSheetForDirectory:nil
                             file:nil
                             types:[NSArray arrayWithObjects:@"mov", @"MOV", nil]
                             modalForWindow:[self window]
                             modalDelegate:self
                             didEndSelector:@selector(openPanelDidEnd:returnCode:contextInfo:)
                             contextInfo:nil];
}

- (IBAction)removeMovie:(id)sender
{
#pragma unused(sender)

    [mMovieArrayController setSelectionIndex:[[mMovieArrayController arrangedObjects] count] - 1];
    [mMovieArrayController remove:self];
}

// called when the extraction/conversion button is pressed
- (IBAction)doExport:(id)sender
{
#pragma unused(sender)

    NSArray *entries = [mMovieArrayController arrangedObjects];
    NSEnumerator *enumerator = [entries objectEnumerator];
    MovieEntry *entry;
    
    [self setCanExport:NO];
    [self setIsExporting:YES];
 
    while ((entry = [enumerator nextObject])) {
        [self performExportSelector:@selector(doExportToPod:) withObject:entry];
        [self performExportSelector:@selector(doExportToATV:) withObject:entry];
        [self performExportSelector:@selector(doExportToPhone:) withObject:entry];
        [self performExportSelector:@selector(doExportToPhoneCell:) withObject:entry];
    }
}

#pragma mark ---- *private methods* ----
#pragma mark ----movie entry  ----

// open the QTMovie and set it in the view replacing the movie that was there
// the MovieWriter object retains the movie during the operation
- (void)openMovie:(NSString *)inFile
{
    NSError *error = nil;
    MovieEntry *entry = nil;
    
    NSDictionary *attrs = [NSDictionary dictionaryWithObjectsAndKeys:(id)inFile, QTMovieFileNameAttribute,
                                            [NSNumber numberWithBool:NO],        QTMovieOpenAsyncOKAttribute, nil];
    
    QTMovie *aMovie = [QTMovie movieWithAttributes:attrs error:&error];
    
    if (aMovie && nil == error) {
	
		NSArray *entries = [mMovieArrayController arrangedObjects];
		NSEnumerator *enumerator = [entries objectEnumerator];
		for (UInt8 index = 0; index < [entries count]; index++) {
			if ([[entry = [enumerator nextObject] fileName] isEqual:[inFile lastPathComponent]]) {
				NSLog(@"%@ has already been added to the export list.\n", [inFile lastPathComponent]);
				NSBeep();
				return;
			}
		}

        // grab a frame of the movie to display
		QTTime  aTime = [aMovie duration];
        aTime.timeValue /= 2;
		// don't use frameImageAtTime: it's crufty, use frameImageAtTime:withAttributes:
		// NSImage *aMovieFrame = [aMovie frameImageAtTime:aTime]; <- if you're doing this in your code, upgrade to the new API
		NSDictionary *imageAttrs = [NSDictionary dictionaryWithObjectsAndKeys:
											 QTMovieFrameImageTypeNSImage, QTMovieFrameImageType,
											 [NSNumber numberWithBool:YES], QTMovieFrameImageHighQuality, nil];
											 
		NSImage *aMovieFrame = [aMovie frameImageAtTime:aTime withAttributes:imageAttrs error:nil];

        entry = [[MovieEntry alloc] init];
        if (nil != entry) {
            [entry setImage:aMovieFrame];
            [entry setFullPath:inFile];
            
            [mMovieArrayController addObject:entry];
            [entry release]; // adding it will retain it
			[[self tableView] scrollRowToVisible:[entries count]-1];
        }
   } else {
    	if (error) {
 			NSAlert *alert = [NSAlert alertWithError:error];
            [alert runModal];
        }
    }
}

#pragma mark ---- getters  ----

- (NSTableView *)tableView
{
	return mTableView;
}

- (NSWindow *)window
{
    return mWindow;
}

#pragma mark ---- tableview overrides ----

static BOOL IsValidFileForDrop(NSString *inFilePath)
{
	BOOL isValidFile = NO;
	
	NSDictionary *fileAttribs = [[NSFileManager defaultManager] fileAttributesAtPath:inFilePath traverseLink:YES];
	if (fileAttribs) {
		if ([NSFileTypeDirectory isNotEqualTo:[fileAttribs objectForKey:NSFileType]]) {
			if ([[[inFilePath pathExtension] lowercaseString] isEqualToString:@"mov"]) {
				isValidFile = YES;
			}
		}
	}

	return isValidFile;
}

// don't allow selections
- (BOOL)tableView:(NSTableView *)tableView shouldSelectRow:(int)row
{
    return NO;
}

// don't care about modifers
- (BOOL)ignoreModifierKeysWhileDragging
{
	return YES;
}

// always add at the bottom
- (NSDragOperation)tableView:(NSTableView *)tableView validateDrop:(id <NSDraggingInfo>)info proposedRow:(int)row proposedDropOperation:(NSTableViewDropOperation)operation
{
	[tableView setDropRow:([[mMovieArrayController arrangedObjects] count]) dropOperation:NSTableViewDropAbove];

	return NSDragOperationCopy;
}

// is the drop something we want
- (BOOL)tableView:(NSTableView *)tableView acceptDrop:(id <NSDraggingInfo>)info row:(int)row dropOperation:(NSTableViewDropOperation)operation
{
	NSPasteboard *pasteBoard = [info draggingPasteboard];
	NSArray	*array = NULL;
	
	// file drop from another app (such as Finder)
	if ((array = [pasteBoard propertyListForType:NSFilenamesPboardType]) != NULL) {
		NSArray *fileNames = NULL;
		// We don't seem to get files from the Finder in any particular order,
		// which means that the same drop performed multiple times has essentially
		// (from the user's perspective) random ordering.  That's not so great.
		// To make it deterministic, sort the incoming list before we add them.
		// We only grab the first entry in the list here.
		if ((fileNames = [array sortedArrayUsingSelector:@selector(localizedCaseInsensitiveCompare:)]) != NULL) {
			// only allow .mov files, not packages/directories or other files
			if (IsValidFileForDrop([fileNames objectAtIndex:0])) {
				[self openMovie:[fileNames objectAtIndex:0]];
				return YES;
			}
		}
	}
	
	return NO;
}

#pragma mark ---- open panel delegate ----

// movie opening panel
- (void)openPanelDidEnd:(NSOpenPanel *)sheet returnCode:(int)returnCode contextInfo:(void *)contextInfo
{
#pragma unused(contextInfo)

    if (NSOKButton == returnCode) {
        NSString *theFilename = [[sheet filenames] objectAtIndex:0];
        [sheet close];
    
        [self openMovie:theFilename];
    }
}
#pragma mark ---- window delegates ----

// don't close the window right in the middle of writing the movie file
- (BOOL)windowShouldClose:(id)sender
{
#pragma unused(sender)

    if ([self isExporting]) return NO;
    
    return YES;
}

#pragma mark ---- application delegates ----

// split when window is closed
- (BOOL)applicationShouldTerminateAfterLastWindowClosed:(NSApplication *)sender
{
#pragma unused(sender)

    return YES;
}

#pragma mark ---- NSSound delegate ----

- (void)sound:(NSSound *)sound didFinishPlaying:(BOOL)aBool
{
	if (TRUE == aBool) [sound release];
}

// handle the situation when a user may quit the application right in the middle
// of an export operation -- we wait for completion then quit
- (NSApplicationTerminateReply)applicationShouldTerminate:(NSApplication *)sender;
{
#pragma unused(sender)

    if ([self isExporting]) {
		mAppIsQuitting = YES;
        return NSTerminateLater;
    } else {
        return NSTerminateNow;
    }
}

#pragma mark ---- export worker methods ----

- (void)performExportSelector:(SEL)aSelector withObject:(MovieEntry *)inEntry
{
    NSError *error = nil;
    
    // create a brand new QTMovie object specifically for export, these individually need to
    // be detached from the main thread and attached to the background thread -- QTMovie objects
    // can ONLY be created on the main thread
    QTMovie *theMovieToExport = [QTMovie movieWithFile:[inEntry fullPath] error:&error];
    
    if (nil == error) {
		BOOL canDetach = [theMovieToExport detachFromCurrentThread];
        
        if (canDetach == YES) {
            // create a new export item
    		ExportItem *item = [ExportItem new];
			
            // retain movie as we're gonna be using it
            [theMovieToExport retain];
			
			// set the movie aperture mode to clean aperture
			if (NO == [theMovieToExport attributeForKey:QTMovieHasApertureModeDimensionsAttribute]) {
				[theMovieToExport generateApertureModeDimensions];
			}
			
			[theMovieToExport setAttribute:QTMovieApertureModeClean forKey:QTMovieApertureModeAttribute];
            
            // reset the progress state for the entry, this will also flip the mDone
            // member of the entry to NO which we check during our completion processing
    		[inEntry setProgressState:0.0];
    
            // set the intem params
    		[item setMovie:theMovieToExport];
    		[item setEntry:inEntry];
        
            // perform the export on a background thread
            [NSThread detachNewThreadSelector:aSelector
                                     toTarget:self
								   withObject:item];
        } else {
        	[inEntry setDone:YES];
            [self exportCompleted:nil];
            
            NSLog(@"%@ could not be detached from the main thread. Error: %d.\n", [inEntry fileName], componentNotThreadSafeErr);
			NSSound *oops = [[NSSound soundNamed:@"Basso"] retain];
			[oops setDelegate:self];
			[oops play];
        }
	} else {
    	[inEntry setDone:YES];
        [self exportCompleted:nil];
        
		NSAlert *alert = [NSAlert alertWithError:error];
        [alert runModal];
    }
}

- (BOOL)checkEntryCompletionState
{
    NSArray *entries = [mMovieArrayController arrangedObjects];
    NSEnumerator *enumerator = [entries objectEnumerator];
    MovieEntry *entry;
    
    while ((entry = [enumerator nextObject])) {
    	if ([entry done] == NO) return NO;
    }
	
	if (YES == mAppIsQuitting) {
        [[NSApplication sharedApplication] replyToApplicationShouldTerminate:YES];
    }
	
    return YES;
    
}

- (void)exportCompleted:(ExportItem *)inItem
{
	if (inItem != nil) {
        QTMovie *theExportedMovie = [inItem movie];
        MovieEntry *theMovieEntry = [inItem entry];
        
        [theExportedMovie attachToCurrentThread];
        [theExportedMovie release];
        
        [theMovieEntry setProgressState:[theMovieEntry progressState] + 1.0];
        
        if ([inItem didSucceed] == NO) {
            NSLog(@"%@ had a problem during export!\n", [theMovieEntry fileName]);
			NSSound *oops = [[NSSound soundNamed:@"Basso"] retain];
			[oops setDelegate:self];
			[oops play];
        } else {
			NSSound *ping = [[NSSound soundNamed:@"Ping"] retain];
			[ping setDelegate:self];
			[ping play];
		}
        
        if ([theMovieEntry done] == YES) {
            if ([self checkEntryCompletionState] == YES) {
                [self setCanExport:YES];
                [self setIsExporting:NO];
            }
        }
        
        [inItem release];
    } else {
    	if ([self checkEntryCompletionState] == YES) {
        	[self setCanExport:YES];
            [self setIsExporting:NO];
        }
    }
}

- (void)doExportToPod:(ExportItem *)inItem
{
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
    
    NSString *fileName = [[[inItem entry] fileName] stringByDeletingPathExtension];
	NSString *newFileName = [NSString stringWithFormat:@"/Users/Shared/%@_iPod.m4v", fileName];
    
    QTMovie *theMovie = [inItem movie];
    
    [QTMovie enterQTKitOnThread];
    [theMovie attachToCurrentThread];

    // do export
    NSDictionary *dictionary = [NSDictionary dictionaryWithObjectsAndKeys:
                                                 [NSNumber numberWithBool:YES], QTMovieExport,
                                                 [NSNumber numberWithLong:'M4V '], QTMovieExportType, nil];

	BOOL didSucceed = [theMovie writeToFile:newFileName withAttributes:dictionary];
	[inItem setDidSucceed:didSucceed];
    
    [theMovie detachFromCurrentThread];
    [QTMovie exitQTKitOnThread];
	
    [self performSelectorOnMainThread:@selector(exportCompleted:) withObject:inItem waitUntilDone:NO];

    [pool release];
}

- (void)doExportToATV:(ExportItem *)inItem
{
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
	
    NSString *fileName = [[[inItem entry] fileName] stringByDeletingPathExtension];
	NSString *newFileName = [NSString stringWithFormat:@"/Users/Shared/%@_ATV.m4v", fileName];
    
    QTMovie *theMovie = [inItem movie];
    
    [QTMovie enterQTKitOnThread];
    [theMovie attachToCurrentThread];

    // do export
    NSDictionary *dictionary = [NSDictionary dictionaryWithObjectsAndKeys:
                                                 [NSNumber numberWithBool:YES], QTMovieExport,
                                                 [NSNumber numberWithLong:'M4VH'], QTMovieExportType, nil];

	BOOL didSucceed = [theMovie writeToFile:newFileName withAttributes:dictionary];
	[inItem setDidSucceed:didSucceed];
    
    [theMovie detachFromCurrentThread];
    [QTMovie exitQTKitOnThread];
	
    [self performSelectorOnMainThread:@selector(exportCompleted:) withObject:inItem waitUntilDone:NO];

    [pool release];
}

- (void)doExportToPhone:(ExportItem *)inItem
{
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
	
    NSString *fileName = [[[inItem entry] fileName] stringByDeletingPathExtension];
	NSString *newFileName = [NSString stringWithFormat:@"/Users/Shared/%@_iPhone.m4v", fileName];
    
    QTMovie *theMovie = [inItem movie];
    
    [QTMovie enterQTKitOnThread];
    [theMovie attachToCurrentThread];

    // do export
    NSDictionary *dictionary = [NSDictionary dictionaryWithObjectsAndKeys:
                                                 [NSNumber numberWithBool:YES], QTMovieExport,
                                                 [NSNumber numberWithLong:'M4VP'], QTMovieExportType, nil];

	BOOL didSucceed = [theMovie writeToFile:newFileName withAttributes:dictionary];
	[inItem setDidSucceed:didSucceed];
    
    [theMovie detachFromCurrentThread];
    [QTMovie exitQTKitOnThread];
	
    [self performSelectorOnMainThread:@selector(exportCompleted:) withObject:inItem waitUntilDone:NO];

    [pool release];
}

- (void)doExportToPhoneCell:(ExportItem *)inItem
{
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
	
    NSString *fileName = [[[inItem entry] fileName] stringByDeletingPathExtension];
	NSString *newFileName = [NSString stringWithFormat:@"/Users/Shared/%@_iPhone_Cell.3gp", fileName];
	
    QTMovie *theMovie = [inItem movie];
    
    [QTMovie enterQTKitOnThread];
    [theMovie attachToCurrentThread];

    // do export
    NSDictionary *dictionary = [NSDictionary dictionaryWithObjectsAndKeys:
                                                 [NSNumber numberWithBool:YES], QTMovieExport,
                                                 [NSNumber numberWithLong:'iphE'], QTMovieExportType, nil];

	BOOL didSucceed = [theMovie writeToFile:newFileName withAttributes:dictionary];
	[inItem setDidSucceed:didSucceed];
    
    [theMovie detachFromCurrentThread];
    [QTMovie exitQTKitOnThread];
	
    [self performSelectorOnMainThread:@selector(exportCompleted:) withObject:inItem waitUntilDone:NO];

    [pool release];
}

@end