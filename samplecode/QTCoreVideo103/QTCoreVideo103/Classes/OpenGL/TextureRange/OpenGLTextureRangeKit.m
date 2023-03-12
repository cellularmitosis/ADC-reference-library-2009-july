//---------------------------------------------------------------------------
//
//	File: OpenGLTextureRangeKit.m
//
//  Abstract: Utility toolkit for handling texture range (VRAM or AGP)
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
#import "OpenGLTextureRangeKit.h"

//---------------------------------------------------------------------------

//---------------------------------------------------------------------------

struct OpenGLTextureRangeAttributes
{
	GLuint         name;				// texture id
	GLuint         samplesPerPixel;		// number of bytes per pixel
	GLuint         size;				// width * height * samples per pixel
	GLint          level;				// level-of-detail	number
	GLint          border;				// width of the border, either 0  or 1
	GLint          xoffset;				// x offset for texture copy
	GLint          yoffset;				// y offset for texture copy
	GLenum         target;				// e.g., texture 2D or texture rectangle
	GLenum         hint;				// type of texture storage
	GLenum         format;				// format
	GLenum         internalFormat;		// internal format
	GLenum         type;				// OpenGL specific type
	vImage_Buffer  buffer;				// An image buffer
	vImage_Buffer  source;				// Source image buffer
};

typedef struct OpenGLTextureRangeAttributes  OpenGLTextureRangeAttributes;

//---------------------------------------------------------------------------

//---------------------------------------------------------------------------

#pragma mark

//---------------------------------------------------------------------------

//---------------------------------------------------------------------------

@implementation OpenGLTextureRangeKit

//---------------------------------------------------------------------------

#pragma mark -- OpenGL Texture Initializations --

//---------------------------------------------------------------------------
//
// The texture range hints parameters can be either of:
//
//		texture->hint = GL_STORAGE_SHARED_APPLE;  AGP
//		texture->hint = GL_STORAGE_CACHED_APPLE;  VRAM
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

- (void) newTextureRangeAttributesWithSize:(NSSize)theTextureSize
						hint:(NSInteger)theTextureHint
{
	texture->name            = 0;
	texture->hint            = theTextureHint;
	texture->level           = 0;
	texture->border          = 0;
	texture->xoffset         = 0;
	texture->yoffset         = 0;
	texture->target          = GL_TEXTURE_RECTANGLE_ARB;
	texture->format          = GL_BGRA;
	texture->type            = GL_UNSIGNED_INT_8_8_8_8_REV;
	texture->internalFormat  = GL_RGBA8;
	texture->samplesPerPixel = 4;
	texture->buffer.width    = (vImagePixelCount)theTextureSize.width;
	texture->buffer.height   = (vImagePixelCount)theTextureSize.height;
	texture->buffer.rowBytes = texture->buffer.width  * texture->samplesPerPixel;
	texture->size            = texture->buffer.height * texture->buffer.rowBytes;
	texture->source.width    = texture->buffer.width;
	texture->source.height   = texture->buffer.height;
	texture->source.rowBytes = texture->buffer.rowBytes;
	texture->source.data     = NULL;

	textureMemObj = [[MemObject alloc] initMemoryWithType:kMemAlloc size:texture->size];
	
	if ( textureMemObj )
	{
		texture->buffer.data = [textureMemObj pointer];
		
		if ( ![textureMemObj isPointerValid] )
		{
			[[AlertPanelKit withTitle:@"OpenGL Texture Kit" 
							  message:@"Failure Allocating Memory For Texture Storage"
								 exit:YES] displayAlertPanel];
		} // if
	} // if
} // newTextureRangeAttributesWithSize
 
//---------------------------------------------------------------------------

- (void) newTextureRangeWithSize:(NSSize)theTextureSize
						hint:(NSInteger)theTextureHint
{
	[self newTextureRangeAttributesWithSize:theTextureSize hint:theTextureHint];

	glGenTextures(1, &texture->name);

	glEnable(texture->target);

	glTextureRangeAPPLE(	texture->target, 
							texture->size, 
							texture->buffer.data );
	
	glPixelStorei(GL_UNPACK_CLIENT_STORAGE_APPLE, GL_TRUE);
	
	glBindTexture(texture->target, texture->name);
	
	// Set the GL_TEXTURE_STORAGE_HINT_APPLE to GL_STORAGE_CACHED_APPLE or GL_STORAGE_SHARED_APPLE 
	// for requesting VRAM or AGP texturing respectively.	
	
	glTexParameteri(texture->target, 
					GL_TEXTURE_STORAGE_HINT_APPLE, 
					texture->hint);
	
	glTexParameteri(texture->target, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(texture->target, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(texture->target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(texture->target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	
	glTexImage2D(	texture->target, 
					texture->level, 
					texture->internalFormat, 
					texture->buffer.width,
					texture->buffer.height, 
					texture->border, 
					texture->format, 
					texture->type, 
					texture->buffer.data );

	glBindTexture(texture->target, 0);
} // newTextureRangeWithSize

//---------------------------------------------------------------------------
//
// Initialize
//
//---------------------------------------------------------------------------

- (id) initTextureRangeWithSize:(NSSize)theTextureSize
						hint:(NSInteger)theTextureHint
{
	self = [super initMemoryWithType:kMemAlloc size:sizeof(OpenGLTextureRangeAttributes)];
	
	if ( self )
	{
		texture = (OpenGLTextureRangeAttributesRef)[self pointer];
		
		if ( [self isPointerValid]  )
		{
			[self newTextureRangeWithSize:theTextureSize hint:theTextureHint];
		} // if
		else
		{
			[[AlertPanelKit withTitle:@"OpenGL Texture Kit" 
							  message:@"Failure Allocating Memory For Texture Attributes"
								 exit:YES] displayAlertPanel];
		}  // else
	} // if
	
	return  self;
} // initTextureRangeWithSize

//---------------------------------------------------------------------------

#pragma mark -- Delete Texture --

//---------------------------------------------------------------------------

- (void) dealloc 
{
	if ( texture->name )
	{
		glDeleteTextures( 1, &texture->name );

		glBindTexture(texture->target, 0);
	} // if
	
	if ( textureMemObj )
	{
		[textureMemObj release];
		
		textureMemObj = nil;
	} // if
	
    [super dealloc];
} // dealloc

//---------------------------------------------------------------------------

#pragma mark -- Texture Utilities --

//---------------------------------------------------------------------------
//
// Update a texture by first getting a pixel buffer base address from
// and video frame and then using  accelerate framework to apply vertical 
// reflect geometrical transformation to an image buffer.
//
//---------------------------------------------------------------------------

- (void) update:(CVPixelBufferRef)thePixelBuffer flags:(CVOptionFlags)theFlags
{
	// Get the base address of the CoreVideo pixel buffer address
	
	CVPixelBufferLockBaseAddress( thePixelBuffer, theFlags );

		texture->source.data = CVPixelBufferGetBaseAddress(thePixelBuffer);
		
	CVPixelBufferUnlockBaseAddress( thePixelBuffer, theFlags );
	
	// Apply the vertical reflect geometrical transformation
	
	vImageVerticalReflect_ARGB8888(&texture->source, &texture->buffer, kvImageNoFlags);
	
	// Update the texture
	
	glBindTexture(texture->target, texture->name);

	glTexSubImage2D(	texture->target, 
						texture->level, 
						texture->xoffset, 
						texture->yoffset, 
						texture->buffer.width,
						texture->buffer.height,
						texture->format, 
						texture->type, 
						texture->buffer.data );
} // update

//---------------------------------------------------------------------------

@end

//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
