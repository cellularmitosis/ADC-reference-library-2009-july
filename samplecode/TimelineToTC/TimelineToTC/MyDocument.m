/*

File: MyDocument.m

Abstract:   Document class implementation for example
			application (TimelineToTC).  This class manages
			the the document UI, parses incoming XML data,
			and calls the conversion code to write exported
			edit list.

Version: 0.5

Disclaimer: IMPORTANT:  This Apple software is supplied to you by
Apple, Inc. ("Apple") in consideration of your agreement to the
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
Neither the name, trademarks, service marks or logos of Apple, Inc. 
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

Copyright ¬© 2007 Apple, Inc., All Rights Reserved

*/ 

#import "MyDocument.h"


//========================================
// document class method implementations
//========================================

@implementation MyDocument

//----------------------------------------
- (id)init
{
    self = [super init];
    if (self) {
    
        // Add your subclass-specific initialization here.
        // If an error occurs here, send a [self release] message and return nil.
		
		parsedData = NULL;
		exportKeyTableData = NULL;
    
    }
    return self;
}

//----------------------------------------
- (void)dealloc
{
	// free the xml parsed data instance
	if (parsedData)
	{
		[parsedData release];
		parsedData = NULL;
	}
	
	if (exportKeyTableData)
	{
		[exportKeyTableData release];
		exportKeyTableData = NULL;
	}
	
	// call superclass
	[super dealloc];
}


//----------------------------------------
- (NSString *)windowNibName
{
    // Override returning the nib file name of the document
    // If you need to use a subclass of NSWindowController or if your document supports multiple NSWindowControllers, you should remove this method and override -makeWindowControllers instead.
    return @"MyDocument";
}

//----------------------------------------
- (void)windowControllerDidLoadNib:(NSWindowController *) aController
{
    [super windowControllerDidLoadNib:aController];
	[self refreshDocumentUI:YES];
}

//----------------------------------------
// write changes to the document file
- (BOOL)writeToURL:(NSURL *)absoluteURL ofType:(NSString *)typeName error:(NSError **)outError
{
	return (NO);	// we only support reading/opening of xml files
}

//----------------------------------------
// read a movie file from the URL, creating a metadata cache
- (BOOL)readFromURL:(NSURL *)absoluteURL ofType:(NSString *)typeName error:(NSError **)outError
{
	// shared locals
	BOOL		success = YES;
	
	// is this a native project, or an XML file?
	if ([typeName isEqualToString:@"fcpXMLDocumentType"] == YES)
	{
		// read and parse the xml data
		if ((parsedData = [FCPDocumentProcessor processorWithFile:absoluteURL]) == NULL)
		{
			success = NO;	// error parsing file
		}
	}
	else
	{
		success = NO;	// unsupported type
	}

	// return results
    return (success);
}

//----------------------------------------
- (void)refreshDocumentUI:(BOOL)rebuildKeyList
{
	NSArray *	seqNames = NULL;
	
	// cleanup UI
	[sequencePopup removeAllItems];

	if (rebuildKeyList)
	{
		[exportKeyTableView setDataSource:NULL];
	}

	// fill sequence name popup
	if ([parsedData getListOfSequences:&seqNames] == YES)
	{
		// fill and enable the popup
		[sequencePopup addItemsWithTitles:seqNames];
		[sequencePopup setEnabled:YES];
		[exportButton setEnabled:YES];

		// select the first item and fill the description field
		[sequencePopup selectItemAtIndex:0];
		[self selectSequence:sequencePopup];
	}
	else
	{
		// add one entry and disable the popup
		[sequencePopup addItemWithTitle:@"(no sequences found)"];
		[sequencePopup setEnabled:NO];
		[exportButton setEnabled:NO];
	}
	
	// optionally rebuild and refill key list
	if (rebuildKeyList)
	{
		// get a list if we don't have one already
		if (exportKeyTableData == NULL)
			exportKeyTableData = [MyKeyTableData keyTableWithNameArray:[parsedData getExportOptionalKeyList]];

		// set the list for the control
		if (exportKeyTableData != NULL)
			[exportKeyTableView setDataSource:exportKeyTableData];
	}
}

//----------------------------------------
- (IBAction)selectSequence:(id)sender;
{
	// get the selected item and a description string, and use it for the description field
	NSString	*theSelectedItem = [sender titleOfSelectedItem];
	NSString	*newDescString = [parsedData getDescriptionOfSequence:theSelectedItem];
	[[[sequenceInfoField textStorage] mutableString] setString:newDescString];
}

//----------------------------------------
- (IBAction)startExport:(id)sender
{
	NSString	*theSelectedItem = [sequencePopup titleOfSelectedItem];
	NSString	*outputText = [parsedData getEditListForSequence:theSelectedItem withFields:[exportKeyTableData getEnabledKeyList]];

	// only continue if we successfully created the output text
	if (outputText)
	{
		// set the save panel to only do text documents
		NSSavePanel	*theSavePanel = [NSSavePanel savePanel];
		[theSavePanel setAllowedFileTypes:[NSArray arrayWithObject:@"txt"]];

		// run the panel
		if ([theSavePanel runModal] == NSFileHandlingPanelOKButton)
		{
			// if the user clicked on ok, create a file with the text in it
			[[NSFileManager defaultManager] createFileAtPath:[theSavePanel filename] contents:[outputText dataUsingEncoding:NSUTF8StringEncoding] attributes:NULL];
		}
	}
}

@end



//========================================
// key table data source class method implimentation
//========================================

@implementation MyKeyTableData

//----------------------------------------
- (id)init
{
    self = [super init];
    if (self) {
		storedData = NULL;
    }
    return self;
}

//----------------------------------------
- (void)dealloc
{
	// free the key dictionary
	if (storedData)
	{
		[storedData release];
		storedData = NULL;
	}
	
	// call superclass
	[super dealloc];
}

//----------------------------------------
+ (MyKeyTableData *)keyTableWithNameArray:(NSArray *) sourceKeyArray
{
	MyKeyTableData *resultInstance = [[[MyKeyTableData alloc] init] retain];
	if (resultInstance)
		if ((resultInstance->storedData = [ [NSMutableArray arrayWithCapacity:[sourceKeyArray count]] retain]) != NULL)
		{
			// enumerate keys, building entries as appropriate
			NSEnumerator *nameEnumerator = [sourceKeyArray objectEnumerator];
			NSString *curKeyName = NULL;
			
			// walk keys, creating an entry for each one
			while (curKeyName = [nameEnumerator nextObject])
			{
				NSDictionary *newEntry = [NSMutableDictionary dictionaryWithObjectsAndKeys:
											curKeyName, @"name", [NSNumber numberWithBool:NO], @"enable", NULL];
				[resultInstance->storedData addObject:newEntry];
			}
		}

	// return instance
	return (resultInstance);
}

//----------------------------------------
- (NSArray *)getEnabledKeyList
{
	NSMutableArray *workingArray = [NSMutableArray arrayWithCapacity:[storedData count]];
	if (workingArray)
	{
		// get an enumerator for the keys
		NSEnumerator *entryEnumerator = [storedData objectEnumerator];
		NSMutableDictionary *curEntry = NULL;

		// walk the entries, copying the names of the items that are enabled
		while (curEntry = (NSMutableDictionary *)[entryEnumerator nextObject])
			if ([[curEntry objectForKey:@"enable"] boolValue] == YES)
				[workingArray addObject:[curEntry objectForKey:@"name"]];

		// return a non mutable copy of the array
		return ([NSArray arrayWithArray:workingArray]);
	}
	
	// fallthrough: return NULL
	return (NULL);
}

//----------------------------------------
- (int)numberOfRowsInTableView:(NSTableView *)tableView
{
	return [storedData count];
}

//----------------------------------------
- (id)tableView:(NSTableView *)tableView objectValueForTableColumn:(NSTableColumn *)tableColumn row:(int)rowIndex
{
	NSParameterAssert(rowIndex >= 0 && rowIndex < [storedData count]);
	return [[storedData objectAtIndex:rowIndex] objectForKey:[tableColumn identifier]];
}

//----------------------------------------
- (void)tableView:(NSTableView *)tableView setObjectValue:(id)object forTableColumn:(NSTableColumn *)tableColumn row:(int)rowIndex;
{
	NSParameterAssert(rowIndex >= 0 && rowIndex < [storedData count]);
	[[storedData objectAtIndex:rowIndex] setObject:object forKey:[tableColumn identifier]];
}

@end

