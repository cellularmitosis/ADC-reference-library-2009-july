/*

File: MyWindowController.m

Abstract: Source file for this sample's main NSWindowController.

Version: 1.0

Disclaimer: IMPORTANT:  This Apple software is supplied to you by Apple
Inc. ("Apple") in consideration of your agreement to the
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

Copyright © 2007 Apple Inc. All Rights Reserved

*/

#import "MyWindowController.h"
#import "SpeedometerView/SpeedometerView.h"

#import <IOKit/pwr_mgt/IOPM.h>
#import <IOKit/ps/IOPSKeys.h>

@implementation MyWindowController

// property synthesis
@synthesize useCustomBadge, useBadgeNumber, useBadgeNumberTimer, hasBattery, reflectBatteryLevel;
@synthesize dockSpeedView, dockImageView;

// which content to use in the dock tile:
enum {
	kSpeedomederView = 1,
	kImageView
};

// -------------------------------------------------------------------------------
//	dealloc:
// -------------------------------------------------------------------------------
- (void)dealloc
{
	self.dockSpeedView = nil;
	self.dockImageView = nil;
	
	[super dealloc];
}

// -------------------------------------------------------------------------------
//	updateSpeedometer
//
//	Update the speedometer view with the current battery level.
// -------------------------------------------------------------------------------
- (void)updateSpeedometer
{
	// make sure the current doc view is the speedometer
	if (reflectBatteryLevel && [dockTile contentView] == dockSpeedView)
	{
		// retrieve the power source data
		CFTypeRef info = (CFTypeRef)IOPSCopyPowerSourcesInfo();
		if (info)
		{
			// retrieve the array of power sources
			NSArray* list = (NSArray*)IOPSCopyPowerSourcesList(info);
			if (list)
			{
				// get the description info on our battery
				NSDictionary* ups_Info = (NSDictionary*)IOPSGetPowerSourceDescription(info, [list objectAtIndex:0]);
				if (ups_Info)
				{
					// compute the percent of power remaining on the battery
					CGFloat t1 = [[ups_Info objectForKey:[NSString stringWithUTF8String:kIOPSCurrentCapacityKey]] floatValue];
					CGFloat t2 = [[ups_Info objectForKey:[NSString stringWithUTF8String:kIOPSMaxCapacityKey]] floatValue];
					CGFloat percentRemaining = (t1/t2)*100;
					
					NSLog(@"level = %f", percentRemaining);
					
					[windowSpeedView setSpeed: percentRemaining];
				}
				CFRelease(list);
			}
			CFRelease(info);
		}
	}
}

// -------------------------------------------------------------------------------
//	powerSourceChange:
//
//	We are being called to reflect the battery power change,
//	the power source (battery or UPS) has changed.
// -------------------------------------------------------------------------------
void powerSourceChange(void* runLoopInfo)
{
	MyWindowController* controller = runLoopInfo;
	[controller updateSpeedometer];
}

// -------------------------------------------------------------------------------
//	awakeFromNib:
// -------------------------------------------------------------------------------
-(void)awakeFromNib
{
	// get our Dock tile to this app
	dockTile = [NSApp dockTile];
	
	// setup our image view for the dock tile		
	NSRect frame = NSMakeRect(0, 0, dockTile.size.width, dockTile.size.height);
	dockImageView = [[NSImageView alloc] initWithFrame: frame];
	[dockImageView setImage: [NSImage imageNamed:@"LakeDonPedro.jpg"]];
	
	// by default, add it to the NSDockTile
	[dockTile setContentView: dockImageView];
	[dockTile display];
	
	// create our speedometer view to be added to the dock tile when requested
	[dockSpeedView = [SpeedometerView alloc] initWithFrame: frame];
	
	// listen for changes in the speedometere's value via "key value observer"
	[windowSpeedView addObserver: self
						forKeyPath: @"speed"
						options: (NSKeyValueObservingOptionNew | NSKeyValueObservingOptionOld) 
						context: NULL];

	badgeCount = 0;
	
	self.useBadgeNumber = NO;	// default to not use badging
	
	// check to see if this CPU has a battery installed
	NSArray* batteryList;
	if (kIOReturnSuccess != IOPMCopyBatteryInfo(kIOMasterPortDefault, &batteryList))
	{
		self.hasBattery = NO;
	}
	else
	{            
		self.hasBattery = YES;
		
		// since we are running on battery power, add a run-loop source (handler) to monitor the battery power,
		// "powerSourceChange" will be called when the power source (battery or UPS) changes.
		CFRunLoopSourceRef runLoopSource = (CFRunLoopSourceRef)IOPSNotificationCreateRunLoopSource(powerSourceChange, self);
		if (runLoopSource)
		{
			CFRunLoopAddSource(CFRunLoopGetCurrent(), runLoopSource, kCFRunLoopDefaultMode);
			CFRelease(runLoopSource);
		}
		
		[batteryList release];
	}
	
	// default to not reflect the battery level to the speedometer view
	self.reflectBatteryLevel = NO;
}


#pragma mark - Badge Number Timer Support

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

// -------------------------------------------------------------------------------
//	updateBadgeTimer:object
// -------------------------------------------------------------------------------
- (void)updateBadgeTimer:(id)object
{
	// start with an empty badge
	NSMutableString* finalBadgeStr = [NSMutableString stringWithString:@""];
	
	if (useCustomBadge)
		[finalBadgeStr appendString:[customBadge stringValue]];
		
	if (useBadgeNumber)
		[finalBadgeStr appendString:[NSString stringWithFormat:@"%ld", badgeCount++]];
	
	[dockTile setBadgeLabel:finalBadgeStr];
	[dockTile display];
}

// -------------------------------------------------------------------------------
//	updateProgress:timer
// -------------------------------------------------------------------------------
- (void)updateProgress:(NSTimer*)t
{	
	if (!useBadgeNumber || !useBadgeNumberTimer)
	{
		// stop the timer
		[t invalidate];
		[self setTimer: nil];
		
		// reset the badge back to its original state,
		// we must do this on the main thread is it affects our UI -
		[self performSelectorOnMainThread: @selector(updateBadge:) withObject:nil waitUntilDone:YES];
	}
		
	// continue with timer operation - update the badge label with counter
	// we must do this on the main thread is it affects our UI -
	[self performSelectorOnMainThread: @selector(updateBadgeTimer:) withObject:nil waitUntilDone:YES];
}

// -------------------------------------------------------------------------------
//	timerAction:sender
// -------------------------------------------------------------------------------
- (IBAction)timerAction:(id)sender
{
	[progressGear setHidden:![[sender selectedCell] state]];
	
	if ([[sender selectedCell] state])
	{
		[progressGear startAnimation:self];
		
		badgeCount = 0;
		[self setTimer: [NSTimer scheduledTimerWithTimeInterval: 1.0	// 1 second
														 target: self
													   selector: @selector(updateProgress:)
													   userInfo: nil
														repeats: YES]];
	}
	else
	{
		[progressGear stopAnimation:self];
	}
}

// -------------------------------------------------------------------------------
//	batteryCheckAction:sender
//
//	User clicked the "Reflect Battery Level" checkbox
// -------------------------------------------------------------------------------
- (IBAction)batteryCheckAction:(id)sender
{
	if (hasBattery && reflectBatteryLevel)
	{
		// are we showing the speedometer view in the dock?
		if ([dockTile contentView] == dockSpeedView)
		{															
			// give the speedometer view its first update
			[self updateSpeedometer];
		}
	}
}


#pragma mark - 

// -------------------------------------------------------------------------------
//	observeValueForKeyPath:ofObject:change:context
//
//	Listen for changes in the SpeedometerView's value, then turn around and reflect
//	the change to the SpeedometerView in the NSDockTile.
// -------------------------------------------------------------------------------
- (void)observeValueForKeyPath:(NSString*)keyPath
                    ofObject:(id)object 
                    change:(NSDictionary*)change 
                    context:(void *)context
{
	[dockSpeedView setSpeed: [object speed]];
	[dockTile display];
}

// -------------------------------------------------------------------------------
//	dragInAction:sender
//
//	User dragged an image into the image well.
// -------------------------------------------------------------------------------
- (IBAction)dragInAction:(id)sender
{
	[imageView setImage:[imageView image]];
	[dockImageView setImage: [imageView image]];
	[dockTile display];
}

// -------------------------------------------------------------------------------
//	updateBadge:obj
//
//	Our timer has fired - update the badge string.
// -------------------------------------------------------------------------------
- (void)updateBadge:(id)obj
{
	// start with an empty badge
	NSMutableString* finalBadgeStr = [NSMutableString stringWithString:@""];
	
	if (useCustomBadge)
		[finalBadgeStr appendString:[customBadge stringValue]];
		
	if (useBadgeNumber)
		[finalBadgeStr appendString:[badgeNumber stringValue]];
	
	[dockTile setBadgeLabel:finalBadgeStr];
	[dockTile display];
}

// -------------------------------------------------------------------------------
//	useBadgeNumber:sender
// -------------------------------------------------------------------------------
- (IBAction)useBadgeNumber:(id)sender
{
	[self updateBadge:nil];
	
	if (useBadgeNumberTimer)
		[self timerAction:self];
}

// -------------------------------------------------------------------------------
//	useCustomBadge:sender
// -------------------------------------------------------------------------------
- (IBAction)useCustomBadge:(id)sender
{
	[self updateBadge:nil];
}

// -------------------------------------------------------------------------------
//	whichViewAction:sender
//
//	User clicked one of the radio buttons to change the image in the dock.
// -------------------------------------------------------------------------------
- (IBAction)whichViewAction:(id)sender
{
	NSInteger tag = [[sender selectedCell] tag];
	switch (tag)
	{ 
		case kSpeedomederView:
		{
			// set the dock tile's view to the speedometer view
			[dockTile setContentView: dockSpeedView];
			
			// now that we are using the speedometer view, possibly start our battery level indicator timer
			[self batteryCheckAction:self];	
			break;
		}
		
		case kImageView:
		{
			// set the dock tile's view to the image view
			[dockTile setContentView: dockImageView];
			break;
		}
	}
	[dockTile display];
}

// -------------------------------------------------------------------------------
//	controlTextDidChange:notification
//
//	The user changed the text in the custom badge or badge number edit fields.
// -------------------------------------------------------------------------------
- (void)controlTextDidChange:(NSNotification*)notification
{
	NSMutableString* finalString = [NSMutableString stringWithString:@""];
	
	if (useCustomBadge)
		[finalString appendString: [customBadge stringValue]];
	if (useBadgeNumber)
		[finalString appendString: [badgeNumber stringValue]];
	
	[dockTile setBadgeLabel: finalString];
}

// -------------------------------------------------------------------------------
//	arrowsAction:sender
//
//	User clicked the little arrows control (NSStepper).
// -------------------------------------------------------------------------------
- (IBAction)arrowsAction:(id)sender	
{
	[badgeNumber setIntValue: [sender integerValue]];
	[self updateBadge:nil];
}
	
@end
