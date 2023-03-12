/*
    File:  MainController.m

    Abstract:  Main controller class for the "Reducer" example application
     
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

#import "MainController.h"
#import "ImageReducer.h"
#import <QuartzCore/CoreImage.h>
#import "CollapsibleBox/CollapsibleBox.h"
#import "CollapsibleBox/HybridImageView.h"

static NSBitmapImageFileType BitmapImageFileTypeForExtension(NSString *extension);
static NSString *ExtensionForBitmapImageFileType(NSBitmapImageFileType type);

@implementation MainController

+ (void)initialize {
    if (self == [MainController class]) {
        // Register this class' attribute dependencies with Key-Value Observing (KVO).  For example, "bitmapImageFileTypeSupportsCompressionFactor" is a derived value that may change when "bitmapImageFileType" changes.
        [self setKeys:[NSArray arrayWithObject:@"bitmapImageFileType"] triggerChangeNotificationsForDependentKey:@"bitmapImageFileTypeSupportsCompressionFactor"];
        [self setKeys:[NSArray arrayWithObject:@"bitmapImageFileType"] triggerChangeNotificationsForDependentKey:@"bitmapImageFileTypeSupportsInterlacing"];
        [self setKeys:[NSArray arrayWithObject:@"bitmapImageFileType"] triggerChangeNotificationsForDependentKey:@"bitmapImageFileTypeSupportsProgressiveLoading"];
        [self setKeys:[NSArray arrayWithObject:@"outputPath"] triggerChangeNotificationsForDependentKey:@"htmlImageReferenceString"];
    }
}

- init {
    self = [super init];
    if (self) {
        [self setBitmapImageFileType:NSJPEGFileType];
        [self setCompressionFactor:0.9];
        [self setGeneratesProgressiveOutput:YES];
        [self setInterlacesOutput:NO];
    }
    return self;
}

- (void)awakeFromNib {
    [self setBitmapImageFileType:NSJPEGFileType];
    [self setOutputPath:@"ReducerOutput.jpg"];
    [imageReducer addObserver:self forKeyPath:@"outputCIImage" options:0 context:NULL];
    [imageReducer addObserver:self forKeyPath:@"addsShadow" options:0 context:NULL];
    [imageReducer addObserver:self forKeyPath:@"drawBorder" options:0 context:NULL];
    [imageReducer addObserver:self forKeyPath:@"borderCornerRadius" options:0 context:NULL];
    [imageReducer setInputImage:[inputImageView image]];

    [[inputImageView window] makeKeyAndOrderFront:self];
}

- (void)updateResultImage {
    if ([imageReducer drawBorder] || [imageReducer addsShadow]) {
        [outputImageView setImage:[imageReducer outputBitmapImageRep]];
    } else {
        [outputImageView setImage:[imageReducer outputCIImage]];
    }
    // Trigger update of <img ...> reference string, since image may have changed size.
    [self willChangeValueForKey:@"htmlImageReferenceString"];
    [self didChangeValueForKey:@"htmlImageReferenceString"];
}

- (void)observeValueForKeyPath:(NSString *)keyPath ofObject:(id)object change:(NSDictionary *)change context:(void *)context {
    if (object == imageReducer) {
        if ([keyPath isEqualToString:@"outputCIImage"]) {
            [self updateResultImage];
        } else if ([keyPath isEqualToString:@"addsShadow"] || [keyPath isEqualToString:@"drawBorder"] || [keyPath isEqualToString:@"borderCornerRadius"]) {
            // If result image comes with meaningful alpha, we should save the file in a format that supports alpha channels.
            BOOL shouldSaveWithAlpha = [imageReducer addsShadow] || ([imageReducer drawBorder] && [imageReducer borderCornerRadius] > 0.0);
            if (shouldSaveWithAlpha) {
                if (!(bitmapImageFileType == NSPNGFileType || bitmapImageFileType == NSTIFFFileType)) {
                    [self setBitmapImageFileType:NSPNGFileType]; // Arbitrarily choose PNG.
                }
            }
        }
    }
}

#pragma mark -
#pragma mark *** Accessors ***

- (ImageReducer *)imageReducer {
    return imageReducer;
}

- (NSBitmapImageFileType)bitmapImageFileType {
    return bitmapImageFileType;
}

- (void)setBitmapImageFileType:(NSBitmapImageFileType)newBitmapImageFileType {
    if (bitmapImageFileType != newBitmapImageFileType) {
        bitmapImageFileType = newBitmapImageFileType;

        // Update filename extension to match new file type.
        NSString *newExtension = ExtensionForBitmapImageFileType(bitmapImageFileType);
        if (newExtension != nil) {
            NSString *filename = [self outputPath];
            filename = [[filename stringByDeletingPathExtension] stringByAppendingString:newExtension];
            [self setOutputPath:filename];
        }
    }
}

- (BOOL)bitmapImageFileTypeSupportsCompressionFactor {
    return (bitmapImageFileType == NSJPEGFileType || bitmapImageFileType == NSJPEG2000FileType) ? YES : NO;
}

- (BOOL)bitmapImageFileTypeSupportsInterlacing {
    return (bitmapImageFileType == NSPNGFileType) ? YES : NO;
}

- (BOOL)bitmapImageFileTypeSupportsProgressiveLoading {
    return (bitmapImageFileType == NSJPEGFileType || bitmapImageFileType == NSJPEG2000FileType) ? YES : NO;
}

- (NSString *)outputPath {
    return [[outputPath retain] autorelease];
}

- (void)setOutputPath:(NSString *)newOutputPath {
    if (outputPath != newOutputPath) {
        [outputPath release];
        outputPath = [newOutputPath copy];
    }
}

- (NSString *)htmlImageReferenceString {
    // Build and return an HTML "<img ...>" tag that references the image.
    NSString *imagePath = [self outputPath];
    NSURL *imageURL = [NSURL URLWithString:(imagePath ? imagePath : @"") relativeToURL:[NSURL fileURLWithPath:[@"~/Desktop/" stringByExpandingTildeInPath]]];
    return [NSString stringWithFormat:@"<img src=\"%@\" width=\"%d\" height=\"%d\" />", [imageURL absoluteString], [imageReducer outputPixelsWide], [imageReducer outputPixelsHigh]];
}

- (float)compressionFactor {
    return compressionFactor;
}

- (void)setCompressionFactor:(float)newCompressionFactor {
    compressionFactor = newCompressionFactor;
}

- (BOOL)interlacesOutput {
    return interlacesOutput;
}

- (void)setInterlacesOutput:(BOOL)flag {
    interlacesOutput = flag;
}

- (BOOL)generatesProgressiveOutput {
    return generatesProgressiveOutput;
}

- (void)setGeneratesProgressiveOutput:(BOOL)flag {
    generatesProgressiveOutput = flag;
}

#pragma mark -
#pragma mark *** Action Methods ***

- (IBAction)inputImageChanged:(id)sender {
    if (sender == inputImageView) {
        [imageReducer setInputImage:[inputImageView image]];

        [self updateResultImage];
    }
}

- (IBAction)outputPathChanged:(id)sender {
    NSString *newOutputPath = [self outputPath];
    NSString *newExtension = [newOutputPath pathExtension];
    if (newExtension != nil) {
        NSBitmapImageFileType newFileType = BitmapImageFileTypeForExtension(newExtension);
        if (newFileType != -1 && newFileType != bitmapImageFileType) {
            [self setBitmapImageFileType:newFileType];
        }
    }
}

- (IBAction)saveButtonClicked:(id)sender {
    // Get result as an NSBitmapImageRep.  (This causes Core Image to fully render it.)
    NSBitmapImageRep *outputBitmap = [imageReducer outputBitmapImageRep];

    // Encode bitmap data in the requested file format.
    NSDictionary *properties = nil;
    switch (bitmapImageFileType) {
        case NSJPEGFileType:
        case NSJPEG2000FileType:
            properties = [NSDictionary dictionaryWithObjectsAndKeys:[NSNumber numberWithFloat:compressionFactor], NSImageCompressionFactor, [NSNumber numberWithBool:generatesProgressiveOutput], NSImageProgressive, nil];
            break;

        case NSPNGFileType:
            properties = [NSDictionary dictionaryWithObjectsAndKeys:[NSNumber numberWithBool:interlacesOutput], NSImageInterlaced, nil];
            break;
            
        default:
            break;
    }
    NSData *outputData = [outputBitmap representationUsingType:bitmapImageFileType properties:properties];

    // Write to file.
    NSString *fullOutputPath = [[@"~/Desktop" stringByExpandingTildeInPath] stringByAppendingPathComponent:[self outputPath]];
    [outputData writeToFile:fullOutputPath atomically:NO];
}

- (IBAction)copyHTMLToPasteboard:(id)sender {
    NSPasteboard *pasteboard = [NSPasteboard generalPasteboard];
    [pasteboard declareTypes:[NSArray arrayWithObject:NSStringPboardType] owner:self];
    [pasteboard setString:[htmlTextView string] forType:NSStringPboardType];
}

- (IBAction)showHelp:(id)sender {
    [[helpTextView window] makeKeyAndOrderFront:sender];
}

#pragma mark -
#pragma mark *** CollapsibleBox Animation Delegate Methods ***

+ (void)animateViewAndSiblings:(NSView *)targetView targetFrame:(NSRect)newTargetFrame {
    // Compare newTargetFrame with targetView's current frame, to get the change in the origin y coordinate.  This is the amount by which we will shift all of the siblings of targetView that are below it.
    NSRect oldTargetFrame = [targetView frame];
    float yOffset = newTargetFrame.origin.y - oldTargetFrame.origin.y;

    // Build the specification for animating targetView's frame to newTargetFrame.
    NSMutableArray *animationList = [NSMutableArray array];
    [animationList addObject:[NSDictionary dictionaryWithObjectsAndKeys:targetView, NSViewAnimationTargetKey, [NSValue valueWithRect:newTargetFrame], NSViewAnimationEndFrameKey, nil]];

    // Now identify all of the siblings of targetView that reside below it in the window.  Add an animation specification for each to animationList, to open or close the gap as needed.
    NSArray *siblingViews = [[targetView superview] subviews];
    unsigned count = [siblingViews count];
    int index;
    for (index = 0; index < count; index++) {
        NSView *sibling = [siblingViews objectAtIndex:index];
        if (sibling != targetView) {
            // If sibling is below the box we're expanding/collapsing, move it down/up to track with the expanding/collapsing box's motion.
            NSRect oldSiblingFrame = [sibling frame];
            if (oldSiblingFrame.origin.y < oldTargetFrame.origin.y) {
                NSRect newSiblingFrame = oldSiblingFrame;
                newSiblingFrame.origin.y += yOffset;
                [animationList addObject:[NSDictionary dictionaryWithObjectsAndKeys:sibling, NSViewAnimationTargetKey, [NSValue valueWithRect:newSiblingFrame], NSViewAnimationEndFrameKey, nil]];
            }
        }
    }

    // Execute the animation.
    NSViewAnimation *animation = [[NSViewAnimation alloc] initWithViewAnimations:animationList];
    [animation setAnimationBlockingMode:NSAnimationBlocking];
    [animation setDuration:0.25];
    [animation startAnimation];
    [animation release];
}

// Since we want to coordinate positioning of adjacent views when one of our CollapsibleBox instances is expanded or collapsed, we provide this delegate method instead of using CollapsibleBox's built-in animation capability.
- (BOOL)collapsibleBox:(CollapsibleBox *)collapsibleBox shouldAnimateToFrame:(NSRect)newFrame expanding:(BOOL)expanding {
    [[self class] animateViewAndSiblings:collapsibleBox targetFrame:newFrame];

    // Report back that we've taken care of animating the box; there's no need for CollapsibleBox to do anything more.
    return NO;
}

@end

static NSBitmapImageFileType BitmapImageFileTypeForExtension(NSString *extension) {
    static NSDictionary *extensionToTypeTable = nil;
    if (extensionToTypeTable == nil) {
        extensionToTypeTable = [[NSDictionary alloc] initWithObjectsAndKeys:
            [NSNumber numberWithInt:NSBMPFileType], @"bmp",
            [NSNumber numberWithInt:NSJPEGFileType], @"jpg",
            [NSNumber numberWithInt:NSJPEGFileType], @"jpeg",
            [NSNumber numberWithInt:NSJPEG2000FileType], @"jp2",
            [NSNumber numberWithInt:NSPNGFileType], @"png",
            [NSNumber numberWithInt:NSTIFFFileType], @"tif",
            [NSNumber numberWithInt:NSTIFFFileType], @"tiff",
            nil];
    }
    id value = [extensionToTypeTable objectForKey:[extension lowercaseString]];
    return value ? [value intValue] : -1;
}

static NSString *ExtensionForBitmapImageFileType(NSBitmapImageFileType bitmapImageFileType) {
    NSString *result = nil;
    switch (bitmapImageFileType) {
        case NSBMPFileType:
            result = @".bmp";
            break;

        case NSJPEGFileType:
            result = @".jpg";
            break;

        case NSJPEG2000FileType:
            result = @".jp2";
            break;

        case NSPNGFileType:
            result = @".png";
            break;

        case NSTIFFFileType:
            result = @".tiff";
            break;
    }
    return result;
}

