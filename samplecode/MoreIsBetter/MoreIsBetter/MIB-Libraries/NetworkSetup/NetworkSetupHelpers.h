/*
	File:		NetworkSetupHelpers.h

	Contains:	High-level network preference routines.

	Written by:	Quinn

	Copyright:	Copyright (c) 1998-2001 by Apple Computer, Inc., All Rights Reserved.

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

$Log: NetworkSetupHelpers.h,v $
Revision 1.20  2002/11/25 17:28:32         
Tried to use my own code after a year away, and discovered that I needed more comments.

Revision 1.19  2002/11/09 00:16:39         
Convert nil to NULL.

Revision 1.18  2001/11/07 15:56:23         
Tidy up headers, add CVS logs, update copyright.


        <17>     21/9/01    Quinn   Changes for CWPro7 Mach-O build.
        <16>      8/2/01    Quinn   [2487952] Support Remote Access "launch status app" preference.
        <15>     11/4/00    Quinn   Add support for getting and setting "below IP" preferences.  Fix
                                    up some comments.
        <14>     18/1/00    Quinn   Further updates for the new Network Setup header file.
        <13>     17/1/00    Quinn   Updates for latest Network Setup headers.
        <12>     7/10/99    Quinn   Comments to cover common "gotcha" cases, thanks to John Norstad.
        <11>     22/9/99    Quinn   Fixed two 'backward branches' in the comments.  Thanks to John
                                    Norstad.
        <10>     13/9/99    Quinn   Implement DHCPRelease.
         <9>     26/5/99    Quinn   Tidied up C++ #if's.  Added cookie4 to NSHConfigurationEntry to
                                    hold protocol type, and thus removed the protocol parameter from
                                    NSHSelectConfiguration.  Improved comments.
         <8>      7/5/99    Quinn   This "redux" word, I don't think it means what you think it
                                    means.
         <7>     22/4/99    Quinn   Added MNSEncodeRemotePassword.
         <6>     21/4/99    Quinn   Added interface to routines which create, duplicate, get, set,
                                    delete and rename configurations for TCP/IP, Remote Access, and
                                    Modem.
         <5>    10/11/98    Quinn   Convert "MorePrefix.h" to "MoreSetup.h".
         <4>     9/11/98    Quinn   AppleTalk on/off support.
         <3>     9/11/98    Quinn   Add "TCP will dial" code.
         <2>     5/11/98    Quinn   Fix header.
         <1>     5/11/98    Quinn   First checked in.
*/

#pragma once

/////////////////////////////////////////////////////////////////
// MoreIsBetter Setup

#include "MoreSetup.h"

/////////////////////////////////////////////////////////////////
// Mac OS Interfaces

#if ! MORE_FRAMEWORK_INCLUDES
	#include <MacTypes.h>
	#include <NetworkSetup.h>
	#include <OpenTransport.h>
	#include <OpenTransportProviders.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

/////////////////////////////////////////////////////////////////
// Routines to Enumerate and Switch TCP/IP and AppleTalk Preferences

struct NSHConfigurationEntry {
	Str255 	name;					// user-visible name of the configuration
	Boolean selected;				// true if this configuration is currently active
	SInt16 	cookie;					// cookies for use by this library
	CfgEntityRef cookie2;
	CfgEntityInfo cookie3;
	OSType	cookie4;
};
typedef struct NSHConfigurationEntry NSHConfigurationEntry;
typedef NSHConfigurationEntry *NSHConfigurationListPtr;
typedef NSHConfigurationListPtr *NSHConfigurationListHandle;

extern pascal ItemCount NSHCountConfigurationList(NSHConfigurationListHandle configList);
	// Returns a count of the number of entries in configList.
	
extern pascal OSStatus  NSHGetConfigurationList(OSType protocol, NSHConfigurationListHandle configList);
	// Reads the list of configurations for the specific protocol,
	// placing a NSHConfigurationEntry in configList for each one.
	// configList must be a non-locked handle allocated by you.
	// The routine will resize it appropriate.  Specify protocol
	// using one of the constants declared in "NetworkSetup.h".
	// Currently handles the following protocols:
	//   o kOTCfgTypeTCPv4
	//   o kOTCfgTypeAppleTalk
	//   o kOTCfgTypeRemote
	//   o kOTCfgTypeModem

extern pascal OSStatus  NSHSelectConfiguration(const NSHConfigurationEntry *chosenEntry);
	// Given an entry from the configList generated by the previous
	// routine, this routine switches the network protocol to use
	// that configuration.
	//
	// Note: In earlier versions of this sample, this routine took a
	// "protocol" parameter.  It no longer needs this parameter because
	// the protocol is now stored in one of the cookies in the
	// NSHConfigurationEntry.

/////////////////////////////////////////////////////////////////
// Routines for Internet Setup Software

// ----- TCP/IP -----

struct NSHTCPv4ConfigurationDigest {
	OSType				fProtocol;			// always kOTCfgTypeTCPv4 for NSHTCPv4ConfigurationDigest
	Str255 				fConfigName;		// user-visible name of the configuration
	OTPortRef			fPortRef;			// OT port for the config (lookup �ddp1� for MacIP)
											// "Connect via" in the UI
	OTCfgTCPConfigMethod fConfigMethod;		// one of the constants from "NetworkSetup.h"
											// "Configure:" in the UI
											// For "PPP Server", use kOTCfgManualConfig with fIPAddress of 0
	InetHost			fIPAddress;			// IP address
	InetHost			fSubnetMask;		// IP subnet mask
	Handle				fRouterList;		// array of InetHost
	Handle				fDNSServerList;		// list of DNS servers, array of InetHost
	Str255				fLocalDomain;		// the local domain for this machine
											// "Implicit Search Path: Starting domain name:" in the UI
	Str255				fAdminDomain;		// "Implicit Search Path: Ending domain name:" in the UI
	Handle				fSearchDomains;		// STR# format
	Str32				fAppleTalkZone;		// for MacIP only, specify empty string otherwise
	UInt32				fFraming;			// framing attributes for this port
											// "Use 802.3" in the UI
											// Only for port with device type kOTEthernetDevice
											// - use kOTFramingEthernet by default
											// - use kOTFraming8022 if the 802.3 checkbox is set
											// Set to 0 for non-Ethernet ports
	OTCfgTCPUnloadAttr	fUnloadAttr;		// one of the constants from "NetworkSetup.h"
	Handle				fBelowIP;			// list of modules to insert below IP, 'STR#' format
	Str255				fHintUserVisiblePortName;	// Hints to find the port name if fPortRef is kOTInvalidPortRef
	Str63				fHintPortName;
	Str63				fHintDriverName;
	UInt16				fHintDeviceType;
};
typedef struct NSHTCPv4ConfigurationDigest NSHTCPv4ConfigurationDigest;

extern Boolean NSHHaveMultipleBelowIPSupport();
	// Returns true if the active system has support for more than one
	// module below IP.

// ----- Remote Access -----

struct NSHRemoteConfigurationDigest {
	OSType				fProtocol;					// always kOTCfgTypeRemote for NSHRemoteConfigurationDigest
	Str255 				fConfigName;				// user-visible name of the configuration
	
	// Main panel in UI
	
	Boolean				fGuestLogin;				// 
	Boolean				fPasswordValid;				// 
	Str255				fUserName;					// only valid if fGuestLogin is false
	Str255				fPassword;					// only valid if fGuestLogin is false and fPasswordValid is true
	Str255				fPhoneNumber;				//
	
	// Dialling tab in UI
	
	OTCfgRemoteRedialMode fRedialMode;				// one of the constants in "NetworkSetup.h"
	UInt32				fRedialTimes;				// only valid if fRedialOptions != kRemoteRedialNone
	UInt32				fRedialDelay;				// (ms) only valid if fRedialOptions != kRemoteRedialNone
	Str255				fAlternatePhoneNumber;		// only valid if fRedialOptions == kRemoteRedialMainAndAlternate

	// Connection tab in UI
	
	Boolean				fVerboseLogging;			//
	Boolean				fFlashIconWhileConnected;	//
	Boolean				fPromptToRemainConnected;	//
	UInt32				fPromptInterval;			// (minutes)
	Boolean				fDisconnectIfIdle;			//
	UInt32				fDisconnectInterval;		// (ms)
	Boolean				fLaunchStatusApp;			// ignored prior to Remote Access 3.5
	
	// Protocol tab in UI
	
	OTCfgRemoteProtocol	fSerialProtocol;			// one of the constants in "NetworkSetup.h"
	Boolean				fPPPConnectAutomatically;			// only valid if fSerialProtocol == kRemoteProtocolPPP
	Boolean				fPPPAllowModemCompression;			// only valid if fSerialProtocol == kRemoteProtocolPPP
	Boolean				fPPPAllowTCPIPHeaderCompression;	// only valid if fSerialProtocol == kRemoteProtocolPPP
	OTCfgRemotePPPConnectScript fPPPConnectMode;			// only valid if fSerialProtocol == kRemoteProtocolPPP, one of the constants from "NetworkSetup.h"
	Str63				fPPPConnectScriptName;				// only valid if fPPPConnectMode == kOTCfgRemotePPPConnectScriptScript
	Handle				fPPPConnectScript;					// only valid if fPPPConnectMode == kOTCfgRemotePPPConnectScriptScript
};
typedef struct NSHRemoteConfigurationDigest NSHRemoteConfigurationDigest;

// ----- Modem -----

struct NSHModemConfigurationDigest {
	OSType					 fProtocol;			// always kOTCfgTypeModem for NSHModemConfigurationDigest
	Str255 					 fConfigName;		// user-visible name of the configuration
	OTPortRef				 fPortRef;			// reference to OT serial port
	FSSpec 					 fModemScript;		// reference to CCL file
	OTCfgModemDialogToneMode fDialToneMode;		// one of the constants in "NetworkSetup.h"
	Boolean 				 fSpeakerOn;			// turn on speaker
	Boolean 				 fPulseDial;			// use pulse dial
	Str63 					 fHintPortName;		// Hint to find the port name if fPortRef is kOTInvalidPortRef
};
typedef struct NSHModemConfigurationDigest NSHModemConfigurationDigest;

// ----- Generic -----

// NSHConfigurationDigestCommon describes the fields that must
// be at the front of all protocol-specific configuration digest
// records.

struct NSHConfigurationDigestCommon {
	OSType				fProtocol;			// protocol of the configuration
	Str255 				fConfigName;		// user-visible name of the configuration
};
typedef struct NSHConfigurationDigestCommon NSHConfigurationDigestCommon;

// NSHConfigurationDigest is a union of all the protocol-specific
// configuration digest records defined above.

union NSHConfigurationDigest {
	NSHConfigurationDigestCommon fCommon;
	NSHTCPv4ConfigurationDigest  fTCPv4;
	NSHRemoteConfigurationDigest fRemote;
	NSHModemConfigurationDigest  fModem;
};
typedef union NSHConfigurationDigest NSHConfigurationDigest;

extern pascal OSStatus NSHCreateConfiguration(const NSHConfigurationDigest *configurationDigest,
												NSHConfigurationEntry *createdConfig);
	// Creates a new connection entity, populating it with the information
	// in configurationDigest.  If createdConfig is not NULL, it is filled out with
	// the information needed to choose this connection using NSHSelectConfiguration.
	// You can pass NULL to createdConfig if you're not interested in it.
	// For Handle fields in configurationDigest, a value of NULL implies that it
	// should take the default.
	//
	// When you create a configuration, you must provide a real OTPortRef for
	// any fPortRef fields in the digest.  However, you do not need to set up
	// the "hint" fields of the digest.
	//
	// You supply any password fields in the digest in plain text form.

extern pascal OSStatus NSHDuplicateConfiguration(NSHConfigurationEntry *config,
												 ConstStr255Param newConfigName,
												 NSHConfigurationEntry *createdConfig);
	// Duplicates the configuration specified by config, creating a new
	// configuration returned in createdConfig, whose name is newConfigName.
	// You can pass NULL to createdConfig if you're not interested in it.
	//
	// These is a significant difference between duplication and create.
	// When you duplicate a config, you get a copy of its preferences,
	// whether they are known to this library or not.  The only difference
	// is the configuration name.  When you create, the library just sets up the
	// preferences it knows about.
	//
	// Note that making newConfigName unique is your problem.  Typically
	// you do this by appending " copy" and then " copy X" until it's unique,
	// handling the case where the name already ends in " copy" or " copy X".
	// The caller has to do this because the intricacies are beyond the
	// scope of this library.

extern pascal OSStatus NSHGetConfiguration(const NSHConfigurationEntry *config,
												NSHConfigurationDigest *configurationDigest);
	// Returns in configurationDigest the information about the configuration
	// specified by config.
	//
	// YOU MUST FILL OUT SOME FIELDS IN configurationDigest!!!
	//
	// Specifically, for Handle fields, you must either NULL out the field or
	// put an empty Handle in the field.  If you don't know the protocol
	// associated with config, you don't know which fields are handles,
	// so the only safe thing to do is zero configurationDigest.
	//
	// Also, for protocols which return an fPortRef, you should note that
	// the returned fPortRef may be kOTInvalidPortRef.  This happens where
	// a configuration is created, the port is removed (eg a PC Card is ejected)
	// and you subsequently read the configuration.  You have to handle this
	// case.  Typically, the protocol-specific digest will include hint fields
	// that allow you to search the OT port registry for some other suitable port.
	// I only mention it here because you might think that NSHSetConfiguration
	// will always work if you pass it a digest generated by NSHGetConfiguration,
	// but that's not true.
	//
	// Finally, this routine returns passwords in their encoded form.  This
	// is a design decision.  If you want the decoded password, you have to
	// decode it yourself.

extern pascal OSStatus NSHSetConfiguration(const NSHConfigurationEntry *config,
												const NSHConfigurationDigest *configurationDigest);
	// Sets the configuration specified by config to the information in 
	// configurationDigest.  For Handle fields in configurationDigest, a value
	// of NULL implies that this routine should not affect the current value.
	// 
	// When you set a configuration, you must provide a real OTPortRef for
	// any fPortRef fields in the digest.  However, you do not need to set up
	// the "hint" fields of the digest.
	//
	// You supply any password fields in the digest in plain text form.

extern pascal OSStatus NSHDeleteConfiguration(const NSHConfigurationEntry *config);
	// Deletes the configuration specified by config.  Deleting an active
	// configuration yields an error.
	
extern pascal OSStatus NSHRenameConfiguration(const NSHConfigurationEntry *config,
											  ConstStr255Param newConfigName,
											  NSHConfigurationEntry *newConfig);
	// Renames the configuration specified by config to the name
	// newConfigName.  newConfig is the returned as a reference
	// to the newly renamed configuration; you can pass NULL if you're not
	// interested in it.

/////////////////////////////////////////////////////////////////
// TCP/IP Will Dial

enum {
	kNSHTCPDialUnknown = 0,
	kNSHTCPDialTCPDisabled,
	kNSHTCPDialYes,
	kNSHTCPDialNo
};

extern pascal OSStatus NSHTCPWillDial(UInt32 *willDial);
	// This routine returns, in willDial, a flag indicating
	// whether opening a TCP/IP provider will cause the modem 
	// to dial.  You must call InitOpenTransport before calling
	// this routine.  [Not because it allocates memory but
	// because it calls OTInetGetInterfaceInfo.]


/////////////////////////////////////////////////////////////////
// DHCP Release

extern pascal OSStatus NSHDHCPRelease(void);
	// This routine lets you force the OT TCP/IP stack to release
	// any address it has aquired from a DHCP server and attempt
	// to acquire a new address from scratch.
	//
	// The implementation is somewhat experimental.  Please
	// let me know if you have problems.  Also, the non-database
	// implementation is completely untested.

/////////////////////////////////////////////////////////////////
// AppleTalk On/Off

extern pascal OSStatus NSHIsAppleTalkActive(Boolean *active);
	// Sets *active to true if the AppleTalk stack is active.

extern pascal OSStatus HSHSetAppleTalkActive(Boolean active);
	// Sets the active state of the AppleTalk stack to the active
	// parameter.

/////////////////////////////////////////////////////////////////
// Remote Access Password Encode

extern pascal OSStatus NSHEncodeRemotePassword(
								ConstStr255Param userName,
								ConstStr255Param password,
								Str255 encodedPassword);
	// Encodes a password to be compatible with the ARA
	// 'pass' preference format.  Note that NSHCreateConfiguration and
	// NSHSetConfiguration will automagically encode (using this routine)
	// ARA passwords for you; this routine is exposed as a convenience of you
	// need to encode passwords for some other reason.

/////////////////////////////////////////////////////////////////

#ifdef __cplusplus
}
#endif
