/*
	File:		AVCDevice.h

 Synopsis: This is the header file for the AVCDevice class.

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

#ifndef __AVCVIDEOSERVICES_AVCDEVICE__
#define __AVCVIDEOSERVICES_AVCDEVICE__

namespace AVS
{

typedef unsigned int AVCStreamType;
enum
{
	kStreamTypeDVTransmitter,
	kStreamTypeDVReceiver,
	kStreamTypeMPEGTransmitter,
	kStreamTypeMPEGReceiver
};

struct AVCDeviceStream
{
	AVCDeviceStream *pNext;
	AVCStreamType streamType;
	class AVCDevice *pAVCDevice;
	UInt8 devicePlugNum;
	StringLogger *stringLogger;
	unsigned int cyclesPerSegment;
	unsigned int numSegments;
	UInt32 isochChannel;
	IOFWSpeed isochSpeed;
	bool doP2PConnection;
	bool plugConnected;
	
	// TODO: Temporarily commented out the following unions.
	// They seem to cause problems with the debugger.
	
	//union
	//{
		DVTransmitter *pDVTransmitter;
		DVReceiver *pDVReceiver;
		MPEG2Transmitter *pMPEGTransmitter;
		MPEG2Receiver *pMPEGReceiver;
	//};

	//union
	//{
		DVTransmitterMessageProc dvTransmitterMessageProc;
		DVReceiverMessageProc dvReceiverMessageProc;
		MPEG2TransmitterMessageProc mpegTransmitterMessageProc;
		MPEG2ReceiverMessageProc mpegReceiverMessageProc;
	//};
	void *pMessageProcRefCon;
	
	//union
	//{
		DVFramePullProc dvFramePullProc;
		DVFrameReceivedProc dvFrameReceivedProc;
		DataPullProc mpegDataPullProc;
		DataPushProc mpegDataPushProc;
	//};
	void *pDataProcRefCon;
	
	//union
	//{
		unsigned int numFrameBuffers;   // used by dv transmit/receive type streams
		unsigned int packetsPerCycle;   // used by mpeg transmit type streams
	//};
	
	// These parameters only used by DV transmit type streams
	DVFrameReleaseProc dvFrameReleaseProc;
	void *pDVFrameReleaseProcRefCon;
	
	// This parameters only used by DV transmit/receive type streams
	UInt8 dvMode;
	
	// This parameter only used by MPEG transmit type streams
	unsigned int tsPacketQueueSizeInPackets;
};

#define kAVCPowerStateOn 0x70
#define kAVCPowerStateOff 0x60

class AVCDeviceController;

class AVCDevice
{
public:
	// Constructor
	AVCDevice(AVCDeviceController *pDeviceController,io_object_t newDevice,UInt64 newGUID);

	// Destructor
	~AVCDevice();

	// ReInit a device already in our database
	IOReturn ReInit(io_object_t newDevice);

	// Send an AVC command to this (opened) AVC Device
	IOReturn AVCCommand(const UInt8 *command, UInt32 cmdLen, UInt8 *response, UInt32 *responseLen);

	// The function for a client to use to open this device
	IOReturn openDevice(AVCDeviceMessageNotification deviceMessageProc = nil, void *pMessageProcRefCon = nil);

	// The function for a client to use to open this device
	IOReturn closeDevice();

	// Check to see if the device has been opened by this object
	bool isOpened();
	
	// Device Identifier Information
	UInt64 guid;
	char deviceName[80];
	char vendorName[40];
	UInt32 vendorID;
	UInt32 modelID;
	
	// Capabilities
	bool isAttached;
	bool supportsFCP;
	bool hasTapeSubunit;
	bool hasMonitorOrTunerSubunit;
	UInt32 subUnits;
	UInt32 numInputPlugs;
	UInt32 numOutputPlugs;
	bool capabilitiesDiscovered;

	// Member Vars specific to DV devices
	bool isDVDevice;
	bool isDVCProDevice;
	UInt8 dvMode;

	// Member Vars specific to MPEG devices
	bool isMPEGDevice;
	UInt8 mpegMode;
	
	// Interfaces
	AVCDeviceController *pAVCDeviceController;
	IOFireWireAVCLibUnitInterface **avcInterface;
	IOFireWireLibDeviceRef deviceInterface;

	// IORegistry Objects
	io_service_t avcUnit;
	io_service_t fwUnit;
	io_service_t fwDevice;
	io_service_t busController;
	io_object_t	interestNotification ;

	//
	// APIs for "managed" DV/MPEG stream objects associated with the AVC device.
	//
	// Use these APIs to create DV/MPEG Transmitter/Receiver objects that have
	// isoch channel, speed, IRM, and device connection-managemnt (CMP), all
	// handled internally by the AVCDevice class object.
	//
	// The users of these APIs should never have to call the APIs of the 
	// DV/MPEG Transmitter/Receiver objects directly.
	//
	
	AVCDeviceStream* CreateMPEGTransmitterForDevicePlug(UInt8 deviceInputPlugNum,
													 DataPullProc dataPullProcHandler,
													 void *pDataPullProcRefCon = nil,
													 MPEG2TransmitterMessageProc messageProcHandler = nil,
													 void *pMessageProcRefCon = nil,
													 StringLogger *stringLogger = nil,
													 unsigned int cyclesPerSegment = kCyclesPerTransmitSegment,
													 unsigned int numSegments = kNumTransmitSegments,
													 unsigned int packetsPerCycle = kNumTSPacketsPerCycle,
													 unsigned int tsPacketQueueSizeInPackets = kTSPacketQueueSizeInPackets);
	
	AVCDeviceStream* CreateMPEGReceiverForDevicePlug(UInt8 deviceOutputPlugNum,
												  DataPushProc dataPushProcHandler,
												  void *pDataPushProcRefCon = nil,
												  MPEG2ReceiverMessageProc messageProcHandler = nil,
												  void *pMessageProcRefCon = nil,
												  StringLogger *stringLogger = nil,
												  unsigned int cyclesPerSegment = kCyclesPerReceiveSegment,
												  unsigned int numSegments = kNumReceiveSegments);
	
	AVCDeviceStream* CreateDVTransmitterForDevicePlug(UInt8 deviceInputPlugNum,
												   DVFramePullProc framePullProcHandler,
												   void *pFramePullProcRefCon,
												   DVFrameReleaseProc frameReleaseProcHandler,
												   void *pFrameReleaseProcRefCon = nil,
												   DVTransmitterMessageProc messageProcHandler = nil,
												   void *pMessageProcRefCon = nil,
												   StringLogger *stringLogger = nil,
												   unsigned int cyclesPerSegment = kCyclesPerDVTransmitSegment,
												   unsigned int numSegments = kNumDVTransmitSegments,
												   UInt8 transmitterDVMode = 0x00,
												   UInt32 numFrameBuffers = 8);
	
	AVCDeviceStream* CreateDVReceiverForDevicePlug(UInt8 deviceOutputPlugNum,
												DVFrameReceivedProc frameReceivedProcHandler,
												void *pFrameReceivedProcRefCon = nil,
												DVReceiverMessageProc messageProcHandler = nil,
												void *pMessageProcRefCon = nil,
												StringLogger *stringLogger = nil,
												unsigned int cyclesPerSegment = kCyclesPerDVReceiveSegment,
												unsigned int numSegments = kNumDVReceiveSegments,
												UInt8 receiverDVMode = 0x00,
												UInt32 numFrameBuffers = kDVReceiveNumFrames);

	// Start a managed AVC device stream
	IOReturn StartAVCDeviceStream(AVCDeviceStream *pAVCDeviceStream);
	
	// Stop a managed AVC device stream
	IOReturn StopAVCDeviceStream(AVCDeviceStream *pAVCDeviceStream);
	
	// Tear down and destroy a managed AVC device stream
	IOReturn DestroyAVCDeviceStream(AVCDeviceStream *pAVCDeviceStream);

	// Support for some AV/C unit-level commands
	IOReturn GetPowerState(UInt8 *pPowerState);
	IOReturn SetPowerState(UInt8 powerState);
	
	// Returns the current FireWire node ID for this device 
	IOReturn GetCurrentNodeID(UInt16 *pNodeID);
	
	// Set/Get Client Private Data - Allows a client to attach a pointer
	// to other data associated with this device.
	void SetClientPrivateData(void *pClientData);
	void *GetClientPrivateData(void);
	
	// Linked list of AVCDeviceStreams assoicated with this device
	AVCDeviceStream *pAVCDeviceStreamQueueHead;
	
	// Semi-Private: Only used by internal registered callbacks
	IOReturn discoverAVCDeviceCapabilities(void);
	AVCDeviceMessageNotification clientDeviceMessageProc;
	void *pClientMessageProcRefCon;
	pthread_mutex_t deviceStreamQueueMutex;
	
private:
	IOReturn createAVCUnitInterface(void);
	IOReturn releaseAVCUnitInterface(void);
	IOReturn createFWDeviceInterface(void);
	IOReturn releaseFWDeviceInterface(void);
	IOReturn getIsochPlugCount(void);
	IOReturn open(void);
	IOReturn close(void);
	
	pthread_mutex_t deviceOpenCloseMutex;
	void *pClientPrivateData;
};

} // namespace AVS

#endif // __AVCVIDEOSERVICES_AVCDEVICE__


