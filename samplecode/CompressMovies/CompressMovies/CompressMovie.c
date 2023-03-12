/*
	File:		CompressMovie.c

	Contains:	Functions for recompression of QuickTime movies.

	Written by: 	

	Copyright:	Copyright © 1991-2001 by Apple Computer, Inc., All Rights Reserved.

	Disclaimer:	IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc.
				("Apple") in consideration of your agreement to the following terms, and your
				use, installation, modification or redistribution of this Apple software
				constitutes acceptance of these terms.  If you do not agree with these terms,
				please do not use, install, modify or redistribute this Apple software.

				In consideration of your agreement to abide by the following terms, and subject
				to these terms, Apple grants you a personal, non-exclusive license, under AppleÕs
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
                                    11/7/2001	srk			Carbonized
				7/28/1999	Karl Groethe	Updated for Metrowerks Codewarror Pro 2.1
				

*/

// INCLUDES
#include "Movies.h"
#include "MoviesFormat.h"

#include "CompressMovie.h"
#include "DTSQTUtilities.h"
	
	
// GLOBALS
static 	Boolean				gFirstTime = true;
static 	Boolean				gShowWindow = true;
static	SCTemporalSettings		gTemporalSettings;
static	SCSpatialSettings			gSpatialSettings;
static	SCDataRateSettings		aDataRateSetting;


// ______________________________________________________________________
// FUNCTIONS

// ______________________________________________________________________
// SetFirstRecompressState is a simple wrapper for the global so that we could control from the outside
// when we have done the first pass, and after that the code in RecompressMovieFile should branch off
// and not display the standard compression dialog boxes. The reason it's a function is that we want to 
// maybe change something later, and it's easier this way than  chasing for globals.
pascal void SetFirstRecompressState(Boolean state)
{
	gFirstTime = state;
}


// ______________________________________________________________________
// RecompressMovieFile is a long and windy function, a lot of it is from the ConvertToMovie Jr. 
// sample (SDK CDs). Many parts have been extracted into the DTSQTLibrary file. Anyway, 
// this function could be taken out and implemented in other tools and parts as it's very much 
// self-contained (if you add the DTSQTUtilities files to your project as well).

pascal OSErr RecompressMovieFile(FSSpec* theMovieFile)
{
	OSErr 			anErr = noErr;
	Boolean			abort;
	
	ComponentInstance 	ci = NULL;
	
	long				ciFlags;
	long				nFrames;
	long				aFrameNum;
	TimeValue			currentMovieTime;

	short				aMovieRefNum;
	FSSpec 			newFileFSSpec;

	Movie				aSourceMovie = NULL;
	Rect				 aMovieRect;
	GWorldPtr			srcGWorld = NULL;
	ImageDescription		**anImageDescription;
	ImageSequence		anImageSequence;
	Movie				aDestinationMovie = NULL;
	Track				aDestinationTrack = NULL;
	Media				aDestinationMedia = NULL;
	
// if we use a window, the following variables are used
	Point				where;
	WindowRef			progressWindow;
	
	
	// Open the standard compression component
	ci = OpenDefaultComponent(StandardCompressionType, StandardCompressionSubType);
         DebugAssert(ci != NULL);
	if (ci == NULL) 
            return couldntGetRequiredComponent; 
	
	// Adjust the user settings, we will operate in 32-bit depth (always), and make it possible to leave the
	// data rate field zero (this indicates to use the existing data rate in the movie)
	SCGetInfo(ci, scPreferenceFlagsType, &ciFlags);
	ciFlags &=~scShowBestDepth;
	ciFlags |= scAllowZeroFrameRate;
	SCSetInfo(ci, scPreferenceFlagsType, &ciFlags);
	
	// Open the movie file, make sure it contains a movie.
	anErr = OpenMovieFile(theMovieFile, &aMovieRefNum, 0); ReturnIfError(anErr);
	
	anErr = NewMovieFromFile(&aSourceMovie, aMovieRefNum, NULL, NULL, newMovieActive, NULL);  ReturnIfError(anErr);
	
	CloseMovieFile(aMovieRefNum);
	
	// If the movie does not contain any video tracks, bail out (we don't re-compress sound or other tracks at this point 
	// of time.
	if(! QTUMediaTypeInTrack(aSourceMovie, VideoMediaType))
	{
		DebugAssert("No video tracks in movie");
		return 	invalidMovie;
	}
	
	// Count the amount of video frames in the movie
	nFrames = QTUCountMediaSamples(aSourceMovie, VideoMediaType); DebugAssert(nFrames != -1);
	
	// Given the movie's bounding rect, create a 32-bit GWorld we will use for rendering movie frames and for possible test images.
	{
		CGrafPtr		aSavedPort = NULL;
		GDHandle		aGDHandle = NULL ;
		PicHandle 		aPicHandle = NULL; 
		
		GetMovieBox(aSourceMovie, &aMovieRect);
		
		anErr = NewGWorld(&srcGWorld, 32, &aMovieRect, NULL, NULL, 0);   DebugAssert(anErr == noErr);
		if(anErr != noErr) goto CleanupMemory;
		
		aPicHandle = GetMoviePosterPict(aSourceMovie); 	// don't need to test if the PicHandle was created or not.
		
		if(aPicHandle)
		{
			GetGWorld(&aSavedPort, &aGDHandle);
			SetGWorld(srcGWorld, NULL);
			EraseRect(&aMovieRect);
			
			DrawPicture(aPicHandle, &aMovieRect);
			KillPicture(aPicHandle);
			SetGWorld(aSavedPort, aGDHandle);
			
			// Use the image now in the GWorld as the initial image inside the dialog box for compression.
            
#if TARGET_OS_WIN32
			anErr = SCSetTestImagePixMap(ci, srcGWorld->portPixMap, NULL, 0);  DebugAssert(anErr == noErr);
#else
			anErr = SCSetTestImagePixMap(ci, GetPortPixMap(srcGWorld), NULL, 0);  DebugAssert(anErr == noErr);
#endif
			if(anErr != noErr) goto CleanupMemory;
		}
	}	
	// Set default settings for the compression dialog, and also get the temporal settings we need for further
	// calculations.

	if(gFirstTime)
	{
#if TARGET_OS_WIN32
		anErr = SCDefaultPixMapSettings(ci, srcGWorld->portPixMap, true);   DebugAssert(anErr == noErr);
#else
		anErr = SCDefaultPixMapSettings(ci, GetPortPixMap(srcGWorld), true);   DebugAssert(anErr == noErr);
#endif
		if(anErr != noErr) goto CleanupMemory;

		anErr = SCGetInfo(ci, scTemporalSettingsType, &gTemporalSettings);   DebugAssert(anErr == noErr);
		if(anErr != noErr) goto CleanupMemory;
		
		gTemporalSettings.frameRate = 0;
		
		anErr = SCSetInfo(ci, scTemporalSettingsType, &gTemporalSettings);   DebugAssert(anErr == noErr);
		if(anErr != noErr) goto CleanupMemory;
		
	}	
	

	
	// Clear out the default frame rate at the first pass selected by the standard compression. 0 means that we
	// need to use the data rate of the movie.
	if(gFirstTime)
	{
		// ask the first time from the end user about the default settings we will use with any other movies passed
		// along to this batch of movies (dragged to the app).
		anErr = SCRequestSequenceSettings(ci); DebugAssert(anErr == noErr);
		if(anErr != noErr) return anErr; // eventually scUserCancelled as the error
	
		// Get a copy of the temporal settings we got from the user interaction, we need the values later for
		// other calculations (new frame amount and so on).
	
		anErr = SCGetInfo(ci, scTemporalSettingsType, &gTemporalSettings);  DebugAssert(anErr == noErr);
		if(anErr != noErr) goto CleanupMemory;
		
		anErr = SCGetInfo(ci, scSpatialSettingsType, &gSpatialSettings); DebugAssert(anErr == noErr);
		if(anErr != noErr) goto CleanupMemory;
	}

	if(!gFirstTime)	// We need to set the temporal and the spatial settings the following times.
	{
		anErr = SCSetInfo(ci, scTemporalSettingsType, &gTemporalSettings);   DebugAssert(anErr == noErr);
		if(anErr != noErr) goto CleanupMemory;
		
		anErr = SCSetInfo(ci, scSpatialSettingsType, &gSpatialSettings); DebugAssert(anErr == noErr);
		if(anErr != noErr) goto CleanupMemory;  
	}
	
	// Calculate the max sound rate, so we know the overall data rate for the video (total = video + sound).
	{
		long soundDataRate;
	
                if(gFirstTime)
                {	
                    anErr = SCGetInfo(ci, scDataRateSettingsType, &aDataRateSetting);  DebugAssert(anErr == noErr);
                    if(anErr != noErr) goto CleanupMemory;
                }
		
		if(aDataRateSetting.dataRate)
		{
			anErr = QTUCountMaxSoundRate(aSourceMovie, &soundDataRate);  DebugAssert(anErr == noErr);
			if(anErr != noErr) goto CleanupMemory;
		
			aDataRateSetting.dataRate  -= soundDataRate;
		}
		
		anErr = SCSetInfo(ci, scDataRateSettingsType, &aDataRateSetting);  DebugAssert(anErr == noErr);
		if(anErr != noErr) goto CleanupMemory;
	}
	
	// Calculate the new amount of frames based on the possible new re-defined frame rate.
	if(gTemporalSettings.frameRate)
		nFrames = QTUGetMovieFrameCount(aSourceMovie, gTemporalSettings.frameRate);
	
	// If we want to show a windows when processing the movie, do this here...
	if(gShowWindow)
	{
		Rect aRect = aMovieRect;
		where.h = where.v = -2;
		
		anErr = SCPositionRect(ci, &aRect, &where);  ReturnIfError(anErr);
		
		progressWindow = NewCWindow(0,&aRect, theMovieFile->name, true, 0, (WindowPtr)-1, false, 0);
	}
	
	// Create a new file for the re-compressed movie.
	{
		Str255 newFileName;
		
		// First fix the name FSSpec and name for this new movie.
		BlockMove(theMovieFile->name, newFileName, sizeof(theMovieFile->name));	 
		newFileName[++newFileName[0]] = '*';			// add this character to the beginning of the new file name
	
		// Then create a new FSSpec.
		anErr = FSMakeFSSpec(theMovieFile->vRefNum, theMovieFile->parID, newFileName, &newFileFSSpec);
		
		// Then create a movie file.
		anErr = CreateMovieFile(&newFileFSSpec, 'TVOD', 0, createMovieFileDeleteCurFile, &aMovieRefNum, &aDestinationMovie); DebugAssert(anErr == noErr);
		if(anErr != noErr) goto CleanupGeneral;
	}
		
	// Copy and create various media and tracks for the new movie.
	{
		MatrixRecord aMatrix;
		// Create a new video movie track with the same dimensions as the entire source movie.
		aDestinationTrack = NewMovieTrack(aDestinationMovie, (long)(aMovieRect.right - aMovieRect.left) << 16, (long)(aMovieRect.bottom - aMovieRect.top) << 16, 0);
		
			
		// Create a media for the new track with the same time scale as the source movie. If the time scales are the same,
		// we don't need to adjust the time scales with scale conversions.
		aDestinationMedia = NewTrackMedia(aDestinationTrack, VIDEO_TYPE, GetMovieTimeScale(aSourceMovie), 0, 0);
		anErr = GetMoviesError();DebugAssert(anErr == noErr);
		if(anErr != noErr) goto CleanupGeneral;
		
		// Copy the user data and settings from the source to the destination movie.
		CopyMovieSettings(aSourceMovie, aDestinationMovie);
		
		// Set the movie matrix to identity and clear the movie clip region so that conversion process transforms and composites
		// *all* video tracks into one untransformed video track.
		SetIdentityMatrix(&aMatrix);
		SetMovieMatrix(aDestinationMovie, &aMatrix);
		SetMovieClipRgn(aDestinationMovie, NULL);
		
		// Prepare for adding frames to the movie.
		anErr = BeginMediaEdits(aDestinationMedia); DebugAssert(anErr == noErr);
		if(anErr != noErr) goto CleanupGeneral;
	}
	
	// Start a compression sequence using the parameters chosen earlier (not these are true for all the other movies passed
	// along with the AE. Pass nil for the source rect to use the entire image. We will get an imagedescription as well. Note
	// that the image description handle is disposed by SCCompressSequenceEnd.
#if TARGET_OS_WIN32
	anErr = SCCompressSequenceBegin(ci, srcGWorld->portPixMap, NULL, &anImageDescription); 
#else
	anErr = SCCompressSequenceBegin(ci, GetPortPixMap(srcGWorld), NULL, &anImageDescription); 
#endif
         DebugAssert(anErr == noErr);
	if(anErr != noErr) goto CleanupGeneral;
		
	// Clear out the GWorld and set the movie to draw into this one.
	SetGWorld(srcGWorld, NULL);
#if TARGET_OS_WIN32
	EraseRect(&srcGWorld->portRect);
#else
         {
         Rect portRect;
            GetPortBounds(srcGWorld, &portRect);
            EraseRect(&portRect);
         }
#endif
	SetMovieGWorld(aSourceMovie, srcGWorld, GetGWorldDevice(srcGWorld));
	
	currentMovieTime = 0;			// set current time value to beginning of movie
		
	
	// Loop through all the interesting times counted earlier
	for(aFrameNum = 0; aFrameNum < nFrames; aFrameNum++)
	{
		short		syncFlag;
		TimeValue	duration;
		long		dataSize;
		Handle	compressedData;
		
		// Abort if the end user clicked the mouse or pressed a key.
		{
			EventRecord anEvent;
			
			abort = false;
			if(EventAvail(keyDownMask | mDownMask, &anEvent))
			{
				abort = true;
				break;
			}
		}
		
		// Get the next frame from the movie.
		{
			// If we are resampling the movie, step to the next frame
			if(gTemporalSettings.frameRate)
			{
				// This could could be much smarter about its calculations. The srcMovie duration and destination movie
				// frame durations are both constant and could be calculated outside this loop.
				
				long dur = GetMovieDuration(aSourceMovie);
				currentMovieTime = aFrameNum * dur / (nFrames - 1);
				duration = dur / nFrames;
			}
			else
			{
				short flags = nextTimeMediaSample;
				OSType whichMediaType = VIDEO_TYPE;
				
				// If this is the first frame, include the frame we are currently on.
				if(aFrameNum == 0)
					flags |= nextTimeEdgeOK;
				
				// If we are maintaining the frame durations of the source movie, skip to the next interesting
				// time and get the duration of that frame.
				GetMovieNextInterestingTime(aSourceMovie, flags, 1, &whichMediaType, currentMovieTime, 0, &currentMovieTime, &duration);
			}
			
			SetMovieTimeValue(aSourceMovie, currentMovieTime);
			MoviesTask(aSourceMovie, 0); MoviesTask(aSourceMovie,0); MoviesTask(aSourceMovie,0);
		} //end stepping to next frame from the movie

		{
			// If data rate constraining is being done, tell Standard Compression the duration of the current frame in
			// milliseconds. We only need to do this if the frames have variable durations.
			SCDataRateSettings datarate;
			if(!SCGetInfo(ci, scDataRateSettingsType, &datarate))
			{
				datarate.frameDuration = duration * 1000 / GetMovieTimeScale(aSourceMovie);
				SCSetInfo(ci, scDataRateSettingsType, &datarate);
			}
		}
		
		// Compress the frame, compressedData will hold a handle to the newly compressed image data. dataSize is
		// the size of the compressed data, which will usually be different than the size of the compressData handle.
		// syncFlag is a value that is a key frame. Note that we don't have to dispose the compressedData handle.
		// It will be disposed for us when we call SCCompressSequenceEnd.
#if TARGET_OS_WIN32
		anErr = SCCompressSequenceFrame(ci,srcGWorld->portPixMap, &aMovieRect, &compressedData, &dataSize, &syncFlag);
#else
		anErr = SCCompressSequenceFrame(ci,GetPortPixMap(srcGWorld), &aMovieRect, &compressedData, &dataSize, &syncFlag);
#endif
		ReturnIfError(anErr);
		
		// Append the compressed image data to the media.
		anErr = AddMediaSample(aDestinationMedia, compressedData, 0, dataSize, duration, (SampleDescriptionHandle)anImageDescription, 1, syncFlag, NULL); DebugAssert(anErr == noErr);
		if(anErr != noErr) goto CleanupGeneral;
		
		// Decompress the compressed frame into the progress window.
		if(gShowWindow)
		{
			char hState;

#if TARGET_OS_WIN32			
			SetGWorld((CGrafPtr)progressWindow, NULL); 	// set port to progress window
#else
			SetGWorld(GetWindowPort(progressWindow),NULL);
#endif			
			// If this is the first frame, start up a decompression sequence.
			if(aFrameNum == 0)
			{
				anErr = DecompressSequenceBegin(&anImageSequence, anImageDescription, NULL, NULL, &aMovieRect,
										NULL, ditherCopy, NULL, 0, codecNormalQuality, anyCodec);  					DebugAssert(anErr == noErr);
				if(anErr != noErr) goto CleanupGeneral;
			}
			
			// Save the locked state of the compressed data and then lock it. We want it locked but standard compression may or 
			// may not.
			hState = HGetState(compressedData);
			HLock(compressedData);
			
			// Decompress the frame to the progress window. Note that we StripAddress the compressedData pointer 
			// because it must be 32-bit clean.
			anErr = DecompressSequenceFrame(anImageSequence, *compressedData, 0, NULL, NULL); 						DebugAssert(anErr == noErr);
			if(anErr != noErr) goto CleanupGeneral;
			
			// Restore the locked state of the data handle.
			HSetState(compressedData, hState);
			if(anErr != noErr) return anErr;
		} // end gShowWindow
	} // end big for loop!

	// Close the compression sequence. This will dispose of the image description and compressed data handles allocated by
	// SCCompressSequenceBegin.
	SCCompressSequenceEnd(ci);
	
	// Close the decompression sequence. Note that this is an Image Compression Manager call, not Standard Compression.
	CDSequenceEnd(anImageSequence);
	
	// Copy all sound tracks from the source to the destination movie. Note that we are currently not copying any other
	// tracks here (text tracks, alternate tracks and so on). We need to provide more options here later.
	anErr = QTUCopySoundTracks(aSourceMovie, aDestinationMovie);  DebugAssert(anErr == noErr);
	if(anErr != noErr) goto CleanupGeneral;
		
	// We have now finished compressing video data. Next, make this data part of our movie.
	if(aDestinationTrack)	// we have a valid destination track
	{
		short resID = 128;
		
		anErr = EndMediaEdits(aDestinationMedia); ReturnIfError(anErr);
		
		// Insert the newly created media into the newly created track at the beginning of the track and lasting
		// for the entire duration of the media. The media rate is 1.0 for normal playback rate.
		InsertMediaIntoTrack(aDestinationTrack, 0, 0, GetMediaDuration(aDestinationMedia), fixed1);
		
		// Add the movie resource into the destination movie file.
		anErr = AddMovieResource(aDestinationMovie, aMovieRefNum, &resID, "\pMovie 1"); DebugAssert(anErr == noErr);
		if(anErr != noErr) goto CleanupGeneral;
		
		// Flatten the movie file just created for performance purposes. Make it crossplatform at the same time.
		CloseMovieFile(aMovieRefNum);	// note: we need to close this file as we will delete this and swap it with a temp file 
															// in the function below.
		anErr = QTUFlattenMovieFile(aDestinationMovie, &newFileFSSpec);
	}
		

// CleanupGeneral is our main entry point if we want to get rid of the window and clean up memory.	
CleanupGeneral:			
	// POSTFIX
	
	// Get Rid of the progress window
	if(gShowWindow)
	{
		DisposeWindow(progressWindow);
		progressWindow = NULL;
	}

        // CleanUpMemory is the entry point if we don't have the window displayed, but we still want to clean up memory.
        CleanupMemory:	
	// Get Rid of any buffers, handles, and other resources allocated earlier
	if(srcGWorld) DisposeGWorld(srcGWorld);		// Get rid of the GWorld handle.
	
	// Clear the test image because we disposed the pixmap it depended upon.
	SCSetTestImagePixMap(ci, NULL, NULL, 0);
	
	CloseComponent(ci);										// Close the component after use.

	return anErr;
}

// THE END