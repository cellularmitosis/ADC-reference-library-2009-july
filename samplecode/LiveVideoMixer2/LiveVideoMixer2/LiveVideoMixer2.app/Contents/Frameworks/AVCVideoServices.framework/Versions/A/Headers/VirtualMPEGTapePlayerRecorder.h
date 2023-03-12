/*
	File:		VirtualMPEGTapePlayerRecorder.h
 
 Synopsis: This is the header file for the VirtualMPEGTapePlayerRecorder class.
 
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

#ifndef __AVCVIDEOSERVICES_VIRTUALMPEGTAPEPLAYERRECORDER__
#define __AVCVIDEOSERVICES_VIRTUALMPEGTAPEPLAYERRECORDER__

namespace AVS
{
	
class VirtualMPEGTapePlayerRecorder
{
	
public:
	VirtualMPEGTapePlayerRecorder();
	~VirtualMPEGTapePlayerRecorder();
	
	// This object must be initialized before it does anything useful
	IOReturn initWithFileName(char* pMpegTSFileName = nil, AVCDevice *pAVCDevice = nil);
	
	// This function enables the client to enable or disable looping playback mode
	void setLoopModeState(bool enableLoopMode);
	
	// This function allows the client to rewind to the beginning of the file
	void  restartTransmit(void);
	
	// This function allows the client to request the player move to a specific frame in the playback file
	IOReturn repositionPlayer(UInt32 targetFrame);
	
	// Allows the client to know if the requested resposition has taken place
	bool isRepositionPending(void);
	
	// This function enables the client to get access to the virtual tape 
	// subunit's current time-code frame-count
	UInt32 getTapeSubunitTimeCodeFrameCount();
	
	// This function enables the client to get access to the virtual tape 
	// subunit's current time-code in hours, minutes, seconds, and frames
	void getTapeSubunitTimeCodeFrameCountInHMSF(UInt32 *pHours, UInt32 *pMinutes, UInt32 *pSeconds, UInt32 *pFrames);

	// Get the current transport state
	void getTransportState(UInt8 *pCurrentTransportMode, UInt8 *pCurrentTransportState, bool *pIsStable);
	
	// Set the directory for files created when transitioning into record mode
	void setRecordFileDirectoryPath(char *pDataPath);
	
	// Functions to get/set the playback filename
	IOReturn setPlaybackFileName(char* pMpegTSFileName);
	char* getPlaybackFileName(void);
	
	// Function to attempt a transport state change
	IOReturn tapeTransportStateChange(UInt8 newTransportMode, UInt8 newTransportState);
	
	// Function for client to set/clear write-protect mode
	void setRecordInhibit(bool isWriteProtected);
	bool isRecordInhibited(void);
	
	// Function to determine if we've identified a navi file for the 
	// currently specified transport stream file
	bool isNaviFileEnabled(void);

	// Get the number of overruns since the stream last started
	UInt32 getOverrunCount(void);
	
	// Functions to change the broadcast channe for the transmitter and receiver
	void  setTransmitterBroadcastIsochChannel(UInt32 isochChannel);
	void  setReceiverBroadcastIsochChannel(UInt32 isochChannel);
	
	// Function to get info about the record or playback stream
	void getStreamInformation(UInt32 *pFrameHorizontalSize,
							  UInt32 *pFrameVerticalSize,
							  UInt32 *pBitRate,
							  MPEGFrameRate *pFrameRate,
							  UInt32 *pNumFrames,
							  UInt32 *pNumTSPackets,
							  double *pCurrentMPEGDataRate);
	
	// Function to get info about the plugs
	void getPlugInformation(UInt32 *pInputPlugConnectionCount,
							UInt32 *pInputPlugChannel,
							UInt32 *pOutputPlugConnectionCount,
							UInt32 *pOutputPlugChannel,
							UInt32 *pOutputPlugSpeed);
			
private:
		
	// Callbacks from tape subunit object
	static IOReturn MyVirtualTapeTransportStateChangeHandler(void *pRefCon, UInt8 newTransportMode, UInt8 newTransportState); 
	static IOReturn MyVirtualTapeSignalModeChangeHandler(void *pRefCon, UInt8 newSignalMode);
	static IOReturn MyVirtualTapeTimeCodeRepositionHandler(void *pRefCon, UInt32 newTimeCode);
	static void MyVirtualTapeCMPConnectionHandler(void *pRefCon, bool isInputPlug, UInt8 isochChannel, UInt8 isochSpeed, UInt8 p2pCount);
	
	// Callbacks for mpeg transmitter object
	static void MPEG2TransmitterMessageProc(UInt32 msg, UInt32 param1, UInt32 param2, void *pRefCon);
	static IOReturn MpegTransmitCallback(UInt32 **ppBuf, bool *pDiscontinuityFlag, void *pRefCon);

	// Callbacks for mpeg Receiver object
	static IOReturn MpegReceiveCallback(UInt32 tsPacketCount, 
										UInt32 **ppBuf, 
										void *pRefCon,
										UInt32 isochHeader,
										UInt32 cipHeader0,
										UInt32 cipHeader1,
										UInt32 fireWireTimeStamp);
	
	static void MPEG2ReceiverMessageProc(UInt32 msg, UInt32 param1, UInt32 param2, void *pRefCon);
	
	TapeTimeCodeFrameRate convertMPEGFrameRateToTapeSubunitFrameRate(UInt32 mpegFrameRate);
	
	MPEGNaviFileWriter *pWriter;
	MPEGNaviFileReader *pReader;
	MPEG2Transmitter *pTransmitter;
	MPEG2Receiver *pReceiver;
	VirtualTapeSubunit *pTapeSubunit;
	bool broadcastStreamingMode;
	bool loopMode;
	bool flushMode;
	unsigned int flushCnt;
	char *pFileName;
	char *pRecordingPath;
	UInt32 currentFrameOffset;
	MPEGFrameRate currentFrameRate;
	UInt32 currentTSFileLengthInFrames;
	char tsPacketBuf[kMPEG2TSPacketSize];
	pthread_mutex_t transportControlMutex;
	bool repositionRequested;
	UInt32 repositionFrame;
	bool writeProtectEnabled;
	bool readerHasNaviFile;
	int overrunCount;
	UInt32 transmitterBroadcastIsochChannel;
	UInt32 receiverBroadcastIsochChannel;
	bool firstTSPacket;
	
};

} // namespace AVS

#endif // __AVCVIDEOSERVICES_VIRTUALMPEGTAPEPLAYERRECORDER__

