/*
    File: DOMElementView.m

    Abstract: A WebKit plug-in view that accesses and modifies its own DOM element.

    Version: 1.0

    Disclaimer: IMPORTANT:  This Apple software is supplied to you by Apple
    Computer, Inc. ("Apple") in consideration of your agreement to the
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
    Neither the name, trademarks, service marks or logos of Apple Computer,
    Inc. may be used to endorse or promote products derived from the Apple
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

    Copyright © 2006 Apple Computer, Inc., All Rights Reserved
*/ 

#import "DOMElementView.h"

#import <WebKit/WebKit.h>

#define HOLE_Y_ADJUST_FACTOR (0.0125)
#define HOLE_INSET_FACTOR (0.14)

@implementation DOMElementView

static NSImage *compassImage = nil;

+ (void)initialize
{
    // Load compass image.  We will use this for the frame of our "magnifying glass".
    NSString *compassPath = [[NSBundle bundleForClass:self] pathForResource:@"compass" ofType:@"png"];
    compassImage = [[NSImage alloc] initWithContentsOfFile:compassPath];
}

/*
 * plugInViewWithArguments is required by the WebPlugIn protocol
 * arguments is an NSDictionary consisting of various properties 
 * the plug-in was created with, including dimensions and PARAM
 * elements
 */
+ (NSView *)plugInViewWithArguments:(NSDictionary *)arguments
{
    DOMElementView *view = [[[self alloc] init] autorelease];
    [view setArguments:arguments];
    return view;
}

- (void)dealloc
{   
    [_arguments release];
    [super dealloc];
}

- (void)setArguments:(NSDictionary *)arguments
{
    if (arguments != _arguments) {
        [_arguments release];
        _arguments = [arguments copy];
    }
}

// Create a magnifying glass effect on the hosting web page's content
- (void)drawRect:(NSRect)rect
{
    // Recursion check -- capturing the superview (see below) could result in a -drawRect: on this view.
    if (capturingSuperview)
        return;
    
    // Capture superview contents into image
    capturingSuperview = YES;
    NSRect frame = [self frame];
    NSView *superview = [self superview];
    NSImage *superviewImage = [[[NSImage alloc] initWithSize:frame.size] autorelease];
    [superviewImage setFlipped:YES];
    [superviewImage lockFocus];
    {
        // We are about to tell the superview to draw this view's frame.  However, the image we are drawing into
        // is only as big as this view's frame.  Therefore we need to translate the graphics context so that this
        // view's origin corresponds with the origin of the desitnation image.
        CGContextRef cgContext = [[NSGraphicsContext currentContext] graphicsPort];
        CGContextSaveGState(cgContext);
        CGContextTranslateCTM(cgContext, -NSMinX(frame), -NSMinY(frame));
        [superview drawRect:frame];
        CGContextRestoreGState(cgContext);
    }
    [superviewImage unlockFocus];
    capturingSuperview = NO;
    
    // Draw the magnifying glass
    NSRect bounds = [self bounds];
    [compassImage drawInRect:bounds fromRect:NSZeroRect operation:NSCompositeSourceOver fraction:1.0];

    // Punch a hole in the middle for the magnified superview contents
    NSRect clipRect = bounds;
    clipRect.origin.y -= NSHeight(bounds) * HOLE_Y_ADJUST_FACTOR;
    clipRect = NSInsetRect(clipRect, NSWidth(bounds) * HOLE_INSET_FACTOR, NSHeight(bounds) * HOLE_INSET_FACTOR);
    NSBezierPath *clipPath = [NSBezierPath bezierPathWithOvalInRect:clipRect];
    [clipPath addClip];
    [[NSColor whiteColor] set];
    [clipPath fill];
    
    // Draw magnified superview contents
    [[NSGraphicsContext currentContext] setImageInterpolation:NSImageInterpolationHigh];
    NSRect fromRect = NSMakeRect(0, 0, NSWidth(frame), NSHeight(frame));
    // ...Making the scale factor adjustable is left as an exercise to the reader...
    [superviewImage drawInRect:[self bounds] fromRect:NSInsetRect(fromRect, NSWidth(fromRect) * 0.25, NSHeight(fromRect) * 0.25) operation:NSCompositeCopy fraction:1.0];
}

// Access the parent web page's DOM from Objective-C to move the magnifying glass around the window
- (void)mouseDragged:(NSEvent *)event
{
    // WebPlugInContainingElementKey is the DOMElement corresponding to this plug-in.
    // By modifying its "left" and "top" CSS properties, we can move the plug-in
    // around on the page.
    DOMElement *element = [_arguments objectForKey:WebPlugInContainingElementKey];
    DOMCSSStyleDeclaration *style = [element style];
    
    // Get "left" property
    NSString *leftString = [style getPropertyValue:@"left"];
    int left = [leftString intValue];
    
    // Track horizontal mouse movement
    left += [event deltaX];
    
    // Set the new "left" property in "px" (pixel) units.
    [style setProperty:@"left" :[NSString stringWithFormat:@"%dpx", left] :@""];

    // Get "top" property
    NSString *topString = [style getPropertyValue:@"top"];
    int top = [topString intValue];
    
    // Track vertical mouse movement
    top += [event deltaY];

    // Set the new "top" property in "px" (pixel) units.
    [style setProperty:@"top" :[NSString stringWithFormat:@"%dpx", top] :@""];
}

@end