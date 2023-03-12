//////////////////////////////////////////////////////////////////////////////////
/*
	File:		ControlInfo.h

	Project:	CarbonEvent Shell

	Contains:	Listing for the Control IDs and signatures for use in
			identifying controls to the application. 
			
	Author: 	Todd Previte
	
	Copyright:	2001 Apple Computer, Inc., All Rights Reserved

	Copyright:	(c) 2002 Apple Computer, Inc., All Rights Reserved
	
	Disclaimer:	

	IMPORTANT: This Apple software is supplied to you by Apple Computer, Inc.
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
//////////////////////////////////////////////////////////////////////////////////
#ifndef __ControlInfo_H
#define __ControlInfo_H

#define kCEVSApplicationSignature	'GLSS'

#define kCEVSTextBox			'TBOX'
#define kCEVSTextBoxID			101
#define kCEVSPauseButton		'PAUS'
#define kCEVSPauseID			102
#define kCEVSTimeSelector		'TSEL'
#define kCEVSTimeSelectorID		103
#define kCEVSTimeOutput			'TIME'
#define kCEVSTimeOutputID		104
#define kCEVSTimerSlider		'SLDR'
#define kCEVSTimerSliderID		105
#define kCEVSLog			'LOG'
#define kCEVSLogID			106
#define kCEVSELApplication		'ELAP'
#define kCEVSELApplicationID		107
#define kCEVSELWindow			'ELWI'
#define kCEVSELWindowID			108
#define kCEVSELMenu			'ELME'
#define kCEVSELMenuID			109
#define kCEVSELControl			'ELCO'
#define kCEVSELControlID		110

#define kCGLSButtonSnapshot		'SNAP'
#define kCGLSButtonSequence		'SEQU'
#define kCGLSButtonStart		'STRT'
#define kCGLSButtonStop			'STOP'
#define kCGLSSwitchTimeLimit		'TLMT'
#define kCGLSSwitchFrameLimit		'FLMT'

#define kCGLSTextFieldTimeInterval	'INTV'
#define kCGLSTextFieldFrameCount1	'FRM1'

#define kCGLSTextFieldElapsedTime	'ELPS'
#define kCGLSTextFieldFrameCount2	'FRM2'

#define kCGLSProgressBar		'PRGB'
#define KCGLSFramesCapturedCount	'FCAP'

#define kCGLSButtonSnapshotID		1001
#define kCGLSButtonSequenceID		1002
#define kCGLSButtonStartID		1003
#define kCGLSButtonStopID		1004
#define kCGLSSwitchTimeLimitID		1005
#define kCGLSSwitchFrameLimitID		1006

#define kCGLSTextFieldTimeIntervalID	1007
#define kCGLSTextFieldFrameCount1ID	1008

#define kCGLSTextFieldElapsedTimeID	1009
#define kCGLSTextFieldFrameCount2ID	1010

#define KCGLSProgressBarID		1011
#define KCGLSFramesCapturedCountID	1013
#endif