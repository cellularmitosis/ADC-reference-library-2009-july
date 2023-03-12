/*
 ClockControl.m
 Implements the clock control, and it cell.  Shows an example of creating a custom control and cell.
 
 Author: CP

 Copyright (c) 2002, Apple Computer, Inc., all rights reserved.
*/

/*
 IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc. ("Apple") in
 consideration of your agreement to the following terms, and your use, installation, 
 modification or redistribution of this Apple software constitutes acceptance of these 
 terms.  If you do not agree with these terms, please do not use, install, modify or 
 redistribute this Apple software.
 
 In consideration of your agreement to abide by the following terms, and subject to these 
 terms, Apple grants you a personal, non-exclusive license, under AppleÕs copyrights in 
 this original Apple software (the "Apple Software"), to use, reproduce, modify and 
 redistribute the Apple Software, with or without modifications, in source and/or binary 
 forms; provided that if you redistribute the Apple Software in its entirety and without 
 modifications, you must retain this notice and the following text and disclaimers in all 
 such redistributions of the Apple Software.  Neither the name, trademarks, service marks 
 or logos of Apple Computer, Inc. may be used to endorse or promote products derived from 
 the Apple Software without specific prior written permission from Apple. Except as expressly
 stated in this notice, no other rights or licenses, express or implied, are granted by Apple
 herein, including but not limited to any patent rights that may be infringed by your 
 derivative works or by other works in which the Apple Software may be incorporated.
 
 The Apple Software is provided by Apple on an "AS IS" basis.  APPLE MAKES NO WARRANTIES, 
 EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED WARRANTIES OF NON-INFRINGEMENT, 
 MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS 
 USE AND OPERATION ALONE OR IN COMBINATION WITH YOUR PRODUCTS.
 
 IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL OR CONSEQUENTIAL 
 DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS 
 OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) ARISING IN ANY WAY OUT OF THE USE, 
 REPRODUCTION, MODIFICATION AND/OR DISTRIBUTION OF THE APPLE SOFTWARE, HOWEVER CAUSED AND 
 WHETHER UNDER THEORY OF CONTRACT, TORT (INCLUDING NEGLIGENCE), STRICT LIABILITY OR 
 OTHERWISE, EVEN IF APPLE HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/


#import "ClockControl.h"
#import "MathUtils.h"                   /* for AngleFromNorth() */

#define kMClockHandWidth	2.0	/* Line width of the minute hand   */
#define kHClockHandWidth 	3.0	/* Line width of the hour hand     */
#define kMClockHandWhite	0.0     /* Color of the minute hand        */
#define kHClockHandWhite 	0.0     /* Color of the hour hand          */
#define kMClockHandAlpha	0.7     /* Alpha component for minute hand */
#define kHClockHandAlpha 	0.5	/* Alpha component for hour hand   */

#define HOUR_MINUTE_MERIDIEM_FORMAT  	@"%I:%M %p"	/* Hour : Minute  Meridiem */
#define MERIDIEM_FORMAT			@"%p"           /* Meridiem */


@implementation ClockCell 

// ---------------------------------------------------------
//  Initialization
// ---------------------------------------------------------

- (id)init {
    self = [super init];
    if (self) {
	// Initialize the time to current, we assume throughout that time is non-nil.
	[self setTime: [NSCalendarDate date]];
    }
    return self;
}

- (id)initWithCoder:(NSCoder *)coder {
    self = [super initWithCoder:coder];
    if ([coder allowsKeyedCoding]) {
	time = [[coder decodeObjectForKey: @"time"] retain];
    } else {
	time = [[coder decodeObject] retain];
    }
    return self;
}

- (void)encodeWithCoder:(NSCoder *)coder {
    [super encodeWithCoder:coder];
    if ([coder allowsKeyedCoding]) {
	[coder encodeObject: [self time] forKey: @"time"];
    } else {
	[coder encodeObject: [self time]];
    }
}

- (id)copyWithZone:(NSZone *)zone {
    ClockCell *newCopy = [[ClockCell alloc] init];
    [newCopy setTime: [self time]];
    return newCopy;
}

- (void)dealloc {
    [time release];
    time = nil;
    [super dealloc];
}

- (void)sendActionToTarget {
    if ([self target] && [self action]) {
        [(NSControl *)[self controlView] sendAction:[self action] to:[self target]];
    }
}

// ---------------------------------------------------------
//  Setting time (conveniences)
// ---------------------------------------------------------

- (void)setTimeToNow:(id)sender {
    [self setTime:[NSCalendarDate date]];
}

- (void)incrementHour:(int)hour andMinute:(int)minute {
    NSCalendarDate *newTime = [[self time] dateByAddingYears:0 months:0 days:0 hours:hour minutes:minute seconds:0];
    [self setTime:newTime];
}

- (void)setHour:(int)hour andMinute:(int)minute {
    NSCalendarDate *currentTime = [self time];
    int minuteInc = (minute>=0 ? minute-[currentTime minuteOfHour] : 0);
    int hourInc = (hour>=0 ? hour-[currentTime hourOfDay] : 0);
    [self incrementHour: hourInc andMinute:minuteInc];
}

- (void)setHourHandByAngleFromNorth:(float)angle {
    int hour = ((angle) * (12.0 / 360.0));
    float hourAngle = (float)hour * (360.0 / 12.0);
    int minute = 60.0*((angle-hourAngle) / (360.0 / 12.0));
    
    // Preserve the AM/PM setting.  
    hour = ([[self time] hourOfDay]>11 ? hour + 12 : hour);
    [self setHour:hour andMinute: minute];
}

// ---------------------------------------------------------
//  Setting and getting values
// ---------------------------------------------------------

- (void)setTime:(NSCalendarDate *)newTime {
    if (newTime && ![time isEqual:newTime]) {
	// Change the time!
	[time release];
	time = [newTime retain];
	
	// Tell our control view to redisplay us.
	[(NSControl *)[self controlView] updateCell: self];
	
	// For this example, we just send the action whenever the time changes.
	// Usually you would only want to send an action in response to user events.
	[self sendActionToTarget];
    }
}

- (NSCalendarDate *)time { 
    return time;
}

- (void)setObjectValue:(NSObject *)object {
    // We understand how to convert from NSCalendarDates, and NSStrings anything else is unexpected.
    if ([object isKindOfClass: [NSCalendarDate class]]) {
	[self setTime: (NSCalendarDate *)object];
    } else if ([object isKindOfClass: [NSString class]]) {
	[self setStringValue: (NSString *)object];
    } else {
        [NSException raise: NSInvalidArgumentException format: @"%@ Invalid object %@", NSStringFromSelector(_cmd), object];
    }
}

- (id)objectValue {
    return time;
}

- (void)setStringValue:(NSString *)stringValue {
    // Allow setting the time with a string, as long as it is in the format we are willing to accept.
    [self setTime: [[[NSCalendarDate alloc] initWithString: stringValue calendarFormat: HOUR_MINUTE_MERIDIEM_FORMAT] autorelease]];
}

- (NSString *)stringValue {
    return [[self time] descriptionWithCalendarFormat: HOUR_MINUTE_MERIDIEM_FORMAT];
}

// ---------------------------------------------------------
//  Target / action methods
// ---------------------------------------------------------

- (IBAction)takeMinuteValueFrom:(id)sender {
    [self setHour: -1 andMinute:[sender intValue]];
}

- (IBAction)takeHourValueFrom:(id)sender {
    [self setHour: [sender intValue] andMinute:-1];
}

- (IBAction)toggleAmPm:(id)sender {
    // Toggle am/pm by changing the time by 12 hours.  This is fine since we don't care about the day.
    [self incrementHour:12 andMinute: 0];
}

// ---------------------------------------------------------
//  Drawing Routines
// ---------------------------------------------------------

- (void)drawClockHandsForTime:(NSCalendarDate *)theTime withFrame:(NSRect)cellFrame inView:(NSView *)controlView {
    float mHandRadius, mHandTheta;
    float hHandRadius, hHandTheta;
    float clockRadius, innerRadius;
    NSPoint centerPoint;
    NSPoint direction, p1, p2;
    NSRect clockRect;
    NSString *meridiemString = nil;
    NSMutableDictionary *stringAttributes = nil;
    
    // Indicate nil time, by not drawing any hands.
    if (!theTime) return;
    
    // Compute where the clock lives in the cellFrame.
    clockRadius = MIN(NSWidth(cellFrame), NSHeight(cellFrame)) / 2.0;
    clockRect   = NSMakeRect(NSMinX(cellFrame), NSMinY(cellFrame), 2.0 * clockRadius, 2.0 * clockRadius);

    // If we have focus, draw a focus ring around the entire cellFrame (inset it a little so it looks nice).
    if ([self showsFirstResponder]) {
	// showsFirstResponder is set for us by the NSControl that is drawing us.
        NSRect focusRingFrame = clockRect;
	focusRingFrame.size.height -= 2.0;
	[NSGraphicsContext saveGraphicsState];
	NSSetFocusRingStyle(NSFocusRingOnly);
	[[NSBezierPath bezierPathWithRect: NSInsetRect(focusRingFrame,4,4)] fill];
	[NSGraphicsContext restoreGraphicsState];
    }
    
    // Determine a few values to help us figure out where to draw the hands.
    mHandRadius = clockRadius * (8.0/10.0);
    hHandRadius = clockRadius * (6.0/10.0);
    innerRadius = clockRadius *-(1.0/10.0);
    mHandTheta  = ToRad( 90.0 - 360.0 * ((float)[theTime minuteOfHour] / 60.0) );
    hHandTheta  = ToRad( 90.0 - (360.0 * ((float)[theTime hourOfDay] / 12.0) + (360.0 / 12.0) * ((float)[theTime minuteOfHour] / 60.0)));
    centerPoint = NSMakePoint(floor(NSMidX(clockRect) + .5), floor(NSMidY(clockRect) + .5));
    
    // Draw the minute hand.
    direction  = NSMakePoint(cos(mHandTheta), sin(mHandTheta));
    if ([controlView isFlipped]) direction.y  = -direction.y;
    p1 = NSMakePoint(centerPoint.x + innerRadius * direction.x, centerPoint.y + innerRadius * direction.y);
    p2 = NSMakePoint(centerPoint.x + mHandRadius * direction.x, centerPoint.y + mHandRadius * direction.y);
    [[NSColor colorWithCalibratedWhite:kMClockHandWhite alpha:kMClockHandAlpha] set];
    [NSBezierPath setDefaultLineWidth:kMClockHandWidth];
    [NSBezierPath strokeLineFromPoint:p1 toPoint:p2];

    // Draw the hour hand.
    direction  = NSMakePoint(cos(hHandTheta), sin(hHandTheta));
    if ([controlView isFlipped]) direction.y  = -direction.y;
    p1 = NSMakePoint(centerPoint.x + innerRadius * direction.x, centerPoint.y + innerRadius * direction.y);
    p2 = NSMakePoint(centerPoint.x + hHandRadius * direction.x, centerPoint.y + hHandRadius * direction.y); 
    [[NSColor colorWithCalibratedWhite:kHClockHandWhite alpha:kHClockHandAlpha] set];
    [NSBezierPath setDefaultLineWidth:kHClockHandWidth];
    [NSBezierPath strokeLineFromPoint:p1 toPoint:p2];
    
    // Draw the AM/PM indication.
    meridiemString = [theTime descriptionWithCalendarFormat: MERIDIEM_FORMAT];
    stringAttributes = [NSMutableDictionary dictionaryWithCapacity:2];
    [stringAttributes setObject:[NSFont fontWithName:@"Times-Roman" size:12.0] forKey:NSFontAttributeName];
    [stringAttributes setObject:[NSColor grayColor] forKey:NSForegroundColorAttributeName];
    [meridiemString drawAtPoint:NSMakePoint(NSMinX(cellFrame)+6,([controlView isFlipped] ? NSMaxY(cellFrame) - 15 : NSMinY(cellFrame)+4)) withAttributes: stringAttributes]; 
    
    // Be nice and reset the line width to something reasonable.
    [NSBezierPath setDefaultLineWidth: 0.0];
}

- (void)drawWithFrame:(NSRect)cellFrame inView:(NSView *)controlView {
    NSImage *clockFaceImage = [NSImage imageNamed: @"ClockFace"];
    float clockRadius = MIN(NSHeight(cellFrame), NSWidth(cellFrame));
    
    // Draw the clock face (draw it flipped if we are in a flipped view, like NSMatrix).
    [clockFaceImage setFlipped:[controlView isFlipped]];
    [clockFaceImage drawInRect:NSMakeRect(NSMinX(cellFrame), NSMinY(cellFrame),clockRadius,clockRadius) fromRect:NSMakeRect(0,0,[clockFaceImage size].width, [clockFaceImage size].height) operation:NSCompositeSourceOver fraction:1.0];
    
    // Draw the clock hour and minute hands.
    [self drawClockHandsForTime:time withFrame:cellFrame inView:controlView];
}

// ---------------------------------------------------------
//  Mouse Tracking
// ---------------------------------------------------------

static inline BOOL PointIsInClockFace(NSPoint locationInCellFrame, NSRect cellFrame) {
    BOOL hit = NO;
    if ( NSPointInRect(locationInCellFrame, cellFrame) ) {
	float radius = (MIN(NSHeight(cellFrame), NSWidth(cellFrame)) / 2.0);
	NSPoint centerPoint = NSMakePoint(floor(NSMinX(cellFrame)+radius+.5), floor(NSMinY(cellFrame)+radius+.5));
	float sqDistanceToCenter = SQR(locationInCellFrame.x - centerPoint.x) + SQR(locationInCellFrame.y - centerPoint.y);
	hit = sqDistanceToCenter < SQR(((9.0/10.0) * radius));
    }
    return hit;
}

- (BOOL)trackMouseForTimeChangeEvent:(NSEvent *)theEvent inRect:(NSRect)cellFrame ofView:(NSView *)controlView {
    NSEvent *currentEvent = nil;
    float radius = MIN(NSWidth(cellFrame), NSHeight(cellFrame))/2.0;
    NSPoint centerPoint = NSMakePoint(floor(NSMinX(cellFrame)+radius+.5), floor(NSMinY(cellFrame)+radius+.5));
    float angleFromNorth = 0.0;
    
    currentEvent = theEvent;

    // Track mouse dragged events until mouse up.  Always enter atleast once.    
    do {
        NSPoint mousePoint = [controlView convertPoint: [currentEvent locationInWindow] fromView:nil];
	switch ([currentEvent type]) {
            case NSLeftMouseDown:
	    case NSLeftMouseDragged:
		// For each movement, update the position of the hour hand by adjusting our time.
		angleFromNorth = AngleFromNorth(centerPoint, mousePoint, [controlView isFlipped]);
		[self setHourHandByAngleFromNorth:angleFromNorth];
                [(NSControl *)controlView updateCell: self];
		break;
	    default:
		// If we find anything other than a mouse dragged (mouse up) we are done.
		return YES;
	}
    } while (currentEvent = [[controlView window] nextEventMatchingMask:(NSLeftMouseDraggedMask  | NSLeftMouseUpMask) untilDate:[NSDate distantFuture] inMode:NSEventTrackingRunLoopMode dequeue:YES]);

    return YES;
}

- (BOOL)trackMouse:(NSEvent *)theEvent inRect:(NSRect)cellFrame ofView:(NSView *)controlView untilMouseUp:(BOOL)flag {
    NSPoint locationInCellFrame = [controlView convertPoint:[theEvent locationInWindow] fromView:nil];
    
    if ( PointIsInClockFace(locationInCellFrame, cellFrame) ) {
	// If the mouse click happened on the clock face, drag the hour hand around.
	[self trackMouseForTimeChangeEvent:theEvent inRect:cellFrame ofView:controlView];
    } else {
	// ... otherwise, just toggle, the AM/PM indication.
	[self toggleAmPm:nil];
    }
    
    return YES;
}

// ---------------------------------------------------------
//  Context Menu
// ---------------------------------------------------------

- (NSMenu *)menuForEvent:(NSEvent *)theEvent inRect:(NSRect)cellFrame ofView:(NSView *)controlView {
    NSMenu *menu = [[NSMenu alloc] init];
    
    // Return a menu that shows our time setting, and provides an option to set the time to now.
    [menu addItemWithTitle:[self stringValue] action:NULL keyEquivalent:@""];
    [menu addItem: [NSMenuItem separatorItem]];
    [[menu addItemWithTitle:@"Set Time To Now" action:@selector(setTimeToNow:) keyEquivalent:@""] setTarget: self];
    
    return menu;
}

// ---------------------------------------------------------
//  Keyboard Event Handling : Binding Methods
// ---------------------------------------------------------

- (void)moveLeft:(id)sender {
    // Use left arrow to decrement the time.  If the shift key is down, use a big step size.
    BOOL shiftKeyDown = ([[[NSApplication sharedApplication] currentEvent] modifierFlags] & NSShiftKeyMask)!=0;
    [self incrementHour:0 andMinute:-(shiftKeyDown ? 15 : 1)];
}

- (void)moveRight:(id)sender {
    // Use right arrow to increment the time.  If the shift key is down, use a big step size.
    BOOL shiftKeyDown = ([[[NSApplication sharedApplication] currentEvent] modifierFlags] & NSShiftKeyMask)!=0;
    [self incrementHour:0 andMinute: (shiftKeyDown ? 15 : 1)];
}

- (void)performClick:(id)sender {
    // Use the space bar to toggle the AM/PM indication.
    [self toggleAmPm: sender];
}

@end

@implementation ClockControl

+ (void)initialize {
    if (self == [ClockControl class]) {
        [self setCellClass: [ClockCell class]];
    }
}

+ (Class)cellClass {
    return [ClockCell class];
}

- (void)dealloc {
    [[NSNotificationCenter defaultCenter] removeObserver:self];
    [super dealloc];
}

// Like most NSControls, we don't do much ourselves....

- (void)moveRight:(id)sender {
    [[self cell] moveRight:sender];
}

- (void)moveLeft:(id)sender {
    [[self cell] moveLeft:sender];
}

- (void)performClick:(id)sender {
    [[self cell] performClick:sender];
}

- (void)setTime:(NSCalendarDate *)newTime {
    [[self cell] setTime: newTime];
}

- (NSCalendarDate *)time {
    return [[self cell] time];
}

- (IBAction)takeMinuteValueFrom:(id)sender {
    [[self cell] takeMinuteValueFrom:sender];
}

- (IBAction)takeHourValueFrom:(id)sender {
    [[self cell] takeHourValueFrom:sender];
}

// ---------------------------------------------------------
//  Focus ring maintenance
// ---------------------------------------------------------

// The code that actually draws the focus ring is in ClockCell
// become/resignFirstResponder and windowKeyStateDidChange just cause the focus ring to be redisplayed as necessary.

- (BOOL)becomeFirstResponder {
    BOOL okToChange = [super becomeFirstResponder];
    if (okToChange) [super setKeyboardFocusRingNeedsDisplayInRect: [self bounds]];
    return okToChange;
}

- (BOOL)resignFirstResponder {
    BOOL okToChange = [super resignFirstResponder];
    if (okToChange) [super setKeyboardFocusRingNeedsDisplayInRect: [self bounds]];
    return okToChange;
}

- (void)windowKeyStateDidChange:(NSNotification *)notif {
    if ([[self window] firstResponder]==self) [super setKeyboardFocusRingNeedsDisplayInRect: [self bounds]];
}

- (void)viewDidMoveToWindow {
    NSNotificationCenter *notifCenter = [NSNotificationCenter defaultCenter];
    SEL callback = @selector(windowKeyStateDidChange:);
    
    // If we've been installed in a new window, unregister for notificaions in the old window...
    [notifCenter removeObserver:self];
    
    // ... then register for notifications in the new window.
    [notifCenter addObserver:self selector:callback name:NSWindowDidBecomeKeyNotification object: [self window]];
    [notifCenter addObserver:self selector:callback name:NSWindowDidResignKeyNotification object: [self window]];
}

- (BOOL)acceptsFirstResponder {
    return YES;		// Use me with the keyboard....
}

- (BOOL)needsPanelToBecomeKey {
    return NO;		// Clicking doesn't make us key, but tabbing to us will...
}

@end
