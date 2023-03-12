/*
	File:		CoreAudioPlayback.c
 
	Abstract:	Utility functions for building a simple AUGraph
				 to play buffers of audio data.
 
	Version:	1.0
				Originally introduced at WWDC 2005 at Session 201:
				"Harnessing the Audio Capabilities of QuickTime 7" 

	Disclaimer: IMPORTANT:  This Apple software is supplied to you by
	Apple Computer, Inc. ("Apple") in consideration of your agreement to the
	following terms, and your use, installation, modification or
	redistribution of this Apple software constitutes acceptance of these
	terms.  If you do not agree with these terms, please do not use,
	install, modify or redistribute this Apple software.

	In consideration of your agreement to abide by the following terms, and
	subject to these terms, Apple grants you a personal, non-exclusive
	license, under AppleÕs copyrights in this original Apple software (the
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

	Copyright © 2005-2008 Apple Inc. All Rights Reserved.

*/


#include "CoreAudioPlayback.h"

// Return a ComponentDescription for a default output device AU
static void ConfigureOutputDescription(ComponentDescription *inOutputDesc)
{
	inOutputDesc->componentType = kAudioUnitType_Output;
	inOutputDesc->componentSubType = kAudioUnitSubType_DefaultOutput;
	inOutputDesc->componentManufacturer = kAudioUnitManufacturer_Apple;
	inOutputDesc->componentFlags = 0;
	inOutputDesc->componentFlagsMask = 0;
}

// Set the stream format on the output device AU.
// This only needs to set the input scope, since the output describes the audio hw settings.
static OSStatus SetOutputUnitStreamFormat(AudioUnit outputUnit, AudioStreamBasicDescription *asbd)
{
	return (AudioUnitSetProperty(outputUnit, kAudioUnitProperty_StreamFormat,
								 kAudioUnitScope_Input, 0, asbd, sizeof(AudioStreamBasicDescription)));
}

// Set the channel layout on the output device AU.
// This only needs to set the input scope, since the output describes the audio hw settings.
// Setting the channel layout here helps route channels properly, but note that the OutputAU
// does not do channel mixdown.
static OSStatus SetOutputUnitChannelLayout(AudioUnit outputUnit, QTPropertyValuePtr layoutProperty, UInt32 size)
{
	return (AudioUnitSetProperty(outputUnit, kAudioUnitProperty_AudioChannelLayout,
								 kAudioUnitScope_Input,0, layoutProperty, size));
}

// Return a ComponentDescription for a ScheduledSoundPlayer AU
static void ConfigureScheduledPlayerDescription(ComponentDescription *inPlayerDesc)
{
	inPlayerDesc->componentType = kAudioUnitType_Generator;
	inPlayerDesc->componentSubType = kAudioUnitSubType_ScheduledSoundPlayer;
	inPlayerDesc->componentManufacturer = kAudioUnitManufacturer_Apple;
	inPlayerDesc->componentFlags = 0;
	inPlayerDesc->componentFlagsMask = 0;
}	

// Set the stream format on the ScheduledSoundPlayer AU.
// This only needs to set the output scope, since the unit doesn't have an input scope.
static OSStatus SetPlayerUnitStreamFormat(AudioUnit playerUnit, AudioStreamBasicDescription *asbd)
{
	return (AudioUnitSetProperty(playerUnit, kAudioUnitProperty_StreamFormat,
								 kAudioUnitScope_Output, 0, asbd, sizeof(AudioStreamBasicDescription)));
}


// Build a simple AUGraph containing a Default Output AU sourced by a ScheduledSoundPlayer AU.
// This allows us to schedule (from any thread) buffers of audio data to play automatically.
OSStatus BuildAUGraphPlayer(AudioChannelLayout *layout, UInt32 layoutSize,
							AudioStreamBasicDescription *asbd,
							AUGraphPlayerRef *graphUnit, AudioUnit *thePlayerUnit)
{
	OSStatus err = noErr;

	// Create a new AUGraph
	*graphUnit = nil;
	err = NewAUGraph(graphUnit);
	if (err) 
	{
		goto bail;	
	}
	// Add an output unit, and a scheduled sound player unit
	// Configure the two units.  
	ComponentDescription	output_desc, player_desc;
	ConfigureOutputDescription(&output_desc);
	ConfigureScheduledPlayerDescription(&player_desc);
	
	// Create new nodes inside our AUGraph for the output unit
	// and the AUScheduledSoundPlayer unit.
	AUNode outputNode, playerNode;
	err = AUGraphAddNode(*graphUnit, &output_desc, &outputNode);
	if (err) 		
	{
		goto bail;
	}	
	err = AUGraphAddNode(*graphUnit, &player_desc, &playerNode);
	if (err) 
	{
		goto bail;
	}
	// Connect the two graph nodes together and specify the way inputs are ordered.
	// The player node will route its output to the output node
	// connect the player to the output unit. 
	err = AUGraphConnectNodeInput(*graphUnit, playerNode, 0, outputNode, 0);
	if (err) 
	{
		goto bail;
	}								  
	// Open the graph: Instantiates every Audio Unit in the graph.
	err = AUGraphOpen(*graphUnit);
	if (err) 
	{
		goto bail;	
	}
	// Get the output unit & the player unit from their respective nodes	
	AudioUnit theOutputUnit;
	err = AUGraphNodeInfo(*graphUnit, outputNode, NULL, &theOutputUnit);
	if (err) 
	{
		goto bail;
	}	
	err = AUGraphNodeInfo(*graphUnit, playerNode, NULL, thePlayerUnit);
	if (err) 
	{
		goto bail;
	}
	// Configure stream formats of the output and player to the specified format
	err = SetOutputUnitStreamFormat(theOutputUnit, asbd);
	if (err) 
	{
		goto bail;
	}	
	err = SetPlayerUnitStreamFormat(*thePlayerUnit, asbd);
	if (err) 
	{
		goto bail;
	}
	// Get the extraction channel layout and map the output device
	err = SetOutputUnitChannelLayout(theOutputUnit, layout, layoutSize);
	if (err) 
	{
		goto bail;
	}									
	// Initialize the graph
	err = AUGraphInitialize(*graphUnit);
	if (err) 
	{
		goto bail;
	}	
bail:
	if (err && (*graphUnit != nil))
	{
		AUGraphClose(*graphUnit);
	}	
	return (err);		
}

// Start the player graph running.
OSStatus StartAUGraphPlayer(AUGraphPlayerRef graphUnit)
{
	OSStatus	err = noErr;
	UInt32		numNodes;
	AudioUnit	playerUnit = nil;

	// Start the AUGraph
	err = AUGraphStart(graphUnit);
	if (err)
	{
		goto bail;
	}
	// Locate the AUScheduledSoundPlayer that we need to start.
	// It was added last, so its node number is the node count.
	err = AUGraphGetNodeCount(graphUnit, &numNodes);
	if (!err)
	{
		err = AUGraphNodeInfo(graphUnit, numNodes, NULL, &playerUnit);
	}	
	if (err || (playerUnit == nil))
	{
		goto bail;
	}
	// Start playback of the scheduledPlayer unit by setting its start time.
	AudioTimeStamp theTimeStamp = {0};

	theTimeStamp.mFlags = kAudioTimeStampSampleTimeValid;
	theTimeStamp.mSampleTime = -1.;		// start now
	err = AudioUnitSetProperty(playerUnit,
				kAudioUnitProperty_ScheduleStartTimeStamp, kAudioUnitScope_Global, 0,
				&theTimeStamp, sizeof(theTimeStamp));

bail:
	return (err);
}

void StopAUGraphPlayer(AUGraphPlayerRef graphUnit)
{
	if (graphUnit != nil) 
	{	
		(void) AUGraphStop(graphUnit);
		(void) AUGraphUninitialize(graphUnit);
	}
}

void CloseAUGraphPlayer(AUGraphPlayerRef graphUnit)
{
	if (graphUnit != nil) 
	{	
		StopAUGraphPlayer(graphUnit);
		(void) AUGraphClose(graphUnit);
	}
}

