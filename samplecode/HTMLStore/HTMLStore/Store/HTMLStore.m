/*

File: HTMLStore.m

Abstract: The basic implementation of the store. Just the methods required
 by NSAtomicStore and NSObjectStore. Most the methods here don't do too much 
 work, but they illustrate where to start when implementing your own atomic
 store.

 Disclaimer: IMPORTANT:  This Apple software is supplied to you by 
 Apple Inc. ("Apple") in consideration of your agreement to the
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
 Neither the name, trademarks, service marks or logos of Apple Inc. 
 may be used to endorse or promote products derived from the Apple
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
 
 Copyright (C) 2007 Apple Inc. All Rights Reserved.
 
*/

#import "HTMLStore_Internal.h"
#import <Foundation/NSXMLDocument.h>

@implementation HTMLStore

- (id)initWithPersistentStoreCoordinator:(NSPersistentStoreCoordinator *)coordinator configurationName:(NSString *)configurationName URL:(NSURL *)url options:(NSDictionary *)options {
    self = [super initWithPersistentStoreCoordinator:coordinator configurationName:configurationName URL:url options:options];
    if (self != nil) {
        pIdentifier = nil;
        pRefDataToCacheNodeMap = nil;
	
	NSInteger options = NSXMLNodePreserveCharacterReferences | NSXMLNodePreserveWhitespace;
	
	NSError *error = nil;	
	pDocument = [[NSXMLDocument alloc] initWithContentsOfURL:url options:options error:&error];
        
	if ((pDocument == nil) && ([[error domain] isEqualToString:NSURLErrorDomain]) && ([url isFileURL])) {
	    // maybe the file just doesn't exist, create it

	    NSInteger code = [error code];
	    if ((code == NSURLErrorCannotOpenFile) || (code == NSURLErrorZeroByteResource)) {
		[[NSFileManager defaultManager] createFileAtPath:[url path] contents:nil attributes:nil];
		pDocument = [[NSXMLDocument alloc] initWithKind:NSXMLDocumentXHTMLKind options:options];
	    }
	}
	
	if (pDocument != nil) {
	    [self loadMetadata];	    
	} else {
	    [[NSException exceptionWithName:NSInvalidArgumentException reason:[NSString stringWithFormat:@"Could not open URL %@", url] userInfo:nil] raise];
	}
    }
    return self;
}


- (void)dealloc {
    [pDocument release];
    [pRefDataToCacheNodeMap release];
    [pIdentifier release];
    [super dealloc];
}


- (NSString *)identifier {
    if (pIdentifier == nil) {
	uuid_t uuid;
	uuid_generate(&uuid);
	pIdentifier = [[NSString stringWithFormat:@"%08X-%04X-%04X-%04X-%012X", uuid, uuid, uuid, uuid, uuid] retain];
    }
    return pIdentifier;
}


- (void)setIdentifier:(NSString *)identifier {
    if (pIdentifier != identifier) {
        [pIdentifier release];
        pIdentifier = [identifier retain];
    }
}


- (NSString *)type {
    return @"HTMLStore";
}


- (BOOL)load:(NSError **)error {
    if (pDocument != nil) {
	[self headElement]; // if this is a new document, ensures that the head element is created before the body element
	NSXMLElement *body = [self bodyElement];
	NSArray *tables = [body elementsForName:@"table"];
	
	for (NSXMLElement *table in tables) {
	    [self loadTable:table];
	}
    }
    
    return (pDocument != nil);
}


- (BOOL)save:(NSError **)error {
    BOOL result = NO;
    [self updateMetadata];
    
    NSString *xmlString = [[pDocument rootElement] XMLString];
    NSData *data = [NSData dataWithBytes:[xmlString cStringUsingEncoding:NSUTF8StringEncoding] length:[xmlString lengthOfBytesUsingEncoding:NSUTF8StringEncoding]];
    if (data != nil) {
	result = [data writeToURL:[self URL] atomically:YES];
    }
    return result;
}

// Called by PSC during save to create cache nodes for newly inserted objects
- (NSAtomicStoreCacheNode *)newCacheNodeForManagedObject:(NSManagedObject *)managedObject {
    NSManagedObjectID *oid = [managedObject objectID];
    id refData = [self referenceObjectForObjectID:oid];

    // create the cache node
    id item = [[NSAtomicStoreCacheNode alloc] initWithObjectID:oid];
    
    // create a new row in the table
    NSXMLElement *table = [self tableForEntity:[managedObject entity]];
    NSXMLElement *newRow = [NSXMLElement elementWithName:@"tr"];
    
    // give the table row an id
    NSXMLNode *rowIDAttribute = [NSXMLNode attributeWithName:@"id" stringValue:refData];
    [newRow addAttribute:rowIDAttribute];
    
    // for each table header column, add a table data element
    NSInteger index, columnCount = [[table childAtIndex:0] childCount];
    for (index=0; index < columnCount; index++) {
	[newRow addChild:[NSXMLElement elementWithName:@"td"]];
    }	
    
    // finally add the new row
    [table addChild:newRow];
    
    // update the cache node's data
    [self updateCacheNode:item fromManagedObject:managedObject];
    
    return item;
}

    
// Called during save by the PSC to update the values of changed objects. 
// Also called during save by self to set the values of newly insert objects
- (void)updateCacheNode:(NSAtomicStoreCacheNode *)node fromManagedObject:(NSManagedObject *)managedObject {    
    id refData = [self referenceObjectForObjectID:[managedObject objectID]];    
    NSXMLElement *row = [self rowWithRefData:refData inTable:nil];
    if (row != nil) {
	NSArray *headerNodes = [[[((NSXMLElement *)[row parent]) elementsForName:@"tr"] objectAtIndex:0] elementsForName:@"th"];
	NSArray *columnNames = [headerNodes valueForKey:@"stringValue"];
	
	[self updateAttributesInCacheNode:node andRow:row fromManagedObject:managedObject usingColumnNames:columnNames];
	
	[self updateRelationshipsInCacheNode:node andRow:row fromManagedObject:managedObject usingColumnNames:columnNames];
    }
}



// called when the PSC generates object IDs for our cachenodes
// This method MUST return a new unique primary key reference data for an instance of entity. This 
// primary key value MUST be an id
- (id)newReferenceObjectForManagedObject:(NSManagedObject *)managedObject {

    NSEntityDescription *entity = [managedObject entity];
    id refData = nil;
        
    NSXMLElement *table = [self tableForEntity:entity];

    NSXMLElement *newRow = [self newRowForEntity:entity inTable:table];
    refData = [[[newRow attributeForName:@"id"] stringValue] copy]; // refData is the value of the id attribute
    [newRow release];
    
    return refData;
}


- (void)willRemoveCacheNodes:(NSSet *)cacheNodes {
    for (NSAtomicStoreCacheNode *node in cacheNodes) {
	NSManagedObjectID *objectID = [node objectID];
	NSEntityDescription *entity = [objectID entity];
	NSXMLElement *table = [self tableForEntity:entity];
	id rowToDelete = [self rowWithRefData:[self referenceObjectForObjectID:objectID] inTable:table];			
	[table removeChildAtIndex:[rowToDelete index]];
    }
}


// Gives the store a chance to do any non-dealloc teardown (for example, closing a network connection) 
// before removal.  Default implementation just does nothing.
- (void)willRemoveFromPersistentStoreCoordinator:(NSPersistentStoreCoordinator *)coordinator {
    [pDocument release];
    pDocument = nil;
    [super willRemoveFromPersistentStoreCoordinator:coordinator];
}


@end
