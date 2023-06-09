/*
	File:		CallMachOFramework.c

	Contains:	Sample showing how to call Mach-O frameworks from CFM.

	Written by:	Quinn

	Copyright:	Copyright (c) 2000-2001 by Apple Computer, Inc., All Rights Reserved.

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

$Log: CallMachOFramework.c,v $
Revision 1.3  2002/11/08 23:09:41         
Convert nil to NULL. Convert MoreAssertQ to assert.

Revision 1.2  2001/11/07 15:49:28         
Tidy up headers, add CVS logs, update copyright.


         <1>     21/9/01    Quinn   First checked in.
*/

/////////////////////////////////////////////////////////////////

#include "MoreSetup.h"

// MoreIsBetter components

#include "CFMLateImport.h"

// Mac OS interfaces

#if ! MORE_FRAMEWORK_INCLUDES
	#include <MacTypes.h>
	#include <CFBundle.h>
	#include <Gestalt.h>
	#include <Folders.h>
#endif

// Standard C interfaces

#include <stdio.h>

/////////////////////////////////////////////////////////////////

// Start of stuff stolen from "unistd.h".

int      gethostname(char *, int);

// End of stuff stolen from "unistd.h".

/////////////////////////////////////////////////////////////////

#if !defined(USE_CFM_LATE_IMPORT)
	#error You must define USE_CFM_LATE_IMPORT one way or the other.
#endif

#if USE_CFM_LATE_IMPORT

	// The CFMLateImport approach requires that you define a fragment 
	// initialisation routine that latches the fragment's connection 
	// ID and locator.  If your code already has a fragment initialiser 
	// you will have to integrate the following into it.

	static CFragConnectionID 			gFragToFixConnID;
	static FSSpec 						gFragToFixFile;
	static CFragSystem7DiskFlatLocator 	gFragToFixLocator;

	extern OSErr FragmentInit(const CFragInitBlock *initBlock);
	extern OSErr FragmentInit(const CFragInitBlock *initBlock)
	{
		assert(initBlock->fragLocator.where == kDataForkCFragLocator);
		gFragToFixConnID	= (CFragConnectionID) initBlock->closureID;
		gFragToFixFile 		= *(initBlock->fragLocator.u.onDisk.fileSpec);
		gFragToFixLocator 	= initBlock->fragLocator.u.onDisk;
		gFragToFixLocator.fileSpec = &gFragToFixFile;
		
		return noErr;
	}

#else

	// The function pointer approach requires you to define a function 
	// pointer for every function you import from a framework.  This 
	// is really easy in this sample (we only call a single function) 
	// but it quickly gets tired for a big framework.

	typedef int (*gethostnamePtr)(char *, int);

#endif

static Boolean RunningOnCarbonX(void)
	// This code taken from DTS Q&A OV03 "Detecting Classic and Carbon X Environment".
	// <http://developer.apple.com/qa/ov/ov03.html>
{
    UInt32 response;
    
    return (Gestalt(gestaltSystemVersion, 
                    (SInt32 *) &response) == noErr)
                && (response >= 0x01000);
}

static OSStatus LoadFrameworkBundle(CFStringRef framework, CFBundleRef *bundlePtr)
	// This routine finds a the named framework and creates a CFBundle 
	// object for it.  It looks for the framework in the frameworks folder, 
	// as defined by the Folder Manager.  Currently this is 
	// "/System/Library/Frameworks", but we recommend that you avoid hard coded 
	// paths to ensure future compatibility.
	//
	// You might think that you could use CFBundleGetBundleWithIdentifier but 
	// that only finds bundles that are already loaded into your context. 
	// That would work in the case of the System framework but it wouldn't 
	// work if you're using some other, less-obvious, framework.
{
	OSStatus 	err;
	FSRef 		frameworksFolderRef;
	CFURLRef	baseURL;
	CFURLRef	bundleURL;
	
	assert(bundlePtr != NULL);
	
	*bundlePtr = NULL;
	
	baseURL = NULL;
	bundleURL = NULL;
	
	// Find the frameworks folder and create a URL for it.
	
	err = FSFindFolder(kOnAppropriateDisk, kFrameworksFolderType, true, &frameworksFolderRef);
	if (err == noErr) {
		baseURL = CFURLCreateFromFSRef(kCFAllocatorSystemDefault, &frameworksFolderRef);
		if (baseURL == NULL) {
			err = coreFoundationUnknownErr;
		}
	}
	
	// Append the name of the framework to the URL.
	
	if (err == noErr) {
		bundleURL = CFURLCreateCopyAppendingPathComponent(kCFAllocatorSystemDefault, baseURL, framework, false);
		if (bundleURL == NULL) {
			err = coreFoundationUnknownErr;
		}
	}
	
	// Create a bundle based on that URL and load the bundle into memory.
	// We never unload the bundle, which is reasonable in this case because 
	// the sample assumes that you'll be calling functions from this 
	// framework throughout the life of your application.
	
	if (err == noErr) {
		*bundlePtr = CFBundleCreate(kCFAllocatorSystemDefault, bundleURL);
		if (*bundlePtr == NULL) {
			err = coreFoundationUnknownErr;
		}
	}
	if (err == noErr) {
	    if ( ! CFBundleLoadExecutable( *bundlePtr ) ) {
			err = coreFoundationUnknownErr;
	    }
	}

	// Clean up.
	
	if (err != noErr && *bundlePtr != NULL) {
		CFRelease(*bundlePtr);
		*bundlePtr = NULL;
	}
	if (bundleURL != NULL) {
		CFRelease(bundleURL);
	}	
	if (baseURL != NULL) {
		CFRelease(baseURL);
	}	
	
	return err;
}

void main(void)
{
	OSStatus 			err;
	CFBundleRef 		sysBundle;
	char				hostname[256];
	
	printf("Hello Cruel World!\n");
	
	err = noErr;
	if ( ! RunningOnCarbonX() ) {
		printf("It does not make sense to run this sample on traditional Mac OS.\n");
		err = -1;
	}
	
	// Start by creating a CFBundle for the framework of interest.
	//
	// IMPORTANT:
	// You should name the correct framework!  In this example I'm 
	// using "System.framework", which is the framework that holds all 
	// the standard BSD functions.  Howver, if you wanted to load something 
	// from a different framework (for example, IOKit), you would have to 
	// use a different value (IOKit.framework).
	//
	// It's easy to work out the correct framework:
	// 
	// 1. Use the name of the umbrella framework (ie the highest level 
	//    framework) that contains the header file which declares the 
	//    functions you need.
	//
	// 2. If the header file is outside of a framework (for example, 
	//    in /usr/include), the function probably resides in the 
	//    System framework. Use the "nm" command line tool to confirm this.
	
	if (err == noErr) {
		err = LoadFrameworkBundle(CFSTR("System.framework"), &sysBundle);
	}
	
	// Now we get the host name using either the CFM late import or the 
	// function pointer approach.
	
	if (err == noErr) {
		#if USE_CFM_LATE_IMPORT
			printf("Using CFMLateImport approach.\n");
			assert( (void *) gethostname == (void *) kUnresolvedCFragSymbolAddress );
	
			err = CFMLateImportBundle(&gFragToFixLocator, gFragToFixConnID, FragmentInit, "\pSystemFrameworkLib", sysBundle);
			if (err == noErr) {
				err = gethostname(hostname, sizeof(hostname));
			}
		#else
			{
				gethostnamePtr ghnPtr;

				printf("Using function pointer approach.\n");
				
				ghnPtr = (gethostnamePtr) CFBundleGetFunctionPointerForName( sysBundle, CFSTR("gethostname") );
				if (ghnPtr == NULL) {
					err = cfragNoSymbolErr;
				}
			
				if (err == noErr) {
					err = ghnPtr(hostname, sizeof(hostname));
				}
			}
		#endif
	}

	// Print the host name, just to make sure that things are working.
	
	if (err == noErr) {	
		printf("hostname = %s\n", hostname);
	}

	if (err == noErr) {
		printf("Success.\n");
	} else {
		printf("Failed with error %d.\n", err);
	}
	
	printf("Done.  Press command-Q to Quit.\n");
}
