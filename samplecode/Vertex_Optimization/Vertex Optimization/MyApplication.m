#import "MyApplication.h"
#import "AppController.h"

@implementation MyApplication

#if 1

- (NSEvent *)nextEventMatchingMask:(unsigned int)mask
	untilDate:(NSDate *)expiration inMode:(NSString *)mode dequeue:(BOOL)flag
{
	NSEvent *event;
	NSDate *now;
	int send_event;
	
	do
	{
		event = [super nextEventMatchingMask:mask untilDate:[NSDate distantPast] inMode:mode dequeue:flag];
		
		/*if(event == nil)*/ [[self delegate] UpdateDrawing];
		
		now = [[NSDate alloc] initWithTimeIntervalSinceNow:0];
		
		if(!event && ([expiration compare:now] > 0)) send_event = false;
		else                                         send_event = true;
		
		[now release];
		
	} while(!send_event);
	
	return event;
}

#else

- (NSEvent *)nextEventMatchingMask:(unsigned int)mask
	untilDate:(NSDate *)expiration inMode:(NSString *)mode dequeue:(BOOL)flag
{
	NSEvent *event;
	
	event = [super nextEventMatchingMask:mask untilDate:expiration inMode:mode dequeue:flag];
		
	[[self delegate] UpdateDrawing];
	
	return event;
}

#endif

@end
