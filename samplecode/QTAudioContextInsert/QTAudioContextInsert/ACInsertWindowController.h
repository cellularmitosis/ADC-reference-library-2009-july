/*
	File:		ACInsertWindowController.h

	Abstract:	Implements the Audio Context Insert Window Controller.
				
				This controller manages the UI related to the Audio
				Context Insert. It provides UI to inspect the movie
				and device summary mix, configure an insert's in/out channel
				layouts and hosts an AudioUnit view. The selected AudioUnit
				is used in the insert's processing backend. 
				
	Version:	1.0

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

#import <Cocoa/Cocoa.h>
#import <AudioToolbox/AudioToolbox.h>
#import <AudioUnit/AudioUnit.h>
#import <CoreAudioKit/CoreAudioKit.h>
#import <AudioUnit/AUCocoaUIView.h>

#import "MovieDocument.h"
#import "ACInsertManager.h"
#import "QuickTimeAudioUtils.h" 

@class ACInsert;
@class MovieDocument;

@interface ACInsertWindowController : NSWindowController 
{
    // IB: AU Hosting
    IBOutlet NSPopUpButton			*uiAUPopUpButton;
    IBOutlet NSButton				*uiAUBypassButton;
	IBOutlet NSBox					*uiAUViewContainer;
    	
	// IB: Pre-Insert data format and layout UI 
	IBOutlet NSTextField			*uiPreInsertFormatTextField;
	
	// IB: Insert in/out layout UI
	IBOutlet NSPopUpButton			*uiInsertInputACLPopUpButton;
	IBOutlet NSPopUpButton			*uiInsertOutputACLPopUpButton;
	
	// IB: Post-Insert data format and layout UI
	IBOutlet NSTextField			*uiPostInsertFormatTextField;
	
	IBOutlet NSPopUpButton			*uiInsertDestinationPopUpButton;

    // Post-nib view manufacturing
    NSScrollView					*mScrollView;

	MovieDocument					*mMovieDocument;
    
    // AU Tracking
	Component						*mAUList;
	
	// Insert in/out layout
	int								mCurrentInsertDestinationIndex;
	Boolean							mInsertLayoutPopUpsPopulated;
	int								mCurrentInLayoutTag;
	int								mCurrentOutLayoutTag;
	int								mCurrentAUIndex;
	
	AudioChannelLayout				*mMovieSummaryLayout;
	AudioChannelLayout				*mDeviceLayout;
	AudioChannelLayout				*mTrackLayout;
	AudioStreamBasicDescription		mMovieSummaryASBD;
	AudioStreamBasicDescription		mDeviceASBD;	
	AudioStreamBasicDescription		mTrackASBD;	
	NSMutableString					*mMovieSummaryMixFormatString;
	NSMutableString					*mDeviceFormatString;
	NSMutableString					*mTrackFormatString;
}

	// class method
+ (BOOL)plugInClassIsValid:(Class)pluginClass;

- (MovieDocument *)movieDocument;

	// IB actions
- (IBAction)iaInsertDestinationPopUpButtonPressed:(id)sender;
- (IBAction)iaAUPopUpButtonPressed:(id)sender;
- (IBAction)iaAUBypassButtonPressed:(id)sender;
- (IBAction)iaInsertInputACLPopUpButtonPressed:(id)sender;
- (IBAction)iaInsertOutputACLPopUpButtonPressed:(id)sender;

	// UI updating
- (void)updateUIMovieSummaryMix;
- (void)appendFloatPCMFormatTextFor:(AudioStreamBasicDescription)asbd intoString:(NSMutableString *)formatStr;

- (void)populateAUPopUpWithEffectAUs;
- (NSString*)GetAUNameForEffectComponentAtIndex:(UInt32)index;
-(void)populateInsertDestinationPopUp;

- (void)syncInsertBypassButton;
- (void)showCocoaViewForAU:(AudioUnit)inAU;

- (void)populateInsertLayoutPopUps;
- (void)addToInserLayoutPopUpsItemWithLayoutTag:(AudioChannelLayoutTag)tag andTitle:(NSString*)title;
- (void)updateInsertACLPopUpButtonChoices;

- (void)updateUIDeviceFormat;
- (void)updateUITrackFormat;
- (void)updatePreAndPostInsertFormatTextFields;

	// notification callbacks
- (void)windowWillClose:(NSNotification *)notification;
- (void)movieTracksChanged:(NSNotification *)notification;
- (void)updateTrackEnabledDisabledStatus;

@end
