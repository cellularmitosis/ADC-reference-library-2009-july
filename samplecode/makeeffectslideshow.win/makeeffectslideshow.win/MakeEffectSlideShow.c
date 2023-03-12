//////////
//
//	File:		MakeEffectSlideShow.c
//
//	Contains:	QuickTime video effect support for QuickTime movies and still images.
//
//	Written by:	Tim Monroe
//				Based (heavily!) on the MakeEffectSlideShow code written by Sam Bushell.
//
//	Copyright:	© 1997-1999 by Apple Computer, Inc., all rights reserved.
//
//	Change History (most recent first):
//
//	   <8>	 	03/20/00	rtm		made changes to get things running under CarbonLib
//	   <7>	 	03/03/99	rtm		added support for MakeImageDescriptionForEffect (QT 4.0 and later)
//	   <6>	 	03/12/98	rtm		added global flag gDoneWithDialog to fix Windows dialog box problems
//	   <5>	 	02/28/98	rtm		revised event/message handling following QTShowEffect.c
//	   <4>	 	08/01/98	rtm		sync'ed with latest code from Sam: reworked QTEffects_GetFirstVideoTrackInMovie
//									to use GetMovieIndTrackType
//	   <3>	 	11/24/97	rtm		reworked event-loop processing: changed QTEffects_DoMakeEffectSlideShowMovieForFiles into
//									QTEffects_DisplayDialogForSources and QTEffects_RespondToDialogSelection
//	   <2>	 	11/21/97	rtm		further work
//	   <1>	 	11/06/97	rtm		first file; integrated existing code with shell framework;
//									added endian macros where appropriate
//	   
//	This application takes the video tracks from two or more movies, asks 
//	the user to select a 2-source effect, and makes a slide show movie 
//	which uses the effect to switch from one video track to the next. 
//	
//	Short video tracks are scaled up to a minimum length, and we can import
//	all sorts of graphical image formats as movies, so you can use this to 
//	generate a slide show movie from still images -- or even a collection
//	of stills and movies.
//	
//	Try it out: drag a bunch of JPEG files of the same size onto the application icon.
//	Alternatively, launch the application and then choose them from the Test... menu.
//
//////////

// header files
#include "MakeEffectSlideShow.h"

// global variables
QTParameterDialog			gEffectsDialog = 0L;
QTAtomContainer				gEffectSample = 0;				// effects sample
QTAtomContainer				gEffectList = 0;
PicHandle					gPosterA = NULL;
PicHandle					gPosterB = NULL;
Movie						gSrcMovies[kMaxNumSources];
Track						gSrcTracks[kMaxNumSources];
UInt16						gSpecCount = 0;		
FSSpec						gSpecList[kMaxNumSources];
Boolean						gDoneWithDialog = false;		// are we done using the effects parameters dialog box?


//////////
//
// QTEffects_GetFirstVideoTrackInMovie
// Return, through the theTrack parameter, the first video track in the specified movie.
//
// Actually, we look for the first track that has the kCharacteristicCanSendVideo characteristic,
// so we can apply effects to MPEG or QD3D tracks as well.
//
// If no such track is found, return invalidTrack as the function result.
//
//////////

OSErr QTEffects_GetFirstVideoTrackInMovie (Movie theMovie, Track *theTrack)
{
	*theTrack = GetMovieIndTrackType(theMovie, 1, kCharacteristicCanSendVideo, movieTrackCharacteristic | movieTrackEnabledOnly);
	
	if (*theTrack == NULL)
		return(invalidTrack);
		
	return(noErr);
}


//////////
//
// QTEffects_GetPosterPictFromFirstVideoTrackInMovieFile
// Get the poster picture for the first video track in the specified movie file.
//
//////////

PicHandle QTEffects_GetPosterPictFromFirstVideoTrackInMovieFile (FSSpec *theSpec)
{
	PicHandle	myPict = NULL;
	Movie		myMovie = NULL;
	Track		myTrack = NULL;
	short		myRefNum = 0;
	OSErr		myErr = noErr;
	
	myErr = OpenMovieFile(theSpec, &myRefNum, fsRdPerm);
	BailError(myErr);

	myErr = NewMovieFromFile(&myMovie, myRefNum, NULL, NULL, 0, NULL);
	BailError(myErr);
	
	SetMoviePlayHints(myMovie, hintsHighQuality, hintsHighQuality);

	myErr = CloseMovieFile(myRefNum);
	BailError(myErr);
	
	myErr = QTEffects_GetFirstVideoTrackInMovie(myMovie, &myTrack);
	BailNil(myTrack);
	
	myPict = GetTrackPict(myTrack, GetMoviePosterTime(myMovie));

bail:
	if (myMovie != NULL)
		DisposeMovie(myMovie);
	
	return(myPict);
}


//////////
//
// QTEffects_CopyPortionOfTrackToTrack
// Extract a portion of the source track and copy it to the destination track.
//
// The parameter theSourcePortions is formed by OR-ing any of the portion constants.
// For example, to copy the start and middle portions, set theSourcePortions to (eStartPortion | eMiddlePortion).
//
// We assume that the track time scale is kTimeScale; we also assume that the track has been scaled
// so that it is at least kMinimumDuration long.
//
//////////

OSErr QTEffects_CopyPortionOfTrackToTrack (Track theSourceTrack, UInt16 theSourcePortions, Track theDestTrack, TimeValue theDestStartTime, TimeValue *theDestDuration)
{
	TimeValue	mySourceTrackDuration;
	TimeValue	mySourceSegmentStart, mySourceSegmentEnd, mySourceSegmentDuration;
	OSErr		myErr = noErr;
	
	mySourceTrackDuration = GetTrackDuration(theSourceTrack);
	
	if (theSourcePortions & eStartPortion)
		mySourceSegmentStart = 0;
	else if (theSourcePortions & eMiddlePortion)
		mySourceSegmentStart = kEffectDuration;
	else
		mySourceSegmentStart = mySourceTrackDuration - kEffectDuration;
	
	if (theSourcePortions & eFinishPortion)
		mySourceSegmentEnd = mySourceTrackDuration;
	else if (theSourcePortions & eMiddlePortion)
		mySourceSegmentEnd = mySourceTrackDuration - kEffectDuration;
	else
		mySourceSegmentEnd = kEffectDuration;
	
	mySourceSegmentDuration = mySourceSegmentEnd - mySourceSegmentStart;
	
	myErr = InsertTrackSegment( theSourceTrack,
								theDestTrack,
								mySourceSegmentStart,
								mySourceSegmentDuration,
								theDestStartTime);
	
	*theDestDuration = mySourceSegmentDuration;
	
	return(myErr);
}


//////////
//
// QTEffects_DisplayDialogForSources
// Display the standard effects parameters dialog box for the movies passed in.
//
//////////

OSErr QTEffects_DisplayDialogForSources (FSSpec *theSpecList, UInt16 theSpecCount)
{
	OSErr					myErr = noErr;
	
	// make sure that there are enough sources: you can't make an omelette without enough eggs
	if (theSpecCount < 2) {
		myErr = paramErr;
		goto bail;
	}

	// assign source count to a global, so QTEffects_RespondToDialogSelection has access to it
	gSpecCount = theSpecCount;		
	
	// get a poster frame for the first two movies
	if (theSpecCount >= 1)
		gPosterA = QTEffects_GetPosterPictFromFirstVideoTrackInMovieFile(&theSpecList[0]);
	
	if (theSpecCount >= 2)
		gPosterB = QTEffects_GetPosterPictFromFirstVideoTrackInMovieFile(&theSpecList[1]);
	
	// ask the user to select a two-source effect

	myErr = QTNewAtomContainer(&gEffectSample);
	BailError(myErr);
	
	myErr = QTGetEffectsList(&gEffectList, 2, 2, 0);		// min == max == 2
	BailError(myErr);
	
	myErr = QTCreateStandardParameterDialog(gEffectList, gEffectSample, 0, &gEffectsDialog);
	BailError(myErr);
	
	// insert poster frames into dialog
	if (gPosterA != NULL) {
		QTParamPreviewRecord			pr;

		pr.sourcePicture = gPosterA;
		pr.sourceID = 1;
		QTStandardParameterDialogDoAction(gEffectsDialog, pdActionSetPreviewPicture, &pr);
	}

	if (gPosterB != NULL) {
		QTParamPreviewRecord			pr;

		pr.sourcePicture = gPosterB;
		pr.sourceID = 2;
		QTStandardParameterDialogDoAction(gEffectsDialog, pdActionSetPreviewPicture, &pr);
	}
	
	// now, the frontmost window is the standard effects parameter dialog box;
	// on the Mac, we call QTEffects_HandleEffectsDialogEvents in our main event loop
	// to find and process events targeted at the effects parameter dialog box; on Windows,
	// we need to use a different strategy: we install a modeless dialog callback procedure
	// that is called internally by QTML

#if TARGET_OS_WIN32
	gDoneWithDialog = false;
	
	// force the dialog box to be drawn
	{
		EventRecord			myEvent = {0};
		
		QTEffects_EffectsDialogCallback(&myEvent, FrontWindow(), 0);
	}
	
	SetModelessDialogCallbackProc(FrontWindow(), (QTModelessCallbackUPP)QTEffects_EffectsDialogCallback);
	QTMLSetWindowWndProc(FrontWindow(), QTEffects_CustomDialogWndProc);
#endif
	
bail:
	return(myErr);
}


//////////
//
// QTEffects_RespondToDialogSelection
// If theErr is codecParameterDialogConfirm, make an effects movie.
// If theErr is userCanceledErr, do any necessary clean up.
//
// Construct a movie that links a sequence of image or movie files together with real-time effects.
// That is, show the first image or movie, transition (fade or whatever) to the second, show the second,
// transition to the third, and so forth.
//
// We use NewMovieFromFile, which will use movie importers or graphics importers as appropriate,
// so each file we open may end up as either.
//
// If a movie has only one sample in its first video track, we assume it's a still image,
// and use several copies of that sample with different lengths. This is flawed, since the sample
// could be a tweened 3DMF movie.
//
//////////

void QTEffects_RespondToDialogSelection (OSErr theErr)
{
	Boolean					myDialogWasCancelled = false;
	short					myResID = movieInDataForkResID;
	UInt16					myMovieIter;
	short					mySrcMovieRefNum = 0;
	Movie					myPrevSrcMovie = NULL;
	Track					myPrevSrcTrack = NULL;
	Movie					myNextSrcMovie = NULL;
	Track					myNextSrcTrack = NULL;
	short					myDestMovieRefNum = 0;
	FSSpec					myFile;
	Boolean					myIsSelected = false;
	Boolean					myIsReplacing = false;	
	StringPtr 				myPrompt = QTUtils_ConvertCToPascalString(kEffectsSaveMoviePrompt);
	StringPtr 				myFileName = QTUtils_ConvertCToPascalString(kEffectsSaveMovieFileName);
	Movie					myDestMovie = NULL;
	Fixed					myDestMovieWidth, myDestMovieHeight;
	ImageDescriptionHandle	myDesc = NULL;
	Track					videoTrackFX, videoTrackA, videoTrackB;
	Media					videoMediaFX, videoMediaA, videoMediaB;
	TimeValue				myCurrentDuration = 0;
	TimeValue				myReturnedDuration;
	Boolean					isFirstTransition = true;
	TimeValue				myMediaTransitionDuration;
	TimeValue				myMediaFXStartTime, myMediaFXDuration;
	OSType					myEffectCode;
	long					myFlags = createMovieFileDeleteCurFile | createMovieFileDontCreateResFile;
	long					myLong;
	OSErr					myErr = noErr;

	// standard parameter box has been dismissed, so remember that fact
	gEffectsDialog = 0L;
	
	myDialogWasCancelled = (theErr == userCanceledErr);
	
	// we're finished with the effect list and movie posters	
	QTDisposeAtomContainer(gEffectList);
	
	if (gPosterA != NULL)
		KillPicture(gPosterA);
		
	if (gPosterB != NULL)
		KillPicture(gPosterB);
	
	// when the sign says stop, then stop
	if (myDialogWasCancelled)
		goto bail;

	// add atoms naming the sources to gEffectSample
	myLong = EndianU32_NtoB(kSourceOneName);
	QTInsertChild(gEffectSample, kParentAtomIsContainer, kEffectSourceName, 1, 0, sizeof(myLong), &myLong, NULL);

	myLong = EndianU32_NtoB(kSourceTwoName);
	QTInsertChild(gEffectSample, kParentAtomIsContainer, kEffectSourceName, 2, 0, sizeof(myLong), &myLong, NULL);
	
	// extract the 'what' atom to find out what kind of effect it is
	{
		QTAtom			myEffectAtom;
		QTAtomID		myEffectAtomID;
		long			myEffectCodeSize;
		Ptr				myEffectCodePtr;

		myEffectAtom = QTFindChildByIndex(gEffectSample, kParentAtomIsContainer, kParameterWhatName, kParameterWhatID, &myEffectAtomID);
		
		myErr = QTLockContainer(gEffectSample);
		BailError(myErr);

		myErr = QTGetAtomDataPtr(gEffectSample, myEffectAtom, &myEffectCodeSize, &myEffectCodePtr);
		BailError(myErr);

		if (myEffectCodeSize != sizeof(OSType)) {
			myErr = paramErr;
			goto bail;
		}
		
		myEffectCode = *(OSType *)myEffectCodePtr;		// "tsk"
		myEffectCode = EndianU32_BtoN(myEffectCode);	// because the data is read from an atom container
		
		myErr = QTUnlockContainer(gEffectSample);
		BailError(myErr);
	}

	// ask the user for the name of the new movie file
	QTFrame_PutFile(myPrompt, myFileName, &myFile, &myIsSelected, &myIsReplacing);
	if (!myIsSelected)
		goto bail;				// deal with user cancelling

	// create a movie file for the destination movie
	myErr = CreateMovieFile(&myFile, FOUR_CHAR_CODE('TVOD'), 0, myFlags, &myDestMovieRefNum, &myDestMovie);
	BailError(myErr);
	
	// open the first file as a movie; call the first movie myPrevSrcMovie
	myErr = OpenMovieFile(&gSpecList[0], &mySrcMovieRefNum, fsRdPerm);
	BailError(myErr);
	
	myErr = NewMovieFromFile(&myPrevSrcMovie, mySrcMovieRefNum, NULL, NULL, 0, NULL);
	BailError(myErr);
	
	myErr = CloseMovieFile(mySrcMovieRefNum);
	BailError(myErr);
	
	// if the movie is shorter than kMinimumDuration, scale it to that length
	SetMovieTimeScale(myPrevSrcMovie, kTimeScale);
	myErr = QTEffects_GetFirstVideoTrackInMovie(myPrevSrcMovie, &myPrevSrcTrack);
	BailNil(myPrevSrcTrack);
	
	if (GetTrackDuration(myPrevSrcTrack) < kMinimumDuration) {
		myErr = ScaleTrackSegment(myPrevSrcTrack, 0, GetTrackDuration(myPrevSrcTrack), kMinimumDuration);
		BailError(myErr);
	}
	
	// find out how big the first movie is; we'll use it as the size of all our tracks
	GetTrackDimensions(myPrevSrcTrack, &myDestMovieWidth, &myDestMovieHeight);
	
#if USES_MAKE_IMAGE_DESC_FOR_EFFECT
	// create a new sample description for the effect,
	// which is just an image description specifying the effect and its dimensions
	myErr = MakeImageDescriptionForEffect(myEffectCode, &myDesc);
	if (myErr != noErr)
		BailError(myErr);
#else
	// create a new sample description for the effect,
	// which is just an image description specifying the effect and its dimensions
	myDesc = (ImageDescriptionHandle)NewHandleClear(sizeof(ImageDescription));
	BailNil(myDesc);
	
	(**myDesc).idSize = sizeof(ImageDescription);
	(**myDesc).cType = myEffectCode;
	(**myDesc).hRes = 72L << 16;
	(**myDesc).vRes = 72L << 16;
	(**myDesc).dataSize = 0L;
	(**myDesc).frameCount = 1;
	(**myDesc).depth = 0;
	(**myDesc).clutID = -1;
#endif
	
	// fill in the fields of the sample description
	(**myDesc).vendor = kAppleManufacturer;
	(**myDesc).temporalQuality = codecNormalQuality;
	(**myDesc).spatialQuality = codecNormalQuality;
	(**myDesc).width = FixRound(myDestMovieWidth);
	(**myDesc).height = FixRound(myDestMovieHeight);

	// add three video tracks to the destination movie:
	// 	- videoTrackFX is where the effects and stills live; it's user-visible.
	//	- videoTrackA is where the "source A"s for effects live; it's hidden by the input map
	//	- videoTrackB is where the "source B"s for effects live; it's hidden by the input map
	videoTrackFX = NewMovieTrack(myDestMovie, myDestMovieWidth, myDestMovieHeight, 0);
	BailNil(videoTrackFX);
	videoMediaFX = NewTrackMedia(videoTrackFX, VideoMediaType, kTimeScale, NULL, 0);
	BailNil(videoMediaFX);
	myErr = BeginMediaEdits(videoMediaFX);
	BailError(myErr);
	
	videoTrackA = NewMovieTrack(myDestMovie, myDestMovieWidth, myDestMovieHeight, 0);
	BailNil(videoTrackA);
	videoMediaA = NewTrackMedia(videoTrackA, VideoMediaType, kTimeScale, NULL, 0);
	BailNil(videoMediaA);

	videoTrackB = NewMovieTrack(myDestMovie, myDestMovieWidth, myDestMovieHeight, 0);
	BailNil(videoTrackB);
	videoMediaB = NewTrackMedia(videoTrackB, VideoMediaType, kTimeScale, NULL, 0);
	BailNil(videoMediaB);

	// create the input map
	{
		long				myRefIndex1, myRefIndex2;
		QTAtomContainer		myInputMap;
		QTAtom				myInputAtom;
		OSType				myInputType;

		QTNewAtomContainer(&myInputMap);

		// first input
		if (videoTrackA) {
		
			AddTrackReference(videoTrackFX, videoTrackA, kTrackModifierReference, &myRefIndex1);
			QTInsertChild(myInputMap, kParentAtomIsContainer, kTrackModifierInput, myRefIndex1, 0, 0, NULL, &myInputAtom);
	
			myInputType = EndianU32_NtoB(kTrackModifierTypeImage);
			QTInsertChild(myInputMap, myInputAtom, kTrackModifierType, 1, 0, sizeof(myInputType), &myInputType, NULL);
	
			myLong = EndianU32_NtoB(kSourceOneName);
			QTInsertChild(myInputMap, myInputAtom, kEffectDataSourceType, 1, 0, sizeof(myLong), &myLong, NULL);
		}

		// second input
		if (videoTrackB) {
		
			AddTrackReference(videoTrackFX, videoTrackB, kTrackModifierReference, &myRefIndex2);
			QTInsertChild(myInputMap, kParentAtomIsContainer, kTrackModifierInput, myRefIndex2, 0, 0, NULL, &myInputAtom);
	
			myInputType = EndianU32_NtoB(kTrackModifierTypeImage);
			QTInsertChild(myInputMap, myInputAtom, kTrackModifierType, 1, 0, sizeof(myInputType), &myInputType, NULL);
	
			myLong = EndianU32_NtoB(kSourceTwoName);
			QTInsertChild(myInputMap, myInputAtom, kEffectDataSourceType, 1, 0, sizeof(myLong), &myLong, NULL);
		}

		// set that map
		SetMediaInputMap(GetTrackMedia(videoTrackFX), myInputMap);
		
		QTDisposeAtomContainer(myInputMap);
	}

	myCurrentDuration = 0;

#if MAKE_STILL_SECTIONS
	// copy the first sample of the first video track of the first movie to videoTrackFX, with duration kStillDuration.
	myErr = CopyPortionOfTrackToTrack(myPrevSrcTrack, eStartPortion + eMiddlePortion, videoTrackFX, myCurrentDuration, &myReturnedDuration);
	BailError(myErr);
	
	myCurrentDuration += myReturnedDuration;
#endif 

	// now process any remaining files
	myMovieIter = 1;
	while (myMovieIter < gSpecCount) {
		
		// open the next file as a movie; call it nextSourceMovie
		myErr = OpenMovieFile(&gSpecList[myMovieIter], &mySrcMovieRefNum, fsRdPerm);
		BailError(myErr);
		
		myErr = NewMovieFromFile(&myNextSrcMovie, mySrcMovieRefNum, NULL, NULL, 0, NULL);
		BailError(myErr);
		
		// we're done with the movie file, so close it
		myErr = CloseMovieFile(mySrcMovieRefNum);
		BailError(myErr);
		
		// if the movie is shorter than kMinimumDuration, scale it to that length
		SetMovieTimeScale(myNextSrcMovie, kTimeScale);
		myErr = QTEffects_GetFirstVideoTrackInMovie(myNextSrcMovie, &myNextSrcTrack);
		BailNil(myNextSrcTrack);
		
		if (GetTrackDuration(myNextSrcTrack) < kMinimumDuration) {
			myErr = ScaleTrackSegment(myNextSrcTrack, 0, GetTrackDuration(myNextSrcTrack), kMinimumDuration);
			BailError(myErr);
		}

		// create a transition effect from the previous source movie's first video sample to the next source movie's first video sample
		// (the effect should have duration kEffectDuration);
		// this involves adding one sample to each of the three video tracks:
		
		//    sample from previous source movie	 -> videoTrackA
		myErr = QTEffects_CopyPortionOfTrackToTrack(myPrevSrcTrack, eFinishPortion, videoTrackA, myCurrentDuration, &myReturnedDuration);
		BailError(myErr);
		
		//    sample from next source movie    	 -> videoTrackB
		myErr = QTEffects_CopyPortionOfTrackToTrack(myNextSrcTrack, eStartPortion, videoTrackB, myCurrentDuration, &myReturnedDuration);
		BailError(myErr);
		
		//    effect sample                 	  -> videoTrackFX
		if (isFirstTransition) {
			myMediaTransitionDuration = myReturnedDuration;
			myMediaFXStartTime = GetMediaDuration(videoMediaFX);
			myErr = AddMediaSample(videoMediaFX, gEffectSample, 0, GetHandleSize(gEffectSample), myMediaTransitionDuration, (SampleDescriptionHandle)myDesc, 1, 0, NULL);
			BailError(myErr);
			
			myMediaFXDuration = GetMediaDuration(videoMediaFX) - myMediaFXStartTime;
			isFirstTransition = false;
		}
		
		myErr = InsertMediaIntoTrack(videoTrackFX, myCurrentDuration, myMediaFXStartTime, myMediaFXDuration, FixRatio(myReturnedDuration, myMediaTransitionDuration));
		BailError(myErr);
		
		myCurrentDuration += myReturnedDuration;
		
#if MAKE_STILL_SECTIONS
		// copy the first video sample of myNextSrcMovie to videoTrackFX, with duration kStillDuration.
		myErr = QTEffects_CopyPortionOfTrackToTrack(myNextSrcTrack, eMiddlePortion + (myMovieIter + 1 == theSpecCount) ? eFinishPortion : 0, videoTrackFX, myCurrentDuration, &myReturnedDuration);
		BailError(myErr);
		
		myCurrentDuration += myReturnedDuration;
#endif // MAKE_STILL_SECTIONS
		
		// dispose of previous source movie.  
		DisposeMovie(myPrevSrcMovie);
		
		myPrevSrcMovie = myNextSrcMovie;
		myPrevSrcTrack = myNextSrcTrack;
		myNextSrcMovie = NULL;
		myNextSrcTrack = NULL;
		
		myMovieIter++;
	} // while
	
	myErr = EndMediaEdits(videoMediaFX);
	BailError(myErr);

	myErr = AddMovieResource(myDestMovie, myDestMovieRefNum, &myResID, "\pMovie 1");
	BailError(myErr);
	
	CloseMovieFile(myDestMovieRefNum);
	
	if (myPrevSrcMovie != NULL)
		DisposeMovie(myPrevSrcMovie);
		
	DisposeMovie(myDestMovie);
	
bail:
	free(myPrompt);
	free(myFileName);

	QTDisposeAtomContainer(gEffectSample);
	DisposeHandle((Handle)myDesc);

	return;
}


#if TARGET_OS_WIN32
//////////
//
// QTEffects_EffectsDialogCallback
// This function is called by QTML when it processes events for the standard or custom effects parameter dialog box.
// 
//////////

static void QTEffects_EffectsDialogCallback (EventRecord *theEvent, DialogRef theDialog, DialogItemIndex theItemHit)
{
	QTParamDialogEventRecord	myRecord;

	myRecord.theEvent = theEvent;
	myRecord.whichDialog = theDialog;
	myRecord.itemHit = theItemHit;

	// 
	QTStandardParameterDialogDoAction(gEffectsDialog, pdActionModelessCallback, &myRecord);
	
	// see if the event is meant for the effects parameter dialog box
	QTEffects_HandleEffectsDialogEvents(theEvent, theItemHit);
}


//////////
//
// QTEffects_CustomDialogWndProc
// Handle messages for the custom effects parameters dialog box.
// 
//////////

LRESULT CALLBACK QTEffects_CustomDialogWndProc (HWND theWnd, UINT theMessage, UINT wParam, LONG lParam)
{
	EventRecord			myEvent = {0};
	
	if (!gDoneWithDialog && (theMessage == 0x7FFF))
		QTEffects_EffectsDialogCallback(&myEvent, GetNativeWindowPort(theWnd), 0);

	return(DefWindowProc(theWnd, theMessage, wParam, lParam));
}
#endif


//////////
//
// QTEffects_HandleEffectsDialogEvents
// Process events that might be targeted at the standard effects parameter dialog box.
// Return true if the event was completely handled.
// 
//////////

Boolean QTEffects_HandleEffectsDialogEvents (EventRecord *theEvent, DialogItemIndex theItemHit)
{
	Boolean			isHandled = false;
	OSErr			myErr = noErr;
	
	// pass the event to the standard effects parameter dialog box handler
	myErr = QTIsStandardParameterDialogEvent(theEvent, gEffectsDialog);
	
	// the result from QTIsStandardParameterDialogEvent tells us how to respond next
	switch (myErr) {
		
		case codecParameterDialogConfirm:
		case userCanceledErr:
			// the user clicked the OK or Cancel button; dismiss the dialog box and respond accordingly
			gDoneWithDialog = true;
			QTDismissStandardParameterDialog(gEffectsDialog);
			QTEffects_RespondToDialogSelection(myErr);
			gEffectsDialog = 0L;
			isHandled = true;
			break;
			
		case noErr:
			// the event was completely handled by QTIsStandardParameterDialogEvent
			isHandled = true;
			break;
			
		case featureUnsupported:
			// the event was not handled by QTIsStandardParameterDialogEvent;
			// let the event be processed normally
			isHandled = false;
			break;
			
		default:
			// the event was not handled by QTIsStandardParameterDialogEvent;
			// do not let the event be processed normally
			isHandled = true;
			break;
	}

	return(isHandled);
}


//////////
//
// QTEffects_PromptUserForFilesAndMakeEffect
// Let the user select some movies, then apply the effect to them.
//
// If the user cancels the first file-open dialog box, there are zero sources.
// If the user cancels the second file-open dialog box, there is one source.
// 
//////////

void QTEffects_PromptUserForFilesAndMakeEffect (void)
{
	QTFrameFileFilterUPP	myFileFilterUPP = NULL;
	int						mySpecCount = 0;
	OSType 					myTypeList[] = {kQTFileTypeMovie};
	short					myNumTypes = 1;
	OSErr					myErr = noErr;
	
#if TARGET_OS_MAC
	myNumTypes = 0;
#endif

	myFileFilterUPP = QTFrame_GetFileFilterUPP((ProcPtr)QTFrame_FilterFiles);

	// ask for up to kMaxNumSources movie files;
	// accept early cancels; they just mean there are fewer input movies
	mySpecCount = 0;
	while (mySpecCount < kMaxNumSources) {
		FSSpec	myFSSpec;

		myTypeList[0] = MovieFileType;

		myErr = QTFrame_GetOneFileWithPreview(myNumTypes, (QTFrameTypeListPtr)myTypeList, &myFSSpec, myFileFilterUPP);
		if (myErr != noErr)
			break;	// the user doesn't want any more source movies
	
		// save the FSSpec from the reply information
		gSpecList[mySpecCount] = myFSSpec;
		
		mySpecCount++;
	}
	
	QTEffects_DisplayDialogForSources(gSpecList, mySpecCount);
	
	if (myFileFilterUPP != NULL)
		DisposeNavObjectFilterUPP(myFileFilterUPP);
}
