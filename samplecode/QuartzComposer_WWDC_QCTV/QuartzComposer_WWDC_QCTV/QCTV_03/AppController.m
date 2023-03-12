/*

File: AppController.m

Abstract: Implements the AppController class used to control this demo
application. The controller takes care of creating the OpenGL context,
attaching it to the destination NSView in the application's window, and
creating the several QCRenderers on that OpenGL context to render the
Quartz Composer compositions. The controller also takes care of creating 
the user-interface for editing the composition parameters and saving /
restoring them from the user defaults. Eventually, the controller handles
the export of the final rendering as a QuickTime movie or a FireWire DV
stream.

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

#import "FrameLiveDVExporter.h"
#import "FrameMovieExporter.h"
#import "AppController.h"

#define __RENDER_WHILE_RESIZING__ 0

enum {
	kExport_None,
	kExport_QuickTimeMovie,
	kExport_FireWireDV
};

#define kDisplayFramerate			60.0

static NSString*					_compositionNames[] = {@"Background", @"Image", @"Contents", @"Title", @"Crawler", @"InfoBox", @"Heading", nil};

@implementation AppController

+ (void) initialize
{
	//Allow the user to pick colors with alpha
	[NSColor setIgnoresAlpha:NO];
}

- (void) _reloadRenderers
{
	NSBundle*						bundle = [NSBundle mainBundle];
	NSRect							frame = [parametersPanel frame];
	unsigned						index = 0;
	NSString*						path;
	QCRenderer*						renderer;
	NSDictionary*					parameters;
	float							height;
	NSAutoreleasePool*				pool;
	
	//Save parameters
	parameters = [_settingsView parameters:NO];
	
	//Destroy all QCRenderers - Use a temporary autorelease pool to make sure they are destroyed immediately and avoid resource conflicts when we recreate them
	pool = [NSAutoreleasePool new];
	[_settingsView removeAllRenderers];
	[pool release];
	
	//Create QCRenderers from composition files
	while(_compositionNames[index] != nil) {
		path = [bundle pathForResource:_compositionNames[index] ofType:@"qtz"];
		renderer = [[QCRenderer alloc] initWithOpenGLContext:_glContext pixelFormat:_glPixelFormat file:path];
		if(renderer) {
			[_settingsView addRenderer:renderer title:_compositionNames[index]];
			[renderer release];
		}
		else
		NSLog(@"Failed loading composition \"%@\"", _compositionNames[index]);
		index += 1;
	}
	
	//Update settings panel
	[_settingsView setFrameSize:[_settingsView bestSize]];
	height = MIN(frame.size.height, [_settingsView bestSize].height + 20);
	frame.origin.y = frame.origin.y + frame.size.height - height;
	frame.size.width = [_settingsView bestSize].width + 10;
	frame.size.height = height;
	[parametersPanel setMinSize:NSMakeSize(frame.size.width, 0)];
	[parametersPanel setMaxSize:NSMakeSize(frame.size.width, [_settingsView bestSize].height + 20)];
	[parametersPanel setFrame:frame display:YES];
	
	//Restore parameters
	[_settingsView setParameters:parameters];
}

- (void) _clearGLContext
{
	CGLContextObj			cgl_ctx = [_glContext CGLContextObj]; //By using CGLMacro.h there's no need to set the current OpenGL context
	
	//Paint OpenGL context in black
	glClearColor(0.0, 0.0, 0.0, 0.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	//Display context on screen
	[_glContext flushBuffer];
}

- (NSToolbarItem*) toolbar:(NSToolbar*)toolbar itemForItemIdentifier:(NSString*)itemIdentifier willBeInsertedIntoToolbar:(BOOL)flag
{
	NSToolbarItem*					item = [[NSToolbarItem alloc] initWithItemIdentifier:itemIdentifier];
	
	//Create the NSToolbarItem from the resolution or export views - FIXME: Copy the views
	if([itemIdentifier isEqualToString:@"resolution"]) {
		[item setLabel:@"Resolution"];
		[item setView:resolutionView];
	}
	else if([itemIdentifier isEqualToString:@"export"]) {
		[item setLabel:@"Live Export"];
		[item setView:exportView];
	}
	[item setPaletteLabel:[item label]];
	[item setMaxSize:[[item view] frame].size];
	[item setMinSize:NSMakeSize([item maxSize].width - 50, [item maxSize].height)];
	
	return [item autorelease];
}
    
- (NSArray*) toolbarDefaultItemIdentifiers:(NSToolbar*)toolbar
{
	return [NSArray arrayWithObjects:NSToolbarFlexibleSpaceItemIdentifier, @"resolution", NSToolbarFlexibleSpaceItemIdentifier, @"export", NSToolbarFlexibleSpaceItemIdentifier, nil];
}

- (NSArray*) toolbarAllowedItemIdentifiers:(NSToolbar*)toolbar
{
	return [self toolbarDefaultItemIdentifiers:toolbar];
}

- (void) _doneResizing
{
	/*
		The QCRenderer class does not pick up automatically OpenGL viewport dimensions changes, which affects rendering quality.
		The workaround is to recreate all QCRenderers wheneven we change the viewport dimensions.
	*/
	if(_resizing) {
		[self _reloadRenderers];
		_resizing = NO;
	}
}

- (void) _setRenderSize:(NSSize)size
{
	NSRect							frame;
	
	//Update window contents size
	frame = [mainWindow contentRectForFrameRect:[mainWindow frame]];
	frame.origin.y = frame.origin.y + frame.size.height - size.height;
	frame.size = size;
	[mainWindow setFrame:[mainWindow frameRectForContentRect:frame] display:YES animate:YES];
	
	//Resizing is finished
	[self _doneResizing];
}

- (void) _setTimerFramerate:(double)framerate
{
	//Destroy current timer
	[_renderTimer invalidate];
	[_renderTimer release];
	_renderTimer = nil;
	
	//Create new timer
	if(framerate > 0.0) {
		_renderTimer = [[NSTimer timerWithTimeInterval:(1.0 / framerate) target:self selector:@selector(_renderTimer:) userInfo:nil repeats:YES] retain];
		[[NSRunLoop currentRunLoop] addTimer:_renderTimer forMode:NSDefaultRunLoopMode];
		[[NSRunLoop currentRunLoop] addTimer:_renderTimer forMode:NSModalPanelRunLoopMode];
		[[NSRunLoop currentRunLoop] addTimer:_renderTimer forMode:NSEventTrackingRunLoopMode];
		_startTime = 0.0;
	}
}

- (IBAction) updateExport:(id)sender
{
	NSSavePanel*					savePanel = [NSSavePanel savePanel];
	NSSize							size = [renderView frame].size;
	double							framerate = kDisplayFramerate;
	CodecType						codec;
	ICMCompressionSessionOptionsRef options;
	
	//Stop current export
	[_exporter release];
	_exporter = nil;
	[_reader release];
	_reader = nil;
	
	//Start new export if necessary
	switch([exportMenu indexOfSelectedItem]) {
		
		//Prompt user for movie location and compression settings, then configure exporter
		case kExport_QuickTimeMovie:
		[savePanel setRequiredFileType:@"mov"];
		[savePanel setCanCreateDirectories:YES];
		[savePanel setCanSelectHiddenExtension:YES];
		if(([savePanel runModalForDirectory:[@"~/Desktop" stringByExpandingTildeInPath] file:@"Export Movie"] == NSOKButton) && (options = [FrameCompressor userOptions:&codec frameRate:&framerate autosaveName:@"CompressionDialogSettings"])) {
			_reader = [[FrameReader alloc] initWithOpenGLContext:_glContext pixelsWide:size.width pixelsHigh:size.height asynchronousFetching:YES];
			if(_reader) {
				_exporter = [[FrameMovieExporter alloc] initWithPath:[savePanel filename] codec:codec pixelsWide:size.width pixelsHigh:size.height options:options];
				if(_exporter == nil) {
					[_reader release];
					_reader = nil;
				}
			}
		}
		if(_exporter == nil) {
			[exportMenu selectItemAtIndex:0];
			NSBeep();
		}
		break;
		
		//Resize rendering view to standard NTSC resolution, then configure exporter
		case kExport_FireWireDV:
		size = [FrameLiveDVExporter sizeForFormat:kDVFormat_NTSC];
		[self _setRenderSize:size];
		_reader = [[FrameReader alloc] initWithOpenGLContext:_glContext pixelsWide:size.width pixelsHigh:size.height asynchronousFetching:YES];
		if(_reader) {
			_exporter = [[FrameLiveDVExporter alloc] initWithDVFormat:kDVFormat_NTSC progressive:YES wideScreen:NO];
			if(_exporter)
			framerate = [FrameLiveDVExporter framerateForFormat:kDVFormat_NTSC];
			else {
				[_reader release];
				_reader = nil;
			}
		}
		if(_exporter == nil) {
			[exportMenu selectItemAtIndex:0];
			NSBeep();
		}
		break;
		
	}
	
	//Update rendering timer framerate
	[self _setTimerFramerate:framerate];
}

- (IBAction) updateResolution:(id)sender
{
	NSSize							size = [renderView frame].size;
	
	//Compute new resolution
	if(sender == widthField)
	size.width = [widthField intValue];
	else if(sender == heightField)
	size.height = [heightField intValue];
	
	//Resize rendering view if necessary
	if(!NSEqualSizes(size, [renderView frame].size))
	[self _setRenderSize:size];
}

- (void) applicationDidFinishLaunching:(NSNotification*)notification
{
	NSOpenGLPixelFormatAttribute	attributes[] = {NSOpenGLPFAAccelerated, NSOpenGLPFANoRecovery, NSOpenGLPFADoubleBuffer, NSOpenGLPFADepthSize, 24, 0};
	NSSize							size;
	NSScrollView*					scrollView;
	NSToolbar*						toolbar;
	
	//Setup window toolbar
	toolbar = [[NSToolbar alloc] initWithIdentifier:@"mainToolbar"];
	[toolbar setDisplayMode:NSToolbarDisplayModeIconOnly];
	[toolbar setAllowsUserCustomization:NO];
	[toolbar setDelegate:self];
	[mainWindow setToolbar:toolbar];
	[toolbar release];
	
	//Configure resolution fields
	size = [mainWindow minSize];
	[[widthField formatter] setMinimum:[NSNumber numberWithFloat:size.width]];
	[[heightField formatter] setMinimum:[NSNumber numberWithFloat:size.height]];
	size = [renderView frame].size;
	[widthField setIntValue:size.width];
	[heightField setIntValue:size.height];
	
	//Show main window immediately so that the OpenGL has a surface
	[mainWindow makeKeyAndOrderFront:nil];
	
	//Create OpenGL context used to render the QCRenderers and attach it to the rendering view
	_glPixelFormat = [[NSOpenGLPixelFormat alloc] initWithAttributes:attributes];
	_glContext = [[NSOpenGLContext alloc] initWithFormat:_glPixelFormat shareContext:nil];
	[_glContext setView:renderView];
	[self _clearGLContext];
	
	//We need to know when the rendering view frame changes so that we can update the OpenGL context
	[[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(_updateRenderView:) name:NSViewFrameDidChangeNotification object:renderView];
	
	//Create all QCRenderers and prepare the settings panel
	_settingsView = [[RenderParametersView alloc] initWithFrame:NSZeroRect];
	scrollView = [[NSScrollView alloc] initWithFrame:NSZeroRect];
	[scrollView setDrawsBackground:NO];
	[scrollView setHasHorizontalScroller:NO];
	[scrollView setHasVerticalScroller:YES];
	[[scrollView verticalScroller] setControlSize:NSSmallControlSize];
	[scrollView setDocumentView:_settingsView];
	[parametersPanel setContentView:scrollView];
	[scrollView release];
	[self _reloadRenderers];
	
	//Restore parameters of settings panel and show it
	[_settingsView setParameters:[[NSUserDefaults standardUserDefaults] objectForKey:@"settings"]];
	[parametersPanel orderFront:nil];
	
	//Create a timer which will regularly call our rendering method
	[self _setTimerFramerate:kDisplayFramerate];
}

- (void) _renderTimer:(NSTimer*)timer
{
	NSTimeInterval					time = [NSDate timeIntervalSinceReferenceDate];
	NSArray*						renderers = [_settingsView renderers];
	unsigned						i;
	CVPixelBufferRef				frame;
	
#if !__RENDER_WHILE_RESIZING__
	//Make sure we don't render anything if in the middle of resizing the rendering view
	if(_resizing)
	return;
#endif
	
	//Compute the local time
	if(_startTime == 0.0)
	_startTime = time;
	time = time - _startTime;
	
	//Render frame by calling all QCRenderers
	for(i = 0; i < [renderers count]; ++i)
	[(QCRenderer*)[renderers objectAtIndex:i] renderAtTime:time arguments:nil];
	
	//Export frame
	if(_exporter) {
		frame = [_reader readFrame];
		if(frame) {
			if([_exporter isKindOfClass:[FrameLiveDVExporter class]])
			[(FrameLiveDVExporter*)_exporter exportFrame:[_reader readFrame]];
			else if([_exporter isKindOfClass:[FrameMovieExporter class]])
			[(FrameMovieExporter*)_exporter exportFrame:[_reader readFrame] timeStamp:time];
		}
	}
	
	//Display frame on screen
	[_glContext flushBuffer];
}

- (void) _updateRenderView:(NSNotification*)notification
{
	NSRect							frame = [renderView frame];
	CGLContextObj					cgl_ctx = [_glContext CGLContextObj]; //By using CGLMacro.h there's no need to set the current OpenGL context
	
	//Stop export
	if(_exporter) {
		[_exporter release];
		_exporter = nil;
		[_reader release];
		_reader = nil;
		[exportMenu selectItemAtIndex:0];
	}
	
	//Notify the OpenGL context its rendering view has changed
	[_glContext update];
	
	//Update the OpenGL viewport
	glViewport(0, 0, frame.size.width, frame.size.height);
	
	//Render a frame immediately
#if __RENDER_WHILE_RESIZING__
	[self _renderTimer:nil];
#else
	[self _clearGLContext];
#endif
	
	//Update resolution fields
	[widthField setIntValue:frame.size.width];
	[heightField setIntValue:frame.size.height];
	
	//Install a callback to be called automatically when resizing has ended
	if(!_resizing) {
		[self performSelector:@selector(_doneResizing) withObject:nil afterDelay:0.0];
		_resizing = YES;
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
	//Stop rendering
	[_renderTimer invalidate];
	[_renderTimer release];
	
	//Stop observing the rendering view
	[[NSNotificationCenter defaultCenter] removeObserver:self name:NSViewFrameDidChangeNotification object:renderView];
	
	//Stop export
	[_exporter release];
	[_reader release];
	
	//Save parameters of settings panel
	[[NSUserDefaults standardUserDefaults] setObject:[_settingsView parameters:YES] forKey:@"settings"];
	[[NSUserDefaults standardUserDefaults] synchronize];
}

- (void) dealloc
{
	//Release our objects
	[_settingsView release];
	[_glContext release];
	[_glPixelFormat release];
	
	[super dealloc];
}

@end
