//////////
//
//	File:		QTUtils.c
//
//	Contains:	QuickTime movie toolbox utility functions.
//
//	Written by:	Apple Developer Technical Support
//
//	Copyright:	© 2001 by Apple Computer, Inc., all rights reserved.
//
//	Change History (most recent first):
//	   
//	   <1>	 	6/12/01	srk		first file
//
//////////

/*
 IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc. ("Apple") in
 consideration of your agreement to the following terms, and your use, installation, 
 modification or redistribution of this Apple software constitutes acceptance of these 
 terms.  If you do not agree with these terms, please do not use, install, modify or 
 redistribute this Apple software.
 
 In consideration of your agreement to abide by the following terms, and subject to these 
 terms, Apple grants you a personal, non-exclusive license, under Apple’s copyrights in 
 this original Apple software (the "Apple Software"), to use, reproduce, modify and 
 redistribute the Apple Software, with or without modifications, in source and/or binary 
 forms; provided that if you redistribute the Apple Software in its entirety and without 
 modifications, you must retain this notice and the following text and disclaimers in all 
 such redistributions of the Apple Software.  Neither the name, trademarks, service marks 
 or logos of Apple Computer, Inc. may be used to endorse or promote products derived from 
 the Apple Software without specific prior written permission from Apple. Except as expressly
 stated in this notice, no other rights or licenses, express or implied, are granted by Apple
 herein, including but not limited to any patent rights that may be infringed by your 
 derivative works or by other works in which the Apple Software may be incorporated.
 
 The Apple Software is provided by Apple on an "AS IS" basis.  APPLE MAKES NO WARRANTIES, 
 EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED WARRANTIES OF NON-INFRINGEMENT, 
 MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS 
 USE AND OPERATION ALONE OR IN COMBINATION WITH YOUR PRODUCTS.
 
 IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL OR CONSEQUENTIAL 
 DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS 
 OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) ARISING IN ANY WAY OUT OF THE USE, 
 REPRODUCTION, MODIFICATION AND/OR DISTRIBUTION OF THE APPLE SOFTWARE, HOWEVER CAUSED AND 
 WHETHER UNDER THEORY OF CONTRACT, TORT (INCLUDING NEGLIGENCE), STRICT LIABILITY OR 
 OTHERWISE, EVEN IF APPLE HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include <Carbon/Carbon.h>
#include <QuickTime/QuickTime.h>
#include <AppKit/AppKit.h>

#include "QTUtils.h"



static StringPtr MakeMovieTimeDisplayString(long	movieTotalSeconds);
static StringPtr MakeMovieTimeScaleDisplayString(long	movieTimeScale);

//////////
//
// QTUtils_IsAutoPlayMovie
//
// Get the autoplay state of a movie file.
//
// A movie file can have information about its autoplay state in a user data item of type 'play'.
// If the movie doesn't contain an item of this type, then we'll assume that it doesn't autoplay.
// If it does contain an item of this type, then the item data (a Boolean) is 0 for normal play
// and 1 for autoplay.
//
//////////

Boolean QTUtils_IsAutoPlayMovie (Movie theMovie)
{
	UserData		myUserData = NULL;
	Boolean			myAutoPlay = false;
	OSErr			myErr = paramErr;

	// make sure we've got a movie
	if (theMovie == NULL)
		return(myAutoPlay);
		
	// get the movie's user data list
	myUserData = GetMovieUserData(theMovie);
	if (myUserData != NULL) {
		myErr = GetUserDataItem(myUserData, &myAutoPlay, sizeof(myAutoPlay), FOUR_CHAR_CODE('play'), 0);
		if (myErr != noErr)
			myAutoPlay = false;
	}

	return(myAutoPlay);
}

//////////
//
// MakeMovieTimeDisplayString
//
// Construct a string of the form "00:00:00" corresponding to the movie time.
//
//////////

static StringPtr MakeMovieTimeDisplayString(long	movieTotalSeconds)
{
	Str255			tempString;
	StringPtr		finalString;

    long			movieHours;
	long			movieMinutes;
	long			movieSeconds;

    finalString = NewPtr(sizeof(Str255));
    finalString[0] = 0;
    
	/* This is the readout that says something like: “2 minutes 5 seconds” */
	
	movieHours = movieTotalSeconds / 60 * 60;
    movieMinutes = movieTotalSeconds / 60;
	movieSeconds = movieTotalSeconds % 60;
	
    tempString[0] = 0;
	
	if (movieHours != 0)
	{			
        if (movieHours < 10)
        {
            PLstrcat(finalString, "\p0");
        }
		NumToString(movieHours, tempString);
        PLstrcat(finalString, (ConstStr255Param)&tempString);
	}
    else
    {
        PLstrcat(finalString,"\p00");
    }
    
    PLstrcat(finalString,"\p:");
    tempString[0] = 0;
	
	if (movieMinutes != 0)
	{			
        if (movieMinutes < 10)
        {
            PLstrcat(finalString, "\p0");
        }
		NumToString(movieMinutes, tempString);
        PLstrcat(finalString, (ConstStr255Param)&tempString);
	}
    else
    {
        PLstrcat(finalString,"\p00");
    }
    
    PLstrcat(finalString,"\p:");
    tempString[0] = 0;

	if (movieSeconds != 0)
	{
        if (movieSeconds < 10)
        {
            PLstrcat(finalString, "\p0");
        }
		NumToString(movieSeconds, tempString);
        PLstrcat(finalString, (ConstStr255Param)&tempString);
	}
    else
    {
        PLstrcat(finalString,"\p00");
    }

	/* Now, if the movie is shorter than one second long (“zero seconds”), but
		_not_ zero duration, handle that case specially.
		*/

	if ((movieMinutes == 0) && (movieSeconds == 0) && (movieHours == 0) && (movieTotalSeconds != 0))
	{
		finalString[0] = 0;
        PLstrcat(finalString,"\pless than one second");
	}

    p2cstrcpy(finalString, (ConstStr255Param)finalString);
    return finalString;
}

//////////
//
// MakeMovieTimeScaleDisplayString
//
// Construct a display string for the movie time scale.
//
//////////

static StringPtr MakeMovieTimeScaleDisplayString(long	movieTimeScale)
{
	StringPtr		finalString;

    finalString = NewPtr(sizeof(Str255));
    finalString[0] = 0;
    
    NumToString(movieTimeScale, finalString);

    p2cstrcpy(finalString, (ConstStr255Param)finalString);
    return finalString;
}

//////////
//
// GetMovieDurationAsString
//
// Construct a string of the form "00:00:00" corresponding to the movie duration.
//
//////////

NSString *GetMovieDurationAsString(Movie theMovie)
{
    StringPtr	movieDurationString = MakeMovieTimeDisplayString(GetMovieDuration(theMovie) / GetMovieTimeScale(theMovie));
    NSString	*movieDurationNSString = [NSString stringWithCString:movieDurationString];
    DisposePtr((Ptr)movieDurationString);
    
    return movieDurationNSString;
}

//////////
//
// GetMovieTimeScaleAsString
//
// Construct a string of the form "00:00:00" corresponding to the movie current time.
//
//////////

NSString *GetMovieTimeScaleAsString(Movie theMovie)
{
    StringPtr	timeScaleString = MakeMovieTimeScaleDisplayString(GetMovieTimeScale(theMovie));
    NSString	*timeScaleNSString = [NSString stringWithCString:timeScaleString];
    DisposePtr((Ptr)timeScaleString);
    
    return timeScaleNSString;
}
