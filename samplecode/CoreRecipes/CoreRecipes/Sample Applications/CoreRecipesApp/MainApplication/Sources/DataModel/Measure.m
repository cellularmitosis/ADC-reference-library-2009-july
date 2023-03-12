/*

File: Measure.m

Abstract: The Measure entity (NSManagedObject subclass)

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


#import "Measure.h"


@implementation Measure 


/**
    Returns a predicate to find other measures similar to this one.  In this 
    case we are looking for measures which have the same name and abbreviation;
    the type and displayOrder attributes are not of interest.
*/

- (NSPredicate *) predicateForSimilarObject {
    return [NSPredicate predicateWithFormat: @"(abbreviation == %@) && (name == %@)", [self abbreviation], [self name]];
}


/**
    Creates a copy of this Measure object in the specified context and 
    store.  This is used to copy instances from one store to another (since we 
    cannot create cross-store relationships.)
*/

- (CRManagedObject *) copyToContext: (NSManagedObjectContext *)context andStore:(id)store {

    // insert a new Measure first
    Measure *newMeasure = [NSEntityDescription insertNewObjectForEntityForName:@"Measure" inManagedObjectContext:context];
    if ( store ) {
        [context assignObject:newMeasure toPersistentStore:store];
    }
    
    // use API on the base class to copy the properties (no relationships to copy)
    [newMeasure copyAttributesFromObject: self];
    return newMeasure;
}


#pragma mark
#pragma mark Core Data accessors/mutators/validation methods
#pragma mark


- (NSString *)abbreviation  {

    NSString * tmpValue;
    
    [self willAccessValueForKey: @"abbreviation"];
    tmpValue = [self primitiveValueForKey: @"abbreviation"];
    [self didAccessValueForKey: @"abbreviation"];
    
    return tmpValue;
}


- (void)setAbbreviation:(NSString *)value {

    [self willChangeValueForKey: @"abbreviation"];
    [self setPrimitiveValue: value forKey: @"abbreviation"];
    [self didChangeValueForKey: @"abbreviation"];
}


- (short)displayOrder  {

    NSNumber *tmpValue;
    
    [self willAccessValueForKey: @"displayOrder"];
    tmpValue = [self primitiveValueForKey: @"displayOrder"];
    [self didAccessValueForKey: @"displayOrder"];
    
    return (tmpValue!=nil) ? [tmpValue shortValue] : 0;
}


- (void)setDisplayOrder:(short)value  {

    [self willChangeValueForKey: @"displayOrder"];
    [self setPrimitiveValue: [NSNumber numberWithShort: value] forKey: @"displayOrder"];
    [self didChangeValueForKey: @"displayOrder"];
}


- (NSString *)name  {

    NSString * tmpValue;
    
    [self willAccessValueForKey: @"name"];
    tmpValue = [self primitiveValueForKey: @"name"];
    [self didAccessValueForKey: @"name"];
    
    return tmpValue;
}


- (void)setName:(NSString *)value  {

    [self willChangeValueForKey: @"name"];
    [self setPrimitiveValue: value forKey: @"name"];
    [self didChangeValueForKey: @"name"];
}


- (short)type  {

    NSNumber *tmpValue;
    
    [self willAccessValueForKey: @"type"];
    tmpValue = [self primitiveValueForKey: @"type"];
    [self didAccessValueForKey: @"type"];
    
    return (tmpValue!=nil) ? [tmpValue shortValue] : 0;
}


- (void)setType:(short)value {

    [self willChangeValueForKey: @"type"];
    [self setPrimitiveValue: [NSNumber numberWithShort: value] forKey: @"type"];
    [self didChangeValueForKey: @"type"];
}


- (void)addIngredientsObject:(Ingredient *)value  {
    
    NSSet *changedObjects = [[NSSet alloc] initWithObjects:&value count:1];

    [self willChangeValueForKey:@"ingredients" withSetMutation:NSKeyValueUnionSetMutation usingObjects:changedObjects];
    [[self primitiveValueForKey: @"ingredients"] addObject: value];
    [self didChangeValueForKey:@"ingredients" withSetMutation:NSKeyValueUnionSetMutation usingObjects:changedObjects];
    
    [changedObjects release];
}


- (void)removeIngredientsObject:(Ingredient *)value  {

    NSSet *changedObjects = [[NSSet alloc] initWithObjects:&value count:1];
    
    [self willChangeValueForKey:@"ingredients" withSetMutation:NSKeyValueMinusSetMutation usingObjects:changedObjects];
    [[self primitiveValueForKey: @"ingredients"] removeObject: value];
    [self didChangeValueForKey:@"ingredients" withSetMutation:NSKeyValueMinusSetMutation usingObjects:changedObjects];
    
    [changedObjects release];
}


- (BOOL)hasIngredients {
    return ( [(NSSet *)[self valueForKey: @"ingredients"] count] > 0 );
}


@end
