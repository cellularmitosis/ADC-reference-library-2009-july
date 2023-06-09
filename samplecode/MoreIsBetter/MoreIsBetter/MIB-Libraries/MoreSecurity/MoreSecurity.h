/*
	File:		MoreSecurity.h

	Contains:	Security utilities.

	Written by:	Quinn

	Copyright:	Copyright (c) 2003 by Apple Computer, Inc., All Rights Reserved.

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

$Log: MoreSecurity.h,v $
Revision 1.8  2003/08/14 21:45:59         
Improved the way temporary helper tools are handled.  I deleted the special routines to handle this case.  Now you just call the standard routines, passing them kTemporaryFolderType.

Revision 1.7  2003/08/08 15:37:36         
Added support for helper tools in the Temporary Items folder. Also, changed some definitions to use the word "ownership" instead of "privileges", because that's what the Finder checkbox actually controls.

Revision 1.6  2003/05/25 12:30:58         
Added support for descriptor passing.

Revision 1.5  2003/01/20 07:58:32         
Corrected some misspellings.

Revision 1.4  2002/12/12 17:47:57         
There should only be one point 5 in the "how to use" comment.

Revision 1.3  2002/12/12 15:40:19         
Eliminate MoreAuthCopyRightCFString because it makes no sense.  A helper tool should always have the right name hard-wired into it, and hardwiring a C string is even easier than hardwiring a CFString.

Revision 1.2  2002/11/25 16:42:29         
Significant changes. Handle more edge cases better (for example, volumes with the "ignore permissions" flag turned on). Also brought MoreSecurity more into the CoreServices world.

Revision 1.1  2002/11/09 00:08:40         
First checked in. A module containing security helpers.


*/

#pragma once

/////////////////////////////////////////////////////////////////

// MoreIsBetter Setup

#include "MoreSetup.h"

#if !TARGET_RT_MAC_MACHO
	#error MoreSecurity requires the use of Mach-O
#endif

// System prototypes

#include <Security/Security.h>
#include <CoreServices/CoreServices.h>

/////////////////////////////////////////////////////////////////

#ifdef __cplusplus
extern "C" {
#endif

/////////////////////////////////////////////////////////////////
#pragma mark ***** UID Management

// Basic utilities to manage your UIDs in setuid root programs.

// RUID = real user id      = ID of user who is running program
// EUID = effective user id = ID used to restrict privileged operations
// SUID = saved user id     = extra ID to allow setuid programs to switch in and out of privileged mode

extern int MoreSecPermanentlySetNonPrivilegedEUID(void);
    // Relinquish privileges by setting RUID, EUID, and SUID 
    // to the RUID.

extern int MoreSecTemporarilySetNonPrivilegedEUID(void);
    // Set the EUID to the RUID, which means that subsequent 
    // code runs with a non-zero EUID.  The process can 
    // still switch back to EUID when needed because the 
    // SUID is still 0.

extern int MoreSecSetPrivilegedEUID(void);
	// Set the EUID to 0.  This is only possible if either 
	// the EUID is already 0 or the SUID is 0.

// Typical Usage Patterns
// ----------------------
// #1 -- Do privileged operation and then drop privs
// 
//       DoPrivilegedStuff();
//       MoreSecPermanentlySetNonPrivilegedEUID();
//       DoNonPrivilegedStuff();
//
// #2 -- Switch in and out of privileged mode
//
//       MoreSecTemporarilySetNonPrivilegedEUID();
//       DoNonPrivilegedStuff();
//       MoreSecSetPrivilegedEUID();
//       DoPrivilegedStuff();
//       ... repeat as necessary ...
//       MoreSecPermanentlySetNonPrivilegedEUID();
//       DoNonPrivilegedStuff();
//

/////////////////////////////////////////////////////////////////
#pragma mark **** Environment Management

// This is a utility routine to destroy any environment that you might have 
// inherited from your parent.  It's most useful in a setuid root program, 
// which has to be careful that it doesn't rely on untrusted information 
// that it inherited.  Also useful for any privileged program which wants 
// to execute a non-privileged program and ensure that no privileged 
// information leaks to that program.

// The following is a set of bit masks used for the whatToDubya parameter 
// of the MoreSecDestroyInheritedEnvironment function.

enum {
    kMoreSecKeepNothingMask           = 0x0000,
    
    kMoreSecKeepArg0Mask              = 0x0001,
        // if set, don't reset argv[0] to the real 
        // executable path
    kMoreSecKeepEnvironmentMask       = 0x0002,
        // if set, don't clear environment
    kMoreSecKeepStandardFilesMask     = 0x0004,
        // if set, don't close stdin, stdout, stderr
    kMoreSecKeepOtherFilesMask        = 0x0008,
        // if set, don't close other file descriptors
    kMoreSecKeepSignalsMask           = 0x0010,
        // if set, don't reset signals to default
    kMoreSecKeepUmaskMask             = 0x0020,
        // if set, don't reset umask
    kMoreSecKeepNiceMask              = 0x0040,
        // if set, don't reset process nice values
    kMoreSecKeepResourceLimitsMask    = 0x0080,
        // if set, don't reset resource limits
    kMoreSecKeepCurrentDirMask        = 0x0100, 
        // if set, don't set CWD to "/"
    kMoreSecKeepTimersMask            = 0x0200, 
        // if set, don't reset all interval timers
        
    kMoreSecKeepEverythingMask        = 0x03FF
};

extern int MoreSecDestroyInheritedEnvironment(int whatToDubya, const char **argv);
    // This function eliminates most of the environmental 
    // factors that you have inherited from your parent 
    // process.  The function is intended for use by setuid 
    // root programs, who shouldn't trust any information 
    // supplied by a potentially malicious parent process.
    // 
    // The whatToDubya parameter is a bit mask that 
    // specifies what environmental factors to *keep*.  The 
    // most secure option is to pass kMoreSecKeepNothingMask 
    // (ie 0), which means you keep no environmental factors.
    //
    // The argv parameter allows the function to access argv 
    // without any special magic.  You can pass NULL if you 
    // whatToDubya has kMoreSecKeepArg0Mask set, which 
    // indicates that you don't want to reset argv[0].
    //
    // The function returns a errno-style error.  If it's 
    // non-zero you probably should err on the side of 
    // caution and refuse to start up.

/////////////////////////////////////////////////////////////////
#pragma mark **** Helper Tool Big Picture

// These utilities let you easily implement a setuid helper tool.
// This design is largely stolen from the DTS sample code "AuthSample", 
// with refinements to cover more cases and allow easy code reuse.
// 
// <http://developer.apple.com/samplecode/Sample_Code/Security/AuthSample.htm>
//
// To create a setuid root help tool, follow these simple instructions.
//
// 1. Define a request/response protocol based around CFDictionaries.
//    This is as simple as defining the set of keys in the request 
//    that describe the operation you want to do, and the set of 
//    keys in the response that hold the results of those operations 
//    (if any).
//
// 2. Create a helper tool whose "main" function looks like:
//
//  	int main(int argc, const char *argv[])
//  	{	
//  		int 				err;
//  		int 				result;
//  		AuthorizationRef 	auth;
//  		
//  		auth = MoreSecHelperToolCopyAuthRef();
//  	
//  		err = MoreSecDestroyInheritedEnvironment(kMoreSecKeepStandardFilesMask, argv);
//  		if (err == 0) {
//  			err = MoreUNIXIgnoreSIGPIPE();
//  		}
//  		if (err == 0) {
//  			err = MoreSecHelperToolMain(STDIN_FILENO, STDOUT_FILENO, auth, MyCommandProc, argc, argv);
//  		}
//  	
//  		return MoreSecErrorToHelperToolResult(err);
//  	}
// 
//    MyCommandProc is a callback that is executed when a request is 
//    passed to the tool.  It can do privileged operations, such as 
//    committing changes to System Configuration framework.  Its 
//    parameters are passed to it as a CFDictionary.  It can pass results 
//    back to the application by creating a response dictionary.
//
// 3. Put the tool in the "MacOS" folder inside your application package.
//    Decide on two tool names, one for the original template tool 
//    (for example, "HelperToolTemplate") and one for the working copy 
//    of the tool (for example, "HelperTool").
//
// 4. In your application, use MoreSecCopyHelperToolURLAndCheckBundled to find 
//    (or create) the working copy of your tool.  
// 
//		err = MoreSecCopyHelperToolURLAndCheckBundled(
//			CFBundleGetMainBundle(), 
//			CFSTR("HelperToolTemplate"), 
//			kApplicationSupportFolderType, 
//			CFSTR("My Application Name"), 
//			CFSTR("HelperTool"), 
//			&tool
//		);
//    
//    The working copy of the helper tool will exist within the user's 
//    Application Support folder.  See the description of 
//    MoreSecCopyHelperToolURLAndCheck[Bundled] for an explanation of this.
//
//    You should be prepared to get the error kMoreSecFolderInappropriateErr 
//    from the above routine.  This error, and how you can handle it, is 
//    discussed below.
// 
// 5. Then use MoreSecExecuteRequestInHelperTool to execute the tool, passing it 
//    a request dictionary.  On success, you'll get back a response dictionary.  Use 
//    MoreSecGetErrorFromResponse to extract the error result from your tool's 
//    MyCommandProc routine.
//
// 6. For extra credit, use AuthorizationCopyRights to have your tool 
//    be self-limiting, as described in the Authorization Services docs.
//
//    <http://developer.apple.com/techpubs/macosx/CoreTechnologies/securityservices/authorizationservices/authservices.html>
// 
//              *              *              *              *
//
// This complex design is required for a number of reasons.
// 
//  o The technique of using a self-limiting setuid root helper tool is 
//    recommended by Apple's security team.
// 
//  o You need to use a working helper tool and a backup helper tool 
//    because of the problems in the Mac OS X Finder.  The problems 
//    are as follows:
//
//    - In Mac OS X 10.0.x, the Finder will silently not copy the  
//      setuid root attribute of a helper program within your bundle.
//
//    - In Mac OS X 10.1.x, the Finder will not copy the setuid root 
//      helper program within your bundle (and display a wacky error 
//      dialog).
//
//    - In Mac OS X 10.2.x, the Finder will refuse to copy an application 
//      that contains a setuid root helper tool.
//
//  o It's a general requirement that a Mac OS X application be 
//    self-contained, that is, it should be drag installable and not 
//    require the user to run an installer.
//
//  o It's a general requirement that a Mac OS X application be runnable 
//    from a read-only volume (like a CD-ROM).  Thus, you can't place 
//    the helper tool within the application bundle because copying it 
//    there would require the volume be writable.
//
//  o Users can choose to ignore ownership on a particular volume 
//    by checking a checkbox in the Finder Get Info window.  A setuid 
//    program on an volume that's ignoring ownership will not be 
//    effective (ie it won't run as EUID 0).
//
//  o Users in a managed environment often have NFS home directories.
//    Most NFS installations do not allow you to create setuid root 
//    on the NFS server.
//
// By placing the setuid root helper tool within the Application Support 
// folder, this code allows you to copy the application bundle from 
// one disk to another without breaking it and without any weird error 
// dialogs.  In addition, you can copy the program to another machine 
// entirely, and the only thing the user has to do is to reenter their 
// admin password when they first run the application.  Finally, the 
// user's Application Support folder is typically inside their home 
// directory, which is typically on the system volume, and thus is 
// not ignoring ownership.
//
// Finally, MoreSecCopyHelperToolURLAndCheck[Bundled] might return the error 
// kMoreSecFolderInappropriateErr, which means that the user's 
// "Application Support" folder is on a volume that doesn't support 
// file ownership.  Typically this is either an HFS Plus volume where 
// the user has turned on "Ignore ownership of this volume" in the 
// Finder's Get Info window, or it's a file system type that doesn't 
// support setuid binaries (for example, the user has an NFS home 
// directory).  In this case, you have a number of choices.
//
// 1. You can display an error to the user telling them what's wrong 
//    and what to do about it.  There are two general types of solutions.
//
//    a) The user can reenable ownership on their volume.
// 
//    b) The user can ask a system administrator to install the setuid 
//       root helper tool in one of the standard places that 
//       MoreSecCopyHelperToolURLAndCheck[Bundled] looks for it (see 
//       the MoreSecCopyHelperToolURLAndCheck comment for details).
//
// 2. You can create a temporary copy of the helper tool by passing 
//    kTemporaryFolderType to the folder parameter of 
//    MoreSecCopyHelperToolURLAndCheck[Bundled].  The temporary items 
//    folder should always be appropriate.  If you do this, you may want 
//    to warn the user about this, because the tool will be periodically 
//    deleted.
//
// This technique may also be useful if the user's home directory is 
// read-only because you're booted from a CD.

/////////////////////////////////////////////////////////////////
#pragma mark **** Helper Tool Implementation

// The following are special error codes used by the helper tool code. 
// These error codes can be mapped to the helper tool return status 
// (ie the result from main).  For example, kMoreSecResultParamErr maps 
// to a return status of 1, kMoreSecResultPrivilegesErr maps to 2, 
// and so on.

enum {
	kMoreSecResultBase			   = 5360,

	kMoreSecResultParamErr         = 5361,
	kMoreSecResultPrivilegesErr    = 5362,
	kMoreSecResultCanceledErr      = 5363, 
	kMoreSecResultInternalErrorErr = 5364,
	
	kMoreSecFirstResultErr = kMoreSecResultParamErr,
	kMoreSecLastResultErr  = kMoreSecResultInternalErrorErr
	
	// other errors defined below
};

extern int MoreSecErrorToHelperToolResult(int errNum);
	// A function to map an errno/OSStatus to a helper tool return 
	// status.  The MoreSec error codes are mapped as described 
	// above, and there is a minimal amount of mapping of other 
	// OSStatus values to those codes.
	
extern int MoreSecHelperToolResultToError(int toolResult);
	// This function maps a helper tool status to the corresponding 
	// errno/OSStatus value.

extern AuthorizationRef MoreSecHelperToolCopyAuthRef(void);
	// When started by AuthorizationExecuteWithPrivileges, a helper tool 
	// gets some important parameters via its environment.  However, 
	// a helper tool is *supposed* to blow away its environment 
	// (using MoreSecDestroyInheritedEnvironment) to minimise security 
	// risks.  This routine allows the tool to recover the parameters 
	// supplied by AuthorizationExecuteWithPrivileges before it 
	// calls MoreSecDestroyInheritedEnvironment to destroy the environment.
	// 
	// IMPORTANT: It's quite normal for this routine to return NULL.
	// You should just pass the result straight to MoreSecHelperToolMain.

typedef OSStatus (*MoreSecCommandProc)(AuthorizationRef auth, 
									   CFDictionaryRef request, CFDictionaryRef *response);
	// This is a callback type, called by MoreSecHelperToolMain to 
	// actually execute commands.
	// 
	// auth will not be NULL
	// request will not be NULL
	// response will not be NULL
	// *response will be NULL
	//
	// The EUID will be set to the RUID, while the SUID will be 0.
	// If you need to do a privileged operation, you should 
	// temporarily set the EUID to 0 via MoreSecSetPrivilegedEUID,
	// and set it back whenh you're done via MoreSecTemporarilySetNonPrivilegedEUID.
	//
	// If you don't need to return anything other than a function 
	// result to your client, you don't need to mess with response.
	// MoreSecHelperToolMain will create a dictionary containing 
	// your function result and pass that back to the client.
	// Even if you do return a dictionary in *response, 
	// MoreSecHelperToolMain will still put your function result into the 
	// kMoreSecErrorNumberKey key unless you've set up that key yourself.
	//
	// If you want to self-limit the capabilities of your helper tool, 
	// you might consider using AuthorizationCopyRights to make sure 
	// that the user has authorization to do specific operations.
	// 
	// If you do create a response, it must be it must be flattenable via 
	// the CFPropertyList APIs.
	//
	// If the response contains a key kMoreSecFileDescriptorsKey, that key 
	// must be an array of valid file descriptors.  [See the declaration of 
	// kMoreSecFileDescriptorsKey for more information on the format.]  Those file 
	// descriptors are passed back to the calling process via descriptor passing on 
	// a UNIX domain socket.  This allows your helper tool to open privileged 
	// descriptors (for example, TCP sockets bound to a low-numbered port) and pass 
	// them back to the calling application.
	//
	// These descriptors are closed by the MoreSecurity infrastructure or 
	// by the calling process.  This is an unusual design style for me 
	// (I ususually prefer that the entity that creates a resource be 
	// responsible for destroying it), but it's unavoidable in this case. 
	// The descriptors are opened by customised code (that is, this callback 
	// but no other customised code is executed before the tool returns 
	// from main, so there's no opportunity for customised code to close the 
	// descriptors.  Hence MoreSecurity must do it for you.

extern int MoreSecHelperToolMain(int fdIn, int fdOut, AuthorizationRef auth, 
								 MoreSecCommandProc commandProc, int argc, const char *argv[]);
	// This function is the helper tool half of MoreSecExecuteRequestInHelperTool.
	// It's designed to be called directly from your helper tool's "main" function, 
	// and to implement the entire tool, calling commandProc to handle the specific 
	// command.
	//
	// IMPORTANT: Contrary to the usual rules, this routine disposes of 
	// auth before returning.  This makes sense when you consider the 
	// standard "main" function, described above.
	//
	// fdIn  must be non-negative, typically you pass in STDIN_FILENO
	// fdOut must be non-negative, typically you pass in STDOUT_FILENO
	// auth may be NULL, typically you just pass in the result of MoreSecHelperToolCopyAuthRef
	// commandProc must not be NULL
	// argc must be 1 or 2, depending on whether the tool is running in self-repair mode, 
	// typically you just pass in the argc that was supplied to main
	// argv must not be NULL, typically you just pass in the value that was supplied to main

/////////////////////////////////////////////////////////////////
#pragma mark **** Calling Helper Tool

enum {
	kMoreSecFolderInappropriateErr = 5370
};

extern OSStatus MoreSecIsFolderIgnoringOwnership(const FSRef *folder, Boolean *ignoringOwnership);
	// Determines whether the specified folder is on a volume that's 
	// ignoring ownership (as per the "Ignore ownership on this volume" 
	// checkbox in the Finder's Get Info dialog).  There is a real API to 
	// do this but it's currently private. I've filed a request to get 
	// this made public [3061192].  Until that's done, I use a workaround 
	// that involves creating a temporary file within the folder and 
	// seeing if I can manipulate its permissions.  For this workaround to 
	// work the folder must be read/write accessible.
	//
	// folder must not be NULL; folder must reference a folder
	// ignoringOwnership must not be NULL;
	// on success, *ignoringOwnership is true if the volume on which the folder 
	// resides is ignoring ownership; on error, *ignoringOwnership is undefined

extern OSStatus MoreSecIsFolderIgnoringSetUID(const FSRef *folder, Boolean *ignoringSetUID);
	// Determines whether the specified folder is on a volume that supports 
	// setuid.  Most NFS volumes are mounted so that they don't support 
	// setuid because doing so can lead to serious security problems.
	// 
	// folder must not be NULL; folder must reference a folder
	// ignoringSetUID must not be NULL;
	// on success, *ignoringSetUID is true if the volume on which the folder 
	// resides is ignoring set UID; on error, *ignoringSetUID is undefined

extern OSStatus MoreSecCopyHelperToolURLAndCheck(
	CFURLRef 		templateTool, 
	OSType 			folder, 
	CFStringRef 	subFolderName, 
	CFStringRef 	toolName, 
	CFURLRef *		tool
);
	// This routine returns a CFURL to your helper tool.  This CFURL typically 
	// points to a working copy of the tool inside the Application Support 
	// folder.  This routine confirms that the tool exists and is valid in that 
	// location.  If that check fails, it restores the tool from the template. 
	// Thus, the tool pointed to by the returned CFURL will be a valid copy of the 
	// template tool specified by templateTool.
	//
	// templateTool must not be NULL; the tool referenced by templateTool 
	// must exist; templateTool is a reference to your original helper tool; 
	// if your template tool is bundled within your application package, you might 
	// want to use the MoreSecCopyHelperToolURLAndCheckBundled wrapper routine 
	// (see below)
	//
	// folder is a Folder Manager folder selector; typically you pass kApplicationSupportFolderType
	//
	// subFolderName is the name of an optional sub-folder within the 
	// folder specified above; if this is not NULL, the returned URL 
	// points to within the named folder; if it is NULL, the URL points 
	// directly within the Folder Manager folder; if you don't want to 
	// pass NULL, you typically use the name of your application
	//
	// toolName is the name of the helper tool itself
	//
	// tool must not be NULL; *tool must be NULL; on success, *tool 
	// is a newly created CFURL that you must release; on error, *tool 
	// will be NULL
	//
	// On current systems, the returned URL will point to one of the 
	// following if the tool already exists.
	//
	//   ~/Library/Application Support/<subFolderName>/<toolName>
	//   /Library/Application Support/<subFolderName>/<toolName>
	//   /Network/Library/Application Support/<subFolderName>/<toolName>
	//   /System/Library/Application Support/<subFolderName>/<toolName>
	//   /private/tmp/<userID>/Temporary Items/<subFolderName>/<toolName>
	//
	// If the tool doesn't already exist, a copy will be made in:
	//
	//   ~/Library/Application Support/<subFolderName>/<toolName>
	//
	// and that URL will be returned.
	//
	// In all cases, you can treat <subFolderName> as the empty string 
	// if subFolderName is NULL.
	//
	// An error result of kMoreSecFolderInappropriateErr means that 
	// the helper tool could not be created because the above folder is 
	// on a volume that's ignoring ownership or setuid.  If you get this 
	// error, you might want to try creating the tool in the temporary 
	// items folder (kTemporaryFolderType) which should always be appropriate.
	// This routine doesn't automatically try the temporary items folder to 
	// give you added flexibility.  For example, you might want to display 
	// an alert to the user telling them that they're using a temporary tool, 
	// and what to do about it.  Alternatively, you might not want to use a 
	// temporary tool at all.
	//
	// IMPORTANT:
	// When create the helper tool from the template tool, this only copies the 
	// data fork of the tool.  If you the helper tool needs to access resources, 
	// you should either pass those resources in the request dictionary or 
	// pass it the URL of your bundle in the request dictionary.
	//
	// Note:
	// This does not verify any tool checksum or digital signature, 
	// for the reasons outlined in the comment "Notes on Code Signing"
	// (in "MoreSecurity.c").

extern OSStatus MoreSecCopyHelperToolURLAndCheckBundled(
	CFBundleRef 	inBundle, 
	CFStringRef 	templateToolName, 
	OSType 			folder, 
	CFStringRef 	subFolderName, 
	CFStringRef 	toolName, 
	CFURLRef *		tool
);
	// Finds (or creates) a helper tool in a bundled application.  
	// inBundle and templateToolName describe the location of the template 
	// tool.  folder, subFolderName, and toolName describe the location of the 
	// working copy of the tool. 
	//
	// This routine is a simple composition of CFBundleCopyAuxiliaryExecutableURL and 
	// MoreSecCopyHelperToolURLAndCheck.  See MoreSecCopyHelperToolURLAndCheck for 
	// a detailed discussion of the core functionality.
	//
	// inBundle must not be NULL; typically you just pass in CFBundleGetMainBundle()
	// 
	// templateToolName must not be NULL; the tool must exist within the executable 
	// folder of inBundle
	//
	// See MoreSecCopyHelperToolURLAndCheck for a full description of the folder, 
	// subFolderName, and toolName parameters.  Typically you pass 
	// kApplicationSupportFolderType for folder and your application name for 
	// subFolderName.
	//
	// tool must not be NULL; *tool must be NULL; on success, *tool will be 
	// a URL to the helper tool; on error, *tool will be NULL

extern OSStatus MoreSecExecuteRequestInHelperTool(CFURLRef helperTool, AuthorizationRef auth, 
											 	  CFDictionaryRef request, CFDictionaryRef *response);
	// Executes a privileged request in a helper tool.  helperTool is 
	// a reference to the helper tool to run.  You typically gets its value 
	// using MoreSecCopyHelperToolURLAndCheckBundled, although it's passed in as a 
	// parameter to allow flexibility.
	//
	// auth is your application's connection to Authorization Services. 
	// It's likely that you've used this connection to pre-authorization 
	// this operation, as described in the Authorization Services docs.
	//
	// <http://developer.apple.com/techpubs/macosx/CoreTechnologies/securityservices/authorizationservices/authservices.html>
	//
	// request specifies what you want the helper tool to do.  Its 
	// contents are not interpreted by MoreSecExecuteRequestInHelperTool, 
	// however, it must be flattenable via the CFPropertyList APIs.
	//
	// response is the reply from the helper tool.  It is not interpreted 
	// by MoreSecExecuteRequestInHelperTool other than to:
	//
	// o add a key (kMoreSecErrorNumberKey) which contains the error from the 
	//   helper tool's commandProc, and
	//
	// o transfer any file descriptors returned from the tool (in the 
	//   kMoreSecFileDescriptorsKey, see below for more details) to the 
	//   calling process.
	//
	// IMPORTANT:
	// You are responsible for closing any file descriptors returned by the 
	// helper tool.  The MoreSecCloseDescriptorArray helper routine may be 
	// useful for this.
	//
	// helperTool must not be NULL
	// auth must not be NULL
	// request must not be NULL
	// response must not be NULL; *response must be NULL; on success, 
	// *response will be a dictionary containing at least the 
	// kMoreSecErrorNumberKey key; on error, *response will be NULL

extern OSStatus MoreSecGetErrorFromResponse(CFDictionaryRef response);
	// This function returns the error stored in a helper tool 
	// response (in the kMoreSecErrorNumberKey key) obtained via 
	// MoreSecExecuteRequestInHelperTool.
	//
	// response must not be NULL

#define kMoreSecErrorNumberKey CFSTR("com.apple.dts.MoreIsBetter.MoreSec.ErrorNumber")		// CFNumberRef, OSStatus
	// The dictionary key where the helper tool error is stored.

#define kMoreSecFileDescriptorsKey CFSTR("com.apple.dts.MoreIsBetter.MoreSec.FDs")			// CFArray of CFNumber of kCFNumberIntType
	// If the helper tool returns file descriptors to your application, 
	// they will be stored in this key within the response.  The key value 
	// will be a CFArray.  Each element of the CFArray will be a CFNumber 
	// of format kCFNumberIntType.  Each number will be a valid file 
	// descriptor returned from the helper tool.
	// 
	// See the discussion of MoreSecCommandProc for more information about this.

extern void MoreSecCloseDescriptorArray(CFArrayRef descArray);
	// Given an array of descriptors (as described in the discussion of 
	// kMoreSecFileDescriptorsKey), this routine closes all of the 
	// descriptors.
	//
	// On input, descArray may be NULL; if so, this routine does nothing; 
	// if not, descArray must be a CFArray of CFNumbers, where each 
	// number is a descriptor to be closed

#ifdef __cplusplus
}
#endif
