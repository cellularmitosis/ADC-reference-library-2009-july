/*
	File:		ConvertMovieSndTrack.c
	
	Description: This sample demonstrates how to use the Sound Converter to transcode a Movies Sound Track
				 from one audio encoding format to another. It uses StdCompression to configure the audio
				 compressor and creates a QuickTime Movie containing the newly encoded Sound Track.
				 This sample also demonstrates the preferred way to use the Sound Converter - using
				 SoundConverterFillBuffer - and shows what you need to do to support the new VBR compression
				 formats - in QuickTime 6, an example is the AAC format, part of MPEG4 audio support.
				 This sample will also work with any audio file format QuickTime can import as a Movie
				 such as AIFF, WAVE and MP3.
				 
				 NOTE: This Sample Requires QuickTime 6.0 or greater and also requires Mac OS X. There is
				 	   no CFM target provided in this sample.
				 
				 Parts of this code were based on testing code written by baron, and other parts first
				 appeared in MP3Player and now appear in it's updated version SoundPlayer.
				 
	Author:		era
	
	Version 	1.0

	Copyright: 	© Copyright 2002 Apple Computer, Inc. All rights reserved.
	
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
				
	Change History (most recent first): <1>  7/28/02 era initial release as ConvertMovieSndTrack.c
*/

#include "ConvertMovieSndTrack.h"

#ifndef AVAILABLE_MAC_OS_X_VERSION_10_2_AND_LATER
	// these didn't make it into the QT6 framework for 10.1.x so include
	// them here if we're not on 10.2 or later - if you have a newer framework
	// or are building a carbon CFM version you shouldn't need these
	enum {
	  scSoundVBRCompressionOK       = 'cvbr', /* pointer to Boolean*/
	  scSoundInputSampleRateType    = 'ssir', /* pointer to UnsignedFixed*/
	  scSoundSampleRateChangeOK     = 'rcok', /* pointer to Boolean*/
	  scAvailableCompressionListType = 'avai' /* pointer to OSType Handle*/
	};
#endif

// globals
QTAtomContainer gSoundSettings = NULL;

// * ----------------------------
// SoundConverterFillBufferDataProc
//
// the callback routine that provides the SOURCE DATA for conversion - it provides data by setting
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
		// degenerate case is only 1 frame -- for non-self-framed vbr formats (like AAC in QT 6)
		// we need to provide some more framing information - either the frameCount, frameSizeArray pair or
		// the commonFrameSize field must be valid -- because we always get equal sized frames, we can use
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
				DebugStr("\pGetMediaSample - Failed in FillBufferDataProc");
		}

		pFillData->getMediaAtThisTime = sourceReturnedTime + (durationPerSample * numberOfSamples);
		
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
// GetSoundDescriptionInfo
//
// this function will extract the information needed to decompress the sound file, this includes 
// retrieving the sample description and the decompression atom saved as a Sample Description Extention
static OSErr GetSoundDescriptionInfo(Media inMedia, Ptr *outAudioAtom, SoundDescriptionPtr outSoundDesc)
{
	OSErr err = noErr;
			
	Size size;
	Handle extension = NULL;
	SoundDescriptionHandle hSoundDescription = (SoundDescriptionHandle)NewHandle(0);
	
	// get the description of the sample data
	GetMediaSampleDescription(inMedia, 1, (SampleDescriptionHandle)hSoundDescription);
	BailErr(GetMoviesError());

	extension = NewHandle(0);
	BailErr(MemError());
	
	// get the "magic" decompression atom
	// This extension to the SoundDescription information stores data specific to a given audio decompressor.
	// Some audio decompression algorithms require a set of out-of-stream values to configure the decompressor
	// which are stored in a siDecompressionParams atom. The contents of the siDecompressionParams atom are dependent
	// on the sound decompressor.
	err = GetSoundDescriptionExtension(hSoundDescription, &extension, siDecompressionParams);
	if (noErr == err) {
		size = GetHandleSize(extension);
		HLock(extension);
		*outAudioAtom = NewPtr(size);
		BailErr(MemError());
		// copy the atom data to our Ptr...
		BlockMoveData(*extension, *outAudioAtom, size);
		HUnlock(extension);
	} else {
		// if it doesn't have an atom, that's ok
		err = noErr;
	}
	
	// set up our sound header
	outSoundDesc->dataFormat = (*hSoundDescription)->dataFormat;
	outSoundDesc->numChannels = (*hSoundDescription)->numChannels;
	outSoundDesc->sampleSize = (*hSoundDescription)->sampleSize;
	outSoundDesc->sampleRate = (*hSoundDescription)->sampleRate;
	outSoundDesc->compressionID = (*hSoundDescription)->compressionID;
	
bail:
	if (extension) DisposeHandle(extension);
	if (hSoundDescription) DisposeHandle((Handle)hSoundDescription);
	
	return err;
}

// * ----------------------------
// ConvertMovieSndTrackToNewMovieSndTrack
//
// this function does the actual work
OSErr ConvertMovieSndTrackToNewMovieSndTrack(const FSSpecPtr inFileToConvert, const FSSpec inNewFile)
{
	SoundConverter			 mySoundConverter = NULL;
	
	Movie					 theSrcMovie = 0,
							 theDstMovie = 0;
	Track					 theDstTrack = 0;
	Media					 theSrcMedia = 0,
							 theDstMedia = 0;
							
	short					 theDstMovieRefNum = -1;
	Handle					 hSys7SoundData = NULL;
	
	Ptr						 theDecompressionParams = NULL;
	Handle 					 theCompressionParams = NULL;

	SoundDescription		 theSrcInputFormatInfo;
	SoundDescriptionV1Handle hSoundDescription = NULL;
	UnsignedFixed 			 theOutputSampleRate;
	SoundComponentData		 theInputFormat,
							 theOutputFormat;

	SCFillBufferData 		 scFillBufferData = { NULL };
	Ptr						 pDecomBuffer = NULL;
										
	Boolean					 isSoundDone = false;
	
	OSErr 					 err = noErr;
	
	// *********** SOURCE: Get sound data info from the first source movie sound track
	
	err = GetMovieMedia(inFileToConvert, &theSrcMovie, &theSrcMedia, &hSys7SoundData);
	BailErr(err);
	
	err = GetSoundDescriptionInfo(theSrcMedia, (Ptr *)&theDecompressionParams, &theSrcInputFormatInfo);
	if (noErr == err) {		
		// setup input format for sound converter
		theInputFormat.flags = 0;
		theInputFormat.format = theSrcInputFormatInfo.dataFormat;
		theInputFormat.numChannels = theSrcInputFormatInfo.numChannels;
		theInputFormat.sampleSize = theSrcInputFormatInfo.sampleSize;
		theInputFormat.sampleRate = theSrcInputFormatInfo. sampleRate;
		theInputFormat.sampleCount = 0;
		theInputFormat.buffer = NULL;
		theInputFormat.reserved = 0;

	// *********** DESTINATION: Set up the destination format
	
		// Standard Sound Compression Dialog will configure output settings goodness for us

		ComponentInstance ci = OpenDefaultComponent(StandardCompressionType, StandardCompressionSubTypeSound);
		Boolean trueBoolean = true;
			
		// both of the following selectors must be set true for VBR audio compressors like MPEG-4 to shows up in the codec list
		// inform StdCompression we know how to deal with VBR
		SCSetInfo(ci, scSoundVBRCompressionOK, &trueBoolean);
		
		// inform StdCompression that we know the output sample rate might be changed by the audio codec
		// MPEG4 for example may change the output sample rate if the user selects a low bit rate
		SCSetInfo(ci, scSoundSampleRateChangeOK, &trueBoolean);
		
		// restore the previous settings if any
		// first, try to grab them from our users preferences - if they're not there that's fine
		GetSoundSettingsPreference();
		if (gSoundSettings) {
			SCSetSettingsFromAtomContainer(ci, gSoundSettings);
			// Dispose the settings we don't need them anymore
			DisposeHandle(gSoundSettings);
			gSoundSettings = NULL;
		}
		
		// request the settings ie. bring up the dialog
		err = SCRequestImageSettings(ci);
		if (userCanceledErr == err) { err = noErr; CloseComponent(ci); goto bail; }
		
		// save the settings for next time
		SCGetSettingsAsAtomContainer(ci, &gSoundSettings);
		SaveSoundSettingsPreference();

		// fill in our output format for the sound converter
		SCGetInfo(ci, scSoundCompressionType, &theOutputFormat.format);
		// If StdCompression returns 'raw ' (k8BitOffsetBinaryFormat), it really means "uncompressed".  You should
		// change 'raw ' to 'NONE', or SoundConverterOpen will return -2003 (cantFindHandler).
		if (theOutputFormat.format == k8BitOffsetBinaryFormat)
			theOutputFormat.format = kSoundNotCompressed;
	 	SCGetInfo(ci, scSoundChannelCountType, &theOutputFormat.numChannels);
	 	SCGetInfo(ci, scSoundSampleSizeType, &theOutputFormat.sampleSize);
	 	// InputSampleRate can be different than the SampleRate so make sure to set up the output format
	 	// sample rate correctly for the sound converter - see "What's new in QuickTime 6", Table 1 page 38 -
	 	// few codecs (but some) implement the InputSampleRate selector so make sure to check for an invalid
	 	// return value and if so get the output sample rate
	 	SCGetInfo(ci, scSoundInputSampleRateType, &theOutputFormat.sampleRate);
		if (theOutputFormat.sampleRate == 0)
	 		SCGetInfo(ci, scSoundSampleRateType, &theOutputFormat.sampleRate);

	 	// we need to get the appropriate codec settings for the compressor
	 	// these 'magic' settings need to be added to the Sound Description
		SCGetInfo(ci, scCodecSettingsType, &theCompressionParams);
		
		theOutputFormat.flags = kNoRealtimeProcessing;   // not anxious about how long it takes to encode
		theOutputFormat.sampleCount = 0;
		theOutputFormat.buffer = NULL;
		theOutputFormat.reserved = 0;
		
		// always remember to close any opened components when you're done with them
		CloseComponent(ci);

	// *********** SOUND CONVERTER: Open converter and prepare for buffer conversion...captain!

		err = SoundConverterOpen(&theInputFormat, &theOutputFormat, &mySoundConverter);
		BailErr(err);
		
		// tell the sound converter we're cool with VBR formats
		SoundConverterSetInfo(mySoundConverter, siClientAcceptsVBR, Ptr(true));															

		// set up the sound converters compression environment
		// pass down siCompressionSampleRate, siCompressionChannels then siCompressionParams
		SoundConverterSetInfo(mySoundConverter, siCompressionSampleRate, &theOutputFormat.sampleRate); // ignore errors
		SoundConverterSetInfo(mySoundConverter, siCompressionChannels, &theOutputFormat.numChannels);

		// set up the compression environment by passing in the 'magic' compression params aquired from
		// standard sound compression eariler
		if (theCompressionParams) {
			HLock(theCompressionParams);
			err = SoundConverterSetInfo(mySoundConverter, siCompressionParams, *theCompressionParams);
			BailErr(err);
			HUnlock(theCompressionParams);
		}

		// set up the decompresson environment by passing in the 'magic' decompression params
		if (theDecompressionParams) {
			// don't check for an error, if the decompressor didn't need the
			// decompression atom for whatever reason we should still be ok
			SoundConverterSetInfo(mySoundConverter, siDecompressionParams, theDecompressionParams);
		}
		
		// we need to know if the output sample rate was changed so we can write it in the image description
		// few codecs (but some) will implement this - MPEG4 for example may change the output sample rate if
		// the user selects a low bit rate -  ignore errors
	 	theOutputSampleRate = theOutputFormat.sampleRate;
		SoundConverterGetInfo(mySoundConverter, siCompressionOutputSampleRate, &theOutputSampleRate);
		
		err = SoundConverterBeginConversion(mySoundConverter);
		BailErr(err);
		
	// *********** DESTINATION MOVIE: Create Movie, Add Sound Track, New Media, Setup Sound Description
	//                                and Add Sound Description Extenstion if present
	
		// create the movie file
		err = CreateMovieFile(&inNewFile, 'TVOD', smSystemScript,
							  createMovieFileDeleteCurFile | createMovieFileDontCreateResFile, &theDstMovieRefNum, &theDstMovie);
		BailErr(err);
		
		// create a new track
		theDstTrack = NewMovieTrack(theDstMovie, 0, 0, kFullVolume);
		BailErr(GetMoviesError());
		
		// it's a sound track so add a SoundMedia
		// this is the sample rate we got back from our attempt to use siCompressionOutputSampleRate above
		theDstMedia = NewTrackMedia(theDstTrack, SoundMediaType, (theOutputSampleRate >> 16), NULL, 0);
		BailErr(GetMoviesError());
		 
		// start media editing session
		err = BeginMediaEdits(theDstMedia);
		BailErr(err);
		
		// we need to get info about data/frame sizes 
		// good practice to fill in the size of this structure
		CompressionInfo compressionFactor = { sizeof(compressionFactor), 0 };
		
		hSoundDescription = (SoundDescriptionV1Handle)NewHandleClear(sizeof(SoundDescriptionV1));	
		BailErr(MemError());
		
		err = SoundConverterGetInfo(mySoundConverter, siCompressionFactor, &compressionFactor);				
		BailErr(err);

		HLock((Handle)hSoundDescription);
		
		(*hSoundDescription)->desc.descSize		 = sizeof(SoundDescriptionV1);
		(*hSoundDescription)->desc.dataFormat	 = (long)theOutputFormat.format;	   // compression format
		(*hSoundDescription)->desc.resvd1		 = 0;								   // must be 0
		(*hSoundDescription)->desc.resvd2		 = 0;							       // must be 0
		(*hSoundDescription)->desc.dataRefIndex	 = 0;								   // 0 - we'll let AddMediaXXX determine the index
		(*hSoundDescription)->desc.version		 = 1;								   // set to 1
		(*hSoundDescription)->desc.revlevel		 = 0;								   // set to 0
		(*hSoundDescription)->desc.vendor		 = 0;
		(*hSoundDescription)->desc.numChannels	 = theOutputFormat.numChannels;		   // number of channels
		(*hSoundDescription)->desc.sampleSize	 = theOutputFormat.sampleSize;		   // bits per sample - everything but 8 bit can be set to 16
		(*hSoundDescription)->desc.compressionID = compressionFactor.compressionID;    // the compression ID (eg. variableCompression)
		(*hSoundDescription)->desc.packetSize	 = 0;								   // set to 0
		(*hSoundDescription)->desc.sampleRate	 = theOutputSampleRate;		   		   // the sample rate
		// version 1 stuff
		(*hSoundDescription)->samplesPerPacket 	 = compressionFactor.samplesPerPacket; // the samples per packet holds the PCM sample count per audio frame (packet)
		(*hSoundDescription)->bytesPerPacket 	 = compressionFactor.bytesPerPacket;   // the bytes per packet
		
		// bytesPerFrame isn't necessarily calculated for us and returned as part of the CompressionFactor - not all codecs that
		// implement siCompressionFactor fill out bytesPerFrame - so we do it here - note that VBR doesn't deserve this treatment
		// but it's not harmful, the Sound Manager would do calculations itself as part of GetCompressionInfo()
		// It should be noted that GetCompressionInfo() doesn't work for codecs that need configuration with 'magic' settings.
		// This requires explicit opening of the codec and the siCompressionFactor selector for SoundComponentGetInfo()
		(*hSoundDescription)->bytesPerFrame 	 = compressionFactor.bytesPerPacket * theOutputFormat.numChannels;
		(*hSoundDescription)->bytesPerSample 	 = compressionFactor.bytesPerSample;							
       
        // the theCompressionParams are not necessarily present
		if (theCompressionParams) {
			// a Sound Description can't be locked when calling AddSoundDescriptionExtension so make sure it's unlocked
			HUnlock((Handle)hSoundDescription);
			err = AddSoundDescriptionExtension((SoundDescriptionHandle)hSoundDescription, theCompressionParams, siDecompressionParams);	
			BailErr(err);
			HLock((Handle)hSoundDescription);
		}
        
        // VBR implies a different media layout, this will affect how AddMediaSample() is called below
        Boolean outputFormatIsVBR = ((*hSoundDescription)->desc.compressionID == variableCompression);
        
	// *********** SOUND CONVERTER: Create buffers and Convert Data

		// figure out sizes for the input and output buffers
		// the input buffer has to be large enough so GetMediaSample isn't going to fail
		// start with some rough numbers which should work well
		UInt32 inputBytes = ((1000 + (theInputFormat.sampleRate >> 16)) * theInputFormat.numChannels) * 4,
			   outputBytes = 0,
			   maxPacketSize = 0;
		
		// ask about maximum packet size (or worst case packet size) so we don't allocate a destination (output)
		// buffer that's too small - an output buffer smaller than MaxPacketSize would be really bad - init maxPacketSize
		// to 0 so if the request isn't understood we can create a number (some multiple of maxPacketSize) and go from there
		// this is likely only implemented by VBR codecs so don't get anxious about it not being implemented
		SoundConverterGetInfo(mySoundConverter, siCompressionMaxPacketSize, &maxPacketSize);
		
		// start with this - you don't really need to use GetBufferSizes just as long as the output buffer is larger than
		// the MaxPacketSize if implemented - we use kMaxBufferSize which is 64k as a minimum
		SoundConverterGetBufferSizes(mySoundConverter, kMaxBufferSize, NULL, NULL, &outputBytes);
		
		if (0 == maxPacketSize)
			maxPacketSize = kMaxBufferSize;   // kMaxBufferSize is 64k
											 								  
		if (inputBytes < kMaxBufferSize)	  // kMaxBufferSize is 64k
			inputBytes = kMaxBufferSize;	  // note this is still too small for DV (NTSC=120000, PAL=144000)
			
		if (outputBytes < maxPacketSize)	  
			outputBytes = maxPacketSize;

		// allocate conversion buffer
		pDecomBuffer = NewPtr(outputBytes);
		BailErr(MemError());
		
		// fill in struct that gets passed to SoundConverterFillBufferDataProc via the refcon
		// this includes the ExtendedSoundComponentData information		
		scFillBufferData.sourceMedia = theSrcMedia;
		scFillBufferData.getMediaAtThisTime = 0;		
		scFillBufferData.sourceDuration = GetMediaDuration(theSrcMedia);
		scFillBufferData.isThereMoreSource = true;
		scFillBufferData.maxBufferSize = inputBytes;
		
		// if the source is VBR it means we're going to set the kExtendedSoundCommonFrameSizeValid
		// flag and use the commonFrameSize field in the FillBuffer callback
		scFillBufferData.isSourceVBR = (theSrcInputFormatInfo.compressionID == variableCompression);
		
		scFillBufferData.hSource = NewHandle((long)scFillBufferData.maxBufferSize);	// allocate source media buffer
		BailErr(MemError());
		HLockHi((Handle)scFillBufferData.hSource);
		
		scFillBufferData.compData.desc = theInputFormat;
		scFillBufferData.compData.desc.buffer = (BytePtr)*scFillBufferData.hSource;
		scFillBufferData.compData.desc.flags = kExtendedSoundData;
		scFillBufferData.compData.recordSize = sizeof(ExtendedSoundComponentData);
		scFillBufferData.compData.extendedFlags = kExtendedSoundBufferSizeValid;
		if (scFillBufferData.isSourceVBR) scFillBufferData.compData.extendedFlags |= kExtendedSoundCommonFrameSizeValid;
		scFillBufferData.compData.bufferSize = 0;	// filled in during FillBuffer callback

		if (err == noErr) {	
			
			UInt32 outputFrames,
				   actualOutputBytes,
				   outputFlags,
				   durationPerMediaSample,
				   numberOfMediaSamples;
				   			
			SoundConverterFillBufferDataUPP theFillBufferDataUPP = NewSoundConverterFillBufferDataUPP(SoundConverterFillBufferDataProc);	

			while (!isSoundDone) {

				err = SoundConverterFillBuffer(mySoundConverter,		// a sound converter
											   theFillBufferDataUPP,	// the callback UPP
											   &scFillBufferData,		// refCon passed to FillDataProc
											   pDecomBuffer,			// the destination data  buffer
											   outputBytes,				// size of the destination buffer
											   &actualOutputBytes,		// number of output bytes
											   &outputFrames,			// number of output frames
											   &outputFlags);			// FillBuffer retured advisor flags
				if (err) break;
				if((outputFlags & kSoundConverterHasLeftOverData) == false) {
					isSoundDone = true;
				}
				
				// see if output buffer is filled so we can write some data	
				if (actualOutputBytes > 0) {					
					// so, what are we going to pass to AddMediaSample?
					// 
					// for variableCompression, a media sample == an audio packet (compressed), this is also true for uncompressed audio
					// for fixedCompression, a media sample is a portion of an audio packet - it is 1 / compInfo.samplesPerPacket worth
					// of data, there's no way to access just a portion of the samples
					// therefore, we need to know if our compression format is VBR or Fixed and make the correct calculations for
					// either VBR or not - Fixed and uncompressed are treated the same
					if (outputFormatIsVBR) {
						numberOfMediaSamples = outputFrames;
						durationPerMediaSample = compressionFactor.samplesPerPacket;
					} else {		
						numberOfMediaSamples = outputFrames * compressionFactor.samplesPerPacket;
						durationPerMediaSample = 1;
					}

					err = AddMediaSample(theDstMedia,
										 &pDecomBuffer,
										 0,
										 actualOutputBytes,
										 durationPerMediaSample,
										 (SampleDescriptionHandle)hSoundDescription,
										 (long)numberOfMediaSamples,
										 0, NULL);
					if (err) break;
				}

			} // while
			
			SoundConverterEndConversion(mySoundConverter, pDecomBuffer, &outputFrames, &actualOutputBytes);

			// if there's any left over data write it out
			if (noErr == err && actualOutputBytes > 0) {
				// see above comments regarding these calculations
				if (outputFormatIsVBR) {
					numberOfMediaSamples = outputFrames;
					durationPerMediaSample = compressionFactor.samplesPerPacket;
				} else {		
					numberOfMediaSamples = outputFrames * compressionFactor.samplesPerPacket;
					durationPerMediaSample = 1;
				}

				err = AddMediaSample(theDstMedia,
								     &pDecomBuffer,
									 0,
									 actualOutputBytes,
									 durationPerMediaSample,
									 (SampleDescriptionHandle)hSoundDescription,
									 (long)numberOfMediaSamples,
									 0, NULL);
				BailErr(err);
			}
			
			if (theFillBufferDataUPP)
				DisposeSoundConverterFillBufferDataUPP(theFillBufferDataUPP);
		}
						
	// *********** DESTINATION MOVIE: All media has been added so clean up
		
		err = EndMediaEdits(theDstMedia);
		BailErr(err);
		
		err = InsertMediaIntoTrack(theDstTrack, 0, 0, GetMediaDuration(theDstMedia), fixed1);
		BailErr(err);
		
		AddUserDataTextToMovie(theDstMovie, "ConvertMovieSndTrack", kUserDataTextEncodedBy);
		if (theOutputFormat.format == FOUR_CHAR_CODE('mp4a'))
			AddUserDataTextToMovie(theDstMovie, "QuickTime 6.0 or greater", kUserDataTextSpecialPlaybackRequirements);

		err = AddMovieResource(theDstMovie, theDstMovieRefNum, NULL, NULL);
		BailErr(err);
	}
	
	DoAlert("\pConvert Movie Sound Track Done!", err);
					
bail:
	if (mySoundConverter)
		SoundConverterClose(mySoundConverter);
	
	if (theDstMovieRefNum != -1)
		CloseMovieFile(theDstMovieRefNum);
		
	if (scFillBufferData.hSource)
		DisposeHandle(scFillBufferData.hSource);
		
	if (pDecomBuffer)
		DisposePtr(pDecomBuffer);
		
	if (theCompressionParams)
		DisposeHandle(theCompressionParams);
		
	if (theDecompressionParams)
		DisposePtr((Ptr)theDecompressionParams);
		
	if (hSoundDescription)
		DisposeHandle((Handle)hSoundDescription);
		
	if (theSrcMovie)
		DisposeMovie(theSrcMovie);
		
	if (theDstMovie)
		DisposeMovie(theDstMovie);
		
	if (hSys7SoundData)
		DisposeHandle(hSys7SoundData);

	return err;
}

// * ----------------------------
// some utility functions we use

// add user data to a movie
OSErr AddUserDataTextToMovie(Movie inMovie, char inText[], OSType inType)
{
	UserData userData;
	Handle   h = NULL;
	long     length = strlen(inText);
	
	OSErr    err = noErr;
	
	userData = GetMovieUserData(inMovie);
	if (userData == NULL) return paramErr;
	
	err = PtrToHand(inText, &h, length);
	if (noErr == err) {

		err = AddUserDataText(userData, h, inType, 1, langEnglish);

		DisposeHandle(h);
	}
	
	return err;
}

// some very basic functions for getting and saving the StdCompression codec settings
// as a user preference, we use a global settings handle because it's easy
OSErr GetSoundSettingsPreference(void)
{
	CFStringRef       codecSoundSettingsKey = CFSTR("codecSoundSettings");
	CFPropertyListRef theSoundSettings;
	OSErr 			  err = noErr;
	
	// already have our settings so return
	if (gSoundSettings) return err;

	// Read the preference
	theSoundSettings = CFPreferencesCopyAppValue(codecSoundSettingsKey, kCFPreferencesCurrentApplication);
	if (theSoundSettings) {				
		err = PtrToHand(CFDataGetBytePtr((CFDataRef)theSoundSettings), &gSoundSettings, CFDataGetLength((CFDataRef)theSoundSettings));
		if (err) gSoundSettings = NULL;
		
		CFRelease(theSoundSettings);
	}
	
	return err;
}

void SaveSoundSettingsPreference(void)
{
	CFStringRef codecSoundSettingsKey = CFSTR("codecSoundSettings");
	CFDataRef theSoundSettings;
	
	HLock(gSoundSettings);
	
	theSoundSettings = CFDataCreate(NULL, (UInt8 *)*gSoundSettings, GetHandleSize(gSoundSettings));
	
	HUnlock(gSoundSettings);
	
	CFPreferencesSetAppValue(codecSoundSettingsKey, theSoundSettings, kCFPreferencesCurrentApplication);
	CFPreferencesAppSynchronize(kCFPreferencesCurrentApplication);
	
	CFRelease(theSoundSettings);
}