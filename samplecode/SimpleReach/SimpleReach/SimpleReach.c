/*
    File:       SimpleReach.c

    Contains:   Demonstrates the System Configuration framework reachability APIs.

    Written by: DTS

    Copyright:  Copyright (c) 2004 by Apple Computer, Inc., All Rights Reserved.

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

$Log: SimpleReach.c,v $
Revision 1.2  2004/05/24 21:43:09         
Added comments explain why I have cleanup code even though it's not entirely necessary.

Revision 1.1  2004/04/28 15:44:36         
First checked in.


*/

#include <assert.h>
#include <errno.h>

#include <CoreFoundation/CoreFoundation.h>
#include <SystemConfiguration/SystemConfiguration.h>

static void CleanupTarget(SCNetworkReachabilityRef thisTarget)
    // Disposes of thisTarget.
{
    assert(thisTarget != NULL);
    
    // Ignore the result from SCNetworkReachabilityUnscheduleFromRunLoop. 
    // It basically tells us whether thisTarget was successfully removed 
    // from the runloop.  It will be false if we never added thisTarget 
    // to the runloop, which is a definite possibility under error 
    // conditions.
    
    (void) SCNetworkReachabilityUnscheduleFromRunLoop(
        thisTarget,
        CFRunLoopGetCurrent(),
        kCFRunLoopDefaultMode
    );
    CFRelease(thisTarget);
}

static void PrintHeader(void)
    // Prints an explanation of the flag coding.
{
    fprintf(stdout, "t = kSCNetworkFlagsTransientConnection\n");
    fprintf(stdout, "r = kSCNetworkFlagsReachable\n");
    fprintf(stdout, "c = kSCNetworkFlagsConnectionRequired\n");
    fprintf(stdout, "C = kSCNetworkFlagsConnectionAutomatic\n");
    fprintf(stdout, "i = kSCNetworkFlagsInterventionRequired\n");
    fprintf(stdout, "l = kSCNetworkFlagsIsLocalAddress\n");
    fprintf(stdout, "d = kSCNetworkFlagsIsDirect\n");
    fprintf(stdout, "\n");
}

static void PrintReachabilityFlags(
    const char *                hostname, 
    SCNetworkConnectionFlags    flags, 
    const char *                comment
)
    // Prints a line that records a reachability transition. 
    // This includes the current time, the new state of the 
    // reachability flags (from the flags parameter), and the 
    // name of the host (from the hostname parameter).
{
    time_t      now;
    struct tm   nowLocal;
    char        nowLocalStr[30];
    
    assert(hostname != NULL);
    
    if (comment == NULL) {
        comment = "";
    }
    
    (void) time(&now);
    (void) localtime_r(&now, &nowLocal);
    (void) strftime(nowLocalStr, sizeof(nowLocalStr), "%X", &nowLocal);

    fprintf(stdout, "%s %c%c%c%c%c%c%c %s%s\n",
        nowLocalStr,
        (flags & kSCNetworkFlagsTransientConnection)  ? 't' : '-',
        (flags & kSCNetworkFlagsReachable)            ? 'r' : '-',
        (flags & kSCNetworkFlagsConnectionRequired)   ? 'c' : '-',
        (flags & kSCNetworkFlagsConnectionAutomatic)  ? 'C' : '-',
        (flags & kSCNetworkFlagsInterventionRequired) ? 'i' : '-',
        (flags & kSCNetworkFlagsIsLocalAddress)       ? 'l' : '-',
        (flags & kSCNetworkFlagsIsDirect)             ? 'd' : '-',
        hostname,
        comment
    );
}

static void MyReachabilityCallback(
    SCNetworkReachabilityRef	target,
    SCNetworkConnectionFlags	flags,
    void *                      info
)
    // This routine is a System Configuration framework callback that 
    // indicates that the reachability of a given host has changed.  
    // It's call from the runloop.  target is the host whose reachability 
    // has changed, the flags indicate the new reachability status, and 
    // info is the context parameter that we passed in when we registered 
    // the callback.  In this case, info is a pointer to the host name.
    // 
    // Our response to this notification is simply to print a line 
    // recording the transition.
{
    assert(target != NULL);
    assert(info   != NULL);
    
    PrintReachabilityFlags( (const char *) info, flags, NULL );
}

int main (int argc, const char * argv[])
    // This program treats each of its arguments as a network host 
    // name.  It monitors the reachability of each host, printing 
    // out a record of each change of reachability.
{
    int                             err;
    int                             argIndex;
    SCNetworkReachabilityRef *      targets;
    Boolean                         ok;

    targets = NULL;

    // If we're run with no arguments, just print the usage.
    
    err = 0;
    if (argc == 1) {
        const char *programName;
        
        programName = strrchr(argv[0], '/');
        if (programName == NULL) {
            programName = argv[0];
        } else {
            programName += 1;
        }
        fprintf(stderr, "Usage: %s hostname...\n", programName);
        err = ECANCELED;
    }

    // We allocate an array to hold all of the SCNetworkReachabilityRefs
    // that we create.  The 0'th slot of this array is never used, but 
    // wasting that trivial amount of memory makes the code easier.
    
    if (err == 0) {
        targets = malloc(argc * sizeof(*targets));
        if (targets == NULL) {
            err = ENOMEM;
        }
    }
    
    // Create a SCNetworkReachabilityRef for each argument.
    
    if (err == 0) {
        PrintHeader();
        fprintf(stdout, "Hit ^C to exit.\n");
        fflush(stdout);
        
        for (argIndex = 1; argIndex < argc; argIndex++) {
            SCNetworkReachabilityRef        thisTarget;
            SCNetworkReachabilityContext    thisContext;
            
            thisContext.version         = 0;
            thisContext.info            = (void *) argv[argIndex];
            thisContext.retain          = NULL;
            thisContext.release         = NULL;
            thisContext.copyDescription = NULL;
            
            // Create the target with the name taken from the command 
            // line arguments.
            
            thisTarget = SCNetworkReachabilityCreateWithName(
                NULL, 
                argv[argIndex]
            );
            if (thisTarget == NULL) {
                err = SCError();
            }
      
            // Set our callback and install on the runloop.
            
            if (err == 0) {
                ok = SCNetworkReachabilitySetCallback(
                        thisTarget, 
                        MyReachabilityCallback,
                        &thisContext
                );
                if ( ! ok ) {
                    err = SCError();
                }
            }
            if (err == 0) {
                ok = SCNetworkReachabilityScheduleWithRunLoop(
                    thisTarget, 
                    CFRunLoopGetCurrent(), 
                    kCFRunLoopDefaultMode
                );
                if ( ! ok ) {
                    err = SCError();
                }
            }

            if (err == 0) {
                SCNetworkConnectionFlags flags;
                
                ok = SCNetworkReachabilityGetFlags(thisTarget, &flags);
                if ( ok ) {
                    PrintReachabilityFlags(
                        argv[argIndex], 
                        flags, 
                        " (from main)"
                    );
                } else {
                    err = SCError();
                }
            }
            
            // Record the reference in the targets array, or clean it 
            // up and bail.
            
            if (err == 0) {
                targets[argIndex] = thisTarget;
            } else {
                if (thisTarget != NULL) {
                    CleanupTarget(thisTarget);
                }
                break;
            }
        }
    }
    
    // If everything went well, run our runloop to handle asynchronous 
    // notification of reachability changes.
    
    if (err == noErr) {        
        CFRunLoopRun();
    }
    
    // Clean up.
    
    // The current implementation never gets to this point in the success 
    // case because the way we terminate the program is with ^C (SIGINT), 
    // which just quits the program.  Alas, there's no easy way to 
    // integrate signal handling into a CFRunLoop <rdar://problem/3635076>.  
    // However, I wanted to leave the termination code around, just to show 
    // how to do it properly.
    
    if (targets != NULL) {
        int i;
        
        for (i = 1; i < argIndex; i++) {
            assert(targets[i] != NULL);
            CleanupTarget(targets[i]);
        }
        free(targets);
    }

    if (err == 0) {
        return EXIT_SUCCESS;
    } else {
        if (err != ECANCELED) {
            fprintf(stderr, "Failed with error %d\n.", err);
        }
        return EXIT_FAILURE;
    }
}
