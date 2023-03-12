/*
	File:   TapeSubunitController.h
 
 Synopsis: This is the header for the TapeSubunitController Class 
 
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

#ifndef __AVCVIDEOSERVICES_TAPESUBUNITCONTROLLER__
#define __AVCVIDEOSERVICES_TAPESUBUNITCONTROLLER__

namespace AVS
{

	/* AVC tape recorder/player opcodes. */
	enum {
		kAVCTapeAnalogAudioOutputModeOpcode	= 0x70,
		kAVCTapeAreaModeOpcode				= 0x72,
		kAVCTapeAbsoluteTrackNumberOpcode	= 0x52,
		kAVCTapeAudioModeOpcode				= 0x71,
		kAVCTapeBackwardOpcode				= 0x56,
		kAVCTapeBinaryGroupOpcode			= 0x5a,
		kAVCTapeEditModeOpcode				= 0x40,
		kAVCTapeForwardOpcode				= 0x55,
		kAVCTapeInputSignalModeOpcode		= 0x79,
		kAVCTapeLoadMediumOpcode			= 0xc1,
		kAVCTapeMarkerOpcode				= 0xca,
		kAVCTapeMediumInfoOpcode			= 0xda,
		kAVCTapeOpenMicOpcode				= 0x60,
		kAVCTapeOutputSignalModeOpcode		= 0x78,
		kAVCTapePlayOpcode					= 0xc3,
		kAVCTapePresetOpcode				= 0x45,
		kAVCTapeReadMicOpcode				= 0x61,
		kAVCTapeRecordOpcode				= 0xc2,
		kAVCTapeRecordingDateOpcode			= 0x53,
		kAVCTapeRecordingSpeedOpcode		= 0xdb,
		kAVCTapeRecordingTimeOpcode			= 0x54,
		kAVCTapeRelativeTimeCounterOpcode	= 0x57,
		kAVCTapeSearchModeOpcode			= 0x50,
		kAVCTapeSMPTERecordingTimeOpcode	= 0x5c,
		kAVCTapeSMPTETimeCodeOpcode			= 0x59,
		kAVCTapeTapePlaybackFormatOpcode	= 0xd3,
		kAVCTapeTapeRecordingFormatOpcode	= 0xd2,
		kAVCTapeTimeCodeOpcode				= 0x51,
		kAVCTapeTransportStateOpcode		= 0xd0,
		kAVCTapeWindOpcode					= 0xc4,
		kAVCTapeWriteMicOpcode				= 0x62
	};
	
	/* avc tape playback mode */
	enum{
		kAVCTapePlayNextFrame				= 0x30,	/* next frame or field			*/
		kAVCTapePlaySlowestFwd				= 0x31,	/* slow speed forward			*/
		kAVCTapePlaySlowFwd6				= 0x32,	/* slow forward speed 6			*/
		kAVCTapePlaySlowFwd5				= 0x33,	/* slow forward speed 5			*/
		kAVCTapePlaySlowFwd4				= 0x34,	/* slow forward speed 4			*/
		kAVCTapePlaySlowFwd3				= 0x35,	/* slow forward speed 3			*/
		kAVCTapePlaySlowFwd2				= 0x36,	/* slow forward speed 2			*/
		kAVCTapePlaySlowFwd1				= 0x37,	/* slow forward speed 1			*/
		kAVCTapePlayX1						= 0x38,	/* normal speed					*/
		kAVCTapePlayFastFwd1				= 0x39,	/* fast forward speed 1			*/
		kAVCTapePlayFastFwd2				= 0x3a,	/* fast forward speed 2			*/
		kAVCTapePlayFastFwd3				= 0x3b,	/* fast forward speed 3			*/
		kAVCTapePlayFastFwd4				= 0x3c,	/* fast forward speed 4			*/
		kAVCTapePlayFastFwd5				= 0x3d,	/* fast forward speed 5			*/
		kAVCTapePlayFastFwd6				= 0x3e,	/* fast forward speed 6			*/
		kAVCTapePlayFastestFwd				= 0x3f,	/* previous frame or field		*/
		kAVCTapePlayPrevFrame				= 0x40,	/* previous frame				*/
		kAVCTapePlaySlowestRev				= 0x41,	/* slowest speed reverse		*/
		kAVCTapePlaySlowRev6				= 0x42,	/* slow reverse speed 6			*/
		kAVCTapePlaySlowRev5				= 0x43,	/* slow reverse speed 5			*/
		kAVCTapePlaySlowRev4				= 0x44,	/* slow reverse speed 4			*/
		kAVCTapePlaySlowRev3				= 0x45,	/* slow reverse speed 3			*/
		kAVCTapePlaySlowRev2				= 0x46,	/* slow reverse speed 2			*/
		kAVCTapePlaySlowRev1				= 0x47,	/* slow reverse speed 1			*/
		kAVCTapePlayX1Rev					= 0x48,	/* normal speed in reverse		*/
		kAVCTapePlayFastRev1				= 0x49,	/* fast reverse speed 1			*/
		kAVCTapePlayFastRev2				= 0x4a,	/* fast reverse speed 2			*/
		kAVCTapePlayFastRev3				= 0x4b,	/* fast reverse speed 3			*/
		kAVCTapePlayFastRev4				= 0x4c,	/* fast reverse speed 4			*/
		kAVCTapePlayFastRev5				= 0x4d,	/* fast reverse speed 5			*/
		kAVCTapePlayFastRev6				= 0x4e,	/* fast reverse speed 6			*/
		kAVCTapePlayFastestRev				= 0x4f,	/* fastest speed reverse		*/
		kAVCTapePlayRev						= 0x65,	/* normal speed reverse			*/
		kAVCTapePlayRevPause				= 0x6d,	/* reverse pause				*/
		kAVCTapePlayFwd						= 0x75,	/* normal speed forward			*/
		kAVCTapePlayFwdPause				= 0x7d	/* forward pause				*/
	};
	
	/* avc tape recording modes */
	enum {
		kAVCTapeRecArea23Ins				= 0x31,	/* Replace the specified type	*/
		kAVCTapeRecArea1Ins					= 0x32,	/* of signal with a new signal	*/
		kAVCTapeRecArea123Ins				= 0x33,	/* but leave the other			*/
		kAVCTapeRecArea3Ins					= 0x34,	/* signal(s) on the medium		*/
		kAVCTapeRecArea2Ins					= 0x36,	/* intact						*/
		kAVCTapeRecArea12Ins				= 0x37,
		kAVCTapeRecArea13Ins				= 0x38,
		kAVCTapeRecArea23InsPause			= 0x41,	/* Pause recording signal or	*/
		kAVCTapeRecArea1InsPause			= 0x42,	/* signals on the medium and	*/
		kAVCTapeRecArea123InsPause			= 0x43,	/* establish the recording mode	*/
		kAVCTapeRecArea3InsPause			= 0x44,	/* indicated					*/
		kAVCTapeRecArea2InsPause			= 0x46,
		kAVCTapeRecArea12InsPause			= 0x47,
		kAVCTapeRecArea13InsPause			= 0x48,
		kAVCTapeRecRecord					= 0x75,	/* Overwrite all signal(s) on the medium */
		kAVCTapeRecordRecordPause			= 0x7d	/* Pause while recording all signals */
	};
	
	/* avc tape recording speed */
	enum {
		kAVCTapeRecSpeedSP					= 0x6f,
		kAVCTapeRecSpeedVP					= 0x22,
		kAVCTapeRecSpeedEP					= 0x21,
		kAVCTapeRecSpeedLP					= 0x20,
	};
	
	/* avc tape signal modes */
	enum {
		kAVCTapeSigModeSD525_60				= 0x00,	/* DVCR Std Defn 525/60			*/
		kAVCTapeSigModeSDL525_60			= 0x04,	/* DVCR Std Defn L 525/60		*/
		kAVCTapeSigModeHD1125_60			= 0x08,	/* DVCR High Defn 1125/60		*/
		kAVCTapeSigModeSD625_50				= 0x80,	/* DVCR Std Defn 625/50			*/
		kAVCTapeSigModeSDL625_50			= 0x84,	/* DVCR Std Defn L 625/50		*/
		kAVCTapeSigModeHD1250_50			= 0x88,	/* DVCR High Defn 1250/50		*/
		kAVCTapeSigModeMPEG25Mbps_60		= 0x10,	/* DVCR MPEG 25Mbps/60			*/
		kAVCTapeSigModeHDV1_60				= 0x10,	/* HDV1 /60						*/
		kAVCTapeSigModeMPEG12Mbps_60		= 0x14,	/* DVCR MPEG 12.5Mbps/60		*/
		kAVCTapeSigModeMPEG6Mbps_60			= 0x18,	/* DVCR MPEG 6.25Mbps/60		*/
		kAVCTapeSigModeMPEG25Mbps_50		= 0x90,	/* DVCR MPEG 25Mbps/50			*/
		kAVCTapeSigModeHDV1_50				= 0x90,	/* HDV1 /50						*/
		kAVCTapeSigModeMPEG12Mbps_50		= 0x94,	/* DVCR MPEG 12/5Mbps/50		*/
		kAVCTapeSigModeMPEG6Mbps_50			= 0x98,	/* DVCR MPEG 6.25Mbps/50		*/
		kAVCTapeSigModeDVHS					= 0x01,	/* D-VHS						*/
		kAVCTapeSigModeVHSNTSC				= 0x05,	/* Analog VHS NTSC 525/60		*/
		kAVCTapeSigModeVHSMPAL				= 0x25,	/* Analog VHS M-PAL 525/60		*/
		kAVCTapeSigModeVHSPAL				= 0xa5,	/* Analog VHS PAL 625/50		*/
		kAVCTapeSigModeVHSNPAL				= 0xb5,	/* Analog VHS N-PAL 625/50		*/
		kAVCTapeSigModeVHSSECAM				= 0xc5,	/* Analog VHS SECAM 625/50		*/
		kAVCTapeSigModeVHSMESECAM			= 0xd5,	/* Analog VHS ME-SECAM 625		*/
		kAVCTapeSigModeSVHS525_60			= 0x0d,	/* Analog S-VHS 525/60			*/
		kAVCTapeSigModeSVHS625_50			= 0xed,	/* Analog S-VHS 625/50			*/
		kAVCTapeSigMode8mmNTSC				= 0x06,	/* Analog 8mm NTSC				*/
		kAVCTapeSigMode8mmPAL				= 0x86,	/* Analog 8mm PAL				*/
		kAVCTapeSigModeHi8NTSC				= 0x0e,	/* Analog Hi8 NTSC				*/
		kAVCTapeSigModeHi8PAL				= 0x8e,	/* Analog Hi8 PAL				*/
		kAVCTapeSigModeMicroMV12Mbps_60		= 0x24,	/* MicroMV 12.5Mbps/60			*/
		kAVCTapeSigModeMicroMV6Mbps_60		= 0x28,	/* MicroMV 6.25Mbps/60			*/
		kAVCTapeSigModeMicroMV12Mbps_50		= 0xA4,	/* MicroMV 12.5Mbps/50			*/
		kAVCTapeSigModeMicroMV6Mbps_50		= 0xA8,	/* MicroMV 6.25Mbps/50			*/
		kAVCTapeSigModeAudido				= 0x20,	/* Audio						*/
		kAVCTapeSigModeHDV2_60				= 0x1A,	/* HDV2 /60						*/
		kAVCTapeSigModeHDV2_50				= 0x9A,	/* HDV2 /50						*/
		kAVCTapeSigModeDVCPro25_625_50		= 0xF8,	/* DVCPro25_625_50				*/
		kAVCTapeSigModeDVCPro25_525_60		= 0x78,	/* DVCPro25_525_60				*/
		kAVCTapeSigModeDVCPro50_625_50		= 0xF4,	/* DVCPro50_625_50				*/
		kAVCTapeSigModeDVCPro50_525_60		= 0x74,	/* DVCPro50_525_60				*/
		kAVCTapeSigModeDVCPro100_50			= 0xF0,	/* DVCPro100_50					*/
		kAVCTapeSigModeDVCPro100_60			= 0x70	/* DVCPro100_60					*/
	};
	
	/* avc tape transport mode */
	enum {
		kAVCTapeTportModeLoad				= 0xc1,	/* transport is loading			*/
		kAVCTapeTportModeRecord				= 0xc2,	/* transport is recording		*/
		kAVCTapeTportModePlay				= 0xc3,	/* transport is playing			*/
		kAVCTapeTportModeWind				= 0xc4	/* transport is winding			*/
	};
	
	/* avc tape wind subfunctions */
	enum {
		/* Move the medium toward the	*/
		/* beginning of medium as		*/
		/* quickly as possible			*/
		kAVCTapeWindHighSpdRew				= 0x45,	
		
		/* Halt all transport mechanism	*/
		/* motion						*/
		kAVCTapeWindStop					= 0x60,	
		
		/* Move the medium toward the	*/
		/* beginning of medium			*/
		kAVCTapeWindRew						= 0x65,
		
		/* Move the medium away from	*/
		/* the beginning of medium		*/
		kAVCTapeWindFastFwd					= 0x75		
	};
	
class TapeSubunitController
{

public:

	// Constructor
	TapeSubunitController(AVCDevice *pDevice, UInt8 subUnitID = 0x00);
	
	// Destructor
	~TapeSubunitController();

	// Commands
	
	IOReturn Play(UInt8 playMode = kAVCTapePlayFwd);
	IOReturn Record(UInt8 recordMode = kAVCTapeRecRecord);
	IOReturn Wind(UInt8 windMode = kAVCTapeWindStop);
	IOReturn EjectTape(void);

	IOReturn GetInputSignalMode(UInt8 *pInputSignalMode);
	IOReturn SetInputSignalMode(UInt8 inputSignalMode);

	IOReturn GetOutputSignalMode(UInt8 *pOutputSignalMode);
	IOReturn SetOutputSignalMode(UInt8 outputSignalMode);
	
	IOReturn GetMediumInfo(UInt8 *pCassetteType, UInt8 *pTapeGradeAndWriteProtect);
	IOReturn GetRelativeTimeCounter(UInt8 *pHour, UInt8 *pMinute, UInt8 *pSecond, UInt8 *pSignAndFrame);
	IOReturn GetTransportState(UInt8 *pTransportMode, UInt8 *pTransportState, bool *pIsStable);

	IOReturn GetTimeCode(UInt8 *pHour, UInt8 *pMinute, UInt8 *pSecond, UInt8 *pFrame);
	IOReturn SearchToTimeCode(UInt8 hour, UInt8 minute, UInt8 second, UInt8 frame);

private:
	
	AVCDevice *pAVCDevice;
	UInt8 subunitTypeAndID;
};
	
} // namespace AVS

#endif /* __AVCVIDEOSERVICES_TAPESUBUNITCONTROLLER__ */
