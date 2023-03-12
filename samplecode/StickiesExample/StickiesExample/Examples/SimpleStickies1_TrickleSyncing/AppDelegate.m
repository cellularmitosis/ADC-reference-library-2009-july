/*
 
 File: AppDelegate.m
 
 Abstract: Application delegate that implements the saving, loading and
 syncing of user records.
 
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

#import "AppDelegate.h"
#import "EntityModel.h"
#import "DataSource.h"
#import "RecordTransformer.h"

#define kDefaultsTrickleSync @"TrickleSync"

@interface AppDelegate (AppDelegatePrivate)
// Syncing Methods
- (BOOL)_sync;
//- (void)_client:(ISyncClient *)client mightWantToSyncEntityNames:(NSArray *)entityNames;

// Saving and Loading Methods
- (BOOL)_save;
- (NSString *)_applicationSupportFolder;
- (BOOL)_readFromURL:(NSURL *)url;
- (BOOL)_writeToURL:(NSURL *)url;
- (NSData *)_dataRepresentation;
- (BOOL)_loadDataRepresentation:(NSData *)data;
- (id)_loadEntityModelFromFile:(NSString *)fileName;
@end


@implementation AppDelegate

// Initialization and Deallocation

- (id)init {
    self = [super init];
    if (self != nil) {
		// These are set when they are first accessed.
		_myDataSource = nil;
		_entityModel = nil;
		
		// Setup the SyncIt instance.
		_syncIt = [[SyncIt alloc] init];
		[_syncIt setSyncSource:self];
		[_syncIt registerSchemas];
		//[_syncIt setSyncAlertHandler:self selector:@selector(_client:mightWantToSyncEntityNames:)];
		
		// Seup the record transformer.
		_recordTransformer = [[RecordTransformer alloc] init];
		[_recordTransformer setDataSource:[self dataSource]];
		
		// Syncs after launching to pull any changes since last running this app.
		[self _sync];		
					
		// Register for sync engine availability notification.
		[[NSDistributedNotificationCenter defaultCenter] addObserver:self 
															selector:@selector(_handleSyncAvailabilityChangedNotification:) 
																name:ISyncAvailabilityChangedNotification object:@"YES"];
		// Register for data source change notifications.
		[(NSNotificationCenter *)[NSNotificationCenter defaultCenter] addObserver:self 
																		 selector:@selector(_handleSaveRequestNotification:) 
																			 name:DataSourceChangedNotification 
																		   object:[self dataSource]];
	}
    return self;
}

- (void) dealloc
{
	[_syncIt release];
	[_recordTransformer release];
	[_myDataSource release];
	[_entityModel release];
	
	[super dealloc];
}


// SyncingSource Methods

// Returns an array of schema bundle paths.
- (NSArray *)schemaBundlePaths {
	return [NSArray arrayWithObject:
		[[[NSBundle mainBundle] resourcePath] stringByAppendingPathComponent:@"Stickies.syncschema"]];
}

// Returns a unique client identifier.
- (NSString *)clientIdentifier
{
	return @"com.mycompany.SimpleStickies";
}

//Return the path to the client description property list.
- (NSString *)clientDescriptionPath
{
	return [[NSBundle mainBundle] pathForResource:@"ClientDescription" ofType:@"plist"];
}

// Returns the syncable entities.
- (NSArray *)entityNames
{
	return [[self entityModel] entityNames];
}

// Transforms the sync source objects for the given entityName into sync records. 
// Invoked when the sync engine requests all records for an entity.
- (NSDictionary *)recordsForEntityName:(NSString *)entityName
{
	NSArray *objects = [[self dataSource] objectsForEntityName:entityName];
	
	// Transform the objects to records and puts them in a dictionary where the keys are record identifiers.
	NSEnumerator *objectEnumerator = [objects objectEnumerator];
	id anObject;
	NSMutableDictionary *dict = [NSMutableDictionary dictionary];
	
	[_recordTransformer setEntityName:entityName];
	while (anObject = [objectEnumerator nextObject]){
		[dict setObject:[_recordTransformer transformedValue:anObject] forKey:[anObject valueForKey:@"primaryKey"]];
	}
	return dict;
}

// Returns a dictionary of changed records where the keys are the record identifiers. Uses RecordTransformer to
// to transform sync source objects to Sync Service records.
- (NSDictionary *)changedRecordsForEntityName:(NSString *)entityName
{
	NSArray *changedObjects = [[self dataSource] changedObjectsForEntityName:entityName];
	
	// Transform the objects to records and put them in a dictionary where the keys are the record identifiers
	NSEnumerator *objectEnumerator = [changedObjects objectEnumerator];
	id anObject;
	NSMutableDictionary *dict = [NSMutableDictionary dictionary];
	
	[_recordTransformer setEntityName:entityName];
	while (anObject = [objectEnumerator nextObject]){
		[dict setObject:[_recordTransformer transformedValue:anObject] forKey:[anObject valueForKey:@"primaryKey"]];
	}
	return dict;
}

// Returns an array of record identifiers of the deleted objects. Invoked when pushing deletions to the sync engine.
- (NSArray *)deletedRecordsForEntityName:(NSString *)entityName
{
	NSArray *deletedRecords = [[self dataSource] deletedObjectsForEntityName:entityName];
	return [deletedRecords valueForKey:@"primaryKey"];
}

// Inserts a new source object for the specified record, record identifier and entity name. Returns YES if 
// successful, NO otherwise.
- (BOOL)addObjectFromChange:(ISyncChange *)change forEntityName:(NSString *)entityName
{
	return [self modifyObjectFromChange:change forEntityName:entityName];
}

// Applies the change, specified by change, to the corresponding sync object. The property values in 
// [change record] are applied to the existing source object. Returns YES if successful, NO otherwise.
- (BOOL)modifyObjectFromChange:(ISyncChange *)change forEntityName:(NSString *)entityName
{
	// Use the transformer to apply the change to the existing source object, or create a new source
	// object if this is an add.
	[_recordTransformer setEntityName:entityName];
	id sourceObject = [_recordTransformer reverseTransformedValueWithChange:change];
	
	if (sourceObject == nil)
		return NO;
	else
		return YES;
}

// Deletes the record specified by record identifier. Returns YES if successful, NO otherwise.
- (BOOL)deleteObjectForRecordIdentifier:(NSString *)recordIdentifier
{
	id anObject = [self objectForRecordIdentifier:recordIdentifier];
	if (anObject == nil){
		NSLog(@"FAILED to delete record with recordIdentifier=%@", recordIdentifier);
		return NO;
	}
	NSLog(@"Deleting object: %@", anObject);	
	[self deleteRecordForRecordIdentifier:recordIdentifier];
	return YES;	
}

// Deletes all objects for the given entity name. Returns YES if successful, NO otherwise.
- (BOOL)deleteAllObjectsForEnityName:(NSString *)entityName {
	[[[self dataSource] mutableArrayValueForKey:[[self dataSource] keyForEntityName:entityName]] removeAllObjects];
	return YES;	
}

// Returns YES if the dataSource changed, NO otherwise.
- (BOOL)hasChanges
{
	return [[self dataSource] isChanged];
}


// Saving and Loading Methods

// Saves the sync source to disk. Returns YES if successful, NO otherwise.
- (BOOL)saveSource
{
	NSString *applicationSupportFolder = [self _applicationSupportFolder];
	NSURL *url = [NSURL fileURLWithPath:[applicationSupportFolder stringByAppendingPathComponent:[self fileName]]];
	if ([self _writeToURL:url] == NO) {
		NSLog(@"Failed to save source.");
		return NO;
	}
	NSLog(@"Successfully saved file.");
	return YES;
}

// Writes the sync source to the specified fie URL. Returns YES if successful, NO otherwise.
- (BOOL)_writeToURL:(NSURL *)url
{
	NSData *myData = [self _dataRepresentation];
	if (![myData writeToURL:url atomically:YES])
		return NO;
	return YES;
}

// Reads the sync source in from the specified file URL. Returns YES if successful, NO otherwise.
- (BOOL)_readFromURL:(NSURL *)url
{
	NSData *myData = [[NSData alloc] initWithContentsOfURL:url];
	if (myData == nil)
		return NO;
	
	[self _loadDataRepresentation:myData];
	[myData release];
	return YES;
}

// Returns an NSData representation of the receiver for saving to disk. Encodes the data source and the 
// preferred sync mode.
- (NSData *)_dataRepresentation
{
	// Convert the DataSource object dictionary to an NSData for archiving because it can contain
	// unsupported property list types, like instances of NSCalendarDate and custom entities.
	NSDictionary *dict = [NSDictionary dictionaryWithObjectsAndKeys:
		[NSArchiver archivedDataWithRootObject:[_myDataSource representation]], @"data source",
		[NSNumber numberWithInt:[_syncIt preferredSyncMode]], @"sync mode", nil];
	NSData * data = (NSData *)CFPropertyListCreateXMLData(kCFAllocatorDefault, (CFPropertyListRef)dict);
	return [data autorelease];
}

// Decodes the receiver from the given data that contains the data source and preferred sync mode.
// Returns YES if successful, NO otherwise.
- (BOOL)_loadDataRepresentation:(NSData *)data {
	NSString *error;
	NSDictionary *dict = (NSDictionary *)CFPropertyListCreateFromXMLData(kCFAllocatorDefault, (CFDataRef)data, 
																		 kCFPropertyListMutableContainersAndLeaves, 
																		 (CFStringRef *)&error);
	// Converts the NSData back into a DataSource representation that may contain non-property list types.
	[_myDataSource setRepresentation:[NSUnarchiver unarchiveObjectWithData:[dict objectForKey:@"data source"]]];
	int syncMode = [[dict objectForKey:@"sync mode"] intValue];
	[_syncIt setPreferredSyncMode:syncMode];
    return YES;
}


// Reverts the data source to the last saved state. Returns YES if successful, NO otherwise.
- (BOOL)revertSource
{
	NSString *applicationSupportFolder = [self _applicationSupportFolder];
	NSURL *url = [NSURL fileURLWithPath:[applicationSupportFolder stringByAppendingPathComponent:[self fileName]]];
	if ([self _readFromURL:url] == NO) {
		NSLog(@"Failed to revert source.");
		return NO;
	}
	NSLog(@"Reverted to the saved file.");
	
	return YES;
}

// SyncingSource callback method that is invoked during a sync session when the client has finished pushing records.
- (void)clientDidFinishPushing:(ISyncClient *)client
{
	return;
}

// SyncingSource callback method that is invoked during a sync session when the client has finished pulling records.
- (void)clientDidFinishPulling:(ISyncClient *)client
{
	return;
}

// Invoked at the end of the sync session. If success is false, then error is set to the reason why.
// For example, you may implement this method to perform some computations based on any pulled records.
- (void)client:(ISyncClient *)client didFinishSyncingWithSuccess:(BOOL)success error:(NSError *)error
{
	return;
}


//
// Syncing Methods
//

// Saves the receiver's records to disk by invoking saveSource.
- (BOOL)_save {
	return [self saveSource];
}

// Syncs the reciever's records using the SyncIt object.
- (BOOL)_sync
{
	NSLog(@"\n\nBEGIN SYNCING...");
	
	// Start the progress indicator.
	NSProgressIndicator* progressIndicator = [self progressIndicator];
	[progressIndicator setUsesThreadedAnimation:YES];
	[progressIndicator setHidden:NO];
	[progressIndicator startAnimation:self];
	[progressIndicator display];
	
	BOOL saveSuccess = [self _save];
	if (!saveSuccess)
		NSLog (@"Failed to save before syncing.");

	// Sync the receiver's changes.
	BOOL syncSuccess = [_syncIt syncIt];
	
	// Clear the changes so they are not synced the next time.
	if (syncSuccess == YES)
		[[self dataSource] clearAllChanges];
	
	// Stop the progress indicator
	[progressIndicator stopAnimation:self];
	[progressIndicator setHidden:YES];
	
	NSLog(@"\nEND SYNCING\n\n");
	return syncSuccess;
}


- (NSProgressIndicator*) progressIndicator {
	return _progressIndicator;
}

// Saves the receiver's records to disk whenever the user changes data.
- (void) _handleSaveRequestNotification:(NSNotification*)notification {
	[[NSRunLoop currentRunLoop] performSelector:@selector(saveAction:) 
										 target:self argument:self order:1000 modes:[NSArray arrayWithObject:NSDefaultRunLoopMode]];
}
/*
// Joins the current sync session.
- (void)_client:(ISyncClient *)client mightWantToSyncEntityNames:(NSArray *)entityNames
{
    [self syncAction:self];
}
*/

//
// Persistent Data Methods
// 

//Returns the receiver's entity model.
- (id)entityModel
{
    if (_entityModel != nil) 
		return _entityModel;
	NSLog(@"Loading the entity model...");
	_entityModel = [self _loadEntityModelFromFile:[DataSource defaultEntityModelFileName]];
	    
    return _entityModel;
}

// Loads the entity model for the first time. Used by the init method.
- (id)_loadEntityModelFromFile:(NSString *)fileName
{
	id resourcePath = [[NSBundle mainBundle] resourcePath];
	return [[EntityModel alloc] initWithContentsOfFile:[resourcePath stringByAppendingPathComponent:fileName]];
}

// Returns the receiver's data source. Attempts to load it from disk if it doesn't exist.
- (id)dataSource 
{
    if (_myDataSource != nil)
        return _myDataSource;
	
	NSLog(@"Creating data source...");
	_myDataSource = [[DataSource alloc] initWithModel:[self entityModel]];

	// Load the data source content (tables and records) from the backup file, if it exists
	NSString *applicationSupportFolder = [self _applicationSupportFolder];
	NSLog(@"Initializing with data from file %@", [applicationSupportFolder stringByAppendingPathComponent:[self fileName]]);
	NSURL *url = [NSURL fileURLWithPath:[applicationSupportFolder stringByAppendingPathComponent:[self fileName]]];
	if ([self _readFromURL:url] == NO) {
		NSLog(@"No backup file, requesting a refresh sync.");
		//[_syncIt setPreferredSyncMode:RefreshSyncMode];		
	}
	else {
		NSLog(@"Read in some data on init from backup file");
	}
	
    return _myDataSource;
}

// Returns the receiver's application support folder that contains the saved records.
- (NSString *)_applicationSupportFolder {
    NSString *applicationSupportFolder = nil;
    FSRef foundRef;
    OSErr err = FSFindFolder(kUserDomain, kApplicationSupportFolderType, kDontCreateFolder, &foundRef);
    if (err != noErr) {
        NSRunAlertPanel(@"Alert", @"Can't find application support folder", @"Quit", nil, nil);
        [[NSApplication sharedApplication] terminate:self];
    } else {
        unsigned char path[1024];
        FSRefMakePath(&foundRef, path, sizeof(path));
        applicationSupportFolder = [NSString stringWithUTF8String:(char *)path];
        applicationSupportFolder = [applicationSupportFolder stringByAppendingPathComponent:[self folderName]];
    }
    return applicationSupportFolder;
}

// Returns the name of the folder contained in the User's Application Support folder.
- (NSString *)folderName
{
	return @"SyncExamples";
}

// Returns the file name that contains the receiver's records.
- (NSString *)fileName
{
	return @"com.mycompany.SimpleStickies.xml";
}

// Action method that saves the receiver's records to disk.
- (IBAction) saveAction:(id)sender {
	//NSLog(@"invoking saveAction:...");
	(void) [self _save];
}

// Action method that syncs the receiver's records.
- (IBAction) syncAction:(id)sender {
	(void) [self _sync];
}

// NSApplication delegate method that saves the records before terminating the application.
- (NSApplicationTerminateReply)applicationShouldTerminate:(NSApplication *)sender {
    int reply = NSTerminateNow;
    
    if (_myDataSource != nil) {
		[self _save];
    }
    return reply;
}


// Deletion Support

// Returns the receiver's object that matches the specified record identifier. Typically, used to
// get a record that will be deleted.
- (id)objectForRecordIdentifier:(NSString *)recordIdentifier
{
	NSEnumerator *entityEnumerator = [[self entityNames] objectEnumerator];
	NSString *entityName;
	id anObject = nil;
	
	entityName = [entityEnumerator nextObject];
	while ((anObject == nil) && (entityName != nil)){
		anObject = [[self dataSource] recordWithPrimaryKey:recordIdentifier forEntityName:entityName];
		entityName = [entityEnumerator nextObject];
	}
	return anObject;
}

// Deletes the record corresponding to the specified record identifier. Typically, invoked when pulling
// deletions from the sync engine.
- (BOOL)deleteRecordForRecordIdentifier:(NSString *)recordIdentifier
{
	NSEnumerator *entityEnumerator = [[self entityNames] objectEnumerator];
	NSString *entityName;
	BOOL success = NO;
	
	entityName = [entityEnumerator nextObject];
	while ((success == NO) && (entityName != nil)){
		success = [[self dataSource] deleteRecordWithPrimaryKey:recordIdentifier forEntityName:entityName];
		entityName = [entityEnumerator nextObject];
	}
	return success;
}

@end

