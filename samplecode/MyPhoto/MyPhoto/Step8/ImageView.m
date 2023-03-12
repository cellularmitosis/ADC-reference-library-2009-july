/*
 
 File: ImageView.m
 
 Abstract: ImageView 
 
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
 
 Copyright © 2005 Apple Computer, Inc., All Rights Reserved
 
 */ 

#import "ImageView.h"

#define kMargin 10.f

@implementation ImageView
// simple image view to display a given file
// uses ImageIO to open the image file
// handles image dpi and EXIF orientation...

// ---------------------------------------------------------------------------------------------------------------------
- (id)initWithFrame: (NSRect)frameRect
{
    // initialize
	if ((self = [super initWithFrame: frameRect]) != nil)
    {
	}
	return self;
}

// ---------------------------------------------------------------------------------------------------------------------
- (void) dealloc
{
    if (mImage)
        CGImageRelease(mImage);
    [mMeta release];
    [super dealloc];
}

// ---------------------------------------------------------------------------------------------------------------------
- (void) setImagePath: (NSString *)path
{
    // use ImageIO to get a CGImageRef for a file at a given path
    NSURL * url = [NSURL fileURLWithPath: path];

	// create the CGImageSourceRef
    CGImageSourceRef isr = CGImageSourceCreateWithURL((CFURLRef)url, NULL);
    
    if (isr)
    {
        // options for getting image and meta data
        // - create a 'cached' image
        // - allow float pixel data
        NSDictionary* options = [NSDictionary dictionaryWithObjectsAndKeys:
            (id)kCFBooleanTrue, (id)kCGImageSourceShouldCache,
            (id)kCFBooleanTrue, (id)kCGImageSourceShouldAllowFloat,
            NULL];
        
        // create image at index 0 (note that ImageIO supports multi-page TIFFs, GIFs, ...)
        CGImageRef image = CGImageSourceCreateImageAtIndex(isr, 0, (CFDictionaryRef)options);
        
        // get the meta data for the image at index 0
        NSDictionary* meta = (NSDictionary*)CGImageSourceCopyPropertiesAtIndex(isr, 0, (CFDictionaryRef)options);
        
        // keep image and meta-data
        [self setImage: image
          withMetadata: meta];
        
        CFRelease (isr);
    }
}

// ---------------------------------------------------------------------------------------------------------------------
- (void) setImage: (CGImageRef)image 
     withMetadata: (NSDictionary *)meta
{
	CGImageRetain(image);
    if (mImage)
        CGImageRelease(mImage);
	mImage = image;
	
	if (meta)
        [meta retain];
    [mMeta release];
    mMeta = meta;

    // update display
    [self setNeedsDisplay:YES];
}

// ---------------------------------------------------------------------------------------------------------------------
- (BOOL) isOpaque
{
	return YES;
}

// ---------------------------------------------------------------------------------------------------------------------
- (float) dpiWidth
{
	NSNumber*	val = [mMeta objectForKey:(id)kCGImagePropertyDPIWidth];
	float		dpi = [val floatValue];
	return (0 == dpi ? 72. : dpi);
}

// ---------------------------------------------------------------------------------------------------------------------
- (float) dpiHeight
{
	NSNumber*	val = [mMeta objectForKey: (id)kCGImagePropertyDPIHeight];
	float		dpi = [val floatValue];
	return (0 == dpi ? 72 : dpi);
}

// ---------------------------------------------------------------------------------------------------------------------
- (int) orientation
{
	NSNumber*	val = [mMeta objectForKey: (id)kCGImagePropertyOrientation];
	int orient = [val intValue];
	if (orient < 1 || orient > 8)
		orient = 1;
	return orient;
}

// ---------------------------------------------------------------------------------------------------------------------
- (CGAffineTransform) imageTransform
{
    // determine the image transform:
    // this honors: EXIF orientation, non-square-pixel dpi, ...
	float xdpi = [self dpiWidth];
	float ydpi = [self dpiHeight];
	int orient = [self orientation];
	
	float x = (ydpi > xdpi) ? ydpi/xdpi : 1;
	float y = (xdpi > ydpi) ? xdpi/ydpi : 1;
	float w = x * CGImageGetWidth(mImage);
	float h = y * CGImageGetHeight(mImage);
	
	CGAffineTransform ctms[8] = {
    { x, 0, 0, y, 0, 0},  //  1 =  row 0 top, col 0 lhs  =  normal
    {-x, 0, 0, y, w, 0},  //  2 =  row 0 top, col 0 rhs  =  flip horizontal
    {-x, 0, 0,-y, w, h},  //  3 =  row 0 bot, col 0 rhs  =  rotate 180
    { x, 0, 0,-y, 0, h},  //  4 =  row 0 bot, col 0 lhs  =  flip vertical
    { 0,-x,-y, 0, h, w},  //  5 =  row 0 lhs, col 0 top  =  rot -90, flip vert
    { 0,-x, y, 0, 0, w},  //  6 =  row 0 rhs, col 0 top  =  rot 90
    { 0, x, y, 0, 0, 0},  //  7 =  row 0 rhs, col 0 bot  =  rot 90, flip vert
    { 0, x,-y, 0, h, 0}   //  8 =  row 0 lhs, col 0 bot  =  rotate -90
	};
    
	return ctms[orient-1];
}


// ---------------------------------------------------------------------------------------------------------------------
- (CGAffineTransform) imageTransformToFitView
{
    // make sure the entire image fits...
    
	CGRect imageRect = CGRectMake(0., 0., CGImageGetWidth(mImage), CGImageGetHeight(mImage));
	
	CGAffineTransform ctm = [self imageTransform];
    
	CGSize ctmdSize = CGRectApplyAffineTransform(imageRect, ctm).size;
	
    // keep a small border
	NSSize destSize = NSInsetRect([self bounds], kMargin, kMargin).size;
	
	// scale to fit in view rect
	float scale = MIN(destSize.width / ctmdSize.width, destSize.height / ctmdSize.height);
	ctm = CGAffineTransformConcat(ctm, CGAffineTransformMakeScale(scale, scale));
	
	return ctm;
}

// ---------------------------------------------------------------------------------------------------------------------
- (void) drawRect: (NSRect)rect
{
	// draw the background...
    [[NSColor colorWithCalibratedRed: 0.75 
                               green: 0.75
                                blue: 0.75
                               alpha: 1.0] set];
    [NSBezierPath fillRect: [self bounds]];
	
    // ... and the image
	if (mImage)
    {
        CGContextRef context = (CGContextRef)[[NSGraphicsContext currentContext] graphicsPort];
        if (context)
        {
            CGContextSaveGState(context);

            NSRect viewBounds = [self bounds];
            size_t width  = CGImageGetWidth(mImage);
            size_t height = CGImageGetHeight(mImage);
            CGRect imageRect = {{0,0}, {width,height}};
            
            // determine transform - fit to view,  EXIF orientation, non-square-pixel dpi, ...
            CGAffineTransform ctm = [self imageTransformToFitView];
            
            // center in view rect
            CGSize ctmdSize = CGRectApplyAffineTransform(imageRect, ctm).size;
            ctm.tx += viewBounds.origin.x + (viewBounds.size.width  - ctmdSize.width)/2;
            ctm.ty += viewBounds.origin.y + (viewBounds.size.height - ctmdSize.height)/2;
            
            CGContextConcatCTM(context, ctm);
            
            // want faster resizing?
            // if we are in live-resizing we use a faster image scaling (kCGInterpolationNone)
            if (mLiveResizing)
            {
                CGInterpolationQuality qual = CGContextGetInterpolationQuality(context);
                
                CGContextSetInterpolationQuality(context, kCGInterpolationNone);
                
                CGContextDrawImage (context, imageRect, mImage );
                
                CGContextSetInterpolationQuality(context, qual);

            } else
            {
                // use default interpolation quality 
                CGContextDrawImage(context, imageRect, mImage);
            }
            
            CGContextRestoreGState(context);
        }
    }
}

// ---------------------------------------------------------------------------------------------------------------------
- (void)viewWillStartLiveResize
{
    mLiveResizing = YES;
}

// ---------------------------------------------------------------------------------------------------------------------
- (void)viewDidEndLiveResize
{
    mLiveResizing = NO;
    
    // at the end of a live resize - redraw. This time it will be in higher quality
    [self display];
}

@end
