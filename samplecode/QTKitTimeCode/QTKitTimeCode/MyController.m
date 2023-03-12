/*

File: MyController.m

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

#import "MyController.h"
#import "TimecodeUtilities.h"

@implementation MyController

- (void)awakeFromNib
{
	if (mPanel == nil) {
		// make sure our cocoa window has a valid windowRef
		[mWindow windowRef];
		
		NSNotificationCenter *nc = [NSNotificationCenter defaultCenter];
		
        // set some defaults for the Timecode panel UI
		[self setDropFrame:YES];
		
        // add some QTMovie notifications
		[nc addObserver:self selector:@selector(movieLoadStateDidChange:) name:@"QTMovieLoadStateDidChangeNotification" object:nil];
		[nc addObserver:self selector:@selector(movieRateDidChangeNotification:) name:@"QTMovieRateDidChangeNotification" object:nil];
		[nc addObserver:self selector:@selector(movieEditedNotification:) name:@"QTMovieEditedNotification" object:nil];
        
        // make sure the font panel has our default font selected
        NSFontManager *fontManager = [NSFontManager sharedFontManager];
        NSFont *theDefaultFont = [NSFont fontWithName:@"Helvetica" size:14.0];
        [fontManager setSelectedFont:theDefaultFont isMultiple:NO];
        
        // clear the default movie view movie
        [mMovieView setMovie:nil];
	}
}

#pragma mark ---- panel callbacks ----

// movie open panel
- (void)openPanelDidEnd:(NSOpenPanel *)sheet returnCode:(int)returnCode contextInfo:(void *)contextInfo
{
#pragma unused(contextInfo)

    if (NSOKButton == returnCode) {
        NSString *theFilename = [[sheet filenames] objectAtIndex:0];
        [sheet close];
    
        [self openMovie:theFilename];
        if (mInSaveLoop) {
        	mInSaveLoop = NO;
        }
    } else if (mInSaveLoop) {
        mInSaveLoop = NO;
        [self setIsDirty:YES];
    }
}

// save alert sheet
- (void)alertDidEnd:(NSAlert *)alert returnCode:(int)returnCode contextInfo:(void *)contextInfo
{
#pragma unused(alert, returnCode, contextInfo)
	
    [[alert window] orderOut:nil];
		
	if (NSAlertDefaultReturn == returnCode) {
		[self doUpdateMovieAtom:nil];
	} else {
		// don't save
        [self setIsDirty:NO];
		[self doOpen:nil];
	}
}

#pragma mark ---- open & save ----

// select a file to open
- (IBAction)doOpen:(id)sender
{
#pragma unused(sender)

	if (mMovie != nil && [self isDirty] ) {
        mInSaveLoop = YES;
        
		NSAlert *alert = [NSAlert alertWithMessageText:@"Do you want to Save the Movie?"
								  defaultButton:@"Save"
								  alternateButton:@"Don't Save"
								  otherButton:nil
								  informativeTextWithFormat:@"You may have added or removed a Timecode track, update the Movie to save these changes."];
							  
		[alert beginSheetModalForWindow:[self window]
			   modalDelegate:self
			   didEndSelector:@selector(alertDidEnd:returnCode:contextInfo:)
			   contextInfo:NULL];
	} else {
		[[NSOpenPanel openPanel] beginSheetForDirectory:nil
								 file:nil
								 types:[NSArray arrayWithObjects:@"mov", @"MOV", nil]
								 modalForWindow:[self window]
								 modalDelegate:self
								 didEndSelector:@selector(openPanelDidEnd:returnCode:contextInfo:)
								 contextInfo:nil];
	}
}

// open Movie and get a QTMovie object
// we handle load state though the movieLoadStateDidChange selector
- (void)openMovie:(NSString *)inFile
{
    NSError *error = nil;
    
    mMovie = [QTMovie movieWithFile:inFile error:&error];    
    if (nil == error) {
    	[mMovie retain];
        
        // if a Movie loads really fast QTKit will not send a LoadStateDidChange notification so we do it here
        [[NSNotificationCenter defaultCenter] postNotification:[NSNotification notificationWithName:@"QTMovieLoadStateDidChangeNotification" object:nil]];
    } else {
		NSAlert *alert = [NSAlert alertWithError:error];
        
		[alert runModal];
    }

}

// called to save modifications to the Movie atom
- (IBAction)doUpdateMovieAtom:(id)sender
{
#pragma unused(sender)
	
	BOOL didSave = [mMovie updateMovieFile];
	if (YES == didSave) {
		[self setIsDirty:NO];
	} else {
		NSAlert *alert = [NSAlert alertWithMessageText:@"Update Failed!"
								  defaultButton:@"Bummers"
								  alternateButton:nil
								  otherButton:nil
								  informativeTextWithFormat:@"QTKit method updateMovieFile failed!"];
                                  
        [alert runModal];
	}
}

#pragma mark ---- getters & setters ----

- (NSWindow *)window
{
    return mWindow;
}

- (NSPanel *)panel
{
    return mPanel;
}

- (BOOL)hasTimecodeTrack
{
    return mHasTimecodeTrack;
}

- (void)setHasTimecodeTrack:(BOOL)value
{
	mHasTimecodeTrack = value;
}

- (BOOL)isDirty
{
    return mIsDirty;
}

- (void)setIsDirty:(BOOL)value
{
	mIsDirty = value;
}

#pragma mark ---- notifications ----

- (void)movieEditedNotification:(NSNotification *)notification
{
#pragma unused(notification)

	[self setIsDirty:YES];
}

// update the source name and timecode time on rate changes
- (void)movieRateDidChangeNotification:(NSNotification *)notification
{
#pragma unused(notification)

	if (!mInSaveLoop) {
		[self updateSourceAndTCDisplay];
    }
}

// movieLoadStateDidChange is called for QTMovieLoadStateDidChangeNotification notifications
- (void)movieLoadStateDidChange:(NSNotification *)notification
{
#pragma unused(notification)

    if (kMovieLoadStateComplete == [[mMovie attributeForKey:QTMovieLoadStateAttribute] longValue]) {
		// at least have a video track to start
		if (TRUE == [[mMovie attributeForKey:QTMovieHasVideoAttribute] boolValue]) {
        
            // if we're replacing a movie get rid of it
        	if (nil != [mMovieView movie]) {
                [mMovieView setMovie:nil];
                [self setIsDirty:NO];
            }
            
            // set the movie into the view
            [mMovieView setMovie:mMovie];
            [mMovie release];
            
			// do we already have a timecode track
			if (YES == QTKitTC_QTMovieHasTimeCodeTrack(mMovie)) {
				[self setHasTimecodeTrack:YES];
			} else {
				[self setHasTimecodeTrack:NO];
			}
            
            [self updateSourceAndTCDisplay];
            
		} else {
			NSAlert *alert = [NSAlert alertWithMessageText:@"No Video Track!"
									  defaultButton:@"OK"
									  alternateButton:nil
									  otherButton:nil
									  informativeTextWithFormat:@"Open some media with at least one video track if you want to add a Timecode track."];

            [alert runModal];
            
            // restore the original movie
            [mMovie release];
            mMovie = [mMovieView movie];
			
			return;
		}
        		
		[mWindow resizeForQTMovieView:mMovieView];
		
        [mWindow setTitle:[mMovie attributeForKey:QTMovieDisplayNameAttribute]];
    }
}

#pragma mark ---- window delegates ----

// ask to save changes if dirty
- (BOOL)windowShouldClose:(id)sender
{
#pragma unused(sender)

    if ([self isDirty]) {
        NSAlert *alert = [NSAlert alertWithMessageText:@"Do you want to update the Movie before you Quit?"
                              defaultButton:@"Save"
                              alternateButton:@"Don't Save"
                              otherButton:nil
                              informativeTextWithFormat:@""];

        if (NSAlertDefaultReturn == [alert runModal]) {
			[self doUpdateMovieAtom:nil];
        }
    }

    return YES;
}

#pragma mark ---- application delegates ----

- (NSApplicationTerminateReply)applicationShouldTerminate:(NSApplication *)sender
{
#pragma unused(sender)

	if ([mWindow isVisible]) {
		[mWindow performClose:nil];
    }
    
    return NSTerminateNow;
}

// split when window is closed
- (BOOL)applicationShouldTerminateAfterLastWindowClosed:(NSApplication *)sender
{
#pragma unused(sender)

    return YES;
}

#pragma mark ---- panel getters & setters ----

-(ConstStringPtr)sourceNamePString
{
	StringPtr theBuffer = (StringPtr)calloc(1, 256);
	if (NULL == theBuffer) return NULL;
	
	BOOL success = CFStringGetPascalString((CFStringRef)[mSourceName stringValue], theBuffer, 256, kCFStringEncodingMacRoman);
	if (!success) return NULL;
	
	return theBuffer;
}

- (TimeScale)timeScale {
    return [mTimeScale intValue];
}

- (TimeValue)frameDuration {
    return [mFrameDuration intValue];
}

- (UInt8)numberOfFrames {
    return [mNumberOfFrames intValue];
}

- (UInt8)hours {
    return [mHours intValue];
}

- (UInt8)minutes {
    return [mMinutes intValue];
}

- (UInt8)seconds {
    return [mSeconds intValue];
}

- (UInt8)frames {
    return [mFrames intValue];
}

- (long)counterValue {
    return [mCounterValue intValue];
}

- (BOOL)dropFrame
{
    return mDropFrame;
}

- (void)setDropFrame:(BOOL)value
{
	mDropFrame = value;
}

- (BOOL)twentyFourHoursMax
{
    return m24HourMax;
}

- (void)setTwentyFourHoursMax:(BOOL)value
{
	m24HourMax = value;
}

- (BOOL)negativeOK
{
    return mNegativeOK;
}

- (void)setNegativeOK:(BOOL)value
{
	mNegativeOK = value;
}

- (BOOL)negative
{
    return mNegative;
}

- (void)setNegative:(BOOL)value
{
	mNegative = value;
}

- (BOOL)useCounter
{
    return mUseCounter;
}

- (void)setUseCounter:(BOOL)value
{
	mUseCounter = value;
}

#pragma mark ---- timecode panel actions ----

- (IBAction)doAddTimecodeTrack:(id)sender
{
#pragma unused(sender)

	if (![NSBundle loadNibNamed:@"Panel" owner:self]) {
        NSLog(@"Warning! Could not load Panel nib file!\n");
    }

    [NSApp runModalForWindow:mPanel];
    [self updateSourceAndTCDisplay];
}

- (IBAction)doRemoveTimecodeTrack:(id)sender
{
#pragma unused(sender)

	QTKitTC_DeleteTimeCodeTracks(mMovie);
    
	[mWindow resizeForQTMovieView:mMovieView];
	[self setHasTimecodeTrack:QTKitTC_QTMovieHasTimeCodeTrack(mMovie)];
    [self updateSourceAndTCDisplay];
}

- (IBAction)doToggleTimecodeDisplay:(id)sender
{
#pragma unused(sender)

	QTKitTC_ToggleTimeCodeDisplay(mMovie);
	[mWindow resizeForQTMovieView:mMovieView];
}

- (IBAction)doOKButton:(id)sender
{
#pragma unused(sender)

	[NSApp stopModalWithCode:noErr];
    [mPanel orderOut:nil];
    
    QTKitTC_AddTimeCodeTrackToQTMovie(mMovie, self);
	[mWindow resizeForQTMovieView:mMovieView];
	[self setHasTimecodeTrack:QTKitTC_QTMovieHasTimeCodeTrack(mMovie)];
}

- (IBAction)doCancelButton:(id)sender
{
#pragma unused(sender)

	[NSApp stopModalWithCode:userCanceledErr];
    [mPanel orderOut:nil];
}

#pragma mark ---- utitlity methods ----

- (void)updateSourceAndTCDisplay
{
	if ([self hasTimecodeTrack]) {
		NSString *sourceName = QTKitTC_GetTimeCodeSourceString(mMovie);
		NSString *timeCodeTime = QTKitTC_GetCurrentTimeCodeString(mMovie);

		[mSourceNameDisplayField setStringValue:sourceName];
		[mTimecodeDisplayField setStringValue:timeCodeTime];

		[sourceName release];
		[timeCodeTime release];
	} else {
    	[mSourceNameDisplayField setStringValue:@""];
		[mTimecodeDisplayField setStringValue:@""];
    }
}

- (unsigned int)validModesForFontPanel:(NSFontPanel *)fontPanel
{
#pragma unused (fontPanel)

	unsigned int fontPanelModeMask = NSFontPanelFaceModeMask | 
    								 NSFontPanelSizeModeMask | 
                                     NSFontPanelCollectionModeMask;
	
    return fontPanelModeMask;
}

@end

@implementation NSWindow (ResizingQTMovieView)

- (void)resizeForQTMovieView:(QTMovieView *)inMovieView
{
	NSSize movieSize = [[[inMovieView movie] attributeForKey:QTMovieNaturalSizeAttribute] sizeValue];

	if ([inMovieView isControllerVisible]) {
		movieSize.height += [inMovieView movieControllerBounds].size.height;

	}
	
	NSRect windowFrame = [self frame];
	NSRect movieViewFrame = [inMovieView frame];
	float deltaHeight = movieSize.height - movieViewFrame.size.height;
	float deltaWidth = movieSize.width - movieViewFrame.size.width;
	
    windowFrame.origin.y -= deltaHeight;
	windowFrame.size.height += deltaHeight;
	windowFrame.size.width += deltaWidth;
	
	[self setFrame:windowFrame display: YES animate: YES];
}

@end