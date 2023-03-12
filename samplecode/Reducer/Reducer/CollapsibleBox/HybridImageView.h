/*
    File:  HybridImageView.h

    Abstract:  An NSView subclass that can toggle between OpenGL and Cocoa/Quartz rendering
     
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

#import <Cocoa/Cocoa.h>

// "HybridImageView" is a subclass of NSView that displays a single image, centered and unscaled over a solid background.  The image to be displayed can be a CIImage or an NSBitmapImageRep.  If it is a CIImage, the view will draw itself using OpenGL for better performance.  If it is an NSBitmapImageRep, the view will draw itself using Cocoa/Quartz APIs.  An instance of HybridImageView switches automatically between OpenGL and Quartz rendering depending on the type of image it is given to display.

@interface HybridImageView : NSView
{
    id                  image;              // the CIImage or NSBitmapImageRep to display
    NSOpenGLContext     *openGLContext;     // the NSOpenGLContext we use to draw the image if it is a CIImage
    NSOpenGLPixelFormat *pixelFormat;       // an NSOpenGLPixelFormat used to create the NSOpenGLContext
    CIContext           *ciContext;         // the CIContext we use to draw the image if it is a CIImage
    NSColor             *backgroundColor;   // background color used to fill the area behind the image
}

#pragma mark -
#pragma mark *** Accessors ***

// Accessors for the image to display: the image can be a CIImage or an NSBitmapImageRep.
- (id)image;
- (void)setImage:(id)newImage;

- (NSColor *)backgroundColor;
- (void)setBackgroundColor:(NSColor *)newColor;

@end
