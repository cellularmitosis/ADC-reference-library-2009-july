/*
	File:		MyDocument.m
	
	Description: Document-specific code for thread-safe movie export test application.

	Author:		QuickTime Engineering

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
				
	Change History (most recent first):  <1> dts 01/28/04 initial release
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
#import "ThreadData.h"
#import "WorkerThread.h"

//////////
//
// constants
//
//////////

#define kMoviePaneOffset            20
#define kMovieBottomOffset          58
#define kMovieControllerBarHeight	16

// menu item tags
#define USE_POSIX_THREAD    2000
#define USE_MAIN_THREAD     2001

@class ProgressDialog;

@interface MyDocument : NSDocument
{
    IBOutlet id		movieView;
    
    IBOutlet id		posixMenu;
    IBOutlet id		mainMenu;

    ProgressDialog 	*progressDialog;
    
    UInt32      _threadModelTag;        // the threading model for this document
    ThreadData  *_currThreadData;       // the thread data for the current export operation
    BOOL        _onlySafeComponents;	// do we require only thread-safe components?
    BOOL		_selectedMovie;         // has the user selected a movie for this document?
    BOOL		_ignoreUnsafeTypes;     // do we ignore unsafe media types during export?
    
    WorkerThreadRef	_worker;            // a worker thread handler
}

// document methods
- (id)init;
- (void)dealloc;
- (NSString *)windowNibName;
- (void)windowControllerDidLoadNib:(NSWindowController *) aController;
- (NSData *)dataRepresentationOfType:(NSString *)aType;
- (BOOL)loadDataRepresentation:(NSData *)data ofType:(NSString *)aType;

// methods
- (IBAction)usePOSIXThreads:(id)sender;
- (IBAction)useMainThread:(id)sender;
- (IBAction)toggleThreadGuard:(id)sender;
- (IBAction)ignoreUnsafeTypes:(id)sender;

- (IBAction)doButton:(id)sender;
- (IBAction)selectMovie:(id)sender;
- (IBAction)doExport:(id)sender;

- (NSSize)windowContentSizeForMovie:(Movie)qtMovie;
- (void)releaseThreadData:(ThreadData *)threadData;

@end
