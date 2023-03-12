//////////
//
//	File:		QTSpritesPlus.c
//
//	Contains:	Using video overrides and tweening with sprites in QuickTime movies.
//
//	Written by:	Tim Monroe
//
//	Copyright:	© 2001 by Apple Computer, Inc., all rights reserved.
//
//	Change History (most recent first):
//
//	   <2>	 	01/19/01	rtm		added multimatrix tween code
//	   <1>	 	01/18/01	rtm		first file; based on existing QTSprites sample code
//	   
//	This sample code shows how to use video overrides and tween tracks with sprite movies.
//
//////////


//////////
//
// header files
//
//////////

#include "QTSpritesPlus.h"


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

	myErr = BeginMediaEdits(myMedia);
	if (myErr != noErr)
		goto bail;

	//////////
	//
	// add the appropriate samples to the sprite media
	//
	//////////
	
	switch (theMenuItem) {
		case IDM_MAKE_VIDEO_SPRITE_MOVIE:
			QTSprites_AddPowerBookMovieSamplesToMedia(myMedia);
			break;
		case IDM_MAKE_MOVE_ICON_MOVIE:
		case IDM_MAKE_SPIN_ICON_MOVIE:
		case IDM_MAKE_ROLL_ICON_MOVIE:
			QTSprites_AddIconMovieSamplesToMedia(myMedia);
			break;
		case IDM_MAKE_PENGUIN_MOVIE:
			QTSprites_AddPenguinMovieSamplesToMedia(myMedia);
			break;
		default:
			goto bail;
	}
	
	myErr = EndMediaEdits(myMedia);
	if (myErr != noErr)
		goto bail;
	
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
	// add video override or tween track to movie
	//
	//////////
	
	switch (theMenuItem) {
		case IDM_MAKE_VIDEO_SPRITE_MOVIE:
			QTSprites_AddVideoOverrideTrack(myMovie, myTrack);
			break;
		case IDM_MAKE_MOVE_ICON_MOVIE:
			QTSprites_AddTweenOverrideTrack(myMovie, myTrack, kTweenTypeMatrix);
			break;
		case IDM_MAKE_SPIN_ICON_MOVIE:
			QTSprites_AddTweenOverrideTrack(myMovie, myTrack, kTweenTypeSpin);
			break;
		case IDM_MAKE_ROLL_ICON_MOVIE:
			QTSprites_AddTweenOverrideTrack(myMovie, myTrack, kTweenTypeMultiMatrix);
			break;
		case IDM_MAKE_PENGUIN_MOVIE:
			QTSprites_AddTweenOverrideTrack(myMovie, myTrack, kTweenTypeGraphicsModeWithRGBColor);
			break;
		default:
			goto bail;
	}
	
	//////////
	//
	// add the movie resource to the movie file
	//
	//////////
	
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
		case IDM_MAKE_VIDEO_SPRITE_MOVIE:
			*theWidth = Long2Fix(kPowerBookSpriteTrackWidth);
			*theHeight = Long2Fix(kPowerBookSpriteTrackHeight);
			break;
		case IDM_MAKE_MOVE_ICON_MOVIE:
		case IDM_MAKE_SPIN_ICON_MOVIE:
		case IDM_MAKE_ROLL_ICON_MOVIE:
			*theWidth = Long2Fix(kIconSpriteTrackWidth);
			*theHeight = Long2Fix(kIconSpriteTrackHeight);
			break;
		case IDM_MAKE_PENGUIN_MOVIE:
			*theWidth = Long2Fix(kPenguinSpriteTrackWidth);
			*theHeight = Long2Fix(kPenguinSpriteTrackHeight);
			break;
	}
}


//////////
//
// QTSprites_AddIconMovieSamplesToMedia
// Build the key frame for the icon sprite movie.
//
//////////

void QTSprites_AddIconMovieSamplesToMedia (Media theMedia)
{
	QTAtomContainer			mySample = NULL;
	QTAtomContainer			mySpriteData = NULL;
	RGBColor				myKeyColor;
	Point					myLocation;
	short					isVisible, myIndex;
	FixedPoint				myPoint;
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
	
	myPoint.x = Long2Fix(kIconDimension / 2);
	myPoint.y = Long2Fix(kIconDimension / 2);

	// add images to the key frame sample
	SpriteUtils_AddPICTImageToKeyFrameSample(mySample, kOldQTIconID, &myKeyColor, 1, &myPoint, NULL);

	//////////
	//
	// add the initial sprite properties to the key frame sample
	//
	//////////
	
	myErr = QTNewAtomContainer(&mySpriteData);
	if (myErr != noErr)
		goto bail;

	// the QT icon sprite
	myLocation.h 	= kIconDimension + (kIconDimension / 2);
	myLocation.v	= kIconDimension + (kIconDimension / 2);
	isVisible		= true;
	myIndex			= kOldQTIconImageIndex;
	
	SpriteUtils_SetSpriteData(mySpriteData, &myLocation, &isVisible, NULL, &myIndex, NULL, NULL, NULL);
	SpriteUtils_AddSpriteToSample(mySample, mySpriteData, kQTIconSpriteAtomID);
	SpriteUtils_AddSpriteSampleToMedia(theMedia, mySample, kSpriteMediaFrameDurationIcon, true, NULL);	

	//////////
	//
	// there are no override samples here, since we'll use tweening to move the icon
	//
	//////////
	

bail:	
	if (mySample != NULL)
		QTDisposeAtomContainer(mySample);

	if (mySpriteData != NULL)
		QTDisposeAtomContainer(mySpriteData);	
}


//////////
//
// QTSprites_AddPowerBookMovieSamplesToMedia
// Build the key frame for the PowerBook sprite movie.
//
//////////

void QTSprites_AddPowerBookMovieSamplesToMedia (Media theMedia)
{
	QTAtomContainer			mySample = NULL;
	QTAtomContainer			mySpriteData = NULL;
	RGBColor				myKeyColor;
	Point					myLocation;
	short					isVisible, myIndex, myLayer;
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
	SpriteUtils_AddPICTImageToKeyFrameSample(mySample, kTitaniumPowerBookID, &myKeyColor, 2, NULL, NULL);

	//////////
	//
	// add the initial sprite properties to the key frame sample
	//
	//////////
	
	myErr = QTNewAtomContainer(&mySpriteData);
	if (myErr != noErr)
		goto bail;

	// the QT icon sprite
	myLocation.h 	= 46;
	myLocation.v	= 8;
	isVisible		= true;
	myIndex			= kOldQTIconImageIndex;
	myLayer			= 1;
	
	SpriteUtils_SetSpriteData(mySpriteData, &myLocation, &isVisible, &myLayer, &myIndex, NULL, NULL, NULL);
	SpriteUtils_AddSpriteToSample(mySample, mySpriteData, kQTIconSpriteAtomID);

	// the PowerBook sprite
	myLocation.h 	= 0;
	myLocation.v	= 0;
	isVisible		= true;
	myIndex			= kPowerBookImageIndex;
	myLayer			= 2;

	SpriteUtils_SetSpriteData(mySpriteData, &myLocation, &isVisible, &myLayer, &myIndex, NULL, NULL, NULL);
	SpriteUtils_AddSpriteToSample(mySample, mySpriteData, kTitaniumPowerBookID);
	SpriteUtils_AddSpriteSampleToMedia(theMedia, mySample, kSpriteMediaFrameDurationPowerBook, true, NULL);	

	//////////
	//
	// there are no override samples here, since we'll use a video override track to animate the icon sprite
	//
	//////////

bail:	
	if (mySample != NULL)
		QTDisposeAtomContainer(mySample);

	if (mySpriteData != NULL)
		QTDisposeAtomContainer(mySpriteData);	
}


//////////
//
// QTSprites_AddPenguinMovieSamplesToMedia
// Build the key frame for the penguin sprite movie.
//
//////////

void QTSprites_AddPenguinMovieSamplesToMedia (Media theMedia)
{
	QTAtomContainer			mySample = NULL;
	QTAtomContainer			mySpriteData = NULL;
	RGBColor				myKeyColor;
	Point					myLocation;
	short					isVisible, myIndex;
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
	myIndex			= 1;
	
	SpriteUtils_SetSpriteData(mySpriteData, &myLocation, &isVisible, NULL, &myIndex, NULL, NULL, NULL);
	SpriteUtils_AddSpriteToSample(mySample, mySpriteData, kPenguinSpriteAtomID);
	SpriteUtils_AddSpriteSampleToMedia(theMedia, mySample, kSpriteMediaFrameDurationPenguin, true, NULL);	

	//////////
	//
	// there are no override samples here, since we'll use tweening to animate the movie
	//
	//////////

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
#pragma unused(theMenuItem)
	QTAtomContainer		myTrackProperties;
	RGBColor			myBackgroundColor;
	OSErr				myErr = noErr;
		
	// add a background color to the sprite track
	myBackgroundColor.red = EndianU16_NtoB(0xffff);
	myBackgroundColor.green = EndianU16_NtoB(0xffff);
	myBackgroundColor.blue = EndianU16_NtoB(0xffff);
	
	myErr = QTNewAtomContainer(&myTrackProperties);
	if (myErr == noErr) {
		QTInsertChild(myTrackProperties, 0, kSpriteTrackPropertyBackgroundColor, 1, 1, sizeof(myBackgroundColor), &myBackgroundColor, NULL);

		SetMediaPropertyAtom(theMedia, myTrackProperties);
		
		QTDisposeAtomContainer(myTrackProperties);
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


//////////
//
// QTSprites_AddVideoOverrideTrack
// Prompt the user for a movie and use the first video track in that movie as an override track
// for the first sprite image in the specified sprite movie.
//
//////////

OSErr QTSprites_AddVideoOverrideTrack (Movie theSpriteMovie, Track theSpriteTrack)
{
	Movie					myVideoMovie = NULL;
	Track					myVideoTrack = NULL;
	short					myRefNum = kInvalidFileRefNum;
	short					myResID = 0;
	FSSpec					myFSSpec;
	OSType 					myTypeList[] = {kQTFileTypeMovie};
	short					myNumTypes = 2;
	QTFrameFileFilterUPP	myFileFilterUPP = NULL;
	TimeValue				myDuration;
	long					myRefIndex;
	QTAtomContainer			myInputMap = NULL;
	OSErr					myErr = noErr;

#if TARGET_OS_MAC
	myNumTypes = 0;
#endif

	//////////
	//
	// have the user select a file; make sure it has a video track in it
	//
	//////////
	
retry:
	myFileFilterUPP = QTFrame_GetFileFilterUPP((ProcPtr)QTFrame_FilterFiles);

	myErr = QTFrame_GetOneFileWithPreview(myNumTypes, (QTFrameTypeListPtr)myTypeList, &myFSSpec, myFileFilterUPP);

	if (myFileFilterUPP != NULL)
		DisposeNavObjectFilterUPP(myFileFilterUPP);

	if (myErr != noErr)
		goto bail;

	myErr = OpenMovieFile(&myFSSpec, &myRefNum, fsRdPerm);
	if (myErr != noErr)
		goto bail;

	// now fetch the first movie from the file
	myResID = 0;
	myErr = NewMovieFromFile(&myVideoMovie, myRefNum, &myResID, NULL, newMovieActive, NULL);
	if (myErr != noErr)
		goto bail;
		
	myVideoTrack = GetMovieIndTrackType(myVideoMovie, 1, VideoMediaType, movieTrackMediaType);
	if (myVideoTrack == NULL)
		goto retry;
		
	//////////
	//
	// copy the video track into the sprite movie
	//
	//////////
	
	myDuration = GetMovieDuration(theSpriteMovie);
	
	myErr = QTSprites_ImportVideoTrack(myVideoMovie, theSpriteMovie, &myVideoTrack);
	if (myErr != noErr)
		goto bail;

	// truncate the new video track to the length of the sprite movie
	DeleteMovieSegment(theSpriteMovie, myDuration, GetMovieDuration(theSpriteMovie) - myDuration);

	//////////
	//
	// attach the video track as a modifier to the sprite track
	//
	//////////
	
	// create a media input map
	myErr = QTNewAtomContainer(&myInputMap);
	if (myErr != noErr)
		goto bail;
		
	myErr = AddTrackReference(theSpriteTrack, myVideoTrack, kTrackModifierReference, &myRefIndex);
	if (myErr != noErr)
		goto bail;

	myErr = QTSprites_AddVideoEntryToInputMap(myInputMap, myRefIndex, kOldQTIconImageIndex, kTrackModifierTypeImage, NULL);
	if (myErr != noErr)
		goto bail;
	
	// attach the input map to the sprite track
	myErr = SetMediaInputMap(GetTrackMedia(theSpriteTrack), myInputMap);

bail:
	if (myVideoMovie != NULL)
		DisposeMovie(myVideoMovie);
		
	if (myRefNum != kInvalidFileRefNum)
		CloseMovieFile(myRefNum);
		
	if (myInputMap != NULL)
		QTDisposeAtomContainer(myInputMap);

	return(myErr);
}


//////////
//
// QTSprites_AddVideoEntryToInputMap
// Add a video entry to the specified input map.
//
//////////

OSErr QTSprites_AddVideoEntryToInputMap (QTAtomContainer theInputMap, long theRefIndex, long theID, OSType theType, char *theName)
{
#pragma unused(theName)
	QTAtom				myInputAtom;
	OSErr				myErr = noErr;
	
	// add an entry to the input map
	myErr = QTInsertChild(theInputMap, kParentAtomIsContainer, kTrackModifierInput, theRefIndex, 0, 0, NULL, &myInputAtom);
	if (myErr != noErr)
		goto bail;
	
	// add two child atoms to the parent atom;
	// these atoms define the type of the modifier input and the image index to override
	theType = EndianU32_NtoB(theType);
	myErr = QTInsertChild(theInputMap, myInputAtom, kTrackModifierType, 1, 0, sizeof(OSType), &theType, NULL);
	if (myErr != noErr)
		goto bail;
	
	theID = EndianS32_NtoB(theID);
	myErr = QTInsertChild(theInputMap, myInputAtom, kSpritePropertyImageIndex, 1, 0, sizeof(long), &theID, NULL);
			
bail:
	return(myErr);
}


//////////
//
// QTSprites_AddTweenOverrideTrack
// Add a tween track to the specified movie, tweening the specified track.
//
//////////

void QTSprites_AddTweenOverrideTrack (Movie theMovie, Track theTargetTrack, UInt32 theTweenType)
{
	Handle						myHandle = NULL;
	SampleDescriptionHandle		mySampleDesc = NULL;
	short						myResRefNum = 0;
	short						myResID = movieInDataForkResID;
	Track						myTweenTrack;
	Media						myTweenMedia;
	QTAtomContainer				mySample = NULL;
	QTAtomContainer				myInputMap = NULL;
	QTAtom						myTweenEntryAtom = 0;
	QTAtomType					myType;
	long						myRefIndex;
	OSErr						myErr = noErr;

	//////////
	//
	// create the tween track and media, and a sample to contain the tween data
	//
	//////////

	// create the tween track and media
	myTweenTrack = NewMovieTrack(theMovie, 0, 0, kNoVolume);
	myTweenMedia = NewTrackMedia(myTweenTrack, TweenMediaType, GetMovieTimeScale(theMovie), NULL, 0);

	// create a new sample; this sample will hold the tween data
	myErr = QTNewAtomContainer(&mySample);
	if (myErr != noErr)
		goto bail;
		
	//////////
	//
	// add a tween entry atom to the atom container
	//
	//////////

	myErr = QTInsertChild(mySample, kParentAtomIsContainer, kTweenEntry, 1, 0, 0, NULL, &myTweenEntryAtom);
	if (myErr != noErr)
		goto bail;
	
	// set the type of this tween entry
	// a kTweenEntry atom can contain only one kTweenType atom, whose ID and index must be 1
	myType = EndianU32_NtoB(theTweenType);
	myErr = QTInsertChild(mySample, myTweenEntryAtom, kTweenType, 1, 0, sizeof(myType), &myType, NULL);
	if (myErr != noErr)
		goto bail;

	//////////
	//
	// add a tween data atom to the tween entry atom
	//
	//////////

	if (theTweenType == kTweenTypeMatrix) {
		MatrixRecord			myMatrix[2];
	
		// set the initial data for this tween entry
		SetIdentityMatrix(&myMatrix[0]);
		TranslateMatrix(&myMatrix[0], Long2Fix(kIconDimension + (kIconDimension / 2)), Long2Fix(kIconDimension + (kIconDimension / 2)));
		EndianUtils_MatrixRecord_NtoB(&myMatrix[0]);
		
		// set the final data for this tween entry
		SetIdentityMatrix(&myMatrix[1]);
		TranslateMatrix(&myMatrix[1], Long2Fix(230 + (kIconDimension / 2)), Long2Fix(kIconDimension + (kIconDimension / 2)));
		EndianUtils_MatrixRecord_NtoB(&myMatrix[1]);
		
		myErr = QTInsertChild(mySample, myTweenEntryAtom, kTweenData, 1, 0, 2 * sizeof(MatrixRecord), myMatrix, NULL);
		if (myErr != noErr)
			goto bail;
		
	} else if (theTweenType == kTweenTypeGraphicsModeWithRGBColor) {
		ModifierTrackGraphicsModeRecord  	myGraphicsMode[2];
		
		// set the initial blend amount (0 = fully transparent)
		myGraphicsMode[0].graphicsMode = EndianU32_NtoB(blend);
		myGraphicsMode[0].opColor.red = 0;
		myGraphicsMode[0].opColor.green = 0;
		myGraphicsMode[0].opColor.blue = 0;

		// set the final blend amount (0xffff = fully opaque)
		myGraphicsMode[1].graphicsMode = EndianU32_NtoB(blend);
		myGraphicsMode[1].opColor.red = EndianU16_NtoB(0xffff);
		myGraphicsMode[1].opColor.green = EndianU16_NtoB(0xffff);
		myGraphicsMode[1].opColor.blue = EndianU16_NtoB(0xffff);
		
		myErr = QTInsertChild(mySample, myTweenEntryAtom, kTweenData, 1, 0, 2 * sizeof(ModifierTrackGraphicsModeRecord), myGraphicsMode, NULL);
		if (myErr != noErr)
			goto bail;
			
	} else if (theTweenType == kTweenTypeSpin) {
		Fixed  	mySpinData[2];
		
		// set the initial rotation value
		mySpinData[0] = EndianU32_NtoB(0);

		// set the number of rotations
		mySpinData[1] = EndianU32_NtoB(Long2Fix(kNumRotations));
		
		myErr = QTInsertChild(mySample, myTweenEntryAtom, kTweenData, 1, 0, 2 * sizeof(Fixed), mySpinData, NULL);
		if (myErr != noErr)
			goto bail;

	} else if (theTweenType == kTweenTypeMultiMatrix) {
		
		QTAtom						myMultiTweenDataAtom = 0;
		QTAtom						myAtom = 0;
		MatrixRecord				myMatrix[2];
		Fixed  						mySpinData[2];
		
		//////////
		//
		// add a multimatrix tween data atom to the tween entry atom
		//
		//////////

		myErr = QTInsertChild(mySample, myTweenEntryAtom, kTweenData, 1, 0, 0, NULL, &myMultiTweenDataAtom);
		if (myErr != noErr)
			goto bail;
			
		//////////
		//
		// add a spin tween to the multimatrix tween data atom
		//
		//////////

		myErr = QTInsertChild(mySample, myMultiTweenDataAtom, kTweenEntry, 1, 0, 0, NULL, &myAtom);
		if (myErr != noErr)
			goto bail;
		
		// set the type of this tween entry
		myType = EndianU32_NtoB(kTweenTypeSpin);
		myErr = QTInsertChild(mySample, myAtom, kTweenType, 1, 0, sizeof(myType), &myType, NULL);
		if (myErr != noErr)
			goto bail;

		// set the initial rotation value
		mySpinData[0] = EndianU32_NtoB(0);

		// set the number of rotations
		mySpinData[1] = EndianU32_NtoB(Long2Fix(kNumRotations));
		
		myErr = QTInsertChild(mySample, myAtom, kTweenData, 1, 0, 2 * sizeof(Fixed), mySpinData, NULL);
		if (myErr != noErr)
			goto bail;
		
		//////////
		//
		// add a translation matrix tween to the multimatrix tween data atom
		//
		//////////

		myErr = QTInsertChild(mySample, myMultiTweenDataAtom, kTweenEntry, 2, 0, 0, NULL, &myAtom);
		if (myErr != noErr)
			goto bail;
		
		// set the type of this tween entry
		myType = EndianU32_NtoB(kTweenTypeMatrix);
		myErr = QTInsertChild(mySample, myAtom, kTweenType, 1, 0, sizeof(myType), &myType, NULL);
		if (myErr != noErr)
			goto bail;

		// set the initial data for this tween entry
		SetIdentityMatrix(&myMatrix[0]);
		TranslateMatrix(&myMatrix[0], Long2Fix(kIconDimension + (kIconDimension / 2)), Long2Fix(kIconDimension + (kIconDimension / 2)));
		EndianUtils_MatrixRecord_NtoB(&myMatrix[0]);
		
		// set the final data for this tween entry
		SetIdentityMatrix(&myMatrix[1]);
		TranslateMatrix(&myMatrix[1], Long2Fix(230 + (kIconDimension / 2)), Long2Fix(kIconDimension + (kIconDimension / 2)));
		EndianUtils_MatrixRecord_NtoB(&myMatrix[1]);
		
		myErr = QTInsertChild(mySample, myAtom, kTweenData, 1, 0, 2 * sizeof(MatrixRecord), myMatrix, NULL);
		if (myErr != noErr)
			goto bail;
	}
	
	// create the sample description
	mySampleDesc = (SampleDescriptionHandle)NewHandleClear(sizeof(SampleDescription));
	if (mySampleDesc == NULL)
		goto bail;
	
	(**mySampleDesc).descSize = sizeof(SampleDescription);
		
	// add the tween sample to the media
	myErr = BeginMediaEdits(myTweenMedia);
	if (myErr != noErr)
		goto bail;
		
	myErr = AddMediaSample(myTweenMedia, mySample, 0, GetHandleSize(mySample), GetMediaDuration(GetTrackMedia(theTargetTrack)), (SampleDescriptionHandle)mySampleDesc, 1, 0, NULL);
	if (myErr != noErr)
		goto bail;
		
	myErr = EndMediaEdits(myTweenMedia);
	if (myErr != noErr)
		goto bail;
	
	// add the media to the track
	myErr = InsertMediaIntoTrack(myTweenTrack, 0, 0, GetMediaDuration(myTweenMedia), fixed1);
	if (myErr != noErr)
		goto bail;
	
	//////////
	//
	// create a link between the sprite track and the tween track, and update the sprite track's input map
	//
	//////////
	
	// create a media input map
	myErr = QTNewAtomContainer(&myInputMap);
	if (myErr != noErr)
		goto bail;
		
	// for *each* tween entry in the tween media sample,
	// create a link between the sprite track and the tween track, and add an entry to the input map
	myErr = AddTrackReference(theTargetTrack, myTweenTrack, kTrackModifierReference, &myRefIndex);
	if (myErr != noErr)
		goto bail;
		
	if (theTweenType == kTweenTypeMatrix)
		myErr = QTSprites_AddTweenEntryToInputMap(myInputMap, myRefIndex, kQTIconSpriteAtomID, kTrackModifierObjectMatrix, NULL);
		
	if (theTweenType == kTweenTypeGraphicsModeWithRGBColor)
		myErr = QTSprites_AddTweenEntryToInputMap(myInputMap, myRefIndex, kPenguinSpriteAtomID, kTrackModifierObjectGraphicsMode, NULL);

	if (theTweenType == kTweenTypeSpin)
		myErr = QTSprites_AddTweenEntryToInputMap(myInputMap, myRefIndex, 1, kTrackModifierObjectMatrix, NULL);
				
	if (theTweenType == kTweenTypeMultiMatrix)
		myErr = QTSprites_AddTweenEntryToInputMap(myInputMap, myRefIndex, 1, kTrackModifierObjectMatrix, NULL);
				
	if (myErr != noErr)
		goto bail;
	
    // attach the input map to the sprite track
    myErr = SetMediaInputMap(GetTrackMedia(theTargetTrack), myInputMap);
	
bail:
	if (mySample != NULL)
		QTDisposeAtomContainer(mySample);

	if (myInputMap != NULL)
		QTDisposeAtomContainer(myInputMap);

	if (mySampleDesc != NULL)
		DisposeHandle((Handle)mySampleDesc);
	
	if (myHandle != NULL)
		DisposeHandle(myHandle);

	return;
}


//////////
//
// QTSprites_AddTweenEntryToInputMap
// Add a tween entry to the specified input map.
//
//////////

OSErr QTSprites_AddTweenEntryToInputMap (QTAtomContainer theInputMap, long theRefIndex, long theID, OSType theType, char *theName)
{
#pragma unused(theName)
	QTAtom				myInputAtom;
	OSErr				myErr = noErr;
	
	// add an entry to the input map
	myErr = QTInsertChild(theInputMap, kParentAtomIsContainer, kTrackModifierInput, theRefIndex, 0, 0, NULL, &myInputAtom);
	if (myErr != noErr)
		goto bail;
	
	// add two child atoms to the parent atom;
	// these atoms define the type of the modifier input and the ID of the sprite to receive the tween data
	theType = EndianU32_NtoB(theType);
	myErr = QTInsertChild(theInputMap, myInputAtom, kTrackModifierType, 1, 0, sizeof(OSType), &theType, NULL);
	if (myErr != noErr)
		goto bail;
			
	theID = EndianU32_NtoB(theID);
	myErr = QTInsertChild(theInputMap, myInputAtom, kTrackModifierObjectID, 1, 0, sizeof(long), &theID, NULL);
		
bail:
	return(myErr);
}


//////////
//
// QTSprites_SetTweenEntryDuration
// Set the duration of the specified tween entry.
//
//////////

OSErr QTSprites_SetTweenEntryDuration (QTAtomContainer theSample, QTAtomID theID, TimeValue theDuration)
{
	QTAtom				myAtom;
	OSErr				myErr = noErr;
	
	// find the tween entry with the specified ID
	myAtom = QTFindChildByID(theSample, kParentAtomIsContainer, kTweenEntry, theID, NULL);
	if (myAtom == 0)
		return(paramErr);
	
	// add in the duration
	theDuration = EndianS32_NtoB(theDuration);
	// a kTweenEntry atom can contain only one kTweenDuration atom, whose ID and index must be 1
	myErr = QTInsertChild(theSample, myAtom, kTweenDuration, 1, 1, sizeof(TimeValue), &theDuration, NULL);
		
	return(myErr);
}


//////////
//
// QTSprites_SetTweenEntryStartOffset
// Set the starting offset of the specified tween entry.
//
//////////

OSErr QTSprites_SetTweenEntryStartOffset (QTAtomContainer theSample, QTAtomID theID, TimeValue theOffset)
{
	QTAtom				myAtom;
	OSErr				myErr = noErr;
	
	// find the tween entry with the specified ID
	myAtom = QTFindChildByID(theSample, kParentAtomIsContainer, kTweenEntry, theID, NULL);
	if (myAtom == 0)
		return(paramErr);
	
	// add in the starting offset
	// a kTweenEntry atom can contain only one kTweenStartOffset atom, whose ID and index must be 1
	theOffset = EndianS32_NtoB(theOffset);
	myErr = QTInsertChild(theSample, myAtom, kTweenStartOffset, 1, 1, sizeof(TimeValue), &theOffset, NULL);
		
	return(myErr);
}


//////////
//
// QTSprites_ImportVideoTrack
// Copy a video track from one movie (the source) to another (the destination); return the new track to the caller.
//
// We assume that we want the first video track in theSrcMovie.
//
//////////

OSErr QTSprites_ImportVideoTrack (Movie theSrcMovie, Movie theDstMovie, Track *theTrack)
{
	Track						mySrcTrack = NULL;
	Media						mySrcMedia = NULL;
	Track						myDstTrack = NULL;
	Media						myDstMedia = NULL;
	Fixed						myWidth, myHeight;
	OSErr						myErr = paramErr;

	// get the first video track in the source movie
	mySrcTrack = GetMovieIndTrackType(theSrcMovie, 1, VideoMediaType, movieTrackMediaType);
	if (mySrcTrack == NULL)
		goto bail;
		
	// get the track's media and dimensions
	mySrcMedia = GetTrackMedia(mySrcTrack);
	GetTrackDimensions(mySrcTrack, &myWidth, &myHeight);
	
	// create a destination track
	myDstTrack = NewMovieTrack(theDstMovie, myWidth, myHeight, kNoVolume);
	if (myDstTrack == NULL)
		goto bail;

	// create a destination media
	myDstMedia = NewTrackMedia(myDstTrack, VideoMediaType, GetMediaTimeScale(mySrcMedia), 0, 0);
	if (myDstMedia == NULL)
		goto bail;

	myErr = BeginMediaEdits(myDstMedia);
	if (myErr != noErr)
		goto bail;
		
	myErr = CopyTrackSettings(mySrcTrack, myDstTrack);
	if (myErr != noErr)
		goto bail;
		
	myErr = InsertTrackSegment(mySrcTrack, myDstTrack, 0, GetTrackDuration(mySrcTrack), 0);
	if (myErr != noErr)
		goto bail;

	myErr = EndMediaEdits(myDstMedia);
	if (myErr != noErr)
		goto bail;
	
bail:
	if (theTrack != NULL)
		*theTrack = myDstTrack;
		
	return(myErr);
}

