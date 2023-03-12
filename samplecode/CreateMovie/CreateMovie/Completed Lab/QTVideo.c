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
	Track theTrack;
	Media theMedia;
	OSErr err = noErr;
	Rect trackFrame = {0,0,100,320};

		// 1. Create the track
		theTrack = NewMovieTrack (theMovie, 					 /* movie specifier */
								  FixRatio(trackFrame.right,1),  /* width */
								  FixRatio(trackFrame.bottom,1), /* height */
								  kNoVolume); 					 /* trackVolume */
		CheckError( GetMoviesError(), "NewMovieTrack error" );

		// 2. Create the media for the track
		theMedia = NewTrackMedia (theTrack,			/* track identifier */
								  VideoMediaType,	/* type of media */
								  kVideoTimeScale, 	/* time coordinate system */
								  nil,				/* data reference - use the file that is associated with the movie  */
								  0);				/* data reference type */
		CheckError( GetMoviesError(), "NewTrackMedia error" );

		// 3. Establish a media-editing session
		err = BeginMediaEdits (theMedia);
		CheckError( err, "BeginMediaEdits error" );

		// 3a. Add Samples to the media
		QTVideo_AddVideoSamplesToMedia (theMedia, &trackFrame);

		// 3b. End media-editing session
		err = EndMediaEdits (theMedia);
		CheckError( err, "EndMediaEdits error" );

		// 4. Insert a reference to a media segment into the track
		err = InsertMediaIntoTrack (theTrack,		/* track specifier */
									kTrackStart,	/* track start time */
									kMediaStart, 	/* media start time */
									GetMediaDuration(theMedia), /* media duration */
									fixed1);		/* media rate ((Fixed) 0x00010000L) */
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
	ImageDescriptionHandle imageDesc = nil;
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
		err = GetMaxCompressionSize(GetPortPixMap(theGWorld),		/* Handle to the source image */
									trackFrame, 					/* bounds */
									kMgrChoose, 					/* let ICM choose depth */
									codecNormalQuality,				/* desired image quality */ 
									kAnimationCodecType,			/* compressor type */ 
									(CompressorComponent)anyCodec,  /* compressor identifier */
									&maxCompressedSize);		    /* returned size */
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
			err = CompressImage(GetPortPixMap(theGWorld), /* source image to compress */
								trackFrame, 			  /* bounds */
								codecNormalQuality,		  /* desired image quality */
								kAnimationCodecType,	  /* compressor identifier */
								imageDesc, 				  /* handle to Image Description Structure; will be resized by call */
								compressedDataPtr);		  /* pointer to a location to recieve the compressed image data */
			CheckError( err, "CompressImage error" );

			// Add sample data and a description to a media
			err = AddMediaSample(theMedia,				 /* media specifier */ 
								 compressedData,		 /* handle to sample data - dataIn */
								 kNoOffset,				 /* specifies offset into data reffered to by dataIn handle */
								 (**imageDesc).dataSize, /* number of bytes of sample data to be added */ 
								 kSampleDuration,		 /* frame duration = 1/10 sec */
								 (SampleDescriptionHandle)imageDesc,	/* sample description handle */ 
								 kAddOneVideoSample,	/* number of samples */
								 kSyncSample,			/* control flag indicating self-contained samples */
								 nil);					/* returns a time value where sample was insterted */
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
