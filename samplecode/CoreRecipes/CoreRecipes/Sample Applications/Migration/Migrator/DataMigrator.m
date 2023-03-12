/*
 
 File: DataMigrator.m
 
 Abstract: <<ABSTRACT CONTENT HERE>>
 
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

#import "DataMigrator.h"
#import "MigrationUtilities.h"

/** DataMigrator performs the actual work of migrating existing data from a data store 
 *  based on the old data model to a newly created data store based on the new model.  
 *  
 *  During the actual migration, independent core data stacks are used for the old and new
 *  managed object data spaces.  No managed objects are passed between these data stacks,
 *  see normalizeCuisines:, migrateRecipesAndIngredients: and migrateChefs:.
 */
@implementation DataMigrator

- (NSManagedObjectModel *)oldManagedObjectModel {
    if (oldManagedObjectModel) {
        return oldManagedObjectModel;
    }
	
    oldManagedObjectModel = [MigrationUtilities retainedOldManagedObjectModel];
    
    return oldManagedObjectModel;
}

- (NSManagedObjectModel *)newManagedObjectModel {
    if (newManagedObjectModel) {
        return newManagedObjectModel;
    }
	
    newManagedObjectModel = [MigrationUtilities retainedNewManagedObjectModel];
    
    // Tweak the model so the recipe->chef relationship is no longer required
    // This is necessary because of the way we move the object graph from 
    // V1 to V2 in pieces instead of all at once
    
    [[[[[newManagedObjectModel entitiesByName] valueForKey: @"Recipe"] relationshipsByName] valueForKey: @"chef"] setOptional: YES];
    
    return newManagedObjectModel;
}


- (NSManagedObjectContext *)createOldCoreDataStackWithStorePath:(NSString *)storePath {
    if (oldManagedObjectContext) {
        return oldManagedObjectContext;
    }
    
    oldManagedObjectContext = [MigrationUtilities createRetainedCoreDataStackWithStorePath: storePath andModel: [self oldManagedObjectModel]];

    return oldManagedObjectContext;
}

- (NSManagedObjectContext *)createNewCoreDataStackWithStorePath:(NSString *)newStorePath {
    if (newManagedObjectContext) {
        return newManagedObjectContext;
    }

    newManagedObjectContext = [MigrationUtilities createRetainedCoreDataStackWithStorePath: newStorePath andModel: [self newManagedObjectModel]];

    // Update the metadata to V2
    if (nil != newManagedObjectContext) {
        [MigrationUtilities setMetadata: @"Version 2" forKey: @"Migration Example Version" inStoreWithPath: newStorePath inContext: newManagedObjectContext];        
    }

    return newManagedObjectContext;
}

- (id)initWithOldStorePath:(NSString *)oldStorePath newStorePath:(NSString *)newStorePath {
    self = [super init];
    if (nil != self) {
        [self createOldCoreDataStackWithStorePath: oldStorePath];
        [self createNewCoreDataStackWithStorePath: newStorePath];
    }
    return self;
}

- (void)dealloc {
    [newManagedObjectModel release];
    newManagedObjectModel = nil;
    [newManagedObjectContext release];
    newManagedObjectContext = nil;
    [oldManagedObjectModel release];
    oldManagedObjectModel = nil;
    [oldManagedObjectContext release];
    oldManagedObjectContext = nil;
    
    [super dealloc];
}

// The first step in the migration process: iterate through the existing Recipes, and create
// Cuisine objects encapsulating what used to be a string attribute. Also makes sure to unique
// the Cuisine names, since we only want one instance each of "Cuisine A", "Cuisine B", and 
// "Cuisine C", even though they appear in more than one recipe
- (BOOL)normalizeCuisines:(NSError **)error {
    NSFetchRequest *oldRecipesFetchRequest = [[[NSFetchRequest alloc] init] autorelease];
    [oldRecipesFetchRequest setEntity: [[oldManagedObjectModel entitiesByName] valueForKey: @"Recipe"]];
    NSArray *oldRecipes = [oldManagedObjectContext executeFetchRequest: oldRecipesFetchRequest error: error];
    
    if (nil == *error) {
        NSArray *oldCuisines = [oldRecipes valueForKey: @"cuisine"];
        NSArray *newCuisines = [NSMutableDictionary dictionary];
        int i = 0, count = [oldCuisines count];
        
        
        // Go through all old recipes' cuisine names and create new managed objects for each unique value
        for ( ; i < count ; i++ ) {
            NSString *cuisineName = [oldCuisines objectAtIndex: i];
            if (nil == [newCuisines valueForKey: cuisineName]) {
                // We don't have a cuisine with this name yet, so create one
                NSManagedObject *cuisine = [NSEntityDescription insertNewObjectForEntityForName: @"Cuisine" inManagedObjectContext: newManagedObjectContext];
                [cuisine setValue: cuisineName forKey: @"name"];
                [newCuisines setValue: cuisine forKey: cuisineName];
            }
        }
        [newManagedObjectContext save: error];
        if (nil !=* error) {
            NSLog(@"Cuisine normalization error");
            return NO;
        }
        return YES;
    }
    return NO;
}

// The second step: migrate all of the Recipes and their Ingredients. Interesting features here
// are the use of the default value defined in the model for the new 'rating' attribute on the 
// Recipe entity, and the  use of the fetch request to find the new instance of the correct 
// Cuisine object for each recipe  as created in the previous step. We take advantage of the 
// model modification made in newManagedObjectModel to leave the Chef relationship nil for now 
// even though the model specifies that it is required. This relationship will be connected
// in the next step.
// The Ingredient transform is simple: nothing in the model changed, so we simply copy over the
// new values.
- (BOOL)migrateRecipesAndIngredients:(NSError **)error {
    NSFetchRequest *oldRecipesFetchRequest = [[[NSFetchRequest alloc] init] autorelease];
    [oldRecipesFetchRequest setEntity: [[oldManagedObjectModel entitiesByName] valueForKey: @"Recipe"]];
    NSArray *oldRecipes = [oldManagedObjectContext executeFetchRequest: oldRecipesFetchRequest error: error];
    
    if (nil == *error) {
        int i = 0, recipeCount = [oldRecipes count];
        
        // There's a default recipe rating value in the new model, so use it when we do the switch
        NSNumber *defaultRating = [[[[[newManagedObjectModel entitiesByName] valueForKey: @"Recipe"] attributesByName] valueForKey: @"rating"] defaultValue];
        
        // Set up a fetch request that I can reuse to find the new cuisines objects for my recipes
        NSFetchRequest *cuisineFetchRequest = [[[NSFetchRequest alloc] init] autorelease];
        [cuisineFetchRequest setEntity: [[newManagedObjectModel entitiesByName] valueForKey: @"Cuisine"]];
        
        for ( ; i < recipeCount ; i++ ) {
            NSManagedObject *oldRecipe = [oldRecipes objectAtIndex: i];
            NSManagedObject *newRecipe = [NSEntityDescription insertNewObjectForEntityForName: @"Recipe" inManagedObjectContext: newManagedObjectContext];
            
            // Copy values that haven't changed over
            [newRecipe setValue: [oldRecipe valueForKey: @"name"] forKey: @"name"];
            [newRecipe setValue: [oldRecipe valueForKey: @"directions"] forKey: @"directions"];
            
            // New value for new attribute
            [newRecipe setValue: defaultRating forKey: @"rating"];
            
            // Find the cuisine containing the value we normalized out in step 1 and connect it
            [cuisineFetchRequest setPredicate: [NSPredicate predicateWithFormat: [NSString stringWithFormat: @"name = \"%@\"", [oldRecipe valueForKey: @"cuisine"]]]];
            
            NSArray *cuisines = [newManagedObjectContext executeFetchRequest: cuisineFetchRequest error: error];
            
            if (nil !=* error) {
                NSLog(@"Can't find cuisine error");
            }    
            
            [[newRecipe mutableSetValueForKey: @"cuisines"] addObject: [cuisines objectAtIndex: 0]];
            
            // Now get the ingredients
            NSEnumerator *oldIngredientsEnumerator = [(NSSet *)[oldRecipe valueForKey: @"ingredients"] objectEnumerator];
            NSManagedObject *oldIngredient = nil;
            
            while (oldIngredient = (NSManagedObject *)[oldIngredientsEnumerator nextObject]) {
                NSManagedObject *newIngredient = [NSEntityDescription insertNewObjectForEntityForName: @"Ingredient" inManagedObjectContext: newManagedObjectContext];
                
                // And do the copying, since nothing interesting changed here
                [newIngredient setValue: [oldIngredient valueForKey: @"name"] forKey: @"name"];
                [newIngredient setValue: [oldIngredient valueForKey: @"amount"] forKey: @"amount"];
                
                [[newRecipe mutableSetValueForKey: @"ingredients"] addObject: newIngredient];
            }
        }
        
        [newManagedObjectContext save: error];
        if (nil !=* error) {
            NSLog(@"Recipe/ingredients save error");
            return NO;
        }        
        return YES;
    }
    return NO;
}

// Creates new Chef instances in the new context, and populates them with the data
// from the old store. The previous 'name' attribute is separated into 'firstName' and
// 'lastName' subcomponents, the 'training' attribute is dropped, and the Recipes 
// relationship is populated from the results of a fetch request in the new store.
// Note that this relationship changed between models, in V1 it was to-one, in V2 it
// is to-many.
- (BOOL)migrateChefs:(NSError **)error {
    NSFetchRequest *oldChefFetchRequest = [[[NSFetchRequest alloc] init] autorelease];
    [oldChefFetchRequest setEntity: [[oldManagedObjectModel entitiesByName] valueForKey: @"Chef"]];
    [oldChefFetchRequest setSortDescriptors: [NSArray arrayWithObject: [[[NSSortDescriptor alloc] initWithKey: @"name" ascending: YES] autorelease]]];
    NSArray *oldChefs = [oldManagedObjectContext executeFetchRequest: oldChefFetchRequest error: error];

    if (nil == *error) {
        int chefCount = [oldChefs count];
        int i = 0;

        NSFetchRequest *newChefRecipeFetchRequest = [[[NSFetchRequest alloc] init] autorelease];
        [newChefRecipeFetchRequest setEntity: [[newManagedObjectModel entitiesByName] valueForKey: @"Recipe"]];

        for ( ; i < chefCount ; i++ ) {
            NSManagedObject *oldChef = [oldChefs objectAtIndex: i];
            NSManagedObject *oldChefRecipe = [oldChef valueForKey: @"recipes"];

            // predicate to find the new version of this chef's recipe, since we've already created and saved it
            [newChefRecipeFetchRequest setPredicate: [NSPredicate predicateWithFormat: [NSString stringWithFormat: @"name = \"%@\"", [oldChefRecipe valueForKey: @"name"]]]];
            
            NSArray *newChefRecipe = [newManagedObjectContext executeFetchRequest: newChefRecipeFetchRequest error: error];
            
            if (nil != *error) {
                NSLog(@"Can't find recipe %@" , [oldChefRecipe valueForKey: @"name"]);
            }
                        
            NSManagedObject *newChef = [NSEntityDescription insertNewObjectForEntityForName: @"Chef" inManagedObjectContext: newManagedObjectContext];
            NSString *oldChefName = [oldChef valueForKey: @"name"];
            NSArray *oldChefNameComponents = [oldChefName componentsSeparatedByString: @", "];
            // Split the previous 'name' attribute into the firstName and lastName parts
            [newChef setValue: [oldChefNameComponents objectAtIndex: 0] forKey: @"lastName"];
            [newChef setValue: [oldChefNameComponents objectAtIndex: 1] forKey: @"firstName"];
            // Don't copy over training, since we've dropped it from the model
            // Recipes was a to-one relationship, now it's a to-many, so we need to put it into the mutableSetValueForKey
            [[newChef mutableSetValueForKey: @"recipes"] addObject: [newChefRecipe objectAtIndex: 0]];
        }
        
        [newManagedObjectContext save: error];
        if (nil !=* error) {
            NSLog(@"Chef fetch error");
            return NO;
        }
        return YES;
    }
    return NO;
}


- (BOOL)migrateData:(NSError **)error {
    if ([self normalizeCuisines: error]) {
        if ([self migrateRecipesAndIngredients: error]) {
            if ([self migrateChefs: error]) {
                return YES;
            }
        }
    }
    return NO;
}

@end
