File: readme.txt

Abstract: ReadMe.txt for CallJS sample code project

Version: 1.0

(c) Copyright 2007 Apple Computer, Inc. All rights reserved.

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



This sample illustrates how to add JavaScript methods that call through to your Objective-C code.  These methods can be used in Mac OS X 10.3.9 and later.


Here is how you would go about implementing your own console.log() method in your WebKit application.  For the purposes of this description we'll assume you already have a delegate object set up for your WebView.  This sample has used this same technique for implementing three Objective-C methods that can be called from JavaScript. 


First, implement a webView:webView windowScriptObjectAvailable: method on your WebFrameLoadDelegate delegate object.  This method will be called once the page is loaded and it is ready for JavaScripts to run.  To install a 'console' object on the window, you would call the setValue:forKey: method on the windowScriptObject received as a parameter.  Here, we have passed a reference to the myConsoleObject and associated it with the name 'console'.  

- (void)webView:(WebView *)webView
        windowScriptObjectAvailable:(WebScriptObject *)windowScriptObject {

    [windowScriptObject setValue:myConsoleObject forKey:@"console"];

}

myConsoleObject is an instance of an object sub-classed from NSObject that will implement all of the Objective-C functionality for the 'console' object being made available to JavaScript.  In this sample, we have used the same object we are using for our WebKit delegates and for our window controller.  

On that object we will need to implement the .log() method that will be called from JavaScript along with a few other minor house keeping methods.  

For example, our .log() method may be implemented something like this:

- (void)doOutputToLog:(NSString *)theMessage
{
	NSLog(@"LOG: %@", theMessage);
}

you'll notice right away that the name of the method is much different than the name we would like to use in JavaScript.  In fact, we cannot use the name "doOutputToLog:" in JavaScript because of the colon.  So, the following house keeping routines will allow us to (a) let WebKit know that JavaScript is allowed to call this method, and (b) provide a mapping between the Objective-C method name and the JavaScript method name.  

To let WebKit know that it is okay to call our method, we implement the following class method on our myConsoleObject's class as follows:

+ (BOOL)isSelectorExcludedFromWebScript:(SEL)selector {
    if (selector == @selector(doOutputToLog:)) {
        return NO;
    }
    return YES;
}

This tells webkit that the method defined for the selector doOutputToLog: should not be excluded from the methods JavaScript is allowed to call.

Next, to provide the name mapping, we would provide the following class method on our myConsoleObject's class:

+ (NSString *) webScriptNameForSelector:(SEL)sel {
    if (sel == @selector(doOutputToLog:)) {
		return @"log";
	} else {
		return nil;
	}
}

This will provide a mapping between the Objective-C name and the the name used in JavaScript.


