 /*
 
 File: JSShape.m
 
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
/* The JSShape pseudo-class describes a JavaScript wrapper
/* around the Shape class.
/************************************************************/

#import "JSShape.h"
#import "Shape.h"

/* Retain and release the Shape because a garbage collected object has an unpredictable lifetime. */
static void JSShape_initialize(JSContextRef ctx, JSObjectRef object)
{
    Shape *shape = JSObjectGetPrivate(object);
    [shape retain];
}

static void JSShape_finalize(JSObjectRef object)
{
    Shape *shape = JSObjectGetPrivate(object);
    [shape release];
}

static JSValueRef JSShape_getLeft(JSContextRef ctx, JSObjectRef object, JSStringRef propertyName, JSValueRef* exception)
{
    Shape *shape = JSObjectGetPrivate(object);
    return JSValueMakeNumber(ctx, [shape left]);
}

static JSValueRef JSShape_getTop(JSContextRef ctx, JSObjectRef object, JSStringRef propertyName, JSValueRef* exception)
{
    Shape *shape = JSObjectGetPrivate(object);
    return JSValueMakeNumber(ctx, [shape top]);
}

static JSValueRef JSShape_getRight(JSContextRef ctx, JSObjectRef object, JSStringRef propertyName, JSValueRef* exception)
{
    Shape *shape = JSObjectGetPrivate(object);
    return JSValueMakeNumber(ctx, [shape right]);
}

static JSValueRef JSShape_getBottom(JSContextRef ctx, JSObjectRef object, JSStringRef propertyName, JSValueRef* exception)
{
    Shape *shape = JSObjectGetPrivate(object);
    return JSValueMakeNumber(ctx, [shape bottom]);
}

static JSValueRef JSShape_getHeight(JSContextRef ctx, JSObjectRef object, JSStringRef propertyName, JSValueRef* exception)
{
    Shape *shape = JSObjectGetPrivate(object);
    return JSValueMakeNumber(ctx, [shape height]);
}

static JSValueRef JSShape_getWidth(JSContextRef ctx, JSObjectRef object, JSStringRef propertyName, JSValueRef* exception)
{
    Shape *shape = JSObjectGetPrivate(object);
    return JSValueMakeNumber(ctx, [shape width]);
}

static JSValueRef JSShape_getMiddleX(JSContextRef ctx, JSObjectRef object, JSStringRef propertyName, JSValueRef* exception)
{
    Shape *shape = JSObjectGetPrivate(object);
    return JSValueMakeNumber(ctx, [shape middleX]);
}

static JSValueRef JSShape_getMiddleY(JSContextRef ctx, JSObjectRef object, JSStringRef propertyName, JSValueRef* exception)
{
    Shape *shape = JSObjectGetPrivate(object);
    return JSValueMakeNumber(ctx, [shape middleY]);
}

/* DontDelete and ReadOnly prevent errant scripts from making arbitrary changes to the model that would break the game flow, or deleting parts of the API. */
static JSStaticValue JSShape_staticValues[] = {
    { "left", JSShape_getLeft, NULL, kJSPropertyAttributeDontDelete | kJSPropertyAttributeReadOnly },
    { "top", JSShape_getTop, NULL, kJSPropertyAttributeDontDelete | kJSPropertyAttributeReadOnly },
    { "right", JSShape_getRight, NULL, kJSPropertyAttributeDontDelete | kJSPropertyAttributeReadOnly },
    { "bottom", JSShape_getBottom, NULL, kJSPropertyAttributeDontDelete | kJSPropertyAttributeReadOnly },
    { "height", JSShape_getHeight, NULL, kJSPropertyAttributeDontDelete | kJSPropertyAttributeReadOnly },
    { "width", JSShape_getWidth, NULL, kJSPropertyAttributeDontDelete | kJSPropertyAttributeReadOnly },
    { "middleX", JSShape_getMiddleX, NULL, kJSPropertyAttributeDontDelete | kJSPropertyAttributeReadOnly },
    { "middleY", JSShape_getMiddleY, NULL, kJSPropertyAttributeDontDelete | kJSPropertyAttributeReadOnly },
    { 0, 0, 0, 0 }
};

/* Caches and returns the JSShape class. */
JSClassRef JSShape_class(JSContextRef ctx)
{
    static JSClassRef jsClass;
    if (!jsClass) {
        JSClassDefinition classDefinition = kJSClassDefinitionEmpty;
        classDefinition.staticValues = JSShape_staticValues;
        classDefinition.initialize = JSShape_initialize;
        classDefinition.finalize = JSShape_finalize;

        jsClass = JSClassCreate(&classDefinition);
    }
    
    return jsClass;
}
