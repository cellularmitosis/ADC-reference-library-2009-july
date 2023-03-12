//////////////////////////////////////////////////////////////////////////////////
/*
	File:		VideoHardwareInfo.m

	Project:	VideoHardwareInfo

	Contains:	Implementation of the VideoHardwareInfo class

 Disclaimer: IMPORTANT:  This Apple software is supplied to you by 
 Apple Inc. ("Apple") in consideration of your agreement to the
 following terms, and your use, installation, modification or
 redistribution of this Apple software constitutes acceptance of these
 terms.  If you do not agree with these terms, please do not use,
 install, modify or redistribute this Apple software.
 
 In consideration of your agreement to abide by the following terms, and
 subject to these terms, Apple grants you a personal, non-exclusive
 license, under Apple's copyrights in this original Apple software (the
 "Apple Software"), to use, reproduce, modify and redistribute the Apple
 Software, with or without modifications, in source and/or binary forms;
 provided that if you redistribute the Apple Software in its entirety and
 without modifications, you must retain this notice and the following
 text and disclaimers in all such redistributions of the Apple Software. 
 Neither the name, trademarks, service marks or logos of Apple Inc. 
 may be used to endorse or promote products derived from the Apple
 Software without specific prior written permission from Apple.  Except
 as expressly stated in this notice, no other rights or licenses, express
 or implied, are granted by Apple herein, including but not limited to
 any patent rights that may be infringed by your derivative works or by
 other works in which the Apple Software may be incorporated.
 
 The Apple Software is provided by Apple on an "AS IS" basis.  APPLE
 MAKES NO WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION
 THE IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS
 FOR A PARTICULAR PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND
 OPERATION ALONE OR IN COMBINATION WITH YOUR PRODUCTS.
 
 IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL
 OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 INTERRUPTION) ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION,
 MODIFICATION AND/OR DISTRIBUTION OF THE APPLE SOFTWARE, HOWEVER CAUSED
 AND WHETHER UNDER THEORY OF CONTRACT, TORT (INCLUDING NEGLIGENCE),
 STRICT LIABILITY OR OTHERWISE, EVEN IF APPLE HAS BEEN ADVISED OF THE
 POSSIBILITY OF SUCH DAMAGE.
 
 Copyright (C) 2004 - 2007 Apple Inc. All Rights Reserved.
 
 */ 
//////////////////////////////////////////////////////////////////////////////////

#import "VideoHardwareInfo.h"
#include <OpenGL/OpenGL.h>
#include <IOKit/IOKitLib.h>
#include <ApplicationServices/ApplicationServices.h>

// DisplayRegisterReconfigurationCallback is a client-supplied callback function that’s invoked 
// whenever the configuration of a local display is changed.  Applications who want to register 
// for notifications of display changes would use CGDisplayRegisterReconfigurationCallback
static void DisplayRegisterReconfigurationCallback (CGDirectDisplayID display, CGDisplayChangeSummaryFlags flags, void *userInfo) 
{
		VideoHardwareInfo * vhiObject = (VideoHardwareInfo*)userInfo;
		static BOOL DisplayConfigurationChanged = NO;
		
		// Before display reconfiguration, this callback fires to inform
		// applications of a pending configuration change. The callback runs
		// once for each on-line display.  The flags passed in are set to
		// kCGDisplayBeginConfigurationFlag.  This callback does not
		// carry other per-display information, as details of how a
		// reconfiguration affects a particular device rely on device-specific
		// behaviors which may not be exposed by a device driver.
		//
		// After display reconfiguration, at the time the callback function
		// is invoked, all display state reported by CoreGraphics, QuickDraw,
		// and the Carbon Display Manager API will be up to date.  This callback
		// runs after the Carbon Display Manager notification callbacks.
		// The callback runs once for each added, removed, and currently
		// on-line display.  Note that in the case of removed displays, calls into
		// the CoreGraphics API with the removed display ID will fail.
		
		// Because the callback is called for each display I use DisplayConfigurationChanged to
		// make sure we only disable the popup to change displays once and then refresh it only once.
		if(flags == kCGDisplayBeginConfigurationFlag) {
			if(DisplayConfigurationChanged == NO) {
				[vhiObject disableUI];
				DisplayConfigurationChanged = YES;
				}
			}
		else if(DisplayConfigurationChanged == YES) {
				[vhiObject interrogateHardware];
				DisplayConfigurationChanged = NO;
			}
}

@implementation VideoHardwareInfo

- (id)init
{
	DisplayRegistrationCallBackSuccessful = NO; // Hasn't been tried yet.
	displays = nil;
	return self;
}

//Prepares the receiver for service after it has been loaded from an Interface Builder archive, or nib file.
-(void)awakeFromNib
{
	// Call our information-gathering routine on startup
    [self interrogateHardware];
	
	 // Applications who want to register for notifications of display changes would use 
	 // CGDisplayRegisterReconfigurationCallback
	 //
	 // Display changes are reported via a callback mechanism.
	 //
	 // Callbacks are invoked when the app is listening for events,
	 // on the event processing thread, or from within the display
	 // reconfiguration function when in the program that is driving the
	 // reconfiguration.
	CGError err = CGDisplayRegisterReconfigurationCallback(DisplayRegisterReconfigurationCallback,self);
	if(err == kCGErrorSuccess)
		DisplayRegistrationCallBackSuccessful = YES;
}

-(void) dealloc
{
	// CGDisplayRemoveReconfigurationCallback Removes the registration of a callback function that’s invoked 
	// whenever a local display is reconfigured.  We only remove the registration if it was successful in the first place.
	if(CGDisplayRemoveReconfigurationCallback != NULL && DisplayRegistrationCallBackSuccessful == YES)
		CGDisplayRemoveReconfigurationCallback(DisplayRegisterReconfigurationCallback, self);

	[super dealloc];
}

-(void) disableUI 
{
	[deviceSelectionButton removeAllItems];
}

// Populate the popup list of displays by iterating over all of the displays
-(void)interrogateHardware
{
	CGError				err = CGDisplayNoErr;
    int                 i;
	CGDisplayCount		dspCount = 0;

	// Remove the default items from the device selection popup
    [deviceSelectionButton removeAllItems];
	
    // How many active displays do we have?
    err = CGGetActiveDisplayList(0, NULL, &dspCount);

	// If we are getting an error here then their won't be much to display.
    if(err != CGDisplayNoErr)
        return;
	
	// Maybe this isn't the first time though this function.
	if(displays != nil)
		free(displays);

	// Allocate enough memory to hold all the display IDs we have
    displays = calloc((size_t)dspCount, sizeof(CGDirectDisplayID));

	// Get the list of active displays
    err = CGGetActiveDisplayList(dspCount,
                                 displays,
                                 &dspCount);
	
	// More error-checking here
    if(err != CGDisplayNoErr)
    {
        NSLog(@"Could not get active display list (%d)\n", err);
        return;
    }

    // Now we iterate through them
    for(i = 0; i < dspCount; i++)
    {		
		[deviceSelectionButton addItemWithTitle:[[NSNumber numberWithUnsignedInt:CGDisplayUnitNumber (displays[i])] stringValue]];
   }

	// Select whatever device is first in the list
	[self selectDevice:nil];

	return;
}

-(void)interrogateIOKitFor: (CGDirectDisplayID) displayID
{

    CFTypeRef           typeCode;
    CFDataRef vendorID, deviceID, model;
	long vramSize;
	io_registry_entry_t dspPort;
		
	// Get the I/O Kit service port for the display
	dspPort = CGDisplayIOServicePort(displayID);

	// Get the information for the device we've selected from the list
	// The vendor ID, device ID, and model are all available as properties of the hardware's I/O Kit service port
	vendorID = IORegistryEntrySearchCFProperty(dspPort,kIOServicePlane,CFSTR("vendor-id"),kCFAllocatorDefault,kIORegistryIterateRecursively | kIORegistryIterateParents);
	deviceID = IORegistryEntrySearchCFProperty(dspPort,kIOServicePlane,CFSTR("device-id"),kCFAllocatorDefault,kIORegistryIterateRecursively | kIORegistryIterateParents);
	model = IORegistryEntrySearchCFProperty(dspPort,kIOServicePlane,CFSTR("model"),
		kCFAllocatorDefault,kIORegistryIterateRecursively | kIORegistryIterateParents);
	
	// Send the appropriate data to the outputs checking to validate the data
    if(vendorID)
		[vendorIDOut setStringValue:[NSString stringWithFormat:@"0x%08X",*((UInt32*)CFDataGetBytePtr(vendorID))]];
		
    if(deviceID)
		[deviceIDOut setStringValue:[NSString stringWithFormat:@"0x%08X",*((UInt32*)CFDataGetBytePtr(deviceID))]];

	if(model)
		[modelOut setStringValue:[[NSString alloc] initWithBytes:CFDataGetBytePtr(model) length:CFDataGetLength(model) encoding:NSUTF8StringEncoding]];
	
	// Release vendorID, deviceID, and model as appropriate
	if(vendorID)
		CFRelease(vendorID);
	if(deviceID)
		CFRelease(deviceID);
	if(model)
		CFRelease(model);

	// Ask IOKit for the property for the display VRAM size
	typeCode = IORegistryEntrySearchCFProperty(dspPort,kIOServicePlane,CFSTR(kIOFBMemorySizeKey),
		kCFAllocatorDefault,kIORegistryIterateRecursively | kIORegistryIterateParents);

	// Ensure we have valid data from IOKit
	if(typeCode && CFGetTypeID(typeCode) == CFNumberGetTypeID())
	{
		// If so, convert the CFNumber into a plain unsigned long
		CFNumberGetValue(typeCode, kCFNumberSInt32Type, &vramSize);
		if(typeCode)
			CFRelease(typeCode);
	}

	// Output the information for the device's vram - note the formatting on the VRAM information
	[vramOut setStringValue:[NSString stringWithFormat:@"%dMB / %dKB", (vramSize / (1024 * 1024)), (vramSize / 1024)]];
	
}

// Query OpenGL for the Render, Vendor, Version, GLSL, GLSL version and 
// the list of extensions.
// Set these in the appropriate NSTextField's in the interface
-(void)interrogateOpenGLFor: (CGDirectDisplayID) displayID
{
	// To read the capabilities of a display first create an OpenGL context
	// on that display, set it as the current context. Using the current context
	// we can then read the attributes and capabilites for that display.
	
	// A CGOpenGLDisplayMask can be used to create a GL context
	// on a specific display, so create one for the display we
	// would like to querry.
	CGOpenGLDisplayMask myDisplayMask = 
					CGDisplayIDToOpenGLDisplayMask (displayID);
	
	// Create a pixel format using the display mask
	CGLPixelFormatAttribute attribs[] = {kCGLPFADisplayMask,
							 myDisplayMask, 
							   nil};

	CGLPixelFormatObj pixelFormat = NULL;
	long numPixelFormats = 0;
	CGLContextObj myCGLContext = 0;
	// Save the current context so we can restore it after we are done
	CGLContextObj curr_ctx = CGLGetCurrentContext ();
	// Create a pixel format based on the Display Mask of the display we are looking at
	if(CGLChoosePixelFormat (attribs, &pixelFormat, &numPixelFormats) == kCGLNoError) {
		if (pixelFormat) {
			// Create a GL context based on the pixel format
			if(CGLCreateContext (pixelFormat, NULL, &myCGLContext) == kCGLNoError) {
				CGLDestroyPixelFormat (pixelFormat);
				// Set the current context to the new context on the target display
				if(CGLSetCurrentContext (myCGLContext) == kCGLNoError) {
					if (myCGLContext) {
						const GLubyte *glExtensions = glGetString(GL_EXTENSIONS);
						// Check for capabilities and functionality here
						NSMutableString *glExtensionsNSString = nil;
						
						// The OpenGL Major version is used when determining the GLSL version.
						int glMajorVersion;
						
						// Grab the strings we need from the current context.
						const GLubyte *glversionString = glGetString(GL_VERSION);
						
						[glRendererOut setStringValue:[NSString stringWithCString:(const char *)glGetString(GL_RENDERER) encoding:NSASCIIStringEncoding]];

						[glVendorOut setStringValue:[NSString stringWithCString:(const char *)glGetString(GL_VENDOR)
							encoding:NSASCIIStringEncoding]];
							
						[glVersionOut setStringValue:[NSString stringWithCString:(const char *)glversionString encoding:NSASCIIStringEncoding]];

						// Since we want to do some formatting with the extensions, we'll use an NSMutableString
						glExtensionsNSString = [NSMutableString stringWithCString:(const char *)glGetString(GL_EXTENSIONS) encoding:NSASCIIStringEncoding];
						// Replace the spaces in the extension string list with newlines
						[glExtensionsNSString replaceOccurrencesOfString:@" "
										withString:@"\n" 
										options:0
										range:NSMakeRange(0, [glExtensionsNSString length])];
						

						// Send this formatted string to the NSTextView
						[extTextView setString:glExtensionsNSString];

						// GLSL
						// OpenGL version 1.x and 2.xx and later support different methods for getting verifying GLSL support and getting
						// the GLSL version, so first parse the GLVersion string using an NSScanner to get the OpenGL Major version
						// (The first int value).
						NSScanner *glVersionScanner = [NSScanner scannerWithString:[NSString stringWithCString:(const char *)glversionString encoding:NSASCIIStringEncoding]];

						// GL v1.xx only provides GLSL 1.00 an an extension
						// GL 2.xx can get the GLSL version as a string using glGetString(GL_SHADING_LANGUAGE_VERSION)					
						if([glVersionScanner scanInt:&glMajorVersion]) {
							if(glMajorVersion == 1) {  // OpenGL version 1.xxx
								if(gluCheckExtension((const GLubyte *)"GL_ARB_shading_language_100",glExtensions)) {
										[glslVersion setStringValue:@"1.00"];
										// We do have GLSL support so we can display that now.
										[glslSupported setStringValue:@"YES"];
									}
							} else if (glMajorVersion >= 2) { // OpenGL version >= 2.xx
								const GLubyte * glslVersionString = glGetString(GL_SHADING_LANGUAGE_VERSION);
								if(glslVersionString != NULL) {
									[glslVersion setStringValue:[NSString stringWithCString:(const char *)glslVersionString encoding:NSASCIIStringEncoding]];
									// We do have GLSL support so we can display that now.
									[glslSupported setStringValue:@"YES"];
								}
							}
						}
					}
				}
				// Restore the current context to the one before we entered this method.
				CGLSetCurrentContext (curr_ctx);
			}

			// Clean up after our selves
			CGLDestroyContext (myCGLContext);
		}
	}
}

// Query Quartz to see if the display is using OpenGL
// acceleration.  If so, then the display is Quartz
// Extreme capable.
-(void)interrogateQuartzFor: (CGDirectDisplayID) displayID
{
	// Is the display hardware accelerated?
	if(CGDisplayUsesOpenGLAcceleration(displayID))
		[glAccelerated setStringValue:@"YES"];
}


//In case of a value isn't avalible make sure the main window will reflect what we know
//by resetting all values that may not be updated.
-(void) resetWindowValues
{
	// vendorIDOut, deviceIDOut, modelOut and vramOut are obtained using IOKit
	[vendorIDOut setStringValue:@"Unknown"];
	[deviceIDOut setStringValue:@"Unknown"];
	[modelOut setStringValue:@"Unknown"];
	[vramOut setStringValue:[NSString stringWithFormat:@"%dMB / %dKB", 0, 0]];
	
	// glslSupported and glslVersion are obtained using OpenGL
	[glslSupported setStringValue:@"NO"];
	[glslVersion setStringValue:@"Unknown"];

	// glAccelerated is obtained using Quartz
	[glAccelerated setStringValue:@"NO"];	
}

// When a device is selected, update the UI with the new vender,device, model then update the OpenGL info.
// This IBAction is wired up to the device selection popup.
-(IBAction)selectDevice:(id)sender
{
	unsigned long currentService = 0;
	      
	// Find out which display is selected
    currentService = [sender indexOfSelectedItem];

	// Make sure the display doesn't preserve values from
	// a previously selected display.
	[self resetWindowValues];

	//Update the IOKit Information for the display
	[self interrogateIOKitFor:displays[currentService]];

	//Update the OpenGL Information for the display
	[self interrogateOpenGLFor:displays[currentService]];

	//Update the Quartz Information for the display
	[self interrogateQuartzFor:displays[currentService]];
}

@end