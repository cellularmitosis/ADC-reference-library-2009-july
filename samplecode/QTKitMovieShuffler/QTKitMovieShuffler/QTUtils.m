/*

File: QTUtils.m

Abstract: Utility functions for QTMovies

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

Copyright © 2005 Apple Computer, Inc., All Rights Reserved

*/ 

#import "QTUtils.h"


@implementation QTUtils

//
// appendMovies
//
// given an array of movies, append them together in sequence
// to create a new movie file at the specified path
//
// Inputs
//		inMoviesArray - an array of QTMovie objects
//		filePath - destination file path for flattened movie
//
// Outputs
//      a new movie containing all the movies in the array
//      appended in sequence.
//
	
+ (QTMovie *)appendMovies:(NSArray *)inMoviesArray destFilePath:(NSString *)filePath
{
	QTMovie			*outMovie = NULL; 
	NSDictionary	*dict = nil;
	NSNumber		*isEditable = nil;
	BOOL			success = NO;
	NSEnumerator	*movieObjEnumerator1 = nil, *movieObjEnumerator2 = nil;
	NSSize			savedMovieSizes = {0,0};

	if (!inMoviesArray)
		goto bail;
		
	if (!filePath) 
		goto bail;

	// create our destination movie which we'll use to
	// write all our movie clips
	outMovie = [QTMovie movie];
	if (!outMovie)
		goto bail;
	
	// in order to add segments to our newly created movie
	// we must make sure the "QTMovieEditableAttribute" 
	// attribute is set
	isEditable = [NSNumber numberWithBool:YES];
	[outMovie setAttribute:isEditable forKey:QTMovieEditableAttribute];

	movieObjEnumerator1 = [inMoviesArray objectEnumerator];
	QTMovie *aQTMovie;

	// first we'll iterate over every movie clip to find the
	// one with the biggest natural size -- we'll then use
	// this size for our new movie
	while (aQTMovie = [movieObjEnumerator1 nextObject]) 
	{
		NSValue *naturalSize = [aQTMovie attributeForKey:QTMovieNaturalSizeAttribute];
		NSSize theSize = [naturalSize sizeValue];
		
		// save the biggest dimensions we find 
		// for height/width to use for our destination
		// movie
		if (theSize.width > savedMovieSizes.width)
		{
			savedMovieSizes.width = theSize.width;
		}
		
		if (theSize.height > savedMovieSizes.height)
		{
			savedMovieSizes.height = theSize.height;
		}
	}

	// we'll use the biggest height/width values found for
	// our destination movie
	[outMovie setAttribute:[NSValue valueWithSize:savedMovieSizes] forKey:QTMovieNaturalSizeAttribute];

	// now iterate each clip and add it to our destination
	// movie
	movieObjEnumerator2 = [inMoviesArray objectEnumerator];
	while (aQTMovie = [movieObjEnumerator2 nextObject]) 
	{
		NSNumber *timeScale = [aQTMovie attributeForKey:QTMovieTimeScaleAttribute];
		
		// make a QTTime corresponding to time 0 for our movie
		QTTime timeZero = QTMakeTime(0,[timeScale longValue]);
		
		// make a QTTimeRange for the entire source movie 
		// (time 0 to movie end)
		QTTimeRange timeRange = QTMakeTimeRange(timeZero, [aQTMovie duration]);

		// insert all src movie tracks from this clip into 
		// our destination movie - specify a time range of
		// the entire src. movie clip
		[outMovie insertSegmentOfMovie:aQTMovie timeRange:timeRange 
				atTime:[outMovie duration]];
	}

	// create a dict. with the movie flatten attribute (QTMovieFlatten)
	// which we'll use to flatten the movie to a file below
	dict = [NSDictionary dictionaryWithObject:[NSNumber numberWithBool:YES] 
				forKey:QTMovieFlatten];
	if (dict)
	{
		// create a new movie file and flatten the movie to the file
		
		// passing the QTMovieFlatten attribute here means the movie
		// will be flattened
		success = [outMovie writeToFile:filePath withAttributes:dict];
	}

bail:
	return outMovie;
}


@end
