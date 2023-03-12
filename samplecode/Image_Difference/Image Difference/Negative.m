//
//  Negative.m
//  Image Difference
//
//  Created by jcr on Fri Sep 13 2002.
//  Copyright (c) 2002 Apple Computer, Inc. All rights reserved.

#import "Negative.h"
	
@interface NSBitmapImageRep (inversion)

- (NSBitmapImageRep *) invertRGBA;  //Various methods for inverting the bitmap data.
- (NSBitmapImageRep *) invertRGB;
- (NSBitmapImageRep *) invertGreyAlpha;
- (NSBitmapImageRep *) invertGrey;

@end
	
@implementation NSImage (Negative)

- (NSImage *) negativeImage
	{
	NSImage 
		*newImage = [[[self class] alloc] initWithSize:[self size]];
		
	NSBitmapImageRep 
		*negRep = [[[self representations] objectAtIndex:0] negativeImageRep];
		
	[newImage addRepresentation:negRep];
	return [newImage autorelease];
	}

@end

@implementation NSBitmapImageRep (Negative)

- (NSBitmapImageRep *) negativeImageRep // Only works for 8bits/sample RGB, RGBA, greyscale or greyScale with Alpha bitmaps
	{
	switch ( [self samplesPerPixel])
		{
		case 4: //RGBA
			return [[[self copy] invertRGBA] autorelease];
			
		case 3: 	// RGB
			return [[[self copy] invertRGB] autorelease];
			
		case 2: // grey scale with alpha
			return [[[self copy] invertGreyAlpha] autorelease];
			
		case 1:  // grey scale
		  return [[[self copy] invertGrey] autorelease];
			
		default: 
			return nil;		
		}	
	}

- (NSBitmapImageRep *) invertRGBA
	{
	RGBAPixel
		*pixels = (RGBAPixel *)[self bitmapData];  // -bitmapData returns a void*, not an NSData object ;-)
		
	int 
		row, 
		column,
		widthInPixels = [self pixelsWide], 
		heightInPixels = [self pixelsHigh];
		
	for (row = 0; row < heightInPixels; row++)
		for (column = 0; column < widthInPixels; column++)
			{
			RGBAPixel 
				*thisPixel = (RGBAPixel *) &(pixels[((widthInPixels * row) + column)]);
			thisPixel->redByte = (255 - thisPixel->redByte);
			thisPixel->greenByte = (255 - thisPixel->greenByte);
			thisPixel->blueByte = (255 - thisPixel->blueByte);
			// We're not going to invert the alpha...
			}
	return self;		
	}

- (NSBitmapImageRep *) invertRGB
	{
	RGBPixel
	 	*pixels = (RGBPixel *)[self bitmapData];  // -bitmapData returns a void*, not an NSData object ;-)
		
	int 
		row, 
		column,
		widthInPixels = [self pixelsWide], 
		heightInPixels = [self pixelsHigh];
		
	for (row = 0; row < heightInPixels; row++)		
		for (column = 0; column < widthInPixels; column++)
			{
			RGBPixel 
				*thisPixel = &(pixels[((widthInPixels * row) + column)]);
			thisPixel->redByte = (255 - thisPixel->redByte);
			thisPixel->greenByte = (255 - thisPixel->greenByte);
			thisPixel->blueByte = (255 - thisPixel->blueByte);
			}
	return self;		
	}	

- (NSBitmapImageRep *) invertGreyAlpha
	{
	GreyAlphaPixel
	 	*pixels = (GreyAlphaPixel *)[self bitmapData];  // -bitmapData returns a void*, not an NSData object ;-)
		
	int 
		row, 
		column,
		widthInPixels = [self pixelsWide], 
		heightInPixels = [self pixelsHigh];
		
	for (row = 0; row < heightInPixels; row++)		
		for (column = 0; column < widthInPixels; column++)
			{
			GreyAlphaPixel 
				*thisPixel = (GreyAlphaPixel *) &(pixels[((widthInPixels * row) + column)]);
			thisPixel->greyByte = (255 - thisPixel->greyByte);
			}
	return self;		
	}	

- (NSBitmapImageRep *) invertGrey
	{
	GreyPixel
	 	*pixels = (GreyPixel *)[self bitmapData];  // -bitmapData returns a void*, not an NSData object ;-)
		
	int 
		row, 
		column,
		widthInPixels = [self pixelsWide], 
		heightInPixels = [self pixelsHigh];
		
	for (row = 0; row < heightInPixels; row++)		
		for (column = 0; column < widthInPixels; column++)
			{
			GreyPixel 
				*thisPixel = (GreyPixel *) &(pixels[((widthInPixels * row) + column)]);
			*thisPixel = (255 - (*thisPixel));
			}
	return self;		
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
