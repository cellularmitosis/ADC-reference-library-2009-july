/*

File: SampleCIScrollableView.m

Abstract:   The SampleCIScrollableView builds on the SimpleCIView
	    from other Core Image sample codes. It is modified to
	    support scrolling by setting up the OpenGL coordinate
	    system accordingly.

Version: 1.0

© Copyright 2005 Apple Computer, Inc. All rights reserved.

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

#import "SampleCIScrollableView.h"

#import <OpenGL/OpenGL.h>
#import <OpenGL/gl.h>

@implementation SampleCIScrollableView

+ (NSOpenGLPixelFormat *)defaultPixelFormat
{
    static NSOpenGLPixelFormat *pf;

    if (pf == nil)
    {
	/* Making sure the context's pixel format doesn't have a recovery
	 * renderer is important - otherwise CoreImage may not be able to
	 * create deeper context's that share textures with this one. */

	static const NSOpenGLPixelFormatAttribute attr[] = {
	    NSOpenGLPFAAccelerated,
	    NSOpenGLPFANoRecovery,
	    NSOpenGLPFAColorSize, 32,
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

- (void)setImage:(CIImage *)image dirtyRect:(CGRect)r
{
    if (_image != image)
    {
	[_image release];
	_image = [image retain];

	if (CGRectIsInfinite (r))
	    [self setNeedsDisplay:YES];
	else
	    [self setNeedsDisplayInRect:*(NSRect *)&r];
    }
}

- (void)setImage:(CIImage *)image
{
    [self setImage:image dirtyRect:CGRectInfinite];
}

- (void)prepareOpenGL
{
    long parm = 1;

    /* Enable beam-synced updates. */

    [[self openGLContext] setValues:&parm forParameter:NSOpenGLCPSwapInterval];

    /* Make sure that everything we don't need is disabled. Some of these
     * are enabled by default and can slow down rendering. */

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


- (void)reshape		// scrolled, moved or resized
{
    _needsReshape = YES;	// reset the viewport etc. on the next draw
}

- (void)updateMatrices
{
    NSRect	visibleRect = [self visibleRect];
    NSRect	mappedVisibleRect = NSIntegralRect([self convertRect: visibleRect toView: [self enclosingScrollView]]);
    
    [[self openGLContext] update];

    /* Install an orthographic projection matrix (no perspective)
     * with the origin in the bottom left and one unit equal to one
     * device pixel. */

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

- (void)drawRect:(NSRect)r
{
    CGRect ir;
    CGImageRef cgImage;

    [[self openGLContext] makeCurrentContext];

    if ([NSGraphicsContext currentContextDrawingToScreen])
    {
	if (_needsReshape)
	{
	    [self updateMatrices];
	    r = [self visibleRect];
	    glClear (GL_COLOR_BUFFER_BIT);
	}
	ir = CGRectIntegral (*(CGRect *)&r);


	if ([self respondsToSelector:@selector (drawRect:inCIContext:)])
	{
	    [self drawRect:*(NSRect *)&ir inCIContext:[self ciContext]];
	}
	else if (_image != nil)
	{
	    [[self ciContext] drawImage:_image atPoint:ir.origin fromRect:ir];
	}

	/* Flush the OpenGL command stream. If the view is double
	 * buffered this should be replaced by [[self openGLContext]
	 * flushBuffer]. */

	glFlush ();
    }
    else
    {
	/* Printing the view contents. Render using CG, not OpenGL. */
	ir = CGRectIntegral (*(CGRect *)&r);

	if ([self respondsToSelector:@selector (drawRect:inCIContext:)])
	{
	    [self drawRect:*(NSRect *)&ir inCIContext:[self ciContext]];
	}
	else if (_image != nil)
	{
	    cgImage = [[self ciContext] createCGImage:_image fromRect:ir];

	    if (cgImage != NULL)
	    {
		CGContextDrawImage ([[NSGraphicsContext currentContext]
				     graphicsPort], ir, cgImage);
		CGImageRelease (cgImage);
	    }
	}
    }
}

- (CIContext*)ciContext
{
    /* Allocate a CoreImage rendering context using the view's OpenGL
     * context as its destination if none already exists. 
     * Make sure this is done before somebody queries the ciContext. */

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
