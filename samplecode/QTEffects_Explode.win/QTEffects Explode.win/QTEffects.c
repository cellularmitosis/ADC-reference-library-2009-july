/*
	File:		QTEffects.c
	
	Contains:	Code to generate a QuickTime movie with a QuickTime video effect in it.
	
	Written by:	Scott Kuechle
				(based heavily on QuickTime SDK QTShowEffect sample code by Tim Monroe)

	Copyright:	� 1999 by Apple Computer, Inc. All rights reserved
	
	Change History (most recent first)
	
		<1>		9/25/99		srk		first file
		<2>		10/19/99	srk		changed cross fade effect to explode effect


This file defines functions that create a QuickTime movie with a video effect as a transition
from one picture to another. This project differs from the QTShowEffect sample code it is based 
on in that it allows the user to create the effect "by hand" e.g. without the aid of the standard 
QuickTime effects parameter dialog box. Instead, the program itself builds the correct effects 
parameter description atoms.

Here's a quick rundown of how the program works:

- First, a QuickTime movie with two video tracks is created - these two tracks will be used as 
sources for a explode video effect

- The effect description and sample description structures for the explode effect are created
	   
- An effects track and media are created and added to our movie for the explode effect

- The input map for the effect is created, and references for the two video tracks are added to 
this input map
		
The resulting movie, when played, will show a smooth alpha blending between the two video tracks.

*/


#ifndef __IMAGECODEC__
#include <ImageCodec.h>
#endif

#ifndef __MOVIES__
#include <Movies.h>
#endif

#ifndef __MOVIESFORMAT__
#include <MoviesFormat.h>
#endif

#ifndef __MEDIAHANDLERS__
#include <MediaHandlers.h>
#endif

#ifndef __NUMBERFORMATTING__
#include <NumberFormatting.h>
#endif

#ifndef __QUICKTIMECOMPONENTS__
#include <QuickTimeComponents.h>
#endif

#ifndef __ENDIAN__
#include <Endian.h>
#endif

#ifndef __GESTALT__
#include <Gestalt.h>
#endif

#ifndef __FIXMATH__
#include <FixMath.h>
#endif

#ifndef __MACWINDOWS__
#include <MacWindows.h>
#endif

#ifndef __PRINTING__
#include <Printing.h>
#endif

#ifndef __SCRIPT__
#include <Script.h>
#endif

#ifndef __SOUND__
#include <Sound.h>
#endif

#ifndef _STRING_H
#include <string.h>
#endif

#ifndef _STDLIB_H
#include <stdlib.h>
#endif

#ifndef __TEXTUTILS__
#include <TextUtils.h>
#endif

#ifndef __TRAPS__
#include <Traps.h>
#endif

#ifndef __RESOURCES__
#include <Resources.h>
#endif

#ifndef __FILETYPESANDCREATORS__
#include <FileTypesAndCreators.h>
#endif

#if TARGET_OS_WIN32
		// Windows headers
	#define	STRICT
	#include <windows.h>
		// QTML stuff
	#include "QTML.h"
#endif


#include "QTEffects.h"


// miscellaneous constants

#define kSourceOneName					FOUR_CHAR_CODE('srcA')
#define kSourceTwoName					FOUR_CHAR_CODE('srcB')
#define kSourceNoneName					FOUR_CHAR_CODE('srcZ')

#define kPictGWorldDepth				32

#define kPict1ResID						128
#define kPict2ResID						129

#define kPrompt							"Specify a new movie file name:"
#define kMovieFileName					"Movie File.mov"

#define kOneSecond						600
#define kEffectMovieDuration			(3 * kOneSecond)

#define kTrackDisplayWidth 				200
#define kTrackDisplayHeight 			240

#define FAIL_OSERR(y)					if (y != noErr) {goto bail;}



////////////////////
//
// QTEffects_GetPictResourceAsGWorld
// Create a new GWorld of the specified size and bit depth; then draw the specified PICT resource into it.
// The new GWorld is returned through the theGW parameter.
//
////////////////////

Movie QTEffects_CreateEffectsMovie()
{
	Movie 					theMovie = nil;
	short 					resRefNum = 0;
	short 					resId = movieInDataForkResID;	
	OSErr 					myErr = noErr;
	Track					sourceTrack1, sourceTrack2;
	ImageDescriptionHandle	mySampleDesc = NULL;
	QTAtomContainer 		theEffectDesc;
	Track					myTrack;
	Media					myMedia;
	FSSpec					movieFSSpec;

	/* Let's first create a movie file with two video tracks. Each
		video track will be used as a source for our explode effect */
	QTEffects_CreateTwoTrackMovie(	&theMovie,
									&resRefNum,
									&resId,
									&movieFSSpec,
									&sourceTrack1,
									&sourceTrack2);

	/* Setup the effect description and sample description for our explode
	effect */
	QTEffects_SetupEffectsDescription(  kExplodeTransitionType,
										&mySampleDesc,
										&theEffectDesc);

	/* Create and add an effects track and media to our movie for the explode
	effect */
	QTEffects_CreateEffectsTrackAndMedia(   theMovie,
											mySampleDesc,
											theEffectDesc,
											&myTrack,
											&myMedia);

	/* Create the input map for the effect, and add references for the two
		video tracks */
	QTEffects_CreateInputMapAndAddTrackReferences(  myTrack,
													myMedia,
													sourceTrack1,
													sourceTrack2);

	/* save our changes to the movie */
	myErr = AddMovieResource (theMovie,
								resRefNum,
								&resId,
	 							nil);
	DisposeMovie(theMovie);

	if (mySampleDesc != NULL)
		DisposeHandle((Handle)mySampleDesc);
		
	return theMovie;
}

////////////////////
//
// QTEffects_CreateTwoTrackMovie
// 
// Create a movie with two video tracks. Each track is based on our stored picture files.
// The two tracks will be used as sources for our two-source explode effect.
//
////////////////////

static void QTEffects_CreateTwoTrackMovie( Movie *theMovie,
											short	*resRefNum,
											short	*resID,
											FSSpec	*movieFSSpec,
											Track	*videoTrack1,
											Track	*videoTrack2)
{
	Point 		where = {100,100};
	SFReply 	theSFReply;
	OSErr 		myErr = noErr;
	GWorldPtr	offscreenGWorld1,offscreenGWorld2;
	

		/* Let's first create a movie file */

	SFPutFile (where, c2pstr(kPrompt), 
				c2pstr(kMovieFileName), nil, &theSFReply);
	if (!theSFReply.good) return; 

	FSMakeFSSpec(theSFReply.vRefNum, 0, theSFReply.fName, movieFSSpec);

	myErr = CreateMovieFile (movieFSSpec, 
							sigMoviePlayer,
							smCurrentScript, 
							createMovieFileDeleteCurFile | createMovieFileDontCreateResFile,
							resRefNum, 
							theMovie);
	FAIL_OSERR(myErr);
	
		/* Now we'll retrieve some stored pictures which will be used in the creation of our video tracks */ 

	myErr =  QTEffects_GetPictResourceAsGWorld (kPict1ResID, kTrackDisplayWidth, kTrackDisplayHeight, kPictGWorldDepth, &offscreenGWorld1);
	FAIL_OSERR(myErr);
	
	myErr =  QTEffects_GetPictResourceAsGWorld (kPict2ResID,  kTrackDisplayWidth, kTrackDisplayHeight, kPictGWorldDepth, &offscreenGWorld2);
	FAIL_OSERR(myErr);


		/* Now add two video tracks to our movie, based on the source pictures just retrieved. 
		
		   Note: video tracks used as sources for an effect should start at the same time as the effect
		   track and end at the same time as the effect track
		   
		*/
	   
	myErr = QTEffects_AddVideoTrackFromGWorld(theMovie, offscreenGWorld1, videoTrack1, 0, kTrackDisplayWidth, kTrackDisplayHeight);
	FAIL_OSERR(myErr);

	myErr = QTEffects_AddVideoTrackFromGWorld(theMovie, offscreenGWorld2, videoTrack2, 0, kTrackDisplayWidth, kTrackDisplayHeight);
	FAIL_OSERR(myErr);

	bail:
	;
	
}

										

////////////////////
//
// QTEffects_GetPictResourceAsGWorld
// Create a new GWorld of the specified size and bit depth; then draw the specified PICT resource into it.
// The new GWorld is returned through the theGW parameter.
//
////////////////////

static OSErr QTEffects_GetPictResourceAsGWorld (short theResID, short theWidth, short theHeight, short theDepth, GWorldPtr *theGW)
{
	PicHandle				myHandle = NULL;
	PixMapHandle			myPixMap = NULL;
	CGrafPtr				mySavedPort;
	GDHandle				mySavedDevice;
	Rect					myRect;
	OSErr					myErr = noErr;

	// get the current drawing environment
	GetGWorld(&mySavedPort, &mySavedDevice);

	// read the specified PICT resource from the application's resource file
	myHandle = GetPicture(theResID);
	if (myHandle == NULL) {
		myErr = ResError();
		if (myErr == noErr)
			myErr = resNotFound;
		goto bail;
	}

	// set the size of the GWorld
	MacSetRect(&myRect, 0, 0, theWidth, theHeight);

	// allocate a new GWorld
	myErr = QTNewGWorld(theGW, theDepth, &myRect, NULL, NULL, kICMTempThenAppMemory);
	FAIL_OSERR(myErr);
	
	SetGWorld(*theGW, NULL);

	// get a handle to the offscreen pixel image and lock it
	myPixMap = GetGWorldPixMap(*theGW);
	LockPixels(myPixMap);

	EraseRect(&myRect);
	DrawPicture(myHandle, &myRect);
	
	if (myPixMap != NULL)
		UnlockPixels(myPixMap);
	
bail:
	// restore the previous port and device
	SetGWorld(mySavedPort, mySavedDevice);

	if (myHandle != NULL)
		ReleaseResource((Handle)myHandle);
	
	return(myErr);
}


////////////////////
//
// QTEffects_AddVideoTrackFromGWorld
// Add to the specified movie a video track for the specified picture resource.
//
////////////////////

static OSErr QTEffects_AddVideoTrackFromGWorld (Movie *theMovie, GWorldPtr theGW, Track *theSourceTrack, long theStartTime, short theWidth, short theHeight)
{
	Media						myMedia;
	ImageDescriptionHandle		myDesc = NULL;
	Rect						myRect;
	long						mySize;
	Handle						myData = NULL;
	Ptr							myDataPtr = NULL;
	GWorldPtr					myGWorld = NULL;
	CGrafPtr 					mySavedPort = NULL;
	GDHandle 					mySavedGDevice = NULL;
	PicHandle					myHandle = NULL;
	PixMapHandle				mySrcPixMap = NULL;
	PixMapHandle				myDstPixMap = NULL;
	OSErr						myErr = noErr;
	
	// get the current port and device
	GetGWorld(&mySavedPort, &mySavedGDevice);
	
	// create a video track in the movie
	*theSourceTrack = NewMovieTrack(*theMovie, FixRatio(theWidth, 1), FixRatio(theHeight, 1), kNoVolume);
	myMedia = NewTrackMedia(*theSourceTrack, VideoMediaType, kOneSecond, NULL, 0);
	
	// get the rectangle for the movie
	GetMovieBox(*theMovie, &myRect);
	
	// begin editing the new track
	BeginMediaEdits(myMedia);
		
	// create a new GWorld; we draw the picture into this GWorld and then compress it
	// (note that we are creating a picture with the maximum bit depth)
	myErr = NewGWorld(&myGWorld, kPictGWorldDepth, &myRect, NULL, NULL, 0L);
	FAIL_OSERR(myErr);
	
	mySrcPixMap = GetGWorldPixMap(theGW);
	// LockPixels(mySrcPixMap);
	
	myDstPixMap = GetGWorldPixMap(myGWorld);
	LockPixels(myDstPixMap);
	
	// create a new image description; CompressImage will fill in the fields of this structure
	myDesc = (ImageDescriptionHandle)NewHandle(4);
	
	SetGWorld(myGWorld, NULL);
	
	// copy the image from the specified GWorld into the new GWorld
	CopyBits((BitMap *)(*mySrcPixMap), (BitMap *)(*myDstPixMap), &(theGW->portRect), &(myGWorld->portRect), srcCopy, 0L);

	// restore the original port and device
	SetGWorld(mySavedPort, mySavedGDevice);
	
	myErr = GetMaxCompressionSize(myDstPixMap, &myRect, 0, codecNormalQuality, kAnimationCodecType, anyCodec, &mySize);
	FAIL_OSERR(myErr);
		
	myData = NewHandle(mySize);
	FAIL_OSERR(myErr);
		
	HLockHi(myData);
	myDataPtr = StripAddress(*myData);
	
	myErr = CompressImage(myDstPixMap, &myRect, codecNormalQuality, kAnimationCodecType, myDesc, myDataPtr);
	FAIL_OSERR(myErr);
		
	myErr = AddMediaSample(myMedia, myData, 0, (**myDesc).dataSize, kEffectMovieDuration, (SampleDescriptionHandle)myDesc, 1, 0, NULL);
	FAIL_OSERR(myErr);

	myErr = EndMediaEdits(myMedia);
	FAIL_OSERR(myErr);

	myErr = InsertMediaIntoTrack(*theSourceTrack, theStartTime, 0, GetMediaDuration(myMedia), fixed1);
	
bail:
	// restore the original port and device
	SetGWorld(mySavedPort, mySavedGDevice);
	
	if (myData != NULL) {
		HUnlock(myData);
		DisposeHandle(myData);
	}

	if (myDesc != NULL)
		DisposeHandle((Handle)myDesc);
		
	// if (mySrcPixMap != NULL)
	// 	UnlockPixels(mySrcPixMap);
		
	if (myDstPixMap != NULL)
		UnlockPixels(myDstPixMap);
		
	if (myGWorld != NULL)
		DisposeGWorld(myGWorld);
	
	return(myErr);
}


////////////////////
//
// QTEffects_SetupEffectsDescription
// Create the effect description and sample description for the effect.
//
////////////////////

static void QTEffects_SetupEffectsDescription( OSType					theEffectType,
												ImageDescriptionHandle	*mySampleDesc,
												QTAtomContainer 		*theEffectDesc)
{
	// set up a new effect description
	QTEffects_CreateEffectDescription(theEffectType, kSourceOneName, kSourceTwoName, theEffectDesc);

	// set up the effects parameters for the explode effect 
	QTEffects_CreateEffectParameterForExplode(*theEffectDesc);

	// create an effect sample description
	*mySampleDesc = QTEffects_MakeSampleDescription(theEffectType, kTrackDisplayWidth, kTrackDisplayHeight);

}


////////////////////
//
// QTEffects_CreateEffectsTrackAndMedia
// Create an effects track and associated media for our movie.
//
////////////////////


static void QTEffects_CreateEffectsTrackAndMedia(Movie myMovie, 
											ImageDescriptionHandle	mySampleDesc,
											QTAtomContainer 		theEffectDesc,
											Track *myTrack,
											Media *myMedia)
{
	OSErr 		myErr;
	TimeValue	mySampleTime;


	*myTrack = NewMovieTrack(myMovie, FixRatio(kTrackDisplayWidth, 1), FixRatio(kTrackDisplayHeight, 1), kNoVolume);
	myErr = GetMoviesError();
	FAIL_OSERR(myErr);
	
	*myMedia = NewTrackMedia(*myTrack, VideoMediaType, kOneSecond, NULL, 0);
	myErr = GetMoviesError();
	FAIL_OSERR(myErr);

	// add the effect description as a sample to the effect track media
	myErr = BeginMediaEdits(*myMedia);
	FAIL_OSERR(myErr);

	myErr = AddMediaSample(*myMedia,
							(Handle)theEffectDesc,
							0,
							GetHandleSize((Handle)theEffectDesc),
							kEffectMovieDuration,
							(SampleDescriptionHandle)mySampleDesc,
							1,
							0,
							&mySampleTime);
	FAIL_OSERR(myErr);


	myErr = EndMediaEdits(*myMedia);
	FAIL_OSERR(myErr);

	// add the media to the track
	myErr = InsertMediaIntoTrack(*myTrack, 0, mySampleTime, GetMediaDuration(*myMedia), fixed1);
	FAIL_OSERR(myErr);

bail:
;


}


////////////////////
//
// QTEffects_CreateInputMapAndAddTrackReferences
// Create an input map, and add references to the input map for both
// video tracks.
//
////////////////////

static void QTEffects_CreateInputMapAndAddTrackReferences(Track effectsTrack,
													Media	effectsTrackMedia,
													Track 	sourceTrack1,
													Track 	sourceTrack2)
{
	QTAtomContainer myInputMap;
	OSErr 			myErr;


	// create the input map
	myErr = QTNewAtomContainer(&myInputMap);
	FAIL_OSERR(myErr);

	// add references for the two video tracks
	myErr = QTEffects_AddTrackReferenceToInputMap(myInputMap, effectsTrack, sourceTrack1, kSourceOneName);
	FAIL_OSERR(myErr);

	// try using existing video track as source
	myErr = QTEffects_AddTrackReferenceToInputMap(myInputMap, effectsTrack, sourceTrack2, kSourceTwoName);
	FAIL_OSERR(myErr);

	// add the input map to the effects track
	myErr = SetMediaInputMap(effectsTrackMedia, myInputMap);
	FAIL_OSERR(myErr);
	

bail:
;
	if (myInputMap != NULL)
	QTDisposeAtomContainer(myInputMap);

}



////////////////////
//
// QTEffects_CreateEffectDescription
// Create an effect description for zero, one, or two sources.
// 
// The effect description specifies which video effect is desired and the parameters for that effect.
// It also describes the source(s) for the effect. An effect description is simply an atom container
// that holds atoms with the appropriate information.
//
// Note that because we are creating an atom container, we must pass big-endian data (hence the calls
// to EndianU32_NtoB).
//
// The caller is responsible for disposing of the returned atom container, by calling QTDisposeAtomContainer.
//
////////////////////

static void QTEffects_CreateEffectDescription (OSType			theEffectName,
											  OSType			theSourceName1,
											  OSType			theSourceName2,
											  QTAtomContainer	*theEffectDesc)
{
	OSType				myType;
	OSErr				myErr = noErr;


	// create a new, empty effect description
	myErr = QTNewAtomContainer(theEffectDesc);
	FAIL_OSERR(myErr);

	// create the effect ID atom: the atom type is kParameterWhatName, and the atom ID is kParameterWhatID
	myType = EndianU32_NtoB(theEffectName);
	myErr = QTInsertChild(*theEffectDesc, kParentAtomIsContainer, kParameterWhatName, kParameterWhatID, 0, sizeof(myType), &myType, NULL);
	FAIL_OSERR(myErr);
		
	// add the first source, if it's not kSourceNoneName
	if (theSourceName1 != kSourceNoneName) {
		myType = EndianU32_NtoB(theSourceName1);
		myErr = QTInsertChild(*theEffectDesc, kParentAtomIsContainer, kEffectSourceName, 1, 0, sizeof(myType), &myType, NULL);
		FAIL_OSERR(myErr);
	}
							
	// add the second source, if it's not kSourceNoneName
	if (theSourceName2 != kSourceNoneName) {
		myType = EndianU32_NtoB(theSourceName2);
		myErr = QTInsertChild(*theEffectDesc, kParentAtomIsContainer, kEffectSourceName, 2, 0, sizeof(myType), &myType, NULL);
	}


bail:
;
}

////////////////////
//
// QTEffects_CreateEffectParameterForExplode
// Create the parameter atoms for the explode effect. This will include a
// percentage atom, along with a tween atom to tween the effect
//
////////////////////

static void QTEffects_CreateEffectParameterForExplode(QTAtomContainer myEffectDesc)
{
	Fixed	percentage[2];
	Fixed	explodeCentreX,explodeCentreY;
	OSErr	myErr = noErr;
	QTAtom	newAtom;
	OSType	tweenType;



	/* add the percentage ('pcnt') atom for the effect */
	myErr = QTInsertChild(myEffectDesc, kParentAtomIsContainer, kParameterUsagePercent, 1, 0, 0, 0, &newAtom);
	FAIL_OSERR(myErr);

	/* this atom specifies the format of the tween data */
	tweenType = EndianU32_NtoB(kParameterTypeDataFixed);
	myErr = QTInsertChild(myEffectDesc, newAtom, kTweenType, 1 , 0, sizeof(OSType), &tweenType, NULL);
	FAIL_OSERR(myErr);

	/* here's the data for the tween  */
	percentage[0] =  EndianS32_NtoB( X2Fix(0.90) );	/* 90% */
	percentage[1] =  EndianS32_NtoB( X2Fix(0.10) );	/* 10% */
	myErr = QTInsertChild(myEffectDesc, newAtom, kTweenData, 1 , 0, sizeof(long) * 2, &percentage, NULL);

	/* Add Explode Centre X atom */
	explodeCentreX = EndianS32_NtoB( X2Fix(0.0) );
	myErr = QTInsertChild(myEffectDesc, kParentAtomIsContainer, 'xcnt', 1 , 0, sizeof(long), &explodeCentreX, NULL);

	/* Add Explode Centre Y atom */
	explodeCentreY = EndianS32_NtoB( X2Fix(0.0) );
	myErr = QTInsertChild(myEffectDesc, kParentAtomIsContainer, 'ycnt', 1 , 0, sizeof(long), &explodeCentreY, NULL);

	bail:
	;
}


////////////////////
//
// QTEffects_MakeSampleDescription
// Return a new image description with default and specified values.
// 
////////////////////

static ImageDescriptionHandle QTEffects_MakeSampleDescription (OSType theEffectType, short theWidth, short theHeight)
{
	ImageDescriptionHandle		mySampleDesc = NULL;

	// create a new sample description
	mySampleDesc = (ImageDescriptionHandle)NewHandleClear(sizeof(ImageDescription));
	if (mySampleDesc == NULL)
		return(NULL);
	
	// fill in the fields of the sample description
	(**mySampleDesc).idSize = sizeof(ImageDescription);
	(**mySampleDesc).cType = theEffectType;
	(**mySampleDesc).vendor = kAppleManufacturer;
	(**mySampleDesc).temporalQuality = codecNormalQuality;
	(**mySampleDesc).spatialQuality = codecNormalQuality;
	(**mySampleDesc).width = theWidth;
	(**mySampleDesc).height = theHeight;
	(**mySampleDesc).hRes = 72L << 16;
	(**mySampleDesc).vRes = 72L << 16;
	(**mySampleDesc).frameCount = 1;
	(**mySampleDesc).depth = 0;
	(**mySampleDesc).clutID = -1;
	
	return(mySampleDesc);
}



////////////////////
//
// QTEffects_AddTrackReferenceToInputMap
// Add a track reference to the specified input map.
// 
////////////////////

static OSErr QTEffects_AddTrackReferenceToInputMap (QTAtomContainer theInputMap, Track theTrack, Track theSrcTrack, OSType theSrcName)
{
	OSErr				myErr = noErr;
	QTAtom				myInputAtom;
	long				myRefIndex;
	OSType				myType;

	myErr = AddTrackReference(theTrack, theSrcTrack, kTrackModifierReference, &myRefIndex);
	FAIL_OSERR(myErr);
			
	// add a reference atom to the input map
	myErr = QTInsertChild(theInputMap, kParentAtomIsContainer, kTrackModifierInput, myRefIndex, 0, 0, NULL, &myInputAtom);
	FAIL_OSERR(myErr);
	
	// add two child atoms to the parent reference atom
	myType = EndianU32_NtoB(kTrackModifierTypeImage);
	myErr = QTInsertChild(theInputMap, myInputAtom, kTrackModifierType, 1, 0, sizeof(myType), &myType, NULL);
	FAIL_OSERR(myErr);
	
	myType = EndianU32_NtoB(theSrcName);
	myErr = QTInsertChild(theInputMap, myInputAtom, kEffectDataSourceType, 1, 0, sizeof(myType), &myType, NULL);
		
bail:
	return(myErr);
}
 