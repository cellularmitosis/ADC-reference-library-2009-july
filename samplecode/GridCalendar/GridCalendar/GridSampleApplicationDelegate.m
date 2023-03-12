/*

File: GridSampleApplicationDelegate.m

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

#import "GridSampleApplicationDelegate.h"
#import "GridSampleServiceBrowser.h"
#import "GridSampleConnectionController.h"
#import "GridSampleJobStateToStringTransformer.h"
#import "GridSampleNewJobWindowController.h"
#import "GridSampleJobInfoWindowController.h"
#import "GridSampleJobResultsWindowController.h"

@interface GridSampleApplicationDelegate (Private)

- (void)registerValueTransformers;

@end

@implementation GridSampleApplicationDelegate

- (id)init;
{
	self = [super init];
	
	if (self != nil) {
	
		_serviceBrowser = [[GridSampleServiceBrowser alloc] init];
	}
	
	return self;
}

- (void)dealloc;
{
	[_serviceBrowser release];
	[_connectionController release];
	
	[super dealloc];
}

#pragma mark *** Accessor methods ***

- (NSString *)displayName;
{
	return [[NSBundle bundleForClass:[self class]] objectForInfoDictionaryKey:@"CFBundleDisplayName"];
}

- (GridSampleServiceBrowser *)serviceBrowser;
{
	return _serviceBrowser;
}

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

- (Class)classForNewJobWindowController;
{
	return [GridSampleNewJobWindowController self];
}

- (Class)classForJobInfoWindowController;
{
	return [GridSampleJobInfoWindowController self];
}

- (Class)classForJobResultsWindowController;
{
	return [GridSampleJobResultsWindowController self];
}

#pragma mark *** Application delegate methods ***

- (void)applicationDidFinishLaunching:(NSNotification *)notification;
{
    [self registerValueTransformers];

    [[self serviceBrowser] browse];
	
	[self setConnectionController:[GridSampleConnectionController connectionController]];
	
	[[self connectionController] begin];
}

- (void)applicationWillTerminate:(NSNotification *)notification;
{
	[[self serviceBrowser] stop];
}

#pragma mark *** Convenience methods ***

- (void)registerValueTransformers;
{
	NSValueTransformer *jobStateToStringTransformer = [[[GridSampleJobStateToStringTransformer alloc] init] autorelease];
    
	[NSValueTransformer setValueTransformer:jobStateToStringTransformer
									forName:@"GridSampleJobStateToStringTransformer"];
}

@end

@implementation NSString (compareNumerically)

- (NSComparisonResult)compareNumerically:(NSString *)string;
{
	return [self compare:string options:NSNumericSearch];
}

@end
