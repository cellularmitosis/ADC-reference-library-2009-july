/*
 
 File: DataSource.h
 
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

#import <Cocoa/Cocoa.h>

@interface DataSource : NSObject
{
	id _entityModel;
	id _representation;
	BOOL _batching;
	BOOL _dataChanged;
}

// Getting the entity model file name
+ (NSString *)defaultEntityModelFileName;

// Initializing a data source
- (id)initWithModel:(id)anEntityModel;

// Getting and setting data source properties
- (id)entityModel;
- (void)setEntityModel:(id)aModel;
- (NSData *) representation;
- (void)setRepresentation:(NSMutableDictionary *)representation;
- (NSArray *)tables;
- (void)setTables:(NSMutableArray *)someTables;
- (NSArray *)changedObjectsForEntityName:(NSString *)entityName;
- (NSMutableArray *)deletedObjectsForEntityName:(NSString *)entityName;
- (id)recordWithPrimaryKey:(NSString *)primaryKey forEntityName:(NSString *)entityName;
- (NSMutableArray *)objectsForEntityName:(NSString *)entityName;

// Getting entity model properties
- (NSArray *)entityNames;
- (NSArray *)propertyKeysForEntityName:(id)entityName;
- (NSArray *)attributeKeysForEntityName:(id)entityName;
- (NSArray *)relationshipKeysForEntityName:(id)entityName;
- (BOOL)isEntityName:(NSString *)entityName;

// Adding and removing records
- (void)addRecord:(id)aRecord forEntityName:(NSString *)entityName;
- (BOOL)deleteRecordWithPrimaryKey:(NSString *)primaryKey forEntityName:(NSString *)entityName;

// KVC support (needed for Cocoa Bindings)
- (NSString *)entityNameForKey:(NSString *)key;
- (NSString *)keyForEntityName:(NSString *)entityName;

// Managing changes
- (BOOL)isChanged;
- (void)setChanged:(BOOL)flag;
- (void)clearAllChanges;
- (BOOL)isBatching;
- (void)beginBatching;
- (void)endBatching;

// String used as name for notification from Data Source when it wants to sync
#define DataSourceChangedNotification @"DataSourceChangedNotification"

@end
