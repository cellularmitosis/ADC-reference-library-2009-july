//////////
//
//	File:		TestFunctions.c
//
//	Contains:	Test functions for changing QuickTime VR cursors.
//
//	Written by:	Tim Monroe
//
//	Copyright:	© 1997 by Apple Computer, Inc., all rights reserved.
//
//	Change History (most recent first):
//
//	   <1>	 	06/27/97	rtm		first file: added code to change some hot spot cursors
//	   
//////////

// header files
#include "TestFunctions.h"
#include "QTVRUtilities.h"


//////////
//
// A word (or two) on cursors.
//
// Each hot spot type is associated with 3 cursors:
// (1) a cursor that is displayed when it is within the hot spot (the "mouse over" cursor);
// (2) a cursor that is displayed when the mouse button is down in the hot spot (the "mouse down on" cursor);
// (3) a cursor that is displayed when the mouse button is released in the hot spot (the "mouse up on" cursor).
// 
// QuickTime VR defines cursors for several types of hot spots ('link', 'navg', 'stil', 'url ', 'misc', and 'undf').
// To my knowledge, QTVR 2.0 uses only the 'link', 'url ', and 'undf' types. You are free, I think, to create
// hots spots having those other types ('navg', 'stil', and 'misc'). To be safe, however, if you want to attach
// custom cursors to particular types of hot spots, you should probably use some other hot spot type.
// 
// You can use several techniques to attach custom cursors to specific hot spots or to specific types of hot spots.
// The easiest way is to attach the custom cursors at authoring time. This is a two-step process: first, create
// a hot spot information atom for the hot spot. One of the items of data in that atom is an array of three cursor IDs
// for cursors that are to be used instead of the default cursors supplied by QTVR. Then, create resources of type
// 'CURS' having the appropriate IDs and include those resources in the movie file. You can use this technique
// for both format 1.0 and 2.0 QuickTime VR movies. (And, of course, to change the cursors for an entire hot spot
// type, just use the same cursor IDs for all hot spots of that type in the movie.)
//
// To change cursors using the API, you can use the QTVRReplaceCursor function. You can use this function to
// change any of the cursors used by QTVR, not just the hot spot cursors. Here, however, we'll suppose that you want
// to use custom cursors for your own hot spot types. (The same ideas should apply for changing any other cursors.)
// In all likelihood, you'll want to have several types of custom hot spots. By default, QTVR uses a single triplet
// of cursors for *all* undefined hot spot types. So you'll need to replace the undefined cursors dynamically,
// based on the type of custom hot spot. The source file QTVRUtilities.c includes a function QTVRUtils_GetHotSpotType
// that returns the type of a hot spot specified by its ID. We'll use that function to get the type of a hot spot
// whenever an appropriate hot spot action occurs, and change the cursor at that time. (You could probably speed things
// up slightly by building a table of IDs and types, so you don't have to keep looking types up.)
//
//////////


//////////
//
// MyMouseOverHotSpotProc
// Change cursor when over certain types of hot spots.
//
//////////

PASCAL_RTN OSErr MyMouseOverHotSpotProc (QTVRInstance theInstance, UInt32 theHotSpotID, UInt32 theFlags, long theRefCon)
{
#pragma unused(theRefCon)

	OSType				myType;
	QTVRCursorRecord	myCursorRec;
	OSErr				myErr = noErr;

	// when we first move into a hot spot, set its cursors according to the hot spot type
	if (theFlags == kQTVRHotSpotEnter) {

		// get the type of the hot spot, given its ID
		QTVRGetHotSpotType(theInstance, theHotSpotID, &myType);

		if (myType == kQTVRHotSpotUndefinedType) {

			myCursorRec.theType = kQTVRStdCursorType;
				
			// change the mouse-over-undefined-hot-spot cursor
			myCursorRec.rsrcID = kCursID_MouseOverUndefHS;
			myCursorRec.handle = (Handle)MacGetCursor(kCursID_MouseOverUndefHS);
			QTVRReplaceCursor(theInstance, &myCursorRec);
			if (myCursorRec.handle != NULL)
				ReleaseResource(myCursorRec.handle);
			
			// change the mouse-down-on-undefined-hot-spot cursor
			myCursorRec.rsrcID = kCursID_MouseDownOnUndefHS;
			myCursorRec.handle = (Handle)MacGetCursor(kCursID_MouseDownOnUndefHS);
			QTVRReplaceCursor(theInstance, &myCursorRec);
			if (myCursorRec.handle != NULL)
				ReleaseResource(myCursorRec.handle);
			
			// change the mouse-up-on-undefined-hot-spot cursor
			myCursorRec.rsrcID = kCursID_MouseUpOnUndefHS;
			myCursorRec.handle = (Handle)MacGetCursor(kCursID_MouseUpOnUndefHS);
			QTVRReplaceCursor(theInstance, &myCursorRec);
			if (myCursorRec.handle != NULL)
				ReleaseResource(myCursorRec.handle);
		}
		
	}
	
	if (theFlags == kQTVRHotSpotLeave) {
			
			myCursorRec.theType = kQTVRUseDefaultCursor;
				
			// change the mouse-over-undefined-hot-spot cursor
			myCursorRec.rsrcID = kCursID_MouseOverUndefHS;
			myCursorRec.handle = NULL;
			QTVRReplaceCursor(theInstance, &myCursorRec);
			
			// change the mouse-down-on-undefined-hot-spot cursor
			myCursorRec.rsrcID = kCursID_MouseDownOnUndefHS;
			myCursorRec.handle = NULL;
			QTVRReplaceCursor(theInstance, &myCursorRec);
			
			// change the mouse-up-on-undefined-hot-spot cursor
			myCursorRec.rsrcID = kCursID_MouseUpOnUndefHS;
			myCursorRec.handle = NULL;
			QTVRReplaceCursor(theInstance, &myCursorRec);
	}

	return(myErr);
}

