/*
 
 File: MyOpenGLView.m
 
 Abstract:	MyOpenGLView.m shows how to integrate Core Image and OpenGL 
			to be able to take advantage of the built it Core Image's
			filters and use the result as an OpenGL texture. 

 Version: 1.0
 
 Disclaimer: IMPORTANT:  This Apple software is supplied to you by 
 Apple Inc. ("Apple") in consideration of your agreement to the
 following terms, and your use, installation, modification or
 redistribution of this Apple software constitutes acceptance of these
 terms.  If you do not agree with these terms, please do not use,
 install, modify or redistribute this Apple software.
 
 In consideration of your agreement to abide by the following terms, and
 subject to these terms, Apple grants you a personal, non-exclusive
 license, under Apple's copyrights in this original Apple software (the
 "Apple Software"), to use, reproduce, modify and redistribute the Apple
 Software, with or without modifications, in source and/or binary forms;
 provided that if you redistribute the Apple Software in its entirety and
 without modifications, you must retain this notice and the following
 text and disclaimers in all such redistributions of the Apple Software. 
 Neither the name, trademarks, service marks or logos of Apple Inc. 
 may be used to endorse or promote products derived from the Apple
 Software without specific prior written permission from Apple.  Except
 as expressly stated in this notice, no other rights or licenses, express
 or implied, are granted by Apple herein, including but not limited to
 any patent rights that may be infringed by your derivative works or by
 other works in which the Apple Software may be incorporated.
 
 The Apple Software is provided by Apple on an "AS IS" basis.  APPLE
 MAKES NO WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION
 THE IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS
 FOR A PARTICULAR PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND
 OPERATION ALONE OR IN COMBINATION WITH YOUR PRODUCTS.
 
 IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL
 OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 INTERRUPTION) ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION,
 MODIFICATION AND/OR DISTRIBUTION OF THE APPLE SOFTWARE, HOWEVER CAUSED
 AND WHETHER UNDER THEORY OF CONTRACT, TORT (INCLUDING NEGLIGENCE),
 STRICT LIABILITY OR OTHERWISE, EVEN IF APPLE HAS BEEN ADVISED OF THE
 POSSIBILITY OF SUCH DAMAGE.
 
 Copyright (C) 2008 Apple Inc. All Rights Reserved.
 
 */

#import "MyOpenGLView.h"

@implementation MyOpenGLView

- (id)initWithFrame:(NSRect)frameRect
{	

	// Nice antialised, hardware accelerated w/ no recovery by the software renderer.
	NSOpenGLPixelFormatAttribute   attribsAntialised[] =
	{
		NSOpenGLPFAAccelerated,
		NSOpenGLPFANoRecovery,
		NSOpenGLPFADoubleBuffer,
		NSOpenGLPFAColorSize, 24,
		NSOpenGLPFAAlphaSize,  8,
		// NOTE: for this particular case we don't need a depth buffer when drawing to the FBO, 
		// if you do need it, add the appropriate depth size,
		// but we don't want to waste VRAM here.
#if 0
		NSOpenGLPFADepthSize, 16,
#endif
		NSOpenGLPFAMultisample,
		NSOpenGLPFASampleBuffers, 1,
		NSOpenGLPFASamples, 4,
		nil
	};
	
	// A little less requirements if the above fails.
	NSOpenGLPixelFormatAttribute   attribsBasic[] =
	{
		NSOpenGLPFAAccelerated,
		NSOpenGLPFADoubleBuffer,
		NSOpenGLPFAColorSize, 24,
		NSOpenGLPFAAlphaSize,  8,
		// NOTE: for this particular case we don't need a depth buffer when drawing to the FBO, 
		// if you do need it, add the appropriate depth size,
		// but we don't want to waste VRAM here.
#if 0		
		NSOpenGLPFADepthSize, 16,
#endif
		nil
	};
	
	pixelFormat = [[[NSOpenGLPixelFormat alloc] initWithAttributes:attribsAntialised] autorelease];
	
	if (nil == pixelFormat) {
		NSLog(@"Couldn't find an FSAA pixel format, trying something more basic");
		pixelFormat = [[[NSOpenGLPixelFormat alloc] initWithAttributes:attribsBasic] autorelease];
	}
	
	self = [super initWithFrame:frameRect pixelFormat:pixelFormat];
	
	if (self)
	{
		NSOpenGLContext *ctx;
        ctx = [self openGLContext];
		
		// Turn on VBL syncing for swaps
		long VBL = 1;
		[ctx setValues:&VBL forParameter:NSOpenGLCPSwapInterval];
	}
	
	return self;
}


// Load the Image and setup the CI filter
- (void)setupImageWithCIFilter
{
	// Load the image
	
	myCIImage    =	[CIImage imageWithContentsOfURL: [NSURL fileURLWithPath:
		[[NSBundle mainBundle] pathForResource: @"Flowers" ofType: @"jpg"]]];
	
	[myCIImage retain];
	
	// Get the size of the image we are going to need throughout
	imageRect = [myCIImage extent];
	
	// Get the aspect ratio for possible scaling (e.g. texture coordinates)
	imageAspectRatio = imageRect.size.width / imageRect.size.height;
	
	// Create the CIFilters
	hueFilter   = [CIFilter filterWithName: @"CIHueAdjust"];
	[hueFilter setDefaults];
	[hueFilter setValue:myCIImage forKey:@"inputImage"];
	[hueFilter retain];
	
	gammaFilter = [CIFilter filterWithName: @"CIGammaAdjust"];
	[gammaFilter setDefaults];
	[gammaFilter setValue:[hueFilter valueForKey:@"outputImage"] forKey:@"inputImage"];
	[gammaFilter retain];

	gloomFilter = [CIFilter filterWithName: @"CIGloom"];
	[gloomFilter setDefaults];
	[gloomFilter setValue:[gammaFilter valueForKey:@"outputImage"] forKey:@"inputImage"];
	[gloomFilter retain];
	
}	

// Create CIContext based on OpenGL context and pixel format
- (BOOL)createCIContext
{
	
	// Create CIContext from the OpenGL context.
	myCIcontext = [CIContext contextWithCGLContext:[[self openGLContext] CGLContextObj] 
									   pixelFormat:[pixelFormat CGLPixelFormatObj]
										   options: nil];
	if (!myCIcontext)
	{ 
		NSLog(@"CIContext creation failed");
		return NO;
	}
	
	[myCIcontext retain];
	
	// Created succesfully
	return YES;
}

// Create or update the hardware accelerated offscreen area
// Framebuffer object aka. FBO
- (void)setFBO
{	
	
	// If not previously setup
	// generate IDs for FBO and its associated texture
	if (!FBOid)
	{
		// Make sure the framebuffer extenstion is supported
		const GLubyte* strExt;
		GLboolean isFBO;
		// Get the extenstion name string.
		// It is a space-delimited list of the OpenGL extenstions 
		// that are supported by the current renderer
		strExt = glGetString(GL_EXTENSIONS);
		isFBO = gluCheckExtension((const GLubyte*)"GL_EXT_framebuffer_object", strExt);
		if (!isFBO)
		{
			NSLog(@"Your system does not support framebuffer extension");
		}
		
		// create FBO object
		glGenFramebuffersEXT(1, &FBOid);
		// the texture
		glGenTextures(1, &FBOTextureId);
	}
	
	// Bind to FBO
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, FBOid);
	
	// Sanity check against maximum OpenGL texture size
	// If bigger adjust to maximum possible size
	// while maintain the aspect ratio
	GLint maxTexSize; 
	glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxTexSize);
	if (imageRect.size.width > maxTexSize || imageRect.size.height > maxTexSize) 
	{
		if (imageAspectRatio > 1)
		{
			imageRect.size.width = maxTexSize; 
			imageRect.size.height = maxTexSize / imageAspectRatio;
		}
		else
		{
			imageRect.size.width = maxTexSize * imageAspectRatio ;
			imageRect.size.height = maxTexSize; 
		}
	}
	
	// Initialize FBO Texture
	glBindTexture(GL_TEXTURE_RECTANGLE_ARB, FBOTextureId);
	// Using GL_LINEAR because we want a linear sampling for this particular case
	// if your intention is to simply get the bitmap data out of Core Image
	// you might want to use a 1:1 rendering and GL_NEAREST
	glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	
	// the GPUs like the GL_BGRA / GL_UNSIGNED_INT_8_8_8_8_REV combination
	// others are also valid, but might incur a costly software translation.
	glTexImage2D(GL_TEXTURE_RECTANGLE_ARB, 0, GL_RGBA, imageRect.size.width, imageRect.size.height, 0, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, NULL);
	
	// and attach texture to the FBO as its color destination
	glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_RECTANGLE_ARB, FBOTextureId, 0);

	// NOTE: for this particular case we don't need a depth buffer when drawing to the FBO, 
	// if you do need it, make sure you add the depth size in the pixel format, and
	// you might want to do something along the lines of:
#if 0
	// Initialize Depth Render Buffer
	GLuint depth_rb;
	glGenRenderbuffersEXT(1, &depth_rb);
	glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, depth_rb);
	glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, GL_DEPTH_COMPONENT, imageRect.size.width, imageRect.size.height);
	// and attach it to the FBO
	glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_RENDERBUFFER_EXT, depth_rb);
#endif	
	
	// Make sure the FBO was created succesfully.
	if (GL_FRAMEBUFFER_COMPLETE_EXT != glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT))
	{
		NSLog(@"Framebuffer Object creation or update failed!");
	}
		
	// unbind FBO 
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
}

- (void)prepareOpenGL
{

	// Clear to black nothing fancy.
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	
	// Setup blending function 
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	
	// Enable texturing 
	glEnable(GL_TEXTURE_RECTANGLE_ARB);
	
	// load the image and setup the Core Image filters
	[self setupImageWithCIFilter];
	
	// create an OpenGL backed CIContext
	[self createCIContext];
	
	// Create FBO and attached texture
	// FBO size depends on CI image extent
	// initialized in the methods called above.
	[self setFBO];	
	
	// Update values and trigger initial rendering of CI to FBO
	[self sliderChanged:self];

}

- (void)dealloc
{
	// Delete the texture
	glDeleteTextures(1, &FBOTextureId);
	// and the FBO
	glDeleteFramebuffersEXT(1, &FBOid);
	
	[super dealloc];
}

// This method actually renders with Core Image to the 
// OpenGL managed, hardware accelerated offscreen area
- (void)renderCoreImageToFBO
{
	// Update values for filters
	[hueFilter setValue: [NSNumber numberWithFloat: hueAngle] forKey: @"inputAngle"];
	[gammaFilter setValue: [NSNumber numberWithFloat: gammaPower] forKey: @"inputPower"];
	[gloomFilter setValue: [NSNumber numberWithFloat: gloomIntensity] forKey: @"inputIntensity"];
	[gloomFilter setValue: [NSNumber numberWithFloat: gloomRadius] forKey: @"inputRadius"];

	// Update images
	[hueFilter setValue:myCIImage forKey:@"inputImage"];
	[gammaFilter setValue:[hueFilter valueForKey:@"outputImage"] forKey:@"inputImage"];
	[gloomFilter setValue:[gammaFilter valueForKey:@"outputImage"] forKey:@"inputImage"];
	
	// Bind FBO 
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, FBOid);
	
	// set GL state 
	GLint width = (GLint)ceil(imageRect.size.width);
	GLint height = (GLint)ceil(imageRect.size.height);
	
	// the next few calls simply map an orthographic
	// projection or screen aligned 2D area for Core Image to
	// draw into
	{
		glViewport(0, 0, width, height);
		
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		glOrtho(0, width, 0, height, -1, 1);
		
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		
		glClear(GL_COLOR_BUFFER_BIT /*| GL_DEPTH_BUFFER_BIT no depth buffer */);
	}
	
	// Render CI 
	[myCIcontext drawImage: [gloomFilter valueForKey:@"outputImage"]
				   atPoint: CGPointZero  fromRect: imageRect];
	
	// Bind to default framebuffer (unbind FBO)
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
	
	[self setNeedsDisplay:YES];
}


// Render a fake reflection
// with 3 quads, one for the floor
// one for the reflection and one
// for the object to be reflected.
- (void) renderScene
{
	NSRect bounds = [self bounds];
		
	// Setup OpenGL with a perspective projection
	// and back to 3D stuff with the depth buffer
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		
		glViewport(0, 0, bounds.size.width, bounds.size.height);
		
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		gluPerspective(60.0, bounds.size.width / bounds.size.height, .1, 100.0);
		
		glMatrixMode(GL_TEXTURE);
		glLoadIdentity();
		// the GL_TEXTURE_RECTANGLE_ARB doesn't use normalized coordinates
		// scale the texture matrix to "increase" the texture coordinates
		// back to the image size
		glScalef(imageRect.size.width,imageRect.size.height,1.0f);
		
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		
		glTranslatef(0.0f, 0.0f, -2.0f);
	}
	
	// Fake reflection below floor
	// Draw the image faintly upside down
	// Using the results from Core Image stored in the texture from the FBO
	glBindTexture(GL_TEXTURE_RECTANGLE_ARB, FBOTextureId);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	// Using GL_MODULATE to have the quad modulate the color/intensity
	// to achieve a going darker effect at the bottom.
	// Enable blending
	glEnable(GL_BLEND);
	// Draw a quad with the correct aspect ratio of the image
	glPushMatrix();
	{
		glScalef(imageAspectRatio,1.0f,1.0f);
		glBegin(GL_QUADS);
		{
			glColor4f(   0.8f, 0.8f,  0.8f, 0.8f );
			glTexCoord2f( 1.0f, 0.0f ); glVertex2f(  0.5f, 0.0f );
			glTexCoord2f( 0.0f, 0.0f ); glVertex2f( -0.5f, 0.0f );
			glColor4f(   0.0f, 0.0f,  0.0f, 0.0f );
			glTexCoord2f( 0.0f, 1.0f ); glVertex2f( -0.5, -1.0f );
			glTexCoord2f( 1.0f, 1.0f ); glVertex2f(  0.5, -1.0f );
		}
		glEnd();
	}
	glPopMatrix();
	
	// Floor
	// Simply draw a faint quad with light gray color
	// on one side and black on the other, rotate a little
	// to achieve the right kind of look
	glPushMatrix();
	{
		glRotatef(20.0f, 1.0f, 0.0f, 0.0f);
		// Don't use the texture, we want just faint fray
		glBindTexture(GL_TEXTURE_RECTANGLE_ARB,0);
		// Draw a quad
		glBegin(GL_QUADS);
		{
			glColor4f(   0.4f, 0.4f,  0.4f, 0.4f );
			glVertex4f( -1.5,  0.0f,  1.5f, 1.0f );
			glVertex4f(	 1.5f, 0.0f,  1.5f, 1.0f );
			glColor4f(   0.0f, 0.0f,  0.0f, 0.0f );
			glVertex4f(  1.5f, 0.0f, -1.5f, 1.0f );
			glVertex4f( -1.5f, 0.0f, -1.5f, 1.0f );
		}
		glEnd();
	}
	glPopMatrix();
	// Disable blending
	glDisable(GL_BLEND);

	// Object on top of floor
	// Draw the image right side up
	// again using the texture from the FBO
	glBindTexture(GL_TEXTURE_RECTANGLE_ARB,FBOTextureId);
	// Using GL_REPLACE because we want the image colors 
	// unaffected by the quad color.
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	// Draw a quad with the correct aspect ratio of the image
	glPushMatrix();
	{
		glScalef(imageAspectRatio,1.0f,1.0f);
		glBegin(GL_QUADS);
		{
			glTexCoord2f( 1.0f, 1.0f ); glVertex2f(  0.5f, 1.0f );
			glTexCoord2f( 0.0f, 1.0f ); glVertex2f( -0.5f, 1.0f );
			glTexCoord2f( 0.0f, 0.0f ); glVertex2f( -0.5f, 0.0f );
			glTexCoord2f( 1.0f, 0.0f ); glVertex2f(  0.5f, 0.0f );
		}
		glEnd();
	}
	glPopMatrix();
}

- (void)drawRect:(NSRect)theRect
{		
	[[self openGLContext] makeCurrentContext];

	// Render using the resulting texture
	[self renderScene];
	
	// Make sure OpenGL does it thing!
	[[self openGLContext] flushBuffer];
}

// Action to receive the values from the slider on the GUI
- (IBAction)sliderChanged: (id)sender
{
    hueAngle = [hueAngleSlider floatValue]*M_PI/180.0f+.001f;
	gammaPower = [gammaPowerSlider floatValue];
	gloomRadius = [gloomRadiusSlider floatValue];
	gloomIntensity = [gloomIntensitySlider floatValue];
	whitePointColor = [whitePointColorWell color];
	
	// Render Core Image to the FBO
	[self renderCoreImageToFBO];
}

- (IBAction)updateCIImage:(id)sender
{
	if (myCIImage) [myCIImage release]; 

	if (nil != [imageView image]) 
	{
		// Load new image
		myCIImage = [CIImage imageWithData:[[imageView image] TIFFRepresentation]];
	} else {
		// Reload default image
		myCIImage =	[CIImage imageWithContentsOfURL: [NSURL fileURLWithPath:[[NSBundle mainBundle] pathForResource: @"Flowers" ofType: @"jpg"]]];
	}
	
	[myCIImage retain];
	
	// Update geometry
	imageRect = [myCIImage extent];
	imageAspectRatio = imageRect.size.width / imageRect.size.height;
		
	// Update FBO for new size and check for correctness
	[self setFBO];	
	
	// Render Core Image to the FBO
	[self renderCoreImageToFBO];
}

@end
