/*
	File:		SoftVDigKEXT.h
	
	Description: KEXT simulating a compressed source device providing data to SoftVDigX.

	Author:		DTS

	Copyright: 	© Copyright 2003 Apple Computer, Inc. All rights reserved.
	
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

#include <IOKit/IOService.h>
#include <IOKit/IOUserClient.h>
#include <IOKit/ndrvsupport/IOMacOSTypes.h>	// for definition of Rect

#include "SoftVDigKEXTCommon.h"

/*
    This is the in-kernel driver (kext) class that we talk to through the UserClient
*/

// fFrameState constants
enum {
	kFrameBufferBusy = -1,
	kFrameBufferFree = 0,
	kFrameBufferReady = 1
};

#pragma mark ----- Driver class -----

class com_dts_iokit_SoftVDigKEXT : public IOService

{

/*
    Declare metaclass information used for runtime type checking of IOKit objects.
*/

    OSDeclareDefaultStructors(com_dts_iokit_SoftVDigKEXT);

friend class SoftVDigKEXTUserClient;

    private:
        // private data and methods go here
            
    protected:
        // protected data and methods go here
        
        void*				fMemBuf;
        UInt32				fFrameSize;
        UInt32				fFrameCounter;
		SInt32				fFrameState;	/*
												A tri-state flag:
													-1 = fMemBuf is busy, i.e. QT has it
													 0 = fMemBuf is free, i.e. QT is done
													 1 = fMemBuf is ready,i.e. has frame data in it
											*/
		UInt32				fFrameRate;
		
		
		IOWorkLoop*			fWorkLoop;		// our workloop
		IOTimerEventSource*	fTimer;			// used to simulate capture hardware
		IOCommandGate*		fCommandGate;	// for incoming userland requests, like CompressDone
        IOMemoryDescriptor*	fMemDesc;		// a memory descriptor
		task_t				fTask;			// the task that fMemBuf is mapped into
		
    public:
        // IOService overrides
		virtual bool 		start(IOService *provider);
        virtual void 		stop(IOService *provider);
		virtual IOWorkLoop* getWorkLoop();

        // Other methods
		
		// timeout handler
		static void timeout( OSObject *owner, IOTimerEventSource *sender );
		void captureFrame( void );	// 'capture' a frame


        // Methods called from userland via the user client

        IOReturn GetDigitizerRect( Rect* rect );
        IOReturn GetMaxSrcRect( Rect* rect );
        IOReturn CompressDone( UInt8* queuedFrameCount, Ptr* theData, long* dataSize );
		IOReturn ReleaseCompressBuffer( void );


};



/*
    This is the UserClient class that is used to talk to the driver (kext) from userland.
*/

#pragma mark ----- UserClient Class -----


class SoftVDigKEXTUserClient : public IOUserClient
{
    /*
        Declare metaclass information used for runtime type checking of IOKit objects.
    */
    
    OSDeclareDefaultStructors(SoftVDigKEXTUserClient);

    private:
        // private data and methods go here
        com_dts_iokit_SoftVDigKEXT*	fProvider;	// the driver we're attached to
        task_t						fTask;		// userland task that instanciated us
        
    public:
        // IOService overrides
        virtual bool start( IOService* provider );
        virtual void stop( IOService* provider );
        
        // IOUserClient overrides
        virtual bool initWithTask( task_t owningTask,
                                    void * securityID,
                                    UInt32 type,
                                    OSDictionary * properties );
        virtual IOReturn clientClose( void );
        virtual IOExternalMethod* getTargetAndMethodForIndex( IOService** targetP, UInt32 index );


        virtual IOReturn clientMemoryForType( UInt32 type, 
                                                IOOptionBits * options,
                                                IOMemoryDescriptor ** memory );



        // video capture methods (External methods)
		IOReturn CompressDone( UInt8* queuedFrameCount, Ptr* theData, long* dataSize );
		IOReturn ReleaseCompressBuffer( void );
		IOReturn SetCompressionOnOff( bool newState );
		IOReturn SetFrameRate( Fixed framesPerSecond );

};


#pragma mark ----- External Methods Table -----

// external methods table
static const IOExternalMethod externalMethods[kNumSoftVDigKEXTUCMethods] = 
{
// ----- These methods are in the userclient -----

    {

        // ::CompressDone( UInt8* queuedFrameCount, Ptr* theData, long* dataSize )
        NULL,
        (IOMethod)&SoftVDigKEXTUserClient::CompressDone,
        kIOUCScalarIScalarO,		// scalar in/out
        0,							// number of scalar inputs
        3							// number of scalar outputs
    },
	
	{
        // ::ReleaseCompressBuffer( void )
        NULL,
        (IOMethod)&SoftVDigKEXTUserClient::ReleaseCompressBuffer,
        kIOUCScalarIScalarO,		// scalar in/out
        0,							// number of scalar inputs
        0							// number of scalar outputs
    },


	{
        // ::SetCompressionOnOff( bool newState )
        NULL,
        (IOMethod)&SoftVDigKEXTUserClient::SetCompressionOnOff,
        kIOUCScalarIScalarO,		// scalar in/out
        1,							// number of scalar inputs
        0							// number of scalar outputs
    },

// ----- Methods below here are in the driver (com_dts_iokit_SoftVDigKEXT)-----

    {
        // ::GetDigitizerRect( Rect* rect )
        NULL,
        (IOMethod)&com_dts_iokit_SoftVDigKEXT::GetDigitizerRect,
        kIOUCStructIStructO,
        0,							// input struct size
        sizeof(Rect)				// output struct size
    },
    
    {
        // ::GetMaxSrcRect( Rect* rect )
        NULL,
        (IOMethod)&com_dts_iokit_SoftVDigKEXT::GetMaxSrcRect,
        kIOUCStructIStructO,
        0,							// input struct size
        sizeof(Rect)				// output struct size
    }
    	
};