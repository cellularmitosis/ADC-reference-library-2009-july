/*
	File:		EffectFilter16.c
	
	Description: 16-bit Filter Implementation

	Author:		QuickTime Engineering

	Copyright: 	© Copyright 1997-2002 Apple Computer, Inc. All rights reserved.
	
	Disclaimer:	IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc.
				("Apple") in consideration of your agreement to the following terms, and your
				use, installation, modification or redistribution of this Apple software
				constitutes acceptance of these terms.  If you do not agree with these terms,
				please do not use, install, modify or redistribute this Apple software.

				In consideration of your agreement to abide by the following terms, and subject
				to these terms, Apple grants you a personal, non-exclusive license, under Apple’s
				copyrights in this original Apple software (the "Apple Software"), to use,
				reproduce, modify and redistribute the Apple Software, with or without
				modifications, in source and/or binary forms; provided that if you redistribute
				the Apple Software in its entirety and without modifications, you must retain
				this notice and the following text and disclaimers in all such redistributions of
				the Apple Software.  Neither the name, trademarks, service marks or logos of
				Apple Computer, Inc. may be used to endorse or promote products derived from the
				Apple Software without specific prior written permission from Apple.  Except as
				expressly stated in this notice, no other rights or licenses, express or implied,
				are granted by Apple herein, including but not limited to any patent rights that
				may be infringed by your derivative works or by other works in which the Apple
				Software may be incorporated.

				The Apple Software is provided by Apple on an "AS IS" basis.  APPLE MAKES NO
				WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED
				WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS FOR A PARTICULAR
				PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND OPERATION ALONE OR IN
				COMBINATION WITH YOUR PRODUCTS.

				IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL OR
				CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
				GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
				ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION, MODIFICATION AND/OR DISTRIBUTION
				OF THE APPLE SOFTWARE, HOWEVER CAUSED AND WHETHER UNDER THEORY OF CONTRACT, TORT
				(INCLUDING NEGLIGENCE), STRICT LIABILITY OR OTHERWISE, EVEN IF APPLE HAS BEEN
				ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
				
	Change History (most recent first):
	   <1>	 	05/11/02	era		updated for X
*/

#include "BltMacros.h"

static void EffectFilter16(BlitGlobals *glob);
static void EffectFilter16(BlitGlobals *glob)
{
	long	height = glob->height;					// Local copy of the height of the sources and destination
	UInt16	*srcA = glob->sources[0].srcBaseAddr;	// Local pointer to the first source image
	UInt16	*srcB = glob->sources[1].srcBaseAddr;	// Local pointer to the second source image
	UInt16	*dst = glob->dstBaseAddr;				// Local pointer to the destination
	long	srcABump;
	long	srcBBump;
	long	dstBump;
	
	float	dimMultiple;
	
	// Work out the source and destination "bumps". The rowBytes value gives you the number
	// of bytes in each scanline of an image. This is not necessarily the same as the number
	// of pixels in a scanline multiplied by the number of bytes each pixel occupies. When
	// we copy pixels from source to destination, via our effect algorithm, we need to
	// account for this discrepancy. The following lines lines pre-calculate the differences.
	srcABump = glob->sources[0].srcRowBytes - (glob->width * 2);
	srcBBump = glob->sources[1].srcRowBytes - (glob->width * 2);
	dstBump  = glob->dstRowBytes - (glob->width * 2);
	
	// Depending on the direction we are currently fading in (fading down the first source,
	// or fading up the second) pre-calculate the percentage brightness of the pixels of the
	// destination. The dimValue always has the percentage and is set in the Begin function.
	if (glob->direction)
		dimMultiple = 1.0 - (((float) glob->dimValue) / 255.0);
	else
		dimMultiple = (((float) glob->dimValue) / 255.0);
	
	// Now, for every scanline in the source image we are dealing with...
	while (height--)
	{
		long 	width = glob->width;
		
		// ...iterate through every pixel in that scanline
		while (width--)
		{
			UInt16 	overlayPixel;
			UInt16	alpha;
			UInt16	newRed, newGreen, newBlue;
			
			// Depending on the direction, take the next pixel of the first or the
			// second source.
			if (glob->direction)
			{
				// NOTE: You must not put the increment operator inside
				// the Get16 macro, or you will end up incrementing srcA
				// multiple times per pass.
				overlayPixel = Get16(srcA);
				srcA++;
			}
			else
			{
				overlayPixel = Get16(srcB);
				srcB++;
			}
			// Call to BltMacros to ensure the pixel format is
			// converted appropriately
			cnv16SPFto16RG(overlayPixel);
			
			// Dim all of the RGB values by the same amount, and set the alpha
			alpha = 0x8000;
						
			// The following lines extract the R, G and B channels of the source
			// pixel, respectively. Each value is then multiplied by the pre-
			// calculated dimMultiple, to produce the destination R, G and B
			// channel values
			newRed   = ((overlayPixel & 0x7C00) >> 10) * dimMultiple;
			newGreen = ((overlayPixel & 0x03E0) >> 5) * dimMultiple;
			newBlue  = ((overlayPixel & 0x001F) >> 0) * dimMultiple;
			
			// Re-assemble the A, R, G and B values into a 16-bit destination pixel
			overlayPixel = (alpha) | (newRed << 10) | (newGreen << 5) | (newBlue << 0);
			
			// Set the destination pixel to be the dimmed version of the
			// appropriate source pixel. The dimmed pixel value is first
			// passed through the BltMacro to ensure the correct pixel
			// format conversion is performed
			cnv16RGto16DPF(overlayPixel);
			Set16(dst,overlayPixel);
			dst++;
		}

		// Bump the source and destination pointers we are using, to avoid
		// problems when moving from one scanline to the next
		if (glob->direction)
			srcA = (void *)(((Ptr) srcA) + srcABump);
		else
			srcB = (void *)(((Ptr) srcB) + srcBBump);
		
		dst = (void *)(((Ptr) dst) + dstBump);
	}
	
	// And we're done...
}