/*

File: MyPluginClass.m

Abstract: Plug-in code for Fortune widget sample 

Version: 1.0

© Copyright 2005 Apple Computer, Inc. All rights reserved.

IMPORTANT:  This Apple software is supplied to 
you by Apple Computer, Inc. ("Apple") in 
consideration of your agreement to the following 
terms, and your use, installation, modification 
or redistribution of this Apple software 
constitutes acceptance of these terms.  If you do 
not agree with these terms, please do not use, 
install, modify or redistribute this Apple 
software.

In consideration of your agreement to abide by 
the following terms, and subject to these terms, 
Apple grants you a personal, non-exclusive 
license, under Apple's copyrights in this 
original Apple software (the "Apple Software"), 
to use, reproduce, modify and redistribute the 
Apple Software, with or without modifications, in 
source and/or binary forms; provided that if you 
redistribute the Apple Software in its entirety 
and without modifications, you must retain this 
notice and the following text and disclaimers in 
all such redistributions of the Apple Software. 
Neither the name, trademarks, service marks or 
logos of Apple Computer, Inc. may be used to 
endorse or promote products derived from the 
Apple Software without specific prior written 
permission from Apple.  Except as expressly 
stated in this notice, no other rights or 
licenses, express or implied, are granted by 
Apple herein, including but not limited to any 
patent rights that may be infringed by your 
derivative works or by other works in which the 
Apple Software may be incorporated.

The Apple Software is provided by Apple on an "AS 
IS" basis.  APPLE MAKES NO WARRANTIES, EXPRESS OR 
IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED 
WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY 
AND FITNESS FOR A PARTICULAR PURPOSE, REGARDING 
THE APPLE SOFTWARE OR ITS USE AND OPERATION ALONE 
OR IN COMBINATION WITH YOUR PRODUCTS.

IN NO EVENT SHALL APPLE BE LIABLE FOR ANY 
SPECIAL, INDIRECT, INCIDENTAL OR CONSEQUENTIAL 
DAMAGES (INCLUDING, BUT NOT LIMITED TO, 
PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS 
OF USE, DATA, OR PROFITS; OR BUSINESS 
INTERRUPTION) ARISING IN ANY WAY OUT OF THE USE, 
REPRODUCTION, MODIFICATION AND/OR DISTRIBUTION OF 
THE APPLE SOFTWARE, HOWEVER CAUSED AND WHETHER 
UNDER THEORY OF CONTRACT, TORT (INCLUDING 
NEGLIGENCE), STRICT LIABILITY OR OTHERWISE, EVEN 
IF APPLE HAS BEEN ADVISED OF THE POSSIBILITY OF 
SUCH DAMAGE.

*/

#import "MyPluginClass.h"

/*********************************************/
// The number of quotes and the quotes themselves; they are randomly chosen 
// below and fed back to the Fortune widget
/*********************************************/

enum {
	NUM_QUOTES = 8
};

static NSString *quotes[NUM_QUOTES] = { 
	@"You will be awarded some great honor.",		
	@"You are soon going to change your present line of work.",
	@"You will have gold pieces by the bushel.",
	@"You will be fortunate in the opportunities presented to you.",
	@"Someone is speaking well of you.",
	@"Be direct, usually one can accomplish more that way.",
	@"You need not worry about your future.",
	@"Generosity and perfection are your everlasting goals.",
};


/*********************************************/
// The implementation of the widget plugin follows...
/*********************************************/

@implementation MyPluginClass


/*********************************************/
// Methods required by the WidgetPlugin protocol
/*********************************************/

// initWithWebView
//
// This method is called when the widget plugin is first loaded as the
// widget's web view is first initialized
-(id)initWithWebView:(WebView*)w {
	//NSLog(@"Entering -initWithWebView:%@", w);
	self = [super init];
	srand(time(NULL));
	return self;
}

-(void)dealloc {
	[super dealloc];
}


/*********************************************/
// Methods required by the WebScripting protocol
/*********************************************/

// windowScriptObjectAvailable
//
// This method gives you the object that you use to bridge between the
// Obj-C world and the JavaScript world.  Use setValue:forKey: to give
// the object the name it's refered to in the JavaScript side.
-(void)windowScriptObjectAvailable:(WebScriptObject*)wso {
	//NSLog(@"windowScriptObjectAvailable");
	[wso setValue:self forKey:@"FortunePlugin"];
}

// webScriptNameForSelector
//
// This method lets you offer friendly names for methods that normally 
// get mangled when bridged into JavaScript.
+(NSString*)webScriptNameForSelector:(SEL)aSel {
	NSString *retval = nil;
	
	//NSLog(@"webScriptNameForSelector");
	if (aSel == @selector(getFortune)) {
		retval = @"getFortune";
	} else if (aSel == @selector(logMessage:)) {
		retval = @"logMessage";
	} else {
		NSLog(@"\tunknown selector");
	}
	
	return retval;
}

// isSelectorExcludedFromWebScript
//
// This method lets you filter which methods in your plugin are accessible 
// to the JavaScript side.
+(BOOL)isSelectorExcludedFromWebScript:(SEL)aSel {	
	if (aSel == @selector(getFortune) || aSel == @selector(logMessage:)) {
		return NO;
	}
	return YES;
}

// isKeyExcludedFromWebScript
//
// Prevents direct key access from JavaScript.
+(BOOL)isKeyExcludedFromWebScript:(const char*)k {
	return YES;
}


/*********************************************/
// The actual methods used in this plugin, to be called by JavaScript and 
// identified in isSelectorExcludedFromWebScript:.
/*********************************************/

// getFortune
//
// Returns the fortune to the widget.
- (NSString *) getFortune {
	short temp;
	temp = rand() % NUM_QUOTES;
	
	if(temp == sayingCount) {
		temp = (temp + 1) % NUM_QUOTES;
	}
	
	sayingCount = temp;
	return quotes[sayingCount];
}

// logMessage
//
// Sends the message passed in from JavaScript to the console
// This demonstrates the translation of a JavaScript string to an
// NSString *; in the real world just call alert() from JavaScript, 
// which Dashboard sends to Console anyway
- (void) logMessage:(NSString *)str {
	NSLog(@"JavaScript says: %@", str);
}

@end
