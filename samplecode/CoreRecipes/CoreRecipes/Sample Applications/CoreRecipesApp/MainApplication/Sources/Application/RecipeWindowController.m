/*
 
 File: RecipeWindowController.m 
 
 Abstract: The NSWindowController used for displaying the recipe information
 in the separate editor window.
 
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

#import "RecipeWindowController.h"
#import "AppDelegate.h"

#import "Ingredient.h"
#import "ManualGroup.h"
#import "GroupViewTreeController.h"

@implementation RecipeWindowController


/**
    Initialization of the controller for the specified window.  In this case
    we merely set all of our instance variables to nil.
*/

- (id)initWithWindow:(NSWindow *)window {

    self = [super initWithWindow:window];
    if (self != nil) {

        managedObjectContext = nil;
        recipe = nil;	
        replacementObject = nil;
                
        
    }

    return self;
}


/**
    Deallocation of the window controller.  Standard release of the retained
    objects happens here.
*/

- (void)dealloc {

    // release instance variables
    [recipe release];
    [replacementObject release];
    [managedObjectContext release];

    // call super
    [super dealloc];
}


#pragma mark
#pragma mark Accessors and Mutators
#pragma mark


/**
    Sets the managed object context for the recipe window controller.  Each 
    controller has a unique context to ensure changes are kept separate from all
    other edits.  If the context set here is different than the existing, we
    must also fault the object we are working with into that context.
*/

- (void)setManagedObjectContext:(NSManagedObjectContext *)context {

    // if we have a different context
    if (context != managedObjectContext) {

        // note the change is about to happen
        [self willChangeValueForKey:@"recipe"];

        // release the existing object, if we have it
        NSManagedObjectID *objectID = nil;
        if ( recipe != nil ) {
            objectID = [recipe objectID];
            [recipe release];
            recipe = nil;
        }
        
        // set the context
        [managedObjectContext release];
        managedObjectContext = [context retain];	
        
        // get the object in the new context
        if (objectID != nil) {
            recipe = [[managedObjectContext objectWithID:objectID] retain];
        }

        // note the change did happen
        [self didChangeValueForKey:@"recipe"];
    }
}


/**
    Returns the managed object context for the recipe window controller.  This
    instance should be unique across the application instance.
*/

- (NSManagedObjectContext *)managedObjectContext {
    return managedObjectContext;    
}


/**
    Sets the replacement object for the window controller.
*/

- (void)setReplacementObject:(NSManagedObject *)mainObject {

    if (replacementObject != mainObject) {
        [replacementObject release];
        replacementObject = [mainObject retain];
    }
}


/**
    Sets the recipe (by ID) for the window controller.  This ID is used to 
    uniquely identify the recipe, and allows us to find an existing editor 
    window for a given recipe rather than always creating a new one.
*/

- (void)setRecipeID:(NSManagedObjectID *)objectID {

    // note the value will change
    [self willChangeValueForKey:@"recipe"];

    // release the existing recipe
    if (recipe != nil) {
        [recipe release];
        recipe = nil;	
    }

    // get the new recipe
    NSManagedObjectContext *context = [self managedObjectContext];
    if (context != nil) {
        recipe = [[context objectWithID:objectID] retain];
    }

    // note the value did change
    [self didChangeValueForKey:@"recipe"];
}


/**
    Sets the prepared date of the recipe for the window to the current date.
*/

- (IBAction)preparedToday:(id)sender {
    [recipe setValue:[NSDate date] forKey:@"lastPreparationDate"];	
}


#pragma mark
#pragma mark Ingredient Manipulation
#pragma mark


/**
    Adds a new ingredient to the list, updating the display order accordingly
    so that the ingredient is inserted in the approriate location and the 
    following values are all pushed up one.
*/

- (IBAction) addIngredient:(id)sender {

    // get the current index
    unsigned int selectedIngredientIdx = [ingredientsController selectionIndex];
    
    // make copy of array because it will change when adding new ingredient
    NSArray *previousIngredients = [NSArray arrayWithArray:[ingredientsController arrangedObjects]]; 

    // create new ingredient and set its recipe
    Ingredient *newIngredient = [NSEntityDescription insertNewObjectForEntityForName:@"Ingredient" inManagedObjectContext:managedObjectContext];
    [newIngredient setRecipe:(id)recipe];
    [managedObjectContext assignObject:newIngredient toPersistentStore:[[recipe objectID] persistentStore]];
    
    // set the display order if it is the first item
    if ([previousIngredients count] == 0) {
        [newIngredient setDisplayOrder:1];
    }
    
    // otherwise
    else {    

        // no selection, just insert at the end of the list
        if (selectedIngredientIdx == NSNotFound) {   
            [newIngredient setDisplayOrder: [previousIngredients count] + 1];
        }
        
        // otherwise
        else {

            Ingredient *selectedIngredient = [previousIngredients objectAtIndex:selectedIngredientIdx];

            // insert in display order next to selected ingredient
            [newIngredient setDisplayOrder: [selectedIngredient displayOrder] + 1];
            
            // add one to display order of ingredients that come after new element
            unsigned elementIdx = selectedIngredientIdx + 1;
            while (elementIdx < [previousIngredients count]) {
            
                Ingredient *nextIngredient = [previousIngredients objectAtIndex:elementIdx];
                [nextIngredient setDisplayOrder: [nextIngredient displayOrder] + 1];
                elementIdx++;
            }
            
        }
    }
        
    // update the store and selection
    [ingredientsController rearrangeObjects];
    [ingredientsController setSelectedObjects:[NSArray arrayWithObject:newIngredient]];

}


/**
    Deletes the currently selected ingredient, consequently updating the 
    displayOrder of all of the remaining ingredients to compensate for the
    change.
*/

- (IBAction) deleteIngredient:(id)sender {

    // get the current selection
    unsigned int selectedIngredientIdx = [ingredientsController selectionIndex];
    if (selectedIngredientIdx == NSNotFound) return;

    // get all of the ingredients and the curent index
    NSArray *allIngredients = [ingredientsController arrangedObjects];
    unsigned int elementIdx = selectedIngredientIdx + 1;
    
    // loop through and update the display order for the remaining items
    while (elementIdx < [allIngredients count]) {
        Ingredient *nextIngredient = [allIngredients objectAtIndex:elementIdx];
        [nextIngredient setDisplayOrder: [nextIngredient displayOrder] - 1];
        elementIdx++;
    }

    // remove the item, re-sort, and re-select
    [ingredientsController remove:nil];
    [ingredientsController rearrangeObjects];
    [ingredientsController setSelectionIndex:selectedIngredientIdx];

}


/**
    Moves the currently selected ingredient in the ingredients controller up one
    location in the view.  This implementation takes the selected item and swaps 
    its location with the one before it (by adjust its 'displayOrder' value
    and then rearranging the table view.)  These changes are not saved until
    the recipe itself is saved.
*/

- (IBAction) moveIngredientUp:(id)sender {

    // get all of the ingredients
    NSArray *allIngredients = [NSArray arrayWithArray:[ingredientsController arrangedObjects]];

    // get the currently selected index
    Ingredient *selectedIngredient = [[ingredientsController selectedObjects] objectAtIndex:0];
    unsigned int selectionIndex = [ingredientsController selectionIndex];

    if (selectedIngredient != nil) {   

        // get the matching ingredient
        Ingredient *previousIngredient = [allIngredients objectAtIndex: selectionIndex - 1];
        
        // swap its order with the one after it
        [selectedIngredient setDisplayOrder: [selectedIngredient displayOrder] - 1];
        [previousIngredient setDisplayOrder: [previousIngredient displayOrder] + 1];
    }
    
    // resort the table and reset the selection
    [ingredientsController rearrangeObjects];
    [ingredientsController setSelectedObjects:[NSArray arrayWithObject:selectedIngredient]];
}


/**
    Moves the currently selected ingredient in the ingredients controller down
    one location in the view.  This implementation takes the selected item and
    swaps its location with the one after it (by adjust its 'displayOrder' value
    and then rearranging the table view.)  These changes are not saved until
    the recipe itself is saved.
*/

- (IBAction) moveIngredientDown:(id)sender {

    // get all of the ingredients
    NSArray *allIngredients = [NSArray arrayWithArray:[ingredientsController arrangedObjects]];

    // get the currently selected index
    Ingredient *selectedIngredient = [[ingredientsController selectedObjects] objectAtIndex:0];
    unsigned int selectionIndex = [ingredientsController selectionIndex];
    if (selectedIngredient != nil) {
       
        // get the matching ingredient
        Ingredient *nextIngredient = [allIngredients objectAtIndex: selectionIndex + 1];
        
        // swap its order with the one after it
        [selectedIngredient setDisplayOrder: [selectedIngredient displayOrder] + 1];
        [nextIngredient setDisplayOrder: [nextIngredient displayOrder] - 1];
    }

    // resort the table and reset the selection
    [ingredientsController rearrangeObjects];
    [ingredientsController setSelectedObjects:[NSArray arrayWithObject:selectedIngredient]];
}


#pragma mark
#pragma mark Window Delegate Methods
#pragma mark


/**
    Returns the undo manager for the window.  In this case, the undo manager 
    returned is the manager for the managed object context retained by the 
    window.
*/

- (NSUndoManager *)windowWillReturnUndoManager:(NSWindow *)window {
    return [[self managedObjectContext] undoManager];
}


/**
    Returns a boolean to indiate if the window should close.  We determine the
    appropriate value by telling the context to commit the editing and then
    save:  an failure of either causes us to display an error and return NO.
*/

- (BOOL)windowShouldClose:(id)sender {

    // potential error
    NSError *error = nil;
    BOOL success = NO;
    
    // tell the context to commit and save
    success = ([recipeController commitEditing] && [managedObjectContext save:&error]);

    // the replacementObject is only non-nil when the recipe we're editing was inserted then edited without saving first. Fix up things.
    if ( (replacementObject != nil) && (success) ) {
    
        // We've created a new recipe in our context, now we want to use it in place of the Recipe in the app context
		
        // clean up by deleting the original recipe
        [[replacementObject managedObjectContext] deleteObject:replacementObject];
    }
    
    // If something went wrong while saving, popup an error sheet. Chances are there was a validation error
    if (success == NO) {
	
        // create a wrapper error that offers a "discard" option
        NSMutableDictionary *userInfo = [NSMutableDictionary dictionary];
        [userInfo setObject:error forKey:NSUnderlyingErrorKey];
        [userInfo setObject:[error localizedDescription] forKey:NSLocalizedDescriptionKey];
        [userInfo setObject:[NSArray arrayWithObjects:@"Discard", @"OK", nil] forKey:NSLocalizedRecoveryOptionsErrorKey];
        [userInfo setObject:self forKey:NSRecoveryAttempterErrorKey];

        //[userInfo setObject:@"OK" forKey:NSLocalizedRecoverySuggestionErrorKey];
        NSError *recoverableError = [NSError errorWithDomain:@"RecipeEditor" code:1 userInfo:userInfo];
        [self presentError:recoverableError modalForWindow:[self window] delegate:nil didPresentSelector:nil contextInfo:nil];
    }

    // return the value
    return success;
}


/**
    Method to attempt to recover from errors generated while attempting to
    save.  The errors are generated above, and the recovery index is from the
    user selection in the error sheet.
*/

- (void)attemptRecoveryFromError:(NSError *)error optionIndex:(unsigned int)recoveryOptionIndex delegate:(id)delegate didRecoverSelector:(SEL)didRecoverSelector contextInfo:(void *)contextInfo {

    //if (([[error domain] isEqualToString:@"RecipeEditor"]) && ([error code] == 1)) {
    // This if statement isn't need in our case, but is here for when you consider the case of multiple error sources	
    //}
    
    BOOL success = NO;

    // the user chose to discard the changes
    if ( recoveryOptionIndex == 0 ) {
    
        // simply close the window, that will result is self being dealloc'd and everything cleaned up
        [[self window] close];
        success = YES;
    }

    // part of the recovery attempter contract
    if (delegate != nil) {
        [delegate performSelector:didRecoverSelector withObject:[NSNumber numberWithBool:success] withObject:(id)contextInfo];
    }
}


/**
    Called when the window for the view loads.  Here we want to ensure the
    display of the ingredients is up to date (meaning the table view order
    matches that of the display order in the actual items.)
*/

- (void)windowDidLoad {

    // call super
    [super windowDidLoad];
    
    // force sorting of ingredients in order
    [ingredientsController setSortDescriptors:[NSArray arrayWithObject:[[NSSortDescriptor alloc] initWithKey:@"displayOrder" ascending:YES]]];
    [ingredientsController rearrangeObjects];
    
    NSArray *stores = [NSArray arrayWithObject:[[recipe objectID] persistentStore]];
    [(MainStoreArrayController *)allCuisinesController setAffectedStores:stores];
    [(MainStoreArrayController *)allChefsController setAffectedStores:stores];
    [(MainStoreArrayController *)allMeasuresController setAffectedStores:stores];    
}


/**
    Notification hander called when the window is about to close.  Here we
    message the application delegate so it can update the recipe editor cache
    information (since this recipe will no longer have an associated editor.)
*/

- (void)windowWillClose:(NSNotification *)notification {
    [[NSApp delegate] closeRecipeEditor:self objectID:[recipe objectID]];
}


/** 
    Delete a cuisine from the Recipe using the selection of the passed in 
    controller. Invoked from target/argument bindings in Cuisines tab of the 
    RecipeEditorWindow nib. 
*/

- (void)deleteCuisine:(NSArrayController *)controller {
    
    // NSArray *cuisines = [[[controller arrangedObjects] copy] autorelease];
    
    // note that we copy the arrangedObjects, since if we delete from the arrayController, simply 
    // NSArray *cuisines = [controller arrangedObjects];
    // would yield an array whose size/contents would change in step with the arrayController.
    
    
    NSMutableSet *objectCuisines = [recipe mutableSetValueForKey:@"cuisines"];
    
    // This won't work well since it happens delayed
    // [controller remove:<#(id)sender#>] 
    // immediately after, ([cuisines count] == [[controller arrangedObjects] count])
    
    // This happens immediately, but we'll find
    //[controller removeObjectAtArrangedObjectIndex:[controller selectionIndex]];
    // immediately after, ([cuisines count] > [[controller arrangedObjects] count]) - good
    
    // but --
    // ([objectCuisines count] != [[controller arrangedObjects] count]) {

    // so the correct thing to do is
    [objectCuisines minusSet:[NSSet setWithArray:[controller selectedObjects]]];
    
}

/** 
    Takes a set or array of objects, and tries to find the same object in context. 
    This is used elsewhere to ensure that objects on both sides of a relationship 
    come from the same context.
*/

- (NSArray *)matchingObjects:(id)objects inContext:(NSManagedObjectContext *)context {
    NSMutableArray *results = [NSMutableArray array];
    
    NSEnumerator *enumerator = [objects objectEnumerator];
    id object;
    
    while ((object = [enumerator nextObject]) != nil) {
	
        // never want to look for temporary object
        NSManagedObjectID *objectID = [object objectID];
        if ([objectID isTemporaryID] == NO) {
            object = [context objectWithID:objectID];
            if (object != nil) {
                [results addObject:object]; 
            }
        }
    }
    
    return results;
}


/**
    Workaround to deal with the many-to-many relationship between the Cuisine
    and Recipe entities.  Here we get the current list of all Cuisines for the
    Recipe, remove them, and then add in the new objects.
*/

- (void)setMakeHack:(NSArray *)objects {

    NSMutableSet *cuisines = [recipe mutableSetValueForKey:@"cuisines"];
    [cuisines removeAllObjects];
    
    NSArray *matchedObjects = [self matchingObjects:objects inContext:[self managedObjectContext]];
    [cuisines addObjectsFromArray:matchedObjects];
}


/**
    Workaround to deal with the many-to-many relationship between the Cuisine
    and Recipe entities.  Here we return the array of Cuisines for the
    specified recipe in the appropriate context.
*/

- (NSArray *)makeHack {

    NSManagedObjectContext *prefsContext = [(AppDelegate *)[NSApp delegate] preferencesContext];
    NSSet *cuisines = [recipe valueForKey:@"cuisines"];
    return [self matchingObjects:cuisines inContext:prefsContext];
}


/**
    Returns an array of all of the Measures for the recipe to use.  Here we 
    simply pull the list of all measures from the arranged objects of the
    measures controller, but fault them into application context (since the
    other controller uses it's own context.)
*/

- (NSArray *)allLocalMeasures {

    NSArray *measures = [allMeasuresController arrangedObjects];
    NSMutableArray *things = [NSMutableArray array];
    NSManagedObjectContext *context = [self managedObjectContext];
    int index, count;;
    
    for (index=0, count=[measures count]; index < count; index++) {
        id object = [context objectWithID:[[measures objectAtIndex:index] objectID]];
        [things addObject:object];
    }

    NSSortDescriptor *desc = [[NSSortDescriptor alloc] initWithKey:@"displayOrder" ascending:YES];
    [things sortUsingDescriptors:[NSArray arrayWithObject:desc]];
    [desc release];
    
    return things;
}


/**
    Mutator the chef for the specified Recipe.  Here we take the currently 
    selected chef and put it into the application context (to ensure it can be
    related to the Recipe), and then set the value.
*/

- (void)setSelectedChef:(NSManagedObject *)chef {
    if ([chef isInserted] == NO) {
        id localChef = [[self managedObjectContext] objectWithID:[chef objectID]];
        [recipe setValue:localChef forKey:@"chef"];    
    }    
}


/**
    Returns the selected Chef for the Recipe.  Here we return the Chef from 
    the preferences context (since that is where they have all been listed, 
    in the controller there.
*/

- (id)selectedChef {

    NSManagedObject *chef = [recipe valueForKey:@"chef"];
    NSManagedObjectContext *prefsContext = [(AppDelegate *)[NSApp delegate] preferencesContext];
    
    if ((chef != nil) && (!NSIsControllerMarker(chef))) {
        id fetchedChef = [prefsContext objectWithID:[chef objectID]];
        return fetchedChef;
    }

    return nil;
}

@end
