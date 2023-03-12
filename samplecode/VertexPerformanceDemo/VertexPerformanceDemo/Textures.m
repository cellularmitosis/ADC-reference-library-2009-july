/*
 *  Textures.c
 *  VertexPerformance
 *
 *  Created by Kent Miller on Tue Oct 29 2002.
 *  Copyright (c) 2002 Apple Computer. All rights reserved.
 *
 */

#include "Textures.h"

void TextureFromNSImage(NSImage *image, GLuint *texID, GLuint * width,GLuint *height)
{
    NSBitmapImageRep	*sourceImageRep;
    GLenum		imageFormat = GL_RGBA;
    GLubyte		*sourcePic;
    long		sourceRowBytes, pixelSize;
    NSSize		imageSize;

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
	
	NSLog(@"Image Width:%f, Image Height:%f\n", imageSize.width, imageSize.height);

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
    glBindTexture(GL_TEXTURE_2D, *texID);

    *width  = imageSize.width;
    *height = imageSize.height;

    glTexImage2D(GL_TEXTURE_RECTANGLE_EXT, 0, imageFormat, *width, *height, 0, imageFormat, GL_UNSIGNED_BYTE, sourcePic);

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

