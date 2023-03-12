/*
	Copyright: 	© Copyright 2006 Apple Computer, Inc. All rights reserved.

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
*/
//
//  File: MouseInterface.m
//

#import "MouseInterface.h"
#import <IOKit/hid/IOHIDKeys.h>
#import <IOKit/hidsystem/IOHIDShared.h>
#define MODIFY_GENERAL_ACCL	1	/* 1 - modify mouse settings for both trackpad and mouse devices */
								/* 0 - modify mouse settings for trackpad OR mouse devices but not both 
										as per the FOR_TRACKPAD detting next */

#if !MODIFY_GENERAL_ACCL
#define FOR_TRACKPAD	0	/* 0 - modify mouse settings for HID mouse */
							/* 1 - modify trackpad settings for portable Mac system */
#endif !MODIFY_GENERAL_ACCL

@implementation MouseInterface

-(id) init {
	self = [super init];
	if (self) {
	
		if ([self findHIDSystem] != 0)
			fprintf(stderr, "failed to find the HID system");
	}
	return self;
}

//------------------------------------------------------------
// mouseDefaultSpeed - return the default mouse speed which can be
// what the original setting was, or the setting that the user
// has selected as the default
//------------------------------------------------------------

- (double)mouseDefaultSpeed
{
	return mouseSpeedSave;
}

//------------------------------------------------------------
// mouseSpeed - use the IOHIDGetMouseAcceleration to obtain the 
// current cursor speed setting.
//------------------------------------------------------------
- (kern_return_t)mouseSpeed:(double*)value
{
    kern_return_t		kernResult; 	
#if !MODIFY_GENERAL_ACCL
#if FOR_TRACKPAD
	CFStringRef			acclCFString = CFSTR(kIOHIDTrackpadAccelerationType);
#else
	CFStringRef			acclCFString = CFSTR(kIOHIDMouseAccelerationType);
#endif	FOR_TRACKPAD
#endif	//!MODIFY_GENERAL_ACCL

	if (mouseDev != nil);
	{
#if MODIFY_GENERAL_ACCL
		kernResult = IOHIDGetMouseAcceleration(mouseDev, &currMouseSpeed );
#else
		kernResult = IOHIDGetAccelerationWithKey( mouseDev, acclCFString, &currMouseSpeed );
#endif
		if (kernResult == KERN_SUCCESS)
			*value = currMouseSpeed;
		else
			*value = 0.0;
	}
	return kernResult;	// if there was a problem, then return no acceleration
}

//------------------------------------------------------------
// setMouseSpeed - use the IOHIDSetMouseAcceleration to set the 
// cursor speed to the desired input value.
//------------------------------------------------------------
- (void)setMouseSpeed:(double)value 
{
	kern_return_t		kernResult; 
#if !MODIFY_GENERAL_ACCL
#if FOR_TRACKPAD
	CFStringRef			acclCFString = CFSTR(kIOHIDTrackpadAccelerationType);
#else
	CFStringRef			acclCFString = CFSTR(kIOHIDMouseAccelerationType);
#endif	FOR_TRACKPAD
#endif	//!MODIFY_GENERAL_ACCL
	
    if (currMouseSpeed != value) 
	{
#if MODIFY_GENERAL_ACCL
		kernResult = IOHIDSetMouseAcceleration(mouseDev, value );
#else
		kernResult = IOHIDSetAccelerationWithKey( mouseDev, acclCFString, value );
#endif	//MODIFY_GENERAL_ACCL
		if (kernResult == KERN_SUCCESS)
			currMouseSpeed = value;
		else
			fprintf(stderr, "IOHIDSetMouseAcceleration returned %d\n", kernResult);
    }
}

/*------------------------------------------------------------
// findHIDSystem -  finds the IOHIDSystem object in the IORegistry and opens
// the object for use by the sample program. If a problem occurs, an error is
// returned
//------------------------------------------------------------
*/
- (kern_return_t) findHIDSystem
{
	io_iterator_t			matchingServices;
    io_object_t				intfService;
    kern_return_t			kernResult; 
    mach_port_t				masterPort;
    CFMutableDictionaryRef	classesToMatch;
	CFMutableDictionaryRef  dict;
	Boolean					done = FALSE;

/*! @function IOMasterPort
    @abstract Returns the mach port used to initiate communication with IOKit.
    @discussion Functions that don't specify an existing object require the IOKit master port to be passed. This function obtains that port.
    @param bootstrapPort Pass MACH_PORT_NULL for the default.
    @param masterPort The master port is returned.
    @result A kern_return_t error code. */

    kernResult = IOMasterPort(MACH_PORT_NULL, &masterPort);
    if (KERN_SUCCESS != kernResult)
        fprintf(stderr, "IOMasterPort returned %d\n", kernResult);
        
	// define the matching class to look for as IOHIDSystem.
	classesToMatch = IOServiceMatching("IOHIDSystem");

    if (classesToMatch == NULL)
	{
        fprintf(stderr, "IOServiceMatching returned a NULL dictionary.\n");
		return KERN_FAILURE;
	}
	// find all IOHIDSystem class devices, which will include pointing and 
	// keyboard devices.
    kernResult = IOServiceGetMatchingServices(masterPort, classesToMatch, &matchingServices);    
    if (kernResult != KERN_SUCCESS)
		// if no such matching devices was found, the print the error and exit
        fprintf(stderr, "IOServiceGetMatchingServices returned %d\n", kernResult);
	if (intfService = IOIteratorNext(matchingServices))
	{
		/* open a connection to the HIDSystem User client so that we can make the 
		IOHIDGetAcceleration/IOHIDGetAccelerationWithKey call. The HID system is a gate for all 
		supported HID devices, keyboards and mice. You can use the IOHIDLib.h function to manipulate 
		the general settings which apply to all devices. You cannot use the IOHIDLib.h calls to manipulate the
		settings for a specific device.
		*/
		
		kernResult = IOServiceOpen( intfService, mach_task_self(), kIOHIDParamConnectType, &mouseDev);
		// have accessed the user client so make the call to get the current acceleration setting
		if (kernResult == 0)
		{
			kernResult = [self mouseSpeed:&mouseSpeedSave];
			if (kernResult == KERN_SUCCESS)
				// we have found a mouse, so no need to iterate further.
				done = TRUE;	// stop the while loop
			else
			{
				fprintf(stderr, "mouseSpeed returned error 0x%X\n", kernResult);
				// we have a problem so cleanup
				(void) IOServiceClose(mouseDev);
				(void) IOObjectRelease(mouseDev);				
			}
		}
		else
		{
			fprintf(stderr, "IOServiceOpen returned error 0x%X\n", kernResult);
			(void) IOObjectRelease(mouseDev);
		}

	}
	
	if (!done)
	{
		fprintf(stderr, "No cursor device found\n");
		kernResult = KERN_FAILURE;
	}
    return kernResult;
}
    
@end
