/*
    File:  AudioCDWindowController.m
    
    Contains:  NSWindowController subclass to handle the UI for the document
     
    Copyright:  (c) Copyright 2005 Apple Computer, Inc. All rights reserved.

    Disclaimer: IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc.
    ("Apple") in consideration of your agreement to the following terms, and your
    use, installation, modification or redistribution of this Apple software
    constitutes acceptance of these terms.  If you do not agree with these terms,
    please do not use, install, modify or redistribute this Apple software.

    In consideration of your agreement to abide by the following terms, and subject
    to these terms, Apple grants you a personal, non-exclusive license, under Apple's
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
              
    Change History (most recent first):
            1.0 (July 5th, 2005)
*/

#import "AudioCDWindowController.h"
#import "EABDictionaryHelpers.h"
#import <unistd.h>


// Helper defines to turn off notification handling while we're
//	modifying the document ourselves.
#define EAB_DISABLE_NOTIFICATIONS		\
	NS_DURING					\
	 disableNotifications = YES;

#define EAB_ENABLE_NOTIFICATIONS		\
	NS_HANDLER					\
	 disableNotifications = NO;	\
	 [localException raise];	\
	NS_ENDHANDLER				\
	disableNotifications = NO;


// -------------------------------------------------------------------------
//	Constants
// -------------------------------------------------------------------------
#pragma mark Constants

enum {
	EABDiscOptionsTab	= 0,
	EABTrackOptionsTab	= 1
};

enum {
	EABTrackOptionsPersonalitySingle = 0,
	EABTrackOptionsPersonalityMultiple = 1,
	EABTrackOptionsPersonalityNone = 2
};

// Internal drag types
NSString * const EABTrackListRows = @"EABTrackListRows";

// Localized strings
NSString * EABSingularSecond = @"second";
NSString * EABPluralSeconds = @"seconds";
NSString * EABSingularTrack = @"%u track";
NSString * EABPluralTracks = @"%u tracks";
NSString * EABDiscLength = @"Total time: %@";
NSString * EABTimeFormat = @"%m:%02s";
NSString * EABInvalid = @"(Invalid)";

NSString * EABTTAddTracks = @"Add tracks";
NSString * EABTTRemoveTracks = @"Remove tracks";
NSString * EABTTBurnDisc = @"Record these files to an audio CD.";


@implementation AudioCDWindowController

#pragma mark -
#pragma mark Initialization

// -------------------------------------------------------------------------
//	initialize
// -------------------------------------------------------------------------
//	ObjC class initializer.
//
+ (void)initialize
{
	// Load the localized strings that we will need.
	EABSingularSecond = NSLocalizedString(EABSingularSecond,@"Noun, single second");
	EABPluralSeconds = NSLocalizedString(EABPluralSeconds,@"Noun, multiple seconds");
	EABSingularTrack = NSLocalizedString(EABSingularTrack,@"Format string, single track");
	EABPluralTracks = NSLocalizedString(EABPluralTracks,@"Format string, multiple tracks");
	EABDiscLength = NSLocalizedString(EABDiscLength,@"Format string, disc length");
	EABTimeFormat = NSLocalizedString(EABTimeFormat,@"Format string, time");
	EABInvalid = NSLocalizedString(EABInvalid,@"Warning text, used to indicate when ISRC is invalid");
	
	EABTTAddTracks = NSLocalizedString(EABTTAddTracks,@"Tooltip - Add tracks");
	EABTTRemoveTracks = NSLocalizedString(EABTTRemoveTracks,@"Tooltip - Remove tracks");
	EABTTBurnDisc = NSLocalizedString(EABTTBurnDisc,@"Tooltip - Burn disc");
}

- (void)dealloc
{
	// Log deallocations to make sure they're happening when we expect them to.
	//NSLog(@"-[AudioCDWindowController dealloc]");
	
	// We're going away, so we no longer care about notifications.
	[[NSNotificationCenter defaultCenter] removeObserver:self];
	[super dealloc];
}

// -------------------------------------------------------------------------
//	windowDidLoad
// -------------------------------------------------------------------------
//	NSWindowController - called when the window has finished loading.
//
- (void)windowDidLoad
{
	// Relay the message to our superclass.
	[super windowDidLoad];
	
	// Initialize instance variables; these are cached items
	//	from the document that we use frequently.
	document = (AudioCDDocument*)[self document];
	
	// Set tool tips on various controls.
	[_buttonDiscAddTrack setToolTip:EABTTAddTracks];
	[_buttonDiscRemoveTrack setToolTip:EABTTRemoveTracks];
	[_buttonDiscBurn setToolTip:EABTTBurnDisc];
	
	// Sign up for notifications on the document so that we'll know
	//	when an undo/redo occurs.
	[(NSNotificationCenter *)[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(trackListDidChange:)
		name:EABAudioCDTrackListChanged object:document];
	[(NSNotificationCenter *)[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(discPropertyDidChange:)
		name:EABAudioCDDiscPropertyChanged object:document];
	[(NSNotificationCenter *)[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(trackPropertyDidChange:)
		name:EABAudioCDTrackPropertyChanged object:document];
	[(NSNotificationCenter *)[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(willUndoOrRedoChange:)
		name:NSUndoManagerWillUndoChangeNotification object:[document undoManager]];
	[(NSNotificationCenter *)[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(willUndoOrRedoChange:)
		name:NSUndoManagerWillRedoChangeNotification object:[document undoManager]];
	[(NSNotificationCenter *)[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(didUndoOrRedoChange:)
		name:NSUndoManagerDidUndoChangeNotification object:[document undoManager]];
	[(NSNotificationCenter *)[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(didUndoOrRedoChange:)
		name:NSUndoManagerDidRedoChangeNotification object:[document undoManager]];
	
	// Register the drag types that the track table can accept.
	[_tableDiscTracks registerForDraggedTypes:[NSArray arrayWithObjects:
            EABTrackListRows, NSFilenamesPboardType, nil]];
	
	// The burn button needs to be told to scale the icon, otherwise it
	//	will try to use the image at full size (128x128) and clip!!
	NSSize	buttonSize = [_buttonDiscBurn frame].size;
	float	imageWidth = buttonSize.width;
	if (imageWidth > buttonSize.height - 20)
		imageWidth = buttonSize.height - 20;
	[[_buttonDiscBurn image] setScalesWhenResized:YES];
	[[_buttonDiscBurn image] setSize:NSMakeSize(imageWidth,imageWidth)];
	
	// Initialize the buttons and text fields for the incoming document,
	//	starting out with nothing selected.
	NSDictionary *discInfo = [document discInfo];
	[_buttonDiscBurn setEnabled:([document count] > 0) ? YES:NO];
	[_buttonDiscAddTrack setEnabled:YES];
	[_buttonDiscRemoveTrack setEnabled:NO];
	
	[self updateDiscTrackTable];
	
	[_tabviewMainOptions selectTabViewItemAtIndex:EABDiscOptionsTab];
	[_checkboxDiscCDText setState:[[discInfo numberForKey:EABDiscCDTextEnabled] intValue]];
	[_textDiscCDTextTitle setStringValue:[discInfo stringForKey:DRCDTextTitleKey]];
	[_textDiscCDTextPerformer setStringValue:[discInfo stringForKey:DRCDTextPerformerKey]];
	[_textDiscCDTextComposer setStringValue:[discInfo stringForKey:DRCDTextComposerKey]];
	[_textDiscCDTextSongwriter setStringValue:[discInfo stringForKey:DRCDTextSongwriterKey]];
	[_textDiscCDTextArranger setStringValue:[discInfo stringForKey:DRCDTextArrangerKey]];
	[_textDiscCDTextMessage setStringValue:[discInfo stringForKey:DRCDTextSpecialMessageKey]];
	[_textDiscCDTextDiscIdent setStringValue:[discInfo stringForKey:DRCDTextDiscIdentKey]];
	[_popupDiscCDTextGenre selectItemAtIndex:[[discInfo numberForKey:DRCDTextGenreCodeKey] intValue]];
	[_textDiscCDTextGenre setStringValue:[discInfo stringForKey:DRCDTextGenreKey]];
	[_textDiscCDTextClosed setStringValue:[discInfo stringForKey:DRCDTextClosedKey]];
	
	[_checkboxDiscMCN setState:[[discInfo numberForKey:EABDiscMCNEnabled] intValue]];
	[_textDiscMCN setStringValue:[discInfo stringForKey:EABDiscMCN]];
	
	[_tabviewTrackOptionsPersonality selectTabViewItemAtIndex:EABTrackOptionsPersonalityNone];
}


// -------------------------------------------------------------------------------
//	respondsToSelector
// -------------------------------------------------------------------------------
//	NSObject - overridden to print out who's asking us for methods.  Sometimes
//	this is useful when adding new code.
//
#if 0
- (BOOL)respondsToSelector:(SEL)selector
{
	NSLog(@"AudioCDWindowController rts - %@", NSStringFromSelector(selector));
	return [super respondsToSelector:selector];
}
#endif



#pragma mark -
#pragma mark Miscellaneous


// -------------------------------------------------------------------------------
//	validateMenuItem:
// -------------------------------------------------------------------------------
//	NSMenuValidation - enables and disables the menu items that we're responsible for.
//
- (BOOL)validateMenuItem:(NSMenuItem *)item
{
	// Nothing's enabled while we're burning.
	if (burning)
		return NO;
	
	// Otherwise check based on the action.
	BOOL enabled = YES;
	SEL action = [item action];
	
	if (action == @selector(discAddTrackRequest:) || action == @selector(discCDTextChanged:))
		enabled = YES;
	if (action == @selector(discRemoveTrackRequest:) || action == @selector(delete:))
		enabled = [_buttonDiscRemoveTrack isEnabled];
	else if (action == @selector(discBurnRequest:))
		enabled = [_buttonDiscBurn isEnabled];
	
	return enabled;
}


// -------------------------------------------------------------------------------
//	delete:
// -------------------------------------------------------------------------------
//	IBAction - equivalent to "Remove Tracks".
//
- (IBAction)delete:(id)sender
{
	[self discRemoveTrackRequest:sender];
}



// -------------------------------------------------------------------------------
//	isBurning
// -------------------------------------------------------------------------------
//	Returns YES if we're in the middle of burning, NO if not.
//
- (BOOL)isBurning
{
	return burning;
}



#pragma mark -
#pragma mark Burning



// -------------------------------------------------------------------------------
//	discBurnRequest:
// -------------------------------------------------------------------------------
//	Called when the "Burn Disc" button is pressed, or when the "Burn Disc" menu
//	item is selected.
//
- (IBAction)discBurnRequest:(id)sender
{
	// If there are no tracks, you can't burn.
	if ([document count] == 0)
		return;
	
	// Display the standard burn panel as a sheet.
	DRBurnSetupPanel*	bsp = [DRBurnSetupPanel setupPanel];
	[bsp beginSetupSheetForWindow:[self window]
		modalDelegate:self
		didEndSelector:@selector(burnSetupPanelDidEnd:returnCode:contextInfo:)
			contextInfo:nil];
}


// -------------------------------------------------------------------------------
//	burnSetupPanelDidEnd:returnCode:contextInfo:
// -------------------------------------------------------------------------------
//	DRSetupPanel delegate - called when the burn setup panel has completed.
//
- (void)burnSetupPanelDidEnd:(DRBurnSetupPanel*)panel returnCode:(int)returnCode contextInfo:(void*)contextInfo
{
	// Hide the sheet.
	[panel orderOut:self];
	
	// If the return code wasn't ok, don't do anything.
	if (returnCode != NSOKButton) return;
	
	// Get the burn object and fill in its properties.
	DRBurn		*burn = [panel burnObject];
	[document setBurnProperties:burn];
	
	// Create the tracks.
	id		layout = [document createLayoutForBurn];
	
	// Start the burn, displaying progress in the standard progress panel.
	DRBurnProgressPanel	*bpp = [DRBurnProgressPanel progressPanel];
	[bpp setDelegate:self];
	[bpp beginProgressSheetForBurn:burn layout:layout modalForWindow:[self window]];
}


// -------------------------------------------------------------------------------
//	burnProgressPanelWillBegin:
// -------------------------------------------------------------------------------
//	DRBurnProgressPanel delegate - called when the progress panel is starting.
//
- (void)burnProgressPanelWillBegin:(NSNotification*)notification
{
	// Remember that we're burning, and don't allow quitting or
	//	closing of the document during a burn.
	burning = YES;
}


// -------------------------------------------------------------------------------
//	burnProgressPanelDidFinish:
// -------------------------------------------------------------------------------
//	DRBurnProgressPanel delegate - called when the progress panel has finished.
//
- (void)burnProgressPanelDidFinish:(NSNotification*)notification
{
	// Whether the burn succeeded or failed, we're no longer burning.
	burning = NO;
}


// -------------------------------------------------------------------------------
//	windowShouldClose
// -------------------------------------------------------------------------------
//	NSWindow delegate - called when the window is trying to decide whether it's
//	okay to close.
//
- (BOOL)windowShouldClose
{
	return !burning;	// Don't close while burning.
}



#pragma mark -
#pragma mark Track table


// -------------------------------------------------------------------------------
//	numberOfRowsInTableView:
// -------------------------------------------------------------------------------
//	NSTableDataSource method - returns the number of rows.
//
- (int)numberOfRowsInTableView:(NSTableView*)table
{
	if (table == _tableDiscTracks)
		return [document count];
	else
		[NSException raise:NSInvalidArgumentException format:@"Unknown table view"];
	
	// make the compiler happy
	return 0;
}


// -------------------------------------------------------------------------------
//	tableView:objectValueForTableColumn:row:
// -------------------------------------------------------------------------------
//	NSTableDataSource method - returns the data to display.
//
- (id)tableView:(NSTableView*)table objectValueForTableColumn:(NSTableColumn*)column row:(int)row
{
	if (table == _tableDiscTracks)
	{
		// Track number
		int		trackNumber = (row+1);
		if (column == _tableColumnTrackNumber)
			return [NSNumber numberWithInt:trackNumber];
		
		// Filename
		NSDictionary	*trackInfo = [[document trackInfo] objectAtIndex:trackNumber];
		if (column == _tableColumnFilename)
			return [[[trackInfo stringForKey:EABTrackFilePath] lastPathComponent] stringByDeletingPathExtension];
		
		// Length
		if (column == _tableColumnLength)
			return [[DRMSF msfWithFrames:[[trackInfo numberForKey:EABTrackLength] intValue]] descriptionWithFormat:EABTimeFormat];
		
		[NSException raise:NSInvalidArgumentException format:@"Unknown table column"];
	}
	else
		[NSException raise:NSInvalidArgumentException format:@"Unknown table view"];
	
	// make the compiler happy
	return nil;
}


// -------------------------------------------------------------------------------
//	tableView:writeRows:toPasteboard:
// -------------------------------------------------------------------------------
//	NSTableDataSource method - validates an outgoing drag, and provides the data.
//
- (BOOL)tableView:(NSTableView*)table writeRows:(NSArray*)rows toPasteboard:(NSPasteboard*)pb
{
	// We only want to allow dragging of selected rows.
	NSEnumerator	*iter = [rows objectEnumerator];
	NSNumber		*row;
	while ((row = [iter nextObject]) != nil)
	{
		if (![table isRowSelected:[row intValue]])
			return NO;
	}
	
	// Check the table.
	if (table == _tableDiscTracks)
	{
		// Intra-table drag - data is the array of rows.
		[pb declareTypes:[NSArray arrayWithObject:EABTrackListRows] owner:nil];
		[pb setPropertyList:rows forType:EABTrackListRows];
		return YES;
	}
	else
		[NSException raise:NSInvalidArgumentException format:@"Unknown table view"];
	
	// make the compiler happy
	return NO;
}


// -------------------------------------------------------------------------------
//	tableView:validateDrop:proposedRow:proposedDropOperation:
// -------------------------------------------------------------------------------
//	NSTableDataSource method - validates an incoming drag to make sure the
//	table handles it correctly.  
//
- (NSDragOperation)tableView:(NSTableView*)table validateDrop:(id <NSDraggingInfo>)info proposedRow:(int)row proposedDropOperation:(NSTableViewDropOperation)op
{
	// Make drops at the end of the table go to the end.
	if (row == -1)
	{
		row = [table numberOfRows];
		op = NSTableViewDropAbove;
		[table setDropRow:row dropOperation:op];
	}
	
	// We don't ever want to drop onto a row, only between rows.
	if (op == NSTableViewDropOn)
		[table setDropRow:(row+1) dropOperation:NSTableViewDropAbove];
	return NSTableViewDropAbove;
}


// -------------------------------------------------------------------------------
//	tableView:acceptDrop:row:dropOperation:
// -------------------------------------------------------------------------------
//	NSTableDataSource method - receives a drag.
//
- (BOOL)tableView:(NSTableView*)table acceptDrop:(id <NSDraggingInfo>)info row:(int)dropRow dropOperation:(NSTableViewDropOperation)op;
{
	NSPasteboard	*pb = [info draggingPasteboard];
	NSDragOperation	srcMask = [info draggingSourceOperationMask];
	
	#if 0
	// Log the source mask for debugging.
	NSLog(@"[info draggingSourceOperationMask] = %u = %s%s%s%s%s%s", (unsigned int)srcMask,
		(srcMask & NSDragOperationCopy) ? "Copy ":"",
		(srcMask & NSDragOperationLink) ? "Link ":"",
		(srcMask & NSDragOperationGeneric) ? "Generic ":"",
		(srcMask & NSDragOperationPrivate) ? "Private ":"",
		(srcMask & NSDragOperationMove) ? "Move ":"",
		(srcMask & NSDragOperationDelete) ? "Delete":"");
	#endif
	
	if (table == _tableDiscTracks)
	{
		BOOL	accepted = NO;
		
		NS_DURING
			
			NSArray	*array;
			
			// - - - - - - - - - - - - - - - - - -
			// Intra-table drag - data is the array of rows.
			if (!accepted && (array = [pb propertyListForType:EABTrackListRows]) != NULL)
			{
				NSEnumerator *	iter = nil;
				BOOL			isCopy = (srcMask & NSDragOperationMove) ? NO:YES;
				
				// Move the tracks.
				dropRow = [document moveTracks:array to:dropRow copying:isCopy];
				
				// Select the newly-dragged items.
				iter = [array objectEnumerator];
				[table deselectAll:self];
				while ([iter nextObject] != NULL)
					[table selectRow:(dropRow++) byExtendingSelection:YES];
				
				// Indicate that we finished the drag.
				accepted = YES;
			}
			
			// - - - - - - - - - - - - - - - - - -
			// File drop from another app (such as Finder)
			if (!accepted && (array = [pb propertyListForType:NSFilenamesPboardType]) != NULL)
			{
				// We don't seem to get files from the Finder in any particular order,
				//	which means that the same drop performed multiple times has essentially
				//	(from the user's perspective) random ordering.  That's not so great.
				//	To make it deterministic, sort the incoming list before we add them.
				[self addFiles:[array sortedArrayUsingSelector:@selector(localizedCaseInsensitiveCompare:)] atIndex:dropRow];
				accepted = YES;
			}
			
		NS_HANDLER
		
			// An exception occurred. Uh-oh. Update the track table so that
			//	it stays consistent, and re-raise the exception.
			[self updateDiscTrackTable];
			[localException raise];
		
		NS_ENDHANDLER
		
		return accepted;
	}
	
	// Still here, so we didn't accept the drop.
	return NO;
}


// -------------------------------------------------------------------------------
//	tableViewSelectionDidChange:
// -------------------------------------------------------------------------------
//	NSTableViewNotifications - delegate method called when the selection changes.
//	We use this to update the options tabview.
//
- (void)tableViewSelectionDidChange:(NSNotification*)notification
{
	NSTableView	*table = (NSTableView*)[notification object];
	
	if (table == _tableDiscTracks)
	{
		// Update the options tab view.
		[self updateOptions];
		
		// Activate a tab based on what the user clicked.
		int	numSelectedRows = [_tableDiscTracks numberOfSelectedRows];
		if (numSelectedRows == 0)
		{
			// Switch to the "disc options" tab in the main view.
			[_tabviewMainOptions selectTabViewItemAtIndex:EABDiscOptionsTab];
			
			// Disable the "remove track" button.
			[_buttonDiscRemoveTrack setEnabled:NO];
		}
		else if (numSelectedRows == 1)
		{
			// Switch to the "track options" tab in the main view.
			[_tabviewMainOptions selectTabViewItemAtIndex:EABTrackOptionsTab];
			
			// Enable the "remove track" button.
			[_buttonDiscRemoveTrack setEnabled:YES];
		}
		else
		{
			// Switch to the "track options" tab in the main view.
			[_tabviewMainOptions selectTabViewItemAtIndex:EABTrackOptionsTab];
			
			// Enable the "remove track" button.
			[_buttonDiscRemoveTrack setEnabled:YES];
		}
	}
	else
		[NSException raise:NSInvalidArgumentException format:@"Unknown table view"];
}

// -------------------------------------------------------------------------------
//	updateOptions
// -------------------------------------------------------------------------------
//	Called when the options tabview needs updating.
//
- (void)updateOptions
{
	int	numSelectedRows = [_tableDiscTracks numberOfSelectedRows];
	
	// Update the disc options tab to reflect the values in the document.
	NSDictionary *discInfo = [document discInfo];
	[_checkboxDiscCDText setState:[[discInfo numberForKey:EABDiscCDTextEnabled] intValue]];
	[_textDiscCDTextTitle setStringValue:[discInfo stringForKey:DRCDTextTitleKey]];
	[_textDiscCDTextPerformer setStringValue:[discInfo stringForKey:DRCDTextPerformerKey]];
	[_textDiscCDTextComposer setStringValue:[discInfo stringForKey:DRCDTextComposerKey]];
	[_textDiscCDTextSongwriter setStringValue:[discInfo stringForKey:DRCDTextSongwriterKey]];
	[_textDiscCDTextArranger setStringValue:[discInfo stringForKey:DRCDTextArrangerKey]];
	[_textDiscCDTextMessage setStringValue:[discInfo stringForKey:DRCDTextSpecialMessageKey]];
	[_textDiscCDTextDiscIdent setStringValue:[discInfo stringForKey:DRCDTextDiscIdentKey]];
	[_popupDiscCDTextGenre selectItemAtIndex:[[discInfo numberForKey:DRCDTextGenreCodeKey] intValue]];
	[_textDiscCDTextGenre setStringValue:[discInfo stringForKey:DRCDTextGenreKey]];
	[_textDiscCDTextClosed setStringValue:[discInfo stringForKey:DRCDTextClosedKey]];
	
	[_checkboxDiscMCN setState:[[discInfo numberForKey:EABDiscMCNEnabled] intValue]];
	[_textDiscMCN setStringValue:[discInfo stringForKey:EABDiscMCN]];
	
	// Change the personality of the track options tab according to the selection,
	//	and update the data in the tab view to reflect the values in the document.
	if (numSelectedRows == 0)
	{
		// Nothing's selected.
		// Pop the track options tab over to its empty personality.
		[_tabviewTrackOptionsPersonality selectTabViewItemAtIndex:EABTrackOptionsPersonalityNone];
	}
	else if (numSelectedRows == 1)
	{
		NSDictionary *trackInfo = [document trackInfoForTrack:[_tableDiscTracks selectedRow]];
		NSNumber *preGap = [trackInfo numberForKey:EABTrackPreGap];
		
		// Switch to single-item personality.
		[_tabviewTrackOptionsPersonality selectTabViewItemAtIndex:EABTrackOptionsPersonalitySingle];
		
		// Update the interface to reflect the document's values for the current selection.
		[_textSingleTrackFilename setStringValue:[[trackInfo stringForKey:EABTrackFilePath] lastPathComponent]];
		[_textSingleTrackFilename setToolTip:[trackInfo stringForKey:EABTrackFilePath]];
		
		[_checkboxSingleTrackPreEmphasis setState:[[trackInfo numberForKey:EABTrackPreEmphasisEnabled] intValue]];
		
		[_textSingleTrackCDTextTitle setStringValue:[trackInfo stringForKey:DRCDTextTitleKey]];
		[_textSingleTrackCDTextPerformer setStringValue:[trackInfo stringForKey:DRCDTextPerformerKey]];
		[_textSingleTrackCDTextComposer setStringValue:[trackInfo stringForKey:DRCDTextComposerKey]];
		[_textSingleTrackCDTextSongwriter setStringValue:[trackInfo stringForKey:DRCDTextSongwriterKey]];
		[_textSingleTrackCDTextArranger setStringValue:[trackInfo stringForKey:DRCDTextArrangerKey]];
		[_textSingleTrackCDTextMessage setStringValue:[trackInfo stringForKey:DRCDTextSpecialMessageKey]];
		[_textSingleTrackCDTextClosed setStringValue:[trackInfo stringForKey:DRCDTextClosedKey]];
		
		[_textSingleTrackPreGap setObjectValue:preGap];
		if ([[_textSingleTrackPreGap stringValue] isEqualToString:@"1"])
			[_textSingleTrackPreGapSeconds setStringValue:EABSingularSecond];
		else
			[_textSingleTrackPreGapSeconds setStringValue:EABPluralSeconds];
		
		[_checkboxSingleTrackISRC setState:[[trackInfo numberForKey:EABTrackISRCEnabled] intValue]];
		[_textSingleTrackISRC setStringValue:[trackInfo stringForKey:EABTrackISRC]];
		if ([document hasValidISRCForTrack:[_tableDiscTracks selectedRow]])
			[_textSingleTrackISRCInvalid setStringValue:@""];
		else
			[_textSingleTrackISRCInvalid setStringValue:EABInvalid];
		[_checkboxSingleTrackISRCCDText setState:[[trackInfo numberForKey:EABTrackISRCInCDText] intValue]];
		
		[_checkboxSingleTrackIndexPoints setState:[[trackInfo numberForKey:EABTrackIndexPointsEnabled] intValue]];
	}
	else
	{
		NSNumber *preGap = [self aggregateTrackNumberForKey:EABTrackPreGap];
		
		// Switch to multiple-item personality.
		[_tabviewTrackOptionsPersonality selectTabViewItemAtIndex:EABTrackOptionsPersonalityMultiple];
		
		// Reset the bulk modify flags, which are used to keep track of what's changed.
		bulkModifyCDTextTitle = NO;
		bulkModifyCDTextPerformer = NO;
		bulkModifyCDTextComposer = NO;
		bulkModifyCDTextSongwriter = NO;
		bulkModifyCDTextArranger = NO;
		bulkModifyCDTextMessage = NO;
		bulkModifyCDTextClosed = NO;
		
		[_textBulkCDTextTitle setStringValue:[self aggregateTrackStringForKey:DRCDTextTitleKey]];
		[_textBulkCDTextPerformer setStringValue:[self aggregateTrackStringForKey:DRCDTextPerformerKey]];
		[_textBulkCDTextComposer setStringValue:[self aggregateTrackStringForKey:DRCDTextComposerKey]];
		[_textBulkCDTextSongwriter setStringValue:[self aggregateTrackStringForKey:DRCDTextSongwriterKey]];
		[_textBulkCDTextArranger setStringValue:[self aggregateTrackStringForKey:DRCDTextArrangerKey]];
		[_textBulkCDTextMessage setStringValue:[self aggregateTrackStringForKey:DRCDTextSpecialMessageKey]];
		[_textBulkCDTextClosed setStringValue:[self aggregateTrackStringForKey:DRCDTextClosedKey]];
		
		int	preemp = [self aggregateTrackCheckboxStateForKey:EABTrackPreEmphasisEnabled];
		[_checkboxBulkPreEmphasis setAllowsMixedState:(preemp == NSMixedState) ? YES:NO];
		[_checkboxBulkPreEmphasis setState:preemp];
		
		if (preGap == nil)
			[_textBulkPreGap setStringValue:@""];
		else
		{
			[_textBulkPreGap setObjectValue:preGap];
			if ([[_textBulkPreGap stringValue] isEqualToString:@"1"])
				[_textBulkPreGapSeconds setStringValue:EABSingularSecond];
			else
				[_textBulkPreGapSeconds setStringValue:EABPluralSeconds];
		}
	}
}


// -------------------------------------------------------------------------------
//	aggregateTrackCheckboxStateForKey:
// -------------------------------------------------------------------------------
//	Helper method for the bulk-modify tab view item.
//
//	Returns a checkbox state which is the aggregate of all selected rows in the
//	track table.
//
//	If values are mixed, returns 2.
//	If all values are 1, returns 1.
//	If all values are 0, returns 0.
//
- (int)aggregateTrackCheckboxStateForKey:(NSString *)key
{
	NSNumber	*val = [self aggregateTrackNumberForKey:key];
	if (val == nil)
		return NSMixedState;
	else if ([val intValue] == 0)
		return NSOffState;
	else
		return NSOnState;
}


// -------------------------------------------------------------------------------
//	aggregateTrackStringForKey:
// -------------------------------------------------------------------------------
//	Helper method for the bulk-modify tab view item.
//
//	Returns a string which is the aggregate of the values for all selected rows
//	in the track table.  nil is considered equivalent to the empty string.
//
//	If values are mixed, returns an empty string.
//	If values are identical, returns the value.
//
- (NSString*)aggregateTrackStringForKey:(NSString*)key
{
	// Loop through and start looking at keys.
	unsigned	i, count = [document count];
	NSString*	string = nil;
	BOOL		firstTime = YES;
	for (i=0; i<count; ++i)
	{
		// Only the selected tracks, please.
		if ([_tableDiscTracks isRowSelected:i])
		{
			NSDictionary *trackInfo = [document trackInfoForTrack:i];
			NSString*	thisString = [trackInfo stringForKey:key];
			
			if (firstTime == YES)
				string = thisString;
			else if (![string isEqualToString:thisString])
				return @"";	// mixed, so return empty string
			
			firstTime = NO;
		}
	}
	
	return string; // all the same, return the string
}


// -------------------------------------------------------------------------------
//	aggregateTrackNumberForKey:
// -------------------------------------------------------------------------------
//	Helper method for the bulk-modify tab view item.
//
//	Returns a number which is the aggregate of the values for all selected rows
//	in the track table.  nil is considered equivalent to NSNumber 0.
//
//	If values are mixed, returns nil.
//	If values are identical, returns the value.
//
- (NSNumber*)aggregateTrackNumberForKey:(NSString*)key
{
	// Loop through and start looking at keys.
	unsigned	i, count = [document count];
	NSNumber*	number = nil;
	BOOL		firstTime = YES;
	for (i=0; i<count; ++i)
	{
		// Only the selected tracks, please.
		if ([_tableDiscTracks isRowSelected:i])
		{
			NSDictionary *trackInfo = [document trackInfoForTrack:i];
			NSNumber*	thisNumber = [trackInfo numberForKey:key];
			
			if (firstTime == YES)
				number = thisNumber;
			else if (![number isEqual:thisNumber])
				return nil;	// mixed, so return nil
			
			firstTime = NO;
		}
	}
	
	return number; // all the same, return the number
}


// -------------------------------------------------------------------------------
//	updateDiscTrackTable
// -------------------------------------------------------------------------------
//	Helper method, called to update the table when one of the following occurs:
//	- The track list changes (add/remove/reorder)
//	- The selection changes
//
- (void)updateDiscTrackTable
{
	// Tell the table to reload.
	[_tableDiscTracks reloadData];
	
	// Manually update the summary items below the table.
	NSString *format = [document count] == 1 ? EABSingularTrack:EABPluralTracks;
	[_textDiscTrackCount setStringValue:[NSString stringWithFormat:format, [document count]]];
	[_textDiscLength setStringValue:[NSString stringWithFormat:EABDiscLength, [[document totalLength] descriptionWithFormat:EABTimeFormat]]];
	
	// Manually enable/disable the "Remove Track" button.
	[_buttonDiscRemoveTrack setEnabled:([_tableDiscTracks numberOfSelectedRows] == 0) ? NO:YES];
	
	// Manually enable/disable the "Burn Disc" button.
	[_buttonDiscBurn setEnabled:([document count] > 0) ? YES:NO];
}



#pragma mark -
#pragma mark Track table actions


// -------------------------------------------------------------------------------
//	discAddTrackRequest:
// -------------------------------------------------------------------------------
//	IBAction - called when the "Add Track" button is pushed, or when the
//	"Add Track" menu item is selected.
//
- (IBAction)discAddTrackRequest:(id)sender
{
	NSOpenPanel	*panel = [NSOpenPanel openPanel];
	[panel setAllowsMultipleSelection:YES];
	[panel setCanChooseDirectories:NO];
	[panel setCanChooseFiles:YES];
	[panel setResolvesAliases:YES];
	[panel setTitle:NSLocalizedString(@"Select one or more audio files.",@"Open panel - prompt when adding tracks")];
	[panel setPrompt:NSLocalizedString(@"Select","Open panel - button when adding tracks")];
	
	[panel beginSheetForDirectory:nil file:nil types:nil
		   modalForWindow:[self window]
		   modalDelegate:self
		   didEndSelector:@selector(openPanelDidEnd:returnCode:contextInfo:)
		   contextInfo:self];
}


// -------------------------------------------------------------------------------
//	openPanelDidEnd:returnCode:contextInfo:
// -------------------------------------------------------------------------------
//	NSOpenPanel delegate - called when the open panel from "Add Track" completes.
//
- (void)openPanelDidEnd:(NSOpenPanel*)panel returnCode:(int)returnCode contextInfo:(void *)contextInfo
{
	// Hide the panel.
	[panel orderOut:self];
	
	// If the return code wasn't ok, don't do anything.
	if (returnCode != NSOKButton) return;
	
	// Get the filenames, then add the tracks at the end.
	NSArray *filenames = [panel filenames];
	[self addFiles:[filenames sortedArrayUsingSelector:@selector(localizedCaseInsensitiveCompare:)] atIndex:[document count]];
}

// -------------------------------------------------------------------------------
//	addFiles:atIndex:
// -------------------------------------------------------------------------------
//	Helper method which adds the specified list of files to the disc.  May be invoked
//	by the following methods:
//
//	- openPanelDidEnd:returnCode:contextInfo:
//	- tableView:acceptDrop:row:dropOperation:
//
//	Currently adding isn't threaded; it blocks the whole UI while processing files.
//	This could be improved.
//
- (void)addFiles:(NSArray*)filenames atIndex:(unsigned)index
{
	unsigned int i = 0, count = [filenames count];
	
	// Sanity check that we're not adding zero files.
	if (count == 0)
		return;
	
	// If we're adding just one file, do it and exit.
	if (count == 1)
	{
		NS_DURING
			EAB_DISABLE_NOTIFICATIONS
			[document addTrackFromFile:[filenames objectAtIndex:0] atIndex:index];
			EAB_ENABLE_NOTIFICATIONS
		NS_HANDLER
		NS_ENDHANDLER
		
		[self updateDiscTrackTable];
		return;
	}
	
	// Otherwise we're doing more than one file, which can take
	//	a while, so we're going to put up a sheet indicating progress
	//	to the user.
	NS_DURING
		
		// Put up the progress sheet.
		[_textAddingFilename setStringValue:@""];
		[_progressAdding setIndeterminate:NO];
		[_progressAdding setMinValue:0.0];
		[_progressAdding setMaxValue:(double)count];
		[_progressAdding setDoubleValue:0.0];
		
		[NSApp beginSheet:_panelAdding
				modalForWindow:[self window]
				modalDelegate:nil
				didEndSelector:nil
				contextInfo:nil];
		
		// Go through the filenames and process them.
		for (i = 0; i < count; ++i)
		{
			NSString	*path = [filenames objectAtIndex:i];
			
			// Update the display in the sheet.
			[_textAddingFilename setStringValue:[path lastPathComponent]];
			[_textAddingFilename display];
			[_progressAdding setDoubleValue:(double)(i+1)];
			[_progressAdding display];
			
			// Add the file.
			NS_DURING
				EAB_DISABLE_NOTIFICATIONS
				[document addTrackFromFile:path atIndex:index++];
				EAB_ENABLE_NOTIFICATIONS
			NS_HANDLER
			NS_ENDHANDLER
			
			// Process a few events so that we're not totally unresponsive.
			//	If we don't respond to certain asynchronous messages, we may get
			//	disconnected from autodiskmount or other important system services.
			[[NSRunLoop currentRunLoop] runUntilDate:[NSDate dateWithTimeIntervalSinceNow:0.1]];
			
			// Update the table as each file is processed.
			[self updateDiscTrackTable];
			[_tableDiscTracks display];
		}
		
		// Tear down the sheet.
		[NSApp endSheet:_panelAdding];
		[_panelAdding orderOut:self];
		
	NS_HANDLER
		
		// Make sure we tear down the sheet if a problem occurs.
		[NSApp endSheet:_panelAdding];
		[_panelAdding orderOut:self];
		
	NS_ENDHANDLER
}


// -------------------------------------------------------------------------------
//	discRemoveTrackRequest:
// -------------------------------------------------------------------------------
//	Called when the "Remove Track" button is pressed, or when the "Remove Track"
//	menu item is selected.
//
- (IBAction)discRemoveTrackRequest:(id)sender
{
	// Only perform this action if there are selected rows.
	if ([_tableDiscTracks numberOfSelectedRows] != 0)
	{
		// Loop (backwards!) through the selected rows.  We temporarily disable
		//	notifications while doing this.
		EAB_DISABLE_NOTIFICATIONS
		unsigned	i, count = [document count];
		for (i=count; i>0; --i)
		{
			// Remove the selected tracks.
			if ([_tableDiscTracks isRowSelected:(i-1)])
				[document removeTrackAtIndex:(i-1)];
		}
		EAB_ENABLE_NOTIFICATIONS
		
		// We've changed the document details, so update.
		[self updateDiscTrackTable];
		
		// Clear the selection.
		[_tableDiscTracks deselectAll:self];
	}
}


#pragma mark -
#pragma mark Disc options


// -------------------------------------------------------------------------------
//	discCDTextChanged:
// -------------------------------------------------------------------------------

- (IBAction)discCDTextChanged:(id)sender
{
	id			oldValue = nil, newValue = nil;
	NSString	*key = nil;
	
	// If the user picked something from the genre code menu, copy the text
	//	of the menu item into the "Genre Name" field.
	if (sender == _popupDiscCDTextGenre)
	{
		[_textDiscCDTextGenre setStringValue:[[_popupDiscCDTextGenre selectedItem] title]];
		[self discCDTextChanged:_textDiscCDTextGenre];
	}
	
	// Find the key to use.
	if (sender == _textDiscCDTextTitle)			key = DRCDTextTitleKey;
	if (sender == _textDiscCDTextPerformer)		key = DRCDTextPerformerKey;
	if (sender == _textDiscCDTextComposer)		key = DRCDTextComposerKey;
	if (sender == _textDiscCDTextSongwriter)	key = DRCDTextSongwriterKey;
	if (sender == _textDiscCDTextArranger)		key = DRCDTextArrangerKey;
	if (sender == _textDiscCDTextMessage)		key = DRCDTextSpecialMessageKey;
	if (sender == _textDiscCDTextClosed)		key = DRCDTextClosedKey;
	if (sender == _textDiscCDTextDiscIdent)		key = DRCDTextDiscIdentKey;
	if (sender == _textDiscCDTextGenre)			key = DRCDTextGenreKey;
	if (sender == _popupDiscCDTextGenre)		key = DRCDTextGenreCodeKey;
	
	// Get the old value.
	oldValue = [[[document discPropertyForKey:key] retain] autorelease];
	
	// Update the document with the values from the text fields.
	EAB_DISABLE_NOTIFICATIONS
	if (sender == _popupDiscCDTextGenre)
		[document setDiscProperty:[NSNumber numberWithInt:[sender indexOfSelectedItem]] forKey:key];
	else
		[document setDiscProperty:[sender stringValue] forKey:key];
	
	// Get the new value.
	newValue = [[[document discPropertyForKey:key] retain] autorelease];
	
	// Compare old and new values, and if something changed then make sure
	//	we set CD-Text to be enabled.
	if (oldValue != newValue &&
		((oldValue != nil && newValue == nil) || (oldValue == nil && newValue != nil) || ![oldValue isEqual:newValue]))
	{
		[document setDiscProperty:[NSNumber numberWithBool:YES] forKey:EABDiscCDTextEnabled];
		[_checkboxDiscCDText setState:NSOnState];
	}
	EAB_ENABLE_NOTIFICATIONS
	
	[self updateOptions];
}

// -------------------------------------------------------------------------------
//	discCDTextToggle:
// -------------------------------------------------------------------------------

- (IBAction)discCDTextToggle:(id)sender
{
	EAB_DISABLE_NOTIFICATIONS
	[document setDiscProperty:[NSNumber numberWithBool:(BOOL)[_checkboxDiscCDText intValue]] forKey:EABDiscCDTextEnabled];
	EAB_ENABLE_NOTIFICATIONS
}

// -------------------------------------------------------------------------------
//	discMCNChanged:
// -------------------------------------------------------------------------------

- (IBAction)discMCNChanged:(id)sender
{
	EAB_DISABLE_NOTIFICATIONS
	[document setDiscProperty:[_textDiscMCN stringValue] forKey:EABDiscMCN];
	
	// Automatically turn off the checkbox when the string is empty.
	if ([[_textDiscMCN stringValue] isEqualToString:@""])
	{
		[document setDiscProperty:[NSNumber numberWithBool:NO] forKey:EABDiscMCNEnabled];
		[_checkboxDiscMCN setState:NSOffState];
	}
	
	// Automatically enable the checkbox when a valid ISRC is entered.
	else
	{
		[document setDiscProperty:[NSNumber numberWithBool:YES] forKey:EABDiscMCNEnabled];
		[_checkboxDiscMCN setState:NSOnState];
	}
	EAB_ENABLE_NOTIFICATIONS
}

// -------------------------------------------------------------------------------
//	discMCNToggle:
// -------------------------------------------------------------------------------

- (IBAction)discMCNToggle:(id)sender
{
	EAB_DISABLE_NOTIFICATIONS
	[document setDiscProperty:[NSNumber numberWithBool:(BOOL)[_checkboxDiscMCN intValue]] forKey:EABDiscMCNEnabled];
	EAB_ENABLE_NOTIFICATIONS
}


#pragma mark -
#pragma mark Single track options


// -------------------------------------------------------------------------------
//	singleTrackShowFileRequest:
// -------------------------------------------------------------------------------

- (IBAction)singleTrackShowFileRequest:(id)sender
{
	NSString *path = [document propertyForKey:EABTrackFilePath ofTrack:[_tableDiscTracks selectedRow]];
	[[NSWorkspace sharedWorkspace] selectFile:path inFileViewerRootedAtPath:nil];
}



// -------------------------------------------------------------------------------
//	singleTrackCDTextChanged:
// -------------------------------------------------------------------------------

- (IBAction)singleTrackCDTextChanged:(id)sender
{
	int trackIndex = [_tableDiscTracks selectedRow];
	
	// Update the document.  We temporarily disable notifications while doing this.
	EAB_DISABLE_NOTIFICATIONS
	[document setProperty:[_textSingleTrackCDTextTitle stringValue] forKey:DRCDTextTitleKey ofTrack:trackIndex];
	[document setProperty:[_textSingleTrackCDTextPerformer stringValue] forKey:DRCDTextPerformerKey ofTrack:trackIndex];
	[document setProperty:[_textSingleTrackCDTextComposer stringValue] forKey:DRCDTextComposerKey ofTrack:trackIndex];
	[document setProperty:[_textSingleTrackCDTextSongwriter stringValue] forKey:DRCDTextSongwriterKey ofTrack:trackIndex];
	[document setProperty:[_textSingleTrackCDTextArranger stringValue] forKey:DRCDTextArrangerKey ofTrack:trackIndex];
	[document setProperty:[_textSingleTrackCDTextMessage stringValue] forKey:DRCDTextSpecialMessageKey ofTrack:trackIndex];
	[document setProperty:[_textSingleTrackCDTextClosed stringValue] forKey:DRCDTextClosedKey ofTrack:trackIndex];
	EAB_ENABLE_NOTIFICATIONS
	
	[self updateOptions];
}

// -------------------------------------------------------------------------------
//	singleTrackPreGapChanged:
// -------------------------------------------------------------------------------

- (IBAction)singleTrackPreGapChanged:(id)sender
{
	EAB_DISABLE_NOTIFICATIONS
	[document setProperty:[NSNumber numberWithFloat:[_textSingleTrackPreGap floatValue]]
			forKey:EABTrackPreGap ofTrack:[_tableDiscTracks selectedRow]];
	EAB_ENABLE_NOTIFICATIONS
	
	// Handle singular/plural consistency.
	if ([[_textSingleTrackPreGap stringValue] isEqualToString:@"1"])
		[_textSingleTrackPreGapSeconds setStringValue:EABSingularSecond];
	else
		[_textSingleTrackPreGapSeconds setStringValue:EABPluralSeconds];
}

// -------------------------------------------------------------------------------
//	singleTrackPreEmphasisToggle:
// -------------------------------------------------------------------------------

- (IBAction)singleTrackPreEmphasisToggle:(id)sender
{
	EAB_DISABLE_NOTIFICATIONS
	[document setProperty:[NSNumber numberWithBool:(BOOL)[_checkboxSingleTrackPreEmphasis intValue]]
			forKey:EABTrackPreEmphasisEnabled ofTrack:[_tableDiscTracks selectedRow]];
	EAB_ENABLE_NOTIFICATIONS
}

// -------------------------------------------------------------------------------
//	singleTrackISRCToggle:
// -------------------------------------------------------------------------------

- (IBAction)singleTrackISRCToggle:(id)sender
{
	EAB_DISABLE_NOTIFICATIONS
	[document setProperty:[NSNumber numberWithBool:(BOOL)[_checkboxSingleTrackISRC intValue]]
			forKey:EABTrackISRCEnabled ofTrack:[_tableDiscTracks selectedRow]];
	[document setProperty:[NSNumber numberWithBool:(BOOL)[_checkboxSingleTrackISRCCDText intValue]]
			forKey:EABTrackISRCInCDText ofTrack:[_tableDiscTracks selectedRow]];
	EAB_ENABLE_NOTIFICATIONS
}

// -------------------------------------------------------------------------------
//	singleTrackISRCChanged:
// -------------------------------------------------------------------------------

- (IBAction)singleTrackISRCChanged:(id)sender
{
	BOOL	valid = [document hasValidISRCForTrack:[_tableDiscTracks selectedRow]];
	
	EAB_DISABLE_NOTIFICATIONS
	[document setProperty:[_textSingleTrackISRC stringValue]
			forKey:EABTrackISRC ofTrack:[_tableDiscTracks selectedRow]];
	
	// Automatically turn off the checkbox when the string is empty.
	if ([[_textSingleTrackISRC stringValue] isEqualToString:@""])
	{
		[document setProperty:[NSNumber numberWithBool:NO]
			forKey:EABTrackISRCEnabled ofTrack:[_tableDiscTracks selectedRow]];
		[_checkboxSingleTrackISRC setState:NSOffState];
	}
	
	// Automatically enable the checkbox when a valid ISRC is entered.
	else if (valid)
	{
		[document setProperty:[NSNumber numberWithBool:YES]
			forKey:EABTrackISRCEnabled ofTrack:[_tableDiscTracks selectedRow]];
		[_checkboxSingleTrackISRC setState:NSOnState];
	}
	EAB_ENABLE_NOTIFICATIONS
	
	// Display validity of the ISRC.
	if (valid)
		[_textSingleTrackISRCInvalid setStringValue:@""];
	else
		[_textSingleTrackISRCInvalid setStringValue:EABInvalid];
}

// -------------------------------------------------------------------------------
//	singleTrackIndexPointsToggle:
// -------------------------------------------------------------------------------

- (IBAction)singleTrackIndexPointsToggle:(id)sender
{
	EAB_DISABLE_NOTIFICATIONS
	[document setProperty:[NSNumber numberWithBool:(BOOL)[_checkboxSingleTrackIndexPoints intValue]]
			forKey:EABTrackIndexPointsEnabled ofTrack:[_tableDiscTracks selectedRow]];
	EAB_ENABLE_NOTIFICATIONS
}

// -------------------------------------------------------------------------------
//	singleTrackIndexPointsChanged:
// -------------------------------------------------------------------------------

- (IBAction)singleTrackIndexPointsChanged:(id)sender
{
	// There's no interface for actually setting index points yet - this is a stub
	//	and is never invoked.
	EAB_DISABLE_NOTIFICATIONS
	EAB_ENABLE_NOTIFICATIONS
}



#pragma mark -
#pragma mark Control delegates



// -------------------------------------------------------------------------
//	control:textShouldBeginEditing:
// -------------------------------------------------------------------------
//	Delegate method for the bulk modify text fields for CD-Text.
//
- (BOOL)control:(NSControl *)control textShouldBeginEditing:(NSText *)text
{
	NSTextField	*	sender = (NSTextField*)control;
	
	// Set the bulk modification flag corresponding to the field that the user has modified.
	if (sender == _textBulkCDTextTitle)				bulkModifyCDTextTitle = YES;
	else if (sender == _textBulkCDTextPerformer)	bulkModifyCDTextPerformer = YES;
	else if (sender == _textBulkCDTextComposer)		bulkModifyCDTextComposer = YES;
	else if (sender == _textBulkCDTextArranger)		bulkModifyCDTextArranger = YES;
	else if (sender == _textBulkCDTextSongwriter)	bulkModifyCDTextSongwriter = YES;
	else if (sender == _textBulkCDTextMessage)		bulkModifyCDTextMessage = YES;
	else if (sender == _textBulkCDTextClosed)		bulkModifyCDTextClosed = YES;
	
	// Okay, go ahead and begin editing.
	return YES;
}


// -------------------------------------------------------------------------
//	controlTextDidChange:
// -------------------------------------------------------------------------
//
- (void)controlTextDidChange:(NSNotification *)notification
{
	id	sender = [notification object];
	if (sender == _textSingleTrackISRC)
	{
		// Check the text as the user's editing it.
		NSString *isrc = [_textSingleTrackISRC stringValue];
		if ([document isValidISRC:isrc incomplete:YES])
			[_textSingleTrackISRCInvalid setStringValue:@""];
		else
			[_textSingleTrackISRCInvalid setStringValue:EABInvalid];
	}
}


#pragma mark -
#pragma mark Multiple track options



// -------------------------------------------------------------------------
//	bulkCDTextChanged:
// -------------------------------------------------------------------------
//	Called when a CD-Text bulk modify field has finished editing.
//
- (IBAction)bulkCDTextChanged:(id)sender
{
	// Gather the values from any text fields whose checkboxes are set.
	NSString	*keys[16], *values[16];
	unsigned	num = 0;
	
	if (bulkModifyCDTextTitle)
	{
		keys[num] = DRCDTextTitleKey;
		values[num++] = [_textBulkCDTextTitle stringValue];
	}
	if (bulkModifyCDTextPerformer)
	{
		keys[num] = DRCDTextPerformerKey;
		values[num++] = [_textBulkCDTextPerformer stringValue];
	}
	if (bulkModifyCDTextComposer)
	{
		keys[num] = DRCDTextComposerKey;
		values[num++] = [_textBulkCDTextComposer stringValue];
	}
	if (bulkModifyCDTextSongwriter)
	{
		keys[num] = DRCDTextSongwriterKey;
		values[num++] = [_textBulkCDTextSongwriter stringValue];
	}
	if (bulkModifyCDTextArranger)
	{
		keys[num] = DRCDTextArrangerKey;
		values[num++] = [_textBulkCDTextArranger stringValue];
	}
	if (bulkModifyCDTextMessage)
	{
		keys[num] = DRCDTextSpecialMessageKey;
		values[num++] = [_textBulkCDTextMessage stringValue];
	}
	if (bulkModifyCDTextClosed)
	{
		keys[num] = DRCDTextClosedKey;
		values[num++] = [_textBulkCDTextClosed stringValue];
	}
	
	// Loop through the selected tracks and update the document with the
	//	modified values.  We temporarily turn off notification handling
	//	while we're doing this.
	EAB_DISABLE_NOTIFICATIONS
	unsigned	i, j, count = [document count];
	for (i=0; i<count; ++i)
	{
		// Only the selected tracks, please.
		if ([_tableDiscTracks isRowSelected:i])
		{
			for (j=0; j<num; ++j)
				[document setProperty:values[j] forKey:keys[j] ofTrack:i];
		}
	}
	EAB_ENABLE_NOTIFICATIONS
	
	[self updateDiscTrackTable];
}




// -------------------------------------------------------------------------
//	bulkPreEmphasisToggle:
// -------------------------------------------------------------------------
//
- (IBAction)bulkPreEmphasisToggle:(id)sender
{
	// Once the user has clicked it, the checkbox no longer allows a mixed state.
	[_checkboxBulkPreEmphasis setAllowsMixedState:NO];
	
	// Get the new state from the checkbox.
	NSNumber	*val = [NSNumber numberWithBool:(BOOL)[_checkboxBulkPreEmphasis intValue]];
	
	// Loop through the selected tracks and update the document with the
	//	modified values.
	EAB_DISABLE_NOTIFICATIONS
	unsigned	i, count = [document count];
	for (i=0; i<count; ++i)
	{
		// Only the selected tracks, please.
		if ([_tableDiscTracks isRowSelected:i])
			[document setProperty:val forKey:EABTrackPreEmphasisEnabled ofTrack:i];
	}
	EAB_ENABLE_NOTIFICATIONS
	
	[self updateOptions];
}

// -------------------------------------------------------------------------
//	bulkPreGapChanged:
// -------------------------------------------------------------------------
//
- (IBAction)bulkPreGapChanged:(id)sender
{
	NSNumber	*val = [NSNumber numberWithFloat:[_textBulkPreGap floatValue]];
	
	// Loop through the selected tracks and update the document with the
	//	modified values.
	EAB_DISABLE_NOTIFICATIONS
	unsigned	i, count = [document count];
	for (i=0; i<count; ++i)
	{
		// Only the selected tracks, please.
		if ([_tableDiscTracks isRowSelected:i])
			[document setProperty:val forKey:EABTrackPreGap ofTrack:i];
	}
	EAB_ENABLE_NOTIFICATIONS
	
	[self updateOptions];
}


#pragma mark -
#pragma mark Notifications



// -------------------------------------------------------------------------
//	trackListDidChange:
// -------------------------------------------------------------------------
//	Notification received when the track list changes.
//
- (void)trackListDidChange:(NSNotification*)notification
{
	// Notifications are disabled when the UI itself is modifying properties.
	if (disableNotifications) return;
	
	// Record this event into the change list.
	id whichTracks = [[notification userInfo] objectForKey:EABWhichTrackKey];
	if (whichTracks != nil) [listOfChanges addObject:whichTracks];
	
	// Update the UI.
	[self updateDiscTrackTable];
}


// -------------------------------------------------------------------------
//	discPropertyDidChange:
// -------------------------------------------------------------------------
//	Notification received when a disc property changes.
//
- (void)discPropertyDidChange:(NSNotification*)notification
{
	// Notifications are disabled when the UI itself is modifying properties.
	if (disableNotifications) return;
	
	// Record this event into the change list.
	[listOfChanges addObject:[NSNumber numberWithInt:EABTracksRemoved]];
	
	// Update the UI.
	[self updateOptions];
}


// -------------------------------------------------------------------------
//	trackPropertyDidChange:
// -------------------------------------------------------------------------
//	Notification received when a track property changes.
//
- (void)trackPropertyDidChange:(NSNotification*)notification
{
	// Notifications are disabled when the UI itself is modifying properties.
	if (disableNotifications) return;
	
	// Record this event into the change list.
	id whichTracks = [[notification userInfo] objectForKey:EABWhichTrackKey];
	if (whichTracks != nil) [listOfChanges addObject:whichTracks];
	
	// Update the UI.
	[self updateOptions];
}


// -------------------------------------------------------------------------
//	willUndoOrRedoChange:
// -------------------------------------------------------------------------
//	Notification received when an undo/redo is starting. 
//
- (void)willUndoOrRedoChange:(NSNotification*)notification
{
	// Start tracking changes.
	[listOfChanges release];
	listOfChanges = [[NSMutableArray alloc] initWithCapacity:[document count]];
}


// -------------------------------------------------------------------------
//	didUndoOrRedoChange:
// -------------------------------------------------------------------------
//	Notification received when an undo/redo has completed.  We change
//	tabs, selections, etc in the UI to display the change(s) that occurred.
//
- (void)didUndoOrRedoChange:(NSNotification*)notification
{
	NSMutableArray	*rowsToSelect = [NSMutableArray arrayWithCapacity:[document count]];
	
	// Parse the list that we built up as we received notifications.
	NSEnumerator	*iter = [listOfChanges objectEnumerator];
	NSNumber		*val;
	while ((val = [iter nextObject]) != nil)
	{
		// Each object in the array is an NSNumber.
		int		trackIndex = [val intValue];
		if (trackIndex == EABTracksRemoved)
			[rowsToSelect removeAllObjects];
		else if (![rowsToSelect containsObject:val])
			[rowsToSelect addObject:val];
	}
	
	// Select the changed rows.  Since we're the tableview delegate, we'll notice
	//	the selection change and automatically switch to the appropriate tab.
	[_tableDiscTracks deselectAll:self];
	iter = [rowsToSelect objectEnumerator];
	while ((val = [iter nextObject]) != nil)
		[_tableDiscTracks selectRow:[val intValue] byExtendingSelection:YES];
	
	// Stop tracking changes.
	[listOfChanges release];
	listOfChanges = nil;
}


@end


