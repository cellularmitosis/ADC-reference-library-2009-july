 /*
 
 File: PongController.m
 
 Version: 1.0
 
 Disclaimer: IMPORTANT:  This Apple software is supplied to you by 
 Apple Inc. ("Apple") in consideration of your agreement to the
 following terms, and your use, installation, modification or
 redistribution of this Apple software constitutes acceptance of these
 terms.  If you do not agree with these terms, please do not use,
 install, modify or redistribute this Apple software.
 
 In consideration of your agreement to abide by the following terms, and
 subject to these terms, Apple grants you a personal, non-exclusive
 license, under Apple's copyrights in this original Apple software (the
 "Apple Software"), to use, reproduce, modify and redistribute the Apple
 Software, with or without modifications, in source and/or binary forms;
 provided that if you redistribute the Apple Software in its entirety and
 without modifications, you must retain this notice and the following
 text and disclaimers in all such redistributions of the Apple Software. 
 Neither the name, trademarks, service marks or logos of Apple Inc. 
 may be used to endorse or promote products derived from the Apple
 Software without specific prior written permission from Apple.  Except
 as expressly stated in this notice, no other rights or licenses, express
 or implied, are granted by Apple herein, including but not limited to
 any patent rights that may be infringed by your derivative works or by
 other works in which the Apple Software may be incorporated.
 
 The Apple Software is provided by Apple on an "AS IS" basis.  APPLE
 MAKES NO WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION
 THE IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS
 FOR A PARTICULAR PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND
 OPERATION ALONE OR IN COMBINATION WITH YOUR PRODUCTS.
 
 IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL
 OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 INTERRUPTION) ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION,
 MODIFICATION AND/OR DISTRIBUTION OF THE APPLE SOFTWARE, HOWEVER CAUSED
 AND WHETHER UNDER THEORY OF CONTRACT, TORT (INCLUDING NEGLIGENCE),
 STRICT LIABILITY OR OTHERWISE, EVEN IF APPLE HAS BEEN ADVISED OF THE
 POSSIBILITY OF SUCH DAMAGE.
 
 Copyright (C) 2007 Apple Inc. All Rights Reserved.
 
 */ 
 
/*************************************************************
/* The PongController class moves the paddles and ball around 
/* the screen on a timer.
/************************************************************/

#import "PongController.h"

#import "PongAI.h"
#import "PongView.h"
#import "Shape.h"

#define kUpdateInterval (1.0f / 60.0f)
#define kUpdateDistance 4

#define kDownArrowKeyCode 125
#define kUpArrowKeyCode 126
#define kSpaceBarKeyCode 49

@implementation PongController

- (void)awakeFromNib
{
    _ball = [[Shape alloc] initWithRect:NSZeroRect];
    _leftPaddle = [[Shape alloc] initWithRect:NSZeroRect];
    _rightPaddle = [[Shape alloc] initWithRect:NSZeroRect];
    _court = [[Shape alloc] initWithRect:NSZeroRect];
    _updateTimer = [NSTimer scheduledTimerWithTimeInterval:kUpdateInterval target:self selector:@selector(update) userInfo:nil repeats:YES];

    [_tabView setDelegate:self];

    // Load default AI script.
    NSString *scriptPath = [[NSBundle mainBundle] pathForResource:@"PongAI" ofType:@"js"];
    NSString *script = [NSString stringWithContentsOfFile:scriptPath
                                                encoding:NSUTF8StringEncoding 
                                                   error:nil];

    [_scriptView setFont:[NSFont fontWithName:@"Courier" size:18]];
    [_scriptView setString:script];
    [_scriptView setSelectedRange:NSMakeRange(0, 0)];
    
    // Begin game.
    [self reset];
}

- (void)dealloc
{
    [_tabView setDelegate:nil];

    [_updateTimer invalidate];
    [self setPongAI:nil];
    [_ball release];
    [_leftPaddle release];
    [_rightPaddle release];
    [_court release];

    [super dealloc];
}

/* Returns all shapes to their starting positions and allocates a new AI. */
- (void)reset
{
    NSRect rect;
    NSRect courtRect = [_pongView bounds];

    [_court setRect:courtRect];

    rect.size.width = 20;
    rect.size.height = 20;
    rect.origin.x = (NSMaxX(courtRect) / 2) - (rect.size.width / 2);
    rect.origin.y = NSMaxY(courtRect) - rect.size.height;
    [_ball setRect:rect];

    rect.size.width = 10;
    rect.size.height = 99;
    rect.origin.x = 1;
    rect.origin.y = NSMaxY(courtRect) / 2 - rect.size.height / 2;
    [_leftPaddle setRect:rect];

    rect.size.width = 10;
    rect.size.height = 99;
    rect.origin.x = NSMaxX(courtRect) - rect.size.width - 1;
    rect.origin.y = NSMaxY(courtRect) / 2 - rect.size.height / 2;
    [_rightPaddle setRect:rect];

    _deltaX = -kUpdateDistance;
    _deltaY = -kUpdateDistance;
    
    [_pongView setBallRect:[_ball rect]];
    [_pongView setLeftPaddleRect:[_leftPaddle rect]];
    [_pongView setRightPaddleRect:[_rightPaddle rect]];

    PongAI *pongAI = [[PongAI alloc] initWithPaddle:_rightPaddle 
                                               ball:_ball 
                                             script:[_scriptView string]];
    [self setPongAI:pongAI];
    [pongAI release];
}

/* Moves paddles and ball. (Called on a timer.) */
- (void)update
{
    unsigned short keyCode = [[_pongView lastKeyDownEvent] keyCode];

    // Spacebar
    if (keyCode == kSpaceBarKeyCode) {
        [self reset];
        return;
    }

    // Exit off left side of screen
    if ([_ball right] < [_court left]) {
        [self reset];
        return;
    }

    // Exit off right side of screen
    if ([_ball left] > [_court right]) {
        [self reset];
        return;
    }

    // Move human paddle
    if (keyCode == kUpArrowKeyCode) {
        [_leftPaddle moveY:kUpdateDistance];
        if ([_leftPaddle top] > [_court top] - 1)
            [_leftPaddle moveY:[_court top] - 1 - [_leftPaddle top]];
        [_pongView setLeftPaddleRect:[_leftPaddle rect]];
    } else if (keyCode == kDownArrowKeyCode) {
        [_leftPaddle moveY: -kUpdateDistance];
        if ([_leftPaddle bottom] < [_court bottom] + 1)
            [_leftPaddle moveY: [_court bottom] + 1 - [_leftPaddle bottom]];
        [_pongView setLeftPaddleRect:[_leftPaddle rect]];
    }

    // Move AI paddle
    Direction direction = [[self pongAI] nextMove];
    if (direction == kUpDirection) {
        [_rightPaddle moveY:kUpdateDistance];
        if ([_rightPaddle top] > [_court top] - 1)
            [_rightPaddle moveY:[_court top] - 1 - [_rightPaddle top]];
        [_pongView setRightPaddleRect:[_rightPaddle rect]];
    } else if (direction == kDownDirection) {
        [_rightPaddle moveY: -kUpdateDistance];
        if ([_rightPaddle bottom] < [_court bottom] + 1)
            [_rightPaddle moveY: [_court bottom] + 1 - [_rightPaddle bottom]];
        [_pongView setRightPaddleRect:[_rightPaddle rect]];
    }

    // Bounce off bottom of screen
    if ([_ball bottom] <= [_court bottom] + 1) {
        [_ball moveY:[_court bottom] + 1 - [_ball bottom]];
        _deltaY = -_deltaY;
    }

    // Bounce off top of screen
    if ([_ball top] >= [_court top] - 1) {
        [_ball moveY:[_court top] - 1 - [_ball top]];
        _deltaY = -_deltaY;
    }

    // Bounce off left paddle
    if ([[_leftPaddle rightFace] intersects:_ball]) {
        _deltaX = abs(_deltaX);

        if ([_ball middleY] > [_leftPaddle top] - ([_leftPaddle height] / 5)) { // top fifth
            _deltaY += kUpdateDistance;
            if (_deltaY == 0)
                _deltaY = kUpdateDistance;
        } else if ([_ball middleY] < [_leftPaddle bottom] + ([_leftPaddle height] / 5)) { // bottom fifth
            _deltaY -= kUpdateDistance;
            if (_deltaY == 0)
                _deltaY = -kUpdateDistance;
        }
    }

    // Bounce off right paddle
    if ([[_rightPaddle leftFace] intersects:_ball]) {
        _deltaX = -abs(_deltaX);

        if ([_ball middleY] < [_rightPaddle bottom] + ([_rightPaddle height] / 5)) { // bottom fifth
            _deltaY -= kUpdateDistance;
            if (_deltaY == 0)
                _deltaY = -kUpdateDistance;
        } else if([_ball middleY] > [_rightPaddle top] - ([_rightPaddle height] / 5)) { // top fifth
            _deltaY += kUpdateDistance;
            if (_deltaY == 0)
                _deltaY = kUpdateDistance;
        }
    }

    // Move ball
    [_ball moveX:_deltaX];
    [_ball moveY:_deltaY];

    [_pongView setBallRect:[_ball rect]];
}

- (PongAI *)pongAI
{
    return [[_pongAI retain] autorelease];
}

- (void)setPongAI:(PongAI *)pongAI
{
    if (_pongAI != pongAI) {
        [_pongAI release];
        _pongAI = [pongAI retain];
    }
}

@end

@implementation PongController(NSTabViewDelegate)

- (void)tabView:(NSTabView *)tabView willSelectTabViewItem:(NSTabViewItem *)tabViewItem
{
    if ([_pongView isDescendantOf:[tabViewItem view]]) {
        [self reset];
    }
}

@end
