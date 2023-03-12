/*
	File:		QuickTimeUtils.c
	
	Description: Misc. QuickTime Utilities

	Author:		Apple DTS

	Copyright: 	© Copyright 2005 Apple Computer, Inc. All rights reserved.
	
	Disclaimer:	IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc.
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
				
	Change History (most recent first):

*/

#include "QuickTimeUtils.h"


//////////
//
// GetMovieFromCFURLRef
// Instantiate a QuickTime movie for a QuickTime movie file CFURL.
// 
//////////

OSErr GetMovieFromCFURLRef(CFURLRef *inURLRef, Movie *outMovieRef)
{
	Handle outDataRef;
	OSType outDataRefType;

	// first create a QuickTime data reference for our CFURL
	OSErr err = QTNewDataReferenceFromCFURL(*inURLRef,
                                            0,
                                            &outDataRef,
                                            &outDataRefType);
	require(err == noErr, bail);

	DataReferenceRecord dataRefRecord;
	Boolean		active = true;
	
	dataRefRecord.dataRefType = outDataRefType;
	dataRefRecord.dataRef = outDataRef;
	
    QTNewMoviePropertyElement newMovieProperties[] = 
    {  
        {kQTPropertyClass_DataLocation, kQTDataLocationPropertyID_DataReference, sizeof(dataRefRecord), &dataRefRecord, 0},
            {kQTPropertyClass_NewMovieProperty, kQTNewMoviePropertyID_Active, sizeof(active), &active, 0},
	};

	// instantiate a QuickTime movie from our CFURL data reference
    err = NewMovieFromProperties(sizeof(newMovieProperties) / sizeof(newMovieProperties[0]), newMovieProperties, 0, nil, outMovieRef);
    DisposeHandle(outDataRef);

bail:
	return err;
}


//////////
//
// InitializeQuickTime
// Initializes the Movie Toolbox.
// 
//////////

OSErr InitializeQuickTime()
{
	return (EnterMovies());
}

//////////
//
// MovieHasSoundTrack
// Determine whether a movie contains a sound track.
// 
//////////

Boolean MovieHasSoundTrack (Movie theMovie)
{
	return(GetMovieIndTrackType(theMovie, 1, AudioMediaCharacteristic, movieTrackCharacteristic | movieTrackEnabledOnly) != NULL);
}


//////////
//
// GetMovieExtractionDuration
//
// Determine the movie audio extraction duration which is
// the duration of the longest sound track in the movie.
//
// Note this is different than the movie duration, which is
// the duration of the entire movie irrespective of how long 
// any sound tracks in the movie may be. For example, a given
// sound track in the movie may only have a duration equal to 
// half the length of the actual movie. In this case, we would
// want to stop audio extraction at the end of the sound track
// duration, not at the end of the movie duration
// 
//////////

Float64 GetMovieExtractionDuration (Movie theMovie)
{
    TimeValue maxDuration = 0;
    UInt8 i;

    SInt32 trackCount = GetMovieTrackCount(theMovie);

    if (trackCount) 
    {
        for (i = 1; i < trackCount + 1; i++) 
        {
            Track aTrack = GetMovieIndTrackType(theMovie,
                                                i,
                                                SoundMediaType,
                                                movieTrackMediaType);
            if (aTrack) 
            {
                TimeValue aDuration = GetTrackDuration(aTrack);

                if (aDuration > maxDuration) maxDuration = aDuration;
            }
        }

        return ((Float64)maxDuration / (Float64)GetMovieTimeScale(theMovie));
    }
}





