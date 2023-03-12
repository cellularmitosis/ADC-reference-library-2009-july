/*
 
 File: DynamicProperties.m
 
 Abstract: Demonstrates dynamically resolved properties. Uses a new
 feature of the Objective C 2.0 runtime:  dynamically resolved methods.
 To support dynamically resolved methods, a class may define two new methods:
 
 + (BOOL)resolveInstanceMethod:(SEL)name;
 + (BOOL)resolveClassMethod:(SEL)name;
 
 The first time an unrecognized message is sent to an object, the runtime
 checks to see if the class implements +resolveInstanceMethod:, and if so
 calls it. +resolveInstanceMethod: should examine the selector and if it
 should be resolved, call class_addMethod() with an appropriate method
 implementation, and return YES. If the method returns NO, the method will
 be handled by standard method forwarding, as before. +resolveClassMethod:
 resolves unrecognized messages sent to a class in a similar way, except
 that the method should be added to the metaclass.
 
 Possible uses of dynamic method resolution:  faulting in method implementations
 that reside in dynamically loaded libraries;  runtime environment specialization,
 such as choosing between different method implementations based on the
 capabilities of the OS or underlying processor; lastly language bridges could
 use dynamic method resolution to lazily bind their object wrapper methods.
 
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

#import <Foundation/Foundation.h>
#import <objc/runtime.h>
#import <math.h>

/*
 This category provides a generic dynamic property resolver service for all NSMutableDictionary
 subclasses. It handles unresolved messages by examining the property metadata of the class,
 searching for a property with a name derived from the accessor method name. The properties system
 uses a subset of the naming scheme Key-Value Coding (KVC) uses; i.e. the following property
 
 @property id myExcellentProperty;
 
 will have accessor methods named "myExcellentProperty" and "setMyExcellentProperty:" regardless
 of the type of the property.
 
 The resolver examines the type of the property that corresponds to the method selector, and
 supports property types of id, double, and NSRect. Other types are easy to add.
 */
@interface NSMutableDictionary (DynamicPropertiesResolver)
+ (BOOL)resolveInstanceMethod:(SEL)name;
@end

@implementation NSMutableDictionary (DynamicPropertiesResolver)

// to speed this code up, should create a map from SEL to NSString mapping selectors to their keys.

// converts a getter selector to an NSString, equivalent to NSStringFromSelector().
NS_INLINE NSString *getterKey(SEL sel) {
    return [NSString stringWithUTF8String:sel_getName(sel)];
}

// converts a setter selector, of the form "set<Key>:" to an NSString of the form @"<key>".
NS_INLINE NSString *setterKey(SEL sel) {
    const char* name = sel_getName(sel) + 3; // skip past 'set'
    int length = strlen(name);
    char buffer[1 + length];
    strcpy(buffer, name);
    buffer[0] = tolower(buffer[0]);
    buffer[length - 1] = '\0';
    return [NSString stringWithUTF8String:buffer];
}

// Generic accessor methods for property types id, double, and NSRect.

static void putProperty(NSMutableDictionary *self, SEL name, id value) {
    [self setObject:value forKey:setterKey(name)];
}

static id getProperty(NSMutableDictionary *self, SEL name) {
    return [self objectForKey:getterKey(name)];
}

static void putDoubleProperty(NSMutableDictionary *self, SEL name, double value) {
    [self setObject:[NSNumber numberWithDouble:value] forKey:setterKey(name)];
}

static double getDoubleProperty(NSMutableDictionary *self, SEL name) {
    return [[self objectForKey:getterKey(name)] doubleValue];
}

static void putNSRectProperty(NSMutableDictionary *self, SEL name, NSRect rect) {
    [self setObject:NSStringFromRect(rect) forKey:setterKey(name)];
}

static NSRect getNSRectProperty(NSMutableDictionary *self, SEL name) {
    return NSRectFromString([self objectForKey:getterKey(name)]);
}

static const char* getPropertyType(objc_property_t property) {
    // parse the property attribues. this is a comma delimited string. the type of the attribute starts with the
    // character 'T' should really just use strsep for this, using a C99 variable sized array.
    const char *attributes = property_getAttributes(property);
    char buffer[1 + strlen(attributes)];
    strcpy(buffer, attributes);
    char *state = buffer, *attribute;
    while ((attribute = strsep(&state, ",")) != NULL) {
        if (attribute[0] == 'T') {
            // return a pointer scoped to the autorelease pool. Under GC, this will be a separate block.
            return (const char *)[[NSData dataWithBytes:(attribute + 1) length:strlen(attribute)] bytes];
        }
    }
    return "@";
}

static BOOL getPropertyInfo(Class cls, NSString *propertyName, Class *propertyClass, const char* *propertyType) {
    const char *name = [propertyName UTF8String];
    while (cls != NULL) {
        objc_property_t property = class_getProperty(cls, name);
        if (property) {
            *propertyClass = cls;
            *propertyType = getPropertyType(property);
            return YES;
        }
        cls = class_getSuperclass(cls);
    }
    return NO;
}

+ (BOOL)resolveInstanceMethod:(SEL)name {
    Class propertyClass;
    const char *propertyType;
    IMP accessor = NULL;
    const char *signature = NULL;
    
    // TODO:  handle more property types. This code handles id, double, and NSRect for demonstration purposes.
    if (strncmp("set", sel_getName(name), 3) == 0) {
        // choose an appropriately typed generic setter function.
        if (getPropertyInfo(self, setterKey(name), &propertyClass, &propertyType)) {
            switch (propertyType[0]) {
            case _C_ID:
                accessor = (IMP)putProperty, signature = "v@:@"; break;
            case _C_DBL:
                accessor = (IMP)putDoubleProperty, signature = "v@:d"; break;
            case _C_STRUCT_B:
                if (strncmp(propertyType, "{_NSRect=", 9) == 0)
                    accessor = (IMP)putNSRectProperty, signature = "v@:{_NSRect}";
                break;
            }
        }
    } else {
        // choose an appropriately typed getter function.
        if (getPropertyInfo(self, getterKey(name), &propertyClass, &propertyType)) {
            switch (propertyType[0]) {
            case _C_ID:
                accessor = (IMP)getProperty, signature = "@@:"; break;
            case _C_DBL:
                accessor = (IMP)getDoubleProperty, signature = "d@:"; break;
            case _C_STRUCT_B:
                if (strncmp(propertyType, "{_NSRect=", 9) == 0)
                    accessor = (IMP)getNSRectProperty, signature = "{_NSRect}@:";
                break;
            }
        }
    }
    if (accessor && signature) {
        class_addMethod(propertyClass, name, accessor, signature);
        return YES;
    }
    return NO;
}

@end

/*
 This category specifies type-safe accessors for named properties on
 all subclasses of NSMutableDictionary. In this sample code only
 3 specific types are supported, but this could easily be extended
 to all the basic types.
 */
@interface NSMutableDictionary (DynamicProperties)
@property(dynamic) double number;
@property(dynamic) NSString *string;
@property(dynamic) NSRect rect;
@end

@implementation NSMutableDictionary (DynamicProperties)
// needed to generate property metadata. otherwise not needed.
@end

/*
 This category converts any object into a dictionary that contains the values of all of the object's properties.
 */
@interface NSObject (PropertyDescription)
@property(dynamic) Class class;
- (NSDictionary *)declaredProperties;
@end

@implementation NSObject (PropertyDescription)
- (NSDictionary *)declaredProperties {
    // uses property metadata API to include all of the possible properties this class has.
    NSMutableDictionary *properties = [NSMutableDictionary dictionary];
    Class cls;
    for (cls = [self class]; cls != NULL; cls = class_getSuperclass(cls)) {
        unsigned int i, count;
        objc_property_t *list = class_copyPropertyList(cls, &count);
        if (list == NULL) continue;
        for (i = 0; i < count; ++i) {
            objc_property_t property = list[i];
            const char *type = getPropertyType(property);
            const char *name = property_getName(property);
            NSString *key = [NSString stringWithUTF8String:name];
            switch (type[0]) {
            case _C_ID:
            case _C_CLASS:
                [properties setObject:objc_msgSend(self, sel_registerName(name)) forKey:key];
                break;
            default:
                [properties setObject:[self valueForKey:key] forKey:key];
            }
        }
        free(list);
    }
    return properties;
}
@end

int main (int argc, const char * argv[]) {
    NSAutoreleasePool * pool = [NSAutoreleasePool new];

    // create a mutable dictionary and write/read strongly typed, but dynamically resolved properties.
    NSMutableDictionary *dict = [NSMutableDictionary dictionary];
    dict.number = M_PI;
    dict.string = @"a string";
    dict.rect = NSMakeRect(0, 0, 10, 10);
    // read the properties back.
    NSLog(@"dict.number = %lg, dict.string = @\"%@\", dict.rect = %@", dict.number, dict.string, NSStringFromRect(dict.rect));
    // print the properties using property metadata.
    NSLog(@"[dict declaredProperties] = %@", [dict declaredProperties]);

    [pool release];
    return 0;
}
