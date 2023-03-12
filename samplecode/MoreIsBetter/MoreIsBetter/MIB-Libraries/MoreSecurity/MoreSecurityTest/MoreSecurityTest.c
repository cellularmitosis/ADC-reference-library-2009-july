/*
	File:		MoreSecurityTest.c

	Contains:	Test program to test MoreSecurity stuff.

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

*/

/////////////////////////////////////////////////////////////////

// System interfaces

#include <Carbon/Carbon.h>

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <fcntl.h>
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

static void DoAbout(void)
	// Displays the about box (with some file descriptor passing test 
	// code left over).
{
	#if 0
		{
			int err;
			int junk;
			int comm[2];
			int	fd;
			
			comm[0] = -1;
			comm[1] = -1;
			fd = -1;
			
			err = socketpair(AF_UNIX, SOCK_STREAM, 0, comm);
			err = EXXXToOSStatus( MoreUNIXErrno(err) );
			if (err == 0) {
				err = MoreUNIXWriteDescriptor(comm[0], 0);
			}
			if (err == 0) {
				err = MoreUNIXReadDescriptor(comm[1], &fd);
			}
			if (comm[0] != -1) {
				junk = close(comm[0]);
				assert(junk == 0);
			}
			if (comm[1] != -1) {
				junk = close(comm[1]);
				assert(junk == 0);
			}
			if (fd != -1) {
				junk = close(fd);
				assert(junk == 0);
			}
		}
	#else
		{
			SInt16 junkHit;
	
			(void) StandardAlert(kAlertPlainAlert, "\pMoreSecurityTest", "\pA simple program to test MoreSecurity.\r\rDTS\r\r� 2002 Apple Computer, Inc.", NULL, &junkHit);
		}
	#endif
}

static void DoUIDTest(void)
	// Called in response to a click of the "UIDs" button. 
	// Demonstrates how to call a setuid root helper tool.
{
	OSStatus 			err;
	OSStatus			junk;
	CFURLRef 			tool;
	CFDictionaryRef 	request;
	CFDictionaryRef 	response;
	AuthorizationRef	auth;
	char				msgStr[256];
	uid_t				ruid;
	uid_t				euid;
	uid_t				suid;
	SInt16				junkHit;
	
	tool     = NULL;
	request  = NULL;
	response = NULL;
	auth 	 = NULL;

	// Create an Authorization Services environment.  Normally your 
	// application would do this as it begins so that it can pre-authorize.
	// However, I don't pre-authorized because a) the pre-authorize flag 
	// does nothing in current versions of Mac OS X [2907852], and b) doing 
	// the pre-authorize triggers two authentication dialogs the first time 
	// you run the application, which is never what you want.
	
    err = AuthorizationCreate(NULL, kAuthorizationEmptyEnvironment, kAuthorizationFlagDefaults, &auth);
    if (err == noErr) {
    	// If we were doing preauthorization, this is where we'd do it.
    }

	// Find our helper tool, possibly restoring it from the template.
	
	if (err == noErr) {
		err = MoreSecCopyHelperToolURLAndCheckBundled(
			CFBundleGetMainBundle(), 
			CFSTR("HelperToolTemplate"), 
			kApplicationSupportFolderType, 
			CFSTR("MoreSecurityTest"), 
			CFSTR("HelperTool"), 
			&tool
		);

		// If the home directory is on an volume that doesn't support 
		// setuid root helper tools, ask the user whether they want to use 
		// a temporary tool.
		
		if (err == kMoreSecFolderInappropriateErr) {
			SInt16 					itemHit;
			AlertStdAlertParamRec 	alertParams;
			
			alertParams.movable       = true;
			alertParams.helpButton    = false;
			alertParams.filterProc    = NULL;
			alertParams.defaultText   = (StringPtr) kAlertDefaultOKText;
			alertParams.cancelText    = (StringPtr) kAlertDefaultCancelText;
			alertParams.otherText     = NULL;
			alertParams.defaultButton = kAlertStdAlertOKButton;
			alertParams.cancelButton  = kAlertStdAlertCancelButton;
			alertParams.position      = kWindowDefaultPosition;
			
			err = StandardAlert(
				kAlertCautionAlert, 
				"\pYour home directory is on a volume that does not support privileged helper tools. "
				"Would you like to use a temporary copy of the tool?", 
				"\pThe temporary tool will be deleted periodically. "
				"In the Finder's Get Info window, uncheck the 'Ignore ownership' on the disk containing your home directory. "
				"Alternatively, ask your system administrator to install the tool for you. ",
				&alertParams, 
				&itemHit
			);
			if ( (err == noErr) && (itemHit == kAlertStdAlertCancelButton) ) {
				err = userCanceledErr;
			}
			
			if (err == noErr) {
				err = MoreSecCopyHelperToolURLAndCheckBundled(
					CFBundleGetMainBundle(), 
					CFSTR("HelperToolTemplate"), 
					kTemporaryFolderType, 
					CFSTR("MoreSecurityTest"), 
					CFSTR("HelperTool"), 
					&tool
		);
			}
		}
	}
	
	// Create the request dictionary.
	
	if (err == noErr) {	
		CFStringRef key;
		CFStringRef value;
		
		key   = kMoreSecurityTestCommandNameKey;
		value = kMoreSecurityTestGetUIDsCommand;
		
		request = CFDictionaryCreate(NULL, (const void **) &key, (const void **) &value, 1, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
		err = CFQError(request);
	}

	// Go go gadget helper tool!

	if (err == noErr) {
		err = MoreSecExecuteRequestInHelperTool(tool, auth, request, &response);
	}
	
	// Extract information from the response.
	
	if (err == noErr) {
		long long tmp;
		
		// CFShow(response);

		err = MoreSecGetErrorFromResponse(response);
		if (err == noErr) {
			err = CFQErrorBoolean( CFNumberGetValue( (CFNumberRef) CFDictionaryGetValue(response, kMoreSecurityTestGetUIDsResponseRUID), kCFNumberLongLongType, &tmp) );
			ruid = (uid_t) tmp;
		}
		if (err == noErr) {
			err = CFQErrorBoolean( CFNumberGetValue( (CFNumberRef) CFDictionaryGetValue(response, kMoreSecurityTestGetUIDsResponseEUID), kCFNumberLongLongType, &tmp) );
			euid = (uid_t) tmp;
		}
		if (err == noErr) {
			err = CFQErrorBoolean( CFNumberGetValue( (CFNumberRef) CFDictionaryGetValue(response, kMoreSecurityTestGetUIDsResponseSUID), kCFNumberLongLongType, &tmp) );
			suid = (uid_t) tmp;
		}
	}

	// Display to the user.
	
	if (err == noErr) {
		(void) sprintf(msgStr, "RUID = %qd, EUID = %qd, SUID = %qd", (long long) ruid, (long long) euid, (long long) suid);
		CopyCStringToPascal(msgStr, (StringPtr) msgStr);
		(void) StandardAlert(kAlertNoteAlert, "\pSuccess", (StringPtr) msgStr, NULL, &junkHit);
	} else {
		(void) sprintf(msgStr, "err = %ld", err);
		CopyCStringToPascal(msgStr, (StringPtr) msgStr);
		(void) StandardAlert(kAlertStopAlert, "\pFailure", (StringPtr) msgStr, NULL, &junkHit);
	}

	// Clean up.
	
	CFQRelease(tool);
	CFQRelease(request);
	CFQRelease(response);
	if (auth != NULL) {	
		junk = AuthorizationFree(auth, kAuthorizationFlagDestroyRights);
		assert(junk == noErr);
	}
}

static void DoLowNumberedPortTest(void)
{
	OSStatus 			err;
	OSStatus			junk;
	CFURLRef 			tool;
	CFDictionaryRef 	request;
	CFDictionaryRef 	response;
	AuthorizationRef	auth;
	char				msgStr[256];
	SInt16				junkHit;
	
	tool     = NULL;
	request  = NULL;
	response = NULL;
	auth 	 = NULL;

    err = AuthorizationCreate(NULL, kAuthorizationEmptyEnvironment, kAuthorizationFlagDefaults, &auth);

	// Find our helper tool, possibly restoring it from the template.
	
	if (err == noErr) {
		err = MoreSecCopyHelperToolURLAndCheckBundled(CFBundleGetMainBundle(), CFSTR("HelperToolTemplate"), 
											  		  kApplicationSupportFolderType, CFSTR("MoreSecurityTest"), CFSTR("HelperTool"), 
											  		  &tool);
	}
	
	// I deliberately don't test for kMoreSecFolderInappropriateErr here 
	// because I want to show the other usage pattern, which is to fail 
	// with an error.
	
	// Create the request dictionary.
	
	if (err == noErr) {	
		CFStringRef key;
		CFStringRef value;
		
		key   = kMoreSecurityTestCommandNameKey;
		value = kMoreSecurityTestLowNumberPortCommand;
		
		request = CFDictionaryCreate(NULL, (const void **) &key, (const void **) &value, 1, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
		err = CFQError(request);
	}

	// Go go gadget helper tool!

	if (err == noErr) {
		err = MoreSecExecuteRequestInHelperTool(tool, auth, request, &response);
	}
	
	// Extract information from the response.
	
	if (err == noErr) {
		// CFShow(response);

		err = MoreSecGetErrorFromResponse(response);
		if (err == noErr) {
			CFArrayRef 	descArray;
			CFIndex		descIndex;
			CFIndex		descCount;
			
			descArray = (CFArrayRef) CFDictionaryGetValue(response, kMoreSecFileDescriptorsKey);
			assert(descArray != NULL);
			assert( CFGetTypeID(descArray) == CFArrayGetTypeID() );
			
			descCount = CFArrayGetCount(descArray);
			for (descIndex = 0; descIndex < descCount; descIndex++) {
				CFNumberRef thisDescNum;
				int 				thisDesc;
				struct sockaddr_in 	addr;
				int 				addrLen;
				
				thisDescNum = (CFNumberRef) CFArrayGetValueAtIndex(descArray, descIndex);
				assert( (thisDescNum != NULL) && (CFGetTypeID(thisDescNum) == CFNumberGetTypeID()) );

				// Normally it's bad to include function calls that have side effects 
				// within an "assert", but in this case the assert is guaranteed 
				// to be in effect because we're inside a MORE_DEBUG block.
				
				assert( CFNumberGetValue(thisDescNum, kCFNumberIntType, &thisDesc) );
				assert(thisDesc >= 0);
				assert( fcntl(thisDesc, F_GETFD, 0) >= 0 );
				addrLen = sizeof(addr);
				assert( getsockname(thisDesc, (struct sockaddr *) &addr, &addrLen) == 0 );
				assert( addrLen == sizeof(addr) );
				assert( addr.sin_len == sizeof(addr) );
				assert( addr.sin_family == AF_INET );
				assert( (130 <= addr.sin_port) && (addr.sin_port <= 132) );
				assert( addr.sin_addr.s_addr == INADDR_ANY);
			}
		}
	}

	// Display to the user.
	
	if (err == noErr) {
		CopyCStringToPascal(msgStr, (StringPtr) msgStr);
		(void) StandardAlert(kAlertNoteAlert, "\pSuccess", "\p", NULL, &junkHit);
	} else {
		(void) sprintf(msgStr, "err = %ld", err);
		CopyCStringToPascal(msgStr, (StringPtr) msgStr);
		(void) StandardAlert(kAlertStopAlert, "\pFailure", (StringPtr) msgStr, NULL, &junkHit);
	}

	// Clean up.
	
	CFQRelease(tool);
	CFQRelease(request);
	if (response != NULL) {
		MoreSecCloseDescriptorArray((CFArrayRef) CFDictionaryGetValue(response, kMoreSecFileDescriptorsKey));
	}
	CFQRelease(response);
	if (auth != NULL) {	
		junk = AuthorizationFree(auth, kAuthorizationFlagDestroyRights);
		assert(junk == noErr);
	}
}

static EventHandlerUPP gApplicationEventHandlerUPP;		// -> ApplicationEventHandler

static const EventTypeSpec kApplicationEvents[] = { {kEventClassCommand, kEventCommandProcess} };

static pascal OSStatus ApplicationEventHandler(EventHandlerCallRef inHandlerCallRef, 
											   EventRef inEvent, void *inUserData)
	// Dispatches HICommands to their implementations.
{
	OSStatus 	err;
	HICommand 	command;
	#pragma unused(inHandlerCallRef)
	#pragma unused(inUserData)
	
	assert( GetEventClass(inEvent) == kEventClassCommand  );
	assert( GetEventKind(inEvent)  == kEventCommandProcess);
	
	err = GetEventParameter(inEvent, kEventParamDirectObject, typeHICommand, NULL, sizeof(command), NULL, &command);
	if (err == noErr) {
		switch (command.commandID) {
			case kHICommandAbout:
				DoAbout();
				break;
			case 'TUID':
				DoUIDTest();
				break;
			case 'TLNP':
				DoLowNumberedPortTest();
				break;
			default:
				err = eventNotHandledErr;
				break;
		}
	}
	
	return err;
}

static WindowRef gMainWindow;

static OSStatus SetupUserInterface(void)
	// Create a user interface from our NIB.
{
	OSStatus 	err;
	IBNibRef 	nibRef;
	Handle		menuBar;

	nibRef  = NULL;
	menuBar = NULL;
		
	err = CreateNibReference(CFSTR("MoreSecurityTest"), &nibRef);
	if (err == noErr) {
		err = CreateMenuBarFromNib(nibRef, CFSTR("MenuBar"), &menuBar);
	}
	if (err == noErr) {
		SetMenuBar(menuBar);
	}
	if (err == noErr) {
		err = CreateWindowFromNib(nibRef, CFSTR("MainWindow"), &gMainWindow);
	}
	if (err == noErr) {
		ShowWindow(gMainWindow);
	}
  	
	if (nibRef != NULL) {
		DisposeNibReference(nibRef);
	}
	if (menuBar != NULL) {
		DisposeHandle(menuBar);
	}
	return err;
}

int main(void)
{
	OSStatus err;

	// SIGPIPE bad.
	
	err = EXXXToOSStatus( MoreUNIXIgnoreSIGPIPE() );
	
	// Start up the UI.
	
	if (err == noErr) {
		err = SetupUserInterface();
	}
	
	// Install our HICommand handler.
	
	if (err == noErr) {
		gApplicationEventHandlerUPP = NewEventHandlerUPP(ApplicationEventHandler);
		assert(gApplicationEventHandlerUPP != NULL);

		err = InstallApplicationEventHandler(gApplicationEventHandlerUPP, 
											 GetEventTypeCount(kApplicationEvents), 
											 kApplicationEvents, NULL, NULL);
	}
	
	// Run the application.
	
	if (err == noErr) {
		RunApplicationEventLoop();
	}
	return 0;
}
