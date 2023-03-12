/* QTSSStatusView */

#import <Cocoa/Cocoa.h>

@interface QTSSStatusView : NSView
{
    IBOutlet NSTextField *myCountField;
    IBOutlet NSTextField *myMaxField;
    NSMutableArray *myConnectionValues;
    NSTimer *myTimer;
    NSColor *myColor;
    id myStatusController;
}

- (NSColor *)myColor;
- (void)setMyColor:(NSColor *)color;
- (void)timerFired;

@end
