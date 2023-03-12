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
#import "DemoGLView.h"
#import "DemoController.h"
#import <OpenGL/gl.h>
#import <OpenGL/glu.h>
#import "Demo2DOverlayView.h"
#import "DemoGLOverlayView.h"
#import "DemoWindow.h"
#import "NSViewOverlays.h"

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

void GetRGBColor(NSColor *color, float rgbColor[4])
{
  color = [color colorUsingColorSpaceName:NSCalibratedRGBColorSpace];
  [color getRed:&rgbColor[0] green:&rgbColor[1] blue:&rgbColor[2] alpha:&rgbColor[3]];
}

// Utility rendering routines shared by all subclasses.
void DrawCubeMinMax(Vector _min, Vector _max)
{
  glBegin(GL_QUADS);

  // Front
  glNormal3f(0.0f,0.0f,1.0f);
  glTexCoord2f(1.0f,0.0f);
  glVertex3f(_max.x,_min.y,_max.z);
  glTexCoord2f(1.0f,1.0f);
  glVertex3f(_max.x,_max.y,_max.z);
  glTexCoord2f(0.0f,1.0f);
  glVertex3f(_min.x,_max.y,_max.z);
  glTexCoord2f(0.0f,0.0f);
  glVertex3f(_min.x,_min.y,_max.z);

  // Top
  glNormal3f(0.0f,1.0f,0.0f);
  glTexCoord2f(1.0f,0.0f);
  glVertex3f(_max.x,_max.y,_max.z);
  glTexCoord2f(1.0f,1.0f);
  glVertex3f(_max.x,_max.y,_min.z);
  glTexCoord2f(0.0f,1.0f);
  glVertex3f(_min.x,_max.y,_min.z);
  glTexCoord2f(0.0f,0.0f);
  glVertex3f(_min.x,_max.y,_max.z);

  // Left
  glNormal3f(-1.0f,0.0f,0.0f);
  glTexCoord2f(1.0f,0.0f);
  glVertex3f(_min.x,_min.y,_max.z);
  glTexCoord2f(1.0f,1.0f);
  glVertex3f(_min.x,_max.y,_max.z);
  glTexCoord2f(0.0f,1.0f);
  glVertex3f(_min.x,_max.y,_min.z);
  glTexCoord2f(0.0f,0.0f);
  glVertex3f(_min.x,_min.y,_min.z);

  // Back
  glNormal3f(0.0f,0.0f,-1.0f);
  glTexCoord2f(1.0f,0.0f);
  glVertex3f(_min.x,_min.y,_min.z);
  glTexCoord2f(1.0f,1.0f);
  glVertex3f(_min.x,_max.y,_min.z);
  glTexCoord2f(0.0f,1.0f);
  glVertex3f(_max.x,_max.y,_min.z);
  glTexCoord2f(0.0f,0.0f);
  glVertex3f(_max.x,_min.y,_min.z);

  // Right
  glNormal3f(1.0f,0.0f,0.0f);
  glTexCoord2f(1.0f,0.0f);
  glVertex3f(_max.x,_min.y,_min.z);
  glTexCoord2f(1.0f,1.0f);
  glVertex3f(_max.x,_max.y,_min.z);
  glTexCoord2f(0.0f,1.0f);
  glVertex3f(_max.x,_max.y,_max.z);
  glTexCoord2f(0.0f,0.0f);
  glVertex3f(_max.x,_min.y,_max.z);

  // Bottom
  glNormal3f(0.0f,-1.0f,0.0f);
  glTexCoord2f(1.0f,0.0f);
  glVertex3f(_min.x,_min.y,_max.z);
  glTexCoord2f(1.0f,1.0f);
  glVertex3f(_min.x,_min.y,_min.z);
  glTexCoord2f(0.0f,1.0f);
  glVertex3f(_max.x,_min.y,_min.z);
  glTexCoord2f(0.0f,0.0f);
  glVertex3f(_max.x,_min.y,_max.z);

  glEnd();
}

void DrawUnitCube(void)
{
  DrawCubeMinMax(MakeVector(-1.0f,-1.0f,-1.0f),
                 MakeVector(1.0f,1.0f,1.0f));
}

@implementation DemoGLView

- (id)initWithFrame:(NSRect)theFrame
{
  NSOpenGLPixelFormatAttribute attribs[] = 
	{NSOpenGLPFAAccelerated,
	 NSOpenGLPFADoubleBuffer,
	 0};
  NSOpenGLPixelFormat *fmt;

  opaque = 1;
    
  /* Choose a pixel format */
  fmt = [[NSOpenGLPixelFormat alloc] initWithAttributes:attribs];

  /* Create a GL context */
  self = [super initWithFrame:theFrame pixelFormat:fmt];
   
  /* Destroy the pixel format */
  [fmt release];

  if(self) {
    NSMutableArray *dragTypes = [NSMutableArray arrayWithObjects:NSFilenamesPboardType, nil];
    [dragTypes addObjectsFromArray:[NSImage imagePasteboardTypes]];
    [self registerForDraggedTypes:dragTypes];

    _orientation = MakeVector(20.0f,45.0f,0.0f);
    [[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(lightingChanged:)
                                                 name:LightingDidChangeNotification object:nil];
    _textureImage = [[NSImage alloc] init];
    [_textureImage setSize:NSMakeSize(512.0f,512.0f)];
    
    _overlayView2D = [[Demo2DOverlayView alloc] initWithFrame:[self bounds]];
    _overlayViewGL = [[DemoGLOverlayView alloc] initWithFrame:[self bounds]];
    
    [self addOverlayView:_overlayView2D ordered:NSWindowAbove];
    _overlayView = _overlayView2D;
  }
  return self;
}

- (IBAction)toggleOverlayType:(id)sender
{
	[self removeOverlayView:_overlayView];
	_overlayView = NULL;
	if([[sender selectedCell] tag] == 0)
	{
		[self addOverlayView:_overlayView2D ordered:NSWindowAbove];
		_overlayView = _overlayView2D;
	}
	else
	{
		[self addOverlayView:_overlayViewGL ordered:NSWindowAbove];
		_overlayView = _overlayViewGL;
	}
}

- (IBAction)toggleOpacity:(id)sender
{
	opaque = [sender intValue];

	[[self openGLContext] setValues:&opaque forParameter:NSOpenGLCPSurfaceOpacity];	
	[self setNeedsDisplay:YES];
}

- (void)setDemoController:(DemoController *)theController
{
  _demoController = theController;
}

- (BOOL)isOpaque
{
	if(opaque)
		return YES;
	else
		return glNeedsDisplay;
}

- (void)lightingChanged:(NSNotification *)theNotification
{
  [self setNeedsDisplay:YES];
}

// Resizing the spit view causes some flicker.  So, as soon as we 
// know the resize is going to happen we use a Carbon call to disable
// screen updates.

// Later when the parent window finally flushes we re-enable updates
// so that everything hits the screen at once.
- (void)splitViewWillResizeSubviews:(NSNotification *)notification
{
	[(DemoWindow *)[self window] disableUpdatesUntilFlush];
}

- (void)drawRect:(NSRect)theRect
{
  NSRect visibleRect;
  float lightModelAmbient[4] = {0.0f,0.0f,0.0f,1.0f};
  float light0Position[4] = {1.0f,1.0f,1.0f,1.0f};
  float light0Ambient[4], light0Diffuse[4], light0Specular[4];
  float materialAmbient[4], materialDiffuse[4], materialSpecular[4], materialEmission[4];
  
  // Get visible bounds...
  visibleRect = [self bounds];

  // Set proper viewport
  glViewport(visibleRect.origin.x, visibleRect.origin.y, visibleRect.size.width, visibleRect.size.height);

  // Clear background to transparent black.
  glClearColor(0.0f,0.0f,0.0f,0.0f);
  glClear(GL_COLOR_BUFFER_BIT);

  // Set up known GL render states.
  glDisable(GL_BLEND);
  glDisable(GL_DEPTH_TEST);

  glCullFace(GL_BACK);
  glEnable(GL_CULL_FACE);
  glEnable(GL_LIGHTING);
  glEnable(GL_LIGHT0);

  // If we have a texture that's been dragged or pasted in, then use it.
  if(_textureImageRep) {
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, 1);
  } else {
    glDisable(GL_TEXTURE_2D);
  }

  // Get current lighting values from UI.
  GetRGBColor([_demoController lightAmbient],light0Ambient);
  GetRGBColor([_demoController lightDiffuse],light0Diffuse);
  GetRGBColor([_demoController lightSpecular],light0Specular);

  // Set GL lighting
  glLightModelfv(GL_LIGHT_MODEL_AMBIENT, lightModelAmbient);
  glLightfv(GL_LIGHT0, GL_AMBIENT, light0Ambient);
  glLightfv(GL_LIGHT0, GL_DIFFUSE, light0Diffuse);
  glLightfv(GL_LIGHT0, GL_SPECULAR, light0Specular);

  // Get current material values from UI
  GetRGBColor([_demoController materialAmbient],materialAmbient);
  GetRGBColor([_demoController materialDiffuse],materialDiffuse);
  GetRGBColor([_demoController materialSpecular],materialSpecular);
  GetRGBColor([_demoController materialEmission],materialEmission);

  // Set OpenGL material properties
  glMaterialfv(GL_FRONT, GL_AMBIENT, materialAmbient);
  glMaterialfv(GL_FRONT, GL_DIFFUSE, materialDiffuse);
  glMaterialfv(GL_FRONT, GL_SPECULAR, materialSpecular);
  glMaterialfv(GL_FRONT, GL_EMISSION, materialEmission);
  glMaterialf(GL_FRONT, GL_SHININESS, [_demoController materialShininess]);

  // Set camera field of view, viewport aspect ratio and near/far clipping planes.
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluPerspective(30.0f,visibleRect.size.width / visibleRect.size.height,1.0f,8192.0f);

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

  // Finally, draw a cube!
  DrawUnitCube();

  // This is an optimization.  If GL isn't the only thing dirty
  // or we're in live resize, don't do a buffer swap.  The GL content
  // will be updated as part of the window flush anyway.
  if(1 || glNeedsDisplay && ![self inLiveResize])
	[[self openGLContext] flushBuffer];
  else
	glFlush();
	
  glNeedsDisplay = NO;
}

- (void)displayGL
{
	glNeedsDisplay = YES;
	[self display];
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
  NSPoint lastPoint, curPoint;
  BOOL special, wait;
  NSDate *distantPast;

  wait = YES;
  lastPoint = curPoint = [self convertPoint:[theEvent locationInWindow] fromView:nil];
  distantPast = [NSDate distantPast];
  if([theEvent modifierFlags] & NSShiftKeyMask)
  {
	_overlayView->doSelection = YES;
	_overlayView->selectionBeginPoint = _overlayView->selectionEndPoint = lastPoint;
  }
  else
	_overlayView->doSelection = NO;
	
  while(1) {
    /* Wait for a single event.. */
	if(wait) {
		theEvent = [[self window] nextEventMatchingMask:(NSLeftMouseDraggedMask | NSLeftMouseUpMask)];
		wait = NO;
	} else {
		theEvent = [[self window] nextEventMatchingMask:(NSLeftMouseDraggedMask | NSLeftMouseUpMask)
		    								  untilDate:distantPast
			    								 inMode:NSDefaultRunLoopMode
				    						    dequeue:YES];
		if(!theEvent) {
			wait = YES;	
			if(_overlayView->doSelection)
			{
				_overlayView->selectionEndPoint = curPoint;
				[_overlayView setNeedsDisplay:YES];            
			}
			else
			{
				glNeedsDisplay = YES;
				[self setNeedsDisplay:YES];
			}
			continue;
		}
	}

    curPoint = [self convertPoint:[theEvent locationInWindow] fromView:nil];
    if(!_overlayView->doSelection)
    {
	special = (([theEvent modifierFlags] & NSAlternateKeyMask) ? YES : NO);
	
	if(!NSEqualPoints(lastPoint, curPoint)) {
		float deltaX, deltaY;
		Vector result;
		
		deltaX = curPoint.x - lastPoint.x;
		deltaY = curPoint.y - lastPoint.y;
		
		if(special) {
			result.x = result.y = 0.0f;
			result.z = -deltaX * 0.5f;
		} else {
			result.x = -deltaY * 0.5f;
			result.y =  deltaX * 0.5f;
			result.z =  0.0f;
		}
		_orientation = SumVectors(_orientation, result);
	}
    }
    lastPoint = curPoint;

    if([theEvent type] == NSLeftMouseUp) {
      break;
    }
  }
  if(_overlayView->doSelection)
  {	
	_overlayView->doSelection = NO;
	[_overlayView setNeedsDisplay:YES];
  }
}

- (void)generateTextureFromImage:(NSImage *)contents
{
  [_sourceImage release];
  _sourceImage = contents;
  [_sourceImage setFlipped:YES];
  [_sourceImage setScalesWhenResized:YES];
  [_sourceImage setSize:NSMakeSize(512.0f,512.0f)];
  [_textureImage lockFocus];
  [_sourceImage compositeToPoint:NSMakePoint(0.0f,0.0f) operation:NSCompositeCopy];
  [_textureImageRep release];
  _textureImageRep = [[NSBitmapImageRep alloc] initWithFocusedViewRect:NSMakeRect(0,0,512,512)];
  [_textureImage unlockFocus];

  [[self openGLContext] makeCurrentContext];
  glEnable(GL_TEXTURE_2D);
  glBindTexture(GL_TEXTURE_2D, 1);
  glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
  gluBuild2DMipmaps(GL_TEXTURE_2D, 4, 512, 512, GL_RGBA, GL_UNSIGNED_BYTE, [_textureImageRep bitmapData]);

  [self setNeedsDisplay:YES];
}

- (void)loadSourceImageFromPasteboard:(NSPasteboard *)pboard
{
  NSString *type = [pboard availableTypeFromArray:[NSImage imagePasteboardTypes]];
  if (type) {
    NSImage *contents = [[NSImage alloc] initWithPasteboard:pboard];
    if (contents) {
      [self generateTextureFromImage:contents];
    }
  }
}

- (void)loadSourceImageFromContentsOfFile:(NSString *)filename
{
  NSString *extension = [filename pathExtension];
  if ([[NSImage imageFileTypes] containsObject:extension]) {
    NSImage *contents = [[NSImage alloc] initWithContentsOfFile:filename];
    if (contents) {
      [self generateTextureFromImage:contents];
    }
  }
}

// Dragging
- (void)updateDragPosition:(id <NSDraggingInfo>)sender {
  //NSPoint locationInView = [self convertPoint:[sender draggingLocation] fromView:nil];
  // Eventually the idea here was to do some select mode stuff and let you drag & drop
  // the image onto a single face.  This hasn't been implemented yet, though.
}

- (unsigned int)dragOperationForDraggingInfo:(id <NSDraggingInfo>)sender {
  NSPasteboard *pboard = [sender draggingPasteboard];
  NSString *type = [pboard availableTypeFromArray:[NSArray arrayWithObjects:NSFilenamesPboardType, nil]];
  if (type) {
    if ([type isEqualToString:NSFilenamesPboardType]) {
      [self updateDragPosition:sender];
      return NSDragOperationCopy;
    }
  }

  type = [pboard availableTypeFromArray:[NSImage imagePasteboardTypes]];
  if (type) {
    [self updateDragPosition:sender];
    return NSDragOperationCopy;
  }
  return NSDragOperationNone;
}

- (unsigned int)draggingEntered:(id <NSDraggingInfo>)sender {
  return [self dragOperationForDraggingInfo:sender];
}

- (unsigned int)draggingUpdated:(id <NSDraggingInfo>)sender {
  return [self dragOperationForDraggingInfo:sender];
}

- (void)draggingExited:(id <NSDraggingInfo>)sender {
  return;
}

- (BOOL)prepareForDragOperation:(id <NSDraggingInfo>)sender {
  return YES;
}

- (BOOL)performDragOperation:(id <NSDraggingInfo>)sender {
  return YES;
}

- (void)concludeDragOperation:(id <NSDraggingInfo>)sender
{
  NSPasteboard *pboard = [sender draggingPasteboard];
  NSString *type = [pboard availableTypeFromArray:[NSArray arrayWithObjects:NSFilenamesPboardType, nil]];

  if (type) {
    if ([type isEqualToString:NSFilenamesPboardType]) {
      NSArray *filenames = [pboard propertyListForType:NSFilenamesPboardType];
      if ([filenames count] == 1) {
        NSString *filename = [filenames objectAtIndex:0];
        [self loadSourceImageFromContentsOfFile:filename];
      }
    }
    return;
  }
  [self loadSourceImageFromPasteboard:pboard];
}

- (IBAction)paste:(id)sender {
  NSPasteboard *pboard = [NSPasteboard generalPasteboard];
  NSString *type = [pboard availableTypeFromArray:[NSArray arrayWithObjects:NSFilenamesPboardType, nil]];

  if (type) {
    if ([type isEqualToString:NSFilenamesPboardType]) {
      NSArray *filenames = [pboard propertyListForType:NSFilenamesPboardType];
      if ([filenames count] == 1) {
        NSString *filename = [filenames objectAtIndex:0];
        [self loadSourceImageFromContentsOfFile:filename];
      }
    }
    return;
  }
 [self loadSourceImageFromPasteboard:pboard];    
}

@end
