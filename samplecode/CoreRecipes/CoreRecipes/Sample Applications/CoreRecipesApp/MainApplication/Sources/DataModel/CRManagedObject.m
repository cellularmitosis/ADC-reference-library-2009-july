/*

File: CRManagedObject.m

Abstract: The NSManagedObject subclass used as a base-class for all of
the entities in the CoreRecipes model.  This is used to implement 
custom methods to copy objects between stores.

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

#import "CRManagedObject.h"


@implementation CRManagedObject


/**
    Returns a predicate to find another object that would be considered "equal"
    in attribute value(s).  This predicate is used by an object in one store to
    find a counterpart in another store to use for the purposes of copying an
    object.  We return NO here, but subclasses should override to implement
    something specific.
*/

- (NSPredicate *) predicateForSimilarObject {
    return [NSPredicate predicateWithFormat:NO];
}


/**
    Returns a object from one passed in stores that matches the current object.
    This implementation uses the predicateForExistingObject API to find an
    object that has matching attributes with the current one and returns it.
    If more than one object is found, the first one is returned.
*/

- (CRManagedObject *) similarObjectInContext:(NSManagedObjectContext *)context andStore:(id)store {

    // object to return
    CRManagedObject *object = nil;
    
    // attempt to find a matching object using the built-in predicate
    NSFetchRequest *fetchRequest = [[NSFetchRequest alloc] init];
    [fetchRequest setEntity: [self entity]];
    [fetchRequest setPredicate: [self predicateForSimilarObject]];
    
    // set the affected store, if any
    if ( store ) {
        [fetchRequest setAffectedStores: [NSArray arrayWithObject: store]];
    }
    
    // get the array of matches
    NSArray *matches = [context executeFetchRequest:fetchRequest error:nil];
    
    // if we have a matching object
    if ( matches && [matches count] > 0 ) {
        object = [matches objectAtIndex: 0];
    }

    // clean up
    [fetchRequest release];
    return object;
}


/**
    Method to return a copy of the current object saved in the specified store 
    using the specified context.  (This is used to copy an object from one store
    to another.)  This implementation returns nothing, as subclasses must 
    override to implement the specifics of their copying requirements (for 
    example, if they must copy shallow or deep, use existing objects or create 
    new ones, etc.)
*/

- (CRManagedObject *) copyToContext: (NSManagedObjectContext *)context andStore:(id)store {
    return nil;
}


/**
    Copies all of the attribute values from the specified object into the
    current object.  No class comparison is attempted here (to avoid having to
    walk the inheritance tree), so care should be taken to copy values from an
    object with matching keys.
*/

- (void) copyAttributesFromObject: (NSManagedObject *)managedObject {

    // get the array of attribute keys and set the items accordingly
    NSArray *attributeKeys = [[self entity] attributeKeys];
    NSString *attributeName;
    
    // loop through the keys
    int i, count = [attributeKeys count];
    for ( i=0; i<count; i++ ) {
    
        // Get the attribute name, then copy the value
        attributeName = [attributeKeys objectAtIndex: i];
        [self setPrimitiveValue: [managedObject primitiveValueForKey: attributeName] forKey: attributeName];
    }
}


/**
    Generic accessor to add an object to a to-many relationship specified by a
    key.  Here we create a set with the changed objects, notify the framework
    of the change, and then add the object into the relationship.  This is the
    standard procedure for changing relationship values.
*/

- (void) addObject: (NSManagedObject *)object toToManyRelationshipForKey: (NSString *) key {

    NSSet *changedObjects = [[NSSet alloc] initWithObjects:&object count:1];

    [self willChangeValueForKey:key withSetMutation:NSKeyValueUnionSetMutation usingObjects:changedObjects];
    [[self primitiveValueForKey:key] addObject: object];
    [self didChangeValueForKey:key withSetMutation:NSKeyValueUnionSetMutation usingObjects:changedObjects];

    [changedObjects release];
}


/**
    Copies the object at the end of the to-one relationship for the specified
    key on the specified object to the current object.  This implementation 
    will either use the existing object for the relationship, or will create a
    new instance of the object (copying all of its values) and set it into the
    current object.  This will also set the reverse relationship as necessary.
*/

- (void) copyToOneRelationshipForKey: (NSString *)key fromObject:(NSManagedObject *)managedObject useExisting:(BOOL)useExisting {

    id objectToCopy;

    // get the object to copy
    objectToCopy = [managedObject primitiveValueForKey: key];
    if ( objectToCopy ) {

        // get the context and store for this object
        NSManagedObjectContext *context = [self managedObjectContext];
        id store = [[self objectID] persistentStore];
        id newCopy;

        // if checking for existing
        if ( useExisting ) {
            newCopy = [objectToCopy similarObjectInContext:context andStore:store];
        }
        
        // if not checking existing, or there wasn't one, create a copy
        if ( newCopy == nil ) {
            newCopy = [objectToCopy copyToContext: context andStore: store];
        }

        // set the new copy
        [self setPrimitiveValue: newCopy forKey: key];
        
        // now check to see if we need to set the inverse
        NSRelationshipDescription *desc = [[[self entity] relationshipsByName] objectForKey: key];
        NSRelationshipDescription *inverseDesc = [desc inverseRelationship];
        if ( inverseDesc ) {
        
            // if the relationship is a to-many
            if ( [inverseDesc isToMany] ) {
                [newCopy addObject:self toToManyRelationshipForKey: [inverseDesc name]];
            }
            
            // otherwise set as a to-one
            else {
                [newCopy setPrimitiveValue:self forKey:[inverseDesc name]];
            }
        }
    }
}



/**
    Copies the objects at the end of the to-many relationship for the specified
    key on the specified object to the current object.  This implementation 
    will either use the existing objects for the relationship, or will create
    new instances of the objects (copying all of their values) and set them as
    the relationship for the current object.  This will also set the reverse 
    relationship as necessary.
*/

- (void) copyToManyRelationshipForKey: (NSString *)key fromObject: (NSManagedObject *)managedObject useExisting:(BOOL)useExisting {

    NSArray *relationshipObjects;
    
    // get the array of relationship objects
    relationshipObjects = [[managedObject primitiveValueForKey: key] allObjects];
    if ( relationshipObjects && [relationshipObjects count] > 0 ) {
    
        // get the context and the store for this object
        NSManagedObjectContext *context = [self managedObjectContext];
        id store = [[self objectID] persistentStore];
        
        // see if we need to look for an inverse
        NSRelationshipDescription *desc = [[[self entity] relationshipsByName] objectForKey: key];
        NSRelationshipDescription *inverseDesc = [desc inverseRelationship];
        BOOL isToMany = [inverseDesc isToMany];
        
        // interate the array of relationship objects
        int i, count = [relationshipObjects count];
        id objectToCopy = nil;
        id newCopy = nil;

        for ( i = 0; i < count; i++ ) {

            // get the object
            objectToCopy = [relationshipObjects objectAtIndex:i];

            // if checking for existing
            if ( useExisting ) {
                newCopy = [objectToCopy similarObjectInContext:context andStore:store];
            }
            
            // if not checking existing, or there wasn't one, create a copy
            if ( newCopy == nil ) {
                newCopy = [objectToCopy copyToContext:context andStore:store];
            }

            // add the value to the relationship
            [self addObject:newCopy toToManyRelationshipForKey:key];

            // process the inverse, if we need to
            if ( inverseDesc ) {
            
                // to-many
                if ( isToMany ) {
                    [newCopy addObject:self toToManyRelationshipForKey: [inverseDesc name]];
                }
                
                // otherwise
                else {
                    [newCopy setPrimitiveValue:self forKey:[inverseDesc name]];
                }
            }
            
            // reset 
            newCopy = nil;
        }
    }

    // more cleanup
    relationshipObjects = nil;
}

@end
