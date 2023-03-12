/*
    File:			SCSIOldAndNew.c
	
    Description:	Part of the sample SCSIOldAndNew. This sample demonstrates how to use the SCSITask User
					Client (STUC) and the (as of Mac OS X 10.2) deprecated IOSCSIUserClient APIs to find
					SCSI devices and issue a simple INQUIRY command to each device.
                        
    Copyright:		© Copyright 2003-2006 Apple Computer, Inc. All rights reserved.
	
    Disclaimer:		IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc.
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

            1.1		10/10/2006			Updated to produce a universal binary. Now requires Xcode 2.2.1 or
										later to build.

            1.0	 	05/08/2003			New sample.
        
*/

#include <Carbon/Carbon.h>
#include <IOKit/IOKitLib.h>
#include <IOKit/scsi-commands/SCSICmds_INQUIRY_Definitions.h>
#include "STUCMethod.h"
#include "OldMethod.h"

#define kCommandDirectAccessSBCDevice				'00h '
#define kCommandSequentialAccessSSCDevice			'01h '
#define kCommandPrinterSSCDevice					'02h '
#define kCommandProcessorSPCDevice					'03h '
#define kCommandWriteOnceSBCDevice					'04h '
#define kCommandCDROM_MMCDevice						'05h '
#define kCommandScannerSCSI2Device					'06h '
#define kCommandOpticalMemorySBCDevice				'07h '
#define kCommandMediumChangerSMCDevice				'08h '
#define kCommandCommunicationsSSCDevice				'09h '
#define kCommandStorageArrayControllerSCC2Device	'0Ch '
#define kCommandEnclosureServicesSESDevice			'0Dh '
#define kCommandSimplifiedDirectAccessRBCDevice		'0Eh '
#define kCommandOpticalCardReaderOCRWDevice			'0Fh '
#define kCommandUnknownOrNoDeviceType				'1Fh '

void TestDevices(int peripheralDeviceType)
{
    io_iterator_t	iterator = IO_OBJECT_NULL;
    
    if (FindDevicesUsingSTUC(peripheralDeviceType, kIOMasterPortDefault, &iterator)) {
        TestDevicesUsingSTUC(peripheralDeviceType, iterator);
    }
#if MAC_OS_X_VERSION_MIN_REQUIRED <= MAC_OS_X_VERSION_10_2
	// The "old" method exists only from Mac OS X 10.0.x through 10.2.x.
    else if (FindDevicesUsingOldMethod(peripheralDeviceType, kIOMasterPortDefault, &iterator)) {
        TestDevicesUsingOldMethod(iterator);
    }
#endif
    else {
        fprintf(stderr, "No devices with peripheral device type %02Xh found.\n", peripheralDeviceType);
    }
    
    if (iterator) {
        IOObjectRelease(iterator);
    }
}

// Handle command-process events at the application level
OSStatus DoAppCommandProcess(EventHandlerCallRef nextHandler, EventRef theEvent, void* userData)
{
    HICommand  aCommand;
    OSStatus   result;

    GetEventParameter(theEvent, kEventParamDirectObject, typeHICommand, NULL, sizeof(HICommand),
                      NULL, &aCommand);
    
    switch (aCommand.commandID) {
		case kCommandDirectAccessSBCDevice:
			TestDevices(kINQUIRY_PERIPHERAL_TYPE_DirectAccessSBCDevice);
			result = noErr;
			break;
						
		case kCommandSequentialAccessSSCDevice:
			TestDevices(kINQUIRY_PERIPHERAL_TYPE_SequentialAccessSSCDevice);
			result = noErr;
			break;
				
		case kCommandPrinterSSCDevice:
			TestDevices(kINQUIRY_PERIPHERAL_TYPE_PrinterSSCDevice);
			result = noErr;
			break;
				
		case kCommandProcessorSPCDevice:
			TestDevices(kINQUIRY_PERIPHERAL_TYPE_ProcessorSPCDevice);
			result = noErr;
			break;
				
		case kCommandWriteOnceSBCDevice:
			TestDevices(kINQUIRY_PERIPHERAL_TYPE_WriteOnceSBCDevice);
			result = noErr;
			break;
				
		case kCommandCDROM_MMCDevice:
			TestDevices(kINQUIRY_PERIPHERAL_TYPE_CDROM_MMCDevice);
			result = noErr;
			break;
				
		case kCommandScannerSCSI2Device:
			TestDevices(kINQUIRY_PERIPHERAL_TYPE_ScannerSCSI2Device);
			result = noErr;
			break;
				
		case kCommandOpticalMemorySBCDevice:
			TestDevices(kINQUIRY_PERIPHERAL_TYPE_OpticalMemorySBCDevice);
			result = noErr;
			break;
				
		case kCommandMediumChangerSMCDevice:
			TestDevices(kINQUIRY_PERIPHERAL_TYPE_MediumChangerSMCDevice);
			result = noErr;
			break;
				
		case kCommandCommunicationsSSCDevice:
			TestDevices(kINQUIRY_PERIPHERAL_TYPE_CommunicationsSSCDevice);
			result = noErr;
			break;
				
		case kCommandStorageArrayControllerSCC2Device:
			TestDevices(kINQUIRY_PERIPHERAL_TYPE_StorageArrayControllerSCC2Device);
			result = noErr;
			break;
				
		case kCommandEnclosureServicesSESDevice:
			TestDevices(kINQUIRY_PERIPHERAL_TYPE_EnclosureServicesSESDevice);
			result = noErr;
			break;
				
		case kCommandSimplifiedDirectAccessRBCDevice:
			TestDevices(kINQUIRY_PERIPHERAL_TYPE_SimplifiedDirectAccessRBCDevice);
			result = noErr;
			break;
				
		case kCommandOpticalCardReaderOCRWDevice:
			TestDevices(kINQUIRY_PERIPHERAL_TYPE_OpticalCardReaderOCRWDevice);
			result = noErr;
			break;
				
		case kCommandUnknownOrNoDeviceType:
			TestDevices(kINQUIRY_PERIPHERAL_TYPE_UnknownOrNoDeviceType);
			result = noErr;
			break;

        case kHICommandQuit:
            QuitApplicationEventLoop();
            result = noErr;
            break;
    
        default:
            result = eventNotHandledErr;
            break;
    }
    
    HiliteMenu(0);
    return result;
}

void InstallAppEvents(void)
{
    EventHandlerUPP	eventHandler;
    EventTypeSpec  	eventType;

    eventHandler = NewEventHandlerUPP(DoAppCommandProcess);
    eventType.eventClass = kEventClassCommand;
    eventType.eventKind = kEventCommandProcess;
    InstallApplicationEventHandler(eventHandler, 1, &eventType, NULL, NULL);
}

int main(int argc, char* argv[])
{
    IBNibRef 		nibRef;
    OSStatus		err;

    // Create a Nib reference passing the name of the nib file (without the .nib extension)
    // CreateNibReference only searches into the application bundle.
    err = CreateNibReference(CFSTR("Main"), &nibRef);
    require_noerr(err, CantGetNibRef);
    
    // Once the nib reference is created, set the menu bar. "MainMenu" is the name of the menu bar
    // object. This name is set in InterfaceBuilder when the nib is created.
    err = SetMenuBarFromNib(nibRef, CFSTR("MenuBar"));
    require_noerr(err, CantSetMenuBar);
    
    // We don't need the nib reference any more.
    DisposeNibReference(nibRef);
    
    // Install our application event handler.
    InstallAppEvents();

    // Call the event loop.
    RunApplicationEventLoop();

CantSetMenuBar:
CantGetNibRef:
    return err;
}

