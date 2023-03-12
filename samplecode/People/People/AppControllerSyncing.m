/*

File: AppControllerSyncing.m

Abstract: Part of the People project demonstrating use of the
              SyncServices framework

Version: 0.1

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

#import <SyncServices/SyncServices.h>

#import "AppControllerExtensions.h"
#import "AppControllerSyncing.h"
#import "Change.h"
#import "Constants.h"
#import "LastNameFilter.h"
#import "NSArrayExtras.h"

@implementation AppController (Syncing)

//
// ===========================================
// Syncing
//

//
// This function encapsulates the control flow for a sync
// session. It is the callback that has been registered
// in the ISyncSession call to 
// beginSessionInBackgroundWithClient:entityNames:target:selector:.
// As documented, it takes an ISyncClient and an ISyncSession
// as its two arguments. If the session is nil, then an
// ISyncSession could not be created, and the function should
// take no action. Also note that this function includes an
// exception handler. This serves as a "top-level" handler
// to catch any exceptions that are raised by the sync process.
//
- (void)performSync:(ISyncClient *)syncClient :(ISyncSession *)syncSession
{
    @try {
        // Sync session could be nil. If it is, you need to clean up and
        // try to create a session again.
        if (syncSession) {
            [self negotiateSession:syncSession];
            [self pushDataForSession:syncSession];
            [self pullDataForSession:syncSession];
        }
    }
    @catch (NSException *exception) {
        NSLog(@"caught exception: %@: %@", [exception name], [exception reason]);
    }
    @finally {
        [self syncCleanup];
    }
}

//
// This method is the sync alert handler registered by the ISyncClient call to
// setSyncAlertHandler:self selector:. It calls sync:, and is therefore
// equivalent in its behavior to the user pressing the "Sync" button in the
// user interface.
//
- (void)client:(ISyncClient *)syncClient willSyncEntityNames:(NSArray *)entityNames
{
    [self sync:self];
}
    
//
// This method registers our schema and client with the sync engine. 
// In the case where the client is not already registered, a client 
// description for the client is also supplied, which tells the sync 
// engine about our client, the schema elements it syncs with, and 
// (optionally) about any sync tool we supply.
//
- (ISyncClient *)performRegistrations 
{
    ISyncManager *syncManager = [ISyncManager sharedManager];
    ISyncClient *syncClient;
    
    // Since we are using one of the Sync Services public schemas in this
    // application, it is not necessary to register it.
    // The contacts schema is guaranteed to be registered automatically.
    // If you are not using one of the public schemas, now is the time
    // to register it.
    //  
    // [syncManager registerSchemaWithBundlePath:CanonicalContactsSchemaPath];
    
    // See if our sync client has already registered...
    if (!(syncClient = [syncManager clientWithIdentifier:ClientIdentifier])) {
        // ...and if it hasn't, register it.
        syncClient = [syncManager registerClientWithIdentifier:ClientIdentifier descriptionFilePath:
            [[NSBundle mainBundle] pathForResource:@"ClientDescription" ofType:@"plist"]];
    }
    
    return syncClient;
}

//
// This method helps to negotiate a sync mode for the session. It takes it cues
// from the user interface in this example, but in the case of a "real" app,
// setting the sync mode would probably take some input from application state.
// For instance, if an application has lost part of its data set, it will likely
// wish to perform a refresh sync.
// It is also important to note that this is a "negotiation" process. You ask
// for the mode you want here, but the kind of session you get is really up
// to the sync engine. Later, in the push and pull phases, we will find out
// what the sync engine has decided.
//
- (void)negotiateSession:(ISyncSession *)syncSession 
{
    switch (m_syncMode) {
        case FastSync:
            // nothing to do here.
            break;
        case SlowSync:
            [syncSession clientWantsToPushAllRecordsForEntityNames:m_entityNames];
            break;
        case RefreshSync:
            [syncSession clientDidResetEntityNames:m_entityNames];
            break;
        case PullTheTruth:
            // not handled here. must be handled before syncSession starts.
            break;
    }
}

//
// The push sync phase. 
//
- (void)pushDataForSession:(ISyncSession *)syncSession
{
    if ([syncSession shouldPushAllRecordsForEntityName:EntityContact]) {
        // Slow sync. Push all records we have. Any record not pushed
        // in this step that the sync engine has in its store will
        // be construed as a delete. Ask for a refresh sync when 
        // negotiating a sync mode if you do not wish to have records
        // you do not push to be considered deletes. 
        NSEnumerator *enumerator = [m_appRecordsForSession objectEnumerator];
        NSDictionary *appRecord;
        while ((appRecord = [enumerator nextObject])) {
            NSDictionary *syncRecord = [self convertAppRecordToSyncRecord:appRecord];
            NSString *identifier = [appRecord objectForKey:IdentifierKey];
            [syncSession pushChangesFromRecord:syncRecord withIdentifier:identifier];
        }
    }
    else if ([syncSession shouldPushChangesForEntityName:EntityContact]) {
        // Fast sync. Fast syncing requires that you to present the only
        // changes since your last sync. Your code must be able to track
        // such changes over time if you wish to fast sync. Note how
        // this is accomplished in the AppController class using the 
        // Change object. Conveniently, such Changes objects are also used
        // by this application to implement undo/redo.
        NSEnumerator *enumerator = [m_changesSinceLastSync objectEnumerator];
        Change *change;
        while ((change = [enumerator nextObject])) {
            switch ([change type]) {
                case AddRecord:
                case ModifyRecord: {
                    NSDictionary *appRecord = [change record];
                    NSString *identifier = [appRecord objectForKey:IdentifierKey];
                    // Note the conversion from an application record to a sync record.
                    // Adds and modifies are treated the same: We just push
                    // the whole record. This seems OK since our records
                    // are small. If our records were larger, it might make sense
                    // to push modifications using the pushChange: method on ISyncSession.
                    NSDictionary *syncRecord = [self convertAppRecordToSyncRecord:appRecord];
                    [syncSession pushChangesFromRecord:syncRecord withIdentifier:identifier];
                    break;
                }
                case DeleteRecord:
                    // No record conversion needed in the case of a delete. However, fast
                    // sync requires that we save the identifiers for deleted records so
                    // we can tell the sync engine about it here.
                    [syncSession deleteRecordWithIdentifier:[[change oldRecord] objectForKey:IdentifierKey]];
                    break;
            }
        }
    }
}

//
// The pull sync phase. 
//
- (void)pullDataForSession:(ISyncSession *)syncSession
{
    // Figure out what to pull. If you are pulling more than one entity type,
    // ask for them individually. You cannot make any assumptions about one
    // entity type based on the result of shouldPullChangesForEntityName: for
    // another entity type.
    BOOL shouldPull = [syncSession shouldPullChangesForEntityName:EntityContact];
    if (!shouldPull) {
        [self syncCleanup];
    }

    // Determine if we need to replace all data from server. This will return
    // YES if pulling the truth. However, it is important to note that you
    // should not throw away your data upon receiving a YES response from this
    // call. You should wait until a later time, at the very least, you should
    // wait until after you call and receive a YES response from
    // prepareToPullChangesForEntityNames:beforeDate:. The prepareXXX function
    // can return NO or may not return in the time you are willing to wait.
    // If you delete your data here and then do not pull, you will wind up
    // with no data in your code.
    // Again, if you are pulling more than one entity type, ask for them 
    // individually. you cannot make any assumptions about one entity
    // type based on the result of shouldReplaceAllRecordsOnClientForEntityName: 
    // for another entity type.
    if ([syncSession shouldReplaceAllRecordsOnClientForEntityName:EntityContact]) {
        m_syncReplaceAllRecords = YES;
    }

    // Ask the sync engine if it is ready to pull changes. This code is willing
    // to wait an indefinite amount of time for this method to return. You may
    // wish to have a shorter time out and call this code in a loop. 
    // Particularly if you are calling this code from the main thread, you may
    // wish to provide feedback to the user while waiting for a YES response.
	if (![syncSession prepareToPullChangesForEntityNames:m_entityNames beforeDate:[NSDate distantFuture]]) {
        [self syncFailed:syncSession error:nil];
        return;
    }	

    // Now that prepareToPullChangesForEntityNames:beforeDate: has been called, it is
    // OK to delete data here when pulling the truth. Optionally, you could wait until
    // all the new records are pulled before deleting your local store, but you should
    // wait at least this long.
    if (m_syncReplaceAllRecords)
        [m_appRecordsForSession removeAllObjects];

    // Now do the actual pulling.
    NSEnumerator *changeEnumerator = [syncSession changeEnumeratorForEntityNames:m_entityNames];
    ISyncChange *change;
    while ((change = [changeEnumerator nextObject])) {
        NSString *identifier = [change recordIdentifier];
        [m_pulledIdentifiers addObject:identifier];
        switch ([change type]) {
            case ISyncChangeTypeAdd: {
                NSDictionary *syncRecord = [change record];
                // Note the conversion of sync record to application record.
                NSDictionary *appRecord = [self convertSyncRecordToAppRecord:syncRecord withIdentifier:identifier];
                [m_appRecordsForSession addObject:appRecord];
                break;
            }
            case ISyncChangeTypeModify: {
                NSDictionary *syncRecord = [change record];
                // Note the conversion of sync record to application record.
                NSDictionary *appRecord = [self convertSyncRecordToAppRecord:syncRecord withIdentifier:identifier];
                int index = [m_appRecordsForSession indexOfSyncRecordWithIdentifier:identifier];
                if ([m_appRecordsForSession count] > index) {
                    [m_appRecordsForSession replaceObjectAtIndex:index withObject:appRecord];
                }
                break;
            }
            case ISyncChangeTypeDelete: {
                unsigned index = [m_appRecordsForSession indexOfSyncRecordWithIdentifier:identifier];
                if ([m_appRecordsForSession count] > index)
                    // No record conversion needed in the case of a delete.
                    [m_appRecordsForSession removeObjectAtIndex:index];
                break;
            }
        }
    }

    // Note how records were not accepted at the time they were pulled. Separating the
    // pull operation from the accept operation gives you the opportunity to run
    // this code inside a critical section. Why might you want to do that?
    // If you are running the sync operation in a background thread, where the main
    // thread might still be accepting changes from the user, you can lock out the UI
    // here while you check the records you have pulled against the records that have
    // been modified since you started syncing. If you have such records, do not
    // accept them. By not accepting them, the sync engine will give them to you 
    // again the next time you sync, and the sync framework conflict resolver will
    // run to automatically handle conflicts.
    // This is also the place where we tell the sync engine about any record formatting
    // we have done.
    // This acts as the first phase of the two-phase commit.
    NSString *identifier;
    NSEnumerator *enumerator = [m_pulledIdentifiers objectEnumerator];
    while ((identifier = [enumerator nextObject])) {
        [syncSession clientAcceptedChangesForRecordWithIdentifier:identifier 
            formattedRecord:[m_formattedRecords objectForKey:identifier]
            newRecordIdentifier:nil];
    }
    
    // Second phase of two-phase commit.
    [syncSession clientCommittedAcceptedChanges];
	[syncSession finishSyncing];

    // Update our local record store with the records modified by the sync operation.
    [m_appRecords removeAllObjects];
    [m_appRecords addObjectsFromArray:m_appRecordsForSession];
}

//
// Catch all failure handler. This would likely communicate the failure to the
// user in some kind, gentle (and hopefully informative) way.
//
- (void)syncFailed:(ISyncSession *)syncSession error:(NSError *)error
{
    [syncSession cancelSyncing];
    NSLog(@"sync failed: %@", [error localizedFailureReason]);
    [self syncCleanup];
}

//
// All sync sessions, whether they succeed or fail, come through this code.
// This handles updating the UI, and releases some objects used in the sync
// session.
//
- (void)syncCleanup
{
    [m_syncProgress stopAnimation:self];
    [m_syncProgress setHidden:YES];
    [m_syncButton setEnabled:YES];
    [m_syncModeButton setEnabled:YES];
    
    [m_appRecordsForSession release];
    [m_changesSinceLastSync release];
    [m_pulledIdentifiers release];
    [m_formattedRecords release];

    [self sortNamesAndDisplay];
    [self update];
    [self writeDataFile];
}

//
// ===========================================
// Record conversion
//

//
// This method converts an application record, like the ones used to
// represent data internally in application-specific code, into the
// form the sync engine uses. The mapping is trivially simple in 
// this example, but real code will likely need to do more serious 
// work here.
//
- (NSDictionary *)convertAppRecordToSyncRecord:(NSDictionary *)record
{
    NSString *firstName = [record objectForKey:FirstNameKey];
    NSString *middleName = [record objectForKey:MiddleNameKey];
    NSString *lastName = [record objectForKey:LastNameKey];
    NSString *company = [record objectForKey:CompanyNameKey];
    NSMutableDictionary *syncRecord = [NSMutableDictionary dictionaryWithObjectsAndKeys:
        EntityContact, ISyncRecordEntityNameKey,
        nil];
    if (firstName && ([firstName isEqualToString:@""] == NO)) {
        [syncRecord setObject:firstName forKey:FirstNameKey];
    }
    if (middleName && ([middleName isEqualToString:@""] == NO)) {
        [syncRecord setObject:middleName forKey:MiddleNameKey];
    }
    if (lastName && ([lastName isEqualToString:@""] == NO)) {
        [syncRecord setObject:lastName forKey:LastNameKey];
    }
    if (company && ([company isEqualToString:@""] == NO)) {
        [syncRecord setObject:company forKey:CompanyNameKey];
    }
    return syncRecord;
}

//
// This method converts a sync record, like the ones pulled from
// the sync engine, into the form used by the application. The mapping
// is trivially simple in this example, but real code will likely need
// to do more serious work here.
//
- (NSDictionary *)convertSyncRecordToAppRecord:(NSDictionary *)record withIdentifier:(NSString *)identifier
{
    NSString *firstName = [record objectForKey:FirstNameKey];
    NSString *middleName = [record objectForKey:MiddleNameKey];
    NSString *lastName = [record objectForKey:LastNameKey];
    NSString *company = [record objectForKey:CompanyNameKey];
    if (m_syncsUsingRecordFormatting) {
        firstName = [firstName length] > FormatLimit ? [firstName substringToIndex:FormatLimit] : firstName;
        middleName = [middleName length] > FormatLimit ? [middleName substringToIndex:FormatLimit] : middleName;
        lastName = [lastName length] > FormatLimit ? [lastName substringToIndex:FormatLimit] : lastName;
        company = [company length] > FormatLimit ? [company substringToIndex:FormatLimit] : company;
    }
    NSDictionary *result = [NSDictionary dictionaryWithObjectsAndKeys:
        identifier, IdentifierKey,
        firstName ? firstName : @"", FirstNameKey,
        middleName ? middleName : @"", MiddleNameKey,
        lastName ? lastName : @"", LastNameKey,
        company ? company : @"", CompanyNameKey,
        nil];
    if (m_syncsUsingRecordFormatting) {
        [m_formattedRecords setObject:[self convertAppRecordToSyncRecord:result] forKey:identifier];
    }
    return result;
}

//
// ===========================================
// IBActions
//

//
// Handles the changes to sync options as shown in the "Options" menu in 
// the application.
//
- (IBAction)syncOptionsChanged:(id)sender
{
    int value = [sender tag];
    switch (value) {
        case UsesRecordFiltering:
            m_syncsUsingRecordFiltering = !m_syncsUsingRecordFiltering;
            break;
        case UsesRecordFormatting:
            m_syncsUsingRecordFormatting = !m_syncsUsingRecordFormatting;
            break;
        case UsesSyncAlertHandler:
            m_syncsUsingSyncAlertHandler = !m_syncsUsingSyncAlertHandler;
            BOOL flag = m_syncsUsingSyncAlertHandler;

            // These few lines of code register a sync alert handler method.
            // When our application is running, it will be asked through the
            // handler we supply if it wishes to join a sync session that is
            // beginning.
            ISyncClient *syncClient = [self performRegistrations];
            [syncClient setShouldSynchronize:flag withClientsOfType:ISyncClientTypeApplication];
            [syncClient setShouldSynchronize:flag withClientsOfType:ISyncClientTypeDevice];
            [syncClient setShouldSynchronize:flag withClientsOfType:ISyncClientTypeServer];
            [syncClient setShouldSynchronize:flag withClientsOfType:ISyncClientTypePeer];
            [syncClient setSyncAlertHandler:self selector:@selector(client:willSyncEntityNames:)];
            break;
        case SyncsOnAppDeactivate:
            m_syncsOnAppDeactivate = !m_syncsOnAppDeactivate;
            NSNotificationCenter *defaultCenter = [NSNotificationCenter defaultCenter];
            if (m_syncsOnAppDeactivate) {
                [defaultCenter addObserver:self 
                    selector:@selector(sync:) 
                    name:NSApplicationWillResignActiveNotification 
                    object:nil];
            }
            else {
                [defaultCenter removeObserver:self 
                    name:NSApplicationWillResignActiveNotification 
                    object:nil];
            }
            break;
    }
}

//
// This method starts a sync session. It updates the UI, creates some
// session-specific objects used to track the activity of the session,
// and starts the session running.
//
- (IBAction)sync:(id)sender
{
    @try {
        [m_syncButton setEnabled:NO];
        [m_syncModeButton setEnabled:NO];
        [m_syncProgress setHidden:NO];
        [m_syncProgress startAnimation:self];
        [[m_window undoManager] removeAllActions];

        m_appRecordsForSession = [m_appRecords mutableCopy];
        m_changesSinceLastSync = [m_changes copy];
        [m_changes removeAllObjects];
        m_pulledIdentifiers = [[NSMutableArray alloc] init];
        m_formattedRecords = [[NSMutableDictionary alloc] init];
        m_syncReplaceAllRecords = NO;
        m_syncMode = [m_syncModeButton indexOfSelectedItem];
    
        ISyncClient *syncClient = [self performRegistrations];
        if (!syncClient) {
            NSLog(@"cannot create sync syncClient.");
            return;
        }

        // If you are going to be doing record filtering, set the filters
        // on the client before starting the session.
        if (m_syncsUsingRecordFiltering) {
            id filter = [LastNameFilter filter];
            [syncClient setFilters:[NSArray arrayWithObject:filter]]; 
        }
        else {
            [syncClient setFilters:[NSArray array]]; 
        }
    
        // If you are pulling the truth, tell the client this fact
        // before starting the session.
        if (m_syncMode == PullTheTruth) {
            [syncClient setShouldReplaceClientRecords:YES forEntityNames:m_entityNames];
        }
    
        // Ask for a session to be started in the background. This is a good choice
        // if starting the session from the main thread, since other clients in 
        // other processes may be joining, and you will not want to block your UI
        // while that handshaking is taking place.
        [ISyncSession beginSessionInBackgroundWithClient:syncClient entityNames:m_entityNames 
            target:self selector:@selector(performSync::)];
    }
    @catch (NSException *exception) {
        NSLog(@"caught exception: %@: %@", [exception name], [exception reason]);
        [self syncCleanup];
    }
}

@end
