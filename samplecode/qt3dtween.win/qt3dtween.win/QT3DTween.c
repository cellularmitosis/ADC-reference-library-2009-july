//////////
//
//	File:		QT3DTween.c
//
//	Contains:	QuickDraw 3D tweening support for QuickTime movies.
//
//	Written by:	Tim Monroe
//				based largely on 3DTween code by Scott Kuechle (Apple Developer Technical Support),
//				which was based on the 3DMF2AnimatedCameraMovie sample code in the QuickTime 2.5 SDK.
//
//	Copyright:	� 1997-1998 by Apple Computer, Inc., all rights reserved.
//
//	Change History (most recent first):
//
//	   <7>	 	03/17/00	rtm		made changes to get things running under CarbonLib
//	   <6>	 	09/30/98	rtm		tweaked call to AddMovieResource to create single-fork movies
//	   <5>	 	05/29/98	rtm		changed ID of kTween3dInitialCondition atom to 1; this makes
//									the rotation tweening work correctly now
//	   <4>	 	12/05/97	rtm		added rotation tween entry support; revised menus accordingly
//	   <3>	 	12/04/97	rtm		finished Windows version
//	   <2>	 	11/26/97	rtm		finished Mac version
//	   <1>	 	11/24/97	rtm		first file; revised to personal coding style
//	   
//
// This sample shows how to create a QuickTime movie that "animates" a 3D object. It does this
// by storing tweening information in a special tween media track. The same techniques can be used
// to dynamically alter other QuickTime movie tracks (such as sound tracks) by adding tween tracks
// to a movie.
// 
// 
// NOTES:
//
// *** (1) ***
// For complete information on creating tween media tracks, see the chapter "Tween Media Handler Components"
// in the QuickTime 2.5 or 3.0 Developers Guide.
//
//
// TO DO:
//
//////////

#include <Fonts.h>
#include <FixMath.h>
#include <Sound.h>

#include "QT3DTween.h"

// global variables
// these variables determine what kind of tween data we add to the movie

Boolean				gAddCameraTweenData = false;		// do we add camera tween data?
Boolean				gAddRotationTweenData = false;		// do we add rotation tween data?


//////////
//
// QT3DTween_Get3DMFFileAliasAndSize
// Have the user select a 3DMF file; return an alias to, and the size of, that file.
//
//////////

OSErr QT3DTween_Get3DMFFileAliasAndSize (Handle *theAlias, long *theSize)
{
	OSType 						myTypeList[] = {kQTFileType3DMF};
	short						myNumTypes = 1;
	FSSpec						myFSSpec;
	short						myRefNum = 0;
	AliasHandle					myAlias = NULL;
	QTFrameFileFilterUPP		myFileFilterUPP = NULL;
	OSErr						myErr = userCanceledErr;
	
#if TARGET_OS_MAC
	myNumTypes = 0;
#endif

	myFileFilterUPP = QTFrame_GetFileFilterUPP((ProcPtr)QT3DTween_FilterFiles);

	myErr = QTFrame_GetOneFileWithPreview(myNumTypes, (QTFrameTypeListPtr)myTypeList, &myFSSpec, myFileFilterUPP);
	if (myErr != noErr)
		goto bail;

	// determine the size of the data in the 3DMF file
	myErr = FSpOpenDF(&myFSSpec, fsRdPerm, &myRefNum);
	if (myErr != noErr)
		goto bail;
		
	myErr = GetEOF(myRefNum, theSize);
	if (myErr != noErr)
		goto bail;
		
	// make an alias to the 3DMF file (QuickTime calls this a "data reference")
	myErr = NewAliasMinimal(&myFSSpec, &myAlias);
		
bail:
	if (myFileFilterUPP != NULL)
		DisposeNavObjectFilterUPP(myFileFilterUPP);

	if (myRefNum != 0)
		myErr = FSClose(myRefNum);

	*theAlias = (Handle)myAlias;
	
	return(myErr);
}


//////////
//
// QT3DTween_GetFinalCameraData
// Fill in a TQ3CameraData structure with the ending camera data.
//
// This data will eventually be written to the movie file as sample data,
// so we need to make sure the data is returned in big-endian format.
//
//////////

void QT3DTween_GetFinalCameraData (TQ3CameraData *theCameraData)
{
	TQ3Point3D		myCameraFrom 	= {0.0, 0.0, 40.0};
	TQ3Point3D		myCameraTo 		= {0.0, 0.0, 0.0};
	TQ3Vector3D		myCameraUp 		= {0.0, 1.0, 0.0};
	
	// set the camera data
	theCameraData->placement.cameraLocation		= myCameraFrom;
	theCameraData->placement.pointOfInterest	= myCameraTo;
	theCameraData->placement.upVector			= myCameraUp;
	
	// convert to proper endian format
	QT3DTween_ConvertFloatToBigEndian(&theCameraData->placement.cameraLocation.x);
	QT3DTween_ConvertFloatToBigEndian(&theCameraData->placement.cameraLocation.y);
	QT3DTween_ConvertFloatToBigEndian(&theCameraData->placement.cameraLocation.z);
	
	QT3DTween_ConvertFloatToBigEndian(&theCameraData->placement.pointOfInterest.x);
	QT3DTween_ConvertFloatToBigEndian(&theCameraData->placement.pointOfInterest.y);
	QT3DTween_ConvertFloatToBigEndian(&theCameraData->placement.pointOfInterest.z);
	
	QT3DTween_ConvertFloatToBigEndian(&theCameraData->placement.upVector.x);
	QT3DTween_ConvertFloatToBigEndian(&theCameraData->placement.upVector.y);
	QT3DTween_ConvertFloatToBigEndian(&theCameraData->placement.upVector.z);
	
	theCameraData->range.yon = 1000.0;
	theCameraData->range.hither = 0.001;
	
	// convert to proper endian format
	QT3DTween_ConvertFloatToBigEndian(&theCameraData->range.yon);
	QT3DTween_ConvertFloatToBigEndian(&theCameraData->range.hither);
	
	theCameraData->viewPort.origin.x = -1.0;
	theCameraData->viewPort.origin.y = 1.0;
	theCameraData->viewPort.width = 2.0;
	theCameraData->viewPort.height = 2.0;
	
	// convert to proper endian format
	QT3DTween_ConvertFloatToBigEndian(&theCameraData->viewPort.origin.x);
	QT3DTween_ConvertFloatToBigEndian(&theCameraData->viewPort.origin.y);
	QT3DTween_ConvertFloatToBigEndian(&theCameraData->viewPort.width);
	QT3DTween_ConvertFloatToBigEndian(&theCameraData->viewPort.height);
}


//////////
//
// QT3DTween_GetStartCameraData
// Fill in a TQ3CameraData structure with the starting tween data.
//
// This data will eventually be written to the movie file as sample data,
// so we need to make sure the data is returned in big-endian format.
//
//////////

void QT3DTween_GetStartCameraData (TQ3CameraData *theCameraData)
{
	TQ3Point3D		myCameraFrom 	= {0.0, 0.0, 40.0};
	
	// reset the original camera location
	theCameraData->placement.cameraLocation = myCameraFrom;

	// set the initial placement of camera
	theCameraData->placement.cameraLocation.x *= k3DPlacementFactor;
	theCameraData->placement.cameraLocation.y *= k3DPlacementFactor;
	theCameraData->placement.cameraLocation.z *= k3DPlacementFactor;
	
	// convert to proper endian format
	QT3DTween_ConvertFloatToBigEndian(&theCameraData->placement.cameraLocation.x);
	QT3DTween_ConvertFloatToBigEndian(&theCameraData->placement.cameraLocation.y);
	QT3DTween_ConvertFloatToBigEndian(&theCameraData->placement.cameraLocation.z);
}


//////////
//
// QT3DTween_CreateTweenMovie
// Create a movie containing a 3D track and a tween track.
//
// To create a tween movie, we need to create a movie containing two tracks, a 3D data track
// and a tween media track. The 3D data track is straightforward: it contains a media sample
// that references a 3DMF file.
//
// The tween media track contains one or more tween media samples. A tween media sample is a
// QTAtomContainer that contains one or more kTweenEntry atoms. Each atom defines a separate
// tweening operation.
//
// Once we've created the 3D track and the tween track, we need to link the two tracks by in-
// serting a reference from the tween track to the 3D track. We also need to update the input
// map of the 3D track, to indicate how it should interpret the data received from the tween
// track. An input map is a QTAtomContainer that contains a kTrackModifierInput atom, which in 
// turn contains two leaf atoms, one atom of type kTrackModifierType and another atom of type
// kInputMapSubInputID.
//
//////////

OSErr QT3DTween_CreateTweenMovie (void)
{
	Handle						myHandle = NULL;
	ThreeDeeDescriptionHandle	mySampleDesc = NULL;
	short						myResRefNum = 0;
	short						myResID = movieInDataForkResID;
	Movie						myMovie = NULL;
	Track						my3DTrack, myTweenTrack;
	Media						my3DMedia, myTweenMedia;
	FSSpec						myFile;
	Boolean						myIsSelected = false;
	Boolean						myIsReplacing = false;	
	StringPtr 					myPrompt = QTUtils_ConvertCToPascalString(k3DTweenSavePrompt);
	StringPtr 					myFileName = QTUtils_ConvertCToPascalString(k3DTweenSaveMovieFileName);
	QTAtomContainer				mySample = NULL;
	QTAtomContainer				myInputMap = NULL;
	QTAtom						myAtom = 0;
	QTAtom						myInputAtom = 0;
	TQ3CameraData				myCameraData;
	TQ3RotateTransformData		myRotateData;
	long						mySize;
	long						myRefIndex;
	long						myFlags = createMovieFileDeleteCurFile | createMovieFileDontCreateResFile;
	OSErr						myErr = noErr;
	
	//////////
	//
	// create the 3D track and media, which contains a reference to the model (in 3DMF format)
	//
	//////////
	
	// get an alias to a 3DMF file, as well as the size of the data in that file
	myErr = QT3DTween_Get3DMFFileAliasAndSize(&myHandle, &mySize);
	if (myErr != noErr)
		goto bail;

	// prompt user for the destination file name
	QTFrame_PutFile(myPrompt, myFileName, &myFile, &myIsSelected, &myIsReplacing);
	if (!myIsSelected)
		goto bail;
		
	// create a movie file for the destination movie
	myErr = CreateMovieFile(&myFile, FOUR_CHAR_CODE('TVOD'), smCurrentScript, myFlags, &myResRefNum, &myMovie);
	if (myErr != noErr)
		goto bail;

	// create the 3D track and media
	my3DTrack = NewMovieTrack(myMovie, k3DMovieWidth, k3DMovieHeight, kNoVolume);
	my3DMedia = NewTrackMedia(my3DTrack, ThreeDeeMediaType, k3DTimeScale, myHandle, rAliasType);
	
	// create the sample description
	mySampleDesc = (ThreeDeeDescriptionHandle)NewHandleClear(sizeof(ThreeDeeDescription));
	if (mySampleDesc == NULL)
		goto bail;
	
	// fill in the fields of the sample description;
	// by zeroing out the structure when we created it, we get default values for all the fields;
	// (if you have non-zero values in any fields following the dataRefIndex field, make sure you
	// convert your data to big-endian format)
	(**mySampleDesc).descSize = sizeof(ThreeDeeDescription);
		
	// add the 3DMF data reference as a sample to the media
	myErr = AddMediaSampleReference(my3DMedia, 0, (unsigned long)mySize, k3DDuration, (SampleDescriptionHandle)mySampleDesc, 1, 0, NULL);
	if (myErr != noErr)
		goto bail;
	
	// insert the 3D media into the track
	myErr = InsertMediaIntoTrack(my3DTrack, 0, 0, GetMediaDuration(my3DMedia), fixed1);
	if (myErr != noErr)
		goto bail;
		
	// dispose of some things we no longer need
	DisposeHandle(myHandle);
	myHandle = NULL;
	
	DisposeHandle((Handle)mySampleDesc);
	mySampleDesc = NULL;
	
	//////////
	//
	// create the tween track and media, and a sample to contain the tween data
	//
	// NOTE: we can add as many tween entries to the tween sample as we like;
	// for present purposes, we'll illustrate adding (1) some camera tweening data and
	// (2) some object rotation tweening data.
	//
	//////////

	// create the tween track and media
	myTweenTrack = NewMovieTrack(myMovie, 0, 0, kNoVolume);
	myTweenMedia = NewTrackMedia(myTweenTrack, TweenMediaType, k3DTimeScale, NULL, 0);

	// create a new sample; this sample will hold the tween data
	myErr = QTNewAtomContainer(&mySample);
	if (myErr != noErr)
		goto bail;
		
	if (gAddCameraTweenData) {
		// add some camera tweening data to the tween media sample

		// get the camera data for the final frame
		QT3DTween_GetFinalCameraData(&myCameraData);
		
		// make a tween entry for that data;
		// a tween entry is a parent atom that contains leaf atoms describing the tweening operation
		myErr = QT3DTween_AddTweenEntryToSample(mySample, k3DCameraTweenID, kTweenType3dCameraData, &myCameraData, sizeof(myCameraData));
		if (myErr != noErr)
			goto bail;
		
		// get the camera data for the start frame
		QT3DTween_GetStartCameraData(&myCameraData);
		
		// add the initial tween data to the sample
		myErr = QT3DTween_SetTweenEntryInitialConditions(mySample, k3DCameraTweenID, &myCameraData, sizeof(myCameraData));
			
		//////////
		//
		// if you want to add other atoms to the tween entry, you can do it here;
		// for instance, you can use the following lines to make the duration of the tweening operation
		// different from the duration of the sample:
		//
		//	{
		//		TimeValue		myDuration;
		//
		//		myDuration = 1000;
		//		myErr = QT3DTween_SetTweenEntryDuration(mySample, k3DCameraTweenID, myDuration);
		//		if (myErr != noErr)
		//			goto bail;
		//	}
		//
		//////////
	
	} // if (gAddCameraTweenData)
	
	if (gAddRotationTweenData) {
		// add some object rotation data to the tween media sample
		
		// get the rotation data for the final frame
		myRotateData.axis = (TQ3Axis)EndianU32_NtoB(kQ3AxisY);
		myRotateData.radians = 2 * 3.14159;
		QT3DTween_ConvertFloatToBigEndian(&myRotateData.radians);
		myErr = QT3DTween_AddTweenEntryToSample(mySample, k3DRotationTweenID, kTweenType3dRotate, &myRotateData, sizeof(myRotateData));
		if (myErr != noErr)
			goto bail;

		// get the rotation data for the start frame
		myRotateData.axis = (TQ3Axis)EndianU32_NtoB(kQ3AxisY);
		myRotateData.radians = 0.0;
		QT3DTween_ConvertFloatToBigEndian(&myRotateData.radians);
		myErr = QT3DTween_SetTweenEntryInitialConditions(mySample, k3DRotationTweenID, &myRotateData, sizeof(myRotateData));
		if (myErr != noErr)
			goto bail;
			
	} // if (gAddRotationTweenData)
		
	// create the sample description
	mySampleDesc = (ThreeDeeDescriptionHandle)NewHandleClear(sizeof(SampleDescription));
	if (mySampleDesc == NULL)
		goto bail;
	
	(**mySampleDesc).descSize = sizeof(SampleDescription);
		
	// add the tween sample to the media
	BeginMediaEdits(myTweenMedia);
		
	myErr = AddMediaSample(myTweenMedia, mySample, 0, GetHandleSize(mySample), k3DDuration, (SampleDescriptionHandle)mySampleDesc, 1, 0, NULL);
	if (myErr != noErr)
		goto bail;
		
	EndMediaEdits(myTweenMedia);
	
	// add the media to the track
	InsertMediaIntoTrack(myTweenTrack, 0, 0, GetMediaDuration(myTweenMedia), fixed1);
	
	// dispose of some things we no longer need
	QTDisposeAtomContainer(mySample);
	mySample = NULL;
	
	DisposeHandle((Handle)mySampleDesc);
	mySampleDesc = NULL;

	//////////
	//
	// create a link between the 3D track and the tween track, and update the 3D track's input map
	//
	//////////
	
	// first, create a new atom container
	myErr = QTNewAtomContainer(&myInputMap);
	if (myErr != noErr)
		goto bail;
	
	// for *each* tween entry in the tween media sample, 	
	// create a link between the 3D track and the tween track, and add an entry to the input map
	
	if (gAddCameraTweenData) {
		myErr = AddTrackReference(my3DTrack, myTweenTrack, kTrackModifierReference, &myRefIndex);
		if (myErr != noErr)
			goto bail;
			
		myErr = QT3DTween_AddTweenEntryToInputMap(myInputMap, myRefIndex, k3DCameraTweenID, kTrackModifierCameraData, NULL);
		if (myErr != noErr)
			goto bail;
	}
	
	if (gAddRotationTweenData) {
		myErr = AddTrackReference(my3DTrack, myTweenTrack, kTrackModifierReference, &myRefIndex);
		if (myErr != noErr)
			goto bail;
			
		myErr = QT3DTween_AddTweenEntryToInputMap(myInputMap, myRefIndex, k3DRotationTweenID, kTrackModifierType3d4x4Matrix, NULL);
		if (myErr != noErr)
			goto bail;
	}

    // attach the input map to the 3D media handler
    myErr = SetMediaInputMap(my3DMedia, myInputMap);
	if (myErr != noErr)
		goto bail;
	
    // dispose of the input map
    QTDisposeAtomContainer(myInputMap);
	myInputMap = NULL;
		
	//////////
	//
	// finish up
	//
	//////////
	
	// add the movie resource to the movie file
	myErr = AddMovieResource(myMovie, myResRefNum, &myResID, NULL);

bail:
	free(myPrompt);
	free(myFileName);

	if (mySample != NULL)
		QTDisposeAtomContainer(mySample);

	if (myInputMap != NULL)
		QTDisposeAtomContainer(myInputMap);

	if (mySampleDesc != NULL)
		DisposeHandle((Handle)mySampleDesc);
	
	if (myResRefNum != 0)
		CloseMovieFile(myResRefNum);

	if (myHandle != NULL)
		DisposeHandle(myHandle);

	if (myMovie != NULL)
		DisposeMovie(myMovie);
	
	return(myErr);
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Tween utilities.
//
// Use these functions add tween entries to samples or to add attributes to tween entries.
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////

//////////
//
// QT3DTween_AddTweenEntryToSample
// Add a tween entry to the specified sample.
//
// A tween entry defines a set of values for a single tweening operation. A tween entry is a parent atom
// whose children define the tween data type, the tween data, and additional attributes of the operation.
//
// The data specified in the theData parameter is assumed to be in big-endian format.
//
//////////

OSErr QT3DTween_AddTweenEntryToSample (QTAtomContainer theSample, QTAtomID theID, QTAtomType theType, void *theData, long theDataSize)
{
	QTAtom				myAtom;
	OSErr				myErr = noErr;
	
	// create an entry for this tween in the sample
	myErr = QTInsertChild(theSample, kParentAtomIsContainer, kTweenEntry, theID, 0, 0, NULL, &myAtom);
	if (myErr != noErr)
		goto bail;
	
	// set the type of this tween entry
	// a kTweenEntry atom can contain only one kTweenType atom, whose ID and index must be 1
	theType = EndianU32_NtoB(theType);
	myErr = QTInsertChild(theSample, myAtom, kTweenType, 1, 0, sizeof(theType), &theType, NULL);
	if (myErr != noErr)
		goto bail;
	
	// set the data for this tween entry
	myErr = QTInsertChild(theSample, myAtom, kTweenData, 1, 0, theDataSize, theData, NULL);
	
bail:
	return(myErr);
}


//////////
//
// QT3DTween_AddTweenEntryToInputMap
// Add a tween entry to the specified input map.
//
//////////

OSErr QT3DTween_AddTweenEntryToInputMap (QTAtomContainer theInputMap, long theRefIndex, long theID, OSType theType, char *theName)
{
	QTAtom				myInputAtom;
	OSErr				myErr = noErr;
	
	// add an entry to the input map
	myErr = QTInsertChild(theInputMap, kParentAtomIsContainer, kTrackModifierInput, theRefIndex, 0, 0, NULL, &myInputAtom);
	if (myErr != noErr)
		goto bail;
	
	// add two child atoms to the parent atom;
	// these atoms define the type of the modifier input and the ID of the tween entry atom
	theType = EndianU32_NtoB(theType);
	myErr = QTInsertChild(theInputMap, myInputAtom, kTrackModifierType, 1, 0, sizeof(OSType), &theType, NULL);
	if (myErr != noErr)
		goto bail;
	
	theID = EndianU32_NtoB(theID);
	myErr = QTInsertChild(theInputMap, myInputAtom, kInputMapSubInputID, 1, 0, sizeof(long), &theID, NULL);
	if (myErr != noErr)
		goto bail;
		
	// set the name of the input atom
	if (theName != NULL) {
		long		myLength = 1;
		Ptr			myPtr = theName;
		UInt16		*myShort;
		
		// determine the length of the name string
		while (*myPtr++)
			myLength++;
		
		// convert the name string to the proper endian format
		myPtr = theName;
		while (*myPtr) {
			myShort = (UInt16 *)myPtr;
			*myPtr = EndianU16_NtoB(*myShort);
			myPtr = myPtr + 2;	// point to next word
		}
		
		myErr = QTInsertChild(theInputMap, myInputAtom, kTrackModifierInputName, 1, 0, myLength, theName, NULL);
	}
	
bail:
	return(myErr);
}


//////////
//
// QT3DTween_SetTweenEntryInitialConditions
// Set the initial conditions of the specified tween entry.
//
//////////

OSErr QT3DTween_SetTweenEntryInitialConditions (QTAtomContainer theSample, QTAtomID theID, void *theData, long theDataSize)
{
	QTAtom				myAtom;
	OSErr				myErr = noErr;
	
	// find the tween entry with the specified ID
	myAtom = QTFindChildByID(theSample, kParentAtomIsContainer, kTweenEntry, theID, NULL);
	if (myAtom == 0)
		return(paramErr);
	
	// add in the initial conditions data;
	// a kTweenEntry atom can contain only one kTween3dInitialCondition atom, whose ID and index must be 1
	myErr = QTInsertChild(theSample, myAtom, kTween3dInitialCondition, 1, 0, theDataSize, theData, NULL);
		
	return(myErr);
}


//////////
//
// QT3DTween_SetTweenEntryDuration
// Set the duration of the specified tween entry.
//
//////////

OSErr QT3DTween_SetTweenEntryDuration (QTAtomContainer theSample, QTAtomID theID, TimeValue theDuration)
{
	QTAtom				myAtom;
	OSErr				myErr = noErr;
	
	// find the tween entry with the specified ID
	myAtom = QTFindChildByID(theSample, kParentAtomIsContainer, kTweenEntry, theID, NULL);
	if (myAtom == 0)
		return(paramErr);
	
	// add in the duration
	theDuration = EndianS32_NtoB(theDuration);
	// a kTweenEntry atom can contain only one kTweenDuration atom, whose ID and index must be 1
	myErr = QTInsertChild(theSample, myAtom, kTweenDuration, 1, 0, sizeof(TimeValue), &theDuration, NULL);
		
	return(myErr);
}


//////////
//
// QT3DTween_SetTweenEntryStartOffset
// Set the starting offset of the specified tween entry.
//
//////////

OSErr QT3DTween_SetTweenEntryStartOffset (QTAtomContainer theSample, QTAtomID theID, TimeValue theOffset)
{
	QTAtom				myAtom;
	OSErr				myErr = noErr;
	
	// find the tween entry with the specified ID
	myAtom = QTFindChildByID(theSample, kParentAtomIsContainer, kTweenEntry, theID, NULL);
	if (myAtom == 0)
		return(paramErr);
	
	// add in the starting offset
	// a kTweenEntry atom can contain only one kTweenStartOffset atom, whose ID and index must be 1
	theOffset = EndianS32_NtoB(theOffset);
	myErr = QTInsertChild(theSample, myAtom, kTweenStartOffset, 1, 0, sizeof(TimeValue), &theOffset, NULL);
		
	return(myErr);
}


//////////
//
// QT3DTween_ConvertFloatToBigEndian
// Convert the specified floating-point number to big-endian format.
//
//////////

void QT3DTween_ConvertFloatToBigEndian (float *theFloat)
{
	unsigned long		*myLongPtr;
	
	myLongPtr = (unsigned long *)theFloat;
	*myLongPtr = EndianU32_NtoB(*myLongPtr);
}


//////////
//
// QT3DTween_FilterFiles
// Filter files for a file-opening dialog box.
//
//////////

#if TARGET_OS_MAC
PASCAL_RTN Boolean QT3DTween_FilterFiles (AEDesc *theItem, void *theInfo, void *theCallBackUD, NavFilterModes theFilterMode)
{
#pragma unused(theCallBackUD, theFilterMode)
	NavFileOrFolderInfo		*myInfo = (NavFileOrFolderInfo *)theInfo;
	
	if (theItem->descriptorType == typeFSS) {
		if (!myInfo->isFolder) {
			OSType			myType = myInfo->fileAndFolder.fileInfo.finderInfo.fdType;
			
			if (myType == kQTFileType3DMF)
				return(true);
			
			// if we got to here, it's a file we cannot open
			return(false);		
		}
	}
	
	// if we got to here, it's a folder or non-HFS object
	return(true);
}
#endif
#if TARGET_OS_WIN32
PASCAL_RTN Boolean QT3DTween_FilterFiles (CInfoPBPtr thePBPtr)
{
#pragma unused(thePBPtr)
	return(false);
}
#endif
