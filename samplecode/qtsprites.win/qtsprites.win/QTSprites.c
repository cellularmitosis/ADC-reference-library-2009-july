//////////
//
//	File:		QTSprites.c
//
//	Contains:	QuickTime sprites support for QuickTime movies.
//
//	Written by:	Tim Monroe
//	Revised by:	Deeje Cooley and Tim Monroe
//				Based (heavily!) on the existing MakeSpriteMovie.c code written by Sean Allen.
//
//	Copyright:	� 1997-1998 by Apple Computer, Inc., all rights reserved.
//
//	Change History (most recent first):
//
//	   <6>	 	01/10/01	rtm		modified QTSprites_CreateSpritesMovie to create one of three
//									different sprite movies (icon, penguin, and space movies)
//	   <5>	 	03/17/00	rtm		made changes to get things running under CarbonLib
//	   <4>	 	09/30/98	rtm		tweaked call to AddMovieResource to create single-fork movies
//	   <3>	 	06/19/98	rtm		moved to new routine names (e.g. SpriteMediaSetSpriteProperty)
//	   <2>	 	04/09/98	rtm		added sprite hit-testing
//	   <1>	 	04/02/98	rtm		first file; integrated existing code with shell framework
//	   
//	This sample code creates a sprite movie containing one sprite track. The contents of the sprite
//	track vary, depending on the value passed to QTSprites_CreateSpritesMovie. In the simplest case,
//	we create a movie with an icon moviing from left to right, with the icon changing at the midpoint.
//	The second movie is contains a penguin sprite whose opacity gradually increases. In the third movie, the sprite
//	track contains a static background picture sprite (or just a colored background, depending on the value of the
//	global variable gUseBackgroundPicture) and three other sprites that change their properties over time.
//	The track's media contains only one key frame sample followed by many override samples. The key
//	frame contains all of the images used by the sprites; the override frames only contain the overrides
//	of the locations, image indices, and layers needed for the other sprites.
//
//	This sample code also shows how to hit test sprites. It uses the function SpriteMediaHitTestAllSprites
//	to find mouse clicks on the sprites in the first sprite track in a movie. If the user clicks on a
//	sprite, we toggle the visibility state of the sprite. You would probably want to do something a bit
//	more interesting when the user clicks on a sprite.
//
//////////


//////////
//
// header files
//
//////////

#include "QTSprites.h"


//////////
//
// global variables
//
//////////

Boolean 						gUseBackgroundPicture = false;		// do we display a background picture?


//////////
//
// QTSprites_InitWindowData
// Do any sprite-specific initialization for the specified window.
//
//////////

ApplicationDataHdl QTSprites_InitWindowData (WindowObject theWindowObject)
{
	ApplicationDataHdl		myAppData = NULL;
	Track					myTrack = NULL;
	MediaHandler			myHandler = NULL;

	myAppData = (ApplicationDataHdl)NewHandleClear(sizeof(ApplicationDataRecord));
	if (myAppData != NULL) {
	
		myTrack = GetMovieIndTrackType((**theWindowObject).fMovie, 1, SpriteMediaType, movieTrackMediaType | movieTrackEnabledOnly);
		if (myTrack != NULL)
			myHandler = GetMediaHandler(GetTrackMedia(myTrack));
	
		// remember the sprite media handler
		(**myAppData).fMovieHasSprites = (myTrack != NULL);
		(**myAppData).fSpriteHandler = myHandler;
	}
	
	return(myAppData);
}


//////////
//
// QTSprites_DumpWindowData
// Get rid of any window-specific data for the sprite media handler.
//
//////////

void QTSprites_DumpWindowData (WindowObject theWindowObject)
{
	ApplicationDataHdl		myAppData = NULL;
		
	myAppData = (ApplicationDataHdl)QTFrame_GetAppDataFromWindowObject(theWindowObject);
	if (myAppData != NULL)
		DisposeHandle((Handle)myAppData);
}


//////////
//
// QTSprites_CreateSpritesMovie
// Create a QuickTime movie containing a sprite track.
//
//////////

OSErr QTSprites_CreateSpritesMovie (UInt16 theMenuItem)
{
	Movie					myMovie = NULL;
	Track					myTrack = NULL;
	Media					myMedia = NULL;
	FSSpec					myFile;
	Boolean					myIsSelected = false;
	Boolean					myIsReplacing = false;	
	Fixed					myHeight = 0;
	Fixed					myWidth = 0;
	StringPtr 				myPrompt = QTUtils_ConvertCToPascalString(kSpriteSavePrompt);
	StringPtr 				myFileName = QTUtils_ConvertCToPascalString(kSpriteSaveMovieFileName);
	long					myFlags = createMovieFileDeleteCurFile | createMovieFileDontCreateResFile;
	short					myResRefNum = 0;
	short					myResID = movieInDataForkResID;
	OSErr					myErr = noErr;

	//////////
	//
	// create a new movie file
	//
	//////////

	// prompt the user for the destination file name
	QTFrame_PutFile(myPrompt, myFileName, &myFile, &myIsSelected, &myIsReplacing);
	myErr = myIsSelected ? noErr : userCanceledErr;
	if (!myIsSelected)
		goto bail;

	// create a movie file for the destination movie
	myErr = CreateMovieFile(&myFile, FOUR_CHAR_CODE('TVOD'), smSystemScript, myFlags, &myResRefNum, &myMovie);
	if (myErr != noErr)
		goto bail;
	
	//////////
	//
	// create the sprite track and media
	//
	//////////
	
	QTSprites_GetMovieSize(theMenuItem, &myHeight, &myWidth);
	
	myTrack = NewMovieTrack(myMovie, myWidth, myHeight, kNoVolume);
	myMedia = NewTrackMedia(myTrack, SpriteMediaType, kSpriteMediaTimeScale, NULL, 0);

	BeginMediaEdits(myMedia);

	//////////
	//
	// add the appropriate samples to the sprite media
	//
	//////////
	
	switch (theMenuItem) {
		case IDM_MAKE_ICONS_MOVIE:
			QTSprites_AddIconMovieSamplesToMedia(myMedia);
			break;
		case IDM_MAKE_PENGUIN_MOVIE:
			QTSprites_AddPenguinMovieSamplesToMedia(myMedia);
			break;
		case IDM_MAKE_SPACE_MOVIE:
			QTSprites_AddSpaceMovieSamplesToMedia(myMedia);
			break;
		default:
			goto bail;
	}
	
	EndMediaEdits(myMedia);
	
	// add the media to the track
	InsertMediaIntoTrack(myTrack, 0, 0, GetMediaDuration(myMedia), fixed1);
	
	//////////
	//
	// set the sprite track properties
	//
	//////////
	
	QTSprites_SetTrackProperties(myMedia, theMenuItem);
	
	//////////
	//
	// finish up
	//
	//////////
	
	// add the movie resource to the movie file
	myErr = AddMovieResource(myMovie, myResRefNum, &myResID, myFile.name);
	
bail:
	if (myResRefNum != 0)
		CloseMovieFile(myResRefNum);

	if (myMovie != NULL)
		DisposeMovie(myMovie);
		
	free(myPrompt);
	free(myFileName);

	return(myErr);
}


//////////
//
// QTSprites_GetMovieSize
// Return the desired size of the specified sprite movie.
//
//////////

void QTSprites_GetMovieSize (UInt16 theMenuItem, Fixed *theHeight, Fixed *theWidth)
{
	if ((theHeight == NULL) || (theWidth == NULL))
		return;
		
	switch (theMenuItem) {
		case IDM_MAKE_ICONS_MOVIE:
			*theWidth = (long)kIconSpriteTrackWidth << 16;
			*theHeight = (long)kIconSpriteTrackHeight << 16;
			break;
		case IDM_MAKE_PENGUIN_MOVIE:
			*theWidth = (long)kPenguinSpriteTrackWidth << 16;
			*theHeight = (long)kPenguinSpriteTrackHeight << 16;
			break;
		case IDM_MAKE_SPACE_MOVIE:
			*theWidth = (long)kSpaceSpriteTrackWidth << 16;
			*theHeight = (long)kSpaceSpriteTrackHeight << 16;
			break;
	}
}


//////////
//
// QTSprites_GetBackgroundColor
// Return the desired size of the specified sprite movie.
//
//////////

void QTSprites_GetBackgroundColor (UInt16 theMenuItem, RGBColor *theColor)
{
	if (theColor == NULL)
		return;
		
	switch (theMenuItem) {
		case IDM_MAKE_ICONS_MOVIE:
		case IDM_MAKE_PENGUIN_MOVIE:
			theColor->red = EndianU16_NtoB(0xffff);
			theColor->green = EndianU16_NtoB(0xffff);
			theColor->blue = EndianU16_NtoB(0xffff);
			break;
		case IDM_MAKE_SPACE_MOVIE:
			theColor->red = EndianU16_NtoB(0x8000);
			theColor->green = EndianU16_NtoB(0);
			theColor->blue = EndianU16_NtoB(0xffff);
			break;
	}
}


//////////
//
// QTSprites_AddIconMovieSamplesToMedia
// Build the key frames and override frames for the icon sprite movie.
//
//////////

void QTSprites_AddIconMovieSamplesToMedia (Media theMedia)
{
	QTAtomContainer			mySample = NULL;
	QTAtomContainer			mySpriteData = NULL;
	RGBColor				myKeyColor;
	Point					myLocation;
	short					isVisible;
	short					myLayer, myIndex, myCount;
	OSErr					myErr = noErr;
	
	//////////
	//
	// create a key frame sample containing the sprite images
	//
	//////////

	// create a new, empty key frame sample
	myErr = QTNewAtomContainer(&mySample);
	if (myErr != noErr)
		goto bail;

	myKeyColor.red = myKeyColor.green = myKeyColor.blue = 0xffff;		// white

	// add images to the key frame sample
	SpriteUtils_AddPICTImageToKeyFrameSample(mySample, kOldQTIconID, &myKeyColor, 1, NULL, NULL);
	SpriteUtils_AddPICTImageToKeyFrameSample(mySample, kNewQTIconID, &myKeyColor, 2, NULL, NULL);

	//////////
	//
	// add the initial sprite properties to the key frame sample
	//
	//////////
	
	myErr = QTNewAtomContainer(&mySpriteData);
	if (myErr != noErr)
		goto bail;

	// the QT icon sprite
	myLocation.h 	= 32;
	myLocation.v	= 32;
	isVisible		= true;
	myLayer			= -1;
	myIndex			= 1;
	
	SpriteUtils_SetSpriteData(mySpriteData, &myLocation, &isVisible, &myLayer, &myIndex, NULL, NULL, NULL);
	SpriteUtils_AddSpriteToSample(mySample, mySpriteData, kQTIconSpriteAtomID);
	SpriteUtils_AddSpriteSampleToMedia(theMedia, mySample, kSpriteMediaFrameDurationIcon, true, NULL);	

	//////////
	//
	// add a few override samples to change the icon's location and image
	//
	//////////
	
	for (myCount = 1; myCount <= kNumOverrideSamples; myCount++) {
		QTRemoveChildren(mySample, kParentAtomIsContainer);
		QTRemoveChildren(mySpriteData, kParentAtomIsContainer);
		
		// every frame, bump the icon's location
		myLocation.h += 2;
		
		// change icon half way thru
		if (myCount == kNumOverrideSamples / 2)
			myIndex = 2;

		SpriteUtils_SetSpriteData(mySpriteData, &myLocation, NULL, NULL, &myIndex, NULL, NULL, NULL);
		SpriteUtils_AddSpriteToSample(mySample, mySpriteData, kQTIconSpriteAtomID);
		SpriteUtils_AddSpriteSampleToMedia(theMedia, mySample, kSpriteMediaFrameDurationIcon, false, NULL);	
	}

bail:	
	if (mySample != NULL)
		QTDisposeAtomContainer(mySample);

	if (mySpriteData != NULL)
		QTDisposeAtomContainer(mySpriteData);	
}


//////////
//
// QTSprites_AddPenguinMovieSamplesToMedia
// Build the key frames and override frames for the penguin sprite movie.
//
//////////

void QTSprites_AddPenguinMovieSamplesToMedia (Media theMedia)
{
	QTAtomContainer			mySample = NULL;
	QTAtomContainer			mySpriteData = NULL;
	RGBColor				myKeyColor;
	Point					myLocation;
	short					isVisible;
	short					myLayer, myIndex, myCount;
	ModifierTrackGraphicsModeRecord 
							myGraphicsMode;
	OSErr					myErr = noErr;
	
	//////////
	//
	// create a key frame sample containing the sprite images
	//
	//////////

	// create a new, empty key frame sample
	myErr = QTNewAtomContainer(&mySample);
	if (myErr != noErr)
		goto bail;

	myKeyColor.red = myKeyColor.green = myKeyColor.blue = 0xffff;		// white

	// add images to the key frame sample
	SpriteUtils_AddPICTImageToKeyFrameSample(mySample, kPenguinPictID, &myKeyColor, kPenguinImageIndex, NULL, NULL);

	//////////
	//
	// add the initial sprite properties to the key frame sample
	//
	//////////
	
	myErr = QTNewAtomContainer(&mySpriteData);
	if (myErr != noErr)
		goto bail;

	// the penguin sprite
	myLocation.h 	= 0;
	myLocation.v	= 0;
	isVisible		= true;
	myLayer			= -1;
	myIndex			= 1;
	
	// set the initial blend amount (0 = fully transparent; 0xffff = fully opaque)
	myGraphicsMode.graphicsMode = blend;
	myGraphicsMode.opColor.red = 0;
	myGraphicsMode.opColor.green = 0;
	myGraphicsMode.opColor.blue = 0;
	
	SpriteUtils_SetSpriteData(mySpriteData, &myLocation, &isVisible, &myLayer, &myIndex, &myGraphicsMode, NULL, NULL);
	SpriteUtils_AddSpriteToSample(mySample, mySpriteData, kPenguinSpriteAtomID);
	SpriteUtils_AddSpriteSampleToMedia(theMedia, mySample, kSpriteMediaFrameDurationPenguin, true, NULL);	

	//////////
	//
	// add a few override samples to change the penguin's opacity
	//
	//////////

	for (myCount = 1; myCount <= kNumOverrideSamples; myCount++) {
		QTRemoveChildren(mySample, kParentAtomIsContainer);
		QTRemoveChildren(mySpriteData, kParentAtomIsContainer);

		// every frame, bump the penguin's opacity
		myGraphicsMode.graphicsMode = blend;
		myGraphicsMode.opColor.red = (myCount - 1) * (0xffff / kNumOverrideSamples - 1);
		myGraphicsMode.opColor.green = (myCount - 1) * (0xffff / kNumOverrideSamples - 1);
		myGraphicsMode.opColor.blue = (myCount - 1) * (0xffff / kNumOverrideSamples - 1);
			
		SpriteUtils_SetSpriteData(mySpriteData, NULL, NULL, NULL, NULL, &myGraphicsMode, NULL, NULL);
		SpriteUtils_AddSpriteToSample(mySample, mySpriteData, kPenguinSpriteAtomID);
		SpriteUtils_AddSpriteSampleToMedia(theMedia, mySample, kSpriteMediaFrameDurationPenguin, false, NULL);	
	}

bail:	
	if (mySample != NULL)
		QTDisposeAtomContainer(mySample);

	if (mySpriteData != NULL)
		QTDisposeAtomContainer(mySpriteData);	
}


//////////
//
// QTSprites_AddSpaceMovieSamplesToMedia
// Build the key frames and override frames for the space sprite movie.
//
//////////

void QTSprites_AddSpaceMovieSamplesToMedia (Media theMedia)
{
	QTAtomContainer			mySample = NULL;
	QTAtomContainer			mySpriteData = NULL;
	RGBColor				myKeyColor;
	Point					myLocation, myIconLocation;
	short					isVisible;
	short					myLayer, myIndex, myCount, myDelta, myIconMinH, myIconMaxH;
	OSErr					myErr = noErr;
	
	//////////
	//
	// create a key frame sample containing the sprite images
	//
	//////////

	// create a new, empty key frame sample
	myErr = QTNewAtomContainer(&mySample);
	if (myErr != noErr)
		goto bail;

	myKeyColor.red = myKeyColor.green = myKeyColor.blue = 0xffff;		// white

	// add images to the key frame sample
	SpriteUtils_AddPICTImageToKeyFrameSample(mySample, kIconPictID, &myKeyColor, kIconImageIndex, NULL, NULL);
	SpriteUtils_AddPICTImageToKeyFrameSample(mySample, kWorldPictID, &myKeyColor, kWorldImageIndex, NULL, NULL);
	SpriteUtils_AddPICTImageToKeyFrameSample(mySample, kBackgroundPictID, &myKeyColor, kBackgroundImageIndex, NULL, NULL);
	for (myCount = 1; myCount <= kNumSpaceShipImages; myCount++)
		SpriteUtils_AddPICTImageToKeyFrameSample(mySample, kFirstSpaceShipPictID + myCount - 1, &myKeyColor, myCount + (kFirstSpaceShipImageIndex - 1), NULL, NULL);

	//////////
	//
	// add the initial sprite properties to the key frame sample
	//
	//////////
	
	myErr = QTNewAtomContainer(&mySpriteData);
	if (myErr != noErr)
		goto bail;

	// the background image
	if (gUseBackgroundPicture) {
		myLocation.h	= 0;
		myLocation.v	= 0;
		isVisible		= true;
		myLayer			= kBackgroundSpriteLayerNum;			// this makes the sprite a background sprite
		myIndex			= kBackgroundImageIndex;
		myErr = SpriteUtils_SetSpriteData(mySpriteData, &myLocation, &isVisible, &myLayer, &myIndex, NULL, NULL, NULL);
		if (myErr != noErr)
			goto bail;
		SpriteUtils_AddSpriteToSample(mySample, mySpriteData, kBackgroundSpriteAtomID);
	}

	// the space ship sprite
	myLocation.h 	= 0;
	myLocation.v	= 60;
	isVisible		= true;
	myLayer			= -1;
	myIndex			= kFirstSpaceShipImageIndex;
	myErr = SpriteUtils_SetSpriteData(mySpriteData, &myLocation, &isVisible, &myLayer, &myIndex, NULL, NULL, NULL);
	if (myErr != noErr)
		goto bail;
	SpriteUtils_AddSpriteToSample(mySample, mySpriteData, kSpaceShipSpriteAtomID);

	// the world sprite
	myLocation.h 	= (kSpaceSpriteTrackWidth / 2) - 24;
	myLocation.v	= (kSpaceSpriteTrackHeight / 2) - 24;
	isVisible		= true;
	myLayer			= 1;
	myIndex			= kWorldImageIndex;
	myErr = SpriteUtils_SetSpriteData(mySpriteData, &myLocation, &isVisible, &myLayer, &myIndex, NULL, NULL, NULL);
	if (myErr != noErr)
		goto bail;
	SpriteUtils_AddSpriteToSample(mySample, mySpriteData, kWorldSpriteAtomID);

	// the icon sprite
	myIconMinH			= (kSpaceSpriteTrackWidth / 2) - 116;
	myIconMaxH			= myIconMinH + 200;
	myDelta				= 2;
	myIconLocation.h 	= myIconMinH;
	myIconLocation.v	= (kSpaceSpriteTrackHeight / 2) - (24 + 12);
	isVisible			= true;
	myLayer				= 0;
	myIndex				= kIconImageIndex;
	myErr = SpriteUtils_SetSpriteData(mySpriteData, &myIconLocation, &isVisible, &myLayer, &myIndex, NULL, NULL, NULL);
	if (myErr != noErr)
		goto bail;
	SpriteUtils_AddSpriteToSample(mySample, mySpriteData, kIconSpriteAtomID);
	
	//////////
	//
	// add the key frame sample to the sprite track media
	//
	// to add the sample data in a compressed form, you would use a QuickTime DataCodec to perform the
	// compression; replace the call to the utility SpriteUtils_AddSpriteSampleToMedia with a call to the utility
	// SpriteUtils_AddCompressedSpriteSampleToMedia to do this
	//
	//////////
	
	SpriteUtils_AddSpriteSampleToMedia(theMedia, mySample, kSpriteMediaFrameDurationSpace, true, NULL);	
	//SpriteUtils_AddCompressedSpriteSampleToMedia(theMedia, mySample, kSpriteMediaFrameDurationSpace, true, zlibDataCompressorSubType, NULL);

	//////////
	//
	// add a few override samples to move the space ship and icon, and to change the icon's layer
	//
	//////////

	// original space ship location
	myIndex			= kFirstSpaceShipImageIndex;
	myLocation.h 	= 0;
	myLocation.v 	= 80;
	isVisible		= true;
	
	for (myCount = 1; myCount <= kNumOverrideSamplesSpace; myCount++) {
		QTRemoveChildren(mySample, kParentAtomIsContainer);
		QTRemoveChildren(mySpriteData, kParentAtomIsContainer);

		// every third frame, bump the space ship's image index (so that it spins as it moves)
		if ((myCount % 3) == 0) {
			myIndex++;
			if (myIndex > kLastSpaceShipImageIndex)
				myIndex = kFirstSpaceShipImageIndex;
		}

		// every frame, bump the space ship's location (so that it moves as it spins)
		myLocation.h += 2;
		myLocation.v += 1;
		
		if (isVisible)
			SpriteUtils_SetSpriteData(mySpriteData, &myLocation, NULL, NULL, &myIndex, NULL, NULL, NULL);
		else {
			isVisible = true;
			SpriteUtils_SetSpriteData(mySpriteData, &myLocation, &isVisible, NULL, &myIndex, NULL, NULL, NULL);
		}
				
		SpriteUtils_AddSpriteToSample(mySample, mySpriteData, kSpaceShipSpriteAtomID);
		
		// make the icon move and change layer
		QTRemoveChildren(mySpriteData, kParentAtomIsContainer);
		myIconLocation.h += myDelta;
		
		if (myIconLocation.h >= myIconMaxH) {
			myIconLocation.h = myIconMaxH;
			myDelta = -myDelta;
		}
		
		if (myIconLocation.h <= myIconMinH) {
			myIconLocation.h = myIconMinH;
			myDelta = -myDelta;
		}
		
		if (myDelta > 0)
			myLayer = 0;
		else
			myLayer = 3;
			
		SpriteUtils_SetSpriteData(mySpriteData, &myIconLocation, NULL, &myLayer, NULL, NULL, NULL, NULL);
		SpriteUtils_AddSpriteToSample(mySample, mySpriteData, kIconSpriteAtomID);
		SpriteUtils_AddSpriteSampleToMedia(theMedia, mySample, kSpriteMediaFrameDurationSpace, false, NULL);	
	}

bail:	
	if (mySample != NULL)
		QTDisposeAtomContainer(mySample);

	if (mySpriteData != NULL)
		QTDisposeAtomContainer(mySpriteData);	
}


//////////
//
// QTSprites_SetTrackProperties
// Set the track properties for the specified sample sprite movie.
//
//////////

void QTSprites_SetTrackProperties (Media theMedia, UInt16 theMenuItem)
{
	QTAtomContainer		myTrackProperties;
	RGBColor			myBackgroundColor;
	OSErr				myErr = noErr;
		
	if (!gUseBackgroundPicture) {
		// add a background color to the sprite track
		QTSprites_GetBackgroundColor(theMenuItem, &myBackgroundColor);
		
		myErr = QTNewAtomContainer(&myTrackProperties);
		if (myErr == noErr) {
			QTInsertChild(myTrackProperties, 0, kSpriteTrackPropertyBackgroundColor, 1, 1, sizeof(RGBColor), &myBackgroundColor, NULL);

			SetMediaPropertyAtom(theMedia, myTrackProperties);
			
			QTDisposeAtomContainer(myTrackProperties);
		}
	}
}
	

//////////
//
// QTSprites_HitTestSprites
// Determine whether a mouse click is on a sprite; return true if it is, false otherwise.
//
// This routine is intended to be called from your movie controller action filter function,
// in response to mcActionMouseDown actions.
//
//////////

Boolean QTSprites_HitTestSprites (WindowObject theWindowObject, EventRecord *theEvent)
{
	ApplicationDataHdl		myAppData = NULL;
	MediaHandler			myHandler = NULL;
	Boolean					isHandled = false;
	long					myFlags = 0L;
	QTAtomID				myAtomID = 0;
	Point					myPoint;
	ComponentResult			myErr = noErr;

	myAppData = (ApplicationDataHdl)QTFrame_GetAppDataFromWindowObject(theWindowObject);
	if (myAppData == NULL)
		goto bail;
		
	if (theEvent == NULL)
		goto bail;
		
	// make sure that the click is in the frontmost window
	if ((**theWindowObject).fWindow != QTFrame_GetFrontMovieWindow())
		goto bail;
		
	myHandler = (**myAppData).fSpriteHandler;
	myFlags = spriteHitTestImage | spriteHitTestLocInDisplayCoordinates | spriteHitTestInvisibleSprites;
	myPoint = theEvent->where;
	
	myErr = SpriteMediaHitTestAllSprites(myHandler, myFlags, myPoint, &myAtomID);
	if ((myErr == noErr) && (myAtomID != 0)) {
		Boolean				isVisible;
		
		// the user has clicked on a sprite;
		// for now, we'll just toggle the visibility state of the sprite
		SpriteMediaGetSpriteProperty(myHandler, myAtomID, kSpritePropertyVisible, (void *)&isVisible);
		SpriteMediaSetSpriteProperty(myHandler, myAtomID, kSpritePropertyVisible, (void *)!isVisible);

		isHandled = true;
	}

bail:
	return(isHandled);
}


