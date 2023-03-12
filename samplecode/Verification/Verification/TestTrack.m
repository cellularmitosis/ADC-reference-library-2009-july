/*
    File:  TestTrack.m
     
    Contains:  Simple class that illustrates how to implement a verifying track data producer.

    Copyright:  © Copyright 2003 - 2004 Apple Computer, Inc. All rights reserved.

    Disclaimer: IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc.
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
        1/24/2004     1.0.1      Fixed build issue with Panther.

*/

#import "TestTrack.h"

@implementation TestTrack

- (id) initWithProducer:(id)producer
{
	if ((self = [super initWithProducer:producer]) != nil)
	{
		NSMutableDictionary*	properties = [[NSMutableDictionary alloc] init];
		
		[properties setObject:[(TestTrackProducer*)producer length] forKey:DRTrackLengthKey];
		[properties setObject:[NSNumber numberWithUnsignedShort:2048] forKey:DRBlockSizeKey];
		[properties setObject:[NSNumber numberWithInt:16] forKey:DRDataFormKey];
		[properties setObject:[NSNumber numberWithInt:8] forKey:DRBlockTypeKey];
		[properties setObject:[NSNumber numberWithInt:4] forKey:DRTrackModeKey];
		[properties setObject:[NSNumber numberWithInt:0] forKey:DRSessionFormatKey];

		[properties setObject:[producer verificationType] forKey:DRVerificationTypeKey];

		[self setProperties:properties];
		[properties release];
	}
	
	return self;
}

@end

@implementation TestTrackProducer

#define POLYNOMIAL 0x04c11db7L

static unsigned long crc_table[256];

/* generate the table of CRC remainders for all possible bytes */
static void GenerateCRCTable()
{
	register int i, j;  
	register unsigned long crc_accum;
	for (i = 0;  i < 256;  i++)
	{
		crc_accum = ((unsigned long) i << 24);
		for (j = 0;  j < 8;  j++)
		{
			if (crc_accum & 0x80000000L)
				crc_accum = (crc_accum << 1) ^ POLYNOMIAL;
			else
				crc_accum = (crc_accum << 1);
		}
		crc_table[i] = crc_accum;
	}
}

/* update the CRC on the data block one byte at a time */
static unsigned long UpdateCRC(unsigned long crc_accum, const char *data_blk_ptr, int data_blk_size)
{
	register int i, j;
	for (j = 0;  j < data_blk_size;  j++)
	{
		i = ((int) (crc_accum >> 24) ^ *data_blk_ptr++) & 0xff;
		crc_accum = (crc_accum << 8) ^ crc_table[i];
	}
	return crc_accum;
}

+ (void) initialize
{
	// see the random number generator for later use.
	srandom([NSDate timeIntervalSinceReferenceDate]);
	GenerateCRCTable();
}

// -------------------------------------------------------------------------
- (id) initWithLength:(DRMSF*)aLength verificationType:(NSString*)type
{
	if ((self = [super init]) != nil)
	{
		length = [aLength retain];
		verificationType = [type retain];
	}
	
	return self;
}

// -------------------------------------------------------------------------
- (void) dealloc
{
	[length release];
	[verificationType release];
	
	[super dealloc];
}

// -------------------------------------------------------------------------
- (DRMSF*) length
{
	return length;
}

// -------------------------------------------------------------------------
- (NSString*) verificationType
{
	return verificationType;
}

#pragma mark -

// -------------------------------------------------------------------------
/* This method is called before the burn is to begin. */
- (BOOL) prepareTrack:(DRTrack*)track forBurn:(DRBurn*)burn toMedia:(NSDictionary*)mediaInfo
{
	productionChecksum = 0xFFFFFFFF;
	
	return YES;
}

// -------------------------------------------------------------------------
/* The meat of the producer object. This method is called repeatedly during a burn.
	It's the producer's job to write data into the passed in buffer each time it's 
	called.  It's best if you can fill up the buffer completely. The buffer is
	a multiple of blockSize in length. */
- (uint32_t) produceDataForTrack:(DRTrack*)track intoBuffer:(char*)buffer length:(uint32_t)bufferLength atAddress:(uint64_t)address blockSize:(uint32_t)blockSize ioFlags:(uint32_t*)flags
{
	uint32_t		expectedFrames = (bufferLength / 4);
	
	if ([[self verificationType] isEqualToString:DRVerificationTypeReceiveData])
	{
		while (expectedFrames--)
		{
			((unsigned long*)buffer)[expectedFrames] = (unsigned long)random();
		}
		productionChecksum = UpdateCRC(productionChecksum, buffer, bufferLength);
	}
	else
	{
		while (expectedFrames--)
		{
			((long*)buffer)[expectedFrames] = expectedFrames;
		}
	}
	
	return bufferLength;
}

// -------------------------------------------------------------------------
/* This method is called after the data has been burned to the disc, but right before
	the data is verified. If no verification is asked for, this method won't
	be called. Rewind file pointers to 0, or do whatever is needed to get ready to
	start verifying. */
- (BOOL) prepareTrackForVerification:(DRTrack*)track
{
	verificationChecksum = 0xFFFFFFFF;
	
	return YES;
}

// -------------------------------------------------------------------------
/* OK, we're doing something to illustrate a point. The track producer has asked for
	DRTrackVerificationReceiveData. This means that every bit of data written to
	CD will be sent back to us through this method for us to verify. It's very similar
	to a production call, except that we need to do verification of the buffer passed in. 
	
	Don't think you have to do it this way. The more typical way of verifying a disc
	is to set the verification type of the track to DRTrackVerificationProduceAgain
	and let the engine do the checking for you. This method is here if you really need
	to do the checking yourself. */
- (BOOL) verifyDataForTrack:(DRTrack*)track inBuffer:(const char*)buffer length:(uint32_t)bufferLength atAddress:(uint64_t)address blockSize:(uint32_t)blockSize ioFlags:(uint32_t*)flags
{
	verificationChecksum = UpdateCRC(verificationChecksum, buffer, bufferLength);

	return YES;	// we won't know for sure if it verified until the end.
}

// -------------------------------------------------------------------------
/* This method is sent when verification is all done and it verified correct.
	Last chance to say something is wrong!! */
- (BOOL) cleanupTrackAfterVerification:(DRTrack*)track
{
	// This works because in the non-checksumming case (DRTrackVerificationProduceAgain)
	// both checksum values are 0xFFFFFFFF. In the other case (DRTrackVerificationReceiveData)
	// they had better be the same or else something is wrong.
	return verificationChecksum == productionChecksum;
}

// -------------------------------------------------------------------------
/* This method is called after the burn is finished and all data has been written
	to disc and optionally verified. This producer, doesn't need to do anything in it, 
	but this would be where you would free up any resources allocated in preBurn. */
- (void) cleanupTrackAfterBurn:(DRTrack*)track
{
}

@end