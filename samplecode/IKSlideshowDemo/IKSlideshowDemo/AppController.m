/*
 
 File: AppController.m
 
 Abstract: implementation of the Controller class
 
 Version: <1.0>
 
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

#import <ApplicationServices/ApplicationServices.h>
#import "AppController.h"


@implementation AppController
//---------------------------------------------------------------------------------------------------------------------- 
- (BOOL)applicationShouldTerminateAfterLastWindowClosed:(NSApplication *)sender
{
    return YES;
}

//---------------------------------------------------------------------------------------------------------------------- 
- (void)addImagesFromPath: (NSString *)path
{
    NSArray *       array  = [[NSFileManager defaultManager] directoryContentsAtPath: path];
    NSEnumerator *  enumerator;
    NSString *      imagePath;
    
    enumerator = [array objectEnumerator];
    while (imagePath = [enumerator nextObject])
    {
        if ([[[imagePath pathExtension] lowercaseString] isEqualToString: @"jpg"])
        {
            [_imagePaths addObject: [NSString stringWithFormat: @"%@/%@", path, imagePath]]; 
        } else
        {
            [self addImagesFromPath: [NSString stringWithFormat: @"%@/%@/", path, imagePath]]; 
        }
    }
}


// ---------------------------------------------------------------------------------------------------------------------
- (void)loadImages
{
    // find all images inside the 'Screen Savers'
    if (NULL == _imagePaths)
    {
        _imagePaths = [[NSMutableArray alloc] init];

        NSArray *       array  = [[NSFileManager defaultManager] directoryContentsAtPath: @"/System/Library/Screen Savers/"];
        NSEnumerator *  enumerator;
        NSString *      path;
        
        enumerator = [array objectEnumerator];
        while (path = [enumerator nextObject])
        {
            if ([[path pathExtension] isEqualToString: @"slideSaver"])
            {
                [self addImagesFromPath: [NSString stringWithFormat: @"/System/Library/Screen Savers/%@/Contents/Resources/", path]];
            }
        }
    }
}

// ---------------------------------------------------------------------------------------------------------------------
- (void)applicationWillTerminate: (NSNotification *)notification
{
    // save user selection
    [[NSUserDefaults standardUserDefaults] setInteger: [_itemMatrix selectedRow]
                                               forKey: @"modeSelection"];
    [[NSUserDefaults standardUserDefaults] setInteger: [_numberOfItems integerValue]
                                               forKey: @"numberOfItems"];
}

// ---------------------------------------------------------------------------------------------------------------------
- (NSString*)samplePDFPath
{
    // return a path to our sample pdf document...
    
    return @"/Developer/ADC Reference Library/documentation/CHUD/SharkUserGuide.pdf";
}

// ---------------------------------------------------------------------------------------------------------------------
- (void)awakeFromNib
{
    // get the image paths
    [self loadImages];
    
    
    // restore user selection
    [_itemMatrix selectCellAtRow: [[NSUserDefaults standardUserDefaults] integerForKey: @"modeSelection"]
                          column: 0];

    [_numberOfItems setIntegerValue: [[NSUserDefaults standardUserDefaults] integerForKey: @"numberOfItems"] ];
    
    if ([_numberOfItems integerValue] == 0)
        [_numberOfItems setIntegerValue: 20];
    
    
    // get the sample PDF document - if it does not exist - disable PDFs
    NSString * path = [self samplePDFPath];
    
    if ([[NSFileManager defaultManager] fileExistsAtPath: path])
    {
        _pdfDoc = [[PDFDocument alloc] initWithURL: [NSURL fileURLWithPath: path
                                                               isDirectory: NO]];
    } else
    {
        id cell;
        cell = [_itemMatrix cellAtRow: 0
                               column: 0];
        [cell setEnabled: NO];
        cell = [_itemMatrix cellAtRow: 1
                               column: 0];
        [cell setEnabled: NO];
        
        [_itemMatrix selectCellAtRow: 2
                              column: 0];
    }
                                                           
}

// ---------------------------------------------------------------------------------------------------------------------
- (IBAction)start: (id)sender
{
    // start the Slideshow
    [[IKSlideshow sharedSlideshow] runSlideshowWithDataSource: (id<IKSlideshowDataSource>)self
                                                      options: NULL];

}

// ---------------------------------------------------------------------------------------------------------------------
- (void)stopSlideshow: (id)sender
{
    // stop the Slideshow
    [[IKSlideshow sharedSlideshow] stopSlideshow: self];
}    

// ---------------------------------------------------------------------------------------------------------------------
- (NSUInteger)numberOfSlideshowItems
{
    int mode = [[_itemMatrix selectedCell] tag];
    
    switch (mode)
    {
        case 0:
            // 1 URL
            return 1;
            break;

        case 1:
            // return the PDF document page count
            return [_pdfDoc pageCount];
            break;

        case 2:
            // return the user specified number of images
            return [_numberOfItems intValue];
            break;
    }
    return 0;
}

// ---------------------------------------------------------------------------------------------------------------------
- (id)slideshowItemAtIndex: (NSUInteger)index
{
    id  result = NULL;
    int mode = [[_itemMatrix selectedCell] tag];
    
    switch (mode)
    {
        case 0:
            // return the URL
            result = [self samplePDFPath];
            break;
            
        case 1:
            // return the PDFPage at index
            result = [_pdfDoc pageAtIndex: index];
            break;
            
        case 2:
            {
                // return the image path at index
                int i =  index % [_imagePaths count];
                result = [_imagePaths objectAtIndex: i];
            }
            break;
    }
    return result;
}

#if 0
// ---------------------------------------------------------------------------------------------------------------------
// to overwrite the name of an image in index-mode, implement nameOfSlideshowItemAtIndex...
- (NSString*)nameOfSlideshowItemAtIndex: (NSUInteger)index
{
    return [NSString stringWithFormat: @"AppController # %d", index];
}
#endif


// ---------------------------------------------------------------------------------------------------------------------
- (void)slideshowWillStart
{
    // IKSlideshow callback - the Slideshow is about to start
    NSLog(@">>> slideshowWillStart");
}

// ---------------------------------------------------------------------------------------------------------------------
- (void)slideshowDidStop
{
    // IKSlideshow callback - the Slideshow was just stopped
    NSLog(@">>> slideshowDidStop");
}

// ---------------------------------------------------------------------------------------------------------------------
- (void)slideshowDidChangeCurrentIndex: (NSUInteger)newIndex
{
    // IKSlideshow callback - the Slideshow was switching to a new index
    NSLog(@"    slideshowDidChangeCurrentIndex - %d", newIndex);
}

@end