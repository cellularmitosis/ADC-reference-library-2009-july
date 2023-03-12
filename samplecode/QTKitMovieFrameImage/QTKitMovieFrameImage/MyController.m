 /*

File: MyController.m

Abstract: Controller class for this sample code project. Contains code to grab an
 NSImage for the current movie frame and display it in an image view. The user
 may also choose a new movie file to display.

Version: <1.0>

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

Copyright (C) 2007 Apple Inc. All Rights Reserved.

*/


#import "MyController.h"


@implementation MyController

//
// awakeFromNib
//
// Here we'll load our default movie and set it on the view
//
-(void)awakeFromNib
{
	/* locate our default QuickTime movie */
	NSBundle *mainBundle = [NSBundle mainBundle];
	NSString *filepath = [mainBundle pathForResource:@"Sample" ofType:@"mov"];
	
	/* make a QTMovie from our default QuickTime movie */
	mMovie = [QTMovie movieWithFile:filepath error:NULL];
	if (mMovie)
	{
		[mMovie retain];
		
		/* set our default movie on the view */
		[mMovieView setMovie: mMovie];
		
		// display movie image for the current movie time
		[self captureMovieImageAtTime:self];
	}

}

#pragma mark Set New Movie
//
// openPanelDidEnd
//
// Called when the open panel is dismissed
// We'll instantiate a new QTMovie from the selected movie file path, and
// then set it on the view and display the image for the current movie time
//
- (void) openPanelDidEnd:(NSOpenPanel *)sheet returnCode:(int)returnCode contextInfo:(void  *)contextInfo
  {
	if (returnCode == 1)
	{
		NSArray *files = [sheet filenames];
		if ([files count])
		{
			// instantiate a new movie from the selected file path
			QTMovie *newMovie = [QTMovie movieWithFile:[files objectAtIndex:0] error:NULL];
			// were we able to properly instantiate a new QTMovie?
			if (newMovie)
			{
				if (mMovie)
				{
					[mMovie release];	// release existing movie
				}
				[newMovie retain];
				mMovie = newMovie;	// save new movie
				
				// set this new movie on the view
				[mMovieView setMovie: mMovie];
				
				// display movie image for the current movie time
				[self captureMovieImageAtTime:self];
			}
		}
	}
  }

//
// openFile
//
// Prompt the user for a new movie file
//
- (void) openFile: (id)sender
  {
	NSOpenPanel *panel = [NSOpenPanel openPanel];
	[panel  // Get the shared open panel
		beginSheetForDirectory:nil	// same directory used in the previous invocation of the panel
		file:nil
		types:[QTMovie movieUnfilteredFileTypes]	// show QTMovie supported file types
		modalForWindow:mWindow  // This makes it show up as a sheet, attached to window
		modalDelegate:self		
		didEndSelector:@selector(openPanelDidEnd:returnCode:contextInfo:)  // Call this method when you're done..
		contextInfo:NULL];  
  }


//
// setMovie
//
// Called when the "set movie" button is pressed
// to allow the user to select and display a different movie.
//
-(IBAction)setMovie:(id)sender
{
	[self  openFile: self];
}

#pragma mark Get Movie Image Frame
//
// captureMovieImageAtTime
//
// Called when the "capture movie image" button is pressed.
// This code gets an NSImage for the current movie frame and
// displays it along with the current movie time in the image view
//
-(IBAction)captureMovieImageAtTime:(id)sender
{
	/*

	The QTMovie frameImageAtTime: method has been updated in Mac OS X Leopard to accept a dictionary of attributes.

	The dictionary of attributes may contain the following keys:

		QTMovieFrameImageSize
		QTMovieFrameImageType
		QTMovieFrameImageRepresentationsType
		QTMovieFrameImageOpenGLContext
		QTMovieFrameImagePixelFormat
		QTMovieFrameImageDeinterlaceFields
		QTMovieFrameImageHighQuality
		QTMovieFrameImageSingleField

	See QTMovie.h for additional information about these attributes.

	Usage examples of these are shown here:

	For the QTMovieFrameImageType attribute the possible values are as follows:
	
		QTMovieFrameImageTypeNSImage
		QTMovieFrameImageTypeCGImageRef
		QTMovieFrameImageTypeCIImage
		QTMovieFrameImageTypeCVPixelBufferRef
		QTMovieFrameImageTypeCVOpenGLTextureRef
		
	Here's how to ask for a particular image type (NSImage is the default):
	
		[attributes setObject:QTMovieFrameImageTypeCIImage forKey:QTMovieFrameImageType];
	
	For NSImage, the default representation is NSBitmapImageRep.
	
	Here's how to specify a particular representation:

		NSArray *typesArray = [NSArray arrayWithObject:@"NSBitmapImageRep"];
		[attributes setObject:typesArray forKey:QTMovieFrameImageRepresentationsType];
		
	Here's how to turn on high quality 
	
		[attributes setObject:[NSNumber numberWithBool:YES] forKey:QTMovieFrameImageHighQuality];
	
	etc.
	
	*/

	NSMutableDictionary *attributes = [NSMutableDictionary dictionary];

	// We'll specify a custom size of 95 X 120 for the returned image:
	NSSize imageSize = NSMakeSize(95, 120);
	NSValue *sizeValue = [NSValue valueWithSize:imageSize];
	[attributes setObject:sizeValue forKey:QTMovieFrameImageSize];

	/* get the current movie time - we'll pass this value to the
	   frameImageAtTime: method below */
	QTTime time = [mMovie currentTime];

	/* return an NSImage for the frame at the current time in the QTMovie */
	NSImage *theImage = [mMovie frameImageAtTime:time
					withAttributes:attributes error:NULL];
	if (theImage)
	{
		/* set the new image on the view */
		[mImageView setImage:theImage];
		
		/* display the current movie time as a string below the 
		  grabbed image in the window */
		[mMovieTime setStringValue:QTStringFromTime(time)];
	}
}

@end
