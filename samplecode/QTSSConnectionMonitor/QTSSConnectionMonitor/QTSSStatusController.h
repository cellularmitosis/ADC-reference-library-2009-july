/* QTSSStatusController */

#import <Cocoa/Cocoa.h>

@interface QTSSStatusController : NSObject
{
    id myAdminProtocolObj;
    IBOutlet NSWindow *myWindow;
    BOOL isStarted;
}

- (id)myAdminProtocolObj;
- (BOOL)isStarted;
- (void)setMyAdminProtocolObj:(id)obj;
- (void)start;

@end
