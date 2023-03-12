/*
	File:		DataQueueUserClient.cpp
	
	Description:	This file creates an IODataQueue which can be used to pass data between a KEXT and
                        a user space application.

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


#include <IOKit/IOLib.h>
#include <IOKit/IODataQueueShared.h>
#include "DataQueueUserClient.h"
#include "SimpleDataQueue.h"
#include "SharedData.h"




/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
#define super IOUserClient
OSDefineMetaClassAndStructors(com_apple_dts_DataQueueUserClient, IOUserClient)

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */




void
com_apple_dts_DataQueueUserClient::myTimeoutHandler(OSObject *owner, IOTimerEventSource *sender)
{
    com_apple_dts_DataQueueUserClient * me;
    bool result;
    
    // Make sure that the owner of the timer is us.
    me = OSDynamicCast(com_apple_dts_DataQueueUserClient, owner);
    
    if (!me)
        return;
        
    // Add the counter value to the IODataQueue so the user space application can retrieve it.
    result = me->fDataQueue->enqueue(me->fProvider->getCount(), sizeof(UInt8));
    
    if (!result)
        IOLog("Enqueue of %d failed.\n", *me->fProvider->getCount());
    
    me->fProvider->incrementCount();
    
    // reset the timer for 1 second
    sender->setTimeoutMS(1000);
}





IOReturn
com_apple_dts_DataQueueUserClient::clientMemoryForType(UInt32 type, IOOptionBits * options, IOMemoryDescriptor ** memory)
{
    IOReturn result = kIOReturnNoMemory;
        
    IOLog("DataQueueUserClient::clientMemoryForType()\n");
    
    *memory = NULL;
    *options = 0;
    
    switch (type)
    {
        case kIODefaultMemoryType:
            if (!fMemoryToShare)
                return kIOReturnNoMemory;
            
            // You don't need to balance with a release() because the user client does the release() for you.
            fMemoryToShare->retain();
            
            *memory = fMemoryToShare;
            *options = 0;
            
            result = kIOReturnSuccess;
            break;
        default:
            IOLog("DataQueueUserClient - Unknown memory type %ld\n", type);
            break;
    }
    
    return result;
}




 
IOReturn
com_apple_dts_DataQueueUserClient::registerNotificationPort(mach_port_t port, UInt32 type, UInt32 refCon)
{    
    IOLog("DataQueueUserClient::registerNotificationPort()\n");
    
    if (!fDataQueue)
        return kIOReturnError;
        
    // Set the notification port of our IODataQueue object.  This is the port that will
    // get notified when we enqueue data into our IODataQueue.  The value of 'port' is
    // passed in when calling IOConnectSetNotificationPort() from user space.
    fDataQueue->setNotificationPort(port);
    
    return kIOReturnSuccess;
}




bool
com_apple_dts_DataQueueUserClient::initWithTask(task_t owningTask, void *security_id , UInt32 type)
{
    IOLog("DataQueueUserClient::initWithTask()\n");
    
    if (!super::initWithTask(owningTask, security_id , type))
        return false;
    
    if (!owningTask)
	return false;
	
    fTask = owningTask;
    
    myWorkLoop = NULL;
    fProvider = NULL;
    fDataQueue = NULL;
    myTimer = NULL;
    fMemoryToShare = NULL;

    return true;
}




bool
com_apple_dts_DataQueueUserClient::myAddTimerToWorkLoop()
{
    myWorkLoop = getWorkLoop();
    
    if (!myWorkLoop)
        return false;
	
    // get a timer and set our timeout handler to be called when it fires
    myTimer = IOTimerEventSource::timerEventSource(this,
        (IOTimerEventSource::Action)&com_apple_dts_DataQueueUserClient::myTimeoutHandler);
    
    if (!myTimer)
        return false;
    
    // add the timer to the workloop
    if (myWorkLoop->addEventSource(myTimer) != kIOReturnSuccess)
        return false;

    // now set the timeout, this example uses 1 second
    myTimer->setTimeoutMS(1000);
    
    return true;
}




void
com_apple_dts_DataQueueUserClient::myRemoveTimerFromWorkLoop()
{
    if(myTimer)
    {
        myTimer->cancelTimeout();
        myWorkLoop->removeEventSource(myTimer);
        myTimer->release();
        myTimer = NULL;
    }
}




bool 
com_apple_dts_DataQueueUserClient::start(IOService * provider)
{
    bool ret;
    
    IOLog("DataQueueUserClient::start()\n");
    
    if (!super::start(provider))
        return false;
    
    // Make sure our provider is com_apple_dts_SimpleDataQueue.
    fProvider = OSDynamicCast(com_apple_dts_SimpleDataQueue, provider);
    
    if (!fProvider)
	return false;
    
    // Create and initialize an IODataQueue object.
    fDataQueue = IODataQueue::withCapacity(32 + DATA_QUEUE_ENTRY_HEADER_SIZE);
    
    if (!fDataQueue)
        return false;
        
    // Returns a pointer to an IOMemoryDescriptor that refers to the buffer inside our IODataQueue object.
    fMemoryToShare = fDataQueue->getMemoryDescriptor();
    
    if (!fMemoryToShare)
    {
        // Release the IODataQueue object.
        fDataQueue->release();
        fDataQueue = NULL;
        
        return false;
    }    
    
    // This adds a timer to the workloop.  This timer will be used to simulate events, and then add
    // those events to an IODataQueue object so that they can be accessed from our user space application.
    ret = myAddTimerToWorkLoop();

    return ret;
}




IOReturn 
com_apple_dts_DataQueueUserClient::clientClose(void)
{
    IOLog("DataQueueUserClient::clientClose()\n");
        
    fProvider = NULL;    
    terminate();

    return kIOReturnSuccess;
}




IOReturn 
com_apple_dts_DataQueueUserClient::clientDied(void)
{
    IOReturn ret;

    IOLog("DataQueueUserClient::clientDied()\n");

    // this just calls clientClose
    ret = super::clientDied();

    return ret;
}




void 
com_apple_dts_DataQueueUserClient::stop(IOService * provider)
{
    UInt8 message = kMyStopListeningMessage;
        
    IOLog("DataQueueUserClient::stop()\n");

    myRemoveTimerFromWorkLoop();
    
    // Tell the user space application to stop listening for events.
    fDataQueue->enqueue(&message, sizeof(message));
    
    if (fMemoryToShare)
    {
        // Release the shared IOMemoryDescriptor.
        fMemoryToShare->release();
        fMemoryToShare = NULL;
    }
    
    if (fDataQueue)
    {
        // Release the IODataQueue object.
        fDataQueue->release();
        fDataQueue = NULL;
    }
    
    super::stop(provider);
}




IOReturn 
com_apple_dts_DataQueueUserClient::message(UInt32 type, IOService * provider, void * argument = 0)
{
    IOLog("DataQueueUserClient::message()\n");
    
    switch ( type )
    {
        default:
            break;
    }
    
    return super::message(type, provider, argument);
}




bool 
com_apple_dts_DataQueueUserClient::finalize( IOOptionBits options )
{
    bool ret;
    
    IOLog("DataQueueUserClient::finalize()\n");
    
    ret = super::finalize(options);
    
    return ret;
}




bool 
com_apple_dts_DataQueueUserClient::terminate( IOOptionBits options )
{
    bool ret;
    
    IOLog("DataQueueUserClient::terminate()\n");

    ret = super::terminate(options);
    
    return ret;
}