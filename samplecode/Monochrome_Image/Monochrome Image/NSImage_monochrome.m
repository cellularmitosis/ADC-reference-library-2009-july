//
//  NSImage_monochrome.m
//  Black and White
//
//  Created by jcr on Wed Aug 21 2002.
//  Copyright (c) 2002 Apple Computer, Inc. All rights reserved.
//

#import "NSImage_monochrome.h"

typedef struct _monochromePixel
	{ 
	unsigned char grayValue; 
	unsigned char alpha; 
	} monochromePixel;

@implementation NSImage (monochrome)

- (NSImage *) monochromeImage
	{
	NSSize 
		mySize = [self size];
		
	NSImage 
		*monochromeImage = [[[self class] alloc] initWithSize:mySize];
		
	int
		row, column, widthInPixels = mySize.width, heightInPixels = mySize.height;
		
		// Need a place to put the monochrome pixels.
	NSBitmapImageRep *blackAndWhiteRep = 
    [[NSBitmapImageRep alloc] 
      initWithBitmapDataPlanes: nil  // Nil pointer tells the kit to allocate the pixel buffer for us.
      pixelsWide: widthInPixels 
      pixelsHigh: heightInPixels
      bitsPerSample: 8
      samplesPerPixel: 2  
      hasAlpha: YES
      isPlanar: NO 
      colorSpaceName: NSCalibratedWhiteColorSpace // 0 = black, 1 = white in this color space.
      bytesPerRow: 0     // Passing zero means "you figure it out."
      bitsPerPixel: 16];  // This must agree with bitsPerSample and samplesPerPixel.
  
  monochromePixel
    *pixels = (monochromePixel *)[blackAndWhiteRep bitmapData];  // -bitmapData returns a void*, not an NSData object ;-)
	
	[self lockFocus]; // necessary for NSReadPixel() to work.
	for (row = 0; row < heightInPixels; row++)
		for (column = 0; column < widthInPixels; column++)
			{
			monochromePixel 
				*thisPixel = &(pixels[((widthInPixels * row) + column)]);
									
			NSColor 
				*pixelColor = NSReadPixel(NSMakePoint(column, heightInPixels - (row +1)));
							
			//  thisPixel->grayValue = 1.0 - rint(255 *      // use this line for negative..
			thisPixel->grayValue = rint(255 *   // use this line for positive...
					(0.299 * [pixelColor redComponent]
								 + 0.587 * [pixelColor greenComponent]
								 + 0.114 * [pixelColor blueComponent]));
									
		  thisPixel->alpha = ([pixelColor alphaComponent]  * 255); // handle the transparency, too
			}
	[self unlockFocus];
	
	[monochromeImage addRepresentation:blackAndWhiteRep];
	[blackAndWhiteRep release];

	return [monochromeImage autorelease];
	}

@end

/*
 IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc. ("Apple") in
 consideration of your agreement to the following terms, and your use, installation, 
 modification or redistribution of this Apple software constitutes acceptance of these 
 terms.  If you do not agree with these terms, please do not use, install, modify or 
 redistribute this Apple software.
 
 In consideration of your agreement to abide by the following terms, and subject to these 
 terms, Apple grants you a personal, non-exclusive license, under Apple’s copyrights in 
 this original Apple software (the "Apple Software"), to use, reproduce, modify and 
 redistribute the Apple Software, with or without modifications, in source and/or binary 
 forms; provided that if you redistribute the Apple Software in its entirety and without 
 modifications, you must retain this notice and the following text and disclaimers in all 
 such redistributions of the Apple Software.  Neither the name, trademarks, service marks 
 or logos of Apple Computer, Inc. may be used to endorse or promote products derived from 
 the Apple Software without specific prior written permission from Apple. Except as expressly
 stated in this notice, no other rights or licenses, express or implied, are granted by Apple
 herein, including but not limited to any patent rights that may be infringed by your 
 derivative works or by other works in which the Apple Software may be incorporated.
 
 The Apple Software is provided by Apple on an "AS IS" basis.  APPLE MAKES NO WARRANTIES, 
 EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED WARRANTIES OF NON-INFRINGEMENT, 
 MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS 
 USE AND OPERATION ALONE OR IN COMBINATION WITH YOUR PRODUCTS.
 
 IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL OR CONSEQUENTIAL 
 DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS 
 OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) ARISING IN ANY WAY OUT OF THE USE, 
 REPRODUCTION, MODIFICATION AND/OR DISTRIBUTION OF THE APPLE SOFTWARE, HOWEVER CAUSED AND 
 WHETHER UNDER THEORY OF CONTRACT, TORT (INCLUDING NEGLIGENCE), STRICT LIABILITY OR 
 OTHERWISE, EVEN IF APPLE HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

