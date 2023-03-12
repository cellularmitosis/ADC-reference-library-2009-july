/*

File: QTMovieExtensions.m

Abstract: QTMovie utility routines, implemented as extensions to the
			QTMovie class.

Version: 1.2

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

Copyright (c) 2005-2007, Apple, Inc., all rights reserved

*/ 

#import "QTMovieExtensions.h"


@implementation QTMovie (QTMovieExtensions)

//
// addImagesAsMPEG4
//
// given an array of image file paths (NSString objects), add each
// image to the movie as a new MPEG4 movie frame
//
// Inputs
//		imageFilesArray - an array of image file paths (NSString objects)
//
// Outputs
//		images specified in imageFilesArray are added to movie
//      as new movie frames
//

- (void)addImagesAsMPEG4:(NSArray *)imageFilesArray
{
	if (!imageFilesArray)
		goto bail;
		
	// when adding images we must provide a dictionary
	// specifying the codec attributes
	NSDictionary *myDict = nil;
	myDict = [NSDictionary dictionaryWithObjectsAndKeys:@"mp4v",
														QTAddImageCodecType,
														[NSNumber numberWithLong:codecHighQuality],
														QTAddImageCodecQuality,
														nil];
	if (!myDict)
		goto bail;

    // create a QTTime value to be used as a duration when adding 
    // the image to the movie
	long long timeValue = 30;
	long timeScale      = 600;
	QTTime duration     = QTMakeTime(timeValue, timeScale);

	// iterate over all the images in the array and add
	// them to our movie one-by-one
	int i;
	for (i = 0; i < [imageFilesArray count]; ++i)
	{
		NSURL *fileUrl = [NSURL fileURLWithPath:[imageFilesArray objectAtIndex:i]];        
		NSImage *anImage = [[NSImage alloc] initWithContentsOfURL:fileUrl];    
        
        if (anImage)
        {
            // Adds an image for the specified duration to the QTMovie
            [self addImage:anImage 
                    forDuration:duration
                    withAttributes:myDict];
            
            // free up our image object
            [anImage release];
        }
	}

bail:
	return;
}

//
// flattenToFilePath
//
// flatten the movie to the specified path
//
// Inputs
//		filePath - destination file path for flattened movie
//
// Outputs
//		movie is flattened to a self-contained movie file
//      specified by the filePath input parameter
//
	
- (BOOL)flattenToFilePath:(NSString *)filePath
{
	BOOL success = NO;

	if (!filePath) 
		goto bail;

	// create a dict. with the movie flatten attribute (QTMovieFlatten)
	// which we'll use to flatten the movie to a file below
	
	// specify a 'YES' in the dictionary to flatten to a new movie file
	
	// specify a 'NO' in the dictionary to only create a reference movie
	NSDictionary	*dict = nil;
	dict = [NSDictionary dictionaryWithObject:[NSNumber numberWithBool:YES] 
				forKey:QTMovieFlatten];
	if (dict)
	{
		// create a new movie file and flatten the movie to the file
		
		// passing the QTMovieFlatten attribute here means the movie
		// will be flattened
		success = [self writeToFile:filePath withAttributes:dict];
	}

bail:
	return success;
}


@end
