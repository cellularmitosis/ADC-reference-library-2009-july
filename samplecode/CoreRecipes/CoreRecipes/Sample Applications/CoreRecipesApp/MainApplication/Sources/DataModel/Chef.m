/*

File: Chef.m

Abstract: The Chef entity (NSManagedObject subclass)

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

#import "Chef.h"
#import "Recipe.h"

@implementation Chef 


/**
    Overridden initialize method, used to set up the list of dependent keys
    for notification.  Here we want to ensure the fullName is updated each time
    either the first or last name attributes changes.
*/

+ (void)initialize {

    if ( self == [Chef class] ) {
        [self setKeys:[NSArray arrayWithObjects:@"firstName", @"lastName", nil] triggerChangeNotificationsForDependentKey:@"fullName"];
    }
}


/**
    Returns a predicate to find an object similar to this Chef.  In this case
    we create a predicate to match the names, homepage, and "Iron Chef" value
    of this object.
*/

- (NSPredicate *) predicateForSimilarObject {
    return [NSPredicate predicateWithFormat: @"(lastName == %@) && (firstName == %@) && (homepage == %@) && (isIronChef == %@)",
        [self lastName], [self firstName], [self homepage], [NSNumber numberWithBool:[self isIronChef]]];
}


/**
    Creates a copy of this Chef object in the specified context and store.  This
    is used to copy Chef instances from one store to another (since we cannot
    create cross-store relationships.)
*/

- (CRManagedObject *) copyToContext: (NSManagedObjectContext *)context andStore:(id)store {

    // insert a new Chef first
    Chef *newChef = [NSEntityDescription insertNewObjectForEntityForName:@"Chef" inManagedObjectContext:context];
    if ( store ) {
        [context assignObject:newChef toPersistentStore:store];
    }
    
    // use API on the base class to copy the properties (no relationships to copy)
    [newChef copyAttributesFromObject: self];
    return newChef;
}


/**
    Returns a boolean indicating if the Chef has recipes or not.  In this case
    we simply examine the "recipes" relationship and return YES if the count of
    the related objects is more than zero.
*/

- (BOOL)hasRecipes {
    return ( [(NSSet *)[self valueForKey: @"recipes"] count] > 0 );
}



#pragma mark
#pragma mark Core Data accessors/mutators/validation methods
#pragma mark


- (NSString *)homepage  {

    NSString * tmpValue;
    
    [self willAccessValueForKey: @"homepage"];
    tmpValue = [self primitiveValueForKey: @"homepage"];
    [self didAccessValueForKey: @"homepage"];
    
    return tmpValue;
}


- (void)setHomepage:(NSString *)value  {

    [self willChangeValueForKey: @"homepage"];
    [self setPrimitiveValue: value forKey: @"homepage"];
    [self didChangeValueForKey: @"homepage"];
}


- (NSString *)fullName {
    return [NSString stringWithFormat:@"%@ %@", [self firstName], [self lastName]];
}


- (NSString *)firstName  {

    NSString * tmpValue;
    
    [self willAccessValueForKey: @"firstName"];
    tmpValue = [self primitiveValueForKey: @"firstName"];
    [self didAccessValueForKey: @"firstName"];
    
    return tmpValue;
}


- (void)setFirstName:(NSString *)value  {

    [self willChangeValueForKey: @"firstName"];
    [self setPrimitiveValue: value forKey: @"firstName"];
    [self didChangeValueForKey: @"firstName"];
}


- (NSString *)lastName  {

    NSString * tmpValue;
    
    [self willAccessValueForKey: @"lastName"];
    tmpValue = [self primitiveValueForKey: @"lastName"];
    [self didAccessValueForKey: @"lastName"];
    
    return tmpValue;
}


- (void)setLastName:(NSString *)value  {

    [self willChangeValueForKey: @"lastName"];
    [self setPrimitiveValue: value forKey: @"lastName"];
    [self didChangeValueForKey: @"lastName"];
}


- (BOOL)isIronChef  {

    NSNumber *tmpValue;
    
    [self willAccessValueForKey: @"isIronChef"];
    tmpValue = [self primitiveValueForKey: @"isIronChef"];
    [self didAccessValueForKey: @"isIronChef"];
    
    return (tmpValue!=nil) ? [tmpValue boolValue] : FALSE;
}


- (void)setIsIronChef:(BOOL)value  {

    [self willChangeValueForKey: @"isIronChef"];
    [self setPrimitiveValue: [NSNumber numberWithBool: value] forKey: @"isIronChef"];
    [self didChangeValueForKey: @"isIronChef"];
}


- (void)addRecipesObject:(Recipe *)value {    

    NSSet *changedObjects = [[NSSet alloc] initWithObjects:&value count:1];
    
    [self willChangeValueForKey:@"recipes" withSetMutation:NSKeyValueUnionSetMutation usingObjects:changedObjects];
    [[self primitiveValueForKey: @"recipes"] addObject: value];
    [self didChangeValueForKey:@"recipes" withSetMutation:NSKeyValueUnionSetMutation usingObjects:changedObjects];
    
    [changedObjects release];
}


- (void)removeRecipesObject:(Recipe *)value {

    NSSet *changedObjects = [[NSSet alloc] initWithObjects:&value count:1];
    
    [self willChangeValueForKey:@"recipes" withSetMutation:NSKeyValueMinusSetMutation usingObjects:changedObjects];
    [[self primitiveValueForKey: @"recipes"] removeObject: value];
    [self didChangeValueForKey:@"recipes" withSetMutation:NSKeyValueMinusSetMutation usingObjects:changedObjects];
    
    [changedObjects release];
}


@end
