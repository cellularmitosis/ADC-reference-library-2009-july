/*

File: main.m

Abstract: A demonstration on how to use a Quartz Composer composition
in combination with QuickTime 7 APIs to display a movie as a series
of frames in a 3D world.

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

Copyright © 2005 Apple Computer, Inc., All Rights Reserved

*/

#import <Cocoa/Cocoa.h>
#import <Quartz/Quartz.h>
#import <QuickTime/QuickTime.h>

#define kRendererEventMask (NSLeftMouseDownMask | NSLeftMouseDraggedMask | NSLeftMouseUpMask | NSRightMouseDownMask | NSRightMouseDraggedMask | NSRightMouseUpMask | NSOtherMouseDownMask | NSOtherMouseUpMask | NSOtherMouseDraggedMask | NSKeyDownMask | NSKeyUpMask | NSFlagsChangedMask | NSScrollWheelMask | NSTabletPointMask | NSTabletProximityMask)
#define kRendererFPS 60.0
#define kFramesCacheMaxSize 10
#define kFramesCacheInputKey @"Frames"

@interface MatrixApplication : NSApplication
{
	NSOpenGLContext*			_openGLContext;
	QCRenderer*					_renderer;
	NSString*					_filePath;
	NSTimer*					_renderTimer;
	NSTimeInterval				_startTime;
	NSSize						_screenSize;
	Movie						_movie;
	QTVisualContextRef			_visualContext;
	NSMutableArray*				_framesCache;
}
@end

@implementation MatrixApplication

- (id) init
{
	//We need to be our own delegate
	if(self = [super init])
	[self setDelegate:self];
	
	return self;
}

- (BOOL) application:(NSApplication*)theApplication openFile:(NSString*)filename
{
	//Let's remember the file for later
	_filePath = [filename retain];
	
	return YES;
}

- (void) applicationDidFinishLaunching:(NSNotification*)aNotification 
{
	Boolean							active = TRUE;
	QTNewMoviePropertyElement		properties[] = {
														{kQTPropertyClass_DataLocation, kQTDataLocationPropertyID_CFStringNativePath, sizeof(CFStringRef), &_filePath, 0},
														{kQTPropertyClass_NewMovieProperty, kQTNewMoviePropertyID_Active, sizeof(Boolean), &active, 0},
														{kQTPropertyClass_Context, kQTContextPropertyID_VisualContext, sizeof(QTVisualContextRef), &_visualContext, 0}
									};
	long							value = 1;
	NSOpenGLPixelFormatAttribute	attributes[] = {
														NSOpenGLPFAFullScreen,
														NSOpenGLPFAScreenMask, CGDisplayIDToOpenGLDisplayMask(kCGDirectMainDisplay),
														NSOpenGLPFANoRecovery,
														NSOpenGLPFADoubleBuffer,
														NSOpenGLPFAAccelerated,
														NSOpenGLPFADepthSize, 24,
														(NSOpenGLPixelFormatAttribute) 0
													};
	NSOpenGLPixelFormat*			format = [[[NSOpenGLPixelFormat alloc] initWithAttributes:attributes] autorelease];
	NSOpenPanel*					openPanel;
	OSStatus						error;
	
	//If no movie file was dropped on the application's icon, we need to ask the user for one
	if(_filePath == nil) {
		openPanel = [NSOpenPanel openPanel];
		[openPanel setAllowsMultipleSelection:NO];
		[openPanel setCanChooseDirectories:NO];
		[openPanel setCanChooseFiles:YES];
		if([openPanel runModalForDirectory:nil file:nil types:[NSArray arrayWithObject:@"mov"]] != NSOKButton) {
			NSLog(@"No movie file specified");
			[NSApp terminate:nil];
		}
		_filePath = [[openPanel filename] retain];
	}
	
	//Capture the main screen and cache its dimensions
	CGDisplayCapture(kCGDirectMainDisplay);
	_screenSize.width = CGDisplayPixelsWide(kCGDirectMainDisplay);
	_screenSize.height = CGDisplayPixelsHigh(kCGDirectMainDisplay);
	
	//Create the fullscreen OpenGL context on the main screen (double-buffered with color and depth buffers)
	_openGLContext = [[NSOpenGLContext alloc] initWithFormat:format shareContext:nil];
	if(_openGLContext == nil) {
		NSLog(@"Cannot create OpenGL context");
		[NSApp terminate:nil];
	}
	[_openGLContext setFullScreen];
	[_openGLContext setValues:&value forParameter:kCGLCPSwapInterval];
	
	//Create a QuickTime visual context with that OpenGL context and open the movie on it
	error = QTOpenGLTextureContextCreate(NULL, [_openGLContext CGLContextObj], [format CGLPixelFormatObj], NULL, &_visualContext);
	if(error == noErr)
	error = NewMovieFromProperties(sizeof(properties) / sizeof(QTNewMoviePropertyElement), properties, 0, NULL, &_movie);
	if(error != noErr) {
		NSLog(@"Cannot open movie");
		[NSApp terminate:nil];
	}
	SetMoviePlayHints(_movie, hintsHighQuality, hintsHighQuality);
	SetTimeBaseFlags(GetMovieTimeBase(_movie), loopTimeBase);
	GoToBeginningOfMovie(_movie);
	
	//Create movie frames cache
	_framesCache = [NSMutableArray new];
	
	//Create the QuartzComposer Renderer with that OpenGL context and the predefined composition file
	_renderer = [[QCRenderer alloc] initWithOpenGLContext:_openGLContext pixelFormat:format file:[[NSBundle mainBundle] pathForResource:@"Matrix" ofType:@"qtz"]];
	if(_renderer == nil) {
		NSLog(@"Cannot create QCRenderer");
		[NSApp terminate:nil];
	}
	
	//Create a timer which will regularly call our rendering method
	_renderTimer = [[NSTimer scheduledTimerWithTimeInterval:(1.0 / (NSTimeInterval)kRendererFPS) target:self selector:@selector(_render:) userInfo:nil repeats:YES] retain];
	if(_renderTimer == nil) {
		NSLog(@"Cannot create NSTimer");
		[NSApp terminate:nil];
	}
	
	//Start playing movie
	StartMovie(_movie);
}

- (void) renderWithEvent:(NSEvent*)event
{
	NSTimeInterval			time = [NSDate timeIntervalSinceReferenceDate];
	NSPoint					mouseLocation;
	NSMutableDictionary*	arguments;
	CVOpenGLTextureRef		imageBuffer;
	
	//Let's compute our local time
	if(_startTime == 0) {
		_startTime = time;
		time = 0;
	}
	else
	time -= _startTime;
	
	//We set up the arguments to pass to the composition (normalized mouse coordinates and an optional event)
	mouseLocation = [NSEvent mouseLocation];
	mouseLocation.x /= _screenSize.width;
	mouseLocation.y /= _screenSize.height;
	arguments = [NSMutableDictionary dictionaryWithObject:[NSValue valueWithPoint:mouseLocation] forKey:QCRendererMouseLocationKey];
	if(event)
	[arguments setObject:event forKey:QCRendererEventKey];
	
	//Give some idle time to QuickTime
	MoviesTask(_movie, 0);
	QTVisualContextTask(_visualContext);
	
	//Check if a new frame is available from the movie
	if(QTVisualContextIsNewImageAvailable(_visualContext, NULL) && (QTVisualContextCopyImageForTime(_visualContext, NULL, NULL, &imageBuffer) == kCVReturnSuccess)) {
		//Update movie frames cache
		[_framesCache insertObject:(id)imageBuffer atIndex:0];
		if([_framesCache count] > kFramesCacheMaxSize)
		[_framesCache removeLastObject];
		CVOpenGLTextureRelease(imageBuffer);
		
		//Pass updated movie frames cache to the composition
		if(![_renderer setValue:_framesCache forInputKey:kFramesCacheInputKey])
		NSLog(@"Could not pass frames cache to composition");
	}
	
	//Render a frame from the composition
	if(![_renderer renderAtTime:time arguments:arguments])
	NSLog(@"Rendering failed at time %.3fs", time);
	
	//Flush the OpenGL context to display the frame on screen
	[_openGLContext flushBuffer];
}

- (void) _render:(NSTimer*)timer
{
	//Simply call our rendering method, passing no event to the composition
	[self renderWithEvent:nil];
}

- (void) sendEvent:(NSEvent*)event
{
	//If the user pressed the [Esc] key, we need to exit
	if(([event type] == NSKeyDown) && ([event keyCode] == 0x35))
	[NSApp terminate:nil];
	
	//If the renderer is active and we have a meaningful event, render immediately passing that event to the composition
	if(_renderer && (NSEventMaskFromType([event type]) & kRendererEventMask))
	[self renderWithEvent:event];
	else
	[super sendEvent:event];
}

- (void) applicationWillTerminate:(NSNotification*)aNotification 
{
	//Stop the timer
	[_renderTimer invalidate];
	[_renderTimer release];
	
	//Destroy the renderer
	[_renderer release];
	
	//Destroy movie frames cache
	[_framesCache release];
	
	//Destroy the movie
	if(_movie) {
		StopMovie(_movie);
		DisposeMovie(_movie);
	}
	if(_visualContext)
	QTVisualContextRelease(_visualContext);
	
	//Destroy the OpenGL context
	[_openGLContext clearDrawable];
	[_openGLContext release];
	
	//Release main screen
	if(CGDisplayIsCaptured(kCGDirectMainDisplay))
	CGDisplayRelease(kCGDirectMainDisplay);
	
	//Release path
	[_filePath release];
}

@end

int main(int argc, const char *argv[])
{
    //Initialize QuickTime
	EnterMovies();
	
	//Start application
	return NSApplicationMain(argc, argv);
}
