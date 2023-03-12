/*

File: SampleController.h

Abstract: declarations for the main controller object
for the CallJS sample.

Version: 1.0

� Copyright 2007 Apple Computer, Inc. All rights reserved.

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

#import <Cocoa/Cocoa.h>
#import <WebKit/WebKit.h>

@interface SampleController : NSObject
{
    IBOutlet id theWebView; /* WebView */
    IBOutlet id paramOne; /* NSTextField */
    IBOutlet id paramTwo; /* NSTextField */
    IBOutlet id callResult; /* NSTextField */
    IBOutlet id runResult; /* NSTextField */
    IBOutlet id scriptText; /* NSTextView */
	
	NSString *sharedValue; /* value shared between Objective-C and JavaScript */
}

	/* storage management for our instance variable(s) */
-(id) init;
- (void) dealloc;

	/* IB/nib methods */
- (IBAction) runJavaScript:(id)sender;
- (IBAction) callJavaScriptWithParameters:(id)sender;
- (void) awakeFromNib;

	/* app delegate method */
- (BOOL)applicationShouldTerminateAfterLastWindowClosed:(NSApplication *)sender;

	/* WebFrameLoadDelegate method */
- (void)webView:(WebView *)webView windowScriptObjectAvailable:(WebScriptObject *)windowScriptObject;

	/* WebUIDelegate method */
- (void)webView:(WebView *)sender runJavaScriptAlertPanelWithMessage:(NSString *)message;

	/* WebScripting methods */
+ (BOOL)isSelectorExcludedFromWebScript:(SEL)selector;
+ (BOOL)isKeyExcludedFromWebScript:(const char *)property;
+ (NSString *) webScriptNameForSelector:(SEL)sel;

	/* methods we're sharing with JavaScript */
- (void) doOutputToLog: (NSString*) theMessage;
- (NSString*) changeJavaScriptText: (NSString*) theScriptText;
- (void) report;

	/* accessor methods for our sharedValue instance variable */
- (NSString *)sharedValue;
- (void)setSharedValue:(NSString *)value;


@end

