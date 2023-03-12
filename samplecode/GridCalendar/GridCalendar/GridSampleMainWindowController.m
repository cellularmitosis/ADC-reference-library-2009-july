/*

File: GridSampleMainWindowController.m

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

#import "GridSampleMainWindowController.h"
#import "GridSampleApplicationDelegate.h"
#import "GridSampleConnectionController.h"
#import "GridSampleLoginController.h"
#import "GridSampleToolbarController.h"
#import "GridSampleNewJobWindowController.h"
#import "GridSampleJobInfoWindowController.h"
#import "GridSampleJobResultsWindowController.h"

@interface GridSampleMainWindowController (Private)

- (NSArray *)selectedJobs;
- (void)generateJobSummary;

@end

@implementation GridSampleMainWindowController

+ (id)mainWindowController;
{
	return [[[self alloc] init] autorelease];
}

- (id)init;
{
	self = [super initWithWindowNibName:@"MainWindow"];
	
	if (self != nil) {
	
		_loginController = [[GridSampleLoginController alloc] initWithMainWindowController:self];
		_newJobWindowControllers = [[NSMutableArray alloc] init];
		_jobInfoWindowControllers = [[NSMutableArray alloc] init];
		_jobResultsWindowControllers = [[NSMutableArray alloc] init];
		_windowTitle = [[[NSApp delegate] displayName] copy];
	}
	
	return self;
}

- (void)dealloc;
{
    [[NSNotificationCenter defaultCenter] removeObserver:self];
	
	[self removeObserver:self forKeyPath:@"jobsArrayController.arrangedObjects"];

	[_loginController release];
	[_newJobWindowControllers release];
	[_jobInfoWindowControllers release];
	[_jobResultsWindowControllers release];
	[_windowTitle release];
	[_gridControllerObjectController release];
	[_gridsArrayController release];
	[_jobsArrayController release];
	
	[super dealloc];
}

#pragma mark *** Window loading ***

- (void)windowDidLoad;
{
	[self addObserver:self forKeyPath:@"jobsArrayController.arrangedObjects" options:0 context:NULL];

	_toolbarController = [[GridSampleToolbarController alloc] init];
	
	[[self window] setToolbar:[_toolbarController toolbar]];
}

#pragma mark *** Window delegate methods ***

- (void)windowWillClose:(NSNotification *)notification;
{
	[_ownerObjectController setContent:nil];
	
	[NSApp terminate:self];
}

#pragma mark *** Accessors ***

- (void)setConnectionController:(GridSampleConnectionController *)connectionController;
{
	if (_connectionController != connectionController) {
	
		[_connectionController autorelease];
		_connectionController = [connectionController retain];
	}
}

- (GridSampleConnectionController *)connectionController;
{
	return _connectionController;
}

- (GridSampleLoginController *)loginController;
{
	return _loginController;
}

- (unsigned int)countOfNewJobWindowControllers;
{
     return [_newJobWindowControllers count]; 
}

- (GridSampleNewJobWindowController *)objectInNewJobWindowControllersAtIndex:(unsigned int)index;
{
	return [_newJobWindowControllers objectAtIndex:index];
}

- (void)insertObject:(GridSampleNewJobWindowController *)newJobWindowController inNewJobWindowControllersAtIndex:(unsigned int)index;
{
	[_newJobWindowControllers insertObject:newJobWindowController atIndex:index];
}

- (void)removeObjectFromNewJobWindowControllersAtIndex:(unsigned int)index;
{
	[_newJobWindowControllers removeObjectAtIndex:index];
}

- (void)addNewJobWindowController:(GridSampleNewJobWindowController *)newJobWindowController;
{
	if ([_newJobWindowControllers containsObject:newJobWindowController] == NO) {
	
		[self insertObject:newJobWindowController inNewJobWindowControllersAtIndex:[_newJobWindowControllers count]];
	
		[newJobWindowController setMainWindowController:self];
	}
}

- (void)removeNewJobWindowController:(GridSampleNewJobWindowController *)newJobWindowController;
{
	unsigned int index = [_newJobWindowControllers indexOfObject:newJobWindowController];
	
	if (index != NSNotFound) {
	
		[newJobWindowController setMainWindowController:nil];
		
		[self removeObjectFromNewJobWindowControllersAtIndex:index];
	}
}

- (unsigned int)countOfJobInfoWindowControllers;
{
     return [_jobInfoWindowControllers count]; 
}

- (GridSampleJobInfoWindowController *)objectInJobInfoWindowControllersAtIndex:(unsigned int)index;
{
	return [_jobInfoWindowControllers objectAtIndex:index];
}

- (void)insertObject:(GridSampleJobInfoWindowController *)jobInfoWindowController inJobInfoWindowControllersAtIndex:(unsigned int)index;
{
	[_jobInfoWindowControllers insertObject:jobInfoWindowController atIndex:index];
}

- (void)removeObjectFromJobInfoWindowControllersAtIndex:(unsigned int)index;
{
	[_jobInfoWindowControllers removeObjectAtIndex:index];
}

- (void)addJobInfoWindowController:(GridSampleJobInfoWindowController *)jobInfoWindowController;
{
	if ([_jobInfoWindowControllers containsObject:jobInfoWindowController] == NO) {
	
		[self insertObject:jobInfoWindowController inJobInfoWindowControllersAtIndex:[_jobInfoWindowControllers count]];
	
		[jobInfoWindowController setMainWindowController:self];
	}
}

- (GridSampleJobInfoWindowController *)jobInfoWindowControllerForJob:(XGJob *)job;
{
	NSEnumerator *jobInfoWindowControllerEnumerator = [_jobInfoWindowControllers objectEnumerator];
	
	GridSampleJobInfoWindowController *jobInfoWindowController = nil;
	
	while (jobInfoWindowController = [jobInfoWindowControllerEnumerator nextObject]) {
	
		if ([jobInfoWindowController job] == job) break;
	}
	
	return jobInfoWindowController;
}

- (void)removeJobInfoWindowController:(GridSampleJobInfoWindowController *)jobInfoWindowController;
{
	unsigned int index = [_jobInfoWindowControllers indexOfObject:jobInfoWindowController];
	
	if (index != NSNotFound) {
	
		[jobInfoWindowController setMainWindowController:nil];
		
		[self removeObjectFromJobInfoWindowControllersAtIndex:index];
	}
}

- (unsigned int)countOfJobResultsWindowControllers;
{
     return [_jobResultsWindowControllers count]; 
}

- (GridSampleJobResultsWindowController *)objectInJobResultsWindowControllersAtIndex:(unsigned int)index;
{
	return [_jobResultsWindowControllers objectAtIndex:index];
}

- (void)insertObject:(GridSampleJobResultsWindowController *)jobResultsWindowController inJobResultsWindowControllersAtIndex:(unsigned int)index;
{
	[_jobResultsWindowControllers insertObject:jobResultsWindowController atIndex:index];
}

- (void)removeObjectFromJobResultsWindowControllersAtIndex:(unsigned int)index;
{
	[_jobResultsWindowControllers removeObjectAtIndex:index];
}

- (void)addJobResultsWindowController:(GridSampleJobResultsWindowController *)jobResultsWindowController;
{
	if ([_jobResultsWindowControllers containsObject:jobResultsWindowController] == NO) {
	
		[self insertObject:jobResultsWindowController inJobResultsWindowControllersAtIndex:[_jobResultsWindowControllers count]];
	
		[jobResultsWindowController setMainWindowController:self];
	}
}

- (GridSampleJobResultsWindowController *)jobResultsWindowControllerForJob:(XGJob *)job;
{
	NSEnumerator *jobResultsWindowControllerEnumerator = [_jobResultsWindowControllers objectEnumerator];
	
	GridSampleJobResultsWindowController *jobResultsWindowController = nil;
	
	while (jobResultsWindowController = [jobResultsWindowControllerEnumerator nextObject]) {
	
		if ([jobResultsWindowController job] == job) break;
	}
	
	return jobResultsWindowController;
}

- (void)removeJobResultsWindowController:(GridSampleJobResultsWindowController *)jobResultsWindowController;
{
	unsigned int index = [_jobResultsWindowControllers indexOfObject:jobResultsWindowController];
	
	if (index != NSNotFound) {
	
		[jobResultsWindowController setMainWindowController:nil];
		
		[self removeObjectFromJobResultsWindowControllersAtIndex:index];
	}
}

#pragma mark *** UI Actions ***

- (BOOL)validateUserInterfaceItem:(id <NSValidatedUserInterfaceItem>)userInterfaceItem;
{
	SEL action = [userInterfaceItem action];
	
	if (action == @selector(disconnect:)) {
	
		return YES;
	}
	else if (action == @selector(newJob:)) {
	
		return YES;
	}
	else if (action == @selector(cloneJob:)) {
	
		return [[self selectedJobs] count] > 0;
	}
	else if (action == @selector(showInfo:)) {
	
		return [[self selectedJobs] count] > 0;
	}
	else if (action == @selector(showResults:)) {
	
		return [[self selectedJobs] count] > 0;
	}
	else if (action == @selector(cancelJob:)) {
	
		return [[self selectedJobs] count] > 0;
	}
	else if (action == @selector(deleteJob:)) {
	
		return [[self selectedJobs] count] > 0;
	}
	else {
	
		return NO;
	}
}

- (IBAction)disconnect:(id)sender;
{
	_shouldDisconnect = YES;
	
	[[[self connectionController] connection] close];
}

- (IBAction)newJob:(id)sender;
{
	GridSampleNewJobWindowController *newJobWindowController = [[[NSApp delegate] classForNewJobWindowController] newJobWindowController];
	
	[self addNewJobWindowController:newJobWindowController];
	
	[[newJobWindowController window] makeKeyAndOrderFront:self];
}

- (IBAction)cloneJob:(id)sender;
{
	NSArray *selectedJobs = [self selectedJobs];
	
	if ([selectedJobs count] == 0) {
	
		NSBeep();
		return;
	}
	
	NSEnumerator *jobEnumerator = [selectedJobs objectEnumerator];
	
	XGJob *job;
	
	while (job = [jobEnumerator nextObject]) {
		
		GridSampleNewJobWindowController *newJobWindowController = [[[NSApp delegate] classForNewJobWindowController] newJobWindowControllerWithJob:job];
		
		[self addNewJobWindowController:newJobWindowController];
		
		[[newJobWindowController window] makeKeyAndOrderFront:self];
	}
}

- (IBAction)showInfo:(id)sender;
{
	NSArray *selectedJobs = [self selectedJobs];
	
	if ([selectedJobs count] == 0) {
	
		NSBeep();
		return;
	}
	
	NSEnumerator *jobEnumerator = [selectedJobs objectEnumerator];
	
	XGJob *job;
	
	while (job = [jobEnumerator nextObject]) {
	
		GridSampleJobInfoWindowController *jobInfoWindowController = [self jobInfoWindowControllerForJob:job];
		
		if (jobInfoWindowController == nil) {
		
			jobInfoWindowController = [[[NSApp delegate] classForJobInfoWindowController] jobInfoWindowControllerWithJob:job];

			[self addJobInfoWindowController:jobInfoWindowController];
		}
		
		[[jobInfoWindowController window] makeKeyAndOrderFront:self];
	}
}

- (IBAction)showResults:(id)sender;
{
	NSArray *selectedJobs = [self selectedJobs];
	
	if ([selectedJobs count] == 0) {
	
		NSBeep();
		return;
	}
	
	NSEnumerator *jobEnumerator = [selectedJobs objectEnumerator];
	
	XGJob *job;
	
	while (job = [jobEnumerator nextObject]) {
	
		GridSampleJobResultsWindowController *jobResultsWindowController = [self jobResultsWindowControllerForJob:job];
		
		if (jobResultsWindowController == nil) {
		
			jobResultsWindowController = [[[NSApp delegate] classForJobResultsWindowController] jobResultsWindowControllerWithJob:job];

			[self addJobResultsWindowController:jobResultsWindowController];
		}
		
		[[jobResultsWindowController window] makeKeyAndOrderFront:self];
	}
}

- (IBAction)cancelJob:(id)sender;
{
	NSArray *selectedJobs = [self selectedJobs];
	
	if ([selectedJobs count] == 0) {
	
		NSBeep();
		return;
	}
	
	NSEnumerator *jobEnumerator = [selectedJobs objectEnumerator];
	
	XGJob *job;
	
	while (job = [jobEnumerator nextObject]) {
		
		XGActionMonitor *stopAction = [job performStopAction];
		
		stopAction = nil;
	}
}

- (IBAction)deleteJob:(id)sender;
{
	NSArray *selectedJobs = [self selectedJobs];
	
	if ([selectedJobs count] == 0) {
	
		NSBeep();
		return;
	}
	
	NSEnumerator *jobEnumerator = [selectedJobs objectEnumerator];
	
	XGJob *job;
	
	while (job = [jobEnumerator nextObject]) {
		
		XGActionMonitor *stopAction = [job performDeleteAction];
		
		stopAction = nil;
	}
}

#pragma mark *** Connection state change handlers ***

- (void)showConnectSheet;
{
	[[self loginController] showConnectSheet];
}

- (void)connectionDidOpen;
{
	[[self loginController] connectionDidOpen];
}

- (void)connectionDidNotOpen;
{
	[[self loginController] connectionDidNotOpen];
}

- (void)connectionWasCanceled;
{
	[[self loginController] connectionWasCanceled];
}

- (void)connectionAuthenticationNeeded;
{
	[[self loginController] connectionAuthenticationNeeded];
}

- (void)connectionAuthenticationFailed;
{
	[[self loginController] connectionAuthenticationFailed];
}

- (void)connectionDidClose;
{
	if (_shouldDisconnect == NO) {
	
		[[self loginController] showConnectionDidCloseSheet];
	}
	else {
	
		_shouldDisconnect = NO;
		
		[[self loginController] showConnectSheet];
	}
	
}

#pragma mark *** Convenience methods ***

- (NSArray *)selectedJobs;
{
	return [_jobsArrayController selectedObjects];
}

#pragma mark *** Key-value observing ***

- (void)generateJobSummary;
{
	NSArray *jobsArrangedObjects = [_jobsArrayController arrangedObjects];
	
	int count = [jobsArrangedObjects count];
	
	char *plural = (count == 1 ? "" : "s");
	
	NSString *jobSummary = [NSString stringWithFormat:@"%d job%s", count, plural];
	
	[self setValue:jobSummary forKey:@"jobSummary"];
}

- (void)observeValueForKeyPath:(NSString *)keyPath ofObject:(id)object change:(NSDictionary *)change context:(void *)context;
{
	if ([keyPath isEqualToString:@"jobsArrayController.arrangedObjects"] == YES) {
	
		[self generateJobSummary];
	}
	else {
	
		[super observeValueForKeyPath:keyPath ofObject:object change:change context:context];
	}
}

@end
