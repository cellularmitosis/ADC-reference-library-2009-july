/*

File: AppController.m

Abstract: Implements the AppController class used to control this demo
application. The controller takes care of creating the OpenGL context,
attaching it to the destination NSView in the application's window, and
creating a QCRenderer on that OpenGL context with the Quartz Composer
composition file selected by the user. The controller also creates an
instance of the OpenGLDVExporter class to outputs the contents of the
OpenGL context as a real-time DV stream on the FireWire port. Both the
QCRenderer and the OpenGLDVExporter are driven from a NSTimer that runs
at the NTSC or PAL framerate.

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

#import <OpenGL/CGLMacro.h>

#import "AppController.h"

@implementation AppController

- (void) _clearGLContext
{
	CGLContextObj			cgl_ctx = [_glContext CGLContextObj]; //By using CGLMacro.h there's no need to set the current OpenGL context
	
	glClearColor(0.0, 0.0, 0.0, 0.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	[_glContext flushBuffer];
}

- (BOOL) _startExport
{
	DVFormat			format = ([formatMatrix selectedColumn] == 0 ? kDVFormat_NTSC : kDVFormat_PAL);
	
	//Create DV exporter
	_exporter = [[OpenGLDVExporter alloc] initWithOpenGLContext:_glContext dvFormat:format flipVertically:YES useTextures:[texturesButton state]];
	if(_exporter == nil)
	return NO;
	
	//Create a timer which will regularly call our rendering method
	_renderTimer = [[NSTimer timerWithTimeInterval:(1.0 / [OpenGLDVExporter framerateForFormat:format]) target:self selector:@selector(_renderExport:) userInfo:nil repeats:YES] retain];
	[[NSRunLoop currentRunLoop] addTimer:_renderTimer forMode:NSDefaultRunLoopMode];
	[[NSRunLoop currentRunLoop] addTimer:_renderTimer forMode:NSModalPanelRunLoopMode];
	[[NSRunLoop currentRunLoop] addTimer:_renderTimer forMode:NSEventTrackingRunLoopMode];
	_startTime = -1.0;
	_fps = 0.0;
	
	return YES;
}

- (void) _renderExport:(NSTimer*)timer
{
	NSTimeInterval			time = [NSDate timeIntervalSinceReferenceDate];
	
	//Compute the local time
	if(_startTime < 0.0) {
		_startTime = time;
		_lastTime = time;
	}
	time = time - _startTime;
	
	//Render a frame from the composition
	[_renderer renderAtTime:time arguments:nil];
	
	//Export frame to DV
	[_exporter exportFrame];
	
	//Display frame on screen
	[_glContext flushBuffer];
	
	//Update FPS field
	_fps = 0.9 * _fps + 0.1 * 1.0 / (time - _lastTime);
	_lastTime = time;
	[fpsField setObjectValue:[NSNumber numberWithFloat:_fps]];
}

- (void) _stopExport
{
	//Destroy timer
	[_renderTimer invalidate];
	[_renderTimer release];
	_renderTimer = nil;
	
	//Destroy DV exporter
	[_exporter release];
	_exporter = nil;
	
	//Clear rendering view
	[self _clearGLContext];
	
	//Clear FPS field
	[fpsField setStringValue:@"n/a"];
}

- (void) applicationDidFinishLaunching:(NSNotification*)notification
{
	NSOpenGLPixelFormatAttribute	attributes[] = {NSOpenGLPFAAccelerated, NSOpenGLPFANoRecovery, NSOpenGLPFADoubleBuffer, NSOpenGLPFADepthSize, 24, 0};
	
	//Create the OpenGL context used to render the composition and attach it to the rendering view
	_glPixelFormat = [[NSOpenGLPixelFormat alloc] initWithAttributes:attributes];
	_glContext = [[NSOpenGLContext alloc] initWithFormat:_glPixelFormat shareContext:nil];
	[_glContext setView:renderView];
	[self _clearGLContext];
	
	//Prompt the user for a Quartz Composer composition immediately
	[self openComposition:nil];
}

- (IBAction) openComposition:(id)sender
{
	NSOpenPanel*					openPanel = [NSOpenPanel openPanel];
	
	//Prompt the user for a Quartz Composer composition file
	[openPanel setAllowsMultipleSelection:NO];
	[openPanel setCanChooseDirectories:NO];
	[openPanel setCanChooseFiles:YES];
	if([openPanel runModalForDirectory:nil file:nil types:[NSArray arrayWithObject:@"qtz"]] == NSOKButton) {
		//Destroy the current QCRenderer and create a new one
		[_renderer release];
		_renderer = [[QCRenderer alloc] initWithOpenGLContext:_glContext pixelFormat:_glPixelFormat file:[openPanel filename]];
		
		//Start DV export if necessary
		if(_exporter == nil)
		[streamButton setState:[self _startExport]];
	}
}

- (IBAction) toggleDVStream:(id)sender
{
	//Start or stop DV exporter
	if([streamButton state]) {
		if(![self _startExport]) {
			[streamButton setState:NSOffState];
			NSBeep();
		}
	}
	else
	[self _stopExport];
}

- (IBAction) toggleTextures:(id)sender
{
	//Restart DV exporter
	if(_exporter) {
		[self _stopExport];
		[self _startExport];
	}
}

- (IBAction) setDVFormat:(id)sender
{
	CGLContextObj			cgl_ctx = [_glContext CGLContextObj]; //By using CGLMacro.h there's no need to set the current OpenGL context
	DVFormat				format = ([formatMatrix selectedColumn] == 0 ? kDVFormat_NTSC : kDVFormat_PAL);
	NSSize					newSize = [OpenGLDVExporter sizeForFormat:format];
	NSSize					oldSize = [renderView frame].size;
	NSRect					frame;
	
	//Resize rendering view according to DV format dimensions
	if(!NSEqualSizes(newSize, oldSize)) {
		//Resize window (this will autoresize the rendering view as well)
		frame = [[renderView window] frame];
		frame.origin.y = frame.origin.y - (newSize.height - oldSize.height);
		frame.size.height += newSize.height - oldSize.height;
		[[renderView window] setFrame:frame display:YES animate:NO];
		
		//Notify the OpenGL context its rendering view has changed
		[_glContext update];
		
		//Update the OpenGL viewport
		glViewport(0, 0, newSize.width, newSize.height);
		
		//Clear GL context
		[self _clearGLContext];
	}
	
	//Restart DV exporter
	if(_exporter) {
		[self _stopExport];
		[self _startExport];
	}
}

- (BOOL) windowShouldClose:(id)sender
{
	//Quits the app when the window is closed
	[NSApp terminate:self];
	
	return YES;
}

- (void) applicationWillTerminate:(NSNotification*)notification
{
	//Stop the DV exporter
	[self _stopExport];
}

- (void) dealloc
{
	//Release our objects
	[_renderer release];
	[_glContext release];
	[_glPixelFormat release];
	
	[super dealloc];
}

@end
