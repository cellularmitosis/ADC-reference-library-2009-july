/*
 
 File: Stage3.c
 Abstract: The third stage of the sample code.
 Version: <1.0>
 
 © Copyright 2005 Apple Computer, Inc. All rights reserved.
 
 IMPORTANT:  This Apple software is supplied to 
 you by Apple Computer, Inc. ("Apple") in 
 consideration of your agreement to the following 
 terms, and your use, installation, modification 
 or redistribution of this Apple software 
 constitutes acceptance of these terms.  If you do 
 not agree with these terms, please do not use, 
 install, modify or redistribute this Apple 
 software.
 
 In consideration of your agreement to abide by 
 the following terms, and subject to these terms, 
 Apple grants you a personal, non-exclusive 
 license, under Apple's copyrights in this 
 original Apple software (the "Apple Software"), 
 to use, reproduce, modify and redistribute the 
 Apple Software, with or without modifications, in 
 source and/or binary forms; provided that if you 
 redistribute the Apple Software in its entirety 
 and without modifications, you must retain this 
 notice and the following text and disclaimers in 
 all such redistributions of the Apple Software. 
 Neither the name, trademarks, service marks or 
 logos of Apple Computer, Inc. may be used to 
 endorse or promote products derived from the 
 Apple Software without specific prior written 
 permission from Apple.  Except as expressly 
 stated in this notice, no other rights or 
 licenses, express or implied, are granted by 
 Apple herein, including but not limited to any 
 patent rights that may be infringed by your 
 derivative works or by other works in which the 
 Apple Software may be incorporated.
 
 The Apple Software is provided by Apple on an "AS 
 IS" basis.  APPLE MAKES NO WARRANTIES, EXPRESS OR 
 IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED 
 WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY 
 AND FITNESS FOR A PARTICULAR PURPOSE, REGARDING 
 THE APPLE SOFTWARE OR ITS USE AND OPERATION ALONE 
 OR IN COMBINATION WITH YOUR PRODUCTS.
 
 IN NO EVENT SHALL APPLE BE LIABLE FOR ANY 
 SPECIAL, INDIRECT, INCIDENTAL OR CONSEQUENTIAL 
 DAMAGES (INCLUDING, BUT NOT LIMITED TO, 
          PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS 
          OF USE, DATA, OR PROFITS; OR BUSINESS 
          INTERRUPTION) ARISING IN ANY WAY OUT OF THE USE, 
 REPRODUCTION, MODIFICATION AND/OR DISTRIBUTION OF 
 THE APPLE SOFTWARE, HOWEVER CAUSED AND WHETHER 
 UNDER THEORY OF CONTRACT, TORT (INCLUDING 
                                 NEGLIGENCE), STRICT LIABILITY OR OTHERWISE, EVEN 
 IF APPLE HAS BEEN ADVISED OF THE POSSIBILITY OF 
 SUCH DAMAGE.
 
 */

#include "Stages.h"

//
// Stage 3 - Draw a 2K by 2K source image to an offscreen 512 x 512 bitmap context,
// effectively caching the original image. The test measures how fast drawing 
// the cached image takes. The cache alleviates the time taken to color match the
// image and also time required to downsample the image. 
// This cache technique is useful when the image has to be repeatedly draw at the
// same scale. 
// The cached image is a 32 bit ARGB image.
// The source image is created programattically as a gradient between colors.
// Returns: the number of operations per second for this stage
//
float drawStage3(CGContextRef context, CGRect rect)
{
    size_t numOperations;
    CGImageRef image, cachedImage;
    CGColorSpaceRef csDisplay;
    double delta;    
    int i;
    unsigned char *data, *cachedData;
    CGContextRef bitmapContext;

    data = createGradientARGBBuffer(W, H);
    if (data == NULL) return 0;
    
    // Create the source image from the data provider
    image = createImage(W, H, data);
    
    
    // Allocate a new 512 x 512 offscreen image where we will cache the data
    cachedData = malloc( (((ScaledToWidth * 32) + 7) / 8) * ScaledToHeight);
    
    // The colorspace for the offscreen cached image is the colorspace of the main display
    csDisplay = getDisplayRGBColorSpace();
    
    // Create a bitmap context that represents the cached image
    bitmapContext = CGBitmapContextCreate(cachedData /* data */,
        ScaledToWidth /* width */, 
        ScaledToHeight /* height */, 
        8  /* bitsPerComponent */, 
        ((ScaledToWidth * 32) + 7) / 8 /* bytesPerRow */, 
        csDisplay /* colorspace */,
        kCGImageAlphaNoneSkipFirst /* CGImageBitmapInfo */);

    CFRelease(csDisplay);   // no longer need the colorspace

    // Now draw the source image to the bitmap context. This will downsample and color match 
    // the source image and store the result int the cached image
    CGContextDrawImage(bitmapContext, CGRectMake(0,0,ScaledToWidth,ScaledToHeight), image);
    CFRelease(image);	 // no longer need the source image.
    free(data);                     // free the bitmap data associated with the source image

    // Retrieve a CGImageRef that points to the bitmap context (the cached image)
    cachedImage = CGBitmapContextCreateImage(bitmapContext);
    CFRelease(bitmapContext);	// No longer need the bitmap context


    // Perform the drawing operation once to get an rough idea of how long it will take
    delta = currentTime();
        // Draw the cached image once for this test
        CGContextDrawImage(context, CGRectMake(0, 0, ScaledToWidth, ScaledToHeight), cachedImage);
    delta = currentTime() - delta;   

    // Calculate the approximage number of operations needed in one second
    numOperations = SecsPerTest / delta;

    // Now run the test again repeatedly
    delta = currentTime();

    for (i = 0 ; i < numOperations; i++) {
	// Draw the 512 x 512 cached image once to the destination
	CGContextDrawImage(context, CGRectMake(0, 0, ScaledToWidth, ScaledToHeight), cachedImage);
    }

    delta = currentTime() - delta;   // Calculate total time taken 

    // Clean up
    CFRelease(cachedImage);
    free(cachedData);

    return (numOperations / delta);
}