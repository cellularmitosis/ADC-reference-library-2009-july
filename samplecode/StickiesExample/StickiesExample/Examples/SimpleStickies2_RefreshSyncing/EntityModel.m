/*
 
 File: EntityModel.m
 
 Abstract: Provides a simple entity-relationship model for describing 
 records, their attributes and relationships.
 
 Version: 1.0
 
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

// EntitModel maintains an entity-relationship model representation used by a data source
// to define its schema. Uses an Entity object to represent each application entity--defines 
// the entity's properties, including to-one and to-many relationships. This class can be
// used independent of Sync Services.

#import "EntityModel.h"
#import "Entity.h"

static NSString *INDEX_FILE_NAME = @"index.plist";

@implementation EntityModel

// Initializes the entity model representation
- (id)init
{
    self = [super init];
    if (self) {
		_representation = [[NSMutableDictionary dictionary] retain];
		[_representation setValue:[NSMutableArray array] forKey:@"entities"];
    }
	
    return self;
}

- (id)initWithFileWrapper:(id)fileWrapper
{
    self = [super init];
    if (self && [fileWrapper isDirectory]) {
		NSDictionary *wrappers = [fileWrapper fileWrappers];
		NSArray *allKeys = [wrappers allKeys];

		// First set the entity model attributes and initial entities array
		id indexWrapper = [wrappers valueForKey:INDEX_FILE_NAME];
		if (indexWrapper != nil){
			NSData *data = [indexWrapper regularFileContents];
			NSString *error;

			// Convert the raw data to a dictionary
			id dict = (NSDictionary *)CFPropertyListCreateFromXMLData(kCFAllocatorDefault, (CFDataRef)data, 
																	  kCFPropertyListMutableContainersAndLeaves, 
																	  (CFStringRef *)&error);
			if (error != nil)
				NSLog(@"ERROR in CFPropertyListCreateFromXMLData() decoding entity data: %@", error);
			
			// Set the properties of the entity model
			[_representation release];
			_representation = [dict retain];
			[_representation setValue:[NSMutableArray array] forKey:@"entities"];			
		}
		
		// Now load all the entities, if they exist
		if ([allKeys count] > 1){
			NSEnumerator *enumerator = [[wrappers allKeys] objectEnumerator];
			id key;
			
			while (key = [enumerator nextObject]) {
				if (![key isEqual:INDEX_FILE_NAME] && [key hasSuffix:@".plist"]){
					NSData *data = [[wrappers valueForKey:key] regularFileContents];
					NSString *error;
					
					// Convert the raw data to a dictionary
					id dict = (NSDictionary *)CFPropertyListCreateFromXMLData(kCFAllocatorDefault, (CFDataRef)data, 
																			  kCFPropertyListMutableContainersAndLeaves, 
																			  (CFStringRef *)&error);
					if (error !=nil)
						NSLog(@"error decoding entity: %@", error);
					
					// Create and add the entity
					id entity = [Entity entityWithDictionary:dict];
					[[self allEntities] addObject:entity];
				}
			}
		}
	}
	
	return self;
}

- (void)dealloc {
    [_representation release];
    [super dealloc];
}

// Returns a file wrapper representation of the receiver, where the receiver and
// each entity has its own plist file in the directory.
- (NSFileWrapper *)fileWrapperRepresentation
{
	NSFileWrapper *directoryWrapper = [[NSFileWrapper alloc] initDirectoryWithFileWrappers:nil];
	NSEnumerator *enumerator;
	id entity;
	
	// Add a file wrapper per entity to the returned directory wrapper
	enumerator = [[self allEntities] objectEnumerator];
	while (entity = [enumerator nextObject]) {
		NSData *data = [entity fileRepresentation];
		[directoryWrapper addRegularFileWithContents:data preferredFilename:
			[[entity valueForKey:@"name"] stringByAppendingPathExtension:@"plist"]];
    }
	
	// Store the entity model attributes
	// This is sorta klunky
	NSDictionary *attrDict = [NSDictionary dictionaryWithObjectsAndKeys:[self name], @"name", nil];
	NSData *emdata = (NSData *)CFPropertyListCreateXMLData(kCFAllocatorDefault, (CFPropertyListRef)attrDict);
	[directoryWrapper addRegularFileWithContents:emdata preferredFilename:INDEX_FILE_NAME];
	
	return directoryWrapper;	
}


// Initializes the entity model with the contents of content.
- (id)initWithContentsOfFile:(NSString *)filePath
{
	NSLog(@"filePath=%@", filePath);
	return [self initWithFileWrapper:[[NSFileWrapper alloc] initWithPath:filePath]];
}


- (NSString *)description
{
	NSString *myDescription = [NSString stringWithFormat:@"[\n%@ = %@\n]", 
		@"_representation", [_representation description]];
	return myDescription;
}

- (NSMutableArray *)allEntities
{
	return [_representation mutableArrayValueForKey:@"entities"];
}

// Returns an array of the entity names defined in the entity model
- (NSArray *)entityNames
{
	id values = [[self allEntities] valueForKey:@"entityName"];
	return values;
}

- (NSString *)name
{
	return [_representation valueForKey:@"name"];
}

// Returns the propertyKeys for the specified entity
- (NSArray *)propertyKeysForEntityName:(id)entityName
{
	return [[self entityWithEntityName:entityName] valueForKey:@"propertyKeys"];
}

- (NSArray *)attributeKeysForEntityName:(id)entityName
{
	return [[self entityWithEntityName:entityName] valueForKey:@"attributeKeys"];
}

- (NSArray *)relationshipKeysForEntityName:(id)entityName
{
	id relationshipKeys = [[self entityWithEntityName:entityName] valueForKey:@"relationshipKeys"];
	//NSLog (@"relationshipKeys for entity name=%@ is %@", entityName, [relationshipKeys description]);
	return relationshipKeys;
}

- (void)setName:(NSString *)name
{
	[_representation setValue:name forKey:@"name"];
	return;
}

- (Entity *)entityWithEntityName:(NSString *)entityName
{
	if (entityName == nil) return nil;
	NSEnumerator *enumerator = [[self allEntities] objectEnumerator];
    id entity = [enumerator nextObject];
	
	while ((entity != nil) && ![[entity valueForKey:@"entityName"] isEqual:entityName]) {
		entity = [enumerator nextObject];
    }
	return entity;	
}

- (Entity *)entityWithName:(NSString *)name
{
	if (name == nil) return nil;
	NSEnumerator *enumerator = [[self allEntities] objectEnumerator];
    id entity = [enumerator nextObject];
	
	while ((entity != nil) && ![[entity valueForKey:@"name"] isEqual:name]) {
		entity = [enumerator nextObject];
    }
	return entity;	
}

// Archiving support

- (NSData *)xmlValue {
	return (NSData *)CFPropertyListCreateXMLData(kCFAllocatorDefault, (CFPropertyListRef)[self allEntities]);
}

// KVC Compliant Methods

- (id)valueForUndefinedKey:(NSString *)key
{
	NSLog(@"value for undefined key=%@", key);
	return nil;
}

- (void)setValue:(id)value forUndefinedKey:(NSString *)key
{
	NSLog(@"set value for undefined key=%@", key);
	return;
}

- (void)insertObject:(id)anObject inEntitiesAtIndex:(unsigned int)index 
{
	[[self allEntities] insertObject:anObject atIndex:index];
    return;	
}

- (void)removeObjectFromEntitiesAtIndex:(unsigned int)index 
{
	[[self allEntities] removeObjectAtIndex:index];
    return;	
}

@end
