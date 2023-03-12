/*

File: VideoWindowController.m of VideoViewer

Author: QuickTime engineering

Change History (most recent first): <1> 05/31/05 initial release

© Copyright 2005 Apple Computer, Inc. All rights reserved.

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

#import "VideoWindowController.h"
#import "VideoDocument.h"
#import "VideoView.h"

#define VIDEO_VIEWER_TEST_SCROLLING 0

@implementation VideoWindowController

- (void)attachToPort
{
    CGrafPtr port = [_videoView quickDrawPort];
    NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
    
    if (port != NULL)
    {
        SetMovieGWorld([[_videoDocument movie] quickTimeMovie], port, NULL);
        [_videoView setPostsFrameChangedNotifications:YES];
        [center addObserver:self selector:@selector(videoViewFrameChanged:) name:NSViewFrameDidChangeNotification object:_videoView];
        [center addObserver:self selector:@selector(videoViewMustDraw:) name:VideoViewMustDrawNotification object:_videoView];
    }
    else
    {
        // The port must not be available yet, so wait for notification to try again
        [center addObserver:self selector:@selector(videoViewCanDraw:) name:VideoViewCanDrawNotification object:_videoView];
    }
}

- (void)setDocument:(NSDocument*)document
{
    _videoDocument = (VideoDocument*)document;
    [super setDocument:document];
}

- (void)windowDidLoad
{
    QTMovie* qtMovie = [_videoDocument movie];
    Movie movie = [qtMovie quickTimeMovie];
    QTVisualContextRef visualContext = [_videoView visualContext];
    NSWindow* window = [_videoView window];
    NSSize size;

    // Resize our window to match the movie's size so that it renders pixel-for-pixel
    [[qtMovie attributeForKey:QTMovieCurrentSizeAttribute] getValue:&size];
    size = [_videoView convertRect:NSMakeRect(0, 0, size.width, size.height) fromView:nil].size;
    [window setContentSize:size];
    
    #if VIDEO_VIEWER_TEST_SCROLLING
    {
        NSScrollView* scroll = [[[NSScrollView alloc] initWithFrame:NSMakeRect(0, 0, size.width, size.height)] autorelease];
        [scroll setHasVerticalScroller:YES];
        [scroll setHasHorizontalScroller:YES];
        [[window contentView] addSubview:scroll];
        [scroll setDocumentView:_videoView];
        [scroll setAutoresizingMask:NSViewWidthSizable | NSViewHeightSizable];
        [_videoView setAutoresizingMask:NSViewMaxXMargin | NSViewMaxYMargin];
    }
    #endif
    
    // Use visual context if available, otherwise fall back to a GWorld
    if (visualContext != NULL)
        SetMovieVisualContext(movie, visualContext);
    else
        [self attachToPort];
    
    PrerollMovie(movie, GetMovieTime(movie, NULL), fixed1);
    MoviesTask(movie, 0);
    [qtMovie play];
}

// If we had failed to get a port before, try again now
- (void)videoViewCanDraw:(NSNotification*)notification
{
    [self attachToPort];
    [[NSNotificationCenter defaultCenter] removeObserver:self name:VideoViewCanDrawNotification object:_videoView];
}

// Since QuickTime performs all rendering when using GWorlds, we need to keep its size in sync with the view
- (void)videoViewFrameChanged:(NSNotification*)notification
{
    NSSize size = [_videoView convertRect:[_videoView bounds] toView:nil].size;
    [[_videoDocument movie] setAttribute: [NSValue value:&size withObjCType:@encode(NSRect)]
                                  forKey: QTMovieCurrentSizeAttribute];
}

// The view is being asked to draw now, so tell the movie to update
- (void)videoViewMustDraw:(NSNotification*)notification
{
    Movie movie = [[_videoDocument movie] quickTimeMovie];
    UpdateMovie(movie);
    MoviesTask(movie, 0);
}

@end
