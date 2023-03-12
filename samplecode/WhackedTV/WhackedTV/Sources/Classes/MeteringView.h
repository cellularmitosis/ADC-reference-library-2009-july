/*	Copyright: 	� Copyright 2005 Apple Computer, Inc. All rights reserved.

	Disclaimer:	IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc.
			("Apple") in consideration of your agreement to the following terms, and your
			use, installation, modification or redistribution of this Apple software
			constitutes acceptance of these terms.  If you do not agree with these terms,
			please do not use, install, modify or redistribute this Apple software.

			In consideration of your agreement to abide by the following terms, and subject
			to these terms, Apple grants you a personal, non-exclusive license, under Apple�s
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

#import <Cocoa/Cocoa.h>

// This view class was stolen from /Developer/Examples/CoreAudio/Services/MatrixMixerTest/
// and sub-classed to be presented horizontally.

@interface MeteringView : NSView {
	int 	mNumChannels;
	
	double  mMinDB;
	double	mMaxDB;
	double 	mMinValue;
	double 	mMaxValue; 
	
	float  *mMeterValues;
	float  *mOldMeterValues;
	
	int	   *mClipValues;
	
	float 	firstTrackOffset;
	
	BOOL drawsMetersOnly;
	BOOL mHasClip;
    BOOL mFirstTime;
}

- (void) setNumChannels: (int) num;
- (int)  numChannels;
- (void) setMinValue: (double) num; // min value (usually 0)
- (void) setMaxValue: (double) num; // max value (usually 1 to √2)

- (void) setDirty: (BOOL)dirty;

- (void) setMinDB: (double) num; // min db value (usually -INF)
- (void) setMaxDB: (double) num; // max db value (usually 0 to +6)

- (void) setHasClipIndicator: (BOOL) hasClip;

- (float)  pixelForValue: (double) value inSize: (int) size;

- (void) updateMeters: (float *) meterValues;	// takes an array of floats
												// meterValue[0]: db value for channel 0
												// meterValue[1]: db peak for channel 0
												// meterValue[2]: db value for channel 1
												// meterValue[3]: db peak for channel 1 ...etc

@end
