/*
    File:  AIFFTrack.m

    Contains:  Simple class that illustrates how to implement a track data producer.

    Copyright:  (c) Copyright 2003 - 2005 Apple Computer, Inc. All rights reserved.

    Disclaimer: IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc.
    ("Apple") in consideration of your agreement to the following terms, and your
    use, installation, modification or redistribution of this Apple software
    constitutes acceptance of these terms.  If you do not agree with these terms,
    please do not use, install, modify or redistribute this Apple software.

    In consideration of your agreement to abide by the following terms, and subject
    to these terms, Apple grants you a personal, non-exclusive license, under Apple's
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
        1/24/2004     1.0.1      Fixed build issue with Panther.
*/

#import "AIFFTrack.h"
#import "EXBuffer.h"

#import <fcntl.h>

#if PRAGMA_STRUCT_ALIGN
	#pragma options align=mac68k
#elif PRAGMA_STRUCT_PACKPUSH
	#pragma pack(push, 2)
#elif PRAGMA_STRUCT_PACK
	#pragma pack(2)
#endif

typedef struct IFFChunkHeader	IFFChunkHeader;
typedef struct AIFFHeader AIFFHeader;
typedef struct AIFFChunk AIFFChunk;
typedef struct AIFFCommonChunk AIFFCommonChunk;
typedef struct AIFFSoundChunk AIFFSoundChunk;

struct IFFChunkHeader
{
	UInt32			chunkID;		// id for this chunk
	UInt32			chunkSize;		// size of this chunk not including header
};


struct AIFFChunk
{
	UInt32			chunkID;		// id for this chunk
	UInt32			chunkSize;		// size of this chunk not including header
	UInt32			data[1];		// streamed data
};

struct AIFFHeader
{
	UInt32			form;			// 'FORM'
	UInt32			formChunkSize;	// largely ignored, but it should be (filesize - 8)
	UInt32			aiff;			// 'AIFF'
	AIFFChunk		chunk[1];		// packed array of chunks
};

struct AIFFCommonChunk
{
	UInt32			comm;				// 'COMM'
	UInt32			commChunkSize;		// size of this chunk not including header (18 bytes)
	UInt16			numChannels;		// number of sound channels
	UInt32			numSampleFrames;	// number of sample frames - probably incorrect!
	UInt16			sampleSize;			// sample size in bits (eg, 16)
	UInt32			sampleFreq;			// sample frequency
	UInt8			zeroes[6];			// AIFF defines the preceding as an 80-bit IEEE 754 fp number... most folks seem to use only 4 bytes
};

struct AIFFSoundChunk
{
	UInt32			ssnd;				// 'SSND'
	UInt32			ssndChunkSize;		// size of this chunk not including header (8 bytes + samples)
	UInt32			offset;				// used for aligning, usually zero
	UInt32			blockSize;			// used for aligning, usually zero
};

const UInt32 AIFF_SAMPLE_RATE_44_1		= 0x400EAC44;
const UInt32 AIFF_SAMPLE_RATE_44_0		= 0x400DAC44;

#if PRAGMA_STRUCT_ALIGN
	#pragma options align=reset
#elif PRAGMA_STRUCT_PACKPUSH
	#pragma pack(pop)
#elif PRAGMA_STRUCT_PACK
	#pragma pack()
#endif

@implementation AIFFAudioTrack

- (id) initWithProducer:(id)producer
{
	if ((self = [super initWithProducer:producer]) != nil)
	{
		NSMutableDictionary*	properties = [[NSMutableDictionary alloc] init];
		
		[properties setObject:[(AIFFTrackProducer*)producer length] forKey:DRTrackLengthKey];
		[properties setObject:[NSNumber numberWithUnsignedShort:2352] forKey:DRBlockSizeKey];
		[properties setObject:[NSNumber numberWithInt:0] forKey:DRDataFormKey];
		[properties setObject:[NSNumber numberWithInt:0] forKey:DRBlockTypeKey];
		[properties setObject:[NSNumber numberWithInt:0] forKey:DRTrackModeKey];
		[properties setObject:[NSNumber numberWithInt:0] forKey:DRSessionFormatKey];
		
		// Audio verification is not supported right now. We could leave this out, but
		// it's here so I could add this nice comment about it.
//		[properties setObject:DRVerificationTypeProduceAgain forKey:DRVerificationTypeKey];

		[self setProperties:properties];
		[properties release];
	}
	
	return self;
}

@end

@implementation AIFFTrackProducer

// -------------------------------------------------------------------------
/* Simple, we'll write an AIFF file to CD. We're not going to do any checking here
	or before we burn since we assume that the file is a valid AIFF. Now wouldn't 
	be the place to find that out :-) */
- (id) initWithPath:(NSString*)filepath
{
	if ((self = [super init]) != nil)
	{
		// We're not going to keep the file open all this time.
		path = [filepath retain];
		
		[[self class] parseFileAtPath:filepath fileInfo:&fileInfo];
	}
	
	return self;
}

// -------------------------------------------------------------------------
- (void) dealloc
{
	[path release];
	[file release];

	[super dealloc];
}

// -------------------------------------------------------------------------
/* Return the size of the track data we'll be burning. This will  turn into the
	track length on disc. In this simple case we're just returning back to the 
	caller the length we were initialized with. In a real application, this would
	(for example) be the length of the audio file or data file or MPEG movie, etc. */
- (DRMSF*) length
{
	// Loosely, for every incoming sample frame we output a sample frame.  At least
	//	until we allow resampling, that is.
	// figure out how many actual byte sof data will be sent. This / 2352 is the length of the
	// audio in frames.
	unsigned long	byteSize = (fileInfo.dataEnd - fileInfo.dataStart) / (fileInfo.sampleBytes * fileInfo.numChannels) * 4;
	return [DRMSF msfWithFrames:byteSize / 2352];
}

#pragma mark -

// -------------------------------------------------------------------------
/* This method is called before the burn is to begin. Here's where we'll open up our
	file to read data from and then update the track length if the file got bigger. */
- (BOOL) prepareTrack:(DRTrack*)track forBurn:(DRBurn*)burn toMedia:(NSDictionary*)mediaInfo
{
	// Turn on F_NOCACHE for the file. We're not going to need this AIFF file data 
	// to be cached by the UBC.
	file = [[NSFileHandle fileHandleForReadingAtPath:path] retain];
	fcntl([file fileDescriptor], F_NOCACHE, 1);
	
	//
	// make sure we know how big the file is once and for all. This will probably never happen
	// to change, but it illustrates that you can update the track info here if need be.
	//
	[[self class] parseFile:file fileInfo:&fileInfo];
	if ([[track length] isEqualTo:[self length]] == NO)
	{
		NSMutableDictionary*	properties = [[track properties] mutableCopy];
		[properties setObject:[track length] forKey:DRTrackLengthKey];
		[track setProperties:properties];
	}
	
	[file seekToFileOffset:fileInfo.dataStart];
	cursor = fileInfo.dataStart;
	mask = ((int)-0x10000) >> fileInfo.sampleBits;

	return YES;
}

// -------------------------------------------------------------------------
/* The meat of the producer object. This method is called repeatedly during a burn.
	It's the producer's job to write data into the passed in buffer each time it's 
	called.  It's best if you can fill up the buffer completely. The buffer is
	a multiple of blockSize in length. */
- (uint32_t) produceDataForTrack:(DRTrack*)track intoBuffer:(char*)buffer length:(uint32_t)bufferLength atAddress:(uint64_t)address blockSize:(uint32_t)blockSize ioFlags:(uint32_t*)flags
{
	unsigned long		expectedFrames = (bufferLength / 4);
	unsigned long		readSize = (fileInfo.sampleBytes * fileInfo.numChannels) * expectedFrames;
	NSData*				tempData;
	const char*			tempBuffer;
	char*				outBuffer = buffer;
	unsigned long		outLength;
	int					i;
	int					step = fileInfo.sampleBytes*fileInfo.numChannels;
	
	if (readSize + cursor > fileInfo.dataEnd)
		readSize = fileInfo.dataEnd - cursor;

	tempData = [file readDataOfLength:readSize];
	tempBuffer = [tempData bytes];
	
	outLength = 0;
	for (i=0; i<readSize; i += step)
	{
		unsigned short	leftSample = 0, rightSample = 0;
		unsigned const char*	sampleFrame = (unsigned char*)tempBuffer + i;
		
		#define READ_SAMPLE_POINT(frame,index)		\
			((*(UInt16*)(frame + fileInfo.sampleBytes*index)) & mask)
		#define MIX(a,b)							\
			(((a * 2) / 3) + ((b * 2) / 3))
		
		// Handle our wonderful multichannel logic.  Yeah, it's almost certainly unnecessary,
		//	but it took an extra 2 minutes to write it this way since I was already thinking
		//	about how to parse the AIFF properly.  Maybe it'll make someone's world a better place.
		if (fileInfo.numChannels == 2)
		{
			leftSample = READ_SAMPLE_POINT(sampleFrame,0);
			rightSample = READ_SAMPLE_POINT(sampleFrame,1);
		}
		else if (fileInfo.numChannels == 1)
		{
			leftSample = READ_SAMPLE_POINT(sampleFrame,0);
			rightSample = leftSample;
		}
		else if (fileInfo.numChannels == 3)
		{
			unsigned short	centerSample = READ_SAMPLE_POINT(sampleFrame,2);
			leftSample = MIX(READ_SAMPLE_POINT(sampleFrame,0),centerSample);
			rightSample = MIX(READ_SAMPLE_POINT(sampleFrame,1),centerSample);
		}
		else if (fileInfo.numChannels == 4)
		{
			// The spec is unclear on how to distinguish quadrophonic vs 4 ch surround AIFFs, as
			//	they both have four channels.  Since surround seems to be the winner of these
			//	ancient audio wars, for now I'm going to assume 4 channels means surround.
			//	Not that it matters.
			UInt16	centerSample = MIX(READ_SAMPLE_POINT(sampleFrame,1),READ_SAMPLE_POINT(sampleFrame,3));
			leftSample = MIX(READ_SAMPLE_POINT(sampleFrame,0),centerSample);
			rightSample = MIX(READ_SAMPLE_POINT(sampleFrame,2),centerSample);
		}
		else if (fileInfo.numChannels == 6)
		{
			unsigned short	centerSample = MIX(READ_SAMPLE_POINT(sampleFrame,2),READ_SAMPLE_POINT(sampleFrame,5));
			leftSample = MIX(MIX(READ_SAMPLE_POINT(sampleFrame,0),READ_SAMPLE_POINT(sampleFrame,1)),centerSample);
			rightSample = MIX(MIX(READ_SAMPLE_POINT(sampleFrame,3),READ_SAMPLE_POINT(sampleFrame,4)),centerSample);
		}
		
		#undef READ_SAMPLE_POINT
		#undef MIX
		
		// Dump the samples into the output.
		((unsigned short*)outBuffer)[0] = NSSwapHostShortToLittle(leftSample);
		((unsigned short*)outBuffer)[1] = NSSwapHostShortToLittle(rightSample);
		outBuffer += 4;
		outLength += 4;
	}
	
	cursor += readSize;
	return outLength;
}

// -------------------------------------------------------------------------
/* This method is called after the burn is finished and all data has been written
	to disc and optionally verified. This producer, since it's
	so simple, doesn't need to do anything in it, but this would be where you would
	free up any resources allocated in prepareTrackForBurn:. */
- (void) cleanupTrackAfterBurn:(DRTrack*)track
{
	// close up the file and get rid of it.
	[file closeFile];
	[file release];
	file = nil;
}

#pragma mark -

// -------------------------------------------------------------------------
/* Parse and check the file to make sure that we can read it. */
+ (BOOL) parseFile:(NSFileHandle*)aiffFile fileInfo:(AIFFFileInfo*)info
{
	AIFFHeader			aiffHeader;
	EXBuffer*			aiffHeaderBuf;
	AIFFCommonChunk		commonChunk;
	EXBuffer*			commonChunkBuf;
	AIFFSoundChunk		soundChunk;
	EXBuffer*			soundChunkBuf;
	IFFChunkHeader		chunkHeader;
	EXBuffer*			chunkHeaderBuf;
	
	
	if (info == NULL)
		return NO;
		
	// Initialize our file info to reasonable values.
	info->numChannels = 0;
	info->sampleFreq = 0;
	info->sampleBits = 0;
	info->sampleBytes = 0;
	info->dataStart = 0;
	info->dataEnd = 0;
	
	// Set up our reader buffers. 
	aiffHeaderBuf = [EXBuffer bufferWithMemory:&aiffHeader capacity:sizeof(aiffHeader)];
	commonChunkBuf = [EXBuffer bufferWithMemory:&commonChunk capacity:sizeof(commonChunk)];
	soundChunkBuf = [EXBuffer bufferWithMemory:&soundChunk capacity:sizeof(soundChunk)];
	chunkHeaderBuf = [EXBuffer bufferWithMemory:&chunkHeader capacity:sizeof(chunkHeader)];
	
	// Read in what should be the aiff file FORM header. Check for valid values.
	[aiffFile seekToFileOffset:0];
	[aiffFile readDataIntoBuffer:aiffHeaderBuf];
	if (aiffHeader.form != 'FORM' || aiffHeader.aiff != 'AIFF') return NO;
	
	// OK, it's a valid AIFF file, search through for the common chunk. We do this
	// by reading in the "header" (my terminology) of a chunk. This will give us the 
	// chunkID and the size of the chunk. If it's not the chunk we want, we'll skip over
	// it to the next one. If the file's corrupted, we'll eventually hit EOF and 
	// fall out.
	[aiffFile seekToFileOffset:[aiffFile offsetInFile] - sizeof(AIFFChunk)];	
	chunkHeader.chunkSize = 0;
	do
	{
		[aiffFile seekToFileOffset:[aiffFile offsetInFile] + chunkHeader.chunkSize];	
		[aiffFile readDataIntoBuffer:chunkHeaderBuf];
	}
	while ([chunkHeaderBuf length] > 0 && chunkHeader.chunkID != 'COMM');

	// If we read in no bytes into our buffer, we hit the end. Goodbye
	if ([chunkHeaderBuf length] == 0)
		return NO;
	
	// Back up a bit and read in the entire Common chunk.
	[aiffFile seekToFileOffset:[aiffFile offsetInFile] - sizeof(chunkHeader)];	
	[aiffFile readDataIntoBuffer:commonChunkBuf];

	// Check for unacceptable values.
	if (commonChunk.commChunkSize != 18) {
		NSLog(@"invalid common chunk size (%ld)... ignoring", (long)commonChunk.commChunkSize);
	}
	if (commonChunk.numSampleFrames == 0) {
		NSLog(@"No samples in the file... aborting");
		return YES;		// It's valid, just nothing we can really read from. We'll get a zero length track if we try.
	}

	info->numChannels = commonChunk.numChannels;
	info->sampleFreq = commonChunk.sampleFreq;
	info->sampleBits = commonChunk.sampleSize;
	info->sampleBytes = (info->sampleBits + 7) / 8;

	if ((info->numChannels == 0) || (info->numChannels == 5) || (info->numChannels > 6)) {
		NSLog(@"invalid number of channels (%ld)", (long)info->numChannels);
		return NO;
	}
	if (info->sampleFreq != AIFF_SAMPLE_RATE_44_1 && info->sampleFreq != AIFF_SAMPLE_RATE_44_0) {
		// we could resample, in fact possibly should for 22 KHz audio.  for others just show me a reason why...
		NSLog(@"invalid sample frequency ($%08X)", (long)info->sampleFreq);
		return NO;
	}
	if ((info->sampleBits < 1) || (info->sampleBits > 128)) {
		NSLog(@"invalid sample size (%ld bits)", (long)info->sampleBits);
		return NO;
	}

	// go back the start of the file (remember chunks in an IFF file don't have to be in any order)
	// and look for the Sound chunk the same way we did the Common chunk.
	[aiffFile seekToFileOffset:12];	
	chunkHeader.chunkSize = 0;
	do
	{
		[aiffFile seekToFileOffset:[aiffFile offsetInFile] + chunkHeader.chunkSize];	
		[aiffFile readDataIntoBuffer:chunkHeaderBuf];
	}
	while ([chunkHeaderBuf length] > 0 && chunkHeader.chunkID != 'SSND');

	// If we read in no bytes into our buffer, we hit the end. Goodbye
	if ([chunkHeaderBuf length] == 0)
		return NO;
	
	// Once again, back up and read in the Sound chunk.
	[aiffFile seekToFileOffset:[aiffFile offsetInFile] - sizeof(chunkHeader)];	
	[aiffFile readDataIntoBuffer:soundChunkBuf];
	
	info->dataStart = [aiffFile offsetInFile] + soundChunk.offset;
	info->dataEnd = [aiffFile offsetInFile] + soundChunk.ssndChunkSize - 8;
	if ((soundChunk.ssndChunkSize < 8) || (info->dataEnd > [aiffFile seekToEndOfFile]))
		info->dataEnd = [aiffFile offsetInFile];
	
	return YES;
}

// -------------------------------------------------------------------------
/* This method is designed to be use by some other code (like in an open panel!)
	to determine if the file is one that this producer can read. */
+ (BOOL) parseFileAtPath:(NSString*)filepath fileInfo:(AIFFFileInfo*)info
{
	return [self parseFile:[NSFileHandle fileHandleForReadingAtPath:filepath] fileInfo:info];
}


@end

