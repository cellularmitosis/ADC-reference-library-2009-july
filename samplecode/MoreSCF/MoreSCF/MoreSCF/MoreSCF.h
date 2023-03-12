/*
	File:		MoreSCF.h

	Contains:	System Configuration framework high-level API.

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

$Log: MoreSCF.h,v $
Revision 1.8  2006/03/27 14:42:07         
Eliminate high-bit set characters.

Revision 1.7  2006/03/24 16:15:21         
Tidy headers: eliminate #pragma once, check C++ guards, eliminate bogus pragmas.

Revision 1.6  2006/03/24 15:44:14         
Updated copyright.

Revision 1.5  2006/03/24 12:38:22         
Eliminate "pascal" keyword.

Revision 1.4  2006/03/24 11:29:47         
Eliminated "MoreSetup.h" to make it easier for folks to copy MIB source into their projects.

Revision 1.3  2002/11/09 00:00:48         
Added compile time environment check. When using framework includes, explicitly include the frameworks we need. Convert nil to NULL.

Revision 1.2  2002/01/22 06:21:07         
Changed API of MoreSCNewService to use a dictionary, which makes for a better match with the results from MoreSCFPortScanner. Also added a lot of comments about the meaning of various dictionary keys.

Revision 1.1  2002/01/16 22:52:21         
First checked in.


*/

#ifndef _MORESCF_H
#define _MORESCF_H

/////////////////////////////////////////////////////////////////

// System prototypes

#include <SystemConfiguration/SystemConfiguration.h>

/////////////////////////////////////////////////////////////////

#ifdef __cplusplus
extern "C" {
#endif

/////////////////////////////////////////////////////////////////
// Trivial Utilities

extern UInt32 MoreSCGetSystemVersion(void);
    // Returns the system version from gestaltSystemVersion.

extern OSStatus MoreSCToOSStatus(int scErr);
	// Maps an SCF error (positive integers defined in 
	// <SystemConfiguration/SystemConfiguration.h>) to an 
	// OSStatus.  Currently we just cast one to the other 
	// (so this routine returns a positive OSStatus that 
	// has the same value as scErr) but, hey, one day I 
	// might come up with an appropriate mapping.
	
extern OSStatus MoreSCErrorBoolean(Boolean mustBeTrue);
	// If mustBeTrue is true, this routine returns noErr.
	// If mustBeTrue is false, it calls SCError (to get the 
	// most recent SCF error), maps it to an OSStatus using 
	// MoreSCToOSStatus, and returns it.  Typically you use 
	// this routine in concert with an SCF routine that returns 
	// a Boolean result (for example, SCPreferencesPathSetValue).

extern OSStatus MoreSCError(const void *value);
	// If value is not NULL, this routine returns noErr.
	// If value is NULL, it calls SCError (to get the 
	// most recent SCF error), maps it to an OSStatus using 
	// MoreSCToOSStatus, and returns it.  Typically you use 
	// this routine in concert with an SCF routine that returns 
	// a pointer result (for example, SCPreferencesPathGetValue).

/////////////////////////////////////////////////////////////////
// Open/Close

// Sequencing
// ----------
// The correct sequence to call these routines is as follows.
//
// (void) MoreSCSetClient(CFSTR("MyClientName")); -- optional
// 
// err = MoreSCOpen(false, false);
// if (err == noErr) {
//     MoreSCGetSCPreferencesRef and do stuff with it
// }
// MoreSCClose(&err, dirty);
//
// You must call MoreSCSetClient first so that the client name is set 
// up before the connection to SCPreferences is created.  You must 
// always  call MoreSCClose, even if the MoreSCOpen failed.  You must 
// not call MoreSCGetSCPreferencesRef outside of the context of 
// a MoreSCOpen/MoreSCClose pair.
//
// It's Optional
// -------------
// All of the MoreSCF public routines will automatically open 
// and close a connection to SCPreferences if you haven't done 
// so already.  The primary reason why you might use 
// MoreSCOpen/MoreSCClose are:
//
// o You want to enforce a locking policy (see below).
//
// o You want to get access to the SCPreferencesRef so that you 
//   can call SCPreferences routines that aren't adequately 
//   wrapped by more MoreSCF.
// 
// Locking
// -------
// By default MoreSCF does *no* locking at all.  Whenever 
// MoreSCF calls MoreSCOpen internally, it passes (false, false) as 
// the parameters.  If you want to do locking (and this 
// is advisable in many circumstances), you must call MoreSCOpen 
// yourself and pass appropriate locking parameters.
//
// Locking is important even if you're calling high-level routines. 
// For example, let's say you're a classic ISP setup program.  
// You call MoreSCFindSetByUserVisibleNameAndCopyID to see whether 
// a specific named set currently exists; if it doesn't, you create it 
// using MoreSCMakeNewDialupSet.  [Both of these routines are defined 
// in "MoreSCFHelpers.h".] To do this safely you should get the 
// SCPreferences lock before calling MoreSCFindSetByUserVisibleNameAndCopyID 
// and release it after calling MoreSCMakeNewDialupSet.  Otherwise its 
// remotely possible that someone could create the set between your 
// calls to MoreSCFindSetByUserVisibleNameAndCopyID and 
// MoreSCMakeNewDialupSet.
//
// The sequence you would use is something like:
//
// Boolean dirty;
//
// dirty = false;
// err = MoreSCOpen(true, true);
// if (err == noErr) {
//     MoreSCFindSetByUserVisibleNameAndCopyID(...);
//     if (! found) {
//         MoreSCMakeNewDialupSet(...);
//         dirty = true;
//     }
// }
// MoreSCClose(&err, dirty);

extern OSStatus MoreSCSetClient(CFStringRef clientName);
	// Sets the client name that MoreSCF passes to routines 
	// like SCPreferencesCreate.  By default this name is 
	// "MoreSCF".  You can override this by calling this routine 
	// and passing in a more appropriate name.
	//
	// clientName can be NULL, in which case MoreSCF returns to 
	// using its default value.
	//
	// clientName is retained by MoreSCF.

extern CFStringRef MoreSCGetClient(void);
	// Gets the client name currently being used by MoreSCF. 
	// If you haven't set a client name using MoreSCSetClient, this 
	// will return NULL.

extern OSStatus MoreSCOpen(Boolean lockThePrefs, Boolean waitForLock);
	// Opens a connection to the SCPreferences.  All of MoreSCF 
	// shares the same actual connection, however the connection 
	// is reference counted so that you can call MoreSCOpen 
	// multiple times without causing any confusion.
	// 
	// If you pass true to lockThePrefs then the routine will 
	// ensure that the preferences are locked.  No other process 
	// will be able to make changes while the lock is held. 
	// The lock will be released when the SCPreferences reference 
	// count drops to 0.  This is not necessarily when you call 
	// MoreSCClose; if some other routine has called MoreSCOpen 
	// then the lock will be released when all calls to MoreSCClose 
	// have been made.  The is explained in more detail in the 
	// comments to MoreSCClose.
	//
	// You must be running as root (have an effective user ID 
	// of 0) to lock the preferences.
	//
	// If waitForLock is true then your thread will block waiting 
	// for the SCPreferences lock if some other SCPreferencesRef 
	// has it locked. If waitForLock is false then this call will 
	// fail with an error if the lock is already held.
	//
	// It is illegal to pass true to waitForLock without also 
	// passing true to lockThePrefs.

extern SCPreferencesRef MoreSCGetSCPreferencesRef(void);
	// Returns the actual SCPreferencesRef that MoreSCF is using. 
	// You should not call this routine outside of a MoreSCOpen/
	// MoreSCClose pair. 

extern void     MoreSCClose(OSStatus *err, Boolean dirty);
	// Closes a connection to the SCPreferences.  If dirty is true 
	// then this records that the preferences have been changed 
	// (assuming *err is not noErr).  If this drops the reference count 
	// to 0 then certain special actions are taken:
	//
	// o If the preferences have been changed, MoreSCF attempts to 
	//   commit those changes.  Any error occuring during the commit 
	//   is recorded in *err if *err is noErr.  If *err is not noErr, 
	//   its initial value is preserved.
	//
	// o If the preferences are locked, MoreSCF releases the lock after 
	//   committing the changes.
	//
	// Note that the dirty and locked atttributes were not necessarily 
	// set for this instance of MoreSCOpen/Close. If an enclosing instance 
	// has either dirtied or locked the preferences, the commit or unlock 
	// is delayed until the reference count hits 0.  For example:
	//
	// err = MoreSCOpen(false, false);
	// if (err == noErr) {
	//     err = MoreSCOpen(true, true);    -- lock happens here
	//     if (err == noErr) {
	//         ...
	//     }
	//     MoreSCClose(&err, true);         -- dirty recorded, but commit and unlock deferred
	// }
	// MoreSCClose(&err, false);			-- commit and unlocked happens here
	//
	// You must be running as root (have an effective user ID 
	// of 0) to commit preference changes.
	// 
	// err must not be NULL.

/////////////////////////////////////////////////////////////////
// Dictionary-Based Routines

// These routines provide access to commonly used SCF preferences. 
// They are used extensively within "MoreSCF.c", however, most of 
// the time you won't need to use them because there are higher-level 
// routines (declared in the next section, or in "MoreSCFHelpers.h") 
// that provide a more useful level of abstraction.

extern OSStatus MoreSCCopySetsDict(CFDictionaryRef *setsDict);
	// Returns a copy of the dictionary which contains all of the 
	// sets information (/Sets).
	//
	// setsDict must not be NULL.
	// On input, *setsDict must be NULL.
	// On error, *setsDict is always NULL.
	// On succces, *setsDict is set to a CFDictionaryRef.  You must 
	// release it.

extern OSStatus MoreSCSetSetsDict(CFDictionaryRef setsDict);
	// Sets the sets dictionary (/Sets) to setsDict.
	//
	// setsDict must not be NULL.

extern OSStatus MoreSCCopyServicesDict(CFStringRef  setID, 
										   CFDictionaryRef *servicesDict,
										   CFArrayRef *		serviceOrder);
	// Returns a copy of the services dictionary of the set 
	// (/Set/<setID>/Network/Service), which contains a list of 
	// all of the services in the set, or the service order array 
	// for the set (/Set/<setID>/Network/Global/IPv4/ServiceOrder), 
	// which contains information about how the services are ordered 
	// (at least as far as IPv4 is concerned).
	//
	// setID may be NULL, which implies the current set.
	//
	// If servicesDict is not NULL, the following applies.
	// On input, *servicesDict must be NULL.
	// On error, *servicesDict is always NULL.
	// On succces, *servicesDict is set to a CFDictionaryRef.  You must 
	// release it.
	//
	// sevicesDict may be NULL, which indicates that this information is 
	// not needed.
	//
	// If serviceOrder is not NULL, the following applies.
	// On input, *serviceOrder must be NULL.
	// On error, *serviceOrder is always NULL.
	// On succces, *serviceOrder is set to a CFDictionaryRef.  You must 
	// release it.
	//
	// serviceOrder may be NULL, which indicates that this information is 
	// not needed.
	//
	// You must not pass NULL for both servicesDict and serviceOrder.
	
extern OSStatus MoreSCCopyServicesDictMutable(CFStringRef  setID, 
										   CFMutableDictionaryRef *servicesDict,
										   CFMutableArrayRef *	   serviceOrder);
	// Just like MoreSCCopyServicesDict except that the returned 
	// servicesDict and serviceOrder are mutable.
	
extern OSStatus MoreSCSetServicesDict(CFStringRef setID,  
										  CFDictionaryRef servicesDict,
										  CFArrayRef 	  serviceOrder);
	// Sets the services dictionary or service order array of the set 
	// to the specified values.  See MoreSCCopyServicesDict for the paths 
	// to these items.
	//
	// setID may be NULL, which implies the current set.
	// 
	// servicesDict may be NULL, in which case that data is not set.
	// 
	// serviceOrder may be NULL, in which case that data is not set.
	
extern OSStatus MoreSCCopyEntitiesDict(CFStringRef setID, CFStringRef serviceID, 
										   CFDictionaryRef *entitiesDict);
	// Copies the entities dictionary for a particular service within 
	// a particular set.  The first step is to look up the link to the 
	// service (/Sets/<setID>/Network/Services/<serviceID>) and resolve it.
	// The dictionary referenced by the link (/NetworkServices/X/) is 
	// then returned.
	// 
	// setID may be NULL, which implies the current set.
	//
	// serviceID may be NULL, which causes the routine to return the 
	// global entities dictionary for that set (/Sets/<setID>/Network/Global).
	// Otherwise serviceID is local to the specified set.
	// 
	// entitiesDict must not be NULL.
	// On input, *entitiesDict must be NULL.
	// On error, *entitiesDict is always NULL.
	// On succces, *entitiesDict is set to a CFDictionaryRef.  You must 
	// release it.
	
extern OSStatus MoreSCCopyEntitiesDictMutable(CFStringRef setID, CFStringRef serviceID, 
										   CFMutableDictionaryRef *entitiesDict);
	// Just like MoreSCCopyEntitiesDict except that the returned 
	// entitiesDict is mutable.

extern OSStatus MoreSCSetEntitiesDict(CFStringRef setID, CFStringRef serviceID, 
										  CFDictionaryRef entitiesDict);
	// Sets the entities dictionary of the set to the specified value.  
	// See MoreSCCopyServicesDict for the paths to this items.
	//
	// setID may be NULL, which implies the current set.
	// 
	// serviceID may be NULL, which causes the routine to set the 
	// global entities dictionary for that set.  Otherwise serviceID 
	// is local to the specified set.
	//
	// entitiesDict must not be NULL.

/////////////////////////////////////////////////////////////////
// Routines to Manipulate Sets (Locations)

extern OSStatus MoreSCCopySetIDs(CFArrayRef *setIDs, CFIndex *indexOfCurrentSet);
	// Returns an array of IDs for all of the sets on the system 
	// and the index of the current set in that array.
	//
	// setIDs must not be NULL.
	// On input, *setIDs must be NULL.
	// On error, *setIDs is always NULL.
	// On succces, *setIDs is set to a CFArrayRef.  You must 
	// release it.
	//
	// indexOfCurrentSet may be NULL if you are not interested in that 
	// information.
	// If indexOfCurrentSet is not NULL then, on output, *indexOfCurrentSet 
	// contains the (zero based) index into the *setIDs array of the 
	// currently active set.

extern OSStatus MoreSCCopyUserVisibleNameOfSet(CFStringRef setID, 
													  CFStringRef *userVisibleName);
    // Returns the user-visible name of a set.  These strings are set 
    // in the dialog brought up by the Edit Location item in the 
    // Location popup in the Network preferences panel.  Typical 
    // values are "Automatic", "Home", "Work", and so on.
    //
	// setID may be NULL, which implies the current set.
	//
	// userVisibleName must not be NULL.
	// On input, *userVisibleName must be NULL.
	// On error, *userVisibleName is always NULL.
	// On succces, *userVisibleName is set to a CFStringRef.  You must 
	// release it.

extern OSStatus MoreSCCopyCurrentSet(CFStringRef *setID);
	// Returns the set ID for the current set.
	//
	// setID must not be NULL.
	// On input, *setID must be NULL.
	// On error, *setID is always NULL.
	// On succces, *setID is set to a CFStringRef.  You must 
	// release it.

extern OSStatus MoreSCSetCurrentSet(CFStringRef setID);
    // Sets the current set to the specified set ID.
    //
    // setID must not be NULL.

extern OSStatus MoreSCNewSet(CFStringRef userVisibleName,
								 	CFStringRef *newSetID);
	// Creates a new set with the specified user-visible name, 
	// returning the new set's ID in *newSetID.  Inside the 
	// set there is one service for each port found 
	// in the I/O Registry (as returned by MoreSCCreatePortArray). 
	// The services default to the same values as they would if 
	// you'd created them with the Network preference panel.
	// This means that each service is marked as active, although 
	// many services can't be started because of incomplete 
	// information. Moreover, all AppleTalk entities are inactive.
	//
	// userVisibleName must not be NULL.
	//
	// Pass NULL to newSetID if you don't care what set was created.  
	// Otherwise, on input, *newSetID must be NULL. On error, *newSetID 
	// is always NULL (and no set is created). On success, *newSetID is 
	// the set ID of the newly created set, which you must release.
	
extern OSStatus MoreSCDuplicateSet(CFStringRef setID, 
									   CFStringRef newSetUserVisibleName, 
									   CFStringRef *newSetID);
	// Duplicates the specified set and all services referenced by 
	// that set.  The new set has the specified user-visible name. 
	// The new set's ID is returned in *newSetID.
	// 
	// This is a deep duplicate. That is, the set is duplicated and then 
	// all of the services that it references are duplicated, with the 
	// new set referencing the duplicates.  We do this because the 
	// Network preferences panel is not ready to handle two sets 
	// sharing the same services.
	//
	// The duplicate inherits all properties of the original, including 
	// the active state and user-visible name of its services and entities.
	//
	// setID may be NULL, which implies the current set.
	//
	// newSetUserVisibleName may be NULL, in which case the new 
	// set has the same user-visible name as the old set.  Eventually 
	// I plan to implement a "foo copy", "foo copy 1", "foo copy 2", ...
	// algorithm, but I don't have time right now.
	//
	// Pass NULL to newSetID if you don't care what set was created.  
	// Otherwise, on input, *newSetID must be NULL. On error, *newSetID 
	// is always NULL (and no set is created). On success, *newSetID is 
	// the set ID of the newly created set, which you must release.
	
extern OSStatus MoreSCDeleteSet(CFStringRef setID);
	// Deletes the specified set and all services which it 
	// references.
	//
	// setID must not be NULL.
	//
	// setID must not be the ID of the current set.

extern OSStatus MoreSCRenameSet(CFStringRef setID, CFStringRef newSetUserVisibleName);
	// Renames the specified set to the specified user-visible name.
	//
	// setID may be NULL, which implies the current set.
	// 
	// newSetUserVisibleName must not be NULL.
	//
	// This routine makes not attempt to ensure that the new 
	// user-visible name is unique.  That's confusing to the user 
	// but not to the underlying database.

/////////////////////////////////////////////////////////////////
// Routines to Manipulate Services Within a Set

extern OSStatus MoreSCCopyServiceIDs(CFStringRef setID, 
									 		CFArrayRef *localServiceIDs, 
									  		CFArrayRef *resolvedServiceIDs);
	// Returns a list of all the services within the current set.
	//
	// *localServiceIDs is the name of the services within the 
	// set (that is, an array of all the keys in 
	// /Sets/<setID>/Network/Service), while *resolvedServiceIDs is 
	// the name of the services referenced by those services 
	// (that is, the X component of /NetworkServices/X for each 
	// serviced referenced by the set).  Right now these two arrays 
	// would be identical because there's a one-to-one mapping between 
	// the service IDs within a set and the service IDs globally.
	// However, you should request the right set of IDs depending 
	// on whether you intend to look up the services within a set 
	// or globally.
	//
	// IMPORTANT: All of the routines in MoreSCF that take a service ID 
	// also take a set ID and expect the service ID to be local to 
	// that set.  So, if you're using MoreSCF you probably should only 
	// use *localServiceIDs.
	//
	// setID may be NULL, which implies the current set.
	//
	// Pass NULL to localServiceIDs if you don't need this information.
	// Otherwise, on input, *localServiceIDs must be NULL. On error, 
	// *localServiceIDs is always NULL. On success, *localServiceIDs is 
	// a CFArray of service ID strings, which you must release.
	//
	// Pass NULL to resolvedServiceIDs if you don't need this information.
	// Otherwise, on input, *resolvedServiceIDs must be NULL. On error, 
	// *resolvedServiceIDs is always NULL. On success, *resolvedServiceIDs is 
	// a CFArray of service ID strings, which you must release.
	//
	// You must not pass NULL to both localServiceIDs and resolvedServiceIDs.
	// 
	// Both returned arrays are sorted according to their IPv4 precedence
	// (ie the service order shown in the Network control panel).
	
extern OSStatus MoreSCCopyUserVisibleNameOfService(CFStringRef setID, 
														  CFStringRef serviceID, 
														  CFStringRef *userVisibleName);
	// Returns the user-visible name of a service within a set. 
	// This is shown in the Active Network Ports panel of the 
	// Network preferences panel.  Typically values are "Internal Modem", 
	// "Built-in Ethernet" and "AirPort".
	//
	// setID may be NULL, which implies the current set.
	//
	// serviceID must not be NULL.  It is local to the specified set.
	//
	// userVisibleName must not be NULL.
	// On input, *userVisibleName must be NULL.
	// On error, *userVisibleName is always NULL.
	// On succces, *userVisibleName is set to a CFStringRef.  You must 
	// release it.

extern OSStatus MoreSCIsServiceActive(CFStringRef setID, 
											 CFStringRef serviceID, 
											 Boolean *active);
	// Returns whether the specified service is active in the 
	// saved preferences.  Note that a service may be marked as 
	// active in the preferences although it can't start up 
	// and thus isn't active in the dynamic store (for example, 
	// the Ethernet cable is unplugged).
	//
	// This setting is controlled by the checkboxes in the 
	// service list in the Active Network Ports panel of the 
	// Network preferences panel.
	//
	// setID may be NULL, which implies the current set.
	// 
	// serviceID must not be NULL.  It is local to the specified set.
	//
	// active must not be NULL. On success it is set to true if the 
	// specified service is active.

extern OSStatus MoreSCSetServiceActive(CFStringRef setID, 
											  CFStringRef serviceID, 
											  Boolean active);
	// Makes the specified service active or inactive in the 
	// preferences.  See MoreSCIsServiceActive for background 
	// about this setting.
	//
	// setID may be NULL, which implies the current set.
	// 
	// serviceID must not be NULL.  It is local to the specified set.

extern OSStatus MoreSCCopyBSDNameOfService(CFStringRef setID, 
												  CFStringRef serviceID, 
												  CFStringRef *bsdName);
	// Returns the BSD name of the specified service.  This is 
	// the name supplied in the kSCPropNetInterfaceDeviceName 
	// elements of the portDict parameter to MoreSCNewService 
	// when service is created.  Typically it is derived from 
	// the I/O Registry via the MoreSCCreatePortArray routine.
	//
	// setID may be NULL, which implies the current set.
	// 
	// serviceID must not be NULL.  It is local to the specified set.
	//
	// bsdName must not be NULL.
	// On input, *bsdName must be NULL.
	// On error, *bsdName is always NULL.
	// On succces, *bsdName is set to a CFStringRef.  You must 
	// release it.
	
extern OSStatus MoreSCNewService(CFStringRef		setID, 
										CFDictionaryRef portDict, 
									 	CFStringRef *	newServiceID);
	// Creates a new service within the specified set and 
	// returns the service ID of the newly created service. 
	// The portDict parameter is typically derived from 
	// the value return by MoreSCCreatePortArray.  It must 
	// contain suitable values for the following keys.
	//
	// o kSCPropUserDefinedName -- the user-visible name of the service
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
	//   or kSCValNetInterfaceSubTypePPPoE, only needed if  
	//   kSCPropNetInterfaceType is kSCValNetInterfaceTypePPP
	// 
	// o kSCPropMACAddress -- the MAC address for Ethernet-like devices
	//
	// o kMoreSCPropNetInterfaceHardwareVariant -- set to 
	//   kMoreSCValNetInterfaceHardwareVariantIrDACOMM to distinguish 
	//   modem/serial ports from IrDA COMM ports
	//
	// The dictionary may also contain other keys, which are ignored.
	//
	// The new service has the default entities for its service type 
	// already  created. The new service is active and most of its 
	// entities are also active (except AppleTalk and PPPoE).
	// 
	// setID may be NULL, which implies the current set.
	// 
	// userVisibleName must not be NULL.
	//
	// kind must not be NULL.
	//
	// bsdName must not be NULL.
	//
	// macAddress must be NULL if the service is PPP-like and must be 
	// valid when the service is Ethernet-like.  See 
	// MoreSCFCreateStringWithMacAddress for a description of the format.
	// 
	// Pass NULL to newServiceID if you don't care what service was created.  
	// Otherwise, on input, *newServiceID must be NULL. On error, *newServiceID 
	// is always NULL (and no service is created). On success, *newServiceID is 
	// the service ID of the newly created service local to the specified set, 
	// which you must release.
	// *** more testing needed
	
extern OSStatus MoreSCDuplicateService(CFStringRef setID, 
										   CFStringRef serviceID, 
										   CFStringRef newServiceUserVisibleName, 
										   CFStringRef *newServiceID);
	// Duplicates a service within a set.  The new service contains 
	// a copy of the original service's entities.  It also duplicates 
	// the original service's active property.  The duplicate service 
	// is at the end of the ordered list of IPv4 services, much as it 
	// would be if you'd duplicated the service in the Active Network Ports 
	// panel of the Network preferences panel.
	//
	// setID may be NULL, which implies the current set.
	//
	// serviceID must not be NULL.  It is local to the specified set.
	// 
	// newServiceUserVisibleName may be nill, in which case the new service's 
	// user-visible name is the same as the old service's.
	// I plan to implement a "foo copy", "foo copy 1", "foo copy 2", ...
	// algorithm, but I don't have time right now.
	//
	// Pass NULL to newServiceID if you don't care what service was created.  
	// Otherwise, on input, *newServiceID must be NULL. On error, *newServiceID 
	// is always NULL (and no service is created). On success, *newServiceID is 
	// the service ID of the newly created service local to the specified set, 
	// which you must release.
	
extern OSStatus MoreSCDeleteService(CFStringRef setID, CFStringRef serviceID);
	// Deletes the service within a set.  Deleting the last service 
	// for a particular port is bad.  Currently this routine does 
	// not stop you from doing this.  I know how to fix this but I 
	// haven't yet coded it.
	//
	// setID may be NULL, which implies the current set.
	//
	// serviceID must not be NULL.  It is local to the specified set.

extern OSStatus MoreSCRenameService(CFStringRef setID, 
										CFStringRef serviceID, 
										CFStringRef newServiceUserVisibleName);
	// Renames the specified service to the specified user-visible name.
	//
	// setID may be NULL, which implies the current set.
	//
	// serviceID must not be NULL.  It is local to the specified set.

/////////////////////////////////////////////////////////////////
// Routines to Manipulate Entities Within a Service

// Entities themselves don't have user-visible names, so MoreSCRenameEntity doesn't 
// make any sense.  Well, this isn't exactly true. For example, PPP entities have a 
// user-visible name (displayed in the "Service Provider" field).  However, 
// this is a pretty minor edge case so I'm ignoring it for the moment.  You can 
// set and get this name by setting and getting the entire entity dictionary 
// and just accessing the value via the kSCPropUserDefinedName key.

extern OSStatus MoreSCIsEntityActive(CFStringRef setID,
											CFStringRef serviceID,
											CFStringRef	protocol,
											Boolean *	active);
	// Determines whether the specified entitiy is active in the 
	// saved preferences.  Note that a entity may be marked as 
	// active in the preferences although it can't start up 
	// and thus isn't active in the dynamic store (for example, 
	// the Ethernet cable is unplugged).
	//
	// The Network preferences panel only displays this setting 
	// for certain protocols.  The "Connect using PPPoE" 
	// checkbox in the PPPoE tab of an Ethernet service editor 
	// reflects the state of this value for the PPP entity in 
	// an Ethernet service.  Also, the "Make AppleTalk Active" 
	// checkbox reflects this value for any AppleTalk entity.
	//
	// Note that this routine only looks at the active state of 
	// the entity; it does not consult the active state of the 
	// parent service.  If the parent service is inactive then 
	// the service won't start, regardless of its active state 
	// as returned by this routine.
	//
	// setID may be NULL, which implies the current set.
	// 
	// serviceID may be NULL, which denotes a global entity 
	// within the set.  Otherwise serviceID is local to the 
	// specified set.
	//
	// protocol must not be NULL.  Typically you would supply 
	// one of the kSCEntNetXxx constants.
	//
	// active must not be NULL. On success it is set to true if the 
	// specified entity is active.

extern OSStatus MoreSCSetEntityActive(CFStringRef setID,
											CFStringRef serviceID,
											CFStringRef	protocol,
											Boolean 	active);
	// Makes the specified entity active or inactive in the 
	// preferences.  See MoreSCIsEntityActive for background 
	// about this setting.
	//
	// setID may be NULL, which implies the current set.
	// 
	// serviceID may be NULL, which denotes a global entity 
	// within the set.  Otherwise serviceID is local to the 
	// specified set.
	//
	// protocol must not be NULL.  Typically you would supply 
	// one of the kSCEntNetXxx constants.

extern OSStatus MoreSCCopyEntity(CFStringRef setID,
									CFStringRef serviceID,
									CFStringRef	protocol,
									CFDictionaryRef *entity);
	// Returns the contents of the specified entity.
	//
	// setID may be NULL, which implies the current set.
	// 
	// serviceID may be NULL, which denotes a global entity 
	// within the set.  Otherwise serviceID is local to the 
	// specified set.
	//
	// protocol must not be NULL.  Typically you would supply 
	// one of the kSCEntNetXxx constants.
	//
	// entity must not be NULL.
	// On input, *entity must be NULL.
	// On error, *entity is always NULL.
	// On succces, *entity is set to a CFDictionaryRef.  You must 
	// release it.
	// *** more testing needed
			
extern OSStatus MoreSCCopyEntityMutable(CFStringRef setID,
									CFStringRef serviceID,
									CFStringRef	protocol,
									CFMutableDictionaryRef *entity);
	// Just like MoreSCCopyEntity except that the returned 
	// entity is mutable.
						
extern OSStatus MoreSCSetEntity(CFStringRef setID,
									CFStringRef serviceID,
									CFStringRef protocol,
									CFDictionaryRef entity);
	// Sets an entities within the specified service.  If an 
	// entity with the specified protocol does not exist 
	// it is created with the specified value (entity). 
	// If an entity with the specified protocol does exist, 
	// its value is replaced.
	// 
	// setID may be NULL, which implies the current set.
	// 
	// serviceID may be NULL, which denotes a global entity 
	// within the set.  Otherwise serviceID is local to the 
	// specified set.
	//
	// protocol must not be NULL.  Typically you would supply 
	// one of the kSCEntNetXxx constants.
	//
	// entity must not be NULL.
	// *** more testing needed

extern OSStatus MoreSCCopyEntities(CFStringRef 	   setID, 
										   CFStringRef 	   serviceID,
										   CFArrayRef *	   entityProtocols,
										   CFArrayRef *	   entityValues);
	// Returns information about all of the entities within 
	// the specified service.
	//
	// setID may be NULL, which implies the current set.
	// 
	// serviceID may be NULL, which denotes a global entity 
	// within the set.  Otherwise serviceID is local to the 
	// specified set.
	//
	// If entityProtocols is not NULL, the following applies.
	// On input, *entityProtocols must be NULL.
	// On error, *entityProtocols is always NULL.
	// On succces, *entityProtocols is set to a CFArrayRef.  You must 
	// release it.  The array elements are CFStrings that represent 
	// entity protocols types (kSCEntNetXxx).
	//
	// entityProtocols may be NULL, which indicates that this information is 
	// not needed.
	//
	// If entityValues is not NULL, the following applies.
	// On input, *entityValues must be NULL.
	// On error, *entityValues is always NULL.
	// On succces, *entityValues is set to a CFArrayRef.  You must 
	// release it.  The array elements are CFDictionaries that represent 
	// the values of each entity.
	//
	// entityValues may be NULL, which indicates that this information is 
	// not needed.
	//
	// You must not pass NULL for both entityProtocols and entityValues.
	//
	// If you request both the protocols and the values, then the 
	// elements of each array are correlated.
	//
	// Valid entities within a service are those elements of the 
	// service dictionary whose values are themselves dictionaries. 
	// This means that other elements of the dictionary, such as the 
	// user-visible name and active properties, are not returned.

// *** would be nice to have MoreSCCopyEntitiesMutable

extern OSStatus MoreSCSetEntities(CFStringRef 			setID,
										  CFStringRef 			serviceID,
										  CFArrayRef     		entityProtocols,
										  CFArrayRef 			entityValues);
	// Sets all of the entities within the specified service. 
	// Each element of the arrays is considered in turn.  If 
	// the entity of the given protocol already exists, its 
	// value is replaced.  If it doesn't exist, a new entity 
	// is created.  Any entity that exists but isn't referenced 
	// by entityProtocols is removed.
	//
	// setID may be NULL, which implies the current set.
	// 
	// serviceID may be NULL, which denotes a global entity 
	// within the set.  Otherwise serviceID is local to the 
	// specified set.
	//
	// entityProtocols must not be NULL.
	//
	// entityValues must not be NULL.
	//
	// Both arrays must have the same number of elements.
	//
	// Only valid entities in the service dictionary (as defined 
	// by MoreSCCopyEntities) are affected by this routine.  
	// Service dictionary elements that are not themselves 
	// dictionaries (such as the active property and the user 
	// visible name) are preserved.
	// *** more testing needed

extern OSStatus MoreSCNewEntity(CFStringRef 		setID, 
										CFStringRef 	serviceID,
										CFStringRef 	protocol,
										CFDictionaryRef entity);
	// Creates a new entity within the specified service.
	// 
	// setID may be NULL, which implies the current set.
	// 
	// serviceID may be NULL, which denotes a global entity 
	// within the set.  Otherwise serviceID is local to the 
	// specified set.
	//
	// protocol must not be NULL.  The entity specified by protocol 
	// must not already exist within the service.
	//
	// entity must not be NULL.
	// *** more testing needed

// As the entity key is typically a protocol (eg AppleTalk, IPv4), 
// it doesn't make sense to provide a MoreSCDuplicateEntity because 
// having two entities for the same protocol is not possible.

extern OSStatus MoreSCDeleteEntity(CFStringRef 	setID, 
										   CFStringRef	serviceID,
										   CFStringRef	protocol);
	// Deletes an entity within the specified service.
	// 
	// setID may be NULL, which implies the current set.
	// 
	// serviceID may be NULL, which denotes a global entity 
	// within the set.  Otherwise serviceID is local to the 
	// specified set.
	//
	// protocol must not be NULL.  The entity specified by protocol 
	// must exist within the service.
	// *** more testing needed

/////////////////////////////////////////////////////////////////

#ifdef __cplusplus
}
#endif

#endif
