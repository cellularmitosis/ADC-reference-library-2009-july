//////////
//
//	File:		QTMultiImage.c
//
//	Contains:	Code for displaying multiple images contained in a single image file.
//
//	Written by:	Tim Monroe
//
//	Copyright:	© 1998 by Apple Computer, Inc., all rights reserved.
//
//	Change History (most recent first):
//
//	   <3>	 	03/21/00	rtm		changed QTMulti_ShowAllImagesInFile to use FSSpec parameter
//	   <2>	 	10/30/98	rtm		cleaned up code; verified on both Mac and Windows
//	   <1>	 	09/30/98	rtm		first file
//	   
//	This file contains code that illustrates how to determine whether an image file contains 
//	more than one image and how to display any of those images. This is useful for working with
//	FlashPix files (which contain multiple resolutions of an image) and PhotoShop files (which
//	store layers as separate images), among others.
//
//	The key new functions to use are GraphicsImportGetImageCount and GraphicsImportSetImageIndex.
//	The rest of the image-handling is done using graphics importer routines that have previously
//	been available.
//
//	This file defines a single function that prompts the user for an image file, determines
//	how many images are contained in that file, and then displays each such image for a short
//	period of time (2 seconds).
//
//////////

#include "QTMultiImage.h"

WindowPtr						gImageWindow = NULL;	// the window we display the image in
GraphicsImportComponent			gImporter = NULL;


//////////
//
// QTMulti_ShowAllImagesInFile
// Display all the images contained in the specified image file.
//
//////////

OSErr QTMulti_ShowAllImagesInFile (FSSpecPtr theFSSpecPtr)
{
	Rect						myRect;
	unsigned long				myCount, myIndex, myIgnore;
	OSErr						myErr = noErr;

	if (theFSSpecPtr == NULL)
		goto bail;
			
	//////////
	//
	// get a graphics importer for the image file and determine how many images are contained in it
	//
	//////////

	myErr = GetGraphicsImporterForFile(theFSSpecPtr, &gImporter);
	if (myErr != noErr)
		goto bail;
	
	myErr = GraphicsImportGetImageCount(gImporter, &myCount);
	if (myErr != noErr)
		goto bail;
	
	//////////
	//
	// loop thru all images the image file, drawing each into a window
	//
	//////////
	
	for (myIndex = 1; myIndex <= myCount; myIndex++) {
	
		// set the image index we want to display
		myErr = GraphicsImportSetImageIndex(gImporter, myIndex);
		if (myErr != noErr)
			goto bail;
		
		// determine the natural size of the image
		myErr = GraphicsImportGetNaturalBounds(gImporter, &myRect);
		if (myErr != noErr)
			goto bail;
			
		MacOffsetRect(&myRect, 50, 50);
		
		// create a window to display the image in
		gImageWindow = NewCWindow(NULL, &myRect, theFSSpecPtr->name, true, movableDBoxProc, (WindowPtr)-1L, true, 0);
		if (gImageWindow == NULL)
			goto bail;
		
		// set the current port and draw
		GraphicsImportSetGWorld(gImporter, (CGrafPtr)gImageWindow, NULL);		
		GraphicsImportDraw(gImporter);
		
		Delay(kImageDisplayTime, &myIgnore);
		DisposeWindow(gImageWindow);
	}
		
bail:
	if (myErr != noErr)
		if (gImporter != NULL)
			CloseComponent(gImporter);
		
	return(myErr);
}

