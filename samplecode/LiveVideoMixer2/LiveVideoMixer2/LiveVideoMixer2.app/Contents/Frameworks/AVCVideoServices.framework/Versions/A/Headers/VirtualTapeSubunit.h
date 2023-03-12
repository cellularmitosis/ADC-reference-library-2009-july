/*
	File:		VirtualTapeSubunit.h
 
 Synopsis: This is the header file for the VirtualTapeSubunit class.
 
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

#ifndef __AVCVIDEOSERVICES_VIRTUALTAPESUBUNIT__
#define __AVCVIDEOSERVICES_VIRTUALTAPESUBUNIT__

namespace AVS
{

class VirtualTapeSubunit;

typedef unsigned int TapeTimeCodeFrameRate;
enum
{
	kTapeTimeCodeFrameRate_25fps,
	kTapeTimeCodeFrameRate_30fps,
	kTapeTimeCodeFrameRate_60fps,
	kTapeTimeCodeFrameRate_29_97fps	
};		
	
typedef unsigned int TapeMediumInfo;
enum
{
	// No tape
	kNoTape = 0x607F,
	
	// Unknown tape
	kUnknownTape = 0x7E7F,
	
	// Standard DV
	kDVStandardTape = 0x3130,
	kDVStandardTape_WriteProtect = 0x3131,
	
	// Small DV
	kDVSmallTape = 0x3230,
	kDVSmallTape_WriteProtect = 0x3231,
	
	// Medium DV
	kDVMediumTape = 0x3340,
	kDVMediumTape_WriteProtect = 0x3341,

	// VHS
	kVHSTape = 0x2230,
	kVHSTape_WriteProtect = 0x2231,
	kSVHSTape = 0x2240,
	kSVHSTape_WriteProtect = 0x2241,
	kDVHSTape = 0x2250,
	kDVHSTape_WriteProtect = 0x2251,

	// VHS-C
	kVHSCTape = 0x2330,
	kVHSCTape_WriteProtect = 0x2331,
	kSVHSCTape = 0x2340,
	kSVHSCTape_WriteProtect = 0x2341,
	
	// 8mm
	k8mmTape = 0x1230,
	k8mmTape_WriteProtect = 0x1231,
	kHi8Tape = 0x1250,
	kHi8Tape_WriteProtect = 0x1251,
	
	// MicroMV
	kMicroMVTape = 0x4130,
	kMicroMVTape_WriteProtect = 0x4131,

	// Analog audio
	kAnalogAudioTape = 0x0130,
	kAnalogAudioTape_WriteProtect = 0x0131
};	

// The spec says all isoch p2p connections must be reconnected
// within one second of each bus-reset. We'll wait 1.5 to be consertive
#define kTapeSubunitCMPBusResetReconnectTime 1.5 	// 1.5 seconds

// Need some additional error codes for this module;
#define kIOReturnAcceptWithNoAutoStateChange iokit_common_err(0x500)  	

/////////////////////////////////////////////////////////
// Callback Prototypes for this class object's clients
/////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////
//
// Transport state change requested by incoming AV/C command
//
// The client must return from this command with one of the following return codes:
//
// kIOReturnSuccess - Accept the transport state change, and set the new transport state as "stable"
// kIOReturnNotReady - Accept the transport state change, and set the new transport state as "in-transition"
// kIOReturnAcceptWithNoAutoStateChange - Accept the transport state change, but don' automatically modify the transport state
//      Note: For this return value, it is expected that the client calls setTransportState(...) within the callback
// Any other return value - Reject the transport change
//
////////////////////////////////////////////////////////////////////////////////////////////////
typedef IOReturn (*VirtualTapeTransportStateChangeHandler)(void *pRefCon, UInt8 newTransportMode, UInt8 newTransportState); 

////////////////////////////////////////////////////////////////////////////////////////////////
//
// Signal mode change requested by incoming AV/C command
//
// The client must return from this command with one of the following return codes:
//
// kIOReturnSuccess - Accept the signal mode change
// Any other return value - Reject the signal mode change
//
////////////////////////////////////////////////////////////////////////////////////////////////
typedef IOReturn (*VirtualTapeSignalModeChangeHandler)(void *pRefCon, UInt8 newSignalMode);

////////////////////////////////////////////////////////////////////////////////////////////////
//
// Time-code reposition requested by incoming AV/C command
//
// The client must return from this command with one of the following return codes:
//
// kIOReturnSuccess - Accept the time-code control command (client then should do reposition)
// Any other return value - Reject the time-code control command
//
// NOTES: If the client returns kIOReturnSuccess from this callback, then the VirtualTapeSubunit
// object will accept the AV/C time-code "reposition" control command. Proper AV/C proceedures
// after accepting the command are to change the transportMode/transportState to wind (if needed)  
// during the reposition, then after locating the frame coresponding to the time-code, switch the
// transportMode/transportState to play-pause. The client app should follow these proceedures.
//
////////////////////////////////////////////////////////////////////////////////////////////////
typedef IOReturn (*VirtualTapeTimeCodeRepositionHandler)(void *pRefCon, UInt32 newTimeCode);

////////////////////////////////////////////////////////////////////////////////////////////////
//
// CMP notification callback
//
////////////////////////////////////////////////////////////////////////////////////////////////
typedef void (*VirtualTapeCMPConnectionHandler)(void *pRefCon, bool isInputPlug, UInt8 isochChannel, UInt8 isochSpeed, UInt8 p2pCount);

//////////////////////////////////////////////////////////////////////////////
// Helper functions for object creation/distruction with dedicated thread
//////////////////////////////////////////////////////////////////////////////

// Create and setup a VirtualTapeSubunit object and it's dedicated thread
IOReturn CreateVirtualTapeSubunit(VirtualTapeSubunit **ppTapeSubunit,
								  UInt8 initialSignalMode = kAVCTapeSigModeSD525_60,
								  TapeMediumInfo initialMediumInfo = kDVStandardTape,
								  VirtualTapeCMPConnectionHandler cmpConnectionHandler = nil,
								  VirtualTapeTransportStateChangeHandler transportStateChangeHandler = nil, 
								  VirtualTapeSignalModeChangeHandler signalModeChangeHandler = nil,
								  VirtualTapeTimeCodeRepositionHandler timeCodeRepositionHandler = nil,
								  void *pCallbackRefCon = nil,
								  AVCDevice *pAVCDevice = nil);

// Destroy a VirtualTapeSubunit object created with CreateVirtualTapeSubunit(), and it's dedicated thread
IOReturn DestroyVirtualTapeSubunit(VirtualTapeSubunit *pTapeSubunit);

/////////////////////////////////////////
//
// VirtualTapeSubunit Class definition
//
/////////////////////////////////////////

class VirtualTapeSubunit
{

public:
	// Constructor
	VirtualTapeSubunit(UInt8 initialSignalMode = kAVCTapeSigModeSD525_60,
					   TapeMediumInfo initialMediumInfo = kDVStandardTape,
					   VirtualTapeCMPConnectionHandler cmpConnectionHandler = nil,
					   VirtualTapeTransportStateChangeHandler transportStateChangeHandler = nil, 
					   VirtualTapeSignalModeChangeHandler signalModeChangeHandler = nil,
					   VirtualTapeTimeCodeRepositionHandler timeCodeRepositionHandler = nil,
					   void *pCallbackRefCon = nil);
	
	// Destructor
	~VirtualTapeSubunit();
	
	// Setup funcion that finds the first available local node
    IOReturn setupVirtualTapeSubunit(void);
 
	// Setup function that finds the local node for the bus an AVCDevice is connected to
	// Note: the client must have the device open for this to succeed!
	IOReturn setupVirtualTapeSubunitWithAVCDevice(AVCDevice *pAVCDevice);

	// Setup function that takes a client supplied local node service object
	// TODO!
		
	// Function for client to get subUnitTypeAndID
	UInt8 getSubunitTypeAndID(void);
	
	// Function for client to get current medium info
	TapeMediumInfo getMediumInfo(void);

	// Function for client to get current transport state
	void getTransportState(UInt8 *pCurrentTransportMode, UInt8 *pCurrentTransportState, bool *pIsStable);

	// Function for client to get current signal mode
	UInt8 getSignalMode(void);
	
	// Function for client to get current timecode frame number
	UInt32 getTimeCodeFrameCount(void);

	// Function for client to get current timecode, converted to hours, minutes, seconds, and frames
	void getTimeCodeFrameCountInHMSF(UInt32 *pHours, UInt32 *pMinutes, UInt32 *pSeconds, UInt32 *pFrames);
										
	// Function for client to get current timecode frame rate
	TapeTimeCodeFrameRate getTimeCodeFrameRate(void);	
	
	// Function for client to get current PCR info
	void getPlugParameters(bool isInputPlug, UInt8 *pIsochChannel, UInt8 *pIsochSpeed, UInt8 *pP2PCount);

	// Function for client to change medium info
	IOReturn setMediumInfo(TapeMediumInfo newMediumInfo);
	
	// Function for client to change transport state
	IOReturn setTransportState(UInt8 newTransportMode, UInt8 newTransportState, bool isStable);
	
	// Function for client to change signal mode
	IOReturn setSignalMode(UInt8 newSignalMode);
	
	// Function for client to change timecode frame number
	IOReturn setTimeCodeFrameCount(UInt32 newTimeCodeFrameCount);
	
	// Function for client to change the timecode frame rate
	IOReturn setTimeCodeFrameRate(TapeTimeCodeFrameRate newTimeCodeFrameRate);	

	// Function for client to change PCR (when no p2p connection exists)
	IOReturn setPlugParameters(bool isInputPlug, UInt8 isochChannel, UInt8 isochSpeed);
	
	// Function for client to get a nub interface for creating an associated stream object
	IOFireWireLibNubRef GetNub(void);
	
	//////////////////////////////////////////////////////////////////
	//
	// These four functions shouldn't be called by clients!
	//
	//////////////////////////////////////////////////////////////////
	void inputPlugReconnectTimeout(void);
	void outputPlugReconnectTimeout(void);
	IOReturn AVCCommandHandlerCallback(UInt32 generation,
									   UInt16 srcNodeID,
									   IOFWSpeed speed,
									   const UInt8 * command,
									   UInt32 cmdLen);
	IOReturn AVCSubUnitPlugHandlerCallback(UInt32 subunitTypeAndID,
										   IOFWAVCPlugTypes plugType,
										   UInt32 plugID,
										   IOFWAVCSubunitPlugMessages plugMessage,
										   UInt32 messageParams);
	
	// A reference to the current run loop for callbacks
	CFRunLoopRef runLoopRef;	
	
private:
		void setPlugSignalFormatWithMode(void);
		UInt32 getSignalModePayloadSize(void);
		IOReturn completeVirtualTapeSubunitSetup(void);

		IOReturn setNewInputPlugValue(UInt32 newVal);
		IOReturn setNewOutputPlugValue(UInt32 newVal);

		IOReturn AVCTransportStateChangeRequested(UInt8 newTransportMode, UInt8 newTransportState);
		
		void startInputPlugReconnectTimer( void );
		void startOutputPlugReconnectTimer( void );

		void stopInputPlugReconnectTimer( void );
		void stopOutputPlugReconnectTimer( void );

		IOFireWireAVCLibProtocolInterface **nodeAVCProtocolInterface;
		AVCDevice *pAVCDeviceForBusIdentification;
		IOFireWireLibNubRef nodeNubInterface;
		
		CFRunLoopTimerRef inputPlugReconnectTimer;
		CFRunLoopTimerRef outputPlugReconnectTimer;

		UInt32 isochInPlugNum;
		UInt32 isochOutPlugNum;
		UInt32 isochInPlugVal;
		UInt32 isochOutPlugVal;
	
		unsigned int timeCodeFrameCount;
		TapeTimeCodeFrameRate timeCodeFrameRate;

		UInt32 subUnitTypeAndID;
		TapeMediumInfo mediumInfo;
		UInt8 signalMode;		// For Tape subunit Input/Output Mode commands
		UInt32 signalFormat;	// For Input/Output plug signal format commands
		UInt8 transportMode;
		UInt8 transportState;
		bool transportIsStable;
		pthread_mutex_t transportControlMutex;
		pthread_mutex_t signalModeControlMutex;
		
		VirtualTapeCMPConnectionHandler clientCMPConnectionHandler;
		VirtualTapeTransportStateChangeHandler clientTransportStateChangeHandler; 
		VirtualTapeSignalModeChangeHandler clientSignalModeChangeHandler;
		VirtualTapeTimeCodeRepositionHandler clientTimeCodeRepositionHandler;
		void *pClientCallbackRefCon;
};
	
} // namespace AVS

#endif // __AVCVIDEOSERVICES_VIRTUALTAPESUBUNIT__




	
