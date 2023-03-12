//////////
//
//	File:		QTHintMovies.c
//
//	Contains:	Sample code for adding hint tracks to a QuickTime movie.
//
//	Written by:	Tim Monroe
//
//	Copyright:	© 1998-1999 by Apple Computer, Inc., all rights reserved.
//
//	Change History (most recent first):
//
//	   <5>	 	03/03/99	rtm		removed QTHints_HintMovieUsingHinterComponent
//	   <4>	 	11/04/98	rtm		reworked code there and there; verified on Mac and Windows
//	   <3>	 	11/03/98	rtm		added code for configuring exporter settings
//	   <2>	 	10/30/98	rtm		added comments; revised code slightly
//	   <1>	 	10/25/98	rtm		first file
//	 
//	A QuickTime movie that is to be streamed should contain a "hint track" for each media track in the movie.
//	A hint track contains information that assists the streaming server in the process of forming and timing
//	network packets. These hint tracks essentially free the server from having to know the details of network
//	protocols or media-specific codecs, thereby reducing run-time processing. This mechanism also allows the
//	server to stream new codec and network protocol types without modification (once they can be hinted).
//
//	Movie hinting is ultimately performed by a track hinter component, which contains code for a specific
//	network protocol (for example, rtp). The track hinter component calls a media packetizer component to hint
//	a specific media and codec type (for example, a video track compressed with JPEG compression). Track hinter
//	components operate on a per-track basis.
//
//	You can also hint an entire movie by calling the hinter movie export component. The hinter movie export
//	component insulates your application from having to call the track hinter component on each media track
//	and provides most of the services needed to create hinted movies. In all likelihood, you'll want to use
//	the hinter movie export component to add hint tracks to existing movies. See the function
//	QTHints_HintMovieUsingExportComponent for sample code that uses the hinter movie export component. Here
//	is one easy way to call QTHints_HintMovieUsingExportComponent for an open movie, myMovie:
//
//			StandardFileReply		myReply;
//				
//			StandardPutFile("\pSave Hinted Movie as:" , "\pHinted.mov", &myReply); 
//			if (myReply.sfGood)
//				QTHints_HintMovieUsingExportComponent(myMovie, &myReply.sfFile, true);
//
//	Finally, you can just use the Movie Toolbox's ConvertMovieToFile function, without specifying any export
//	component, to display the export settings dialog box, which allows the user to export a hinted movie. See
//	the function QTHints_HintMovieUsingToolbox for sample code that does this. (Of course, the user is free to
//	select some other export option, so you can't be sure he/she will actually create a hinted movie....)
//
//	This sample code snippet also illustrates how to configure the hinter movie export component, using either
//	MovieExportDoUserDialog to display the component's settings dialog or MovieExportSetSettingsFromAtomContainer
//	to restore some previously-saved settings from an atom container. See QTHints_HintMovieUsingExportComponent
//	for details.
//
//////////

#include "QTHintMovies.h"

extern ConstStr255Param				gSettingsFileName;			// the name of our settings preferences file


//////////
//
// QTHints_HintMovieUsingToolbox
// Add a hint track to a QuickTime movie, using the Movie Toolbox.
//
//////////

OSErr QTHints_HintMovieUsingToolbox (Movie theMovie, FSSpecPtr theFSSpecPtr)
{
	long						myFlags = 0L;
	OSErr						myErr = noErr;

	myFlags = createMovieFileDeleteCurFile | showUserSettingsDialog | movieFileSpecValid;
	
	// set the default progress procedure
	SetMovieProgressProc(theMovie, (MovieProgressUPP)-1L, 0);
	
	// export the movie into a file
	myErr = ConvertMovieToFile(	theMovie,				// the movie to convert
								NULL,					// all tracks in the movie
								theFSSpecPtr,			// the output file
								MovieFileType,			// the output file type
								FOUR_CHAR_CODE('TVOD'),	// the output file creator
								smSystemScript,			// the script
								NULL, 					// no resource ID to be returned
								myFlags,				// conversion flags
								NULL);					// no specific component

	return(myErr);
}


//////////
//
// QTHints_HintMovieUsingExportComponent
// Add a hint track to a QuickTime movie, using the hinter movie export component.
//
// The theFSSpecPtr parameter is the address of an FSSpec record for the output hinted
// movie. The thePromptUser parameter determines whether we display the movie exporter
// settings dialog box to allow the user to select export options (true) or whether we
// try to read the export options from an existing preferences file (false).
//
//////////

OSErr QTHints_HintMovieUsingExportComponent (Movie theMovie, FSSpecPtr theFSSpecPtr, Boolean thePromptUser)
{
	ComponentDescription		myCompDesc;
	MovieExportComponent		myExporter = NULL;
	long						myFlags = createMovieFileDeleteCurFile | movieFileSpecValid;
	FSSpec						myPrefsFile;
	ComponentResult				myErr = badComponentType;

	// find and open a movie export component that can hint a movie file
	myCompDesc.componentType = MovieExportType;
	myCompDesc.componentSubType = MovieFileType;
	myCompDesc.componentManufacturer = FOUR_CHAR_CODE('hint');
	myCompDesc.componentFlags = 0;
	myCompDesc.componentFlagsMask = 0;
	myExporter = OpenComponent(FindNextComponent(NULL, &myCompDesc));
	if (myExporter == NULL)
		goto bail;

	// use the default progress procedure
	SetMovieProgressProc(theMovie, (MovieProgressUPP)-1L, 0);

	// get the preferences file for this application
	QTHints_GetPrefsFileSpec(&myPrefsFile, (void *)theFSSpecPtr);
	
	// read existing movie exporter settings from a file; if we aren't going to prompt
	// the user for exporter settings, these stored settings will be used; otherwise,
	// these stored settings will be used as initial values in the settings dialog box
	QTUtils_GetExporterSettingsFromFile(myExporter, &myPrefsFile);
	
	if (thePromptUser) {
		Boolean		myCancelled = false;
		
		// display a dialog box to prompt the user for desired movie exporter settings		
		myErr = MovieExportDoUserDialog(myExporter, theMovie, NULL, 0, 0, &myCancelled);
		if (myCancelled)
			goto bail;
		
		// save the existing settings into our preferences file
		QTUtils_SaveExporterSettingsInFile(myExporter, &myPrefsFile);
	}
		
	// export the movie into a file
	myErr = ConvertMovieToFile(	theMovie,				// the movie to convert
								NULL,					// all tracks in the movie
								theFSSpecPtr,			// the output file
								MovieFileType,			// the output file type
								FOUR_CHAR_CODE('TVOD'),	// the output file creator
								smSystemScript,			// the script
								NULL, 					// no resource ID to be returned
								myFlags,				// conversion flags
								myExporter);			// hinter movie export component

bail:
	// close the movie export component
	if (myExporter != NULL)
		CloseComponent(myExporter);

	return((OSErr)myErr);
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Hint track utilities.
//
// (These functions aren't currently used in this sample.)
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////

//////////
//
// QTHints_GetIndHintTrack
// Find the hint track in the specified movie that has the specified index.
//
//////////

Track QTHints_GetIndHintTrack (Movie theMovie, long theIndex)
{
	return(GetMovieIndTrackType(theMovie, theIndex, FOUR_CHAR_CODE('hint'), movieTrackMediaType));
}


//////////
//
// QTHints_GetIndHintedTrack
// Find the track in the specified movie for which the specified hint track is a hint track;
// typically, a hint track is a hint track for only one original track, but a hint track can
// refer to more than one original track; hence the theIndex parameter.
//
//////////

Track QTHints_GetIndHintedTrack (Track theHintTrack, long theIndex)
{
	return(GetTrackReference(theHintTrack, FOUR_CHAR_CODE('hint'), theIndex));
}


//////////
//
// QTHints_MovieHasHintTrack
// Does the specified movie contain a hint track?
//
//////////

Boolean QTHints_MovieHasHintTrack (Movie theMovie)
{
	Boolean		isHinted = false;
	Track		myHintTrack = NULL;
	
	// look for a hint track
	myHintTrack = QTHints_GetIndHintTrack(theMovie, 1);
	
	// if a hint track exists, make sure that it actually refers to an original media track
	if (myHintTrack != NULL)
		if (QTHints_GetIndHintedTrack(myHintTrack, 1) != NULL)
			isHinted = true;

	return(isHinted);
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Settings utilities.
//
// Use these functions to save/retrieve movie exporter settings into/from a preferences file.
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////

//////////
//
// QTHints_GetPrefsFileSpec
// Fill in the specified FSSpec with info about this application's preferences file.
//
// The theRefCon parameter is a pointer to some application-specific data, which you
// might use to find the preferences file; here, we assume it's a pointer to an FSSpec
// for the output hinted file. We'll specify a preferences file in the same folder as
// the hinted file that has the name specified by the global variable gSettingsFileName.
//
//////////

OSErr QTHints_GetPrefsFileSpec (FSSpecPtr thePrefsSpecPtr, void *theRefCon)
{
	FSSpecPtr	myFSSpecPtr = (FSSpecPtr)theRefCon;
	OSErr		myErr = noErr;

	if (myFSSpecPtr == NULL)
		return(paramErr);
		
	myErr = FSMakeFSSpec(myFSSpecPtr->vRefNum, myFSSpecPtr->parID, gSettingsFileName, thePrefsSpecPtr);
	
	return(myErr);
}


//////////
//
// QTUtils_SaveExporterSettingsInFile
// Get the current settings of the specified movie exporter and save them into a file.
//
//////////

OSErr QTUtils_SaveExporterSettingsInFile (MovieExportComponent theExporter, FSSpecPtr theFSSpecPtr)
{	
	QTAtomContainer		myContainer = NULL;
	ComponentResult		myErr = noErr;
		
	myErr = MovieExportGetSettingsAsAtomContainer(theExporter, &myContainer);
	if (myErr != noErr)
		goto bail;
		
	myErr = QTUtils_WriteHandleToFile((Handle)myContainer, theFSSpecPtr);

bail:
	if (myContainer != NULL)
		QTDisposeAtomContainer(myContainer);
		
	return((OSErr)myErr);
}


//////////
//
// QTUtils_GetExporterSettingsFromFile
// Read the movie exporter settings saved in the specified file.
//
//////////

OSErr QTUtils_GetExporterSettingsFromFile (MovieExportComponent theExporter, FSSpecPtr theFSSpecPtr)
{	
	Handle				myHandle = NULL;
	ComponentResult		myErr = fnfErr;		// assume we cannot find the file
		
	myHandle = QTUtils_ReadHandleFromFile(theFSSpecPtr);
	if (myHandle == NULL)
		goto bail;
		
	myErr = MovieExportSetSettingsFromAtomContainer(theExporter, (QTAtomContainer)myHandle);
		
bail:
	if (myHandle != NULL)
		DisposeHandle(myHandle);
		
	return((OSErr)myErr);
}


//////////
//
// QTUtils_WriteHandleToFile
// Write the data in the specified handle into the specified file;
// if the file already exists, it is overwritten.
//
//////////

OSErr QTUtils_WriteHandleToFile (Handle theHandle, FSSpecPtr theFSSpecPtr)
{
	short			myRefNum = 0;
	short			myVolNum;
	long			mySize = 0;
	OSErr			myErr = paramErr;

	if (theHandle == NULL)
		goto bail;

	mySize = GetHandleSize(theHandle);
	if (mySize == 0)
		goto bail;

	HLock(theHandle);
	
	// delete the file;
	// if it doesn't exist yet, we'll get an error (fnfErr), which we just ignore
	myErr = FSpDelete(theFSSpecPtr);
	
	// create and open the file
	myErr = FSpCreate(theFSSpecPtr, kSettingsFileCreator, kSettingsFileType, smSystemScript);

	if (myErr == noErr)
		myErr = FSpOpenDF(theFSSpecPtr, fsRdWrPerm, &myRefNum);
	
	// position the file mark to the beginning of the file and write the data
	if (myErr == noErr)
		myErr = SetFPos(myRefNum, fsFromStart, 0);

	if (myErr == noErr)
		myErr = FSWrite(myRefNum, &mySize, *theHandle);

	if (myErr == noErr)
		myErr = SetFPos(myRefNum, fsFromStart, mySize);

	// resize the file to the number of bytes written
	if (myErr == noErr)
		myErr = SetEOF(myRefNum, mySize);
				
	// close the file			 
	if (myErr == noErr)		
		myErr = FSClose(myRefNum);

#if TARGET_OS_MAC	
	// flush the volume
	if (myErr == noErr)		
		myErr = GetVRefNum(myRefNum, &myVolNum);

	if (myErr == noErr)		
		myErr = FlushVol(NULL, myVolNum);
#endif	// TARGET_OS_MAC	

bail:
	HUnlock(theHandle);

	return(myErr);
}


//////////
//
// QTUtils_ReadHandleFromFile
// Read the data in the specified file into a new handle.
//
//////////

Handle QTUtils_ReadHandleFromFile (FSSpecPtr theFSSpecPtr)
{
	Handle			myHandle = NULL;
	short			myRefNum = 0;
	long			mySize = 0;
	OSErr			myErr = noErr;

	// open the file
	myErr = FSpOpenDF(theFSSpecPtr, fsRdWrPerm, &myRefNum);
	
	if (myErr == noErr)
		myErr = SetFPos(myRefNum, fsFromStart, 0);

	// get the size of the file data
	if (myErr == noErr)
		myErr = GetEOF(myRefNum, &mySize);
		
	// allocate a new handle
	if (myErr == noErr)
		myHandle = NewHandleClear(mySize);
	
	if (myHandle == NULL)
		goto bail;

	HLock(myHandle);

	// read the data from the file into the handle
	if (myErr == noErr)
		myErr = FSRead(myRefNum, &mySize, *myHandle);

bail:
	HUnlock(myHandle);
	
	if (myRefNum != 0)		
		FSClose(myRefNum);

	return(myHandle);
}




