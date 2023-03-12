/*

File: MyController.h

Author: QuickTime DTS

Change History (most recent first):

	<2> 09/09/07 added counter option
	<1> 08/01/07 initial release

© Copyright 2007 Apple Computer, Inc. All rights reserved.

IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc. ("Apple") in
consideration of your agreement to the following terms, and your use, installation,
modification or redistribution of this Apple software constitutes acceptance of these
terms.  If you do not agree with these terms, please do not use, install, modify or
redistribute this Apple software.

In consideration of your agreement to abide by the following terms, and subject to these
terms, Apple grants you a personal, non-exclusive license, under Apple's copyrights in
this original Apple software (the "Apple Software"), to use, reproduce, modify and
redistribute the Apple Software, with or without modifications, in source and/or binary
forms; provided that if you redistribute the Apple Software in its entirety and without
modifications, you must retain this notice and the following text and disclaimers in all
such redistributions of the Apple Software. Neither the name, trademarks, service marks
or logos of Apple Computer, Inc. may be used to endorse or promote products derived from
the Apple Software without specific prior written permission from Apple.  Except as
expressly stated in this notice, no other rights or licenses, express or implied, are
granted by Apple herein, including but not limited to any patent rights that may be
infringed by your derivative works or by other works in which the Apple Software may be
incorporated.

The Apple Software is provided by Apple on an "AS IS" basis.  APPLE MAKES NO WARRANTIES,
EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED WARRANTIES OF
NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE, REGARDING THE
APPLE SOFTWARE OR ITS USE AND OPERATION ALONE OR IN COMBINATION WITH YOUR PRODUCTS.

IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) ARISING IN ANY WAY OUT OF THE
USE, REPRODUCTION, MODIFICATION AND/OR DISTRIBUTION OF THE APPLE SOFTWARE, HOWEVER
CAUSED AND WHETHER UNDER THEORY OF CONTRACT, TORT (INCLUDING NEGLIGENCE), STRICT
LIABILITY OR OTHERWISE, EVEN IF APPLE HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/

#import <QTKit/QTKit.h>

@interface MyController : NSObject
{
	QTMovie *				mMovie;
    IBOutlet NSWindow *		mWindow;
    IBOutlet QTMovieView *	mMovieView;
    IBOutlet NSTextField *	mSourceNameDisplayField;
    IBOutlet NSTextField *	mTimecodeDisplayField;
	
    BOOL					mHasTimecodeTrack;
    BOOL					mIsDirty;
    BOOL                    mInSaveLoop;
	
	IBOutlet NSPanel *     mPanel;
	IBOutlet NSTextField * mSourceName;
	IBOutlet NSTextField * mTimeScale;
	IBOutlet NSTextField * mFrameDuration;
	IBOutlet NSTextField * mNumberOfFrames;
	IBOutlet NSTextField * mHours;
	IBOutlet NSTextField * mMinutes;
	IBOutlet NSTextField * mSeconds;
	IBOutlet NSTextField * mFrames;
    IBOutlet NSTextField * mCounterValue;
	IBOutlet NSButton *	   mOKButton;
	IBOutlet NSButton *    mCancelButton;
	
	BOOL mDropFrame;
	BOOL m24HourMax;
	BOOL mNegativeOK;
	BOOL mNegative;
    BOOL mUseCounter;
}

- (IBAction)doOpen:(id)sender;
- (IBAction)doUpdateMovieAtom:(id)sender;

- (IBAction)doAddTimecodeTrack:(id)sender;
- (IBAction)doRemoveTimecodeTrack:(id)sender;
- (IBAction)doToggleTimecodeDisplay:(id)sender;
- (IBAction)doOKButton:(id)sender;
- (IBAction)doCancelButton:(id)sender;

- (void)updateSourceAndTCDisplay;

- (void)openMovie:(NSString *)inFile;
- (void)openPanelDidEnd:(NSOpenPanel *)sheet returnCode:(int)returnCode contextInfo:(void *)contextInfo;
- (void)alertDidEnd:(NSAlert *)alert returnCode:(int)returnCode contextInfo:(void *)contextInfo;

- (NSWindow *)window;
- (NSPanel *)panel;

- (BOOL)hasTimecodeTrack;
- (void)setHasTimecodeTrack:(BOOL)value;

- (BOOL)isDirty;
- (void)setIsDirty:(BOOL)value;

- (ConstStringPtr)sourceNamePString;
- (TimeScale)timeScale;
- (TimeValue)frameDuration;
- (UInt8)numberOfFrames;
- (UInt8)hours;
- (UInt8)minutes;
- (UInt8)seconds;
- (UInt8)frames;
- (long)counterValue;

- (BOOL)dropFrame;
- (void)setDropFrame:(BOOL)value;

- (BOOL)twentyFourHoursMax;
- (void)setTwentyFourHoursMax:(BOOL)value;

- (BOOL)negativeOK;
- (void)setNegativeOK:(BOOL)value;

- (BOOL)negative;
- (void)setNegative:(BOOL)value;

- (BOOL)useCounter;
- (void)setUseCounter:(BOOL)value;

@end

@interface NSWindow (ResizingQTMovieView)

- (void)resizeForQTMovieView:(QTMovieView *)inMovieView;

@end
