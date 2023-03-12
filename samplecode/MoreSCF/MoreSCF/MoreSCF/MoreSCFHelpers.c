/*
	File:		MoreSCFHelpers.c

	Contains:	High-level System Configuration framework operations.

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

$Log: MoreSCFHelpers.c,v $
Revision 1.11  2006/03/27 14:42:28         
Eliminate high-bit set characters.

Revision 1.10  2006/03/27 11:19:18         
Quieten "may be used without being initialised" warnings.

Revision 1.9  2006/03/24 15:44:34         
Updated copyright.

Revision 1.8  2006/03/24 12:38:42         
Eliminate "pascal" keyword.

Revision 1.7  2006/03/24 11:30:08         
Eliminated "MoreSetup.h" to make it easier for folks to copy MIB source into their projects.

Revision 1.6  2003/04/14 15:51:37         
Use CFQAllocate/Deallocate to prevent "malloc(0) returns NULL" problems.

Revision 1.5  2003/02/26 12:44:47         
Reworked the code used to implement MoreSCMakeNewDialupSet to support new features (MoreSCMakeNewPPPoESet) and the newly exported routine (MoreSCMakeNewSingleServiceSet).

Revision 1.4  2002/11/25 16:54:58         
Correct error in MoreSCMakeNewDialupSet that was found by a developer tripping over an assert.

Revision 1.3  2002/11/09 00:02:01         
Include our prototype early to flush out any missing dependencies. Convert nil to NULL. Convert MoreAssertQ to assert.

Revision 1.2  2002/01/22 06:22:50         
Adapt to the new port scanner API.

Revision 1.1  2002/01/16 22:52:31         
First checked in.


*/

/////////////////////////////////////////////////////////////////

// Our prototypes

#include "MoreSCFHelpers.h"

// System interfaces

#include <SystemConfiguration/SystemConfiguration.h>

// MIB Interfaces

#include "MoreCFQ.h"
#include "MoreSCF.h"
#include "MoreSCFPortScanner.h"

/////////////////////////////////////////////////////////////////

#if MAC_OS_X_VERSION_MIN_REQUIRED >= MAC_OS_X_VERSION_10_4 && ! MORE_SCF_NO_DEPRECATION_WARNINGS
    #warning MoreSCF is deprecated if you are building for 10.4 or later.
#endif

/////////////////////////////////////////////////////////////////
#pragma mark ***** AppleTalk On/Off

extern OSStatus MoreSCIsAppleTalkActive(CFStringRef setID, Boolean *active)
	// See comment in header.
{
	OSStatus 	err;
	CFArrayRef	serviceIDs;
	
	assert(active != NULL);
	
	*active = false;
	serviceIDs = NULL;
	
	// Open a connection to SCF, get a list of all of the set's 
	// services, iterate through those services to see whether any 
	// of their AppleTalk entities are active.
	
	err = MoreSCOpen(false, false);
	if (err == noErr) {
		err = MoreSCCopyServiceIDs(setID, &serviceIDs, NULL);
	}
	if (err == noErr) {
		CFIndex serviceCount;
		CFIndex serviceIndex;
		
		serviceCount = CFArrayGetCount(serviceIDs);
		serviceIndex = 0;
		while ( (err == noErr) && (serviceIndex < serviceCount) && !(*active) ) {
			CFStringRef thisService;
			Boolean		isActive;
			
			thisService = (CFStringRef) CFArrayGetValueAtIndex(serviceIDs, serviceIndex);
			err = MoreSCIsServiceActive(setID, thisService, &isActive);
			if (err == noErr && isActive) {

                // There may be no AppleTalk entity in this service (for example, 
                // a PPP service), so we don't consider a failure of MoreSCIsEntityActive 
                // an error.
                
				if ( (MoreSCIsEntityActive(setID, thisService, kSCEntNetAppleTalk, &isActive) == noErr) && isActive) {
					*active = true;
				}			
			}
			serviceIndex += 1;
		}
	}
	MoreSCClose(&err, false);
	
	CFQRelease(serviceIDs);
	
	return err;
}

extern OSStatus MoreSCSetAppleTalkActive(CFStringRef setID, Boolean active)
	// See comment in header.
{
	OSStatus 	err;
	CFArrayRef	serviceIDs;
	Boolean		madeChanges;
	
	serviceIDs = NULL;

	// Open a connection to SCF, get a list of all of the set's 
	// services, iterate through those services to make their 
	// AppleTalk entities active.
		
	madeChanges = false;
	err = MoreSCOpen(false, false);
	if (err == noErr) {
		err = MoreSCCopyServiceIDs(setID, &serviceIDs, NULL);
	}
	if (err == noErr) {
		CFIndex serviceCount;
		CFIndex serviceIndex;
		
		serviceCount = CFArrayGetCount(serviceIDs);
		for (serviceIndex = 0; serviceIndex < serviceCount; serviceIndex++) {
			CFStringRef 	thisService;
			Boolean			entityIsActive;

			thisService = (CFStringRef) CFArrayGetValueAtIndex(serviceIDs, serviceIndex);
			
			// There may be no AppleTalk entity in this service (for example, 
			// a PPP service), so we don't consider a failure of MoreSCIsEntityActive 
            // an error.
			
			if ( MoreSCIsEntityActive(setID, thisService, kSCEntNetAppleTalk, &entityIsActive) == noErr ) {
				if (entityIsActive != active) {
					err = MoreSCSetEntityActive(setID, thisService, kSCEntNetAppleTalk, active);
					if (err == noErr) {
						madeChanges = true;
					}
				}
			}
			
			if (err != noErr) {
				break;
			}
		}
	}
	MoreSCClose(&err, madeChanges);
	
	CFQRelease(serviceIDs);
	
	return err;
}

/////////////////////////////////////////////////////////////////
#pragma mark ***** DHCP Release

static OSStatus IsSetupConfiguredToUseDHCP(SCDynamicStoreRef storeRef, CFStringRef configuredIPv4Key, Boolean *isDHCP)
	// Given a path to an IPv4 entity within the Setup: domain of the 
	// dynamic store, sets *isDHCP to indicate whether the entity is 
	// configured to use DHCP address acquisition.
	//
	// The input string is something like "Setup:/Network/Service/<serviceID>/IPv4".
{
	OSStatus err;
	CFDictionaryRef value;
	CFStringRef configMethod;
	
	assert(storeRef != NULL);
	assert(configuredIPv4Key != NULL);
	assert(isDHCP != NULL);
	
	value = NULL;
	
	// Get the value of the IPv4 entity, extract the "ConfigMethod" 
	// property, and compare it to "DHCP".
	
	value = (CFDictionaryRef) SCDynamicStoreCopyValue(storeRef, configuredIPv4Key);
	err = MoreSCError( (void *) value );
	if (err == noErr) {
		assert( CFGetTypeID(value) == CFDictionaryGetTypeID() );
		if ( ! CFDictionaryGetValueIfPresent(value, kSCPropNetIPv4ConfigMethod, (const void **) &configMethod) ) {
			err = -2;
		}
	}
	if (err == noErr) {
		assert( CFGetTypeID(configMethod) == CFStringGetTypeID() );
		*isDHCP = CFEqual(configMethod, kSCValNetIPv4ConfigMethodDHCP);
	}

	// Clean up.
	
	CFQRelease(value);

	return err;
}

static OSStatus AppendServiceIDForEachDHCPDynamicStoreKey(SCDynamicStoreRef storeRef,
														  CFArrayRef configuredIPv4DynamicKeys, 
														  CFMutableArrayRef configuredDHCPServiceIDs)
	// Given an array of keys for configured IPv4 entities in the dynamic 
	// store, and a created mutable array of strings, this routine appends a 
	// service ID string to the output array for each element in 
	// the input array whoses ConfigMethod property is DHCP.
	//
	// Input array elements look like "Setup:/Network/Service/<serviceID>/IPv4".
	// Output array elements look like: "<serviceID>".
{
	OSStatus 	err;
	CFIndex		ipv4KeyCount;
	CFIndex 	keyIndex;

	assert(configuredIPv4DynamicKeys != NULL);
	assert(configuredDHCPServiceIDs  != NULL);
	
	// Iterate over each element in the input array.
	
	ipv4KeyCount = CFArrayGetCount(configuredIPv4DynamicKeys);

	err = noErr;
	for (keyIndex = 0; keyIndex < ipv4KeyCount; keyIndex++) {
		CFStringRef thisKey;			// got with "Get", so doesn't need to be released
		CFArrayRef	thisKeyExploded;
		Boolean		isDHCP;
		
        isDHCP = false;                     // quieten warning
        
		thisKeyExploded = NULL;
		
		// Get the keyIndex'th array element and assert that it looks like a 
		// valid dynamic store key.
		
		thisKey = (CFStringRef) CFArrayGetValueAtIndex(configuredIPv4DynamicKeys, keyIndex);
		if (thisKey == NULL) {
			err = coreFoundationUnknownErr;
		} else {
			assert( CFGetTypeID(thisKey) == CFStringGetTypeID() );
		}
		#if !defined(NDEBUG)
			if (err == noErr) {
				assert( CFStringHasPrefix(thisKey, CFSTR("Setup:/Network/Service/")) );
				assert( CFStringHasSuffix(thisKey, CFSTR("/IPv4")) );
			}
		#endif
		
		// Test to see whether we're interested in this element.

		if (err == noErr) {
			err = IsSetupConfiguredToUseDHCP(storeRef, thisKey, &isDHCP);
		}
		
		if (err == noErr && isDHCP) {
			// If we are, extract the service ID from the key and add it to 
			// the output array.
			
			// Explode the key into it's component pieces (separated by "/").  This is, IMHO, 
			// the easiest way to pull out the service ID part of the key (the "<serviceID>" in the 
			// examples at the beginning of this routine).
			
			if (err == noErr) {
				thisKeyExploded =  CFStringCreateArrayBySeparatingStrings(NULL, thisKey, CFSTR("/"));
				if (thisKeyExploded == NULL) {
					err = coreFoundationUnknownErr;
				}
			}

			// Add the service ID (the 3rd element of the exploded path) to the end of the results array.
			
			if (err == noErr) {
				assert( CFArrayGetCount(thisKeyExploded) == 5 );
				assert( CFGetTypeID(CFArrayGetValueAtIndex(thisKeyExploded, 3)) == CFStringGetTypeID() );

				CFArrayAppendValue(configuredDHCPServiceIDs, CFArrayGetValueAtIndex(thisKeyExploded, 3));
			}
		}
		
		// Clean up once each time around the loop.
		
		CFQRelease(thisKeyExploded);

		if (err != noErr) {
			break;
		}
	} // for
	
	return err;
}

extern OSStatus MoreSCCreateActiveDHCPServicesArray(CFArrayRef *activeDHCP)
	// See comment in header.
{
	OSStatus 			err;
	CFStringRef 		dhcpPattern;
	CFArrayRef 			configuredIPv4DynamicKeys;
	CFMutableArrayRef 	result;
	SCDynamicStoreRef   storeRef;
		
	assert( activeDHCP != NULL);
	assert(*activeDHCP == NULL);
	
	dhcpPattern = NULL;
	configuredIPv4DynamicKeys = NULL;
	result = NULL;

	// Open a connection to the dynamic store.
	
    storeRef = SCDynamicStoreCreate(NULL, MoreSCGetClient(), NULL, NULL);
    err = MoreSCError( (void *) storeRef);
	if (err == noErr) {
		// Create dhcpPattern as "Setup:/Network/Service/[^/]+/IPv4".  
		// SCDynamicStoreKeyCreateNetworkServiceEntity provides both 
		// kSCCompNetwork ("Network") and kSCCompService ("Service").
		//
		// We work in the Setup: domain not the state domain because 
		// that's where we find the ConfigMethod = DHCP property.  We 
		// can't just search for DHCP entities (which is what I originally 
		// implemented) because, if the interface is configured to use DHCP 
		// but didn't find a server, it's assigned an autonet address 
		// (169.x.x.x) but no DHCP entity is created.
		
		dhcpPattern = SCDynamicStoreKeyCreateNetworkServiceEntity(
							NULL, 							// allocator
							kSCDynamicStoreDomainSetup,		// domain    = "Setup:"
							kSCCompAnyRegex,				// serviceID = "[^/]+" (1 or more non-slash chars)
							kSCEntNetIPv4);					// entity    = "IPv4"
		err = MoreSCError( (void *) dhcpPattern );
	}
	
	// Find all the configured IPv4 entities in the dynamic store.
	
	if (err == noErr) {
		configuredIPv4DynamicKeys = SCDynamicStoreCopyKeyList(storeRef, dhcpPattern);
		err = MoreSCError( (void *) configuredIPv4DynamicKeys );
	}
	
	// Create an array for the output results.
	
	if (err == noErr) {
		result = CFArrayCreateMutable(NULL, CFArrayGetCount(configuredIPv4DynamicKeys), &kCFTypeArrayCallBacks);
		if (result == NULL) {
			err = coreFoundationUnknownErr;
		}
	}
	
	// Walk through the list of IPv4 Setup entities looking for those with the 
	// ConfigMethod = DHCP property.  For each one you find, append the service ID 
	// to the result list.
	
	if (err == noErr) {
		err = AppendServiceIDForEachDHCPDynamicStoreKey(storeRef, configuredIPv4DynamicKeys, result);
	}
	
	// Clean up.
	
	CFQRelease(dhcpPattern);
	CFQRelease(configuredIPv4DynamicKeys);
	if (err != noErr) {
		CFQRelease(result);
		result = NULL;
	}
	*activeDHCP = result;
	CFQRelease(storeRef);
	
	assert( (err == noErr) == (*activeDHCP != NULL) );
	
	return err;
}

extern OSStatus MoreSCDHCPRelease(CFStringRef serviceID)
	// See comment in header.
{
	OSStatus 				err;

	// Mark the entity inactive.
		
	err = MoreSCOpen(false, false);	
	if (err == noErr) {
		err = MoreSCSetEntityActive(NULL, serviceID, kSCEntNetIPv4, false);
	}
	MoreSCClose(&err, true);

	// Mark the entity active.
	
	if (err == noErr) {	
		err = MoreSCOpen(false, false);
		if (err == noErr) {
			err = MoreSCSetEntityActive(NULL, serviceID, kSCEntNetIPv4, true);
	    }
	    MoreSCClose(&err, true);
	}

	return err;
}

/////////////////////////////////////////////////////////////////
#pragma mark ***** ISP APIs

extern OSStatus MoreSCFindSetByUserVisibleNameAndCopyID(CFStringRef userVisibleName, CFStringRef *setID)
	// See comment in header.
{
	OSStatus 	err;
	CFArrayRef 	setIDs;
	
	assert(userVisibleName != NULL);
	assert( setID != NULL);
	assert(*setID == NULL);
	
	setIDs = NULL;

	// Get an array of the set IDs, iterate over that array looking for 
	// an element whose user-visible name matches.
	
	err = MoreSCCopySetIDs(&setIDs, NULL);
	if (err == noErr) {
		CFIndex setCount;
		CFIndex setIndex;
		
		setCount = CFArrayGetCount(setIDs);
		setIndex = 0;
		while ( (err == noErr) && (setIndex < setCount) && (*setID == NULL) ) {
			CFStringRef thisName;
			CFStringRef thisSet;
			
			thisSet = (CFStringRef) CFArrayGetValueAtIndex(setIDs, setIndex);
			
			thisName = NULL;
			err = MoreSCCopyUserVisibleNameOfSet(thisSet, &thisName);
			if (err == noErr) {
				if ( CFEqual(userVisibleName, thisName) ) {
					*setID = (CFStringRef) CFRetain(thisSet);
				}
			}
			if (err == noErr && *setID == NULL) {
				setIndex += 1;
			}
			
			CFQRelease(thisName);
		}
	}
	
	CFQRelease(setIDs);
	
	return err;
}

extern OSStatus MoreSCFCopyDefaultPortBSDName(CFStringRef hardwareType, CFStringRef *bsdName)
	// See comment in header.
{
	OSStatus 	err;
	CFArrayRef 	portArray;

	assert( bsdName != NULL);
	assert(*bsdName == NULL);
	
	portArray = NULL;
	
	err = MoreSCCreatePortArray(&portArray);
	if (err == noErr) {
		CFIndex portCount;
		CFIndex portIndex;
	
		portCount = CFArrayGetCount(portArray);
		for (portIndex = 0; portIndex < portCount; portIndex++) {
			CFStringRef kind;
			
			kind = (CFStringRef) CFDictionaryGetValue( (CFDictionaryRef) CFArrayGetValueAtIndex(portArray, portIndex), kSCPropNetInterfaceHardware );
			if ( CFEqual(kind, hardwareType) ) {
				*bsdName = (CFStringRef) CFRetain( CFDictionaryGetValue( (CFDictionaryRef) CFArrayGetValueAtIndex(portArray, portIndex), kSCPropNetInterfaceDeviceName) );
				break;
			}
		}
	}
	
	CFQRelease(portArray);

	// The following isn't true.  *bsdName can be NULL in the noErr 
	// case if the port isn't found.
	//
	// assert( (err == noErr) == (*bsdName != NULL) );
	
	return err;
}

extern OSStatus MoreSCMakeNewSingleServiceSet(
									CFStringRef					bsdName, 
									CFStringRef					userVisibleName, 
									CFDictionaryRef				serviceSettings,
									CFStringRef *				newSetID)
	// See comment in header.
{
	OSStatus 			err;
	CFStringRef 		newSetIDLocal;
	CFArrayRef  		serviceIDs;
	CFStringRef *		settingsKeys;
	CFDictionaryRef *	settingsValues;
	CFIndex				settingsIndex;
	CFIndex				settingsCount;

	assert(bsdName != NULL);
	assert(userVisibleName != NULL);
	assert(serviceSettings != NULL);
	assert( (newSetID == NULL) || (*newSetID == NULL) );

	newSetIDLocal 	= NULL;
	serviceIDs 		= NULL;
	settingsKeys 	= NULL;
	settingsValues 	= NULL;
	
	settingsCount = CFDictionaryGetCount(serviceSettings);

	// We open up a connection here to ensure that all of our 
	// changes get committed at the end.
	
	err = MoreSCOpen(false, false);
	
	// Get the keys and values from the serviceSettings dictionary.
	
	if (err == noErr) {
		err = CFQAllocate(settingsCount * sizeof(*settingsKeys), (void **) &settingsKeys);
		if (err == noErr) {
			err = CFQAllocate(settingsCount * sizeof(*settingsValues), (void **) &settingsValues);
		}
	}
	if (err == noErr) {
		CFDictionaryGetKeysAndValues(serviceSettings, 
									 (const void **) settingsKeys, 
									 (const void **) settingsValues);
	}
	
	// Create a set and get the list of service IDs.
	
	if (err == noErr) {
		err = MoreSCNewSet(userVisibleName, &newSetIDLocal);
	}
	if (err == noErr) {
		err = MoreSCCopyServiceIDs(newSetIDLocal, &serviceIDs, NULL);
	}
	
	if (err == noErr) {
		CFIndex serviceCount;
		CFIndex serviceIndex;
		
		// Loop through every service in the new set.  If the service's 
		// BSD name matches the port we're configuring, overwrite the 
		// service's entities with ones created from our settings.  
		// For non-matching services, just disable the service.
		
		serviceCount = CFArrayGetCount(serviceIDs);
		for (serviceIndex = 0; serviceIndex < serviceCount; serviceIndex++) {
			CFStringRef thisService;
			CFStringRef thisServiceBSDName;
			
			thisServiceBSDName = NULL;
			thisService = (CFStringRef) CFArrayGetValueAtIndex(serviceIDs, serviceIndex);
			err = MoreSCCopyBSDNameOfService(newSetIDLocal, thisService, &thisServiceBSDName);
			if (err == noErr) {
				if ( CFEqual(bsdName, thisServiceBSDName) ) {
					for (settingsIndex = 0; settingsIndex < settingsCount; settingsIndex++) {
					
						// Now we encounter a slight hack.  If the entity we're setting up 
						// is kSCEntNetInterface, we merge the settings into the entity.  
						// Otherwise we overwrite them entirely.  This is required because 
						// the PPPoE setup (MoreSCMakeNewPPPoESet) needs to be able to 
						// modify some keys in the kSCEntNetInterface entity, but it 
						// doesn't know all of the existing values so it can't do the 
						// merge itself.
						
						if ( CFEqual(settingsKeys[settingsIndex], kSCEntNetInterface) ) {
							CFMutableDictionaryRef entityDict;

							entityDict = NULL;
							err = MoreSCCopyEntityMutable(newSetIDLocal, thisService, 
														  settingsKeys[settingsIndex], &entityDict);
							if (err == noErr) {
								err = CFQDictionaryMerge(entityDict, settingsValues[settingsIndex]);
							}
							if (err == noErr) {
								err = MoreSCSetEntity(newSetIDLocal, thisService, 
													  settingsKeys[settingsIndex], entityDict);
							}
							
							CFQRelease(entityDict);
						} else {
							err = MoreSCSetEntity(newSetIDLocal, thisService, 
												  settingsKeys[settingsIndex], settingsValues[settingsIndex]);
						}
						if (err != noErr) {
							break;
						}
					}
				} else {
					err = MoreSCSetServiceActive(newSetIDLocal, thisService, false);
				}
			}
			
			CFQRelease(thisServiceBSDName);
			
			if (err != noErr) {
				break;
			}
		}
	}
	if (err != noErr) {
		// On error, delete any partially created set.
		if (newSetIDLocal != NULL) {
			(void) MoreSCDeleteSet(newSetIDLocal);
			CFRelease(newSetIDLocal);
			newSetIDLocal = NULL;
		}
	}
	MoreSCClose(&err, true);
	
	// Clean up.
	
	// If we got an error during the MoreSCClose (which might happen 
	// if we have bad privs), make sure we return a NULL set ID.  Note that 
	// this somewhat duplicates the results of the code around MoreSCDeleteSet 
	// above, but it actually handles a distinct case where we have no error 
	// up to the MoreSCClose but then MoreSCClose fails.
	if (err != noErr) {
		CFQRelease(newSetIDLocal);
		newSetIDLocal = NULL;
	}
	CFQRelease(serviceIDs);
	if (newSetID != NULL) {
		*newSetID = newSetIDLocal;
	} else {
		CFQRelease(newSetIDLocal);
	}
	CFQDeallocate(settingsKeys);
	CFQDeallocate(settingsValues);
	
	assert( (err == noErr) == ( (newSetID == NULL) || (*newSetID != NULL) ) );
	
	return err;
}

static OSStatus SetupEntity(CFMutableDictionaryRef settings, CFStringRef protocol, 
							const MoreSCDigest  *digest)
	// Given all of the information required to set up an entity
	// (its set, its service within that set, and its protocol 
	// (ie entity type), and a digest of the value we want), 
	// this routine sets up the entity.
	//
	// digest can be NULL, in which case this routine sets 
	// the entity to have a default value.
{
	OSStatus 		err;
	MoreSCDigest 	digestLocal;
	CFDictionaryRef entityDict;

	assert(settings  != NULL);
	assert(protocol  != NULL);
	
	entityDict = NULL;

	// If we didn't get a digest, create a default one 
	// (all zeroes).
		
	if (digest == NULL) {
		BlockZero(&digestLocal, sizeof(digestLocal));
		digest = &digestLocal;
	}

	// Create an entity dictionary from the digest and put 
	// it into the settings dictionary.
	
	err = MoreSCCreateEntity(protocol, digest, &entityDict);
	if (err == noErr) {
		CFDictionaryAddValue(settings, protocol, entityDict);
	}

	CFQRelease(entityDict);
	
	return err;
}

extern OSStatus MoreSCMakeNewDialupSet(
									CFStringRef					bsdName, 
									CFStringRef					userVisibleName, 
									const MoreSCModemDigest *	modemSettings, 
									const MoreSCPPPDigest * 	pppSettings, 
									const MoreSCIPv4Digest *	ipv4Settings,
									const MoreSCDNSDigest *		dnsSettings,
									CFStringRef *				newSetID)
	// See comment in header.
{
	OSStatus 				err;
	CFStringRef 			portName;
	CFMutableDictionaryRef 	settings;

	assert(userVisibleName != NULL);
	assert(pppSettings != NULL);
	assert(pppSettings->active);
	assert( (newSetID == NULL) || (*newSetID == NULL) );

	portName = NULL;
	settings = NULL;

	// Find the appropriate BSD port name.
	
	if (bsdName == NULL) {
		err = MoreSCFCopyDefaultPortBSDName(kSCEntNetModem, &portName);
	} else {
		portName = (CFStringRef) CFRetain(bsdName);
		err = noErr;
	}
	if (err == noErr && portName == NULL) {
//		assert(false);		// You supplied a BSD name that doesn't exist.
		err = paramErr;		// Or you supplied NULL and there is no default port.
	}
	
	// Create the settings dictionary for dialup.  This is easy because the 
	// defaults work out well.
	
	if (err == noErr) {
		err = CFQDictionaryCreateMutable(&settings);
	}
	if (err == noErr) {
		err = SetupEntity(settings, kSCEntNetPPP, (MoreSCDigest *) pppSettings);
	}
	if (err == noErr && modemSettings != NULL) {
		err = SetupEntity(settings, kSCEntNetModem, (MoreSCDigest *) modemSettings);
	}
	if (err == noErr && ipv4Settings != NULL) {
		err = SetupEntity(settings, kSCEntNetIPv4, (MoreSCDigest *) ipv4Settings);
	}
	if (err == noErr && dnsSettings != NULL) {
		err = SetupEntity(settings, kSCEntNetDNS, (MoreSCDigest *) dnsSettings);
	}
	
	// Create the set using our helper routine.

	if (err == noErr) {
		err = MoreSCMakeNewSingleServiceSet(portName, userVisibleName, settings, newSetID);
	}

	CFQRelease(portName);
	CFQRelease(settings);
	
	assert( (err == noErr) == ( (newSetID == NULL) || (*newSetID != NULL) ) );
	
	return err;
}

extern OSStatus MoreSCMakeNewPPPoESet(
									CFStringRef					bsdName, 
									CFStringRef					userVisibleName, 
									const MoreSCPPPDigest * 	pppSettings, 
									const MoreSCIPv4Digest *	ipv4Settings,
									const MoreSCDNSDigest *		dnsSettings,
									CFStringRef *				newSetID)
{
	OSStatus 				err;
	CFStringRef 			portName;
	CFMutableDictionaryRef 	settings;

	assert(userVisibleName != NULL);
	assert(pppSettings != NULL);
	assert(pppSettings->active);
	assert( (newSetID == NULL) || (*newSetID == NULL) );

	portName = NULL;
	settings = NULL;

	// Find the appropriate BSD port name.
	
	if (bsdName == NULL) {
		err = MoreSCFCopyDefaultPortBSDName(kSCEntNetEthernet, &portName);
	} else {
		portName = (CFStringRef) CFRetain(bsdName);
		err = noErr;
	}
	if (err == noErr && portName == NULL) {
//		assert(false);		// You supplied a BSD name that doesn't exist.
		err = paramErr;		// Or you supplied NULL and there is no default port.
	}
	
	// Create the settings dictionary for PPPoE.  This is somewhat trickier than 
	// dialup because the defaults aren't in our favour.
	
	if (err == noErr) {
		err = CFQDictionaryCreateMutable(&settings);
	}
	if (err == noErr) {
		err = SetupEntity(settings, kSCEntNetPPP, (MoreSCDigest *) pppSettings);
	}
	if (err == noErr) {
		MoreSCIPv4Digest ipv4SettingsLocal;
		
		// The default settings for IPv4 on Ethernet are to use DHCP.  In 
		// the PPPoE case we must use PPP as the default.
		
		if (ipv4Settings == NULL) {
			ipv4Settings = &ipv4SettingsLocal;
			
			BlockZero(&ipv4SettingsLocal, sizeof(ipv4SettingsLocal));
			ipv4SettingsLocal.configMethod = kSCValNetIPv4ConfigMethodPPP;
		}
		err = SetupEntity(settings, kSCEntNetIPv4, (MoreSCDigest *) ipv4Settings);
	}
	if (err == noErr && dnsSettings != NULL) {
		err = SetupEntity(settings, kSCEntNetDNS, (MoreSCDigest *) dnsSettings);
	}
	if (err == noErr) {
		CFDictionaryRef entityDict;
		CFStringRef 	keys[2];
		CFStringRef 	values[2];
		
		keys[0]   = kSCPropNetInterfaceType;
		values[0] = kSCValNetInterfaceTypePPP;
		keys[1]   = kSCPropNetInterfaceSubType;
		values[1] = kSCValNetInterfaceSubTypePPPoE;
		
		entityDict = CFDictionaryCreate(NULL, (const void **) keys, (const void **) values, 2, 
								&kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
		if (entityDict == NULL) {
			err = coreFoundationUnknownErr;
		}		
		if (err == noErr) {
			CFDictionaryAddValue(settings, kSCEntNetInterface, entityDict);
		}
		CFQRelease(entityDict);
	}
	
	// Create the set using our helper routine.

	if (err == noErr) {
		err = MoreSCMakeNewSingleServiceSet(portName, userVisibleName, settings, newSetID);
	}

	CFQRelease(portName);
	CFQRelease(settings);
	
	assert( (err == noErr) == ( (newSetID == NULL) || (*newSetID != NULL) ) );
	
	return err;
}
