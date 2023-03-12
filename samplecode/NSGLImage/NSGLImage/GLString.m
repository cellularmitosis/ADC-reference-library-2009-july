/*
 *  GLString.m

 *  Created November 3 2004.
 *  Copyright (c) 2004 Apple Computer Inc. All rights reserved.

	Disclaimer:	IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc.
			("Apple") in consideration of your agreement to the following terms, and your
			use, installation, modification or redistribution of this Apple software
			constitutes acceptance of these terms.  If you do not agree with these terms,
			please do not use, install, modify or redistribute this Apple software.

			In consideration of your agreement to abide by the following terms, and subject
			to these terms, Apple grants you a personal, non-exclusive license, under Apple‚Äôs
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

#import "GLString.h"

@implementation GLString

// Accessors
-(GLuint)texID						{ return texID; }
-(NSSize)texSize					{ return texSize; }
-(NSColor*)textColor				{ return textColor; }
-(NSColor*)boxColor					{ return boxColor; }
-(NSColor*)borderColor				{ return borderColor; }
-(NSSize)marginSize					{ return marginSize; }
-(BOOL)staticFrame					{ return staticFrame; }

// =======================================================

-(id)init
{
	self = [super init];
	cgl_ctx = NULL;
	texID = 0;
	texSize.width = 0.0f;
	texSize.height = 0.0f;
	str = [[[NSMutableAttributedString alloc] init] retain];
	textColor = [[NSColor colorWithDeviceRed:1.0 green:1.0 blue:1.0 alpha:0.75] retain];
	boxColor = [[NSColor colorWithDeviceRed:0.667 green:0.75 blue:0.667 alpha:0.75] retain];
	borderColor  = [[NSColor colorWithDeviceRed:1.0 green:1.0 blue:1.0 alpha:0.75] retain];
	attribs = [[NSMutableDictionary dictionary] retain];
	theFont = [[NSFont fontWithName:@"Helvetica" size:12] retain];
	staticFrame = NO;
	marginSize.width = 4.0f; // standard margins
	marginSize.height = 2.0f;
	[self initTextAttributes];
	return self;
}

-(void)initTextAttributes
{
	[attribs setObject:theFont forKey:NSFontAttributeName];
	[attribs setObject:textColor forKey:NSForegroundColorAttributeName];
}

-(id)initWithString:(NSMutableString*)newString
{
	self = [self init];
	[[str mutableString] setString:newString];
	return self;
}

-(id)initWithAttributedString:(NSAttributedString*)newString
{
	[str setAttributedString:newString];
	[self genTexture];
	return self;
}

-(id)initWithString:(NSMutableString*)newString textColor:(NSColor*)txColor boxColor:(NSColor*)bxColor borderColor:(NSColor*)brColor
{
	// Release our old colors
	if(textColor)
		[textColor release];
	if(boxColor)
		[boxColor release];
	if(borderColor)
		[borderColor release];
	// Retain our new colors
	textColor = [txColor retain];
	boxColor = [bxColor retain];
	borderColor = [brColor retain];
	
	// Change our text color
	[attribs setObject:textColor forKey:NSForegroundColorAttributeName];
	[self initWithString:newString];
	return self;
}

-(void)dealloc
{
	[self deleteTexture];
	[textColor release];
	[boxColor release];
	[borderColor release];
	[str release];
	[theFont release];
	[attribs release];
	[super dealloc];
}

-(void)deleteTexture
{
	if (texID && cgl_ctx) {
		(*cgl_ctx->disp.delete_textures)(cgl_ctx->rend, 1, &texID);
		texID = 0;
		cgl_ctx = 0;
	}
}

-(void)setString:(NSMutableString*)newString
{
	[[str mutableString] setString:newString];
	// Make sure we update our texture
	[self genTexture];
}

-(void)setTextColor:(NSColor*)newColor
{
	if(textColor)
		[textColor release];
	textColor = [newColor retain];
}

-(void)setBorderColor:(NSColor*)newColor
{
	if(borderColor)
		[borderColor release];
	borderColor = [newColor retain];
}

-(void)setBoxColor:(NSColor*)newColor
{
	if(boxColor)
		[boxColor release];
	boxColor = [newColor retain];
}

-(NSSize)frameSize
{
	if ((NO == staticFrame) && (0.0f == frameSize.width) && (0.0f == frameSize.height)) { // find frame size if we have not already found it
		frameSize = [str size]; // current string size
		frameSize.width += marginSize.width * 2.0f; // add padding
		frameSize.height += marginSize.height * 2.0f;
	}
	return frameSize;
}

-(void)genTexture; // generates the texture without drawing texture to current context
{
	NSImage* image;
	
	[self deleteTexture];
	if ((NO == staticFrame) && (0.0f == frameSize.width) && (0.0f == frameSize.height)) { // find frame size if we have not already found it
		frameSize = [str size]; // current string size
		frameSize.width += marginSize.width * 2.0f; // add padding
		frameSize.height += marginSize.height * 2.0f;
	}
	image = [[NSImage alloc] initWithSize:frameSize];
	[image lockFocus];
	if ([boxColor alphaComponent]) { // this should be == 0.0f but need to make sure
		[boxColor set]; 
		NSRectFill (NSMakeRect (0.0f, 0.0f, frameSize.width, frameSize.height));
	}
	if ([borderColor alphaComponent]) {
		[borderColor set]; 
		NSFrameRect (NSMakeRect (0.0f, 0.0f, frameSize.width, frameSize.height));
	}
	[str setAttributes:attribs range:NSMakeRange(0, [str length])];
	[textColor set]; 
	[str drawAtPoint:NSMakePoint (marginSize.width, marginSize.height)]; // draw at offset position
	// Remove our old bitmap if it's present
	if(bmpRep)
		[bmpRep release];
	// Allocate the new bitmap image rep
	bmpRep = [[NSBitmapImageRep alloc] initWithFocusedViewRect:NSMakeRect (0.0f, 0.0f, frameSize.width, frameSize.height)];
	[image unlockFocus];
	texSize.width = [bmpRep size].width;
	texSize.height = [bmpRep size].height;
	if (cgl_ctx = CGLGetCurrentContext())
	{ // if we successfully retrieve a current context (required)
		glPixelStorei(GL_UNPACK_CLIENT_STORAGE_APPLE, 1);
		glPixelStorei(GL_UNPACK_ROW_LENGTH, texSize.width);
		glGenTextures (1, &texID);
		glBindTexture (GL_TEXTURE_RECTANGLE_EXT, texID);
		glTexImage2D (GL_TEXTURE_RECTANGLE_EXT, 0, GL_RGBA, texSize.width, texSize.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, [bmpRep bitmapData]);
	}
	else
		NSLog (@"StringTexture -genTexture: Failure to get current OpenGL context\n");
	[image release];
}

-(void)drawWithBounds:(NSRect)bounds
{
	if (!texID)
		[self genTexture];
	if (texID) {
		glBindTexture (GL_TEXTURE_RECTANGLE_EXT, texID);
		glBegin (GL_QUADS);
			glTexCoord2f (0.0f, 0.0f); // draw upper left in world coordinates
			glVertex2f (bounds.origin.x, bounds.origin.y);
	
			glTexCoord2f (0.0f, texSize.height); // draw lower left in world coordinates
			glVertex2f (bounds.origin.x, bounds.origin.y + bounds.size.height);
	
			glTexCoord2f (texSize.width, texSize.height); // draw upper right in world coordinates
			glVertex2f (bounds.origin.x + bounds.size.width, bounds.origin.y + bounds.size.height);
	
			glTexCoord2f (texSize.width, 0.0f); // draw lower right in world coordinates
			glVertex2f (bounds.origin.x + bounds.size.width, bounds.origin.y);
		glEnd ();
	}
}

-(void)drawAtPoint:(NSPoint)point
{
	if (!texID)
		[self genTexture]; // ensure size is calculated for bounds
	if (texID) // if successful
		[self drawWithBounds:NSMakeRect (point.x, point.y, texSize.width, texSize.height)];
}

// these will force the texture to be regenerated at the next draw
- (void) setMargins:(NSSize)size // set offset size and size to fit with offset
{
	[self deleteTexture];
	marginSize = size;
	if (NO == staticFrame) { // ensure dynamic frame sizes will be recalculated
		frameSize.width = 0.0f;
		frameSize.height = 0.0f;
	}
}

- (void) useStaticFrame:(NSSize)size // set static frame size and size to frame
{
	[self deleteTexture];
	frameSize = size;
	staticFrame = YES;
}

- (void) useDynamicFrame
{
	if (staticFrame) { // set to dynamic frame and set to regen texture
		[self deleteTexture];
		staticFrame = NO;
		frameSize.width = 0.0f; // ensure frame sizes will be recalculated
		frameSize.height = 0.0f;
	}
}

@end
