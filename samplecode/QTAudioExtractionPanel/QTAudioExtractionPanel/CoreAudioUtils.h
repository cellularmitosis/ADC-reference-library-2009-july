/*
 * CoreAudioUtils.h
 * Audio Extraction Panel Sample Code
 *
 * Copyright:	©2005 by Apple Computer, Inc., all rights reserved.
 */

#include <Carbon/Carbon.h>
#include <QuickTime/QuickTime.h>
#include <AudioUnit/AudioUnit.h>
#include <AudioToolbox/AudioToolbox.h>

extern void ConfigureOutputDescription(ComponentDescription *inOutputDesc);
extern void ConfigureScheduledPlayerDescription(ComponentDescription *inPlayerDesc);
extern OSStatus SetOutputUnitStreamFormat (AudioUnit outputUnit, AudioStreamBasicDescription *asbd);
extern OSStatus SetPlayerUnitStreamFormat (AudioUnit playerUnit, AudioStreamBasicDescription *asbd);
extern OSStatus SetOutputUnitChannelLayout (AudioUnit outputUnit, QTPropertyValuePtr layoutProperty, UInt32 size);