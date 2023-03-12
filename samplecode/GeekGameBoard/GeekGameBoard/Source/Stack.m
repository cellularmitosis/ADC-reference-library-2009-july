/*

File: Stack.m

Abstract: A holder for multiple Bits that lines them up in stacks or rows.

Version: 1.0

Disclaimer: IMPORTANT:  This Apple software is supplied to you by 
Apple Inc. ("Apple") in consideration of your agreement to the
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

Copyright © 2007 Apple Inc. All Rights Reserved.

*/


#import "Stack.h"
#import "QuartzUtils.h"


/**  WARNING: THIS CODE REQUIRES GARBAGE COLLECTION!
 **  This sample application uses Objective-C 2.0 garbage collection.
 **  Therefore, the source code in this file does NOT perform manual object memory management.
 **  If you reuse any of this code in a process that isn't garbage collected, you will need to
 **  add all necessary retain/release/autorelease calls, and implement -dealloc methods,
 **  otherwise unpleasant leakage will occur!
 **/


@implementation Stack


- (id) initWithStartPos: (CGPoint)startPos spacing: (CGSize)spacing
           wrapInterval: (int)wrapInterval wrapSpacing: (CGSize)wrapSpacing
{
    self = [super init];
    if (self != nil) {
        _startPos = startPos;
        _spacing = spacing;
        _wrapInterval = wrapInterval;
        _wrapSpacing = wrapSpacing;
        self.cornerRadius = 8;
        self.backgroundColor = kAlmostInvisibleWhiteColor;
        self.borderColor = kHighlightColor;
        _bits = [NSMutableArray array];
    }
    return self;
}

- (id) initWithStartPos: (CGPoint)startPos spacing: (CGSize)spacing;
{
    return [self initWithStartPos: startPos spacing: spacing 
                     wrapInterval: INT_MAX wrapSpacing: CGSizeZero];
}


@synthesize spacing=_spacing, wrapSpacing=_wrapSpacing, startPos=_startPos, wrapInterval=_wrapInterval;
@synthesize dragAsStacks=_dragAsStacks;
@synthesize bits=_bits;


- (Bit*) topBit
{
    return [_bits lastObject];
}


- (void) dump
{
    printf("Stack = ");
    for( CALayer *layer in self.sublayers )
        printf("%s @z=%g   ", [[layer description] UTF8String],layer.zPosition);
    printf("\n");
}


- (void) x_repositionBit: (Bit*)bit forIndex: (int)i
{
    bit.position = CGPointMake(_startPos.x + _spacing.width *(i%_wrapInterval) + _wrapSpacing.width *(i/_wrapInterval),
                               _startPos.y + _spacing.height*(i%_wrapInterval) + _wrapSpacing.height*(i/_wrapInterval));
}

- (void) addBit: (Bit*)bit
{
    if( [bit isKindOfClass: [DraggedStack class]] ) {
        for( Bit *subBit in [(DraggedStack*)bit bits] )
            [self addBit: subBit];
    } else {
        int n = _bits.count;
        [_bits addObject: bit];
        ChangeSuperlayer(bit, self, n);
        [self x_repositionBit: bit forIndex: n];
    }
}


- (void) setHighlighted: (BOOL)highlighted
{
    [super setHighlighted: highlighted];
    self.borderWidth = (highlighted ?6 :0);
}


- (Bit*) canDragBit: (Bit*)bit
{
    NSInteger index = [_bits indexOfObjectIdenticalTo: bit];
    if( index==NSNotFound )
        return nil;
    if( _dragAsStacks && index < _bits.count-1 ) {
        // Move bit and those above it into a temporary DraggedStack:
        NSRange r = NSMakeRange(index,_bits.count-index);
        NSArray *bitsToDrag = [_bits subarrayWithRange: r];
        [_bits removeObjectsInRange: r];
        DraggedStack *stack = [[DraggedStack alloc] initWithBits: bitsToDrag];
        [self addSublayer: stack];
        stack.anchorPoint = CGPointMake( bit.position.x/stack.bounds.size.width,
                                         bit.position.y/stack.bounds.size.height );
        stack.position = bit.position;
        return stack;
    } else {
        [_bits removeObjectIdenticalTo: bit];
        return bit;
    }
}


- (void) cancelDragBit: (Bit*)bit
{
    [self addBit: bit];
    if( [bit isKindOfClass: [DraggedStack class]] ) {
        [bit removeFromSuperlayer];
    }
}


- (void) draggedBit: (Bit*)bit to: (id<BitHolder>)dst
{
    int i=0;
    for( Bit *bit in self.sublayers )
        [self x_repositionBit: bit forIndex: i++];
}


- (BOOL) dropBit: (Bit*)bit atPoint: (CGPoint)point
{
    [self addBit: bit];
    return YES;
}

@end




@implementation DraggedStack


- (id) initWithBits: (NSArray*)bits
{
    self = [super init];
    if( self ) {
        CGRect bounds = CGRectZero;
        for( Bit *bit in bits ) {
            bounds = CGRectUnion(bounds, bit.frame);
            [self addSublayer: bit];
        }
        self.bounds = bounds;
        self.anchorPoint = CGPointZero;
        self.position = CGPointZero;
    }
    return self;
}

- (NSArray*) bits
{
    return self.sublayers.copy;
}

@end
