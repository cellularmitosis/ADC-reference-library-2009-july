/*
	File:		myDocument.h
	
	Description:Header file for the document class for the CASoundLab2 sample.
                CASoundLab2 is a Cocoa sample that demonstrates low-level access to Core Audio at the Harware Abstraction Layer (HAL).  CASoundLab2 implements a very simple tone generator capable of sine, square, and sawtooth waveforms of varying amplitudes and phases.
                
                Note:  There are more efficient ways to implement multiple waveforms using the AudioUnit framework.  Watch for a new version of this sample coming soon.  Also, this sample makes poor use of the Cocoa multiple-document architecture.  You can open multiple windows but they all use the same waveform parameters.  Not too exciting.
                                
	Author:		DH

	Copyright: 	© Copyright 2001 Apple Computer, Inc. All rights reserved.
	
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
				
	Change History (most recent first):
	
		6/22/2001		DH		First release of WWDC 2001 sample code

*/


#import <Cocoa/Cocoa.h>

@interface MyDocument : NSDocument
{
    IBOutlet NSControl* waveform1;
    IBOutlet NSControl* waveform2;
    IBOutlet NSControl* waveform3;
    IBOutlet NSControl* waveform4;

    IBOutlet NSControl* freq1;
    IBOutlet NSControl* freq2;
    IBOutlet NSControl* freq3;
    IBOutlet NSControl* freq4;
    
    IBOutlet NSTextField* freq1text;
    IBOutlet NSTextField* freq2text;
    IBOutlet NSTextField* freq3text;
    IBOutlet NSTextField* freq4text;
    
    IBOutlet NSControl* phase1;
    IBOutlet NSControl* phase2;
    IBOutlet NSControl* phase3;
    IBOutlet NSControl* phase4;
    
    IBOutlet NSTextField* phase1text;
    IBOutlet NSTextField* phase2text;
    IBOutlet NSTextField* phase3text;
    IBOutlet NSTextField* phase4text;

    IBOutlet NSControl* amp1;
    IBOutlet NSControl* amp2;
    IBOutlet NSControl* amp3;
    IBOutlet NSControl* amp4;
    
    IBOutlet NSTextField* amp1text;
    IBOutlet NSTextField* amp2text;
    IBOutlet NSTextField* amp3text;
    IBOutlet NSTextField* amp4text;
    
    IBOutlet NSView* wave1View;
    IBOutlet NSView* wave2View;
    IBOutlet NSView* wave3View;
    IBOutlet NSView* wave4View;
}


- (void)startAudio;
- (void)stopAudio;
- (void)dealloc;



- (IBAction) wave1changed: (id) sender;
- (IBAction) wave2changed: (id) sender;
- (IBAction) wave3changed: (id) sender;
- (IBAction) wave4changed: (id) sender;

- (void) drawWaveform: (int)waveform withFrequency: (float)freq phase: (float)phase andAmplitude: (float)amp inView: (NSView*)wave1View;

@end
