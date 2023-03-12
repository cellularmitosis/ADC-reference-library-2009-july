/*
	File:		simpleAVCSample.c
	
	Description:	Sample showing use of the IOFireWireAVCFamily

	Author:		<JAS>

	Copyright: 	© Copyright 2002 Apple Computer, Inc. All rights reserved.
	
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
	When		Who	Version	What
	====		===	=======	====
	04/04/02	JAS	1.0	Created

*/

#include <stdio.h>
#include <IOKit/IOKitLib.h>
#include <IOKit/IOCFPlugin.h>
#include <IOKit/avc/IOFireWireAVCLib.h>
#include <IOKit/avc/IOFireWireAVCConsts.h>
#include <CoreFoundation/CoreFoundation.h>
#include <unistd.h>

#include "simpleAVCSample.h"

// prototypes
void DisplayAVCResponse( UInt8* response, UInt32 responseLen );
void DisplayUnitInfo( UInt8* response, UInt32 responseLen );
void DisplaySubunitInfo( UInt8* response, UInt32 responseLen );
void Wait10Seconds( void );
CFMutableDictionaryRef CreateMatchingDictionary( void );


int main (int argc, const char * argv[])
{

    kern_return_t			result;
    mach_port_t				masterPort;
    CFMutableDictionaryRef	matchingDictionary;
    io_iterator_t			iter;
    io_object_t				service;
    
    
    // get the master port for talking with IOKit
    result = IOMasterPort( MACH_PORT_NULL, &masterPort );

    // create a matching dictionary for AVC units
    matchingDictionary = CreateMatchingDictionary();
	
    // find all IOServices of class IOFireWireAVCUnit that use AVC protocol
    result = IOServiceGetMatchingServices( masterPort, matchingDictionary, &iter );
    
	/*
	    Note:
	    IOServiceGetMatchingServices consumes one reference to the dictionary passed in.
	    Since there is only one reference to our dictionary it will automatically be destroyed.
	    This is why we don't release the dictionary later when we clean up.
	*/
	
    // try opening a userclient to send commands to the first device
    if( service = IOIteratorNext( iter ) )
    {
	IOCFPlugInInterface**	interface;
	SInt32			score = 0;

	// get the IUnknown interface for the IOFireWireAVCUnit
	result = IOCreatePlugInInterfaceForService( service,
						    kIOFireWireAVCLibUnitTypeID,
						    kIOCFPlugInInterfaceID,
						    &interface,
						    &score );
							
	// if we got the IUnknown interface
	if( (kIOReturnSuccess == result) && interface ) 
	{
	    IOFireWireAVCLibUnitInterface**	avcUnit;
	    
	    // ask for the AVC unit interface
	    result = (*interface)->QueryInterface( interface,
						    CFUUIDGetUUIDBytes( kIOFireWireAVCLibUnitInterfaceID ),
						    (LPVOID)&avcUnit );

	    // if we got the AVC unit interface
	    if( kIOReturnSuccess == result )
	    {
		UInt8	command[20];
		UInt32	cmdLen = 0;
		UInt8	response[256];
		UInt32	responseLen = 0;
		
		
		// open the AVC unit interface
		(*avcUnit)->open( avcUnit );
		
		// set up the UNIT INFO command
		command[kAVCCommandResponse]	= kAVCStatusInquiryCommand;
		command[kAVCAddress] 		= 0xFF;
		command[kAVCOpcode]		= kAVCUnitInfoOpcode;
		command[kAVCOperand0]		= 0xFF;
		command[kAVCOperand1]		= 0xFF;
		command[kAVCOperand2]		= 0xFF;
		command[kAVCOperand3]		= 0xFF;
		command[kAVCOperand4]		= 0xFF;
		
		cmdLen = 8;
		
		responseLen = sizeof( response );
		
		// try sending a UNIT INFO command
		result = (*avcUnit)->AVCCommand( avcUnit,
						command,
						cmdLen,
						response,
						&responseLen );
		
		printf( "\nUNIT INFO result: 0x%08X responseLen: %ld\n", result, responseLen );
		
		if( kIOReturnSuccess == result )
		{
		    // show info returned in the UNIT INFO response
		    DisplayAVCResponse( response, responseLen );
		    DisplayUnitInfo( response, responseLen );
		}
		
		// set up the SUBUNIT INFO command
		command[kAVCCommandResponse]	= kAVCStatusInquiryCommand;
		command[kAVCAddress] 		= 0xFF;
		command[kAVCOpcode]		= kAVCSubunitInfoOpcode;
		command[kAVCOperand0]		= 0x07;
		command[kAVCOperand1]		= 0xFF;
		command[kAVCOperand2]		= 0xFF;
		command[kAVCOperand3]		= 0xFF;
		command[kAVCOperand4]		= 0xFF;
		
		cmdLen = 8;

		responseLen = sizeof( response );
		
		// try sending the SUBUNIT INFO command
		result = (*avcUnit)->AVCCommand( avcUnit,
						command,
						cmdLen,
						response,
						&responseLen );
						
		printf( "\nSUBUNIT INFO result: 0x%08X responseLen: %ld\n", result, responseLen );
		
		if( kIOReturnSuccess == result )
		{
		    // show info returned by the SUBUNIT INFO command
		    DisplayAVCResponse( response, responseLen );
		    DisplaySubunitInfo( response, responseLen );
		}
		
		// try sending a PLAY command to a VCR
		// set up the PLAY command
		command[kAVCCommandResponse]	= kAVCControlCommand;
		command[kAVCAddress] 		= (kAVCTapeRecorder << 3) | 0;
		command[kAVCOpcode]		= 0xC3;	// PLAY
		command[kAVCOperand0]		= 0x75;	// forward normal speed
		
		cmdLen = 4;

		responseLen = sizeof( response );
		
		result = (*avcUnit)->AVCCommand( avcUnit,
						command,
						cmdLen,
						response,
						&responseLen );
		
		printf( "\nPLAY result: 0x%08X\n", result );
		if( kIOReturnSuccess == result )
		{
		    DisplayAVCResponse( response, responseLen );
		    if( response[kAVCCommandResponse] == kAVCAcceptedStatus )
			Wait10Seconds();
		}
		
		
		// set up the WIND command
		command[kAVCCommandResponse]	= kAVCControlCommand;
		command[kAVCAddress] 		= (kAVCTapeRecorder << 3) | 0;
		command[kAVCOpcode]		= 0xC4;	// WIND
		command[kAVCOperand0]		= 0x60;	// stop all transport motion
		
		cmdLen = 4;

		responseLen = sizeof( response );
		
		// try sending a WIND STOP command
		result = (*avcUnit)->AVCCommand( avcUnit,
						command,
						cmdLen,
						response,
						&responseLen );
		
		printf( "\nWIND STOP result: 0x%08X\n", result );

		if( kIOReturnSuccess == result )
			DisplayAVCResponse( response, responseLen );
		
		// close the AVC unit interface
		(*avcUnit)->close( avcUnit );
		
		// release the AVC unit interface
		(*avcUnit)->Release( avcUnit );		
	    }
	}
						    
	// clean up
    
	if( interface )
	    result = IODestroyPlugInInterface( interface );
    }
    
    else	// guess we did not find any AVC units
	printf( "\n***Did not find any AVC devices***\n" );
	
    return 0;
}


// display the response from the device
void DisplayAVCResponse( UInt8* response, UInt32 responseLen )
{
    printf( "\tresponse: 0x%02X %s\n", response[0] & 0x0F, responseStr[response[0] & 0x0F] );
    printf( "\tsubunit_type: 0x%02X\n", IOAVCType(response[1]) );
    printf( "\tsubunit_ID: 0x%02X\n", IOAVCId(response[1]) );
    printf( "\topcode: 0x%02X\n", response[2] );
}

// display the unit info
void DisplayUnitInfo( UInt8* response, UInt32 responseLen )
{
    printf( "\toperand0: 0x%02X\n", response[kAVCOperand0] );
    printf( "\tunit_type: 0x%02X %s\n", IOAVCType(response[kAVCOperand1]),
					unitTypeStr[IOAVCType(response[kAVCOperand1])] );
    printf( "\tunit: 0x%02X\n", IOAVCId(response[kAVCOperand1]) );
    printf( "\tcompany ID: 0x%02X%02X%02X\n", response[kAVCOperand2],
					    response[kAVCOperand3],
					    response[kAVCOperand4] );
}


// display info about the first 4 subunits
void DisplaySubunitInfo( UInt8* response, UInt32 responseLen )
{
    printf( "\toperand0: 0x%02X\n", response[kAVCOperand0] );
    printf( "\tsubunit type: 0x%02X %s", IOAVCType(response[kAVCOperand1]),
					unitTypeStr[IOAVCType(response[kAVCOperand1])] );
    printf( " count: %d\n", IOAVCId(response[kAVCOperand1]) + 1 );
    printf( "\tsubunit type: 0x%02X %s", IOAVCType(response[kAVCOperand2]),
					unitTypeStr[IOAVCType(response[kAVCOperand2])] );
    printf( " count: %d\n", IOAVCId(response[kAVCOperand2]) + 1 );
    printf( "\tsubunit type: 0x%02X %s", IOAVCType(response[kAVCOperand3]),
					unitTypeStr[IOAVCType(response[kAVCOperand3])] );
    printf( " count: %d\n", IOAVCId(response[kAVCOperand3]) + 1 );
    printf( "\tsubunit type: 0x%02X %s", IOAVCType(response[kAVCOperand4]),
					unitTypeStr[IOAVCType(response[kAVCOperand4])] );
    printf( " count: %d\n", IOAVCId(response[kAVCOperand4]) + 1 );
}


// delay for 10 seconds printing a . onscreen every second
void Wait10Seconds( void )
{
    int i;
    
    printf( "\nPlaying:" );
    for( i=0; i<10; i++ )
    {
	printf( "." );
	fflush( stdout );
	sleep(1);
    }
    printf( "\n" );
}


CFMutableDictionaryRef CreateMatchingDictionary( void )
{
	CFMutableDictionaryRef	dict;
	UInt32 					value ;
	CFNumberRef				cfValue ;
	
	// create a matching dictionary for AVC units
	dict = IOServiceMatching( "IOFireWireAVCUnit" );

	// add properties so we only match devices that use the AVC protocol

	// add key/value to dictionary to match on Unit_Spec_ID
	value = 0x00A02D;	// ID for 1394TA -> www.1394ta.org

	// create a CFNumber using the value
	cfValue	=  CFNumberCreate( kCFAllocatorDefault, kCFNumberSInt32Type, &value );

	// add it to the matching dictionary
	CFDictionaryAddValue( dict, CFSTR("Unit_Spec_ID"), cfValue );
	
	// we're done with the CFNumber so release it
	CFRelease(cfValue) ;
	
	// add key/value to dictionary to match on Unit_SW_Vers
	value = 0x010001;	// indicates AV/C protocol

	// create a CFNumber using the value
	cfValue = CFNumberCreate( kCFAllocatorDefault, kCFNumberSInt32Type, & value );

	// add it to the matching dictionary
	CFDictionaryAddValue( dict, CFSTR("Unit_SW_Vers"), cfValue );

	// we're done with the CFNumber so release it
	CFRelease( cfValue );

	return( dict );
}