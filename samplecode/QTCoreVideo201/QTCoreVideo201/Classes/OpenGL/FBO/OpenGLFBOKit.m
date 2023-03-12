//---------------------------------------------------------------------------
//
//	File: OpenGLFBOKit.m
//
//  Abstract: Utility class for managing FBOs
// 			 
//  Disclaimer: IMPORTANT:  This Apple software is supplied to you by Apple
//  Computer, Inc. ("Apple") in consideration of your agreement to the
//  following terms, and your use, installation, modification or
//  redistribution of this Apple software constitutes acceptance of these
//  terms.  If you do not agree with these terms, please do not use,
//  install, modify or redistribute this Apple software.
//  
//  In consideration of your agreement to abide by the following terms, and
//  subject to these terms, Apple grants you a personal, non-exclusive
//  license, under Apple's copyrights in this original Apple software (the
//  "Apple Software"), to use, reproduce, modify and redistribute the Apple
//  Software, with or without modifications, in source and/or binary forms;
//  provided that if you redistribute the Apple Software in its entirety and
//  without modifications, you must retain this notice and the following
//  text and disclaimers in all such redistributions of the Apple Software. 
//  Neither the name, trademarks, service marks or logos of Apple Computer,
//  Inc. may be used to endorse or promote products derived from the Apple
//  Software without specific prior written permission from Apple.  Except
//  as expressly stated in this notice, no other rights or licenses, express
//  or implied, are granted by Apple herein, including but not limited to
//  any patent rights that may be infringed by your derivative works or by
//  other works in which the Apple Software may be incorporated.
//  
//  The Apple Software is provided by Apple on an "AS IS" basis.  APPLE
//  MAKES NO WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION
//  THE IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS
//  FOR A PARTICULAR PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND
//  OPERATION ALONE OR IN COMBINATION WITH YOUR PRODUCTS.
//  
//  IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL
//  OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
//  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
//  INTERRUPTION) ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION,
//  MODIFICATION AND/OR DISTRIBUTION OF THE APPLE SOFTWARE, HOWEVER CAUSED
//  AND WHETHER UNDER THEORY OF CONTRACT, TORT (INCLUDING NEGLIGENCE),
//  STRICT LIABILITY OR OTHERWISE, EVEN IF APPLE HAS BEEN ADVISED OF THE
//  POSSIBILITY OF SUCH DAMAGE.
// 
//  Copyright (c) 2008 Apple Inc., All rights reserved.
//
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------

#import "AlertPanelKit.h"
#import "OpenGLFBOStatusKit.h"
#import "OpenGLFBOKit.h"

//---------------------------------------------------------------------------

//---------------------------------------------------------------------------

struct OpenGLImageBuffer
{
    GLubyte  *data;			// Pointer to the top left pixel of the buffer.
    GLuint	  height;		// The height (in pixels) of the buffer
    GLuint	  width;		// The width (in pixels) of the buffer
    GLuint    rowBytes;		// The number of bytes in a pixel row
};

typedef struct OpenGLImageBuffer  OpenGLImageBuffer;

//---------------------------------------------------------------------------

struct OpenGLTextureAttributes
{
	GLuint             name;				// texture id
	GLuint             samplesPerPixel;		// number of bytes per pixel
	GLuint             size;				// width * height * samples per pixel
	GLint              level;				// level-of-detail	number
	GLint              border;				// width of the border, either 0  or 1
	GLint              xoffset;				// x offset for texture copy
	GLint              yoffset;				// y offset for texture copy
	GLenum             target;				// e.g., texture 2D or texture rectangle
	GLenum             hint;				// type of texture storage
	GLenum             format;				// format
	GLenum             internalFormat;		// internal format
	GLenum             type;				// OpenGL specific type
	OpenGLImageBuffer  buffer;				// An image buffer
};

typedef struct OpenGLTextureAttributes  OpenGLTextureAttributes;

//---------------------------------------------------------------------------

struct OpenGLFramebufferAttributes
{
	GLuint   name;			// Framebuffer object id
	GLenum   target;		// Framebuffer target
	GLenum   attachment;	// Color attachment "n" extension
	BOOL     isValid;		// Framebuffer status
};

typedef struct OpenGLFramebufferAttributes  OpenGLFramebufferAttributes;

//---------------------------------------------------------------------------

struct OpenGLRenderbufferAttributes
{
	GLuint   name;				// Depth renderbuffer id
	GLenum   internalFormat;	// Renderbuffer internal format
	GLenum   target;			// Target type for renderbuffer
	GLenum   attachment;		// Type of frameBufferAttachment for renderbuffer
};

typedef struct OpenGLRenderbufferAttributes  OpenGLRenderbufferAttributes;

//---------------------------------------------------------------------------

struct OpenGLViewportAttributes
{
	GLint    x;			// lower left x coordinate
	GLint    y;			// lower left y coordinate
	GLsizei  width;		// viewport height
	GLsizei  height;	// viewport width
};

typedef struct OpenGLViewportAttributes  OpenGLViewportAttributes;

//---------------------------------------------------------------------------

struct OpenGLOrthoProjAttributes
{
	GLdouble left;		// left vertical clipping plane
	GLdouble right;		// right vertical clipping plane
	GLdouble bottom;	// bottom horizontal clipping plane
	GLdouble top;		// top horizontal clipping plane
	GLdouble zNear;		// nearer depth clipping plane
	GLdouble zFar;		// farther depth clipping plane
};

typedef struct OpenGLOrthoProjAttributes  OpenGLOrthoProjAttributes;

//---------------------------------------------------------------------------

struct OpenGLQuadAttributes
{
	GLuint   name;		// display list id
	GLfloat  width;		// quad width
	GLfloat  height;	// quad height
};

typedef struct OpenGLQuadAttributes  OpenGLQuadAttributes;

//---------------------------------------------------------------------------

struct OpenGLFBOAttributes
{
	OpenGLOrthoProjAttributes     orthographic;		// Attributes for orthographic projection
	OpenGLViewportAttributes      viewport;			// FBO viewport dimensions
	OpenGLTextureAttributes       texture;			// Texture bound to the framebuffer
	OpenGLQuadAttributes          quad;             // Quad attributes for FBO drawing
	OpenGLFramebufferAttributes   framebuffer;		// Framebuffer object attributes
	OpenGLRenderbufferAttributes  renderbuffer;		// Depth render buffer
};

typedef struct OpenGLFBOAttributes  OpenGLFBOAttributes;

//---------------------------------------------------------------------------

//---------------------------------------------------------------------------

#pragma mark -- OpenGL Texture Initializations --

//---------------------------------------------------------------------------
//
// The texture range hints parameters can be either of:
//
//		theAttributes->texture.hint = GL_STORAGE_SHARED_APPLE;  VRAM
//		theAttributes->texture.hint = GL_STORAGE_CACHED_APPLE;  AGP
//
// Some other initialization values may come from:
//
//		theAttributes->texture.target         = GL_TEXTURE_RECTANGLE_EXT;
//		theAttributes->texture.format         = GL_BGRA_EXT;
//		theAttributes->texture.internalFormat = GL_RGBA;
//
// For samples-per-pixel with OpenGL type GL_UNSIGNED_INT_8_8_8_8 or 
// GL_UNSIGNED_INT_8_8_8_8_REV:
//
//		theAttributes->texture.samplesPerPixel = 4;
//
//---------------------------------------------------------------------------

static void InitOpenGLTextureAttributes( const NSSize *theTextureSize, OpenGLFBOAttributesRef theAttributes )
{
	theAttributes->texture.name            = 0;
	theAttributes->texture.hint            = GL_STORAGE_PRIVATE_APPLE;
	theAttributes->texture.level           = 0;
	theAttributes->texture.border          = 0;
	theAttributes->texture.xoffset         = 0;
	theAttributes->texture.yoffset         = 0;
	theAttributes->texture.target          = GL_TEXTURE_RECTANGLE_ARB;
	theAttributes->texture.format          = GL_BGRA;
	theAttributes->texture.type            = GL_UNSIGNED_INT_8_8_8_8_REV;
	theAttributes->texture.internalFormat  = GL_RGBA8;
	theAttributes->texture.samplesPerPixel = 4;
	theAttributes->texture.buffer.width    = (GLuint)theTextureSize->width;
	theAttributes->texture.buffer.height   = (GLuint)theTextureSize->height;
	theAttributes->texture.buffer.rowBytes = theAttributes->texture.buffer.width  * theAttributes->texture.samplesPerPixel;
	theAttributes->texture.size            = theAttributes->texture.buffer.height * theAttributes->texture.buffer.rowBytes;
	theAttributes->texture.buffer.data     = NULL;
} // InitOpenGLTextureAttributes
 
//---------------------------------------------------------------------------

static void InitOpenGLQuadAttributes( OpenGLFBOAttributesRef theAttributes )
{
	theAttributes->quad.name   = 0;
	theAttributes->quad.width  = theAttributes->texture.buffer.width;
	theAttributes->quad.height = theAttributes->texture.buffer.height;
} // InitOpenGLQuadAttributes

//---------------------------------------------------------------------------

static void InitOpenGLFramebufferAttributes( OpenGLFBOAttributesRef theAttributes )
{
	theAttributes->framebuffer.name       = 0;
	theAttributes->framebuffer.target     = GL_FRAMEBUFFER_EXT;
	theAttributes->framebuffer.attachment = GL_COLOR_ATTACHMENT0_EXT;
	theAttributes->framebuffer.isValid    = NO;
} // InitOpenGLFramebufferAttributes

//---------------------------------------------------------------------------

static void InitOpenGLRenderbufferAttributes( OpenGLFBOAttributesRef theAttributes )
{
	theAttributes->renderbuffer.name           = 0;
	theAttributes->renderbuffer.target         = GL_RENDERBUFFER_EXT;
	theAttributes->renderbuffer.internalFormat = GL_DEPTH_COMPONENT24;
	theAttributes->renderbuffer.attachment     = GL_DEPTH_ATTACHMENT_EXT;
} // InitOpenGLRenderbufferAttributes

//---------------------------------------------------------------------------

static void InitOpenGLViewportAttributes( OpenGLFBOAttributesRef theAttributes )
{
	theAttributes->viewport.x      = 0;
	theAttributes->viewport.y      = 0;
	theAttributes->viewport.width  = theAttributes->texture.buffer.width;
	theAttributes->viewport.height = theAttributes->texture.buffer.height;
} // InitOpenGLViewportAttributes

//---------------------------------------------------------------------------

static void InitOpenGLOrthoProjAttributes( OpenGLFBOAttributesRef theAttributes )
{
	theAttributes->orthographic.left   =  0;
	theAttributes->orthographic.right  =  theAttributes->texture.buffer.width;
	theAttributes->orthographic.bottom =  0;
	theAttributes->orthographic.top    =  theAttributes->texture.buffer.height;
	theAttributes->orthographic.zNear  = -10.0;
	theAttributes->orthographic.zFar   =  10.0;
} // InitOpenGLOrthoProjAttributes

//---------------------------------------------------------------------------

//---------------------------------------------------------------------------

#pragma mark -- Check FBO Status --

//---------------------------------------------------------------------------

//---------------------------------------------------------------------------

#pragma mark

//---------------------------------------------------------------------------

//---------------------------------------------------------------------------

@implementation OpenGLFBOKit

//---------------------------------------------------------------------------

#pragma mark -- Initialize an OpenGL FBO --

//---------------------------------------------------------------------------

- (void) initFBOAttributes:(const NSSize *)theTextureSize
{
	InitOpenGLTextureAttributes( theTextureSize, attributes );
	InitOpenGLQuadAttributes( attributes );
	InitOpenGLViewportAttributes( attributes );
	InitOpenGLOrthoProjAttributes( attributes );
	InitOpenGLRenderbufferAttributes( attributes );
	InitOpenGLFramebufferAttributes( attributes );
} // initFBOAttributes

//---------------------------------------------------------------------------

 - (void) newQuad
 {
	attributes->quad.name = glGenLists( 1 );

	glNewList( attributes->quad.name, GL_COMPILE );

		glBegin(GL_QUADS);
		
			glTexCoord2f( 0.0f, 0.0f );
			glVertex2f( 0.0f, 0.0f );
			
			glTexCoord2f( 0.0f, attributes->texture.buffer.height );
			glVertex2f( 0.0f, attributes->quad.height );
			
			glTexCoord2f( attributes->texture.buffer.width, attributes->texture.buffer.height );
			glVertex2f( attributes->quad.width, attributes->quad.height );
			
			glTexCoord2f( attributes->texture.buffer.width, 0.0f );
			glVertex2f( attributes->quad.width, 0.0f );

		glEnd();

	glEndList();
} // newQuad
 
//---------------------------------------------------------------------------
//
// Initialize the fbo bound texture
//
//---------------------------------------------------------------------------	

- (void) newTexture
{
	glDisable(GL_TEXTURE_2D);
	glEnable(attributes->texture.target);
	
	glTextureRangeAPPLE(attributes->texture.target, 0, NULL);
	glTextureRangeAPPLE(GL_TEXTURE_2D, 0, NULL);
	glPixelStorei(GL_UNPACK_CLIENT_STORAGE_APPLE, GL_FALSE);

	glGenTextures(1, &attributes->texture.name);
	glBindTexture(attributes->texture.target, attributes->texture.name);

	glTexParameteri(attributes->texture.target, GL_TEXTURE_STORAGE_HINT_APPLE, GL_STORAGE_PRIVATE_APPLE);
	glTexParameteri(attributes->texture.target, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(attributes->texture.target, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(attributes->texture.target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(attributes->texture.target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	
	glTexImage2D(	attributes->texture.target, 
					attributes->texture.level,
					attributes->texture.internalFormat, 
					attributes->texture.buffer.width, 
					attributes->texture.buffer.height, 
					attributes->texture.border, 
					attributes->texture.format, 
					attributes->texture.type, 
					attributes->texture.buffer.data );
} // newTexture

//---------------------------------------------------------------------------
//
// Initialize depth render buffer
//
//---------------------------------------------------------------------------

- (void) newDepthRenderbuffer
{
	glGenRenderbuffersEXT(1, &attributes->renderbuffer.name);

	glBindRenderbufferEXT(attributes->renderbuffer.target, attributes->renderbuffer.name);

	glRenderbufferStorageEXT(	attributes->renderbuffer.target, 
								attributes->renderbuffer.internalFormat, 
								attributes->texture.buffer.width, 
								attributes->texture.buffer.height );
} // newDepthRenderbuffer

//---------------------------------------------------------------------------
//
// Bind to FBO before checking status
//
//---------------------------------------------------------------------------

- (void) bindToFBO
{	
	glGenFramebuffersEXT(1, &attributes->framebuffer.name);

	glBindFramebufferEXT(attributes->framebuffer.target, attributes->framebuffer.name);
	
	glFramebufferTexture2DEXT(	attributes->framebuffer.target, 
								attributes->framebuffer.attachment, 
								attributes->texture.target, 
								attributes->texture.name,
								attributes->texture.level );
	
	glFramebufferRenderbufferEXT(	attributes->framebuffer.target, 
									attributes->renderbuffer.attachment, 
									attributes->renderbuffer.target, 
									attributes->renderbuffer.name );
	
	attributes->framebuffer.isValid = [[OpenGLFBOStatusKit withFBOTarget:attributes->framebuffer.target 
																	exit:YES] framebufferComplete];

	glBindRenderbufferEXT(attributes->renderbuffer.target, 0);
	glBindFramebufferEXT(attributes->framebuffer.target, 0);
} // bindToFBO

//---------------------------------------------------------------------------

- (void) newFBO:(const NSSize *)theTextureSize
{
	[self initFBOAttributes:theTextureSize];
	[self newTexture];
	[self newQuad];
	[self newDepthRenderbuffer];
	[self bindToFBO];
} // newFBO

//---------------------------------------------------------------------------
//
// Initialize on startup
//
//---------------------------------------------------------------------------

- (id) initFBOWithSize:(NSSize)theTextureSize
{
	self = [super initMemoryWithType:kMemAlloc size:sizeof(OpenGLFBOAttributes)];
	
	if ( self )
	{
		attributes = (OpenGLFBOAttributesRef)[self pointer];
		
		if ( [self isPointerValid]  )
		{
			[self newFBO:&theTextureSize];
		} // if
		else
		{
			[[AlertPanelKit withTitle:@"OpenGL FBO Kit" 
							  message:@"Failure Allocating Memory For FBO Attributes"
								 exit:YES] displayAlertPanel];
		} // else
	} // if
	
	return  self;
} // initFBOWithSize

//---------------------------------------------------------------------------

#pragma mark -- Cleanup all the Resources --

//---------------------------------------------------------------------------

- (void) deleteOpenGLFBO
{
	if ( attributes->renderbuffer.name )
	{
		glDeleteRenderbuffersEXT( 1, &attributes->renderbuffer.name );
	} // if
	
	if ( attributes->framebuffer.name )
	{
		glDeleteFramebuffersEXT( 1, &attributes->framebuffer.name );
	} // if
	
	if ( attributes->quad.name )
	{
		glDeleteLists( attributes->quad.name, 1 );
	} // if
	
	if ( attributes->texture.name )
	{
		glDeleteTextures( 1, &attributes->texture.name );
	} // if
} // deleteOpenGLFBO

//---------------------------------------------------------------------------

- (void) dealloc 
{
	[self  deleteOpenGLFBO];
    [super dealloc];
} // dealloc

//---------------------------------------------------------------------------

#pragma mark -- Draw into FBO --

//---------------------------------------------------------------------------
//
// Reset the current viewport
//
//---------------------------------------------------------------------------

- (void) reset
{
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

	glViewport(	attributes->viewport.x, 
				attributes->viewport.y, 
				attributes->viewport.width, 
				attributes->viewport.height );
	
	// select the projection matrix
	
	glMatrixMode(GL_PROJECTION);
	
	 // reset it
	 
	glLoadIdentity();

	// Orthographic projection
	
	glOrtho(	attributes->orthographic.left, 
				attributes->orthographic.right, 
				attributes->orthographic.bottom, 
				attributes->orthographic.top, 
				attributes->orthographic.zNear, 
				attributes->orthographic.zFar );
	
	glMatrixMode(GL_TEXTURE);
	glLoadIdentity();
	
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
} // reset

//---------------------------------------------------------------------------

- (void) draw:(CVOpenGLTextureRef)theCVOpenGLTexture
{
	// get the texture target
	
	GLenum target = CVOpenGLTextureGetTarget( theCVOpenGLTexture );
	
	// get the texture target name
	
	GLint name = CVOpenGLTextureGetName( theCVOpenGLTexture );
	
	// bind to the CoreVideo texture

	glBindTexture( target, name );
	
	// draw the quad
	
	glCallList( attributes->quad.name );
} // draw

//---------------------------------------------------------------------------
//
// Render frame provided by Core Video to the Framebuffer Object (FBO)
//
//---------------------------------------------------------------------------

- (void) update:(CVOpenGLTextureRef)theCVOpenGLTexture
{
	// bind buffers and make attachments
	
	glBindFramebufferEXT( attributes->framebuffer.target, attributes->framebuffer.name );
	glBindRenderbufferEXT( attributes->renderbuffer.target, attributes->renderbuffer.name );

	// we have a video frame so draw to fbo
	
	// reset the current viewport
	
	[self reset];
	
	// draw to FBO

	[self draw:theCVOpenGLTexture];
	
	// unbind buffers
	
	glBindRenderbufferEXT( attributes->renderbuffer.target, 0 ); 
	glBindFramebufferEXT( attributes->framebuffer.target, 0 );
} // update

//---------------------------------------------------------------------------

- (void) bind
{
	glBindTexture( attributes->texture.target, attributes->texture.name );
} // bind

//---------------------------------------------------------------------------

@end

//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
