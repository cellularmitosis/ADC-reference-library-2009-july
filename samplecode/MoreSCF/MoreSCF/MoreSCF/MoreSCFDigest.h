/*
	File:		MoreSCFDigest.h

	Contains:	Routines for working with SC entities.

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

$Log: MoreSCFDigest.h,v $
Revision 1.12  2006/03/27 14:42:25         
Eliminate high-bit set characters.

Revision 1.11  2006/03/24 16:15:27         
Tidy headers: eliminate #pragma once, check C++ guards, eliminate bogus pragmas.

Revision 1.10  2006/03/24 15:44:30         
Updated copyright.

Revision 1.9  2006/03/24 12:38:38         
Eliminate "pascal" keyword.

Revision 1.8  2006/03/24 11:30:04         
Eliminated "MoreSetup.h" to make it easier for folks to copy MIB source into their projects.

Revision 1.7  2003/02/26 20:52:05         
<rdar://problem/3183087> Added support for V.92 modem hold.

Revision 1.6  2003/02/26 12:15:14         
In MoreSCPPPDigest, document that commRemoteAddress is also the PPPoE service name.

Revision 1.5  2002/11/14 20:16:35         
Tidy up the debug checking in MoreSCCreateIPv4Entity and add a way to disable it for the benefit of MoreSCNewSet.

Revision 1.4  2002/11/09 00:01:02         
Added compile time environment check. When using framework includes, explicitly include the frameworks we need. Convert nil to NULL.

Revision 1.3  2002/08/14 16:26:28         
You must now get the default PPP options via a function, so that I can return different values depending on the context (system version and whether its PPPoE).

Revision 1.2  2002/01/22 06:18:06         
Added variant field to MoreSCInterfaceDigest to support IrDA.

Revision 1.1  2002/01/16 22:52:30         
First checked in.


*/

#ifndef _MORESCFDIGEST_H
#define _MORESCFDIGEST_H

/////////////////////////////////////////////////////////////////

// System prototypes

#include <CoreServices/CoreServices.h>

#include <netinet/in.h>			// for in_addr

typedef struct in_addr in_addr;

/////////////////////////////////////////////////////////////////

#ifdef __cplusplus
extern "C" {
#endif

/////////////////////////////////////////////////////////////////
// Utilities

extern OSStatus MoreSCFCreateStringWithMacAddress(const UInt8 *macAddr, CFStringRef *result);
	// Creates a CFString that represents macAddr in the traditional 
	// way (six colon delimited hex bytes).
	//
	// macAddr must not be NULL.  It must point to a 6 byte MAC address.
	// result must not be NULL.
	// On input, *result must be NULL.
	// On error, *result is always NULL.
	// On succces, *result is set to a CFStringRef.  You must 
	// release it.

extern OSStatus MoreSCFStringToMacAddress(CFStringRef cfStr, UInt8 *macAddr);
	// Parses a MAC address string (cfStr) and puts the results into 
	// a 6 byte MAC address pointed to by macAddr.
	//
	// cfStr must not be NULL.
	// macAddr must not be NULL. It must point to a 6 byte buffer.

extern OSStatus MoreSCFCreateStringWithInetAddress(in_addr addr, CFStringRef *result);
	// Creates a CFString with that represents an IP address in the 
	// traditional dotted decimal notation.
	//
	// result must not be NULL.
	// On input, *result must be NULL.
	// On error, *result is always NULL.
	// On succces, *result is set to a CFStringRef.  You must 
	// release it.

extern OSStatus MoreSCFCreateArrayWithInetAddresses(ItemCount addrCount, const in_addr *addresses, CFArrayRef *result);
	// Creates a CFArray that contains elements which are CFStrings 
	// that represent the IP addresses in the traditional dotted 
	// decimal notation.
	//
	// addresses must not be NULL unless addrCount is 0.
	// result must not be NULL.
	// On input, *result must be NULL.
	// On error, *result is always NULL.
	// On succces, *result is set to a CFArrayRef.  You must 
	// release it.

extern OSStatus MoreSCFCopyEncodedPPPPassword(CFStringRef password, CFDataRef *encodedPassword);
	// Encodes the password string into a CFData format suitable 
	// for use in the kSCPropNetPPPAuthPassword property of a 
	// PPP entity.
	//
	// password must not be NULL.
	// encodedPassword must not be NULL.
	// On input, *encodedPassword must be NULL.
	// On error, *encodedPassword is always NULL.
	// On succces, *encodedPassword is set to a CFDataRef.  You must 
	// release it.

/////////////////////////////////////////////////////////////////
// Digest Stuff

// Entity's required for a particular type of service:
//
// Entities          PPP   Ethernet   AirPort  Global
// --------          ---   --------   -------  ------
// IPv4											 +
// NetInfo								         +
//
// Interface          +        +        +
// Proxies            +        +        +
//
// AppleTalk                   +        +
// DNS                +        +        +
// IPv4               +        +        +
// PPP                +        +        +
//
// AirPort                              +
// Ethernet                    +
// Modem              +

// Global Entities
// ---------------

// IPv4

struct MoreSCIPv4GlobalDigest {
	UInt32 dummy;
};
typedef struct MoreSCIPv4GlobalDigest MoreSCIPv4GlobalDigest;

extern OSStatus MoreSCCreateIPv4GlobalEntity(const MoreSCIPv4GlobalDigest *digest, CFDictionaryRef *entity);
	// See MoreSCCreateGlobalEntity.

// NetInfo

// I've skipped over a bunch of possibly relevant properties.  I'll 
// have to return to this one day.

struct MoreSCNetInfoGlobalDigest {
	UInt32 dummy;
};
typedef struct MoreSCNetInfoGlobalDigest MoreSCNetInfoGlobalDigest;

extern OSStatus MoreSCCreateNetInfoGlobalEntity(const MoreSCNetInfoGlobalDigest *digest, CFDictionaryRef *entity);
	// See MoreSCCreateGlobalEntity.

// Universal Entities
// ------------------
// You must have one of these in every service.

// Interface

struct MoreSCInterfaceDigest {
	CFStringRef deviceName;			// BSD name of port
	CFStringRef hardware;			// name of hardware entity, one of
									// kSCEntNetAirPort, kSCEntNetEthernet, kSCEntNetModem
	CFStringRef variant;			// set to kMoreSCValNetInterfaceHardwareVariantIrDACOMM to 
									// indicate a IrDA COMM port
	CFStringRef type;				// kSCValNetInterfaceTypeEthernet or kSCValNetInterfaceTypePPP
	CFStringRef subType;			// NULL or kSCValNetInterfaceSubTypePPPoE     for type == kSCValNetInterfaceTypeEthernet
									// NULL or kSCValNetInterfaceSubTypePPPSerial for type == kSCValNetInterfaceTypePPP
	CFStringRef	userDefinedName;	// user visible name of port
	Boolean		supportsHold;		// only meaningful if hardware is kSCEntNetModem,
									// true if the modem support V.92 hold
};
typedef struct MoreSCInterfaceDigest MoreSCInterfaceDigest;

extern OSStatus MoreSCCreateInterfaceEntity(const MoreSCInterfaceDigest *digest, CFDictionaryRef *entity);
	// See MoreSCCreateEntity.
	// 
	// Typically you get the values for these fields from the results 
	// of MoreSCCreatePortArray.

// Proxies

// To get the correct default for the MoreSCProxiesDigest structure, just 
// BlockZero the entire thing.

struct MoreSCProxiesDigest {
	// The bulk of the proxy preferences have a common structure.  The 
	// following don't fit within that structure.
	
	CFArrayRef 	exceptionsList;		// CFArray of CFString
	Boolean		ftpPassive;

	// For each of the following, the value of the "enable" field is reflected 
	// in the checkbox of the Network preferences panel.  You can independently
	// choose to set the "proxy" field to NULL, in which case no proxy is recorded. 
	// Furthermore, you can set the "proxy" field to non-NULL, even if the checkbox 
	// is disabled, in which case the string will be the default if the user 
	// enables the checkbox.  A similar logic applies to a value of 0 in the "port" 
	// field.  Note that this prevents you from setting a proxy to port 0, which is 
	// a value supported by the data structures but not by MoreSCF.  Curiously enough, 
	// the Network preferences panel also treats a port of 0 as equivalent to an empty 
	// field.
	
	Boolean 	ftpEnable;
	CFStringRef ftpProxy;
	UInt16		ftpPort;
	Boolean 	gopherEnable;
	CFStringRef gopherProxy;
	UInt16		gopherPort;
	Boolean 	httpEnable;
	CFStringRef httpProxy;
	UInt16		httpPort;
	Boolean 	httpsEnable;
	CFStringRef httpsProxy;
	UInt16		httpsPort;
	Boolean 	rtspEnable;
	CFStringRef rtspProxy;
	UInt16		rtspPort;
	Boolean 	socksEnable;
	CFStringRef socksProxy;
	UInt16		socksPort;
    
    Boolean     excludeSimpleHostnames;         // 10.4 and later
    Boolean     autoDiscoveryEnable;            // 10.4 and later
};
typedef struct MoreSCProxiesDigest MoreSCProxiesDigest;

extern OSStatus MoreSCCreateProxiesEntity(const MoreSCProxiesDigest *digest, CFDictionaryRef *entity);
	// See MoreSCCreateEntity.

// Protocol Entities
// -----------------
// Most of these are always present (but can be inactive), 
// except for AppleTalk on PPP, which is never present.

// AppleTalk

// We add an active Boolean for AppleTalk only because, as a policy, 
// we default to have AppleTalk disabled for new services.

struct MoreSCAppleTalkDigest {
	Boolean			active;
	CFStringRef		configMethod;			// only kSCValNetAppleTalkConfigMethodNode support for now
	Boolean			manual;
	UInt16			networkID;				// relevant only if manual == true
	UInt8			nodeID;					// relevant only if manual == true
	CFStringRef		defaultZone;			// NULL gets you default default zone (-:
};
typedef struct MoreSCAppleTalkDigest MoreSCAppleTalkDigest;

extern OSStatus MoreSCCreateAppleTalkEntity(const MoreSCAppleTalkDigest *digest, CFDictionaryRef *entity);
	// See MoreSCCreateEntity.

// DNS

struct MoreSCDNSDigest {
	ItemCount		serverAddressesCount;
	in_addr *		serverAddresses;		// NULL OK if count 0
	ItemCount		searchDomainsCount;
	CFStringRef * 	searchDomains;			// NULL OK if count 0
};
typedef struct MoreSCDNSDigest MoreSCDNSDigest;

extern OSStatus MoreSCCreateDNSEntity(const MoreSCDNSDigest *digest, CFDictionaryRef *entity);
	// See MoreSCCreateEntity.

// IPv4

// The digest doesn't allow you to configure multiple IPs on the 
// same interfaces; if you need this you'll have to create or 
// modify the entity yourself.

struct MoreSCIPv4Digest {
	CFStringRef		configMethod;			// one of kSCValNetIPv4ConfigMethod*
	in_addr			address;				// set to 0 if meaningless (ie DHCP)
	in_addr			subnetMask;				// set to 0 if meaningless
	in_addr			router;					// set to 0 if meaningless
	CFStringRef 	clientID;				// NULL OK
};
typedef struct MoreSCIPv4Digest MoreSCIPv4Digest;

extern OSStatus MoreSCCreateIPv4Entity(const MoreSCIPv4Digest *digest, CFDictionaryRef *entity);
	// See MoreSCCreateEntity.

#if !defined(NDEBUG)

	// Set to quieten a address checking assert.
	
	extern Boolean gMoreSCCreateIPv4EntityDontCheck;

#endif

// PPP

// The following structure encapsulates all of the user-visible 
// PPP options.  The comments include the default value (in 
// square brackets []) and the user-visible label (in quotes "").

struct MoreSCPPPOptions {
	Boolean			dialOnDemand;			// [false] "Connect automatically when starting TCP/IP applications"
	Boolean			idleReminder;			// [false] "Prompt every N minutes to maintain connection"
	SInt32			idleReminderTimer;		// [1800] seconds
	Boolean			disconnectOnIdle;		// [true] "Disconnect if idle for N minutes"
	SInt32			disconnectOnIdleTimer;	// [900] seconds
	Boolean			disconnectOnLogout;		// [true] "Disconnect when user logs out"
	Boolean			commRedialEnabled;		// [true] "Redial if busy"
	SInt32			commRedialCount;		// [1] "Redial N times"
	SInt32			commRedialInterval;		// [30] seconds "Wait N seconds before redialing"
	Boolean			lcpEchoEnabled;			// [true] "Send PPP echo packets"
	Boolean			ipcpCompressionVJ;		// [true] "Use TCP header compression"
	Boolean			commDisplayTerminalWindow;	// [false] "Connect using a terminal window"
	Boolean			verboseLogging;			// [false] "Use verbose logging"
};
typedef struct MoreSCPPPOptions MoreSCPPPOptions;

// You can set the above PPP options by pointing the options field 
// of MoreSCPPPDigest to point to a MoreSCPPPOptions structure.  If you 
// don't pass in an options structure, you get the default values 
// (which are also available via the function below).

extern const MoreSCPPPOptions *MoreSCGetDefaultPPPOptions(Boolean isPPPoE);
	// This returns one of a range of statically allocated 
	// constant structures, so you don't have to free the 
	// result.

struct MoreSCPPPDigest {
	Boolean				active;
	CFStringRef			userDefinedName;				// NULL OK "Service Provider"
	CFStringRef			authName;						// NULL OK "Account Name"
	CFStringRef			authPassword;					// NULL OK "Password"
	CFStringRef			commRemoteAddress;				// NULL OK "Telephone Number" (or "PPPoE Service Name" for PPPoE)
	CFStringRef			commAlternateRemoteAddress;		// NULL OK "Alternate Number"
	const MoreSCPPPOptions *options;					// NULL OK
};
typedef struct MoreSCPPPDigest MoreSCPPPDigest;

extern OSStatus MoreSCCreatePPPEntity(const MoreSCPPPDigest *digest, CFDictionaryRef *entity);
	// See MoreSCCreateEntity.
	// 
	// Note that unless you specify an options pointer, you will get 
	// the PPP-over-serial options (as returned by 
	// MoreSCGetDefaultPPPOptions(false) by default.

// Hardware Entities
// -----------------
// Must must have at most one of the following in each service, 
// depending on the type of service.

// AirPort

struct MoreSCAirPortDigest {
	UInt8			macAddress[6];
	CFStringRef		preferredNetwork;		// NULL OK
};
typedef struct MoreSCAirPortDigest MoreSCAirPortDigest;

extern OSStatus MoreSCCreateAirPortEntity(const MoreSCAirPortDigest *digest, CFDictionaryRef *entity);
	// See MoreSCCreateEntity.

// Ethernet

struct MoreSCEthernetDigest {
	UInt8			macAddress[6];
};
typedef struct MoreSCEthernetDigest MoreSCEthernetDigest;

extern OSStatus MoreSCCreateEthernetEntity(const MoreSCEthernetDigest *digest, CFDictionaryRef *entity);
	// See MoreSCCreateEntity.

// Modem

// Modem supports many more options that I don't have time to deal with right now.

struct MoreSCModemDigest {
	CFStringRef		connectionScript;					// name of file in modem scripts folder
	Boolean			dataCompressionErrorCorrection;		// [true] "Enable error correction and compression in modem"
	Boolean			speaker;							// [true] "Speaker"
	Boolean			pulseDial;							// [false] "Dialing: Pulse"
	Boolean			waitForDialTone;					// [true] "Wait for dial tone"
	Boolean			supportsHold;						// whether to add default V.92 properties
};
typedef struct MoreSCModemDigest MoreSCModemDigest;

extern const MoreSCModemDigest kMoreSCModemDigestDefault;

// IMPORTANT:
// The default value for connectionScript is NULL, which probably 
// isn't what you want.

extern OSStatus MoreSCCreateModemEntity(const MoreSCModemDigest *digest, CFDictionaryRef *entity);
	// See MoreSCCreateEntity.

/////////////////////////////////////////////////////////////////

// The reason for the horribly distinction between global and 
// per-service entities is that the same protocol key (kSCEntNetIPv4) 
// can be used for both a global and a per-service entity.  
// Therefore, it wasn't possible to a have a single "create entity"
// routine, because if protocol was kSCEntNetIPv4 we wouldn't know 
// whether to create a global or a per-service entity )-:

union MoreSCGlobalDigest {
	MoreSCIPv4GlobalDigest 		ipv4;
	MoreSCNetInfoGlobalDigest 	netInfo;
};
typedef union MoreSCGlobalDigest MoreSCGlobalDigest;

extern OSStatus MoreSCCreateGlobalEntity(CFStringRef protocol, const MoreSCGlobalDigest *digest, CFDictionaryRef *entity);
	// Creates an entity dictionary based on a digest structure. 
	// protocol must be set to either kSCEntNetIPv4 or kSCEntNetNetInfo 
	// to indicate which member of the MoreSCGlobalDigest union is 
	// valid.
	//
	// protocol must not be NULL.
	// digest must not be NULL.
	// entity must not be NULL.
	// On input, *entity must be NULL.
	// On error, *entity is always NULL.
	// On succces, *entity is set to a CFDictionaryRef.  You must 
	// release it.

union MoreSCDigest {
	MoreSCInterfaceDigest 	interface;
	MoreSCProxiesDigest 	proxies;
	MoreSCAppleTalkDigest 	appleTalk;
	MoreSCDNSDigest 		dns;
	MoreSCIPv4Digest 		ipv4;
	MoreSCPPPDigest 		ppp;
	MoreSCAirPortDigest 	airPort;
	MoreSCEthernetDigest 	ethernet;
	MoreSCModemDigest 		modem;
};
typedef union MoreSCDigest MoreSCDigest;

extern OSStatus MoreSCCreateEntity(CFStringRef protocol, const MoreSCDigest *digest, CFDictionaryRef *entity);
	// Creates an entity dictionary based on a digest structure. 
	// protocol must be set to kSCEntNetInterface, kSCEntNetProxies, 
	// kSCEntNetAppleTalk, kSCEntNetDNS, kSCEntNetIPv4, kSCEntNetPPP, 
	// kSCEntNetAirPort, kSCEntNetEthernet, or kSCEntNetModem
	// to indicate which member of the MoreSCDigest union is 
	// valid.
	//
	// protocol must not be NULL.
	// digest must not be NULL.
	// entity must not be NULL.
	// On input, *entity must be NULL.
	// On error, *entity is always NULL.
	// On succces, *entity is set to a CFDictionaryRef.  You must 
	// release it.

/////////////////////////////////////////////////////////////////

#ifdef __cplusplus
}
#endif

#endif
