/*
 *  EthernetSocketTool.c
 *  BSDLLCTest
 *

	Copyright:	Copyright (c) 1998-2003 by Apple Computer, Inc., All Rights Reserved.

	Disclaimer:	IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc.
 ("Apple") in consideration of your agreement to the following terms, and your
 use, installation, modification or redistribution of this Apple software
 constitutes acceptance of these terms.  If you do not agree with these terms,
 please do not use, install, modify or redistribute this Apple software.

 In consideration of your agreement to abide by the following terms, and subject
 to these terms, Apple grants you a personal, non-exclusive license, under Apple’s
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


 *
 */

#include "BSDLLCTestCommon.h"
#include "MoreUNIX.h"
#include "MoreSecurity.h"
#include "MoreCFQ.h"
#include "EthernetSocketStuff.h"
#include <Carbon/Carbon.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/fcntl.h>
#include <sys/errno.h>
#include <sys/ipc.h>
#include <sys/uio.h>
#include <sys/un.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mach-o/dyld.h>

static OSStatus OpenPFNDRVSocketCommand(CFDictionaryRef *result);

/*
	OpenPFNDRVSocketCommand takes the CFDictionaryRef argument passed to the 
	tool by the application and opens the PFNDRV socket.
*/
static OSStatus OpenPFNDRVSocketCommand(CFDictionaryRef *result)
{
	OSStatus			err;
	CFMutableArrayRef	descArray;
	CFNumberRef			descNum;
	int					fd;
	
	assert( result != NULL);
	assert(*result == NULL);
	
	descArray = NULL;
	
	// create the mutable array which shall be used to return the opened
	// socket 
	err = CFQArrayCreateMutable(&descArray);
	if (err == noErr)
	{
		// make sure we are in privileged mode
		MoreSecSetPrivilegedEUID();
#if MORE_DEBUG
		fprintf(stderr, "About to open the raw socket\n");
#endif
		// open a PF_NDRV socket - requires privedged access
		fd = socket(AF_NDRV, SOCK_RAW, 0);

		// done making priviledged call
		// disable privileged mode
		MoreSecTemporarilySetNonPrivilegedEUID();
		
		err = EXXXToOSStatus( MoreUNIXErrno(fd) );
		if (err == noErr)
		{
#if MORE_DEBUG
			fprintf(stderr, "raw socket opened  %d\n", fd);
#endif
			descNum = CFNumberCreate(NULL, kCFNumberIntType, &fd);
			if (descNum == NULL) 
			{
				err = coreFoundationUnknownErr;
			}
		}
		
		if (err == noErr) 
		{
			CFArrayAppendValue(descArray, descNum);
		}
		
		// Clean up
		CFQRelease(descNum);	
	}
	
	if (err == noErr)
	{
		CFStringRef key;
		
		// since we are passing back a file descriptor, we must set the key to the
		// magic flag kMoreSecFileDescriptorsKey, which MoreAuth will special case
		// when returning the file descriptor to the application.
		key = kMoreSecFileDescriptorsKey;
		*result = CFDictionaryCreate(NULL, (const void **) &key, (const void **) &descArray, 1, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
		err = CFQError(*result);
	}
	
	if (err != noErr)
	{
		MoreSecCloseDescriptorArray(descArray);
	}
	CFQRelease(descArray);
	
	assert( (err == noErr) == (*result != NULL) );

#if MORE_DEBUG
	fprintf(stderr, "tool returning error  %d\n", err);
#endif


	return err;
}

static OSStatus GetPFNDRVSocketToolCommandProc(AuthorizationRef auth, CFDictionaryRef request, CFDictionaryRef *result)
// Our command callback for MoreSecHelperToolMain.  Extracts
// the command name from the request dictionary and calls
// through to the appropriate command handler (in this case
// there's only one).
{
    OSStatus 			err;
    CFStringRef 		command;
    int					fd = 0;
    
    assert(auth != NULL);
    assert(request != NULL);
    assert( result != NULL);
    assert(*result == NULL);
    assert(geteuid() == getuid());
    
    err = kMyAuthorizedCommandSuccess;
    command = (CFStringRef) CFDictionaryGetValue(request, kBSDLLCTestCommandNameKey);
    if ((command == NULL) || (CFGetTypeID(command) != CFStringGetTypeID()))
    {
        err = kMyAuthorizedCommandInternalError;
    }
    
    if (err == kMyAuthorizedCommandSuccess)
    {
        if (CFEqual(command, kBSDLLCTestGetPFNDRVSocket))
        {
            static const char *kRightName = "com.apple.dts.BSDLLCTest.GetPFNDRVSocketTool";
            static const AuthorizationFlags kAuthFlags = kAuthorizationFlagDefaults
                | kAuthorizationFlagInteractionAllowed
                | kAuthorizationFlagExtendRights
                ;
            AuthorizationItem   right  = { kRightName, 0, NULL, 0 };
            AuthorizationRights rights = { 1, &right };
            			// Before doing our privileged work, acquire an authorization right.
               // This allows the system administrator to configure the system
               // (via "/etc/authorization") for the security level that they want.
               //
               // Unfortunately, the default rule in "/etc/authorization" always
               // triggers a password dialog.  Right now, there's no way around
               // this [2939908].  One commonly accepted workaround is to not
               // acquire a authorization right (ie don't call AuthorizationCopyRights
               // here) but instead limit your tool in some other way.  For example,
               // an Internet setup assistant helper tool might only allow the user
               // to modify network locations that they created.

            err = AuthorizationCopyRights(auth, &rights, kAuthorizationEmptyEnvironment, kAuthFlags, NULL);

#if MORE_DEBUG
            fprintf(stderr, "BSDLLCTest: GetPFNDRVSocketTool: ACR returned %ld\n", err);
#endif

            if (err == kMyAuthorizedCommandSuccess)
            {
				err = OpenPFNDRVSocketCommand(result);
			}
        }
    }
                            
    return err;
}
        
int main(int argc, const char *argv[])
{
    int 				err;
    int 				result;
    AuthorizationRef 	auth;

    // It's vital that we get any auth ref passed to us from
    // AuthorizationExecuteWithPrivileges before we call
    // MoreSecDestroyInheritedEnvironment, because AEWP passes its
    // auth ref to us via the environment.
    //
    // auth may come back as NULL, and that's just fine.  It signals
    // that we're not being executed by AuthorizationExecuteWithPrivileges.
    auth = MoreSecHelperToolCopyAuthRef();
    // Because we're normally running as a setuid root program, it's
    // important that we not trust any information coming to us from
    // our potentially malicious parent process.
    // MoreSecDestroyInheritedEnvironment eliminates all sources of
    // such information, so we can't depend on it ever if we try.
    err = MoreSecDestroyInheritedEnvironment(kMoreSecKeepStandardFilesMask, argv);
    // Mask SIGPIPE, otherwise stuff won't work properly.
    if (err == 0)
    {
        err = MoreUNIXIgnoreSIGPIPE();
    }
    // Call the MoreSecurity helper routine.
    if (err == 0)
    {
        err = MoreSecHelperToolMain(STDIN_FILENO, STDOUT_FILENO, auth, GetPFNDRVSocketToolCommandProc, argc, argv);
    }


    // Map the error code to a tool result.
    result = MoreSecErrorToHelperToolResult(err);

#if MORE_DEBUG
	fprintf(stderr, "  err    = %d\n", err);
	fprintf(stderr, "  result = %d\n", result);
#endif

    return result;
}
