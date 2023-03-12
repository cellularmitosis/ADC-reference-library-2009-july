/*

File: Ingredient.m

Abstract: The Ingredient entity (NSManagedObject subclass)

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


#import "Ingredient.h"

#import "Recipe.h"
#import "Measure.h"

@implementation Ingredient 


/**
    Creates a copy of this Ingredient object in the specified context and 
    store.  This is used to copy instances from one store to another (since we 
    cannot create cross-store relationships.)
*/

- (CRManagedObject *) copyToContext: (NSManagedObjectContext *)context andStore:(id)store {

    // insert a new Ingredient first
    Ingredient *newIngredient = [NSEntityDescription insertNewObjectForEntityForName:@"Ingredient" inManagedObjectContext:context];
    if ( store ) {
        [context assignObject:newIngredient toPersistentStore:store];
    }
    
    // use API on the base class to copy the properties
    [newIngredient copyAttributesFromObject: self];

    // copy the measure (checking for an existing one first)
    [newIngredient copyToOneRelationshipForKey: @"measure" fromObject: self useExisting:YES];
    
    // return
    return newIngredient;
}


#pragma mark
#pragma mark Core Data accessors/mutators/validation methods
#pragma mark


- (NSString *)name {

    NSString * tmpValue;
    
    [self willAccessValueForKey: @"name"];
    tmpValue = [self primitiveValueForKey: @"name"];
    [self didAccessValueForKey: @"name"];
    
    return tmpValue;
}


- (void)setName:(NSString *)value {

    [self willChangeValueForKey: @"name"];
    [self setPrimitiveValue: value forKey: @"name"];
    [self didChangeValueForKey: @"name"];
}


- (BOOL)validateName: (id *)valueRef error:(NSError **)outError {
    return YES;
}


- (BOOL)optional {

    NSNumber *tmpValue;
    
    [self willAccessValueForKey: @"optional"];
    tmpValue = [self primitiveValueForKey: @"optional"];
    [self didAccessValueForKey: @"optional"];
    
    return (tmpValue!=nil) ? [tmpValue boolValue] : FALSE;
}


- (void)setOptional:(BOOL)value {

    [self willChangeValueForKey: @"optional"];
    [self setPrimitiveValue: [NSNumber numberWithBool: value]
                     forKey: @"optional"];
    [self didChangeValueForKey: @"optional"];
}


- (double)amount {

    NSNumber *tmpValue;
    
    [self willAccessValueForKey: @"amount"];
    tmpValue = [self primitiveValueForKey: @"amount"];
    [self didAccessValueForKey: @"amount"];
    
    return (tmpValue!=nil) ? [tmpValue doubleValue] : 0.0;
}


- (void)setAmount:(double)value {

    [self willChangeValueForKey: @"amount"];
    [self setPrimitiveValue: [NSNumber numberWithDouble: value] forKey: @"amount"];
    [self didChangeValueForKey: @"amount"];
}


- (short)displayOrder {

    NSNumber *tmpValue;
    
    [self willAccessValueForKey: @"displayOrder"];
    tmpValue = [self primitiveValueForKey: @"displayOrder"];
    [self didAccessValueForKey: @"displayOrder"];
    
    return (tmpValue!=nil) ? [tmpValue shortValue] : 0;
}


- (void)setDisplayOrder:(short)value {

    [self willChangeValueForKey: @"displayOrder"];
    [self setPrimitiveValue: [NSNumber numberWithShort: value] forKey: @"displayOrder"];
    [self didChangeValueForKey: @"displayOrder"];
}


- (Recipe *)recipe {

    id tmpObject;
    
    [self willAccessValueForKey: @"recipe"];
    tmpObject = [self primitiveValueForKey: @"recipe"];
    [self didAccessValueForKey: @"recipe"];
    
    return tmpObject;
}


- (void)setRecipe:(Recipe *)value {

    [self willChangeValueForKey: @"recipe"];
    [self setPrimitiveValue: value forKey: @"recipe"];
    [self didChangeValueForKey: @"recipe"];
}



- (Measure *)measure {

    id tmpObject;
    
    [self willAccessValueForKey: @"measure"];
    tmpObject = [self primitiveValueForKey: @"measure"];
    [self didAccessValueForKey: @"measure"];
    
    return tmpObject;
}


- (void)setMeasure:(Measure *)value {

    [self willChangeValueForKey: @"measure"];
    [self setPrimitiveValue: value forKey: @"measure"];
    [self didChangeValueForKey: @"measure"];
}


@end
