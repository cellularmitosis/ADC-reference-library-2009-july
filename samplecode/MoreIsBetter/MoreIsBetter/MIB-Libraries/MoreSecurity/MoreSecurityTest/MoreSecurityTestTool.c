/*
	File:		MoreSecurityTestTool.c

	Contains:	Helper tool for the MoreSecurityTest program.

	Written by:	DTS

	Copyright:	Copyright � 2002 by Apple Computer, Inc., All Rights Reserved.

	Disclaimer:	IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc.
				("Apple") in consideration of your agreement to the following terms, and your
				use, installation, modification or redistribution of this Apple software
				constitutes acceptance of these terms.  If you do not agree with these terms,
				please do not use, install, modify or redistribute this Apple software.

				In consideration of your agreement to abide by the following terms, and subject
				to these terms, Apple grants you a personal, non-exclusive license, under Apple�s
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

$Log: MoreSecurityTestTool.c,v $
Revision 1.5  2003/05/25 12:25:12         
Added the CVS Log keyword.  It's a mystery why it wasn't already included.


*/

/////////////////////////////////////////////////////////////////

// System interfaces

#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>

// MoreIsBetter interfaces

#include "MoreUNIX.h"
#include "MoreSecurity.h"
#include "MoreCFQ.h"

// Our interfaces

#include "MoreSecurityTestCommon.h"

/////////////////////////////////////////////////////////////////

static OSStatus DoGetUIDsCommand(CFDictionaryRef *result)
	// Executes a very simple command, which just returns 
	// the EUID, RUID, and SUID of the helper tool.
{
	OSStatus 	err;
	int 		junk;
	uid_t 		euid;
	uid_t 		ruid;
	uid_t 		suid;
	CFStringRef	keys[3];
	CFNumberRef	values[3];
	long long	tmp;

	assert( result != NULL);
	assert(*result == NULL);

	euid = geteuid();
	ruid = getuid();

	// There's no direct accessor for the SUID, so I try to 
	// set the EUID to 0.  If that works, our SUID must be 0 (-:
	
	if ( (euid != 0) && (MoreSecSetPrivilegedEUID() == 0) ) {
		suid = 0;
		junk = MoreSecTemporarilySetNonPrivilegedEUID();
		assert(junk == 0);
	} else {
		suid = ruid;
	}
	
	keys[0] = kMoreSecurityTestGetUIDsResponseRUID;
	keys[1] = kMoreSecurityTestGetUIDsResponseEUID;
	keys[2] = kMoreSecurityTestGetUIDsResponseSUID;
	
	tmp = euid;
	values[0] = CFNumberCreate(NULL, kCFNumberLongLongType, &tmp);
	tmp = ruid;
	values[1] = CFNumberCreate(NULL, kCFNumberLongLongType, &tmp);
	tmp = suid;
	values[2] = CFNumberCreate(NULL, kCFNumberLongLongType, &tmp);
	
	err = noErr;
	if (values[0] == NULL || values[1] == NULL || values[2] == NULL) {
		err = coreFoundationUnknownErr;
	}
	if (err == noErr) {
		*result = CFDictionaryCreate(NULL, (const void **) &keys, (const void **) &values, 3, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
		err = CFQError(*result);
	}
	
	CFQRelease(values[0]);
	CFQRelease(values[1]);
	CFQRelease(values[2]);

	assert( (err == noErr) == (*result != NULL) );
		
	return err;
}

static OSStatus OpenAndBindDescAndAppendToArray(UInt16 port, CFMutableArrayRef descArray)
{
	OSStatus 	err;
	int 		junk;
	int 		desc;
	CFNumberRef	descNum;
	
	descNum = NULL;
	
	desc = socket(AF_INET, SOCK_STREAM, 0);
	err = EXXXToOSStatus( MoreUNIXErrno(desc) );
	if (err == noErr) {
		descNum = CFNumberCreate(NULL, kCFNumberIntType, &desc);
		if (descNum == NULL) {
			err = coreFoundationUnknownErr;
		}
	}
	if (err == noErr) {
		struct sockaddr_in addr;

		memset(&addr, 0, sizeof(addr));
		addr.sin_len    = sizeof(addr);
		addr.sin_family = AF_INET;
		addr.sin_port   = htons(port);

		err = MoreSecSetPrivilegedEUID();
		if (err == 0) {
			static const int kOne = 1;

			err = setsockopt(desc, SOL_SOCKET, SO_REUSEADDR, (void *)&kOne, sizeof(kOne));
			err = MoreUNIXErrno(err);

			if (err == 0) {
		    	err = bind(desc, (struct sockaddr *) &addr, sizeof(addr));
				err = MoreUNIXErrno(err);
			}
			
			(void) MoreSecTemporarilySetNonPrivilegedEUID();
		}
		err = EXXXToOSStatus(err);
	}
	if (err == noErr) {
		CFArrayAppendValue(descArray, descNum);
	}
	
	// Clean up.
	
	if (err != noErr) {
		if (desc != -1) {
			junk = close(desc);
			assert(junk == 0);
		}
	}
	CFQRelease(descNum);
	
	return err;
}

static OSStatus DoLowNumberedPortsCommand(CFDictionaryRef *result)
{
	OSStatus			err;
	CFMutableArrayRef	descArray;
	
	assert( result != NULL);
	assert(*result == NULL);
	
	descArray = NULL;
	
	err = CFQArrayCreateMutable(&descArray);
	if (err == noErr) {
		err = OpenAndBindDescAndAppendToArray(130, descArray);
	}
	if (err == noErr) {
		err = OpenAndBindDescAndAppendToArray(131, descArray);
	}
	if (err == noErr) {
		err = OpenAndBindDescAndAppendToArray(132, descArray);
	}
	if (err == noErr) {
		CFStringRef key;
		
		key = kMoreSecFileDescriptorsKey;
		*result = CFDictionaryCreate(NULL, (const void **) &key, (const void **) &descArray, 1, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
		err = CFQError(*result);
	}

	if (err != noErr) {
		MoreSecCloseDescriptorArray(descArray);
	}	
	CFQRelease(descArray);
	
	assert( (err == noErr) == (*result != NULL) );
		
	return err;
}

static OSStatus TestToolCommandProc(AuthorizationRef auth, CFDictionaryRef request, CFDictionaryRef *result)
	// Our command callback for MoreSecHelperToolMain.  Extracts 
	// the command name from the request dictionary and calls 
	// through to the appropriate command handler (in this case 
	// there's only one).
{
	OSStatus 	err;
	CFStringRef command;
	
	assert(auth != NULL);
	assert(request != NULL);
	assert( result != NULL);
	assert(*result == NULL);
	assert(geteuid() == getuid());
	
	err = noErr;
	command = (CFStringRef) CFDictionaryGetValue(request, kMoreSecurityTestCommandNameKey);
	if (   (command == NULL) 
		|| (CFGetTypeID(command) != CFStringGetTypeID()) ) {
		err = paramErr;
	}
	if (err == noErr) {
		if (CFEqual(command, kMoreSecurityTestGetUIDsCommand)) {
		    static const char *kRightName = "com.apple.dts.MoreIsBetter.MoreSecurity.TestTool";
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
			
			#if MORE_DEBUG
				fprintf(stderr, "MoreSecurityTest: HelperTool: Calling ACR\n");
			#endif

			err = AuthorizationCopyRights(auth, &rights, kAuthorizationEmptyEnvironment, kAuthFlags, NULL);

			#if MORE_DEBUG
				fprintf(stderr, "MoreSecurityTest: HelperTool: ACR returned %ld\n", err);
			#endif

			if (err == noErr) {
				err = DoGetUIDsCommand(result);
			}
		} else if (CFEqual(command, kMoreSecurityTestLowNumberPortCommand)) {
			// On the other hand, in this example, opening these low-numbered ports is 
			// not considered a privileged operation, and so we don't acquire a right 
			// before doing it.
			
			err = DoLowNumberedPortsCommand(result);
		} else {
			err = paramErr;
		}
	}
	return err;
}

int main(int argc, const char *argv[])
	// Our main function.  Apart from comments and debugging, this looks 
	// remarkably like the template shown in "MoreSecurity.h" (-:
{	
	int 				err;
	int 				result;
	AuthorizationRef 	auth;
	
	#if MORE_DEBUG
		if (1) {
			fprintf(stderr, "PID %qd starting\n", (long long) getpid());
			fprintf(stderr, "  EUID = %ld\n", (long) geteuid());
			fprintf(stderr, "  RUID = %ld\n", (long) getuid());
			if (0) {
				fprintf(stderr, "Waiting for debugger\n");
				(void) pause();
			}
		}
	#endif
	
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
	
	if (err == 0) {
		err = MoreUNIXIgnoreSIGPIPE();
	}
	
	// Call the MoreSecurity helper routine.
	
	if (err == 0) {
		err = MoreSecHelperToolMain(STDIN_FILENO, STDOUT_FILENO, auth, TestToolCommandProc, argc, argv);
	}

	// Map the error code to a tool result.
	
	result = MoreSecErrorToHelperToolResult(err);

	#if MORE_DEBUG
		if (1) {
			fprintf(stderr, "PID %qd stopping\n", (long long) getpid());
			fprintf(stderr, "  err    = %d\n", err);
			fprintf(stderr, "  result = %d\n", result);
		}
	#endif
	
	return result;
}
