/*
	File:		MPEGTrickModes.h
 
 Synopsis: This is the header file for the MPEGTrickModes functions.
 
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

#ifndef __AVCVIDEOSERVICES_MPEGTRICKMODES__
#define __AVCVIDEOSERVICES_MPEGTRICKMODES__

namespace AVS
{

///////////////////////////////////////////////////////////////////////////////////////
//
//  MPEG2-TS navigation control file: A navigation control file is used in conjunction
//  with a transport stream file to assist with random access and time-code generation.
//
//  The version 1 navi file structure consists of a NaviFileStreamInfo struct, 
//  followed by many NaviFileFrameInfo struct, one for each frame in
//  the transport stream file.
//  
//  The class object MPEGNaviFileReader supports file reading with time-code and 
//  repositioning capabilites
//
//  The class object MPEGNaviFileWriter supports generation of a navi file along with
//  the creation of a transport stream file
//
///////////////////////////////////////////////////////////////////////////////////////

#define kNaviFileStructureRevision_1 1

typedef enum
{
	kMPEGFrameType_I,
	kMPEGFrameType_P,
	kMPEGFrameType_B,
	kMPEGFrameType_Unknown,
}MPEGFrameType;

typedef enum
{
	MPEGFrameRate_23_976,
	MPEGFrameRate_24,
	MPEGFrameRate_25,
	MPEGFrameRate_29_97,
	MPEGFrameRate_30,
	MPEGFrameRate_50,
	MPEGFrameRate_59_94,
	MPEGFrameRate_60,
	MPEGFrameRate_Unknown
}MPEGFrameRate;

// The first thing in the navigation file is this structure
typedef struct _NaviFileStreamInfo
{
	UInt32 naviFileStructureRevision;
	UInt32 frameHorizontalSize;
	UInt32 frameVerticalSize;
	UInt32 bitRate;
	MPEGFrameRate frameRate;
}NaviFileStreamInfo;

// After the stream info structure, there is one of these structs
// for each frame in the file
typedef struct _NaviFileFrameInfo
{
	UInt32 frameTSPacketOffset;
	MPEGFrameType frameType;
}NaviFileFrameInfo;


/////////////////////////////////////
//
// MPEGNaviFileReader Class object
//
/////////////////////////////////////
class MPEGNaviFileReader
{

public:
	
	// Constructor/Destructor
	MPEGNaviFileReader();
	~MPEGNaviFileReader();
	
	// Initialization
	IOReturn InitWithTSFile(char *pTSFileName, bool failIfNoNaviFile = true);
	
	// Read TS packets from file. Note: Buffer size must be numTSPackets * 188 bytes
	UInt32 ReadNextTSPackets(void *pBuffer, UInt32 numTSPackets);
	
	// Random Access
	IOReturn SeekForwards(UInt32 seconds);
	IOReturn SeekBackwards(UInt32 seconds);
	IOReturn SeekToSpecificFrame(UInt32 frameOffset);
	IOReturn SeekToBeginning(void);

	// TimeCode
	UInt32 GetCurrentTimeCodePositionInFrames(void);
	
	// Get stream info
	IOReturn GetStreamInfo(UInt32 *pHorizontalResolution, 
						   UInt32 *pVerticalResolution, 
						   MPEGFrameRate *pFrameRate, 
						   UInt32 *pBitRate, 
						   UInt32 *pNumFrames,
						   UInt32 *pNumTSPackets);

	// Get the file length
	UInt32 FileLenInTSPackets(void);
	
	// Determine if the file pointer is at an i-frame boundary
	bool isIFrameBoundary(void);

	// Client can use this to determine if we've opened a navi file for the current TS file
	bool hasNaviFile;
	
private:
		
	FILE *tsFile;	
	FILE *naviFile;
	UInt8 *pNaviBuf;
	char *pNaviFileName;
	UInt64 naviFileSize;
	NaviFileStreamInfo *pStreamInfo;
	NaviFileFrameInfo *pFrameInfo;
	UInt32 frameHorizontalSize;
	UInt32 frameVerticalSize;
	UInt32 bitRate;
	MPEGFrameRate frameRate;
	UInt32 numFrames;
	UInt32 numTSPacketsInFile;
	UInt32 currentOffsetInTSPackets;
	UInt32 currentOffsetInFrames;
};

/////////////////////////////////////
//
// MPEGNaviFileWriter Class object
//
/////////////////////////////////////
class MPEGNaviFileWriter
{
	
public:
	
	// Constructor/Destructor
	MPEGNaviFileWriter();
	~MPEGNaviFileWriter();
	
	// Initialization
	IOReturn InitWithTSFile(char *pTSFileName, bool alsoCreateNaviFile = true);

	// Write TS packets to file, and parse 
	// Entries for navi file will be written as well.
	// Note: Buffer size must be numTSPackets * 188 bytes
	UInt32 WriteNextTSPackets(void *pBuffer, UInt32 numTSPackets);

	// Close Files - Allows for another call to InitWithTSFile(...) without tearing down this object
	IOReturn CloseFiles(void);
	
	// TimeCode
	UInt32 GetCurrentTimeCodePositionInFrames(void);
	
	// Get stream info
	IOReturn GetStreamInfo(UInt32 *pHorizontalResolution, 
						   UInt32 *pVerticalResolution, 
						   MPEGFrameRate *pFrameRate, 
						   UInt32 *pBitRate, 
						   UInt32 *pNumFrames, 
						   UInt32 *pNumTSPackets);
	
	// Get the file length
	UInt32 FileLenInTSPackets(void);
	
private:
		
	static IOReturn PESCallback(TSDemuxerMessage msg, PESPacketBuf* pPESPacket,void *pRefCon);
		
	FILE *tsFile;	
	FILE *naviFile;
	UInt32 frameHorizontalSize;
	UInt32 frameVerticalSize;
	UInt32 bitRate;
	MPEGFrameRate frameRate;
	UInt32 numFrames;
	UInt32 numTSPacketsStored;
	TSDemuxer *pTSDemuxer;
	bool firstIFrameFound;
	bool tsDemuxerHadFileWriteError;

};

/////////////////////////////////////
//
// NaviFileCreator Class object
//
/////////////////////////////////////

// Function prototype for PES packet ready callback.
typedef IOReturn (*NaviFileCreatorProgressCallback) (UInt32 percentageComplete, void *pRefCon);

class NaviFileCreator
{
public:
	NaviFileCreator();
	~NaviFileCreator();
	
	IOReturn CreateMPEGNavigationFileForTSFile(char *pTSFilename);
	
	UInt32 percentageComplete;
	UInt32 horizontalResolution;
	UInt32 verticalResolution;
	MPEGFrameRate frameRate;
	UInt32 iFrames;
	UInt32 pFrames;
	UInt32 bFrames;
	UInt32 bitRate;
	
	// The client of this class can register for notifications each time the percentageComplete
	// increments by one.
	void RegisterProgressNotificationCallback(NaviFileCreatorProgressCallback progressCallback, void *pRefCon);

private:
	static IOReturn PESCallback(TSDemuxerMessage msg, PESPacketBuf* pPESPacket,void *pRefCon);
	
	FILE *pInFile;
	FILE *pOutFile;
	bool firstIFrameFound;
	UInt32 streamTSPacketNumber;
	bool tsDemuxerHadFileWriteError;
	UInt64 tsFileSize;
	UInt32 tsFileSizeInPackets;
	UInt32 previousPercentageComplete;
	NaviFileCreatorProgressCallback clientProgressCallback;
	void *pClientRefCon;
};

///////////////////////////////////////////////////////////////////////////////////////
//
//  MPEGTrickMode_RepositionFilePointerForward: This function can perform a forward 
//  skip in time (specified in seconds) to an i-frame in a transport stream file.
//  without the assistance of a navi file
//
///////////////////////////////////////////////////////////////////////////////////////
IOReturn MPEGTrickMode_RepositionFilePointerForward(FILE *pInFile, UInt32 timeInSeconds, UInt32 *pTotalSkippedFrames);

///////////////////////////////////////////////////////////////////////////////////////
//
// Utility Functions
//
///////////////////////////////////////////////////////////////////////////////////////

// FramesPerSecond: Convert from MPEGFrameRate to a approximate number of frames per second
UInt32 FramesPerSecond(MPEGFrameRate frameRate);

} // namespace AVS

#endif // __AVCVIDEOSERVICES_MPEGTRICKMODES__
