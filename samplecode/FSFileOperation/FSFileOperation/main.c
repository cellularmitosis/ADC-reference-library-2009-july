/*
	File:		main.c

	Abstract:	File copy engine tool which demonstrates the use of the FSFileOperation, and 
				FSCopyObject APIs.

	Version:	1.0

	Disclaimer:	IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc.
				("Apple") in consideration of your agreement to the following terms, and your
				use, installation, modification or redistribution of this Apple software
				constitutes acceptance of these terms.  If you do not agree with these terms,
				please do not use, install, modify or redistribute this Apple software.

				In consideration of your agreement to abide by the following terms, and subject
				to these terms, Apple grants you a personal, non-exclusive license, under Apple's
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

	Copyright � 2005 Apple Computer, Inc., All Rights Reserved
*/

#include <CoreServices/CoreServices.h>
#include <CoreFoundation/CoreFoundation.h>
#include <unistd.h>

static Boolean gDone;
static Boolean gVerbose;

static void statusCallbackPath(FSFileOperationRef fileOp, const char *currentItem, FSFileOperationStage stage, OSStatus error, CFDictionaryRef statusDictionary, void *info)
{
	if (gVerbose)
	{
		printf("statusCallback called: stage: %ld, Error: %ld\n", stage, error);
		if (currentItem)
			printf("\tcurrentItem: %s\n", currentItem);
		
		// attempt to get status from the dictionary
		if (statusDictionary)
		{
			CFNumberRef throughput, itemsCompleted, bytesCompleted, totalItems, totalBytes, itemsRemaining, bytesRemaining;
			
			throughput = (CFNumberRef) CFDictionaryGetValue(statusDictionary, kFSOperationThroughputKey);
			if (throughput)
			{
				fprintf(stderr, "\tthroughput: ");
				CFShow(throughput);
			}

			itemsCompleted = (CFNumberRef) CFDictionaryGetValue(statusDictionary, kFSOperationObjectsCompleteKey);
			if (itemsCompleted)
			{
				fprintf(stderr, "\titemsCompleted: ");
				CFShow(itemsCompleted);
			}

			bytesCompleted = (CFNumberRef) CFDictionaryGetValue(statusDictionary, kFSOperationBytesCompleteKey);
			if (bytesCompleted)
			{
				fprintf(stderr, "\tbytesCompleted: ");
				CFShow(bytesCompleted);
			}

			totalItems = (CFNumberRef) CFDictionaryGetValue(statusDictionary, kFSOperationTotalObjectsKey);
			if (totalItems)
			{
				fprintf(stderr, "\ttotalItems: ");
				CFShow(totalItems);
			}

			totalBytes = (CFNumberRef) CFDictionaryGetValue(statusDictionary, kFSOperationTotalBytesKey);
			if (totalBytes)
			{
				fprintf(stderr, "\ttotalBytes: ");
				CFShow(totalBytes);
			}

			itemsRemaining = (CFNumberRef) CFDictionaryGetValue(statusDictionary, kFSOperationObjectsRemainingKey);
			if (itemsRemaining)
			{
				fprintf(stderr, "\titemsRemaining: ");
				CFShow(itemsRemaining);
			}

			bytesRemaining = (CFNumberRef) CFDictionaryGetValue(statusDictionary, kFSOperationBytesRemainingKey);
			if (bytesRemaining)
			{
				fprintf(stderr, "\tbytesRemaining: ");
				CFShow(bytesRemaining);
			}
		}
		if (info)
		{
			CFStringRef theString = (CFStringRef)info;
			fprintf(stderr, "Info %lx\n", info);
			CFShow(theString);
		}
	}	
	if (stage == kFSOperationStageComplete)
		gDone = true;
}

static void statusCallback(FSFileOperationRef fileOp, const FSRef *currentItem, FSFileOperationStage stage, OSStatus error, CFDictionaryRef statusDictionary, void *info)
{
	if (gVerbose)
	{
		printf("statusCallback called: stage: %ld, Error: %ld\n", stage, error);
		if (currentItem)
			PrintFSRef(currentItem);
		
		// attempt to get status from the dictionary
		if (statusDictionary)
		{
			CFNumberRef throughput, itemsCompleted, bytesCompleted, totalItems, totalBytes, itemsRemaining, bytesRemaining;
			
			throughput = (CFNumberRef) CFDictionaryGetValue(statusDictionary, kFSOperationThroughputKey);
			if (throughput)
			{
				fprintf(stderr, "\tthroughput: ");
				CFShow(throughput);
			}

			itemsCompleted = (CFNumberRef) CFDictionaryGetValue(statusDictionary, kFSOperationObjectsCompleteKey);
			if (itemsCompleted)
			{
				fprintf(stderr, "\titemsCompleted: ");
				CFShow(itemsCompleted);
			}

			bytesCompleted = (CFNumberRef) CFDictionaryGetValue(statusDictionary, kFSOperationBytesCompleteKey);
			if (bytesCompleted)
			{
				fprintf(stderr, "\tbytesCompleted: ");
				CFShow(bytesCompleted);
			}

			totalItems = (CFNumberRef) CFDictionaryGetValue(statusDictionary, kFSOperationTotalObjectsKey);
			if (totalItems)
			{
				fprintf(stderr, "\ttotalItems: ");
				CFShow(totalItems);
			}

			totalBytes = (CFNumberRef) CFDictionaryGetValue(statusDictionary, kFSOperationTotalBytesKey);
			if (totalBytes)
			{
				fprintf(stderr, "\ttotalBytes: ");
				CFShow(totalBytes);
			}

			itemsRemaining = (CFNumberRef) CFDictionaryGetValue(statusDictionary, kFSOperationObjectsRemainingKey);
			if (itemsRemaining)
			{
				fprintf(stderr, "\titemsRemaining: ");
				CFShow(itemsRemaining);
			}

			bytesRemaining = (CFNumberRef) CFDictionaryGetValue(statusDictionary, kFSOperationBytesRemainingKey);
			if (bytesRemaining)
			{
				fprintf(stderr, "\tbytesRemaining: ");
				CFShow(bytesRemaining);
			}
		}
		
		if (info)
		{
			CFStringRef theString = (CFStringRef)info;
			fprintf(stderr, "Info:  0x%lx\n", info);
			CFShow(theString);
		}
		
	}	
	if (stage == kFSOperationStageComplete)
		gDone = true;
}

static OSStatus SetAsyncOpInMotion(Boolean copy, FSFileOperationRef fileOp, const FSRef *source, const FSRef *destDir, CFStringRef destName, OptionBits flags, FSFileOperationStatusProcPtr callback, CFTimeInterval statusChangeInterval, CFStringRef contextString)
{
	FSFileOperationClientContext	clientContext;
	OSStatus err;
	
	// The FSFileOperation will copy the data from the passed in clientContext so using
	// a stack based record that goes out of scope during the operation is fine.
	if (contextString)
	{
		clientContext.version = 0;
		clientContext.info = (void *) contextString;
		clientContext.retain = CFRetain;
		clientContext.release = CFRelease;
		clientContext.copyDescription = CFCopyDescription;
	}

	if (copy)
		err = FSCopyObjectAsync(fileOp, source, destDir, destName, flags, callback, statusChangeInterval, contextString != NULL ? &clientContext : NULL);
	else
		err = FSMoveObjectAsync(fileOp, source, destDir, destName, flags, callback, statusChangeInterval, contextString != NULL ? &clientContext : NULL);
		
	return err;
}

static OSStatus OperateByRef(Boolean copy, const char *sourcePath, const char *destPath, const char *destName, CFTimeInterval statusInterval, OptionBits options, Boolean async, CFStringRef contextString)
{

	FSRef   srcRef, destDirRef, target;
	OSStatus	err;
	CFRunLoopRef	runLoop;
	FSFileOperationRef  fileOp;
	CFStringRef	destString = NULL;
	
	gDone = false;
	err = FSPathMakeRef((UInt8 *)sourcePath, &srcRef, NULL);
	require_noerr(err, FSPathMakeRefsourceFailed);
	err = FSPathMakeRef((UInt8 *)destPath, &destDirRef, NULL);
	require_noerr(err, FSPathMakeRefdestFailed);
	
	if (destName)
	{
		destString = CFStringCreateWithCString(kCFAllocatorDefault, destName, kCFStringEncodingUTF8);
		if ((destString == NULL) && (gVerbose))
			fprintf(stderr, "Could not create destString for %s, using NULL\n", destName);
	}
	
	if (async)
	{
		runLoop = CFRunLoopGetCurrent();
		fileOp = FSFileOperationCreate(kCFAllocatorDefault);
		
		require(fileOp, FSFileOperationCreateFailed);
		
		err = FSFileOperationScheduleWithRunLoop(fileOp, runLoop, kCFRunLoopDefaultMode);
		require_noerr(err, FSFileOperationScheduleWithRunLoopFailed);
		
		err = SetAsyncOpInMotion(copy, fileOp, &srcRef, &destDirRef, destString, options, statusCallback, statusInterval, contextString);  

		// The operation will retain the data it requires, so go ahead andrelease
		// the contextString and destString now if they were provided
		if (contextString)
			CFRelease(contextString);
		
		if (destString)
			CFRelease(destString);
		
		require_noerr(err, FSXXXXObjectAsyncFailed);

		// This is a wait loop, since the tool does not have anything better to do.
		// In a real application there should be plenty of better things to do.
		// Make sure to run the runloop that the operation is scheduled on in the
		// specified mode. 
		while (!gDone)
			CFRunLoopRunInMode(kCFRunLoopDefaultMode, 5.0, true);

		(void) FSFileOperationCopyStatus(fileOp, &target, NULL, &err, NULL, NULL);
		
	FSXXXXObjectAsyncFailed:
	FSFileOperationScheduleWithRunLoopFailed:
		CFRelease(fileOp);
	}
	else
	{
		// contextString does not apply to the sync call, so release it now
		if (contextString)
			CFRelease(contextString);

		if (copy)
			err = FSCopyObjectSync(&srcRef, &destDirRef, destString, &target, options);
		else
			err = FSMoveObjectSync(&srcRef, &destDirRef, destString, &target, options);

		if (destString)
			CFRelease(destString);
	}
	
	if ((err == noErr) && (gVerbose))
	{
		printf("target: ");
		PrintFSRef(&target);
	}
	
FSFileOperationCreateFailed:
FSPathMakeRefsourceFailed:
FSPathMakeRefdestFailed:
	return err;
}

static OSStatus SetAsyncPathOpInMotion(Boolean copy, FSFileOperationRef fileOp, const char *sourcePath, const char *destDirPath, CFStringRef destName, OptionBits flags, FSPathFileOperationStatusProcPtr callback, CFTimeInterval statusChangeInterval, CFStringRef contextString)
{
	FSFileOperationClientContext	clientContext;
	OSStatus err;
	
	// The FSFileOperation will copy the data from the passed in clientContext so using
	// a stack based record that goes out of scope during the operation is fine.
	if (contextString)
	{
		clientContext.version = 0;
		clientContext.info = (void *) contextString;
		clientContext.retain = CFRetain;
		clientContext.release = CFRelease;
		clientContext.copyDescription = CFCopyDescription;
	}

	if (copy)
		err = FSPathCopyObjectAsync(fileOp, sourcePath, destDirPath, destName, flags, callback, statusChangeInterval, contextString != NULL ? &clientContext : NULL);
	else
		err = FSPathMoveObjectAsync(fileOp, sourcePath, destDirPath, destName, flags, callback, statusChangeInterval, contextString != NULL ? &clientContext : NULL);
		
	return err;
}

static OSStatus OperateByPath(Boolean copy, const char *sourcePath, const char *destPath, const char *destName, CFTimeInterval statusInterval, OptionBits options, Boolean async, CFStringRef contextString)
{

	OSStatus	err;
	CFRunLoopRef	runLoop;
	FSFileOperationRef  fileOp;
	CFStringRef	destString = NULL;
	char *		targetPath;
	
	gDone = false;
	
	if (destName)
	{
		destString = CFStringCreateWithCString(kCFAllocatorDefault, destName, kCFStringEncodingUTF8);
		if ((destString == NULL) && (gVerbose))
			fprintf(stderr, "Could not create destString for %s, using NULL\n", destName);
	}
	
	if (async)
	{
		runLoop = CFRunLoopGetCurrent();
		fileOp = FSFileOperationCreate(kCFAllocatorDefault);
		
		require(fileOp, FSFileOperationCreateFailed);
		
		err = FSFileOperationScheduleWithRunLoop(fileOp, runLoop, kCFRunLoopDefaultMode);
		require_noerr(err, FSFileOperationScheduleWithRunLoopFailed);
		

		err = SetAsyncPathOpInMotion(copy, fileOp, sourcePath, destPath, destString, options, statusCallbackPath, statusInterval, contextString);

		// The operation will retain the data it requires, so go ahead andrelease
		// the contextString and destString now if they were provided
		if (contextString)
			CFRelease(contextString);

		if (destString)
			CFRelease(destString);
		require_noerr(err, FSXXXXObjectAsyncFailed);
		
			
		// This is a wait loop, since the tool does not have anything better to do.
		// In a real application there should be plenty of better things to do.
		// Make sure to run the runloop that the operation is scheduled on in the
		// specified mode. 
		while (!gDone)
			CFRunLoopRunInMode(kCFRunLoopDefaultMode, 5.0, true);
	
		(void) FSPathFileOperationCopyStatus(fileOp, &targetPath, NULL, &err, NULL, NULL);
		
	FSXXXXObjectAsyncFailed:
	FSFileOperationScheduleWithRunLoopFailed:
		CFRelease(fileOp);
	}
	else
	{
		// contextString does not apply to the sync call, so release it now
		if (contextString)
			CFRelease(contextString);

		if (copy)
			err = FSPathCopyObjectSync(sourcePath, destPath, destString, &targetPath, options);
		else
			err = FSPathMoveObjectSync(sourcePath, destPath, destString, &targetPath, options);

		if (destString)
			CFRelease(destString);
	}
	
	if ((err == noErr) && (gVerbose) && (targetPath))
	{
		printf("targetPath: %s\n", targetPath);
		free(targetPath);
	}
FSFileOperationCreateFailed:
	return err;
}

static void usage(const char *progname)
{
	printf("Usage: %s -s sourcePath -d destPath [-i statusInterval] [-p] [-n destinationName] [-f infoString] [-a] [-m] [-v] [-o] [-w] [-c] [-e]\n",progname);
	printf("\tsourcePath: utf-8 path to source file or directory\n\tdestPath: utf-8 path to destination directory\n\tstatusInterval: minimum number of seconds between status callbacks.  Default is 1.0\n");
	printf("\t-p uses path based call.  Default is to use FSRef based call\n\tdestinationName: utf8 string specifying new name for source item in destination directory.  Default is source item name.\n");
	printf("\t-f infoString will pass a clientContext with a CFString created from infoString\t-a uses async call.  Default is sync\n\t-v verbose mode: print more status info\n\t-m move instead of copy.\n");
	printf("\t-o kFSFileOperationOverwrite, -w kFSFileOperationSkipSourcePermissionErrors, -c kFSFileOperationDoNotMoveAcrossVolumes, -e kFSFileOperationSkipPreflight.  Default is kFSFileOperationDefaultOptions.\n");
	exit(1);
}   

int main (int argc, char * const * argv) 
{
	char ch;
	Boolean byPath = false, async = false;
	char *sourcePath, *destDirPath, *destName;
	CFTimeInterval  statusInterval = 1.0;
	OSStatus	err;
	OptionBits	options = kFSFileOperationDefaultOptions;
	Boolean copy;
	CFStringRef contextString = NULL;
	
	sourcePath = destDirPath = destName = NULL;
	gVerbose = false;
	copy = true;
	
	while ((ch = getopt(argc, argv, "s:d:i:pn:f:owceavm")) != -1)
		switch (ch)
		{
			case 's':
				sourcePath = optarg;
				break;
			case 'd':
				destDirPath = optarg;
				break;
			case 'i':
				statusInterval = atof(optarg);
				break;
			case 'p':
				byPath = true;
				break;
			case 'n':
				destName = optarg;
				break;
			case 'a':
				async = true;
				break;
			case 'o':
				options |= kFSFileOperationOverwrite;
				break;
			case 'w':
				options |= kFSFileOperationSkipSourcePermissionErrors;
				break;
			case 'c':
				options |= kFSFileOperationDoNotMoveAcrossVolumes;
				break;
			case 'e':
				options |= kFSFileOperationSkipPreflight;
				break;
			case 'v':
				gVerbose = true;
				break;
			case 'm':
				copy = false;
				break;
			case 'f':
				contextString = CFStringCreateWithCString(NULL, optarg, kCFStringEncodingUTF8);
				break;
			case '?':
			default:
				usage(argv[0]);
		}
		 
	if ((sourcePath == NULL) || (destDirPath == NULL))
		usage(argv[0]);
	
	if (copy)
		printf("Copying");
	else
		printf("Moving");
		
	printf(" %s to %s" ,sourcePath,destDirPath);
	
	if (async)
		printf(" with statusInterval of %g", statusInterval);
		
	if (destName)
		printf(" with destName %s", destName);

	if (options == kFSFileOperationDefaultOptions)
		printf(" using kFSFileOperationDefaultOptions");
	else
	{
		printf(" with");
		if (options & kFSFileOperationOverwrite)
			printf(" kFSFileOperationOverwrite");
		if (options & kFSFileOperationSkipSourcePermissionErrors)
			printf(" kFSFileOperationSkipSourcePermissionErrors");
		if (options & kFSFileOperationDoNotMoveAcrossVolumes)
			printf(" kFSFileOperationDoNotMoveAcrossVolumes");
		if (options & kFSFileOperationSkipPreflight)
			printf(" kFSFileOperationSkipPreflight");
		printf(" set");
	}
	
	if (gVerbose)
		printf(" in verbose mode");
		
	if (contextString)
		printf(" with a client context");

	if (async)
		printf(" using async");
	else
		printf(" using sync");

	if (byPath)
		printf(" path based call.\n");
	else
		printf(" ref based call.\n");
		
	if (byPath)
		err = OperateByPath(copy, sourcePath, destDirPath, destName, statusInterval, options, async, contextString);
	else
		err = OperateByRef(copy, sourcePath, destDirPath, destName, statusInterval, options, async, contextString);
	
	if (gVerbose)
	{
		if (err == noErr)
			printf("returning noErr\n");
	}
	if (err != noErr)
		printf("Error: %s (%d)\n", GetMacOSStatusErrorString(err), err);
	return err;
}
