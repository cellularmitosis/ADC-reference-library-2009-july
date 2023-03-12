/*
	File:   PanelSubunitController.h
 
 Synopsis: This is the header for the PanelSubunitController Class 
 
	Copyright: 	¬© Copyright 2001-2003 Apple Computer, Inc. All rights reserved.
 
	Written by: ayanowitz
 
 Disclaimer:	IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc.
 ("Apple") in consideration of your agreement to the following terms, and your
 use, installation, modification or redistribution of this Apple software
 constitutes acceptance of these terms.  If you do not agree with these terms,
 please do not use, install, modify or redistribute this Apple software.
 
 In consideration of your agreement to abide by the following terms, and subject
 to these terms, Apple grants you a personal, non-exclusive license, under Apple‚Äôs
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

#ifndef __AVCVIDEOSERVICES_PANELSUBUNITCONTROLLER__
#define __AVCVIDEOSERVICES_PANELSUBUNITCONTROLLER__

namespace AVS
{
	
#define kPassThroughStatePressed 0	
#define kPassThroughStateReleased 1	

/* Panel PASS_THROUGH Operation IDs */
enum {
	kAVCPanelKeySelect = 0x00,	
	kAVCPanelKeyUp,
	kAVCPanelKeyDown,
	kAVCPanelKeyLeft,
	kAVCPanelKeyRight,
	kAVCPanelKeyRightUp,
	kAVCPanelKeyRightDown,
	kAVCPanelKeyLeftUp,
	kAVCPanelKeyLeftDown,
	kAVCPanelKeyRootMenu,
	kAVCPanelKeySetupMenu,
	kAVCPanelKeyContentMenu,
	kAVCPanelKeyFavMenu,
	kAVCPanelKeyExit,
	
	kAVCPanelKeyDigit0 = 0x20,
	kAVCPanelKeyDigit1,
	kAVCPanelKeyDigit2,
	kAVCPanelKeyDigit3,
	kAVCPanelKeyDigit4,
	kAVCPanelKeyDigit5,
	kAVCPanelKeyDigit6,
	kAVCPanelKeyDigit7,
	kAVCPanelKeyDigit8,
	kAVCPanelKeyDigit9,
	kAVCPanelKeyDot,
	kAVCPanelKeyEnter,
	kAVCPanelKeyClear,
	
	kAVCPanelKeyChUp = 0x30,
	kAVCPanelKeyChDown,
	kAVCPanelKeyPrevious,
	kAVCPanelKeyAudioSelect,
	kAVCPanelKeyInputSelect,
	kAVCPanelKeyInfo,
	kAVCPanelKeyHelp,
	kAVCPanelKeyPageUp,
	kAVCPanelKeyPageDown,
	
	kAVCPanelKeyPower = 0x40,
	kAVCPanelKeyVolUp,
	kAVCPanelKeyVolDown,
	kAVCPanelKeyMute,
	kAVCPanelKeyPlay,
	kAVCPanelKeyStop,
	kAVCPanelKeyPause,
	kAVCPanelKeyRecord,
	kAVCPanelKeyRewind,
	kAVCPanelKeyFastFwd,
	kAVCPanelKeyEject,
	kAVCPanelKeyForward,
	kAVCPanelKeyBackward,
	
	kAVCPanelKeyAngle = 0x50,
	kAVCPanelKeySubPicture,

	kAVCPanelKeyPlayFunction = 0x60,
	kAVCPanelKeyPlayPauseFunction,
	kAVCPanelKeyRecordFunction,
	kAVCPanelKeyPauseRecordFunction,
	kAVCPanelKeyStopFunction,
	kAVCPanelKeyMuteFunction,
	kAVCPanelKeyRestoreVolumeFunction,
	kAVCPanelKeyTuneFunction,
	kAVCPanelKeySelectDiscFunction,
	kAVCPanelKeySelectAVInputFunction,
	kAVCPanelKeySelectAudioInputFunction,
	
	kAVCPanelKeyF1 = 0x71,
	kAVCPanelKeyF2,
	kAVCPanelKeyF3,
	kAVCPanelKeyF4,
	kAVCPanelKeyF5,
	
	kAVCPanelKeyVendorUniq = 0x7E
};

class PanelSubunitController
{
	
public:
	
	// Constructor
	PanelSubunitController(AVCDevice *pDevice, UInt8 subUnitID = 0x00);
	
	// Destructor
	~PanelSubunitController();
	
	// Commands
	IOReturn PassThrough(UInt8 operationID, bool stateFlag, UInt8 operationDataLen, UInt8 *pOperationData);

	IOReturn Tune(UInt16 channelNum);
	IOReturn TuneTwoPartChannel(UInt16 majorChannelNum, UInt16 minorChannelNum);
		
private:
		
	AVCDevice *pAVCDevice;
	UInt8 subunitTypeAndID;
};

	
} // namespace AVS

#endif /* __AVCVIDEOSERVICES_PANELSUBUNITCONTROLLER__ */

	