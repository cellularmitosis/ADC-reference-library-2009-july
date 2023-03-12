/*
	File:		QTVideo.c
	
	Contains:	Code to create video tracks for QuickTime CreateMovie sample
	
	Written by:	Scott Kuechle
				(based heavily on QuickTime sample code in Inside Macintosh: QuickTime)

	Copyright:	© 1998 by Apple Computer, Inc. All rights reserved
	
	Change History (most recent first)
	
		<1>		6/26/98		srk		first file


*/

/************************************************************
*                                                           *
*    INCLUDE FILES                                          *
*                                                           *
*************************************************************/


#include "ConditionalMacros.h"

#if TARGET_OS_WIN32
	#include <QTML.h>
#endif

#include <MacTypes.h>
#include <Errors.h>
#include <Fonts.h>
#include <QuickDraw.h>
#include <FixMath.h>
#include <Sound.h>
#include <Movies.h>
#include <ImageCompression.h>
#include <NumberFormatting.h>

#include "CreateMovie.h"
#include "QTVideo.h"

/************************************************************
*                                                           *
*    FUNCTION PROTOTYPES                                    *
*                                                           *
*************************************************************/

static void QTVideo_AddVideoSamplesToMedia (Media theMedia, const Rect *trackFrame);
static void QTVideo_DrawFrame (const Rect *trackFrame, long curSample);

/************************************************************
*                                                           *
*    CONSTANTS                                              *
*                                                           *
*************************************************************/

#define		kVideoTimeScale 	600
#define		kNumVideoFrames 	70
#define		kPixelDepth 		8	/* use 8-bit depth */
#define		kNoOffset 			0
#define		kMgrChoose			0
#define		kSyncSample 		0
#define		kAddOneVideoSample	1
#define		kSampleDuration 	60	/* frame duration = 1/10 sec */
#define		kTrackStart			0
#define		kMediaStart			0


/************************************************************
*                                                           *
*    QTVideo_CreateMyVideoTrack()                           *
*                                                           *
*    Creates a video track for a given QuickTime movie      *
*                                                           *
*************************************************************/

void QTVideo_CreateMyVideoTrack(Movie theMovie)
{

// Step 1.
// Insert "TrackMediaVars.clp" here

	OSErr err = noErr;
	Rect trackFrame = {0,0,100,320};

		// 1. Create the track

// Step 2.
// Insert "NewMovieTrack.clp" here

		CheckError( GetMoviesError(), "NewMovieTrack error" );

		// 2. Create the media for the track

// Step 3.
// Insert "NewTrackMedia.clp" here

		CheckError( GetMoviesError(), "NewTrackMedia error" );

		// 3. Establish a media-editing session

// Step 4.
// Insert "BeginMediaEdits.clp" here

		CheckError( err, "BeginMediaEdits error" );

		// 3a. Add Samples to the media
		
// Step 5.
// Build AddVideoSampleToMedia function
		QTVideo_AddVideoSamplesToMedia (theMedia, &trackFrame);

		// 3b. End media-editing session

// Step 6.
// Insert "EndMediaEdits.clp" here

		CheckError( err, "EndMediaEdits error" );

		// 4. Insert a reference to a media segment into the track

// Step 7.
// Insert "InsertMediaIntoTrack.clp" here

		CheckError( err, "InsertMediaIntoTrack error" );
} 

/************************************************************
*                                                           *
*    QTVideo_AddVideoSamplesToMedia()                       *
*                                                           *
*    Creates video samples for the media in a track         *
*                                                           *
*************************************************************/

static void QTVideo_AddVideoSamplesToMedia(Media theMedia, const Rect *trackFrame)
{
	GWorldPtr theGWorld = nil;
	long maxCompressedSize;
	long curSample;
	Handle compressedData = nil;
	Ptr compressedDataPtr;
	
// Step 1.
// Insert "ImageDescriptionVar.clp" here
	
	CGrafPtr oldPort;
	GDHandle oldGDeviceH;
	OSErr err = noErr;

		// Create a graphics world
		err = NewGWorld (&theGWorld,		/* pointer to created gworld */	
						 kPixelDepth,		/* pixel depth */
						 trackFrame, 		/* bounds */
						 nil, 				/* color table */
						 nil,				/* handle to GDevice */ 
						 (GWorldFlags)0);	/* flags */
		CheckError (err, "NewGWorld error");

		// Lock the pixels
		LockPixels (GetPortPixMap(theGWorld));
		
		// Determine the maximum size the image will be after compression.
		// Specify the compression characteristics, along with the image.

// Step 2.
// Insert "GetMaxCompressionSize.clp" here

		CheckError (err, "GetMaxCompressionSize error" );

		// Create a new handle of the right size for our compressed image data
		compressedData = NewHandle(maxCompressedSize);
		CheckError( MemError(), "NewHandle error" );

		MoveHHi( compressedData );
		HLock( compressedData );
		compressedDataPtr = *compressedData;

		// Create a handle for the Image Description Structure
		imageDesc = (ImageDescriptionHandle)NewHandle(4);
		CheckError( MemError(), "NewHandle error" );

		// Change the current graphics port to the GWorld
		GetGWorld(&oldPort, &oldGDeviceH);
		SetGWorld(theGWorld, nil);

		// For each sample...
		for (curSample = 1; curSample <= kNumVideoFrames; curSample++) {
			
			// Call DrawFrame to actually do the drawing of our image
			QTVideo_DrawFrame(trackFrame, curSample);

			// Use the ICM to compress the image 

// Step 3.
// Insert "CompressImage.clp" here

			CheckError( err, "CompressImage error" );

			// Add sample data and a description to a media

// Step 4.
// Insert "AddMediaSample.clp" here

			CheckError( err, "AddMediaSample error" );
		} // for loop
		
		SetGWorld (oldPort, oldGDeviceH);

		// Dealocate our previously alocated handles and GWorld
		if (imageDesc)
		{
			DisposeHandle ((Handle)imageDesc);
		}
		
		if (compressedData)
		{
			DisposeHandle (compressedData);
		}
		
		if (theGWorld)
		{
			DisposeGWorld (theGWorld);
		}
} 


/************************************************************
*                                                           *
*    QTVideo_DrawFrame()                                    *
*                                                           *
*    contains code to "draw" a video frame                  *
*                                                           *
*************************************************************/

static void QTVideo_DrawFrame(const Rect *trackFrame, long curSample)
{
	Str255 numStr;

		EraseRect (trackFrame);
		ForeColor( redColor );
		PaintRect( trackFrame );

		ForeColor( blueColor );
		NumToString (curSample, numStr);
		MoveTo ( (short)(trackFrame->right / 2), (short)(trackFrame->bottom / 2));
		TextSize ( (short)(trackFrame->bottom / 3));
		DrawString (numStr);
		
		if ( curSample == 70 ) {
			ForeColor( greenColor );
			MoveTo((short)(trackFrame->left + 12), (short)(trackFrame->bottom / 2 ));
			TextSize( (short)(trackFrame->bottom / 4) );
			DrawString("\pWhoa cool!");
		}
} 
