/*
	File:		myDocument.m
	
	Description:Implementation file for the document class for the CASoundLab2 sample.
                CASoundLab2 is a Cocoa sample that demonstrates low-level access to Core Audio at the Harware Abstraction Layer (HAL).  CASoundLab2 implements a very simple tone generator capable of sine, square, and sawtooth waveforms of varying amplitudes and phases.
                
                Note:  There are more efficient ways to implement multiple waveforms using the AudioUnit framework.  Watch for a new version of this sample coming soon.  Also, this sample makes poor use of the Cocoa multiple-document architecture.  You can open multiple windows but they all use the same waveform parameters.  Not too exciting.
                                
	Author:		DH

	Copyright: 	© Copyright 2001 Apple Computer, Inc. All rights reserved.
	
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
	
		6/22/2001		DH		First release of WWDC 2001 sample code

*/


#import "MyDocument.h"

#import <CoreAudio/CoreAudio.h>

struct waveformGlobals
{
    struct
    {
        int		waveform;
        float	freq;
        float	phase;
        float	amp;
    } wave1;
    
    struct
    {
        int		waveform;
        float	freq;
        float	phase;
        float	amp;
    } wave2;
    
    struct
    {
        int		waveform;
        float	freq;
        float	phase;
        float	amp;
    } wave3;
    
    struct
    {
        int		waveform;
        float	freq;
        float	phase;
        float	amp;
    } wave4;
    
} gWaveformGlobals;


static AudioDeviceID gOutputDeviceID;

float generateWaveform( int waveform, float freq, float phase, float amp, AudioBufferList* outOutputData, float lastTime );


OSStatus wave1IOProc (	AudioDeviceID			inDevice,
						const AudioTimeStamp*	inNow,
						const AudioBufferList*	inInputData,
						const AudioTimeStamp*	inInputTime,
						AudioBufferList*		outOutputData, 
						const AudioTimeStamp*	inOutputTime,
						void*					inClientData);

OSStatus wave2IOProc (	AudioDeviceID			inDevice,
						const AudioTimeStamp*	inNow,
						const AudioBufferList*	inInputData,
						const AudioTimeStamp*	inInputTime,
						AudioBufferList*		outOutputData, 
						const AudioTimeStamp*	inOutputTime,
						void*					inClientData);

OSStatus wave3IOProc (	AudioDeviceID			inDevice,
						const AudioTimeStamp*	inNow,
						const AudioBufferList*	inInputData,
						const AudioTimeStamp*	inInputTime,
						AudioBufferList*		outOutputData, 
						const AudioTimeStamp*	inOutputTime,
						void*					inClientData);

OSStatus wave4IOProc (	AudioDeviceID			inDevice,
						const AudioTimeStamp*	inNow,
						const AudioBufferList*	inInputData,
						const AudioTimeStamp*	inInputTime,
						AudioBufferList*		outOutputData, 
						const AudioTimeStamp*	inOutputTime,
						void*					inClientData);

OSStatus wave1IOProc (	AudioDeviceID			inDevice,
						const AudioTimeStamp*	inNow,
						const AudioBufferList*	inInputData,
						const AudioTimeStamp*	inInputTime,
						AudioBufferList*		outOutputData, 
						const AudioTimeStamp*	inOutputTime,
						void*					inClientData)
{
    struct waveformGlobals* globalsPtr = &gWaveformGlobals;
    int waveformNumber = (int)inClientData;
	int waveform;
	float freq;
	float phase;
	float amp;
    static float lastTime = 0.0;
    
    switch ( waveformNumber )
    {
        case 1:
            {
                waveform = globalsPtr->wave1.waveform;
                freq = globalsPtr->wave1.freq;
                phase = globalsPtr->wave1.phase;
                amp = globalsPtr->wave1.amp;
            }
            break;

        case 2:
            {
                waveform = globalsPtr->wave2.waveform;
                freq = globalsPtr->wave2.freq;
                phase = globalsPtr->wave2.phase;
                amp = globalsPtr->wave2.amp;
            }
            break;

        case 3:
            {
                waveform = globalsPtr->wave3.waveform;
                freq = globalsPtr->wave3.freq;
                phase = globalsPtr->wave3.phase;
                amp = globalsPtr->wave3.amp;
            }
            break;

        case 4:
            {
                waveform = globalsPtr->wave4.waveform;
                freq = globalsPtr->wave4.freq;
                phase = globalsPtr->wave4.phase;
                amp = globalsPtr->wave4.amp;
            }
            break;
    }
    
    //	Generate waveform
    lastTime = generateWaveform( waveform, freq, phase, amp, outOutputData, lastTime );

    return kAudioHardwareNoError;
}


OSStatus wave2IOProc (	AudioDeviceID			inDevice,
						const AudioTimeStamp*	inNow,
						const AudioBufferList*	inInputData,
						const AudioTimeStamp*	inInputTime,
						AudioBufferList*		outOutputData, 
						const AudioTimeStamp*	inOutputTime,
						void*					inClientData)
{
    struct waveformGlobals* globalsPtr = &gWaveformGlobals;
    int waveformNumber = (int)inClientData;
	int waveform;
	float freq;
	float phase;
	float amp;
    static float lastTime = 0.0;
    
    switch ( waveformNumber )
    {
        case 1:
            {
                waveform = globalsPtr->wave1.waveform;
                freq = globalsPtr->wave1.freq;
                phase = globalsPtr->wave1.phase;
                amp = globalsPtr->wave1.amp;
            }
            break;

        case 2:
            {
                waveform = globalsPtr->wave2.waveform;
                freq = globalsPtr->wave2.freq;
                phase = globalsPtr->wave2.phase;
                amp = globalsPtr->wave2.amp;
            }
            break;

        case 3:
            {
                waveform = globalsPtr->wave3.waveform;
                freq = globalsPtr->wave3.freq;
                phase = globalsPtr->wave3.phase;
                amp = globalsPtr->wave3.amp;
            }
            break;

        case 4:
            {
                waveform = globalsPtr->wave4.waveform;
                freq = globalsPtr->wave4.freq;
                phase = globalsPtr->wave4.phase;
                amp = globalsPtr->wave4.amp;
            }
            break;
    }
    
    //	Generate waveform
    lastTime = generateWaveform( waveform, freq, phase, amp, outOutputData, lastTime );

    return kAudioHardwareNoError;
}


OSStatus wave3IOProc (	AudioDeviceID			inDevice,
						const AudioTimeStamp*	inNow,
						const AudioBufferList*	inInputData,
						const AudioTimeStamp*	inInputTime,
						AudioBufferList*		outOutputData, 
						const AudioTimeStamp*	inOutputTime,
						void*					inClientData)
{
    struct waveformGlobals* globalsPtr = &gWaveformGlobals;
    int waveformNumber = (int)inClientData;
	int waveform;
	float freq;
	float phase;
	float amp;
    static float lastTime = 0.0;
   
    switch ( waveformNumber )
    {
        case 1:
            {
                waveform = globalsPtr->wave1.waveform;
                freq = globalsPtr->wave1.freq;
                phase = globalsPtr->wave1.phase;
                amp = globalsPtr->wave1.amp;
            }
            break;

        case 2:
            {
                waveform = globalsPtr->wave2.waveform;
                freq = globalsPtr->wave2.freq;
                phase = globalsPtr->wave2.phase;
                amp = globalsPtr->wave2.amp;
            }
            break;

        case 3:
            {
                waveform = globalsPtr->wave3.waveform;
                freq = globalsPtr->wave3.freq;
                phase = globalsPtr->wave3.phase;
                amp = globalsPtr->wave3.amp;
            }
            break;

        case 4:
            {
                waveform = globalsPtr->wave4.waveform;
                freq = globalsPtr->wave4.freq;
                phase = globalsPtr->wave4.phase;
                amp = globalsPtr->wave4.amp;
            }
            break;
    }
    
    //	Generate waveform
    lastTime = generateWaveform( waveform, freq, phase, amp, outOutputData, lastTime );

    return kAudioHardwareNoError;
}


OSStatus wave4IOProc (	AudioDeviceID			inDevice,
						const AudioTimeStamp*	inNow,
						const AudioBufferList*	inInputData,
						const AudioTimeStamp*	inInputTime,
						AudioBufferList*		outOutputData, 
						const AudioTimeStamp*	inOutputTime,
						void*					inClientData)
{
    struct waveformGlobals* globalsPtr = &gWaveformGlobals;
    int waveformNumber = (int)inClientData;
	int waveform;
	float freq;
	float phase;
	float amp;
    static float lastTime = 0.0;
    
    switch ( waveformNumber )
    {
        case 1:
            {
                waveform = globalsPtr->wave1.waveform;
                freq = globalsPtr->wave1.freq;
                phase = globalsPtr->wave1.phase;
                amp = globalsPtr->wave1.amp;
            }
            break;

        case 2:
            {
                waveform = globalsPtr->wave2.waveform;
                freq = globalsPtr->wave2.freq;
                phase = globalsPtr->wave2.phase;
                amp = globalsPtr->wave2.amp;
            }
            break;

        case 3:
            {
                waveform = globalsPtr->wave3.waveform;
                freq = globalsPtr->wave3.freq;
                phase = globalsPtr->wave3.phase;
                amp = globalsPtr->wave3.amp;
            }
            break;

        case 4:
            {
                waveform = globalsPtr->wave4.waveform;
                freq = globalsPtr->wave4.freq;
                phase = globalsPtr->wave4.phase;
                amp = globalsPtr->wave4.amp;
            }
            break;
    }
    
    //	Generate waveform
    lastTime = generateWaveform( waveform, freq, phase, amp, outOutputData, lastTime );

    return kAudioHardwareNoError;
}


//	Generate waveform
float generateWaveform( int waveform, float freq, float phase, float amp, AudioBufferList* outOutputData, float lastTime )
{
    //	Let's try to get the sine wave right first
    
    /*
        struct AudioBuffer
        {
            UInt32	mNumberChannels;	//	number of interleaved channels in the buffer
            UInt32	mDataByteSize;		//	the size of the buffer pointed to by mData
            void*	mData;				//	the pointer to the buffer
        };
        
        struct AudioBufferList
        {
            UInt32		mNumberBuffers;
            AudioBuffer	mBuffers[1];
        };
     */
    
    //	Gather the data we need
    float* bufPtr  = outOutputData->mBuffers[0].mData;
    UInt32 bufSizeInBytes = outOutputData->mBuffers[0].mDataByteSize;
    UInt32 numChannels = outOutputData->mBuffers[0].mNumberChannels;
    UInt32 bufSizeInSamples = bufSizeInBytes / 4;
    float samplesToSeconds = 1.0 / 44100;//gSampleRate;
    UInt32 bufSizeInFrames = bufSizeInSamples / numChannels;
    
    switch ( waveform )
    {
        case 0:
        {
            UInt32 sampleIndex;
            for ( sampleIndex = 0; sampleIndex < bufSizeInFrames; sampleIndex++ )
            {
                UInt32 channel;
                
                //	Generate the sample
                float sample = amp * sin( ((sampleIndex * samplesToSeconds + lastTime) * freq + phase) * 2 * 3.1416 );
                
                for ( channel = 0; channel < numChannels; channel++ )
                {
                    *bufPtr++ = sample;
                }
            }
            
            lastTime += bufSizeInFrames * samplesToSeconds;
        }
        break;
        
        case 1:			//	Square wave
        {
            UInt32 sampleIndex;
            for ( sampleIndex = 0; sampleIndex < bufSizeInFrames; sampleIndex++ )
            {
                UInt32 channel;
                
                //	Generate the sample
                //	-1 for the first half of the cycle, +1 for the second
                float radians = ((sampleIndex * samplesToSeconds + lastTime) * freq + phase);
                float sample;
                if ( (radians - trunc(radians)) < 0.5 )
                    sample = -amp;
                else
                    sample = +amp;
                
                for ( channel = 0; channel < numChannels; channel++ )
                {
                    *bufPtr++ = sample;
                }
            }
            
            lastTime += bufSizeInFrames * samplesToSeconds;
        }
        break;
        
        case 2:
        {
            UInt32 sampleIndex;
            for ( sampleIndex = 0; sampleIndex < bufSizeInFrames; sampleIndex++ )
            {
                UInt32 channel;
                
                //	Generate the sample
                //	-1 for the first half of the cycle, +1 for the second
                float radians = ((sampleIndex * samplesToSeconds + lastTime) * freq + phase);
                float sample;
                float fracRadians = radians - trunc(radians);
                if ( fracRadians < 0.5 )
                    sample = -amp + 2 * amp * fracRadians;
                else
                    sample = +2 * amp - 2 * amp * fracRadians;
                
                for ( channel = 0; channel < numChannels; channel++ )
                {
                    *bufPtr++ = sample;
                }
            }
            
            lastTime += bufSizeInFrames * samplesToSeconds;
        }
        break;
        
        default:
        {
            NSLog( @"Invalid waveform %d in generateWaveform", waveform );
        }
    };
    
    return lastTime;
}


@implementation MyDocument

- (NSString *)windowNibName
{
    // Override returning the nib file name of the document
    // If you need to use a subclass of NSWindowController or if your document supports multiple NSWindowControllers, you should remove this method and override -makeWindowControllers instead.
    return @"MyDocument";
}

- (void)windowControllerDidLoadNib:(NSWindowController *) aController
{
    [super windowControllerDidLoadNib:aController];
    // Add any code here that need to be executed once the windowController has loaded the document's window.
    
    [[[[self windowControllers] objectAtIndex: 0] window] setTitle: @"CASoundLab"];
    [self showWindows];
    [self startAudio];
}

- (NSData *)dataRepresentationOfType:(NSString *)aType
{
    // Insert code here to write your document from the given data.  You can also choose to override -fileWrapperRepresentationOfType: or -writeToFile:ofType: instead.
    return nil;
}

- (BOOL)loadDataRepresentation:(NSData *)data ofType:(NSString *)aType
{
    // Insert code here to read your document from the given data.  You can also choose to override -loadFileWrapperRepresentation:ofType: or -readFromFile:ofType: instead.
    return YES;
}



- (void)startAudio
{
    #define SAMPLES_PER_BUFFER (8*1024)

    OSStatus status;
    UInt32 propertySize, bufferByteCount;

    NSLog(@"startAudio");
    
    //	First make sure the globals are right
    [self wave1changed: self];
    [self wave2changed: self];
    [self wave3changed: self];
    [self wave4changed: self];
    
    // Get the output device
    propertySize = sizeof(gOutputDeviceID);
    status = AudioHardwareGetProperty(kAudioHardwarePropertyDefaultOutputDevice, &propertySize, &gOutputDeviceID);
    if (status) {
        NSLog(@"AudioHardwareGetProperty returned %d", status);
        return;
    }
	
    if (gOutputDeviceID == kAudioDeviceUnknown) {
        NSLog(@"AudioHardwareGetProperty: outputDeviceID is kAudioDeviceUnknown");
        return;
    }

    // Configure the output device	
    propertySize = sizeof(bufferByteCount);
    bufferByteCount = SAMPLES_PER_BUFFER * sizeof(float);
    status = AudioDeviceSetProperty(gOutputDeviceID, NULL, 0, NO, kAudioDevicePropertyBufferSize, propertySize, &bufferByteCount);
    if (status) {
        NSLog(@"AudioDeviceSetProperty: returned %d when setting kAudioDevicePropertyBufferSize to %d", status, SAMPLES_PER_BUFFER);
        return;
    }


    // Start sound running
    status = AudioDeviceAddIOProc(gOutputDeviceID, wave1IOProc, (void*)1/*(void*)(&gWaveformGlobals)*/);
    status = AudioDeviceAddIOProc(gOutputDeviceID, wave2IOProc, (void*)2/*(void*)(&gWaveformGlobals)*/);
    status = AudioDeviceAddIOProc(gOutputDeviceID, wave3IOProc, (void*)3/*(void*)(&gWaveformGlobals)*/);
    status = AudioDeviceAddIOProc(gOutputDeviceID, wave4IOProc, (void*)4/*(void*)(&gWaveformGlobals)*/);
    if (status) {
        NSLog(@"AudioDeviceAddIOProc: returned %d", status);
        return;
    }

    status = AudioDeviceStart(gOutputDeviceID, wave1IOProc);
    status = AudioDeviceStart(gOutputDeviceID, wave2IOProc);
    status = AudioDeviceStart(gOutputDeviceID, wave3IOProc);
    status = AudioDeviceStart(gOutputDeviceID, wave4IOProc);
    if (status) {
        NSLog(@"AudioDeviceStart: returned %d", status);
        return;
    }
}


- (void)stopAudio
{
    OSStatus err;
    err = AudioDeviceStop( gOutputDeviceID, wave1IOProc );
    err = AudioDeviceStop( gOutputDeviceID, wave2IOProc );
    err = AudioDeviceStop( gOutputDeviceID, wave3IOProc );
    err = AudioDeviceStop( gOutputDeviceID, wave4IOProc );

}


- (void)dealloc
{
    [self stopAudio];
}





//	Waveform 0 = sine
//	Waveform 1 = square
//	Waveform 2 = sawtooth
- (IBAction) wave1changed: (id) sender
{
    //	Get the current values of the controls
    int waveform = [(NSMatrix*)waveform1 selectedRow];
    float freq = [freq1 floatValue];
    float phase = [phase1 floatValue];
    float amp = [amp1 floatValue];
    
    //	Set the text fields
    [freq1text setFloatValue: freq];
    [phase1text setFloatValue: phase];
    [amp1text setFloatValue: amp];
        
    //  Change the sound generation globals
    gWaveformGlobals.wave1.waveform = waveform;
    gWaveformGlobals.wave1.freq = freq;
    gWaveformGlobals.wave1.phase = phase;
    gWaveformGlobals.wave1.amp = amp;
    
    //	Update the waveform display
    [self drawWaveform: waveform withFrequency: freq phase: phase andAmplitude: amp inView: wave1View];
    
}


- (void) drawWaveform: (int)waveform withFrequency: (float)freq phase: (float)phase andAmplitude: (float)amp inView: (NSView*)view
{
    NSRect bounds = [wave1View bounds];
    float xAxis = bounds.origin.y + bounds.size.height / 2;
    float xCoord;
    
    float previousY = xAxis;
    float previousX = bounds.origin.x;
    float samplesToSeconds = 1.0 / 44100;//gSampleRate;
    float scale = bounds.size.height / 2;
    float xScale = bounds.size.width / 100.0;
    
    if ( [view lockFocusIfCanDraw] == YES )
    {
    
        //	Clear the rect
        [[NSColor blackColor] set];
        [NSBezierPath fillRect: bounds];
        
        //	Draw the waveform
        [[NSColor greenColor] set];
        for ( xCoord = bounds.origin.x; xCoord < bounds.origin.x + bounds.size.width; xCoord++ )
        {
            float sample;
            float yCoord;
            
            switch (waveform)
            {
                case 0:
                    {
                        sample = amp * sin( ((xCoord * xScale * samplesToSeconds + 0.0) * freq + phase) * 2 * 3.1416 );
                    }
                    break;
                
                case 1:
                    {
                        float pos = (xCoord * xScale * samplesToSeconds + 0.0) * freq + phase;
                        sample = (pos - trunc( pos )) < 0.5 ? -amp : +amp;
                    }
                    break;
                    
                case 2:
                    {
                        float pos = (xCoord * xScale * samplesToSeconds + 0.0) * freq + phase;
                        float fracPos = pos - trunc(pos);
                        if ( fracPos < 0.5 )
                            sample = -amp + 4 * amp * fracPos;
                        else
                            sample = +3 * amp - 4 * amp * fracPos;
                    }
                    break;
            };
            
            yCoord = xAxis + sample * scale;
            
            [NSBezierPath strokeLineFromPoint: NSMakePoint(previousX, previousY) toPoint: NSMakePoint( xCoord, yCoord )];
            
            previousX = xCoord;
            previousY = yCoord;
        }
    
        [view unlockFocus];
    }
}

- (IBAction) wave2changed: (id) sender
{
    //	Get the current values of the controls
    int waveform = [(NSMatrix*)waveform2 selectedRow];
    float freq = [freq2 floatValue];
    float phase = [phase2 floatValue];
    float amp = [amp2 floatValue];
    
    //	Set the text fields
    [freq2text setFloatValue: freq];
    [phase2text setFloatValue: phase];
    [amp2text setFloatValue: amp];
        
    //  Change the sound generation globals
    gWaveformGlobals.wave2.waveform = waveform;
    gWaveformGlobals.wave2.freq = freq;
    gWaveformGlobals.wave2.phase = phase;
    gWaveformGlobals.wave2.amp = amp;
    
    
    //	Update the waveform display
    [self drawWaveform: waveform withFrequency: freq phase: phase andAmplitude: amp inView: wave2View];
    
}


- (IBAction) wave3changed: (id) sender
{
    //	Get the current values of the controls
    int waveform = [(NSMatrix*)waveform3 selectedRow];
    float freq = [freq3 floatValue];
    float phase = [phase3 floatValue];
    float amp = [amp3 floatValue];
    
    //	Set the text fields
    [freq3text setFloatValue: freq];
    [phase3text setFloatValue: phase];
    [amp3text setFloatValue: amp];
    
    //  Change the sound generation globals
    gWaveformGlobals.wave3.waveform = waveform;
    gWaveformGlobals.wave3.freq = freq;
    gWaveformGlobals.wave3.phase = phase;
    gWaveformGlobals.wave3.amp = amp;
    
    
    //	Update the waveform display
    [self drawWaveform: waveform withFrequency: freq phase: phase andAmplitude: amp inView: wave3View];
    
}


- (IBAction) wave4changed: (id) sender
{
    //	Get the current values of the controls
    int waveform = [(NSMatrix*)waveform4 selectedRow];
    float freq = [freq4 floatValue];
    float phase = [phase4 floatValue];
    float amp = [amp4 floatValue];
    
    //	Set the text fields
    [freq4text setFloatValue: freq];
    [phase4text setFloatValue: phase];
    [amp4text setFloatValue: amp];
    
    //  Change the sound generation globals
    gWaveformGlobals.wave4.waveform = waveform;
    gWaveformGlobals.wave4.freq = freq;
    gWaveformGlobals.wave4.phase = phase;
    gWaveformGlobals.wave4.amp = amp;
    
    
    //	Update the waveform display
    [self drawWaveform: waveform withFrequency: freq phase: phase andAmplitude: amp inView: wave4View];
    
}


@end
