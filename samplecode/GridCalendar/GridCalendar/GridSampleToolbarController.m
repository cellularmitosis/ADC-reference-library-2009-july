/*

File: GridSampleToolbarController.m

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

Copyright 2005 Apple Computer, Inc., All Rights Reserved

*/ 

#import "GridSampleToolbarController.h"
#import "GridSampleMainWindowController.h"

NSString * const GridSampleToolbarNewJobItemIdentifier = @"GridSampleToolbarNewJobItemIdentifier";
NSString * const GridSampleToolbarNewJobItemLabel = @"New Job";
NSString * const GridSampleToolbarNewJobItemImageName = @"new";

NSString * const GridSampleToolbarCloneJobItemIdentifier = @"GridSampleToolbarCloneJobItemIdentifier";
NSString * const GridSampleToolbarCloneJobItemLabel = @"Clone Job";
NSString * const GridSampleToolbarCloneJobItemImageName = @"clone";

NSString * const GridSampleToolbarShowInfoItemIdentifier = @"GridSampleToolbarShowInfoItemIdentifier";
NSString * const GridSampleToolbarShowInfoItemLabel = @"Show Info";
NSString * const GridSampleToolbarShowInfoItemImageName = @"info";

NSString * const GridSampleToolbarShowResultsItemIdentifier = @"GridSampleToolbarShowResultsItemIdentifier";
NSString * const GridSampleToolbarShowResultsItemLabel = @"Show Results";
NSString * const GridSampleToolbarShowResultsItemImageName = @"addFile";

NSString * const GridSampleToolbarCancelJobItemIdentifier = @"GridSampleToolbarCancelJobItemIdentifier";
NSString * const GridSampleToolbarCancelJobItemLabel = @"Cancel Job";
NSString * const GridSampleToolbarCancelJobItemImageName = @"stop";

NSString * const GridSampleToolbarDeleteJobItemIdentifier = @"GridSampleToolbarDeleteJobItemIdentifier";
NSString * const GridSampleToolbarDeleteJobItemLabel = @"Delete Job";
NSString * const GridSampleToolbarDeleteJobItemImageName = @"delete";

@implementation GridSampleToolbarController

- (id)init;
{
	self = [super init];
	
	if (self != nil) {
	
		_toolbar = [(NSToolbar *)[NSToolbar alloc] initWithIdentifier:@"com.apple.xgrid.calendar.toolbar"];
    
		[_toolbar setAllowsUserCustomization:YES];
		[_toolbar setAutosavesConfiguration:YES];
		[_toolbar setDelegate:self];
	}
	
	return self;
}

- (void)dealloc;
{
	[_toolbar release];
	
	[super dealloc];
}

- (NSToolbar *)toolbar;
{
	return _toolbar;
}

#pragma mark *** NSToolbar delegate methods ***

- (NSArray *)toolbarDefaultItemIdentifiers:(NSToolbar *)toolbar {
    return [NSArray arrayWithObjects:
		NSToolbarFlexibleSpaceItemIdentifier,
        GridSampleToolbarNewJobItemIdentifier,
        GridSampleToolbarCloneJobItemIdentifier,
        GridSampleToolbarShowInfoItemIdentifier,
        GridSampleToolbarShowResultsItemIdentifier,
        GridSampleToolbarCancelJobItemIdentifier,
        GridSampleToolbarDeleteJobItemIdentifier,
		NSToolbarFlexibleSpaceItemIdentifier,
        nil];
}

- (NSArray *)toolbarAllowedItemIdentifiers:(NSToolbar *)toolbar {
    return [NSArray arrayWithObjects:
        GridSampleToolbarNewJobItemIdentifier,
        GridSampleToolbarCloneJobItemIdentifier,
        GridSampleToolbarShowInfoItemIdentifier,
        GridSampleToolbarShowResultsItemIdentifier,
        GridSampleToolbarCancelJobItemIdentifier,
        GridSampleToolbarDeleteJobItemIdentifier,
		NSToolbarSeparatorItemIdentifier,
		NSToolbarSpaceItemIdentifier,
		NSToolbarFlexibleSpaceItemIdentifier,
        NSToolbarCustomizeToolbarItemIdentifier,
        nil];
}

- (NSToolbarItem*)toolbar:(NSToolbar*)toolbar itemForItemIdentifier:(NSString *)itemIdentifier willBeInsertedIntoToolbar:(BOOL)willBeInsertedIntoToolbar;
{
	NSToolbarItem *item = nil;
	
    if ([itemIdentifier isEqualToString:GridSampleToolbarNewJobItemIdentifier]) {
	
        item = [[NSToolbarItem alloc] initWithItemIdentifier:itemIdentifier];
        [item setPaletteLabel:GridSampleToolbarNewJobItemLabel];
        [item setLabel:GridSampleToolbarNewJobItemLabel];
        [item setImage:[NSImage imageNamed:GridSampleToolbarNewJobItemImageName]];
        [item setAction:@selector(newJob:)];
        [item setToolTip:nil];
    }
	else if ([itemIdentifier isEqualToString:GridSampleToolbarCloneJobItemIdentifier]) {
	
        item = [[NSToolbarItem alloc] initWithItemIdentifier:itemIdentifier];
        [item setPaletteLabel:GridSampleToolbarCloneJobItemLabel];
        [item setLabel:GridSampleToolbarCloneJobItemLabel];
        [item setImage:[NSImage imageNamed:GridSampleToolbarCloneJobItemImageName]];
        [item setAction:@selector(cloneJob:)];
        [item setToolTip:nil];
    }
	else if ([itemIdentifier isEqualToString:GridSampleToolbarShowInfoItemIdentifier]) {
	
        item = [[NSToolbarItem alloc] initWithItemIdentifier:itemIdentifier];
        [item setPaletteLabel:GridSampleToolbarShowInfoItemLabel];
        [item setLabel:GridSampleToolbarShowInfoItemLabel];
        [item setImage:[NSImage imageNamed:GridSampleToolbarShowInfoItemImageName]];
        [item setAction:@selector(showInfo:)];
        [item setToolTip:nil];
    }
	else if ([itemIdentifier isEqualToString:GridSampleToolbarShowResultsItemIdentifier]) {
	
        item = [[NSToolbarItem alloc] initWithItemIdentifier:itemIdentifier];
        [item setPaletteLabel:GridSampleToolbarShowResultsItemLabel];
        [item setLabel:GridSampleToolbarShowResultsItemLabel];
        [item setImage:[NSImage imageNamed:GridSampleToolbarShowResultsItemImageName]];
        [item setAction:@selector(showResults:)];
        [item setToolTip:nil];
    }
	else if ([itemIdentifier isEqualToString:GridSampleToolbarCancelJobItemIdentifier]) {
	
        item = [[NSToolbarItem alloc] initWithItemIdentifier:itemIdentifier];
        [item setPaletteLabel:GridSampleToolbarCancelJobItemLabel];
        [item setLabel:GridSampleToolbarCancelJobItemLabel];
        [item setImage:[NSImage imageNamed:GridSampleToolbarCancelJobItemImageName]];
        [item setAction:@selector(cancelJob:)];
        [item setToolTip:nil];
    }
	else if ([itemIdentifier isEqualToString:GridSampleToolbarDeleteJobItemIdentifier]) {
	
        item = [[NSToolbarItem alloc] initWithItemIdentifier:itemIdentifier];
        [item setPaletteLabel:GridSampleToolbarDeleteJobItemLabel];
        [item setLabel:GridSampleToolbarDeleteJobItemLabel];
        [item setImage:[NSImage imageNamed:GridSampleToolbarDeleteJobItemImageName]];
        [item setAction:@selector(deleteJob:)];
        [item setToolTip:nil];
    }

    return [item autorelease];
}

@end
