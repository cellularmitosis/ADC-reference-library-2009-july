//////////
//
//	File:		SoundSnippets.c
//
//	Contains:	Code snippets showing how to perform a few typical sound-related operations.
//
//	Written by:	Tim Monroe
//				Some routines based on existing code by Jim Reekes and Kip Olson.
//
//	Copyright:	© 1998-2003 by Apple Computer, Inc., all rights reserved.
//
//	Change History (most recent first):
//	   <8>		08/06/03	era		fixed SndSnip_ConvertWAVEFormats as it did not previously set
//								    the value for numChannels in the Sound Description
//	   <7>	 	02/03/99	rtm		minor tweaks based on AudioConvert.c sample code
//	   <6>	 	02/03/99	rtm		reworked prompt and filename handling to remove "\p" sequences
//	   <5>	 	06/25/98	rtm		added SndSnip_HasSoundManager3_1; modified the function
//									SndSnip_PromptUserForDiskFileAndSaveCompressed to compress a
//									file in 40K chunks
//	   <4>	 	06/22/98	rtm		added SndSnip_GetAudioSettings and SndSnip_ConvertWAVEFormats;
//									added SndSnip_PromptUserForAudioFileAndCompress and
//									SndSnip_PromptUserForDiskFileAndSaveCompressed, which illustrate
//									both the standard sound compression dialog component and the
//									sound converter routines
//	   <3>	 	05/29/98	rtm		added SndSnip_GetHardwareSettings, SndSnip_CheckVersionNumber,
//									SndSnip_GetVolume, and SndSnip_SetVolume
//	   <2>	 	04/24/98	rtm		got bufferCmd double-buffering working
//	   <1>	 	04/17/98	rtm		first file
//	  
//	This sample code illustrates how to perform some common sound-related operations,
//	such as saving a movie's sound track into a separate file or playing a continuous sound using
//	the bufferCmd sound command. This is not really a library of utilities, but more a grab-bag
//	of "how-to" recipes. Here's a partial list of what's illustrated here:
//
//	*Save a sound-only QuickTime movie as a WAVE file.
//	*Save a sound-only QuickTime movie as a file whose type the user selects.
//	*Save a sound track in a QuickTime movie as a file whose type the user selects.
//	*Create a handle that contains the first sound track in a QuickTime movie.
//	*Open and play a WAVE file using the QuickTime API.
//	*Set the volume level of a sound track in a QuickTime movie.
//	*Use the bufferCmd sound command to continuously play a large buffer of audio data.
//	*Get the current hardware settings.
//	*Get the settings for a sound track in a QuickTime movie.
//	*Get and set the current left and right volumes of a sound channel.
//	*Let the user select an audio file and select its compression settings; then compress it.
//
//	NOTES:
//
//	*** (1) ***
//	The function SndSnip_PromptUserForAudioFileAndCompress uses the standard sound compression dialog routines
//	to elicit compression/conversion settings from the user. These routines were based on the standard image
//	compression dialog routines. You can use modal-dialog filter functions and hook functions with the sound
//	routines in the same way that you use them with the image routines; this, however, is not illustrated here.
//	See the code snippet QTStdCompr for sample code that does illustrate these features of the standard
//	compression dialog routines.
//
//	*** (2) ***
//	The function SndSnip_PromptUserForDiskFileAndSaveCompressed uses the sound converter routines (such as
//	SoundConverterConvertBuffer) to compress audio data. These routines were introduced in Sound Manager 3.2.
//	If you want to run in earlier Sound Manager versions, you'll need to replace this conversion method with
//	some other method. The method illustrated here is based on code in the file SoundConvert.c by Andrew Wulf,
//	Apple Developer Technical Support. 
//
//////////

#include "SoundSnippets.h"
#include "QTUtilities.h"


// global variables
SoundHeader					gSndHeader1;
SoundHeader					gSndHeader2;
Boolean						gCurrChunkDirty = false;
short						gCurrChunkIndex;
SoundHeaderPtr				gCurrChunkHeader;
SndChannelPtr				gCurrChannel;


///////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Sound movie conversion routines.
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////

//////////
//
// SndSnip_SaveSoundMovieAsWAVEFile
// Save a sound-only QuickTime movie as a WAVE file.
//
//////////

void SndSnip_SaveSoundMovieAsWAVEFile (Movie theMovie)
{	
	StandardFileReply		myReply;
	StringPtr 				myPrompt = QTUtils_ConvertCToPascalString(kSaveSoundPrompt);
	StringPtr 				myFileName = QTUtils_ConvertCToPascalString(kSaveSoundWaveName);
	
	// have the user select the name and location of the new WAVE file
	StandardPutFile(myPrompt, myFileName, &myReply);
	if (!myReply.sfGood)
		return;
		
	// use the default progress procedure, if any
	SetMovieProgressProc(theMovie, (MovieProgressUPP)-1L, 0);
	
	// export the movie into a file
	ConvertMovieToFile(	theMovie,					// the movie to convert
						NULL,						// all tracks in the movie
						&myReply.sfFile,			// the output file
						kQTFileTypeWave,			// the output file type
						FOUR_CHAR_CODE('TVOD'),		// the output file creator
						smSystemScript,				// the script
						NULL, 						// no resource ID to be returned
						0L,							// no flags
						NULL);						// no specific component
						
	free(myPrompt);
	free(myFileName);
}


//////////
//
// SndSnip_SaveSoundMovieAsAnyTypeFile
// Save a sound-only QuickTime movie as a file whose type the user selects.
//
//////////

void SndSnip_SaveSoundMovieAsAnyTypeFile (Movie theMovie)
{	
	StandardFileReply		myReply;
	StringPtr 				myPrompt = QTUtils_ConvertCToPascalString(kSaveSoundPrompt);
	StringPtr 				myFileName = QTUtils_ConvertCToPascalString(kSaveSoundFileName);
	
	// have the user select the name and location of the new file
	StandardPutFile(myPrompt, myFileName, &myReply);
	if (!myReply.sfGood)
		return;
		
	// use the default progress procedure, if any
	SetMovieProgressProc(theMovie, (MovieProgressUPP)-1L, 0);
	
	// export the movie into a file; since the flag showUserSettingsDialog is set,
	// the user gets to select the audio format of the saved audio data
	ConvertMovieToFile(	theMovie,					// the movie to convert
						NULL,						// all tracks in the movie
						&myReply.sfFile,			// the output file
						0L,							// the output file type
						FOUR_CHAR_CODE('TVOD'),		// the output file creator
						smSystemScript,				// the script
						NULL, 						// no resource ID to be returned
						createMovieFileDeleteCurFile | movieToFileOnlyExport | showUserSettingsDialog,	
						NULL);						// no specific component
						
	free(myPrompt);
	free(myFileName);

}


//////////
//
// SndSnip_SaveSoundTrackAsAnyTypeFile
// Save the first sound track in a QuickTime movie as a file whose type the user selects.
//
//////////

void SndSnip_SaveSoundTrackAsAnyTypeFile (Movie theMovie)
{	
	StandardFileReply		myReply;
	Track					myTrack = NULL;
	StringPtr 				myPrompt = QTUtils_ConvertCToPascalString(kSaveSoundPrompt);
	StringPtr 				myFileName = QTUtils_ConvertCToPascalString(kSaveSoundFileName);
	
	// find the first sound track in the specified movie
	myTrack = GetMovieIndTrackType(theMovie, 1, AudioMediaCharacteristic, movieTrackCharacteristic | movieTrackEnabledOnly);
	if (myTrack == NULL)
		return;
	
	// have the user select the name and location of the new file
	StandardPutFile(myPrompt, myFileName, &myReply);
	if (!myReply.sfGood)
		return;
		
	// use the default progress procedure, if any
	SetMovieProgressProc(theMovie, (MovieProgressUPP)-1L, 0);
	
	// export the specified track into a file; since the flag showUserSettingsDialog is set,
	// the user gets to select the audio format of the saved audio data
	ConvertMovieToFile(	theMovie,					// the movie to convert
						myTrack,					// save only the specified track
						&myReply.sfFile,			// the output file
						0L,							// the output file type
						FOUR_CHAR_CODE('TVOD'),		// the output file creator
						smSystemScript,				// the script
						NULL, 						// no resource ID to be returned
						createMovieFileDeleteCurFile | movieToFileOnlyExport | showUserSettingsDialog,	
						NULL);						// no specific component
						
	free(myPrompt);
	free(myFileName);
}


//////////
//
// SndSnip_ExtractSoundTrackIntoHandle
// Create a handle that contains the first sound track in a QuickTime movie.
//
// NOTE: To create a handle that contains ALL the sound tracks in the movie
// mixed together, pass NULL instead of myTrack to PutMovieIntoTypedHandle.
//
//////////

void SndSnip_ExtractSoundTrackIntoHandle (Movie theMovie)
{	
	Track					myTrack = NULL;
	Handle					myHandle = NULL;
	OSErr					myErr = noErr;
	
	// find the first sound track in the specified movie
	myTrack = GetMovieIndTrackType(theMovie, 1, AudioMediaCharacteristic, movieTrackCharacteristic | movieTrackEnabledOnly);
	if (myTrack == NULL)
		goto bail;
	
	// create an empty handle; this will be resized by PutMovieIntoTypedHandle
	myHandle = NewHandle(0);
	if (myHandle == NULL)
		goto bail;
	
	// use the default progress procedure, if any
	SetMovieProgressProc(theMovie, (MovieProgressUPP)-1L, 0);
	
	// extract the sound track and put it into the handle
	myErr = PutMovieIntoTypedHandle(
							theMovie,					// the movie to convert
							myTrack,					// save only the specified track
							soundListRsrc,				// make it a sound resource handle
							myHandle,					// the handle into which to put the audio data
							0,							// start time
							GetTrackDuration(myTrack),	// ending time
							0L,							// flags
							NULL);						// no specific component
	if (myErr != noErr)
		goto bail;
		
	// to prove we really did create the handle, play it
	SndPlay(NULL, (SndListHandle)myHandle, false);
	
bail:
	if (myHandle != NULL)
		DisposeHandle(myHandle);
}


//////////
//
// SndSnip_PlayWAVEFileWithQuickTime
// Open and play a WAVE file using the QuickTime API.
//
//////////

void SndSnip_PlayWAVEFileWithQuickTime (void)
{
	StandardFileReply		myReply;
	SFTypeList				myTypeList = {kQTFileTypeWave, 0, 0, 0};
	short					myRefNum;
	
	// elicit a file from the user
	StandardGetFile(NULL, 1, myTypeList, &myReply);
	if (myReply.sfGood) {
		Movie				myMovie;
		
		OpenMovieFile(&myReply.sfFile, &myRefNum, fsRdPerm);
		NewMovieFromFile(&myMovie, myRefNum, NULL, (StringPtr)NULL, newMovieActive, NULL);
		if (myRefNum != 0)
			CloseMovieFile(myRefNum);
			
		// play the movie once thru
		StartMovie(myMovie);
		while (!IsMovieDone(myMovie))
			MoviesTask(myMovie, 0);
		DisposeMovie(myMovie);
	}
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Volume routines.
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////

//////////
//
// SndSnip_SetVolumeOfSoundTrack
// Set the volume level of the first sound track in a QuickTime movie.
//
// The range for the theVolume parameter is 0 (no volume) to 256 (full volume).
// You can also pass negative values (e.g., -128). No sound is produced for negative
// values, but they are useful for maintaining the absolute value of a volume.
//
//////////

void SndSnip_SetVolumeOfSoundTrack (Movie theMovie, short theVolume)
{
	Track			myTrack = NULL;
	
	// find the first sound track in the specified movie
	myTrack = GetMovieIndTrackType(theMovie, 1, AudioMediaCharacteristic, movieTrackCharacteristic | movieTrackEnabledOnly);
	if (myTrack == NULL)
		return;
	
	// set the volume of the track
	SetTrackVolume(myTrack, theVolume);
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Double-buffering routines.
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////

//////////
//
// SndSnip_PlaySoundResourceUsingBufferCmds
// Use the bufferCmd sound command to continuously play a large buffer of audio data.
//
// NOTE: This routine exists solely to illustrate one way to use bufferCmd sound commands
// to play a buffer of audio data continuously; here, we read the audio data from a resource
// and then hand chunks of it to the Sound Manager using bufferCmds. OF COURSE we could have
// played the entire buffer in one fell swoop by calling SndPlay. The idea is that a real
// application would read the audio data from disk in bite-sized chunks and use the techniques
// illustrated here to play that data.
//
//////////

OSErr SndSnip_PlaySoundResourceUsingBufferCmds (void)
{
	Handle					myHandle = NULL;
	SoundComponentData		myResourceInfo;
	unsigned long			myNumFrames;
	unsigned long			myOffset;
	Ptr						myPtr = NULL;
	Size					mySize;
	long					myFreq;
	SndChannelPtr			mySndChannel = NULL;
	SndCallBackUPP			mySndCallBackProc;
	unsigned long			myBufferSize;			// the size of the buffer we pass to each bufferCmd command
	OSErr					myErr = memFullErr;

	// get the resource data; we assume this resource contains sampled sound data
	myHandle = GetResource(soundListRsrc, kSampleResourceID);
	if (myHandle == NULL)
		goto bail;
		
	// get info from the sound resource header
	myErr = ParseSndHeader((SndListHandle)myHandle, &myResourceInfo, &myNumFrames, &myOffset);
	if (myErr != noErr)
		goto bail;
	
	// get the base frequency from the sampled sound header
	myFreq = SndSnip_GetSndBaseFrequency(myHandle);
	
	// lock the handle, so we can use a pointer to cruise thru the sound data
	HLockHi(myHandle);

	myPtr = *myHandle + myOffset;
	mySize = GetHandleSize(myHandle) - myOffset;
	myBufferSize = mySize / kNumberOfBufferChunks;
	
	// allocate a routine descriptor for our sound callback routine
	mySndCallBackProc = NewSndCallBackProc(SndSnip_CallbackProc);

	// create storage for a new sound channel
	mySndChannel = (SndChannelPtr)NewPtrClear(sizeof(SndChannel));
	if (mySndChannel == NULL)
		goto bail;
	
	// set the number of commands in the sound channel queue;
	// pass the application's A5 value in the userInfo field
	mySndChannel->qLength = kNumberOfCmdsInQueue;
#if TARGET_OS_MAC	
	mySndChannel->userInfo = SetCurrentA5();
#endif
	
	// create the sound channel
	myErr = SndNewChannel(&mySndChannel, sampledSynth, initMono, mySndCallBackProc);
	if (myErr != noErr) {
		DisposePtr((Ptr)mySndChannel);
		goto bail;
	}
	
	// the basic idea is this: fill two buffers with data and pass them to the Sound Manager
	// using the bufferCmd sound command; each bufferCmd is immediately followed in the queue
	// by a callBackCmd sound command; a callBackCmd triggers the _CheckBuffers routine, which
	// installs another buffer of data and an associated callBackCmd
	
	// configure the first sound header
	gSndHeader1.samplePtr = myPtr;
	gSndHeader1.length = myBufferSize;
	gSndHeader1.sampleRate = myResourceInfo.sampleRate;
	gSndHeader1.loopStart = 0;
	gSndHeader1.loopEnd = 0;
	gSndHeader1.encode = stdSH;
	gSndHeader1.baseFrequency = myFreq;
	
	// install the first bufferCmd, to play the first chunk of the buffer
	myErr = SndSnip_InstallBufferCmd(mySndChannel, &gSndHeader1);

	// install the first callBackCmd
	myErr = SndSnip_InstallCallbackCmd(mySndChannel, 0, (long)&gSndHeader1);
	
	// configure the second sound header
	gSndHeader2.samplePtr = myPtr + myBufferSize;
	gSndHeader2.length = myBufferSize;
	gSndHeader2.sampleRate = myResourceInfo.sampleRate;
	gSndHeader2.loopStart = 0;
	gSndHeader2.loopEnd = 0;
	gSndHeader2.encode = stdSH;
	gSndHeader2.baseFrequency = myFreq;
	
	// install the second bufferCmd, to play the second chunk of the buffer
	myErr = SndSnip_InstallBufferCmd(mySndChannel, &gSndHeader2);

	// install the second callBackCmd
	myErr = SndSnip_InstallCallbackCmd(mySndChannel, 1, (long)&gSndHeader2);
	
bail:
	return(myErr);
}


//////////
//
// SndSnip_CallbackProc
// Handle callback messages. Keep it quick, okay?
//
//////////

PASCAL_RTN void SndSnip_CallbackProc (SndChannelPtr theChannel, SndCommand *theCommand)
{
#if TARGET_OS_MAC	
	long		myA5;
	
	myA5 = SetA5(theChannel->userInfo);
#endif	

	// in this sample code, param1 of theCommand is the 0-based index of the chunk of the buffer
	// that just finished playing, and param2 is a pointer to the sound header for that chunk
	gCurrChunkDirty = true;
	gCurrChunkIndex = theCommand->param1;
	gCurrChunkHeader = (SoundHeaderPtr)(theCommand->param2);
	gCurrChannel = theChannel;
		
#if TARGET_OS_MAC	
	myA5 = SetA5(myA5);
#endif	
}


//////////
//
// SndSnip_CheckBuffers
// Handle callback messages.
//
// This function needs to be called periodically. On MacOS, you could call this function
// every time thru your event loop. On Windows, you'll need to use a different strategy;
// for instance, you could install a timer that is called often enough that the bufferCmd
// gets installed before the currently-playing buffer is exhausted.
//
//////////

void SndSnip_CheckBuffers (void)
{
	if (gCurrChunkDirty) {

		// install next buffer of data, if there is one
		if (gCurrChunkIndex < (kNumberOfBufferChunks - 2)) {
		
			gCurrChunkHeader->samplePtr = gCurrChunkHeader->samplePtr + (2 * (gCurrChunkHeader->length));
			
			// install a bufferCmd command, to play the next buffer of data			
			SndSnip_InstallBufferCmd(gCurrChannel, gCurrChunkHeader);

			// install a callBackCmd command, to be called when this buffer is finished playing		
			SndSnip_InstallCallbackCmd(gCurrChannel, gCurrChunkIndex + 2, (long)gCurrChunkHeader);
		}
	}
	
	gCurrChunkDirty = false;
}


//////////
//
// SndSnip_InstallBufferCmd
// Install a bufferCmd sound command to play the specified buffer of data.
//
//////////

static OSErr SndSnip_InstallBufferCmd (SndChannelPtr theChannel, SoundHeaderPtr theHeaderPtr)
{
	SndCommand		mySndCommand;
	
	mySndCommand.cmd = bufferCmd;
	mySndCommand.param1 = 0;
	mySndCommand.param2 = (long)theHeaderPtr;
	return(SndDoCommand(theChannel, &mySndCommand, kWait));
}


//////////
//
// SndSnip_InstallCallbackCmd
// Install a callBackCmd sound command.
//
//////////

static OSErr SndSnip_InstallCallbackCmd (SndChannelPtr theChannel, short theParam1, long theParam2)
{
	SndCommand		mySndCommand;
	
	mySndCommand.cmd = callBackCmd;
	mySndCommand.param1 = theParam1;
	mySndCommand.param2 = theParam2;
	return(SndDoCommand(theChannel, &mySndCommand, kWait));
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Sample sound information routines.
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////

//////////
//
// SndSnip_GetSoundHeader
// Returns a pointer to the sound header in a sampled sound resource.
//
//////////

SoundHeaderPtr SndSnip_GetSoundHeader (Handle theSndHandle)
{
	SoundHeaderPtr		mySndHeader = NULL;
	long				myOffset = 0;
	OSErr				myErr = noErr;
	
	myErr = GetSoundHeaderOffset((SndListHandle)theSndHandle, &myOffset);
	if (myErr == noErr)
		mySndHeader = (SoundHeaderPtr)((long)*theSndHandle + myOffset);

	return(mySndHeader);
}


//////////
//
// SndSnip_GetSndBaseFrequency
// Returns the base frequency of a sampled sound.
//
//////////

long SndSnip_GetSndBaseFrequency (Handle theSndHandle)
{
	SoundHeaderPtr		mySndHeader;
	long				myBaseFreq = kMiddleC;		// a reasonable default
	
	mySndHeader = SndSnip_GetSoundHeader(theSndHandle);
	if (mySndHeader != NULL)
		myBaseFreq = mySndHeader->baseFrequency;

	return(myBaseFreq);
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Settings routines.
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////

//////////
//
// SndSnip_GetHardwareSettings
// Get the current hardware settings.
//
// Based on the function GetHardwareSettings by Kip Olson (develop, issue 24).
//
//////////

OSErr SndSnip_GetHardwareSettings (SndChannelPtr theChannel, SoundComponentData *theInfo)
{
	NumVersion			myVersion;
	OSErr				myErr = noErr;
	
	// SndGetInfo is available only in Sound Manager 3.1 or later,	
	// so check for an appropriate Sound Manager version
	myVersion = SndSoundManagerVersion();

	if (!SndSnip_CheckVersionNumber(&myVersion, 3, 1, 0))
		goto bail;
	
	myErr = SndGetInfo(theChannel, siNumberChannels, &theInfo->numChannels);
	if (myErr != noErr)
		goto bail;
	
	myErr = SndGetInfo(theChannel, siSampleRate, &theInfo->sampleRate);
	if (myErr != noErr)
		goto bail;
	
	myErr = SndGetInfo(theChannel, siSampleSize, &theInfo->sampleSize);
	if (myErr != noErr)
		goto bail;
	
	if (theInfo->sampleSize == 8)
		theInfo->format = kOffsetBinary;
	else
		theInfo->format = kTwosComplement;

bail:		
	return(myErr);
}


//////////
//
// SndSnip_GetAudioSettings
// Get the settings for the first audio track in the specified movie.
//
//////////

OSErr SndSnip_GetAudioSettings (Movie theMovie, SoundComponentData *theInfo)
{
	Track						myTrack = NULL;
	Media						myMedia = NULL;
	SoundDescriptionHandle		myDesc = NULL;
	OSErr						myErr = paramErr;
	
	// get the first sound track in the specified movie
	myTrack = GetMovieIndTrackType(theMovie, 1, AudioMediaCharacteristic, movieTrackCharacteristic | movieTrackEnabledOnly);
	if (myTrack == NULL)
		goto bail;
	
	myMedia = GetTrackMedia(myTrack);
	if (myMedia == NULL)
		goto bail;

	myDesc = (SoundDescriptionHandle)NewHandle(0);
	if (myDesc == NULL)
		goto bail;

	GetMediaSampleDescription(myMedia, 1, (SampleDescriptionHandle)myDesc);
	if (GetHandleSize((Handle)myDesc) == 0)
		goto bail;

	theInfo->numChannels = (**myDesc).numChannels;
	theInfo->sampleRate = (**myDesc).sampleRate;
	theInfo->sampleSize = (**myDesc).sampleSize;
	theInfo->format = (**myDesc).dataFormat;
	
	myErr = noErr;
	
bail:

	if (myDesc != NULL)
		DisposeHandle((Handle)myDesc);
		
	return(myErr);
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Sound information routines.
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////

//////////
//
// SndSnip_HasSoundManager3_1
// Returns true if the installed version of the Sound Manager is at least 3.1.
//
//////////

Boolean SndSnip_HasSoundManager3_1 (void)
{
	NumVersionVariant		myVersion;
	
	myVersion.parts = SndSoundManagerVersion();
	return(myVersion.whole >= 0x03100000);
}


//////////
//
// SndSnip_CheckVersionNumber
// Returns true if the given version number is compatible with
// (that is, not older than) version theMajor.theMinor.theBug.
//
//////////

Boolean SndSnip_CheckVersionNumber (
	const NumVersion		*theVersion,
	UInt8					theMajor,
	UInt8					theMinor,
	UInt8					theBug)
{
	if (theVersion->majorRev != theMajor)
		return(theVersion->majorRev > theMajor);
	else
		return(theVersion->minorAndBugRev >= theMinor << 4 | theBug);
}


//////////
//
// SndSnip_GetVolume
// Get the current left and right volumes of a sound channel.
//
// Based on the function GetVolume by Kip Olson (develop, issue 24).
//
//////////

OSErr SndSnip_GetVolume (SndChannelPtr theChannel, unsigned short *theLeftVol, unsigned short *theRightVol)
{
	SndCommand			mySndCommand;
	unsigned long		myVolume;
	OSErr				myErr = noErr;

	// getVolumeCmd is available only in Sound Manager version 3.0 and later;
	// you might want to check for a correct version here

	mySndCommand.cmd = getVolumeCmd;
	mySndCommand.param1 = 0;
	mySndCommand.param2 = (long)&myVolume;
	myErr = SndDoImmediate(theChannel, &mySndCommand);
	
	if (myErr == noErr) {
		*theLeftVol = myVolume & 0x0000ffff;
		*theRightVol = myVolume >> 16;
	}
	
	return(myErr);
}


//////////
//
// SndSnip_SetVolume
// Set the left and right volumes of a sound channel.
//
// Based on the function SetVolume by Kip Olson (develop, issue 24).
//
//////////

OSErr SndSnip_SetVolume (SndChannelPtr theChannel, unsigned short theLeftVol, unsigned short theRightVol)
{
	SndCommand			mySndCommand;
	OSErr				myErr = noErr;

	// volumeCmd is available only in Sound Manager version 3.0 and later;
	// you might want to check for a correct version here
		
	mySndCommand.cmd = volumeCmd;
	mySndCommand.param1 = 0;
	mySndCommand.param2 = (theRightVol << 16) | theLeftVol;
	
	myErr = SndDoImmediate(theChannel, &mySndCommand);
	return(myErr);
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Movie export component routines.
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////

//////////
//
// SndSnip_ConvertWAVEFormats
// Convert an 8-bit, 22 kHz WAVE file to a 16-bit, 22 kHz IMAPCM WAVE file.
//
//////////

OSErr SndSnip_ConvertWAVEFormats (Movie theMovie, FSSpec *theFile)
{
	ComponentInstance			myComponent = NULL;
	SoundDescriptionHandle		myDesc = NULL;
	ComponentResult				myErr = badComponentType;

	// open a movie export component
	myComponent = OpenDefaultComponent(MovieExportType, kQTFileTypeWave);
	if (myComponent == NULL)
		goto bail;

	// create and fill in a sound description
	myDesc = (SoundDescriptionHandle)NewHandleClear(sizeof(SoundDescription));
	if (myDesc == NULL) {
		myErr = MemError();
		goto bail;
	}
	
	(**myDesc).descSize = sizeof(SoundDescription);
	(**myDesc).numChannels = 2;
	(**myDesc).sampleSize = 16;
	(**myDesc).sampleRate = rate22050hz;
	(**myDesc).dataFormat = k16BitLittleEndianFormat;
	
	// tell the export component to use the specified audio characteristics
	myErr = MovieExportSetSampleDescription(myComponent, (SampleDescriptionHandle)myDesc, SoundMediaType);
	if (myErr != noErr)
		goto bail;
		
	// export the movie into a file
	myErr = ConvertMovieToFile(	
						theMovie,					// the movie to convert
						NULL,						// all tracks in the movie
						theFile,					// the output file
						kQTFileTypeWave,			// the output file type
						FOUR_CHAR_CODE('TVOD'),		// the output file creator
						smSystemScript,				// the script
						NULL, 						// no resource ID to be returned
						0L,							// no flags
						myComponent);				// the export component

bail:
	// dispose of any storage we allocated
	if (myComponent != NULL)
		CloseComponent(myComponent);
		
	if (myDesc != NULL)
		DisposeHandle((Handle)myDesc);

	return((OSErr)myErr);
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Standard sound compression dialog routines.
//
// These routines illustrate how to use QuickTime's standard sound compression dialog routines
// to get compression settings from the user and to compress a sound using those settings. See
// Chapter 20 of QuickTime 3 Reference for information on the standard sound compression dialog
// routines.
//
// These routines show how to display the standard sound compression dialog box directly; you can
// also display the dialog box indirectly by using ConvertMovieToFile with the showUserSettingsDialog
// flag set; see SndSnip_SaveSoundTrackAsAnyTypeFile above for an example.
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////

//////////
//
// SndSnip_PromptUserForAudioFileAndCompress
// Let the user select an audio file and select its compression settings; then compress it.
//
//////////

void SndSnip_PromptUserForAudioFileAndCompress (void)
{
	SFTypeList					myTypeList;
	StandardFileReply			myReply;
	short						myRefNum = 0;
	SoundComponentData			mySrcInfo;
	SoundComponentData			myDstInfo;
	unsigned long				myNumFrames = 0;
	unsigned long				myDataOffset = 0;
	ComponentInstance			myComponent = NULL;
	OSErr						myErr = noErr;

	//////////
	//
	// have the user select an AIFF or AIFF-C file
	//
	//////////

	myTypeList[0] = kQTFileTypeAIFF;
	myTypeList[1] = kQTFileTypeAIFC;

	StandardGetFilePreview(NULL, 2, myTypeList, &myReply);
	if (!myReply.sfGood)
		goto bail;
	
	//////////
	//
	// open the selected sound file and get the characteristics of the sound
	//
	//////////
	
	myErr = FSpOpenDF(&myReply.sfFile, fsCurPerm, &myRefNum);
	if (myErr != noErr)
		goto bail;

	myErr = ParseAIFFHeader(myRefNum, &mySrcInfo, &myNumFrames, &myDataOffset);
	if (myErr != noErr)
		goto bail;
	
	// zero out fields we're not interested in
	mySrcInfo.flags = 0L;
	mySrcInfo.sampleCount = 0L;
	mySrcInfo.buffer = NULL;
	mySrcInfo.reserved = 0L;
	
	//////////
	//
	// display the standard sound compression dialog box
	//
	//////////
	
	// open the standard compression dialog component
	myComponent = OpenDefaultComponent(StandardCompressionType, StandardCompressionSubTypeSound);
	if (myComponent == NULL)
		goto bail;
			
	// set the initial values for the dialog box, using the current characteristics of the selected sound
	SCSetInfo(myComponent, scSoundSampleRateType, &(mySrcInfo.sampleRate));
	SCSetInfo(myComponent, scSoundSampleSizeType, &(mySrcInfo.sampleSize));
	SCSetInfo(myComponent, scSoundChannelCountType, &(mySrcInfo.numChannels));
	SCSetInfo(myComponent, scSoundCompressionType, &(mySrcInfo.format));
	
	// request sound compression settings from the user; in other words, put up the dialog box
	// (don't let "Image" in the name here fool you: it's just a relic from the fact that the
	// sound compression dialog routines were based on the existing image compression dialog routines)
	myErr = SCRequestImageSettings(myComponent);
	if (myErr == userCanceledErr)
		goto bail;

	//////////
	//
	// retrieve the compression/conversion settings selected by the user
	//
	//////////
	
	SCGetInfo(myComponent, scSoundSampleRateType, &(myDstInfo.sampleRate));
	SCGetInfo(myComponent, scSoundSampleSizeType, &(myDstInfo.sampleSize));
	SCGetInfo(myComponent, scSoundChannelCountType, &(myDstInfo.numChannels));
	SCGetInfo(myComponent, scSoundCompressionType, &(myDstInfo.format));

	// any non-compressed format appears as 'raw ', which is not what the Sound Manager expects
	if (myDstInfo.format == k8BitOffsetBinaryFormat)
		myDstInfo.format = kSoundNotCompressed;

	// zero out fields we're not interested in
	myDstInfo.flags = 0L;
	myDstInfo.sampleCount = 0L;
	myDstInfo.buffer = NULL;
	myDstInfo.reserved = 0L;
	
	//////////
	//
	// save the compressed sound in a new file
	//
	//////////
	
	SndSnip_PromptUserForDiskFileAndSaveCompressed(myRefNum, &mySrcInfo, &myDstInfo, myDataOffset, myNumFrames);
	
bail:

	if (myComponent != NULL)
		CloseComponent(myComponent);
		
	if (myRefNum != 0)
		FSClose(myRefNum);
}


//////////
//
// SndSnip_PromptUserForDiskFileAndSaveCompressed
// Let the user select a new disk file, then convert the specified audio data into that file.
//
// This routine uses the Sound Converter routines introduced in Sound Manager 3.2.
//
//////////

void SndSnip_PromptUserForDiskFileAndSaveCompressed (short theSrcRefNum, SoundComponentData *theSrcInfo, SoundComponentData *theDstInfo, unsigned long theSrcDataOffset, unsigned long theSrcNumFrames)
{
	StandardFileReply			myReply;
	SoundConverter				myConverter = NULL;
	unsigned long				myNumInputFrames, myNumInputBytes, myNumOutputBytes;
	Ptr							myInputBuffer = NULL;
	Ptr							myOutputBuffer = NULL;
	short						myDstRefNum = 0;
	StringPtr 					myPrompt = QTUtils_ConvertCToPascalString(kSaveSoundPrompt);
	StringPtr 					myFileName = QTUtils_ConvertCToPascalString(kSaveSoundFileName);
	OSErr						myErr = noErr;

	//////////
	//
	// prompt the user for a file to put the compressed sound into
	//
	//////////
	
	StandardPutFile(myPrompt, myFileName, &myReply);
	if (!myReply.sfGood)
		goto bail;

	if (myReply.sfReplacing) {
		myErr = FSpDelete(&myReply.sfFile);
		if (myErr != noErr)
			goto bail;
	}

	//////////
	//
	// open a sound converter
	//
	//////////
	
	// perform in non-real time for best quality
	theDstInfo->flags |= kNoRealtimeProcessing;			

	myErr = SoundConverterOpen(theSrcInfo, theDstInfo, &myConverter);
	if (myErr != noErr)
		goto bail;

	myErr = SoundConverterGetBufferSizes(myConverter, kNumberOfTargetBytes, &myNumInputFrames, &myNumInputBytes, &myNumOutputBytes);
	if (myErr != noErr)
		goto bail;

	//////////
	//
	// convert the sound to the desired output format
	//
	//////////
	
	// create the input and output buffers
	myInputBuffer = NewPtrClear(myNumInputBytes);
	if (myInputBuffer == NULL)
		goto bail;

	myOutputBuffer = NewPtrClear(myNumOutputBytes);
	if (myOutputBuffer == NULL)
		goto bail;

	// create and open the output file
	myErr = FSpCreate(&myReply.sfFile, FOUR_CHAR_CODE('TVOD'), kQTFileTypeAIFC, smSystemScript);
	if (myErr != noErr)
		goto bail;

	myErr = FSpOpenDF(&myReply.sfFile, fsRdWrPerm, &myDstRefNum);
	if (myErr != noErr)
		goto bail;

	SetEOF(myDstRefNum, 0L);
	
	// convert the audio data
	myErr = SoundConverterBeginConversion(myConverter);
	if (myErr == noErr) {
		long				myInputFileSize = 0L;
		unsigned long		myActOutputFrames = 0L;
		unsigned long		myActOutputSize = 0L;
		unsigned long		myAddOutputFrames = 0L;
		unsigned long		myAddOutputSize = 0L;
		long				myCurrentFrames = 0L;
		long				myNumFramesLeft = theSrcNumFrames;
		long				myBytesRead = 0L;
		long				myBytesWritten = 0L;
		long				myFramesWritten = 0L;
		
		// set up the input file for reading
		GetEOF(theSrcRefNum, &myInputFileSize);

		myErr = SetFPos(theSrcRefNum, fsFromStart, theSrcDataOffset);
		if (myErr != noErr)
			goto bail;

		myInputFileSize -= theSrcDataOffset;

		// set up a preliminary AIFF header for the output file;
		// we'll update the actual values later....
		myErr = SetupAIFFHeader(myDstRefNum,
								theDstInfo->numChannels,
								theDstInfo->sampleRate,
								theDstInfo->sampleSize,
								theDstInfo->format,
								0L,
								0L);
		if (myErr != noErr)
			goto bail;
		
		myCurrentFrames = myNumInputFrames;
		
		// loop through buffers of size myNumInputFrames
		while (myNumFramesLeft > 0) {
		
			// read a buffer of data from the input file		
			myBytesRead = myNumInputBytes;
			myErr = FSRead(theSrcRefNum, &myBytesRead, myInputBuffer);
			if ((myErr != noErr) && (myErr != eofErr))
				goto bail;

			// convert the buffer
			myErr = SoundConverterConvertBuffer(myConverter,
												myInputBuffer,
												myCurrentFrames,
												myOutputBuffer,
												&myActOutputFrames,
												&myActOutputSize);
			if (myErr != noErr)
				goto bail;

			// write the converted data into the output file
			myErr = FSWrite(myDstRefNum, (long *)&myActOutputSize, myOutputBuffer);
			if (myErr != noErr)
				goto bail;
	
			myBytesWritten += myActOutputSize;
			myFramesWritten += myActOutputFrames;
		
			// calculate remaining frames and adjust myCurrentFrames for last buffer
			myNumFramesLeft -= myCurrentFrames;
			if (myNumFramesLeft < myNumInputFrames)
				myCurrentFrames = myNumFramesLeft;
		}

		// end the conversion, and see if we get back a few more bytes of data	
		myErr = SoundConverterEndConversion(myConverter, myOutputBuffer, &myAddOutputFrames, &myAddOutputSize);
		if (myErr != noErr)
			goto bail;
	
		// if we got any more data, write it to disk	
		if (myAddOutputFrames != 0L) {
			myErr = FSWrite(myDstRefNum, (long *)&myAddOutputSize, myOutputBuffer);
			if (myErr != noErr)
				goto bail;
		}
		
		myErr = SetFPos(myDstRefNum, fsFromStart, 0);
		if (myErr != noErr)
			goto bail;
	
		// update AIFF header
		myErr = SetupAIFFHeader(
							myDstRefNum,
							theDstInfo->numChannels,
							theDstInfo->sampleRate,
							theDstInfo->sampleSize,
							theDstInfo->format,
							myBytesWritten + myAddOutputSize,
							myFramesWritten + myAddOutputFrames);
	}

bail:

	free(myPrompt);
	free(myFileName);

	if (myConverter != NULL)
		SoundConverterClose(myConverter);	

	if (myInputBuffer != NULL)
		DisposePtr(myInputBuffer);

	if (myOutputBuffer != NULL)
		DisposePtr(myOutputBuffer);
		
	if (myDstRefNum != 0)
		FSClose(myDstRefNum);

#if TARGET_OS_MAC
	// make sure the file data gets written out to disk
	FlushVol("\p", myReply.sfFile.vRefNum);
#endif
}
