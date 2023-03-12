/*

File: FeedPicker.m

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

#import "FeedPicker.h"
#import <PubSub/PubSub.h>
#import <ApplicationServices/ApplicationServices.h>


static NSString* NameOfAppWithBundleID( NSString* bundleID )
{
    FSRef appRef;
    OSStatus err = LSFindApplicationForInfo(kLSUnknownType,(CFStringRef)bundleID,NULL,
                                            &appRef,NULL);
    if( ! err ) {
        NSString *name = nil;
        err = LSCopyDisplayNameForRef(&appRef,(CFStringRef*)&name);
        if( ! err )
            return [name autorelease];
    }
    return nil;
}


static NSURL* SmartURLFromString( NSString *str, NSArray *allowedSchemes )
{
    NSURL *url = [NSURL URLWithString: str];
    if( url ) {
        NSString *scheme = [url scheme];
        if( scheme == nil ) {
            str = [@"http://" stringByAppendingString: str];
            url = [NSURL URLWithString: str];
            scheme = [url scheme];
        }
        if( !allowedSchemes || [allowedSchemes containsObject: scheme] )
            if( [[url host] length] && [url path]!=nil )
                return url;
    }
    return nil;
}


@implementation FeedPicker


@synthesize selectedFeedURL = _selectedFeedURL;


- (void) setTarget: (id)target action: (SEL)action
{
    _target = target;
    _action = action;
}


- (void) open: (id)sender
{
    _clientIdentifiers = [[PSClient allClientBundleIdentifiers] mutableCopy];
    [_clientIdentifiers removeObject: [[PSClient applicationClient] signature]];

    [_browser setDoubleAction: @selector(_rowDoubleClicked:)];
    [_browser reloadColumn: 0];

    [_panel center];
    [_panel makeKeyAndOrderFront: self];
}


- (NSURL*) selectedFeedURL
{
    return SmartURLFromString([_URLField stringValue],
                              [NSArray arrayWithObjects: @"http",@"https",@"file",@"feed",nil]);
}

- (void) setSelectedFeedURL: (NSURL*)url
{
    [_URLField setObjectValue: url];
}


- (BOOL) _validateURL
{
    // Grab the new url from the text field in the sheet
    // If the URL's not valid, eep, and prevent the sheet from closing, so that the user has
    // a chance to correct their input.
    if( self.selectedFeedURL )
        return true;
    else {
        [_panel makeFirstResponder: _URLField];
        NSBeep();
        return false;
    }
}


- (IBAction) dismiss: (id)sender
{
    if( sender == _cancelButton )
        self.selectedFeedURL = nil;
    else if( ! [self _validateURL] )
        return;
        
    [_panel close];
}

- (BOOL) windowShouldClose: (id)sender
{
    // Close box is same as Cancel.
    self.selectedFeedURL = nil;
    return YES;
}

- (void) windowWillClose: (NSNotification*)n
{
    _clientIdentifiers = nil;
    
    // Tell target we're done
    [_target performSelector: _action withObject: self];
}


static NSInteger compareFeedTitles( id a, id b, void* context )
{
    NSString *aTitle = [a title] ?: [[a URL] absoluteString];
    NSString *bTitle = [b title] ?: [[b URL] absoluteString];
    return [aTitle localizedCaseInsensitiveCompare: bTitle];
}
    
    
- (void)browser:(NSBrowser *)browser createRowsForColumn:(NSInteger)column inMatrix:(NSMatrix *)matrix
{
    int i=0;
    NSString *bundleID, *name;
    NSBrowserCell *cell;
    
    if( column == 0 ) {
        [matrix renewRows: [_clientIdentifiers count] columns: 1];
        for( bundleID in _clientIdentifiers ) {
            name = NameOfAppWithBundleID(bundleID);
            if( ! name )
                name = bundleID;
            cell = [matrix cellAtRow: i++ column: 0];
            [cell setStringValue: name];
            [cell setLeaf: NO];
        }
        
    } else {
        int sel = [browser selectedRowInColumn: 0];
        bundleID = [_clientIdentifiers objectAtIndex: sel];
        NSArray *feeds = [[PSClient clientForBundleIdentifier: bundleID] feeds];
        feeds = [feeds sortedArrayUsingFunction: &compareFeedTitles context: NULL];
        [matrix renewRows: [feeds count] columns: 1];
        for( PSFeed *feed in feeds ) {
            name = [feed title];
            if( ! name )
                name = [[feed URL] absoluteString];
            cell = [matrix cellAtRow: i++ column: 0];
            [cell setStringValue: name];
            [cell setLeaf: YES];
            [cell setRepresentedObject: [feed URL]];
        }
    }
}


- (IBAction) rowSelected: (id)sender
{
    NSBrowserCell *cell = [_browser selectedCellInColumn: 1];
    [_URLField setObjectValue: [cell representedObject]];
    [_okButton setEnabled: (self.selectedFeedURL != nil)];
    NSLog(@"FeedPicker: selected <%@>",self.selectedFeedURL);
}

- (IBAction) _rowDoubleClicked: (id)sender
{
    if( self.selectedFeedURL )
        [self dismiss: self];
    else
        NSBeep();
}


- (void) controlTextDidChange: (NSNotification*)n
{
    if( [n object] == _URLField ) {
        // We shouldn't ask for the field's stringValue here, because this gets called on
        // every keystroke, and if the user's in the middle of typing a multi-key sequence
        // (like in a Japanese input method) the -stringValue method will abort the sequence.
        // Instead, just look at the field editor's text directly to see if it's non-empty.
        NSTextView *fieldEditor = [[n userInfo] objectForKey: @"NSFieldEditor"];
        [_okButton setEnabled: [[fieldEditor textStorage] length] > 0];
    }
}
    

@end
