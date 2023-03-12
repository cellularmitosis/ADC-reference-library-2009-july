/*

File: EntryController.m

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

#import "EntryController.h"
#import <PubSub/PubSub.h>
#import <PubSub/PSEnclosure.h>
#import <WebKit/WebKit.h>


@interface EntryController (Private)
- (NSString*) currentHTML;
@end


@implementation EntryController


// The current entry
- (PSEntry*) currentEntry
{
    return _entry;
}


// Sets the current entry
- (void) setCurrentEntry: (PSEntry*)entry
{
    if( entry!=_entry && ! [entry isEqual: _entry] ) {   // Don't compare entries with ==
        _entry = entry;
        [self reload];
        
        // If this entry is displayed for 1 second, mark it read:
        // This is a simple mechanism to ensure that the user really intends to mark-as-read
        // when they're changing the selection in the table view (e.g. by arrowing up and down).
        if( _entry && ! [_entry isRead] )
            [self performSelector: @selector(_markRead:) withObject: _entry afterDelay: 1.0];
    }
}


// If the input PSEntry is equal to our current entry, change its status (using the PubSub API) to read
- (void) _markRead: (PSEntry*)entry
{
    if( [entry isEqual: _entry] )
        [_entry setRead: YES];
}


#pragma mark -
#pragma mark HTML GENERATION:


- (NSString*) enclosureHTML: (PSEnclosure*)encl
{
    static NSString *sTemplate;
    if( ! sTemplate ) {
        NSString *path = [[NSBundle mainBundle] pathForResource: @"EnclosureTemplate" ofType: @"html"];
        sTemplate = [[NSString alloc] initWithContentsOfFile: path];
        NSAssert(sTemplate,@"Can't load EnclosureTemplate.html");
    }

    NSString *urlStr = [[encl URL] absoluteString];
    NSString *name = [[[encl URL] path] lastPathComponent];
    NSString *type = [encl MIMEType];
    if( ! type )
        type = @"";
    long long length = [encl length];
    NSString *lengthStr = length ?[NSString stringWithFormat: @"(%.0f KB)", length/1024.0]
                                 : @"";
    
    NSMutableString *html = [[sTemplate mutableCopy] autorelease];
    [html replaceOccurrencesOfString: @"{URL}" withString: urlStr
                                options: 0 range: NSMakeRange(0,[html length])];
    [html replaceOccurrencesOfString: @"{NAME}" withString: name
                                options: 0 range: NSMakeRange(0,[html length])];
    [html replaceOccurrencesOfString: @"{TYPE}" withString: type
                                options: 0 range: NSMakeRange(0,[html length])];
    [html replaceOccurrencesOfString: @"{LENGTH}" withString: lengthStr
                                options: 0 range: NSMakeRange(0,[html length])];
    return html;
}

- (NSString*) enclosuresHTML
{
    NSMutableString *html = [NSMutableString string];
    for( PSEnclosure *encl in [_entry enclosures] )
        [html appendString: [self enclosureHTML: encl]];
    return html;
}


// The HTML representation of the current entry
- (NSString*) currentHTML
{
    // Without an entry, use the empty string
    if( ! _entry )
        return @"";
    
    // Get the title, if any. Entries don't have to have titles, but usually do.
    NSString *title = [_entry title];
    if( ! title ) title = @"";
    
    // Get the content, if any. Some entries only have a summary, some have nothing.
    NSString *content = [[_entry content] HTMLString];
    if( ! content )
        content = [[_entry summary] HTMLString];
    if( ! content ) content = @"";
    
    // Get the alternateURL (sometimes also known as "permalink"). This is almost always
    // the url of the entry's web page.
    NSString *link = [[_entry alternateURL] absoluteString];
    if( ! link ) link = @"";
    
    // Set the <base> value in the HTML, so that relative URLs in the content
    // will be interpreted correctly.
    NSString *base = [[_entry baseURL] absoluteString];
    if( ! base )
        base = link;
    if( ! base )
        base = [[[_entry feed] alternateURL] absoluteString];

    // Generate HTML for any enclosures:
    NSString *encls = [self enclosuresHTML];

    // There follows a very dumb (but easy!) HTML templating engine:
    // We plug the above values in to our template HTML, stored in our application's bundle.
    static NSString *sTemplate, *sNoLinkTemplate;
    
    // First, grab our templates out of our application's Resources directory
    if( ! sTemplate ) {
        NSString *path = [[NSBundle mainBundle] pathForResource: @"EntryTemplate" ofType: @"html"];
        sTemplate = [[NSString alloc] initWithContentsOfFile: path];
        NSAssert(sTemplate,@"Can't load EntryTemplate.html");
        
        path = [[NSBundle mainBundle] pathForResource: @"EntryTemplate_NoLink" ofType: @"html"];
        sNoLinkTemplate = [[NSString alloc] initWithContentsOfFile: path];
        NSAssert(sNoLinkTemplate,@"Can't load EntryTemplate.html");
    }
    
    NSString *template = [link length] ?sTemplate :sNoLinkTemplate;
    NSMutableString *html = [[template mutableCopy] autorelease];
    
    // Now, replace some constants that occur in the templates with our entry's data
    [html replaceOccurrencesOfString: @"{BASE}" withString: base
    options: 0 range: NSMakeRange(0,[html length])];
    [html replaceOccurrencesOfString: @"{LINK}" withString: link
    options: 0 range: NSMakeRange(0,[html length])];
    [html replaceOccurrencesOfString: @"{TITLE}" withString: title
    options: 0 range: NSMakeRange(0,[html length])];
    [html replaceOccurrencesOfString: @"{CONTENT}" withString: content
    options: 0 range: NSMakeRange(0,[html length])];
    [html replaceOccurrencesOfString: @"{ENCLOSURES}" withString: encls
    options: 0 range: NSMakeRange(0,[html length])];
    return html;
}


#pragma mark -
#pragma mark LOADING:


// Set the object value of the status text field to the input string
- (void) _setStatus: (NSString*)status
{
    [_statusField setObjectValue: status];
}


// Reloads the entry into the WebView
- (void) reload
{
    [self _setStatus: nil];
    [[_webView mainFrame] loadHTMLString: [self currentHTML] baseURL: [_entry alternateURL]];
}


// Start animating our progress indicator, and set our status field accordingly
- (void) startedLoading
{
    _loading = YES;
    [_loadingIndicator startAnimation: self];
    [self _setStatus: @"Loading..."];
}


// Stop animating our progress indicator, and set our status field accordingly (to nil
// if no error occurred, or to an error string if we've encountered an error)
- (void) stoppedLoading: (NSError*)error
{
    _loading = NO;
    NSString *status = nil;
    if( error )
        status = [@"Error: " stringByAppendingString: [error localizedDescription]];
    [_loadingIndicator stopAnimation: self];
    [self _setStatus: status];
}


#pragma mark -
#pragma mark WEBVIEW DELEGATES:


// WebView delegate methods

// Used to indicate that we've started loading (so that we can update our progress indicator
// and status text field)
- (void)webView:(WebView *)sender didStartProvisionalLoadForFrame:(WebFrame *)frame
{
    if( frame == [_webView mainFrame] )
        [self startedLoading];
}


// Used to indicate that we've finished loading (so that we can update our progress indicator
// and status text field)
- (void)webView:(WebView *)sender didFinishLoadForFrame:(WebFrame *)frame
{
    if( frame == [_webView mainFrame] )
        [self stoppedLoading: nil];
}


// Used to indicate that we've encountered an error during loading (so that we can update our 
// progress indicator and status text field)
- (void)webView:(WebView *)sender didFailLoadWithError:(NSError *)error forFrame:(WebFrame *)frame
{
    NSLog(@"EntryController: Failed load: %@",error);
    if( frame == [_webView mainFrame] )
        [self stoppedLoading: error];
}


// Also used to indicate that we've encountered an error during loading (so that we can update our 
// progress indicator and status text field)
- (void)webView:(WebView *)sender didFailProvisionalLoadWithError:(NSError *)error forFrame:(WebFrame *)frame
{
    NSLog(@"EntryController: Failed provisional load: %@",error);
    if( frame == [_webView mainFrame] )
        [self stoppedLoading: error];
}


// We simply log to Console when we've encountered a policy implementation failure within a WebFrame.
// (You might want to do more.)
- (void)webView:(WebView *)sender unableToImplementPolicyWithError:(NSError *)error frame:(WebFrame *)frame
{
    NSLog(@"EntryController: %s %@",_cmd,error);
}


// We simply log to Console when we've encountered an error loading a (sub)resource within the WebView.
// (You might want to do more.)
- (void)webView:(WebView *)sender 
       resource:(id)identifier 
        didFailLoadingWithError:(NSError *)error
 fromDataSource:(WebDataSource *)dataSource
{
    NSLog(@"EntryController: Failed to load <%@> -- %@",
        [[error userInfo] objectForKey: NSErrorFailingURLStringKey],
        [error localizedDescription]);
}


// Track the mouse to handle roll-over for links
// This allows us to set our status text field to something informative when the user's cursor
// is over a link.
- (void)webView:(WebView *)sender 
        mouseDidMoveOverElement:(NSDictionary *)elementInformation 
        modifierFlags:(unsigned int)modifierFlags
{
    if( ! _loading ) {
        NSString *status = nil;
        NSURL *link = [elementInformation objectForKey: WebElementLinkURLKey];
        if( link ) {
            WebFrame *target = [elementInformation objectForKey: WebElementLinkTargetFrameKey];
            // If the Command key is held down, the link will open in the web browser:
            if( (modifierFlags & NSCommandKeyMask) || target != [_webView mainFrame] )
                status = @"Open \"%@\" in browser";
            else
                status = @"Go to \"%@\"";
            status = [NSString stringWithFormat: status, [link absoluteString]];
        }
        [self _setStatus: status];
    }
}


// Used to send a URL to the user's default web browser. If they're holding down the <Shift> key,
// we'll send the URL without activating the receiving application.
- (BOOL) _sendBrowserRequest: (NSURLRequest*)request forAction: (NSDictionary*)actionInformation
{
    NSWorkspaceLaunchOptions options = NSWorkspaceLaunchDefault;
    unsigned modifiers = [[actionInformation objectForKey: WebActionModifierFlagsKey] unsignedIntValue];
    if( (modifiers & NSShiftKeyMask) )
        options |= NSWorkspaceLaunchWithoutActivation;
    BOOL ok = [[NSWorkspace sharedWorkspace] openURLs: [NSArray arrayWithObject: [request URL]]
                              withAppBundleIdentifier: nil
                                              options: options
                       additionalEventParamDescriptor: nil
                                    launchIdentifiers: NULL];
    if( ! ok )
        NSBeep();
    return ok;
}


// Intercept link clicks, and send them to the web browser if the user Cmd-clicked
- (void)webView:(WebView *)sender decidePolicyForNavigationAction:(NSDictionary *)actionInformation 
        request:(NSURLRequest *)request 
          frame:(WebFrame *)frame
         decisionListener:(id<WebPolicyDecisionListener>)listener
{
    WebNavigationType navType = [[actionInformation objectForKey: WebActionNavigationTypeKey] intValue];
    unsigned modifiers = [[actionInformation objectForKey: WebActionModifierFlagsKey] unsignedIntValue];
    if( navType == WebNavigationTypeLinkClicked && (modifiers & NSCommandKeyMask) ) {
        [self _sendBrowserRequest: request forAction: actionInformation];
        [listener ignore];
    } else {
        [listener use];
    }
}


// Intercept link clicks destined for new windows, and send them to the user's default web browser
- (void)webView:(WebView *)sender decidePolicyForNewWindowAction:(NSDictionary *)actionInformation
        request:(NSURLRequest *)request 
   newFrameName:(NSString *)frameName 
        decisionListener:(id<WebPolicyDecisionListener>)listener
{
    [self _sendBrowserRequest: request forAction: actionInformation];
    [listener ignore];
}


@end
