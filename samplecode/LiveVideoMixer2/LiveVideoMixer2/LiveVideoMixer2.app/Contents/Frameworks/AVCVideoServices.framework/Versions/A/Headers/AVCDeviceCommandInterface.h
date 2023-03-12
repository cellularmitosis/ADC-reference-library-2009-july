/*
	File:		AVCDeviceCommandInterface.h
 
 Synopsis: This is the header file for the AVCDeviceCommandInterface class.
 
	Copyright: 	© Copyright 2001-2003 Apple Computer, Inc. All rights reserved.
 
	Written by: ayanowitz
 
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

#ifndef __AVCVIDEOSERVICES_AVCDEVICECOMMANDINTERFACE__
#define __AVCVIDEOSERVICES_AVCDEVICECOMMANDINTERFACE__

namespace AVS
{

// Callback for client notification of changes to this avc device
typedef IOReturn (*AVCDeviceCommandInterfaceMessageNotification) (class AVCDeviceCommandInterface *pAVCDeviceCommandInterface,
												  natural_t messageType,
												  void * messageArgument,
												  void *pRefCon);
	
class AVCDeviceCommandInterface
{
public:
	// Constructor
	AVCDeviceCommandInterface(AVCDevice *pAVCDevice);
	
	// Destructor
	~AVCDeviceCommandInterface();
	
	// Function to activate this AVC device's command interface
    IOReturn activateAVCDeviceCommandInterface(AVCDeviceCommandInterfaceMessageNotification deviceMessageProc, void *pMessageProcRefCon);

	// Function to deactivate this AVC device's command interface
    IOReturn deactivateAVCDeviceCommandInterface(void);
	
	// Send an AVC command to this AVC Device
	IOReturn AVCCommand(const UInt8 *command, UInt32 cmdLen, UInt8 *response, UInt32 *responseLen);
	
	// Semi-Private: Only used by internal registered callbacks
	AVCDeviceCommandInterfaceMessageNotification clientDeviceMessageProc;
	void *pClientMessageProcRefCon;

private:
	io_service_t avcUnit;
	io_object_t	interestNotification ;
	AVCDeviceController *pAVCDeviceController;
	IOFireWireAVCLibUnitInterface **avcInterface;
	IOReturn createAVCUnitInterface(void);
	IOReturn releaseAVCUnitInterface(void);
};

} // namespace AVS

#endif // __AVCVIDEOSERVICES_AVCDEVICECOMMANDINTERFACE__

