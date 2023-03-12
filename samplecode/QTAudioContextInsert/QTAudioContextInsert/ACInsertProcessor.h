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

#import <Cocoa/Cocoa.h>
#import <QuickTime/QuickTime.h>
#import <QTKit/QTKit.h>
#import <AudioToolbox/AudioToolbox.h>
#import <AudioUnit/AudioUnit.h>

#import "QuickTimeAudioUtils.h"

@class MovieDocument;

@interface ACInsertProcessor : NSObject  
{
	MovieDocument					*mMovieDocument;
	AudioUnit						mAudioUnit;
	Boolean							mProcessorIsInitialized;
	
	AudioChannelLayout				mInputChannelLayout;
	AudioChannelLayout				mOutputChannelLayout;
	
	AudioStreamBasicDescription		mInputASBD;
	AudioStreamBasicDescription		mOutputASBD;
	AudioStreamBasicDescription		mMovieSummaryASBD;
	
	UInt32							mMaxFrames;
	Float64							mPlaybackSampleRate;

	AudioBufferList					*mPullBuffer;
	Boolean							mIsBypassed;
	Float64							mLatency;
	Float64							mTailTime;
	
}

- (id)initFromAUComponent:(Component)auComponent;

- (AudioUnit)audioUnit;
- (Boolean)processorIsInitialized;
- (AudioChannelLayoutTag)inputLayoutTag;
- (AudioChannelLayoutTag)outputLayoutTag;
- (void) aUIsBypassable:(BOOL*)isBypassable currentBypassState:(UInt32*)bypassState;

- (void)setAUBypassed:(UInt32)isBypassed;
- (void)setInputLayout:(AudioChannelLayoutTag)layoutTag;
- (void)setOutputLayout:(AudioChannelLayoutTag)layoutTag;

- (BOOL)canDoInputChannels:(int)inNumChannels outputChannels:(int)outNumChannels;
- (BOOL)canDoOutputChannels:(int)outNumChannels;

- (void)getRegistrationInformation:(QTAudioContextInsertRegistryInfo *)regInfo;
- (OSStatus)setAUFormatAndLayout;
- (OSStatus)setAURenderCallback;
- (void)setAUParametersToThoseOfAU:(AudioUnit)auToClone;
- (BOOL)IsElementCountWritableForScope:(AudioUnitScope)scope;

- (OSStatus)myAudioUnitInputProc:(AudioUnitRenderActionFlags *)ioActionFlags
                            timestamp:(const AudioTimeStamp *)inTimeStamp
                            bus:(UInt32)inBusNumber 
                            numFrames:(UInt32)inNumberFrames
                            buffer:(AudioBufferList *)ioData;
							
- (OSStatus)myInsertProcessDataCallback:(AudioUnitRenderActionFlags *)ioRenderFlags
							timestamp:(const AudioTimeStamp *)inTimeStamp
							numFrames:(UInt32)inNumberFrames
							inData:(AudioBufferList *)inInputData
							outData:(AudioBufferList *)outOutputData;
							
- (OSStatus)myInsertResetCallback:(Float64)inSampleRate
					maxFrames:(UInt32)inMaxFrames
					outLatency:(Float64*)outLatency
					outTailTime:(Float64*)outTailTime;

@end
