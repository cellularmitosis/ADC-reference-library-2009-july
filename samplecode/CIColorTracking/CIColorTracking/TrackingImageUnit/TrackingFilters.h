/*
Project: TrackingImageUnit

File: TrackingFilters.h

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


#import <Foundation/Foundation.h>
#import <QuartzCore/QuartzCore.h>

@interface TrackingFiltersPlugInLoader : NSObject <CIPlugInRegistration>
{

}

-(BOOL)load:(void*)host;

@end

// Overlays an image (inputDuck) onto the tracked portion (inputLocation, inputOffset) of
// an image (inputImage), scaling (inputScale) the overlay image to account from its
// distance from the camera.
@interface CompositeFilter: CIFilter
{
    CIImage   *inputImage;
    CIImage   *inputOverlayImage;
    CIImage   *inputLocation;
    CIVector  *inputOffset;
    NSNumber  *inputScale;
}

@end

// Computes the centroid of the target color value in an image. 
@interface Centroid: CIFilter
{
    CIImage   *inputImage;
}

@end

// Downsamples the input image to compute the average value.
// Computes the pixel location that corresponds to the centroid. 
@interface CoordinateMaskWithColor: CIFilter
{
    CIImage   *inputImage;
}

@end

// Creates a mask from an image, a color, and a threshold.
// The input color defines the color to track. The input threshold defines the range of 
// color values around the input color that are included in the calculations for tracking. 
@interface MaskFromColor: CIFilter
{
    CIImage   *inputImage;
    CIColor   *inputColor;
    NSNumber  *inputThreshold;
}

@end

// Creates the mean value of an image by downscaling.
@interface GPUGemsAreaMean: CIFilter
{
    CIImage   *inputImage;
	CIVector  *inputExtent;
}

@end
