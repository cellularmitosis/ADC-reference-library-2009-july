//////////
//
//	File:		MakeEffectMovie.c
//
//	Contains:	QuickTime video effect support for QuickTime movies.
//				This file is used for BOTH MacOS and Windows.
//
//	Written by:	Tim Monroe
//				Based (heavily!) on the previous MakeEffectMovie code written by Sam Bushell,
//				which was based on ConvertToMovie.c from ConvertToMovie Jr. sample code.
//
//	Copyright:	� 1997-1999 by Apple Computer, Inc., all rights reserved.
//
//	Change History (most recent first):
//
//	   <7>	 	03/20/00	rtm		made changes to get things running under CarbonLib
//	   <6>	 	03/03/99	rtm		added support for MakeImageDescriptionForEffect (QT 4.0 and later)
//	   <5>	 	03/12/98	rtm		added global flag gDoneWithDialog to fix Windows dialog box problems
//	   <4>	 	02/28/98	rtm		revised event/message handling following QTShowEffect.c
//	   <3>	 	01/08/98	rtm		sync'ed with latest code from Sam: reworked QTEffects_GetFirstVideoTrackInMovie
//									to use GetMovieIndTrackType and fixed time scale setting code; added global flag
//									gCopyMovieMedia and menu commands to set that flag
//	   <2>	 	11/21/97	rtm		factored out QTEffects_GetFirstVideoTrackInMovie;
//									reworked event-loop processing: changed QTEffects_MakeEffectMovieForSources into
//									QTEffects_DisplayDialogForSources and QTEffects_RespondToDialogSelection.
//	   <1>	 	11/06/97	rtm		first file; integrated existing code with shell framework;
//									added endian macros where appropriate
//	   
//	This application takes the video tracks from zero, one, or two movies 
//	and creates a new movie with an effects track for them.
//
//	Try it out: drag two short movies of the same size onto the application icon.
//  The QuickTime effects dialog will appear and you can select an effect to transition
//	from the first movie to the second. Or, drag a single movie onto the application icon
//	and you'll get an effects dialog for one-source effects.
//
//	You can also launch the application and choose the Make Effect Movie menu command in
//	the Test menu. You'll get a file-opening dialog box for each movie you want to apply
//	the effects to. Just hit Cancel in the first dialog box to get zero-source effects (like fire).
//
//////////

// TO DO:
// + copy sound tracks from original movie(s); add tweening of sound tracks as effect progresses

// header files
#include "MakeEffectMovie.h"

// global variables
QTParameterDialog			gEffectsDialog = 0L;			// identifier for the standard parameter dialog box
QTAtomContainer				gEffectSample = NULL;			// effects sample
QTAtomContainer				gEffectList = NULL;
PicHandle					gPosterA = NULL;
PicHandle					gPosterB = NULL;
Movie						gSrcMovies[kMaxNumSources] = {NULL, NULL};
Track						gSrcTracks[kMaxNumSources] = {NULL, NULL};
UInt16						gSpecCount = 0;		
Boolean						gCopyMovieMedia = false;		// should we copy the movie media into the new effects movie?
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
// QTEffects_DisplayDialogForSources
// Display the standard effects parameters dialog box for the movies passed in.
//
//////////

OSErr QTEffects_DisplayDialogForSources (FSSpec *theSpecList, UInt16 theSpecCount)
{
	OSErr					myErr = noErr;
	UInt16					myMovieIter;
	
	// make sure that there aren't too many sources
	if (theSpecCount > kMaxNumSources) {
		myErr = paramErr;
		goto bail;
	}
	
	// assign source count to a global, so QTEffects_RespondToDialogSelection has access to it
	gSpecCount = theSpecCount;		
	
	// open the source movie files, get movies from them, and get the first video track from each movie
	for (myMovieIter = 0; myMovieIter < theSpecCount; myMovieIter++) {
		short	myRefNum;
		
		// open a movie file using the FSSpec and create a movie from that file
		myErr = OpenMovieFile(&theSpecList[myMovieIter], &myRefNum, 0);
		BailError(myErr);
		
		myErr = NewMovieFromFile(&gSrcMovies[myMovieIter], myRefNum, NULL, NULL, newMovieActive, NULL);
		BailError(myErr);
		
		SetMoviePlayHints(gSrcMovies[myMovieIter], hintsHighQuality, hintsHighQuality);

		// we're done with the movie file, so close it
		CloseMovieFile(myRefNum);
		
		// find the first video track in the source movie
		myErr = QTEffects_GetFirstVideoTrackInMovie(gSrcMovies[myMovieIter], &gSrcTracks[myMovieIter]);
		BailError(myErr);
	}
	
	// ask the user to select an effect

	myErr = QTNewAtomContainer(&gEffectSample);
	BailError(myErr);
	
	myErr = QTGetEffectsList(&gEffectList, theSpecCount, theSpecCount, 0);
	BailError(myErr);
	
	myErr = QTCreateStandardParameterDialog(gEffectList, gEffectSample, 0, &gEffectsDialog);
	BailError(myErr);
	
	// insert poster frames into dialog
	if (gSrcTracks[0] != NULL) {
		gPosterA = GetTrackPict(gSrcTracks[0], GetMoviePosterTime(gSrcMovies[0]));
		if (gPosterA != NULL) {
			QTParamPreviewRecord			myPreviewRecord;

			myPreviewRecord.sourcePicture = gPosterA;
			myPreviewRecord.sourceID = 1;
			myErr = QTStandardParameterDialogDoAction(gEffectsDialog, pdActionSetPreviewPicture, &myPreviewRecord);
		}
	}

	if (gSrcTracks[1] != NULL) {
		gPosterB = GetTrackPict(gSrcTracks[1], GetMoviePosterTime(gSrcMovies[1]));
		if (gPosterB != NULL) {
			QTParamPreviewRecord			myPreviewRecord;

			myPreviewRecord.sourcePicture = gPosterB;
			myPreviewRecord.sourceID = 2;
			myErr = QTStandardParameterDialogDoAction(gEffectsDialog, pdActionSetPreviewPicture, &myPreviewRecord);
		}
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
//////////

void QTEffects_RespondToDialogSelection (OSErr theErr)
{
	short					myMovieRefNum = 0;
	short					myResID = movieInDataForkResID;
	Boolean					myDialogWasCancelled = false;
	OSType					myEffectCode;
	Fixed					videoTrackFXWidth, videoTrackFXHeight;
	TimeScale				myMovieTimeScale = 600; 
	TimeValue				myEffectDuration = 0;
	TimeValue				mySampleTime = 0;
	FSSpec					myFile;
	Boolean					myIsSelected = false;
	Boolean					myIsReplacing = false;	
	StringPtr 				myPrompt = QTUtils_ConvertCToPascalString(kEffectsSaveMoviePrompt);
	StringPtr 				myFileName = QTUtils_ConvertCToPascalString(kEffectsSaveMovieFileName);
	Movie					myDestMovie = NULL;
	Track					videoTracks[kMaxNumSources] = {NULL, NULL};
	Track					videoTrackFX = NULL;
	Media					videoMediaFX = NULL;
	UInt16					myMovieIter;
	long					myFlags = createMovieFileDeleteCurFile | createMovieFileDontCreateResFile;
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
	
	if (myDialogWasCancelled)
		goto bail;
	
	// add atoms naming the sources to gEffectSample
	{
		long	myLong;
		
		if (gSpecCount >= 1) {
			myLong = EndianU32_NtoB(kSourceOneName);
			QTInsertChild(gEffectSample, kParentAtomIsContainer, kEffectSourceName, 1, 0, sizeof(myLong), &myLong, NULL);
		}
		
		if (gSpecCount >= 2) {
			myLong = EndianU32_NtoB(kSourceTwoName);
			QTInsertChild(gEffectSample, kParentAtomIsContainer, kEffectSourceName, 2, 0, sizeof(myLong), &myLong, NULL);
		}
	}
	
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
	myErr = CreateMovieFile(&myFile, FOUR_CHAR_CODE('TVOD'), smSystemScript, myFlags, &myMovieRefNum, &myDestMovie);
	BailError(myErr);
	
	// copy the user data and settings from the source to the destination movie;
	// these settings include information like user data
	if (gSpecCount > 0)
		CopyMovieSettings(gSrcMovies[0], myDestMovie);
	
	// convert all the movies to have a common time scale;
	// we pick the largest time scale out of all the source movies, with a minimum of 600
	myMovieTimeScale = 600;
	for (myMovieIter = 0; myMovieIter < gSpecCount; myMovieIter++) {
		if (myMovieTimeScale < GetMovieTimeScale(gSrcMovies[myMovieIter]))
			myMovieTimeScale = GetMovieTimeScale(gSrcMovies[myMovieIter]);
	}
	
	for (myMovieIter = 0; myMovieIter < gSpecCount; myMovieIter++) {
		if (myMovieTimeScale != GetMovieTimeScale(gSrcMovies[myMovieIter]))
			SetMovieTimeScale(gSrcMovies[myMovieIter], myMovieTimeScale);
	}
	
	// the effect duration is the minimum of the length of the tracks
	if (gSpecCount == 0)
		myEffectDuration = myMovieTimeScale * 10;
	else
		myEffectDuration = GetTrackDuration(gSrcTracks[0]);
	
	for (myMovieIter = 1; myMovieIter < gSpecCount; myMovieIter++) {
		if (myEffectDuration > GetTrackDuration(gSrcTracks[myMovieIter]))
			myEffectDuration = GetTrackDuration(gSrcTracks[myMovieIter]);
	}
	
	// default size when there are no video tracks
	videoTrackFXWidth = 160L << 16;
	videoTrackFXHeight = 120L << 16;

	for (myMovieIter = 0; myMovieIter < kMaxNumSources; myMovieIter++) {
		videoTracks[myMovieIter] = NULL;
	}
	
	// make the video tracks
	for (myMovieIter = 0; myMovieIter < gSpecCount; myMovieIter++) {
		Fixed	mySrcTrackWidth, mySrcTrackHeight;
		OSType	mySrcMediaType = 0;
		Media	videoMedia;

		// videoTracks[n] is a clone of gSrcTracks[n]
		GetTrackDimensions(gSrcTracks[myMovieIter], &mySrcTrackWidth, &mySrcTrackHeight);
		
		if ((myMovieIter == 0) || (videoTrackFXWidth < mySrcTrackWidth))
				videoTrackFXWidth = mySrcTrackWidth;
				
		if ((myMovieIter == 0) || (videoTrackFXHeight < mySrcTrackHeight))
				videoTrackFXHeight = mySrcTrackHeight;
		
		GetMediaHandlerDescription(GetTrackMedia(gSrcTracks[myMovieIter]), &mySrcMediaType, NULL, NULL);

		videoTracks[myMovieIter] = NewMovieTrack(myDestMovie, mySrcTrackWidth, mySrcTrackHeight, 0);
		BailNil(videoTracks[myMovieIter]);
		videoMedia = NewTrackMedia(videoTracks[myMovieIter], mySrcMediaType, myMovieTimeScale, NULL, 0);
		BailNil(videoMedia);
		
		if (gCopyMovieMedia) {
			myErr = BeginMediaEdits(videoMedia);
			BailError(myErr);
		}

		myErr = CopyTrackSettings(gSrcTracks[myMovieIter], videoTracks[myMovieIter]);
		BailError(myErr);
		myErr = InsertTrackSegment(gSrcTracks[myMovieIter], videoTracks[myMovieIter], 0, myEffectDuration, 0);
		BailError(myErr);
		
		if (gCopyMovieMedia) {
			myErr = EndMediaEdits(videoMedia);
			BailError(myErr);
		}
	}
	
	{
		// videoTrackFX is the special track that implements the effect
		videoTrackFX = NewMovieTrack(myDestMovie, videoTrackFXWidth, videoTrackFXHeight, 0);
		BailNil(videoTrackFX);
		videoMediaFX = NewTrackMedia(videoTrackFX, VideoMediaType, myMovieTimeScale, NULL, 0);
		BailNil(videoMediaFX);
	}

	{
		ImageDescriptionHandle		myDesc = NULL;

#if USES_MAKE_IMAGE_DESC_FOR_EFFECT
		OSErr						myErr = noErr;
		
		// create a new sample description
		myErr = MakeImageDescriptionForEffect(myEffectCode, &myDesc);
		if (myErr != noErr)
			BailError(myErr);
#else
		// create a new sample description
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
		(**myDesc).width = videoTrackFXWidth >> 16;
		(**myDesc).height = videoTrackFXHeight >> 16;
		
		// add effects sample to movie
		myErr = BeginMediaEdits(videoMediaFX);
		BailError(myErr);

		myErr = AddMediaSample(videoMediaFX, gEffectSample, 0, GetHandleSize(gEffectSample), myEffectDuration, (SampleDescriptionHandle)myDesc, 1, 0, &mySampleTime);
		BailError(myErr);

		myErr = EndMediaEdits(videoMediaFX);
		BailError(myErr);

		QTDisposeAtomContainer(gEffectSample);
		DisposeHandle((Handle)myDesc);
	}

	myErr = InsertMediaIntoTrack(videoTrackFX, 0, mySampleTime, myEffectDuration, fixed1);
	BailError(myErr);

	// create and add the input map
	{
		long				myRefIndex1, myRefIndex2;
		QTAtomContainer		myInputMap;
		QTAtom				myInputAtom;
		OSType				myInputType;
		long				myLong;

		QTNewAtomContainer(&myInputMap);

		// first input
		if (videoTracks[0]) {
			AddTrackReference(videoTrackFX, videoTracks[0], kTrackModifierReference, &myRefIndex1);
			QTInsertChild(myInputMap, kParentAtomIsContainer, kTrackModifierInput, myRefIndex1, 0, 0, NULL, &myInputAtom);
	
			myInputType = EndianU32_NtoB(kTrackModifierTypeImage);
			QTInsertChild(myInputMap, myInputAtom, kTrackModifierType, 1, 0, sizeof(myInputType), &myInputType, NULL);
	
			myLong = EndianU32_NtoB(kSourceOneName);
			QTInsertChild(myInputMap, myInputAtom, kEffectDataSourceType, 1, 0, sizeof(myLong), &myLong, NULL);
		}

		// second input
		if (videoTracks[1]) {
			AddTrackReference(videoTrackFX, videoTracks[1], kTrackModifierReference, &myRefIndex2);
			QTInsertChild(myInputMap, kParentAtomIsContainer, kTrackModifierInput, myRefIndex2, 0, 0, NULL, &myInputAtom);
	
			myInputType = EndianU32_NtoB(kTrackModifierTypeImage);
			QTInsertChild(myInputMap, myInputAtom, kTrackModifierType, 1, 0, sizeof(myInputType), &myInputType, NULL);
	
			myLong = EndianU32_NtoB(kSourceTwoName);
			QTInsertChild(myInputMap, myInputAtom, kEffectDataSourceType, 1, 0, sizeof(myLong), &myLong, NULL);
		}

		// set that map
		if (gSpecCount > 0)
			SetMediaInputMap(GetTrackMedia(videoTrackFX), myInputMap);
		
		QTDisposeAtomContainer(myInputMap);
	}

	myErr = AddMovieResource(myDestMovie, myMovieRefNum, &myResID, "\pMovie 1");
	BailError(myErr);
	
	CloseMovieFile(myMovieRefNum);
	
	for (myMovieIter = 0; myMovieIter < gSpecCount; myMovieIter++)
		DisposeMovie(gSrcMovies[myMovieIter]);
		
	DisposeMovie(myDestMovie);
	
bail:
	free(myPrompt);
	free(myFileName);

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

	if (gEffectsDialog != 0L) {
		QTStandardParameterDialogDoAction(gEffectsDialog, pdActionModelessCallback, &myRecord);
	
		// see if the event is meant for the effects parameter dialog box
		QTEffects_HandleEffectsDialogEvents(theEvent, theItemHit);
	}
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
			QTStandardParameterDialogDoAction(gEffectsDialog, pdActionConfirmDialog, NULL);
			QTDismissStandardParameterDialog(gEffectsDialog);
			gEffectsDialog = 0L;
			QTEffects_RespondToDialogSelection(myErr);
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
	FSSpec					mySpecs[kMaxNumSources];
	int						mySpecCount;
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
		mySpecs[mySpecCount] = myFSSpec;
		
		mySpecCount++;
	}
	
	QTEffects_DisplayDialogForSources(mySpecs, mySpecCount);
	
	if (myFileFilterUPP != NULL)
		DisposeNavObjectFilterUPP(myFileFilterUPP);
}
