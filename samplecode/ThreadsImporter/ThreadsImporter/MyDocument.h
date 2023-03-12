/*
	File:		MyDocument.h
	
	Description: Document-specific code for thread-safety test application.

	Author:		QTEngineering, dts

	Copyright: 	© Copyright 2003-2004 Apple Computer, Inc. All rights reserved.
	
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
				
	Change History (most recent first): <1> dts 11/16/03 initial release

*/

//////////
//
// header files
//
//////////

#import <Cocoa/Cocoa.h>
#import <Carbon/Carbon.h>
#import <QuickTime/QuickTime.h>
#import <pthread.h>
#import "FileObject.h"
#import "WorkerThread.h"

@class AutoRunSettings;

//////////
//
// constants
//
//////////

#define TOGGLE_THREAD_GUARD		2200

// timer periods
#define kAutoRunInterval		0.10

//////////
//
// interfaces
//
//////////

@interface MyDocument : NSDocument
{
    IBOutlet id folderBtn;
    IBOutlet id autorunBtn;
    IBOutlet id progressBar;
    IBOutlet id quickdrawView;
    IBOutlet id tableView;
    
    IBOutlet id posixMenu;
    IBOutlet id mainMenu;

    IBOutlet id handleDHMenu;
    IBOutlet id pointerDHMenu;
    IBOutlet id fileDHMenu;
    IBOutlet id urlDHMenu;
    
    IBOutlet id timeField;
    IBOutlet id sizeField;
    IBOutlet id statusField;

    AutoRunSettings 	*autoRunSettings;

    NSTimer        *_autorunTimer;		// timer for the auto-run state
    NSMutableArray *_fileArray;         // an array of FileObjects
    UInt32          _threadModelTag;	// the threading model for this document
    UInt32          _dhTag;             // the data handler type for this document
    ThreadData     *_currThreadData;	// the thread data for the current import operation
    FileObject     *_fileObject;		// the current image file being displayed or imported
    BOOL            _onlySafeComps;		// do we require only thread-safe components?
    BOOL            _doneDrawing;		// have we finished drawing the current image?
    
    UInt32          _randomAutoRun;		// do we auto-run randomly?
    UInt32          _autoRunIterations;	// number of times we loop through image file list

    WorkerThreadRef	_worker;            // a worker thread handler
}

// document methods
- (id)init;
- (void)dealloc;
- (NSString *)windowNibName;
- (void)windowControllerDidLoadNib:(NSWindowController *) aController;
- (NSData *)dataRepresentationOfType:(NSString *)aType;
- (BOOL)loadDataRepresentation:(NSData *)data ofType:(NSString *)aType;

// window notifications
- (void)windowWillClose:(NSNotification*)notification;

- (void)updateQDImageView:(ThreadData *)threadData  updateTime:(BOOL)doUpdateTime;
//- (void)updateQDImageView:(BOOL)updateTimer;
- (void)cycleImages:(NSTimer *)timer;

// button action handlers
- (IBAction)autoRun:(id)sender;
- (IBAction)selectFolder:(id)sender;

// table notifcation handler and data source methods
- (void)tableViewSelectionDidChange:(NSNotification *)aNotification;
- (int)numberOfRowsInTableView:(NSTableView *)tableView;
- (id)tableView:(NSTableView *)tableView objectValueForTableColumn:(id)column row:(int)row;

// auto-run settings accessors
- (void)setIterations:(UInt32)iterations;
- (void)setRandom:(UInt32)random;

// menu validator and handlers
- (BOOL)validateMenuItem:(NSMenuItem *)item;

- (IBAction)useHandleDH:(id)sender;
- (IBAction)usePointerDH:(id)sender;
- (IBAction)useFileDH:(id)sender;
- (IBAction)useURLDH:(id)sender;

- (IBAction)usePOSIXThreads:(id)sender;
- (IBAction)useMainThread:(id)sender;

- (IBAction)toggleThreadGuard:(id)sender;

- (IBAction)doAutoRunSettingsBox:(id)sender;

- (void)updateWindowTitle;

// getters/setters
- (id)statusField;
- (Rect)qdViewBounds;
- (void *)currThreadData;
- (void)setCurrThreadData:(ThreadData *)threadData;
- (void)disposeThreadData:(ThreadData *)threadData;

- (NSMutableArray *)fileArray;
- (void)setFileArray:(NSMutableArray *)array;
- (NSTimer *)autorunTimer;
- (void)setAutoRunTimer:(NSTimer *)theTimer;

@end
