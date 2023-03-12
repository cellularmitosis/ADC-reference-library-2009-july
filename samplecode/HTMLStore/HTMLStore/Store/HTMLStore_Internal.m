/*
 
 File: HTMLStore_Internal.m
 
 Abstract: The core implementation of the store. Just the methods that are
 called from the HTMLStore AtomicStore API live here. The methods here 
 do most of the work required implement HTML support for the store. 
 
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

#import <openssl/bio.h>
#import <openssl/evp.h>


@implementation HTMLStore (HTMLStore_Internal)

// gets the indexes of <objects> in <array>, the indexset is in terms of <array> ordering
// the number of indexes may be smaller than the number of objects in array
// we use this to get the property names from an entity in the same order they appear in in the html
- (NSIndexSet *)indexesOfObjects:(NSArray *)objects inArray:(NSArray *)array {
    NSMutableIndexSet *indexSet = [NSMutableIndexSet indexSet];
    for (id object in objects) {
        NSUInteger index = [array indexOfObject:object];
        if (index != NSNotFound) {
            [indexSet addIndex:index];
        }
    }
    return indexSet;
}


// Get, or create and remember, a cache node
- (id)cacheNodeForEntity:(NSEntityDescription *)entity withReferenceData:(id)refData {
    if (pRefDataToCacheNodeMap == nil) {
        pRefDataToCacheNodeMap = [[NSMutableDictionary alloc] init];
    }
    
    id item = [pRefDataToCacheNodeMap objectForKey:refData];
    if (item == nil) {
        NSManagedObjectID *oid = [self objectIDForEntity:entity referenceObject:refData];    
        item = [[NSAtomicStoreCacheNode alloc] initWithObjectID:oid];
        [pRefDataToCacheNodeMap setObject:item forKey:refData];
    }
    return item;
}

// Get HTML table row element corresponding to refData in table. If table == nil, search the whole document. 
// Returns exactly 1 node or nil
- (NSXMLElement *)rowWithRefData:(id)refData inTable:(NSXMLElement *)contextNode {
    NSString *baseString = @"tr[@id=\"%@\"]";
    if (contextNode == nil) {
	baseString = @"/html/body/table/tr[@id=\"%@\"]";
	contextNode = [pDocument rootElement];
    }
    NSString *xpath = [NSString stringWithFormat:baseString, refData];
    NSArray *nodes = [contextNode nodesForXPath:xpath error:nil];
    return ((nodes != nil) && ([nodes count] == 1)) ? [nodes objectAtIndex:0] : nil;
}

// returns the nodes of the relationship filled out by columnData. Will be a single cache node if relationship is to-one and a set of cache nodes if relatinoship is to-many
- (id)cacheNodesForRelationshipData:(NSXMLElement *)columnData andRelationship:(NSRelationshipDescription *)relationship  fromTable:(NSXMLElement *)table {
    id results = [NSMutableSet set];
    
    // column data will be several link elements
    //<A href="#section1" /A>
    NSArray *links = [columnData elementsForName:@"a"];
    
    if ([relationship isToMany] == NO) {
        NSAssert1(([links count] <= 1), @"More than one destination object described for to-one relationship '%@'", [relationship name]);
    }
    
    // for each link, find the node and see if it's been processed 
    for (NSXMLElement *link in links) {
        NSString *destinationID = [[link attributeForName:@"href"] objectValue];
        NSAssert(((destinationID != nil) && ([destinationID length] > 1)), @"destinationID wasn't set for relationship");
        destinationID = [destinationID substringFromIndex:1]; // drop the # at the beginning of each link href
	
        // if one was created brand new, we'll end up with a cache node that's mostly empty, when its table is processed later, then the values will be filled out
        [results addObject:[self cacheNodeForEntity:[relationship destinationEntity] withReferenceData:destinationID]];
    }
    
    if ([relationship isToMany] == NO) {
        results = [results anyObject];
    }
    
    return results;
}

- (NSDateFormatter *)dateFormatter {
    static NSDateFormatter *dateFormatter = nil;
    if (dateFormatter == nil) {
	dateFormatter = [[NSDateFormatter alloc] init];
    }
    return dateFormatter;
}


- (void)loadTable:(NSXMLElement *)table {
    NSString *entityName = [[table attributeForName:@"class"] stringValue];
    NSPersistentStoreCoordinator *coordinator = [self persistentStoreCoordinator];
    NSEntityDescription *entity = [[[coordinator managedObjectModel] entitiesByName] valueForKeyPath:entityName];
    if (entity != nil) {
        
        NSDictionary *attributes = [entity attributesByName];
        NSDictionary *relationships = [entity relationshipsByName];
        
        // get the available column names
        NSArray *allRows = [table elementsForName:@"tr"];
        NSArray *columnNames = [[[allRows objectAtIndex:0] elementsForName:@"th"] valueForKey:@"stringValue"];
        
        // get the attribute and relationship names
        NSArray *attributeNames = [attributes allKeys];
        NSArray *relationshipNames = [relationships allKeys];
        
        // convert everything to be in terms of the column ordering and column names
        
        // get the indexes of columns with attribute data
        NSIndexSet *attributeIndexes = [self indexesOfObjects:attributeNames inArray:columnNames];
	
        // get the indexes of columns with relationship data
        NSIndexSet *relationshipIndexes = [self indexesOfObjects:relationshipNames inArray:columnNames];
        
        
        NSMutableSet *results = [NSMutableSet set];
        NSUInteger index;
        
        NSUInteger lastIndex = 1;
        
        
        for (NSXMLElement *row in allRows) {
            NSArray *rowData = [row elementsForName:@"td"];
            if ([rowData count] > 0) { // the first row will not have any rowdata - it's a th row
		
                // get the row id 
                NSXMLNode *rowID = [row attributeForName:@"id"];
                id refData = [rowID stringValue];
                
                // if there wasn't an id, make one up. 
                while (refData == nil) {
                    refData = [NSString stringWithFormat:@"%@_%@", entityName, [NSNumber numberWithInt:lastIndex++]];
		    id row = [self rowWithRefData:refData inTable:table];
                    if (row != nil) {
                        refData = nil;
                    } else {
                        [rowID setObjectValue:refData];
                    }
                }
		
                // create the cache node to register
                id item = [self cacheNodeForEntity:entity withReferenceData:refData];                
		
                // set attribute values
                for (index=[attributeIndexes firstIndex]; index != NSNotFound; index=[attributeIndexes indexGreaterThanIndex:index]) {                
                    NSXMLElement *columnData = [rowData objectAtIndex:index];
                    id objectValue = [columnData objectValue];
                    NSString *attributeName = [columnNames objectAtIndex:index];
                    NSAttributeDescription *attribute = [attributes objectForKey:attributeName];
		    NSAttributeType attributeType = [attribute attributeType];
		    
                    if (attributeType == NSDecimalAttributeType) {
                        objectValue = [NSDecimalNumber decimalNumberWithString:objectValue];
                    } else if (attributeType == NSDoubleAttributeType) {
			objectValue = [NSNumber numberWithDouble:[objectValue doubleValue]];
                    } else if (attributeType == NSFloatAttributeType) {
			objectValue = [NSNumber numberWithFloat:[objectValue floatValue]];
                    } else if (attributeType == NSBooleanAttributeType) {
			objectValue = [NSNumber numberWithBool:[objectValue intValue]];
                    } else if (attributeType == NSDateAttributeType) {
			objectValue = [NSCalendarDate dateWithNaturalLanguageString:objectValue];
                    } else if (attributeType == NSBinaryDataAttributeType) {
			
			// if this element has children, maybe it's an <a href> instead of inline junk
			NSXMLElement *content = nil;
			if ([columnData childCount] == 1) {
			    content = (NSXMLElement *)[columnData childAtIndex:0];
			}
			if ((content != nil) && ([[content name] isEqualToString:@"a"])) {
			    NSString *hrefValue = [[content attributeForName:@"href"] objectValue];
			    if (hrefValue != nil) {
				NSURL *refURL = [NSURL URLWithString:hrefValue];
				objectValue = [NSData dataWithContentsOfURL:refURL];
			    } 				
			} else {
			    // we have to do this in order to decode the base64 value of the data attribute into an NSData. Using openSSL is the only "free" way we have to decode base64
			    objectValue = [[content attributeForName:@"data"] objectValue];
			    BIO *base64Bio = BIO_new(BIO_f_base64());
			    BIO_set_flags(base64Bio, BIO_FLAGS_BASE64_NO_NL);
			    BIO *memBio = BIO_new_mem_buf((void *)[objectValue cStringUsingEncoding:NSUTF8StringEncoding],[objectValue lengthOfBytesUsingEncoding:NSUTF8StringEncoding]);
			    memBio = BIO_push(base64Bio, memBio);
			    NSMutableData *dataValue = [[NSMutableData alloc] init];
			    char buffer[4096];
			    int bytesRead = 0;
			    while ((bytesRead = BIO_read(memBio,&buffer,4096)) > 0) {
				[dataValue appendBytes:&buffer length:bytesRead];
			    }
			    BIO_free_all(memBio);
			    objectValue = [NSData dataWithData:dataValue];
			    [dataValue release];
			}
                    } else if ((attributeType == NSInteger16AttributeType) || (attributeType == NSInteger32AttributeType) || (attributeType == NSInteger64AttributeType)) {
			objectValue = [NSNumber numberWithInteger:[objectValue integerValue]];
		    }
                    
                    [item setValue:objectValue forKey:attributeName];
                }
                
                // set relationships values
                for (index=[relationshipIndexes firstIndex]; index != NSNotFound; index=[relationshipIndexes indexGreaterThanIndex:index]) {                
                    NSXMLElement *columnData = [rowData objectAtIndex:index];
                    id destinationCacheNodes = [self cacheNodesForRelationshipData:columnData andRelationship:[relationships objectForKey:[columnNames objectAtIndex:index]]  fromTable:table];
                    [item setValue:destinationCacheNodes forKey:[columnNames objectAtIndex:index]];
                }
		
                [results addObject:item];
                [item release];
            }
        }
        
        [self addCacheNodes:results];        
    }
}

// loads metadata from document HTML meta tags and call [self setMetadata] to make the metadata available to Core Data
- (void)loadMetadata {
    NSXMLElement *headElement = [self headElement];
    NSMutableDictionary *metadata = [NSMutableDictionary dictionary];
    
    for (NSXMLElement *metaElement in [headElement children]) {
	NSString *metaName = [[metaElement attributeForName:@"name"] stringValue];
	
	if (metaName != nil) {
	    // we'll always try to do propertyList processing since that's what we'll usually want
	    id metaContent = [[metaElement attributeForName:@"content"] objectValue];
	    @try {
		metaContent = [metaContent propertyList];
	    } @catch (NSException *parseException) {
		// that's fine, we'll treat the content as a plain string then
	    }
	    [metadata setObject:metaContent forKey:metaName];	    
	}
    }
    [self setMetadata:metadata];
}

// updates the NSXMLDocument's HTML meta tags
- (void)updateMetadata {
    NSXMLElement *headElement = [self headElement];
    
    // attempt to preserve non-meta lements in the head element - only delete meta elements 
    NSUInteger index, count = [headElement childCount];
    for (index=count; index > 0; index--) {
	NSXMLElement *element = (NSXMLElement *)[headElement childAtIndex:(index-1)];
	if ([[element name] isEqualToString:@"meta"]) {
	    [headElement removeChildAtIndex:(index-1)];
	}
    }
    
    NSDictionary *metadata = [self metadata];    
    for (NSString *name in [metadata allKeys]) {
	NSXMLElement *metaElement = [[NSXMLElement alloc] initWithName:@"meta"];
	
	NSXMLNode *attribute = [NSXMLNode attributeWithName:@"name" stringValue:name];
	[metaElement addAttribute:attribute];
	
	attribute = [NSXMLNode attributeWithName:@"content" stringValue:[[metadata objectForKey:name] description]];
	//	[attribute setObjectValue:[metadata objectForKey:name]]; // use setObject value to ensure we get escaping
	[metaElement addAttribute:attribute];
	
	[headElement addChild:metaElement];
	[metaElement release];
    }
}

- (void)updateAttributesInCacheNode:(NSAtomicStoreCacheNode *)node andRow:(NSXMLElement *)row fromManagedObject:(NSManagedObject *)managedObject usingColumnNames:(NSArray *)columnNames {
    NSArray *attributeKeys = [[[managedObject entity] attributesByName] allKeys];
    NSIndexSet *indexSet = [self indexesOfObjects:attributeKeys inArray:columnNames];
    NSUInteger index;
    
    // attributes
    for (index=[indexSet firstIndex]; index != NSNotFound; index=[indexSet indexGreaterThanIndex:index]) {
        NSString *name = [columnNames objectAtIndex:index];
        
        NSXMLElement *columnData = (NSXMLElement *)[row childAtIndex:index];
	NSXMLElement *content = nil;
	NSAttributeType attributeType = [[[[managedObject entity] attributesByName] objectForKey:name] attributeType];
	id valueToStore = [managedObject valueForKey:name];
	if (attributeType == NSFloatAttributeType) {
	    content = [NSXMLElement elementWithName:@"pre" stringValue:[valueToStore stringValue]];
	} else if (attributeType == NSBinaryDataAttributeType) {
	    BOOL writeBytes = YES;
	    if ([columnData childCount] == 1) {
		content = (NSXMLElement *)[columnData childAtIndex:0];
	    }
	    
	    // if the original content was a link, don't just overwrite it. Try to write data back to the url first.
	    if ((content != nil) && ([[content name] isEqualToString:@"a"])) {
		NSString *hrefValue = [[content attributeForName:@"href"] objectValue];
		if (hrefValue != nil) {
		    NSURL *url = [NSURL URLWithString:hrefValue];
		    if ([valueToStore writeToURL:url atomically:YES] == YES) {
			writeBytes = NO;
		    }
		}
	    }
	    if (writeBytes == YES) {
		// setting up the node this way encodes the NSData as bsae64 in data attribute
		content = [NSXMLElement elementWithName:@"object"];
		NSXMLNode *dataAttribute = [NSXMLNode attributeWithName:@"data" stringValue:nil];
		[dataAttribute setObjectValue:valueToStore];
		[content addAttribute:dataAttribute];
		[content setStringValue:@"binary object value"];
	    }
	} else {
	    content = [NSXMLElement elementWithName:@"pre"];
	    [content setObjectValue:valueToStore];
	}
	[columnData setChildren:[NSArray arrayWithObject:content]];
    }
    
    [node setValuesForKeysWithDictionary:[managedObject valuesForKeys:attributeKeys]];
}

- (void)updateRelationshipsInCacheNode:(NSAtomicStoreCacheNode *)node andRow:(NSXMLElement *)row fromManagedObject:(NSManagedObject *)managedObject usingColumnNames:(NSArray *)columnNames {
    NSDictionary *relationshipsByName = [[managedObject entity] relationshipsByName];
    NSIndexSet *indexSet = [self indexesOfObjects:[relationshipsByName allKeys] inArray:columnNames];
    NSUInteger index;
    
    // relationships
    for (index=[indexSet firstIndex]; index != NSNotFound; index=[indexSet indexGreaterThanIndex:index]) {
        NSString *name = [columnNames objectAtIndex:index];
	
        NSXMLElement *columnData = (NSXMLElement *)[row childAtIndex:index];
	[columnData setChildren:nil];
	id relatedObjects = [managedObject valueForKey:name];
	NSMutableSet *relatedCacheNodes = [[NSMutableSet alloc] init];
	
	if (([[relationshipsByName objectForKey:name] isToMany] == NO) && (relatedObjects != nil)) {
	    //if this is a to-One relationship, relatedObjects will be a managedObject, not a set
	    relatedObjects = [NSSet setWithObject:relatedObjects];
	}
	
	// <a href="#Entity2_4">Entity2_4</a>
	for (NSManagedObject *destinationObject in relatedObjects ) {
	    id refData = [self referenceObjectForObjectID:[destinationObject objectID]];
	    
	    id destinationNode = [self cacheNodeForEntity:[destinationObject entity] withReferenceData:refData];
	    [relatedCacheNodes addObject:destinationNode];
	    
	    NSXMLElement *linkNode = [NSXMLElement elementWithName:@"a"];
	    NSString *hrefString = [NSString stringWithFormat:@"#%@", refData];
	    NSXMLElement *hrefAttribute = [NSXMLElement attributeWithName:@"href" stringValue:hrefString];
	    [linkNode addAttribute:hrefAttribute];
	    [linkNode setObjectValue:hrefString];
	    [columnData addChild:linkNode];
	}
	
	[node setValue:relatedCacheNodes forKey:name];
	[relatedCacheNodes release];
    }
}    

// creates a new row based on entity and the peer rows that already exist in table. callers have to add it to the table.
- (NSXMLElement *)newRowForEntity:(NSEntityDescription *)entity inTable:(NSXMLElement *)table {
    NSXMLElement *row = [[NSXMLElement alloc] initWithName:@"tr"];
    
    // id = @"entityname_uniquenumber"
    NSString *idString = [entity name];
    
    // does the table have a unique last number attribute?
    NSXMLNode *nextIDAttribute = [table attributeForName:@"nextid"];
    NSString *nextIDValue = [nextIDAttribute stringValue];
    NSInteger nextID = 0;
    if (nextIDValue != nil) {
        nextID = [nextIDValue integerValue];
	nextIDValue = [NSString stringWithFormat:@"%@_%ld", idString, nextID];
    } else {
        // make one up 
	NSXMLElement *lastRow = [[table elementsForName:@"tr"] lastObject];
	nextID = [[[lastRow attributeForName:@"id"] stringValue] integerValue];
	
	while (YES) {
	    nextIDValue = [NSString stringWithFormat:@"%@_%ld", idString, nextID];
	    id row = [self rowWithRefData:nextIDValue inTable:table];
	    if ((row == nil) || (nextID == NSNotFound)) {
		break;
	    }
	    nextID++;
        } 
    }
    
    NSAssert((nextID != NSNotFound), @"blew past some limit trying to find an unused id for a new row");
    
    // nextIDValue is our new id
    nextID++;
    if (nextIDAttribute == nil) {
        nextIDAttribute = [NSXMLNode attributeWithName:@"nextid" stringValue:nil];
        [table addAttribute:nextIDAttribute];
    }
    [nextIDAttribute setObjectValue:[NSString stringWithFormat:@"%ld", nextID]];
    
    NSXMLNode *rowIDAttribute = [row attributeForName:@"id"];
    if (rowIDAttribute == nil) {
        rowIDAttribute = [NSXMLNode attributeWithName:@"id" stringValue:nil];
        [row addAttribute:rowIDAttribute];
    }
    [rowIDAttribute setObjectValue:nextIDValue];
    
    // add the table datas
    NSUInteger index, headerCount = [[[table elementsForName:@"tr"] objectAtIndex:0] childCount];    
    for (index=0; index < headerCount; index++) {
        [row addChild:[NSXMLElement elementWithName:@"td"]];
    }
    
    return row;
}

- (NSXMLElement *)htmlEnvelope {
    NSXMLElement *htmlEnvelope = [pDocument rootElement];
    
    if (htmlEnvelope == nil) {
	htmlEnvelope = [NSXMLElement elementWithName:@"html"];
	NSXMLNode *namespaceAttribute = [NSXMLNode attributeWithName:@"xmlns" stringValue:@"http://www.w3.org/1999/xhtml"];
	[htmlEnvelope addAttribute:namespaceAttribute];
	[pDocument setRootElement:htmlEnvelope];
    }
    
    return htmlEnvelope;
}

- (NSXMLElement *)rootElementWithName:(NSString *)name {
    NSXMLElement *htmlEnvelope = [self htmlEnvelope];
    NSXMLElement *result = [[htmlEnvelope elementsForName:name]  lastObject];    
    
    if (result == nil) {
	result = [NSXMLElement elementWithName:name];
	[htmlEnvelope addChild:result];
    }
    
    return result;
}

- (NSXMLElement *)headElement {
    return [self rootElementWithName:@"head"];
}

- (NSXMLElement *)bodyElement {
    return [self rootElementWithName:@"body"];
}

- (NSXMLElement *)tableForEntity:(NSEntityDescription *)entity {
    
    // find the table to insert the new object into    
    NSString *entityName = [entity name];
    NSString *xpath = [NSString stringWithFormat:@"/html/body/table[@class=\"%@\"]", entityName];
    NSArray *nodes = [[pDocument rootElement] nodesForXPath:xpath error:nil];
    NSAssert1(([nodes count] <= 1), @"multiple tables for entity '%@' found", entityName);
    NSXMLElement *table = nil;
    
    if ([nodes count] == 1) {
	table = [nodes objectAtIndex:0];
    } else {
	table = [NSXMLElement elementWithName:@"table"];
	NSXMLElement *tableHeaderRow = [NSXMLElement elementWithName:@"tr"];
	for (NSPropertyDescription *property in [entity properties]) {
	    if ([property isTransient] == NO) {
		[tableHeaderRow addChild:[NSXMLElement elementWithName:@"th" stringValue:[property name]]];
	    }
	}
	
	NSXMLElement *tableClassAttribute = [NSXMLNode attributeWithName:@"class" stringValue:[entity name]];
	[table addAttribute:tableClassAttribute];
	[table addChild:tableHeaderRow];
	
	NSXMLElement *body = [self bodyElement];
	[body addChild:table];	
    }
    return table;
}


@end
