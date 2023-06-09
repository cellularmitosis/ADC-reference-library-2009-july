//////////
//
//	File:		QTWiredSpritesJr.c
//
//	Contains:	QuickTime wired sprites support for QuickTime movies.
//
//	Written by:	Tim Monroe
//
//	Copyright:	� 2001 by Apple Computer, Inc., all rights reserved.
//
//	Change History (most recent first):
//
//	   <1>	 	01/18/01	rtm		first file; based on existing QTSprites and QTWiredSprites sample code
//	   
//
//////////

//////////
//
// header files
//
//////////

#include "QTWiredSpritesJr.h"


//////////
//
// QTWired_CreateDraggableIconMovie
// Create a QuickTime movie containing a sprite track.
//
//////////

OSErr QTWired_CreateDraggableIconMovie (void)
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
	OSType					myType = FOUR_CHAR_CODE('none');
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
	
	// select the "no-interface" movie controller
	myType = EndianU32_NtoB(myType);
	SetUserDataItem(GetMovieUserData(myMovie), &myType, sizeof(myType), kUserDataMovieControllerType, 1);
	
	//////////
	//
	// create the sprite track and media
	//
	//////////
	
	myWidth = Long2Fix(kIconSpriteTrackWidth);
	myHeight = Long2Fix(kIconSpriteTrackHeight);

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
	
	myErr = QTWired_AddIconMovieSamplesToMedia(myMedia);
	if (myErr != noErr)
		goto bail;
	
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
	
	QTWired_SetTrackProperties(myMedia, 1);
	
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
// QTWired_AddIconMovieSamplesToMedia
// Build the key frame for the icon sprite movie.
//
//////////

OSErr QTWired_AddIconMovieSamplesToMedia (Media theMedia)
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
	SpriteUtils_AddPICTImageToKeyFrameSample(mySample, kNewQTIconID, &myKeyColor, 1, &myPoint, NULL);
	SpriteUtils_AddPICTImageToKeyFrameSample(mySample, kNewQTIconDownID, &myKeyColor, 2, &myPoint, NULL);

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
	myIndex			= kNewQTIconImageIndex;
	
	SpriteUtils_SetSpriteData(mySpriteData, &myLocation, &isVisible, NULL, &myIndex, NULL, NULL, NULL);
	SpriteUtils_AddSpriteToSample(mySample, mySpriteData, kQTIconSpriteAtomID);
	QTWired_MakeSpriteDraggable(mySample, kQTIconSpriteAtomID);
	QTWired_AddCursorChangeToSprite(mySample, kQTIconSpriteAtomID);
	SpriteUtils_AddSpriteSampleToMedia(theMedia, mySample, kSpriteMediaFrameDurationIcon, true, NULL);	

bail:	
	if (mySample != NULL)
		QTDisposeAtomContainer(mySample);

	if (mySpriteData != NULL)
		QTDisposeAtomContainer(mySpriteData);
	
	return(myErr);
}


//////////
//
// QTWired_AddControllerButtonSamplesToMedia
// Build the key frame for the controller sprite track.
//
//////////

void QTWired_AddControllerButtonSamplesToMedia (Media theMedia, long theTrackWidth, long theTrackHeight, TimeValue theDuration)
{
	QTAtomContainer			mySample = NULL;
	QTAtomContainer			myActions = NULL;
	QTAtomContainer			myPauseButton, myPlayButton, myPrevButton, myNextButton, myStartButton, myEndButton;
	RGBColor				myKeyColor;
	Point					myLocation;
	short					isVisible, myIndex, myLayer;
	short					myCount;
	long					mySixth;
#if !USE_WIRED_UTILITIES
	QTAtom					myEventAtom = 0;
	QTAtom					myActionAtom = 0;
	long					myAction;
#endif
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
	for (myCount = 0; myCount < kNumControllerImages; myCount++)
		SpriteUtils_AddPICTImageToKeyFrameSample(mySample, kFirstControllerImageID + myCount, &myKeyColor, myCount + 1, NULL, NULL);

	// assign group IDs to the images
	SpriteUtils_AssignImageGroupIDsToKeyFrame(mySample);
	
	//////////
	//
	// add the initial sprite properties and actions to the key frame sample
	//
	//////////
	
	mySixth = theTrackWidth / kNumControllerButtons;

	// the Pause button sprite
	myErr = QTNewAtomContainer(&myPauseButton);
	if (myErr != noErr)
		goto bail;

	myLocation.h 	= (kPauseSpritePosition * mySixth) + ((mySixth - kButtonWidth) / 2);
	myLocation.v	= theTrackHeight - (kButtonHeight + 5);
	isVisible		= true;
	myIndex			= kPauseUpIndex;
	myLayer			= 1;
	
	SpriteUtils_SetSpriteData(myPauseButton, &myLocation, &isVisible, &myLayer, &myIndex, NULL, NULL, NULL);
	WiredUtils_AddSpriteSetImageIndexAction(myPauseButton, kParentAtomIsContainer, kQTEventMouseClick, 0, NULL, 0, 0, NULL, kPauseDownIndex, NULL);
	WiredUtils_AddSpriteSetImageIndexAction(myPauseButton, kParentAtomIsContainer, kQTEventMouseClickEnd, 0, NULL, 0, 0, NULL, kPauseUpIndex, NULL);
	WiredUtils_AddMovieSetRateAction(myPauseButton, kParentAtomIsContainer, kQTEventMouseClickEndTriggerButton, 0);
	SpriteUtils_AddSpriteToSample(mySample, myPauseButton, kPauseSpriteAtomID);	

	// the Play button sprite
	myErr = QTNewAtomContainer(&myPlayButton);
	if (myErr != noErr)
		goto bail;

	myLocation.h 	= (kPlaySpritePosition * mySixth) + ((mySixth - kButtonWidth) / 2);
	myLocation.v	= theTrackHeight - (kButtonHeight + 5);
	isVisible		= true;
	myIndex			= kPlayUpIndex;
	myLayer			= 1;
	
	SpriteUtils_SetSpriteData(myPlayButton, &myLocation, &isVisible, &myLayer, &myIndex, NULL, NULL, NULL);
	WiredUtils_AddSpriteSetImageIndexAction(myPlayButton, kParentAtomIsContainer, kQTEventMouseClick, 0, NULL, 0, 0, NULL, kPlayDownIndex, NULL);
	WiredUtils_AddSpriteSetImageIndexAction(myPlayButton, kParentAtomIsContainer, kQTEventMouseClickEnd, 0, NULL, 0, 0, NULL, kPlayUpIndex, NULL);
	WiredUtils_AddMovieSetRateAction(myPlayButton, kParentAtomIsContainer, kQTEventMouseClickEndTriggerButton, fixed1);
	SpriteUtils_AddSpriteToSample(mySample, myPlayButton, kPlaySpriteAtomID);	

	// the Go-To-Start button sprite
	myErr = QTNewAtomContainer(&myStartButton);
	if (myErr != noErr)
		goto bail;

	myLocation.h 	= (kToBeginSpritePosition * mySixth) + ((mySixth - kButtonWidth) / 2);
	myLocation.v	= theTrackHeight - (kButtonHeight + 5);
	isVisible		= true;
	myIndex			= kToBeginUpIndex;
	myLayer			= 1;
	
	SpriteUtils_SetSpriteData(myStartButton, &myLocation, &isVisible, &myLayer, &myIndex, NULL, NULL, NULL);
	
#if USE_WIRED_UTILITIES
	// add actions to the Start button, using the wired sprite utilities
	WiredUtils_AddSpriteSetImageIndexAction(myStartButton, kParentAtomIsContainer, kQTEventMouseClick, 0, NULL, 0, 0, NULL, kToBeginDownIndex, NULL);
	WiredUtils_AddSpriteSetImageIndexAction(myStartButton, kParentAtomIsContainer, kQTEventMouseClickEnd, 0, NULL, 0, 0, NULL, kToBeginUpIndex, NULL);
	WiredUtils_AddMovieGoToBeginningAction(myStartButton, kParentAtomIsContainer, kQTEventMouseClickEndTriggerButton);
#else
	// add actions to the Start button, without using the wired sprite utilities;
	// significantly longer, eh?

	//////////
	//
	// add a mouse click event handler
	//
	//////////

	// add a kQTEventMouseClick event atom to the Start button atom
	myErr = QTInsertChild(myStartButton, kParentAtomIsContainer, kQTEventType, kQTEventMouseClick, 1, 0, NULL, &myEventAtom);
	if (myErr != noErr)
		goto bail;
		
	// add an action atom to the event handler
	myErr = QTInsertChild(myStartButton, myEventAtom, kAction, 0, 0, 0, NULL, &myActionAtom);
	if (myErr != noErr)
		goto bail;
	
	myAction = EndianU32_NtoB(kActionSpriteSetImageIndex);
	myErr = QTInsertChild(myStartButton, myActionAtom, kWhichAction, 1, 1, sizeof(myAction), &myAction, NULL);
	if (myErr != noErr)
		goto bail;

	// add a parameter to the set image index action: image index
	myIndex = EndianU32_NtoB(kToBeginDownIndex);
	myErr = QTInsertChild(myStartButton, myActionAtom, kActionParameter, 0, (short)kFirstParam, sizeof(myIndex), &myIndex, NULL);
	if (myErr != noErr)
		goto bail;

	//////////
	//
	// add a mouse click end event handler
	//
	//////////

	// add a kQTEventMouseClickEnd event atom to the Start button atom
	myErr = QTInsertChild(myStartButton, kParentAtomIsContainer, kQTEventType, kQTEventMouseClickEnd, 1, 0, NULL, &myEventAtom);
	if (myErr != noErr)
		goto bail;
		
	// add an action atom to the event handler
	myErr = QTInsertChild(myStartButton, myEventAtom, kAction, 0, 0, 0, NULL, &myActionAtom);
	if (myErr != noErr)
		goto bail;
	
	myAction = EndianU32_NtoB(kActionSpriteSetImageIndex);
	myErr = QTInsertChild(myStartButton, myActionAtom, kWhichAction, 1, 1, sizeof(myAction), &myAction, NULL);
	if (myErr != noErr)
		goto bail;

	// add a parameter to the set image index action: image index
	myIndex = EndianU32_NtoB(kToBeginUpIndex);
	myErr = QTInsertChild(myStartButton, myActionAtom, kActionParameter, 0, (short)kFirstParam, sizeof(myIndex), &myIndex, NULL);
	if (myErr != noErr)
		goto bail;

	//////////
	//
	// add a mouse button click event handler
	//
	//////////

	// add a kQTEventMouseClickEndTriggerButton event atom to the Start button atom
	myErr = QTInsertChild(myStartButton, kParentAtomIsContainer, kQTEventType, kQTEventMouseClickEndTriggerButton, 1, 0, NULL, &myEventAtom);
	if (myErr != noErr)
		goto bail;
		
	// add an action atom to the event handler
	myErr = QTInsertChild(myStartButton, myEventAtom, kAction, 0, 0, 0, NULL, &myActionAtom);
	if (myErr != noErr)
		goto bail;
	
	myAction = EndianU32_NtoB(kActionMovieGoToBeginning);
	myErr = QTInsertChild(myStartButton, myActionAtom, kWhichAction, 1, 1, sizeof(myAction), &myAction, NULL);
	if (myErr != noErr)
		goto bail;
#endif

	SpriteUtils_AddSpriteToSample(mySample, myStartButton, kToBeginSpriteAtomID);	

	// the Go-To-End button sprite
	myErr = QTNewAtomContainer(&myEndButton);
	if (myErr != noErr)
		goto bail;

	myLocation.h 	= (kToEndSpritePosition * mySixth) + ((mySixth - kButtonWidth) / 2);
	myLocation.v	= theTrackHeight - (kButtonHeight + 5);
	isVisible		= true;
	myIndex			= kToEndUpIndex;
	myLayer			= 1;
	
	SpriteUtils_SetSpriteData(myEndButton, &myLocation, &isVisible, &myLayer, &myIndex, NULL, NULL, NULL);
	WiredUtils_AddSpriteSetImageIndexAction(myEndButton, kParentAtomIsContainer, kQTEventMouseClick, 0, NULL, 0, 0, NULL, kToEndDownIndex, NULL);
	WiredUtils_AddSpriteSetImageIndexAction(myEndButton, kParentAtomIsContainer, kQTEventMouseClickEnd, 0, NULL, 0, 0, NULL, kToEndUpIndex, NULL);
	WiredUtils_AddMovieGoToEndAction(myEndButton, kParentAtomIsContainer, kQTEventMouseClickEndTriggerButton);
	SpriteUtils_AddSpriteToSample(mySample, myEndButton, kToEndSpriteAtomID);	

	// the Step Back button sprite
	myErr = QTNewAtomContainer(&myPrevButton);
	if (myErr != noErr)
		goto bail;

	myLocation.h 	= (kBackStepSpritePosition * mySixth) + ((mySixth - kButtonWidth) / 2);
	myLocation.v	= theTrackHeight - (kButtonHeight + 5);
	isVisible		= true;
	myIndex			= kBackStepUpIndex;
	myLayer			= 1;
	
	SpriteUtils_SetSpriteData(myPrevButton, &myLocation, &isVisible, &myLayer, &myIndex, NULL, NULL, NULL);
	WiredUtils_AddSpriteSetImageIndexAction(myPrevButton, kParentAtomIsContainer, kQTEventMouseClick, 0, NULL, 0, 0, NULL, kBackStepDownIndex, NULL);
	WiredUtils_AddSpriteSetImageIndexAction(myPrevButton, kParentAtomIsContainer, kQTEventMouseClickEnd, 0, NULL, 0, 0, NULL, kBackStepUpIndex, NULL);
	WiredUtils_AddMovieStepBackwardAction(myPrevButton, kParentAtomIsContainer, kQTEventMouseClickEndTriggerButton);
	SpriteUtils_AddSpriteToSample(mySample, myPrevButton, kBackStepSpriteAtomID);	

	// the Step Forward button sprite
	myErr = QTNewAtomContainer(&myNextButton);
	if (myErr != noErr)
		goto bail;

	myLocation.h 	= (kAheadStepSpritePosition * mySixth) + ((mySixth - kButtonWidth) / 2);
	myLocation.v	= theTrackHeight - (kButtonHeight + 5);
	isVisible		= true;
	myIndex			= kAheadStepUpIndex;
	myLayer			= 1;
	
	SpriteUtils_SetSpriteData(myNextButton, &myLocation, &isVisible, &myLayer, &myIndex, NULL, NULL, NULL);
	WiredUtils_AddSpriteSetImageIndexAction(myNextButton, kParentAtomIsContainer, kQTEventMouseClick, 0, NULL, 0, 0, NULL, kAheadStepDownIndex, NULL);
	WiredUtils_AddSpriteSetImageIndexAction(myNextButton, kParentAtomIsContainer, kQTEventMouseClickEnd, 0, NULL, 0, 0, NULL, kAheadStepUpIndex, NULL);
	WiredUtils_AddMovieStepForwardAction(myNextButton, kParentAtomIsContainer, kQTEventMouseClickEndTriggerButton);
	SpriteUtils_AddSpriteToSample(mySample, myNextButton, kAheadStepSpriteAtomID);	

	SpriteUtils_AddSpriteSampleToMedia(theMedia, mySample, theDuration, true, NULL);	

bail:	
	if (mySample != NULL)
		QTDisposeAtomContainer(mySample);
	if (myPauseButton != NULL)
		QTDisposeAtomContainer(myPauseButton);	
	if (myPlayButton != NULL)
		QTDisposeAtomContainer(myPlayButton);	
	if (myPrevButton != NULL)
		QTDisposeAtomContainer(myPrevButton);	
	if (myNextButton != NULL)
		QTDisposeAtomContainer(myNextButton);	
	if (myStartButton != NULL)
		QTDisposeAtomContainer(myStartButton);	
	if (myEndButton != NULL)
		QTDisposeAtomContainer(myEndButton);	
}


//////////
//
// QTWired_SetTrackProperties
// Set the track properties for the specified sample sprite movie.
//
//////////

void QTWired_SetTrackProperties (Media theMedia, UInt32 theIdleFrequency)
{
	QTAtomContainer		myTrackProperties;
	RGBColor			myBackgroundColor;
	Boolean				hasActions;
	UInt32				myFrequency;
	OSErr				myErr = noErr;
		
	// add a background color to the sprite track
	myBackgroundColor.red = EndianU16_NtoB(0xffff);
	myBackgroundColor.green = EndianU16_NtoB(0xffff);
	myBackgroundColor.blue = EndianU16_NtoB(0xffff);
	
	myErr = QTNewAtomContainer(&myTrackProperties);
	if (myErr == noErr) {
		QTInsertChild(myTrackProperties, 0, kSpriteTrackPropertyBackgroundColor, 1, 1, sizeof(myBackgroundColor), &myBackgroundColor, NULL);

		// tell the movie controller that this sprite track has actions
		hasActions = true;
		QTInsertChild(myTrackProperties, 0, kSpriteTrackPropertyHasActions, 1, 1, sizeof(hasActions), &hasActions, NULL);
	
		// tell the sprite track to generate QTIdleEvents
		myFrequency = EndianU32_NtoB(theIdleFrequency);
		QTInsertChild(myTrackProperties, 0, kSpriteTrackPropertyQTIdleEventsFrequency, 1, 1, sizeof(myFrequency), &myFrequency, NULL);

		SetMediaPropertyAtom(theMedia, myTrackProperties);
		
		QTDisposeAtomContainer(myTrackProperties);
	}
}
	

//////////
//
// QTWired_AddSpriteControllerTrack
// Add a track to the specified movie that contains wired sprites for controlling the movie.
//
// We add six sprites to the track, which will function like buttons. On mouse down, the image
// of the sprite is changed to the "clicked" image; on mouse up, the image is returned to the
// normal "unclicked" image; on mouse button events (that is, kQTEventMouseClickEndTriggerButton),
// we execute an action (for instance kActionMovieGoToBeginning). No other logic and no sprite track
// variables are required here.
//
//////////

OSErr QTWired_AddSpriteControllerTrack (Movie theMovie)
{
	Track					myTrack = NULL;
	Media					myMedia = NULL;
	MatrixRecord			myMatrix;
	RGBColor				myKeyColor;
	Fixed					myWidth, myHeight;
	Rect					myRect;
	TimeValue				myDuration = 0L;
	TimeValue				myTimeScale = 0L;
	OSType					myType = FOUR_CHAR_CODE('none');
	MatrixRecord			myInverseMatrix;
	OSErr					myErr = noErr;

	//////////
	//
	// get some information about the target movie
	//
	//////////

	if (theMovie == NULL) {
		myErr = paramErr;
		goto bail;
	}

	GetMovieBox(theMovie, &myRect);
	myWidth = Long2Fix(myRect.right - myRect.left);
	myHeight = Long2Fix(myRect.bottom - myRect.top);
	myDuration = GetMovieDuration(theMovie);
	myTimeScale = GetMovieTimeScale(theMovie);
	
	//////////
	//
	// create a new sprite track in the target movie
	//
	//////////
	
	myTrack = NewMovieTrack(theMovie, myWidth, myHeight, kNoVolume);
	myMedia = NewTrackMedia(myTrack, SpriteMediaType, myTimeScale, NULL, 0);

	// set the track matrix to compensate for any existing movie matrix
	GetMovieMatrix(theMovie, &myMatrix);
	if (InverseMatrix(&myMatrix, &myInverseMatrix))
		SetTrackMatrix(myTrack, &myInverseMatrix);

	myErr = BeginMediaEdits(myMedia);
	if (myErr != noErr)
		goto bail;
	
	//////////
	//
	// add sprite images and sprites to the sprite track; add actions to the sprites
	//
	//////////
	
	QTWired_AddControllerButtonSamplesToMedia(myMedia, myRect.right - myRect.left, myRect.bottom - myRect.top, myDuration);
	
	//////////
	//
	// insert media into track
	//
	//////////
	
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
	
	QTWired_SetTrackProperties(myMedia, kNoQTIdleEvents);
	
	myKeyColor.red = myKeyColor.green = myKeyColor.blue = 0xffff;		// white
	MediaSetGraphicsMode(GetMediaHandler(myMedia), transparent, &myKeyColor);
	
	// make sure that the sprite track is in the frontmost layer
	SetTrackLayer(myTrack, kMaxLayerNumber);
	SetTrackLayer(myTrack, QTWired_GetLowestLayerInMovie(theMovie) - 1);
	
	// select the "no-interface" movie controller
	myType = EndianU32_NtoB(myType);
	SetUserDataItem(GetMovieUserData(theMovie), &myType, sizeof(myType), kUserDataMovieControllerType, 1);
	
bail:
	return(myErr);
}


//////////
//
// QTWired_MakeSpriteDraggable
// Add atoms to the specified sprite to make it draggable.
//
// To make a sprite draggable, we need to install handlers for three kinds of events: mouse click, mouse click end,
// and idle. On mouse click (within the sprite), we set a track variable to 1; on mouse click end, we set that variable
// to 0. Thus, the value of that variable can be used to tell us if the mouse is down within the sprite. On idle, we
// look to see if the value of the variable is 1; if it is, we set the position of the sprite to the current mouse position.
//
//////////

OSErr QTWired_MakeSpriteDraggable (QTAtomContainer theContainer, QTAtomID theID)
{
	QTAtom								mySpriteAtom = 0;
	QTAtom								myEventAtom = 0;
	QTAtom								myActionAtom = 0;
	QTAtom								myParamAtom = 0;
	QTAtom								myConditionalAtom, myExpressionAtom, myOperatorAtom, myActionListAtom, myParameterAtom;
	short								myOperandIndex;
	QTAtomID							myVariableID;
	float								myConstantValue;
	Boolean								myIsAbsolute;
#if !USE_WIRED_UTILITIES
	QTAtom								myOperandAtom, myOperandTypeAtom;
	long								myAction;
	float								myVariableValue;
#endif
	OSErr								myErr = noErr;
	
	// find the sprite atom with the specified ID in the specified container
	mySpriteAtom = QTFindChildByID(theContainer, kParentAtomIsContainer, kSpriteAtomType, theID, NULL);
	if (mySpriteAtom == 0) {
		myErr = paramErr;
		goto bail;
	}
	
	//////////
	//
	// add a mouse click event handler
	//
	//////////
	
#if USE_WIRED_UTILITIES
	myErr = WiredUtils_AddSpriteTrackSetVariableAction(theContainer, mySpriteAtom, kQTEventMouseClick, kMouseStateVariableID, 1, 0, NULL, 0);
	if (myErr != noErr)
		goto bail;
#else
	// find the event atom of type kQTEventMouseClick in the sprite atom
	myEventAtom = QTFindChildByID(theContainer, mySpriteAtom, kQTEventType, kQTEventMouseClick, NULL);
	if (myEventAtom == 0) {
		// if there is none, insert a new event atom of type kQTEventMouseClick into the sprite atom
		myErr = QTInsertChild(theContainer, mySpriteAtom, kQTEventType, kQTEventMouseClick, 1, 0, NULL, &myEventAtom);
		if (myErr != noErr)
			goto bail;
	}
	
	// add an action atom to the mouse click event handler
	myErr = QTInsertChild(theContainer, myEventAtom, kAction, 0, 0, 0, NULL, &myActionAtom);
	if (myErr != noErr)
		goto bail;
	
	myAction = EndianU32_NtoB(kActionSpriteTrackSetVariable);
	myErr = QTInsertChild(theContainer, myActionAtom, kWhichAction, 1, 1, sizeof(myAction), &myAction, NULL);
	if (myErr != noErr)
		goto bail;
			
	// add parameters to the set variable action: variable ID (QTAtomID) and value (float)
	myVariableID = EndianU32_NtoB(kMouseStateVariableID);
	myErr = QTInsertChild(theContainer, myActionAtom, kActionParameter, 0, (short)kFirstParam, sizeof(myVariableID), &myVariableID, NULL);
	if (myErr != noErr)
		goto bail;
	
	myVariableValue = (float)1;
	EndianUtils_Float_NtoB(&myVariableValue);
	myErr = QTInsertChild(theContainer, myActionAtom, kActionParameter, 0, (short)kSecondParam, sizeof(myVariableValue), &myVariableValue, NULL);
	if (myErr != noErr)
		goto bail;
#endif
	
	//////////
	//
	// add a mouse click end event handler
	//
	//////////
	
#if USE_WIRED_UTILITIES
	myErr = WiredUtils_AddSpriteTrackSetVariableAction(theContainer, mySpriteAtom, kQTEventMouseClickEnd, kMouseStateVariableID, 0, 0, NULL, 0);
	if (myErr != noErr)
		goto bail;
#else
	// find the event atom of type kQTEventMouseClick in the sprite atom
	myEventAtom = QTFindChildByID(theContainer, mySpriteAtom, kQTEventType, kQTEventMouseClickEnd, NULL);
	if (myEventAtom == 0) {
		// if there is none, insert a new event atom of type kQTEventMouseClick into the sprite atom
		myErr = QTInsertChild(theContainer, mySpriteAtom, kQTEventType, kQTEventMouseClickEnd, 1, 0, NULL, &myEventAtom);
		if (myErr != noErr)
			goto bail;
	}
	
	// add an action atom to the mouse click event handler
	myErr = QTInsertChild(theContainer, myEventAtom, kAction, 0, 0, 0, NULL, &myActionAtom);
	if (myErr != noErr)
		goto bail;
	
	myAction = EndianU32_NtoB(kActionSpriteTrackSetVariable);
	myErr = QTInsertChild(theContainer, myActionAtom, kWhichAction, 1, 1, sizeof(myAction), &myAction, NULL);
	if (myErr != noErr)
		goto bail;
	
	// add parameters to the set variable action: variable ID (QTAtomID) and value (float)
	myVariableID = EndianU32_NtoB(kMouseStateVariableID);
	myErr = QTInsertChild(theContainer, myActionAtom, kActionParameter, 0, (short)kFirstParam, sizeof(myVariableID), &myVariableID, NULL);
	if (myErr != noErr)
		goto bail;
	
	myVariableValue = (float)0;
	EndianUtils_Float_NtoB(&myVariableValue);
	myErr = QTInsertChild(theContainer, myActionAtom, kActionParameter, 0, (short)kSecondParam, sizeof(myVariableValue), &myVariableValue, NULL);
	if (myErr != noErr)
		goto bail;
#endif
	
	//////////
	//
	// add an idle event handler
	//
	//////////
	
#if USE_WIRED_UTILITIES
	myErr = WiredUtils_AddQTEventAndActionAtoms(theContainer, mySpriteAtom, kQTEventIdle, kActionCase, &myActionAtom);
	if (myErr != noErr)
		goto bail;

	// add a parameter atom to the kActionCase action atom; this will serve as a parent to hold the expression and action atoms
	myErr = WiredUtils_AddActionParameterAtom(theContainer, myActionAtom, kFirstParam, 0, NULL, &myParamAtom);
	if (myErr != noErr)
		goto bail;
	
	myErr = WiredUtils_AddConditionalAtom(theContainer, myParamAtom, 1, &myConditionalAtom);
	if (myErr != noErr)
		goto bail;
		
	myErr = WiredUtils_AddExpressionContainerAtomType(theContainer, myConditionalAtom, &myExpressionAtom);
	if (myErr != noErr)
		goto bail;
		
	myErr = WiredUtils_AddOperatorAtom(theContainer, myExpressionAtom, kOperatorEqualTo, &myOperatorAtom);
	if (myErr != noErr)
		goto bail;

	myOperandIndex = 1;	
	myConstantValue = 1;
	myErr = WiredUtils_AddOperandAtom(theContainer, myOperatorAtom, kOperandConstant, myOperandIndex, NULL, myConstantValue);
	if (myErr != noErr)
		goto bail;

	myOperandIndex = 2;
	myVariableID = kMouseStateVariableID;
	myErr = WiredUtils_AddVariableOperandAtom(theContainer, myOperatorAtom, myOperandIndex, 0, NULL, 0, myVariableID);
	if (myErr != noErr)
		goto bail;
		
	myErr = WiredUtils_AddActionListAtom(theContainer, myConditionalAtom, &myActionListAtom);
	if (myErr != noErr)
		goto bail;

	myErr = WiredUtils_AddActionAtom(theContainer, myActionListAtom, kActionSpriteTranslate, &myActionAtom);
	if (myErr != noErr)
		goto bail;
		
	// add parameters to the translate action: Fixed x, Fixed y, Boolean isAbsolute

	// first parameter: get current mouse position x
	myErr = WiredUtils_AddActionParameterAtom(theContainer, myActionAtom, kFirstParam, 0, NULL, &myParameterAtom);
	if (myErr != noErr)
		goto bail;

	myErr = WiredUtils_AddExpressionContainerAtomType(theContainer, myParameterAtom, &myExpressionAtom);
	if (myErr != noErr)
		goto bail;

	myErr = WiredUtils_AddOperandAtom(theContainer, myExpressionAtom, kOperandMouseLocalHLoc, 1, NULL, 0);
	if (myErr != noErr)
		goto bail;
		
	// second parameter: get current mouse position y
	myErr = WiredUtils_AddActionParameterAtom(theContainer, myActionAtom, kSecondParam, 0, NULL, &myParameterAtom);
	if (myErr != noErr)
		goto bail;

	myErr = WiredUtils_AddExpressionContainerAtomType(theContainer, myParameterAtom, &myExpressionAtom);
	if (myErr != noErr)
		goto bail;

	myErr = WiredUtils_AddOperandAtom(theContainer, myExpressionAtom, kOperandMouseLocalVLoc, 1, NULL, 0);
	if (myErr != noErr)
		goto bail;
	
	// third parameter: true
	myIsAbsolute = true;
	myErr = WiredUtils_AddActionParameterAtom(theContainer, myActionAtom, kThirdParam, sizeof(myIsAbsolute), &myIsAbsolute, NULL);
		
#else
	// find the event atom of type kQTEventIdle in the sprite atom
	myEventAtom = QTFindChildByID(theContainer, mySpriteAtom, kQTEventType, kQTEventIdle, NULL);
	if (myEventAtom == 0) {
		// if there is none, insert a new event atom of type kQTEventIdle into the sprite atom
		myErr = QTInsertChild(theContainer, mySpriteAtom, kQTEventType, kQTEventIdle, 1, 0, NULL, &myEventAtom);
		if (myErr != noErr)
			goto bail;
	}
	
	// add an action atom to the mouse click event handler
	myErr = QTInsertChild(theContainer, myEventAtom, kAction, 0, 0, 0, NULL, &myActionAtom);
	if (myErr != noErr)
		goto bail;
	
	myAction = EndianU32_NtoB(kActionCase);
	myErr = QTInsertChild(theContainer, myActionAtom, kWhichAction, 1, 1, sizeof(myAction), &myAction, NULL);
	if (myErr != noErr)
		goto bail;

	// add a parameter atom to the kActionCase action atom; this will serve as a parent to hold conditional atom
	myErr = QTInsertChild(theContainer, myActionAtom, kActionParameter, 1, kFirstParam, 0, NULL, &myParamAtom);
	if (myErr != noErr)
		goto bail;
	
	// the condition atom is the parent of the expression and action list atoms
	myErr = QTInsertChild(theContainer, myParamAtom, kConditionalAtomType, 0, 1, 0, NULL, &myConditionalAtom);
	if (myErr != noErr)
		goto bail;

	myErr = QTInsertChild(theContainer, myConditionalAtom, kExpressionContainerAtomType, 1, 1, 0, NULL, &myExpressionAtom);
	if (myErr != noErr)
		goto bail;

	myErr = QTInsertChild(theContainer, myExpressionAtom, kOperatorAtomType, kOperatorEqualTo, 1, 0, NULL, &myOperatorAtom);
	if (myErr != noErr)
		goto bail;
		
	// add the operands to the operator atom
	myOperandIndex = 1;	
	myConstantValue = 1;
	myErr = QTInsertChild(theContainer, myOperatorAtom, kOperandAtomType, 0, myOperandIndex, 0, NULL, &myOperandAtom);
	if (myErr != noErr)
		goto bail;
	
	myErr = QTInsertChild(theContainer, myOperandAtom, kOperandConstant, 1, 1, 0, NULL, &myOperandTypeAtom);
	if (myErr != noErr)
		goto bail;

	EndianUtils_Float_NtoB(&myConstantValue);
	myErr = QTSetAtomData(theContainer, myOperandTypeAtom, sizeof(myConstantValue), &myConstantValue);
	
	myOperandIndex = 2;
	myVariableID = kMouseStateVariableID;
	myErr = QTInsertChild(theContainer, myOperatorAtom, kOperandAtomType, 0, myOperandIndex, 0, NULL, &myOperandAtom);
	if (myErr != noErr)
		goto bail;
	
	myErr = QTInsertChild(theContainer, myOperandAtom, kOperandSpriteTrackVariable, 1, 1, 0, NULL, &myOperandTypeAtom);
	if (myErr != noErr)
		goto bail;
	
	myVariableID = EndianU32_NtoB(myVariableID);
	myErr = QTInsertChild(theContainer, myOperandTypeAtom, kActionParameter, 1, 1, sizeof(myVariableID), &myVariableID, NULL);
	if (myErr != noErr)
		goto bail;

	// add an action list atom
	myErr = QTInsertChild(theContainer, myConditionalAtom, kActionListAtomType, 1, 1, 0, NULL, &myActionListAtom);
	if (myErr != noErr)
		goto bail;

	// add sprite translate action
	myErr = QTInsertChild(theContainer, myActionListAtom, kAction, 0, 0, 0, NULL, &myActionAtom);
	if (myErr != noErr)
		goto bail;
	
	myAction = EndianU32_NtoB(kActionSpriteTranslate);
	myErr = QTInsertChild(theContainer, myActionAtom, kWhichAction, 1, 1, sizeof(myAction), &myAction, NULL);
	if (myErr != noErr)
		goto bail;

	// add parameters to the translate action: Fixed x, Fixed y, Boolean isAbsolute
	
	// first parameter: get current mouse position x
	myErr = QTInsertChild(theContainer, myActionAtom, kActionParameter, 0, (short)kFirstParam, 0, NULL, &myParameterAtom);
	if (myErr != noErr)
		goto bail;
		
	myErr = QTInsertChild(theContainer, myParameterAtom, kExpressionContainerAtomType, 1, 1, 0, NULL, &myExpressionAtom);
	if (myErr != noErr)
		goto bail;

	myErr = QTInsertChild(theContainer, myExpressionAtom, kOperandAtomType, 0, 1, 0, NULL, &myOperandAtom);
	if (myErr != noErr)
		goto bail;
		
	myErr = QTInsertChild(theContainer, myOperandAtom, kOperandMouseLocalHLoc, 1, 1, 0, NULL, NULL);
	if (myErr != noErr)
		goto bail;

	// second parameter: get current mouse position y
	myErr = QTInsertChild(theContainer, myActionAtom, kActionParameter, 0, (short)kSecondParam, 0, NULL, &myParameterAtom);
	if (myErr != noErr)
		goto bail;
		
	myErr = QTInsertChild(theContainer, myParameterAtom, kExpressionContainerAtomType, 1, 1, 0, NULL, &myExpressionAtom);
	if (myErr != noErr)
		goto bail;

	myErr = QTInsertChild(theContainer, myExpressionAtom, kOperandAtomType, 0, 1, 0, NULL, &myOperandAtom);
	if (myErr != noErr)
		goto bail;
		
	myErr = QTInsertChild(theContainer, myOperandAtom, kOperandMouseLocalVLoc, 1, 1, 0, NULL, NULL);
	if (myErr != noErr)
		goto bail;

	// third parameter: true
	myIsAbsolute = true;
	myErr = QTInsertChild(theContainer, myActionAtom, kActionParameter, 0, (short)kThirdParam, sizeof(myIsAbsolute), &myIsAbsolute, NULL);
#endif
	
bail:
	return(myErr);
}


//////////
//
// QTWired_AddCursorChangeToSprite
// Add cursor-change-on-mouse-over and -down actions to the sprite having the specified ID
// in the specified atom container.
//
// Here we use the new "sprite behaviors atom" introduced in QuickTime 4.0, which
// simplifies adding button-like capabilities to sprites.
//
//////////

OSErr QTWired_AddCursorChangeToSprite (QTAtomContainer theContainer, QTAtomID theID)
{
	QTAtom								mySpriteAtom = 0;
	QTAtom								myBehaviorAtom = 0;
	QTSpriteButtonBehaviorStruct		myBehaviorRec;
	OSErr								myErr = paramErr;
	
	// find the sprite atom with the specified ID in the specified container
	mySpriteAtom = QTFindChildByID(theContainer, kParentAtomIsContainer, kSpriteAtomType, theID, NULL);
	if (mySpriteAtom == 0)
		goto bail;
	
	// insert a new sprite behaviors atom into the sprite atom
	myErr = QTInsertChild(theContainer, mySpriteAtom, kSpriteBehaviorsAtomType, 1, 1, 0, NULL, &myBehaviorAtom);
	if (myErr != noErr)
		goto bail;
	
	// set the sprite cursor behavior; -1 means: no change associated with this state transition
	myBehaviorRec.notOverNotPressedStateID = EndianS32_NtoB(-1);
	myBehaviorRec.overNotPressedStateID = EndianS32_NtoB(kQTCursorOpenHand);
	myBehaviorRec.overPressedStateID = EndianS32_NtoB(kQTCursorClosedHand);
	myBehaviorRec.notOverPressedStateID = EndianS32_NtoB(-1);

	myErr = QTInsertChild(theContainer, myBehaviorAtom, kSpriteCursorBehaviorAtomType, 1, 1, sizeof(QTSpriteButtonBehaviorStruct), &myBehaviorRec, NULL);

bail:
	return(myErr);
}


//////////
//
// QTWired_GetLowestLayerInMovie
// Return the layer of the frontmost layer in the specified movie.
//
//////////

short QTWired_GetLowestLayerInMovie (Movie theMovie)
{
	long		myCount = 0;
	long		myIndex;
	short		myLayer = 0;
	short		myMinLayer = kMaxLayerNumber;
	
	myCount = GetMovieTrackCount(theMovie);
	
	for (myIndex = 1; myIndex <= myCount; myIndex++) {
		myLayer = GetTrackLayer(GetMovieIndTrack(theMovie, myIndex));
		if (myLayer < myMinLayer)
			myMinLayer = myLayer;
	}
	
	return(myMinLayer);
}

