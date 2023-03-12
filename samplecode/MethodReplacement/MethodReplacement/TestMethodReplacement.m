/*
 
 File: TestMethodReplacement.m
 
 Abstract:

 Objective C 2.0 Compatible class_poseAs() replacement. This
 demonstrates how to replace a method in an existing Objective C class,
 and how to call the old version without having to store IMP function
 pointers in a global. The trick is to define a category on the class
 whose method you want to replace. In the category, declare and
 implement a method with the same signature as the replaced method. In
 the +load method of the category, look up the original and replacement
 methods using class_getInstanceMethod(), then call
 method_exchangeImplementations(). Subsequent calls to the original
 method will call the replacement method, and an apparently recursive
 call to the replacement method will call the original method. Voila.
 
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
 
 Copyright © 2006 Apple Computer, Inc., All Rights Reserved
 
 */

#import <AppKit/AppKit.h>
#import <objc/runtime.h>

@interface NSWindow (TestMethodReplacement)
- (void)TestMethodReplacement_sendEvent:(NSEvent *)event;
@end

@implementation NSWindow (TestMethodReplacement)

+ (void)load {
    if (self == [NSWindow class]) {
        // Swap the implementations of -[NSWindow sendEvent:] and -[NSWindow TestMethodReplacement_sendEvent:].
        // When the -sendEvent: message is sent to an NSWindow instance, -TestMethodReplacement_sendEvent: will
        // be called instead. Calling [self TestMethodReplacement_sendEvent:event] thus calls the original method.
        Method originalMethod = class_getInstanceMethod(self, @selector(sendEvent:));
        Method replacedMethod = class_getInstanceMethod(self, @selector(TestMethodReplacement_sendEvent:));
        method_exchangeImplementations(originalMethod, replacedMethod);
    }
}

- (void)TestMethodReplacement_sendEvent:(NSEvent *)event {
    if ([event type] == NSLeftMouseDown && [event modifierFlags] & NSCommandKeyMask) {
        [NSAnimationContext beginGrouping];
        [[NSAnimationContext currentContext] setDuration:1.0];
        [[self animator] setAlphaValue:0.0];
        [NSAnimationContext endGrouping];
    } else {
        // Call the original sendEvent: method, whose implementation was exchanged with our own.
        // Note:  this ISN'T a recursive call, because this method should have been called through -sendEvent:.
        NSParameterAssert(_cmd == @selector(sendEvent:));
        [self TestMethodReplacement_sendEvent:event];
    }
}

@end
