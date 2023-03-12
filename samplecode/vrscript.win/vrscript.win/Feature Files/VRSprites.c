//////////
//
//	File:		VRSprites.c
//
//	Contains:	Support for QuickTime sprite tracks in VR nodes.
//
//	Written by:	Tim Monroe
//
//	Copyright:	© 1998 by Apple Computer, Inc., all rights reserved.
//
//	Change History (most recent first):
//
//	   <2>	 	06/24/98	rtm		added VRSprites_SetLocation; general clean-up
//	   <1>	 	06/19/98	rtm		first file, based on code in QTSprites.c; added VRSprites_Set*;
//									moved to new routine names (e.g. SpriteMediaSetSpriteProperty)
//	   
//	This code supports displaying and hit-testing QuickTime sprite tracks in QTVR movies.
//
//////////

//////////
//
// header files
//	   
//////////

#include "VRSprites.h"


//////////
//
// VRSprites_InitWindowData
// Initialize window-specific data for sprites.
//
//////////

void VRSprites_InitWindowData (WindowObject theWindowObject)
{
	ApplicationDataHdl	myAppData = NULL;
	MediaHandler		myHandler = NULL;
	Track				myTrack = NULL;
	
	myAppData = (ApplicationDataHdl)QTFrame_GetAppDataFromWindowObject(theWindowObject);
	if (myAppData != NULL) {

		// look for any sprite tracks in the VR movie file
		myTrack = GetMovieIndTrackType((**theWindowObject).fMovie, 1, SpriteMediaType, movieTrackMediaType | movieTrackEnabledOnly);
		if (myTrack != NULL)
			myHandler = GetMediaHandler(GetTrackMedia(myTrack));
	
		// remember the sprite media handler
		(**myAppData).fMovieHasSprites = (myTrack != NULL);
		(**myAppData).fSpriteHandler = myHandler;
	}
}


//////////
//
// VRSprites_DumpWindowData
// Get rid of any window-specific data for sprites.
//
//////////

void VRSprites_DumpWindowData (WindowObject theWindowObject)
{
#pragma unused(theWindowObject)

}


//////////
//
// VRSprites_SetVisibleState
// Set the visibility state (on or off) of a sprite.
//
//////////

void VRSprites_SetVisibleState (WindowObject theWindowObject, QTAtomID theSpriteID, Boolean theState, UInt32 theOptions)
{
#pragma unused(theOptions)

	ApplicationDataHdl		myAppData = NULL;
	MediaHandler			myHandler = NULL;
	Boolean					isVisible;

	myAppData = (ApplicationDataHdl)QTFrame_GetAppDataFromWindowObject(theWindowObject);
	if (myAppData == NULL)
		return;

	myHandler = (**myAppData).fSpriteHandler;
	if (myHandler == NULL)
		return;

	if (theState == kVRState_Toggle) {
		SpriteMediaGetSpriteProperty(myHandler, theSpriteID, kSpritePropertyVisible, (void *)&isVisible);
		SpriteMediaSetSpriteProperty(myHandler, theSpriteID, kSpritePropertyVisible, (void *)!isVisible);
	} else {
		SpriteMediaSetSpriteProperty(myHandler, theSpriteID, kSpritePropertyVisible, (void *)(theState == kVRState_Show));
	}
}


//////////
//
// VRSprites_SetLayer
// Set the layer of a sprite.
//
//////////

void VRSprites_SetLayer (WindowObject theWindowObject, QTAtomID theSpriteID, short theLayer, UInt32 theOptions)
{
#pragma unused(theOptions)

	ApplicationDataHdl		myAppData = NULL;
	MediaHandler			myHandler = NULL;

	myAppData = (ApplicationDataHdl)QTFrame_GetAppDataFromWindowObject(theWindowObject);
	if (myAppData == NULL)
		return;

	myHandler = (**myAppData).fSpriteHandler;
	if (myHandler == NULL)
		return;
		
	SpriteMediaSetSpriteProperty(myHandler, theSpriteID, kSpritePropertyLayer, (void *)theLayer);
}


//////////
//
// VRSprites_SetGraphicsMode
// Set the graphics mode of a sprite.
//
//////////

void VRSprites_SetGraphicsMode (WindowObject theWindowObject, QTAtomID theSpriteID, long theMode, UInt32 theOptions)
{
#pragma unused(theOptions)

	ApplicationDataHdl					myAppData = NULL;
	MediaHandler						myHandler = NULL;
	ModifierTrackGraphicsModeRecord		myRec;

	myAppData = (ApplicationDataHdl)QTFrame_GetAppDataFromWindowObject(theWindowObject);
	if (myAppData == NULL)
		return;

	myHandler = (**myAppData).fSpriteHandler;
	if (myHandler == NULL)
		return;
		
	SpriteMediaGetSpriteProperty(myHandler, theSpriteID, kSpritePropertyGraphicsMode, &myRec);
	myRec.graphicsMode = theMode;
	SpriteMediaSetSpriteProperty(myHandler, theSpriteID, kSpritePropertyGraphicsMode, &myRec);
}


//////////
//
// VRSprites_SetImageIndex
// Set the image index of a sprite.
//
// Remember that the image index is the index in the entire set of sprite images in a sprite track,
// NOT (necessarily) the index of the images attached to the sprite with the specified ID. 
//
//////////

void VRSprites_SetImageIndex (WindowObject theWindowObject, QTAtomID theSpriteID, short theIndex, UInt32 theOptions)
{
#pragma unused(theOptions)

	ApplicationDataHdl		myAppData = NULL;
	MediaHandler			myHandler = NULL;

	myAppData = (ApplicationDataHdl)QTFrame_GetAppDataFromWindowObject(theWindowObject);
	if (myAppData == NULL)
		return;

	myHandler = (**myAppData).fSpriteHandler;
	if (myHandler == NULL)
		return;
		
	SpriteMediaSetSpriteProperty(myHandler, theSpriteID, kSpritePropertyImageIndex, (void *)theIndex);
}


//////////
//
// VRSprites_SetMatrix
// Set the matrix of a sprite.
//
//////////

void VRSprites_SetMatrix (WindowObject theWindowObject, QTAtomID theSpriteID, MatrixRecord *theMatrix, UInt32 theOptions)
{
#pragma unused(theOptions)

	ApplicationDataHdl		myAppData = NULL;
	MediaHandler			myHandler = NULL;

	myAppData = (ApplicationDataHdl)QTFrame_GetAppDataFromWindowObject(theWindowObject);
	if (myAppData == NULL)
		return;

	myHandler = (**myAppData).fSpriteHandler;
	if (myHandler == NULL)
		return;
		
	SpriteMediaSetSpriteProperty(myHandler, theSpriteID, kSpritePropertyMatrix, theMatrix);
}


//////////
//
// VRSprites_SetLocation
// Set the location of a sprite.
//
// The thePoint parameter specifies either the desired absolute position or the desired relative offset
// from the current position, depending on whether theOptions is kVRValue_Absolute or kVRValue_Relative.
// Values for the fields in thePoint are in pixels.
//
//////////

void VRSprites_SetLocation (WindowObject theWindowObject, QTAtomID theSpriteID, Point *thePoint, UInt32 theOptions)
{
	ApplicationDataHdl		myAppData = NULL;
	MediaHandler			myHandler = NULL;

	myAppData = (ApplicationDataHdl)QTFrame_GetAppDataFromWindowObject(theWindowObject);
	if (myAppData == NULL)
		return;

	myHandler = (**myAppData).fSpriteHandler;
	if (myHandler != NULL) {
		MatrixRecord		myMatrix;
		long				myNewH = (long)(thePoint->h);
		long				myNewV = (long)(thePoint->v);
		
		// get the current matrix for the specified sprite
		SpriteMediaGetSpriteProperty(myHandler, theSpriteID, kSpritePropertyMatrix, &myMatrix);
	
		// theOptions determines whether the new location is absolute or relative to the current location
		if (theOptions == kVRValue_Absolute) {
			myMatrix.matrix[2][0] = Long2Fix(myNewH);
			myMatrix.matrix[2][1] = Long2Fix(myNewV);
		} else {
			long			myCurrH = Fix2Long(myMatrix.matrix[2][0]);
			long			myCurrV = Fix2Long(myMatrix.matrix[2][1]);
			
			myCurrH += myNewH;
			myCurrV += myNewV;
		
			myMatrix.matrix[2][0] = Long2Fix(myCurrH);
			myMatrix.matrix[2][1] = Long2Fix(myCurrV);
		}
		
		// set the new matrix for the specified sprite	
		SpriteMediaSetSpriteProperty(myHandler, theSpriteID, kSpritePropertyMatrix, &myMatrix);
	}
}


