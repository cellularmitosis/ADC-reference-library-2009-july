/*
    File:  HybridImageView.m

    Abstract:  An NSView subclass that can toggle between OpenGL and Cocoa/Quartz rendering
     
    Version:  1.0

    Copyright:  © Copyright 2005 Apple Computer, Inc. All rights reserved.

    Disclaimer: IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc.
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

#import "HybridImageView.h"
#import <OpenGL/OpenGL.h>
#import <OpenGL/gl.h>
#import <QuartzCore/CoreImage.h>

@interface HybridImageView (Internals)
- (NSPoint)originForCenteredImage;
- (void)drawUsingQuartzInRect:(NSRect)rect;
- (void)drawUsingOpenGLInRect:(NSRect)viewRect;

// OpenGL drawing support
- (NSOpenGLPixelFormat *)pixelFormat;
- (NSOpenGLContext *)openGLContext;
- (void)setupOpenGL;
- (void)updateMatrices;
- (void)clearGLContext;
- (CIContext *)CIContext;
@end

@implementation HybridImageView

// Initializes the attributes that HybridImageView adds to NSView.  This method is called by -initWithFrame:, and is also used by our Interface Builder palette.
- (void)initAddedProperties {
    [self setBackgroundColor:[NSColor colorWithDeviceRed:0.75 green:0.75 blue:0.75 alpha:1.0]];
}

- (id)initWithFrame:(NSRect)frame {
    self = [super initWithFrame:frame];
    if (self != nil) {
        [self initAddedProperties];
    }
    return self;
}

- (void)dealloc {
    [image release];
    [ciContext release];
    [openGLContext release];
    [pixelFormat release];
    [backgroundColor release];
    [super dealloc];
}

#pragma mark -
#pragma mark *** Key-Value Binding Support ***

+ (void)initialize {
    if (self == [HybridImageView class]) {
        // Declare the attributes of this class that should be shown in the Bindings inspector in Interface Builder.
        [self exposeBinding:@"backgroundColor"];
    }
}

#pragma mark -
#pragma mark *** NSView Method Overrides ***

- (BOOL)isOpaque {
    return YES;
}

- (void)drawRect:(NSRect)rect {
    if ([image isKindOfClass:[CIImage class]]) {
        [self drawUsingOpenGLInRect:rect];
    } else {
        [self drawUsingQuartzInRect:rect];
    }
}

#pragma mark -
#pragma mark *** Accessors ***

- (id)image {
    return [[image retain] autorelease];
}

- (void)setImage:(id)newImage {
    if (image != newImage) {
        [image release];
        image = [newImage retain];

        [self setNeedsDisplay:YES];
    }
}

- (NSColor *)backgroundColor {
    return backgroundColor;
}

- (void)setBackgroundColor:(NSColor *)newColor {
    if (newColor != nil && backgroundColor != newColor) {
        [backgroundColor release];
        backgroundColor = [newColor retain];

        [self setNeedsDisplay:YES];
    }
}

#pragma mark -
#pragma mark *** Archiving ***

// A view must know how to archive and unarchive itself in order to be placed in a custom Interface Builder palette.

- (void)encodeWithCoder:(NSCoder *)aCoder {
    [super encodeWithCoder:aCoder];
    if ([aCoder allowsKeyedCoding]) {
        [aCoder encodeObject:backgroundColor forKey:@"backgroundColor"];
    }
}

- (id)initWithCoder:(NSCoder *)aDecoder {
    self = [super initWithCoder:aDecoder];
    if (self) {
        if ([aDecoder allowsKeyedCoding]) {
            backgroundColor = [[aDecoder decodeObjectForKey:@"backgroundColor"] copy];
        }
    }
    return self;
}

@end

@implementation HybridImageView (Internals)

// Computes the appropriate origin for the composited image so that it will be centered in the view.
- (NSPoint)originForCenteredImage {
    if (image) {
        NSRect bounds = [self bounds];
        NSSize imageSize;
        if ([image isKindOfClass:[CIImage class]]) {
            // In general, it is usually necessary to allow for the possibility that the extent of a CIImage may be infinite.  In this example, however, we know we are dealing with a result image of finite size.
            CGRect extent = [image extent];
            imageSize = NSMakeSize(extent.size.width, extent.size.height);
        } else {
            imageSize = [image size];
        }
        return NSMakePoint(floor(0.5 * (bounds.size.width - imageSize.width)), floor(0.5 * (bounds.size.height - imageSize.height)));
    } else {
        return NSZeroPoint;
    }
}

// Draws the view's content using the Cocoa/Quartz API.
- (void)drawUsingQuartzInRect:(NSRect)rect {
    // If we're in OpenGL mode, switch back to regular Quartz drawing.
    if (openGLContext != nil) {
        // Suspend screen updates until the view has had the chance to redraw itself.
        [[self window] disableScreenUpdatesUntilFlush];
        [self clearGLContext];
    }

    // Fill with the background color.
    [[self backgroundColor] set];
    NSRectFill(rect);

    if ([image isKindOfClass:[NSBitmapImageRep class]]) {
        NSBitmapImageRep *bitmapImageRep = (NSBitmapImageRep *)image;

        // Create a temporary NSImage from the NSBitmapImageRep, and use it to composite the bitmap into the window.
        NSImage *nsImage = [[NSImage alloc] initWithSize:[bitmapImageRep size]];
        [nsImage addRepresentation:bitmapImageRep];
        NSSize imageSize = [nsImage size];
        [nsImage drawAtPoint:[self originForCenteredImage] fromRect:NSMakeRect(0, 0, imageSize.width, imageSize.height) operation:NSCompositeSourceOver fraction:1.0];
        [nsImage release];
    }
}

// Draws the view's content using OpenGL and Core Image.
- (void)drawUsingOpenGLInRect:(NSRect)viewRect {
    if ([image isKindOfClass:[CIImage class]]) {
        NSPoint imageOrigin = [self originForCenteredImage];

        // If we're in Cocoa/Quartz mode, switch to OpenGL drawing.
        if (openGLContext == nil) {
            // Suspend screen updates until the view has had the chance to redraw itself.
            [[self window] disableScreenUpdatesUntilFlush];
        }
        [[self openGLContext] makeCurrentContext];
        [self updateMatrices];

        // Fill with background color.
        float r, g, b, a;
        [[[self backgroundColor] colorUsingColorSpaceName:NSDeviceRGBColorSpace] getRed:&r green:&g blue:&b alpha:&a];
        glClearColor(r, g, b, a);
        glClear (GL_COLOR_BUFFER_BIT);

        // Composite the image using a CIContext.
        [[self CIContext] drawImage:image atPoint:*(CGPoint *)&imageOrigin fromRect:[image extent]];

        /* Flush the OpenGL command stream. If the view is double
         * buffered this should be replaced by [[self openGLContext]
         * flushBuffer]. */
        glFlush ();
    }
}

- (NSOpenGLPixelFormat *)pixelFormat {
    /* Making sure the context's pixel format doesn't have a recovery
     * renderer is important - otherwise CoreImage may not be able to
     * create deeper context's that share textures with this one. */

    static const NSOpenGLPixelFormatAttribute attr[] = {
        NSOpenGLPFAAccelerated,
        NSOpenGLPFANoRecovery,
        NSOpenGLPFAColorSize, 32,
        0
    };

    if (pixelFormat == nil) {
        pixelFormat = [[NSOpenGLPixelFormat alloc] initWithAttributes:(void *)&attr];
    }

    return pixelFormat;
}

- (NSOpenGLContext *)openGLContext {
    // create a context the first time through
    //
    if (openGLContext == nil) {

        openGLContext = [[NSOpenGLContext alloc] initWithFormat:[self pixelFormat] shareContext:nil];
        [openGLContext setView:self];
        [openGLContext makeCurrentContext];
        [self setupOpenGL];
    }

    return openGLContext;
}

- (void)setupOpenGL {
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
    glHint (GL_TRANSFORM_HINT_APPLE, GL_FASTEST);
}

- (void)updateMatrices {
    NSRect r = [self bounds];

    [[self openGLContext] update];

    /* Install an orthographic projection matrix (no perspective)
     * with the origin in the bottom left and one unit equal to one
     * device pixel. */

    glViewport (0, 0, r.size.width, r.size.height);

    glMatrixMode (GL_PROJECTION);
    glLoadIdentity ();
    glOrtho (0, r.size.width, 0, r.size.height, -1, 1);

    glMatrixMode (GL_MODELVIEW);
    glLoadIdentity ();
}

- (void)clearGLContext {
    if (openGLContext != nil) {
        if ([openGLContext view] == self) {
            [openGLContext clearDrawable];
        }
        [ciContext release];
        ciContext = nil;
        [openGLContext release];
        openGLContext = nil;
    }
}

- (CIContext *)CIContext {
    /* Allocate a CoreImage rendering context using the view's OpenGL
     * context as its destination if none already exists. */

    if (ciContext == nil) {
	ciContext = [[CIContext contextWithCGLContext: CGLGetCurrentContext() pixelFormat:[[self pixelFormat] CGLPixelFormatObj] options: nil] retain];
    }
    return ciContext;
}

@end
