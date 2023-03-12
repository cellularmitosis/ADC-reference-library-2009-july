/*

File: NaiveEncoder.h, part of ExampleIPBCodec

Abstract: Core routines of an encoder for a simplistic video encoding format.

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

#ifndef NAIVEENCODER_H
#define NAIVEENCODER_H

#include <QuickTime/QuickTime.h>

#include "NaiveFormat.h"

// Encode the source frame in sourceBuffer, possibly predicted from the frames in predictionBuffers.
// Write bytes at dataPtr and report the number of bytes written in *bytesWrittenOut.
// planeByteBudget indicates how many bytes are available for this plane; if possible we should emit 
// this many bytes or fewer, but we do not guarantee to hit a limit precisely.
// predAllowedArray is an array of booleans indicating which prediction buffers may be used.
// keyFrame is true if this is to be a key frame (it is the caller's responsibility to ensure 
// that frames earlier in display order have already been encoded).
// if droppableFrame is true, this is a droppable frame; if false, storageIndex indicates the 
// prediction buffer it will be stored in.
extern OSStatus
NaiveEncoder_EncodeFrame(
		size_t width,
		size_t height,
		struct InternalPixelBuffer *sourceBuffer,
		UInt8 *dataPtr,
		size_t *bytesWrittenOut,
		size_t frameByteBudget,
		Boolean predAllowedArray[kMaxStoredFrames],
		struct InternalPixelBuffer predictionBuffers[kMaxStoredFrames],
		Boolean keyFrame,
		Boolean droppableFrame,
		int storageIndex );

// Report the maximum number of bytes it could take to encode a frame of given dimensions.
extern void
NaiveEncoder_GetMaxEncodedDataSize( size_t width, size_t height, size_t *maxBytesOut );

// Report the minimum number of bytes it could take to encode a frame of given dimensions.
extern void
NaiveEncoder_GetMinEncodedDataSize( size_t width, size_t height, size_t *minBytesOut );

#endif // NAIVEENCODER_H
