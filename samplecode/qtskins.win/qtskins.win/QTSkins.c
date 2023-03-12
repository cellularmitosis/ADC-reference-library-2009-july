//////////
//
//	File:		QTSkins.c
//
//	Contains:	Sample code for using QuickTime's skins.
//
//	Written by:	Tim Monroe
//
//	Copyright:	� 2000 by Apple Computer, Inc., all rights reserved.
//
//	Change History (most recent first):
//
//	   <5>	 	12/27/00	rtm		reworked window region geometry calculations, following StupidPlayer.c
//	   <4>	 	12/06/00	rtm		further refinements
//	   <3>	 	12/06/00	rtm		added Windows support
//	   <2>	 	11/24/00	rtm		more work; now apparently working fine on Macintosh
//	   <1>	 	11/15/00	rtm		first file 
//
//////////


//////////
//	   
// header files
//	   
//////////

#include "QTSkins.h"


//////////
//	   
// global variables
//	   
//////////

extern Handle 				gValidFileTypes;							// the list of file types that our application can open
extern long					gFirstGITypeIndex;							// the index in gValidFileTypes of the first graphics importer file type

WindowDefSpec   			gDefSpec;


///////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Utilities for creating movies with skin tracks.
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////

//////////
//
// QTSkin_AddSkinTrack
// Add a skin track to the specified movie.
//
//////////

OSErr QTSkin_AddSkinTrack (Movie theMovie)
{
	Track					myTrack = NULL;			// the movie track
	Media					myMedia = NULL;			// the movie track's media
	Rect					myRect;
	MediaHandler			myHandler = NULL;
	PicHandle				myContentPic = NULL;
	PicHandle				myDragPic = NULL;
	OSErr					myErr = paramErr;

	if (theMovie == NULL)
		goto bail;
		
	// elicit the two pictures we need from the user
	myContentPic = QTSkin_GetPicHandleFromFile();
	if (myContentPic == NULL)
		goto bail;

	myDragPic = QTSkin_GetPicHandleFromFile();
	if (myDragPic == NULL)
		goto bail;
	
	// get the movie's dimensions
	GetMovieBox(theMovie, &myRect);
	MacOffsetRect(&myRect, -myRect.left, -myRect.top);
	
	// create the skin track and media
	myTrack = NewMovieTrack(theMovie, FixRatio(myRect.right, 1), FixRatio(myRect.bottom, 1), kNoVolume);
	if (myTrack == NULL)
		goto bail;
		
	myMedia = NewTrackMedia(myTrack, FOUR_CHAR_CODE('skin'), GetMovieTimeScale(theMovie), NULL, 0);
	if (myMedia == NULL)
		goto bail;

	myHandler = GetMediaHandler(myMedia);
	if (myHandler == NULL)
		goto bail;

	// add the skin content picture as public media information
	myErr = MediaSetPublicInfo(myHandler, FOUR_CHAR_CODE('skcr'), (void *)myContentPic, 0);
	if (myErr != noErr)
		goto bail;
	
	// add the skin drag picture as public media information
	myErr = MediaSetPublicInfo(myHandler, FOUR_CHAR_CODE('skdr'), (void *)myDragPic, 0);
	if (myErr != noErr)
		goto bail;
	
	// add the media to the track
	myErr = InsertMediaIntoTrack(myTrack, 0, 0, GetMediaDuration(myMedia), fixed1);

bail:
	if (myContentPic != NULL)
		KillPicture(myContentPic);

	if (myDragPic != NULL)
		KillPicture(myDragPic);

	return(myErr);
}


//////////
//
// QTSkin_GetPicHandleFromFile
// Elicit a picture file from the user and return the picture in it.
//
//////////

PicHandle QTSkin_GetPicHandleFromFile (void)
{
	OSType 					myTypeList = kQTFileTypeQuickTimeImage;		// any image file readable by GetGraphicsImporterForFile
	short					myNumTypes = 1;
	FSSpec					myPictSpec;
	QTFrameFileFilterUPP	myFilterUPP = NULL;
	GraphicsImportComponent	myImporter = NULL;
	PicHandle				myPicture = NULL;
	OSErr					myErr = noErr;

#if TARGET_OS_MAC
	myNumTypes = 0;
#endif

	// have the user select an image file
	myFilterUPP = QTFrame_GetFileFilterUPP((ProcPtr)QTSkin_FileFilterFunction);
	
	myErr = QTFrame_GetOneFileWithPreview(myNumTypes, (QTFrameTypeListPtr)&myTypeList, &myPictSpec, myFilterUPP);
	if (myErr != noErr)
		goto bail;

	// get a graphics importer for the image file
	myErr = GetGraphicsImporterForFile(&myPictSpec, &myImporter);
	if (myErr != noErr)
		goto bail;
	
	// convert the image into a PicHandle
	myErr = GraphicsImportGetAsPicture(myImporter, &myPicture);
	
bail:
	if (myFilterUPP != NULL)
		DisposeNavObjectFilterUPP(myFilterUPP);

	if (myImporter != NULL)
		CloseComponent(myImporter);

	return(myPicture);
}


//////////
//
// QTSkin_FileFilterFunction
// Filter files for a file-opening dialog box.
//
//////////

#if TARGET_OS_MAC
PASCAL_RTN Boolean QTSkin_FileFilterFunction (AEDesc *theItem, void *theInfo, void *theCallBackUD, NavFilterModes theFilterMode)
{
#pragma unused(theCallBackUD, theFilterMode)
	NavFileOrFolderInfo		*myInfo = (NavFileOrFolderInfo *)theInfo;
	OSErr					myErr = noErr;
	
	if (gValidFileTypes == NULL)
		QTFrame_BuildFileTypeList();

	if (theItem->descriptorType == typeFSS)
		if (!myInfo->isFolder) {
			OSType			myType = myInfo->fileAndFolder.fileInfo.finderInfo.fdType;
			OSType			*myTypes = (OSType *)*gValidFileTypes;
			short			myCount;
			short			myIndex;
			
			// see whether the file type is in the list of image file types that QuickTime can open 
			myCount = GetHandleSize(gValidFileTypes) / sizeof(OSType);
			for (myIndex = gFirstGITypeIndex; myIndex < myCount; myIndex++)
				if (myType == myTypes[myIndex])
					return(true);

			// if we got to here, it's a file we cannot open
			return(false);		
		}

	return(true);
}
#endif
#if TARGET_OS_WIN32
PASCAL_RTN Boolean QTSkin_FileFilterFunction (CInfoPBPtr thePBPtr)
{
#pragma unused(thePBPtr)
	return(false);
}
#endif


///////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Utilities for displaying movies with skin tracks.
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////

//////////
//
// QTSkin_Init
// Perform any application-specific initialization for skinned movie support.
//
//////////

void QTSkin_Init (void)
{
	// set up the window procedure definition structure
	gDefSpec.defType = kWindowDefProcPtr;
	gDefSpec.u.defProc = NewWindowDefUPP(QTSkin_SkinWindowDef);
}


//////////
//
// QTSkin_Stop
// Perform any application-specific tear-down for skinned movie support.
//
//////////

void QTSkin_Stop (void)
{
	// dispose of the window procedure UPP
	if (gDefSpec.u.defProc != NULL)
		DisposeWindowDefUPP(gDefSpec.u.defProc);
}


//////////
//
// QTSkin_SkinWindowDef
// A custom window definition procedure (WDEF) for a skinned movie window.
//
//////////

static PASCAL_RTN long QTSkin_SkinWindowDef (short theVarCode, WindowRef theWindow, short theMessage, long theParam)
{
#pragma unused(theVarCode)

	switch (theMessage) {

		case kWindowMsgInitialize:
		case kWindowMsgCleanUp:
		case kWindowMsgDrawGrowOutline:
		case kWindowMsgDrawGrowBox:
			// nothing here
			break;
          
		case kWindowMsgDraw:
			break;
		
		case kWindowMsgHitTest: {
  			ApplicationDataHdl		myAppData = NULL;
  			Point					myPoint;	// location of cursor when mouse button pressed, in global coordinates
  			Point					myLocal;	// location of cursor when mouse button pressed, in local coordinates
			GrafPtr					myPort;
		    
			myAppData = (ApplicationDataHdl)QTFrame_GetAppDataFromWindow(QTFrame_GetWindowReferenceFromWindow(theWindow));
 			if (myAppData == NULL)
				return(wNoHit);
 				
 			// on entry, theParam contains the mouse location, in global screen coordinates
 			myPoint.v = HiWord(theParam);
			myPoint.h = LoWord(theParam);
			
			// the content and drag regions are offset relative to the window origin
			GetPort(&myPort);			
			SetPortWindowPort(theWindow);
							
			myLocal = myPoint;
			GlobalToLocal(&myLocal);
			
			MacSetPort(myPort);
			
			// look first to see if the mouse event is in the drag region;
			// it takes precedence over the content region
			if (PtInRgn(myLocal, (**myAppData).fDragRegion))
				return(wInDrag);
				
			if (PtInRgn(myLocal, (**myAppData).fContentRegion))
				return(wInContent);
			
			return(wNoHit);
		}
      
		case kWindowMsgGetFeatures:
			if (theParam != 0L)
				*(OptionBits *)theParam = kWindowCanGetWindowRegion;
			return(1);
          
		case kWindowMsgGetRegion: {
			// return the region corresponding to the specified region code, in global coordinates
			GetWindowRegionRec		*myRgnRec = (GetWindowRegionRec *)theParam;
  			ApplicationDataHdl		myAppData = NULL; 
			GrafPtr					myPort;
	  		Rect					myPortBounds;
	  		Point					myTopLeft;
	    
			myAppData = (ApplicationDataHdl)QTFrame_GetAppDataFromWindow(QTFrame_GetWindowReferenceFromWindow(theWindow));
 			if (myAppData == NULL)
 				return(noErr);
 			
 			if (myRgnRec == NULL)
 				return(noErr);
 				
			// get the top-left corner of the window, in global coordinates
 			GetPort(&myPort);			
			SetPortWindowPort(theWindow);
			
#if TARGET_API_MAC_CARBON
			GetPortBounds(GetWindowPort(theWindow), &myPortBounds);
#else
			myPortBounds = theWindow->portRect;
#endif
			myTopLeft.h = myPortBounds.left;
			myTopLeft.v = myPortBounds.top;
			LocalToGlobal(&myTopLeft);
			
			MacSetPort(myPort);

        	switch (myRgnRec->regionCode) {
  				case kWindowTitleBarRgn:
				case kWindowCloseBoxRgn:
					break;
          			
           		case kWindowDragRgn:
					MacCopyRgn((**myAppData).fDragRegion, myRgnRec->winRgn);
 					MacOffsetRgn(myRgnRec->winRgn, myTopLeft.h, myTopLeft.v);
        			break;
          			
       			case kWindowContentRgn:
					MacCopyRgn((**myAppData).fContentRegion, myRgnRec->winRgn);
  					MacOffsetRgn(myRgnRec->winRgn, myTopLeft.h, myTopLeft.v);
        			break;
          			
				case kWindowStructureRgn:
					MacCopyRgn((**myAppData).fStructRegion, myRgnRec->winRgn);
 					MacOffsetRgn(myRgnRec->winRgn, myTopLeft.h, myTopLeft.v);
					break;
          			
 				default:
					break;
      		}
			
			return(noErr);
		}
       
#if !TARGET_API_MAC_CARBON
		case kWindowMsgCalculateShape:	// aka wCalcRgns
			// this message is not used under Carbon; instead, the kWindowMsgGetRegion message is used
			break;
#endif
		
		default:
			break;
	}
  
	return(0L);
}


//////////
//
// QTSkin_CreateSkinsWindow
// Create a new window for a movie that contains a skins track.
//
// We need to call this function only on MacOS, where it is called instead of QTFrame_CreateMovieWindow;
// on Windows, QTFrame_CreateMovieWindow eventually calls QTSkin_InitWindowData, which in turn calls SetWindowRgn
// to install the custom window shape.
//
//////////

WindowReference QTSkin_CreateSkinsWindow (void)
{
	WindowPtr      			myWindow = NULL;
	WindowReference      	myWindowRef = NULL;
	Rect					myRect = {10, 60, 200, 200};

	// call CreateCustomWindow to create a window using our custom window definition procedure
	CreateCustomWindow(&gDefSpec, kDocumentWindowClass, kWindowNoAttributes, &myRect, &myWindow);
	if (myWindow != NULL) {
		// get the "window reference" for this window
		myWindowRef = QTFrame_GetWindowReferenceFromWindow(myWindow);
		
		// create a new window object associated with the new window
		QTFrame_CreateWindowObject(myWindowRef);
	}
	
	return(myWindowRef);
}


//////////
//
// QTSkin_ConvertPictureToRegion
// Get a region for the specified picture.
//
//////////

OSErr QTSkin_ConvertPictureToRegion (PicHandle thePicture, RgnHandle *theRegionPtr)
{
	Rect					myRect;
	GWorldPtr				myGWorld = NULL;
	PixMapHandle			myPixMap = NULL;
	CGrafPtr				mySavedPort = NULL;
	GDHandle				mySavedDevice = NULL;
	RgnHandle				myRegion = NULL;
	OSErr					myErr = noErr;

	if ((thePicture == NULL) || (theRegionPtr == NULL))
		return(paramErr);

	// get the current graphics port and device
	GetGWorld(&mySavedPort, &mySavedDevice);
	
	// get the bounding box of the picture
	myRect = (**thePicture).picFrame;
	myRect.bottom = EndianS16_BtoN(myRect.bottom);
	myRect.right = EndianS16_BtoN(myRect.right);

	// create a new GWorld and draw the picture into it
	myErr = QTNewGWorld(&myGWorld, k1MonochromePixelFormat, &myRect, NULL, NULL, kICMTempThenAppMemory);
	if (myGWorld == NULL)
		goto bail;
		
	SetGWorld(myGWorld, NULL);

	myPixMap = GetPortPixMap(myGWorld);
	if (myPixMap == NULL)
		goto bail;

	LockPixels(myPixMap);
	HLock((Handle)myPixMap);
	
	EraseRect(&myRect);
	DrawPicture(thePicture, &myRect);

	// create a new region and convert the pixmap into a region
	myRegion = NewRgn();
	myErr = MemError();
	if (myErr != noErr)
		goto bail;
		
	myErr = BitMapToRegion(myRegion, (BitMap *)*myPixMap);
	
bail:
	if (myErr != noErr) {
		if (myRegion != NULL) {
			DisposeRgn(myRegion);
			myRegion = NULL;
		}
	}

	if (myGWorld != NULL)
		DisposeGWorld(myGWorld);

	// restore the original graphics port and device
	SetGWorld(mySavedPort, mySavedDevice);
	
	*theRegionPtr = myRegion;
	
	return(myErr);
}


//////////
//
// QTSkin_InitWindowData
// Do any skin-specific initialization for the specified window.
//
//////////

ApplicationDataHdl QTSkin_InitWindowData (WindowObject theWindowObject)
{
	ApplicationDataHdl		myAppData = NULL;
	Track					myTrack = NULL;
	MediaHandler			myHandler = NULL;
	PicHandle				myPicture = NULL;
	RgnHandle				myRegion = NULL;
	MatrixRecord			myMatrix;
	OSErr					myErr = noErr;

	// if we already have some window data, dump it
	myAppData = (ApplicationDataHdl)QTFrame_GetAppDataFromWindowObject(theWindowObject);
 	if (myAppData != NULL)
		QTSkin_DumpWindowData(theWindowObject);

	myAppData = (ApplicationDataHdl)NewHandleClear(sizeof(ApplicationDataRecord));
	if (myAppData != NULL) {
	
		myTrack = GetMovieIndTrackType((**theWindowObject).fMovie, 1, FOUR_CHAR_CODE('skin'), movieTrackCharacteristic);
		if (myTrack != NULL) {
			myHandler = GetMediaHandler(GetTrackMedia(myTrack));
			if (myHandler != NULL) {
			
				// get the current movie matrix
				GetMovieMatrix((**theWindowObject).fMovie, &myMatrix);
				
				myPicture = (PicHandle)NewHandle(0);
				if (myPicture == NULL)
					goto bail;
					
		        // get the content region picture
		        myErr = MediaGetPublicInfo(myHandler, FOUR_CHAR_CODE('skcr'), myPicture, NULL);
		        if (myErr != noErr)
		        	goto bail;

		        // convert it to a region
		        myErr = QTSkin_ConvertPictureToRegion(myPicture, &myRegion);
		        (**myAppData).fContentRegion = myRegion;
		        if (myErr != noErr)
		        	goto bail;

		        // scale that region so the window scales with the movie
		        myErr = TransformRgn(&myMatrix, (**myAppData).fContentRegion);
		        if (myErr != noErr)
		        	goto bail;
		        	
#if TARGET_OS_WIN32
				(**myAppData).fWinHRGN = MacRegionToNativeRegion((**myAppData).fContentRegion);
				if ((**myAppData).fWinHRGN != NULL) {
					RECT		myRect;
					int			myResult;

					GetRgnBox((**myAppData).fWinHRGN, &myRect);
					// the coordinates of a window region are relative to the upper-left corner of the window
					// (not to the client area of the window)
					OffsetRgn((**myAppData).fWinHRGN, -myRect.left + GetSystemMetrics(SM_CXFRAME), -myRect.top + GetSystemMetrics(SM_CYCAPTION) + GetSystemMetrics(SM_CXFRAME));
					myResult = SetWindowRgn((**theWindowObject).fWindow, (**myAppData).fWinHRGN, true);
					if (myResult == 0) {
						// SetWindowRgn failed
						DeleteObject((**myAppData).fWinHRGN);
						(**myAppData).fWinHRGN = NULL;
		        		goto bail;
		        	}
				}
#endif

				// repeat with drag region picture
		        myErr = MediaGetPublicInfo(myHandler, FOUR_CHAR_CODE('skdr'), myPicture, NULL);
		        if (myErr != noErr)
		        	goto bail;
		        
		        // convert it to a region
		        myErr = QTSkin_ConvertPictureToRegion(myPicture, &myRegion);
		        (**myAppData).fDragRegion = myRegion;
		        if (myErr != noErr)
		        	goto bail;
		        
		        // scale that region so the window scales with the movie
		        myErr = TransformRgn(&myMatrix, (**myAppData).fDragRegion);
		        if (myErr != noErr)
		        	goto bail;
		        	
		        // copy the content region into the structure region
		        (**myAppData).fStructRegion = NewRgn();
				MacCopyRgn((**myAppData).fContentRegion, (**myAppData).fStructRegion);
	        }
        }
	}

bail:
	if (myPicture != NULL)
		KillPicture(myPicture);
		
	return(myAppData);
}


//////////
//
// QTSkin_DumpWindowData
// Do any skin-specific tear-down for the specified window.
//
//////////

void QTSkin_DumpWindowData (WindowObject theWindowObject)
{
	ApplicationDataHdl		myAppData = NULL;
		
	myAppData = (ApplicationDataHdl)QTFrame_GetAppDataFromWindowObject(theWindowObject);
	if (myAppData != NULL) {
		if ((**myAppData).fContentRegion != NULL)
			DisposeRgn((**myAppData).fContentRegion);
	
		if ((**myAppData).fDragRegion != NULL)
			DisposeRgn((**myAppData).fDragRegion);
	
		if ((**myAppData).fStructRegion != NULL)
			DisposeRgn((**myAppData).fStructRegion);

		(**myAppData).fContentRegion = NULL;
		(**myAppData).fDragRegion = NULL;
		(**myAppData).fStructRegion = NULL;
		
#if TARGET_OS_WIN32
		// according to the documentation, after passing a region to SetWindowRgn, the operating system
		// owns that region; I assume this means that we do not need to delete (**myAppData).fWinHRGN
		(**myAppData).fWinHRGN = NULL;
#endif
	
		DisposeHandle((Handle)myAppData);
		(**theWindowObject).fAppData = NULL;
	}
}


#if TARGET_OS_WIN32
//////////
//
// QTSkin_IsDragClick
// Return true if the mouse click whose coordinates are in lParam is in the drag region, false otherwise.
//
//////////

Boolean QTSkin_IsDragClick (WindowObject theWindowObject, LONG lParam)
{
	WindowObject			myWindowObject = NULL;
	ApplicationDataHdl		myAppData = NULL;
	HRGN					myRegion = NULL;
	POINT					myPoint;
	Boolean					isDragClick = false;
		
	myAppData = (ApplicationDataHdl)QTFrame_GetAppDataFromWindowObject(theWindowObject);
	if (myAppData != NULL) {
		myPoint.x = LOWORD(lParam);
		myPoint.y = HIWORD(lParam);

		myRegion = MacRegionToNativeRegion((**myAppData).fDragRegion);

		if (PtInRegion(myRegion, myPoint.x, myPoint.y))
			isDragClick = true;
			
		DeleteObject(myRegion);
	}
	
	return(isDragClick);
}
#endif


///////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Miscellaneous utilities.
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////

//////////
//
// QTSkin_IsSkinnedMovie
// Is the specified movie a skinned movie?
//
//////////

Boolean QTSkin_IsSkinnedMovie (Movie theMovie) 
{
	return(GetMovieIndTrackType(theMovie, 1, FOUR_CHAR_CODE('skin'), movieTrackCharacteristic) != NULL);
}




