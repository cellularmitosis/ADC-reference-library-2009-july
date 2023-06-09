//////////
//
//	File:		AddVRActions.c
//
//	Contains:	Sample code for adding wired actions to a QuickTime VR movie.
//
//	Written by:	Tim Monroe
//				Based on existing code by Bill Wright.
//
//	Copyright:	� 1999 by Apple Computer, Inc., all rights reserved.
//
//	Change History (most recent first):
//
//	   <4>	 	03/21/00	rtm		changes for supporting CarbonLib
//	   <3>	 	11/18/99	rtm		changed _SetFrameLoadedWiredActions to _SetWiredActionsToNode,
//									to allow support for idle events as well as frame-loaded events;
//									added AddVRAct_CreateIdleActionContainer to illustrate use of
//									idle events; here we autopan to the left on idle events
//	   <2>	 	07/15/99	rtm		added Endian macros; runs fine on both Mac and Windows;
//									added comments; removed reliance on AtomUtilities.c and
//									FileUtilities.c
//	   <1>	 	07/14/99	rtm		first file from bw; revised to sample code coding style
//	   
//
//	This file contains some sample code that adds a few wired actions to a QuickTime VR movie.
//	Currently you can add two kinds of wired actions to VR movies: (1) actions that are global
//	to a particular node and (2) actions associated with a particular hot spot in a node. An
//	example of a node-specific action might be setting the pan and tilt angles that are used
//	when the user first enters the node. An example of a hot-spot-specific action might be playing
//	a sound when the mouse is moved over the hot spot.
//
//	All currently-supported QTVR wired actions are specific to some particular node, so the atom
//	containers implementing the actions are placed in the node information atom container that is
//	contained in the media sample for that node in the QTVR track. (See the book Virtual Reality
//	Programming With QuickTime VR 2.x for complete information on the format of VR movie files.)
//	So our job here boils down to finding a media sample in the QTVR track, constructing some atom
//	containers for our desired actions, placing those action containers into the appropriate place
//	in the media sample, and then writing the modified media sample back into the QTVR track. We
//	also need to put an atom into the media property atom container to enable wired action processing.
//
//	For complete information about wired actions, see the chapter "Wired Sprites" in the book
//	Programming With QuickTime Sprites.
//
//////////

#include "AddVRActions.h"


Str255				gAppName;							// the name of this application


//////////
//
// main/WinMain 
// The main function for this application.
//
//////////

#if TARGET_OS_MAC
void main (void)
#elif TARGET_OS_WIN32
int CALLBACK WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR theCmdLine, int nCmdShow)
#endif
{
	OSType					myTypeList = MovieFileType;
	short					myNumTypes = 1;
	FSSpec					myFile;
	OSErr					myErr = noErr;

#if TARGET_OS_WIN32
	InitializeQTML(0L);											// initialize QuickTime Media Layer
#endif	

#if TARGET_OS_MAC
#if !TARGET_API_MAC_CARBON	
	MaxApplZone();												// init everything
	InitGraf(&qd.thePort);
	InitFonts();
	FlushEvents(everyEvent, 0);
	InitWindows();
	InitMenus();
	InitDialogs(NULL);
	TEInit();
#endif
	InitCursor();
	
	// get the application's name from the resource file
	GetIndString(gAppName, kAppNameResID, kAppNameResIndex);
#endif	
	
	myErr = EnterMovies();
	if (myErr != noErr)
		goto bail;

	// elicit a movie file from the user
	myErr = AddVRAct_GetOneFileWithPreview(myNumTypes, (QTFrameTypeListPtr)&myTypeList, &myFile, NULL);
	if (myErr != noErr)
		goto bail;

	AddVRAct_AddWiredActionsToQTVRMovie(&myFile);
		
bail:
	ExitMovies();

#if TARGET_OS_WIN32
	// terminate the QuickTime Media Layer
	TerminateQTML();
	return(1);
#endif	

#if TARGET_OS_MAC
	return;
#endif	
}


//////////
//
// AddVRAct_GetFirstHotSpot
// Return, through the theHotSpotID parameter, the ID of the first hot spot in the specified atom container
// (which is assumed to be a node information atom container).
//
// The returned ID is not necessarily the numerically-least ID; it's just the ID of the first hot spot atom
// in the atom container.
//
//////////

static OSErr AddVRAct_GetFirstHotSpot (Handle theSample, long *theHotSpotID)
{
	QTAtom			myHotSpotParentAtom = 0;
	QTAtom			myHotSpotAtom = 0;
	OSErr			myErr = noErr;
	
	*theHotSpotID = 0;

	myHotSpotParentAtom = QTFindChildByIndex(theSample, kParentAtomIsContainer, kQTVRHotSpotParentAtomType, kIndexOne, NULL);
	if (myHotSpotParentAtom != 0)
		myHotSpotAtom = QTFindChildByIndex(theSample, myHotSpotParentAtom, kQTVRHotSpotAtomType, kIndexOne, theHotSpotID);
	
	return(myErr);
}


//////////
//
// AddVRAct_CreateHotSpotActionContainer
// Return, through the theActions parameter, an atom container that contains a hot spot action.
//
// Here we set the pan angle to 10.0 degrees when the hot spot is clicked.
//
//////////

static OSErr AddVRAct_CreateHotSpotActionContainer (QTAtomContainer *theActions)
{
	QTAtom			myEventAtom = 0;
	QTAtom			myActionAtom = 0;
	long			myAction;
	float			myPanAngle;
	OSErr			myErr = noErr;
	
	myErr = QTNewAtomContainer(theActions);
	if (myErr != noErr)
		goto bail;
	
	myErr = QTInsertChild(*theActions, kParentAtomIsContainer, kQTEventType, kQTEventMouseClick, kIndexOne, kZeroDataLength, NULL, &myEventAtom);
	if (myErr != noErr)
		goto bail;
	
	myErr = QTInsertChild(*theActions, myEventAtom, kAction, kIndexOne, kIndexOne, kZeroDataLength, NULL, &myActionAtom);
	if (myErr != noErr)
		goto bail;
	
	myAction = EndianS32_NtoB(kActionQTVRSetPanAngle);
	myErr = QTInsertChild(*theActions, myActionAtom, kWhichAction, kIndexOne, kIndexOne, sizeof(long), &myAction, NULL);
	if (myErr != noErr)
		goto bail;
	
	myPanAngle = 10.0;
	AddVRAct_ConvertFloatToBigEndian(&myPanAngle);
	myErr = QTInsertChild(*theActions, myActionAtom, kActionParameter, kIndexOne, kIndexOne, sizeof(float), &myPanAngle, NULL);
	if (myErr != noErr)
		goto bail;
		
bail:
	return(myErr);
}


//////////
//
// AddVRAct_CreateFrameLoadedActionContainer
// Return, through the theActions parameter, an atom container that contains a frame-loaded event action.
//
// Here we set the pan angle to 180.0 degrees.
//
//////////

static OSErr AddVRAct_CreateFrameLoadedActionContainer (QTAtomContainer *theActions)
{
	QTAtom			myEventAtom = 0;
	QTAtom			myActionAtom = 0;
	long			myAction;
	float			myPanAngle;
	OSErr			myErr = noErr;
	
	myErr = QTNewAtomContainer(theActions);
	if (myErr != noErr)
		goto bail;
	
	myErr = QTInsertChild(*theActions, kParentAtomIsContainer, kQTEventFrameLoaded, kIndexOne, kIndexOne, kZeroDataLength, NULL, &myEventAtom);
	if (myErr != noErr)
		goto bail;
	
	myErr = QTInsertChild(*theActions, myEventAtom, kAction, kIndexOne, kIndexOne, kZeroDataLength, NULL, &myActionAtom);
	if (myErr != noErr)
		goto bail;
	
	myAction = EndianS32_NtoB(kActionQTVRSetPanAngle);
	myErr = QTInsertChild(*theActions, myActionAtom, kWhichAction, kIndexOne, kIndexOne, sizeof(long), &myAction, NULL);
	if (myErr != noErr)
		goto bail;
	
	myPanAngle = 180.0;
	AddVRAct_ConvertFloatToBigEndian(&myPanAngle);
	myErr = QTInsertChild(*theActions, myActionAtom, kActionParameter, kIndexOne, kIndexOne, sizeof(float), &myPanAngle, NULL);
	if (myErr != noErr)
		goto bail;	
		
bail:
	return(myErr);
}	


//////////
//
// AddVRAct_CreateIdleActionContainer
// Return, through the theActions parameter, an atom container that contains an idle event action.
//
// Here we set the pan angle to a relative +1.0 degree, with wrapping at the min and max values.
//
//////////

static OSErr AddVRAct_CreateIdleActionContainer (QTAtomContainer *theActions)
{
	QTAtom			myEventAtom = 0;
	QTAtom			myActionAtom = 0;
	long			myAction;
	float			myPanAngle;
	UInt32			myFlags;
	OSErr			myErr = noErr;
	
	myErr = QTNewAtomContainer(theActions);
	if (myErr != noErr)
		goto bail;
	
	myErr = QTInsertChild(*theActions, kParentAtomIsContainer, kQTEventIdle, kIndexOne, kIndexOne, kZeroDataLength, NULL, &myEventAtom);
	if (myErr != noErr)
		goto bail;
	
	myErr = QTInsertChild(*theActions, myEventAtom, kAction, kIndexOne, kIndexOne, kZeroDataLength, NULL, &myActionAtom);
	if (myErr != noErr)
		goto bail;
	
	myAction = EndianS32_NtoB(kActionQTVRSetPanAngle);
	myErr = QTInsertChild(*theActions, myActionAtom, kWhichAction, kIndexOne, kIndexOne, sizeof(long), &myAction, NULL);
	if (myErr != noErr)
		goto bail;

	myPanAngle = 1.0;
	AddVRAct_ConvertFloatToBigEndian(&myPanAngle);
	myErr = QTInsertChild(*theActions, myActionAtom, kActionParameter, kIndexOne, kIndexOne, sizeof(float), &myPanAngle, NULL);
	if (myErr != noErr)
		goto bail;	
		
	myFlags = EndianU32_NtoB(kActionFlagActionIsDelta | kActionFlagParameterWrapsAround);
	myErr = QTInsertChild(*theActions, myActionAtom, kActionFlags, kIndexOne, kIndexOne, sizeof(UInt32), &myFlags, NULL);
				
bail:
	return(myErr);
}	


//////////
//
// AddVRAct_SetWiredActionsToNode
// Set the specified actions to be a node action of the specified type. If theActions is NULL, remove any
// existing action of that type from theSample.
//
// The theSample parameter is assumed to be a node information atom container; any actions that are global
// to the node should be inserted at the root level of this atom container; in addition, the container type
// should be the same as the event type and should have an atom ID of 1.
//
//////////

static OSErr AddVRAct_SetWiredActionsToNode (Handle theSample, QTAtomContainer theActions, UInt32 theActionType)
{
	QTAtom			myEventAtom = 0;
	QTAtom			myTargetAtom = 0;
	OSErr			myErr = noErr;
	
	// look for an event atom in the specified actions atom container
	if (theActions != NULL)
		myEventAtom = QTFindChildByID(theActions, kParentAtomIsContainer, theActionType, kIndexOne, NULL);
			
	// look for an event atom in the node information atom container
	myTargetAtom = QTFindChildByID(theSample, kParentAtomIsContainer, theActionType, kIndexOne, NULL);
	if (myTargetAtom != 0) {
		// if there is already an event atom in the node information atom container,
		// then either replace it with the one we were passed or remove it
		if (theActions != NULL)
			myErr = QTReplaceAtom(theSample, myTargetAtom, theActions, myEventAtom);	
		else
			myErr = QTRemoveAtom(theSample, myTargetAtom);	
	} else {
		// there is no event atom in the node information atom container,
		// so add in the one we were passed
		if (theActions != NULL)
			myErr = QTInsertChildren(theSample, kParentAtomIsContainer, theActions);
	}
		
	return(myErr);
}


//////////
//
// AddVRAct_SetWiredActionsToHotSpot
// Set the specified actions to be a hot-spot action. If theActions is NULL, remove any existing
// hot-spot actions for the specified hot spot from theSample.
//
//////////

static OSErr AddVRAct_SetWiredActionsToHotSpot (Handle theSample, long theHotSpotID, QTAtomContainer theActions)
{
	QTAtom			myHotSpotParentAtom = 0;
	QTAtom			myHotSpotAtom = 0;
	short			myCount,
					myIndex;
	OSErr			myErr = paramErr;
	
	myHotSpotParentAtom = QTFindChildByIndex(theSample, kParentAtomIsContainer, kQTVRHotSpotParentAtomType, kIndexOne, NULL);
	if (myHotSpotParentAtom == NULL)
		goto bail;
	
	myHotSpotAtom = QTFindChildByID(theSample, myHotSpotParentAtom, kQTVRHotSpotAtomType, theHotSpotID, NULL);
	if (myHotSpotAtom == NULL)
		goto bail;
	
	// see how many events are already associated with the specified hot spot
	myCount = QTCountChildrenOfType(theSample, myHotSpotAtom, kQTEventType);
	
	for (myIndex = myCount; myIndex > 0; myIndex--) {
		QTAtom		myTargetAtom = 0;
		
		// remove all the existing events
		myTargetAtom = QTFindChildByIndex(theSample, myHotSpotAtom, kQTEventType, myIndex, NULL);
		if (myTargetAtom != 0) {
			myErr = QTRemoveAtom(theSample, myTargetAtom);
			if (myErr != noErr)
				goto bail;
		}
	}
	
	if (theActions) {
		myErr = QTInsertChildren(theSample, myHotSpotAtom, theActions);
		if (myErr != noErr)
			goto bail;
	}
	
bail:
	return(myErr);
}


//////////
//
// AddVRAct_WriteMediaPropertyAtom
// Add a media property action to the specified media.
//
// We assume that the data passed to us through the theProperty parameter is big-endian.
//
//////////

static OSErr AddVRAct_WriteMediaPropertyAtom (Media theMedia, long thePropertyID, long thePropertySize, void *theProperty)
{
	QTAtomContainer		myPropertyAtom = NULL;
	QTAtom				myAtom = 0;
	OSErr				myErr = noErr;

	// get the current media property atom
	myErr = GetMediaPropertyAtom(theMedia, &myPropertyAtom);
	if (myErr != noErr)
		goto bail;
	
	// if there isn't one yet, then create one
	if (myPropertyAtom == NULL) {
		myErr = QTNewAtomContainer(&myPropertyAtom);
		if (myErr != noErr)
			goto bail;
	}

	// see if there is an existing atom of the specified type; if not, then create one
	myAtom = QTFindChildByID(myPropertyAtom, kParentAtomIsContainer, thePropertyID, kIndexOne, NULL);
	if (myAtom == NULL) {
		myErr = QTInsertChild(myPropertyAtom, kParentAtomIsContainer, thePropertyID, kIndexOne, kIndexZero, kZeroDataLength, NULL, &myAtom);
		if ((myErr != noErr) || (myAtom == NULL))
			goto bail;
	}
	
	// set the data of the specified atom to the data passed in
	myErr = QTSetAtomData(myPropertyAtom, myAtom, thePropertySize, (Ptr)theProperty);
	if (myErr != noErr)
		goto bail;

	// write the new atom data out to the media property atom
	myErr = SetMediaPropertyAtom(theMedia, myPropertyAtom);

bail:
	if (myPropertyAtom != NULL)
		QTDisposeAtomContainer(myPropertyAtom);
	
	return(myErr);
}


//////////
//
// AddVRAct_AddWiredActionsToQTVRMovie
// Add some wired actions to the specified QTVR movie.
//
// Wired actions are added to a QTVR movie by adding atom containers in the appropriate locations.
//
//////////

static void AddVRAct_AddWiredActionsToQTVRMovie (FSSpec *theFSSpec)
{	
	short							myResID = 0;
	short							myResRefNum = -1;
	Movie							myMovie = NULL;
	Track							myTrack = NULL;
	Media							myMedia = NULL;
	TimeValue						myTrackOffset;
	TimeValue						myMediaTime;
	TimeValue						mySampleDuration;
	TimeValue						mySelectionDuration;
	TimeValue						myNewMediaTime;
	QTVRSampleDescriptionHandle		myQTVRDesc = NULL;
	Handle							mySample = NULL;
	short							mySampleFlags;
	Fixed 							myTrackEditRate;
	QTAtomContainer					myActions = NULL;	
	Boolean							myHasActions;
	long							myHotSpotID = 0L;						
	UInt32							myFrequency;
	OSErr							myErr = noErr;
	
	//////////
	//
	// open the movie file and get the QTVR track from the movie
	//
	//////////
	
	// open the movie file for reading and writing
	myErr = OpenMovieFile(theFSSpec, &myResRefNum, fsRdWrPerm);
	if (myErr != noErr)
		goto bail;

	myErr = NewMovieFromFile(&myMovie, myResRefNum, &myResID, NULL, newMovieActive, NULL);
	if (myErr != noErr)
		goto bail;
		
	// find the first QTVR track in the movie;
	// this assumes that the movie is a QuickTime VR movie formatted according to version 2.0
	// or later (version 1.0 VR movies don't have a QTVR track)
	myTrack = GetMovieIndTrackType(myMovie, kIndexOne, kQTVRQTVRType, movieTrackMediaType);
	if (myTrack == NULL)
		goto bail;
	
	//////////
	//
	// get the first media sample in the QTVR track
	//
	// the QTVR track contains one media sample for each node in the movie;
	// that sample contains a node information atom container, which contains general information
	// about the node (such as its type, its ID, its name, and a list of its hot spots)
	//
	//////////
	
	myMedia = GetTrackMedia(myTrack);
	if (myMedia == NULL)
		goto bail;
	
	myTrackOffset = GetTrackOffset(myTrack);
	myMediaTime = TrackTimeToMediaTime(myTrackOffset, myTrack);

	// allocate some storage to hold the sample description for the QTVR track
	myQTVRDesc = (QTVRSampleDescriptionHandle)NewHandle(4);
	if (myQTVRDesc == NULL)
		goto bail;

	mySample = NewHandle(0);
	if (mySample == NULL)
		goto bail;

	myErr = GetMediaSample(myMedia, mySample, 0, NULL, myMediaTime, NULL, &mySampleDuration, (SampleDescriptionHandle)myQTVRDesc, NULL, 1, NULL, &mySampleFlags);
	if (myErr != noErr)
		goto bail;

	//////////
	//
	// add idle actions
	//
	//////////
	
	// create an action container for idle actions
	myErr = AddVRAct_CreateIdleActionContainer(&myActions);
	if (myErr != noErr)
		goto bail;
	
	// add idle actions to sample
	myErr = AddVRAct_SetWiredActionsToNode(mySample, myActions, kQTEventIdle);
	if (myErr != noErr)
		goto bail;
	
	myErr = QTDisposeAtomContainer(myActions);
	if (myErr != noErr)
		goto bail;
	
	//////////
	//
	// add frame-loaded actions
	//
	//////////
	
	// create an action container for frame-loaded actions
	myErr = AddVRAct_CreateFrameLoadedActionContainer(&myActions);
	if (myErr != noErr)
		goto bail;
	
	// add frame-loaded actions to sample
	myErr = AddVRAct_SetWiredActionsToNode(mySample, myActions, kQTEventFrameLoaded);
	if (myErr != noErr)
		goto bail;
	
	myErr = QTDisposeAtomContainer(myActions);
	if (myErr != noErr)
		goto bail;
	
	//////////
	//
	// add hot-spot actions
	//
	//////////
	
	// find the first hot spot in the selected node; don't bail if there are no hot spots
	myErr = AddVRAct_GetFirstHotSpot(mySample, &myHotSpotID);
	if ((myErr == noErr) && (myHotSpotID != 0)) {
		// create an action container for hot-spot actions
		myErr = AddVRAct_CreateHotSpotActionContainer(&myActions);
		if (myErr != noErr)
			goto bail;
		
		// add hot-spot actions to sample 	
		myErr = AddVRAct_SetWiredActionsToHotSpot(mySample, myHotSpotID, myActions);
		if (myErr != noErr)
			goto bail;
	}
	
	//////////
	//
	// replace sample in media
	//
	//////////
	
	myTrackEditRate = GetTrackEditRate(myTrack, myTrackOffset);
	if (GetMoviesError() != noErr)
		goto bail;

	GetTrackNextInterestingTime(myTrack, nextTimeMediaSample | nextTimeEdgeOK, myTrackOffset, fixed1, NULL, &mySelectionDuration);
	if (GetMoviesError() != noErr)
		goto bail;

	myErr = DeleteTrackSegment(myTrack, myTrackOffset, mySelectionDuration);
	if (myErr != noErr)
		goto bail;
		
	myErr = BeginMediaEdits(myMedia);
	if (myErr != noErr)
		goto bail;
	
	myErr = AddMediaSample(	myMedia,
							mySample,
							0,
							GetHandleSize(mySample),
							mySampleDuration,
							(SampleDescriptionHandle)myQTVRDesc, 
							1,
							mySampleFlags,
							&myNewMediaTime);
	if (myErr != noErr)
		goto bail;
	
	myErr = EndMediaEdits(myMedia);
	if (myErr != noErr)
		goto bail;
	
	// add the media to the track
	myErr = InsertMediaIntoTrack(myTrack, myTrackOffset, myNewMediaTime, mySelectionDuration, myTrackEditRate);
	if (myErr != noErr)
		goto bail;

	//////////
	//
	// set the media property atom to enable wired action processing and idle-time processing
	//
	//////////
			
	myHasActions = true;	// since sizeof(Boolean) == 1, there is no need to swap bytes here
	myErr = AddVRAct_WriteMediaPropertyAtom(myMedia, kSpriteTrackPropertyHasActions, sizeof(Boolean), &myHasActions);
	if (myErr != noErr)
		goto bail;

	myFrequency = EndianU32_NtoB(1);
	myErr = AddVRAct_WriteMediaPropertyAtom(myMedia, kSpriteTrackPropertyQTIdleEventsFrequency, sizeof(UInt32), &myFrequency);
	if (myErr != noErr)
		goto bail;
	
	//////////
	//
	// update the movie resource
	//
	//////////
	
	myErr = UpdateMovieResource(myMovie, myResRefNum, myResID, NULL);
		
bail:
	// close the movie file
	if (myResRefNum != -1)
		CloseMovieFile(myResRefNum);
	
	if (myActions != NULL)
		QTDisposeAtomContainer(myActions);
	
	if (mySample != NULL)
		DisposeHandle(mySample);		
	
	if (myQTVRDesc != NULL)
		DisposeHandle((Handle)myQTVRDesc);		
	
	if (myMovie != NULL)
		DisposeMovie(myMovie);	
}


//////////
//
// AddVRAct_ConvertFloatToBigEndian
// Convert the specified floating-point number to big-endian format.
//
//////////

void AddVRAct_ConvertFloatToBigEndian (float *theFloat)
{
	unsigned long		*myLongPtr;
	
	myLongPtr = (unsigned long *)theFloat;
	*myLongPtr = EndianU32_NtoB(*myLongPtr);
}

//////////
//
// AddVRAct_GetOneFileWithPreview
// Display the appropriate file-opening dialog box, with an optional QuickTime preview pane. If the user
// selects a file, return information about it using the theFSSpecPtr parameter.
//
//////////

OSErr AddVRAct_GetOneFileWithPreview (short theNumTypes, QTFrameTypeListPtr theTypeList, FSSpecPtr theFSSpecPtr, void *theFilterProc)
{
#if TARGET_OS_WIN32
	StandardFileReply	myReply;
#endif
#if TARGET_OS_MAC
	NavReplyRecord		myReply;
	NavDialogOptions	myDialogOptions;
	NavTypeListHandle	myOpenList = NULL;
#endif
	OSErr				myErr = noErr;
	
	if (theFSSpecPtr == NULL)
		return(paramErr);
	
#if TARGET_OS_WIN32
	// prompt the user for a file
	StandardGetFilePreview((FileFilterUPP)theFilterProc, theNumTypes, (ConstSFTypeListPtr)theTypeList, &myReply);
	if (!myReply.sfGood)
		return(userCanceledErr);
	
	// make an FSSpec record
	myErr = FSMakeFSSpec(myReply.sfFile.vRefNum, myReply.sfFile.parID, myReply.sfFile.name, theFSSpecPtr);
#endif

#if TARGET_OS_MAC
	// specify the options for the dialog box
	NavGetDefaultDialogOptions(&myDialogOptions);
//	myDialogOptions.dialogOptionFlags -= kNavNoTypePopup;
//	myDialogOptions.dialogOptionFlags -= kNavAllowMultipleFiles;
	BlockMoveData(gAppName, myDialogOptions.clientName, gAppName[0] + 1);
	
	// create a handle to an 'open' resource
	myOpenList = (NavTypeListHandle)AddVRAct_CreateOpenHandle(FOUR_CHAR_CODE('aVRa'), theNumTypes, theTypeList);
	if (myOpenList != NULL)
		HLock((Handle)myOpenList);
	
	// prompt the user for a file
	myErr = NavGetFile(NULL, &myReply, &myDialogOptions, NULL, NULL, (NavObjectFilterUPP)theFilterProc, myOpenList, NULL);
	if ((myErr == noErr) && myReply.validRecord) {
		AEKeyword		myKeyword;
		DescType		myActualType;
		Size			myActualSize = 0;
		
		// get the FSSpec for the selected file
		if (theFSSpecPtr != NULL)
			myErr = AEGetNthPtr(&(myReply.selection), 1, typeFSS, &myKeyword, &myActualType, theFSSpecPtr, sizeof(FSSpec), &myActualSize);

		NavDisposeReply(&myReply);
	}
	
	if (myOpenList != NULL) {
		HUnlock((Handle)myOpenList);
		DisposeHandle((Handle)myOpenList);
	}
	
#endif
 
	return(myErr);
}

#if TARGET_OS_MAC
//////////
//
// AddVRAct_CreateOpenHandle
// Return a handle to a dynamically-created 'open' resource.
//
//////////

Handle AddVRAct_CreateOpenHandle (OSType theApplicationSignature, short theNumTypes, QTFrameTypeListPtr theTypeList)
{
	Handle			myHandle = NULL;
	
	if (theTypeList == NULL)
		return(myHandle);
	
	if (theNumTypes > 0) {
		myHandle = NewHandle(sizeof(NavTypeList) + (theNumTypes * sizeof(OSType)));
		if (myHandle != NULL) {
			NavTypeListHandle 	myOpenResHandle	= (NavTypeListHandle)myHandle;
			
			(*myOpenResHandle)->componentSignature = theApplicationSignature;
			(*myOpenResHandle)->osTypeCount = theNumTypes;
			BlockMoveData(theTypeList, (*myOpenResHandle)->osType, theNumTypes * sizeof(OSType));
		}
	}
	
	return(myHandle);
}
#endif
