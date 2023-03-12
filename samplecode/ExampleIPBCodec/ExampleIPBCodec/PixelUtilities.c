/*

File: PixelUtilities.c, part of ExampleIPBCodec

Abstract: Utilities for converting between chunky YUV 4:2:2 and planar YUV 4:2:0.  

Version: 1.0

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

#include "PixelUtilities.h"

extern OSStatus CopyChunkyYUV422ToPlanarYUV420( 
		size_t width, 
		size_t height, 
		const UInt8 *baseAddr_2vuy, 
		size_t rowBytes_2vuy, 
		UInt8 *baseAddr_y, 
		size_t rowBytes_y, 
		UInt8 *baseAddr_u, 
		size_t rowBytes_u, 
		UInt8 *baseAddr_v, 
		size_t rowBytes_v )
{
	size_t x, y;
	const UInt8 *lineBase_2vuy = baseAddr_2vuy;
	UInt8 *lineBase_y = baseAddr_y;
	UInt8 *lineBase_u = baseAddr_u;
	UInt8 *lineBase_v = baseAddr_v;
	for( y = 0; y < height; y += 2 ) {
		// Take two lines at a time and average the U and V samples.
		const UInt8 *pixelPtr_2vuy_top = lineBase_2vuy;
		const UInt8 *pixelPtr_2vuy_bot = lineBase_2vuy + rowBytes_2vuy;
		UInt8 *pixelPtr_y_top = lineBase_y;
		UInt8 *pixelPtr_y_bot = lineBase_y + rowBytes_y;
		UInt8 *pixelPtr_u = lineBase_u;
		UInt8 *pixelPtr_v = lineBase_v;
		for( x = 0; x < width; x += 2 ) {
			// 2vuy contains samples clustered Cb, Y0, Cr, Y1.  
			// Convert a 2x2 block of pixels from two 2vuy pixel blocks to 4 separate Y samples, 1 U and 1 V.
			pixelPtr_y_top[0] = pixelPtr_2vuy_top[1];
			pixelPtr_y_top[1] = pixelPtr_2vuy_top[3];
			pixelPtr_y_bot[0] = pixelPtr_2vuy_bot[1];
			pixelPtr_y_bot[1] = pixelPtr_2vuy_bot[3];
			pixelPtr_u[0] = ( pixelPtr_2vuy_top[0] + pixelPtr_2vuy_bot[0] ) / 2;
			pixelPtr_v[0] = ( pixelPtr_2vuy_top[2] + pixelPtr_2vuy_bot[2] ) / 2;
			// Advance to the next 2x2 block of pixels.
			pixelPtr_2vuy_top += 4;
			pixelPtr_2vuy_bot += 4;
			pixelPtr_y_top += 2;
			pixelPtr_y_bot += 2;
			pixelPtr_u += 1;
			pixelPtr_v += 1;
		}
		lineBase_2vuy += 2 * rowBytes_2vuy;
		lineBase_y += 2 * rowBytes_y;
		lineBase_u += rowBytes_u;
		lineBase_v += rowBytes_v;
	}
	return noErr;
}

extern OSStatus CopyPlanarYUV420ToChunkyYUV422( 
		size_t width, 
		size_t height, 
		const UInt8 *baseAddr_y, 
		size_t rowBytes_y, 
		const UInt8 *baseAddr_u, 
		size_t rowBytes_u, 
		const UInt8 *baseAddr_v, 
		size_t rowBytes_v, 
		UInt8 *baseAddr_2vuy, 
		size_t rowBytes_2vuy )
{
	size_t x, y;
	const UInt8 *lineBase_y = baseAddr_y;
	const UInt8 *lineBase_u = baseAddr_u;
	const UInt8 *lineBase_v = baseAddr_v;
	UInt8 *lineBase_2vuy = baseAddr_2vuy;
	for( y = 0; y < height; y += 2 ) {
		// Take two lines at a time.
		const UInt8 *pixelPtr_y_top = lineBase_y;
		const UInt8 *pixelPtr_y_bot = lineBase_y + rowBytes_y;
		const UInt8 *pixelPtr_u = lineBase_u;
		const UInt8 *pixelPtr_v = lineBase_v;
		UInt8 *pixelPtr_2vuy_top = lineBase_2vuy;
		UInt8 *pixelPtr_2vuy_bot = lineBase_2vuy + rowBytes_2vuy;
		for( x = 0; x < width; x += 2 ) {
			// Convert a 2x2 block of pixels from 4 separate Y samples, 1 U and 1 V to two 2vuy pixel blocks.
			pixelPtr_2vuy_top[1] = pixelPtr_y_top[0];
			pixelPtr_2vuy_top[3] = pixelPtr_y_top[1];
			pixelPtr_2vuy_bot[1] = pixelPtr_y_bot[0];
			pixelPtr_2vuy_bot[3] = pixelPtr_y_bot[1];
			pixelPtr_2vuy_top[0] = pixelPtr_u[0];
			pixelPtr_2vuy_bot[0] = pixelPtr_u[0];
			pixelPtr_2vuy_top[2] = pixelPtr_v[0];
			pixelPtr_2vuy_bot[2] = pixelPtr_v[0];
			// Advance to the next 2x2 block of pixels.
			pixelPtr_2vuy_top += 4;
			pixelPtr_2vuy_bot += 4;
			pixelPtr_y_top += 2;
			pixelPtr_y_bot += 2;
			pixelPtr_u += 1;
			pixelPtr_v += 1;
		}
		lineBase_y += 2 * rowBytes_y;
		lineBase_u += rowBytes_u;
		lineBase_v += rowBytes_v;
		lineBase_2vuy += 2 * rowBytes_2vuy;
	}
	return noErr;
}
