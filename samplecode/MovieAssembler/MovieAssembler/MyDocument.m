/*

File: MyDocument.m

Abstract:   Document delegate class implementation for example
			application (MovieAssembler).  This class manages
			the the document UI, drives the watchfolder handling
			and initiates communication with the target instance
			of Final Cut Pro.

Version: 1.0

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

Copyright © 2007 Apple, Inc., All Rights Reserved

*/ 

#import "MyDocument.h"
#import "FCP_AppleEvents.h"
#import "apple_event.h"
#include <sys/types.h>
#include <sys/stat.h>
#import <Cocoa/Cocoa.h>
#import <QTKit/QTKit.h>


@implementation MyDocument

//----------------------------------------
- (id)init
{
    self = [super init];
    if (self) {
		
		// init member variables
		processingFiles = NO;
		theWatchTimer = NULL;
		workingProjectURL = NULL;
		activelyWatching = NO;
    }
    return self;
}

//----------------------------------------
- (void)dealloc
{
	// stop the watch folder timer if it's still running
	if (theWatchTimer)
	{
		[theWatchTimer invalidate];
		theWatchTimer = NULL;
	}
	
	// free saved project URL
	if (workingProjectURL)
		[workingProjectURL release];
	
	// call superclass
	[super dealloc];
}

//----------------------------------------
- (NSString *)windowNibName
{
    return @"MyDocument";	// Override returning the nib file name of the document
}

//----------------------------------------
- (void)windowControllerDidLoadNib:(NSWindowController *) aController
{
    [super windowControllerDidLoadNib:aController];
	[self refreshCachedProjectData:YES];	// attempt to refresh the UI
}

//----------------------------------------
// write changes to the document file
- (BOOL)writeToURL:(NSURL *)absoluteURL ofType:(NSString *)typeName error:(NSError **)outError
{
	return (NO);	// we only support reading/opening of projects
}

//----------------------------------------
// read a movie file from the URL, creating a metadata cache
- (BOOL)readFromURL:(NSURL *)absoluteURL ofType:(NSString *)typeName error:(NSError **)outError
{
	// shared locals
	BOOL		success = YES;
	
	// is this a native project, or an XML file?
	if ([typeName isEqualToString:@"fcpProjectType"] == YES)
	{
		// mark as native and save URL
		workingProjectURL = [absoluteURL retain];

		// open project in FCP
		success = [self openProjectInFinalCutPro];
	}
	else
	{
		success = NO;	// unsupported type
	}

	// return results
    return (success);
}


//----------------------------------------
// browse for a new media folder path
- (IBAction)browseMediaFolder:(id)sender
{
	// bring up an open panel, filling the media field once it's done
	NSOpenPanel	*openPanel = [NSOpenPanel openPanel];
	[openPanel setTitle:@"Select Media Folder"];
	[openPanel setCanChooseDirectories:YES];
	[openPanel setCanChooseFiles:NO];
	if ([openPanel runModalForTypes:NULL] == NSOKButton)
		[mediaPathEdit setStringValue:[[openPanel filenames] objectAtIndex:0]];
}

//----------------------------------------
// browse for a new watch folder path
- (IBAction)browseWatchFolder:(id)sender
{
	// bring up an open panel, filling the media field once it's done
	NSOpenPanel	*openPanel = [NSOpenPanel openPanel];
	[openPanel setTitle:@"Select Watch Folder"];
	[openPanel setCanChooseDirectories:YES];
	[openPanel setCanChooseFiles:NO];
	if ([openPanel runModalForTypes:NULL] == NSOKButton)
		[watchPathEdit setStringValue:[[openPanel filenames] objectAtIndex:0]];
}


//----------------------------------------
// change the current target sequence
- (IBAction)selectTargetSequence:(id)sender
{
	// if the user selected the "new sequence" option, then make one
	if ([sender indexOfSelectedItem] == 0)
	{
		// bring up new sequence name panel
		[NSApp beginSheet: newSequenceNamePanel
				modalForWindow: [self windowForSheet]
				modalDelegate: nil
				didEndSelector: nil
				contextInfo: nil];
		[NSApp runModalForWindow: newSequenceNamePanel];
		// (dialog is running here)
		[NSApp endSheet: newSequenceNamePanel];
		[newSequenceNamePanel orderOut: self];
		
		// get string from dialog
		NSString *newSeqName = [newSequenceNameEdit stringValue];
		if ([newSeqName length] <= 0)
			newSeqName = NULL;

		// only continue if we received a string
		if (newSeqName)
		{
			// read base file from bundle
			NSString * sourceSnippetPath = [NSString stringWithFormat:@"%@/EmptySequence.xml", [[NSBundle mainBundle] resourcePath]];
			NSString * rawXMLString = [NSString stringWithContentsOfFile:sourceSnippetPath encoding:NSUTF8StringEncoding error:NULL];
			
			// only continue if we read the base xml file
			if (rawXMLString)
			{
				// construct XML snippet for the sequence
				NSMutableString *xmlSnippet = [[NSMutableString alloc] initWithCapacity:2048];
				[xmlSnippet appendString:rawXMLString];
				NSRange stringRange = {0, [xmlSnippet length]};
				[xmlSnippet replaceOccurrencesOfString:@"NewEmptySequence" withString:newSeqName options:NSCaseInsensitiveSearch range:stringRange];
				
				// send the sequence to Final Cut Pro
				apple_event *theEvent =	[[apple_event alloc] init];
				NSData	*finalData =	[NSData	dataWithBytes:[xmlSnippet UTF8String] 
												length:[xmlSnippet lengthOfBytesUsingEncoding:NSUTF8StringEncoding]];

				[theEvent	create:kAEImportXMLToDocument	// Event: open a project file
							class:kFCPEventClass			// Class: FCP Events
							dest:kFCPEventClass];			// Destination: FCP Events

				[theEvent	projectfile:NULL				// no path (using URL)
							url:workingProjectURL			// project file (specified by URL)
							as:sendAsURL];					// send type (URL)

				[theEvent	sendXML:finalData];				// attach the XML Data
				
				// send the event
				[theEvent send];
			
				// add the new sequence to the end of the popup and select it
				[sequencePopup addItemWithTitle:newSeqName];
				[sequencePopup selectItemAtIndex:([sequencePopup numberOfItems]-1)];

				// log success
				[self recordLogText:@"Succesfully created new sequence."];
			}
		}
	}
}

//----------------------------------------
// finish name panel
- (IBAction)closeSequenceNamePanel:(id)sender
{
    [NSApp stopModal];
}

//----------------------------------------
// start or stop monitoring of the watch folder, and the processing of events
- (IBAction)startStopMonitoring:(id)sender
{
	// locals
	BOOL	success = YES;

	// are we watching or not?
	if (activelyWatching == NO)
	{
		// validate paths
		NSFileManager *theFileManager = [NSFileManager defaultManager];
		NSString * curMediaPath = [mediaPathEdit stringValue];
		NSString * curWatchPath = [watchPathEdit stringValue];
		BOOL	pathIsFolder = NO;
		
		// does the media path exist, and is it a folder?
		if (curMediaPath == NULL || [curMediaPath length] < 1)
		{
			[self recordLogText:@"ERROR: Specified media path does not exist, aborting monitoring startup."];
			success = NO;
		}
		else if ([theFileManager fileExistsAtPath:curMediaPath isDirectory:&pathIsFolder] == NO || pathIsFolder == NO)
		{
			[self recordLogText:@"ERROR: Specified media path is not a folder, aborting monitoring startup."];
			success = NO;
		}
		
		// does the watch path exist?
		if (curWatchPath == NULL || [curWatchPath length] < 1)
		{
			[self recordLogText:@"ERROR: Specified watch path does not exist, aborting monitoring startup."];
			success = NO;
		}
		else if ([theFileManager fileExistsAtPath:curWatchPath isDirectory:&pathIsFolder] == NO || pathIsFolder == NO)
		{
			[self recordLogText:@"ERROR: Specified watch path is not a folder, aborting monitoring startup."];
			success = NO;
		}
		
		// attempt conversation with Final Cut Pro
		if (success == YES)
			success = [self openProjectInFinalCutPro];
				
		// start watch timer
		if (success == YES)
			if ((theWatchTimer = [NSTimer scheduledTimerWithTimeInterval:2 target:self selector:@selector(watchFolderTimerMethod:) userInfo:NULL repeats:YES]) == NULL)
				success = NO;

		// reset state
		if (success == YES)
		{
			[mediaPathEdit setEnabled:NO];
			[mediaPathButton setEnabled:NO];
			[watchPathEdit setEnabled:NO];
			[watchPathButton setEnabled:NO];
		
			if (startStopButton)
				[startStopButton setTitle:@"STOP"];
			activelyWatching = YES;
			[self recordLogText:@"Initiated watch folder monitoring."];
		}
		else
		{
			[self recordLogText:@"ERROR: Failed to START monitoring watch folder."];
		}
	}
	else
	{
		// stop watch timer
		if (theWatchTimer)
		{
			[theWatchTimer invalidate];
			theWatchTimer = NULL;
		}

		// reset state
		if (success == YES)
		{
			[mediaPathEdit setEnabled:YES];
			[mediaPathButton setEnabled:YES];
			[watchPathEdit setEnabled:YES];
			[watchPathButton setEnabled:YES];

			if (startStopButton)
				[startStopButton setTitle:@"START"];
			activelyWatching = NO;
			[self recordLogText:@"Ended watch folder monitoring."];
		}
		else
		{
			[self recordLogText:@"ERROR: Failed to STOP monitoring watch folder."];
		}
	}
}

//----------------------------------------
// change the current target sequence
- (IBAction)refreshSequenceList:(id)sender
{
	[self refreshCachedProjectData:NO];	// refresh the sequence popup
}

//----------------------------------------
// add text to the log window
- (void)recordLogText:(NSString*)newLogText
{
	NSString * finalString = [[NSString alloc] initWithFormat:@"%@ : %@\n", [[NSDate date] description], newLogText];

	if (actionLogEdit)
	{
		 NSTextStorage *textStorage = [actionLogEdit textStorage];
		 NSAttributedString *attribText = [[NSAttributedString alloc] initWithString:finalString];
		 [textStorage insertAttributedString:attribText atIndex:0];
	}
}

//----------------------------------------
// open the current project in final cut pro - done when monitoring begins
- (BOOL)openProjectInFinalCutPro
{
	// allocate an apple event transmitter object
	apple_event *theEvent = [[apple_event alloc] init];

	[theEvent	create:kAEFCPOpenProjectFile	// Event: open a project file
				class:kFCPEventClass			// Class: FCP Events
				dest:kFCPEventClass];			// Destination: FCP Events

	[theEvent	projectfile:NULL				// no path (using URL)
				url:workingProjectURL			// project file (specified by URL)
				as:sendAsURL];					// send type (URL)
	
	// send the event
	[theEvent send];

	// return success
	return (YES);
}

//----------------------------------------
// get the xml from FCP for the project and scrape it for useful information
- (BOOL)refreshCachedProjectData:(BOOL)retrievePaths
{
	// locals
	BOOL			success = YES;
	NSData			*resultXMLData = NULL;
	NSMutableArray	*localSequenceInfoArray = NULL;
	
	// get XML data for the current project from FCP
	if (success == YES)
	{
		// allocate an apple event transmitter object
		apple_event *theEvent = [[apple_event alloc] init];

		[theEvent	create:kKGAEGetDocumentXML	// Event: get the xml for the project
					class:kFCPEventClass		// Class: FCP Events
					dest:kFCPEventClass];		// Destination: FCP Events

		[theEvent	projectfile:NULL			// no path (using URL)
					url:workingProjectURL		// project file (specified by URL)
					as:sendAsURL];				// send type (URL)
					
		[theEvent	version:@"3.0"];				// set desired XML version
		
		// send the event
		[theEvent send];
		
		// get the XML data
		resultXMLData = [theEvent getXMLResultTextData];
		if (resultXMLData == NULL)
			success = NO;
	}
	
	// scrape the XML data for sequence information
	if (success == YES) {
	
		NSError	*localError = NULL;
		NSXMLDocument *localXMLTreeRoot = [[NSXMLDocument alloc] initWithData:resultXMLData options:0 error:&localError];
		
		if (localXMLTreeRoot == NULL)
		{
			success = NO;
		}
		else
		{
			NSEnumerator * seqEnumerator = NULL, *pathEnumerator = NULL;
			NSXMLNode * curElement = NULL, *nameElement = NULL;
			NSArray * foundSequences = NULL, *foundFiles = NULL, *foundNames = NULL;
			NSString *theSeqName = NULL;
			
			// find each sequence, storing it's name in our list
			if ((foundSequences = [localXMLTreeRoot objectsForXQuery:@".//sequence" error:&localError]) != NULL)
				if ((seqEnumerator = [foundSequences objectEnumerator]) != NULL)
				{
					localSequenceInfoArray = [NSMutableArray arrayWithCapacity:[foundSequences count]];
					while ((curElement = (NSXMLNode*) [seqEnumerator nextObject]))
						if ((foundNames = [curElement objectsForXQuery:@"./name" error:&localError]))
							if ((nameElement = [foundNames objectAtIndex:0]) != NULL)
							{
								theSeqName = [nameElement stringValue];
								[localSequenceInfoArray addObject:theSeqName];
							}
				}
				
			// rebuild the sequence popup and set the selection
			[sequencePopup removeAllItems];
			[sequencePopup addItemWithTitle:@"(create new sequence)"];
			if ([localSequenceInfoArray count])
			{
				[sequencePopup addItemsWithTitles:localSequenceInfoArray];
				[sequencePopup selectItemAtIndex:1];
			}
			[self recordLogText:@"Successfully rebuilt sequence list."];

			// deal with monitoring paths
			if (retrievePaths == YES)
			{
				NSString *foundMediaPath = NULL, *foundWatchPath = NULL;
				
				// scan the project for a media path, using the first movie file we find
				if ((foundFiles = [localXMLTreeRoot objectsForXQuery:@".//pathurl" error:&localError]) != NULL)
					if ((pathEnumerator = [foundFiles objectEnumerator]) != NULL)
						while ((curElement = (NSXMLNode*) [pathEnumerator nextObject]) && foundMediaPath == NULL)
						{
							NSURL * candidateURL = [NSURL URLWithString:[curElement stringValue]];
							NSString * candidatePath = [candidateURL path];
							if ([[candidatePath pathExtension] caseInsensitiveCompare:@"mov"] == NSOrderedSame)
							{
								[self recordLogText:@"Found media storage path in project."];
								foundMediaPath = [candidatePath stringByDeletingLastPathComponent];
							}
						}

				// infer media path
				if (foundMediaPath == NULL)
				{
					foundMediaPath = [[[workingProjectURL path] stringByDeletingLastPathComponent] stringByAppendingString:@"/Media"];
					[self recordLogText:@"Inferred media storage path."];
				}
				
				// infer watch path
				if (foundWatchPath == NULL)
				{
					foundWatchPath = [[[workingProjectURL path] stringByDeletingLastPathComponent] stringByAppendingString:@"/Incoming"];
					[self recordLogText:@"Inferred watch folder path."];
				}
				
				// update UI with paths
				if (foundMediaPath)	[mediaPathEdit setStringValue:foundMediaPath];
				if (foundWatchPath)	[watchPathEdit setStringValue:foundWatchPath];
			}
		}
	}
	
	// return results
	return (success);
}

//----------------------------------------
// timer callback method: check the watchfolder for files we haven't processed
- (void)watchFolderTimerMethod:(NSTimer*)theTimer
{
	// make sure we don't enter this twice
	if (processingFiles == NO)
	{
		// set timer semaphore
		processingFiles = YES;
	
		// look for files in the watch folder
		NSFileManager *theFileManager = [NSFileManager defaultManager];
		NSDirectoryEnumerator *watchFolderEnumerator = [theFileManager enumeratorAtPath:[watchPathEdit stringValue]];
		NSString *curFilename = NULL;
		BOOL isFolder = NO;
		
		// walk the files, processing movie files
		while ((curFilename = [watchFolderEnumerator nextObject]))
		{
			NSString *sourcePath = [[watchPathEdit stringValue] stringByAppendingPathComponent:curFilename];
			[theFileManager fileExistsAtPath:sourcePath isDirectory:&isFolder];
			if (isFolder == YES)
			{
				[watchFolderEnumerator skipDescendents];
			}
			else
			{
				// add to set to record as being tracked
				NSString	*cachedFilename = [NSString stringWithString:curFilename];

				// construct destination path, and check for existance
				NSString *destPath = [[mediaPathEdit stringValue] stringByAppendingPathComponent:cachedFilename];
				
				if ([theFileManager fileExistsAtPath:destPath] == NO)
				{
					NSWorkspace *theWorkspace = [NSWorkspace sharedWorkspace];
					int operationTag = -1;
					NSArray * theFileArray = [NSArray arrayWithObject:cachedFilename];
					struct stat	fileStats[2];
					
					// get the initial file modification stats
					memset(fileStats, 0, (sizeof(stat)*2));
					stat([sourcePath UTF8String], &fileStats[0]);

					// while the time is changing, keep on waiting
					do
					{
						// log a message?
						if (fileStats[1].st_mtimespec.tv_sec != 0)
							NSLog(@"new file in watchfolder is busy, retrying");

						// sleep for a second
						sleep(1);
						
						// get the new stats so we can compare to the old ones
						fileStats[0] = fileStats[1];
						stat([sourcePath UTF8String], &fileStats[1]);
					}
					while (fileStats[0].st_mtimespec.tv_sec != fileStats[1].st_mtimespec.tv_sec);
					
					// perform the actual move operation
					if ([theWorkspace	performFileOperation:NSWorkspaceMoveOperation 
											source:[watchPathEdit stringValue]
											destination:[mediaPathEdit stringValue]
											files:theFileArray
											tag:&operationTag] == YES && operationTag >= 0)
					{
						// log this file as being moved
						[self recordLogText:[NSString stringWithFormat:@"Successfully moved %@ into media folder.", cachedFilename]];
						
						// attempt to add this file to the sequence/project we're monitoring
						[self processFileForProject:destPath];
					}
				}
			}
		}
		
		// reset timer semaphore
		processingFiles = NO;
	}
}

//----------------------------------------
// process file for project
- (BOOL)processFileForProject:(NSString *)newFilePath
{
	// locals
	BOOL			success = YES;
	NSData			*resultXMLData = NULL;
	NSString		*newClipID = NULL, *finalFilePathURL = NULL;
	
	
	// get the clip ID from the file
	if ((newClipID = [self readClipIDFromFile:newFilePath]) == NULL)
	{
		NSLog(@"Unable to get ID for new file");
		return (NO);
	}
	else
	{
		NSURL * tempURL = [NSURL fileURLWithPath:newFilePath];
		finalFilePathURL = [tempURL absoluteString];
	}
	
	// get XML data for the current project from FCP
	if (success == YES)
	{
		// allocate an apple event transmitter object
		apple_event *theEvent = [[apple_event alloc] init];

		[theEvent	create:kKGAEGetDocumentXML	// Event: get the xml for the project
					class:kFCPEventClass		// Class: FCP Events
					dest:kFCPEventClass];		// Destination: FCP Events

		[theEvent	projectfile:NULL			// no path (using URL)
					url:workingProjectURL		// project file (specified by URL)
					as:sendAsURL];				// send type (URL)
					
		[theEvent	version:@"3.0"];				// set desired XML version
		
		// send the event
		[theEvent send];
		
		// get the XML data
		resultXMLData = [theEvent getXMLResultTextData];
		if (resultXMLData == NULL)
			success = NO;
	}
	
	// scrape the XML data for sequence information
	if (success == YES) {
	
		NSError	*localError = NULL;
		NSXMLDocument *localXMLTreeRoot = [[NSXMLDocument alloc] initWithData:resultXMLData options:0 error:&localError];
		NSString * selSequenceTitle = [sequencePopup titleOfSelectedItem];
		NSXMLNode * workingSequence = NULL;
		
		if (localXMLTreeRoot == NULL)
		{
			success = NO;
		}
		else
		{
			NSEnumerator * seqEnumerator = NULL;
			NSXMLNode * curElement = NULL;
			NSArray * foundSequences = NULL;
			
			// find the sequence we need
			if ((foundSequences = [localXMLTreeRoot objectsForXQuery:@".//sequence" error:&localError]) != NULL)
				if ((seqEnumerator = [foundSequences objectEnumerator]) != NULL)
					while ((curElement = (NSXMLNode*) [seqEnumerator nextObject]) && workingSequence == NULL)
						if ([[[[curElement objectsForXQuery:@"./name" error:&localError] objectAtIndex:0] stringValue] compare:selSequenceTitle] == NSOrderedSame)
							workingSequence = curElement;
							
			// assuming we found the sequence, query the sequence for clipitems with the same name
			if (workingSequence != NULL)
			{
				NSMutableArray * sharedFileDefinitions = NULL;
				NSEnumerator * itemEnumerator = NULL;
				NSXMLElement * curClipItem = NULL;
				NSArray * foundClipItems = NULL;
				
				// walk clipitems with the specified clip ID for it's name
				NSString * clipItemSearchString = [NSString stringWithFormat:@".//clipitem[name=\"%@\"]", newClipID];
				if ((foundClipItems = [workingSequence objectsForXQuery:clipItemSearchString error:&localError]) != NULL)
					if ((itemEnumerator = [foundClipItems objectEnumerator]) != NULL)
						while ((curClipItem = (NSXMLElement*) [itemEnumerator nextObject]))
						{
							// replace file definition
							NSXMLNode * nameElement = [NSXMLNode elementWithName:@"name" stringValue:newClipID];
							NSXMLNode * pathElement = [NSXMLNode elementWithName:@"pathurl" stringValue:finalFilePathURL];
							
							NSXMLElement * newFileElement = [[NSXMLElement alloc] initWithName:@"file"];
							[newFileElement addChild:nameElement];
							[newFileElement addChild:pathElement];
							
							// find the file definition for this item
							NSXMLElement * oldFile = [[curClipItem objectsForXQuery:@"./file" error:&localError] objectAtIndex:0];
							unsigned int oldFileIndex = [oldFile index];

							// FIXUP! Deal with shared file definitions going away...
							if ([oldFile childCount] > 0)
							{
								// alloc a mutable array if we don't have one...
								if (sharedFileDefinitions == NULL)
									sharedFileDefinitions = [NSMutableArray arrayWithCapacity:4];
									
								// add this file to the tracking array
								[sharedFileDefinitions addObject:oldFile];
							}
							
							// detach the old file definition and add a new file definition to this clipitem
							[oldFile detach];
							[curClipItem insertChild:newFileElement atIndex:oldFileIndex];
						}
				
				// deal with shared file definitions...
				if (sharedFileDefinitions)
				{
					NSEnumerator *oldFileEnumerator = NULL;
					NSXMLElement *curOldFileElement = NULL;
					NSXMLNode *fileIDAttribute = NULL;
					
					// for each "old" file, look for the first sparse usage of it (if any) and replace it with the full definition
					if ((oldFileEnumerator = [sharedFileDefinitions objectEnumerator]) != NULL)
						while ((curOldFileElement = (NSXMLElement*) [oldFileEnumerator nextObject]) != NULL)
							if ((fileIDAttribute = [curOldFileElement attributeForName:@"id"]) != NULL)
							{
								NSString * oldFileID = [fileIDAttribute stringValue];
								NSString * oldFileSearchString = [NSString stringWithFormat:@".//file[@id=\"%@\"]", oldFileID];
								NSArray * foundSeqFiles = [workingSequence objectsForXQuery:oldFileSearchString error:&localError];
								
								// if we found at least one file, replace it
								if ([foundSeqFiles count] > 0)
								{
									NSXMLElement *replaceFileElement = [foundSeqFiles objectAtIndex:0];
									NSXMLElement *replaceFileParent = (NSXMLElement*) [replaceFileElement parent];
									unsigned int replaceFileIndex = [replaceFileElement index];
									
									// detach the old file and insert the new/old full file definition at it's location.
									[replaceFileElement detach];
									[replaceFileParent insertChild:curOldFileElement atIndex:replaceFileIndex];
								}
							}
				}

				// change updatebehavior in sequence
				[[[workingSequence objectsForXQuery:@"./updatebehavior" error:&localError] objectAtIndex:0] setStringValue:@"replaceiffound"];
							
				// create new xml document
				NSXMLDocument *newTreeRoot = [[NSXMLDocument alloc] 
					initWithXMLString:@"<?xml version=\"1.0\" encoding=\"UTF-8\"?><!DOCTYPE xmeml><xmeml version=\"3\"></xmeml>"
					options:0 error:&localError];

				// add sequence to new document
				[workingSequence detach];
				NSXMLElement * newRootElement = [[newTreeRoot objectsForXQuery:@"./xmeml" error:&localError] objectAtIndex:0];
				[newRootElement addChild:workingSequence];
				
				// send XML to final cut pro
				// send the sequence to Final Cut Pro
				apple_event *sendEvent =	[[apple_event alloc] init];
				NSData	*finalData =	[newTreeRoot XMLData];

				[sendEvent	create:kAEImportXMLToDocument	// Event: open a project file
							class:kFCPEventClass			// Class: FCP Events
							dest:kFCPEventClass];			// Destination: FCP Events

				[sendEvent	projectfile:NULL				// no path (using URL)
							url:workingProjectURL			// project file (specified by URL)
							as:sendAsURL];					// send type (URL)

				[sendEvent	sendXML:finalData];				// attach the XML Data
				
				// send the event
				[sendEvent send];
			}
			else
			{
				success = NO;
			}
		}
	}

	return (success);
}

//----------------------------------------
// read clip ID from media file
- (NSString*)readClipIDFromFile:(NSString *)filePath
{
	// locals
	NSString *	resultString = NULL;
	NSURL *		fileURL = NULL;
	QTMovie *	theMovie = NULL;
	NSError	*	theError = NULL;
	
	// open the movie from the path
	if ((fileURL = [NSURL fileURLWithPath:filePath]) != NULL)
		if ((theMovie = [QTMovie movieWithURL:fileURL error:&theError]) != NULL)
		{
			QTMetaDataRef		curMetadataContainer = NULL;
			QTMetaDataItem		curMetadataItem = kQTMetaDataItemUninitialized;
			char				*keyName = CLIP_ID_METADATA_KEY;
			ByteCount			propValueSize = 0, propValueSizeUsed = 0;
			QTPropertyValueType	propType = kQTMetaDataTypeBinary;
			QTPropertyValuePtr	propValuePtr = NULL;

			QTCopyMovieMetaData([theMovie quickTimeMovie], &curMetadataContainer);
			if (curMetadataContainer != NULL)
			{
				if (QTMetaDataGetNextItem(curMetadataContainer, kQTMetaDataStorageFormatQuickTime, 
					curMetadataItem, kQTMetaDataKeyFormatQuickTime, (UInt8*)keyName, (ByteCount)strlen(keyName), &curMetadataItem) == noErr)
					if (QTMetaDataGetItemPropertyInfo(curMetadataContainer, curMetadataItem, kPropertyClass_MetaDataItem, 
						kQTMetaDataItemPropertyID_Value, &propType, &propValueSize, NULL) == noErr)
					{
						// alloc a buffer
						if ((propValuePtr = malloc(propValueSize+4)) != NULL)
						{
							// clear the pointer (just to be safe)
							memset(propValuePtr, 0, (propValueSize+4));

							// retrieve the value
							if (QTMetaDataGetItemProperty(curMetadataContainer, curMetadataItem, kPropertyClass_MetaDataItem, 
								kQTMetaDataItemPropertyID_Value, propValueSize, propValuePtr, &propValueSizeUsed) == noErr)
									resultString = [NSString stringWithCString:propValuePtr encoding:NSUTF8StringEncoding];

							// cleanup
							free(propValuePtr);
							propValuePtr = NULL;
						}
					}

				QTMetaDataRelease(curMetadataContainer);
				curMetadataContainer = NULL;
			}
		}

	// return results
	return (resultString);
}

@end
