 /*
 
 File: PongAI.m
 
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

/*************************************************************
/* The PongAI class decides the AI's next move by returning
/* a Direction from -nextMove. Behind the scenes, it manages
/* a JavaScript execution context, and calls a "nextMove"
/* function defined in JavaScript in order to decide how to 
/* move.
/************************************************************/

#import "PongAI.h"

#import "JSShape.h"
#import "Shape.h"

/* Convenience function for getting a property that is an object. */
static JSObjectRef getObjectProperty(JSContextRef ctx, JSObjectRef object, CFStringRef name)
{
    JSStringRef nameJS = JSStringCreateWithCFString(name);
    JSValueRef function = JSObjectGetProperty(ctx, object, nameJS, NULL);
    JSStringRelease(nameJS);

    if (!function || !JSValueIsObject(ctx, function))
        return NULL;
    return (JSObjectRef)function;
}

/* Convenience function for setting a property that is a numeric constant. */
static void setConstantNumberProperty(JSContextRef ctx, JSObjectRef object, CFStringRef name, double value)
{
    JSStringRef nameJS = JSStringCreateWithCFString(name);
    JSValueRef exception = NULL;
    JSObjectSetProperty(ctx, object, nameJS, JSValueMakeNumber(ctx, value), kJSPropertyAttributeDontDelete | kJSPropertyAttributeReadOnly, &exception);
    assert(exception == NULL);
    JSStringRelease(nameJS);
}

@implementation PongAI

- (PongAI *)initWithPaddle:(Shape *)paddle ball:(Shape *)ball script:(NSString *)script
{
    self = [super init];
    if (!self)
        return nil;

    // Create execution context.
    JSGlobalContextRef ctx = JSGlobalContextCreate(NULL);
    [self setJSGlobalContext:ctx];
    JSGlobalContextRelease(ctx);

    // Retrieve global object.
    JSObjectRef jsGlobalObject = JSContextGetGlobalObject(ctx);

    // Set kUpDirection, kDownDirection, and kStationaryDirection global constants.
    setConstantNumberProperty(ctx, jsGlobalObject, CFSTR("kUpDirection"), kUpDirection);
    setConstantNumberProperty(ctx, jsGlobalObject, CFSTR("kDownDirection"), kDownDirection);
    setConstantNumberProperty(ctx, jsGlobalObject, CFSTR("kStationaryDirection"), kStationaryDirection);

    // Evaluate AI script.
    JSStringRef scriptJS = JSStringCreateWithCFString((CFStringRef)script);
    JSEvaluateScript(ctx, scriptJS, NULL, NULL, 0, NULL);
    JSStringRelease(scriptJS);

    // Extract and store 'nextMove' function.
    [self setJSNextMoveFunction:getObjectProperty(ctx, jsGlobalObject, CFSTR("nextMove"))];

    // Cache 'paddle' and 'ball' wrapper objects to pass to 'nextMove' function.
    [self setJSPaddle:JSObjectMake(ctx, JSShape_class(ctx), paddle)];
    [self setJSBall:JSObjectMake(ctx, JSShape_class(ctx), ball)];

    return self;
}

- (Direction)nextMove
{
    JSContextRef ctx = [self jsGlobalContext];
    
    JSObjectRef function = [self jsNextMoveFunction];
    if (!function) // In case the AI script was malformed.
        return kStationaryDirection;

    // Call 'nextMove' function, passing in 'paddle' and 'ball' arguments.
    JSValueRef arguments[] = { [self jsPaddle], [self jsBall] };
    JSValueRef result = JSObjectCallAsFunction(ctx, function, NULL, 2, arguments, NULL);

    if (!result) // In case the nextMove function threw an exception.
        return kStationaryDirection;

    // Return result as a number.
    return JSValueToNumber(ctx, result, NULL);
}

- (JSObjectRef)jsBall
{
    return _jsBall;
}

- (void)setJSBall:(JSObjectRef)jsBall
{
    JSContextRef ctx = [self jsGlobalContext];

    // Protect the JavaScript object because we are storing it on the heap.
    if (_jsBall)
        JSValueUnprotect(ctx, _jsBall);
    if (jsBall)
        JSValueProtect(ctx, jsBall);
    _jsBall = jsBall;
}

- (JSObjectRef)jsPaddle
{
    return _jsPaddle;
}

- (void)setJSPaddle:(JSObjectRef)jsPaddle
{
    JSContextRef ctx = [self jsGlobalContext];
    
    // Protect the JavaScript object because we are storing it on the heap.
    if (_jsPaddle)
        JSValueUnprotect(ctx, _jsPaddle);
    if (jsPaddle)
        JSValueProtect(ctx, jsPaddle);
    _jsPaddle = jsPaddle;
}

- (JSObjectRef)jsNextMoveFunction
{
    return _jsNextMoveFunction;
}

- (void)setJSNextMoveFunction:(JSObjectRef)jsNextMoveFunction
{
    JSContextRef ctx = [self jsGlobalContext];
    
    // Protect the JavaScript object because we are storing it on the heap.
    if (_jsNextMoveFunction)
        JSValueUnprotect(ctx, _jsNextMoveFunction);
    if (jsNextMoveFunction)
        JSValueProtect(ctx, jsNextMoveFunction);
    _jsNextMoveFunction = jsNextMoveFunction;
}

- (JSGlobalContextRef)jsGlobalContext
{
    return _ctx;
}

- (void)setJSGlobalContext:(JSGlobalContextRef)ctx
{
    if (ctx == _ctx)
        return;

    if (_ctx)
        JSGlobalContextRelease(_ctx);
    if (ctx)
        JSGlobalContextRetain(ctx);
    _ctx = ctx;
}

- (void)dealloc
{
    // Release JavaScript resources.
    [self setJSBall:NULL];
    [self setJSPaddle:NULL];
    [self setJSNextMoveFunction:NULL];
    [self setJSGlobalContext:NULL];

    [super dealloc];
}

@end
