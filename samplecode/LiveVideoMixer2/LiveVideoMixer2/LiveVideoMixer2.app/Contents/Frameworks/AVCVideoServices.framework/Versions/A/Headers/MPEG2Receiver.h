/*
	File:		MPEG2Receiver.h

    Synopsis: This is the header file for the MPEG2Receiver class. 
 
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

#ifndef __AVCVIDEOSERVICES_MPEG2RECEIVER__
#define __AVCVIDEOSERVICES_MPEG2RECEIVER__

namespace AVS
{

// enum for this module's messages
enum
{
	kMpeg2ReceiverReceivedBadPacket,
	kMpeg2ReceiverDCLOverrun,
	kMpeg2ReceiverAllocateIsochPort,
	kMpeg2ReceiverReleaseIsochPort
};

// enum for transport states
enum
{
	kMpeg2ReceiverTransportStopped,
	kMpeg2ReceiverTransportRecording
};

// Default values for receive segment size and number of segments.
// These can be overriden in the receiver class constructor
enum
{
	kNumReceiveSegments	= 3,
	kCyclesPerReceiveSegment = 2000
};

// Other defines
#define kMaxNumReceivePacketsPerCycle 5
#define kMPEG2ReceiveBufferSize ((kMaxNumReceivePacketsPerCycle*192)+16)

// Structure containing vars for each receive segment
struct MPEGReceiveSegment
{
	DCLLabelPtr pSegmentLabelDCL;
	DCLJumpPtr pSegmentJumpDCL;
};

///////////////////////////////////////////////////////////////////////////////////////
//
// Data Push Callbacks:
//
// Due to the desire to keep backwards compatibility for users of older versions of
// this module, while still being able to provide new features to current users,
// there are now three different methods (i.e. three different callbacks) used to push 
// data to the clients. They are
//
//  1) The original DataPushProc(...), which is the default callback, and is 
//  installed using registerDataPushCallback(...), or via the helper APIs provided by
//  FireWireMPEG.h and AVCDevice.h (for instatiating and setting up a MPEG2Receiver on a 
//  dedicated thread). This callback provides just a count of ts packets (from a single
//  cycle of the DCL program), and and array of pointers to those packets. A refcon is
//  also provided to the client.
//
//  2) The ExtendedDataPushProc(...), which is installed using the method call
//  registerExtendedDataPushCallback(...), and provides the basic info from the default
//  DataPushProc, plus the additions of CIP header, isoch header, and firewire timestamp data.
//  It is still called for each cycle that contains ts data.
//
//  3) The StructuredDataPushProc(...), which is installed using the method call
//  registerStructuredDataPushCallback(...), and provides all the same info from the
//  ExtendedDataPushProc, plus the addition of nano-seconds timestamps (based on the system up-time). 
//  All this info from one cycle is contained in a struct (MPEGReceiveCycleData), and the callback passes
//  an array of pointers to one or more of these MPEGReceiveCycleData structs, the maximum
//  number of which is specified in the call to registerStructuredDataPushCallback(...), or
//  the sepecified cyclesPerSegment, whichever is smaller. This allows a client to only
//  receive one callback per segment of the program, or one per cycle, or anything in between.
//  
//  Only one of these callback methods will be called, and the precedence is the following:
//
//		A - If a callback is installed using registerStructuredDataPushCallback(...) the 
//      StructuredDataPushProc will be used, overriding any ExtendedDataPushProc, or DataPushProc
//      installed.
//
//      B - If registerStructuredDataPushCallback(...) is not called (or called with a pointer
//      to NULL for the StructuredDataPushProc, and a callback is installed using
//      registerExtendedDataPushCallback, the ExtendedDataPushProc will be used, overriding
//      any any DataPushProc installed.
//
//      C - Otherwise, the DataPushProc is used.
//
///////////////////////////////////////////////////////////////////////////////////////

// The prototypes for the three data-push callback schemes:

// Function prototype for data push callback.
typedef IOReturn (*DataPushProc) (UInt32 tsPacketCount, UInt32 **ppBuf, void *pRefCon);

// Function prototype for alternate (extended) data push callback.
typedef IOReturn (*ExtendedDataPushProc) (UInt32 tsPacketCount, 
										  UInt32 **ppBuf, 
										  void *pRefCon, 
										  UInt32 isochHeader,
										  UInt32 cipHeader0,
										  UInt32 cipHeader1,
										  UInt32 fireWireTimeStamp);

// Struct to hold all info regarding a single cycle's received MPEG data. 
// Only used by the "structured-data" callback method.
struct MPEGReceiveCycleData
{
	UInt32 tsPacketCount;
	UInt32 *pBuf[kMaxNumReceivePacketsPerCycle]; 
	void *pRefCon; 
	UInt32 isochHeader;
	UInt32 cipHeader0;
	UInt32 cipHeader1;
	UInt32 fireWireTimeStamp;
	UInt64 nanoSecondsTimeStamp;
};

// Function prototype for alternate "structured" data push callback.
typedef IOReturn (*StructuredDataPushProc) (UInt32 CycleDataCount, MPEGReceiveCycleData *pCycleData, void *pRefCon);

///////////////////////////////////////////////////////////////////////////////////////

// Function prototype for message callback.
typedef void (*MPEG2ReceiverMessageProc) (UInt32 msg, UInt32 param1, UInt32 param2, void *pRefCon);

// Function prototype for "no-data" callback.
typedef IOReturn (*MPEG2NoDataProc) (void *pRefCon);

//
// The MPEG2 Receiver Class Declaration
//
class MPEG2Receiver
{

public:
	// Constructor
    MPEG2Receiver(StringLogger *stringLogger = nil,
				  IOFireWireLibNubRef nubInterface = nil,
				  unsigned int cyclesPerSegment = kCyclesPerReceiveSegment,
				  unsigned int numSegments = kNumReceiveSegments,
				  bool doIRMAllocations = false);

    // Destructor
    ~MPEG2Receiver();

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

	// Function to install a handler for receiving data packets
	IOReturn registerDataPushCallback(DataPushProc handler, void *pRefCon);

	// Function to install a handler for receiving data packets using the alternate "extended" callback prototype
	// Note: If a non-nil value is set for this extended callback, the standard callback will not be called.
	IOReturn registerExtendedDataPushCallback(ExtendedDataPushProc handler, void *pRefCon);

	// Function to install a handler for receiving data packets using the alternate "structured-data" callback prototype
	// Note: If a non-nil value is set for this structured-data callback, the standard callback and/or the extended
	// callback will not be called.
	IOReturn registerStructuredDataPushCallback(StructuredDataPushProc handler, UInt32 maxCycleStructsPerCallback, void *pRefCon);
	
	// Function to install a handler for receiving messages
	IOReturn registerMessageCallback(MPEG2ReceiverMessageProc handler, void *pRefCon);

	// Function to install a handler for "no-data" notifications
	IOReturn registerNoDataNotificationCallback(MPEG2NoDataProc handler, void *pRefCon, UInt32 noDataTimeInMSec);
	
	// Function to set whether the packet arrived callbacks include 
	// pointers to SPH+TS or just TS themselves
	void ReceiveSourcePacketHeaders(bool wantSPH);
	
	// Publically visible vars
    double mpegDataRate;	// Users should treat as read-only for status
	unsigned int transportState;

	// A reference to the current run loop for isoch callbacks
	CFRunLoopRef runLoopRef;
	
private:

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
	DCLCommandStruct *pFirstDCL;
	DCLCommandPtr *updateDCLList;
	unsigned int receiveChannel;
	IOFWSpeed receiveSpeed;
    UInt8 *pReceiveBuffer;
	MPEGReceiveSegment *receiveSegmentInfo;
	DCLLabelPtr pDCLOverrunLabel;
	UInt32 currentSegment;
	unsigned int isochCyclesPerSegment;
	unsigned int isochSegments;
	unsigned int dclCommandPoolSize;
	bool doIRM;
	bool noLogger;
	unsigned int dclVMBufferSize;
	volatile bool finalizeCallbackCalled;
	bool includeSPH;
	pthread_mutex_t transportControlMutex;
	pthread_mutex_t noDataTimerMutex;
	
	// Registered Handler functions
	DataPushProc packetPush;
	ExtendedDataPushProc extendedPacketPush;
	
	StructuredDataPushProc structuredDataPush;
	UInt32 maxNumStructuredDataStructsInCallback;
	MPEGReceiveCycleData *pCyclDataStruct;
		
	void *pPacketPushRefCon;
	MPEG2ReceiverMessageProc messageProc;
	void *pMessageProcRefCon;
	MPEG2NoDataProc noDataProc;
	void *pNoDataProcRefCon;
	double noDataTimeLimitInSeconds;
	UInt32 *pTimeStamps;
	UInt32 *pOverrunReceiveBuffer;
	
	// No data callback timer stuff
	void startNoDataTimer( void );
	void stopNoDataTimer( void );
	CFRunLoopTimerRef noDataTimer;
	
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
	void MPEG2ReceiveDCLCallback(void);
	void MPEG2ReceiveOverrunDCLCallback(void);
	void MPEG2ReceiveFinalizeCallback(void);
	
	// Callback for timer timeout
	void NoDataTimeout(void);

#ifdef kAVS_Enable_ForceStop_Handler	
	// Force Stop Callback
	void MPEG2ReceiveForceStop(UInt32 stopCondition);
#endif
	
};

} // namespace AVS

#endif // __AVCVIDEOSERVICES_MPEG2RECEIVER__
