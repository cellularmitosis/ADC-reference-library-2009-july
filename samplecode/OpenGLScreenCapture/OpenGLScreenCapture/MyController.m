/*

File: MyController.m

Abstract: Implementation for the MyController class. This class performs
program initialization and responds to the program user interface (UI). 
At startup it will create an OpenGL context and Core Video display link 
which will be used to drive screen capture. It is responsible for initiating
actual screen capture based on the user selection (for the type of capture).

Version: 1.0

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

Copyright (C) 2007 Apple Inc. All Rights Reserved.

*/

#import "MyController.h"
#import "FrameMovieExporter.h"

// Duration (in seconds) for the movie screen capture
#define kMovieCaptureDuration		10.0

// Number of reader objects used by the program at once.
// Each reader object is designed to read and hold a 
// single screen frame buffer.
#define kNumReaderObjects		20


// This is the CoreVideo display link callback. The display link invokes
// this callback whenever it wants you to output a frame. In our case, 
// we call our displayLinkCallback to perform readback of the screen using
// OpenGL
static CVReturn MyRenderCallback(CVDisplayLinkRef displayLink, 
					 const CVTimeStamp *inNow, 
					 const CVTimeStamp *inOutputTime, 
					 CVOptionFlags flagsIn, 
					 CVOptionFlags *flagsOut, 
					void *displayLinkContext)
{
	return [(MyController *)displayLinkContext displayLinkCallback:inOutputTime flagsOut:flagsOut];
}


@implementation MyController

#pragma mark ---------- Initialization/Termination ----------

// Setup notifications to let us know when application is finished
// launching so we can use this time to create the OpenGL context
// used to render, and to let us know when the app. is terminating
// so  we can perform cleanup
-(id)init
{
	if (self = [super init])
	{
		mGLContext = nil;
		mExporterObj = nil;
		
		[[NSNotificationCenter defaultCenter] addObserver:self
			selector:@selector(applicationDidFinishLaunching:)
			name:@"NSApplicationDidFinishLaunchingNotification" object:NSApp];

		[[NSNotificationCenter defaultCenter] addObserver:self
			selector:@selector(applicationWillTerminate:)
			name:@"NSApplicationWillTerminateNotification" object:NSApp];
	}
	
	return self;
}

// Perform cleanup when the application terminates
- (void) applicationWillTerminate:(NSNotification*)notification
{
	// Cancel render timer
	if (mRenderDurationTimer)
	{
		[mRenderDurationTimer invalidate];
		[mRenderDurationTimer release];
	}

	// Cancel any current renderings
	[self readTimerExpired:nil];
}

// Create OpenGL context used to render
- (void) applicationDidFinishLaunching:(NSNotification*)notification
{
	NSOpenGLPixelFormatAttribute attributes[] = {
			NSOpenGLPFAFullScreen,
			NSOpenGLPFAScreenMask,
				CGDisplayIDToOpenGLDisplayMask(kCGDirectMainDisplay),
			(NSOpenGLPixelFormatAttribute) 0
	};

	mGLPixelFormat = [[NSOpenGLPixelFormat alloc] initWithAttributes:attributes];
	NSAssert( mGLPixelFormat != nil, @"No Full-Screen Renderer");
	if (!mGLPixelFormat) return;

	//Create OpenGL context used to render
	mGLContext = [[NSOpenGLContext alloc] initWithFormat:mGLPixelFormat shareContext:nil];
	NSAssert( mGLContext != nil, @"NSOpenGLContext initialization failure");	
	[mGLContext makeCurrentContext];
	[mGLContext setFullScreen];

	CGDirectDisplayID displayID = CGMainDisplayID();
	NSAssert( displayID != nil, @"CGMainDisplayID failure");
	if (displayID)
	{
		mDisplayRect = CGDisplayBounds(displayID);
	}
}

// Called when our screen reader timer expires to let
// us perform cleanup after recording frames to a 
// movie.
- (void) readTimerExpired:(NSTimer*)timer
{
	// Stop CVDisplayLink to prevent
	// more frames from being read
	if (mDisplayLink)
	{
		CVDisplayLinkStop(mDisplayLink);
		CVDisplayLinkRelease(mDisplayLink);
		mDisplayLink = NULL;
	}

	// Stop current export
	
	// Free our reader and exporter 
	// objects
	if (mExporterObj)
	{
		[mExporterObj release];
		mExporterObj = nil;
	}

	// Free our queue controller
	if (mFrameQueueController)
	{
		[mFrameQueueController release];
		mFrameQueueController = nil;
	}
	
	[mMovieRecordingFinishedWindow makeKeyAndOrderFront:self];
}

#pragma mark -------- Reader --------

// Called from our display link callback.
// This routine will attempt to get an available reader object
// to initiate a screen grab operation (to fill the object's buffer).
// It then checks to see if any reader objects have indeed been 
// filled (a screen grab operation has completed and the object's 
// buffer is filled) and if so it passes the reader object to the 
// exporter/compressor object so it can be compressed and the frame
// added to the movie.
- (void) readAndCompressFrames
{
	//Compute the local time
	if(mStartTime == 0.0)
	{
		NSTimeInterval	time = [NSDate timeIntervalSinceReferenceDate];
		mStartTime = time;
	}

	// Get an available reader object from the reader "free" queue
	FrameReader *freeReaderObj = [mFrameQueueController removeOldestItemFromFreeQ];
	if (freeReaderObj)
	{
		[freeReaderObj setBufferReadTime:mStartTime];
		// pass object to FrameReader and do a read operation
		// this call spawns a new thread to do the read
		[freeReaderObj readScreenAsyncOnSeparateThread];
	}

	// Compress any available frames
	
	// see if there are available frames in the "filled" queue
	FrameReader *filledReaderObj = [mFrameQueueController removeOldestItemFromFilledQ];
	if (filledReaderObj)
	{
		// compress the frame and add it to our movie
		[mExporterObj exportFrame:filledReaderObj];
	}
}

#pragma mark -------- Display Link --------

// This is called from the Display Link callback.
// We'll use this callback to read/compress our frames.
- (CVReturn)displayLinkCallback:(const CVTimeStamp*)timeStamp flagsOut:(CVOptionFlags*)flagsOut
{
	// there is no autorelease pool when this method is called because it will be called from another thread
	// it's important to create one or you will leak objects
	NSAutoreleasePool *pool = [NSAutoreleasePool new];

        // Each iteration we will attemp to read the screen into a buffer (if
        // one is available), compress the buffer contents, then add the 
        // compressed frame to a movie
	[self readAndCompressFrames];

	[pool release];

	return kCVReturnSuccess;
}

#pragma mark ---------- Action Methods ----------


// Called to initiate capture of the screen for a timed interval
-(IBAction)captureScreenAsMovie
{
	NSSavePanel*				savePanel = [NSSavePanel savePanel];
	ICMCompressionSessionOptionsRef	options;
	CodecType					codec;
	double					framerate;

	[mMovieRecordingFinishedWindow close];

	// first ask user where to save movie file
	[savePanel setRequiredFileType:@"mov"];
	[savePanel setCanCreateDirectories:YES];
	[savePanel setCanSelectHiddenExtension:YES];
	if(([savePanel runModalForDirectory:[@"~/Desktop" stringByExpandingTildeInPath] file:@"Screen Capture.mov"] == NSOKButton) && (options = [FrameCompressor userOptions:&codec frameRate:&framerate autosaveName:@"CompressionDialogSettings"])) 
	{
		NSNumber *widthNum,*heightNum;
		widthNum = [NSNumber numberWithFloat:mDisplayRect.size.width];
		heightNum = [NSNumber numberWithFloat:mDisplayRect.size.height];

		// make an exporter object
		mExporterObj = [[FrameMovieExporter alloc] initWithPath:[savePanel filename] codec:codec pixelsWide:[widthNum unsignedIntValue] pixelsHigh:[heightNum unsignedIntValue] options:options];

		// make a frame queue controller, which will create and manage the
		// underlying set of (multiple) frame reader objects
		mFrameQueueController = [[QueueController alloc] initWithReaderObjects:
										kNumReaderObjects	// create this many frame reader objects
										aContext:mGLContext 
										pixelsWide:[widthNum unsignedIntValue] 
										pixelsHigh:[heightNum unsignedIntValue] ];

		//Create countdown timer which will specify rendering to stop after 
		//kMovieCaptureDuration seconds have expired
		mRenderDurationTimer = [[NSTimer timerWithTimeInterval:kMovieCaptureDuration target:self selector:@selector(readTimerExpired:) userInfo:nil repeats:NO] retain];
		if (NULL != mRenderDurationTimer)
		{
			[[NSRunLoop currentRunLoop] addTimer:mRenderDurationTimer forMode:NSDefaultRunLoopMode];
			[[NSRunLoop currentRunLoop] addTimer:mRenderDurationTimer forMode:NSModalPanelRunLoopMode];
			[[NSRunLoop currentRunLoop] addTimer:mRenderDurationTimer forMode:NSEventTrackingRunLoopMode];
		}

		mStartTime = 0.0;

		// Create a timer which will regularly call our rendering method

		// create display link for the main display
		CVDisplayLinkCreateWithCGDisplay(kCGDirectMainDisplay, &mDisplayLink);
		if (NULL != mDisplayLink) 
		{
			// set the current display of a display link.
			CVDisplayLinkSetCurrentCGDisplay(mDisplayLink, kCGDirectMainDisplay);
			
			// set the renderer output callback function
			CVDisplayLinkSetOutputCallback(mDisplayLink, &MyRenderCallback, self);
			
			// activates a display link.
			CVDisplayLinkStart(mDisplayLink);
		}
	}
}

// Displays the capture movie window for the user
- (IBAction)presentCaptureScreenToMovieWindow:(id)sender
{
	// show movie capture window
	[mMovieCaptureWindow center];
	[mMovieCaptureWindow makeKeyAndOrderFront:self];
}

// Called if the user presses the "Cancel" button in
// the capture movie window
- (IBAction)captureScreenToMovieCancelButton:(id)sender
{
	// pressing cancel button in movie capture window
	// simply closes the window
	[mMovieCaptureWindow close];
}

// Called if the user presses the "OK" button in
// the capture movie window
- (IBAction)captureScreenToMovieOKButton:(id)sender
{
	// pressing ok button in movie capture window
	// closes the window and starts the actual capture
	[mMovieCaptureWindow close];
	[self captureScreenAsMovie];
}

// Called if the user presses the "OK" button in
// the window displayed when the screen capture
// has completed
- (IBAction)movieRecordingFinishedOKButton:(id)sender
{
	[mMovieRecordingFinishedWindow close];
}


@end
