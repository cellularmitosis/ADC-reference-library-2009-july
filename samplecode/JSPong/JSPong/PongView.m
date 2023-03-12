 /*
 
 File: PongView.m
 
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
/* The PongView class draws the background, paddles, and ball
/* typical of a pong court. Its setters are convenience
/* functions that handle calling -setNeedsDisplayInRect.
/* The PongView also handles tracking whether the up/down 
/* arrow keys are depressed.
/************************************************************/

#import "PongView.h"

@implementation PongView

-(BOOL)isOpaque
{
    return YES;
}

- (void)drawRect:(NSRect)rect
{
    const NSRect *rects;
    int count;
    int i;

    [self getRectsBeingDrawn:&rects count:&count];
    [[NSColor blackColor] setFill];
    for (i = 0; i < count; i++)
        [NSBezierPath fillRect:rects[i]];
        
    if ([self needsToDrawRect:_leftPaddleRect]) {
        [[NSColor whiteColor] setFill];
        [NSBezierPath fillRect:_leftPaddleRect];
    }
    
    if ([self needsToDrawRect:_rightPaddleRect]) {
        [[NSColor whiteColor] setFill];
        [NSBezierPath fillRect:_rightPaddleRect];
    }
    
    if ([self needsToDrawRect:_ballRect]) {
        [[NSColor whiteColor] setFill];
        [[NSBezierPath bezierPathWithOvalInRect:_ballRect] fill];
    }
}

- (NSRect)leftPaddleRect
{
    return _leftPaddleRect;
}

- (void)setLeftPaddleRect:(NSRect)value
{
    if (!NSEqualRects(_leftPaddleRect, value)) {
        [self setNeedsDisplayInRect:_leftPaddleRect];
        _leftPaddleRect = value;
        [self setNeedsDisplayInRect:_leftPaddleRect];
    }
}

- (NSRect)rightPaddleRect
{
    return _rightPaddleRect;
}

- (void)setRightPaddleRect:(NSRect)value
{
    if (!NSEqualRects(_rightPaddleRect, value)) {
        [self setNeedsDisplayInRect:_rightPaddleRect];
        _rightPaddleRect = value;
        [self setNeedsDisplayInRect:_rightPaddleRect];
    }
}

- (NSRect)ballRect
{
    return _ballRect;
}

- (void)setBallRect:(NSRect)value
{
    if (!NSEqualRects(_ballRect, value)) {
        [self setNeedsDisplayInRect:_ballRect];
        _ballRect = value;
        [self setNeedsDisplayInRect:_ballRect];
    }
}

/* Tracks which key is depressed. */

- (NSEvent *)lastKeyDownEvent
{
    return [[_lastKeyDownEvent retain] autorelease];
}

- (void)setLastKeyDownEvent:(NSEvent *)event
{
    if (_lastKeyDownEvent != event) {
        [_lastKeyDownEvent release];
        _lastKeyDownEvent = [event retain];
    }    
}

@end

@implementation PongView (NSResponder)

- (BOOL)acceptsFirstResponder
{
    return YES;
}

- (void)keyDown:(NSEvent *)event
{
    [self setLastKeyDownEvent:event];
}

- (void)keyUp:(NSEvent *)event
{
    if ([event keyCode] == [[self lastKeyDownEvent] keyCode])
        [self setLastKeyDownEvent:nil];
}

@end
