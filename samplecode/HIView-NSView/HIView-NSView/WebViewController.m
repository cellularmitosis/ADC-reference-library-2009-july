/*
	WebViewController.m
	
	This is the NSViewController that manages the the UI for the webview.
	All controls in this view are  embedded inside the HICocoaView.  

	v1.5
	
	Copyright (c) 2006-2007, Apple Inc., all rights reserved.
*/

/*
 IMPORTANT:  This Apple software is supplied to you by Apple Inc. ("Apple") in
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
 such redistributions of the Apple Software.  Neither the name, trademarks, service marks
 or logos of Apple Inc. may be used to endorse or promote products derived from
 the Apple Software without specific prior written permission from Apple. Except as expressly
 stated in this notice, no other rights or licenses, express or implied, are granted by Apple
 herein, including but not limited to any patent rights that may be infringed by your
 derivative works or by other works in which the Apple Software may be incorporated.

 The Apple Software is provided by Apple on an "AS IS" basis.  APPLE MAKES NO WARRANTIES,
 EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED WARRANTIES OF NON-INFRINGEMENT,
 MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS
 USE AND OPERATION ALONE OR IN COMBINATION WITH YOUR PRODUCTS.

 IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL OR CONSEQUENTIAL
 DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) ARISING IN ANY WAY OUT OF THE USE,
 REPRODUCTION, MODIFICATION AND/OR DISTRIBUTION OF THE APPLE SOFTWARE, HOWEVER CAUSED AND
 WHETHER UNDER THEORY OF CONTRACT, TORT (INCLUDING NEGLIGENCE), STRICT LIABILITY OR
 OTHERWISE, EVEN IF APPLE HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#import "WebViewController.h"

@implementation WebViewController

NSString* const kDefaultWebsiteStr	= @"http://www.apple.com/";
NSString* const kPrefixStr			= @"http://";


// -------------------------------------------------------------------------------
//	webViewChanged:
//
//	As a WebFrameLoadDelegate, we get notified when the webview frame is finished
//	loading its new URL location.  Here we update the url text field to reflect the
//	new location.
// -------------------------------------------------------------------------------
- (void)webView:(WebView*)sender didFinishLoadForFrame:(WebFrame*)frame
{
	// don't go to the URL if the webView is already there
	if (![[urlView stringValue] isEqualToString: [webView mainFrameURL]])
	{
		[urlView setStringValue:[webView mainFrameURL]];
	}
}

// -------------------------------------------------------------------------------
//	awakeFromNib:
// -------------------------------------------------------------------------------
- (void)awakeFromNib
{
	// set the default URL for our URL text field and webView
	[urlView setStringValue: kDefaultWebsiteStr];
	[webView setMainFrameURL: kDefaultWebsiteStr];
	
	// become the delegate for the web view (WebFrameLoadDelegate) when the frame changes URL
	[webView setFrameLoadDelegate:self];
}

// -------------------------------------------------------------------------------
//	urlChanged:sender
//
//	The user finished editing the url text.
// -------------------------------------------------------------------------------
- (IBAction)urlChanged:(id)sender
{
	NSString* urlText = [urlView stringValue];
	NSString* finalURL;
	
	// add the prefix in case the user forgot to type it
	if (![urlText hasPrefix: kPrefixStr])
	{
		finalURL = [NSString stringWithFormat:@"%@%@", kPrefixStr, urlText];
		[urlView setStringValue:finalURL];
	}
	else
	{
		finalURL = urlText;
	}
	
	// don't go to the URL if the webView is already there
	if (![[urlView stringValue] isEqualToString: [webView mainFrameURL]])
		[webView setMainFrameURL: finalURL];
}

// -------------------------------------------------------------------------------
//	gotoURL:sender
//
//	User clicked the "Go" button.
// -------------------------------------------------------------------------------
- (IBAction)gotoURL:(id)sender
{
	// don't go to the URL if the webView is already there
	[[urlView stringValue] isEqualToString: [webView mainFrameURL]];
}

// -------------------------------------------------------------------------------
//	forwardAction:sender
//
//	User clicked the Forward button.
// -------------------------------------------------------------------------------
- (IBAction)forwardAction:(id)sender
{
	[webView goForward:self];
}

// -------------------------------------------------------------------------------
//	backAction:sender
//
//	User clicked the Back button.
// -------------------------------------------------------------------------------
- (IBAction)backAction:(id)sender
{
	[webView goBack:self];
}

@end
