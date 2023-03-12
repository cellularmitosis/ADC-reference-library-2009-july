//////////
//
//	File:		VRBackBuffer.c
//
//	Contains:	Sample code showing how to work with orientation of QuickTime VR back buffer.
//
//	Written by:	Ken Doyle
//	Revised by: Tim Monroe
//
//	Copyright:	© 1999 by Apple Computer, Inc., all rights reserved.
//
//	Change History (most recent first):
//
//	   <2>	 	03/20/00	rtm		made changes to get things running under CarbonLib
//	   <1>	 	06/26/99	rtm		first file 
//	   
//	This file contains functions that illustrate how to determine whether the back buffer is oriented
//	vertically or horizontally, and how to draw a picture accordingly.
//
//////////

//////////
//	   
// header files
//	   
//////////

#include "VRBackBuffer.h"


//////////
//
// VRBB_InitWindowData
// Initialize any window-specific data.
//
//////////

ApplicationDataHdl VRBB_InitWindowData (WindowObject theWindowObject)
{
#pragma unused(theWindowObject)

	ApplicationDataHdl	myAppData;
	
	myAppData = (ApplicationDataHdl)NewHandleClear(sizeof(ApplicationDataRecord));
	if (myAppData != NULL) {
		(**myAppData).fPicture = NULL;
		(**myAppData).fPictureWidth = kDefaultEmbPictWidth;
		(**myAppData).fBackBufferProc = NewQTVRBackBufferImagingUPP(VRBB_BackBufferImagingProc);
		(**myAppData).fCurrMenuItem = 0;
	}

	return(myAppData);
}


//////////
//
// VRBB_DumpWindowData
// Dispose of any window-specific data.
//
//////////

void VRBB_DumpWindowData (WindowObject theWindowObject)
{
	ApplicationDataHdl		myAppData = NULL;

	myAppData = (ApplicationDataHdl)QTFrame_GetAppDataFromWindowObject(theWindowObject);
	if (myAppData == NULL)
		return;

	// if we have a lingering picture, dump it
	if ((**myAppData).fPicture != NULL) {
		KillPicture ((**myAppData).fPicture);
		(**myAppData).fPicture = NULL;
	}
	
	DisposeQTVRBackBufferImagingUPP((**myAppData).fBackBufferProc);
}
	
	
//////////
//
// VRBB_InstallBackBufferImagingProc
// Install a back buffer imaging procedure.
//
//////////

OSErr VRBB_InstallBackBufferImagingProc (QTVRInstance theInstance, WindowObject theWindowObject, short thePictResID, Boolean theIsHoriz)
{
	ApplicationDataHdl		myAppData;
	QTVRAreaOfInterest		myArea;
	float					myWidth, myHeight;
	Rect					myPictRect;
	OSErr					myErr = noErr;

	// initialize; clean up any existing back buffer procedure
	if ((theInstance == NULL) || (theWindowObject == NULL)) 
		return(paramErr);

	myAppData = (ApplicationDataHdl)QTFrame_GetAppDataFromWindowObject(theWindowObject);
	if (myAppData == NULL) 
		return(paramErr);

	HLock((Handle)myAppData);
	
	VRBB_CleanUpBackBuffer(theWindowObject);

	// get the picture to embed	
	(**myAppData).fPicture = GetPicture(thePictResID);
	if ((**myAppData).fPicture == NULL) {
		VRBB_CleanUpBackBuffer(theWindowObject);
		goto bail;
	}

	DetachResource((Handle)(**myAppData).fPicture);
	
	myPictRect = (*(**myAppData).fPicture)->picFrame;
#if TARGET_OS_WIN32
	myPictRect.top = EndianU16_BtoN(myPictRect.top);
	myPictRect.left = EndianU16_BtoN(myPictRect.left);
	myPictRect.bottom = EndianU16_BtoN(myPictRect.bottom);
	myPictRect.right = EndianU16_BtoN(myPictRect.right);
#endif
	
	MacOffsetRect(&myPictRect, -myPictRect.left, -myPictRect.top);

	// set the area of interest	
	myWidth = kDefaultEmbPictWidth;
	
	if (theIsHoriz)
		myHeight = myWidth * (((float)myPictRect.bottom) / ((float)myPictRect.right));
	else
		myHeight = myWidth * (((float)myPictRect.right) / ((float)myPictRect.bottom));

	// center the picture on the current pan and tilt angles
	myArea.panAngle = QTVRGetPanAngle(theInstance) + (myWidth/2);
	myArea.tiltAngle = QTVRGetTiltAngle(theInstance) + (myHeight/2);

	myArea.width = myWidth;
	myArea.height = myHeight;

	// if the back buffer is oriented horizontally, set the appropriate flag
	myArea.flags = theIsHoriz ? kQTVRBackBufferHorizontal : 0;

	// install our procedure
	myErr = QTVRSetBackBufferImagingProc(theInstance, (**myAppData).fBackBufferProc, 1, &myArea, (SInt32)theWindowObject);

bail:
	HUnlock((Handle)myAppData);
	return(myErr);
}
	

//////////
//
// VRBB_CleanUpBackBuffer
//
//////////

OSErr VRBB_CleanUpBackBuffer (WindowObject theWindowObject)
{
	ApplicationDataHdl		myAppData;
	OSErr					myErr = paramErr;

	if (theWindowObject == NULL) 
		goto bail;
	
	myAppData = (ApplicationDataHdl)QTFrame_GetAppDataFromWindowObject(theWindowObject);
	if (myAppData == NULL)
		goto bail;
	
	// if we have a lingering picture, dump it
	if ((**myAppData).fPicture != NULL) {
		KillPicture ((**myAppData).fPicture);
		(**myAppData).fPicture = NULL;
	}
	
	// clear any back-buffer procedure
	myErr = QTVRSetBackBufferImagingProc((**theWindowObject).fInstance, NULL, 0, NULL, 0);

	// make sure the back buffer is clean
	myErr = QTVRRefreshBackBuffer((**theWindowObject).fInstance, 0);
	
bail:
	return(myErr);
}


//////////
//
// VRBB_BackBufferImagingProc
// The back buffer imaging procedure: draw the current picture into the back buffer.
//
//////////

PASCAL_RTN OSErr VRBB_BackBufferImagingProc (QTVRInstance theInstance, Rect *theRect, UInt16 theAreaIndex, UInt32 theFlagsIn, UInt32 *theFlagsOut, long theRefCon)
{
#pragma unused(theAreaIndex)
	WindowObject			myWindowObject = (WindowObject)theRefCon;
	ApplicationDataHdl		myAppData = NULL;
	Boolean					myIsDrawing = theFlagsIn & kQTVRBackBufferRectVisible;
	OSErr					myErr = paramErr;
	
	// assume we're not going to draw anything
	*theFlagsOut = 0;
	
	if ((theInstance == NULL) || (myWindowObject == NULL)) 
		goto bail;

	myAppData = (ApplicationDataHdl)QTFrame_GetAppDataFromWindowObject(myWindowObject);
	if (myAppData == NULL) 
		goto bail;

	if (myIsDrawing) {
		if ((**myAppData).fPicture != NULL) {
			DrawPicture((**myAppData).fPicture, theRect);
			*theFlagsOut = kQTVRBackBufferFlagDidDraw;		// if we drew something, tell QuickTime VR
		}
	}
	
	myErr = noErr;
	
bail:	
	return(myErr);
}


//////////
//
// VRBB_IsBackBufferHorizontal
// Is the back buffer oriented horizontally?
//
//////////

Boolean VRBB_IsBackBufferHorizontal (QTVRInstance theInstance)
{
	UInt32		myGeometry;
	UInt16		myResolution;
	UInt32		myCachePixelFormat;
	SInt16		myCacheSize;
	OSErr		myErr = noErr;

	if (theInstance == NULL)
		return(false);

	myErr = QTVRGetBackBufferSettings(theInstance, &myGeometry, &myResolution, &myCachePixelFormat, &myCacheSize);
	if (myErr != noErr)
		return(false);
	else
		return(myGeometry == kQTVRHorizontalCylinder);
}



