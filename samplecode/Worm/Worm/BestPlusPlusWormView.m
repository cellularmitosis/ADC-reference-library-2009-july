/*
 
 File: BestPlusPlusWormView.m
 Abstract: This subclass of WormView implements a separate
                display update timer that periodically updates 
                the view. It collects all the dirty portions of the 
                view since the last update and flushes only the dirty 
                portion when updating.
 
 Version: <1.0>
 
 © Copyright 2005 Apple Computer, Inc. All rights reserved.
 
 IMPORTANT:  This Apple software is supplied to 
 you by Apple Computer, Inc. ("Apple") in 
 consideration of your agreement to the following 
 terms, and your use, installation, modification 
 or redistribution of this Apple software 
 constitutes acceptance of these terms.  If you do 
 not agree with these terms, please do not use, 
 install, modify or redistribute this Apple 
 software.
 
 In consideration of your agreement to abide by 
 the following terms, and subject to these terms, 
 Apple grants you a personal, non-exclusive 
 license, under Apple's copyrights in this 
 original Apple software (the "Apple Software"), 
 to use, reproduce, modify and redistribute the 
 Apple Software, with or without modifications, in 
 source and/or binary forms; provided that if you 
 redistribute the Apple Software in its entirety 
 and without modifications, you must retain this 
 notice and the following text and disclaimers in 
 all such redistributions of the Apple Software. 
 Neither the name, trademarks, service marks or 
 logos of Apple Computer, Inc. may be used to 
 endorse or promote products derived from the 
 Apple Software without specific prior written 
 permission from Apple.  Except as expressly 
 stated in this notice, no other rights or 
 licenses, express or implied, are granted by 
 Apple herein, including but not limited to any 
 patent rights that may be infringed by your 
 derivative works or by other works in which the 
 Apple Software may be incorporated.
 
 The Apple Software is provided by Apple on an "AS 
 IS" basis.  APPLE MAKES NO WARRANTIES, EXPRESS OR 
 IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED 
 WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY 
 AND FITNESS FOR A PARTICULAR PURPOSE, REGARDING 
 THE APPLE SOFTWARE OR ITS USE AND OPERATION ALONE 
 OR IN COMBINATION WITH YOUR PRODUCTS.
 
 IN NO EVENT SHALL APPLE BE LIABLE FOR ANY 
 SPECIAL, INDIRECT, INCIDENTAL OR CONSEQUENTIAL 
 DAMAGES (INCLUDING, BUT NOT LIMITED TO, 
          PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS 
          OF USE, DATA, OR PROFITS; OR BUSINESS 
          INTERRUPTION) ARISING IN ANY WAY OUT OF THE USE, 
 REPRODUCTION, MODIFICATION AND/OR DISTRIBUTION OF 
 THE APPLE SOFTWARE, HOWEVER CAUSED AND WHETHER 
 UNDER THEORY OF CONTRACT, TORT (INCLUDING 
                                 NEGLIGENCE), STRICT LIABILITY OR OTHERWISE, EVEN 
 IF APPLE HAS BEEN ADVISED OF THE POSSIBILITY OF 
 SUCH DAMAGE.
 
 */

#import "BestPlusPlusWormView.h"


@implementation BestPlusPlusWormView

// The display timer calls this method periodically

- (void)fireUpdate 
{
    // Indicate that the accumulated update rectangle is dirty 
    [self setNeedsDisplayInRect:mUpdateRect];
    mUpdateRect = NSMakeRect(0,0,0,0);  // reset the accumulated dirty rectangle
}


// Display Timer callback - Just fire a display update on the callback

static void wormDisplayTimerCallBack(CFRunLoopTimerRef timer, void *info) 
{
    CFAbsoluteTime startTime = CFAbsoluteTimeGetCurrent();

    BestPlusPlusWormView *self = (BestPlusPlusWormView *)info;
    [self fireUpdate];

    CFRunLoopTimerSetNextFireDate(timer, startTime + CFRunLoopTimerGetInterval(timer));
}


// Update the view at 30 Hz

- (float)desiredDisplayFrameRate {
    return 30;
}


// Do everything of the super, but add a new display update timer.

- (void)start 
{
    [super start];

    // Install a separate Display Timer that updates the display periodically
    if (!displayTimer) 
    {
        CFRunLoopTimerContext context = {0, self, NULL, NULL, NULL};
        displayTimeStamp = CFAbsoluteTimeGetCurrent();
        displayTimer =  CFRunLoopTimerCreate(NULL, displayTimeStamp, 1.0/[self desiredDisplayFrameRate], 
					     0, 0, wormDisplayTimerCallBack, &context);
        CFRunLoopAddTimer(CFRunLoopGetCurrent(), displayTimer, kCFRunLoopCommonModes);
    }

    // Keep a rectangle representing the area that needs to be updated. Initially it is empty. 
    mUpdateRect = NSMakeRect(0,0,0,0);

}


// Called once for each frame of the animation.
// Just need to calculate the update rectangle for this frame based on where the worm
// has been and will be.

- (BOOL)performAnimation 
{
    BOOL done;
    NSRect rect = NSZeroRect;
    GamePosition oldTargetPosition = targetPosition;
    unsigned int i;
    
    // Compute a rect covering the worm's original position
    for (i = 0; i < wormLength; i++) {
        rect = NSUnionRect(rect, [self rectForPosition:wormPositions[i]]);
    }
    
    // Update game state
    done = [self updateState];
    
    // Union in the new head (since the worm has moved)
    rect = NSUnionRect(rect, [self rectForPosition:wormPositions[0]]);
    
    // If the target has changed locations, also include that in
    if (oldTargetPosition.x != targetPosition.x || oldTargetPosition.y != targetPosition.y) {
        rect = NSUnionRect(rect, [self rectForPosition:targetPosition]);
    }
  
    // Accumulate the current frame's update rectangle
    mUpdateRect = NSUnionRect(rect, mUpdateRect);
  
    // No longer need to update the display - the display timer will handle this
    //    [self setNeedsDisplayInRect:rect];
    
    return done;
}

//  Stop the display timer when stopping the animation

- (void)stop:(BOOL)flag {
    [super stop:flag];
    if (displayTimer) {
        CFRunLoopTimerInvalidate(displayTimer);
        CFRelease(displayTimer);
        displayTimer = NULL;
    }
}
@end
