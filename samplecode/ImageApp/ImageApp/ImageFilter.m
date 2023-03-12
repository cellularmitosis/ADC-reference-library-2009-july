/*

File: ImageFilter.m

Abstract: ImageFilter.m class implementation

Version: <1.0>

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

Copyright © 2005-2008 Apple Inc. All Rights Reserved.

*/

#import "ImageFilter.h"


@implementation ImageFilter

- (void) dealloc
{
    CGImageRelease(mImage);
    [mCIImage release];
    [mProfile release];
    [mCIExposure release];
    [mCIColorControls release];
    [mCIColorCube release];

    [super dealloc];
}


- (id) initWithImage:(CGImageRef)image
{
    if ((self = [super init]))
    {
        mImage = CGImageRetain(image);
        mCIImage = [[CIImage imageWithCGImage:mImage] retain];
    }
    return self;
}

// specify profile for use with image effect transform
//
- (void) setProfile:(Profile*)profile
{
    [mProfile release];
    mProfile = [profile retain];

    if (mProfile == nil)
    {
        [mCIColorCube autorelease];
        mCIColorCube = nil;
    }
    else
    {
        // Use the CIColorCube filter three-dimensional color table 
        // to transform the source image pixels
        if (mCIColorCube == nil)
            mCIColorCube = [[CIFilter filterWithName: @"CIColorCube"] retain];
        
        // Get the transformed data
        static const int kSoftProofGrid = 32;
        NSData *data = [mProfile dataForCISoftproofTextureWithGridSize:kSoftProofGrid];
        
        [mCIColorCube setValue:data
            forKey:@"inputCubeData"];
        [mCIColorCube setValue:[NSNumber numberWithInt:kSoftProofGrid]
            forKey:@"inputCubeDimension"];
    }
}

// Use CIExposureAdjust Color adjustment filter change color values.
// The CIExposureAdjust filter adjusts the exposure setting for an image by mimicking 
// a camera’s F-stop adjustment. 
//
- (void) setExposure:(NSNumber *)exposure
{
    if (mCIExposure == nil)
        mCIExposure = [[CIFilter filterWithName: @"CIExposureAdjust"] retain];

    [mCIExposure setValue:exposure
        forKey: @"inputEV"];
}

// Use the CIColorControls filter to adjust saturation 
//
- (void) setSaturation:(NSNumber *)saturation
{
    if (mCIColorControls == nil)
        mCIColorControls = [[CIFilter filterWithName: @"CIColorControls"] retain];

    // set new saturation value
    [mCIColorControls setValue:saturation
        forKey: @"inputSaturation"];

    // hold brightness unchanged. kCIAttributeIdentity = A value that results 
    // in no effect on the input image.
    [mCIColorControls setValue:[[[mCIColorControls attributes]
                                    objectForKey: @"inputBrightness"]
                                        objectForKey: @"CIAttributeIdentity"]
        forKey: @"inputBrightness"];

    // hold contrast unchanged. kCIAttributeIdentity = A value that results 
    // in no effect on the input image.
    [mCIColorControls setValue:[[[mCIColorControls attributes]
                                    objectForKey: @"inputContrast"]
                                        objectForKey: @"CIAttributeIdentity"]
        forKey: @"inputContrast"];
}


- (CIImage*) imageWithTransform:(CGAffineTransform)ctm
{
    // Returns a new image representing the original image with the transform
    // 'ctm' appended to it.
    CIImage* ciimg = [mCIImage imageByApplyingTransform:ctm];

    // exposure adjustment
    if (mCIExposure)
    {
        [mCIExposure setValue:ciimg forKey:@"inputImage"];
        ciimg = [mCIExposure valueForKey: @"outputImage"];
    }

    // saturation adjustment
    if (mCIColorControls)
    {
        [mCIColorControls setValue:ciimg forKey:@"inputImage"];
        ciimg = [mCIColorControls valueForKey: @"outputImage"];
    }

    // three-dimensional color table adjustment
    if (mCIColorCube)
    {
        [mCIColorCube setValue:ciimg forKey: @"inputImage"];
        ciimg = [mCIColorCube valueForKey: @"outputImage"];
    }

    return ciimg;
}


- (CGImageRef) createCGImage
{
    if (mImage==nil)
        return nil;

    Profile* prof = mProfile;
    if (mProfile==nil)
        prof = [Profile profileWithGenericRGB];

    // calculate bits per pixel and row bytes and alphaInfo
    size_t height = CGImageGetHeight(mImage);
    size_t width = CGImageGetWidth(mImage);
    CGRect rect = {{0,0}, {width, height}};
    size_t bitsPerComponent = 8;
    size_t bytesPerRow = 0;
    CGImageAlphaInfo alphaInfo = kCGImageAlphaNone;

    switch ([prof spaceType])
    {
        case cmGrayData:
            bytesPerRow = width;
            alphaInfo = kCGImageAlphaNone; /* RGB. */
            break;
        case cmRGBData:
            bytesPerRow = width*4;
            alphaInfo = kCGImageAlphaPremultipliedLast; /* premultiplied RGBA */
            break;
        case cmCMYKData:
            bytesPerRow = width*4;
            alphaInfo = kCGImageAlphaNone; /* RGB. */
            break;
        default:
            return nil;
            break;
    }

    CGContextRef context = CGBitmapContextCreate(nil, width, height,
                                    bitsPerComponent, bytesPerRow,
                                    [prof colorspace], alphaInfo);

    CIContext* cicontext = [CIContext contextWithCGContext: context options: nil];

    // If context doesn't support alpha then first fill it with white.
    // That is most likely to be desireable.
    if (alphaInfo == kCGImageAlphaNone)
    {
        CGColorSpaceRef graySpace = CGColorSpaceCreateDeviceGray();
        const float whiteComps[2] = {1.0, 1.0};
        CGColorRef whiteColor = CGColorCreate(graySpace, whiteComps);
        CFRelease(graySpace);
        CGContextSetFillColorWithColor(context, whiteColor);
        CGContextFillRect(context, rect);
        CFRelease(whiteColor);
    }

    CIImage* ciimg = mCIImage;

    // exposure adjustment
    if (mCIExposure)
    {
        [mCIExposure setValue:ciimg forKey:@"inputImage"];
        ciimg = [mCIExposure valueForKey: @"outputImage"];
    }

    // three-dimensional color table adjustment
    if (mCIColorControls)
    {
        [mCIColorControls setValue:ciimg forKey:@"inputImage"];
        ciimg = [mCIColorControls valueForKey: @"outputImage"];
    }


    CGRect extent = [ciimg extent];

    [cicontext drawImage: ciimg inRect:rect fromRect:extent];

    CGImageRef image = CGBitmapContextCreateImage(context);

    CGContextRelease(context);

    return image;
}

@end
