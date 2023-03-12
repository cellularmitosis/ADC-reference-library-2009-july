/*
	File:		SoftVDigKEXT_UC.cpp
	
	Description: KEXT User Client interface for SoftVDigX.

	Author:		DTS

	Copyright: 	© Copyright 2003 - 2005 Apple Computer, Inc. All rights reserved.
	
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
       <2>      08/10/05    dts      updated for 10.4 & Universal Binaries
	   <1>		07/23/03	dts		updated and initial release for X
*/

#include "SoftVDigKEXT.h"
#include <IOKit/IOLib.h>
#include <IOKit/IOCommandGate.h>
#include <IOKit/IOTimerEventSource.h>

#define super IOUserClient
OSDefineMetaClassAndStructors( SoftVDigKEXTUserClient, IOUserClient );

/* 
 * Since this sample uses the IOUserClientClass property, the SoftVDigKEXTUserClient
 * is created automatically in response to IOServiceOpen(). More complex applications
 * might have several kinds of clients each with a differnt IOUserClient subclass,
 * with different enumerated types. In that case the SoftVDigKEXT class must implement
 * the newUserClient() method (see IOService.h headerdoc).
 */

bool SoftVDigKEXTUserClient::initWithTask( task_t owningTask,
                                        void * securityID,
                                        UInt32 type,
                                        OSDictionary * properties )
{
    //IOLog("SoftVDigKEXTUserClient::initWithTask(type %ld)\n", type);
    
    fTask = owningTask;	// remember who instanciated us
	
    return( super::initWithTask( owningTask, securityID, type, properties ));
}


bool SoftVDigKEXTUserClient::start( IOService* provider )
{
    bool	result = true;
    
    
    //IOLog( "SoftVDigKEXTUserClient::start\n" );
    
    if( !super::start( provider ) )
        return( false );
    
    /*
        our expected provider is an SoftVDigKEXT object
    */
    
    // see if it's the correct class and remember it at the same time
    fProvider = OSDynamicCast( com_dts_iokit_SoftVDigKEXT, provider );
    
    if( !fProvider )
    {
        IOLog( "SoftVDigKEXT - Provider is not a com_dts_iokit_SoftVDigKEXT object\n" );
        result = false;
    }
	
	// the driver needs fTask to get the userspace address for CompressDone
	
	fProvider->fTask = fTask;

    return( result );
}

IOReturn SoftVDigKEXTUserClient::clientMemoryForType( UInt32 type,
													IOOptionBits * options,
													IOMemoryDescriptor ** memory )
{
    //IOLog( "clientMemoryForType\n" );

    IOReturn	result = kIOReturnSuccess;
    
	
	if( fProvider->fMemDesc )
	{
	/*
		We must retain the IOMD we pass back because the caller is going to call
		release on it. This is because ::clientMemoryForType will be called again
		when the userland task calls IOConnectUnmapMemory and we have to pass back
		the same IOMD for a given value of the type parameter.
	*/

		fProvider->fMemDesc->retain();
	
		*memory = fProvider->fMemDesc;	// pass back the IOMD (if any)
	}
	
	else
		result = kIOReturnNoMemory;
		
    return( result );
}


void SoftVDigKEXTUserClient::stop( IOService* provider )
{
    //IOLog( "SoftVDigKEXTUserClient::stop\n" );
    
    super::stop( provider );
}


/*
    clientClose
    
    the default implementation of clientDied() just calls clientClose()

*/
IOReturn SoftVDigKEXTUserClient::clientClose( void )
{
    IOReturn	result = kIOReturnSuccess;
    
    //IOLog( "SoftVDigKEXTUserClient::clientClose\n" );
    
    fTask = NULL;
    fProvider = NULL;
    terminate();
    
    return( result );

}

IOExternalMethod* SoftVDigKEXTUserClient::getTargetAndMethodForIndex(IOService** targetP,
                                                                     UInt32 index )
{
    // if the index is >= kGetDigitizerRect we're calling a method in the driver
    // so return a pointer to the driver in targetP
    if( index >= kGetDigitizerRect )
        *targetP = fProvider;
    
    // otherwise we're calling a method in the userclient so return this in targetP
    else
        *targetP = this;	// we are the target of all our external methods
    
    // validate index and return the appropriate IOExternalMethod
    if( index < kNumSoftVDigKEXTUCMethods )
        return( (IOExternalMethod*)(externalMethods + index ) );
    
    else
        return( NULL );
}


IOReturn SoftVDigKEXTUserClient::CompressDone( UInt8* queuedFrameCount, Ptr* theData, long* dataSize )
{
	IOReturn	result;

	// call the driver using the command gate
	result = fProvider->fCommandGate->runCommand( queuedFrameCount, theData, dataSize );

	return( result );
}

IOReturn SoftVDigKEXTUserClient::ReleaseCompressBuffer( void )
{
//	IOLog( "ReleaseCompressBuffer\n" );
	IOReturn result;

/*
    Under 10.4 and later there's a macro, OSMemberFunctionCast, that will cast a member function to a callback type.
    If it's defined, we use it when creating our IOCommandGate. Otherwise we just cast our member function to the proper type.
    
    Note if you see "warning: control may reach end of non-void function" here this is a known warning and may safely be ignored.
*/

#ifdef OSMemberFunctionCast

	result = ( fProvider->fCommandGate->runAction( OSMemberFunctionCast( IOCommandGate::Action, this, &com_dts_iokit_SoftVDigKEXT::ReleaseCompressBuffer) ));

#else

	result = ( fProvider->fCommandGate->runAction( (IOCommandGate::Action)&com_dts_iokit_SoftVDigKEXT::ReleaseCompressBuffer ));

#endif
	
	return( result );
}


IOReturn SoftVDigKEXTUserClient::SetCompressionOnOff( bool newState )
{
	if( newState )
	{
        /*
            newState == true means turn capture ON
            Log a message and enable the timer in the driver to run so we can 'capture'.
        */
        
		IOLog( "SoftVDigKEXT - Capture ON\n" );
		fProvider->fTimer->enable();
	}
    
	else
	{
        /*
            newState == false means turn capture OFF
            Log a message, disable the timer in the driver, and reset the frame counter.
        */
        
		IOLog( "SoftVDigKEXT - Capture OFF\n" );
		fProvider->fTimer->disable();
		fProvider->fFrameCounter = 0;	// always start from frame 0
	}
	
	return( kIOReturnSuccess );
}