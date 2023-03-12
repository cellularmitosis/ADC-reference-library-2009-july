/*

File: VideoView.m of VideoViewer

Author: QuickTime engineering

Change History (most recent first): <2> 07/26/05 added call to sync with screen refresh
                                                 in createWorkingColorSpace to avoid tearing
                                    <1> 05/31/05 initial release

© Copyright 2005 Apple Computer, Inc. All rights reserved.

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

#import "VideoView.h"
#import <OpenGL/OpenGL.h>
#import <OpenGL/CGLMacro.h>
#import <QuartzCore/QuartzCore.h>

// macros to test fallback modes
#define VIDEO_VIEWER_DISABLE_VISUAL_CONTEXT 0
#define VIDEO_VIEWER_DISABLE_COREIMAGE 0

#pragma mark

// Child view of VideoView that renders with CoreImage or OpenGL using visual contexts
@interface VideoVCView : NSView
{
    // OpenGL-specific
    NSOpenGLPixelFormat* _glPixelFormat;         // Cocoa-based OpenGL pixel format and context
    NSOpenGLContext*     _glContext;
    CGLPixelFormatObj    _cglPixelFormat;        // Handy references to the underlying CGL objects
    CGLContextObj        _cglContext;
    GLsizei              _glWidth;               // Width and height, in pixels, of our OpenGL surface
    GLsizei              _glHeight;
    GLenum               _glTextureTarget;       // The currently-enabled OpenGL texture target
    GLfloat              _vertices[4][2];        // Geometry and texture coordinates for OpenGL:
    GLfloat              _texCoords[4][2];       // 4 corners of our video (counter-clockwise from origin)
    
    // QuickTime/CoreVideo-specific
    QTVisualContextRef   _textureContext;        // Visual context for OpenGL textures
    CGColorSpaceRef      _workingColorSpace;     // Working color space for both QuickTime and CoreImage
    CVOpenGLTextureRef   _texture;               // Most recent frame of video from visual context
    CVDisplayLinkRef     _displayLink;           // Display link for managing rendering thread

    // CoreImage-specific
    CIContext*           _ciContext;             // CoreImage rendering context
    CIImage*             _image;                 // Most recent frame of video rendered with CI
    CGRect               _imageExtent;           // Bounds of the original video frame (before any filtering)
    BOOL                 _coreImageAccelerated;  // YES, if CoreImage will be accelerated
}
- (QTVisualContextRef)visualContext;
- (void)reshapeOpenGL;
- (void)renderTexture;
- (void)globalFrameDidChange:(NSNotification*)notification;
- (void)displayProfileChanged:(NSNotification*)notification;
- (CVReturn)displayForTime:(const CVTimeStamp*)outputTime;
- (void)createWorkingColorSpace;
@end

// Child view of VideoView that renders with Quickdraw (used when visual contexts are not available)
@interface VideoQDView : NSQuickDrawView
{
}
- (CGrafPtr)quickDrawPort;
@end

#pragma mark

NSString* VideoViewCanDrawNotification = @"VideoViewCanDraw";
NSString* VideoViewMustDrawNotification = @"VideoViewMustDraw";

@implementation VideoView

- (id)initWithFrame:(NSRect)frame
{
    if ((self = [super initWithFrame:frame]) != nil)
    {
        NSRect bounds = [self bounds];
        NSView* view;
        
        view = _vcView = [[VideoVCView alloc] initWithFrame:bounds];
        
        if (view == nil)
            view = _qdView = [[VideoQDView alloc] initWithFrame:bounds];
        
        [view setAutoresizingMask:NSViewWidthSizable | NSViewHeightSizable];
        [self addSubview:view];
    }
    
    return self;
}

- (void)dealloc
{
    [_vcView release];
    [_qdView release];
    [super dealloc];
}

- (QTVisualContextRef)visualContext
{
    return [_vcView visualContext];
}

- (CGrafPtr)quickDrawPort
{
    return [_qdView quickDrawPort];
}

@end


#pragma mark

@implementation VideoVCView

static void VideoVCViewCMPrefsChangedCallBack(CFNotificationCenterRef center, void *observer, CFStringRef name, const void *object, CFDictionaryRef userInfo)
{
    [(VideoVCView*)observer displayProfileChanged:nil];
}

- (id)initWithFrame:(NSRect)frame
{
    if ((self = [super initWithFrame:frame]) != nil)
    {
        static NSOpenGLPixelFormatAttribute attributes[] = {NSOpenGLPFAAccelerated, NSOpenGLPFADoubleBuffer, 0};

        // Create OpenGL, CoreImage, CoreVideo and QuickTime rendering objects
        _glPixelFormat = [[NSOpenGLPixelFormat alloc] initWithAttributes:attributes];
        _glContext = [[NSOpenGLContext alloc] initWithFormat:_glPixelFormat shareContext:nil];
        _cglContext = (CGLContextObj)[_glContext CGLContextObj];
        _cglPixelFormat = (CGLPixelFormatObj)[_glPixelFormat CGLPixelFormatObj];
        CVDisplayLinkCreateWithActiveCGDisplays(&_displayLink);
        QTOpenGLTextureContextCreate(kCFAllocatorDefault, _cglContext, _cglPixelFormat, NULL, &_textureContext);

        if (_textureContext != NULL && !VIDEO_VIEWER_DISABLE_VISUAL_CONTEXT)
        {
            SInt32 swapInterval = 1;
            
            // QuickTime and CoreImage should use the same working color space
            [self createWorkingColorSpace];
            // Sync with screen refresh to avoid tearing
            [_glContext setValues:&swapInterval forParameter:NSOpenGLCPSwapInterval];
            QTVisualContextSetAttribute(_textureContext, kQTVisualContextWorkingColorSpaceKey, _workingColorSpace);

            // Listen for changes to the displays' ColorSync profiles
            CFNotificationCenterAddObserver(CFNotificationCenterGetDistributedCenter(), self,
                &VideoVCViewCMPrefsChangedCallBack, kCMDeviceProfilesNotification, NULL, 
                CFNotificationSuspensionBehaviorDeliverImmediately);
            [self displayProfileChanged:nil];           
        }
        else
        {
            [self release];
            self = nil;
        }
    }
    
    return self;
}

- (void)dealloc
{
    CFNotificationCenterRemoveObserver(CFNotificationCenterGetDistributedCenter(), self, NULL, NULL);
    [[NSNotificationCenter defaultCenter] removeObserver:self];

    CVDisplayLinkRelease(_displayLink);
    [_image release];
    [_ciContext release];
    CVBufferRelease(_texture);
    QTVisualContextRelease(_textureContext);
    [_glContext release];
    [_glPixelFormat release];
    CGColorSpaceRelease(_workingColorSpace);

    [super dealloc];
}

- (BOOL)isOpaque
{
    return YES;
}

- (BOOL)wantsDefaultClipping
{
    return NO;
}

- (BOOL)displaysWhenScreenProfileChanges
{
    return YES;
}

- (void)viewWillMoveToWindow:(NSWindow*)newWindow
{
    NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
    [center removeObserver:self name:NSWindowDidChangeScreenProfileNotification object:nil];
    [center addObserver:self selector:@selector(displayProfileChanged:) name:NSWindowDidChangeScreenProfileNotification object:newWindow];
    
    // When using OpenGL, we should disable the window's "one-shot" feature
    [newWindow setOneShot:NO];
}

- (void)displayProfileChanged:(NSNotification*)notification
{
    CGDirectDisplayID did = (CGDirectDisplayID)[[[[[self window] screen] deviceDescription] objectForKey:@"NSScreenNumber"] pointerValue];
    CMProfileRef displayProfile = NULL;
    CGColorSpaceRef displayColorSpace = NULL;

    // Ask ColorSync for our current display's profile
    CMGetProfileByAVID((CMDisplayIDType)did, &displayProfile);
    displayColorSpace = CGColorSpaceCreateWithPlatformColorSpace(displayProfile);
    CMCloseProfile(displayProfile);
    
    CGLLockContext(_cglContext);
    {
        // Create a new CIContext using the new output color space
        NSDictionary* options = [NSDictionary dictionaryWithObjectsAndKeys:
            (id)_workingColorSpace, kCIContextWorkingColorSpace,
            (id)displayColorSpace,  kCIContextOutputColorSpace,
            nil];
            
        [_ciContext release];
        _ciContext = [[CIContext contextWithCGLContext:_cglContext pixelFormat:_cglPixelFormat options:options] retain];
    }
    CGLUnlockContext(_cglContext);
    
    CGColorSpaceRelease(displayColorSpace);
}

static CVReturn VideoVCViewDisplayLinkCallback(CVDisplayLinkRef displayLink, const CVTimeStamp* inNow, 
    const CVTimeStamp* inOutputTime, CVOptionFlags flagsIn, CVOptionFlags* flagsOut, void* displayLinkContext)
{
    // This is effectively our entry point on the rendering thread, so make sure we have an autorelease pool
    NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];
    CVReturn result = [(VideoVCView*)displayLinkContext displayForTime:inOutputTime];
    [pool drain];
    return result;
}

- (void)lockFocus
{
    [super lockFocus];
    
    // If we're using OpenGL, make sure it is connected and that the display link is running
    if ([_glContext view] != self)
    {
        [_glContext setView:self];
        [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(globalFrameDidChange:) name:NSViewGlobalFrameDidChangeNotification object:self];
        CVDisplayLinkSetOutputCallback(_displayLink, &VideoVCViewDisplayLinkCallback, self);
        CVDisplayLinkSetCurrentCGDisplayFromOpenGLContext(_displayLink, _cglContext, _cglPixelFormat);
        CVDisplayLinkStart(_displayLink);
    }
}

- (void)drawRect:(NSRect)rect
{
    CGLLockContext(_cglContext);
    {
        // Try for a texture, just in case the display link has not yet started
        if (_texture == NULL)
            QTVisualContextCopyImageForTime(_textureContext, kCFAllocatorDefault, NULL, &_texture);

        [self renderTexture];
    }
    CGLUnlockContext(_cglContext);
}

- (void)renewGState
{
    // Synchronize with window server to avoid flashes or corrupt drawing
    [[self window] disableScreenUpdatesUntilFlush];
    [self globalFrameDidChange:nil];
    [super renewGState];
}

- (QTVisualContextRef)visualContext
{
    return _textureContext;
}

- (void)reshapeOpenGL
{
    CGLContextObj cgl_ctx = _cglContext;
    float uiScale = [[self window] userSpaceScaleFactor];
    GLint viewportLeft, viewportBottom;
    NSRect glRect;

    // Calculate the pixel-aligned rectangle in which OpenGL will render
    glRect.size = NSIntegralRect([self convertRect:[self bounds] toView:nil]).size;
    glRect.origin = [self convertRect:NSIntegralRect([self convertRect:[self visibleRect] toView:nil]) fromView:nil].origin;
    viewportLeft   = glRect.origin.x > 0 ? -glRect.origin.x * uiScale : 0;
    viewportBottom = glRect.origin.y > 0 ? -glRect.origin.y * uiScale : 0;
    _glWidth = glRect.size.width;
    _glHeight = glRect.size.height;
    glViewport(viewportLeft, viewportBottom, _glWidth, _glHeight);
    
    // Set up our coordinate system with lower-left=(0,0) and upper-right=(_glWidth,_glHeight)
    // since CoreImage works best when coordinates are 1:1 with pixels
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, _glWidth, 0, _glHeight, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    
    // Configure OpenGL to get vertex and texture coordinates from our two arrays
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glVertexPointer(2, GL_FLOAT, 0, _vertices);
    glTexCoordPointer(2, GL_FLOAT, 0, _texCoords);

    // Specify video rectangle vertices counter-clockwise from (0,0)
    _vertices[1][0] = _vertices[2][0] = _glWidth;
    _vertices[2][1] = _vertices[3][1] = _glHeight;
}

- (void)renderTexture
{   
    CGLContextObj cgl_ctx = _cglContext;

    [self reshapeOpenGL];

    if (_texture != NULL)
    {
        // If the hardware supports CoreImage, use it, otherwise fall back to raw OpenGL
        if (_coreImageAccelerated && !VIDEO_VIEWER_DISABLE_COREIMAGE)
        {
            // Create a CIImage if necessary and keep track of its original size since the output from
            // some Image Units may have infinite extent
            if (_image == NULL)
            {
                _image = [[CIImage alloc] initWithCVImageBuffer:_texture];
                _imageExtent = [_image extent];
            }
            
            [_ciContext drawImage: _image
                           inRect: CGRectMake(0, 0, _glWidth, _glHeight)
                         fromRect: _imageExtent];
        }
        else
        {
            GLenum textureTarget = CVOpenGLTextureGetTarget(_texture);

            // Make sure the correct texture target is enabled
            if (textureTarget != _glTextureTarget)
            {
                glDisable(_glTextureTarget);
                _glTextureTarget = textureTarget;
                glEnable(_glTextureTarget);
            }
            
            // Get the current texture's coordinates, bind the texture, and draw our rectangle
            CVOpenGLTextureGetCleanTexCoords(_texture, _texCoords[0], _texCoords[1], _texCoords[2], _texCoords[3]);
            glBindTexture(_glTextureTarget, CVOpenGLTextureGetName(_texture));
            glDrawArrays(GL_QUADS, 0, 4);
        }
    }
    else
    {
        // There is no video at this time, so just clear the surface
        glClear(GL_COLOR_BUFFER_BIT);
    }
    
    // Flush drawing to the screen and allow the visual context time to perform housekeeping
    // while we still have the OpenGL lock held.
    [_glContext flushBuffer];
    QTVisualContextTask(_textureContext);
}

- (void)globalFrameDidChange:(NSNotification*)notification
{
    CGLLockContext(_cglContext);
    {
        CGLContextObj cgl_ctx = _cglContext;
        const char* glExtensions;
        
        [_glContext update];
        
        // CoreImage might be too slow if the current renderer doesn't support GL_ARB_fragment_program
        glExtensions = (const char*)glGetString(GL_EXTENSIONS);
        _coreImageAccelerated = (strstr(glExtensions, "GL_ARB_fragment_program") != NULL);
    }
    CGLUnlockContext(_cglContext);

    // Make sure we are synchronized with the correct display
    CVDisplayLinkSetCurrentCGDisplayFromOpenGLContext(_displayLink, _cglContext, _cglPixelFormat);
}

- (CVReturn)displayForTime:(const CVTimeStamp*)outputTime
{
    // Ask the visual context if a new image is available for the specified time
    if (QTVisualContextIsNewImageAvailable(_textureContext, outputTime))
    {
        CGLLockContext(_cglContext);
        {
            // Now that we know a new image is ready, we can release the previous one
            [_image release];           _image = nil;
            CVBufferRelease(_texture);  _texture = NULL;
            
            // Get the new image and render it
            QTVisualContextCopyImageForTime(_textureContext, kCFAllocatorDefault, outputTime, &_texture);
            [self renderTexture];
        }
        CGLUnlockContext(_cglContext);
    }

    return kCVReturnSuccess;
}

- (void)createWorkingColorSpace
{
    // An RGB color space with linear gamma curves will make a good working space
    CMProfileRef workingProfile = NULL;
    NSNumber* one = [NSNumber numberWithFloat:1.0f];
    NSDictionary* workingProfileSpec = [NSDictionary dictionaryWithObjectsAndKeys:
        @"displayRGB",                 @"profileType",
        one,                           @"gammaR",
        one,                           @"gammaG",
        one,                           @"gammaB",
        @"HDTV",                       @"phosphorSet",
        [NSNumber numberWithInt:6500], @"whiteTemp",
        nil];
        
    CMNewProfile(&workingProfile, NULL);
    CMMakeProfile(workingProfile, (CFDictionaryRef)workingProfileSpec);
    _workingColorSpace = CGColorSpaceCreateWithPlatformColorSpace(workingProfile);
    CMCloseProfile(workingProfile);
}

@end


#pragma mark

@implementation VideoQDView

// Attempt to return our Quickdraw port.
- (CGrafPtr)quickDrawPort
{
    CGrafPtr port = NULL;
    if ([self lockFocusIfCanDraw])
    {
        port = (CGrafPtr)[self qdPort];
        [self unlockFocus];
    }
    return port;
}

- (void)lockFocus
{
    [super lockFocus];
    [[NSNotificationCenter defaultCenter] postNotificationName:VideoViewCanDrawNotification object:[self superview]];
}

- (void)drawRect:(NSRect)rect
{
    [[NSNotificationCenter defaultCenter] postNotificationName:VideoViewMustDrawNotification object:[self superview]];
}

@end
