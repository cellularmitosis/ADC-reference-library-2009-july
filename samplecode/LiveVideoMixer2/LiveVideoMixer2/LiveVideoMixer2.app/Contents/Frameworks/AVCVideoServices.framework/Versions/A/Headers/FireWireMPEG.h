/*
	File:		FireWireMPEG.h

    Synopsis: This is the top level header file for the FireWireMPEG framework.

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

#ifndef __AVCVIDEOSERVICES_FIREWIREMPEG__
#define __AVCVIDEOSERVICES_FIREWIREMPEG__

namespace AVS
{

//////////////////////////////////////////////////////////////////////////////////
//
// Prototypes for FireWireMPEG object creation/destruction helper functions.
// These functions create/prepare or destroy both a FireWireMPEG class object, 
// as well as a dedicated real-time thread for the object's callbacks and DCL
// processing.
//
//////////////////////////////////////////////////////////////////////////////////

// Create and setup a MPEG2Receiver object and it's dedicated thread
IOReturn CreateMPEG2Receiver(MPEG2Receiver **ppReceiver,
							 DataPushProc dataPushProcHandler,
							 void *pDataPushProcRefCon = nil,
							 MPEG2ReceiverMessageProc messageProcHandler = nil,
							 void *pMessageProcRefCon = nil,
							 StringLogger *stringLogger = nil,
							 IOFireWireLibNubRef nubInterface = nil,
							 unsigned int cyclesPerSegment = kCyclesPerReceiveSegment,
							 unsigned int numSegments = kNumReceiveSegments,
							 bool doIRMAllocations = false);

// Destroy a MPEG2Receiver object created with CreateMPEG2Receiver(), and it's dedicated thread
IOReturn DestroyMPEG2Receiver(MPEG2Receiver *pReceiver);

// Create and setup a MPEG2Transmitter object and it's dedicated thread
IOReturn CreateMPEG2Transmitter(MPEG2Transmitter **ppTransmitter,
								DataPullProc dataPullProcHandler,
								void *pDataPullProcRefCon = nil,
								MPEG2TransmitterMessageProc messageProcHandler = nil,
								void *pMessageProcRefCon = nil,
								StringLogger *stringLogger = nil,
								IOFireWireLibNubRef nubInterface = nil,
								unsigned int cyclesPerSegment = kCyclesPerTransmitSegment,
								unsigned int numSegments = kNumTransmitSegments,
								bool doIRMAllocations = false,
								unsigned int packetsPerCycle = kNumTSPacketsPerCycle,
								unsigned int tsPacketQueueSizeInPackets = kTSPacketQueueSizeInPackets);

// Destroy a MPEG2Transmitter object created with CreateMPEG2Transmitter(), and it's dedicated thread
IOReturn DestroyMPEG2Transmitter(MPEG2Transmitter *pTransmitter);

} // namespace AVS

#endif // __AVCVIDEOSERVICES_FIREWIREMPEG__