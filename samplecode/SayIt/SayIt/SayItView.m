/*
    File: SayItView.h

    Abstract: A WebKit plug-in that can speak text passed from JavaScript;
		For more on plug-in programming and scripting see "Web Kit Plug-In
		Programming Topics" in the ADC Reference Library

    Version: 1.0

    Disclaimer: IMPORTANT:  This Apple software is supplied to you by Apple
    Computer, Inc. ("Apple") in consideration of your agreement to the
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
    Neither the name, trademarks, service marks or logos of Apple Computer,
    Inc. may be used to endorse or promote products derived from the Apple
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

    Copyright � 2006 Apple Computer, Inc., All Rights Reserved
*/ 

#import "SayItView.h"


@implementation SayItView


/*
 * plugInViewWithArguments is required by the WebPlugIn protocol
 * arguments is an NSDictionary consisting of various properties 
 * the plug-in was created with, including dimensions and PARAM
 * elements
 */

+ (NSView *)plugInViewWithArguments:(NSDictionary *)arguments
{
    return [[[self alloc] initWithFrame:NSZeroRect] autorelease];
}

/*
 * Optional WebPlugIn methods
 * Implement these to respond to various events during your plug-in's life cycle
 */
- (void)webPlugInInitialize
{
    if (!synth) {
        synth = [[NSSpeechSynthesizer alloc] init]; 
        [synth setDelegate:self];
    }
}

- (void)webPlugInDestroy
{
    if (synth) {
        [synth release];
        synth = nil;
    }
}

/*
 * WebScripting protocol methods
 * Implement these to allow JavaScript/Plug-In communication
 */
 
// Decide which methods will be exposed to JavaScript code
+ (BOOL)isSelectorExcludedFromWebScript:(SEL)selector
{
    if (selector == @selector(startSpeakingString:) || 
        selector == @selector(stopSpeaking))
        return NO;
        
    return YES;
}

// Produce JavaScript-readable function names that will be
// mapped to our exposed plug-in methods (see isSelectorExcludedFromWebScript:)
+ (NSString *)webScriptNameForSelector:(SEL)sel
{
    if (sel == @selector(startSpeakingString:))
		return @"startSpeakingString";
	else if (sel == @selector(stopSpeaking))
		return @"stopSpeaking";
    
    return nil;
}

// Requested by the hosting Web Kit application to make JavaScript access possible
- (id)objectForWebScript
{
    return self;
}


// Main plug-in functionality; activate and deactivate the speech synthesizer

- (void)startSpeakingString:(NSString *)sayWhat
{
    if (synth) {
        [synth stopSpeaking];
        [synth startSpeakingString:sayWhat];
    }
}

- (void)stopSpeaking
{
	if (synth)
        [synth stopSpeaking];
}
@end