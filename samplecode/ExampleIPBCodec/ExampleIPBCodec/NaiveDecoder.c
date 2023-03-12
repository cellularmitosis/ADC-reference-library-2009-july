/*

File: NaiveDecoder.c, part of ExampleIPBCodec

Abstract: Core routines of a decoder for a simplistic video encoding format.

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

#include "NaiveDecoder.h"

// Decode an 8x8 I-block from the bitstream, writing it at destBaseAddr/destRowBytes.
// dataPtr points to the second byte of the encoded block.
// destBaseAddr points where to write the decoded block; destRowBytes is the offset from one row to the next.
// Return in bytesConsumedOut the number of bytes consumed in the bitstream.
static OSStatus 
decodeStandaloneBlock( 
		const UInt8 *dataPtr, 
		int *bytesConsumedOut,
		UInt8 *destBaseAddr,
		size_t destRowBytes )
{
	int firstBit = dataPtr[0];
	int numberOfBits = dataPtr[1];
	int bitIndex, x, y;
	UInt8 *destLine;
	UInt8 bit;
	
	dataPtr += 2;
	
	// Sanity check.
	if( ( firstBit > 7 ) || ( numberOfBits > 8 ) )
		return codecBadDataErr;
	
	// Clear the block.
	destLine = destBaseAddr;
	for( y = 0; y < 8; y++ ) {
		for( x = 0; x < 8; x++ ) {
			destLine[x] = 0;
		}
		destLine += destRowBytes;
	}
	
	// OR in the other bits.
	bit = 1 << firstBit;
	for( bitIndex = 0; bitIndex < numberOfBits; bitIndex++ ) {
		destLine = destBaseAddr;
		for( y = 0; y < 8; y++ ) {
			UInt8 cluster = *dataPtr++;
			// Spread the 8 bits of cluster across the indexed bit of the 8 output bytes.
			for( x = 0; x < 8; x++ ) {
				if( cluster & ( 1 << ( 7 - x ) ) )
					destLine[x] |= bit;
			}
			destLine += destRowBytes;
		}
		bit = bit >> 1; 
	}
	
	*bytesConsumedOut = 2 + 8 * numberOfBits;
	
	return noErr;
}

// Decode an 8x8 predicted block from the bitstream, writing it at destBaseAddr/destRowBytes.
// dataPtr points to the second byte of the encoded block.
// predBaseAddr points to the prediction block; predRowBytes is the offset from one row to the next.
// destBaseAddr points where to write the decoded block; destRowBytes is the offset from one row to the next.
// Return in bytesConsumedOut the number of bytes consumed in the bitstream.
static OSStatus 
decodePredictedBlock( 
		const UInt8 *dataPtr, 
		int *bytesConsumedOut,
		const UInt8 *predBaseAddr,
		size_t predRowBytes, 
		UInt8 *destBaseAddr,
		size_t destRowBytes )
{
	int firstBit = dataPtr[0];
	int numberOfBits = dataPtr[1];
	int bitIndex, x, y;
	const UInt8 *predLine;
	UInt8 *destLine;
	UInt8 bit;
	
	dataPtr += 2;
	
	// Sanity check.
	if( ( firstBit > 7 ) || ( numberOfBits > 8 ) )
		return codecBadDataErr;
	
	// Copy the prediction block.
	predLine = predBaseAddr;
	destLine = destBaseAddr;
	for( y = 0; y < 8; y++ ) {
		for( x = 0; x < 8; x++ ) {
			destLine[x] = predLine[x];
		}
		predLine += predRowBytes;
		destLine += destRowBytes;
	}
	
	// XOR in the bits.
	bit = 1 << firstBit;
	for( bitIndex = 0; bitIndex < numberOfBits; bitIndex++ ) {
		destLine = destBaseAddr;
		for( y = 0; y < 8; y++ ) {
			UInt8 cluster = *dataPtr++;
			// Spread the 8 bits of cluster across the indexed bit of the 8 output bytes.
			for( x = 0; x < 8; x++ ) {
				if( cluster & ( 1 << ( 7 - x ) ) )
					destLine[x] ^= bit;
			}
			destLine += destRowBytes;
		}
		bit = bit >> 1; 
	}
	
	*bytesConsumedOut = 2 + 8 * numberOfBits;
	
	return noErr;
}

// Decode a plane of 8x8 blocks from the bitstream.
// destBaseAddr points where to write the decoded block; destRowBytes is the offset from one row to the next.
static OSStatus 
decodePlane(
		size_t width,
		size_t height,
		const UInt8 **dataPtrInOut, 
		ICMDataProcRecord *dataProc,
		size_t *bytesConsumedOut,
		UInt8 *predBaseAddrArray[kMaxStoredFrames],
		size_t predRowBytesArray[kMaxStoredFrames],
		UInt8 *destPlaneBaseAddr,
		size_t destRowBytes )
{
	OSStatus err = noErr;
	const UInt8 *dataPtr = *dataPtrInOut;
	size_t widthInBlocks, heightInBlocks, xblock, yblock;
	int bytesConsumed;
	size_t totalBytesConsumed = 0;
	UInt8 *blockLineBaseAddr = destPlaneBaseAddr;
	
	widthInBlocks = ( width + 7 ) / 8;
	heightInBlocks = ( height + 7 ) / 8;
	
	for( yblock = 0; yblock < heightInBlocks; yblock++ ) {
		UInt8 *blockBaseAddr = blockLineBaseAddr;
		
		if( dataProc ) {
			// Call the data-loading proc to ensure that we have enough data loaded to 
			// output one row of blocks.  (With our data format, the most data we might 
			// need for n blocks is n*(3+8*8).)  It is safe to ask for up to 32K of data
			// at a time.
			// NOTE: During normal movie playback, the data will be fully loaded and
			// dataProc will be NULL.  However, video frames may be transferred to other
			// containers such as PICT files and QuickTime Image files, and when drawing
			// them the data will be provided using data-loading procs.
			long bytesNeeded = widthInBlocks * (3+8*8);
			dataProc->dataProc( (Ptr *)&dataPtr, bytesNeeded, dataProc->dataRefCon );
		}
		
		for( xblock = 0; xblock < widthInBlocks; xblock++ ) {
			UInt8 predIndex = dataPtr[0];
			dataPtr += 1;
			totalBytesConsumed += 1;
			
			if( predIndex > kMaxStoredFrames ) {
				// "Predictor index out of range"
				err = codecBadDataErr;
				goto bail;
			}
			
			if( 0 == predIndex ) {
				err = decodeStandaloneBlock( 
						dataPtr, &bytesConsumed, blockBaseAddr, destRowBytes );
				if( err )
					goto bail;
			}
			else {
				const UInt8 *predBaseAddr = predBaseAddrArray[predIndex-1];
				size_t predRowBytes = predRowBytesArray[predIndex-1];
				predBaseAddr += yblock * 8 * predRowBytes + xblock * 8;
				
				err = decodePredictedBlock( 
						dataPtr, &bytesConsumed, predBaseAddr, predRowBytes, blockBaseAddr, destRowBytes );
				if( err )
					goto bail;
			}
			dataPtr += bytesConsumed;
			totalBytesConsumed += bytesConsumed;
			
			blockBaseAddr += 8;
		}
		blockLineBaseAddr += 8 * destRowBytes;
	}
	
	*dataPtrInOut = dataPtr;
	*bytesConsumedOut = totalBytesConsumed;
bail:
	return err;
}

// Read the 16-byte header at the start of the frame without advancing dataPtr.
extern OSStatus
NaiveDecoder_DecodeFrameHeader( 
		const UInt8 *dataPtr, 
		Boolean *keyFrameOut,
		Boolean *differenceFrameOut,
		Boolean *droppableFrameOut,
		int *storageIndexOut )
{
	OSStatus err = noErr;
	UInt32 boxSize;
	UInt8 keyFrame, differenceFrame, storageIndex;
	
	// Check that the header is the right length.
	boxSize = *(UInt32 *)(dataPtr + 0);
	boxSize = EndianU32_BtoN( boxSize );
	if( 11 != boxSize ) {
		// "Header size check failed"
		err = codecDataVersErr;
		goto bail;
	}
	
	// Check the header has the right check code.
	if( EndianU32_NtoB(kNaiveHeaderCheckCode) != *(OSType *)(dataPtr + 4) ) {
		// "Header check code incorrect"
		err = codecDataVersErr;
		goto bail;
	}
	
	keyFrame = dataPtr[8];
	if( keyFrame > 1 ) {
		// "Bad keyFrame value"
		err = codecBadDataErr;
		goto bail;
	}
	*keyFrameOut = keyFrame;
	
	differenceFrame = dataPtr[9];
	if( differenceFrame > 1 ) {
		// "Bad differenceFrame value"
		err = codecBadDataErr;
		goto bail;
	}
	*differenceFrameOut = differenceFrame;
	
	storageIndex = dataPtr[10];
	if( storageIndex > kMaxStoredFrames ) {
		// "Bad storageIndex value"
		err = codecBadDataErr;
		goto bail;
	}
	*droppableFrameOut = (storageIndex == 0);
	*storageIndexOut = storageIndex - 1;
	
bail:
	return err;
}

static void
selectPlaneFromEachBuffer( 
		struct InternalPixelBuffer predictionBuffers[kMaxStoredFrames],
		int planeIndex,
		UInt8 *predBaseAddrArray[kMaxStoredFrames],
		size_t predRowBytesArray[kMaxStoredFrames] )
{
	int frameIndex;
	for( frameIndex = 0; frameIndex < kMaxStoredFrames; frameIndex++ ) {
		predBaseAddrArray[frameIndex] = predictionBuffers[frameIndex].planeArray[planeIndex].planeBaseAddr;
		predRowBytesArray[frameIndex] = predictionBuffers[frameIndex].planeArray[planeIndex].planeRowBytes;
	}
}

// Read the frame payload and decode a frame of the given dimensions into destBuffer.
// dataPtrInOut points to a variable that points to the incoming datastream; 
// it will be updated with the advanced datastream pointer after a successful decode.
// dataProc points to a data-loading callback record if the codec is being
// called with one; otherwise it is NULL.  
// dataSize is the total size of compressed data if available, 0 if not.
// (Note that some legacy APIs allow applications to request image decompression 
// without providing the data size; decompressors should handle this case robustly.)
// predictionBuffers is an array of planar YUV 4:2:0 buffers containing 
// previously-decoded frames that this frame may depend upon.
extern OSStatus
NaiveDecoder_DecodeFramePayload( 
		size_t width,
		size_t height,
		const UInt8 **dataPtrInOut, 
		ICMDataProcRecord *dataProc,
		size_t dataSize,
		struct InternalPixelBuffer predictionBuffers[kMaxStoredFrames],
		struct InternalPixelBuffer *destBuffer )
{
	OSStatus err = noErr;
	const UInt8 *dataPtr = *dataPtrInOut;
	size_t bytesConsumed = 0, bytesConsumedByPlane;
	UInt32 boxSize;
	OSType boxKind;
	UInt8 *predBaseAddrArray[kMaxStoredFrames];
	size_t predRowBytesArray[kMaxStoredFrames];
	int planeIndex, subsampling;
	Boolean didPlane[3] = { false, false, false };
	
	// Our codec data is structured as a sequence of boxes/atoms, each starting with a 4 byte size and 4 byte kind.  
	// We ignore unrecognised boxes provided that they have legal sizes, so the data format can be extended later
	// by introducing new box types.
	while( ( dataSize == 0 ) || ( bytesConsumed + 4 <= dataSize ) ) {
		boxSize = *(UInt32 *)(dataPtr + 0);
		boxSize = EndianU32_BtoN( boxSize );
		if( 0 == boxSize ) {
			// End of frame data indicator.
			break;
		}
		
		if( ( boxSize < 8 ) || ( ( dataSize != 0 ) && ( bytesConsumed + boxSize > dataSize ) ) ) {
			// "Illegal box size"
			err = codecBadDataErr;
			goto bail;
		}
		
		boxKind = *(OSType *)(dataPtr + 4);
		boxKind = EndianU32_BtoN( boxKind );
		dataPtr += 8;
		bytesConsumed += 8;
		boxSize -= 8;
		
		switch( boxKind ) {
			case kNaiveYPlaneCode:
			case kNaiveCbPlaneCode:
			case kNaiveCrPlaneCode:
				switch( boxKind ) {
					case kNaiveYPlaneCode:
						planeIndex = 0;
						subsampling = 1;
						break;
				
					case kNaiveCbPlaneCode:
						planeIndex = 1;
						subsampling = 2;
						break;
				
					case kNaiveCrPlaneCode:
						planeIndex = 2;
						subsampling = 2;
						break;
				}
				
				if( didPlane[planeIndex] ) {
					// "Plane repeated"
					err = codecBadDataErr;
					goto bail;
				}
				selectPlaneFromEachBuffer( predictionBuffers, planeIndex, predBaseAddrArray, predRowBytesArray );
				err = decodePlane( width/subsampling, height/subsampling, 
						&dataPtr, dataProc, &bytesConsumedByPlane, 
						predBaseAddrArray, predRowBytesArray,
						destBuffer->planeArray[planeIndex].planeBaseAddr,
						destBuffer->planeArray[planeIndex].planeRowBytes );
				if( err )
					goto bail;
				if( bytesConsumedByPlane != boxSize ) {
					// "Plane data consumed did not fit box size"
					err = codecBadDataErr;
					goto bail;
				}
				// decodePlane advances dataPtr
				bytesConsumed += bytesConsumedByPlane;
				didPlane[planeIndex] = true;
				break;
				
			default:
				dataPtr += boxSize;
				bytesConsumed += boxSize;
				break;
		}
	}
	
	if( didPlane[0] && didPlane[1] && didPlane[2] ) {
		err = noErr;
	}
	else {
		// "Plane data missing"
		err = codecBadDataErr;
		goto bail;
	}
	
	*dataPtrInOut = dataPtr;
bail:
	return err;
}
