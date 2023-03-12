/*

File: EntryListController.m

Abstract: Use the PubSub framework to create a simple RSS news reader. 

Version: <1.0>

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

Copyright © 2006-2007 Apple Inc. All Rights Reserved.

*/

#import "EntryListController.h"
#import "FeedListController.h"
#import "ContentFormatting.h"
#import "NewsReaderApp.h"
#import <PubSub/PubSub.h>


#pragma mark -
#pragma mark ENTRYLISTCONTROLLER:


// Time since update until activity level decays to zero
#define kActivityThresholdTime  (2 * 60 * 60)


@implementation EntryListController


static NSFont *kNormalFont, *kBoldFont;


- (void) awakeFromNib
{
    // We're going to swap back and forth between these fonts later, depending upon what's happening
    // with the entry that's being displayed. Save these off.
    kNormalFont = [[[[_table tableColumns] objectAtIndex: 0] dataCell] font];
    kBoldFont = [[NSFontManager sharedFontManager] convertFont: kNormalFont toHaveTrait: NSBoldFontMask];
    
    // Double clicks in the table view should call openEntryInBrowser:
    [_table setDoubleAction: @selector(openEntryInBrowser:)];
}


// The feed for which entries are currently being displayed
- (PSFeed*) currentFeed
{
    return _currentFeed;
}


// Change the feed for which entries are being displayed
- (void) setCurrentFeed: (PSFeed*)feed
{
    if( feed != _currentFeed ) {
        NSLog(@"EntryListController: Now showing %@",feed);
        // If we're changing our notion of currentFeed, we'll want to remove ourself as observer
        // for its PSFeedEntriesChangedNotification
        if( _currentFeed )
            [[NSNotificationCenter defaultCenter] removeObserver: self
                                                            name: PSFeedEntriesChangedNotification
                                                          object: _currentFeed];
                                                          
        // Then, remember the new one
        _currentFeed = feed;
        
        // Reload our table view (using the new feed, or, it'll empty out if the input feed is nil)
        [self reloadEntries];
        
        // And add ourself as observer of this new feed's PSFeedEntriesChangedNotification
        if( _currentFeed )
            [[NSNotificationCenter defaultCenter] addObserver: self
                                                     selector: @selector(_entriesChanged:)
                                                         name: PSFeedEntriesChangedNotification 
                                                       object: _currentFeed];
    }
}


// Sorts the table view using the columns' sort descriptors
- (void) _sortEntryList
{
    // Remember what's selected at the moment, so that we can restore selection after the sort
    PSEntry *sel = [self selectedEntry];
    
    _entries = [[_entrySet allObjects] sortedArrayUsingDescriptors: [_table sortDescriptors]];
    [_table reloadData];
    [_table noteNumberOfRowsChanged];
    [_table setNeedsDisplay: YES];
    
    self.selectedEntry = sel;
}


// Reloads the table view with the PSEntry objects from the currentFeed
- (void) reloadEntries
{
    NSArray *entries = [_currentFeed entries];
    if( ! entries ) {
        if( _currentFeed )
            [gApp fatalError: @"Couldn't get list of entries"];
        entries = [NSArray array];
    }
    _entrySet = [[NSMutableSet alloc] initWithArray: entries];
    [self _sortEntryList];
}


// Apply updates to entries in our mutable set
- (void) updateEntries: (NSArray*)updated
{
    if( [updated count] ) {
        NSSet* updatedSet = [NSSet setWithArray: updated];
        [_entrySet minusSet: updatedSet];      // Replace old copies of entries
        [_entrySet unionSet: updatedSet];      // with new (updated) ones
    }
}


// Called when we receive a PSFeedEntriesChangedNotification
- (void) _entriesChanged: (NSNotification*)n
{
    if( [n object] != _currentFeed ) {
        // Got PSFeedEntriesChangedNotification for wrong feed!
        return;
    }
    NSLog(@"EntryListController: Entries changed in %@",_currentFeed);
    
    // Information as to what exactly changed is included in the userInfo dictionary
    NSDictionary *info = [n userInfo];
    
    // If we've removed entries, just call reloadEntries to resync our table view with the PSFeed's entries
    if( [[info objectForKey: PSFeedRemovedEntriesKey] count] )
        [self reloadEntries];
    else {
        // If we've added entries, union them with our existing set of entries
        NSArray *added = [info objectForKey: PSFeedAddedEntriesKey];
        if( added )
            [_entrySet unionSet: [NSSet setWithArray: added]];
            
        // If it's state changes for existing objects in our set of entries, just update them
        [self updateEntries: [info objectForKey: PSFeedUpdatedEntriesKey]];
        [self updateEntries: [info objectForKey: PSFeedDidChangeEntryFlagsKey]];
    }
    
    // Resort, to accomodate the changes we've just applied
    [self _sortEntryList];
}


// The currently selected entry
- (PSEntry*) selectedEntry
{
    int row = [_table selectedRow];
    return row>=0 ?[_entries objectAtIndex: row] :nil;
}


// Sets the current entry selection, or deselects all if the requested entry isn't present 
// in the table view
- (void) setSelectedEntry: (PSEntry*)entry
{
    NSUInteger row = entry ?[_entries indexOfObject: entry] :NSNotFound;
    if( row != NSNotFound ) {
        [_table selectRow: row byExtendingSelection: NO];
        [_table scrollRowToVisible: row];
    } else {
        [_table deselectAll: self];
    }
}


// Open the current entry's alternateURL in the user's default web browser
-  (IBAction) openEntryInBrowser: (id)sender
{
    NSURL *url = [[self selectedEntry] alternateURL];
    if( ! url || ! [[NSWorkspace sharedWorkspace] openURL: url] )
        NSBeep();
}


#pragma mark -
#pragma mark REDRAWING:


// If we don't already have a pending scheduled redraw, schedule a call to
// _redrawTable in 60 seconds.
- (void) _scheduleTableRedraw
{
    if( ! _scheduledTableRedraw ) {
        [self performSelector: @selector(_redrawTable)
                   withObject: nil
                   afterDelay: 60.0];
        _scheduledTableRedraw = YES;
    }
}


// Redraw our table view
- (void) _redrawTable
{
    _scheduledTableRedraw = NO;
    [_table setNeedsDisplay: YES];
}


#pragma mark -
#pragma mark TABLE VIEW DATA SOURCE:


// NSTableView datasource protocol

// We have as many rows as we have entries
- (int)numberOfRowsInTableView:(NSTableView *)tableView
{
    return [_entries count];
}


// Our table uses KVC (key-value coding) to obtain values for itself. We use this method
// to modify those values, if desired, e.g. to change the formating of a date string,
// or provide a "look" for a boolean value.
- (id)tableView:(NSTableView *)tableView objectValueForTableColumn:(NSTableColumn *)tableColumn
            row:(int)row
{
    PSEntry *entry = [_entries objectAtIndex: row];
    NSString *ident = [tableColumn identifier];
    id value = [entry valueForKey: ident];              // Use KVC to get property value
    
    if( [ident isEqualToString: @"isRead"] ) {
        if( [value boolValue]==NO )
            value = @"*";
        else if( [entry isFlagged] )
            value = @"F";
        else
            value = @"";
    } else if( [value isKindOfClass: [NSDate class]] ) {
        value = ShortDateTimeString(value);
    }
    return value;
}


// When a titleForDisplay cell in our table is going to be displayed, we'd like to bold its font if the entry
// is unread. The newest items also are color-coded.
- (void)tableView:(NSTableView *)tableView willDisplayCell:(id)cell forTableColumn:(NSTableColumn *)tableColumn row:(int)row;
{
    PSEntry *entry = [_entries objectAtIndex: row];
    NSString *ident = [tableColumn identifier];
    if( [ident isEqualToString: @"titleForDisplay"] ) {
        NSFont *font;
        NSColor *color;
        if( [entry isRead] ) {
            font = kNormalFont;
            color = nil;
        } else {
            font = kBoldFont;
            color = ActivityColorForDate([entry localDateCreated]);
        }
        [cell setFont: font];
        if( color ) {
            [cell setTextColor: color];
            // Redraw the table in a minute so the color fades
            [self _scheduleTableRedraw];
        } else {
            [cell setTextColor: [NSColor controlTextColor]];
        }
        
    } else if( [ident isEqualToString: @"dateForDisplay"] ) {
        // Dates that aren't from the feed should be grayed-out
        // (If the XML feed contained no date information, we'll use the date PubSub downloaded and stored
        // the entry into its database.)
        BOOL hasRealDate = [entry datePublished] != nil;
        [cell setTextColor: (hasRealDate ?[NSColor controlTextColor] :[NSColor disabledControlTextColor])];
    }
}


// The table view is telling us that we might want to resort by calling this. We do.
- (void)tableView:(NSTableView *)tableView sortDescriptorsDidChange:(NSArray *)oldDescriptors
{
    [self _sortEntryList];
}


// When we're notified that the table view's selection has changed, we tell our application that
// its selected entry has been set to the new selection's value.
- (void) tableViewSelectionDidChange: (NSTableView*)tableView
{
    [gApp setSelectedEntry: [self selectedEntry]];
}


@end


// Different colors indicate the newness of an entry, to convey urgency or freshness for the newest.
NSColor* ActivityColorForDate( NSDate *updatedDate )
{
    if( updatedDate ) {
        float activityLevel = 1.0 - (-[updatedDate timeIntervalSinceNow]) / kActivityThresholdTime;
        if( activityLevel > 0.0 ) {
            activityLevel =  MIN(1.0, activityLevel);
            return [NSColor colorWithCalibratedHue: 0.0 // red (green is 0.333, blue is 0.666)
                                        saturation: 1.0 
                                        brightness: activityLevel
                                             alpha: 1.0];
        }
    }
    return nil;
}