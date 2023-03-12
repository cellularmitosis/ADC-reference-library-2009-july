/*
	File:		MPEG2Transmitter.h

    Synopsis: This is the header file for the MPEG2Transmitter class. 

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

#ifndef __AVCVIDEOSERVICES_MPEG2TRANSMITTER__
#define __AVCVIDEOSERVICES_MPEG2TRANSMITTER__

namespace AVS
{

// enum for this module's messages
enum
{
	kMpeg2TransmitterPreparePacketFetcher,
	kMpeg2TransmitterAllocateIsochPort,
	kMpeg2TransmitterReleaseIsochPort,
	kMpeg2TransmitterDCLOverrun,	/* Not currently used! */
	kMpeg2TransmitterTimeStampAdjust	
};

// enum for playback modes
enum
{
	kMpeg2TransmitterPlaybackModeForward = 0x60,
	kMpeg2TransmitterPlaybackModePause = 0x7D
};

// enum for transport states
enum
{
	kMpeg2TransmitterTransportStopped,
	kMpeg2TransmitterTransportPlaying
};

enum
{
	// Default values for transmit segment size, number of segments
	// and number of MPEG TS packets contained in cycles that have data.
	// These can be overriden in the transmitter class constructor
	kCyclesPerTransmitSegment = 1500,
	kNumTransmitSegments = 3,
	kNumTSPacketsPerCycle = 2,

	// Define the cycle count value to transmit in source
	// packet headers when isoch transmit starts. We
	// always wait for cycle-count = 0 before starting
	// isoch.
	kMPEGSourcePacketCycleCountStartValue = 4,

	// Define the default number of transport stream packets
	// in the processing queue
	kTSPacketQueueSizeInPackets = 2048,
	
	// Define how many transport stream packets we allow
	// between PCRs before giving up, invalidating the current PSI
	// tables and rescanning for new PSI.
	kMPEG2TransmitterMaxPacketsBetweenPCRs = 10000
};

// To prevent erroneously low bit-rate calculations from stalling the transmitter,
// the MPEG2Transmitter will ignore bit-rates lower than the following threshold.
// These erroneously low bit-rate calculations may result from stream 
// discontinuties, or packet errors.
#define kMPEG2TransmitterLowBitRateThreshold 400000.0

// Function prototype for data pull callback.
// Notes: The registered data-pull function is called every time the
// MPEG transmitter is ready for the next TS packet. The application
// that implements this callback should provide a pointer to its buffer
// that contains the 188 byte packet. The application should not modify
// the contents of this buffer after returning from this callback, until
// either the next callback, or until the transmitter is stopped. Setting
// the discontinuity flag will result in the mpeg transmitter not modifying
// the mpeg data rate until two subsequent packets containing PCRs are received.
typedef IOReturn (*DataPullProc) (UInt32 **ppBuf, bool *pDiscontinuityFlag, void *pRefCon);

// Function prototype for message callback.
typedef void (*MPEG2TransmitterMessageProc) (UInt32 msg, UInt32 param1, UInt32 param2, void *pRefCon);

//
// The MPEG2 Transmitter Class Declaration
//
class MPEG2Transmitter
{
public:

	// Constructor
    MPEG2Transmitter(StringLogger *stringLogger = nil,
					 IOFireWireLibNubRef nubInterface = nil,
					 unsigned int cyclesPerSegment = kCyclesPerTransmitSegment,
					 unsigned int numSegments = kNumTransmitSegments,
					 bool doIRMAllocations = false,
					 unsigned int packetsPerCycle = kNumTSPacketsPerCycle,
					 unsigned int tsPacketQueueSizeInPackets = kTSPacketQueueSizeInPackets);

    // Destructor
    ~MPEG2Transmitter();

    // Function to setup all the isoc stuff
    IOReturn setupIsocTransmitter(void);

	// Set the isoch transmit channel
	IOReturn setTransmitIsochChannel(unsigned int chan);

	// Set the isoch transmit speed
	IOReturn setTransmitIsochSpeed(IOFWSpeed speed);

	// Start/Stop routines
	IOReturn startTransmit(void);
	IOReturn stopTransmit(void);

	// Function to install a handler for pulling data
	IOReturn registerDataPullCallback(DataPullProc handler, void *pRefCon);

	// Function to install a handler for receiving messages
	IOReturn registerMessageCallback(MPEG2TransmitterMessageProc handler, void *pRefCon);
	
	// Publically visible vars
	UInt8 playbackMode;	// Allows for user to signal entering/exiting pause mode
    double mpegDataRate;
	unsigned int transportState;

	// A reference to the current run loop for isoch callbacks
	CFRunLoopRef runLoopRef;

private:

	// Function to pre-initialize all the isoch xmit buffers
	IOReturn prepareForTransmit(void);

	// Registered Handler functions
	DataPullProc packetFetch;
	void *pPacketFetchRefCon;
	MPEG2TransmitterMessageProc messageProc;
	void *pMessageProcRefCon;
	
    // Function to create MPEG2 source packet
    // headers from currentMPEGTime
    unsigned int sourcePacketHeader(void);

	// Interface pointers
	IOFireWireLibNubRef nodeNubInterface;
	IOFireWireLibRemoteIsochPortRef remoteIsocPort ;
	IOFireWireLibLocalIsochPortRef localIsocPort;
	IOFireWireLibIsochChannelRef isochChannel;
	
	void FillCycleBuffer(MPEG2XmitCycle *pCycle, UInt16 nodeID, bool doUpdateJumpTarget);
	TSPacket *GetNextTSPacketQueuePacket(void);
	
	IOFireWireLibDCLCommandPoolRef dclCommandPool;
	DCLCommandStruct *pFirstDCL;
	DCLCommandStruct *pOverrunCallbackLabel;
	MPEG2XmitCycle *pFirstCycleObject;
	MPEG2XmitCycle *pNextUpdateCycle;
	MPEG2XmitCycle *pLastCallbackCycle;
	MPEG2XmitCycle **ppCallbackCycles;
	unsigned int dclCommandPoolSize;
	UInt32 *pTimeStamps;
	TSPacket *pPacketQueueHead;
	TSPacket *pLastPCRPacket;
	unsigned int *pPacketQueueBuf;
	TSPacket *pPacketQueueIn;
	TSPacket *pPacketQueueOut;
	
    // Other vars
    double currentIsochTime;
    double currentMPEGTime;
    double isochCycleClocksPerTSPacket;
	double lastPCR;
	unsigned int packetsBetweenPCR;
    unsigned int dbcCount;
	unsigned int xmitChannel;
	IOFWSpeed xmitSpeed;
	PSITables *psiTables;
	bool firstPCRFound;
	UInt8 *pTransmitBuffer;
	unsigned int isochCyclesPerSegment;
	unsigned int isochSegments;

	bool doIRM;
	bool noLogger;
	unsigned int tsPacketsPerCycle;
	unsigned int xmitBufferSize;
	UInt32 currentSegment;
	UInt32 expectedTimeStampCycle;
	volatile bool finalizeCallbackCalled;
	UInt32 numTSPacketsInPacketQueue;
	pthread_mutex_t transportControlMutex;

	// Packet Processing Queue Functions
	void AddPacketToTSPacketQueue(void);
	
	StringLogger *logger;
	
public:
	// Callbacks for remote isoch port object
	IOReturn RemotePort_GetSupported(
								  IOFireWireLibIsochPortRef interface,
								  IOFWSpeed *outMaxSpeed,
								  UInt64 *outChanSupported);
	IOReturn RemotePort_AllocatePort(
								  IOFireWireLibIsochPortRef interface,
								  IOFWSpeed maxSpeed,
								  UInt32 channel);
	IOReturn RemotePort_ReleasePort(
								 IOFireWireLibIsochPortRef interface);
	IOReturn RemotePort_Start(
						   IOFireWireLibIsochPortRef interface);
	IOReturn RemotePort_Stop(
						  IOFireWireLibIsochPortRef interface);

	// Callbacks for DCL
	void MPEG2XmitDCLCallback(MPEG2XmitCycle *pCallBackCycle);
	
	void MPEG2XmitDCLOverrunCallback();
	void MPEG2XmitFinalizeCallback(void);

#ifdef kAVS_Enable_ForceStop_Handler	
	// Force Stop Callback
	void MPEG2XmitForceStop(UInt32 stopCondition);
#endif
	
};

} // namespace AVS

#endif // __AVCVIDEOSERVICES_MPEG2TRANSMITTER__