 /*
 
 File: main.c
 
 Abstract: Contains code showing how to create a QuickTime movie from a
           data file containing just the video frames.
 
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
 
 Copyright (C) 2007 Apple Inc. All Rights Reserved.
 
 */ 
 
#include <TargetConditionals.h>

#if TARGET_OS_WIN32
	#include <stdlib.h>
	#include <stdio.h>
	#include <QuickTimeComponents.h>
	#include <QTML.h>
	#define	STRICT
	#include <windows.h>
#else
	#import <QuickTime/QuickTime.h>
#endif

static const int			kFrameWidth			= 200; // your frameWidth
static const int			kFrameHeight		= 240; // your frameHeight
static const TimeScale		kMediaTimeScale		= 600; // your media time scale
static const TimeValue		kDurationPerSample	= 60;  // your duration per sample
static const long			kNumberOfSamples	= 60;  // your number of samples
static const OSType			kCodecType			= 'raw ';	// your codec type 

OSErr makeMovieFromVideoFramesFile(char *inDestMovieFile);
void makeDataRefFromFullPath(char *inFilePath, Handle *outDataRef, OSType *outDataRefType);

/*

Main

Parses command line input parameters looking for a specified
path to a video frames file. Then it calls QuickTime routines
to create a QuickTime movie from this video frames file.

*/

int main (int argc, char * const argv[]) 
{
	OSErr err;
	char *inFile = nil;
	int i;

	// look for the "-input" parameter which designates the video frames file
    for (i = 1; i < argc; i++)
    {
        if ( (0 == strncmp("-input", argv[i], 2)) && i + 1 < argc )
            inFile = (char*)argv[++i];
    }
	
    if (NULL == inFile)
    {
		fprintf(stderr, "CreateMovieFromReferences -input /path/to/inputVideoFramesfile\n");
        err = paramErr;
        goto bail;
    }

	// create a QuickTime movie from the video frames file
	err = makeMovieFromVideoFramesFile(inFile);
	
bail:
    return err;
}

/*

makeMovieFromVideoFramesFile

Creates a movie from a file of contiguous video frames

inDestMovieFile - Your raw video frames file. Once the program has finished, this file
will be converted "in place" to a QuickTime movie.
Note: your video frames file must have the first 16 bytes of your your video frames should
be overwritten with the bit pattern that QuickTime will later recognize as the start of the
'mdat' atom, which it will patch when the movie resource is appended to the file (see the 
"Read Me" file associated with this sample code for more details). Then the first compressed 
video sample can be written at byte offset 16.

*/

OSErr makeMovieFromVideoFramesFile(char *inDestMovieFile)
{
    Handle					dataRef = NULL;
    OSType					dataRefType;
    ImageDescriptionHandle	videoDescH = NULL;
	OSErr					err;

#if TARGET_OS_WIN32
	err = InitializeQTML(0);
	if ((err = GetMoviesError()) != noErr) goto bail;
#endif
	err = EnterMovies();
	if ((err = GetMoviesError()) != noErr) goto bail;

	// create a data reference from the full path to the video frames file
	makeDataRefFromFullPath(inDestMovieFile, &dataRef, &dataRefType);
	if (dataRef == NULL) goto bail;

    Movie m = NewMovie(0);
	if ((err = GetMoviesError()) != noErr) goto bail;

	// create the video track for the movie
    Track videoT = NewMovieTrack( m, Long2Fix(kFrameWidth), Long2Fix(kFrameHeight), kNoVolume);
	if ((err = GetMoviesError()) != noErr) goto bail;
	
	// create the video track media
    Media videoM = NewTrackMedia( videoT, VideoMediaType, kMediaTimeScale, dataRef, dataRefType);
	if ((err = GetMoviesError()) != noErr) goto bail;
	
	videoDescH = (ImageDescriptionHandle)NewHandleClear(sizeof(ImageDescription));
	if (videoDescH == NULL) goto bail;
	
	// create the ImageDescription that will describe our video track media samples
    videoDescH[0]->idSize = sizeof(ImageDescription);
    videoDescH[0]->cType = kCodecType; // the codec type for your data 
    videoDescH[0]->temporalQuality = codecNormalQuality;
    videoDescH[0]->spatialQuality = codecNormalQuality;
    videoDescH[0]->width = kFrameWidth;
    videoDescH[0]->height = kFrameHeight;
    videoDescH[0]->hRes = 72L << 16;
    videoDescH[0]->vRes = 72L << 16;
    videoDescH[0]->depth = 32;
    videoDescH[0]->clutID = -1;

    SampleReference64Record videoRef;
    videoRef.dataOffset.hi = 0; videoRef.dataOffset.lo = 0;
    videoRef.dataSize = (kFrameWidth*kFrameHeight*4) * kNumberOfSamples; 
    videoRef.durationPerSample = kDurationPerSample;
    videoRef.numberOfSamples = kNumberOfSamples;
    videoRef.sampleFlags = 0;

	// now add all of our media samples to the movie data file.
    err = AddMediaSampleReferences64(videoM, (SampleDescriptionHandle)videoDescH,
                            1, &videoRef, 0);
	if (err != noErr) goto bail;

    TimeValue mediaDuration = kNumberOfSamples * kDurationPerSample;
	// inserts a reference to our media samples into the track.
    err = InsertMediaIntoTrack(videoT, 0, 0, mediaDuration, 
				fixed1);	// media's rate (1.0 = media's natural playback rate).
	if (err != noErr) goto bail;

    DataHandler outDataHandler;
	// opens a data handler for our movie storage (the video frames file)
    err = OpenMovieStorage (dataRef, dataRefType, kDataHCanWrite, &outDataHandler );
	if (err != noErr) goto bail;
	
	// add a movie to our movie storage container
    err = AddMovieToStorage (m, outDataHandler );
	if (err != noErr) goto bail;

    err = CloseMovieStorage (outDataHandler);
	outDataHandler = NULL;
	
bail:
	if (videoDescH)
	{
		DisposeHandle((Handle)videoDescH);
	}
	
	return err;
}

/*

makeDataRefFromFullPath

Creates a data reference from a full path string.

inFilePath - pointer to the full path C-string
outDataRef - on return this contains the data reference handle
outDataRefType - on return this contains the data reference type

*/
void makeDataRefFromFullPath(char *inFilePath, Handle *outDataRef, OSType *outDataRefType)
{
	*outDataRef = NULL;

	/* The default encoding for the system; 
		untagged 8-bit characters are usually in this encoding */
	CFStringEncoding sysEncoding = CFStringGetSystemEncoding();
	CFStringRef destFileRef = CFStringCreateWithCString (
								   kCFAllocatorDefault,
								   inFilePath,
								   sysEncoding
								);
	if (NULL == destFileRef)
	{
		return;
	}
	
    // Create data reference for our raw frames file
    QTNewDataReferenceFromFullPathCFString(destFileRef, kQTNativeDefaultPathStyle, 0, outDataRef, outDataRefType);
	CFRelease(destFileRef);
}