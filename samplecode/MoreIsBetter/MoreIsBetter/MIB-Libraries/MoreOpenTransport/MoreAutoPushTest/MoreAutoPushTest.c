/*
	File:		MoreAutoPushTest.c

	Contains:	Program to test MoreAutoPush module.

	Written by:	Quinn "The Eskimo!"

	Copyright:	Copyright (c) 1997-2001 by Apple Computer, Inc., All Rights Reserved.

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

$Log: MoreAutoPushTest.c,v $
Revision 1.3  2002/11/08 23:41:41         
Include <Files.h> before <OpenTransportProviders.h> because of a bug in UI.

Revision 1.2  2001/11/07 15:51:14         
Tidy up headers, add CVS logs, update copyright.


         <1>     5/10/00    Quinn   First checked in.
*/

/////////////////////////////////////////////////////////////////////

// Standard Mac OS interfaces

#include <Files.h>
#include <OpenTransportProviders.h>

// Standard C interfaces

#include <stdio.h>

// Pick up the thing we're trying to test (-:

#include "MoreAutoPush.h"

/////////////////////////////////////////////////////////////////////
#pragma mark ----- Global Variables -----

static Boolean gVerboseMode = false;

/////////////////////////////////////////////////////////////////////
#pragma mark ----- Printing Utilities -----

// This code stolen directly from the OTDumpPortRegistry sample.

enum {
	kNumDeviceTypeNames = 20
};

static char *gDeviceTypeNames[kNumDeviceTypeNames] = {
	"kOTNoDeviceType",
	"kOTADEVDevice",
	"kOTMDEVDevice",
	"kOTLocalTalkDevice",
	"kOTIRTalkDevice",
	"kOTTokenRingDevice",
	"kOTISDNDevice",
	"kOTATMDevice",
	"kOTSMDSDevice",

	"kOTSerialDevice",
	"kOTEthernetDevice",
	"kOTSLIPDevice",
	"kOTPPPDevice",
	"kOTModemDevice",
	"kOTFastEthernetDevice",
	"kOTFDDIDevice",
	"kOTIrDADevice",
	"kOTATMSNAPDevice",
	"kOTFibreChannelDevice",
	"kOTFireWireDevice"
};

static char *GetNameForPortsDeviceType(OTPortRef portRef)
{
	UInt16 deviceType;
	char *deviceTypeName;

	deviceType = OTGetDeviceTypeFromPortRef(portRef);
	if (deviceType >= 0 && deviceType < kNumDeviceTypeNames) {
		deviceTypeName = gDeviceTypeNames[deviceType];
	} else {
		if (deviceType == kOTPseudoDevice) {
			deviceTypeName = "kOTPseudoDevice";
		} else {
			deviceTypeName = "unknown";
		}
	}
	return deviceTypeName;
}

static void PrintFlag(char *flagName, UInt32 flagField, UInt32 flagMask)
{
  printf("  %s = %d\n", flagName, (flagField & flagMask) != 0);
}

static void PrintAnyPortVerbose(const OTPortRecord *port)
	// Print an (extremely) verbose description of a port.
{
	OTPortRecord	childPort;
	UInt16			deviceType;
	UInt16			busType;
	UInt16			slotNumber;
	UInt16			slotOtherInfo;
	SInt32			childIndex;
	Str255			userFriendlyName;

	// Information we can get from the port reference, fRef
	
	busType = OTGetBusTypeFromPortRef(port->fRef);
	deviceType = OTGetDeviceTypeFromPortRef(port->fRef);
	slotNumber = OTGetSlotFromPortRef(port->fRef, &slotOtherInfo);
	
	// Let's print some info!!!
	
	printf("••• Dumping information for port “%s”.\n\n", port->fPortName);

	printf("fPortFlags...\n");
	PrintFlag("kOTPortIsActive", port->fPortFlags, kOTPortIsActive);
	PrintFlag("kOTPortIsDisabled", port->fPortFlags, kOTPortIsDisabled);
	PrintFlag("kOTPortIsUnavailable", port->fPortFlags, kOTPortIsUnavailable);
	PrintFlag("kOTPortIsOffline", port->fPortFlags, kOTPortIsOffline);
	printf("\n");
	
	printf("fInfoFlags = 0x%08X...\n", port->fInfoFlags);
	PrintFlag("kOTPortIsDLPI", port->fInfoFlags, kOTPortIsDLPI);
	PrintFlag("kOTPortIsTPI", port->fInfoFlags, kOTPortIsTPI);
	PrintFlag("kOTPortCanYield", port->fInfoFlags, kOTPortCanYield);
	PrintFlag("kOTPortCanArbitrate", port->fInfoFlags, kOTPortCanArbitrate);
	PrintFlag("kOTPortIsTransitory", port->fInfoFlags, kOTPortIsTransitory);
	PrintFlag("kOTPortAutoConnects", port->fInfoFlags, kOTPortAutoConnects);
	PrintFlag("kOTPortIsSystemRegistered", port->fInfoFlags, kOTPortIsSystemRegistered);
	PrintFlag("kOTPortIsPrivate", port->fInfoFlags, kOTPortIsPrivate);
	PrintFlag("kOTPortIsAlias", port->fInfoFlags, kOTPortIsAlias);
	printf("\n");

	printf("fCapabilities = 0x%08X\n", port->fCapabilities);
	switch (deviceType) {
		case kOTEthernetDevice:
		case kOTFastEthernetDevice:
			PrintFlag("kOTFramingEthernet", port->fCapabilities, kOTFramingEthernet);
			PrintFlag("kOTFramingEthernetIPX", port->fCapabilities, kOTFramingEthernetIPX);
			PrintFlag("kOTFraming8023", port->fCapabilities, kOTFraming8023);
			PrintFlag("kOTFraming8022", port->fCapabilities, kOTFraming8022);
			break;
		case kOTISDNDevice:
			PrintFlag("kOTISDNFramingTransparentSupported", port->fCapabilities, kOTISDNFramingTransparentSupported);
			PrintFlag("kOTISDNFramingHDLCSupported", port->fCapabilities, kOTISDNFramingHDLCSupported);
			PrintFlag("kOTISDNFramingV110Supported", port->fCapabilities, kOTISDNFramingV110Supported);
			PrintFlag("kOTISDNFramingV14ESupported", port->fCapabilities, kOTISDNFramingV14ESupported);
			break;
		case kOTSerialDevice:
			PrintFlag("kOTSerialFramingAsync", port->fCapabilities, kOTSerialFramingAsync);
			PrintFlag("kOTSerialFramingHDLC", port->fCapabilities, kOTSerialFramingHDLC);
			PrintFlag("kOTSerialFramingSDLC", port->fCapabilities, kOTSerialFramingSDLC);
			PrintFlag("kOTSerialFramingAsyncPackets", port->fCapabilities, kOTSerialFramingAsyncPackets);
			break;
		default:
			break;
	}
	printf("\n");
	
	if (port->fNumChildPorts == 0) {
		printf("No child ports.\n");
	} else {
	  	printf("fChildPorts...\n");
	  	for (childIndex = 0; childIndex < port->fNumChildPorts; childIndex++) {
	  		if (OTFindPortByRef(&childPort, port->fChildPorts[childIndex])) {
					printf("  %s\n", childPort.fPortName);
	  		} else {
	  			printf("That's very strange.\n");
	  		}
	  	}
	}
	printf("\n");
	  	
	printf("fModuleName = %s\n", port->fModuleName);
	printf("fSlotID = %s (only set on PCI ports).\n", port->fSlotID);
	printf("fResourceInfo = %s (optional config library name).\n", port->fResourceInfo);

	// Extra information gleaned from port ref
	
	printf("fPortRef = %08x\n\n", port->fRef);

	printf("Bus type is ");
	switch (busType) {
		case kOTUnknownBusPort:
			printf("kOTUnknownBusPort");
			break;
		case kOTMotherboardBus:
			printf("kOTMotherboardBus");
			break;
		case kOTNuBus:
			printf("kOTNuBus");
			break;
		case kOTPCIBus:
			printf("kOTPCIBus");
			break;
		case kOTGeoPort:
			printf("kOTGeoPort");
			break;
		case kOTPCCardBus:
			printf("kOTPCCardBus");
			break;
		case kOTFireWireBus:
			printf("kOTFireWireBus");
			break;
		default:
			printf("unknown");
	}
	printf(" (%d).\n", busType);

	printf("Device type is %s (%d).\n", GetNameForPortsDeviceType(port->fRef), deviceType);
	
	printf("Slot number is %d, other is %d.\n", slotNumber, slotOtherInfo);

	OTGetUserPortNameFromPortRef(port->fRef, userFriendlyName);
	printf("OTGetUserPortNameFromPortRef = %#s\n", userFriendlyName);

	printf("\n\n");
}

/////////////////////////////////////////////////////////////////////
#pragma mark ----- Printing Commands -----

static void PrintAnyPort(const OTPortRecord *port)
	// Prints a port, regardless of its characteristics.
	// 
	// This routine is designed as a callback for
	// ForEachPort.
{
	if (gVerboseMode) {
		PrintAnyPortVerbose(port);
	} else {
		printf("%s (%s) (%s)\n", port->fPortName, port->fModuleName, GetNameForPortsDeviceType(port->fRef));
	}
}

static void PrintPortIfTCP(const OTPortRecord *port)
	// Prints a port if it is suitable for TCP/IP.
	// This will not generate the exact list produced
	// by the TCP/IP control panel, but it's close.
	// 
	// This routine is designed as a callback for
	// ForEachPort.
{
	switch ( OTGetDeviceTypeFromPortRef(port->fRef) ) {
		case kOTADEVDevice:
		case kOTIRTalkDevice:
		case kOTISDNDevice:
		case kOTATMDevice:
		case kOTSMDSDevice:
		case kOTSerialDevice:
		case kOTModemDevice:
			// do nothing
			break;
			
		default:
			PrintAnyPort(port);
			break;
	}
}

static void PrintPortIfAppleTalk(const OTPortRecord *port)
	// Prints a port if it is suitable for AppleTalk.
	// This will not generate the exact list produced
	// by the AppleTalk control panel ('adev's, for example,
	// show up via other magic), but it's close.
	// 
	// This routine is designed as a callback for
	// ForEachPort.
{
	switch ( OTGetDeviceTypeFromPortRef(port->fRef) ) {
		case kOTSerialDevice:
		case kOTModemDevice:
		case kOTISDNDevice:
		case kOTATMDevice:
		case kOTATMSNAPDevice:
		case kOTMDEVDevice:
			// do nothing
			break;
			
		case kOTEthernetDevice:
		case kOTFastEthernetDevice:
			// Ethernet ports must supports 802.2 framing.
			if (port->fCapabilities & kOTFraming8022) {
				PrintAnyPort(port);
			}
			break;
			
		default:
			if (port->fInfoFlags & kOTPortIsDLPI) {
				PrintAnyPort(port);
			}
			break;
	}
}

/////////////////////////////////////////////////////////////////////

typedef void (*PortActionProc)(const OTPortRecord *port);

static void ForEachPort(PortActionProc doThis)
	// Iterates through each port in the OT port
	// registry, calling the doThis routine for
	// each one.  Used as the driving core for all
	// of the printing commands.
{
	Boolean gotIt;
	ItemCount portIndex;
	OTPortRecord port;
	
	portIndex = 0;
	do {
		gotIt = OTGetIndexedPort(&port, portIndex);
		if (gotIt) {
			doThis(&port);
			portIndex += 1;
		}
	} while (gotIt);
}

/////////////////////////////////////////////////////////////////////
#pragma mark ----- AutoPush Commands -----

static void DumpAutoPush(void)
	// Add a module to a driver's autopush list.
	// Basically a console user interface for
	// GetAutoPushList.
{
	OSStatus err;
	char driverName[256];
	OTAutopushInfo autoPushInfo;
    ItemCount moduleIndex;
	
	printf("Enter the driver name:\n");
	gets(driverName);
	if (driverName[0] != 0) {
		err = GetAutoPushList(driverName, &autoPushInfo);
		if (err == noErr) {
			for (moduleIndex = 0; moduleIndex < autoPushInfo.sap_npush; moduleIndex++) {
				printf("%s\n", autoPushInfo.sap_list[moduleIndex]);
			}
		} else {
			printf("Failed with error %ld.\n", err);
		}
	}
}

static void DoAutoPush(void)
	// Add a module to a driver's autopush list.
	// Basically a console user interface for
	// AddModuleToAutoPushList.
{
	OSStatus err;
	char driverName[256];
	char moduleName[256];
	
	printf("Enter the driver name:\n");
	gets(driverName);
	if (driverName[0] != 0) {
		printf("Enter the module name:\n");
		gets(moduleName);
		if (moduleName[0] != 0) {
			err = AddModuleToAutoPushList(driverName, moduleName);
			if (err == noErr) {
				printf("Success!\n");
			} else {
				printf("Failed with error %ld.\n", err);
			}
		}
	}
}

static void UndoAutoPush(void)
	// Add a module to a driver's autopush list.
	// Basically a console user interface for
	// RemoveModuleDriverAutoPushList.
{
	OSStatus err;
	char driverName[256];
	char moduleName[256];
	
	printf("Enter the driver name:\n");
	gets(driverName);
	if (driverName[0] != 0) {
		printf("Enter the module name:\n");
		gets(moduleName);
		if (moduleName[0] != 0) {
			err = RemoveModuleDriverAutoPushList(driverName, moduleName);
			if (err == noErr) {
				printf("Success!\n");
			} else {
				printf("Failed with error %ld.\n", err);
			}
		}
	}
}

static void ValidateModule(void)
	// Determine whether a module is available to STREAMS.
{
	OSStatus err;
	char moduleName[256];
	
	printf("Enter the module name:\n");
	gets(moduleName);
	if (moduleName[0] != 0) {
		err = ValidateModuleExists(moduleName);
		if (err == noErr) {
			printf("Module is available!\n");
		} else if (err == kOTNotFoundErr) {
			printf("Module is not available!\n");
		} else {
			printf("Failed with error %ld.\n", err);
		}
	}
}

/////////////////////////////////////////////////////////////////////

static void PrintHelp(void)
{
	printf("v) Toggle verbose mode\n");
	printf("l) List ports and modules\n");
	printf("t) List TCP/IP ports\n");
	printf("a) List AppleTalk ports\n");
	printf("d) Display autopush list for a driver\n");
	printf("p) Autopush a module on a driver\n");
	printf("P) Remove a module from a driver's autopush list\n");
	printf("e) Determine whether a module exists\n");
	printf("?) Help\n");
	printf("q) Quit\n");
}

void main(void)
{
	OSStatus err;
	Boolean quitNow;
	char commandStr[256];
	
	printf("Hello Cruel World!\n");
	
	err = InitOpenTransport();
	
	if (err == noErr) {
	
		PrintHelp();

		quitNow = false;
		do {
			printf("Enter a command:\n");
			gets(commandStr);
			switch (commandStr[0]) {
				case 'v':
					gVerboseMode = ! gVerboseMode;
					if (gVerboseMode) {
						printf("Verbose mode is now on.\n");
					} else {
						printf("Verbose mode is now off.\n");
					}
					break;
				case 'l':
					ForEachPort(PrintAnyPort);
					break;
				case 't':
					ForEachPort(PrintPortIfTCP);
					break;
				case 'a':
					ForEachPort(PrintPortIfAppleTalk);
					break;
				case 'd':
					DumpAutoPush();
					break;
				case 'p':
					DoAutoPush();
					break;
				case 'P':
					UndoAutoPush();
					break;
				case 'e':
					ValidateModule();
					break;
				case '?':
					PrintHelp();	
					break;
				case 'q':
					quitNow = true;
					break;
				default:
					printf("Huh?\n");
					break;
			}
		} while ( ! quitNow );
		
		CloseOpenTransport();
	}
	
	if (err == noErr) {
		printf("Success.\n");
	} else {
		printf("Failed with error %ld.\n", err);
	}
	printf("Done.  Press command-Q to Quit.\n");
}
