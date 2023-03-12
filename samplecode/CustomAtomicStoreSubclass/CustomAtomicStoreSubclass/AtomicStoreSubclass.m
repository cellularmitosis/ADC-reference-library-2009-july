/*
 
 File: AtomicStoreSubclass.m
 
 Abstract: Sample implementation of a custom CoreData store using
 the NSAtomicStore API.
 
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
 
 Copyright © 2007 Apple, Inc., All Rights Reserved
 
 */

#import <CoreData/NSPersistentStore.h>
#import <CoreData/NSAtomicStore.h>

#import "AtomicStoreSubclass.h"
#import "AtomicStoreCacheNodeSubclass.h"


NSString *MyAtomicStoreSubclassDomain = @"MyAtomicStoreSubclass";


@implementation AtomicStoreSubclass

#pragma - implementation details

/*
 * createStubAndGenerateMetadata
 
 Called from initWithPersistentStoreCoordinator:configurationName:URL:options:
 Creates a stub file to reserve the store's future location on disk;
 return NO (don't initialize) if we can't create the file
 */
- (BOOL)createStubAndGenerateMetadata {
	NSFileManager *fileManager = [NSFileManager defaultManager];
	NSURL *fileURL = [self URL];
	NSString *filePath = [fileURL path];
	
	BOOL success = [fileManager createFileAtPath:filePath contents:nil attributes:nil];
	if (!success) {
		return NO;
	}
	
	[self setMetadata: [NSDictionary dictionaryWithObject: [NSNumber numberWithInteger: 0] forKey: @"lastReferenceObject"]];
	removeStub = YES;
	return YES;
}

/*
 * readFile
 
 Called from initWithPersistentStoreCoordinator:configurationName:URL:options:
 Reads the file and returns NO (don't complete initialization) if the store isn't approximately
 what we expect it to be (an archive containing the high level format we expect)
 */
 - (BOOL)readFile {
	NSData *storeContents = nil;

	@try {
		storeContents = [NSKeyedUnarchiver unarchiveObjectWithData: [NSData dataWithContentsOfURL:[self URL]]];
	} @catch (NSException *ex) {
		// bad archive
		return NO;
	}
	
	if (nil == storeContents) {
		return NO;
	} else {
		if (![storeContents isKindOfClass: [NSDictionary class]])  {
			// didn't get what we were expecting
			return NO;
		}
		NSDictionary *storeMetadata = [storeContents valueForKey:@"Metadata"];
		NSArray *storedObjects = [storeContents valueForKey:@ "Objects"];
		if (nil == storeMetadata || nil == storedObjects) {
			// not my store
			return NO;
		} else {
			[self setMetadata: storeMetadata];
			temp_objects = [storedObjects retain];
		}
	}
	return YES;
}

/*
 * processObjects:
 
 Go through the external representation array and create cache nodes for the 
 represented objects
 */
- (BOOL)processObjects:(NSError **)error {
	if (nil == temp_objects) {
		// This is a new store
		return YES;
	}
	
	NSMutableSet *cacheNodes = [NSMutableSet set];
	NSDictionary *entities = [[[self persistentStoreCoordinator] managedObjectModel] entitiesByName];
	
	for (NSDictionary *externalRepresentation in temp_objects) {
		NSNumber *idReferenceData = [externalRepresentation valueForKey: @"idReferenceData"];
		NSString *entityName = [externalRepresentation valueForKey:@"entityName"];
		
		if (nil == idReferenceData) {
			*error = [NSError errorWithDomain:MyAtomicStoreSubclassDomain code:51 userInfo:nil];
			return NO;
		}
		if (nil == entityName) {
			*error = [NSError errorWithDomain:MyAtomicStoreSubclassDomain code:52 userInfo:nil];
			return NO;
		}
		
		NSEntityDescription *entity = [entities valueForKey:entityName];
		if (nil == entity) {
			// should only end up here if the metadata lied about what was in the store
			*error = [NSError errorWithDomain:MyAtomicStoreSubclassDomain code:53 userInfo:nil];
			return NO;
		}
		
		NSManagedObjectID *objectID = [self objectIDForEntity: entity referenceObject:idReferenceData];
		AtomicStoreCacheNodeSubclass *newNode = [[AtomicStoreCacheNodeSubclass alloc] initWithObjectID:objectID];
		
		[newNode setPropertyCacheData: [externalRepresentation valueForKey:@"propertyData"]];
		[cacheNodes addObject: newNode];
	}
	
	[self addCacheNodes: cacheNodes];
	
	NSError *localError = nil;
	for (AtomicStoreCacheNodeSubclass *node in cacheNodes) {
		[node resolvePropertyValues: &localError];
		if (nil != localError) {
			*error = localError;
			return NO;
		}
	}
	
	[temp_objects release];
	temp_objects = nil;
	return YES;
}

/* Gather up all the external representations of the objects in the cache nodes in
 * preparation for saving
 */
- (NSSet *)getExternalRepresentations {
	NSSet *cacheNodes = [self cacheNodes];
	NSMutableSet *externalRepresentations = [NSMutableSet set];
	
	for (AtomicStoreCacheNodeSubclass *node in cacheNodes) {
		NSMutableDictionary *externalRepresentation  = [NSMutableDictionary dictionary];
		
		[externalRepresentation setValue: [[node entity] name] forKey: @"entityName"];
		[externalRepresentation setValue: [self referenceObjectForObjectID: [node objectID]] forKey:@"idReferenceData"];
		[externalRepresentation setValue: [self getPropertyCacheDataForKVCCompliantObject:node] forKey: @"propertyData"];
		
		[externalRepresentations addObject: externalRepresentation];
	}
	
	return externalRepresentations;
}

/*
 * getPropertyCacheDataForKVCCompliantObject:
 
 Get the external representation of an object from either a managedObject or a cacheNode.
 This takes advantage of the fact that managed objects and cache nodes are very similar:  
 both must respond to valueForKey:attributeName with a valid attribute value, and  
 both must respond to valueForKey:relationshipName with an object (or set of objects,
 in the case of a toMany relationship) which do likewise
 */
- (NSDictionary *)getPropertyCacheDataForKVCCompliantObject:(id)somethingRespondingToValueForKey {
	NSMutableDictionary *newValues = [NSMutableDictionary dictionary];
	
	NSDictionary *attributeDescriptions = [[somethingRespondingToValueForKey entity] attributesByName];
	
	for (NSString *attributeName in attributeDescriptions) {
		NSAttributeDescription *attributeDescription = [attributeDescriptions valueForKey: attributeName];
		id attributeValue = [somethingRespondingToValueForKey valueForKey: attributeName];
		if (NSTransformableAttributeType == [attributeDescription attributeType]) {
			NSString *transformerName = [attributeDescription valueTransformerName];
			attributeValue = (nil == transformerName) ? [[NSValueTransformer valueTransformerForName: NSKeyedUnarchiveFromDataTransformerName] reverseTransformedValue: attributeValue] : [[NSValueTransformer valueTransformerForName: transformerName] transformedValue: attributeValue];
		}
		[newValues setValue: attributeValue forKey: attributeName];
	}
	
	NSDictionary *relationshipDescriptions = [[somethingRespondingToValueForKey entity] relationshipsByName];
	for (NSString *relationshipName in relationshipDescriptions) {
		NSRelationshipDescription *relationshipDescription = [relationshipDescriptions valueForKey: relationshipName];
		NSMutableSet *destinationInfos = [NSMutableSet set];
		
		if (![relationshipDescription isToMany]) {
			id destinationObject = [somethingRespondingToValueForKey valueForKey: relationshipName];
			if (nil != destinationObject) {
				[destinationInfos addObject: [NSArray arrayWithObjects: [[destinationObject entity] name], [self referenceObjectForObjectID: [destinationObject objectID]], nil]];
			}
		} else {
			for (NSManagedObject *destinationObject in [somethingRespondingToValueForKey valueForKey: relationshipName]) {
				[destinationInfos addObject: [NSArray arrayWithObjects: [[destinationObject entity] name], [self referenceObjectForObjectID: [destinationObject objectID]], nil]];
			}
		}
		
		[newValues setValue: destinationInfos forKey: relationshipName];
	}
	
	return newValues;
}

/*
 * initWithPersistentStoreCoordinator:configurationName:URL:options:
 
 Do necessary initialization; return nil up front if for some reason the URL
 or its contents are known bad
 */
- (id)initWithPersistentStoreCoordinator:(NSPersistentStoreCoordinator *)coordinator configurationName:(NSString *)configurationName URL:(NSURL*)url options:(NSDictionary *)options {
	if (nil == url || ![url isFileURL]) {
		[self release];
		return nil;
	}
	if (nil != (self = [super initWithPersistentStoreCoordinator:coordinator configurationName:configurationName URL:url options:options])) {
		NSFileManager *fileManager = [NSFileManager defaultManager];
		NSString *filePath = [url path];
		BOOL isDirectory;
		if (![fileManager fileExistsAtPath:filePath isDirectory:&isDirectory]) {
			// no file, create one
			if (![self createStubAndGenerateMetadata]) {
				// can only get here if we can't create the stub
				[self release];
				return nil;
			}
		} else if (isDirectory) {
			// directories aren't my type of store
			[self release];
			return nil;
		} else {
			if (![self readFile]) {
				// can only get here if we can't read the file, so it isn't a store, or we can't read it, or something
				[self release];
				return nil;
			}
		}
	}
	return self;
}


/*
 * dealloc
 
 release our variables
 */
- (void)dealloc {
	[temp_objects release], temp_objects = nil;
	[store_identifier release], store_identifier = nil;
	[super dealloc];
}

#pragma - NSPersistentStore required overrides

/*
 * metadataForPersistentStoreWithURL:
 
 Try to load the store to get metadata - not very efficient, but simple
 */
+ (NSDictionary *)metadataForPersistentStoreWithURL:(NSURL*)url error:(NSError **)error {
	NSData *storeContents = nil;
	
	@try {
		storeContents = [NSKeyedUnarchiver unarchiveObjectWithData: [NSData dataWithContentsOfURL:url]];
	} @catch (NSException *ex) {
		*error = [NSError errorWithDomain:MyAtomicStoreSubclassDomain code:1 userInfo:nil];
		return NO;
	}
	
	if (nil == storeContents) {
		if (nil != error) {
			*error = [NSError errorWithDomain:MyAtomicStoreSubclassDomain code:2 userInfo:nil];
		}
		return nil;
	} else {
		if (! [storeContents isKindOfClass: [NSDictionary class]])  {
			if (nil != error) {
				*error = [NSError errorWithDomain:MyAtomicStoreSubclassDomain code:3 userInfo:nil];
			}
			return nil;
		}
		NSDictionary *storeMetadata = [storeContents valueForKey:@"Metadata"];
		NSDictionary *storedObjects = [storeContents valueForKey:@ "Objects"];
		if (nil == storeMetadata || nil == storedObjects) {
			if (nil != error) {
				*error = [NSError errorWithDomain:MyAtomicStoreSubclassDomain code:4 userInfo:nil];
			}
			return nil;
		} else {
			return storeMetadata;
		}
	}
}

/*
 * setMetadata:forPersistentStoreWithURL:error:
 
 Try to set metadata
 */
+ (BOOL)setMetadata:(NSDictionary *)metadata forPersistentStoreWithURL:(NSURL*)url error:(NSError **)error {
	NSData *storeContents = nil;
	
	@try {
		storeContents = [NSKeyedUnarchiver unarchiveObjectWithData: [NSData dataWithContentsOfURL:url]];
	} @catch (NSException *ex) {
		*error = [NSError errorWithDomain:MyAtomicStoreSubclassDomain code:11 userInfo:nil];
		return NO;
	}
	
	if (nil == storeContents) {
		if (nil != error) {
			*error = [NSError errorWithDomain:MyAtomicStoreSubclassDomain code:12 userInfo:nil];
		}
		return NO;
	} else {
		if (! [storeContents isKindOfClass: [NSDictionary class]])  {
			if (nil != error) {
				*error = [NSError errorWithDomain:MyAtomicStoreSubclassDomain code:13 userInfo:nil];
			}
			return NO;
		}
		NSDictionary *storeMetadata = [storeContents valueForKey:@"Metadata"];
		NSDictionary *storedObjects = [storeContents valueForKey:@ "Objects"];
		if (nil == storeMetadata || nil == storedObjects) {
			if (nil != error) {
				*error = [NSError errorWithDomain:MyAtomicStoreSubclassDomain code:14 userInfo:nil];
			}
			return NO;
		} else {
			NSString *storeIdentifier = [storeMetadata valueForKey: NSStoreUUIDKey];
			NSString *storeType = [storeMetadata valueForKey: NSStoreTypeKey];
			if (nil == storeIdentifier || nil == storeType) {
				if (nil != error) {
					*error = [NSError errorWithDomain:MyAtomicStoreSubclassDomain code:15 userInfo:nil];
				}
				return NO;
			}
			NSMutableDictionary *newStoreContents = [NSMutableDictionary dictionary];
			NSMutableDictionary *newMetadata = [metadata mutableCopy];

			
			if ([newMetadata objectForKey:@"NSStoreModelVersionHashesVersion"] == nil) {
				[newMetadata setObject:[storeMetadata objectForKey:@"NSStoreModelVersionHashesVersion"] forKey: @"NSStoreModelVersionHashesVersion"];
			}
			if ([newMetadata objectForKey:NSStoreModelVersionHashesKey] == nil) {
				[newMetadata setObject:[storeMetadata objectForKey:NSStoreModelVersionHashesKey] forKey: NSStoreModelVersionHashesKey];
			}
			if ([newMetadata objectForKey:NSStoreModelVersionIdentifiersKey] == nil) {
				[newMetadata setObject:[storeMetadata objectForKey:NSStoreModelVersionIdentifiersKey] forKey: NSStoreModelVersionIdentifiersKey];
			}
			if ([newMetadata objectForKey:NSStoreTypeKey] == nil) {
				[newMetadata setObject:[storeMetadata objectForKey:NSStoreTypeKey] forKey: NSStoreTypeKey];
			}
			if ([newMetadata objectForKey:NSStoreUUIDKey] == nil) {
				[newMetadata setObject:[storeMetadata objectForKey:NSStoreUUIDKey] forKey: NSStoreUUIDKey];
			}
			[newStoreContents setValue: newMetadata forKey: @"Metadata"];
			[newMetadata release];
			[newStoreContents setValue: storedObjects forKey: @"Objects"];
			NSData *newData = [NSKeyedArchiver archivedDataWithRootObject:newStoreContents];
			[newData writeToURL:url atomically:YES];
		}
	}
	return YES;
}

/*
 * type:
 
 Return a type for this store
 */
- (NSString *)type {
	return @"MyAtomicStoreSubclass";
}

/*
 * identifier:
 
 Return the identifier for this store
 */
- (NSString *)identifier {
	return store_identifier;
}

/*
 * setIdentifier:
 
 Set the identifier for this store
 */
- (void)setIdentifier:(NSString *)identifier {
	if (identifier != store_identifier) {
		[identifier retain];
		[store_identifier release];
		store_identifier = identifier;
	}
}

#pragma - NSAtomicStore required overrides 

/*
 * load:
 
 Load the objects; the metadata was loaded earlier
 */
- (BOOL)load:(NSError **)error {
	if (nil != temp_objects) {
		NSError *localError = nil;
		BOOL success = [self processObjects:&localError];
		if (success) {
			return YES;
		}
		if (nil != error) {
			*error = localError;
		}
	} else {
		return YES;
	}
	return NO;
}


/*
 save:
 
 use standard archiving to save the metadata and the external representations
 of the objects to disk 
 */
- (BOOL)save:(NSError **)error {
	if ([self isReadOnly]) {
		if (nil != error) {
			*error = [NSError errorWithDomain:MyAtomicStoreSubclassDomain code:31 userInfo:nil];
		}
	}
	NSMutableDictionary *dictionaryToSave = [NSMutableDictionary dictionary];
	
	[dictionaryToSave setValue: [self metadata] forKey:@"Metadata"];
	
	[dictionaryToSave setValue: [self getExternalRepresentations] forKey:@"Objects"];
	
	NSData *encodedStore = [NSKeyedArchiver archivedDataWithRootObject:dictionaryToSave];
	
	if (!encodedStore) {
		if (nil != error) {
			*error = [NSError errorWithDomain:MyAtomicStoreSubclassDomain code:32 userInfo:nil];
			return NO;
		}
	} 
	BOOL success = [encodedStore writeToURL:[self URL] atomically:YES];
	if (!success) {
		*error = [NSError errorWithDomain:MyAtomicStoreSubclassDomain code:33 userInfo:nil];
	}
	removeStub = NO;
	return success;
}

/*
 * newReferenceObjectForManagedObject:
 
 Reference objectIDs are constructed using a monotonically increasing number
 across all object types; this ensures uniqueness within inheritance hierarchies
 */
- (id)newReferenceObjectForManagedObject:(NSManagedObject*)managedObject {
	NSDictionary *metadata = [self metadata];
	long nextKey = 0;
	NSNumber *lastReferenceObject = [metadata valueForKey:@"lastReferenceObject"];
	if (nil == lastReferenceObject) {
		nextKey = 1;
	} else {
		nextKey = [lastReferenceObject longValue];
		nextKey++;
	}
	NSNumber *newReferenceObject = [NSNumber numberWithLong: nextKey];
	
	NSMutableDictionary *newMetadata = [[self metadata] mutableCopy];
	[newMetadata setValue: newReferenceObject forKey: @"lastReferenceObject"];
	[self setMetadata: newMetadata];
	[newMetadata release];

	return [newReferenceObject retain];
	
}

/*
 * newCacheNodeForManagedObject:
 
 Create and populate a new cache node by reusing the mechanism that unpacks the
 external representation of the object
 */
- (NSAtomicStoreCacheNode *)newCacheNodeForManagedObject:(NSManagedObject *)managedObject {
	AtomicStoreCacheNodeSubclass *newNode = [[AtomicStoreCacheNodeSubclass alloc] initWithObjectID: [managedObject objectID]];
	[newNode setPropertyCacheData: [self getPropertyCacheDataForKVCCompliantObject: managedObject]];
	return newNode;
}

/*
 * updateCacheNode:
 
 Update the cache node by reusing the mechanism that unpacks the
 external representation of the object
 */
- (void)updateCacheNode:(NSAtomicStoreCacheNode *)node fromManagedObject:(NSManagedObject *)managedObject {
	[(AtomicStoreCacheNodeSubclass *)node setPropertyCache: nil];
	[(AtomicStoreCacheNodeSubclass *)node setPropertyCacheData: [self getPropertyCacheDataForKVCCompliantObject: managedObject]];
}

#pragma - graph management overrides

/*
 * willRemoveCacheNodes:
 
 The nodes are being removed, so break any retain cycles that may have been created
 between them by removing the property cache 
 */
 - (void)willRemoveCacheNodes:(NSSet *)cacheNodes {
	for (NSAtomicStoreCacheNode *node in cacheNodes) {
		[node setPropertyCache: nil];
	}
}

/*
 * willRemoveFromPersistentStoreCoordinator:
 
 Only called when a store is being removed from the coordinator
 do memory cleanup by removing all property caches
 and the retain cycles they contain
 */
 - (void)willRemoveFromPersistentStoreCoordinator:(NSPersistentStoreCoordinator *)coordinator {
 	for (NSAtomicStoreCacheNode *node in [self cacheNodes]) {
 		[node setPropertyCache: nil];
 	}
	
	if (removeStub) {
		[[NSFileManager defaultManager] removeFileAtPath: [[self URL] path] handler:nil];
	}
	[super willRemoveFromPersistentStoreCoordinator:coordinator];
}

@end

