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
#include <MacErrors.h>
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

		theTrack = NewMovieTrack (theMovie, 
								FixRatio(trackFrame.right,1),
								FixRatio(trackFrame.bottom,1), 
								kNoVolume);
		CheckError( GetMoviesError(), "NewMovieTrack error" );

		theMedia = NewTrackMedia (theTrack, VideoMediaType,
								kVideoTimeScale, /* Video Time Scale */
								nil, 0);
		CheckError( GetMoviesError(), "NewTrackMedia error" );

		err = BeginMediaEdits (theMedia);
		CheckError( err, "BeginMediaEdits error" );

		QTVideo_AddVideoSamplesToMedia (theMedia, &trackFrame);

		err = EndMediaEdits (theMedia);
		CheckError( err, "EndMediaEdits error" );

		err = InsertMediaIntoTrack (theTrack, kTrackStart,/* track start time */
									kMediaStart, /* media start time */
									GetMediaDuration (theMedia),
									fixed1);
		CheckError( err, "InsertMediaIntoTrack error" );
} 

/************************************************************
*                                                           *
*    QTVideo_AddVideoSamplesToMedia()                       *
*                                                           *
*    Creates video samples for the media in a track         *
*                                                           *
*************************************************************/

static void QTVideo_AddVideoSamplesToMedia (Media theMedia, const Rect *trackFrame)
{
	long maxCompressedSize;
	GWorldPtr theGWorld = nil;
	long curSample;
	Handle compressedData = nil;
	Ptr compressedDataPtr;
	ImageDescriptionHandle imageDesc = nil;
	CGrafPtr oldPort;
	GDHandle oldGDeviceH;
	OSErr err = noErr;



		err = NewGWorld (&theGWorld, 
						kPixelDepth,	/* pixel depth */
						trackFrame, 
						nil, 
						nil, 
						(GWorldFlags) 0 );
		CheckError (err, "NewGWorld error");

		LockPixels (theGWorld->portPixMap);
		err = GetMaxCompressionSize(theGWorld->portPixMap,
									trackFrame, 
									kMgrChoose, /* let ICM choose depth */
									codecNormalQuality, 
									kAnimationCodecType, 
									(CompressorComponent) anyCodec,
									&maxCompressedSize);
		CheckError (err, "GetMaxCompressionSize error" );

		compressedData = NewHandle(maxCompressedSize);
		CheckError( MemError(), "NewHandle error" );

		MoveHHi( compressedData );
		HLock( compressedData );
		compressedDataPtr = StripAddress( *compressedData );

		imageDesc = (ImageDescriptionHandle)NewHandle(4);
		CheckError( MemError(), "NewHandle error" );

		GetGWorld (&oldPort, &oldGDeviceH);
		SetGWorld (theGWorld, nil);

		for (curSample = 1; curSample <= kNumVideoFrames; curSample++) 
		{
			EraseRect (trackFrame);

			QTVideo_DrawFrame(trackFrame, curSample);

			err = CompressImage (theGWorld->portPixMap, 
								trackFrame, 
								codecNormalQuality,
								kAnimationCodecType,
								imageDesc, 
								compressedDataPtr );
			CheckError( err, "CompressImage error" );

			err = AddMediaSample(theMedia, 
								compressedData,
								kNoOffset,	/* no offset in data */
								(**imageDesc).dataSize, 
								kSampleDuration,	/* frame duration = 1/10 sec */
								(SampleDescriptionHandle)imageDesc, 
								kAddOneVideoSample,	/* one sample */
								kSyncSample,	/* self-contained samples */
								nil);
			CheckError( err, "AddMediaSample error" );
		}
		SetGWorld (oldPort, oldGDeviceH);

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

static void QTVideo_DrawFrame (const Rect *trackFrame, long curSample)
{
	Str255 numStr;

		ForeColor( redColor );
		PaintRect( trackFrame );

		ForeColor( blueColor );
		NumToString (curSample, numStr);
		MoveTo ( (short)(trackFrame->right / 2), (short)(trackFrame->bottom / 2));
		TextSize ( (short)(trackFrame->bottom / 3));
		DrawString (numStr);
} 
