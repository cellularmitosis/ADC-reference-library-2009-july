/*
    File:  ImageReducer.h

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

#import <Cocoa/Cocoa.h>

@class CIFilter;
@class CIImage;

@interface ImageReducer : NSObject
{
    // Input Image
    NSBitmapImageRep *inputBitmapImageRep;
    CIImage *inputCIImage;

    // Filter Chain Bookkeeping
    CIFilter *finalFilter;

    // Stage 1: Resampling Parameters
    BOOL scalesImage;
    int maxPixelsWide;
    int maxPixelsHigh;
    CIFilter *scaleTransformFilter;

    // Stage 2: Sharpening Parameters
    BOOL sharpensImage;
    CIFilter *unsharpMaskFilter;

    // Stage 3: Border and Background Parameters
    BOOL drawBorder;
    float borderCornerRadius;
    float borderThickness;
    NSColor *borderColor;

    // Stage 4: Shadow Parameters
    BOOL addsShadow;
    NSShadow *shadow;
}

#pragma mark *** Initializers ***

- init;

#pragma mark -
#pragma mark *** Input Image Accessors ***

- (NSBitmapImageRep *)inputBitmapImageRep;
- (void)setInputBitmapImageRep:(NSBitmapImageRep *)newInputBitmapImageRep;

#pragma mark -
#pragma mark *** Stage 1: Resampling Parameters ***

- (BOOL)scalesImage;
- (void)setScalesImage:(BOOL)flag;

- (int)maxPixelsWide;
- (void)setMaxPixelsWide:(int)newMaxPixelsWide;

- (int)maxPixelsHigh;
- (void)setMaxPixelsHigh:(int)newMaxPixelsHigh;

#pragma mark -
#pragma mark *** Stage 2: Sharpening Parameters ***

- (BOOL)sharpensImage;
- (void)setSharpensImage:(BOOL)flag;

#pragma mark -
#pragma mark *** Stage 3: Border and Background Parameters ***

- (BOOL)drawBorder;
- (void)setDrawBorder:(BOOL)flag;

- (float)borderCornerRadius;
- (void)setBorderCornerRadius:(float)newBorderCornerRadius;

- (float)borderThickness;
- (void)setBorderThickness:(float)newBorderThickness;

- (NSColor *)borderColor;
- (void)setBorderColor:(NSColor *)newBorderColor;

#pragma mark -
#pragma mark *** Stage 4: Shadow Parameters ***

- (BOOL)addsShadow;
- (void)setAddsShadow:(BOOL)flag;

- (NSShadow *)shadow;

#pragma mark -
#pragma mark *** Filter Accessors ***

- (float)scaleFactor;
- (CIFilter *)unsharpMaskFilter;

#pragma mark -
#pragma mark *** Output Image Accessors ***

- (CIImage *)outputCIImage;
- (NSBitmapImageRep *)outputBitmapImageRep;
- (int)outputPixelsWide;
- (int)outputPixelsHigh;

@end

@interface ImageReducer (NonPrimitiveMethods)
// This is a convenience setter that gets or creates an inputBitmapImageRep from an NSImage.
- (void)setInputImage:(NSImage *)newInputImage;

// This is a convenience getter that wraps the outputBitmapImageRep in an NSImage.
- (NSImage *)outputImage;
@end
