#import "QTSSStatusController.h"
#import "QTSSStatusView.h"

@implementation QTSSStatusController

- (void)awakeFromNib
{
    isStarted = NO;
}

- (id)myAdminProtocolObj
{
    return myAdminProtocolObj;
}

- (BOOL)isStarted
{
    return isStarted;
}

- (void)setMyAdminProtocolObj:(id)obj
{
    [obj retain];
    if (myAdminProtocolObj)
        [myAdminProtocolObj release];
    myAdminProtocolObj = obj;
}

- (void)start
{
    [myWindow makeKeyAndOrderFront:self];
    isStarted = YES;
}

- (void)dealloc
{
    if (myAdminProtocolObj)
        [myAdminProtocolObj release];
}

@end
