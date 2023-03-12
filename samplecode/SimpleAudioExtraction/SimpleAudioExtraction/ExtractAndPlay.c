/*
	File:		ExtractAndPlay
	
	Description: Code to use the Audio Extraction APIs to extract audio out of QuickTime movies 

	Author:		Apple DTS

	Copyright: 	© Copyright 2005 Apple Computer, Inc. All rights reserved.
	
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

*/

#import <CoreAudio/CoreAudio.h>

#include "ExtractAndPlay.h"
#include "CoreAudioUtils.h"
#include "MovieAudioExtractionUtils.h"

#pragma mark ----- Typedefs -----

// NEW

struct ExtractionInfoForCallback
{
    AudioStreamBasicDescription	asbd;
    AudioChannelLayout          *layout;				
    UInt32                      layoutSize;
    
    Boolean                     discrete;
    SInt64                      samplesRemaining;
    Boolean                     playerUnitStarted;
    ScheduledAudioSlice         *sliceList;
    Float64                     sampleTimeStamp;

    MovieAudioExtractionRef     extractionSessionRef;
    TimeRecord                  startTime;
    Float64                     duration;	

};
typedef struct ExtractionInfoForCallback ExtractionInfoForCallback;


static Movie				mCurrentMovie = nil;
static AUGraphPlayerRef		mAUGraphUnit;
static AudioUnit			mPlayerUnit, mEffectUnit;
static CFRunLoopSourceRef	mRunLoopRef;

static	Boolean			mStopPreview;
static	Boolean			mUseEffectAU;

static	TimeRecord		*mStartTimePtr;
static	TimeRecord		mStartTime;


#pragma mark ----- Constants -----

// number of slices we try to keep in the play queue
const UInt32 kNumSlices = 30;

// Maximum size, in frames, of the MovieAudioExtractionFillBuffer calls
const UInt32 kMaxExtractionPacketCount	= 4096;

// flags used to update our UI (play button)
const Boolean kExtractionStarted	= false;
const Boolean kExtractionStopped	= true;
	
#ifndef fieldOffset
	#define fieldOffset(type, field) ((size_t) &((type *) 0)->field)
#endif

#pragma mark ----- Prototypes -----

static const void *MyInfoRetainCallBack (const void *info);
static void MyInfoReleaseCallBack (const void *info);
static void MyRunLoopPerformCallBack (void *info);
static void MyRunLoopScheduleCallBack (void *info, 
                                       CFRunLoopRef rl, 
                                       CFStringRef mode);
static OSStatus ExtractSliceAndScheduleToPlay(MovieAudioExtractionRef extraction, 
                                                    AudioStreamBasicDescription asbd, 
                                                    AudioUnit playerUnit, 	
                                                    ScheduledAudioSlice* slice, 
                                                    SInt64 *ioNumSamples,
                                                    Boolean *extractionComplete);
static UInt32 PreviewBufferScheduleSlices(ScheduledAudioSlice *sliceList,
                                        UInt32 numSlices,
                                        MovieAudioExtractionRef extraction,
                                        AudioStreamBasicDescription asbd,
                                        Float64 * ioSampleTimeStamp,
                                        SInt64* ioSamplesRemaining,
                                        Boolean * outExtractionComplete);
static OSStatus PreviewBufferAllocate(ScheduledAudioSlice *sliceList,
                                        UInt32 numSlices,
                                        AudioStreamBasicDescription asbd,
                                        void * condLock);
static void PreviewBufferDeallocate(ScheduledAudioSlice *sliceList, UInt32 numSlices);
static void PreviewCompleted();
static void PreviewOnMainThreadCallBack(ExtractionInfoForCallback *info);
static void StartPreview();
static OSStatus GetMovieExtractionParameters(   Movie                           theMovie,
                                                AudioChannelLayout              **layout, 
                                                UInt32                          *layoutSize,
                                                AudioStreamBasicDescription		*asbd, 
                                                TimeRecord                      *startTime,
                                                Float64                         *duration, 
                                                Boolean                         *allDiscrete);

#pragma mark ----- Run Loop callbacks -----

// 	The callback used to add a retain for the source on
//    the info pointer for the life of the source, and may be
//    used for temporary references the source needs to take.
//    This callback returns the actual info pointer to store in
//    the source, almost always just the pointer passed as the
//    parameter.
static const void *MyInfoRetainCallBack (const void *info)
{
	return (info);
}

// 	The callback used to remove a retain previously
//    added for the source on the info pointer.
static void MyInfoReleaseCallBack (const void *info)
{
	free((void *)info);
}

// 	This callback is called when the source has been marked "signaled" with the
//    CFRunLoopSourceSignal() function, and should do whatever
//    handling is required for the source.
static void MyRunLoopPerformCallBack (void *info)
{
	PreviewOnMainThreadCallBack((ExtractionInfoForCallback *)info);
}

// This callback is called whenever the source is added to a run loop mode. 
static void MyRunLoopScheduleCallBack ( void *info, 
                                        CFRunLoopRef rl, 
                                        CFStringRef mode)
{
	PreviewOnMainThreadCallBack(info);
}



#pragma mark ----- AUScheduledSoundPlayer  -----

// Extract a slice of audio and schedule it for play in the AUScheduledSoundPlayer.
// The ScheduledAudioSlice parameter contains the AudioBufferList to fill with
// extracted audio data (which is always deinterleaved, since we're passing
// it along to an AUGraph).  'playerUnit' specifies an AUScheduledSoundPlayer
// instance to which the filled slice is queued.
static OSStatus ExtractSliceAndScheduleToPlay(MovieAudioExtractionRef extraction, 
                                                AudioStreamBasicDescription asbd, 
                                                AudioUnit playerUnit, 	
                                                ScheduledAudioSlice* slice, 
                                                SInt64 *ioNumSamples,
                                                Boolean *extractionComplete)
{
    OSStatus	err = noErr;
    UInt32		bufIndex;
    UInt32		flags;
    UInt32		numFrames = *ioNumSamples;

	// Make sure the buffer list has all the buffer sizes reset.
	for (bufIndex = 0; bufIndex < slice->mBufferList->mNumberBuffers; bufIndex++) 
	{
		slice->mBufferList->mBuffers[bufIndex].mDataByteSize = *ioNumSamples * asbd.mBytesPerPacket;
	}

	// Extract into the slice's bufferlist
	err = MovieAudioExtractionFillBuffer(extraction, &numFrames, slice->mBufferList, &flags);
	*extractionComplete = (flags & kQTMovieAudioExtractionComplete);
	if (!err && (numFrames != 0)) 
	{
        // Fill in the slice frame count
        slice->mNumberFrames = numFrames;
        err = AudioUnitSetProperty(playerUnit, 
                                   kAudioUnitProperty_ScheduleAudioSlice, 
                                   kAudioUnitScope_Global, 
                                   0, 
                                   slice, 
                                   sizeof(ScheduledAudioSlice));	
        require(err == noErr, bail);	
	}

bail:
	if (err)
		numFrames = 0;
	*ioNumSamples = numFrames;
	return err;
}

// Fill and schedule the audio slice buffers.
// Return the number of slices free once we're done.
static UInt32 PreviewBufferScheduleSlices(ScheduledAudioSlice *sliceList,
                                            UInt32 numSlices,
                                            MovieAudioExtractionRef extraction,
                                            AudioStreamBasicDescription asbd,
                                            Float64 * ioSampleTimeStamp,
                                            SInt64* ioSamplesRemaining,
                                            Boolean * outExtractionComplete)
{
	UInt32      sliceNumber;
	UInt32      numSlicesFree = 0;
	OSStatus	err;

	// Iterate through our slice list.
	// For each completed (or never used) slice, fill and schedule it.
	for (sliceNumber = 0; sliceNumber < numSlices ; sliceNumber++)
	{
		if (sliceList[sliceNumber].mFlags & kScheduledAudioSliceFlag_Complete)
		{
			// Count this slice as free until we fill it up
			numSlicesFree++;

			// If there are any samples left to extract...		
			if (*ioSamplesRemaining == 0)
				*outExtractionComplete = true;
			if (!*outExtractionComplete)
			{
				// We will read numSamplesThisSlice number of samples
				SInt64 numSamplesThisSlice = *ioSamplesRemaining;
				if ((numSamplesThisSlice > kMaxExtractionPacketCount) || (numSamplesThisSlice == -1))
					numSamplesThisSlice = kMaxExtractionPacketCount;

				sliceList[sliceNumber].mTimeStamp.mSampleTime = *ioSampleTimeStamp;
				err = ExtractSliceAndScheduleToPlay(extraction,
                                                    asbd, 
                                                    mPlayerUnit,
                                                    &sliceList[sliceNumber], 
                                                    &numSamplesThisSlice,
                                                    outExtractionComplete);
				if (!err && (numSamplesThisSlice > 0)) 
                {
					numSlicesFree--;
				} 
               else 
               {
					sliceList[sliceNumber].mFlags = kScheduledAudioSliceFlag_Complete;
                }
				// Whether there was an error or not, numSamplesThisSlice is valid
				*ioSampleTimeStamp += numSamplesThisSlice;
				if (*ioSamplesRemaining != -1)
					*ioSamplesRemaining -= numSamplesThisSlice;
			}
		}
	}
	return (numSlicesFree);
}


// Allocate and initialize audio slice buffers for the AUScheduledSoundPlayer
static OSStatus PreviewBufferAllocate(ScheduledAudioSlice               *sliceList,
                                        UInt32                          numSlices,
                                        AudioStreamBasicDescription     asbd,
                                        void                            *condLock)
{
	UInt32		sliceNumber;
	OSStatus	err = noErr;

	bzero(sliceList, numSlices * sizeof(ScheduledAudioSlice));

	for (sliceNumber = 0; sliceNumber < numSlices ; sliceNumber++)
	{
		AudioBufferList	*bufList = nil;
		UInt32		bufNumber = 0;
		UInt32		bufSize, listSize;
		UInt32		mallocSize;

		// Accumulate the size of all the memory objects we need for this slice.
		// Then allocate it and parcel it out.
		// Make sure all sizes are rounded up to Altivec boundaries;
		listSize = fieldOffset(AudioBufferList, mBuffers[asbd.mChannelsPerFrame]);
		listSize = ((listSize + 15) / 16) * 16;	// round to Altivec boundary
		mallocSize = listSize;

		bufSize = kMaxExtractionPacketCount * asbd.mBytesPerPacket;
		bufSize = ((bufSize + 15) / 16) * 16; // round to Altivec boundary
		mallocSize += (bufSize * asbd.mChannelsPerFrame);

		bufList = (AudioBufferList*) calloc(1, mallocSize);
		if (bufList == nil) 
        {
			err = memFullErr;
			goto bail;
		}
		bufList->mNumberBuffers = asbd.mChannelsPerFrame;
		for (bufNumber = 0; bufNumber < bufList->mNumberBuffers; bufNumber++)
		{
			bufList->mBuffers[bufNumber].mNumberChannels = 1;
			bufList->mBuffers[bufNumber].mData = (char *) bufList + listSize + (bufNumber * bufSize);
			bufList->mBuffers[bufNumber].mDataByteSize = bufSize;
		}
		sliceList[sliceNumber].mBufferList = bufList;
		sliceList[sliceNumber].mNumberFrames = kMaxExtractionPacketCount;  
		sliceList[sliceNumber].mTimeStamp.mFlags = kAudioTimeStampSampleTimeValid;
		sliceList[sliceNumber].mCompletionProcUserData = condLock; 
		sliceList[sliceNumber].mCompletionProc = (ScheduledAudioSliceCompletionProc) (nil);
		sliceList[sliceNumber].mFlags = kScheduledAudioSliceFlag_Complete;
		sliceList[sliceNumber].mReserved = 0;
	}
bail:
	return (err);
}

// Free the audio slice buffers
static void PreviewBufferDeallocate(ScheduledAudioSlice *sliceList, UInt32 numSlices)
{
	UInt32 sliceNumber;

	for (sliceNumber = 0; sliceNumber < numSlices ; sliceNumber++) 
	{
		if (sliceList[sliceNumber].mBufferList != nil)
        {
			free (sliceList[sliceNumber].mBufferList);
        }
	}
}

static void PreviewCompleted()
{
	// Stop and dispose the playback AudioUnit Graph
	CloseAUGraphPlayer(mAUGraphUnit);
	mAUGraphUnit = nil;

	// Set the Preview button back to its original state
	
	// Send custom event to our window to tell it status
	// of extraction so we can update our UI (button) elements
	SendExtractionStatusEventToWindow(kExtractionStopped);

}

// This callback is scheduled on the main thread.
// In order to keep from locking up the UI, schedules what it can, and then reschedules itself.
static void PreviewOnMainThreadCallBack(ExtractionInfoForCallback *info)
{
	AudioStreamBasicDescription		asbd				= info->asbd;
	AudioChannelLayout              *layout             = info->layout;
	UInt32                          layoutSize			= info->layoutSize;
	TimeRecord                      startTime			= info->startTime;
	SInt64                          samplesRemaining	= info->samplesRemaining;
	Boolean                         discrete			= info->discrete;
	Boolean                         playerUnitStarted	= info->playerUnitStarted;
	Float64                         sampleTimeStamp     = info->sampleTimeStamp;
	ScheduledAudioSlice             *sliceList			= info->sliceList;
	MovieAudioExtractionRef			extraction			= info->extractionSessionRef;

	OSStatus err = noErr;

	//-----------------------------------------------------------

	// Prepare for extraction if this is the first entry

	if (extraction == NULL)	
	{	
		// Open a session, configure audio format 
		err = PrepareMovieForExtraction(mCurrentMovie, 
                                        &extraction, 
                                        discrete,
                                        asbd, 
                                        &layout, 
                                        &layoutSize, 
                                        startTime);						
		if (layout)
		{
			free(layout);
		}
        require(err == noErr, bail);
		
		// Save for future entries
		info->extractionSessionRef = extraction;	
	}
	
	// Prepare audio "slice" list if this is the first time through
	
	if (sliceList == nil)
	{
		sliceList = (ScheduledAudioSlice *) calloc(kNumSlices, sizeof(ScheduledAudioSlice));
		if (sliceList == nil) 
		{
			err = memFullErr;
			goto bail;
		}
		
		err = PreviewBufferAllocate(sliceList,
                                    kNumSlices,
                                    asbd,
                                    (void *)nil);
        require(err == noErr, bail);

		info->sliceList = sliceList;
	}

	//-----------------------------------------------------------
	
	UInt32  numSlicesFree;
	Boolean extractionComplete = false;
	
	if (!mStopPreview)
	{
		// Iterate through our slice list.
		// For each completed (or never used) slice, fill and schedule it.
		numSlicesFree = PreviewBufferScheduleSlices (sliceList,
                                                    kNumSlices,
                                                    extraction,
                                                    asbd,
                                                    &sampleTimeStamp,
                                                    &samplesRemaining,
                                                    &extractionComplete);
	}

	if (!playerUnitStarted)
	{
		// Start the AUGraph player
		err = StartAUGraphPlayer(mAUGraphUnit);
                  require(err == noErr, bail);

		info->playerUnitStarted = true;
	}

	// Save updated state for the next time through this method
	info->samplesRemaining = samplesRemaining;
	info->sampleTimeStamp = sampleTimeStamp;

	// Set the complete flag if we could not queue anything, for what ever reason
	// (ie, we finished playing everything or we never queued anything).
	extractionComplete = (numSlicesFree == kNumSlices);

bail:

	// check for:
	//   - extraction errors
	//   - extraction completed normally
	//   - extraction was stopped by user
	
	if (err || extractionComplete || mStopPreview)
	{
		if (extraction)
		{
			MovieAudioExtractionEnd(extraction);
		}

		StopAUGraphPlayer(mAUGraphUnit);
		if (sliceList != nil) 
		{
			PreviewBufferDeallocate(sliceList,kNumSlices);
			free (sliceList);
		}
		
		if (mRunLoopRef != NULL)
		{
			CFRunLoopSourceInvalidate (mRunLoopRef);
			CFRelease(mRunLoopRef);
			mRunLoopRef = NULL;
		}

		// Call the completion routine to reset the UI
		PreviewCompleted();
	}
	else
	{
		// Reschedule to perform this routine again on the next run loop cycle

		// Use CFRunLoopSource to schedule playing of slices on the main thread

		CFRunLoopSourceContext myContext;

		myContext.version           = 0; 
		myContext.info              = info; 
		myContext.retain			= &MyInfoRetainCallBack; 
		myContext.release			= &MyInfoReleaseCallBack; 
		myContext.copyDescription	= nil; 
		myContext.equal             = nil; 
		myContext.hash              = nil; 
		myContext.schedule          = &MyRunLoopScheduleCallBack;
		myContext.cancel			= nil;
		myContext.perform           = &MyRunLoopPerformCallBack;

		// do CFRunLoopSource initialization if not already done
		
		if (mRunLoopRef == NULL)
		{
			// Create a CFRunLoopSource object.
			
			mRunLoopRef = CFRunLoopSourceCreate (kCFAllocatorDefault, 
                                               0, 
                                               &myContext);
			if (mRunLoopRef)
			{
				// add the CFRunLoopSource object to a run loop
				
				CFRunLoopAddSource (CFRunLoopGetCurrent(), 
								   mRunLoopRef, 
								   kCFRunLoopCommonModes);
			}
		}
		else
		{
			// Signal the CFRunLoopSource, marking it as ready to fire
			
			CFRunLoopSourceSignal (mRunLoopRef);
		}

	}
}

#pragma mark ----- Audio Extraction -----


// Method invoked by the Preview Start button
void DoStartMoviePreview(Movie inMovie, TimeRecord *inMovieStartTime, Boolean inUseEffect)
{	
	// if we are given a different movie let's dispose the current one
	if (mCurrentMovie && (mCurrentMovie != inMovie))
	{
		DisposeMovie(mCurrentMovie);
		mCurrentMovie = NULL;
	}

	mCurrentMovie = inMovie;

	// add a Core Audio Effect Audio Unit to our playback?
	mUseEffectAU = inUseEffect;
	
	// If a movie start time is specified we'll use it, otherwise
	// use a default start time of 0
	mStartTimePtr = nil;
	if (inMovieStartTime != nil)
	{
        memcpy(&mStartTime, inMovieStartTime, sizeof(TimeRecord));
        mStartTimePtr = &mStartTime;
	}
	
	// Extract and play audio (asynchronously)
	StartPreview();
	
	// Send custom event to our window to tell it status
	// of extraction so we can update our UI (button) elements.
	SendExtractionStatusEventToWindow(kExtractionStarted);
}

// Start a preview playback
static void StartPreview()
{
	OSStatus err = noErr;

	// when this is set true, preview stops on the next cycle
	mStopPreview = false;
	
	// initialize our run loop source reference value
	// this is used for our rendering async callbacks
	mRunLoopRef = NULL;

	AudioStreamBasicDescription		asbd;
	AudioChannelLayout              *layout     = nil;
	UInt32                          layoutSize	= 0;
	Boolean                         discrete	= false;
	Float64                         duration	= 0;
	TimeRecord                      startTime;

	// Get default values for extraction layout, layoutsize, 
	// extraction startTime, duration and whether we need to 
	// extract in the "All Channels Discrete" mode
	err = GetMovieExtractionParameters(mCurrentMovie,
                                        &layout, 
                                        &layoutSize,
                                        &asbd,
                                        &startTime, 
                                        &duration, 
                                        &discrete);
	require(err == noErr, bail);
	
	// override default movie start time if another time value was specified
	if (mStartTimePtr != nil)
	{
		// use the specified start time
		memcpy(&startTime, mStartTimePtr, sizeof(struct TimeRecord));
	}

	// Build an AU Graph with a scheduled sound player unit and
	// an output unit for playback -- and insert an effect unit
	// in the chain if specified by the user

	if (mUseEffectAU)
	{
		err = BuildAUGraphPlayerWithEffect(layout, layoutSize, &asbd, &mAUGraphUnit, &mPlayerUnit, &mEffectUnit);
	}
	else
	{
		err = BuildAUGraphPlayer(layout, layoutSize, &asbd, &mAUGraphUnit, &mPlayerUnit);
	}
	require(err == noErr, bail);

	// refCon information that will be sent to the preview thread/timer
	ExtractionInfoForCallback *info = nil;

	info = malloc(sizeof (struct ExtractionInfoForCallback));
	require(info != nil, bail);

	info->asbd                  = asbd;
	info->layout				= layout;				
	info->layoutSize			= layoutSize;
	info->startTime             = startTime;
	info->duration              = duration;
	info->extractionSessionRef	= NULL;
	info->discrete              = false;
	info->sliceList             = nil;
	info->playerUnitStarted		= false;
	info->samplesRemaining		= (duration > 0) ? (duration * asbd.mSampleRate) : -1;
	info->sampleTimeStamp		= 0;

	// for now do all on main thread...
	PreviewOnMainThreadCallBack(info);

bail:

	if (err) 
	{
		if (layout)
		{
			free(layout);
		}
	}
}

// Method invoked by the Preview Stop button
void DoStopMoviePreview()
{
	// Set the stop flag.  The rest of the cleanup will occur in PreviewCompleted.
	mStopPreview = true;
}

// Fill in all the parameters necessary to configure an audio extraction
// from the panel UI and cached values.
static OSStatus GetMovieExtractionParameters(
                                            Movie					theMovie,
                                            AudioChannelLayout		**layout, 
                                            UInt32					*layoutSize,
                                            AudioStreamBasicDescription	*asbd, 
                                            TimeRecord				*startTime,
                                            Float64					*duration, 
                                            Boolean					*allDiscrete)
{
    OSStatus err = noErr;
    AudioChannelLayout *summaryLayout;

	// Set all the parameters to safe values 
	// in the event that this method has to bail on an error
	*layout         = nil;
	*layoutSize		= 0;
	*duration		= 0;		// 0 indicates that the entire movie is to be extracted
	*allDiscrete    = false;	// true means do export of all audio channels without mixing
	
	// Start Time
	// by default we'll start at time 0
	TimeRecord tr;
	tr.value.hi = 0;
	tr.value.lo = 0;
	tr.scale = GetMovieTimeScale(theMovie);
	tr.base = GetMovieTimeBase(theMovie);
	memcpy(startTime, &tr, sizeof(struct TimeRecord));
	
	// Duration
	*duration = GetMovieExtractionDuration(theMovie) - GetMovieTime (theMovie, startTime);

	// ASBD
	err = GetDefaultExtractionLayout(theMovie, nil, &summaryLayout, asbd);

	// Return a copy of the cached extraction layout, which should always be current
	*layoutSize = fieldOffset(AudioChannelLayout, mChannelDescriptions[summaryLayout->mNumberChannelDescriptions]);

	*layout = (AudioChannelLayout*) calloc(1, *layoutSize);
	if (*layout == nil) 
	{
		err = memFullErr;
		goto bail;
	}
	
    memcpy(*layout, summaryLayout, *layoutSize);
    free (summaryLayout);

bail:
	if (err)
	{
		if (*layout)
		{
			free(*layout);
		}
	}
	
	return (err);
}




