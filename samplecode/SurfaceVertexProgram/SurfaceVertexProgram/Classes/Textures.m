/*
 *  Textures.c
 *  VertexPerformance
 *
 *  Created by Kent Miller on Tue Oct 29 2002.
 *  Copyright (c) 2002 Apple Computer. All rights reserved.
 *

 Disclaimer: IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc.
   ("Apple") in consideration of your agreement to the following terms, and your
   use, installation, modification or redistribution of this Apple software
   constitutes acceptance of these terms.  If you do not agree with these terms,
   please do not use, install, modify or redistribute this Apple software.

   In consideration of your agreement to abide by the following terms, and subject
   to these terms, Apple grants you a personal, non-exclusive license, under Apple's
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


#include "Textures.h"

static unsigned int truncPowerOf2(unsigned int x)
{
    int i = 0;
    while (x = x>>1) i++;
    return (1<<i);
}

void TextureFromNSImage(NSImage *image, GLuint *texID, GLuint * width,GLuint *height)
{
    NSBitmapImageRep	*sourceImageRep;
    GLenum		imageFormat = GL_RGBA;
    GLubyte		*sourcePic;
    long		sourceRowBytes, pixelSize;
    NSSize		imageSize;
	GLint		err;
	
    sourceImageRep = [[NSBitmapImageRep alloc]initWithData: [image TIFFRepresentation]];

    if ([sourceImageRep hasAlpha] == YES)
    {
        if ([sourceImageRep bitsPerPixel] != 32) return;
        imageFormat = GL_RGBA;
        pixelSize = 4;
    }
    else
    {
        if ([sourceImageRep bitsPerPixel] != 24) return;
        imageFormat = GL_RGB;
        pixelSize = 3;
    }

    sourceRowBytes = [sourceImageRep bytesPerRow];
    sourcePic = (GLubyte*) [sourceImageRep bitmapData];
    imageSize = [sourceImageRep size];

    //Do the OpenGL flip
    {
        GLubyte *pic = malloc( imageSize.height * sourceRowBytes );
        GLuint	i;
        
        for (i = 0; i < imageSize.height; i++)
        {
            memcpy ( pic + (i * sourceRowBytes),
                    sourcePic + ((((int)imageSize.height - i) - 1) * sourceRowBytes),
                    sourceRowBytes);
        }
        sourcePic = pic;
    }

    glPixelStorei(GL_UNPACK_ROW_LENGTH, sourceRowBytes/pixelSize);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    if (*texID == 0)
        glGenTextures(1, texID);
		
    glBindTexture(GL_TEXTURE_2D,*texID);

    *width = truncPowerOf2(imageSize.width);
    *height = truncPowerOf2(imageSize.height);

    glTexImage2D(GL_TEXTURE_2D, 0, imageFormat, *width, *height, 0, imageFormat, GL_UNSIGNED_BYTE, sourcePic);
    if (err = glGetError())
    {
            printf("GL reports err %0x8X\n", (unsigned int)err);
    }
	
    [sourceImageRep release];
}

void StringToTexture( char * string, char * fontName, 
            int fontSize, GLuint *texID, GLuint texWidth, 
            GLuint texHeight, NSColor *color)
{
    NSImage		*textImage = [[NSImage alloc] initWithSize:NSMakeSize((float)texWidth, (float) texHeight)];
    NSString 		*s = [NSString stringWithCString: string];
    NSMutableDictionary *textAttributes = [[NSMutableDictionary alloc]init];
    GLuint		w,h;

    [textAttributes setObject: [NSFont fontWithName: [NSString stringWithCString: fontName] size: fontSize] forKey: NSFontAttributeName];
    if (color == 0)
        [textAttributes setObject: [NSColor whiteColor] forKey: NSForegroundColorAttributeName];
    else
        [textAttributes setObject: color forKey: NSForegroundColorAttributeName];
    
    [textImage lockFocus];

    [s drawInRect: NSMakeRect(0,0,texWidth,texHeight) withAttributes:textAttributes];

    if (*texID == 0)
        glGenTextures(1, texID);
    glBindTexture(GL_TEXTURE_2D, *texID);

    TextureFromNSImage(textImage, texID, &w, &h);

    [textImage unlockFocus];
}

