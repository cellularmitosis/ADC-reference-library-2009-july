/*
File:		CustomView.m

Description: 	This is the implementation file for the CustomView class, which handles the drawing of the window content.
		We use a circle graphic and a pentagram graphic, switching between the two depending upon the 
		window's transparency.

Copyright: 	© Copyright 2003 Apple Computer, Inc. All rights reserved.

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
05/2003 - EPJ - Modified from RoundTransparentWindow Cocoa sample for Java Splash Screen sample.
*/

#import "CustomView.h"

@implementation CustomView

/*
//  Please see the RoundTransparentWindow Cocoa sample for more custom window and splash screen samples.
//  The Splash Screen and CustomView portions of this code are derived from that sample.
*/

//This routine is called at app launch time when this class is unpacked from the nib.
//We get set up here.
-(void)awakeFromNib
{
    //load the images we'll use from the bundle's Resources directory
    splashImage = [NSImage imageNamed:@"SplashScreen"];

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
    // Nothing to draw if the splash screen image hasn't been loaded yet.
    if(splashImage == NULL)
        // Return now to avoid setting the content size to nothing.
        return;

    //erase whatever graphics were there before with clear
    [[NSColor clearColor] set];
    NSRectFill([self frame]);
    
    [splashImage compositeToPoint:NSZeroPoint operation:NSCompositeSourceOver];
    
    [[self window] center];
    [[self window] setContentSize:[splashImage size]];
}

@end
