/*
 *
	Disclaimer:	IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc.
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
 *
 */

#import "MainOpenGLView.h"

#include <sys/time.h>
#include <unistd.h>

#include <QuickTime/ImageCompression.h> // for image loading and decompression
#include <QuickTime/QuickTimeComponents.h> // for file type support

#include <OpenGL/CGLCurrent.h>
#include <OpenGL/CGLContext.h>

@implementation MainOpenGLView

// ---------------------------------
// Returns a packed array of pixels of depth bufferDepth bits with size imageWidth x imageHeight.  
// Will fill origImageWidth & origImageHeight with dimensions of image in file
// Will return NULL on error
// Image is scaled and/or inset depending imageScale, maxTextureSize

- (void) loadBufferFromImageFile: (NSString*) path: (GLubyte *) imagePtr: (GLuint) imageWidth: (GLuint) imageHeight: (GLuint) imageDepth
{
	CFURLRef url;
 	FSRef fsRef;
	BOOL ok;
	GWorldPtr pGWorld = NULL;
	OSType pixelFormat;
	FSSpec fsspecImage;
	long rowStride; // length, in bytes, of a pixel row in the image
	GraphicsImportComponent giComp; // componenet for importing image
	Rect rectImage; // rectangle of source image
    ImageDescriptionHandle hImageDesc; // handle to image description used to get image depth
    MatrixRecord matrix;
	GDHandle origDevice; // save field for current graphics device
	CGrafPtr origPort; // save field for current graphics port
	OSStatus err = noErr; // err return value
	long origImageWidth, origImageHeight;
		
	url = CFURLCreateWithFileSystemPath(NULL, (CFStringRef) path, kCFURLPOSIXPathStyle, false);
	if(!url) return;
	
	ok = CFURLGetFSRef(url, &fsRef);
	CFRelease(url);
	if(!ok) return;
	
	err = FSGetCatalogInfo(&fsRef, kFSCatInfoNone, NULL, NULL, &fsspecImage, NULL);
	if(err) return;
	
	// get imorter for the image tyoe in file
	GetGraphicsImporterForFile (&fsspecImage, &giComp);
    if (err != noErr) return;

	// Create GWorld
    err = GraphicsImportGetNaturalBounds (giComp, &rectImage); //get the image bounds
    if (err != noErr) return;
	
    hImageDesc = (ImageDescriptionHandle) NewHandle (sizeof (ImageDescriptionHandle)); // create a handle for the image description
    HLock ((Handle) hImageDesc); // lock said handle
    err = GraphicsImportGetImageDescription (giComp, &hImageDesc); // retrieve the image description
    if (err != noErr) return;
	
    origImageWidth = (long) (rectImage.right - rectImage.left); // find width from right side - left side bounds
    origImageHeight = (long) (rectImage.bottom - rectImage.top); // same for height
	if (imageDepth <= 16) // we are using a 16 bit texture for all images 16 bits or less
	{
		imageDepth = 16;
		pixelFormat = k16BE555PixelFormat;
	}
    else // otherwise
	{
		imageDepth = 32;
		pixelFormat = k32ARGBPixelFormat;
	}
	
	SetRect (&rectImage, 0, 0, (short) imageWidth, (short) imageHeight); // l, t, r. b  set image rectangle for creation of GWorld
	rowStride = imageWidth * (imageDepth >> 3); // set stride in bytes width of image * pixel depth in bytes

	// create a new gworld using our unpadded buffer, ensure we set the pixel type correctly for the expected image bpp
	QTNewGWorldFromPtr (&pGWorld, pixelFormat, &rectImage, NULL, NULL, 0, imagePtr, rowStride); 
	if (NULL == pGWorld)
	{
		CloseComponent(giComp);
		return;
    }
    
	GetGWorld (&origPort, &origDevice); // save onscreen graphics port
	// decompress (draw) to gworld and thus fill buffer
    SetIdentityMatrix (&matrix); // set transform matrix to identity (basically pass thorugh)
	// this scale really only does something the case of non-tiled textures to inset them one pixel 
	//  thus maintaining the power of 2 (or desired) dimension of the overall texture
	ScaleMatrix (&matrix, X2Fix ((float) (imageWidth) / (float) origImageWidth), 
						  X2Fix ((float) (imageHeight) / (float) origImageHeight), 
						  X2Fix (0.0), X2Fix (0.0));
	// inset texture size to image size and offset by 1 pixel into the image so the 
	//  decompression puts the image into to center of the pixmap inset by one on each side
	TranslateMatrix (&matrix, X2Fix (0.0), X2Fix (0.0)); // step in for border
	err = GraphicsImportSetMatrix(giComp, &matrix); // set our matrix as the importer matrix
    if (err == noErr)
		err = GraphicsImportSetGWorld (giComp, pGWorld, NULL); // set the destination of the importer component
	if (err == noErr)
		err = GraphicsImportSetQuality (giComp, codecLosslessQuality); // we want lossless decompression
	if ((err == noErr) && GetGWorldPixMap (pGWorld) && LockPixels (GetGWorldPixMap (pGWorld)))
		GraphicsImportDraw (giComp); // if everything looks good draw image to locked pixmap
	else
	{
    	DisposeGWorld (pGWorld); // dump gworld
    	pGWorld = NULL;
		CloseComponent(giComp); // dump component
        return;
    }
	UnlockPixels (GetGWorldPixMap (pGWorld)); // unlock pixels
	CloseComponent(giComp); // dump component
	SetGWorld(origPort, origDevice); // set current graphics port to offscreen
	// done with gworld and image since they are loaded to a texture
	DisposeGWorld (pGWorld); // do not need gworld
	pGWorld = NULL;
}


- (id)initWithFrame:(NSRect)frameRect
{
	int i;
	NSBundle *mainBndl;
	NSString *bndlPath;
	
	// Init pixel format attribs
    NSOpenGLPixelFormatAttribute attrs[] =
    {
            NSOpenGLPFAAccelerated,
            NSOpenGLPFANoRecovery,
            NSOpenGLPFADoubleBuffer,
            0
    };

	// Get pixel format from OpenGL
    NSOpenGLPixelFormat* pixFmt = [[NSOpenGLPixelFormat alloc] initWithAttributes:attrs];
    if (!pixFmt)
    {
            NSLog(@"No pixel format -- exiting");
            exit(1);
    }
	
	// Set default path to resource directory
	mainBndl = [NSBundle mainBundle];
	bndlPath = [mainBndl resourcePath];
	chdir([bndlPath cString]);
	
    self = [super initWithFrame:frameRect pixelFormat:pixFmt];
    
	[[self openGLContext] makeCurrentContext];
	
	// Init object members
	texture_range  = GL_TRUE;
	texture_hint   = GL_STORAGE_CACHED_APPLE ;
	client_storage = GL_TRUE;
	buffers        = 5;
	rect_texture   = GL_TRUE;
	frame_rate     = 30;
	infiniteFPS    = 0;
	
	timer = [[NSTimer scheduledTimerWithTimeInterval: (1.0f / 30.0f) target: self selector:@selector(drawRect:) userInfo:self repeats:true] retain];
	
	// Setup some basic OpenGL stuff
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	
	image_base = (GLubyte *) malloc(IMAGE_COUNT * IMAGE_SIZE * IMAGE_SIZE * (IMAGE_DEPTH >> 3));
	
	// Load all images into memory
	for(i = 0; i < IMAGE_COUNT; i++)
	{
		image[i] = image_base + i * IMAGE_SIZE * IMAGE_SIZE * (IMAGE_DEPTH >> 3);
		[self loadBufferFromImageFile :[NSString stringWithFormat:@"../Resources/%d.jpg", i]:image[i]:IMAGE_SIZE:IMAGE_SIZE:IMAGE_DEPTH];
	}
	
	// Create and load textures for the first time
	[self loadTextures:GL_TRUE];
	
	// Init fps timer
	gettimeofday(&cycle_time, NULL);
	
	// Call for a redisplay
    [self setNeedsDisplay:true];
		
    return self;
}

- (void)displayMPixels
{
	static long loop_count = 0;
	struct timeval t2;
	unsigned long t;
	float avg_fps;
	
	loop_count++;
	
	gettimeofday(&t2, NULL);
	t = 1000000 * (t2.tv_sec - cycle_time.tv_sec) + (t2.tv_usec - cycle_time.tv_usec);
	
	// Display the average data rate
	if(t > 1000000 * STAT_UPDATE)
	{
		gettimeofday(&t2, NULL);
		t = 1000000 * (t2.tv_sec - cycle_time.tv_sec) + (t2.tv_usec - cycle_time.tv_usec);
		gettimeofday(&cycle_time, NULL);
		avg_fps = (1000000.0f * (float) loop_count) / (float) t;
	
		[setFPS setFloatValue:avg_fps];
		[setMPixels setFloatValue:(avg_fps * IMAGE_SIZE * IMAGE_SIZE * (IMAGE_DEPTH >> 3)) / 1000000];
		
		loop_count = 0;
		
		gettimeofday(&cycle_time, NULL);
	}
}

- (void)drawRect:(NSRect)aRect
{
	static long draw_image = 0;
	struct	timeval t1, t2;
	GLfloat	t=0, tr;
	GLuint	reps = 0, min_reps, i;
	
	// Make this context current
	[[self openGLContext] makeCurrentContext];
	
	if(infiniteFPS)
	{
		tr = 100000.0f;
		min_reps = 23;
	}
	else
	{
		tr = 0.0f;
		min_reps = 1;
	}
	
	// Bind, update and draw new image
	if(rect_texture)
	{
		gettimeofday(&t1, NULL);
		do
		{
			for(i = 0; i < min_reps; i++)
			{
				glBindTexture(GL_TEXTURE_RECTANGLE_EXT, draw_image+1);
				
				// glTexSubImage2D is faster when not using a texture range
				glTexSubImage2D(GL_TEXTURE_RECTANGLE_EXT, 0, 0, 0, IMAGE_SIZE, IMAGE_SIZE, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, image[draw_image]);
				//glTexImage2D(GL_TEXTURE_RECTANGLE_EXT, 0, GL_RGBA, IMAGE_SIZE, IMAGE_SIZE, 0, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, image[draw_image]);
				glBegin(GL_QUADS);
					glTexCoord2f(0.0f, 0.0f);
					glVertex2f(-1.0f, 1.0f);
					
					glTexCoord2f(0.0f, 1024.0f);
					glVertex2f(-1.0f, -1.0f);
					
					glTexCoord2f(1024.0f, 1024.0f);
					glVertex2f(1.0f, -1.0f);
					
					glTexCoord2f(1024.0f, 0.0f);
					glVertex2f(1.0f, 1.0f);
				glEnd();
				
				glFlush();
				
				// Increment to next image
				draw_image++;
				if(draw_image > (buffers - 1)) draw_image = 0;
				
				reps++;
			}
			
			gettimeofday(&t2, NULL);
			t = 1000000.0f *(t2.tv_sec - t1.tv_sec) +(t2.tv_usec - t1.tv_usec);
		}
		while(t < tr); //.1 seconds for infinite_fps, 0 otherwise
		
		if(infiniteFPS)
		{
			GLfloat fps;
			
			glFinish();
			gettimeofday(&t2, NULL);
			t = (t2.tv_sec - t1.tv_sec) + (t2.tv_usec - t1.tv_usec) / 1000000.0f;
			
			fps = (GLfloat) reps / t;
			
			[setFPS setFloatValue: fps];
			[setMPixels setFloatValue:(fps * IMAGE_SIZE * IMAGE_SIZE * (IMAGE_DEPTH >> 3)) / 1000000.0f];
		}
	}
	else
	{
		gettimeofday(&t1, NULL);
		do
		{
			for(i = 0; i < min_reps; i++)
			{
				glBindTexture(GL_TEXTURE_2D, draw_image+1);
				
				// glTexSubImage2D is faster when not using a texture range
				glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, IMAGE_SIZE, IMAGE_SIZE, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, image[draw_image]);
				//glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, IMAGE_SIZE, IMAGE_SIZE, 0, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, image[draw_image]);
				glBegin(GL_QUADS);
					glTexCoord2f(0.0f, 0.0f);
					glVertex2f(-1.0f, 1.0f);
					
					glTexCoord2f(0.0f, 1.0f);
					glVertex2f(-1.0f, -1.0f);
					
					glTexCoord2f(1.0f, 1.0f);
					glVertex2f(1.0f, -1.0f);
					
					glTexCoord2f(1.0f, 0.0f);
					glVertex2f(1.0f, 1.0f);
				glEnd();
				
				glFlush();
				
				// Increment to next image
				draw_image++;
				if(draw_image > (buffers - 1)) draw_image = 0;
				
				reps++;
			}
			
			gettimeofday(&t2, NULL);
			t = 1000000.0f *(t2.tv_sec - t1.tv_sec) +(t2.tv_usec - t1.tv_usec);
		}
		while(t < tr); //.1 seconds for infinite_fps, 0 otherwise
		
		if(infiniteFPS)
		{
			GLfloat fps;
			
			glFinish();
			gettimeofday(&t2, NULL);
			t = (t2.tv_sec - t1.tv_sec) + (t2.tv_usec - t1.tv_usec) / 1000000.0f;
			
			fps = (GLfloat) reps / t;
			
			[setFPS setFloatValue: fps];
			[setMPixels setFloatValue:(fps * IMAGE_SIZE * IMAGE_SIZE * (IMAGE_DEPTH >> 3)) / 1000000.0f];
		}
	}
	
	// Swap buffer to screen
	[[self openGLContext] flushBuffer];
	
	// Display the average data rate
	if(!infiniteFPS) [self displayMPixels];
}

- (void)update  // moved or resized
{
	NSRect rect;
	
	[super update];
	
	[[self openGLContext] makeCurrentContext];
	[[self openGLContext] update];
	
	rect = [self bounds];
	
    glViewport(0, 0, (int) rect.size.width, (int) rect.size.height);
	
	glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
	
	glMatrixMode(GL_MODELVIEW);
    glLoadIdentity(); 
	
	[self setNeedsDisplay:true];
}

- (void)reshape	// scrolled, moved or resized
{
	NSRect rect;
	
	[super reshape];
	
	[[self openGLContext] makeCurrentContext];
	[[self openGLContext] update];
	
	rect = [self bounds];
	
	glViewport(0, 0, (int) rect.size.width, (int) rect.size.height);
	
	glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
	
	glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
	
	[self setNeedsDisplay:true];
}

- (IBAction) clientStorage: (id) sender
{
	client_storage = [sender state];
	
	[self loadTextures:GL_FALSE];
}

- (IBAction) textureRange: (id) sender
{
	texture_range = [sender state];
	
	[self loadTextures:GL_FALSE];
}

- (IBAction) infinite: (id) sender
{
	GLfloat t;
	infiniteFPS = [sender state];
	
	if(infiniteFPS)
	{
		t = 0.5f;
		
		[timer invalidate];
		[timer release];
		
		timer = [[NSTimer scheduledTimerWithTimeInterval: t target: self selector:@selector(drawRect:) userInfo:self repeats:true] retain];
		
		[self setNeedsDisplay:true];
	}
	else
	{
		t = 1.0f / (GLfloat) frame_rate;
		
		[timer invalidate];
		[timer release];
		
		timer = [[NSTimer scheduledTimerWithTimeInterval: t target: self selector:@selector(drawRect:) userInfo:self repeats:true] retain];
	}
}

- (IBAction) rectTextures: (id) sender
{
	rect_texture = [sender state];
	
	[self loadTextures:GL_FALSE];
}

- (IBAction) textureHint: (id) sender
{
	if([[sender titleOfSelectedItem] isEqualToString:@"Cached"])   texture_hint = GL_STORAGE_CACHED_APPLE;
	if([[sender titleOfSelectedItem] isEqualToString:@"Private"])  texture_hint = GL_STORAGE_PRIVATE_APPLE;
	if([[sender titleOfSelectedItem] isEqualToString:@"Shared"])   texture_hint = GL_STORAGE_SHARED_APPLE;
	
	[self loadTextures:GL_FALSE];
}

- (IBAction) numBuffers: (id) sender
{
	buffers = [sender intValue];
}

- (IBAction) frameRate: (id) sender
{
	GLfloat t;
	frame_rate = [sender intValue];
	
	if(!infiniteFPS)
	{
		t = 1.0f / (GLfloat) frame_rate;
		
		[timer invalidate];
		[timer release];
		
		timer = [[NSTimer scheduledTimerWithTimeInterval: t target: self selector:@selector(drawRect:) userInfo:self repeats:true] retain];
	}
}

- (void)loadTextures: (GLboolean)first
{
	GLint i;
	
	[[self openGLContext] makeCurrentContext];
	[[self openGLContext] update];
	
	if(rect_texture)
	{
		for(i = 0; i < IMAGE_COUNT; i++)
		{
			if(!first)
			{
				GLint dt = i+1;
				glDeleteTextures(1, &dt);
			}
			
			glDisable(GL_TEXTURE_2D);
			glEnable(GL_TEXTURE_RECTANGLE_EXT);
			glBindTexture(GL_TEXTURE_RECTANGLE_EXT, i+1);
			
			if(texture_range) glTextureRangeAPPLE(GL_TEXTURE_RECTANGLE_EXT, IMAGE_COUNT * IMAGE_SIZE * IMAGE_SIZE * (IMAGE_DEPTH >> 3), image_base);
			else              glTextureRangeAPPLE(GL_TEXTURE_RECTANGLE_EXT, 0, NULL);
		
			glTexParameteri(GL_TEXTURE_RECTANGLE_EXT, GL_TEXTURE_STORAGE_HINT_APPLE , texture_hint);
			glPixelStorei(GL_UNPACK_CLIENT_STORAGE_APPLE, client_storage);
			glTexParameteri(GL_TEXTURE_RECTANGLE_EXT, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_RECTANGLE_EXT, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_RECTANGLE_EXT, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_RECTANGLE_EXT, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
			
			glTexImage2D(GL_TEXTURE_RECTANGLE_EXT, 0, GL_RGBA, IMAGE_SIZE,
				IMAGE_SIZE, 0, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, image[i]);
		}
	}
	else
	{
		glTextureRangeAPPLE(GL_TEXTURE_RECTANGLE_EXT, 0, NULL);
		if(texture_range) glTextureRangeAPPLE(GL_TEXTURE_2D, IMAGE_COUNT * IMAGE_SIZE * IMAGE_SIZE * (IMAGE_DEPTH >> 3), image_base);
		else              glTextureRangeAPPLE(GL_TEXTURE_2D, 0, NULL);
		
		for(i = 0; i < IMAGE_COUNT; i++)
		{
			if(!first)
			{
				GLint dt = i+1;
				glDeleteTextures(1, &dt);
			}
			
			glDisable(GL_TEXTURE_RECTANGLE_EXT);
			glEnable(GL_TEXTURE_2D);
			glBindTexture(GL_TEXTURE_2D, i+1);
			
			if(texture_range) glTextureRangeAPPLE(GL_TEXTURE_2D, IMAGE_COUNT * IMAGE_SIZE * IMAGE_SIZE * (IMAGE_DEPTH >> 3), image_base);
			else              glTextureRangeAPPLE(GL_TEXTURE_2D, 0, NULL);

			glTexParameteri(GL_TEXTURE_RECTANGLE_EXT, GL_TEXTURE_STORAGE_HINT_APPLE , texture_hint);
			glPixelStorei(GL_UNPACK_CLIENT_STORAGE_APPLE, client_storage);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
			
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, IMAGE_SIZE,
				IMAGE_SIZE, 0, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, image[i]);
		}
	}
}

- (void)mouseDragged:(NSEvent *)theEvent
{
}

- (void)mouseDown:(NSEvent *)theEvent
{
}

@end
