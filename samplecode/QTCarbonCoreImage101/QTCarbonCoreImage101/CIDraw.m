/*

File: CIDraw.m

Author: QuickTime DTS

Change History (most recent first): <1> 2/8/06 initial release

© Copyright 2006 Apple Computer, Inc. All rights reserved.

IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc. ("Apple") in
consideration of your agreement to the following terms, and your use, installation,
modification or redistribution of this Apple software constitutes acceptance of these
terms.  If you do not agree with these terms, please do not use, install, modify or
redistribute this Apple software.

In consideration of your agreement to abide by the following terms, and subject to these
terms, Apple grants you a personal, non-exclusive license, under Apple's copyrights in
this original Apple software (the "Apple Software"), to use, reproduce, modify and
redistribute the Apple Software, with or without modifications, in source and/or binary
forms; provided that if you redistribute the Apple Software in its entirety and without
modifications, you must retain this notice and the following text and disclaimers in all
such redistributions of the Apple Software. Neither the name, trademarks, service marks
or logos of Apple Computer, Inc. may be used to endorse or promote products derived from
the Apple Software without specific prior written permission from Apple.  Except as
expressly stated in this notice, no other rights or licenses, express or implied, are
granted by Apple herein, including but not limited to any patent rights that may be
infringed by your derivative works or by other works in which the Apple Software may be
incorporated.

The Apple Software is provided by Apple on an "AS IS" basis.  APPLE MAKES NO WARRANTIES,
EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED WARRANTIES OF
NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE, REGARDING THE
APPLE SOFTWARE OR ITS USE AND OPERATION ALONE OR IN COMBINATION WITH YOUR PRODUCTS.

IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) ARISING IN ANY WAY OUT OF THE
USE, REPRODUCTION, MODIFICATION AND/OR DISTRIBUTION OF THE APPLE SOFTWARE, HOWEVER
CAUSED AND WHETHER UNDER THEORY OF CONTRACT, TORT (INCLUDING NEGLIGENCE), STRICT
LIABILITY OR OTHERWISE, EVEN IF APPLE HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/

#import <QuartzCore/QuartzCore.h>
#include "CIDraw.h"

// globals
static CIFilter *gPixellateFilter = nil;
static CIFilter *gPointillizeFilter = nil;
static CIContext *gCIContext = nil;
static CIImage *gTextImagePtr = nil;

static CGRect gTextLayerRect = { 0.0, 0.0, 250.0, 140.0 };

Boolean gButtonValue = 1;
SInt32 gSliderValue = 3;

const float semiTransparentGreen[] = { 0.0, 1.0, 0.0, 0.4 };

static void drawRomanText(CGContextRef context, CGRect destRect)
{
    static const char *text = "CARBON";
    size_t textlen = strlen(text);
    static const float fontSize = 60;
        
    // set the fill color space
    CGContextSetFillColorSpace(context, CGColorSpaceCreateDeviceRGB());
    
    // set the stroke color
    CGContextSetRGBStrokeColor(context, 0.0, 0.0, 0.0, 1.0);

    // set the fill color
    CGContextSetFillColor(context, semiTransparentGreen);
    
    // the Cocoa framework calls the draw method with an undefined
    // text matrix. It's best to set it to what is needed by
    // this code: the identity transform
    CGContextSetTextMatrix(context, CGAffineTransformIdentity);
       
    // choose the font with the PostScript name "Times-Roman", at
    // fontSize points, with the encoding MacRoman encoding
    CGContextSelectFont(context, "Times-Roman", fontSize, kCGEncodingMacRoman);

    // font smoothing
    CGContextSetShouldSmoothFonts(context, true);
    
    // we want a shadaw because it's cool
    CGContextSetShadow(context, CGSizeMake(5, -20), 1.0);
    
    // set the drawing mode
    CGContextSetTextDrawingMode(context, kCGTextFillStroke);
    
    // draw the text
    CGContextShowTextAtPoint(context, 0, 70, text, textlen);

}

// create the CI effect filters, the CIContext and the text image
void InitializeCIFilters(CGLContextObj inCGLContext, CGLPixelFormatObj inCGLPixelFormat)
{
    NSAutoreleasePool	*pool = [[NSAutoreleasePool alloc] init];
    
    gPixellateFilter = [[CIFilter filterWithName:@"CIPixellate"] retain];  // edgework filter  
    [gPixellateFilter setDefaults];      // set the filter to its default values
    
    gPointillizeFilter = [[CIFilter filterWithName:@"CIPointillize"] retain];  // pixellate filter  
    [gPointillizeFilter setDefaults];   // set the filter to its default values
    
    gCIContext = [[CIContext contextWithCGLContext:inCGLContext pixelFormat:inCGLPixelFormat options:nil] retain];
    
    CGLayerRef layer = [gCIContext createCGLayerWithSize:gTextLayerRect.size info: nil];
    CGContextRef context = CGLayerGetContext(layer);

    drawRomanText(context, gTextLayerRect);
    gTextImagePtr = [[CIImage alloc] initWithCGLayer:layer];
    CGLayerRelease(layer);
    
    [pool release];
}

// draw the frame processed though Core Image
void DoDraw(CVImageBufferRef inCurrentFrame, CGRect inBounds)
{
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
    
    CIImage *inputImage = [CIImage imageWithCVImageBuffer:inCurrentFrame];
    if (NULL == inputImage) goto done;
    
    // make sure to get the image extent before applying any filters -- it is the
	// rectangle that specifies the x-value of the rectangle origin, the y-value of
	// the rectangle origin, and the width and height in working space coordinates
	CGRect imageRect = [inputImage extent];
                
    if (1 == gButtonValue) {
        [gPixellateFilter setValue:inputImage forKey:@"inputImage"];
        [gPixellateFilter setValue:[NSNumber numberWithInt:gSliderValue] forKey:@"inputScale"];
        
        // render our resulting image into our context
        // this call will scale the frame to fit the content bounds of the window
        [gCIContext drawImage: [gPixellateFilter valueForKey:@"outputImage"] 
                    inRect:CGRectMake(0, 0, inBounds.size.width, inBounds.size.height)
                    fromRect:imageRect];
    } else {
        [gPointillizeFilter setValue:inputImage forKey:@"inputImage"];
        [gPointillizeFilter setValue:[NSNumber numberWithFloat:gSliderValue] forKey:@"inputRadius"];

        // render our resulting image into our context
        // this call will scale the frame to fit the content bounds of the window
        [gCIContext drawImage: [gPointillizeFilter valueForKey:@"outputImage"] 
                    inRect:CGRectMake(0, 0, inBounds.size.width, inBounds.size.height)
                    fromRect:imageRect];
    }
    
    // you can just as easily use the CISourceOverCompositing filter instead of drawing
    // this image separately...however, in this case we didn't want this layer to be scaled
    // along with the video frame so we chose to do it this way
    // using the CISourceOverCompositing filter is more efficent if it fits your needs.
    [gCIContext drawImage:gTextImagePtr 
		    atPoint: CGPointMake((int)((inBounds.size.width - gTextLayerRect.size.width) * 0.5),
                                 (int)((inBounds.size.height - gTextLayerRect.size.height) * 0.5))
		    fromRect:gTextLayerRect];

done:

    glFlush();
     
    [pool release];
}