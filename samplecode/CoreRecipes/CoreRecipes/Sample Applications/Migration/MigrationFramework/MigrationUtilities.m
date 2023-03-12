/*
 
 File: MigrationUtilities.m
 
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

#import "MigrationUtilities.h"


/** Utility method used by the Generator and Migrator.  Useful stuff
 *  but nothing really specific to the task of migrating data between different 
 *  versions of a data model.
 */
@implementation MigrationUtilities

+ (NSString *)pathForModelNamed:(NSString *)modelName {
    NSString *path = nil;
    NSArray *allBundles = [NSBundle allFrameworks];
    NSBundle *currentBundle = nil;
    int i= 0 , bundleCount = [allBundles count];
    
    for( ; i < bundleCount ; i++ ){
        currentBundle = [allBundles objectAtIndex: i];
        path =  [currentBundle pathForResource: modelName ofType: @"mom"];
        
        if (nil != path) {
            break;
        }
    }
    
    if (nil == path) {
        @throw [NSException exceptionWithName: @"MissingResourceException" reason: [NSString stringWithFormat: @"Can't find model %@!", modelName] userInfo: nil];
    }
    
    return path;
}

+ (NSManagedObjectModel *)retainedOldManagedObjectModel {
    return [[NSManagedObjectModel alloc] initWithContentsOfURL: [NSURL fileURLWithPath: [self pathForModelNamed: @"Migrator_Old"]]];
}

+ (NSManagedObjectModel *)retainedNewManagedObjectModel {
    return [[NSManagedObjectModel alloc] initWithContentsOfURL: [NSURL fileURLWithPath: [self pathForModelNamed: @"Migrator_New"]]];
}

+ (NSManagedObjectContext *)createRetainedCoreDataStackWithStorePath:(NSString *)storePath andModel:(NSManagedObjectModel *)storeModel {
    NSError *error;
    NSPersistentStoreCoordinator *coordinator;
    NSManagedObjectContext *managedObjectContext = nil;
    
    coordinator = [[NSPersistentStoreCoordinator alloc] initWithManagedObjectModel: storeModel];
    id store = nil;
    if (store = [coordinator addPersistentStoreWithType:NSXMLStoreType configuration:nil URL:[NSURL fileURLWithPath: storePath] options:nil error:&error]){
        managedObjectContext = [[NSManagedObjectContext alloc] init];
        [managedObjectContext setPersistentStoreCoordinator: coordinator];
    } 
    
    [coordinator release];
    
    return managedObjectContext;
    
}

+ (void)setMetadata:(NSString *)value forKey:(NSString *)key inStoreWithPath:(NSString *)path inContext:(NSManagedObjectContext *)context {
    NSPersistentStoreCoordinator *coordinator = [context persistentStoreCoordinator];
    id store = [coordinator persistentStoreForURL: [NSURL fileURLWithPath: path]];
    NSMutableDictionary *metadata = [[coordinator metadataForPersistentStore: store] mutableCopy];
    [metadata setValue: value forKey: key];
    [coordinator setMetadata: metadata forPersistentStore: store];    
}

+ (NSString *)getMetadataForKey:(NSString *)key inStoreWithPath:(NSString *)path inContext:(NSManagedObjectContext *)context {
    NSString *version = nil;
    
    if (nil == context) {
        NSError *error = nil;
        NSDictionary *metadata = [NSPersistentStoreCoordinator metadataForPersistentStoreWithURL: [NSURL fileURLWithPath: path] error:&error];
        if (nil == error) {
            version = [metadata valueForKey: key];
        }
    } else {
        NSPersistentStoreCoordinator *coordinator = [context persistentStoreCoordinator];
        id store = [coordinator persistentStoreForURL: [NSURL fileURLWithPath: path]];
        NSMutableDictionary *metadata = [[coordinator metadataForPersistentStore: store] mutableCopy];
        version = [metadata valueForKey: key];
    } 
    return version;
}

+ (void)presentErrorWithDescription:(NSString *)errorString {
    static NSString *domainString;
    
    if (nil == domainString) {
        NSArray *identifierComponents = [[[NSBundle mainBundle] bundleIdentifier] componentsSeparatedByString: @"."];
        domainString = [identifierComponents objectAtIndex: [identifierComponents count] - 1];
    }
    NSError *error = [NSError errorWithDomain: domainString code: 0 userInfo: [NSDictionary dictionaryWithObject: errorString forKey: NSLocalizedDescriptionKey]];
    [[NSApplication sharedApplication] presentError:error];
}

+ (BOOL)createPathIfNecessary:(NSString *)storeDirectory {
    NSFileManager *defaultManager = [NSFileManager defaultManager];
    BOOL success = NO;
    
    
    int i, c;
    NSArray* components = [storeDirectory pathComponents];
    NSString* current = @"";
    c = [components count];  
    for (i = 0; i < c; i++) {
        NSString* index = [components objectAtIndex:i];
        NSString* next = [current stringByAppendingPathComponent:index];
        current = next;
        if (![[NSFileManager defaultManager] fileExistsAtPath: next]) {
            success = [defaultManager createDirectoryAtPath: next attributes: nil];
            if (!success) {
                NSError *error = nil;
                error = [self presentErrorWithDescription: [NSString stringWithFormat: @"Can't create directory at path (%@)", next]];
                return NO;
            }
        } 
    }
    
    return YES;
}

+ (BOOL)validatePathForNewStore:(NSString *) storePath {
    NSFileManager *fileManager = [NSFileManager defaultManager];
    NSString *storeDir = [storePath stringByDeletingLastPathComponent];
    BOOL isDirectory = NO;
    
    if (nil == storePath || [@"" isEqualToString: storePath]) {
        [self presentErrorWithDescription: @"New store path must not be null"];
        return NO;
    }
    
    if ([fileManager fileExistsAtPath: storePath isDirectory: &isDirectory]) {
        // if there is a file at storePath, can we write a store to it?
        if (isDirectory) {
            // not if it's a directory
            [self presentErrorWithDescription: [NSString stringWithFormat: @"Can't save store to path - that location is a directory (%@)", storePath]];
            return NO;
        } else {
            if (![fileManager removeFileAtPath: storePath handler: nil]) {
                // Fail if we can't get rid of the old file
                [self presentErrorWithDescription: [NSString stringWithFormat: @"Can't remove pre-existing file at path (%@)", storePath]];      
                return NO;
            }
        }
    } else if ([fileManager fileExistsAtPath: storeDir isDirectory: &isDirectory] ) {
        // if there isn't a file, is there a parent
        if (isDirectory) {
            // if there is is it a directory?
            if (![fileManager isWritableFileAtPath: storeDir]) {
                // can we write into the parent directory?
                [self presentErrorWithDescription: [NSString stringWithFormat: @"Can't write file to path - directory is not writable (%@)", storeDir]];       
                return NO;
            }
        } else {
            // we can't write to the store then, because the parent isn't a directory
            [self presentErrorWithDescription: [NSString stringWithFormat: @"Can't write file to path - parent is not a directory (%@)", storeDir]]; 
            return NO;
        }
    } else {
        // We don't have a directory, so create one - this does its own error message presentation
        return [self createPathIfNecessary: storeDir];
    }
    return YES;
}

// Very simplistic attribute to descriptive string conversion
+ (NSString *)descriptionForAttributeWithKey:(NSString *)key ofObject:(NSManagedObject *)object
{
    id value = [object valueForKey:key];
    if ([value isKindOfClass:[NSString class]])
        if ([(NSString *)value length] > 20)
            return [NSString stringWithFormat:@"\"%@...\"", [(NSString *)value substringToIndex:17]];
        return [NSString stringWithFormat:@"\"%@\"", value];

    return [NSString stringWithFormat:@"%@", value];
}

// Returns a string description of the managed object, displaying the entity name and  
// objectID detail, and if object is not in the visited objects set the attributes and 
// descriptions for related objects (by recursively invoking
// contentDescriptionForManagedObject:linePrefix:visitedObjects:.)
+ (NSString *)contentDescriptionForManagedObject:(NSManagedObject *)object linePrefix:(NSString *)prefix visitedObjects:(NSMutableSet *)visitedObjects {
    NSMutableString *buffer = [NSMutableString string];
    NSEntityDescription *entity = [object entity];
    NSString *newPrefix = [prefix stringByAppendingString:@"\t"];

    // Describe the entity and object ID, include the 'name' if that's an attribute
    [buffer appendFormat:@"%@(...%@)", [entity name], [[[object objectID] URIRepresentation] path]];
    if ([visitedObjects containsObject:object]) 
        [buffer appendString:@"*"]; // indicate that this managed object has already been visited
    if ([[entity attributesByName] objectForKey:@"name"] != nil) 
        [buffer appendFormat:@" -- \"%@\"", [object valueForKey:@"name"]];
    [buffer appendString:@"\n"];

    // If this object has already been visited, return immediately
    if ([visitedObjects containsObject:object]) {
        return buffer;
    } else {
        [visitedObjects addObject:object];
    }

    // Describe the attributes
    NSString *key = nil;
    NSEnumerator *attributeKeyEnumerator = [[[[entity attributesByName] allKeys] sortedArrayUsingSelector:@selector(compare:)] objectEnumerator];
    while (key = [attributeKeyEnumerator nextObject]) {
        [buffer appendFormat:@"%@%@ = %@\n", newPrefix, key, [self descriptionForAttributeWithKey:key ofObject:object]];
    }

    // Describe the relatiohships and their contents 
    NSEnumerator *relationshipKeyEnumerator = [[[[entity relationshipsByName] allKeys] sortedArrayUsingSelector:@selector(compare:)] objectEnumerator];
    while (key = [relationshipKeyEnumerator nextObject]) {
        NSRelationshipDescription *relationship = [[entity relationshipsByName] objectForKey:key];
        
        if ([relationship isToMany]) {
            [buffer appendFormat:@"%@%@ =>> [\n", newPrefix, key, newPrefix];
            NSString *toManyPrefix = [newPrefix stringByAppendingString:@"\t"];
            NSEnumerator *relatedObjectEnumerator = nil;
            // try to enumerate the to many relationship in order by the name of the destination objects
            if ([[[relationship destinationEntity] attributesByName] objectForKey:@"name"]) {
                relatedObjectEnumerator = [[[[object valueForKey:key] allObjects] sortedArrayUsingDescriptors:[NSArray arrayWithObject:[[[NSSortDescriptor alloc] initWithKey:@"name" ascending:YES] autorelease]]] objectEnumerator];
            } else {
                relatedObjectEnumerator = [[object valueForKey:key] objectEnumerator];
            }
            NSManagedObject *relatedObject = nil;
            while (relatedObject = [relatedObjectEnumerator nextObject]) {
                [buffer appendFormat:@"%@%@", toManyPrefix, [self contentDescriptionForManagedObject:relatedObject linePrefix:toManyPrefix visitedObjects:visitedObjects]];
            }
            [buffer appendFormat:@"%@]\n", newPrefix];
        } else {
            [buffer appendFormat:@"%@\t%@ => %@", prefix, key, [self contentDescriptionForManagedObject:[object valueForKey:key] linePrefix:newPrefix visitedObjects:visitedObjects]];
        }
    }
    return buffer;
}

// Returns a string that describes the contents
// of the store found at the path set in the text field.
+ (NSString *)contentDescriptionForStoreAtPath:(NSString *)storePath withModel:(NSManagedObjectModel *)model {
    if (storePath == nil || [storePath length] == 0)
        return @"No store path provided";
    
    NSManagedObjectContext *context = [[MigrationUtilities createRetainedCoreDataStackWithStorePath:storePath andModel:model] autorelease];
    
    NSFetchRequest *recipesFetchRequest = [[[NSFetchRequest alloc] init] autorelease];
    NSEntityDescription *recipeEntity = [[model entitiesByName] valueForKey: @"Recipe"];
    [recipesFetchRequest setEntity: recipeEntity];
    [recipesFetchRequest setSortDescriptors: [NSArray arrayWithObject:[[[NSSortDescriptor alloc] initWithKey:@"name" ascending:YES] autorelease]]];

    NSError *error = nil;
    NSArray *recipes = [context executeFetchRequest: recipesFetchRequest error: &error];
    
    if (nil != error) 
        return [@"Missing store at path: " stringByAppendingString:storePath];
    
    NSMutableString *buffer = [NSMutableString string];
    NSEnumerator *recipesEnumerator = [recipes objectEnumerator];
    NSManagedObject *recipe = nil;

    NSMutableSet *visitedObjects = [NSMutableSet set];
    // Go through all recipes and add the content description
    while (recipe = [recipesEnumerator nextObject]) {
        // for each iteration: mark every recipe (except the current one) as visited 
        // so they will not be exposed before they are touched in this outside loop 
        [visitedObjects addObjectsFromArray:recipes];
        [visitedObjects removeObject:recipe];
        [buffer appendFormat:@"%@\n", [self contentDescriptionForManagedObject:recipe linePrefix:@"" visitedObjects:visitedObjects]];
    }
    return buffer;
}



@end
