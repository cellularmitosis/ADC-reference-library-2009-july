/*
Project: CIColorTracking

File: VideoCIView.m

Abstract: 
This is the implementation file for VideoCIView, a class that sets up and manages the OpenGL context that shows the video.

Version 1.0

Disclaimer: IMPORTANT:  This Apple software is supplied to you by Apple Inc.
("Apple") in consideration of your agreement to the following terms, and your
use, installation, modification or redistribution of this Apple software
constitutes acceptance of these terms.  If you do not agree with these terms,
please do not use, install, modify or redistribute this Apple software.

In consideration of your agreement to abide by the following terms, and subject
to these terms, Apple grants you a personal, non-exclusive license, under
Apple's copyrights in this original Apple software (the "Apple Software"), to
use, reproduce, modify and redistribute the Apple Software, with or without
modifications, in source and/or binary forms; provided that if you redistribute
the Apple Software in its entirety and without modifications, you must retain
this notice and the following text and disclaimers in all such redistributions
of the Apple Software.
Neither the name, trademarks, service marks or logos of Apple Inc. may be used
to endorse or promote products derived from the Apple Software without specific
prior written permission from Apple.  Except as expressly stated in this notice,
no other rights or licenses, express or implied, are granted by Apple herein,
including but not limited to any patent rights that may be infringed by your
derivative works or by other works in which the Apple Software may be
incorporated.

The Apple Software is provided by Apple on an "AS IS" basis.  APPLE MAKES NO
WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED
WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS FOR A PARTICULAR
PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND OPERATION ALONE OR IN
COMBINATION WITH YOUR PRODUCTS.

IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION, MODIFICATION AND/OR
DISTRIBUTION OF THE APPLE SOFTWARE, HOWEVER CAUSED AND WHETHER UNDER THEORY OF
CONTRACT, TORT (INCLUDING NEGLIGENCE), STRICT LIABILITY OR OTHERWISE, EVEN IF
APPLE HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

Copyright (C) 2008 Apple Inc. All Rights Reserved.


*/



#import "VideoCIView.h"
#import <OpenGL/OpenGL.h>
#import <OpenGL/gl.h>

@implementation VideoCIView

+ (NSOpenGLPixelFormat *)defaultPixelFormat
{
    static NSOpenGLPixelFormat *pf;

    if (pf == nil)
    {
	// You must make sure that the pixel format of the context does not 
	// have a recovery renderer is important. Otherwise CoreImage may not be able to
	// create contexts that share textures with this context.

	static const NSOpenGLPixelFormatAttribute attr[] = {
	    NSOpenGLPFAAccelerated,
	    NSOpenGLPFANoRecovery,
	    NSOpenGLPFAColorSize, 32,
#if MAC_OS_X_VERSION_MAX_ALLOWED > MAC_OS_X_VERSION_10_4
		NSOpenGLPFAAllowOfflineRenderers,
#endif
	    0
	};

	pf = [[NSOpenGLPixelFormat alloc] initWithAttributes:(void *)&attr];
    }

    return pf;
}

- (void)dealloc
{
    [_image release];
    [_context release];

    [super dealloc];
}

- (CIImage *)image
{
    return [[_image retain] autorelease];
}

- (void)setImage:(CIImage *)image
{
	if (_image != image)
    {
		[_image release];
		_image = [image retain];
	}
	[self render];
}

- (void)prepareOpenGL
{
    GLint parm = 1;

    //  Set the swap interval to 1 to ensure that buffers swaps occur only during the vertical retrace of the monitor.

    [[self openGLContext] setValues:&parm forParameter:NSOpenGLCPSwapInterval];

    // To ensure best performance, disbale everything you don't need. 
    
    glDisable (GL_ALPHA_TEST);
    glDisable (GL_DEPTH_TEST);
    glDisable (GL_SCISSOR_TEST);
    glDisable (GL_BLEND);
    glDisable (GL_DITHER);
    glDisable (GL_CULL_FACE);
    glColorMask (GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    glDepthMask (GL_FALSE);
    glStencilMask (0);
    glClearColor (0.0f, 0.0f, 0.0f, 0.0f);
    glHint (GL_TRANSFORM_HINT_APPLE, GL_FASTEST);
    _needsReshape = YES;
}

// Called when the user scrolls, moves, or resizes the view.
- (void)reshape		
{
    // Resets the viewport on the next draw operation.
    _needsReshape = YES;	
}

- (void)updateMatrices
{
    NSRect	visibleRect = [self visibleRect];
    NSRect	mappedVisibleRect = NSIntegralRect([self convertRect: visibleRect toView: [self enclosingScrollView]]);
    
    [[self openGLContext] update];

    // Install an orthographic projection matrix (no perspective)
    // with the origin in the bottom left and one unit equal to one device pixel.  

    glViewport (0, 0,mappedVisibleRect.size.width, mappedVisibleRect.size.height);

    glMatrixMode (GL_PROJECTION);
    glLoadIdentity ();
    glOrtho(visibleRect.origin.x,
		visibleRect.origin.x + visibleRect.size.width,
		visibleRect.origin.y,
		visibleRect.origin.y + visibleRect.size.height,
		-1, 1);

    glMatrixMode (GL_MODELVIEW);
    glLoadIdentity ();
    _needsReshape = NO;
}

- (void)render
{
    NSRect		frame = [self bounds];

    [[self openGLContext] makeCurrentContext];

	if (_needsReshape)
	{
	    [self updateMatrices];
	    glClear (GL_COLOR_BUFFER_BIT);
	}

	CGRect		imageRect = [_image extent];
	CGRect		destRect = *((CGRect*)&frame);
	
	[[self ciContext] drawImage:_image inRect:destRect fromRect:imageRect];

	// Flush the OpenGL command stream. If the view is double-buffered
	// you should  replace  this call with [[self openGLContext]

	glFlush ();
    
}

- (CIContext*)ciContext
{
    // Allocate a CoreImage rendering context using the view's OpenGL
    // context as its destination if none already exists. 
    // You must do this before sending any queries to the CIContext. 

    if (_context == nil)
    {
	[[self openGLContext] makeCurrentContext];
	NSOpenGLPixelFormat *pf;

	pf = [self pixelFormat];
	if (pf == nil)
	    pf = [[self class] defaultPixelFormat];

	_context = [[CIContext contextWithCGLContext: CGLGetCurrentContext()
		     pixelFormat: [pf CGLPixelFormatObj] options: nil] retain];
    }
    return _context;
}

@end
