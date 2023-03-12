//////////
//
//	File:		VRPicture.c
//
//	Contains:	Code for drawing pictures into the prescreen buffer.
//
//	Written by:	Tim Monroe
//
//	Copyright:	© 1997 by Apple Computer, Inc., all rights reserved.
//
//	Change History (most recent first):
//
//	   <5>	 	04/30/97	rtm		reworked semantics to be like sounds and movies
//	   <4>	 	03/10/97	rtm		copied file from VRLogo project and integrated with VRScript;
//									removed support for object nodes
//	   <3>	 	02/21/97	rtm		worked around a QTVR bug by adding code to node-leaving proc
//	   <2>	 	02/14/97	rtm		added support for object nodes
//	   <1>	 	01/28/97	rtm		first file
//	
// This file contains routines for drawing a picture on top of a panorama.
//
//////////

// TO DO:
// + drawing large PICTs into a small rectangle can be slow; 
//   we should cache the small image for improved performance
// + rework this to allow any kind of image file to be opened (using graphics importer routines)

//////////
//
// header files
//
//////////

#include "VRPicture.h"
#include "VRScript.h"


//////////
//
// VRPicture_ShowPicture
// Display a picture, or hide a displayed picture.
//
//////////

void VRPicture_ShowPicture (WindowObject theWindowObject, UInt32 theResID, UInt32 theEntryID, UInt32 theHeight, UInt32 theWidth, UInt32 thePegSides, UInt32 theOffset, UInt32 theOptions)
{
	ApplicationDataHdl		myAppData;
	PicHandle				myHandle;
	Boolean					myNeedShowPicture = false;
	Boolean					myNeedPictureData = false;
	VRScriptPicturePtr		myPointer = NULL;
	
	myAppData = (ApplicationDataHdl)QTFrame_GetAppDataFromWindowObject(theWindowObject);
	if (myAppData == NULL)
		return;
	
	// see if this picture is already in our list of displayed pictures				
	myPointer = (VRScriptPicturePtr)VRScript_GetObjectByEntryID(theWindowObject, kVREntry_OverlayPicture, theEntryID);
		
	if (myPointer == NULL) {
		// this picture isn't in our list yet, so we'll need to add it to the list
		myNeedPictureData = true;
		myNeedShowPicture = true;
	} else {
		// this picture is already in our list; theOptions determines how we handle this request:
		
		switch (theOptions) {
		
			case kVRMedia_TogglePause:
			case kVRMedia_ToggleStop:
			case kVRMedia_Stop:
				// stop displaying the picture
				myNeedShowPicture = false;
				myNeedPictureData = false;
				break;
				
			case kVRMedia_PlayNew:
			case kVRMedia_Restart:
			case kVRMedia_Continue:
			default:
				// show the specified picture
				myNeedShowPicture = true;
				myNeedPictureData = false;
				break;
		}
	}
	
	if (myNeedShowPicture) {
		if (myNeedPictureData) {
			myHandle = GetPicture((short)theResID);
			if (myHandle != NULL)
				myPointer = VRScript_EnlistOverlayPicture(theWindowObject, theResID, theEntryID, theHeight, theWidth, thePegSides, theOffset, myHandle, theOptions);
		}
	} else {
		if (myPointer != NULL) 
			VRScript_DelistEntry(theWindowObject, (VRScriptGenericPtr)myPointer);
	}
}


///////////
//
// VRPicture_DrawNodePictures
// Draw the selected picture into the prescreen buffer.
//
//////////

void VRPicture_DrawNodePictures (WindowObject theWindowObject)
{
	ApplicationDataHdl		myAppData;
	VRScriptPicturePtr		myPointer;

	// get the application-specific data associated with the window
	myAppData = (ApplicationDataHdl)QTFrame_GetAppDataFromWindowObject(theWindowObject);
	if (myAppData == NULL)
		return;
	
	// walk our linked list and draw any overlay pictures for this node
	myPointer = (VRScriptPicturePtr)(**myAppData).fListArray[kVREntry_OverlayPicture];
	while (myPointer != NULL) {
		if (myPointer->fResourceData != NULL) {
			Rect	myMovieRect;
			Rect	myPictRect;

			// get the current size of the movie
			GetMovieBox((**theWindowObject).fMovie, &myMovieRect);
			
			// set the size of the overlay rectangle
			if ((myPointer->fBoxHeight == kVRUseMovieHeight) && (myPointer->fBoxWidth == kVRUseMovieWidth))
				MacSetRect(&myPictRect, myMovieRect.left, myMovieRect.top, myMovieRect.right, myMovieRect.bottom);
			else
				MacSetRect(&myPictRect, 0, 0, (short)myPointer->fBoxWidth, (short)myPointer->fBoxHeight);
				
			// by default, the picture is centered in the movie rectangle
			MacOffsetRect(&myPictRect, ((myMovieRect.right - myMovieRect.left) - (myPictRect.right - myPictRect.left)) / 2, 
									((myMovieRect.bottom - myMovieRect.top) - (myPictRect.bottom - myPictRect.top)) / 2);
			
			// set the position of the overlay rectangle
			if ((myPointer->fPegSides) & kPegSide_Left) {
				MacOffsetRect(&myPictRect, (short)myPointer->fOffset, 0);
			} else if ((myPointer->fPegSides) & kPegSide_Right) {
				MacOffsetRect(&myPictRect, (myMovieRect.right - myMovieRect.left) - (myPictRect.right + myPointer->fOffset), 0);
			}
			
			if ((myPointer->fPegSides) & kPegSide_Top) {
				MacOffsetRect(&myPictRect, 0, (short)myPointer->fOffset);
			} else if ((myPointer->fPegSides) & kPegSide_Bottom) {
				MacOffsetRect(&myPictRect, 0, (myMovieRect.bottom - myMovieRect.top) - (myPictRect.bottom + (short)myPointer->fOffset));
			}
			
			// draw the picture
			DrawPicture(myPointer->fResourceData, &myPictRect);
		}
			
		myPointer = myPointer->fNextEntry;
	}

	return;
}


//////////
//
// VRPicture_DumpNodePictures
// Get rid of any overlay pictures associated with the current node.
//
//////////

void VRPicture_DumpNodePictures (WindowObject theWindowObject)
{
	VRScript_DeleteListOfType(theWindowObject, kVREntry_OverlayPicture);
}


//////////
//
// VRPicture_DumpEntryMem
// Release any memory associated with the specified list entry.
//
//////////

void VRPicture_DumpEntryMem (VRScriptPicturePtr theEntry)
{
	if (theEntry != NULL)
		ReleaseResource((Handle)(theEntry->fResourceData));
}
