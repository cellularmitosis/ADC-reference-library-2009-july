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

#import "DemoGLOverlayView.h"
#import <OpenGL/gl.h>

@implementation DemoGLOverlayView

- (id)initWithFrame:(NSRect)frame
{
	NSOpenGLPixelFormatAttribute attribs[] = 
		{NSOpenGLPFAAccelerated,
		NSOpenGLPFADoubleBuffer,
		0};
	NSOpenGLPixelFormat *fmt;
	
	/* Choose a pixel format */
	fmt = [[NSOpenGLPixelFormat alloc] initWithAttributes:attribs];
	
	/* Create a GL context */
	self = [super initWithFrame:frame];

	openGLContext = [[NSOpenGLContext alloc] initWithFormat:fmt shareContext:nil];

	[[NSNotificationCenter defaultCenter]
		addObserver:self
		selector:@selector(globalFrameChanged:)
		name:NSViewGlobalFrameDidChangeNotification
		object:self];
	
	/* Destroy the pixel format */
	[fmt release];

	if(self)
	{
		long opaque = 0;
		[openGLContext setValues:&opaque forParameter:NSOpenGLCPSurfaceOpacity];		
	}
	return self;
}

- (void)animate:(NSTimer *)timer
{
	glNeedsDisplay = YES;
	[super animate:timer];
}

- (BOOL)isOpaque
{
	return glNeedsDisplay;
}

- (void)vewWillMoveToWindow:(NSWindow *)window
{
	if(window != [self window])
		[openGLContext clearDrawable];
}

- (void)viewDidMoveToWindow
{
	if([self window])
		[openGLContext setView:self];
}

- (void)globalFrameChanged:(NSNotification *)note
{
	[openGLContext update];
}

- (void)drawRect:(NSRect)rect 
{
    NSRect bounds = [self bounds];
    int i;
    
    // Drawing code here.
    if(!glNeedsDisplay)
    {
	[[NSColor clearColor] set];
	NSRectFill(bounds);
    }
   
    [openGLContext makeCurrentContext];
    
    glViewport(0,0,bounds.size.width,bounds.size.height);
    glClearColor(0,0,0,0);
    glClear(GL_COLOR_BUFFER_BIT);
    
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glOrtho(0,bounds.size.width,0,bounds.size.height,-1,1);
    
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_LINE_SMOOTH);

    glBegin(GL_LINES);
    for(i = 0; i < NUM_LINES; i++)
    {
	float f = (i+1)/(double)NUM_LINES;
	glColor4f(1.0f,1.0f*f,0.0f,1.0f*f);
	glVertex2f(lines[i][0].x,lines[i][0].y);
	glVertex2f(lines[i][1].x,lines[i][1].y);	
    }
    glEnd();
    glDisable(GL_LINE_SMOOTH);
    
    if(doSelection)
    {
        float minX, maxX, minY, maxY;
        
        minX = MIN(selectionBeginPoint.x,selectionEndPoint.x);
        maxX = MAX(selectionBeginPoint.x,selectionEndPoint.x);
        minY = MIN(selectionBeginPoint.y,selectionEndPoint.y);
        maxY = MAX(selectionBeginPoint.y,selectionEndPoint.y);
	glColor4f(0.0f,0.0f,1.0f,0.3f);
	glRectf(minX,minY,maxX,maxY);
	glColor4f(1.0f,1.0f,1.0f,0.8f);
	glBegin(GL_LINE_LOOP);
	glVertex2f(minX+0.5f,minY+0.5f);
	glVertex2f(maxX-0.5f,minY+0.5f);
	glVertex2f(maxX-0.5f,maxY-0.5f);
	glVertex2f(minX+0.5f,maxY-0.5f);
	glEnd();
    }    
    
    if(glNeedsDisplay)
	[openGLContext flushBuffer];
    else
	glFlush();
	
    glNeedsDisplay = NO;
    
    [NSOpenGLContext clearCurrentContext];
}

@end
