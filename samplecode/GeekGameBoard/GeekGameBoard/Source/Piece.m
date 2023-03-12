/*

File: Piece.m

Abstract: A playing piece. A concrete subclass of Bit that displays an image..

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


#import "Piece.h"
#import "QuartzUtils.h"


/**  WARNING: THIS CODE REQUIRES GARBAGE COLLECTION!
 **  This sample application uses Objective-C 2.0 garbage collection.
 **  Therefore, the source code in this file does NOT perform manual object memory management.
 **  If you reuse any of this code in a process that isn't garbage collected, you will need to
 **  add all necessary retain/release/autorelease calls, and implement -dealloc methods,
 **  otherwise unpleasant leakage will occur!
 **/


@implementation Piece


- (id) initWithImageNamed: (NSString*)imageName
                    scale: (CGFloat)scale
{
    self = [super init];
    if (self != nil) {
        _imageName = imageName;
        [self setImage: GetCGImageNamed(imageName) scale: scale];
        self.zPosition = kPieceZ;
    }
    return self;
}


- (id) initWithCoder: (NSCoder*)decoder
{
    self = [super initWithCoder: decoder];
    if( self ) {
        _imageName = [decoder decodeObjectForKey: @"imageName"];
        // (actual image (self.contents) was already restord by superclass)
    }
    return self;
}

- (void) encodeWithCoder: (NSCoder*)coder
{
    [super encodeWithCoder: coder];
    [coder encodeObject: _imageName forKey: @"imageName"];
}


- (NSString*) description
{
    return [NSString stringWithFormat: @"%@[%@]", 
            [self class],
            _imageName.lastPathComponent.stringByDeletingPathExtension];
}


@synthesize imageName=_imageName;


- (void) setImage: (CGImageRef)image scale: (CGFloat)scale
{
    self.contents = (id) image;
    self.contentsGravity = @"resize";
    self.minificationFilter = kCAFilterLinear;
    int width = CGImageGetWidth(image), height = CGImageGetHeight(image);
    if( scale > 0 ) {
        if( scale >= 4.0 )
            scale /= MAX(width,height);             // interpret scale as target dimensions
        width = ceil( width * scale);
        height= ceil( height* scale);
    }
    self.bounds = CGRectMake(0,0,width,height);
    _imageName = nil;
}

- (void) setImage: (CGImageRef)image
{
    CGSize size = self.bounds.size;
    [self setImage: image scale: MAX(size.width,size.height)];
}

- (void) setImageNamed: (NSString*)name
{
    [self setImage: GetCGImageNamed(name)];
    _imageName = name;
}


- (BOOL)containsPoint:(CGPoint)p
{
    // Overrides CGLayer's implementation,
    // returning YES only for pixels at which this layer's alpha is at least 0.5.
    // This takes into account the opacity, bg color, and background image's alpha channel.
    if( ! [super containsPoint: p] )
        return NO;
    float opacity = self.opacity;
    if( opacity < 0.5 )
        return NO;
    float thresholdAlpha = 0.5 / self.opacity;
    
    CGColorRef bg = self.backgroundColor;
    float alpha = bg ?CGColorGetAlpha(bg) :0.0;
    if( alpha < thresholdAlpha ) {
        CGImageRef image = (CGImageRef)self.contents;
        if( image ) {
            // Note: This makes the convenient assumption that the image entirely fills the bounds.
            alpha = MAX(alpha, GetPixelAlpha(image, self.bounds.size, p));
        }
    }
    return alpha >= thresholdAlpha;
}


@end
