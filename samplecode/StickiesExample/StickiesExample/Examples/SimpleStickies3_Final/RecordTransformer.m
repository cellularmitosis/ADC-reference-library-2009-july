/*
 
 File: RrecordTransformer.m
 
 Abstract: Transforms a sync record to a local record and back. 
 Used during the pulling and pushing phases.
 
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

// RecordTransformer transforms a KVC compliant object into a sync record and back.
// When pushing records, Sync Services expects a dictionary not an object that is simply KVC compliant.
// It's best to create the dictionary from scratch to separate any non-persistent properties 
// you may have added to the records. Also attributes and relationsihps need to be handled 
// differently. Similarly, when pulling records, you need to transform the Sync Services representation
// to your KVC compliant object. In this example, the KVC compliant object happens to be a dictionary
// too, although distinctly different from the Sync Services dictionary (for Cocoa Bindings to work).

// Use transformedValue: to transform your KVC compliant object to a Sync Services record, and use
// reverseTransformedValue: to transform a Sync Services record to your KVC object.

// NOTE: This class inherits from NSValueTransformer although it is not currently used as a transformer
// for Cocoa Bindings, just for converting records when syncing.

#import <SyncServices/SyncServices.h>
#import "DataSource.h"
#import "EntityModel.h"
#import "Entity.h"
#import "RecordTransformer.h"

@implementation RecordTransformer

// Returns an NSObject since any KVC compliant object can be transformed using this class.
+ (Class)transformedValueClass
{
    return [NSObject self];
}

// Returns NO. We do "reverse transformations" of a sort, but not in the Cocoa Bindings
// sense - these are done to get back from a Sync Services Record and id to
// an entity object, and the API is different.
+ (BOOL)supportsReverseTransformation
{
    return NO;
}


// Creating record transformers

// Convenience method for creating and registering instances with NSValueTransformer
+ (RecordTransformer *)recordTransformerForEntityName:(NSString *)aName andDataSource:(id)aSource
{
	id transformer = [[RecordTransformer alloc] init];
	[transformer setDataSource:aSource];
	[transformer setEntityName:aName];
	return [transformer autorelease];
}

- (void)dealloc {
	[dataSource release];
    [entityName release];
    [super dealloc];
}


// Getting and setting record transformer properties

- (NSString *)entityName
{
    return entityName;
}

- (void)setEntityName:(NSString *)aString
{
    [entityName release];
    entityName = [aString retain];
    return;
}

- (id)dataSource
{
	return dataSource;
}

- (void)setDataSource:(id)anObject
{
    [dataSource release];
    dataSource = [anObject retain];
    return;
}


// Transforming objects

// Transforms the application's entity object into a Sync Services record. The beforeObject is 
// simply any KVC compliant object that has properties defined by the specified entity model.
// Returns a record dictionary used by Sync Services for push operations.
- (id)transformedValue:(id)beforeObject
{
	unsigned ii, count;
    //NSLog(@"RecordTransformer transformedValue:");
    //NSLog(@"beforeObject=%@", [beforeObject description]);

    if ((beforeObject == nil) || (dataSource == nil) || (entityName == nil))
		return nil;
	
	NSMutableDictionary *afterObject = [NSMutableDictionary dictionary];
				
	// Add the attribute key-value pairs to the dictionary first.
	NSArray *attributeKeys = [dataSource attributeKeysForEntityName:entityName];
	for (ii = 0, count = [attributeKeys count]; ii < count; ii++) {
		id key = [attributeKeys objectAtIndex:ii];
		id value = [beforeObject valueForKey:key];
		if (value != NULL) [afterObject setValue:value forKey:key];
	}
	
	// Now add the relationships (if any) to the dictionary by converting relationship key
	// to the unique, long name that Sync Services expects.
	NSArray *relationshipKeys = [dataSource relationshipKeysForEntityName:entityName];
	for (ii = 0, count = [relationshipKeys count]; ii < count; ii++) {
		//NSMutableArray *array = [NSMutableArray array];
		id key = [relationshipKeys objectAtIndex:ii];
		id value = [beforeObject valueForKey:key];
		//NSLog(@"relationship key=%@ value=%@", key, [value description]);
		[afterObject setValue:value forKey:key];
	}				
	
	[afterObject setValue:entityName forKey:ISyncRecordEntityNameKey]; // sync engine requires this property		
	
	//NSLog(@"afterObject=%@", [afterObject description]);	
	return afterObject;
}

// Returns the object that corresponds to the specified sync record and record identifier. 
// Returns nil if the object could not be found.
- (id)reverseTransformedValue:(id)beforeObject withRecordIdentifier:(NSString*)recordIdentifier
{
	// Get the matching record, otherwise return nil.
	id recordEntityName = [beforeObject valueForKey:ISyncRecordEntityNameKey];
	id afterObject = [dataSource recordWithPrimaryKey:recordIdentifier forEntityName:recordEntityName];
	
	return afterObject;
}

// Adds a new record pulled from the sync engine, or applies changes to an existing object.
// Uses reverseTransformedValue:withRecordIdentifier: to get an existing record.
- (id)reverseTransformedValueWithChange:(ISyncChange *)syncChange
{
	id beforeObject = [syncChange record];
    //NSLog(@"RecordTransformer reverseTransformedValueWithChange:");
	//NSLog(@"syncChanges=%@", [syncChange changes]);
    //NSLog(@"beforeObject=%@", [beforeObject description]);
	
	// Get the matching record or create a new record
	id afterObject = [self reverseTransformedValue:beforeObject withRecordIdentifier:[syncChange recordIdentifier]];
	id recordEntityName = [beforeObject valueForKey:ISyncRecordEntityNameKey];

	if (afterObject == nil) {
		NSLog(@"Adding new record %@:\n%@", [syncChange recordIdentifier], [beforeObject description]);
        // Create a new entity object
		NSString *className = [[[dataSource entityModel] entityWithEntityName:recordEntityName] className];
		afterObject = [[NSClassFromString(className) alloc] init];
		[afterObject takeStoredValue:[syncChange recordIdentifier] forKey:@"primaryKey"];
		
		// Need to manually copy over the attribute values
		NSArray *attributeKeys = [dataSource attributeKeysForEntityName:recordEntityName];
		unsigned ii, iicount = [attributeKeys count];
		//NSLog(@"Setting attributes, count=%d...", iicount);
		for (ii = 0; ii < iicount; ii++){
			id key = [attributeKeys objectAtIndex:ii];
			[afterObject takeStoredValue:[beforeObject valueForKey:key] forKey:key];
		}
		
		// Need to transform the sync relationship keys to keys used by the entity model
		NSArray *relationshipKeys = [dataSource relationshipKeysForEntityName:recordEntityName];
		unsigned jj, jjcount = [relationshipKeys count];
		//NSLog(@"Setting relationships, count=%d...", jjcount);
		for (jj = 0; jj < jjcount; jj++) {
			id key = [relationshipKeys objectAtIndex:jj]; // get the application relationship key
			[afterObject takeStoredValue:[beforeObject valueForKey:key] forKey:key];
		}				
		[dataSource addRecord:afterObject forEntityName:recordEntityName];
        [afterObject autorelease];
	}
	// Otherwise just apply the changes to the existing entity.
	else {
		//NSLog(@"Modifying existing record %@ with changes:\n%@", [syncChange recordIdentifier], [[syncChange changes] description]);
        NSEnumerator *enumerator = [[syncChange changes] objectEnumerator];
        id change;
		
		NSArray *attributeKeys = [dataSource attributeKeysForEntityName:entityName];
		NSArray *relationshipKeys = [dataSource relationshipKeysForEntityName:recordEntityName];
        while (change = [enumerator nextObject]) {
			// Apply attribute changes
			id key = [change valueForKey:ISyncChangePropertyNameKey];
			if ([attributeKeys containsObject:key]){
				[afterObject takeValue:[change valueForKey:ISyncChangePropertyValueKey] forKey:key];
			}
			// Apply relationship changes
			else {
				if ([relationshipKeys containsObject:key]){
					[afterObject takeValue:[change valueForKey:ISyncChangePropertyValueKey] forKey:key];
				}
			}
			// NOTE: the above code checks if the key is really a property key because other non-properties may be contained in there.
        }
	}
	
	//NSLog(@"afterObject=%@", [afterObject description]);	
	return afterObject;
}

@end
