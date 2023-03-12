/*
    File:       MountMFSLives.c

    Contains:   Tool to mount MFSLives volume.

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

$Log: MountMFSLives.c,v $
Revision 1.1  2006/07/27 15:49:13         
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
#include <stdbool.h>
#include <sys/mount.h>
#include <sys/stat.h>

// Mount argument definitions shared with kernel

#include "MFSLivesMountArgs.h"

extern char **environ;

/////////////////////////////////////////////////////////////////////
#pragma mark ***** Mount Option Parsing

/*
    Mac OS X has standard mount option parsing code, in the form of the getmntopts 
    routine from "mntopts.h".  However, this code isn't exported to third party 
    developers in the current system <rdar://problem/4498991>.  So, I've had to 
    reimplement the parsing code myself.
    
    My philosophy when implementing this code was to accept all possible mount 
    options.  I leave it to the kernel and the VFS plug-in to decide which 
    are appropriate.
    
    If you want to see the code that this is based on, check out the following 
    files in Darwin.

    o diskdev_cmds/disklib/mntopts.h
    o diskdev_cmds/disklib/getmntopts.c
*/

struct MountOption {
    const char *    optionName;             // name of option, without "no" prefix
    bool            optionIsInverted;       // true if option if inverted, for example, MNT_NOSUID
    int             optionMask;             // a MNT_xxx flag
};
typedef struct MountOption MountOption;

static int MountOptionsParseString(const char *optionStr, const MountOption options[], int *mountFlagsPtr)
    // Parses the string specified by optionStr using the options specified by options, 
    // adding any found options to *mountFlagsPtr.  The caller should initialise 
    // *mountFlagsPtr appropriately (typically to 0) before calling this routine.
{
    int             err;
    char *          str;
    char *          cursor;
    const char *    thisToken;
    bool            prefixedByNo;
    size_t          optionIndex;
    
    assert(optionStr != NULL);
    assert(mountFlagsPtr != NULL);
    
    // We need to modify str, so make a copy and work on that.
    
    err = 0;
    str = strdup(optionStr);
    if (str == NULL) {
        err = ENOMEM;
    }
    
    if (err == 0) {
        cursor = str;
        do {
            // Get the next comma separated token.
            
            thisToken = strsep(&cursor, ",");
            
            // If it's valid, and it's not empty.
            
            if ( (thisToken != NULL) && (*thisToken != 0) ) {
                // Handle any "no" prefix.
                
                prefixedByNo = (strncmp(thisToken, "no", 2) == 0);
                if (prefixedByNo) {
                    thisToken += 2;
                }
                
                // If it's still not empty.
                
                if (*thisToken == 0) {
                    err = EINVAL;
                } else {

                    // Search the options array for the token.
                    
                    optionIndex = 0;
                    do {
                        if (options[optionIndex].optionName == NULL) {
                            err = EINVAL;
                            break;
                        }
                        if (strcasecmp(thisToken, options[optionIndex].optionName) == 0) {
                            if (prefixedByNo == options[optionIndex].optionIsInverted) {
                                *mountFlagsPtr |= options[optionIndex].optionMask;
                            } else {
                                *mountFlagsPtr &= ~options[optionIndex].optionMask;
                            }
                            break;
                        }
                        optionIndex += 1;
                    } while (true);
                }
            }
            
        } while ( (err == 0) && (thisToken != NULL) );
    }
    
    free(str);
    
    return err;
}

static char * MountOptionsCreateString(int mountFlags, const MountOption options[])
    // Given a set of mount flags and a description of the known mount options, 
    // return a string that contains a pretty printed form of the mount flags 
    // (a comma separated list of option names).  The caller is responsible 
    // for freeing the string using the System framework's free.
{
    int         pass;
    char *      buf;
    size_t      bufSize;
    size_t      optionIndex;
    bool        isOff;
    int         processedOptions;
    bool        needComma;

    buf = NULL;
    bufSize = 1;                // to allow for trailing null
    for (pass = 0; pass < 2; pass++) {
        needComma = false;

        // Process each option, either calculating the buffer size 
        // (on pass 1, that is, when buf is NULL) or building the string 
        // (on pass 2, that is, when buf is not NULL).
        
        processedOptions = 0;
        optionIndex = 0;
        while (options[optionIndex].optionName != NULL) {
            // Only work with /implemented/ options.
            
            if (options[optionIndex].optionMask != 0) {
            
                // If the option has multiple aliases, only the first is effective.
                
                if ( (options[optionIndex].optionMask & processedOptions) == 0 ) {

                    // Add a comma, if we haven't already.
                    
                    if (needComma) {
                        if (buf == NULL) {
                            bufSize += strlen(",");
                        } else {
                            strlcat(buf, ",", bufSize);
                        }
                    }
                    needComma = true;

                    // Determine if the option is on or off, and whether the flag is 
                    // inverted, and use that to print the option name along with the 
                    // "no" prefix.
                    
                    isOff = (((mountFlags & options[optionIndex].optionMask) == 0) != options[optionIndex].optionIsInverted);
                    if (isOff) {
                        if (buf == NULL) {
                            bufSize += strlen("no");
                        } else {
                            strlcat(buf, "no", bufSize);
                        }
                    }
                    if (buf == NULL) {
                        bufSize += strlen(options[optionIndex].optionName);
                    } else {
                        strlcat(buf, options[optionIndex].optionName, bufSize);
                    }
                    
                    // Mark the option as processed, to avoid us processing any 
                    // aliases.
                    
                    processedOptions |= options[optionIndex].optionMask;
                }
            }
            optionIndex += 1;
        }
        
        // If we're on pass 1, this is the end of the calculation of the buffer size, 
        // so we can allocate the buffer.
        
        if (buf == NULL) {
            buf = malloc(bufSize);
            if (buf == NULL) {
                break;
            } else {
                buf[0] = 0;         // to make it a valid empty string
            }
        }
    }
    
    return buf;
}

static void MountOptionsPrintUsage(FILE *f, int indent, const MountOption options[])
    // Print a list of mount options, suitable for use in a PrintUsage 
    // routine.  f is the file that the list is printed to.  indent 
    // is the number of spaces to indent each line.  options is the 
    // list of known options.
{
    int     processedOptions;
    size_t  optionIndex;
    
    processedOptions = 0;
    optionIndex = 0;
    while (options[optionIndex].optionName != NULL) {
        if (options[optionIndex].optionMask == 0) {
            fprintf(f, "%*s[no]%s is accepted but has no effect\n", indent, "", options[optionIndex].optionName);
        } else if (options[optionIndex].optionMask & processedOptions) {
            int     i;
            
            for (i = 0; i < optionIndex; i++) {
                if (options[i].optionMask == options[optionIndex].optionMask) {
                    break;
                }
            }
            fprintf(
                f, 
                "%*s[no]%s is equivalent to '%s%s'\n", 
                indent, "", 
                options[optionIndex].optionName, 
                (options[i].optionIsInverted == options[optionIndex].optionIsInverted) ? "" : "no",
                options[i].optionName
            );
        } else {
            fprintf(
                f, 
                "%*s[no]%s\n", 
                indent, "", 
                options[optionIndex].optionName
            );
            processedOptions |= options[optionIndex].optionMask;
        }
        optionIndex += 1;
    }
}

// These are the mount options that I know about.

static const MountOption kMountOptions[] = {
    // "auto" is parsed by mount, but isn't used by individual file systems.
    
    { "auto",        false, 0 },

    // These are the standard options defined in the mount document <x-man-page://8/mount>.

    { "async",       false, MNT_ASYNC },
    { "force",       false, MNT_FORCE },
    { "dev",         true,  MNT_NODEV },
    { "exec",        true,  MNT_NOEXEC },
    { "suid",        true,  MNT_NOSUID },
    { "rdonly",      false, MNT_RDONLY },
    { "sync",        false, MNT_SYNCHRONOUS },
    { "update",      false, MNT_UPDATE },
    { "union",       false, MNT_UNION },

    // These options provide <x-man-page://5/fstab> compatibility.
    
    { "ro",          false, MNT_RDONLY },
    { "rw",          true,  MNT_RDONLY },

    // These options cover addition MNT_flags.
    
    { "browse",      true,  MNT_DONTBROWSE },
    { "automounted", false, MNT_AUTOMOUNTED },
    { "defwrite",    false, MNT_DEFWRITE },
    { "perm",        true,  MNT_IGNORE_OWNERSHIP },
    { "owners",      true,  MNT_IGNORE_OWNERSHIP },

    // I have no idea what these are about, but they are handled by getmntopts
    // so I handle them as well.
    
    { "userquota",   false, 0 },
    { "groupquota",  false, 0 },

    { NULL }
};

/////////////////////////////////////////////////////////////////////
#pragma mark ***** Core Code

static FILE *   gLog;

static int gVerbose = 0;

static int LoadKEXT(void)
    // Try to load the file system KEXT into the kernel.  This routine is called 
    // if the mount fails indicating that the "MFSLive" file system is not available.
{
    int     err;
    pid_t   forkResult;
    pid_t   childPID;
    pid_t   waitResult;
    int     childStatus;
    char *  args[6];
    int     argIndex;

    childPID = 0;                   // quieten warning
    
    // Construct the command arguments.
    
    argIndex = 0;
    args[argIndex] = "/sbin/kextload";
    argIndex += 1;
    
    // Unless the user asked for verbosity, remove the "-q" (quiet) flag from kextload.
    
    if (gVerbose == 0) {
        args[argIndex] = "-q";
        argIndex += 1;
    }

    // In the debug build, have kextload generate symbols.
    
    #if ! NDEBUG
        args[argIndex] = "-s";
        argIndex += 1;
        args[argIndex] = "/System/Library/Filesystems/MFSLives.fs/MFSLives.kext/";
        argIndex += 1;
    #endif
    
    args[argIndex] = "/System/Library/Filesystems/MFSLives.fs/MFSLives.kext";
    argIndex += 1;
    
    args[argIndex] = NULL;
        
    if (gLog != NULL) {
        int     argIndex;
        
        fprintf(gLog, "[%ld] LoadKEXT\n", (long) getpid());
        argIndex = 0;
        while ( args[argIndex] != NULL ) {
            fprintf(gLog, "[%ld]   arg[%d] = '%s'\n", (long) getpid(), argIndex, args[argIndex]);
            argIndex += 1;
        }
    }

    // Run kextload in a separate process.  The process inherits stdio from us, which is 
    // what I want; that way, if it does any I/O, it's as if we did it.  It also inherits 
    // other file descriptors from us too, but that's fine too.  We don't have any meaningful 
    // files open at this point in time and, even if we did, kextload is trustworthy code.
    
    forkResult = fork();
    switch (forkResult) {
        case -1:
            err = errno;
            break;
        case 0:
            err = execve(args[0], args, environ);
            _exit(1);
            break;
        default:
            childPID = forkResult;
            err = 0;
            break;
    }
    if (gLog != NULL) fprintf(gLog, "[%ld]   fork -> %ld\n", (long) getpid(), (long) forkResult);

    // Wait for the kextload process to complete and reap its exit status.
    
    if (err == 0) {
        do {
            waitResult = waitpid(childPID, &childStatus, 0);
        } while ( (waitResult == -1) && (errno == EINTR) );

        if (gLog != NULL) fprintf(gLog, "[%ld]   waitpid %ld -> %ld\n", (long) getpid(), (long) childPID, (long) waitResult);

        if (waitResult < 0) {
            err = errno;
        } else {
            assert(waitResult == childPID);
            
            err = ECANCELED;
            if ( WIFEXITED(childStatus) ) {
                if (gLog != NULL) fprintf(gLog, "[%ld]   WIFEXITED -> %d\n", (long) getpid(), WEXITSTATUS(childStatus));
                if ( WEXITSTATUS(childStatus) == 0 ) {
                    err = 0;
                }
            } else if ( WIFSIGNALED(childStatus) ) {
                if (gLog != NULL) fprintf(gLog, "[%ld]   WIFSIGNALED -> %d, %d\n", (long) getpid(), WTERMSIG(childStatus), WCOREDUMP(childStatus));
            } else if ( WIFSTOPPED(childStatus) ) {
                if (gLog != NULL) fprintf(gLog, "[%ld]   WIFSTOPPED -> %d\n", (long) getpid(), WSTOPSIG(childStatus));
            } else {
                if (gLog != NULL) fprintf(gLog, "[%ld]   WIFWTF\n", (long) getpid());
                assert(false);
                err = -1;
            }
        }
    }
    
    return err;
}

static int DoMount(const char *devNode, const char *mountPoint, int flags, bool forceMount, bool forceFailure)
    // Mount the file system.  devNode, mountPoint and flags are all standard 
    // mount arguments.  The remaining arguments are MFSLives specific and 
    // are passed to the kernel as part of the mount parameter block 
    // (MFSLivesMountArgs).
{
    int                 err;
    MFSLivesMountArgs   mountArgs;
    char                realMountPoint[MAXPATHLEN];

    // We have to canonicalise the mount point path because otherwise 
    // <x-man-page://8/umount> can't unmount it by name.
    
    err = 0;
    if ( realpath(mountPoint, realMountPoint) == NULL ) {
        err = EINVAL;
        if (gLog != NULL) fprintf(gLog, "[%ld] realpath %s -> NULL\n", (long) getpid(), mountPoint);
    }

    // Assemble our MFSLivesMountArgs parameter block and call mount.
    
    if (err == 0) {
        mountArgs.fDevNodePath  = devNode;
        mountArgs.fMagic        = kMFSLivesMountArgsMagic;
        mountArgs.fForceMount   = forceMount;
        mountArgs.fForceFailure = forceFailure;
        
        err = mount("MFSLives", realMountPoint, flags, &mountArgs);
        if (err < 0) {
            err = errno;
        }
        if (gLog != NULL) fprintf(gLog, "[%ld] mount '%s' '%s' 0x%x -> %d\n", (long) getpid(), devNode, realMountPoint, flags, err);
        
        // If we get the error that indicates that our KEXT isn't loaded and 
        // we're running as root, load the KEXT and try again.
        
        if (err == ENODEV) {
            if (getuid() == 0) {
                err = LoadKEXT();
                if (gLog != NULL) fprintf(gLog, "[%ld] load KEXT -> %d\n", (long) getpid(), err);
                
                if (err == 0) {
                    err = mount("MFSLives", realMountPoint, flags, &mountArgs);
                    if (err < 0) {
                        err = errno;
                    }
                    if (gLog != NULL) fprintf(gLog, "[%ld] mount retry %s %s 0x%x -> %d\n", (long) getpid(), devNode, realMountPoint, flags, err);
                }
            } else {
                if (gLog != NULL) fprintf(gLog, "[%ld] load KEXT needs root\n", (long) getpid());
            }
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
    fprintf(stderr, "usage: %s [-v] [-F] [-o option] special-device filesystem-node\n", progName);
    fprintf(stderr, "    options:\n");
    MountOptionsPrintUsage(stderr, 8, kMountOptions);
}

extern int main(int argc, char **argv)
{
    int         err;
    int         retVal;
    static const char *kLogPath = "/var/log/mount_MFSLives.log";
    struct stat junkSB;
    int         ch;
    bool        forceMount;
    bool        forceFailure;
    int         mountFlags;
    
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
    }

    // Parse command line options.
    
    forceMount   = false;
    forceFailure = false;
    mountFlags = 0;
    
    retVal = EXIT_SUCCESS;
    do {
        ch = getopt(argc, argv, "vFo:");
        if (ch != -1) {
            switch (ch) {
                case 'v':
                    gVerbose += 1;
                    break;
                case 'f':
                    forceMount = true;
                    break;
                case 'F':
                    forceFailure = true;
                    break;
                case 'o':
                    err = MountOptionsParseString(optarg, kMountOptions, &mountFlags);
                    if (err != 0) {
                        if (err == EINVAL) {
                            PrintUsage(argv[0]);
                        } else {
                            err = errno;
                            perror(NULL);
                        }
                        retVal = EXIT_FAILURE;
                    }
                    break;
                case '?':
                default:
                    PrintUsage(argv[0]);
                    retVal = EXIT_FAILURE;
                    break;
            }
        }
    } while ( (ch != -1) && (retVal == EXIT_SUCCESS) );
    
    // Fail if we don't have exactly two remaining arguments.
    
    if ( (retVal == EXIT_SUCCESS) && ((argc - optind) != 2) ) {
        PrintUsage(argv[0]);
        retVal = EXIT_FAILURE;
    }
    
    // If all is well, do the mount.
    
    if (retVal == EXIT_SUCCESS) {
    
        // If logging, pretty print the options to the log.
        
        if (gLog != NULL) {
            char *  optionsStr;
            
            optionsStr = MountOptionsCreateString(mountFlags, kMountOptions);

            if (optionsStr != NULL) {
                fprintf(gLog, "[%ld] options = 0x%08x (%s)\n", (long) getpid(), mountFlags, optionsStr);
            } else {
                fprintf(gLog, "[%ld] options = 0x%08x\n", (long) getpid(), mountFlags);
            }
            
            free(optionsStr);
        }
        
        // Do the mount.
        
        err = DoMount(argv[optind], argv[optind + 1], mountFlags, forceMount, forceFailure);

        // If it fails, process the error.
        
        if (err != 0) {
            if (err == ECANCELED) {
                fprintf(stderr, "Failed to load KEXT\n");
            } else {
                errno = err;
                perror(NULL);
            }
            retVal = EXIT_FAILURE;
        }
    }

    if (gLog) {
        fprintf(gLog, "[%ld] returning %d\n", (long) getpid(), retVal);
    }
    
    return retVal;
}
