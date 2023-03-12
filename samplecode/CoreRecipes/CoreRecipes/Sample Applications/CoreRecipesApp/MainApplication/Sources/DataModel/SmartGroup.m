/*

File: SmartGroup.m

Abstract: The SmartGroup entity (NSManagedObject subclass)

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

#import "SmartGroup.h"
#import "InferenceCore.h"


@implementation SmartGroup 


/** 
    Called from awakeFromInsert and awakeFromFetch. Here we register for the 
    notification for changes to the managed object context, in order to be able 
    to refresh the smart group when the object graph changes.
*/

- (void)commonAwake {

    recipes = nil;
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(refresh:) 
        name:NSManagedObjectContextObjectsDidChangeNotification object:[self managedObjectContext]];        

    [self willAccessValueForKey:@"priority"];
    [self setValue:[NSNumber numberWithInt:2] forKeyPath:@"priority"];
    [self didAccessValueForKey:@"priority"];
}


/** 
    Overridden awakeFromInsertion method, used to call the commonAwake 
    implementation (to register for the notification of changes to the 
    predicate.)  We also ensure the predicate is something valid here.
*/

- (void)awakeFromInsert  {

    // awake from insert
    [super awakeFromInsert];
    [self commonAwake];
    
    // create an initial predicate
    [self setPredicate: [NSPredicate predicateWithValue: YES]];
}


/** 
    Overridden awakeFromFetch method, used to call the commonAwake 
    implementation (to register for the notification of changes to the 
    predicate.)
*/


- (void)awakeFromFetch  {

    [super awakeFromFetch];
    [self commonAwake];
}


/** 
    Overridden didTurnIntoFault method.  In addition to releasing the array of 
    recipes, the fetch request, and the predicate, we need to unregister for the 
    notifications from the managed object context before dealloc-ing.
*/

- (void)didTurnIntoFault {

    [[NSNotificationCenter defaultCenter] removeObserver: self 
        name:NSManagedObjectContextObjectsDidChangeNotification object:[self managedObjectContext]];

    [recipes release], recipes = nil;
    [fetchRequest release], fetchRequest = nil;
    [predicate release], predicate = nil;
    
    [super didTurnIntoFault];
}


/**
    Method to refresh the content of the smart group.  Here we simply note the
    contents of the "recipes" array is going to change, and then release the
    array.  A new one will be lazily created as necessary.
*/

- (void)refresh {

	[self willChangeValueForKey:@"summaryString"];
	[self willChangeValueForKey:@"recipes"];
	[recipes release], recipes = nil;    
	[self didChangeValueForKey:@"recipes"];
	[self didChangeValueForKey:@"summaryString"];
}


/**
    Method to refresh the SmartGroup object.  This method is invoked either when 
    the predicate changes OR when object change notifications are received from 
    the context.  Since a change to a predicate is immediately pushed into the 
    fetch request, all we need do here is clear the array of recipes so it will 
    be re-created on the next access.
*/

- (void)refresh:(NSNotification *)notification {

    // Performance and Infinite loop avoidance:  Only refresh if the 
    // updated/deleted/inserted objects include Recipes (the entity of the 
    // [self fetchRequest]) We don't want to re-fetch recipes if unrelated 
    // objects (for example, other smart groups) change.
	
	NSEnumerator *enumerator;
	id object;
	BOOL refresh = NO;
	
	NSEntityDescription *entity = [[self fetchRequest] entity];
	
	NSSet *updated = [[notification userInfo] objectForKey:NSUpdatedObjectsKey];
	NSSet *inserted = [[notification userInfo] objectForKey:NSInsertedObjectsKey];
	NSSet *deleted = [[notification userInfo] objectForKey:NSDeletedObjectsKey];
	
	enumerator = [updated objectEnumerator];	
	while ((refresh == NO) && (object = [enumerator nextObject])) {
		if ([object entity] == entity) {
			refresh = YES;	
		}
	}

	enumerator = [inserted objectEnumerator];	
	while ((refresh == NO) && (object = [enumerator nextObject])) {
		if ([object entity] == entity) {
			refresh = YES;	
		}
	}

	enumerator = [deleted objectEnumerator];	
	while ((refresh == NO) && (object = [enumerator nextObject])) {
		if ([object entity] == entity) {
			refresh = YES;	
		}
	}
	    
    if ( (refresh == NO) && (([updated count] == 0) && ([inserted count] == 0) && ([deleted count]==0))) {
        refresh = YES;
    }
    
	// OPTIMIZATION TIP:  We could collect all of the Recipe objects from the 
    // inserted and updated NSSets and add them to an array. Filter the array 
    // using [self predicate]. Only if the filtered array is non-empty would we 
    // need to update our recipes set (we could simply add the objects to the 
    // set)

	// OPTIMIZATION TIP: We could remove the objects of the deleted set 
    // directly from our recipes set ([recipes minusSet:deleted]).
	
    if (refresh) {
		[self refresh];
    }
}


/**
    Accessor for the fetch request for the SmartGroup.  The fetch
    request is used to fetch all of the matching objects for the predicate for 
    the SmartGroup.  The fetch request is only created once per object (since 
    the entity will not change), though the predicate for the request can 
    change as needed.
*/

- (NSFetchRequest *)fetchRequest  {

    if ( fetchRequest == nil ) {

        // create the fetch request for the recipes
        fetchRequest = [[NSFetchRequest alloc] init];
        [fetchRequest setEntity: [NSEntityDescription entityForName:@"Recipe" inManagedObjectContext:[self managedObjectContext]]];

        // set the affected stores
        id store = [[self objectID] persistentStore];
        if (store != nil) {
            [fetchRequest setAffectedStores:[NSArray arrayWithObject:store]];
        }

        // set the predicate
        [fetchRequest setPredicate: [self predicate]];
    }
    
    return fetchRequest;
}


/** 
    Accessor for the predicate for the SmartGroup instance.  The value of the 
    instance variable comes from decoding the NSData for the "predicateData" 
    attribute (where the predicate is archived.)
*/

- (NSPredicate *)predicate {

    NSData *predicateData;
    if ( predicate == nil ) {

        predicateData = [self valueForKey: @"predicateData"];
        if ( predicateData != nil ) {
            predicate = [(NSPredicate *)[NSKeyedUnarchiver unarchiveObjectWithData: predicateData] retain];
        }
    }
    
    return predicate;
}


/**
    Mutator method for the predicate for the SmartGroup instance.  When a new 
    predicate is set for the SmartGroup, we must first store the new value in 
    the instance variable, then encode it into a data representation (to set 
    into the model), and then update the fetch request for the smart group with 
    the new predicate.
*/

- (void)setPredicate: (NSPredicate *)newPredicate {

    if ( predicate != newPredicate )  {

        // release the old
        [predicate autorelease];

        // ensure we have a predicate
        if ( newPredicate == nil ) {
            newPredicate = [NSPredicate predicateWithValue: YES];
        }
        
        // retain the new predicate and update the data
        predicate = [newPredicate retain];
		NSData *predicateData = [NSKeyedArchiver archivedDataWithRootObject:predicate];
        [self setValue: predicateData forKey: @"predicateData"];

        // update the fetch request
        [[self fetchRequest] setPredicate: predicate];
		[self refresh];
    }
}

- (NSSet *)items {
    return nil;
}


- (void)setItems:(NSSet *)items {
    // noop
}

/** 
    Accesor for the array of recipes for the SmartGroup.  This implementation 
    returns the objects matching the specified predicate using the cached fetch 
    request.  An empty set (not nil) is returned if there are no objects to be 
    found OR if an error was encountered with the fetch.
*/

- (NSSet *)recipes {

    if ( recipes == nil )  {

        // variables
        NSError *error = nil;
        NSArray *results = nil;
        
        // in case the predicate is bad
        @try {  results = [[self managedObjectContext] executeFetchRequest:[self fetchRequest] error:&error];  }
        @catch ( NSException *e ) {  /* no-op */ }
        
        // use an empty set in the case where something went awry
        recipes = ( error != nil || results == nil) ? [[NSSet alloc] init] : [[NSSet alloc] initWithArray:results];
    }

    return recipes;
}


/** 
    Recipes for smart groups aren't really settable. Ensure that nothing tries 
    to mutate recipes by KVC.
*/

- (void)setRecipes:(NSSet *)newRecipes  {
    // noop   
}


/**
    Mutator for the group image name.  This name is used in the display views
    for the groups (in order to help differentiate specialized forms of the 
    smart group.)
*/

- (void)setGroupImageName:(NSString *)inGroupImageName {

    if (![groupImageName isEqualToString:inGroupImageName]) {
        [groupImageName release];
        groupImageName = [inGroupImageName retain];            
    }
}


/**
    Returns the name of the image used to represent this smart group instance
    in the view.  If no value has been specified, the default image name is
    returned.
*/

- (NSString *)groupImageName {
    return (groupImageName != nil) ? groupImageName : @"image_group_smart";
}

- (BOOL)isLeaf {
    return YES;
}


- (BOOL)isSmartGroup {
    return YES;
}


- (BOOL)canEditPredicate {
    return YES;
}


/**
    The summary string for the group.  This implementation returns a string with
    the count of the recipes in the group, as well as a count of how many days
    the recipes would last for (assuming three meals a day...)
*/

- (NSString *)summaryString {

    int recipeCount = [recipes count];
    return [NSString stringWithFormat: @"%i recipes, %.2f day%@", 
        recipeCount, (float)recipeCount/3, ( recipeCount/3 == 1 ? @"" : @"s" )];
}

@end
