/*

File: FeedListController.m

Abstract: Use the PubSub framework to create a simple RSS news reader.

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

Copyright © 2006-2007 Apple Inc. All Rights Reserved.

*/

#import "FeedListController.h"
#import "NewsReaderApp.h"
#import <PubSub/PubSub.h>
#import <PubSub/PSLink.h>


#define WebURLsWithTitlesPboardType @"WebURLsWithTitlesPboardType"


@implementation FeedListController


static NSFont *kNormalFont, *kBoldFont;


- (void) awakeFromNib
{
    // Start out with an empty array of feeds
    _feeds = [[NSMutableArray alloc] init];
    
    // Register our outline view for a couple of pasteboard types that we'd like to accept drags for
    [_outline registerForDraggedTypes: [NSArray arrayWithObjects: NSURLPboardType,
                                                                  WebURLsWithTitlesPboardType,
                                                                  nil]];
    
    // We're going to swap back and forth between these fonts later, depending upon what's happening
    // with the feed that's being displayed. Save these off.
    kNormalFont = [[[[_outline tableColumns] objectAtIndex: 0] dataCell] font];
    kBoldFont = [[NSFontManager sharedFontManager] convertFont: kNormalFont toHaveTrait: NSBoldFontMask];

    // Add ourself as an observer for a couple of notifications sent by PSFeed objects
    [[NSNotificationCenter defaultCenter] addObserver: self selector: @selector(feedRefreshing:)
                                                 name: PSFeedRefreshingNotification object: nil];
    [[NSNotificationCenter defaultCenter] addObserver: self selector: @selector(feedChanged:)
                                                 name: PSFeedEntriesChangedNotification object: nil];
}


- (void) finalize
{
    [[NSNotificationCenter defaultCenter] removeObserver: self];
    [super finalize];
}


// Allows callers to set our knowledge of our PSClient
- (void) setPSClient: (PSClient*)client;
{
    _client = client;
    
    // In case the new client means a change to what's in our list, update it
    [self updateFeedList];
}


// A comparator used to position feeds in our list
static NSInteger compareFeeds( id a, id b, void *context )
{
    NSArray *_feeds = context;
    // Compare based on current positions in _feeds.
    // New feeds go at the end, in alphabetical order.
    NSUInteger aRow = [_feeds indexOfObject: a];
    NSUInteger bRow = [_feeds indexOfObject: b];
    if( aRow != NSNotFound ) {
        if( bRow != NSNotFound )
            return a-b;
        else
            return -1;
    } else {
        if( bRow != NSNotFound )
            return 1;
        else
            return [[a title] localizedCaseInsensitiveCompare: [b title]];
    }
}


// Update our feed list to reflect our PSClient's current status
- (void) updateFeedList
{
    NSAssert(_client,@"No _client set");
    
    // Remember what's currently selected, and reset the selection to this 
    // after the update has happened (last line, below)
    PSFeed *sel = [self selectedFeed];
    
    NSArray *curFeeds = [[_client feeds] sortedArrayUsingFunction: &compareFeeds
                                                          context: _feeds];
    if( ! curFeeds )
        [gApp fatalError: @"Couldn't get list of feeds"];
    [_feeds setArray: curFeeds];
    [_outline noteNumberOfRowsChanged];
    [_outline reloadData];
    
    self.selectedFeed = sel;
}


// Locate the input PSFeed in our outline view, returning the row number
- (int) _rowOfFeed: (PSFeed*)feed
{
    return feed ?[_outline rowForItem: feed] :-1;
}


// Redraw the outline view cell for this feed
- (void) redrawFeed: (PSFeed*)feed
{
    int row = [self _rowOfFeed: feed];
    if( row >= 0 ) {
        [_outline reloadItem: feed];
        [_outline setNeedsDisplayInRect: [_outline rectOfRow: row]];
    }
}


// The currently selected feed
- (PSFeed*) selectedFeed
{
    int row = [_outline selectedRow];
    if( row >= 0 )
        return [_outline itemAtRow: row];
    else
        return nil;
}


// Sets the current feed selection, or deselects all if
// the requested feed isn't present in the outline view
- (void) setSelectedFeed: (PSFeed*)feed
{
    int row = [self _rowOfFeed: feed];
    if( row >= 0 ) {
        [_outline selectRow: row byExtendingSelection: NO];
        [gApp setSelectedFeed: feed];
    } else {
        [_outline deselectAll: self];
        [gApp setSelectedFeed: nil];
    }
}


// Inserts a new feed url into the outline view, at the input row.
// Calls -[PSClient addFeedWithURL:] to add the new feed to the client too.
- (BOOL) insertFeedURL: (NSURL*)url atRow: (int)row
{
    PSFeed *feed = [_client addFeedWithURL: url];
    if( ! feed )
        return NO;
    if( [_feeds containsObject: feed] )
        return NO;
    
    [_feeds insertObject: feed atIndex: row];
    [_outline noteNumberOfRowsChanged];
    [_outline reloadData];
    self.selectedFeed = feed;
    
    // Watch when the new feed finishes refreshing, to check whether it's actually an HTML page:
    if( [[feed entries] count] == 0 ) {
        [[NSNotificationCenter defaultCenter] addObserver: self selector: @selector(feedFirstRefresh:)
                                                 name: PSFeedRefreshingNotification object: feed];
    }

    return YES;
}


// Allows for insertion of multiple urls at once. See description
// of -[FeedListController insertFeedURL:atRow:] for more information.
- (BOOL) insertFeedURLs: (NSArray*)urls atRow: (int)index
{
    if( index < 0 )
        index = [_feeds count];
    BOOL any = NO;
    for( unsigned i=0; i<[urls count]; i++ ) {
        NSString *urlStr = [urls objectAtIndex: i];
        if( [urlStr length]==0 )
            continue;
        NSURL *url = [NSURL URLWithString: urlStr];
        if( url && [self insertFeedURL: url atRow: index] ) {
            index++;
            any = YES;
        }
    }
    return any;
}


- (BOOL) addFeedWithURL: (NSURL*)url
{
    NSParameterAssert(url);
    int row = [_outline selectedRow];
    if( row < 0 )
        row = [_feeds count];
    return [self insertFeedURL: url atRow: row];
}


#pragma mark -
#pragma mark REDRAWING:


// If we don't already have a pending scheduled redraw, schedule a call to
// _redrawOutline in 60 seconds.
- (void) _scheduleOutlineRedraw
{
    if( ! _scheduledOutlineRedraw ) {
        [self performSelector: @selector(_redrawOutline)
                   withObject: nil
                   afterDelay: 60.0];
        _scheduledOutlineRedraw = YES;
    }
}


// Redraw our outline view
- (void) _redrawOutline
{
    _scheduledOutlineRedraw = NO;
    [_outline setNeedsDisplay: YES];
}


#pragma mark -
#pragma mark NOTIFICATIONS:


// A notification sent by PSFeed objects. We listen for this so that we
// can update the feed's display in our outline view.
- (void) feedChanged: (NSNotification*)n
{
    PSFeed *feed = [n object];
    [self redrawFeed: feed];
}


// A feed has started or stopped refreshing.
- (void) feedRefreshing: (NSNotification*)n
{
    PSFeed *feed = [n object];
    NSLog(@"%@ refreshing=%i", feed,feed.refreshing);
    [self redrawFeed: feed];
}


// A feed that was just subscribed to has started/stopped refreshing:
- (void) feedFirstRefresh: (NSNotification*)n
{
    PSFeed *feed = [n object];
    if( ! [feed isRefreshing] ) {
        [[NSNotificationCenter defaultCenter] removeObserver: self 
                                                name: PSFeedRefreshingNotification
                                                object: feed];
        NSError *error = [feed lastError];
        if( [[error domain] isEqualToString: PSErrorDomain] && [error code]==PSNotAFeedError ) {
            // The data at that URL is not a feed. Try to autodiscover a link to a feed:
            PSLink *bestLink = nil;
            for( PSLink *link in [feed links] ) {
                switch( [link linkKind] ) {
                    case PSLinkToAtom:
                        if( bestLink==nil || [bestLink linkKind]==PSLinkToRSS )
                            bestLink = link;
                        break;
                    case PSLinkToRSS:
                        if( bestLink==nil )
                            bestLink = link;
                        break;
                    default:
                        break;
                }
            }
            
            if( bestLink ) {
                // Replace the current 'feed' with the real one:
                PSFeed *newFeed = [_client addFeedWithURL: [bestLink URL]];
                if( newFeed && newFeed != feed ) {
                    [_client removeFeed: feed];
                    [self updateFeedList];
                    self.selectedFeed =  newFeed;
                }
            }
        }
    }
}


#pragma mark -
#pragma mark OUTLINE VIEW:


// NSOutlineView datasource protocol

// Our feeds have no children, so this returns the input item, or if that's nil,
// returns the item at the input index in the outline view.
- (id)outlineView:(NSOutlineView *)outlineView child:(int)index ofItem:(id)item
{
    return item ?nil :[_feeds objectAtIndex: index];
}


// The item is not expandable only if it's nil.
- (BOOL)outlineView:(NSOutlineView *)outlineView isItemExpandable:(id)item
{
    return (item==nil);
}


// Feeds themselves have no children, but the outline view itself has all of its
// feeds as children.
- (int)outlineView:(NSOutlineView *)outlineView numberOfChildrenOfItem:(id)item
{
    return item ?0 :[_feeds count];
}


// Our outline has three columns with identifiers: title, status, and unreadCount. 
// A value in the title column is set using the PSFeed title, or, if unavailable, the absoluteString of the PSFeed URL.
// A value in the status column is set to "@" if the feed is currently refreshing, or "!" if there's been an error refreshing the feed.
// A value in the unreadCount column is set to an NSNumber indicating the PSFeed unreadCount.
- (id)outlineView:(NSOutlineView *)outlineView objectValueForTableColumn:(NSTableColumn *)tableColumn byItem:(id)item
{
    PSFeed *feed = item;
    NSString *identifier = [tableColumn identifier];
    if( [identifier isEqualToString: @"title"] ) {
        NSString *title = [feed title];
        if( ! [title length] )
            title = [[feed URL] absoluteString];
        return title;
    }else if( [identifier isEqualToString: @"status"] ) {
        if( [feed isRefreshing] )
            return @"@";
        else if( [feed lastError] )
            return @"!";
    }else if( [identifier isEqualToString: @"unreadCount"] ) {
        int unread = [feed unreadCount];
        if( unread > 0 )
            return [NSNumber numberWithInt: unread];
    }
    return nil;
}


// When a cell in our outline view is going to be displayed, we'd like to bold its font if it has unread entries,
// or use the normal font if not.
- (void)outlineView:(NSOutlineView *)outlineView willDisplayCell:(id)cell forTableColumn:(NSTableColumn *)tableColumn item:(id)item
{
    PSFeed *feed = item;
    if( [feed unreadCount] > 0 )
        [cell setFont: kBoldFont];
    else
        [cell setFont: kNormalFont];
}


// Our outline view's columns are not editable
- (BOOL)outlineView:(NSOutlineView *)outlineView shouldEditTableColumn:(NSTableColumn *)tableColumn item:(id)item;
{
    return NO;
}


// Our outline view does not provide menus for its items
- (NSMenu*) outlineView: (NSOutlineView *)outlineView menuForItem: (id)item
{
    return [outlineView menu];
}


// When we're notified that the outline view's selection has changed, we tell our application that
// its selected feed has been set to the new selection's value.
- (void)outlineViewSelectionDidChange:(NSNotification *)notification
{
    [gApp setSelectedFeed: [self selectedFeed]];
}


#pragma mark -
#pragma mark DRAG & DROP:


// Extract an NSArray of urls upon receipt of a (drag and) drop
- (NSArray*) _URLsFromDrag: (id<NSDraggingInfo>)info
{
    NSPasteboard *pb = [info draggingPasteboard];
    id value = [pb propertyListForType: WebURLsWithTitlesPboardType];
    if( value ) {
        return [value objectAtIndex: 0];                                // First item is array of URLs
    } else {
        NSArray *contents = [pb propertyListForType: NSURLPboardType];
        return [contents subarrayWithRange: NSMakeRange(0,1)];          // Only 1st item contains a URL
    }
}    


// This gives us a change to validate a drop that is about to occur. Depending
// upon what's being dropped, we change the proposedChildIndex.
- (NSDragOperation)outlineView:(NSOutlineView *)outlineView 
                  validateDrop:(id <NSDraggingInfo>)info 
                  proposedItem:(id)item
            proposedChildIndex:(int)index
{
    if( item != nil )
        index = [_feeds indexOfObject: item];
    else if( index < 0 )
        index = [_feeds count];
    [outlineView setDropItem: nil dropChildIndex: index];
    return NSDragOperationCopy;
}


// When we accept a drop, we extract any/all urls from the pasteboard info, and
// insert new feed urls for them into our outline view.
- (BOOL)outlineView:(NSOutlineView *)outlineView 
         acceptDrop:(id <NSDraggingInfo>)info 
               item:(id)item 
         childIndex:(int)index
{
    if( item != nil )
        return NO;
    NSArray *urls = [self _URLsFromDrag: info];
    return [urls count]>0 && [self insertFeedURLs: urls atRow: index];
}

#pragma mark -
#pragma mark ACTIONS:


// Remove the currently selected feed, from the outline view, and from our PSClient
- (IBAction) delete: (id)sender
{
    PSFeed *sel = [self selectedFeed];
    if( sel && [_client removeFeed: sel] )
        [self updateFeedList];
    else
        NSBeep();
}


@end
