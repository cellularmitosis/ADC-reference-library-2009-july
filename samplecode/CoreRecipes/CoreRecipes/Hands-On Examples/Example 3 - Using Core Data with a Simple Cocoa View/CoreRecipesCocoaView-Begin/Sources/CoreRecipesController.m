
#import "CoreRecipesController.h"


@implementation CoreRecipesController

// Build the managed object model from the compiled data models in our bundle
- (NSManagedObjectModel *)managedObjectModel {
    if (managedObjectModel) return managedObjectModel;
	
	NSBundle *mainBundle = [NSBundle mainBundle];	
    
    managedObjectModel = [[NSManagedObjectModel mergedModelFromBundles:[NSArray arrayWithObject:mainBundle]] retain];
    
    return managedObjectModel;
}

// Return our managed object context, first setting up the persistence stack if needed
- (NSManagedObjectContext *)managedObjectContext {
    NSError *error;
    NSURL *url;
    NSPersistentStoreCoordinator *coordinator;
    
    if (managedObjectContext) {
        return managedObjectContext;
    }
    
	// This example uses a prepared store file which is in the app's bundle and is opened read-only
    NSBundle *mainBundle = [NSBundle mainBundle];
	NSString *storePath = [mainBundle pathForResource:@"CoreRecipesCocoaView" ofType:@"xml"];
	NSAssert(nil != storePath, @"No store file found in application bundle.");
	url = [NSURL fileURLWithPath:storePath];
	
    coordinator = [[NSPersistentStoreCoordinator alloc] initWithManagedObjectModel: [self managedObjectModel]];
	NSDictionary *storeOptionsDict = [NSDictionary dictionaryWithObject:[NSNumber numberWithBool:YES] forKey:NSReadOnlyPersistentStoreOption];
    if ([coordinator addPersistentStoreWithType:NSXMLStoreType configuration:nil URL:url options:storeOptionsDict error:&error]){
        managedObjectContext = [[NSManagedObjectContext alloc] init];
        [managedObjectContext setPersistentStoreCoordinator: coordinator];
    } else {
        [[NSApplication sharedApplication] presentError:error];
    }    
    [coordinator release];
    
    return managedObjectContext;
}

// Populate our array controller's array of recipes with all the recipes in the store
- (void)populateRecipes
{
	[recipes release];
/*
	NSManagedObjectContext *context = [self managedObjectContext];
	NSFetchRequest *recipeFetch = [[[NSFetchRequest alloc] init] autorelease];
	NSEntityDescription *recipeEntity = [NSEntityDescription entityForName:@"Recipe" inManagedObjectContext:context];
	[recipeFetch setEntity:recipeEntity];
*/
/*
	NSError *fetchError;
	recipes = [NSMutableArray arrayWithArray:[context executeFetchRequest:recipeFetch error:&fetchError]];
	NSAssert(nil != recipes, ([NSString stringWithFormat:@"Recipe fetch returned error:%@", fetchError]));
*/
	[recipes retain];
}

// TableView datasource methods

- (int)numberOfRowsInTableView:(NSTableView *)aTableView
{
/*
	if (nil == recipes) {
		[self populateRecipes];
	}
	
	return [recipes count];
*/
}

- (id)tableView:(NSTableView *)aTableView objectValueForTableColumn:(NSTableColumn *)aTableColumn row:(int)rowIndex
{
/*
	if (nil == recipes) {
		[self populateRecipes];
	}

	id columnIdentifier = [aTableColumn identifier];

	NSAssert(rowIndex >= 0 && rowIndex < [recipes count], @"Invalid row index for tableview datasource");
	return [[recipes objectAtIndex:rowIndex] valueForKeyPath:columnIdentifier];
*/
}

// Delete a recipe from our context
- (IBAction)deleteRecipe:(id)sender
{
/*
	int selectedRow = [recipeTable selectedRow];
	if (-1 == selectedRow) { // No row selected
		NSBeep();
	} else {
		NSAssert(selectedRow >= 0 && selectedRow < [recipes count], @"Row selected in table view does not exist in recipes");
		NSManagedObject *recipeToDelete = [recipes objectAtIndex:selectedRow];
		[[self managedObjectContext] deleteObject:recipeToDelete];
		[recipes removeObjectAtIndex:selectedRow];
		
		[recipeTable reloadData];
	}
*/
}

@end

/*

File: CoreRecipesController.m

Abstract: Main controller class for the application

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
