/*
	File:		MoreSCFPortScanner.c

	Contains:	Code to generate list of network ports on Mac OS X.

	Written by:	Quinn, based on code by Robert Ulrich

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

$Log: MoreSCFPortScanner.c,v $
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

#include <mach/mach.h>
#include <IOKit/IOKitLib.h>
#include <IOKit/IOBSD.h>
#include <IOKit/network/IONetworkController.h>
#include <IOKit/network/IOEthernetInterface.h>
#include <IOKit/serial/IOSerialKeys.h>
#include <IOKit/ndrvsupport/IOMacOSTypes.h>
#include <IOKit/storage/IOStorageDeviceCharacteristics.h>
#include <SystemConfiguration/SystemConfiguration.h>

// MIB prototypes

#include "MoreCFQ.h"

/////////////////////////////////////////////////////////////////

static CFStringRef CreateStringFromData(CFDataRef data)
	// Some IOKit strings are encoded in CFDataRefs; extract as a CFStringRef.
	// Also remove any trailing null characters, which can lurk in the 
	// I/O Registry strings.
{
    CFIndex len;

	assert(data != NULL);
	
	// Decrement len until to eliminate any trailing null characters.
	    
    len = CFDataGetLength(data);
    do {
        char ch;
        
        if (len < 1) {
            break;
        }
        CFDataGetBytes(data, CFRangeMake(len - 1, 1), (UInt8 *) &ch);
        if (ch != 0) {
            break;
        }
        len -= 1;
    } while (true);
    
    return CFStringCreateWithBytes(NULL, 
                            CFDataGetBytePtr(data), len, kCFStringEncodingMacRoman, false );
}

static CFStringRef CreateMACFromData(CFDataRef data)
	// Extracts a MAC address as a colon delimited string from 
	// a CFDataRef extracted from I/O Registry.
{
    // mac addresses are 6 bytes long, which translates to 18 chars.

    UInt8 bytes[6];
    UInt8 *p = &bytes[0];
    char s [19];
    char *cp = &s[0];
    CFIndex len;

	assert(data != NULL);
	
	len = CFDataGetLength( data );

    // make sure it's valid
    
    if ( len != 6 )
        return NULL;

    CFDataGetBytes( data, CFRangeMake( 0, 6 ), &bytes[0] );

    // add digits in pairs with colon separators

    while ( len-- )
        cp += sprintf( cp, "%2.2x:", *p++ );

    // terminate string

    s[17] = '\0';

    return CFStringCreateWithCString( NULL, s, kCFStringEncodingMacRoman );
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

// Ports are sorted by a primary sort order (defined by the enumeration 
// below) and then sorted by name within their groups.  This enumeration 
// closely mirrors the behaviour of the Network preferences panel.

enum {
	kInternalModemSortOrder 	= 0,
	kUSBModemSortOrder 			= 1,
	kModemSortOrder 			= 2,
	kIrDASerialSortOrder		= 3,
	kSerialSortOrder 			= 4,
	kBuiltInEthernetSortOrder 	= 5,
	kEthernetPCISortOrder 		= 6,
	kAirPortSortOrder 			= 7,
	kWirelessSortOrder 			= 8,
	kEthernetSortOrder 			= 9,
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
	
	assert( interface       != 0  );
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
	UInt32			sysVer;
	Boolean			tenTwoOrLater;
	
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
	
	tenTwoOrLater = (Gestalt(gestaltSystemVersion, (SInt32 *) &sysVer) == noErr) 
					&& (sysVer >= 0x01020);
	if (tenTwoOrLater) {
	
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

static kern_return_t CopyUserVisibleNameForEthernetPort(io_object_t interface, 
														CFMutableDictionaryRef interfaceInfo,
														CFStringRef *userVisibleName)
	// Given an Ethernet device (interface) and a dictionary of 
	// information about the device (interfaceInfo), guess at the 
	// user-visible name for the device.
	// 
	// I'm somewhat unhappy with this code (both its length and its style) 
	// but there really isn't a better solution right now.  Apple 
	// is actively working on a better way to do this.
{
	kern_return_t 			err;
	kern_return_t			junk;
	io_object_t 			controller;
	io_object_t				bus;
	CFStringRef				bsdName;
	CFMutableDictionaryRef 	controllerDict;
	
	assert(interface        != 0  );
	assert(interfaceInfo    != NULL);
	assert( userVisibleName != NULL);
	assert(*userVisibleName == NULL);

	// Look up the parent service, and the parent of that.
	
	controller = 0;
	bus = 0;
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
        } else if ( CFDictionaryGetValueIfPresent(controllerDict, CFSTR(kIOClassKey), (const void **) &tmpStr)
	                && CFEqualString(tmpStr, CFSTR("AirPortDriver"))) {
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
				CFDataRef nameData;
				
				nameData = (CFDataRef) CFDictionaryGetValue(busDict, CFSTR("name"));
				if (nameData == NULL) {
					nameProperty = NULL;
				} else {
					nameProperty = CreateStringFromData(nameData);
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
					CFDataRef	slotNameAsData;
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
					slotNameAsData = (CFDataRef) IORegistryEntryCreateCFProperty(bus, CFSTR("AAPL,slot-name"), NULL, kNilOptions);
					if (slotNameAsData != NULL) {
						slotName = CreateStringFromData(slotNameAsData);
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
	// but there really isn't a better solution right now.  Apple 
	// is actively working on a better way to do this.
{
	kern_return_t 	err;
	kern_return_t 	junk;
	CFStringRef   	serialType;
	Boolean			isModem;
	CFStringRef		baseName;
	CFStringRef		productName;
	
	assert(interface        != 0  );
	assert(interfaceInfo    != NULL);
	assert( userVisibleName != NULL);
	assert(*userVisibleName == NULL);

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
	
	assert(interface        != 0  );
	assert(interfaceInfo    != NULL);
	assert( userVisibleName != NULL);
	assert(*userVisibleName == NULL);
	
	proposedName = NULL;
	baseName = NULL;
	
	// First we synthesise our candidate user-visible name.
	
	if ( IOObjectConformsTo(interface, kIOEthernetInterfaceClass) ) {
		err = CopyUserVisibleNameForEthernetPort(interface, interfaceInfo, &proposedName);
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
		
		#if MORE_DEBUG
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

extern pascal void MoreSCFSetPortNameCallback(MoreSCFPortNameCallback callback)
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
	CFStringRef		bsdName;
	io_object_t		controller;

	assert(interface        != 0  );
	assert(interfaceInfo    != NULL);
	
	controller = 0;
	bsdName = NULL;
	
	junk = CFQDictionarySetNumber(interfaceInfo, kSortOrderKey, kEthernetSortOrder);
	assert(junk == 0);
	
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

	CFDictionarySetValue(interfaceInfo, kSCPropNetInterfaceHardware, kSCEntNetEthernet);
	
	// The above value is overriden (set to kSCEntNetAirPort) by the user-visible name code 
	/// when it determines that the card is really an AirPort card.

    // kSCPropNetInterfaceType

	CFDictionarySetValue(interfaceInfo, kSCPropNetInterfaceType, kSCValNetInterfaceTypeEthernet);

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

	// Clean up.

	junk = IOObjectRelease(controller);
	assert(junk == 0);

	CFQRelease(bsdName);
	
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
	CFMutableDictionaryRef	interfaceDict;
	CFStringRef				baseName;
	CFNumberRef				supportsHold;
	
	assert(interface        != 0  );
	assert(interfaceInfo    != NULL);

	interfaceDict = NULL;
	supportsHold  = false;
	
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
    
    CFQRelease(interfaceDict);
   	CFQRelease(supportsHold);
    
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

	assert(masterPort        != 0  );
	assert(deviceMatchDict   != NULL);
	assert(deviceToDict      != NULL);
	assert(result            != NULL);
	
	iterator = 0;
	
    err = IOServiceGetMatchingServices(masterPort, deviceMatchDict, &iterator);
	if (err == 0) {
	    do {
			interface = IOIteratorNext(iterator);
			if (interface != 0) {
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
	    } while (interface != NULL && err == 0);
	}
	    
    if (iterator != 0) {
    	junk = IOObjectRelease(iterator);
    	assert(junk == 0);
    }
    
    return err;
}

static mach_port_t gMasterPort = 0;

extern pascal OSStatus MoreSCCreatePortArray(CFArrayRef *portArray)
	// See comment in header.
{
    kern_return_t		err;
    CFMutableArrayRef	result;
	CFStringRef 		keys[2];
	CFStringRef 		values[2];

	assert( portArray != NULL);
	assert(*portArray == NULL);
	
	result = NULL;
	    
	err = CFQArrayCreateMutable(&result);
	if (err == 0 && gMasterPort == 0) {
	    err = IOMasterPort(bootstrap_port, &gMasterPort);
	}
	
	// Ethernet-like devices (includes AirPort)
	
    if (err == 0) {
		err = AddMatchingDevicesToArray(gMasterPort, IOServiceMatching(kIOEthernetInterfaceClass), 
										EthernetDeviceToDictProc, NULL, result);
    }
    
    // Set up common key/value pairs for the next two calls.
    
	  keys[0] = CFSTR(kIOProviderClassKey);
	values[0] = CFSTR(kIOSerialBSDServiceValue);
	  keys[1] = CFSTR(kIOSerialBSDTypeKey);

	// Modem devices
	
    if (err == 0) {
		values[1] = CFSTR(kIOSerialBSDModemType);

		err = AddMatchingDevicesToArray(gMasterPort, CFDictionaryCreate(NULL, (const void **) keys, (const void **) values, 2, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks), 
										ModemOrSerialDeviceToDictProc, NULL, result);
    }
    
    // Serial devices (includes IrDA)
    
    if (err == 0) {
		values[1] = CFSTR(kIOSerialBSDRS232Type);

		err = AddMatchingDevicesToArray(gMasterPort, CFDictionaryCreate(NULL, (const void **) keys, (const void **) values, 2, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks), 
										ModemOrSerialDeviceToDictProc, NULL, result);
    }
    if (err == 0) {
	    CFArraySortValues(result, CFRangeMake( 0, CFArrayGetCount( result )), PortSorter, NULL);
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
