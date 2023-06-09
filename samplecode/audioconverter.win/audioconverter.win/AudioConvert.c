//////////
//
//	File:		AudioConvert.c
//
//	Contains:	Sample code showing how to use the Sound Manager's SoundConverter routines. 
//
//	Written by:	Jim Reekes
//	Revised by: Tim Monroe
//
//	Copyright:	� 1999 by Apple Computer, Inc., all rights reserved.
//
//	Change History (most recent first):
//
//	   <5>	 	03/29/00	rtm		made changes to get things running under CarbonLib
//	   <4>	 	07/13/99	rtm		final adjustments; fixed cross-platform problems resulting from using
//									kSoundNotCompressed ('NONE') format (thanks to EJ Campbell!); see NOTE below
//	   <3>	 	07/07/99	rtm		further work; revised coding style; added some comments
//	   <2>	 	03/12/99	rtm		added (theDialog == NULL) check to AudConv_SFGetDialogHook to prevent
//									crashing on Windows; simplified UPP handling
//	   <1>	 	03/09/99	rtm		first file from jr; added some comments; added Windows-specific code
//									and endian macros
//
//	This code shows how to use the SoundConverter routines (introduced in Sound Manager version 3.2). It
//	provides routines that convert an AIFF or AIFF-C file into either a QuickTime movie or into an AIFF or
//	AIFF-C file. The user can select the desired target format and settings in a dialog box displayed by
//	a call to SCRequestImageSettings.
//
//	You should note in particular that this sample code shows how to handle the compression parameters chunk
//	that might be present in an AIFF or AIFF-C file. The compression parameters chunk contains parameters
//	that are required to decompress the audio data in the file.
//
//	NOTE: Do not use kSoundNotCompressed as an output format for multi-byte audio data if you want to create
//	movies that are to be shared across platforms. QuickTime needs to know the endianness of the sound data,
//	and kSoundNotCompressed does not indicate whether the data is big- or little-endian. Since in this sample
//	code we are reading our data from an AIFF file, we specify that any non-compressed multi-byte data is of
//	the format k16BitBigEndianFormat. 
//
//////////

#include "AudioConvert.h"


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
	SoundComponentData		myDestInfo;
#if TARGET_API_MAC_CARBON
	NavReplyRecord			myReply;
	NavDialogOptions		myDialogOptions;
	AEKeyword				myKeyword;
	DescType				myActualType;
	Size					myActualSize = 0;
	NavEventUPP				myEventUPP = NewNavEventUPP(AudConv_HandleNavEvent);
	NavObjectFilterUPP		myFilterProc = NewNavObjectFilterUPP(AudConv_NavObjectFilterProc);
#else
	OSType					myTypeList[2] = {kQTFileTypeAIFF, kQTFileTypeAIFC};
	StandardFileReply		myReply;
#endif
	FSSpec					myInputFSSpec;
	FSSpec					myOutputFSSpec;
	Boolean					myOutputAIFF;
	Str255					myAppName;
	OSErr					myErr = noErr;

#if TARGET_OS_WIN32
	char					myFileName[MAX_PATH];
	DWORD					myLength;
	short 					myAppResFile = -1;					// file reference number for this application's resource file
	
	InitializeQTML(0L);											// initialize QuickTime Media Layer
#endif	

#if TARGET_OS_MAC
#if !TARGET_API_MAC_CARBON	
	MaxApplZone();												// init everything
	InitGraf(&qd.thePort);
	InitFonts();
	FlushEvents(everyEvent,0);
	InitWindows();
	InitMenus();
	InitDialogs(NULL);
	TEInit();
#endif	
	InitCursor();
#endif	
	
	myErr = EnterMovies();
	FailIf(myErr != noErr, Exit);

#if TARGET_OS_WIN32
	// open the application's resource file, if it exists
	myLength = GetModuleFileName(NULL, myFileName, MAX_PATH);	// NULL means: the current process
	if (myLength != 0) {
		FSSpec				myFSSpec;
		
		NativePathNameToFSSpec(myFileName, &myFSSpec, 0L);

		myAppResFile = FSpOpenResFile(&myFSSpec, fsRdWrPerm);
		if (myAppResFile != -1)
			UseResFile(myAppResFile);
	}
#endif
		
	GetIndString(myAppName, rStringsResID, rAppNameStringIndex);
	myOutputAIFF = false;										// set default file format to 'MooV' file

#if TARGET_API_MAC_CARBON
	// we are running under Carbon; use Navigation Services
	
	//////////
	//
	// elicit an AIFF or an AIFC file from the user
	//
	//////////
	
	// specify the options for the dialog box
	NavGetDefaultDialogOptions(&myDialogOptions);
	myDialogOptions.dialogOptionFlags -= kNavNoTypePopup;
	myDialogOptions.dialogOptionFlags -= kNavAllowMultipleFiles;
	BlockMoveData(myAppName, myDialogOptions.clientName, myAppName[0] + 1);

	myErr = NavGetFile(NULL, &myReply, &myDialogOptions, myEventUPP, NULL, (NavObjectFilterUPP)myFilterProc, NULL, NULL);
	if ((myErr == noErr) && myReply.validRecord) {
		StringPtr 				myPrompt = AudConv_ConvertCToPascalString(kACSavePrompt);
		StringPtr 				myFileName = AudConv_ConvertCToPascalString(kMovieFileSaveName);
		NavMenuItemSpecHandle	myMenuItemHandle = NULL;
		
		// get the FSSpec for the selected file
		myErr = AEGetNthPtr(&(myReply.selection), 1, typeFSS, &myKeyword, &myActualType, &myInputFSSpec, sizeof(FSSpec), &myActualSize);
		NavDisposeReply(&myReply);

		// elicit a file to save the converted sound data into;
		// we want to allow the user to save as a movie or as an AIFF file
		myMenuItemHandle = (NavMenuItemSpecHandle)NewHandleClear(sizeof(NavMenuItemSpec));
		if (myMenuItemHandle != NULL) {
			(**myMenuItemHandle).version = kNavMenuItemSpecVersion;
			(**myMenuItemHandle).menuCreator = FOUR_CHAR_CODE('AuCo');
			(**myMenuItemHandle).menuType = kQTFileTypeAIFF;
			GetIndString((**myMenuItemHandle).menuItemName, rStringsResID, rAIFFMenuLabelIndex);
		}
		
		NavGetDefaultDialogOptions(&myDialogOptions);
		myDialogOptions.dialogOptionFlags -= kNavAllowStationery;
		myDialogOptions.dialogOptionFlags += kNavDontAutoTranslate;
		myDialogOptions.popupExtension = myMenuItemHandle;
		BlockMoveData(myFileName, myDialogOptions.savedFileName, myFileName[0] + 1);
		BlockMoveData(myAppName, myDialogOptions.clientName, myAppName[0] + 1);
		BlockMoveData(myPrompt, myDialogOptions.message, myPrompt[0] + 1);
		
		myErr = NavPutFile(NULL, &myReply, &myDialogOptions, myEventUPP, MovieFileType, sigMoviePlayer, &myOutputAIFF);
		if ((myErr == noErr) && myReply.validRecord) {
			
			// get the FSSpec for the selected file
			myErr = AEGetNthPtr(&(myReply.selection), 1, typeFSS, &myKeyword, &myActualType, &myOutputFSSpec, sizeof(FSSpec), &myActualSize);
			
			if (myReply.replacing) {
				myErr = FSpDelete(&myOutputFSSpec);
				FailIf(myErr != noErr, Failure);
			}

			myErr = AudConv_GetOutputFormat(&myDestInfo);
			if (myErr == noErr) {
				if (myOutputAIFF)
					myErr = AudConv_ConvertToAIFF(&myInputFSSpec, &myOutputFSSpec, &myDestInfo);
				else
					myErr = AudConv_ConvertToMovie(&myInputFSSpec, &myOutputFSSpec, &myDestInfo);
				FailIf(myErr != noErr, Failure);
			}
			
			if (myErr == userCanceledErr)
				myErr = noErr;
			FailIf(myErr != noErr, Failure);
			
			NavDisposeReply(&myReply);
		}

		free(myPrompt);
		free(myFileName);
	}
	
	DisposeNavEventUPP(myEventUPP);
	DisposeNavObjectFilterUPP(myFilterProc);

#else
	// we are NOT running under Carbon; use Standard File Package
	
	//////////
	//
	// elicit an AIFF or an AIFC file from the user
	//
	//////////

	StandardGetFile(NULL, 2, myTypeList, &myReply);
	if (myReply.sfGood) {
		DlgHookYDUPP		myDlgHookUPP = NULL;
		Point				myTopLeft;
		
		myDlgHookUPP = NewDlgHookYDProc(AudConv_SFGetDialogHook);
		
		myInputFSSpec = myReply.sfFile;							// save input file spec

		// elicit a file to save the converted sound data into
		myTopLeft.v = -1;										// -1,-1 means to center the dialog
		myTopLeft.h = -1;
		CustomPutFile(NULL, c2pstr(kAIFFFileSaveName), &myReply, rCustomGetFileDialog, myTopLeft, myDlgHookUPP, NULL, NULL, NULL, &myOutputAIFF);
		if (myReply.sfGood) {
			myOutputFSSpec = myReply.sfFile;						// save output file spec
			
			if (myReply.sfReplacing) {
				myErr = FSpDelete(&myOutputFSSpec);
				FailIf(myErr != noErr, Failure);
			}

			myErr = AudConv_GetOutputFormat(&myDestInfo);
			if (myErr == noErr) {
				if (myOutputAIFF)
					myErr = AudConv_ConvertToAIFF(&myInputFSSpec, &myOutputFSSpec, &myDestInfo);
				else
					myErr = AudConv_ConvertToMovie(&myInputFSSpec, &myOutputFSSpec, &myDestInfo);
				FailIf(myErr != noErr, Failure);
			}
			
			if (myErr == userCanceledErr)
				myErr = noErr;
			FailIf(myErr != noErr, Failure);
		}
	}
#endif

Failure:
	ExitMovies();
Exit:

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
// AudConv_ConvertToAIFF 
// Read the source AIFF file and convert to the destination format; output a new AIFF file.
// The new file must include the compression parameters chunk, if used.
//
//////////

OSErr AudConv_ConvertToAIFF (ConstFSSpecPtr theInputFSSpec, ConstFSSpecPtr theOutputFSSpec, SoundComponentDataPtr theDestInfo)
{
	SoundComponentData		mySourceInfo;
	Handle					mySourceCompParamsHandle = NULL;
	Handle					myDestHandle = NULL;
	Handle					myDestCompParamsHandle = NULL;
	long					mySourceOffset;
	long					mySourceSize;
	short					myInputFile;
	short					myOutputFile;
	long					myCount;
	OSErr					myErr = noErr;

	// open the source file and parse it
	myErr = FSpOpenDF(theInputFSSpec, fsRdPerm, &myInputFile);
	FailIf(myErr != noErr, NoInputFile);

	myErr = AudConv_GetDataFromAIFF(myInputFile, &mySourceInfo, &mySourceOffset, &mySourceSize, &mySourceCompParamsHandle);
	FailIf(myErr != noErr, NoData);

	// create the buffer into which we read the source data
	mySourceInfo.buffer = (Byte *)NewPtrClear(mySourceSize);
	FailWithAction(mySourceInfo.buffer == NULL, myErr = MemError(), NoSource);

	// create the buffer to hold the converted data
	myDestHandle = NewHandle(0);
	FailWithAction(myDestHandle == NULL, myErr = MemError(), NoDest);

	// read the source data from the source file into the source data buffer
	myErr = SetFPos(myInputFile, fsFromStart, mySourceOffset);
	FailIf(myErr != noErr, InputFileErr);

	myErr = FSRead(myInputFile, &mySourceSize, mySourceInfo.buffer);
	FailIf(myErr != noErr, InputFileErr);

	// create the output AIFF file, to hold the converted data
	myErr = FSpCreate(theOutputFSSpec, sigMoviePlayer, AIFFID, smSystemScript);
	FailIf(myErr != noErr, CreateOutputErr);

	myErr = FSpOpenDF(theOutputFSSpec, fsRdWrPerm, &myOutputFile);
	FailIf(myErr != noErr, OutputFileErr);

	// convert the source data into the output data
	myErr = AudConv_ConvertAudioIntoHandle(&mySourceInfo, mySourceCompParamsHandle, myDestHandle, theDestInfo, &myDestCompParamsHandle, NULL);
	FailIf(myErr != noErr, ConvertErr);

	// put the output data into the output AIFF file
	myErr = AudConv_PrepFileAsAIFF(myOutputFile, theDestInfo, myDestCompParamsHandle);
	FailIf(myErr != noErr, ConvertErr);

	HLock(myDestHandle);
	myCount = GetHandleSize(myDestHandle);
	myErr = FSWrite(myOutputFile, &myCount, *myDestHandle);
	HUnlock(myDestHandle);
	FailIf(myErr != noErr, ConvertErr);

	myErr = AudConv_FinishFileAsAIFF(myOutputFile, theDestInfo->sampleCount, myCount);
	FailIf(myErr != noErr, ConvertErr);

	// close the input and output files
	myErr = FSClose(myInputFile);
	FailIf(myErr != noErr, ConvertErr);

	myErr = FSClose(myOutputFile);
	FailIf(myErr != noErr, ConvertErr);

	// dispose of any storage we allocated and no longer need
	if (mySourceInfo.buffer != NULL)
		DisposePtr((Ptr)mySourceInfo.buffer);
	
	if (myDestHandle != NULL)
		DisposeHandle(myDestHandle);
		
	if (mySourceCompParamsHandle != NULL)
		DisposeHandle(mySourceCompParamsHandle);
		
	if (myDestCompParamsHandle != NULL)
		DisposeHandle(myDestCompParamsHandle);
	
	return(noErr);

ConvertErr:
	FSClose(myOutputFile);
	
OutputFileErr:
	FSpDelete(theOutputFSSpec);
	
CreateOutputErr:
InputFileErr:
	if (myDestHandle != NULL)
		DisposeHandle(myDestHandle);
	
NoDest:
	if (mySourceInfo.buffer != NULL)
		DisposePtr((Ptr)mySourceInfo.buffer);
	
NoSource:
	if (mySourceCompParamsHandle != NULL)
		DisposeHandle(mySourceCompParamsHandle);
		
NoData:
	FSClose(myInputFile);
	
NoInputFile:
	return(myErr);
}


//////////
//
// AudConv_ConvertToMovie
// Read the source AIFF file and convert to the destination format; output a new movie file.
// The new file must include the compression parameters chunk, if used.
//
//////////

OSErr AudConv_ConvertToMovie (ConstFSSpecPtr theInputFSSpec, ConstFSSpecPtr theOutputFSSpec, SoundComponentDataPtr theDestInfo)
{
	SoundComponentData		mySourceInfo;
	Handle					mySourceCompParamsHandle = NULL;
	Handle					myDestHandle = NULL;
	Handle					myDestCompParamsHandle = NULL;
	long					mySourceOffset;
	long					mySourceSize;
	short					myInputFile;
	CompressionInfo			myDestCompInfo;
	Movie					myMovie;
	Track					myTrack;
	short					myResRefNum = -1;
	short					myResID = movieInDataForkResID;
	long					myFlags = createMovieFileDeleteCurFile | createMovieFileDontCreateResFile;
	OSErr					myErr = noErr;

	// open the source file and parse it
	myErr = FSpOpenDF(theInputFSSpec, fsRdPerm, &myInputFile);
	FailIf(myErr != noErr, NoInputFile);

	myErr = AudConv_GetDataFromAIFF(myInputFile, &mySourceInfo, &mySourceOffset, &mySourceSize, &mySourceCompParamsHandle);
	FailIf(myErr != noErr, NoData);

	// create the buffer into which we read the source data
	mySourceInfo.buffer = (Byte *)NewPtrClear(mySourceSize);
	FailWithAction(mySourceInfo.buffer == NULL, myErr = MemError(), NoSource);

	// create the buffer to hold the converted data
	myDestHandle = NewHandle(0);
	FailWithAction(myDestHandle == NULL, myErr = MemError(), NoDest);

	// read the source data from the source file into the source data buffer
	myErr = SetFPos(myInputFile, fsFromStart, mySourceOffset);
	FailIf(myErr != noErr, InputFileErr);

	myErr = FSRead(myInputFile, &mySourceSize, mySourceInfo.buffer);
	FailIf(myErr != noErr, InputFileErr);

	// create the output movie file, to hold the converted data
	myErr = CreateMovieFile(theOutputFSSpec, sigMoviePlayer, smCurrentScript, myFlags, &myResRefNum, &myMovie);
	FailIf(myErr != noErr, CreateOutputErr);

	// convert the source data into the output data
	myErr = AudConv_ConvertAudioIntoHandle(&mySourceInfo, mySourceCompParamsHandle, myDestHandle, theDestInfo, &myDestCompParamsHandle, &myDestCompInfo);
	FailIf(myErr != noErr, CompressErr);

	// put the output data into a track in the output movie file
	myTrack = NewMovieTrack(myMovie, 0, 0, kFullVolume);
	myErr = GetMoviesError();
	FailIf(myErr != noErr, CompressErr);

	myErr = AudConv_PutAudioIntoTrack(myTrack, myDestHandle, theDestInfo, myDestCompParamsHandle, &myDestCompInfo);
	FailIf(myErr != noErr, CompressErr);

	myErr = AddMovieResource(myMovie, myResRefNum, &myResID, NULL);
	FailIf(myErr != noErr, CompressErr);

	// close the movie and movie file
	if (myResRefNum != -1)
		CloseMovieFile(myResRefNum);
	
	if (myMovie != NULL)
		DisposeMovie(myMovie);

	// dispose of any storage we allocated and no longer need
	if (mySourceInfo.buffer != NULL)
		DisposePtr((Ptr)mySourceInfo.buffer);
	
	if (myDestHandle != NULL)
		DisposeHandle(myDestHandle);
		
	if (mySourceCompParamsHandle != NULL)
		DisposeHandle(mySourceCompParamsHandle);
		
	if (myDestCompParamsHandle != NULL)
		DisposeHandle(myDestCompParamsHandle);
		
	return(noErr);

CompressErr:
	if (myMovie != NULL)
		DisposeMovie(myMovie);
		
	if (myResRefNum != -1)
		CloseMovieFile(myResRefNum);
		
CreateOutputErr:
InputFileErr:
	if (mySourceCompParamsHandle != NULL)
		DisposeHandle(mySourceCompParamsHandle);
		
NoDest:
	if (mySourceInfo.buffer != NULL)
		DisposePtr((Ptr)mySourceInfo.buffer);
	
NoSource:
	if (mySourceCompParamsHandle != NULL)
		DisposeHandle(mySourceCompParamsHandle);
		
NoData:
	FSClose(myInputFile);
	
NoInputFile:
	return(myErr);
}


//////////
//
// AudConv_GetDataFromAIFF
// Parse the specified file and read the data in the required chunks in it.
//
// This is a very simple AIFF parser which will read three basic chunks: the
// main ContainerChunk, the Common or ExtCommonChunk, and the SoundDataChunk.
// That's all that's required. One last chunk is read if found, which contains the
// decompression parameters required to decompress this audio. It's a simple
// chunk with unknown data contained within. Copy this data into a handle, and
// it becomes the audio atoms used with the siDecompressionParams selector.
//
// Keep in mind that the data in an AIFF file is always big-endian.
//
//////////

OSErr AudConv_GetDataFromAIFF (short theRefNum, SoundComponentDataPtr theSourceInfo, long *theSourceOffset, long *theSourceSize, Handle *theDestCompParams)
{
	union {
		ContainerChunk	cnt;
		CommonChunk		com;
		ExtCommonChunk	ext;
		SoundDataChunk	snd;
	} 					myChunk;
	ChunkHeader			myCHeader;
	Handle				myDestCompParamsHandle;
	long double			mySampleRate;
	long				myOffset;
	long				myDataSize;
	long				myByteCount;
	OSErr				myErr = noErr;

	myErr = SetFPos(theRefNum, fsFromStart, 0);		// we have to start at the beginning
	FailIf(myErr != noErr, Failure);

	// everything is contained in a container chunk, which therefore is the first chunk in the file;
	// read the beginning of the container chunk into memory
	myByteCount = sizeof(ContainerChunk);
	myErr = FSRead(theRefNum, &myByteCount, &myChunk);
	FailIf(myErr != noErr, Failure);

	// make sure it's a FORM chunk
	FailWithAction(EndianU32_BtoN(myChunk.cnt.ckID) != FORMID, myErr = badFileFormat, Failure);

	// find the CommonChunk or ExtCommonChunk
	myErr = AudConv_SetFPosToChunk(theRefNum, CommonID);
	FailIf(myErr != noErr, Failure);

	switch (EndianU32_BtoN(myChunk.cnt.formType)) {
		case AIFFID:
			myByteCount = sizeof(CommonChunk);
			myErr = FSRead(theRefNum, &myByteCount, &myChunk);
			FailIf(myErr != noErr, Failure);

			theSourceInfo->format = k16BitBigEndianFormat;
			break;

		case AIFCID:
			myByteCount = offsetof(ExtCommonChunk, compressionName);
			myErr = FSRead(theRefNum, &myByteCount, &myChunk);
			FailIf(myErr != noErr, Failure);

			theSourceInfo->format = EndianU32_BtoN(myChunk.ext.compressionType);
			break;

		default:
			FailWithAction(badFileFormat, myErr = badFileFormat, Failure);
			break;
	}
	
	theSourceInfo->flags = 0L;
	theSourceInfo->numChannels = EndianS16_BtoN(myChunk.com.numChannels);
	theSourceInfo->sampleSize = EndianS16_BtoN(myChunk.com.sampleSize);
#if TARGET_CPU_68K
	mySampleRate = myChunk.com.sampleRate;
#else
	x80told(&myChunk.com.sampleRate, &mySampleRate);
#endif
	theSourceInfo->sampleRate = (unsigned long)(mySampleRate * 65536.0);
	theSourceInfo->sampleCount = EndianU32_BtoN(myChunk.com.numSampleFrames);
	theSourceInfo->buffer = NULL;
	theSourceInfo->reserved = 0;

	myErr = AudConv_SetFPosToChunk(theRefNum, SoundDataID);
	FailIf(myErr != noErr, Failure);

	myByteCount = sizeof(SoundDataChunk);
	myErr = FSRead(theRefNum, &myByteCount, &myChunk);
	FailIf(myErr != noErr, Failure);

	myDataSize = EndianS32_BtoN(myChunk.snd.ckSize) - sizeof(SoundDataChunk);	// size of source data
	myErr = GetFPos(theRefNum, &myOffset);
	FailIf(myErr != noErr, Failure);

	// return NULL if no siDecompressionParams
	myDestCompParamsHandle = NULL;
	myErr = AudConv_SetFPosToChunk(theRefNum, siDecompressionParams);
	if (myErr == noErr) {
		myByteCount = sizeof(myCHeader);
		myErr = FSRead(theRefNum, &myByteCount, &myCHeader);
		FailIf(myErr != noErr, Failure);

		myByteCount = EndianS32_BtoN(myCHeader.ckSize);
		myDestCompParamsHandle = NewHandle(myByteCount);
		FailIf(myDestCompParamsHandle == NULL, Failure);

		HLock(myDestCompParamsHandle);
		myErr = FSRead(theRefNum, &myByteCount, *myDestCompParamsHandle);
		HUnlock(myDestCompParamsHandle);
		FailIf(myErr != noErr, ReadParamsErr);
	}
	
	*theSourceOffset = myOffset;						// offset from start of file to audio samples
	*theSourceSize = myDataSize;						// size in bytes of the audio samples
	*theDestCompParams = myDestCompParamsHandle;		// possible handle to the decompression parameters
	return(noErr);

ReadParamsErr:
	if (myDestCompParamsHandle != NULL)
		DisposeHandle(myDestCompParamsHandle);
		
Failure:
	return(myErr);
}


//////////
//
// AudConv_ConvertAudioIntoHandle
// Convert the specified sound data.
//
//////////

OSErr AudConv_ConvertAudioIntoHandle (	SoundComponentDataPtr theSourceInfo, 
										Handle theSourceCompParamsHandle,
										Handle theDestHandle, 
										SoundComponentDataPtr theDestInfo,
										Handle *theDestCompParamsHandle, 
										CompressionInfoPtr theDestCompInfo)
{
	SoundConverter		myConverter;
	Handle				myDestCompParamsHandle = NULL;			
	CompressionInfo		myCompressionInfo;
	unsigned short		mySourceBytesPerFrame;
	UInt16				hasOptionsDialog = 0;
	OSErr				myErr = noErr;

	// perform in non-real time for best quality
	theDestInfo->flags |= kNoRealtimeProcessing;			

	//////////
	//				
	// open a sound converter
	//				
	//////////

	myErr = SoundConverterOpen(theSourceInfo, theDestInfo, &myConverter);
	FailIf(myErr != noErr, Exit);

	//////////
	//				
	// display the options dialog for the sound converter
	//
	// the GetInfo call isn't required; you can just call SetInfo and it will show the dialog
	// or return the same error that GetInfo would have; you might call GetInfo to determine
	// whether the options dialog is supported, perhaps because you want to show a button that
	// would bring up this dialog
	//				
	//////////

	myErr = SoundConverterGetInfo(myConverter, siOptionsDialog, &hasOptionsDialog);
	if ((myErr == noErr) && (hasOptionsDialog != 0)) {
		myErr = SoundConverterSetInfo(myConverter, siOptionsDialog, NULL);
		FailIf(myErr != noErr, DialogErr);
	}

	// set the number of output channels
	myErr = SoundConverterSetInfo(myConverter, siCompressionChannels, &theDestInfo->numChannels);
	// ignore this error, some codecs don't use this selector (makes QDesign work)

	//////////
	//				
	// get any custom decompression settings
	//
	// not all sound compressors have custom settings, in which case myDestCompParamsHandle
	// will be returned unchanged (NULL); we return these custom settings to the caller, who
	// will put them in the proper place; if this audio is stored in a QuickTime movie, these
	// settings will be stored in a SoundDescriptionExtension of type siDecompressionParams
	//
	//////////

	SoundConverterGetInfo(myConverter, siCompressionParams, &myDestCompParamsHandle);	
	
	// send the source and destination compression settings to the sound converter; this is
	// necessary to make some sound converters (for instance, QDesign) work properly
	if (myDestCompParamsHandle != NULL) {
		HLockHi(myDestCompParamsHandle);
		myErr = SoundConverterSetInfo(myConverter, siCompressionParams, *myDestCompParamsHandle);
		HUnlock(myDestCompParamsHandle);
		FailIf(myErr != noErr, ConverterErr);
	}

	if (theSourceCompParamsHandle != NULL) {
		HLockHi(theSourceCompParamsHandle);
		myErr = SoundConverterSetInfo(myConverter, siDecompressionParams, *theSourceCompParamsHandle);
		HUnlock(theSourceCompParamsHandle);
		FailIf(myErr != noErr, ConverterErr);
	}

	*theDestCompParamsHandle = myDestCompParamsHandle;

	//////////
	//				
	// get the sizes of the source and destination sample frames
	//
	// we return the destination sample frame size to the caller, who will put it in the
	// proper place; if this audio data is stored in a QuickTime movie, this size will be
	// stored the Version 1 sound description handle
	//				
	//////////

	// get the source number of bytes in each sample frame
	myErr = SoundConverterGetInfo(myConverter, siCompressionFactor, &myCompressionInfo);
	if ((myErr != noErr) || (myCompressionInfo.format != theSourceInfo->format)) {
		myErr = GetCompressionInfo(fixedCompression, theSourceInfo->format, theSourceInfo->numChannels, theSourceInfo->sampleSize, &myCompressionInfo);
		FailIf(myErr != noErr, ConverterErr);
	}
	
	// bytesPerFrame is not filled in by GetCompressionInfo, so we set it here
	mySourceBytesPerFrame = myCompressionInfo.bytesPerPacket * theSourceInfo->numChannels;
	
	// get the destination number of bytes in each sample frame
	if (theDestCompInfo != NULL) {
		// get info about destination compression.
		myErr = SoundConverterGetInfo(myConverter, siCompressionFactor, theDestCompInfo);
		if ((myErr != noErr) || (theDestCompInfo->format != theDestInfo->format)) {
			myErr = GetCompressionInfo(fixedCompression, theDestInfo->format, theDestInfo->numChannels, theDestInfo->sampleSize, theDestCompInfo);
			FailIf(myErr != noErr, ConverterErr);
		}
		
		// bytesPerFrame is not filled in by GetCompressionInfo, so we set it here
		theDestCompInfo->bytesPerFrame = theDestCompInfo->bytesPerPacket * theDestInfo->numChannels;
	}
	
	//////////
	//				
	// do the actual data conversion
	//
	//////////

	myErr = AudConv_WriteAudioToHandle(myConverter, theSourceInfo, mySourceBytesPerFrame, theDestInfo, theDestHandle);
	FailIf(myErr != noErr, ConverterErr);

	SoundConverterClose(myConverter);
	return(noErr);

ConverterErr:
	if (myDestCompParamsHandle != NULL)
		DisposeHandle(myDestCompParamsHandle);
		
DialogErr:
	if (myConverter != NULL)
		SoundConverterClose(myConverter);
	
Exit:
	return(myErr);
}


//////////
//
// AudConv_WriteAudioToHandle
// Convert the source data into an ever-expanding handle of the given format.
// Of course this means you've only got one shot at converting the data, and it
// all has to fit into memory. Converting larger amounts is left to the reader.
//
//////////

OSErr AudConv_WriteAudioToHandle (SoundConverter theConverter, SoundComponentDataPtr theSourceInfo, short theSourceBytesPerFrame, SoundComponentDataPtr theDestInfo, Handle theDestHandle)
{
	unsigned long	myNumFramesLeft;
	unsigned long	mySourceFrames;
	unsigned long	myDestFrames;
	unsigned long	mySourceBytes;
	unsigned long	myDestBytes;
	unsigned long	myDataOffset = 0;
	Ptr				myDestPtr = NULL;
	OSErr			myErr = noErr;

	// get sound converter buffer size info
	myErr = SoundConverterGetBufferSizes(theConverter, kMaxSndConvtBufferSize, &mySourceFrames, &mySourceBytes, &myDestBytes);
	FailIf(myErr != noErr, Exit);

	// create destination data buffer
	myDestPtr = NewPtrClear(myDestBytes);
	FailWithAction(myErr != noErr, myErr = MemError(), Exit);

	//////////
	//
	// convert the sound to the desired output format
	//
	//////////

	myErr = SoundConverterBeginConversion(theConverter);
	FailIf(myErr != noErr, ConverterErr);

	// initialize destination total frame count to zero
	theDestInfo->sampleCount = 0;
	myNumFramesLeft = (unsigned long)theSourceInfo->sampleCount;
	
	while (myNumFramesLeft > 0) {
		// if there are fewer frames remaining than our source buffer size,
		// we're near the end of our data, so get just what's remaining
		if (mySourceFrames > myNumFramesLeft)
			mySourceFrames = myNumFramesLeft;

		myErr = SoundConverterConvertBuffer(theConverter, theSourceInfo->buffer + myDataOffset, mySourceFrames, myDestPtr, &myDestFrames, &myDestBytes);
		FailIf(myErr != noErr, ConverterErr);

		// write this data to the output handle
		if (myDestBytes > 0) {
			myErr = PtrAndHand(myDestPtr, theDestHandle, myDestBytes);
			FailIf(myErr != noErr, ConverterErr);
		}

		// move offset to appropriate place in source data
		myDataOffset += mySourceFrames * theSourceBytesPerFrame;
		
		// keep track of total frames returned by converter
		theDestInfo->sampleCount += myDestFrames;
		
		myNumFramesLeft -= mySourceFrames;
	}

	// end the conversion, and see if we get back a few more bytes of data	
	myErr = SoundConverterEndConversion(theConverter, myDestPtr, &myDestFrames, &myDestBytes);
	FailIf(myErr != noErr, ConverterErr);

	if (myDestBytes > 0) {
		// write this data to the output handle
		myErr = PtrAndHand(myDestPtr, theDestHandle, myDestBytes);
		FailIf(myErr != noErr, ConverterErr);
	}
	
	theDestInfo->sampleCount += myDestFrames;			// update final amount of samples

ConverterErr:
	if (myDestPtr != NULL)
		DisposePtr(myDestPtr);
		
Exit:
	return(myErr);
}


//////////
//
// AudConv_GetOutputFormat
// Display the QuickTime standard sound settings dialog to obtain the output format.
//
//////////

OSErr AudConv_GetOutputFormat (SoundComponentDataPtr theDestInfo)
{
	ComponentInstance	myComponent = NULL;
	ComponentResult		myErr = noErr;

	myComponent = OpenDefaultComponent(StandardCompressionType, StandardCompressionSubTypeSound);
	FailWithAction(myComponent == NULL, myErr = cantFindHandler, NoComponent);

	myErr = SCRequestImageSettings(myComponent);			// show the dialog to user
	if (myErr == noErr) {									// then get the settings
		myErr = SCGetInfo(myComponent, scSoundCompressionType, &theDestInfo->format);
		FailIf(myErr != noErr, SCGetInfoFailed);

		myErr = SCGetInfo(myComponent, scSoundChannelCountType, &theDestInfo->numChannels);
		FailIf(myErr != noErr, SCGetInfoFailed);

		myErr = SCGetInfo(myComponent, scSoundSampleSizeType, &theDestInfo->sampleSize);
		FailIf(myErr != noErr, SCGetInfoFailed);

		myErr = SCGetInfo(myComponent, scSoundSampleRateType, &theDestInfo->sampleRate);
		FailIf(myErr != noErr, SCGetInfoFailed);

		// any non-compressed format appears as 'raw ', which is not what the Sound Manager expects
		if (theDestInfo->format == k8BitOffsetBinaryFormat) {
			if (theDestInfo->sampleSize > 8)
				theDestInfo->format = k16BitBigEndianFormat;
			else
				theDestInfo->format = kSoundNotCompressed;
		}

		// clear the rest of the unused parameters
		theDestInfo->flags = 0;
		theDestInfo->sampleCount = 0;
		theDestInfo->buffer = NULL;
		theDestInfo->reserved = 0L;
	}
	
	return((OSErr)myErr);

SCGetInfoFailed:
	CloseComponent(myComponent);
	
NoComponent:
	return((OSErr)myErr);
}


//////////
//
// AudConv_SetFPosToChunk
// Set the file's position to the start of the given chunk.
//
//////////

OSErr AudConv_SetFPosToChunk (short theRefNum, ID theChunkID)
{
	long				myCount;
	ChunkHeader			myCHeader;
	OSErr				myErr = noErr;

	// position the file mark past the ContainerChunk, which is always the first thing in the file
	myErr = SetFPos(theRefNum, fsFromStart, sizeof(ContainerChunk));
	FailIf(myErr != noErr, Exit);

	while (true) {
		myCount = sizeof(myCHeader);
		myErr = FSRead(theRefNum, &myCount, &(myCHeader.ckID));
		if (myErr == eofErr)							// reached the end of file, exit gracefully
			break;
		FailIf(myErr != noErr, Exit);

		if (theChunkID == EndianU32_BtoN(myCHeader.ckID)) {
			myCount = -myCount;							// rewind ptr to top of chunk if found
			myErr = SetFPos(theRefNum, fsFromMark, myCount);
			FailIf(myErr != noErr, Exit);
			break;
		} else {										// else skip over the entire chunk
			myErr = SetFPos(theRefNum, fsFromMark, EndianS32_BtoN(myCHeader.ckSize));
			if (myErr == eofErr)						// reached the end of file, exit gracefully
				break;
			FailIf(myErr != noErr, Exit);
		}
	}

Exit:
	return(myErr);
}


//////////
//
// AudConv_PrepFileAsAIFF
// Write out the necessary chunks, without audio samples, to prepare for writing;
// leave file position at the proper place for audio samples.
//
//////////

OSErr AudConv_PrepFileAsAIFF (short theOutputFile, SoundComponentDataPtr theDestInfo, Handle theDestParams)
{
	Str255				myCompressionName;
	ContainerChunk		myContainer;
	FormatVersionChunk	myVersion;
	ExtCommonChunk		myCommon;
	SoundDataChunk		mySoundData;
	ID					myFormType;
	long				myCount;
	OSErr				myErr = noErr;

	switch (theDestInfo->format) {
		case kSoundNotCompressed:
		case k8BitOffsetBinaryFormat:
		case k16BitBigEndianFormat:
		case k24BitFormat:
		case k32BitFormat:
			myFormType = AIFFID;
			break;

		default :
			myFormType = AIFCID;
			break;
	}

	myErr = SetFPos(theOutputFile, fsFromStart, 0);
	FailIf(myErr != noErr, Exit);

	// write out the first chunk
	myContainer.ckID = EndianU32_NtoB(FORMID);
	myContainer.ckSize = EndianU32_NtoB(0);									// nothing in the file just yet
	myContainer.formType = EndianU32_NtoB(myFormType);
	myCount = sizeof(myContainer);
	myErr = FSWrite(theOutputFile, &myCount, &myContainer);
	FailIf(myErr != noErr, Exit);

	// write out the version chunk for AIFC files
	if (myFormType == AIFCID) {
		myVersion.ckID = EndianU32_NtoB(FormatVersionID);
		myVersion.ckSize = EndianS32_NtoB(sizeof(FormatVersionChunk) - sizeof(ChunkHeader));
		myVersion.timestamp = EndianU32_NtoB(AIFCVersion1);					// timestamp for this version
		myCount = sizeof(myVersion);
		myErr = FSWrite(theOutputFile, &myCount, &myVersion);
		FailIf(myErr != noErr, Exit);
	}

	// write out the CommonChunk or ExtCommonChunk if the data is compressed
	myCommon.ckID = EndianU32_NtoB(CommonID);
	myCommon.ckSize = EndianS32_NtoB(sizeof(CommonChunk) - sizeof(ChunkHeader));	// size of common chunk variables without compression info
	myCommon.numChannels = EndianS16_NtoB(theDestInfo->numChannels);
	myCommon.numSampleFrames = EndianU32_NtoB(0);							// nothing in the file just yet
	myCommon.sampleSize = EndianS16_NtoB(theDestInfo->sampleSize);
#if TARGET_CPU_68K
	myCommon.sampleRate = theDestInfo->sampleRate / 65536.0;
#else
	{
		long double		ldRate;

		ldRate = theDestInfo->sampleRate / 65536.0;
		ldtox80(&ldRate, &myCommon.sampleRate);
	}
#endif
	myCommon.compressionType = EndianU32_NtoB(theDestInfo->format);
	myCommon.compressionName[0] = 0;

	if (myFormType == AIFCID) {
		myErr = GetCompressionName(theDestInfo->format, myCompressionName);
		if (myErr == noErr) {
			// write out the ExtCommonChunk for compressed data, with the name
			myCount = offsetof(ExtCommonChunk, compressionName);
			myCommon.ckSize = myCount - sizeof(ChunkHeader) + myCompressionName[0] + 1;
			myCommon.ckSize = ++myCommon.ckSize & ~1;			// must be word aligned
			myCommon.ckSize = EndianS32_NtoB(myCommon.ckSize);
			myErr = FSWrite(theOutputFile, &myCount, &myCommon);
			FailIf(myErr != noErr, Exit);

			myCount = myCompressionName[0] + 1;
			myCount = ++myCount & ~1;							// must be word aligned
			myErr = FSWrite(theOutputFile, &myCount, &myCompressionName);
			FailIf(myErr != noErr, Exit);
		} else {
			// write out the ExtCommonChunk for compressed data, without the name
			myCommon.ckSize = EndianS32_NtoB(sizeof(ExtCommonChunk) - sizeof(ChunkHeader));	// size of common chunk variables without compression info
			myCount = offsetof(ExtCommonChunk, compressionName);
			myErr = FSWrite(theOutputFile, &myCount, &myCommon);
			FailIf(myErr != noErr, Exit);
		}
	} else {	// write out the CommonChunk for non-compressed data
		myCount = sizeof(CommonChunk);
		myErr = FSWrite(theOutputFile, &myCount, &myCommon);
		FailIf(myErr != noErr, Exit);
	}

	if (theDestParams != NULL) {
		ChunkHeader		myCHeader;

		myCHeader.ckID = EndianU32_NtoB(siDecompressionParams);
		myCHeader.ckSize = EndianS32_NtoB(GetHandleSize(theDestParams));
		myCount = sizeof(myCHeader);
		myErr = FSWrite(theOutputFile, &myCount, &myCHeader);
		FailIf(myErr != noErr, Exit);

		myCount = EndianS32_BtoN(myCHeader.ckSize);
		HLock(theDestParams);
		myErr = FSWrite(theOutputFile, &myCount, *theDestParams);
		FailIf(myErr != noErr, Exit);
	}

	mySoundData.ckID = EndianU32_NtoB(SoundDataID);
	mySoundData.ckSize = EndianS32_NtoB(0);							// nothing in the file just yet
	mySoundData.offset = EndianU32_NtoB(0);
	mySoundData.blockSize = EndianU32_NtoB(0);
	myCount = sizeof(mySoundData);
	myErr = FSWrite(theOutputFile, &myCount, &mySoundData);
	FailIf(myErr != noErr, Exit);

	// now go back and update Container to current file size
	myErr = SetFPos(theOutputFile, fsFromStart, 0);
	FailIf(myErr != noErr, Exit);

	myCount = sizeof(myContainer);
	myErr = FSRead(theOutputFile, &myCount, &myContainer);
	FailIf(myErr != noErr, Exit);

	myErr = GetEOF(theOutputFile, &myContainer.ckSize);
	FailIf(myErr != noErr, Exit);

	myErr = SetFPos(theOutputFile, fsFromStart, 0);
	FailIf(myErr != noErr, Exit);

	myContainer.ckSize -= sizeof(ChunkHeader);
	myContainer.ckSize = EndianS32_NtoB(myContainer.ckSize);
	myCount = sizeof(myContainer);
	myErr = FSWrite(theOutputFile, &myCount, &myContainer);
	FailIf(myErr != noErr, Exit);

	// set current file position to the end of end of the file
	// the last chunk is the SoundDataChunk, and that's where to begin writing data
	myErr = SetFPos(theOutputFile, fsFromStart, EndianS32_BtoN(myContainer.ckSize) + sizeof(ChunkHeader));
	FailIf(myErr != noErr, Exit);

Exit:
	return(myErr);
}


//////////
//
// AudConv_FinishFileAsAIFF
// This assumes that all the necessary chunks have been written to the file and
// then audio samples have been added. Finally, update the chunks to reflect the
// samples that were added.
//
//////////

OSErr AudConv_FinishFileAsAIFF (short theOutputFile, unsigned long destFramesMoved, unsigned long destBytesMoved)
{
	CommonChunk			myCommon;
	SoundDataChunk		mySoundData;
	long				myCount;
	OSErr				myErr = noErr;

	// find the CommonChunk and update the numSampleFrames
	myErr = AudConv_SetFPosToChunk(theOutputFile, CommonID);
	FailIf(myErr != noErr, Exit);

	myCount = sizeof(myCommon);
	myErr = FSRead(theOutputFile, &myCount, &myCommon);
	FailIf(myErr != noErr, Exit);

	myErr = SetFPos(theOutputFile, fsFromMark, -myCount);		// reset to start of chunk
	FailIf(myErr != noErr, Exit);

	myCommon.numSampleFrames = EndianU32_NtoB(destFramesMoved);
	myCount = sizeof(myCommon);
	myErr = FSWrite(theOutputFile, &myCount, &myCommon);
	FailIf(myErr != noErr, Exit);

	// find the SoundDataChunk and update the ckSize
	myErr = AudConv_SetFPosToChunk(theOutputFile, SoundDataID);
	FailIf(myErr != noErr, Exit);

	myCount = sizeof(mySoundData);
	myErr = FSRead(theOutputFile, &myCount, &mySoundData);
	FailIf(myErr != noErr, Exit);

	myErr = SetFPos(theOutputFile, fsFromMark, -myCount);		// reset to start of chunk
	FailIf(myErr != noErr, Exit);

	mySoundData.ckSize = EndianS32_NtoB(sizeof(SoundDataChunk) - sizeof(ChunkHeader) + destBytesMoved);
	myCount = sizeof(mySoundData);
	myErr = FSWrite(theOutputFile, &myCount, &mySoundData);
	FailIf(myErr != noErr, Exit);

Exit:
	return(myErr);
}


//////////
//
// AudConv_PutAudioIntoTrack
// Add the audio as a track with the necessary decompression extension.
//
//////////

OSErr AudConv_PutAudioIntoTrack (	Track theTrack, 
									Handle theDestAudioData, 
									SoundComponentDataPtr theDestInfo,
									Handle theDestCompParamsHandle, 
									CompressionInfoPtr theDestCompInfo)
{
	SoundDescriptionV1Handle	mySampleDesc;
	Media						myMedia;
	OSErr						myErr = noErr;

	//////////
	//				
	// create a media for the track passed in
	//				
	//////////

	// set new track to be a sound track
	myMedia = NewTrackMedia(theTrack, SoundMediaType, theDestInfo->sampleRate >> 16, NULL, 0);
	myErr = GetMoviesError();
	FailIf(myErr != noErr, Exit);

	// start a media editing session
	myErr = BeginMediaEdits(myMedia);
	FailIf(myErr != noErr, Exit);

	//////////
	//				
	// create a sound sample description
	//				
	//////////

	// use the SoundDescription format 1 because it adds fields for data size information
	// and is required by AddSoundDescriptionExtension if an extension is required for the compression format
	mySampleDesc = (SoundDescriptionV1Handle)NewHandleClear(sizeof(SoundDescriptionV1));
	FailWithAction(myErr != noErr, myErr = MemError(), Exit);

	(**mySampleDesc).desc.descSize			= sizeof(SoundDescriptionV1);
	(**mySampleDesc).desc.dataFormat		= theDestInfo->format;
	(**mySampleDesc).desc.resvd1			= 0;
	(**mySampleDesc).desc.resvd2			= 0;
	(**mySampleDesc).desc.dataRefIndex		= 1;
	(**mySampleDesc).desc.version			= 1;
	(**mySampleDesc).desc.revlevel			= 0;
	(**mySampleDesc).desc.vendor			= 0;
	(**mySampleDesc).desc.numChannels		= theDestInfo->numChannels;
	(**mySampleDesc).desc.sampleSize		= theDestInfo->sampleSize;
	(**mySampleDesc).desc.compressionID		= 0;
	(**mySampleDesc).desc.packetSize		= 0;
	(**mySampleDesc).desc.sampleRate		= theDestInfo->sampleRate;
	(**mySampleDesc).samplesPerPacket		= theDestCompInfo->samplesPerPacket;
	(**mySampleDesc).bytesPerPacket			= theDestCompInfo->bytesPerPacket;
	(**mySampleDesc).bytesPerFrame			= theDestCompInfo->bytesPerFrame;
	(**mySampleDesc).bytesPerSample			= theDestCompInfo->bytesPerSample;

	// not all compression formats have compression params, so we need to add a
	// sound description extension only for those that do
	if (theDestCompParamsHandle != NULL) {
		myErr = AddSoundDescriptionExtension((SoundDescriptionHandle)mySampleDesc, theDestCompParamsHandle, siDecompressionParams);
		FailIf(myErr != noErr, MediaErr);
	}

	//////////
	//				
	// add samples to the media
	//				
	//////////

	myErr = AddMediaSample(	myMedia,
							theDestAudioData,
							0,
							theDestInfo->sampleCount * theDestCompInfo->bytesPerFrame,
							1,
							(SampleDescriptionHandle)mySampleDesc,
							theDestInfo->sampleCount * theDestCompInfo->samplesPerPacket,
							0,
							NULL);
	FailIf(myErr != noErr, MediaErr);

	myErr = EndMediaEdits(myMedia);
	FailIf(myErr != noErr, MediaErr);

	//////////
	//				
	// insert the media into the track
	//				
	//////////

	myErr = InsertMediaIntoTrack(theTrack, 0, 0, GetMediaDuration(myMedia), fixed1);
	FailIf(myErr != noErr, MediaErr);

MediaErr:
	if (mySampleDesc != NULL)	
		DisposeHandle((Handle)mySampleDesc);
		
Exit:
	return(myErr);
}


//////////
//
// AudConv_SFGetDialogHook
// Hook function for the get file dialog box.
//
//////////

PASCAL_RTN short AudConv_SFGetDialogHook (short theItem, DialogPtr theDialog, void *theOutputAIFF)
{
	ControlHandle			myAIFFControl;
	ControlHandle			myMovieControl;
	Rect					myRect;
	short					myKind;
	
	// make sure we've got a real dialog
	if (theDialog == NULL)
		return(theItem);

	GetDialogItem(theDialog, kOutputAIFFButton, &myKind, (Handle *)&myAIFFControl, &myRect);
	GetDialogItem(theDialog, kOutputMovieButton, &myKind, (Handle *)&myMovieControl, &myRect);

	switch (theItem) {
		case sfHookFirstCall:
			if (*(Boolean *)theOutputAIFF)
				SetControlValue(myAIFFControl, kControlRadioButtonCheckedValue);
			else
				SetControlValue(myMovieControl, kControlRadioButtonUncheckedValue);
			break;

		case kOutputAIFFButton:
			SetControlValue(myAIFFControl, kControlRadioButtonCheckedValue);
			SetControlValue(myMovieControl, kControlRadioButtonUncheckedValue);
			*(Boolean *)theOutputAIFF = true;
			break;

		case kOutputMovieButton:
			SetControlValue(myAIFFControl, kControlRadioButtonUncheckedValue);
			SetControlValue(myMovieControl, kControlRadioButtonCheckedValue);
			*(Boolean *)theOutputAIFF = false;
			break;
	}
	
	return(theItem);
}


//////////
//
// AudConv_NavObjectFilterProc
// Object filter function for the get file dialog box.
//
//////////

PASCAL_RTN Boolean AudConv_NavObjectFilterProc (AEDesc *theItem, void *theInfo, void *theCallBackUD, NavFilterModes theFilterMode)
{
#pragma unused(theCallBackUD, theFilterMode)
	NavFileOrFolderInfo		*myInfo = (NavFileOrFolderInfo *)theInfo;
	
	if (theItem->descriptorType == typeFSS) {
		if (!myInfo->isFolder) {
			OSType			myType = myInfo->fileAndFolder.fileInfo.finderInfo.fdType;
			
			// see whether the file type is in the list of file types that our application can open 
			if ((myType == kQTFileTypeAIFF) || (myType == kQTFileTypeAIFC))
				return(true);

			// if we got to here, it's a file we cannot open
			return(false);		
		}
	}
	
	// if we got to here, it's a folder or non-HFS object
	return(true);
}


//////////
//
// AudConv_ConvertCToPascalString
// Convert a C string into a Pascal string.
//
// The caller is responsible for disposing of the pointer returned by this function (by calling free).
//
//////////

StringPtr AudConv_ConvertCToPascalString (char *theString)
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


//////////
//
// AudConv_HandleNavEvent
// A callback procedure that handles events while a Navigation Service dialog box is displayed.
//
//////////

PASCAL_RTN void AudConv_HandleNavEvent (NavEventCallbackMessage theCallBackSelector, NavCBRecPtr theCallBackParms, void *theCallBackUD)
{
	Boolean				myOutputAIFF = *(Boolean *)theCallBackUD;
	NavMenuItemSpec		myMenuItemSpec;
	Str255				myFileName;
	static Str15		myMooVExt;
	static Str15		myAIFFExt;
	
	if (theCallBackSelector == kNavCBStart) {
		// get the filename extensions
		GetIndString(myMooVExt, rStringsResID, rMooVExtensionIndex);
		GetIndString(myAIFFExt, rStringsResID, rAIFFExtensionIndex);
	}

	if (theCallBackSelector == kNavCBPopupMenuSelect) {
		myMenuItemSpec = *(NavMenuItemSpec *)theCallBackParms->eventData.eventDataParms.param;
		myOutputAIFF = (myMenuItemSpec.menuType == kQTFileTypeAIFF);
	}
	
	if (*(Boolean *)theCallBackUD != myOutputAIFF) {
		// the output file type has changed, so adjust the filename extension accordingly
		short			myIndex;
		short			myCount;
		
		// get the current filename
		NavCustomControl(theCallBackParms->context, kNavCtlGetEditFileName, &myFileName);

		// find the position of the last filename separator in the current filename
		myIndex = myFileName[0];
		while ((myFileName[myIndex] != kFileExtSeparator) && (myIndex > 0))
			myIndex--;

		if (myIndex != 0) {
			if (myOutputAIFF) {
				for (myCount = 1; myCount <= myAIFFExt[0]; myCount++)
					myFileName[myIndex + myCount - 1] = myAIFFExt[myCount];
			} else {
				for (myCount = 1; myCount <= myMooVExt[0]; myCount++)
					myFileName[myIndex + myCount - 1] = myMooVExt[myCount];
			}
			
			myFileName[0] = myIndex + myCount - 2;
			NavCustomControl(theCallBackParms->context, kNavCtlSetEditFileName, &myFileName);
		}
		
		*(Boolean *)theCallBackUD = myOutputAIFF;
	}
}


