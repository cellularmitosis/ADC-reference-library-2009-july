/*
    File:			OldMethod.c
	
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
					to these terms, Apple grants you a personal, non-exclusive license, under Appleâ€™s
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

// The "old" method exists only from Mac OS X 10.0.x through 10.2.x.
#include <AvailabilityMacros.h>
#if MAC_OS_X_VERSION_MIN_REQUIRED <= MAC_OS_X_VERSION_10_2

#include "OldMethod.h"

void CreateMatchingDictionaryForOldMethod(SInt32 peripheralDeviceType, CFMutableDictionaryRef *matchingDict)
{
    SInt32		deviceTypeNumber = peripheralDeviceType;
    CFNumberRef	deviceTypeRef = NULL;
    
    // Set up a matching dictionary to search the I/O Registry by class name for
    // all subclasses of IOSCSIDevice.
    *matchingDict = IOServiceMatching(kIOSCSIDeviceClassName);

    if (*matchingDict != NULL)
    {
        // Add key for device type to refine the matching dictionary. 
        // First create a CFNumber to store in the dictionary.
        deviceTypeRef = CFNumberCreate(kCFAllocatorDefault, kCFNumberIntType, &deviceTypeNumber);
        CFDictionarySetValue(*matchingDict, CFSTR(kSCSIPropertyDeviceTypeID), deviceTypeRef);
    }
}

boolean_t FindDevicesUsingOldMethod(SInt32 peripheralDeviceType, mach_port_t masterPort, io_iterator_t *iterator)
{
    CFMutableDictionaryRef	matchingDict = NULL;
    boolean_t				result = false;
    
    // Set up a matching dictionary to search the I/O Registry for SCSI devices
    // we are interested in.
    CreateMatchingDictionaryForOldMethod(peripheralDeviceType, &matchingDict);
    
    if (matchingDict == NULL) {
        fprintf(stderr, "Couldn't create a matching dictionary.\n");
    }
    else {
		kern_return_t	kr;
        
        // Now search I/O Registry for matching devices.
        kr = IOServiceGetMatchingServices(masterPort, matchingDict, iterator);

        if (*iterator && kr == kIOReturnSuccess) {
            result = true;
        }
    }
    
    // IOServiceGetMatchingServices consumes a reference to the
    // matching dictionary, so we don't need to release the dictionary ref.

    return result;
}

void PrintSCSIInquiryDataUsingOldMethod(SCSIInquiry *inquiryData, UInt32 inquiryDataSize)
{	
    int		dataSize;

    fprintf(stderr, "\n*********** INQUIRY DATA ***********\n");
    fprintf(stderr, "Got inquiry results size = %ld\n", inquiryDataSize);

    fprintf(stderr, "  Peripheral device type = 0x%02x\n", inquiryData->devType & kSCSIDevTypeMask);
    fprintf(stderr, "  Removable media bit = %d\n", inquiryData->devTypeMod == kSCSIDevTypeModRemovable);
    fprintf(stderr, "  Version = 0x%x\n", inquiryData->version);
    fprintf(stderr, "  Response data format = 0x%x\n", inquiryData->format);
    fprintf(stderr, "  Additional length = 0x%x\n", inquiryData->length);
    fprintf(stderr, "  SCCS and Reserved = 0x%x\n", inquiryData->reserved5);
    fprintf(stderr, "  Byte 6 flags = 0x%x\n", inquiryData->reserved6);
    fprintf(stderr, "  Byte 7 flags = 0x%x\n", inquiryData->flags);

    // The SCSI Primary Commands spec doesn't require VENDOR IDENTIFICATION, PRODUCT IDENTIFICATION,
    // or PRODUCT REVISION LEVEL to be NUL-terminated, hence the precision values in the following
    // fprintf format strings.

    fprintf(stderr, "  Vendor identification  = \"%.8s\"\n", inquiryData->vendorName);
    fprintf(stderr, "  Product identification = \"%.16s\"\n", inquiryData->productName);
    fprintf(stderr, "  Product revision level = \"%.4s\"\n", inquiryData->productRevision);
    fprintf(stderr, "  Vendor specific        = \"%.20s\"\n", inquiryData->vendorSpecific);

    // Determine if there is any more data to print. To do so, substract the number of 
    // bytes in the structure before the moreReserved field from the passed
    // value for the total size, giving the amount of data in the moreReserved field.
    // If there is any more data, print the hex values for the characters.
    
    dataSize = inquiryDataSize - ((char *) &inquiryData->moreReserved - (char *) inquiryData);
    if (dataSize > 0) {
        int	i;
        
        fprintf(stderr, "  moreReserved(%d) = ", dataSize);
        for (i = 0; i < dataSize; i++) {
            if ( !(i & 0xf) ) {
                fprintf(stderr, "\n");
            }
            fprintf(stderr, "%02x ", ((UInt8 *) inquiryData->moreReserved)[i]);
        }
        
        fprintf(stderr, "\n");
    }

    fprintf(stderr, "\n");
}

void GetInquiryDataUsingOldMethod(IOSCSIDeviceInterface **interface)
{
    UInt8			inquiryData[255];
    UInt32			inquiryDataSize = sizeof(inquiryData);
    kern_return_t	kr = kIOReturnSuccess;
    
    bzero(inquiryData, sizeof(inquiryData));	// Zero data block.

    // Call a function of the SCSI user client that returns cached information
    // about the device.
    kr = (*interface)->getInquiryData(interface, (SCSIInquiry *) inquiryData, 
                                      sizeof(inquiryData), &inquiryDataSize);

    if (kr != kIOReturnSuccess) {
        fprintf(stderr, "Couldn't get inquiry data for device. (0x%08x)\n", kr);
    }
    else {
        PrintSCSIInquiryDataUsingOldMethod((SCSIInquiry *) inquiryData, inquiryDataSize);
    }
}

void CreateDeviceInterfaceUsingOldMethod(io_object_t scsiDevice, IOSCSIDeviceInterface ***interface)
{
    io_name_t			className;
    IOCFPlugInInterface	**plugInInterface = NULL;
    HRESULT				plugInResult = S_OK;
    kern_return_t		kr = kIOReturnSuccess;
    SInt32				score = 0;

    // Get the object's class name just to display it
    kr = IOObjectGetClass(scsiDevice, className);

    if (kr != kIOReturnSuccess) {
        fprintf(stderr, "Failed to get class name. (0x%08x)\n", kr);
    }
    else {
        fprintf(stderr, "Found device class \"%s\" using old method.\n", className);

        // Create the base interface of type IOCFPlugInInterface.
        // This object will be used to create the SCSI device interface object.
        kr = IOCreatePlugInInterfaceForService( scsiDevice,
                          kIOSCSIUserClientTypeID, kIOCFPlugInInterfaceID,
                          &plugInInterface, &score);

        if (kr != kIOReturnSuccess) {
            fprintf(stderr, "Couldn't create a plugin interface for the io_service_t. (0x%08x)\n", kr);
        }
        else {
            // Query the base plugin interface for an instance of the specific SCSI device interface
            // object.
            plugInResult = (*plugInInterface)->QueryInterface(plugInInterface, 
                                                CFUUIDGetUUIDBytes(kIOSCSIDeviceInterfaceID),
                          			(LPVOID *) interface);
            
            if (plugInResult != S_OK) {
                fprintf(stderr, "Couldn't create SCSI device interface. (%ld)\n", plugInResult);
            }
                    
            // We're now finished with the instance of IOCFPlugInInterface.
            IODestroyPlugInInterface(plugInInterface);
        }
    }
}

IOCDBCommandInterface **CreateCommandInterfaceUsingOldMethod(IOSCSIDeviceInterface **interface)
{
    HRESULT					plugInResult = S_OK;
    IOCDBCommandInterface	**cdbCmdInterface = NULL;

    fprintf(stderr, "Opened device\n");
    plugInResult = (*interface)->QueryInterface(interface, CFUUIDGetUUIDBytes(kIOCDBCommandInterfaceID), 
                                                (LPVOID *) &cdbCmdInterface);

    if (plugInResult != S_OK) {
        fprintf(stderr, "Couldn't create a CDB command. (%ld)\n", plugInResult);
    }
    
    return cdbCmdInterface;
}

void ExecuteInquiryUsingOldMethod(IOCDBCommandInterface **cdbCommandInterface)
{
    UInt8			inquiryData[36 /* 255 */];
    IOVirtualRange	range[1];
    CDBInfo			cdb;
    CDBResults		results;
    UInt32			seqNumber;
    kern_return_t	kr = kIOReturnSuccess;

    bzero(inquiryData, sizeof(inquiryData));	// Zero data block.

    range[0].address = (IOVirtualAddress) inquiryData;
    range[0].length  = sizeof(inquiryData);

    bzero(&cdb, sizeof(cdb));
    cdb.cdbLength = 6;
    cdb.cdb[0] = kSCSICmdInquiry;
    cdb.cdb[4] = sizeof(inquiryData);

    kr = (*cdbCommandInterface)->setAndExecuteCommand(
                                    cdbCommandInterface,
                                    &cdb,
                                    sizeof(inquiryData),
                                    range,
                                    sizeof(range) / sizeof(range[0]),
                                    0, /* isWrite */
                                    0, /* timeoutMS */
                                    0, /* target */
                                    0, /* callback */
                                    0, /* refcon */
                                    &seqNumber);
                                    
    if (kr != kIOReturnSuccess && kr != kIOReturnUnderrun) {
        fprintf(stderr, "Couldn't execute a CDB command. (0x%08x)\n", kr);            
    }
    else {
        if (kr == kIOReturnUnderrun) {
            fprintf(stderr, "Command underrun.\n");
        }
        
        kr = (*cdbCommandInterface)->getResults(cdbCommandInterface, &results);
        
        if (kr != kIOReturnSuccess && kr != kIOReturnUnderrun) {
            fprintf(stderr, "Couldn't get results of a command. (0x%08x)\n", kr);            
        }
        else {
            if (kr == kIOReturnUnderrun) {
                fprintf(stderr, "getResults underrun.\n");
            }
            
            PrintSCSIInquiryDataUsingOldMethod((SCSIInquiry *) inquiryData, results.bytesTransferred);
        }
    }
}

void TestADeviceUsingOldMethod(IOSCSIDeviceInterface **interface)
{
    kern_return_t			kr = kIOReturnSuccess;
    IOCDBCommandInterface	**cdbCommandInterface = NULL;

    // Get and print cached device information.
    // Could examine cached information to verify this is the device to work with.
    GetInquiryDataUsingOldMethod(interface);

    // Open the device, attempt to get live information for it, then close it.
    kr = (*interface)->open(interface);
    if (kr != kIOReturnSuccess) {
        fprintf(stderr, "Error opening SCSI device. (0x%08x)\n", kr);            
    }
    else {
        // Create a CDB command interface and execute a command to query the device.
        cdbCommandInterface = CreateCommandInterfaceUsingOldMethod(interface);
        if (cdbCommandInterface) {
            ExecuteInquiryUsingOldMethod(cdbCommandInterface);
            (*cdbCommandInterface)->Release(cdbCommandInterface);
        }
        // Close the device we opened.
        (*interface)->close(interface);
    }
}

void TestDevicesUsingOldMethod(io_iterator_t iterator)
{
    io_service_t			scsiDevice = NULL;
    IOSCSIDeviceInterface	**interface = NULL;
    kern_return_t			kr = kIOReturnSuccess;

    while ((scsiDevice = IOIteratorNext(iterator))) {
    	// Get each device, then print cached info, then run live query.
    	CreateDeviceInterfaceUsingOldMethod(scsiDevice, &interface);
    	
    	kr = IOObjectRelease(scsiDevice); // Done with SCSI object from I/O Registry.
            
        if (kr != kIOReturnSuccess) {
            fprintf(stderr, "Error releasing SCSI device. (0x%08x)\n", kr);
        }
        
        if (interface != NULL) {
            TestADeviceUsingOldMethod(interface);
            (*interface)->Release(interface);
        }
    }    
}

#endif