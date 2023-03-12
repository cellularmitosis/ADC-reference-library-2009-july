//---------------------------------------------------------------------------
//
//	File: OpenGLPBOUnpackKit.m
//
//  Abstract: Utility toolkit for handling PBOs (unpack)
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

#import <Accelerate/Accelerate.h>

//---------------------------------------------------------------------------

#import "AlertPanelKit.h"
#import "MemObject.h"
#import "OpenGLPBOUnpackKit.h"

//---------------------------------------------------------------------------

//---------------------------------------------------------------------------

#define BUFFER_OFFSET(i) ((char *)NULL + (i))

//---------------------------------------------------------------------------

//---------------------------------------------------------------------------

struct OpenGLTextureAttributes
{
	GLuint         name;				// texture id
	GLuint         samplesPerPixel;		// number of bytes per pixel
	GLuint         size;				// width * height * samples per pixel
	GLint          level;				// level-of-detail	number
	GLint          hint;				// texture hint
	GLint          border;				// width of the border, either 0  or 1
	GLint          xoffset;				// x offset for texture copy
	GLint          yoffset;				// y offset for texture copy
	GLenum         target;				// e.g., texture 2D or texture rectangle
	GLenum         format;				// format
	GLenum         internalFormat;		// internal format
	GLenum         type;				// OpenGL specific type
	vImage_Buffer  buffer;				// An image buffer
};

typedef struct OpenGLTextureAttributes  OpenGLTextureAttributes;

//---------------------------------------------------------------------------

struct OpenGLPBOAttributes
{
	GLuint         name[2];		// PBO ids
	GLuint         index;		// PBO current index
	GLenum         target;		// e.g., pixel pack or unpack
	GLenum         usage;		// e.g., stream draw
	GLenum         access;		// e.g., read, write, or both
	GLuint         size;		// width * height * data size
	vImage_Buffer  buffer;		// An image buffer
};

typedef struct OpenGLPBOAttributes  OpenGLPBOAttributes;

//---------------------------------------------------------------------------

struct OpenGLPBOUnpackAttributes
{
	OpenGLTextureAttributes  texture;	// OpenGL texture attributes
	OpenGLPBOAttributes      pbo;		// OpenGL PBO attributes
	vImage_Buffer            source;	// Source image buffer
};

typedef struct OpenGLPBOUnpackAttributes   OpenGLPBOUnpackAttributes;

//---------------------------------------------------------------------------

//---------------------------------------------------------------------------

#pragma mark

//---------------------------------------------------------------------------

//---------------------------------------------------------------------------

@implementation OpenGLPBOUnpackKit

//---------------------------------------------------------------------------

#pragma mark -- OpenGL PBO Unpack Attribute Initializations --

//---------------------------------------------------------------------------
//
// Some other initialization values may come from:
//
//		texture->target         = GL_TEXTURE_RECTANGLE_EXT;
//		texture->format         = GL_BGRA_EXT;
//		texture->internalFormat = GL_RGBA;
//
// For samples-per-pixel with OpenGL type GL_UNSIGNED_INT_8_8_8_8 or 
// GL_UNSIGNED_INT_8_8_8_8_REV:
//
//		texture->samplesPerPixel = 4;
//
//---------------------------------------------------------------------------

- (void) newOpenGLTextureAttributes:(const NSSize *)theTextureSize
{
	attributes->texture.name            = 0;
	attributes->texture.level           = 0;
	attributes->texture.border          = 0;
	attributes->texture.xoffset         = 0;
	attributes->texture.yoffset         = 0;
	attributes->texture.hint            = GL_STORAGE_PRIVATE_APPLE;
	attributes->texture.target          = GL_TEXTURE_RECTANGLE_ARB;
	attributes->texture.format          = GL_BGRA;
	attributes->texture.type            = GL_UNSIGNED_INT_8_8_8_8_REV;
	attributes->texture.internalFormat  = GL_RGBA8;
	attributes->texture.samplesPerPixel = 4;
	attributes->texture.buffer.width    = (vImagePixelCount)theTextureSize->width;
	attributes->texture.buffer.height   = (vImagePixelCount)theTextureSize->height;
	attributes->texture.buffer.rowBytes = attributes->texture.buffer.width  * attributes->texture.samplesPerPixel;
	attributes->texture.size            = attributes->texture.buffer.height * attributes->texture.buffer.rowBytes;
	attributes->texture.buffer.data	    = NULL;
} // newOpenGLTextureAttributes
 
//---------------------------------------------------------------------------
//
//  Other PBO usage types that may be utilized,
//
//		theAttributes->pbo.usage = GL_STATIC_DRAW;
//
//---------------------------------------------------------------------------

- (void) newOpenGLPBOAttributes
{
	attributes->pbo.name[0]         = 0;
	attributes->pbo.name[1]         = 0;
	attributes->pbo.index           = 0;
	attributes->pbo.target          = GL_PIXEL_UNPACK_BUFFER_ARB;
	attributes->pbo.usage           = GL_STREAM_DRAW_ARB;
	attributes->pbo.access          = GL_WRITE_ONLY_ARB;
	attributes->pbo.size            = attributes->texture.size;
	attributes->pbo.buffer.width    = attributes->texture.buffer.width;
	attributes->pbo.buffer.height   = attributes->texture.buffer.height;
	attributes->pbo.buffer.rowBytes = attributes->texture.buffer.rowBytes;
	attributes->pbo.buffer.data     = NULL;
} // newOpenGLPBOAttributes

//---------------------------------------------------------------------------

- (void) newSourceImageAttributes
{
	attributes->source.width    = attributes->texture.buffer.width;
	attributes->source.height   = attributes->texture.buffer.height;
	attributes->source.rowBytes = attributes->texture.buffer.rowBytes;
	attributes->source.data     = NULL;
} // newSourceImageAttributes

//---------------------------------------------------------------------------

#pragma mark -- Designated initializer --

//---------------------------------------------------------------------------

- (void) newOpenGLAttributes:(const NSSize *)thePBOSize
{
	[self newOpenGLTextureAttributes:thePBOSize];
	[self newOpenGLPBOAttributes];
	[self newSourceImageAttributes];
} // newOpenGLAttributes

//---------------------------------------------------------------------------

- (void) newOpenGLTexture
{
	glGenTextures(1, &attributes->texture.name);
	
	glEnable(attributes->texture.target);
	
	glTextureRangeAPPLE(attributes->texture.target, 0, NULL);
	glTextureRangeAPPLE(GL_TEXTURE_2D, 0, NULL);
	
	glPixelStorei(GL_UNPACK_CLIENT_STORAGE_APPLE, GL_FALSE);
	
	glBindTexture(attributes->texture.target, attributes->texture.name);
	
	glTexParameteri(attributes->texture.target, GL_TEXTURE_STORAGE_HINT_APPLE, attributes->texture.hint);
	glTexParameteri(attributes->texture.target, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(attributes->texture.target, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(attributes->texture.target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(attributes->texture.target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	
	glTexImage2D(attributes->texture.target, 
				 attributes->texture.level, 
				 attributes->texture.internalFormat, 
				 attributes->texture.buffer.width,
				 attributes->texture.buffer.height, 
				 attributes->texture.border, 
				 attributes->texture.format,
				 attributes->texture.type, 
				 BUFFER_OFFSET(0));
	
	glDisable(attributes->texture.target);
} // newOpenGLTexture

//---------------------------------------------------------------------------

- (void) newOpenGLPBOUnpack
{
	glGenBuffersARB(2, attributes->pbo.name);
	
	glBindBufferARB(attributes->pbo.target, attributes->pbo.name[0]);
	
	glBufferDataARB(attributes->pbo.target, 
					attributes->pbo.size, 
					NULL, 
					attributes->pbo.usage);
	
	glBindBufferARB(attributes->pbo.target, attributes->pbo.name[1]);
	
	glBufferDataARB(attributes->pbo.target, 
					attributes->pbo.size, 
					NULL, 
					attributes->pbo.usage);
	
	glBindBufferARB(attributes->pbo.target, 0);
} // newOpenGLPBOUnpack

//---------------------------------------------------------------------------
//
// Initialize
//
//---------------------------------------------------------------------------

- (id) initPBOUnpackWithSize:(NSSize)thePBOSize
{
	self = [super initMemoryWithType:kMemAlloc size:sizeof(OpenGLPBOUnpackAttributes)];
	
	if ( self )
	{
		attributes = (OpenGLPBOUnpackAttributesRef)[self pointer];
		
		if ( [self isPointerValid] )
		{
			[self newOpenGLAttributes:&thePBOSize];
			[self newOpenGLTexture];
			[self newOpenGLPBOUnpack];
		} // if
		else
		{
			[[AlertPanelKit withTitle:@"OpenGL PBO Unpack Kit" 
							  message:@"Failure Allocating Memory For PBO Unpack Attributes"
								 exit:YES] displayAlertPanel];
		} // else
	} // if
	
	return  self;
} // initPBOUnpackWithSize

//---------------------------------------------------------------------------

#pragma mark -- Dealloc all the Resources --

//---------------------------------------------------------------------------

- (void) dealloc 
{
	if ( attributes->pbo.name )
	{
		glDeleteBuffers( 2, attributes->pbo.name );
	} // if
	
	if ( attributes->texture.name )
	{
		glDeleteTextures( 1, &attributes->texture.name );
	} // if

    [super dealloc];
} // dealloc

//---------------------------------------------------------------------------

#pragma mark -- Draw into the OpenGL View --

//---------------------------------------------------------------------------

- (void) pboWrite
{
    GLint nextIndex = 0;  // pbo index used for next frame

	// Increment the current index first, then increment the 
	// next index:
	//
	// (1)  "attributes->pbo.index" is used to copy pixels 
	//      from a  PBO to a texture.
	//
	// (2)  The local variable "nextIndex" is used to update 
	//      pixels in a PBO
	
	attributes->pbo.index = (attributes->pbo.index + 1) % 2;
	
	nextIndex = (attributes->pbo.index + 1) % 2;

	// Bind the texture and PBO
	
	glBindTexture(attributes->texture.target, attributes->texture.name);
	
	glBindBufferARB(attributes->pbo.target, 
					attributes->pbo.name[attributes->pbo.index] );

	// Use offset instead of pointer to copy pixels from PBO 
	// to texture.
	
	glTexSubImage2D(attributes->texture.target, 
					attributes->texture.level, 
					attributes->texture.xoffset, 
					attributes->texture.yoffset, 
					attributes->texture.buffer.width, 
					attributes->texture.buffer.height,
					attributes->texture.format, 
					attributes->texture.type, 
					BUFFER_OFFSET(0) );
	
	// Bind PBO to update pixel values
	
	glBindBufferARB(attributes->pbo.target, 
					attributes->pbo.name[nextIndex]);
	
	// If GPU is working with a buffer, glMapBufferARB results in a sync
	// issue and will stall the GPU pipeline until such time the current 
	// job is processed. To avoid stalling the pipeline, one should call 
	// the API glBufferDataARB with a NULL pointer before calling the API,
	// glMapBufferARB. If you do so then the previous data in a PBO will 
	// be discarded and glMapBufferARB returns a new allocated pointer 
	// immediately even thought the GPU is still processing the previous 
	// data.
	
	glBufferDataARB(attributes->pbo.target, 
					attributes->pbo.size, 
					NULL,
					attributes->pbo.usage );
	
	attributes->pbo.buffer.data 
	= (GLubyte *)glMapBufferARB(attributes->pbo.target, 
								attributes->pbo.access);	
	
	if( attributes->pbo.buffer.data != NULL )
	{
		// Copy the pixel buffer to pbo and vertically
		// reflect the pixels
		
		vImageVerticalReflect_ARGB8888(&attributes->source, 
									   &attributes->pbo.buffer, 
									   kvImageNoFlags);
		
		// Release pointer to the mapping buffer
		
		glUnmapBufferARB(attributes->pbo.target);
	} // if

	// At this stage, it is good idea to release a PBO 
	// (with ID 0) after use. Once bound to ID 0, all 
	// pixel operations default to normal behavior.
	
	glBindBufferARB(attributes->pbo.target, 0);
} // pboWrite

//---------------------------------------------------------------------------

- (void) update:(CVPixelBufferRef)thePixelBuffer 
		  flags:(CVOptionFlags)theFlags
{
	// Get the base address of the CoreVideo pixel buffer address
	
	CVPixelBufferLockBaseAddress( thePixelBuffer, theFlags );

		attributes->source.data = CVPixelBufferGetBaseAddress(thePixelBuffer);
		
	CVPixelBufferUnlockBaseAddress( thePixelBuffer, theFlags );
	
	// Update the pbo
	
	[self pboWrite];

	// Activate the pbo bound texture
	
	glEnable( attributes->texture.target );

	glBindTexture( attributes->texture.target, attributes->texture.name );
} // update

//---------------------------------------------------------------------------

@end

//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
