//
// File:		MyDocument.m
//
// Abstract:	The NSDocument subclass for the CDMetadataBrowser's coredata document.
//				Overrides the NSDocument methods involving cut-copy-pasting, document
//				reading, writing . Responds also as a delegate to the enclosed table-view.
//
// Version:		1.0
//				Originally introducted at WWDC 2008 at Session:
//				Extending and Integrating Post-Production Applications with Final Cut Pro
//
// Disclaimer: IMPORTANT:  This Apple software is supplied to you by Apple Inc. ("Apple")
//             in consideration of your agreement to the following terms, and your use,
//             installation, modification or redistribution of this Apple software
//             constitutes acceptance of these terms.  If you do not agree with these
//             terms, please do not use, install, modify or redistribute this Apple
//             software.
//
//             In consideration of your agreement to abide by the following terms, and
//             subject to these terms, Apple grants you a personal, non - exclusive
//             license, under Apple's copyrights in this original Apple software ( the
//             "Apple Software" ), to use, reproduce, modify and redistribute the Apple
//             Software, with or without modifications, in source and / or binary forms;
//             provided that if you redistribute the Apple Software in its entirety and
//             without modifications, you must retain this notice and the following text
//             and disclaimers in all such redistributions of the Apple Software. Neither
//             the name, trademarks, service marks or logos of Apple Inc. may be used to
//             endorse or promote products derived from the Apple Software without specific
//             prior written permission from Apple.  Except as expressly stated in this
//             notice, no other rights or licenses, express or implied, are granted by
//             Apple herein, including but not limited to any patent rights that may be
//             infringed by your derivative works or by other works in which the Apple
//             Software may be incorporated.
//
//             The Apple Software is provided by Apple on an "AS IS" basis.  APPLE MAKES NO
//             WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED
//             WARRANTIES OF NON - INFRINGEMENT, MERCHANTABILITY AND FITNESS FOR A
//             PARTICULAR PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND OPERATION
//             ALONE OR IN COMBINATION WITH YOUR PRODUCTS.
//
//             IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL OR
//             CONSEQUENTIAL DAMAGES ( INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
//             SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
//             INTERRUPTION ) ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION, MODIFICATION
//             AND / OR DISTRIBUTION OF THE APPLE SOFTWARE, HOWEVER CAUSED AND WHETHER
//             UNDER THEORY OF CONTRACT, TORT ( INCLUDING NEGLIGENCE ), STRICT LIABILITY OR
//             OTHERWISE, EVEN IF APPLE HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// Copyright ( C ) 2008 Apple Inc. All Rights Reserved.
//

#import "MyDocument.h"
#import <Foundation/Foundation.h>
#import "MovieDocument.h"
#import "MetadataAttributeMO.h"

NSString *MetadataAttributePBType = @"MetadataAttributePBType";

// forward declarations for QT metadata utility functions
NSString * LocalMetaDataGetItemKey(QTMetaDataRef metadataContainer, QTMetaDataItem metadataItem);
id LocalMetaDataGetItemValue(QTMetaDataRef metadataContainer, QTMetaDataItem metadataItem, UInt32 *pKeyDataType);
BOOL LocalRemoveSupportedMetadataElements( QTMetaDataRef metadataContainer );
BOOL LocalEmptyMovieMetadataContainers( Movie theMovie );

// document class implementation

@implementation MyDocument

//----------------------------------------
- (id)init 
{
    self = [super init];
    if (self != nil) {

        // initialization code
		theMovie = NULL;
    }
    return self;
}


//----------------------------------------
- (void)dealloc
{
	// dispose of local data
	if (theMovie) { [theMovie release]; theMovie = NULL; }
	[super dealloc];
}


//----------------------------------------
- (NSString *)windowNibName 
{
    return @"MyDocument";
}

//----------------------------------------
//setup the bindings for the metadata controllers to update against the movie
- (IBAction)resetMetadataBindings{
	[myMetadataController unbind:@"contentSet"];
	[myMetadataController setContent:nil];
	
	//set the content set of the metadataTreeController
	NSMutableDictionary*  options;
	options = [NSMutableDictionary new];
	[options setObject:[NSNumber numberWithBool:NO] forKey:@"NSConditionallySetsEnabled"];
	[options setObject:[NSNumber numberWithBool:NO] forKey:@"NSRaisesForNotApplicableKeys"];
	[myMetadataController bind:@"contentSet" toObject:myMovieController withKeyPath:@"selection.metadata" options:options];
	[options release];
}

//----------------------------------------
// once the UI is up and running, setup the data
- (void)windowControllerDidLoadNib:(NSWindowController *)windowController 
{
	// do the normal stuff
    [super windowControllerDidLoadNib:windowController];

    // point the movie view at our document movie
	[myMovieView setMovie:theMovie];
	
	//set the metadata content-- needs to happen after the movie is loaded or you'll confuse the controllers.
	[self resetMetadataBindings];
}


//========================================
# pragma mark table view delegate
//delegate of the table view stuff:


//--------------------------------------------------------------------------------
// if the user tries to edit some binary data or one of the image file types,
// pop open the inspector. Otherwise, let them edit!

- (BOOL)outlineView:(NSOutlineView *)outlineView shouldEditTableColumn:(NSTableColumn *)tableColumn item:(id)item{
	BOOL allowEdit = YES;
	
	//if the selection is on the data
	if (tableColumn == myValueTableColumn){
		// get the object from the right row
		MetadataAttributeMO *firstSelectedObject = [[myMetadataController selectedObjects] objectAtIndex: 0];
		
		//check if this is a piece of binary data that requires the inspector to edit
		UInt32 selectedObjectType = [[firstSelectedObject valueForKey:@"type"] intValue];
		switch (selectedObjectType){
			case kQTMetaDataTypeBinary:
			case kQTMetaDataTypeJPEGImage:
			case kQTMetaDataTypePNGImage:
			case kQTMetaDataTypeBMPImage:
				[self showInspector:self];
				allowEdit = NO;
				break;
//			default:
//				//do nothing, allow the edit to happen in the table.
		}
	}
	return allowEdit;
}

//==================================================
# pragma mark cut/copy/paste
// cut/copy/paste

//----------------------------------------
//build up a copy of the managed objects, both string and metadata types
- (void)copy:sender {
	// get the selected objects
    NSArray *selectedObjects = [myMetadataController selectedObjects];
    unsigned i, count = [selectedObjects count];
    if (count == 0) {
        return;
    }

	//make some arrays to store our copied representations
	NSMutableArray *copyObjectsArray = [NSMutableArray arrayWithCapacity:count];
	NSMutableArray *copyStringsArray = [NSMutableArray arrayWithCapacity:count];
	MetadataAttributeMO *metadata;
	 
	//iterate through the objects storing off a representation of each
	for (i = 0; i < count; i++) {
		metadata = (MetadataAttributeMO *)[selectedObjects objectAtIndex:i];
		[copyObjectsArray addObject:[metadata dictionaryRepresentation]];
		[copyStringsArray addObject:[metadata stringDescription]];
	}
	
	NSPasteboard *generalPasteboard = [NSPasteboard generalPasteboard];

	//make sure the pasteboard knows to send my type objects to me
	[generalPasteboard declareTypes:
			[NSArray arrayWithObjects:MetadataAttributePBType, NSStringPboardType, nil]
							  owner:self];

	// cache away the copies on the paste-board.
	NSData *copyData = [NSArchiver archivedDataWithRootObject:copyObjectsArray];
	[generalPasteboard setData:[copyData retain] forType:MetadataAttributePBType];
	[generalPasteboard setString:
		[[copyStringsArray componentsJoinedByString:@"\n"] retain]
						 forType:NSStringPboardType];
}

//----------------------------------------
//when we cut, just cache away the selection for me to use
- (void)cut:sender {
    [self copy:sender];
 
    NSManagedObjectContext *moc = [self managedObjectContext];
 
    NSArray *selectedMetadata = [myMetadataController selectedObjects];
    unsigned i, count = [selectedMetadata count];
 
    for (i = 0; i < count; i++) {
        NSManagedObject *metadata;
        metadata = (NSManagedObject *)[selectedMetadata objectAtIndex:i];
        [moc deleteObject:metadata];
    }
}

//----------------------------------------
//when we paste, add each item in
- (void)paste:sender {

	// collect up the right type from the pasteboard
    NSPasteboard *generalPasteboard = [NSPasteboard generalPasteboard];
    NSData *data = [generalPasteboard dataForType:MetadataAttributePBType];
    if (data == nil) {
        return;
    }
 
	//figure out where we are pasting to (which movie container)
    NSManagedObjectContext *documentMOC = [self managedObjectContext];
    NSArray *selectedObjects = [myMovieController selectedObjects];
    NSArray *pasteArray = [NSUnarchiver unarchiveObjectWithData:data];

    unsigned i, pasteFromCount = [pasteArray count], j, pasteToCount = [selectedObjects count];

	//for each place we're pasting to
	for (j = 0; j < pasteToCount; j++) {
		NSMutableSet *container = [[selectedObjects objectAtIndex:j] mutableSetValueForKey:@"metadata"];
		//for each object we're pasting
		for (i = 0; i < pasteFromCount; i++) {
			NSManagedObject			*keyEntry = NULL;
			
			//add the object
			keyEntry = [NSEntityDescription insertNewObjectForEntityForName:@"attribute" inManagedObjectContext:documentMOC];
			[keyEntry setValuesForKeysWithDictionary:[pasteArray objectAtIndex:i]];
			[container addObject:keyEntry];
		}
	}
			
 
}
//==================================================
# pragma mark inspector
// open/update the inspector

//----------------------------------------
//open up the inspector with the first item in the selection
- (IBAction)showInspector:(id)sender{
	id selectedObjects = [myMetadataController selectedObjects];
	
	//open up the inspector with either the first item in the selection or
	// clear out the selection data in the inspector
	if ([selectedObjects count] > 0)
		[myInspectorController editValue:[selectedObjects objectAtIndex: 0]];
	else [myInspectorController editValue: nil];
	
	//refresh
	[myMetadataController prepareContent];
}

//----------------------------------------
//update the inspector with either the first item in the selection or clear it out
- (IBAction)updateInspector:(id)sender{
	id selectedObjects = [myMetadataController selectedObjects];
	if ([selectedObjects count] > 0)
		[myInspectorController setEditItem:[selectedObjects objectAtIndex: 0]];
	else [myInspectorController setEditItem: nil];
}

//==================================================
# pragma mark read
// read metadata from QT movie


//----------------------------------------
//add the object to the destination set
 - (BOOL) readMetadataAddValue:(id)theValue forKey:(NSString*)theKey ofType:(UInt32)dataType ofSize:(UInt32)dataSize withIndex:(long)index toContainer:(NSMutableSet*)destinationSet error:(NSError **)errorPtr;
 {
 	// shared locals
	NSManagedObjectContext	*documentMOC = [self managedObjectContext];
	NSManagedObject			*keyEntry = NULL;


	keyEntry = [NSEntityDescription insertNewObjectForEntityForName:@"attribute" inManagedObjectContext:documentMOC];
	if (theKey != nil){
		[keyEntry setValue:theKey forKey:@"name"];
		[keyEntry setValue:[NSNumber numberWithLong:(dataType)] forKey:@"type"];
		[keyEntry setValue:[NSNumber numberWithLong:(index)] forKey:@"index"];
		if (dataSize > 0)
			[keyEntry setValue:[NSNumber numberWithLong:(dataSize)] forKey:@"size"];
		[keyEntry setValue:theValue forKey:[MetadataAttributeMO dataKeyForType:dataType]];
	}
	[destinationSet addObject:keyEntry];

	// return results
	return ((keyEntry != NULL)?(YES):(NO));
 }


//----------------------------------------
// read the metadata out of a movie container and add it to the set specified
 - (BOOL) readMetadataFromMovieContainer:(QTMetaDataRef)metadataContainer toSet:(NSMutableSet*)destinationSet error:(NSError **)errorPtr
 {
	// shared locals
	QTMetaDataItem	curMetadataItem = kQTMetaDataItemUninitialized;
	long			keyOrderingIndex = 0;
	BOOL			success = YES, foundMetadata = FALSE;
	
	// iterate over the metadata items using the reverse-dns namespace
	while (metadataContainer != nil && QTMetaDataGetNextItem(metadataContainer, kQTMetaDataStorageFormatQuickTime, 
		curMetadataItem, kQTMetaDataKeyFormatQuickTime, NULL, 0, &curMetadataItem) == noErr)
	{
		UInt32		keyDataType = 0, finalValueSize = 0;
		id			finalValueInstance = NULL;
		NSString	*finalKeyName = NULL;
		BOOL		useThisKey = YES;
		
		foundMetadata = TRUE;

		// retrieve the key
		if ((finalKeyName = LocalMetaDataGetItemKey(metadataContainer, curMetadataItem)) == NULL)
			useThisKey = NO;
		else if ((finalValueInstance = LocalMetaDataGetItemValue(metadataContainer, curMetadataItem, &keyDataType)) == NULL)
			useThisKey = NO;
			
		// assuming we were successful, add an entry to the output set/container
		if (success == YES && useThisKey == YES && finalValueInstance != NULL && finalKeyName != NULL)
		{
			success = [self readMetadataAddValue:finalValueInstance 
							forKey:finalKeyName
							ofType:keyDataType
							ofSize:finalValueSize
							withIndex:keyOrderingIndex
							toContainer:destinationSet
							error:errorPtr];
		
			// increment ordering index
			keyOrderingIndex++;
		}
	}

	// return results
	return (success);
 }

//----------------------------------------
// read a movie file from the URL, creating a metadata cache
- (BOOL)readFromURL:(NSURL *)absoluteURL ofType:(NSString *)typeName error:(NSError **)outError
{
	// shared locals
	BOOL		success = YES;

	// try to open the movie from the URL
	if ((theMovie = [QTMovie movieWithURL:absoluteURL error:outError]) == NULL)
		success = NO;
	else
		[theMovie retain];
		
	// assuming we opened the movie, call the superclass to configure an in-memory core data store
	// (the use of an in-memory store is specified in the file types pane)
	if (success == YES)
		success = [super readFromURL:absoluteURL ofType:typeName error:outError];
		
	// read data from the movie, filling our in-memory data store
	if (success == YES)
		success = [self readDataFromMovieFile:outError];
	
	// mark as NOT dirty at this point
	if (success == YES) {
		success = [[self managedObjectContext] save:outError];
		[[self undoManager] removeAllActions];
	}

	// return results
    return (success);
} // readFromURL


//----------------------------------------
// read a movie file from the URL, filling our in-memory data store
 - (BOOL) readDataFromMovieFile:(NSError **)errorPtr
 {
	// shared locals
	NSManagedObjectContext		*documentMOC = [self managedObjectContext];
	QTMetaDataRef				curMetadataContainer = NULL;
	BOOL						success = YES;
	
	// reset store state
	[documentMOC reset];
	
	// build base movie entry
	NSManagedObject *movieContainer = [NSEntityDescription insertNewObjectForEntityForName:@"container" inManagedObjectContext:documentMOC];
	[movieContainer setValue:[NSString stringWithFormat:@"Movie (%@)", [self displayName]] forKey:@"name"];
	NSMutableSet *movieChildrenSet = [movieContainer mutableSetValueForKey:@"children"];
	
	
	// read movie metadata
	QTCopyMovieMetaData([theMovie quickTimeMovie], &curMetadataContainer);
	if (curMetadataContainer != NULL)
	{
		success = [self readMetadataFromMovieContainer:curMetadataContainer toSet:[movieContainer mutableSetValueForKey:@"metadata"] error:errorPtr];
		QTMetaDataRelease(curMetadataContainer);
		curMetadataContainer = NULL;
	}

	// start iterating over tracks
	QTTrack * curTrack = NULL;
	NSEnumerator *	trackEnumerator = [[theMovie tracks] objectEnumerator];
	while (success == YES && (curTrack = [trackEnumerator nextObject]) != NULL)
	{
		// add the track
		NSManagedObject *trackContainer = [NSEntityDescription insertNewObjectForEntityForName:@"container" inManagedObjectContext:documentMOC];
		[trackContainer setValue:[NSString stringWithFormat:@"%@ - %@", 
			[curTrack attributeForKey:QTTrackIDAttribute], 
			[curTrack attributeForKey:QTTrackDisplayNameAttribute]] 
			forKey:@"name"];
		[movieChildrenSet addObject:trackContainer];
		NSMutableSet *trackChildrenSet = [trackContainer mutableSetValueForKey:@"children"];
		
		// read track metadata
		QTCopyTrackMetaData([curTrack quickTimeTrack], &curMetadataContainer);
		if (curMetadataContainer != NULL)
		{
			success = [self readMetadataFromMovieContainer:curMetadataContainer toSet:[trackContainer mutableSetValueForKey:@"metadata"] error:errorPtr];
			QTMetaDataRelease(curMetadataContainer);
			curMetadataContainer = NULL;
		}

		// add the media
		QTMedia * curTrackMedia = [curTrack media];
		NSManagedObject *mediaContainer = [NSEntityDescription insertNewObjectForEntityForName:@"container" inManagedObjectContext:documentMOC];
		[mediaContainer setValue:[NSString stringWithFormat:@"Media (%@)", [curTrackMedia attributeForKey:QTMediaTypeAttribute]] forKey:@"name"];
		[trackChildrenSet addObject:mediaContainer];
		
		// read media metadata
		QTCopyMediaMetaData([curTrackMedia quickTimeMedia], &curMetadataContainer);
		if (curMetadataContainer != NULL)
		{
			success = [self readMetadataFromMovieContainer:curMetadataContainer toSet:[mediaContainer mutableSetValueForKey:@"metadata"] error:errorPtr];
			QTMetaDataRelease(curMetadataContainer);
			curMetadataContainer = NULL;
		}
	}

	// return results
	return (success);
 }

//==================================================
# pragma mark write
// write metadata to QT movie


//----------------------------------------
//for saving back to the movie, override dataRepresentationOfType to map back to the movie's info
- (NSData *)dataRepresentationOfType:(NSString *)docType
{
    return [theMovie movieFormatRepresentation];
}

//--------------------------------------------------------------------------------
//write out metadata from our runtime managed object container to a specific a QT container
- (BOOL) writeMetadataFromContainer:(NSManagedObject*)container forKey:(NSString*)theKey ofType:(QTPropertyValueType)dataType toFileContainer:(QTMetaDataRef)destinationContainer error:(NSError **)errorPtr;
{
	BOOL			success = YES;

	NSData* nameData = [theKey dataUsingEncoding:NSUTF8StringEncoding];
	id data = [container valueForKey:@"data"];
	NSData* dataValue = nil;
	UInt8			*dataValuePtr = NULL;
	ByteCount		dataSize = 0;

	//switch here on type to get various things. 
	switch (dataType)
	{
		case kQTMetaDataTypeJPEGImage:
		case kQTMetaDataTypePNGImage:
		case kQTMetaDataTypeBMPImage:
		case kQTMetaDataTypeBinary:
			dataValue = (NSData*)data;
			break;

		case kQTMetaDataTypeUTF8:
			dataValue = [(NSString*)data dataUsingEncoding:NSUTF8StringEncoding];
			
			break;

		case kQTMetaDataTypeUTF16BE:
			dataValue = [(NSString*)data dataUsingEncoding:NSUnicodeStringEncoding];
			break;

		case kQTMetaDataTypeMacEncodedText:
			dataValue = [(NSString*)data dataUsingEncoding:NSMacOSRomanStringEncoding];
			break;
		
		case kQTMetaDataTypeSignedIntegerBE:
			{
				int intValue = NSSwapHostIntToBig([(NSNumber*)data intValue]);
				dataValuePtr = (UInt8*)(&intValue);
				dataSize = sizeof( int);
			}
			break;

		case kQTMetaDataTypeUnsignedIntegerBE:
			{
				unsigned int unsignedIntValue = NSSwapHostIntToBig([(NSNumber*)data unsignedIntValue]);
				dataValuePtr = (UInt8*)(&unsignedIntValue);
				dataSize = sizeof(unsigned int);
			}
			break;

		case kQTMetaDataTypeFloat32BE:
			{
				float floatValue = [(NSNumber*)data floatValue];
				dataValuePtr = (UInt8*)(&floatValue);
				dataSize = sizeof(float);
			}
			break;

		case kQTMetaDataTypeFloat64BE:
			{
				double doubleValue = [(NSNumber*)data doubleValue];
				dataValuePtr = (UInt8*)(&doubleValue);
				dataSize = sizeof(double);
			}
			break;
			
		// NOTE: in addition to the other data types, deal with containers!
		default:
			dataValue = (NSData*)data;
			NSLog ([NSString stringWithFormat:@"(unable to read value for type: %u, reading as binary)", dataType]);
			break;
	}

	if (dataValue != nil && dataValuePtr== NULL){
		dataValuePtr = (UInt8*)[dataValue bytes];
		dataSize = [dataValue length];
	}
	
	//now actually make the new metadata entry
	OSStatus errors = noErr;
	QTMetaDataItem	newMetadataEntry = kQTMetaDataItemUninitialized;
	
	if ((errors = QTMetaDataAddItem(destinationContainer, 
		kQTMetaDataStorageFormatQuickTime,			// format
		kQTMetaDataKeyFormatQuickTime,				// namespace
		[nameData bytes], ([nameData length]),		// element key name (strip trailing NULL)
		dataValuePtr, dataSize,		// element data
		dataType, &newMetadataEntry)) != noErr){
			success = NO;
//			NSLog(@"failed. No metadata written. :( :::::::::::::::::::::::::::::");
		} else {
//			NSLog(@"wrote metadata!!!");
		}

	if (errorPtr!= nil)
		*errorPtr = [NSError errorWithDomain:NSOSStatusErrorDomain code:errors userInfo:nil];
	
	return success;

} 

//--------------------------------------------------------------------------------
// write the contents of the cache dictionary to a movie file
- (BOOL) writeMetadata: ( NSMutableSet *)metadataSet toMovieContainer: (QTMetaDataRef)targetMetadataContainer
{
	// locals
	BOOL			success = YES;
	NSEnumerator *	metadataEnumerator = NULL;
	NSManagedObject *currMetadata = NULL;
	// enumerate keys
	if ((metadataEnumerator = [metadataSet objectEnumerator]) != NULL)
		while ((currMetadata = [metadataEnumerator nextObject]) != NULL && success == YES)
		{			
			// get the data, convert it into bytes, and add it to the metadata container
			NSString* name = [currMetadata valueForKey:@"name"];
			UInt32 dataType = [[currMetadata valueForKey:@"type"] longValue];
			// only one value for this name
			success = [self writeMetadataFromContainer: currMetadata forKey:name ofType:(QTPropertyValueType)dataType toFileContainer:targetMetadataContainer error:nil];

		}

	// cleanup and return
	return (success);
} // writeMetadata:metadataSet toMovieContainer:targetMetadataContainer

//----------------------------------------
// write any changes to the movie file
- (BOOL)writeWithBackupToFile:(NSString *)fullDocumentPath ofType:(NSString *)type saveOperation:(NSSaveOperationType)saveOperation
{
	BOOL		success = NO;
	
	// shared locals
	NSManagedObjectContext		*documentMOC = [self managedObjectContext];
	QTMetaDataRef				curMetadataContainer = NULL;

	//first wipe out everything in the container (it is easier to simply do this than to diff each piece of data)
	success = LocalEmptyMovieMetadataContainers( [theMovie quickTimeMovie] );

	if (success == YES && documentMOC){
		NSFetchRequest * theFetch = [[NSFetchRequest alloc] init];
		
		//at the top level, the movie container
		[theFetch setEntity:[NSEntityDescription entityForName:@"container" inManagedObjectContext:documentMOC]];
		NSString *movieContainerName = [NSString stringWithFormat:@"Movie (%@)", [self displayName]];
		
		//build up a predicate to find all the metadata that goes at this level
		[theFetch setPredicate:[NSPredicate 
			predicateWithFormat:@"(name != nil) AND (name == %@)", movieContainerName]];
		
		//find all the matching metadata in our run time database we want to write at this level
		NSError * curError = NULL;
		NSArray * theMovieFindResults = [documentMOC executeFetchRequest:theFetch error:&curError];
		if (theMovieFindResults != nil){
			NSEnumerator *storedMovieMetadataParentContainers = [theMovieFindResults objectEnumerator];
			NSManagedObject *storedParent = nil;
			
			// get the container where we're going to write the metadata to
			QTCopyMovieMetaData([theMovie quickTimeMovie], &curMetadataContainer);
			
			//for each metadata container in our data base
			while (curMetadataContainer != NULL && ((storedParent = [storedMovieMetadataParentContainers nextObject]) != NULL))
			{
				//write the metadata to the specified container
				success = [self writeMetadata: [storedParent mutableSetValueForKey:@"metadata"] toMovieContainer: curMetadataContainer];
				QTMetaDataRelease(curMetadataContainer);
				curMetadataContainer = NULL;
			}
		}
		
		// start iterating over tracks
		QTTrack * curTrack = NULL;
		NSEnumerator *	trackEnumerator = [[theMovie tracks] objectEnumerator];
		while (success == YES && (curTrack = [trackEnumerator nextObject]) != NULL)
		{
			// get the metadata for the track
			NSString *trackContainerName = [NSString stringWithFormat:@"%@ - %@", 
												[curTrack attributeForKey:QTTrackIDAttribute], 
												[curTrack attributeForKey:QTTrackDisplayNameAttribute]];

			//build up a predicate to find all the metadata that goes at this level
			[theFetch setPredicate:[NSPredicate 
				predicateWithFormat:@"(name != nil) AND (name == %@)", trackContainerName]];
			
			//find all the matching metadata in our run time database we want to write at this level
			NSError * curTrackError = NULL;
			NSArray * theTrackFindResults = [documentMOC executeFetchRequest:theFetch error:&curTrackError];
			if (theTrackFindResults != nil){
				NSEnumerator *storedTrackMetadataParentContainers = [theTrackFindResults objectEnumerator];
				NSManagedObject *storedParent = nil;
				
				// get the container where we're going to write the metadata to
				QTCopyTrackMetaData([curTrack quickTimeTrack], &curMetadataContainer);
				
				// for each metadata container in our data base
				while ((curMetadataContainer != NULL) && ((storedParent = [storedTrackMetadataParentContainers nextObject]) != nil))
				{
					//write the metadata to the specified container
					success = [self writeMetadata: [storedParent mutableSetValueForKey:@"metadata"] toMovieContainer: curMetadataContainer];
					QTMetaDataRelease(curMetadataContainer);
					curMetadataContainer = NULL;
				}
			}

			// get the metadata for the media
			QTMedia * curTrackMedia = [curTrack media];
			NSString *mediaContainerName = [NSString stringWithFormat:@"Media (%@)", [curTrackMedia attributeForKey:QTMediaTypeAttribute]];

			//build up a predicate to find all the metadata that goes at this level
			[theFetch setPredicate:[NSPredicate 
				predicateWithFormat:@"(name != nil) AND (name == %@)", mediaContainerName]];
			
			//find all the matching metadata in our run time database we want to write at this level
			NSError * curMediaErr = NULL;
			NSArray * theMediaFindResults = [documentMOC executeFetchRequest:theFetch error:&curMediaErr];
			if (theMediaFindResults != nil){
				NSEnumerator *storedMediaMetadataParentContainers = [theMediaFindResults objectEnumerator];
				NSManagedObject *storedParent = nil;
				
				// get the container where we're going to write the metadata to
				QTCopyMediaMetaData([curTrackMedia quickTimeMedia], &curMetadataContainer);
				
				// for each metadata container in our data base
				while (curMetadataContainer != NULL && ((storedParent = [storedMediaMetadataParentContainers nextObject])!= nil))
				{
					//write the metadata to the specified container
					success = [self writeMetadata: [storedParent mutableSetValueForKey:@"metadata"] toMovieContainer: curMetadataContainer];
					QTMetaDataRelease(curMetadataContainer);
					curMetadataContainer = NULL;
				}
			}
		}
	}

    // update/save
	if (success == YES){
	
		//save the qt movie (just updating an existing file)
		if (saveOperation == NSSaveOperation)
			success = [theMovie updateMovieFile];
		else{
			//save off the movieDoc (not this doc which is an NSPersistent doc of the metadata hierarchy tree, but instead the QT movie).
			// this is a save as operation
			MovieDocument* newMovieDoc = [MovieDocument movieDocumentWithMovie:theMovie];
			success = [newMovieDoc writeWithBackupToFile:fullDocumentPath ofType:type saveOperation:saveOperation];
		
			//update our runtime database to match the saved off file rather than be un-saved.
			if (success== YES){
				NSError * readErr = NULL;
				NSURL *fileURL = [NSURL fileURLWithPath:fullDocumentPath];

				//clear out the file so then we get our displayName refreshed when we revert the file to the newly saved as one.
				[self setFileURL:fileURL];
				[theMovie release];

				//revert to cleanup managedObjectContext and just get what we pushed down to the file, and to read in metadata back into UI with the new file name (at the top level).
				[self revertToContentsOfURL:fileURL ofType:type error:&readErr]; //Overridden to clean up the managedObjectContext and controllers during a revert. 
			}
		}
	}

    return (success);
	
}

@end
//========================================
# pragma mark utilites
// metadata utility functions

//----------------------------------------
// get the key for a bit of metadata on the QT movie
NSString * LocalMetaDataGetItemKey(QTMetaDataRef metadataContainer, QTMetaDataItem metadataItem)
{
	// lcoal shared variables
	ByteCount			propValueSize = 0, propValueSizeUsed = 0;
	QTPropertyValueType	propType = kQTMetaDataTypeBinary;
	QTPropertyValuePtr	propValuePtr = NULL;
	NSString			*finalString = NULL;

	// check arguments
	if (metadataContainer == NULL || metadataItem == kQTMetaDataItemUninitialized)
		return (NULL);

	// retrieve the key (FUNCTION)
	if (QTMetaDataGetItemPropertyInfo(metadataContainer, metadataItem, kPropertyClass_MetaDataItem, 
		kQTMetaDataItemPropertyID_Key, &propType, &propValueSize, NULL) == noErr && propType == 'cbuf')
	{
		// alloc a buffer
		if ((propValuePtr = malloc(propValueSize+4)) != NULL)
		{
			// clear the pointer (just to be safe)
			memset(propValuePtr, 0, (propValueSize+4));
			
			// retrieve the key, and create an NSString from it
			if (QTMetaDataGetItemProperty(metadataContainer, metadataItem, kPropertyClass_MetaDataItem, 
				kQTMetaDataItemPropertyID_Key, propValueSize, propValuePtr, &propValueSizeUsed) == noErr)
					finalString = [NSString stringWithCString:propValuePtr encoding:NSUTF8StringEncoding];

			// cleanup
			free(propValuePtr);
			propValuePtr = NULL;
		}
	}
	
	// return final string object
	return (finalString);
} // LocalMetaDataGetItemKey

//----------------------------------------
// get the value for a bit of metadata on the QT movie
id LocalMetaDataGetItemValue(QTMetaDataRef metadataContainer, QTMetaDataItem metadataItem, UInt32 *pKeyDataType)
{
	// lcoal shared variables
	ByteCount			propValueSize = 0, propValueSizeUsed = 0;
	QTPropertyValueType	propType = kQTMetaDataTypeBinary;
	QTPropertyValuePtr	propValuePtr = NULL;
	UInt32				keyDataType = 0;
	id					finalValueObject = NULL;
	OSErr				qtErr = noErr;

	// check arguments
	if (metadataContainer == NULL || metadataItem == kQTMetaDataItemUninitialized)
		return (NULL);

	// retrieve the data type for the value
	if ((qtErr = QTMetaDataGetItemPropertyInfo(metadataContainer, metadataItem, kPropertyClass_MetaDataItem, 
		kQTMetaDataItemPropertyID_DataType, &propType, &propValueSize, NULL)) == noErr && propType == 'ui32')
		if ((qtErr = QTMetaDataGetItemProperty(metadataContainer, metadataItem, kPropertyClass_MetaDataItem, 
			kQTMetaDataItemPropertyID_DataType, sizeof(keyDataType), &keyDataType, &propValueSizeUsed)) == noErr)
			keyDataType = keyDataType;

	// retrieve the actual value payload
	if (qtErr == noErr)
		if ((qtErr = QTMetaDataGetItemPropertyInfo(metadataContainer, metadataItem, kPropertyClass_MetaDataItem, 
			kQTMetaDataItemPropertyID_Value, &propType, &propValueSize, NULL)) == noErr)
		{
			// alloc a buffer
			if ((propValuePtr = malloc(propValueSize+4)) != NULL)
			{
				// clear the pointer (just to be safe)
				memset(propValuePtr, 0, (propValueSize+4));
				
				// retrieve the value
				if ((qtErr = QTMetaDataGetItemProperty(metadataContainer, metadataItem, kPropertyClass_MetaDataItem, 
					kQTMetaDataItemPropertyID_Value, propValueSize, propValuePtr, &propValueSizeUsed)) == noErr)
					switch (keyDataType)
					{
						case kQTMetaDataTypeJPEGImage:
						case kQTMetaDataTypePNGImage:
						case kQTMetaDataTypeBMPImage:
						case kQTMetaDataTypeBinary:
							finalValueObject = [NSData dataWithBytes:propValuePtr length:propValueSize];
							break;

						case kQTMetaDataTypeUTF8:
							finalValueObject = [NSString stringWithCString:propValuePtr encoding:NSUTF8StringEncoding];
							break;

						case kQTMetaDataTypeUTF16BE:
							finalValueObject = [NSString stringWithCString:propValuePtr encoding:NSUnicodeStringEncoding];
							break;

						case kQTMetaDataTypeMacEncodedText:
							finalValueObject = [NSString stringWithCString:propValuePtr encoding:NSMacOSRomanStringEncoding];
							break;
						
						case kQTMetaDataTypeSignedIntegerBE:
							if (propValueSize == sizeof(short))
								finalValueObject = [NSNumber numberWithShort:(NSSwapBigShortToHost(*((short *)propValuePtr)))];
							else if (propValueSize == sizeof(long))
								finalValueObject = [NSNumber numberWithLong:(NSSwapBigShortToHost(*((long *)propValuePtr)))];
							else if (propValueSize == sizeof(int))
								finalValueObject = [NSNumber numberWithLong:(NSSwapBigIntToHost(*((int *)propValuePtr)))];
							break;

						case kQTMetaDataTypeUnsignedIntegerBE:
							if (propValueSize == sizeof(unsigned short))
								finalValueObject = [NSNumber numberWithUnsignedShort:(NSSwapBigShortToHost(*((unsigned short *)propValuePtr)))];
							else if (propValueSize == sizeof(unsigned long))
								finalValueObject = [NSNumber numberWithUnsignedLong:(NSSwapBigLongToHost(*((unsigned long *)propValuePtr)))];
							else if (propValueSize == sizeof(unsigned int))
								finalValueObject = [NSNumber numberWithLong:(NSSwapBigIntToHost(*((unsigned int *)propValuePtr)))];
							break;

						case kQTMetaDataTypeFloat32BE:
							finalValueObject = [NSNumber numberWithFloat:(*((float *)propValuePtr))];
							break;

						case kQTMetaDataTypeFloat64BE:
							finalValueObject = [NSNumber numberWithDouble:(*((double *)propValuePtr))];
							break;
							
						// NOTE: in addition to the other data types, deal with containers!
						default:
							NSLog ([NSString stringWithFormat:@"(unable to read value for type: %u, storing as binary)", keyDataType]);
							finalValueObject = [NSData dataWithBytes:propValuePtr length:propValueSize];
							break;
					}


				// cleanup
				free(propValuePtr);
				propValuePtr = NULL;
			}
		}

	// retain and return the data type
	if (finalValueObject && pKeyDataType != NULL)
		(*pKeyDataType) = keyDataType;
	
	// return pointer
	return (finalValueObject);
} // LocalMetaDataGetItemValue

//--------------------------------------------------------------------------------
// empty any metadata containers found on the specified movie
BOOL LocalEmptyMovieMetadataContainers( Movie theMovie )
{
	// locals
	BOOL		success = YES;
	QTMetaDataRef		curMetadataContainer = NULL;
	long				trackCounter, trackTotal = 0;
	Track				curTrack = nil;
		
	// enumerate container contents on the movie
	QTCopyMovieMetaData(theMovie, &curMetadataContainer);
	if (curMetadataContainer != NULL)
	{
		// clear supported entries
		success = LocalRemoveSupportedMetadataElements(curMetadataContainer);

		// release and clear metadata container reference
		QTMetaDataRelease(curMetadataContainer);
		curMetadataContainer = NULL;
	}

	// enumerate tracks and media
	trackTotal = GetMovieTrackCount(theMovie);
	for (trackCounter = 1; trackCounter<=trackTotal && success==YES; trackCounter++)
		if ((curTrack = GetMovieIndTrack(theMovie, trackCounter)) != nil)
		{
			// locals
			Media					curMedia = nil;
			
			// go for track metadata
			curMetadataContainer = NULL;
			QTCopyTrackMetaData(curTrack, &curMetadataContainer);
			if (curMetadataContainer != NULL)
			{
				// clear supported entries
				success = LocalRemoveSupportedMetadataElements(curMetadataContainer);

				// release and clear metadata container reference
				QTMetaDataRelease(curMetadataContainer);
				curMetadataContainer = NULL;
			}
			
			// go for media metadata
			if ((curMedia = GetTrackMedia(curTrack)) != NULL)
				QTCopyMediaMetaData(curMedia, &curMetadataContainer);
			if (curMetadataContainer != NULL)
			{
				// clear supported entries
				success = LocalRemoveSupportedMetadataElements(curMetadataContainer);

				// release and clear metadata container reference
				QTMetaDataRelease(curMetadataContainer);
				curMetadataContainer = NULL;
			}
		}


	// cleanup and return
	return (success);
} // LocalEmptyMovieMetadataContainers



//--------------------------------------------------------------------------------
// check to see if we have metadata in a container that we care about
BOOL LocalRemoveSupportedMetadataElements( QTMetaDataRef metadataContainer )
{
	// locals
	QTMetaDataItem	curMetadataItem = kQTMetaDataItemUninitialized;

	// see if we have any items present
	if (QTMetaDataGetNextItem(metadataContainer, kQTMetaDataStorageFormatQuickTime, 
		kQTMetaDataItemUninitialized, kQTMetaDataKeyFormatQuickTime, NULL, 0, &curMetadataItem) == noErr)
	{
		// while we have entries AND we haven't found one that is appropriate, keep on enumerating
		while (curMetadataItem != kQTMetaDataItemUninitialized)
		{
			QTMetaDataItem			elementToRemove = kQTMetaDataItemUninitialized;
			ByteCount				propValueSize = 0, propValueSizeUsed = 0;
			QTPropertyValueType		propType = kQTMetaDataTypeBinary;
			UInt32					keyFormat = 0;
			
			// confirm this item uses the quicktime metadata format (reverse dns)
			if (QTMetaDataGetItemPropertyInfo(metadataContainer, curMetadataItem, kPropertyClass_MetaDataItem, 
				kQTMetaDataItemPropertyID_KeyFormat, &propType, &propValueSize, NULL) == noErr && propValueSize == sizeof(keyFormat))
				if (QTMetaDataGetItemProperty(metadataContainer, curMetadataItem, kPropertyClass_MetaDataItem, 
					kQTMetaDataItemPropertyID_KeyFormat, propValueSize, &keyFormat, &propValueSizeUsed) == noErr)
					if (keyFormat == kQTMetaDataKeyFormatQuickTime)		// NOTE: QuickTime handles endian swapping for values retrieved via the properties API
						elementToRemove = curMetadataItem;

			// get the next element
			if (QTMetaDataGetNextItem(metadataContainer, kQTMetaDataStorageFormatQuickTime, 
				curMetadataItem, kQTMetaDataKeyFormatQuickTime, NULL, 0, &curMetadataItem) != noErr)
				curMetadataItem = kQTMetaDataItemUninitialized;
				
			// if the previous element met our criteria, remove it
			if (elementToRemove != kQTMetaDataItemUninitialized)
				QTMetaDataRemoveItem(metadataContainer, elementToRemove);
		}
	}

	// return success
	return (YES);
} // LocalRemoveSupportedMetadataElements



