/*
    File:       SimpleDial.c

    Contains:   Demonstrates the System Configuration framework network connection APIs.

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

$Log: SimpleDial.c,v $
Revision 1.2  2004/07/13 15:25:45         
Work around a problem with the callback get incorrect status values.  Tidy up the termination of the runloop.  Add "\n" to some text strings that needed them.

Revision 1.1  2004/04/28 15:44:28         
First checked in.


*/

#include <assert.h>
#include <unistd.h>

#include <CoreFoundation/CoreFoundation.h>
#include <SystemConfiguration/SystemConfiguration.h>

static const char * StatusToString(SCNetworkConnectionStatus status)
    // Returns a string representation of the network connection status.
{
    const char *        result;
    static const char * kStatusStrings[] = { 
        "kSCNetworkConnectionInvalid",
        "kSCNetworkConnectionDisconnected",
        "kSCNetworkConnectionConnecting",
        "kSCNetworkConnectionConnected",
        "kSCNetworkConnectionDisconnecting"
    };
    
    if (  (status < kSCNetworkConnectionInvalid) 
       || (status > kSCNetworkConnectionDisconnecting) 
       ) {
        result = "unknown";
    } else {
        result = kStatusStrings[status + 1];
    }
    return result;
}

static const char * MinorStatusToString(
    SCNetworkConnectionPPPStatus minorStatus
)
    // Returns a string representation of the minor connection status.
{
    const char *        result;
    static const char * kMinorStatusStrings[] = { 
        "kSCNetworkConnectionPPPDisconnected", 
        "kSCNetworkConnectionPPPInitializing", 
        "kSCNetworkConnectionPPPConnectingLink", 
        "kSCNetworkConnectionPPPDialOnTraffic", 
        "kSCNetworkConnectionPPPNegotiatingLink", 
        "kSCNetworkConnectionPPPAuthenticating", 
        "kSCNetworkConnectionPPPWaitingForCallBack", 
        "kSCNetworkConnectionPPPNegotiatingNetwork", 
        "kSCNetworkConnectionPPPConnected", 
        "kSCNetworkConnectionPPPTerminating", 
        "kSCNetworkConnectionPPPDisconnectingLink", 
        "kSCNetworkConnectionPPPHoldingLinkOff", 
        "kSCNetworkConnectionPPPSuspended", 
        "kSCNetworkConnectionPPPWaitingForRedial"
    };
    
    if (  (minorStatus < kSCNetworkConnectionPPPDisconnected) 
       || (minorStatus > kSCNetworkConnectionPPPWaitingForRedial) 
       ) {
        result = "unknown";
    } else {
        result = kMinorStatusStrings[minorStatus];
    }
    return result;
}

static SCNetworkConnectionPPPStatus GetMinorStatus(
    SCNetworkConnectionRef connection
)
    // Gets the minor connection status from the extended status 
    // dictionary associated with the connection.  Returns -1 if 
    // it can't get the status, for whatever reason.
{
    SCNetworkConnectionPPPStatus    result;
    CFDictionaryRef                 statusDict;
    CFDictionaryRef                 pppDict;
    CFNumberRef                     minorStatusNum;
    
    result = -1;
    
    // Get the extended status dictionary.
    
    statusDict = SCNetworkConnectionCopyExtendedStatus(connection);
    if (statusDict != NULL) {
    
        // Extract the PPP sub-dictionary.
        
        pppDict = CFDictionaryGetValue(statusDict, kSCEntNetPPP);
        
        if (  (pppDict != NULL) 
           && (CFGetTypeID(pppDict) == CFDictionaryGetTypeID()) 
           ) {
           
            // Extract the minor status value.
            
            minorStatusNum = CFDictionaryGetValue(
                pppDict, 
                kSCPropNetPPPStatus
            );
            
            if (  (minorStatusNum != NULL) 
               && (CFGetTypeID(minorStatusNum) == CFNumberGetTypeID()) 
               ) {
                SInt32 tmp;
                
                if ( CFNumberGetValue(
                         minorStatusNum, 
                        kCFNumberSInt32Type, &tmp
                     ) 
                   ) {
                    result = tmp;
                }
            }
        }
        CFRelease(statusDict);
    }
    return result;
}

enum {
    kCallbackParamsMagic = 0x66642666
};

struct CallbackParams {
    int                             magic;      // kCallbackParamsMagic
    Boolean                         forcePrintStatus;
    SCNetworkConnectionStatus       lastMajorStatus;
    SCNetworkConnectionPPPStatus    lastMinorStatus;
};
typedef struct CallbackParams CallbackParams;

static void MyNetworkConnectionCallBack(
    SCNetworkConnectionRef          connection,
    SCNetworkConnectionStatus       status,
    void                            *info
)
    // Our network connection callback.  Called out of the runloop 
    // when there's a change in the status of the network connection.
    // It can be called as part of both a connection attempt and a 
    // disconnection attempt.
    //
    // In response to this callback we do two things:
    //
    // 1. Print the current connection status.
    // 2. Once the [dis]connection attempt is resolved (the status 
    //    hits either 'connected' or 'disconnected'), we stop 
    //    the runloop.  This has the effect of breaking the "main" 
    //    function out of its called to CFRunLoopRun, after which 
    //    it can examine the results of the connection.
    //
    // The "info" parameter is a pointer to our per-connection data. 
    // In this case we use this as a pointer to a CallbackParams 
    // structure.  We use this to track the previous state of the 
    // connection so that we don't print redundant status changes.
{
    CallbackParams *                params;
    SCNetworkConnectionPPPStatus    minorStatus;
    time_t                          now;
    struct tm                       nowLocal;
    char                            nowLocalStr[30];
    Boolean                         printMinorStatus;
    
    assert(connection != NULL);
    assert(info != NULL);
    params = (CallbackParams *) info;
    assert(params->magic = kCallbackParamsMagic);

    // Get a string that represents the current time.
    
    (void) time(&now);
    (void) localtime_r(&now, &nowLocal);
    (void) strftime(nowLocalStr, sizeof(nowLocalStr), "%X", &nowLocal);

    // Due to a bug <rdar://problem/3725976>, it's best to get the major status via 
    // SCNetworkConnectionGetStatus than rely on the value being passed into 
    // the callback.

    status = SCNetworkConnectionGetStatus(connection);
    
    // Get the minor status from the extended status associated with 
    // the connection.
    
    minorStatus = GetMinorStatus(connection);
    
    // Print any status changes.
    
    printMinorStatus = (params->lastMinorStatus != minorStatus);
    if (  (params->forcePrintStatus) 
       || (params->lastMajorStatus != status) 
       ) {
        fprintf(
            stderr, 
            "%s %s (%ld)\n", 
            nowLocalStr, 
            StatusToString(status), 
            (long) status
        );
        printMinorStatus = true;
        params->forcePrintStatus = false;
    }
    if (printMinorStatus) {
        fprintf(
            stderr, 
            "%s     %s (%ld)\n", 
            nowLocalStr, 
            MinorStatusToString(minorStatus), 
            (long) minorStatus
        );
    }

    // If we hit either the connected or disconnected state, 
    // we signal the runloop to stop so that the main function 
    // can process the result of the [dis]connection attempt.
    
    if (  (  minorStatus == kSCNetworkConnectionPPPDisconnected ) 
       || (  minorStatus == kSCNetworkConnectionPPPConnected    ) 
       ) {
        CFRunLoopStop(CFRunLoopGetCurrent());
    }

    // Record the new status.
    
    params->lastMajorStatus = status;
    params->lastMinorStatus = minorStatus;
}

int main (int argc, const char * argv[])
    // This program finds the first PPP connection service in the 
    // current network configuration and then connects and 
    // disconnects that service.
{
    int                     err;
    Boolean                 ok;
    CFStringRef             serviceToDial;
    CFDictionaryRef         optionsForDial;
    SCNetworkConnectionRef  connection;
    CallbackParams          params;
    
    serviceToDial  = NULL;
    optionsForDial = NULL;
    connection     = NULL;
    
    // If we're run without an argument, just print the usage.
    
    err = 0;
    if (argc != 1) {
        const char *programName;
        
        programName = strrchr(argv[0], '/');
        if (programName == NULL) {
            programName = argv[0];
        } else {
            programName += 1;
        }
        fprintf(stderr, "Usage: %s\n", programName);
        err = ECANCELED;
    }

    // Find the serviceID of the PPP service to dial and the user's 
    // preferred dialling options.  This routine picks up the last 
    // service dialled using Internet Connect (and the associated 
    // options) or, if Internet Connect has not been used, returns 
    // the first PPP service listed in the Network preferences pane.
    
    if (err == 0) {
        ok = SCNetworkConnectionCopyUserPreferences(
            NULL, 
            &serviceToDial,
            &optionsForDial
        );
        if ( ! ok ) {
            err = SCError();
        }
    }
    
    // Create a SCNetworkConnectionRef for it.
    
    if (err == 0) {
        SCNetworkConnectionContext context;

        // Set up the parameters to our callback function.
        
        params.magic            = kCallbackParamsMagic;
        params.forcePrintStatus = true;
        params.lastMinorStatus  = kSCNetworkConnectionDisconnected;
        params.lastMinorStatus  = kSCNetworkConnectionPPPDisconnected;
        
        // Set up the context to reference those parameters.
        
        context.version         = 0;
        context.info            = (void *) &params;
        context.retain          = NULL;
        context.release         = NULL;
        context.copyDescription = NULL;
        
        connection = SCNetworkConnectionCreateWithServiceID(
            NULL,
            serviceToDial, 
            MyNetworkConnectionCallBack,
            &context
        );
        if (connection == NULL) {
            err = SCError();
        }
    }

    // Schedule our callback with the runloop.
    
    if (err == 0) {
        ok = SCNetworkConnectionScheduleWithRunLoop(
            connection,
            CFRunLoopGetCurrent(),
            kCFRunLoopDefaultMode
        );
        if ( ! ok ) {
            err = SCError();
        }
    }
    
    // Check the status.  If we're already connected tell the user. 
    // If we're not connected, initiate the connection.
    
    if (err == 0) {
        err = ECANCELED;    // Most cases involve us bailing out, 
                            // so set the error here.
        
        switch ( SCNetworkConnectionGetStatus(connection) ) {
            case kSCNetworkConnectionDisconnected:
                err = 0;
                break;
            case kSCNetworkConnectionConnecting:
                fprintf(stderr, "Service is already connecting.\n");
                break;
            case kSCNetworkConnectionDisconnecting:
                fprintf(stderr, "Service is disconnecting.\n");
                break;
            case kSCNetworkConnectionConnected:
                fprintf(stderr, "Service is already connected.\n");
                break;
            case kSCNetworkConnectionInvalid:
                fprintf(stderr, "Service is invalid.  Weird.\n");
                break;
            default:
                fprintf(stderr, "Unexpected status.\n");
                break;
        }
    }
    
    // Initiate the connection.
    
    if (err == 0) {
        fprintf(stderr, "Connecting...\n");

        ok = SCNetworkConnectionStart(
            connection,
            optionsForDial,
            false
        );
        if ( ! ok ) {
            err = SCError();
        }
    }
    
    // Run the runloop and wait for our connection attempt to be resolved. 
    // Once that happens, print the result.
    
    if (err == 0) {
        CFRunLoopRun();
        
        switch (params.lastMinorStatus) {
            case kSCNetworkConnectionPPPConnected:
                fprintf(stderr, "Connection succeeded\n");
                break;
            case kSCNetworkConnectionPPPDisconnected:
                fprintf(stderr, "Connection failed\n");
                err = ECANCELED;
                break;
            default:
                fprintf(
                    stderr, 
                    "Bad params.lastMinorStatus (%ld)\n", 
                    (long) params.lastMinorStatus
                );
                err = EINVAL;
                break;
        }
    }
    
    // Wait for a few seconds.
    
    if (err == 0) {
        fprintf(stderr, "Waiting for a few seconds...\n");
        
        (void) sleep(5);
    
    // Initiate a disconnect.

        params.forcePrintStatus = true;

        fprintf(stderr, "Disconnecting...\n");

        ok = SCNetworkConnectionStop(
            connection,
            true
        );
        if ( ! ok ) {
            err = SCError();
        }
    }
    
    // Run the runloop and wait for our disconnection attempt to be 
    // resolved.  Once that happens, print the result.

    if (err == 0) {
        CFRunLoopRun();
        
        switch (params.lastMinorStatus) {
            case kSCNetworkConnectionPPPDisconnected:
                fprintf(stderr, "Disconnection succeeded\n");
                break;
            case kSCNetworkConnectionPPPConnected:
                fprintf(stderr, "Disconnection failed\n");
                err = ECANCELED;
                break;
            default:
                fprintf(
                    stderr, 
                    "Bad params.lastMinorStatus (%ld)\n", 
                    (long) params.lastMinorStatus
                );
                err = EINVAL;
                break;
        }
    }

    // Clean up.
    
    if (serviceToDial != NULL) {
        CFRelease(serviceToDial);
    }
    if (optionsForDial != NULL) {
        CFRelease(optionsForDial);
    }
    if (connection != NULL) {
        (void) SCNetworkConnectionUnscheduleFromRunLoop(
            connection,
            CFRunLoopGetCurrent(),
            kCFRunLoopDefaultMode
        );
        CFRelease(connection);
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
