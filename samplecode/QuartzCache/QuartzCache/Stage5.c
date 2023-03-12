/*
 
 File: Stage5.c
 Abstract: The fifth stage of the sample code.
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
// Stage 5 - This measures how fast we can have an updating (i.e. mutable) 
// offscreen bitmap context to the window.
// The offscreen bitmap context is repeatedly modified using CoreGraphics 
// drawing calls. Alternatively we could have also modified the bitmap 
// data's bits directly if we had our own rendering engine.
// The test measures how fast drawing the modified bitmap context takes. 
// This technique demonstrates how to draw images (even if they are changing 
// frame to frame ). Use this technique when you need to modify the bitmap
// bits directly yourself.
// Note that the gradient is just used as a backdrop. 
// Returns: the number of operations per second for this stage
//
float drawStage5(CGContextRef context, CGRect rect)
{
    size_t numOperations;
    CGImageRef cachedImage;
    size_t bytesPerRow, size, skipBytes;
    CGColorSpaceRef csDisplay;
    double delta;
    int i;
    unsigned char * p, *data;
    CGContextRef bitmapContext;
    
    
    // Allocate the data for the offscreen bitmap context
    bytesPerRow = ((ScaledToWidth * 32) + 7) / 8;
    skipBytes = bytesPerRow - ScaledToWidth * 4;
    size = bytesPerRow * ScaledToHeight;
    p = data = malloc(size);
    
    csDisplay = getDisplayRGBColorSpace();  // Display RGB colorspace
    
    // Create the bitmap 
    bitmapContext = CGBitmapContextCreate(data /* data */,
        ScaledToWidth /* width */, 
        ScaledToHeight /* height */, 
        8  /* bitsPerComponent */, 
        ((ScaledToWidth * 32) + 7) / 8 /* bytesPerRow */, 
        csDisplay /* colorspace */,
        kCGImageAlphaNoneSkipFirst /* CGImageBitmapInfo */);

    CFRelease(csDisplay);

    // Fill the bitmap context background with white
    CGContextSetRGBFillColor(bitmapContext, 1,1,1,1);
    CGContextFillRect(bitmapContext,rect);


    // Perform the drawing operation once to get an rough idea of how long it will take
    delta = currentTime();

        // Simulate modification to the bitmap context by drawing a random 10x10 rectangle to it
        CGContextFillRect(bitmapContext,CGRectMake(random()%ScaledToWidth, 
                                               random()%ScaledToHeight, 10, 10));
        // Create an image from the bitmap context
        cachedImage = CGBitmapContextCreateImage(bitmapContext);
        // Draw the image
        CGContextDrawImage(context, CGRectMake(0, 0, ScaledToWidth, ScaledToHeight), cachedImage);
        // release the image
    CFRelease(cachedImage);

    delta = currentTime() - delta;  // calculate the time taken

    // Calculate the approximage number of operations needed in one second
    numOperations = SecsPerTest / delta;


    // Setup a fill color for the rectangle
    CGContextSetRGBFillColor(bitmapContext,1, 0, 0, 1);


    // Now run the test again repeatedly
    delta = currentTime();

    for (i = 0 ; i < numOperations; i++) {
	// Simulate modification to the bitmap context by drawing a random 10x10 rectangle to it
	CGContextFillRect(bitmapContext,CGRectMake(random()%ScaledToWidth, random()%ScaledToHeight, 10, 10));
	
	// Create a new image from the now modified bitmap context
	cachedImage = CGBitmapContextCreateImage(bitmapContext);
	
	// Draw this new image
	CGContextDrawImage(context, CGRectMake(0, 0, ScaledToWidth, ScaledToHeight), cachedImage);
	
	// Done with the image
	CFRelease(cachedImage);
    }

    delta = currentTime() - delta;      // Calculate total time taken 

    // Clean up
    CFRelease(bitmapContext);
    free(data);

    return (numOperations / delta);
}