/*
	File:		DVReceiver.h

    Synopsis: This is the header file for the DVReceiver class. 
 
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

#ifndef __AVCVIDEOSERVICES_DVRECEIVER__
#define __AVCVIDEOSERVICES_DVRECEIVER__

namespace AVS
{

// enum for this module's messages
enum
{
	kDVReceiverReceivedBadPacket,
	kDVReceiverDCLOverrun,
	kDVReceiverAllocateIsochPort,
	kDVReceiverReleaseIsochPort
};

// enum for transport states
enum
{
	kDVReceiverTransportStopped,
	kDVReceiverTransportRecording
};

// enum for message passed in Frame Received Callback
typedef unsigned int DVFrameReceiveMessage;
enum
{
	kDVFrameReceivedSuccessfully,
	kDVFrameDropped,
	kDVFrameCorrupted,
	kDVFrameWrongMode
};

// Default values for receive segment size and number of segments.
// These can be overriden in the receiver class constructor
enum
{
	kNumDVReceiveSegments	= 3,
	kCyclesPerDVReceiveSegment = 800,
	kDVReceiveNumFrames = 4
};

// Structure for frame data
// TODO: Change this to a class with accessor functions
// to hide the private members of this structure
struct DVReceiveFrame
{
	// The client can access these, but should never modify them
	UInt8 *pFrameData;
	UInt32 frameLen;
	UInt32 frameSYTTime;
	UInt32 frameReceivedTimeStamp;
	UInt8 frameMode;

	// These parameters are should be considered off-limits to the clients
	UInt32 refCount;
	UInt32 frameBufferSize;
	UInt32 currentOffset;

	// A pointer to the specific DVReceiver object that created this frame struct
	class DVReceiver *pDVReceiver;
};

// Function prototype for Frame Received Callback.
typedef IOReturn (*DVFrameReceivedProc) (DVFrameReceiveMessage msg, DVReceiveFrame* pFrame, void *pRefCon);

// Function prototype for message callback.
typedef void (*DVReceiverMessageProc) (UInt32 msg, UInt32 param1, UInt32 param2, void *pRefCon);

// Function prototype for "no-data" callback.
typedef IOReturn (*DVNoDataProc) (void *pRefCon);

// Structure containing vars for each receive segment
struct DVReceiveSegment
{
	DCLLabelPtr pSegmentLabelDCL;
	DCLJumpPtr pSegmentJumpDCL;
};

// Struct for maintaining a list of Frame Receive Notify Clients
struct DVFrameNotifyInst
{
	DVFrameReceivedProc handler;
	void *refCon;
};

//
// The DV Receiver Class Declaration
//
class DVReceiver
{

public:
	// Constructor
    DVReceiver(StringLogger *stringLogger = nil,
			   IOFireWireLibNubRef nubInterface = nil,
			   UInt32 numReceiveFrames = kDVReceiveNumFrames,
			   UInt8 receiverDVMode = 0x00,
			   unsigned int cyclesPerSegment = kCyclesPerDVReceiveSegment,
			   unsigned int numSegments = kNumDVReceiveSegments,
			   bool doIRMAllocations = false);

    // Destructor
    ~DVReceiver();

    // Function to setup all the isoc stuff
    IOReturn setupIsocReceiver(void);

	// Set the isoch receive channel
	IOReturn setReceiveIsochChannel(unsigned int chan);

	// Set the isoch receive speed -
	// Note: Only important to set if doIRMAllocations is enabled in class constructor
	IOReturn setReceiveIsochSpeed(IOFWSpeed speed);

	// Start/Stop routines
	IOReturn startReceive(void);
	IOReturn stopReceive(void);

	// Functions to install/remove a handler for receiving DV Frames
	// Can be called more than once if multiple notifications are desired
	IOReturn registerFrameReceivedCallback(DVFrameReceivedProc handler, void *refCon, DVFrameNotifyInst* *ppNotifyInstance);
	IOReturn unregisterFrameReceivedCallback(DVFrameNotifyInst* pNotifyInstance);

	// Function to install a handler for receiving messages
	IOReturn registerMessageCallback(DVReceiverMessageProc handler, void *pRefCon);

	// Function to install a handler for "no-data" notifications
	IOReturn registerNoDataNotificationCallback(DVNoDataProc handler, void *pRefCon, UInt32 noDataTimeInMSec);
	
	// Function to release a DVReceiveFrame that was passed to the client via the FrameReceivedProc
	IOReturn releaseFrame(DVReceiveFrame* pFrame);

	// Publically visible vars
	unsigned int transportState;

	// A reference to the current run loop for isoch callbacks
	CFRunLoopRef runLoopRef;
	
private:

	// Take the dvMode, and find the DVFormat info
	IOReturn FindDVFormatForMode(void);

	// Get a frame off the frame queue
	DVReceiveFrame* getNextQueuedFrame(void);
	DVReceiveFrame* pCurrentFrame;
	
	// Interface pointers
	IOFireWireLibNubRef nodeNubInterface;
	IOFireWireLibDCLCommandPoolRef dclCommandPool;
	IOFireWireLibRemoteIsochPortRef remoteIsocPort ;
	IOFireWireLibLocalIsochPortRef localIsocPort;
	IOFireWireLibIsochChannelRef isochChannel;
	StringLogger *logger;

	// Call to fixup jump targets in receive dcl
	void fixupDCLJumpTargets(void);
	
	// Class Variables
	UInt8 dvMode;
	DCLCommandPtr *updateDCLList;
	DCLCommandStruct *pFirstDCL;
	unsigned int receiveChannel;
	IOFWSpeed receiveSpeed;
    UInt8 *pReceiveBuffer;
	DVReceiveSegment *receiveSegmentInfo;
	DCLLabelPtr pDCLOverrunLabel;
	UInt32 currentSegment;
	unsigned int isochCyclesPerSegment;
	unsigned int isochSegments;
	unsigned int dclCommandPoolSize;
	bool doIRM;
	bool noLogger;
	unsigned int dclVMBufferSize;
	UInt32 numFrames;
	DVFormats* pDVFormat;
	UInt32 receiveCycleBufferSize;
	UInt32 *pTimeStamps;
	pthread_mutex_t frameQueueMutex;
	UInt32 numFrameClients;
	UInt32 *pOverrunReceiveBuffer;
	volatile bool finalizeCallbackCalled;
	pthread_mutex_t transportControlMutex;
	std::deque<DVReceiveFrame*> frameQueue;
		
	// Message Handler functions
	DVReceiverMessageProc messageProc;
	void *pMessageProcRefCon;
	std::deque<DVFrameNotifyInst*> frameNotifyQueue;
	
	// No data callback timer stuff
	void startNoDataTimer( void );
	void stopNoDataTimer( void );

	CFRunLoopTimerRef noDataTimer;
	pthread_mutex_t noDataTimerMutex;
	DVNoDataProc noDataProc;
	void *pNoDataProcRefCon;
	double noDataTimeLimitInSeconds;

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
	void DVReceiveDCLCallback(void);
	void DVReceiveOverrunDCLCallback(void);
	void DVReceiveFinalizeCallback(void);

	// Callback for timer timeout
	void NoDataTimeout(void);
	
#ifdef kAVS_Enable_ForceStop_Handler	
	// Force Stop Callback
	void DVReceiveForceStop(UInt32 stopCondition);
#endif
	
};

} // namespace AVS

#endif // __AVCVIDEOSERVICES_DVRECEIVER__
