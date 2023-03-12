//////////
//
//	File:		SoundConverter.c
//
//	Contains:	Sound format conversion sample code.
//
//	Written by:	Bob Aron
//	Revised by:	Tim Monroe
//
//	Copyright:	© 1999 by Apple Computer, Inc., all rights reserved.
//
//	Change History (most recent first):
//
//	   <4>	 	03/20/00	rtm		made changes to get things running under CarbonLib
//	   <3>	 	07/07/99	rtm		final tweaks
//	   <2>	 	07/02/99	rtm		fixed problem causing crashes on Windows
//	   <1>	 	07/01/99	rtm		first file from Bob Aron; conversion to personal coding style
//	 
//	This is a simple application that demonstrates how to convert some uncompressed audio into any compression
//	format supported by QuickTime 4.0 and Sound Manager 3.4. There are two routines of interest in this file:
//	SndConv_ConvertSomeUncompressedAudio and SndConv_CreateSoundMovie.
//	
//	SndConv_ConvertSomeUncompressedAudio creates a 16-bit, mono, 44.1 kHz sine wave to use as source audio data
//	and then converts it to whatever format is desired. The output format is hard-coded at the beginning of the function
//	SndConv_DriveAudioConversion. SndConv_ConvertSomeUncompressedAudio places the converted data into a handle and then
//	calls SndConv_CreateSoundMovie to add the data to a newly-created movie.
//
//////////

#include "SoundConverter.h"


//////////
//
// SndConv_DriveAudioConversion
// Create some sound data and compress it into a file.
//
//////////

void SndConv_DriveAudioConversion (void)
{
	Handle					mySourceHandle,								
							myDestHandle;
	Handle					myDestCompParamsHandle = NULL;			
	SoundComponentData		mySourceInfo,								
							myDestInfo;
	unsigned long			mySourceTotalFrames,
							myDestTotalFrames;
	short					myResRefNum = -1;
	CompressionInfo			myDestCompressionInfo;
	Movie					myMovie = NULL;
	FSSpec					myFile;
	Boolean					myIsSelected = false;
	Boolean					myIsReplacing = false;	
	long					myFlags = createMovieFileDeleteCurFile | createMovieFileDontCreateResFile;
	StringPtr 				myPrompt = QTUtils_ConvertCToPascalString(kSaveSoundPrompt);
	StringPtr 				myFileName = QTUtils_ConvertCToPascalString(kSaveSoundFileName);
	OSErr					myErr = noErr;

	//////////
	//
	// allocate handles to hold the uncompressed (source) and compressed (destination) sound data
	//
	//////////

	mySourceHandle = NewHandleClear(0);
	myDestHandle = NewHandleClear(0);
	
	if ((mySourceHandle == NULL) || (myDestHandle == NULL))
		goto Bail;
		
	// create some uncompressed, 16-bit, mono, 44.1 kHz sound data; also, fill in the mySourceInfo structure
	// and return the total number of frames, which we'll use later
	myErr = SndConv_UncompressedSineWaveToHandle(mySourceHandle, &mySourceInfo, &mySourceTotalFrames);
	FailIf(myErr != noErr, Bail);
	
	//////////
	//
	// convert the source data
	//
	//////////

	// fill in the myDestInfo fields with the data format that you want to convert to
	myDestInfo.flags |= kNoRealtimeProcessing;		// perform in non-real time for best quality (this can also be set to zero)		
	myDestInfo.format = k16BitBigEndianFormat;		// data format
	myDestInfo.numChannels = 1;						// number of channels (1 for mono, 2 for stereo)
	myDestInfo.sampleSize = 16;						// sample size (may be left at 16 for all formats except k8BitOffsetBinaryFormat)
	myDestInfo.sampleRate = rate44khz;				// sample rate
	myDestInfo.sampleCount = 0;						// leave set to 0
	myDestInfo.buffer = NULL;						// leave set to NULL
	myDestInfo.reserved = 0;						// leave set to 0
	
	// convert the source data
	myErr = SndConv_ConvertSomeUncompressedAudio(	mySourceHandle,
													mySourceInfo,
													mySourceTotalFrames,
													myDestHandle,
													myDestInfo, 
								 					&myDestTotalFrames,
								 					&myDestCompressionInfo,
								 					&myDestCompParamsHandle);
	FailIf(myErr != noErr, Bail);

	//////////
	//
	// create a new movie to hold the converted sound data
	//
	//////////

	// elicit a location from the user
	QTFrame_PutFile(myPrompt, myFileName, &myFile, &myIsSelected, &myIsReplacing);
	if (!myIsSelected)
		goto Bail;
	
	if (myIsReplacing) {
		myErr = FSpDelete(&myFile);
		if (myErr != noErr)
			goto Bail;
	}

	myErr = CreateMovieFile(&myFile, FOUR_CHAR_CODE('TVOD'), smSystemScript, myFlags, &myResRefNum, &myMovie);
	FailIf(myErr != noErr, Bail);

	// put the newly-converted data into the movie
	myErr = SndConv_CreateSoundMovie(	myDestHandle,
										myResRefNum,
										myMovie,
										myDestInfo,
										&myDestCompParamsHandle, 
										myDestCompressionInfo,
										myDestTotalFrames);
	FailIf(myErr != noErr, Bail);	

Bail:

	free(myPrompt);
	free(myFileName);
	
	if (myMovie != 0)
		DisposeMovie(myMovie);
		
	if (myResRefNum != -1)
		CloseMovieFile(myResRefNum);

	if (myDestHandle != NULL)
		DisposeHandle(myDestHandle);
		
	if (mySourceHandle != NULL)
		DisposeHandle(mySourceHandle);
		
	if (myDestCompParamsHandle != NULL)
		DisposeHandle(myDestCompParamsHandle);
	
	return;
}


//////////
//
// SndConv_ConvertSomeUncompressedAudio
// Convert the specified sound data.
//
// This routine expects uncompressed, 16-bit mono data as source data.
//
//////////


OSErr SndConv_ConvertSomeUncompressedAudio (	Handle theSourceHandle,
												SoundComponentData theSourceInfo,
												unsigned long theSourceTotalFrames, 
												Handle theDestHandle,
												SoundComponentData theDestInfo,
												unsigned long *theDestFramesMoved, 
												CompressionInfo *theDestCompInfo,
												Handle *theDestCompParams)
{
	SoundConverter		myConverter;
	Handle				myDestCompParamsHandle = NULL;
	unsigned long		myNumFramesLeft,
						mySourceFrames, 
						myDestFrames,
						mySourceBytes, 
						myDestBytes,
						myDataOffset = 0;
	Ptr					myDestPtr = NULL;
	UInt16				hasOptionsDialog = 0;
	short 				mySourceBytesPerFrame = 2;		// 16-bit mono uncompressed data is 2 bytes per frame
	OSErr				myErr = noErr;

	//////////
	//
	// open a sound converter
	//
	//////////
	
	myErr = SoundConverterOpen(&theSourceInfo, &theDestInfo, &myConverter);						
	if (myErr != noErr)
		goto Bail;

	// see if the destination format has an options dialog and open if it does
	myErr = SoundConverterGetInfo(myConverter, siOptionsDialog, &hasOptionsDialog);
	if ((myErr == noErr) && hasOptionsDialog) {	
		myErr = SoundConverterSetInfo(myConverter, siOptionsDialog, NULL);
		FailIf(myErr != noErr, Bail);
	}
	
	myErr = SoundConverterSetInfo(myConverter, siCompressionChannels, &theDestInfo.numChannels);		
	// ignore this error, since some codecs don't use this selector (makes QDesign work)

	//////////
	//
	// get the sound converter-specific settings.
	//
	//////////

	// Not all sound converters have custom settings; in this case myDestCompParamsHandle will be unchanged.
	// we need to return these settings to the caller, as they are required by the decompressor to be able
	// to decompress the compressed audio. If this audio is stored in a QuickTime movie, these parameters
	// will be stored in a SoundDescriptionExtension of type siDecompressionParams.
	myErr = SoundConverterGetInfo(myConverter, siCompressionParams, &myDestCompParamsHandle);
	
	// if any compression params were passed back, send it to the sound converter now
	if (myErr == noErr) {
		HLockHi(myDestCompParamsHandle);
		myErr = SoundConverterSetInfo(myConverter, siCompressionParams, *myDestCompParamsHandle);			
		HUnlock(myDestCompParamsHandle);
		FailIf(myErr != noErr, Bail);
	} else {
		// no audio atom list to deal with, so set to NULL
		myDestCompParamsHandle = NULL;
	}

	// get sound converter buffer size info
	myErr = SoundConverterGetBufferSizes(myConverter, kMaxBufferSize, &mySourceFrames, &mySourceBytes, &myDestBytes);	
	FailIf(myErr != noErr, Bail);
	
	// create destination data buffer
	myDestPtr = NewPtrClear(myDestBytes);												
	myErr = MemError();
	FailIf(myErr != noErr, Bail);

	//////////
	//
	// convert the sound to the desired output format
	//
	//////////

	myErr = SoundConverterBeginConversion(myConverter);										
	FailIf(myErr != noErr, Bail);

	// get info about destination compression
	// We need to return this information to the caller as they are required
	// in order to put this compressed audio into a QuickTime movie. This information
	// will go into the Version 1 Sound Description Handle.

	myErr = SoundConverterGetInfo(myConverter, siCompressionFactor, theDestCompInfo);					
	if (myErr != noErr) {
		myErr = GetCompressionInfo(fixedCompression, theDestInfo.format, theDestInfo.numChannels, theDestInfo.sampleSize, theDestCompInfo);	
		FailIf(myErr != noErr, Bail);
	}

	// myBytesPerFrame is not filled in by GetInfo, so we set it here
	theDestCompInfo->bytesPerFrame = theDestCompInfo->bytesPerPacket * theDestInfo.numChannels;	
	
	// initialize destination total frame count to zero
	*theDestFramesMoved = 0;																														
	
	myNumFramesLeft = theSourceTotalFrames;
	HLockHi(theSourceHandle);

	// loop through buffers of size mySourceFrames
	while (myNumFramesLeft > 0) {
		// if there are fewer frames remaining than our source buffer size,
		// we're near the end of our data, so get what's remaining
		if (myNumFramesLeft < mySourceFrames)																						
			mySourceFrames = myNumFramesLeft;											

		myErr = SoundConverterConvertBuffer(myConverter, *theSourceHandle + myDataOffset, mySourceFrames, myDestPtr, &myDestFrames, &myDestBytes);
		FailIf(myErr != noErr, Bail);

		// place the converted data into a handle
		myErr = PtrAndHand(myDestPtr, theDestHandle, myDestBytes);
		FailIf(myErr != noErr, Bail);

		// move offset to appropriate place in source data
		myDataOffset += mySourceFrames * mySourceBytesPerFrame;
		
		// keep track of total frames returned by converter
		*theDestFramesMoved += myDestFrames;

		myNumFramesLeft -= mySourceFrames;
	}

	// end the conversion, and see if we get back a few more bytes of data	
	myErr = SoundConverterEndConversion(myConverter, myDestPtr, &myDestFrames, &myDestBytes);		
	FailIf(myErr != noErr, Bail);

	HUnlock(theSourceHandle);

	// place any leftover converted data into the handle
	myErr = PtrAndHand(myDestPtr, theDestHandle, myDestBytes);
	FailIf(myErr != noErr, Bail);

	*theDestFramesMoved += myDestFrames;	

Bail:

	*theDestCompParams = myDestCompParamsHandle;

	if (myDestPtr != NULL)
		DisposePtr(myDestPtr);
		
	if (myConverter != NULL)
		SoundConverterClose(myConverter);
		
	return(myErr);
}


//////////
//
// SndConv_CreateSoundMovie
// Create a sound movie from the specified handle of audio data.
//
//////////

OSErr SndConv_CreateSoundMovie (	Handle theDestAudioData,
									short theMovieRefNum, 
									Movie theMovie,
									SoundComponentData theDestInfo, 
									Handle *theDestCompParams,
									CompressionInfo theDestCompInfo, 
									unsigned long theDestFrameCount)
{
	Track						myTrack = NULL;
	Media						myMedia = NULL;
	TimeScale					myTimeScale;
	SoundDescriptionV1Handle	mySampleDesc = NULL;
	unsigned long				myLengthInSamples;
	OSErr						myErr = noErr;
		
	//////////
	//				
	// create the movie track and media
	//				
	//////////

	myTrack = NewMovieTrack(theMovie, 0, 0, kFullVolume);					
	myErr = GetMoviesError();
	FailIf(myErr != noErr, Bail);

	myTimeScale = (theDestInfo.sampleRate >> 16);
	
	// set new track to be a sound track
	myMedia = NewTrackMedia(myTrack, SoundMediaType, myTimeScale, NULL, 0); 	
	myErr = GetMoviesError();	
	FailIf(myErr != noErr, Bail);

	// start a media editing session
	myErr = BeginMediaEdits(myMedia);										
	FailIf(myErr != noErr, Bail);
	
	//////////
	//				
	// create a sound sample description
	//				
	//////////

	// use the SoundDescription format 1 because it adds fields for data size information
	// and is required by AddSoundDescriptionExtension if an extension is required for the compression format
	mySampleDesc = (SoundDescriptionV1Handle)NewHandleClear(sizeof(SoundDescriptionV1));	
	myErr = MemError();
	FailIf(myErr != noErr, Bail);
	
	// fill in the fields of the sample description
	(*mySampleDesc)->desc.descSize			= sizeof(SoundDescriptionV1);
	(*mySampleDesc)->desc.dataFormat		= theDestInfo.format;						
	(*mySampleDesc)->desc.resvd1			= 0;									
	(*mySampleDesc)->desc.resvd2			= 0;									
	(*mySampleDesc)->desc.dataRefIndex		= 1;									
	(*mySampleDesc)->desc.version			= 1;									
	(*mySampleDesc)->desc.revlevel			= 0;									
	(*mySampleDesc)->desc.vendor			= 0;									
	(*mySampleDesc)->desc.numChannels		= theDestInfo.numChannels;					
	(*mySampleDesc)->desc.sampleSize		= theDestInfo.sampleSize;					 
	(*mySampleDesc)->desc.compressionID		= 0;									
	(*mySampleDesc)->desc.packetSize		= 0;									
	(*mySampleDesc)->desc.sampleRate		= theDestInfo.sampleRate;
	(*mySampleDesc)->samplesPerPacket 		= theDestCompInfo.samplesPerPacket;
	(*mySampleDesc)->bytesPerPacket 		= theDestCompInfo.bytesPerPacket;
	(*mySampleDesc)->bytesPerFrame 			= theDestCompInfo.bytesPerFrame;
	(*mySampleDesc)->bytesPerSample 		= theDestCompInfo.bytesPerSample;

	// not all compression formats have compression params, so we only need to add a
	// sound description extension for those that do 
	if (*theDestCompParams != NULL)
		AddSoundDescriptionExtension((SoundDescriptionHandle)mySampleDesc, *theDestCompParams, siDecompressionParams);

	//////////
	//				
	// add samples to the media
	//				
	//////////

	myLengthInSamples = theDestFrameCount * theDestCompInfo.samplesPerPacket;
	myErr = AddMediaSample(	myMedia,
							theDestAudioData,
							0,
							theDestFrameCount * theDestCompInfo.bytesPerFrame,
							1, 
						 	(SampleDescriptionHandle)mySampleDesc,
						 	myLengthInSamples,
						 	0,
						 	NULL);
	FailIf(myErr != noErr, Bail);
		

	myErr = EndMediaEdits(myMedia);
	FailIf(myErr != noErr, Bail);

	//////////
	//				
	// add the media to the track
	//				
	//////////

	myErr = InsertMediaIntoTrack(myTrack, 0, 0, GetMediaDuration(myMedia), FixRatio(1, 1));
	FailIf(myErr != noErr, Bail);

	myErr = AddMovieResource(theMovie, theMovieRefNum, NULL, NULL);
	FailIf(myErr != noErr, Bail);
		
Bail:
	if (mySampleDesc != NULL)	
		DisposeHandle((Handle)mySampleDesc);
		
	return(myErr);
}


//////////
//
// SndConv_UncompressedSineWaveToHandle
// Create some 16-bit, monophonic, 44.1kHz data and stick it into a handle.
//
//////////

OSErr SndConv_UncompressedSineWaveToHandle (Handle theData, SoundComponentData *theCompInfo, unsigned long *theTotalFrames)
{
	long			myIndex, myByteCounter;
	double_t		mySinResult, myRate;
	long			myPlotMe;
	float			myPi = 3.14159;
	Byte			*myBufferPtr = NULL;
	Byte			*myStartOfBufferPtr = NULL;
	unsigned long	myBytesCount = 0;
	long			myFramesInBuffer = 10240;
	long 			myHz = kConcertA,
					mySeconds = 3;
	short			myBytesPerFrame;
	OSErr			myErr = noErr;

    *theTotalFrames = 0;
	
	theCompInfo->flags = 0;
	theCompInfo->format = k16BitBigEndianFormat;
	theCompInfo->numChannels = 1;
	theCompInfo->sampleSize = 16;
	theCompInfo->sampleRate = rate44khz;
	theCompInfo->sampleCount = mySeconds * (theCompInfo->sampleRate >> 16);
	theCompInfo->buffer = NULL;
	theCompInfo->reserved = 0;
	
	myBytesPerFrame = theCompInfo->sampleSize / 8;

	myBufferPtr = (Byte *)NewPtrClear(myFramesInBuffer * myBytesPerFrame);	
	FailWithAction(myBufferPtr == NULL, myErr = memFullErr, Bail);
	
	myStartOfBufferPtr = myBufferPtr;
		
	myRate = theCompInfo->sampleRate / 65536.0;

	//////////
	//  		2 ¹ hz			
	//	(sin(i * ------)) * samplesize
	//			 rate																							
	//////////

	for (myIndex = 0; myIndex < theCompInfo->sampleCount; myIndex++) {
		mySinResult = sin(myIndex * ((2 * myPi * myHz) / myRate));
		myPlotMe = mySinResult * ((1 << (theCompInfo->sampleSize - 1)) - 1);								
					
		for (myByteCounter = 1; myByteCounter <= (theCompInfo->sampleSize / 8); myByteCounter++, myBytesCount++, myBufferPtr++) {
			*myBufferPtr = myPlotMe >> (theCompInfo->sampleSize - (myByteCounter * 8));
		}
		
		if (myBytesCount == myFramesInBuffer * myBytesPerFrame) {
			myErr = PtrAndHand(myStartOfBufferPtr, theData, myBytesCount);
			FailIf(myErr != noErr, Bail);
				
			*theTotalFrames += myBytesCount / myBytesPerFrame;						
			myBytesCount = 0;													// reset bytes count
			myBufferPtr = myStartOfBufferPtr;									// reset myBufferPtr
		}
	}

	myErr = PtrAndHand(myStartOfBufferPtr, theData, myBytesCount);
	FailIf(myErr != noErr, Bail);

	*theTotalFrames += myBytesCount / myBytesPerFrame;
	
Bail:
	if (myBufferPtr != NULL)
		DisposePtr((Ptr)myBufferPtr);
		
	return(myErr);
}


