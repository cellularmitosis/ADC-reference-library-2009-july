/*
 
 File: SyncIt.m
 
 Abstract: Encapsulates core syncing methods so they can be reused by 
 typical applications that want to sync records.
 
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
 
 Copyright � 2005 Apple Computer, Inc., All Rights Reserved
 
 */ 
#import "SyncIt.h"
#import <Foundation/NSKeyValueObserving.h>

@interface SyncIt (SyncItPrivate)

- (void)_registerSchemas;
- (ISyncClient *)_syncClient;
- (void)_setFilters:(NSArray *)filters;
- (NSDictionary *)_negotiateChangesToPush;
- (BOOL)_pushChanges:(NSDictionary *)entitiesToPush;
- (NSArray *)_negotiateEntityNamesToPull;
- (BOOL)_pullChangesForEntityNames:(NSArray *)filteredEntityNames;
@end

@implementation SyncIt

// Initializataion and deallocation methods

- (id)init{
	self = [super init];
	if (self != nil){
		// Sync source is set by the application.
		_syncSource = nil;
		
		// These are set the first time they are used.
		_syncClient = nil;
		_syncSession = nil;
		
		// Fast sync is the default even though sync engine will force a slow sync the first time
		// the client syncs.
		_preferredSyncMode = FastSyncMode;
		_syncMode = FastSyncMode;
        // Should be able to get the type programmatically and remove this constant
        _savedSyncMode = NoSyncMode;
	}
	return self;
}

- (void)dealloc {
	[_syncSource release];
    [_syncClient release];
    [super dealloc];
}

// Accessor Methods

- (void)setSyncSource:(id)aSource{
    [_syncSource release];
    _syncSource = [aSource retain];
    return;
}

- (id)syncSource{
    return _syncSource;
}


- (SyncMode)preferredSyncMode
{
	return _preferredSyncMode;
}

- (void)setPreferredSyncMode:(SyncMode)mode
{
	_preferredSyncMode = mode;
	return;

}

// Sets the filters for this client.
- (void)_setFilters:(NSArray *)filters
{
	// Will create a sync client if it doesn't exist yet
	[[self _syncClient] setFilters:filters];
}


// Core Syncing Methods

-(void)setSyncAlertHandler:(id)handler selector:(SEL)selector
{
    ISyncClient* client = [self _syncClient];
    if (client == nil) {
        NSLog(@"Warning: Can't get sync client to register alert handler");
        return;
    }
    [client setShouldSynchronize:YES withClientsOfType:ISyncClientTypeApplication];
    [client setShouldSynchronize:YES withClientsOfType:ISyncClientTypeServer];
    [client setSyncAlertHandler:handler selector:selector];
}

- (void)registerSchemas
{
	NS_DURING
		NSEnumerator *pathEnumerator = [[_syncSource schemaBundlePaths] objectEnumerator];
		NSString *bundlePath = [pathEnumerator nextObject];
		while (bundlePath != nil){
			[[ISyncManager sharedManager] registerSchemaWithBundlePath:bundlePath];
			bundlePath = [pathEnumerator nextObject];
		}		
	NS_HANDLER
		NSLog(@"Exception occured trying to register schema:\n%@", localException);
	NS_ENDHANDLER	
}

// Returns a sync client for this document.
- (ISyncClient *)_syncClient {
	// Create the client the first time this method is invoked.
	if (_syncClient == nil) {
		// Try to get a registered client using the client identifier.
		NSLog(@"Getting the sync client...");
		ISyncClient *client = [[ISyncManager sharedManager] clientWithIdentifier:[_syncSource clientIdentifier]];
		
		// Otherwise, register the client for the first time.
		if (client == nil){
			NSLog(@"Registering the client for the first time...");
			client = [[ISyncManager sharedManager] registerClientWithIdentifier:[_syncSource clientIdentifier] 
															descriptionFilePath:[_syncSource clientDescriptionPath]];
			// #warning Workaround for client image, remove when fixed
			NSString *imageFile = [[client imagePath] lastPathComponent];
			[client setImagePath:[[NSBundle mainBundle] pathForResource:[imageFile stringByDeletingPathExtension]
																 ofType:[imageFile pathExtension]]];
		}
		
		// Raise if creating a client was unsuccessful.
		if (client == nil) {
			[NSException raise:[_syncSource clientIdentifier] format:@"can't register client."];
		}
		// Otherwise, retain the client you created for the next time this method is invoked.
		else {
			[_syncClient release];
			_syncClient = [client retain];
		}
	}			
	
	return _syncClient;
}

- (void)_setSyncSession:(ISyncSession *)session
{
	[_syncSession release];
	_syncSession = [session retain];
}

- (BOOL)changeRecordIdentifiers:(NSDictionary*)oldToNew {
    ISyncSession* session = _syncSession;
    // If we don't already have a session, create one (which we have to finish/release below)
    if (session == nil) {
        session = [ISyncSession beginSessionWithClient:_syncClient entityNames:[_syncSource entityNames] beforeDate:[NSDate dateWithTimeIntervalSinceNow:3600]];
        if (session == nil) {
            NSLog(@"Timed out trying to create a Sync Session... WARNING: can't change record identifiers.");
            return NO;
        }
    }
	
    // Remap the record ids
    [session clientChangedRecordIdentifiers:oldToNew];
	
    // If we don't have a session, we created one (above), so finish/release it.
    if (_syncSession == nil) {
		// Finishing
        [session finishSyncing];
        [session release];
    }
    return YES;
}

// The primary sync method that registers the client, creates a session, and syncs the doc. 
// Use the syncType argument to request a sync mode. Pass AnySyncType if you want to let the 
// sync engine decide to fast or slow sync the document.
- (BOOL) syncIt {
	// Register the client.
	(void) [self _syncClient];
	
	// Attempt to use the preferred request
	_syncMode = _preferredSyncMode;
	_preferredSyncMode = SlowSyncMode; // Force slow sync on next sync unless we succeed below
	BOOL syncSuccess = NO;
	
	NS_DURING
	// Beginning a Sync Session...
	NSLog(@"Beginning a sync session...");
	[self _setSyncSession:[ISyncSession beginSessionWithClient:_syncClient entityNames:[_syncSource entityNames] 
													beforeDate:[NSDate dateWithTimeIntervalSinceNow:360]]];
	if (_syncSession == nil) {
		NSLog(@"Timed out trying to create a Sync Session");
		return NO;
	}
	else {
		// Negotiating
		
		// Refresh Syncing
		if (_syncMode == RefreshSyncMode) {
			NSLog(@"Requesting refresh sync from engine for entity names %@", [_syncSource entityNames]);
			[_syncSession clientDidResetEntityNames:[_syncSource entityNames]];
		}
		NSDictionary *changes = [self _negotiateChangesToPush];
		
		// Pushing, mingling and pulling
		NS_DURING
			if ([self _pushChanges:changes] == YES){
				NSArray *filteredEntityNames = [self _negotiateEntityNamesToPull];

				// Mingling
				// Prepare the sync engine for pulling records--enters the session mingling state.
				NSLog(@"MINGLING, invoking prepareToPullChangesForEntityNames: with filteredEntityNames=%@", [filteredEntityNames description]);
				if ([_syncSession prepareToPullChangesForEntityNames:filteredEntityNames beforeDate:[NSDate dateWithTimeIntervalSinceNow:3600]]) {
					if ([self _pullChangesForEntityNames:filteredEntityNames] == YES)
						syncSuccess = YES;
				}
			}
			
		NS_HANDLER
			NSLog(@"Exception syncing: %@, reason %@", [localException name], [localException reason]);
		NS_ENDHANDLER		
		
		// If successful, commit the accepted changes
        // NOTE: If successful, you need to save the pulled changes. If not successful, revert to the saved document.
        // If successful, the didPullChanges: callback is invoked after the second save so that the record identifiers
        // can be changed to those used by Core Data, and clients may perform any additional operations on the pulled changes.
        //  
        // Also note that notifications are sent immediately, so care must be taken so that these callbacks don't cause
        // an infinite loop. Hence didPullChanges: sets the notification selector back to didSaveContext: (subclasses
        // that override this method MUST call super first).
        
		// Finishing
		if (syncSuccess && ([_syncSession isCancelled] == NO)) {
            if ([_syncSource hasChanges] == YES) {
                syncSuccess = [_syncSource saveSource];
            }
            if (syncSuccess) {
                NSLog(@"Committing accepted changes...");
                [_syncSession clientCommittedAcceptedChanges]; // Commit all accepted changes
            } else {
                NSLog(@"Client failed saving changes after sync!");
            }
            _preferredSyncMode = FastSyncMode; // Fast sync next time
			[_syncSession finishSyncing];
			NSLog(@"Ended sync session.");
		}
		else {
			NSLog(@"Error occurred syncing client, not saving or committing changes. Reverting file save...");
            [_syncSource revertSource];
			if ([_syncSession isCancelled] == NO){
				[_syncSession finishSyncing];
				NSLog(@"Ended sync session.");
			}
		}
	}
	NS_HANDLER
		NSLog(@"Exception occured while syncing: %@", localException);
	NS_ENDHANDLER

	[_syncSource client:_syncClient didFinishSyncingWithSuccess:syncSuccess error:nil];
	[self _setSyncSession:nil]; // Can't reuse sync sessions after they are canceled or finished.

	return syncSuccess;
}

// Returns changes from the syncing source to push to the sync engine. Keys are record ids, values are records.
- (NSDictionary *)_negotiateChangesToPush
{
	id entityName;
	NSEnumerator *entityEnumerator;
    NSMutableDictionary *entitiesToPush = [NSMutableDictionary dictionary];
	
	// Checks if the sync engine wants to do a fast sync and applies only the changed records, otherwise slow syncs.
	// Also handles a refresh sync mode if specified to replace all client records.
	
	NSLog(@"NEGOTIATING entities to push to the sync engine...");
	entityEnumerator = [[_syncSource entityNames] objectEnumerator];
	while (entityName = [entityEnumerator nextObject]){
		NSDictionary *records = nil;
		BOOL serverWantsAllRecordsForEntityName = [_syncSession shouldPushAllRecordsForEntityName:entityName];
		// Get the records that have changed since the last sync (or all of the records, if slow syncing). Clears
		// the cache if this is a refresh sync mode.

		// Push Changes?		
		if (![_syncSession shouldPushChangesForEntityName:entityName]){
			NSLog(@"Server does not want us to push changes for entityName %@...", entityName);
		}		
		// Push All Records?
		else if ((_syncMode == SlowSyncMode) || serverWantsAllRecordsForEntityName) {
			// Slow Syncing
			NSLog(@"Slow syncing entityName=%@...", entityName);
			if (serverWantsAllRecordsForEntityName == NO) {
				// Let server know that WE want to push all records (slow sync)
				NSLog(@"Letting the server know our honorable intention to push all records for entityName %@...", entityName);
				[_syncSession clientWantsToPushAllRecordsForEntityNames:[NSArray arrayWithObject:entityName]];
			}
			records = [_syncSource recordsForEntityName:entityName];
			[entitiesToPush setValue:records forKey:entityName]; // replaces the changed records with ALL the records
		}
		
		// Otherwise, push changes only (fast sync)
		else {
			NSLog(@"Fast syncing entityName=%@...", entityName);
			records = [_syncSource changedRecordsForEntityName:entityName]; // fast sync the cached changes
			// If nothing changed, but some records deleted, make sure we won't skip below
			if ((records == nil) && ([_syncSource deletedRecordsForEntityName:entityName] != nil)) records = [NSDictionary dictionary]; // placeholder
			if (records != nil) [entitiesToPush setValue:records forKey:entityName];
		}		
	}
	return entitiesToPush;
}

- (BOOL)_pushChanges:(NSDictionary *)entitiesToPush
{
	// Pushing Records
	// Now we actually push changes for any records that are changed and that the engine allows us to push
	NSLog(@"PUSHING changes to sync engine for entities %@", [entitiesToPush allKeys]);
	id entityName;
	NSEnumerator *entityEnumerator;
	entityEnumerator = [entitiesToPush keyEnumerator];
	while (entityName = [entityEnumerator nextObject]) { 
		NSDictionary *pushRecords = [entitiesToPush valueForKey:entityName];
		id recordEnumerator = [pushRecords keyEnumerator];
		
		// Push the whole record that changed. (Alternatively, you can push just the properties that changed 
		// but then you need to record them.)
		id recordIdentifier;
		while (recordIdentifier = [recordEnumerator nextObject]) {
			id record = [pushRecords objectForKey:recordIdentifier];
			NSLog(@"pushing sync record %@:\n%@", recordIdentifier, [record description]);
			[_syncSession pushChangesFromRecord:record withIdentifier:recordIdentifier];
		}
	}

	// Pushing Delete Changes
	// Now push deletions that the engine allows us to push and if fast syncing only.
	NSLog(@"PUSHING deletions to sync engine for entities %@", [entitiesToPush allKeys]);
	entityEnumerator = [entitiesToPush keyEnumerator];
	while (entityName = [entityEnumerator nextObject]) { 
		if ([_syncSession shouldPushAllRecordsForEntityName:entityName] == NO) {
			NSArray *records = [_syncSource deletedRecordsForEntityName:entityName];
			
			if (records) {
				NSLog(@"Deleting records for entity: %@", entityName);			
				NSEnumerator *recordEnumerator = [records objectEnumerator];
				NSString *recordIdentifier;
				
				while (recordIdentifier = [recordEnumerator nextObject]) {
					NSLog(@"    Deleting record with record identifier: %@", recordIdentifier);
					[_syncSession deleteRecordWithIdentifier:recordIdentifier];
				}
			}
		}
	}
	// Notifiy the client.
	[_syncSource clientDidFinishPushing:_syncClient];	
	return YES;
}

- (NSArray *)_negotiateEntityNamesToPull {
	// Filter the entity names to pull based on what the sync engine thinks should be pulled
	NSLog(@"NEGOTIATING what entities to pull...");
    id entityName;
	NSMutableArray *filteredEntityNames = [NSMutableArray array];
	NSEnumerator *entityEnumerator = [[_syncSource entityNames] objectEnumerator];
	// Pull Changes?
	while (entityName = [entityEnumerator nextObject]){
		if ([_syncSession shouldPullChangesForEntityName:entityName])
			[filteredEntityNames addObject:entityName];
	}
	return filteredEntityNames;			
}				

// Pulls changes from session and applies them to the document.
- (BOOL)_pullChangesForEntityNames:(NSArray *)filteredEntityNames
{
	NSLog(@"PULLING changes from the sync engine...");
	NSEnumerator *entityEnumerator;
	id entityName;
	
	// For each entity, check to see if there are changes to pull.
	entityEnumerator = [filteredEntityNames objectEnumerator];
	while (entityName = [entityEnumerator nextObject]){
		// Replace All Records?
		// Should you replace all records? For example, if pulling the truth or refresh syncing.
		if ([_syncSession shouldReplaceAllRecordsOnClientForEntityName:entityName]) {
			[_syncSource deleteAllObjectsForEnityName:entityName];
		}
		
		// Now apply all the pulled changes for this entity.
		NSEnumerator *enumerator = [_syncSession changeEnumeratorForEntityNames:[NSArray arrayWithObject:entityName]];
		ISyncChange *change; 
		while (change = [enumerator nextObject]) {
			BOOL success = NO;
			NSString *recordIdentifier = [change recordIdentifier];
			switch ([change type])
			{
				// Add and modfies are treated the same in this example.
				case ISyncChangeTypeAdd:
					success = [_syncSource addObjectFromChange:change forEntityName:entityName];
					if (success == YES) {
						// Committing Changes
						[_syncSession clientAcceptedChangesForRecordWithIdentifier:recordIdentifier formattedRecord:nil newRecordIdentifier:nil];
					} else
						NSLog(@"FAILED to add recordIdentifier=%@", recordIdentifier);
					break;
				case ISyncChangeTypeModify:
					success = [_syncSource modifyObjectFromChange:change forEntityName:entityName];
					if (success == YES) {
						// Committing Changes
						[_syncSession clientAcceptedChangesForRecordWithIdentifier:recordIdentifier formattedRecord:nil newRecordIdentifier:nil];
					} else
						NSLog(@"FAILED to modify recordIdentifier=%@", recordIdentifier);
					break;
				case ISyncChangeTypeDelete:
					// Delete the record from the local data source.
					NSLog(@"Pulling deleted recordIdentifier=%@", recordIdentifier);
					success = [_syncSource deleteObjectForRecordIdentifier:recordIdentifier];
					if (success == YES)
						// Committing Changes
						[_syncSession clientAcceptedChangesForRecordWithIdentifier:recordIdentifier formattedRecord:nil 
														  newRecordIdentifier:nil];
					else
						NSLog(@"FAILED to delete recordIdentifier=%@", recordIdentifier);
					
					break;
			}
		}
	}
	[_syncSource clientDidFinishPulling:_syncClient];

	return YES;
}

@end
