/*
	File:		MoreSCFHelpers.h

	Contains:	High-level System Configuration framework operations.

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

$Log: MoreSCFHelpers.h,v $
Revision 1.4  2003/02/26 12:42:22         
Added a new routine, MoreSCMakeNewPPPoESet, and exported some of the sub-routines used by the MoreSCMakeNewXxxSet routines so that you can more create custom high-level routines without modifying MoreSCF itself.

Revision 1.3  2002/12/12 15:25:06         
Correct file name in header comment.

Revision 1.2  2002/11/09 00:01:06         
Added compile time environment check. When using framework includes, explicitly include the frameworks we need. Convert nil to NULL.

Revision 1.1  2002/01/16 22:52:34         
First checked in.


*/

#pragma once

/////////////////////////////////////////////////////////////////

// MoreIsBetter Setup

#include "MoreSetup.h"

#if !TARGET_RT_MAC_MACHO
	#error MoreSCFHelpers requires the use of Mach-O
#endif

// MIB Prototypes

#include "MoreSCFDigest.h"

// System prototypes

#include <CoreServices/CoreServices.h>

/////////////////////////////////////////////////////////////////

#ifdef __cplusplus
extern "C" {
#endif

/////////////////////////////////////////////////////////////////
// TCP/IP Will Dial

// You no longer need a helper function to ask the "will this connection 
// dial the modem" question.  System Configuration framework provides 
// high-level APIs to answer this question, to wit:
//
// SCNetworkCheckReachabilityByAddress
// SCNetworkCheckReachabilityByName

/////////////////////////////////////////////////////////////////
// AppleTalk On/Off

// Note that you can't really save and restore the state of AppleTalk 
// using these routines because AppleTalk could be enabled on one, but 
// not all, of the AppleTalk capable interfaces.  I need to introduce 
// new routines that return an array of Booleans.

extern pascal OSStatus MoreSCIsAppleTalkActive(CFStringRef setID, Boolean *active);
	// Sets *active to true if the AppleTalk stack is active 
	// on any active interface in the set denoted by setID
	// (pass NULL to work on the current set).  This tests the 
	// configured value in the preferences, not the current state 
	// (AppleTalk might be configured active but inactive 
	// because, say, the Ethernet cable isn't connected).

extern pascal OSStatus MoreSCSetAppleTalkActive(CFStringRef setID, Boolean active);
	// Sets the active state of the AppleTalk stack to the active
	// parameter on each interface capable of AppleTalk in the set 
	// denoted by setID (pass NULL to work on the current set).

/////////////////////////////////////////////////////////////////
// DHCP Release

extern pascal OSStatus MoreSCCreateActiveDHCPServicesArray(CFArrayRef *activeDHCP);
	// Creates a CFArray of active DHCP service IDs.  Each element is 
	// the name of a service that contains an IPv4 entity configured 
	// to use DHCP.  Typically these are really boring names, like "3".
	//
	// activeDHCP must not be NULL.
	// On input, *activeDHCP must be NULL.
	// On error, *activeDHCP will be NULL.
	// On success, *activeDHCP will be a CFArrayRef. You must release it.
	// �more testing needed
	
extern pascal OSStatus MoreSCDHCPRelease(CFStringRef serviceID);
	// Forces a DHCP release on the IPv4 entity inside the service 
	// denoted by serviceID.  serviceID is typically one of the 
	// elements of the array returned by the previous API.
	// serviceID is assumed to be within the current set because 
	// it doesn't make sense to force a release on an interface 
	// that's not active.
	// �more testing needed

/////////////////////////////////////////////////////////////////
// Super High-Level APIs for Internet Setup Assistants

extern pascal OSStatus MoreSCFindSetByUserVisibleNameAndCopyID(CFStringRef userVisibleName, CFStringRef *setID);
	// Searches the list of sets for one with the specified user 
	// visible name.  ISP software typically creates a set with 
	// a well known name.  This routine allows them to quickly 
	// determine if their set already exists.
	// 
	// userVisibleName must not be NULL.
	// setID must not be NULL.
	// On input, *setID must be NULL.
	// On error, *setID is always NULL.
	// On succces, *setID is set to NULL if no name is found or 
	// to the name of the found set.  If *setID is not NULL you 
	// must release it.

extern pascal OSStatus MoreSCFCopyDefaultPortBSDName(CFStringRef hardwareType, CFStringRef *bsdName);
	// Returns in *bsdName the BSD name of the default port of 
	// the specified hardwareType.  hardwareType is typically 
	// kSCEntNetEthernet, kSCEntNetAirPort, or kSCEntNetModem. 
	// It works by getting a list of all of the ports 
	// and then searching that list for the first port of that type.
	// MoreSCCreatePortArray returns the ports in a well-specified 
	// order, with the default port of a particular port type 
	// listed first.
	//
	// hardwareType must not be NULL.
	// bsdName must not be NULL.
	// On input, *bsdName must be NULL.
	// On error, *bsdName is always NULL.
	// On success, *bsdName is NULL if no port is found or 
	// the name of the found port. If *bsdName is not NULL, 
	// you must release it.

extern pascal OSStatus MoreSCMakeNewSingleServiceSet(
									CFStringRef					bsdName, 
									CFStringRef					userVisibleName, 
									CFDictionaryRef				serviceSettings,
									CFStringRef *				newSetID);
	// A low-level routine that forms the core of the following 
	// two high-level routines.  Use the high-level routines if you 
	// can.
	//
	// Creates a new set in which one service is active and writes 
	// serviceSettings to that service.
	// 
	// bsdName must not be NULL.  This is the BSD name of the port 
	// that should be active.
	//
	// You must not pass NULL to userVisibleName.  This is the user-visible 
	// name of the newly created set.
	//
	// serviceSettings must not be NULL.  It's a dictionary of dictionaries. 
	// The keys for the top-level dictionary are entity types (protocols), 
	// for example, kSCEntNetPPP and kSCEntNetIPv4.  The values for the 
	// top-level dictionary are entity dictionaries.  For each element 
	// of the dictionary, this routine sets the entity specified by the 
	// key to the value specified by the value.  There is, however, one 
	// exception.  For kSCEntNetInterface, the routine actually merges 
	// the entity value into the existing entity value.
	//
	// Pass NULL to setID if you don't care what set was created.  Otherwise,
	// on input, *setID must be NULL. On error, *setID is always NULL (and 
	// no set is created). On success, *setID is the set ID of the newly 
	// created set, which you must release.

extern pascal OSStatus MoreSCMakeNewDialupSet(
									CFStringRef					bsdName, 
									CFStringRef					userVisibleName, 
									const MoreSCModemDigest *	modemSettings, 
									const MoreSCPPPDigest * 	pppSettings, 
									const MoreSCIPv4Digest *	ipv4Settings,
									const MoreSCDNSDigest *		dnsSettings,
									CFStringRef *				newSetID);
	// Creates a new dialup set and configures it appropriately.
	// 
	// Pass NULL to bsdName to use first modem port; alternatively 
	// pass the name of the modem port you want the set to reference.
	// You can get this name from the kMoreSCPSPropertyDevice property 
	// of the list of ports returned by MoreSCCreatePortArray.
	//
	// You must not pass NULL to userVisibleName.  This is the user-visible 
	// name of the newly created set.
	//
	// You can pass NULL to modemSettings, in which case default settings 
	// (kMoreSCModemDigestDefault) are used (except for the modem script 
	// which is set to a useful default).
	//
	// You can't pass NULL to pppSettings.  You should at least fill out 
	// the active, authName, and commRemoteAddress fields.  You probably 
	// also want to fill out the authPassword field (in plain text).
	//
	// Pass NULL to ipv4Settings to get default.  Typically this is just 
	// fine for a PPP setup.
	//
	// Pass NULL to dnsSettings to get default.  Typically this is what 
	// you want to do unless your PPP server doesn't support RFC 1877, 
	// in which case you'll need to fill out the serverAddressesCount and 
	// serverAddresses fields.
	//
	// Pass NULL to setID if you don't care what set was created.  Otherwise,
	// on input, *setID must be NULL. On error, *setID is always NULL (and 
	// no set is created). On success, *setID is the set ID of the newly 
	// created set, which you must release.
	//
	// This routine does not activate the newly created set.  If you 
	// want to do this pass the returned setID to MoreSCSetCurrentSet.

extern pascal OSStatus MoreSCMakeNewPPPoESet(
									CFStringRef					bsdName, 
									CFStringRef					userVisibleName, 
									const MoreSCPPPDigest * 	pppSettings, 
									const MoreSCIPv4Digest *	ipv4Settings,
									const MoreSCDNSDigest *		dnsSettings,
									CFStringRef *				newSetID);
	// Creates a new PPPoE set and configures it appropriately.
	// 
	// Pass NULL to bsdName to use first Ethernet port (note that this is 
	// real Ethernet ports; AirPort ports aren't considered Ethernet); 
	// alternatively pass the name of the Ethernet port you want the set to 
	// reference. You can get this name from the kMoreSCPSPropertyDevice 
	// property of the list of ports returned by MoreSCCreatePortArray.
	//
	// You must not pass NULL to userVisibleName.  This is the user-visible 
	// name of the newly created set.
	//
	// You can't pass NULL to pppSettings.  You should at least fill out 
	// the active and authName fields.  You probably also want to fill out 
	// the authPassword field (in plain text).  If you wish to connect to 
	// a specific PPPoE service, you should set commRemoteAddress to that 
	// name; otherwise NULL is OK.
	//
	// Pass NULL to ipv4Settings to get default.  Typically this is just 
	// fine for a PPPoE setup.
	//
	// Pass NULL to dnsSettings to get default.  Typically this is what 
	// you want to do unless your PPP server doesn't support RFC 1877, 
	// in which case you'll need to fill out the serverAddressesCount and 
	// serverAddresses fields.
	//
	// Pass NULL to setID if you don't care what set was created.  Otherwise,
	// on input, *setID must be NULL. On error, *setID is always NULL (and 
	// no set is created). On success, *setID is the set ID of the newly 
	// created set, which you must release.
	//
	// This routine does not activate the newly created set.  If you 
	// want to do this pass the returned setID to MoreSCSetCurrentSet.
	
/////////////////////////////////////////////////////////////////

#ifdef __cplusplus
}
#endif
