//////////
//
//	File:		VRMovies.c
//
//	Contains:	Support for QuickTime movie playback in VR nodes.
//
//	Written by:	Tim Monroe
//				Some code borrowed from QTVRSamplePlayer by Bryce Wolfson.
//
//	Copyright:	© 1996-1998 by Apple Computer, Inc., all rights reserved.
//
//	Change History (most recent first):
//
//	   <16>	 	03/20/00	rtm		made changes to get things running under CarbonLib
//	   <15>	 	06/23/99	rtm		added VRMoov_MacRGBToWinRGB and VRMoov_WinRGBToMacRGB
//	   <14>	 	06/22/99	rtm		more work on matrices; now everything works okay (AFAICT)
//	   <13>	 	06/21/99	rtm		added call to UpdateMovie to back buffer imaging procedure
//	   <12>	 	06/18/99	rtm		got movie matrices working correctly; added color picking to Windows
//	   <11>	 	06/17/99	rtm		reworked geometry of back buffer yet again; now we copy from the
//									offscreen GWorld to back buffer using use DecompressSequenceFrameS
//	   <10>	 	05/16/99	rtm		added VRMoov_DumpWindowData; further work on geometry of back buffer
//	   <9>	 	05/15/99	rtm		reworked geometry handling to support horizontal back buffer in QT 4.0
//	   <8>	 	05/04/98	rtm		added automatic rotation of movie
//	   <7>	 	03/06/97	rtm		started to implement video masking
//	   <6>	 	03/05/97	rtm		added VRMoov_SetChromaColor; added fChromaColor to app data record
//	   <5>	 	03/04/97	rtm		fixed compositing problems at back buffer edges
//	   <4>	 	03/03/97	rtm		added VRMoov_SetVideoGraphicsMode to handle compositing
//									without using an offscreen GWorld
//	   <3>	 	02/27/97	rtm		further development: borrowed some ideas from QTVRSamplePlayer;
//									added VRMoov_SetEmbeddedMovieWidth etc.
//	   <2>	 	12/12/96	rtm		further development: borrowed some ideas from BoxMoov demo 
//	   <1>	 	12/11/96	rtm		first file 
//	   
// This code draws the QuickTime movie frames into the back buffer, either directly
// or indirectly (via an offscreen GWorld). Direct drawing gives the best performance,
// but indirect drawing is necessary for some visual effects. 
//
//////////

// TO DO:
// + finish video masking by custom 'hide' hot spots (so video goes *behind* such hot spots)
// + verify that everything works okay if *images* are used instead of movies (scaling seems to be a problem)


//////////
//	   
// header files
//	   
//////////

#include "VRMovies.h"


//////////
//	   
// constants
//	   
//////////

const RGBColor			kClearColor = {0x0000, 0xffff, 0x0000};		// the default chroma key color
const RGBColor			kBlackColor = {0x0000, 0x0000, 0x0000};
const RGBColor			kWhiteColor = {0xffff, 0xffff, 0xffff};


//////////
//	   
// global variables
//	   
//////////

#if TARGET_OS_MAC
UserEventUPP			gColorFilterUPP = NULL;						// UPP to our custom color picker dialog event filter
#endif

//////////
//
// VRMoov_InitWindowData
// Initialize any window-specific data.
//
//////////

ApplicationDataHdl VRMoov_InitWindowData (WindowObject theWindowObject)
{
#pragma unused(theWindowObject)

	ApplicationDataHdl	myAppData;
	
	myAppData = (ApplicationDataHdl)NewHandleClear(sizeof(ApplicationDataRecord));
	if (myAppData != NULL) {
		(**myAppData).fMovie = NULL;
		(**myAppData).fOffscreenGWorld = NULL;
		(**myAppData).fOffscreenPixMap = NULL;
		(**myAppData).fPrevBBufGWorld = NULL;
		(**myAppData).fMovieCenter.x = 0.0;
		(**myAppData).fMovieCenter.y = 0.0;
		(**myAppData).fMovieScale = 1.0;
		(**myAppData).fMovieWidth = kDefaultEmbMovieWidth;
		(**myAppData).fUseOffscreenGWorld = false;
		(**myAppData).fUseMovieCenter = true;
		(**myAppData).fQTMovieHasSound = false;
		(**myAppData).fCompositeMovie = false;
		(**myAppData).fUseHideRegion = false;
		(**myAppData).fChromaColor = kClearColor;
		(**myAppData).fHideRegion = NULL;
		(**myAppData).fBackBufferIsHoriz = false;
		(**myAppData).fImageDesc = NULL;
		(**myAppData).fImageSequence = 0;
		
		SetIdentityMatrix(&(**myAppData).fMovieMatrix);
		SetIdentityMatrix(&(**myAppData).fOrigMovieMatrix);
		
		// create a new routine descriptor
		(**myAppData).fBackBufferProc = NewQTVRBackBufferImagingUPP(VRMoov_BackBufferImagingProc);
	}

#if TARGET_OS_MAC	
	if (gColorFilterUPP == NULL)
		gColorFilterUPP = NewUserEventUPP(VRMoov_ColorDialogEventFilter);
#endif
		
	return(myAppData);
}


//////////
//
// VRMoov_DumpWindowData
// Dispose of any window-specific data.
//
//////////

void VRMoov_DumpWindowData (WindowObject theWindowObject)
{
	ApplicationDataHdl		myAppData = NULL;

	myAppData = (ApplicationDataHdl)QTFrame_GetAppDataFromWindowObject(theWindowObject);
	if (myAppData == NULL)
		return;

	VRMoov_DumpEmbeddedMovie(theWindowObject);
	
	DisposeQTVRBackBufferImagingUPP((**myAppData).fBackBufferProc);
}


///////////
//
// VRMoov_GetEmbeddedMovie
// Get the QuickTime movie to be embedded in a panorama.
// Returns a Boolean to indicate success (true) or failure (false).
//
//////////

Boolean VRMoov_GetEmbeddedMovie (WindowObject theWindowObject)
{
	FSSpec					myFSSpec;
	OSType 					myTypeList = kQTFileTypeMovie;
	OSErr					myErr = paramErr;

	// do some preliminary parameter checking
	if (theWindowObject == NULL)
		return(false);
	
	if (!QTFrame_IsWindowObjectOurs(theWindowObject))
		return(false);
	
	// elicit the movie file from user
	myErr = QTFrame_GetOneFileWithPreview(1, (QTFrameTypeListPtr)&myTypeList, &myFSSpec, NULL);
	if (myErr != noErr) {
		VRMoov_DumpEmbeddedMovie(theWindowObject); 	// clean up any existing embedded movie
		return(false);
	}
		
	return(VRMoov_LoadEmbeddedMovie(&myFSSpec, theWindowObject));
}


//////////
//
// VRMoov_LoadEmbeddedMovie
// Load the QuickTime movie in the specified file.
// Returns a Boolean to indicate success (true) or failure (false).
//
//////////

Boolean VRMoov_LoadEmbeddedMovie (FSSpec *theMovieFile, WindowObject theWindowObject)
{
	short					myMovieFileRef;
	Movie					myMovie;
	GWorldPtr				myGWorld = NULL;
	Rect					myRect;
	ApplicationDataHdl		myAppData = NULL;
	OSErr					myErr = paramErr;
		
	myAppData = (ApplicationDataHdl)QTFrame_GetAppDataFromWindowObject(theWindowObject);
	if (myAppData == NULL)
		goto bail;

	HLock((Handle)myAppData);
	
	//////////
	//
	// open the movie file and the movie
	//
	//////////
	
	myErr = OpenMovieFile(theMovieFile, &myMovieFileRef, fsRdPerm);
	if (myErr != noErr)
		goto bail;

	myErr = NewMovieFromFile(&myMovie, myMovieFileRef, NULL, (StringPtr)NULL, newMovieActive, NULL);
	if (myErr != noErr)
		goto bail;

	//////////
	//
	// get the movie geometry
	//
	//////////
	
	GetMovieBox(myMovie, &myRect);
	GetMovieMatrix(myMovie, &(**myAppData).fOrigMovieMatrix);
	MacOffsetRect(&myRect, -myRect.left, -myRect.top);
	SetMovieBox(myMovie, &myRect);

	// keep track of the movie and movie rectangle (in our app-specific data structure)
	(**myAppData).fMovie = myMovie;
	(**myAppData).fMovieBox = myRect;

	// determine the orientation of the back buffer
	(**myAppData).fBackBufferIsHoriz = QTVRUtils_IsBackBufferHorizontal((**theWindowObject).fInstance);
	
	// get rid of any existing offscreen graphics world
	if ((**myAppData).fOffscreenGWorld != NULL) {
		DisposeGWorld((**myAppData).fOffscreenGWorld);
		(**myAppData).fOffscreenGWorld = NULL;
	}
	
	// clear out any existing custom cover/uncover functions and reset the video media graphics mode
	// (these may have been modified for direct-screen drawing)
	SetMovieCoverProcs(myMovie, NULL, NULL, 0L);
	VRMoov_SetVideoGraphicsMode(myMovie, myAppData, false);
	
	//////////
	//
	// if necessary, create an offscreen graphics world
	//
	// this is where we'll image the movie before copying it into the back buffer
	// when we want to do special effects
	//
	//////////
	
	if ((**myAppData).fUseOffscreenGWorld) {
		myErr = NewGWorld(&myGWorld, 0, &myRect, NULL, NULL, 0);
		(**myAppData).fOffscreenGWorld = myGWorld;
		(**myAppData).fOffscreenPixMap = GetGWorldPixMap(myGWorld);
		
		// make an image description, which is needed by DecompressSequenceBegin
		LockPixels((**myAppData).fOffscreenPixMap);
		MakeImageDescriptionForPixMap((**myAppData).fOffscreenPixMap, &((**myAppData).fImageDesc));
		UnlockPixels((**myAppData).fOffscreenPixMap);
	} else {
		// set the video media graphics mode to drop out the chroma key color in a movie;
		// we also need to install an uncover function that doesn't erase the uncovered region
		if ((**myAppData).fCompositeMovie) {
			VRMoov_SetVideoGraphicsMode(myMovie, myAppData, true);
			SetMovieCoverProcs(myMovie, NewMovieRgnCoverUPP(VRMoov_UncoverProc), NULL, (long)theWindowObject);
		}
	}
	
	//////////
	//
	// install the back-buffer imaging procedure
	//
	//////////
	
	if ((**theWindowObject).fInstance != NULL)
		myErr = VRMoov_InstallBackBufferImagingProc((**theWindowObject).fInstance, theWindowObject);
		
	// start the movie playing in a loop
	VRMoov_LoopEmbeddedMovie(myMovie);

bail:
	// we don't want to edit the embedded movie, so we can close the movie file
	if (myMovieFileRef != 0)
		CloseMovieFile(myMovieFileRef);
		
	HUnlock((Handle)myAppData);

	return(myErr == noErr);
}
	
	
//////////
//
// VRMoov_LoopEmbeddedMovie
// Start the QuickTime movie playing in a loop.
//
//////////

void VRMoov_LoopEmbeddedMovie (Movie theMovie)
{
	TimeBase		myTimeBase;

	// throw the movie into loop mode
	myTimeBase = GetMovieTimeBase(theMovie);
	SetTimeBaseFlags(myTimeBase, GetTimeBaseFlags(myTimeBase) | loopTimeBase);

	// start playing the movie
	StartMovie(theMovie);
}
	

//////////
//
// VRMoov_DumpEmbeddedMovie
// Stop any existing embedded movie from playing and then clean up.
//
//////////

void VRMoov_DumpEmbeddedMovie (WindowObject theWindowObject)
{
	ApplicationDataHdl		myAppData;

	if (theWindowObject == NULL) 
		goto bail;
	
	myAppData = (ApplicationDataHdl)QTFrame_GetAppDataFromWindowObject(theWindowObject);
	if (myAppData == NULL)
		goto bail;
	
	// if we have an embedded movie, stop it from playing and dispose of it
	if ((**myAppData).fMovie != NULL) {
		StopMovie((**myAppData).fMovie);
		DisposeMovie((**myAppData).fMovie);
		(**myAppData).fMovie = NULL;
	}
	
	// get rid of any existing offscreen graphics world
	if ((**myAppData).fOffscreenGWorld != NULL) {
		DisposeGWorld((**myAppData).fOffscreenGWorld);
		(**myAppData).fOffscreenGWorld = NULL;
	}
	
	// clear the existing back buffer imaging proc
	QTVRSetBackBufferImagingProc((**theWindowObject).fInstance, NULL, 0, NULL, 0);
	
	// clear out any other movie-specific data
	(**myAppData).fPrevBBufGWorld = NULL;
	MacSetRect(&(**myAppData).fPrevBBufRect, 0, 0, 0, 0);
	
	MacSetRect(&(**myAppData).fMovieBox, 0, 0, 0, 0);
	SetIdentityMatrix(&(**myAppData).fMovieMatrix);
	SetIdentityMatrix(&(**myAppData).fOrigMovieMatrix);
	
	if ((**myAppData).fImageDesc != NULL) {
		DisposeHandle((Handle)(**myAppData).fImageDesc);
		(**myAppData).fImageDesc = NULL;
	}
	
	VRMoov_RemoveDecompSeq(theWindowObject);
	
	// make sure the back buffer is clean
	QTVRRefreshBackBuffer((**theWindowObject).fInstance, 0);

bail:
	return;
}
	

//////////
//
// VRMoov_InstallBackBufferImagingProc
// Install a back buffer imaging procedure.
// (This routine might sometimes be called to move or resize the area of interest within the panorama.)
//
//////////

OSErr VRMoov_InstallBackBufferImagingProc (QTVRInstance theInstance, WindowObject theWindowObject)
{
	ApplicationDataHdl		myAppData;
	QTVRAreaOfInterest		myArea;
	float					myWidth, myHeight;
	OSErr					myErr = noErr;

	//////////
	//
	// initialize; clean up any existing back buffer procedure
	//
	//////////
	
	if ((theInstance == NULL) || (theWindowObject == NULL)) 
		return(paramErr);

	myAppData = (ApplicationDataHdl)QTFrame_GetAppDataFromWindowObject(theWindowObject);
	if (myAppData == NULL) 
		return(paramErr);

	HLock((Handle)myAppData);
	
	// remove any existing back buffer imaging procedure
	if ((**myAppData).fBackBufferProc != NULL)
		QTVRSetBackBufferImagingProc(theInstance, NULL, 0, NULL, 0);

	//////////
	//
	// set the area of interest
	//
	// the panAngle and tiltAngle fields define the top-left corner, in panorama space, of the area of interest;
	// so here we do not have to worry about whether the back buffer is oriented vertically or horizontally
	//
	//////////
	
	// the application data structure holds the desired width, center, size, and scale of the movie
	myWidth = (**myAppData).fMovieWidth * (**myAppData).fMovieScale;
	myHeight = myWidth * (((float)(**myAppData).fMovieBox.bottom) / ((float)(**myAppData).fMovieBox.right));
	
	if ((**myAppData).fUseMovieCenter) {
		// use the stored movie center
		myArea.panAngle = (**myAppData).fMovieCenter.x + (myWidth/2);
		myArea.tiltAngle = (**myAppData).fMovieCenter.y + (myHeight/2);
	} else {
		// center the movie on the current pan and tilt angles
		myArea.panAngle = QTVRGetPanAngle(theInstance) + (myWidth/2);
		myArea.tiltAngle = QTVRGetTiltAngle(theInstance) + (myHeight/2);
	}

	myArea.width = myWidth;
	myArea.height = myHeight;

	//////////
	//
	// set the back buffer flags and install the back buffer procedure
	//
	//////////
	
	// make sure we get called on every idle event, so we can keep playing the embedded movie;
	// also make sure we get called on every back buffer update
	if ((**myAppData).fCompositeMovie)
		myArea.flags = kQTVRBackBufferEveryIdle | kQTVRBackBufferEveryUpdate | kQTVRBackBufferAlwaysRefresh;
	else
		myArea.flags = kQTVRBackBufferEveryIdle | kQTVRBackBufferEveryUpdate;
	
	// if the back buffer is oriented horizontally, set the appropriate flag
	if ((**myAppData).fBackBufferIsHoriz)
		myArea.flags |= kQTVRBackBufferHorizontal;

	// install our procedure
	myErr = QTVRSetBackBufferImagingProc(theInstance, (**myAppData).fBackBufferProc, 1, &myArea, (SInt32)theWindowObject);

	HUnlock((Handle)myAppData);
	return(myErr);
}
	
	
//////////
//
// VRMoov_CalcImagingMatrix
// Calculate the movie matrix required to draw the embedded movie into the specified rectangle.
//
//////////

OSErr VRMoov_CalcImagingMatrix (WindowObject theWindowObject, Rect *theBBufRect)
{
	ApplicationDataHdl		myAppData = NULL;
	Rect					myDestRect = *theBBufRect;

	myAppData = (ApplicationDataHdl)QTFrame_GetAppDataFromWindowObject(theWindowObject);
	if (myAppData == NULL) 
		return(paramErr);
		
	// reset the current movie matrix with the original movie matrix of the embedded movie;
	// we need to preserve that matrix in our calculations below if we are not drawing the
	// movie into an offscreen GWorld
	(**myAppData).fMovieMatrix = (**myAppData).fOrigMovieMatrix;
	
	// in general, it's easiest to construct the desired matrix by first doing the scaling
	// and then doing the rotation and translation (if necessary); so we need to swap the
	// right and bottom edges of the back buffer rectangle before doing the scaling, if a
	// rotation will also be necessary
	if (!(**myAppData).fBackBufferIsHoriz) {
		myDestRect.bottom = theBBufRect->right;
		myDestRect.right = theBBufRect->bottom;
	}
	
	// set up the scaling matrix
	// (MapMatrix concatenates the new matrix to the existing matrix, whereas RectMatrix first
	// sets the existing matrix to the identity matrix)
	if ((**myAppData).fUseOffscreenGWorld)
		RectMatrix(&(**myAppData).fMovieMatrix, &(**myAppData).fMovieBox, &myDestRect);
	else
		MapMatrix(&(**myAppData).fMovieMatrix, &(**myAppData).fMovieBox, &myDestRect);

	// add a rotation and translation, if necessary
	if (!(**myAppData).fBackBufferIsHoriz) {
		RotateMatrix(&(**myAppData).fMovieMatrix, Long2Fix(-90), 0, 0);
		TranslateMatrix(&(**myAppData).fMovieMatrix, 0, Long2Fix(RECT_HEIGHT(*theBBufRect)));
	}
	
	return(noErr);
}


//////////
//
// VRMoov_SetupDecompSeq
// Set up the decompression sequence for DecompressionSequenceFrameS.
//
// This needs to be called whenever either the rectangle or the GWorld of the back buffer changes.
//
//////////

OSErr VRMoov_SetupDecompSeq (WindowObject theWindowObject, GWorldPtr theDestGWorld)
{
	ApplicationDataHdl		myAppData = NULL;
	short					myMode = srcCopy;
	OSErr					myErr = noErr;
	
	myAppData = (ApplicationDataHdl)QTFrame_GetAppDataFromWindowObject(theWindowObject);
	if (myAppData == NULL) 
		return(paramErr);
	
	// make sure we don't have a decompression sequence already open
	VRMoov_RemoveDecompSeq(theWindowObject);
	
	// set up the transfer mode
	if ((**myAppData).fCompositeMovie)
		myMode = srcCopy | transparent;
		
	// set up the image decompression sequence
	myErr = DecompressSequenceBegin(	&(**myAppData).fImageSequence,
										(**myAppData).fImageDesc,
										(CGrafPtr)theDestGWorld,
										NULL,
										NULL,							// entire source image
										&(**myAppData).fMovieMatrix,
				   						myMode,
										NULL,							// no mask
										0,
										(**(**myAppData).fImageDesc).spatialQuality,
										NULL);
									
	return(myErr);
}


//////////
//
// VRMoov_RemoveDecompSeq
// Remove the decompression sequence for DecompressionSequenceFrameS.
//
//////////

OSErr VRMoov_RemoveDecompSeq (WindowObject theWindowObject)
{
	ApplicationDataHdl		myAppData = NULL;
	OSErr					myErr = paramErr;
	
	if (theWindowObject != NULL) {
		myAppData = (ApplicationDataHdl)QTFrame_GetAppDataFromWindowObject(theWindowObject);
		if (myAppData != NULL) 
			if ((**myAppData).fImageSequence != 0) {
				myErr = CDSequenceEnd((**myAppData).fImageSequence);
				(**myAppData).fImageSequence = 0;
			}
	}
	
	return(myErr);
}


//////////
//
// VRMoov_BackBufferImagingProc
// The back buffer imaging procedure: get a frame of movie and image it into the back buffer.
// Also, do any additional compositing that might be desired.
//
//////////

PASCAL_RTN OSErr VRMoov_BackBufferImagingProc (QTVRInstance theInstance, Rect *theRect, UInt16 theAreaIndex, UInt32 theFlagsIn, UInt32 *theFlagsOut, long theRefCon)
{
#pragma unused(theAreaIndex)
	WindowObject			myWindowObject = (WindowObject)theRefCon;
	ApplicationDataHdl		myAppData = NULL;
	Movie					myMovie = NULL;
	Boolean					myIsDrawing = theFlagsIn & kQTVRBackBufferRectVisible;
	GWorldPtr				myBBufGWorld, myMovGWorld;
	GDHandle				myBBufGDevice, myMovGDevice;
	Rect					myRect;
	OSErr					myErr = paramErr;
	
	//////////
	//
	// initialize; make sure that we've got the data we need to continue
	//
	//////////

	// assume we're not going to draw anything
	*theFlagsOut = 0;
	
	if ((theInstance == NULL) || (myWindowObject == NULL)) 
		goto bail;

	myAppData = (ApplicationDataHdl)QTFrame_GetAppDataFromWindowObject(myWindowObject);
	if (myAppData == NULL) 
		goto bail;

	HLock((Handle)myAppData);

	myMovie = (**myAppData).fMovie;
	if (myMovie == NULL) {
		// we don't have an embedded movie, so remove this back-buffer imaging procedure
		VRMoov_DumpEmbeddedMovie(myWindowObject);
		goto bail;
	}
	
	//////////
	//
	// make sure that the movie GWorld is set correctly;
	// note that we call SetMovieGWorld only if we have to (for performance reasons)
	//
	//////////
	
	// get the current graphics world
	// (on entry, the current graphics world is [usually] set to the back buffer)
	GetGWorld(&myBBufGWorld, &myBBufGDevice);

	// get the embedded movie's graphics world
	GetMovieGWorld(myMovie, &myMovGWorld, &myMovGDevice);

	if ((**myAppData).fUseOffscreenGWorld) {
		// we're using an offscreen graphics world, so set movie's GWorld to be that offscreen graphics world
		if (myMovGWorld != (**myAppData).fOffscreenGWorld)
			SetMovieGWorld(myMovie, (**myAppData).fOffscreenGWorld, GetGWorldDevice((**myAppData).fOffscreenGWorld));			
	} else {
		// we're not using an offscreen graphics world, so set movie GWorld to be the back buffer;
		if ((myMovGWorld != myBBufGWorld) || (myMovGDevice != myBBufGDevice))
			SetMovieGWorld(myMovie, myBBufGWorld, myBBufGDevice);
	}

	//////////
	//
	// make sure the movie rectangle and movie matrix are set correctly
	//
	//////////
	
	if (myIsDrawing) {
		// if we weren't previously visible, make sure we are now
		GetMovieBox(myMovie, &myRect);
		if (EmptyRect(&myRect))
			SetMovieBox(myMovie, &(**myAppData).fMovieBox);
		
		// when no offscreen GWorld is being used...
		if (!(**myAppData).fUseOffscreenGWorld) {
			
			// ...make sure the movie matrix is set correctly...
			if (!MacEqualRect(theRect, &(**myAppData).fPrevBBufRect)) {
				VRMoov_CalcImagingMatrix(myWindowObject, theRect);
				SetMovieMatrix(myMovie, &(**myAppData).fMovieMatrix);
			}
			
			// ...and, if we are compositing, force QuickTime to draw a movie frame when MoviesTask is called
			// (since the previous frame was automatically erased from the back buffer by QuickTime VR)
			if ((**myAppData).fCompositeMovie)
				UpdateMovie(myMovie);
		}
		
	} else {
		// if we're not visible, set the movie rectangle to an empty rectangle
		// so we're not wasting time trying to draw a movie frame
		MacSetRect(&myRect, 0, 0, 0, 0);
		SetMovieBox(myMovie, &myRect);
	}

	//////////
	//
	// draw a new movie frame into the movie's graphics world (and play movie sound)
	//
	//////////
	
	MoviesTask(myMovie, 1L);
//	MoviesTask(myMovie, 0);
	
	// if we got here, everything is okay so far
	myErr = noErr;
	
	//////////
	//
	// perform any additional compositing
	//
	//////////
	
	// that is, draw, using the current chroma key color, anything to be dropped out of the image;
	// note that this technique works *only* if we're using an offscreen graphics world
	if ((**myAppData).fUseOffscreenGWorld && (**myAppData).fCompositeMovie && myIsDrawing) {
		RGBColor	myColor;
	
		// since we're using an offscreen graphics world, make sure we draw there
		SetGWorld((**myAppData).fOffscreenGWorld, GetGWorldDevice((**myAppData).fOffscreenGWorld));

		// set up compositing environment
		GetForeColor(&myColor);
		RGBForeColor(&(**myAppData).fChromaColor);
		
		// do the drawing
		if ((**myAppData).fHideRegion != NULL)
			MacPaintRgn((**myAppData).fHideRegion);
			
		// restore original drawing environment
		RGBForeColor(&myColor);
		
		// restore original graphics world
		SetGWorld(myBBufGWorld, myBBufGDevice);
	}

	//////////
	//
	// if we're using an offscreen graphics world, copy it into the back buffer
	//
	//////////
	
	if (myIsDrawing) {
	
		if ((**myAppData).fUseOffscreenGWorld) {
			PixMapHandle	myPixMap;
			
			// if anything relevant to DecompressSequenceFrameS has changed, reset the decompression sequence
			if ((myBBufGWorld != (**myAppData).fPrevBBufGWorld) || !(MacEqualRect(theRect, &(**myAppData).fPrevBBufRect))) {
				VRMoov_CalcImagingMatrix(myWindowObject, theRect);
				VRMoov_SetupDecompSeq(myWindowObject, myBBufGWorld);
			}
			
			myPixMap = GetGWorldPixMap((**myAppData).fOffscreenGWorld);
			LockPixels(myPixMap);
			
			// set the chroma key color, if necessary
			if ((**myAppData).fCompositeMovie)
				RGBBackColor(&(**myAppData).fChromaColor);
			
			// copy the image from the offscreen graphics world into the back buffer
			myErr = DecompressSequenceFrameS(	(**myAppData).fImageSequence,
#if TARGET_CPU_68K
												StripAddress(GetPixBaseAddr(myPixMap)),
#else
												GetPixBaseAddr(myPixMap),
#endif
												(**(**myAppData).fImageDesc).dataSize,
												0,
												NULL,
												NULL);

			// reset the chroma key color;
			// we need to do this because the buffer we just drew into might NOT actually
			// be the real back buffer (see Virtual Reality Programming With QuickTime VR, p. 1-154);
			// the copy between the intermediate buffer and the back buffer respects the current back color.
			if ((**myAppData).fCompositeMovie)
				RGBBackColor(&kWhiteColor);
				
			UnlockPixels(myPixMap);
		}
	}
	
	//////////
	//
	// finish up
	//
	//////////
	
	// keep track of the GWorld and rectangle passed to us this time
	(**myAppData).fPrevBBufGWorld = myBBufGWorld;
	(**myAppData).fPrevBBufRect = *theRect;
	
	// if we drew something, tell QuickTime VR
	if (myIsDrawing)
		*theFlagsOut = kQTVRBackBufferFlagDidDraw;
	
bail:	
	HUnlock((Handle)myAppData);
	return(myErr);
}


//////////
//
// VRMoov_GetEmbeddedMovieWidth
// Get the width of the embedded movie.
//
//////////

float VRMoov_GetEmbeddedMovieWidth (WindowObject theWindowObject)
{
	ApplicationDataHdl		myAppData;
	float					myWidth;

	myAppData = (ApplicationDataHdl)QTFrame_GetAppDataFromWindowObject(theWindowObject);
	if (myAppData == NULL)
		myWidth = 0;
	else
		myWidth = (**myAppData).fMovieWidth;
		
	return(myWidth);
}


//////////
//
// VRMoov_SetEmbeddedMovieWidth
// Set the width of the embedded movie.
//
//////////

void VRMoov_SetEmbeddedMovieWidth (WindowObject theWindowObject, float theWidth)
{
	ApplicationDataHdl		myAppData;
	QTVRInstance			myInstance;

	myAppData = (ApplicationDataHdl)QTFrame_GetAppDataFromWindowObject(theWindowObject);
	if (myAppData == NULL)
		return;
		
	myInstance = (**theWindowObject).fInstance;
	if (myInstance == NULL)
		return;
	
	// install the desired width in our application data structure
	(**myAppData).fMovieWidth = theWidth;
	
	// clear out the existing area of interest
	QTVRRefreshBackBuffer(myInstance, 0);

	// reinstall the back buffer imaging procedure
	VRMoov_InstallBackBufferImagingProc(myInstance, theWindowObject);
}


//////////
//
// VRMoov_GetEmbeddedMovieCenter
// Get the center of the embedded movie.
//
//////////

void VRMoov_GetEmbeddedMovieCenter (WindowObject theWindowObject, QTVRFloatPoint *theCenter)
{
	ApplicationDataHdl		myAppData;

	myAppData = (ApplicationDataHdl)QTFrame_GetAppDataFromWindowObject(theWindowObject);
	if (myAppData == NULL) {
		theCenter->x = 0.0;
		theCenter->y = 0.0;
	} else {
		theCenter->x = (**myAppData).fMovieCenter.x;
		theCenter->y = (**myAppData).fMovieCenter.y;
	}
		
}


//////////
//
// VRMoov_SetEmbeddedMovieCenter
// Set the center of the embedded movie.
//
//////////

void VRMoov_SetEmbeddedMovieCenter (WindowObject theWindowObject, const QTVRFloatPoint *theCenter)
{
	ApplicationDataHdl		myAppData;
	QTVRInstance			myInstance;
	float					myX, myY;

	myAppData = (ApplicationDataHdl)QTFrame_GetAppDataFromWindowObject(theWindowObject);
	if (myAppData == NULL)
		return;
		
	myInstance = (**theWindowObject).fInstance;
	if (myInstance == NULL)
		return;
	
	myX = theCenter->x;
	myY = theCenter->y;
	
	// subject the values passed in to the current view constraints
	QTVRWrapAndConstrain(myInstance, kQTVRPan, myX, &myX);
	QTVRWrapAndConstrain(myInstance, kQTVRTilt, myY, &myY);
			
	// install the desired center in our application data structure
	(**myAppData).fMovieCenter.x = myX;
	(**myAppData).fMovieCenter.y = myY;
	
	// clear out the existing area of interest
	QTVRRefreshBackBuffer(myInstance, 0);

	// reinstall the back buffer imaging procedure
	VRMoov_InstallBackBufferImagingProc(myInstance, theWindowObject);
}


//////////
//
// VRMoov_GetEmbeddedMovieScale
// Get the scale of the embedded movie.
//
//////////

float VRMoov_GetEmbeddedMovieScale (WindowObject theWindowObject)
{
	ApplicationDataHdl		myAppData;
	float					myScale;

	myAppData = (ApplicationDataHdl)QTFrame_GetAppDataFromWindowObject(theWindowObject);
	if (myAppData == NULL)
		myScale = 0;
	else
		myScale = (**myAppData).fMovieScale;
		
	return(myScale);
}


//////////
//
// VRMoov_SetEmbeddedMovieScale
// Set the scale factor of the embedded movie.
//
//////////

void VRMoov_SetEmbeddedMovieScale (WindowObject theWindowObject, float theScale)
{
	ApplicationDataHdl		myAppData;
	QTVRInstance			myInstance;

	myAppData = (ApplicationDataHdl)QTFrame_GetAppDataFromWindowObject(theWindowObject);
	if (myAppData == NULL)
		return;
		
	myInstance = (**theWindowObject).fInstance;
	if (myInstance == NULL)
		return;
	
	// install the desired scale factor in our application data structure
	(**myAppData).fMovieScale = theScale;
	
	// clear out the existing area of interest
	QTVRRefreshBackBuffer(myInstance, 0);

	// reinstall the back buffer imaging procedure
	VRMoov_InstallBackBufferImagingProc(myInstance, theWindowObject);
}


//////////
//
// VRMoov_SetChromaColor
// Set the chroma key color for a window object:
// display color picker dialog and remember the newly-selected color.
//
//////////

void VRMoov_SetChromaColor (WindowObject theWindowObject)
{
#if TARGET_OS_MAC
	ColorPickerInfo		myColorInfo;
#endif	
#if TARGET_OS_WIN32
	static CHOOSECOLOR	myColorRec;
	static COLORREF		myColorRef[16];
#endif	
	ApplicationDataHdl	myAppData;

	myAppData = (ApplicationDataHdl)QTFrame_GetAppDataFromWindowObject(theWindowObject);
	if (myAppData == NULL)
		return;

#if TARGET_OS_MAC
	// pass in existing color
	myColorInfo.theColor.color.rgb.red = (**myAppData).fChromaColor.red;
	myColorInfo.theColor.color.rgb.green = (**myAppData).fChromaColor.green;
	myColorInfo.theColor.color.rgb.blue = (**myAppData).fChromaColor.blue;
	
	// not much here...
	myColorInfo.theColor.profile = 0L;
	myColorInfo.dstProfile = 0L;
	myColorInfo.flags = 0L;
	myColorInfo.placeWhere = kCenterOnMainScreen;
	myColorInfo.pickerType = 0L;
	myColorInfo.eventProc = gColorFilterUPP;
	myColorInfo.colorProc = NULL;
	myColorInfo.colorProcData = 0L;
	GetIndString(myColorInfo.prompt, kColorPickerTextStringID, 1);
	
	// set Edit menu info
	myColorInfo.mInfo.editMenuID = kEditMenuResID;
	myColorInfo.mInfo.undoItem = MENU_ITEM(IDM_EDITUNDO);
	myColorInfo.mInfo.cutItem = MENU_ITEM(IDM_EDITCUT);
	myColorInfo.mInfo.copyItem = MENU_ITEM(IDM_EDITCOPY);
	myColorInfo.mInfo.pasteItem = MENU_ITEM(IDM_EDITPASTE);
	myColorInfo.mInfo.clearItem = MENU_ITEM(IDM_EDITCLEAR);
	
	// call Color Picker
	if ((PickColor(&myColorInfo) == noErr) && (myColorInfo.newColorChosen)) {
		// install the newly chosen color in the palette
		(**myAppData).fChromaColor.red = myColorInfo.theColor.color.rgb.red;
		(**myAppData).fChromaColor.green = myColorInfo.theColor.color.rgb.green;
		(**myAppData).fChromaColor.blue = myColorInfo.theColor.color.rgb.blue;
	}
#endif
#if TARGET_OS_WIN32
	myColorRec.lStructSize = sizeof(CHOOSECOLOR);
	myColorRec.hwndOwner = NULL;
	myColorRec.hInstance = NULL;
	
	VRMoov_MacRGBToWinRGB(&(**myAppData).fChromaColor, &(myColorRec.rgbResult));
	
	myColorRec.lpCustColors = myColorRef;
	myColorRec.Flags = CC_RGBINIT | CC_FULLOPEN;
	myColorRec.lCustData = 0;
	myColorRec.lpfnHook = NULL;
	myColorRec.lpTemplateName = NULL;
	
	// call Common Color dialog
	if (ChooseColor(&myColorRec)) {
		// install the newly chosen color in the palette
		VRMoov_WinRGBToMacRGB(&(**myAppData).fChromaColor, myColorRec.rgbResult);
	}
#endif
}
	

//////////
//
// VRMoov_ColorDialogEventFilter
// Handle events before they get passed to the Color Picker dialog box.
//
//////////

PASCAL_RTN Boolean VRMoov_ColorDialogEventFilter (EventRecord *theEvent)
{
#if TARGET_OS_WIN32
#pragma unused(theEvent)
#endif
	Boolean				myEventHandled = false;
	OSErr				myErr = noErr;

#if TARGET_OS_MAC
	switch (theEvent->what) {
		case updateEvt: {
			if ((WindowPtr)theEvent->message != FrontWindow()) {
				QTFrame_HandleEvent(theEvent);
				myEventHandled = true;
			}
			break;
		}

		case nullEvent: {
			// do idle-time processing for all open windows in our window list
			WindowObject		myWindowObject = NULL;
			ApplicationDataHdl	myAppData = NULL;
			WindowReference		myWindow = NULL;

			myWindow = QTFrame_GetFrontMovieWindow();
			while (myWindow != NULL) {
				myWindowObject = (WindowObject)QTFrame_GetWindowObjectFromWindow(myWindow);
				myAppData = (ApplicationDataHdl)QTFrame_GetAppDataFromWindow(myWindow);
				if ((myWindowObject != NULL) && (myAppData != NULL))
					if ((**myAppData).fMovie != NULL)
						QTVRUpdate((**myWindowObject).fInstance, kQTVRCurrentMode);
				myWindow = QTFrame_GetNextMovieWindow(myWindow);
			}
			
			myEventHandled = false;
			break;
		}
		
		case kHighLevelEvent:
			myErr = AEProcessAppleEvent(theEvent);
			if (myErr != noErr)
				myEventHandled = true;
		 	break;
			
		default:
			myEventHandled = false;
			break;
	}	
	
#endif
	return(myEventHandled);
}


//////////
//
// VRMoov_UncoverProc
// The uncover function of the embedded movie.
//
//////////

PASCAL_RTN OSErr VRMoov_UncoverProc (Movie theMovie, RgnHandle theRegion, long theRefCon)
{
#pragma unused(theMovie, theRegion, theRefCon)
	return(noErr);
}


//////////
//
// VRMoov_SetVideoGraphicsMode
// Set the video media graphics mode of the embedded movie.
//
//////////

void VRMoov_SetVideoGraphicsMode (Movie theMovie, ApplicationDataHdl theAppData, Boolean theSetVGM)
{
	long				myTrackCount;
	short				myIndex;
	Track				myTrack = NULL;
	Media				myMedia = NULL;
	OSType				myMediaType;
	
	if ((theMovie == NULL) || (theAppData == NULL))
		return;
	
	myTrackCount = GetMovieTrackCount(theMovie);
	for (myIndex = 1; myIndex <= myTrackCount; myIndex++) {
		myTrack = GetMovieIndTrack(theMovie, myIndex);
		myMedia = GetTrackMedia(myTrack);
		GetMediaHandlerDescription(myMedia, &myMediaType, NULL, NULL);
		if (myMediaType == VideoMediaType) {
			if (theSetVGM)
				MediaSetGraphicsMode(GetMediaHandler(myMedia), srcCopy | transparent, &(**theAppData).fChromaColor);
			else
				MediaSetGraphicsMode(GetMediaHandler(myMedia), srcCopy, &kWhiteColor);
		}
	}
	
	return;
}


//////////
//
// VRMoov_GetVideoGraphicsPixelDepth
// Return the highest pixel depth supported by a QuickTime movie.
//
//////////

short VRMoov_GetVideoGraphicsPixelDepth (Movie theMovie)
{
	long				myTrackCount;
	short				myIndex;
	Track				myMovieTrack = NULL;
	Media				myMedia;
	OSType				myMediaType;
	short				myQuality;
	
	myTrackCount = GetMovieTrackCount(theMovie);
	for (myIndex = 1; myIndex <= myTrackCount; myIndex++) {
		myMovieTrack = GetMovieIndTrack(theMovie, myIndex);
		myMedia = GetTrackMedia(myMovieTrack);
		GetMediaHandlerDescription(myMedia, &myMediaType, NULL, NULL);
		if (myMediaType == VideoMediaType) {
			myQuality = GetMediaQuality(myMedia);
			if (myQuality >> 5)
				return(32);
			if (myQuality >> 4)
				return(16);
			if (myQuality >> 3)
				return(8);
			if (myQuality >> 2)
				return(4);
			if (myQuality >> 1)
				return(2);
			if (myQuality >> 0)
				return(1);
		}
	}
	
	return(0);
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Color conversion utilities.
//
// Macintosh usually represents colors using an RGBColor structure, where each color component is a 16-bit
// unsigned integer. Windows represents colors using a 32-bit unsigned COLORREF, where each color component
// occupies 8 bits. The following two functions allow us to convert from Mac to Windows colors and back. We
// need to do this because we use Mac-style colors when doing compositing.
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////

#if TARGET_OS_WIN32

#define kMaxMacRGBValue				0xffff
#define kMaxWinRGBValue				0x00ff
#define MAC_TO_WIN_COMP(color)		(color*((float)kMaxWinRGBValue/(float)kMaxMacRGBValue))
#define WIN_TO_MAC_COMP(color)		(color*((float)kMaxMacRGBValue/(float)kMaxWinRGBValue))


//////////
//
// VRMoov_MacRGBToWinRGB
// Convert an RGBColor structure into a COLORREF value.
//
//////////

void VRMoov_MacRGBToWinRGB (RGBColorPtr theRGBColor, COLORREF *theColorRef)
{
	*theColorRef = RGB(	(BYTE)MAC_TO_WIN_COMP(theRGBColor->red),
						(BYTE)MAC_TO_WIN_COMP(theRGBColor->green),
						(BYTE)MAC_TO_WIN_COMP(theRGBColor->blue));
}


//////////
//
// VRMoov_WinRGBToMacRGB
// Convert a COLORREF value into an RGBColor structure.
//
//////////

void VRMoov_WinRGBToMacRGB (RGBColorPtr theRGBColor, COLORREF theColorRef)
{
	theRGBColor->red = (unsigned long)WIN_TO_MAC_COMP(GetRValue(theColorRef));
	theRGBColor->green = (unsigned long)WIN_TO_MAC_COMP(GetGValue(theColorRef));
	theRGBColor->blue = (unsigned long)WIN_TO_MAC_COMP(GetBValue(theColorRef));
}
#endif	// TARGET_OS_WIN32


