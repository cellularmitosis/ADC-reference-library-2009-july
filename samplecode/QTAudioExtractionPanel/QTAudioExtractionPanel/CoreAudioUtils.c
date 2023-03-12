/*
 * CoreAudioUtils.c
 * Audio Extraction Panel Sample Code
 *
 * Copyright:	©2005 by Apple Computer, Inc., all rights reserved.
 */
 
#include "CoreAudioUtils.h"

void ConfigureOutputDescription(ComponentDescription *inOutputDesc)
{
	inOutputDesc->componentType = kAudioUnitType_Output;
	inOutputDesc->componentSubType = kAudioUnitSubType_DefaultOutput;
	inOutputDesc->componentManufacturer = kAudioUnitManufacturer_Apple;
	inOutputDesc->componentFlags = 0;
	inOutputDesc->componentFlagsMask = 0;
}

void ConfigureScheduledPlayerDescription(ComponentDescription *inPlayerDesc)
{
	inPlayerDesc->componentType = kAudioUnitType_Generator;
	inPlayerDesc->componentSubType = kAudioUnitSubType_ScheduledSoundPlayer;
	inPlayerDesc->componentManufacturer = kAudioUnitManufacturer_Apple;
	inPlayerDesc->componentFlags = 0;
	inPlayerDesc->componentFlagsMask = 0;
}	

OSStatus SetOutputUnitStreamFormat (AudioUnit outputUnit, AudioStreamBasicDescription *asbd)
{
	return (AudioUnitSetProperty (outputUnit,
									kAudioUnitProperty_StreamFormat,
									kAudioUnitScope_Input,
									0,/*output*/
									asbd, 
									sizeof(AudioStreamBasicDescription)));
}

OSStatus SetPlayerUnitStreamFormat (AudioUnit playerUnit, AudioStreamBasicDescription *asbd)
{
		
	return   (AudioUnitSetProperty (playerUnit,
									kAudioUnitProperty_StreamFormat,
									kAudioUnitScope_Output,
									0,/*output*/
									asbd, 
									sizeof(AudioStreamBasicDescription)));
}

OSStatus SetOutputUnitChannelLayout(AudioUnit outputUnit, QTPropertyValuePtr layoutProperty, UInt32 size)
{
	return (AudioUnitSetProperty (outputUnit,
									kAudioUnitProperty_AudioChannelLayout,
									kAudioUnitScope_Input,
									0,/*output*/
									layoutProperty,
									size));
}
