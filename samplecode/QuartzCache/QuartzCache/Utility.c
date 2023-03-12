/*
 
 File: Utility.c
 Abstract: Common utiltity functions
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

#include "Utility.h"
#include <mach/mach_time.h>

unsigned char r = 0xff;

//
// Allocate a bitmap buffer and fill it with a gradient
//
unsigned char * createGradientARGBBuffer(size_t W, size_t H)
{
    int i, j;
    unsigned char * p, *data;
    size_t bytesPerRow, size, skipBytes;
    
    // Allocate the 3K by 3K image
    bytesPerRow = ((W * 32) + 7) / 8;
    skipBytes = bytesPerRow - W * 4;
    size = bytesPerRow * H;
    p = data = malloc(size);
    
    if (data == NULL) return NULL;
    r = 0xff - r;
    // Fill the image with a color gradient
    for (i=0; i < H; i++) {
        for (j=0; j < W; j++) {
            *p++ = 0xff;				// A
            *p++ = r;                                           // R
            *p++ = 0x00 + ((float)i / H) * 0xff;	// G
            *p++ = 0xff - ((float)j / W) * 0xff;	// B
        }
        p += skipBytes;
    }
    return data;
}

//
//  Create a 32 bit XRGB CGImageRef based on the width, height and bitmap data
//
CGImageRef createImage(size_t W, size_t H, unsigned char *data)
{
    CGImageRef image;
    size_t bytesPerRow, size, skipBytes;
    CGColorSpaceRef csGeneric;
    CGDataProviderRef provider;
    
    // Allocate the 3K by 3K image
    bytesPerRow = ((W * 32) + 7) / 8;
    skipBytes = bytesPerRow - W * 4;
    size = bytesPerRow * H;

    // Use GenericRGB as the colorspace for the source image
    csGeneric = CGColorSpaceCreateWithName(kCGColorSpaceGenericRGB);

    // Create a data provider for the bitmap data
    provider = CGDataProviderCreateWithData(NULL, data, size, NULL);

    // Create the source image as a 32 bit XRGB image
    image = CGImageCreate(W, H, 8, 32, bytesPerRow, csGeneric, kCGImageAlphaNoneSkipFirst,
                        provider, NULL, true, kCGRenderingIntentDefault);
    CFRelease(csGeneric);   // no longer need the colorspace
    CFRelease(provider);    // no longer need the data provider
    return(image);
}

//
// Returns the current time in seconds
// 
double currentTime(void)
{
    static double scale = 0;

    if (scale == 0) {
        mach_timebase_info_data_t info;
        mach_timebase_info(&info);
        scale = info.numer / info.denom * 1e-9;
    }

    return mach_absolute_time() * scale;
}



//
// Create a colorspace for the main display
// 
CGColorSpaceRef getDisplayRGBColorSpace(void)
{
    CMProfileRef sysprof = NULL;
    CGColorSpaceRef dispColorSpace = NULL;

    // Get the system profile (which represents the profile for the main display
    if (CMGetSystemProfile(&sysprof) == noErr)
    {
	// Build a CGColorSpaceRef from the system profile
	dispColorSpace = CGColorSpaceCreateWithPlatformColorSpace(sysprof);
	CMCloseProfile(sysprof);
    }
    return dispColorSpace;
}
















