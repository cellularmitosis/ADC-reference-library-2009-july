/*

File: Recipe.m

Abstract: The "Recipe" entity (NSManagedObject subclass)

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


#import "Recipe.h"

#import "RecipeMedia.h"
#import "Ingredient.h"
#import "Cuisine.h"
#import "Chef.h"

@implementation Recipe 


/**
    Overridden initialize method, used to set up the list of dependent keys
    for notification.  Here we want to ensure the cuisinesArray value is updated
    every time the "cuisines" relationship changes.
*/

+ (void)initialize {
    
    if ( self == [Recipe class] ) {
        [self setKeys:[NSArray arrayWithObject:@"cuisines"] triggerChangeNotificationsForDependentKey:@"cuisineArray"];
    }
}


/**
    Creates a copy of this Recipe object in the specified context and store.  
    This is used to copy instances from one store to another (since we cannot 
    create cross-store relationships.)
*/

- (CRManagedObject *) copyToContext: (NSManagedObjectContext *)context andStore:(id)store {

    // insert a new Recipe first
    Recipe *newRecipe = [NSEntityDescription insertNewObjectForEntityForName:@"Recipe" inManagedObjectContext:context];
    if ( store ) {
        [context assignObject:newRecipe toPersistentStore:store];
    }
    
    // use API on the base class to copy the properties
    [newRecipe copyAttributesFromObject: self];
        
    // copy the ingredients, media, chef, and cuisine.  note that we want to always create
    // new ingredient and media copies, but will use existing chef and cuisines
    [newRecipe copyToOneRelationshipForKey: @"chef" fromObject:self useExisting:YES];
    [newRecipe copyToManyRelationshipForKey: @"cuisines" fromObject:self useExisting:YES];
    [newRecipe copyToManyRelationshipForKey: @"ingredients" fromObject:self useExisting:NO];
    [newRecipe copyToManyRelationshipForKey: @"recipeMedia" fromObject:self useExisting:NO];
    
    // return
    return newRecipe;
}


/**
    Returns the contents of the "cuisines" relationship in array format, rather
    than the Core Data standard set format.  This is a convenience accessor that
    is dependently bound to the "cuisines" relationship.
*/

- (NSArray *)cuisineArray {
    return [[self valueForKey:@"cuisines"] allObjects];    
}


/**
    Returns a comma separated string of the cuisine (names) for this recipe.
    This is a convenience accessor in order to display all of the cuisines for 
    a given recipe in a single view (cell).
*/

- (NSString *) cuisinesAsString {

    // Return a string of the cuisine names separated by commas
    NSArray *cuisines = [self cuisineArray];
    if ( cuisines && [cuisines count] > 0 ) {
        return [[cuisines valueForKey: @"name"] componentsJoinedByString: @", "];
    }
    
    // no cuisines -- return nothing
    return nil;
}

- (NSString *)chefFullName {
    return [[self chef] valueForKey:@"fullName"];
}

/**
    Returns the icon for the recipe.  In this case, the icon is hard-coded for
    all recipes, but some day it may be necessary to change this implementation
    to include a mutator so some recipes could have different icons.
*/

- (NSImage *)recipeIcon {
    return [NSImage imageNamed:@"image_recipe"];
}


/**
    Returns the display color for the Recipes" entity:  in this case, Recipes
    always use "green" as their display color.
*/

- (NSColor *)displayColor {
    NSColor *color = [NSColor greenColor];
    return color;
}


#pragma mark
#pragma mark Core Data accessors/mutators/validation methods
#pragma mark

- (short)cookingTime  {

    NSNumber *tmpValue;
    
    [self willAccessValueForKey: @"cookingTime"];
    tmpValue = [self primitiveValueForKey: @"cookingTime"];
    [self didAccessValueForKey: @"cookingTime"];
    
    return (tmpValue!=nil) ? [tmpValue shortValue] : 0;
}


- (void)setCookingTime:(short)value  {

    [self willChangeValueForKey: @"cookingTime"];
    [self setPrimitiveValue: [NSNumber numberWithShort: value]
                     forKey: @"cookingTime"];
    [self didChangeValueForKey: @"cookingTime"];
}


- (NSCalendarDate *)lastPreparationDate {

    NSCalendarDate * tmpValue;
    
    [self willAccessValueForKey: @"lastPreparationDate"];
    tmpValue = [self primitiveValueForKey: @"lastPreparationDate"];
    [self didAccessValueForKey: @"lastPreparationDate"];
    
    return tmpValue;
}


- (void)setLastPreparationDate:(NSCalendarDate *)value {

    [self willChangeValueForKey: @"lastPreparationDate"];
    [self setPrimitiveValue: value forKey: @"lastPreparationDate"];
    [self didChangeValueForKey: @"lastPreparationDate"];
}


- (NSString *)detailDescription {

    NSString * tmpValue;
    
    [self willAccessValueForKey: @"detailDescription"];
    tmpValue = [self primitiveValueForKey: @"detailDescription"];
    [self didAccessValueForKey: @"detailDescription"];
    
    return tmpValue;
}


- (void)setDetailDescription:(NSString *)value {

    [self willChangeValueForKey: @"detailDescription"];
    [self setPrimitiveValue: value forKey: @"detailDescription"];
    [self didChangeValueForKey: @"detailDescription"];
}


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


- (short)preparationTime {

    NSNumber *tmpValue;
    
    [self willAccessValueForKey: @"preparationTime"];
    tmpValue = [self primitiveValueForKey: @"preparationTime"];
    [self didAccessValueForKey: @"preparationTime"];
    
    return (tmpValue!=nil) ? [tmpValue shortValue] : 0;
}


- (void)setPreparationTime:(short)value {

    [self willChangeValueForKey: @"preparationTime"];
    [self setPrimitiveValue: [NSNumber numberWithShort: value]
                     forKey: @"preparationTime"];
    [self didChangeValueForKey: @"preparationTime"];
}


- (NSData *)directions {

    NSData * tmpValue;
    
    [self willAccessValueForKey: @"directions"];
    tmpValue = [self primitiveValueForKey: @"directions"];
    [self didAccessValueForKey: @"directions"];
    
    return tmpValue;
}


- (void)setDirections:(NSData *)value {

    [self willChangeValueForKey: @"directions"];
    [self setPrimitiveValue: value forKey: @"directions"];
    [self didChangeValueForKey: @"directions"];
}


- (short)numberOfServings {

    NSNumber *tmpValue;
    
    [self willAccessValueForKey: @"numberOfServings"];
    tmpValue = [self primitiveValueForKey: @"numberOfServings"];
    [self didAccessValueForKey: @"numberOfServings"];
    
    return (tmpValue!=nil) ? [tmpValue shortValue] : 0;
}


- (void)setNumberOfServings:(short)value {

    [self willChangeValueForKey: @"numberOfServings"];
    [self setPrimitiveValue: [NSNumber numberWithShort: value]
                     forKey: @"numberOfServings"];
    [self didChangeValueForKey: @"numberOfServings"];
}


- (void)addRecipeMediaObject:(RecipeMedia *)value {    

    NSSet *changedObjects = [[NSSet alloc] initWithObjects:&value count:1];
    
    [self willChangeValueForKey:@"recipeMedia" withSetMutation:NSKeyValueUnionSetMutation usingObjects:changedObjects];
    [[self primitiveValueForKey: @"recipeMedia"] addObject: value];
    [self didChangeValueForKey:@"recipeMedia" withSetMutation:NSKeyValueUnionSetMutation usingObjects:changedObjects];
    
    [changedObjects release];
}


- (void)removeRecipeMediaObject:(RecipeMedia *)value {

    NSSet *changedObjects = [[NSSet alloc] initWithObjects:&value count:1];
    
    [self willChangeValueForKey:@"recipeMedia" withSetMutation:NSKeyValueMinusSetMutation usingObjects:changedObjects];
    [[self primitiveValueForKey: @"recipeMedia"] removeObject: value];
    [self didChangeValueForKey:@"recipeMedia" withSetMutation:NSKeyValueMinusSetMutation usingObjects:changedObjects];
    
    [changedObjects release];
}


- (void)addIngredientsObject:(Ingredient *)value {    

    NSSet *changedObjects = [[NSSet alloc] initWithObjects:&value count:1];
    
    [self willChangeValueForKey:@"ingredients" withSetMutation:NSKeyValueUnionSetMutation usingObjects:changedObjects];
    [[self primitiveValueForKey: @"ingredients"] addObject: value];
    [self didChangeValueForKey:@"ingredients" withSetMutation:NSKeyValueUnionSetMutation usingObjects:changedObjects];
    
    [changedObjects release];
}


- (void)removeIngredientsObject:(Ingredient *)value {

    NSSet *changedObjects = [[NSSet alloc] initWithObjects:&value count:1];
    
    [self willChangeValueForKey:@"ingredients" withSetMutation:NSKeyValueMinusSetMutation usingObjects:changedObjects];
    [[self primitiveValueForKey: @"ingredients"] removeObject: value];
    [self didChangeValueForKey:@"ingredients" withSetMutation:NSKeyValueMinusSetMutation usingObjects:changedObjects];
    
    [changedObjects release];
}


- (void)addCuisinesObject:(Cuisine *)value {    

    NSSet *changedObjects = [[NSSet alloc] initWithObjects:&value count:1];
    
    [self willChangeValueForKey:@"cuisines" withSetMutation:NSKeyValueUnionSetMutation usingObjects:changedObjects];
    [[self primitiveValueForKey: @"cuisines"] addObject: value];
    [self didChangeValueForKey:@"cuisines" withSetMutation:NSKeyValueUnionSetMutation usingObjects:changedObjects];
    
    [changedObjects release];
}


- (void)removeCuisinesObject:(Cuisine *)value {

    NSSet *changedObjects = [[NSSet alloc] initWithObjects:&value count:1];
    
    [self willChangeValueForKey:@"cuisines" withSetMutation:NSKeyValueMinusSetMutation usingObjects:changedObjects];
    [[self primitiveValueForKey: @"cuisines"] removeObject: value];
    [self didChangeValueForKey:@"cuisines" withSetMutation:NSKeyValueMinusSetMutation usingObjects:changedObjects];
    
    [changedObjects release];
}


- (Chef *)chef {

    id tmpObject;
    
    [self willAccessValueForKey: @"chef"];
    tmpObject = [self primitiveValueForKey: @"chef"];
    [self didAccessValueForKey: @"chef"];
    
    return tmpObject;
}


- (void)setChef:(Chef *)value {

    [self willChangeValueForKey: @"chef"];
    [self setPrimitiveValue: value forKey: @"chef"];
    [self didChangeValueForKey: @"chef"];
}


@end
