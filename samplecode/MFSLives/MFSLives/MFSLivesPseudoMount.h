/*
    File:       MFSLivesPseudoMount.h

    Contains:   BSD-level user space code to access MFS disks.

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

$Log: MFSLivesPseudoMount.h,v $
Revision 1.1  2006/07/27 15:48:11         
First checked in.


*/

#ifndef _MFSLIVESPSEUDOMOUNT_H
#define _MFSLIVESPSEUDOMOUNT_H

#include <stdio.h>

/////////////////////////////////////////////////////////////////////

// This module provides a relatively high-level abstraction for listing the directory 
// of and extracting files from an MFS container (typically a disk image file).  It's 
// designed to allow user space tools, like MFSLives.util, to manipulate files without 
// actually mounting the MFS container as a volume.

// All of these return an errno-style error code.  In some cases (where there is no 
// appropriate error code), the routines will print a message to stderr and return 
// ECANCELED, indicating that the caller should not also print an error message.

typedef struct MFSPMount *  MFSPMountRef;

extern void MFSPMountSetLogFile(FILE *logFile);
    // Sets the destination file for any logging done by this module.  If you don't 
    // call this, the module does not generate any log entries.  If logFile is NULL, 
    // logging is disabled.

extern int MFSPMountCreate(const char *containerPath, MFSPMountRef *pmountPtr);
    // Creates an MFSLives pseudomount for the specified container.  The container 
    // can be a Disk Copy 4.2 disk image file (.img), a raw disk image file 
    // (typically .bin, or .cdr, or .iso), or a cooked or raw disk device 
    // (for example, '/dev/disk1' or '/dev/rdisk1').
    //
    // containerPath must not be NULL.
    // pmountPtr must not be NULL
    // On entry, *pmountPtr must be NULL
    // On success, *pmountPtr will be a reference to the pseudomount
    // On error, *pmountPtr will be NULL

extern void MFSPMountDestroy(MFSPMountRef pmount);
    // Destroys a pseudomount created using MFSPMountCreate.  pmount may be NULL, 
    // in which case this does nothing.

extern const void * MFSPMountGetMDBVABM(MFSPMountRef pmount);
    // Gets the MDB/VABM pointer for the pseudomount.  This pointer is only 
    // valid as long as pmount exists.

// The MFSPMountFileInfo structure holds the information needed to location a 
// directory entry on an MFS pseudomount.

struct MFSPMountFileInfo {
    const void *    dirBlockPtr;
    size_t          dirOffset;
};
typedef struct MFSPMountFileInfo MFSPMountFileInfo;

extern int MFSPMountListFiles(MFSPMountRef pmount, MFSPMountFileInfo files[], size_t filesSize, size_t *fileCountPtr);
    // Returns a listing of the directory entries on the pseudomount.
    //
    // pmount must not be NULL
    // files may be NULL if filesSize is 0
    // filesSize is the number of entries available in the files array
    // fileCountPtr must not be NULL
    // On entry, *fileCountPtr is ignored
    // On success, files holds information about the directory entries
    // On success, *fileCountPtr is the number of directory entries; this may be more than filesSize
    // On error, *fileCountPtr is undefined
    //
    // The dirBlockPtr fields of the result files entries are only valid as long as 
    // pmount exists.

extern int MFSPMountExtractFile(MFSPMountRef pmount, const char *fileName, const char *outputFilePath);
    // Extracts the file named fileName from the pseudomount and writes it to the 
    // file at outputFilePath (which must not exist).
    //
    // pmount must not be NULL
    // fileName is the name of the file to extract
    // outputFilePath is the path to the file to create; if this is NULL, the file is 
    // extracted to a fileName in the current directory

#endif
