/*
	File:		MoreSCFTest.c

	Contains:	Test program for all of MoreSCF.

	Written by:	Quinn

	Copyright:	Copyright (c) 2001 by Apple Computer, Inc., All Rights Reserved.

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

	Change History (most recent first):

$Log: MoreSCFTest.c,v $
Revision 1.6  2003/03/11 23:22:05         
Added DebugCreatePPPoE.

Revision 1.5  2003/02/26 21:02:49         
Serious rewrite. The tool now behaves differently depending on command line parameters. Also, worked around a bug in the Network prefs panel that was causing the set comparison to fail even though MoreSCF is doing the right thing.

Revision 1.4  2003/02/26 12:47:05         
Don't print high-bit set characters because they look ugly when running under Terminal. In the code to compare sets, changed lhs and rhs to new and def (default), so make it easier to understand which is which. Modified TestISP to test the PPPoE code. Added DebugCreateSet.

Revision 1.3  2002/11/09 00:02:39         
Convert nil to NULL. Convert MoreAssertQ to assert.

Revision 1.2  2002/01/22 06:24:13         
Adapt to the new port scanner API. Also added some extra printfs when we get a comparison failure in TestCreateSet to make it easier to track down problems.

Revision 1.1  2002/01/16 22:53:11         
First checked in.


*/

/////////////////////////////////////////////////////////////////

// MoreIsBetter Setup

#include "MoreSetup.h"

// System interfaces

#include <CoreFoundation/CoreFoundation.h>
#include <stdio.h>
#include <unistd.h>

// MIB Interfaces

#include "MoreCFQ.h"
#include "MoreSCFPortScanner.h"
#include "MoreSCFCCLScanner.h"
#include "MoreSCF.h"
#include "MoreSCFDigest.h"
#include "MoreSCFHelpers.h"

/////////////////////////////////////////////////////////////////
#pragma mark ***** Tests

static Boolean gNoRootTests = false;
	// If this is true, we only runs tests root tests if we're running as root.
	
static Boolean gRunQuiet = false;
	// When this is set to true the test routines print nothing 
	// during successful operation. This prevents a bunch of meaningless 
	// output during the memory leak tests.

static Boolean gCheckLeaks = false;
	// When this is set, tests are run repeatedly to check for leaks.

static const char *gProgramName;
	// The last path element of argv[0].  Used when printing error messages.
	
extern void PrintPropertyList(CFPropertyListRef propList);
	// Forward declaration.  Declared external for debugging purposes 
	// in other modules.

static void PrintTestResult(OSStatus errNum, const char *subTestName);
	// Forward declaration of this test engine function.
	
static void TestPortScanner(void)
	// A simple test of the port scanner code.  This doesn't do a 
	// lot of automated testing, but you can look at the results 
	// and visually check them.
{
	OSStatus   	err;
	CFArrayRef 	portArray;
	CFIndex 	portCount;
	CFIndex 	portIndex;
	long		order;
	CFNumberRef	supportsHold;
	
	portArray = NULL;
	
	err = MoreSCCreatePortArray(&portArray);
	if (err == noErr) {
		portCount = CFArrayGetCount(portArray);
		for (portIndex = 0; portIndex < portCount; portIndex++) {
			CFDictionaryRef thisPort;
			
			thisPort = (CFDictionaryRef) CFArrayGetValueAtIndex(portArray, portIndex);
			if (!gRunQuiet) {
				fprintf(stderr, "Port %ld\n", portIndex);
				fprintf(stderr, "  device   = ");
				CFShow(CFDictionaryGetValue(thisPort, kSCPropNetInterfaceDeviceName));
				fprintf(stderr, "  name     = ");
				CFShow(CFDictionaryGetValue(thisPort, kSCPropUserDefinedName));
				fprintf(stderr, "  hardware = ");
				CFShow(CFDictionaryGetValue(thisPort, kSCPropNetInterfaceHardware));
				fprintf(stderr, "  variant  = ");
				CFShow(CFDictionaryGetValue(thisPort, kMoreSCPropNetInterfaceHardwareVariant));
				fprintf(stderr, "  type     = ");
				CFShow(CFDictionaryGetValue(thisPort, kSCPropNetInterfaceType));
				fprintf(stderr, "  subtype  = ");
				CFShow(CFDictionaryGetValue(thisPort, kSCPropNetInterfaceSubType));
				fprintf(stderr, "  MAC      = ");
				CFShow(CFDictionaryGetValue(thisPort, kSCPropMACAddress));
				(void) CFNumberGetValue((CFNumberRef) CFDictionaryGetValue(thisPort, CFSTR("SortOrder")), kCFNumberLongType, &order);
				
				supportsHold = (CFNumberRef) CFDictionaryGetValue(thisPort, kSCPropNetInterfaceSupportsModemOnHold);
				if (supportsHold != NULL) {
					long hold;
					
					CFNumberGetValue(supportsHold, kCFNumberLongType, &hold);
					fprintf(stderr, "  hold     = %ld\n", hold);
				}

				fprintf(stderr, "  sort     = %ld\n", order);
			}
		}
	}
	
	CFQRelease(portArray);

	PrintTestResult(err, NULL);
}

static void TestCopySetsDict(void)
	// A test of the MoreSCCopySetsDict routine, which proved key 
	// in tracking down a reference count leak.
{
	OSStatus 		err;
	CFDictionaryRef setsDict;

	setsDict = NULL;
	err = MoreSCCopySetsDict(&setsDict);
	CFQRelease(setsDict);

	PrintTestResult(err, NULL);
}

static void TestSetEnumerationAndSwitch(void)
	// A test of set enumeration and switching routines.
{
	OSStatus	err;
	CFIndex  	setCount;
	CFIndex	  	setIndex;
	CFArrayRef	setIDs;
	CFIndex		indexOfCurrentSet;
	
	setIDs = NULL;
	
	// Enumeration
	
	err = MoreSCCopySetIDs(&setIDs, &indexOfCurrentSet);
	if (err == noErr) {
		setCount = CFArrayGetCount(setIDs);
		for (setIndex = 0; setIndex < setCount; setIndex++) {
			CFStringRef userVis;
			
			userVis = NULL;
			err = MoreSCCopyUserVisibleNameOfSet( (CFStringRef) CFArrayGetValueAtIndex(setIDs, setIndex), &userVis);
			if (err == noErr && !gRunQuiet) {
				fprintf(stderr, "#%ld %c ", setIndex, (setIndex == indexOfCurrentSet) ? '*' : ' ');
				CFShow(userVis);
				CFShow(CFArrayGetValueAtIndex(setIDs, setIndex));
			}
			CFQRelease(userVis);
			
			if (err != noErr) {
				break;
			}
		}
	}
	if (err == noErr) {
		CFStringRef currentSetID;
		
		currentSetID = NULL;
		err = MoreSCCopyCurrentSet(&currentSetID);
		if (err == noErr && !gRunQuiet) {
			fprintf(stderr, "Current set ID is ");
			CFShow(currentSetID);
		}
		
		CFQRelease(currentSetID);
	}
	
	// Switch
	
	if (err == noErr) {
		ItemCount newSet;
		
		if (indexOfCurrentSet == 0) {
			newSet = 1;
		} else {
			newSet = 0;
		}
		err = MoreSCSetCurrentSet( (CFStringRef) CFArrayGetValueAtIndex(setIDs, newSet));
		if (err == noErr) {
			err = MoreSCSetCurrentSet( (CFStringRef) CFArrayGetValueAtIndex(setIDs, indexOfCurrentSet));
		}
	}

	CFQRelease(setIDs);
	
	PrintTestResult(err, NULL);
}

static void TestDuplicateAndDeleteSet(void)
	// A test of the set duplication and deleting routines.
{
	OSStatus err;
	CFStringRef currentSetID;
	CFStringRef newSetID;
	
	currentSetID = NULL;
	newSetID = NULL;
	err = MoreSCCopyCurrentSet(&currentSetID);
	if (err == noErr) {
		err = MoreSCDuplicateSet(currentSetID, CFSTR("Frog"), &newSetID);
	}
	if (err == noErr) {
		if (!gRunQuiet) {
			fprintf(stderr, "New set ID is ");
			CFShow(newSetID);
		}
		
		err = MoreSCDeleteSet(newSetID);
	}
	
	CFQRelease(currentSetID);
	CFQRelease(newSetID);

	PrintTestResult(err, NULL);
}

static void TestRenameSet(void)
	// A test of the MoreSCRenameSet routine.
{
	OSStatus err;
	CFStringRef currentSetID;
	CFStringRef originalName;
	CFStringRef newName;
	
	currentSetID = NULL;
	originalName = NULL;
	newName = NULL;
	
	err = MoreSCCopyCurrentSet(&currentSetID);
	if (err == noErr) {
		err = MoreSCCopyUserVisibleNameOfSet(currentSetID, &originalName);
	}
	if (err == noErr) {
		err = MoreSCRenameSet(currentSetID, CFSTR("Frog"));
	}
	if (err == noErr) {
		err = MoreSCCopyUserVisibleNameOfSet(currentSetID, &newName);
	}
	if (err == noErr) {
		if ( ! CFEqual(newName, CFSTR("Frog")) ) {
			fprintf(stderr, "*** newName isn't 'Frog'\n");
			CFShow(newName);
		}
		err = MoreSCRenameSet(currentSetID, originalName);
	}
	if (err == noErr) {
		CFQRelease(newName);
		newName = NULL;
		
		err = MoreSCCopyUserVisibleNameOfSet(currentSetID, &newName);
	}
	if (err == noErr) {
		if ( ! CFEqual(newName, originalName) ) {
			fprintf(stderr, "*** newName isn't the same as originalName\n");
			CFShow(newName);
		}
	}
	
	CFQRelease(currentSetID);
	CFQRelease(originalName);
	CFQRelease(newName);

	PrintTestResult(err, NULL);
}

static void TestServiceEnumerate(void)
	// A test of the service enumeration routines.
{
	OSStatus  err;
	ItemCount serviceCount;
	ItemCount serviceIndex;
	CFArrayRef localServiceIDs;
	CFArrayRef resolvedServiceIDs;

	localServiceIDs = NULL;
	resolvedServiceIDs = NULL;
	
	err = MoreSCCopyServiceIDs(NULL, &localServiceIDs, &resolvedServiceIDs);
	if (err == noErr) {
		serviceCount = CFArrayGetCount(localServiceIDs);
		for (serviceIndex = 0; serviceIndex < serviceCount; serviceIndex++) {
			CFStringRef userVisible;
			Boolean     active;
			
			userVisible = NULL;
			err = MoreSCCopyUserVisibleNameOfService(NULL, (CFStringRef) CFArrayGetValueAtIndex(localServiceIDs, serviceIndex), &userVisible);
			if (err == noErr) {
				err = MoreSCIsServiceActive(NULL, (CFStringRef) CFArrayGetValueAtIndex(localServiceIDs, serviceIndex), &active);
			}
			if (err == noErr && !gRunQuiet) {
				fprintf(stderr, "#%ld %c ", serviceIndex, (active) ? ' ' : 'X');
				CFShow(userVisible);
				CFShow(CFArrayGetValueAtIndex(localServiceIDs, serviceIndex));
				CFShow(CFArrayGetValueAtIndex(resolvedServiceIDs, serviceIndex));
			}
			
			CFQRelease(userVisible);
			
			if (err != noErr) {
				break;
			}
		}
	}

	CFQRelease(localServiceIDs);
	CFQRelease(resolvedServiceIDs);

	PrintTestResult(err, NULL);
}

static void TestDuplicateAndDeleteService(void)
	// A test of the service duplication and deleting routines.
{
	OSStatus 	err;
	CFArrayRef  localServiceIDs;
	CFStringRef newServiceID;
	
	newServiceID = NULL;
	localServiceIDs = NULL;

	// Use NULL for the set ID to indicate that we're operating on
	// the current set.
	//
	// Can't use NULL for a service ID, so we have to come up with 
	// a valid service ID first.  We do this by choosing the first 
	// service ID.
	
	err = MoreSCCopyServiceIDs(NULL, &localServiceIDs, NULL);
	if (err == noErr) {
		assert( CFArrayGetCount(localServiceIDs) > 0 );
		err = MoreSCDuplicateService(NULL, (CFStringRef) CFArrayGetValueAtIndex(localServiceIDs, 0), CFSTR("Frog"), &newServiceID);
	}

	if (err == noErr) {
		if (!gRunQuiet) {
			fprintf(stderr, "New service ID is ");
			CFShow(newServiceID);
		}
		
		err = MoreSCDeleteService(NULL, newServiceID);
	}

	CFQRelease(localServiceIDs);
	CFQRelease(newServiceID);

	PrintTestResult(err, NULL);
}

static void TestRenameService(void)
	// A test of the MoreSCRenameService routine.
{
	OSStatus 	err;
	CFArrayRef  localServiceIDs;
	CFStringRef originalName;
	CFStringRef newName;
	CFStringRef serviceZero;
	
	localServiceIDs = NULL;
	originalName = NULL;
	newName = NULL;

	// Use NULL for the set ID to indicate that we're operating on
	// the current set.
	//
	// Can't use NULL for a service ID, so we have to come up with 
	// a valid service ID first.  We do this by choosing the first 
	// service ID.
	
	err = MoreSCCopyServiceIDs(NULL, &localServiceIDs, NULL);
	if (err == noErr) {
		assert( CFArrayGetCount(localServiceIDs) > 0 );
		serviceZero = (CFStringRef) CFArrayGetValueAtIndex(localServiceIDs, 0);
		err = MoreSCCopyUserVisibleNameOfService(NULL, serviceZero, &originalName);
	}
	if (err == noErr) {
		err = MoreSCRenameService(NULL, serviceZero, CFSTR("Frog"));
	}
	if (err == noErr) {
		err = MoreSCCopyUserVisibleNameOfService(NULL, serviceZero, &newName);
	}
	if (err == noErr) {
		if ( ! CFEqual(newName, CFSTR("Frog")) ) {
			fprintf(stderr, "*** newName isn't 'Frog'\n");
			CFShow(newName);
		}
		err = MoreSCRenameService(NULL, serviceZero, originalName);
	}
	if (err == noErr) {
		CFQRelease(newName);
		newName = NULL;
		
		err = MoreSCCopyUserVisibleNameOfService(NULL, serviceZero, &newName);
	}
	if (err == noErr) {
		if ( ! CFEqual(newName, originalName) ) {
			fprintf(stderr, "*** newName isn't the same as originalName\n");
			CFShow(newName);
		}
	}
	
	CFQRelease(localServiceIDs);
	CFQRelease(originalName);
	CFQRelease(newName);

	PrintTestResult(err, NULL);
}

static void TestEnumerateEntities(void)
	// A test of the MoreSCCopyEntities routine.
{
	OSStatus 	err;
	CFArrayRef  localServiceIDs;
	ItemCount	entityCount;
	ItemCount	entityIndex;
	CFArrayRef  protocols;
	CFArrayRef  values;
	
	// Use NULL for the set ID to indicate that we're operating on
	// the current set.
	//
	// Can't use NULL for a service ID, so we have to come up with 
	// a valid service ID first.  We do this by choosing the first 
	// service ID.
	
	localServiceIDs = NULL;
	protocols = NULL;
	values = NULL;
	
	err = MoreSCCopyServiceIDs(NULL, &localServiceIDs, NULL);
	if (err == noErr) {
		assert(CFArrayGetCount(localServiceIDs) > 0);
		err = MoreSCCopyEntities(NULL, (CFStringRef) CFArrayGetValueAtIndex(localServiceIDs, 0), &protocols, &values);
	}

	if (err == noErr && !gRunQuiet) {
		entityCount = CFArrayGetCount(protocols);
		for (entityIndex = 0; entityIndex < entityCount; entityIndex++) {
			fprintf(stderr, "#%ld ", entityIndex);
			CFShow(CFArrayGetValueAtIndex(protocols, entityIndex));
			CFShow(CFArrayGetValueAtIndex(values,    entityIndex));
		}
	}

	CFQRelease(localServiceIDs);
	CFQRelease(protocols);
	CFQRelease(values);
	
	PrintTestResult(err, NULL);
}

static OSStatus WorkaroundNetworkPrefsBug(CFDictionaryRef *entitiesDictPtr)
	// If this is an Ethernet interface and LCPEchoEnabled is false, 
	// set it to true.  This works around what I think is a bug in 
	// the Network preferences panel <rdar://problem/3182846> where the 
	// LCPEchoEnabled flag is mysteriously set to false for PCI Ethernet 
	// interfaces.
{
	OSStatus 	err;
	CFStringRef hardwarePath[2];
	CFStringRef lcpPath[2];
	CFStringRef	hardwareStr;
	CFNumberRef lcpValue;
	long		enabled;
	
	hardwarePath[0] = kSCEntNetInterface;
	hardwarePath[1] = kSCPropNetInterfaceHardware;

	lcpPath[0] = kSCEntNetPPP;
	lcpPath[1] = kSCPropNetPPPLCPEchoEnabled;
	
	hardwareStr = NULL;			// just to make debugging easier
	lcpValue = NULL;
	
	err = noErr;
	if (    CFQDictionaryGetValueAtPath(*entitiesDictPtr, (const void **) hardwarePath, 2, (const void **) &hardwareStr) == noErr
		 && CFEqual(hardwareStr, kSCEntNetEthernet)
		 && CFQDictionaryGetValueAtPath(*entitiesDictPtr, (const void **) lcpPath, 2, (const void **) &lcpValue) == noErr
		 && CFNumberGetValue(lcpValue, kCFNumberLongType, &enabled)
		 && (enabled == 0) ) {
		CFMutableDictionaryRef 	newDict;
		CFNumberRef 			numRef;
		
		if ( ! gRunQuiet ) {
			fprintf(stderr, "Applied workaround\n");
		}
		
		numRef = NULL;
		newDict = CFDictionaryCreateMutableCopy(NULL, 0, *entitiesDictPtr);
		err = CFQError(newDict);
		if (err == noErr) {
			enabled = true;
			numRef = CFNumberCreate(NULL, kCFNumberLongType, &enabled);
			err = CFQError(numRef);
		}
		if (err == noErr) {
			err = CFQDictionarySetValueAtPath(newDict, (const void **) lcpPath, 2, numRef);
		}
		if (err == noErr) {
			CFQRelease(*entitiesDictPtr);
			*entitiesDictPtr = newDict;
			newDict = NULL;
		}
		
		CFQRelease(newDict);
		CFQRelease(numRef);
	}

	return err;	
}

static OSStatus CompareServices(CFStringRef newSet, CFStringRef newService, 
								CFStringRef defSet, CFStringRef defService)
	// A service comparison routine used by the CompareSets routine.
	// This is godawful code but it's only test code and I don't have time to 
	// make it more elegant at this point.
{
	OSStatus    err;
	CFStringRef newUserVisible;
	CFStringRef defUserVisible;
	Boolean     newActive;
	Boolean     defActive;
	CFStringRef newBSD;
	CFStringRef defBSD;
	CFArrayRef  newProtocols;
	CFArrayRef  defProtocols;
	CFArrayRef  newValues;
	CFArrayRef  defValues;
	CFDictionaryRef newDict;
	CFDictionaryRef defDict;
	
	newUserVisible = NULL;
	defUserVisible = NULL;
	newBSD = NULL;
	defBSD = NULL;
	newProtocols = NULL;
	defProtocols = NULL;
	newValues = NULL;
	defValues = NULL;
	newDict = NULL;
	defDict = NULL;

	#if 0
		fprintf(stderr, "newSet, defSet, newService, defService = \n");
		CFShow(newSet);
		CFShow(defSet);
		CFShow(newService);
		CFShow(defService);
	#endif
	
	// User visible name
	
	err = MoreSCCopyUserVisibleNameOfService(newSet, newService, &newUserVisible);
	if (err == noErr) {
		err = MoreSCCopyUserVisibleNameOfService(defSet, defService, &defUserVisible);
	}
	if (err == noErr) {
		if ( ! CFEqual(newUserVisible, defUserVisible) ) {
			fprintf(stderr, "*** User visible names don't match\n");
			CFShow(newUserVisible);
			CFShow(defUserVisible);
			err = -1;
		}
	}
	
	// Active flag
	
	if (err == noErr) {
		err = MoreSCIsServiceActive(newSet, newService, &newActive);
	}
	if (err == noErr) {
		err = MoreSCIsServiceActive(defSet, defService, &defActive);
	}
	if (err == noErr) {
		if ( newActive != defActive ) {
			fprintf(stderr, "*** Active flags don't match\n");
			err = -1;
		}
	}

	// BSD name
	
	if (err == noErr) {
		err = MoreSCCopyBSDNameOfService(newSet, newService, &newBSD);
	}
	if (err == noErr) {
		err = MoreSCCopyBSDNameOfService(defSet, defService, &defBSD);
	}
	if (err == noErr) {
		if ( ! CFEqual(newBSD, defBSD) ) {
			fprintf(stderr, "*** BSD names don't match\n");
			CFShow(newBSD);
			CFShow(defBSD);
			err = -1;
		}
	}

	// Entities in service
	
	if (err == noErr) {
		err = MoreSCCopyEntities(newSet, newService, &newProtocols, &newValues);
	}
	if (err == noErr) {
		err = MoreSCCopyEntities(defSet, defService, &defProtocols, &defValues);
	}
	if (err == noErr) {
		err = CFQDictionaryCreateWithArrayOfKeysAndValues(newProtocols, newValues, &newDict);
	}
	if (err == noErr) {
		err = CFQDictionaryCreateWithArrayOfKeysAndValues(defProtocols, defValues, &defDict);
	}
	if (err == noErr) {
		err = WorkaroundNetworkPrefsBug(&defDict);
	}
	if (err == noErr) {
		if ( ! CFEqual(newDict, defDict) ) {
			fprintf(stderr, "*** Entities don't match\n");
			PrintPropertyList(newDict);
			PrintPropertyList(defDict);
			err = -1;
		}
	}

	CFQRelease(newUserVisible);
	CFQRelease(defUserVisible);
	CFQRelease(newBSD);
	CFQRelease(defBSD);
	CFQRelease(newProtocols);
	CFQRelease(defProtocols);
	CFQRelease(newValues);
	CFQRelease(defValues);
	CFQRelease(newDict);
	CFQRelease(defDict);
	
	return err;
}

static OSStatus CompareGlobalEntities(CFStringRef newSet, CFStringRef defSet)
	// A routine to compare global entities, taking care to ignore 
	// certain entities that aren't relevant.  This is used by the 
	// CompareSets routine.
	//
	// This is godawful code but it's only test code and I don't have time to 
	// make it more elegant at this point.
{
	OSStatus		err;
	CFArrayRef  	newProtocols;
	CFArrayRef  	defProtocols;
	CFArrayRef  	newValues;
	CFArrayRef  	defValues;
	CFDictionaryRef newDict;
	CFDictionaryRef defDict;
	CFMutableDictionaryRef newMutableDict;
	CFMutableDictionaryRef defMutableDict;
	CFStringRef     serviceOrderPath[2];

	newProtocols = NULL;
	defProtocols = NULL;
	newValues = NULL;
	defValues = NULL;
	newDict = NULL;
	defDict = NULL;
	newMutableDict = NULL;
	defMutableDict = NULL;

	// Create two mutable dictionaries, ones for the new entities and the 
	// other for the def entities.
	
	err = MoreSCCopyEntities(newSet, NULL, &newProtocols, &newValues);
	if (err == noErr) {
		err = MoreSCCopyEntities(defSet, NULL, &defProtocols, &defValues);
	}
	if (err == noErr) {
		err = CFQDictionaryCreateWithArrayOfKeysAndValues(newProtocols, newValues, &newDict);
	} 
	if (err == noErr) {
		err = CFQDictionaryCreateWithArrayOfKeysAndValues(defProtocols, defValues, &defDict);
	}
	if (err == noErr) {
		newMutableDict = CFDictionaryCreateMutableCopy(NULL, 0, newDict);
		if (newMutableDict == NULL) {
			err = coreFoundationUnknownErr;
		}
	}
	if (err == noErr) {
		defMutableDict = CFDictionaryCreateMutableCopy(NULL, 0, defDict);
		if (defMutableDict == NULL) {
			err = coreFoundationUnknownErr;
		}
	}

	// Remove the elementsl at "IPv4/ServiceOrder" because they will never 
	// compare equal because they contain different service IDs.

	serviceOrderPath[0] = kSCEntNetIPv4;
	serviceOrderPath[1] = kSCPropNetServiceOrder;
	if (err == noErr) {
		err = CFQDictionaryRemoveValueAtPath(newMutableDict, (const void **) serviceOrderPath, 2);
	}
	if (err == noErr) {
		err = CFQDictionaryRemoveValueAtPath(defMutableDict, (const void **) serviceOrderPath, 2);
	}
	
	// Now compare the dictionaries.
	
	if (err == noErr) {
		if ( ! CFEqual(newMutableDict, defMutableDict) ) {
			fprintf(stderr, "*** Global entities don't match\n");
			PrintPropertyList(newMutableDict);
			PrintPropertyList(defMutableDict);
			err = -1;
		}
	}
	
	CFQRelease(newProtocols);
	CFQRelease(defProtocols);
	CFQRelease(newValues);
	CFQRelease(defValues);
	CFQRelease(newDict);
	CFQRelease(defDict);
	CFQRelease(newMutableDict);
	CFQRelease(defMutableDict);
	
	return err;
}

static OSStatus CompareSets(CFStringRef newSet, CFStringRef defSet)
	// A routine to compare two sets to see if they're identical. 
	// TestCreateSet uses this to check that the newly created set 
	// was created correctly.
{
	OSStatus   err;
	CFArrayRef newServices;
	CFArrayRef defServices;
	CFIndex    serviceCount;
	CFIndex    serviceIndex;
	
	// MoreSCCopyUserVisibleNameOfSet not called because 
	// user visible name of sets will be different.
	
	newServices = NULL;
	defServices = NULL;
	
	// Services within the set
	
	err = MoreSCCopyServiceIDs(newSet, &newServices, NULL);
	if (err == noErr) {
		err = MoreSCCopyServiceIDs(defSet, &defServices, NULL);
	}
	if (err == noErr) {
		if ( CFArrayGetCount(newServices) != CFArrayGetCount(defServices) ) {
			fprintf(stderr, "*** Service counts not equal.\n");
			fprintf(stderr, "%ld\n", CFArrayGetCount(newServices));
			fprintf(stderr, "%ld\n", CFArrayGetCount(defServices));
			err = -1;
		}
	}
	if (err == noErr) {
		serviceCount = CFArrayGetCount(newServices);
		
		for (serviceIndex = 0; serviceIndex < serviceCount; serviceIndex++) {
			err = CompareServices(newSet, (CFStringRef) CFArrayGetValueAtIndex(newServices, serviceIndex),
								  defSet, (CFStringRef) CFArrayGetValueAtIndex(defServices, serviceIndex));
			if (err != noErr) {
				break;
			}
		}
	}
	
	// Global entities
	
	if (err == noErr) {
		err = CompareGlobalEntities(newSet, defSet);
	}
	
	CFQRelease(newServices);
	CFQRelease(defServices);

	return err;
}

#define kDefaultLocationName "DefaultLocationForMoreSCFTest"

static void TestCreateSet(void)
	// A routine to test the set creation and deletion code. 
	// It compares the newly created set against a set called 
	// "DefaultLocationForMoreSCFTest" to ensure that the set 
	// created by MoreSCF matches a set created by the control 
	// panel.
{
	OSStatus 		err;
	OSStatus 		err2;
	CFStringRef		newSetID;
	CFStringRef		templateSetID;
	
	newSetID = NULL;
	templateSetID = NULL;

	err = MoreSCFindSetByUserVisibleNameAndCopyID(CFSTR(kDefaultLocationName), &templateSetID);
	if (err != noErr || templateSetID == NULL) {
		fprintf(stderr, "*** TestCreateSet requires that you create a default set named '%s' using the Network preferences panel.\n", kDefaultLocationName);
		err = -1;
	}
	if (err == noErr) {
		err = MoreSCNewSet(CFSTR("Frog"), &newSetID);
	}
	if (err == noErr) {
		if (!gRunQuiet) {
			fprintf(stderr, "New set ID is ");
			CFShow(newSetID);
		}
		
		err = CompareSets(newSetID, templateSetID);

		err2 = MoreSCDeleteSet(newSetID);
		if (err == noErr) {
			err = err2;
		}
	}
	
	CFQRelease(newSetID);
	CFQRelease(templateSetID);

	PrintTestResult(err, NULL);
}

static void TestISP(void)
	// A routine to test the ISP support routines.
{
	OSStatus err;
	CFStringRef setID;
	CFStringRef setID2;
	CFStringRef newSetID;
	
	setID = NULL;
	setID2 = NULL;
	newSetID = NULL;
	
	err = MoreSCFindSetByUserVisibleNameAndCopyID(CFSTR(kDefaultLocationName), &setID);
	if (err == noErr) {
		if (setID == NULL) {
			fprintf(stderr, "*** Couldn't find the set '%s'\n", kDefaultLocationName);
		} else if (!gRunQuiet) {
			fprintf(stderr, "Set ID for '%s' is\n", kDefaultLocationName);
			CFShow(setID);
		}
	}
	if (err == noErr) {
		err = MoreSCFindSetByUserVisibleNameAndCopyID(CFSTR("Who would give a set such a silly name"), &setID2);
		if (setID2 != NULL) {
			fprintf(stderr, "*** Found set that shouldn't exist\n");
		}
	}
	if (err == noErr) {
		MoreSCPPPDigest ppp;
		
		BlockZero(&ppp, sizeof(ppp));
		ppp.active            = true;
		ppp.authName          = CFSTR("Quinn");
		ppp.authPassword      = CFSTR("eskimo");
		ppp.commRemoteAddress = CFSTR("123 4567");

		err = MoreSCMakeNewDialupSet(NULL, CFSTR("Frog PPP"), NULL, &ppp, NULL, NULL, &newSetID);
	}
	if (err == noErr) {
		err = MoreSCDeleteSet(newSetID);
	}
	
	CFQRelease(newSetID);
	newSetID = NULL;

	PrintTestResult(err, "dialup");

    err = noErr;
	
	if (err == noErr) {
		MoreSCPPPDigest ppp;
		
		BlockZero(&ppp, sizeof(ppp));
		ppp.active            = true;
		ppp.authName          = CFSTR("Quinn");
		ppp.authPassword      = CFSTR("eskimo");

		err = MoreSCMakeNewPPPoESet(NULL, CFSTR("Frog PPPoE"), &ppp, NULL, NULL, &newSetID);
	}
	if (err == noErr) {
		err = MoreSCDeleteSet(newSetID);
	}
	
	CFQRelease(setID);
	CFQRelease(setID2);
	CFQRelease(newSetID);

	PrintTestResult(err, "PPPoE");
}

static void TestAppleTalk(void)
	// A routine to test the AppleTalk on/off routines.
{
	OSStatus 	err;
	Boolean  	originalState;
	Boolean		newState;

	err = MoreSCIsAppleTalkActive(NULL, &originalState);
	if (err == noErr && !gRunQuiet) {
		fprintf(stderr, "AppleTalk is %s.\n", originalState ? "active" : "inactive");
	}
	if (err == noErr) {
		err = MoreSCSetAppleTalkActive(NULL, !originalState);
	}
	if (err == noErr) {
		err = MoreSCIsAppleTalkActive(NULL, &newState);
	}
	if (err == noErr) {
		if (!gRunQuiet) {
			fprintf(stderr, "AppleTalk is now %s.\n", newState ? "active" : "inactive");
		}
		if (newState != !originalState) {
			fprintf(stderr, "*** Failed to change state.\n");
		}
	}
	if (err == noErr) {
		err = MoreSCSetAppleTalkActive(NULL, originalState);
	}
	if (err == noErr) {
		err = MoreSCIsAppleTalkActive(NULL, &newState);
	}
	if (err == noErr) {
		if (!gRunQuiet) {
			fprintf(stderr, "AppleTalk is now %s.\n", newState ? "active" : "inactive");
		}
		if (newState != originalState) {
			fprintf(stderr, "*** Failed to restore state.\n");
		}
	}

	PrintTestResult(err, NULL);
}

/////////////////////////////////////////////////////////////////
#pragma mark ***** Test Engine

static pascal void PrintPropertyListCallback(CFTypeRef key, CFTypeRef node, void *context)
	// A callback routine used by PrintPropertyList to print 
	// a property list in a nicely formatted way.
{
	#pragma unused(key)
	int i;
	int depth;
	
	depth = (int)context;
	
	for (i = 0; i < depth; i++) {
		fprintf(stderr, "  ");
	}

	{
		CFStringRef fullDesc;
		CFStringRef typeDesc;
		CFStringRef valueDesc;

		fullDesc = NULL;		
		typeDesc = CFCopyTypeIDDescription(CFGetTypeID(node));
		valueDesc = NULL;
		if ( CFQPropertyListIsLeaf(node) ) {
			if ( CFGetTypeID(node) == CFStringGetTypeID() ) {
				valueDesc = (CFStringRef) CFRetain(node);
			} else if ( CFGetTypeID(node) == CFNumberGetTypeID() ) {
				valueDesc = (CFStringRef) CFRetain(node);
			} else {
				valueDesc = CFCopyDescription(node);
			}
			fullDesc = CFStringCreateWithFormat(NULL, NULL, CFSTR("%@ : %@ [%d] = %@"), key, typeDesc, CFGetRetainCount(node), valueDesc);
		} else {
			fullDesc = CFStringCreateWithFormat(NULL, NULL, CFSTR("%@ : %@ [%d]"), key, typeDesc, CFGetRetainCount(node));
		}
		CFShow(fullDesc);
		CFQRelease(fullDesc);
		CFQRelease(valueDesc);
		CFQRelease(typeDesc);
	}

	if ( ! CFQPropertyListIsLeaf(node) ) {
		CFQPropertyListShallowApplyFunction(node, PrintPropertyListCallback, (void *) (depth + 1) );
	}
}

extern void PrintPropertyList(CFPropertyListRef propList)
	// This routine prints a CFPropertyList in a nicely formatted way.
{
	PrintPropertyListCallback(CFSTR("ROOT"), propList, (void *) 0);
}

static pascal void RefCounter(CFTypeRef node, void *context)
	// A callback routine that adds node's reference count 
	// to a global total.  Used by TotalAllRefCounts.
{
	(*((SInt32 *)context)) += CFGetRetainCount(node);
}

static SInt32 TotalAllRefCounts(SCPreferencesRef prefsRef, CFArrayRef allKeys)
	// Given a connection to the SCF preferences database and an array 
	// of preference keys, this routine calculates the total of all 
	// of the reference counts of all of the nodes in all of the SCF 
	// preferences.  I use this routine to check for reference count leaks 
	// in my use of the SCF preferences database.
{
	CFIndex keyCount;
	CFIndex keyIndex;
	SInt32	result;
	
	result = 0;
	keyCount = CFArrayGetCount(allKeys);
	for (keyIndex = 0; keyIndex < keyCount; keyIndex++) {
		CFPropertyListRef thisPref;
		
		thisPref = SCPreferencesGetValue(prefsRef, (CFStringRef) CFArrayGetValueAtIndex(allKeys, keyIndex));	// C++ requires cast
		CFQPropertyListDeepApplyFunction(thisPref, RefCounter, (void *) &result );
	}
	return result;
}

#if 0

static void PrintAllPreferences(SCPreferencesRef prefsRef, CFArrayRef allKeys)
	// This routine prints all of the preferences in the SCF preferences 
	// database.  I use it during debugging.
{
	CFIndex keyCount;
	CFIndex keyIndex;
	
	keyCount = CFArrayGetCount(allKeys);
	for (keyIndex = 0; keyIndex < keyCount; keyIndex++) {
		CFPropertyListRef thisPref;
		
		thisPref = SCPreferencesGetValue(prefsRef, (CFStringRef) CFArrayGetValueAtIndex(allKeys, keyIndex));	// C++ requires cast
		PrintPropertyList(thisPref);
	}
}

#endif

typedef void (*TestFunc)(void);

static void LeakTest(TestFunc tester)
	// Given a test name and a pointer to a test function, 
	// this routine calls the test function repeatedly to check 
	// for SCF preferences reference count leaks.
{
	OSStatus 			err;
	SCPreferencesRef 	prefsRef;
	CFArrayRef 			allKeys;
	int					i;
	SInt32				startCount;
	SInt32				endCount;
	
	// This is kinda cheesy.  We need to use the same SCPreferencesRef 
	// that MoreSCF uses because otherwise we can't see the reference 
	// count changes done by MoreSCF.  Ideally, we wouldn't want to run 
	// within a MoreSCOpen/MoreSCClose pair because then our changes 
	// aren't necessarily being committed to the database.  However, 
	// given the current MoreSCF architecture, where the SCPreferenceRef 
	// is only valid inside the MoreSCOpen/MoreSCClose pair, that's 
	// exactly what we have to do.

	allKeys = NULL;
	
	err = MoreSCOpen(false, false);
	if (err == noErr) {
		prefsRef = MoreSCGetSCPreferencesRef();
	}

	// Now get a copy of the array of all keys in the database.  
	// We make this copy using CFPropertyListCreateDeepCopy so we 
	// know that it's completely independent of the SCF.
	
	if (err == noErr) {
		CFArrayRef allKeysOrig;
		
		allKeysOrig = NULL;

		allKeysOrig = SCPreferencesCopyKeyList(prefsRef);
		if (allKeysOrig == NULL) {
			err = SCError();
		}
		if (err == noErr) {
			allKeys = (CFArrayRef) CFPropertyListCreateDeepCopy(NULL, allKeysOrig, kCFPropertyListMutableContainersAndLeaves);	// C++ requires cast
			if (allKeys == NULL) {
				err = coreFoundationUnknownErr;
			}
		}
		
		CFQRelease(allKeysOrig);
	}

	// Now do the reference counting test.  Call the tester function
	// a few times to allow the refcounts to stabilise.  Then get 
	// a summ of all the nodes in all of the keys in SCF.  Then 
	// run the test a 10 more times and get another count.  If 
	// the counts are different, we're in trouble.
	
	if (err == noErr) {
		for (i = 0; i < 3; i++) {
			tester();
		}
		
		startCount = TotalAllRefCounts(prefsRef, allKeys);
		
		for (i = 0; i < 10; i++) {
			tester();
		}
		
		endCount = TotalAllRefCounts(prefsRef, allKeys);
		
		if (startCount != endCount) {
			CFIndex keyCount;
			CFIndex keyIndex;
			
			fprintf(stderr, "*** Leaked %ld reference counts.\n", endCount - startCount);

			keyCount = CFArrayGetCount(allKeys);
			for (keyIndex = 0; keyIndex < keyCount; keyIndex++) {
				CFPropertyListRef thisPref;

				// The commented out code is only needed when you're actively 
				// trying to track down a leak.  Given that leaks are rare 
				// I decided against making a proper architecture for this. 
				// Just uncomment the code and modify it appropriately.
				
				startCount = 0;
				thisPref = SCPreferencesGetValue(prefsRef, (CFStringRef) CFArrayGetValueAtIndex(allKeys, keyIndex));	// C++ requires cast
				CFQPropertyListDeepApplyFunction(thisPref, RefCounter, (void *) &startCount );
//				if (keyIndex == 0) {
//					fprintf(stderr, "*** BEFORE ***\n");
//					PrintPropertyList(thisPref);
//				}

				tester();

				endCount = 0;
				thisPref = SCPreferencesGetValue(prefsRef, (CFStringRef) CFArrayGetValueAtIndex(allKeys, keyIndex));	// C++ requires cast
				CFQPropertyListDeepApplyFunction(thisPref, RefCounter, (void *) &endCount );
//				if (keyIndex == 0) {
//					fprintf(stderr, "*** AFTER ***\n");
//					PrintPropertyList(thisPref);
//				}
				
				if (startCount != endCount) {
					fprintf(stderr, "*** Leaked %ld reference counts in set number %ld\n", endCount - startCount, keyIndex);
					CFShow(CFArrayGetValueAtIndex(allKeys, keyIndex));
				}
			}
		}
	}

	MoreSCClose(&err, false);

	CFQRelease(allKeys);

    if (err != noErr) {
        fprintf(stderr, "*** Failed with error %ld!\n", err);
    }
}

static void PrintUsage(void)
{
	fprintf(stderr, "Usage: %s [-q] [-l] [ TestName... ] \n", gProgramName);
	fprintf(stderr, "       -u only run root tests if running as root\n");
	fprintf(stderr, "       -q quiet mode\n");
	fprintf(stderr, "       -l run tests repeatedly to check for leaks (implies -q)\n");
}

static void PrintTestResult(OSStatus errNum, const char *subTestName)
{
	if (errNum == noErr) {
		if ( ! gRunQuiet ) {
			if (subTestName == NULL) {
				fprintf(stderr, "Success!\n");
			} else {
				fprintf(stderr, "Success! (%s)\n", subTestName);
			}
		}
	} else {
		if (subTestName == NULL) {
			fprintf(stderr, "*** Failed with error %ld\n", errNum);
		} else {
			fprintf(stderr, "*** Failed (%s) with error %ld\n", subTestName, errNum);
		}
	}
}

struct TestInfo {
	const char *	testName;
	TestFunc    	tester;
	Boolean			needsRoot;
};
typedef struct TestInfo TestInfo;

static void RunTest(const TestInfo *theTest)
{
	Boolean runTheTest;
	
	runTheTest = true;
	if ( gRunQuiet && gCheckLeaks ) {
		fprintf(stderr, "LeakTest(%s)\n", theTest->testName);
	} else if ( ! gRunQuiet ) {
		fprintf(stderr, "%s\n", theTest->testName);
	}
	if ( theTest->needsRoot ) {
		if (gNoRootTests) {
			fprintf(stderr, "*** Did not run test because it requires root.\n");
			runTheTest = false;
		} else if ( geteuid() != 0 ) {
    		fprintf(stderr, "*** Did not run because the test requires root.\n");
			runTheTest = false;
	    }
	}
	
	if (runTheTest) {
		if ( gCheckLeaks ) {
			LeakTest(theTest->tester);
		} else {
		    theTest->tester();
		}
	}
	if ( ! gRunQuiet ) {
		fprintf(stderr, "\n");
	}
}

static const TestInfo kTests[] = {
	{ "TestPortScanner", 				TestPortScanner,				false	}, 
	{ "TestCopySetsDict", 				TestCopySetsDict,				false	}, 
	{ "TestSetEnumerationAndSwitch", 	TestSetEnumerationAndSwitch,	true	}, 
	{ "TestDuplicateAndDeleteSet", 		TestDuplicateAndDeleteSet,		true	}, 
	{ "TestRenameSet", 					TestRenameSet,					true	}, 
	{ "TestServiceEnumerate", 			TestServiceEnumerate,			false	}, 
	{ "TestDuplicateAndDeleteService",	TestDuplicateAndDeleteService,	true	}, 
	{ "TestRenameService", 				TestRenameService,				true	}, 
	{ "TestEnumerateEntities", 			TestEnumerateEntities,			false	}, 
	{ "TestCreateSet", 					TestCreateSet,					true	}, 
	{ "TestISP", 						TestISP,						true	}, 
	{ "TestAppleTalk", 					TestAppleTalk,					true	}, 
	{ NULL, NULL }
};

static void RunTestsMatching(const char *testName)
{
	int testIndex;
	
	testIndex = 0;
	while ( kTests[testIndex].testName != NULL ) {
		if ( (testName == NULL) || (strcmp(testName, kTests[testIndex].testName) == 0) ) {
			RunTest(&kTests[testIndex]);
		}
		testIndex += 1;
	}
}

/////////////////////////////////////////////////////////////////
#pragma mark ***** Ad Hoc Debugging

// This code is for debugging individual modules and hasn't been integrated 
// into a formal test yet.

#if 0

	static void DebugModemScriptSearch(void)
		// Used for debugging the modem script code.
	{
		OSStatus 	err;
		CFArrayRef	cclArray;
		CFIndex		indexOfDefaultCCL;
		
		cclArray = NULL;
		err = MoreSCCreateCCLArray(&cclArray, &indexOfDefaultCCL);
		if (err == noErr) {
			CFIndex i;
			CFIndex c;
			
			c = CFArrayGetCount(cclArray);
			fprintf(stderr, "CCL Count = %ld\n", c);
			for (i = 0; i < c; i++) {
				fprintf(stderr, "%3ld %c ", i, i == indexOfDefaultCCL ? '*' : ' ');
				CFShow(CFArrayGetValueAtIndex(cclArray, i));
			}
		}
		
		CFQRelease(cclArray);
		
	    if (err == noErr) {
    	    fprintf(stderr, "Success!\n");
	    } else {
    	    fprintf(stderr, "*** Failed with error %ld!\n", err);
	    }
	}

#endif

#if 0

	static void DebugCreateProxiesEntity(void)
		// Used for debugging MoreSCCreateProxiesEntity.
	{
		OSStatus 			err;
		MoreSCProxiesDigest proxy;
		CFDictionaryRef     entity;
		
		BlockZero(&proxy, sizeof(proxy));
		entity = NULL;
		err = MoreSCCreateProxiesEntity(&proxy, &entity);
		if (err == noErr) {
			PrintPropertyList(entity);
		}
		CFQRelease(entity);
	}

#endif

#if 0

	static void DebugOpenClose(void)
		// Used for debugging MoreSCOpen/Close.
	{
		OSStatus 	err;
		int_t 		oldUID;
		
		// Simple open/close.

		err = MoreSCOpen(false, false);
		MoreSCClose(&err, false);
	    if (err != noErr) {
	        fprintf(stderr, "*** DebugOpenClose (step 1) failed with error %ld!\n", err);
	    }
	    
	    // Open and close dirty.
	    
	    err = MoreSCOpen(false, false);
	    MoreSCClose(&err, true);
	    if (err != noErr) {
	        fprintf(stderr, "*** DebugOpenClose (step 2) failed with error %ld!\n", err);
	    }
		
	    // Open locked and close not dirty.
	    
	    err = MoreSCOpen(true, true);
	    MoreSCClose(&err, false);
	    if (err != noErr) {
	        fprintf(stderr, "*** DebugOpenClose (step 3) failed with error %ld!\n", err);
	    }
		
	    // Open locked and close dirty.
	    
	    err = MoreSCOpen(true, true);
	    MoreSCClose(&err, true);
	    if (err != noErr) {
	        fprintf(stderr, "*** DebugOpenClose (step 4) failed with error %ld!\n", err);
	    }
		
		// Simple recursive open with no privileges needed.
		
		err = MoreSCOpen(false, false);
		if (err == noErr) {
			err = MoreSCOpen(false, false);
			
			MoreSCClose(&err, false);
		}
		MoreSCClose(&err, false);
	    if (err != noErr) {
	        fprintf(stderr, "*** DebugOpenClose (step 5) failed with error %ld!\n", err);
	    }
		
		// Recursive open with no lock but dirty made.
		
		err = MoreSCOpen(false, false);
		if (err == noErr) {
			err = MoreSCOpen(false, false);
			
			MoreSCClose(&err, false);
		}
		MoreSCClose(&err, true);
	    if (err != noErr) {
	        fprintf(stderr, "*** DebugOpenClose (step 6) failed with error %ld!\n", err);
	    }
		
		// Recursive open with lock and no dirty.
		
		err = MoreSCOpen(true, true);
		if (err == noErr) {
			err = MoreSCOpen(false, false);
			
			MoreSCClose(&err, false);
		}
		MoreSCClose(&err, false);
	    if (err != noErr) {
	        fprintf(stderr, "*** DebugOpenClose (step 7) failed with error %ld!\n", err);
	    }

		// Recursive open with lock and dirty.
		
		err = MoreSCOpen(true, true);
		if (err == noErr) {
			err = MoreSCOpen(false, false);
			
			MoreSCClose(&err, false);
		}
		MoreSCClose(&err, true);
	    if (err != noErr) {
	        fprintf(stderr, "*** DebugOpenClose (step 8) failed with error %ld!\n", err);
	    }
		
		// Recursive open with inner no lock and dirty.
		
		err = MoreSCOpen(false, false);
		if (err == noErr) {
			err = MoreSCOpen(false, false);
			
			MoreSCClose(&err, true);
		}
		MoreSCClose(&err, false);
	    if (err != noErr) {
	        fprintf(stderr, "*** DebugOpenClose (step 9) failed with error %ld!\n", err);
	    }
		
		// Recursive open with inner lock and no dirty.
		
		err = MoreSCOpen(false, false);
		if (err == noErr) {
			err = MoreSCOpen(true, true);
			
			MoreSCClose(&err, false);
		}
		MoreSCClose(&err, false);
	    if (err != noErr) {
	        fprintf(stderr, "*** DebugOpenClose (step 10) failed with error %ld!\n", err);
	    }

		// Recursive open with inner lock and dirty.
		
		err = MoreSCOpen(false, false);
		if (err == noErr) {
			err = MoreSCOpen(true, true);
			
			MoreSCClose(&err, true);
		}
		MoreSCClose(&err, false);
	    if (err != noErr) {
	        fprintf(stderr, "*** DebugOpenClose (step 11) failed with error %ld!\n", err);
	    }
		
		// Recursive open with both inner and outer lock and dirty.
		
		err = MoreSCOpen(true, true);
		if (err == noErr) {
			err = MoreSCOpen(true, true);
			
			MoreSCClose(&err, true);
		}
		MoreSCClose(&err, true);
	    if (err != noErr) {
	        fprintf(stderr, "*** DebugOpenClose (step 12) failed with error %ld!\n", err);
	    }

		// Open/close with error.
		
		err = MoreSCOpen(false, false);
		if (err == noErr) {
			err = -1;
		}
		MoreSCClose(&err, false);
	    if (err != -1) {
	        fprintf(stderr, "*** DebugOpenClose (step 13) failed with error %ld!\n", err);
	    }

		// Open/close with dirty and error.
		
		err = MoreSCOpen(false, false);
		if (err == noErr) {
			err = -1;
		}
		MoreSCClose(&err, true);
	    if (err != -1) {
	        fprintf(stderr, "*** DebugOpenClose (step 14) failed with error %ld!\n", err);
	    }
		
		// Open/close with lock, dirty and error.
		
		err = MoreSCOpen(true, true);
		if (err == noErr) {
			err = -1;
		}
		MoreSCClose(&err, true);
	    if (err != -1) {
	        fprintf(stderr, "*** DebugOpenClose (step 15) failed with error %ld!\n", err);
	    }
		
		// Switch to the nobody user.
		
		oldUID = seteuid(-2);

		// Lock but no dirty, should trigger error on the open.
		
		err = MoreSCOpen(true, true);
		assert(err == kSCStatusAccessError);
		MoreSCClose(&err, false);
	    if (err != kSCStatusAccessError) {
	        fprintf(stderr, "*** DebugOpenClose (step 16) failed with error %ld!\n", err);
	    }

		// No lock but dirty, should trigger error on the close.
		
		err = MoreSCOpen(false, false);
		assert(err == noErr);
		MoreSCClose(&err, true);
	    if (err != kSCStatusAccessError) {
	        fprintf(stderr, "*** DebugOpenClose (step 16) failed with error %ld!\n", err);
	    }

		
		(void) seteuid(oldUID);
	}

#endif

#if 0

	static void DebugCreateSet(void)
	{
		OSStatus err;
		
		err = MoreSCNewSet(CFSTR("Frog"), NULL);

	    if (err == noErr) {
    	    fprintf(stderr, "Success!\n");
	    } else {
    	    fprintf(stderr, "*** Failed with error %ld!\n", err);
	    }
	}

#endif

#if 0
    
    #define kPPPoESetName CFSTR("MoreSCF DebugCreatePPPoE Set Name")
    
    static void DebugCreatePPPoE(void)
    {
        OSStatus        err;
        CFStringRef     setID;
        
        setID = NULL;
    
        err = MoreSCFindSetByUserVisibleNameAndCopyID(kPPPoESetName, &setID);
        if (err == noErr && setID != NULL) {
            err = MoreSCDeleteSet(setID);
            
            CFQRelease(setID);
            setID = NULL;
        }
        if (err == noErr) {
            MoreSCPPPDigest ppp;
            
            BlockZero(&ppp, sizeof(ppp));
            ppp.active            = true;
            ppp.authName          = CFSTR("Quinn");
            ppp.authPassword      = CFSTR("eskimo");
    
            err = MoreSCMakeNewPPPoESet(NULL, kPPPoESetName, &ppp, NULL, NULL, &setID);
        }
        if (err == noErr) {
            fprintf(stderr, "PPPoE set ID is ");
            CFShow(setID);
        }
        
        CFQRelease(setID);
    
        if (err == noErr) {
            fprintf(stderr, "Success!\n");
        } else {
            fprintf(stderr, "*** Failed with error %ld\n", err);
        }
    }

#endif

int main(int argc, char *argv[])
{
	int ch;
	int argIndex;
	
	// Set up gProgramName to be the last path element of argv[0].
	
	gProgramName = strrchr(argv[0], '/');
	if (gProgramName == NULL) {
		gProgramName = argv[0];
	} else {
		gProgramName += 1;
	}
	
    fprintf(stderr, "%s\n", gProgramName);
    
	// Parse arguments.

    do {
        ch = getopt(argc, argv, "uql");
        if (ch != -1) {
            switch (ch) {
                case 'u':
                	gNoRootTests = true;
                    break;
                case 'q':
                	gRunQuiet = true;
                    break;
				case 'l':
					gCheckLeaks = true;
                	gRunQuiet   = true;
					break;
                case '?':
                default:
                    PrintUsage();
                    exit(1);
                    break;
            }
        }
    } while (ch != -1);
	
	#if 0
		DebugCreatePPPoE();
		DebugModemScriptSearch();
		DebugCreateProxiesEntity();
		DebugOpenClose();
		DebugCreateSet();
	#endif
	
    if (optind == argc) {
    	fprintf(stderr, "Running all tests\n");
    	fprintf(stderr, "\n");
    	RunTestsMatching(NULL);
    } else {
    	for (argIndex = optind; argIndex < argc; argIndex++) {
    		RunTestsMatching(argv[argIndex]);
    	}
    }

    return 0;
}
