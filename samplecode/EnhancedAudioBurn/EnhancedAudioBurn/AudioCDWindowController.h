/*
    File:  AudioCDWindowController.h
    
    Contains:  NSWindowController subclass to handle the UI for the document
     
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
#import <DiscRecordingUI/DiscRecordingUI.h>
#import "AudioCDDocument.h"

@interface AudioCDWindowController : NSWindowController
{
    IBOutlet NSButton *_buttonDiscAddTrack;
    IBOutlet NSButton *_buttonDiscBurn;
    IBOutlet NSButton *_buttonDiscRemoveTrack;
    IBOutlet NSButton *_buttonSingleTrackShowFile;
    IBOutlet NSButton *_checkboxDiscCDText;
    IBOutlet NSButton *_checkboxDiscMCN;
	IBOutlet NSButton *_checkboxBulkPreEmphasis;
    IBOutlet NSButton *_checkboxSingleTrackIndexPoints;
    IBOutlet NSButton *_checkboxSingleTrackPreEmphasis;
	IBOutlet NSButton *_checkboxSingleTrackISRC;
	IBOutlet NSButton *_checkboxSingleTrackISRCCDText;
	IBOutlet NSPanel *_panelAdding;
    IBOutlet NSPopUpButton *_popupDiscCDTextGenre;
	IBOutlet NSProgressIndicator *_progressAdding;
    IBOutlet NSTableColumn *_tableColumnFilename;
    IBOutlet NSTableColumn *_tableColumnLength;
    IBOutlet NSTableColumn *_tableColumnTrackNumber;
    IBOutlet NSTableView *_tableDiscTracks;
    IBOutlet NSTabView *_tabviewMainOptions;
    IBOutlet NSTabView *_tabviewTrackOptionsPersonality;
	IBOutlet NSTextField *_textAddingFilename;
    IBOutlet NSTextField *_textDiscCDTextArranger;
    IBOutlet NSTextField *_textDiscCDTextClosed;
    IBOutlet NSTextField *_textDiscCDTextComposer;
    IBOutlet NSTextField *_textDiscCDTextDiscIdent;
    IBOutlet NSTextField *_textDiscCDTextGenre;
    IBOutlet NSTextField *_textDiscCDTextMessage;
    IBOutlet NSTextField *_textDiscCDTextPerformer;
    IBOutlet NSTextField *_textDiscCDTextSongwriter;
	IBOutlet NSTextField *_textDiscCDTextTitle;
    IBOutlet NSTextField *_textDiscLength;
	IBOutlet NSTextField *_textDiscMCN;
    IBOutlet NSTextField *_textDiscTrackCount;
    IBOutlet NSTextField *_textBulkCDTextArranger;
    IBOutlet NSTextField *_textBulkCDTextClosed;
    IBOutlet NSTextField *_textBulkCDTextComposer;
    IBOutlet NSTextField *_textBulkCDTextMessage;
    IBOutlet NSTextField *_textBulkCDTextPerformer;
    IBOutlet NSTextField *_textBulkCDTextSongwriter;
    IBOutlet NSTextField *_textBulkCDTextTitle;
    IBOutlet NSTextField *_textBulkPreGap;
	IBOutlet NSTextField *_textBulkPreGapSeconds;
    IBOutlet NSTextField *_textSingleTrackCDTextArranger;
    IBOutlet NSTextField *_textSingleTrackCDTextClosed;
    IBOutlet NSTextField *_textSingleTrackCDTextComposer;
    IBOutlet NSTextField *_textSingleTrackCDTextMessage;
    IBOutlet NSTextField *_textSingleTrackCDTextPerformer;
    IBOutlet NSTextField *_textSingleTrackCDTextSongwriter;
    IBOutlet NSTextField *_textSingleTrackCDTextTitle;
    IBOutlet NSTextField *_textSingleTrackFilename;
	IBOutlet NSTextField *_textSingleTrackISRC;
	IBOutlet NSTextField *_textSingleTrackISRCInvalid;
    IBOutlet NSTextField *_textSingleTrackPreGap;
    IBOutlet NSTextField *_textSingleTrackPreGapSeconds;
	
	// Cached representations of the document and disc information.
	AudioCDDocument *document;
	BOOL			disableNotifications;
	NSMutableArray	*listOfChanges;
	BOOL			burning;
	
	BOOL			bulkModifyCDTextTitle;
	BOOL			bulkModifyCDTextPerformer;
	BOOL			bulkModifyCDTextComposer;
	BOOL			bulkModifyCDTextArranger;
	BOOL			bulkModifyCDTextSongwriter;
	BOOL			bulkModifyCDTextMessage;
	BOOL			bulkModifyCDTextClosed;
}

- (IBAction)discBurnRequest:(id)sender;
- (void)burnSetupPanelDidEnd:(DRBurnSetupPanel*)panel returnCode:(int)returnCode contextInfo:(void*)contextInfo;
- (void)burnProgressPanelWillBegin:(NSNotification*)notification;
- (void)burnProgressPanelDidFinish:(NSNotification*)notification;

- (int)aggregateTrackCheckboxStateForKey:(NSString *)key;
- (NSString*)aggregateTrackStringForKey:(NSString*)key;
- (NSNumber*)aggregateTrackNumberForKey:(NSString*)key;

- (void)updateOptions;
- (void)updateDiscTrackTable;

- (IBAction)discAddTrackRequest:(id)sender;
- (void)openPanelDidEnd:(NSOpenPanel*)panel returnCode:(int)returnCode contextInfo:(void *)contextInfo;
- (void)addFiles:(NSArray*)filenames atIndex:(unsigned)index;
- (IBAction)discRemoveTrackRequest:(id)sender;

- (IBAction)discCDTextChanged:(id)sender;
- (IBAction)discCDTextToggle:(id)sender;
- (IBAction)discMCNChanged:(id)sender;
- (IBAction)discMCNToggle:(id)sender;

- (BOOL)control:(NSControl*)control textShouldBeginEditing:(NSText*)text;
- (void)controlTextDidChange:(NSNotification *)notification;

- (IBAction)bulkCDTextChanged:(id)sender;
- (IBAction)bulkPreEmphasisToggle:(id)sender;
- (IBAction)bulkPreGapChanged:(id)sender;
- (IBAction)singleTrackCDTextChanged:(id)sender;
- (IBAction)singleTrackIndexPointsChanged:(id)sender;
- (IBAction)singleTrackIndexPointsToggle:(id)sender;
- (IBAction)singleTrackPreEmphasisToggle:(id)sender;
- (IBAction)singleTrackPreGapChanged:(id)sender;
- (IBAction)singleTrackShowFileRequest:(id)sender;

- (void)trackListDidChange:(NSNotification*)notification;
- (void)discPropertyDidChange:(NSNotification*)notification;
- (void)trackPropertyDidChange:(NSNotification*)notification;
- (void)willUndoOrRedoChange:(NSNotification*)notification;
- (void)didUndoOrRedoChange:(NSNotification*)notification;

@end


