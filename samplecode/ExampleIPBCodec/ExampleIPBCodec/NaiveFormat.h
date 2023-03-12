/*

File: NaiveFormat.h, part of ExampleIPBCodec

Abstract: Definitions for a naive compression format and an internal planar YUV 4:2:0 format.

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

#ifndef NAIVEFORMAT_H
#define NAIVEFORMAT_H

#include <Carbon/Carbon.h>

/*
  Frame data format:
    4 bytes: header size = 11
    4 bytes: check code (kNaiveHeaderCheckCode)
	1 byte:  1 if this frame is a reset point for the stream (key frame / sync frame), 0 if not
    1 byte:  0 if a standalone frame, 1 if frame is predicted from prior frames
    1 byte:  0 if droppable, or buffer index if this frame should be stored to help decode other frames (it cannot be predicted from a frame in the same buffer)
    4 bytes: p+8
    4 bytes: 'y ->' (kNaiveYPlaneCode)
    p bytes: Y encoded plane (see below for plane format)
    4 bytes: q+8
    4 bytes: 'u ->' (kNaiveCbPlaneCode)
    q bytes: Cb encoded plane (downsampled by 2x2)
	4 bytes: r+8
    4 bytes: 'v ->' (kNaiveCrPlaneCode)
    r bytes: Cr encoded plane (downsampled by 2x2)
    4 bytes: 0
    4 bytes: 0
	
  Encoded plane data format:
    for each 8x8 block:
      1 byte: 0 for I-block, or 1-based source buffer index for prediction
      1 byte: first bit = s (bits labelled 7..0).  0 if no bits stored.
      1 byte: # bits stored = t.  0 if no bits stored.
      8*t bytes: 1 bit per byte; first MSB bit is for top-left byte of block, next bit is one byte right, etc.
 */

enum {
	kNaiveHeaderCheckCode = 'naiv',
	kNaiveYPlaneCode = 'y ->',
	kNaiveCbPlaneCode = 'u ->',
	kNaiveCrPlaneCode = 'v ->'
};

enum {
	kMaxStoredFrames = 2
};

struct InternalPixelPlane {
	UInt8 *planeBaseAddr;
	size_t planeRowBytes;
};
struct InternalPixelBuffer {
	struct InternalPixelPlane planeArray[3];
};

// Utilities to free and allocate internal pixel buffers.
extern void 
freeInternalPixelBuffer( struct InternalPixelBuffer *buffer );

extern OSStatus 
callocInternalPixelBuffer( size_t width, size_t height, struct InternalPixelBuffer *buffer );

#endif // NAIVEFORMAT_H
