/*
	File:		EffectFilter32.c
	
	Description: 32-bit Filter Implementation

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

// --------------------------------------------------------------------------------------

//
//	Code that draws our actual effect. Note that this code is very slow. 
//  It is intended only as an example/placeholder for code you will write.
//  A good first optimization would be to replace the floating-point math
//  with fixed point.
//	
//	For an explanation of what this function does, see the comments in the
//  16-bit case in the file EffectFilter16.c
//
static void EffectFilter32(BlitGlobals *glob);
static void EffectFilter32(BlitGlobals *glob)
{
	long height = glob->height;
	long *srcA = glob->sources[0].srcBaseAddr;
	long *srcB = glob->sources[1].srcBaseAddr;
	long *dst = glob->dstBaseAddr;
	long srcABump = glob->sources[0].srcRowBytes - (glob->width * 4);
	long srcBBump = glob->sources[1].srcRowBytes - (glob->width * 4);
	long dstBump = glob->dstRowBytes - (glob->width * 4);

	float	dimMultiple;
	
	if (glob->direction)
		dimMultiple = 1.0 - ((float) glob->dimValue) / 255;
	else
		dimMultiple = ((float) glob->dimValue) / 255;
		
	while (height--)
	{
		long width = glob->width;

		while (width--)
		{
			UInt32 	thePixelValue;
			UInt8	pixels[4];
			
			if (glob->direction)
			{
				thePixelValue = Get32(srcA);
				srcA++;
			}
			else
			{
				thePixelValue = Get32(srcB);
				srcB++;
			}
			// Convert the pixel format
			cnv32SPFto32AR(thePixelValue);

			// extract alpha, red, green, and blue from pixel

			pixels[0] = thePixelValue >> 24;
			pixels[1] = (thePixelValue >> 16) & 0xff;
			pixels[2] = (thePixelValue >> 8) & 0xff;
			pixels[3] = (thePixelValue >> 0) & 0xff;
			
			// Dim all of the RGB values by the same amount, and make the alpha opaque
			pixels[0] = 0xff;
			pixels[1] = pixels[1] * dimMultiple;
			pixels[2] = pixels[2] * dimMultiple;
			pixels[3] = pixels[3] * dimMultiple;
			
			thePixelValue = (pixels[0] << 24) | (pixels[1] << 16) | (pixels[2] << 8) | (pixels[3]);
				
			cnv32ARto32DPF(thePixelValue);
			Set32(dst,thePixelValue);
			dst++;
		}

		if (glob->direction)
			srcA = (void *) (((Ptr) srcA) + srcABump);
		else
			srcB = (void *) (((Ptr) srcB) + srcBBump);
			
		dst = (void *) (((Ptr) dst) + dstBump);
	}
}