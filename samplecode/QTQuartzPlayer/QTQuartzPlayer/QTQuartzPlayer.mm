/*

File: QTQuartzPlayer.mm

Abstract: Implementation file for QuartzPlayer

Version: 1.0

© Copyright 2005 Apple Computer, Inc. All rights reserved.

IMPORTANT:  This Apple software is supplied to you by Apple Computer,
Inc. ("Apple") in consideration of your agreement to the following
terms, and your use, installation, modification or redistribution of
this Apple software constitutes acceptance of these terms.  If you do
not agree with these terms, please do not use, install, modify or
redistribute this Apple software.

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

Revision History:
	<1>	08/08/2005  initial release

*/ 

#import "QTQuartzPlayer.h"

#import <OpenGL/OpenGL.h>
#import <OpenGL/gl.h>
#import <OpenGL/glu.h>
#import <QuickTime/QuickTime.h>
#import <QTKit/QTKit.h>
#import <QuartzCore/QuartzCore.h>
#import <CoreAudio/HostTime.h>
#import "Camera.h"
#import "LiveVideo.h"

#define lengthof(x) (sizeof(x) / sizeof(x[0]))

#pragma mark Display Link Callback
// The renderer output callback function.
// The display link invokes this callback whenever it wants you to output a frame.
static CVReturn MyOutputCallback(CVDisplayLinkRef displayLink, const CVTimeStamp *inNow, const CVTimeStamp *inOutputTime, CVOptionFlags flagsIn, CVOptionFlags *flagsOut, void *displayLinkContext)
{
	return [(MyView*)displayLinkContext outputForTime:inOutputTime];
}

// qsort compare function that compares two elements. The function takes  two parameters that are pointers to SortedFrame elements,
// and  returns an int value with the result of comparing them:
// return value	description:
// < 0	*elem1 goes before *elem2
// 0	*elem1 == *elem2
// > 0	*elem1 goes after *elem2
static int CompareFrameDepths(const SortedFrame* a, const SortedFrame* b)
{
	return (a->depth < b->depth) ? 1 : (a->depth > b->depth) ? -1 : 0;
}

#pragma mark View

// Implementation of the View Class
// We're not using the NSOpenGLView in this case for thread-safety.
// There are a number of reasons you may want to roll your own OpenGL View; Thread-safety, Resolution indipendance and ColorSync
// for example. In this sample we're doing as much work as needed to draw but nothing more. See VideoViewer for a more complete
// example of a custom OpenGL View.
@implementation MyView

// Dealocation
-(void)dealloc
{
	CVDisplayLinkStop(_displayLink);
	CVDisplayLinkRelease(_displayLink);
	
	[self disposeVideo];
	
	[[NSNotificationCenter defaultCenter] removeObserver:self];
	[_openGLContext release];
	[_openGLPixelFormat release];
	QTVisualContextRelease(_textureContext);

	[_lock release];
	[_camera release];
	[_frames release];
	[_quickTimeRate release];
	[_openGLRate release];
	
	[super dealloc];
}

// Initialization after loading the NIB
-(void)awakeFromNib
{
	_lock = [[NSLock alloc] init];
	_camera = [[Camera alloc] init];
	_frames = [[NSMutableArray alloc] init];
	_quickTimeRate = [[FrameRate alloc] init];
	_openGLRate = [[FrameRate alloc] init];

	_needToRender = YES;
	_length = 1;

	[self updateInfo];

	// Enable drag & drop
	[self registerForDraggedTypes:[NSArray arrayWithObject:NSFilenamesPboardType]];			
}

// Update the View
-(void)update:(NSNotification*)notification
{
	[_lock lock];
		[_openGLContext update];
		[self setNeedsDisplay:YES];
	[_lock unlock];
}

// Initalization of the View
-(void)initializeOpenGL
{
    // Create an NSOpenGLPixelFormat Attributes list for [NSOpenGLPixelFormat initWithAttributes]
	NSOpenGLPixelFormatAttribute attributes[] = {
		NSOpenGLPFAColorSize, NSOpenGLPixelFormatAttribute(24),
		NSOpenGLPFAAlphaSize, NSOpenGLPixelFormatAttribute(8),
		NSOpenGLPFADepthSize, NSOpenGLPixelFormatAttribute(32),
		NSOpenGLPixelFormatAttribute(0)
	};
	
	_openGLPixelFormat = [(NSOpenGLPixelFormat*)[NSOpenGLPixelFormat alloc] initWithAttributes:attributes];
	_openGLContext = [[NSOpenGLContext alloc] initWithFormat:_openGLPixelFormat shareContext:nil];
	
	if (_openGLContext != nil) {
		long swapInterval = 1; // If swap interval is set to 1, the buffers are swapped only during the vertical retrace of the monitor.
		[_openGLContext setView:self];
		[_openGLContext setValues:&swapInterval forParameter:NSOpenGLCPSwapInterval];
		
		// Listen for movements
		[[NSNotificationCenter defaultCenter] addObserver:self  selector:@selector(update:) name:NSViewGlobalFrameDidChangeNotification object:self];
							
		// Create display link
		CVDisplayLinkCreateWithActiveCGDisplays(&_displayLink);
		CVDisplayLinkSetCurrentCGDisplay(_displayLink, kCGDirectMainDisplay);
		CVDisplayLinkSetOutputCallback(_displayLink, &MyOutputCallback, self);
		CVDisplayLinkStart(_displayLink);
		
		// Create visual context
		QTOpenGLTextureContextCreate(NULL, CGLContextObj([_openGLContext CGLContextObj]), CGLPixelFormatObj([_openGLPixelFormat CGLPixelFormatObj]), nil, &_textureContext);
		
	} else if (_openGLPixelFormat == nil) {
		_openGLPixelFormat = [[NSOpenGLView defaultPixelFormat] retain];
    }
}

// Perform a flush
-(void)flush
{
	// We're assuming _lock is locked by the caller
	[_openGLContext flushBuffer];
	QTVisualContextTask(_textureContext);
	_needToFlush = NO;
}

-(void)transformFrame:(int)i
{
	// Stack frames based on their position
	glTranslated(0, 0, -i / 5.0);
	
	// Optionally rotate frames
	if ([_spinningModeItem state] == NSOnState)
		glRotated(i * 5.0, 0, 0, 1);
}

// Calculate the projected depth of a frame on the screen in order to sort them back-to-front
-(double)projectedFrameDepth:(int)i
{
	GLdouble modelView[16], projection[16];
	GLint viewport[4];
	GLdouble x, y, z;

	glPushMatrix();
		[self transformFrame:i];
		glGetDoublev(GL_MODELVIEW_MATRIX, modelView);
		glGetDoublev(GL_PROJECTION_MATRIX, projection);
		glGetIntegerv(GL_VIEWPORT, viewport);

		// We'll base our calculation on the origin, because that is where our frames are centered
		gluProject(0, 0, 0, modelView, projection, viewport, &x, &y, &z);
	glPopMatrix();
	
	return z;
}

// Main rendering routine
-(void)renderForTime:(const CVTimeStamp*)timeStamp flush:(BOOL)flush
{
	NSSize newSize = [self bounds].size;
	
	[_openGLRate tick];
	[_openGLContext makeCurrentContext];
	
	// Re-configure OpenGL if our view has changed size since the last render
	if (!NSEqualSizes(newSize, _previousSize))
	{
		GLint width = GLint(newSize.width);
		GLint height = GLint(newSize.height);
		
		glViewport(0, 0, width, height);
		
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		gluPerspective(30, double(width) / height, 1, 1000);
	
		_previousSize = newSize;
	}
	
	glClearColor(0, 0, 0, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	// If the up or down keys are held down, pan the camera in and out a bit
	if (_zooming != 0.0)
		[_camera panX:0 Y:0 Z:_zooming forTime:timeStamp];
	
	// Tell the camera to transform our projection
	[_camera lookForTime:timeStamp];

	// Render the video frames
	{
		unsigned count = [_frames count];
		SortedFrame* sortedFrames = new SortedFrame[count];  // Yeah, yeah, this is inefficient :)

		glEnable(GL_DEPTH_TEST);
		glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

			// All translucent objects must be rendered back-to-front
			for (unsigned i = 0; i < count; i++)
			{
				sortedFrames[i].texture = CVOpenGLTextureRef([_frames objectAtIndex:i]);
				sortedFrames[i].index = i;
				sortedFrames[i].depth = [self projectedFrameDepth:i];
			}

			qsort(sortedFrames, count, sizeof(SortedFrame), (int (*)(const void*, const void*))&CompareFrameDepths);

			for (unsigned i = 0; i < count; i++)
			{
				SortedFrame* frame = &sortedFrames[i];
				
				if (CFGetTypeID(frame->texture) == CVOpenGLTextureGetTypeID()) // Protect against NSNull objects
				{
					GLenum target = CVOpenGLTextureGetTarget(frame->texture);
					GLint name = CVOpenGLTextureGetName(frame->texture);
					GLfloat topLeft[2], topRight[2], bottomRight[2], bottomLeft[2];
					
					CVOpenGLTextureGetCleanTexCoords(frame->texture, bottomLeft, bottomRight, topRight, topLeft);
					
					glPushMatrix();
						// For live video, flip horizontally as if you're looking in a mirror (a-la iChat)
						if (_flipped)
							glScaled(-1, 1, 1);

						[self transformFrame:frame->index];
						
						// Scale by the aspect ratio of the movie so we can draw our texture into a square (-1,-1) -> (1,1)
						glScaled(_movieSize.width / _movieSize.height, 1.0, 1.0);
						
						// Make the older frames more and more transparent
						glColor4f(1.0f, 1.0f, 1.0f, 1.0 - float(frame->index) / count);

						// Draw texture!
						glEnable(target);
							glBindTexture(target, name);
							glBegin(GL_QUADS);
								glTexCoord2fv(bottomLeft);  glVertex2i(-1, -1);
								glTexCoord2fv(topLeft);     glVertex2i(-1,  1);
								glTexCoord2fv(topRight);    glVertex2i( 1,  1);
								glTexCoord2fv(bottomRight); glVertex2i( 1, -1);
							glEnd();
						glDisable(target);
					glPopMatrix();
				}
			}
		glDisable(GL_BLEND);
		glDisable(GL_DEPTH_TEST);
		
		delete [] sortedFrames;
	}

	// We're done drawing for now...
	glFlush();
	_needToRender = NO;
	_needToFlush = YES;
	
	// We only flush immediately when called from drawRect:, otherwise we wait for the DisplayLink's 'display' callback
	if (flush)
		[self flush];
}

// Standard Draw method for the view
-(void)drawRect:(NSRect)rect
{
	if (_openGLPixelFormat == nil)
		[self initializeOpenGL];
	
	if (_openGLContext != nil)
	{
		CVTimeStamp timeStamp;
		
		timeStamp.version = 0;
		timeStamp.flags = kCVTimeStampHostTimeValid;
		
		[_lock lock];
			timeStamp.hostTime = AudioGetCurrentHostTime();
			[self renderForTime:&timeStamp flush:YES];
		[_lock unlock];
	}
	else
	{
		// We get here if OpenGL failed for some reason
		[[NSColor redColor] set];
		NSRectFill(rect);
	}
}

#pragma mark Display Link

// Method called from the Display Link callback to check for new textures and render if required
- (CVReturn)outputForTime:(const CVTimeStamp*)timeStamp
{
	NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];
	BOOL locked = NO;
	
	// Check for new textures
	if (_textureContext != nil && QTVisualContextIsNewImageAvailable(_textureContext, timeStamp))
	{
		CVOpenGLTextureRef texture;

		[_quickTimeRate tick];
		[_lock lock];
		locked = YES;

		while ([_frames count] >= _length) {
			[_frames removeLastObject];
        }
            
		if (QTVisualContextCopyImageForTime(_textureContext, NULL, timeStamp, &texture) == noErr) {
            // the above call may produce a null texture so we must check for this first
            if (NULL != texture) {
                [_frames insertObject:texture != NULL ? (id)texture : [NSNull null] atIndex:0];
                CVOpenGLTextureRelease(texture);
                _needToRender = YES;
            }
		}
	}

	// Render only if necessary
	if (_needToRender || [_camera isAnimatedForTime:timeStamp] || _zooming != 0.0)
	{
		if (!locked)
			[_lock lock];
		locked = YES;
		[self renderForTime:timeStamp flush:NO];
	}

	if (_needToFlush)
	{
		if (!locked)
			[_lock lock];
		[self flush];
	}

	if (locked)
		[_lock unlock];

	[self performSelectorOnMainThread:@selector(updateInfo) withObject:nil waitUntilDone:NO];

	[pool release];
    
	return kCVReturnSuccess;
}

#pragma mark Manipulation

// Handle Mouse Moved/Dragged events
-(void)mouseMoved:(NSEvent*)e
{
	[self mouseMotion:e dragged:NO];
}

-(void)mouseDragged:(NSEvent*)e
{
	[self mouseMotion:e dragged:YES];
}

// Handle Key Down messages
-(BOOL)keyDown:(NSEvent*)event
{
	static const double zoomStep = 0.15;
	NSString* characters = [event charactersIgnoringModifiers];
	unichar key = [characters characterAtIndex:0];
	BOOL taken = YES;
	double speed = ([event modifierFlags] & NSAlternateKeyMask) != 0 ? 5.0 : 1.0;
	
	[_lock lock];
	{
		CVTimeStamp now;
		now.hostTime = AudioGetCurrentHostTime();

		switch (key)
		{
		case NSUpArrowFunctionKey:
			_zooming = zoomStep;
			break;
		
		case NSDownArrowFunctionKey:
			_zooming = -zoomStep;
			break;
		
		case '.':
			[self increaseLength:self];
			break;
		
		case ',':
			[self decreaseLength:self];
			break;
			
		case ' ':
			if (_movie != nil)
				[_movie setRate:[_movie rate] == 0.0 ? 1.0 : 0.0];
			break;
			
		case 27: // Escape
			[_camera animateTo:[Camera camera] over:speed];
			break;
		
		default:
			taken = NO;
			
			if (key >= 'a' && key <= 'z')
			{
				NSUserDefaults* defaults = [NSUserDefaults standardUserDefaults];
				NSString* preset = [NSString stringWithFormat:@"Preset-%@", characters];
				unsigned int smashed = NSControlKeyMask | NSAlternateKeyMask | NSCommandKeyMask;
				
				if (([event modifierFlags] & smashed) == smashed)
				{
					[defaults setObject:[NSKeyedArchiver archivedDataWithRootObject:_camera] forKey:preset];
				}
				else
				{
					NSData* data = [defaults dataForKey:preset];
					
					if (data)
					{
						[_camera animateTo:[NSKeyedUnarchiver unarchiveObjectWithData:data] over:speed];
					}
				}
				
				taken = YES;
			}
		}

		_needToRender = YES;
	}
	[_lock unlock];
	
	return taken;
}

-(void)keyUp:(NSEvent*)event
{
	_zooming = 0.0;
}

#pragma mark Actions

-(void)openPanelDidEnd:(NSOpenPanel*)sheet returnCode:(int)returnCode contextInfo:(void*)contextInfo
{
	[self openMovie:[[sheet filenames] objectAtIndex:0]];
}

-(IBAction)open:(id)sender
{
	[[NSOpenPanel openPanel] beginSheetForDirectory:nil file:nil types:nil modalForWindow:[self window] modalDelegate:self didEndSelector:@selector(openPanelDidEnd:returnCode:contextInfo:) contextInfo:nil];
}

-(IBAction)showInfo:(id)sender
{
	[sender setState:[sender state] == NSOnState ? NSOffState : NSOnState];
	if ([sender state] == NSOnState)
		[_infoPanel orderFront:self];
	else
		[_infoPanel orderOut:self];
}

-(IBAction)halfSize:(id)sender
{
	if (_movieSize.width > 0 && _movieSize.height > 0)
	{
		[self toggleSize:sender];
		[self setSize:NSMakeSize(_movieSize.width / 2, _movieSize.height / 2) animate:YES];
	}
}

-(IBAction)normalSize:(id)sender
{
	if (_movieSize.width > 0 && _movieSize.height > 0)
	{
		[self toggleSize:sender];
		[self setSize:_movieSize animate:YES];
	}
}

-(IBAction)doubleSize:(id)sender
{
	if (_movieSize.width > 0 && _movieSize.height > 0)
	{
		[self toggleSize:sender];
		[self setSize:NSMakeSize(_movieSize.width * 2, _movieSize.height * 2) animate:YES];
	}
}

-(IBAction)fillScreen:(id)sender
{
	[self toggleSize:sender];
	[self setSize:[[NSScreen mainScreen] visibleFrame].size animate:YES];
}

-(IBAction)normalMode:(id)sender
{
	[self toggleMode:sender];
	[self resetCamera];
}

-(IBAction)stackedMode:(id)sender
{
	[self toggleMode:sender];
}

-(IBAction)spinningMode:(id)sender
{
	[self toggleMode:sender];
}

-(IBAction)increaseLength:(id)sender
{
	[self changeLength:5];
}

-(IBAction)decreaseLength:(id)sender
{
	[self changeLength:-5];
}

-(IBAction)live:(id)sender
{
	if (_liveVideo == nil)
	{
		[self disposeVideo];
		_flipped = YES;
		_liveVideo = [[LiveVideo alloc] initWithVisualContext:_textureContext];
		_movieSize = [_liveVideo size];
	}
}

#pragma mark File Drag & Drop

-(NSDragOperation)draggingEntered:(id<NSDraggingInfo>)sender
{	
	NSArray* files = [[sender draggingPasteboard] propertyListForType:NSFilenamesPboardType];
	return [files count] == 1 && [[NSFileManager defaultManager] isReadableFileAtPath:[files objectAtIndex:0]]
	        ? NSDragOperationGeneric : NSDragOperationNone;
}

-(BOOL)performDragOperation:(id<NSDraggingInfo>)sender
{
	NSArray* files = [[sender draggingPasteboard] propertyListForType:NSFilenamesPboardType];
	[self openMovie:[files objectAtIndex:0]];
	return YES;
}

-(NSDragOperation)draggingUpdated:(id<NSDraggingInfo>)sender {return NSDragOperationGeneric;}
-(void)draggingExited:(id<NSDraggingInfo>)sender {}
-(void)draggingEnded:(id<NSDraggingInfo>)sender {}
-(BOOL)prepareForDragOperation:(id<NSDraggingInfo>)sender {return YES;}
-(void)concludeDragOperation:(id<NSDraggingInfo>)sender {}

#pragma mark Utility

// Dispose a Movie and/or a LiveVideo Object
-(void)disposeVideo
{
	if (_movie != nil)
		[_movie release];
	_movie = nil;
	
	if (_liveVideo != nil)
		[_liveVideo release];
	_liveVideo = nil;
}

-(void)mouseMotion:(NSEvent*)event dragged:(BOOL)dragged
{
	static const double xPanFactor = 1.0 / 100.0;
	static const double yPanFactor = -1.0 / 100.0;
	static const double xRotateFactor = 1.0 / 5.0;
	static const double yRotateFactor = 1.0 / 5.0;
	
    CVTimeStamp now;
	
	[_lock lock];
		now.hostTime = AudioGetCurrentHostTime();

		if (([event modifierFlags] & NSControlKeyMask) != 0)
			[_camera rotateX:[event deltaX] * xRotateFactor Y:[event deltaY] * yRotateFactor forTime:&now];
		else if (dragged)
			[_camera panX:[event deltaX] * xPanFactor Y:[event deltaY] * yPanFactor Z:0.0 forTime:&now];

		_needToRender = YES;
	[_lock unlock];
}

-(void)resetCamera
{
	[_lock lock];
		[_camera release];
		_camera = [[Camera alloc] init];
		_needToRender = YES;
	[_lock unlock];
}

// Manage the number of frames in our frame queue
-(void)changeLength:(int)amount
{
	if ((int)_length + amount < 1)
		_length = 1;
	else
		_length += amount;

    // could also do this
    //_length = MAX(1U, _length + amount);
}

#pragma mark Miscellaneous

-(void)updateInfo
{
	static const double updatesPerSecond = 2;
	static UInt64 lastUpdate = 0;
	static UInt64 updatePeriod = 0;	
	UInt64 now = AudioGetCurrentHostTime();
	
	// Don't waste CPU time updating the frame rate widgets
	if (updatePeriod == 0)
		updatePeriod = UInt64(AudioGetHostClockFrequency() / updatesPerSecond);
	
	if ((now - lastUpdate) > updatePeriod)
	{
		NSSize glSize = [self bounds].size;
		lastUpdate = now;

		[_quickTimeSizeField  setStringValue:[NSString stringWithFormat:@"%gx%g", _movieSize.width, _movieSize.height]];
		[_quickTimeRateField  setStringValue:[NSString stringWithFormat:@"%0.1lf", [_quickTimeRate rate]]];
		[_quickTimeCountField setStringValue:_length > 0 ? [NSString stringWithFormat:@"x%i", _length] : @""];
		[_openGLSizeField     setStringValue:[NSString stringWithFormat:@"%gx%g", glSize.width, glSize.height]];
		[_openGLRateField     setStringValue:[NSString stringWithFormat:@"%0.1lf", [_openGLRate rate]]];
	}
}

-(void)setSize:(NSSize)size animate:(BOOL)animate
{
	NSWindow* window = [self window];
	NSSize oldSize = [self bounds].size;
	NSRect screenFrame = [[window screen] visibleFrame];
	NSRect frame = [window frame];
	NSSize padding = NSMakeSize(frame.size.width - oldSize.width, frame.size.height - oldSize.height);
	NSPoint oldCenter = NSMakePoint(NSMidX(frame), NSMaxY(frame));

	// Grow or shrink frame to fit
	frame.size.width = size.width + padding.width;
	frame.size.height = size.height + padding.height;

	// Constrain to screen size
	NSSize newSize = NSMakeSize(MIN(size.width,  screenFrame.size.width  - padding.width),
	                            MIN(size.height, screenFrame.size.height - padding.height));

	// Constrain to movie aspect ratio
	newSize.width  = MIN(newSize.width,  _movieSize.width  * newSize.height / _movieSize.height);
	newSize.height = MIN(newSize.height, _movieSize.height * newSize.width  / _movieSize.width);

	// Re-center title bar
	frame.size = NSMakeSize(newSize.width + padding.width, newSize.height + padding.height);
	frame.origin.x += oldCenter.x - NSMidX(frame);
	frame.origin.y += oldCenter.y - NSMaxY(frame);

	// Re-constrain to screen
	if (NSMinX(frame) < NSMinX(screenFrame))
		frame.origin.x = NSMinX(screenFrame);
	if (NSMinY(frame) < NSMinY(screenFrame))
		frame.origin.y = NSMinY(screenFrame);
	if (NSMaxX(frame) > NSMaxX(screenFrame))
		frame.origin.x -= NSMaxX(frame) - NSMaxX(screenFrame);
	if (NSMaxY(frame) > NSMaxY(screenFrame))
		frame.origin.y -= NSMaxY(frame) - NSMaxY(screenFrame);
	
	[window setFrame:frame display:YES animate:animate];
	[self resetCamera];
}

-(void)openMovie:(NSString*)path
{
	if (_textureContext != nil)
	{
		[self disposeVideo];
		
        // we don't "mirror" movies
		_flipped = NO;
		
		_movie = [[QTMovie alloc] initWithFile:path error:nil];
		[[_movie attributeForKey:QTMovieNaturalSizeAttribute] getValue:&_movieSize];
		[_movie setAttribute:[NSNumber numberWithBool:YES] forKey:QTMovieLoopsAttribute];
		
        SetMovieVisualContext([_movie quickTimeMovie], _textureContext);
		
        [_movie setRate:1.0];
		[[self window] setTitle:[_movie attributeForKey:QTMovieDisplayNameAttribute]];
		[self setSize:_movieSize animate:YES];
	}
}

-(void)toggleItem:(id)sender others:(id*)items count:(size_t)count
{
	for (size_t i = 0; i < count; i++)
		[items[i] setState:items[i] == sender ? NSOnState : NSOffState];
}

-(void)toggleSize:(id)sender
{
	id items[] = {_halfSizeItem, _normalSizeItem, _doubleSizeItem, _fillScreenItem};
	[self toggleItem:sender others:items count:lengthof(items)];
}

-(void)toggleMode:(id)sender
{
	id items[] = {_normalModeItem, _stackedModeItem, _spinningModeItem};
	[self toggleItem:sender others:items count:lengthof(items)];
}

-(BOOL)acceptsFirstResponder
{
	return YES;
}

-(BOOL)applicationShouldTerminateAfterLastWindowClosed:(NSApplication*)sender
{
	return YES;
}

-(BOOL)application:(NSApplication *)sender openFile:(NSString*)filename
{
	[self openMovie:filename];
	return YES;
}

@end

#pragma mark Window
@implementation MyWindow

-(void)awakeFromNib
{
	[self setAcceptsMouseMovedEvents:YES];
}

-(void)sendEvent:(NSEvent*)event
{
	BOOL taken = NO;
	
	if ([event type] == NSKeyDown)
		taken = [_myView keyDown:event];

	if (!taken)
		[super sendEvent:event];
}

@end

#pragma mark FrameRate
@implementation FrameRate
-(id)init
{
	[super init];
	_count = 0;
	_frequency = AudioGetHostClockFrequency();
	return self;
}

-(void)tick
{
	for (int i = _count; i > 0; i--)
		_stamps[i] = _stamps[i-1];
	_stamps[0] = AudioGetCurrentHostTime();
	_count++;
	if (_count > NumStamps)
		_count = NumStamps;
}

-(double)rate
{
	return double(_count - 1) / ((_stamps[0] - _stamps[_count - 1]) / _frequency);
}

@end

#pragma mark

int main(int argc, const char **argv)
{
	return NSApplicationMain(argc,argv);
}
