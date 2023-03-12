/*
	File:		MoreSCFCCLScanner.c

	Contains:	Code to generate list of modem script on Mac OS X.

	Written by:	DTS

	Copyright:	Copyright (c) 2007 by Apple Inc., All Rights Reserved.

	Disclaimer:	IMPORTANT:  This Apple software is supplied to you by Apple Inc.
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
				Apple Inc. may be used to endorse or promote products derived from the
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

$Log: MoreSCFCCLScanner.c,v $
Revision 1.10  2006/03/27 14:42:11         
Eliminate high-bit set characters.

Revision 1.9  2006/03/24 15:44:18         
Updated copyright.

Revision 1.8  2006/03/24 12:38:26         
Eliminate "pascal" keyword.

Revision 1.7  2006/03/24 11:29:50         
Eliminated "MoreSetup.h" to make it easier for folks to copy MIB source into their projects.

Revision 1.6  2002/12/12 15:24:47         
Work around a problem with FSFindFolder where it doesn't find a Modem Script folder in any domain if you run the application from a non-boot volume on traditional Mac OS.

Revision 1.5  2002/11/25 16:48:14         
Added MoreSCSetDefaultCCL. Also made changes to support building with CFM, even on Mac OS 9.  The algorithm to scan for CCLs is the same on 9 vs X, so we might as well use the same code.

Revision 1.4  2002/11/14 20:21:58         
Write my own code to test for file invisibility to eliminate the dependency on Launch Services, which brings in all of ApplicationServices.framework, which is inappropriate for the places that MoreSCF is used (like in setuid root helper tools).

Revision 1.3  2002/11/09 00:01:51         
Include our prototype early to flush out any missing dependencies. Convert nil to NULL. Convert MoreAssertQ to assert.

Revision 1.2  2002/01/22 06:15:53         
C++ compatibility.

Revision 1.1  2002/01/16 22:52:24         
First checked in.


*/

/////////////////////////////////////////////////////////////////

// Our prototypes

#include "MoreSCFCCLScanner.h"

// System Interfaces

#include <stdlib.h>
#include <SystemConfiguration/SystemConfiguration.h>

// MIB prototypes

#include "MoreCFQ.h"
#include "MoreSCF.h"
#include "MoreSCFPortScanner.h"

/////////////////////////////////////////////////////////////////

#if MAC_OS_X_VERSION_MIN_REQUIRED >= MAC_OS_X_VERSION_10_4 && ! MORE_SCF_NO_DEPRECATION_WARNINGS
    #warning MoreSCF is deprecated if you are building for 10.4 or later.
#endif

/////////////////////////////////////////////////////////////////

static CFStringRef gDefaultCCL;

extern void MoreSCSetDefaultCCL(CFStringRef cclName)
{
	CFQRelease(gDefaultCCL);
	gDefaultCCL = cclName;
	CFQRetain(gDefaultCCL);
}

// This variable is set by the test script to enable bug-for-bug compatibility 
// with 10.4.  This is necessary to pass certain tests, but it's not what I'd 
// recommend for general use.

Boolean gMoreSCFCCLScannerBugForBugCompatibility = false;

static CFStringRef MoreSCCopyDefaultCCL(void)
{
    // If the user hasn't supplied a default, first see if the internal 
    // modem support V.92.  If so, use the V.92 modem script as the default.
    //
    // Except, if bug-for-bug compatibility is enabled and we're running on 
    // 10.4 and later, don't try to do this.  Apparently 10.4 always sets 
    // the default modem script to V.90, even if the modem supports V.92.
    
    if ( (gDefaultCCL == NULL) && ! gMoreSCFCCLScannerBugForBugCompatibility && (MoreSCGetSystemVersion() < 0x01040) ) {
        OSStatus        err;
        CFArrayRef      ports;
        CFIndex         portCount;
        CFIndex         portIndex;
        
        ports = NULL;
        
        err = MoreSCCreatePortArray(&ports);
        if (err == noErr) {
            portCount = CFArrayGetCount(ports);
            
            for (portIndex = 0; portIndex < portCount; portIndex++) {
                CFDictionaryRef     thisPort;
                CFStringRef         thisPortHardware;
                CFStringRef         thisPortName;
                CFNumberRef         onHoldRef;
                int                 onHold;
                
                thisPort = (CFDictionaryRef) CFArrayGetValueAtIndex(ports, portIndex);
                assert(thisPort != NULL);
                assert( CFGetTypeID(thisPort) == CFDictionaryGetTypeID() );
                
                thisPortHardware = (CFStringRef) CFDictionaryGetValue(thisPort, kSCPropNetInterfaceHardware);
                assert(thisPortHardware != NULL);
                assert( CFGetTypeID(thisPortHardware) == CFStringGetTypeID() );

                if ( (thisPortHardware != NULL) && CFEqual(thisPortHardware, kSCEntNetModem) ) {
                    thisPortName = (CFStringRef) CFDictionaryGetValue(thisPort, kSCPropNetInterfaceDeviceName);
                    assert(thisPortName != NULL);
                    assert( CFGetTypeID(thisPortName) == CFStringGetTypeID() );
                    
                    if ( (thisPortName != NULL) && CFEqual(thisPortName, CFSTR("modem")) ) {
                        onHoldRef = (CFNumberRef) CFDictionaryGetValue(thisPort, kSCPropNetInterfaceSupportsModemOnHold);
                        assert(onHoldRef != NULL);
                        assert( CFGetTypeID(onHoldRef) == CFNumberGetTypeID() );
                        
                        if ( (onHoldRef != NULL) && CFNumberGetValue(onHoldRef, kCFNumberIntType, &onHold) && (onHold != 0) ) {
                            assert(onHold == 1);        // would be weird otherwise
                            
                            gDefaultCCL = CFSTR("Apple Internal 56K Modem (v.92)");
                            CFQRetain(gDefaultCCL);
                        }
                        
                        // Regardless of whether we set on hold or not, we stop 
                        // once we've found the built-in modem.
                        
                        break;
                    }
                }
            }
        }
        
        CFQRelease(ports);
        
        assert(err == noErr);           // just for debugging
    }
    
    // If we still don't have a default, go with the standard default.
    
    if (gDefaultCCL == NULL) {
        gDefaultCCL = CFSTR("Apple Internal 56K Modem (v.90)");
        CFQRetain(gDefaultCCL);
    }
    
    CFQRetain(gDefaultCCL);
    
    return gDefaultCCL;
}

static OSStatus MyIsVisibleFile(const FSRef *ref, Boolean *visible)
	// Determine whether a ref points to a visible file.
	//
	// I really want to call LSCopyItemInfoForRef and test the resulting 
	// flags for kLSItemInfoIsInvisible and kLSItemInfoIsPlainFile, but 
	// if I do that I have to take a dependency on Launch Services, which 
	// is part of ApplicationServices.framework [3101095].  That's not 
	// acceptable, because this code is commonly used in a setuid root helper 
	// tool and such a tool should depend on the minimal set of services, 
	// so I roll my own visibility test code.
{
	OSStatus		err;
	FSCatalogInfo 	info;
	HFSUniStr255  	name;
	
	assert(ref != NULL);
	assert(visible != NULL);
	
	err = FSGetCatalogInfo(ref, kFSCatInfoNodeFlags | kFSCatInfoFinderInfo, &info, &name, NULL, NULL);
	if (err == noErr) {
		*visible =     ((info.nodeFlags & kFSNodeIsDirectoryMask) == 0) 					// file
					&& ((((FInfo *) &info.finderInfo[0])->fdFlags & kIsInvisible) == 0)		// visible
					&& (name.unicode[0] != '.');											// doesn't begin with .
	}
	return err;
}

enum {
	kFolderItemsPerBulkCall = 50
};

static OSStatus AddCCLsInFolderToArray(const FSRef *folderRef, CFMutableArrayRef result)
	// Iterates through the contents of the folder specified by folderRef, 
	// adding the name of each CCL file (ie any file that's visible) to 
	// the result array.
{
	OSStatus			err;
	OSStatus			junk;
	FSRef *				scriptRefs;
	HFSUniStr255 *		scriptNames;
	FSIterator 			iter;
	ItemCount  			actCount;
	ItemCount			thisItemIndex;
	Boolean    			done;

	iter = NULL;
	scriptRefs = NULL;
	scriptNames = NULL;
	
	// Allocate buffers for the FSRefs and long Unicode names.  Given that 
	// we're processing 50 files at a time, these buffers are way too big 
	// to store on the stack (25 KB for scriptNames alone).
	
	scriptRefs  = (FSRef *) malloc( kFolderItemsPerBulkCall * sizeof(FSRef) );
	scriptNames = (HFSUniStr255 *) malloc( kFolderItemsPerBulkCall * sizeof(HFSUniStr255) );
	err = noErr;
	if (scriptRefs == NULL || scriptNames == NULL) {
		err = memFullErr;
	}
	
	// Create the iterator.
	
	if (err == noErr) {
		err = FSOpenIterator(folderRef, kFSIterateFlat, &iter);
	}
	
	// Iterate over the contents of the directory.  For each item found 
	// we check whether it's a visible plain file and, if it is, add its 
	// name to the array.
	
	if (err == noErr) {
		done = false;
		do {
			err = FSGetCatalogInfoBulk(iter, kFolderItemsPerBulkCall, &actCount, NULL, kFSCatInfoNone, NULL, 
									   scriptRefs, NULL, scriptNames);
			if (err == errFSNoMoreItems) {
				// We ran off the end of the directory.  Record that we're
				// done, but set err to noErr so that we process any partial
				// results.
				done = true;
				err = noErr;
			}
			if (err == noErr) {
				for (thisItemIndex = 0; thisItemIndex < actCount; thisItemIndex++) {
					Boolean visible;

					err = MyIsVisibleFile(&scriptRefs[thisItemIndex], &visible);
					if ( (err == noErr) && visible ) {
						CFStringRef thisItemName;
						
						thisItemName = CFStringCreateWithCharacters(NULL, scriptNames[thisItemIndex].unicode, scriptNames[thisItemIndex].length);
						if (thisItemName == NULL) {
							err = coreFoundationUnknownErr;
						}
						if (err == noErr) {
							CFArrayAppendValue(result, thisItemName);
						}
						CFQRelease(thisItemName);
					}
					if (err != noErr) {
						break;
					}
				}						
			}
		} while (err == noErr && !done);
	}

	// Clean up.
	
	if (iter != NULL) {
		junk = FSCloseIterator(iter);
		assert(junk == noErr);
	}
	if (scriptRefs != NULL) {
		free(scriptRefs);
	}
	if (scriptNames != NULL) {
		free(scriptNames);
	}

	return err;
}

enum {
	kMyStringCompareOptions = kCFCompareCaseInsensitive | kCFCompareLocalized | kCFCompareNumerically
};

extern OSStatus MoreSCCreateCCLArray(CFArrayRef *result, CFIndex *indexOfDefaultCCL)
	// See comment in header.
{
	OSStatus 			err;
	CFMutableArrayRef	localResult;
	UInt32   			domainIndex;
	FSRef				folderRef;
	static const SInt16 kFolderDomains[] = {kUserDomain, kNetworkDomain, kLocalDomain, kSystemDomain, 0};
	
	assert( result != NULL);
	assert(*result == NULL);
	
	// Create an array containing the names of the CCLs from the Modem Scripts 
	// folder in each domain.

	localResult = NULL;
		
	err = CFQArrayCreateMutable(&localResult);
	if (err == noErr) {
		UInt32 scannedFolders;
		
		scannedFolders = 0;
		domainIndex = 0;
		do {
			err = FSFindFolder(kFolderDomains[domainIndex], kModemScriptsFolderType, false, &folderRef);
			if (err != noErr) {
				// If we can't find the folder in this domain, just ignore the domain.
				err = noErr;
			} else {
				scannedFolders += 1;
				// If we found the folder, add each CCL in it to the array.
				err = AddCCLsInFolderToArray(&folderRef, localResult);
			}
			domainIndex += 1;
		} while (err == noErr && kFolderDomains[domainIndex] != 0);
		
		// Testing reveals that under certain circumstances, the above loop 
		// will never find any folders.  The specific case I saw was on 
		// Mac OS 9.1 with CarbonLib 1.6 when running the application off 
		// the non-boot volume.  So, if FSFindFolder never returns any 
		// folders in the domains we looped over above, let's do it the old 
		// way and call FSFindFolder with kOnSystemDisk.
		//
		// I didn't file a bug against FSFindFolder because traditional Mac OS 
		// bugs are no longer being fixed at this time.
		// -- Quinn, 10 Dec 2002
		
		if ( (err == noErr) && (scannedFolders == 0) ) {
			if (FSFindFolder(kOnSystemDisk, kModemScriptsFolderType, false, &folderRef) == noErr) {
				err = AddCCLsInFolderToArray(&folderRef, localResult);
			}
		}
	}
	
	// Sort the resulting array and delete any duplicates.
	
	if (err == noErr) {
		CFIndex cursor;
		
		CFArraySortValues(localResult, CFRangeMake(0, CFArrayGetCount(localResult)), 
						  (CFComparatorFunction) CFStringCompare, (void *) kMyStringCompareOptions);
		
		cursor = 1;
		while (cursor < CFArrayGetCount(localResult)) {
			if ( CFEqual(CFArrayGetValueAtIndex(localResult, cursor), CFArrayGetValueAtIndex(localResult, cursor - 1)) ) {
				CFArrayRemoveValueAtIndex(localResult, cursor);
			} else {
				cursor += 1;
			}
		}
	}
	
	// Find the index of the default CCL (or use the first CCL if the default 
	// isn't found).  The array is already sorted for user presentation, so 
	// we can use CFArrayBSearchValues rather than searching by hand.  This 
	// might even be a relevant speed increase because the number of CCLs can 
	// range into the hundreds.
	
	if (err == noErr && indexOfDefaultCCL != NULL) {
		CFStringRef defaultCCLName;
		CFIndex     itemIndex;
		
        defaultCCLName = MoreSCCopyDefaultCCL();

		itemIndex = CFArrayBSearchValues(localResult, CFRangeMake(0, CFArrayGetCount(localResult)), 
											defaultCCLName, 
											(CFComparatorFunction) CFStringCompare, (void *) kMyStringCompareOptions);
		// itemIndex is either:
		//
		// o pointing directly at the correct element, if an exact match is found, or 
		//
		// o if no exact match is found, pointing after the element that's immediately 
		//   less than defaultCCLName
		//
		// So, to confirm that we have the default script we check that the item is in 
		// bounds and that its value matches defaultCCLName.  If either of these 
		// checks fails, we return the index of the first CCL.
		
		if ( (itemIndex < CFArrayGetCount(localResult)) 
				&& (CFStringCompare(defaultCCLName, 
									(CFStringRef) CFArrayGetValueAtIndex(localResult, itemIndex), 
									kMyStringCompareOptions) == kCFCompareEqualTo)) {
			*indexOfDefaultCCL = itemIndex;
		} else {
			*indexOfDefaultCCL = 0;
		}
        
        CFQRelease(defaultCCLName);
	}

	// Clean up.
	
	if (err != noErr) {
		CFQRelease(localResult);
		localResult = NULL;
	}
	*result = localResult;
	
	assert( (err == noErr) == (*result != NULL) );
	assert( 	(err != noErr) 							// either we got an error
				 || (indexOfDefaultCCL == NULL) 				// or the user didn't ask for the default CCL
				 || (CFArrayGetCount(*result) == 0) 		// or there are no CCLs
				 || (*indexOfDefaultCCL >= 0 && *indexOfDefaultCCL < CFArrayGetCount(*result)) );
				 											// or the default CCL we're returning is in bounds

	return err;
}
