/*

File: InferenceCore.m

Abstract: Core class for the inference system, which is a rules-based
system used by the predicate editor views.

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

Copyright © 2005 Apple Computer, Inc., All Rights Reserved

*/

#import "InferenceCore.h"

@interface InferredValuesAccessor : NSObject {
    InferenceCore  *inferenceCore;
}

- (id)initWithInferenceCore:(id)core;

@end

@implementation InferredValuesAccessor

- (id)initWithInferenceCore:(id)core {
    self = [super init];
    if (self != nil) {
        inferenceCore = core; // don't retain
    }
    return self;
}

- (id)valueForKey:(NSString *)key {
    return [inferenceCore inferredValueForKeyPath:key];   
}

- (id)valueForKeyPath:(NSString *)keyPath {
    return [inferenceCore inferredValueForKeyPath:keyPath];   
}

@end

@implementation InferencePair

- (id)value {
    return value;
}

- (void)setValue:(id)inValue {
    if (value != inValue) {
        [value release];
        value = [inValue retain];
    }
}

- (NSPredicate *)predicate {
    return condition;
}

- (void)setPredicate:(NSPredicate *)inValue {
    if (condition != inValue) {
        [condition release];
        condition = [inValue retain];
    }
}

@end

@implementation InferenceCore

static InferenceCore *defaultCore = nil;

+ (InferenceCore *)defaultCore {
    if (defaultCore == nil) {
        defaultCore = [[InferenceCore alloc] init];
    }

    return defaultCore;
}

- (id)privateInit {
    self = [super init];
    return self;
}

- (id)init {
    self = [super init];
    if (self != nil) {
	backingStore = [[NSMutableDictionary alloc] init];
	inferenceStore = [[NSMutableDictionary alloc] init];
	inferredValuesAccessor = nil;
	[backingStore setObject:[self inferredValues] forKey:@"inferredValues"];
    }
    return self;
}

- (void)dealloc {
    [inferredValuesAccessor release];
    [backingStore release];
    [inferenceStore release];
    [super dealloc];
}

- (id)copyWithZone:(NSZone *)zone {
    InferenceCore *core = [[InferenceCore allocWithZone:zone] privateInit];
    core->backingStore = [backingStore mutableCopy];
    core->inferenceStore = [inferenceStore mutableCopy];
    core->inferredValuesAccessor = nil;
    [core->backingStore setObject:[core inferredValues] forKey:@"inferredValues"];
    return core;
}

- (void)setValue:(id)value forKey:(NSString *)key {
    [self willChangeValueForKey:@"inferredValues"];
    [self willChangeValueForKey:key];
    [backingStore setObject:value forKey:key];
    [self didChangeValueForKey:key];
    [self didChangeValueForKey:@"inferredValues"];
}

- (void)setValue:(id)value forKey:(NSString *)key condition:(NSPredicate *)condition {
    InferencePair *inferencePair = [[InferencePair alloc] init];
    [inferencePair setPredicate:condition];
    [inferencePair setValue:value];
    
    NSMutableSet *bucket = [inferenceStore objectForKey:key];
    if (bucket == nil) {
        bucket = [[NSMutableSet alloc] init];
        [inferenceStore setObject:bucket forKey:key];
        [bucket release];
    }
    [bucket addObject:inferencePair];
    [inferencePair release];
}

- (id)valueForKey:(NSString *)key {
    return [backingStore objectForKey:key];   
}

- (id)valueForKeyPath:(NSString *)keyPath {
    return [self inferredValueForKeyPath:keyPath];
}

- (id)inferredValueForKeyPath:(NSString *)keyPath {
    id value = self;
    NSString *key;
    NSArray *components = [keyPath componentsSeparatedByString:@"."];
    NSEnumerator *enumerator = [components objectEnumerator];
    BOOL checkBuckets = YES; // we don't want to infer values for anything but the first keypath component
	
    while ((key = [enumerator nextObject]) != nil) {
	
	BOOL success = NO;
	if (checkBuckets) {  // we only infer values for the first keypath component
	    NSSet *bucket = [inferenceStore objectForKey:key];
	   // NSLog(@"Inference logging: Will infer value for key %@", key);
	    if (bucket != nil) {
		NSArray *bucketValues = [bucket allObjects];
		InferencePair *inferencePair;
		int count = [bucketValues count];
		int index=0;
		for (;index < count; index++) {
		    inferencePair = [bucketValues objectAtIndex:index];
		    if ([[inferencePair predicate] evaluateWithObject:self]) {
			value = [inferencePair value];
			success = YES;
			//NSLog(@"Inference logging: Inferred value for key %@: %@", key, value);
			break;
		    }
		}
	    } 
	}
	
	if (success == NO) {
	  //  NSLog(@"Inference logging: Check backing store for key %@", key);
	    value = [value valueForKey:key];   
	    if (value == nil) {
	//	NSLog(@"Inference logging: Failed check backing store value for key %@", key);
	    } else {
	//	NSLog(@"Inference logging: Found backing store value %@ for key %@", value, key);
	    }
	} else if (value == [self inferredValues]) {
	    continue;
	}
	
	if ([value isKindOfClass:[InferenceObject self]]) {
	//    NSLog(@"Inference logging: Will evaluate value for key %@", [value valueForKey:@"key"]);
	    value = [value evaluateWithInferenceCore:self];   
	  //  NSLog(@"Inference logging: Result of evaluation for key %@: %@", key, value);
	}
	
	checkBuckets = NO; // we don't want to infer values for anything but the first keypath component
    }
    
    return (value != self) ? value : nil;
}

- (id)inferredValues {
    if (inferredValuesAccessor == nil) {
	inferredValuesAccessor = [[InferredValuesAccessor alloc] initWithInferenceCore:self];
    }
    return inferredValuesAccessor;
}

@end

@implementation InferenceObject

- (id)initWithTarget:(id)inTarget key:(NSString *)inKey {
    self = [super init];
    if (self != nil) {
	target = inTarget;
	key = [inKey retain];
    }
    return self;
}

- (id)evaluateWithInferenceCore:(InferenceCore *)core {
    NSString *selectorName = [NSString stringWithFormat:@"%@:", key];
    SEL selector = NSSelectorFromString(selectorName);
    
    id value = [target performSelector:selector withObject:core];
    
    return value;   
}

@end

