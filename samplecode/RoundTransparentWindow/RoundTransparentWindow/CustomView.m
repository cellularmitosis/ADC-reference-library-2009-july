/*
File:		CustomView.m

Description: 	This is the implementation file for the CustomView class, which handles the drawing of the window content.
		We use a circle graphic and a pentagram graphic, switching between the two depending upon the 
		window's transparency.

Author:		MCF

Copyright: 	© Copyright 2001 Apple Computer, Inc. All rights reserved.

Disclaimer:	IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc.
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
                
Change History (most recent first):

03/2001 - MCF - initial version

*/


#import "CustomView.h"

@implementation CustomView

//This routine is called at app launch time when this class is unpacked from the nib.
//We get set up here.
-(void)awakeFromNib
{
    //load the images we'll use from the bundle's Resources directory
    circleImage = [NSImage imageNamed:@"circle"];
    pentaImage = [NSImage imageNamed:@"pentagram"];
    //tell ourselves that we need displaying
    [self setNeedsDisplay:YES];
}

//When it's time to draw, this routine is called.
//This view is inside the window, the window's opaqueness has been turned off,
//and the window's styleMask has been set to NSBorderlessWindowMask on creation,
//so what this view draws *is all the user sees of the window*.  The first two lines below
//then fill things with "clear" color, so that any images we draw are the custom shape of the window,
//for all practical purposes.  Furthermore, if the window's alphaValue is <1.0, drawing will use
//transparency.
-(void)drawRect:(NSRect)rect
{
    //erase whatever graphics were there before with clear
    [[NSColor clearColor] set];
    NSRectFill([self frame]);
    //if our window transparency is >0.7, we decide to draw the circle.  Otherwise, draw the pentagram.
    //If we called -disolveToPoint:fraction: instead of -compositeToPoint:operation:, then the image
    //could itself be drawn with less than full opaqueness, but since we're already setting the alpha
    //on the entire window, we don't bother with that here.
    if ([[self window] alphaValue]>0.7)
    [circleImage compositeToPoint:NSZeroPoint operation:NSCompositeSourceOver];
    else
    [pentaImage compositeToPoint:NSZeroPoint operation:NSCompositeSourceOver];
    //the next line resets the CoreGraphics window shadow (calculated around our custom window shape content)
    //so it's recalculated for the new shape, etc.  The API to do this was introduced in 10.2.
    if (floor(NSAppKitVersionNumber) <= NSAppKitVersionNumber10_1)
    {
        [[self window] setHasShadow:NO];
        [[self window] setHasShadow:YES];
    }
    else
        [[self window] invalidateShadow];
}

@end
