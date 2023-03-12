/*
	File:		MoreSCFDigest.c

	Contains:	Routines for working with SC entities.

	Written by:	Quinn

	Copyright:	Copyright (c) 2001 by Apple Computer, Inc., All Rights Reserved.

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

$Log: MoreSCFDigest.c,v $
Revision 1.9  2003/04/14 15:51:21         
Use CFQAllocate/Deallocate to prevent "malloc(0) returns NULL" problems.

Revision 1.8  2003/02/26 20:51:59         
<rdar://problem/3183087> Added support for V.92 modem hold.

Revision 1.7  2003/02/26 12:14:28         
In MoreSCCreateIPv4Entity, digest->configMethod can't be NULL, so let's assert that.

Revision 1.6  2002/11/25 16:49:41         
Changes to build in Project Builder without warnings.

Revision 1.5  2002/11/14 20:16:31         
Tidy up the debug checking in MoreSCCreateIPv4Entity and add a way to disable it for the benefit of MoreSCNewSet.

Revision 1.4  2002/11/09 00:01:55         
Include our prototype early to flush out any missing dependencies. Convert nil to NULL. Convert MoreAssertQ to assert.

Revision 1.3  2002/08/14 16:33:50         
Added support for v 1.3 interfaces changes.  Certain hardwired defaults now get different values on 10.2 and later.  Changed some asserts to fprintfs in MoreSCCreateIPv4Entity because changes for post 10.1 compatibility trigger them.

Revision 1.2  2002/01/22 06:19:42         
Changes to support variant field in MoreSCInterfaceDigest. Also adapted to minor CFQ changes.

Revision 1.1  2002/01/16 22:52:28         
First checked in.


*/

/////////////////////////////////////////////////////////////////

// Our prototypes

#include "MoreSCFDigest.h"

// System interfaces

#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>

// MIB Interfaces

#include "MoreCFQ.h"
#include "MoreSCFPortScanner.h"
#include "MoreSCF.h"

/////////////////////////////////////////////////////////////////
#pragma mark ***** Utilities

extern pascal OSStatus MoreSCFCreateStringWithMacAddress(const UInt8 *macAddr, CFStringRef *result)
	// See comment in header.
	//
	// This code was stolen from Robert's code (CreateMACFromData in "MoreSCFPortScanner.c".  
	// I should unify these eventually.
{
	OSStatus err;
    const UInt8 *p;
    char s [19];			  // mac addresses are 6 bytes long, which translates to 18 chars.
    char *cp = &s[0];
    int len = 6;
    
    assert(macAddr != NULL);
	assert( result != NULL);
	assert(*result == NULL);

    // add digits in pairs with colon separators

	p = macAddr;
    while ( len-- )
        cp += sprintf( cp, "%2.2x:", *p++ );

    // terminate string

    s[17] = '\0';

	err = noErr;
    *result = CFStringCreateWithCString(NULL, s, kCFStringEncodingASCII);
    if (*result == NULL) {
    	err = coreFoundationUnknownErr;
    }

	assert( (err == noErr) == (*result != NULL) );

    return err;
}

extern pascal OSStatus MoreSCFStringToMacAddress(CFStringRef cfStr, UInt8 *macAddr)
	// See comment in header.
{
	OSStatus err;
	char buf[ (6 * 2) + 5 + 1 ];		// 6 hex bytes (2 chars each) + 5 separators + null
	int macAsInts[6];
	
	assert(cfStr   != NULL);
	assert(macAddr != NULL);
	
	err = noErr;
	if ( ! CFStringGetCString(cfStr, buf, sizeof(buf), kCFStringEncodingASCII) ) {
		err = coreFoundationUnknownErr;
	}
	if (err == noErr) {
		if ( sscanf(buf, "%02x:%02x:%02x:%02x:%02x:%02x", 
							&macAsInts[0], &macAsInts[1], &macAsInts[2], 
							&macAsInts[3], &macAsInts[4], &macAsInts[5]) != 6 ) {
			err = -1;
		}
	}
	if (err == noErr) {
		int i;
		
		for (i = 0; i < 6; i++) {
			assert((macAsInts[i] >= 0) && (macAsInts[i] < 256));
			macAddr[i] = (UInt8) macAsInts[i];
		}
	}

	return err;
}

extern pascal OSStatus MoreSCFCreateStringWithInetAddress(in_addr addr, CFStringRef *result)
	// See comment in header.
{
	OSStatus 	err;
	const char *addrStr;

	assert( result != NULL);
	assert(*result == NULL);

	err = noErr;	
	addrStr = inet_ntoa(addr);
	*result = CFStringCreateWithCString(NULL, addrStr, kCFStringEncodingASCII);
	if (*result == NULL) {
		err = coreFoundationUnknownErr;
	}

	assert( (err == noErr) == (*result != NULL) );

	return err;
}

extern pascal OSStatus MoreSCFCreateArrayWithInetAddresses(ItemCount addrCount, const in_addr *addresses, CFArrayRef *result)
	// See comment in header.
{
	OSStatus 			err;
	ItemCount 			index;
	CFMutableArrayRef 	addressesArray;
	
	assert( (addrCount == 0) || (addresses != NULL) );
	assert( result != NULL);
	assert(*result == NULL);
	
	addressesArray = NULL;
	err = CFQArrayCreateMutable(&addressesArray);
	if (err == noErr) {
		for (index = 0; index < addrCount; index++) {
			CFStringRef value;
			
			value = NULL;
			err = MoreSCFCreateStringWithInetAddress(addresses[index], &value);
			if (err == noErr) {
				CFArrayAppendValue(addressesArray, value);
			}
			CFQRelease(value);
			if (err != noErr) {
				break;
			}
		}
	}

	if (err != noErr) {
		CFQRelease(addressesArray);
		addressesArray = NULL;
	}
	*result = addressesArray;

	assert( (err == noErr) == (*result != NULL) );
	
	return err;
}

extern pascal OSStatus MoreSCFCopyEncodedPPPPassword(CFStringRef password, CFDataRef *encodedPassword)
	// See comment in header.
{
	OSStatus err;
	CFIndex  numUniChars;
	UniChar *uniChars;
	
	assert(password != NULL);
	assert( encodedPassword != NULL);
	assert(*encodedPassword == NULL);

	uniChars = NULL;
	
	// Can't use CFStringCreateExternalRepresentation because, according to the 
	// documentation, it always includes a byte order mark (BOM) character.
	
	numUniChars = CFStringGetLength(password);
	err = CFQAllocate(numUniChars * sizeof(UniChar), (void **) &uniChars);
	if (err == noErr) {
		CFStringGetCharacters(password, CFRangeMake(0, numUniChars), uniChars);
		
		*encodedPassword = CFDataCreate(NULL, (UInt8 *) uniChars, numUniChars * (CFIndex) sizeof(UniChar));
		if (*encodedPassword == NULL) {
			err = coreFoundationUnknownErr;
		}
	}
	
	CFQDeallocate(uniChars);
	
	assert( (err == noErr) == (*encodedPassword != NULL) );
	return err;
}

/////////////////////////////////////////////////////////////////
#pragma mark ***** Global Entities

extern pascal OSStatus MoreSCCreateIPv4GlobalEntity(const MoreSCIPv4GlobalDigest *digest, CFDictionaryRef *entity)
	// See comment in header.
{
	OSStatus err;
	CFMutableDictionaryRef result;
	
	assert( digest != NULL);
	assert( entity != NULL);
	assert(*entity == NULL);
	
	result = NULL;
	err = CFQDictionaryCreateMutable(&result);
	if (err == noErr) {
		err = CFQDictionarySetNumber(result, kSCPropNetPPPOverridePrimary, 0);
	}
	if (err == noErr) {
		CFArrayRef serviceOrder;
		
		serviceOrder = CFArrayCreate(NULL, NULL, 0, &kCFTypeArrayCallBacks);
		if (serviceOrder == NULL) {
			err = coreFoundationUnknownErr;
		}
		CFDictionaryAddValue(result, kSCPropNetServiceOrder, serviceOrder);
		CFQRelease(serviceOrder);
	}
	
	if (err != noErr) {
		CFQRelease(result);
		result = NULL;
	}
	*entity = result;
	
	return err;
}

extern pascal OSStatus MoreSCCreateNetInfoGlobalEntity(const MoreSCNetInfoGlobalDigest *digest, CFDictionaryRef *entity)
	// See comment in header.
{
	OSStatus err;
	CFMutableDictionaryRef result;
	
	assert( digest != NULL);
	assert( entity != NULL);
	assert(*entity == NULL);
	
	result = NULL;
	err = CFQDictionaryCreateMutable(&result);
	// No useful values in MoreSCNetInfoGlobalDigest, something 
	// that I should probably address in the future.
	
	*entity = result;
	
	return err;
}

extern pascal OSStatus MoreSCCreateGlobalEntity(CFStringRef protocol, const MoreSCGlobalDigest *digest, CFDictionaryRef *entity)
	// See comment in header.
{
	OSStatus err;
	
	assert(protocol != NULL);
	assert(digest != NULL);
	assert( entity != NULL);
	assert(*entity == NULL);
	
	// Call the appropriate protocol-specific routine based on the 
	// protocol type supplied.

	if (        CFEqual(protocol, kSCEntNetIPv4) ) {
		err = MoreSCCreateIPv4GlobalEntity(&digest->ipv4, entity);
	} else if ( CFEqual(protocol, kSCEntNetNetInfo) ) {
		err = MoreSCCreateNetInfoGlobalEntity(&digest->netInfo, entity);
	} else {
		assert(false);
		err = -1;
	}
	return err;
}

/////////////////////////////////////////////////////////////////
#pragma mark ***** Service Entities

extern pascal OSStatus MoreSCCreateInterfaceEntity(const MoreSCInterfaceDigest *digest, CFDictionaryRef *entity)
	// See comment in header.
{
	OSStatus err;
	CFMutableDictionaryRef result;
	
	assert( digest != NULL);
	assert( entity != NULL);
	assert(*entity == NULL);
	
	result = NULL;
	err = CFQDictionaryCreateMutable(&result);
	if (err == noErr) {
		assert(digest->deviceName != NULL);
		assert(digest->userDefinedName != NULL);
		CFDictionaryAddValue(result, kSCPropNetInterfaceDeviceName, digest->deviceName);
		CFDictionaryAddValue(result, kSCPropUserDefinedName, digest->userDefinedName);
		assert( 	CFEqual(digest->hardware, kSCEntNetAirPort) 
					|| 	CFEqual(digest->hardware, kSCEntNetEthernet) 
					|| 	CFEqual(digest->hardware, kSCEntNetModem) );
		CFDictionaryAddValue(result, kSCPropNetInterfaceHardware, digest->hardware);
		if ( CFEqual(digest->hardware, kSCEntNetModem) ) {
			if (digest->supportsHold) {
				err = CFQDictionarySetNumber(result, kSCPropNetInterfaceSupportsModemOnHold, 1);
			}
		}
	}
	if (err == noErr) {
		assert( 	CFEqual(digest->type, kSCValNetInterfaceTypeEthernet) 
					|| 	CFEqual(digest->type, kSCValNetInterfaceTypePPP) );
		CFDictionaryAddValue(result, kSCPropNetInterfaceType, digest->type);
		if (digest->subType != NULL) {
			assert( CFEqual(digest->type, kSCValNetInterfaceTypePPP) );
			assert( 	CFEqual(digest->subType, kSCValNetInterfaceSubTypePPPSerial) 
						|| 	CFEqual(digest->subType, kSCValNetInterfaceSubTypePPPoE) );
			CFDictionaryAddValue(result, kSCPropNetInterfaceSubType, digest->subType);
		}
		if (digest->variant != NULL) {
			assert( CFEqual(digest->hardware, kSCEntNetModem) );
			CFDictionaryAddValue(result, kMoreSCPropNetInterfaceHardwareVariant, digest->variant);
		}
	}
	
	if (err != noErr) {
		CFQRelease(result);
		result = NULL;
	}
	*entity = result;
	
	assert( (err == noErr) == (*entity != NULL) );
	
	return err;
}

static OSStatus AddProxy(CFMutableDictionaryRef dict, 
                            CFStringRef enableKey, CFStringRef proxyKey,   CFStringRef portKey, 
							Boolean enableValue,   CFStringRef proxyValue, UInt16 portValue)
	// Adds a proxy entry to dict.  The entry consists of up to three 
	// key/value pairs.  The enable property is a number that 
	// indicates whether the proxy is to be used.  It is always 
	// added.  The other two properties, proxy and port, are optional 
	// (controlled by whether their values are NULL or 0).
{
	OSStatus err;
	
	assert(dict        != NULL);
	assert(enableKey   != NULL);
	
	err = CFQDictionarySetNumber(dict, enableKey, enableValue);
	if (err == noErr && proxyValue != NULL) {
		CFDictionaryAddValue(dict, proxyKey, proxyValue);
	}
	if (err == noErr && portValue != 0) {
		err = CFQDictionarySetNumber(dict, portKey, portValue);
	}
	
	return err;
}

extern pascal OSStatus MoreSCCreateProxiesEntity(const MoreSCProxiesDigest *digest, CFDictionaryRef *entity)
	// See comment in header.
{
	OSStatus err;
	CFMutableDictionaryRef result;
	
	assert( digest != NULL);
	assert( entity != NULL);
	assert(*entity == NULL);

	result = NULL;
	err = CFQDictionaryCreateMutable(&result);
	
	// Add the "ExceptionsList" property, if supplied.
	
	if (err == noErr) {
		if (digest->exceptionsList != NULL) {
			CFDictionaryAddValue(result, kSCPropNetProxiesExceptionsList, digest->exceptionsList);
		}
		err = CFQDictionarySetNumber(result, kSCPropNetProxiesFTPPassive, digest->ftpPassive);
	}
	
	// Add properties for each of the 6 possible proxies, if supplied.
	
	if (err == noErr) {
		err = AddProxy(result, kSCPropNetProxiesFTPEnable, kSCPropNetProxiesFTPProxy, kSCPropNetProxiesFTPPort, 
						digest->ftpEnable, digest->ftpProxy, digest->ftpPort);
	}
	if (err == noErr) {
		err = AddProxy(result, kSCPropNetProxiesGopherEnable, kSCPropNetProxiesGopherProxy, kSCPropNetProxiesGopherPort, 
						digest->gopherEnable, digest->gopherProxy, digest->gopherPort);
	}
	if (err == noErr) {
		err = AddProxy(result, kSCPropNetProxiesHTTPEnable, kSCPropNetProxiesHTTPProxy, kSCPropNetProxiesHTTPPort, 
						digest->httpEnable, digest->httpProxy, digest->httpPort);
	}
	if (err == noErr) {
		err = AddProxy(result, kSCPropNetProxiesHTTPSEnable, kSCPropNetProxiesHTTPSProxy, kSCPropNetProxiesHTTPSPort, 
						digest->httpsEnable, digest->httpsProxy, digest->httpsPort);
	}
	if (err == noErr) {
		err = AddProxy(result, kSCPropNetProxiesRTSPEnable, kSCPropNetProxiesRTSPProxy, kSCPropNetProxiesRTSPPort, 
						digest->rtspEnable, digest->rtspProxy, digest->rtspPort);
	}
	if (err == noErr) {
		err = AddProxy(result, kSCPropNetProxiesSOCKSEnable, kSCPropNetProxiesSOCKSProxy, kSCPropNetProxiesSOCKSPort, 
						digest->socksEnable, digest->socksProxy, digest->socksPort);
	}
	
	if (err != noErr) {
		CFQRelease(result);
		result = NULL;
	}
	*entity = result;
	
	return err;
}

extern pascal OSStatus MoreSCCreateAppleTalkEntity(const MoreSCAppleTalkDigest *digest, CFDictionaryRef *entity)
	// See comment in header.
{
	OSStatus err;
	CFMutableDictionaryRef result;
	
	assert( digest != NULL);
	assert( entity != NULL);
	assert(*entity == NULL);

	result    = NULL;
	
	err = CFQDictionaryCreateMutable(&result);
	if (err == noErr) {
		assert( CFEqual(digest->configMethod, kSCValNetAppleTalkConfigMethodNode) );
		CFDictionaryAddValue(result, kSCPropNetAppleTalkConfigMethod, digest->configMethod);

		if (digest->defaultZone != NULL) {
			CFDictionaryAddValue(result, kSCPropNetAppleTalkDefaultZone, digest->defaultZone);
		}

		if (digest->manual) {
			err = CFQDictionarySetNumber(result, kSCPropNetAppleTalkNetworkID, digest->networkID);
			if (err == noErr) {
				err = CFQDictionarySetNumber(result, kSCPropNetAppleTalkNodeID, digest->nodeID);
			}
		}
	}
	if (err == noErr && ! digest->active ) {
		err = CFQDictionarySetNumber(result, kSCResvInactive, 1);
	}
	
	if (err != noErr) {
		CFQRelease(result);
		result = NULL;
	}
	*entity = result;
	
	return err;
}

extern pascal OSStatus MoreSCCreateDNSEntity(const MoreSCDNSDigest *digest, CFDictionaryRef *entity)
	// See comment in header.
{
	OSStatus err;
	CFMutableDictionaryRef result;
	ItemCount index;
	
	assert( digest != NULL);
	assert( entity != NULL);
	assert(*entity == NULL);

	result = NULL;
	
	err = CFQDictionaryCreateMutable(&result);
	
	// If the caller supplied a DNS server addresses, add them 
	// to the entity.
	
	if (err == noErr && digest->serverAddressesCount > 0) {
		CFArrayRef 			addresses;
		
		assert(digest->serverAddresses != NULL);

		addresses = NULL;
		err = MoreSCFCreateArrayWithInetAddresses(digest->serverAddressesCount, 
											 	  digest->serverAddresses, 
											 	  &addresses);
		CFDictionaryAddValue(result, kSCPropNetDNSServerAddresses, addresses);

		CFQRelease(addresses);	
	}
	
	// If the caller support a DNS search domains, add each to the entity.
	
	if (err == noErr && digest->searchDomainsCount > 0) {
		CFMutableArrayRef 	domains;
		
		assert(digest->searchDomains != NULL);

		domains = NULL;
		
		// I could have used CFArrayCreate here but I wanted to loop through 
		// each element so that I can check for NULL.
		
		err = CFQArrayCreateMutable(&domains);
		if (err == noErr) {
			for (index = 0; index < digest->searchDomainsCount; index++) {
				assert(digest->searchDomains[index] != NULL);
				
				CFArrayAppendValue(domains, digest->searchDomains[index]);
			}
		}
		CFDictionaryAddValue(result, kSCPropNetDNSSearchDomains, domains);

		CFQRelease(domains);	
	}

	if (err != noErr) {
		CFQRelease(result);
		result = NULL;
	}
	*entity = result;
	
	return err;
}

#if MORE_DEBUG

	Boolean gMoreSCCreateIPv4EntityDontCheck;

#endif

extern pascal OSStatus MoreSCCreateIPv4Entity(const MoreSCIPv4Digest *digest, CFDictionaryRef *entity)
	// See comment in header.
{
	OSStatus err;
	CFMutableDictionaryRef result;
	
	assert( digest != NULL);
	assert( digest->configMethod != NULL);
	assert( entity != NULL);
	assert(*entity == NULL);

	result = NULL;
	err = CFQDictionaryCreateMutable(&result);
	if (err == noErr) {
	
		// If we're debugging, make sure that the configMethod is one of 
		// the supported options, and that other fields match the 
		// config method (for example, you can't supply a client ID if 
		// you're not using DHCP).
		
		#if MORE_DEBUG
			if ( ! gMoreSCCreateIPv4EntityDontCheck ) {
				OSType  method;
				Boolean printed;
				
				printed = false;
				
				if (CFEqual(digest->configMethod, kSCValNetIPv4ConfigMethodBOOTP)) {
					method = 'boot';
				} else if (CFEqual(digest->configMethod, kSCValNetIPv4ConfigMethodDHCP)) {
					method = 'dhcp';
				} else if (CFEqual(digest->configMethod, kSCValNetIPv4ConfigMethodINFORM)) {
					method = 'info';
				} else if (CFEqual(digest->configMethod, kSCValNetIPv4ConfigMethodManual)) {
					method = 'manu';
				} else if (CFEqual(digest->configMethod, kSCValNetIPv4ConfigMethodPPP)) {
					method = 'ppp ';
				} else {
					method = 'bad!';
				}
				assert(method != 'bad!');

				// These used to be asserts but they are now triggered by 
				// some of our test cases on Mac OS X 10.2, where the default 
				// setup for an Ethernet port is Manual with IP address 0.
				
				if ( (digest->address.s_addr == 0) !=
							  (	   (method == 'boot')
								|| (method == 'dhcp')
								|| (method == 'ppp ')  ) ) {
					fprintf(stderr, "MoreSCCreateIPv4Entity: IP address zero makes sense for BOOTP, DHCP, and PPP config methods only\n");
					printed = true;
				}
				if ( ( digest->subnetMask.s_addr != 0) != (method == 'manu') ) {
					fprintf(stderr, "MoreSCCreateIPv4Entity: Subnet mask non-zero makes sense for Manual config method only\n");
					printed = true;
				}
				if ( ( digest->router.s_addr != 0) != (method == 'manu') ) {
					fprintf(stderr, "MoreSCCreateIPv4Entity: Router address non-zero makes sense for Manual config method only\n");
					printed = true;
				}
				if (printed) {
					fprintf(stderr, "  address      = %08x\n", digest->address.s_addr);
					fprintf(stderr, "  subnetMask   = %08x\n", digest->subnetMask.s_addr);
					fprintf(stderr, "  router       = %08x\n", digest->router.s_addr);
					fprintf(stderr, "  method       = %4.4s\n", (char *) &method);
					fprintf(stderr, "  configMethod = ");
					CFShow(digest->configMethod);
				}
				assert( (digest->clientID == NULL) || (method == 'dhcp') );
			}
		#endif
		
		// Add the config method and client ID (if supplied).
		
		CFDictionaryAddValue(result, kSCPropNetIPv4ConfigMethod, digest->configMethod);
		if (digest->clientID != NULL) {
			CFDictionaryAddValue(result, kSCPropNetIPv4DHCPClientID, digest->clientID);
		}

		// If the caller supplied an address, add it.
		
		if (digest->address.s_addr != 0) {
			CFArrayRef addresses;
			
			addresses = NULL;
			err = MoreSCFCreateArrayWithInetAddresses(1, &digest->address, &addresses);
			
			CFDictionaryAddValue(result, kSCPropNetIPv4Addresses, addresses);
			CFQRelease(addresses);
		}
		
		// If the caller supplied a subnet mask, add it.
		
		if (err == noErr && digest->subnetMask.s_addr != 0) {
			CFArrayRef subnetMasks;
			
			subnetMasks = NULL;
			err = MoreSCFCreateArrayWithInetAddresses(1, &digest->subnetMask, &subnetMasks);
			
			CFDictionaryAddValue(result, kSCPropNetIPv4SubnetMasks, subnetMasks);
			CFQRelease(subnetMasks);
		}

		// If the caller supplied a router mask, add it.
		
		if (err == noErr && digest->router.s_addr != 0) {
			CFStringRef router;
			
			router = NULL;
			err = MoreSCFCreateStringWithInetAddress(digest->router, &router);
			CFDictionaryAddValue(result, kSCPropNetIPv4Router, router);
			CFQRelease(router);
		}
	}
	
	if (err != noErr) {
		CFQRelease(result);
		result = NULL;
	}
	*entity = result;
	
	return err;
}

// This structure is the default options for PPP if the caller doesn't supply 
// an options structure.

const MoreSCPPPOptions kMoreSCPPPOptionsDefaultTenOne =
{
	false,		// dialOnDemand
	false,		// idleReminder
	1800,		// idleReminderTimer
	true,		// disconnectOnIdle
	900,		// disconnectOnIdleTimer
	true,		// disconnectOnLogout
	true,		// commRedialEnabled
	1,			// commRedialCount
	30,			// commRedialInterval
	true,		// lcpEchoEnabled
	true,		// ipcpCompressionVJ
	false,		// commDisplayTerminalWindow
	false		// verboseLogging
};

const MoreSCPPPOptions kMoreSCPPPOptionsDefaultTenTwoAndLater =
{
	false,		// dialOnDemand
	false,		// idleReminder
	1800,		// idleReminderTimer
	true,		// disconnectOnIdle
	600,		// disconnectOnIdleTimer
	true,		// disconnectOnLogout
	true,		// commRedialEnabled
	1,			// commRedialCount
	5,			// commRedialInterval
	true,		// lcpEchoEnabled
	true,		// ipcpCompressionVJ
	false,		// commDisplayTerminalWindow
	false		// verboseLogging
};

const MoreSCPPPOptions kMoreSCPPPoEOptionsDefaultTenTwoAndLater =
{
	false,		// dialOnDemand
	false,		// idleReminder
	1800,		// idleReminderTimer
	false,		// disconnectOnIdle
	1800,		// disconnectOnIdleTimer
	true,		// disconnectOnLogout
	true,		// commRedialEnabled
	1,			// commRedialCount
	5,			// commRedialInterval
	true,		// lcpEchoEnabled
	true,		// ipcpCompressionVJ
	false,		// commDisplayTerminalWindow
	false		// verboseLogging
};

static Boolean gTenTwoOrLater;

extern const MoreSCPPPOptions *MoreSCGetDefaultPPPOptions(Boolean isPPPoE)
{
	const MoreSCPPPOptions *result;
	UInt32					sysVer;
	
	gTenTwoOrLater = (Gestalt(gestaltSystemVersion, (SInt32 *) &sysVer) == noErr) 
					 && (sysVer >= 0x01020);
	if (gTenTwoOrLater) {
		if (isPPPoE) {
			result = &kMoreSCPPPoEOptionsDefaultTenTwoAndLater;
		} else {
			result = &kMoreSCPPPOptionsDefaultTenTwoAndLater;
		}
	} else {
		result = &kMoreSCPPPOptionsDefaultTenOne;
	}
	return result;
}

// This trickery allows us to build with either 10.1 or 10.2 headers.

#if !defined(kSCPropNetPPPCommUseTerminalScript)
	#define kSCPropNetPPPCommUseTerminalScript       SCSTR("CommUseTerminalScript")           /* CFNumber (0 or 1) */
#endif

extern pascal OSStatus MoreSCCreatePPPEntity(const MoreSCPPPDigest *digest, CFDictionaryRef *entity)
	// See comment in header.
{
	OSStatus err;
	CFMutableDictionaryRef result;
	const MoreSCPPPOptions *options;
	
	assert( digest != NULL);
	assert( entity != NULL);
	assert(*entity == NULL);

	// If the caller supplied options, use them, otherwise use the defaults.
	// Note that we always call MoreSCGetDefaultPPPOptions to guarantee that 
	// gTenTwoOrLater has been set up, which we need below.
	
	options = MoreSCGetDefaultPPPOptions(false);
	if (digest->options != NULL) {
		options = digest->options;
	}
	result = NULL;
	err = CFQDictionaryCreateMutable(&result);
	
	if (err == noErr) {
	
		// Set the user-visible name, if supplied.
		
		if (digest->userDefinedName != NULL) {
			CFDictionaryAddValue(result, kSCPropUserDefinedName, digest->userDefinedName);
		}		
		
		// If the entity isn't active, record that.
		
		if (! digest->active ) {
			err = CFQDictionarySetNumber(result, kSCResvInactive, 1);
		}
	}
	
	// Identifiers listed in comments are included in "SCSchemaDefinitions.h"
	// but aren't created by the Network preferences panel by default (and 
	// not created by us (or it) by default).
	
	// PPP-level properties
	
	if (err == noErr) {
		err = CFQDictionarySetNumber(result, kSCPropNetPPPDialOnDemand, options->dialOnDemand);
	}
	if (err == noErr) {
		err = CFQDictionarySetNumber(result, kSCPropNetPPPDisconnectOnIdle, options->disconnectOnIdle);
	}
	if (err == noErr) {
		err = CFQDictionarySetNumber(result, kSCPropNetPPPDisconnectOnIdleTimer, options->disconnectOnIdleTimer);
	}
	if (err == noErr) {
		err = CFQDictionarySetNumber(result, kSCPropNetPPPDisconnectOnLogout, options->disconnectOnLogout);
	}
	if (err == noErr) {
		err = CFQDictionarySetNumber(result, kSCPropNetPPPIdleReminder, options->idleReminder);
	}
	if (err == noErr) {
		err = CFQDictionarySetNumber(result, kSCPropNetPPPIdleReminderTimer, options->idleReminderTimer);
	}
	if (err == noErr) {
		CFDictionaryAddValue(result, kSCPropNetPPPLogfile, CFSTR("/tmp/ppp.log"));
	}
	// kSCPropNetPPPSessionTimer
	if (err == noErr) {
		err = CFQDictionarySetNumber(result, kSCPropNetPPPVerboseLogging, options->verboseLogging);
	}

	// Authentication properties

	if (err == noErr && digest->authName != NULL) {
		CFDictionaryAddValue(result, kSCPropNetPPPAuthName, digest->authName);
	}
	if (err == noErr && digest->authPassword != NULL) {
		CFDataRef encodedPassword;
		
		encodedPassword = NULL;
		err = MoreSCFCopyEncodedPPPPassword(digest->authPassword, &encodedPassword);
		if (err == noErr) {
			CFDictionaryAddValue(result, kSCPropNetPPPAuthPassword, encodedPassword);
		}
		
		CFQRelease(encodedPassword);
	}
	// kSCPropNetPPPAuthPasswordEncryption
	// kSCPropNetPPPAuthProtocol
		
	// Communications-level properties
	
	if (err == noErr && digest->commAlternateRemoteAddress != NULL) {
		CFDictionaryAddValue(result, kSCPropNetPPPCommAlternateRemoteAddress, digest->commAlternateRemoteAddress);
	}
	// kSCPropNetPPPCommConnectDelay
	if (err == noErr) {
		err = CFQDictionarySetNumber(result, kSCPropNetPPPCommDisplayTerminalWindow, options->commDisplayTerminalWindow);
	}
	if (err == noErr) {
		err = CFQDictionarySetNumber(result, kSCPropNetPPPCommRedialCount, options->commRedialCount);
	}
	if (err == noErr) {
		err = CFQDictionarySetNumber(result, kSCPropNetPPPCommRedialEnabled, options->commRedialEnabled);
	}
	if (err == noErr) {
		err = CFQDictionarySetNumber(result, kSCPropNetPPPCommRedialInterval, options->commRedialInterval);
	}
	if (err == noErr && digest->commRemoteAddress != NULL) {
		CFDictionaryAddValue(result, kSCPropNetPPPCommRemoteAddress, digest->commRemoteAddress);
	}
	// kSCPropNetPPPCommTerminalScript
	if (err == noErr) {
		if (gTenTwoOrLater) {
			err = CFQDictionarySetNumber(result, kSCPropNetPPPCommUseTerminalScript, 0);
		}
	}

	// IPCP-level properties
	
	if (err == noErr) {
		err = CFQDictionarySetNumber(result, kSCPropNetPPPIPCPCompressionVJ, options->ipcpCompressionVJ);
	}

	// LCP-level properties
	
	if (err == noErr) {
		err = CFQDictionarySetNumber(result, kSCPropNetPPPLCPEchoEnabled, options->lcpEchoEnabled);
	}
	if (err == noErr) {
		if (gTenTwoOrLater) {
			err = CFQDictionarySetNumber(result, kSCPropNetPPPLCPEchoFailure, 4);
		} else {
			err = CFQDictionarySetNumber(result, kSCPropNetPPPLCPEchoFailure, 3);
		}
	}
	if (err == noErr) {
		err = CFQDictionarySetNumber(result, kSCPropNetPPPLCPEchoInterval, 10);
	}
	// kSCPropNetPPPLCPCompressionACField
	// kSCPropNetPPPLCPCompressionPField
	// kSCPropNetPPPLCPMRU
	// kSCPropNetPPPLCPMTU
	// kSCPropNetPPPLCPReceiveACCM
	// kSCPropNetPPPLCPTransmitACCM
	
	if (err != noErr) {
		CFQRelease(result);
		result = NULL;
	}
	*entity = result;
	
	return err;
}

extern pascal OSStatus MoreSCCreateAirPortEntity(const MoreSCAirPortDigest *digest, CFDictionaryRef *entity)
	// See comment in header.
{
	OSStatus err;
	CFMutableDictionaryRef result;
	
	assert( digest != NULL);
	assert( entity != NULL);
	assert(*entity == NULL);

	result = NULL;
	err = CFQDictionaryCreateMutable(&result);
	if (err == noErr) {
		CFStringRef mac;
		
		mac = NULL;
		err = MoreSCFCreateStringWithMacAddress(digest->macAddress, &mac);
		CFDictionaryAddValue(result, kSCPropMACAddress, mac);
		CFQRelease(mac);
	}
	if (err == noErr && digest->preferredNetwork != NULL) {
		CFDictionaryAddValue(result, kSCPropNetAirPortPreferredNetwork, digest->preferredNetwork);
	}
	
	// ��� "SCSchemaDefinitions.h" defines the following.  Not sure 
	// what to do with them right now.
	
	// kSCPropNetAirPortPowerEnabled
	// kSCPropNetAirPortAuthPassword
	// kSCPropNetAirPortAuthPasswordEncryption
	
	if (err != noErr) {
		CFQRelease(result);
		result = NULL;
	}
	*entity = result;
	
	return err;
}

extern pascal OSStatus MoreSCCreateEthernetEntity(const MoreSCEthernetDigest *digest, CFDictionaryRef *entity)
	// See comment in header.
{
	OSStatus err;
	CFMutableDictionaryRef result;
	
	assert( digest != NULL);
	assert( entity != NULL);
	assert(*entity == NULL);

	result = NULL;
	err = CFQDictionaryCreateMutable(&result);
	if (err == noErr) {
		CFStringRef mac;
		
		mac = NULL;
		err = MoreSCFCreateStringWithMacAddress(digest->macAddress, &mac);
		CFDictionaryAddValue(result, kSCPropMACAddress, mac);
		CFQRelease(mac);
	}
	
	if (err != noErr) {
		CFQRelease(result);
		result = NULL;
	}
	*entity = result;
	
	return err;
}

// This structure contains the default options for modem.  It's 
// exported to clients but isn't referenced by this module itself.

const MoreSCModemDigest kMoreSCModemDigestDefault =
{
	NULL,			// connectionScript
	true,			// dataCompressionErrorCorrection
	true,			// speaker
	false,			// pulseDial
	true,			// waitForDialTone
	false,			// supportsHold
};

extern pascal OSStatus MoreSCCreateModemEntity(const MoreSCModemDigest *digest, CFDictionaryRef *entity)
	// See comment in header.
	//
	// Note that the dataCompressionErrorCorrection digest field controls 
	// both kSCPropNetModemConnectionScript and kSCPropNetModemErrorCorrection.  
	// This is inline with the current user interface, which has a single 
	// checkbox that controls both of these parameters.
{
	OSStatus err;
	CFMutableDictionaryRef result;
	
	assert( digest != NULL);
	assert( entity != NULL);
	assert(*entity == NULL);

	result = NULL;
	err = CFQDictionaryCreateMutable(&result);
	if (err == noErr) {
		assert(digest->connectionScript != NULL);
		CFDictionaryAddValue(result, kSCPropNetModemConnectionScript, digest->connectionScript);
	}
	if (err == noErr) {
		err = CFQDictionarySetNumber(result, kSCPropNetModemDataCompression, digest->dataCompressionErrorCorrection);
	}
	if (err == noErr) {
		if (digest->waitForDialTone) {
			CFDictionaryAddValue(result, kSCPropNetModemDialMode, kSCValNetModemDialModeWaitForDialTone);
		} else {
			CFDictionaryAddValue(result, kSCPropNetModemDialMode, kSCValNetModemDialModeIgnoreDialTone);
		}
	}
	if (err == noErr) {
		err = CFQDictionarySetNumber(result, kSCPropNetModemErrorCorrection, digest->dataCompressionErrorCorrection);
	}
	if (err == noErr) {
		err = CFQDictionarySetNumber(result, kSCPropNetModemPulseDial, digest->pulseDial);
	}
	if (err == noErr) {
		err = CFQDictionarySetNumber(result, kSCPropNetModemSpeaker, digest->speaker);
	}

	// kSCPropNetModemSpeed is listed in "SCSchemaDefinitions.h" but I don't 
	// think it's ever stored in preferences; it more likely to be found in 
	// the dynamic store.

	if (err == noErr && digest->supportsHold) {
		err = CFQDictionarySetNumber(result, kSCPropNetModemHoldCallWaitingAudibleAlert, 1);
		if (err == noErr) {
			err = CFQDictionarySetNumber(result, kSCPropNetModemHoldDisconnectOnAnswer, 0);
		}
		if (err == noErr) {
			err = CFQDictionarySetNumber(result, kSCPropNetModemHoldEnabled, 0);
		}
		if (err == noErr) {
			err = CFQDictionarySetNumber(result, kSCPropNetModemHoldReminder, 1);
		}
		if (err == noErr) {
			err = CFQDictionarySetNumber(result, kSCPropNetModemHoldReminderTime, 10);
		}
	}
	
	if (err != noErr) {
		CFQRelease(result);
		result = NULL;
	}
	*entity = result;
	
	return err;
}

extern pascal OSStatus MoreSCCreateEntity(CFStringRef protocol, const MoreSCDigest *digest, CFDictionaryRef *entity)
	// See comment in header.
{
	OSStatus err;
	
	assert(protocol != NULL);
	assert(digest != NULL);
	assert( entity != NULL);
	assert(*entity == NULL);
	
	// Call the appropriate protocol-specific routine based on the 
	// protocol type supplied.
	
	if (        CFEqual(protocol, kSCEntNetInterface) ) {
		err = MoreSCCreateInterfaceEntity(&digest->interface, entity);
	} else if ( CFEqual(protocol, kSCEntNetProxies) ) {
		err = MoreSCCreateProxiesEntity(&digest->proxies, entity);
	} else if ( CFEqual(protocol, kSCEntNetAppleTalk) ) {
		err = MoreSCCreateAppleTalkEntity(&digest->appleTalk, entity);
	} else if ( CFEqual(protocol, kSCEntNetDNS) ) {
		err = MoreSCCreateDNSEntity(&digest->dns, entity);
	} else if ( CFEqual(protocol, kSCEntNetIPv4) ) {
		err = MoreSCCreateIPv4Entity(&digest->ipv4, entity);
	} else if ( CFEqual(protocol, kSCEntNetPPP) ) {
		err = MoreSCCreatePPPEntity(&digest->ppp, entity);
	} else if ( CFEqual(protocol, kSCEntNetAirPort) ) {
		err = MoreSCCreateAirPortEntity(&digest->airPort, entity);
	} else if ( CFEqual(protocol, kSCEntNetEthernet) ) {
		err = MoreSCCreateEthernetEntity(&digest->ethernet, entity);
	} else if ( CFEqual(protocol, kSCEntNetModem) ) {
		err = MoreSCCreateModemEntity(&digest->modem, entity);
	} else {
		assert(false);
		err = -1;
	}
	return err;
}

