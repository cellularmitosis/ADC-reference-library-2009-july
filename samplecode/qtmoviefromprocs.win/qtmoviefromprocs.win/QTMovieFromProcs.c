//////////
//
//	File:		QTMovieFromProcs.c
//
//	Contains:	Sample code for creating a QuickTime movie using application-defined procedures.
//
//	Written by:	Tim Monroe
//				Based on sample code in the QuickTime 3.0 Reference document.
//
//	Copyright:	© 1998-1999 by Apple Computer, Inc., all rights reserved.
//
//	Change History (most recent first):
//
//	   <8>	 	03/32/00	rtm		removed calls to Standard File Package (FSSpec now passed in);
//									made changes to run under CarbonLib
//	   <7>	 	02/09/99	rtm		minor tweaks; verified on Windows and Mac; added more comments
//	   <6>	 	02/05/99	rtm		added code to enumerate all components that can export-by-procs
//									add their names to the Test menu, and use the selected exporter
//	   <5>	 	02/02/99	rtm		reworked prompt and filename handling to remove "\p" sequences
//	   <4>	 	11/11/98	rtm		minor tweaks; added more comments
//	   <3>	 	11/06/98	rtm		incorporated settings file code from QTHintMovies.c
//	   <2>	 	10/19/98	rtm		further work; works fine on Mac and Windows
//	   <1>	 	10/16/98	rtm		first file
//	 
//	QuickTime 3.0 provides functions that allow you to create a movie using data supplied by one or more
//	application-defined procedures. In particular, you use the function MovieExportAddDataSource to add a
//	track-generating procedure and you use the function MovieExportFromProceduresToDataRef to do the actual
//	movie exporting.
//
//	In this example, we will create a QuickTime movie with a video track and an audio track; we generate the
//	video track by drawing a series of individual frames, and we generate the audio track by generating
//	10 seconds of silence.
//
//	This sample code finds all movie export components that can export using application-defined procedures,
//	adds their names to a menu, and then calls the appropriate exporter when the user selects a menu item. The
//	only real interesting code in the QTMenu_ functions is the code that enumerates the available exporters
//	that can export-by-procedures.
//
//	This sample code snippet also illustrates how to configure the movie export component, using either
//	MovieExportDoUserDialog to display the component's settings dialog or MovieExportSetSettingsFromAtomContainer
//	to restore some previously-saved settings from an atom container. Note that any settings configured by the
//	user or by a call to MovieExportSetSettingsFromAtomContainer may be overridden by the settings returned by 
//	your track property callback function (in this sample, the functions QTMoovProcs_AudioTrackPropertyProc and
//	QTMoovProcs_VideoTrackPropertyProc).
//
//////////


#include "QTMovieFromProcs.h"


// global variables
QTMoovProcsAudioDataRec				gAudioDataRec;
QTMoovProcsVideoDataRec				gVideoDataRec;

MovieExportGetPropertyUPP			gAudioPropProcUPP = NULL;
MovieExportGetDataUPP				gAudioDataProcUPP = NULL;

MovieExportGetPropertyUPP			gVideoPropProcUPP = NULL;
MovieExportGetDataUPP				gVideoDataProcUPP = NULL;

QTProcExportersInfo					gExporterInfo;

extern ConstStr255Param				gSettingsFileName;

#if TARGET_OS_WIN32
extern HWND							ghWnd;					// the MDI frame window; this window has the menu bar
#include "ComResource.h"
#endif


//////////
//
// QTMoovProcs_CreateMovieFromProcs
// Create a QuickTime movie using audio- and video-data generating procedures.
//
//////////

OSErr QTMoovProcs_CreateMovieFromProcs (FSSpecPtr theOutputFile, MovieExportComponent theExporter, Boolean thePromptUser)
{	
	Handle						myDataRef = NULL;
	FSSpec						myPrefsFile;
	OSErr						myErr = userCanceledErr;

	if (theOutputFile == NULL)
		goto bail;
	
	// add sources of audio and video data to the movie exporter;
	// we ignore errors here, since some exporters might be audio-only or video-only
	myErr = QTMoovProcs_AddAudioSourceToExporter(theExporter);
	myErr = QTMoovProcs_AddVideoSourceToExporter(theExporter);
		
	// create a data reference for the output movie file
	myDataRef = NewHandle(sizeof(Handle));
    if (myDataRef == NULL)
    	goto bail;

	QTNewAlias(theOutputFile, (AliasHandle *)&myDataRef, true);
	
	// get the preferences file for this application
	QTMoovProcs_GetPrefsFileSpec(&myPrefsFile, (void *)(theOutputFile));
	
	// read existing movie exporter settings from a file; if we aren't going to prompt
	// the user for exporter settings, these stored settings will be used; otherwise,
	// these stored settings will be used as initial values in the settings dialog box
	QTUtils_GetExporterSettingsFromFile(theExporter, &myPrefsFile);
	
	if (thePromptUser) {
		Boolean		myCancelled = false;
		
		// display a dialog box to prompt the user for desired movie exporter settings		
		myErr = MovieExportDoUserDialog(theExporter, NULL, NULL, 0, 0, &myCancelled);
		if (myCancelled)
			goto bail;
		
		// save the existing settings into our preferences file
		QTUtils_SaveExporterSettingsInFile(theExporter, &myPrefsFile);
	}
		
	// export the audio and video data to the data reference
	myErr = MovieExportFromProceduresToDataRef(theExporter, myDataRef, rAliasType);
	
bail:
	// dispose of the routine descriptors
	DisposeMovieExportGetPropertyUPP(gAudioPropProcUPP);
	DisposeMovieExportGetDataUPP(gAudioDataProcUPP);
	DisposeMovieExportGetPropertyUPP(gVideoPropProcUPP);
	DisposeMovieExportGetDataUPP(gVideoDataProcUPP);
	
	// dispose of any memory that we allocated before or during the export operation
    if (myDataRef != NULL)
    	DisposeHandle(myDataRef);
	
	if (gAudioDataRec.fSoundData != NULL)
		DisposePtr(gAudioDataRec.fSoundData);
		
	if (gAudioDataRec.fSoundDescription != NULL)
		DisposeHandle((Handle)gAudioDataRec.fSoundDescription);
		
	if (gVideoDataRec.fGWorld != NULL)
		DisposeGWorld(gVideoDataRec.fGWorld);
		
	if (gVideoDataRec.fImageDescription != NULL)
		DisposeHandle((Handle)gVideoDataRec.fImageDescription);
		
	return(myErr);
}


//////////
//
// QTMoovProcs_AddAudioSourceToExporter
// Attach a source for audio data to the specified movie exporter.
//
//////////

OSErr QTMoovProcs_AddAudioSourceToExporter (MovieExportComponent theExporter)
{	
	SoundDescriptionPtr			mySoundDescPtr = NULL;
	short						myIndex;
	OSErr						myErr = memFullErr;
		
	// create a buffer to hold the audio data
	gAudioDataRec.fSoundData = NewPtrClear(kSoundBufferSize);
	if (gAudioDataRec.fSoundData == NULL)
		goto bail;
	
	// our sound data format is 8-bit, offset binary, where a sample point is an 8-bit value
	// in the range 0 to 255 (0x00 to 0xFF); for silence, set all sample points to 128 (0x80)
	for (myIndex = 0; myIndex < kSoundBufferSize; myIndex++)
		gAudioDataRec.fSoundData[myIndex] = (UInt8)128;
	
	// create and configure a sound description record
	gAudioDataRec.fSoundDescription = (SoundDescriptionHandle)NewHandleClear(sizeof(SoundDescription));
	if (gAudioDataRec.fSoundDescription == NULL)
		goto bail;

	mySoundDescPtr = *(gAudioDataRec.fSoundDescription);
	mySoundDescPtr->descSize = sizeof(SoundDescription);
	mySoundDescPtr->dataFormat = k8BitOffsetBinaryFormat;
	mySoundDescPtr->numChannels = 1;
	mySoundDescPtr->sampleSize = 8;
	mySoundDescPtr->sampleRate = kAudioSampleRate << 16;
	
	// create UPPs for the two app-defined export functions
	gAudioPropProcUPP = NewMovieExportGetPropertyUPP(QTMoovProcs_AudioTrackPropertyProc);
	gAudioDataProcUPP = NewMovieExportGetDataUPP(QTMoovProcs_AudioTrackDataProc);
	
	myErr = MovieExportAddDataSource(theExporter,
										SoundMediaType,
										kAudioSampleRate,
										&gAudioDataRec.fTrackID,
										gAudioPropProcUPP,
										gAudioDataProcUPP,
										&gAudioDataRec);

bail:
	return(myErr);
}


//////////
//
// QTMoovProcs_AddVideoSourceToExporter
// Attach a source for video data to the specified movie exporter.
//
//////////

OSErr QTMoovProcs_AddVideoSourceToExporter (MovieExportComponent theExporter)
{	
	OSErr	myErr = noErr;

	gVideoDataRec.fGWorld = NULL;	
	gVideoDataRec.fImageDescription = NULL;
	
	// create UPPs for the two app-defined export functions
	gVideoPropProcUPP = NewMovieExportGetPropertyUPP(QTMoovProcs_VideoTrackPropertyProc);
	gVideoDataProcUPP = NewMovieExportGetDataUPP(QTMoovProcs_VideoTrackDataProc);
	
	myErr = MovieExportAddDataSource(theExporter,
										VideoMediaType,
										kVideoSampleRate,
										&gVideoDataRec.fTrackID,
										gVideoPropProcUPP,
										gVideoDataProcUPP,
										&gVideoDataRec);

	return(myErr);
}


//////////
//
// QTMoovProcs_AudioTrackPropertyProc
// Handle requests for information about the output audio data.
//
//////////

PASCAL_RTN OSErr QTMoovProcs_AudioTrackPropertyProc (void *theRefcon, long theTrackID, OSType thePropertyType, void *thePropertyValue)
{
#pragma unused(theRefcon, theTrackID)

	OSErr	myErr = noErr;
	
	switch (thePropertyType) {
	
		case scSoundSampleRateType:
			*(Fixed *)thePropertyValue = 32000L << 16;
			break;
			
		default:
			myErr = paramErr;	// non-zero value means: use default value provided by export component
			break;
	}
	
	return(myErr);
}


//////////
//
// QTMoovProcs_AudioTrackDataProc
// Provide the output audio data.
//
//////////

PASCAL_RTN OSErr QTMoovProcs_AudioTrackDataProc (void *theRefcon, MovieExportGetDataParams *theParams)
{
	QTMoovProcsAudioDataRecPtr	myAudioDataRecPtr;
	OSErr						myErr = noErr;
	
	myAudioDataRecPtr = (QTMoovProcsAudioDataRecPtr)theRefcon;

	// end the data after desired length of movie
	if (theParams->requestedTime > kAudioSampleRate * kMovieLengthInSeconds)
		return(eofErr);
	
	theParams->actualTime = theParams->requestedTime;
	theParams->dataPtr = myAudioDataRecPtr->fSoundData;
	theParams->dataSize = kSoundBufferSize;
	theParams->desc = (SampleDescriptionHandle)(myAudioDataRecPtr->fSoundDescription);
	theParams->descType = SoundMediaType;
	theParams->descSeed = 1;
	theParams->actualSampleCount = kSoundBufferSize;
	theParams->durationPerSample = 1;
	theParams->sampleFlags = 0L;
	
	return(myErr);
}


//////////
//
// QTMoovProcs_VideoTrackPropertyProc
// Handle requests for information about the output video data.
//
//////////

PASCAL_RTN OSErr QTMoovProcs_VideoTrackPropertyProc (void *theRefcon, long theTrackID, OSType thePropertyType, void *thePropertyValue)
{
#pragma unused(theRefcon, theTrackID)

	OSErr	myErr = noErr;
	
	switch (thePropertyType) {
		case movieExportWidth:
			*(Fixed *)thePropertyValue = kVideoFrameWidth << 16;
			break;
			
		case movieExportHeight:
			*(Fixed *)thePropertyValue = kVideoFrameHeight << 16;
			break;

		case scSpatialSettingsType: {
			SCSpatialSettings		*mySpatialSettings = thePropertyValue;
			
			mySpatialSettings->codecType = kJPEGCodecType;
			mySpatialSettings->codec = 0;
			mySpatialSettings->depth = 0;
			mySpatialSettings->spatialQuality = codecNormalQuality;
		}
			break;

		default:
			myErr = paramErr;	// non-zero value means: use default value provided by export component
			break;
	}
	
	return(myErr);
}


//////////
//
// QTMoovProcs_VideoTrackDataProc
// Provide the output audio data.
//
//////////

PASCAL_RTN OSErr QTMoovProcs_VideoTrackDataProc (void *theRefcon, MovieExportGetDataParams *theParams)
{	
	QTMoovProcsVideoDataRecPtr	myVideoDataRecPtr;
	PixMapHandle				myPixMap = NULL;
	Rect						myRect;		
	CGrafPtr					myOldPort = NULL;
	GDHandle					myOldDevice = NULL;
	Str255						myString;
	static long					myFrameNum = 0;
	OSErr						myErr = noErr;
	
	myVideoDataRecPtr = (QTMoovProcsVideoDataRecPtr)theRefcon;
	
	// end the data after desired length of movie
	if (theParams->requestedTime > kVideoSampleRate * kMovieLengthInSeconds)
		return(eofErr);
	
	// set the size of the video frame
	MacSetRect(&myRect, 0, 0, kVideoFrameWidth, kVideoFrameHeight);

	// if we haven't allocated a GWorld yet, do so now
	if (myVideoDataRecPtr->fGWorld == NULL) {
		NewGWorld(&(myVideoDataRecPtr->fGWorld), 32, &myRect, NULL, NULL, (GWorldFlags)0);
		if (myVideoDataRecPtr->fGWorld == NULL)
			return(memFullErr);
			
		myPixMap = GetGWorldPixMap(myVideoDataRecPtr->fGWorld);
		if (myPixMap == NULL)
			return(memFullErr);
		
		LockPixels(myPixMap);
		
		myErr = MakeImageDescriptionForPixMap(myPixMap, &(myVideoDataRecPtr->fImageDescription));
		if (myErr != noErr)
			goto bail;
	}
	
	GetGWorld(&myOldPort, &myOldDevice);
	SetGWorld(myVideoDataRecPtr->fGWorld, NULL);
	
	// draw a frame: white rectangle with black frame number centered horizontally in frame
	EraseRect(&myRect);
	ForeColor(whiteColor);
	PaintRect(&myRect);

	ForeColor(blackColor);
	
	NumToString(++myFrameNum, myString);
	MoveTo((short)(myRect.right / 2) - (StringWidth(myString) / 2), (short)(myRect.bottom / 2));
	TextSize((short)(myRect.bottom / 4));
	DrawString(myString);

	theParams->actualTime = theParams->requestedTime;
	theParams->dataPtr = GetPixBaseAddr(GetGWorldPixMap(myVideoDataRecPtr->fGWorld));
	theParams->dataSize = (**(myVideoDataRecPtr->fImageDescription)).dataSize;
	theParams->desc = (SampleDescriptionHandle)(myVideoDataRecPtr->fImageDescription);
	theParams->descType = VideoMediaType;
	theParams->descSeed = 1;
	theParams->actualSampleCount = 1;
	theParams->durationPerSample = kVideoFrameDuration;
	theParams->sampleFlags = 0L;

bail:
	// restore the original graphics port and device
	SetGWorld(myOldPort, myOldDevice);
	
	return(myErr);
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Menu utilities.
//
// Use these functions to add component names to a menu and to handle user selections in that menu. These
// exist mainly to illustrate how to find all movie export components that can export using application-
// defined procedures. 
//
// It's useful to remember that Macintosh menu items are numbered from 1, whereas Windows menu items are
// numbered from 0.
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////

//////////
//
// QTMenu_InitializeTestMenu
// Initialize the Test menu; add component names to it.
//
//////////

OSErr QTMenu_InitializeTestMenu (void)
{
	short			myIndex;
	short			myNumItems;

#if TARGET_OS_MAC
	gExporterInfo.fTestMenuHandle = GetMenuHandle(kTestMenuID);
	
	// remove any existing menu items
	myNumItems = CountMenuItems(gExporterInfo.fTestMenuHandle);
	for (myIndex = 0; myIndex < myNumItems; myIndex++)
		DeleteMenuItem(gExporterInfo.fTestMenuHandle, 1);
#elif TARGET_OS_WIN32
	gExporterInfo.fTestMenuHandle = GetSubMenu(GetMenu(ghWnd), WINDOWMENU - 1);

	// remove any existing menu items
	myNumItems = GetMenuItemCount(gExporterInfo.fTestMenuHandle);
	for (myIndex = 0; myIndex < myNumItems; myIndex++)
		RemoveMenu(gExporterInfo.fTestMenuHandle, 0, MF_BYPOSITION);
#endif

	gExporterInfo.fNextAvailIndex = 0;
	
	// add names of all components that can export-by-procedures to Test menu
	QTMenu_AddComponentNamesToMenu();
	
	return(noErr);
}


//////////
//
// QTMenu_AddComponentNamesToMenu
// Add the name of all movie export components that can export-by-procedures to the Test menu.
//
//////////

OSErr QTMenu_AddComponentNamesToMenu (void)
{
	Component				myComponent = NULL;
	Handle					myCompName = NewHandleClear(0);
	ComponentDescription	myCompDesc;
	
	// find all movie export components that can export-by-procedures
	do {
		myCompDesc.componentType = MovieExportType;
		myCompDesc.componentSubType = 0;
		myCompDesc.componentManufacturer = 0;
		myCompDesc.componentFlags = canMovieExportFromProcedures;
		myCompDesc.componentFlagsMask = canMovieExportFromProcedures;
	
		myComponent = FindNextComponent(myComponent, &myCompDesc);
		if (myComponent != NULL) {
			// get the component information
			GetComponentInfo(myComponent, &myCompDesc, myCompName, NULL, NULL);

			// add the component's name to the menu
#if TARGET_OS_MAC		
			MacAppendMenu(gExporterInfo.fTestMenuHandle, (StringPtr)*myCompName);
#elif TARGET_OS_WIN32
			AppendMenu(gExporterInfo.fTestMenuHandle, MF_STRING | MF_ENABLED, MENU_IDENTIFIER(kTestMenu, gExporterInfo.fNextAvailIndex + 1), p2cstr((StringPtr)*myCompName));
#endif			
			// keep track of this component
			gExporterInfo.fComponentType[gExporterInfo.fNextAvailIndex] = myCompDesc.componentType;
			gExporterInfo.fComponentSubType[gExporterInfo.fNextAvailIndex] = myCompDesc.componentSubType;
			gExporterInfo.fComponentManufacturer[gExporterInfo.fNextAvailIndex] = myCompDesc.componentManufacturer;
			
			gExporterInfo.fNextAvailIndex++;
		}
	} while (myComponent != NULL);
	
	if (myCompName != NULL)
		DisposeHandle(myCompName);
	
	return(noErr);
}


//////////
//
// QTMenu_HandleTestMenu
// Handle user selection of items in the Test menu.
//
//////////

OSErr QTMenu_HandleTestMenu (UInt16 theMenuItem)
{
	MovieExportComponent		myExporter = NULL;
	ComponentDescription		myCompDesc;
	short						myIndex; 
	OSErr						myErr = noErr;
	
	myIndex = MENU_ITEM(theMenuItem) - 1;
	if ((myIndex < 0) || (myIndex >= gExporterInfo.fNextAvailIndex))
		return(paramErr);
	
	myCompDesc.componentType = gExporterInfo.fComponentType[myIndex];
	myCompDesc.componentSubType = gExporterInfo.fComponentSubType[myIndex];
	myCompDesc.componentManufacturer = gExporterInfo.fComponentManufacturer[myIndex];
	myCompDesc.componentFlags = canMovieExportFromProcedures;
	myCompDesc.componentFlagsMask = canMovieExportFromProcedures;

	// open the selected movie export component
	myExporter = OpenComponent(FindNextComponent(NULL, &myCompDesc));
	if (myExporter == NULL)
		return(badComponentType);
		
	// create the movie using application-defined procedures
	myErr = QTMoovProcs_CreateMovieFromProcs(myExporter, true);
	
	// close the movie export component
	if (myExporter != NULL)
		CloseComponent(myExporter);
		
	return(myErr);
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
// QTMoovProcs_GetPrefsFileSpec
// Fill in the specified FSSpec with info about this application's preferences file.
//
// The theRefCon parameter is a pointer to some application-specific data, which you
// might use to find the preferences file; here, we assume it's a pointer to an FSSpec
// for the output hinted file. We'll specify a preferences file in the same folder as
// the hinted file that has the name specified by the global variable gSettingsFileName.
//
//////////

OSErr QTMoovProcs_GetPrefsFileSpec (FSSpecPtr thePrefsSpecPtr, void *theRefCon)
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
#if TARGET_OS_MAC	
	short			myVolNum;
#endif
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
#endif	

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



