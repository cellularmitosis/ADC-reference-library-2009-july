//////////
//
//	File:		QTMovieTrack.c
//
//	Contains:	Sample code for working with QuickTime movie tracks.
//
//	Written by:	Tim Monroe
//
//	Copyright:	© 2000 by Apple Computer, Inc., all rights reserved.
//
//	Change History (most recent first):
//	   
//	   <4>	 	05/11/00	rtm		made a few UI tweaks
//	   <3>	 	05/08/00	rtm		added child rectangle previewing button
//	   <2>	 	05/06/00	rtm		added user interface for specifying options
//	   <1>	 	05/04/00	rtm		first file
//
//
//	This sample code shows how to add a "movie track" to an existing QuickTime movie; a movie track
//	is a track that specifies another movie that is to be embedded into the existing movie. The existing
//	movie will be called the "parent movie" or the "containing movie"; the embedded movie will be called
//	the "child movie", the "embedded movie", or the "movie-in-movie".
//
//	A movie track is added using standard Movie Toolbox routines. The sample description is generic. The
//	media sample data consists of an atom container that contains one or more child atoms. The child atom
//	of type kMovieMediaDataReference is required. All other child atoms are optional. The movie media
//	handler will use intelligent default values for any missing atoms.
//
//////////


//////////
//
// header files
//
//////////

#include "QTMovieTrack.h"


//////////
//
// global variables
//
//////////

extern short 				gAppResFile;						// file reference number for this application's resource file
extern ModalFilterUPP		gModalFilterUPP;					// UPP to our custom dialog event filter

RGBColor					gWhite = {0xffff, 0xffff, 0xffff};
RGBColor					gDGray = {0x7777, 0x7777, 0x7777};
RGBColor					gBlack = {0x0000, 0x0000, 0x0000};


//////////
//
// QTMIM_AddFileAsMovieTrack
// Elicit a movie file from the user and add it to a parent movie as a child movie.
//
//////////

OSErr QTMIM_AddFileAsMovieTrack (WindowObject theWindowObject)
{
	FSSpec					myFSSpec;
	OSType 					myTypeList[] = {kQTFileTypeMovie};
	short					myNumTypes = 1;
	QTFrameFileFilterUPP	myFileFilterUPP = NULL;
	AliasHandle				myAlias = NULL;
	OSErr					myErr = noErr;

#if TARGET_OS_MAC
	myNumTypes = 0;
#endif
	// elicit a movie file from the user
	myFileFilterUPP = QTFrame_GetFileFilterUPP((ProcPtr)QTMIM_FilterFiles);

	myErr = QTFrame_GetOneFileWithPreview(myNumTypes, (QTFrameTypeListPtr)myTypeList, &myFSSpec, myFileFilterUPP);
	if (myErr != noErr)
		goto bail;

	// create an alias to the selected file
	myErr = NewAliasMinimal(&myFSSpec, &myAlias);
	if (myErr != noErr)
		goto bail;

	// add the movie file as a movie track to the parent movie
	myErr = QTMIM_AddMovieTrack(theWindowObject, rAliasType, (Handle)myAlias);

bail:
	if (myFileFilterUPP != NULL)
		DisposeNavObjectFilterUPP(myFileFilterUPP);

	if (myAlias != NULL)
		DisposeHandle((Handle)myAlias);

	return(myErr);
}


//////////
//
// QTMIM_AddURLAsMovieTrack
// Elicit a URL from the user and add it to a parent movie as a child movie.
//
//////////

OSErr QTMIM_AddURLAsMovieTrack (WindowObject theWindowObject)
{
	char			*myURL = NULL;
	Handle			myHandle = NULL;
	Size			mySize = 0;
	OSErr			myErr = noErr;

	// elicit a URL from the user
	myURL = QTMIM_GetURLFromUser();
	if (myURL == NULL)
		goto bail;
		
	// get the size of the URL, plus the terminating null byte
	mySize = (Size)strlen(myURL) + 1;
	if (mySize == 0)
		goto bail;
	
	// allocate a new handle
	myHandle = NewHandleClear(mySize);
    if (myHandle == NULL)
		goto bail;

	// copy the URL into the handle
	BlockMove(myURL, *myHandle, mySize);

	// add the URL as a movie track to the parent movie
	myErr = QTMIM_AddMovieTrack(theWindowObject, URLDataHandlerSubType, myHandle);

bail:
	free(myURL);
	
	if (myHandle != NULL)
		DisposeHandle(myHandle);

	return(myErr);
}


//////////
//
// QTMIM_AddMovieTrack
// Add, to the specified parent movie, a movie track for the movie having the specified data reference.
//
//////////

OSErr QTMIM_AddMovieTrack (WindowObject theWindowObject, OSType theDataRefType, Handle theDataRef)
{
	Movie			 		myMovie = NULL;			// the parent movie
	Track					myTrack = NULL;			// the movie track
	Media					myMedia = NULL;			// the movie track's media
	OSErr					myErr = paramErr;
	
	if ((theWindowObject == NULL) || (theDataRef == NULL))
		goto bail;
		
	myMovie = (**theWindowObject).fMovie;
		
	// create the movie track and media
	myTrack = NewMovieTrack(myMovie, FixRatio(kChildMovieWidth, 1), FixRatio(kChildMovieHeight, 1), kFullVolume);
	myErr = GetMoviesError();
	if (myErr != noErr)
		goto bail;
		
	myMedia = NewTrackMedia(myTrack, MovieMediaType, kMovieTimeScale, NULL, 0);
	myErr = GetMoviesError();
	if (myErr != noErr)
		goto bail;

	// create the media sample(s)
	myErr = BeginMediaEdits(myMedia);
	if (myErr != noErr)
		goto bail;

	myErr = QTMIM_AddMovieTrackSampleToMedia(theWindowObject, myMedia, theDataRefType, theDataRef);
	if (myErr != noErr)
		goto bail;

	myErr = EndMediaEdits(myMedia);
	if (myErr != noErr)
		goto bail;
	
	// add the media to the track
	myErr = InsertMediaIntoTrack(myTrack, 0, 0, GetMediaDuration(myMedia), fixed1);

bail:
	return(myErr);
}


//////////
//
// QTMIM_AddMovieTrackSampleToMedia
// Add a sample to the specified media.
//
//////////

OSErr QTMIM_AddMovieTrackSampleToMedia (WindowObject theWindowObject, Media theMedia, OSType theDataRefType, Handle theDataRef)
{
	ApplicationDataHdl			myAppData = NULL;
	QTAtomContainer				mySample = NULL;
	QTAtom						myRegionAtom;
	SampleDescriptionHandle		myImageDesc = NULL;
	Ptr							myData = NULL;
	OSType						myType;
	Boolean						myBoolean;
	UInt8						myLoop;
	UInt32						myValue;
	OSErr						myErr = paramErr;
	
	//////////
	//
	// retrieve the application data; it determines what kind of atoms we create
	//
	//////////
	
	myAppData = (ApplicationDataHdl)QTFrame_GetAppDataFromWindowObject(theWindowObject);
	if (myAppData == NULL)
		return(myErr);

	//////////
	//
	// create a new atom container and add an atom of type kMovieMediaDataReference
	//
	//////////

	// create a new atom container to hold the sample data
	myErr = QTNewAtomContainer(&mySample);
	if (myErr != noErr)
		goto bail;

	// concatenate the specified data reference type and data reference into a single block of data
	myData = NewPtrClear(sizeof(OSType) + GetHandleSize(theDataRef));
	if (myData == NULL)
		goto bail;
	
	// convert the data to big-endian format
	myType = EndianU32_NtoB(theDataRefType);
	
	BlockMove(&myType, myData, sizeof(OSType));
	BlockMove(*theDataRef, myData + sizeof(OSType), GetHandleSize(theDataRef));
	
	// add an atom of type kMovieMediaDataReference to the atom container
	myErr = QTInsertChild(mySample, kParentAtomIsContainer, kMovieMediaDataReference, 1, 1, GetPtrSize(myData), myData, NULL);
	if (myErr != noErr)
		goto bail;

	//////////
	//
	// add an auto-start atom
	//
	//////////

	myBoolean = (**myAppData).fAutoPlayChild;
	myErr = QTInsertChild(mySample, kParentAtomIsContainer, kMovieMediaAutoPlay, 1, 1, sizeof(myBoolean), &myBoolean, NULL);
	if (myErr != noErr)
		goto bail;
	
	//////////
	//
	// add a frame-stepping atom
	//
	//////////

	myBoolean = (**myAppData).fFrameStepChild;
	myErr = QTInsertChild(mySample, kParentAtomIsContainer, kMovieMediaEnableFrameStepping, 1, 1, sizeof(myBoolean), &myBoolean, NULL);
	if (myErr != noErr)
		goto bail;
	
	//////////
	//
	// add a looping atom, if requested
	//
	//////////

	if ((**myAppData).fLoopingOn) {
		// add an atom of type kMovieMediaLoop to the atom container
		myLoop = (**myAppData).fLoopingState;
		myErr = QTInsertChild(mySample, kParentAtomIsContainer, kMovieMediaLoop, 1, 1, sizeof(myLoop), &myLoop, NULL);
		if (myErr != noErr)
			goto bail;
	}
	
	//////////
	//
	// set the slaving characteristics of the child movie
	//
	//////////
	
	myBoolean = (**myAppData).fSlaveTimebase;
	myErr = QTInsertChild(mySample, kParentAtomIsContainer, kMovieMediaSlaveTime, 1, 1, sizeof(myBoolean), &myBoolean, NULL);
	if (myErr != noErr)
		goto bail;
	
	myBoolean = (**myAppData).fSlaveAudio;
	myErr = QTInsertChild(mySample, kParentAtomIsContainer, kMovieMediaSlaveAudio, 1, 1, sizeof(myBoolean), &myBoolean, NULL);
	if (myErr != noErr)
		goto bail;
	
	myBoolean = (**myAppData).fSlaveGraphicsMode;
	myErr = QTInsertChild(mySample, kParentAtomIsContainer, kMovieMediaSlaveGraphicsMode, 1, 1, sizeof(myBoolean), &myBoolean, NULL);
	if (myErr != noErr)
		goto bail;

	myBoolean = (**myAppData).fSlaveDuration;
	myErr = QTInsertChild(mySample, kParentAtomIsContainer, kMovieMediaSlaveTrackDuration, 1, 1, sizeof(myBoolean), &myBoolean, NULL);
	if (myErr != noErr)
		goto bail;
	
	//////////
	//
	// set the spatial characteristics of the child movie, if requested
	//
	//////////

	if ((**myAppData).fCustomRectOn || (**myAppData).fScalingOn) {
		// add an atom of type kMovieMediaRegionAtom to the atom container
		myErr = QTInsertChild(mySample, kParentAtomIsContainer, kMovieMediaRegionAtom, 1, 1, 0, NULL, &myRegionAtom);
		if (myErr != noErr)
			goto bail;
	}
	
	if (myRegionAtom != 0) {
		
		if ((**myAppData).fCustomRectOn) {
			// set the child movie's rectangle
			QTAtom			myRectAtom;
			
			myErr = QTInsertChild(mySample, myRegionAtom, kMovieMediaRectangleAtom, 1, 1, 0, NULL, &myRectAtom);
			if (myRectAtom != 0) {
				if ((**myAppData).fTopAndLeftOn) {
					myValue = EndianU32_NtoB((**myAppData).fTop);
					myErr = QTInsertChild(mySample, myRectAtom, kMovieMediaTop, 1, 1, sizeof(myValue), &myValue, NULL);
				
					myValue = EndianU32_NtoB((**myAppData).fLeft);
					myErr = QTInsertChild(mySample, myRectAtom, kMovieMediaLeft, 1, 1, sizeof(myValue), &myValue, NULL);
				}
				
				if ((**myAppData).fHeightAndWidthOn) {
					myValue = EndianU32_NtoB((**myAppData).fHeight);
					myErr = QTInsertChild(mySample, myRectAtom, kMovieMediaHeight, 1, 1, sizeof(myValue), &myValue, NULL);
				
					myValue = EndianU32_NtoB((**myAppData).fWidth);
					myErr = QTInsertChild(mySample, myRectAtom, kMovieMediaWidth, 1, 1, sizeof(myValue), &myValue, NULL);
				}
			}
		}
		
		// set the scaling characteristics of the child movie
		if ((**myAppData).fScalingOn) {
			// add an atom of type kMovieMediaSpatialAdjustment to the atom container
			myValue = EndianU32_NtoB((**myAppData).fScalingType);
			myErr = QTInsertChild(mySample, myRegionAtom, kMovieMediaSpatialAdjustment, 1, 1, sizeof(myValue), &myValue, NULL);
			if (myErr != noErr)
				goto bail;
		}
	}
	
	//////////
	//
	// create the media sample
	//
	//////////

	// create a sample description
	myImageDesc = (SampleDescriptionHandle)NewHandleClear(sizeof(SampleDescription));
	if (myImageDesc == NULL)
		goto bail;

	(**myImageDesc).descSize = sizeof(SampleDescription);
	(**myImageDesc).dataFormat = MovieMediaType;

	myErr = AddMediaSample(	theMedia, 
							mySample,
							0,																	// no offset in data
							GetHandleSize((Handle)mySample), 
							GetMovieDuration(GetTrackMovie(GetMediaTrack(theMedia))),			// frame duration
							myImageDesc, 
							1,																	// one sample
							0,																	// self-contained samples
							NULL);

bail:
	if (myData != NULL)
		DisposePtr(myData);

	if (myImageDesc != NULL)
		DisposeHandle((Handle)myImageDesc);

	if (mySample != NULL)
		QTDisposeAtomContainer(mySample);

	return(myErr);
}


//////////
//
// QTMIM_GetURLFromUser
// Display a dialog box to elicit a URL from the user; return a C string that contains the text in the
// dialog box when the user clicks the OK button; otherwise, return NULL.
//
// The caller is responsible for disposing of the pointer returned by this function (by calling free).
//
//////////

char *QTMIM_GetURLFromUser (void)
{
	short 			myItem;
	short 			mySavedResFile;
	GrafPtr			mySavedPort;
	DialogPtr		myDialog = NULL;
	short			myItemKind;
	Handle			myItemHandle;
	Rect			myItemRect;
	Str255			myString;
	char			*myURL = NULL;
	OSErr			myErr = noErr;

	//////////
	//
	// save the current resource file and graphics port
	//
	//////////

	mySavedResFile = CurResFile();
	GetPort(&mySavedPort);

	// set the application's resource file
	UseResFile(gAppResFile);

	//////////
	//
	// create the dialog box in which the user will enter a URL
	//
	//////////

	myDialog = GetNewDialog(kURLBoxResourceID, NULL, (WindowPtr)-1L);
	if (myDialog == NULL)
		goto bail;

#if TARGET_API_MAC_CARBON		
	SetPortDialogPort(myDialog);
#else
	MacSetPort(myDialog);
#endif
	
	SetDialogDefaultItem(myDialog, kURLBoxItemOK);
	SetDialogCancelItem(myDialog, kURLBoxItemCancel);
	
	MacShowWindow(GetDialogWindow(myDialog));
	
	//////////
	//
	// display and handle events in the dialog box until the user clicks OK or Cancel
	//
	//////////
	
	do {
		ModalDialog(gModalFilterUPP, &myItem);
	} while ((myItem != kURLBoxItemOK) && (myItem != kURLBoxItemCancel));
	
	//////////
	//
	// handle the selected button
	//
	//////////
	
	if (myItem != kURLBoxItemOK) {
		myErr = userCanceledErr;
		goto bail;
	}
	
	// retrieve the edited text
	GetDialogItem(myDialog, kURLBoxItemEditBox, &myItemKind, &myItemHandle, &myItemRect);
	GetDialogItemText(myItemHandle, myString);
	myURL = QTUtils_ConvertPascalToCString(myString);
	
bail:
	// restore the previous resource file and graphics port
	MacSetPort(mySavedPort);
	UseResFile(mySavedResFile);
	
	if (myDialog != NULL)
		DisposeDialog(myDialog);

	return(myURL);
}


//////////
//
// QTMIM_ShowPropertiesDialogBox
// Display the Movie Track Properties dialog box and process the user's selections.
//
//////////

OSErr QTMIM_ShowPropertiesDialogBox (WindowObject theWindowObject)
{
	ApplicationDataHdl		myAppData = NULL;
	short 					myItem;
	short 					mySavedResFile;
	GrafPtr					mySavedPort;
	DialogPtr				myDialog = NULL;
	short					myItemKind;
	Handle					myItemHandle;
	Rect					myItemRect;
	SInt16					myValue;
	SInt16					myIndex;
	Str255					myString;
	UserItemUPP				myUserItemProcUPP = NULL;			// UPP to our custom dialog user item procedure
	OSErr					myErr = paramErr;
		
	if (theWindowObject == NULL)
		return(myErr);

	myAppData = (ApplicationDataHdl)QTFrame_GetAppDataFromWindowObject(theWindowObject);
	if (myAppData == NULL)
		return(myErr);
		
	myUserItemProcUPP = NewUserItemUPP(QTMIM_UserItemProcedure);

	//////////
	//
	// save the current resource file and graphics port
	//
	//////////

	mySavedResFile = CurResFile();
	GetPort(&mySavedPort);

	// set the application's resource file
	UseResFile(gAppResFile);

	//////////
	//
	// create the dialog box 
	//
	//////////

	myDialog = GetNewDialog(kPropsBoxResourceID, NULL, (WindowPtr)-1L);
	if (myDialog == NULL)
		goto bail;

	MacSetPort(GetDialogPort(myDialog));
	
	// set user-item procedures
	GetDialogItem(myDialog, kSlaveOptionsUserItem, &myItemKind, &myItemHandle, &myItemRect);
	SetDialogItem(myDialog, kSlaveOptionsUserItem, myItemKind, (Handle)myUserItemProcUPP, &myItemRect);
	
	GetDialogItem(myDialog, kLoopingOptionsUserItem, &myItemKind, &myItemHandle, &myItemRect);
	SetDialogItem(myDialog, kLoopingOptionsUserItem, myItemKind, (Handle)myUserItemProcUPP, &myItemRect);
	
	GetDialogItem(myDialog, kScalingOptionsUserItem, &myItemKind, &myItemHandle, &myItemRect);
	SetDialogItem(myDialog, kScalingOptionsUserItem, myItemKind, (Handle)myUserItemProcUPP, &myItemRect);
	
	GetDialogItem(myDialog, kRectangleOptionsUserItem, &myItemKind, &myItemHandle, &myItemRect);
	SetDialogItem(myDialog, kRectangleOptionsUserItem, myItemKind, (Handle)myUserItemProcUPP, &myItemRect);
	
	GetDialogItem(myDialog, kCommentBoxUserItem, &myItemKind, &myItemHandle, &myItemRect);
	SetDialogItem(myDialog, kCommentBoxUserItem, myItemKind, (Handle)myUserItemProcUPP, &myItemRect);
	
	SetDialogDefaultItem(myDialog, kPropsItemOK);
	SetDialogCancelItem(myDialog, kPropsItemCancel);
	
	// set default values
	GetDialogItem(myDialog, kPropsAutoPlayChild, &myItemKind, &myItemHandle, &myItemRect);
	SetControlValue((ControlHandle)myItemHandle, 0);
	
	GetDialogItem(myDialog, kFrameStepChild, &myItemKind, &myItemHandle, &myItemRect);
	SetControlValue((ControlHandle)myItemHandle, 0);
	
	GetDialogItem(myDialog, kScalingOptionFill, &myItemKind, &myItemHandle, &myItemRect);
	SetControlValue((ControlHandle)myItemHandle, 1);
	
	GetDialogItem(myDialog, kPropsNoLooping, &myItemKind, &myItemHandle, &myItemRect);
	SetControlValue((ControlHandle)myItemHandle, 1);
	
	GetDialogItem(myDialog, kUseTrackBoxRadio, &myItemKind, &myItemHandle, &myItemRect);
	SetControlValue((ControlHandle)myItemHandle, 1);
	
	// disable rectangle specifiers
	GetDialogItem(myDialog, kTopAndLeftCheckBox, &myItemKind, &myItemHandle, &myItemRect);
	HiliteControl((ControlHandle)myItemHandle, 255);
	
	GetDialogItem(myDialog, kHeightAndWidthCheckBox, &myItemKind, &myItemHandle, &myItemRect);
	HiliteControl((ControlHandle)myItemHandle, 255);
	
	GetDialogItem(myDialog, kLoopingOptionsCheckBox, &myItemKind, &myItemHandle, &myItemRect);
	SetControlValue((ControlHandle)myItemHandle, 1);
	
	GetDialogItem(myDialog, kScalingOptionsCheckBox, &myItemKind, &myItemHandle, &myItemRect);
	SetControlValue((ControlHandle)myItemHandle, 1);
	
	MacShowWindow(GetDialogWindow(myDialog));
	
	QTMIM_ShowComment(myDialog, kDefaultCommentItem);
	
	//////////
	//
	// display and handle events in the dialog box until the user clicks OK or Cancel
	//
	//////////
	
	do {
		ModalDialog(gModalFilterUPP, &myItem);
		
		// erase any existing comment
		QTMIM_ShowComment(myDialog, kEraseCommentFieldItem);
		
		switch (myItem) {
			// handle clicks on check boxes
			case kPropsSlaveTimebase:
			case kPropsSlaveAudio:
			case kPropsSlaveGraphicsMode:
			case kPropsSlaveDuration:
			case kPropsAutoPlayChild:
			case kFrameStepChild:
			case kScalingOptionsCheckBox:
			case kTopAndLeftCheckBox:
			case kHeightAndWidthCheckBox:
			case kLoopingOptionsCheckBox:
				GetDialogItem(myDialog, myItem, &myItemKind, &myItemHandle, &myItemRect);
				myValue = 1 - GetControlValue((ControlHandle)myItemHandle);	
				if (myValue == 1)
					QTMIM_ShowComment(myDialog, myItem);
				SetControlValue((ControlHandle)myItemHandle, myValue);
				
				if (myItem == kScalingOptionsCheckBox) {
					myValue = GetControlValue((ControlHandle)myItemHandle);	
				
					for (myIndex = kScalingOptionClip; myIndex <= kScalingOptionScroll; myIndex++) {
						GetDialogItem(myDialog, myIndex, &myItemKind, &myItemHandle, &myItemRect);
						HiliteControl((ControlHandle)myItemHandle, myValue ? 0 : 255);
					}
				}
				
				if (myItem == kLoopingOptionsCheckBox) {
					myValue = GetControlValue((ControlHandle)myItemHandle);	
				
					for (myIndex = kPropsNoLooping; myIndex <= kPropsPalindromeLooping; myIndex++) {
						GetDialogItem(myDialog, myIndex, &myItemKind, &myItemHandle, &myItemRect);
						HiliteControl((ControlHandle)myItemHandle, myValue ? 0 : 255);
					}
				}
				
				break;
				
			// handle clicks in radio button groups
			case kPropsNoLooping:
			case kPropsNormalLooping:
			case kPropsPalindromeLooping:
				for (myIndex = kPropsNoLooping; myIndex <= kPropsPalindromeLooping; myIndex++) {
					GetDialogItem(myDialog, myIndex, &myItemKind, &myItemHandle, &myItemRect);
					if (myItem == myIndex)
						SetControlValue((ControlHandle)myItemHandle, 1);	
					else
						SetControlValue((ControlHandle)myItemHandle, 0);	
				}
				
				QTMIM_ShowComment(myDialog, myItem);
				break;
				
			case kScalingOptionClip:
			case kScalingOptionFill:
			case kScalingOptionMeet:
			case kScalingOptionSlice:
			case kScalingOptionScroll:
				for (myIndex = kScalingOptionClip; myIndex <= kScalingOptionScroll; myIndex++) {
					GetDialogItem(myDialog, myIndex, &myItemKind, &myItemHandle, &myItemRect);
					if (myItem == myIndex)
						SetControlValue((ControlHandle)myItemHandle, 1);	
					else
						SetControlValue((ControlHandle)myItemHandle, 0);	
				}
				
				QTMIM_ShowComment(myDialog, myItem);
				break;
				
			case kUseTrackBoxRadio:
			case kUseRectangleRadio:
				for (myIndex = kUseTrackBoxRadio; myIndex <= kUseRectangleRadio; myIndex++) {
					GetDialogItem(myDialog, myIndex, &myItemKind, &myItemHandle, &myItemRect);
					if (myItem == myIndex)
						SetControlValue((ControlHandle)myItemHandle, 1);	
					else
						SetControlValue((ControlHandle)myItemHandle, 0);
						
					GetDialogItem(myDialog, kUseRectangleRadio, &myItemKind, &myItemHandle, &myItemRect);
					myValue = GetControlValue((ControlHandle)myItemHandle);	
				
					// enable or disable rectangle specifiers
					GetDialogItem(myDialog, kTopAndLeftCheckBox, &myItemKind, &myItemHandle, &myItemRect);
					HiliteControl((ControlHandle)myItemHandle, myValue ? 0 : 255);
				
					GetDialogItem(myDialog, kHeightAndWidthCheckBox, &myItemKind, &myItemHandle, &myItemRect);
					HiliteControl((ControlHandle)myItemHandle, myValue ? 0 : 255);
				}
				
				QTMIM_ShowComment(myDialog, myItem);
				break;
				
			// handle clicks in buttons
			case kShowChildOnParent:
				QTMIM_ShowChildRectOnParent(myDialog, theWindowObject);
				break;
		}
		
		// if OK button clicked, make sure we've got all the info we need
		if (myItem == kPropsItemOK) {
			// if "Use Rectangle" is selected, make sure either T/L or H/W is also selected
			GetDialogItem(myDialog, kUseRectangleRadio, &myItemKind, &myItemHandle, &myItemRect);
			if (GetControlValue((ControlHandle)myItemHandle) == 1) {
				Boolean		myTopLeftChecked = false;
				Boolean		myHeightWidthChecked = false;
			
				GetDialogItem(myDialog, kTopAndLeftCheckBox, &myItemKind, &myItemHandle, &myItemRect);
				myTopLeftChecked = (Boolean)GetControlValue((ControlHandle)myItemHandle);

				GetDialogItem(myDialog, kHeightAndWidthCheckBox, &myItemKind, &myItemHandle, &myItemRect);
				myHeightWidthChecked = (Boolean)GetControlValue((ControlHandle)myItemHandle);

				if (!myTopLeftChecked && !myHeightWidthChecked) {
					// show warning and don't exit the dialog box
					QTMIM_ShowComment(myDialog, kNeedTLOrHWItem);
					QTFrame_Beep();
					myItem = kPropsBogusItem;
				}
			}
		}
		
	} while ((myItem != kPropsItemOK) && (myItem != kPropsItemCancel));
	
	//////////
	//
	// handle the selected button
	//
	//////////
	
	if (myItem != kPropsItemOK) {
		myErr = userCanceledErr;
		goto bail;
	}
	
	// read the values from the dialog box items into a data structure
	
	// slaving options
	GetDialogItem(myDialog, kPropsSlaveTimebase, &myItemKind, &myItemHandle, &myItemRect);
	(**myAppData).fSlaveTimebase = GetControlValue((ControlHandle)myItemHandle);

	GetDialogItem(myDialog, kPropsSlaveAudio, &myItemKind, &myItemHandle, &myItemRect);
	(**myAppData).fSlaveAudio = GetControlValue((ControlHandle)myItemHandle);

	GetDialogItem(myDialog, kPropsSlaveGraphicsMode, &myItemKind, &myItemHandle, &myItemRect);
	(**myAppData).fSlaveGraphicsMode = GetControlValue((ControlHandle)myItemHandle);

	GetDialogItem(myDialog, kPropsSlaveDuration, &myItemKind, &myItemHandle, &myItemRect);
	(**myAppData).fSlaveDuration = GetControlValue((ControlHandle)myItemHandle);

	// scaling type
	GetDialogItem(myDialog, kScalingOptionsCheckBox, &myItemKind, &myItemHandle, &myItemRect);
	(**myAppData).fScalingOn = GetControlValue((ControlHandle)myItemHandle);
	if ((**myAppData).fScalingOn) {
		GetDialogItem(myDialog, kScalingOptionClip, &myItemKind, &myItemHandle, &myItemRect);
		if (GetControlValue((ControlHandle)myItemHandle))
			(**myAppData).fScalingType = kMovieMediaFitClipIfNecessary;
			
		GetDialogItem(myDialog, kScalingOptionFill, &myItemKind, &myItemHandle, &myItemRect);
		if (GetControlValue((ControlHandle)myItemHandle))
			(**myAppData).fScalingType = kMovieMediaFitFill;
			
		GetDialogItem(myDialog, kScalingOptionMeet, &myItemKind, &myItemHandle, &myItemRect);
		if (GetControlValue((ControlHandle)myItemHandle))
			(**myAppData).fScalingType = kMovieMediaFitMeet;
			
		GetDialogItem(myDialog, kScalingOptionSlice, &myItemKind, &myItemHandle, &myItemRect);
		if (GetControlValue((ControlHandle)myItemHandle))
			(**myAppData).fScalingType = kMovieMediaFitSlice;
			
		GetDialogItem(myDialog, kScalingOptionScroll, &myItemKind, &myItemHandle, &myItemRect);
		if (GetControlValue((ControlHandle)myItemHandle))
			(**myAppData).fScalingType = kMovieMediaFitScroll;
	} else {
		(**myAppData).fScalingType = kMovieMediaFitNone;
	}

	// looping type
	GetDialogItem(myDialog, kLoopingOptionsCheckBox, &myItemKind, &myItemHandle, &myItemRect);
	(**myAppData).fLoopingOn = GetControlValue((ControlHandle)myItemHandle);
	if ((**myAppData).fLoopingOn) {
		GetDialogItem(myDialog, kPropsNoLooping, &myItemKind, &myItemHandle, &myItemRect);
		if (GetControlValue((ControlHandle)myItemHandle))
			(**myAppData).fLoopingState = kMIMNoLooping;
			
		GetDialogItem(myDialog, kPropsNormalLooping, &myItemKind, &myItemHandle, &myItemRect);
		if (GetControlValue((ControlHandle)myItemHandle))
			(**myAppData).fLoopingState = kMIMNormalLooping;
			
		GetDialogItem(myDialog, kPropsPalindromeLooping, &myItemKind, &myItemHandle, &myItemRect);
		if (GetControlValue((ControlHandle)myItemHandle))
			(**myAppData).fLoopingState = kMIMPalindromeLooping;
	} else {
		(**myAppData).fLoopingState = kMIMNoLooping;
	}

	// autoplay option
	GetDialogItem(myDialog, kPropsAutoPlayChild, &myItemKind, &myItemHandle, &myItemRect);
	(**myAppData).fAutoPlayChild = GetControlValue((ControlHandle)myItemHandle);

	// frame-stepping option
	GetDialogItem(myDialog, kFrameStepChild, &myItemKind, &myItemHandle, &myItemRect);
	(**myAppData).fFrameStepChild = GetControlValue((ControlHandle)myItemHandle);

	// media rectangle
	GetDialogItem(myDialog, kUseRectangleRadio, &myItemKind, &myItemHandle, &myItemRect);
	(**myAppData).fCustomRectOn = GetControlValue((ControlHandle)myItemHandle);
	if ((**myAppData).fCustomRectOn) {
		GetDialogItem(myDialog, kTopAndLeftCheckBox, &myItemKind, &myItemHandle, &myItemRect);
		(**myAppData).fTopAndLeftOn = GetControlValue((ControlHandle)myItemHandle);

		GetDialogItem(myDialog, kHeightAndWidthCheckBox, &myItemKind, &myItemHandle, &myItemRect);
		(**myAppData).fHeightAndWidthOn = GetControlValue((ControlHandle)myItemHandle);

		GetDialogItem(myDialog, kTopEditTextBox, &myItemKind, &myItemHandle, &myItemRect);
		GetDialogItemText(myItemHandle, myString);
		StringToNum(myString, &(**myAppData).fTop);

		GetDialogItem(myDialog, kLeftEditTextBox, &myItemKind, &myItemHandle, &myItemRect);
		GetDialogItemText(myItemHandle, myString);
		StringToNum(myString, &(**myAppData).fLeft);

		GetDialogItem(myDialog, kHeightEditTextBox, &myItemKind, &myItemHandle, &myItemRect);
		GetDialogItemText(myItemHandle, myString);
		StringToNum(myString, &(**myAppData).fHeight);

		GetDialogItem(myDialog, kWidthEditTextBox, &myItemKind, &myItemHandle, &myItemRect);
		GetDialogItemText(myItemHandle, myString);
		StringToNum(myString, &(**myAppData).fWidth);
	}
	
	myErr = noErr;
	
bail:
	// restore the previous resource file and graphics port
	MacSetPort(mySavedPort);
	UseResFile(mySavedResFile);
	
	if (myDialog != NULL)
		DisposeDialog(myDialog);
		
	DisposeUserItemUPP(myUserItemProcUPP);

	return(myErr);
}


//////////
//
// QTMIM_UserItemProcedure
// A user-item procedure to draw boxes.
//
//////////

PASCAL_RTN void QTMIM_UserItemProcedure (DialogPtr theDialog, short theItem)
{
	short					myItemKind;
	Handle					myItemHandle = NULL;
	Rect					myItemRect;
	GrafPtr					mySavedPort;
	PenState				myPenState;
	OSErr					myErr = noErr;
	
	GetPort(&mySavedPort);
#if TARGET_API_MAC_CARBON		
	SetPortDialogPort(theDialog);
#else
	MacSetPort(theDialog);
#endif
	
	// get the rectangle surrounding the user item
	GetDialogItem(theDialog, theItem, &myItemKind, &myItemHandle, &myItemRect);
	
	GetPenState(&myPenState);

	// draw some bevels around the rectangle
	MacOffsetRect(&myItemRect, 1, 1);
	RGBForeColor(&gWhite);
	MacFrameRect(&myItemRect);

	MacOffsetRect(&myItemRect, -1, -1);
	RGBForeColor(&gDGray);
	MacFrameRect(&myItemRect);

	if (theItem == kRectangleOptionsUserItem) {
		RGBForeColor(&gDGray);
		MoveTo(myItemRect.left, myItemRect.top + ((myItemRect.bottom - myItemRect.top) / 2));
		Line((myItemRect.right - myItemRect.left) - 2, 0);
		RGBForeColor(&gWhite);
		MoveTo(myItemRect.left + 1, myItemRect.top + ((myItemRect.bottom - myItemRect.top) / 2) + 1);
		Line((myItemRect.right - myItemRect.left) - 3, 0);
	}

	SetPenState(&myPenState);
	RGBForeColor(&gBlack);

	MacSetPort(mySavedPort);
}


//////////
//
// QTMIM_ShowComment
// Print a comment in the Movie Track Properies dialog box.
//
//////////

void QTMIM_ShowComment (DialogPtr theDialog, short theItem)
{
	short					myTextSize;
	short					myItemKind;
	Handle					myItemHandle = NULL;
	Rect					myItemRect;
	Str255					myString;
	GrafPtr 				mySavedPort;
	OSErr					myErr = noErr;
	
#if TARGET_OS_MAC		
	myTextSize = 10;
#endif
#if TARGET_OS_WIN32		
	myTextSize = 8;
#endif

	GetPort(&mySavedPort);
#if TARGET_API_MAC_CARBON		
	SetPortDialogPort(theDialog);
#else
	MacSetPort(theDialog);
#endif

	GetIndString(myString, kCommentStringResID, theItem);
	
	// get the rectangle surrounding the comment user item
	GetDialogItem(theDialog, kCommentBoxUserItem, &myItemKind, &myItemHandle, &myItemRect);
	MacInsetRect(&myItemRect, 2, 2);
	EraseRect(&myItemRect);
	MacInsetRect(&myItemRect, -2, -2);

	TextSize(myTextSize);
	TextFont(1);
	TextFace(0);
	
	MoveTo(myItemRect.left + 5, myItemRect.bottom - 6);
	DrawString(myString);
	
	TextSize(12);
	TextFont(0);
	TextFace(0);
	
	MacSetPort(mySavedPort);
}


//////////
//
// QTMIM_ShowChildRectOnParent
// Show the currently selected child rectangle on the parent.
//
//////////

void QTMIM_ShowChildRectOnParent (DialogPtr theDialog, WindowObject theWindowObject)
{
	short				myItemKind;
	Handle				myItemHandle = NULL;
	Rect				myItemRect;
	Rect				myChildRect;
	GrafPtr 			mySavedPort;
	MovieController		myMC = NULL;
	
	GetPort(&mySavedPort);
	MacSetPort(QTFrame_GetPortFromWindowReference(QTFrame_GetFrontMovieWindow()));

	myMC = QTFrame_GetMCFromFrontWindow();
	if (myMC == NULL)
		goto bail;
	
	if (theWindowObject == NULL)
		goto bail;
	
	// redraw the current movie frame (to erase any existing child rectangle)
	MCDoAction(myMC, mcActionDraw, (void *)QTFrame_GetWindowFromWindowReference(QTFrame_GetFrontMovieWindow()));
	
	RGBForeColor(&gDGray);
	
	// set the default rectangle
	MacSetRect(&myChildRect, 0, 0, kChildMovieWidth, kChildMovieHeight);

	// get the current child rectangle option
	GetDialogItem(theDialog, kUseTrackBoxRadio, &myItemKind, &myItemHandle, &myItemRect);
	if (GetControlValue((ControlHandle)myItemHandle) == 1) {
		// the "Use Track Box" option is selected; just use the default values defined above
	} else {
		// the "Use Rectangle" option is selected
		Boolean			myTopLeftChecked = false;
		Boolean			myHeightWidthChecked = false;
		Str255			myString;
		long			myPos;
	
		GetDialogItem(theDialog, kTopAndLeftCheckBox, &myItemKind, &myItemHandle, &myItemRect);
		myTopLeftChecked = (Boolean)GetControlValue((ControlHandle)myItemHandle);

		GetDialogItem(theDialog, kHeightAndWidthCheckBox, &myItemKind, &myItemHandle, &myItemRect);
		myHeightWidthChecked = (Boolean)GetControlValue((ControlHandle)myItemHandle);

		if (!myTopLeftChecked && !myHeightWidthChecked) {
			// show warning and don't exit the dialog box
			QTMIM_ShowComment(theDialog, kNeedTLOrHWItem);
			QTFrame_Beep();
			goto bail;
		}

		if (myTopLeftChecked) {
			// get the Top and Left values
			GetDialogItem(theDialog, kTopEditTextBox, &myItemKind, &myItemHandle, &myItemRect);
			GetDialogItemText(myItemHandle, myString);
			StringToNum(myString, &myPos);
			myChildRect.top = (short)myPos;
			myChildRect.bottom += myChildRect.top;

			GetDialogItem(theDialog, kLeftEditTextBox, &myItemKind, &myItemHandle, &myItemRect);
			GetDialogItemText(myItemHandle, myString);
			StringToNum(myString, &myPos);
			myChildRect.left = (short)myPos;
			myChildRect.right += myChildRect.left;
		}
	
		if (myHeightWidthChecked) {
			// get the Height and Width values
			GetDialogItem(theDialog, kHeightEditTextBox, &myItemKind, &myItemHandle, &myItemRect);
			GetDialogItemText(myItemHandle, myString);
			StringToNum(myString, &myPos);
			myChildRect.bottom = myChildRect.top + (short)myPos;

			GetDialogItem(theDialog, kWidthEditTextBox, &myItemKind, &myItemHandle, &myItemRect);
			GetDialogItemText(myItemHandle, myString);
			StringToNum(myString, &myPos);
			myChildRect.right = myChildRect.left + (short)myPos;
		}
	}

	if (!EmptyRect(&myChildRect)) {
		Rect			myMovieRect;
		RgnHandle		myClipRegion = NULL;
		
		// clip the child rectangle to the movie rectangle
		myClipRegion = NewRgn();
		if (myClipRegion != NULL) {
			GetClip(myClipRegion);
			GetMovieBox((**theWindowObject).fMovie, &myMovieRect);
			ClipRect(&myMovieRect);
			PaintRect(&myChildRect);
			SetClip(myClipRegion);
			DisposeRgn(myClipRegion);
		}
		
		QTMIM_ShowComment(theDialog, kShowChildOnParent);
	} else {
		QTMIM_ShowComment(theDialog, kEmptyRectWarning);
		QTFrame_Beep();
	}
		
bail:
	RGBForeColor(&gBlack);
	MacSetPort(mySavedPort);
}


//////////
//
// QTMIM_FilterFiles
// Filter files for a file-opening dialog box.
//
//////////

#if TARGET_OS_MAC
PASCAL_RTN Boolean QTMIM_FilterFiles (AEDesc *theItem, void *theInfo, void *theCallBackUD, NavFilterModes theFilterMode)
{
#pragma unused(theCallBackUD, theFilterMode)
	NavFileOrFolderInfo		*myInfo = (NavFileOrFolderInfo *)theInfo;
	
	if (theItem->descriptorType == typeFSS) {
		if (!myInfo->isFolder) {
			if (myInfo->fileAndFolder.fileInfo.finderInfo.fdType == kQTFileTypeMovie)
				return(true);

			// if we got to here, it's a file we cannot open
			return(false);		
		}
	}
	
	// if we got to here, it's a folder or non-HFS object
	return(true);
}
#endif
#if TARGET_OS_WIN32
PASCAL_RTN Boolean QTMIM_FilterFiles (CInfoPBPtr thePBPtr)
{
#pragma unused(thePBPtr)
	return(false);
}
#endif
