/*

File: Bit.m

Abstract: A moveable item in a card/board game.

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


#import "Bit.h"
#import "Game.h"
#import "QuartzUtils.h"


/**  WARNING: THIS CODE REQUIRES GARBAGE COLLECTION!
 **  This sample application uses Objective-C 2.0 garbage collection.
 **  Therefore, the source code in this file does NOT perform manual object memory management.
 **  If you reuse any of this code in a process that isn't garbage collected, you will need to
 **  add all necessary retain/release/autorelease calls, and implement -dealloc methods,
 **  otherwise unpleasant leakage will occur!
 **/


@implementation Bit


- (id) copyWithZone: (NSZone*)zone
{
    // NSLayer isn't copyable, but it is archivable. So create a copy by archiving to
    // a temporary data block, then unarchiving a new layer from that block.
    // One complication is that, due to a bug in Core Animation, CALayer can't archive
    // a pattern-based CGColor. So as a workaround, clear the background before archiving,
    // then restore it afterwards.
    CGColorRef bg = CGColorRetain(self.backgroundColor);
    self.backgroundColor = NULL;
    NSData *data = [NSKeyedArchiver archivedDataWithRootObject: self];
    self.backgroundColor = bg;
    Bit *clone = [NSKeyedUnarchiver unarchiveObjectWithData: data];
    clone.backgroundColor = bg;
    CGColorRelease(bg);

    clone->_owner = _owner;             // _owner is not archived
    return clone;
}


- (NSString*) description
{
    return [NSString stringWithFormat: @"%@[(%g,%g)]", self.class,self.position.x,self.position.y];
}


@synthesize owner=_owner;

- (BOOL) isFriendly         {return _owner.friendly;}
- (BOOL) isUnfriendly       {return _owner.unfriendly;}


- (CGFloat) scale
{
    NSNumber *scale = [self valueForKeyPath: @"transform.scale"];
    return scale.floatValue;
}

- (void) setScale: (CGFloat)scale
{
    [self setValue: [NSNumber numberWithFloat: scale]
        forKeyPath: @"transform.scale"];
}


- (int) rotation
{
    NSNumber *rot = [self valueForKeyPath: @"transform.rotation"];
    return round( rot.doubleValue * 180.0 / M_PI );
}

- (void) setRotation: (int)rotation
{
    [self setValue: [NSNumber numberWithDouble: rotation*M_PI/180.0]
        forKeyPath: @"transform.rotation"];
}


- (BOOL) pickedUp
{
    return self.zPosition >= kPickedUpZ;
}

- (void) setPickedUp: (BOOL)up
{
    if( up != self.pickedUp ) {
        CGFloat shadow, offset, radius, opacity, z, scale;
        if( up ) {
            shadow = 0.8;
            offset = 2;
            radius = 8;
            opacity = 0.9;
            scale = 1.2;
            z = kPickedUpZ;
            _restingZ = self.zPosition;
        } else {
            shadow = offset = radius = 0.0;
            opacity = 1.0;
            scale = 1.0/1.2;
            z = _restingZ;
        }
        
        self.zPosition = z;
        self.shadowOpacity = shadow;
        self.shadowOffset = CGSizeMake(offset,-offset);
        self.shadowRadius = radius;
        self.opacity = opacity;
        self.scale *= scale;
    }
}


- (BOOL)containsPoint:(CGPoint)p
{
    // Make picked-up pieces invisible to hit-testing.
    // Otherwise, while dragging a Bit, hit-testing the cursor position would always return
    // that Bit, since it's directly under the cursor...
    if( self.pickedUp )
        return NO;
    else
        return [super containsPoint: p];
}


-(id<BitHolder>) holder
{
    // Look for my nearest ancestor that's a BitHolder:
    for( CALayer *layer=self.superlayer; layer; layer=layer.superlayer ) {
        if( [layer conformsToProtocol: @protocol(BitHolder)] )
            return (id<BitHolder>)layer;
        else if( [layer isKindOfClass: [Bit class]] )
            return nil;
    }
    return nil;
}


- (void) destroy
{
    // "Pop" the Bit by expanding it 5x as it fades away:
    self.scale = 5;
    self.opacity = 0.0;
    // Removing the view from its superlayer right now would cancel the animations.
    // Instead, defer the removal until sometime shortly after the animations finish:
    [self performSelector: @selector(removeFromSuperlayer) withObject: nil afterDelay: 1.0];
}


@end
