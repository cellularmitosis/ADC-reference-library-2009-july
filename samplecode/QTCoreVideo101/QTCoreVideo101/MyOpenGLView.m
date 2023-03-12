/*

File: MyOpenGLView.m of QTCoreVideo101

Author: QuickTime DTS

Change History (most recent first): <3> 04/15/08 added automatic texture coordinate generation to wrap the
												 video nicely around the teapot with proper scaling
									<2> 06/14/05 call QTVisualContextTask while owning lock
                                                 overide and add lock around update
                                                 add clean up code
                                    <1> 05/23/05 initial release

� Copyright 2005-2008 Apple Computer, Inc. All rights reserved.

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

#import "MyOpenGLView.h"

#pragma mark Render Callback
// this is the CoreVideo DisplayLink callback notifying the application when the display will need each frame
// and is called when the DisplayLink is running -- in response, we call our getFrameForTime method
static CVReturn MyRenderCallback(CVDisplayLinkRef displayLink, 
								 const CVTimeStamp *inNow, 
								 const CVTimeStamp *inOutputTime, 
								 CVOptionFlags flagsIn, 
								 CVOptionFlags *flagsOut, 
                                 void *displayLinkContext)
{
	return [(MyOpenGLView *)displayLinkContext getFrameForTime:inOutputTime flagsOut:flagsOut];
}

#pragma mark
@implementation MyOpenGLView

// initialize
-(void)awakeFromNib
{
	movie = nil;
    displayLink = nil;
	textureContext = NULL;
    teapotList = 0;
    
    // we need a lock around our draw function so two different
    // threads don't try and draw at the same time
    lock = [NSRecursiveLock new];
    
    // use the teapot or the quad
    drawTeapot = TRUE;
}

- (void) dealloc {
	[self cleanUp];
    [super dealloc];
}

// it is very important that we clean up the rendering
// objects before the view is disposed, remember that with the
// display link running you're applications render callback may be
// called at any time including when the application is quitting or the
// view is being disposed, additionally you need to make sure you're not
// consuming OpenGL resources or leaking textures -- this clean up routine
// makes sure to stop and release everything
-(void)cleanUp
{
	// stop and release the movie
    if (movie) {
    	[movie setRate:0.0];
        SetMovieVisualContext([movie quickTimeMovie], NULL);
        [movie release];
        movie = nil;
    }
    
    // it is critical to dispose of the display link
    if (displayLink) {
    	CVDisplayLinkStop(displayLink);
        CVDisplayLinkRelease(displayLink);
        displayLink = NULL;
    }
    
    // don't leak textures
    if (currentFrame) {
    	CVOpenGLTextureRelease(currentFrame);
        currentFrame = NULL;
    }

	// release the OpenGL Texture Context
    if (textureContext) {
    	 CFRelease(textureContext);
         textureContext = NULL;
    }
    
    if (lock) {
    	[lock release];
        lock = nil;
    }
}

// set up the OpenGL environs
- (void)prepareOpenGL
{
	long swapInterval = 1;
    
    glShadeModel(GL_SMOOTH);                // enable smooth shading
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);   // black background
    glClearDepth(1.0f);                     // depth buffer setup
    glEnable(GL_DEPTH_TEST);                // enable depth testing
    glDepthFunc(GL_LEQUAL);                 // type of depth test to do

    // really nice perspective calculations
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
	
	// turn on sphere map automatic texture coordinate generation
	// http://www.opengl.org/sdk/docs/man/xhtml/glTexGen.xml
	glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);
	glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);

    // make the teapot display list
    teapotList = glGenLists(1);
    glNewList(teapotList, GL_COMPILE);
    glutSolidTeapot(1.0);
    glEndList();
   
   	// set up the GL contexts swap interval -- passing 1 means that
    // the buffers are swapped only during the vertical retrace of the monitor
	[[self openGLContext] setValues:&swapInterval forParameter:NSOpenGLCPSwapInterval];
   
    // create display link for the main display
    CVDisplayLinkCreateWithCGDisplay(kCGDirectMainDisplay, &displayLink);
    if (NULL != displayLink) {
    	// set the current display of a display link.
    	CVDisplayLinkSetCurrentCGDisplay(displayLink, kCGDirectMainDisplay);
        
        // set the renderer output callback function
    	CVDisplayLinkSetOutputCallback(displayLink, &MyRenderCallback, self);
        
        // activates a display link.
    	CVDisplayLinkStart(displayLink);
    }
    
	// creates a new OpenGL texture context for a specified OpenGL context and pixel format
	QTOpenGLTextureContextCreate(kCFAllocatorDefault,										// an allocator to Create functions
    							 CGLContextObj([[self openGLContext] CGLContextObj]),		// the OpenGL context
                                 CGLPixelFormatObj([[self pixelFormat] CGLPixelFormatObj]), // pixelformat object that specifies buffer types and other attributes of the context
                                 NULL,														// a CF Dictionary of attributes
                                 &textureContext);											// returned OpenGL texture context
}

// good practice to lock around update
- (void)update
{
    [lock lock];
    	[super update];
    [lock unlock];
}

//  adjust the viewport
- (void)reshape
{ 
    NSRect sceneBounds = [self bounds];

    // reset current viewport
    glViewport(0, 0, GLsizei(sceneBounds.size.width), GLsizei(sceneBounds.size.height));
    glMatrixMode(GL_PROJECTION);   // select the projection matrix
    glLoadIdentity();              // reset it

    // calculate the aspect ratio of the view
    gluPerspective(45.0f, sceneBounds.size.width / sceneBounds.size.height, 0.1f, 100.0f);
    glMatrixMode(GL_MODELVIEW);    // select the modelview matrix
    glLoadIdentity();              // reset it
}

// draw
- (void)drawRect:(NSRect)rect
{  
	static GLfloat angle = 0; // angle of rotation
    
    [lock lock]; // prevent drawing from another thread if we're drawing already
    
    	// make the GL context the current context
        [[self openGLContext] makeCurrentContext];
        
        // some set up
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    	
        glMatrixMode(GL_TEXTURE);
        glLoadIdentity();
        
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        
        glTranslatef(0.0f, 0.0f, -3.0f);
    	glRotatef(angle, 0.0f, 1.0f, 0.0f);
        
        if (NULL != currentFrame) {
        	// we have a frame so draw something
            
            GLfloat topLeft[2], topRight[2], bottomRight[2], bottomLeft[2];
            
    		GLenum target = CVOpenGLTextureGetTarget(currentFrame);	// get the texture target (for example, GL_TEXTURE_2D) of the texture
    		GLint name = CVOpenGLTextureGetName(currentFrame);		// get the texture target name of the texture
            
            // get the texture coordinates for the part of the image that should be displayed
            CVOpenGLTextureGetCleanTexCoords(currentFrame, bottomLeft, bottomRight, topRight, topLeft);
            
            if (TRUE == drawTeapot) {
				// enable automatic texture coordinate generation
				glEnable(GL_TEXTURE_GEN_S);
				glEnable(GL_TEXTURE_GEN_T);
				
           	 	glPushMatrix();
                
                // since we get GL_TEXTURE_RECTANGLE_EXT textures which use pixel coordinates
                // rather than normalized coordinates, we need to scale the texturing matrix
                glMatrixMode(GL_TEXTURE);
				glScalef(movieSize.width, movieSize.height, 1);	// to rotate without skewing or translation, we must be in 0-centered normalized texture coordinates
            	
				// bind the texture and draw the teapot
            	glEnable(target);
                glBindTexture(target, name);
				glMatrixMode(GL_MODELVIEW);
                glPopMatrix();
                glCallList(teapotList);
            	
				glDisable(target);
				glDisable(GL_TEXTURE_GEN_S);
				glDisable(GL_TEXTURE_GEN_T);
            } else {
                glPushMatrix();
                
                glScaled(movieSize.width / movieSize.height, 1.0, 1.0);
                
                // bind the texture and draw the quad
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
            
        	// increment the rotation angle
			angle += 0.4f;
        }
        
        glFlush();
        
        // give time to the Visual Context so it can release internally held resources for later re-use
        // this function should be called in every rendering pass, after old images have been released, new
        // images have been used and all rendering has been flushed to the screen.
        QTVisualContextTask(textureContext);
    
    [lock unlock];
}

#pragma mark Display Link
// getFrameForTime is called from the Display Link callback when it's time for us to check to see
// if we have a frame available to render -- if we do, draw -- if not, just task the Visual Context and split
- (CVReturn)getFrameForTime:(const CVTimeStamp*)timeStamp flagsOut:(CVOptionFlags*)flagsOut
{
	// there is no autorelease pool when this method is called because it will be called from another thread
    // it's important to create one or you will leak objects
	NSAutoreleasePool *pool = [NSAutoreleasePool new];
	
	// check for new frame
	if (textureContext != NULL && QTVisualContextIsNewImageAvailable(textureContext, timeStamp)) {
    	
        // if we have a previous frame release it
		if (NULL != currentFrame) {
        	CVOpenGLTextureRelease(currentFrame);
        	currentFrame = NULL;
        }
        
        // get a "frame" (image buffer) from the Visual Context, indexed by the provided time
		OSStatus status = QTVisualContextCopyImageForTime(textureContext, NULL, timeStamp, &currentFrame);
		
        // the above call may produce a null frame so check for this first
        // if we have a frame, then draw it
		if ((noErr == status) && (NULL != currentFrame)) {
        	[self drawRect:NSZeroRect];
		}
	}
    
    [pool release];

	return kCVReturnSuccess;
}

#pragma mark Movie
// open a Movie File and instantiate a QTMovie object
-(void)openMovie:(NSString*)path
{
	if (textureContext != nil) {
        
        // if we already have a QTMovie release it
        if (nil != movie) [movie release];
        
        movie = [[QTMovie alloc] initWithFile:path error:nil];
        
        // get the Movie size
        [[movie attributeForKey:QTMovieNaturalSizeAttribute] getValue:&movieSize];
        
        // set Movie to loop
        [movie setAttribute:[NSNumber numberWithBool:YES] forKey:QTMovieLoopsAttribute];
        
        // targets a Movie to render into a visual context
        SetMovieVisualContext([movie quickTimeMovie], textureContext);
        
        // play the Movie
        [movie setRate:1.0];

		// set the window title from the Movie if it has a name associated with it
        [[self window] setTitle:[movie attributeForKey:QTMovieDisplayNameAttribute]];
    }
}

#pragma mark Accessors
-(CVDisplayLinkRef)displayLink
{
	return displayLink;
}

// control whether or not we'll draw the teapot
-(void)setDrawTeapotState:(BOOL)inState
{
	drawTeapot = inState;
}

@end