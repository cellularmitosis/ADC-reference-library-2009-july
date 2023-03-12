/*
Project: CIColorTracking

File: AppController.h

Abstract: 
This is the header file for AppController, a class that handles the interaction between the
GUI, the video, and the tracking filters, allowing you to select a source movie, change
tracking options, view the tracking video, see a preview of the mask used for the video, 
and overlay the mask image onto the tracking video.

Version 1.0

Disclaimer: IMPORTANT:  This Apple software is supplied to you by Apple Inc.
("Apple") in consideration of your agreement to the following terms, and your
use, installation, modification or redistribution of this Apple software
constitutes acceptance of these terms.  If you do not agree with these terms,
please do not use, install, modify or redistribute this Apple software.

In consideration of your agreement to abide by the following terms, and subject
to these terms, Apple grants you a personal, non-exclusive license, under
Apple's copyrights in this original Apple software (the "Apple Software"), to
use, reproduce, modify and redistribute the Apple Software, with or without
modifications, in source and/or binary forms; provided that if you redistribute
the Apple Software in its entirety and without modifications, you must retain
this notice and the following text and disclaimers in all such redistributions
of the Apple Software.
Neither the name, trademarks, service marks or logos of Apple Inc. may be used
to endorse or promote products derived from the Apple Software without specific
prior written permission from Apple.  Except as expressly stated in this notice,
no other rights or licenses, express or implied, are granted by Apple herein,
including but not limited to any patent rights that may be infringed by your
derivative works or by other works in which the Apple Software may be
incorporated.

The Apple Software is provided by Apple on an "AS IS" basis.  APPLE MAKES NO
WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED
WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS FOR A PARTICULAR
PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND OPERATION ALONE OR IN
COMBINATION WITH YOUR PRODUCTS.

IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION, MODIFICATION AND/OR
DISTRIBUTION OF THE APPLE SOFTWARE, HOWEVER CAUSED AND WHETHER UNDER THEORY OF
CONTRACT, TORT (INCLUDING NEGLIGENCE), STRICT LIABILITY OR OTHERWISE, EVEN IF
APPLE HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

Copyright (C) 2008 Apple Inc. All Rights Reserved.


*/

#import <Cocoa/Cocoa.h>
#import <QuartzCore/QuartzCore.h>
#import "VideoHandler.h";

@class VideoCIView;

@interface AppController : NSObject <VideoHandlerDelegate>
{
    IBOutlet id				colorTrackingBox;
    IBOutlet VideoCIView	*maskView;
    IBOutlet id				movieInfoBox;
    IBOutlet id				playButton;
    IBOutlet id				toleranceField;
    IBOutlet id				toleranceSlider;
    IBOutlet id				videoScrubber;
    IBOutlet id				maskOpacitySlider;
    IBOutlet id				showCompositeCheckBox;
    IBOutlet VideoCIView	*videoView;
	
	NSColor					*trackingColor;
	float					trackingTolerance;
	
	VideoHandler			*videoHandler;
	
	CIFilter				*cropFilter;
	CIFilter				*stage1_Filter_maskFromColor;
	CIFilter				*stage2_Filter_multiplyCoordinates;
	CIFilter				*stage2_Filter_areaMean;
	CIFilter				*stage2_Filter_coordinateNormalize;
	CIFilter				*stage3_Filter_compositeResult;
	CIFilter				*postProcessing_Filter_colorMatrix;
	CIFilter				*postProcessing_Filter_composite;
	
	NSRecursiveLock			*lock;
}
- (IBAction)movieScrubAction:(id)sender;
- (IBAction)selectMovie:(id)sender;
- (IBAction)setCompositeVisible:(id)sender;
- (IBAction)setMaskOverlayOpacity:(id)sender;
- (IBAction)setTrackingColor:(id)sender;
- (IBAction)setTrackingTolerance:(id)sender;
- (IBAction)togglePlayback:(id)sender;

- (void)renderFrame:(CVImageBufferRef)inFrame progress:(double)inProgress;
@end
