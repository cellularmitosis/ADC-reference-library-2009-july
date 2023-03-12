/*
	File:		NetworkSetupTest.c

	Contains:	A simple test program for the NetworkSetupHelpers library.

	Written by:	Quinn

	Copyright:	Copyright (c) 1998-2001 by Apple Computer, Inc., All Rights Reserved.

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

$Log: NetworkSetupTest.c,v $
Revision 1.21  2002/11/09 00:19:22         
Convert nil to NULL. Convert MoreAssertQ to assert.

Revision 1.20  2001/11/07 15:56:26         
Tidy up headers, add CVS logs, update copyright.


        <19>      8/2/01    Quinn   [2487952] Test Remote Access "launch status app" preference
                                    support.
        <18>    22/11/00    Quinn   Switch to "MacTypes.h". Also switch to "MacErrors.h".
        <17>     12/4/00    Quinn   Add rudimentary code to test below IP support.
        <16>     20/3/00    Quinn   Changes for UI 3.3.1 (approaching final).
        <15>      7/3/00    Quinn   Cosmetic change to test CodeWarrior Projector edge case.
        <14>     18/1/00    Quinn   Further updates for the new Network Setup header file.
        <13>     17/1/00    Quinn   Updates for latest Network Setup headers and Universal
                                    Interfaces 3.3.
        <12>    19/10/99    Quinn   Fix embarrassing spelling error.
        <11>     13/9/99    Quinn   Added code to test DHCPRelease.
        <10>     26/5/99    Quinn   Accomodate changes in NSHSelectConfiguration.  Fix serial port
                                    finding code to work on 8.6.
         <9>      7/5/99    Quinn   This "redux" word, I don't think it means what you think it
                                    means.
         <8>     22/4/99    Quinn   Added test cases for configuration creation and Remote Access
                                    password encoding.
         <7>     21/4/99    Quinn   Completely rewrote to support interactive and batch tests and to
                                    test the new functionality.
         <6>    10/11/98    Quinn   Convert "MorePrefix.h" to "MoreSetup.h".
         <5>     9/11/98    Quinn   AppleTalk on/off support.
         <4>     9/11/98    Quinn   Add “TCP will dial” code.
         <3>     5/11/98    Quinn   Use MoreAssertQ instead of MoreAssert.
         <2>     5/11/98    Quinn   Fix headers.
         <1>     5/11/98    Quinn   First checked in.
*/

/////////////////////////////////////////////////////////////////
// MoreIsBetter Setup

#include "MoreSetup.h"

/////////////////////////////////////////////////////////////////
// Mac OS Interfaces

#include <MacTypes.h>
#include <Gestalt.h>
#include <NetworkSetup.h>
#include <PLStringFuncs.h>
#include <OpenTransportProviders.h>
#include <Folders.h>

/////////////////////////////////////////////////////////////////
// Standard C Interfaces

#include <string.h>
#include <stdio.h>

/////////////////////////////////////////////////////////////////
// MIB Interfaces

#include "MoreNetworkSetup.h"
#include "NetworkSetupHelpers.h"
#include "MoreMemory.h"
#include "MoreTextUtils.h"

/////////////////////////////////////////////////////////////////
#pragma mark ----- Command Helpers -----

static OSType GetProtocolType(void)
{
	char commandStr[256];
	OSType protocol;
	
	printf("a) AppleTalk\n");
	printf("m) Modem\n");
	printf("r) Remote Access\n");
	printf("t) TCP/IP\n");
	printf("\n");
	printf("Choose a protocol:\n");
	gets(commandStr);
	switch (commandStr[0]) {
		case 'a': protocol = kOTCfgTypeAppleTalk; break;
		case 'm': protocol = kOTCfgTypeModem;     break;
		case 'r': protocol = kOTCfgTypeRemote;    break;
		case 't': protocol = kOTCfgTypeTCPv4;     break;
		default:
			printf("Huh?\n");
			protocol = 0;
			break;
	}
	printf("\n");
	return protocol;
}

static const char *GetProtocolName(OSType protocol)
{
	const char *result;
	
	switch (protocol) {
		case kOTCfgTypeAppleTalk: result = "AppleTalk"; break;
		case kOTCfgTypeModem:     result = "Modem";     break;
		case kOTCfgTypeRemote:    result = "Remote";    break;
		case kOTCfgTypeTCPv4:     result = "TCP/IP";    break;
		default:
			assert(false);
			break;
	}
	return result;
}

const char gSelectedChars[2] = " *";

static OSStatus ListAndChooseConfiguration(OSType *protocol, NSHConfigurationEntry *chosenEntry, const char *prompt)
	// If chosenEntry is NULL, this routine just lists the configurations.
{
	OSStatus err;
	NSHConfigurationListHandle configList;
	ItemCount i;
	char commandStr[256];
	SInt8 s;
	
	configList = NULL;
	
	err = noErr;
	*protocol = GetProtocolType();
	if (*protocol == 0) {
		err = userCanceledErr;
	}
	
	// Build and print a configuration list for that protocol.
	
	if (err == noErr) {
		configList = (NSHConfigurationListHandle) NewHandle(0);
		err = MemError();
		
		if (err == noErr) {
			printf("Building configuration list...\n\n");
			assert(configList != NULL);			
			err = NSHGetConfigurationList(*protocol, configList);
		}
		if (err == noErr) {
			s = HGetState( (Handle) configList);
			HLock( (Handle) configList );
			for (i = 0; i < NSHCountConfigurationList(configList); i++) {
				printf("%c) %c “%#s”\n", (char) ('a' + i), gSelectedChars[(*configList)[i].selected], (*configList)[i].name);
			}
			printf("\n");
			HSetState( (Handle) configList, s);
			
			if (chosenEntry != NULL) {
				printf("Choose a configuration to %s:\n", prompt);
				gets(commandStr);
				if ( commandStr[0] >= 'a' && commandStr[0] < ('a' + NSHCountConfigurationList(configList)) ) {
					*chosenEntry = (*configList)[commandStr[0] - 'a'];
				} else {
					printf("Huh?\n");
					err = userCanceledErr;
				}
			}
		} 
	}
	
	// Clean up.
	
	if ( configList != NULL ) {
		DisposeHandle( (Handle) configList );
		assert(MemError() == noErr);			
	}

	return err;
}

/////////////////////////////////////////////////////////////////
#pragma mark ----- Interactive Commands -----

static void PrintHelp(void)
{
	printf("l) List configurations\n");
	printf("s) Set active configuration\n");
	printf("d) Duplicate configuration\n");
	printf("D) Delete configuration\n");
	printf("r) Rename configuration\n");
	printf("p) Print configuration\n");
	printf("c) Create configuration\n");
	printf("w) Will TCP/IP dial if you open an endpoint\n");
	printf("A) AppleTalk ON\n");
	printf("a) AppleTalk OFF\n");
	printf("å) Is AppleTalk active?\n");
	printf("e) Try out Remote Access password encoding\n");
	printf("L) Release DHCP address\n");
	printf("b) Below IP tests\n");
	printf("0) Automated tests?\n");
	printf("?) Help\n");
	printf("q) Quit\n");
	printf("\n");
}

static void DoListConfigurations(void)
{
	OSStatus err;
	OSType protocol;

	err = ListAndChooseConfiguration(&protocol, NULL, NULL);

	if (err != noErr) {
		printf("Failed with error %ld.\n", err);
	}
}

static void DoSetConfiguration(void)
{
	OSStatus err;
	OSType protocol;
	NSHConfigurationEntry chosen;
	
	err = ListAndChooseConfiguration(&protocol, &chosen, "make current");
	if (err == noErr) {
		err = NSHSelectConfiguration(&chosen);
	} 

	if (err != noErr) {
		printf("Failed with error %ld.\n", err);
	}
}

static void DoDuplicateConfiguration(void)
{
	OSStatus err;
	OSType protocol;
	NSHConfigurationEntry chosen;
	Str255 newName;
	
	err = ListAndChooseConfiguration(&protocol, &chosen, "duplicate");
	if (err == noErr) {
		PLstrcpy(newName, chosen.name);
		PLstrcat(newName, "\p copy");
		err = NSHDuplicateConfiguration(&chosen, newName, NULL);
	}

	if (err != noErr) {
		printf("Failed with error %ld.\n", err);
	}
}

static void DoDeleteConfiguration(void)
{
	OSStatus err;
	OSType protocol;
	NSHConfigurationEntry chosen;
	
	err = ListAndChooseConfiguration(&protocol, &chosen, "delete");
	if (err == noErr) {
		err = NSHDeleteConfiguration(&chosen);
	} 

	if (err != noErr) {
		printf("Failed with error %ld.\n", err);
	}
}

static void DoRenameConfiguration(void)
{
	OSStatus err;
	OSType protocol;
	NSHConfigurationEntry chosen;
	Str255 newName;
	
	err = ListAndChooseConfiguration(&protocol, &chosen, "rename");
	if (err == noErr) {
		printf("Enter a new configuration name:\n");
		gets( (char *) newName);
		if (newName[0] == 0) {
			err = userCanceledErr;
		}
	}
	if (err == noErr) {
		BlockMoveData(newName, newName + 1, 255);
		newName[0] = strlen( (char *) (newName + 1));
		err = NSHRenameConfiguration(&chosen, newName, NULL);
	} 

	if (err != noErr) {
		printf("Failed with error %ld.\n", err);
	}
}

static pascal void PrintPrefIterator(OSType prefType, void *prefData, ByteCount prefSize, void *refcon)
{
	#pragma unused(prefData)
	#pragma unused(refcon)
	printf("'%4.4s' %5ld\n", &prefType, prefSize);
}

static void DoPrintConfiguration(void)
{
	#if TARGET_RT_MAC_CFM
		{
			OSStatus err;
			OSStatus err2;
			OSType protocol;
			NSHConfigurationEntry chosen;
			MNSDatabaseRef ref;

			err = ListAndChooseConfiguration(&protocol, &chosen, "print");

			// Call MoreNetworkSetup to iterate through the preferences
			// in the entity.
			
			if (err == noErr) {
				err = MNSOpenDatabase(&ref, false);
				if (err == noErr) {
					printf("Type    Size\n");
					printf("----    ----\n");

					err = MNSIterateEntity(&ref, &chosen.cookie2, PrintPrefIterator, NULL);
					
					err2 = MNSCloseDatabase(&ref, false);
					if (err == noErr) {
						err = err2;
					}
				}
			}
			
			if (err != noErr) {
				printf("Failed with error %ld.\n", err);
			}
		}
	#else
		printf("This feature is not available for 68K builds.\n");
	#endif
}

typedef enum {
	kEthernet,
	kPPP,
	kMacIP
} TCPv4PortType;

static void InitTCPv4Digest(NSHTCPv4ConfigurationDigest *digest)
{
	OTPortRecord portRec;
	TCPv4PortType portType;
	
	// Find a port to configure TCP/IP over.
	
	if (OTFindPort(&portRec, "enet")) {
		portType = kEthernet;
	} else if (OTFindPort(&portRec, "IPCP")) {
		portType = kPPP;
	} else if (OTFindPort(&portRec, "ddp1")) {
		portType = kMacIP;
	} else {
		// OK, you don't have Ethernet, you don't have PPP, and you don't
		// have AppleTalk turned on!  What sort of networking engineer are you!?!
		assert(false);
	}
	
	// Now set up each field in the digest.

	OTMemzero(digest, sizeof(*digest));
	digest->fProtocol = kOTCfgTypeTCPv4;
	PLstrcpy(digest->fConfigName, "\pTest TCPv4");
	digest->fPortRef = portRec.fRef;
	digest->fConfigMethod = kOTCfgManualConfig;
	digest->fIPAddress = 0x01020304;
	digest->fSubnetMask = 0xFFFFFF00;
	digest->fRouterList = NULL;
	digest->fDNSServerList = NULL;
	PLstrcpy(digest->fLocalDomain, "\pdts.apple.com");
	PLstrcpy(digest->fAdminDomain, "\papple.com");
	digest->fSearchDomains = NULL;
	if (portType == kMacIP) {
		PLstrcpy(digest->fAppleTalkZone, "\p*");
	} else {
		PLstrcpy(digest->fAppleTalkZone, "\p");
	}
	switch (portType) { 
		case kEthernet:
			digest->fFraming = kOTFramingEthernet;
			break;
		case kPPP:
			digest->fFraming = 0;
			break;
	}
	digest->fUnloadAttr = kOTCfgTCPActiveLoadedOnDemand;
}

static void InitRemoteDigest(NSHRemoteConfigurationDigest *digest)
{
	OTMemzero(digest, sizeof(*digest));
	digest->fProtocol = kOTCfgTypeRemote;
	PLstrcpy(digest->fConfigName, "\pTest Remote");
	digest->fGuestLogin = false;
	digest->fPasswordValid = true;
	PLstrcpy(digest->fUserName, "\pQuinn");
	PLstrcpy(digest->fPassword, "\pwomble");
	PLstrcpy(digest->fPhoneNumber, "\p123456");
	digest->fRedialMode = kOTCfgRemoteRedialMainAndAlternate;
	digest->fRedialTimes = 5;
	digest->fRedialDelay = 10 * 1000;
	PLstrcpy(digest->fAlternatePhoneNumber, "\p654321");
	digest->fVerboseLogging = true;
	digest->fFlashIconWhileConnected = true;
	digest->fPromptToRemainConnected = true;
	digest->fPromptInterval = 15;
	digest->fDisconnectIfIdle = true;
	digest->fDisconnectInterval = 30 * 60 * 1000;
	digest->fLaunchStatusApp = true;
	digest->fSerialProtocol = kOTCfgRemoteProtocolPPP;
	digest->fPPPConnectAutomatically = true;
	digest->fPPPAllowModemCompression = true;
	digest->fPPPAllowTCPIPHeaderCompression = true;
	digest->fPPPConnectMode = kOTCfgRemotePPPConnectScriptTerminalWindow;
	PLstrcpy(digest->fPPPConnectScriptName, "\p");
	digest->fPPPConnectScript = NULL;
}

static void InitModemDigest(NSHModemConfigurationDigest *digest)
{
	OSStatus err;
	OTPortRecord portRec;
	FSSpec modemScript;
	HFileInfo cpb;
	ItemCount portIndex;
	Boolean found;
	
	// Find a port to configure the modem on.
	
	found = false;
	portIndex = 0;
	do {
		if ( OTGetIndexedPort(&portRec, portIndex) ) {
			found = OTGetDeviceTypeFromPortRef(portRec.fRef) == kOTSerialDevice;
			portIndex += 1;
		} else {
			// You don't have a serial port.
			assert(false);
		}
	} while ( ! found );
	
	// Find a CCL, any CCL, first CCL.
	
	err = FindFolder(kOnSystemDisk, kModemScriptsFolderType, true, &modemScript.vRefNum, &modemScript.parID);
	if (err == noErr) {
		PLstrcpy(modemScript.name, "\p");
		cpb.ioNamePtr = modemScript.name;
		cpb.ioVRefNum = modemScript.vRefNum;
		cpb.ioFDirIndex = 1;
		cpb.ioDirID = modemScript.parID;
		err = PBGetCatInfoSync( (CInfoPBPtr) &cpb);
	}
	assert(err == noErr);
	
	// Fill out the digest.
	
	OTMemzero(digest, sizeof(*digest));
	digest->fProtocol = kOTCfgTypeModem;
	PLstrcpy(digest->fConfigName, "\pTest Modem");
	digest->fPortRef = portRec.fRef;
	digest->fModemScript = modemScript;
	digest->fDialToneMode = kOTCfgModemDialToneIgnore;
	digest->fSpeakerOn = true;
	digest->fPulseDial = true;
}

static void DoCreateConfiguration(void)
{
	OSStatus err;
	OSType protocol;
	NSHConfigurationDigest digest;
	
	protocol = GetProtocolType();
	err = noErr;
	switch (protocol) {
		case kOTCfgTypeTCPv4:
			InitTCPv4Digest(&digest.fTCPv4);
			break;
		case kOTCfgTypeRemote:
			InitRemoteDigest(&digest.fRemote);
			break;
		case kOTCfgTypeModem:
			InitModemDigest(&digest.fModem);
			break;
		default:
			err = userCanceledErr;
			break;
	}
	if (err == noErr) {
		err = NSHCreateConfiguration(&digest, NULL);
	}
	
	if (err != noErr) {
		printf("Failed with error %ld.\n", err);
	}
}

static void DoTCPWillDial(void)
{
	OSStatus err;
	UInt32 willDial;
	
	printf("Determining whether TCP/IP will dial...\n\n");
	err = NSHTCPWillDial(&willDial);
	if (err == noErr) {
		switch (willDial) {
			case kNSHTCPDialUnknown:
				printf("  Unknown.\n\n");
				break;
			case kNSHTCPDialTCPDisabled:
				printf("  TCP/IP is disabled.\n\n");
				break;
			case kNSHTCPDialYes:
				printf("  TCP/IP will dial.\n\n");
				break;
			case kNSHTCPDialNo:
				printf("  TCP/IP will not dial.\n\n");
				break;
			default:
				assert(false);
				break;
		}
	} else {
		printf("Failed with error %ld.\n", err);
	}
}

static void DoRemoteAccessEncoding(void)
{
	OSStatus err;
	char userName[256];
	char password[256];
	Str255 encodedPassword;
	UInt32 row;
	UInt32 col;
	
	printf("Enter a user name:\n");
	gets(userName);
	if (userName[0] != 0) {
		printf("Enter a password:\n");
		gets(password);
		if (password[0] != 0) {
			// Do two C to P strings
			BlockMoveData(userName, userName + 1, 255);
			userName[0] = strlen(userName + 1);
			BlockMoveData(password, password + 1, 255);
			password[0] = strlen(password + 1);
			err = NSHEncodeRemotePassword( (UInt8 *) userName, (UInt8 *) password, encodedPassword);
			if (err == noErr) {
				for (row = 0; row < 16; row++) {
					for (col = 0; col < 16; col++) {
						printf("%02X ", encodedPassword[row * 16 + col]);
					}
					printf("\n");
				}
			} else {
				printf("Failed with error %ld.\n", err);
			}
		}
	}
}

/////////////////////////////////////////////////////////////////
#pragma mark ----- Automated Tests -----

static void DoDHCPRelease(void)
{
	OSStatus err;
	
	printf("Attempting to release DHCP address...\n");
	err = NSHDHCPRelease();
	if (err == noErr) {
		printf("Success!\n");
	} else {
		printf("Failed with error %ld.\n", err);
	}
}

/////////////////////////////////////////////////////////////////
#pragma mark ----- Automated Tests -----

static void DoAppleTalkOnOff(Boolean active)
{
	OSStatus err;
	
	if (active) {
		printf("Activating AppleTalk...\n\n");
	} else {
		printf("Deactivating AppleTalk...\n\n");
	}
	err = HSHSetAppleTalkActive(active);
	if (err != noErr) {
		printf("Failed with error %ld.\n", err);
	}
}

static void DoIsAppleTalkActive(void)
{
	OSStatus err;
	Boolean activeAppleTalk;
	
	printf("Determining whether AppleTalk is active...\n\n");
	err = NSHIsAppleTalkActive(&activeAppleTalk);
	if (err == noErr) {
		if (activeAppleTalk) {
			printf("  AppleTalk active.\n\n");
		} else {
			printf("  AppleTalk inactive.\n\n");
		}
	} else {
		printf("Failed with error %ld.\n", err);
	}
}

static void DoAutomatedAppleTalkOnOffTest(void)
{
	OSStatus err;
	Boolean originalState;
	Boolean newState;

	printf("AppleTalk On/Off Suite...\n");
	printf("  getting current state...\n");
	err = NSHIsAppleTalkActive(&originalState);
	if (err == noErr) {
		printf("  switching state...\n");
		err = HSHSetAppleTalkActive( !originalState );
	}
	if (err == noErr) {
		printf("  getting new state...\n");
		err = NSHIsAppleTalkActive(&newState);
	}
	if (err == noErr) {
		if ( newState != !originalState ) {
			printf("  ••• Failed to switch AppleTalk state.\n");
		}
		printf("  restoring state...\n");
		err = HSHSetAppleTalkActive(originalState);
	}
	if (err == noErr) {
		printf("  getting state again...\n");
		err = NSHIsAppleTalkActive(&newState);
	}
	if (err == noErr) {
		if ( newState != originalState ) {
			printf("  ••• Failed to restore AppleTalk state.\n");
		}
	}
	if (err == noErr) {
		printf("Success!\n");
	} else {
		printf("Failed with error %ld.\n", err);
	}
	printf("\n");
}

static Boolean CompareHandle(Handle origH, Handle newH)
{
	return	   ((origH == NULL) && (newH == NULL))
			|| (   (origH != NULL) 
				&& (newH != NULL) 
				&& (*origH != NULL) 
				&& (*newH != NULL)
				&& (GetHandleSize(origH) == GetHandleSize(newH))
				&& MoreBlockCompare(*origH, *newH, GetHandleSize(origH))
			   );
}

static Boolean CompareTCPv4ConfigurationDigests(const NSHConfigurationDigest *origR,
											const NSHConfigurationDigest *newR)
{
	Boolean result;
	NSHTCPv4ConfigurationDigest *origDigest;
	NSHTCPv4ConfigurationDigest *newDigest;

	origDigest = (NSHTCPv4ConfigurationDigest *) origR;
	newDigest = (NSHTCPv4ConfigurationDigest *) newR;
	
	result = true;
	if (origDigest->fProtocol != newDigest->fProtocol) result = false;
	if (PLstrcmp(origDigest->fConfigName, newDigest->fConfigName) != 0) result = false;
	if (origDigest->fPortRef != newDigest->fPortRef) result = false;
	if (origDigest->fConfigMethod != newDigest->fConfigMethod) result = false;
	if (origDigest->fIPAddress != newDigest->fIPAddress) result = false;
	if (origDigest->fSubnetMask != newDigest->fSubnetMask) result = false;
	if (! CompareHandle(origDigest->fRouterList, newDigest->fRouterList)) result = false;
	if (! CompareHandle(origDigest->fDNSServerList, newDigest->fDNSServerList)) result = false;
	if (PLstrcmp(origDigest->fLocalDomain, newDigest->fLocalDomain) != 0) result = false;
	if (PLstrcmp(origDigest->fAdminDomain, newDigest->fAdminDomain) != 0) result = false;
	if (! CompareHandle(origDigest->fSearchDomains, newDigest->fSearchDomains)) result = false;
	if (PLstrcmp(origDigest->fAppleTalkZone, newDigest->fAppleTalkZone) != 0) result = false;
	if (origDigest->fFraming != newDigest->fFraming) result = false;
	if (origDigest->fUnloadAttr != newDigest->fUnloadAttr) result = false;
	if (PLstrcmp(origDigest->fHintUserVisiblePortName, newDigest->fHintUserVisiblePortName) != 0) result = false;
	if (PLstrcmp(origDigest->fHintPortName, newDigest->fHintPortName) != 0) result = false;
	if (PLstrcmp(origDigest->fHintDriverName, newDigest->fHintDriverName) != 0) result = false;
	if (origDigest->fHintDeviceType != newDigest->fHintDeviceType) result = false;

	return result;	
}

static Boolean CompareRemoteConfigurationDigests(const NSHConfigurationDigest *origR,
											const NSHConfigurationDigest *newR)
{
	Boolean result;
	NSHRemoteConfigurationDigest *origDigest;
	NSHRemoteConfigurationDigest *newDigest;

	origDigest = (NSHRemoteConfigurationDigest *) origR;
	newDigest = (NSHRemoteConfigurationDigest *) newR;
	
	result = true;

	if (origDigest->fProtocol != newDigest->fProtocol) result = false;
	if (PLstrcmp(origDigest->fConfigName, newDigest->fConfigName) != 0) result = false;
	if (origDigest->fGuestLogin != newDigest->fGuestLogin) result = false;
	if (origDigest->fPasswordValid != newDigest->fPasswordValid) result = false;
	if (PLstrcmp(origDigest->fUserName, newDigest->fUserName) != 0) result = false;
	if (PLstrcmp(origDigest->fPassword, newDigest->fPassword) != 0) result = false;
	if (PLstrcmp(origDigest->fPhoneNumber, newDigest->fPhoneNumber) != 0) result = false;
	if (origDigest->fRedialMode != newDigest->fRedialMode) result = false;
	if (origDigest->fRedialTimes != newDigest->fRedialTimes) result = false;
	if (origDigest->fRedialDelay != newDigest->fRedialDelay) result = false;
	if (PLstrcmp(origDigest->fAlternatePhoneNumber, newDigest->fAlternatePhoneNumber) != 0) result = false;
	if (origDigest->fVerboseLogging != newDigest->fVerboseLogging) result = false;
	if (origDigest->fFlashIconWhileConnected != newDigest->fFlashIconWhileConnected) result = false;
	if (origDigest->fPromptToRemainConnected != newDigest->fPromptToRemainConnected) result = false;
	if (origDigest->fPromptInterval != newDigest->fPromptInterval) result = false;
	if (origDigest->fDisconnectIfIdle != newDigest->fDisconnectIfIdle) result = false;
	if (origDigest->fDisconnectInterval != newDigest->fDisconnectInterval) result = false;
	if (origDigest->fLaunchStatusApp != newDigest->fLaunchStatusApp) result = false;
	if (origDigest->fSerialProtocol != newDigest->fSerialProtocol) result = false;
	if (origDigest->fPPPConnectAutomatically != newDigest->fPPPConnectAutomatically) result = false;
	if (origDigest->fPPPAllowModemCompression != newDigest->fPPPAllowModemCompression) result = false;
	if (origDigest->fPPPAllowTCPIPHeaderCompression != newDigest->fPPPAllowTCPIPHeaderCompression) result = false;
	if (origDigest->fPPPConnectMode != newDigest->fPPPConnectMode) result = false;
	if (PLstrcmp(origDigest->fPPPConnectScriptName, newDigest->fPPPConnectScriptName) != 0) result = false;
	if (! CompareHandle(origDigest->fPPPConnectScript, newDigest->fPPPConnectScript)) result = false;

	return result;	
}

static Boolean CompareModemConfigurationDigests(const NSHConfigurationDigest *origR,
											const NSHConfigurationDigest *newR)
{
	Boolean result;
	NSHModemConfigurationDigest *origDigest;
	NSHModemConfigurationDigest *newDigest;

	origDigest = (NSHModemConfigurationDigest *) origR;
	newDigest = (NSHModemConfigurationDigest *) newR;
	
	result = true;

	if (origDigest->fProtocol != newDigest->fProtocol) result = false;
	if (PLstrcmp(origDigest->fConfigName, newDigest->fConfigName) != 0) result = false;
	if (origDigest->fPortRef != newDigest->fPortRef) result = false;
	if (origDigest->fModemScript.vRefNum != newDigest->fModemScript.vRefNum) result = false;
	if (origDigest->fModemScript.parID != newDigest->fModemScript.parID) result = false;
	if (PLstrcmp(origDigest->fModemScript.name, newDigest->fModemScript.name) != 0) result = false;
	if (origDigest->fDialToneMode != newDigest->fDialToneMode) result = false;
	if (origDigest->fSpeakerOn != newDigest->fSpeakerOn) result = false;
	if (origDigest->fPulseDial != newDigest->fPulseDial) result = false;
	if (PLstrcmp(origDigest->fHintPortName, newDigest->fHintPortName) != 0) result = false;

	return result;
}

typedef Boolean (*CompareConfigurationDigestsProc)(const NSHConfigurationDigest *origDigest,
											const NSHConfigurationDigest *newDigest);

static OSStatus DoConfigurationDigestTestsForProtocol(OSType protocol)
{
	OSStatus err;
	OSStatus junk;
	Boolean haveCreatedConfig;
	CompareConfigurationDigestsProc compareProc;
	NSHConfigurationListHandle configList;
	NSHConfigurationEntry configToClone;
	NSHConfigurationDigest configToCloneDigest;
	NSHConfigurationEntry createdConfig;
	NSHConfigurationDigest createConfigDigest;
	
	haveCreatedConfig = false;
	
	switch (protocol) {
		case kOTCfgTypeTCPv4:
			compareProc = CompareTCPv4ConfigurationDigests;
			break;
		case kOTCfgTypeRemote:
			compareProc = CompareRemoteConfigurationDigests;
			break;
		case kOTCfgTypeModem:
			compareProc = CompareModemConfigurationDigests;
			break;
		default:
			assert(false);
			break;
	}
	
	configList = (NSHConfigurationListHandle) NewHandle(0);
	err = MemError();
	if (err == noErr) {
		printf("    building configuration list...\n");
		assert(configList != NULL);			
		err = NSHGetConfigurationList(protocol, configList);
	}
	if (err == noErr) {
		if ( NSHCountConfigurationList(configList) < 1 ) {
			printf("    ••• test can't run because there's only one configuration\n");
			err = userCanceledErr;
		} else {
			configToClone = (*configList)[0];
		}
	}
	if (err == noErr) {
		printf("    getting configuration digest...\n");
		OTMemzero(&configToCloneDigest, sizeof(configToCloneDigest));
		err = NSHGetConfiguration(&configToClone, &configToCloneDigest);
	}
	if (err == noErr) {
		printf("    creating configuration based on digest...\n");

		// Change the first character of the configuration name to
		// '†' to ensure that createdConfig won't have the same
		// name as configToClone.  In real code, we'd deploy some
		// smarted code (adding " copy X" to uniquify the name),
		// but in this simple test program we can assume that we
		// won't encounter any configurations that begin with a '†'.
		
		assert(configToCloneDigest.fCommon.fConfigName[0] > 0);
		configToCloneDigest.fCommon.fConfigName[1] = '†';
		err = NSHCreateConfiguration(&configToCloneDigest, &createdConfig);
		haveCreatedConfig = (err == noErr);
	}
	if (err == noErr) {
		printf("    getting created configuration digest...\n");
		OTMemzero(&createConfigDigest, sizeof(createConfigDigest));
		err = NSHGetConfiguration(&createdConfig, &createConfigDigest);
	}
	if (err == noErr) {
		printf("    comparing digests...\n");
		if ( compareProc(&configToCloneDigest, &createConfigDigest) ) {
			printf("    digests are equal\n");
		} else {
			printf("    ••• digests are not equal\n");
		}
	}
	if (err == noErr) {
		printf("    setting created configuration...\n");
		err = NSHSetConfiguration(&createdConfig, &configToCloneDigest);
	}
	if (err == noErr) {
		printf("    getting created configuration digest again...\n");
		OTMemzero(&createConfigDigest, sizeof(createConfigDigest));
		err = NSHGetConfiguration(&createdConfig, &createConfigDigest);
	}
	if (err == noErr) {
		printf("    comparing digests again...\n");
		if ( compareProc(&configToCloneDigest, &createConfigDigest) ) {
			printf("    digests are equal\n");
		} else {
			printf("    ••• digests are not equal\n");
		}
	}
	
	// Clean up.
	
	if (haveCreatedConfig) {
		junk = NSHDeleteConfiguration(&createdConfig);
		assert(junk == noErr);
	}
	if (configList != NULL) {
		DisposeHandle( (Handle) configList);
		assert(MemError() == noErr);
	}
	return err;
}

static void DoConfigurationDigestTests(void)
{
	OSStatus err;
	
	printf("Configuration Digest Test...\n");
	printf("  TCP/IP...\n");
	err = DoConfigurationDigestTestsForProtocol(kOTCfgTypeTCPv4);
	if (err == noErr) {
		printf("  Remote...\n");
		err = DoConfigurationDigestTestsForProtocol(kOTCfgTypeRemote);
	}
	if (err == noErr) {
		printf("  Modem...\n");
		err = DoConfigurationDigestTestsForProtocol(kOTCfgTypeModem);
	}
	if (err == noErr) {
		printf("Success!\n");
	} else {
		printf("Failed with error %ld.\n", err);
	}
	printf("\n");
}

static OSStatus DoReadEveryConfigTestForProtocol(OSType protocol)
{
	OSStatus err;
	NSHConfigurationListHandle configList;
	ItemCount i;
	NSHConfigurationDigest junkConfigDigest;
	
	configList = (NSHConfigurationListHandle) NewHandle(0);
	err = MemError();
	
	if (err == noErr) {
		assert(configList != NULL);
		printf("    reading configuration list...\n");		
		err = NSHGetConfigurationList(protocol, configList);
	}
	if (err == noErr) {
		HLock( (Handle) configList );
		for (i = 0; i < NSHCountConfigurationList(configList); i++) {
			printf("    reading configuration “%#s”...\n", (*configList)[i].name);
			OTMemzero(&junkConfigDigest, sizeof(junkConfigDigest));
			err = NSHGetConfiguration(&(*configList)[i], &junkConfigDigest);
		}
		printf("\n");
	}
	
	// Clean up.
	
	if (configList != NULL) {
		DisposeHandle( (Handle) configList );
		assert(MemError() == noErr);
	}
		
	return err;
}

static void DoReadEveryConfigTest(void)
{
	OSStatus err;
	
	printf("Read Every Configuration Test...\n");
	printf("  TCP/IP...\n");
	err = DoReadEveryConfigTestForProtocol(kOTCfgTypeTCPv4);
	if (err == noErr) {
		printf("  Remote...\n");
		err = DoReadEveryConfigTestForProtocol(kOTCfgTypeRemote);
	}
	if (err == noErr) {
		printf("  Modem...\n");
		err = DoReadEveryConfigTestForProtocol(kOTCfgTypeModem);
	}
	if (err == noErr) {
		printf("Success!\n");
	} else {
		printf("Failed with error %ld.\n", err);
	}
	printf("\n");
}

static void BelowIPTests(void)
{
	OSStatus err;
	NSHConfigurationListHandle configList;
	ItemCount 				   configIndex;
	NSHConfigurationEntry      foundConfig;
	NSHTCPv4ConfigurationDigest tcpDigest;
	Handle newBelowIP;
	
	newBelowIP = NULL;
	OTMemzero(&tcpDigest, sizeof(tcpDigest));

	configList = (NSHConfigurationListHandle) NewHandle(0);
	err = MoreMemError(configList);
	if (err == noErr) {
		err = NSHGetConfigurationList(kOTCfgTypeTCPv4, configList);
	}
	if (err == noErr) {
		// Find the active configuration.  This will do bogus things if
		// there’s no active configuration, or indeed there’s no configuration,
		// but I don’t care for this test program.
		for (configIndex = 0; configIndex < NSHCountConfigurationList(configList); configIndex++) {
			if ( (*configList)[configIndex].selected ) {
				foundConfig = (*configList)[configIndex];
				break;
			}
		}
	}
	if (err == noErr) {
		tcpDigest.fBelowIP = NewHandle(0);
		err = MoreMemError(tcpDigest.fBelowIP);
	}
	if (err == noErr) {
		err = NSHGetConfiguration(&foundConfig, (NSHConfigurationDigest *) &tcpDigest);
	}
	if (err == noErr) {
		// I’m not prepared to handle where the current configuration has
		// an invalid port ref right now.
		assert(tcpDigest.fPortRef != kOTInvalidPortRef);
		err = MoreStrListAppend(tcpDigest.fBelowIP, "\pNetworkSetupTest");
	}
	if (err == noErr) {
		newBelowIP = tcpDigest.fBelowIP;
		err = HandToHand(&newBelowIP);
		if (err != noErr) {
			newBelowIP = NULL;
		}
	}
	if (err == noErr) {
		err = NSHSetConfiguration(&foundConfig, (NSHConfigurationDigest *) &tcpDigest);
	}
	if (err == noErr) {
		err = NSHGetConfiguration(&foundConfig, (NSHConfigurationDigest *) &tcpDigest);
	}
	if (err == noErr) {
		if ( ! CompareHandle(tcpDigest.fBelowIP, newBelowIP) ) {
			printf("    ••• handle comparison failure\n");
		}
	}
	if (err == noErr) {
		err = MoreStrListDeleteIndexed(tcpDigest.fBelowIP, MoreStrListCount(tcpDigest.fBelowIP));
		assert(err == noErr);
		err = NSHSetConfiguration(&foundConfig, (NSHConfigurationDigest *) &tcpDigest);
	}
	
	// Clean up.
	
	if (configList != NULL) {
		DisposeHandle( (Handle) configList );
		assert(MemError() == noErr);
	}
	if (tcpDigest.fBelowIP != NULL) {
		DisposeHandle(tcpDigest.fBelowIP);
		assert(MemError() == noErr);
	}
	if (newBelowIP != NULL) {
		DisposeHandle(newBelowIP);
		assert(MemError() == noErr);
	}
	
	if (err == noErr) {
		printf("Success!\n");
	} else {
		printf("Failed with error %ld.\n", err);
	}
	printf("\n");
}

static void DoAutomatedTests(void)
{
	char commandStr[256];
	
	printf("List of automated tests:\n");
	printf("  a) AppleTalk on/off\n");
	printf("  r) configuration digest tests\n");
	printf("  R) read every configuration\n");
	printf("  A) all\n");
	printf("Choose a test:\n");
	gets(commandStr);
	switch (commandStr[0]) {
		case 'a':
			DoAutomatedAppleTalkOnOffTest();
			break;
		case 'r':
			DoConfigurationDigestTests();
			break;
		case 'R':
			DoReadEveryConfigTest();
			break;
		case 'A':
			DoReadEveryConfigTest();
			DoAutomatedAppleTalkOnOffTest();
			DoConfigurationDigestTests();
			break;
		case 'b':
			BelowIPTests();
			break;
		default:
			printf("Huh?\n");
			break;
	}
}

/////////////////////////////////////////////////////////////////
#pragma mark ----- Main -----

enum {
	kOTVersion111 = 0x01118000
};

void main(void) 
{
	OSStatus err;
	UInt32 otVersion;
	char commandStr[256];
	Boolean quitNow;
	
	printf("NetworkSetupTest\n");
	printf("-- A simple test program for the NetworkSetupHelpers library.\n");

	err = InitOpenTransport();
	if (err == noErr) {
		err = Gestalt(gestaltOpenTptVersions, (long *) &otVersion);
		if (err != noErr || otVersion < kOTVersion111) {
			printf("This program requires Open Transport 1.1.1 or higher.\n");
			err = -9;
		}
		if (err == noErr) {
			if (IsNetworkSetupAvailable()) {
				printf("-- Using Network Setup library.\n");
			} else {
				printf("-- Using old, cheesy, undocumented system calls.\n");
			}
			printf("\n");

			PrintHelp();
			quitNow = false;
			do {
				printf("Enter a command:\n");
				gets(commandStr);
				switch (commandStr[0]) {
					case 'l': DoListConfigurations(); break;
					case 's': DoSetConfiguration(); break;
					case 'd': DoDuplicateConfiguration(); break;
					case 'D': DoDeleteConfiguration(); break;
					case 'r': DoRenameConfiguration(); break;
					case 'p': DoPrintConfiguration(); break;
					case 'c': DoCreateConfiguration(); break;
					case 'w': DoTCPWillDial(); break;
					case 'A': DoAppleTalkOnOff(true); break;
					case 'a': DoAppleTalkOnOff(false); break;
					case 'å': DoIsAppleTalkActive(); break;
					case 'e': DoRemoteAccessEncoding(); break;
					case 'L': DoDHCPRelease(); break;
					case 'b': BelowIPTests(); break;
					case '0': DoAutomatedTests(); break;
					case '?': PrintHelp(); break;
					case 'q': quitNow = true; break;
					default:
						printf("Huh?\n");
						break;
				}
			} while ( ! quitNow );
		}
		
		CloseOpenTransport();
	}
	if (err == noErr) {
		printf("Success.\n");
	} else {
		printf("Failed with error %ld.\n", err);
	}
	printf("Done.  Press command-Q to Quit.\n");
}
