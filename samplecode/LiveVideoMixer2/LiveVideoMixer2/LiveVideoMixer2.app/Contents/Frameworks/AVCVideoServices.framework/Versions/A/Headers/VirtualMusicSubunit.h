/*
	File:		VirtualMusicSubunit.h
 
 Synopsis: This is the header file for the VirtualMusicSubunit class.
 
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

#ifndef __AVCVIDEOSERVICES_VIRTUALMUSICSUBUNIT__
#define __AVCVIDEOSERVICES_VIRTUALMUSICSUBUNIT__

namespace AVS
{

class VirtualMusicSubunit;

// Uncommenting the following line will have this object create an
// audio subunit to go along with the music subunit only for the purpose of 
// driver matching with the Mac OS-X FireWireAudio driver.
#define kEnableAudioSubunitForDriverMatching 1	
	
// The spec says all isoch p2p connections must be reconnected
// within one second of each bus-reset. We'll wait 1.5 to be consertive
#define kMusicSubunitCMPBusResetReconnectTime 1.5

// The default number of audio in and audio out signals
#define kDefaultNumAudioSignals 6	
	
typedef unsigned int MusicSubunitSampleRate;
enum
{
	kMusicSubunitSampleRate_32000 = 0x00,
	kMusicSubunitSampleRate_44100 = 0x01,
	kMusicSubunitSampleRate_48000 = 0x02,
	kMusicSubunitSampleRate_88200 = 0x03,
	kMusicSubunitSampleRate_96000 = 0x04,
	kMusicSubunitSampleRate_176400 = 0x05,
	kMusicSubunitSampleRate_192000 = 0x06
};		

typedef unsigned int MusicSubunitInfoBlockTypes;
enum
{
	kRoutingStatusInfoBlock		= 0x8108,
	kSubunitPlugInfoBlock		= 0x8109,
	kClusterInfoBlockType		= 0x810A,
	kMusicPlugInfoBlockType		= 0x810B
};

typedef unsigned int MusicSubunitPlugUsages;
enum 
{
	kIsochStreamSubunitPlug		= 0x00,
	kAsynchStreamSubunitPlug	= 0x01,
	kMidiSubunitPlug			= 0x02,
	kSyncSubunitPlug			= 0x03,
	kAnalogSubunitPlug			= 0x04,
	kDigitalSubunitPlug			= 0x05
};

typedef unsigned int MusicClusterFormats;
enum 
{
	kStreamFormatIEC60958_3			= 0x00,
	kStreamFormatIEC61937_3			= 0x01,
	kStreamFormatIEC61937_4			= 0x02,
	kStreamFormatIEC61937_5			= 0x03,
	kStreamFormatIEC61937_6			= 0x04,
	kStreamFormatIEC61937_7			= 0x05,
	kStreamFormatMBLA				= 0x06,
	kStreamFormatDVDAudio			= 0x07,
	kStreamFormatHiPrecisionMBLA	= 0x0C,
	kStreamFormatMidiConf			= 0x0D
};

typedef unsigned int MusicPortTypes;
enum 
{
	kMusicPortTypeSpeaker 		= 0x00,
	kMusicPortTypeHeadPhone 	= 0x01,
	kMusicPortTypeMicrophone 	= 0x02,
	kMusicPortTypeLine 			= 0x03,
	kMusicPortTypeSpdif 		= 0x04,
	kMusicPortTypeAdat 			= 0x05,
	kMusicPortTypeTdif			= 0x06,
	kMusicPortTypeMadi 			= 0x07,
	kMusicPortTypeAnalog 		= 0x08,
	kMusicPortTypeDigital 		= 0x09,
	kMusicPortTypeMidi 			= 0x0A,
	kMusicPortTypeAesEbu 		= 0x0B,
	kMusicPortTypeNoType 		= 0xFF,
};

typedef unsigned int MusicPlugLocation;
enum 
{
	kMusicPlugLocationLeftFront 		= 0x01,
	kMusicPlugLocationRightFront 		= 0x02,
	kMusicPlugLocationCenterFront		= 0x03,
	kMusicPlugLocationLowFreqEnhance	= 0x04,
	kMusicPlugLocationLeftSurround		= 0x05,
	kMusicPlugLocationRightSurround		= 0x06,
	kMusicPlugLocationLeftOfCenter		= 0x07,
	kMusicPlugLocationRightOfCenter		= 0x08,
	kMusicPlugLocationSurround			= 0x09,
	kMusicPlugLocationSideLeft			= 0x0A,
	kMusicPlugLocationSideRight			= 0x0B,
	kMusicPlugLocationTop				= 0x0C,
	kMusicPlugLocationBottom			= 0x0D,
	kMusicPlugLocationLeftFrontEffect	= 0x0E,
	kMusicPlugLocationRightFrontEffect	= 0x0F,
	kMusicPlugLocationUnknown			= 0xFF
};

typedef unsigned int MusicPlugTypes;
enum 
{
	kMusicPlugTypeAudio 		= 0x00,
	kMusicPlugTypeMidi 			= 0x01, 
	kMusicPlugTypeSmpte 		= 0x02,
	kMusicPlugTypeSampleCount	= 0x03,
	kMusicPlugTypeSync 			= 0x80
};

typedef unsigned int MusicPlugRoutingSupport;
enum 
{
	kMusicPlugRoutingFixed 		= 0x00,
	kMusicPlugRoutingCluster 	= 0x01,
	kMusicPlugRoutingFlexible	= 0x02,
	kMusicPlugRoutingUnknown	= 0xFF
};

//
// MusicPlug - Class for describing a music plug
//
class MusicPlug
{
public:
	char *pName;
	unsigned short musicPlugID;
	MusicPlugTypes type;
	MusicPlugRoutingSupport routingSupport;
	
	unsigned char sourcePlugID; // The ID of a subunit Dest plug
	unsigned char sourceStreamPosition;
	unsigned char sourceStreamLocation;
	
	unsigned char destPlugID; // The ID of a subunit Source plug
	unsigned char destStreamPosition;
	unsigned char destStreamLocation;
};

// MusicPlugCluster - Class for describing a music plug cluster
class MusicPlugCluster
{
public:
	char *pName;
	MusicClusterFormats format;
	MusicPortTypes portType;
	unsigned char numPlugs;
	class MusicPlug **ppMusicPlugs;
};

// MusicSubunitPlug - Class for describing a music subunit source or dest plug
class MusicSubunitPlug
{
public:
	char *pName;
	unsigned char subunitPlugID;
	unsigned short fdf_fmt;
	MusicSubunitPlugUsages plugUsage;
	unsigned short numClusters;
	class MusicPlugCluster **ppMusicPlugClusters;
};

// MusicSubinitRoutingStatus - Class for describing the routing satus info block
// for the music subunit status descriptor
class MusicSubinitRoutingStatus
{
public:
	unsigned char numSubunitDestPlugs;
	unsigned char numSubunitSourcePlugs;
	unsigned short numMusicPlugs;
	class MusicSubunitPlug **ppSubunitDestPlugs;
	class MusicSubunitPlug **ppSubunitSourcePlugs;
	class MusicPlug **ppMusicPlugs;
};

/////////////////////////////////////////////////////////
// Callback Prototypes for this class object's clients
/////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////
//
// CMP notification callback
//
////////////////////////////////////////////////////////////////////////////////////////////////
typedef void (*VirtualMusicCMPConnectionHandler)(void *pRefCon, bool isInputPlug, UInt8 isochChannel, UInt8 isochSpeed, UInt8 p2pCount);

////////////////////////////////////////////////////////////////////////////////////////////////
//
// Sample rate change requested via input/output signal format command.
// If the client returns kIOReturnSuccess, the signal format command is ACCEPTED,
// and the client should change the sample rate. Otherwise, the signal
// format command control command will be rejected.
//
////////////////////////////////////////////////////////////////////////////////////////////////
typedef IOReturn (*VirtualMusicSampleRateChangeHandler)(void *pRefCon, MusicSubunitSampleRate newSampleRate);

//////////////////////////////////////////////////////////////////////////////
// Helper functions for object creation/distruction with dedicated thread
//////////////////////////////////////////////////////////////////////////////

// Create and setup a VirtualMusicSubunit object and it's dedicated thread
IOReturn CreateVirtualMusicSubunit(VirtualMusicSubunit **ppMusicSubunit,
								   MusicSubunitSampleRate initialSampleRate = kMusicSubunitSampleRate_48000,
								   VirtualMusicCMPConnectionHandler cmpConnectionHandler = nil,
								   VirtualMusicSampleRateChangeHandler sampleRateChangeHandler = nil,
								   void *pCallbackRefCon = nil,
								   AVCDevice *pAVCDevice = nil,
								   UInt32 numAudioInputSignals = kDefaultNumAudioSignals,
								   UInt32 numAudioOutputSignals = kDefaultNumAudioSignals);

// Destroy a VirtualMusicSubunit object created with CreateVirtualMusicSubunit(), and it's dedicated thread
IOReturn DestroyVirtualMusicSubunit(VirtualMusicSubunit *pMusicSubunit);

/////////////////////////////////////////
//
// VirtualMusicSubunit Class definition
//
/////////////////////////////////////////

class VirtualMusicSubunit
{
	
public:
	// Constructor
	VirtualMusicSubunit(MusicSubunitSampleRate initialSampleRate = kMusicSubunitSampleRate_48000,
						VirtualMusicCMPConnectionHandler cmpConnectionHandler = nil,
						VirtualMusicSampleRateChangeHandler sampleRateChangeHandler = nil,
						void *pCallbackRefCon = nil,
						UInt32 numAudioInputSignals = kDefaultNumAudioSignals,
						UInt32 numAudioOutputSignals = kDefaultNumAudioSignals);
	
	// Destructor
	~VirtualMusicSubunit();
	
	// Setup funcion that finds the first available local node
    IOReturn setupVirtualMusicSubunit(void);
	
	// Setup function that finds the local node for the bus an AVCDevice is connected to
	// Note: the client must have the device open for this to succeed!
	IOReturn setupVirtualMusicSubunitWithAVCDevice(AVCDevice *pAVCDevice);
	
	// Setup function that takes a client supplied local node service object
	// TODO!
	
	// Function for client to get subUnitTypeAndID
	UInt8 getSubunitTypeAndID(void);
	
	// Function for client to get current PCR info
	void getPlugParameters(bool isInputPlug, UInt8 *pIsochChannel, UInt8 *pIsochSpeed, UInt8 *pP2PCount);
	
	// Function for client to change PCR (when no p2p connection exists)
	IOReturn setPlugParameters(bool isInputPlug, UInt8 isochChannel, UInt8 isochSpeed);
	
	// Function for client to get a nub interface for creating an associated stream object
	IOFireWireLibNubRef GetNub(void);
	
	// Function to get the current sample rate of the virtual music device
	MusicSubunitSampleRate GetCurrentSubunitSampleRate(void);
	
	// Function to manually change the sample rate
	void SetSubunitSampleRate(MusicSubunitSampleRate newSampleRate);
	
	//////////////////////////////////////////////////////////////////
	//
	// These three functions shouldn't be called by clients!
	//
	//////////////////////////////////////////////////////////////////
	void inputPlugReconnectTimeout(void);
	void outputPlugReconnectTimeout(void);
	IOReturn AVCSubUnitPlugHandlerCallback(UInt32 subunitTypeAndID,
										   IOFWAVCPlugTypes plugType,
										   UInt32 plugID,
										   IOFWAVCSubunitPlugMessages plugMessage,
										   UInt32 messageParams);
	
	// A reference to the current run loop for callbacks
	CFRunLoopRef runLoopRef;	
	
private:
	IOReturn completeVirtualMusicSubunitSetup(void);
	
	void setPlugSignalFormatWithSampleRate(void);
	
	IOReturn setNewInputPlugValue(UInt32 newVal);
	IOReturn setNewOutputPlugValue(UInt32 newVal);
	
	void startInputPlugReconnectTimer( void );
	void startOutputPlugReconnectTimer( void );
	
	void stopInputPlugReconnectTimer( void );
	void stopOutputPlugReconnectTimer( void );

	void SetByteBuf(unsigned char *pByteBuf);
	void SetNextByte(unsigned char byteVal);
	void SetNameInfoBlockBytes(char* str, unsigned short len);
	void SetMusicPlugInfoBlockBytes(MusicPlug *pMusicPlug);
	void SetClusterInfoBlockBytes(MusicPlugCluster *pCluster, bool isInputCluster);
	void SetSubunitPlugInfoBlock(MusicSubunitPlug *pSubunitPlug, bool isSubunitSourcePlug);
	void SetRoutingStatusInfoBlock(void);
	unsigned short SizeOfMusicPlugInfoBlock(MusicPlug *pMusicPlug);
	unsigned short SizeOfMusicPlugClusterInfoBlock(MusicPlugCluster *pCluster);
	unsigned short SizeOfMusicSubunitPlugInfoBlock(MusicSubunitPlug *pSubunitPlug);
	unsigned short SizeOfMusicSubinitRoutingStatusInfoBlock(void);
	
	static IOReturn MusicSubunitCommandHandlerCallback( void *refCon,
										UInt32 generation,
										UInt16 srcNodeID,
										IOFWSpeed speed,
										const UInt8 * command,
										UInt32 cmdLen);

#ifdef kEnableAudioSubunitForDriverMatching
	static IOReturn AudioSubunitCommandHandlerCallback( void *refCon,
														UInt32 generation,
														UInt16 srcNodeID,
														IOFWSpeed speed,
														const UInt8 * command,
														UInt32 cmdLen);
#endif
	
	static IOReturn AVCUnit_PlugInfoHandlerCallback( void *refCon,
											  UInt32 generation,
											  UInt16 srcNodeID,
											  IOFWSpeed speed,
											  const UInt8 * command,
											  UInt32 cmdLen);
	
	static IOReturn AVCUnit_StreamInfoHandlerCallback( void *refCon,
												UInt32 generation,
												UInt16 srcNodeID,
												IOFWSpeed speed,
												const UInt8 * command,
												UInt32 cmdLen);
	
	static IOReturn AVCUnit_InputPlugSignalFormatHandlerCallback( void *refCon,
														   UInt32 generation,
														   UInt16 srcNodeID,
														   IOFWSpeed speed,
														   const UInt8 * command,
														   UInt32 cmdLen);
	
	static IOReturn AVCUnit_OutputPlugSignalFormatHandlerCallback( void *refCon,
															UInt32 generation,
															UInt16 srcNodeID,
															IOFWSpeed speed,
															const UInt8 * command,
															UInt32 cmdLen);
	
	static IOReturn AVCUnit_SignalSourceHandlerCallback( void *refCon,
												  UInt32 generation,
												  UInt16 srcNodeID,
												  IOFWSpeed speed,
												  const UInt8 * command,
												  UInt32 cmdLen);
	
	UInt8 subunitSampleRateToStreamFormatInfoSampleRate(void);
	UInt32 subunitSampleRateToPayloadInQuadlets(void);
	
	void updateMusicSubunitDescriptorWithNewSampleRate(void);
	
	void generateDiscriptorStructs(void);
	
	IOFireWireAVCLibProtocolInterface **nodeAVCProtocolInterface;
	AVCDevice *pAVCDeviceForBusIdentification;
	IOFireWireLibNubRef nodeNubInterface;
	
	CFRunLoopTimerRef inputPlugReconnectTimer;
	CFRunLoopTimerRef outputPlugReconnectTimer;
	
	UInt32 isochInPlugNum;
	UInt32 isochOutPlugNum;
	UInt32 isochInPlugVal;
	UInt32 isochOutPlugVal;
	
	UInt32 subUnitTypeAndID;
	
	UInt32 numAudioInputs;
	UInt32 numAudioOutputs;
	UInt32 numMusicPlugs;
	MusicPlug *pMusicSubunitMusicPlugs;
	
	MusicPlug **ppAudioOutClusterPlugArray;
	MusicPlug **ppMidiOutClusterPlugArray;
	MusicPlug **ppAudioInClusterPlugArray;
	MusicPlug **ppMidiInClusterPlugArray;
	MusicPlug **ppSyncClusterPlugArray;
	
	unsigned short statusDescriptorSize;
	unsigned char *pStatusDescriptor;
	unsigned char *pDescriptorByteBuf;

	MusicSubunitSampleRate sampleRate;
	
	VirtualMusicCMPConnectionHandler clientCMPConnectionHandler;
	VirtualMusicSampleRateChangeHandler clientSampleRateChangeHandler;
	void *pClientCallbackRefCon;
};

} // namespace AVS

#endif // __AVCVIDEOSERVICES_VIRTUALMUSICSUBUNIT__
