/*
 
 File: Stage1.c
 Abstract: The first stage in the sample code.
                Draw an image repeatedly by creating a CGImageRef,
                drawing it to the context, and then releasing it.
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
// Stage 1 - Draw a 2K by 2K source image to a 512 x 512 destination repeatedly. 
// At each step the image is recreated, drawn and released. 
// The image is also color matched because the source image is tagged as GenericRGB.
// The image is created programattically as a gradient between colors.
// Returns: the number of operations per second for this stage
//
float drawStage1(CGContextRef context, CGRect rect)
{
    int i;
    double delta;
    float numOperations;
    CGImageRef image;
    unsigned char * data = NULL;
    
    data = createGradientARGBBuffer(W, H);
    if (data == NULL) return 0;
    
    // Perform the drawing operation once to get an rough idea of how long it will take
    delta = currentTime();
        // Create the source image from the data provider
        image = createImage(W, H, data);
        // Draw the image to a smaller 512 x 512 destination rectangle
        CGContextDrawImage(context, CGRectMake(0,0,ScaledToWidth,ScaledToHeight), image);
        // Release the image
        CFRelease(image);
    delta = currentTime() - delta;	// Find the total time for a single operation
    
    // Calculate the approximage number of operations needed in one second
    numOperations = SecsPerTest / delta;
    
    // Now run the test again repeatedly
    delta = currentTime();
    
    for (i = 0 ; i < numOperations; i++) {
        // Create a 2k x 2k source image from the data provider
        image = createImage(W, H, data);
        // Draw the image to a smaller 512 x 512 destination rectangle
        CGContextDrawImage(context, CGRectMake(0,0,ScaledToWidth,ScaledToHeight), image);
        // Release the image
        CFRelease(image);        
    }
    
    delta = currentTime() - delta;	// Calculate total time taken
    
    // Clean up
    free(data);
    return ((float)numOperations / delta);
}