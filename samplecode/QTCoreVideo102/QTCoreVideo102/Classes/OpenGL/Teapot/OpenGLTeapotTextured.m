//------------------------------------------------------------------------
//
//	File: OpenGLTeapotTextured.m
//
//  Abstract: Class that implements a method for generating a teapot
//            with enabled sphere map texture coordinate generation.
//
//  Disclaimer: IMPORTANT:  This Apple software is supplied to you by
//  Apple Inc. ("Apple") in consideration of your agreement to the
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
//  Neither the name, trademarks, service marks or logos of Apple Inc.
//  may be used to endorse or promote products derived from the Apple
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
//------------------------------------------------------------------------

//------------------------------------------------------------------------

#import "AlertPanelKit.h"
#import "OpenGLTeapotTextured.h"

//------------------------------------------------------------------------

//------------------------------------------------------------------------

struct OpenGLTeapotTexturedAttributes
{
	GLfloat   scaleX;
	GLfloat   scaleY;
};

typedef struct OpenGLTeapotTexturedAttributes  OpenGLTeapotTexturedAttributes;

//------------------------------------------------------------------------

//------------------------------------------------------------------------

@implementation OpenGLTeapotTextured

//------------------------------------------------------------------------

- (void) initTeapotScale:(const NSSize *)theScale
{
	if ( ( theScale->width <= 0.0f ) || ( theScale->height <= 0.0f ) )
	{
		teapotTextured->scaleX = 1920.0f;	// Default texture width for a HD movie
		teapotTextured->scaleY =  820.0f;	// Default texture height for a HD movie
	} // if
	else
	{
		teapotTextured->scaleX = theScale->width;
		teapotTextured->scaleY = theScale->height;
	} // else
} // initTeapotScale

//------------------------------------------------------------------------

- (void) initTeapotTexGenSphereMap
{
	glTexGeni( GL_S, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP );
	glTexGeni( GL_T, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP );
} // initTeapotTexGenSphereMap

//------------------------------------------------------------------------

- (id) initTeapotTexturedWithType:(const GLenum)theType 
							range:(const GLsizei)theRange
							grid:(const GLint)theGrid
							size:(const GLdouble)theSize
							scale:(const NSSize *)theScale
{
	self = [super initTeapotWithType:theType
								range:theRange
								grid:theGrid
								size:theSize];
	
	if ( self )
	{
		teapotMemObj = [[MemObject alloc] initMemoryWithType:kMemAlloc size:sizeof(OpenGLTeapotTexturedAttributes)];
		
		if ( teapotMemObj )
		{
			teapotTextured = (OpenGLTeapotTexturedAttributesRef)[teapotMemObj pointer];
			
			if ( [teapotMemObj isPointerValid] )
			{
				[self initTeapotScale:theScale];
				[self initTeapotTexGenSphereMap];
			} // if
			else
			{
				[[AlertPanelKit withTitle:@"OpenGL Teapot Textured" 
								  message:@"Failure Allocating Memory For Textured OpenGL Teapot Attributes"
									 exit:NO] displayAlertPanel];
			} // else
		}  // if
	} // if
	
	return  self;
} // initTeapotTexturedWithType

//------------------------------------------------------------------------

- (void) dealloc
{
	if ( teapotMemObj )
	{
		[teapotMemObj release];
		
		teapotMemObj = nil;
	} // if
	
	[super dealloc];
} // dealloc

//---------------------------------------------------------------------------
//
// Enable sphere map automatic texture coordinate generation.
//
//---------------------------------------------------------------------------

- (void) enableSphereMap
{
	glEnable( GL_TEXTURE_GEN_S );
	glEnable( GL_TEXTURE_GEN_T );
} // enableSphereMap

//---------------------------------------------------------------------------
//
// Disable shere map automatic texture coordinate generation.
//
//---------------------------------------------------------------------------

- (void) disableSphereMap
{
	glDisable( GL_TEXTURE_GEN_T );
	glDisable( GL_TEXTURE_GEN_S );
} // enableSphereMap

//------------------------------------------------------------------------

- (void) draw
{
	[self enableSphereMap];

		// Since we will be using GL_TEXTURE_RECTANGLE_EXT textures which 
		// uses pixel coordinates rather than normalized coordinates, we 
		// need to scale the texturing matrix

		glMatrixMode( GL_TEXTURE );

		// To rotate without skewing or translation, we must be in 0-centered 
		// normalized texture coordinates 

		glScalef( teapotTextured->scaleX, teapotTextured->scaleY, 1.0f );

		glMatrixMode( GL_MODELVIEW );

		// Draw the teapot

		[self callList];
	
	[self disableSphereMap];
} // draw

//------------------------------------------------------------------------

@end

//------------------------------------------------------------------------

//------------------------------------------------------------------------


