//////////
//
//	File:		QTInfo.c
//
//	Contains:	Sample code for working with QuickTime movie information.
//
//	Written by:	Tim Monroe
//
//	Copyright:	© 2000 by Apple Computer, Inc., all rights reserved.
//
//	Change History (most recent first):
//	   
//	   <1>	 	04/23/00	rtm		first file
//
//////////


//////////
//
// header files
//
//////////

#include "QTInfo.h"


//////////
//
// global variables
//
//////////

extern short 				gAppResFile;						// file reference number for this application's resource file
extern ModalFilterUPP		gModalFilterUPP;					// UPP to our custom dialog event filter


//////////
//
// QTInfo_MovieHasPreview
// Does the specified movie have a movie preview?
//
//////////

Boolean QTInfo_MovieHasPreview (Movie theMovie)
{
	TimeValue			myStart;
	TimeValue			myDuration;
	long				myCount = 0L;
	long				myIndex = 0L;
	Track				myTrack = NULL;
	long				myUsage = 0L;
	Boolean				myHasPreview = false;

	// see if the movie has a positive preview duration
	GetMoviePreviewTime(theMovie, &myStart, &myDuration);
	if (myDuration > 0)
		myHasPreview = true;

	// make sure that some track is used in the movie preview
	myCount = GetMovieTrackCount(theMovie);
	for (myIndex = 1; myIndex <= myCount; myIndex++) {
		myTrack = GetMovieIndTrack(theMovie, myIndex);
		if (myTrack == NULL)
			continue;
			
		myUsage = GetTrackUsage(myTrack);
		if (myUsage & trackUsageInPreview)
			break;				// we found a track with the trackUsageInPreview flag set; break out of the loop
	}
	
	if (myIndex > myCount)
		myHasPreview = false;	// we went thru all tracks without finding one with a preview usage

	return(myHasPreview);
}


//////////
//
// QTInfo_MovieHasPoster
// Does the specified movie have a movie poster?
//
//////////

Boolean QTInfo_MovieHasPoster (Movie theMovie)
{
	long				myCount = 0L;
	long				myIndex = 0L;
	Track				myTrack = NULL;
	long				myUsage = 0L;
	Boolean				myHasPoster = true;

	// make sure that some track is used in the movie poster
	myCount = GetMovieTrackCount(theMovie);
	for (myIndex = 1; myIndex <= myCount; myIndex++) {
		myTrack = GetMovieIndTrack(theMovie, myIndex);
		if (myTrack == NULL)
			continue;
			
		myUsage = GetTrackUsage(myTrack);
		if (myUsage & trackUsageInPoster)
			break;				// we found a track with the trackUsageInPoster flag set; break out of the loop
	}
	
	if (myIndex > myCount)
		myHasPoster = false;	// we went thru all tracks without finding one with a poster usage

	return(myHasPoster);
}


//////////
//
// QTInfo_SetPreviewToSelection
// Set the movie preview to be the current movie selection.
//
//////////

OSErr QTInfo_SetPreviewToSelection (Movie theMovie, MovieController theMC)
{
	TimeValue			myStart;
	TimeValue			myDuration;
	ComponentResult		myErr = noErr;
	
	GetMovieSelection(theMovie, &myStart, &myDuration);
	SetMoviePreviewTime(theMovie, myStart, myDuration);

	myErr = MCMovieChanged(theMC, theMovie);
	
	return((OSErr)myErr);
}


//////////
//
// QTInfo_SetSelectionToPreview
// Set the movie selection to the current movie preview.
//
//////////

OSErr QTInfo_SetSelectionToPreview (Movie theMovie, MovieController theMC)
{
	TimeValue			myStart;
	TimeValue			myDuration;
	ComponentResult		myErr = noErr;

	GetMoviePreviewTime(theMovie, &myStart, &myDuration);
	SetMovieSelection(theMovie, myStart, myDuration);

	myErr = MCMovieChanged(theMC, theMovie);
	
	return((OSErr)myErr);
}


//////////
//
// QTInfo_ClearPreview
// Clear the movie preview.
//
//////////

OSErr QTInfo_ClearPreview (Movie theMovie, MovieController theMC)
{
	long				myCount = 0L;
	long				myIndex = 0L;
	Track				myTrack = NULL;
	long				myUsage = 0L;
	ComponentResult		myErr = noErr;

	// set the movie preview start time and duration to 0
	SetMoviePreviewTime(theMovie, 0, 0);

	// remove all tracks that are used *only* in the movie preview
	myCount = GetMovieTrackCount(theMovie);
	for (myIndex = myCount; myIndex >= 1; myIndex--) {
		myTrack = GetMovieIndTrack(theMovie, myIndex);
		if (myTrack == NULL)
			continue;
		
		myUsage = GetTrackUsage(myTrack);
		myUsage &= trackUsageInMovie | trackUsageInPreview | trackUsageInPoster;
		if (myUsage == trackUsageInPreview)
			DisposeMovieTrack(myTrack);
	}

	// add trackUsageInPreview to any remaining tracks that are in the movie
	// (so that subsequently setting the preview to a selection will include
	// these tracks)
	myCount = GetMovieTrackCount(theMovie);
	for (myIndex = 1; myIndex <= myCount; myIndex++) {
		myTrack = GetMovieIndTrack(theMovie, myIndex);
		if (myTrack == NULL)
			continue;
			
		myUsage = GetTrackUsage(myTrack);
		if (myUsage & trackUsageInMovie)
			SetTrackUsage(myTrack, myUsage | trackUsageInPreview);
	}

	myErr = MCMovieChanged(theMC, theMovie);
	
	return((OSErr)myErr);
}


//////////
//
// QTInfo_GoToPosterFrame
// Set the current movie time to the movie poster time.
//
//////////

OSErr QTInfo_GoToPosterFrame (Movie theMovie, MovieController theMC)
{
	TimeRecord			myTimeRecord;
	ComponentResult		myErr = noErr;

	// stop the movie from playing
	myErr = MCDoAction(theMC, mcActionPlay, (void *)0L);
	if (myErr != noErr)
		goto bail;
		
	// set up a time record with the desired movie time, scale, and base
	myTimeRecord.value.hi = 0;
	myTimeRecord.value.lo = GetMoviePosterTime(theMovie);
	myTimeRecord.base = GetMovieTimeBase(theMovie);
	myTimeRecord.scale = GetMovieTimeScale(theMovie);	

	myErr = MCDoAction(theMC, mcActionGoToTime, &myTimeRecord);

bail:
	return((OSErr)myErr);
}


//////////
//
// QTInfo_SetPosterToFrame
// Set the movie poster time to the current movie time.
//
//////////

OSErr QTInfo_SetPosterToFrame (Movie theMovie, MovieController theMC)
{
	TimeValue			myTime;
	ComponentResult		myErr = noErr;

	// stop the movie from playing
	myErr = MCDoAction(theMC, mcActionPlay, (void *)0L);
	if (myErr != noErr)
		goto bail;
	
	myTime = GetMovieTime(theMovie, NULL);
	SetMoviePosterTime(theMovie, myTime);

	myErr = MCMovieChanged(theMC, theMovie);
	
bail:
	return((OSErr)myErr);
}


//////////
//
// QTInfo_MakeFilePreview
// Add a file preview to the specified file.
//
// If theRefNum picks out a resource fork, we simply call the ICM function MakeFilePreview.
// If theRefNum picks out a data fork, we need to add a 'pnot' atom to that file; we do this
// by appending the appropriate atom data to the end of the file. This strategy will fail if
// the atom that was previously the last one in the file does not have a correct length value
// in its atom header.
//
// For poster previews, we also need to add an atom of type 'PICT' to the end of the file.
//
// NOTE: The current version of this function does not replace any existing 'pnot' atom in the
// data fork. A better algorithm would be: determine which type of file preview is being created,
// and count any existing preview data atoms of that type; then look for an existing 'pnot' atom;
// if one is found, update it to use the (possibly new) file preview type and set the index to
// one more than the number of existing preview data atoms of that type; than add the new preview
// data atom. These and other enhancements are left as an exercise for the reader.
//
//////////

OSErr QTInfo_MakeFilePreview (Movie theMovie, short theRefNum, ICMProgressProcRecordPtr theProgressProc)
{
	unsigned long			myModDate;
	PreviewResourceRecord	myPNOTRecord;
	long					myEOF;
	long					mySize;
	unsigned long			myAtomHeader[2];	// an atom header: size and type
	OSType					myPreviewType;
	OSErr					myErr = noErr;

	//////////
	//
	// determine whether theRefNum is a file reference number of a data fork or a resource fork;
	// if it's a resource fork, then we'll just call the existing ICM function MakeFilePreview
	//
	//////////

	if (QTInfo_IsRefNumOfResourceFork(theRefNum)) {
		myErr = MakeFilePreview(theRefNum, theProgressProc);
		goto bail;
	}
	
	//////////
	//
	// determine the preview type
	//
	//////////
	
	// if the movie has a movie preview, use that as the file preview; otherwise use a thumbnail
	// of the movie poster frame as the file preview
	if (QTInfo_MovieHasPreview(theMovie))
		myPreviewType = MovieAID;
	else
		myPreviewType = kQTFileTypePicture;

	//////////
	//
	// construct the 'pnot' atom
	//
	//////////

	// fill in the 'pnot' atom header
	myAtomHeader[0] = EndianU32_NtoB(sizeof(myAtomHeader) + sizeof(myPNOTRecord));
	myAtomHeader[1] = EndianU32_NtoB(ShowFilePreviewComponentType);
	
	// fill in the 'pnot' atom data
	GetDateTime(&myModDate);
	
	myPNOTRecord.modDate = EndianU32_NtoB(myModDate);		// the modification time of the preview
	myPNOTRecord.version = EndianS16_NtoB(0);				// version number; must be 0
	myPNOTRecord.resType = EndianU32_NtoB(myPreviewType);	// atom type containing preview
	myPNOTRecord.resID = EndianS16_NtoB(1);					// the 1-based index of the atom of the specified type to use
	
	//////////
	//
	// write the 'pnot' atom at the end of the data fork
	//
	//////////

	// get the current logical end-of-file and extend it by the desired amount
	myErr = GetEOF(theRefNum, &myEOF);
	if (myErr != noErr)
		goto bail;
	
	myErr = SetEOF(theRefNum, myEOF + sizeof(myAtomHeader) + sizeof(myPNOTRecord));
	if (myErr != noErr)
		goto bail;

	// set the file mark
	myErr = SetFPos(theRefNum, fsFromStart, myEOF);
	if (myErr != noErr)
		goto bail;
	
	// write the atom header into the file
	mySize = sizeof(myAtomHeader);
	myErr = FSWrite(theRefNum, &mySize, myAtomHeader);
	if (myErr != noErr)
		goto bail;
	
	// write the atom data into the file
	mySize = sizeof(myPNOTRecord);
	myErr = FSWrite(theRefNum, &mySize, &myPNOTRecord);
	if (myErr != noErr)
		goto bail;
	
	//////////
	//
	// write the preview data atom at the end of the data fork
	//
	//////////

	if (myPreviewType == MovieAID) {
		// in theory, we don't need to do anything here, since our 'pnot' atom points
		// to the 'moov' atom; in practice, this doesn't work correctly (it seems like
		// a bug in StandardGetFilePreview)
	}
	
	if (myPreviewType == kQTFileTypePicture) {
		PicHandle		myPicture = NULL;
		PicHandle		myThumbnail = NULL;

		// get the poster frame picture
		myPicture = GetMoviePosterPict(theMovie);
		if (myPicture != NULL) {
		
			// create a thumbnail
			myThumbnail = (PicHandle)NewHandleClear(4);
			if (myThumbnail != NULL) {
				myErr = MakeThumbnailFromPicture(myPicture, 0, myThumbnail, theProgressProc);
				if (myErr == noErr) {
				
					myAtomHeader[0] = EndianU32_NtoB(sizeof(myAtomHeader) + GetHandleSize((Handle)myThumbnail));
					myAtomHeader[1] = EndianU32_NtoB(myPreviewType);

					// write the atom header into the file
					mySize = sizeof(myAtomHeader);
					myErr = FSWrite(theRefNum, &mySize, myAtomHeader);
					if (myErr == noErr) {
						// write the atom data into the file
						mySize = GetHandleSize((Handle)myThumbnail);
						myErr = FSWrite(theRefNum, &mySize, *myThumbnail);
					}
				}
				
				KillPicture(myThumbnail);
			}
		
			KillPicture(myPicture);
		}
	}
	
#if 0
	// here's how you'd add a text preview; note that the text is hard-coded here....
	if (myPreviewType == kQTFileTypeText) {
		char 	myText[] = "The penguin gradually appears from the mist of the ice floe.";
		
		myAtomHeader[0] = EndianU32_NtoB(sizeof(myAtomHeader) + strlen(myText));
		myAtomHeader[1] = EndianU32_NtoB(myPreviewType);

		// write the atom header into the file
		mySize = sizeof(myAtomHeader);
		myErr = FSWrite(theRefNum, &mySize, myAtomHeader);
		if (myErr != noErr)
			goto bail;
		
		// write the atom data into the file
		mySize = strlen(myText);
		myErr = FSWrite(theRefNum, &mySize, myText);
		if (myErr != noErr)
			goto bail;
	}
#endif
	
#if TARGET_OS_MAC
	if (myErr == noErr) {
		short				myVolNum;
		
		// flush the volume
		myErr = GetVRefNum(theRefNum, &myVolNum);
		if (myErr != noErr)
			goto bail;
		
		myErr = FlushVol(NULL, myVolNum);
	}	
#endif	

bail:
	return(myErr);
}


//////////
//
// QTInfo_IsRefNumOfResourceFork
// Does the specified file reference number refer to an open resource fork?
//
//////////

Boolean QTInfo_IsRefNumOfResourceFork (short theRefNum)
{
	FCBPBRec		myFCBPBRec;
	Boolean			myIsResFork = false;
	OSErr			myErr = noErr;
	
	myFCBPBRec.ioCompletion = NULL;
	myFCBPBRec.ioNamePtr = NULL;
	myFCBPBRec.ioVRefNum = 0;
	myFCBPBRec.ioRefNum = theRefNum;
	myFCBPBRec.ioFCBIndx = 0;
	
	myErr = PBGetFCBInfoSync(&myFCBPBRec);
	if (myErr == noErr)
		if (myFCBPBRec.ioFCBFlags & kioFCBResourceMask)
			myIsResFork = true;
	
	return(myIsResFork);
}


//////////
//
// QTInfo_EditAnnotation
// Allow the user to enter or edit the annotation of the specified sort. Return a Boolean value
// that indicates whether the user clicked the OK button in the displayed dialog box and the text was
// successfully added to the movie (it may of course have been the same text already in the movie).
//
//////////

Boolean QTInfo_EditAnnotation (Movie theMovie, OSType theType)
{
	DialogPtr		myDialog = NULL;
	short 			myItem;
	short 			mySavedResFile;
	GrafPtr			mySavedPort;
	Handle			myHandle = NULL;
	short			myItemKind;
	Handle			myItemHandle;
	UserData		myUserData = NULL;
	Rect			myItemRect;
	Str255			myString;
	Boolean			myIsChanged = false;
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

	// get the movie user data
	myUserData = GetMovieUserData(theMovie);
	if (myUserData == NULL)
		goto bail;

	//////////
	//
	// create the dialog box in which the user will add or edit the annotation
	//
	//////////

	myDialog = GetNewDialog(kEditTextResourceID, NULL, (WindowPtr)-1L);
	if (myDialog == NULL)
		goto bail;

	MacSetPort(GetDialogPort(myDialog));
	
	SetDialogDefaultItem(myDialog, kEditTextItemOK);
	SetDialogCancelItem(myDialog, kEditTextItemCancel);
	
	// get a string for the specified annotation type
	switch (theType) {
		case kUserDataTextFullName:
			GetIndString(myString, kTextKindsResourceID, kTextKindsFullName);
			break;
	
		case kUserDataTextCopyright:
			GetIndString(myString, kTextKindsResourceID, kTextKindsCopyright);
			break;
	
		case kUserDataTextInformation:
			GetIndString(myString, kTextKindsResourceID, kTextKindsInformation);
			break;
	}
	
	GetDialogItem(myDialog, kEditTextItemEditLabel, &myItemKind, &myItemHandle, &myItemRect);
	SetDialogItemText(myItemHandle, myString);

	//////////
	//
	// set the current annotation of the specified type, if it exists
	//
	//////////

	myHandle = NewHandleClear(4);
	if (myHandle != NULL) {
		myErr = GetUserDataText(myUserData, myHandle, theType, 1, GetScriptManagerVariable(smRegionCode));
		if (myErr == noErr) {
			QTInfo_TextHandleToPString(myHandle, myString);
			GetDialogItem(myDialog, kEditTextItemEditBox, &myItemKind, &myItemHandle, &myItemRect);
			SetDialogItemText(myItemHandle, myString);
			SelectDialogItemText(myDialog, kEditTextItemEditBox, 0, myString[0]);
		}
		
		DisposeHandle(myHandle);
	}

	MacShowWindow(GetDialogWindow(myDialog));
	
	//////////
	//
	// display and handle events in the dialog box until the user clicks OK or Cancel
	//
	//////////
	
	do {
		ModalDialog(gModalFilterUPP, &myItem);
	} while ((myItem != kEditTextItemOK) && (myItem != kEditTextItemCancel));
	
	//////////
	//
	// handle the selected button
	//
	//////////
	
	if (myItem != kEditTextItemOK)
		goto bail;
	
	// retrieve the edited text
	myHandle = NewHandleClear(4);
	if (myHandle != NULL) {
		GetDialogItem(myDialog, kEditTextItemEditBox, &myItemKind, &myItemHandle, &myItemRect);
		GetDialogItemText(myItemHandle, myString);
		QTInfo_PStringToTextHandle(myString, myHandle);
		myErr = AddUserDataText(myUserData, myHandle, theType, 1, GetScriptManagerVariable(smRegionCode));
		myIsChanged = (myErr == noErr);
		DisposeHandle(myHandle);
	}

bail:	
	// restore the previous resource file and graphics port
	MacSetPort(mySavedPort);
	UseResFile(mySavedResFile);
	
	if (myDialog != NULL)
		DisposeDialog(myDialog);

	return(myIsChanged);
}


//////////
//
// QTInfo_TextHandleToPString
// Copy the characters in the specified handle into the specified Pascal string.
//
//////////

void QTInfo_TextHandleToPString (Handle theHandle, Str255 theString)
{
	short		myCount;
	
	myCount = GetHandleSize(theHandle);
	if (myCount > 255)
		myCount = 255;

	theString[0] = myCount;
	BlockMoveData(*theHandle, &(theString[1]), myCount);
}


//////////
//
// QTInfo_PStringToTextHandle
// Copy the characters in the specified Pascal string into the specified handle.
//
//////////

void QTInfo_PStringToTextHandle (Str255 theString, Handle theHandle)
{
	SetHandleSize(theHandle, theString[0]);
	if (GetHandleSize(theHandle) != theString[0])
		return;

	BlockMoveData(&(theString[1]), *theHandle, theString[0]);
}