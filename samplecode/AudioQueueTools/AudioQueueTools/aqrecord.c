/*=============================================================================
	aqrecord.cpp
	
	$Log: aqrecord.c,v $
	Revision 1.5  2007/06/01 23:28:26        
	fix up InferAudioFileFormatFromFilename
	
	Revision 1.4  2007/05/24 01:08:18        
	fix usage text
	
	Revision 1.3  2007/05/24 00:21:08        
	use AudioHardwareServices to get device sample rate
	
	Revision 1.2  2007/05/24 00:12:10        
	cleanup
	
	Revision 1.1  2007/05/22 22:15:51        
	first time, named .c
	
	Revision 1.8  2007/05/21 23:45:37        
	latest
	
	Revision 1.7  2007/03/24 01:07:06       
	some minor cleanup
	
	Revision 1.6  2007/03/23 23:18:02       
	improve and make consistent both examples
	
	Revision 1.5  2007/03/23 03:05:34       
	add logic to be smarter about allocating sizes
	
	Revision 1.4  2007/03/07 02:54:57       
	working in some way
	
	Revision 1.3  2007/03/06 22:59:24       
	tweak formatting
	
	Revision 1.2  2007/03/06 22:55:54       
	first pass at new example code
	
	Revision 1.1  2007/02/24 02:37:58          
	first revision
	
	Revision 1.3  2007/02/08 17:45:44        
	record packetized
	
	Revision 1.2  2007/02/06 01:59:57        
	latest
	
	Revision 1.1  2007/01/26 22:51:16        
	first time
	
	created 12/15/06, Doug Wyatt
	Copyright (c) 2006-7 Apple Inc.  All Rights Reserved

	$NoKeywords: $
=============================================================================*/

#include <AudioToolbox/AudioToolbox.h>

#define USE_AUDIO_SERVICES 1

// ____________________________________________________________________________________
// report a usage error and exit with an error.
static void	usage()
{
	fprintf(stderr,
	"usage: AQRecord [options] <recordfile>\n"
	"options:\n"
	"  -d <format>       specify file audio data format (e.g. 'lpcm', 'aac ' etc.; defaults to 16-bit big endian PCM)\n"
	"  -c <nchannels>    specify number of channels to record (2/stereo is the default)\n"
	"  -r <sample_rate>  specify sample rate; default is to use the hardware rate\n"
	"  -s <seconds>      record for a fixed period of time\n"
	"  -v                show verbose progress\n"
		);
	exit(2);
}

// ____________________________________________________________________________________
// report a missing argument for an option.
static void MissingArgument(const char *opt)
{
	fprintf(stderr, "Missing argument for option '%s'\n\n", opt);
	usage();
}

// ____________________________________________________________________________________
// report an unparseable argument
static void ParseError(const char *opt, const char *val)
{
	fprintf(stderr, "Couldn't parse argument '%s' for option '%s'\n\n", val, opt);
	usage();
}

// ____________________________________________________________________________________
// Convert a C string to a 4-char code.
// interpret hex literals such as "\x00".
// return number of characters parsed.
static int StrTo4CharCode(const char *str, FourCharCode *p4cc)
{
	char buf[4];
	const char *p = str;
	int i, x;
	for (i = 0; i < 4; ++i) {
		if (*p != '\\') {
			if ((buf[i] = *p++) == '\0') {
				// special-case for 'aac ': if we only got three characters, assume the last was a space
				if (i == 3) {
					--p;
					buf[i] = ' ';
					break;
				}
				goto fail;
			}
		} else {
			if (*++p != 'x') goto fail;
			if (sscanf(++p, "%02X", &x) != 1) goto fail;
			buf[i] = x;
			p += 2;
		}
	}
	*p4cc = CFSwapInt32BigToHost(*(UInt32 *)buf);
	return p - str;
fail:
	return 0;
}

// ____________________________________________________________________________________
// return true if testExt (should not include ".") is in the array "extensions".
static Boolean MatchExtension(CFArrayRef extensions, CFStringRef testExt)
{
	CFIndex n = CFArrayGetCount(extensions), i;
	for (i = 0; i < n; ++i) {
		CFStringRef ext = (CFStringRef)CFArrayGetValueAtIndex(extensions, i);
		if (CFStringCompare(testExt, ext, kCFCompareCaseInsensitive) == kCFCompareEqualTo) {
			return TRUE;
		}
	}
	return FALSE;
}

// ____________________________________________________________________________________
// Infer an audio file type from a filename's extension.
static Boolean InferAudioFileFormatFromFilename(CFStringRef filename, AudioFileTypeID *outFiletype)
{
	OSStatus err;
	
	// find the extension in the filename.
	CFRange range = CFStringFind(filename, CFSTR("."), kCFCompareBackwards);
	if (range.location == kCFNotFound)
		return FALSE;
	range.location += 1;
	range.length = CFStringGetLength(filename) - range.location;
	CFStringRef extension = CFStringCreateWithSubstring(NULL, filename, range);
	
	UInt32 propertySize = sizeof(AudioFileTypeID);
	err = AudioFileGetGlobalInfo(kAudioFileGlobalInfo_TypesForExtension, sizeof(extension), &extension, &propertySize, outFiletype);
	CFRelease(extension);
	
	return (err == noErr && propertySize > 0);
}

static Boolean MyFileFormatRequiresBigEndian(AudioFileTypeID audioFileType, int bitdepth)
{
	AudioFileTypeAndFormatID ftf;
	UInt32 propertySize;
	OSStatus err;
	Boolean requiresBigEndian;
	
	ftf.mFileType = audioFileType;
	ftf.mFormatID = kAudioFormatLinearPCM;
	
	err = AudioFileGetGlobalInfoSize(kAudioFileGlobalInfo_AvailableStreamDescriptionsForFormat, sizeof(ftf), &ftf, &propertySize);
	if (err) return FALSE;

	AudioStreamBasicDescription *formats = (AudioStreamBasicDescription *)malloc(propertySize);
	err = AudioFileGetGlobalInfo(kAudioFileGlobalInfo_AvailableStreamDescriptionsForFormat, sizeof(ftf), &ftf, &propertySize, formats);
	requiresBigEndian = TRUE;
	int i, nFormats = propertySize / sizeof(AudioStreamBasicDescription);
	for (i = 0; i < nFormats; ++i) {
		if (formats[i].mBitsPerChannel == bitdepth
		&& !(formats[i].mFormatFlags & kLinearPCMFormatFlagIsBigEndian))
			return FALSE;
	}
	free(formats);
	return requiresBigEndian;
}

// ____________________________________________________________________________________
// ____________________________________________________________________________________
// ____________________________________________________________________________________


// custom data structure "MyRecorder"
// data we need during callback functions.

#define kNumberRecordBuffers	3

typedef struct MyRecorder {
	AudioQueueRef				queue;
	
	CFAbsoluteTime				queueStartStopTime;
	AudioFileID					recordFile;
	SInt64						recordPacket; // current packet number in record file
	Boolean						running;
	Boolean						verbose;
} MyRecorder;


// ____________________________________________________________________________________
// generic error handler - if err is nonzero, prints error message and exits program.
static void CheckError(OSStatus error, const char *operation)
{
	if (error == noErr) return;
	
	char str[20];
	// see if it appears to be a 4-char-code
	*(UInt32 *)(str + 1) = CFSwapInt32HostToBig(error);
	if (isprint(str[1]) && isprint(str[2]) && isprint(str[3]) && isprint(str[4])) {
		str[0] = str[5] = '\'';
		str[6] = '\0';
	} else
		// no, format it as an integer
		sprintf(str, "%d", (int)error);

	fprintf(stderr, "Error: %s (%s)\n", operation, str);

	exit(1);
}

// ____________________________________________________________________________________
// Determine the size, in bytes, of a buffer necessary to represent the supplied number
// of seconds of audio data.
static int MyComputeRecordBufferSize(const AudioStreamBasicDescription *format, AudioQueueRef queue, float seconds)
{
	int packets, frames, bytes;
	
	frames = (int)ceil(seconds * format->mSampleRate);
	
	if (format->mBytesPerFrame > 0)
		bytes = frames * format->mBytesPerFrame;
	else {
		UInt32 maxPacketSize;
		if (format->mBytesPerPacket > 0)
			maxPacketSize = format->mBytesPerPacket;	// constant packet size
		else {
			UInt32 propertySize = sizeof(maxPacketSize); 
			CheckError(AudioQueueGetProperty(queue, kAudioConverterPropertyMaximumOutputPacketSize, &maxPacketSize,
				&propertySize), "couldn't get queue's maximum output packet size");
		}
		if (format->mFramesPerPacket > 0)
			packets = frames / format->mFramesPerPacket;
		else
			packets = frames;	// worst-case scenario: 1 frame in a packet
		if (packets == 0)		// sanity check
			packets = 1;
		bytes = packets * maxPacketSize;
	}
	return bytes;
}

// ____________________________________________________________________________________
// Copy a queue's encoder's magic cookie to an audio file.
static void MyCopyEncoderCookieToFile(AudioQueueRef theQueue, AudioFileID theFile)
{
	OSStatus err;
	UInt32 propertySize;
	
	// get the magic cookie, if any, from the converter		
	err = AudioQueueGetPropertySize(theQueue, kAudioConverterCompressionMagicCookie, &propertySize);
	
	if (err == noErr && propertySize > 0) {
		// there is valid cookie data to be fetched;  get it
		Byte *magicCookie = (Byte *)malloc(propertySize);
		CheckError(AudioQueueGetProperty(theQueue, kAudioConverterCompressionMagicCookie, magicCookie,
			&propertySize), "get audio converter's magic cookie");
		// now set the magic cookie on the output file
		err = AudioFileSetProperty(theFile, kAudioFilePropertyMagicCookieData, propertySize, magicCookie);
		free(magicCookie);
	}
}

// ____________________________________________________________________________________
// AudioQueue callback function, called when a property changes.
static void MyPropertyListener(void *userData, AudioQueueRef queue, AudioQueuePropertyID propertyID)
{
	MyRecorder *aqr = (MyRecorder *)userData;
	if (propertyID == kAudioQueueProperty_IsRunning)
		aqr->queueStartStopTime = CFAbsoluteTimeGetCurrent();
}

// ____________________________________________________________________________________
// AudioQueue callback function, called when an input buffers has been filled.
static void MyInputBufferHandler(	void *                          inUserData,
									AudioQueueRef                   inAQ,
									AudioQueueBufferRef             inBuffer,
									const AudioTimeStamp *          inStartTime,
									UInt32							inNumPackets,
									const AudioStreamPacketDescription *inPacketDesc)
{
	MyRecorder *aqr = (MyRecorder *)inUserData;
	
	if (aqr->verbose) {
		printf("buf data %p, 0x%x bytes, 0x%x packets\n", inBuffer->mAudioData,
			(int)inBuffer->mAudioDataByteSize, (int)inNumPackets);
	}
	
	if (inNumPackets > 0) {
		// write packets to file
		CheckError(AudioFileWritePackets(aqr->recordFile, FALSE, inBuffer->mAudioDataByteSize,
			inPacketDesc, aqr->recordPacket, &inNumPackets, inBuffer->mAudioData),
			"AudioFileWritePackets failed");
		aqr->recordPacket += inNumPackets;
	}

	// if we're not stopping, re-enqueue the buffe so that it gets filled again
	if (aqr->running)
		CheckError(AudioQueueEnqueueBuffer(inAQ, inBuffer, 0, NULL), "AudioQueueEnqueueBuffer failed");
}

// ____________________________________________________________________________________
// get sample rate of the default input device
OSStatus	MyGetDefaultInputDeviceSampleRate(Float64 *outSampleRate)
{
	OSStatus err;
	AudioDeviceID deviceID = 0;

	// get the default input device
	AudioObjectPropertyAddress addr;
	UInt32 size;
	addr.mSelector = kAudioHardwarePropertyDefaultInputDevice;
	addr.mScope = kAudioObjectPropertyScopeGlobal;
	addr.mElement = 0;
	size = sizeof(AudioDeviceID);
#if USE_AUDIO_SERVICES
	err = AudioHardwareServiceGetPropertyData(kAudioObjectSystemObject, &addr, 0, NULL, &size, &deviceID);
#else
	err = AudioObjectGetPropertyData(kAudioObjectSystemObject, &addr, 0, NULL, &size, &deviceID);
#endif
	if (err) return err;

	// get its sample rate
	addr.mSelector = kAudioDevicePropertyNominalSampleRate;
	addr.mScope = kAudioObjectPropertyScopeGlobal;
	addr.mElement = 0;
	size = sizeof(Float64);
#if USE_AUDIO_SERVICES
	err = AudioHardwareServiceGetPropertyData(deviceID, &addr, 0, NULL, &size, outSampleRate);
#else
	err = AudioObjectGetPropertyData(deviceID, &addr, 0, NULL, &size, outSampleRate);
#endif

	return err;
}

// ____________________________________________________________________________________
// main program
int	main(int argc, const char *argv[])
{
	const char *recordFileName = NULL;
	int i, nchannels, bufferByteSize;
	float seconds = 0;
	AudioStreamBasicDescription recordFormat;
	MyRecorder aqr;
	UInt32 size;
	CFURLRef url;
	
	// fill structures with 0/NULL
	memset(&recordFormat, 0, sizeof(recordFormat));
	memset(&aqr, 0, sizeof(aqr));
	
	// parse arguments
	for (i = 1; i < argc; ++i) {
		const char *arg = argv[i];
		
		if (arg[0] == '-') {
			switch (arg[1]) {
			case 'c':
				if (++i == argc) MissingArgument(arg);
				if (sscanf(argv[i], "%d", &nchannels) != 1)
					usage();
				recordFormat.mChannelsPerFrame = nchannels;
				break;
			case 'd':
				if (++i == argc) MissingArgument(arg);
				if (StrTo4CharCode(argv[i], &recordFormat.mFormatID) == 0)
					ParseError(arg, argv[i]);
				break;
			case 'r':
				if (++i == argc) MissingArgument(arg);
				if (sscanf(argv[i], "%lf", &recordFormat.mSampleRate) != 1)
					ParseError(arg, argv[i]);
				break;
			case 's':
				if (++i == argc) MissingArgument(arg);
				if (sscanf(argv[i], "%f", &seconds) != 1)
					ParseError(arg, argv[i]);
				break;
			case 'v':
				aqr.verbose = TRUE;
				break;
			default:
				fprintf(stderr, "unknown option: '%s'\n\n", arg);
				usage();
			}
		} else if (recordFileName != NULL) {
			fprintf(stderr, "may only specify one file to record\n\n");
			usage();
		} else
			recordFileName = arg;
	}
	if (recordFileName == NULL) // no record file path provided
		usage();
	
	// determine file format
	AudioFileTypeID audioFileType = kAudioFileCAFType;	// default to CAF
	CFStringRef cfRecordFileName = CFStringCreateWithCString(NULL, recordFileName, kCFStringEncodingUTF8);
	InferAudioFileFormatFromFilename(cfRecordFileName, &audioFileType);
	CFRelease(cfRecordFileName);

	// adapt record format to hardware and apply defaults
	if (recordFormat.mSampleRate == 0.)
		MyGetDefaultInputDeviceSampleRate(&recordFormat.mSampleRate);

	if (recordFormat.mChannelsPerFrame == 0)
		recordFormat.mChannelsPerFrame = 2;
	
	if (recordFormat.mFormatID == 0 || recordFormat.mFormatID == kAudioFormatLinearPCM) {
		// default to PCM, 16 bit int
		recordFormat.mFormatID = kAudioFormatLinearPCM;
		recordFormat.mFormatFlags = kLinearPCMFormatFlagIsSignedInteger | kLinearPCMFormatFlagIsPacked;
		recordFormat.mBitsPerChannel = 16;
		if (MyFileFormatRequiresBigEndian(audioFileType, recordFormat.mBitsPerChannel))
			recordFormat.mFormatFlags |= kLinearPCMFormatFlagIsBigEndian;
		recordFormat.mBytesPerPacket = recordFormat.mBytesPerFrame =
			(recordFormat.mBitsPerChannel / 8) * recordFormat.mChannelsPerFrame;
		recordFormat.mFramesPerPacket = 1;
		recordFormat.mReserved = 0;
	}
	
	// create the queue
	CheckError(AudioQueueNewInput(
		&recordFormat,
		MyInputBufferHandler,
		&aqr /* userData */,
		NULL /* run loop */, NULL /* run loop mode */,
		0 /* flags */, &aqr.queue), "AudioQueueNewInput failed");

	// get the record format back from the queue's audio converter --
	// the file may require a more specific stream description than was necessary to create the encoder.
	size = sizeof(recordFormat);
	CheckError(AudioQueueGetProperty(aqr.queue, kAudioConverterCurrentOutputStreamDescription,
		&recordFormat, &size), "couldn't get queue's format");

	// convert recordFileName from C string to CFURL
	url = CFURLCreateFromFileSystemRepresentation(NULL, (Byte *)recordFileName, strlen(recordFileName), FALSE);
	
	// create the audio file
	CheckError(AudioFileCreateWithURL(url, audioFileType, &recordFormat, kAudioFileFlags_EraseFile,
		&aqr.recordFile), "AudioFileCreateWithURL failed");
	CFRelease(url);

	// copy the cookie first to give the file object as much info as we can about the data going in
	MyCopyEncoderCookieToFile(aqr.queue, aqr.recordFile);

	// allocate and enqueue buffers
	bufferByteSize = MyComputeRecordBufferSize(&recordFormat, aqr.queue, 0.5);	// enough bytes for half a second
	for (i = 0; i < kNumberRecordBuffers; ++i) {
		AudioQueueBufferRef buffer;
		CheckError(AudioQueueAllocateBuffer(aqr.queue, bufferByteSize, &buffer),
			"AudioQueueAllocateBuffer failed");
		CheckError(AudioQueueEnqueueBuffer(aqr.queue, buffer, 0, NULL),
			"AudioQueueEnqueueBuffer failed");
	}
	
	// record
	if (seconds > 0) {
		// user requested a fixed-length recording (specified a duration with -s)
		// to time the recording more accurately, watch the queue's IsRunning property
		CheckError(AudioQueueAddPropertyListener(aqr.queue, kAudioQueueProperty_IsRunning,
			MyPropertyListener, &aqr), "AudioQueueAddPropertyListener failed");
		
		// start the queue
		aqr.running = TRUE;
		CheckError(AudioQueueStart(aqr.queue, NULL), "AudioQueueStart failed");
		CFAbsoluteTime waitUntil = CFAbsoluteTimeGetCurrent() + 10;

		// wait for the started notification
		while (aqr.queueStartStopTime == 0.) {
			CFRunLoopRunInMode(kCFRunLoopDefaultMode, 0.010, FALSE);
			if (CFAbsoluteTimeGetCurrent() >= waitUntil) {
				fprintf(stderr, "Timeout waiting for the queue's IsRunning notification\n");
				goto cleanup;
			}
		}
		printf("Recording...\n");
		CFAbsoluteTime stopTime = aqr.queueStartStopTime + seconds;
		CFAbsoluteTime now = CFAbsoluteTimeGetCurrent();
		CFRunLoopRunInMode(kCFRunLoopDefaultMode, stopTime - now, FALSE);
	} else {
		// start the queue
		aqr.running = TRUE;
		CheckError(AudioQueueStart(aqr.queue, NULL), "AudioQueueStart failed");
		
		// and wait
		printf("Recording, press <return> to stop:\n");
		getchar();
	}

	// end recording
	printf("* recording done *\n");
	aqr.running = FALSE;
	CheckError(AudioQueueStop(aqr.queue, TRUE), "AudioQueueStop failed");
	
	// a codec may update its cookie at the end of an encoding session, so reapply it to the file now
	MyCopyEncoderCookieToFile(aqr.queue, aqr.recordFile);

cleanup:
	AudioQueueDispose(aqr.queue, TRUE);
	AudioFileClose(aqr.recordFile);

	return 0;
}
