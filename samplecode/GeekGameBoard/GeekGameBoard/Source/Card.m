/*

File: Card.m

Abstract: A card of some type (playing card, Community Chest, money, ...).

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


#import "Card.h"
#import "QuartzUtils.h"


/**  WARNING: THIS CODE REQUIRES GARBAGE COLLECTION!
 **  This sample application uses Objective-C 2.0 garbage collection.
 **  Therefore, the source code in this file does NOT perform manual object memory management.
 **  If you reuse any of this code in a process that isn't garbage collected, you will need to
 **  add all necessary retain/release/autorelease calls, and implement -dealloc methods,
 **  otherwise unpleasant leakage will occur!
 **/


@implementation Card


static CATransform3D kFaceUpTransform, kFaceDownTransform;

+ (void) initialize
{
    if( self==[Card class] ) {
        kFaceUpTransform = kFaceDownTransform = CATransform3DIdentity;
        // Construct a 180-degree rotation matrix:
        kFaceDownTransform.m11 = kFaceDownTransform.m33 = -1;
        // The more obvious way to create kFaceDownTransform would be to call
        // CATransform3DMakeRotation(pi,0,1,0), but due to round-off errors, that transform
        // will have non-zero values in some other places, making it appear to CA as a true
        // 3D transform; this will then cause unexpected clipping behaviors when used.
    }
}


+ (NSRange) serialNumberRange;
{
    NSAssert1(NO,@"%@ forgot to override +serialNumberRange",self);
    return NSMakeRange(0,0);
}


- (id) initWithSerialNumber: (int)serial position: (CGPoint)pos
{
    self = [super init];
    if (self != nil) {
        _serialNumber = serial;
        self.bounds = CGRectMake(0,0,kCardWidth,kCardHeight);
        self.position = pos;
        self.edgeAntialiasingMask = 0;
        _back = [self createBack];
        [self addSublayer: _back];
        _front = [self createFront];
        _front.transform = kFaceDownTransform;
        [self addSublayer: _front];
    }
    return self;
}


- (void)encodeWithCoder:(NSCoder *)aCoder
{
    [super encodeWithCoder: aCoder];
    [aCoder encodeInt: _serialNumber forKey: @"serialNumber"];
}

- (id)initWithCoder:(NSCoder *)aDecoder
{
    self = [super initWithCoder: aDecoder];
    if( self ) {
        _serialNumber = [aDecoder decodeIntForKey: @"serialNumber"];
    }
    return self;
}


- (NSString*) description
{
    return [NSString stringWithFormat: @"%@[#%i]",self.class,_serialNumber];
}


@synthesize serialNumber=_serialNumber;


- (BOOL) faceUp
{
    return _faceUp;
}

- (void) setFaceUp: (BOOL)up
{
    if( up != _faceUp ) {
        // The Card has separate sub-layers for its front and back. At any time, one of them
        // is hidden, by having a 180 degree rotation about the Y axis.
        // To flip the card, both front and back layers are flipped over.
        CATransform3D xform;
        xform = up ?kFaceUpTransform :kFaceDownTransform;
        _front.transform = xform;
        
        xform = up ?kFaceDownTransform :kFaceUpTransform;
        _back.transform = xform;
        _faceUp = up;
    }
}


- (CALayer*) createFront
{
    CALayer *front = [[CALayer alloc] init];
    front.bounds = CGRectMake(0,0,kCardWidth,kCardHeight);
    front.position = CGPointMake(kCardWidth/2,kCardHeight/2);
    front.edgeAntialiasingMask = 0;
    front.backgroundColor = kWhiteColor;
    front.cornerRadius = 8;
    front.borderWidth = 1;
    front.borderColor = CGColorCreateGenericGray(0.7, 1.0);
    front.doubleSided = NO;         // this makes the layer invisible when it's flipped
    return front;
}


- (CALayer*) createBack
{
    CGSize size = self.bounds.size;
    CALayer *back = [[CALayer alloc] init];
    back.bounds = CGRectMake(0,0,size.width,size.height);
    back.position = CGPointMake(kCardWidth/2,kCardHeight/2);
    back.contents = (id) GetCGImageNamed(@"/Library/Desktop Pictures/Classic Aqua Blue.jpg");
    back.contentsGravity = kCAGravityResize;
    back.masksToBounds = YES;
    back.borderWidth = 4;
    back.borderColor = kWhiteColor;
    back.cornerRadius = 8;
    back.edgeAntialiasingMask = 0;
    back.doubleSided = NO;          // this makes the layer invisible when it's flipped
    
    CATextLayer *label = AddTextLayer(back, @"\u2603",          // Unicode snowman character
                                      [NSFont systemFontOfSize: 0.9*size.width],
                                      kCALayerWidthSizable|kCALayerHeightSizable);
    label.foregroundColor = CGColorCreateGenericGray(1.0,0.5);
    return back;
}    


#pragma mark -
#pragma mark DRAG-AND-DROP:


// An image from another app can be dragged onto a Card to change its background. */


- (NSDragOperation)draggingEntered:(id <NSDraggingInfo>)sender
{
    NSPasteboard *pb = [sender draggingPasteboard];
    if( [NSImage canInitWithPasteboard: pb] )
        return NSDragOperationCopy;
    else
        return NSDragOperationNone;
}

- (BOOL)performDragOperation:(id <NSDraggingInfo>)sender
{
    CGImageRef image = GetCGImageFromPasteboard([sender draggingPasteboard]);
    if( image ) {
        CALayer *face = _faceUp ?_front :_back;
        face.contents = (id) image;
        face.contentsGravity = kCAGravityResizeAspectFill;
        face.masksToBounds = YES;
        return YES;
    } else
        return NO;
}


@end
