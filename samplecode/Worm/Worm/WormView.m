/*
    WormView.m
    Copyright (c) 2001-2004, Apple Computer, Inc., all rights reserved.
    Author: Douglas Davidson
        
    Implements the basic view class for the WormView.
    Various subclasses show off varying degrees of optimizations.
*/

/*
 IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc. ("Apple") in
 consideration of your agreement to the following terms, and your use, installation, 
 modification or redistribution of this Apple software constitutes acceptance of these 
 terms.  If you do not agree with these terms, please do not use, install, modify or 
 redistribute this Apple software.
 
 In consideration of your agreement to abide by the following terms, and subject to these 
 terms, Apple grants you a personal, non-exclusive license, under Apple’s copyrights in 
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

#import "WormView.h"

#define MAX_WORM_LENGTH 1000
#define WIDTH_QUANTUM 22
#define HEIGHT_QUANTUM 22
#define DEFAULT_LENGTH 19

@implementation WormView

- (id)initWithFrame:(NSRect)frame {
    self = [super initWithFrame:frame];
    if (self) {
        srandom((unsigned)time(NULL));
        wormPositions = malloc(MAX_WORM_LENGTH * sizeof(GamePosition));
        wormTextAttributes = [[NSDictionary alloc] initWithObjectsAndKeys:
            [NSFont boldSystemFontOfSize:16.0], NSFontAttributeName, 
            nil];
        backgroundColor = [[NSColor colorWithPatternImage:[NSImage imageNamed:@"Background Pattern"]] copy];
        [self setString:NSLocalizedString(@"Default Worm String", "The string to be used for the worm's body")];
        [self setInitialLength:DEFAULT_LENGTH];
        [self setDesiredFrameRate:1000.0];
        [self reset];
    }
    return self;
}

- (void)dealloc {
    [self stop:YES];
    [wormString release];
    [wormTextAttributes release];
    [backgroundColor release];
    [super dealloc];
}

// Manage the score

- (int)score {
    return score;
}

- (void)setScore:(int)newScore {
    score = newScore;
    [controller scoreChanged:self];
}

// Managing the controller

- (void)setController:(WormController *)obj {
    controller = obj;
}

- (WormController *)controller {
    return controller;
}

// Set/get the string which makes up the worm's body

- (void)setString:(NSString *)string {
    if (string != wormString) {
        [wormString release];
        wormString = [[string uppercaseString] copy];
    }
}

- (NSString *)string {
    return [[wormString retain] autorelease];
}

// Called once for each frame of the animation

- (BOOL)performAnimation {
    BOOL gameOverFlag;
    gameOverFlag = [self updateState];
    [self setNeedsDisplay:YES];
    return gameOverFlag;
}

// Draw the game area, worm, and target

- (void)drawRect:(NSRect)rect {
    unsigned i;
    NSRect tRect;
    
    [backgroundColor set];
    NSRectFill([self bounds]);
    
    for (i = 0; i < wormLength; i++) {
        NSRect wRect = [self rectForPosition:wormPositions[i]];
        unsigned characterIndex = i % [wormString length];
        NSString *string = [wormString substringWithRange:NSMakeRange(characterIndex, 1)];

        [string drawInRect:wRect withAttributes: wormTextAttributes];
    }
    
    tRect = [self rectForPosition:targetPosition];
    [[NSColor blackColor] set];
    [[NSBezierPath bezierPathWithOvalInRect:NSInsetRect(tRect, 2, 2)] fill];
}

// Coordinates are based on top left corner

- (BOOL)isFlipped {
    return YES;
}

// On click, become first responder

- (BOOL)acceptsFirstResponder {
    return YES;
}


// Keyboard handling

- (void)keyDown:(NSEvent *)event {
    NSString *keys = [event charactersIgnoringModifiers];
    wormHeading = kGameHeadingStraight;
    if (keys && [keys length] > 0) {
        unichar c = [keys characterAtIndex:0];
        if (c == NSLeftArrowFunctionKey) {
            wormHeading = kGameHeadingLeft;
        } else if (c == NSRightArrowFunctionKey) {
            wormHeading = kGameHeadingRight;
        }
    }
}

- (void)setInitialLength:(unsigned)length {
    initialWormLength = (length > 1) ? length : 1;
}

- (void)setDesiredFrameRate:(float)rate {
    desiredFrameRate = (rate > 0) ? rate : 1.0;
    if (wormTimer) [self start];
}

- (float)desiredFrameRate {
    return desiredFrameRate;
}

- (float)actualFrameRate {
    return actualFrameRate;
}

- (BOOL)autoturnAtWalls {
    return YES;
}

- (void)stampTime {
    CFAbsoluteTime time = CFAbsoluteTimeGetCurrent();
    if (time > timeStamp) {
        actualFrameRate = (actualFrameRate + 1 / (time - timeStamp)) / 2;
    }
    timeStamp = time;
}

static void wormTimerCallBack(CFRunLoopTimerRef timer, void *info) 
{
    WormView *self = (WormView *)info;
    CFAbsoluteTime startTime = CFAbsoluteTimeGetCurrent();
    BOOL gameOverFlag = [self performAnimation];
    [self displayIfNeeded];
    [self stampTime];
    if (gameOverFlag) {
        [self stop:YES];
    } else {
        CFRunLoopTimerSetNextFireDate(timer, startTime + CFRunLoopTimerGetInterval(timer));
    }
}

- (void)start {
    if (gameOver) [self reset];
    gameOver = NO;
    if (!wormTimer) {
        CFRunLoopTimerContext context = {0, self, NULL, NULL, NULL};
        timeStamp = CFAbsoluteTimeGetCurrent();
        wormTimer =  CFRunLoopTimerCreate(NULL, timeStamp, 1.0/[self desiredFrameRate], 0, 0, wormTimerCallBack, &context);
        CFRunLoopAddTimer(CFRunLoopGetCurrent(), wormTimer, kCFRunLoopCommonModes);
        [controller gameStatusChanged:self];
    }
}

- (void)stop:(BOOL)flag {
    if (!gameOver && flag) gameOver = YES;
    if (wormTimer) {
        CFRunLoopTimerInvalidate(wormTimer);
        CFRelease(wormTimer);
        wormTimer = NULL;
        [controller gameStatusChanged:self];
    }
}

- (BOOL)gameIsRunning {
    return wormTimer != NULL;
}

- (BOOL)gameIsOver {
    return gameOver;
}

- (BOOL)updateState {
    NSSize size = [self bounds].size;
    unsigned width = size.width / WIDTH_QUANTUM, height = size.height / HEIGHT_QUANTUM;
    GameState state = worm_guts(wormPositions, &wormLength, &wormDirection, &wormHeading, &targetPosition, width > 0 ? width : 1, height > 0 ? height : 1, [self autoturnAtWalls] ? 0 : 1);
    if (state == kGameStateScore) [self setScore:score + 1];
    return (state == kGameStateCrash);
}

- (NSRect)rectForPosition:(GamePosition)position {
    return NSMakeRect(position.x * WIDTH_QUANTUM, position.y * HEIGHT_QUANTUM, WIDTH_QUANTUM, HEIGHT_QUANTUM);
}

- (NSRect)integralRectForRect:(NSRect)rect {
    NSRect scaleRect = NSIntegralRect(NSMakeRect(rect.origin.x / WIDTH_QUANTUM, rect.origin.y / HEIGHT_QUANTUM, rect.size.width / WIDTH_QUANTUM, rect.size.height / HEIGHT_QUANTUM));
    return NSMakeRect(scaleRect.origin.x * WIDTH_QUANTUM, scaleRect.origin.y * HEIGHT_QUANTUM, scaleRect.size.width * WIDTH_QUANTUM, scaleRect.size.height * HEIGHT_QUANTUM);
}

- (void)reset {
    unsigned i;
    [self stop:NO];
    wormLength = initialWormLength - 1;
    for (i = 0; i < wormLength; i++) {
        wormPositions[i].x = -1;
        wormPositions[i].y = 0;
    }
    targetPosition.x = -1;
    targetPosition.y = 0;
    wormDirection = kGameDirectionEast;
    wormHeading = kGameHeadingStraight;
    actualFrameRate = 0.0;
    [self setScore:0];
    [self setNeedsDisplay:YES];
}

@end
