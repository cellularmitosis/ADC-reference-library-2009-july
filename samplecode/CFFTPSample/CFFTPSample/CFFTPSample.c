/*
    File:  CFFTPSample.c

    Abstract:  This file shows how to use the CFFTPStream API to download and
    upload files using FTP, and also shows how to parse FTP directory listings.

    Version:  1.2

    (c) Copyright 2006 Apple Computer, Inc. All rights reserved.

    IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc.
    ("Apple") in consideration of your agreement to the following terms, and
    your use, installation, modification or redistribution of this Apple
    software constitutes acceptance of these terms.  If you do not agree with
    these terms, please do not use, install, modify or redistribute this Apple
    software.

    In consideration of your agreement to abide by the following terms, and
    subject to these terms, Apple grants you a personal, non-exclusive license,
    under Apple's copyrights in this original Apple software (the "Apple
    Software"), to use, reproduce, modify and redistribute the Apple Software,
    with or without modifications, in source and/or binary forms; provided that
    if you redistribute the Apple Software in its entirety and without
    modifications, you must retain this notice and the following text and
    disclaimers in all such redistributions of the Apple Software. Neither the
    name, trademarks, service marks or logos of Apple Computer, Inc. may be used
    to endorse or promote products derived from the Apple Software without
    specific prior written permission from Apple.  Except as expressly stated in
    this notice, no other rights or licenses, express or implied, are granted by
    Apple herein, including but not limited to any patent rights that may be
    infringed by your derivative works or by other works in which the Apple
    Software may be incorporated.

    The Apple Software is provided by Apple on an "AS IS" basis.  APPLE MAKES NO
    WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED
    WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS FOR A PARTICULAR
    PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND OPERATION ALONE OR IN
    COMBINATION WITH YOUR PRODUCTS.

    IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL OR
    CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
    SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
    INTERRUPTION) ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION, MODIFICATION
    AND/OR DISTRIBUTION OF THE APPLE SOFTWARE, HOWEVER CAUSED AND WHETHER UNDER
    THEORY OF CONTRACT, TORT (INCLUDING NEGLIGENCE), STRICT LIABILITY OR
    OTHERWISE, EVEN IF APPLE HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
              
    Change History (most recent first):
        1.2      May 2, 2006
        1.1    July 25, 2005
        1.0   August 6, 2004
*/

#include <CoreServices/CoreServices.h>
#include <CoreFoundation/CoreFoundation.h>
#include <SystemConfiguration/SystemConfiguration.h>

#include <sys/dirent.h>
#include <sys/stat.h>
#include <unistd.h>         // getopt
#include <string.h>         // strmode
#include <stdlib.h>
#include <inttypes.h>


#pragma mark ***** Common Code and Data Structures


/* When using file streams, the 32KB buffer is probably not enough.
A good way to establish a buffer size is to increase it over time.
If every read consumes the entire buffer, start increasing the buffer
size, and at some point you would then cap it. 32KB is fine for network
sockets, although using the technique described above is still a good idea.
This sample avoids the technique because of the added complexity it
would introduce. */
#define kMyBufferSize  32768


/* MyStreamInfo holds the state of a particular operation (download, upload, or 
directory listing.  Some fields are only valid for some operations, as explained 
by their comments. */
typedef struct MyStreamInfo {

    CFWriteStreamRef  writeStream;              // download (destination file stream) and upload (FTP stream) only
    CFReadStreamRef   readStream;               // download (FTP stream), upload (source file stream), directory list (FTP stream)
    CFDictionaryRef   proxyDict;                // necessary to workaround <rdar://problem/3745574>, per discussion below
    SInt64            fileSize;                 // download only, 0 indicates unknown
    UInt32            totalBytesWritten;        // download and upload only
    UInt32            leftOverByteCount;        // upload and directory list only, number of valid bytes at start of buffer
    UInt8             buffer[kMyBufferSize];    // buffer to hold left over bytes

} MyStreamInfo;


static const CFOptionFlags kNetworkEvents = 
      kCFStreamEventOpenCompleted
    | kCFStreamEventHasBytesAvailable
    | kCFStreamEventEndEncountered
    | kCFStreamEventCanAcceptBytes
    | kCFStreamEventErrorOccurred;
    

/* MyStreamInfoCreate creates a MyStreamInfo 'object' with the specified read and write stream. */
static void
MyStreamInfoCreate(MyStreamInfo ** info, CFReadStreamRef readStream, CFWriteStreamRef writeStream)
{
    MyStreamInfo * streamInfo;

    assert(info != NULL);
    assert(readStream != NULL);
    // writeStream may be NULL (this is the case for the directory list operation)
    
    streamInfo = malloc(sizeof(MyStreamInfo));
    assert(streamInfo != NULL);
    
    streamInfo->readStream        = readStream;
    streamInfo->writeStream       = writeStream;
    streamInfo->proxyDict         = NULL;           // see discussion of <rdar://problem/3745574> below
    streamInfo->fileSize          = 0;
    streamInfo->totalBytesWritten = 0;
    streamInfo->leftOverByteCount = 0;

    *info = streamInfo;
}


/* MyStreamInfoDestroy destroys a MyStreamInfo 'object', cleaning up any resources that it owns. */                                       
static void
MyStreamInfoDestroy(MyStreamInfo * info)
{
    assert(info != NULL);
    
    if (info->readStream) {
        CFReadStreamUnscheduleFromRunLoop(info->readStream, CFRunLoopGetCurrent(), kCFRunLoopCommonModes);
        (void) CFReadStreamSetClient(info->readStream, kCFStreamEventNone, NULL, NULL);
        
        /* CFReadStreamClose terminates the stream. */
        CFReadStreamClose(info->readStream);
        CFRelease(info->readStream);
    }

    if (info->writeStream) {
        CFWriteStreamUnscheduleFromRunLoop(info->writeStream, CFRunLoopGetCurrent(), kCFRunLoopCommonModes);
        (void) CFWriteStreamSetClient(info->writeStream, kCFStreamEventNone, NULL, NULL);
        
        /* CFWriteStreamClose terminates the stream. */
        CFWriteStreamClose(info->writeStream);
        CFRelease(info->writeStream);
    }

    if (info->proxyDict) {
        CFRelease(info->proxyDict);             // see discussion of <rdar://problem/3745574> below
    }
    
    free(info);
}


/* MyCFStreamSetUsernamePassword applies the specified user name and password to the stream. */
static void
MyCFStreamSetUsernamePassword(CFTypeRef stream, CFStringRef username, CFStringRef password)
{
    Boolean success;
    assert(stream != NULL);
    assert( (username != NULL) || (password == NULL) );
    
    if (username && CFStringGetLength(username) > 0) {

        if (CFGetTypeID(stream) == CFReadStreamGetTypeID()) {
            success = CFReadStreamSetProperty((CFReadStreamRef)stream, kCFStreamPropertyFTPUserName, username);
            assert(success);
            if (password) {
                success = CFReadStreamSetProperty((CFReadStreamRef)stream, kCFStreamPropertyFTPPassword, password);
                assert(success);
            }
        } else if (CFGetTypeID(stream) == CFWriteStreamGetTypeID()) {
            success = CFWriteStreamSetProperty((CFWriteStreamRef)stream, kCFStreamPropertyFTPUserName, username);
            assert(success);
            if (password) {
                success = CFWriteStreamSetProperty((CFWriteStreamRef)stream, kCFStreamPropertyFTPPassword, password);
                assert(success);
            }
        } else {
            assert(false);
        }
    }
}


/* MyCFStreamSetFTPProxy applies the current proxy settings to the specified stream.  This returns a 
reference to the proxy dictionary that we used because of <rdar://problem/3745574>, discussed below. */
static void
MyCFStreamSetFTPProxy(CFTypeRef stream, CFDictionaryRef * proxyDictPtr)
{
    CFDictionaryRef  proxyDict;
    CFNumberRef      passiveMode;
    CFBooleanRef     isPassive;
    Boolean          success;
    
    assert(stream != NULL);
    assert(proxyDictPtr != NULL);
    
    /* SCDynamicStoreCopyProxies gets the current Internet proxy settings.  Then we call
    CFReadStreamSetProperty, with property name kCFStreamPropertyFTPProxy, to apply the
    settings to the FTP read stream. */
    proxyDict = SCDynamicStoreCopyProxies(NULL);
    assert(proxyDict != NULL);    
    
    /* Get the FTP passive mode setting from the proxy dictionary.  Because of a bug <rdar://problem/3625438>
    setting the kCFStreamPropertyFTPProxy property using the SCDynamicStore proxy dictionary does not
    currently set the FTP passive mode setting on the stream, so we need to do it ourselves. 
    Also, <rdar://problem/4526438> indicates that out in the real world some people are setting 
    kSCPropNetProxiesFTPPassive to a Boolean, as opposed to a number.  That's just incorrect, 
    but I've hardened the code against it. */
    passiveMode = CFDictionaryGetValue(proxyDict, kSCPropNetProxiesFTPPassive);
    if ( (passiveMode != NULL) && (CFGetTypeID(passiveMode) == CFNumberGetTypeID()) ) {
        int         value;
        
        success = CFNumberGetValue(passiveMode, kCFNumberIntType, &value);
        assert(success);
        
        if (value) isPassive = kCFBooleanTrue;
        else isPassive = kCFBooleanFalse;
    } else {
        assert(false);
        isPassive = kCFBooleanTrue;         // if prefs malformed, we just assume true
    }
        
    if (CFGetTypeID(stream) == CFReadStreamGetTypeID()) {
        success = CFReadStreamSetProperty((CFReadStreamRef)stream, kCFStreamPropertyFTPProxy, proxyDict);
        assert(success);
        success = CFReadStreamSetProperty((CFReadStreamRef)stream, kCFStreamPropertyFTPUsePassiveMode, isPassive);
        assert(success);
    } else if (CFGetTypeID(stream) == CFWriteStreamGetTypeID()) {
        success = CFWriteStreamSetProperty((CFWriteStreamRef)stream, kCFStreamPropertyFTPProxy, proxyDict);
        assert(success);
        success = CFWriteStreamSetProperty((CFWriteStreamRef)stream, kCFStreamPropertyFTPUsePassiveMode, isPassive);
        assert(success);
    } else {
        fprintf(stderr, "This is not a CFStream\n");
    }

    /* Prior to Mac OS X 10.4, CFFTPStream has a bug <rdar://problem/3745574> that causes it to reference the 
    proxy dictionary that you applied /after/ it has released its last reference to that dictionary.  This causes 
    a crash.  We work around this bug by holding on to our own reference to the proxy dictionary until we're 
    done with the stream.  Thus, our reference prevents the dictionary from being disposed, and thus CFFTPStream 
    can access it safely.  So, rather than release our reference to the proxy dictionary, we pass it back to 
    our caller and require it to release it. */
    
    // CFRelease(proxyDict);  After bug #3745574 is fixed, we'll be able to release the proxyDict here.
    
    *proxyDictPtr = proxyDict;
}


#pragma mark ***** Download Command


/* MyDownloadCallBack is the stream callback for the CFFTPStream during a download operation. 
Its main purpose is to read bytes off the FTP stream for the file being downloaded and write 
them to the file stream of the destination file. */
static void
MyDownloadCallBack(CFReadStreamRef readStream, CFStreamEventType type, void * clientCallBackInfo)
{
    MyStreamInfo      *info = (MyStreamInfo *)clientCallBackInfo;
    CFIndex           bytesRead = 0, bytesWritten = 0;
    CFStreamError     error;
    CFNumberRef       cfSize;
    SInt64            size;
    float             progress;
    
    assert(readStream != NULL);
    assert(info       != NULL);
    assert(info->readStream == readStream);

    switch (type) {

        case kCFStreamEventOpenCompleted:
            /* Retrieve the file size from the CFReadStream. */
            cfSize = CFReadStreamCopyProperty(info->readStream, kCFStreamPropertyFTPResourceSize);
            
            fprintf(stderr, "Open complete\n");
            
            if (cfSize) {
                if (CFNumberGetValue(cfSize, kCFNumberSInt64Type, &size)) {
                    fprintf(stderr, "File size is %" PRId64 "\n", size);
                    info->fileSize = size;
                }
                CFRelease(cfSize);
            } else {
                fprintf(stderr, "File size is unknown\n");
                assert(info->fileSize == 0);            // It was set up this way by MyStreamInfoCreate.
            }
            break;
        case kCFStreamEventHasBytesAvailable:

            /* CFReadStreamRead will return the number of bytes read, or -1 if an error occurs
            preventing any bytes from being read, or 0 if the stream's end was encountered. */
            bytesRead = CFReadStreamRead(info->readStream, info->buffer, kMyBufferSize);
            if (bytesRead > 0) {
                /* Just in case we call CFWriteStreamWrite and it returns without writing all
                the data, we loop until all the data is written successfully.  Since we're writing
                to the "local" file system, it's unlikely that CFWriteStreamWrite will return before
                writing all the data, but it would be bad if we simply exited the callback because
                we'd be losing some of the data that we downloaded. */
                bytesWritten = 0;
                while (bytesWritten < bytesRead) {
                    CFIndex result;

                    result = CFWriteStreamWrite(info->writeStream, info->buffer + bytesWritten, bytesRead - bytesWritten);
                    if (result <= 0) {
                        fprintf(stderr, "CFWriteStreamWrite returned %ld\n", result);
                        goto exit;
                    }
                    bytesWritten += result;
                }
                info->totalBytesWritten += bytesWritten;
            } else {
                /* If bytesRead < 0, we've hit an error.  If bytesRead == 0, we've hit the end of the file.  
                In either case, we do nothing, and rely on CF to call us with kCFStreamEventErrorOccurred 
                or kCFStreamEventEndEncountered in order for us to do our clean up. */
            }
            
            if (info->fileSize > 0) {
                progress = 100*((float)info->totalBytesWritten/(float)info->fileSize);
                fprintf(stderr, "\r%.0f%%", progress);
            }
            break;
        case kCFStreamEventErrorOccurred:
            error = CFReadStreamGetError(info->readStream);
            fprintf(stderr, "CFReadStreamGetError returned (%d, %ld)\n", error.domain, error.error);
            goto exit;
        case kCFStreamEventEndEncountered:
            fprintf(stderr, "\nDownload complete\n");
            goto exit;
        default:
            fprintf(stderr, "Received unexpected CFStream event (%d)", type);
            break;
    }
    return;
    
exit:    
    MyStreamInfoDestroy(info);
    CFRunLoopStop(CFRunLoopGetCurrent());
    return;
}


/* MySimpleDownload implements the download command.  It sets up a MyStreamInfo 'object' 
with the read stream being an FTP stream of the file to download and the write stream being 
a file stream of the destination file.  It then returns, and the real work happens 
asynchronously in the runloop.  The function returns true if the stream setup succeeded, 
and false if it failed. */
static Boolean
MySimpleDownload(CFStringRef urlString, CFURLRef destinationFolder, CFStringRef username, CFStringRef password)
{
    CFReadStreamRef        readStream;
    CFWriteStreamRef       writeStream;
    CFStreamClientContext  context = { 0, NULL, NULL, NULL, NULL };
    CFURLRef               downloadPath, downloadURL;
    CFStringRef            fileName;
    Boolean                dirPath, success = true;
    MyStreamInfo           *streamInfo;

    assert(urlString != NULL);
    assert(destinationFolder != NULL);
    assert( (username != NULL) || (password == NULL) );

    /* Returns true if the CFURL path represents a directory. */
    dirPath = CFURLHasDirectoryPath(destinationFolder);
    if (!dirPath) {
        fprintf(stderr, "Download destination must be a directory.\n");
        return false;
    }
    
    /* Create a CFURL from the urlString. */
    downloadURL = CFURLCreateWithString(kCFAllocatorDefault, urlString, NULL);
    assert(downloadURL != NULL);

    /* Copy the end of the file path and use it as the file name. */
    fileName = CFURLCopyLastPathComponent(downloadURL);
    assert(fileName != NULL);

    /* Create the downloadPath by taking the destination folder and appending the file name. */
    downloadPath = CFURLCreateCopyAppendingPathComponent(kCFAllocatorDefault, destinationFolder, fileName, false);
    assert(downloadPath != NULL);
    CFRelease(fileName);

    /* Create a CFWriteStream for the file being downloaded. */
    writeStream = CFWriteStreamCreateWithFile(kCFAllocatorDefault, downloadPath);
    assert(writeStream != NULL);
    CFRelease(downloadPath);
    
    /* CFReadStreamCreateWithFTPURL creates an FTP read stream for downloading from an FTP URL. */
    readStream = CFReadStreamCreateWithFTPURL(kCFAllocatorDefault, downloadURL);
    assert(readStream != NULL);
    CFRelease(downloadURL);
    
    /* Initialize our MyStreamInfo structure, which we use to store some information about the stream. */
    MyStreamInfoCreate(&streamInfo, readStream, writeStream);
    context.info = (void *)streamInfo;

    /* CFWriteStreamOpen will return success/failure.  Opening a stream causes it to reserve all the
    system resources it requires.  If the stream can open non-blocking, this will always return TRUE;
    listen to the run loop source to find out when the open completes and whether it was successful. */
    success = CFWriteStreamOpen(writeStream);
    if (success) {
    
        /* CFReadStreamSetClient registers a callback to hear about interesting events that occur on a stream. */
        success = CFReadStreamSetClient(readStream, kNetworkEvents, MyDownloadCallBack, &context);
        if (success) {

            /* Schedule a run loop on which the client can be notified about stream events.  The client
            callback will be triggered via the run loop.  It's the caller's responsibility to ensure that
            the run loop is running. */
            CFReadStreamScheduleWithRunLoop(readStream, CFRunLoopGetCurrent(), kCFRunLoopCommonModes);
            
            MyCFStreamSetUsernamePassword(readStream, username, password);
            MyCFStreamSetFTPProxy(readStream, &streamInfo->proxyDict);
            
            /* Setting the kCFStreamPropertyFTPFetchResourceInfo property will instruct the FTP stream
            to fetch the file size before downloading the file.  Note that fetching the file size adds
            some time to the length of the download.  Fetching the file size allows you to potentially
            provide a progress dialog during the download operation. You will retrieve the actual file
            size after your CFReadStream Callback gets called with a kCFStreamEventOpenCompleted event. */
            CFReadStreamSetProperty(readStream, kCFStreamPropertyFTPFetchResourceInfo, kCFBooleanTrue);
            
            /* CFReadStreamOpen will return success/failure.  Opening a stream causes it to reserve all the
            system resources it requires.  If the stream can open non-blocking, this will always return TRUE;
            listen to the run loop source to find out when the open completes and whether it was successful. */
            success = CFReadStreamOpen(readStream);
            if (success == false) {
                fprintf(stderr, "CFReadStreamOpen failed\n");
                MyStreamInfoDestroy(streamInfo);
            }
        } else {
            fprintf(stderr, "CFReadStreamSetClient failed\n");
            MyStreamInfoDestroy(streamInfo);
        }
    } else {
        fprintf(stderr, "CFWriteStreamOpen failed\n");
        MyStreamInfoDestroy(streamInfo);
    }

    return success;
}


#pragma mark ***** Upload Command


/* MyUploadCallBack is the stream callback for the CFFTPStream during an upload operation. 
Its main purpose is to wait for space to become available in the FTP stream (the write stream), 
and then read bytes from the file stream (the read stream) and write them to the FTP stream. */
static void
MyUploadCallBack(CFWriteStreamRef writeStream, CFStreamEventType type, void * clientCallBackInfo)
{
    MyStreamInfo     *info = (MyStreamInfo *)clientCallBackInfo;
    CFIndex          bytesRead;
    CFIndex          bytesAvailable;
    CFIndex          bytesWritten;
    CFStreamError    error;
    
    assert(writeStream != NULL);
    assert(info        != NULL);
    assert(info->writeStream == writeStream);

    switch (type) {

        case kCFStreamEventOpenCompleted:
            fprintf(stderr, "Open complete\n");
            break;
        case kCFStreamEventCanAcceptBytes:

            /* The first thing we do is check to see if there's some leftover data that we read
            in a previous callback, which we were unable to upload for whatever reason. */
            if (info->leftOverByteCount > 0) {
                bytesRead = 0;
                bytesAvailable = info->leftOverByteCount;
            } else {
                /* If not, we try to read some more data from the file.  CFReadStreamRead will 
                return the number of bytes read, or -1 if an error occurs preventing 
                any bytes from being read, or 0 if the stream's end was encountered. */
                bytesRead = CFReadStreamRead(info->readStream, info->buffer, kMyBufferSize);
                if (bytesRead < 0) {
                    fprintf(stderr, "CFReadStreamRead returned %ld\n", bytesRead);
                    goto exit;
                }
                bytesAvailable = bytesRead;
            }
            bytesWritten = 0;
            
            if (bytesAvailable == 0) {
                /* We've hit the end of the file being uploaded.  Shut everything down. 
                Previous versions of this sample would terminate the upload stream 
                by writing zero bytes to the stream.  After discussions with CF engineering, 
                we've decided that it's better to terminate the upload stream by just 
                closing the stream. */
                fprintf(stderr, "\nEnd up uploaded file; closing down\n");
                goto exit;
            } else {

                /* CFWriteStreamWrite returns the number of bytes successfully written, -1 if an error has
                occurred, or 0 if the stream has been filled to capacity (for fixed-length streams).
                If the stream is not full, this call will block until at least one byte is written. 
                However, as we're in the kCFStreamEventCanAcceptBytes callback, we know that at least 
                one byte can be written, so we won't block. */

                bytesWritten = CFWriteStreamWrite(info->writeStream, info->buffer, bytesAvailable);
                if (bytesWritten > 0) {

                    info->totalBytesWritten += bytesWritten;
                    
                    /* If we couldn't upload all the data that we read, we temporarily store the data in our MyStreamInfo
                    context until our CFWriteStream callback is called again with a kCFStreamEventCanAcceptBytes event. 
                    Copying the data down inside the buffer is not the most efficient approach, but it makes the code 
                    significantly easier. */
                    if (bytesWritten < bytesAvailable) {
                        info->leftOverByteCount = bytesAvailable - bytesWritten;
                        memmove(info->buffer, info->buffer + bytesWritten, info->leftOverByteCount);
                    } else {
                        info->leftOverByteCount = 0;
                    }
                } else if (bytesWritten < 0) {
                    fprintf(stderr, "CFWriteStreamWrite returned %ld\n", bytesWritten);
                    /* If CFWriteStreamWrite failed, the write stream is dead.  We will clean up 
                    when we get kCFStreamEventErrorOccurred. */
                }
            }
            
            /* Print a status update if we made any forward progress. */
            if ( (bytesRead > 0) || (bytesWritten > 0) ) {
                fprintf(stderr, "\rRead %7ld bytes; Wrote %8ld bytes", bytesRead, info->totalBytesWritten);
            }
            break;
        case kCFStreamEventErrorOccurred:
            error = CFWriteStreamGetError(info->writeStream);
            fprintf(stderr, "CFReadStreamGetError returned (%d, %ld)\n", error.domain, error.error);
            goto exit;
        case kCFStreamEventEndEncountered:
            fprintf(stderr, "\nUpload complete\n");
            goto exit;
        default:
            fprintf(stderr, "Received unexpected CFStream event (%d)", type);
            break;
    }
    return;
    
exit:
    MyStreamInfoDestroy(info);
    CFRunLoopStop(CFRunLoopGetCurrent());
    return;
}



/* MySimpleUpload implements the upload command.  It sets up a MyStreamInfo 'object' 
with the read stream being a file stream of the file to upload and the write stream being 
an FTP stream of the destination file.  It then returns, and the real work happens 
asynchronously in the runloop.  The function returns true if the stream setup succeeded, 
and false if it failed. */
static Boolean
MySimpleUpload(CFStringRef uploadDirectory, CFURLRef fileURL, CFStringRef username, CFStringRef password)
{
    CFWriteStreamRef       writeStream;
    CFReadStreamRef        readStream;
    CFStreamClientContext  context = { 0, NULL, NULL, NULL, NULL };
    CFURLRef               uploadURL, destinationURL;
    CFStringRef            fileName;
    Boolean                success = true;
    MyStreamInfo           *streamInfo;

    assert(uploadDirectory != NULL);
    assert(fileURL != NULL);
    assert( (username != NULL) || (password == NULL) );
    
    /* Create a CFURL from the upload directory string */
    destinationURL = CFURLCreateWithString(kCFAllocatorDefault, uploadDirectory, NULL);
    assert(destinationURL != NULL);

    /* Copy the end of the file path and use it as the file name. */
    fileName = CFURLCopyLastPathComponent(fileURL);
    assert(fileName != NULL);

    /* Create the destination URL by taking the upload directory and appending the file name. */
    uploadURL = CFURLCreateCopyAppendingPathComponent(kCFAllocatorDefault, destinationURL, fileName, false);
    assert(uploadURL != NULL);
    CFRelease(destinationURL);
    CFRelease(fileName);
    
    /* Create a CFReadStream from the local file being uploaded. */
    readStream = CFReadStreamCreateWithFile(kCFAllocatorDefault, fileURL);
    assert(readStream != NULL);
    
    /* Create an FTP write stream for uploading operation to a FTP URL. If the URL specifies a
    directory, the open will be followed by a close event/state and the directory will have been
    created. Intermediary directory structure is not created. */
    writeStream = CFWriteStreamCreateWithFTPURL(kCFAllocatorDefault, uploadURL);
    assert(writeStream != NULL);
    CFRelease(uploadURL);
    
    /* Initialize our MyStreamInfo structure, which we use to store some information about the stream. */
    MyStreamInfoCreate(&streamInfo, readStream, writeStream);
    context.info = (void *)streamInfo;

    /* CFReadStreamOpen will return success/failure.  Opening a stream causes it to reserve all the
    system resources it requires.  If the stream can open non-blocking, this will always return TRUE;
    listen to the run loop source to find out when the open completes and whether it was successful. */
    success = CFReadStreamOpen(readStream);
    if (success) {
        
        /* CFWriteStreamSetClient registers a callback to hear about interesting events that occur on a stream. */
        success = CFWriteStreamSetClient(writeStream, kNetworkEvents, MyUploadCallBack, &context);
        if (success) {

            /* Schedule a run loop on which the client can be notified about stream events.  The client
            callback will be triggered via the run loop.  It's the caller's responsibility to ensure that
            the run loop is running. */
            CFWriteStreamScheduleWithRunLoop(writeStream, CFRunLoopGetCurrent(), kCFRunLoopCommonModes);
            
            MyCFStreamSetUsernamePassword(writeStream, username, password);
            MyCFStreamSetFTPProxy(writeStream, &streamInfo->proxyDict);
            
            /* CFWriteStreamOpen will return success/failure.  Opening a stream causes it to reserve all the
            system resources it requires.  If the stream can open non-blocking, this will always return TRUE;
            listen to the run loop source to find out when the open completes and whether it was successful. */
            success = CFWriteStreamOpen(writeStream);
            if (success == false) {
                fprintf(stderr, "CFWriteStreamOpen failed\n");
                MyStreamInfoDestroy(streamInfo);
            }
        } else {
            fprintf(stderr, "CFWriteStreamSetClient failed\n");
            MyStreamInfoDestroy(streamInfo);
        }
    } else {
        fprintf(stderr, "CFReadStreamOpen failed\n");
        MyStreamInfoDestroy(streamInfo);
    }

    return success;
}


#pragma mark ***** List Command


/* MyPrintDirectoryListing prints a FTP directory entry, represented by a CFDictionary 
as returned by CFFTPCreateParsedResourceListing, as a single line of text, much like 
you'd get from "ls -l". */
static void
MyPrintDirectoryListing(CFDictionaryRef dictionary)
{
    CFDateRef             cfModDate;
    CFNumberRef           cfType, cfMode, cfSize;
    CFStringRef           cfOwner, cfName, cfLink, cfGroup;
    char                  owner[256], group[256], name[256];
    char                  permString[12], link[1024];
    SInt64                size;
    SInt32                mode, type;

    assert(dictionary != NULL);

    /* You should not assume that the directory entry dictionary will contain all the possible keys.
    Most of the time it will, however, depending on the FTP server, some of the keys may be missing. */
        
    cfType = CFDictionaryGetValue(dictionary, kCFFTPResourceType);
    if (cfType) {
        assert(CFGetTypeID(cfType) == CFNumberGetTypeID());
        CFNumberGetValue(cfType, kCFNumberSInt32Type, &type);
        
        cfMode = CFDictionaryGetValue(dictionary, kCFFTPResourceMode);
        if (cfMode) {
            assert(CFGetTypeID(cfMode) == CFNumberGetTypeID());
            CFNumberGetValue(cfMode, kCFNumberSInt32Type, &mode);
            
            /* Converts inode status information into a symbolic string */
            strmode(mode + DTTOIF(type), permString);
            
            fprintf(stderr, "%s ", permString);
        }
    }
    
    cfOwner = CFDictionaryGetValue(dictionary, kCFFTPResourceOwner);
    if (cfOwner) {
        assert(CFGetTypeID(cfOwner) == CFStringGetTypeID());
        CFStringGetCString(cfOwner, owner, sizeof(owner), kCFStringEncodingASCII);
        fprintf(stderr, "%9s", owner);
    }
    
    cfGroup = CFDictionaryGetValue(dictionary, kCFFTPResourceGroup);
    if (cfGroup) {
        assert(CFGetTypeID(cfGroup) == CFStringGetTypeID());
        CFStringGetCString(cfGroup, group, sizeof(group), kCFStringEncodingASCII);
        fprintf(stderr, "%9s", group);
    }
    
    cfSize = CFDictionaryGetValue(dictionary, kCFFTPResourceSize);
    if (cfSize) {
        assert(CFGetTypeID(cfSize) == CFNumberGetTypeID());
        CFNumberGetValue(cfSize, kCFNumberSInt64Type, &size);
        fprintf(stderr, "%9lld ", size);
    }
    
    cfModDate = CFDictionaryGetValue(dictionary, kCFFTPResourceModDate);
    if (cfModDate) {
        CFLocaleRef           locale;
        CFDateFormatterRef    formatDate;
        CFDateFormatterRef    formatTime;
        CFStringRef           cfDate;
        CFStringRef           cfTime;
        char                  date[256];
        char                  time[256];

        assert(CFGetTypeID(cfModDate) == CFDateGetTypeID());

        locale = CFLocaleCopyCurrent();
        assert(locale != NULL);
        
        formatDate = CFDateFormatterCreate(kCFAllocatorDefault, locale, kCFDateFormatterShortStyle, kCFDateFormatterNoStyle   );
        assert(formatDate != NULL);

        formatTime = CFDateFormatterCreate(kCFAllocatorDefault, locale, kCFDateFormatterNoStyle,    kCFDateFormatterShortStyle);
        assert(formatTime != NULL);

        cfDate = CFDateFormatterCreateStringWithDate(kCFAllocatorDefault, formatDate, cfModDate);
        assert(cfDate != NULL);

        cfTime = CFDateFormatterCreateStringWithDate(kCFAllocatorDefault, formatTime, cfModDate);
        assert(cfTime != NULL);

        CFStringGetCString(cfDate, date, sizeof(date), kCFStringEncodingUTF8);
        CFStringGetCString(cfTime, time, sizeof(time), kCFStringEncodingUTF8);
        fprintf(stderr, "%10s %5s ", date, time);

        CFRelease(cfTime);
        CFRelease(cfDate);
        CFRelease(formatTime);
        CFRelease(formatDate);
        CFRelease(locale);
    }

    /* Note that this sample assumes UTF-8 since that's what the Mac OS X
    FTP server returns, however, some servers may use a different encoding. */
    cfName = CFDictionaryGetValue(dictionary, kCFFTPResourceName);
    if (cfName) {
        assert(CFGetTypeID(cfName) == CFStringGetTypeID());
        CFStringGetCString(cfName, name, sizeof(name), kCFStringEncodingUTF8);
        fprintf(stderr, "%s", name);

        cfLink = CFDictionaryGetValue(dictionary, kCFFTPResourceLink);
        if (cfLink) {
            assert(CFGetTypeID(cfLink) == CFStringGetTypeID());
            CFStringGetCString(cfLink, link, sizeof(link), kCFStringEncodingUTF8);
            if (strlen(link) > 0) fprintf(stderr, " -> %s", link);
        }
    }

    fprintf(stderr, "\n");
}


/* MyDirectoryListingCallBack is the stream callback for the CFFTPStream during a directory 
list operation. Its main purpose is to read bytes off the FTP stream, which is returning bytes 
of the directory listing, parse them, and 'pretty' print the resulting directory entries. */
static void
MyDirectoryListingCallBack(CFReadStreamRef readStream, CFStreamEventType type, void * clientCallBackInfo)
{
    MyStreamInfo     *info = (MyStreamInfo *)clientCallBackInfo;
    CFIndex          bytesRead;
    CFStreamError    error;
    CFDictionaryRef  parsedDict;
    
    assert(readStream != NULL);
    assert(info       != NULL);
    assert(info->readStream == readStream);

    switch (type) {

        case kCFStreamEventOpenCompleted:
            fprintf(stderr, "Open complete\n");
            break;
        case kCFStreamEventHasBytesAvailable:
        
            /* When we get here, there are bytes to be read from the stream.  There are two cases:
            either info->leftOverByteCount is zero, in which case we complete processed the last 
            buffer full of data (or we're at the beginning of the listing), or 
            info->leftOverByteCount is non-zero, in which case there are that many bytes at the 
            start of info->buffer that were left over from the last time that we were called. 
            By definition, any left over bytes were insufficient to form a complete directory 
            entry.
            
            In both cases, we just read the next chunk of data from the directory listing stream 
            and append it to our buffer.  We then process the buffer to see if it now contains 
            any complete directory entries. */

            /* CFReadStreamRead will return the number of bytes read, or -1 if an error occurs
            preventing any bytes from being read, or 0 if the stream's end was encountered. */
            bytesRead = CFReadStreamRead(info->readStream, info->buffer + info->leftOverByteCount, kMyBufferSize - info->leftOverByteCount);
            if (bytesRead > 0) {
                const UInt8 *   nextByte;
                CFIndex         bytesRemaining;
                CFIndex         bytesConsumedThisTime;

                /* Parse directory entries from the buffer until we either run out of bytes 
                or we stop making forward progress (indicating that the buffer does not have 
                enough bytes of valid data to make a complete directory entry). */

                nextByte       = info->buffer;
                bytesRemaining = bytesRead + info->leftOverByteCount;
                do
                {                    

                    /* CFFTPCreateParsedResourceListing parses a line of file or folder listing
                    of Unix format, and stores the extracted result in a CFDictionary. */
                    bytesConsumedThisTime = CFFTPCreateParsedResourceListing(NULL, nextByte, bytesRemaining, &parsedDict);
                    if (bytesConsumedThisTime > 0) {

                        /* It is possible for CFFTPCreateParsedResourceListing to return a positive number 
                        but not create a parse dictionary.  For example, if the end of the listing text 
                        contains stuff that can't be parsed, CFFTPCreateParsedResourceListing returns 
                        a positive number (to tell the calle that it's consumed the data), but doesn't 
                        create a parse dictionary (because it couldn't make sens of the data).
                        So, it's important that we only try to print parseDict if it's not NULL. */
                        
                        if (parsedDict != NULL) {
                            MyPrintDirectoryListing(parsedDict);
                            CFRelease(parsedDict);
                        }

                        nextByte       += bytesConsumedThisTime;
                        bytesRemaining -= bytesConsumedThisTime;

                    } else if (bytesConsumedThisTime == 0) {
                        /* This should never happen because we supply a pretty large buffer. 
                        Still, we handle it by leaving the loop, which leaves the remaining 
                        bytes in the buffer. */
                    } else if (bytesConsumedThisTime == -1) {
                        fprintf(stderr, "CFFTPCreateParsedResourceListing parse failure\n");
                        goto exit;
                    }

                } while ( (bytesRemaining > 0) && (bytesConsumedThisTime > 0) );
                
                /* If any bytes were left over, leave them in the buffer for next time. */
                if (bytesRemaining > 0) {
                    memmove(info->buffer, nextByte, bytesRemaining);                    
                }
                info->leftOverByteCount = bytesRemaining;
            } else {
                /* If bytesRead < 0, we've hit an error.  If bytesRead == 0, we've hit the end of the 
                directory listing.  In either case, we do nothing, and rely on CF to call us with 
                kCFStreamEventErrorOccurred or kCFStreamEventEndEncountered in order for us to do our 
                clean up. */
            }
            break;
        case kCFStreamEventErrorOccurred:
            error = CFReadStreamGetError(info->readStream);
            fprintf(stderr, "CFReadStreamGetError returned (%d, %ld)\n", error.domain, error.error);
            goto exit;
        case kCFStreamEventEndEncountered:
            fprintf(stderr, "Listing complete\n");
            goto exit;
        default:
            fprintf(stderr, "Received unexpected CFStream event (%d)", type);
            break;
    }
    return;

exit:
    MyStreamInfoDestroy(info);
    CFRunLoopStop(CFRunLoopGetCurrent());
    return;
}


/* MySimpleDirectoryListing implements the directory list command.  It sets up a MyStreamInfo 
'object' with the read stream being an FTP stream of the directory to list and with no 
write stream.  It then returns, and the real work happens asynchronously in the runloop.  
The function returns true if the stream setup succeeded, and false if it failed. */
static Boolean
MySimpleDirectoryListing(CFStringRef urlString, CFStringRef username, CFStringRef password)
{
    CFReadStreamRef        readStream;
    CFStreamClientContext  context = { 0, NULL, NULL, NULL, NULL };
    CFURLRef               downloadURL;
    Boolean                success = true;
    MyStreamInfo           *streamInfo;

    assert(urlString != NULL);

    downloadURL = CFURLCreateWithString(kCFAllocatorDefault, urlString, NULL);
    assert(downloadURL != NULL);

    /* Create an FTP read stream for downloading operation from an FTP URL. */
    readStream = CFReadStreamCreateWithFTPURL(kCFAllocatorDefault, downloadURL);
    assert(readStream != NULL);
    CFRelease(downloadURL);
        
    /* Initialize our MyStreamInfo structure, which we use to store some information about the stream. */
    MyStreamInfoCreate(&streamInfo, readStream, NULL);
    context.info = (void *)streamInfo;

    /* CFReadStreamSetClient registers a callback to hear about interesting events that occur on a stream. */
    success = CFReadStreamSetClient(readStream, kNetworkEvents, MyDirectoryListingCallBack, &context);
    if (success) {

        /* Schedule a run loop on which the client can be notified about stream events.  The client
        callback will be triggered via the run loop.  It's the caller's responsibility to ensure that
        the run loop is running. */
        CFReadStreamScheduleWithRunLoop(readStream, CFRunLoopGetCurrent(), kCFRunLoopCommonModes);
        
        MyCFStreamSetUsernamePassword(readStream, username, password);
        MyCFStreamSetFTPProxy(readStream, &streamInfo->proxyDict);
    
        /* CFReadStreamOpen will return success/failure.  Opening a stream causes it to reserve all the
        system resources it requires.  If the stream can open non-blocking, this will always return TRUE;
        listen to the run loop source to find out when the open completes and whether it was successful. */
        success = CFReadStreamOpen(readStream);
        if (success == false) {
            fprintf(stderr, "CFReadStreamOpen failed\n");
            MyStreamInfoDestroy(streamInfo);
        }
    } else {
        fprintf(stderr, "CFReadStreamSetClient failed\n");
        MyStreamInfoDestroy(streamInfo);
    }

    return success;
}


#pragma mark ***** Main


static void
MyPrintUsage(const char *executablePath)
{
    const char *programName = strrchr(executablePath, '/') ? strrchr(executablePath, '/') + 1 : executablePath;
    
    fprintf(stderr, "usage: %s -d ftp://ftp.example.com/file.txt [username [password]]\n",   programName);
    fprintf(stderr, "       %s -u ftp://ftp.example.com/ /file.txt [username [password]]\n", programName);
    fprintf(stderr, "       %s -l ftp://ftp.example.com/ [username [password]]\n",           programName);
}


                                            
int
main (int argc, const char * argv[])
{
    CFStringRef  urlString;
    CFStringRef  username = NULL, password = NULL;
    CFURLRef     destinationURL, fileURL;
    char         *cwd;
    int          operation;
    Boolean      status;
    
    /* Must pass in at least three arguments to the command-line tool. */
    if (argc < 3) { MyPrintUsage(argv[0]); return EXIT_FAILURE; }

    /* Retrieve the first argument and return an error if it's not 'd', 'u', or 'l'. */
    operation = getopt(argc, (char * const *)argv, "dul");
    if (operation == -1) { MyPrintUsage(argv[0]); return EXIT_FAILURE; }
    
    /* Use the second argument as the URL and create a CFString from it. */
    urlString = CFStringCreateWithCString(NULL, argv[2], kCFStringEncodingUTF8);
    assert(urlString != NULL);
    
    switch (operation) {
        case 'd':  // Download.
            cwd = getcwd(NULL, 0);     // Get a pointer to the current working directory path.
            assert(cwd != NULL);
            
            /* Create a CFURL from the current working directory CFString. */
            destinationURL = CFURLCreateFromFileSystemRepresentation(NULL, (const UInt8 *) cwd, strlen(cwd), true);
            assert(destinationURL != NULL);
            free(cwd);
            
            /* If there's a third command-line argument, use it as the username for the download. */
            if (argv[3]) {
                username = CFStringCreateWithCString(NULL, argv[3], kCFStringEncodingUTF8);
                assert(username != NULL);
            
                /* If there's a fourth command-line argument, use it as the password for the download. */
                if (argv[4]) {
                    password = CFStringCreateWithCString(NULL, argv[4], kCFStringEncodingUTF8);
                    assert(password != NULL);
                }
            }
            
            /* Start downloading the specified URL to the current working directory. */
            status = MySimpleDownload(urlString, destinationURL, username, password);
            if (!status) fprintf(stderr, "MySimpleDownload failed\n");

            CFRelease(destinationURL);
            break;
        case 'u':  // Upload.
        
            /* Use the third argument as the path of the file to upload. */
            if (argv[3]) {
                fileURL = CFURLCreateFromFileSystemRepresentation(NULL, (const UInt8 *) argv[3], strlen(argv[3]), false);
                assert(fileURL != NULL);
            
                /* If there's a fourth command-line argument, use it as the username for the upload. */
                if (argv[4]) {
                    username = CFStringCreateWithCString(NULL, argv[4], kCFStringEncodingUTF8);
                    assert(username != NULL);
                
                    /* If there's a fifth command-line argument, use it as the password for the upload. */
                    if (argv[5]) {
                        password = CFStringCreateWithCString(NULL, argv[5], kCFStringEncodingUTF8);
                        assert(password != NULL);
                    }
                }
            
                /* Start uploading a file to the specified URL destination. */
                status = MySimpleUpload(urlString, fileURL, username, password);
                if (!status) fprintf(stderr, "MySimpleUpload failed\n");
                
                CFRelease(fileURL);
            
            } else {
                status = false;
                fprintf(stderr, "You must specify a file to upload.\n");
            }
            
            break;
        case 'l': // Directory Listing.
        
            /* If there's a third command-line argument, use it as the username for the directory listing. */
            if (argv[3]) {
                username = CFStringCreateWithCString(NULL, argv[3], kCFStringEncodingUTF8);
                assert(username != NULL);
            
                /* If there's a fourth command-line argument, use it as the password for the directory listing. */
                if (argv[4]) {
                    password = CFStringCreateWithCString(NULL, argv[4], kCFStringEncodingUTF8);
                    assert(password != NULL);
                }
            }
            
            /* Retrieve the directory listing for the specified URL. */
            status = MySimpleDirectoryListing(urlString, username, password);
            if (!status) fprintf(stderr, "MySimpleDirectoryListing failed\n");
            
            break;
        default:
            assert(false);
            status = false;         // just to quieten a warning
            break;
    }
    CFRelease(urlString);
    if (username) CFRelease(username);
    if (password) CFRelease(password);
    
    /* Start the run loop to receive asynchronous callbacks. */
    if (status) CFRunLoopRun();

    return EXIT_SUCCESS;
}
