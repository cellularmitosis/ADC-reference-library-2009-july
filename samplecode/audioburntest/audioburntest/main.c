#include <glob.h>
#include "dru_devices.h"
#include "dru_burning.h"
#import <fcntl.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>

typedef struct AIFFFileInfo
{
	int				fd;				// The file descriptor of the open file
	unsigned		numChannels;	// The number of channels of audio 
	unsigned		sampleFreq;		// the sampling frequency. We're only going to support 44.1KHz.
	unsigned		sampleBits;		// bits/sample
	unsigned		sampleBytes;	// number of bytes for each sample. Calculated value
	unsigned long	dataStart;		// Where the sound data starts in the file
	unsigned long	dataEnd;		// Where the sound data ends in the file.
	unsigned long	cursor;			// where in the file we're at
	int				mask;			// channel mixing mask

} AIFFFileInfo;


int main(int argc, char *argv[]);

/* Creating the audio tracks */
CFArrayRef		 createTracks();
CFMutableDictionaryRef 
				createAudioTrackProperties(uint32_t trackLength);

/* track production callback functions */
OSStatus		sampleTrackProducerCallback(DRTrackRef track, DRTrackMessage message, void* ioParam);
OSStatus		preBurnCallback(DRTrackRef track);
OSStatus		productionCallback(DRTrackRef track, DRTrackProductionInfo* prodInfo);
OSStatus		postBurnCallback(DRTrackRef track);

/* utility routines */
void			expandPathname(char *path);
AIFFFileInfo*	parseFile(char* filepath);
char*			promptForFolder(const char *prompt, const char *defaultPath, char* folderPath, int folderPathLen);
void			trackRefConDestructor(const void* refCon);


#pragma mark -


int main(int argc, char *argv[])
{
#pragma unused(argc, argv)
	DRDeviceRef				device = NULL;
	DRBurnRef				burn = NULL;
	CFArrayRef				trackLayout = NULL;
	
	/* Hello world! */
	printf("Welcome to the audio burn sample code!\n\n");
	
	/* First, let's use DRU to prompt the user to 
	   pick a device. If there's only one device to 
	   choose, the selection is automatic. */
	device = druPromptForDevice(NULL,druFilter_AnyBurner);
	
	/* We know we're going to burn to this device - acquire blank media
		reservations just in case the user inserts something. 
		Just because we ask for a shot at blank media doesn't mean that
		we'll get it. Some other app might not be willing to give 
		up the blank media for our use. */
	DRDeviceAcquireMediaReservation(device);
		
	/* create the AIFF tracks we'll be burning to CD. */
	trackLayout = createTracks();
	
	/* Next, use DRU to prompt the user to insert blank media. */
	druPromptForBlankMediaInDevice(device);
	
	/* Check to see if we have the media reservation. If we don't 
		have it that means that some other application has it and 
		won't give it up. We might not get it if, for instance, if
		the Finder has claimed the media for it's CD proxy and the user
		has placed files onto that proxy image. In this case the 
		Finder wouldn't want to unmount the proxy image since the
		user's intention is to burn that information to the inserted
		disc. */
	if (druMediaIsReserved(device))
	{
		/* We've got blank media, and a folder to burn ... 
		now set up the burn objects. */
		burn = DRBurnCreate(device);
		
		/* Use DRU to run the burn and report progress. */
		druBurn(burn,trackLayout);	
	}
	else
	{
		printf("The media in the selected device is reserved for use by another application.\n");
	}
	
	/* Clean up after ourselves. */
	if (burn != NULL)
		CFRelease(burn);
		
	if (trackLayout != NULL)
		CFRelease(trackLayout);
		
	if (device != NULL)
	{
		DRDeviceReleaseMediaReservation(device);
		CFRelease(device);
	}
	return 0;
}





CFArrayRef createTracks()
{
	CFMutableArrayRef		trackArray = CFArrayCreateMutable(kCFAllocatorDefault, 0, &kCFTypeArrayCallBacks);
	char 					path[PATH_MAX];
	int						pathLen;
	
	/* Prompt the user to pick a folder of AIFF's to burn. */
	promptForFolder(NULL,"~/Music", path, PATH_MAX);
	pathLen = strlen(path);
	
	// Remove trailing slashes, if any.
	if (pathLen>0 && path[pathLen-1] == '/') pathLen--;
	
	// Open the directory.
	DIR	*dirfd = opendir((const char*)path);
	if (dirfd == NULL) return 0;
	
	while (1)
	{
		CFMutableStringRef	fileStr;
		CFStringRef			aStr;
		AIFFFileInfo*		info;
		char				filepath[PATH_MAX];
		DRRefConCallbacks	callbacks;
		CFMutableDictionaryRef	properties;
		
		/* Get the next child.  This is threadsafe, because the struct dirent* is
		   not global; it's actually pointing to a buffer that was malloc'ed
		   inside the DIR structure. */
		struct dirent *childPtr = readdir(dirfd);
		if (childPtr == NULL) break;
		int	nameLength = childPtr->d_namlen;
		char *name = childPtr->d_name;
		
		if (childPtr->d_type == DT_DIR)
			continue;
			
		/* Skip "." and ".." entries, and AppleDouble "._" entries which 
		   we treat as one file. */
		if (name[0] == '.' && (nameLength == 1 || name[1] == '_' || (name[1] == '.' && nameLength == 2)))
			continue;
		
		/* skip all non-AIFF files by looking at the extension. */
		if (strncmp(".aiff", &name[nameLength-5], 5) != 0 && strncmp(".aif", &name[nameLength-4], 4) != 0)
			continue;
		
		aStr = CFStringCreateWithCString(kCFAllocatorDefault, path, kCFStringEncodingUTF8);
		fileStr = CFStringCreateMutableCopy(kCFAllocatorDefault, 0, aStr);
		CFRelease(aStr);
		CFStringAppendCString(fileStr, "/", kCFStringEncodingUTF8);
		CFStringAppendCString(fileStr, name, kCFStringEncodingUTF8);
			
		/* parse out the file information and construct an information structure 
		   to hold this info for use when we start the burn and start producing data. */
		CFStringGetCString(fileStr, filepath, PATH_MAX, kCFStringEncodingUTF8);
		info = parseFile(filepath);
		
		/* Create the properties dictionary describing this track. */
		properties = createAudioTrackProperties(((info->dataEnd - info->dataStart) / (info->sampleBytes * info->numChannels) * 4) / 2352);
		
		/* Add the file path for this file to the track properties. This will 
		   be used by the producer callback later. */
		CFDictionarySetValue(properties, CFSTR("kTrackSourceFilePath"), fileStr);
		CFRelease(fileStr);
		
		/* Create a track that is described by properties and whose data is 
		   produced by sampleTrackProducerCallback */
		DRTrackRef track = DRTrackCreate(properties, sampleTrackProducerCallback);
		
		/* stuff the info structure pointer into the refCon of the track object. Each object in
		   DiscRecording has a refCon associated with it. */
		callbacks.version = 0;
		callbacks.retain = NULL;
		callbacks.release = trackRefConDestructor;	/* we just need to destroy it when the track is released */
		DRSetRefCon(track, info, &callbacks);
		
		/* Put the track into the array for burning */
		CFArrayAppendValue(trackArray, track);
		CFRelease(properties);
		CFRelease(track);
	}
	
	closedir(dirfd);
	
	
	if (CFArrayGetCount(trackArray) == 0)
	{
		printf("No AIFF files were located!  Aborting.\n");
		exit(1);
	}
	
	return trackArray;
}





CFMutableDictionaryRef createAudioTrackProperties(uint32_t trackLength)
{
	CFMutableDictionaryRef	properties = CFDictionaryCreateMutable(kCFAllocatorDefault, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
	CFNumberRef				value;
	uint32_t				temp;
	
	/* Create a properties dictionary for all of the tracks. This dictionary
	   is common to each since each will be an audio track and other than the size
	   will be identical. */	
	temp = kDRBlockSizeAudio;
	value = CFNumberCreate(kCFAllocatorDefault, kCFNumberSInt32Type, &temp);
	CFDictionaryAddValue(properties, kDRBlockSizeKey, value);
	CFRelease(value);

	temp = kDRBlockTypeAudio;
	value = CFNumberCreate(kCFAllocatorDefault, kCFNumberSInt32Type, &temp);
	CFDictionaryAddValue(properties, kDRBlockTypeKey, value);
	CFRelease(value);

	temp = kDRDataFormAudio;
	value = CFNumberCreate(kCFAllocatorDefault, kCFNumberSInt32Type, &temp);
	CFDictionaryAddValue(properties, kDRDataFormKey, value);
	CFRelease(value);

	temp = kDRSessionFormatAudio;
	value = CFNumberCreate(kCFAllocatorDefault, kCFNumberSInt32Type, &temp);
	CFDictionaryAddValue(properties, kDRSessionFormatKey, value);
	CFRelease(value);

	temp = kDRTrackModeAudio;
	value = CFNumberCreate(kCFAllocatorDefault, kCFNumberSInt32Type, &temp);
	CFDictionaryAddValue(properties, kDRTrackModeKey, value);
	CFRelease(value);

	value = CFNumberCreate(kCFAllocatorDefault, kCFNumberSInt32Type, &trackLength);
	CFDictionarySetValue(properties, kDRTrackLengthKey, value);
	CFRelease(value);
	
	return properties;
}





/* This is a dispatcher to take the message and callout to different 
   functions to handle the messages we handle. */
OSStatus sampleTrackProducerCallback(DRTrackRef track, DRTrackMessage message, void* ioParam)
{
	switch (message)
	{
		case kDRTrackMessagePreBurn:
			return preBurnCallback(track);
			break;
			
		case kDRTrackMessageProduceData:
			return productionCallback(track, (DRTrackProductionInfo*)ioParam);
			break;
		
		case kDRTrackMessagePostBurn:
			return postBurnCallback(track);
			break;
	}
	
	return noErr;
}





/* This callback is called before the burn is to begin. It it we open up the AIFF file to read
   from and make sure it's valid (this could have been done before we started also). We also
   set up the final length of the track */
OSStatus preBurnCallback(DRTrackRef track)
{
	CFDictionaryRef	properties = DRTrackGetProperties(track);
	CFStringRef		fileStr = CFDictionaryGetValue(properties, CFSTR("kTrackSourceFilePath"));
	char			filepath[PATH_MAX];
	AIFFFileInfo*	info = (AIFFFileInfo*)DRGetRefCon(track);

	/* Get the file string from the properties dictionary and get the c string from that 
	   to open the file using open */
	CFStringGetCString(fileStr, filepath, PATH_MAX, kCFStringEncodingUTF8);
	
	info->fd = open(filepath, O_RDONLY, 0);
	if (info->fd == -1) return kDRDataProductionErr;

	fcntl(info->fd, F_NOCACHE, 1);						/* Don't cache the file in the UBC */
	lseek(info->fd, info->dataStart, SEEK_SET);			/* put the file pointer at the start of the sound data */
	info->mask = ((int)-0x10000) >> info->sampleBits;	/* set up the mask to do channel mixing */
	info->cursor = info->dataStart;
	
	return noErr;
}





/* The meat of the producer code. This callback is called repeatedly during a burn.
	It's this function's job to write data into the passed in buffer each time it's 
	called.  It's best if you can fill up the buffer completely. The buffer is
	a multiple of the track's block size in length. */
OSStatus productionCallback(DRTrackRef track, DRTrackProductionInfo* prodInfo)
{
	unsigned long		expectedFrames = (prodInfo->reqCount / 4);
	unsigned long		readSize, readLen;
	char*				tempBuffer;
	char*				outBuffer = prodInfo->buffer;
	unsigned long		outLength;
	unsigned long		i;
	unsigned long		step;
	AIFFFileInfo*		info = (AIFFFileInfo*)DRGetRefCon(track);

	readSize = (info->sampleBytes * info->numChannels) * expectedFrames;
	step = info->sampleBytes*info->numChannels;

	if (readSize + info->cursor > info->dataEnd)
		readSize = info->dataEnd - info->cursor;

	tempBuffer = (char*)malloc(readSize);
	
	readLen = read(info->fd, tempBuffer, readSize);
	if (readSize != readLen) return kDRDataProductionErr;

	outLength = 0;
	for (i=0; i<readSize; i += step)
	{
		unsigned short			leftSample = 0, rightSample = 0;
		unsigned const char*	sampleFrame = tempBuffer + i;
		
		#define READ_SAMPLE_POINT(frame,index)		\
			((*(uint16_t*)(frame + info->sampleBytes*index)) & info->mask)
		#define MIX(a,b)							\
			(((a * 2) / 3) + ((b * 2) / 3))
		
		/* Handle our wonderful multichannel logic.  Yeah, it's almost certainly unnecessary,
		   but it took an extra 2 minutes to write it this way since I was already thinking
		   about how to parse the AIFF properly.  Maybe it'll make someone's world a better place. */
		if (info->numChannels == 2)
		{
			leftSample = READ_SAMPLE_POINT(sampleFrame,0);
			rightSample = READ_SAMPLE_POINT(sampleFrame,1);
		}
		else if (info->numChannels == 1)
		{
			leftSample = READ_SAMPLE_POINT(sampleFrame,0);
			rightSample = leftSample;
		}
		else if (info->numChannels == 3)
		{
			unsigned short	centerSample = READ_SAMPLE_POINT(sampleFrame,2);
			leftSample = MIX(READ_SAMPLE_POINT(sampleFrame,0),centerSample);
			rightSample = MIX(READ_SAMPLE_POINT(sampleFrame,1),centerSample);
		}
		else if (info->numChannels == 4)
		{
			/* The spec is unclear on how to distinguish quadrophonic vs 4 ch surround AIFFs, as
			   they both have four channels.  Since surround seems to be the winner of these
			   ancient audio wars, for now I'm going to assume 4 channels means surround.
			   unsigned short that it matters. */
			UInt16	centerSample = MIX(READ_SAMPLE_POINT(sampleFrame,1),READ_SAMPLE_POINT(sampleFrame,3));
			leftSample = MIX(READ_SAMPLE_POINT(sampleFrame,0),centerSample);
			rightSample = MIX(READ_SAMPLE_POINT(sampleFrame,2),centerSample);
		}
		else if (info->numChannels == 6)
		{
			unsigned short	centerSample = MIX(READ_SAMPLE_POINT(sampleFrame,2),READ_SAMPLE_POINT(sampleFrame,5));
			leftSample = MIX(MIX(READ_SAMPLE_POINT(sampleFrame,0),READ_SAMPLE_POINT(sampleFrame,1)),centerSample);
			rightSample = MIX(MIX(READ_SAMPLE_POINT(sampleFrame,3),READ_SAMPLE_POINT(sampleFrame,4)),centerSample);
		}
		else
			return kDRDataProductionErr;
			
		#undef READ_SAMPLE_POINT
		#undef MIX
		
		// Dump the samples into the output.
		((unsigned short*)outBuffer)[0] = ((leftSample & 0xFF00) >> 8) + ((leftSample & 0x00FF) << 8);
		((unsigned short*)outBuffer)[1] = ((rightSample & 0xFF00) >> 8) + ((rightSample & 0x00FF) << 8);
		outBuffer += 4;
		outLength += 4;
	}
	
	free(tempBuffer);
	
	info->cursor += outLength;
	prodInfo->actCount = outLength;
	return noErr;
}





/* This callback comes in after the burn is finished and all data has been written
	to disc and optionally verified. All we need to do is to close the file opened in
	preBurn. */
OSStatus postBurnCallback(DRTrackRef track)
{
	AIFFFileInfo*		info = (AIFFFileInfo*)DRGetRefCon(track);

	close(info->fd);
	info->fd = 0;
	
	return noErr;
}





void trackRefConDestructor(const void* refCon)
{
	AIFFFileInfo*	info = (AIFFFileInfo*)refCon;
	if (info->fd != 0)
	{
		close(info->fd);
		info->fd = 0;
	}
	free(info);
}





char* promptForFolder(const char *prompt, const char *defaultPath, char* folderPath, int folderPathLen)
{
	char		path[PATH_MAX];
	OSStatus	err;
	Boolean		isDirectory;
	int			len;
	FSRef		ref;
	
	/* Display the prompt. */
	if (prompt == NULL)
		prompt = "Please enter the path to a folder:";
	printf("%s ", prompt);
	if (defaultPath != NULL)
		printf("[%s] ",defaultPath);
	fflush(stdout);
	
	/* Get user input, and trim trailing newlines. */
	fgets(path,sizeof(path),stdin);
	for (len = strlen(path); len > 0 && path[len-1] == '\n';)
		path[--len] = 0;
	if (path[0] == 0)
		strcpy(path,defaultPath);
	
	/* Expand magic characters just like a shell (mostly so ~ will work) */
	expandPathname(path);
	
	/* Convert the path into an FSRef, which is what the burn engine needs. */
	err = FSPathMakeRef((const UInt8*)path,&ref,&isDirectory);
	if (err != noErr)
	{
		printf("Bad path. Aborting. (%d)\n", (int)err);
		exit(1);
	}
	if (!isDirectory)
	{
		printf("That's a file, not a directory!  Aborting.\n");
		exit(1);
	}
	
	return strncpy(folderPath, path, folderPathLen);
}





void expandPathname(char *path)
{
	char	original[PATH_MAX];
	int		i;
	glob_t	g = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	
	/* Preserve the incoming path */
	strcpy(original,path);
	
	/* Call glob to expand it. */
	glob(path,GLOB_NOSORT | GLOB_TILDE | GLOB_QUOTE,NULL,&g);
	if (g.gl_matchc == 1)
		strcpy(path,g.gl_pathv[0]);
	else
	{
		printf("Bad path! ");
		if (g.gl_matchc == 0)
			printf("Not found. ");
		else
		{
			printf("%d matches found:\n",g.gl_matchc);
			for (i=0; i<g.gl_matchc; ++i)
				printf(" %s\n", g.gl_pathv[i]);
		}
		printf("Aborted.\n");
		exit(1);
	}
	globfree(&g);
}





#if PRAGMA_STRUCT_ALIGN
	#pragma options align=mac68k
#elif PRAGMA_STRUCT_PACKPUSH
	#pragma pack(push, 2)
#elif PRAGMA_STRUCT_PACK
	#pragma pack(2)
#endif

typedef struct IFFChunkHeader	IFFChunkHeader;
typedef struct AIFFHeader AIFFHeader;
typedef struct AIFFChunk AIFFChunk;
typedef struct AIFFCommonChunk AIFFCommonChunk;
typedef struct AIFFSoundChunk AIFFSoundChunk;

struct IFFChunkHeader
{
	UInt32			chunkID;		// id for this chunk
	UInt32			chunkSize;		// size of this chunk not including header
};


struct AIFFChunk
{
	UInt32			chunkID;		// id for this chunk
	UInt32			chunkSize;		// size of this chunk not including header
	UInt32			data[1];		// streamed data
};

struct AIFFHeader
{
	UInt32			form;			// 'FORM'
	UInt32			formChunkSize;	// largely ignored, but it should be (filesize - 8)
	UInt32			aiff;			// 'AIFF'
	AIFFChunk		chunk[1];		// packed array of chunks
};

struct AIFFCommonChunk
{
	UInt32			comm;				// 'COMM'
	UInt32			commChunkSize;		// size of this chunk not including header (18 bytes)
	UInt16			numChannels;		// number of sound channels
	UInt32			numSampleFrames;	// number of sample frames - probably incorrect!
	UInt16			sampleSize;			// sample size in bits (eg, 16)
	UInt32			sampleFreq;			// sample frequency
	UInt8			zeroes[6];			// AIFF defines the preceding as an 80-bit IEEE 754 fp number... most folks seem to use only 4 bytes
};

struct AIFFSoundChunk
{
	UInt32			ssnd;				// 'SSND'
	UInt32			ssndChunkSize;		// size of this chunk not including header (8 bytes + samples)
	UInt32			offset;				// used for aligning, usually zero
	UInt32			blockSize;			// used for aligning, usually zero
};

const UInt32 AIFF_SAMPLE_RATE_44_1		= 0x400EAC44;
const UInt32 AIFF_SAMPLE_RATE_44_0		= 0x400DAC44;

#if PRAGMA_STRUCT_ALIGN
	#pragma options align=reset
#elif PRAGMA_STRUCT_PACKPUSH
	#pragma pack(pop)
#elif PRAGMA_STRUCT_PACK
	#pragma pack()
#endif




AIFFFileInfo* parseFile(char* filepath)
{
	AIFFFileInfo*		info = (AIFFFileInfo*)malloc(sizeof(AIFFFileInfo));
	AIFFHeader			aiffHeader;
	AIFFCommonChunk		commonChunk;
	AIFFSoundChunk		soundChunk;
	IFFChunkHeader		chunkHeader;
	ssize_t				readLen;
	off_t				filepos;
	
	/* Initialize our file info to reasonable values. */
	info->numChannels = 0;
	info->sampleFreq = 0;
	info->sampleBits = 0;
	info->sampleBytes = 0;
	info->dataStart = 0;
	info->dataEnd = 0;
	info->cursor = 0;
	info->mask = 0;
	
	/* Get a c string of the file path so we can open up the file */
	info->fd = open(filepath, O_RDONLY, 0);
	if (info->fd == -1)
	{
		printf("file '%s' could not be opened. Aborting\n", filepath);
		exit(1);
	}
	/* Read in what should be the aiff file FORM header. Check for valid values. */
	readLen = read(info->fd, &aiffHeader, sizeof(aiffHeader));
	if (sizeof(aiffHeader) != readLen)
	{
		printf("file '%s' is an invalid AIFF. Aborting\n", filepath);
		exit(1);
	}
	
	if (aiffHeader.form != 'FORM' || aiffHeader.aiff != 'AIFF')
	{
		printf("file '%s' is an invalid AIFF. Aborting\n", filepath);
		exit(1);
	}


	/* OK, it's a valid AIFF file, search through for the common chunk. We do this
	   by reading in the "header" (my terminology) of a chunk. This will give us the 
	   chunkID and the size of the chunk. If it's not the chunk we want, we'll skip over
	   it to the next one. If the file's corrupted, we'll eventually hit EOF and 
	   fall out. */	
	filepos = lseek(info->fd, 12, SEEK_SET);
	chunkHeader.chunkSize = 0;
	do
	{
		filepos = lseek(info->fd, 0, SEEK_CUR);
		filepos = lseek(info->fd, filepos + chunkHeader.chunkSize, SEEK_SET);
		readLen = read(info->fd, &chunkHeader, sizeof(chunkHeader));
		if (sizeof(chunkHeader) != readLen)
		{
			printf("file '%s' is an invalid AIFF. Aborting\n", filepath);
			exit(1);
		}
	}
	while (chunkHeader.chunkID != 'COMM');


	filepos = lseek(info->fd, 0, SEEK_CUR);
	filepos = lseek(info->fd, filepos-sizeof(chunkHeader), SEEK_SET);
	readLen = read(info->fd, &commonChunk, sizeof(commonChunk));
	if (sizeof(commonChunk) != readLen)
	{
		printf("file '%s' is an invalid AIFF. Aborting\n", filepath);
		exit(1);
	}
	
	if (commonChunk.numSampleFrames == 0) {
		return noErr;		/* It's valid, just nothing we can really read from. We'll get a zero length track if we try. */
	}

	info->numChannels = commonChunk.numChannels;
	info->sampleFreq = commonChunk.sampleFreq;
	info->sampleBits = commonChunk.sampleSize;
	info->sampleBytes = (info->sampleBits + 7) / 8;

	if ((info->numChannels == 0) || (info->numChannels == 5) || (info->numChannels > 6))
	{
		printf("file '%s' is an invalid AIFF - wrong number of channels. Aborting\n", filepath);
		exit(1);
	}

	if (info->sampleFreq != AIFF_SAMPLE_RATE_44_1 && info->sampleFreq != AIFF_SAMPLE_RATE_44_0)
	{
		printf("file '%s' is an invalid AIFF - wrong sample rate. Aborting\n", filepath);
		exit(1);
	}

	if ((info->sampleBits < 1) || (info->sampleBits > 128))
	{
		printf("file '%s' is an invalid AIFF - sample bits are wrong. Aborting\n", filepath);
		exit(1);
	}


	/* go back the start of the file (remember chunks in an IFF file don't have to be in any order)
	   and look for the Sound chunk the same way we did the Common chunk. */
	filepos = lseek(info->fd, 12, SEEK_SET);
	chunkHeader.chunkSize = 0;
	do
	{
		filepos = lseek(info->fd, 0, SEEK_CUR);
		filepos = lseek(info->fd, filepos + chunkHeader.chunkSize, SEEK_SET);
		readLen = read(info->fd, &chunkHeader, sizeof(chunkHeader));
		if (sizeof(chunkHeader) != readLen)
		{
			printf("file '%s' is an invalid AIFF. Aborting\n", filepath);
			exit(1);
		}
	}
	while (chunkHeader.chunkID != 'SSND');

	filepos = lseek(info->fd, 0, SEEK_CUR);
	filepos = lseek(info->fd, filepos-sizeof(chunkHeader), SEEK_SET);
	readLen = read(info->fd, &soundChunk, sizeof(soundChunk));
	if (sizeof(soundChunk) != readLen)
	{
		printf("file '%s' is an invalid AIFF. Aborting\n", filepath);
		exit(1);
	}

	info->dataStart = lseek(info->fd, 0, SEEK_CUR) + soundChunk.offset;
	info->dataEnd = lseek(info->fd, 0, SEEK_CUR) + soundChunk.ssndChunkSize - 8;
	if ((soundChunk.ssndChunkSize < 8) || (info->dataEnd > lseek(info->fd, 0, SEEK_END)))
		info->dataEnd = lseek(info->fd, 0, SEEK_END);
		
	close(info->fd);
	
	return info;
}




