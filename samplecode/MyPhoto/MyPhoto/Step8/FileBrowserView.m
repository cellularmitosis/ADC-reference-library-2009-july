/*
 
 File: FileBrowserView.m
 
 Abstract: FileBrowserView 
 
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

#import "ICAHandler.h"
#import "ImageView.h"
#import "FileBrowserView.h"

#define MAX_ICON_SIZE       600
#define DEFAULT_ICON_SIZE   160
#define ICON_BORDER         20
#define X_INDENT            20
#define Y_INDENT            20

@implementation FileBrowserView

// simple view class to display image thumbnails for all images in a given folder
// in this sample we are not using a very simple way to arrange and draw the thumbnails, 
// real-world code will probably use a better way: create an objC wrapper for the image, let the
// wrapper do the drawing, hit detection, ....

// ---------------------------------------------------------------------------------------------------------------------
- (BOOL)isFlipped
{
	return YES;
}

// ---------------------------------------------------------------------------------------------------------------------
- (id)initWithFrame:(NSRect)frameRect
{
	if ((self = [super initWithFrame:frameRect]) != nil) 
    {
        // create a NSMutableDictionary to hold all the images... 
        mImages = [[NSMutableDictionary alloc] init];
        
        // update files and UI
		[self updateFiles];
	}
	return self;
}

// ---------------------------------------------------------------------------------------------------------------------
- (void)awakeFromNib
{
    // set the default thumbnail size
    mThumbnailSize = DEFAULT_ICON_SIZE;
    
    [mSlider setMaxValue: MAX_ICON_SIZE];
    [mSlider setDoubleValue: DEFAULT_ICON_SIZE];
}

// ---------------------------------------------------------------------------------------------------------------------
- (void)setFrameSize: (NSSize)newSize
{
    // readjust the frame height...
    int     iconSize = mThumbnailSize + ICON_BORDER;
    int     count = [mImages count];
    int     columns = newSize.width / iconSize;
    
    int     rows = (count / columns) + 1;
    
    [super setFrameSize: NSMakeSize( newSize.width, rows * iconSize)];
}

// ---------------------------------------------------------------------------------------------------------------------
- (CGImageRef)thumbnailForFile: (NSString*)name
                        atPath: (NSString*)filePath
{
    // use ImageIO to get a thumbnail for a file at a given path
    CGImageSourceRef    isr = NULL;
    NSString *          path = [filePath stringByExpandingTildeInPath];
    CGImageRef          image = NULL;
    
    path = [path stringByAppendingPathComponent: name];
    
    // create the CGImageSourceRef
    isr = CGImageSourceCreateWithURL((CFURLRef)[NSURL fileURLWithPath: path], NULL);
    if (isr)
    {
        // create a thumbnail:
        // - specify max pixel size
        // - create the thumbnail with honoring the EXIF orientation flag (correct transform)
        // - always create the thumbnail from the full image (ignore the thumbnail that may be embedded in the image -
        //                                                  reason: our MAX_ICON_SIZE is larger than existing thumbnail)
        image = CGImageSourceCreateThumbnailAtIndex (isr, 0, (CFDictionaryRef)[NSDictionary dictionaryWithObjectsAndKeys:
            [NSNumber numberWithInt: MAX_ICON_SIZE],  kCGImageSourceThumbnailMaxPixelSize,
            (id)kCFBooleanTrue,                       kCGImageSourceCreateThumbnailWithTransform,
            (id)kCFBooleanTrue,                       kCGImageSourceCreateThumbnailFromImageAlways,
            NULL] );
        
        CFRelease(isr);
    }
    return image;
}

// ---------------------------------------------------------------------------------------------------------------------
- (void)drawRect: (NSRect)rect
{
    CGContextRef    context = [[NSGraphicsContext currentContext] graphicsPort];
    NSRect          frame = [self frame];
    int             iconSize = mThumbnailSize + ICON_BORDER;
    int             loop;
    CGImageRef      icon;
    NSArray *       keys = [mImages allKeys];
    int             count = [keys count];
    NSString *      name;
    int             x = X_INDENT;
    int             y = frame.size.height + Y_INDENT - iconSize;
    CGRect          r;
    
    // blindly draw all the thumbnails
    // note: this could be optimized - there's no need to draw the thumbnails that are outside the update rect ('rect')
    for (loop = 0; loop < count; loop++)
    {
        name = [keys objectAtIndex: loop];
        icon = (CGImageRef)[mImages objectForKey: name];
        
        // lazyly initialize the thumbnail image for a given file
        if ([NSNull null] == (id)icon)
        {
            icon = [self thumbnailForFile: name
                                   atPath: @"~/Pictures/myPhoto/"];
            
            if (icon)
            {
                [mImages setObject: (id)icon
                            forKey: name];
            } else
            {
                icon = [self thumbnailForFile: @""
                                       atPath: [[NSBundle mainBundle] pathForResource: @"file"
                                                                               ofType: @"tiff"]];
            }
        }
        
        // if we have the image - finally draw it
        if (icon)
        {
            r = CGRectMake(x, y,
                           CGImageGetWidth(icon) * mThumbnailSize / MAX_ICON_SIZE,
                           CGImageGetHeight(icon) * mThumbnailSize / MAX_ICON_SIZE);
            
            x += iconSize;
            if (x + iconSize> frame.size.width)
            {
                x = X_INDENT;
                y -= iconSize;
            }
            
            CGContextSaveGState(context);
            
            CGContextScaleCTM(context,1.,-1.);
            CGContextTranslateCTM(context,0.,-frame.size.height);
            CGContextDrawImage (context, r, icon );
            
            CGContextRestoreGState(context);
        }
    }
}

// ---------------------------------------------------------------------------------------------------------------------
- (void)updateFiles
{
    // sync files in mImages with what's in ~/Pictures/myPhoto/
    
    NSString *      path = [@"~/Pictures/myPhoto/" stringByExpandingTildeInPath];
    NSArray *   	array = [[NSFileManager defaultManager] directoryContentsAtPath: path];
    NSEnumerator *  enumerator;
    NSMutableSet *  myImages = [[[NSMutableSet alloc] initWithArray: [mImages allKeys]] autorelease];
    NSMutableSet *  images   = [[[NSMutableSet alloc] initWithArray: array] autorelease];
    NSString *      name;
    
    // remove all images from myImages that are no longer on disk
    [myImages minusSet: images];
    
    enumerator = [myImages objectEnumerator];
    while (name = [enumerator nextObject])
    {
        [mImages removeObjectForKey: name];
    }
    
    // add all images from disk that are not in myImages
    myImages = [[[NSMutableSet alloc] initWithArray: [mImages allKeys]] autorelease];
    [images minusSet: myImages];
    
    enumerator = [images objectEnumerator];
    while (name = [enumerator nextObject])
    {
        if ('.' != [name characterAtIndex: 0])
            [mImages setObject: [NSNull null]
                        forKey: name];
    }
    
 //   NSLog(@"mImages = %@", mImages);
}

// ---------------------------------------------------------------------------------------------------------------------
- (IBAction)setThumbnailSize: (id)sender
{
    // user changed thumbnail size via slider: resize and redraw
    mThumbnailSize = round([sender floatValue]);
    NSRect frame = [self frame];
    [self setFrameSize: frame.size];

    [self setNeedsDisplay: YES];
}

// ---------------------------------------------------------------------------------------------------------------------
- (void)mouseDown: (NSEvent *)theEvent
{
    // when user double-clicks on a thumbnail, switch to 'single-image' mode...
    
    if ([theEvent clickCount] > 1)
    {
        // find the thumbnail that user double-clicked...
        // again - better way would be to create an objC wrapper for each image/thumbnail,
        // and ask each wrapper if mousedown was inside its rect...
        
        NSRect          frame = [self frame];
        int             iconSize = mThumbnailSize + ICON_BORDER;
        int             loop;
        int             count = [mImages count];
        int             x = X_INDENT;
        int             y = frame.size.height + Y_INDENT - iconSize;
        NSRect          r;
        NSPoint         loc = [self convertPoint: [theEvent locationInWindow] 
                                        fromView: NULL];
        
        loc.y = frame.size.height - loc.y;
        
        for (loop = 0; loop < count; loop++)
        {
            r = NSMakeRect(x, y,  mThumbnailSize, mThumbnailSize);
            
            if (NSPointInRect(loc, r))
            {
                // if mousedown was inside thumbnails rect - ask mICAHandler (ImageView) to display the image
                [mICAHandler showImageAtIndex: loop];
                break;
            }
            x += iconSize;
            if (x + iconSize> frame.size.width)
            {
                x = X_INDENT;
                y -= iconSize;
            }
        }
    }
}

// ---------------------------------------------------------------------------------------------------------------------
- (void)showImageAtIndex: (int)index
{
    // display the image at given index in ImageView
    mCurrentIndex = index;
    NSArray *       keys = [mImages allKeys];

    NSString *      name;
    name = [keys objectAtIndex: index];
    
    NSString * path = [@"~/Pictures/myPhoto/" stringByExpandingTildeInPath];
    [mImageView setImagePath: [path stringByAppendingPathComponent: name]];
}

// ---------------------------------------------------------------------------------------------------------------------
- (void)nextImage
{
    // display the next image
    mCurrentIndex++;
    if (mCurrentIndex >= [mImages count])
        mCurrentIndex = 0;
    [self showImageAtIndex: mCurrentIndex];
}

// ---------------------------------------------------------------------------------------------------------------------
- (void)prevImage
{
    // display the previous image
    mCurrentIndex--;
    if (mCurrentIndex < 0)
        mCurrentIndex = [mImages count]-1;
    [self showImageAtIndex: mCurrentIndex];
}


@end
