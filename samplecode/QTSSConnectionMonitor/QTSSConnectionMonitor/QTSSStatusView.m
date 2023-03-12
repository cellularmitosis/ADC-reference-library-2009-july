#import "QTSSStatusView.h"
#import "QTSSStatusController.h"
#import "AdminProtocolAccessObj.h"

@implementation QTSSStatusView

- (void)awakeFromNib
{    
    NSTimeInterval interval = .5;

    myConnectionValues = [[NSMutableArray array] retain];
    myTimer = [NSTimer scheduledTimerWithTimeInterval:interval
                                               target:self
                                             selector:@selector(timerFired)
                                             userInfo:nil
                                             repeats:YES];
    [myTimer retain];
}

- (NSColor *)myColor
{
    if (!myColor)
        myColor = [[NSColor greenColor] retain];
    return myColor;
}

- (void)setMyColor:(NSColor *)color
{
    [color retain];
    if (myColor)
        [myColor release];
    myColor = color;
}

- (void)timerFired
{
    AdminProtocolAccessObj *adminProtocolAccessObj;
    NSMutableArray *result = [NSMutableArray array];
    float rtspThroughput;
    float mp3Throughput;

    // only do something if the user has logged in
    if ([myStatusController isStarted]) {
        adminProtocolAccessObj = [myStatusController myAdminProtocolObj];

        // get the maximum connections
        [adminProtocolAccessObj getValue:@"/modules/admin/server/qtssSvrPreferences/maximum_bandwidth" withResult:result];
        [myMaxField setStringValue:[result objectAtIndex:0]];

        // get the RTSP bandwidth
        [adminProtocolAccessObj getValue:@"/modules/admin/server/qtssRTPSvrCurBandwidth" withResult:result];
        rtspThroughput = [[result objectAtIndex:0] floatValue];

        // get the mp3 bandwidth
        [adminProtocolAccessObj getValue:@"/modules/admin/server/qtssMP3SvrCurBandwidth" withResult:result];
        mp3Throughput = [[result objectAtIndex:0] floatValue];

        // total the two and turn it into kilobits
        [myCountField setIntValue:((rtspThroughput + mp3Throughput) / 1024)];

        // add the bandwidth to the history array
        [myConnectionValues addObject:[NSNumber numberWithFloat:[myCountField floatValue]]];
        [self display];
    }
}

- (void)drawRect:(NSRect)aRect
{
    NSRect bounds = [self bounds];
    int i;
    int barHeight;
    NSRect connectionsRect;

    [[NSColor blackColor] set];
    [NSBezierPath fillRect:bounds];

    connectionsRect.size.width = 1;
    connectionsRect.origin.x = bounds.size.width - 1;
    connectionsRect.origin.y = 0;

    for (i = [myConnectionValues count] - 1; i >= 0; i--) {
        barHeight = (bounds.size.height * [[myConnectionValues objectAtIndex:i] floatValue]) / [myMaxField intValue];
        connectionsRect.size.height = barHeight;
        [[self myColor] set];
        [NSBezierPath fillRect:connectionsRect];
        connectionsRect.origin.x -= 2;
        if (connectionsRect.origin.x < 0)
            i = (-1);
    }
}

- (void)dealloc
{
    [myConnectionValues release];
    [myTimer release];        
    if (myColor)
        [myColor release];
}

@end
