/*
 
 File: Stage6.c
 Abstract: The sixth stage of the sample code.
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
// Stage 6 - This test demonstrates how fast we can draw an updating
// (i.e. mutable) CG Layer. The Layer is modified using CoreGraphics drawing
// calls. Use this technique when you want to cache you drawing using 
// CoreGraphics.
// Returns: the number of operations per second for this stage
//
float drawStage6(CGContextRef context, CGRect rect)
{
    size_t numOperations;
    double delta;
    int i;
    CGLayerRef lyr;
    CGContextRef lyrContext;
    
    // Fill the bitmap context background with white
    CGContextSetRGBFillColor(context,1, 1, 1, 1);
    CGContextFillRect(context,rect);
    
    // Create a layer from the destination context. The layer is the best representation
    // for the layer. We have to specify the size of the layer we want to create 
    // in default userspace units. 
    lyr = CGLayerCreateWithContext(context,CGSizeMake(ScaledToWidth,ScaledToHeight),NULL);
    // Now we want to draw in to layer, so get the CGContext for the layer to draw to
    lyrContext = CGLayerGetContext(lyr);
    
    
    // Perform the drawing operation once to get an rough idea of how long it will take
    delta = currentTime();
    // Draw the contents of the layer to the destination
    CGContextDrawLayerAtPoint(context,CGPointMake(0,0),lyr);
    delta = currentTime() - delta;   
    
    // Calculate the approximage number of operations needed in one second
    numOperations = SecsPerTest / delta;

    
    // Now run the test again repeatedly
    delta = currentTime();
    
    // Setup a fill color for the rectangle
    CGContextSetRGBFillColor(lyrContext,1, 0, 0, 1);
    
    for (i = 0 ; i < numOperations; i++) {
        // Modify the layer ref  by drawing a random 10x10 rectangle to it
        CGContextFillRect(lyrContext,CGRectMake(random()%ScaledToWidth, 
                                                random()%ScaledToHeight, 10, 10));
        
        // Draw the contents of the layer to the destination
        CGContextDrawLayerAtPoint(context,CGPointMake(0,0),lyr);
    }
    
    delta = currentTime() - delta;   
    
    CFRelease(lyr);	    // done with the layer ref
    
    return (numOperations / delta);
}