/*
	Disclaimer:	IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc.
				("Apple") in consideration of your agreement to the following terms, and your
				use, installation, modification or redistribution of this Apple software
				constitutes acceptance of these terms.  If you do not agree with these terms,
				please do not use, install, modify or redistribute this Apple software.
	
				In consideration of your agreement to abide by the following terms, and subject
				to these terms, Apple grants you a personal, non-exclusive license, under Apple’s
				copyrights in this original Apple software (the "Apple Software"), to use,
				reproduce, modify and redistribute the Apple Software, with or without
				modifications, in source and/or binary forms; provided that if you redistribute
				the Apple Software in its entirety and without modifications, you must retain
				this notice and the following text and disclaimers in all such redistributions of
				the Apple Software.  Neither the name, trademarks, service marks or logos of
				Apple Computer, Inc. may be used to endorse or promote products derived from the
				Apple Software without specific prior written permission from Apple.  Except as
				expressly stated in this notice, no other rights or licenses, express or implied,
				are granted by Apple herein, including but not limited to any patent rights that
				may be infringed by your derivative works or by other works in which the Apple
				Software may be incorporated.
	
				The Apple Software is provided by Apple on an "AS IS" basis.  APPLE MAKES NO
				WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED
				WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS FOR A PARTICULAR
				PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND OPERATION ALONE OR IN
				COMBINATION WITH YOUR PRODUCTS.
	
				IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL OR
				CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
				GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
				ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION, MODIFICATION AND/OR DISTRIBUTION
				OF THE APPLE SOFTWARE, HOWEVER CAUSED AND WHETHER UNDER THEORY OF CONTRACT, TORT
				(INCLUDING NEGLIGENCE), STRICT LIABILITY OR OTHERWISE, EVEN IF APPLE HAS BEEN
				ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/
#import "CubeGLView.h"
#import "CubeController.h"
#import <OpenGL/gl.h>
#import <OpenGL/glext.h>
#import <OpenGL/glu.h>
#import "Cube.h"

// Some simple vector utility routines ripped out of VectorLib.h.
static inline Vector MakeVector(float _x, float _y, float _z)
{
  Vector v;
  v.x = _x; v.y = _y; v.z = _z;
  return v;
}

static inline Vector SumVectors(Vector _a, Vector _b)
{
  _a.x += _b.x;
  _a.y += _b.y;
  _a.z += _b.z;
  return _a;
}

@implementation CubeGLView

- (id)initWithFrame:(NSRect)theFrame
{
	NSOpenGLContext *ctx;
	long swapInterval = 1;
	
	/* Create a GL context */
	self = [super initWithFrame:theFrame pixelFormat:[[NSApp delegate] sharedPixelFormat]];

	/* Replace OpenGL context with one of our own making.  This is required
	   because NSOpenGLView currently doesn't have built in support for shared
	   OpenGL contexts.  The other option would be to not use NSOpenGLView at
	   all, but I'm being lazy. */
	[self clearGLContext];
	ctx = [[NSOpenGLContext alloc] initWithFormat:[[NSApp delegate] sharedPixelFormat]
		shareContext:[[NSApp delegate] sharedOpenGLContext]];		
	[self setOpenGLContext:ctx];
	[ctx release];
	
	if(self) {		
		_orientation = MakeVector(20.0f,45.0f,0.0f);
		[[self openGLContext] setValues:&swapInterval forParameter:NSOpenGLCPSwapInterval];
		
		[[NSNotificationCenter defaultCenter] addObserver:self
							selector:@selector(cubeChanged:)
							name:CubeDidChangeNotification object:nil];
	}
	return self;
}

- (void)dealloc
{
	[[NSNotificationCenter defaultCenter] removeObserver:self];
	[super dealloc];
}

- (void)cubeChanged:(NSNotification *)theNotification
{
	if([theNotification object] == [[NSApp delegate] cube])
		[self display];
}

- (BOOL)isOpaque
{
	return YES;
}

- (void)reshape
{
	sizeChanged = YES;
	[super reshape];
}

- (void)drawRect:(NSRect)theRect
{
	NSRect visibleRect;
	float lightModelAmbient[4] = {0.0f,0.0f,0.0f,1.0f};
	float light0Position[4] = {1.0f,1.0f,1.0f,1.0f};
	float light0Ambient[4] = {0.0f, 0.0f, 0.0f, 0.0f};
	float light0Diffuse[4] = {1.0f, 1.0f, 1.0f, 1.0f};
	float light0Specular[4] = {1.0f, 1.0f, 1.0f, 0.0f};
	
	// Get visible bounds...
	visibleRect = [self bounds];
	
	// Set proper viewport
	glViewport(visibleRect.origin.x, visibleRect.origin.y, 
	           visibleRect.size.width, visibleRect.size.height);
	
	// Clear background to transparent black.
	glClearColor(0.0f,0.0f,1.0f,0.0f);
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	
	// Set up known GL render states.
	glDisable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);
	
	glCullFace(GL_BACK);
	glEnable(GL_CULL_FACE);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	
	// Set GL lighting
	glLightModeli(GL_LIGHT_MODEL_COLOR_CONTROL, GL_SEPARATE_SPECULAR_COLOR);
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, lightModelAmbient);
	glLightfv(GL_LIGHT0, GL_AMBIENT, light0Ambient);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, light0Diffuse);
	glLightfv(GL_LIGHT0, GL_SPECULAR, light0Specular);
		
	// Set camera field of view, viewport aspect ratio and near/far clipping planes.
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(30.0f, visibleRect.size.width / visibleRect.size.height, 1.0f,8192.0f);
	
	// Reset modelview matrix stack.
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	
	// Set up fixed light position.
	glLightfv(GL_LIGHT0, GL_POSITION, light0Position);
	
	// Back off from object position
	glTranslatef(0.0f,0.0f,-10.0f);
	
	glTranslatef(_position.x, _position.y, _position.z);
	
	// Do orientation.
	glRotatef(_orientation.z,0.0f,0.0f,1.0f);
	glRotatef(_orientation.x,1.0f,0.0f,0.0f);
	glRotatef(_orientation.y,0.0f,1.0f,0.0f);
	
	// Finally, draw the cube!
	[[[NSApp delegate] cube] draw];
	
	// This is an optimization.  If the view is being
	// resized, don't do a buffer swap.  The GL content
	// will be updated as part of the window flush anyway.
	// This makes live resize look nicer as the GL view
	// won't get flushed ahead of the window flush.  It also
	// makes live resize faster since we're not flushing twice.
	// Because I want the animtion to continue while resize
	// is happening, I use my own flag rather than calling
	// [self inLiveReize].  For most apps this wouldn't be
	// necessary.
	if(!sizeChanged)
		[[self openGLContext] flushBuffer];
	else
		glFlush();
	sizeChanged = NO;
}

- (BOOL)acceptsFirstResponder
{
	return YES;
}

- (BOOL)becomeFirstResponder
{
	return YES;
}

- (BOOL)resignFirstResponder
{	
	return YES;
}

- (void)mouseDown:(NSEvent *)theEvent
{
	BOOL special;
	float deltaX, deltaY;
	Vector result;
			
	while(1) {
		/* Wait for a single event.. */
		theEvent = [[self window] nextEventMatchingMask:
			(NSLeftMouseDraggedMask | NSLeftMouseUpMask)];

		special = (([theEvent modifierFlags] & NSAlternateKeyMask) ? YES : NO);
		
		
		deltaX = [theEvent deltaX];
		deltaY = [theEvent deltaY];
		
		if(special) {
			result.x = result.y = 0.0f;
			result.z = -deltaX * 0.5f;
		} else {
			result.x =  deltaY * 0.5f;
			result.y =  deltaX * 0.5f;
			result.z =  0.0f;
		}
		_orientation = SumVectors(_orientation, result);

		[self setNeedsDisplay:YES];
	
		if([theEvent type] == NSLeftMouseUp)
			break;
	}
}

- (void)keyDown:(NSEvent *)theEvent
{
	Cube *cube = [[NSApp delegate] cube];
	
	if([[theEvent characters] isEqual:@"L"])
		[cube performSequence:@"L-"];
	else if([[theEvent characters] isEqual:@"l"])
		[cube performSequence:@"L+"];
	else if([[theEvent characters] isEqual:@"R"])
		[cube performSequence:@"R-"];
	else if([[theEvent characters] isEqual:@"r"])
		[cube performSequence:@"R+"];
	else if([[theEvent characters] isEqual:@"T"])
		[cube performSequence:@"T-"];
	else if([[theEvent characters] isEqual:@"t"])
		[cube performSequence:@"T+"];
	else if([[theEvent characters] isEqual:@"B"])
		[cube performSequence:@"B-"];
	else if([[theEvent characters] isEqual:@"b"])
		[cube performSequence:@"B+"];
	else if([[theEvent characters] isEqual:@"F"])
		[cube performSequence:@"F-"];
	else if([[theEvent characters] isEqual:@"f"])
		[cube performSequence:@"F+"];
	else if([[theEvent characters] isEqual:@"P"])
		[cube performSequence:@"P-"];
	else if([[theEvent characters] isEqual:@"p"])
		[cube performSequence:@"P+"];
	else if([[theEvent characters] isEqual:@"X"])
		[cube performSequence:@"X-"];
	else if([[theEvent characters] isEqual:@"x"])
		[cube performSequence:@"X+"];
	else if([[theEvent characters] isEqual:@"Y"])
		[cube performSequence:@"Y-"];
	else if([[theEvent characters] isEqual:@"y"])
		[cube performSequence:@"Y+"];
	else if([[theEvent characters] isEqual:@"Z"])
		[cube performSequence:@"Z-"];
	else if([[theEvent characters] isEqual:@"z"])
		[cube performSequence:@"Z+"];
	else if([[theEvent characters] isEqual:@"s"])
		[cube performSequence:[cube solveCube]];

	[self setNeedsDisplay:YES];
}

- (void)undo:(NSEvent *)theEvent
{
	[[[NSApp delegate] undoManager] undo];
}

- (void)redo:(NSEvent *)theEvent
{
	[[[NSApp delegate] undoManager] redo];
}

@end
