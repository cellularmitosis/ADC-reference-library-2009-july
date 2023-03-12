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

$Log: MoreSCFPortScanner.h,v $
Revision 1.5  2003/02/26 20:59:18         
<rdar://problem/3183087> Added support for V.92 modem hold.

Revision 1.4  2002/11/09 00:01:09         
Added compile time environment check. When using framework includes, explicitly include the frameworks we need. Convert nil to NULL.

Revision 1.3  2002/01/31 10:20:37         
Added bug numbers for the bugs requesting port scanner APIs.

Revision 1.2  2002/01/22 06:17:22         
Total rewrite. Kept Robert's ideas but complete rewrote the implementation and also changed the API to work better in an SCF environment.

Revision 1.1  2002/01/16 22:52:39         
First checked in.


*/

#pragma once

/////////////////////////////////////////////////////////////////

// MoreIsBetter Setup

#include "MoreSetup.h"

#if !TARGET_RT_MAC_MACHO
	#error MoreSCFPortScanner requires the use of Mach-O
#endif

// System Interfaces

#include <IOKit/IOKitLib.h>
#include <CoreServices/CoreServices.h>

/////////////////////////////////////////////////////////////////

#ifdef __cplusplus
extern "C" {
#endif

/////////////////////////////////////////////////////////////////
// Port Scanner

// IMPORTANT
// Apple currently provides no high-level API for finding all of the ports suitable 
// for networking, nor do we provide an API for getting the user-visible name of a 
// port.  This module solves that problems.  It searches the IORegistry and creates 
// a port list that includes the port's user visible name.  To use it properly you 
// must provide a callback (set via MoreSCFSetPortNameCallback) which it can use 
// to get the localised versions of certain strings.  This means that you have 
// to hard code port name strings in your application.  This is not an ideal 
// solution, but it's the best you can do until we provide the necessary APIs
// [2851695, 2851696, 2851697].  Hopefully I will be able to layer a later 
// version of this code on top of any new APIs we introduce.  At a minimum you 
// will have to recompile your application when these new APIs are introduced.

extern pascal OSStatus MoreSCCreatePortArray(CFArrayRef *portArray);
	// Builds a list of network ports and returns their attributes as a CFArray 
	// of CFDictionaries, with one dictionary per port.  The array is sorted in 
	// the default order which one would display the ports to the user (and set 
	// them up in the IPv4 service order array). Each entries contains the 
	// following required keys.
	//
	// o kSCPropUserDefinedName -- the user-visible name of the port
	//
	// o kSCPropNetInterfaceHardware -- the hardware type of the interface, 
	//   typically kSCEntNetEthernet, kSCEntNetAirPort, or kSCEntNetModem
	//
	// o kSCPropNetInterfaceType -- either kSCValNetInterfaceTypeEthernet 
	//   or kSCValNetInterfaceTypePPP
	//
	// o kSCPropNetInterfaceDeviceName -- typically the BSD name of the 
	//   network device; for serial-like devices this is the base 
	//   name, including the suffix, but not including any leading "cu." 
	//   and so on
	//
	// The dictionary may also contain values for the following keys. 
	//
	// o kSCPropNetInterfaceSubType -- either kSCValNetInterfaceSubTypePPPSerial 
	//   or kSCValNetInterfaceSubTypePPPoE, only present if  
	//   kSCPropNetInterfaceType is kSCValNetInterfaceTypePPP
	// 
	// o kSCPropMACAddress -- the MAC address for Ethernet-like devices,
	//   encoded as a colon delimited string
	//
	// o kMoreSCPropNetInterfaceHardwareVariant -- set to 
	//   kMoreSCValNetInterfaceHardwareVariantIrDACOMM to distinguish 
	//   modem/serial ports from IrDA COMM ports
	//
	// o kSCPropNetInterfaceSupportsModemOnHold -- may be present for 
	//   modem devices (kSCPropNetInterfaceHardware is kSCEntNetModem); if 
	//   present, this is a CFNumber of value 0 or 1 indicating whether the 
	//   modem supports V.92 call waiting; if not present, assume 0
	//
	// The dictionary may also contain other keys. These keys were set up 
	// during the operation of the function. You may find their values useful.
	// 
	// o "AAPL,slot-name" -- For PCI devices this is the name of the 
	//   slot that the device is plugged in to.
	//
	// o "IOChildIndex" -- For PCI devices thish is the port number.
	//   To tell you the truth I'm not exactly sure what this means )-:
	//
	// o kIOBSDNameKey -- For generic Ethernet-like devices this is the 
	//   BSD name of the underlying device.
	//
	// o kIOTTYBaseNameKey -- For generic serial ports this is the 
	//   base name of the serial driver.
	//
	// This function takes a guess at the user-visible name of the port 
	// and then calls your callback function (which you install using 
	// MoreSCFSetPortNameCallback) to refine that name.  If your 
	// callback function doesn't override the user-visible name 
	// this function will just returns its guess.  Note that the code 
	// that generates the guess is bug-for-bug compatible with the 
	// Network preferences panel in Mac OS X 10.1.x and may not produce 
	// ideal results. Moreover, the guess is not localised.
	// 
	//  portArray must not be NULL.
	// *portArray must be NULL.
	// On success, *portArray will be non-NULL; the caller is responsible for 
	// releasing it.
	// On error, *portArray will be NULL.

#define kMoreSCPropNetInterfaceHardwareVariant CFSTR("HardwareVariant")

#define kMoreSCValNetInterfaceHardwareVariantIrDACOMM CFSTR("IrDA-IrCOMM")
/////////////////////////////////////////////////////////////////
// Localisable Port Names

// You can install a MoreSCFPortNameCallback to return the localised 
// name for a port.  The function is called with a non-localised 
// port label and is expected to return a localised name.
//
// If you don't install a callback the default (English) names for 
// the ports are used.
//
// One day a revision of IOKit will make it easy to get port names. 
// Until that day...

typedef pascal kern_return_t (*MoreSCFPortNameCallback)(io_object_t interface, 
														CFMutableDictionaryRef interfaceInfo, 
														CFStringRef proposedUserVisibleName, 
														CFStringRef *userVisibleName);
	// MoreSCF calls this callback when it needs to determine the user-visible 
	// name of a port.  The callback is provided with a reference to the 
	// I/O Registry object, a dictionary of information about that object 
	// (this is basically the same dictionary as is returned by 
	// MoreSCCreatePortArray, without the kSCPropUserDefinedName of course) 
	// and a proposed user-visible name.  The routine can either leave 
	// *userVisibleName as NULL, in which case MoreSCF uses the 
	// proposedUserVisible name, or set it to a user-visible name that's 
	// better than the proposed one.  
	//
	// For example, your code might have a set of localised strings 
	// for each of the port labels listed below, and return the 
	// appropriate string based on the current user's preferences.  Or 
	// your code might have special information about a specific 
	// interface, and return a custom string for that interface. 
	//
	// MoreSCF does keyword substitution (for example, replacing 
	// %2$@ with the PCI slot name that the device resides in) *after* 
	// calling this routine, so you don't have to parse these variable 
	// keywords out of proposedUserVisibleName when deciding on your 
	// localisations.  You can also modify the substitions that are 
	// performed by returning a string with different keywords.
	//
	// interface will not be 0.
	// interfaceInfo will not be NULL.
	// proposedUserVisibleName will not be NULL.  It will be either one of the 
	// labels listed below, or a more specific name derived from the I/O Registry.
	// userVisibleName will not be NULL.
	// *userVisibleName will be NULL.
	// On success, *userVisibleName may be NULL or not NULL.  If not NULL, 
	// it's the user visible name of the port.  If NULL, your code doesn't 
	// have a user visible name for this type of port.
	// On error, *userVisibleName must be NULL.
	//
	// *userVisibleName can contain the following strings which are substituted 
	// by MoreSCF upon return.
	//
	//   %1$@ is the BSD name of the device
	//   %2$@ is the PCI slot name of the device
	//   %3$@ is the function number of a multifunction PCI device
	//   %4$@ is the lowercased TTY base name of the device
	//        Why lowercase?  Because that's what the 10.1.x control 
	//        panel does, for reasons unknown.

#define kMoreSCPortLabelEthernetBuiltIn 		CFSTR("Built-in Ethernet")
	// built-in Ethernet
#define kMoreSCPortLabelAirPort 				CFSTR("AirPort")
	// Apple AirPort
#define kMoreSCPortLabelWireless 				CFSTR("Wireless Network Adaptor")
	// non-AirPort wireless
#define kMoreSCPortLabelEthernetPCI 			CFSTR("PCI Ethernet Slot %2$@")
	// generic PCI card
#define kMoreSCPortLabelEthernetPCIMultiport 	CFSTR("PCI Ethernet Slot %2$@, Port %3$@")
	// multiport PCI card (I think we handle this wrongly but it's compatible 
	// with the Network preferences panel)
#define kMoreSCPortLabelEthernet 				CFSTR("Ethernet Adaptor (%1$@)")	
	// non-PCI Ethernet
#define kMoreSCPortLabelModemInternal 			CFSTR("Internal Modem")
	// built-in modem
#define kMoreSCPortLabelModemPort 				CFSTR("Modem Port")					
	// built-in serial port (eg Beige G3)
#define kMoreSCPortLabelPrinterPort 			CFSTR("Printer Port")				
	// ditto
#define kMoreSCPortLabelModemPrinterPort 		CFSTR("Modem/Printer Port")			
	// built-in serial port (eg Wallstreet)
#define kMoreSCPortLabelModemUSB 				CFSTR("USB Modem")					
	// generic USB modem, usually we find a better name in I/O registry
#define kMoreSCPortLabelModemIrDA 				CFSTR("IrDA Modem Port")			
	// USB-based IrDA, including built-in port on TiBook
#define kMoreSCPortLabelSerial 					CFSTR("%4$@-port")					
	// generic serial port

extern pascal void MoreSCFSetPortNameCallback(MoreSCFPortNameCallback callback);
	// Installs the callback function, eliminating any existing one. 
	// Pass NULL to remove your callback.

/////////////////////////////////////////////////////////////////

#ifdef __cplusplus
}
#endif
