 /*
 
 File: SquiggleView.m
 
 Abstract: Implementation of the SquiggleView class
 
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
 
 Copyright (C) 2008 Apple Inc. All Rights Reserved.
 
 */ 

#import "SquiggleView.h"
#import "Squiggle.h"
#import "MyDocument.h"

// The "View" class for the Squiggles application
@implementation SquiggleView

// The designated initializer for NSView - we'll want to additioinally create a gradient
- (id)initWithFrame:(NSRect)frame {
    self = [super initWithFrame:frame];
    if (self) {
        gradient = [[NSGradient alloc] initWithColorsAndLocations:[NSColor blackColor], 0., [NSColor colorWithCalibratedRed:0/255. green:16/255. blue:59/255. alpha:1.], 1., nil];
    }
    return self;
}

// The method invoked when it is time for an NSView to draw itself.
- (void)drawRect:(NSRect)rect {
    NSInteger i;
    NSRect bounds = [self bounds];
    CGFloat rotations = [document rotations];
    [gradient drawInRect:bounds angle:90.];
    
	// create a coordinate transformation based on the value of the rotation slider. to be repeatedly applied below
    NSAffineTransform *transform = [NSAffineTransform transform];
    [transform translateXBy:bounds.size.width / 2 yBy:bounds.size.height / 2];
    [transform rotateByDegrees: 360. / rotations];
    [transform translateXBy:-bounds.size.width / 2 yBy:-bounds.size.height / 2];
    
	// for each rotation, draw the the full list of squiggles 
    for (i=0; i < rotations; i++) {
	// iterate over the model with ObjC 2 fast enumeration
	for (Squiggle *squiggle in [document squiggles]) {
	    NSBezierPath *path = [squiggle path];
	    [path setLineWidth:[squiggle thickness]];
	    [[squiggle color] set];
	    [path stroke];
	}
	//  apply the transform to rotate in preparation for the next pass 
	[transform concat];
    }
}


// Now override two of NSResponder's mouse handling methods to address only the events we want

// Start drawing a new squiggle when mouse down is delivered to us
- (void)mouseDown:(NSEvent *)event {
	// convert from the window's coordinate system to this view's coordinates
    NSPoint locationInView = [self convertPoint:[event locationInWindow] fromView:nil];
    [document startNewSquiggleWithPoint:locationInView];
}

- (void)mouseDragged:(NSEvent *)event {
	// convert from the window's coordinate system to this view's coordinates
    NSPoint locationInView = [self convertPoint:[event locationInWindow] fromView:nil];
    [document continueSquiggleWithPoint:locationInView];
}



@end
