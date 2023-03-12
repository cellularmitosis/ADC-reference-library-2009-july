/*
    File:  AppController.m
 
    Contains:  Application delegate class illustrating how to setup and start an audio burn.
    
    Copyright:  © Copyright 2003 - 2004 Apple Computer, Inc. All rights reserved.

    Disclaimer: IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc.
    ("Apple") in consideration of your agreement to the following terms, and your
    use, installation, modification or redistribution of this Apple software
    constitutes acceptance of these terms.  If you do not agree with these terms,
    please do not use, install, modify or redistribute this Apple software.

    In consideration of your agreement to abide by the following terms, and subject
    to these terms, Apple grants you a personal, non-exclusive license, under Apple’s
    copyrights in this original Apple software (the "Apple Software"), to use,
    reproduce, modify and redistribute the Apple Software, with or without
    modifications, in source and/or binary forms; provided that if you redistribute
    the Apple Software in its entirety and without modifications, you must retain
    this notice and the following text and disclaimers in all such redistributions of
    the Apple Software.  Neither the name, trademarks, service marks or logos of
    Apple Computer, Inc. may be used to endorse or promote products derived from the
    Apple Software without specific prior written permission from Apple.  Except as
    expressly stated in this notice, no other rights or licenses, express or implied,
    are granted by Apple herein, including but not limited to any patent rights that
    may be infringed by your derivative works or by other works in which the Apple
    Software may be incorporated.

    The Apple Software is provided by Apple on an "AS IS" basis.  APPLE MAKES NO
    WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED
    WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS FOR A PARTICULAR
    PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND OPERATION ALONE OR IN
    COMBINATION WITH YOUR PRODUCTS.

    IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL OR
    CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
    GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
    ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION, MODIFICATION AND/OR DISTRIBUTION
    OF THE APPLE SOFTWARE, HOWEVER CAUSED AND WHETHER UNDER THEORY OF CONTRACT, TORT
    (INCLUDING NEGLIGENCE), STRICT LIABILITY OR OTHERWISE, EVEN IF APPLE HAS BEEN
    ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
              
    Change History (most recent first):
        1/24/2004     1.0.1      Fixed build issue with Panther.

*/
    
#import "AppController.h"

#import <DiscRecording/DiscRecording.h>
#import <DiscRecordingUI/DiscRecordingUI.h>
#import "TestTrack.h"

@implementation AppController

/* App's done launching, put up the burn setup panel. If the user chooses burn, 
	we'll start the burn, otherwise, just quit the app. */
- (void)applicationDidFinishLaunching:(NSNotification *)aNotification;
{
	NSArray*	tracks = [self createTracks];
	
	if (tracks)
	{
		DRBurnSetupPanel*	bsp = [DRBurnSetupPanel setupPanel];

		// We'll be the delegate for the setup panel. This allows us to show off some 
		// of the customization you can do.
		[bsp setDelegate:self];
		
		if ([bsp runSetupPanel] == NSOKButton)
		{
			DRBurnProgressPanel*	bpp = [DRBurnProgressPanel progressPanel];

			[bpp setDelegate:self];

			// And start off the burn itself. This will put up the progress dialog 
			// and do all the nice pretty things that a happy app does.
			[bpp beginProgressPanelForBurn:[bsp burnObject] layout:tracks];
			
			/* If you wanted to run this as a sheet you would have sent
			  [bpp beginProgressSheetForBurn:[bsp burnObject] layout:tracks modalForWindow:aWindow];
			*/
		}
		else
			[NSApp terminate:self];
	}
	else
		[NSApp terminate:self];
}

- (NSArray*) createTracks
{
	NSMutableArray*			trackArray = [NSMutableArray arrayWithCapacity:2];
	TestTrack*				track;
	TestTrackProducer*		producer;

	// Create two tracks. One of them will verify using the production loop again.
	// This is the more typical path for code to take. It's easy and you only have to
	// write one path for data production.
	producer = [[TestTrackProducer alloc] initWithLength:[DRMSF msfWithFrames:75*60] verificationType:DRVerificationTypeProduceAgain];
	track = [[TestTrack alloc] initWithProducer:producer];
	[producer release];

	[trackArray addObject:track];

	// This one will verify itself from data sent back from the engine. This sort of
	// verification is useful for data that can't be recreated again for example.
	producer = [[TestTrackProducer alloc] initWithLength:[DRMSF msfWithFrames:75*60] verificationType:DRVerificationTypeReceiveData];
	track = [[TestTrack alloc] initWithProducer:producer];
	[producer release];

	[trackArray addObject:track];
	[track release];
	
	return trackArray;
}

- (BOOL) validateMenuItem:(id)sender
{
	if ([sender action] == @selector(terminate:))
	{
		return (burning == NO);		// No quitting while a burn is going on
	}
	else
		return [super validateMenuItem:sender];
}


#pragma mark Progress Panel Delegate Methods

/* Here we are setting up this nice little instance variable that prevents the app from
	quitting while a burn is in progress. This gets checked up in validateMenu: and we'll
	set it to NO in burnProgressPanelDidFinish: */
- (void) burnProgressPanelWillBegin:(NSNotification*)aNotification
{
	burning = YES;	// Keep the app from being quit from underneath the burn.
}

- (void) burnProgressPanelDidFinish:(NSNotification*)aNotification
{
	burning = NO;	// OK we can quit now.
}

/* OK, nothing fancy here. we just want to illustrate that it's possible for a delegate of the 
	progress panel to alter how the burn is handled once it completes. You may want to put up
	your own dialog, sent a notification if you're in the background, or just ignore it no matter what.
	
	We'll just NSLog the fact it finished (for good or bad) and return YES to indicate
	that we didn't handle it ourselves and that the progress panel should continue on its
	merry way. */
- (BOOL) burnProgressPanel:(DRBurnProgressPanel*)theBurnPanel burnDidFinish:(DRBurn*)burn
{
	NSDictionary*	burnStatus = [burn status];
	NSString*		state = [burnStatus objectForKey:DRStatusStateKey];
	
	if ([state isEqualToString:DRStatusStateFailed])
	{
		NSDictionary*	errorStatus = [burnStatus objectForKey:DRErrorStatusKey];
		NSString*		errorString = [errorStatus objectForKey:DRErrorStatusErrorStringKey];
		
		NSLog(@"The burn failed (%@)!", errorString);
	}
	else
		NSLog(@"Burn finished fine");
	
	return YES;
}

@end