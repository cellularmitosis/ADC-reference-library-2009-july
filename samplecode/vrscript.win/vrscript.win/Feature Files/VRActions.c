//////////
//
//	File:		VRActions.c
//
//	Contains:	Support for reacting to QuickTime wired actions in VR nodes.
//
//	Written by:	Tim Monroe
//
//	Copyright:	© 1999 by Apple Computer, Inc., all rights reserved.
//
//	Change History (most recent first):
//
//	   <1>	 	08/05/99	rtm		first file
//	   
//	This code supports intercepting and reacting to wired actions in QuickTime VR movies.
//	Currently, we support two kinds of wired actions: (1) wired actions triggered by sprite
//	actions and (2) wired actions triggered by VR-specific actions (namely, entering a node
//	and interacting with hot spots). 
//
//////////

// header files
#include "VRActions.h"


//////////
//
// VRActions_InitWindowData
// Initialize window-specific data for wired actions.
//
//////////

void VRActions_InitWindowData (WindowObject theWindowObject)
{
	ApplicationDataHdl	myAppData = NULL;
	Track				myTrack = NULL;
	short				myCount = 0;
	QTAtomContainer		myContainer = NULL;
	QTAtom				myAtom = 0;
	OSErr				myErr = noErr;
	
	myAppData = (ApplicationDataHdl)QTFrame_GetAppDataFromWindowObject(theWindowObject);
	if (myAppData != NULL) {
	
		// assume that the movie has no wired actions of any kind
		(**myAppData).fMovieHasActions = false;

		//////////
		//
		// look for any QTVR- or sprite-specific wired actions
		//
		//////////

		// loop thru all sprite tracks in the VR movie file; 
		// **but the first time thru the loop, the track is the QTVR track**
		myCount = 0;
		myTrack = GetMovieIndTrackType((**theWindowObject).fMovie, 1, kQTVRQTVRType, movieTrackMediaType);
		while (myTrack != NULL) {
			
			// see if this track has any wired actions; we do this by looking for 
			// an atom of type kSpriteTrackPropertyHasActions in the media property atom
			
			myErr = QTNewAtomContainer(&myContainer);
			if (myErr != noErr)
				goto bail;
			
			myErr = GetMediaPropertyAtom(GetTrackMedia(myTrack), &myContainer);
			if (myErr != noErr)
				goto bail;

			// find a child of type kSpriteTrackPropertyHasActions and with ID 1
			myAtom = QTFindChildByIndex(myContainer, kParentAtomIsContainer, kSpriteTrackPropertyHasActions, 1, NULL);
			if (myAtom != 0) {
				Ptr				myData = NULL;
				long			mySize = sizeof(Boolean);
				Boolean			myHasActions = false;
				
				// get the data in this atom
				myErr = QTGetAtomDataPtr(myContainer, myAtom, &mySize, &myData);
				if (myErr != noErr)
					goto bail;
				
				// if the data is 1, then the track has wired actions
				myHasActions = (Boolean)*myData;
				if (myHasActions) {
					(**myAppData).fMovieHasActions = true;
					goto bail;
				}
			}

			QTDisposeAtomContainer(myContainer);
			myContainer = NULL;

			// get the next sprite track
			myCount++;
			myTrack = GetMovieIndTrackType((**theWindowObject).fMovie, myCount, SpriteMediaType, movieTrackMediaType | movieTrackEnabledOnly);
		}
	}
	
bail:
	if (myContainer != NULL)
		QTDisposeAtomContainer(myContainer);
}


//////////
//
// VRActions_DumpWindowData
// Get rid of any window-specific data for sprites.
//
//////////

void VRActions_DumpWindowData (WindowObject theWindowObject)
{
#pragma unused(theWindowObject)
	// nothing yet....
}
