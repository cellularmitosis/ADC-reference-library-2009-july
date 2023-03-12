#import <Cocoa/Cocoa.h>
#include "newave.h"

#include <OpenGL/gl.h>
#include <OpenGL/glext.h>
#include <OpenGL/glu.h>

@interface MainOpenGLView : NSOpenGLView
{
	GLdouble tottime;
	GLint passes;
	
	struct timeval cycle_time;
	struct timeval display_time;
	
	WaveOject  *wave;
	
	IBOutlet NSTextField *setFPS;
	IBOutlet NSTextField *setTriRate;
	
	NSTimer* timer;
}

@end
