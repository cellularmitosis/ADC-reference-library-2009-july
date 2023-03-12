/*
	File:		DVTransmitter.h

    Synopsis: This is the header file for the DVTransmitter class. 

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

#ifndef __AVCVIDEOSERVICES_DVTRANSMITTER__
#define __AVCVIDEOSERVICES_DVTRANSMITTER__

namespace AVS
{

// enum for this module's messages
enum
{
	kDVTransmitterPreparePacketFetcher,
	kDVTransmitterAllocateIsochPort,
	kDVTransmitterReleaseIsochPort,
	kDVTransmitterDCLOverrun,
	kDVTransmitterTimeStampAdjust	
};

// enum for transport states
enum
{
	kDVTransmitterTransportStopped,
	kDVTransmitterTransportPlaying
};

enum
{
	// Default values for transmit segment size, number of segments
	// These can be overriden in the transmitter class constructor
	kCyclesPerDVTransmitSegment = 600,
	kNumDVTransmitSegments = 3,
	kDVTransmitNumFrameBuffers = 4,
	
	// Define the maximum number of lost-cycles
	// that we will try to recover after a 
	// time-stamp discrepancy, by discarding
	// future CIP-only cycles
	kDVTransmitterLostCycleRecoveryThreshold = 15
};

// Structure for frame data
// TODO: Change this to a class with accessor functions
// to hide the private members of this structure
struct DVTransmitFrame
{
	// This is for the client to use for his own queue
	DVTransmitFrame *pNext;
	
	// The client can access these, but should never modify them
	UInt32 frameIndex;
	UInt8 *pFrameData;
	UInt32 frameLen;
	UInt32 frameSYTTime;
	UInt32 frameTransmitStartCycleTime;

	// The following three parameters are should be considered off-limits to the clients
	bool inUse;
	UInt32 curOffset;
	UInt32 dclProgramRefCount;

	// If timeStampSecondsFieldValid is true, then the 3-bits of "seconds field" in the 
	// frameTransmitStartCycleTime are valid (bits 27:25). Otherwise, if
	// timeStampSecondsFieldValid is false, the 3-bits of "seconds field" are not valid.
	// Note: This is needed because we cannot guarrantee an accurate value of the "seconds field"
	// in the frameTransmitStartCycleTime time-stamp until the DCL program is running ,and
	// we get our first DCL callback, because the DCL engine starts on any 1-second boundary.
	bool timeStampSecondsFieldValid;
};

// Prototype for callback to ask the client for a frame
typedef IOReturn (*DVFramePullProc) (UInt32 *pFrameIndex, void *pRefCon);

// Prototype for callback to release a frame back to the client
typedef IOReturn (*DVFrameReleaseProc) (UInt32 frameIndex, void *pRefCon);

// Prototype for message callback.
typedef void (*DVTransmitterMessageProc) (UInt32 msg, UInt32 param1, UInt32 param2, void *pRefCon);

//
// The DV Transmitter Class Declaration
//
class DVTransmitter
{
public:

	// Constructor
    DVTransmitter(StringLogger *stringLogger = nil,
				  IOFireWireLibNubRef nubInterface = nil,
				  UInt8 transmitterDVMode = 0x00,
				  UInt32 numFrameBuffers = kDVTransmitNumFrameBuffers,
				  unsigned int cyclesPerSegment = kCyclesPerDVTransmitSegment,
				  unsigned int numSegments = kNumDVTransmitSegments,
				  bool doIRMAllocations = false);
    // Destructor
    ~DVTransmitter();

    // Function to setup all the isoc stuff
    IOReturn setupIsocTransmitter(void);

	// Set the isoch transmit channel
	IOReturn setTransmitIsochChannel(unsigned int chan);

	// Set the isoch transmit speed
	IOReturn setTransmitIsochSpeed(IOFWSpeed speed);
	
	// Accessors for chan and speed
	unsigned int getTransmitIsochChannel(void) {return xmitChannel;};
	IOFWSpeed getTransmitIsochSpeed(void) {return xmitSpeed;};
	
	
	// Start/Stop routines
	IOReturn startTransmit(void);
	IOReturn stopTransmit(void);

	// Function to install a handler for pulling data
	IOReturn registerFrameCallbacks(DVFramePullProc framePullHandler,
								 void *pFramePullHandlerRefCon,
								 DVFrameReleaseProc frameReleaseHandler,
								 void *pFrameReleaseHandlerRefCon);

	// Function to install a handler for receiving messages
	IOReturn registerMessageCallback(DVTransmitterMessageProc handler, void *pRefCon);

	// Returns the total number of frame structs allocated
	UInt32 getNumFrames(void);

	// Convert a frame index into a pointer to the frame struct
	DVTransmitFrame* getFrame(UInt32 frameIndex);
	
	// Publically visible vars
	unsigned int transportState;

	// A reference to the current run loop for isoch callbacks
	CFRunLoopRef runLoopRef;

private:

	// Function to pre-initialize all the isoch xmit buffers
	IOReturn prepareForTransmit(bool notifyClient);
		
	// Internal function used during DCL callbacks
	void FillCycleBuffer(DVXmitCycle *pCycle, UInt16 nodeID, bool doUpdateJumpTarget);

	// Function to create DV SYT fields
	// from currentDVTime
    unsigned int calculateSYTTime(void);

	// Take the dvMode, and find the DVFormat info
	IOReturn FindDVFormatForMode(void);
	
	// Frame handler functions
	DVFramePullProc framePullProc;
	void *pFramePullProcRefCon;
	DVFrameReleaseProc frameReleaseProc;
	void *pFrameReleaseProcRefCon;

	DVTransmitterMessageProc messageProc;
	void *pMessageProcRefCon;

	// Interface pointers
	IOFireWireLibNubRef nodeNubInterface;
	IOFireWireLibDCLCommandPoolRef dclCommandPool;
	IOFireWireLibRemoteIsochPortRef remoteIsocPort ;
	IOFireWireLibLocalIsochPortRef localIsocPort;
	IOFireWireLibIsochChannelRef isochChannel;

	DVXmitCycle *pFirstCycleObject;
	DVXmitCycle *pNextUpdateCycle;
	DVXmitCycle *pLastCallbackCycle;
	DVXmitCycle **ppCallbackCycles;

    // Other vars
	DCLCommandStruct *pFirstDCL;
	DCLCommandStruct *pOverrunCallbackLabel;

	UInt8 dvMode;
	UInt8 dbc;
	UInt32 numFrames;
	DVFormats* pDVFormat;
    double currentIsochTime;
	double currentDVTime;
	unsigned int xmitChannel;
	IOFWSpeed xmitSpeed;
	UInt8 *pTransmitBuffer;
	unsigned int isochCyclesPerSegment;
	unsigned int isochSegments;
	unsigned int dclCommandPoolSize;
	bool doIRM;
	bool noLogger;
	unsigned int xmitPayloadSize;
	UInt32 *pTimeStamps;
	UInt32 currentSegment;
	UInt32 expectedTimeStampCycle;
	UInt32 transmitBufferSize;
	DVTransmitFrame** framePtrs;
	StringLogger *logger;
	DVTransmitFrame* pCurrentFrame;
	volatile bool finalizeCallbackCalled;
	pthread_mutex_t transportControlMutex;
	UInt32 sourcePacketCycleCountStartValue;
	bool firstDCLCallbackOccurred;
	
#ifdef DVTransmitter_DCL_Callback_Timing
	// Vars for storing DCL Callback timing parameters
	UInt64 DCL_Timing_Accumulator;
	UInt64 DCL_Timing_Max;
	UInt64 DCL_Timing_Min;
	UInt32 DCL_Timing_Count;
#endif	

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
	void DVXmitDCLCallback(DVXmitCycle *pCallBackCycle);
	void DVXmitDCLOverrunCallback(void);
	void DVXmitFinalizeCallback(void);
	
#ifdef kAVS_Enable_ForceStop_Handler	
	// Force Stop Callback
	void DVXmitForceStop(UInt32 stopCondition);
#endif
	
};

} // namespace AVS

#endif // __AVCVIDEOSERVICES_DVTRANSMITTER__