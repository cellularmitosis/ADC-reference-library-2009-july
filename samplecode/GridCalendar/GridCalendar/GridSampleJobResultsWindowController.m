/*

File: GridSampleJobResultsWindowController.m

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

Copyright 2005 Apple Computer, Inc., All Rights Reserved

*/ 

#import "GridSampleJobResultsWindowController.h"
#import "GridSampleMainWindowController.h"

static BOOL createOrVerifyDirectoryPath(NSString *directoryPath)
{
    if (directoryPath == nil || [directoryPath isEqualToString:@""] == YES) {
        
        return NO;
    }
    
    NSFileManager *fm = [NSFileManager defaultManager];
    
    BOOL isDirectory = NO;
    
    BOOL doesExist = NO;
    
    doesExist = [fm fileExistsAtPath:directoryPath isDirectory:&isDirectory];
    
    if (doesExist == YES) {
        
        return isDirectory;
    }
    
    NSString *parentPath = [directoryPath stringByDeletingLastPathComponent];
    
    if ([parentPath isEqualToString:@"/"] == NO) {
        
        BOOL didSucceed = createOrVerifyDirectoryPath(parentPath);
        
        if (didSucceed == YES) {
            
            didSucceed = [fm createDirectoryAtPath:directoryPath attributes:nil];
        }
        
        return didSucceed;
    }
    
    return YES;
}

static NSString * const TaskIdentifierKey = @"taskIdentifier";
static NSString * const StandardOutputStreamDownloadKey = @"standardOutputStreamDownload";
static NSString * const StandardOutputStreamDataKey = @"standardOutputStreamData";
static NSString * const StandardOutputStreamStringKey = @"standardOutputStreamString";
static NSString * const StandardErrorStreamDownloadKey = @"standardErrorStreamDownload";
static NSString * const StandardErrorStreamDataKey = @"standardErrorStreamData";
static NSString * const StandardErrorStreamStringKey = @"standardErrorStreamString";
static NSString * const OutputFilesKey = @"outputFiles";

@implementation GridSampleJobResultsWindowController

+ (id)jobResultsWindowControllerWithJob:(XGJob *)job;
{
	return [[[self alloc] initWithJob:job] autorelease];
}

- (id)initWithJob:(XGJob *)job;
{
	self = [super initWithWindowNibName:@"JobResults"];
	
	if (self != nil) {
		
		_taskOutputs = [[NSMutableArray alloc] init];
		_taskOutputsByIdentifier = [[NSMutableDictionary alloc] init];
	
		[self setJob:job];
	}
	
	return self;
}

- (void)dealloc;
{
    [[NSNotificationCenter defaultCenter] removeObserver:self];
	
	[_streamsActionMonitor removeObserver:self forKeyPath:@"outcome"];
	[_filesActionMonitor removeObserver:self forKeyPath:@"outcome"];
	
	[_job release];
	
	[_streamsActionMonitor release];
	
	[_filesActionMonitor release];
	[_taskOutputs release];
	[_taskOutputsByIdentifier release];
	[_selectedTaskIdentifier release];
	[_outputFiles release];
	
	[super dealloc];
}

#pragma mark *** Window delegate methods ***

- (void)windowDidLoad;
{
	[super windowDidLoad];
	
	NSSortDescriptor *sortDescriptor = [[[NSSortDescriptor alloc] initWithKey:@"taskIdentifier" ascending:YES selector:@selector(compareNumerically:)] autorelease];
	
	NSArray *sortDescriptors = [NSArray arrayWithObjects:sortDescriptor, nil];
	
	[_taskOutputsArrayController setSortDescriptors:sortDescriptors];
}
	
- (void)windowWillClose:(NSNotification *)notification;
{
	[_ownerObjectController setContent:nil];

	[self retain];
	[[self mainWindowController] removeJobResultsWindowController:self];
	[self autorelease];
}

#pragma mark *** Accessors ***

- (void)setMainWindowController:(GridSampleMainWindowController *)mainWindowController;
{
	_mainWindowController = mainWindowController;
}

- (GridSampleMainWindowController *)mainWindowController;
{
	return _mainWindowController;
}

- (void)setJob:(XGJob *)job;
{
	if (_job != job) {
	
		[_job autorelease];
		_job = [job retain];
				
		_streamsActionMonitor = [[job performGetOutputStreamsAction] retain];
		[_streamsActionMonitor addObserver:self forKeyPath:@"outcome" options:0 context:NULL];

		_filesActionMonitor = [[job performGetOutputFilesAction] retain];
		[_filesActionMonitor addObserver:self forKeyPath:@"outcome" options:0 context:NULL];
	}
}

- (XGJob *)job;
{
	return _job;
}

- (unsigned int)countOfTaskOutputs;
{
	return [_taskOutputs count]; 
}
- (NSDictionary *)objectInTaskOutputsAtIndex:(unsigned int)index;
{
	return [_taskOutputs objectAtIndex:index];
}

- (void)insertObject:(NSDictionary *)taskOutput inTaskOutputsAtIndex:(unsigned int)index;
{
	[_taskOutputs insertObject:taskOutput atIndex:index];
}

- (void)removeObjectFromTaskOutputsAtIndex:(unsigned int)index;
{
	[_taskOutputs removeObjectAtIndex:index];
}

- (void)replaceObjectInTaskOutputsAtIndex:(unsigned int)index withObject:(NSDictionary *)taskOutput;
{
	[_taskOutputs replaceObjectAtIndex:index withObject:taskOutput];
}

- (void)setSelectedTaskIdentifier:(NSString *)selectedTaskIdentifier;
{
	[_selectedTaskIdentifier autorelease];
	_selectedTaskIdentifier = [selectedTaskIdentifier retain];
}

- (NSString *)selectedTaskIdentifier;
{
	return _selectedTaskIdentifier;
}

- (void)setDidLoad:(BOOL)didLoad;
{
	_didLoad = didLoad;
	
	if (_didLoad == YES) {
		
		[_taskOutputsArrayController setSelectionIndex:0];
	}
}

- (BOOL)didLoad;
{
	return _didLoad;
}

- (int)countOfStreamDownloads;
{
	int countOfStreamDownloads = 0;
	
	NSEnumerator *taskOutputEnumerator = [_taskOutputs objectEnumerator];
	NSDictionary *taskOutput;
	
	while (taskOutput = [taskOutputEnumerator nextObject]) {
		
		if ([taskOutput objectForKey:StandardOutputStreamDownloadKey] != nil) countOfStreamDownloads++;
		if ([taskOutput objectForKey:StandardErrorStreamDownloadKey] != nil) countOfStreamDownloads++;
	}
	
	return countOfStreamDownloads;
}

- (NSMutableDictionary *)taskOutputWithTaskIdentifier:(NSString *)taskIdentifier;
{
	NSMutableDictionary *taskOutput = [_taskOutputsByIdentifier objectForKey:taskIdentifier];
	
	if (taskOutput == nil) {
		
		taskOutput = [NSMutableDictionary dictionary];
		
		[taskOutput setObject:taskIdentifier forKey:TaskIdentifierKey];
		
		[_taskOutputsByIdentifier setObject:taskOutput forKey:taskIdentifier];
		
		[self insertObject:taskOutput inTaskOutputsAtIndex:[self countOfTaskOutputs]];
	}
	
	return taskOutput;
}

- (NSMutableArray *)outputFilesWithTaskIdentifier:(NSString *)taskIdentifier;
{
	NSMutableDictionary *taskOutput = [self taskOutputWithTaskIdentifier:taskIdentifier];
	
	if ([taskOutput objectForKey:OutputFilesKey] == nil) {
		
		[taskOutput setObject:[NSMutableArray array] forKey:OutputFilesKey];
	}
	
	return [taskOutput mutableArrayValueForKey:OutputFilesKey];
}

#pragma mark *** Convenience methods ***

- (void)handleOutputStreams:(NSArray *)outputStreams;
{
	NSEnumerator *outputStreamEnumerator = [outputStreams objectEnumerator];
	
	XGFile *outputStream;
	
	while (outputStream = [outputStreamEnumerator nextObject]) {
		
		if ([[outputStream path] isEqualToString:XGFileStandardOutputPath] == YES) {
			
			NSString *taskIdentifier = [outputStream taskIdentifier];
			
			NSMutableDictionary *taskOutput = [self taskOutputWithTaskIdentifier:taskIdentifier];
			
			XGFileDownload *standardOutputStreamDownload = [[[XGFileDownload alloc] initWithFile:outputStream delegate:self] autorelease];
			
			[taskOutput setObject:standardOutputStreamDownload forKey:StandardOutputStreamDownloadKey];
		}
		else if ([[outputStream path] isEqualToString:XGFileStandardErrorPath] == YES) {
		
			NSString *taskIdentifier = [outputStream taskIdentifier];
			
			NSMutableDictionary *taskOutput = [self taskOutputWithTaskIdentifier:taskIdentifier];
			
			XGFileDownload *standardErrorStreamDownload = [[[XGFileDownload alloc] initWithFile:outputStream delegate:self] autorelease];
			
			[taskOutput setObject:standardErrorStreamDownload forKey:StandardErrorStreamDownloadKey];
		}
	}

	if ([self countOfStreamDownloads] == 0 && _filesActionMonitor == nil) {
	
		[self setDidLoad:YES];
		
		// [self setStandardOutputString:@"There was no output to load."];
	}
}

- (void)handleOutputFiles:(NSArray *)outputFiles;
{
	NSEnumerator *outputFileEnumerator = [outputFiles objectEnumerator];
	XGFile *outputFile;
	
	while (outputFile = [outputFileEnumerator nextObject]) {
		
		NSString *taskIdentifier = [outputFile taskIdentifier];
		
		NSMutableArray *outputFiles = [self outputFilesWithTaskIdentifier:taskIdentifier];
				
		[outputFiles addObject:outputFile];
	}

	if (_streamsActionMonitor == nil && [self countOfStreamDownloads] == 0 && _filesActionMonitor == nil) {
	
		[self setDidLoad:YES];
	}
}

#pragma mark *** Action methods ***

- (void)saveFiles:(NSArray *)files toDirectory:(NSString *)directory;
{
	NSEnumerator *fileEnumerator = [files objectEnumerator];
	
	XGFile *file;
	
	while (file = [fileEnumerator nextObject]) {
		
		XGFileDownload *fileDownload = [[XGFileDownload alloc] initWithFile:file delegate:self];
		
		NSString *destination = [directory stringByAppendingPathComponent:[[fileDownload file] path]];
		
		destination = [destination stringByExpandingTildeInPath];
		
		[fileDownload setDestination:destination allowOverwrite:NO];
	}
}

- (void)showChooseJobResultsDestinationSheetWithFiles:(NSArray *)files;
{
    NSOpenPanel *openPanel = [NSOpenPanel openPanel];
	
    [openPanel setCanChooseDirectories:YES];
    [openPanel setCanChooseFiles:NO];
    [openPanel setAllowsMultipleSelection:NO];
    [openPanel setPrompt:@"Choose Destination"];
    [openPanel setCanCreateDirectories:YES];
	
    [openPanel beginSheetForDirectory:nil
								 file:nil
								types:nil
					   modalForWindow:[self window]
						modalDelegate:self
					   didEndSelector:@selector(chooseJobResultsDestinationOpenPanelDidEnd:returnCode:contextInfo:)
						  contextInfo:[files retain]];
}

- (void)chooseJobResultsDestinationOpenPanelDidEnd:(NSOpenPanel *)openPanel returnCode:(int)returnCode contextInfo:(void *)contextInfo;
{
	NSArray *files = [(NSArray *)contextInfo autorelease];
	
	[openPanel orderOut:nil];
	
    if (returnCode == NSFileHandlingPanelOKButton) {
        
		NSString *directory = [openPanel filename];

		[self saveFiles:files toDirectory:directory];
    }
}

- (void)saveFiles:(NSArray *)files;
{
	[self showChooseJobResultsDestinationSheetWithFiles:files];
}

#pragma mark *** XGFileDownload delegate methods ***

- (void)fileDownloadDidBegin:(XGFileDownload *)fileDownload;
{
	if ([[fileDownload file] type] == XGFileTypeStream) {
	
		if ([[[fileDownload file] path] isEqualToString:XGFileStandardOutputPath] == YES) {
					
			NSString *taskIdentifier = [[fileDownload file] taskIdentifier];
			
			NSMutableDictionary *taskOutput = [_taskOutputsByIdentifier objectForKey:taskIdentifier];
			
			NSMutableData *standardOutputStreamData = [NSMutableData data];
			
			[taskOutput setObject:standardOutputStreamData forKey:StandardOutputStreamDataKey];
		}
		else if ([[[fileDownload file] path] isEqualToString:XGFileStandardErrorPath] == YES) {
		
			NSString *taskIdentifier = [[fileDownload file] taskIdentifier];
			
			NSMutableDictionary *taskOutput = [_taskOutputsByIdentifier objectForKey:taskIdentifier];
			
			NSMutableData *standardErrorStreamData = [NSMutableData data];
			
			[taskOutput setObject:standardErrorStreamData forKey:StandardErrorStreamDataKey];
		}
	}
}

- (void)fileDownload:(XGFileDownload *)fileDownload didReceiveAttributes:(NSDictionary *)attributes;
{
}

- (void)fileDownload:(XGFileDownload *)fileDownload decideDestinationWithSuggestedPath:(NSString *)path;
{
	if ([[fileDownload file] type] == XGFileTypeRegular) {

		createOrVerifyDirectoryPath([[fileDownload destination] stringByDeletingLastPathComponent]);
	}
}

- (void)fileDownload:(XGFileDownload *)fileDownload didCreateDestination:(NSString *)destination;
{
}

- (void)fileDownload:(XGFileDownload *)fileDownload didReceiveData:(NSData *)data;
{
	if ([[fileDownload file] type] == XGFileTypeStream) {
	
		if ([[[fileDownload file] path] isEqualToString:XGFileStandardOutputPath] == YES) {

			NSString *taskIdentifier = [[fileDownload file] taskIdentifier];
			
			NSMutableDictionary *taskOutput = [_taskOutputsByIdentifier objectForKey:taskIdentifier];
			
			NSMutableData *standardOutputStreamData = [taskOutput objectForKey:StandardOutputStreamDataKey];
			
			[standardOutputStreamData appendData:data];
		}
		else if ([[[fileDownload file] path] isEqualToString:XGFileStandardErrorPath] == YES) {
		
			NSString *taskIdentifier = [[fileDownload file] taskIdentifier];
			
			NSMutableDictionary *taskOutput = [_taskOutputsByIdentifier objectForKey:taskIdentifier];
			
			NSMutableData *standardErrorStreamData = [taskOutput objectForKey:StandardErrorStreamDataKey];
			
			[standardErrorStreamData appendData:data];
		}
	}
}

- (void)fileDownload:(XGFileDownload *)fileDownload didFailWithError:(NSError *)error;
{
	if ([[fileDownload file] type] == XGFileTypeStream) {
	
		if ([[[fileDownload file] path] isEqualToString:XGFileStandardOutputPath] == YES) {

			NSString *taskIdentifier = [[fileDownload file] taskIdentifier];
			
			NSMutableDictionary *taskOutput = [_taskOutputsByIdentifier objectForKey:taskIdentifier];
			
			[taskOutput setObject:@"The standard output failed to load." forKey:StandardOutputStreamStringKey];
		}
		else if ([[[fileDownload file] path] isEqualToString:XGFileStandardErrorPath] == YES) {
		
			NSString *taskIdentifier = [[fileDownload file] taskIdentifier];
			
			NSMutableDictionary *taskOutput = [_taskOutputsByIdentifier objectForKey:taskIdentifier];
			
			[taskOutput setObject:@"The standard error failed to load." forKey:StandardErrorStreamStringKey];
		}
		
		if ([self countOfStreamDownloads] == 0 &&
			_filesActionMonitor == nil) {
		
			[self setDidLoad:YES];
		}
	}
}

- (void)fileDownloadDidFinish:(XGFileDownload *)fileDownload;
{
	if ([[fileDownload file] type] == XGFileTypeStream) {
	
		if ([[[fileDownload file] path] isEqualToString:XGFileStandardOutputPath] == YES) {

			NSString *taskIdentifier = [[fileDownload file] taskIdentifier];
			
			NSMutableDictionary *taskOutput = [_taskOutputsByIdentifier objectForKey:taskIdentifier];

			[taskOutput removeObjectForKey:StandardOutputStreamDownloadKey];

			NSMutableData *standardOutputStreamData = [taskOutput objectForKey:StandardOutputStreamDataKey];

			NSString *standardOutputStreamString = [[[NSString alloc] initWithData:standardOutputStreamData encoding:NSUTF8StringEncoding] autorelease];
			
			if (standardOutputStreamString == nil) standardOutputStreamString = [[[NSString alloc] initWithData:standardOutputStreamData encoding:NSASCIIStringEncoding] autorelease];
			
			[taskOutput setObject:standardOutputStreamString forKey:StandardOutputStreamStringKey];
			
			[taskOutput removeObjectForKey:StandardOutputStreamDataKey];
		}
		else if ([[[fileDownload file] path] isEqualToString:XGFileStandardErrorPath] == YES) {
		
			NSString *taskIdentifier = [[fileDownload file] taskIdentifier];
			
			NSMutableDictionary *taskOutput = [_taskOutputsByIdentifier objectForKey:taskIdentifier];
			
			[taskOutput removeObjectForKey:StandardErrorStreamDownloadKey];
			
			NSMutableData *standardErrorStreamData = [taskOutput objectForKey:StandardErrorStreamDataKey];
			
			NSString *standardErrorStreamString = [[[NSString alloc] initWithData:standardErrorStreamData encoding:NSUTF8StringEncoding] autorelease];
			
			if (standardErrorStreamString == nil) standardErrorStreamString = [[[NSString alloc] initWithData:standardErrorStreamData encoding:NSASCIIStringEncoding] autorelease];
			
			[taskOutput setObject:standardErrorStreamString forKey:StandardErrorStreamStringKey];
			
			[taskOutput removeObjectForKey:StandardErrorStreamDataKey];
		}
		
		if ([self countOfStreamDownloads] == 0 &&
			_filesActionMonitor == nil) {
		
			[self setDidLoad:YES];
		}
	}
	else if ([[fileDownload file] type] == XGFileTypeRegular) {
	
		[fileDownload autorelease];
	}
}

#pragma mark *** Key-value observer method ***

- (void)observeValueForKeyPath:(NSString *)keyPath
					  ofObject:(id)object 
                        change:(NSDictionary *)change
                       context:(void *)context;
{
	if (object == _streamsActionMonitor && [keyPath isEqualToString:@"outcome"] == YES) {
		
		NSArray *outputStreams = [[_streamsActionMonitor results] objectForKey:XGActionMonitorResultsOutputStreamsKey];
		
		[_streamsActionMonitor removeObserver:self forKeyPath:@"outcome"];
		
		[_streamsActionMonitor autorelease];
		_streamsActionMonitor = nil;

		[self handleOutputStreams:outputStreams];
	}
	else if (object == _filesActionMonitor && [keyPath isEqualToString:@"outcome"] == YES) {
		
		NSArray *outputFiles = [[_filesActionMonitor results] objectForKey:XGActionMonitorResultsOutputFilesKey];
		
		[_filesActionMonitor removeObserver:self forKeyPath:@"outcome"];
		
		[_filesActionMonitor autorelease];
		_filesActionMonitor = nil;

		[self handleOutputFiles:outputFiles];
	}
	else {
		
		[super observeValueForKeyPath:keyPath ofObject:object change:change context:context];
	}
}

@end
