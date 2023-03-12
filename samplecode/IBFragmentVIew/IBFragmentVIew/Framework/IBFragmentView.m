/*

File: IBFragmentView.m

Abstract: Abstract: The fragment view class.

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

Copyright © 2006 Apple Computer, Inc., All Rights Reserved

*/

#import "IBFragmentView.h"
#import "IBFragment.h"

static NSBezierPath *BezierPathWithRoundedRect(NSRect rect, CGFloat cornerRadius) {
    NSBezierPath *path = [NSBezierPath bezierPath];
    if (!NSIsEmptyRect(rect)) {
        if (cornerRadius > 0.0) {
            // Clamp radius to be no larger than half the rect's width or height.
            float clampedRadius = MIN(cornerRadius, 0.5 * MIN(rect.size.width, rect.size.height));
            
            NSPoint topLeft = NSMakePoint(NSMinX(rect), NSMaxY(rect));
            NSPoint topRight = NSMakePoint(NSMaxX(rect), NSMaxY(rect));
            NSPoint bottomRight = NSMakePoint(NSMaxX(rect), NSMinY(rect));
            
            [path moveToPoint:NSMakePoint(NSMidX(rect), NSMaxY(rect))];
            [path appendBezierPathWithArcFromPoint:topLeft     toPoint:rect.origin radius:clampedRadius];
            [path appendBezierPathWithArcFromPoint:rect.origin toPoint:bottomRight radius:clampedRadius];
            [path appendBezierPathWithArcFromPoint:bottomRight toPoint:topRight    radius:clampedRadius];
            [path appendBezierPathWithArcFromPoint:topRight    toPoint:topLeft     radius:clampedRadius];
            [path closePath];
        } else {
            // When radius == 0.0, this degenerates to the simple case of a plain rectangle.
            [path appendBezierPathWithRect:rect];
        }
    }
    return path;
}

@implementation IBFragmentView
+ (void)initialize {
    if (self == [IBFragmentView class]) {
        [self setKeys:[NSArray arrayWithObjects:@"fragments", nil] triggerChangeNotificationsForDependentKey:@"fragmentCount"];
    }
}

- (id)initWithFrame:(NSRect)frame {
    if ((self = [super initWithFrame:frame])) {
        fragments = [[NSMutableArray alloc] init];
        IBFragment *fragment = [[[IBFragment alloc] init] autorelease];
        [fragment setTitle:@"Fragment"];
        [self addFragment:fragment];
    }
    return self;
}

- (id)initWithCoder:(NSCoder *)coder {
    if ((self = [super initWithCoder:coder])) {
        fragments = [[coder decodeObjectForKey:@"fragments"] mutableCopy];
    }
    return self;
}

- (void)encodeWithCoder:(NSCoder *)coder {
    [super encodeWithCoder:coder];
    [coder encodeObject:[self fragments] forKey:@"fragments"];
}

- (void)dealloc {
    [fragments release];
    [super dealloc];
}

#pragma mark - Geometry
- (BOOL)isFlipped {
    return YES;
}

- (CGFloat)idealWidth {
    CGFloat idealWidth = 0.0f;
    for(IBFragment *fragment in [self fragments]) {
        idealWidth += [fragment idealSize].width;
    }
    return idealWidth;
}

- (NSArray *)calculateFragmentRects {
    CGFloat idealWidth = [self idealWidth];
    NSSize actualSize = [self bounds].size;
    NSArray *idealWidths = [self valueForKeyPath:@"fragments.idealWidth"];
    
    NSMutableArray *layoutRects = [NSMutableArray array];
    CGFloat x = 0.0f;
    int fragmentIndex = 0;
    for(IBFragment *fragment in [self fragments]) {
        NSRect layoutRect = NSMakeRect(x, 0, 0, 0);
        float percentOfIdeal = [[idealWidths objectAtIndex:fragmentIndex] floatValue] / idealWidth;
        layoutRect.size.width = floor(percentOfIdeal * actualSize.width);
        layoutRect.size.height = actualSize.height;
        x += layoutRect.size.width;
        if (fragment == [[self fragments] lastObject]) {
            layoutRect.size.width += (actualSize.width - x);
        }
        [layoutRects addObject:[NSValue valueWithRect:layoutRect]];
        fragmentIndex += 1;
    }
    return layoutRects;
}

- (void)invalidateLayout {
    [self setNeedsDisplay:YES];
}

#pragma mark - Fragment Management
- (NSArray *)fragments {
    return [[fragments copy] autorelease];
}

- (void)setFragments:(NSArray *)newFragments {
    if (fragments != newFragments) {
        for(IBFragment *fragment in [[[self fragments] copy] autorelease]) {
            [self removeFragment:fragment];
        }
        for(IBFragment *fragment in newFragments) {
            [self addFragment:fragment];
        }
    }
}

- (void)setFragmentCount:(NSInteger)fragmentCount {
    while ([fragments count] > fragmentCount) {
        [self removeFragment:[fragments lastObject]];
    }
    while ([fragments count] < fragmentCount) {
        IBFragment *fragment = [[[IBFragment alloc] init] autorelease];
        [fragment setTitle:@"Fragment"];
        [self addFragment:fragment];
    }    
}

- (NSInteger)fragmentCount {
    return [fragments count];
}

- (void)removeFragment:(IBFragment *)fragment {
    NSAssert([fragments containsObject:fragment], @"Illegal fragment");
    [self willChangeValueForKey:@"fragments"];
    [fragment setFragmentView:nil];
    [fragments removeObject:fragment];
    [self invalidateLayout];
    [self didChangeValueForKey:@"fragments"];
}

- (void)addFragment:(IBFragment *)fragment {
    [self insertFragment:fragment atIndex:[self fragmentCount]];
}

- (void)insertFragment:(IBFragment *)fragment atIndex:(int)index {
    [self willChangeValueForKey:@"fragments"];
    [fragments insertObject:fragment atIndex:index];
    [fragment setFragmentView:self];
    [self invalidateLayout];
    [self didChangeValueForKey:@"fragments"];
}

- (void)drawRect:(NSRect)rect {
    NSArray *fragmentRects = [self calculateFragmentRects];
    NSInteger fragmentIndex = 0;
    NSBezierPath *borderPath = BezierPathWithRoundedRect([self bounds], [self bounds].size.height / 2);
    [[NSColor whiteColor] set];
    [borderPath stroke];
    [borderPath addClip];
    [[[NSColor blackColor] colorWithAlphaComponent:0.25f] set];
    [borderPath fill];
    for(IBFragment *fragment in [self fragments]) {
        NSRect fragmentRect = [[fragmentRects objectAtIndex:fragmentIndex++] rectValue];
        [fragment drawInRect:fragmentRect];
        if (fragment != [[self fragments] lastObject]) {
            [[NSColor whiteColor] set];
            NSBezierPath *line = [NSBezierPath bezierPath];
            [line moveToPoint:NSMakePoint(NSMaxX(fragmentRect), NSMaxY(fragmentRect))]; 
            [line lineToPoint:NSMakePoint(NSMaxX(fragmentRect), NSMinY(fragmentRect))]; 
            [line stroke];
        }
    }
}
@end
