/*
 
 File: DataGenerator.m
 
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

#import "DataGenerator.h"
#import "MigrationUtilities.h"

/** Generates sample data using the older version of the data model and saves 
 *  it to the persistent store using the file path spefified in initWithPath:.  Also 
 *  sets metadata to the store with the key "Migration Example Version" and value 
 *  "Version 1".
 */
@implementation DataGenerator

- (NSManagedObjectModel *)managedObjectModel {
    if (managedObjectModel) {
        return managedObjectModel;
    }
	
    managedObjectModel = [MigrationUtilities retainedOldManagedObjectModel];
    
    return managedObjectModel;
}

- (NSManagedObjectContext *) createCoreDataStackWithStorePath:(NSString *)storePath {
    if (managedObjectContext) {
        return managedObjectContext;
    }

    managedObjectContext = [MigrationUtilities createRetainedCoreDataStackWithStorePath: storePath andModel: [self managedObjectModel]];
    
    if (nil != managedObjectContext) {
        [MigrationUtilities setMetadata: @"Version 1" forKey: @"Migration Example Version" inStoreWithPath: storePath inContext: managedObjectContext];        
    }
    
    return managedObjectContext;
}

- (id)initWithPath:(NSString *)storePath {
    self = [super init];
    if (nil != self) {
        [self createCoreDataStackWithStorePath: storePath];
    }
    return self;
}

- (void)dealloc {
    [managedObjectModel release];
    managedObjectModel = nil;
    [managedObjectContext release];
    managedObjectContext = nil;
    
    [super dealloc];
}

- (BOOL)generateData:(NSError **)error {
    BOOL success = [self generateAndSaveChefs: error];
    if (!success) {
        return NO;
    } else {
        success = [self generateAndSaveRecipesAndIngredients: error];
        if (!success) {
            return NO;
        }                
    }
    return YES;
}

// Generate and save data for five chefs
- (BOOL)generateAndSaveChefs:(NSError **)err {
    int i = 0;
    
    for ( ; i < 5 ; i++ ) {
        NSManagedObject *chef = [NSEntityDescription insertNewObjectForEntityForName: @"Chef" inManagedObjectContext: managedObjectContext];
        [chef setValue: [NSString stringWithFormat: @"Last%i, First%i", i + 1, i + 1] forKey: @"name"];
        [chef setValue: [NSString stringWithFormat: @"Studied at Culinary Institute of America"] forKey: @"training"];
    }
    
    
    [managedObjectContext save: err];
    
    if (nil != *err) {
        return NO;
    }
    return YES;
}

// Generate and save one recipe for each chef
- (BOOL)generateAndSaveRecipesAndIngredients:(NSError **)err {
    NSFetchRequest *chefFetchRequest = [[[NSFetchRequest alloc] init] autorelease];
    [chefFetchRequest setEntity: [[managedObjectModel entitiesByName] valueForKey: @"Chef"]];
    [chefFetchRequest setSortDescriptors: [NSArray arrayWithObject: [[[NSSortDescriptor alloc] initWithKey: @"name" ascending: YES] autorelease]]];
    NSArray *chefs = [managedObjectContext executeFetchRequest: chefFetchRequest error: err];
    
    if (nil == *err) {
        int chefCount = [chefs count];
        int i = 0;
        
        for ( ; i < chefCount ; i++ ) {
            NSManagedObject *chef = [chefs objectAtIndex: i];
            NSManagedObject *recipe = [NSEntityDescription insertNewObjectForEntityForName: @"Recipe" inManagedObjectContext: managedObjectContext];
            int remainder = (i % 3);
            [recipe setValue: [NSString stringWithFormat: @"Recipe %i", i + 1] forKey: @"name"];
            [recipe setValue: [NSString stringWithFormat: @"Cuisine %c", (0 == remainder) ? 'A' : ((1 == remainder) ? 'B' : 'C')] forKey: @"cuisine"];
            [chef setValue: recipe forKey: @"recipes"];
            
            int j = 0;
            for ( ; j <= i ; j++) {
                NSManagedObject *ingredient = [NSEntityDescription insertNewObjectForEntityForName: @"Ingredient" inManagedObjectContext: managedObjectContext];
                [ingredient setValue: [NSString stringWithFormat: @"Ingredient %i", j + 1] forKey: @"name"];
                [ingredient setValue: [NSString stringWithFormat: @"%i %c", j + 1, (0 == (j % 2)) ? 'C' : 'T'] forKey: @"amount"];
                [[recipe mutableSetValueForKey: @"ingredients"] addObject: ingredient];
            }
        }
        [managedObjectContext save: err];
        if (nil != *err) {
            return NO;
        }
    }
    return YES;
}

@end
