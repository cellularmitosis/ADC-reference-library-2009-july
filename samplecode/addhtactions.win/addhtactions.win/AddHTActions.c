//////////
//
//	File:		AddHTActions.c
//
//	Contains:	Sample code for adding hypertext links to a QuickTime movie with a text track.
//
//	Written by:	Tim Monroe
//				Based on existing code by Bill Wright
//
//	Copyright:	© 1999 by Apple Computer, Inc., all rights reserved.
//
//	Change History (most recent first):
//
//	   <4>	 	07/10/02	rtm		tweaked position of CloseMovieFile in AddHTAct_AddHyperTextToTextMovie
//	   <3>	 	08/07/00	rtm		fixed bug in AddHTAct_PutFile that caused failure on Mac OS X
//	   <2>	 	03/21/00	rtm		changes for supporting CarbonLib
//	   <1>	 	07/19/99	rtm		first file from bw; revised to sample code coding style;
//									added Endian macros and other niceties for cross-platform
//									support; works fine on Windows and MacOS
//	   
//	This file contains some sample code that adds a few wired actions to a movie that has a
//	text track. In particular, it adds two go-to-URL actions to parts of the text track. 
//
//	A text media sample is a 16-bit length word followed by the text of that sample. Optionally,
//	one or more atoms of additional data (called text atom extensions) may follow the text in the
//	sample. These text atom extensions are organized as "classic" atom structures: a 32-bit length
//	field, followed by a 32-bit type field, followed by the data in the atom. Here, the length field
//	specifies the total length of the atom (that is, 8 plus the number of bytes in the data).
//	All the data in the text extension atom must be in big-endian format.
//	
//	To add hypertext actions to a text sample, you simply add a text atom extension of the type
//	kHyperTextTextAtomType to the end of the sample; the data of the text atom extension is the
//	container that holds the information about the actions. So our task boils down to this: find
//	a text sample, create an atom container holding information about the desired actions, and then
//	append a text extension atom whose data is that atom container to the end of the text sample.
//	Then replace the previous text sample with the new one in the text track.
//
//	For complete information about wired actions, see the chapter "Wired Sprites" in the book
//	Programming With QuickTime Sprites.
//
//////////

#include "AddHTActions.h"


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
	OSType					myTypeList[1] = {MovieFileType};
	FSSpec					myFile;
	Boolean					myIsSelected = false;
	Boolean					myIsReplacing = false;	
	StringPtr				myPrompt = AddHTAct_ConvertCToPascalString(kSavePrompt);
	StringPtr				myDefaultName = AddHTAct_ConvertCToPascalString(kSaveFileName);
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

	// elicit a file from the user to save the new movie into
	AddHTAct_PutFile(myPrompt, myDefaultName, &myFile, &myIsSelected, &myIsReplacing);

	// create a text movie with hypertext links
	if (myIsSelected) {
		myErr = AddHTAct_CreateTextMovie(&myFile);
		if (myErr == noErr)
			myErr = AddHTAct_AddHyperTextToTextMovie(&myFile);
	}
	
bail:
	free(myPrompt);
	free(myDefaultName);
	
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
// AddHTAct_CreateTextMovie 
// Create a movie with a text track.
//
//////////

static OSErr AddHTAct_CreateTextMovie (FSSpec *theFSSpec)
{
	short			myResRefNum = -1;
	short			myResID = movieInDataForkResID;
	Movie			myMovie = NULL;
	Track			myTrack = NULL;
	Media			myMedia = NULL;
	MediaHandler	myHandler = NULL;
	Rect			myTextBox;
	RGBColor		myTextColor =	{0x6666, 0xCCCC, 0xCCCC};
	RGBColor		myBackColor =	{0x3333, 0x6666, 0x6666};
	RGBColor		myHiliteColor =	{0x0000, 0x0000, 0x0000};
	long			myDisplayFlags = 0;
	short			myHiliteStart = 0;
	short			myHiliteEnd = 0;
	TimeValue		myNewMediaTime;
	TimeValue		myScrollDelay = 0;
#if TARGET_OS_MAC
	char			myText[] = "\rPlease take me to Apple or CNN";
#else
	char			myText[] = "\nPlease take me to Apple or CNN";
#endif
	long			myFlags = createMovieFileDeleteCurFile | createMovieFileDontCreateResFile | newMovieActive;
	OSErr			myErr = noErr;
		
	//////////
	//
	// create a new text movie
	//
	//////////

	myErr = CreateMovieFile(theFSSpec, FOUR_CHAR_CODE('TVOD'), 0, myFlags, &myResRefNum, &myMovie);
	if (myErr != noErr)
		goto bail;
	
	myTrack = NewMovieTrack(myMovie, FixRatio(kWidth320, 1), FixRatio(kHeight240, 1), kTrackVolumeZero);	
	myMedia = NewTrackMedia(myTrack, TextMediaType, kTimeScale600, NULL, 0);
	if ((myTrack == NULL) || (myMedia == NULL))
		goto bail;
	
	myErr = BeginMediaEdits(myMedia);
	if (myErr != noErr)
		goto bail;

	myHandler = GetMediaHandler(myMedia);
	if (myHandler == NULL)
		goto bail;

	//////////
	//
	// add a text sample to the movie
	//
	//////////

	MacSetRect(&myTextBox, 0, 0, kWidth320, kHeight240);
	MacInsetRect(&myTextBox, kTextBoxInset, kTextBoxInset);
	
	myErr = (OSErr)TextMediaAddTextSample(	myHandler,
											myText,
											strlen(myText),
											kFontIDSymbol,
											kSize48,
											kFacePlain,
											&myTextColor,
											&myBackColor,
											teCenter,
											&myTextBox,
											myDisplayFlags,
											myScrollDelay,
											myHiliteStart,
											myHiliteEnd,
											&myHiliteColor,
											kTimeScale600,
											&myNewMediaTime);
	if (myErr != noErr)
		goto bail;

	myErr = EndMediaEdits(myMedia);
	if (myErr != noErr)
		goto bail;
		
	// add the media to the track, at time 0
	myErr = InsertMediaIntoTrack(myTrack, kTrackStartTimeZero, myNewMediaTime, kTimeScale600, fixed1);
	if (myErr != noErr)
		goto bail;
	
	// add the movie resource
	myErr = AddMovieResource(myMovie, myResRefNum, &myResID, NULL);
	
bail:
	if (myResRefNum != -1)
		CloseMovieFile(myResRefNum);
	
	if (myMovie != NULL)
		DisposeMovie(myMovie);
	
	return(myErr);
}


//////////
//
// AddHTAct_CreateHyperTextActionContainer
// Return, through the theActions parameter, an atom container that contains some hypertext actions.
//
//////////

static OSErr AddHTAct_CreateHyperTextActionContainer (QTAtomContainer *theActions)
{
	QTAtom			myEventAtom = 0;
	QTAtom			myActionAtom = 0;
	QTAtom			myHyperTextAtom = 0;
	QTAtom			myWiredObjectsAtom = 0;
	long			myAction;
	long			mySelStart1 = 19;
	long			mySelEnd1 = 24;
	long			mySelStart2 = 28;
	long			mySelEnd2 = 31;
	long			myValue;
	char			myAppleURL[64] = "\pwww.apple.com\0";
	char			myCnnURL[64] = "\pwww.cnn.com\0";
	OSErr			myErr = noErr;
	
	myErr = QTNewAtomContainer(theActions);
	if (myErr != noErr)
		goto bail;
	
	// create a wired objects atom
	myErr = QTInsertChild(*theActions, kParentAtomIsContainer, kTextWiredObjectsAtomType, kIndexOne, kIndexZero, kZeroDataLength, NULL, &myWiredObjectsAtom);
	if (myErr != noErr)
		goto bail;
	
	//////////
	//
	// add a hypertext link to the wired objects atom: ID 1
	//
	//////////
	
	myErr = QTInsertChild(*theActions, myWiredObjectsAtom, kHyperTextItemAtomType, kIDOne, kIndexZero, kZeroDataLength, NULL, &myHyperTextAtom);
	if (myErr != noErr)
		goto bail;
	
	myValue = EndianS32_NtoB(mySelStart1);
	myErr = QTInsertChild(*theActions, myHyperTextAtom, kRangeStart, kIDOne, kIndexZero, sizeof(long), &myValue, NULL);
	if (myErr != noErr)
		goto bail;
	
	myValue = EndianS32_NtoB(mySelEnd1);
	myErr = QTInsertChild(*theActions, myHyperTextAtom, kRangeEnd, kIDOne, kIndexZero, sizeof(long), &myValue, NULL);
	if (myErr != noErr)
		goto bail;
	
	// add an event atom to the hypertext atom;
	// the event type can be any of the five mouse events: down, up, trigger, enter, exit
	myErr = QTInsertChild(*theActions, myHyperTextAtom, kQTEventType, kQTEventMouseClick, kIndexZero, kZeroDataLength, NULL, &myEventAtom);
	if (myErr != noErr)
		goto bail;
	
	myErr = QTInsertChild(*theActions, myEventAtom, kAction, kIndexOne, kIndexOne, kZeroDataLength, NULL, &myActionAtom);
	if (myErr != noErr)
		goto bail;
	
	myAction = EndianS32_NtoB(kActionGoToURL);
	myErr = QTInsertChild(*theActions, myActionAtom, kWhichAction, kIndexOne, kIndexOne, sizeof(long), &myAction, NULL);
	if (myErr != noErr)
		goto bail;

	myErr = QTInsertChild(*theActions, myActionAtom, kActionParameter, kIndexOne, kIndexOne, myAppleURL[0] + 1, &myAppleURL[1], NULL);
	if (myErr != noErr)
		goto bail;
		
	//////////
	//
	// add a hypertext link to the wired objects atom: ID 2
	//
	//////////

	myErr = QTInsertChild(*theActions, myWiredObjectsAtom, kHyperTextItemAtomType, kIDTwo, kIndexZero, kZeroDataLength, NULL, &myHyperTextAtom);
	if (myErr != noErr)
		goto bail;
	
	myValue = EndianS32_NtoB(mySelStart2);
	myErr = QTInsertChild(*theActions, myHyperTextAtom, kRangeStart, kIDOne, kIndexZero, sizeof(long), &myValue, NULL);
	if (myErr != noErr)
		goto bail;
	
	myValue = EndianS32_NtoB(mySelEnd2);
	myErr = QTInsertChild(*theActions, myHyperTextAtom, kRangeEnd, kIDOne, kIndexZero, sizeof(long), &myValue, NULL);
	if (myErr != noErr)
		goto bail;
	
	// add an event atom to the hypertext atom;
	// the event type can be any of the five mouse events: down, up, trigger, enter, exit
	myErr = QTInsertChild(*theActions, myHyperTextAtom, kQTEventType, kQTEventMouseClick, kIndexZero, kZeroDataLength, NULL, &myEventAtom);
	if (myErr != noErr)
		goto bail;
	
	myErr = QTInsertChild(*theActions, myEventAtom, kAction, kIndexOne, kIndexOne, kZeroDataLength, NULL, &myActionAtom);
	if (myErr != noErr)
		goto bail;
	
	myAction = EndianS32_NtoB(kActionGoToURL);
	myErr = QTInsertChild(*theActions, myActionAtom, kWhichAction, kIndexOne, kIndexOne, sizeof(long), &myAction, NULL);
	if (myErr != noErr)
		goto bail;

	myErr = QTInsertChild(*theActions, myActionAtom, kActionParameter, kIndexOne, kIndexOne, myCnnURL[0] + 1, &myCnnURL[1], NULL);
	if (myErr != noErr)
		goto bail;

bail:
	return(myErr);
}	


//////////
//
// AddHTAct_AddHyperActionsToSample
// Add the specified atom container to the end of the specified media sample.
//
// Hypertext actions are stored at the end of the sample as a normal text atom extension.
// In this case, the text atom type is kHyperTextTextAtomType and the data is the container data.
//
//////////

static OSErr AddHTAct_AddHyperActionsToSample (Handle theSample, QTAtomContainer theActions)
{
	Ptr			myPtr = NULL;
	long		myHandleLength;
	long		myContainerLength;
	long		myNewLength;		
	OSErr		myErr = noErr;

	myHandleLength = GetHandleSize(theSample);
	myContainerLength = GetHandleSize((Handle)theActions);
	
	myNewLength = (long)(sizeof(long) + sizeof(OSType) + myContainerLength);
			
	SetHandleSize(theSample, (myHandleLength + myNewLength));
	myErr = MemError();		
	if (myErr != noErr)
		goto bail;
	
	HLock(theSample);
	
	// get a pointer to the beginning of the new block of space added to the sample
	// by the previous call to SetHandleSize; we need to format that space as a text
	// atom extension
	myPtr = *theSample + myHandleLength;
	
	// set the length of the text atom extension
	*(long *)myPtr = EndianS32_NtoB((long)(sizeof(long) + sizeof(OSType) + myContainerLength));
	myPtr += (sizeof(long));
	
	// set the type of the text atom extension
	*(OSType *)myPtr = EndianS32_NtoB(kHyperTextTextAtomType);
	myPtr += (sizeof(OSType));
	
	// set the data of the text atom extension;
	// we assume that this data is already in big-endian format
	HLock((Handle)theActions);
	BlockMove(*theActions, myPtr, myContainerLength);
	
	HUnlock((Handle)theActions);
	HUnlock(theSample);

bail:
	return(myErr);
}


//////////
//
// AddHTAct_AddHyperTextToTextMovie
// Add some hypertext actions to the specified text movie.
//
//////////

static OSErr AddHTAct_AddHyperTextToTextMovie (FSSpec *theFSSpec)
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
	TextDescriptionHandle			myTextDesc = NULL;
	Handle							mySample = NULL;
	short							mySampleFlags;
	Fixed 							myTrackEditRate;
	QTAtomContainer					myActions = NULL;	
	OSErr							myErr = noErr;

	//////////
	//
	// open the movie file and get the first text track from the movie
	//
	//////////
	
	// open the movie file for reading and writing
	myErr = OpenMovieFile(theFSSpec, &myResRefNum, fsRdWrPerm);
	if (myErr != noErr)
		goto bail;

	myErr = NewMovieFromFile(&myMovie, myResRefNum, &myResID, NULL, newMovieActive, NULL);
	if (myErr != noErr)
		goto bail;
		
	// find first text track in the movie
	myTrack = GetMovieIndTrackType(myMovie, kIndexOne, TextMediaType, movieTrackMediaType);
	if (myTrack == NULL)
		goto bail;
	
	//////////
	//
	// get first media sample in the text track
	//
	//////////
	
	myMedia = GetTrackMedia(myTrack);
	if (myMedia == NULL)
		goto bail;
	
	myTrackOffset = GetTrackOffset(myTrack);
	myMediaTime = TrackTimeToMediaTime(myTrackOffset, myTrack);

	// allocate some storage to hold the sample description for the text track
	myTextDesc = (TextDescriptionHandle)NewHandle(4);
	if (myTextDesc == NULL)
		goto bail;

	mySample = NewHandle(0);
	if (mySample == NULL)
		goto bail;

	myErr = GetMediaSample(myMedia, mySample, 0, NULL, myMediaTime, NULL, &mySampleDuration, (SampleDescriptionHandle)myTextDesc, NULL, 1, NULL, &mySampleFlags);
	if (myErr != noErr)
		goto bail;

	//////////
	//
	// add hypertext actions to the first media sample
	//
	//////////
	
	// create an action container for hypertext actions
	myErr = AddHTAct_CreateHyperTextActionContainer(&myActions);
	if (myErr != noErr)
		goto bail;
	
	// add hypertext actions actions to sample
	myErr = AddHTAct_AddHyperActionsToSample(mySample, myActions);
	if (myErr != noErr)
		goto bail;
		
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
							(SampleDescriptionHandle)myTextDesc, 
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
	// update the movie resource
	//
	//////////
	
	myErr = UpdateMovieResource(myMovie, myResRefNum, myResID, NULL);
	
bail:
	// close the movie file
	if (myResRefNum != -1)
		CloseMovieFile(myResRefNum);
		
	if (myActions != NULL)
		(void)QTDisposeAtomContainer(myActions);
	
	if (mySample != NULL)
		DisposeHandle(mySample);		
	
	if (myTextDesc != NULL)
		DisposeHandle((Handle)myTextDesc);		
	
	if (myMovie != NULL)
		DisposeMovie(myMovie);	

	return(myErr);
}

//////////
//
// AddHTAct_PutFile
// Save a file under the specified name. Return Boolean values indicating whether the user selected a file
// and whether the selected file is replacing an existing file.
//
//////////

OSErr AddHTAct_PutFile (ConstStr255Param thePrompt, ConstStr255Param theFileName, FSSpecPtr theFSSpecPtr, Boolean *theIsSelected, Boolean *theIsReplacing)
{
#if TARGET_OS_WIN32
	StandardFileReply	myReply;
#endif
#if TARGET_OS_MAC
	NavReplyRecord		myReply;
	NavDialogOptions	myDialogOptions;
#endif
	OSErr				myErr = noErr;

	if ((theFSSpecPtr == NULL) || (theIsSelected == NULL) || (theIsReplacing == NULL))
		return(paramErr);
	
	// assume we are not replacing an existing file
	*theIsReplacing = false;
	
#if TARGET_OS_WIN32
	StandardPutFile(thePrompt, theFileName, &myReply);
	*theFSSpecPtr = myReply.sfFile;
	*theIsSelected = myReply.sfGood;
	if (myReply.sfGood)
		*theIsReplacing = myReply.sfReplacing;
#endif

#if TARGET_OS_MAC
	// specify the options for the dialog box
	NavGetDefaultDialogOptions(&myDialogOptions);
	myDialogOptions.dialogOptionFlags += kNavNoTypePopup;
	myDialogOptions.dialogOptionFlags += kNavDontAutoTranslate;
	BlockMoveData(theFileName, myDialogOptions.savedFileName, theFileName[0] + 1);
	BlockMoveData(gAppName, myDialogOptions.clientName, gAppName[0] + 1);
	BlockMoveData(thePrompt, myDialogOptions.message, thePrompt[0] + 1);
	
	// prompt the user for a file
	myErr = NavPutFile(NULL, &myReply, &myDialogOptions, NULL, MovieFileType, FOUR_CHAR_CODE('TVOD'), NULL);
	if ((myErr == noErr) && myReply.validRecord) {
		AEKeyword		myKeyword;
		DescType		myActualType;
		Size			myActualSize = 0;
		
		// get the FSSpec for the selected file
		if (theFSSpecPtr != NULL)
			myErr = AEGetNthPtr(&(myReply.selection), 1, typeFSS, &myKeyword, &myActualType, theFSSpecPtr, sizeof(FSSpec), &myActualSize);
	}
		
	*theIsSelected = myReply.validRecord;
	if (myReply.validRecord)
		*theIsReplacing = myReply.replacing;
		
	NavDisposeReply(&myReply);
#endif

	return(myErr);
}


//////////
//
// AddHTAct_ConvertCToPascalString
// Convert a C string into a Pascal string.
//
// The caller is responsible for disposing of the pointer returned by this function (by calling free).
//
//////////

StringPtr AddHTAct_ConvertCToPascalString (char *theString)
{
	StringPtr	myString = malloc(strlen(theString) + 1);
	short		myIndex = 0;

	while (theString[myIndex] != '\0') {
		myString[myIndex + 1] = theString[myIndex];
		myIndex++;
	}
	
	myString[0] = (unsigned char)myIndex;
	
	return(myString);
}



