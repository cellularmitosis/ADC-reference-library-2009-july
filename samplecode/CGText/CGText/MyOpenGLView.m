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

#import <Foundation/Foundation.h>
#import <Appkit/AppKit.h>

#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#include <OpenGL/glext.h>

#import "GBuffer.h"
#import "CGText.h"

#import "MyOpenGLView.h"

CGPoint mouse_pos;
int mouse_down_count;

const char* font_names[3] =
{
	"Monaco",
	"Times New Roman",
	"LucidaGrande"
};

@implementation MyOpenGLView

- (void) initView
{
	glShadeModel(GL_SMOOTH);
	glDisable(GL_DITHER);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	glCullFace(GL_BACK);
	glEnable(GL_CULL_FACE);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER,GL_TRUE);
	glLightModeli(GL_LIGHT_MODEL_TWO_SIDE,GL_FALSE);

	{
		GLfloat lightamb[4] = { 0.1, 0.1, 0.1, 1.0 };
		glLightfv(GL_LIGHT0, GL_AMBIENT, lightamb);
	}
	
	{
		GLfloat lightdif[4] = { 1.0, 1.0, 1.0, 1.0 };
		glLightfv(GL_LIGHT0, GL_DIFFUSE, lightdif);
	}
	
	glEnable(GL_LIGHT0);
	glDisable(GL_LIGHTING);

	glEnable(GL_NORMALIZE);

	glEnable(GL_TEXTURE_2D);
	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

	glStencilMask(GL_FALSE);

	glClearColor(0.0, 0.0, 1.0, 0.0);

	{
		GLint alpha_bits;
		GLint stencil_bits;
		GLint depth_bits;
		
		glGetIntegerv(GL_ALPHA_BITS, &alpha_bits);
		glGetIntegerv(GL_STENCIL_BITS, &stencil_bits);
		glGetIntegerv(GL_DEPTH_BITS, &depth_bits);
	
		NSLog(@"Alpha bits  = %d, Stencil bits = %d, Depth bits = %d", alpha_bits, stencil_bits, depth_bits);
	}

	current_font = 0;
	font_size = 12;
	
	[CGText setFont:font_names[current_font] size: font_size debug: debug];
	
	processFunc = nil;
}

- (void) processView
{
}

- (void) renderView: (NSRect) view_rect
{
	glClear(GL_COLOR_BUFFER_BIT);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(view_rect.origin.x, view_rect.origin.x+view_rect.size.width, view_rect.origin.y, view_rect.origin.y+view_rect.size.height);
	glMatrixMode(GL_MODELVIEW);
	
	if (display_message)
		[CGText displayText:[display_message cString] at: CGPointMake(2, view_rect.size.height-font_size)];
	
	glFinish();
}

- (void) awakeFromNib
{
	int i;

	[fontList removeItemAtIndex:0];
	for (i = 0; i < 3; i++)
	{
		[fontList addItemWithTitle: [NSString stringWithCString:font_names[i]]];
		[[[fontList itemArray] objectAtIndex:i] setTag:i];
	}
	[fontList  setEnabled:YES];

	[[NSNotificationCenter defaultCenter] addObserver:self
		selector:@selector(textDidEnd:)
		name:NSControlTextDidEndEditingNotification
		object:textField];
		
	processFunc = @selector(initView);
	
	debug = NO;
}

- (void) updateFont
{
	[CGText setFont:font_names[current_font] size: font_size debug:debug];

	[self setNeedsDisplay:YES];
}

- (void) drawRect: (NSRect) rect
{
	if (processFunc)
		[self performSelector: processFunc];

	[self processView];
	[self renderView:rect];
}

/* Delegate Events */

- (void) mouseDown: (NSEvent*) theEvent
{
	[self mouseDragged: theEvent];
}

- (void) mouseDragged: (NSEvent*) theEvent
{
	NSPoint pos = [self convertPoint: [theEvent locationInWindow] fromView: [[self window] contentView]];
	
	mouse_pos.x = pos.x, mouse_pos.y = pos.y;
	
	mouse_down_count++;

	[self setNeedsDisplay:true];
}

- (void) mouseUp: (NSEvent*) theEvent
{
	mouse_down_count = 0;
}

/* UI Events */

- (IBAction) debug: (id) sender
{
	debug = [sender intValue];
}

- (IBAction) fontSelected:(id)sender
{
	current_font = [[sender selectedItem] tag];

	[self updateFont];
}

- (IBAction) fontSized: (id) sender
{
	font_size = [[sender selectedItem] tag];

	[self updateFont];
}

- (IBAction)setText:(id)sender
{
	[display_message release];
	display_message = [[textField stringValue] retain];
	[self setNeedsDisplay:YES];
}

- (void)textDidEnd:(NSNotification *)notification
{
	[self setText:textField];
}


@end
