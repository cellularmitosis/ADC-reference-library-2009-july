/*
Project: CIColorTracking

File: VideoHandler.m

Abstract: 
This is the implementation file for VideoHandler, a class that handles showing the tracking movie. 

Version 1.0

Disclaimer: IMPORTANT:  This Apple software is supplied to you by Apple Inc.
("Apple") in consideration of your agreement to the following terms, and your
use, installation, modification or redistribution of this Apple software
constitutes acceptance of these terms.  If you do not agree with these terms,
please do not use, install, modify or redistribute this Apple software.

In consideration of your agreement to abide by the following terms, and subject
to these terms, Apple grants you a personal, non-exclusive license, under
Apple's copyrights in this original Apple software (the "Apple Software"), to
use, reproduce, modify and redistribute the Apple Software, with or without
modifications, in source and/or binary forms; provided that if you redistribute
the Apple Software in its entirety and without modifications, you must retain
this notice and the following text and disclaimers in all such redistributions
of the Apple Software.
Neither the name, trademarks, service marks or logos of Apple Inc. may be used
to endorse or promote products derived from the Apple Software without specific
prior written permission from Apple.  Except as expressly stated in this notice,
no other rights or licenses, express or implied, are granted by Apple herein,
including but not limited to any patent rights that may be infringed by your
derivative works or by other works in which the Apple Software may be
incorporated.

The Apple Software is provided by Apple on an "AS IS" basis.  APPLE MAKES NO
WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED
WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS FOR A PARTICULAR
PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND OPERATION ALONE OR IN
COMBINATION WITH YOUR PRODUCTS.

IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION, MODIFICATION AND/OR
DISTRIBUTION OF THE APPLE SOFTWARE, HOWEVER CAUSED AND WHETHER UNDER THEORY OF
CONTRACT, TORT (INCLUDING NEGLIGENCE), STRICT LIABILITY OR OTHERWISE, EVEN IF
APPLE HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

Copyright (C) 2008 Apple Inc. All Rights Reserved.


*/

#import "VideoHandler.h"

@interface VideoHandler (private)

- (CVReturn)renderTime:(const CVTimeStamp *)timeStamp;

@end

//--------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------

static CVReturn renderCallback(CVDisplayLinkRef displayLink, 
                                                const CVTimeStamp *inNow, 
                                                const CVTimeStamp *inOutputTime, 
                                                CVOptionFlags flagsIn, 
                                                CVOptionFlags *flagsOut, 
                                                void *displayLinkContext)
{
    return [(VideoHandler*)displayLinkContext renderTime:inOutputTime];
}

//--------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------


@implementation VideoHandler

//--------------------------------------------------------------------------------------------------

- (id)initWithView:(NSOpenGLView*)baseView delegate:(id)videoDelegate
{
	CVReturn			    ret;
    OSStatus			    error;

	self = [super init];
	

	/* Create display link */
	CGOpenGLDisplayMask		totalDisplayMask = 0;
	int						virtualScreen;
	long					displayMask;
	
	NSOpenGLPixelFormat	*openGLPixelFormat = [baseView pixelFormat];
	// Start the view on the main display.
	viewDisplayID = (CGDirectDisplayID)[[[[[baseView window] screen] deviceDescription] objectForKey:@"NSScreenNumber"] intValue]; 
	
	// Build a list of displays from the OpenGL pixel format.
	for (virtualScreen = 0; virtualScreen < [openGLPixelFormat  numberOfVirtualScreens]; virtualScreen++)
	{
		[openGLPixelFormat getValues:&displayMask forAttribute:NSOpenGLPFAScreenMask forVirtualScreen:virtualScreen];
		totalDisplayMask |= displayMask;
	}
	ret = CVDisplayLinkCreateWithOpenGLDisplayMask(totalDisplayMask, &displayLink);
	[[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(windowChangedScreen:) name:NSWindowDidMoveNotification object:nil];
	
	// Set up callbacks for the display link.
	CVDisplayLinkSetOutputCallback(displayLink, renderCallback, self);

	// Create a visual context.
	NSDictionary	    *attributes = nil;
    attributes = [NSDictionary dictionaryWithObjectsAndKeys:[NSDictionary dictionaryWithObjectsAndKeys:[NSNumber numberWithFloat:720.0], kQTVisualContextTargetDimensions_WidthKey, 
													[NSNumber numberWithFloat:480.0], kQTVisualContextTargetDimensions_HeightKey, nil], 
							    kQTVisualContextTargetDimensionsKey, 
							    [NSDictionary dictionaryWithObjectsAndKeys:[NSNumber numberWithFloat:720.0], kCVPixelBufferWidthKey, 
													[NSNumber numberWithFloat:480.0], kCVPixelBufferHeightKey, nil], 
							    kQTVisualContextPixelBufferAttributesKey,
							    nil];
    error = QTOpenGLTextureContextCreate(NULL, 
										(CGLContextObj)[[baseView openGLContext] CGLContextObj],
										(CGLPixelFormatObj)[[baseView pixelFormat] CGLPixelFormatObj],
										(CFDictionaryRef)attributes, &qtVisualContext);

	view = baseView;
	delegate = videoDelegate;
	return self;
}

//--------------------------------------------------------------------------------------------------

- (void)dealloc
{
	[[NSNotificationCenter defaultCenter] removeObserver:self];
	CVDisplayLinkRelease(displayLink);
	QTVisualContextRelease(qtVisualContext);
	[movie release];
	[super dealloc];
}

//--------------------------------------------------------------------------------------------------

- (void)windowChangedScreen:(NSNotification*)inNotification
{
	// If the video moves to a different screen, synchronize to the timing of that screen.
    NSWindow *window = [inNotification object]; 
    CGDirectDisplayID displayID = (CGDirectDisplayID)[[[[window screen] deviceDescription] objectForKey:@"NSScreenNumber"] intValue];

    if((displayID != NULL) && (viewDisplayID != displayID))
    {
		CVDisplayLinkSetCurrentCGDisplay(displayLink, displayID);
		viewDisplayID = displayID;
    }
}

//--------------------------------------------------------------------------------------------------

- (BOOL)loadMovieFromPath:(NSString*)moviePath
{
    NSError		*error;
        
    [movie release];
    movie = [[QTMovie movieWithFile:moviePath error:&error] retain];
    if(movie)
    {
		OSStatus status;
		status = SetMovieVisualContext([movie quickTimeMovie],qtVisualContext);
		SetMoviePlayHints([movie quickTimeMovie],hintsHighQuality, hintsHighQuality);	
		// set Movie to loop
		[movie setAttribute:[NSNumber numberWithBool:YES] forKey:QTMovieLoopsAttribute];					
		[movie gotoBeginning];
		MoviesTask([movie quickTimeMovie], 0);	//QTKit is not doing this automatically
		movieDuration = [[[movie movieAttributes] objectForKey:QTMovieDurationAttribute] QTTimeValue];
		[self renderTime:nil];
	}

	return error != nil;
}

//--------------------------------------------------------------------------------------------------

- (IBAction)togglePlayback
{
    if(CVDisplayLinkIsRunning(displayLink))
    {
		CVDisplayLinkStop(displayLink);
		[movie stop];
    } else {
		[movie play];
		CVDisplayLinkStart(displayLink);
    }
}

//--------------------------------------------------------------------------------------------------

- (void)scrubMovie:(double)progress
{
    if(CVDisplayLinkIsRunning(displayLink))
		[self togglePlayback];

    // Get the movie time.
    QTTime currentTime;
        
    currentTime.timeValue = (long long)((double)movieDuration.timeValue * progress);
    currentTime.timeScale = movieDuration.timeScale;
    currentTime.flags = 0;
        
    [movie setCurrentTime:currentTime];
	[self renderTime:nil];
}

//--------------------------------------------------------------------------------------------------

- (BOOL)isPlaying
{
	return CVDisplayLinkIsRunning(displayLink);
}

//--------------------------------------------------------------------------------------------------

- (BOOL)getFrameForTime:(const CVTimeStamp *)timeStamp
{
    OSStatus error = noErr;

    // Check to see whether a new frame is available.

    QTVisualContextTask(qtVisualContext);
	if(QTVisualContextIsNewImageAvailable(qtVisualContext,timeStamp))
	{	    
		CVOpenGLTextureRelease(currentFrame);
		QTVisualContextCopyImageForTime(qtVisualContext,
			NULL,
			timeStamp,
			&currentFrame);
				
		// In general this shouldn't happen, but just in case, handle the error.
		if(error != noErr && !currentFrame)
		{
			NSLog(@"QTVisualContextCopyImageForTime: %ld\n",error);
			return NO;
		}
		return YES;
	} 
    return NO;
}

//--------------------------------------------------------------------------------------------------

- (CVReturn)renderTime:(const CVTimeStamp *)timeStamp
{
	// This method is called by the Core Video diplay link render thread. 
	
    CVReturn rv = kCVReturnError;
    NSAutoreleasePool *pool;
    
    // Use a local autorelease pool because this gets called from a separate thread.
    pool = [[NSAutoreleasePool alloc] init]; 
    if([self getFrameForTime:timeStamp])
    {
		QTTime  currentTime = [movie currentTime];
		
		[delegate renderFrame:currentFrame progress:(double)currentTime.timeValue / (double)movieDuration.timeValue];

		rv = kCVReturnSuccess;
    } else {
		rv = kCVReturnError;
    }
    [pool release];
    return rv;
}

//--------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------

@end
