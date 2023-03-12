/*
	File:		MoreSCFPortScanner.c

	Contains:	Code to generate list of network ports on Mac OS X.

	Written by:	DTS, based on code by Network preferences engineering

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

$Log: MoreSCFPortScanner.c,v $
Revision 1.20  2006/11/24 15:01:12         
Make it possible to explicitly test the old port scanning code.

Revision 1.19  2006/11/24 12:43:52         
Added a new implementation that calls the real API.  Added rudimentary Bluetooth and FireWire support for the old implementation.  Don't show hidden serial ports.  Other minor changes.

Revision 1.18  2006/11/23 15:04:13         
Correct an uninitialised variable bug in previous check in.

Revision 1.17  2006/11/23 14:58:10         
When getting the "name" property, handle it being a CFString as well as a CFData.

Revision 1.16  2006/03/27 14:42:37         
Eliminate high-bit set characters.

Revision 1.15  2006/03/24 15:44:42         
Updated copyright.

Revision 1.14  2006/03/24 12:38:50         
Eliminate "pascal" keyword.

Revision 1.13  2006/03/24 11:30:16         
Eliminated "MoreSetup.h" to make it easier for folks to copy MIB source into their projects.

Revision 1.12  2006/03/23 15:19:31         
Correct a compiler warning by adopting IO_OBJECT_NULL.

Revision 1.11  2003/03/11 23:20:45         
Fixed a crashing bug in CopyUserVisibleSlotName caused by not retaining the constant strings that we return.

Revision 1.10  2003/02/26 21:32:24         
Only translate slot names to numbers on 10.2 and later.

Revision 1.9  2003/02/26 20:58:37         
<rdar://problem/3183087> Added support for V.92 modem hold. <rdar://problem/3182842> We now translate slot names into user friendly numbers. <rdar://problem/3182889> Handle USB "Product Name" more like the Network control panel.

Revision 1.8  2003/02/26 12:35:09         
C++ compatibility.

Revision 1.7  2003/01/20 07:54:16         
Correctly handle the case where the user visible name returned by the port name callback is NULL.  This was always allowed in the documentation, but it didn't work.

Revision 1.6  2002/11/25 16:49:03         
Handle weird devices that have no "name" property.

Revision 1.5  2002/11/09 00:02:11         
Include our prototype early to flush out any missing dependencies. Convert nil to NULL. Convert MoreAssertQ to assert.

Revision 1.4  2002/10/25 16:41:15         
Messed up handling of BSD name in SubstituteKeywordsInUserVisibleName [3084354].

Revision 1.3  2002/08/14 13:45:06         
Fix problem with IrDA ports on tray load iMacs showing up even though they're not supported on Mac OS X.

Revision 1.2  2002/01/22 06:17:20         
Total rewrite. Kept Robert's ideas but complete rewrote the implementation and also changed the API to work better in an SCF environment.

Revision 1.1  2002/01/16 22:52:36         
First checked in.


*/

/////////////////////////////////////////////////////////////////

// Our prototypes

#include "MoreSCFPortScanner.h"

// System Interfaces

#include <net/if_types.h>
#include <mach/mach.h>
#include <IOKit/IOKitLib.h>
#include <IOKit/IOBSD.h>
#include <IOKit/network/IONetworkController.h>
#include <IOKit/network/IONetworkInterface.h>
#include <IOKit/network/IOEthernetInterface.h>
#include <IOKit/serial/IOSerialKeys.h>
#include <IOKit/ndrvsupport/IOMacOSTypes.h>
#include <IOKit/storage/IOStorageDeviceCharacteristics.h>
#include <SystemConfiguration/SystemConfiguration.h>

// MIB prototypes

#include "MoreCFQ.h"
#include "MoreSCF.h"

/////////////////////////////////////////////////////////////////

#if MAC_OS_X_VERSION_MIN_REQUIRED >= MAC_OS_X_VERSION_10_4 && ! MORE_SCF_NO_DEPRECATION_WARNINGS
    #warning MoreSCF is deprecated if you are building for 10.4 or later.
#endif

/////////////////////////////////////////////////////////////////
#pragma mark ***** Old Implementation

// Ports are sorted by a primary sort order (defined by the enumeration 
// below) and then sorted by name within their groups.  This enumeration 
// closely mirrors the behaviour of the Network preferences panel.

enum {
	kInternalModemSortOrder 	,
	kUSBModemSortOrder 			,
	kModemSortOrder 			,
	kBluetoothSortOrder         ,
	kIrDASerialSortOrder		,
	kSerialSortOrder 			,
	kBuiltInEthernetSortOrder 	,
	kBuiltInFireWireSortOrder 	,
	kEthernetPCISortOrder 		,
	kFireWireSortOrder 			,
	kAirPortSortOrder 			,
	kWirelessSortOrder 			,
	kEthernetSortOrder 			,
	kDefaultSortOrder 			= 100
};

#define kSortOrderKey CFSTR("SortOrder")
	// We use this key to store the sort order inside the port's 
	// dictionary, which means we don't have to track it by other 
	// means.
	
static long SortOrder(CFDictionaryRef portDict)
	// Extract the kSortOrderKey from the dictionary as a number.
{
	long 		result;
	CFNumberRef numRef;

	assert(portDict != NULL);
		
	result = kDefaultSortOrder;
	
	numRef = (CFNumberRef) CFDictionaryGetValue(portDict, kSortOrderKey);
	if (numRef != NULL) {
        assert( CFGetTypeID(numRef) == CFNumberGetTypeID() );
		(void) CFNumberGetValue(numRef, kCFNumberLongType, &result);
	}
	
	return result;
}

static CFComparisonResult PortSorter(const void *lhs, const void *rhs, void *context)
	// Compares two port dictionaries and orders them appropriately.
{
	#pragma unused(context)
	CFDictionaryRef lhsDict;
	CFDictionaryRef rhsDict;
	int lhsOrder;
	int rhsOrder;

	assert(lhs != NULL);
	assert(rhs != NULL);
	
	lhsDict = (CFDictionaryRef) lhs;
	assert( CFGetTypeID(lhsDict) == CFDictionaryGetTypeID() );
	rhsDict = (CFDictionaryRef) rhs;
	assert( CFGetTypeID(rhsDict) == CFDictionaryGetTypeID() );
	
	lhsOrder = SortOrder(lhsDict);
	rhsOrder = SortOrder(rhsDict);

    if (lhsOrder > rhsOrder)
        return kCFCompareGreaterThan;

    if (lhsOrder < rhsOrder)
        return kCFCompareLessThan;

    // objects are the same type, so order by name

    return CFStringCompare( 
                (CFStringRef) CFDictionaryGetValue(lhsDict, kSCPropUserDefinedName), 
                (CFStringRef) CFDictionaryGetValue(rhsDict, kSCPropUserDefinedName), kCFCompareLocalized);
}

static CFStringRef CreateStringFromProperty(CFTypeRef prop)
	// Some IOKit strings are encoded in CFDatas.  Some are encoded as CFStrings.  
    // Some have trailing null characters.  Handle all of these edge cases.
{
    CFStringRef result;

	assert(prop != NULL);
    
    result = NULL;
    
    if ( CFGetTypeID(prop) == CFDataGetTypeID() ) {
        result = CFStringCreateWithBytes(NULL, CFDataGetBytePtr(prop), CFDataGetLength(prop), kCFStringEncodingMacRoman, false);
    } else if ( CFGetTypeID(prop) == CFStringGetTypeID() ) {
        (void) CFRetain(prop);
        result = (CFStringRef) prop;
    }
    
    if (result != NULL) {
        CFIndex         len;
        CFStringRef     tmp;
        
        len = CFStringGetLength(result);
        while ( (len > 0) && (CFStringGetCharacterAtIndex(result, len - 1) == 0) ) {
            len -= 1;
        }
        
        tmp = CFStringCreateWithSubstring(NULL, result, CFRangeMake(0, len));
        CFRelease(result);          // Do the swap even if tmp is NULL, that way the 
        result = tmp;               // function as a whole returns NULL.
    }
    
    return result;
}

static CFStringRef CreateMACFromData(CFDataRef data)
{
    CFIndex             dataCount;
    CFIndex             dataIndex;
    CFMutableStringRef  result;
    
    assert(data != NULL);

    result = NULL;

    dataCount = CFDataGetLength(data);
    assert( (dataCount == 6) || (dataCount == 8) );     // Ethernet == 6, FireWire == 8, anything else we should investigate
    
    if (dataCount > 0) {
        result = CFStringCreateMutable(NULL, 0);
        assert(result != NULL);
    }
    
    if (result != NULL) {
        for (dataIndex = 0; dataIndex < dataCount; dataIndex++) {
            CFStringAppendFormat(result, NULL, CFSTR("%2.2x:"), CFDataGetBytePtr(data)[dataIndex]);
        }
        CFStringTrim(result, CFSTR(":"));   // get rid of last colon
    } else {
        assert(false);
    }
 
    return result;
}

static Boolean CFEqualString( CFStringRef s1, CFStringRef s2 )
	// Compares two CFStringRefs and return true if they're equal.
	// Handles NULL, which CFEqual does not.
{
    if (( s1 == NULL ) || ( s2 == NULL ))
        return false;

    if ( s1 == s2 )
        return true;

    return ( CFStringCompare( s1, s2, 0 ) == kCFCompareEqualTo );
}

static MoreSCFPortNameCallback gPortNameCallback = NULL;

static kern_return_t SubstituteKeywordsInUserVisibleName(io_object_t interface, 
														 CFMutableDictionaryRef interfaceInfo, 
														 CFStringRef nameTemplate, 
														 CFStringRef *userVisibleName)
	// This routine creates a new string and returns it in *userVisibleName. 
	// The string is based on nameTemplate, with any of the keywords (defined 
	// in our header file) replace with the appropriate strings.
	// 
	// I could probably write better, more general, code to do this, but 
	// this is the way the code turned out (after several design iterations) 
	// and it's not too atrocious.
	//
	// Note that this routine assumes that whoever set up nameTemplate 
	// to included a keyword also included the appropriate string in the 
	// interfaceInfo dictionary.  This might not be true if the client's 
	// callback routine returns a wacky localisation for the user-visible 
	// name.  I haven't seen fit to fix this because it's an obscure 
	// case and the client's callback could fix it by adding the appropriate 
	// keys to the interfaceInfo dictionary (which it is passed).
	//
	// I originally tried to using CFStringCreateWithFormat to create 
	// the output string (hence the keyword syntax) but I discovered 
	// that the CF routine requires you to always substitute all keywords 
	// (which is obvious when you consider how varargs works, but it 
	// threw me for a loop).  So I abandoned CFStringCreateWithFormat, but 
	// I couldn't think of any good reason to change the keyword syntax.
{
	#pragma unused(interface)
	kern_return_t 		err;
	CFMutableStringRef  result;
	CFRange				foundRange;
	
	assert( interface       != IO_OBJECT_NULL);
	assert( interfaceInfo   != NULL);
	assert( nameTemplate    != NULL);
	assert( userVisibleName != NULL);
	assert(*userVisibleName == NULL);

	err = 0;
	result = CFStringCreateMutableCopy(NULL, 0, nameTemplate);
	if (result == NULL) {
		err = -1;
	}
	
	// BSD Name
	
	if ( (err == 0) && CFStringFindWithOptions(result, CFSTR("%1$@"), CFRangeMake(0, CFStringGetLength(result)), kNilOptions, &foundRange) ) {
		assert(CFDictionaryGetValue(interfaceInfo, CFSTR(kIOBSDNameKey)) != NULL);
		CFStringReplace(result, foundRange, (CFStringRef) CFDictionaryGetValue(interfaceInfo, CFSTR(kIOBSDNameKey)));
	}
	
	// PCI Slot Name
	
	if ( (err == 0) && CFStringFindWithOptions(result, CFSTR("%2$@"), CFRangeMake(0, CFStringGetLength(result)), kNilOptions, &foundRange) ) {
		assert(CFDictionaryGetValue(interfaceInfo, CFSTR("AAPL,slot-name")) != NULL);
		CFStringReplace(result, foundRange, (CFStringRef) CFDictionaryGetValue(interfaceInfo, CFSTR("AAPL,slot-name")));
	}
	
	// PCI Function Number
	
	if ( (err == 0) && CFStringFindWithOptions(result, CFSTR("%3$@"), CFRangeMake(0, CFStringGetLength(result)), kNilOptions, &foundRange) ) {
		assert(CFDictionaryGetValue(interfaceInfo, CFSTR("IOChildIndex")) != NULL);
        CFStringReplace(result, foundRange, (CFStringRef) CFDictionaryGetValue(interfaceInfo, CFSTR("IOChildIndex")));
	}
	
	// IOTTYBaseName
	
	if ( (err == 0) && CFStringFindWithOptions(result, CFSTR("%4$@"), CFRangeMake(0, CFStringGetLength(result)), kNilOptions, &foundRange) ) {
		assert(CFDictionaryGetValue(interfaceInfo, CFSTR(kIOTTYBaseNameKey)) != NULL);
		CFStringReplace(result, foundRange, (CFStringRef) CFDictionaryGetValue(interfaceInfo, CFSTR(kIOTTYBaseNameKey)));
	}
	
	// Clean up.

	if (err == 0) {
		*userVisibleName = result;
	} else {
		CFQRelease(result);
	}
	
	assert( (err == 0) == (*userVisibleName != NULL) );
	
	return err;
}

static CFStringRef CopyUserVisibleSlotName(CFStringRef slotName)
	// Translate the PCI card slot name into a user friendly number.
	// 
	// Machine							Slot Name (in numeric order, starting at 1)
	// ---------						---------
	// Beige G3 						A1,  B1,  C1
	// B&W G3, G4 (PCI Graphics)		J12, J11, J10, J9
	// G4 (AGP Graphics)				A,   B,   C,   D
	// G4 (Digital Audio)...			1,   2,   3,   4, 5
	// ... and later					ditto
{
	CFStringRef 	result;
	CFStringRef 	slotPrefix;
	CFStringRef		trimmedSlotName;
	
	assert(slotName != NULL);

	result			= NULL;		// not really necessary, but used for post-condition debug assert
	slotPrefix      = NULL;
	trimmedSlotName = NULL;
	
	slotPrefix = CFSTR("SLOT-");
	if ( CFStringHasPrefix(slotName, slotPrefix) ) {
		trimmedSlotName = CFStringCreateWithSubstring(NULL, 
										  slotName, 
										  CFRangeMake(CFStringGetLength(slotPrefix), 
													  CFStringGetLength(slotName) - CFStringGetLength(slotPrefix))
										 );
	}
	if (trimmedSlotName == NULL) {
		trimmedSlotName = (CFStringRef) CFRetain(slotName);
	}

	// The Network preferences panel on 10.2 and later will translate 
	// slot names into slot numbers.  We replicate it's behaviour here.
    
	if (MoreSCGetSystemVersion() >= 0x01020) {
	
		// Beige G3
		
		if ( CFEqual(trimmedSlotName, CFSTR("A1")) ) {
			result = CFSTR("1");
		} else if ( CFEqual(trimmedSlotName, CFSTR("B1")) ) {
			result = CFSTR("2");
		} else if ( CFEqual(trimmedSlotName, CFSTR("C1")) ) {
			result = CFSTR("3");
			
		// Blue and White G3, G4 (PCI Graphics)
		
		} else if ( CFEqual(trimmedSlotName, CFSTR("J12")) ) {
			result = CFSTR("1");
		} else if ( CFEqual(trimmedSlotName, CFSTR("J11")) ) {
			result = CFSTR("2");
		} else if ( CFEqual(trimmedSlotName, CFSTR("J10")) ) {
			result = CFSTR("3");
		} else if ( CFEqual(trimmedSlotName, CFSTR("J9")) ) {
			result = CFSTR("4");
			
		// G4 (AGP Graphics)

		} else if ( CFEqual(trimmedSlotName, CFSTR("A")) ) {
			result = CFSTR("1");
		} else if ( CFEqual(trimmedSlotName, CFSTR("B")) ) {
			result = CFSTR("2");
		} else if ( CFEqual(trimmedSlotName, CFSTR("C")) ) {
			result = CFSTR("3");
		} else if ( CFEqual(trimmedSlotName, CFSTR("D")) ) {
			result = CFSTR("4");
		
		// all later models
		
		} else {
			result = trimmedSlotName;
		}
	} else {
		result = trimmedSlotName;
	}

	CFRetain(result);
	
	CFQRelease(trimmedSlotName);

	assert(result != NULL);
		
	return result;
}

static kern_return_t CopyUserVisibleNameForEthernetLikePort(io_object_t interface, 
														CFMutableDictionaryRef interfaceInfo,
														CFStringRef *userVisibleName)
	// Given an Ethernet-like device (interface) and a dictionary of 
	// information about the device (interfaceInfo), guess at the 
	// user-visible name for the device.
	// 
	// I'm somewhat unhappy with this code (both its length and its style) 
	// but there's no point fixing it now that Apple provides a proper 
    // API for this (and thus this code only runs on old systems).
{
	kern_return_t 			err;
	kern_return_t			junk;
	io_object_t 			controller;
	io_object_t				bus;
	CFStringRef				bsdName;
	CFMutableDictionaryRef 	controllerDict;
	
	assert(interface        != IO_OBJECT_NULL);
	assert(interfaceInfo    != NULL);
	assert( userVisibleName != NULL);
	assert(*userVisibleName == NULL);

	// Look up the parent service, and the parent of that.
	
	controller = IO_OBJECT_NULL;
	bus = IO_OBJECT_NULL;
	bsdName = NULL;
	controllerDict = NULL;

    err = IORegistryEntryGetParentEntry(interface, kIOServicePlane, &controller);
    if (err == 0) {
        err = IORegistryEntryGetParentEntry(controller, kIOServicePlane, &bus);
    }
    
    // Establish dictionary for the controller and bsdName for the device.
    
    if (err == 0) {
	    err = IORegistryEntryCreateCFProperties(controller, &controllerDict, NULL, kNilOptions );
    }
    if (err == 0) {
		bsdName = (CFStringRef) IORegistryEntryCreateCFProperty(interface, CFSTR(kIOBSDNameKey), NULL, kNilOptions);
		if (bsdName == NULL) {
			err = -1;
		}
	}
    
    // Now handle all of the wacky special cases.

	if (err == 0) {
		CFStringRef tmpStr;

		// Set userVisibleName to a reference to the port's user-visible name, 
		// handling all of the special cases.
		
        if ( CFEqualString(bsdName, CFSTR("en0")) ) {
            // built-in ethernet
            
			junk = CFQDictionarySetNumber(interfaceInfo, kSortOrderKey, kBuiltInEthernetSortOrder);
			assert(junk == 0);

	        *userVisibleName = (CFStringRef) CFQRetain( kMoreSCPortLabelEthernetBuiltIn );
        } else if ( CFEqualString(bsdName, CFSTR("fw0")) ) {
            // built-in FireWire
            
			junk = CFQDictionarySetNumber(interfaceInfo, kSortOrderKey, kBuiltInFireWireSortOrder);
			assert(junk == 0);

	        *userVisibleName = (CFStringRef) CFQRetain( kMoreSCPortLabelFireWireBuiltIn );
        } else if ( IOObjectConformsTo(interface, "IO80211Interface")
                    || (  CFDictionaryGetValueIfPresent(controllerDict, CFSTR(kIOClassKey), (const void **) &tmpStr)
	                   && CFStringHasPrefix(tmpStr, CFSTR("AirPort"))
                       )
                  ) {
            // airport

			CFDictionarySetValue(interfaceInfo, kSCPropNetInterfaceHardware, kSCEntNetAirPort);
			
			junk = CFQDictionarySetNumber(interfaceInfo, kSortOrderKey, kAirPortSortOrder);
			assert(junk == 0);

	        *userVisibleName = (CFStringRef) CFQRetain( kMoreSCPortLabelAirPort );
        } else {
            CFStringRef 			nameProperty;
            CFMutableDictionaryRef	busDict;

			// Some other type of Ethernet-like device.

			busDict      = NULL;
			nameProperty = NULL;
			
			err = IORegistryEntryCreateCFProperties(bus, &busDict, NULL, kNilOptions);
			if (err == 0) {
				CFTypeRef nameProp;
				
				nameProp = (CFTypeRef) CFDictionaryGetValue(busDict, CFSTR("name"));
				if (nameProp == NULL) {
					nameProperty = NULL;
				} else {
					nameProperty = CreateStringFromProperty(nameProp);
					if (nameProperty == NULL) {
						err = -1;
					}
				}
			}

			if (err == 0) {
	            if ( (nameProperty != NULL) && (CFEqualString(nameProperty, CFSTR("radio"))) ) {
	                // non-airport wireless (assuming someone writes a driver)

					junk = CFQDictionarySetNumber(interfaceInfo, kSortOrderKey, kWirelessSortOrder);
					assert(junk == 0);

			        *userVisibleName = (CFStringRef) CFQRetain( kMoreSCPortLabelWireless );
	            } else if ( CFDictionaryGetValueIfPresent(controllerDict, CFSTR(kIOProviderClassKey), (const void **) &tmpStr)
	                    && CFEqualString(tmpStr, CFSTR("IOPCIDevice"))) {
					CFTypeRef	slotProp;
					CFStringRef slotName;
	                CFNumberRef portNum;
	                SInt32      junkNum;

					// It's on a PCI bus.

					junk = CFQDictionarySetNumber(interfaceInfo, kSortOrderKey, kEthernetPCISortOrder);
					assert(junk == 0);
					
					// Get "AAPL,slot-name" property from I/O Registry and copy it into 
					// our interfaceInfo dictionary.  But first, mutate the slot name 
					// into a user-visible name version.
					
					slotName = NULL;
					slotProp = (CFTypeRef) IORegistryEntryCreateCFProperty(bus, CFSTR("AAPL,slot-name"), NULL, kNilOptions);
					if (slotProp != NULL) {
						slotName = CreateStringFromProperty(slotProp);
					}
					if (slotName != NULL) {
						CFStringRef tmp;
						
						tmp = CopyUserVisibleSlotName(slotName);
						if (tmp != NULL) {
							CFQRelease(slotName);
							slotName = tmp;
						}
					}
					if (slotName == NULL) {
						CFDictionarySetValue(interfaceInfo, CFSTR("AAPL,slot-name"), CFSTR("unknown"));
					} else {
						CFDictionarySetValue(interfaceInfo, CFSTR("AAPL,slot-name"), slotName);
					}
					CFQRelease(slotName);
					
					// Now get the IOChildIndex property to decide whether it's a single 
					// or multi-port Ethernet card.

					portNum = (CFNumberRef) CFDictionaryGetValue(busDict, CFSTR("IOChildIndex"));
					
	                if ( (portNum == NULL) || ! CFNumberGetValue(portNum, kCFNumberSInt32Type, &junkNum) ) {
	                    // single-port pci enet card

	                	// Either there is no IOChildIndex property, or it doesn't contain a 
	                	// valid number.

				        *userVisibleName = (CFStringRef) CFQRetain( kMoreSCPortLabelEthernetPCI );
	                } else {
	                	CFNumberRef portNumRef;
	                	SInt32		portNum;
	                	CFStringRef portNumString;
	                	
	                    // multi-port pci enet card

						// Get "IOChildIndex" property from I/O Registry and copy it into 
						// our interfaceInfo dictionary.
						
						portNumString = NULL;
						portNumRef = (CFNumberRef) IORegistryEntryCreateCFProperty(bus, CFSTR("IOChildIndex"), NULL, kNilOptions);
				        if ( (portNumRef != NULL) && CFNumberGetValue(portNumRef, kCFNumberSInt32Type, &portNum) ) {
				            portNumString = CFStringCreateWithFormat(NULL, NULL, CFSTR("%ld"), portNum + 1);
				        }
						if (portNumString == NULL) {
							CFDictionarySetValue(interfaceInfo, CFSTR("IOChildIndex"), CFSTR("unknown"));
						} else {
							CFDictionarySetValue(interfaceInfo, CFSTR("IOChildIndex"), portNumString);
						}
				        CFQRelease(portNumString);
				        CFQRelease(portNumRef);

				        *userVisibleName = (CFStringRef) CFQRetain( kMoreSCPortLabelEthernetPCIMultiport );
	                }
	            } else {
					CFStringRef			bsdName;

	                // unknown enet device - use a generic name and include its BSD name, 
	                // since we don't know what else to do

					bsdName = (CFStringRef) IORegistryEntryCreateCFProperty(interface, CFSTR(kIOBSDNameKey), NULL, kNilOptions);
					if (bsdName == NULL) {
						CFDictionarySetValue(interfaceInfo, CFSTR(kIOBSDNameKey), CFSTR("unknown"));
					} else {
						CFDictionarySetValue(interfaceInfo, CFSTR(kIOBSDNameKey), bsdName);
					}
					CFQRelease(bsdName);

			        *userVisibleName = (CFStringRef) CFQRetain( kMoreSCPortLabelEthernet );
	            }
			}

            CFQRelease(nameProperty);
            CFQRelease(busDict);
        }
	}
    
	// Clean up.
	
	junk = IOObjectRelease(bus);
	assert(junk == 0);
	
	junk = IOObjectRelease(controller);
	assert(junk == 0);

	CFQRelease(controllerDict);
	CFQRelease(bsdName);
	
	assert( (err == 0) == (*userVisibleName != NULL) );
	
	return err;
}

static kern_return_t CopyUserVisibleNameForModemOrSerialPort(io_object_t interface, 
															 CFMutableDictionaryRef interfaceInfo,
															 CFStringRef *userVisibleName)
	// Given a serial device (interface) and a dictionary of 
	// information about the device (interfaceInfo), guess at the 
	// user-visible name for the device.
	// 
	// I'm somewhat unhappy with this code (both its length and its style) 
	// but there's no point fixing it now that Apple provides a proper 
    // API for this (and thus this code only runs on old systems).
{
	kern_return_t 	err;
	kern_return_t 	junk;
	CFStringRef   	serialType;
	Boolean			isModem;
	CFStringRef		baseName;
	CFStringRef		productName;
	
	assert(interface        != IO_OBJECT_NULL);
	assert(interfaceInfo    != NULL);
	assert( userVisibleName != NULL);
	assert(*userVisibleName == NULL);

    isModem = false;                // quieten warning

	serialType = NULL;
	baseName = NULL;
	productName = NULL;
	
	// Determine whether the device is a modem.
	
	err = 0;
	serialType = (CFStringRef) IORegistryEntryCreateCFProperty(interface, CFSTR(kIOSerialBSDTypeKey), NULL, kNilOptions);
	if (serialType == NULL) {
		err = -1;
	}
	if (err == 0) {
		isModem = CFEqual(serialType, CFSTR(kIOSerialBSDModemType));
		if (isModem) {
			junk = CFQDictionarySetNumber(interfaceInfo, kSortOrderKey, kModemSortOrder);
			assert(junk == 0);
		}
	}

	// Get the "IOTTYBaseName" property and derive the user-visible name from that.
	
	if (err == 0) {
		baseName = (CFStringRef) IORegistryEntryCreateCFProperty(interface, CFSTR(kIOTTYBaseNameKey), NULL, kNilOptions);
		if (baseName == NULL) {
			err = -1;
		}
	}
	if (err == noErr) {
		// See whether the device has a "Product Name" property anywhere in its 
		// parent chain.

		productName = (CFStringRef) IORegistryEntrySearchCFProperty(interface, kIOServicePlane,
                            						  CFSTR(kIOPropertyProductNameKey), 
                            						  NULL, 
                            						  kIORegistryIterateRecursively | kIORegistryIterateParents);
	}
	if (err == 0) {
	    if ( isModem && CFEqualString(baseName, CFSTR("modem")) ) {

			junk = CFQDictionarySetNumber(interfaceInfo, kSortOrderKey, kInternalModemSortOrder);
			assert(junk == 0);
	    	
	        *userVisibleName = kMoreSCPortLabelModemInternal;
		    CFRetain(*userVisibleName);
	    } else if ( CFEqualString(baseName, CFSTR("modem"))) {
	        *userVisibleName = kMoreSCPortLabelModemPort;
		    CFRetain(*userVisibleName);
	    } else if ( CFEqualString(baseName, CFSTR("printer"))) {
	        *userVisibleName = kMoreSCPortLabelPrinterPort;
		    CFRetain(*userVisibleName);
	    } else if ( CFEqualString(baseName, CFSTR("modem-printer"))) {
	        *userVisibleName = kMoreSCPortLabelModemPrinterPort;
		    CFRetain(*userVisibleName);
	    } else if ( CFEqualString(baseName, CFSTR("IrDA-IrCOMM"))) {
	        *userVisibleName = kMoreSCPortLabelModemIrDA;
		    CFRetain(*userVisibleName);
	    } else if ( CFStringHasPrefix(baseName, CFSTR("Bluetooth-"))) {
	        *userVisibleName = kMoreSCPortLabelModemBluetooth;
		    CFRetain(*userVisibleName);
	    } else if ( CFEqualString(baseName, CFSTR("usbmodem")) || (productName != NULL) ) {

			// It may not be a modem (it may just be a USB serial device), so 
			// we only override the default sort order (serial) if the we 
			// know it's a modem.
			
			if (isModem) {
				junk = CFQDictionarySetNumber(interfaceInfo, kSortOrderKey, kUSBModemSortOrder);
				assert(junk == 0);
			}
			
			if (productName != NULL) {
				// Serial or modem USB device with "Product Name" key.
				*userVisibleName = productName;
			} else {
				// productName is NULL so baseName must be "usbmodem".
		        *userVisibleName = kMoreSCPortLabelModemUSB;
			}
		    CFRetain(*userVisibleName);
			
	    } else {
			CFStringRef 		baseNameProperty;
			CFMutableStringRef	ttyBase;
			
			ttyBase = NULL;
			baseNameProperty = (CFStringRef) IORegistryEntryCreateCFProperty(interface, CFSTR(kIOTTYBaseNameKey), NULL, kNilOptions);
			if (baseNameProperty != NULL) {
				ttyBase = CFStringCreateMutableCopy(NULL, 0, baseNameProperty);
			}
			if (ttyBase == NULL) {
				CFDictionarySetValue(interfaceInfo, CFSTR(kIOTTYBaseNameKey), CFSTR("unknown"));
			} else {
				CFStringLowercase(ttyBase, NULL);
				CFDictionarySetValue(interfaceInfo, CFSTR(kIOTTYBaseNameKey), ttyBase);
			}
			CFQRelease(ttyBase);
			CFQRelease(baseNameProperty);

	    	// This string is pretty unlikely, but it does match what the 
	    	// Network preferences panel creates in Mac OS X 10.1.x for 
	    	// unknown devices, such as a Keyspan serial adapter.

	    	*userVisibleName = kMoreSCPortLabelSerial;
		    CFRetain(*userVisibleName);
	    }
	}

	CFQRelease(serialType);
	CFQRelease(baseName);
	CFQRelease(productName);
	
	assert( (err == 0) == (*userVisibleName != NULL) );
	
	return err;
}

static kern_return_t CopyUserVisiblePortName(io_object_t interface, 
											 CFMutableDictionaryRef interfaceInfo, 
											 CFStringRef *userVisibleName)
	// Given an IOKit device (interface) and a dictionary of 
	// information about the device (interfaceInfo), guess at the 
	// user-visible name for the device.  The basic algorithm is 
	// as follows:
	// 
	// 1. Decide on a proposed user-visible name, using heuristics 
	//    that are based on the device type.  While doing this add 
	//    keyword substition values to interfaceInfo.
	//
	// 2. Call the client's callback to see whether it wants to 
	//    modify our decision.
	//
	// 3. Finally, replace certain keywords in the string with 
	//    values from the interfaceInfo dictionary.
	//
	// This design allows the client callback to see a limited number 
	// of strings, and hence makes it easy for the client to localise 
	// those strings.  It also means we only have one set of I/O Registry 
	// parsing code (in step 1), rather than messing with IOKit once in 
	// step 1 to decide on the device type and again in step 3 when 
	// doing the keyword replacement.
{
	kern_return_t err;
	CFStringRef   proposedName;
	CFStringRef   baseName;
	
	assert(interface        != IO_OBJECT_NULL);
	assert(interfaceInfo    != NULL);
	assert( userVisibleName != NULL);
	assert(*userVisibleName == NULL);
	
	proposedName = NULL;
	baseName = NULL;
	
	// First we synthesise our candidate user-visible name.
	
	if ( IOObjectConformsTo(interface, kIONetworkInterfaceClass) ) {
		err = CopyUserVisibleNameForEthernetLikePort(interface, interfaceInfo, &proposedName);
	} else if ( IOObjectConformsTo(interface, kIOSerialBSDServiceValue) ) {
		err = CopyUserVisibleNameForModemOrSerialPort(interface, interfaceInfo, &proposedName);
	} else {
		err = 0;
		proposedName = (CFStringRef) IORegistryEntryCreateCFProperty(interface, CFSTR(kIOBSDNameKey), NULL, kNilOptions);
		if (proposedName == NULL) {
			err = -1;
		}
	}
	
	// Then we ask our client to munge it as it likes.

	if (gPortNameCallback != NULL) {
		err = gPortNameCallback(interface, interfaceInfo, proposedName, &baseName);
	} else {
		// The debug build complains if you don't install a callback because, 
		// hey, localisability is important.
		
		#if !defined(NDEBUG)
			{
				static Boolean gHavePrinted;
				
				if ( !gHavePrinted ) {
					fprintf(stderr, "MoreSCFPortScanner.c: You should install a port name callback.\n");
					gHavePrinted = true;
				}
			}
		#endif
	}
	
	// If we didn't get a baseName, just use proposedName.
	
	if ( (err == 0) && (baseName == NULL) ) {
		baseName = CFStringCreateCopy(NULL, proposedName);
		if (baseName == NULL) {
			err = -1;
		}
	}
	
	// Then we do keyword substitution (as specified in the header file) on the result.
	
	if (err == 0) {
		err = SubstituteKeywordsInUserVisibleName(interface, interfaceInfo, baseName, userVisibleName);
	}
	
	CFQRelease(proposedName);
	CFQRelease(baseName);
	
	assert( (err == 0) == (*userVisibleName != NULL) );
	
	return err;
}

extern void MoreSCFSetPortNameCallback(MoreSCFPortNameCallback callback)
	// See comment in header.
{
	gPortNameCallback = callback;
}

static kern_return_t EthernetDeviceToDictProc(void *contextPtr, io_object_t interface, 
											  CFMutableDictionaryRef interfaceInfo)
	// This routine is called (via function pointer) by AddMatchingDevicesToArray 
	// to add Ethernet-specific information for the Ethernet-like device (which 
	// includes AirPort) specified by interface to the interfaceInfo dictionary.
{
	#pragma unused(contextPtr)
	kern_return_t	err;
	kern_return_t	junk;
    int             type;
    CFNumberRef     typeNum;
	CFStringRef		bsdName;
	io_object_t		controller;

	assert(interface        != IO_OBJECT_NULL);
	assert(interfaceInfo    != NULL);
	
	controller = IO_OBJECT_NULL;
	typeNum    = NULL;
	bsdName    = NULL;
	
    err = 0;
    
    // First do stuff that's interface type specific.
    
    typeNum = (CFNumberRef) IORegistryEntryCreateCFProperty(interface, CFSTR(kIOInterfaceType), NULL, kNilOptions);
    if ( (typeNum == NULL) || (CFGetTypeID(typeNum) != CFNumberGetTypeID()) || ! CFNumberGetValue(typeNum, kCFNumberIntType, &type) ) {
        type = IFT_OTHER;           // if anything goes wrong, we ignore this interface
    }

    switch (type) {
        case IFT_ETHER:
            CFDictionarySetValue(interfaceInfo, kSCPropNetInterfaceHardware, kSCEntNetEthernet);
            CFDictionarySetValue(interfaceInfo, kSCPropNetInterfaceType,     kSCValNetInterfaceTypeEthernet);

            junk = CFQDictionarySetNumber(interfaceInfo, kSortOrderKey, kEthernetSortOrder);
            assert(junk == 0);
            break;
        case IFT_IEEE1394:
            CFDictionarySetValue(interfaceInfo, kSCPropNetInterfaceHardware, kSCEntNetFireWire);
			CFDictionarySetValue(interfaceInfo, kSCPropNetInterfaceType,     kSCValNetInterfaceTypeFireWire);

            junk = CFQDictionarySetNumber(interfaceInfo, kSortOrderKey, kFireWireSortOrder);
            assert(junk == 0);
            break;
        default:
            // do nothing
            break;
    }
    
	// If that succeeds, do the common stuff.  If it fails, we return an empty dictionary 
    // which causes our caller to skip this interface.
    
    if ( CFDictionaryGetCount(interfaceInfo) > 0 ) {

        // kSCPropNetInterfaceDeviceName

        err = 0;
        bsdName = (CFStringRef) IORegistryEntryCreateCFProperty(interface, CFSTR(kIOBSDNameKey), NULL, kNilOptions);
        if (bsdName == NULL) {
            err = -1;
        }
        if (err == 0) {
            CFDictionarySetValue(interfaceInfo, kSCPropNetInterfaceDeviceName, bsdName);
        }

        // kSCPropNetInterfaceHardware
        // kSCPropNetInterfaceType
        
        // done above
        
        // The kSCPropNetInterfaceHardware value is overwritten (set to kSCEntNetAirPort) by the 
        // user-visible name code when it determines that the card is really an AirPort card.

        // kSCPropNetInterfaceSubType
        
        // No sub-type for Ethernet interfaces.
        
        // kSCPropMACAddress

        if (err == 0) {	
            err = IORegistryEntryGetParentEntry(interface, kIOServicePlane, &controller);
        }
        if (err == 0) {
            CFDataRef macData;
            CFStringRef macStr;
            
            macStr = NULL;
            macData = NULL;
            
            macData = (CFDataRef) IORegistryEntryCreateCFProperty(controller, CFSTR(kIOMACAddress), NULL, kNilOptions);
            if (macData != NULL) {
                macStr = CreateMACFromData(macData);
                if (macStr != NULL) {
                    CFDictionarySetValue(interfaceInfo, kSCPropMACAddress, macStr);
                }
            }
            
            CFQRelease(macData);
            CFQRelease(macStr);
        }
        
        // kSCPropUserDefinedName set up by caller.
    }

	// Clean up.

	junk = IOObjectRelease(controller);
	assert(junk == 0);

	CFQRelease(bsdName);
    CFQRelease(typeNum);
	
	return err;
}

static kern_return_t ModemOrSerialDeviceToDictProc(void *contextPtr, 
												   io_object_t interface, 
												   CFMutableDictionaryRef interfaceInfo)
	// This routine is called (via function pointer) by AddMatchingDevicesToArray 
	// to add modem/serial-specific information for the modem/serial-like device 
	// (which includes internal modems, built-in serial ports, USB serial adapters, 
	// USB modems, and IrDA) specified by interface to the interfaceInfo dictionary.
{
	#pragma unused(contextPtr)
	kern_return_t 			err;
	kern_return_t 			junk;
    CFTypeRef               hiddenVal;
	CFMutableDictionaryRef	interfaceDict;
	CFStringRef				baseName;
	CFNumberRef				supportsHold;
	
	assert(interface        != IO_OBJECT_NULL);
	assert(interfaceInfo    != NULL);

	interfaceDict = NULL;
	supportsHold  = false;

    err = noErr;
	
    // Don't show ports with the "HiddenPort" property in the port or any of 
    // its ancestors.
    
	hiddenVal = IORegistryEntrySearchCFProperty(
        interface,
        kIOServicePlane,
        CFSTR("HiddenPort"),
        NULL,
        kIORegistryIterateRecursively | kIORegistryIterateParents
    );
    if (hiddenVal == NULL) {
        // Create a dictionary of the properties for the port.
        
        err = IORegistryEntryCreateCFProperties(interface, &interfaceDict, NULL, kNilOptions );
     
        // Get IOTTYBaseName

        // Yetch.  We specifically exclude ports named "irda" because otherwise the IrDA 
        // ports on the original iMac (rev's A through D) show up as serial ports.  Given 
        // that only the rev A actually had an IrDA port, and Mac OS X doesn't even support 
        // it, these ports definitely shouldn't be listed.
            
        if (err == 0 
                && CFDictionaryGetValueIfPresent(interfaceDict, CFSTR(kIOTTYBaseNameKey), (const void **) &baseName )
                && ! CFEqual(baseName, CFSTR("irda")) ) {
            junk = CFQDictionarySetNumber(interfaceInfo, kSortOrderKey, kSerialSortOrder);
            assert(junk == 0);
        
            // kSCPropNetInterfaceDeviceName

            CFDictionarySetValue(interfaceInfo, kSCPropNetInterfaceDeviceName, CFDictionaryGetValue(interfaceDict, CFSTR(kIOTTYDeviceKey)));
            
            // kSCPropNetInterfaceHardware
            
            CFDictionarySetValue(interfaceInfo, kSCPropNetInterfaceHardware, kSCEntNetModem);

            // kSCPropNetInterfaceType
            
            CFDictionarySetValue(interfaceInfo, kSCPropNetInterfaceType, kSCValNetInterfaceTypePPP);

            // kSCPropNetInterfaceSubType
            
            CFDictionarySetValue(interfaceInfo, kSCPropNetInterfaceSubType, kSCValNetInterfaceSubTypePPPSerial);

            // "HardwareVariant"
            
            // A special hack for IrDA, modelled directly on the code from the 
            // control panel.
            
            if ( CFStringHasPrefix(baseName, kMoreSCValNetInterfaceHardwareVariantIrDACOMM) ) {
                junk = CFQDictionarySetNumber(interfaceInfo, kSortOrderKey, kIrDASerialSortOrder);
                assert(junk == 0);
                
                CFDictionarySetValue(interfaceInfo, kMoreSCPropNetInterfaceHardwareVariant, kMoreSCValNetInterfaceHardwareVariantIrDACOMM);
            } else if ( CFStringHasPrefix(baseName, kMoreSCValNetInterfaceHardwareVariantBluetooth) ) {
                junk = CFQDictionarySetNumber(interfaceInfo, kSortOrderKey, kBluetoothSortOrder);
                assert(junk == 0);
                
                CFDictionarySetValue(interfaceInfo, kMoreSCPropNetInterfaceHardwareVariant, kMoreSCValNetInterfaceHardwareVariantBluetooth);
            }
            
            // kSCPropNetInterfaceSupportsModemOnHold
            
            supportsHold = (CFNumberRef) IORegistryEntrySearchCFProperty(interface, kIOServicePlane,
                                                          CFSTR("V92Modem"), 
                                                          NULL, 
                                                          kIORegistryIterateRecursively | kIORegistryIterateParents);
            if (supportsHold != NULL) {
                assert( CFGetTypeID(supportsHold) == CFNumberGetTypeID() );
                CFDictionarySetValue(interfaceInfo, kSCPropNetInterfaceSupportsModemOnHold, supportsHold);
            }

            // kSCPropUserDefinedName set up by caller.
        }
    }
    
    CFQRelease(interfaceDict);
   	CFQRelease(supportsHold);
    CFQRelease(hiddenVal);
    
    return err;
}

typedef kern_return_t (*DeviceToDictProc)(void *contextPtr, io_object_t interface, CFMutableDictionaryRef interfaceInfo);

static kern_return_t AddMatchingDevicesToArray(mach_port_t 			masterPort, 
											   CFDictionaryRef		deviceMatchDict,
											   DeviceToDictProc 	deviceToDict,
											   void *				contextPtr,
											   CFMutableArrayRef 	result)
	// Given a connection to IOKit (masterPort) and a device matching 
	// dictionary (deviceMatchDict), this routine searches the I/O Registry 
	// for all matching devices and calls the deviceToDict routine to 
	// add information about those devices to result array.
	//
	// IMPORTANT: In line with IOKit conventions this routine releases deviceMatchDict.
	//
	// Note that this routine has a contextPtr parameter which it passes 
	// through to the deviceToDict proc, even though none of the existing 
	// deviceToDict procs need a context.  Originally the modem/serial 
	// callback needed this context however, in one of the various 
	// iterations, this requirement disappeared.  I left the contextPtr 
	// in the code because, hey, its always a good idea for callbacks 
	// to have context.
{
	kern_return_t 	err;
	kern_return_t 	junk;
	io_iterator_t 	iterator;
   	io_object_t   	interface;

//	assert(masterPort        != 0  );           // now that we're using kIOMasterPortDefault, 0 is OK
	assert(deviceMatchDict   != NULL);
	assert(deviceToDict      != NULL);
	assert(result            != NULL);
	
	iterator = IO_OBJECT_NULL;
	
    err = IOServiceGetMatchingServices(masterPort, deviceMatchDict, &iterator);
	if (err == 0) {
	    do {
			interface = IOIteratorNext(iterator);
			if (interface != IO_OBJECT_NULL) {
				CFMutableDictionaryRef	interfaceInfo;
			   	CFStringRef				userVisibleName;

				interfaceInfo = NULL;
				userVisibleName = NULL;

				err = CFQDictionaryCreateMutable(&interfaceInfo);
				if (err == 0) {
					err = deviceToDict(contextPtr, interface, interfaceInfo);
				}
				
				// If the deviceToDict proc returns an empty dictionary 
				// and no error, we just ignore this device.
				
		    	if ( (err == 0) && (CFDictionaryGetCount(interfaceInfo) > 0) ) {
					err = CopyUserVisiblePortName(interface, interfaceInfo, &userVisibleName);
					if (err == 0) {
						CFDictionarySetValue(interfaceInfo, kSCPropUserDefinedName, userVisibleName);
				        CFArrayAppendValue(result, interfaceInfo);
					}
		    	}
		    	
				junk = IOObjectRelease(interface);
		    	assert(junk == 0);
		    	
	    		CFQRelease(interfaceInfo);
			    CFQRelease(userVisibleName);
			}
	    } while (interface != IO_OBJECT_NULL && err == 0);
	}
	    
    if (iterator != IO_OBJECT_NULL) {
    	junk = IOObjectRelease(iterator);
    	assert(junk == 0);
    }
    
    return err;
}

// I declare this as extern so that I can call it from the test program. 
// However, I don't want it in the header because I don't want folks 
// calling it in general.

extern OSStatus MoreSCCreatePortArrayOld(CFArrayRef *portArray);

extern OSStatus MoreSCCreatePortArrayOld(CFArrayRef *portArray)
    // Implementation of MoreSCCreatePortArray when SCNetworkInterfaceCopyAll 
    // is not available.  See MoreSCCreatePortArray header comments for a 
    // description of the parameters.
{
    kern_return_t		err;
    CFMutableArrayRef	result;
	CFStringRef 		keys[2];
	CFStringRef 		values[2];

	assert( portArray != NULL);
	assert(*portArray == NULL);
	
	result = NULL;
	    
	err = CFQArrayCreateMutable(&result);
	
	// Ethernet-like devices (includes AirPort and FireWire)
	
    if (err == 0) {
		err = AddMatchingDevicesToArray(kIOMasterPortDefault, IOServiceMatching(kIONetworkInterfaceClass), 
										EthernetDeviceToDictProc, NULL, result);
    }
    
    // Set up common key/value pairs for the next two calls.
    
	  keys[0] = CFSTR(kIOProviderClassKey);
	values[0] = CFSTR(kIOSerialBSDServiceValue);
	  keys[1] = CFSTR(kIOSerialBSDTypeKey);

	// Modem devices
	
    if (err == 0) {
		values[1] = CFSTR(kIOSerialBSDModemType);

		err = AddMatchingDevicesToArray(kIOMasterPortDefault, CFDictionaryCreate(NULL, (const void **) keys, (const void **) values, 2, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks), 
										ModemOrSerialDeviceToDictProc, NULL, result);
    }
    
    // Serial devices (includes IrDA)
    
    if (err == 0) {
		values[1] = CFSTR(kIOSerialBSDRS232Type);

		err = AddMatchingDevicesToArray(kIOMasterPortDefault, CFDictionaryCreate(NULL, (const void **) keys, (const void **) values, 2, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks), 
										ModemOrSerialDeviceToDictProc, NULL, result);
    }
    
    // Sort the list.
    
    if (err == noErr) {
	    CFArraySortValues(result, CFRangeMake( 0, CFArrayGetCount(result)), PortSorter, NULL);
	}
    
    // Clean up.

	if (err != 0) {
		CFQRelease(result);
		result = NULL;
	}
	*portArray = result;

	assert( (err == noErr) == (*portArray != NULL) );
	
	// This is cheesy.  We cast the kern_return_t directly to an OSStatus.
	
	return err;    
}

/////////////////////////////////////////////////////////////////
#pragma mark ***** New Implementation

static Boolean ModemInterfaceSupportsModemOnHold(SCNetworkInterfaceRef interface)
{
    Boolean         result;
    kern_return_t   junkKern;
    CFStringRef     bsdName;
    CFDictionaryRef propertyDict;
    CFDictionaryRef matchDict;
    io_service_t    service;
    CFNumberRef     onHoldRef;
    int             onHold;
    
    assert(interface != NULL);
    assert(CFEqual(SCNetworkInterfaceGetInterfaceType(interface), kSCNetworkInterfaceTypeModem));
    
    // Prepare for failure.
    
    result       = false;
    propertyDict = NULL;
    matchDict    = NULL;
    service      = IO_OBJECT_NULL;
    onHoldRef    = NULL;
    
    // Get the BSD name from the interface (for the built-in modem this is 
    // typically "modem") and construct a matching dictionary for services whose 
    // "IOTTYBaseName" property matches that.
    
    bsdName = SCNetworkInterfaceGetBSDName(interface);
    if (bsdName != NULL) {
        CFStringRef propertyKeys[1];
        CFStringRef propertyValues[1];
        
          propertyKeys[0] = CFSTR(kIOTTYBaseNameKey);
        propertyValues[0] = bsdName;

        propertyDict = CFDictionaryCreate(NULL, (const void **) propertyKeys, (const void **) propertyValues, 1, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
        assert(propertyDict != NULL);
    }
    if (propertyDict != NULL) {
        CFStringRef     matchKeys[1];
        CFDictionaryRef matchValues[1];

          matchKeys[0] = CFSTR(kIOPropertyMatchKey);
        matchValues[0] = propertyDict;
        
        matchDict = CFDictionaryCreate(NULL, (const void **) matchKeys, (const void **) matchValues, 1, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
        assert(matchDict != NULL);
    }
    
    // Get the first service that matches.
    
    if (matchDict != NULL) {
        service = IOServiceGetMatchingService(kIOMasterPortDefault, matchDict);
        assert(service != IO_OBJECT_NULL);      // If we don't find any service, that's very weird.
        
        matchDict = NULL;                       // IOServiceGetMatchingService releases it for us
    }
    
    // Once we've found the service, look up the value of the "V92Modem" property 
    // and, if it's non-zero, we've support V.92.

    if (service != IO_OBJECT_NULL) {
        onHoldRef = (CFNumberRef) IORegistryEntrySearchCFProperty(
            service, 
            kIOServicePlane,
            CFSTR("V92Modem"),
            NULL, 
            kIORegistryIterateRecursively | kIORegistryIterateParents
        );
    }
    if (onHoldRef != NULL) {
        if (CFGetTypeID(onHoldRef) == CFNumberGetTypeID()) {
            if ( CFNumberGetValue(onHoldRef, kCFNumberIntType, &onHold) ) {
                if (onHold != 0) {
                    assert(onHold == 1);
                    result = true;
                }
            } else {
                assert(false);
            }
        } else {
            assert(false);
        }
    }
    
    // Clean up.
    
    CFQRelease(onHoldRef);
    CFQRelease(propertyDict);
    CFQRelease(matchDict);
    if (service != IO_OBJECT_NULL) {
        junkKern = IOObjectRelease(service);
        assert(junkKern == KERN_SUCCESS);
    }
    
    return result;
}

static OSStatus AirPortFixup(CFMutableDictionaryRef interfaceDict, Boolean *resortPtr)
    // Mac OS X 10.4.2 through 10.4.7 have a bug that causes SCNetworkInterfaceCopyAll 
    // to return an AirPort interface as an Ethernet interface.  This happens on both 
    // PowerPC and Intel.  I fix this by looking up any Ethernet interface in the 
    // I/O registry to see if it's controller looks like an AirPort driver and, 
    // if so, I set kSCPropNetInterfaceHardware to kSCEntNetAirPort.  I also have 
    // to set up the correct user-visible name.
    //
    // Also, if I change anything I set *resortPtr to true to force a resort of 
    // the interface array; see AirPortFixupResort for the details.
{
    OSStatus            err;
    kern_return_t       kr;
    kern_return_t       junkKern;
    char *              bsdNameCStr;
    io_service_t        interface;
    io_service_t        controller;

    assert(interfaceDict != NULL);
    assert( CFDictionaryGetValue(interfaceDict, kSCPropNetInterfaceType) != NULL );
    assert( CFEqual(CFDictionaryGetValue(interfaceDict, kSCPropNetInterfaceType), kSCValNetInterfaceTypeEthernet) );
    assert( CFDictionaryGetValue(interfaceDict, kSCPropNetInterfaceDeviceName) != NULL );
    assert(resortPtr != NULL);
    
    bsdNameCStr = NULL;
    interface = IO_OBJECT_NULL;
    controller = IO_OBJECT_NULL;
    
    // Use the BSD name to find the IONetworkInterface in the I/O registry.
    
    err = CFQStringCopyCString( CFDictionaryGetValue(interfaceDict, kSCPropNetInterfaceDeviceName), kCFStringEncodingUTF8, &bsdNameCStr);
    if (err == 0) {
        interface = IOServiceGetMatchingService(kIOMasterPortDefault, IOBSDNameMatching(kIOMasterPortDefault, 0, bsdNameCStr));
        if (interface == IO_OBJECT_NULL) {
            assert(false);                      // this is /very/ weird
            err = paramErr;
        } else {
            assert( IOObjectConformsTo(interface, "IONetworkInterface") );
        }
    }

    // Then find the parent controller object.
    
    if (err == noErr) {
		kr = IORegistryEntryGetParentEntry(interface, kIOServicePlane, &controller);
        if (kr != KERN_SUCCESS) {
            err = paramErr;
        }
    }
    
    // If it looks like an AirPort controller, change the interface dictionary 
    // appropriately.  This is the same test as used by System Configuration 
    // framework in 10.4.7.

    if (err == noErr) {
        if (    IOObjectConformsTo(controller, "IO80211Controller")
             || IOObjectConformsTo(controller, "AirPortPCI"       )
             || IOObjectConformsTo(controller, "AirPortDriver"    ) ) {
            CFBundleRef     bundle;
            CFStringRef     str;
            
            CFDictionarySetValue(interfaceDict, kSCPropNetInterfaceHardware, kSCEntNetAirPort);
            CFDictionarySetValue(interfaceDict, kSCPropUserDefinedName,      CFSTR("AirPort"));
            
            // Override the user-visible name with a value from the System Configuration 
            // framweork on disk.  This introduces a serious binary compatibility risk. 
            // The only reason it's acceptable in this case is that we know that this 
            // code will only run on 10.4.2 through 10.4.6, and those systems are 
            // unlikely to change at this point.
            
            str = NULL;
            
            bundle = CFBundleGetBundleWithIdentifier(CFSTR("com.apple.SystemConfiguration"));
            if (bundle != NULL) {
                str = CFBundleCopyLocalizedString(bundle, CFSTR("airport"), CFSTR("AirPort"), CFSTR("NetworkInterface"));
                if (str != NULL) {
                    CFDictionarySetValue(interfaceDict, kSCPropUserDefinedName, str);
                }
            }
            
            CFQRelease(str);

            *resortPtr = true;
        }
    }

    // Clean up.
    
    if (controller != IO_OBJECT_NULL) {
        junkKern = IOObjectRelease(controller);
        assert(junkKern == KERN_SUCCESS);
    }
    if (interface != IO_OBJECT_NULL) {
        junkKern = IOObjectRelease(interface);
        assert(junkKern == KERN_SUCCESS);
    }
    free(bsdNameCStr);
    
    return err;
}

static void AirPortFixupResort(CFMutableArrayRef result)
    // If AirPortFixup changes the properties of an interface, it breaks the sort 
    // order of the interface list.  For example, if you have Built-in Ethernet 
    // (en0), AirPort (en1), and Built-in FireWire (fw0), SCNetworkInterfaceCopyAll 
    // will mistaken the AirPort as an Ethernet interface and return the interfaces 
    // sorted as en0, en1, and fw0.  This is wrong, because the AirPort should go 
    // after the FireWire.  So, I have to resort the array.  This is hard because 
    // SCNetworkInterfaceCopyAll doesn't publish its sort keys.  Instead, 
    // I do some sneaky shuffling as described in the comments below.
{
    CFIndex         interfaceCount;
    CFIndex         interfaceIndex;
    CFIndex         lastEthernetOrFireWireInterfaceIndex;
    CFIndex         insertionPoint;
    CFStringRef     hardware;
    
    interfaceCount = CFArrayGetCount(result);
    
    // Go through the list and find the index of the last Ethernet/FireWire 
    // interface.
    
    lastEthernetOrFireWireInterfaceIndex = -1;
    for (interfaceIndex = 0; interfaceIndex < interfaceCount; interfaceIndex++) {
        hardware = CFDictionaryGetValue(CFArrayGetValueAtIndex(result, interfaceIndex), kSCPropNetInterfaceHardware);
        assert( (hardware != NULL) && (CFGetTypeID(hardware) == CFStringGetTypeID()) );

        if ( CFEqual(hardware, kSCEntNetEthernet) || CFEqual(hardware, kSCEntNetFireWire) ) {
            lastEthernetOrFireWireInterfaceIndex = interfaceIndex;
        }
    }
    
    // If there are no Ethernet/FireWire interfaces, give up now.  The 
    // AirPort interfaces are already in the right place.
    //
    // Otherwise, go through the list removing at AirPort interfaces 
    // before the last Ethernet/FireWire interface and inserting them 
    // after.
    
    if (lastEthernetOrFireWireInterfaceIndex != -1) {
        interfaceIndex = 0;
        insertionPoint = lastEthernetOrFireWireInterfaceIndex + 1;
        while (interfaceIndex < lastEthernetOrFireWireInterfaceIndex) {
            // Loop invariants:
            
            assert(CFArrayGetCount(result) == interfaceCount);
            assert( (lastEthernetOrFireWireInterfaceIndex >= 0) && (lastEthernetOrFireWireInterfaceIndex < interfaceCount) );
            assert( (insertionPoint > lastEthernetOrFireWireInterfaceIndex) && (insertionPoint <= interfaceCount) );

            assert( (interfaceIndex >= 0) && (interfaceIndex < interfaceCount) );                
            hardware = CFDictionaryGetValue(CFArrayGetValueAtIndex(result, interfaceIndex), kSCPropNetInterfaceHardware);
            assert( (hardware != NULL) && (CFGetTypeID(hardware) == CFStringGetTypeID()) );
            
            if ( CFEqual(hardware, kSCEntNetAirPort) ) {                
                CFArrayInsertValueAtIndex(result, insertionPoint, CFArrayGetValueAtIndex(result, interfaceIndex));
                assert( (interfaceIndex >= 0) && (interfaceIndex < (interfaceCount + 1)) );
                CFArrayRemoveValueAtIndex(result, interfaceIndex);

                // We've just moved an element from before lastEthernetOrFireWireInterfaceIndex 
                // to after it.  Thus, we decrement lastEthernetOrFireWireInterfaceIndex.
                
                lastEthernetOrFireWireInterfaceIndex -= 1;
                
                // We've just inserted an element at insertionPoint.  In order 
                // for our resort to be stable, we want to insert future 
                // elements after that.
                
                insertionPoint += 1;
            } else {
                interfaceIndex += 1;
            }
        }
    }
}

static OSStatus MoreSCCreatePortArrayNew(CFArrayRef *portArray)
    // Implementation of MoreSCCreatePortArray using SCNetworkInterfaceCopyAll.
    // See MoreSCCreatePortArray header comments for a description 
    // of the parameters.  This /must/ unimpErr if SCNetworkInterfaceCopyAll 
    // is not available.
{
    OSStatus            err;
    OSStatus            junk;
    CFArrayRef          interfaces;
    CFIndex             interfaceCount;
    CFIndex             interfaceIndex;
    CFMutableArrayRef   result;
    Boolean             needAirPortFixup;
    Boolean             resortAirPort;

    assert( portArray != nil);
    assert(*portArray == nil);

    result = NULL;
    interfaces = NULL;
    
    // Mac OS X 10.4.2 through 10.4.7 have a bug that causes SCNetworkInterfaceCopyAll 
    // to return an AirPort interface as an Ethernet interface.  We conditionally 
    // fix that up.
    
    needAirPortFixup = (MoreSCGetSystemVersion() >= 0x01042) && (MoreSCGetSystemVersion() < 0x01047);
    resortAirPort = false;

    if (SCNetworkInterfaceCopyAll != NULL) {
        interfaces = SCNetworkInterfaceCopyAll();
        err = MoreSCError(interfaces);
    } else {
        err = unimpErr;
    }

    if (err == noErr) {
        err = CFQArrayCreateMutable(&result);
    }

    if (err == noErr) {
        interfaceCount = CFArrayGetCount(interfaces);

        for (interfaceIndex = 0; interfaceIndex < interfaceCount; interfaceIndex++) {
            Boolean                 skipIt;
            SCNetworkInterfaceRef   thisInterface;
            CFMutableDictionaryRef  interfaceDict;
            CFStringRef             type;
            CFStringRef             userVisibleName;
            CFStringRef             bsdName;
            CFStringRef             macAddr;

            skipIt = false;
            interfaceDict = NULL;
            
            thisInterface = CFArrayGetValueAtIndex(interfaces, interfaceIndex);
            assert(thisInterface != NULL);
            
            err = CFQDictionaryCreateMutable(&interfaceDict);
            if (err == noErr) {
                type = SCNetworkInterfaceGetInterfaceType(thisInterface);
                if (type != NULL) {
                    if        (CFEqual(type, kSCNetworkInterfaceTypeBluetooth)) {
                        CFDictionarySetValue(interfaceDict, kSCPropNetInterfaceType    , kSCValNetInterfaceTypePPP);
                        CFDictionarySetValue(interfaceDict, kSCPropNetInterfaceSubType , kSCValNetInterfaceSubTypePPPSerial);
                        CFDictionarySetValue(interfaceDict, kSCPropNetInterfaceHardware, kSCEntNetModem);
                        CFDictionarySetValue(interfaceDict, kMoreSCPropNetInterfaceHardwareVariant, kMoreSCValNetInterfaceHardwareVariantBluetooth);
                    } else if (CFEqual(type, kSCNetworkInterfaceTypeEthernet)) {
                        CFDictionarySetValue(interfaceDict, kSCPropNetInterfaceType    , kSCValNetInterfaceTypeEthernet);
                        CFDictionarySetValue(interfaceDict, kSCPropNetInterfaceHardware, kSCEntNetEthernet);
                    } else if (CFEqual(type, kSCNetworkInterfaceTypeFireWire)) {
                        CFDictionarySetValue(interfaceDict, kSCPropNetInterfaceType    , kSCValNetInterfaceTypeFireWire);
                        CFDictionarySetValue(interfaceDict, kSCPropNetInterfaceHardware, kSCEntNetFireWire);
                    } else if (CFEqual(type, kSCNetworkInterfaceTypeIEEE80211)) {
                        CFDictionarySetValue(interfaceDict, kSCPropNetInterfaceType    , kSCValNetInterfaceTypeEthernet);
                        CFDictionarySetValue(interfaceDict, kSCPropNetInterfaceHardware, kSCEntNetAirPort);
                    } else if (CFEqual(type, kSCNetworkInterfaceTypeModem)) {
                        CFDictionarySetValue(interfaceDict, kSCPropNetInterfaceType    , kSCValNetInterfaceTypePPP);
                        CFDictionarySetValue(interfaceDict, kSCPropNetInterfaceSubType , kSCValNetInterfaceSubTypePPPSerial);
                        CFDictionarySetValue(interfaceDict, kSCPropNetInterfaceHardware, kSCEntNetModem);
                        if ( ModemInterfaceSupportsModemOnHold(thisInterface) ) {
                            junk = CFQDictionarySetNumber(interfaceDict, kSCPropNetInterfaceSupportsModemOnHold, 1);
                            assert(junk == noErr);
                        }
                    } else {
                        // fprintf(stderr, "skipping unrecognized interface\n"); CFShow(thisInterface);
                        skipIt = true;
                    }
                } else {
                    skipIt = true;
                }
            }

            if ( (err == noErr) && ! skipIt) {
                userVisibleName = SCNetworkInterfaceGetLocalizedDisplayName(thisInterface);
                if (userVisibleName != NULL) {
                    CFDictionarySetValue(interfaceDict, kSCPropUserDefinedName, userVisibleName);
                }

                bsdName = SCNetworkInterfaceGetBSDName(thisInterface);
                if (bsdName != NULL) {
                    CFDictionarySetValue(interfaceDict, kSCPropNetInterfaceDeviceName, bsdName);
                }
                
                macAddr = SCNetworkInterfaceGetHardwareAddressString(thisInterface);
                if (macAddr != NULL) {
                    CFDictionarySetValue(interfaceDict, kSCPropMACAddress, macAddr);
                }

                if ( CFEqual(type, kSCNetworkInterfaceTypeEthernet) && needAirPortFixup) {
                    junk = AirPortFixup(interfaceDict, &resortAirPort);
                    assert(junk == noErr);
                }
                
                CFArrayAppendValue(result, interfaceDict);
            }
            CFQRelease(interfaceDict);
            
            if (err != noErr) {
                break;
            }
        }
    }

    // Continue with our AirPort fix.  If we changed any Ethernet interfaces 
    // to AirPort, resort the array so that all AirPort interfaces are after 
    // all Ethernet and FireWire interfaces.

    if ( (err == noErr) && resortAirPort && (CFArrayGetCount(result) > 1) ) {
        AirPortFixupResort(result);
    }

    // Clean up.
    
    if (err == noErr) {
        *portArray = result;
    } else {
        CFQRelease(result);
    }
    CFQRelease(interfaces);

	assert( (err == noErr) == (*portArray != NULL) );
    
    return err;
}

/////////////////////////////////////////////////////////////////
#pragma mark ***** Common Code

// MoreSCF now has two implementations of MoreSCCreatePortArray.  The new implementation 
// uses the snazzy, new (as of 10.4), SCNetworkInterfaceCopyAll API.  It's nice and 
// small.  The old implementation uses the old horrible code which tries to shadow 
// the code that the Network preferences pane uses.  It's important that you use the 
// new code if SCNetworkInterfaceCopyAll is available.  However, for testing and 
// debugging the old code, I have a compile-time switch (MORESCFPORTSCANNER_TEST_OLD_ON_NEW) 
// that allows you to run the old code even if SCNetworkInterfaceCopyAll is present.

#if !defined(MORESCFPORTSCANNER_TEST_OLD_ON_NEW)
    #define MORESCFPORTSCANNER_TEST_OLD_ON_NEW 0
#endif

extern OSStatus MoreSCCreatePortArray(CFArrayRef *portArray)
{
    OSStatus            err;
    
    assert( portArray != nil);
    assert(*portArray == nil);
    
    #if MORESCFPORTSCANNER_TEST_OLD_ON_NEW
        #warning MoreSCFPortScanner: Test code enabled; do not ship this way!
        err = unimpErr;
    #else
        err = MoreSCCreatePortArrayNew(portArray);
    #endif
    if (err == unimpErr) {
        err = MoreSCCreatePortArrayOld(portArray);
    }

	assert( (err == noErr) == (*portArray != NULL) );
    
    return err;
}
