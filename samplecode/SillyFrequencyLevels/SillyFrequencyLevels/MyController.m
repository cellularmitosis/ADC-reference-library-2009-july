/*

File: MyController.m

Author: QuickTime DTS

Change History (most recent first): <1> 11/24/05 initial release

© Copyright 2005 Apple Computer, Inc. All rights reserved.

IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc. ("Apple") in
consideration of your agreement to the following terms, and your use, installation,
modification or redistribution of this Apple software constitutes acceptance of these
terms.  If you do not agree with these terms, please do not use, install, modify or
redistribute this Apple software.

In consideration of your agreement to abide by the following terms, and subject to these
terms, Apple grants you a personal, non-exclusive license, under Apple's copyrights in
this original Apple software (the "Apple Software"), to use, reproduce, modify and
redistribute the Apple Software, with or without modifications, in source and/or binary
forms; provided that if you redistribute the Apple Software in its entirety and without
modifications, you must retain this notice and the following text and disclaimers in all
such redistributions of the Apple Software. Neither the name, trademarks, service marks
or logos of Apple Computer, Inc. may be used to endorse or promote products derived from
the Apple Software without specific prior written permission from Apple.  Except as
expressly stated in this notice, no other rights or licenses, express or implied, are
granted by Apple herein, including but not limited to any patent rights that may be
infringed by your derivative works or by other works in which the Apple Software may be
incorporated.

The Apple Software is provided by Apple on an "AS IS" basis.  APPLE MAKES NO WARRANTIES,
EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED WARRANTIES OF
NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE, REGARDING THE
APPLE SOFTWARE OR ITS USE AND OPERATION ALONE OR IN COMBINATION WITH YOUR PRODUCTS.

IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) ARISING IN ANY WAY OUT OF THE
USE, REPRODUCTION, MODIFICATION AND/OR DISTRIBUTION OF THE APPLE SOFTWARE, HOWEVER
CAUSED AND WHETHER UNDER THEORY OF CONTRACT, TORT (INCLUDING NEGLIGENCE), STRICT
LIABILITY OR OTHERWISE, EVEN IF APPLE HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/

#import "MyController.h"

static UInt32 numberOfBandLevels = 4;       // increase this number for more frequency bands
static UInt32 numberOfChannels = 2;         // for StereoMix - If using DeviceMix, you need to get the channel count of the device.
static UInt8  maxLevelIndicatorValue = 20;  // NSLevelIndicators are set up to have this max value

@implementation MyController

- (void)awakeFromNib
{
    UInt8 j;
    
    [mButton setEnabled:FALSE];
    
    // allocate memory for the QTAudioFrequencyLevels struct and set it up
    // depending on the number of channels and frequency bands you want    
    mFreqResults = malloc(offsetof(QTAudioFrequencyLevels, level[numberOfBandLevels * numberOfChannels]));

    mFreqResults->numChannels = numberOfChannels;
    mFreqResults->numFrequencyBands = numberOfBandLevels;
    
    // create an array and load up the UI elements, each NSLevelIndicator has
    // the appropriate tag added in IB
    mArray = [NSMutableArray array];
    [mArray retain];

	// somewhat cheezoid, but NSLevelIndicator doesn't allow us to create a vertical control
    // so we've created two views containing our controls and we rotate them to vertical
	[mMeterView1 setFrameRotation: 90.0];
    [mMeterView2 setFrameRotation: 90.0];
    
    // add each set of controls to an array we can later interate through
    // one for each channel
  	for (j = 0; j < mFreqResults->numFrequencyBands; j++) {
        id levelIndicator = [mMeterView1 viewWithTag:j];
        
        [levelIndicator setEnabled:FALSE]; // prevent values being changed by clicking on the indicators
        [mArray addObject:levelIndicator];
    }
    
  	for (j = 0; j < mFreqResults->numFrequencyBands; j++) {
        id levelIndicator = [mMeterView2 viewWithTag:j];
        
        [levelIndicator setEnabled:FALSE]; // prevent values being changed by clicking on the indicators
        [mArray addObject:levelIndicator];
    }
}

#pragma mark ---- panel callbacks ----

// movie opening panel
- (void)openPanelDidEnd:(NSOpenPanel *)sheet returnCode:(int)returnCode contextInfo:(void *)contextInfo
{
    if (NSOKButton == returnCode) {
        NSString *theFilename = [[sheet filenames] objectAtIndex:0];
        [sheet close];
    
        [self openMovie:theFilename];
    }
}

#pragma mark ---- timer ----

// timer method to display the frequency levels in the UI
- (void)myTimerFireMethod:(NSTimer*)theTimer
{
	UInt8 i, j;
	NSEnumerator *enumerator = [mArray objectEnumerator]; // get a enumerator for the array of NSLevelIndicator objects
    
    // get the levels from the movie
	OSStatus err = GetMovieAudioFrequencyLevels([mMovie quickTimeMovie], kQTAudioMeter_StereoMix, mFreqResults);
    if (err) {
    	// something went wrong so turn it off
		[mWindow setTitle:[NSString stringWithFormat:@"Error %d", err]];
				
		[mButton setEnabled:FALSE];
        [mButton setState:NSOffState];
        [self toggleFreqLevels:self];
        
        return;
    }
    
    // iterate though the frequency level array and though the UI elements getting
    // and setting the levels appropriately
    for (i = 0; i < mFreqResults->numChannels; i++) {
    	for (j = 0; j < mFreqResults->numFrequencyBands; j++) {
        	// the frequency levels are Float32 values between 0. and 1.
            Float32 value = (mFreqResults->level[(i * mFreqResults->numFrequencyBands) + j]) * maxLevelIndicatorValue;
            
            [[enumerator nextObject] setFloatValue:value];
        }
    }
}

#pragma mark ---- open movie/set up metering ----

// select a file to open
- (IBAction)doOpen:(id)sender
{
	[[NSOpenPanel openPanel] beginSheetForDirectory:nil
                             file:nil
                             types:nil
                             modalForWindow:mWindow
                             modalDelegate:self
                             didEndSelector:@selector(openPanelDidEnd:returnCode:contextInfo:)
                             contextInfo:nil];
}

// called when the button is pressed - turns the level meters on/off by setting up a timer
- (IBAction)toggleFreqLevels:(id)sender
{
    if (NSOnState == [mButton state]) {
    	// turning it on, set up a timer and add it to the run loop
        mTimer = [NSTimer timerWithTimeInterval:1.0/15 target:self selector:@selector(myTimerFireMethod:) userInfo:nil repeats:YES];
        
        [[NSRunLoop currentRunLoop] addTimer:mTimer forMode:(NSString *)kCFRunLoopCommonModes];
    } else {
    	// turning it off invalidate the timer and make sure each level indicator is set to 0
    	NSEnumerator *enumerator = [mArray objectEnumerator];
    	id levelIndicator;
    	
        [mTimer invalidate];
        
        while (levelIndicator = [enumerator nextObject]) {
        	[levelIndicator setFloatValue:0.0];
        }
    }
}

// open the QTMovie and set it in the view replacing the movie that was there
- (void)openMovie:(NSString *)inFile
{
    NSAlert *alert = nil;
    NSError *error = nil;
    
	if (mMovie != nil) {
        [mMovieView pause:self];
		[mMovieView setMovie:nil];
	}
    
    mMovie = [QTMovie movieWithFile:inFile error:&error];
    
    if (nil == error) {

        [mWindow setTitle:[mMovie attributeForKey:QTMovieDisplayNameAttribute]];
    
        [mMovieView setMovie:mMovie];
        [mMovieView setNeedsDisplay:TRUE];
        
        if (TRUE == [[mMovie attributeForKey:QTMovieHasAudioAttribute] boolValue]) {

            // do this once per movie to establish metering
            OSStatus err = SetMovieAudioFrequencyMeteringNumBands([mMovie quickTimeMovie], kQTAudioMeter_StereoMix, &numberOfBandLevels);
            if (err) {
            	// if something went wrong turn it off
            	[mWindow setTitle:[NSString stringWithFormat:@"Error %d", err]];
				
                [mButton setEnabled:FALSE];
                [mButton setState:NSOffState];
                [self toggleFreqLevels:self];
            } else {
            	[mButton setEnabled:TRUE];
            }
        } else {
            alert = [NSAlert alertWithMessageText:@"No Sound Track!"
                             defaultButton:@"OK"
                             alternateButton:nil
                             otherButton:nil
                             informativeTextWithFormat:@"Open some media that contains audio if you're planing to see some frequency levels."];
            
            [alert runModal];
        }
    } else {
        [mButton setEnabled:FALSE];
        
        alert = [NSAlert alertWithError:error];
        
		[alert runModal];
    }
}

#pragma mark ---- application delegates ----

// split when window is closed
- (BOOL)applicationShouldTerminateAfterLastWindowClosed:(NSApplication *)sender
{
    return YES;
}

@end