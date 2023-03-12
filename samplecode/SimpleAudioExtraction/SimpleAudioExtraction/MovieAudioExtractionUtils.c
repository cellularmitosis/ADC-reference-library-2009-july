/*
	File:		MovieAudioExtractionUtils
	
	Description: Utilities for working with Movie Audio Extraction APIs

	Author:		Apple DTS

	Copyright: 	© Copyright 2005 Apple Computer, Inc. All rights reserved.
	
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
				
	Change History (most recent first):

*/

#include "MovieAudioExtractionUtils.h"

//
// Set the current time for the audio extraction session 
// 
// Use the MovieAudioExtractionSetProperty function and the
// kQTMovieAudioExtractionMoviePropertyID_CurrentTime property
//

OSStatus SetMovieAudioExtractionCurrentTime(MovieAudioExtractionRef inSessionRef,
                                            TimeRecord *inTimeRecord)
{
	// Set the current time for the audio extraction session 
	return(MovieAudioExtractionSetProperty( inSessionRef,
                                            kQTPropertyClass_MovieAudioExtraction_Movie,
                                            kQTMovieAudioExtractionMoviePropertyID_CurrentTime,
                                            sizeof(TimeRecord),
                                            (ConstQTPropertyValuePtr)inTimeRecord));
}

// 
// Retrieve the audio stream basic description (ASBD) for the audio extraction session
//
// Use MovieAudioExtractionGetPropertyInfo with the
// kQTMovieAudioExtractionAudioPropertyID_AudioStreamBasicDescription property.
// 

OSStatus GetMovieAudioExtractionASBD(MovieAudioExtractionRef inSessionRef,
                                        QTPropertyValuePtr *outASBDPtr)
{
	QTPropertyValueType outPropType;
	ByteCount			outPropValueSize;
	UInt32              outPropertyFlags;
	OSStatus			status = noErr;
	
	*outASBDPtr = nil;
	
	// first get the size of the property so we know how
	// much memory to allocate for our property buffer
	status = MovieAudioExtractionGetPropertyInfo(	inSessionRef,
                                                    kQTPropertyClass_MovieAudioExtraction_Audio,
                                                    kQTMovieAudioExtractionAudioPropertyID_AudioStreamBasicDescription,
                                                    &outPropType,
                                                    &outPropValueSize,
                                                    &outPropertyFlags);
	assert(noErr == status);
	
	// allocate a buffer for the property using the size value just obtained
	*outASBDPtr = malloc(outPropValueSize);
	assert(*outASBDPtr != nil);

	// now get the actual property and put it into our buffer
	ByteCount outPropValueSizeUsed;
	status = MovieAudioExtractionGetProperty( inSessionRef,
                                              kQTPropertyClass_MovieAudioExtraction_Audio,
                                              kQTMovieAudioExtractionAudioPropertyID_AudioStreamBasicDescription,
                                              outPropValueSize,
                                              *outASBDPtr,
                                              &outPropValueSizeUsed);

	return status;
}


// Get the default extraction layout for this movie, expanded into individual channel descriptions.
// The channel layout returned by this routine must be deallocated by the client.
// If 'asbd' is non-NULL, fill it with the default extraction asbd, which contains the
// highest sample rate among the sound tracks that will be contributing.
//	'outLayoutSize' and 'asbd' may be nil.
OSStatus GetDefaultExtractionLayout(Movie movie, UInt32* outLayoutSize,
                            AudioChannelLayout** outLayout, AudioStreamBasicDescription *asbd)
{
	OSStatus                    err = noErr;
	AudioChannelLayout          *layout = NULL;
	UInt32                      size = 0;
	MovieAudioExtractionRef     extractionSessionRef = nil;

	if (!outLayout)
	{
		err = paramErr;
		goto bail;
	}

	// Initiate a dummy audio extraction here, in order to get the resulting default channel layout.
	err = MovieAudioExtractionBegin(movie, 0, &extractionSessionRef);
	require(err == noErr, bail);	
	
	// Get the size of the extraction output layout
	err = MovieAudioExtractionGetPropertyInfo(extractionSessionRef, kQTPropertyClass_MovieAudioExtraction_Audio,
                                                kQTMovieAudioExtractionAudioPropertyID_AudioChannelLayout,
                                                NULL, &size, NULL);
	require(err == noErr, bail);	

	// Allocate memory for the layout
	layout = (AudioChannelLayout *) calloc(1, size);
	if (layout == nil) 
	{
		err = memFullErr;
		goto bail;
	}

	// Get the layout for the current extraction configuration.
	// This will have already been expanded into channel descriptions.
	err = MovieAudioExtractionGetProperty(extractionSessionRef, kQTPropertyClass_MovieAudioExtraction_Audio,
										  kQTMovieAudioExtractionAudioPropertyID_AudioChannelLayout,
										  size, layout, nil);
	require(err == noErr, bail);	
	
	// Return the layout and size, if requested
	*outLayout = layout;
	if (outLayoutSize)	
		*outLayoutSize = size;

	// If the ASBD was requested, get that also.
	if (asbd != nil)
	{
		// Get the layout for the current extraction configuration.
		// This will have already been expanded into channel descriptions.
		size = sizeof (*asbd);
		err = MovieAudioExtractionGetProperty(extractionSessionRef,
											  kQTPropertyClass_MovieAudioExtraction_Audio,
											  kQTMovieAudioExtractionAudioPropertyID_AudioStreamBasicDescription,
											  size, asbd, nil);
        require(err == noErr, bail);	
	}

bail:
	if (err != noErr)
	{
		if (layout)
			free(layout);
		if (outLayoutSize)	
			*outLayoutSize = 0;	
	}

	// Throw away the temporary audio extraction session
	if (extractionSessionRef)
	{
		MovieAudioExtractionEnd(extractionSessionRef);
	}
	
	return err;				
}


// Prepare the specified movie for extraction:
//	Open an extraction session.
//	Set the "All Channels Discrete" property if required.
//	Set the ASBD and output layout, if specified.
//      Set the extraction start time. 
// Return the audioExtractionSessionRef.
OSStatus PrepareMovieForExtraction( Movie movie, 
                                    MovieAudioExtractionRef* extractionRefPtr, 
                                    Boolean discrete, 
                                    AudioStreamBasicDescription asbd, 
                                    AudioChannelLayout** layout, 
                                    UInt32* layoutSizePtr, 
                                    TimeRecord startTime)
{
	OSStatus	err = noErr;
	
	if (extractionRefPtr == nil)
	{
		err = paramErr;
		goto bail;
	}
	
	// Movie extraction begin: Open an extraction session
	err = MovieAudioExtractionBegin(movie, 0, extractionRefPtr);
	require(err == noErr, bail);	
	
	// If we need to extract all discrete channels, set that property
	if (discrete)
	{
            err = MovieAudioExtractionSetProperty(*extractionRefPtr,
                                                    kQTPropertyClass_MovieAudioExtraction_Movie,
                                                    kQTMovieAudioExtractionMoviePropertyID_AllChannelsDiscrete,
                                                    sizeof (discrete), 
                                                    &discrete);
            require(err == noErr, bail);	
	}
	// Set the extraction ASBD
	err = MovieAudioExtractionSetProperty(*extractionRefPtr,
                                            kQTPropertyClass_MovieAudioExtraction_Audio,
                                            kQTMovieAudioExtractionAudioPropertyID_AudioStreamBasicDescription,
                                            sizeof (asbd), &asbd);
	require(err == noErr, bail);	

	// Set the output layout, if supplied
	if (*layout)
	{
		err = MovieAudioExtractionSetProperty(*extractionRefPtr,
                                                kQTPropertyClass_MovieAudioExtraction_Audio,
                                                kQTMovieAudioExtractionAudioPropertyID_AudioChannelLayout,
                                                *layoutSizePtr, *layout);
                require(err == noErr, bail);	
	}

	// Set the extraction start time.  The duration will be determined by how much is pulled.
	err = MovieAudioExtractionSetProperty(*extractionRefPtr,
                                            kQTPropertyClass_MovieAudioExtraction_Movie,
                                            kQTMovieAudioExtractionMoviePropertyID_CurrentTime,
                                            sizeof(TimeRecord), &startTime);

bail:
	// If error, close the extraction session
	if (err != noErr)
	{
		if (*extractionRefPtr != nil)
			MovieAudioExtractionEnd(*extractionRefPtr);
	}	
	return err;
}
