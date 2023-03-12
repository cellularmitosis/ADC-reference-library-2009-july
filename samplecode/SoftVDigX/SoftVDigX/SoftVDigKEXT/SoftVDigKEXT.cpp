/*
	File:		SoftVDigKEXT.cpp
	
	Description: KEXT simulating a compressed source device providing data to SoftVDigX.

	Author:		DTS

	Copyright: 	© Copyright 2003 - 2005 Apple Computer, Inc. All rights reserved.
	
	Disclaimer:	IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc.
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
       <2>      08/10/05    dts     updated for 10.4 & Universal Binaries    
	   <1>		07/23/03	dts		updated and initial release for X
*/

/*
    This is the kext that matches to IOResources so we have something in the kernel to talk to through the UserClient.
*/

#include <IOKit/IOLib.h>
#include <IOKit/IOTimerEventSource.h>
#include <IOKit/IOWorkLoop.h>
#include <IOKit/IOCommandGate.h>

#include "SoftVDigKEXT.h"
#include "ImageData.h"


/*
extern "C" {
	#include <pexpert/pexpert.h>     //This is for debugging purposes ONLY
}
*/

#define	DEFAULT_FRAME_RATE	30	// default = 30 fps

// Define my superclass
#define super IOService

// REQUIRED! This macro defines the class's constructors, destructors,
// and several other methods I/O Kit requires.  Do NOT use super as the
// second parameter.  You must use the literal name of
// the superclass.

OSDefineMetaClassAndStructors(com_dts_iokit_SoftVDigKEXT, IOService);

// start method
bool com_dts_iokit_SoftVDigKEXT::start(IOService *provider)
{
	// call start for our superclass
    bool result = super::start(provider);
	
    // if start for our superclass was successful do our own startup
    if( result )
        do {
            /*
                Allocate a "capture buffer".
                
                For hardware that has on-board memory you would use mapDeviceMemoryWithRegister
                to map the on-board memory and use that memory as capture buffer(s).
            */
            
            fFrameSize = sizeof(frame_00);	// get frame size
        
            fMemBuf = IOMallocAligned(fFrameSize, 32);	// allocate a buffer
            
            if( !fMemBuf )
            {
                result = false;	// fail if we can't get buffer
                break;
            }
            
            // the frame buffer begins life in the free state
            fFrameState = kFrameBufferFree;

            /*	
                Create an IOMemoryDescriptor (IOMD) for the "capture buffer" memory in the kernel task.
                
                In the case of real hardware, mapDeviceMemoryWithRegister returns an IOMemoryMap.
                
                Calling getDeviceMemoryWithRegister returns an IODeviceMemory (subclass of IOMemoryDescriptor).
            */
             

            fMemDesc = IOMemoryDescriptor::withAddress( fMemBuf,
                                                        fFrameSize,
                                                        kIODirectionIn );	// set up memory for user-land read
        
            // fail start if we don't get a memory descriptor
            if( !fMemDesc )
            {
                IOLog( "couldn't get IOMemoryDescriptor?\n" );
                result = false;
                break;
            }
            
            /*
                Create a workloop for receiving events (like interrupts).
                
                In the case of real hardware that generates interrupts you do not need to create a workloop.
                
                Mac OS X creates a workloop for each hardware interrupt source, for example a PCI card.
                
                You can ask your provider, for example an IOPCIDevice, for its workloop by calling getWorkLoop.
            */
            
            fWorkLoop = IOWorkLoop::workLoop();	// since we have no real hardware create a workloop
            
            // fail start if we don't get a workloop
            if( !fWorkLoop )
            {
                IOLog( "couldn't get IOWorkLoop?\n" );
                result = false;
                break;
            }
        
            // set default frame rate
            fFrameRate = DEFAULT_FRAME_RATE;
                
            // create a timer event source to simulate hardware interrupts
            fTimer = IOTimerEventSource::timerEventSource( this, 
                                        (IOTimerEventSource::Action)&com_dts_iokit_SoftVDigKEXT::timeout );
            
            // fail start if we don't get a timer
            if( !fTimer )
            {
                IOLog( "couldn't get IOTimerEventSource?\n" );
                result = false;
                break;
            }

            /*
                We initially disable the timer. It is enabled/disabled by the SetCompressionOnOff routine of the userclient
                in response to calls to the vdig
            */
            
            fWorkLoop->addEventSource( fTimer );        // add the timer event source to the workloop
            fTimer->disable();                          // disable the timer initially
            fTimer->setTimeoutMS( 1000/fFrameRate );	// set the timeout
            
            
    /*
        Create an IOCommandGate for serializing user requests.
        
        Under 10.4 and later there is a macro, OSMemberFunctionCast, that will cast a member function to a callback type.
        If it's defined, we use it when creating our IOCommandGate. Otherwise we just cast our member function to the proper type.
        
        Note if you see "warning: control may reach end of non-void function" here this is a known warning and may safely be ignored.
    */

    #ifdef OSMemberFunctionCast

            IOCommandGate::Action myAction = OSMemberFunctionCast( IOCommandGate::Action, this, &com_dts_iokit_SoftVDigKEXT::CompressDone );
            fCommandGate = IOCommandGate::commandGate( this, myAction );	

    #else

            fCommandGate = IOCommandGate::commandGate( this, 
                                (IOCommandGate::Action)&com_dts_iokit_SoftVDigKEXT::CompressDone );	

    #endif

            // fail start if we don't get an IOCommandGate
            if( !fCommandGate )
            {
                IOLog( "couldn't get IOCommandGate?\n" );
                result = false;
                break;
            }

            // add the command gate to the work loop. a command gate is an IOEventSource
            fWorkLoop->addEventSource( fCommandGate );
            
            registerService();	// make us visible in the IORegistry and for matching by IOServiceGetMatchingServices
        
        } while( false );
    
	
	if( result != true )	// if we failed start for some reason
		stop( provider );	// call stop to clean up any objects we've instanciated
			
    return( result );
}


void com_dts_iokit_SoftVDigKEXT::stop(IOService *provider)
{
    // clean up any objects or memory we've allocated
        
		if( fWorkLoop )	// if we have a workloop remove any event sources then release the workloop
		{
			if( fCommandGate )
			{
				fCommandGate->disable();                        // disable the event source
				while( fCommandGate->onThread() );              // wait for any current command to complete
				fWorkLoop->removeEventSource( fCommandGate );	// remove it from the workloop
				fCommandGate->release();                        // release the command gate
				fCommandGate = 0;
			}
			
			if( fTimer )
			{
				fTimer->cancelTimeout();                // disable any outstanding calls
				fTimer->disable();                      // disable the timer callback
				fWorkLoop->removeEventSource( fTimer );	// remove timer from the workloop
				fTimer->release();                      // release the timer
				fTimer = 0;
			}
			
			// now that all the event sources are removed, release the workloop
			
			fWorkLoop->release();	// release the workloop
			fWorkLoop = 0;
		
		}
		
		// clean up our IOMemoryDescriptor
		if( fMemDesc )
		{
			fMemDesc->release();
			fMemDesc = 0;
		}
		
        // release our buffer
        if( fMemBuf )
		{
            IOFreeAligned( fMemBuf, fFrameSize );	// must use appropriate IOFree... call
			fMemBuf = 0;
		}
            
	super::stop(provider);
}

// timeout handler, called when the timer fires
void com_dts_iokit_SoftVDigKEXT::timeout( OSObject *owner, IOTimerEventSource *sender )
{
	com_dts_iokit_SoftVDigKEXT*	me;
	
	me = OSDynamicCast( com_dts_iokit_SoftVDigKEXT, owner );	// make sure that it's us
	if( me )
	{
        /*
            This timer is used to simulate a hardware interrupt indicating we have "captured" a frame
		
            the workloop gate is closed before we are called
        */
        
		me->captureFrame();	// call the method that copies the next frame into fMemBuf
	}
}

// 'capture' a frame of video
void com_dts_iokit_SoftVDigKEXT::captureFrame( void )
{
    /*
        If fMemBuf is not being used by QuickTime, "capture" the next frame.
        We don't care if this frame hasn't been grabbed by QuickTime.
        Log info if a frame is dropped.
    */

    fTimer->setTimeoutMS( 1000/fFrameRate );	// reset the timer so we get called again
    

    switch( fFrameState )
    {
        /*
            If the frame buffer is busy, then QuickTime currently owns it and we can't modify it.
            This means we dropped the next frame (the frame that should be 'captured' now) since the frame buffer is busy.
            Log the dropped frame in system.log.
        */
        case kFrameBufferBusy:
            IOLog( "SoftVDigKEXT - Dropped frame: %ld\n", fFrameCounter+1 );
            break;

        /*
            If the frame buffer is ready, then QuickTime failed to grab the frame before
            we 'captured' the current frame.
            This means we dropped the frame that is in the frame buffer.
            Log the dropped frame in system.log and copy the next frame into the frame buffer
        */
        case kFrameBufferReady:
            IOLog( "SoftVDigKEXT - Dropped frame: %ld\n", fFrameCounter );
            memcpy( fMemBuf, frame[fFrameCounter % NUM_FRAMES], fFrameSize );
            break;

        /*
            If the frame buffer is free, then QuickTime was able to grab the frame before the next frame arrived.
            Copy the next frame into the frame buffer and set the frame buffer state to ready.
        */
        case kFrameBufferFree:
            memcpy( fMemBuf, frame[fFrameCounter % NUM_FRAMES], fFrameSize );
            fFrameState = kFrameBufferReady;
            break;
            
        /*
            We should never get here. If we ever do, please file a bug against this sample and let DTS know. Thanks.
        */
        default:
            IOLog( "default case?!?\n" );
    
    }

    // increment frame counter every time
    fFrameCounter++;
}


IOWorkLoop* com_dts_iokit_SoftVDigKEXT::getWorkLoop()
{
	/*
        We supply our own workloop because we don't have any real hardware.
        We override this method here in case one of our clients asks for our workloop.
        In the case of real hardware you don't need to override this method.
    */
    
	return( fWorkLoop );
}

#pragma mark ------------------------------------------------------------

/*
    Methods called from userland via the user client:
*/

IOReturn com_dts_iokit_SoftVDigKEXT::GetDigitizerRect( Rect* rect )
{
	// QuickTime calls SoftVDigXGetDigitizerRect in the vdig, which calls us

    //IOLog( "GetDigitizerRect\n" );
    rect->top		= 0;
    rect->left		= 0;
    rect->bottom	= IMAGE_HEIGHT;
    rect->right		= IMAGE_WIDTH;
    
    return( kIOReturnSuccess );	// returned from IOConnectMethodStructureIStructureO in user land
}

IOReturn com_dts_iokit_SoftVDigKEXT::GetMaxSrcRect( Rect* rect )
{
	// QuickTime calls SoftVDigXGetMaxSrcRect in the vdig, which calls us

    //IOLog( "GetMaxSrcRect\n" );
    rect->top		= 0;
    rect->left		= 0;
    rect->bottom	= IMAGE_HEIGHT;
    rect->right		= IMAGE_WIDTH;
    
    return( kIOReturnSuccess );	// returned from IOConnectMethodScalarIScalarO in user land
}



IOReturn com_dts_iokit_SoftVDigKEXT::CompressDone( UInt8* queuedFrameCount, Ptr* theData, long* dataSize )
{

	/*
		QuickTime calls this to ask if we have a frame ready. This example shows calling through to the driver, which
		is probably not the most efficient way to do this. A better way might be a small area of memory mapped
		into the userland task that is used as flags to indicate the state of the buffer(s). That would save a
		great number of calls into the kernel which can be expensive.
	*/
	
	IOReturn		result = kIOReturnSuccess;
	IOMemoryMap*	tempMap;
	
	// if there's a frame available return it
	if( fFrameState == kFrameBufferReady )
	{	
        /* 
            fMemBuf was mapped into user land by the vdig - see IOConnectMapMemory.
            Since a mapping already exists, we ask for any existing mapping in the user task (fTask)
            in order to get the user space address of the mapped memory.
        */
		tempMap = fMemDesc->map( fTask, NULL, kIOMapAnywhere | kIOMapReference );	// get any existing mapping
		if(tempMap)
		{
//			IOLog( "tempMap: %p addr: %p\n", tempMap, tempMap->getVirtualAddress() );
			
			*theData = (Ptr)tempMap->getVirtualAddress();	// get the address in userspace
			tempMap->release();								// release this reference
			*queuedFrameCount = 1;							// we currently only support one frame at a time
			*dataSize = fFrameSize;
			fFrameState = kFrameBufferBusy;					// QT now has control of fMemBuf
			
			//IOLog( "SoftVDigKEXT - FrameCount: %d theData: %p dataSize: %ld\n", *queuedFrameCount, theData, *dataSize );
		}	
		
		else	// mapping doesn't exist???
		{
			IOLog( "Didn't get map\n" );
			*queuedFrameCount = 0;
			*theData = NULL;		
			*dataSize = 0;
			result = kIOReturnNoMemory;	// pass back an error
		}
	}
	
	else
	{
		// no frame is ready yet
		*queuedFrameCount = 0;
		*theData = NULL;
		*dataSize = 0;
	}
	
    return( result );	// returned from IOConnectMethodScalarIScalarO in user land
}

IOReturn com_dts_iokit_SoftVDigKEXT::ReleaseCompressBuffer( void )
{
//	IOLog( "ReleaseCompressBuffer\n" );

	// make fMemBuf available for capturing
	if ( fFrameState == kFrameBufferBusy ) {
		// IOLog( "ReleaseCompressBuffer: about to relase a non-busy frame\n" );
		fFrameState = kFrameBufferFree;
	}
	
	return( kIOReturnSuccess );
}