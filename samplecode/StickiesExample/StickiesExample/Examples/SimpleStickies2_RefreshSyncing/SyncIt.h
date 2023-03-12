/*
 
 File: SyncIt.h
 
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
 
 Copyright © 2005 Apple Computer, Inc., All Rights Reserved
 
 */ 
#import <Foundation/Foundation.h>
#import <SyncServices/SyncServices.h>


@protocol SyncingSource

// Returns an array of schema bundle paths for the schemas used by this client.
- (NSArray *)schemaBundlePaths;

// Returns a unique client identifier to represent the client to the sync engine.
// This method is invoked when creating a sync client before beginning a sync session.
- (NSString *)clientIdentifier;

// Returns an absolute path to the client's description property list. This method is
// invoked when registering the client before beginning a sync session.
- (NSString *)clientDescriptionPath;

// Returns an array of syncable entity names.
- (NSArray *)entityNames;

// Transforms the source objects for entityName into sync records. 
// This method is used when the sync engine requests all records for an entity.
- (NSDictionary *)recordsForEntityName:(NSString *)entityName;

// Returns a dictionary of the changed records for pushing where the keys are the record identifiers and the values are the sync records.
- (NSDictionary *)changedRecordsForEntityName:(NSString *)entityName;

// Returns an array of record identifiers of the deleted objects for pushing. 
- (NSArray *)deletedRecordsForEntityName:(NSString *)entityName;

// Inserts and returns a new source object for the specified record, record identifier and entity name. Returns nil if the
// source object could not be created.
- (BOOL)addObjectFromChange:(ISyncChange *)change forEntityName:(NSString *)entityName;

// Changes and returns a source object for the specified record identifier and entity name. The property values in record are
// applied to the existing source object. Returns nil if the source object could not be found.
- (BOOL)modifyObjectFromChange:(ISyncChange *)change forEntityName:(NSString *)entityName;

// Removes the source object corresponding to the specified record identifier.
- (BOOL)deleteObjectForRecordIdentifier:(NSString *)recordIdentifier;

// Removes all the source objects for the specified entity name.
- (BOOL)deleteAllObjectsForEnityName:(NSString *)entityName;

// Returns YES if the sync source changed, NO otherwise.
- (BOOL)hasChanges;

// Saves the sync source. Returns YES if successful, NO otherwise.
- (BOOL)saveSource;

// Reverts the sync source to the last saved state. Returns YES if successful, NO otherwise.
- (BOOL)revertSource;

// Invoked during a sync session when the client has finished pushing records.
- (void)clientDidFinishPushing:(ISyncClient *)client;

// Invoked during a sync session when the client has finished pulling records.
- (void)clientDidFinishPulling:(ISyncClient *)client;

// Invoked at the end of the sync session. If success is false, then error is set to the reason why.
- (void)client:(ISyncClient *)client didFinishSyncingWithSuccess:(BOOL)success error:(NSError *)error;

@end


typedef enum {
    NoSyncMode,
	FastSyncMode,
	SlowSyncMode,
	RefreshSyncMode
} SyncMode;

@interface SyncIt : NSObject{
	id _syncSource;
	ISyncClient *_syncClient;
	ISyncSession *_syncSession;
	SyncMode _preferredSyncMode;
	SyncMode _syncMode;
    SyncMode _savedSyncMode;
}

// Accessor methods for setting and getting the sync source object.
- (void)setSyncSource:(id)aSource;
- (id)syncSource;
- (void)setPreferredSyncMode:(SyncMode)mode;
- (SyncMode)preferredSyncMode;

// Core syncing methods
- (void)registerSchemas;
- (BOOL)syncIt;
- (BOOL)changeRecordIdentifiers:(NSDictionary*)oldToNew;
-(void)setSyncAlertHandler:(id)handler selector:(SEL)selector;
@end
