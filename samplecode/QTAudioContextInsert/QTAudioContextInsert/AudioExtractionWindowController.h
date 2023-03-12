/*
	File:		AudioExtractionWindowController.h

	Abstract:   Implements the Audio Extraction Window.
					
				1. Demonstrates the opening and configuration of an audio 
				extraction session (setting of layout, start time, duration,
				etc.) and how to preview and perform extraction on a worker
				thread, or alternatively, on a main thread.
				
				2. Demonstrates how an Audio Context Insert can be used 
				during extraction.
				
				3. Also shows how CoreAudio can be used for playback of 
				the extracted audio.
				
				
	Version:	1.0
				Contains modifies versions of code that is part of the
				QTAudioExtractionPanel sample code project, available
				online and introduced originally at WWDC 2005 Session 201:
				"Harnessing the Audio Capabilities of QuickTime 7" 
 
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

#import <AudioToolbox/AudioToolbox.h>
#import <AudioUnit/AudioUnit.h>
#import <Carbon/Carbon.h>
#import <Cocoa/Cocoa.h>
#import <CoreAudio/CoreAudioTypes.h>
#import <QTKit/QTKit.h>
#import <QuickTime/QuickTime.h>

#import "CustomDataTypes.h"
#import "CustomTableColumns.h"
#import "CoreAudioPlayback.h"
#import "QuickTimeAudioUtils.h"
#include <unistd.h>
#import "ACInsertManager.h"

@class MovieDocument;
@class ACInsert;

@interface AudioExtractionController : NSWindowController 
{
	IBOutlet NSPopUpButton		*uiExtractionLayoutSelectorPopUpButton;
	IBOutlet NSTableView		*uiExtractionChannelLayoutTableView;
	IBOutlet NSTextField		*uiExtractionStartTimeTextField;
	IBOutlet NSTextField		*uiExtractionEndTimeTextField;
	IBOutlet NSButton			*uiExtractionPreviewButton;
	IBOutlet NSButton			*uiExtractionExportButton;
	
	
	MovieDocument				*mMovieDocument;
	QTMovie						*mClonedMovie;			
	
	NSValue						*mCurrentExtractStartTime;
	NSValue						*mCurrentExtractEndTime;
	Boolean						mStopPreview;
	Boolean						mStopExport;
	Boolean						mUseExtractionEndTime;

	NSMutableArray				*mCommonChannelLabelOptions;
	
	AudioStreamBasicDescription	mSummaryASBD;
	AudioChannelLayout			*mSummaryLayout;
	AudioChannelLayout			*mExtractionLayout;
	
	// For playback through CoreAudio during extraction preview
	AUGraphPlayerRef			mGraphUnit;
	AudioUnit					mPlayerAudioUnit;
	
	// AudioFileID of the file to which extracted audio will be written
	AudioFileID					mExportFileID;
}

    // init
- (id)init;

	// IB actions
- (IBAction) doSelectExtractionChannelLayout:(id)sender;
- (IBAction) doChangeExtractionTime:(id)sender;
- (IBAction) doStartPreview:(id)sender;		
- (IBAction) doStopPreview:(id)sender;
- (IBAction) doStartExport:(id)sender;
- (IBAction) doStopExport:(id)sender;

	// getter
- (QTMovie*)  movie;

	// setter
- (void)setExtractionTime:(NSValue *)theTimeValue isStartTime:(BOOL)isStart isInit:(BOOL)isInit;

	// UI related
- (void) getCommonChannelLabelOptions;
- (void) populateExtractChannelsSelectorPopUpButton;
- (void) refreshExtractionTableView;

	// NSTableColumn delegate
- (id)dataCellForRow:(int)row forTable:(NSTableView*)tableView;

	// Export and Preview	
	// Get the user-specified parameters required for export/playback 
- (OSStatus)getExtractionParameters:(AudioChannelLayout**)layout 
					outLayoutSize:(UInt32*)layoutSize
					outASBD:(AudioStreamBasicDescription*)asbd 
					startTime:(TimeRecord*)startTime 
					duration:(Float64*)duration 
					allDiscrete:(Boolean*)allDiscrete
					insertRegInfo:(InsertRegistryInfo*)regInfoRef;

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

	// Utility methods
- (void) getSummaryASBD;

// Convert time strings to QTTime and vice versa
- (NSString*) StringFromQTTime:(QTTime) time;
- (QTTime) QTTimeFromString:(NSString*)timeString timeScale:(long)scale;

	// notification callbacks
- (void)windowWillClose:(NSNotification*)notification;
- (void)movieTracksChanged:(NSNotification *)notification;

@end

// This notification is sent whenever a track is enabled or disabled
// or a track's channel layout changes. The Audio Properties Window
// controller posts this notification and the Audio Extraction Window
// and the Audio Context Inserts Window are observers of this 
// notification
extern	NSString *QTAudioContextInsertMovieTracksChangedNotification;