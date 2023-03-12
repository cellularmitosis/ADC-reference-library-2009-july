/*
 *  GLImage.m

 *  Created November 3 2004.
 *  Copyright (c) 2004 Apple Computer Inc. All rights reserved.

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

#import "GLImage.h"


@implementation GLImage

-(NSBitmapImageRep*)bmpImage	{ return bmpRep; }
-(GLString*)imageInfo			{ return imageInfo; }

-(id)init
{
	self = [super init];
	scaleFactor = 100.0;
	imgWidth = imgHeight = 0.0;
	scaleX = scaleY = 1.0;
	txID = 0;
	[self genQuad];
	ready = NO;
	return self;
}

-(void)genQuad
{
	float base = 1.0;
	float depth = 1.0;
	
	float scaledX = base * scaleX;
	float scaledY = base * scaleY;
	
	glQuad = malloc(sizeof(Vector3) * 4);
	
	glQuad[0].x = scaledX;
	glQuad[0].y = scaledY;
	glQuad[0].z = depth;

	glQuad[1].x = -scaledX;
	glQuad[1].y = scaledY;
	glQuad[1].z = depth;

	glQuad[2].x = -scaledX;
	glQuad[2].y = -scaledY;
	glQuad[2].z = depth;

	glQuad[3].x = scaledX;
	glQuad[3].y = -scaledY;
	glQuad[3].z = depth;

}

-(void)drawTexture
{
	// Bail if our image rep is invalid
	if(!bmpRep)
		return;

	// Bail out if we have no image data
	if(![bmpRep bitmapData])
		return;

	// Shut off depth and face culling
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	// Enable texturing
	glEnable(GL_TEXTURE_RECTANGLE_EXT);
	// Make sure we've got the right texture
	glBindTexture(GL_TEXTURE_RECTANGLE_EXT, txID);
	
	// Set color so we get full display
	glColor4f(1.0, 1.0, 1.0, 1.0);
	
	// Draw it!
	glBegin(GL_QUADS);
		glTexCoord2f(0.0, 0.0);								// Lower Left
		glVertex3f(glQuad[0].x, glQuad[0].y, glQuad[0].z);	// Upper Right

		glTexCoord2f(imgWidth, 0.0);						// Lower Right
		glVertex3f(glQuad[1].x, glQuad[1].y, glQuad[1].z);	// Upper Left

		glTexCoord2f(imgWidth, imgHeight);					// Upper Right
		glVertex3f(glQuad[2].x, glQuad[2].y, glQuad[2].z);	// Lower Left

		glTexCoord2f(0.0, imgHeight);						// Upper Left
		glVertex3f(glQuad[3].x, glQuad[3].y, glQuad[3].z);	// Lower Right
	glEnd();

	// Gotta turn this off here, or we die
	glPixelStorei(GL_UNPACK_CLIENT_STORAGE_APPLE, 0);
	// Turn all the other stuff back on
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glEnable(GL_CULL_FACE);
	// Reset the blending function to its previous state
	glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
}

-(void)initGL
{
	GLuint err = 0;
	// Enable texturing
	glEnable(GL_TEXTURE_RECTANGLE_EXT);
	// Enable client storage
	glPixelStorei(GL_UNPACK_CLIENT_STORAGE_APPLE, 1);
	glPixelStorei(GL_UNPACK_ROW_LENGTH, imgWidth);
	// Generate a texture ID
	glGenTextures(1, &txID);
	// Bind to our newly created texture ID
	glBindTexture(GL_TEXTURE_RECTANGLE_EXT, txID);
	// Specify the texture
	glTexImage2D(GL_TEXTURE_RECTANGLE_EXT, 0, bpp, imgWidth, imgHeight, 0, GL_RGB, datatype, [bmpRep bitmapData]);
	err = glGetError();
	if(err)
	{
		NSLog(@"WARNING: OpenGL Error 0x%x while loading texture with glTexImage2D().", err);
		[self discardData];
		ready = NO;
		return;
	}
	// Rebuild our geometry to match the texture
	[self genQuad];
	ready = YES;
}


-(void)loadTextureFromFile:(NSString*)filePath;
{
	// First, deallocate any image reps we might have.
	if(bmpRep)
	{
		[bmpRep release];
		// We also need to delete our texture ID
		glDeleteTextures(1, &txID);
	}
	if(!filePath)
		return;
	// Get raw bitmap data from the NSImage
	bmpRep = [[NSBitmapImageRep imageRepWithContentsOfFile:filePath] retain];
	// Set the image height & width for convenience
	imgWidth = [bmpRep size].width;
	imgHeight = [bmpRep size].height;
	[self genImageInfo:[filePath lastPathComponent]];
	// Set our scaling factors
	scaleX = imgWidth / scaleFactor;
	scaleY = imgHeight / scaleFactor;
	NSLog(@"Image: %d x %d", imgWidth, imgHeight);
	[self getTextureInfo];
	// Run through our initialization routine
	[self initGL];
	
}

-(void)discardData
{
	// Dump the bitmap
	if(bmpRep)
	{
		// Release it
		[bmpRep release];
		// Null out the pointer
		bmpRep = nil;
	}

	// Dump the texture
	glDeleteTextures(1, &txID);
	
	// Clear the image width & height
	imgWidth = 0.0;
	imgHeight = 0.0;
	
	[self genImageInfo:@"No Info"];
}

-(void)dealloc
{
	if(glQuad)
		free(glQuad);
	
	[super dealloc];
}

-(void)getTextureInfo
{
	if([bmpRep hasAlpha])
		bpp = GL_RGBA;
	else
		bpp = GL_RGB;
	
	datatype = GL_UNSIGNED_BYTE;
}

-(void)genImageInfo:(NSString*)imgName
{
	NSMutableString *info = [NSMutableString stringWithFormat:@"Image Name:%@\n Size (w x h): %d x %d pixels ", imgName, imgWidth, imgHeight];
	
	if(!imageInfo)
		imageInfo = [[(GLString*)[GLString alloc] initWithString:info] retain];
	else
		[imageInfo setString:info];
	
}

@end
