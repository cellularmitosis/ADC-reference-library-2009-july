/*

File: scaudiocompress.c

Abstract:   Demonstrates typical usage of the SCAudio compression API's, in
            particular, SCAudioFillBuffer.  Also demonstrates how to read
            from files using (1) MovieAudioExtraction, and (2) AudioFile API,
            and how to write audio QuickTime movies using AddMediaSample2 or
            .caf files using AudioFile API.

Version: 1.0

Disclaimer: IMPORTANT:  This Apple software is supplied to you by Apple
Computer, Inc. ("Apple") in consideration of your agreement to the
following terms, and your use, installation, modification or
redistribution of this Apple software constitutes acceptance of these
terms.  If you do not agree with these terms, please do not use,
install, modify or redistribute this Apple software.

In consideration of your agreement to abide by the following terms, and
subject to these terms, Apple grants you a personal, non-exclusive
license, under Apple's copyrights in this original Apple software (the
"Apple Software"), to use, reproduce, modify and redistribute the Apple
Software, with or without modifications, in source and/or binary forms;
provided that if you redistribute the Apple Software in its entirety and
without modifications, you must retain this notice and the following
text and disclaimers in all such redistributions of the Apple Software. 
Neither the name, trademarks, service marks or logos of Apple Computer,
Inc. may be used to endorse or promote products derived from the Apple
Software without specific prior written permission from Apple.  Except
as expressly stated in this notice, no other rights or licenses, express
or implied, are granted by Apple herein, including but not limited to
any patent rights that may be infringed by your derivative works or by
other works in which the Apple Software may be incorporated.

The Apple Software is provided by Apple on an "AS IS" basis.  APPLE
MAKES NO WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION
THE IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS
FOR A PARTICULAR PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND
OPERATION ALONE OR IN COMBINATION WITH YOUR PRODUCTS.

IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL
OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION,
MODIFICATION AND/OR DISTRIBUTION OF THE APPLE SOFTWARE, HOWEVER CAUSED
AND WHETHER UNDER THEORY OF CONTRACT, TORT (INCLUDING NEGLIGENCE),
STRICT LIABILITY OR OTHERWISE, EVEN IF APPLE HAS BEEN ADVISED OF THE
POSSIBILITY OF SUCH DAMAGE.

Copyright © 2006 Apple Computer, Inc., All Rights Reserved

*/ 

#include <TargetConditionals.h>

#if TARGET_OS_MAC
	#include <CoreServices/CoreServices.h>
	#include <QuickTime/QuickTime.h>
	#include <AudioToolbox/AudioToolbox.h>
	#include <ApplicationServices/ApplicationServices.h>
#else
	#include <stdlib.h>
	#include <stdio.h>
	#include <string.h>
	#include <QuickTimeComponents.h>
	#include <QTML.h>
#endif

#define kQTAudioPropertyID_MaxAudioSampleSize   'mssz'

#define DEFAULT_PULL_FRAMES         5000
#define LOG_FILE                    stderr
#define LOG_DATA_FLOW				0

#if TARGET_OS_MAC
	#if LOG_DATA_FLOW
		#define logflow(...)			fprintf(LOG_FILE, __VA_ARGS__)
	#else
		#define logflow(...)
	#endif
#else
        // Windows compiler unfortunately does not allow variadic macros
        
		int __cdecl logflow(const char *format, ...)
		{
			int		retval = 0;
		#if LOG_DATA_FLOW
			va_list args;
			char *	buffer = NULL;
			int		len;
			
			va_start( args, format );

			len = _vscprintf( format, args ) // _vscprintf doesn't count
										+ 1; // terminating '\0'
			buffer = malloc( len * sizeof(char) );
			retval = vsprintf( buffer, format, args );
			OutputDebugString(buffer);
			free( buffer );

			va_end(args);
		#endif
			return retval;
		}
#endif

typedef struct {
    char *							inFile;
    char *							outFile;
    Boolean							suppressUI;
    Boolean							forceQTRead;
	Boolean							extractionComplete;
	Boolean							writingMovie;
    Boolean                         writingRaw;
    UInt8                       pad[1];
	FILE *							rawFile;
	SInt32							onlyChannel;
    UInt32							pullPackets;
    AudioStreamBasicDescription     inasbd;
    UInt32                          inputBufferSize;
    void **                         inputBuffers;
    
    Movie							m;
    MovieAudioExtractionRef			mae;
 
#if TARGET_OS_MAC
    AudioFileID						af;
#endif
	UInt64							srcFilePacketCount;
	SInt64							srcPacketsRead;
	AudioStreamPacketDescription *	packetDescs;
	UInt32							numPacketDescs;
	UInt32							srcFileMaxPacketSize;
	void *							srcFileReadBuffer;
	UInt32							srcFileReadBufferSize;

#if TARGET_OS_MAC
    AudioFileID						outaf;
#endif
	SInt64							numPacketsWritten;
	
	Movie							outm;
	DataHandler						dataH;
	Media							media;
	SoundDescriptionHandle			sdh;
    
    ComponentInstance				stdAudio;
} SCAudioCompressDataRecord, *SCAudioCompressData;



// --------------------------------------------------------------------------------
/*
    mySCAudioInputDataProc is called for input in the specified source format.
    The source format is specified using the kQTSCAudioPropertyID_InputBasicDescription
    property in the ConfigureStdAudio function below.  The input data proc may
    provide exactly the number of packets of audio requested in *ioNumberDataPackets,
    fewer packets, or more packets.  The means to signal end of data is identical
    to that specified for AudioConverters in technical Q&A 1317:
    http://developer.apple.com/qa/qa2001/qa1317.html
    
    Unlike AudioConverters, the source format and destination format can have 
    totally different channel layouts.  SCAudioFillBuffer's processing chain
    uses:
    (1) an AudioConverter for decode (if necessary)
    (2) a MatrixMixer for mixing (if necessary)
    (3) another AudioConverter for encode (if necessary)
*/
// --------------------------------------------------------------------------------

static ComponentResult
mySCAudioInputDataProc( ComponentInstance ci, 
						UInt32 *ioNumberDataPackets, 
						AudioBufferList *ioData, 
						AudioStreamPacketDescription **outDataPacketDescription, 
						void *inRefCon)
{
	ComponentResult err = noErr;
	SCAudioCompressData globs = (SCAudioCompressData)inRefCon;
	UInt32 pullPackets = *ioNumberDataPackets;
	
	*ioNumberDataPackets = 0;
	
	if (globs->mae)
	{
	
	
		UInt32 flags = 0;
		
		// if it's movie audio extraction, we know we're doing pcm, packetdescriptions
		// are unnecessary.
		if ( globs->extractionComplete )
		{
			ioData->mBuffers[0].mData = NULL;
			ioData->mBuffers[0].mDataByteSize = 0;
			goto bail;
		}
        
        if ( ioData->mBuffers[0].mData == NULL )
        {
            UInt32 i, neededBufSize = pullPackets * globs->inasbd.mBytesPerPacket;
            if ( globs->inputBufferSize < neededBufSize )
            {
                globs->inputBufferSize = neededBufSize;
                if ( globs->inputBuffers )
                {
                    for ( i = 0; i < globs->inasbd.mChannelsPerFrame; i++ )
                        free(globs->inputBuffers[i]);
                    free(globs->inputBuffers);
                    globs->inputBuffers = NULL;
                }
                
                globs->inputBuffers = malloc( sizeof(void*) * globs->inasbd.mChannelsPerFrame );
                    
                for ( i = 0; i < globs->inasbd.mChannelsPerFrame; i++ )
                    globs->inputBuffers[i] = malloc( globs->inputBufferSize );
            }
            
            for ( i = 0; i < ioData->mNumberBuffers; i++ )
            {
                ioData->mBuffers[i].mDataByteSize = globs->inputBufferSize;
                ioData->mBuffers[i].mData = globs->inputBuffers[i];
            }
        }
        
		
		logflow("Calling MovieAudioExtractionFillBuffer() for %lu frames\n", pullPackets);
		err = MovieAudioExtractionFillBuffer(	globs->mae, &pullPackets, ioData, &flags);
		if ( flags & kQTMovieAudioExtractionComplete )
		{
			globs->extractionComplete = true;
			logflow("setting kQTMovieAudioExtractionComplete to true\n");
		}
		
		*ioNumberDataPackets = pullPackets;
		logflow("MovieAudioExtractionFillBuffer() returned %lu frames\n", pullPackets);
		
		
		
	}
#if TARGET_OS_MAC
	else if ( globs->af ) {
		
		// do it the audio file way
		
		if ( globs->srcPacketsRead == globs->srcFilePacketCount )
		{
			// end of data
			ioData->mBuffers[0].mData = NULL;
			ioData->mBuffers[0].mDataByteSize = 0;
			goto bail;
		}
		
		if ( pullPackets > (globs->srcFilePacketCount - globs->srcPacketsRead) )
			pullPackets = globs->srcFilePacketCount - globs->srcPacketsRead;
		
		if (outDataPacketDescription)
		{
			if (pullPackets > globs->numPacketDescs)
			{
				if (globs->packetDescs)
					free(globs->packetDescs);
				globs->packetDescs = (AudioStreamPacketDescription*)calloc(pullPackets, sizeof(AudioStreamPacketDescription));
				globs->numPacketDescs = pullPackets;
			}
		}
		
		if ( pullPackets * globs->srcFileMaxPacketSize > globs->srcFileReadBufferSize )
		{
			if (globs->srcFileReadBuffer)
				free(globs->srcFileReadBuffer);
			globs->srcFileReadBufferSize = pullPackets * globs->srcFileMaxPacketSize;
			globs->srcFileReadBuffer = calloc(1, globs->srcFileReadBufferSize);
		}
		
		ioData->mBuffers[0].mData = globs->srcFileReadBuffer;
	
		logflow("Reading %lu packets from src AudioFile\n", pullPackets);
		
		err = AudioFileReadPackets (	globs->af, 
										false,			// inUseCache,
										&ioData->mBuffers[0].mDataByteSize,		// outNumBytes,
										globs->packetDescs,
										globs->srcPacketsRead, 
										&pullPackets, 
										ioData->mBuffers[0].mData);
		if (err)
		{
			fprintf(LOG_FILE, "### AudioFileReadPackets() returned err %ld\n", err);
			goto bail;
		}
		
		logflow("AudioFileReadPackets() returned %ld packets\n", pullPackets);
		
		*ioNumberDataPackets = pullPackets;
		if (outDataPacketDescription)
			*outDataPacketDescription = globs->packetDescs;
		
		globs->srcPacketsRead += pullPackets;
	}
#endif // TARGET_OS_MAC
	else
		err = paramErr;
	
bail:	
	return err;
}



// --------------------------------------------------------------------------------
/*
    DoSCAudioCompress - here is where we call SCAudioFillBuffer repeatedly and
    write the resulting packets to file using QuickTime's AddMediaSample2 API,
    and AudioFile's AudioFileWritePackets API.  Code is also provided to write
    raw PCM (headerless) interleaved frames to disk.
*/
// --------------------------------------------------------------------------------


static OSStatus
DoSCAudioCompress( SCAudioCompressData globs )
{
	OSStatus err = noErr;
	AudioStreamPacketDescription * aspds = NULL;
	AudioStreamBasicDescription asbd;
	UInt32 maxBytesPerPacket = 0;
	AudioBufferList * abl = NULL;
	void * buffer = NULL;
	UInt32 bufferSize = 0;
	SInt32 onlyChannel = 0;
	Boolean requiresPackets = false;
	
	// see if the output format is externally framed and requires packet descriptions
	err = QTGetComponentProperty(	globs->stdAudio, kQTPropertyClass_SCAudio, 
									kQTSCAudioPropertyID_OutputFormatIsExternallyFramed,
									sizeof(requiresPackets), &requiresPackets, NULL );
	if (err)
		fprintf(LOG_FILE, "### Get(kQTSCAudioPropertyID_OutputFormatIsExternallyFramed) returned err %ld\n", err);
	if (requiresPackets)
		aspds = (AudioStreamPacketDescription*)calloc(globs->pullPackets, sizeof(AudioStreamPacketDescription));
	
	
	
	err = QTGetComponentProperty(	globs->stdAudio, kQTPropertyClass_SCAudio, 
									kQTSCAudioPropertyID_BasicDescription,
									sizeof(asbd), &asbd, NULL );
	if (err)
	{
		fprintf(LOG_FILE, "### In DoSCAudioCompress(): QTGetComponentProperty(stdAudio, BasicDescription) returned err %ld\n", err);
		goto bail;
	}
	
	// figure out the max bytes per packet
	if ( asbd.mBytesPerPacket )
	{
		maxBytesPerPacket = asbd.mBytesPerPacket;
	}
	else {
		err = QTGetComponentProperty(	globs->stdAudio, kQTPropertyClass_SCAudio, 
										kQTSCAudioPropertyID_MaximumOutputPacketSize, 
										sizeof(maxBytesPerPacket), &maxBytesPerPacket, NULL);
		if (err)
		{
			fprintf(LOG_FILE, "### Get(kQTSCAudioPropertyID_MaximumOutputPacketSize) returned err %ld\n", err);
			goto bail;
		}
	}
	
	
	// now we can make an audio buffer list big enough for the data we want to get
	bufferSize = maxBytesPerPacket * globs->pullPackets;
	if ( asbd.mFormatFlags & kLinearPCMFormatFlagIsNonInterleaved )
		bufferSize *= asbd.mChannelsPerFrame;
		
	if (bufferSize == 0)
	{
		fprintf(LOG_FILE, "### bufferSize rolled over.\n");
		err = memFullErr;
		goto bail;
	}
	buffer = malloc(bufferSize);
	bufferSize = maxBytesPerPacket * globs->pullPackets;
	
	if ( asbd.mFormatFlags & kLinearPCMFormatFlagIsNonInterleaved )
	{
		abl = (AudioBufferList*)calloc(1, 
				offsetof(AudioBufferList, mBuffers[asbd.mChannelsPerFrame]));
		abl->mNumberBuffers = asbd.mChannelsPerFrame;
	}
	else {
		abl = (AudioBufferList*)calloc(1, offsetof(AudioBufferList, mBuffers[1]));
		abl->mNumberBuffers = 1;
	}
	
	if ( globs->onlyChannel > -1 )
		onlyChannel = globs->onlyChannel;
	
	while ( true )
	{
		UInt32 i, pullPackets = globs->pullPackets;
		void * ptr = buffer;
		
		for (i = 0; i < abl->mNumberBuffers; i++)
		{
			abl->mBuffers[i].mNumberChannels = (abl->mNumberBuffers > 1 ? 1 : asbd.mChannelsPerFrame);
			abl->mBuffers[i].mDataByteSize = bufferSize;
			abl->mBuffers[i].mData = ptr;
			ptr = (UInt8*)ptr + bufferSize;
		}
		
		logflow("calling SCAudioFillBuffer for %lu packets\n", pullPackets);
		err = SCAudioFillBuffer(globs->stdAudio,
								mySCAudioInputDataProc,
								(void*)globs,
								&pullPackets,
								abl,
								aspds);
		if (err)
		{
			fprintf(LOG_FILE, "received err %ld from SCAudioFillBuffer\n", err);
			goto bail;
		}
		else if (pullPackets == 0 && abl->mBuffers[0].mDataByteSize == 0) {
			fprintf(LOG_FILE, "received EOS from SCAudioFillBuffer, finishing\n");
			break;
		}
		
		logflow("SCAudioFillBuffer returned %ld packets\n", pullPackets);
		if (pullPackets)
		{
		#if TARGET_OS_MAC
			if (globs->outaf)
			{
				err = AudioFileWritePackets (	globs->outaf,  
												false,							// inUseCache,
												abl->mBuffers[onlyChannel].mDataByteSize,	// inNumBytes,
												aspds,							// inPacketDescriptions,
												globs->numPacketsWritten,		// inStartingPacket, 
												&pullPackets,					// ioNumPackets, 
												abl->mBuffers[onlyChannel].mData );		// inBuffer
				if (err)
				{
					fprintf(LOG_FILE, "### AudioFileWritePackets() returned err %ld\n", err);
					goto bail;
				}
				
				logflow("AudioFileWritePackets() wrote %ld packets\n", pullPackets);
				globs->numPacketsWritten += pullPackets;
			}
			else
		#endif // TARGET_OS_MAC
            if (globs->writingRaw) {
                UInt32 i;
                
                for (i = 0; i < abl->mNumberBuffers; i++)
                {
					if ( 1 > fwrite(abl->mBuffers[i].mData, abl->mBuffers[i].mDataByteSize, 1, globs->rawFile) )
					{
                        fprintf(LOG_FILE, "### fwrite() returned err %d\n", ferror(globs->rawFile));
                        goto bail;
                    }
                }
            }
			else {
				
				// add to the movie
				if (aspds)
				{
					// if we've got packet descriptions, we need to add them one at a time
					UInt32 i;
					
					for (i = 0; i < pullPackets; i++)
					{
						err = AddMediaSample2(	globs->media,
												(UInt8*)abl->mBuffers[onlyChannel].mData + aspds[i].mStartOffset,
												aspds[i].mDataByteSize,
												(aspds[i].mVariableFramesInPacket 
													? aspds[i].mVariableFramesInPacket 
													: asbd.mFramesPerPacket), //decodeDurationPerSample
												0, //displayOffset
												(SampleDescriptionHandle)globs->sdh,
												1, // numberOfSamples
												0, // sampleFlags
												NULL ); // sampleDecodeTimeOut
						if (err)
						{
							fprintf(LOG_FILE, "### AddMediaSample2() returned err %ld\n", err);
							goto bail;
						}
					}
				}
				else {
					// make believe the frames in each packet are individually accessible.
					// this is commonly known as "THE LIE".  It's the truth for PCM.
					// It's a lie for all CBR compressed formats.
					ItemCount numSamples = pullPackets * asbd.mFramesPerPacket;
					err = AddMediaSample2(	globs->media,
											abl->mBuffers[onlyChannel].mData,
											abl->mBuffers[onlyChannel].mDataByteSize,
											1, //decodeDurationPerSample
											0, //displayOffset
											(SampleDescriptionHandle)globs->sdh,
											numSamples, // numberOfSamples
											0, // sampleFlags
											NULL ); // sampleDecodeTimeOut
					if (err)
					{
						fprintf(LOG_FILE, "### AddMediaSample2() returned err %ld\n", err);
						goto bail;
					}
				}
			}
		}
	}
	
bail:
	if (aspds)
		free(aspds);
	if (buffer)
		free(buffer);
	
	return err;
}


// --------------------------------------------------------------------------------
/*
    ConfigureStdAudio - Here's where we instantiate StdAudio, and feed it information
    about our source (asbd, layout, magic cookie).  If we've not been directed to
    suppress ui, we display the StdAudio modal dialog and let the user select
    his/her desired output format.
*/
// --------------------------------------------------------------------------------


static OSStatus
ConfigureStdAudio ( SCAudioCompressData globs )
{
    OSStatus err = noErr;
    AudioStreamBasicDescription inasbd;
	AudioChannelLayout * pLayout = NULL;
	void * magicCookie = NULL;
	UInt32 size = sizeof(inasbd), layoutSize = 0, cookieSize = 0;
	UInt32 numDiscreteInputChannels = 0;
    
    err = OpenADefaultComponent(StandardCompressionType, StandardCompressionSubTypeAudio, &globs->stdAudio);
    if (err)
    {
        fprintf(LOG_FILE, "### OpenADefaultComponent('scdi', 'audi') returned err %ld\n", err);
        goto bail;
    }

#if TARGET_OS_MAC
    if (globs->af)
    {
        
		
		
        // 1. get asbd
        err = AudioFileGetProperty( globs->af, kAudioFilePropertyDataFormat, &size, &inasbd);
        if (err)
        {
            fprintf(LOG_FILE, "### AudioFileGetProperty(kAudioFilePropertyDataFormat) returned err %ld\n", err);
            goto bail;
        }
		
		
		// 2. get channel layout
		if ( noErr == AudioFileGetPropertyInfo( globs->af, kAudioFilePropertyChannelLayout, &layoutSize, NULL ) )
		{
			pLayout = (AudioChannelLayout*)calloc(1, layoutSize);
			if ( noErr != AudioFileGetProperty( globs->af, kAudioFilePropertyChannelLayout, &layoutSize, pLayout) )
			{
				free(pLayout);
				pLayout = NULL;
			}
		}
		
		
		// 3. get magic cookie
		if ( noErr == AudioFileGetPropertyInfo( globs->af, kAudioFilePropertyMagicCookieData, &cookieSize, NULL ) )
		{
			magicCookie = calloc(1, cookieSize);
			if ( noErr != AudioFileGetProperty( globs->af, kAudioFilePropertyMagicCookieData, &cookieSize, magicCookie) )
			{
				free(magicCookie);
				magicCookie = NULL;
			}
		}
		
		numDiscreteInputChannels = inasbd.mChannelsPerFrame;
		
    }
#endif // TARGET_OS_MAC
    else if ( globs->m ) {
		
		UInt32 maxSampleSize = 0;
		Boolean allChansDiscrete;
	
        // do it the qt way - use MovieAudioExtraction API's
		err = MovieAudioExtractionBegin( globs->m, 0, &globs->mae );
		if (err)
		{
			fprintf(LOG_FILE, "### MovieAudioExtractionBegin() returned err %ld\n", err);
			goto bail;
		}
		
		
		
		
		// first things first, temporarily set the extraction session to "AllChannelsDiscrete", so
		// we can get the true (unmixed) channel count in the source movie.  We want the user to
		// have the option of exporting all the source channels without doing any mixing, and
		// for that we need to tell StdAudio the discrete number of channels to allow
		allChansDiscrete = true;
		
		err = MovieAudioExtractionSetProperty(  globs->mae,
												kQTPropertyClass_MovieAudioExtraction_Movie,
												kQTMovieAudioExtractionMoviePropertyID_AllChannelsDiscrete,
												sizeof(allChansDiscrete), &allChansDiscrete);
		if (err)
		{
			fprintf(LOG_FILE, "### MovieAudioExtractionSetProperty(allChannelsDiscrete) returned err %ld\n", err);
			goto bail;
		}
		err = MovieAudioExtractionGetProperty(	globs->mae, kQTPropertyClass_MovieAudioExtraction_Audio,
												kQTMovieAudioExtractionAudioPropertyID_AudioStreamBasicDescription,
												sizeof(inasbd), &inasbd, NULL );
		if (err)
		{
			fprintf(LOG_FILE, "### MovieAudioExtractionGetProperty(asbd (allChannelsDiscrete)) returned err %ld\n", err);
			goto bail;
		}
		numDiscreteInputChannels = inasbd.mChannelsPerFrame;
		
		allChansDiscrete = false;
		err = MovieAudioExtractionSetProperty(  globs->mae,
												kQTPropertyClass_MovieAudioExtraction_Movie,
												kQTMovieAudioExtractionMoviePropertyID_AllChannelsDiscrete,
												sizeof(allChansDiscrete), &allChansDiscrete);
		if (err)
		{
			fprintf(LOG_FILE, "### MovieAudioExtractionSetProperty(allChannelsDiscrete) returned err %ld\n", err);
			goto bail;
		}
		
		
		
		
		
		// not doing AllChannelsDiscrete any more - find out info about the summary mixed audio
		
		err = MovieAudioExtractionGetProperty(	globs->mae, kQTPropertyClass_MovieAudioExtraction_Audio,
												kQTMovieAudioExtractionAudioPropertyID_AudioStreamBasicDescription,
												sizeof(inasbd), &inasbd, NULL );
		if (err)
		{
			fprintf(LOG_FILE, "### MovieAudioExtractionGetProperty(asbd) returned err %ld\n", err);
			goto bail;
		}
        
        // get max sample size (needed for accurate 'alac').
        err = MovieAudioExtractionGetProperty(	globs->mae, kQTPropertyClass_MovieAudioExtraction_Audio,
												kQTAudioPropertyID_MaxAudioSampleSize,
												sizeof(maxSampleSize), &maxSampleSize, NULL );
        if (noErr == err && maxSampleSize)
        {
            inasbd.mBitsPerChannel = maxSampleSize;
            if (maxSampleSize <= 32)
            {
                inasbd.mFormatFlags &= ~(kLinearPCMFormatFlagIsFloat | kLinearPCMFormatFlagIsNonInterleaved);
                inasbd.mFormatFlags |= kLinearPCMFormatFlagIsSignedInteger;
                inasbd.mBytesPerFrame = inasbd.mBytesPerPacket = maxSampleSize/8 * inasbd.mChannelsPerFrame; 
            }
            
            err = MovieAudioExtractionSetProperty(  globs->mae,
                                                    kQTPropertyClass_MovieAudioExtraction_Audio,
                                                    kQTMovieAudioExtractionAudioPropertyID_AudioStreamBasicDescription,
                                                    sizeof(inasbd), &inasbd);
            if (err)
            {
                fprintf(LOG_FILE, "### MovieAudioExtractionSetProperty(asbd) returned err %ld\n", err);
                goto bail;
            }
        }
		
		
		if ( noErr == MovieAudioExtractionGetPropertyInfo(	globs->mae, kQTPropertyClass_MovieAudioExtraction_Audio,
															kQTMovieAudioExtractionAudioPropertyID_AudioChannelLayout,
															NULL, &layoutSize, NULL ) )
		{
			if (layoutSize)
			{
				pLayout = (AudioChannelLayout*)calloc(1, layoutSize);
				if (noErr != MovieAudioExtractionGetProperty(globs->mae, kQTPropertyClass_MovieAudioExtraction_Audio,
															kQTMovieAudioExtractionAudioPropertyID_AudioChannelLayout,
															layoutSize, pLayout, &layoutSize))
				{
					free(pLayout);
					pLayout = NULL;
				}
			}
		}
    }
	else {
	
		fprintf(LOG_FILE, "### ConfigureStdAudio() called without a source AudioFileID or Movie!\n");
		err = paramErr;
		goto bail;
		
	}
	
	
	
	
	
	// set the input desc info on stdaudio
	err = QTSetComponentProperty(   globs->stdAudio, kQTPropertyClass_SCAudio, 
									kQTSCAudioPropertyID_InputBasicDescription,
									sizeof(inasbd), &inasbd );
	if (err)
	{
		fprintf(LOG_FILE, "### QTSetComponentProperty(stdaudio, InputBasicDescription) returned err %ld\n", err);
		goto bail;
	}
	
	if (pLayout)
	{
		err = QTSetComponentProperty(	globs->stdAudio, kQTPropertyClass_SCAudio,
										kQTSCAudioPropertyID_InputChannelLayout,
										layoutSize, pLayout );
		if (err)
		{
			fprintf(LOG_FILE, "### QTSetComponentProperty(stdaudio, InputChannelLayout) returned err %ld\n", err);
			goto bail;
		}
	}
	
	if (magicCookie)
	{
		err = QTSetComponentProperty(	globs->stdAudio, kQTPropertyClass_SCAudio,
										kQTSCAudioPropertyID_InputMagicCookie,
										cookieSize, magicCookie );
		if (err)
		{
			fprintf(LOG_FILE, "### QTSetComponentProperty(stdaudio, InputMagicCookie) returned err %ld\n", err);
			goto bail;
		}
	}
	
	
	// the following function call is optional - we can specify a list of allowable AudioChannelLayoutTag's
	// to StdAudio.  StdAudio will prune its list of available layout tags to our specified set.
	// If you want StdAudio to allow export to a custom layout (i.e. if the source movie has a 
	// whacky AudioChannelLayout that is not encompassed by any of the AudioChannelLayoutTag's),
	// or to show "n Discrete Channels" as an option, you need to set the
	// kQTSCAudioPropertyID_ClientRestrictedChannelLayoutTagList property, as shown
	// below.
	{
		AudioChannelLayoutTag tagList[] = 
			{
				kAudioChannelLayoutTag_DiscreteInOrder | numDiscreteInputChannels,	// OR in the real discrete number of channels
				kAudioChannelLayoutTag_UseChannelDescriptions,						// allows passthru of source's whacky custom channel layout (if it has one)
				
				kAudioChannelLayoutTag_Mono,
				kAudioChannelLayoutTag_Stereo,
				kAudioChannelLayoutTag_Quadraphonic,
				kAudioChannelLayoutTag_MPEG_5_0_A,
				kAudioChannelLayoutTag_MPEG_5_0_B,
				kAudioChannelLayoutTag_MPEG_5_0_C,
				kAudioChannelLayoutTag_MPEG_5_0_D,
				kAudioChannelLayoutTag_MPEG_5_1_A,
				kAudioChannelLayoutTag_MPEG_5_1_B,
				kAudioChannelLayoutTag_MPEG_5_1_C,
				kAudioChannelLayoutTag_MPEG_5_1_D,
				kAudioChannelLayoutTag_AudioUnit_6_0,
				kAudioChannelLayoutTag_AAC_6_0,
				kAudioChannelLayoutTag_MPEG_6_1_A,
				kAudioChannelLayoutTag_AAC_6_1,
				kAudioChannelLayoutTag_AudioUnit_7_0,
				kAudioChannelLayoutTag_AAC_7_0,
				kAudioChannelLayoutTag_MPEG_7_1_A,
				kAudioChannelLayoutTag_MPEG_7_1_B,
				kAudioChannelLayoutTag_MPEG_7_1_C,
				kAudioChannelLayoutTag_Emagic_Default_7_1,
				kAudioChannelLayoutTag_AAC_Octagonal
			 };
			 
		(void)QTSetComponentProperty(globs->stdAudio,
				kQTPropertyClass_SCAudio,
				kQTSCAudioPropertyID_ClientRestrictedChannelLayoutTagList,
				sizeof(tagList),
				tagList);
	}
	
	
	
	if (globs->onlyChannel > -1 ||
        globs->writingRaw)
	{
		// if globs->onlyChannel > -1, we need to only write one of the source channels.  This requires that
		// (1) onlyChannel must be < inasbd.mChannelsPerFrame
		// (2) StdAudio must be configured to only do deinterleaved
		// (3) Stdaudio must be configured to only do PCM (as per 2 above)
		SCAudioFormatFlagsRestrictions restrictions = { 0 };
		OSType restrictedFormat = 'lpcm';
		
		if (globs->onlyChannel >= (SInt32)inasbd.mChannelsPerFrame)
		{
			fprintf(LOG_FILE, "### The requested channel (%lu) to write is greater than the source number of channels (%lu)\n",
				globs->onlyChannel, inasbd.mChannelsPerFrame);
			err = badFormat;
			goto bail;
		}
		
        restrictions.formatFlagsMask = kAudioFormatFlagIsNonInterleaved;
        
        if (globs->onlyChannel > -1)
        {
            restrictions.formatFlagsValues = kAudioFormatFlagIsNonInterleaved;
        }
        else {
            // if writing raw, we want to force interleaved
        }
		
		err = QTSetComponentProperty(	globs->stdAudio, kQTPropertyClass_SCAudio,
										kQTSCAudioPropertyID_ClientRestrictedLPCMFlags,
										sizeof(restrictions), &restrictions );
		if (err)
		{	
			fprintf(LOG_FILE, "### QTSetComponentProperty(stdaudio, ClientRestrictedLPCMFlags) returned err %ld\n", err);
			goto bail;
		}
		
		err = QTSetComponentProperty(	globs->stdAudio, kQTPropertyClass_SCAudio,
										kQTSCAudioPropertyID_ClientRestrictedCompressionFormatList,
										sizeof(restrictedFormat), &restrictedFormat );
		if (err)
		{
			fprintf(LOG_FILE, "### QTSetComponentProperty(stdaudio, ClientRestrictedCompressionFormatList) returned err %ld\n", err);
			goto bail;
		}
	}
	
	
	
	// now StdAudio has been configured with the input info.
	// Display the dialog now
	if ( false == globs->suppressUI )
	{
		SCExtendedProcs xProcs;
		
		
		// make our command line test app an app that can become the foreground process
	#if TARGET_OS_MAC
		extern	int32_t	CPSEnableForegroundOperation( ProcessSerialNumber * PSN );
		
		ProcessSerialNumber psn;
		
		GetCurrentProcess(&psn);
		CPSEnableForegroundOperation(&psn);
		SetFrontProcess(&psn);
	#endif
		
		memset(&xProcs, 0, sizeof(xProcs));
		strcpy((char*)xProcs.customName + 1, "Select Output Format");
		xProcs.customName[0] = (unsigned char)strlen((char*)xProcs.customName + 1);
		(void)QTSetComponentProperty(	globs->stdAudio, kQTPropertyClass_SCAudio, 
										kQTSCAudioPropertyID_ExtendedProcs, 
										sizeof(xProcs), &xProcs);
										
		err = SCRequestImageSettings(globs->stdAudio);
		if (err == userCanceledErr)
			err = noErr; // User cancelling is ok.
		if (err)
		{
			fprintf(LOG_FILE, "### SCRequestImageSettings() returned err %ld\n", err);
			goto bail;
		}
	}
	
	// Now we've given the user an opportunity to specify output settings, including
	// the "Render Quality" for the export operation (Render Quality is one of the
	// choices in the dialog).  If we are extracting from a movie, we should apply
	// that same render quality to the MovieAudioExtraction Session, so that
	// any scaled edits or codec decompressions are done at the same render quality
	// that was specified in the dialog
	if (globs->mae)
	{
		UInt32 renderQuality;
		
		if (noErr == QTGetComponentProperty(globs->stdAudio, kQTPropertyClass_SCAudio,
											kQTSCAudioPropertyID_RenderQuality,
											sizeof(renderQuality), &renderQuality, NULL))
		{
			err = MovieAudioExtractionSetProperty(  globs->mae,
													kQTPropertyClass_MovieAudioExtraction_Audio,
													kQTMovieAudioExtractionAudioPropertyID_RenderQuality,
													sizeof(renderQuality), &renderQuality);
			if (err)
			{
				fprintf(LOG_FILE, "### MovieAudioExtractionSetProperty(renderQuality) returned err %ld\n", err);
				goto bail;
			}
		}
	}
	
	// if we've been configured to produce discrete output channels, we need
	// to set up MovieAudioExtraction to render the PCM samples without mixing
	if (globs->mae)
	{
		AudioChannelLayout * outputLayout = NULL;
		UInt32				 outputLayoutSize = 0;
		
		err = QTGetComponentPropertyInfo(globs->stdAudio, kQTPropertyClass_SCAudio,
				kQTSCAudioPropertyID_ChannelLayout, NULL, &outputLayoutSize, NULL);
		if (err)
		{
			fprintf(LOG_FILE, "### QTGetComponentPropertyInfo(layoutSize) returned err %ld\n", err);
			goto bail;
		}
		
		outputLayout = (AudioChannelLayout*)malloc(outputLayoutSize);
		
		err = QTGetComponentProperty(globs->stdAudio, kQTPropertyClass_SCAudio,
											kQTSCAudioPropertyID_ChannelLayout,
											outputLayoutSize, outputLayout, &outputLayoutSize);
		if (err)
		{
			fprintf(LOG_FILE, "### QTGetComponentProperty(layoutSize) returned err %ld\n", err);
		}
		if (err == noErr && 
			((outputLayout->mChannelLayoutTag & 0xFFFF0000) == kAudioChannelLayoutTag_DiscreteInOrder))
		{
			Boolean allChansDiscrete = true;
			err = MovieAudioExtractionSetProperty(  globs->mae,
													kQTPropertyClass_MovieAudioExtraction_Movie,
													kQTMovieAudioExtractionMoviePropertyID_AllChannelsDiscrete,
													sizeof(allChansDiscrete), &allChansDiscrete);
			if (err)
			{
				fprintf(LOG_FILE, "### MovieAudioExtractionSetProperty(allChannelsDiscrete - for real this time!) returned err %ld\n", err);
			}
			
			if (err == noErr)
			{
				// re-get the input asbd (with new number of channels)
				err = MovieAudioExtractionGetProperty(	globs->mae, kQTPropertyClass_MovieAudioExtraction_Audio,
														kQTMovieAudioExtractionAudioPropertyID_AudioStreamBasicDescription,
														sizeof(inasbd), &inasbd, NULL );
				if (err)
				{
					fprintf(LOG_FILE, "### MovieAudioExtractionGetProperty(asbd) returned err %ld\n", err);
				}
				
				// tell StdAudio about the input format change
				if (err == noErr)
				{
					SoundDescriptionHandle sdh = NULL;
					
					err = QTSoundDescriptionCreate(&inasbd, outputLayout, outputLayoutSize, NULL, 0,
							kQTSoundDescriptionKind_Movie_AnyVersion, &sdh);
							
					if (err)
					{
						fprintf(LOG_FILE, "### QTSoundDescriptionCreate() returned err %ld\n", err);
					}
					
					if (err == noErr)
					{
						err = QTSetComponentProperty(globs->stdAudio, kQTPropertyClass_SCAudio,
											kQTSCAudioPropertyID_InputSoundDescription,
											sizeof(sdh), &sdh);
						if (err)
						{
							fprintf(LOG_FILE, "### QTSetComponentProperty(InputSDH) returned err %ld\n", err);
						}
					}
					DisposeHandle((Handle)sdh);
				}
			}
		}
		free(outputLayout);
		if (err) 
			goto bail;
	}
    
    globs->inasbd = inasbd;
    
bail:
	if (pLayout)
		free(pLayout);
	if (magicCookie)
		free(magicCookie);
    return err;
}


// --------------------------------------------------------------------------------
/*
    OpenDestinationFile - Here's where we open the destination file, either a
    QuickTime movie, .caf file, or raw pcm file.
*/
// --------------------------------------------------------------------------------


static OSStatus
OpenDestinationFile ( SCAudioCompressData globs )
{
    OSStatus err = noErr;
#if TARGET_OS_MAC
	CFStringRef	filename = NULL;
#endif
	UInt32 size;
	AudioStreamBasicDescription format = { 0 };
	
	// get the format we'll be writing
	err = QTGetComponentProperty(	globs->stdAudio, kQTPropertyClass_SCAudio, 
									kQTSCAudioPropertyID_BasicDescription,
									sizeof(format), &format, NULL );
	if (err)
	{
		fprintf(LOG_FILE, "### QTGetComponentProperty(stdAudio, BasicDescription) returned err %ld\n", err);
		goto bail;
	}
	
	if (globs->onlyChannel > -1)
	{
		if (globs->onlyChannel >= (SInt32)format.mChannelsPerFrame)
		{
			fprintf(LOG_FILE, "### The requested channel (%lu) to write is greater than the source number of channels (%lu)\n",
				globs->onlyChannel + 1, format.mChannelsPerFrame);
			err = badFormat;
			goto bail;
		}
		
		format.mFormatFlags &= ~kLinearPCMFormatFlagIsNonInterleaved;
		format.mChannelsPerFrame = 1;
	}
	
	
	
	// delete the file if there already is one
	remove(globs->outFile);
	
	err = noErr;
	

		
	if (globs->writingMovie)
	{
		
		CFStringRef fullpath = CFStringCreateWithCString(NULL, globs->outFile, kCFStringEncodingASCII);
		Handle dataRef = NULL;
		OSType dataRefType = 0;
		Track t;
		
		
		err = QTNewDataReferenceFromFullPathCFString(fullpath, kQTNativeDefaultPathStyle, 0, &dataRef, &dataRefType);
		CFRelease(fullpath);
		if( err ) 
		{
			fprintf( LOG_FILE, "### QTNewDataReferenceFromFullPathCFString() returned err %ld\n", err );
			goto bail;
		}
		
		err = CreateMovieStorage(	dataRef, dataRefType, 'TVOD', 0, createMovieFileDeleteCurFile,
									&globs->dataH, &globs->outm );
		DisposeHandle(dataRef);
		if (err)
		{
			fprintf( LOG_FILE, "### CreateMovieStorage() returned err %ld\n", err);
			goto bail;
		}
		
		t = NewMovieTrack(globs->outm, 0, 0, kFullVolume);
		err = GetMoviesError();
		if (err)
		{
			fprintf(LOG_FILE, "### NewMovieTrack() returned err %ld\n", err);
			goto bail;
		}
		
		globs->media = NewTrackMedia(t, SoundMediaType, (TimeScale)format.mSampleRate, NULL, 0);
		err = GetMoviesError();
		if (err)
		{
			fprintf(LOG_FILE, "### NewTrackMedia() returned err %ld\n", err);
			goto bail;
		}
		
		err = BeginMediaEdits( globs->media );
		if (err)
		{
			fprintf(LOG_FILE, "### BeginMediaEdits() returned err %ld\n", err);
		}
		
		err = QTSoundDescriptionCreate( &format, NULL, 0, NULL, 0, 
										kQTSoundDescriptionKind_Movie_LowestPossibleVersion,
										&globs->sdh);
		if (err)
		{
			fprintf(LOG_FILE, "### QTSoundDescriptionCreate() returned err %ld\n", err);
			goto bail;
		}
		
		
		
	}
    else if ( globs->writingRaw ) {

		globs->rawFile = fopen(globs->outFile, "w");

        if ( NULL == globs->rawFile )
        {
            fprintf(LOG_FILE, "### fopen() returned err %d\n", errno);
			goto bail;
        }

    }
#if TARGET_OS_MAC
	else {
		unsigned char sep = '/'; // For Windows, sep would be '\\'
		FSRef fsdir, fspath;
		Boolean isDirectory = false;
		UInt32 i, len = (UInt32)strlen(globs->outFile);

			// separate the directory from the last component of the path
		for (i = len; i > 0; i--)
		{
			if ( globs->outFile[i - 1] == sep )
			{
				char temp;
				filename = CFStringCreateWithCString(NULL, globs->outFile + i, kCFStringEncodingASCII);
				temp = globs->outFile[i];
				globs->outFile[i] = '\0';
				err = FSPathMakeRef((UInt8*)globs->outFile, &fsdir, &isDirectory);
				globs->outFile[i] = temp;
				if (err || !isDirectory)
				{
					fprintf(LOG_FILE, "### FSPathMakeRef for \"%s\" returned err %ld\n", globs->outFile, err);
					goto bail;
				}
				break;
			}
		}
		
		if ( NULL == filename )
		{
			fprintf(LOG_FILE, "### Couldn't parse \"%s\" for the last path component\n", globs->outFile);
			err = paramErr;
			goto bail;
		}

		err = AudioFileCreate (	&fsdir, filename, kAudioFileCAFType, &format,
								0, // inFlags,
								&fspath, &globs->outaf);
		if (err)
		{
			fprintf(LOG_FILE, "### AudioFileCreate() returned err %ld\n", err);
			goto bail;
		}
	}	
#endif	
	
	if ( globs->onlyChannel == -1 && false == globs->writingRaw )
	{
		// set the layout on the file
		if (noErr == QTGetComponentPropertyInfo(globs->stdAudio, kQTPropertyClass_SCAudio,
												kQTSCAudioPropertyID_ChannelLayout,
												NULL, &size, NULL))
		{
			if (size)
			{
				AudioChannelLayout * pLayout = (AudioChannelLayout*)calloc(1, size);
				if (noErr == QTGetComponentProperty(globs->stdAudio, kQTPropertyClass_SCAudio,
													kQTSCAudioPropertyID_ChannelLayout,
													size, pLayout, &size))
				{
				#if TARGET_OS_MAC
					if (globs->outaf)
					{
						err = AudioFileSetProperty(globs->outaf, kAudioFilePropertyChannelLayout, size, pLayout);
						if (err)
							fprintf(LOG_FILE, "### AudioFileSetProperty(kAudioFilePropertyChannelLayout) returned err %ld\n", err);
					}
					else 
				#endif
					{
						err = QTSoundDescriptionSetProperty(globs->sdh, kQTPropertyClass_SoundDescription,
															kQTSoundDescriptionPropertyID_AudioChannelLayout,
															size, pLayout);
						if (err)
							fprintf(LOG_FILE, "### QTSoundDescriptionSetProperty(ChannelLayout) returned err %ld\n", err);
					}
				}
				free(pLayout);
				if (err)
					goto bail;
			}
		}
		
		
		// set the magic cookie on the file
		if (noErr == QTGetComponentPropertyInfo(globs->stdAudio, kQTPropertyClass_SCAudio,
												kQTSCAudioPropertyID_MagicCookie,
												NULL, &size, NULL))
		{
			if (size)
			{
				void * magicCookie = calloc(1, size);
				if (noErr == QTGetComponentProperty(globs->stdAudio, kQTPropertyClass_SCAudio,
													kQTSCAudioPropertyID_MagicCookie,
													size, magicCookie, &size))
				{
				#if TARGET_OS_MAC
					if (globs->outaf)
					{
						err = AudioFileSetProperty(globs->outaf, kAudioFilePropertyMagicCookieData, size, magicCookie);
						if (err)
							fprintf(LOG_FILE, "### AudioFileSetProperty(kAudioFilePropertyChannelLayout) returned err %ld\n", err);
					}
					else 
				#endif
					{
						err = QTSoundDescriptionSetProperty(globs->sdh, kQTPropertyClass_SoundDescription,
															kQTSoundDescriptionPropertyID_MagicCookie,
															size, magicCookie);
						if (err)
							fprintf(LOG_FILE, "### QTSoundDescriptionSetProperty(MagicCookie) returned err %ld\n", err);
					}
				}
				free(magicCookie);
				if (err)
					goto bail;
			}
		}
	}
	
	
	
	
bail:
#if TARGET_OS_MAC
	if (filename)
		CFRelease(filename);
#endif
    return err;
}


// --------------------------------------------------------------------------------
/*
    OpenSourceFile - Here's where we open the source file.  First we try to open
    it using AudioFileOpen, and fall back to importing it as a QuickTime movie
    if AudioFile doesn't understand the file format.
*/
// --------------------------------------------------------------------------------


static OSStatus
OpenSourceFile( SCAudioCompressData globs )
{
    OSStatus err = noErr;
    
    if ( NULL == globs->inFile || NULL == globs->outFile )
        return paramErr;

#if TARGET_OS_MAC    
    if (false == globs->forceQTRead)
    {
		UInt32 size;
		FSRef fsref;
        err = FSPathMakeRef( (UInt8*)globs->inFile, &fsref, NULL );
        if (err)
        {
            fprintf(LOG_FILE, "### FSPathMakeRef(\"%s\") returned err %ld\n", globs->inFile, err);
            goto bail;
        }
		err = AudioFileOpen( &fsref, fsRdPerm, 0, &globs->af );
        if (err)
		{
            fprintf(LOG_FILE, "AudioFileOpen(\"%s\") returned err %ld (reverting to QT read path)\n", globs->inFile, err);
			goto readWithQT;
		}
			
		size = sizeof(globs->srcFilePacketCount);
		err = AudioFileGetProperty(	globs->af, kAudioFilePropertyAudioDataPacketCount,
									&size, &globs->srcFilePacketCount);
		if (err)
		{
			fprintf(LOG_FILE, "AudioFileGetProperty(kAudioFilePropertyAudioDataPacketCount) returned err %ld\n", err);
			goto bail;
		}
		
		size = sizeof(globs->srcFileMaxPacketSize);
		err = AudioFileGetProperty(	globs->af, kAudioFilePropertyMaximumPacketSize,
									&size, &globs->srcFileMaxPacketSize);
		if (err)
		{
			fprintf(LOG_FILE, "AudioFileGetProperty(kAudioFilePropertyMaximumPacketSize) returned err %ld\n", err);
			goto bail;
		}
    }

readWithQT:    
    // open the input file with QT if needed
    if ( globs->af == 0 )
#endif // TARGET_OS_MAC
    {
        Handle dataRef = NULL;
        OSType dataRefType = 0;
        short resID = 0;
		CFStringRef fullpath = CFStringCreateWithCString(NULL, globs->inFile, kCFStringEncodingASCII);
        
		err = QTNewDataReferenceFromFullPathCFString(fullpath, kQTNativeDefaultPathStyle, 0, &dataRef, &dataRefType);
		CFRelease(fullpath);
        if (err)
        {
            fprintf(LOG_FILE, "### QTNewDataReferenceFromFullPathCFString(\"%s\") returned err %ld\n", globs->inFile, err);
            goto bail;
        }
        
        err = NewMovieFromDataRef(  &globs->m, 
                                    newMovieActive | newMovieDontAskUnresolvedDataRefs,
                                    &resID,
                                    dataRef, dataRefType);
        DisposeHandle(dataRef);
        if (err)
        {
            fprintf(LOG_FILE, "### NewMovieFromDataRef() returned err %ld\n", err);
            goto bail;
        }
    }
bail:
    return err;
}


// --------------------------------------------------------------------------------


static OSStatus
RunTheTest( SCAudioCompressData globs )
{
    OSStatus err = noErr;
	
#if TARGET_OS_WIN32
	InitializeQTML(0);
#endif
	err = EnterMovies();
	if (err) 
	{ 
		fprintf(LOG_FILE, "EnterMovies() returned err %ld\n", err);
		goto bail; 
	}
    
    err = OpenSourceFile(globs);
    if (err) goto bail;
    
    err = ConfigureStdAudio(globs);
    if (err) goto bail;
    
    err = OpenDestinationFile(globs);
    if (err) goto bail;
	
	err = DoSCAudioCompress(globs);
	if (err) goto bail;
bail:
    return err;
}



// --------------------------------------------------------------------------------


static void
PrintUsage(FILE * f)
{
#if TARGET_OS_MAC
    fprintf(f, "scaudiocompress -i /path/to/inputfile -o /path/to/outputfile.(caf | mov | pcm) { [-writeraw] | [-noui] | [-pullPackets #] | [-forceqtinput] }\n");
    fprintf(f, "\t(default output file format is .caf)\n");
#else
    fprintf(f, "scaudiocompress -i /path/to/inputfile -o /path/to/outputfile.(mov | pcm) { [-writeraw] | [-noui] | [-pullPackets #] | [-forceqtinput] }\n");
    fprintf(f, "\t(default output file format is .mov)\n");
#endif
    fprintf(f, "\t-noui          = Run automated test.  No StdAudio dialog UI is shown.\n");
    fprintf(f, "\t-pullpackets   = The number of packets of audio to pull for at a time.  Default is %lu.\n", DEFAULT_PULL_FRAMES);
    fprintf(f, "\t-forceqtinput  = Don't try to open the input file using AudioFile API's, just use QuickTime API's\n");
	fprintf(f, "\t-channel       = Only write the specified input channel (zero-based) to the output movie/audio file\n");
}


// --------------------------------------------------------------------------------


int main (int argc, const char * argv[]) {
    int i;
    OSStatus err = noErr;
    SCAudioCompressDataRecord globs = { 0 };
    
    globs.pullPackets = DEFAULT_PULL_FRAMES;
	globs.onlyChannel = -1;
    
    for (i = 1; i < argc; i++)
    {
        if ( (0 == strncmp("-input", argv[i], 2)) && i + 1 < argc )
            globs.inFile = (char*)argv[++i];
		if ( (0 == strncmp("-channel", argv[i], 2)) && i + 1 < argc )
			globs.onlyChannel = strtol(argv[++i], NULL, 0);
        if ( (0 == strncmp("-output", argv[i], 2)) && i + 1 < argc )
            globs.outFile = (char*)argv[++i];
        if ( 0 == strncmp("-noui", argv[i], 2) )
            globs.suppressUI = true;
        if ( (0 == strncmp("-pullpackets", argv[i], 2)) && i + 1 < argc )
            globs.pullPackets = strtoul(argv[++i], NULL, 0);
        if ( 0 == strncmp("-forceqtinput", argv[i], 2) )
            globs.forceQTRead = true;
    }
    
    if ( NULL == globs.inFile || NULL == globs.outFile )
    {
        PrintUsage(LOG_FILE);
        err = paramErr;
        goto bail;
    }
    
    // see if they want to write a quicktime movie, or raw pcm file
	if ( strlen(globs.outFile) > 4 )
	{
        const char * extension = globs.outFile + strlen(globs.outFile) - 4;
		if ( 0 == strcmp(extension, ".mov") )
			globs.writingMovie = true;
        else if ( 0 == strcmp(extension, ".pcm") )
            globs.writingRaw = true;
	}
    
    
    err = RunTheTest( &globs );
    if (err)
    {
        fprintf(LOG_FILE, "### DoSCAudioCompressTest returned %ld\n", err);
        goto bail;
    }
    
bail:

    // close up stuff
    if (globs.stdAudio)
        CloseComponent(globs.stdAudio);
    
    if (globs.mae)
        MovieAudioExtractionEnd(globs.mae);
        
    if (globs.m)
        DisposeMovie(globs.m);

#if TARGET_OS_MAC        
    if (globs.af)
        AudioFileClose(globs.af);
#endif

	if (globs.packetDescs)
		free(globs.packetDescs);
		
	if (globs.srcFileReadBuffer)
		free(globs.srcFileReadBuffer);

#if TARGET_OS_MAC
    if (globs.outaf)
    {
        OSStatus afcloseErr = AudioFileClose(globs.outaf);
        if (afcloseErr)
            fprintf(LOG_FILE, "### AudioFileClose \"%s\" returned %ld\n", globs.outFile, afcloseErr);
    }
#endif
	
	if (globs.outm)
	{
		Track t = GetMediaTrack( globs.media );
		err = EndMediaEdits( globs.media );
		if (err)
			fprintf(LOG_FILE, "### EndMediaEdits() returned err %ld\n", err);
		
		err = InsertMediaIntoTrack( t, 0, 0, GetMediaDuration(globs.media), fixed1 );
		if (err)
			fprintf(LOG_FILE, "### InsertMediaIntoTrack() returned err %ld\n", err);
			
		err = AddMovieToStorage( globs.outm, globs.dataH );
		if (err)
			fprintf(LOG_FILE, "### AddMovieToStorage() returned err %ld\n", err);
			
		CloseMovieStorage( globs.dataH );
		DisposeMovie(globs.outm);
		DisposeHandle((Handle)globs.sdh);
	}
    
    if ( globs.inputBuffers )
    {
        for ( i = 0; i < (int)globs.inasbd.mChannelsPerFrame; i++ )
            free(globs.inputBuffers[i]);
        free(globs.inputBuffers);
    }
    
    if (globs.writingRaw && globs.rawFile)
    {
		fclose(globs.rawFile);
    }

    return err;
}
