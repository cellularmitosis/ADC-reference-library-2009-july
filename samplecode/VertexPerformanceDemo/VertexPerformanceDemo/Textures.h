/*
 *  Textures.h
 *  VertexPerformance
 *
 *  Created by Kent Miller on Tue Oct 29 2002.
 *  Copyright (c) 2002 Apple Computer. All rights reserved.
 *
 */

#import <Cocoa/Cocoa.h>
#import <OpenGL/gl.h>

#define kTextureUnitCount 8

void TextureFromNSImage(NSImage *image, GLuint *texID, GLuint * width,GLuint *height);
void StringToTexture( char * string, char * fontName, 
            int fontSize, GLuint *texID, GLuint texWidth, 
            GLuint texHeight, NSColor *color);
