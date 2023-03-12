/*

File: ImageUtils.c

Abstract: This file contains the core functionality of the sample.
	IICreateImage loads and creates the ImageInfo struct that is core to the sample
	IISaveImage saves the transformed image to a JPEG.
	IIApplyTransformation shows how to rotate about center, scale, and translate the image
		using the IIRotateContext, IIScaleContext and IITranslateContext helpers
	IIDrawImage draws the image into the given context
	IIDrawImageTransformed calls IIApplyTransformation and IIDrawImage to do both at once
		while preserving the original context's CTM
	IIRelease releases the ImageInfo struct allocated by IICreateImage

Version: 1.0

Disclaimer: IMPORTANT:  This Apple software is supplied to you by Apple
Inc. ("Apple") in consideration of your agreement to the
following terms, and your use, installation, modification or
redistribution of this Apple software constitutes acceptance of these
terms.  If you do not agree with these terms, please do not use,
install, modify or redistribute this Apple software.

In consideration of your agreement to abide by the following terms, and
subject to these terms, Apple grants you a personal, non-exclusive
license, under Apple's copyrights in this original Apple software (the
"Apple Software"), to use, reproduce, modify and redistribute the Apple
Software, with or without modifications, in source and/or binary forms;
provided that if you redistribute the Apple Software in its entirety and
without modifications, you must retain this notice and the following
text and disclaimers in all such redistributions of the Apple Software. 
Neither the name, trademarks, service marks or logos of Apple Computer,
Inc. may be used to endorse or promote products derived from the Apple
Software without specific prior written permission from Apple.  Except
as expressly stated in this notice, no other rights or licenses, express
or implied, are granted by Apple herein, including but not limited to
any patent rights that may be infringed by your derivative works or by
other works in which the Apple Software may be incorporated.

The Apple Software is provided by Apple on an "AS IS" basis.  APPLE
MAKES NO WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION
THE IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS
FOR A PARTICULAR PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND
OPERATION ALONE OR IN COMBINATION WITH YOUR PRODUCTS.

IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL
OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION,
MODIFICATION AND/OR DISTRIBUTION OF THE APPLE SOFTWARE, HOWEVER CAUSED
AND WHETHER UNDER THEORY OF CONTRACT, TORT (INCLUDING NEGLIGENCE),
STRICT LIABILITY OR OTHERWISE, EVEN IF APPLE HAS BEEN ADVISED OF THE
POSSIBILITY OF SUCH DAMAGE.

Copyright © 2007 Apple Inc., All Rights Reserved

*/

#include "ImageUtils.h"

void IIGetOrientationTransform(ImageInfo * image);
int IIGetImageOrientation(ImageInfo * image);

void IIRotateContext(ImageInfo * image, CGContextRef context, CGRect bounds);
void IIScaleContext(ImageInfo * image, CGContextRef context, CGRect bounds);
void IITranslateContext(ImageInfo * image, CGContextRef context);

// Create a new image from a file at the given url
// Returns NULL if unsuccessful.
ImageInfo * IICreateImage(CFURLRef url)
{
	ImageInfo * ii = NULL;
	// Try to create an image source to the image passed to us
	CGImageSourceRef imageSrc = CGImageSourceCreateWithURL(url, NULL);
	if(imageSrc != NULL)
	{
		// And if we can, try to obtain the first image available
		CGImageRef image = CGImageSourceCreateImageAtIndex(imageSrc, 0, NULL);
		if(image != NULL)
		{
			// and if we could, create the ImageInfo struct with default values
			ii = (ImageInfo*)malloc(sizeof(ImageInfo));
			ii->fRotation = 0.0;
			ii->fScaleX = 1.0;
			ii->fScaleY = 1.0;
			ii->fTranslateX = 0.0;
			ii->fTranslateY = 0.0;
			// the ImageInfo struct owns this CGImageRef now, so no need for a retain.
			ii->fImageRef = image;
			// the ImageInfo struct owns this CFDictionaryRef, so no need for a retain.
			ii->fProperties = CGImageSourceCopyPropertiesAtIndex(imageSrc, 0, NULL);
			// Setup the orientation transformation matrix so that the image will display with the proper orientation
			IIGetOrientationTransform(ii);
		}
		// cleanup the image source
		CFRelease(imageSrc);
	}
	return ii;
}

// Transforms the context based on the orientation of the image.
// This ensures the image always appears correctly when drawn.
void IIGetOrientationTransform(ImageInfo * image)
{
	float w = CGImageGetWidth(image->fImageRef);
	float h = CGImageGetHeight(image->fImageRef);
	if(image->fProperties != NULL)
	{
		// The Orientations listed here are mirroed from CGImageProperties.h,
		// listed under the kCGImagePropertyOrientation key.
		switch(IIGetImageOrientation(image))
		{
			case 1:
				// 1 = 0th row is at the top, and 0th column is on the left.
				// Orientation Normal
				image->fOrientation = CGAffineTransformMake(1.0, 0.0, 0.0, 1.0, 0.0, 0.0);
				break;
				
			case 2:
				// 2 = 0th row is at the top, and 0th column is on the right.
				// Flip Horizontal
				image->fOrientation = CGAffineTransformMake(-1.0, 0.0, 0.0, 1.0, w, 0.0);
				break;
				
			case 3:
				// 3 = 0th row is at the bottom, and 0th column is on the right.
				// Rotate 180 degrees
				image->fOrientation = CGAffineTransformMake(-1.0, 0.0, 0.0, -1.0, w, h);
				break;
				
			case 4:
				// 4 = 0th row is at the bottom, and 0th column is on the left.
				// Flip Vertical
				image->fOrientation = CGAffineTransformMake(1.0, 0.0, 0, -1.0, 0.0, h);
				break;
				
			case 5:
				// 5 = 0th row is on the left, and 0th column is the top.
				// Rotate -90 degrees and Flip Vertical
				image->fOrientation = CGAffineTransformMake(0.0, -1.0, -1.0, 0.0, h, w);
				break;
				
			case 6:
				// 6 = 0th row is on the right, and 0th column is the top.
				// Rotate 90 degrees
				image->fOrientation = CGAffineTransformMake(0.0, -1.0, 1.0, 0.0, 0.0, w);
				break;
				
			case 7:
				// 7 = 0th row is on the right, and 0th column is the bottom.
				// Rotate 90 degrees and Flip Vertical
				image->fOrientation = CGAffineTransformMake(0.0, 1.0, 1.0, 0.0, 0.0, 0.0);
				break;
				
			case 8:
				// 8 = 0th row is on the left, and 0th column is the bottom.
				// Rotate -90 degrees
				image->fOrientation = CGAffineTransformMake(0.0, 1.0,-1.0, 0.0, h, 0.0);
				break;
		}
	}
}

// Gets the orientation of the image from the properties dictionary if available
// If the kCGImagePropertyOrientation is not available or invalid,
// then 1, the default orientation, is returned.
int IIGetImageOrientation(ImageInfo * image)
{
	int result = 1;
	CFNumberRef orientation = CFDictionaryGetValue(image->fProperties, kCGImagePropertyOrientation);
	if(orientation != NULL)
	{
		int orient;
		if(CFNumberGetValue(orientation, kCFNumberIntType, &orient))
		{
			result = orient;
		}
	}
	return result;
}

// Save the given image to a file at the given url.
// Returns true if successful, false otherwise.
bool IISaveImage(ImageInfo * image, CFURLRef url, size_t width, size_t height)
{
	bool result = false;

	// If there is no image, no destination, or the width/height is 0, then fail early.
	require((image != NULL) && (url != NULL) && (width != 0) && (height != 0), bail);
	
	// Try to create a jpeg image destination at the url given to us
	CGImageDestinationRef imageDest = CGImageDestinationCreateWithURL(url, kUTTypeJPEG, 1, NULL);
	if(imageDest != NULL)
	{
		// And if we can, then we can start building our final image.
		// We begin by creating a CGBitmapContext to host our desintation image.
		
		// Allocate enough space to hold our pixels
		UInt32 * imageData = malloc(sizeof(UInt32) * width * height);
		
		// Create the bitmap context
		CGContextRef bitmapContext = CGBitmapContextCreate(
			imageData, // image data we just allocated...
			width, // width
			height, // height
			8, // 8 bits per component
			sizeof(UInt32) * width, // bytes per pixel times number of pixels wide
			CGImageGetColorSpace(image->fImageRef), // use the same colorspace as the original image
			kCGImageAlphaPremultipliedFirst); // use premultiplied alpha
			
		// Check that all that went well
		if(bitmapContext != NULL)
		{
			// Now, we draw the image to the bitmap context
			IIDrawImageTransformed(image, bitmapContext, CGRectMake(0.0, 0.0, width, height));
			
			// We have now gotten our image data to the bitmap context, and correspondingly
			// into imageData. If we wanted to, we could look at any of the pixels of the image
			// and manipulate them in any way that we desire, but for this case, we're just
			// going to ask ImageIO to write this out to disk.
			
			// Obtain a CGImageRef from the bitmap context for ImageIO
			CGImageRef imageIOImage = CGBitmapContextCreateImage(bitmapContext);
			
			// Check if we have additional properties from the original image
			if(image->fProperties != NULL)
			{
				// If we do, then we want to inspect the orientation property.
				// If it exists and is not the default orientation, then we
				// want to replace that orientation in the destination file
				int orientation = IIGetImageOrientation(image);
				if(orientation != 1)
				{
					// If the orientation in the original image was not the default,
					// then we need to replace that key in a duplicate of that dictionary
					// and then pass that dictionary to ImageIO when adding the image.
					CFMutableDictionaryRef prop = CFDictionaryCreateMutableCopy(NULL, 0, image->fProperties);
					orientation = 1;
					CFNumberRef cfOrientation = CFNumberCreate(NULL, kCFNumberIntType, &orientation);
					CFDictionarySetValue(prop, kCGImagePropertyOrientation, cfOrientation);
					
					// And add the image with the new properties
					CGImageDestinationAddImage(imageDest, imageIOImage, prop);
					
					// Clean up after ourselves
					CFRelease(prop);
					CFRelease(cfOrientation);
				}
				else
				{
					// Otherwise, the image was already in the default orientation and we can just save
					// it with the original properties.
					CGImageDestinationAddImage(imageDest, imageIOImage, image->fProperties);
				}
			}
			else
			{
				// If we don't, then just add the image without properties
				CGImageDestinationAddImage(imageDest, imageIOImage, NULL);
			}
			
			// Release the image and the context, since we are done with both.
			CGImageRelease(imageIOImage);
			CGContextRelease(bitmapContext);
		}
		
		// Deallocate the image data
		free(imageData);
		
		// Finalize the image destination
		result = CGImageDestinationFinalize(imageDest);
		CFRelease(imageDest);
	}
	
	bail:
	return result;
}

// Applies the transformations specified in the ImageInfo struct without drawing the actual image
void IIApplyTransformation(ImageInfo * image, CGContextRef context, CGRect bounds)
{
	if(image != NULL)
	{
		// Whenever you do multiple CTM changes, you have to be very careful with order.
		// Changing the order of your CTM changes changes the outcome of the drawing operation.
		// For example, if you scale a context by 2.0 along the x-axis, and then translate
		// the context by 10.0 along the x-axis, then you will see your drawing will be
		// in a different position than if you had done the operations in the opposite order.

		// Our intent with this operation is that we want to change the location from which we start drawing
		// (translation), then rotate our axies so that our image appears at an angle (rotation), and finally
		// scale our axies so that our image has a different size (scale).
		// Changing the order of operations will markedly change the results.
		IITranslateContext(image, context);
		IIRotateContext(image, context, bounds);
		IIScaleContext(image, context, bounds);
	}
}

// Draw the image to the given context centered inside the given bounds
void IIDrawImage(ImageInfo * image, CGContextRef context, CGRect bounds)
{
	CGRect imageRect;
	
	if((image != NULL) && (context != NULL))
	{
		// Setup the image size so that the image fills it's natural boudaries in the base coordinate system.
		imageRect.size.width = CGImageGetWidth(image->fImageRef);
		imageRect.size.height = CGImageGetHeight(image->fImageRef);

		// Determine the correct origin of the image such that it is centered in the coordinate system.
		// The exact calculations depends on the image orientation, but the basic idea
		// is that the image is located such that it is positioned so that half the difference
		// between the image's size and the bounds to be drawn is used as it's x/y location.
		if((image->fProperties == NULL) || (IIGetImageOrientation(image) < 5))
		{
			// For orientations 1-4, the images are unrotated, so the width and height of the base image
			// can be used as the width and height of the coordinate translation calculation.
			imageRect.origin.x = floorf((bounds.size.width - imageRect.size.width) / 2.0f);
			imageRect.origin.y = floorf((bounds.size.height - imageRect.size.height) / 2.0f);
		}
		else
		{
			// For orientations 5-8, the images are rotated 90 or -90 degrees, so we need to use
			// the image width in place of the height and vice versa.
			imageRect.origin.x = floorf((bounds.size.width - imageRect.size.height) / 2.0f);
			imageRect.origin.y = floorf((bounds.size.height - imageRect.size.width) / 2.0f);
		}
		
		// Obtain the orientation matrix for this image
		CGAffineTransform ctm = image->fOrientation;

		// Finally, orient the context so that the image draws naturally.
		CGContextConcatCTM(context, ctm);
		
		// And draw the image.
		CGContextDrawImage(context, imageRect, image->fImageRef);
	}
}

// Rotates the context around the center point of the given bounds
void IIRotateContext(ImageInfo * image, CGContextRef context, CGRect bounds)
{
	// First we translate the context such that the 0,0 location is at the center of the bounds
	CGContextTranslateCTM(context, bounds.size.width/2.0f, bounds.size.height/2.0f);
	
	// Then we rotate the context, converting our angle from degrees to radians
	CGContextRotateCTM(context, image->fRotation * M_PI / 180.0f);
	
	// Finally we have to restore the center position
	CGContextTranslateCTM(context, -bounds.size.width/2.0f, -bounds.size.height/2.0f);	
}

// Scale the context around the center point of the given bounds
void IIScaleContext(ImageInfo * image, CGContextRef context, CGRect bounds)
{
	// First we translate the context such that the 0,0 location is at the center of the bounds
	CGContextTranslateCTM(context, bounds.size.width/2.0f, bounds.size.height/2.0f);
	
	// Next we scale the context to the size that we want
	CGContextScaleCTM(context, image->fScaleX, image->fScaleY);
	
	// Finally we have to restore the center position
	CGContextTranslateCTM(context, -bounds.size.width/2.0f, -bounds.size.height/2.0f);	
}

// Translate the context
void IITranslateContext(ImageInfo * image, CGContextRef context)
{
	// Translation is easy, just translate.
	CGContextTranslateCTM(context, image->fTranslateX, image->fTranslateY);
}

// Draw the image to the given context centered inside the given bounds with
// the transformation info. The CTM of the context is unchanged after this call
void IIDrawImageTransformed(ImageInfo * image, CGContextRef context, CGRect bounds)
{
	// We save the current graphics state so as to not disrupt it for the caller.
	CGContextSaveGState(context);
	
	// Apply the transformation
	IIApplyTransformation(image, context, bounds);
	
	// Draw the image centered in the context
	IIDrawImage(image, context, bounds);
	
	// Restore our original graphics state.
	CGContextRestoreGState(context);
}

// Release the ImageInfo struct and other associated data
// you should not refer to the reference after this call
// This function is NULL safe.
void IIRelease(ImageInfo * image)
{
	if(image != NULL)
	{
		CGImageRelease(image->fImageRef);
		if(image->fProperties != NULL)
		{
			CFRelease(image->fProperties);
		}
		free(image);
	}
}