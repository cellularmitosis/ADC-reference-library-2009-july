//////////
//
//	File:		QTStreamSplicer.c
//
//	Contains:	Code to splice a still frame onto a streamed movie.
//
//	Written by:	Dan Crow
//	Revised by: Tim Monroe
//
//	Copyright:	� 1999 by Apple Computer, Inc., all rights reserved.
//
//	Change History (most recent first):
//
//	   <6>	 	07/18/00	rtm		fixed bug in QTSplicer_SpliceImageOverStream
//	   <5>	 	03/20/00	rtm		made changes to get things running under CarbonLib
//	   <4>	 	06/02/99	rtm		further work on QTSplicer_SpliceImageOverStream
//	   <3>	 	05/28/99	rtm		added QTSplicer_SpliceImageOverStream, for sound-only streams
//	   <2>	 	05/19/99	rtm		minor tweaking for Windows version; added some comments; added ability
//									to save spliced file as self-contained
//	   <1>	 	05/18/99	rtm		first file from Dan Crow; modified to better cohere with QT SDK
//									standards (did some renaming of variables and functions, but otherwise
//									this is pretty much Dan's code)
//
//	This file defines two principal functions, QTSplicer_SpliceImageOntoStream and QTSplicer_SpliceImageOverStream.
//	QTSplicer_SpliceImageOntoStream prompts the user for a picture and a streamed movie, which is assumed to have
//	a video track. QTSplicer_SpliceImageOntoStream is designed to splice the selected picture onto the beginning of
//	the streamed movie, so that that picture appears in place of the standard QuickTime pre-preroll sequence, for a
//	user-specified duration.
//
//	QTSplicer_SpliceImageOverStream has a slightly different purpose. It prompts the user for a picture and a streamed
//	movie, which is assumed to have no video track. QTSplicer_SpliceImageOverStream is designed to splice the picture
//	over the streamed track for the entire duration of the streamed movie. You might want to do this to add a graphic
//	to a streamed radio broadcast or to a streamed music selection.
//
//////////

#include "QTStreamSplicer.h"


//////////
//
// QTSplicer_SpliceImageOntoStream
//
// Prompt the user for an image file and a streamed movie; then splice the image
// onto the front of the stream, for a specific duration; save the new spliced movie.
//
// This function contains the user interface for getting the input image and movie,
// the output spliced movie location, and the desired splicing settings. The function
// QTSplicer_CreateSplicedOntoMovie does all the nitty-gritty movie creation stuff.
//
//////////

OSErr QTSplicer_SpliceImageOntoStream (void)
{
	FSSpec					myImageSpec;
	FSSpec					myStreamSpec;
	FSSpec					myMovieSpec;
	Movie					myMovie = NULL;
	Boolean					isScaleImage = false;
	Boolean					isSelfContained = false;
	Boolean					gotImage = false;
	Boolean					gotStream = false;
	short					myResID = movieInDataForkResID;
	short					myRefNum = 0;
	DialogPtr				myDialog = NULL;
	DialogItemIndex			myItem = 0;
	DialogItemType			myItemType;
	Handle					myItemHandle = NULL;
	Rect					myItemRect;
	long					myImageDuration;
	Str255					myDurationString;
	FSSpec					myFSSpec;
	OSType 					myTypeList[] = {kQTFileTypeMovie};
	short					myNumTypes = 1;
	QTFrameFileFilterUPP	myFileFilterUPP = NULL;
	OSErr					myErr = noErr;

#if TARGET_OS_MAC
	myNumTypes = 0;
#endif

	myFileFilterUPP = QTFrame_GetFileFilterUPP((ProcPtr)QTFrame_FilterFiles);
	
	// prompt the user for the image file to splice onto the stream
	do {
		myErr = QTFrame_GetOneFileWithPreview(myNumTypes, (QTFrameTypeListPtr)myTypeList, &myFSSpec, myFileFilterUPP);
		if (myErr == noErr) {
			if (QTUtils_IsImageFile(&myFSSpec)) {
				myImageSpec = myFSSpec;
				gotImage = true;
			}
		} else {
			break;		// if the user cancelled
		}					
	} while (!gotImage);
	
	// prompt the user for the live stream file to splice onto the end of the created movie
	do {
		myErr = QTFrame_GetOneFileWithPreview(myNumTypes, (QTFrameTypeListPtr)myTypeList, &myFSSpec, myFileFilterUPP);
		if (myErr == noErr) {
			if (QTSplicer_FileContainsStream(myFSSpec)) {
				myStreamSpec = myFSSpec;
				gotStream = true;
			}
		} else {
			break;		// if the user cancelled
		}
	} while (!gotStream);
	
	if (myFileFilterUPP != NULL)
		DisposeNavObjectFilterUPP(myFileFilterUPP);

	// make sure we have both an image and a stream
	if (!gotStream || !gotImage)
		goto bail;

	// get the desired splicing settings; open up the Splicer dialog box
	myDialog = GetNewDialog(kSpliceOntoDialogID, NULL, (WindowPtr)-1L);
	if (myDialog == NULL)
		goto bail;
		
	SetDialogDefaultItem(myDialog, kSpliceButtonDone);
	MacShowWindow(GetDialogWindow(myDialog));
		
	do {
		ModalDialog(NULL, &myItem);
		if ((myItem == kSpliceScaleCheckbox) || (myItem == kSelfContainedCheckbox)) {
			Boolean			myValue;
			
			GetDialogItem(myDialog, myItem, &myItemType, &myItemHandle, &myItemRect);
			myValue = (Boolean)GetControlValue((ControlHandle)myItemHandle);
			SetControlValue((ControlHandle)myItemHandle, !myValue);
		}
	} while (myItem != kSpliceButtonDone);
	
	// set the isScaleImage variable depending on the state of the kSpliceScaleCheckbox item
	GetDialogItem(myDialog, kSpliceScaleCheckbox, &myItemType, &myItemHandle, &myItemRect);
	isScaleImage = (Boolean)GetControlValue((ControlHandle)myItemHandle);
	
	// set the isSelfContained variable depending on the state of the kSelfContainedCheckbox item
	GetDialogItem(myDialog, kSelfContainedCheckbox, &myItemType, &myItemHandle, &myItemRect);
	isSelfContained = (Boolean)GetControlValue((ControlHandle)myItemHandle);
	
	// get the desired image duration from the kSpliceDuration item
	GetDialogItem(myDialog, kSpliceDuration, &myItemType, &myItemHandle, &myItemRect);
	GetDialogItemText(myItemHandle, myDurationString);
	StringToNum(myDurationString, &myImageDuration);
	
	DisposeDialog(myDialog);

	// prompt the user for the location of the output spliced movie
	{
		FSSpec				myFile;
		Boolean				myIsSelected = false;
		Boolean				myIsReplacing = false;	
		StringPtr 			myMoviePrompt = QTUtils_ConvertCToPascalString(kSaveSplicePrompt);
		StringPtr 			myMovieFileName = QTUtils_ConvertCToPascalString(kSaveSpliceFileName);

		QTFrame_PutFile(myMoviePrompt, myMovieFileName, &myFile, &myIsSelected, &myIsReplacing);
		if (!myIsSelected)
			goto bail;				// deal with user cancelling

		if (myIsReplacing)
			DeleteMovieFile(&myFile);

		myMovieSpec = myFile;

		free(myMoviePrompt);
		free(myMovieFileName);

		// create a movie file for the destination ("spliced") movie
		myErr = CreateMovieFile(&myMovieSpec, FOUR_CHAR_CODE('TVOD'), smCurrentScript, createMovieFileDeleteCurFile | createMovieFileDontCreateResFile, &myRefNum, &myMovie);
		if (myErr != noErr)
			goto bail;
	}
	
	// now create a movie that contains the image file and the stream spliced together
	myErr = QTSplicer_CreateSplicedOntoMovie(myImageSpec, myStreamSpec, myMovie, isScaleImage, isSelfContained, myImageDuration * kOneSecond);
	if (myErr != noErr)
		goto bail;
		
	// put the movie resource into the file
	myErr = AddMovieResource(myMovie, myRefNum, &myResID, NULL);
		
bail:
	if (myRefNum != 0)
		CloseMovieFile(myRefNum);
	
	if (myMovie != NULL)
		DisposeMovie(myMovie);
	
	return(myErr);
}


//////////
//
// QTSplicer_SpliceImageOverStream
//
// Prompt the user for an image file and a streamed movie, which is a sound-only stream;
// then splice the image over the stream, for the duration of the stream; save the new spliced movie.
//
// This function contains the user interface for getting the input image and movie,
// the output spliced movie location, and the desired splicing settings. The function
// QTSplicer_CreateSplicedOverMovie does all the nitty-gritty movie creation stuff.
//
//////////

OSErr QTSplicer_SpliceImageOverStream (void)
{
	FSSpec					myImageSpec;
	FSSpec					myStreamSpec;
	FSSpec					myMovieSpec;
	Movie					myMovie = NULL;
	Boolean					isScaleImage = false;
	Boolean					isSelfContained = false;
	Boolean					gotImage = false;
	Boolean					gotStream = false;
	short					myResID = movieInDataForkResID;
	short					myRefNum = 0;
	DialogPtr				myDialog = NULL;
	DialogItemIndex			myItem = 0;
	DialogItemType			myItemType;
	Handle					myItemHandle = NULL;
	Rect					myItemRect;
	long					myImageHeight;
	long					myImageWidth;
	Str255					myHeightString;
	Str255					myWidthString;
	FSSpec					myFSSpec;
	OSType 					myTypeList[] = {kQTFileTypeMovie};
	short					myNumTypes = 1;
	QTFrameFileFilterUPP	myFileFilterUPP = NULL;
	OSErr					myErr = noErr;

#if TARGET_OS_MAC
	myNumTypes = 0;
#endif

	myFileFilterUPP = QTFrame_GetFileFilterUPP((ProcPtr)QTFrame_FilterFiles);
	
	// prompt the user for the image file to splice onto the stream
	do {
		myErr = QTFrame_GetOneFileWithPreview(myNumTypes, (QTFrameTypeListPtr)myTypeList, &myFSSpec, myFileFilterUPP);
		if (myErr == noErr) {
			if (QTUtils_IsImageFile(&myFSSpec)) {
				myImageSpec = myFSSpec;
				gotImage = true;
			}
		} else {
			break;		// if the user cancelled
		}					
	} while (!gotImage);
	
	// prompt the user for the live stream file to splice under the image file
	do {
		myErr = QTFrame_GetOneFileWithPreview(myNumTypes, (QTFrameTypeListPtr)myTypeList, &myFSSpec, myFileFilterUPP);
		if (myErr == noErr) {
			if (QTSplicer_FileContainsStream(myFSSpec)) {
				myStreamSpec = myFSSpec;
				gotStream = true;
			}
		} else {
			break;		// if the user cancelled
		}					
	} while (!gotStream);
		
	// make sure we have both an image and a stream
	if (!gotStream || !gotImage)
		goto bail;

	// get the desired splicing settings; open up the Splicer dialog box
	myDialog = GetNewDialog(kSpliceOverDialogID, NULL, (WindowPtr)-1L);
	if (myDialog == NULL)
		goto bail;
		
	SetDialogDefaultItem(myDialog, kSpliceButtonDone);
	MacShowWindow(GetDialogWindow(myDialog));
		
	do {
		ModalDialog(NULL, &myItem);
		if ((myItem == kSpliceScaleCheckbox) || (myItem == kSelfContainedCheckbox)) {
			Boolean			myValue;
			
			GetDialogItem(myDialog, myItem, &myItemType, &myItemHandle, &myItemRect);
			myValue = (Boolean)GetControlValue((ControlHandle)myItemHandle);
			SetControlValue((ControlHandle)myItemHandle, !myValue);
		}
	} while (myItem != kSpliceButtonDone);
	
	// set the isScaleImage variable depending on the state of the kSpliceScaleCheckbox item
	GetDialogItem(myDialog, kSpliceScaleCheckbox, &myItemType, &myItemHandle, &myItemRect);
	isScaleImage = (Boolean)GetControlValue((ControlHandle)myItemHandle);
	
	// set the isSelfContained variable depending on the state of the kSelfContainedCheckbox item
	GetDialogItem(myDialog, kSelfContainedCheckbox, &myItemType, &myItemHandle, &myItemRect);
	isSelfContained = (Boolean)GetControlValue((ControlHandle)myItemHandle);
	
	if (isScaleImage) {
		// get the desired image width from the kSpliceWidth item
		GetDialogItem(myDialog, kSpliceWidth, &myItemType, &myItemHandle, &myItemRect);
		GetDialogItemText(myItemHandle, myWidthString);
		StringToNum(myWidthString, &myImageWidth);
		
		// get the desired image height from the kSpliceHeight item
		GetDialogItem(myDialog, kSpliceHeight, &myItemType, &myItemHandle, &myItemRect);
		GetDialogItemText(myItemHandle, myHeightString);
		StringToNum(myHeightString, &myImageHeight);
	}
	
	DisposeDialog(myDialog);

	// prompt the user for the location of the output spliced movie
	{
		FSSpec				myFile;
		Boolean				myIsSelected = false;
		Boolean				myIsReplacing = false;	
		StringPtr 			myMoviePrompt = QTUtils_ConvertCToPascalString(kSaveSplicePrompt);
		StringPtr 			myMovieFileName = QTUtils_ConvertCToPascalString(kSaveSpliceFileName);

		QTFrame_PutFile(myMoviePrompt, myMovieFileName, &myFile, &myIsSelected, &myIsReplacing);
		if (!myIsSelected)
			goto bail;				// deal with user cancelling

		if (myIsReplacing)
			DeleteMovieFile(&myFile);

		myMovieSpec = myFile;

		free(myMoviePrompt);
		free(myMovieFileName);

		// create a movie file for the destination ("spliced") movie
		myErr = CreateMovieFile(&myMovieSpec, FOUR_CHAR_CODE('TVOD'), smCurrentScript, createMovieFileDeleteCurFile | createMovieFileDontCreateResFile, &myRefNum, &myMovie);
		if (myErr != noErr)
			goto bail;
	}
	
	// now create a movie that contains the image file and the stream spliced together
	myErr = QTSplicer_CreateSplicedOverMovie(myImageSpec, myStreamSpec, myMovie, isScaleImage, isSelfContained, myImageHeight, myImageWidth);
	if (myErr != noErr)
		goto bail;
		
	// put the movie resource into the file
	myErr = AddMovieResource(myMovie, myRefNum, &myResID, NULL);
		
bail:
	if (myRefNum != 0)
		CloseMovieFile(myRefNum);
	
	if (myMovie != NULL)
		DisposeMovie(myMovie);
	
	return(myErr);
}


//////////
//
// QTSplicer_CreateSplicedOntoMovie
//
// Create a movie file that contains the specified movie, with the specified image
// spliced onto the front for the specified duration.
//
//////////

OSErr QTSplicer_CreateSplicedOntoMovie (FSSpec theImageSpec, FSSpec theMovieSpec, Movie theSplicedMovie, Boolean isScaleImage, Boolean isSelfContained, long theImageDuration)
{
	short						myRefNum = kInvalidFileRefNum;
	short						myResID = 0;
	Movie						myMovie = NULL;
	MovieController				mySplicedController = NULL;
	Rect						myStreamBox, myImageRect;
	GraphicsImportComponent		myImporter = NULL;
	GWorldPtr					myImageGWorld = NULL;
	ComponentResult				myErr = noErr;
				
	// open the streamed movie file
	myErr = OpenMovieFile(&theMovieSpec, &myRefNum, fsRdPerm);
	if (myErr != noErr)
		goto bail;

	myErr = NewMovieFromFile(&myMovie, myRefNum, &myResID, NULL, newMovieActive, NULL);
	if (myErr != noErr)
		goto bail;
	
	// find out the size of the streamed movie
	GetMovieBox(myMovie, &myStreamBox);
	
	// get a graphics importer for the image file
	myErr = GetGraphicsImporterForFile(&theImageSpec, &myImporter);
	if (myErr != noErr)
		goto bail;
				
	// get the size of the image
	if (isScaleImage) {
		myImageRect = myStreamBox;
		MacOffsetRect(&myImageRect, -myStreamBox.left, -myStreamBox.top);
	} else {
		GraphicsImportGetBoundsRect(myImporter, &myImageRect);
	}

	// allocate a new GWorld to draw the image into
	myErr = QTNewGWorld(&myImageGWorld, 32, &myImageRect, NULL, NULL, kICMTempThenAppMemory);
	if (myErr != noErr) {
		if (myImporter != NULL)
			CloseComponent(myImporter);
		goto bail;
	}
	
	// draw the picture into the GWorld
	GraphicsImportSetGWorld(myImporter, myImageGWorld, NULL);
	GraphicsImportSetBoundsRect(myImporter, &myImageRect);
	GraphicsImportDraw(myImporter);
	
	// close the graphics importer component
	if (myImporter != NULL)
		CloseComponent(myImporter);
	
	// create a controller for the spliced movie
	mySplicedController = NewMovieController(theSplicedMovie, &myStreamBox, 0);
	
	// make sure we can edit this movie
	MCEnableEditing(mySplicedController, true);
		
	// paste the still image into the spliced movie as a new video track
	myErr = QTSplicer_AddVideoTrackFromGWorld(&theSplicedMovie, myImageGWorld, myImageRect.right - myImageRect.left, myImageRect.bottom - myImageRect.top, theImageDuration);
	if (myErr != noErr)
		goto bail;

	// go to the end of the spliced movie; this jumps past the first track,
	// which we have just pasted into the spliced movie.
	QTSplicer_SetCurrentTime(mySplicedController, GetMovieDuration(theSplicedMovie));

	if (isSelfContained) {
		Track			mySrcTrack = NULL;
		Track			myDstTrack = NULL;
		Media			myDstMedia = NULL;
		Fixed			myWidth, myHeight;
		
		mySrcTrack = GetMovieIndTrackType(myMovie, 1, kQTSStreamMediaType, movieTrackMediaType);
		if (mySrcTrack == NULL)
			goto bail;
		
		GetTrackDimensions(mySrcTrack, &myWidth, &myHeight);
		
		myDstTrack = NewMovieTrack(theSplicedMovie, myWidth, myHeight, kNoVolume);
		if (myDstTrack == NULL)
			goto bail;
		
		myDstMedia = NewTrackMedia(myDstTrack, kQTSStreamMediaType, GetMovieTimeScale(theSplicedMovie), NULL, 0);
		if (myDstTrack == NULL)
			goto bail;
		
		myErr = BeginMediaEdits(myDstMedia);
		if (myErr != noErr)
			goto bail;
		
		myErr = CopyTrackSettings(mySrcTrack, myDstTrack);
		if (myErr != noErr)
			goto bail;
		
		myErr = InsertTrackSegment(mySrcTrack, myDstTrack, 0, (0x7FFFFFFF - GetMovieDuration(theSplicedMovie)), GetMovieDuration(theSplicedMovie));
		if (myErr != noErr)
			goto bail;
		
		myErr = EndMediaEdits(myDstMedia);
		if (myErr != noErr)
			goto bail;
	} else {
		// insert the streamed movie after the image track
		myErr = InsertMovieSegment(myMovie, theSplicedMovie, 0, (0x7FFFFFFF - GetMovieDuration(theSplicedMovie)), GetMovieDuration(theSplicedMovie));
		if (myErr != noErr)
			goto bail;
	}
	
	// select none on the resulting movie
	QTSplicer_SetSelectionTimes(mySplicedController, 0, 0);
	
	// go to the start of the spliced movie
	QTSplicer_SetCurrentTime(mySplicedController, 0);
	
bail:
	if (myRefNum != 0)
		CloseMovieFile(myRefNum);

	if (myImageGWorld != NULL) {
		DisposeGWorld(myImageGWorld);
		myImageGWorld = NULL;
	}
	
	if (mySplicedController != NULL) {
		DisposeMovieController(mySplicedController);
	}
	
	return(myErr);
}


//////////
//
// QTSplicer_CreateSplicedOverMovie
//
// Create a movie file that contains the specified movie, with the specified image as the video track.
//
//////////

OSErr QTSplicer_CreateSplicedOverMovie (FSSpec theImageSpec, FSSpec theMovieSpec, Movie theSplicedMovie, Boolean isScaleImage, Boolean isSelfContained, long theImageHeight, long theImageWidth)
{
	short						myRefNum = kInvalidFileRefNum;
	short						myResID = 0;
	Movie						myMovie = NULL;
	MovieController				mySplicedController = NULL;
	Rect						myImageRect;
	GraphicsImportComponent		myImporter = NULL;
	GWorldPtr					myImageGWorld = NULL;
	Track						mySrcTrack = NULL;
	Track						myDstTrack = NULL;
	Media						myDstMedia = NULL;
	Fixed						myWidth, myHeight;
	ComponentResult				myErr = noErr;
				
	// open the streamed movie file
	myErr = OpenMovieFile(&theMovieSpec, &myRefNum, fsRdPerm);
	if (myErr != noErr)
		goto bail;

	myErr = NewMovieFromFile(&myMovie, myRefNum, &myResID, NULL, newMovieActive, NULL);
	if (myErr != noErr)
		goto bail;
	
	// get a graphics importer for the image file
	myErr = GetGraphicsImporterForFile(&theImageSpec, &myImporter);
	if (myErr != noErr)
		goto bail;
				
	// get the size of the image
	if (isScaleImage) {
		myImageRect.top = 0;
		myImageRect.left = 0;
		myImageRect.bottom = (short)theImageHeight;
		myImageRect.right = (short)theImageWidth;
	} else {
		GraphicsImportGetBoundsRect(myImporter, &myImageRect);
	}

	// allocate a new GWorld to draw the image into
	myErr = QTNewGWorld(&myImageGWorld, 32, &myImageRect, NULL, NULL, kICMTempThenAppMemory);
	if (myErr != noErr) {
		if (myImporter != NULL)
			CloseComponent(myImporter);
		goto bail;
	}
	
	// draw the picture into the GWorld
	GraphicsImportSetGWorld(myImporter, myImageGWorld, NULL);
	GraphicsImportSetBoundsRect(myImporter, &myImageRect);
	GraphicsImportDraw(myImporter);
	
	// close the graphics importer component
	if (myImporter != NULL)
		CloseComponent(myImporter);
	
	// create a controller for the spliced movie
	mySplicedController = NewMovieController(theSplicedMovie, &myImageRect, 0);
	
	// make sure we can edit this movie
	MCEnableEditing(mySplicedController, true);
		
	// paste the still image into the spliced movie as a new video track
	myErr = QTSplicer_AddVideoTrackFromGWorld(&theSplicedMovie, myImageGWorld, myImageRect.right - myImageRect.left, myImageRect.bottom - myImageRect.top, GetMovieDuration(myMovie));
	if (myErr != noErr)
		goto bail;
			
	//////////
	//
	// copy over the streamed track
	//
	//////////
	
	mySrcTrack = GetMovieIndTrackType(myMovie, 1, kQTSStreamMediaType, movieTrackMediaType);
	if (mySrcTrack == NULL)
		goto bail;
	
	GetTrackDimensions(mySrcTrack, &myWidth, &myHeight);
	
	myDstTrack = NewMovieTrack(theSplicedMovie, myWidth, myHeight, kNoVolume);
	if (myDstTrack == NULL)
		goto bail;
	
	myDstMedia = NewTrackMedia(myDstTrack, kQTSStreamMediaType, GetMovieTimeScale(theSplicedMovie), NULL, 0);
	if (myDstTrack == NULL)
		goto bail;
	
	if (isSelfContained)
		BeginMediaEdits(myDstMedia);
	
	myErr = CopyTrackSettings(mySrcTrack, myDstTrack);
	if (myErr != noErr)
		goto bail;
	
	myErr = InsertTrackSegment(mySrcTrack, myDstTrack, 0, GetMovieDuration(myMovie), 0);
	if (myErr != noErr)
		goto bail;
	
	if (isSelfContained)
		EndMediaEdits(myDstMedia);
	
	// select none on the resulting movie
	QTSplicer_SetSelectionTimes(mySplicedController, 0, 0);
	
	// go to the start of the spliced movie
	QTSplicer_SetCurrentTime(mySplicedController, 0);
	
bail:
	if (myRefNum != 0)
		CloseMovieFile(myRefNum);

	if (myImageGWorld != NULL) {
		DisposeGWorld(myImageGWorld);
		myImageGWorld = NULL;
	}
	
	if (mySplicedController != NULL) {
		DisposeMovieController(mySplicedController);
	}
	
	return(myErr);
}


//////////
//
// QTSplicer_FileContainsStream
//
// Determine whether the specified movie file contains a streaming track.
//
// Returns true iff the file is a movie file that has at least one streaming
// track in it, false otherwise.
//
//////////

Boolean QTSplicer_FileContainsStream (FSSpec theMovieFile)
{
	short		myRefNum = 0;
	short		myResID = 0;
	Movie		myMovie = NULL;
	Boolean		myResult = false;
	OSErr		myErr = noErr;

	myErr = OpenMovieFile(&theMovieFile, &myRefNum, fsRdPerm);
	if (myErr != noErr)
		goto bail;
		
	myErr = NewMovieFromFile(&myMovie, myRefNum, &myResID, NULL, newMovieActive, NULL);
	if (myErr != noErr)
		goto bail;
	
	if (GetMovieIndTrackType(myMovie, 1, kQTSStreamMediaType, movieTrackMediaType) != NULL)
		myResult = true;
	
bail:
	if (myRefNum != 0)
		CloseMovieFile(myRefNum);

	return(myResult);
}


//////////
//
// QTSplicer_SetSelectionTimes
//
// Set the movie selection to the specified times.
//
//////////

OSErr QTSplicer_SetSelectionTimes (MovieController theController, TimeValue theTime1, TimeValue theTime2)
{
	TimeRecord 			myTimeRecord;
	Movie				myMovie = MCGetMovie(theController);
	ComponentResult		myErr = noErr;
	
	myTimeRecord.value.hi = 0;
	myTimeRecord.value.lo = theTime1;
	myTimeRecord.base = 0;
	myTimeRecord.scale = GetMovieTimeScale(myMovie);
	myErr = MCDoAction(theController, mcActionSetSelectionBegin, &myTimeRecord);
	if (myErr != noErr)
		goto done;
	
	myTimeRecord.value.lo = theTime2 - theTime1;

	myErr = MCDoAction(theController, mcActionSetSelectionDuration, &myTimeRecord);
	
done:
	return((OSErr)myErr);
}


//////////
//
// QTSplicer_SetCurrentTime
//
// Set the movie current time to the specified time.
//
//////////

OSErr QTSplicer_SetCurrentTime (MovieController theController, TimeValue theTime)
{
	TimeRecord 			myTimeRecord;
	Movie				myMovie = MCGetMovie(theController);
	ComponentResult		myErr = noErr;
	
	myTimeRecord.value.hi = 0;
	myTimeRecord.value.lo = theTime;
	myTimeRecord.base = GetMovieTimeBase(myMovie);
	myTimeRecord.scale = GetMovieTimeScale(myMovie);
	myErr = MCDoAction(theController, mcActionGoToTime, &myTimeRecord);

	return((OSErr)myErr);
}


//////////
//
// QTSplicer_AddVideoTrackFromGWorld
//
// Add to the specified movie a video track, using the image in the specified GWorld.
//
//////////

OSErr QTSplicer_AddVideoTrackFromGWorld (Movie *theMovie, GWorldPtr theGW, short theWidth, short theHeight, long theImageDuration)
{
	Track						myTrack = NULL;
	Media						myMedia = NULL;
	ImageDescriptionHandle		myDesc = NULL;
	Rect						myRect;
	long						mySize;
	Handle						myData = NULL;
	Ptr							myDataPtr = NULL;
	PixMapHandle				myDstPixMap = NULL;
	OSErr						myErr = noErr;
	
	// create a video track in the movie
	myTrack = NewMovieTrack(*theMovie, FixRatio(theWidth, 1), FixRatio(theHeight, 1), kNoVolume);
	myMedia = NewTrackMedia(myTrack, VideoMediaType, kOneSecond, NULL, 0);
	
	myRect.top = 0;
	myRect.left = 0;
	myRect.right = theWidth;
	myRect.bottom = theHeight;
	
	// begin editing the new track
	BeginMediaEdits(myMedia);

	// create a new image description; CompressImage will fill in the fields of this structure
	myDesc = (ImageDescriptionHandle)NewHandle(4);
	
	myDstPixMap = GetGWorldPixMap(theGW);
	LockPixels(myDstPixMap);
	
	myErr = GetMaxCompressionSize(myDstPixMap, &myRect, 0, codecNormalQuality, kAnimationCodecType, anyCodec, &mySize);
	if (myErr != noErr)
		goto bail;
		
	myData = NewHandle(mySize);
	if (myData == NULL)
		goto bail;
		
	HLockHi(myData);
#if TARGET_CPU_68K
	myDataPtr = StripAddress(*myData);
#else
	myDataPtr = *myData;
#endif
	
	myErr = CompressImage(myDstPixMap, &myRect, codecNormalQuality, kAnimationCodecType, myDesc, myDataPtr);
	if (myErr != noErr)
		goto bail;
		
	myErr = AddMediaSample(myMedia, myData, 0, (**myDesc).dataSize, theImageDuration, (SampleDescriptionHandle)myDesc, 1, 0, NULL);
	if (myErr != noErr)
		goto bail;

	myErr = EndMediaEdits(myMedia);
	if (myErr != noErr)
		goto bail;

	myErr = InsertMediaIntoTrack(myTrack, 0, 0, GetMediaDuration(myMedia), fixed1);
	
bail:
	if (myData != NULL) {
		HUnlock(myData);
		DisposeHandle(myData);
	}

	if (myDesc != NULL)
		DisposeHandle((Handle)myDesc);
		
	if (myDstPixMap != NULL)
		UnlockPixels(myDstPixMap);
	
	return(myErr);
}
