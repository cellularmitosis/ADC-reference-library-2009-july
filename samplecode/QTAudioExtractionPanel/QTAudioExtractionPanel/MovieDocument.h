/*
File:				MovieDocument.h

	Description:	Demonstrates how to build and extend 
					a QuickTime media player app, using the new QTKit framework. 
					Originally introduced at WWDC 2004 at
					Session 214, "Programming QuickTime with Cocoa." 
					Sample code is explained in detail in 
					"QTKit Programming Guide" documentation.

	Copyright:		� Copyright 2004, 2005 Apple Computer, Inc.
					All rights reserved.

	Disclaimer: IMPORTANT:  This Apple software is supplied to you by
	Apple Computer, Inc. ("Apple") in consideration of your agreement to the
	following terms, and your use, installation, modification or
	redistribution of this Apple software constitutes acceptance of these
	terms.  If you do not agree with these terms, please do not use,
	install, modify or redistribute this Apple software.

	In consideration of your agreement to abide by the following terms, and
	subject to these terms, Apple grants you a personal, non-exclusive
	license, under Apple�s copyrights in this original Apple software (the
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

#import <Cocoa/Cocoa.h>
#import <QTKit/QTKit.h>
#import "QTKitPlayerWindowController.h"
#import "AudioPropInfo.h"

@interface MovieDocument : NSDocument
{
        // movie window
    IBOutlet NSWindow		*mMovieWindow;
    IBOutlet QTMovieView	*mMovieView;

        // export
    IBOutlet NSView			*mExportAccessoryView;
    IBOutlet NSPopUpButton	*mExportTypePopUpButton;

	
        // movie document
    QTMovie					*mMovie;
	NSWindowController		*mWindowController;
	AudioPropInfo			*mAudioPropInfo;	// the audio routing panel class
}

    // class methods
+ (id)movieDocumentWithMovie:(QTMovie *)movie;

    // init
- (id)initWithMovie:(QTMovie *)movie;


    // NSMenu validation protocol
- (BOOL)validateMenuItem:(NSMenuItem *)menuItem;


	// getters
- (QTMovie *)movie;
- (NSWindow *)movieWindow;
- (AudioPropInfo *)audioPropInfo;

    // setters
- (void)setMovie:(QTMovie *)movie;
- (void)setAudioPropInfo:(AudioPropInfo *)propInfo;

    // NSDocument overrides
- (NSString *)windowNibName;
- (void)windowControllerDidLoadNib:(NSWindowController *)windowController;
- (NSData *)dataRepresentationOfType:(NSString *)docType;
- (BOOL)writeWithBackupToFile:(NSString *)fullDocumentPath ofType:(NSString *)type saveOperation:(NSSaveOperationType)saveOperation;
- (BOOL)readFromFile:(NSString *)fileName ofType:(NSString *)type;
- (BOOL)readFromURL:(NSURL *)url ofType:(NSString *)type;
- (void)printShowingPrintPanel:(BOOL)showPanels;

    // NSSavePanel delegates
- (void)exportPanelDidEnd:(NSSavePanel *)sheet returnCode:(int)returnCode contextInfo:(void *)contextInfo;


    // actions
- (IBAction)doExport:(id)sender;
- (IBAction)doSetFillColourPanel:(id)sender;
- (IBAction)doSetFillColour:(id)sender;
- (IBAction)doShowController:(id)sender;
- (IBAction)doPreserveAspectRatio:(id)sender;
- (IBAction)doLoop:(id)sender;
- (IBAction)doLoopPalindrome:(id)sender;
- (IBAction)doClone:(id)sender;

@end
