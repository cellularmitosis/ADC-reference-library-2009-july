/*
	File:		AVCVideoServices.h

    Synopsis: This is the top level header file for the AVCVideoServices framework. This
    is the only header that needs to be included to use the clases and functions in
    the framework.
 
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

#ifndef __AVCVIDEOSERVICES_AVCVIDEOSERVICES__
#define __AVCVIDEOSERVICES_AVCVIDEOSERVICES__

// Include Required Standard Headers
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>
#include <mach/mach.h>
#include <mach/vm_map.h>
#include <mach/mach_time.h>

// Include required STL Headers
#include <deque>

// Include Required IOKit Headers
#include <IOKit/IOKitLib.h>
#include <IOKit/IOMessage.h>
#include <CoreFoundation/CoreFoundation.h>
#include <CoreServices/CoreServices.h>
#include <IOKit/firewire/IOFireWireLib.h>
#include <IOKit/firewire/IOFireWireLibIsoch.h>
#include <IOKit/firewire/IOFireWireFamilyCommon.h>
#include <IOKit/avc/IOFireWireAVCLib.h>
#include <IOKit/avc/IOFireWireAVCConsts.h>

namespace AVS
{

	
// The missing defines for Jaguar systems
#ifndef	KIOFWMessagePowerStateChanged
#define KIOFWMessagePowerStateChanged (UInt32)iokit_fw_err(2001)
#endif
	
#ifndef kAVCPlugSignalFormatMPEGTS
#define kAVCPlugSignalFormatMPEGTS 0xa0000000
#endif
	
#ifndef kAVCPlugSignalFormatNTSCDV
#define kAVCPlugSignalFormatNTSCDV 0x80000000
#endif
	
#ifndef kAVCPlugInfoOpcode
#define kAVCPlugInfoOpcode 0x02
#endif
	
/////////////////////////////////////////////
// Build Time Conditionals -
// Comment out any of these you don't want
/////////////////////////////////////////////
#define kAVS_Enable_ForceStop_Handler 1
#define kAVS_Bypass_AVCDevice_Rediscovery 1
#define kAVS_Delay_AVCDevice_Open_And_Close 1
#define kAVS_Enable_VirtualTape_AVCResponse_Delays 1
//#define DVTransmitter_DCL_Callback_Timing 1
	
// The DV Modes
enum
{
	kDVModeSDL_625_50 = 0x84,
	kDVModeSDL_525_60 = 0x04,
	kDVModeSD_625_50 = 0x80,
	kDVModeSD_525_60 = 0x00,
	kDVModeDVCPro25_625_50 = 0xF8,
	kDVModeDVCPro25_525_60 = 0x78,
	kDVModeDVCPro50_625_50 = 0xF4,
	kDVModeDVCPro50_525_60 = 0x74,
	kDVModeHD_1250_50 = 0x88,
	kDVModeHD_1125_60 = 0x08,
	kDVModeDVCPro100_50 = 0xF0,
	kDVModeDVCPro100_60 = 0x70
};

// The DV transfer speed
// Note: These can be logically-or'ed with the DV Mode
enum
{
	kDVTransferSpeedRealTime = 0x00,
	kDVTransferSpeed_1x = 0x00,
	kDVTransferSpeed_2x = 0x01,
	kDVTransferSpeed_4x = 0x02
};

struct DVFormats
{
	// Frame size, in bytes
	UInt32 frameSize;

	// DV mode (8-bit value)
	UInt8 mode;

	// Data block size, in quadlets
	UInt8 dbs;

	// Represents the number of data-blocks in a source packet
	// A 2-bit value
	// 0 = 1 datablock, 1 = 2 datablocks, 2 = 4 datablocks, 3 = 8 datablocks
	UInt8 fn;
	
	// The starting SYT offset for transmitting this particular mode of DV
	UInt32 startingSYTOffset;
};

typedef unsigned int XmitCycleMode;
enum
{
    CycleModeFull,
    CycleModeCIPOnly
};

// A define for any available isoch channel.
// Specify this value as the DV/MPEG2 receiver
// or transmitter channel num, to allow the FireWire
// family's isoch code determine and allocate an available
// channel. An application can then determine which
// channel was allocated in the message callback
// for the AllocateChannel message.
// Note: This value should only be used if the
// DV/MPEG Transmitter or DV/MPEG Recevier are created
// with the doIRMAllocations flag set to true.
#define kAnyAvailableIsochChannel 0xFFFFFFFF

// Macro to extract the DV standard from the DV mode
// Same thing works for MPEG devices (HDV, DVHS)
// A 1-bit value, 0 = NTSC, 1 = PAL
#define DVstandard(a) (((a) & 0x80) >> 7)
#define MPEGstandard(a) (((a) & 0x80) >> 7)

#define kVideoStandard60 0
#define kVideoStandard50 1
#define kVideoStandardNTSC 0
#define kVideoStandardPAL 1

// Macro to extract the STYPE from the DV mode
// A 5-bit value
#define DVstype(a) (((a) & 0x7C) >> 2)

// Macro to extract the DV speed from the DV mode
// A 2-bit value, 0 = 1x, 1 = 2x, 2 = 4x
#define DVspeed(a) ((a) & 0x03)

// NTSC DV Timing Constants
// ------------------------
// FramesPerSec = ((1000/1001)*30) = 29.97002997
// BitsPerSec = (FramesPerSec*(120000*8)) = 28771228.7712288
// BitsPerSourcePacket = (480*8) = 3840
// FWClocksPerSec = 24576000.0
// FWClocksPerSourcePacket = FWClocksPerSec/(BitsPerSec/3840) = 3280.0768
#define kNTSCFramesPerSecond 29.97002997
#define kIsochCycleClocksPerNTSCSourcePacket 3280.0768

// PAL DV Timing Constants
// -----------------------
// FramesPerSec = 25.000000
// BitsPerSec = (FramesPerSec*(144000*8)) = 28800000.000
// BitsPerSourcePacket = (480*8) = 3840
// FWClocksPerSec = 24576000.0
// FWClocksPerSourcePacket = FWClocksPerSec/(BitsPerSec/3840) = 3276.8000
#define kPALFramesPerSecond 25.000000
#define kIsochCycleClocksPerPALSourcePacket 3276.8000

// Misc DV Constants
enum
{
	kDVStandardNTSC = 0,
	kDVStandardPAL = 1,
	kDVXmitCIPOnlySize = 8
};

// Sizes of MPEG2 stuff
enum
{
	kMPEG2XmitCIPOnlySize = 8,
	kMPEG2TSPacketSize = 188,
	kMPEG2TSPacketSizeInWords = 47,
	kMPEG2SourcePacketSize = 192,
	kMPEG2DataBlocksPerPacket = 8
};

// Some Max data rate constants for different number of packets per cycle
enum
{
	kMaxDataRate_EighthTSPacketPerCycle 	= 1504000,
	kMaxDataRate_QuarterTSPacketPerCycle 	= 3008000,
	kMaxDataRate_HalfTSPacketPerCycle 		= 6016000,
	kMaxDataRate_OneTSPacketPerCycle 		= 12032000,
	kMaxDataRate_TwoTSPackestPerCycle 		= 24064000,
	kMaxDataRate_ThreeTSPacketsPerCycle 	= 36096000,
	kMaxDataRate_FourTSPacketsPerCycle 		= 48128000,
	kMaxDataRate_FiveTSPacketsPerCycle 		= 60160000
};

// Callback for client notification of messages for a device
typedef IOReturn (*AVCDeviceMessageNotification) (class AVCDevice *pAVCDevice,
												  natural_t messageType,
												  void * messageArgument,
												  void *pRefCon);

} // namespace AVS

// Include Local Headers
#include "AVSCommon.h"
#include "StringLogger.h"
#include "TSPacket.h"
#include "PSITables.h"
#include "MPEG2XmitCycle.h"
#include "MPEG2Transmitter.h"
#include "MPEG2Receiver.h"
#include "TSDemuxer.h"
#include "DVXmitCycle.h"
#include "DVTransmitter.h"
#include "DVReceiver.h"
#include "FireWireDV.h"
#include "FireWireMPEG.h"
#include "AVCDevice.h"
#include "AVCDeviceController.h"
#include "AVCDeviceCommandInterface.h"
#include "TapeSubunitController.h"
#include "PanelSubunitController.h"
#include "MPEGTrickModes.h"
#include "VirtualTapeSubunit.h"
#include "VirtualMPEGTapePlayerRecorder.h"
#include "VirtualMusicSubunit.h"

namespace AVS
{
	// Create and setup an AVCDeviceController object and it's dedicated thread
	IOReturn CreateAVCDeviceController(AVCDeviceController **ppAVCDeviceController,
									   AVCDeviceControllerNotification clientNotificationProc = nil,
									   void *pRefCon = nil,
									   AVCDeviceMessageNotification globalAVCDeviceMessageProc = nil);

	// Destroy a AVCDeviceController object created with CreateAVCDeviceController(), and it's dedicated thread
	IOReturn DestroyAVCDeviceController(AVCDeviceController *pAVCDeviceController);

} // namespace AVS

#endif // #define __AVCVIDEOSERVICES_AVCVIDEOSERVICES__
