/*
	File:		SimpleDataQueueTool.c
	
	Description:	This file shows how to setup a shared memory region with a KEXT which allows
                        a user space application to dequeue items that were placed into an IODataQueue.

	Author:		MK

	Copyright: 	© Copyright 2001 Apple Computer, Inc. All rights reserved.
	
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
                    1.0d1

*/


#include <IOKit/IOKitLib.h>
#include <IOKit/IODataQueueShared.h>
#include <IOKit/IODataQueueClient.h>
#include <ApplicationServices/ApplicationServices.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include "SharedData.h"

#define kMyDriversIOKitClassName 	"com_apple_dts_SimpleDataQueue"
#define kMyPathToSystemLog 		"/var/log/system.log"


static IOReturn
MyListenForData(io_connect_t connection)
{
    kern_return_t		kernResult; 
    UInt8 			data;
    UInt32 			dataSize = sizeof(UInt8);
    IODataQueueMemory * 	queueMappedMemory;
    vm_size_t 			queueMappedMemorySize;
    vm_address_t		address = nil;
    vm_size_t			size = 0;
    unsigned int		msgType = 1;
    mach_port_t			recvPort;
        
    // Allocates and returns a new mach port able to receive data available notifications from an IODataQueue.
    recvPort = IODataQueueAllocateNotificationPort();
    
    if (!recvPort)
    {
        printf("IODataQueueAllocateNotificationPort returned a NULL mach_port_t\n");
        return kIOReturnError;
    }
    
    // IOConnectSetNotificationPort will call registerNotificationPort() inside your user client class.
    kernResult = IOConnectSetNotificationPort(connection, msgType, recvPort, 0);
    
    if (kernResult != kIOReturnSuccess)
    {
        printf("IOConnectSetNotificationPort returned %d\n", kernResult);
        
        mach_port_destroy(mach_task_self(), recvPort);
        
        return kernResult;
    }
    
    // IOConnectMapMemory will call clientMemoryForType() inside your user client class.
    kernResult = IOConnectMapMemory(connection, kIODefaultMemoryType, mach_task_self(), &address, &size, kIOMapAnywhere);
    
    if (kernResult != kIOReturnSuccess)
    {
        printf("IOConnectMapMemory returned %d\n", kernResult );
        
        mach_port_destroy(mach_task_self(), recvPort);

        return kernResult;
    }
    
    queueMappedMemory = (IODataQueueMemory *) address;
    queueMappedMemorySize = size;    

    
    // IODataQueueWaitForAvailableData doesn't return until there's data on the recvPort.
    while (IODataQueueWaitForAvailableData(queueMappedMemory, recvPort) == kIOReturnSuccess)
    {            
        // IODataQueueDataAvailable returns true when the shared memory contains data to dequeue.
        while (IODataQueueDataAvailable(queueMappedMemory))
        {   
            // Dequeues the next available entry on the queue and copies it into the given data pointer.             
            kernResult = IODataQueueDequeue(queueMappedMemory, &data, &dataSize);
            
            if (kernResult == kIOReturnSuccess)
            {
                // The KEXT is telling us to stop listening, so exit.
                if (data == kMyStopListeningMessage) goto exit;
            
                printf("Data = %d, Size = %ld, Result = %d\n", data, dataSize, kernResult);
                fflush(stdout);
            }
            else
            {
                printf("IODataQueueDequeue returned %d\n", kernResult );
            }
        }
    }


exit:
    
    // IOConnectUnmapMemory will call clientMemoryForType() inside your user client class.
    kernResult = IOConnectUnmapMemory(connection, kIODefaultMemoryType, mach_task_self(), address);
    
    if (kernResult != kIOReturnSuccess)
    {
        printf("IOConnectUnmapMemory returned %d\n", kernResult);
    }
    
    mach_port_destroy(mach_task_self(), recvPort);
    
    return kernResult;
}




static void
MyLaunchConsoleApp()
{
    CFURLRef pathRef;

    pathRef = CFURLCreateWithString(NULL, CFSTR(kMyPathToSystemLog), NULL);
    
    if (pathRef)
    {
        LSOpenCFURLRef(pathRef, NULL);
        CFRelease(pathRef);
    }
}




int
main(int argc, char* argv[])
{
    kern_return_t	kernResult; 
    mach_port_t		masterPort;
    io_iterator_t	iterator;
    io_service_t	serviceObject;
    CFDictionaryRef	classToMatch;
    pthread_t 		dataQueueThread;
    io_connect_t 	connection;
    int			err;
    
    
    // This will launch the Console.app so you can see the IOLogs from the KEXT.
    MyLaunchConsoleApp();
    
    
    // Returns the mach port used to initiate communication with IOKit.
    kernResult = IOMasterPort(MACH_PORT_NULL, &masterPort);
    
    if (kernResult != kIOReturnSuccess)
    {
        printf("IOMasterPort returned %d\n", kernResult);
        return 0;
    }
    
    classToMatch = IOServiceMatching(kMyDriversIOKitClassName);
    
    if (classToMatch == NULL)
    {
        printf("IOServiceMatching returned a NULL dictionary.\n");
        return 0;
    }
    
    
    // This creates an io_iterator_t of all instances of our drivers class that exist in the IORegistry.
    kernResult = IOServiceGetMatchingServices(masterPort, classToMatch, &iterator);
    
    if (kernResult != kIOReturnSuccess)
    {
        printf("IOServiceGetMatchingServices returned %d\n", kernResult);
        return 0;
    }
    
    
    // Get the first item in the iterator.
    serviceObject = IOIteratorNext(iterator);
    
    // Release the io_iterator_t now that we're done with it.
    IOObjectRelease(iterator);
    
        
    if (!serviceObject)
    {
        printf("Couldn't find any matches.\n");
        return 0;
    }
    
    // This call will cause the user client to be instantiated.
    kernResult = IOServiceOpen(serviceObject, mach_task_self(), 0, &connection);
    
    IOObjectRelease(serviceObject);
    
    if (kernResult != kIOReturnSuccess)
    {
        printf("IOServiceOpen returned %d\n", kernResult);
        return 0;
    }
    
    
    // This moves the IODataQueue listener to a separate thread.
    err = pthread_create(&dataQueueThread, NULL, (void *)MyListenForData, (void *)connection);
    
    if (err != noErr)
    {
        printf( "pthread_create returned %d\n", err);
        goto close;
    }
    
    
            
    /*****This is where you would do other things while waiting for kernel events.*****/
    
    
    
    // This waits for the listener thread to exit.
    pthread_join(dataQueueThread, (void **)&kernResult);


close:

    // Close a connection to an IOService and destroy the connect handle.
    kernResult = IOServiceClose(connection);
    
    if (kernResult != kIOReturnSuccess)
    {
        printf("IOServiceClose returned %d\n", kernResult);
    }
  
                  
    return 0;
}