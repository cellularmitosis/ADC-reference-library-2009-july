/*
File: Plugin.m

Abstract: A WebKit Plug-in showing how to use the QTCapture framework
	to perform video/audio capture and recording to a QuickTime movie.

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

Copyright (C) 2007 Apple Inc. All Rights Reserved.
*/

#import "Plugin.h"

@implementation Plugin

	/* adjust the frame boundaries inside of your awakeFromNib method */
- (void)awakeFromNib 
{
	PDEBUG();

	if( mViewPanel != nil ) 
	{
		[self setFrame:[mViewPanel frame]]; /* adjust the frame from your nib's view to the plug-in's view */
		[self addSubview:mViewPanel]; /* add the view from the nib as our one and only sub-view */

		// MyCaptureView class needs a way to make calls to JavaScript (with a parameter).
		// Currently, calls to JavaScript must be done from the main Plugin class, so we've implemented a
		// method notifyContainingPageOfDeviceStateChange: here in the Plugin class which the MyCaptureView object
		// can use as a mechanism to do this. Here we are simply telling the MyCaptureView object to
		// use this particular method to call through to JavaScript.
		[mViewPanel setPlugInCallback:@selector(notifyContainingPageOfDeviceStateChange:) onTarget:self];
	}
}

	/* deallocation for our plug-in */
- (void) dealloc 
{
	PDEBUG();

	[self setPluginArguments: nil];
	[super dealloc];
}


	/* initializer for our plug-in */
#pragma mark *** WebPlugInViewFactory methods ***
-(id) initWithArguments:(NSDictionary *)arguments 
{
	PDEBUG();
	
	if ((self = [super init]) != nil) 
	{
		[self setPluginArguments: arguments];
	}
	return self;
}

	/* WebPlugInViewFactory message handler for vending plug-in instances */
+ (NSView *)plugInViewWithArguments:(NSDictionary *)arguments 
{
	PDEBUG();
	
	Plugin *thePlugin;
	thePlugin = [[[self alloc] initWithArguments:arguments] autorelease];
	
	return thePlugin;
}


	/* accessors for pluginArguments */
- (NSDictionary *)pluginArguments 
{
	PDEBUG();
	
	return [[mPluginArguments retain] autorelease];
}

- (void)setPluginArguments:(NSDictionary *)value 
{
	PDEBUG();
	
	if (mPluginArguments != value) 
	{
		[mPluginArguments release];
		mPluginArguments = [value copy];
	}
}

	/* Load your nib file inside of your webPlugInInitialize method. */
#pragma mark *** WebPlugin methods ***
- (void)webPlugInInitialize 
{
	PDEBUG();

	BOOL result;
	result = [NSBundle loadNibNamed:@"PluginView" owner:self];
}

	/* Returns the object that exposes the plug-in's interface */ 
- (id)objectForWebScript 
{
	PDEBUG();
	
	return self;
}

#pragma mark *** WebScripting methods ***

	/* Prevent direct key access from JavaScript.
	Write accessor methods and expose those if necessary */

+ (BOOL) isKeyExcludedFromWebScript:(const char*)key {
	return YES;
}

	/* the following two class methods allow us to export our 
	performOp:withOptions: method so it can be called from JavaScript
	using the name PerformOperation(opname,options). */

+ (BOOL)isSelectorExcludedFromWebScript:(SEL)selector {
	PDEBUG(@"selector = '%@'", NSStringFromSelector(selector));
    if ( selector == @selector(web_startRecording) ) {
        return NO;
    } else if ( selector == @selector(web_stopRecording) ) {
        return NO;
    } else if ( selector == @selector(web_stopCapture) ) {
        return NO;
    } else if ( selector == @selector(web_startCapture) ) {
        return NO;
    }
    return YES;
}

+ (NSString *)webScriptNameForSelector:(SEL)aSelector {
	PDEBUG(@"aSelector = '%@'", NSStringFromSelector(aSelector));
    if ( aSelector == @selector(web_startRecording) ) {
        return @"StartRecording";
    } else if ( aSelector == @selector(web_stopRecording) ) {
        return @"StopRecording";
    } else if ( aSelector == @selector(web_stopCapture) ) {
        return @"StopCapture";
    } else if ( aSelector == @selector(web_startCapture) ) {
        return @"StartCapture";
    } else {
	return nil; }
}

#pragma mark *** Web-exposed methods ***

	/* these are the 4 selectors being exported to the script environment */
	
	/* start recording capture to a QuickTime movie */
- (void) web_startRecording {
	[mViewPanel startRecord];
}

	/* stop recording capture to a QuickTime movie */
- (void) web_stopRecording {
	[mViewPanel stopRecord];
}

	/* start capture */
- (void) web_startCapture {
	[mViewPanel startCapture];
}

	/* stop capture */
- (void) web_stopCapture {
	[mViewPanel stopCapture];
}

	/* Calling JavaScript in the containing page from our plug-in. */

#pragma mark *** Plugin calls to JavaScript ***
-(void)notifyContainingPageOfDeviceStateChange:(NSString *)state
{
	id pluginContainer = [[self pluginArguments] objectForKey:WebPlugInContainerKey];
	if (pluginContainer) 
	{
			/* retrieve a reference to the webview */
		WebView *myWebView = [[pluginContainer webFrame] webView];
		
			/* Set the name of our callback */
		NSString* callbackName = @"DeviceStateChange";
		
			/* create the list of parameters for our call back.  in this case, there
			is only one parameter. */
		NSArray* parameterList = [NSArray arrayWithObject:state];
			
			/* make a simple call through to JavaScript. */
		[[myWebView windowScriptObject]
			callWebScriptMethod:callbackName withArguments:parameterList];
	}
}

@end
