/*
 
 File: ActualWormView.m
 Abstract: This subclass of WormView, GoodWormView, and 
                EvenBetterWormView uses an NSLayoutManager to 
                cache the layout and thus redraw the string very quickly.
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

#import "EvenBetterWormView.h"

@implementation EvenBetterWormView

- (void)setString:(NSString *)string {
    [super setString:string];
    if (!wormStorage) {
        wormStorage = [[NSTextStorage alloc] init];
        wormLayout = [[NSLayoutManager alloc] init];
        wormContainer = [[NSTextContainer alloc] init];
        [wormLayout addTextContainer:wormContainer];
        [wormStorage addLayoutManager:wormLayout];
    }
    [[wormStorage mutableString] setString:wormString];
    [wormStorage setAttributes:wormTextAttributes range:NSMakeRange(0, [wormStorage length])];
}

/* This drawRect is identical to one in BetterWormView, except it draws the glyphs directly out of the NSLayoutManager, rather than using NSString drawing. Because the same strings are being drawn over and over again, not using NSString drawing cuts down on the time required to set up the text system and compute and lay out the glyphs.

We still rely on the optimization done in performAnimation in BetterWormView.m to compute the tight update rect.
*/
- (void)drawRect:(NSRect)rect {
    unsigned i;
    NSRect tRect;
    
    [backgroundColor set];
    NSRectFill(rect);	// Fill in only the update rect, not whole bounds
    
    for (i = 0; i < wormLength; i++) {
        NSRect wRect = [self rectForPosition:wormPositions[i]];
        if (NSIntersectsRect(wRect, rect)) {	// Draw this portion of the body only if in the update rect
            unsigned glyphIndex = i % [wormLayout numberOfGlyphs];
            NSPoint glyphLocation = [wormLayout locationForGlyphAtIndex:glyphIndex];
            NSPoint origin = NSMakePoint(wRect.origin.x - glyphLocation.x, wRect.origin.y);

            [wormLayout drawGlyphsForGlyphRange:NSMakeRange(glyphIndex, 1) atPoint:origin];
        }
    }
    
    tRect = [self rectForPosition:targetPosition];
    if (NSIntersectsRect(tRect, rect)) {	// Draw target only if in the update rect
        [[NSColor blackColor] set];
        [[NSBezierPath bezierPathWithOvalInRect:NSInsetRect(tRect, 2, 2)] fill];
    }
}

// Override dealloc to release the items allocated

- (void)dealloc {
    [wormContainer release];
    [wormLayout release];
    [wormStorage release];
    [super dealloc];
}

@end
