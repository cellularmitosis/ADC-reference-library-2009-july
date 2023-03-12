/*
 
 File: Entity.m
 
 Abstract: Describes an entity belonging to an EntityModel.
 
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

// Encapsulates information about a single entity used in an EntityModel.
// An EntityModel defines the mapping between application Entity objects
// and the data source schema. This class can be used independent of 
// Sync Services.

#import "Entity.h"

@implementation Entity

// Creating an Entity object

// Creates and returns a new entity represented by the given dictionary.
+ (id)entityWithDictionary:(NSDictionary *)dict
{
	return [[[Entity alloc] initWithDictionary:dict] autorelease];	
}

// Initialization and deallocation

- (id)init
{
    self = [super init];
    if (self) {
		_representation = [[NSMutableDictionary dictionary] retain];
		[_representation setValue:[NSMutableArray array] forKey:@"attributes"];
		[_representation setValue:[NSMutableArray array]forKey:@"relationships"];		
		[_representation setValue:[NSMutableArray array]forKey:@"identityKeys"];		
    }
	
    return self;
}

- (id)initWithDictionary:(NSDictionary *)dict
{
    self = [super init];
    if (self) {
		[_representation release];
		_representation = [[NSMutableDictionary dictionaryWithDictionary:dict] retain];
	}
	return self;
}

- (void)dealloc {
    [_representation release];
    [super dealloc];
}


// Saving and loading support

// Returns an NSData representation of the receiver for saving to disk.
- (NSData *)fileRepresentation
{
	return (NSData *)CFPropertyListCreateXMLData(kCFAllocatorDefault, (CFPropertyListRef)_representation);
}

// Returns a printable string representation of the receiver.
- (NSString *)description
{
	NSString *myDescription = [NSString stringWithFormat:@"[\n%@ = %@\n]", 
		@"_representation", [_representation description]];
	return myDescription;
}

// Adding and removing properties

// Adds an attribute to the receiver.
- (void)addAttribute:(id)attribute
{
	[[self attributes] addObject:attribute];
	return;
}

// Removes an attribute from the receiver.
- (void)removeAttribute:(id)attribute
{
	[[self attributes] removeObject:attribute];
	return;
}

// Adds a relationship to the receiver.
- (void)addRelationship:(id)relationship
{
	[[self relationships] addObject:relationship];
	return;
}

// Removes a relationship from the receiver.
- (void)removeRelationship:(id)relationship
{
	[[self relationships] removeObject:relationship];
	return;
}

// Getting and setting entity properties

// Returns the name of the receiver.
- (NSString *)name
{
	return [_representation valueForKey:@"name"];
}

// Sets the name of the receiver to name.
- (void)setName:(NSString *)name
{
	[_representation setValue:name forKey:@"name"];
	return;
}

// Returns the class name used to create instances of this entity.
- (NSString *)className
{
	return [_representation valueForKey:@"className"];
}

// Sets the class name used to create instances of this entity to className.
- (void)setClassName:(NSString *)className
{
	[_representation setValue:className forKey:@"className"];
	return;
}

// Returns a user readable name of the receiver.
- (NSString *)displayName
{
	return [_representation valueForKey:@"displayName"];
}

// Sets the user readable name of the receiver to displayName.
- (void)setDisplayName:(NSString *)displayName
{
	[_representation setValue:displayName forKey:@"displayName"];
	return;
}

- (NSString *)entityName
{
	return [_representation valueForKey:@"entityName"];
}

- (void)setEntityName:(NSString *)entityName
{
	[_representation setValue:entityName forKey:@"entityName"];
	return;
}

// Returns an array of the property keys that are used to uniquely identify records of this entity.
- (NSArray *)identityKeys
{
	return [_representation valueForKey:@"identityKeys"];
}

// Sets the identity keys for this entity to identityKeys.
- (void)setIdentityKeys:(NSArray *)identityKeys
{
	[_representation setValue:identityKeys forKey:@"identityKeys"];
	return;
}

// Returns YES if records of this entity are read only, NO otherwise.
- (BOOL)isReadOnly
{
	return NO; // not supported yet
}

- (void)setReadOnly:(BOOL)flag;
{
	return; // not supported yet
}

- (NSArray *)propertyKeys
{
	NSArray *array1 = [[self attributes] valueForKey:@"name"];
	NSArray *array2 = [[self relationships] valueForKey:@"name"];
	NSMutableArray *propertyKeys = [NSMutableArray array];
	[propertyKeys addObjectsFromArray:array1];
	[propertyKeys addObjectsFromArray:array2];
	return propertyKeys;
}


// Getting and setting attribute properties

- (id)attributeNamed:(NSString *)name
{
	NSEnumerator *enumerator = [[self attributes] objectEnumerator];
	id anObject;
	
	for (anObject = [enumerator nextObject]; ![[anObject valueForKey:@"name"] isEqual:name];){
		anObject = [enumerator nextObject];
	}
	return anObject;
}

- (NSMutableArray *)attributes
{
	return [_representation mutableArrayValueForKey:@"attributes"];
}

- (NSArray *)attributeKeys
{
	return [[self attributes] valueForKey:@"name"];
}

// Getting and setting relationship properties

- (id)relationshipNamed:(NSString *)name
{
	NSEnumerator *enumerator = [[self relationships] objectEnumerator];
	id anObject;
	
	for (anObject = [enumerator nextObject]; ![[anObject valueForKey:@"name"] isEqual:name];){
		anObject = [enumerator nextObject];
	}
	return anObject;
}

- (NSMutableArray *)relationships
{
	return [_representation mutableArrayValueForKey:@"relationships"];
}

- (NSArray *)relationshipKeys
{
	return [[self relationships] valueForKey:@"name"];
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

- (void)insertObject:(id)anObject inAttributesAtIndex:(unsigned int)index 
{
	[[self attributes] insertObject:anObject atIndex:index];
    return;	
}

- (void)removeObjectFromAttributesAtIndex:(unsigned int)index 
{
	[[self attributes] removeObjectAtIndex:index];
    return;	
}

- (void)insertObject:(id)anObject inRelationshipsAtIndex:(unsigned int)index 
{
	[[self relationships] insertObject:anObject atIndex:index];
    return;	
}

- (void)removeObjectFromRelationshipsAtIndex:(unsigned int)index 
{
	[[self relationships] removeObjectAtIndex:index];
    return;	
}

@end
