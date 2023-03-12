//
//  SoundPlayer.h
//
//  Created by Michael K Larson on Wed Jun 04 2003.
//  Copyright (c) 2003 __MyCompanyName__. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <QuickTime/QuickTime.h>

#define BailErr(x) {err = x; if(err != noErr) goto bail;}

//////////
//
// constants
//
//////////

#define kSndEqNumBands				8
#define	kSndEqResID					1000		// resource ID of DLOG resource to contain the equalizer
#define	kSndEqUserItemIndex			1			// index of user item we draw the equalizer into
#define	kSndEqTickThreshold			5			// the number of ticks we wait before updating display

// the band frequencies (these are the same bands used by QuickTime Player)
#define kBandFreq0				0x00C80000; 	// 00200 Hz
#define kBandFreq1				0x01900000; 	// 00400 Hz
#define kBandFreq2				0x03200000; 	// 00800 Hz
#define kBandFreq3				0x06400000; 	// 01600 Hz
#define kBandFreq4				0x0C800000; 	// 03200 Hz
#define kBandFreq5				0x19000000; 	// 06400 Hz
#define kBandFreq6				0x32000000; 	// 12800 Hz
#define kBandFreq7				0x52080000; 	// 21000 Hz

#define kSndEqNumCmdsInQueue			4		// number of commands in a sound channel queue

@interface SoundPlayer : NSObject {
    MediaHandler			m_mediaHandler;
}

- (void) PlaySound: (const FSSpec *) inFileToPlay;
- (void) GetSoundEqualizerBandLevels: (UInt8 *) levels;
- (int) IsMusicPlaying;

@end

