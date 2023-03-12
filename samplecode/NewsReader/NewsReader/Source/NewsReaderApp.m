/*

File: NewsReaderApp.m

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

#import "NewsReaderApp.h"
#import "FeedListController.h"
#import "EntryListController.h"
#import "EntryController.h"
#import "FeedPicker.h"
#import "ContentFormatting.h"
#import <PubSub/PubSub.h>


NewsReaderApp *gApp;


@implementation NewsReaderApp


+ (void) initialize
{
    [ShortDateTimeTransformer class];   // Register transformers
    [ErrorTransformer class];
    [ArrayTransformer class];
    [PSContentTransformer class];
}


- (void) awakeFromNib
{
    gApp = self;
    [_feedPicker setTarget: self action: @selector(feedPicked:)];
    [_hSplitter setAutosaveName: @"horiz"];
    [_vSplitter setAutosaveName: @"vert"];
}


// NSApplication delegate method called when the application has finished launching
- (void)applicationDidFinishLaunching:(NSNotification *)notification
{
    // Retain our PSClient, so that we can message it later
    _client = [PSClient applicationClient];
    if( ! _client )
        [self fatalError: @"Couldn't get PSClient!"];
        
    // Set ourselves as the PSClient delegate
    [_client setDelegate: self];
    
    // Tell the FeedListController about our PSClient
    [_feedListController setPSClient: _client];
    
    // Ensure that our preferences panel will appear onscreen, and that
    // its values will be up-to-date (in case any other application changed
    // our client's settings).
    [[_refreshIntervalPopUp window] center];
    [self _updatePreferencesPanel];
}


// NSApplication delegate method called when the application has activated
- (void)applicationDidActivate:(NSNotification *)notification
{
    // Again, just make sure that our notion of our settings are up-to-date. (See
    // note above as to why they can get out-dated.)
    [self _updatePreferencesPanel];
}


// Bad things can happen; this is the unpredictable Internet, after all!
// Displays an alert panel, and quits the application.
- (void) fatalError: (NSString*)message
{
    NSRunCriticalAlertPanel(@"Fatal Error!",
                            @"Sorry, NewsReader is not going to work: %@",
                            @"Quit",nil,nil,
                            message);
    exit(1);
}


// Returns the EntryListController for the input PSFeed. Returns nil if the input
// feed is not the our EntryListController's currentFeed.
- (EntryListController*) entryControllerForFeed: (PSFeed*)feed
{
    if( feed == _entryListController.currentFeed )
        return _entryListController;
    else
        return nil;
}


// Updates the EntryListController's notion of the current (selected) feed.
- (void) setSelectedFeed: (PSFeed*)feed
{
    if( feed != _selectedFeed ) {
        _selectedFeed = feed;
        _entryListController.currentFeed = feed;
    }
}


// Updates the EntryController's notion of the current (selected) entry.
- (void) setSelectedEntry: (PSEntry*)entry
{
    if( entry != _selectedEntry ) {
        _selectedEntry = entry;
        [_entryController setCurrentEntry: entry];
    }
}


// The currently selected feed
- (PSFeed*) selectedFeed
{
    return _selectedFeed;
}


// The currently selected entry
- (PSEntry*) selectedEntry
{
    return _selectedEntry;
}


#pragma mark -
#pragma mark PREFS:


// This method takes the input NSTimeInterval, and translates it into a value in one of
// our preferences popups, selecting the item assumed to have that value in the popup.
// See note below about the assumptions made regarding popup items and their values.
- (void) _setTimeInterval: (NSTimeInterval)interval ofPrefsPopUp: (NSPopUpButton*)popUp
{
    // Assumes items are in increasing time order, and last two are separator and "Never"
    NSMenuItem *selItem = nil;
    
    // A negative interval is a semaphore indicating "Never"
    if( interval < 0 ) {
        selItem = [popUp lastItem];
    } else {
        // Iterate through the items in the popup
        for( NSMenuItem *item in [popUp itemArray] ) {
            // Bypass separator items
            if( [item isSeparatorItem] )
                break;
            else {
                selItem = item;
                // We've got a "match" with the first item whose tag is greater than or
                // equal to our input interval
                if( [item tag] >= interval )
                    break;
            }
        }
    }
    // Change the popup's selected item to our newly identified one
    [popUp selectItem: selItem];
}


// Using our PSClient's settings, set our refreshInterval and expirationInterval
// popups in our preferences panel.
- (void) _updatePreferencesPanel
{
    PSFeedSettings *settings = [_client settings];
    NSAssert(settings!=nil,@"Couldn't get client settings");
    [self _setTimeInterval: [settings refreshInterval] 
              ofPrefsPopUp: _refreshIntervalPopUp];
    [self _setTimeInterval: [settings expirationInterval]
              ofPrefsPopUp: _expirationIntervalPopUp];
}


// Adjust the stored value for refreshInterval or expiration Interval, 
// in our PSClient's settings, according to the selection in the sender
// popup.
- (IBAction) changeIntervalPref: (id)sender
{
    PSFeedSettings *settings = [_client settings];
    NSAssert(settings!=nil,@"Couldn't get client settings");
    NSTimeInterval interval = [[sender selectedItem] tag];
    if( sender == _refreshIntervalPopUp ) {
        [settings setRefreshInterval: interval];
    } else {
        [settings setExpirationInterval: interval];
    }
    [_client setSettings: settings];

    // Changing the expiration interval affects the entries shown in the list:
    if( sender == _expirationIntervalPopUp )
        [_entryListController reloadEntries];
}


#pragma mark -
#pragma mark PSCLIENT DELEGATE:


// Called by PSClient when one or more entries in a feed have been updated
- (void) feed:(PSFeed *)feed didUpdateEntries:(NSArray *)entries
{
    // If the entry being displayed is updated, reload it
    if( [entries containsObject: [_entryController currentEntry]] )
        [_entryController reload];
}


#pragma mark -
#pragma mark ACTIONS:


// Most of the menu commands are implemented here, rather than in one of the controller
// classes, so that they'll always be enabled regardless of which view has focus.
// (The application delegate is always in the responder chain, while the other views'
// controllers only are while their view is the first responder.)


// Request an update the selected feed immediately
- (IBAction) updateNow: (id)sender
{
    PSFeed *sel = [self selectedFeed];
    if( sel ) {
        NSError *error = nil;
        if( [sel refresh: &error] )
            return; // success
        else
            NSLog(@"Couldn't refresh %@: %@",sel,error);
    }
    NSBeep();
}


// Feed Picker got a feed.
- (IBAction) feedPicked: (id)sender
{
    NSURL *url = ((FeedPicker*)sender).selectedFeedURL;
    NSLog(@"Feed picked: <%@>",url);
    if( url )
        if( ! [_feedListController addFeedWithURL: url] )
            NSBeep();
}


// Remove selected feed from the client
- (IBAction) unsubscribe: (id)sender
{
    [_feedListController delete: self];
}


// Change read/unread status of the selected entry
- (IBAction) toggleUnread: (id)sender
{
    PSEntry *entry = [self selectedEntry];
    if( entry )
        [entry setRead: ![entry isRead]];
    else
        NSBeep();
}


// Change flagged/unflagged status of the selected entry
- (IBAction) toggleFlagged: (id)sender
{
    PSEntry *entry = [self selectedEntry];
    if( entry )
        [entry setFlagged: ![entry isFlagged]];
    else
        NSBeep();
}


// Change read/unread status of all entries in the selected feed
- (IBAction) toggleAllUnread: (id)sender;
{
    PSFeed *sel = [self selectedFeed];
    if( ! sel ) {
        NSBeep();
        return;
    }
    NSArray *entries = [sel entries];
    unsigned n = [entries count];
    for( unsigned i=0; i<n; i++ )
        [[entries objectAtIndex: i] setRead: YES];
}


// Open the current entry's alternateURL in the user's default web browser
// Dispatches to the EntryListController
-  (IBAction) openEntryInBrowser: (id)sender
{
    [_entryListController openEntryInBrowser: sender];
}


// Gives us the opportunity to enable/disable user interface items, according to
// our knowledge of what their state should be. We also use this to change
// the title of menu items (e.g. read to unread, flagged to unflagged, etc.).
- (BOOL) validateUserInterfaceItem: (id<NSValidatedUserInterfaceItem>)item
{
    SEL action = [item action];
    
    // Only enable user interface items with the updateNow: and unsubscribe: actions
    // if we have a selected feed to act upon
    if( action==@selector(updateNow:) || action==@selector(unsubscribe:) )
        return [self selectedFeed]!=nil;
        
    // Similarly, only enable the ability to toggleUnread: and toggleFlagged: if we 
    // have a selected entry
    else if( action==@selector(toggleUnread:) || action==@selector(toggleFlagged:) ) {
        PSEntry *entry = [self selectedEntry];
        if( ! entry )
            return NO;
            
        // Check the selected entry's current read or flagged state, and update
        // the user interface item's title as appropriate (to a string that would
        // indicate a change in state)
        NSString *title;
        if( action==@selector(toggleUnread:) )
            title = ([entry isRead] ?@"Mark As Unread" :@"Mark As Read");
        else
            title = ([entry isFlagged] ?@"Mark As Unflagged" :@"Mark As Flagged");
        [(NSMenuItem*)item setTitle: title];
        
        return YES;
    } 
    
    // Only enable user interface items with the openEntryInBrowser: action if the 
    // selected entry has an alternateURL (which is used as the url to open)
    else if( action==@selector(openEntryInBrowser:) )
        return [[self selectedEntry] alternateURL] != nil;
    else
        return YES;
}


@end
