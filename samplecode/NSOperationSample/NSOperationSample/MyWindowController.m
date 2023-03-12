/*

File: MyWindowController.m

Abstract: Source file for this sample's main NSWindowController "TestWindow".

Version: 1.0

Disclaimer: IMPORTANT:  This Apple software is supplied to you by Apple Inc.
("Apple") in consideration of your agreement to the
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

Copyright © 2006-2007 Apple Inc., All Rights Reserved

*/

#import "MyWindowController.h"
#import "GetPathsOperation.h"
#import "LoadOperation.h"

// -------------------------------------------------------------------------------
@interface MyWindowController (Private)

- (void)loadFilePaths:(NSString*)fromPath;
- (void)setResultsString:(NSString*)string;
- (void)updateProgress:(NSTimer*)t;
- (NSTimer*)timer;
- (void)setTimer:(NSTimer*)value;
- (void)updateCountIndicator;

@end


@implementation MyWindowController

// -------------------------------------------------------------------------------
//	awakeFromNib:
// -------------------------------------------------------------------------------
- (void)awakeFromNib
{	
	// register for the notification when an image file has been loaded by the NSOperation: "LoadOperation"
	[[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(anyThread_handleLoadedImages:) name:LoadImageDidFinish object:nil];
	
	// make sure double-click on a table row calls "doubleClickAction"
	[myTableView setTarget:self];
	[myTableView setDoubleAction:@selector(doubleClickAction:)];
}

// -------------------------------------------------------------------------------
//	init:
// -------------------------------------------------------------------------------
- (id)init
{
	[super init];
	
	queue = [[NSOperationQueue alloc] init];
	tableRecords = [[NSMutableArray alloc] init];
	
	return self;
}

// -------------------------------------------------------------------------------
//	dealloc:
// -------------------------------------------------------------------------------
-(void)dealloc
{
	[tableRecords release];
	[queue release];
	[super dealloc];
}

// -------------------------------------------------------------------------------
//	updateCountIndicator:
//
//	Canned routine for updating the number of items in the table (used in several places).
// -------------------------------------------------------------------------------
-(void)updateCountIndicator
{
	// set the number of images found indicator string
	NSString* resultStr = [NSString stringWithFormat:@"Images found: %ld", [tableRecords count]];
	[self setResultsString: resultStr];
}

// -------------------------------------------------------------------------------
//	applicationShouldTerminateAfterLastWindowClosed:sender
//
//	NSApplication delegate method placed here so the sample conveniently quits
//	after we close the window.
// -------------------------------------------------------------------------------
- (BOOL)applicationShouldTerminateAfterLastWindowClosed:(NSApplication*)sender
{
	return YES;
}

// -------------------------------------------------------------------------------
//	mainThread_handleLoadedImages:note
//
//	The method used to modify the table's data source on the main thread.
//	This will cause the table to update itself once the NSArrayController is changed.
//
//	The notification contains an NSDictionary containing the image file's info
//	to add to the table view.
// -------------------------------------------------------------------------------
- (void)mainThread_handleLoadedImages:(NSNotification*)note
{
    // Pending NSNotifications can possibly back up while waiting to be executed,
	// and if the user stops the queue, we may have left-over pending
	// notifications to process.
	//
	// So make sure we have "active" running NSOperations in the queue
	// if we are to continuously add found image files to the table view.
	// Otherwise, we let any remaining notifications drain out.
	//
	if ([myStopButton isEnabled])
	{
		[tableRecords addObject:[note userInfo]];
		[myTableView reloadData];
		
		// set the number of images found indicator string
		[self updateCountIndicator];
	}
}

// -------------------------------------------------------------------------------
//	anyThread_handleLoadedImages:note
//
//	This method is called from any possible thread (any NSOperation) used to 
//	update our table view and its data source.
//	
//	The notification contains the NSDictionary containing the image file's info
//	to add to the table view.
// -------------------------------------------------------------------------------
- (void)anyThread_handleLoadedImages:(NSNotification*)note
{
	// update our table view on the main thread
	[self performSelectorOnMainThread:@selector(mainThread_handleLoadedImages:) withObject:note waitUntilDone:NO];
}

// -------------------------------------------------------------------------------
//	alertDidEnd:
// -------------------------------------------------------------------------------
-(void)alertDidEnd:(NSAlert*)alert returnCode:(NSInteger)returnCode contextInfo:(void*)contextInfo
{}

// -------------------------------------------------------------------------------
//	windowShouldClose:
// -------------------------------------------------------------------------------
- (BOOL)windowShouldClose:(id)sender
{
	// are you sure you want to close, (threads running)
	NSInteger numOperationsRunning = [[queue operations] count];
	
	if (numOperationsRunning > 0)
	{
		NSAlert* alert = [NSAlert alertWithMessageText: @"Image files are currently loading."
									defaultButton: @"OK"
								  alternateButton: nil
									  otherButton: nil
						informativeTextWithFormat: @"Please click the \"Stop\" button before closing."];
		[alert beginSheetModalForWindow: [self window] modalDelegate:self didEndSelector:@selector(alertDidEnd:returnCode:contextInfo:) contextInfo:nil];
	}
	
	return (numOperationsRunning == 0);
}

// -------------------------------------------------------------------------------
//	loadFilePaths:fromPath
// -------------------------------------------------------------------------------
-(void)loadFilePaths:(NSString*)fromPath
{
	[queue cancelAllOperations];
	
	// start the GetPathsOperation with the root path to start the search
	GetPathsOperation* getPathsOp = [[GetPathsOperation alloc] initWithRootPath:fromPath operationClass:[LoadOperation class] queue:queue];
	
	[queue addOperation: getPathsOp];	// this will start the "GetPathsOperation"
	[getPathsOp release];
}

// -------------------------------------------------------------------------------
//	setResultsString:string
// -------------------------------------------------------------------------------
-(void)setResultsString:(NSString*)string
{
	[self willChangeValueForKey:@"imagesFoundStr"];
	imagesFoundStr = [NSMutableString stringWithString:string];
	[self didChangeValueForKey:@"imagesFoundStr"];
}


#pragma mark - Actions

// -------------------------------------------------------------------------------
//	doubleClickAction:sender
//
//	Inspect our selected objects (user double-clicked them).
// -------------------------------------------------------------------------------
- (void)doubleClickAction:(id)sender
{
	NSTableView* theTableView = (NSTableView*)sender;
	NSInteger selectedRow = [theTableView selectedRow];
	if (selectedRow != -1)
	{
		NSDictionary* objectDict = [tableRecords objectAtIndex: selectedRow];
		if (objectDict != nil)
		{
			NSString* pathStr = [objectDict valueForKey: @"path"];
			NSString* completeURLStr = [pathStr stringByAppendingPathComponent:[objectDict valueForKey: @"name"]];
			
			[[NSWorkspace sharedWorkspace] openURL:[NSURL fileURLWithPath:completeURLStr]];
		}
	}
}

// -------------------------------------------------------------------------------
//	stopAction:sender
// -------------------------------------------------------------------------------
- (IBAction)stopAction:(id)sender
{
	[queue cancelAllOperations];
		
	[myStopButton setEnabled:NO];
	[myStartButton setEnabled:YES];

	[myProgressInd setHidden:YES];
	[myProgressInd stopAnimation:self];
	
	[self updateCountIndicator];
}

// -------------------------------------------------------------------------------
//	chooseDidEnd:panel:returnCode:contextInfo
// -------------------------------------------------------------------------------
-(void)chooseDidEnd:(NSOpenPanel*)panel returnCode:(int)returnCode contextInfo:(void*)contextInfo
{
	[panel orderOut:self];
	[panel release];
	
	if (returnCode == NSFileHandlingPanelOKButton)
	{
		// user has chosen a directory, start finding image files:
		
		[tableRecords removeAllObjects];	// clear the table data
		[self updateCountIndicator];
		
		[myStopButton setEnabled:YES];
		[myStartButton setEnabled:NO];
		
		[myProgressInd setHidden:NO];
		[myProgressInd startAnimation:self];
		
		[self loadFilePaths:[[panel URL] path]];	// start the file search NSOperation
		
		// schedule our update timer for the spinning gear control
        [self setTimer: [NSTimer scheduledTimerWithTimeInterval: 1.0
                                                         target: self
                                                       selector: @selector(updateProgress:)
                                                       userInfo: nil
                                                        repeats: YES]];
	}
}

// -------------------------------------------------------------------------------
//	startAction:sender
// -------------------------------------------------------------------------------
- (IBAction)startAction:(id)sender
{	
	NSOpenPanel* openPanel = [[NSOpenPanel alloc] init];
	
	[openPanel setResolvesAliases:YES];
	[openPanel setCanChooseDirectories:YES];
	[openPanel setAllowsMultipleSelection:NO];
	[openPanel setCanChooseFiles:NO];
	[openPanel setPrompt:@"Choose"];
	[openPanel setMessage:@"Choose a directory that has a large number image files:"];
	[openPanel setTitle:@"Choose"];
	
	[openPanel beginSheetForDirectory:nil file:nil types:nil modalForWindow:[self window] modalDelegate:self didEndSelector:@selector(chooseDidEnd:returnCode:contextInfo:) contextInfo:nil];
}


#pragma mark - Timer Support

// -------------------------------------------------------------------------------
//	updateProgress:timer
// -------------------------------------------------------------------------------
-(void)updateProgress:(NSTimer*)t
{
	if ([[queue operations] count] == 0)
	{
		[t invalidate];
		[self setTimer: nil];
		
		[myProgressInd stopAnimation:self];
		[myProgressInd setHidden:YES];
		[myStopButton setEnabled:NO];
		[myStartButton setEnabled:YES];
		
		// set the number of images found indicator string
		[self updateCountIndicator];
	}
}

// -------------------------------------------------------------------------------
//	timer:
// -------------------------------------------------------------------------------
- (NSTimer*)timer
{
    return [[timer retain] autorelease];
}

// -------------------------------------------------------------------------------
//	setTimer:value
// -------------------------------------------------------------------------------
- (void)setTimer:(NSTimer*)value
{
    if (timer != value)
	{
        [timer release];
        timer = [value retain];
    }
}

#pragma mark - Data Source

// -------------------------------------------------------------------------------
//	numberOfRowsInTableView:aTableView
// -------------------------------------------------------------------------------
- (int)numberOfRowsInTableView:(NSTableView*)aTableView
{
	return [tableRecords count];
}

// -------------------------------------------------------------------------------
//	objectValueForTableColumn:aTableColumn:row
// -------------------------------------------------------------------------------
- (id)tableView:(NSTableView *)aTableView
      objectValueForTableColumn:(NSTableColumn *)aTableColumn
      row:(int)rowIndex
{
    id theRecord, theValue;
    
    theRecord = [tableRecords objectAtIndex:rowIndex];
    theValue = [theRecord objectForKey:[aTableColumn identifier]];
    
    return theValue;
}

// -------------------------------------------------------------------------------
//	sortWithDescriptor:descriptor
// -------------------------------------------------------------------------------
- (void)sortWithDescriptor:(id)descriptor
{
	NSMutableArray* sorted = [[NSMutableArray alloc] initWithCapacity:1];
	[sorted addObjectsFromArray:[tableRecords sortedArrayUsingDescriptors:[NSArray arrayWithObject:descriptor]]];
	[tableRecords removeAllObjects];
	[tableRecords addObjectsFromArray:sorted];
	[myTableView reloadData];
	[sorted release];
}

// -------------------------------------------------------------------------------
//	didClickTableColumn:tableColumn
// -------------------------------------------------------------------------------
- (void)tableView: (NSTableView*)inTableView didClickTableColumn:(NSTableColumn*)tableColumn
{	
	NSArray* allColumns=[inTableView tableColumns];
	NSInteger i;
	for (i=0; i<[inTableView numberOfColumns]; i++) 
	{
		if ([allColumns objectAtIndex:i]!=tableColumn)
		{
			[inTableView setIndicatorImage:nil inTableColumn:[allColumns objectAtIndex:i]];
		}
	}
	[inTableView setHighlightedTableColumn:tableColumn];
	
	if ([inTableView indicatorImageInTableColumn:tableColumn]!=[NSImage imageNamed:@"NSAscendingSortIndicator"])
	{
		[inTableView setIndicatorImage:[NSImage imageNamed:@"NSAscendingSortIndicator"] inTableColumn:tableColumn];  
		[self sortWithDescriptor:[[NSSortDescriptor alloc] initWithKey:[tableColumn identifier] ascending:YES]];
	}
	else {
		[inTableView setIndicatorImage:[NSImage imageNamed:@"NSDescendingSortIndicator"] inTableColumn:tableColumn];
		[self sortWithDescriptor:[[NSSortDescriptor alloc] initWithKey:[tableColumn identifier] ascending:NO]];
	}
}

@end
