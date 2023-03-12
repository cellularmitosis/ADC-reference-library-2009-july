/*
    File:       MountEmptyFS.c

    Contains:   Tool to mount EmptyFS volume.

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

$Log: MountEmptyFS.c,v $
Revision 1.1  2006/07/04 14:04:03         
First checked in.


*/

/////////////////////////////////////////////////////////////////////

// System interfaces

#include <assert.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <mach/mach.h>
#include <sys/mount.h>

// Mount argument definitions shared with kernel

#include "EmptyFSMountArgs.h"

/////////////////////////////////////////////////////////////////////

static int DoMount(const char *devNode, const char *mountPoint, uint32_t debugLevel, boolean_t forceFailure)
	// Mount the file system on devNode at mountPoint.  debugLevel 
	// and forceFailure are passed to the kernel as part of the mount 
	// arguments.
{
    int                 err;
    EmptyFSMountArgs    mountArgs;
    char                realMountPoint[MAXPATHLEN];

    // We have to canonicalise the mount point path because otherwise 
    // <x-man-page://8/umount> can't unmount it by name.
    
    err = 0;
    if ( realpath(mountPoint, realMountPoint) == NULL ) {
        err = EINVAL;
    }

    if (err == 0) {
        mountArgs.fDevNodePath  = devNode;
        mountArgs.fMagic        = kEmptyFSMountArgsMagic;
        mountArgs.fDebugLevel   = debugLevel;
        mountArgs.fForceFailure = forceFailure;
        
        err = mount("EmptyFS", realMountPoint, 0, &mountArgs);
        if (err < 0) {
            err = errno;
        }
    }
    
    return err;
}

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
    fprintf(stderr, "usage: %s [ -d | -F ] special-device filesystem-node\n", progName);
}

extern int main(int argc, char **argv)
{
    int			err;
    int			retVal;
    int			ch;
    uint32_t	debugLevel;
	boolean_t	forceFailure;
    
	// Parse command line options.
	
	debugLevel   = 0;
	forceFailure = FALSE;
	
    retVal = EXIT_SUCCESS;
    do {
        ch = getopt(argc, argv, "dF");
        if (ch != -1) {
            switch (ch) {
                case 'd':
                    debugLevel += 1;
                    break;
                case 'F':
                    forceFailure = TRUE;
                    break;
                case '?':
                default:
                    PrintUsage(argv[0]);
                    retVal = EXIT_FAILURE;
                    break;
            }
        }
    } while (ch != -1);
    
	// Fail if we don't have exactly two remaining arguments.
	
    if ( (retVal == EXIT_SUCCESS) && ((argc - optind) != 2) ) {
        PrintUsage(argv[0]);
        retVal = EXIT_FAILURE;
    }
    
	// If all is well, do the mount.
	
    if (retVal == EXIT_SUCCESS) {
        err = DoMount(argv[optind], argv[optind + 1], debugLevel, forceFailure);
        
        if (err != 0) {
            errno = err;
            perror(NULL);
            retVal = EXIT_FAILURE;
        }
    }
    
    return retVal;
}
