/*

File: GridSampleNewJobWindowController.m

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

#import "GridSampleNewJobWindowController.h"
#import "GridSampleMainWindowController.h"
#import "GridSampleConnectionController.h"

@interface GridSampleNewJobWindowController (Private)

- (void)setGetSpecificationActionMonitor:(XGActionMonitor *)actionMonitor;
- (XGActionMonitor *)getSpecificationActionMonitor;

- (void)setSubmitJobActionMonitor:(XGActionMonitor *)actionMonitor;
- (XGActionMonitor *)submitJobActionMonitor;

- (void)loadJobSpecification;

- (void)jobSpecificationDidLoad:(NSDictionary *)jobSpecification;
- (void)jobSpecificationDidNotLoad:(NSDictionary *)results;

- (void)jobDidSubmit:(NSDictionary *)results;
- (void)jobDidNotSubmit:(NSDictionary *)results;

- (XGGrid *)selectedGrid;

- (NSDictionary *)jobSpecification;

@end

@implementation GridSampleNewJobWindowController

+ (id)newJobWindowController;
{
	return [[[self alloc] init] autorelease];
}

+ (id)newJobWindowControllerWithJob:(XGJob *)job;
{
	return [[[self alloc] initWithJob:job] autorelease];
}

- (id)initWithWindowNibName:(NSString *)windowNibName;
{
	return [super initWithWindowNibName:windowNibName];
}	

- (id)init;
{
	return [self initWithWindowNibName:@"NewJob"];
}

- (id)initWithJob:(XGJob *)job;
{
	self = [self init];
	
	if (self != nil) {
	
		[self setJob:job];
	}
	
	return self;
}

- (void)dealloc;
{
    [[NSNotificationCenter defaultCenter] removeObserver:self];
	
	[_job release];
	
	[self setGetSpecificationActionMonitor:nil];
	[self setSubmitJobActionMonitor:nil];
	
	[super dealloc];
}

#pragma mark *** Window loading ***

- (void)windowDidLoad;
{
	if ([self job] != nil) {
	
		[self loadJobSpecification];
	}
}

#pragma mark *** Window delegate methods ***

- (void)windowWillClose:(NSNotification *)notification;
{
	[_ownerObjectController setContent:nil];

	[self retain];
	[[self mainWindowController] removeNewJobWindowController:self];
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
	}
}

- (XGJob *)job;
{
	return _job;
}

- (void)setGetSpecificationActionMonitor:(XGActionMonitor *)actionMonitor;
{
	if (_getSpecificationActionMonitor != actionMonitor) {
		
		[_getSpecificationActionMonitor removeObserver:self forKeyPath:@"outcome"];
		
		[_getSpecificationActionMonitor autorelease];
		_getSpecificationActionMonitor = [actionMonitor retain];

		[_getSpecificationActionMonitor addObserver:self forKeyPath:@"outcome" options:0 context:NULL];
	}
}

- (XGActionMonitor *)getSpecificationActionMonitor;
{
	return _getSpecificationActionMonitor;
}

- (void)setSubmitJobActionMonitor:(XGActionMonitor *)actionMonitor;
{
	if (_submitJobActionMonitor != actionMonitor) {
		
		[_submitJobActionMonitor removeObserver:self forKeyPath:@"outcome"];
		
		[_submitJobActionMonitor autorelease];
		_submitJobActionMonitor = [actionMonitor retain];

		[_submitJobActionMonitor addObserver:self forKeyPath:@"outcome" options:0 context:NULL];
	}
}

- (XGActionMonitor *)submitJobActionMonitor;
{
	return _submitJobActionMonitor;
}

#pragma mark *** UI actions ***

- (IBAction)submitJob:(id)sender;
{
    if ([[(NSView *)sender window] makeFirstResponder:nil] == NO) {
	
        NSBeep();
        return;
    } 
    
	[self setValue:@"Submitting new job..." forKey:@"statusMessage"];
	[self setValue:[NSNumber numberWithBool:YES] forKey:@"isLoading"];

	NSDictionary *jobSpecification = [self jobSpecification];
	
	if (jobSpecification != nil) {

		NSString *gridIdentifier = [[self selectedGrid] identifier];
		
		XGActionMonitor *actionMonitor = [[[[self mainWindowController] connectionController] controller] performSubmitJobActionWithJobSpecification:jobSpecification gridIdentifier:gridIdentifier];
	
		[self setSubmitJobActionMonitor:actionMonitor];
	}
	else {
		
		[self setValue:@"Submission failed due to a local error" forKey:@"statusMessage"];
		[self setValue:[NSNumber numberWithBool:NO] forKey:@"isLoading"];
		
		NSBeep();
	}
}

#pragma mark *** Convenience methods ***

- (void)loadJobSpecification;
{	
	[self setValue:@"Loading job specification..." forKey:@"statusMessage"];
	[self setValue:[NSNumber numberWithBool:YES] forKey:@"isLoading"];
	
	XGActionMonitor *actionMonitor = [[self job] performGetSpecificationAction];
	
	[self setGetSpecificationActionMonitor:actionMonitor];
}

- (void)jobSpecificationDidLoad:(NSDictionary *)jobSpecification;
{
	NSString *name = [jobSpecification objectForKey:XGJobSpecificationNameKey];

	NSDictionary *taskSpecifications = [jobSpecification objectForKey:XGJobSpecificationTaskSpecificationsKey];
	NSDictionary *taskSpecification = [taskSpecifications objectForKey:@"0"];
	NSArray *arguments = [taskSpecification objectForKey:XGJobSpecificationArgumentsKey];
	NSString *command = [arguments count] > 1 ? [arguments objectAtIndex:1] : nil;
	
	[self setValue:name forKey:@"jobName"];
	[self setValue:command forKey:@"command"];

	[self setValue:@"" forKey:@"statusMessage"];
	[self setValue:[NSNumber numberWithBool:NO] forKey:@"isLoading"];
	
	[self setGetSpecificationActionMonitor:nil];
}

- (void)jobSpecificationDidNotLoad:(NSDictionary *)results;
{
	[self setValue:@"Clone failed" forKey:@"statusMessage"];
	[self setValue:[NSNumber numberWithBool:NO] forKey:@"isLoading"];

	[self setGetSpecificationActionMonitor:nil];
}

- (void)jobDidSubmit:(NSDictionary *)results;
{
	[self setValue:@"" forKey:@"statusMessage"];
	[self setValue:[NSNumber numberWithBool:NO] forKey:@"isLoading"];
	
	[self setSubmitJobActionMonitor:nil];
	
	[[self window] close];
}

- (void)jobDidNotSubmit:(NSDictionary *)results;
{
	[self setValue:@"Submission failed" forKey:@"statusMessage"];
	[self setValue:[NSNumber numberWithBool:NO] forKey:@"isLoading"];
	
	[self setSubmitJobActionMonitor:nil];
}

- (XGGrid *)selectedGrid;
{
	NSArray *selectedObjects = [_gridsArrayController selectedObjects];
	
	if ([selectedObjects count] == 1) {
	
		return [selectedObjects objectAtIndex:0];
	}

	return nil;
}

- (NSDictionary *)jobSpecification;
{
	NSString *name = _jobName;
	NSString *command = _command;
	
	if (name == nil || [name isEqualToString:@""] == YES) {
	
		name = command;
	}
	
	// task specification
	NSString *taskCommand = @"/bin/sh";
	NSArray *taskArguments = [NSArray arrayWithObjects:@"-c", command, nil];
	NSMutableDictionary *taskSpecification = [NSMutableDictionary dictionary];
	[taskSpecification setObject:taskCommand forKey:XGJobSpecificationCommandKey];
	[taskSpecification setObject:taskArguments forKey:XGJobSpecificationArgumentsKey];
	
	// task specifications
	NSString *taskIdentifier = @"0";
	NSMutableDictionary *taskSpecifications = [NSMutableDictionary dictionary];
	[taskSpecifications setObject:taskSpecification forKey:taskIdentifier];
	
	// job specification
	NSString *jobSubmissionIdentifier = @"abc";
	NSMutableDictionary *jobSpecification = [NSMutableDictionary dictionary];
	[jobSpecification setObject:XGJobSpecificationTypeTaskListValue forKey:XGJobSpecificationTypeKey];
	[jobSpecification setObject:name forKey:XGJobSpecificationNameKey];
	[jobSpecification setObject:jobSubmissionIdentifier forKey:XGJobSpecificationSubmissionIdentifierKey];
	[jobSpecification setObject:taskSpecifications forKey:XGJobSpecificationTaskSpecificationsKey];
	
	return jobSpecification;
}

#pragma mark *** Key-value observer method ***

- (void)observeValueForKeyPath:(NSString *)keyPath
					  ofObject:(id)object 
                        change:(NSDictionary *)change
                       context:(void *)context;
{
	if (object == [self getSpecificationActionMonitor] && [keyPath isEqualToString:@"outcome"] == YES) {
		
		XGActionMonitor *actionMonitor = object;
		
		if ([actionMonitor outcome] == XGActionMonitorOutcomeSuccess) {
		
			[self jobSpecificationDidLoad:[actionMonitor results]];
		}
		else {
		
			[self jobSpecificationDidNotLoad:[actionMonitor results]];
		}
	}
	else if (object == [self submitJobActionMonitor] && [keyPath isEqualToString:@"outcome"] == YES) {
		
		XGActionMonitor *actionMonitor = object;
		
		if ([actionMonitor outcome] == XGActionMonitorOutcomeSuccess) {
		
			[self jobDidSubmit:[actionMonitor results]];
		}
		else {
		
			[self jobDidNotSubmit:[actionMonitor results]];
		}
	}
	else {
		
		[super observeValueForKeyPath:keyPath ofObject:object change:change context:context];
	}
}

@end
