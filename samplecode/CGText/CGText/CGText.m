/*

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

*/

#import <Appkit/AppKit.h>

#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#include <OpenGL/glext.h>

#import "GBuffer.h"

#import "CGText.h"


typedef struct _GlyphInfo
{
	CGRect bounds;
	float v_offset;
} GlyphInfo;

typedef struct _FontRecord
{
	char name[256];
	int size;
	GBuffer* image;
	NSFont* font;

	GlyphInfo glyph_infos[256];
	GLenum texture_id;

	CGSize inv_buffer_size;
} FontRecord;


/* the following 2 are temporaries, used when iterating through the glyphs of a font */

CGPoint image_offset;
float current_row_max_height;



@implementation CGText

/* ProcessGlyph will render a single glyph into the image bitmap */

void ProcessGlyph(unsigned char c, FontRecord* info, NSTextView* text_view)
{
	GlyphInfo* glyph_info = &info->glyph_infos[c];
	char c_str[2] = { c, '\0' };

	float font_height = [info->font ascender] - [info->font descender];

	[text_view setString:[NSString stringWithCString:c_str]];

	{
		NSGlyph char_glyph = [[text_view layoutManager] glyphAtIndex:0];

		if (char_glyph > [info->font numberOfGlyphs])
		{
			glyph_info->bounds = CGRectMake(0,0,0,0);
			return;
		}
		else
		{
			NSPoint nominal_glyph_size = [info->font positionOfGlyph: NSNullGlyph precededByGlyph: char_glyph isNominal: nil];

			if (image_offset.x + nominal_glyph_size.x > info->image->buffer_size.width)
			{
				image_offset.x = 0;
				image_offset.y += current_row_max_height;
				current_row_max_height = 0;
			}
	
			[info->image renderGlyph:(CGGlyph) char_glyph typeface:info->name size:info->size offset:image_offset];
		
			/* the 0.999's below are fudge factors */

			glyph_info->bounds = CGRectMake(image_offset.x+0.999, image_offset.y, nominal_glyph_size.x+0.999, font_height); 
			glyph_info->v_offset = [info->font descender];
	
			if (current_row_max_height < font_height)
				current_row_max_height = font_height;
				
			image_offset.x += nominal_glyph_size.x+4; //allow 4 slop columns between each character
		}
	}
}

/* SetFontImage will render all legal glyphs into the font's image bitmap */

void SetFontImage(FontRecord* info, BOOL debug)
{
	NSTextView* text_view = [[NSTextView alloc] initWithFrame:NSMakeRect(0.0f,0.0f,128.0f,128.0f)];

	info->font = [NSFont fontWithName:[NSString stringWithCString: info->name] size:info->size];

	[text_view setFont:info->font];

	[info->image clear: debug ? 1.0 : 0.0];

	image_offset = CGPointMake(0,- [info->font descender]);
	current_row_max_height = 0;

	{
		int i;
		for (i = 33; i < 256; i++)
			ProcessGlyph(i, info, text_view);
	}
	
	[text_view release];
	
	[info->font release];
	info->font = nil;
}


/* SetFontTexture will create an OpenGL texture from the font image */

void SetFontTexture(FontRecord* info)
{
	GBuffer* image = info->image;

	GLenum rgba_mode = GL_LUMINANCE_ALPHA;
	GLenum format = GL_BGRA_EXT;
	GLenum type = image->buffer_bitdepth == 32 ? GL_UNSIGNED_INT_8_8_8_8_REV : GL_UNSIGNED_SHORT_1_5_5_5_REV;

	glGenTextures(1, &info->texture_id);
	glBindTexture(GL_TEXTURE_2D, info->texture_id);
	
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

	glTexImage2D(GL_TEXTURE_2D, 0, rgba_mode, (GLint) image->buffer_size.width, (GLint) image->buffer_size.height, 0, format, type, image->buffer_ptr);

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	glBindTexture(GL_TEXTURE_2D, 0);
	
	info->inv_buffer_size = CGSizeMake(1/image->buffer_size.width, 1/image->buffer_size.height);
}
			

/* SetFont is called to bind the current font's OpenGL texture */

void SetFont(FontRecord* font)
{
	if (!font->texture_id)
		return;
		
	glBindTexture(GL_TEXTURE_2D, font->texture_id);
}

/* RenderFontText is called to render out the text. Note that an 2D projection is assumed */
	
void RenderFontText(FontRecord* font, const char* text, CGPoint dst)
{
	GLfloat coord_array[256*4][2];
	GLfloat vertex_array[256*4][2];

	SetFont(font);

	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glEnableClientState(GL_VERTEX_ARRAY);

	glColor4f(1.0, 1.0, 1.0, 1.0);

	{
		const unsigned char* c;
		
		int num_quads = 0;

		GLfloat* current_coord = &coord_array[0][0];
		GLfloat* current_vertex = &vertex_array[0][0]; 

		glTexCoordPointer(2, GL_FLOAT, 0, coord_array);
		glVertexPointer(2, GL_FLOAT, 0, vertex_array);

		for (c = (const unsigned char*) text; *c; dst.x += (*c <= ' ' ? 8 : font->glyph_infos[*c].bounds.size.width), ++c)
		{
			if (*c > ' ')
			{
				typedef struct { GLfloat s,t; } TexCoord;
				typedef struct { GLfloat x,y; } VertexCoord;
				
				GlyphInfo* info = &font->glyph_infos[*c];

				TexCoord tc0 = { info->bounds.origin.x * font->inv_buffer_size.width, (info->bounds.origin.y + info->v_offset) * font->inv_buffer_size.height };
				TexCoord tc1 = { info->bounds.size.width * font->inv_buffer_size.width, info->bounds.size.height * font->inv_buffer_size.height };
				
				tc1.s += tc0.s, tc1.t += tc0.t;
		
				// the image we loaded was inverted, so invert texture coordinates t component here:

				tc0.t = 1 - tc0.t;
				tc1.t = 1 - tc1.t;
	

				// push coords into arrays:

				*current_coord++ = tc0.s;
				*current_coord++ = tc0.t;
					
				*current_vertex++ = dst.x;
				*current_vertex++ = dst.y;
				
				*current_coord++ = tc1.s;
				*current_coord++ = tc0.t;
					
				*current_vertex++ = dst.x+info->bounds.size.width;
				*current_vertex++ = dst.y;
	
				*current_coord++ = tc1.s;
				*current_coord++ = tc1.t;
					
				*current_vertex++ = dst.x+info->bounds.size.width;
				*current_vertex++ = dst.y+info->bounds.size.height;
				
				*current_coord++ = tc0.s;
				*current_coord++ = tc1.t;
					
				*current_vertex++ = dst.x;
				*current_vertex++ = dst.y+info->bounds.size.height;
				
				num_quads++;
			}
		}

		glDrawArrays(GL_QUADS, 0, num_quads*4);

		glDisableClientState(GL_TEXTURE_COORD_ARRAY);
		glDisableClientState(GL_VERTEX_ARRAY);
	}
}

FontRecord* font_info;	

+ (void) setFont:(const char*) typeface size:(int) size debug:(BOOL) debug
{
	if (!font_info)
	{
		font_info = malloc(sizeof(FontRecord));
		font_info->texture_id = 0;
	}
	else
	{
		glDeleteTextures(1, &font_info->texture_id);
		font_info->texture_id = 0;
	}
	
	strcpy(font_info->name, typeface);
	font_info->size = size;

	/* NOTE: the CG copyRect routine doesn't really support 8 or 16 bit images */
	/* Also the hardcoded 256x256 size won't be big enough for larger text sizes */
	
	font_info->image = [GBuffer create: CGSizeMake(256, 256) bitDepth:32];

	SetFontImage(font_info, debug);
	SetFontTexture(font_info);

	[font_info->image release];
	font_info->image = nil;
}

+ (void) displayText:(const char*) text at: (CGPoint) dst;
{
	if (font_info && text)
		RenderFontText(font_info, text, dst);
}

@end
