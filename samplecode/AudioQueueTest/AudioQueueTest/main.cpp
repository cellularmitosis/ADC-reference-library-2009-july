/*

File:main.cpp

Abstract: simple demonstration of AudioQueue

Version: 1.0

Disclaimer: IMPORTANT:  This Apple software is supplied to you by Apple
Computer, Inc. ("Apple") in consideration of your agreement to the
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

Copyright © 2006 Apple Computer, Inc., All Rights Reserved

*/
#include <CoreServices/CoreServices.h>
#include <AudioToolbox/AudioQueue.h>
#include <AudioToolbox/AudioFile.h>

// helpers
#include "CAXException.h"
#include "CAStreamBasicDescription.h"

static const int kNumberBuffers = 2;
static int gBufferSizeBytes = 0x10000;

struct AQTestInfo {
	AudioFileID						mAudioFile;
	CAStreamBasicDescription		mDataFormat;
	AudioQueueRef					mQueue;
	AudioQueueBufferRef				mBuffers[kNumberBuffers];
	UInt64							mPacketIndex;
	UInt32							mNumPacketsToRead;
	AudioStreamPacketDescription *	mPacketDescs;
	bool							mDone;
};

// This function will be called upon to provide data when the audio queue calls it.
// It can also be called manually shortly after initialization to prime to queue.
static void AQTestBufferCallback(void *					inUserData,
								AudioQueueRef			inAQ,
								AudioQueueBufferRef		inCompleteAQBuffer) 
{
	AQTestInfo * myInfo = (AQTestInfo *)inUserData;
	if (myInfo->mDone) return;
		
	UInt32 numBytes;
	UInt32 nPackets = myInfo->mNumPacketsToRead;

	// Read nPackets worth of data into buffer
	XThrowIfError (AudioFileReadPackets(myInfo->mAudioFile, false, &numBytes, myInfo->mPacketDescs, myInfo->mPacketIndex, &nPackets, 
								inCompleteAQBuffer->mAudioData), "AudioFileReadPackets failed");
		
	if (nPackets > 0) {
		inCompleteAQBuffer->mAudioDataByteSize = numBytes;		

		// Queues the buffer for audio input/output.
		AudioQueueEnqueueBuffer(inAQ, inCompleteAQBuffer, (myInfo->mPacketDescs ? nPackets : 0), myInfo->mPacketDescs);
		
		myInfo->mPacketIndex += nPackets;
	} else {
		XThrowIfError(AudioQueueStop(myInfo->mQueue, false), "AudioQueueStop(false) failed");
			// reading nPackets == 0 is our EOF condition
		myInfo->mDone = true;
	}
}


int main (int argc, const char * argv[]) 
{
	if (argc < 2) {
		printf ("Must specifiy a file path\n"); 
		exit(1);
	}
	
	UInt8 *filePath = (UInt8 *)argv[1];
	
	printf ("Playing file: %s\n", filePath);
	
	AQTestInfo myInfo;
	
	FSRef fsref;	
	XThrowIfError(FSPathMakeRef(filePath, &fsref, NULL), "Input file not found");
		
	XThrowIfError(AudioFileOpen(&fsref, fsRdPerm, 0, &myInfo.mAudioFile), "AudioFileOpen failed");
		
	UInt32 size = sizeof(myInfo.mDataFormat);
	XThrowIfError(AudioFileGetProperty(myInfo.mAudioFile, 
								kAudioFilePropertyDataFormat, &size, &myInfo.mDataFormat), "couldn't get file's data format");
	
	printf ("File format: "); myInfo.mDataFormat.Print();

	XThrowIfError(AudioQueueNewOutput(&myInfo.mDataFormat, AQTestBufferCallback, &myInfo, NULL, NULL, 0, &myInfo.mQueue), "AudioQueueNew failed");

	// We have a couple of things to take care of now
	// (1) Setting up the conditions around VBR or a CBR format - affects how we will read from the file
		// if format is VBR we need to use a packet table.
	if (myInfo.mDataFormat.mBytesPerPacket == 0 || myInfo.mDataFormat.mFramesPerPacket == 0) {
		// first check to see what the max size of a packet is - if it is bigger
		// than our allocation default size, that needs to become larger
		UInt32 maxPacketSize;
		size = sizeof(maxPacketSize);
		XThrowIfError(AudioFileGetProperty(myInfo.mAudioFile, 
								kAudioFilePropertyPacketSizeUpperBound, &size, &maxPacketSize), "couldn't get file's max packet size");
		if (maxPacketSize > gBufferSizeBytes) 
			gBufferSizeBytes = maxPacketSize;
		
		// we also need packet descpriptions for the file reading
		myInfo.mNumPacketsToRead = gBufferSizeBytes / maxPacketSize;
		myInfo.mPacketDescs = new AudioStreamPacketDescription [myInfo.mNumPacketsToRead];
		
	} else {
		myInfo.mNumPacketsToRead = gBufferSizeBytes / myInfo.mDataFormat.mBytesPerPacket;
		myInfo.mPacketDescs = NULL;
	}

	// (2) If the file has a cookie, we should get it and set it on the AQ
	size = sizeof(UInt32);
	OSStatus result = AudioFileGetPropertyInfo (myInfo.mAudioFile, kAudioFilePropertyMagicCookieData, &size, NULL);

	if (!result && size) {
		char* cookie = new char [size];		
		XThrowIfError (AudioFileGetProperty (myInfo.mAudioFile, kAudioFilePropertyMagicCookieData, &size, cookie), "get cookie from file");
		XThrowIfError (AudioQueueSetProperty(myInfo.mQueue, kAudioQueueProperty_MagicCookie, cookie, size), "set cookie on queue");
		delete [] cookie;
	}
	
	
		// prime the queue with some data before starting
	myInfo.mDone = false;
	myInfo.mPacketIndex = 0;
	for (int i = 0; i < kNumberBuffers; ++i) {
		XThrowIfError(AudioQueueAllocateBuffer(myInfo.mQueue, gBufferSizeBytes, &myInfo.mBuffers[i]), "AudioQueueAllocateBuffer failed");

		AQTestBufferCallback (&myInfo, myInfo.mQueue, myInfo.mBuffers[i]);
		
		if (myInfo.mDone) break;
	}	
		
		// lets start playing now - stop is called in the AQTestBufferCallback when there's
		// no more to read from the file
	XThrowIfError(AudioQueueStart(myInfo.mQueue, NULL), "AudioQueueStart failed");
	
	do {
		CFRunLoopRunInMode(kCFRunLoopDefaultMode, 0.25, false);
	} while (myInfo.mDone == false);
	
		// $$$ NOT IMPLEMENTED YET - should use the IsRunning property to determine
		// when the AQ is actually finished running
	CFRunLoopRunInMode(kCFRunLoopDefaultMode, 2, false);

	XThrowIfError(AudioQueueDispose(myInfo.mQueue, true), "AudioQueueDispose(true) failed");
	XThrowIfError(AudioFileClose(myInfo.mAudioFile), "AudioQueueDispose(false) failed");
	delete [] myInfo.mPacketDescs;
	
    return 0;
}
