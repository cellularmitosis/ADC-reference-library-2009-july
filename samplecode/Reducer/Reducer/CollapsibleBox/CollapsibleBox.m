/*
    File:  CollapsibleBox.m

    Abstract:  An NSBox subclass that can toggle its frame size using animation
     
    Version:  1.0

    Copyright:  © Copyright 2005 Apple Computer, Inc. All rights reserved.

    Disclaimer: IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc.
    ("Apple") in consideration of your agreement to the following terms, and your
    use, installation, modification or redistribution of this Apple software
    constitutes acceptance of these terms.  If you do not agree with these terms,
    please do not use, install, modify or redistribute this Apple software.

    In consideration of your agreement to abide by the following terms, and subject
    to these terms, Apple grants you a personal, non-exclusive license, under Apple’s
    copyrights in this original Apple software (the "Apple Software"), to use,
    reproduce, modify and redistribute the Apple Software, with or without
    modifications, in source and/or binary forms; provided that if you redistribute
    the Apple Software in its entirety and without modifications, you must retain
    this notice and the following text and disclaimers in all such redistributions of
    the Apple Software.  Neither the name, trademarks, service marks or logos of
    Apple Computer, Inc. may be used to endorse or promote products derived from the
    Apple Software without specific prior written permission from Apple.  Except as
    expressly stated in this notice, no other rights or licenses, express or implied,
    are granted by Apple herein, including but not limited to any patent rights that
    may be infringed by your derivative works or by other works in which the Apple
    Software may be incorporated.

    The Apple Software is provided by Apple on an "AS IS" basis.  APPLE MAKES NO
    WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED
    WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS FOR A PARTICULAR
    PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND OPERATION ALONE OR IN
    COMBINATION WITH YOUR PRODUCTS.

    IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL OR
    CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
    GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
    ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION, MODIFICATION AND/OR DISTRIBUTION
    OF THE APPLE SOFTWARE, HOWEVER CAUSED AND WHETHER UNDER THEORY OF CONTRACT, TORT
    (INCLUDING NEGLIGENCE), STRICT LIABILITY OR OTHERWISE, EVEN IF APPLE HAS BEEN
    ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#import "CollapsibleBox.h"

#define ANIMATION_DURATION           0.25
#define DEFAULT_COLLAPSED_HEIGHT    38.0

@implementation CollapsibleBox

// Initializes the attributes that CollapsibleBox adds to NSBox.  This method is called by -initWithFrame:, and is also used by our Interface Builder palette.
- (void)initAddedProperties {
    expanded = YES;
    otherFrameSize = NSMakeSize([self frame].size.width, DEFAULT_COLLAPSED_HEIGHT);
}

- (id)initWithFrame:(NSRect)frame {
    self = [super initWithFrame:frame];
    if (self) {
        [self setTitlePosition:NSNoTitle];
        [self initAddedProperties];
    }
    return self;
}

#pragma mark -
#pragma mark *** Key-Value Binding Support ***

+ (void)initialize {
    if (self == [CollapsibleBox class]) {
        // Declare the attributes of this class that should be shown in the Bindings inspector in Interface Builder.
        [self exposeBinding:@"expanded"];
    }
}

#pragma mark -
#pragma mark *** NSView Method Overrides ***

- (BOOL)isFlipped {
    return YES;
}

#pragma mark -
#pragma mark *** State Accessors ***

- (BOOL)isExpanded {
    return expanded;
}

- (void)setExpanded:(BOOL)flag {
    [self setExpanded:flag animate:YES];
}

- (void)setExpanded:(BOOL)flag animate:(BOOL)animate {
    if (expanded != flag) {

        // Save new state.
        expanded = flag;

        // Get frame size for new state, and remember the frame size for the state we're leaving.
        NSSize newFrameSize = otherFrameSize;
        otherFrameSize = [self frame].size;

        // Resize self, possibly using animation.
        [self setFrameSize:newFrameSize animate:animate];
    }
}

#pragma mark -
#pragma mark *** Resizing / Animation Support ***

- (NSRect)frameForNewSizePinnedToTopLeft:(NSSize)newFrameSize {
    NSRect frame = [self frame];
    if (![[self superview] isFlipped]) {
        float yOffset = newFrameSize.height - frame.size.height;
        frame.origin.y -= yOffset;
    }
    frame.size = newFrameSize;
    return frame;
}

- (void)setFrameSizeWithPinningToTopLeft:(NSSize)newFrameSize {
    NSRect oldFrame = [self frame];
    NSRect newFrame = [self frameForNewSizePinnedToTopLeft:newFrameSize];
    [self setFrame:newFrame];

    // Offset all children of box's contentView to match.
    float yOffset = newFrame.size.height - oldFrame.size.height;
    NSArray *subviews = [[self contentView] subviews];
    unsigned count = [subviews count];
    unsigned index;
    for (index = 0; index < count; index++) {
        NSView *subview = [subviews objectAtIndex:index];
        NSPoint origin = [subview frame].origin;
        origin.y += yOffset;
        [subview setFrameOrigin:origin];
    }
}

- (void)setFrameSize:(NSSize)newFrameSize animate:(BOOL)animate {
    NSView *superview = [self superview];
    if (animate) {
        // Compute the new frame.
        NSRect newFrame = [self frameForNewSizePinnedToTopLeft:newFrameSize];

        // Give animationDelegate, if any, first shot at animating the resize (in case the app needs to coordinate with other animations).
        if (animationDelegate != nil && [animationDelegate respondsToSelector:@selector(collapsibleBox:shouldAnimateToFrame:expanding:)]) {
            if (![animationDelegate collapsibleBox:self shouldAnimateToFrame:newFrame expanding:expanded]) {
                return;
            }
        }

        // Else, do the animation ourselves.
        NSMutableArray *animationArray = [NSMutableArray arrayWithObject:[NSDictionary dictionaryWithObjectsAndKeys:self, NSViewAnimationTargetKey, [NSValue valueWithRect:newFrame], NSViewAnimationEndFrameKey, nil]];
        NSAnimation *animation = [[NSViewAnimation alloc] initWithViewAnimations:animationArray];
        [animation setAnimationBlockingMode:NSAnimationBlocking];
        [animation setDuration:ANIMATION_DURATION];
        [animation startAnimation];
        [animation release];
    } else {
        [superview setNeedsDisplayInRect:[self frame]];
        [self setFrameSizeWithPinningToTopLeft:newFrameSize];
        [self setNeedsDisplay:YES];
    }
}

#pragma mark -
#pragma mark *** Archiving ***

// A view must know how to archive and unarchive itself in order to be placed in a custom Interface Builder palette.

- (void)encodeWithCoder:(NSCoder *)aCoder {
    [super encodeWithCoder:aCoder];
    if ([aCoder allowsKeyedCoding]) {
        [aCoder encodeBool:expanded forKey:@"expanded"];
        [aCoder encodeSize:otherFrameSize forKey:@"otherFrameSize"];
        [aCoder encodeConditionalObject:animationDelegate forKey:@"animationDelegate"];
    }
}

- (id)initWithCoder:(NSCoder *)aDecoder {
    self = [super initWithCoder:aDecoder];
    if (self) {
        if ([aDecoder allowsKeyedCoding]) {
            // Read archived attributes.
            expanded = [aDecoder decodeBoolForKey:@"expanded"];
            otherFrameSize = [aDecoder decodeSizeForKey:@"otherFrameSize"];
            animationDelegate = [aDecoder decodeObjectForKey:@"animationDelegate"];
        }
    }
    return self;
}

@end
