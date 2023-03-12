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
	ScaleMatrix (&matrix, X2Fix ((GLfloat) (imageWidth) / (GLfloat) origImageWidth), 
						  X2Fix ((GLfloat) (imageHeight) / (GLfloat) origImageHeight), 
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
	frame_rate       = 30;
	image_size       = 1024;
	infinite_fps     = GL_FALSE;
	
	timer = [[NSTimer scheduledTimerWithTimeInterval: (1.0f / 30.0f) target: self selector:@selector(drawRect:) userInfo:self repeats:true] retain];
	
	// Setup some basic OpenGL stuff
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glClearColor(1.0, 1.0, 1.0, 1.0);
	glColor4f(1.0, 1.0, 1.0, 1.0);
	glEnable(GL_RASTER_POSITION_UNCLIPPED_IBM);
	glDisable(GL_DITHER);
	
	// Load all images into memory
	image = malloc(image_size * image_size * 4);
	[self loadBufferFromImageFile :[NSString stringWithFormat:@"../Resources/0.jpg"]:image:image_size:image_size:32];
	
	// Init fps timer
	gettimeofday(&cycle_time, NULL);
	
	rasterPosX = 0;
	rasterPosY = 0;

	// Call for a redisplay
	[self setNeedsDisplay:true];
	return self;
}

- (void)displayMPixels
{
	static long loop_count = 0;
	struct timeval t2;
	unsigned long t;
	GLfloat avg_fps;
	
	loop_count++;
	
	gettimeofday(&t2, NULL);
	t = 1000000.0f * (t2.tv_sec - cycle_time.tv_sec) + (t2.tv_usec - cycle_time.tv_usec);
	
	// Display the average data rate
	if(t > 1000000.0f * STAT_UPDATE)
	{
		gettimeofday(&t2, NULL);
		t = 1000000.0f * (t2.tv_sec - cycle_time.tv_sec) + (t2.tv_usec - cycle_time.tv_usec);
		gettimeofday(&cycle_time, NULL);
		avg_fps = (1000000.0f * (GLfloat) loop_count) / (GLfloat) t;
	
		[setFPS setFloatValue:avg_fps];
		[setMPixels setFloatValue:(avg_fps * image_size * image_size * 4) / 1000000.0f];
		
		loop_count = 0;
		
		gettimeofday(&cycle_time, NULL);
	}
}

- (void)mouseDown:(NSEvent *)theEvent
{
    BOOL keepOn = YES;
    BOOL isInside = YES;
    NSPoint mouseLoc;
    while (keepOn) {
        theEvent = [[self window] nextEventMatchingMask: NSLeftMouseUpMask |
                NSLeftMouseDraggedMask];
        mouseLoc = [self convertPoint:[theEvent locationInWindow] fromView:nil];
        isInside = [self mouse:mouseLoc inRect:[self bounds]];


        switch ([theEvent type]) 
        {
            case NSLeftMouseUp:
                    keepOn = NO;
            case NSLeftMouseDragged:
                    //if (mouseLoc.x < 0) mouseLoc.x = 0;
                    //if (mouseLoc.y < 0) mouseLoc.y = 0;
                   // if (mouseLoc.x > [self bounds].size.width) [self bounds].size.width - 1;
                    //if (mouseLoc.y > [self bounds].size.height) [self bounds].size.height - 1;
                    rasterPosX = mouseLoc.x - ([self bounds].size.width/2);
                    rasterPosY = mouseLoc.y - ([self bounds].size.height/2);
                    [self display];
                    break;
            default:
                    /* Ignore any other kind of event. */
                    break;        }
    }

    return;
}

- (void)keyDown:(NSEvent *)theEvent
{
    unichar  unicodeKey = [[theEvent charactersIgnoringModifiers] characterAtIndex:0];;
    if (unicodeKey==NSUpArrowFunctionKey)
    {
        [zoomSlider setFloatValue:[zoomSlider floatValue]+0.01f];
        [zoomText setFloatValue:[zoomSlider floatValue]];
        [self zoom:zoomSlider];
    }
    else if (unicodeKey == NSDownArrowFunctionKey)
    {
        [zoomSlider setFloatValue:[zoomSlider floatValue]-0.01f];
        [zoomText setFloatValue:[zoomSlider floatValue]];
        [self zoom:zoomSlider];
    }
}
- (BOOL) acceptsFirstResponder{ return YES;}
- (void)drawRect:(NSRect)aRect
{
	// Make this context current
	[[self openGLContext] makeCurrentContext];
	
	glClear(GL_COLOR_BUFFER_BIT);
	
	glRasterPos2f(rasterPosX, rasterPosY);

	if(infinite_fps)
	{
		struct	timeval t1, t2;
		GLfloat	t=0;
		GLuint	reps = 0, i;
	
		gettimeofday(&t1, NULL);
		do
		{
			for(i = 0; i < 50; i++)
				glDrawPixels(image_size, image_size, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, image);
			
			gettimeofday(&t2, NULL);
			t = 1000000.0f *(t2.tv_sec - t1.tv_sec) +(t2.tv_usec - t1.tv_usec);
			reps += 50;
		}
		while(t < 100000.0f); //.1 seconds for infinite_fps
		glFinish();
		gettimeofday(&t2, NULL);
		t = (t2.tv_sec - t1.tv_sec) + (t2.tv_usec - t1.tv_usec) / 1000000.0f;
		
		[setFPS setFloatValue: (GLfloat) reps / t];
		[setMPixels setFloatValue:(reps * image_size * image_size * 4) / (1000000.0f * t)];
	}
	else
	{
		glDrawPixels(image_size, image_size, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, image);
		
		// Display the average data rate
		[self displayMPixels];
	}
	
	// Swap buffer to screen
	[[self openGLContext] flushBuffer];
}

- (void)update  // moved or resized
{
	NSRect rect;
	
	[super update];
	
	[[self openGLContext] makeCurrentContext];
	[[self openGLContext] update];
	
	rect = [self bounds];
	
	glViewport(0, 0, (GLint) rect.size.width, (GLint) rect.size.height);
	
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(-(rect.size.width/2), rect.size.width/2, -(rect.size.height/2), rect.size.height/2);
	
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
	
	glViewport(0, 0, (GLint) rect.size.width, (GLint) rect.size.height);
	
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(-(rect.size.width/2), rect.size.width/2, -(rect.size.height/2), rect.size.height/2);
	
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
        
	[self setNeedsDisplay:true];
}

- (IBAction) zoom: (id) sender
{
	GLfloat z = [sender floatValue];
	
	if(z > 0.995f && z < 1.005f) z = 1.0f;
	glPixelZoom(z, z);
	[zoomText setFloatValue:z];
	[self setNeedsDisplay:true];
}

- (IBAction) frameRate: (id) sender
{
	GLfloat t;
	frame_rate = [sender intValue];
	
	if(!infinite_fps)
	{
		t = 1.0f / (GLfloat) frame_rate;
		
		[timer invalidate];
		[timer release];
		
		timer = [[NSTimer scheduledTimerWithTimeInterval: t target: self selector:@selector(drawRect:) userInfo:self repeats:true] retain];
	}
}

- (IBAction) imageSize: (id) sender
{
	NSString *str;
	image_size = [sender intValue];
	
	if(image) free(image);
	
	image = malloc(image_size * image_size * 4);
	[self loadBufferFromImageFile :[NSString stringWithFormat:@"../Resources/0.jpg"]:image:image_size:image_size:32];
	
	str = [NSString stringWithFormat:@"%dx%dx4", image_size, image_size];
	[setImageSize setIntValue:image_size];
	[setImageSizeTitle setStringValue:str];
}

- (IBAction) infiniteFPS: (id) sender
{
	GLfloat t;
	infinite_fps = [sender intValue];
	
	if(infinite_fps)
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

- (void)mouseDragged:(NSEvent *)theEvent
{
	[self setNeedsDisplay:true];
}

@end
