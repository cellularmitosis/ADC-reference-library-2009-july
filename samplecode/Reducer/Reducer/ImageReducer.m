/*
    File:  ImageReducer.m

    Abstract:  Demonstrates basic image processing using Core Image + AppKit
     
    Version:  1.0

    Copyright:  © Copyright 2005 Apple Computer, Inc. All rights reserved.

    Disclaimer: IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc.
    ("Apple") in consideration of your agreement to the following terms, and your
    use, installation, modification or redistribution of this Apple software
    constitutes acceptance of these terms.  If you do not agree with these terms,
    please do not use, install, modify or redistribute this Apple software.

    In consideration of your agreement to abide by the following terms, and subject
    to these terms, Apple grants you a personal, non-exclusive license, under Apple’s
    copyrights in this original Apple software (the "Apple Software"), to use,
    reproduce, modify and redistribute the Apple Software, with or without
    modifications, in source and/or binary forms; provided that if you redistribute
    the Apple Software in its entirety and without modifications, you must retain
    this notice and the following text and disclaimers in all such redistributions of
    the Apple Software.  Neither the name, trademarks, service marks or logos of
    Apple Computer, Inc. may be used to endorse or promote products derived from the
    Apple Software without specific prior written permission from Apple.  Except as
    expressly stated in this notice, no other rights or licenses, express or implied,
    are granted by Apple herein, including but not limited to any patent rights that
    may be infringed by your derivative works or by other works in which the Apple
    Software may be incorporated.

    The Apple Software is provided by Apple on an "AS IS" basis.  APPLE MAKES NO
    WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED
    WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS FOR A PARTICULAR
    PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND OPERATION ALONE OR IN
    COMBINATION WITH YOUR PRODUCTS.

    IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL OR
    CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
    GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
    ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION, MODIFICATION AND/OR DISTRIBUTION
    OF THE APPLE SOFTWARE, HOWEVER CAUSED AND WHETHER UNDER THEORY OF CONTRACT, TORT
    (INCLUDING NEGLIGENCE), STRICT LIABILITY OR OTHERWISE, EVEN IF APPLE HAS BEEN
    ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#import "ImageReducer.h"
#import "NSBezierPathAdditions.h"
#import <QuartzCore/QuartzCore.h>

static NSBitmapImageRep *BitmapImageRepFromNSImage(NSImage *nsImage);

@interface ImageReducer (Internals)
- (void)updateOutputImage;

- (void)clearFilterChain;
- (void)addFilter:(CIFilter *)aFilter;
@end

@implementation ImageReducer

+ (void)initialize {
    if (self == [ImageReducer class]) {
        [self setKeys:[NSArray arrayWithObjects:@"inputBitmapImageRep", @"maxPixelsWide", @"maxPixelsHigh", @"addsShadow", nil] triggerChangeNotificationsForDependentKey:@"outputPixelsWide"];
        [self setKeys:[NSArray arrayWithObjects:@"inputBitmapImageRep", @"maxPixelsWide", @"maxPixelsHigh", @"addsShadow", nil] triggerChangeNotificationsForDependentKey:@"outputPixelsHigh"];
    }
}

- (void)observeValueForKeyPath:(NSString *)keyPath ofObject:(id)object change:(NSDictionary *)change context:(void *)context {
    if (object == unsharpMaskFilter || object == shadow) {
        [self updateOutputImage];
    }
}

- init {
    self = [super init];
    if (self != nil) {
        scalesImage = YES;
        [self setMaxPixelsWide:400];
        [self setMaxPixelsHigh:1000];
        sharpensImage = YES;
        drawBorder = YES;
        borderCornerRadius = 0.0;
        borderThickness = 2.0;
        borderColor = [[NSColor whiteColor] retain];
        addsShadow = YES;
        shadow = [[NSShadow alloc] init];
        [shadow setShadowOffset:NSMakeSize(6, -4)];
        [shadow setShadowBlurRadius:9.0];

        scaleTransformFilter = [[CIFilter filterWithName:@"CILanczosScaleTransform"] retain];
        [scaleTransformFilter setDefaults];

        unsharpMaskFilter = [[CIFilter filterWithName:@"CIUnsharpMask"] retain];
        [unsharpMaskFilter setDefaults];

        [unsharpMaskFilter setValue:[scaleTransformFilter valueForKey:@"outputImage"] forKey:@"inputImage"];

        [unsharpMaskFilter addObserver:self forKeyPath:@"inputIntensity" options:0 context:NULL];
        [unsharpMaskFilter addObserver:self forKeyPath:@"inputRadius" options:0 context:NULL];

        [shadow addObserver:self forKeyPath:@"shadowBlurRadius" options:0 context:NULL];
    }
    return self;
}

- (void)dealloc {
    [inputBitmapImageRep release];
    [inputCIImage release];
    [scaleTransformFilter release];
    [unsharpMaskFilter release];
    [borderColor release];
    [shadow release];
    [super dealloc];
}

#pragma mark -
#pragma mark *** Input Image Accessors ***

- (NSBitmapImageRep *)inputBitmapImageRep {
    return [[inputBitmapImageRep retain] autorelease];
}

- (void)setInputBitmapImageRep:(NSBitmapImageRep *)newInputBitmapImageRep {
    if (inputBitmapImageRep != newInputBitmapImageRep) {

        [inputBitmapImageRep release];
        inputBitmapImageRep = [newInputBitmapImageRep retain];

        // Create a CIImage from the NSBitmapImageRep, and make it inputImage to the first CIFilter in our filter pipeline.
        [inputCIImage release];
        inputCIImage = [[CIImage alloc] initWithBitmapImageRep:inputBitmapImageRep];
        [scaleTransformFilter setValue:inputCIImage forKey:@"inputImage"];

        // Update the result image.
        [self updateOutputImage];
    }
}

#pragma mark -
#pragma mark *** Stage 1: Resampling Parameters ***

- (BOOL)scalesImage {
    return scalesImage;
}

- (void)setScalesImage:(BOOL)flag {
    if (scalesImage != flag) {
        scalesImage = flag;
        [self updateOutputImage];
    }
}

- (int)maxPixelsWide {
    return maxPixelsWide;
}

- (void)setMaxPixelsWide:(int)newMaxPixelsWide {
    if (maxPixelsWide != newMaxPixelsWide) {
        maxPixelsWide = newMaxPixelsWide;
        [self updateOutputImage];
    }
}

- (int)maxPixelsHigh {
    return maxPixelsHigh;
}

- (void)setMaxPixelsHigh:(int)newMaxPixelsHigh {
    if (maxPixelsHigh != newMaxPixelsHigh) {
        maxPixelsHigh = newMaxPixelsHigh;
        [self updateOutputImage];
    }
}

#pragma mark -
#pragma mark *** Stage 2: Sharpening Parameters ***

- (BOOL)sharpensImage {
    return sharpensImage;
}

- (void)setSharpensImage:(BOOL)flag {
    if (sharpensImage != flag) {
        sharpensImage = flag;
        [self updateOutputImage];
    }
}

#pragma mark -
#pragma mark *** Stage 3: Border and Background Parameters ***

- (BOOL)drawBorder {
    return drawBorder;
}

- (void)setDrawBorder:(BOOL)flag {
    if (drawBorder != flag) {
        drawBorder = flag;
        [self updateOutputImage];
    }
}

- (float)borderCornerRadius {
    return borderCornerRadius;
}

- (void)setBorderCornerRadius:(float)newBorderCornerRadius {
    if (borderCornerRadius != newBorderCornerRadius) {
        borderCornerRadius = newBorderCornerRadius;
        [self updateOutputImage];
    }
}

- (float)borderThickness {
    return borderThickness;
}

- (void)setBorderThickness:(float)newBorderThickness {
    if (borderThickness != newBorderThickness) {
        borderThickness = newBorderThickness;
        [self updateOutputImage];
    }
}

- (NSColor *)borderColor {
    return [[borderColor retain] autorelease];
}

- (void)setBorderColor:(NSColor *)newBorderColor {
    if (borderColor != newBorderColor) {
        [borderColor release];
        borderColor = [newBorderColor copy];

        [self updateOutputImage];
    }
}

#pragma mark -
#pragma mark *** Stage 4: Shadow Parameters ***

- (BOOL)addsShadow {
    return addsShadow;
}

- (void)setAddsShadow:(BOOL)flag {
    if (addsShadow != flag) {
        addsShadow = flag;
        [self updateOutputImage];
    }
}

- (NSShadow *)shadow {
    return shadow;
}

#pragma mark -
#pragma mark *** Filter Accessors ***

- (float)scaleFactor {
    NSBitmapImageRep *sourceBitmap = [self inputBitmapImageRep];
    if (sourceBitmap != nil && [self scalesImage]) {
        // Determine scale factor needed to fit within size constraints.
        float widthScale = (float)[self maxPixelsWide] / (float)[sourceBitmap pixelsWide];
        float heightScale = (float)[self maxPixelsHigh] / (float)[sourceBitmap pixelsHigh];
        return MIN(1.0, MIN(widthScale, heightScale));
    } else {
        return 1.0;
    }
}

- (CIFilter *)unsharpMaskFilter {
    return unsharpMaskFilter;
}

#pragma mark -
#pragma mark *** Output Image Accessors ***

- (NSBitmapImageRep *)outputBitmapImageRep {
    NSBitmapImageRep *bitmapImageRep = nil;

    CIImage *ciImage = [self outputCIImage];
    if (ciImage != nil) {

        // Get the CIImage's extents.  The filters we're using in this example should always produce an output image of finite extent, but in the general case one needs to account for the possibility of the output image being infinite in extent.
        CGRect extent = [ciImage extent];
        if (CGRectIsInfinite(extent)) {
            extent.size.width = 1024;
            extent.size.height = 1024;
            NSLog(@"-[%@ %s] Trimmed infinite rect to arbitrary finite rect", self, _cmd);
        }

        // Compute size of output bitmap.
        NSSize outputBitmapSize = NSMakeSize([self outputPixelsWide], [self outputPixelsHigh]);

        // Create a new NSBitmapImageRep that matches the CIImage's extents.
        bitmapImageRep = [[NSBitmapImageRep alloc] initWithBitmapDataPlanes:NULL pixelsWide:outputBitmapSize.width pixelsHigh:outputBitmapSize.height bitsPerSample:8 samplesPerPixel:4 hasAlpha:YES isPlanar:NO colorSpaceName:NSDeviceRGBColorSpace bytesPerRow:0 bitsPerPixel:0];

        // Create an NSGraphicsContext that draws into the NSBitmapImageRep, and make it current.
        NSGraphicsContext *nsContext = [NSGraphicsContext graphicsContextWithBitmapImageRep:bitmapImageRep];
        [NSGraphicsContext saveGraphicsState];
        [NSGraphicsContext setCurrentContext:nsContext];

        // Clear the bitmap to zero alpha.
        [[NSColor clearColor] set];
        NSRectFill(NSMakeRect(0, 0, [bitmapImageRep pixelsWide], [bitmapImageRep pixelsHigh]));

        // Decide where the image will go.
        CGRect imageDestinationRect = CGRectMake(0.0, [bitmapImageRep pixelsHigh] - extent.size.height, extent.size.width, extent.size.height);

        // Create the path for the border (if any).
        NSBezierPath *borderPath = nil;
        [NSGraphicsContext saveGraphicsState];
        if (drawBorder) {
            borderPath = [NSBezierPath bezierPathWithRoundedRect:NSInsetRect(*(NSRect *)&imageDestinationRect, 0.5 * borderThickness, 0.5 * borderThickness) cornerRadius:borderCornerRadius];

            if (addsShadow) {
                [NSGraphicsContext saveGraphicsState];
                [shadow set];
                [[NSColor whiteColor] set];
                [borderPath fill];
                [NSGraphicsContext restoreGraphicsState];
            }

            [borderPath addClip];
        } else if (addsShadow) {
            borderPath = [NSBezierPath bezierPathWithRect:*(NSRect *)&imageDestinationRect];
            [NSGraphicsContext saveGraphicsState];
            [shadow set];
            [[NSColor whiteColor] set];
            [borderPath fill];
            [NSGraphicsContext restoreGraphicsState];
        }

        // Get a CIContext from the NSGraphicsContext, and use it to draw the CIImage into the NSBitmapImageRep.
        CIContext *ciContext = [nsContext CIContext];
        [ciContext drawImage:ciImage atPoint:imageDestinationRect.origin fromRect:extent];
        [NSGraphicsContext restoreGraphicsState];

        // Draw the border path.
        if (drawBorder && borderPath) {
            [borderPath setLineWidth:borderThickness];
            [borderColor set];
            [borderPath stroke];
        }

        // Restore the previous NSGraphicsContext.
        [NSGraphicsContext restoreGraphicsState];
    }

    // Return the new NSBitmapImageRep.
    return [bitmapImageRep autorelease];
}

- (int)outputPixelsWide {
    int result;
    if (addsShadow) {
        result = ceil([self scaleFactor] * [[self inputBitmapImageRep] pixelsWide] + fabs([shadow shadowOffset].width) + [shadow shadowBlurRadius]);
    } else {
        result = floor([self scaleFactor] * [[self inputBitmapImageRep] pixelsWide]);
    }
    return result;
}

- (int)outputPixelsHigh {
    int result;
    if (addsShadow) {
        result = ceil([self scaleFactor] * [[self inputBitmapImageRep] pixelsHigh] + fabs([shadow shadowOffset].height) + [shadow shadowBlurRadius]);
    } else {
        result = floor([self scaleFactor] * [[self inputBitmapImageRep] pixelsHigh]);
    }
    return result;
}

- (CIImage *)outputCIImage {
    return (finalFilter != nil && [self inputBitmapImageRep] != nil) ? [finalFilter valueForKey:@"outputImage"] : inputCIImage;
}

@end

@implementation ImageReducer (NonPrimitiveMethods)

- (void)setInputImage:(NSImage *)newInputImage {    
    [self setInputBitmapImageRep:BitmapImageRepFromNSImage(newInputImage)];
}

- (NSImage *)outputImage {
    NSImage *result = nil;
    if ([self inputBitmapImageRep] != nil) {
        CIImage *outputCIImage = [self outputCIImage];
        if (outputCIImage != nil) {
            CGRect extent = [outputCIImage extent];
            if (CGRectIsInfinite(extent)) {
                NSLog(@"*** OUTPUT IMAGE HAS INFINITE EXTENT ***");
            } else {
                result = [[NSImage alloc] initWithSize:NSMakeSize(extent.size.width, extent.size.height)];
                NSCIImageRep *ciImageRep = [NSCIImageRep imageRepWithCIImage:outputCIImage];
                [result addRepresentation:ciImageRep];
                [result autorelease];
            }
        }
    }
    return result;
}

@end

@implementation ImageReducer (Internals)

- (void)updateOutputImage {
    [self willChangeValueForKey:@"outputCIImage"];

    [self clearFilterChain];

    if ([self scalesImage]) {
        [scaleTransformFilter setValue:[NSNumber numberWithFloat:[self scaleFactor]] forKey:@"inputScale"];
        [self addFilter:scaleTransformFilter];
    }

    if ([self sharpensImage]) {
        [self addFilter:unsharpMaskFilter];
    }
    
    [self didChangeValueForKey:@"outputCIImage"];
}

- (void)clearFilterChain {
    finalFilter = nil;
}

- (void)addFilter:(CIFilter *)aFilter {
    [aFilter setValue:(finalFilter ? [finalFilter valueForKey:@"outputImage"] : inputCIImage) forKey:@"inputImage"];
    finalFilter = aFilter;
}

@end

static NSBitmapImageRep *BitmapImageRepFromNSImage(NSImage *nsImage) {
    // See if the NSImage has an NSBitmapImageRep.  If so, return the first NSBitmapImageRep encountered.  An NSImage that is initialized by loading the contents of a bitmap image file (such as JPEG, TIFF, or PNG) and, not subsequently rescaled, will usually have a single NSBitmapImageRep.
    NSEnumerator *enumerator = [[nsImage representations] objectEnumerator];
    NSImageRep *representation;
    while (representation = [enumerator nextObject]) {
        if ([representation isKindOfClass:[NSBitmapImageRep class]]) {
            return (NSBitmapImageRep *)representation;
        }
    }

    // If we didn't find an NSBitmapImageRep (perhaps because we received a PDF image), we can create one using one of two approaches: (1) lock focus on the NSImage, and create the bitmap using -[NSBitmapImageRep initWithFocusedViewRect:], or (2) (Tiger and later) create an NSBitmapImageRep, and an NSGraphicsContext that draws into it using +[NSGraphicsContext graphicsContextWithBitmapImageRep:], and composite the NSImage into the bitmap graphics context.  We'll use approach (1) here, since it is simple and supported on all versions of Mac OS X.
    NSSize size = [nsImage size];
    [nsImage lockFocus];
    NSBitmapImageRep *bitmapImageRep = [[NSBitmapImageRep alloc] initWithFocusedViewRect:NSMakeRect(0, 0, size.width, size.height)];
    [nsImage unlockFocus];

    return [bitmapImageRep autorelease];
}
