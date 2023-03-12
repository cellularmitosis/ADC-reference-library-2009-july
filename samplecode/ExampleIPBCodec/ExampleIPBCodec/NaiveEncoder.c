/*

File: NaiveEncoder.c, part of ExampleIPBCodec

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

#include "NaiveEncoder.h"

// OR together all of the bytes in the block and return the result.
static int
getBlockDifferentBitsFromZeroBlock( 
		const UInt8 *srcBaseAddr,
		size_t srcRowBytes )
{
	int x, y;
	const UInt8 *srcLineBase = srcBaseAddr;
	UInt8 accumulator = 0;
	
	for( y = 0; y < 8; y++ ) {
		for( x = 0; x < 8; x++ ) {
			accumulator |= srcLineBase[x]; // same as srcLineBase[x] ^ 0 -- we're measuring the bits different from 0.
		}
		srcLineBase += srcRowBytes;
	}
	
	return accumulator;
}

// XOR each byte in the block with the corresponding byte in the prediction block; 
// OR together the results and return that.
static int
getBlockDifferentBitsFromBlock( 
		const UInt8 *srcBaseAddr,
		size_t srcRowBytes,
		const UInt8 *predBaseAddr,
		size_t predRowBytes )
{
	int x, y;
	const UInt8 *srcLineBase = srcBaseAddr;
	const UInt8 *predLineBase = predBaseAddr;
	UInt8 accumulator = 0;
	
	for( y = 0; y < 8; y++ ) {
		for( x = 0; x < 8; x++ ) {
			accumulator |= ( srcLineBase[x] ^ predLineBase[x] );
		}
		srcLineBase += srcRowBytes;
		predLineBase += predRowBytes;
	}
	
	return accumulator;
}

// Find the most significant and least significant set bit in the byte.
// Return the most significant set bit in *firstBit and the width of the range 
// including the most and least significant set bits in *bitCount.
// Number bits 7..0, so bit n is 1<<n.
static void
getFirstBitAndBitCount( int byte, int *firstBit, int *bitCount )
{
	int mostSignificantSetBit, leastSignificantSetBit;
	
	for( mostSignificantSetBit = 7; mostSignificantSetBit >= 0; mostSignificantSetBit-- ) {
		if( ( 1 << mostSignificantSetBit ) & byte )
			break;
	}
	for( leastSignificantSetBit = 0; leastSignificantSetBit <= 7; leastSignificantSetBit++ ) {
		if( ( 1 << leastSignificantSetBit ) & byte )
			break;
	}
	*firstBit = mostSignificantSetBit;
	if( mostSignificantSetBit < leastSignificantSetBit )
		*bitCount = 0; // byte must have been zero.
	*bitCount = mostSignificantSetBit - leastSignificantSetBit + 1;
}

// Collect the (1<<bitShift) bit from each byte of an 8x8 block, writing them into an 8-byte vector at dataPtr.
static void
encodeBitAcrossBlock( 
		const UInt8 *srcBaseAddr,
		size_t srcRowBytes,
		int bitShift,
		UInt8 *dataPtr )
{
	int x, y;
	int bit = 1<<bitShift;
	const UInt8 *srcLineBase = srcBaseAddr;
	
	for( y = 0; y < 8; y++ ) {
		UInt8 cluster = 0;
		for( x = 0; x < 8; x++ ) {
			if( srcLineBase[x] & bit )
				cluster |= ( 1 << ( 7 - x ) );
		}
		dataPtr[y] = cluster;
		srcLineBase += srcRowBytes;
	}
}

// Collect the (1<<bitShift) bit from XORing each byte of an 8x8 block with the corresponding byte of 
// an 8x8 predictor block, writing them into an 8-byte vector at dataPtr.
static void
encodeBitAcrossBlockWithPredictor( 
		const UInt8 *srcBaseAddr,
		size_t srcRowBytes,
		int bitShift,
		UInt8 *dataPtr,
		const UInt8 *predBaseAddr,
		size_t predRowBytes )
{
	int x, y;
	int bit = 1<<bitShift;
	const UInt8 *srcLineBase = srcBaseAddr;
	const UInt8 *predLineBase = predBaseAddr;
	
	for( y = 0; y < 8; y++ ) {
		UInt8 cluster = 0;
		for( x = 0; x < 8; x++ ) {
			if( ( srcLineBase[x] ^ predLineBase[x] ) & bit )
				cluster |= ( 1 << ( 7 - x ) );
		}
		dataPtr[y] = cluster;
		srcLineBase += srcRowBytes;
		predLineBase += predRowBytes;
	}
}

// Encode an 8x8 block from srcBaseAddr/srcRowBytes into the bitstream, writing it at dataPtr.
// Return in *bytesWrittenOut the number of bytes emitted.
// srcBaseAddr points to the first byte of the block; srcRowBytes is the offset from one row to the next.
// byteBudget indicates how many bytes are available for this block; if possible we should emit 
// this many bytes or fewer, but it takes 3 bytes per block to code a no-op block in our naive format.
// predAllowedArray is an array of booleans indicating which prediction buffers may be used.
// predBaseAddrArray and predRowBytesArray indicate the corresponding block of the prediction buffers.
static OSStatus
encodeBlock(
		const UInt8 *srcBaseAddr,
		size_t srcRowBytes,
		UInt8 *dataPtr,
		size_t *bytesWrittenOut,
		int byteBudget,
		const Boolean predAllowedArray[kMaxStoredFrames],
		const UInt8 *predBaseAddrArray[kMaxStoredFrames],
		const size_t predRowBytesArray[kMaxStoredFrames] )
{
	int frameIndex;
	int differentBitsFromZeroPredictor, differentBitsFromPredictor[kMaxStoredFrames];
	int chosenPredictor; // 0 for no predictor, n+1 for predictorArray[n]
	int differentBits;
	int firstBit, bitCount;
	int bitIndex;
	int bytesWritten = 0;
	
	// Find the closest block out of (a) an 8x8 block full of zeros and (b) the available predictor blocks.
	// (In a real codec, we would probably perform some kind of motion vector search.)
	differentBitsFromZeroPredictor = getBlockDifferentBitsFromZeroBlock( srcBaseAddr, srcRowBytes );
	chosenPredictor = 0;
	differentBits = differentBitsFromZeroPredictor;
	for( frameIndex = 0; frameIndex < kMaxStoredFrames; frameIndex++ ) {
		if( predAllowedArray[frameIndex] ) {
 			differentBitsFromPredictor[frameIndex] = getBlockDifferentBitsFromBlock( 
					srcBaseAddr, srcRowBytes, 
					predBaseAddrArray[frameIndex], predRowBytesArray[frameIndex] );
			if( differentBits > differentBitsFromPredictor[frameIndex] ) {
				// This predictor is a better match.
				chosenPredictor = frameIndex + 1;
				differentBits = differentBitsFromPredictor[frameIndex];
			}
		}
	}
	
	// If we have an exact match, code that; we're done.
	if( 0 == differentBits ) {
		dataPtr[0] = chosenPredictor;
		dataPtr[1] = 0;
		dataPtr[2] = 0;
		*bytesWrittenOut = 3;
	}
	else {
		// Work out which bits we would need to code to get an exact match.
		getFirstBitAndBitCount( differentBits, &firstBit, &bitCount );
		
		// If that would overflow our byte budget, slim the bit count down until we fit.
		// If we did not choose a predictor (ie, prediction from a zero block), always code at least one bit.
		// (When the budget is tight it would be smarter to spend bytes on more significant bits across the
		// frame -- this code is daft in the interests of simplicity.)
		while( ( bitCount > 0 ) && ( ( 3 + bitCount * 8 ) > byteBudget ) ) {
			if( ( bitCount == 1 ) && ( chosenPredictor == 0 ) )
				break;
			bitCount--;
		}
		if( bitCount == 0 )
			firstBit = 0;
		
		// Encode the bits we've decided to.
		dataPtr[0] = chosenPredictor;
		dataPtr[1] = firstBit;
		dataPtr[2] = bitCount;
		dataPtr += 3;
		bytesWritten += 3;
		for( bitIndex = 0; bitIndex < bitCount; bitIndex++ ) {
			if( 0 == chosenPredictor ) {
				encodeBitAcrossBlock( srcBaseAddr, srcRowBytes, firstBit - bitIndex, dataPtr );
			}
			else {
				encodeBitAcrossBlockWithPredictor( srcBaseAddr, srcRowBytes, firstBit - bitIndex, dataPtr,
						predBaseAddrArray[chosenPredictor-1], predRowBytesArray[chosenPredictor-1] );
			}
			dataPtr += 8;
			bytesWritten += 8;
		}
		*bytesWrittenOut = bytesWritten;
	}
	
	return noErr;
}

// Encode an entire plane.
// Write bytes at dataPtr and report the number of bytes written in *bytesWrittenOut.
// planeByteBudget indicates how many bytes are available for this plane; if possible we should emit 
// this many bytes or fewer, but it takes 3 bytes per block to code a no-op block in our naive format.
// predAllowedArray is an array of booleans indicating which prediction buffers may be used.
// predBaseAddrArray and predRowBytesArray indicate the corresponding plane of the prediction buffers.
static OSStatus
encodePlane(
		size_t width,
		size_t height,
		const UInt8 *srcPlaneBaseAddr,
		size_t srcPlaneRowBytes,
		UInt8 *dataPtr,
		size_t *bytesWrittenOut,
		size_t planeByteBudget,
		const Boolean predAllowedArray[kMaxStoredFrames],
		const UInt8 *predBaseAddrArray[kMaxStoredFrames],
		const size_t predRowBytesArray[kMaxStoredFrames] )
{
	OSStatus err = noErr;
	size_t widthInBlocks, heightInBlocks, xblock, yblock;
	size_t bytesWritten;
	size_t totalBytesWritten = 0;
	const UInt8 *blockLineBaseAddr;
	int frameIndex;
	const UInt8 *predLineBaseAddrArray[kMaxStoredFrames];
	float averageBlockByteBudget, runningBlockByteBudget;
	
	widthInBlocks = ( width + 7 ) / 8;
	heightInBlocks = ( height + 7 ) / 8;
	
	// We'll serve out the byte budget as we go down the frame.
	averageBlockByteBudget = ((float)planeByteBudget) / ((float)widthInBlocks * heightInBlocks);
	runningBlockByteBudget = 0.0;
	
	blockLineBaseAddr = srcPlaneBaseAddr;
	for( frameIndex = 0; frameIndex < kMaxStoredFrames; frameIndex++ )
		predLineBaseAddrArray[frameIndex] = predBaseAddrArray[frameIndex];
	
	for( yblock = 0; yblock < heightInBlocks; yblock++ ) {
		const UInt8 *blockBaseAddr = blockLineBaseAddr;
		const UInt8 *predBlockBaseAddrArray[kMaxStoredFrames];
		for( frameIndex = 0; frameIndex < kMaxStoredFrames; frameIndex++ )
			predBlockBaseAddrArray[frameIndex] = predLineBaseAddrArray[frameIndex];
		
		for( xblock = 0; xblock < widthInBlocks; xblock++ ) {
			runningBlockByteBudget += averageBlockByteBudget;
			
			err = encodeBlock( blockBaseAddr, srcPlaneRowBytes, dataPtr, &bytesWritten,
					(int)runningBlockByteBudget, predAllowedArray, 
					predBlockBaseAddrArray, predRowBytesArray );
			if( err )
				goto bail;
			
			dataPtr += bytesWritten;
			totalBytesWritten += bytesWritten;
			runningBlockByteBudget -= bytesWritten;
			
			blockBaseAddr += 8;
			for( frameIndex = 0; frameIndex < kMaxStoredFrames; frameIndex++ )
				predBlockBaseAddrArray[frameIndex] += 8;
		}
		blockLineBaseAddr += 8 * srcPlaneRowBytes;
		for( frameIndex = 0; frameIndex < kMaxStoredFrames; frameIndex++ )
			predLineBaseAddrArray[frameIndex] += 8 * predRowBytesArray[frameIndex];
	}
	
	*bytesWrittenOut = totalBytesWritten;
	
bail:
	return err;
}

static void
selectPlaneFromEachBuffer( 
		struct InternalPixelBuffer predictionBuffers[kMaxStoredFrames],
		int planeIndex,
		const UInt8 *predBaseAddrArray[kMaxStoredFrames],
		size_t predRowBytesArray[kMaxStoredFrames] )
{
	int frameIndex;
	for( frameIndex = 0; frameIndex < kMaxStoredFrames; frameIndex++ ) {
		predBaseAddrArray[frameIndex] = predictionBuffers[frameIndex].planeArray[planeIndex].planeBaseAddr;
		predRowBytesArray[frameIndex] = predictionBuffers[frameIndex].planeArray[planeIndex].planeRowBytes;
	}
}

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
		int storageIndex )
{
	OSStatus err = noErr;
	size_t totalBytesWritten = 0, bytesWrittenForPlane;
	Boolean standaloneFrame;
	int frameIndex;
	const UInt8 *predBaseAddrArray[kMaxStoredFrames];
	size_t predRowBytesArray[kMaxStoredFrames];
	UInt32 boxSize;
	OSType boxKind;
	int planeIndex, subsampling;
	size_t planeByteBudget;
	
	// Make sure we won't use the same buffer to predict a frame and to store it afterwards.
	if( ! droppableFrame )
		predAllowedArray[storageIndex] = false;
	
	// Make sure we don't use any prediction for key frames.
	if( keyFrame )
		for( frameIndex = 0; frameIndex < kMaxStoredFrames; frameIndex++ )
			predAllowedArray[frameIndex] = false;
	
	// The frame is a standalone frame (an I-frame) if it will does not use any prediction buffers.
	// Do not confuse this with a key frame, which is a standalone frame AND a sync point in
	// the sequence of frames: no later frames depend on information from earlier frames.
	standaloneFrame = true;
	for( frameIndex = 0; frameIndex < kMaxStoredFrames; frameIndex++ ) {
		if( predAllowedArray[frameIndex] )
			standaloneFrame = false;
	}
	
	// Subtract the fixed overheads from the budget we'll portion out to the planes.
	frameByteBudget -= 11 + 8 + 8 + 8 + 8;
	
	// Write the header.
	*(UInt32 *)(dataPtr + 0) = EndianU32_NtoB( 11 );
	*(OSType *)(dataPtr + 4) = EndianU32_NtoB( kNaiveHeaderCheckCode );
	dataPtr[8] = keyFrame ? 1 : 0;
	dataPtr[9] = standaloneFrame ? 1 : 0;
	dataPtr[10] = droppableFrame ? 0 : ( storageIndex + 1 );
	dataPtr += 11;
	totalBytesWritten += 11;
	
	for( planeIndex = 0; planeIndex < 3; planeIndex++ ) {
		static const OSType planeKindArray[3] = { kNaiveYPlaneCode, kNaiveCbPlaneCode, kNaiveCrPlaneCode };
		static const int planeSubsamplingArray[3] = { 1, 2, 2 };
		static const float planeProportionOfBudgetArray[3] = { 0.66, 0.17, 0.17 };
		
		boxKind = planeKindArray[planeIndex];
		subsampling = planeSubsamplingArray[planeIndex];
		planeByteBudget = planeProportionOfBudgetArray[planeIndex] * frameByteBudget;
		
		// Write the box header.  
		// We don't know yet how many bytes it will be -- we'll fix that after we write the box contents.
		*(UInt32 *)(dataPtr + 0) = EndianU32_NtoB( 0 );
		*(OSType *)(dataPtr + 4) = EndianU32_NtoB( boxKind );
		
		selectPlaneFromEachBuffer( predictionBuffers, planeIndex, predBaseAddrArray, predRowBytesArray );
		err = encodePlane( width/subsampling, height/subsampling,
				sourceBuffer->planeArray[planeIndex].planeBaseAddr,
				sourceBuffer->planeArray[planeIndex].planeRowBytes, 
				dataPtr + 8, &bytesWrittenForPlane, planeByteBudget, 
				predAllowedArray, predBaseAddrArray, predRowBytesArray );
		if( err )
			goto bail;
		
		// (In the interests of simplicity we do not apply spare byte budget from one plane to the next.)
		
		boxSize = 8 + bytesWrittenForPlane;
		*(UInt32 *)(dataPtr + 0) = EndianU32_NtoB( boxSize );
		dataPtr += boxSize;
		totalBytesWritten += boxSize;
	}

	// Write the termination atom.
	*(UInt32 *)(dataPtr + 0) = EndianU32_NtoB( 0 );
	*(UInt32 *)(dataPtr + 4) = EndianU32_NtoB( 0 );
	dataPtr += 8;
	totalBytesWritten += 8;
	
	*bytesWrittenOut = totalBytesWritten;
	
bail:
	return err;
}

// Report the maximum number of bytes it could take to encode a frame of given dimensions.
extern void
NaiveEncoder_GetMaxEncodedDataSize( size_t width, size_t height, size_t *maxBytesOut )
{
	size_t widthInMacroBlocks, heightInMacroBlocks;
	widthInMacroBlocks = ( width + 15 ) / 16;
	heightInMacroBlocks = ( height + 15 ) / 16;

	*maxBytesOut = 
		  11 + 8 + 8 + 8 + 8 // constant overheads for header and 4 boxes
		+ widthInMacroBlocks * heightInMacroBlocks * 6 * (3+8*8); // 6 blocks per macroblock, max 69 bytes each
}

// Report the minimum number of bytes it could take to encode a frame of given dimensions.
extern void
NaiveEncoder_GetMinEncodedDataSize( size_t width, size_t height, size_t *minBytesOut )
{
	size_t widthInMacroBlocks, heightInMacroBlocks;
	widthInMacroBlocks = ( width + 15 ) / 16;
	heightInMacroBlocks = ( height + 15 ) / 16;

	*minBytesOut = 
		  11 + 8 + 8 + 8 + 8 // constant overheads for header and 4 boxes
		+ widthInMacroBlocks * heightInMacroBlocks * 6 * (3+8*0); // 6 blocks per macroblock, min 3 bytes each
}
