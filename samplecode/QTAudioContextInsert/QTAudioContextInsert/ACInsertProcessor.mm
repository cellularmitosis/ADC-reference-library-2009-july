/*
	File:		ACInsertProcessor.h	

	Abstract:	This file implements the Audio Context Insert processor 
				object which does the actual processing for the insert. It
				is a wrapper around an Audio Unit, and implements the
				insert reset and process data callbacks.
					
	Version:	1.0

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

	Copyright © 2006-2008 Apple Inc. All Rights Reserved.
*/

#import "ACInsertProcessor.h"

/*----------  Callbacks  -------------------------------------------------------------
*/
// AudioUnit Input Supplier Proc
static OSStatus audioUnitInputProc(void *inRefCon, 
                AudioUnitRenderActionFlags *ioActionFlags, 
                const AudioTimeStamp *inTimeStamp, 
                UInt32 inBusNumber, 
                UInt32 inNumberFrames, 
                AudioBufferList *ioData)
{
	ACInsertProcessor *myself = (ACInsertProcessor*)inRefCon;
	return ([myself myAudioUnitInputProc:ioActionFlags
					timestamp:inTimeStamp
					bus:inBusNumber
					numFrames:inNumberFrames
					buffer:ioData]);	
}


// Audio Context Insert Process data callback
static OSStatus insertProcessDataCallback (
        void						*inUserData,
        AudioUnitRenderActionFlags  *ioRenderFlags,
        const AudioTimeStamp     *inTimeStamp,
        UInt32                   inNumberFrames,
        AudioBufferList          *inInputData,
        AudioBufferList		     *outOutputData
)
{
	ACInsertProcessor *myself = (ACInsertProcessor*)inUserData;
	return ([myself myInsertProcessDataCallback:ioRenderFlags
					timestamp:inTimeStamp
					numFrames:inNumberFrames
					inData:inInputData
					outData:outOutputData]);
}


// Audio Context Insert Reset callback
static OSStatus insertResetCallback(
			void			*inUserData,
			Float64			inSampleRate,
			UInt32			inMaxFrames,
			Float64			*outLatency,
			Float64			*outTailTime
								)
{
	ACInsertProcessor *myself = (ACInsertProcessor*)inUserData;
	return ([myself myInsertResetCallback:inSampleRate
					maxFrames:inMaxFrames
					outLatency:outLatency
					outTailTime:outTailTime]);
}

// Audio Context Insert Finalize callback
// This callback is called by the Audio Context when it is being disposed (ie, the MovieAudioContext
// has been reset or the movie was disposed). No more calls for the insert registered with that Audio
// Context will be made. Here's our chance to dispose the Audio Context Processor instance that was
// created for the insert registered with this particular Audio Context.
static OSStatus insertFinalizeCallback(void	*inUserData)
{
	ACInsertProcessor *processor = (ACInsertProcessor *)inUserData;
	[processor release];	// de'allocs the processor, which closes the associated Audio Unit
	return noErr;
}

/*--------------------------------------------------------------------------------------
*/
@implementation ACInsertProcessor

- (id)initFromAUComponent:(Component)auComponent
{
	self = [super init];
	if (self) 
	{
		OpenAComponent(auComponent, &mAudioUnit);
		NSAssert(mAudioUnit != nil, @"initFromAUComponent: Unable to open AU");	
		[self setAURenderCallback];
		mMovieDocument = [[NSDocumentController sharedDocumentController] currentDocument];
		mProcessorIsInitialized = false;
		mMaxFrames = 0;
		mPullBuffer = NULL;
		mIsBypassed = false;
		mLatency = 0.;
		mTailTime = 0.;
	
	}
	return self;
}

- (void) dealloc 
{
	// Close Audio Unit
	if (mAudioUnit) 
	{
		AudioUnitUninitialize(mAudioUnit);
		CloseComponent(mAudioUnit);
	}
	[super dealloc];
}

#pragma mark
#pragma mark ---- Getters ----
- (AudioUnit)audioUnit
{
	return mAudioUnit;
}
- (Boolean)processorIsInitialized
{
	return mProcessorIsInitialized;
}
- (AudioChannelLayoutTag)inputLayoutTag
{
	return mInputChannelLayout.mChannelLayoutTag;
}

- (AudioChannelLayoutTag)outputLayoutTag
{
	return mOutputChannelLayout.mChannelLayoutTag;
}


#pragma mark
#pragma mark ---- Setters ----
// This method is called by the ACInsertManager when
// the bypass state of this insert needs to be changed.
// We change the bypass state of our Audio Unit to the
// new state
- (void) setAUBypassed:(UInt32)isBypassed
{
	OSStatus err = noErr;
	UInt32 bypass = isBypassed;
	
	err = AudioUnitSetProperty (mAudioUnit, 
							kAudioUnitProperty_BypassEffect,
							kAudioUnitScope_Global, 0,
							&bypass, sizeof(bypass));
	if (noErr == err) 
	{
		mIsBypassed = bypass;
	}
}

// This method is called by the ACInsertManager when the
// insert's input layout is changed. We update our 
// cached input layout.
- (void) setInputLayout:(AudioChannelLayoutTag)layoutTag
{
	
	memset(&mInputChannelLayout, 0, sizeof(AudioChannelLayout));
	mInputChannelLayout.mChannelLayoutTag = layoutTag;
}

// This method is called by the ACInsertManager when the
// insert's output layout is changed. We update our 
// cached output layout and then set the format on our
// underlying Audio Unit.
- (void) setOutputLayout:(AudioChannelLayoutTag)layoutTag
{
	OSStatus err = noErr;
	
	memset(&mOutputChannelLayout, 0, sizeof(AudioChannelLayout));
	mOutputChannelLayout.mChannelLayoutTag = layoutTag;

	// Once we have an output layout, we have all the information
	// needed to set the stream format and channel layout on our AU
	err = [self setAUFormatAndLayout];

}

// This method is called by the ACInsertManager in order to determine
// whether this insert is bypassable, and if it is, what the current 
// bypass state is. We answer these questions based on the bypass
// ability and state of our Audio Unit.
- (void) aUIsBypassable:(BOOL*)isBypassable currentBypassState:(UInt32*)bypassState
{
	OSStatus err = noErr;
	UInt32 ioDataSize = sizeof(UInt32);
	Boolean writable;

	err = AudioUnitGetPropertyInfo(mAudioUnit, 
								kAudioUnitProperty_BypassEffect, 
								kAudioUnitScope_Global, 0,
								&ioDataSize, &writable);
	if (isBypassable != NULL) 
	{
		*isBypassable = (err ? NO : (writable ? YES : NO));
		if (*isBypassable && (bypassState != NULL)) 
		{
			err = AudioUnitGetProperty (mAudioUnit,
										 kAudioUnitProperty_BypassEffect, 
										 kAudioUnitScope_Global, 0,
										 bypassState, &ioDataSize);
			if (err) 
			{
				*isBypassable = NO;
			}
			else 
			{
				mIsBypassed = *bypassState;
			}
		}
	}
}

// This following two methods are called by the ACInsertManager in order to determine
// whether a particular output layout / input layout combination is valid for the
// the insert. We answer this question based on our AU's layout restrictions.
- (BOOL) canDoOutputChannels:(int)outNumChannels
{
	UInt32 inNumChannels = AudioChannelLayoutTag_GetNumberOfChannels(mInputChannelLayout.mChannelLayoutTag);
	return ([self canDoInputChannels:inNumChannels outputChannels:outNumChannels]);
}

- (BOOL) canDoInputChannels:(int)inNumChannels outputChannels:(int)outNumChannels
{
	OSStatus err = noErr;
	AUChannelInfo *channelInfo = NULL;
	UInt32 numChanInfo = 0;
	// The default assumption of an audio effect unit
	Boolean* isWritable = 0;
	UInt32	dataSize = 0;
	// Find out if the unit has any channel restrictions
	err = AudioUnitGetPropertyInfo (mAudioUnit,
									kAudioUnitProperty_SupportedNumChannels,
									kAudioUnitScope_Global, 0,
									&dataSize, isWritable); //don't care if this is writable
		
	// if this property is NOT implemented an FX unit
	// is expected to deal with same channel valance in and out
	if (err) 
	{
		if (inNumChannels == outNumChannels) 
		{
			return true;
		}
		else 
		{
			// assume the worst
			return false;
		}
	}
	
	// Allocate
	channelInfo = (AUChannelInfo *) calloc(1, dataSize);
	err = AudioUnitGetProperty(mAudioUnit,
									kAudioUnitProperty_SupportedNumChannels,
									kAudioUnitScope_Global, 0,
									channelInfo, &dataSize);
	if (err) 
	{ 
		return false; 
	}
	numChanInfo = (dataSize / sizeof (AUChannelInfo));

// we've the following cases (combinations) to test here:
/*
>0		An explicit number of channels on either side
0		that side (generally input!) has no elements
-1		wild card:
-1,-1	any num channels as long as same channels on in and out
-1,-2	any num channels channels on in and out - special meaning
-2+ 	indicates total num channs AU can handle 
			- elements configurable to any num channels, 
			- element count in scope must be writable
*/

	// chan layout can contain -1 for either scope (ie. doesn't care)
	// when -1, the unit can handle any number of channels on that scope
	for (unsigned int i = 0; i < numChanInfo; ++i)
	{
			//less than zero on both sides - check for special attributes
		if ((channelInfo[i].inChannels < 0) && (channelInfo[i].outChannels < 0))
		{
				// unit can handle any number of channels in and out PROVIDED
				// they are the same number of channels
			if (channelInfo[i].inChannels == -1 && channelInfo[i].outChannels == -1) 
			{
				if (outNumChannels == inNumChannels) 
				{
					return true;
				}
			}
				// unit can handle any number of channels in and out
			else if ((channelInfo[i].inChannels == -1 && channelInfo[i].outChannels == -2)
					|| (channelInfo[i].inChannels == -2 && channelInfo[i].outChannels == -1)) 
			{
				return true;
			}
				// these are our total num channels matches
				// element count MUST be writable
			else 
			{
				bool outWrite = false; bool inWrite = false;
				outWrite = [self IsElementCountWritableForScope:kAudioUnitScope_Output];
				inWrite = [self IsElementCountWritableForScope:kAudioUnitScope_Input];
				if (inWrite && outWrite) 
				{
					if ((outNumChannels <= abs(channelInfo[i].outChannels))
						&& (inNumChannels <= abs(channelInfo[i].inChannels))) 
					{
						return true;
					}
				}
			}
		}
			
			// special meaning on input, specific num on output
		else if (channelInfo[i].inChannels < 0) 
		{
			if (channelInfo[i].outChannels == outNumChannels) 
			{
					// can do any in channels
				if (channelInfo[i].inChannels == -1) 
				{
					return true;
				} 
					// total chans on input
				else 
				{
					bool inWrite = false;
					inWrite = [self IsElementCountWritableForScope:kAudioUnitScope_Input];
					if (inWrite && (inNumChannels <= abs(channelInfo[i].inChannels))) 
					{
						return true;
					}
				}
			}
		}
		
			// special meaning on output, specific num on input
		else if (channelInfo[i].outChannels < 0) 
		{
			if (channelInfo[i].inChannels == inNumChannels) 
			{
					// can do any out channels
				if (channelInfo[i].outChannels == -1) 
				{
					return true;
				} 
					// total chans on output
				else 
				{
					bool outWrite = false;
					outWrite = [self IsElementCountWritableForScope:kAudioUnitScope_Output];
					if (outWrite && (outNumChannels <= abs(channelInfo[i].outChannels))) 
					{
						return true;
					}
				}
			}
		}

			// both chans in struct >= 0 - thus has to explicitly match
		else if ((channelInfo[i].inChannels == inNumChannels) && (channelInfo[i].outChannels == outNumChannels)) 
		{
			return true;
		} 
		
	}
	return false;
}

// This is a helper method called by the canDoInputChannels:: method
// to determine whether a particular element count is settable for 
// the given scope
- (BOOL) IsElementCountWritableForScope:(AudioUnitScope)scope
{
	OSStatus err = noErr;
	Boolean isWritable = false;
	UInt32 outDataSize = 0;
	err = AudioUnitGetPropertyInfo (mAudioUnit, kAudioUnitProperty_ElementCount, scope, 0, &outDataSize, &isWritable);
	if (err ) 
	{
		return NO;
	} 
	else
	{
		return ( isWritable ? true : false );
	}
}

// This method is called by the ACInsertManager when it is cloning processors
// This method sets the parameter values of the current (new) AU instance to those
// of the original AU instance
- (void) setAUParametersToThoseOfAU:(AudioUnit)auToClone
{
	OSStatus err = noErr;
	UInt32 size = 0;
	Boolean writable = false;
	AudioUnitParameterID *parameterArray = NULL;

	// Get parameter list
	err = AudioUnitGetPropertyInfo(auToClone,
								kAudioUnitProperty_ParameterList,
								kAudioUnitScope_Global, 0,
								&size, &writable 
								);
								
	if (err == noErr && size > 0) 
	{
		parameterArray = (AudioUnitParameterID*)malloc(size);
		if (parameterArray) 
		{
			err = AudioUnitGetProperty(auToClone,
									kAudioUnitProperty_ParameterList,
									kAudioUnitScope_Global, 0,
									parameterArray, &size);
			if (err == noErr)
			{
				// For each parameter in this list, get the value from the original
				// AU instance and set it on the current AU instance
				for (UInt32 paramNumber = 0; paramNumber < size / sizeof (AudioUnitParameterID) ; paramNumber++)
				{
					Float32 paramValue = 0.;
					err = AudioUnitGetParameter(auToClone,
												parameterArray[paramNumber],
												kAudioUnitScope_Global, 0,
												&paramValue
												);
					if (err == noErr) 
					{
						err = AudioUnitSetParameter(mAudioUnit,
												parameterArray[paramNumber],
												kAudioUnitScope_Global, 0,
												paramValue, 0);
						if (err) 
						{
							NSLog(@"setAUParametersToThoseOfAU: Unable to set parameter %d, value %f", parameterArray[paramNumber], paramValue);
						}
					}							
				}
			
			}
			free(parameterArray);
		}
	}	
}

#pragma mark
#pragma mark ---- AU Configuration ----
// This method is called whenever a new processor instance
// is created. It sets the input data supplier proc on the
// AU instance that it wraps around.
- (OSStatus)setAURenderCallback
{
	OSStatus err = noErr;
	AURenderCallbackStruct cb;
			
	// Set the render callback which
	// supplies input data to the AU
	cb.inputProc = audioUnitInputProc;
	cb.inputProcRefCon = (void*)self;
	err = AudioUnitSetProperty(
							mAudioUnit,
							kAudioUnitProperty_SetRenderCallback,
							kAudioUnitScope_Global, 0,
							&cb, sizeof(cb) );
	if (err) 
	{
		NSLog(@"setAURenderCallback: Unable to set render callback on AU (err=%ld)", err);
	}		
	return err;
}

/// This method is called once the insert's input and output layouts have been set
// It sets the input and output stream formats and channel layouts on the AU instance
// that it wraps around.
- (OSStatus)setAUFormatAndLayout
{
	OSStatus err = noErr;
	
	// The data that QuickTime will provide will always be non-interleaved, Float32, linear PCM
	// with a number of channels equal to the insert's input number of channels. QuickTime
	// will inform us of the sample rate of the data on the reset callback. Until then, assume a
	// default value of 44100 Hz. 
	mPlaybackSampleRate = 44100.0;
	mInputASBD.mSampleRate = mPlaybackSampleRate; 
	mInputASBD.mFormatID = kAudioFormatLinearPCM;
	mInputASBD.mFormatFlags = kAudioFormatFlagsNativeFloatPacked | kLinearPCMFormatFlagIsNonInterleaved;
	mInputASBD.mFramesPerPacket = 1;
	mInputASBD.mBytesPerFrame = sizeof( Float32 );  
	mInputASBD.mBytesPerPacket = mInputASBD.mBytesPerFrame;
	mInputASBD.mChannelsPerFrame = AudioChannelLayoutTag_GetNumberOfChannels(mInputChannelLayout.mChannelLayoutTag);
	mInputASBD.mBitsPerChannel = sizeof( Float32 ) * 8;
	
	memcpy(&mOutputASBD, &mInputASBD, sizeof(AudioStreamBasicDescription));
	mOutputASBD.mChannelsPerFrame = AudioChannelLayoutTag_GetNumberOfChannels(mOutputChannelLayout.mChannelLayoutTag);	
	
	// [1] Uninitialize the Audio Unit
	err = AudioUnitUninitialize(mAudioUnit);
	if (err) 
	{
		NSLog(@"setAUFormatAndLayout: Unable to uninitialize AU (err=%ld)", err);
		goto bail;
	}
	
	// [2.1] Set input stream format
	err = AudioUnitSetProperty(
			mAudioUnit,
			kAudioUnitProperty_StreamFormat,
			kAudioUnitScope_Input,
			0, &mInputASBD, sizeof(mInputASBD));
			
	if (err) 
	{
		NSLog(@"setAUFormatAndLayout: Unable to set input stream format (err=%ld)", err);
	}
	// [2.2] Set input channel layout
	if (err == noErr) 
	{
		UInt32 dataSizeDontCare;
		Boolean isWritable = false;
		
		err = AudioUnitGetPropertyInfo(
			mAudioUnit,
			kAudioUnitProperty_AudioChannelLayout,
			kAudioUnitScope_Input, 0,
			&dataSizeDontCare, &isWritable
		);
		if (err || isWritable == false)
		{
			// Not the end of the world if this AU doesn't
			// support setting of channel layouts.
			err = noErr;
		} 
		else 
		{
			err = AudioUnitSetProperty(
				mAudioUnit,
				kAudioUnitProperty_AudioChannelLayout,
				kAudioUnitScope_Input,
				0, &mInputChannelLayout, sizeof(AudioChannelLayout));
			if (err) 
			{
				NSLog(@"setAUFormatAndLayout: Unable to set input layout (err=%ld)", err);
			}	
		}
	}	
		
	// [3.1] Set output stream format 	
	err = AudioUnitSetProperty(
			mAudioUnit,
			kAudioUnitProperty_StreamFormat, 
			kAudioUnitScope_Output,
			0, &mOutputASBD, sizeof(mOutputASBD));
	if (err) 
	{
		NSLog(@"setAUFormatAndLayout: Unable to set output stream format (err=%ld)", err);
	}
	// [3.2] Set output channel layout
	if (err == noErr) 
	{
		UInt32 dataSizeDontCare;
		Boolean isWritable = false;
		
		err = AudioUnitGetPropertyInfo(
			mAudioUnit,
			kAudioUnitProperty_AudioChannelLayout,
			kAudioUnitScope_Input, 0,
			&dataSizeDontCare, &isWritable
		);
		if (err || isWritable == false) 
		{
			// Not the end of the world if this AU doesn't
			// support setting of channel layouts.
			err = noErr;
		} 
		else 
		{
			err = AudioUnitSetProperty(
				mAudioUnit,
				kAudioUnitProperty_AudioChannelLayout,
				kAudioUnitScope_Output,
				0, &mOutputChannelLayout, sizeof(AudioChannelLayout));
			if (err) 
			{
				NSLog(@"setAUFormatAndLayout: Unable to set output layout (err=%ld)", err);
			}
		}		
	}
	
	// [4] Initialize the AU, we're ready to start running		
	err = AudioUnitInitialize(mAudioUnit);
	if (err) 
	{
		NSLog(@"setAUFormatAndLayout: Unable to initialize AU (err=%ld)", err);
	}
	else
	{
		mProcessorIsInitialized = true;
	}
	
bail:	
	return err;
}

// This method is called by the ACInsertManger to get information about the insert's
// input and output layouts and the addresses of the reset and process data callbacks
// Fill in the necessary information here.
-(void) getRegistrationInformation:(QTAudioContextInsertRegistryInfo *)registryInfo
{
	registryInfo->userData = (void*)self;
	registryInfo->inputChannelLayoutSize = sizeof(mInputChannelLayout); 
	registryInfo->inputChannelLayout = &mInputChannelLayout; 
	registryInfo->outputChannelLayoutSize = sizeof(mOutputChannelLayout); 
	registryInfo->outputChannelLayout = &mOutputChannelLayout; 
	registryInfo->processDataCallback = insertProcessDataCallback;
	registryInfo->resetCallback = insertResetCallback;
	registryInfo->finalizeCallback = insertFinalizeCallback;
}

/*-------------------------------------------------------------------------------------------------------------------------------------------------------------
*/
#pragma mark
#pragma mark ---- Callbacks ----

// This is the input data supplier proc, called when the AU we 
// wrap around needs input data to process.
- (OSStatus)myAudioUnitInputProc:(AudioUnitRenderActionFlags *)ioActionFlags
                            timestamp:(const AudioTimeStamp *)inTimeStamp
                            bus:(UInt32)inBusNumber 
                            numFrames:(UInt32)inNumberFrames
                            buffer:(AudioBufferList *)ioData
{

#pragma unused (ioActionFlags, inTimeStamp, inBusNumber, inNumberFrames)

	OSStatus err = noErr;

	// Ensure that the buffer we're about to provide is compatible
	// with what the Audio Unit is expecting
	if (mPullBuffer && (mPullBuffer->mNumberBuffers == ioData->mNumberBuffers))
	{            
		for (UInt32 i = 0; i < ioData->mNumberBuffers; i++)
		{
			ioData->mBuffers[i].mData = mPullBuffer->mBuffers[i].mData;
			ioData->mBuffers[i].mDataByteSize = mPullBuffer->mBuffers[i].mDataByteSize;
		}
	} 
	else 
	{
		NSLog(@"myAudioUnitInputProc: NULL input bufferlist or incompatible output bufferlist");
	}
	
	return err;	
}

// This is the process data callback called by the Audio Context for each buffer of audio it renders.
// The source data is provided to us in inInputData, we process the data using our AudioUnit and hand
// it back to the Audio Context in outOutputData.  
- (OSStatus)myInsertProcessDataCallback:(AudioUnitRenderActionFlags *)ioRenderFlags
							timestamp:(const AudioTimeStamp *)inTimeStamp
							numFrames:(UInt32)inNumberFrames
							inData:(AudioBufferList *)inInputData
							outData:(AudioBufferList *)outOutputData
{
	OSStatus err = noErr;
	
	// Store a pointer to the source data so our input
	// proc callback can hand it to the AU 
	mPullBuffer = (AudioBufferList*)inInputData;

	// Pull on our AU
	err = AudioUnitRender(mAudioUnit, ioRenderFlags, inTimeStamp, 0/*output bus*/, inNumberFrames, outOutputData) ;
	
	if (err) 
	{
		static BOOL printedErr = false;
		if (!printedErr) 
		{
			NSLog(@"myInsertProcessDataCallback(%@) at %f: AudioUnitRender (err=%ld)", self, inTimeStamp->mSampleTime / mPlaybackSampleRate, err);
			printedErr = true;
		} 
	}
	
	return err;
}

// This is the reset callback called by the Audio Context to initialize for rendering and whenever there
// is an interruption in the render chain (eg. playback jumps to a new point or changes direction). We 
// should reset any state that we're maintaining. We are informed about the sample rate and the maximum 
// number of frames that we will be asked to process on any single process data callback. We get to report
// our processing latency and tail times.
- (OSStatus) myInsertResetCallback:(Float64)inSampleRate
					maxFrames:(UInt32)inMaxFrames
					outLatency:(Float64*)outLatency
					outTailTime:(Float64*)outTailTime
{	
	OSStatus err = noErr;
	OSStatus tempErr = noErr;
	UInt32 size;

	if (mPlaybackSampleRate == inSampleRate && mMaxFrames == inMaxFrames)
	{
		// Neither the sample rate nor max render frames have changed
		// Just reset the Audio Unit	
		err = AudioUnitReset(mAudioUnit, kAudioUnitScope_Global, 0);
		if (err) 
		{
			NSLog(@"myInsertResetCallback(%@): Unable to reset AU (err=%ld)", self, err);
			goto bail;
		} 
	} 
	else 
	{
		// [1] Uninitialize the Audio Unit
		err = AudioUnitUninitialize(mAudioUnit);
		if (err) 
		{
			NSLog(@"myInsertResetCallback(%@): Unable to uninitialize AU (err=%ld)", self, err);
			goto bail;
		}
		if (mPlaybackSampleRate != inSampleRate) 
		{
			// Playback samplerate has changed 
			// Set the new rate on the AU
			mPlaybackSampleRate = inSampleRate;
			mInputASBD.mSampleRate = mPlaybackSampleRate;
			err = AudioUnitSetProperty(
					mAudioUnit,
					kAudioUnitProperty_StreamFormat,
					kAudioUnitScope_Input,
					0, &mInputASBD, sizeof(mInputASBD));
			if (err) 
			{
				NSLog(@"myInsertResetCallback(%@): Unable to set input stream format on AU (err=%ld)", self, err);
			} 
			mOutputASBD.mSampleRate = mPlaybackSampleRate;
			err = AudioUnitSetProperty(
					mAudioUnit,
					kAudioUnitProperty_StreamFormat, 
					kAudioUnitScope_Output,
					0, &mOutputASBD, sizeof(mOutputASBD));
				
			if (err) 
			{
				NSLog(@"myInsertResetCallback(%@): Unable to set output stream format on AU (err=%ld)", self, err);
			}
			
		}
		// Max render frames have changed
		if (mMaxFrames != inMaxFrames) 
		{
			mMaxFrames = inMaxFrames;
			err = AudioUnitSetProperty(
					mAudioUnit, 
					kAudioUnitProperty_MaximumFramesPerSlice, 
					kAudioUnitScope_Global,
					0, &mMaxFrames, sizeof(mMaxFrames));
			if (err) 
			{
				NSLog(@"myInsertResetCallback(%@): Unable to set max render frames on AU (err=%ld)", self, err);
			} 
		}
		tempErr = AudioUnitInitialize(mAudioUnit);
		if (tempErr) 
		{
			NSLog(@"myInsertResetCallback(%@): Unable to initialize AU");
			err = tempErr;
			goto bail;
		} 
	} 

	// Get the audio unit's latency
	mLatency = 0.;
	size = sizeof(mLatency);
	err = AudioUnitGetProperty (mAudioUnit, 
								kAudioUnitProperty_Latency, 
								kAudioUnitScope_Global, 0, 
								&mLatency, &size);
	if (err) 
	{
		NSLog(@"myInsertResetCallback(%@): Unable to get AU latency (err=%ld)", err);
	}
	
	// Get the audio unit's tailtime
	mTailTime = 0.;
	size = sizeof(mTailTime);
	err = AudioUnitGetProperty(mAudioUnit,
							   kAudioUnitProperty_TailTime,
							   kAudioUnitScope_Global,0,
							   &mTailTime, &size);
	if (err) 
	{
		NSLog(@"myInsertResetCallback(%@): Unable to get AU tail time (err=%ld)", err);
	}
	
bail:
	if (noErr == err) 
	{
		if (outLatency) 
		{
			*outLatency = mLatency;
		}
		if (outTailTime)
		{
			*outTailTime = mTailTime;
		}	
	}
	return err;
}

@end