/*

File: GetRecipesSummaryAction.m

Abstract: The class to provide the CoreRecipes Automator action.

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


#import "GetRecipesSummaryAction.h"

#import <OSAKit/OSAKit.h>
#import "Recipe.h"
#import "RecipeUtilities.h"


#pragma mark -
#pragma mark Static Definitions
#pragma mark

// Name of the application support folder
static NSString * LIBRARY_FOLDER_NAME = @"CoreRecipes";

// Name of the store file
static NSString * LIBRARY_FILE_NAME = @"CoreRecipesLibrary.crx";

// Indexes of the sarch types, these correspond to the popup menu item indexes
enum SearchTypes {
    RecipeSearch = 0,
    GroupSearch,
    CuisineSearch
};


/**
    GetRecipesSummaryAction is a custom AMBundleAction that provides 
    an Automator action that searches for recipes based on user configured  
    search parameters and generates text summaries for matching recipes.
*/


@implementation GetRecipesSummaryAction

#pragma mark -
#pragma mark Automator Action Hook
#pragma mark

/**
    Generates and returns an array of recipe summaries as strings, matching 
    recipes based on the user-configured parameters searchType and searchText.  
    Any input passed into this method is preserved and returned with the recipe 
    summary text output.
*/

- (id)runWithInput:(id)input fromAction:(AMAction *)anAction error:(NSDictionary **)errorInfo {

    NSMutableArray *output = [NSMutableArray array];
    NSString *actionMessage = nil;
    NSArray *recipes = nil;
    NSArray *summaries = nil;
    
    // automator actions should pass through any input they do not consume/modify
	if (input != nil) {
        if ([input isKindOfClass:[NSArray class]]) {
            [output addObjectsFromArray: input];
        } else {
            [output addObject: input];
        }
    }
	
    // create the core data stack, find the recipes and generate the summaries
    // within an exception handler so we can give error feedback to Automator
    @try {
        if (managedObjectContext == nil) {
            actionMessage = @"accessing user recipe library";
            [self initCoreDataStack];
        }
        actionMessage = @"finding recipes";
        recipes = [self recipesMatchingSearchParameters];
        actionMessage = @"generating recipe summaries";
        summaries = [self summariesFromRecipes:recipes];
    }
    @catch (NSException *exception) {
        NSMutableDictionary *errorDict = [NSMutableDictionary dictionary];
        [errorDict setObject:[NSString stringWithFormat:@"Error %@: %@", actionMessage, [exception reason]] forKey:OSAScriptErrorMessage];
        [errorDict setObject:[NSNumber numberWithInt:errOSAGeneralError] forKey:OSAScriptErrorNumber];
        *errorInfo = errorDict;
        return input;
    }

    // invalidate and clear the core data stack 
    [managedObjectContext release];
    managedObjectContext = nil;
    [persistentStoreCoordinator release];
    persistentStoreCoordinator = nil;
    
    [output addObjectsFromArray:summaries];
	
    return output;
}

#pragma mark -
#pragma mark Recipe Searching
#pragma mark

/**
    Examines the searchType and searchText parameters (configured via bindings in
    the user interface), returns the recipes matched by the searchType-appropriate 
    recipe search method.
*/

- (NSArray *)recipesMatchingSearchParameters {

    NSString *searchText = [[self parameters] valueForKey:@"searchText"];
    NSNumber *searchType = [[self parameters] valueForKey:@"searchType"];
    switch ([searchType intValue]) {
        case RecipeSearch:
            return [self recipesWhoseRecipeNameContainsString:searchText];
        case GroupSearch:
            return [self recipesWhoseGroupNameContainsString:searchText];
        case CuisineSearch:
            return [self recipesWhoseCuisineNameContainsString:searchText];
    }
    return nil;
}

/**
    Searches for Recipes with a name containing the searchText and 
    returns a name-ordered array of matching recipes.
*/

- (NSArray *)recipesWhoseRecipeNameContainsString:(NSString *)searchText {

    NSFetchRequest *request = [[NSFetchRequest alloc] init];
    NSSortDescriptor *sortDescriptor = [[[NSSortDescriptor alloc] initWithKey:@"name" ascending:YES] autorelease];
    NSPredicate *searchPredicate = [NSPredicate predicateWithFormat:@"name contains[cd] %@", searchText];
    NSEntityDescription *entity = [NSEntityDescription entityForName:@"Recipe" inManagedObjectContext:managedObjectContext];
    [request setEntity:entity];
    [request setPredicate:searchPredicate];
    [request setSortDescriptors:[NSArray arrayWithObject:sortDescriptor]];

    NSError *error = nil;
    NSArray *recipes = [managedObjectContext executeFetchRequest:request error:&error];
    if (error != nil)
        [NSException raise:@"RecipeSearchError" format:@"Recipe search failed - %@", [error localizedDescription]];
    return recipes;
}

/**
    Searches for Cuisines with a name containing the searchText and 
    returns a name-ordered array of distinct recipes containined by them.  
*/

- (NSArray *)recipesWhoseCuisineNameContainsString:(NSString *)searchText {

    NSFetchRequest *request = [[NSFetchRequest alloc] init];
    NSSortDescriptor *sortDescriptor = [[[NSSortDescriptor alloc] initWithKey:@"name" ascending:YES] autorelease];
    NSPredicate *searchPredicate = [NSPredicate predicateWithFormat:@"name contains[cd] %@", searchText];
    NSEntityDescription *entity = [NSEntityDescription entityForName:@"Cuisine" inManagedObjectContext:managedObjectContext];
    [request setEntity:entity];
    [request setPredicate:searchPredicate];
    
    NSError *error = nil;
    NSArray *cuisines = [managedObjectContext executeFetchRequest:request error:&error];
    if (error != nil)
        [NSException raise:@"RecipeSearchError" format:@"Recipe search failed - %@", [error localizedDescription]];
    NSArray *recipes = [cuisines valueForKeyPath:@"@distinctUnionOfSets.recipes"];
    return [recipes sortedArrayUsingDescriptors:[NSArray arrayWithObject:sortDescriptor]];
}


/**
    Searches for Groups (SmartGroups) with a name containing the searchText and 
    returns a name-ordered array of distinct recipes containined by them.  Provides
    special case behavior for the searchText "Library" which returns the entire 
    set of recipes ("Library" is the name a SmartGroup that contains every recipe but 
    only exists in memory)
*/

- (NSArray *)recipesWhoseGroupNameContainsString:(NSString *)searchText {

    NSFetchRequest *request = [[NSFetchRequest alloc] init];
    NSSortDescriptor *sortDescriptor = [[[NSSortDescriptor alloc] initWithKey:@"name" ascending:YES] autorelease];
    NSPredicate *searchPredicate = [NSPredicate predicateWithFormat:@"name contains[cd] %@", searchText];
    // special case the "Library" group - since it only exists in-memory
    if ([searchText isEqualToString:@"Library"]) {
        NSEntityDescription *entity = [NSEntityDescription entityForName:@"Recipe" inManagedObjectContext:managedObjectContext];
        [request setEntity:entity];
        [request setSortDescriptors:[NSArray arrayWithObject:sortDescriptor]];
        
        NSError *error = nil;
        NSArray *recipes = [managedObjectContext executeFetchRequest:request error:&error];
        if (error != nil)
            [NSException raise:@"RecipeSearchError" format:@"Recipe search failed - %@", [error localizedDescription]];
        return recipes;
    }        
    NSEntityDescription *entity = [NSEntityDescription entityForName:@"Group" inManagedObjectContext:managedObjectContext];
    [request setEntity:entity];
    [request setPredicate:searchPredicate];
    
    NSError *error = nil;
    NSArray *groups = [managedObjectContext executeFetchRequest:request error:&error];
    if (error != nil)
        [NSException raise:@"RecipeSearchError" format:@"Recipe search failed - %@", [error localizedDescription]];
    NSArray *recipes = [groups valueForKeyPath:@"@distinctUnionOfSets.recipes"];
    return [recipes sortedArrayUsingDescriptors:[NSArray arrayWithObject:sortDescriptor]];
}

#pragma mark -
#pragma mark Text Summary Generation
#pragma mark

/**
    Returns an array of recipe summaries as NSStrings by iterating through the 
    recipes and using RecipeUtilities -RTFDataForRecipe:includesHyperlink: to
    generate the RTF text which is then converted to a simple string.
*/

- (NSArray *)summariesFromRecipes:(NSArray *)recipes {

    NSEnumerator *recipeEnumerator = [recipes objectEnumerator];
    NSMutableArray *summaries = [NSMutableArray array];
    Recipe *recipe = nil;
    while (recipe = [recipeEnumerator nextObject]) {
        NSError *error = nil;
        NSDictionary *attributes = nil;
        NSData *summaryData = [RecipeUtilities RTFDataForRecipe:recipe includesHyperlink:NO];
        NSString *summary = [[[NSAttributedString alloc] initWithData:summaryData options:nil documentAttributes:&attributes error:&error] string];
        if (error != nil)
            [NSException raise:@"RecipeSummaryError" format:@"Recipe summary creation failed - %@", [error localizedDescription]];
        [summaries addObject:summary];
    }
    return summaries;
}

#pragma mark -
#pragma mark CoreData Stack Initialization
#pragma mark

/**
    Initializes the default store URL and adds the persistent store to the 
    coordinator for the application (which is heretofore known as the "default
    store".)  This implementation looks for the store file in the search path 
    location (the application support folder).
*/

- (void)initDefaultStore {
    
    // open up the default store
    NSArray *searchpaths = NSSearchPathForDirectoriesInDomains(NSApplicationSupportDirectory, NSUserDomainMask, YES);
    if ([searchpaths count] > 0) {
        
        // create the path to the library folder
        NSString *path = [[searchpaths objectAtIndex:0] stringByAppendingPathComponent: LIBRARY_FOLDER_NAME];

        // set up the path for the "CoreRecipes.crx" file
        path = [[path stringByAppendingPathComponent: LIBRARY_FILE_NAME] stringByResolvingSymlinksInPath];

        // if the store doesn't exist, raise an exception
        if (![[NSFileManager defaultManager] fileExistsAtPath:path]) 
            [NSException raise:@"MissingResourceException" format:@"Missing recipe library"];
        
        NSURL *defaultStoreURL = [[[NSURL alloc] initFileURLWithPath: path] autorelease];
        
        // load the store
        [persistentStoreCoordinator addPersistentStoreWithType:NSXMLStoreType configuration:nil URL:defaultStoreURL options:nil error:nil];
    }
}


/**
    Initializes the default Core Data stack variables for the application: the
    model we are going to use (from the application bundle), the persistent 
    store coordinator, and the managed object context.
 */

- (void)initCoreDataStack {
    
    // get the bundles
    NSArray *bundles = [NSArray arrayWithObject:[NSBundle bundleForClass:[self class]]];
    
    // create the model and coordinator
    NSManagedObjectModel *model = [NSManagedObjectModel mergedModelFromBundles:bundles];
    persistentStoreCoordinator = [[NSPersistentStoreCoordinator alloc] initWithManagedObjectModel:model];
    
    // craete the context and set its coordinator and merge policy
    managedObjectContext = [[NSManagedObjectContext alloc] init];
    [managedObjectContext setPersistentStoreCoordinator:persistentStoreCoordinator];
    
    // initialize the default store
    [self initDefaultStore];
    
}


@end
