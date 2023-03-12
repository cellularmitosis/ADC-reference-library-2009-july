/*
    File:       MFSLivesUtil.c

    Contains:   MFSLives utility; user space support included in the .fs bundle.

    Written by: DTS

    Copyright:  Copyright (c) 2006 by Apple Computer, Inc., All Rights Reserved.

    Disclaimer: IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc.
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

    Change History (most recent first):

$Log: MFSLivesUtil.c,v $
Revision 1.2  2006/10/17 15:21:05         
Move the mount and utility tools into Contents/Resources.

Revision 1.1  2006/07/27 15:48:15         
First checked in.


*/

/////////////////////////////////////////////////////////////////////

// System interfaces

#include <assert.h>
#include <errno.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <getopt.h>
#include <fcntl.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/disk.h>
#include <sys/loadable_fs.h>
#include <sys/mman.h>
#include <sys/xattr.h>

// MFS core code

#include "MFSCore.h"
#include "MFSLivesPseudoMount.h"

/////////////////////////////////////////////////////////////////////

static FILE *   gLog;

static int gVerbose;

/////////////////////////////////////////////////////////////////////
#pragma mark ***** Commands to Support DiskArb

static int ProbeCommand(const char *diskDeviceName)
    // Probe the disk specified by diskDeviceName (this is of the form 
    // "disk1", that is, a block device name, not a path) to see if it's 
    // an MFS disk.  Returns FSUR_RECOGNIZED if it is, and FSUR_UNRECOGNIZED 
    // otherwise.
{
    int             err;
    int             junk;
    int             fd;
    char            rawDevPath[MAXPATHLEN];
    uint32_t        blockSize;
    uint64_t        blockCount;
    void *          buf;
    size_t          mdbAndVABMSizeInBytes;
    uint16_t        directoryStartBlock;
    uint16_t        directoryBlockCount;
    uint16_t        allocationBlocksStartBlock;
    uint32_t        allocationBlockSizeInBytes;
    ssize_t         bytesRead;
    ssize_t         bytesWritten;
    char            volumeName[MAXPATHLEN];
    struct vfs_attr attr;
    
    assert(diskDeviceName != NULL);
    
    if (gLog != NULL) fprintf(gLog, "[%ld] Probe '%s'\n", (long) getpid(), diskDeviceName);

    buf = NULL;
    
    // Construct the path to the character (raw) device, for example, 
    // "/dev/rdisk1" and open it up.
    
    snprintf(rawDevPath, sizeof(rawDevPath), "/dev/r%s", diskDeviceName);
    err = 0;
    fd = open(rawDevPath, O_RDONLY);
    if (fd < 0) {
        err = errno;
    }
    if (gLog != NULL) fprintf(gLog, "[%ld]   open '%s' -> %d\n", (long) getpid(), rawDevPath, err);

    // Get its block size and fail if it's not 512.
    
    if (err == 0) {
        blockSize = 0;
        
        err = ioctl(fd, DKIOCGETBLOCKSIZE, &blockSize);
        if (err < 0) {
            err = errno;
        }
        if (gLog != NULL) fprintf(gLog, "[%ld]   ioctl DKIOCGETBLOCKSIZE -> %d, %lu\n", (long) getpid(), err, (long) blockSize);
    }
    if ( (err == 0) && (blockSize != 512) ) {
        err = EINVAL;
    }
    
    // Allocate a buffer, and read the MFS MDB block into it.
    
    if (err == 0) {
        buf = malloc(blockSize);
        if (buf == NULL) {
            err = ENOMEM;
        }
    }
    if (err == 0) {
        bytesRead = pread(fd, buf, blockSize, kMFSMDBBlock * blockSize);
        if (bytesRead < 0) {
            err = errno;
        } else if (bytesRead != blockSize) {
            err = EINVAL;
        }
        if (gLog != NULL) fprintf(gLog, "[%ld]   read %lu -> %d\n", (long) getpid(), (long) blockSize, err);
    }

    // Get the block count, because MFSMDBCheck wants it in order to do proper checking.
    
    if (err == 0) {
        blockCount = 0;
        
        err = ioctl(fd, DKIOCGETBLOCKCOUNT, &blockCount);
        if (err < 0) {
            err = errno;
        }
        if (gLog != NULL) fprintf(gLog, "[%ld]   ioctl DKIOCGETBLOCKCOUNT -> %d, %llu\n", (long) getpid(), err, (long long) blockCount);
    }
    
    // Call the MFS core code to see if it looks like an MFS disk.
    
    if (err == 0) {
        mdbAndVABMSizeInBytes = 0;
        
        err = MFSMDBCheck(buf, blockCount, &mdbAndVABMSizeInBytes, &directoryStartBlock, &directoryBlockCount, &allocationBlocksStartBlock, &allocationBlockSizeInBytes);
        if (gLog != NULL) fprintf(gLog, "[%ld]   MFSMDBCheck -> %d, %zu\n", (long) getpid(), err, mdbAndVABMSizeInBytes);

        // If the check fails, log what went wrong.
        
        if ( (err == EINVAL) && (gLog != NULL) ) {
            char errStr[256];

            MFSMDBGetError(buf, blockCount, errStr, sizeof(errStr));
            fprintf(gLog, "[%ld]   MFSMDBGetError -> %s\n", (long) getpid(), errStr);
        }
    }
    
    // If so, free the current buffer, allocate a new buffer big enough to contain the entire 
    // MDB and VABM, and fill it.
    
    if (err == 0) {
        mdbAndVABMSizeInBytes = (mdbAndVABMSizeInBytes + (blockSize - 1)) / blockSize * blockSize;
        
        free(buf);
        buf = malloc(mdbAndVABMSizeInBytes);
        if (buf == NULL) {
            err = ENOMEM;
        }
    }
    if (err == 0) {
        bytesRead = pread(fd, buf, mdbAndVABMSizeInBytes, kMFSMDBBlock * blockSize);
        if (bytesRead < 0) {
            err = errno;
        } else if (bytesRead != mdbAndVABMSizeInBytes) {
            err = EINVAL;
        }
        if (gLog != NULL) fprintf(gLog, "[%ld]   read %zu -> %d\n", (long) getpid(), mdbAndVABMSizeInBytes, err);
    }
    
    // Call the MFS core code to get the volume name.
    
    if (err == 0) {
        VFSATTR_INIT(&attr);
        VFSATTR_WANTED(&attr, f_vol_name);
        attr.f_vol_name = volumeName;
        
        err = MFSMDBGetAttr(buf, &attr);
        if (err != 0) {
            volumeName[0] = 0;
        }
        if (gLog != NULL) fprintf(gLog, "[%ld]   MFSMDBGetAttr -> %d, '%s'\n", (long) getpid(), err, volumeName);
    }
    
    // Print the volume name to stdout, which is where DiskArb expects to pick it up.
    
    if (err == 0) {
        // MFSCore returns the data in UTF-8 format, which is exactly what we need.  
        // We append a return purely for the sake of human users; DiskArb will be 
        // happy either way.  An MFS volume name is /way/ less than MAXPATHLEN, so 
        // there's plenty of space for this extra character.
        
        strlcat(volumeName, "\n", sizeof(volumeName));
        
        bytesWritten = write(STDOUT_FILENO, volumeName, strlen(volumeName));
        if (bytesWritten < 0) {
            err = errno;
        } else if (bytesWritten != strlen(volumeName)) {
            err = EINVAL;
        }
        if (gLog != NULL) fprintf(gLog, "[%ld]   write -> %d\n", (long) getpid(), err);
    }
    
    // Clean up.

    free(buf);
    if (fd != -1) {
        junk = close(fd);
        assert(junk == 0);
    }
    
    // If we're running as root and we successfully recognise a volume, create 
    // a symlink from "/sbin/mount_MFSLives" to "/System/Library/Filesystems/MFSLives.fs/Contents/Resources/mount_MFSLives". 
    // I do this because:
    //
    // a) The system expects to find the mount_MFSLives tool in /sbin, and things don't 
    //    work if it's not there.  Specifically, mount -t MFSLives will only find the 
    //    mount tool in /sbin, and that's what DiskArb uses to mount the disk.
    //
    // b) I don't want to place the real tool there, because I prefer everything to be 
    //    bundled up together in my .fs bundle.
    //
    // c) I don't want a separate installation step (installer, whatever) to be responsible 
    //    for creating this symlink.
    //
    // Note that I ignore any failure from this operation.  I just let DiskArb get 
    // and handle the resulting mount -t failure.  This simplifies matters because 
    // I /always/ try to create the symlink.  If it already exists, it just fails.
    //
    // I hard-code the paths (as opposed to, say, using NSGetExecutablePath to find 
    // my own path, and use that to track down mount_MFSLives tool) because it makes 
    // the security implications much easier to understand.
    //
    // I only do this if I'm run as root (both effective and real user IDs) because 
    // a) it will only work if EUID is 0, and there's no point trying otherwise, and 
    // b) any attacker that's already root doesn't need my help.
    //
    // There shouldn't be any serious security gotchas here because both /sbin 
    // and /System/Library/Filesystems are only writable by root.
    
    if ( (getuid() == 0) && (geteuid() == 0) ) {
        int     err2;
        
        err2 = symlink("/System/Library/Filesystems/MFSLives.fs/Contents/Resources/mount_MFSLives", "/sbin/mount_MFSLives");
        if (err2 < 0) {
            err2 = errno;
        }
        if (gLog != NULL) {
            if (err2 == EEXIST) {
                fprintf(gLog, "[%ld]   symlink already exists\n", (long) getpid());
            } else {
                fprintf(gLog, "[%ld]   symlink -> %d\n", (long) getpid(), err2);            
            }
        }
    }

    return (err == 0) ? FSUR_RECOGNIZED : FSUR_UNRECOGNIZED;
}

/////////////////////////////////////////////////////////////////////
#pragma mark ***** Commands For The Users

// IMPORTANT
// The List and Extract commands are /not/ required by Disk Arbitration.  
// They're here for the convenience of the user (so you don't have to risk 
// life and limb installing a kernel extension just to get files off your 
// MFS disks).

#pragma mark - List Command

static void OSTypeToUTF8String(uint8_t *ostPtr, char *utf8Name, size_t utf8NameSize)
    // Convert an OSType to a UTF-8 string, handling byte reversal, the possibility 
    // of null characters, and the MacRoman-to-UTF-8 conversion.  Why is this so hard?
{
    uint32_t    ost;
    uint8_t     pstr[5];
    int         i;
    size_t      sizeNeeded;
    
    // Do the byte swap.
    
    ost = OSReadBigInt32(ostPtr, 0);
    
    // Convert zeros to spaces.
    
    pstr[0] = 4;
    for (i = 3; i >= 0; i--) {
        uint8_t     ch;

        ch = ost & 0x00FF;
        ost >>= 8;
        
        if (ch == 0) {
            ch = ' ';
        }
        pstr[i + 1] = ch;
    }

    // Convert to UTF-8.
    
    sizeNeeded = MFSNameToUTF8(pstr, utf8Name, utf8NameSize);
    assert(sizeNeeded <= utf8NameSize);     // if this fails, we've truncated
}

static void PrintDirectoryEntry(const struct vnode_attr *attr, uint8_t finderInfo[], const MFSForkInfo forkInfos[])
    // Pretty prints an MFS directory entry in one of three ways depending on 
    // the setting of gVerbose.
    //
    // finderInfo points to an array of 16 bytes.
    // forkInfos points to an array of two elements, indexed by the forkIndex.
{
    char            fileTypeStr[32];
    char            fileCreatorStr[32];
    int             i;
    struct tm       tm;
    char            dateTimeStr[256];
    size_t          size;

    assert(attr != NULL);
    assert(finderInfo != NULL);
    assert(forkInfos != NULL);
    
    switch (gVerbose) {
        case 0:
            fprintf(stdout, "%s\n", attr->va_name);
            break;
        case 1:
            OSTypeToUTF8String(&finderInfo[0], fileTypeStr,    sizeof(fileTypeStr)   );
            OSTypeToUTF8String(&finderInfo[4], fileCreatorStr, sizeof(fileCreatorStr));
            // type crea size size name
            fprintf(stdout, "%10u %s %s %10u %10u %s\n", (unsigned int) attr->va_fileid, fileTypeStr, fileCreatorStr, forkInfos[0].lengthInBytes, forkInfos[1].lengthInBytes, attr->va_name);
            break;
        default:
            fprintf(stdout, "name: %s\n", attr->va_name);
            fprintf(stdout, "fileNumber: %u\n", (unsigned int) attr->va_fileid);
            fprintf(stdout, "finderInfo:");
            for (i = 0; i < 16; i++) {
                fprintf(stdout, " %02x", finderInfo[i]);
            }
            fprintf(stdout, "\n");
            fprintf(stdout, "dataLengthInBytes: %u\n", forkInfos[0].lengthInBytes);
            fprintf(stdout, "dataPhysicalLengthInBytes: %u\n", forkInfos[0].physicalLengthInBytes);
            fprintf(stdout, "rsrcLengthInBytes: %u\n", forkInfos[1].lengthInBytes);
            fprintf(stdout, "rsrcPhysicalLengthInBytes: %u\n", forkInfos[1].physicalLengthInBytes);

            // For an explanation of why I use gmtime_r and not localtime_r here, see 
            // the "Dates/Time Values" comment in "MFSCore.h".
            
            (void) gmtime_r(&attr->va_create_time.tv_sec, &tm);
            size = strftime(dateTimeStr, sizeof(dateTimeStr), "%a, %d %b %Y %H:%M:%S %Z", &tm);
            assert(size != 0);
            fprintf(stdout, "creationDate: %s\n", dateTimeStr);

            (void) gmtime_r(&attr->va_modify_time.tv_sec, &tm);
            size = strftime(dateTimeStr, sizeof(dateTimeStr), "%a, %d %b %Y %H:%M:%S %Z", &tm);
            assert(size != 0);
            fprintf(stdout, "modificationDate: %s\n", dateTimeStr);
            break;
    }
}

static int ListCommand(const char *containerPath)
    // Implements the list command.  Pseudo mounts the 'volume' and iterates 
    // each directory block, printing the results.
{
    int                 err;
    MFSPMountRef        pmount;
    size_t              fileCountToAlloc;
    size_t              fileCount;
    size_t              fileIndex;
    MFSPMountFileInfo * files;
    struct vnode_attr   attr;
    
    assert(containerPath != NULL);

    if (gLog != NULL) fprintf(gLog, "[%ld] List '%s'\n", (long) getpid(), containerPath);

    pmount = NULL;
    files  = NULL;
    
    // Pseudo mount the 'volume'.
    
    err = MFSPMountCreate(containerPath, &pmount);

    // Get the directory list.
    
    if (err == 0) {
        err = MFSPMountListFiles(pmount, NULL, 0, &fileCountToAlloc);
    }
    if (err == 0) {
        files = malloc(fileCountToAlloc * sizeof(*files));
        if (files == NULL) {
            err = ENOMEM;
        }
    }
    if (err == 0) {
        err = MFSPMountListFiles(pmount, files, fileCountToAlloc, &fileCount);
    }
    
    // Print it.
    
    if (err == 0) {
        assert(fileCount == fileCountToAlloc);

        for (fileIndex = 0; fileIndex < fileCount; fileIndex++) {
            char            name[MAXPATHLEN];
            MFSForkInfo     forkInfos[2];
            uint8_t         finderInfo[16];
            
            name[0] = 0;    // init to empty string to allow logging even if we get an error
            
            // Gather all of the necessary information about the directory entry.
            
            VATTR_INIT(&attr);
            attr.va_name = name;
            VATTR_WANTED(&attr, va_name);
            VATTR_WANTED(&attr, va_fileid);
            VATTR_WANTED(&attr, va_create_time);
            VATTR_WANTED(&attr, va_modify_time);
            
            err = MFSDirectoryEntryGetAttr(
                files[fileIndex].dirBlockPtr,
                files[fileIndex].dirOffset,
                &attr
            );
            if (err == 0) {
                err = MFSDirectoryEntryGetFinderInfo(files[fileIndex].dirBlockPtr, files[fileIndex].dirOffset, &finderInfo);
            }
            if (err == 0) {
                err = MFSDirectoryEntryGetForkInfo(files[fileIndex].dirBlockPtr, files[fileIndex].dirOffset, 0, &forkInfos[0]);
            }
            if (err == 0) {
                err = MFSDirectoryEntryGetForkInfo(files[fileIndex].dirBlockPtr, files[fileIndex].dirOffset, 1, &forkInfos[1]);
            }
            if (gLog != NULL) fprintf(gLog, "[%ld]  %3d %3zu %3zu '%s'\n", (long) getpid(), err, fileIndex, files[fileIndex].dirOffset, name);

            // Now that we have everything we need to know about this directory entry, 
            // let's print it.
            
            if (err == 0) {
                // It's kinda ugly testing gVerbose here, but I prefer it to passing fileIndex 
                // to PrintDirectoryEntry.
                
                if ( (fileIndex > 0) && (gVerbose > 1) ) {
                    fprintf(stdout, "\n");
                }
                
                PrintDirectoryEntry(&attr, finderInfo, forkInfos);
            }
        }
    }
    
    // Clean up.
    
    free(files);
    MFSPMountDestroy(pmount);
    
    // Print the error, unless the underlying code has already done so (indicated 
    // by it return ECANCELED).
    
    if ( (err != 0) && (err != ECANCELED) ) {
        errno = err;
        perror(NULL);
    }
    
    return ((err == 0) ? EXIT_SUCCESS : EXIT_FAILURE);
}

#pragma mark - Extract Command

static int ExtractCommand(const char *containerPath, const char *fileName, const char *outputFilePath)
    // Implements the extract command.  Pseudo mounts the 'volume', finds the file whose  
    // name is fileName, and extracts it to a newly created file at outputFilePath (or, 
    // if outputFilePath is NULL, to a newly created file named fileName in the current 
    // working directory).
{
    int                 err;
    MFSPMountRef        pmount;

    assert(containerPath != NULL);
    assert(fileName != NULL);
    // outputFilePath may be NULL

    if (gLog != NULL) fprintf(gLog, "[%ld] Extract '%s' '%s' '%s'\n", (long) getpid(), containerPath, fileName, ((outputFilePath != NULL) ? outputFilePath : "") );

    pmount = NULL;
    
    // Initialise the MFS core.
    
    err = MFSPMountCreate(containerPath, &pmount);
    
    // Do the work.
    
    if (err == 0) {
        err = MFSPMountExtractFile(pmount, fileName, outputFilePath);
    }
    
    // Clean up.
    
    MFSPMountDestroy(pmount);
    
    // Print the error, unless the underlying code has already done so (indicated 
    // by it return ECANCELED).

    if ( (err != 0) && (err != ECANCELED) ) {
        errno = err;
        perror(NULL);
    }
    
    return ((err == 0) ? EXIT_SUCCESS : EXIT_FAILURE);
}

/////////////////////////////////////////////////////////////////////
#pragma mark ***** Main etc

static void PrintUsage(const char *argv0)
    // Print a helpful help message.
{
    const char *    progName;
    
    progName = strrchr(argv0, '/');
    if (progName == NULL) {
        progName = argv0;
    } else {
        progName += 1;
    }
    fprintf(stderr, "usage: %s [-v] -p diskDeviceName info...\n", progName);
    fprintf(stderr, "       %s [-v] -L containerPath\n", progName);
    fprintf(stderr, "       %s [-v] -X containerPath fileName [ outputFilePath ]\n", progName);
    fprintf(stderr, "    where:\n");
    fprintf(stderr, "        o diskDeviceName is the name of a disk device (for example, 'disk1')\n");
    fprintf(stderr, "        o containerPath is the path to a Disk Copy 4.2 file (.img), a raw disk \n");
    fprintf(stderr, "          image file (typically .bin, or .cdr, or .iso), or a cooked or raw \n");
    fprintf(stderr, "          disk device (for example, '/dev/disk1' or '/dev/rdisk1')\n");
    
}

extern int main(int argc, char **argv)
{
    int         retVal;
    static const char *kLogPath = "/var/log/MFSLives.util.log";
    struct stat junkSB;
    int         ch;
    enum {
        kCommandUnspecified,
        kCommandProbe,
        kCommandList,
        kCommandExtract
    } command;
    
    // Set up logging
    
    // Because the log file is in a directory that's only writable by root, 
    // there are no nasty security gotchas here.
    
    gLog = NULL;
    
    if ( stat(kLogPath, &junkSB) == 0 ) {
        gLog = fopen(kLogPath, "a");
    }
    if (gLog != NULL) {
        int argIndex;
        
        fprintf(gLog, "[%ld] launch (%ld, %ld):\n", (long) getpid(), (long) geteuid(), (long) getuid());
        for (argIndex = 0; argIndex < argc; argIndex++) {
            fprintf(gLog, "[%ld]   arg[%d] = '%s'\n", (long) getpid(), argIndex, argv[argIndex]);
        }
        fflush(gLog);
        (void) setvbuf(gLog, NULL, _IONBF, 0);
        
        MFSPMountSetLogFile(gLog);
    }
    
    // Parse command line options.
    
    command = kCommandUnspecified;
    
    retVal = FSUR_IO_SUCCESS;
    do {
        ch = getopt(argc, argv, "vpLX");
        if (ch != -1) {
            switch (ch) {
                case 'v':
                    gVerbose += 1;
                    break;
                case 'p':
                    if (command == kCommandUnspecified) {
                        command = kCommandProbe;
                    } else {
                        PrintUsage(argv[0]);
                        retVal = FSUR_INVAL;
                    }
                    break;
                case 'L':
                    if (command == kCommandUnspecified) {
                        command = kCommandList;
                    } else {
                        PrintUsage(argv[0]);
                        retVal = FSUR_INVAL;
                    }
                    break;
                case 'X':
                    if (command == kCommandUnspecified) {
                        command = kCommandExtract;
                    } else {
                        PrintUsage(argv[0]);
                        retVal = FSUR_INVAL;
                    }
                    break;
                case '?':
                default:
                    PrintUsage(argv[0]);
                    retVal = FSUR_INVAL;
                    break;
            }
        }
    } while ( (ch != -1) && (retVal == FSUR_IO_SUCCESS) );

    // Do the commands.
    
    if (retVal == FSUR_IO_SUCCESS) {
        bool printUsage;
        
        printUsage = false;
        switch (command) {
            case kCommandProbe:
                if (optind < argc) {     // we ignore arguments after the device name
                    retVal = ProbeCommand(argv[optind]);
                } else {
                    PrintUsage(argv[0]);
                    retVal = FSUR_INVAL;
                }
                break;
            case kCommandList:
                if ( (argc - optind) == 1 ) {
                    retVal = ListCommand(argv[optind]);
                } else {
                    printUsage = true;
                }
                break;
            case kCommandExtract:
                if ( ((argc - optind) == 2) || ((argc - optind) == 3) ) {
                    retVal = ExtractCommand(argv[optind], argv[optind + 1], argv[optind + 2]);
                } else {
                    printUsage = true;
                }
                break;
            default:
                PrintUsage(argv[0]);
                retVal = FSUR_INVAL;
                break;
        }
        
        if (printUsage) {
            PrintUsage(argv[0]);
            retVal = FSUR_INVAL;
        }
    }
    
    if (gLog) {
        fprintf(gLog, "[%ld] returning %d\n", (long) getpid(), retVal);
    }
    
    return retVal;
}
