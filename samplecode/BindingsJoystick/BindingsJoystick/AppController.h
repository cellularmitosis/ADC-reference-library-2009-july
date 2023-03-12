
#import <Cocoa/Cocoa.h>

@class JoystickView;

@interface AppController : NSObject
{
	IBOutlet JoystickView *joystick;
	IBOutlet NSArrayController *arrayController;
	NSMutableArray *anArray;
}


- (unsigned int)countOfAnArray;
- (id)objectInAnArrayAtIndex:(unsigned int)index;
- (void)insertObject:(id)anObject inAnArrayAtIndex:(unsigned int)index;
- (void)removeObjectFromAnArrayAtIndex:(unsigned int)index;
- (void)replaceObjectInAnArrayAtIndex:(unsigned int)index withObject:(id)anObject;


@end
