/*
	File:       SampleTool.c

    Contains:   Helper tool side of the example of how to use BetterAuthorizationSampleLib.

    Written by: DTS

    Copyright:  Copyright (c) 2007 Apple Inc. All Rights Reserved.

    Disclaimer: IMPORTANT:  This Apple software is supplied to you by Apple, Inc.
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
                Apple, Inc. may be used to endorse or promote products derived from the
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

 */
 
#include <netinet/in.h>
#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>

#include <CoreServices/CoreServices.h>

#include "BetterAuthorizationSampleLib.h"

#include "SampleCommon.h"

/////////////////////////////////////////////////////////////////
#pragma mark ***** Get Version Command

static OSStatus DoGetVersion(
	AuthorizationRef			auth,
    const void *                userData,
	CFDictionaryRef				request,
	CFMutableDictionaryRef      response,
    aslclient                   asl,
    aslmsg                      aslMsg
)
    // Implements the kSampleGetVersionCommand.  Returns the version number of 
    // the helper tool.
{	
	OSStatus					retval = noErr;
	CFNumberRef					value;
    static const int kCurrentVersion = 17;          // something very easy to spot
	
	// Pre-conditions
	
	assert(auth != NULL);
    // userData may be NULL
	assert(request != NULL);
	assert(response != NULL);
    // asl may be NULL
    // aslMsg may be NULL
	
    // Add them to the response.
    
	value = CFNumberCreate(NULL, kCFNumberIntType, &kCurrentVersion);
	if (value == NULL) {
		retval = coreFoundationUnknownErr;
    } else {
        CFDictionaryAddValue(response, CFSTR(kSampleGetVersionResponse), value);
	}
	
	if (value != NULL) {
		CFRelease(value);
	}

	return retval;
}

/////////////////////////////////////////////////////////////////
#pragma mark ***** Get UIDs Command

static OSStatus DoGetUID(
	AuthorizationRef			auth,
    const void *                userData,
	CFDictionaryRef				request,
	CFMutableDictionaryRef      response,
    aslclient                   asl,
    aslmsg                      aslMsg
)
    // Implements the kSampleGetUIDsCommand.  Gets the process's three UIDs and 
    // adds them to the response dictionary.
{	
	OSStatus					retval = noErr;
    int                         err;
	uid_t						euid;
	uid_t						ruid;
	CFNumberRef					values[2];
	long long					tmp;
	
	// Pre-conditions
	
	assert(auth != NULL);
    // userData may be NULL
	assert(request != NULL);
	assert(response != NULL);
    // asl may be NULL
    // aslMsg may be NULL
	
    // Get the UIDs.
    
	euid = geteuid();
	ruid = getuid();
	
	err = asl_log(asl, aslMsg, ASL_LEVEL_DEBUG, "euid=%ld, ruid=%ld", (long) euid, (long) ruid);
    assert(err == 0);
	
    // Add them to the response.
    
	tmp = euid;
	values[0] = CFNumberCreate(NULL, kCFNumberLongLongType, &tmp);
	tmp = ruid;
	values[1] = CFNumberCreate(NULL, kCFNumberLongLongType, &tmp);
	
	if ( (values[0] == NULL) || (values[1] == NULL) ) {
		retval = coreFoundationUnknownErr;
    } else {
        CFDictionaryAddValue(response, CFSTR(kSampleGetUIDsResponseRUID), values[0]);
        CFDictionaryAddValue(response, CFSTR(kSampleGetUIDsResponseEUID), values[1]);
	}
	
	if (values[0] != NULL) {
		CFRelease(values[0]);
	}
	if (values[1] != NULL) {
		CFRelease(values[1]);
	}

	return retval;
}

/////////////////////////////////////////////////////////////////
#pragma mark ***** Low-Numbered Ports Command

static OSStatus OpenAndBindDescAndAppendToArray(
	uint16_t					port,               // in host byte order
	CFMutableArrayRef			descArray,
    aslclient                   asl,
    aslmsg                      aslMsg
)
    // A helper routine for DoGetLowNumberedPorts.  Opens a TCP port and 
    // stashes the resulting descriptor in descArray.
{
	OSStatus                    retval;
	int							err;
	int							desc;
	CFNumberRef					descNum;
	
	// Pre-conditions
	
	assert(port != 0);
	assert(descArray != NULL);
    // asl may be NULL
    // aslMsg may be NULL
	
	descNum = NULL;
	
    retval = noErr;
	desc = socket(AF_INET, SOCK_STREAM, 0);
    if (desc < 0) {
        retval = BASErrnoToOSStatus(errno);
    }
	if (retval == noErr) {
		descNum = CFNumberCreate(NULL, kCFNumberIntType, &desc);
		if (descNum == NULL) {
			retval = coreFoundationUnknownErr;
		}
	}
	if (retval == 0) {
		struct sockaddr_in addr;

		memset(&addr, 0, sizeof(addr));
		addr.sin_len    = sizeof(addr);
		addr.sin_family = AF_INET;
		addr.sin_port   = htons(port);
		
        static const int kOne = 1;

        err = setsockopt(desc, SOL_SOCKET, SO_REUSEADDR, (void *)&kOne, sizeof(kOne));
        if (err < 0) {
            retval = BASErrnoToOSStatus(errno);
        }

        if (retval == noErr) {
            err = bind(desc, (struct sockaddr *) &addr, sizeof(addr));
            if (err < 0) {
                retval = BASErrnoToOSStatus(errno);
            }
        }
	}
	if (retval == noErr) {
		CFArrayAppendValue(descArray, descNum);
	}
    if (retval == noErr) {
        err = asl_log(asl, aslMsg, ASL_LEVEL_DEBUG, "Opened port %u", (unsigned int) port);
    } else {
        errno = BASOSStatusToErrno(retval);                         // so that %m can pick it up
        err = asl_log(asl, aslMsg, ASL_LEVEL_ERR, "Failed to open port %u: %m", (unsigned int) port);
    }
    assert(err == 0);
	
	// Clean up.
	
	if ( (retval != noErr) && (desc != -1) ) {
		err = close(desc);
		assert(err == 0);
	}
	if (descNum != NULL) {
		CFRelease(descNum);
	}
	
	return retval;
}

static OSStatus DoGetLowNumberedPorts(
	AuthorizationRef			auth,
    const void *                userData,
	CFDictionaryRef				request,
	CFMutableDictionaryRef      response,
    aslclient                   asl,
    aslmsg                      aslMsg
)
    // Implements the kSampleLowNumberedPortsCommand.  Opens three low-numbered ports 
    // and adds them to the descriptor array in the response dictionary.
{	
	OSStatus					retval = noErr;
	CFMutableArrayRef			descArray = NULL;
	
	// Pre-conditions
    
	assert(auth != NULL);
    // userData may be NULL
	assert(request != NULL);
	assert(response != NULL);
    // asl may be NULL
    // aslMsg may be NULL
	
	descArray = CFArrayCreateMutable(NULL, 0, &kCFTypeArrayCallBacks);
	if (descArray == NULL) {
		retval = coreFoundationUnknownErr;
	}
	
	if (retval == noErr) {
		retval = OpenAndBindDescAndAppendToArray(130, descArray, asl, aslMsg);
	}
	if (retval == noErr) {
		retval = OpenAndBindDescAndAppendToArray(131, descArray, asl, aslMsg);
	}
	if (retval == noErr) {
        if ( CFDictionaryContainsKey(request, CFSTR(kSampleLowNumberedPortsForceFailure)) ) {
            retval = BASErrnoToOSStatus( EADDRINUSE );
        } else {
            retval = OpenAndBindDescAndAppendToArray(132, descArray, asl, aslMsg);
        }
	}
	 
	if (retval == noErr) {
        CFDictionaryAddValue(response, CFSTR(kBASDescriptorArrayKey), descArray);
	}
	
    // Clean up.
    
	if (retval != noErr) {
		BASCloseDescriptorArray(descArray);
	}
	if (descArray != NULL) {
		CFRelease(descArray);
	}
	
	return retval;
}

/////////////////////////////////////////////////////////////////
#pragma mark ***** Tool Infrastructure

/*
    IMPORTANT
    ---------
    This array must be exactly parallel to the kSampleCommandSet array 
    in "SampleCommon.c".
*/

static const BASCommandProc kSampleCommandProcs[] = {
    DoGetVersion,
    DoGetUID,
    DoGetLowNumberedPorts,
    NULL
};

int main(int argc, char **argv)
{
    // Go directly into BetterAuthorizationSampleLib code.
	
    // IMPORTANT
    // BASHelperToolMain doesn't clean up after itself, so once it returns 
    // we must quit.
    
	return BASHelperToolMain(kSampleCommandSet, kSampleCommandProcs);
}
