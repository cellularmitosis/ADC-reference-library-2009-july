/*
    File:  AudioCDDocument.h
    
    Contains:  Document class for handling creation of RedBook audio CDs
     
    Copyright:  (c) Copyright 2005 Apple Computer, Inc. All rights reserved.

    Disclaimer: IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc.
    ("Apple") in consideration of your agreement to the following terms, and your
    use, installation, modification or redistribution of this Apple software
    constitutes acceptance of these terms.  If you do not agree with these terms,
    please do not use, install, modify or redistribute this Apple software.

    In consideration of your agreement to abide by the following terms, and subject
    to these terms, Apple grants you a personal, non-exclusive license, under Apple's
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
            1.0 (July 5th, 2005)
*/


#import <Cocoa/Cocoa.h>
#import <DiscRecording/DiscRecording.h>


@class AudioCDWindowController;

@interface AudioCDDocument : NSDocument
{
	IBOutlet AudioCDWindowController*	windowController;
	
	NSMutableArray	*	trackArray;
	NSMenuItem *		_menuSaveDocument;
}

- (NSDictionary*)discInfo;							// read-only
- (NSDictionary*)trackInfoForTrack:(unsigned)index;	// read-only
- (NSArray*)trackInfo;								// read-only
- (unsigned)count;
- (DRMSF*)totalLength;

// Format verification
- (BOOL)hasValidISRCForTrack:(unsigned)index;
- (BOOL)isValidISRC:(NSString*)isrc incomplete:(BOOL)incomplete;

// Accessors for getting and setting properties
- (id)discPropertyForKey:(NSString*)key;
- (void)setDiscProperty:(id)property forKey:(NSString*)key;

- (id)propertyForKey:(NSString*)key ofTrack:(unsigned)index;
- (void)setProperty:(id)property forKey:(NSString*)key ofTrack:(unsigned)index;

// Accessors for changing the track list
- (void)addTrackFromFile:(NSString*)pathToFile;
- (void)addTrackFromFile:(NSString*)pathToFile atIndex:(unsigned)index;
- (void)removeTrackAtIndex:(unsigned)index;
- (unsigned)moveTracks:(NSArray*)tracks to:(unsigned)destination copying:(BOOL)copying;
- (unsigned)moveTrack:(unsigned)source to:(unsigned)destination copying:(BOOL)copying;

// Burn properties and layout
- (void)setBurnProperties:(DRBurn*)burn;
- (id)createLayoutForBurn;

@end


/* Keys for the disc info dictionary */
extern NSString * const EABDiscCDTextEnabled;	// bool
extern NSString * const EABDiscMCNEnabled;		// bool
extern NSString * const EABDiscMCN;				// NSString


/* Keys for the track info dictionaries */
extern NSString * const EABTrackFilePath;			// NSString, path to file
extern NSString * const EABTrackLength;				// NSNumber, track length in frames
extern NSString * const EABTrackPreGap;				// NSNumber, pregap in seconds
extern NSString * const EABTrackPreEmphasisEnabled;	// bool
extern NSString * const EABTrackIndexPointsEnabled;	// bool
extern NSString * const EABTrackIndexPoints;		// NSArray of NSNumbers, index points
extern NSString * const EABTrackISRCEnabled;		// bool
extern NSString * const EABTrackISRC;				// NSString
extern NSString * const EABTrackISRCInCDText;		// bool


/* Notifications */
extern NSString * const EABAudioCDTrackListChanged;		// notification sent when the track list changes
extern NSString * const EABAudioCDDiscPropertyChanged;	// notification sent when a disc property changes
extern NSString * const EABAudioCDTrackPropertyChanged;	// notification sent when a track property changes
extern NSString * const EABWhichPropertyKey;			// key in notification's userInfo dictionary indicating which property changed
extern NSString * const EABWhichTrackKey;				// key in notification's userInfo dictionary indicating which track(s) were added/changed

enum {
	EABTracksRemoved		= -1						// special meta-value for EABWhichTrackKey indicating that tracks were removed
};

