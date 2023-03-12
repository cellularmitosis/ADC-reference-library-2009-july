//////////////////////////////////////////////////////////////////////////////////
/*
	File:		VideoHardwareInfo.m

	Project:	VideoHardwareInfo

	Contains:	Implementation of the VideoHardwareInfo class (Cocoa)
	
	Author: 	Todd Previte
	
	Copyright:	(c) 2004 Apple Computer, Inc., All Rights Reserved
	
	Disclaimer:	

	IMPORTANT: This Apple software is supplied to you by Apple Computer, Inc.
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
//////////////////////////////////////////////////////////////////////////////////

#import "VideoHardwareInfo.h"


@implementation VideoHardwareInfo

-(void)awakeFromNib
{
	// Call our information-gathering routine on startup
    [self interrogateHardware];
	// Select whatever device is first in the list
	[self selectDevice:nil];
}

-(void)interrogateHardware
{
	// Local variables for kernel stuff
    kern_return_t err;
    mach_port_t masterPort;
    io_iterator_t itThis;
    io_service_t service;

	// Use CG to get the VRAM info for the display
	[self vramSize];
	
	// Get a mach port for us and check for errors
    err = IOMasterPort(MACH_PORT_NULL, &masterPort);
    if(err)
    {
		NSLog(@"Error: Could not get master port!");
		return;
    }
    // Grab all the PCI devices out of the registry
    err = IOServiceGetMatchingServices(masterPort, IOServiceMatching("IOPCIDevice"), &itThis);
    if(err)
    {
		NSLog(@"Error: No matching services!");
		return;
    } 

    // Clear the popup button
    [deviceSelectionButton removeAllItems];
    [glDeviceSelectionButton removeAllItems];
    
    // Yank everything out of the iterator
    while(1)
    {
		service = IOIteratorNext(itThis);
		io_name_t dName;
		
		// Make sure we have a valid service
		if(service)
		{
			// Get the classcode so we know what we're looking at
			CFDataRef classCode =  IORegistryEntryCreateCFProperty(service,CFSTR("class-code"),kCFAllocatorDefault,0);
			// Only accept devices that are 
			// PCI Spec - 0x00030000 is a display device
			if((*(UInt32*)CFDataGetBytePtr(classCode) & 0x00ff0000) == 0x00030000)
			{
				// If all that crap in the if() is true, then this is a display controller
				serviceList[serviceCount++] = service;
				// Get the name of the service (hw)
				IORegistryEntryGetName(service, dName);
				// Add this service to the list of available services
				[deviceSelectionButton addItemWithTitle:[NSString stringWithCString:dName]];			
				[glDeviceSelectionButton addItemWithTitle:[NSString stringWithCString:dName]];			
			}
		}
		else
			break;
	}
	
	// Now we need to get some information out of OpenGL
	[self interrogateOpenGL];
}

-(IBAction)selectDevice:(id)sender
{
    CFDataRef vendorID, deviceID, model;
	unsigned long currentService = 0;
	
    // Find out which one we selected
    currentService = [sender indexOfSelectedItem];
	// Validate the selection
    if(currentService < 0 || currentService > serviceCount)
    {
		NSLog(@"Bad service count from button!");
		return;
    }
    
	// Get the information for the device we've selected from the list
    vendorID = IORegistryEntryCreateCFProperty(serviceList[currentService], CFSTR("vendor-id"),kCFAllocatorDefault,0);
    deviceID = IORegistryEntryCreateCFProperty(serviceList[currentService], CFSTR("device-id"),kCFAllocatorDefault,0);
    model = IORegistryEntryCreateCFProperty(serviceList[currentService], CFSTR("model"),kCFAllocatorDefault,0);

	// Send the appropriate data to the outputs - note the formatting on the VRAM information
    [vendorIDOut setStringValue:[NSString stringWithFormat:@"0x%08X",*((UInt32*)CFDataGetBytePtr(vendorID))]];
    [deviceIDOut setStringValue:[NSString stringWithFormat:@"0x%08X",*((UInt32*)CFDataGetBytePtr(deviceID))]];
    [modelOut setStringValue:[NSString stringWithCString:CFDataGetBytePtr(model)]];
	[vramOut setStringValue:[NSString stringWithFormat:@"%dMB / %dKB / %d bytes", 
							(vramSize / (1024 * 1024)), (vramSize / 1024), vramSize]];
}

-(void)vramSize
{
	int					i = 0;
	short				MAXDISPLAYS = 8;
	io_service_t		dspPorts[MAXDISPLAYS];
	CGDirectDisplayID   displays[MAXDISPLAYS];
	CFTypeRef			typeCode;
	
	// First we're going to grab the online displays
	CGGetOnlineDisplayList(MAXDISPLAYS, displays, &displayCount);
	
	// Now we iterate through them
	for(i = 0; i < displayCount; i++)
		dspPorts[i] = CGDisplayIOServicePort(displays[i]);

	// Ask for the physical size of VRAM of the primary display
	typeCode = IORegistryEntryCreateCFProperty(dspPorts[0], CFSTR("IOFBMemorySize"), kCFAllocatorDefault, kNilOptions);
	
	// Validate our data and make sure we're getting the right type
	if(typeCode && CFGetTypeID(typeCode) == CFNumberGetTypeID())
	{
		long vramStorage = 0;
		// Convert this to a useable number
		CFNumberGetValue(typeCode, kCFNumberSInt32Type, &vramStorage);
		// If we get something other than 0, we'll use it
		if(vramStorage > 0)
			vramSize = vramStorage;
	}
}

-(void)interrogateOpenGL
{
	// First thing we need to do is setup a basic GL context
	[glv initWithFrame:[glv frame] pixelFormat:[NSOpenGLView defaultPixelFormat]];
	// If we've got a valid context we can continue
	if([glv openGLContext])
	{
		NSMutableString *glExtensions = nil;
		// Grab the strings we need
		[glRendererOut setStringValue:[NSString stringWithCString:glGetString(GL_RENDERER)]];
		[glVendorOut setStringValue:[NSString stringWithCString:glGetString(GL_VENDOR)]];
		[glVersionOut setStringValue:[NSString stringWithCString:glGetString(GL_VERSION)]];

		// Since we want to do some formatting with the extensions, we'll use an NSMutableString
		glExtensions = [NSMutableString stringWithCString:glGetString(GL_EXTENSIONS)];
		// Replace the spaces in the extension string list with newlines
		[glExtensions replaceOccurrencesOfString:@" "
						withString:@"\n" 
						options:0
						range:NSMakeRange(0, [glExtensions length])];
		// Send this formatted string to the NSTextView
		[extTextView setString:glExtensions];
	}
}

@end
