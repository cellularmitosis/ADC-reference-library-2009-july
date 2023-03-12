/*

File: Cuisine.m

Abstract: The Cuisine entity (NSManagedObject subclass)

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

#import "Cuisine.h"

#import "Recipe.h"

@implementation Cuisine 


/**
    Overridden initialize method, used to set up the list of dependent keys
    for notification.  Here we want to ensure the recipeCount value is updated
    every time the "recipes" relationship changes.
*/

+ (void)initialize {

    if ( self == [Cuisine class] ) {
        [self setKeys:[NSArray arrayWithObject:@"recipes"] triggerChangeNotificationsForDependentKey:@"recipeCount"];
    }
}


/**
    Returns a predicate to find a Cuisine object similar to this one.  In this
    case we create a predicate simply to match the name of the Cuisine, since 
    there are no other attributes of interest.
*/

- (NSPredicate *) predicateForSimilarObject {
    return [NSPredicate predicateWithFormat: @"name == %@", [self name]];
}


/**
    Creates a copy of this Cuisine object in the specified context and store.  
    This is used to copy Cuisine instances from one store to another (since we 
    cannot create cross-store relationships.)
*/

- (CRManagedObject *) copyToContext: (NSManagedObjectContext *)context andStore:(id)store {

    // insert a new Cuisine first
    Cuisine *newCuisine = [NSEntityDescription insertNewObjectForEntityForName:@"Cuisine" inManagedObjectContext:context];
    if ( store ) {
        [context assignObject:newCuisine toPersistentStore:store];
    }
    
    // use API on the base class to copy the properties (no relationships to copy)
    [newCuisine copyAttributesFromObject: self];
    return newCuisine;
}


/** 
    Returns the number of recipes associated to this cuisine as an integer.
    This API is used in the UI layer to report how many recipes are bound to
    a given cuisine, using the reverse relationship.
*/

- (int)recipeCount  {    

    NSNumber *tmpValue;
    tmpValue = [self valueForKeyPath: @"recipes.@count"];
    
    return (tmpValue!=nil) ? [tmpValue intValue] : 0;
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


- (void)addRecipesObject:(Recipe *)value {    

    NSSet *changedObjects = [[NSSet alloc] initWithObjects:&value count:1];
    
    [self willChangeValueForKey:@"recipes" withSetMutation:NSKeyValueUnionSetMutation usingObjects:changedObjects];
    [[self primitiveValueForKey: @"recipes"] addObject: value];
    [self didChangeValueForKey:@"recipes" withSetMutation:NSKeyValueUnionSetMutation usingObjects:changedObjects];
    
    [changedObjects release];
}


- (void)removeRecipesObject:(Recipe *)value  {

    NSSet *changedObjects = [[NSSet alloc] initWithObjects:&value count:1];
    
    [self willChangeValueForKey:@"recipes" withSetMutation:NSKeyValueMinusSetMutation usingObjects:changedObjects];
    [[self primitiveValueForKey: @"recipes"] removeObject: value];
    [self didChangeValueForKey:@"recipes" withSetMutation:NSKeyValueMinusSetMutation usingObjects:changedObjects];
    
    [changedObjects release];
}

@end
