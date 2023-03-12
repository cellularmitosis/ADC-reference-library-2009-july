#import "MainOpenGLView.h"

#include <sys/time.h>
#include <unistd.h>

#define VERTEX_COUNT     256
#define ITERATIONS       10

#define NO_DATA          0
#define HAS_DATA         1

@implementation MainOpenGLView

- (void) awakeFromNib
{
}

- (id)initWithFrame:(NSRect)frameRect
{
	NSBundle *mainBndl;
	NSString *bndlPath;
	
	NSOpenGLPixelFormatAttribute attrs[] =
	{
		NSOpenGLPFANoRecovery,
		NSOpenGLPFAAccelerated,
		NSOpenGLPFADoubleBuffer,
		NSOpenGLPFADepthSize, 16,
		0
	};
	
	NSOpenGLPixelFormat* pixFmt = [[NSOpenGLPixelFormat alloc] initWithAttributes:attrs];
	if(!pixFmt)
	{
		NSLog(@"No pixel format -- exiting");
		exit(1);
	}
	
	mainBndl = [NSBundle mainBundle];
	bndlPath = [mainBndl resourcePath];
	chdir([bndlPath cString]);
	
	timer = [[NSTimer scheduledTimerWithTimeInterval: (1.0f / 140.0f) target: self selector:@selector(drawRect:) userInfo:self repeats:true] retain];	
	tottime	= 0;
	passes   = 0;
	 
	gettimeofday(&cycle_time, NULL);
	gettimeofday(&display_time, NULL);
	
	self = [super initWithFrame:frameRect pixelFormat: pixFmt];
	
	[[self openGLContext] makeCurrentContext];
	
	wave = [[WaveOject  alloc] init];
	
	// Compute the initial buffer
	[wave WaveMotion: 0];
	[wave WaveMotion: 1];
	
	[self setNeedsDisplay:true];
	
	return self;
}

- (void)drawRect:(NSRect)aRect
{
	GLuint tris = 0;
	static GLint buffer = 0;
		
	// Compute the next buffer in queue
	[[self openGLContext] makeCurrentContext];
	[wave WaveMotion: buffer];
	tris = [wave WaveDisplay: buffer];
	[[self openGLContext] flushBuffer];
	
	if(buffer) buffer = 0;
	else       buffer = 1;
	
	passes++;
	
	gettimeofday(&cycle_time, NULL);
	tottime = (GLdouble) (cycle_time.tv_sec - display_time.tv_sec) + (GLdouble) (cycle_time.tv_usec - display_time.tv_usec) * 0.000001;
	
	if(tottime > 0.5)
	{
		gettimeofday(&display_time, NULL);
		
		tottime  /= (GLdouble) passes;
	
		[setFPS setFloatValue:(GLdouble) 1.0 / (GLdouble) tottime];
		[setTriRate setFloatValue:(GLdouble) tris / (GLdouble) tottime];
		
		tottime	 = 0.0;
		passes   = 0;
	}
}

- (void)update  // moved or resized
{
	NSRect rect;
	
	[super update];
	
	[[self openGLContext] makeCurrentContext];
	[[self openGLContext] update];
	
	rect = [self bounds];
	
	[wave WaveReshape:(int) rect.size.width:(int) rect.size.height];
	
	[self setNeedsDisplay:true];
}

- (void)reshape	// scrolled, moved or resized
{
	NSRect rect;
	
	[super reshape];
	
	[[self openGLContext] makeCurrentContext];
	[[self openGLContext] update];
	
	rect = [self bounds];
	
	[wave WaveReshape:(int) rect.size.width:(int) rect.size.height];
	
	[self setNeedsDisplay:true];
}

- (void)mouseDragged:(NSEvent *)theEvent
{
	NSPoint pt;
	
	[[self openGLContext] makeCurrentContext];
	
	pt = [theEvent locationInWindow];
	
	[wave WaveMouseMotion:(int) pt.x:(int) pt.y];
	
	[self setNeedsDisplay:true];
}

- (void)mouseDown:(NSEvent *)theEvent
{
	NSPoint pt;
	
	[[self openGLContext] makeCurrentContext];
	
	pt = [theEvent locationInWindow];
	
	[wave WaveMouseDown:(int) pt.x:(int) pt.y];
	
	[self setNeedsDisplay:true];
}

- (BOOL)acceptsFirstResponder
{
   return YES;
}

- (BOOL)becomeFirstResponder
{
   return  YES;
}

- (BOOL)resignFirstResponder
{
   return YES;
}

@end
