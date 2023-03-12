#import <CoreData/CoreData.h>


// Prototypes for functions used below.

BOOL InitializeCoreDataStack(NSError **error);
NSArray *FetchAllChefs(NSError **error);
NSManagedObject *InsertChefAndSave(NSError **error);
void DisplayChefs(NSArray *chefs);


// Global variables representing the Core Data persistence stack used by this tool.

NSManagedObjectModel *model = nil;
NSPersistentStoreCoordinator *coordinator = nil;
NSManagedObjectContext *context = nil;
id persistentStore = nil;


// Main entry point.  This sample shows how to initialize a Core Data persistence stack, fetch some information from a persistent store using that stack, and create a managed object and save it to a persistent store.

int main (int argc, const char * argv[]) {
    int result = EXIT_SUCCESS;
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
    
    // Initialize the Core Data persistence stack.
    
    NSError *errorInitializingStack = nil;
    
    if (InitializeCoreDataStack(&errorInitializingStack)) {
        
        // Initializing the stack succeeded, so there is a persistent store available.  Verify that the persistent store contains at least one Chef.
        
        NSError *errorFetchingChefs = nil;
        
        NSArray *chefs = FetchAllChefs(&errorFetchingChefs);
        
        if (chefs != nil) {
            
            // Fetching all Chefs succeeded.  If there are no Chefs, create one.  Otherwise, display all found Chefs.
            
            if ([chefs count] == 0) {
                NSError *errorSavingChef = nil;
                
                NSManagedObject *chef = InsertChefAndSave(&errorSavingChef);
                
                if (chef != nil) {
                    NSLog(@"No initial Chef was found, inserted and saved an initial Chef: %@", chef);
                } else {
                    NSLog(@"An error occurred while inserting and saving an initial Chef: %@",
                          [errorSavingChef localizedDescription]);
                }
            } else {
                DisplayChefs(chefs);
            }
        } else {
            
            // Fetching all Chefs did not succeed, report the error.
            
            NSLog(@"An error occurred while fetching all Chefs: %@",
                  [errorFetchingChefs localizedDescription]);
            
            result = EXIT_FAILURE;
        }
    } else {
        
        // Initializing the stack did not succeed, report the error.
        
        NSLog(@"An error occured while initializing a Core Data persistence stack: %@",
              [errorInitializingStack localizedDescription]);
        result = EXIT_FAILURE;
    }
    
    // Clean up
    
    [model release];
    [coordinator release];
    [context release];
    
    [pool release];
    
    // Exit with appropriate status code
    
    return result;
}


// Initialize the Core Data persistence stack used by this tool.  This consists of a model, a persistent store coordinator, a managed object context, and a persistent store, all global.  Return YES on success, NO and an autoreleased NSError in *error on failure.

BOOL InitializeCoreDataStack(NSError **error) {
    
    // Create a merged managed object model based on the models in the main bundle.
    // For a Foundation tool, the main bundle is the directory that contains it.
    
    NSArray *bundlesToSearch = [NSArray arrayWithObject:[NSBundle mainBundle]];
    model = [NSManagedObjectModel mergedModelFromBundles:bundlesToSearch];
    [model retain];
    
    // Create a persistent store coordinator to act as a facade on all of the persistent stores.
    
    coordinator = [[NSPersistentStoreCoordinator alloc] initWithManagedObjectModel:model];
    
    // Create a managed object context to manage the object graph, and attach it to the coordinator created above.
    
    context = [[NSManagedObjectContext alloc] init];
    [context setPersistentStoreCoordinator:coordinator];
    
    // Set up a persistent store to use.
    
    NSURL *storeURL = [NSURL fileURLWithPath:@"/tmp/CoreRecipesTool.xml"];
    
    persistentStore = [coordinator addPersistentStoreWithType:NSXMLStoreType configuration:nil URL:storeURL options:nil error:error];
    
    return (persistentStore != nil);
}


// Fetch all Chef instances available via the tool's persistence stack.  Return the fetched Chefs if the fetch was successful, even if there were no Chefs fetched.  Return nil and an autoreleased NSError in *error if the fetch was unsuccessful.

NSArray *FetchAllChefs(NSError **error) {
    NSArray *chefs = nil;
    
    // Create a request to fetch all Chefs.
    
    NSEntityDescription *chefEntity = [NSEntityDescription entityForName:@"Chef" inManagedObjectContext:context];
    
    NSFetchRequest *allChefsRequest = [[NSFetchRequest alloc] init];
    [allChefsRequest setEntity:chefEntity];
    
    // Ask the context for everything matching the request.
    // If an error occurs, the context will return nil and an error in *error.
    
    chefs = [context executeFetchRequest:allChefsRequest error:error];
    
    // Clean up and return.
    
    [allChefsRequest release];
    
    return chefs;
}


// No Chef existed, so create one with some default attributes and save the object graph to the persistent store.  Return the new Chef if saving was successful, otherwise return nil and an autoreleased NSError in *error.

NSManagedObject *InsertChefAndSave(NSError **error) {
    NSManagedObject *chef = nil;

    chef = [NSEntityDescription insertNewObjectForEntityForName:@"Chef" inManagedObjectContext:context];
    
    [chef setValue:@"John" forKey:@"firstName"];
    [chef setValue:@"Doe" forKey:@"lastName"];
    
    if ([context save:error] == NO) {
        [context deleteObject:chef];
        
        chef = nil;
    }
    
    return chef;
}


// Print information on the found chefs.

void DisplayChefs(NSArray *chefs) {
    NSLog(@"Chefs found: %u", [chefs count]);
    
    NSEnumerator *chefsEnumerator = [chefs objectEnumerator];
    NSManagedObject *chef;
    
    while ((chef = [chefsEnumerator nextObject]) != nil) {
        NSLog(@"Found Chef: %@ %@", [chef valueForKey:@"firstName"], [chef valueForKey:@"lastName"]);
    }
}

/*

File: CoreRecipesTool.m

Abstract: A command-line tool demonstrating how to set up a Core Data
persistence stack, fetch instances of entities, insert a new instance
of an entity, and save an object graph.

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
