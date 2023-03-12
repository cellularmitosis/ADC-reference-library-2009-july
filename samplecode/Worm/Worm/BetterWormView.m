/*
 
 File: ActualWormView.m
 Abstract: This subclass of WormView and GoodWormView draws 
                only the changed rectangle, rather than the whole view,
                on each frame. 
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

#import "BetterWormView.h"

@implementation BetterWormView

- (BOOL)performAnimation {

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
    
    // Now, mark only the computed rect as needing redisplay
    [self setNeedsDisplayInRect:rect];
    
    return done;
}

/* BetterWormView overrides drawRect: to pay attention to the rect passed in, instead of drawing everything. This optimization goes hand-in-hand with the optimization in performAnimation above to compute the tight update rect. 

Note that because the tight update rect already contains the worm's body, it might seem unnecessary to do an intersection check in the code below for the worm's body --- after all, the update rect will always include the body, right? Usually yes, but there might be situations where that is not the case. For instance, if someone put the worm in a scrollView, or if something else caused the window to redraw and passed in a different rect.
*/
- (void)drawRect:(NSRect)rect {
    unsigned i;
    NSRect tRect;
    
    [backgroundColor set];
    NSRectFill(rect);	// Fill in only the update rect, not whole bounds
    
    for (i = 0; i < wormLength; i++) {
        NSRect wRect = [self rectForPosition:wormPositions[i]];
        if (NSIntersectsRect(wRect, rect)) {	// Draw this portion of the body only if in the update rect
            unsigned characterIndex = i % [wormString length];
            NSString *string = [wormString substringWithRange:NSMakeRange(characterIndex, 1)];

            [string drawInRect:wRect withAttributes: wormTextAttributes];
        }
    }
    
    tRect = [self rectForPosition:targetPosition];
    if (NSIntersectsRect(tRect, rect)) {	// Draw target only if in the update rect
        [[NSColor blackColor] set];
        [[NSBezierPath bezierPathWithOvalInRect:NSInsetRect(tRect, 2, 2)] fill];
    }
}

@end
