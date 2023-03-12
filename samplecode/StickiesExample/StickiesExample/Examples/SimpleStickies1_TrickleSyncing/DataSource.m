/*
 
 File: DataSource.m
 
 Abstract: Provides a simple table representation and XML storage for 
 KVC-compliant records.
 
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

#import "DataSource.h"
#import "EntityModel.h"

static NSString *ENTITY_MODEL_FILE_NAME = @"EntityModel.plistd";
static NSString *RECORDS_SUFFIX = @"Records";

@interface DataSource(DataSourcePrivate)
// Private KVO support
- (void)_observeRecord:(id)record withEntityName:(NSString *)entityName;
- (void)_unobserveRecord:(id)record withEntityName:(NSString *)entityName;
- (void)_didAddRecords:(id)newRecords forEntityName:(NSString *)entityName;
- (void)_didRemoveRecords:(id)oldRecords forEntityName:(NSString *)entityName;

// Private managing changes
- (void)_postChangeNotification;

// Private table and record methods
- (id)_tableForEntityName:(NSString *)entityName;
- (void)_setRecords:(NSMutableArray *)newRecords forEntityName:(NSString *)entityName;
- (void)_validateRecord:(id)record;
- (NSString *)_newPrimaryKey;
@end

// DataSource implements a simple data source for saving and loading records to a plist, and read/write
// access from an application. This data source also keeps track of the changed and deleted records
// to support syncing. It uses the EntityModel to define the schema, and expects all records to be KVO
// compliant. This class can be used independent of Sync Services too.

@implementation DataSource

// Returns the default file name for the entity model.
+ (NSString *)defaultEntityModelFileName
{
	return ENTITY_MODEL_FILE_NAME;
}

// Initializing a data source

- (id)init
{
    self = [super init];
    if (self) {
		_representation = [[NSMutableDictionary dictionary] retain];
		[_representation setValue:[NSMutableArray array] forKey:@"tables"];
		_batching = NO;
		_dataChanged = NO;
    }
	
    return self;
}

// Designated initializer for this class that creates and returns a new data source object
// with the specified entity model.
- (id)initWithModel:(id)aModel
{
    self = [self init];
    if (self) {
		[self setEntityModel:aModel]; // creates initial empty tables
    }
	
    return self;
}

- (void)dealloc {
	[_entityModel release];
    [_representation release];
    [super dealloc];
}


// Getting and setting data source properties

// Returns the receiver's entity model.
- (id)entityModel
{
	return _entityModel;
}

// Sets the receiver's entity model to aModel.
- (void)setEntityModel:(id)aModel
{
	[_entityModel release];
	[self setTables:[NSMutableArray array]]; // releases the old records
	_entityModel = [aModel retain];
	
	// Set up the tables with initial "empty" collections of records
	if (_entityModel != nil){
		NSEnumerator *enumerator;
		id entityName, table;
		NSMutableArray *tables = [_representation valueForKey:@"tables"];
		
		enumerator = [[_entityModel entityNames] objectEnumerator];	
		while (entityName = [enumerator nextObject]) {
			table = [NSMutableDictionary dictionaryWithObjectsAndKeys:
				entityName, @"entityName",
				[NSMutableArray array], @"records",
				[NSMutableArray array], @"deleted",
				nil];
			[tables addObject:table];
			
			// Add the data source as an observer of the records array using a key path
			//NSLog(@"addObserver:... keyPath=%@", [self keyForEntityName:entityName]);
			[self addObserver:self forKeyPath:[self keyForEntityName:entityName]
					  options:(NSKeyValueObservingOptionNew | NSKeyValueObservingOptionOld) context:NULL];
		}
	}
}

// Returns the internal representation of the receiver.
- (NSData *) representation {
	return [[_representation retain] autorelease];
}

// Sets the internal representation of the receiver to representation, a dictionary of key-value pairs.
- (void)setRepresentation:(NSMutableDictionary *)representation
{
	NSEnumerator *tableEnumerator;
	NSDictionary *table;
	NSString *entityName;
	
	// Should remove this object as an observer of each record before releasing the old representation.
	tableEnumerator = [[_representation valueForKey:@"tables"] objectEnumerator];
	while (table = [tableEnumerator nextObject]){
		NSEnumerator *recordsEnumerator = [[table valueForKey:@"records"] objectEnumerator];
		id record;
		entityName = [table valueForKey:@"entityName"];
		while (record = [recordsEnumerator nextObject]){
			[self _unobserveRecord:record withEntityName:entityName];		
		}
	}
	
	[_representation release];
	_representation = [representation retain];
		
	// Now add the recevier as an observer of each record in each table (for all property changes)
	tableEnumerator = [[_representation valueForKey:@"tables"] objectEnumerator];
	while (table = [tableEnumerator nextObject]){
		NSEnumerator *recordsEnumerator = [[table valueForKey:@"records"] objectEnumerator];
		id record;
		entityName = [table valueForKey:@"entityName"];
		while (record = [recordsEnumerator nextObject]){
			[self _observeRecord:record withEntityName:entityName];		
		}
	}
}

// Returns the tables that contain the receiver's records where each table is a dictionary.
- (NSArray *)tables
{
	return [_representation valueForKey:@"tables"];
}

// Sets the receiver's tables to tables, an array of dictionaries.
- (void)setTables:(NSMutableArray *)tables
{
	[_representation setValue:tables forKey:@"tables"]; // will release the old tables
}

// Returns the changed records for the specified entity. 
- (NSArray *)changedObjectsForEntityName:(NSString *)entityName
{
	// This will be inefficient for large numbers of records. Improve this implementation later.
	NSEnumerator *recordEnumerator = [[self valueForKey:[self keyForEntityName:entityName]] objectEnumerator];
	id record;
	NSMutableArray *changedRecords = [NSMutableArray array];
	
	while (record = [recordEnumerator nextObject]) {
		if ([record valueForKey:@"changed"] == @"YES")
			[changedRecords addObject:record];
	}
	//NSLog(@"changedRecords=%@", [changedRecords description]);
	if ([changedRecords count] == 0)
		return nil;
	return changedRecords;
}

// Returns the deleted records for the specified entity.
- (NSMutableArray *)deletedObjectsForEntityName:(NSString *)entityName
{
	return [[self _tableForEntityName:entityName] valueForKey:@"deleted"];
}

// Returns a printable description of the receiver.
- (NSString *)description
{
	NSString *myDescription = [NSString stringWithFormat:@"[\n%@ = %@ \n%@ = %@\n]", 
		@"entityModel", [_entityModel description], @"_representation", [_representation description]];
	return myDescription;
}

// Returns the record that corresponds to the specified primary key (record identifier) and entity name.
- (id)recordWithPrimaryKey:(NSString *)primaryKey forEntityName:(NSString *)entityName
{
	if ((primaryKey == nil) || (entityName == nil)) return nil;
	id table = [self _tableForEntityName:entityName];
	NSEnumerator *enumerator = [[table valueForKey:@"records"] objectEnumerator];
    id record = [enumerator nextObject];
	
	while ((record != nil) && ![[record valueForKey:@"primaryKey"] isEqual:primaryKey]) {
		record = [enumerator nextObject];
    }
	return record;	
}


// Getting entity model properties

// Returns the receiver's entity names.
- (NSArray *)entityNames
{
	return [_entityModel entityNames];
}

// Returns YES if entityName is in the entity model, NO otherwise.
- (BOOL)isEntityName:(NSString *)name
{
	// Has to be a valid entity name too
	if ([[_entityModel entityNames] containsObject:name])
		return YES;
	else
		return NO;
}

// Returns all the property keys for the specified entity name.
- (NSArray *)propertyKeysForEntityName:(id)entityName
{
	return [_entityModel propertyKeysForEntityName:entityName];
}

// Returns all the attribute keys for the specified entity name.
- (NSArray *)attributeKeysForEntityName:(id)entityName
{
	return [_entityModel attributeKeysForEntityName:entityName];
}

// Returns the relationship keys for the specified entity name.
- (NSArray *)relationshipKeysForEntityName:(id)entityName
{
	return [_entityModel relationshipKeysForEntityName:entityName];
}


// Adding and removing records

// Adds a new record for an entity. Used to programmatically add records when importing or syncing.
// Does some checks and uses mutableArrayValueForKey: to add the records so KVO works.
- (void)addRecord:(id)aRecord forEntityName:(NSString *)entityName
{
	if ((aRecord == nil) || (entityName == nil)) return; // Raise?
	if (![[_entityModel entityNames] containsObject:entityName]) return; // not valid
	[[self mutableArrayValueForKey:[self keyForEntityName:entityName]] addObject:aRecord];
	
	return;
}

// Deletes the record with the specified primaryKey and entityName. Uses mutableArrayValueForKey:
// so that KVO works and this record is added to the deleted records (for fast syncing).
- (BOOL)deleteRecordWithPrimaryKey:(NSString *)primaryKey forEntityName:(NSString *)entityName
{
	if (entityName == nil){
		NSLog(@"FAILED to apply delete, entityName is nil!");
		return NO;
	}
	id records = [self mutableArrayValueForKey:[self keyForEntityName:entityName]];
	id r = [self recordWithPrimaryKey:primaryKey forEntityName:entityName];
	if (r == nil) return NO; // not found
	[records removeObject:r];
	
	return YES;
}


// KVO support (needed for Cocoa Bindings and marking changes)

// Converts a to-many key, of the form <entityName>Records, to an entityName. Returns nil
// if the key is not valid.
- (NSString *)entityNameForKey:(NSString *)key
{
	if (key == nil) return nil;
	NSString *name = [key substringToIndex:[key length] - [RECORDS_SUFFIX length]];
	if (name == nil) return nil;
	name = [name capitalizedString];
	
	Entity *entity = [_entityModel entityWithName:name];
	return [entity entityName];
}

// Converts an entityName to a to-many key, of the form <entityName>Records. Returns nil
// if entityName is not valid (not in the entity model).
- (NSString *)keyForEntityName:(NSString *)entityName
{
	Entity *entity = [_entityModel entityWithEntityName:entityName];
	NSString *key = [NSString stringWithFormat:@"%@%@", [[entity name] lowercaseString], RECORDS_SUFFIX];
	return key;
}

// Override the KVC "undefinedKey" methods so that entity-specific to-many keys, of the form <entityName>Records,
// can be used in Cocoa Bindings.

- (id)valueForUndefinedKey:(NSString *)key
{
	return [self objectsForEntityName:[self entityNameForKey:key]];
}

- (void)setValue:(id)value forUndefinedKey:(NSString *)key
{
	[self _setRecords:value forEntityName:[self entityNameForKey:key]];
}

// Records the changes made to the record arrays.
- (void)observeValueForKeyPath:(NSString *)keyPath ofObject:(id)object change:(NSDictionary *)change context:(void *)context
{
	//NSLog(@"invoking observeValueForKeyPath...");
	NSString *entityName;
	
	// #warning Workaround for observer method change dictionary ValueChangeKind
	// NOTE: This method is currently being invoked without the change dictionary set as expected. The method is invoked
	// repeatedly with the NSKeyValueChangeKindKey equal to NSKeyValueChangeSetting, none of the other types of changes are
	// sent. The NSKeyValueChangeNewKey contains ALL the records, not just the added ones. Hence there is a workaround below
	// that can be removed if this issue is fixed.
	// 04-06-13 Still appears to be a bug in the Tiger seed release.
	
	// A to-many relationship to one of this data source's records changed
	if ((object == self) && ((entityName = [self entityNameForKey:keyPath]) != nil)){
		//NSLog(@"change=%@", [change description]);
		//NSLog(@"newRecords=%@", [[[change valueForKey:NSKeyValueChangeNewKey] valueForKey:@"title"] description]);
		//NSLog(@"oldRecords=%@", [[[change valueForKey:NSKeyValueChangeOldKey] valueForKey:@"title"] description]);
		//NSLog(@"change=%d", [[change valueForKey:NSKeyValueChangeKindKey] intValue]);

		// Work around is to compare the two arrays and figure out if this is a change, remove, or add operation
		int changeKind = [[change valueForKey:NSKeyValueChangeKindKey] intValue];
		id newRecords = [change valueForKey:NSKeyValueChangeNewKey];
		id oldRecords = [change valueForKey:NSKeyValueChangeOldKey];
		NSMutableArray *delta = nil;
		if ([newRecords count] > [oldRecords count]){
			// guess that a record was added
			delta = [NSMutableArray arrayWithArray:newRecords];
			[delta removeObjectsInArray:oldRecords];
			changeKind = NSKeyValueChangeInsertion;
		}
		else if ([newRecords count] < [oldRecords count]){
			// guess that a record was removed
			delta = [NSMutableArray arrayWithArray:oldRecords];
			[delta removeObjectsInArray:newRecords];
			changeKind = NSKeyValueChangeRemoval;
		}		
		else{
			// This is probably a wrong conclusion because a record could have been replaced. In which case,
			// invoking _didAddRecords:forEntityName: on all the old and new records is harmless because it will
			// just mark records that don't have primaryKeys yet (won't harm exisitng records).
			changeKind = NSKeyValueChangeSetting;
		}
		
		if (changeKind ==  NSKeyValueChangeSetting){
			//NSLog(@"change=NSKeyValueChangeSetting");
			[self _didAddRecords:[change valueForKey:NSKeyValueChangeNewKey] forEntityName:entityName];	
		}		
		if (changeKind == NSKeyValueChangeInsertion){
			//NSLog(@"change=NSKeyValueChangeInsertion");
			[self _didAddRecords:delta forEntityName:entityName];	
		}
		if (changeKind  == NSKeyValueChangeRemoval){
			//NSLog(@"change=NSKeyValueChangeRemoval");
			[self _didRemoveRecords:delta forEntityName:entityName];
		}
		if (changeKind == NSKeyValueChangeReplacement){
			//NSLog(@"change=NSKeyValueChangeReplacement");
			[self _didRemoveRecords:[change valueForKey:NSKeyValueChangeOldKey] forEntityName:entityName];
			[self _didAddRecords:[change valueForKey:NSKeyValueChangeNewKey] forEntityName:entityName];					
		}
	}
	
	// A property of a record in a table changed
	else {
		// NOTE: Should probably test if object is an entity object. Too bad I couldn't get the context:
		// argument in the call to addObserver:... to work, otherwise would have passed the entityName 
		// as the context. Could check if the object has a primaryKey attribute?
		//NSLog(@"change=%d", [[change valueForKey:NSKeyValueChangeKindKey] intValue]);
		[object setValue:@"YES" forKey:@"changed"]; // mark as changed
	}
	[self setChanged:YES];
	
	return;
}


// Managing changes

// Returns YES if the receiver changed, NO otherwise.
- (BOOL) isChanged {
	return _dataChanged;
}

// Sets whether or not the receiver changed.
- (void)setChanged:(BOOL)flag
{
	_dataChanged = flag;
	
	// Post a change notification if not batching
	if ((_dataChanged == YES) && (_batching == NO)){
		[self _postChangeNotification];
	}
}

// Clears all recorded changes to the recevier.
- (void) clearAllChanges {
	// If successful, delete the deletedRecords arrays for each entity and reset changed values to nil
	// so you don't try to push the same changes during the next sync. Note that this is done after
	// both pushing changes to the sync engine and getting changes from the sync engine. If the push
	// doesn't succeed, we're probably going to have to do a slow sync next time, but we don't bother
	// clearing anything.
	NSEnumerator *entityEnumerator = [[_entityModel entityNames] objectEnumerator];
	id entityName;
	
	while (entityName = [entityEnumerator nextObject]){	
		[[self deletedObjectsForEntityName:entityName] removeAllObjects];
		
		NSArray *changedRecords = [self changedObjectsForEntityName:entityName];
		unsigned i, count = [changedRecords count];
		for (i = 0; i < count; i++) {
			id record = [changedRecords objectAtIndex:i];
			//NSLog(@"resetting changed attribute for primaryKey=%@", [record valueForKey:@"primaryKey"]);
			[record setValue:nil forKey:@"changed"]; // reset changed attribute
		}
	}
	_dataChanged = NO;
}

// Returns YES, if batching is turned on.
- (BOOL)isBatching
{
	return _batching;
}

// Starts batching. 
- (void)beginBatching
{
	_batching = YES;
}

// Ends batching.
- (void)endBatching
{
	_batching = NO;
	
	// Post change notification at the end of a batch
	if (_dataChanged)
		[self _postChangeNotification];
}

// Posts a change notification.
- (void)_postChangeNotification {
	[(NSNotificationCenter *)[NSNotificationCenter defaultCenter] postNotificationName:DataSourceChangedNotification object:self];
}


// Private table and record methods

// Supports fast syncing (keeping track of changes) and Cocoa Bindings.
// Marks added records as changed, and adds this data source as an observer of each.
- (void)_didAddRecords:(id)newRecords forEntityName:(NSString *)entityName
{
	//NSLog(@"_didAddRecords:forEntityName: entityName=%@ newRecords=%@", entityName, [[newRecords valueForKey:@"title"] description]);
	//NSLog(@"_didAddRecords:forEntityName: entityName=%@ count=%d", entityName, [newRecords count]);

	NSEnumerator *recordEnumerator = [newRecords objectEnumerator];
	id record;	
	
	while (record = [recordEnumerator nextObject]){
		[self _validateRecord:record]; // Sets primary key and marks it as changed for syncing
		[self _observeRecord:record withEntityName:entityName];		
	}
}

- (void)_observeRecord:(id)record withEntityName:(NSString *)entityName
{
	// Add this data source as an observer of every property key for this record
	NSEnumerator *propertyEnumerator = [[_entityModel propertyKeysForEntityName:entityName] objectEnumerator];
	id property;					
	while (property = [propertyEnumerator nextObject]){
		//NSLog(@"adding data source as an observer for keyPath=%@", property);
		[record addObserver:self forKeyPath:property
					options:(NSKeyValueObservingOptionNew | NSKeyValueObservingOptionOld) 
					context:NULL];
	}
}

- (void)_unobserveRecord:(id)record withEntityName:(NSString *)entityName
{
	// Remove this data source as an observer of every property key for this record
	NSEnumerator *propertyEnumerator = [[_entityModel propertyKeysForEntityName:entityName] objectEnumerator];
	id property;					
	while (property = [propertyEnumerator nextObject]){
		[record removeObserver:self forKeyPath:property];
	}
}

// Supports fast syncing (keeping track of changes) and Cocoa Bindings.
// Move removed or replaced records to deleted array and remove self as an observer
- (void)_didRemoveRecords:(id)oldRecords forEntityName:(NSString *)entityName
{
	NSEnumerator *recordEnumerator = [oldRecords objectEnumerator];
	id record;
	
	//NSLog(@"_didRemoveRecords:forEntityName: entityName=%@ oldRecords=%@", entityName, [[oldRecords valueForKey:@"title"] description]);

	while (record = [recordEnumerator nextObject]){
		[[self deletedObjectsForEntityName:entityName] addObject:record];
		[self _unobserveRecord:record withEntityName:entityName];
	}
}

// Creates a new primary key based on a UUID
- (NSString *)_newPrimaryKey
{
	CFUUIDRef uuid = CFUUIDCreate(kCFAllocatorDefault);
	NSString *newKey = (NSString *)CFUUIDCreateString(kCFAllocatorDefault, uuid);
	CFRelease(uuid);
	
	return [newKey autorelease];
}

// Set the primaryKey for the record if it doesn't already have one and marks it as changed 
// in support of fast syncing.
- (void)_validateRecord:(id)record
{
	if ([record valueForKey:@"primaryKey"] == nil){
		[record setValue:[self _newPrimaryKey] forKey:@"primaryKey"];
		// If it doesn't have a primaryKey then it's new and needs to be marked as changed
		[record setValue:@"YES" forKey:@"changed"];
	}
}

// Private Methods

// Returns the table for the specified entity name.
- (id)_tableForEntityName:(NSString *)entityName
{
	if (entityName == nil) return nil;
	NSEnumerator *enumerator = [[self tables] objectEnumerator];
    id table = [enumerator nextObject];
	
	while ((table != nil) && ![[table valueForKey:@"entityName"] isEqual:entityName]) {
		table = [enumerator nextObject];
    }
	return table;
}

// Always return the actual NSMutableArray so that bindings work
- (NSMutableArray *)objectsForEntityName:(NSString *)entityName
{
	if (entityName == nil) return nil;
	id table = [self _tableForEntityName:entityName];
	return [table valueForKey:@"records"];
}

// Sets the records for the specified entity name.
- (void)_setRecords:(NSMutableArray *)newRecords forEntityName:(NSString *)entityName
{
	if (entityName == nil) return;
	id table = [self _tableForEntityName:entityName];
	[table setValue:newRecords forKey:@"records"]; // will release the old records if need be
}

@end
