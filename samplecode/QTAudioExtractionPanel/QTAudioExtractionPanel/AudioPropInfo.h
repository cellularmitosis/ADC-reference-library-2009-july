/*
	File:		AudioPropInfo.h

	Description:    Demonstrates how audio channel layouts and track 
					and movie properties can be manipulated.
				    Demonstrates the opening and configuration of an audio 
					extraction sessions (setting of layout, starttime, duration, etc.)
					Shows how CoreAudio can be used for playback of the extracted audio.
					Provides examples of performing the preview and extraction tasks 
					on a worker thread, and alternatively on the main thread
												
		Originally introduced at WWDC 2005 at Session 201:
			"Harnessing the Audio Capabilities of QuickTime 7" 


	Copyright:   © Copyright 2004, 2005 Apple Computer, Inc.
				 All rights reserved.

	Disclaimer: IMPORTANT:  This Apple software is supplied to you by
	Apple Computer, Inc. ("Apple") in consideration of your agreement to the
	following terms, and your use, installation, modification or
	redistribution of this Apple software constitutes acceptance of these
	terms.  If you do not agree with these terms, please do not use,
	install, modify or redistribute this Apple software.

	In consideration of your agreement to abide by the following terms, and
	subject to these terms, Apple grants you a personal, non-exclusive
	license, under Apple’s copyrights in this original Apple software (the
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

*/

 
#import <AudioToolbox/AudioToolbox.h>
#import <AudioUnit/AudioUnit.h>
#import <Carbon/Carbon.h>
#import <Cocoa/Cocoa.h>
#import <CoreAudio/CoreAudioTypes.h>
#import <QTKit/QTKit.h>
#import <QuickTime/QuickTime.h>

#import "InfoObject.h"
#import "PopUpTableColumn.h"
#import "CoreAudioPlayback.h"
#import "InfoForCallback.h"
#import "QuickTimeAudioUtils.h"
#include <unistd.h>


@class MovieDocument;

@interface AudioPropInfo : NSObject
{
    IBOutlet NSPanel			*_audioPanel;
	IBOutlet NSTabView			*_audioTabView;

	IBOutlet NSPopUpButton		*_audTrackSelectorPopUpButton;
	IBOutlet NSButton			*_audTrackEnabledCheckBox;
	IBOutlet NSTableView		*_audTrackChannelLayoutTableView;
	IBOutlet NSSlider			*_audTrackGainSlider;
	IBOutlet NSTextField		*_audSummaryChannelLayoutTextField;
	
	
	IBOutlet NSTableView		*_audDeviceChannelLayoutTableView;
	IBOutlet NSSlider			*_audMovieGainSlider;
	IBOutlet NSButton			*_audConfigureSpeakersButton;

	IBOutlet NSPopUpButton		*_audExtractLayoutSelectorPopUpButton;
	IBOutlet NSTableView		*_audExtractChannelLayoutTableView;
	IBOutlet NSButton			*_audExtractPreviewButton;
	IBOutlet NSButton			*_audExtractExportButton;
	IBOutlet NSTextField		*_audExtractStartTimeTextField;
	IBOutlet NSTextField		*_audExtractEndTimeTextField;

	
	QTMovie						*_currentMovie;
	QTTrack						*_currentTrack;
	Movie						_clonedMovie;
	NSString					*_currentDocFileName;

	NSMutableArray				*_trackChannelLabelNames;
	NSMutableArray				*_trackChannelLabelsMenusArray;
	NSMutableArray				*_trackChannelLabelsIndexOfSelectedMenuItemArray;

	NSMutableArray				*_extractionChannelLabelNames;
	NSMutableArray				*_extractionLayoutMenuList;
	NSValue						*_currentExtractStartTime;
	NSValue						*_currentExtractEndTime;
	
	AudioChannelLayout			*_summaryLayout;
	AudioChannelLayout			*_deviceLayout;
	AudioChannelLayout			*_extractionLayout;
	AudioStreamBasicDescription	_summaryASBD;
	
	AUGraphPlayerRef			_graphUnit;
	AudioUnit					_thePlayerUnit;
	AudioFileID					_exportFileID;

	Boolean						_stopPreview;
	Boolean						_stopExport;

}
    // class method
+ (AudioPropInfo *)audioPropInfo;

    // init
- (id)init;

#pragma mark
#pragma mark ---- Actions ----------------------------------------------

- (IBAction) doSelectTrack:(id)sender;
- (IBAction) doChangeTrackEnabled:(id)sender;
- (IBAction) doChangeTrackGain:(id)sender;	
- (IBAction) doLaunchAMS:(id)sender;	
- (IBAction) doChangeMovieGain:(id)sender;			
- (IBAction) doSelectExtractionChannelLayout:(id)sender;
- (IBAction) doStartPreview:(id)sender;		
- (IBAction) doStopPreview:(id)sender;
- (IBAction) doStartExport:(id)sender;
- (IBAction) doStopExport:(id)sender;
- (IBAction) doChangeExtractionTime:(id)sender;

#pragma mark
#pragma mark ---- Getters ----------------------------------------------

- (NSPanel*)  audioPropInfoPanel;
- (QTMovie*)  movie;
- (QTTrack*)  track;
- (NSSlider*) movieGainSlider;
- (NSSlider*) trackGainSlider;
- (float)     movieGain;
- (float)     trackGainForTrack:(QTTrack *)track;

#pragma mark
#pragma mark ---- Setters ----------------------------------------------

- (void) setMovie:(QTMovie*)theMovie fileName:(NSString *)name;
- (void) setTrack:(QTTrack*)theTrack;
- (void) setMovieGain:(float) gain;
- (void) setTrackGain:(float) gain forTrack:(QTTrack *)track;
- (void) setExtractionTime:(NSValue *)theTimeValue isStartTime:(BOOL)isStart isInit:(BOOL)isInit;

#pragma mark
#pragma mark ---- Methods for the Panel UI -----------------------------

- (void) loadPanel;
- (void) showPanel;
- (void) hidePanel;
- (BOOL) isVisible;
- (void) rebuildInfoPanel;
- (void) refreshExtractionTableView;
- (void) populateTrackSelectorPopUpButton;
- (void) populateExtractChannelsSelectorPopUpButton;
- (void) setSummaryChannelLayoutTextField;
- (void) createLabelsArray;
- (void) addLabelToLabelNamesArray:(NSMutableArray *)namesArray label:(AudioChannelLabel) thisLabel;

	// NSTableColumn delegate
- (id)dataCellForRow:(int)row forTable:(NSTableView*)tableView;


#pragma mark
#pragma mark ---- Audio Extraction: Export and Preview --------------------

	// Get the user-specified parameters required for export/playback 
- (OSStatus)getExtractionParameters:(AudioChannelLayout**)layout 
					outLayoutSize:(UInt32*)layoutSize
					outASBD:(AudioStreamBasicDescription*)asbd 
					startTime:(TimeRecord*)startTime 
					duration:(Float64*)duration 
					allDiscrete:(Boolean*)allDiscrete;


	// Export: Extract to file 
- (void) startExport:(NSSavePanel *)sheet returnCode:(int)returnCode contextInfo:(void *)contextInfo;
- (void) exportOnMainThreadCallBack:(id)object;
- (void) exportExtractionThread:(id)theObject;
- (void) exportCompletedNotification:(id)object;

	// Playback: Preview extraction
- (void) startPreview;
- (void) previewOnMainThreadCallBack:(id)object;
- (void) previewExtractionThread:(id)theObject;
- (void) previewCompletedNotification:(id)object;
- (void) previewBufferDeallocate:(ScheduledAudioSlice *)sliceList numSlices:(UInt32)numSlices;
- (OSStatus) previewBufferAllocate:(ScheduledAudioSlice *)sliceList
									numSlices:(UInt32)numSlices
									asbd:(AudioStreamBasicDescription)asbd
									lock:(void *)condLock;
- (UInt32) previewBufferScheduleSlices:(ScheduledAudioSlice *)sliceList
									numSlices:(UInt32)numSlices
									extractionSession:(MovieAudioExtractionRef)extraction
									asbd:(AudioStreamBasicDescription)asbd
									timeStamp:(Float64*)ioSampleTimeStamp
									remaining:(SInt64*)ioSamplesRemaining
									complete:(Boolean*)outExtractionComplete;

#pragma mark
#pragma mark ---- Utility Methods ----------------------------------------

// Cocoa wrappers around C QuickTimeUtility functions
- (void) getSummaryChannelLayout;
- (void) getDeviceLayout;
- (void) getAllDiscreteLayout;
- (void) addMovieGainPropertyListener:(Movie)movie;
- (void) addTrackGainPropertyListener:(Track)track;
- (void) removeMovieGainPropertyListener:(Movie)movie;
- (void) removeTrackGainPropertyListener:(Track)track;

// Convert time strings to QTTime and vice versa
- (NSString*) StringFromQTTime:(QTTime) time;
- (QTTime) QTTimeFromString:(NSString*)timeString timeScale:(long)scale;

@end
