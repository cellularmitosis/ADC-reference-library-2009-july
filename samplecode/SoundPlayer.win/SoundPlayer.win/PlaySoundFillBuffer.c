/*
	File:		PlaySoundFillBuffer.c
	
	Description: This sample demonstates how to use the Sound Description Extention atom information with the
				 SoundConverterFillBuffer API to play VBR and Non-VBR audio encodings. Originally available as
				 part of the MP3 Player sample this file has been updated to support AAC added in QT 6.0
				 
				 NOTE: This Sample Requires QuickTime 6.0 or greater.

	Author:		mc, era, srk
	
	Version 	1.5

	Copyright: 	© Copyright 2000-2002 Apple Computer, Inc. All rights reserved.
	
	Disclaimer:	IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc.
				("Apple") in consideration of your agreement to the following terms, and your
				use, installation, modification or redistribution of this Apple software
				constitutes acceptance of these terms.  If you do not agree with these terms,
				please do not use, install, modify or redistribute this Apple software.

				In consideration of your agreement to abide by the following terms, and subject
				to these terms, Apple grants you a personal, non-exclusive license, under Apple’s
				copyrights in this original Apple software (the "Apple Software"), to use,
				reproduce, modify and redistribute the Apple Software, with or without
				modifications, in source and/or binary forms; provided that if you redistribute
				the Apple Software in its entirety and without modifications, you must retain
				this notice and the following text and disclaimers in all such redistributions of
				the Apple Software.  Neither the name, trademarks, service marks or logos of
				Apple Computer, Inc. may be used to endorse or promote products derived from the
				Apple Software without specific prior written permission from Apple.  Except as
				expressly stated in this notice, no other rights or licenses, express or implied,
				are granted by Apple herein, including but not limited to any patent rights that
				may be infringed by your derivative works or by other works in which the Apple
				Software may be incorporated.

				The Apple Software is provided by Apple on an "AS IS" basis.  APPLE MAKES NO
				WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED
				WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS FOR A PARTICULAR
				PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND OPERATION ALONE OR IN
				COMBINATION WITH YOUR PRODUCTS.

				IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL OR
				CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
				GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
				ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION, MODIFICATION AND/OR DISTRIBUTION
				OF THE APPLE SOFTWARE, HOWEVER CAUSED AND WHETHER UNDER THEORY OF CONTRACT, TORT
				(INCLUDING NEGLIGENCE), STRICT LIABILITY OR OTHERWISE, EVEN IF APPLE HAS BEEN
				ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
				
	Change History (most recent first): <3>  7/07/02 era added current (QT 6.0) support for VBR, fixed leaky bits,
														 removed the bufferCmd / callBackCmd pair in favour of the
														 scheduledSoundCmd, added MPSemaphors to keep the play
														 buffers in sync and be X friendly
										<2> 10/31/00 era modified to use SoundConverterFillBuffer APIs
										<1>  7/26/00 mc initial release as PlaySound.c
*/

#include "SoundPlayer.h"





// globals

#if TARGET_OS_MAC

MPSemaphoreID gBufferDoneSignalID;

#endif

#if TARGET_OS_WIN32

HANDLE semaphore;

#endif


// * ----------------------------
// MySoundCallBackFunction
//
// used to signal when a buffer is done playing
static pascal void MySoundCallBackFunction(SndChannelPtr theChannel, SndCommand *theCmd)
{
#pragma unused(theChannel, theCmd)
	
#if TARGET_OS_MAC

	MPSignalSemaphore(gBufferDoneSignalID);

#endif

#if TARGET_OS_WIN32

	ReleaseSemaphore(semaphore, 1, 0);

#endif

}

// * ----------------------------
// SoundConverterFillBufferDataProc
//
// the callback routine that provides the source data for conversion - it provides data by setting
// outData to a pointer to a properly filled out ExtendedSoundComponentData structure
static pascal Boolean SoundConverterFillBufferDataProc(SoundComponentDataPtr *outData, void *inRefCon)
{
	SCFillBufferDataPtr pFillData = (SCFillBufferDataPtr)inRefCon;
	
	OSErr err;
							
	// if after getting the last chunk of data the total time is over the duration, we're done
	if (pFillData->getMediaAtThisTime >= pFillData->sourceDuration) {
		pFillData->isThereMoreSource = false;
		pFillData->compData.desc.buffer = NULL;
		pFillData->compData.desc.sampleCount = 0;
		pFillData->compData.bufferSize = 0;
		pFillData->compData.commonFrameSize = 0;
	}
	
	if (pFillData->isThereMoreSource) {
	
		long	  sourceBytesReturned;
		long	  numberOfSamples;
		TimeValue sourceReturnedTime, durationPerSample;
		
		// in calling GetMediaSample, we'll get a buffer that consists of equal sized frames - the
		// degenerate case is only 1 frame -- for non-self-framed vbr formats (like AAC in QT 6.0)
		// we need to provide some more framing information - either the frameCount, frameSizeArray pair or
		// commonFrameSize field must be valid -- because we always get equal sized frames, we use
		// commonFrameSize and set the kExtendedSoundCommonFrameSizeValid flag -- if there is
		// only 1 frame then (common frame size == media sample size), if there are multiple frames,
		// then (common frame size == media sample size / number of frames).
		
		err = GetMediaSample(pFillData->sourceMedia,		// specifies the media for this operation
							 pFillData->hSource,			// function returns the sample data into this handle
							 pFillData->maxBufferSize,		// maximum number of bytes of sample data to be returned
							 &sourceBytesReturned,			// the number of bytes of sample data returned
							 pFillData->getMediaAtThisTime,	// starting time of the sample to be retrieved (must be in Media's TimeScale)
							 &sourceReturnedTime,			// indicates the actual time of the returned sample data
							 &durationPerSample,			// duration of each sample in the media
							 NULL,							// sample description corresponding to the returned sample data (NULL to ignore)
							 NULL,							// index value to the sample description that corresponds to the returned sample data (NULL to ignore)
							 0,								// maximum number of samples to be returned (0 to use a value that is appropriate for the media)
							 &numberOfSamples,				// number of samples it actually returned
							 NULL);							// flags that describe the sample (NULL to ignore)

		if ((noErr != err) || (sourceBytesReturned == 0)) {
			pFillData->isThereMoreSource = false;
			pFillData->compData.desc.buffer = NULL;
			pFillData->compData.desc.sampleCount = 0;		
			pFillData->compData.bufferSize = 0;		
			pFillData->compData.commonFrameSize = 0;
			
			if ((err != noErr) && (sourceBytesReturned > 0))
				DebugStr((ConstStr255Param)"\pGetMediaSample - Failed in FillBufferDataProc");
		}
		
		pFillData->getMediaAtThisTime = sourceReturnedTime + (durationPerSample * numberOfSamples);

		// we've specified kExtendedSoundSampleCountNotValid and the 'studly' Sound Converter will
		// take care of sampleCount for us, so while this is not required we fill out all the information
		// we have to simply demonstrate how this would be done
		// sampleCount is the number of PCM samples
		pFillData->compData.desc.sampleCount = numberOfSamples * durationPerSample;

		// kExtendedSoundBufferSizeValid was specified - make sure this field is filled in correctly
		pFillData->compData.bufferSize = sourceBytesReturned;

		// for VBR audio we specified the kExtendedSoundCommonFrameSizeValid flag - make sure this field is filled in correctly
		if (pFillData->isSourceVBR) pFillData->compData.commonFrameSize = sourceBytesReturned / numberOfSamples;
	}

	// set outData to a properly filled out ExtendedSoundComponentData struct
	*outData = (SoundComponentDataPtr)&pFillData->compData;
	
	return (pFillData->isThereMoreSource);
}

// * ----------------------------
// GetMovieMedia
//
// returns a Media identifier - if the file is a System 7 Sound a non-in-place import is done and
// a handle to the data is passed back to the caller who is responsible for disposing of it
static OSErr GetMovieMedia(const FSSpec *inFile, Movie *outMovie, Media *outMedia, Handle *outHandle)
{
	Movie theMovie = 0;
	Track theTrack;
	FInfo fndrInfo;

	OSErr err = noErr;

	err = FSpGetFInfo(inFile, &fndrInfo);
	BailErr(err);
	
	if (kQTFileTypeSystemSevenSound == fndrInfo.fdType) {
		// if this is an 'sfil' handle it appropriately
		// QuickTime can't import these files in place, but that's ok,
		// we just need a new place to put the data
		
		MovieImportComponent theImporter = 0;
		Handle 				 hDataRef = NULL;
		
		// create a new movie
		theMovie = NewMovie(newMovieActive);
		
		// allocate the data handle and create a data reference for this handle
		// the caller is responsible for disposing of the data handle once done with the sound
		*outHandle = NewHandle(0);
		err = PtrToHand(outHandle, &hDataRef, sizeof(Handle));
		if (noErr == err) {
			SetMovieDefaultDataRef(theMovie, hDataRef, HandleDataHandlerSubType);
			OpenADefaultComponent(MovieImportType, kQTFileTypeSystemSevenSound, &theImporter);
			if (theImporter) {
				Track		ignoreTrack;
				TimeValue 	ignoreDuration;		
				long 		ignoreFlags;
				
				err = MovieImportFile(theImporter, inFile, theMovie, 0, &ignoreTrack, 0, &ignoreDuration, 0, &ignoreFlags);
				CloseComponent(theImporter);
			}
		} else {
			if (*outHandle) {
				DisposeHandle(*outHandle);
				*outHandle = NULL;
			}
		}
		if (hDataRef) DisposeHandle(hDataRef);
		BailErr(err);
	} else {
		// import in place
		
		short theRefNum;
		short theResID = 0;	// we want the first movie
		Boolean wasChanged;
		
		// open the movie file
		err = OpenMovieFile(inFile, &theRefNum, fsRdPerm);
		BailErr(err);

		// instantiate the movie
		err = NewMovieFromFile(&theMovie, theRefNum, &theResID, NULL, newMovieActive, &wasChanged);
		CloseMovieFile(theRefNum);
		BailErr(err);
	}
		
	// get the first sound track
	theTrack = GetMovieIndTrackType(theMovie, 1, SoundMediaType, movieTrackMediaType);
	if (NULL == theTrack ) BailErr(invalidTrack);

	// get and return the sound track media
	*outMedia = GetTrackMedia(theTrack);
	if (NULL == *outMedia) err = invalidMedia;
	
	*outMovie = theMovie;
	
bail:
	return err;
}

// * ----------------------------
// MyGetSoundDescriptionExtension
//
// this function will extract the information needed to decompress the sound file, this includes 
// retrieving the sample description, the decompression atom and setting up the sound header
static OSErr MyGetSoundDescriptionExtension(Media inMedia, AudioFormatAtomPtr *outAudioAtom, CmpSoundHeaderPtr outSoundInfo)
{
	OSErr err = noErr;
			
	Size size;
	Handle extension;
	SoundDescriptionHandle hSoundDescription = (SoundDescriptionHandle)NewHandle(0);
	
	// get the description of the sample data
	GetMediaSampleDescription(inMedia, 1, (SampleDescriptionHandle)hSoundDescription);
	BailErr(GetMoviesError());

	extension = NewHandle(0);
	
	// get the "magic" decompression atom
	// This extension to the SoundDescription information stores data specific to a given audio decompressor.
	// Some audio decompression algorithms require a set of out-of-stream values to configure the decompressor
	// which are stored in a siDecompressionParams atom. The contents of the siDecompressionParams atom are dependent
	// on the audio decompressor.
	err = GetSoundDescriptionExtension(hSoundDescription, &extension, siDecompressionParams);
	if (noErr == err) {
		size = GetHandleSize(extension);
		HLock(extension);
		*outAudioAtom = (AudioFormatAtomPtr)NewPtr(size);
		err = MemError();
		// copy the atom data to our buffer...
		BlockMoveData(*extension, *outAudioAtom, size);
		HUnlock(extension);
	} else {
		// if it doesn't have an atom, that's ok
		err = noErr;
	}
	
	// set up our sound header
	outSoundInfo->format = (*hSoundDescription)->dataFormat;
	outSoundInfo->numChannels = (*hSoundDescription)->numChannels;
	outSoundInfo->sampleSize = (*hSoundDescription)->sampleSize;
	outSoundInfo->sampleRate = (*hSoundDescription)->sampleRate;
	outSoundInfo->compressionID = (*hSoundDescription)->compressionID;
	
bail:
	if (extension) DisposeHandle(extension);
	if (hSoundDescription) DisposeHandle((Handle)hSoundDescription);
	
	return err;
}

// * ----------------------------
// PlaySound
//
// this function does the actual work of playing the sound file, it sets up the sound converter environment, allocates play buffers,
// creates the sound channel and sends the appropriate sound commands to play the converted sound data
OSErr DoPlaySound(const FSSpec *inFileToPlay)
{	
	Movie					theMovie = 0;
	AudioCompressionAtomPtr	theDecompressionAtom = NULL;
	CmpSoundHeader			theCmpSoundInfo;
	SoundComponentData		theInputFormat,
							theOutputFormat;
	SoundConverter			mySoundConverter = NULL;
	ScheduledSoundHeader	mySndHeader0,
							mySndHeader1;
	SCFillBufferData 		scFillBufferData = {NULL};
	Media					theSoundMedia = NULL;

	Ptr						pDecomBuffer0 = NULL,
							pDecomBuffer1 = NULL;
							
	Handle					hSys7SoundData = NULL;
						
	Boolean					isSoundDone = false;
	
	OSErr 					err = noErr;
	
	err = GetMovieMedia(inFileToPlay, &theMovie, &theSoundMedia, &hSys7SoundData);
	BailErr(err);
	
	err = MyGetSoundDescriptionExtension(theSoundMedia, (AudioFormatAtomPtr *)&theDecompressionAtom, &theCmpSoundInfo);
	if (noErr == err) {		
		// setup input/output format for sound converter
		theInputFormat.flags = 0;
		theInputFormat.format = theCmpSoundInfo.format;
		theInputFormat.numChannels = theCmpSoundInfo.numChannels;
		theInputFormat.sampleSize = theCmpSoundInfo.sampleSize;
		theInputFormat.sampleRate = theCmpSoundInfo. sampleRate;
		theInputFormat.sampleCount = 0;
		theInputFormat.buffer = NULL;
		theInputFormat.reserved = 0;

		theOutputFormat.flags = 0;
		theOutputFormat.format = kSoundNotCompressed;
		theOutputFormat.numChannels = theInputFormat.numChannels;
		theOutputFormat.sampleSize = theInputFormat.sampleSize;
		theOutputFormat.sampleRate = theInputFormat.sampleRate;
		theOutputFormat.sampleCount = 0;
		theOutputFormat.buffer = NULL;
		theOutputFormat.reserved = 0;
		
		// variableCompression means we're going to use the commonFrameSize field and the kExtendedSoundCommonFrameSizeValid flag
		scFillBufferData.isSourceVBR = (theCmpSoundInfo.compressionID == variableCompression);

		err = SoundConverterOpen(&theInputFormat, &theOutputFormat, &mySoundConverter);
		BailErr(err);
		
		// this isn't crucial or even required for decompression only, but it does tell
		// the sound converter that we're cool with VBR audio
		SoundConverterSetInfo(mySoundConverter, siClientAcceptsVBR, (Ptr)true);

		// set up the sound converters decompresson 'environment' by passing in the 'magic' decompression atom
		err = SoundConverterSetInfo(mySoundConverter, siDecompressionParams, theDecompressionAtom);
		if (theDecompressionAtom) DisposePtr((Ptr)theDecompressionAtom);
		if (siUnknownInfoType == err) {
			// clear this error, the decompressor didn't
			// need the decompression atom and that's OK
			err = noErr;
		} else BailErr(err);
		
		// the input buffer has to be large enough so GetMediaSample isn't going to fail, your mileage may vary
		UInt32 inputBytes = ((1000 + (theInputFormat.sampleRate >> 16)) * theInputFormat.numChannels) * 4,
			   outputBytes = kMaxOutputBuffer; // kMaxOutputBuffer is 64k which should be ok, make it larger if you like
			   
		// allocate play buffers
		pDecomBuffer0 = NewPtr(outputBytes);
		BailErr(MemError());
		
		pDecomBuffer1 = NewPtr(outputBytes);
		BailErr(MemError());

		err = SoundConverterBeginConversion(mySoundConverter);
		BailErr(err);

		// setup first header
		mySndHeader0.u.cmpHeader.samplePtr = pDecomBuffer0;
		mySndHeader0.u.cmpHeader.numChannels = theOutputFormat.numChannels;
		mySndHeader0.u.cmpHeader.sampleRate = theOutputFormat.sampleRate;
		mySndHeader0.u.cmpHeader.loopStart = 0;
		mySndHeader0.u.cmpHeader.loopEnd = 0;
		mySndHeader0.u.cmpHeader.encode = cmpSH;					// compressed sound header encode value
		mySndHeader0.u.cmpHeader.baseFrequency = kMiddleC;
		mySndHeader0.u.cmpHeader.numFrames = 0;						// numFrames will equal outputFrames as we're converting
		// mySndHeader0.u.cmpHeader.AIFFSampleRate;					// this is not used
		mySndHeader0.u.cmpHeader.markerChunk = NULL;
		mySndHeader0.u.cmpHeader.format = theOutputFormat.format;
		mySndHeader0.u.cmpHeader.futureUse2 = 0;
		mySndHeader0.u.cmpHeader.stateVars = NULL;
		mySndHeader0.u.cmpHeader.leftOverSamples = NULL;
		mySndHeader0.u.cmpHeader.compressionID = fixedCompression;	// compression ID for fixed-sized compression, even uncompressed sounds use fixedCompression
		mySndHeader0.u.cmpHeader.packetSize = 0;					// the Sound Manager will figure this out for us
		mySndHeader0.u.cmpHeader.snthID = 0;
		mySndHeader0.u.cmpHeader.sampleSize = theOutputFormat.sampleSize;
		mySndHeader0.u.cmpHeader.sampleArea[0] = 0;					// no samples here because we use samplePtr to point to our buffer instead			
		mySndHeader0.flags = kScheduledSoundDoCallBack;
		mySndHeader0.callBackParam1 = 0;							// we don't pass anything to the callback routine
		mySndHeader0.callBackParam2 = 0;

		// setup second header, only the buffer ptr is different
		BlockMoveData(&mySndHeader0, &mySndHeader1, sizeof(mySndHeader0));
		mySndHeader1.u.cmpHeader.samplePtr = pDecomBuffer1;
		
		// fill in struct that gets passed to SoundConverterFillBufferDataProc via the refcon
		// this includes the ExtendedSoundComponentData record		
		scFillBufferData.sourceMedia = theSoundMedia;
		scFillBufferData.getMediaAtThisTime = 0;		
		scFillBufferData.sourceDuration = GetMediaDuration(theSoundMedia);
		scFillBufferData.isThereMoreSource = true;
		scFillBufferData.maxBufferSize = inputBytes;
		scFillBufferData.hSource = NewHandle((long)scFillBufferData.maxBufferSize);	// allocate source media buffer
		BailErr(MemError());
		MoveHHi((Handle)scFillBufferData.hSource);
		HLock((Handle)scFillBufferData.hSource);
		
		scFillBufferData.compData.desc = theInputFormat;
		scFillBufferData.compData.desc.buffer = (BytePtr)*scFillBufferData.hSource;
		scFillBufferData.compData.desc.flags = kExtendedSoundData;
		scFillBufferData.compData.recordSize = sizeof(ExtendedSoundComponentData);
		scFillBufferData.compData.extendedFlags = kExtendedSoundSampleCountNotValid | kExtendedSoundBufferSizeValid;
		if (scFillBufferData.isSourceVBR) scFillBufferData.compData.extendedFlags |= kExtendedSoundCommonFrameSizeValid;
		scFillBufferData.compData.bufferSize = 0;	// filled in during FillBuffer callback
		
		// create a semaphore used to sync the filling of the play buffers, setup the callback,
		// create the sound channel and play the sound -- we will continue to convert the sound data
		// into a free (non playing) buffer --  because we want to fill up both buffers immediately
		// without waiting for the first buffer to complete, a binary semaphore is created with the maximumValue
		// and initialValue set to 1, when we call MPWaitOnSemaphore if the value of the semaphore is greater
		// than zero, the value is decremented and the function returns immediately which is what we want the
		// very first time through - after that, MPWaitOnSemaphore will block waiting for our callback
		// to fire where we signal the semaphore, this indicates a free buffer and the process can continue
		#if TARGET_OS_MAC

			MPCreateBinarySemaphore(&gBufferDoneSignalID);

		#endif



		#if TARGET_OS_WIN32

			semaphore = CreateSemaphore(0,1,1,0);

		#endif

		SndCallBackUPP theSoundCallBackUPP = NewSndCallBackUPP(MySoundCallBackFunction);
		SndChannelPtr pSoundChannel = NULL;
		 		
		err = SndNewChannel(&pSoundChannel, sampledSynth, 0, theSoundCallBackUPP);

		if (err == noErr) {			
			SndCommand		  thePlayCmd0,
							  thePlayCmd1;
			SndCommand	 	  *pPlayCmd;
			Ptr 			  pDecomBuffer = NULL;
			CmpSoundHeaderPtr pSndHeader = NULL;
			
			UInt32 			  outputFrames,
							  actualOutputBytes,
							  outputFlags;
			eBufferNumber     whichBuffer = kFirstBuffer;
			
			SoundConverterFillBufferDataUPP theFillBufferDataUPP = NewSoundConverterFillBufferDataUPP(SoundConverterFillBufferDataProc);
			
			thePlayCmd0.cmd = scheduledSoundCmd;
			thePlayCmd0.param1 = 0;						// not used, but clear it out anyway just to be safe
			thePlayCmd0.param2 = (long)&mySndHeader0;

			thePlayCmd1.cmd = scheduledSoundCmd;
			thePlayCmd1.param1 = 0;						// not used, but clear it out anyway just to be safe
			thePlayCmd1.param2 = (long)&mySndHeader1;
			
			if (noErr == err) {
				while (!isSoundDone && !Button()) {							
					if (kFirstBuffer == whichBuffer) {
						pPlayCmd = &thePlayCmd0;
						pDecomBuffer = pDecomBuffer0;
						pSndHeader = &mySndHeader0.u.cmpHeader;
						whichBuffer = kSecondBuffer;
					} else {
						pPlayCmd = &thePlayCmd1;
						pDecomBuffer = pDecomBuffer1;
						pSndHeader = &mySndHeader1.u.cmpHeader;
						whichBuffer = kFirstBuffer;
					}

					err = SoundConverterFillBuffer(mySoundConverter,		// a sound converter
												   theFillBufferDataUPP,	// the callback UPP
												   &scFillBufferData,		// refCon passed to FillDataProc
												   pDecomBuffer,			// the decompressed data 'play' buffer
												   outputBytes,				// size of the 'play' buffer
												   &actualOutputBytes,		// number of output bytes
												   &outputFrames,			// number of output frames
												   &outputFlags);			// fillbuffer retured advisor flags
					if (err) break;
					if((outputFlags & kSoundConverterHasLeftOverData) == false) {
						isSoundDone = true;
					}
					
					pSndHeader->numFrames = outputFrames;
					
					err = SndDoCommand(pSoundChannel, pPlayCmd, true);				// play the next buffer

					#if TARGET_OS_MAC

						MPWaitOnSemaphore(gBufferDoneSignalID, kDurationForever);	// block waiting for a free buffer

					#endif					



					#if TARGET_OS_WIN32

						WaitForSingleObject(semaphore,INFINITE);

					#endif

					
				} // while
			}
			
			SoundConverterEndConversion(mySoundConverter, pDecomBuffer, &outputFrames, &outputBytes);

			if (noErr == err && outputFrames) {
				pSndHeader->numFrames = outputFrames;
				SndDoCommand(pSoundChannel, pPlayCmd, true);	// play the last buffer.
			}
			
			if (theFillBufferDataUPP)
				DisposeSoundConverterFillBufferDataUPP(theFillBufferDataUPP);
		}
		
		if (pSoundChannel)
			err = SndDisposeChannel(pSoundChannel, false);		// wait until sounds stops playing before disposing of channel
		
		if (theSoundCallBackUPP)
			DisposeSndCallBackUPP(theSoundCallBackUPP);
		
		#if TARGET_OS_MAC		

			MPDeleteSemaphore(gBufferDoneSignalID);

		#endif



		#if TARGET_OS_WIN32

			CloseHandle(semaphore);

		#endif

	}
					
bail:
	if (scFillBufferData.hSource)
		DisposeHandle(scFillBufferData.hSource);
		
	if (mySoundConverter)
		SoundConverterClose(mySoundConverter);
		
	if (pDecomBuffer0)
		DisposePtr(pDecomBuffer0);
		
	if (pDecomBuffer1)
		DisposePtr(pDecomBuffer1);
		
	if (hSys7SoundData)
		DisposeHandle(hSys7SoundData);
	
	if (theMovie)
		DisposeMovie(theMovie);

	return err;
}