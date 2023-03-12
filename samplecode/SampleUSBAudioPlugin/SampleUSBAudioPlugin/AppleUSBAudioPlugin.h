/*
*	File:		 ProcessFloatSamples.cpp
*
*	Contains:	 functions for managing filter operations and related data structures.
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

#ifndef __APPLEUSBAUDIOPLUGIN__
#define __APPLEUSBAUDIOPLUGIN__

#include <IOKit/IOService.h>
#include <IOKit/audio/IOAudioStream.h>
#include <IOKit/usb/USB.h>

class AppleUSBAudioPlugin : public IOService {

    OSDeclareDefaultStructors (AppleUSBAudioPlugin)

private:
	AppleUSBAudioPlugin *			mOurProvider;

protected:
    struct ExpansionData { };

    ExpansionData *reserved;

private:
    OSMetaClassDeclareReservedUnused (AppleUSBAudioPlugin, 0);
    OSMetaClassDeclareReservedUnused (AppleUSBAudioPlugin, 1);
    OSMetaClassDeclareReservedUnused (AppleUSBAudioPlugin, 2);
    OSMetaClassDeclareReservedUnused (AppleUSBAudioPlugin, 3);
    OSMetaClassDeclareReservedUnused (AppleUSBAudioPlugin, 4);
    OSMetaClassDeclareReservedUnused (AppleUSBAudioPlugin, 5);
    OSMetaClassDeclareReservedUnused (AppleUSBAudioPlugin, 6);
    OSMetaClassDeclareReservedUnused (AppleUSBAudioPlugin, 7);
    OSMetaClassDeclareReservedUnused (AppleUSBAudioPlugin, 8);
    OSMetaClassDeclareReservedUnused (AppleUSBAudioPlugin, 9);
    OSMetaClassDeclareReservedUnused (AppleUSBAudioPlugin, 10);
    OSMetaClassDeclareReservedUnused (AppleUSBAudioPlugin, 11);
    OSMetaClassDeclareReservedUnused (AppleUSBAudioPlugin, 12);
    OSMetaClassDeclareReservedUnused (AppleUSBAudioPlugin, 13);
    OSMetaClassDeclareReservedUnused (AppleUSBAudioPlugin, 14);
    OSMetaClassDeclareReservedUnused (AppleUSBAudioPlugin, 15);

public:
	virtual	bool		start (IOService * provider);
	virtual	void		stop (IOService * provider);

	IOReturn			pluginDeviceRequest (IOUSBDevRequest * request, IOUSBCompletion * completion = NULL);

	virtual	IOReturn	pluginInit (IOService * provider, UInt16 vendorID, UInt16 productID);
	virtual	IOReturn	pluginStart ();
	virtual	IOReturn	pluginReset ();
	virtual	IOReturn	pluginSetFormat (const IOAudioStreamFormat * const newFormat, const IOAudioSampleRate * const newSampleRate);
	virtual	IOReturn	pluginProcess (float * mixBuf, UInt32 numSampleFrames, UInt32 numChannels);
	virtual	IOReturn	pluginStop ();
};

#endif