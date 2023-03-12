#import "PerfBarOpenGLView.h"

#include <OpenGL/gl.h>
#include <OpenGL/glext.h>
#include <OpenGL/glu.h>

@implementation PerfBarOpenGLView

- (id)initWithFrame:(NSRect)frameRect
{
	NSRect rect;
	NSOpenGLPixelFormatAttribute attrs[] =
	{
        NSOpenGLPFANoRecovery,
        NSOpenGLPFAAccelerated,
        NSOpenGLPFADoubleBuffer,
        0
	};
	
	NSOpenGLPixelFormat* pixFmt = [[NSOpenGLPixelFormat alloc] initWithAttributes:attrs];
	if (!pixFmt)
	{
		NSLog(@"No pixel format -- exiting");
		exit(1);
	}
	
	total_time = 0.0f;
	draw_time  = 0.0f;
	cpu_threaded = false;
	bar_scale = 1.0f;
	
	rect = [self bounds];
	
	bar_width  = (int) rect.size.width;
	bar_height = (int) rect.size.height;
	
	self = [super initWithFrame:frameRect pixelFormat:pixFmt];
	
	[[self openGLContext] makeCurrentContext];
	
	glClearColor(0.8, 0.8, 0.8, 1.0);
	glViewport(0, 0, bar_width, bar_height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(0, bar_width, 0, bar_height);
		
	return self;
}

#define BAR_SCALE  80000.0f

- (void)drawRect:(NSRect)aRect
{
    float appbar, drawbar, compbar;
	 float scale = BAR_SCALE * bar_scale;
	
    [super drawRect:aRect];
	 
    [[self openGLContext] makeCurrentContext];
    
    glClear(GL_COLOR_BUFFER_BIT);
	
	if(cpu_threaded)
	{
		if(total_time < scale)
		{
			appbar  = (float) (bar_height * (total_time - draw_time)) / scale;
			compbar = (float) (bar_height * comp_time) / scale;
			drawbar = appbar + (float) (bar_height * draw_time) / scale;
		}
		else
		{
			appbar  = (float) (bar_height * (total_time - draw_time)) / total_time;
			compbar = (float) (bar_height * comp_time) / total_time;
			drawbar = appbar + (float) (bar_height * draw_time) / total_time;
		}
		
		glColor3f(1.0, 0.0, 0.0);
		glBegin(GL_QUADS);
			glVertex2f(0.0, 0.0);
			glVertex2f((bar_width >> 1), 0.0);
			glVertex2f((bar_width >> 1), appbar);
			glVertex2f(0.0, appbar);
		glEnd();
		
		glColor3f(0.0, 1.0, 0.0);
		glBegin(GL_QUADS);
			glVertex2f((bar_width >> 1), 0);
			glVertex2f(bar_width, 0);
			glVertex2f(bar_width, compbar);
			glVertex2f((bar_width >> 1), compbar);
		glEnd();
		
		glColor3f(0.0, 0.0, 1.0);
		glBegin(GL_QUADS);
			glVertex2f(0.0, appbar);
			glVertex2f((bar_width >> 1), appbar);
			glVertex2f((bar_width >> 1), drawbar);
			glVertex2f(0.0, drawbar);
		glEnd();
	}
	else
	{
		if(total_time < scale)
		{
			appbar  = (float) (bar_height * (total_time - draw_time - comp_time)) / scale;
			compbar = appbar + (float) (bar_height * comp_time) / scale;
			drawbar = compbar + (float) (bar_height * draw_time) / scale;
		}
		else
		{
			appbar  = (float) (bar_height * (total_time - draw_time - comp_time)) / total_time;
			compbar = appbar + (float) (bar_height * comp_time) / total_time;
			drawbar = compbar + (float) (bar_height * draw_time) / total_time;
		}
		
		glColor3f(1.0, 0.0, 0.0);
		glBegin(GL_QUADS);
			glVertex2f(0.0, 0.0);
			glVertex2f(bar_width, 0.0);
			glVertex2f(bar_width, appbar);
			glVertex2f(0.0, appbar);
		glEnd();
		
		glColor3f(0.0, 1.0, 0.0);
		glBegin(GL_QUADS);
			glVertex2f(0.0, appbar);
			glVertex2f(bar_width, appbar);
			glVertex2f(bar_width, compbar);
			glVertex2f(0.0, compbar);
		glEnd();
		
		glColor3f(0.0, 0.0, 1.0);
		glBegin(GL_QUADS);
			glVertex2f(0.0, compbar);
			glVertex2f(bar_width, compbar);
			glVertex2f(bar_width, drawbar);
			glVertex2f(0.0, drawbar);
		glEnd();

	}
	
    [[self openGLContext] flushBuffer];
}

- (void)update  // moved or resized
{
	NSRect rect;
	
	[super update];
	
	[[self openGLContext] makeCurrentContext];
	[[self openGLContext] update];
	
	rect = [self bounds];
	
	bar_width  = (int) rect.size.width;
	bar_height = (int) rect.size.height;
	
	glViewport(0, 0, bar_width, bar_height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(0, bar_width, 0, bar_height);
	
	[self setNeedsDisplay:true];
}

- (void)reshape	// scrolled, moved or resized
{
	NSRect rect;
	
	[super reshape];
	
	[[self openGLContext] makeCurrentContext];
	[[self openGLContext] update];
	
	rect = [self bounds];
	
	bar_width  = (int) rect.size.width;
	bar_height = (int) rect.size.height;
	
	glViewport(0, 0, bar_width, bar_height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(0, bar_width, 0, bar_height);
	
	[self setNeedsDisplay:true];
}

- (void)setPerfTimes:(float)tottime:(float)comptime:(float)drawtime:(int)threaded:(float)lag
{
	total_time = total_time * lag + tottime  * 1000000.0f * (1.0f - lag);
	draw_time  = draw_time  * lag + drawtime * 1000000.0f * (1.0f - lag);
	comp_time  = comp_time  * lag + comptime * 1000000.0f * (1.0f - lag);
	cpu_threaded = threaded;
	
	//NSLog(@"total_time : %f, draw_time : %f, comp_time : %f", total_time, draw_time, comp_time);
	
	[self setNeedsDisplay:true];
}

- (void)setBarScaleFactor:(float)factor
{
	bar_scale = factor;
	
	[self setNeedsDisplay:true];
}


@end
