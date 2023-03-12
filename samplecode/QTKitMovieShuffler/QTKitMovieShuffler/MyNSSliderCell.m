/*

File: MyNSSliderCell.m

Abstract: Code to manage our volume slider

Version: <1.0>

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

Copyright © 2005 Apple Computer, Inc., All Rights Reserved

*/ 


#import "MyNSSliderCell.h"
#import "QTMovieExtensions.h"


@implementation MyNSSliderCell

-(id)initWithValues:(NSTextField *)aTextField controller:(AppController *)aController
{
	if (self = [super init])
	{
		NSFont *font = [NSFont fontWithName:@"Helvetica" size:12.0];
		attrs = [NSMutableDictionary dictionary];
		[attrs setObject:font forKey:NSFontAttributeName];
		[attrs setObject:[NSColor greenColor] forKey:NSBackgroundColorAttributeName];
		
		userIsDraggingSlider = NO;
		continuePlayingAfterDrag = NO;
		
		myController = aController;
		movieTimeStringBox = aTextField;
		
		previousStringRect = NSMakeRect(0,0,0,0);
	}
	
	return (self);
}


	// NSCell invokes this method when tracking begins
- (BOOL)startTrackingAt:(NSPoint)startPoint inView:(NSView *)controlView
{
	MyQTMovie *curMovie = [myController currentPlayingMovie];

	[super startTrackingAt:startPoint inView:controlView];

	userIsDraggingSlider = YES;
	
	continuePlayingAfterDrag = NO;
		// if movie is currently playing, we'll want to continue
		// playing even after tracking finishes
	if ([curMovie isPlaying] == YES)
	{
		continuePlayingAfterDrag = YES;
		// stop movie playing while tracking happens...
		[curMovie stop];
	}
	
	return (YES);
}

	// NSCell invokes this method when the cursor has left the bounds
	// of the receiver or the mouse button goes up (in which case flag is YES).
- (void)stopTracking:(NSPoint)lastPoint at:(NSPoint)stopPoint inView:(NSView *)controlView mouseIsUp:(BOOL)flag
{
	[super stopTracking:lastPoint at:stopPoint inView:controlView mouseIsUp:flag];

	userIsDraggingSlider = NO;

	// if the movie was playing when the tracking started,
	// resume playing once tracking is done
	if (continuePlayingAfterDrag == YES)
	{
		[[myController currentPlayingMovie] play];
	}
}

	// return YES if user is currently dragging the slider, NO
	// if not
-(BOOL)isUserDraggingSlider
{
	return userIsDraggingSlider;
}


@end
