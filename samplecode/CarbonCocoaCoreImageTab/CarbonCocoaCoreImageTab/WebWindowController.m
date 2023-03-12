/*
	File:		WebWindowController.m
 
	Abstract:	Contains the implementation of our Cocoa based controller class,
	WebWindowController.  DisplayCocoaWebWindow(), called from our Carbon based code,
	is responsible for creating an instance of our WebWindowController class, and
	hooking up our object graph to the instance.
 
	Version:	1.0
 
	Disclaimer:	IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc.
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
 
	Copyright © 2006 Apple Computer, Inc., All Rights Reserved
 */


#import "WebWindowController.h"

@implementation WebWindowController

OSStatus	DisplayCocoaWebWindow( CFURLRef urlRef )
{
	WebWindowController	*controller	= [[WebWindowController alloc] init];		//	We instantiate ourselves, and link the object graph created from the nib in the init method.
	
	[controller showWindow:nil];
	[controller loadWebPage:(NSURL*)urlRef];
	
	return( noErr );
}

- (NSString *)windowNibName
{
    return @"WebWindow";
}

- (void)loadWebPage:(NSURL *)url															//	Given a url, load the web page
{
	WebView	*webView	= [self currentWebView];
	[[webView mainFrame] loadRequest:[NSURLRequest requestWithURL:url]];
}


- (void)tabView:(NSTabView *)tabView didSelectTabViewItem:(NSTabViewItem *)tabViewItem
{
	WebView		*webView	= [self currentWebView];
	NSString	*urlString	= [[[[[webView mainFrame] dataSource] request] URL] absoluteString];
	NSString	*title		= [[[webView mainFrame] dataSource] pageTitle];
	
	( urlString != nil ) ? [urlTextField setStringValue:urlString] : [urlTextField setStringValue:@""];
	( title != nil ) ? [[self window] setTitle:title] : [[self window] setTitle:@"Untitled"];
}

- (void)webView:(WebView *)sender didStartProvisionalLoadForFrame:(WebFrame *)frame
{
	if (frame == [[self currentWebView] mainFrame])		//	Only report feedback for the main frame.
	{													//	We update the urlTextField to match the current main frame
		[urlTextField setStringValue:[[[[frame provisionalDataSource] request] URL] absoluteString]];
    }
}

- (void)webView:(WebView *)sender didReceiveTitle:(NSString *)title forFrame:(WebFrame *)frame
{
	if (frame == [[self currentWebView] mainFrame])		//	Only report feedback for the main frame.
	{
		[[self window] setTitle:title];
	}
}


- (IBAction)urlToPage:(id)sender
{
	WebView	*webView	= [self currentWebView];
	[[webView mainFrame] loadRequest:[NSURLRequest requestWithURL:[NSURL URLWithString:[urlTextField stringValue]]]];
}


- (WebView*) currentWebView
{
	NSView		*view;
	unsigned	i;
	
	NSTabViewItem	*selectedTabItem	= [tabView selectedTabViewItem];
	NSArray	*subViews					= [[selectedTabItem view] subviews];
	
	for ( i = 0 ; i < [subViews count] ; i++ )
	{
		view	= [subViews objectAtIndex:i];
		if ( [view isKindOfClass:[WebView class]] == YES )	return( (WebView*)view );
	}
	
	return( nil );
}

@end










