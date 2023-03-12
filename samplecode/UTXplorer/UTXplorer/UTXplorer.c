/*
    File:       UTXplorer.c

    Contains:   A tool that demonstrates basic use of the <utmpx.h> API.

    Written by: DTS

    Copyright:  Copyright (c) 2008 Apple Inc. All Rights Reserved.

    Disclaimer: IMPORTANT: This Apple software is supplied to you by Apple Inc.
                ("Apple") in consideration of your agreement to the following
                terms, and your use, installation, modification or
                redistribution of this Apple software constitutes acceptance of
                these terms.  If you do not agree with these terms, please do
                not use, install, modify or redistribute this Apple software.

                In consideration of your agreement to abide by the following
                terms, and subject to these terms, Apple grants you a personal,
                non-exclusive license, under Apple's copyrights in this
                original Apple software (the "Apple Software"), to use,
                reproduce, modify and redistribute the Apple Software, with or
                without modifications, in source and/or binary forms; provided
                that if you redistribute the Apple Software in its entirety and
                without modifications, you must retain this notice and the
                following text and disclaimers in all such redistributions of
                the Apple Software. Neither the name, trademarks, service marks
                or logos of Apple Inc. may be used to endorse or promote
                products derived from the Apple Software without specific prior
                written permission from Apple.  Except as expressly stated in
                this notice, no other rights or licenses, express or implied,
                are granted by Apple herein, including but not limited to any
                patent rights that may be infringed by your derivative works or
                by other works in which the Apple Software may be incorporated.

                The Apple Software is provided by Apple on an "AS IS" basis. 
                APPLE MAKES NO WARRANTIES, EXPRESS OR IMPLIED, INCLUDING
                WITHOUT LIMITATION THE IMPLIED WARRANTIES OF NON-INFRINGEMENT,
                MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE, REGARDING
                THE APPLE SOFTWARE OR ITS USE AND OPERATION ALONE OR IN
                COMBINATION WITH YOUR PRODUCTS.

                IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT,
                INCIDENTAL OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
                TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
                DATA, OR PROFITS; OR BUSINESS INTERRUPTION) ARISING IN ANY WAY
                OUT OF THE USE, REPRODUCTION, MODIFICATION AND/OR DISTRIBUTION
                OF THE APPLE SOFTWARE, HOWEVER CAUSED AND WHETHER UNDER THEORY
                OF CONTRACT, TORT (INCLUDING NEGLIGENCE), STRICT LIABILITY OR
                OTHERWISE, EVEN IF APPLE HAS BEEN ADVISED OF THE POSSIBILITY OF
                SUCH DAMAGE.

*/

#include <assert.h>
#include <errno.h>
#include <getopt.h>
#include <notify.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdint.h>
#include <utmpx.h>

// IMPORTANT:
// In portable code you can't assume that the utx types are contiguous, but 
// the values for Mac OS X are fixed for all time because of binary 
// compatibility so I'm going to exploit that in this program.  Just to be 
// sure, I used UTXCommandsOK to do a quick check of this assumption.

enum {
    kNumberOfUTXTypes = SHUTDOWN_TIME + 1
};

// Masks for the fieldMask field of UTXType.  These indicate which fields 
// of the (struct utmpx) are valid for that type.

enum {
    kUserMask = 1,
    kUIDMask  = 2,
    kLineMask = 4,
    kPIDMask  = 8,
    kTimeMask = 16,
    kHostMask = 32
};

// kUTXTypes describes useful features of the utx types that we understand.

struct UTXType {
    bool            listByDefault;      // whether to display it by default
    const char *    name;               // the identifier
    uint32_t        fieldMask;          // which fields of the (struct utmpx) are valid
};
typedef struct UTXType UTXType;
static UTXType kUTXTypes[kNumberOfUTXTypes] = {
    { false, "EMPTY",                                                                           0 }, 
    { true,  "RUN_LVL",                                                                         0 }, 
    { true,  "BOOT_TIME",                                                   kTimeMask             }, 
    { true,  "OLD_TIME",                                                    kTimeMask             }, 
    { true,  "NEW_TIME",                                                    kTimeMask             }, 
    { true,  "INIT_PROCESS",              kUIDMask |             kPIDMask | kTimeMask,            }, 
    { true,  "LOGIN_PROCESS", kUserMask | kUIDMask |             kPIDMask | kTimeMask             }, 
    { true,  "USER_PROCESS",  kUserMask | kUIDMask | kLineMask | kPIDMask | kTimeMask | kHostMask }, 
    { true,  "DEAD_PROCESS",              kUIDMask |             kPIDMask | kTimeMask             }, 
    { true,  "ACCOUNTING",                                                                      0 }, 
    { true,  "SIGNATURE",                                                                       0 }, 
    { true,  "SHUTDOWN_TIME",                                               kTimeMask             }
};

#if ! defined(NDEBUG)

    static bool UTXTypesOK(void)
        // Does a quick check of kUTXTypes to ensure that we're not in the weeds, 
        // returning true if everything is OK.  See the "IMPORTANT" note above.
    {
        return ( strcmp(kUTXTypes[SHUTDOWN_TIME].name, "SHUTDOWN_TIME") == 0 );
    }

#endif

// Field width constants for all of the output.

enum {
    kTimeFieldWidth = 24,
    kTypeFieldWidth = 13,
    kIDFieldWidth   = _UTX_IDSIZE,
    kPIDFieldWidth  = 5,
    kUserFieldWidth = 16,
    kLineFieldWidth = 16
};

static void DoGetUTXEntCommand(
    bool            wtmp, 
    bool            waitForMore, 
    uint32_t        verbose, 
    const bool      enabledTypes[]
)
    // Prints all of the utx records in either utmp mode (records currently 
    // active, wtmp == false) or wtmp mode (historical records, wtmp == true). 
    // If waitForMode is true, it prints the current state and then waits, 
    // printing new records as they arrive.  verbose controls the level of 
    // debugging output.  enabledTypes, whose size is kNumberOfUTXTypes, 
    // indicates which type of records should be displayed.
{
    int             waitFD;
    uint32_t        noteErr;
    int             tokenRegistered;
    bool            firstTime;
    struct timeval  latestTime;
    struct utmpx *  utx;
    bool            enabled;
    const char *    typeStr;
    uint32_t        fieldMask;
    ssize_t         bytesRead;
    int             tokenRead;
    
    assert(enabledTypes != NULL);
    
    // If we've been told to wait for more, set up our notification file descriptor.
    
    waitFD = -1;
    if (waitForMore) {
        noteErr = notify_register_file_descriptor(
            UTMPX_CHANGE_NOTIFICATION,
            &waitFD, 
            0,
            &tokenRegistered
        );
        assert(noteErr == NOTIFY_STATUS_OK);
    }
    
    // Print the header.
    
    fprintf(
        stdout, 
        "%-*s %-*s %-*s %-*s %-*s %-*s %s\n", 
        kTimeFieldWidth, "time", 
        kTypeFieldWidth, "type", 
        kIDFieldWidth,   "id", 
        kPIDFieldWidth,  "pid", 
        kUserFieldWidth, "user", 
        kLineFieldWidth, "line", 
                         "host"
    );
    fprintf(
        stdout, 
        "%-*s %-*s %-*s %-*s %-*s %-*s %s\n", 
        kTimeFieldWidth, "----", 
        kTypeFieldWidth, "----", 
        kIDFieldWidth,   "--", 
        kPIDFieldWidth,  "---", 
        kUserFieldWidth, "----", 
        kLineFieldWidth, "----", 
                         "----"
    );

    // By default getutxent_wtmp returns its values in reverse chronological order. 
    // This isn't really what we want for this program, so we call setutxent_wtmp(1) 
    // to get chronological order.
    //
    // By the way, while setutxent_wtmp is currently not documented <rdar://problem/5770912>, 
    // it is a supported routine.
    
    if (wtmp) {
        setutxent_wtmp(1);
    }

    // The outer loop runs while we wait for more.
    // The inner loop runs for each entry returned by getutxent[_wtmp].
    
    firstTime = true;
    timerclear(&latestTime);
    do {
        do {
            // Get the next database record.  If we get none, the inner loop is done.
            
            if (wtmp) {
                utx = getutxent_wtmp();
            } else {
                utx = getutxent();
            }
            if (utx == NULL) {
                break;
            }

            // We do different things for each type of record.  Extract the 
            // information based on the record type.  Do something sensible if 
            // we get a record type we don't understand.

            if ( (utx->ut_type >= 0) && (utx->ut_type < kNumberOfUTXTypes) ) {
                enabled   = enabledTypes[utx->ut_type];
                typeStr   = kUTXTypes[utx->ut_type].name;
                fieldMask = kUTXTypes[utx->ut_type].fieldMask;
            } else {
                enabled   = true;
                typeStr   = "<unknown>";
                fieldMask = 0;
            }
            
            // If we're very verbose, ignore the default enabled setting.  If we're 
            // verbose but not very verbose, log events that we discarded because 
            // they're the wrong type.
            
            if (verbose >= 2) {
                enabled = true;
            }
            if ( ! enabled && (verbose == 1) ) {
                fprintf(stdout, "*** %s event discarded because it is disabled\n", typeStr);
            }
            
            // Only print events without a time stamp the first time around.
            
            if ( enabled && (verbose < 2) && ! firstTime && ((fieldMask & kTimeMask) == 0) ) {
                if (verbose == 1) {
                    fprintf(stdout, "*** %s event discarded because it has no time\n", typeStr);
                }
                enabled = false;
            }

            // If this isn't the first time around, discard any event that's 
            // timestamped earlier than the last event we saw.
            
            if ( enabled && (verbose < 2) && ! firstTime && timercmp(&utx->ut_tv, &latestTime, <=) ) {
                if (verbose == 1) {
                    fprintf(
                        stdout, 
                        "*** %s event discarded because it's too old %ld.%d <= %ld.%d\n", 
                        typeStr, 
                        utx->ut_tv.tv_sec, utx->ut_tv.tv_usec, 
                        latestTime.tv_sec, latestTime.tv_usec
                    );
                }
                enabled = false;
            }
            
            // In utmp mode, discard any DEAD_PROCESS events the first time around.  
            // Without this utmp mode prints a DEAD_PROCESS event for each pseudo-tty 
            // that's been used but is not currently used.  Not helpful.
            
            if ( enabled && (verbose < 2) && firstTime && ! wtmp && (utx->ut_type == DEAD_PROCESS) ) {
                if (verbose == 1) {
                    fprintf(stdout, "*** %s event discarded because it's not meaningful in this mode\n", typeStr);
                }
                enabled = false;
            }

            // Print the event if it's enabled.
            
            if (enabled) {
                if (fieldMask & kTimeMask) {
                    char    timeStr[kTimeFieldWidth + 1];
                    size_t  timeStrLen;
                    
                    if ( timercmp(&utx->ut_tv, &latestTime, >) ) {
                        latestTime = utx->ut_tv;
                        if (verbose >= 2) {
                            fprintf(stdout, "*** latest time %ld.%d\n", latestTime.tv_sec, latestTime.tv_usec);
                        }
                    }

                    timeStrLen = strftime(timeStr, sizeof(timeStr), "%F %T %Z", localtime(&utx->ut_tv.tv_sec) );
                    assert(timeStrLen != 0);
                    
                    fprintf(stdout, "%-*s ", kTimeFieldWidth, timeStr);
                } else {
                    fprintf(stdout, "%*s ", kTimeFieldWidth, "");
                }
                
                fprintf(stdout, "%-*s ", kTypeFieldWidth, typeStr);

                if (fieldMask & kUIDMask) {
                    fprintf(stdout, "%-*.*s ", kIDFieldWidth, kIDFieldWidth, utx->ut_id);
                } else {
                    fprintf(stdout, "%*s ", kIDFieldWidth, "");
                }

                if (fieldMask & kPIDMask) {
                    fprintf(stdout, "%-*ld ", kPIDFieldWidth, (long) utx->ut_pid);
                } else {
                    fprintf(stdout, "%*s ", kPIDFieldWidth, "");
                }

                if (fieldMask & kUserMask) {
                    fprintf(stdout, "%-*s ", kUserFieldWidth, utx->ut_user);
                } else {
                    fprintf(stdout, "%*s ", kUserFieldWidth, "");
                }

                if (fieldMask & kLineMask) {
                    fprintf(stdout, "%-*s ", kLineFieldWidth, utx->ut_line);
                } else {
                    fprintf(stdout, "%*s ", kLineFieldWidth, "");
                }

                if (fieldMask & kHostMask) {
                    fprintf(stdout, "%s", utx->ut_host);
                }

                fprintf(stdout, "\n");
            }
        } while (true);
        
        // The inner loop is done; we're out of database entries.  If we're not 
        // in wait mode, leave now.
        
        if (waitFD == -1) {
            break;
        }

        // If we are in wait mode, wait.  It's a little bogus that, if we 
        // get an error waiting, we just discard that error.  I could propagate the 
        // error up to my caller, but the mechanics are a little tricky and 
        // the current behaviour isn't truly toxic.
        
        fflush(stdout);
        if (verbose >= 2) {
            fprintf(stdout, "*** start waiting\n");
        }
        bytesRead = read(waitFD, &tokenRead, sizeof(tokenRead));
        if (verbose >= 2) {
            fprintf(stdout, "*** stop waiting\n");
        }
        if (bytesRead == 0) {
            break;
        } else if (bytesRead < 0) {
            if (errno != EINTR) {
                break;
            }
        } else if (bytesRead != sizeof(tokenRead)) {
            assert(false);  // not prepared to handle partial reads
            break;
        } else {
            // Have to swap to native endianness <rdar://problem/5352778>.
            assert( tokenRegistered == (int) ntohl(tokenRead) );
        }

        // Set things up for the next loop.  setutxent[_wtmp] reset the state 
        // of the utx subsystem so that the next call to getutx[_wtmp] resumes 
        // at the start of the log.
        
        firstTime = false;
        if (wtmp) {
            setutxent_wtmp(1);
        } else {
            setutxent();
        }
    } while (true);
}

static void DoGetLastLogCommand(bool byName, const char *arg)
    // Implements the getlastlogx (byName == false) and getlastlogxbyname 
    // (byName == true) commands.
{
    struct lastlogx *   ll;
    char                timeStr[kTimeFieldWidth + 1];
    size_t              timeStrLen;

    assert(arg != NULL);
    
    // Get the lastlogx record.
    
    if (byName) {
        ll = getlastlogxbyname(arg, NULL);
    } else {
        ll = getlastlogx((uid_t) (long) atol(arg), NULL);
    }
    
    // Print it.
    
    if (ll == NULL) {
        fprintf(stdout, "<none>\n");
    } else {
        timeStrLen = strftime(timeStr, sizeof(timeStr), "%F %T %Z", localtime(&ll->ll_tv.tv_sec) );
        assert(timeStrLen != 0);
        
        fprintf(stdout, "%-*s %-*s %s\n", kTimeFieldWidth, "time",  kLineFieldWidth, "line", "host");
        fprintf(stdout, "%-*s %-*s %s\n", kTimeFieldWidth, "----",  kLineFieldWidth, "----", "----");
        fprintf(stdout, "%-*s %-*s %s\n", kTimeFieldWidth, timeStr, kLineFieldWidth, ll->ll_line, ll->ll_host);
    }
    
    free(ll);
}

int main(int argc, char **argv)
{
    int             err;
    int             retVal;
    int             ch;
    size_t          typeIndex;

    assert(UTXTypesOK());

    retVal = EXIT_SUCCESS;
    err = 0;
    if (argc == 1) {
        retVal = EXIT_FAILURE;
    } else {
        // Process the possible commands.
        
        if ( (strcasecmp(argv[1], "getutxent") == 0) || (strcasecmp(argv[1], "getutxent_wtmp") == 0) ) {
            bool            enabledTypes[kNumberOfUTXTypes];
            bool            didSetType;
            uint32_t        verbose;
            bool            waitForMore;
        
            // The "getutxent" and "getutxent_wtmp" commands share a common set of options.

            // Clear out the values that store our options.
            
            for (typeIndex = 0; typeIndex < kNumberOfUTXTypes; typeIndex++) {
                enabledTypes[typeIndex] = false;
            }
            didSetType = false;
            verbose = 0;
            waitForMore = false;
            
            // Parse our command line options.
            
            optind = 2;     // tell getopt to skip the command in argv[1]
            do {
                ch = getopt(argc, argv, "wvt:a");
                if (ch != -1) {
                    switch (ch) {
                        case 'w':
                            waitForMore += 1;
                            break;
                        case 'v':
                            verbose += 1;
                            break;
                        case 't':
                            didSetType = false;
                            for (typeIndex = 0; typeIndex < kNumberOfUTXTypes; typeIndex++) {
                                if ( strcasecmp(optarg, kUTXTypes[typeIndex].name) == 0 ) {
                                    enabledTypes[typeIndex] = true;
                                    didSetType = true;
                                }
                            }
                            if ( ! didSetType ) {
                                retVal = EXIT_FAILURE;
                                break;
                            }
                            break;
                        case 'a':
                            for (typeIndex = 0; typeIndex < kNumberOfUTXTypes; typeIndex++) {
                                enabledTypes[typeIndex] = true;
                            }
                            didSetType = true;
                            break;
                        case '?':
                        default:
                            retVal = EXIT_FAILURE;
                            break;
                    }
                }
            } while (ch != -1);

            // If we had no -t or -a options, default to all enabled.
            
            if ( ! didSetType ) {
                for (typeIndex = 0; typeIndex < kNumberOfUTXTypes; typeIndex++) {
                    enabledTypes[typeIndex] = kUTXTypes[typeIndex].listByDefault;
                }
            }
            
            // Fail if we have extra arguments.
            
            if ( (retVal == EXIT_SUCCESS) && (optind != argc) ) {
                retVal = EXIT_FAILURE;
            }
            
            // Do the command.
            
            if (retVal == EXIT_SUCCESS) {
                DoGetUTXEntCommand( (strcasecmp(argv[1], "getutxent_wtmp") == 0),  waitForMore, verbose, enabledTypes);
            }
        } else if ( (argc == 3) && (strcasecmp(argv[1], "getlastlogx") == 0) && (argv[2][0] != 0) ) {
            char *      endPtr;
            
            (void) strtol(argv[2], &endPtr, 10);
            if (*endPtr == 0) {
                // strtol consumed all of the string, thus it's a valid number.
                DoGetLastLogCommand(false, argv[2]);
            } else {
                retVal = EXIT_FAILURE;
            }
        } else if ( (argc == 3) && (strcasecmp(argv[1], "getlastlogxbyname") == 0) && (argv[2][0] != 0) ) {
            DoGetLastLogCommand(true, argv[2]);
        } else {
            retVal = EXIT_FAILURE;
        }
    }

    // Report failure.
    
    if (retVal == EXIT_FAILURE) {
        if (err == 0) {
            fprintf(stderr, "usage: %s command [options] [arguments]\n", getprogname());
            fprintf(stderr, "    commands:\n");
            fprintf(stderr, "        getutxent      [-v] [-w] [-a] [-t type]...\n");
            fprintf(stderr, "        getutxent_wtmp [-v] [-w] [-a] [-t type]...\n");
            fprintf(stderr, "            options:\n");
            fprintf(stderr, "                 -v         verbose\n");
            fprintf(stderr, "                 -w         wait for changes\n");
            fprintf(stderr, "                 -a         show all types of entries\n");
            fprintf(stderr, "                 -t type    show entries of this type; type values listed below;\n");
            fprintf(stderr, "                            values marked with * are not shown by default\n");
            for (typeIndex = 0; typeIndex < kNumberOfUTXTypes; typeIndex++) {
                fprintf(stderr, "%*s%s%s\n", 32, "", kUTXTypes[typeIndex].name, (kUTXTypes[typeIndex].listByDefault ? "" : " *"));
            }
//          fprintf(stderr, "        getutxid type\n");      *** maybe someday in the future
//          fprintf(stderr, "        getutxline tty\n");
            fprintf(stderr, "        getlastlogx uid\n");
            fprintf(stderr, "        getlastlogxbyname username\n");
        } else if (err != ECANCELED) {
            fprintf(stderr, "%s: failed with error: %s (%d)\n", getprogname(), strerror(err), err);
        }
    }

    return retVal;
}
