#import "MainOpenGLView.h"

#include <sys/time.h>
#include <unistd.h>

#define VERTEX_COUNT     256
#define ITERATIONS       10

#define APP_OPT_MAX      20000

#define NO_DATA          0
#define HAS_DATA         1

@interface MainOpenGLView (Helper)
- (void)DrawThread;
@end

@implementation MainOpenGLView (Helper)
- (void)DrawThread
{
	struct timeval t1, t2;
	GLuint count = 0;
	GLdouble ut;
	GLint qitem;
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
	
	[NSThread setThreadPriority:1.0];
	
	[[self openGLContext] makeCurrentContext];
	
	gettimeofday(&t1, NULL);

	do
	{
		[buffer_lock[COMPUTATION_QUEUE] lockWhenCondition:HAS_DATA];
		qitem = work_queue[COMPUTATION_QUEUE][0];
		
		//[buffer_lock[DISPLAY_QUEUE] lock];
		//glFinishFenceAPPLE(qitem + 1);
		//[buffer_lock[DISPLAY_QUEUE] unlock];
		
		[wave WaveMotion:qitem];
		work_queue[COMPUTATION_QUEUE][0] = work_queue[COMPUTATION_QUEUE][1];
		work_queue[COMPUTATION_QUEUE][1] = -1;
		[buffer_lock[COMPUTATION_QUEUE] unlockWithCondition:(work_queue[COMPUTATION_QUEUE][0] < 0 ? NO_DATA : HAS_DATA)];
		
		[buffer_lock[DISPLAY_QUEUE] lock];
		if(work_queue[DISPLAY_QUEUE][0] < 0)
			work_queue[DISPLAY_QUEUE][0] = qitem;
		else
			work_queue[DISPLAY_QUEUE][1] = qitem;
		[buffer_lock[DISPLAY_QUEUE] unlockWithCondition:HAS_DATA];
		
		gettimeofday(&t2, NULL);
		count++;
		ut = (GLdouble) (t2.tv_sec - t1.tv_sec) + (GLdouble) (t2.tv_usec - t1.tv_usec) * 0.000001;
		if(ut > 0.5)
		{
			comptime = ut / (GLdouble) count;
			gettimeofday(&t1, NULL);
			
			count = 0;
		}
	} while(multi_threaded);
	
	[pool release];
	
	[NSThread exit];
}
@end

@implementation MainOpenGLView

- (IBAction)setAppOptLevelAction:(id)sender
{
    int app_time = -([sender intValue] - APP_OPT_MAX);
    
    appOptLevel = app_time;
    
    [self setNeedsDisplay:true];
}

- (void) UpdateLabels
{
    NSTextField *textField = NULL;
    char		labelString[64];
    char		*labels[] = {	  "Quads",
									  "Quad Strips",
                             "Vertex Arrays",
                             "Vertex Array Range",
                             "Vertex Array Range + Altivec",
                             "Vertex Program"};
    int 	i;
    
    for(i=0; i<6; i++)
    {
        if (i == 0) textField = optLevelLabel_0;
        if (i == 1) textField = optLevelLabel_1;
        if (i == 2) textField = optLevelLabel_2;
        if (i == 3) textField = optLevelLabel_3;
        if (i == 4) textField = optLevelLabel_4;
        if (i == 5) textField = optLevelLabel_5;

        if (i == openglOptLevel)
        {
            sprintf(labelString, labels[i]);

            [textField setBezeled: YES];

            [textField setStringValue: [NSString stringWithCString: labelString]];
				
				[textField setTextColor: [NSColor redColor]];
        }
        else
        {
            labelString[0] = 0;
            
            [textField setBezeled: NO];

            [textField setStringValue: [NSString stringWithCString: labelString]];
        }
    }

    [self setNeedsDisplay:true];
}

- (IBAction)setOptLevelAction: (id)sender
{                                      
    openglOptLevel = [sender intValue];

    [[self openGLContext] makeCurrentContext];

	if (5 == openglOptLevel && multi_threaded)
	{
		[self setMPAction: 0];
	}
	
    [wave WaveSetDisplay: openglOptLevel];

    [self UpdateLabels];

    [self setNeedsDisplay:true];
}

- (IBAction)setMPAction:(id)sender
{
    int mp = [sender intValue];
	
	if(mp)
	{
		if (openglOptLevel != 5)
		{
			if(!multi_threaded)
			{
				multi_threaded = true;
				[NSThread detachNewThreadSelector:@selector(DrawThread) toTarget:self withObject:nil];
			}
		}
		else
		{
			[selectMP setState: 0];
		}
	}
	else
	{
		multi_threaded = false;
		[selectMP setState: 0];
	}
    
    [self setNeedsDisplay:true];
}

- (IBAction)setBarScaleFactor:(id)sender
{
    float factor;
	 int setting = [sender intValue];
	 
	 if(setting) factor = 0.1f;
	 else        factor = 1.0f;
	 
	 [perfBarGLView setBarScaleFactor:factor];
}

- (void) awakeFromNib
{
        [self UpdateLabels];
}

- (id)initWithFrame:(NSRect)frameRect
{
	GLint qitem;
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
	
	buffer_lock[COMPUTATION_QUEUE] = [[NSConditionLock alloc] initWithCondition:HAS_DATA];
	buffer_lock[DISPLAY_QUEUE]     = [[NSConditionLock alloc] initWithCondition:NO_DATA];
	
	work_queue[COMPUTATION_QUEUE][0] = 0;
	work_queue[COMPUTATION_QUEUE][1] = 1;
	work_queue[DISPLAY_QUEUE][0] = -1;
	work_queue[DISPLAY_QUEUE][1] = -1;
	
	appOptLevel    = APP_OPT_MAX;
	openglOptLevel = 0;
	multi_threaded = false;
	fillmode	   = 1;
	
	tottime	= 0;
	comptime	= 0;
	ogltime 	= 0;
	passes   = 0;
	prev_tottime = 0;
	 
	gettimeofday(&cycle_time, NULL);
	gettimeofday(&display_time, NULL);
	
	self = [super initWithFrame:frameRect pixelFormat: pixFmt];
	
	[[self openGLContext] makeCurrentContext];
	
	wave = [[WaveOject  alloc] init];
	
	// Compute the initial buffer
	qitem = work_queue[COMPUTATION_QUEUE][0];
	[wave WaveMotion: qitem];
	work_queue[COMPUTATION_QUEUE][0] = work_queue[COMPUTATION_QUEUE][1];
	work_queue[COMPUTATION_QUEUE][1] = -1;
	if(work_queue[DISPLAY_QUEUE][0] < 0)
		work_queue[DISPLAY_QUEUE][0] = qitem;
	else
		work_queue[DISPLAY_QUEUE][1] = qitem;
	
	[self UpdateLabels];
	
	[self setNeedsDisplay:true];
	
	return self;
}

- (void)drawRect:(NSRect)aRect
{
	struct timeval t1, t2, ct;
	GLuint r, tris = 0;
	GLint qitem;
		
	[[self openGLContext] makeCurrentContext];
	
	// Display the current buffer
	if(multi_threaded)
	{
		gettimeofday(&t1, NULL);
		
		for(r = 0; r < ((openglOptLevel + 1) * 2); r++, passes++)
		{
			[buffer_lock[DISPLAY_QUEUE] lockWhenCondition:HAS_DATA];
			
			qitem = work_queue[DISPLAY_QUEUE][0];
			if(qitem >= 0)
			{
				tris = [wave WaveDisplay: qitem];
				glSetFenceAPPLE(qitem + 1);
				work_queue[DISPLAY_QUEUE][0] = work_queue[DISPLAY_QUEUE][1];
				work_queue[DISPLAY_QUEUE][1] = -1;
			}
			[buffer_lock[DISPLAY_QUEUE] unlockWithCondition:(work_queue[DISPLAY_QUEUE][0] < 0 ? NO_DATA : HAS_DATA)];
				
			[buffer_lock[COMPUTATION_QUEUE] lock];
			if(work_queue[COMPUTATION_QUEUE][0] < 0)
				work_queue[COMPUTATION_QUEUE][0] = qitem;
			else
				work_queue[COMPUTATION_QUEUE][1] = qitem;
				
			[buffer_lock[COMPUTATION_QUEUE] unlockWithCondition:HAS_DATA];
			
			[[self openGLContext] flushBuffer];
		}
		
		gettimeofday(&t2, NULL);
		ogltime += (GLdouble) (t2.tv_sec - t1.tv_sec) + (GLdouble) (t2.tv_usec - t1.tv_usec) * 0.000001;
	}
	else
	{
		for(r = 0; r < ((openglOptLevel + 1) * 2); r++, passes++)
		{
			gettimeofday(&t1, NULL);
		
			// Compute the next buffer in queue
			qitem = work_queue[COMPUTATION_QUEUE][0];
			if(qitem >= 0)
			{
				[wave WaveMotion: qitem];
				
				work_queue[COMPUTATION_QUEUE][0] = work_queue[COMPUTATION_QUEUE][1];
				work_queue[COMPUTATION_QUEUE][1] = -1;
				
				if(work_queue[DISPLAY_QUEUE][0] < 0)
					work_queue[DISPLAY_QUEUE][0] = qitem;
				else
					work_queue[DISPLAY_QUEUE][1] = qitem;
			}
			
			gettimeofday(&ct, NULL);
			comptime += (GLdouble) (ct.tv_sec - t1.tv_sec) + (GLdouble) (ct.tv_usec - t1.tv_usec) * 0.000001;
			
			[[self openGLContext] makeCurrentContext];
			
			qitem = work_queue[DISPLAY_QUEUE][0];
			if(qitem >= 0)
			{
				tris = [wave WaveDisplay: qitem];
	
				work_queue[DISPLAY_QUEUE][0] = work_queue[DISPLAY_QUEUE][1];
				work_queue[DISPLAY_QUEUE][1] = -1;
				
				if(work_queue[COMPUTATION_QUEUE][0] < 0)
					work_queue[COMPUTATION_QUEUE][0] = qitem;
				else
					work_queue[COMPUTATION_QUEUE][1] = qitem;
			}
			
			[[self openGLContext] flushBuffer];
		
			gettimeofday(&t2, NULL);
			ogltime += (GLdouble) (t2.tv_sec - ct.tv_sec) + (GLdouble) (t2.tv_usec - ct.tv_usec) * 0.000001;
		}
	}
	
	gettimeofday(&cycle_time, NULL);
	
	tottime = (GLdouble) (cycle_time.tv_sec - display_time.tv_sec) + (GLdouble) (cycle_time.tv_usec - display_time.tv_usec) * 0.000001;
	if(tottime > 0.5)
	{
		GLdouble lag_time;
		
		gettimeofday(&display_time, NULL);
		
		tottime  /= (GLdouble) passes;
		if(!multi_threaded)
			comptime	/= (GLdouble) passes;
		ogltime 	/= (GLdouble) passes;
		
		lag_time = prev_tottime * 0.1 + tottime * 0.9;
		prev_tottime = tottime;
	
		[setFPS setFloatValue:1.0 / lag_time];
		[setTriRate setFloatValue:(GLdouble) tris / lag_time];
		
		[perfBarGLView setPerfTimes:tottime:comptime:ogltime:multi_threaded:0.1];
		
		tottime	= 0.0;
		if(!multi_threaded)
			comptime	= 0.0;
		ogltime 	= 0.0;
		passes   = 0;
	}
}

- (void)update  // moved or resized
{
	NSRect rect;
	
	[super update];
	
	[buffer_lock[DISPLAY_QUEUE] lock];
	
	[[self openGLContext] makeCurrentContext];
	[[self openGLContext] update];
	
	rect = [self bounds];
	
	[wave WaveReshape:(int) rect.size.width:(int) rect.size.height];
	
	[buffer_lock[DISPLAY_QUEUE] unlock];
	
	[self setNeedsDisplay:true];
}

- (void)reshape	// scrolled, moved or resized
{
	NSRect rect;
	
	[super reshape];
	
	[buffer_lock[DISPLAY_QUEUE] lock];
	
	[[self openGLContext] makeCurrentContext];
	[[self openGLContext] update];
	
	rect = [self bounds];
	
	[wave WaveReshape:(int) rect.size.width:(int) rect.size.height];
	
	[buffer_lock[DISPLAY_QUEUE] unlock];
	
	[self setNeedsDisplay:true];
}

- (void)mouseDragged:(NSEvent *)theEvent
{
	NSPoint pt;
	
	[buffer_lock[DISPLAY_QUEUE] lock];
	
	[[self openGLContext] makeCurrentContext];
	
	pt = [theEvent locationInWindow];
	
	[wave WaveMouseMotion:(int) pt.x:(int) pt.y];
	
	[buffer_lock[DISPLAY_QUEUE] unlock];
	
	[self setNeedsDisplay:true];
}

- (void)mouseDown:(NSEvent *)theEvent
{
	NSPoint pt;
	
	[buffer_lock[DISPLAY_QUEUE] lock];
	
	[[self openGLContext] makeCurrentContext];
	
	pt = [theEvent locationInWindow];
	
	[wave WaveMouseDown:(int) pt.x:(int) pt.y];
	
	[buffer_lock[DISPLAY_QUEUE] unlock];
	
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

- (void)keyDown:(NSEvent *)theEvent
{
	unichar		c = [theEvent keyCode];
	
	if ([theEvent isARepeat])
	{
		return;
	}
	
	switch (c)
	{
		case 0x31:
		fillmode = fillmode ? 0 : 1;
		[wave WaveSetFillMode: fillmode];
		break;

		default:
		[super keyDown: theEvent];
	}
}

@end
