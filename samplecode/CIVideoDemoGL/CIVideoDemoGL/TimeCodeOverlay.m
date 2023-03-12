/*

File: TimeCodeOverlay.m

Abstract:   Helper class to render QTTime into a CIImage.

Version: 1.0

© Copyright 2005-2007 Apple Inc., All rights reserved.

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

#import "TimeCodeOverlay.h"

@implementation TimeCodeOverlay

- (id)initWithAttributes:(NSDictionary*)inFontAttributes targetSize:(NSSize)inTargetSize
{
    self = [super init];
    
    if (self == nil)
	return nil;
    
    CGColorSpaceRef	colorspace = CGColorSpaceCreateDeviceRGB();
    size_t		width, height, bytesPerRow;
    CGDataProviderRef	bitmapContextDataProvider;

    targetSize = inTargetSize;
    // setup the font attributes
    if(inFontAttributes)
    {
	fontAttributes = inFontAttributes;
    } else {
	fontAttributes = [NSDictionary dictionaryWithObjectsAndKeys:[NSFont messageFontOfSize:48.0] , NSFontAttributeName, nil];
    }
    [fontAttributes retain];
    
    //create the CGBitmapContext in which we will render into
    width = floor(targetSize.width);
    height = floor(targetSize.height);
    bytesPerRow = (width * 4 + 63) & ~63;
    bitmapData = calloc(bytesPerRow * height, 1);
    bitmapContext = CGBitmapContextCreate(bitmapData, width, height, 8 ,bytesPerRow, colorspace, kCGImageAlphaPremultipliedLast);
    graphicsContext = [[NSGraphicsContext graphicsContextWithGraphicsPort:bitmapContext flipped:NO] retain];
    bitmapContextDataProvider = CGDataProviderCreateWithData(nil, bitmapData, bytesPerRow * height, nil);
    bitmapImage = CGImageCreate(width,	
				height, 
				8, 
				32, 
				bytesPerRow, 
				colorspace, 
				kCGImageAlphaPremultipliedLast, 
				bitmapContextDataProvider, 
				nil, 
				false, 
				kCGRenderingIntentDefault);
    CGDataProviderRelease(bitmapContextDataProvider);
    CGColorSpaceRelease(colorspace);
    return self;
}

- (void)dealloc
{
    [graphicsContext release];
    [fontAttributes release];
    CGContextRelease(bitmapContext);
    CGImageRelease(bitmapImage);
    free(bitmapData);
    [super dealloc];
}

- (void)renderText:(NSString*)inString
{
    NSGraphicsContext	*savedContext = [NSGraphicsContext currentContext];

    NSSize	textSize = [inString sizeWithAttributes:fontAttributes];
    // render the text into the cleared context
    CGContextClearRect(bitmapContext,CGRectMake(0.0, 0.0, targetSize.width, targetSize.height));
    
    [NSGraphicsContext setCurrentContext:graphicsContext];
    [inString drawAtPoint:NSMakePoint((targetSize.width - textSize.width) * 0.5, (targetSize.height - textSize.height) * 0.5) withAttributes:fontAttributes];	// draw the text centered into our bitmap context
    if(savedContext)
	[NSGraphicsContext setCurrentContext:savedContext];
   
}

- (CIImage*)getImageForTime:(QTTime)inTime
{
    CIImage *textImage = nil;
    
    [self renderText:QTStringFromTime(inTime)]; 
    textImage = [CIImage imageWithCGImage:bitmapImage];	// return the CIImage of the rendering
    return textImage;
}

@end
