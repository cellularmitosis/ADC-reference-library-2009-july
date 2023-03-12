 /*

File: MyDocument.m

Abstract: Demonstrates how to play a movie in a QTMovieView with a
          Core Image filter applied. The QTMovieView in this case has 
          been designated as a Core Animation layer-backed view, which
          allows it to take advantage of the many advanced visual 
          properties of a Core Animation layer such as the ability to
          apply a Core Image Filter to the contents of the view.

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

#import "MyDocument.h"

@implementation MyDocument

//
// windowNibName
//
// Override returning the nib file name of the document.
//
// If you need to use a subclass of NSWindowController 
// or if your document supports multiple NSWindowControllers, 
// you should remove this method and override -makeWindowControllers 
// instead.
//

- (NSString *)windowNibName
{
    return @"MyDocument";
}

//
// windowControllerDidLoadNib
//
// Be notified that a window controller will or did load a nib with this 
// document as the nib file's owner. 
//
// We will use this opportunity to instantiate a document with either
// a default movie or the movie previously set with setFileURL:
//

- (void)windowControllerDidLoadNib:(NSWindowController *) aController
{
    [super windowControllerDidLoadNib:aController];

    // If movie wasn't chosen via File menu then instantiate a default one.
    if (!mMovie)
    {
        // Use the project default movie to instantiate a QTMovie.
        NSString *path = [[NSBundle mainBundle] pathForResource: @"Sample" ofType:@"mov"];
        mMovie = [QTMovie movieWithFile:path error:nil];
        [mMovie retain];
        // Mark the movie is "editable", so any editing operations
        // like cut, copy, paste will work.
        [mMovie setAttribute:[NSNumber numberWithBool:YES] forKey:QTMovieEditableAttribute];
    }

    // Set the QTMovie on the View for display.
    [mMovieView setMovie:mMovie];
	
    // Show the movie controller grow box.
    [mMovieView setShowsResizeIndicator:YES];

    // Hide the window's resize indicator so it does not
    // interfere with the movie controller.
    [[mMovieView window] setShowsResizeIndicator:NO];

}

//
// readFromURL
//
// Sets the contents of this document by reading from a 
// file or file package, of a specified type, located by a URL
// and return YES if successful.
//
- (BOOL)readFromURL:(NSURL *)absoluteURL ofType:(NSString *)typeName error:(NSError **)outError
{
    mMovie = [QTMovie movieWithURL:absoluteURL error:NULL];
    if (mMovie)
    {
        [mMovie retain];
        
        // Mark the movie is "editable", so any editing operations, such as like cut, copy, paste will work.
        [mMovie setAttribute:[NSNumber numberWithBool:YES] forKey:QTMovieEditableAttribute];

        return YES;
    }
	
    return NO;
}


//
// dealloc
//
// Perform cleanup
//

-(void)dealloc
{
    if (mMovie)
    {
        [mMovie release];
    }
	
    if (mCIFilter)
    {
        [mCIFilter release];
    }
	
    [super dealloc];
}

#pragma mark  Movie Filter Routines


//
// availableFilters
//
// An array of CIFilters to display for the Filters popup menu.
// This method is bound to the popup menu in Interface
// Builder.
//

- (NSArray *)availableFilters
{
    return [NSArray arrayWithObjects:
		@"CIKaleidoscope",
		@"CIGaussianBlur",
		@"CIZoomBlur",
		@"CIColorInvert",
		@"CISepiaTone",
		@"CIBumpDistortion",
		@"CICircularWrap",
		@"CIHoleDistortion",
		@"CITorusLensDistortion",
		@"CITwirlDistortion",
		@"CIVortexDistortion",
		@"CICMYKHalftone",
		@"CIColorPosterize",
		@"CIDotScreen",
		@"CIHatchedScreen",
		@"CIBloom",
		@"CICrystallize",
		@"CIEdges",
		@"CIEdgeWork",
		@"CIGloom",
		@"CIPixellate",
		nil];
}

//
// videoPreviewFilter
//
// Getter method that returns the current filter selection
//

- (NSString *)videoPreviewFilter
{
    return mVideoPreviewFilter;
}

//
// setVideoPreviewFilter
//
// Setter method for the current filter name
//

- (void)setVideoPreviewFilter:(NSString *)videoPreviewFilter
{
    // check to see if the user selected a new filter name
    if (videoPreviewFilter != mVideoPreviewFilter) {
        [mVideoPreviewFilter release];
        mVideoPreviewFilter = [videoPreviewFilter copy];

        // save the new CIFilter
        CIFilter *ciFilter = [CIFilter filterWithName:mVideoPreviewFilter];
        {
            [mCIFilter release];
            mCIFilter = [ciFilter copy];
        }

        // set all input values for the filter to default values
        [mCIFilter setDefaults];

        // mark the view as needing display since the filter changed
        [mMovieView setNeedsDisplay:YES];
    }
	
    [[mMovieView layer] setFilters:[NSArray arrayWithObject:mCIFilter]];
}

#pragma mark  Action Routines

//
// play
//
// Action method to play the movie
//

- (IBAction)play:(id)sender {
	[mMovieView play:sender];
}

//
// pause
//
// Action method to pause the movie
//

- (IBAction)pause:(id)sender {
	[mMovieView pause:sender];
}

@end