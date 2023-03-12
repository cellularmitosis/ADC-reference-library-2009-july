/*
	File:		ViewTestsController.m
 
	Originally introduced at WWDC 2004 at
			  Session 214, "Programming QuickTime with Cocoa." 
			  Sample code is explained in detail in 
			  "QTKit Programming Guide" documentation.

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


#import "ViewTestsController.h"

@implementation ViewTestsController

-(void)awakeFromNib
{
	// we want to be notified when the window
	// is closing so we can do cleanup
    [ [NSNotificationCenter defaultCenter]
       addObserver:self selector:@selector(closeWin:)
       name:NSWindowWillCloseNotification object:mViewTestsWindow ];
}

	// returns YES if the movie is currently playing, NO if not
-(BOOL)isPlaying:(QTMovie *)aMovie
{
	if ([aMovie rate] == 0)
	{
		return NO;
	}
	
	return YES;
}

-(void)stopPlayingMovie:(QTMovie *)aMovie
{
	if ([self isPlaying:aMovie] == YES)
	{
		[aMovie stop];
	}
}


- (void)closeWin:(void *)userInfo
{
	// stop any currently playing movies
	[self stopPlayingMovie:[mSplitViewMovieView1 movie]];
	[self stopPlayingMovie:[mSplitViewMovieView2 movie]];
	[self stopPlayingMovie:[mSplitViewMovieView3 movie]];
	[self stopPlayingMovie:[mTabViewMovieView1 movie]];
	[self stopPlayingMovie:[mTabViewMovieView2 movie]];
	[self stopPlayingMovie:[mScrollViewMovieView movie]];

    // clear the movies that were previously set for each view
    [mSplitViewMovieView1 setMovie:NULL];
    [mSplitViewMovieView2 setMovie:NULL];
    [mSplitViewMovieView3 setMovie:NULL];
    [mTabViewMovieView1 setMovie:NULL];
    [mTabViewMovieView2 setMovie:NULL];
    [mScrollViewMovieView setMovie:NULL];

}

#pragma mark --- actions ---

- (QTMovie *)getAMovieFile
{
    NSOpenPanel *openPanel;
	
	openPanel = [NSOpenPanel openPanel];
    [openPanel setCanChooseDirectories:NO];

    if ([openPanel runModalForTypes:[QTMovie movieUnfilteredFileTypes]] == NSOKButton)
	{
        return [QTMovie movieWithFile:[openPanel filename] error:NULL];
	}

	return nil;
}

- (IBAction)doSetMovies:(id)sender
{
    // set the movies
    [mSplitViewMovieView1 setMovie:[self getAMovieFile]];
    [mSplitViewMovieView2 setMovie:[self getAMovieFile]];
    [mSplitViewMovieView3 setMovie:[self getAMovieFile]];
    [mTabViewMovieView1 setMovie:[self getAMovieFile]];
    [mTabViewMovieView2 setMovie:[self getAMovieFile]];
    [mScrollViewMovieView setMovie:[self getAMovieFile]];
}

- (IBAction)doShowViewTestsWindow:(id)sender
{
    // add a toolbar
    if ([mViewTestsWindow toolbar] == nil)
        [mViewTestsWindow setToolbar:[[[NSToolbar alloc] initWithIdentifier:@"QTAudioExtractionPanel"] autorelease]];

    // show the window
    [mViewTestsWindow makeKeyAndOrderFront:nil];
}

@end
