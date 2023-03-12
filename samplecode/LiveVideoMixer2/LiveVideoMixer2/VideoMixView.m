/*

File: VideoMixView.m

Abstract:   The VideoMixView is a subclass of NSOpenGLView.
	    Here you find the code for the view interaction as
	    well as the setup of the OpenGL context with the
	    setting up of the OpenGL state.

Version: 1.0

å© Copyright 2005 Apple Computer, Inc. All rights reserved.

IMPORTANT:  This Apple software is supplied to 
you by Apple Computer, Inc. ("Apple") in 
consideration of your agreement to the following 
terms, and your use, installation, modification 
or redistribution of this Apple software 
constitutes acceptance of these terms.  If you do 
not agree with these terms, please do not use, 
install, modify or redistribute this Apple 
software.

In consideration of your agreement to abide by 
the following terms, and subject to these terms, 
Apple grants you a personal, non-exclusive 
license, under Apple's copyrights in this 
original Apple software (the "Apple Software"), 
to use, reproduce, modify and redistribute the 
Apple Software, with or without modifications, in 
source and/or binary forms; provided that if you 
redistribute the Apple Software in its entirety 
and without modifications, you must retain this 
notice and the following text and disclaimers in 
all such redistributions of the Apple Software. 
Neither the name, trademarks, service marks or 
logos of Apple Computer, Inc. may be used to 
endorse or promote products derived from the 
Apple Software without specific prior written 
permission from Apple.  Except as expressly 
stated in this notice, no other rights or 
licenses, express or implied, are granted by 
Apple herein, including but not limited to any 
patent rights that may be infringed by your 
derivative works or by other works in which the 
Apple Software may be incorporated.

The Apple Software is provided by Apple on an "AS 
IS" basis.  APPLE MAKES NO WARRANTIES, EXPRESS OR 
IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED 
WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY 
AND FITNESS FOR A PARTICULAR PURPOSE, REGARDING 
THE APPLE SOFTWARE OR ITS USE AND OPERATION ALONE 
OR IN COMBINATION WITH YOUR PRODUCTS.

IN NO EVENT SHALL APPLE BE LIABLE FOR ANY 
SPECIAL, INDIRECT, INCIDENTAL OR CONSEQUENTIAL 
DAMAGES (INCLUDING, BUT NOT LIMITED TO, 
PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS 
OF USE, DATA, OR PROFITS; OR BUSINESS 
INTERRUPTION) ARISING IN ANY WAY OUT OF THE USE, 
REPRODUCTION, MODIFICATION AND/OR DISTRIBUTION OF 
THE APPLE SOFTWARE, HOWEVER CAUSED AND WHETHER 
UNDER THEORY OF CONTRACT, TORT (INCLUDING 
NEGLIGENCE), STRICT LIABILITY OR OTHERWISE, EVEN 
IF APPLE HAS BEEN ADVISED OF THE POSSIBILITY OF 
SUCH DAMAGE.

*/ 

#import "VideoMixView.h"
#import <OpenGL/gl.h>
#import <OpenGL/glext.h>
#import "LiveVideoMixerController.h"

@implementation VideoMixView


//--------------------------------------------------------------------------------------------------

- (void)awakeFromNib
{
    // setup the pthread lock. We will use this lock to make sure the OpenGL surface is only
    // getting called from one thread at a time.
    pthread_mutexattr_t attr;
    
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);	// the recursive thread allows us to aquire the same lock again from the same thread
								// but still lock against another thread

    pthread_mutex_init(&drawingLock, &attr);
		
}

//--------------------------------------------------------------------------------------------------

- (void)setupSharedContext:(NSRect)targetRect
{
    // in this example we use a shared context for some of the texture operations. Shared contexts help to get around thread safety issues
    sharedContextTargetRect = targetRect;
    
    NSOpenGLPixelFormatAttribute attributes[] = {
								NSOpenGLPFAColorSize,  24,
								NSOpenGLPFAAlphaSize,  8,
								0,
							};
	    
    // create an offscreen OpenGL surface for the VideoOut - this allows us to render just the composited main video in QuickTime coordinates
    // without touching our output on the screen
    NSOpenGLPixelFormat *format = [[(NSOpenGLPixelFormat*)[NSOpenGLPixelFormat alloc] initWithAttributes:attributes] autorelease];
    _offscreenWindow = [[NSWindow alloc] initWithContentRect:targetRect styleMask:NSBorderlessWindowMask backing:NSBackingStoreNonretained defer:NO];
    _offscreenOpenGLView = [[NSOpenGLView alloc] initWithFrame:targetRect pixelFormat:format];
    [[_offscreenWindow contentView] addSubview:_offscreenOpenGLView];
    [_offscreenOpenGLView display];
	    

	    
    sharedContext = [[NSOpenGLContext alloc] initWithFormat:format shareContext:nil];
    
    [self setOpenGLContext:[[[NSOpenGLContext alloc] initWithFormat:[NSOpenGLView defaultPixelFormat] shareContext:sharedContext] autorelease]];
    [[self openGLContext] setView:self];
    
    [sharedContext setView: _offscreenOpenGLView];
	    
    [sharedContext makeCurrentContext];
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glEnable( GL_BLEND );				    // enable blending
    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );    // blend func for non premultiplied images
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    glHint(GL_CLIP_VOLUME_CLIPPING_HINT_EXT, GL_FASTEST);
    glViewport(0, 0, sharedContextTargetRect.size.width ,sharedContextTargetRect.size.height);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, sharedContextTargetRect.size.width, 0, sharedContextTargetRect.size.height, -1.0, 1.0);

    glClearColor(1.0, 0.0, 0.0, 1.0);

    glClear(GL_COLOR_BUFFER_BIT);
	    
}

//--------------------------------------------------------------------------------------------------
- (NSOpenGLContext*)sharedContext
{
    return sharedContext;
}

//--------------------------------------------------------------------------------------------------

- (void)dealloc
{
    [sharedContext release];
    [_offscreenWindow release];
    [_offscreenOpenGLView release];
    pthread_mutex_destroy(&drawingLock);
    [super dealloc];
}

//--------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------

- (BOOL)isFlipped
{
    return YES;
}

//--------------------------------------------------------------------------------------------------

- (void)lock
{
    pthread_mutex_lock(&drawingLock);
    if(contextIsInitialized)
		[[self openGLContext] makeCurrentContext];
}

//--------------------------------------------------------------------------------------------------

- (void)unlock
{
    pthread_mutex_unlock(&drawingLock);
}

//--------------------------------------------------------------------------------------------------

- (void)update
{
    // The NSOpenGLView issues OpenGL calls in its update method. Therefore it is important to lock
    // around this call as it would otherwise run in conflict with our rendering thread
    [self lock];
    [super update];
    [self unlock];
}

//--------------------------------------------------------------------------------------------------

- (void)reshape		// scrolled, moved or resized
{
    needsReshape = YES;	// reset the viewport etc. on the next draw
    
    // if we are not playing, force an immediate draw otherwise it will update with the next frame 
    // coming through. This makes the resize performance better as it reduces the number of redraws
    // espcially on the main thread
    if(![controller isPlaying])	
    {
	[self setDirty:YES];
	[self flushOutput];
    }
}

//--------------------------------------------------------------------------------------------------

- (void)clear
{
    [self lock];
    // clear content
    glClearColor(0.0, 0.0, 0.0, 0.0);
    glClear(GL_COLOR_BUFFER_BIT);
    [self unlock];
}

//--------------------------------------------------------------------------------------------------

// flushOutput is our drawing call. It makes sure that the view coordinate system is setup
// properly and causes the controller object to draw its content if needed.
- (void)flushOutput
{
    [self lock];
    if(needsReshape)
    {
	NSRect		frame = [self frame];
	NSRect		bounds = [self bounds];
	GLfloat 	minX, minY, maxX, maxY;

	minX = NSMinX(bounds);
	minY = NSMinY(bounds);
	maxX = NSMaxX(bounds);
	maxY = NSMaxY(bounds);

	[[self openGLContext] makeCurrentContext];
	[self update]; 

	// the first time we need to setup the OpenGL environment in this view
	if(!contextIsInitialized)
	{
	    long swapInterval = 1;
	    
	    [[self openGLContext] setValues:&swapInterval forParameter:NSOpenGLCPSwapInterval];	// synchronize the glFlush with the VBL
	    glMatrixMode(GL_PROJECTION);
	    glLoadIdentity();
	    glEnable( GL_BLEND );				    // enable blending
	    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );    // blend func for non premultiplied images
	    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	    glHint(GL_CLIP_VOLUME_CLIPPING_HINT_EXT, GL_FASTEST);
	    contextIsInitialized = YES;
	}

	if(NSIsEmptyRect([self visibleRect])) 
	{
	    glViewport(0, 0, 1, 1);
	} else {
	    glViewport(0, 0,  frame.size.width ,frame.size.height);
	}
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(minX, maxX, maxY, minY, -1.0, 1.0);

	glClearColor(0.0, 0.0, 0.0, 0.0);

	glClear(GL_COLOR_BUFFER_BIT);
	needsReshape = NO;
    }
    if([self dirty])
    {
	[controller drawContent:[controller mainVideoRect] mainVideoOnly:NO];   // invoke the controller to draw all the video channels
	// render the main video in the shared context for VideoOut in full size
	[sharedContext makeCurrentContext];
	[controller drawContent:sharedContextTargetRect mainVideoOnly:YES];   // invoke the controller to draw all the video channels	
    }
    [self setDirty:NO];
    [self unlock];
}

//--------------------------------------------------------------------------------------------------

- (void)setDirty:(BOOL)inDirty
{
    dirty = inDirty;
}

//--------------------------------------------------------------------------------------------------

- (BOOL)dirty
{
    return dirty;
}

//--------------------------------------------------------------------------------------------------

- (void)mouseDown:(NSEvent *)theEvent
{
    // mouse handling is done in the controller as it handles the video channels 
    [controller mouseDown:(NSEvent *)theEvent];	
}


@end
