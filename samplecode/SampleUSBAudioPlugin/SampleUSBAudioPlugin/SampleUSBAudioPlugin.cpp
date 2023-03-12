/*
*	File:		SampleUSBAudioPlugin.cpp
*
*	Contains:	com_MySoftwareCompany_driver_SampleUSBAudioPlugin Implementation
*
*			A simple USB audio plug-in that performs a lowpass filtering operation on audio streamed
*			through the Griffin iMic. The target SampleUSBAudioPlugin.kext must be copied to the 
*			/System/Library/Extensions folder of the boot volume by an administrator using a 
*			command like (from the project folder):
*
*			sudo cp -rf ./build/SampleUSBAudioPlugin.kext /System/Library/Extensions
*
*           Then you must notify the operating system that the contents of the kernel extension directory has 
*           changed by "touching the directory":
*
*			sudo touch /System/Library/Extensions
*
*			Restart the system.
* 
*			After restarting the system, the plug-in should automatically load when the device
*			specified in the Info.plist is plugged in. Plug-in logging can be observed by viewing 
*			the system log using Console in /Applications/Utilities.
*			
*	
*	Version:	1.0
*
*	Created:	11-23-2004
*
*   © Copyright 2004 Apple Computer, Inc. All rights reserved.
*
*	Disclaimer:	IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc. ("Apple") in
*				consideration of your agreement to the following terms, and your use, installation, modification
*				or redistribution of this Apple software constitutes acceptance of these terms.  If you do
*				not agree with these terms, please do not use, install, modify or redistribute this Apple
*				software.
*
*				In consideration of your agreement to abide by the following terms, and subject to these terms,
*				Apple grants you a personal, non-exclusive license, under Apple's copyrights in this
*				original Apple software (the "Apple Software"), to use, reproduce, modify and redistribute the
*				Apple Software, with or without modifications, in source and/or binary forms; provided that if you
*				redistribute the Apple Software in its entirety and without modifications, you must retain this
*				notice and the following text and disclaimers in all such redistributions of the Apple Software.
*				Neither the name, trademarks, service marks or logos of Apple Computer, Inc. may be used to
*				endorse or promote products derived from the Apple Software without specific prior written
*				permission from Apple.  Except as expressly stated in this notice, no other rights or
*				licenses, express or implied, are granted by Apple herein, including but not limited to any
*				patent rights that may be infringed by your derivative works or by other works in which the
*				Apple Software may be incorporated.
*
*				The Apple Software is provided by Apple on an "AS IS" basis.  APPLE MAKES NO WARRANTIES, EXPRESS OR
*				IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY
*				AND FITNESS FOR A PARTICULAR PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND OPERATION ALONE
*				OR IN COMBINATION WITH YOUR PRODUCTS.
*
*				IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL OR CONSEQUENTIAL
*				DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
*				OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) ARISING IN ANY WAY OUT OF THE USE,
*				REPRODUCTION, MODIFICATION AND/OR DISTRIBUTION OF THE APPLE SOFTWARE, HOWEVER CAUSED AND WHETHER
*				UNDER THEORY OF CONTRACT, TORT (INCLUDING NEGLIGENCE), STRICT LIABILITY OR OTHERWISE, EVEN
*				IF APPLE HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*
*/

#include <IOKit/IOLib.h>

#include "SampleUSBAudioPlugin.h"

#define super AppleUSBAudioPlugin

OSDefineMetaClassAndStructors (com_MySoftwareCompany_driver_SampleUSBAudioPlugin, AppleUSBAudioPlugin);

// AppleUSBAudioPlugin methods
/*****************************************************
*
* com_MySoftwareCompany_driver_SampleUSBAudioPlugin::pluginInit (IOService * provider, UInt16 vendorID, UInt16 productID)
*
* Purpose:  
*
* Inputs:   provider   -  AppleUSBAudioEngine object associated with device
*           vendorID   -  idVendor
*           productID  -  idProduct
*
* Returns:  IOReturn  result -  (0 == no error)
*/
IOReturn com_MySoftwareCompany_driver_SampleUSBAudioPlugin::pluginInit (IOService * provider, UInt16 vendorID, UInt16 productID) 
{
	IOReturn				result;
	//IOUSBDevRequest		myRequest;

	// Init our superclass first
	result = super::pluginInit (provider, vendorID, productID);

#warning   Change the idVendor & idProduct here (and in Info.plist) to match the audio device you want to use! 
	// Double check that we're loaded on the device that we want to be; 
	if (vendorID != 0x5FC || productID != 0x7849) 
	{	
		//NOTE: Avoid logging this type of information (idVendor, idProduct) in a released product
		//The plug-in should fail silently, if the incorrect (idVendor, idProduct) is given.
		//Only log this type of information during development!
		//IOLog ("vendorID = %u, productID = %u", vendorID, productID);
	
		return kIOReturnNoDevice;
	}
	
	/* pluginDeviceRequest is not used in this example, but it could be implemented here if there were a need to communicate
	   directly to the USB audio hardware. More information on device requests can be found at http://developer.apple.com/devicedrivers/ .
		
		As defined in IOKit/usb/USB.h
		typedef struct {
			UInt8       bmRequestType;
			UInt8       bRequest;
			UInt16      wValue;
			UInt16      wIndex;
			UInt16      wLength;
			void *      pData;
			UInt32      wLenDone;
		} IOUSBDevRequest;
	*/
	
	// Prepare to make the device request
	
	/*
	 theData = IOMalloc (4);
	 if (!theData) 
	 {
		 return kIOReturnNoMemory;
	 }
	 
	 myRequest.bmRequestType = ;
	 myRequest.bRequest = ;
	 myRequest.wValue = ;
	 myRequest.wIndex = ;
	 myRequest.wLength = ;
	 myRequest.pData = theData;
	 
	 result = pluginDeviceRequest (&myRequest);
	 */
	
	if (result == kIOReturnSuccess) 
	{
		// Do our one time initialization
		IOLog ("com_MySoftwareCompany_driver_SampleUSBAudioPlugin initialized successfully\n");

		/* Just initialize to some random values to verify that pluginSetFormat is working.
		   These values should be overwritten by the device's actual format when pluginSetFormat is called. */
		mSampleRate = 44101;
		mNumChannels = 3;
		
		setProcessingParameters (&mParamStruct, mSampleRate, mNumChannels);
	}

	return result;
}

/*****************************************************
*
* com_MySoftwareCompany_driver_SampleUSBAudioPlugin::pluginStart ()
*
* Purpose:  Start processing audio
*
* Inputs:   (void)
*
* Returns:  IOReturn  result -  (0 == no error)
*/
IOReturn com_MySoftwareCompany_driver_SampleUSBAudioPlugin::pluginStart () 
{
	IOReturn				result;

	// Start our superclass first
	result = super::pluginStart ();
	
	if (result == kIOReturnSuccess) 
	{
		// Do our every time initialization
		resetProcessingState (&mStateStruct);
		IOLog ("com_MySoftwareCompany_driver_SampleUSBAudioPlugin starting.\n");
	}

	return result;
}

/*****************************************************
*
* com_MySoftwareCompany_driver_SampleUSBAudioPlugin::pluginReset ()
*
* Purpose:  Reset processing parameters and state.
*
* Inputs:   (void)
*
* Returns:  IOReturn  result -  (0 == no error)
*/
IOReturn com_MySoftwareCompany_driver_SampleUSBAudioPlugin::pluginReset () 
{
	IOReturn				result;

	IOLog ("com_MySoftwareCompany_driver_SampleUSBAudioPlugin resetting...\n");

	// Reset the plugin's state
	setProcessingParameters (&mParamStruct, mSampleRate, mNumChannels);
	resetProcessingState (&mStateStruct);

	// No need to call super::pluginReset
	result = kIOReturnSuccess;

	return result;
}

/*****************************************************
*
* com_MySoftwareCompany_driver_SampleUSBAudioPlugin::pluginSetFormat(const IOAudioStreamFormat * const newFormat, const IOAudioSampleRate * const newSampleRate)
*
* Purpose:  Changes the current audio stream format
*
* Inputs:   newFormat - new audio stream format
*			newSampleRate- new sample rate
*
* Returns:  IOReturn  result -  (0 == no error)
*/
IOReturn com_MySoftwareCompany_driver_SampleUSBAudioPlugin::pluginSetFormat (const IOAudioStreamFormat * const newFormat, const IOAudioSampleRate * const newSampleRate) 
{
	IOReturn				result;

	IOLog ("com_MySoftwareCompany_driver_SampleUSBAudioPlugin changing format\n");
	
	// If parameters depend on sample rate or number of channels (or other format info), update them here
	if (NULL != newSampleRate) 
	{
		mSampleRate = newSampleRate->whole;
	}
	
	if (NULL != newFormat) 
	{
		mNumChannels = newFormat->fNumChannels;
	}

	setProcessingParameters (&mParamStruct, mSampleRate, mNumChannels);

	IOLog ("mSampleRate = %ld, mNumChannels = %ld\n", mSampleRate, mNumChannels);
	
	// No need to call super::pluginProcess
	result = kIOReturnSuccess;

	return result;
	
} 

/*****************************************************
*
* com_MySoftwareCompany_driver_SampleUSBAudioPlugin::pluginProcess (float * mixBuf, UInt32 numSampleFrames, UInt32 numChannels) 
*
* Purpose:  Calls processSamples() (low pass filter) to process audio data
*
* Inputs:   mixBuf - pointer to the mixed buffer samples
*			numSampleFrames - number of sample frames in mixed buffer
*			numChannels - number of channels that are interleaved in the mixed buffer
*
* Returns:  IOReturn  result -  (kIOReturnSuccess == no error)
*/
IOReturn com_MySoftwareCompany_driver_SampleUSBAudioPlugin::pluginProcess (float * mixBuf, UInt32 numSampleFrames, UInt32 numChannels) 
{
	IOReturn				result;

	// Do our data processing methods
	
	if (    (mSampleRate == 44100)
		 && (mNumChannels <= 2))
	{
		processSamples (mixBuf, numSampleFrames, numChannels, &mParamStruct, &mStateStruct);
	}
	
	// No need to call super::pluginProcess
	result = kIOReturnSuccess;

	return result;
}

/*****************************************************
*
* com_MySoftwareCompany_driver_SampleUSBAudioPlugin::pluginStop ()
*
* Purpose:  Start processing audio
*
* Inputs:   (void)
*
* Returns:  IOReturn  result -  (0 == no error)
*/
IOReturn com_MySoftwareCompany_driver_SampleUSBAudioPlugin::pluginStop () 
{
	IOReturn				result;

	// Do our stop work first, then call super
	IOLog ("com_MySoftwareCompany_driver_SampleUSBAudioPlugin stopping.\n");

	pluginReset ();
	result = super::pluginStop ();

	return result;
}
