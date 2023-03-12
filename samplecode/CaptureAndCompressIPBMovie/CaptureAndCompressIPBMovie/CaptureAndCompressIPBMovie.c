/*

File: CaptureAndCompressIPBMovie.c

Abstract: Shows how to capture video, recompress it on the fly with H.264 and
store the output in a movie file, using QuickTime 7's Compression Session APIs.
Shows how to capture audio using SGAudioMediaType SGChannel.

Version: 1.1

© Copyright 2005 Apple Computer, Inc. All rights reserved.

IMPORTANT:  This Apple software is supplied to 
you by Apple Computer, Inc. ("Apple") in 
consideration of your agreement to the following 
terms, and your use, installation, modification 
or redistribution of this Apple software 
constitutes acceptance of these terms.  If you do 
not agree with these terms, please do not use, 
install, modify or redistribute this Apple 
software.

In consideration of your agreement to abide by 
the following terms, and subject to these terms, 
Apple grants you a personal, non-exclusive 
license, under Apple's copyrights in this 
original Apple software (the "Apple Software"), 
to use, reproduce, modify and redistribute the 
Apple Software, with or without modifications, in 
source and/or binary forms; provided that if you 
redistribute the Apple Software in its entirety 
and without modifications, you must retain this 
notice and the following text and disclaimers in 
all such redistributions of the Apple Software. 
Neither the name, trademarks, service marks or 
logos of Apple Computer, Inc. may be used to 
endorse or promote products derived from the 
Apple Software without specific prior written 
permission from Apple.  Except as expressly 
stated in this notice, no other rights or 
licenses, express or implied, are granted by 
Apple herein, including but not limited to any 
patent rights that may be infringed by your 
derivative works or by other works in which the 
Apple Software may be incorporated.

The Apple Software is provided by Apple on an "AS 
IS" basis.  APPLE MAKES NO WARRANTIES, EXPRESS OR 
IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED 
WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY 
AND FITNESS FOR A PARTICULAR PURPOSE, REGARDING 
THE APPLE SOFTWARE OR ITS USE AND OPERATION ALONE 
OR IN COMBINATION WITH YOUR PRODUCTS.

IN NO EVENT SHALL APPLE BE LIABLE FOR ANY 
SPECIAL, INDIRECT, INCIDENTAL OR CONSEQUENTIAL 
DAMAGES (INCLUDING, BUT NOT LIMITED TO, 
PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS 
OF USE, DATA, OR PROFITS; OR BUSINESS 
INTERRUPTION) ARISING IN ANY WAY OUT OF THE USE, 
REPRODUCTION, MODIFICATION AND/OR DISTRIBUTION OF 
THE APPLE SOFTWARE, HOWEVER CAUSED AND WHETHER 
UNDER THEORY OF CONTRACT, TORT (INCLUDING 
NEGLIGENCE), STRICT LIABILITY OR OTHERWISE, EVEN 
IF APPLE HAS BEEN ADVISED OF THE POSSIBILITY OF 
SUCH DAMAGE.

*/

#include <QuickTime/QuickTime.h>


#define	ASK_SEQUENCE_GRABBER_TO_CONVERT_AUDIO_TO_AAC		0


// defines
#define BailErr(x) {err = x; if(err != noErr) goto bail;}
#define DisplayAndBail(x, y) {pMungData->err = x; if(x != noErr) { displayError(pMungData->pWindow, y, x); goto bail;}}

// constants
const EventTime kTimerInterval = kEventDurationSecond / 60;	// idle timer interval

// mung data struct
typedef struct {
    WindowRef					pWindow;	// window
	int							width;		// dest width
	int							height;		// dest height
	CodecType					codecType;	// codec
	SInt32						averageDataRate; // data rate
	HIViewRef					imageView;	// for displaying decoded frames
	ICMDecompressionSessionRef	decompressionSession; // decompresses and scales captured frames
	ICMCompressionSessionRef	compressionSession; // compresses frames
	Movie						outputMovie; // movie file for storing compressed frames
	Media						outputVideoMedia; // media for our video track in the movie
	DataHandler					outputMovieDataHandler; // storage for movie header
	CFURLRef					outputMovieURL;	// URL to the movie file (once we create it)
	Boolean						didBeginVideoMediaEdits;
	Boolean						verbose;
    SeqGrabComponent			seqGrab;	// sequence grabber
    TimeValue					lastTime;
    TimeScale					timeScale;
	int							desiredFramesPerSecond;
	TimeValue					minimumFrameDuration;
    long						frameCount;
    Boolean						isGrabbing;
    EventLoopTimerRef			timerRef;
    OSErr						err;
	
	SGChannel					audioChan;
	Media						soundMedia;
	Boolean						didBeginSoundMediaEdits;
    TimeScale                   audioTimeScale;
    SoundDescriptionHandle      audioDescH;
    AudioStreamBasicDescription asbd;
} MungDataRecord, *MungDataPtr;

static MungDataPtr initializeMungData(WindowRef inWindow, Rect inBounds, SeqGrabComponent inSeqGrab);
static void disposeMungData(MungDataPtr pMungData);

static OSStatus setUpOutputMovie( MungDataPtr pMungData );
static void finishOutputMovie( MungDataPtr pMungData );
static void openOutputMovie( MungDataPtr pMungData );

// --------------------
// initializeMungData
//
static MungDataPtr initializeMungData(WindowRef inWindow, Rect inBounds, SeqGrabComponent inSeqGrab)
{
	MungDataPtr pMungData = NULL;
	HIViewRef contentView = NULL;
	HIViewID kImageID = { 'IMAG', 0 };
    
    OSErr err = noErr;
    
    // allocate memory for the data
    pMungData = (MungDataPtr)calloc(sizeof(MungDataRecord), 1);
    if (MemError() || NULL == pMungData ) return NULL;
    
	pMungData->pWindow = inWindow;
	pMungData->width = inBounds.right - inBounds.left;
	pMungData->height = inBounds.bottom - inBounds.top;
	pMungData->codecType = kH264CodecType;
	pMungData->averageDataRate = 100000; // 100 kbyte/sec == 800 kbit/sec
	pMungData->desiredFramesPerSecond = 15;
	pMungData->seqGrab = inSeqGrab;
	pMungData->isGrabbing = false;
	
	pMungData->verbose = false;
	
	GetRootControl( pMungData->pWindow, &contentView );
	HIViewFindByID( contentView, kImageID, &pMungData->imageView );
	
	err = setUpOutputMovie( pMungData );
	if( err )
		goto bail;
	
	return pMungData;
bail:
	// clean up and get out
	if (pMungData) {
		disposeMungData(pMungData);
	}
	
	return NULL;
}

// --------------------
// disposeMungData
//
static void disposeMungData(MungDataPtr pMungData)
{
    // clean up the bits
    if(pMungData) {
        if (pMungData->seqGrab)
    		CloseComponent(pMungData->seqGrab);
		ICMDecompressionSessionRelease( pMungData->decompressionSession );
		
		// It is important to push out any remaining frames before we release the compression session.
		// If we knew the timestamp following the last source frame, you should pass it in here.
		ICMCompressionSessionCompleteFrames( pMungData->compressionSession, true, 0, 0 );
		ICMCompressionSessionRelease( pMungData->compressionSession );
		
		if( pMungData->outputMovie ) {
			finishOutputMovie( pMungData );
			openOutputMovie( pMungData );
		}
		
		if( pMungData->outputMovieURL )
			CFRelease( pMungData->outputMovieURL );
		
        DisposeHandle((Handle)pMungData->audioDescH);
        
        free((Ptr)pMungData);
        pMungData = NULL;
    }
}

static OSStatus promptForOutputMovieDataRef( Handle *dataRefOut, OSType *dataRefTypeOut )
{
	OSStatus err = noErr;
	NavDialogCreationOptions navOptions = { kNavDialogCreationOptionsVersion, 0, };
	NavDialogRef navDialog = NULL;
	NavReplyRecord navReply = { kNavReplyRecordVersion, };
	AEDesc actualDesc = { 0, 0 };
	FSRef parentFSRef;
	
	err = NavGetDefaultDialogCreationOptions( &navOptions );
	if( err )
		goto bail;
	navOptions.windowTitle = CFSTR("Save Captured Movie As...");
	navOptions.message = CFSTR("Pick where to save the captured and compressed movie.  (Make sure your camera is plugged in!)");
	navOptions.saveFileName = CFSTR("captured.mov");
	navOptions.modality = kWindowModalityAppModal;
	
	err = NavCreatePutFileDialog( &navOptions, MovieFileType, 'TVOD', NULL, NULL, &navDialog );
	if( err )
		goto bail;
	
	err = NavDialogRun( navDialog );
	if( err )
		goto bail;
	
	if( kNavUserActionSaveAs != NavDialogGetUserAction( navDialog ) ) {
		err = userCanceledErr;
		goto bail;
	}
	
	err = NavDialogGetReply( navDialog, &navReply );
	if( err ) 
		goto bail;
	
	err = AECoerceDesc( &navReply.selection, typeFSRef, &actualDesc );
	if( err )
		goto bail;

	err = AEGetDescData( &actualDesc, &parentFSRef, sizeof( FSRef ) );
	if( err )
		goto bail;
	
	err = QTNewDataReferenceFromFSRefCFString( &parentFSRef, navReply.saveFileName, 0, dataRefOut, dataRefTypeOut );
	if( err ) {
		fprintf( stderr, "QTNewDataReferenceFromFSRefCFString() failed (%d)\n", err );
		goto bail;
	}
	
bail:
	NavDisposeReply( &navReply );
	NavDialogDispose( navDialog );
	AEDisposeDesc( &actualDesc );
	return err;
}

static OSStatus setUpOutputMovie( MungDataPtr pMungData )
{
	OSStatus err = noErr;
	Handle outputMovieDataRef = NULL;
	OSType outputMovieDataRefType = 0;
	CFStringRef outputMovieFullPathString = NULL;

	// Prompt the user for an output file.
	err = promptForOutputMovieDataRef( &outputMovieDataRef, &outputMovieDataRefType );
	if( err )
		goto bail;
	
	// Create a new movie file. 
	// If you're using CreateMovieFile, consider switching to CreateMovieStorage, which is long-file-name aware.
	err = CreateMovieStorage( outputMovieDataRef, outputMovieDataRefType, 'TVOD', 0, 
			createMovieFileDeleteCurFile, &pMungData->outputMovieDataHandler, &pMungData->outputMovie );
	if( err ) {
		fprintf( stderr, "CreateMovieStorage failed (%d)\n", err );
		goto bail;
	}
	
	// Remember a URL for the movie file so we can open it in QuickTime Player afterwards.
	err = QTGetDataReferenceFullPathCFString( outputMovieDataRef, outputMovieDataRefType, 
			kQTPOSIXPathStyle, &outputMovieFullPathString );
	if( err )
		goto bail;
	
	pMungData->outputMovieURL = CFURLCreateWithFileSystemPath( NULL, outputMovieFullPathString, kCFURLPOSIXPathStyle, false );
	
bail:
	DisposeHandle( outputMovieDataRef );
	if( outputMovieFullPathString )
		CFRelease( outputMovieFullPathString );
 	return err;
}

static void finishOutputMovie(MungDataPtr pMungData)
{
	OSStatus err = noErr;
	Track videoTrack = NULL;
	
	if( pMungData->didBeginVideoMediaEdits ) {
		// End the media sample-adding session.
		err = EndMediaEdits( pMungData->outputVideoMedia );
		if( err ) {
			fprintf( stderr, "EndMediaEdits() failed (%d)\n", err );
			goto bail;
		}
	}
	
	
	// Make sure things are extra neat.
	ExtendMediaDecodeDurationToDisplayEndTime( pMungData->outputVideoMedia, NULL );
	
	// Insert the stuff we added into the track, at the end.
	videoTrack = GetMediaTrack( pMungData->outputVideoMedia );
	err = InsertMediaIntoTrack( videoTrack, 
			GetTrackDuration(videoTrack), 
			0, GetMediaDisplayDuration( pMungData->outputVideoMedia ), // NOTE: use this instead of GetMediaDuration
			fixed1 );
	if( err ) {
		fprintf( stderr, "InsertMediaIntoTrack() failed (%d)\n", err );
		goto bail;
	}
	
	
	
	if ( pMungData->didBeginSoundMediaEdits )
	{
		err = EndMediaEdits( pMungData->soundMedia );
		if( err ) {
			fprintf( stderr, "EndMediaEdits(soundMedia) failed (%d)\n", err );
			goto bail;
		}
	}
	
    {
        Track soundTrack = GetMediaTrack( pMungData->soundMedia );
        TimeValue soundTrackDur, videoTrackDur;
        err = InsertMediaIntoTrack( soundTrack, 0, 0, GetMediaDuration( pMungData->soundMedia ), fixed1 );
        if ( err ) {
            fprintf( stderr, "InsertMediaIntoTrack(soundTrack) failed (%d)\n", err );
            goto bail;
        }
        
        // trim the sound track to the duration of the video track 
        // (no one likes white frames at the end of their movies)
        soundTrackDur = GetTrackDuration(soundTrack);
	    videoTrackDur = GetTrackDuration(videoTrack);
        if (soundTrackDur > videoTrackDur)
        {
			fprintf( stderr, "sound track ran long, soundTrackDur = %ld, videoTrackDur = %ld\n", soundTrackDur, videoTrackDur );
            DeleteTrackSegment(soundTrack, videoTrackDur, soundTrackDur - videoTrackDur);
        }
	}
	
	// Write the movie header to the file.
	err = AddMovieToStorage( pMungData->outputMovie, pMungData->outputMovieDataHandler );
	if( err ) {
		fprintf( stderr, "AddMovieToStorage() failed (%d)\n", err );
		goto bail;
	}
	
	CloseMovieStorage( pMungData->outputMovieDataHandler );
	pMungData->outputMovieDataHandler = 0;
	
	DisposeMovie( pMungData->outputMovie );
	
bail:
	return;
}

static void openOutputMovie( MungDataPtr pMungData )
{
	OSStatus err = noErr;
	
	// Open the movie we've created in QuickTime Player.
	err = LSOpenCFURLRef( pMungData->outputMovieURL, NULL );
	
bail:
	return;
}

// --------------------
// makeSequenceGrabber
//
static SeqGrabComponent makeSequenceGrabber(WindowRef pWindow)
{
	SeqGrabComponent seqGrab = NULL;
	OSErr			 err = noErr;

    // open the default sequence grabber
    seqGrab = OpenDefaultComponent(SeqGrabComponentType, 0);
    if (seqGrab != NULL) { 
    	// initialize the default sequence grabber component
    	err = SGInitialize(seqGrab);

    	if (err == noErr)
        	// set its graphics world to the specified window
        	err = SGSetGWorld(seqGrab, GetWindowPort(pWindow), NULL);
    	
    	if (err == noErr)
    		// specify the destination data reference for a record operation
    		// tell it we're not making a movie
    		// if the flag seqGrabDontMakeMovie is used, the sequence grabber still calls
    		// your data function, but does not write any data to the movie file
    		// writeType will always be set to seqGrabWriteAppend
    		err = SGSetDataRef(seqGrab,
    						   0,
    						   0,
    						   seqGrabDontMakeMovie);
    }

    if (err && (seqGrab != NULL)) { // clean up on failure
    	CloseComponent(seqGrab);
        seqGrab = NULL;
    }
    
	return seqGrab;
}

// --------------------
// makeSequenceGrabVideoChannel
//
static OSErr makeSequenceGrabVideoChannel(SeqGrabComponent seqGrab, SGChannel *sgchanVideo, Rect const *rect)
{
    long  flags = 0;
    
    OSErr err = noErr;
    
    err = SGNewChannel(seqGrab, VideoMediaType, sgchanVideo);
    if (err == noErr) {
	    err = SGSetChannelBounds(*sgchanVideo, rect);
	    if (err == noErr)
	    	// set usage for new video channel to avoid playthrough
	   		// note we don't set seqGrabPlayDuringRecord
	    	err = SGSetChannelUsage(*sgchanVideo, flags | seqGrabRecord);
	    
	    if (err != noErr) {
	        // clean up on failure
	        SGDisposeChannel(seqGrab, *sgchanVideo);
	        *sgchanVideo = NULL;
	    }
    }

	return err;
}



// --------------------
// makeSequenceGrabAudioChannel
//
static OSErr makeSequenceGrabAudioChannel(SeqGrabComponent seqGrab, SGChannel *sgchanAudio)
{
    OSErr err = noErr;
    
    err = SGNewChannel(seqGrab, SGAudioMediaType, sgchanAudio);
    if (err == noErr) {
		// set usage for new audio channel.
		// We will use the SGAudioChannel's built in mechanism for previewing
		// by adding the seqGrabPlayDuringRecord flag
		err = SGSetChannelUsage(*sgchanAudio, seqGrabRecord | seqGrabPlayDuringRecord);
		
		if (err == noErr)
		{
			// By default, the device from which the SGAudioChannel captures is
			// the audio portion of your selected video device.  If your video device
			// has no audio recording capabilities, SGAudioChannel will capture from
			// the default device specified in your system-wide audio settings, found
			// in /Applications/Utilities/Audio Midi Setup.app
			
			// If you are recording live from a microphone (i.e. the built in microphone
			// on an iSight camera), chances are good that your microphone is
			// situated in close proximity to the speakers through which you will
			// be previewing the audio -- which may introduce a feedback loop.
			// To prevent this, we can set the master gain of the preview device
			// to be very low (NOTE - the gain is adjusted in software.  The physical
			// volume of your playback device will not be altered).
			Float32 lowVolume = 0.05;  // 1.0 is full, 0.0 is muted
			err = QTSetComponentProperty(*sgchanAudio, kQTPropertyClass_SGAudioPreviewDevice,
										 kQTSGAudioPropertyID_MasterGain,
										 sizeof(lowVolume), &lowVolume);
		}

#if ASK_SEQUENCE_GRABBER_TO_CONVERT_AUDIO_TO_AAC		
		if (err == noErr)
		{
			AudioStreamBasicDescription outputFormat;
			
			// get the current output format, so we carry over the source
			// sample rate.  we'll mix down to stereo if there are > 2 source
			// channels.
			err = QTGetComponentProperty(*sgchanAudio, kQTPropertyClass_SGAudio,
										kQTSGAudioPropertyID_StreamFormat,
										sizeof(outputFormat), &outputFormat, NULL);
			if (err == noErr)
			{
				outputFormat.mFormatID = 'aac ';
				if (outputFormat.mChannelsPerFrame > 2)
					outputFormat.mChannelsPerFrame = 2;
					
				outputFormat.mBitsPerChannel = 0;
				outputFormat.mBytesPerFrame = 0;
				outputFormat.mBytesPerPacket = 0;
				outputFormat.mFormatFlags = 0;
				outputFormat.mFramesPerPacket = 0; // Sequence Grabber will fill in the fields we don't know.

				err = QTSetComponentProperty(*sgchanAudio, kQTPropertyClass_SGAudio,
											kQTSGAudioPropertyID_StreamFormat,
											sizeof(outputFormat), &outputFormat);
			}
		}
#endif
		
		// See QuickTimeComponents.h for a full list of the SGAudioChannel's configurable 
		// properties.  Search for SGAudioMediaType
	    
	    if (err != noErr) {
	        // clean up on failure
	        SGDisposeChannel(seqGrab, *sgchanAudio);
	        *sgchanAudio = NULL;
	    }
    }

	return err;
}




static void displayError(WindowRef inWindow, char inStr[], OSErr inError)
{
	// set the window title to display the error
	char errMsg[64];
	
	sprintf(errMsg, "%s: %d", inStr, inError);
	CopyCStringToPascal(errMsg, (unsigned char *)&errMsg);
	SetWTitle(inWindow, (unsigned char *)errMsg);
}

static OSStatus createDecompressionSession( 
		ImageDescriptionHandle imageDesc, 
		int width, 
		int height, 
		OSType pixelFormat, 
		ICMDecompressionTrackingCallback trackingCallback,
		void *trackingRefCon,
		ICMDecompressionSessionRef *decompressionSessionOut )
{
	OSStatus err = noErr;
	CFNumberRef number = NULL;
	CFMutableDictionaryRef pixelBufferAttributes = NULL;
	ICMDecompressionSessionOptionsRef sessionOptions = NULL;
	ICMDecompressionTrackingCallbackRecord trackingCallbackRecord;
	
	// Make a CFDictionary that describes the pixel buffers we want the decompression session to output.  
	// We want them to be the right dimensions and pixel format; we want them to be compatible with 
	// CGBitmapContext and CGImage.  
	
	// Note: if we didn't need to draw on frames with CG or display the frames in an HIImageView, we 
	// could improve performance by getting the compression session's preferred pixel buffer attributes
	// via the kICMCompressionSessionPropertyID_CompressorPixelBufferAttributes property
	// and passing that when creating the decompression session.
	pixelBufferAttributes = CFDictionaryCreateMutable( NULL, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks );
	
	number = CFNumberCreate( NULL, kCFNumberIntType, &width );
	CFDictionaryAddValue( pixelBufferAttributes, kCVPixelBufferWidthKey, number );
	CFRelease( number );
	
	number = CFNumberCreate( NULL, kCFNumberIntType, &height );
	CFDictionaryAddValue( pixelBufferAttributes, kCVPixelBufferHeightKey, number );
	CFRelease( number );
	
	number = CFNumberCreate( NULL, kCFNumberSInt32Type, &pixelFormat );
	CFDictionaryAddValue( pixelBufferAttributes, kCVPixelBufferPixelFormatTypeKey, number );
	CFRelease( number );
	
	CFDictionaryAddValue( pixelBufferAttributes, kCVPixelBufferCGBitmapContextCompatibilityKey, kCFBooleanTrue );
	CFDictionaryAddValue( pixelBufferAttributes, kCVPixelBufferCGImageCompatibilityKey, kCFBooleanTrue );
	
	trackingCallbackRecord.decompressionTrackingCallback = trackingCallback;
	trackingCallbackRecord.decompressionTrackingRefCon = trackingRefCon;
	
	err = ICMDecompressionSessionCreate( NULL, imageDesc, sessionOptions, 
										 pixelBufferAttributes, &trackingCallbackRecord, decompressionSessionOut );
	if( err ) {
		fprintf( stderr, "ICMDecompressionSessionCreate failed (%d)", err );
	}
	
	CFRelease( pixelBufferAttributes );
	ICMDecompressionSessionOptionsRelease( sessionOptions );
	
	return err;
}

static OSStatus
createCompressionSession( 
		int width, int height, CodecType codecType, SInt32 averageDataRate, TimeScale timeScale, 
		ICMEncodedFrameOutputCallback outputCallback, void *outputRefCon,
		ICMCompressionSessionRef *compressionSessionOut )
{
	OSStatus err = noErr;
	ICMEncodedFrameOutputRecord encodedFrameOutputRecord = {0};
	ICMCompressionSessionOptionsRef sessionOptions = NULL;
	
	err = ICMCompressionSessionOptionsCreate( NULL, &sessionOptions );
	if( err ) {
		fprintf( stderr, "ICMCompressionSessionOptionsCreate() failed (%d)\n", err );
		goto bail;
	}
	
	// We must set this flag to enable P or B frames.
	err = ICMCompressionSessionOptionsSetAllowTemporalCompression( sessionOptions, true );
	if( err ) {
		fprintf( stderr, "ICMCompressionSessionOptionsSetAllowTemporalCompression() failed (%d)\n", err );
		goto bail;
	}
	
	// We must set this flag to enable B frames.
	err = ICMCompressionSessionOptionsSetAllowFrameReordering( sessionOptions, true );
	if( err ) {
		fprintf( stderr, "ICMCompressionSessionOptionsSetAllowFrameReordering() failed (%d)\n", err );
		goto bail;
	}
	
	// Set the maximum key frame interval, also known as the key frame rate.
	err = ICMCompressionSessionOptionsSetMaxKeyFrameInterval( sessionOptions, 30 );
	if( err ) {
		fprintf( stderr, "ICMCompressionSessionOptionsSetMaxKeyFrameInterval() failed (%d)\n", err );
		goto bail;
	}

	// This allows the compressor more flexibility (ie, dropping and coalescing frames).
	err = ICMCompressionSessionOptionsSetAllowFrameTimeChanges( sessionOptions, true );
	if( err ) {
		fprintf( stderr, "ICMCompressionSessionOptionsSetAllowFrameTimeChanges() failed (%d)\n", err );
		goto bail;
	}
	
	// We need durations when we store frames.
	err = ICMCompressionSessionOptionsSetDurationsNeeded( sessionOptions, true );
	if( err ) {
		fprintf( stderr, "ICMCompressionSessionOptionsSetDurationsNeeded() failed (%d)\n", err );
		goto bail;
	}
	
	// Set the average data rate.
	err = ICMCompressionSessionOptionsSetProperty( sessionOptions, 
				kQTPropertyClass_ICMCompressionSessionOptions,
				kICMCompressionSessionOptionsPropertyID_AverageDataRate,
				sizeof( averageDataRate ),
				&averageDataRate );
	if( err ) {
		fprintf( stderr, "ICMCompressionSessionOptionsSetProperty(AverageDataRate) failed (%d)\n", err );
		goto bail;
	}
	
	encodedFrameOutputRecord.encodedFrameOutputCallback = outputCallback;
	encodedFrameOutputRecord.encodedFrameOutputRefCon = outputRefCon;
	encodedFrameOutputRecord.frameDataAllocator = NULL;
	
	err = ICMCompressionSessionCreate( NULL, width, height, codecType, timeScale,
			sessionOptions, NULL, &encodedFrameOutputRecord, compressionSessionOut );
	if( err ) {
		fprintf( stderr, "ICMCompressionSessionCreate() failed (%d)\n", err );
		goto bail;
	}
	
bail:
	ICMCompressionSessionOptionsRelease( sessionOptions );
	
	return err;
}

// Utility to wrap a CGBitmapContext around a 32ARGB CVPixelBuffer.
static OSStatus
createCGBitmapContextFor32ARGBCVPixelBuffer(
		CVPixelBufferRef pixelBuffer,
		CGContextRef *contextOut )
{
	size_t width, height, rowBytes;
	void *baseAddr;
	OSStatus err;
	CGColorSpaceRef colorspace = NULL;
	CGContextRef context = NULL;
	
	err = CVPixelBufferLockBaseAddress( pixelBuffer, 0 );
	if( err ) {
		fprintf( stderr, "CVPixelBufferLockBaseAddress() failed (%d)\n", err );
		goto bail;
	}
	rowBytes = CVPixelBufferGetBytesPerRow( pixelBuffer );
	baseAddr = CVPixelBufferGetBaseAddress( pixelBuffer );
	width = CVPixelBufferGetWidth( pixelBuffer );
	height = CVPixelBufferGetHeight( pixelBuffer );
	
	colorspace = CGColorSpaceCreateDeviceRGB();
	
	context = CGBitmapContextCreate( baseAddr, width, height, 
			8, rowBytes, colorspace, kCGImageAlphaPremultipliedFirst );
	
	*contextOut = context;
	context = NULL;
	
bail:
	if( colorspace ) CGColorSpaceRelease( colorspace );
	if( context ) CGContextRelease( context );
	return err;
}

static void
unlockCVPixelBufferAndReleaseCGBitmapContext( 
		CVPixelBufferRef pixelBuffer,
		CGContextRef context )
{
	CGContextRelease( context );
	CVPixelBufferUnlockBaseAddress( pixelBuffer, 0 );
}

// Utility to draw a string left- or right- justified with a stroked border in one color, filled with another color.
enum Justify {
	kLeftJustify, kRightJustify
};
static void showString( CGContextRef c, float x, float y, enum Justify justify, const char *str)
{
	CGPoint after;
	CGContextSaveGState( c );
	if( kRightJustify == justify ) {
		CGContextSetTextPosition( c, 0, 0 );
		CGContextSetTextDrawingMode( c, kCGTextInvisible );
		CGContextShowText( c, str, strlen( str ) );
		after = CGContextGetTextPosition( c );
		x = x - after.x;
		y = y - after.y;
	}
	CGContextSetTextPosition( c, x, y );
	CGContextSetTextDrawingMode( c, kCGTextFillStroke );
	CGContextShowText( c, str, strlen( str ) );
	CGContextRestoreGState( c );
}

// Draw "LIVE" and the number of seconds into the capture on the pixel buffer.
static void drawOnPixelBuffer( CVPixelBufferRef pixelBuffer, TimeValue64 displayTime, TimeScale timeScale )
{
	CGContextRef context = NULL;
	if( noErr == createCGBitmapContextFor32ARGBCVPixelBuffer( pixelBuffer, &context ) ) {
		int width = CVPixelBufferGetWidth( pixelBuffer );
		int timeInSeconds = displayTime / timeScale;
		char buf[100];
		
		sprintf( buf, "%d:%02d", timeInSeconds / 60, timeInSeconds % 60 );
		
		CGContextSelectFont( context, "Arial-Black", 32, kCGEncodingMacRoman );
		CGContextSetLineWidth( context, 1.5 );
		CGContextSetRGBStrokeColor( context, 1.0, 1.0, 1.0, 1.0 ); // white
		CGContextSetRGBFillColor( context, 0.2, 0.0, 0.0, 1.0 ); // dark red
		showString( context, 10, 10, kLeftJustify, "LIVE" );
		CGContextSetRGBFillColor( context, 0.0, 0.0, 0.3, 1.0 ); // dark blue
		showString( context, width-10, 10, kRightJustify, buf );
		
		unlockCVPixelBufferAndReleaseCGBitmapContext( pixelBuffer, context );
	}
}

// Utility to wrap a CGImage around a 32ARGB CVPixelBuffer.

static void releaseAndUnlockThis( void *info, const void *data, size_t size )
{
	CVPixelBufferRef pixelBuffer = info;
	
	CVPixelBufferUnlockBaseAddress( pixelBuffer, 0 );
	CVBufferRelease( pixelBuffer );
}

static CGImageRef createCGImageFrom32XRGBCVPixelBuffer( CVPixelBufferRef pixelBuffer )
{
	size_t width, height, rowBytes;
	void *baseAddr = NULL;
	CGColorSpaceRef colorspace = NULL;
	CGDataProviderRef provider = NULL;
	CGImageRef image = NULL;
	
	CVPixelBufferLockBaseAddress( pixelBuffer, 0 );
	
	rowBytes = CVPixelBufferGetBytesPerRow( pixelBuffer );
	baseAddr = CVPixelBufferGetBaseAddress( pixelBuffer );
	width = CVPixelBufferGetWidth( pixelBuffer );
	height = CVPixelBufferGetHeight( pixelBuffer );
	
	colorspace = CGColorSpaceCreateDeviceRGB();
	CVBufferRetain( pixelBuffer );
	provider = CGDataProviderCreateWithData( pixelBuffer, baseAddr, rowBytes * height, releaseAndUnlockThis );
	image = CGImageCreate( width, height, 8, 32, rowBytes, colorspace, 
						   kCGImageAlphaNoneSkipFirst, provider, NULL, true, kCGRenderingIntentDefault );
	
bail:
	if( provider ) CGDataProviderRelease( provider );
	if( colorspace ) CGColorSpaceRelease( colorspace );
	return image;
}

// Replace the image in an HIImageView and force it to be displayed immediately.
static void putCGImageInHIImageView( CGImageRef image, HIViewRef imageView )
{
	OSStatus err;
	if( imageView ) {
		// Pass our view the new image
		err = HIImageViewSetImage( imageView, image );
		err = HIViewSetNeedsDisplay( imageView, true );
		err = HIViewRender( imageView );
		err = HIWindowFlush( HIViewGetWindow( imageView ) );
	}
}

// The tracking callback function for the decompression session.
// Draw on pixel buffers, display them in the window and feed them to the compression session.
static void 
displayAndCompressFrame( 
		void *decompressionTrackingRefCon,
		OSStatus result,
		ICMDecompressionTrackingFlags decompressionTrackingFlags,
		CVPixelBufferRef pixelBuffer,
		TimeValue64 displayTime,
		TimeValue64 displayDuration,
		ICMValidTimeFlags validTimeFlags,
		void *reserved,
		void *sourceFrameRefCon )
{
	OSStatus err = noErr;
	MungDataPtr pMungData = (MungDataPtr)decompressionTrackingRefCon;
	
	if( kICMDecompressionTracking_ReleaseSourceData & decompressionTrackingFlags ) {
		// if we were responsible for managing source data buffers, we should release the source buffer here,
		// using sourceFrameRefCon to identify it.
	}
	if( ( kICMDecompressionTracking_EmittingFrame & decompressionTrackingFlags ) && pixelBuffer ) {
		ICMCompressionFrameOptionsRef frameOptions = NULL;
		CGImageRef image = NULL;
		
		// Draw in the corner of the pixelBuffer.
		drawOnPixelBuffer( pixelBuffer, displayTime, pMungData->timeScale );
		
		// Display the frame in the imageView.
		image = createCGImageFrom32XRGBCVPixelBuffer( pixelBuffer );
		putCGImageInHIImageView( image, pMungData->imageView );
		if( image ) CFRelease( image );
		
		// Feed the frame to the compression session.
		err = ICMCompressionSessionEncodeFrame( pMungData->compressionSession, pixelBuffer,
				displayTime, displayDuration, validTimeFlags,
				frameOptions, NULL, NULL );
		if( err ) {
			fprintf( stderr, "ICMCompressionSessionEncodeFrame() failed (%d)\n", err );
		}
	}
}

// Create a video track and media to hold encoded frames.
// This is called the first time we get an encoded frame back from the compression session.
static OSStatus
createVideoMedia( 
		MungDataPtr pMungData,
		ImageDescriptionHandle imageDesc,
		TimeScale timescale )
{
	OSStatus err = noErr;
	Fixed trackWidth, trackHeight;
	Track outputTrack = NULL;
	
	err = ICMImageDescriptionGetProperty( 
			imageDesc,
			kQTPropertyClass_ImageDescription, 
			kICMImageDescriptionPropertyID_ClassicTrackWidth,
			sizeof( trackWidth ),
			&trackWidth,
			NULL );
	if( err ) {
		fprintf( stderr, "ICMImageDescriptionGetProperty(kICMImageDescriptionPropertyID_DisplayWidth) failed (%d)\n", err );
		goto bail;
	}
	
	err = ICMImageDescriptionGetProperty( 
			imageDesc,
			kQTPropertyClass_ImageDescription, 
			kICMImageDescriptionPropertyID_ClassicTrackHeight,
			sizeof( trackHeight ),
			&trackHeight,
			NULL );
	if( err ) {
		fprintf( stderr, "ICMImageDescriptionGetProperty(kICMImageDescriptionPropertyID_DisplayHeight) failed (%d)\n", err );
		goto bail;
	}
	
	if( pMungData->verbose ) {
		printf( "creating %g x %g track\n", Fix2X(trackWidth), Fix2X(trackHeight) );
	}
	
	outputTrack = NewMovieTrack( pMungData->outputMovie, trackWidth, trackHeight, 0 );
	err = GetMoviesError();
	if( err ) {
		fprintf( stderr, "NewMovieTrack() failed (%d)\n", err );
		goto bail;
	}
	
	pMungData->outputVideoMedia = NewTrackMedia( outputTrack, VideoMediaType, timescale, 0, 0 );
	err = GetMoviesError();
	if( err ) {
		fprintf( stderr, "NewTrackMedia() failed (%d)\n", err );
		goto bail;
	}
	
	err = BeginMediaEdits( pMungData->outputVideoMedia );
	if( err ) {
		fprintf( stderr, "BeginMediaEdits() failed (%d)\n", err );
		goto bail;
	}
	pMungData->didBeginVideoMediaEdits = true;
	
bail:
	return err;
}

// This is the tracking callback function for the compression session.
// Write the encoded frame to the movie file.
// Note that this function adds each sample separately; better chunking can be achieved
// by flattening the movie after it is finished, or by grouping samples, writing them in 
// groups to the data reference manually, and using AddSampleTableToMedia.
static OSStatus 
writeEncodedFrameToMovie( void *encodedFrameOutputRefCon, 
				   ICMCompressionSessionRef session, 
				   OSStatus err,
				   ICMEncodedFrameRef encodedFrame,
				   void *reserved )
{
	MungDataPtr pMungData = encodedFrameOutputRefCon;
	ImageDescriptionHandle imageDesc = NULL;
	TimeValue64 decodeDuration;
	
	if( err ) {
		fprintf( stderr, "writeEncodedFrameToMovie received an error (%d)\n", err );
		goto bail;
	}
	
	err = ICMEncodedFrameGetImageDescription( encodedFrame, &imageDesc );
	if( err ) {
		fprintf( stderr, "ICMEncodedFrameGetImageDescription() failed (%d)\n", err );
		goto bail;
	}
	
	if( ! pMungData->outputVideoMedia ) {
		err = createVideoMedia( pMungData, imageDesc, ICMEncodedFrameGetTimeScale( encodedFrame ) );
		if( err ) 
			goto bail;
	}
	
	decodeDuration = ICMEncodedFrameGetDecodeDuration( encodedFrame );
	if( decodeDuration == 0 ) {
		// You can't add zero-duration samples to a media.  If you try you'll just get invalidDuration back.
		// Because we don't tell the ICM what the source frame durations are,
		// the ICM calculates frame durations using the gaps between timestamps.
		// It can't do that for the final frame because it doesn't know the "next timestamp"
		// (because in this example we don't pass a "final timestamp" to ICMCompressionSessionCompleteFrames).
		// So we'll give the final frame our minimum frame duration.
		decodeDuration = pMungData->minimumFrameDuration * ICMEncodedFrameGetTimeScale( encodedFrame ) / pMungData->timeScale;
	}
	
	if( pMungData->verbose ) {
		printf( "adding %ld byte sample: decode duration %ld, display offset %ld, flags %#lx", 
				(long)ICMEncodedFrameGetDataSize( encodedFrame ),
				(long)decodeDuration, 
				(long)ICMEncodedFrameGetDisplayOffset( encodedFrame ),
				(long)ICMEncodedFrameGetMediaSampleFlags( encodedFrame ) );
		if( true ) {
			ICMValidTimeFlags validTimeFlags = ICMEncodedFrameGetValidTimeFlags( encodedFrame );
			if( kICMValidTime_DecodeTimeStampIsValid & validTimeFlags )
				printf( ", decode time stamp %ld", (long)ICMEncodedFrameGetDecodeTimeStamp( encodedFrame ) );
			if( kICMValidTime_DisplayTimeStampIsValid & validTimeFlags )
				printf( ", display time stamp %ld", (long)ICMEncodedFrameGetDisplayTimeStamp( encodedFrame ) );
		}
		printf( "\n" );
	}
	
	err = AddMediaSample2(
		pMungData->outputVideoMedia,
		ICMEncodedFrameGetDataPtr( encodedFrame ),
		ICMEncodedFrameGetDataSize( encodedFrame ),
		decodeDuration,
		ICMEncodedFrameGetDisplayOffset( encodedFrame ),
		(SampleDescriptionHandle)imageDesc,
		1,
		ICMEncodedFrameGetMediaSampleFlags( encodedFrame ),
		NULL );
	if( err ) {
		fprintf( stderr, "AddMediaSample2() failed (%d)\n", err );
		goto bail;
	}
	
	// Note: if you don't need to intercept any values, you could equivalently call:
	// err = AddMediaSampleFromEncodedFrame( pMungData->outputVideoMedia, encodedFrame, NULL );
	// if( err ) {
	//     fprintf( stderr, "AddMediaSampleFromEncodedFrame() failed (%d)\n", err );
	//     goto bail;
	// }

	
bail:
	return err;
}

static OSStatus
writeAudioToMovie(MungDataPtr pMungData, UInt8 *p, long len, long *offset, TimeValue time, long chRefCon)
{
    OSStatus err = noErr;
    UInt32 numSamples = 0;
    
	if (!pMungData)
		return -1;
		
	if (pMungData->soundMedia == NULL)
	{
        Track t;
        // Get the timescale
        err = SGGetChannelTimeScale(pMungData->audioChan, &pMungData->audioTimeScale);
        if ( err ) {
            fprintf( stderr, "SGGetChannelTimeScale(audioChan) failed (%d)\n", err );
            goto bail;
        }
        
		// create the sound track
        t = NewMovieTrack(pMungData->outputMovie, 0, 0, kFullVolume);
        err = GetMoviesError();
        if ( err ) {
            fprintf( stderr, "NewMovieTrack(SoundMediaType) failed (%d)\n", err );
            goto bail;
        }
        
        pMungData->soundMedia = NewTrackMedia(t, SoundMediaType, pMungData->audioTimeScale, NULL, 0);
        err = GetMoviesError();
        if ( err ) {
            fprintf(stderr, "NewTrackMedia(SoundMediaType) failed (%d)\n", err );
            goto bail;
        }
        
        err = BeginMediaEdits( pMungData->soundMedia );
        if( err ) {
            fprintf( stderr, "BeginMediaEdits(soundMedia) failed (%d)\n", err );
            goto bail;
        }
        pMungData->didBeginSoundMediaEdits = true;
        
        // cache the sound sample description
        err = QTGetComponentProperty(pMungData->audioChan, kQTPropertyClass_SGAudio,
                kQTSGAudioPropertyID_SoundDescription, sizeof(pMungData->audioDescH),
                &pMungData->audioDescH, NULL);
        if ( err ) {
            fprintf( stderr, "QTGetComponentProperty(kQTSGAudioPropertyID_SoundDescription) failed (%d)\n", err );
            goto bail;
        }
        
        // and get the AudioStreamBasicDescription equivalent (see CoreAudioTypes.h for more info on this struct)
        err = QTSoundDescriptionGetProperty(pMungData->audioDescH, kQTPropertyClass_SoundDescription,
                kQTSoundDescriptionPropertyID_AudioStreamBasicDescription,
                sizeof(pMungData->asbd), &pMungData->asbd, NULL);
        if ( err ) {
            fprintf( stderr, "QTSoundDescriptionGetProperty(ASBD) failed (%d)\n", err );
            goto bail;
        }
	}
    
	// Handle CBR and VBR audio.  For VBR formats (such as AAC), an array of 
	// AudioStreamPacketDescriptions accompanies each blob of audio packets (you'll find them
	// wrapped in a CFDataRef in the sgdataproc's chRefCon parameter).  If you are writing
	// VBR data, you must call AddMediaSample2 for each AudioStreamPacketDescription.
	
	if ( pMungData->asbd.mBytesPerPacket )
	{
		numSamples = (len / pMungData->asbd.mBytesPerPacket) * pMungData->asbd.mFramesPerPacket;

		err = AddMediaSample2(  pMungData->soundMedia,          // the Media
								p,                              // const UInt8 * dataIn
								len,                            // ByteCount size
								1,                              // TimeValue64 decodeDurationPerSample
								0,                              // TimeValue64 displayOffset
				(SampleDescriptionHandle)pMungData->audioDescH, // SampleDescriptionHandle sampleDescriptionH
								numSamples,                     // ItemCount numberOfSamples
								0,                              // MediaSampleFlags sampleFlags
								NULL );                         // TimeValue64 * sampleDecodeTimeOut
		if( err ) {
			fprintf( stderr, "AddMediaSample2(soundMedia) failed (%d)\n", err );
			goto bail;
		}
	}
	else {
		AudioStreamPacketDescription * aspd = (chRefCon ? (AudioStreamPacketDescription*)CFDataGetBytePtr((CFDataRef)chRefCon) : NULL);
		numSamples = CFDataGetLength((CFDataRef)chRefCon) / sizeof(AudioStreamPacketDescription);
		UInt32 i;

		for (i = 0; i < numSamples; i++)
		{
			err = AddMediaSample2(  pMungData->soundMedia,
									p + aspd[i].mStartOffset,
									aspd[i].mDataByteSize,
									pMungData->asbd.mFramesPerPacket,
									0,
									(SampleDescriptionHandle)pMungData->audioDescH,
									1,
									0,
									NULL );
			if ( err ) {
				fprintf( stderr, "AddMediaSample2(soundMedia) #%lu of %lu failed (%d)\n", i, numSamples, err );
				goto bail;
			}
		}
		
		if (chRefCon)
			CFRelease((CFDataRef)chRefCon);
	}
    
bail:
    return err;
}

/* ---------------------------------------------------------------------- */
/* sequence grabber data procedure - this is where the work is done
/* ---------------------------------------------------------------------- */
/* mungGrabDataProc - the sequence grabber calls the data function whenever
   any of the grabber’s channels write digitized data to the destination movie file.
   
   NOTE: We really mean any, if you have an audio and video channel then the DataProc will
   		 be called for either channel whenever data has been captured. Be sure to check which
   		 channel is being passed in.
   
   This data function makes sure its compression and decompression sessions are set up,
   and then feeds the frame to the decompression session.
   
   For more information refer to Inside Macintosh: QuickTime Components, page 5-120
   c - the channel component that is writing the digitized data.
   p - a pointer to the digitized data.
   len - the number of bytes of digitized data.
   offset - a pointer to a field that may specify where you are to write the digitized data,
   			and that is to receive a value indicating where you wrote the data.
   chRefCon - per channel reference constant specified using SGSetChannelRefCon.
   time	- the starting time of the data, in the channel’s time scale.
   writeType - the type of write operation being performed.
   		seqGrabWriteAppend - Append new data.
   		seqGrabWriteReserve - Do not write data. Instead, reserve space for the amount of data
   							  specified in the len parameter.
   		seqGrabWriteFill - Write data into the location specified by offset. Used to fill the space
   						   previously reserved with seqGrabWriteReserve. The Sequence Grabber may
   						   call the DataProc several times to fill a single reserved location.
   refCon - the reference constant you specified when you assigned your data function to the sequence grabber.
*/
pascal OSErr mungGrabDataProc(SGChannel c, Ptr p, long len, long *offset, long chRefCon, TimeValue time, short writeType, long refCon);
pascal OSErr mungGrabDataProc(SGChannel c, Ptr p, long len, long *offset, long chRefCon, TimeValue time, short writeType, long refCon)
{
#pragma unused(offset,chRefCon,writeType)
	ICMFrameTimeRecord frameTime = {{0}};
    ComponentResult err = noErr;
	int n;
    
    MungDataPtr pMungData = (MungDataPtr)refCon;
    if (NULL == pMungData) return -1;
	
	if (c == pMungData->audioChan) {
		// Handle audio in this other function:
		return (writeAudioToMovie(pMungData, (UInt8 *)p, len, offset, time, chRefCon));
	}
	
	// reset frame and time counters after a stop/start
	if (pMungData->lastTime > time) {
		pMungData->lastTime = 0;
		pMungData->frameCount = 0;
	}
    
    if (pMungData->timeScale == 0) {
    	// first time here so set the time scale
    	err = SGGetChannelTimeScale(c, &pMungData->timeScale);
    	DisplayAndBail(err, "SGGetChannelTimeScale");
		
		// Work out how much to throttle frame times
		pMungData->minimumFrameDuration = pMungData->timeScale / pMungData->desiredFramesPerSecond;
    }

	// round times to a multiple of the frame rate
	n = floor( ( ((float)time) * pMungData->desiredFramesPerSecond / pMungData->timeScale ) + 0.5 );
	time = n * pMungData->timeScale / pMungData->desiredFramesPerSecond;
    
	if( ( pMungData->lastTime > 0 ) && ( time < pMungData->lastTime + pMungData->minimumFrameDuration ) ) {
		// drop this frame.
		err = noErr;
		goto bail;
	}
	
    pMungData->frameCount++;
        
	if (pMungData->compressionSession == NULL) {
		// Set up a compression session that will compress each frame and call 
		// writeEncodedFrameToMovie with each output frame.
		err = createCompressionSession( 
				pMungData->width, pMungData->height, pMungData->codecType, 
				pMungData->averageDataRate, pMungData->timeScale,
				writeEncodedFrameToMovie, pMungData,
				&pMungData->compressionSession );
		DisplayAndBail(err, "createCompressionSession");
	}

	if (pMungData->decompressionSession == NULL) {
		// Set up decompression session
		
		ImageDescriptionHandle imageDesc = (ImageDescriptionHandle)NewHandle(0);
		
		// retrieve a channel’s current sample description, the channel returns a sample description that is
		// appropriate to the type of data being captured
		err = SGGetChannelSampleDescription(c, (Handle)imageDesc);
		DisplayAndBail(err, "SGGetChannelSampleDescription");
		
		// Set up a decompression session that will call displayAndCompressFrame with each decoded frame.
		// Note: if we didn't need to draw on frames with CG or display the frames in an HIImageView, we 
		// could improve performance by getting the compression session's preferred pixel buffer attributes
		// via the kICMCompressionSessionPropertyID_CompressorPixelBufferAttributes property
		// and passing that when creating the decompression session.
		err = createDecompressionSession( imageDesc, 
				pMungData->width, pMungData->height, k32ARGBPixelFormat, 
				displayAndCompressFrame, pMungData, 
				&pMungData->decompressionSession );
		DisplayAndBail(err, "createDecompressionSession");
		
		DisposeHandle((Handle)imageDesc);         
	}
	
	frameTime.recordSize = sizeof(ICMFrameTimeRecord);
	*(TimeValue64 *)&frameTime.value = time;
	frameTime.scale = pMungData->timeScale;
	frameTime.rate = fixed1;
	frameTime.frameNumber = pMungData->frameCount;
	frameTime.flags = icmFrameTimeIsNonScheduledDisplayTime;
	
	// Push encoded frame in.
	err = ICMDecompressionSessionDecodeFrame( pMungData->decompressionSession,
			(UInt8 *)p, len, NULL, &frameTime, pMungData );
	DisplayAndBail(err, "ICMDecompressionSessionDecodeFrame");
	
	// Pull decoded frame out.
	ICMDecompressionSessionSetNonScheduledDisplayTime( pMungData->decompressionSession, time, pMungData->timeScale, 0 );
	
    pMungData->lastTime = time;

bail:
	return err;
}

// Munggrab idle timer to idle the sequence grabber, call this at least
// as much as the desired frame rate - more is better
static pascal void mgIdleTimer(EventLoopTimerRef inTimer, void *inUserData)
{
#pragma unused(inTimer)
	
	MungDataPtr pMungData = (MungDataPtr)inUserData;
	if (NULL == pMungData) return;
	
	OSErr err = SGIdle(pMungData->seqGrab);
	if (err && err != pMungData->err) {
			// some error specific to SGIdle occurred - any errors returned from the
			// data proc will also show up here and we don't want to write over them
			
			// in QT 4 you would always encounter a cDepthErr error after a user drags
			// the window, this failure condition has been greatly relaxed in QT 5
			// it may still occur but should only apply to vDigs that really control
			// the screen
			
			// you don't always know where these errors originate from, some may come
			// from the VDig...
			
			displayError(pMungData->pWindow, "SGIdle", err);
			
			// ...to fix this we simply call SGStop and SGStartRecord again
			// calling stop allows the SG to release and re-prepare for grabbing
			// hopefully fixing any problems, this is obviously a very relaxed
			// approach
			SGStop(pMungData->seqGrab);
			SGStartRecord(pMungData->seqGrab);
	}
}

static void disposeCaptureWindow( WindowRef window )
{
    MungDataPtr 		pMungData = (MungDataPtr)GetWRefCon( window );

	DisposeWindow(window);
    disposeMungData(pMungData);
}

static pascal OSStatus mgWindowEventHandler(EventHandlerCallRef nextHandler, EventRef theEvent, void *inUserData)
{
#pragma unused(nextHandler)

	OSStatus status = eventNotHandledErr;
    UInt32 eventKind, eventClass;
	
	MungDataPtr pMungData = (MungDataPtr)inUserData;
	if (NULL == pMungData) goto done;
	
	
	eventKind = GetEventKind(theEvent);
	eventClass = GetEventClass(theEvent);
	
		
	switch (eventClass) {
    case kEventClassKeyboard:
        {
            switch (eventKind) {
            case kEventRawKeyDown:
			case kEventRawKeyRepeat:
                {
                    // adjust the audio preview volume if it's the up or down arrow key
                    char	macKeyCode;
                    OSErr   err;
					Float32 volume;
                    
                    err = GetEventParameter(theEvent, kEventParamKeyMacCharCodes, typeChar, 
												NULL, sizeof(macKeyCode), NULL, &macKeyCode);
                    if (err) goto done;
                    
                    switch (macKeyCode) {
					case 31: // down arrow key
						if (noErr == QTGetComponentProperty(	pMungData->audioChan,
																kQTPropertyClass_SGAudioPreviewDevice,
																kQTSGAudioPropertyID_MasterGain,
																sizeof(volume), &volume, NULL ) )
						{
							if (volume > 0.)
							{
								volume -= .1;
									// pin it to zero
								if (volume < 0.)
									volume = 0.;
									
								(void)QTSetComponentProperty(	pMungData->audioChan,
																kQTPropertyClass_SGAudioPreviewDevice,
																kQTSGAudioPropertyID_MasterGain,
																sizeof(volume), &volume );
							}
							status = noErr;
						}
						break;
						
					case 30: // up arrow key
						if (noErr == QTGetComponentProperty(	pMungData->audioChan,
																kQTPropertyClass_SGAudioPreviewDevice,
																kQTSGAudioPropertyID_MasterGain,
																sizeof(volume), &volume, NULL ) )
						{
							// don't pin the volume to an upper limit of 1.0.  The volume
							// level is being set in software on a mixer, so it can be
							// set to levels greater than 1.0, in effect boosting low
							// signals.
							volume += .1;
								
							(void)QTSetComponentProperty(	pMungData->audioChan,
															kQTPropertyClass_SGAudioPreviewDevice,
															kQTSGAudioPropertyID_MasterGain,
															sizeof(volume), &volume );
							status = noErr;
						}
						break;
					
					}
                }
				break;
			}
        }
        break;
	case kEventClassWindow:
		{
			switch (eventKind) {
			case kEventWindowClose:
				{
					// we're done
					WindowRef theWindow;
					
					// we need the window ref or bail
					OSErr err;
					err = GetEventParameter(theEvent, kEventParamDirectObject, typeWindowRef, NULL, sizeof(theWindow), NULL, &theWindow);
					if (err) goto done;
					
					RemoveEventLoopTimer(pMungData->timerRef);
					disposeCaptureWindow( theWindow ); // this also finishes creating the output movie
					QuitApplicationEventLoop();
					status = noErr;
					break;
				}
				break;
			}
		}
		break;
	
	default:
		break;
	}

done:
	return status;
}

pascal OSErr handleQuitAE(const AppleEvent *theAppleEvent, AppleEvent *reply, long inRefcon);
pascal OSErr handleQuitAE(const AppleEvent *theAppleEvent, AppleEvent *reply, long inRefcon)
{
#pragma unused (theAppleEvent, reply)

	MungDataPtr pMungData = (MungDataPtr)inRefcon;
	
	if (pMungData) {
		RemoveEventLoopTimer(pMungData->timerRef);
		disposeCaptureWindow( pMungData->pWindow ); // this also finishes creating the output movie
	}
	QuitApplicationEventLoop();
	
	return noErr;
}

static OSErr installEventHandlers(MungDataPtr inMungData)
{
	OSStatus err = eventInternalErr;	
	const EventTypeSpec windowEventList[] = { { kEventClassWindow, kEventWindowClose },
                                              { kEventClassKeyboard, kEventRawKeyDown },
                                              { kEventClassKeyboard, kEventRawKeyRepeat } };

	err = InstallEventLoopTimer(GetMainEventLoop(), kEventDurationNoWait, kTimerInterval, NewEventLoopTimerUPP(mgIdleTimer), inMungData, &inMungData->timerRef);
	BailErr(err);
	
	err = InstallWindowEventHandler(inMungData->pWindow, 
                                    NewEventHandlerUPP(mgWindowEventHandler), 
                                    GetEventTypeCount(windowEventList), 
                                    windowEventList, inMungData, NULL);
	BailErr(err);
	
	err = AEInstallEventHandler(kCoreEventClass, kAEQuitApplication, NewAEEventHandlerUPP(handleQuitAE), (long)inMungData, false);

bail:
	return err;	
}

static OSStatus startCaptureUsingWindow( WindowRef window )
{
	OSStatus			err = noErr;
	Rect				portRect = { 0, 0, 240, 320 };
    MungDataPtr 		pMungData = NULL;
    SeqGrabComponent	seqGrab = 0;
    SGChannel			sgchanVideo = NULL, sgchanAudio = NULL;
    
    GetPortBounds(GetWindowPort(window), &portRect);
	
    // create and initialize the sequence grabber
    seqGrab = makeSequenceGrabber( window );
    BailErr(NULL == seqGrab);
    
    // create the video channel
    err = makeSequenceGrabVideoChannel( seqGrab, &sgchanVideo, &portRect );
    BailErr(err);
	
	// create the audio channel - disregarding the error.  We'll
	// carry on whether or not there's an audio recording device present
	if (noErr != makeSequenceGrabAudioChannel( seqGrab, &sgchanAudio ) )
		sgchanAudio = NULL;
	
    
    // initialize our data that's going to be passed around
    pMungData = initializeMungData( window, portRect, seqGrab );
    BailErr(NULL == pMungData);
	
	SetWRefCon( window, (long)pMungData );
	pMungData->audioChan = sgchanAudio;
	
	
    
    // specify a sequence grabber data function
	err = SGSetDataProc( seqGrab, NewSGDataUPP(mungGrabDataProc), (long)pMungData );
	BailErr(err);
	
	// install carbon event handlers
	err = installEventHandlers( pMungData );
	BailErr(err);
	
	// lights...camera...
	err = SGPrepare( seqGrab, false, true );
	BailErr(err); 
    
    // ...action
	err = SGStartRecord( seqGrab );
	BailErr(err);
	pMungData->isGrabbing = true;
	
bail:
	return err;
}

int main(int argc, char* argv[])
{
    IBNibRef 		nibRef = NULL;
    WindowRef 		window = NULL;
    
    OSStatus		err = noErr;

	EnterMovies();

    // Create a Nib reference passing the name of the nib file (without the .nib extension)
    // CreateNibReference only searches into the application bundle.
    err = CreateNibReference( CFSTR("main"), &nibRef );
    require_noerr( err, CantGetNibRef );
    
    // Once the nib reference is created, set the menu bar. "MainMenu" is the name of the menu bar
    // object. This name is set in InterfaceBuilder when the nib is created.
    err = SetMenuBarFromNib( nibRef, CFSTR("MenuBar") );
    require_noerr( err, CantSetMenuBar );
    
    // Then create a window. "MainWindow" is the name of the window object. This name is set in 
    // InterfaceBuilder when the nib is created.
    err = CreateWindowFromNib( nibRef, CFSTR("MainWindow"), &window );
    require_noerr( err, CantCreateWindow );

    // We don't need the nib reference anymore.
    DisposeNibReference( nibRef );
    
	err = startCaptureUsingWindow( window );
	if( err ) {
		DialogRef alert = NULL;
		DialogItemIndex ignored;
		CFStringRef explanation = CFStringCreateWithFormat( NULL, NULL, CFSTR("Error code %d."), (int)err );
		CreateStandardAlert( kAlertStopAlert,
				CFSTR("Could not set up capture.  Make sure you have a camera attached and turned on."),
				explanation, NULL, &alert );
		RunStandardAlert( alert, NULL, &ignored );
		CFRelease( explanation );
		goto CantStartCapture;
	}
	
    // The window was created hidden so show it.
    ShowWindow( window );
    
    // Call the event loop
    RunApplicationEventLoop();

CantStartCapture:
CantCreateWindow:
CantSetMenuBar:
CantGetNibRef:
	exit(0);
}

