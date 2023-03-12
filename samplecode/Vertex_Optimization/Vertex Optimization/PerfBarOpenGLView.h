#import <Cocoa/Cocoa.h>

#include <sys/time.h>

@interface PerfBarOpenGLView : NSOpenGLView
{
	float total_time;
	float draw_time;
	float comp_time;
	
	float bar_scale;
	
	int bar_width;
	int bar_height;
	
	int cpu_threaded;
}

- (void)setPerfTimes:(float)tottime:(float)comptime:(float)drawtime:(int)threaded:(float)lag;
- (void)setBarScaleFactor:(float)factor;

@end
