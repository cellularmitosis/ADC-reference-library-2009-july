//////////
//
//	File:		MyMovieController.h
//
//	Contains:	Public interface file for the MyMovieController class.
//
//	Written by:	Apple Developer Technical Support
//
//	Copyright:	© 2001 by Apple Computer, Inc., all rights reserved.
//
//	Change History (most recent first):
//	   
//	   <1>	 	6/18/01	srk		first file
//
//////////

/*
 IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc. ("Apple") in
 consideration of your agreement to the following terms, and your use, installation, 
 modification or redistribution of this Apple software constitutes acceptance of these 
 terms.  If you do not agree with these terms, please do not use, install, modify or 
 redistribute this Apple software.
 
 In consideration of your agreement to abide by the following terms, and subject to these 
 terms, Apple grants you a personal, non-exclusive license, under Apple’s copyrights in 
 this original Apple software (the "Apple Software"), to use, reproduce, modify and 
 redistribute the Apple Software, with or without modifications, in source and/or binary 
 forms; provided that if you redistribute the Apple Software in its entirety and without 
 modifications, you must retain this notice and the following text and disclaimers in all 
 such redistributions of the Apple Software.  Neither the name, trademarks, service marks 
 or logos of Apple Computer, Inc. may be used to endorse or promote products derived from 
 the Apple Software without specific prior written permission from Apple. Except as expressly
 stated in this notice, no other rights or licenses, express or implied, are granted by Apple
 herein, including but not limited to any patent rights that may be infringed by your 
 derivative works or by other works in which the Apple Software may be incorporated.
 
 The Apple Software is provided by Apple on an "AS IS" basis.  APPLE MAKES NO WARRANTIES, 
 EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED WARRANTIES OF NON-INFRINGEMENT, 
 MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS 
 USE AND OPERATION ALONE OR IN COMBINATION WITH YOUR PRODUCTS.
 
 IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL OR CONSEQUENTIAL 
 DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS 
 OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) ARISING IN ANY WAY OUT OF THE USE, 
 REPRODUCTION, MODIFICATION AND/OR DISTRIBUTION OF THE APPLE SOFTWARE, HOWEVER CAUSED AND 
 WHETHER UNDER THEORY OF CONTRACT, TORT (INCLUDING NEGLIGENCE), STRICT LIABILITY OR 
 OTHERWISE, EVEN IF APPLE HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#import <Cocoa/Cocoa.h>
#import <QuickTime/QuickTime.h>

@interface MyMovieController : NSObject
{
    IBOutlet NSButton *goToBeginningButton;
    IBOutlet NSButton *goToEndButton;
    IBOutlet NSMovieView *movieViewObject;
    IBOutlet NSMenuItem *moviePropertiesMenuItem;
    IBOutlet NSWindow *moviePropertiesWindow;
    IBOutlet NSWindow *movieWindow;
    IBOutlet NSButton *playButton;
    IBOutlet NSSlider *setVolumeSlider;
    IBOutlet NSButton *stepBackButton;
    IBOutlet NSButton *stepForwardButton;
    IBOutlet NSTableView *movieTrackMediaTypes;
    IBOutlet NSMenuItem *menuItem_New;
    
    IBOutlet NSTextField *autoPlayEnabled;
    IBOutlet NSTextField *trackCount;
    IBOutlet NSTextField *movieTimeScale;
    IBOutlet NSTextField *movieDuration;

    NSMutableArray	*movieTrackMediaTypesArray;
    QTCallBackUPP	gMoviePlayingCompleteCallBack;
    QTCallBack		gQtCallBack;
    BOOL		gIsMoviePlaying;
    float 		gMoviePlaybackRate;
    Movie		gMovie;

}

- (IBAction)goToBeginning:(id)sender;
- (IBAction)goToEnd:(id)sender;
- (IBAction)movieProperties:(id)sender;
- (IBAction)newWindow:(id)sender;
- (IBAction)play:(id)sender;
- (IBAction)setVolume:(id)sender;
- (IBAction)stepBack:(id)sender;
- (IBAction)stepForward:(id)sender;

- (int)numberOfRowsInTableView:(NSTableView *)aTableView;
- (id)tableView:(NSTableView *)aTableView
    objectValueForTableColumn:(NSTableColumn *)aTableColumn
    row:(int)rowIndex;
- (void)resetPlayButtonForMovieStopState:(id)sender;
- (void)buildTrackMediaTypesArray:(Movie)qtmovie;
- (void)saveCurrentMoviePlayingState;
- (void)restoreMoviePlayingState:(id)sender;
- (void)showTheWindow:(NSWindow *)window;
- (void)restoreMoviePlayCompleteCallBack;
- (void)setMoviePropertyWindowControlValues:(Movie)qtmovie;
- (void)setupMoviePlayingCompleteCallback:(Movie)theMovie callbackUPP:(QTCallBackUPP) callbackUPP;

@end

pascal void MyCallBackProc (QTCallBack cb, long refcon);
void adjustPlayButtonForMyMovieControllerObject(void);
void saveObjectForCallback(void *theObject);
