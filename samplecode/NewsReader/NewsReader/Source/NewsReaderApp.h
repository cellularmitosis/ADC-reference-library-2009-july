/*

File: NewsReaderApp.h

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

#import <Cocoa/Cocoa.h>
@class PSClient, PSFeed, PSEntry;
@class FeedListController, EntryListController, EntryController, FeedPicker;


// NSApplication delegate, and PSClient delegate
@interface NewsReaderApp : NSObject 
{
    IBOutlet NSWindow *_window;                         // The main window
    IBOutlet FeedListController *_feedListController;   // Controls the outline view of feeds
    IBOutlet EntryListController *_entryListController; // Controls the table view of entries 
    IBOutlet EntryController *_entryController;         // Controls the html display of an individual entry
    IBOutlet FeedPicker *_feedPicker;
    
    IBOutlet NSPopUpButton *_refreshIntervalPopUp,      // Preferences, to set how often feeds are periodically updated, and
                           *_expirationIntervalPopUp;   // how long before entries in feeds are expired (and so, removed)
    IBOutlet NSSplitView *_hSplitter, *_vSplitter;      // The horizontal/vertical splitters
    
    PSClient *_client;                                  // The PSClient is this application
    PSFeed *_selectedFeed;                              // The currently selected feed
    PSEntry *_selectedEntry;                            // The currently selected entry
}

- (PSFeed*) selectedFeed;
- (PSEntry*) selectedEntry;

- (void) fatalError: (NSString*)message;                // Bad things can happen; this is the unpredictable Internet, after all!

- (IBAction) updateNow: (id)sender;                     // Request an update the selected feed immediately
- (IBAction) feedPicked: (id)sender;                    // Sent by FeedPicker when user picks a feed
- (IBAction) unsubscribe: (id)sender;                   // Remove selected feed from the client
- (IBAction) toggleUnread: (id)sender;                  // Change read/unread status of the selected entry
- (IBAction) toggleFlagged: (id)sender;                 // Change flagged/unflagged status of the selected entry
- (IBAction) toggleAllUnread: (id)sender;               // Change read/unread status of all entries in the selected feed
- (IBAction) openEntryInBrowser: (id)sender;            // Open the current entry's alternateURL in the user's default web browser

- (IBAction) changeIntervalPref: (id)sender;            // The action for either of our above preferences, to change their values.

// Only FeedListController and EntryListController should call these:
- (void) setSelectedFeed: (PSFeed*)feed;
- (void) setSelectedEntry: (PSEntry*)entry;

// Private:
- (void) _updatePreferencesPanel;

@end

extern NewsReaderApp *gApp;

