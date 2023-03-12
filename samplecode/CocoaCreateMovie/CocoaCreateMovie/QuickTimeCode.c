//////////
//
//	File:		QuickTimeCode.c
//
//	Contains:	QuickTime routines to create a movie.
//
//	Written by:	Apple Developer Technical Support
//
//	Copyright:	© 2001 by Apple Computer, Inc., all rights reserved.
//
//	Change History (most recent first):
//	   
//	   <1>	 	7/18/01	srk		first file
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

//////////
//
// header files
//
//////////

#import "MyController.h"
#import <QuickTime/QuickTime.h>
#include <Carbon/Carbon.h>
#include <QuickTimeCode.h>

//////////
//
// prototypes
//
//////////

static StringPtr QTUtils_ConvertCToPascalString (char *theString);
static void CheckError(OSErr err, char *message );
static void QTVideo_CreateMyVideoTrack(Movie theMovie, Rect *trackFrame);
static void QTVideo_AddVideoSamplesToMedia(Media theMedia, const Rect *trackFrame);

//////////
//
// globals
//
//////////

void *gSelf;

//////////
//
// setObjCObject
//
// Store a reference to our ObjC object
// for the MyController class. This will allow
// us to call methods in this class
//
//////////

void setObjCObject(void *theObject)
{
    gSelf = theObject;
}

//////////
//
// CreateMovie
//
// QuickTime code to create a movie containing
// a single video track. We use the NSImages as
// the data for our video track.
//
//////////

Movie CreateMovie(Rect *trackFrame)
{
    Movie theMovie = nil;
    FSSpec mySpec;
    short resRefNum = 0;
    short resId = movieInDataForkResID;
    OSErr err = noErr;

    FSMakeFSSpec(0, 0, QTUtils_ConvertCToPascalString ("NewMovie.mov"), &mySpec);
    err = CreateMovieFile (&mySpec, 
                            kMyCreatorType,
                            smCurrentScript, 
                            createMovieFileDeleteCurFile | createMovieFileDontCreateResFile,
                            &resRefNum, 
                            &theMovie );

    QTVideo_CreateMyVideoTrack (theMovie, trackFrame);
    err = AddMovieResource (theMovie, resRefNum, &resId, QTUtils_ConvertCToPascalString ("testing"));
    if (resRefNum)
    {
        CloseMovieFile (resRefNum);
    }

    return theMovie;
}

//////////
//
// CheckError
//
//////////

static void CheckError(OSErr err, char *message )
{
    if (err != noErr)
    {
        printf(message);
    }
}

//////////
//
// QTVideo_CreateMyVideoTrack
//
// QuickTime code to create a video track
// for our movie
//
//////////

static void QTVideo_CreateMyVideoTrack(Movie theMovie, Rect *trackFrame)
{
	Track theTrack;
	Media theMedia;
	OSErr err = noErr;

		// 1. Create the track
		theTrack = NewMovieTrack (theMovie, 		/* movie specifier */
                                    FixRatio((*trackFrame).right,1),  /* width */
                                    FixRatio((*trackFrame).bottom,1), /* height */
								  kNoVolume);  /* trackVolume */
		CheckError( GetMoviesError(), "NewMovieTrack error" );

		// 2. Create the media for the track
		theMedia = NewTrackMedia (theTrack,		/* track identifier */
                                        VideoMediaType,		/* type of media */
                                        kVideoTimeScale, 	/* time coordinate system */
                                        nil,			/* data reference - use the file that is associated with the movie  */
                                        0);			/* data reference type */
		CheckError( GetMoviesError(), "NewTrackMedia error" );

		// 3. Establish a media-editing session
		err = BeginMediaEdits (theMedia);
		CheckError( err, "BeginMediaEdits error" );

		// 3a. Add Samples to the media
		QTVideo_AddVideoSamplesToMedia (theMedia, trackFrame);

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

//////////
//
// QTVideo_AddVideoSamplesToMedia
//
// Creates video samples for the media in a track
//
//////////

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
        err = NewGWorld (&theGWorld,	/* pointer to created gworld */	
                kPixelDepth,		/* pixel depth */
                trackFrame, 		/* bounds */
                nil, 			/* color table */
                nil,			/* handle to GDevice */ 
                (GWorldFlags)0);	/* flags */
        CheckError (err, "NewGWorld error");

        // Lock the pixels
        LockPixels (GetGWorldPixMap(theGWorld)/*GetPortPixMap(theGWorld)*/);

        // Determine the maximum size the image will be after compression.
        // Specify the compression characteristics, along with the image.
        err = GetMaxCompressionSize(GetGWorldPixMap(theGWorld),		/* Handle to the source image */
                            trackFrame, 				/* bounds */
                            kMgrChoose, 				/* let ICM choose depth */
                            codecNormalQuality,				/* desired image quality */ 
                            kAnimationCodecType,			/* compressor type */ 
                            (CompressorComponent)anyCodec,  		/* compressor identifier */
                            &maxCompressedSize);		    	/* returned size */
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
        for (curSample = 1; curSample <= [(MyController *)gSelf getNumberOfImages]; curSample++) 
        {
            // do the drawing of our image
            CopyNSImageToGWorld([(MyController *)gSelf getImageAtArrayIndex:((int)curSample-1)], theGWorld);

            // Use the ICM to compress the image 
            err = CompressImage(GetGWorldPixMap(theGWorld),	/* source image to compress */
                                trackFrame, 		/* bounds */
                                codecNormalQuality,	/* desired image quality */
                                kAnimationCodecType,	/* compressor identifier */
                                imageDesc, 		/* handle to Image Description Structure; will be resized by call */
                                compressedDataPtr);	/* pointer to a location to recieve the compressed image data */
            CheckError( err, "CompressImage error" );

            // Add sample data and a description to a media
            err = AddMediaSample(theMedia,	/* media specifier */ 
                    compressedData,	/* handle to sample data - dataIn */
                    kNoOffset,		/* specifies offset into data reffered to by dataIn handle */
                    (**imageDesc).dataSize, /* number of bytes of sample data to be added */ 
                    kSampleDuration,		 /* frame duration = 1/10 sec */
                    (SampleDescriptionHandle)imageDesc,	/* sample description handle */ 
                    kAddOneVideoSample,	/* number of samples */
                    kSyncSample,	/* control flag indicating self-contained samples */
                    nil);		/* returns a time value where sample was insterted */
            CheckError( err, "AddMediaSample error" );
        } // for loop
		
        UnlockPixels (GetGWorldPixMap(theGWorld)/*GetPortPixMap(theGWorld)*/);

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



//////////
//
// QTUtils_ConvertCToPascalString
// Convert a C string into a Pascal string.
//
// The caller is responsible for disposing of the pointer returned by this function (by calling free).
//
//////////

static StringPtr QTUtils_ConvertCToPascalString (char *theString)
{
	StringPtr	myString = malloc(strlen(theString) + 1);
	short		myIndex = 0;

	while (theString[myIndex] != '\0') {
		myString[myIndex + 1] = theString[myIndex];
		myIndex++;
	}
	
	myString[0] = (unsigned char)myIndex;
	
	return(myString);
}
