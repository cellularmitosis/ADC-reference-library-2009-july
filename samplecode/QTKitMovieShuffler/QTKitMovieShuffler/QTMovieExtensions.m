/*

File: QTMovieExtensions.m

Abstract: Our custom utility extensions to the QTMovie class

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

#import "QTMovieExtensions.h"


@implementation QTMovie (QTMovieExtensions)

	// returns YES if the movie is currently playing, NO if not
-(BOOL)isPlaying
{
	if ([self rate] == 0)
	{
		return NO;
	}
	
	return YES;
}

	// return the movie duration as a string
-(NSString *)durationAsString
{
	QTTime durationQTTime = [self duration];
	long long durationInSeconds = durationQTTime.timeValue/durationQTTime.timeScale;
	long long durationInMinutes = durationInSeconds/60;
	long long durationInHours = durationInMinutes/60;
	return ([NSString stringWithFormat:@"%qi:%qi:%qi" , 
								durationInHours, 
									durationInMinutes, 
										durationInSeconds]);
}

	// return the movie's current play time
-(QTTime)currentPlayTime
{
	NSValue *attribute = [self attributeForKey:QTMovieCurrentTimeAttribute];
	QTTime movieTime = [attribute QTTimeValue];
	
	return (movieTime);
}

	// set the movie's current time
-(void)setTime:(int)timeValue
{
	QTTime movieQTTime;
	NSValue *valueForQTTime;
	
	[self timeToQTTime:timeValue resultTime:&movieQTTime];

	valueForQTTime = [NSValue valueWithQTTime:movieQTTime];

	[self setAttribute:valueForQTTime forKey:QTMovieCurrentTimeAttribute];
}

	// return the movie's current (non-playing) time
-(double)currentTimeValue
{
	QTTime time = [self currentTime];

	return(time.timeValue);
}

	// returns YES if the movie's current time is the movie end,
	// NO if not
-(BOOL)currentTimeEqualsDuration
{	
	QTTime movDuration = [self duration];
	QTTime curTime = [self currentTime];
	if (QTTimeCompare(movDuration,curTime) == NSOrderedSame)
	{
		return YES;
	}
	
	return NO;
}

	// returns the movie's poster image - return a generic image
	// if there isn't one
-(NSImage *)posterImage
{
	NSImage *moviePosterImage = nil;
	
	moviePosterImage = [self posterImage];
	
	// create a dummy poster image if none exists
	if (nil == moviePosterImage)
	{
		moviePosterImage = [self currentFrameImage];
	}
	
	if (!moviePosterImage) goto bail;
	
	// set a poster name if none exists
	if (![moviePosterImage name])
	{
		NSString *aName = [self movieFileName];
		if (aName)
		{
			[moviePosterImage setName:aName];
		}
		else
		{
			[moviePosterImage setName:@"<no name>"];
		}
	}

bail:
	return moviePosterImage;
}


	// convert a time value (long) to a QTTime structure
-(void)timeToQTTime:(long)timeValue resultTime:(QTTime *)aQTTime
{
	NSNumber *timeScaleObj;
	long timeScaleValue;

	timeScaleObj = [self attributeForKey:QTMovieTimeScaleAttribute];
	timeScaleValue = [timeScaleObj longValue];

	*aQTTime = QTMakeTime(timeValue, timeScaleValue);
}

	// return the movie file path
-(NSString *)movieFilePath
{
	return ([self attributeForKey:QTMovieFileNameAttribute]);
}

	// return the movie file name
-(NSString *)movieFileName
{
	return ([self filenameFromFullPath:[self movieFilePath]]);
}

	// returns the movie's filename specified by the full path
- (NSString *)filenameFromFullPath:(NSString *)fullPath
{
	NSArray *pathComponents = [fullPath componentsSeparatedByString:@"/"];
	return ([pathComponents objectAtIndex:([pathComponents count] - 1)]);
}


@end
