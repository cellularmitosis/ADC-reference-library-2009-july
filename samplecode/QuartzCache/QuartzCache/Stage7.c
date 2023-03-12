/*
 
 File: Stage7.c
 Abstract: The seventh stage of the sample code.
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
// Stage 7 - This test uses the CGImageSource API to read a JPG file from disk.
// It repeatedly draws the image image in a loop. 
// Returns: the number of operations per second for this stage
//
float drawStage7(CGContextRef context, CGRect rect)
{
    int i;
    double delta;
    size_t numOperations;
    CGImageSourceRef imageSource;
    CGImageRef image;
    CFURLRef url = CFBundleCopyResourceURL(CFBundleGetMainBundle(),CFSTR("test.jpg"),NULL,NULL);
    
    // Create an image source from the URL 
    imageSource = CGImageSourceCreateWithURL(url,NULL);
    image = CGImageSourceCreateImageAtIndex(imageSource, 0, NULL);
    CFRelease(imageSource);
    
    // Perform the drawing operation once to get an rough idea of how long it will take
    delta = currentTime();
    // Just draw the image    
    CGContextDrawImage(context, CGRectMake(0, 0, ScaledToWidth, ScaledToHeight), image);
    delta = currentTime() - delta;   
    
    // Calculate the approximage number of operations needed in one second
    numOperations = SecsPerTest / delta;
    
    // Now run the test again repeatedly
    delta = currentTime();
    
    for (i = 0 ; i < numOperations; i++) {
    	// Make the image smaller each time. Because the image is smaller than the last time
        // CoreGraphics will not have cached the image. So the entire image is decoded
        // when drawn. 
        CGContextScaleCTM(context, 0.95, 0.95);
        // Just draw the image
        CGContextDrawImage(context, CGRectMake(0, 0, ScaledToWidth, ScaledToHeight), image);
    }
    
    delta = currentTime() - delta;   
    
    CFRelease(image);
    
    return (numOperations / delta);
}
