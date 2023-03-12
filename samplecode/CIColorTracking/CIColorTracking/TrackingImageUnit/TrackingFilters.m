/*
Project: TrackingImageUnit

File: TrackingFilters.m

Abstract: 
This is the implementation file for TrackingFilters, which defines thee filters needed by CIColorTraking program:  
CompositeFilter, Centeroid, CoordinateMaskWithColor, MaskFromColor, and GPUGemsAreaMean. Each of these filters is
a subclass of CIFilter. The file also implements the TrackingFiltersPlugInLoader class to takes care of registring the filters.  
When you build the TrackingImageUnit project, you'll get a plug-in (an image unit) that contains the filters. You'll need to copy that
image unit to /Library/Graphics/Image Units.

Version 1.0

Disclaimer: IMPORTANT:  This Apple software is supplied to you by Apple Inc.
("Apple") in consideration of your agreement to the following terms, and your
use, installation, modification or redistribution of this Apple software
constitutes acceptance of these terms.  If you do not agree with these terms,
please do not use, install, modify or redistribute this Apple software.

In consideration of your agreement to abide by the following terms, and subject
to these terms, Apple grants you a personal, non-exclusive license, under
Apple's copyrights in this original Apple software (the "Apple Software"), to
use, reproduce, modify and redistribute the Apple Software, with or without
modifications, in source and/or binary forms; provided that if you redistribute
the Apple Software in its entirety and without modifications, you must retain
this notice and the following text and disclaimers in all such redistributions
of the Apple Software.
Neither the name, trademarks, service marks or logos of Apple Inc. may be used
to endorse or promote products derived from the Apple Software without specific
prior written permission from Apple.  Except as expressly stated in this notice,
no other rights or licenses, express or implied, are granted by Apple herein,
including but not limited to any patent rights that may be infringed by your
derivative works or by other works in which the Apple Software may be
incorporated.

The Apple Software is provided by Apple on an "AS IS" basis.  APPLE MAKES NO
WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED
WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS FOR A PARTICULAR
PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND OPERATION ALONE OR IN
COMBINATION WITH YOUR PRODUCTS.

IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION, MODIFICATION AND/OR
DISTRIBUTION OF THE APPLE SOFTWARE, HOWEVER CAUSED AND WHETHER UNDER THEORY OF
CONTRACT, TORT (INCLUDING NEGLIGENCE), STRICT LIABILITY OR OTHERWISE, EVEN IF
APPLE HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

Copyright (C) 2008 Apple Inc. All Rights Reserved.

*/

// The following filters provide the main work behind the color tracking.
// A detailed explanation of the whole math behind it can be found in the
// GPU Gems 3 book.

#import "TrackingFilters.h"

// Kernels to be used by the filters. These get only loaded once.
static CIKernel *coordinateMask = nil;
static CIKernel *sampleAndScale = nil;
static CIKernel *centroid = nil;
static CIKernel *overlayComposite = nil;
static CIKernel *maskFromColor = nil;
static CIKernel *scaleXY4 = nil;
static CIKernel *scaleX2 = nil;
static CIKernel *scaleY2 = nil;


@implementation TrackingFiltersPlugInLoader

-(BOOL)load: (void*)host
{
	// load the kernels if not already done
    if(!coordinateMask)
    {
        NSBundle    *bundle  = [NSBundle bundleForClass: [self class]];
		NSString    *code    = [NSString stringWithContentsOfFile: [bundle
            pathForResource: @"TrackingKernels" ofType:@"cikernel"]];
        NSArray     *kernels = [CIKernel kernelsWithString: code];

        maskFromColor        = [[kernels objectAtIndex: 0] retain];
        sampleAndScale       = [[kernels objectAtIndex: 1] retain];
        coordinateMask       = [[kernels objectAtIndex: 2] retain];
        centroid			 = [[kernels objectAtIndex: 3] retain];
        overlayComposite     = [[kernels objectAtIndex: 4] retain];
        scaleXY4             = [[kernels objectAtIndex: 5] retain];
        scaleX2              = [[kernels objectAtIndex: 6] retain];
        scaleY2              = [[kernels objectAtIndex: 7] retain];
    }

    return YES;
}

@end

@implementation CompositeFilter

// ROI function. See Image Unit Tutorial on the ADC site for explanation.
- (CGRect)regionOf: (int)sampler  destRect: (CGRect)rect  userInfo: (id)info
{
    if(sampler == 2)  return CGRectMake(0,0, 1,1);
	if(sampler == 1)  return CGRectInfinite;
    return rect;
}

- (CIImage *)outputImage
{
    CISampler  *src, *overlay, *location;
    float       w,h;

    src      = [CISampler samplerWithImage: inputImage];
    overlay  = [CISampler samplerWithImage: inputOverlayImage];
    location = [CISampler samplerWithImage: inputLocation];
    w        = CGRectGetWidth([inputImage extent]);
    h        = CGRectGetHeight([inputImage extent]);

    return [self apply: overlayComposite, src, overlay, location, [CIVector vectorWithX: w  Y: h],
        inputOffset, inputScale, kCIApplyOptionDefinition, [src definition], nil];
}

// customAttributes defines the default values for the parameters/attributes of the filter
// as well as information of the possible range for the given parameter.
- (NSDictionary *)customAttributes
{
    return [NSDictionary dictionaryWithObjectsAndKeys:

        [NSDictionary dictionaryWithObjectsAndKeys:
            [CIVector vectorWithX:145.0 Y:50.0],   kCIAttributeDefault,
            kCIAttributeTypePosition,           kCIAttributeType,
            nil],                               @"inputOffset",

        [NSDictionary dictionaryWithObjectsAndKeys:
            [NSNumber numberWithDouble: 0.00], kCIAttributeSliderMin,
            [NSNumber numberWithDouble:10.00], kCIAttributeSliderMax,
            [NSNumber numberWithDouble: 8.00], kCIAttributeDefault,
            kCIAttributeTypeScalar,            kCIAttributeType,
            nil],                              @"inputScale",

        nil];
}

@end


@implementation Centroid

- (CIImage *)outputImage
{
    CISampler         *src;


    src     = [CISampler samplerWithImage: inputImage];
    return [self apply: centroid, src, kCIApplyOptionDefinition, [src definition], nil];
}

@end

@implementation CoordinateMaskWithColor

- (CIImage *)outputImage
{
    CISampler  *src;
    CIVector   *invSize;

    invSize = [CIVector vectorWithX: 1.0/CGRectGetWidth([inputImage extent])  Y: 1.0/CGRectGetHeight([inputImage extent])];
    src     = [CISampler samplerWithImage: inputImage];

    return [self apply: coordinateMask, src, invSize, kCIApplyOptionDefinition, [src definition], nil];
}

// customAttributes defines the default values for the parameters/attributes of the filter
// as well as information of the possible range for the given parameter.
- (NSDictionary *)customAttributes
{
    return [NSDictionary dictionaryWithObjectsAndKeys:

        [NSDictionary dictionaryWithObjectsAndKeys:
            [CIVector vectorWithX:0.0 Y:0.0],   kCIAttributeDefault,
            kCIAttributeTypePosition,           kCIAttributeType,
            nil],                               @"invSize",

        nil];
}

@end

@implementation MaskFromColor

- (CIImage *)outputImage
{
    CISampler  *src;
    float       w,h;

    src     = [CISampler samplerWithImage: inputImage];
    w       = CGRectGetWidth([inputImage extent]);
    h       = CGRectGetHeight([inputImage extent]);

    return [self apply: maskFromColor, src, inputColor, inputThreshold, kCIApplyOptionDefinition, [src definition], nil];
}

- (NSDictionary *)customAttributes
{
    return [NSDictionary dictionaryWithObjectsAndKeys:

        [NSDictionary dictionaryWithObjectsAndKeys:
            [CIColor colorWithRed: .9  green: .34  blue: .75],  kCIAttributeDefault,
            nil],                              @"inputColor",

        [NSDictionary dictionaryWithObjectsAndKeys:
            [NSNumber numberWithDouble: 0.00], kCIAttributeSliderMin,
            [NSNumber numberWithDouble: 1.00], kCIAttributeSliderMax,
            [NSNumber numberWithDouble: 0.02], kCIAttributeDefault,
            kCIAttributeTypeScalar,            kCIAttributeType,
            nil],                              @"inputThreshold",

        nil];
}

@end


@implementation GPUGemsAreaMean

// ROI function. See Image Unit Tutorial on the ADC site for explanation.
- (CGRect)regionOf: (int)sampler  destRect: (CGRect)r
    userInfo: (CIVector *)scale
{
    float   sx,sy, xMin,xMax, yMin,yMax;

    sx   = [scale X];
    sy   = [scale Y];
    xMin = floorf(CGRectGetMinX(r)*sx);
    xMax = ceilf(CGRectGetMaxX(r)*sx);
    yMin = floorf(CGRectGetMinY(r)*sy);
    yMax = ceilf(CGRectGetMaxY(r)*sy);

    r.origin.x    = xMin;
    r.size.width  = xMax - xMin;
    r.origin.y    = yMin;
    r.size.height = yMax - yMin;
    return r;
}

- (CIImage *)outputImage
{
    CISampler      *src;
    CIImage        *img = [inputImage imageByApplyingTransform: CGAffineTransformMakeTranslation(-[inputExtent X], -[inputExtent Y])];
    CIFilterShape  *definition;
    float           scale;
	CIFilter		*cropFilter = [CIFilter filterWithName:@"CICrop" keysAndValues:
											@"inputImage", img,
											@"inputRectangle", [CIVector vectorWithX:0.0 Y:0.0 Z:[inputExtent Z] W:[inputExtent W]], 
											nil];

	img = [cropFilter valueForKey: @"outputImage"];

	// Downsample to a 2 pixel image by linear scaling
    scale = 1.0;
    while(CGRectGetMaxX([img extent]) > 2.0  &&  CGRectGetMaxY([img extent]) > 2.0)
    {
        src   = [CISampler samplerWithImage: img
            keysAndValues: kCISamplerFilterMode, kCISamplerFilterLinear, nil];

        scale      = scale * 16.0;
        definition = [[src definition] transformBy:
            CGAffineTransformMakeScale(1.0/4.0, 1.0/4.0)  interior: NO];

        img        = [self apply: scaleXY4, src,
            kCIApplyOptionDefinition, definition,
            kCIApplyOptionUserInfo, [CIVector vectorWithX: 4.0  Y: 4.0], nil];
    }

	// Downsample even further to a single pixel image by linear scaling
    while(CGRectGetMaxY([img extent]) > 1.0)
    {
        src   = [CISampler samplerWithImage: img
            keysAndValues: kCISamplerFilterMode, kCISamplerFilterLinear, nil];

        scale      = scale * 2.0;
        definition = [[src definition] transformBy:
            CGAffineTransformMakeScale(1.0, 1.0/2.0)  interior: NO];

        img        = [self apply: scaleY2, src,
            kCIApplyOptionDefinition, definition,
            kCIApplyOptionUserInfo, [CIVector vectorWithX: 1.0  Y: 2.0], nil];
    }

    while(CGRectGetMaxX([img extent]) > 1.0)
    {
        src   = [CISampler samplerWithImage: img
            keysAndValues: kCISamplerFilterMode, kCISamplerFilterLinear, nil];

        scale      = scale * 2.0;
        definition = [[src definition] transformBy:
            CGAffineTransformMakeScale(1.0/2.0, 1.0)  interior: NO];

        img        = [self apply: scaleX2, src,
            kCIApplyOptionDefinition, definition,
            kCIApplyOptionUserInfo, [CIVector vectorWithX: 2.0  Y: 1.0], nil];
    }

    scale = scale / ([inputExtent Z] * [inputExtent W]);
    if(scale != 1.0)
    {
        CIFilter  *filter;

        filter = [CIFilter filterWithName: @"CIColorMatrix"  keysAndValues:
            @"inputImage", img,
            @"inputRVector", [CIVector vectorWithX: scale  Y: 0  Z: 0  W: 0],
            @"inputGVector", [CIVector vectorWithX: 0  Y: scale  Z: 0  W: 0],
            @"inputBVector", [CIVector vectorWithX: 0  Y: 0  Z: scale  W: 0],
            @"inputAVector", [CIVector vectorWithX: 0  Y: 0  Z: 0  W: scale],
            @"inputBiasVector", [CIVector vectorWithX: 0  Y: 0  Z: 0  W: 0],
            nil];

        img    = [filter valueForKey: @"outputImage"];
    }

    return img;
}

// customAttributes defines the default values for the parameters/attributes of the filter
// as well as information of the possible range for the given parameter.
- (NSDictionary *)customAttributes
{
    return [NSDictionary dictionaryWithObjectsAndKeys:

        [NSDictionary dictionaryWithObjectsAndKeys:
            kCIAttributeTypeRectangle,                kCIAttributeType,
            [CIVector vectorWithX:0 Y:0 Z:300 W:300], kCIAttributeDefault,
            nil],                                     @"inputExtent",

        nil];
}

@end
