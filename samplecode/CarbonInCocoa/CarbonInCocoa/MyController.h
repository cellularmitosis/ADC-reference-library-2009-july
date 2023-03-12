#import <Cocoa/Cocoa.h>
#include <Carbon/Carbon.h>

@interface MyController : NSObject
{
    WindowRef   window;
    NSWindow *cocoaFromCarbonWin;
}
@end
