//////////
//
//	File:		SpriteUtilities.c
//
//	Contains:	Utilities for adding sprite tracks to QuickTime movies.
//
//	Written by:	Sean Allen
//	Revised by:	Chris Flick and Tim Monroe
//
//	Copyright:	© 1997-1998 by Apple Computer, Inc., all rights reserved.
//
//	Change History (most recent first):
//
//	   <4>	 	11/10/00	rtm		general clean-up to bring this file into conformance with style of other
//									sample code
//	   <3>	 	03/27/98	rtm		added error checking to AddPICTImageToKeyFrameSample to prevent crashes
//									if PICT resources not found
//	   <2>	 	03/27/98	cf		further fixes for Windows compiles
//	   <1>	 	03/26/98	rtm		made fixes for Windows compiles
//	   
//
//////////

#ifndef _SPRITEUTILITIES_
#include "SpriteUtilities.h"
#endif


//////////
//
// SpriteUtils_SetSpriteData
// Set sprite properties for non-NULL parameters, overriding or adding atoms as necessary.
//
// Keep in mind that all data in an atom container must be in big-endian format.
//
//////////

OSErr SpriteUtils_SetSpriteData (QTAtomContainer theSprite, Point *theLocation, short *theVisible, short *theLayer, short *theImageIndex, ModifierTrackGraphicsModeRecord *theGraphicsMode, StringPtr theSpriteName, QTAtomContainer theActionAtoms)
{
	QTAtom				myPropertyAtom;
	OSErr				myErr = noErr;
	
	// set the sprite location data
	if (theLocation != NULL) {
		MatrixRecord	myMatrix;
		
		SetIdentityMatrix(&myMatrix);
		myMatrix.matrix[2][0] = ((long)theLocation->h << 16);
		myMatrix.matrix[2][1] = ((long)theLocation->v << 16);
		EndianUtils_MatrixRecord_NtoB(&myMatrix);

		myPropertyAtom = QTFindChildByIndex(theSprite, kParentAtomIsContainer, kSpritePropertyMatrix, 1, NULL);
		if (myPropertyAtom == 0)
			myErr = QTInsertChild(theSprite, kParentAtomIsContainer, kSpritePropertyMatrix, 1, 0, sizeof(MatrixRecord), &myMatrix, NULL);
		else
			myErr = QTSetAtomData(theSprite, myPropertyAtom, sizeof(MatrixRecord), &myMatrix);
			
		if (myErr != noErr)
			goto bail;
	}
	
	// set the sprite visibility state
	if (theVisible != NULL) {
		short 			myVisible = *theVisible;
		
		myVisible = EndianS16_NtoB(myVisible);
		
		myPropertyAtom = QTFindChildByIndex(theSprite, kParentAtomIsContainer, kSpritePropertyVisible, 1, NULL);
		if (myPropertyAtom == 0)
			myErr = QTInsertChild(theSprite, kParentAtomIsContainer, kSpritePropertyVisible, 1, 0, sizeof(short), &myVisible, NULL);
		else
			myErr = QTSetAtomData(theSprite, myPropertyAtom, sizeof(short), &myVisible);
			
		if (myErr != noErr)
			goto bail;
	}
	
	// set the sprite layer
	if (theLayer != NULL) {
		short 			myLayer = *theLayer;
		
		myLayer = EndianS16_NtoB(myLayer);

		myPropertyAtom = QTFindChildByIndex(theSprite, 0, kSpritePropertyLayer, 1, NULL);
		if (myPropertyAtom == 0)
			myErr = QTInsertChild(theSprite, 0, kSpritePropertyLayer, 1, 0, sizeof(short), &myLayer, NULL);
		else
			myErr = QTSetAtomData(theSprite, myPropertyAtom, sizeof(short), &myLayer);
			
		if (myErr != noErr)
			goto bail;
	}
	
	// set the sprite image index
	if (theImageIndex != NULL) {
		short 			myImageIndex = *theImageIndex;

		myImageIndex = EndianS16_NtoB(myImageIndex);
		
		myPropertyAtom = QTFindChildByIndex(theSprite, kParentAtomIsContainer, kSpritePropertyImageIndex, 1, NULL);
		if (myPropertyAtom == 0)
			myErr = QTInsertChild(theSprite, kParentAtomIsContainer, kSpritePropertyImageIndex, 1, 0, sizeof(short), &myImageIndex, NULL);
		else
			myErr = QTSetAtomData(theSprite, myPropertyAtom, sizeof(short), &myImageIndex);
			
		if (myErr != noErr)
			goto bail;
	}
	
	// set the sprite graphics mode
	if (theGraphicsMode != NULL) {
		ModifierTrackGraphicsModeRecord		myGraphicsMode;
		
		myGraphicsMode.graphicsMode = EndianU32_NtoB(theGraphicsMode->graphicsMode);
		myGraphicsMode.opColor.red = EndianU16_NtoB(theGraphicsMode->opColor.red);
		myGraphicsMode.opColor.green = EndianU16_NtoB(theGraphicsMode->opColor.green);
		myGraphicsMode.opColor.blue = EndianU16_NtoB(theGraphicsMode->opColor.blue);

		myPropertyAtom = QTFindChildByIndex(theSprite, kParentAtomIsContainer, kSpritePropertyGraphicsMode, 1, NULL);
		if (myPropertyAtom == 0)
			myErr = QTInsertChild(theSprite, kParentAtomIsContainer, kSpritePropertyGraphicsMode, 1, 0, sizeof(myGraphicsMode), &myGraphicsMode, NULL);
		else
			myErr = QTSetAtomData(theSprite, myPropertyAtom, sizeof(myGraphicsMode), &myGraphicsMode);
			
		if (myErr != noErr)
			goto bail;
	}
	
	// set the sprite name
	if (theSpriteName != NULL) {
		QTAtom 		mySpriteNameAtom;
		
		mySpriteNameAtom = QTFindChildByIndex(theSprite, kParentAtomIsContainer, kSpriteNameAtomType, 1, NULL);
		if (mySpriteNameAtom == 0)
			myErr = QTInsertChild(theSprite, kParentAtomIsContainer, kSpriteNameAtomType, 1, 0, theSpriteName[0] + 1, theSpriteName, NULL);
		else
			myErr = QTSetAtomData(theSprite, mySpriteNameAtom, theSpriteName[0] + 1, theSpriteName);

		if (myErr != noErr)
			goto bail;
	}
	
	// set the action atoms
	if (theActionAtoms != NULL)
		myErr = QTInsertChildren(theSprite, kParentAtomIsContainer, theActionAtoms);
	
bail:
	if ((myErr != noErr) && (theSprite != NULL))
		QTRemoveChildren(theSprite, 0);

	return(myErr);
}


//////////
//
// SpriteUtils_AddSpriteToSample
// Add a sprite to a sample.
//
//////////

OSErr SpriteUtils_AddSpriteToSample (QTAtomContainer theSample, QTAtomContainer theSprite, QTAtomID theSpriteID)
{
	QTAtom				mySpriteAtom = 0;
	OSErr				myErr = paramErr;
	
	// see if the sample already contains a sprite atom of the specified ID
	mySpriteAtom = QTFindChildByID(theSample, kParentAtomIsContainer, kSpriteAtomType, theSpriteID, NULL);
	if (mySpriteAtom != 0)
		goto bail;
	
	// here, the index 0 means to append the sprite to the sample
	myErr = QTInsertChild(theSample, kParentAtomIsContainer, kSpriteAtomType, theSpriteID, 0, 0, NULL, &mySpriteAtom);
	if (myErr != noErr)
		goto bail;

	myErr = QTInsertChildren(theSample, mySpriteAtom, theSprite);
	
bail:
	return(myErr);
}


//////////
//
// SpriteUtils_AddSpriteSampleToMedia
// Add a sprite sample to a sprite track's media.
//
//////////

OSErr SpriteUtils_AddSpriteSampleToMedia (Media theMedia, QTAtomContainer theSample, TimeValue theDuration, Boolean isKeyFrame, TimeValue *theSampleTime)
{
	SampleDescriptionHandle 	mySampleDesc = NULL;
	OSErr						myErr = noErr;
	
	mySampleDesc = (SampleDescriptionHandle)NewHandleClear(sizeof(SpriteDescription));
	if (mySampleDesc == NULL) {
		myErr = MemError();
		goto bail;
	}
	
	myErr = AddMediaSample(theMedia,
							(Handle)theSample,
							0,
							GetHandleSize(theSample),
							theDuration,
							mySampleDesc,
							1,
							(short)(isKeyFrame ? 0 : mediaSampleNotSync),
							theSampleTime);
							
bail:
	if (mySampleDesc != NULL)
		DisposeHandle((Handle) mySampleDesc);
	return(myErr);
}


//////////
//
// SpriteUtils_AddCompressedSpriteSampleToMedia
// Add a compressed sprite sample to a sprite track's media.
//
//////////

OSErr SpriteUtils_AddCompressedSpriteSampleToMedia (Media theMedia, QTAtomContainer theSample, TimeValue theDuration, Boolean isKeyFrame, OSType theDataCompressorType, TimeValue *theSampleTime)
{
	SpriteDescriptionHandle		mySampleDesc = NULL;
	Handle						myCompressedSample = NULL;
	ComponentInstance			myComponent = NULL;
	OSErr						myErr = noErr;
	
	myErr = OpenADefaultComponent(DataCompressorComponentType, theDataCompressorType, &myComponent);
	if (myErr != noErr)
		goto bail;

	mySampleDesc = (SpriteDescriptionHandle)NewHandleClear(sizeof(SpriteDescription));
	if (mySampleDesc == NULL) {
		myErr = MemError();
		goto bail;
	}
	
	if (myComponent != NULL) {
		UInt32					myCompressBufferSize, myActualCompressedSize, myDecompressSlop = 0;
		UInt32					myUncompressedSize;
		SignedByte 				mySaveState = HGetState(theSample);
		
		myErr = (OSErr)DataCodecGetCompressBufferSize(myComponent, GetHandleSize(theSample), &myCompressBufferSize);
		if (myErr != noErr)
			goto bail;
		
		myCompressedSample = NewHandle(sizeof(UInt32) + myCompressBufferSize);
		myErr = MemError();
		if (myErr != noErr)
			goto bail;
		
		HLockHi(theSample);
		HLockHi(myCompressedSample);
		myErr = (OSErr)DataCodecCompress(myComponent, 
										*theSample, 
										GetHandleSize(theSample), 
										*myCompressedSample + sizeof(UInt32), 		// room for size at beginning
										myCompressBufferSize, 
										&myActualCompressedSize,
										&myDecompressSlop);
		
		HSetState(theSample, mySaveState);
		HUnlock(myCompressedSample);
		
		if (myErr != noErr)
			goto bail;
		
		SetHandleSize(myCompressedSample, sizeof(UInt32) + myActualCompressedSize);
		myErr = MemError();
		if (myErr != noErr)
			goto bail;

		(**mySampleDesc).decompressorType = EndianU32_NtoB(theDataCompressorType);
	
		myUncompressedSize = GetHandleSize(theSample);
		(*(UInt32*) *myCompressedSample) = EndianU32_NtoB(myUncompressedSize);		// add uncompressed size at beginning
		
		myErr = AddMediaSample(theMedia,
								(Handle)myCompressedSample,
								0,
								GetHandleSize(myCompressedSample),
								theDuration,
								(SampleDescriptionHandle)mySampleDesc,
								1,
								(short)(isKeyFrame ? 0 : mediaSampleNotSync),
								theSampleTime);
	} else {
		myErr = AddMediaSample(theMedia,
								(Handle)theSample,
								0,
								GetHandleSize(theSample),
								theDuration,
								(SampleDescriptionHandle)mySampleDesc,
								1,
								(short)(isKeyFrame ? 0 : mediaSampleNotSync),
								theSampleTime);
	}
	
bail:
	if (myCompressedSample != NULL)
		DisposeHandle(myCompressedSample);
		
	if (mySampleDesc != NULL)
		DisposeHandle((Handle)mySampleDesc);
		
	if (myComponent != NULL)
		CloseComponent(myComponent);
	
	return(myErr);
}


//////////
//
// SpriteUtils_AddPICTImageToKeyFrameSample
// Compress a PICT with the animation compressor and add the image data to a sprite key sample's images container atom.
//
//////////

OSErr SpriteUtils_AddPICTImageToKeyFrameSample (QTAtomContainer theKeySample, short thePictID, RGBColor *theKeyColor, QTAtomID theID, FixedPoint *theRegistrationPoint, StringPtr theImageName)
{
	PicHandle				myPicture = NULL;
	Handle					myCompressedPicture = NULL;
	ImageDescriptionHandle	myImageDesc = NULL;
	OSErr					myErr = noErr;
	
	// get picture from resource
	myPicture = (PicHandle)GetPicture(thePictID);
	if (myPicture == NULL)
		myErr = resNotFound;

	if (myErr != noErr)
		goto bail;
	
	DetachResource((Handle)myPicture);
	
	// convert it to image data compressed by the animation compressor
	myErr = ICUtils_RecompressPictureWithTransparency(myPicture, theKeyColor, NULL, &myImageDesc, &myCompressedPicture);
	if (myErr != noErr)
		goto bail;

	// add it to the key sample
	HLock(myCompressedPicture);
	myErr = SpriteUtils_AddCompressedImageToKeyFrameSample(theKeySample, myImageDesc, GetHandleSize(myCompressedPicture), *myCompressedPicture, theID, theRegistrationPoint, theImageName);
	
bail:
	if (myPicture != NULL)
		KillPicture(myPicture);
		
	if (myCompressedPicture != NULL)
		DisposeHandle(myCompressedPicture);
		
	if (myImageDesc != NULL)
		DisposeHandle((Handle)myImageDesc);
		
	return(myErr);
}


//////////
//
// SpriteUtils_AddCompressedImageToKeyFrameSample
// Add compressed image data to a sprite key sample's images container atom.
//
//////////

OSErr SpriteUtils_AddCompressedImageToKeyFrameSample (QTAtomContainer theKeySample, ImageDescriptionHandle theImageDesc, long theDataSize, Ptr theCompressedDataPtr, QTAtomID theImageID, FixedPoint *theRegistrationPoint, StringPtr theImageName)
{
	Handle						myImageData = NULL;
	QTAtom						myDefaultsAtom, myImagesContainerAtom, myImageAtom;
	ImageDescriptionHandle		myImageDesc = NULL;
	OSErr						myErr = noErr;

#if TARGET_RT_LITTLE_ENDIAN
	myImageDesc = (ImageDescriptionHandle)NewHandle(GetHandleSize((Handle)theImageDesc));
	BlockMoveData(*theImageDesc, *myImageDesc, GetHandleSize((Handle)theImageDesc));
	EndianUtils_ImageDescription_NtoB(myImageDesc);
#else
	myImageDesc = theImageDesc;			// already is big endian
#endif

	// append compressed picture data to myImageDesc to obtain sprite image data
	myImageData = NewHandle(0);
	myErr = MemError();
	if (myErr != noErr)
		goto bail;
	
	myErr = HandAndHand((Handle)myImageDesc, myImageData);
	if (myErr != noErr)
		goto bail;
	
	myErr = PtrAndHand(theCompressedDataPtr, myImageData, theDataSize);
	if (myErr != noErr)
		goto bail;
	
	myDefaultsAtom = QTFindChildByIndex(theKeySample, 0, kSpriteSharedDataAtomType, 1, NULL);
	if (myDefaultsAtom == 0) {
		myErr = QTInsertChild(theKeySample, kParentAtomIsContainer, kSpriteSharedDataAtomType, 1, 0, 0, NULL, &myDefaultsAtom);
		if (myErr != noErr)
			goto bail;
	}
		
	myImagesContainerAtom = QTFindChildByIndex(theKeySample, myDefaultsAtom, kSpriteImagesContainerAtomType, 1, NULL);
	if (myImagesContainerAtom == 0) {
		myErr = QTInsertChild(theKeySample, myDefaultsAtom, kSpriteImagesContainerAtomType, 1, 0, 0, NULL, &myImagesContainerAtom);
		if (myErr != noErr)
			goto bail;
	}

	myErr = QTInsertChild(theKeySample, myImagesContainerAtom, kSpriteImageAtomType, theImageID, 0, 0, NULL, &myImageAtom);
	if (myErr != noErr)
		goto bail;

	HLock(myImageData);
	myErr = QTInsertChild(theKeySample, myImageAtom, kSpriteImageDataAtomType, 1, 0, GetHandleSize(myImageData), *myImageData, NULL);
	if (myErr != noErr)
		goto bail;
	HUnlock(myImageData);
	
	if (theRegistrationPoint != NULL) {
		FixedPoint 			myRegistrationPoint;
		
		myRegistrationPoint.x = EndianS32_NtoB(theRegistrationPoint->x);
		myRegistrationPoint.y = EndianS32_NtoB(theRegistrationPoint->y);
		
		myErr = QTInsertChild(theKeySample, myImageAtom, kSpriteImageRegistrationAtomType, 1, 0, sizeof(myRegistrationPoint), &myRegistrationPoint, NULL);
		if (myErr != noErr)
			goto bail;
	} else {
		FixedPoint 			myRegistrationPoint = { 0, 0 };		// flipping {0,0} doesn't change anything so we don't flip
		
		myErr = QTInsertChild(theKeySample, myImageAtom, kSpriteImageRegistrationAtomType, 1, 0, sizeof(myRegistrationPoint), &myRegistrationPoint, NULL);
		if (myErr != noErr)
			goto bail;
	}
	
	if (theImageName != NULL) {
		myErr = QTInsertChild(theKeySample, myImageAtom, kSpriteImageNameAtomType, 1, 0, theImageName[0], &theImageName[1], NULL);
		if (myErr != noErr)
			goto bail;
	}

bail:
#if TARGET_RT_LITTLE_ENDIAN
	if (myImageDesc != NULL)
		DisposeHandle((Handle)myImageDesc);
#else
	// myImageDesc is still theImageDesc, so don't dispose of it
#endif

	if (myImageData != NULL)
		DisposeHandle(myImageData);
		
	return(myErr);
}


//////////
//
// SpriteUtils_AssignImageGroupIDsToKeyFrame
// Assign image group IDs to the images in a key frame sample.
//
//////////

OSErr SpriteUtils_AssignImageGroupIDsToKeyFrame (QTAtomContainer theKeySample)
{
	QTAtom						myDefaultsAtom, myImagesContainerAtom;
	ImageDescriptionHandle		myFirstImageDesc = NULL;
	ImageDescriptionHandle		mySecondImageDesc = NULL;
	short						myFirstIndex, mySecondIndex, myNumImages;
	CodecType					myFirstImageType, mySecondImageType;
	long						myGroupID = 0, myTestID;
	OSErr						myErr = noErr;
	
	myDefaultsAtom = QTFindChildByIndex(theKeySample, 0, kSpriteSharedDataAtomType, 1, NULL);
	if (myDefaultsAtom == 0)
		goto bail;
	
	myImagesContainerAtom = QTFindChildByIndex(theKeySample, myDefaultsAtom, kSpriteImagesContainerAtomType, 1, NULL);
	if (myImagesContainerAtom == 0)
		goto bail;
	
	myFirstImageDesc = (ImageDescriptionHandle)NewHandle(0);
	if (myFirstImageDesc == NULL) {
		myErr = memFullErr;
		goto bail; 
	}

	mySecondImageDesc = (ImageDescriptionHandle)NewHandle(0);
	if (mySecondImageDesc == NULL) {
		myErr = memFullErr;
		goto bail; 
	}

	myNumImages = QTCountChildrenOfType(theKeySample, myImagesContainerAtom, kSpriteImageAtomType);
	for (myFirstIndex = 1; myFirstIndex <= myNumImages; myFirstIndex++) {
		myErr = SpriteUtils_SetImageGroupID(theKeySample, myImagesContainerAtom, myFirstIndex, 0);
		if (myErr != noErr)
			goto bail;
	}
	
	for (myFirstIndex = 1; myFirstIndex <= (myNumImages - 1); myFirstIndex++) {
		myErr = SpriteUtils_GetImageGroupID(theKeySample, myImagesContainerAtom, myFirstIndex, &myTestID);
		if (myErr != noErr)
			goto bail;
		
		if (myTestID == 0) {
			myGroupID++;
			myErr = SpriteUtils_SetImageGroupID(theKeySample, myImagesContainerAtom, myFirstIndex, myGroupID);
			if (myErr != noErr)
				goto bail;
			
			myErr = SpriteUtils_GetImageDescription(theKeySample, myImagesContainerAtom, myFirstIndex, myFirstImageDesc);
			myFirstImageType = (**myFirstImageDesc).cType;
			if (myErr != noErr)
				goto bail;

			for (mySecondIndex = (myFirstIndex + 1); mySecondIndex <= myNumImages; mySecondIndex++) {
				myErr = SpriteUtils_GetImageGroupID(theKeySample, myImagesContainerAtom, mySecondIndex, &myTestID);
				if (myErr != noErr)
					goto bail;
					
				if (myTestID == 0) {
					myErr = SpriteUtils_GetImageDescription(theKeySample, myImagesContainerAtom, mySecondIndex, mySecondImageDesc);
					if (myErr != noErr)
						goto bail;
					mySecondImageType = (**mySecondImageDesc).cType;
				
					if (myFirstImageType == mySecondImageType) {
						ImageSequence	mySeqID;
						Boolean			isEquivalent;
						
						myErr = DecompressSequenceBegin(&mySeqID, myFirstImageDesc, NULL, NULL, NULL, NULL, ditherCopy, (RgnHandle)NULL,  0, codecNormalQuality, anyCodec);
						if (myErr != noErr)
							goto bail;
									 
						CDSequenceEquivalentImageDescription(mySeqID, mySecondImageDesc, &isEquivalent);
						CDSequenceEnd(mySeqID);
						
						if (isEquivalent) {
							myErr = SpriteUtils_SetImageGroupID(theKeySample, myImagesContainerAtom, mySecondIndex, myGroupID);
							if (myErr != noErr)
								goto bail;
						}
					}
				}
			}
		}
	}
	
	// assign an ID to the last image
	myErr = SpriteUtils_GetImageGroupID(theKeySample, myImagesContainerAtom, myNumImages, &myTestID);
	if (myErr != noErr)
		goto bail;
		
	if (myTestID == 0) {
		myGroupID++;
		myErr = SpriteUtils_SetImageGroupID(theKeySample, myImagesContainerAtom, myNumImages, myGroupID);
	}
	
bail:
	if (myFirstImageDesc != NULL)
		DisposeHandle((Handle)myFirstImageDesc);
		
	if (mySecondImageDesc != NULL)
		DisposeHandle((Handle)mySecondImageDesc);
		
	return(myErr);
}


//////////
//
// SpriteUtils_AssignImageGroupIDsToKeyFrame
// Assign image group IDs to the images in a key frame sample.
//
//////////

static OSErr SpriteUtils_GetImageDescription (QTAtomContainer theKeySample, QTAtom theImagesContainerAtom, short theImageIndex, ImageDescriptionHandle theImageDesc)
{
	QTAtom						myImageAtom, myImageDataAtom;
	UInt8						mySaveState;
	UInt32						mySize;
	OSErr						myErr = noErr;

	myImageAtom = QTFindChildByIndex(theKeySample, theImagesContainerAtom, kSpriteImageAtomType, theImageIndex, NULL);
	if (myImageAtom == 0)	{ 
		myErr = cannotFindAtomErr; 
		goto bail;
	}

	myImageDataAtom = QTFindChildByIndex(theKeySample, myImageAtom, kSpriteImageDataAtomType, 1, NULL);
	if (myImageDataAtom == 0)	{ 
		myErr = cannotFindAtomErr; 
		goto bail;
	}

	mySaveState = HGetState((Handle)theImageDesc);
	HUnlock((Handle)theImageDesc);

	// copy the data (ImageDescription followed by image data) to a handle
	myErr = QTCopyAtomDataToHandle(theKeySample, myImageDataAtom, (Handle)theImageDesc);
	if (myErr != noErr)
		goto bail;

	mySize = EndianU32_BtoN((**theImageDesc).idSize);

	// pull off anything following the image description (& its color table, if any, and any image description extensions)
	SetHandleSize((Handle)theImageDesc, mySize);

#if TARGET_RT_LITTLE_ENDIAN
	EndianUtils_ImageDescription_BtoN(theImageDesc);
#endif

	HSetState((Handle)theImageDesc, mySaveState);
	myErr = MemError();
	
bail:
	return(myErr);
}


//////////
//
// SpriteUtils_SetImageGroupID
// Set the group ID of an image group.
//
//////////

static OSErr SpriteUtils_SetImageGroupID (QTAtomContainer theKeySample, QTAtom theImagesContainerAtom, short theImageIndex, long theGroupID)
{
	QTAtom			myImageAtom, myImageGroupAtom;
	OSErr			myErr = noErr;
	
	myImageAtom = QTFindChildByIndex(theKeySample, theImagesContainerAtom, kSpriteImageAtomType, theImageIndex, NULL);
	if (myImageAtom == 0)	{ 
		myErr = cannotFindAtomErr;
		goto bail;
	}

	myImageGroupAtom = QTFindChildByIndex(theKeySample, myImageAtom, kSpriteImageGroupIDAtomType, 1, NULL);
	if (myImageGroupAtom == 0) {
		myErr = QTInsertChild(theKeySample, myImageAtom, kSpriteImageGroupIDAtomType, 1, 1, 0, NULL, &myImageGroupAtom);
		if (myErr != noErr)
			goto bail;
	}

	theGroupID = EndianU32_NtoB(theGroupID);
	myErr = QTSetAtomData(theKeySample, myImageGroupAtom, sizeof(theGroupID), &theGroupID);

bail:
	return(myErr);
}
 

//////////
//
// SpriteUtils_GetImageGroupID
// Get the group ID of an image group.
//
//////////

static OSErr SpriteUtils_GetImageGroupID (QTAtomContainer theKeySample, QTAtom theImagesContainerAtom, short theImageIndex, long *theGroupID)
{
	QTAtom			myImageAtom, myImageGroupAtom;
	OSErr			myErr = noErr;

	myImageAtom = QTFindChildByIndex(theKeySample, theImagesContainerAtom, kSpriteImageAtomType, theImageIndex, NULL);
	if (myImageAtom == 0)	{ 
		myErr = cannotFindAtomErr;
		goto bail;
	}

	myImageGroupAtom = QTFindChildByIndex(theKeySample, myImageAtom, kSpriteImageGroupIDAtomType, 1, NULL);
	
	if (myImageGroupAtom == 0)
		*theGroupID = 0;
	else {
		myErr = QTCopyAtomDataToPtr(theKeySample, myImageGroupAtom, false, sizeof(*theGroupID), (Ptr)theGroupID, NULL);
		if (myErr != noErr)
			goto bail;

		*theGroupID = EndianU32_BtoN(*theGroupID);		// return native endian long
	}
	
bail:
	return(myErr);
}
 
