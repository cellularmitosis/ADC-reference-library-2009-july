/*

File: LiveVideoMixerController.h

Abstract:   This is the application controller object.
	    It also functions as the document as it holds
	    the video channels.

Version: 1.0

� Copyright 2005 Apple Computer, Inc. All rights reserved.

IMPORTANT:  This Apple software is supplied to 
you by Apple Computer, Inc. ("Apple") in 
consideration of your agreement to the following 
terms, and your use, installation, modification 
or redistribution of this Apple software 
constitutes acceptance of these terms.  If you do 
not agree with these terms, please do not use, 
install, modify or redistribute this Apple 
software.

In consideration of your agreement to abide by 
the following terms, and subject to these terms, 
Apple grants you a personal, non-exclusive 
license, under Apple's copyrights in this 
original Apple software (the "Apple Software"), 
to use, reproduce, modify and redistribute the 
Apple Software, with or without modifications, in 
source and/or binary forms; provided that if you 
redistribute the Apple Software in its entirety 
and without modifications, you must retain this 
notice and the following text and disclaimers in 
all such redistributions of the Apple Software. 
Neither the name, trademarks, service marks or 
logos of Apple Computer, Inc. may be used to 
endorse or promote products derived from the 
Apple Software without specific prior written 
permission from Apple.  Except as expressly 
stated in this notice, no other rights or 
licenses, express or implied, are granted by 
Apple herein, including but not limited to any 
patent rights that may be infringed by your 
derivative works or by other works in which the 
Apple Software may be incorporated.

The Apple Software is provided by Apple on an "AS 
IS" basis.  APPLE MAKES NO WARRANTIES, EXPRESS OR 
IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED 
WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY 
AND FITNESS FOR A PARTICULAR PURPOSE, REGARDING 
THE APPLE SOFTWARE OR ITS USE AND OPERATION ALONE 
OR IN COMBINATION WITH YOUR PRODUCTS.

IN NO EVENT SHALL APPLE BE LIABLE FOR ANY 
SPECIAL, INDIRECT, INCIDENTAL OR CONSEQUENTIAL 
DAMAGES (INCLUDING, BUT NOT LIMITED TO, 
PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS 
OF USE, DATA, OR PROFITS; OR BUSINESS 
INTERRUPTION) ARISING IN ANY WAY OUT OF THE USE, 
REPRODUCTION, MODIFICATION AND/OR DISTRIBUTION OF 
THE APPLE SOFTWARE, HOWEVER CAUSED AND WHETHER 
UNDER THEORY OF CONTRACT, TORT (INCLUDING 
NEGLIGENCE), STRICT LIABILITY OR OTHERWISE, EVEN 
IF APPLE HAS BEEN ADVISED OF THE POSSIBILITY OF 
SUCH DAMAGE.

*/ 

#import "VideoChannel.h"
#import <Cocoa/Cocoa.h>
#import <QuartzCore/CoreVideo.h>

#import "VideoOutSupport.h"

// constants for the mask textures that build the shapes
#define	kNumTextures				4
#define	kTextureTarget				GL_TEXTURE_RECTANGLE_EXT    
#define	kSpeechBubbleTextureID			kNumTextures - 1
#define	kSpeechBubble				1

// size of the area in which the user can click to resize the movie in positioning mode
#define	kSelectionHandleSize			20.0f

//for simplicity in this example we assume a fixed movie size of 720x480
#define	kVideoWidth				720.0f
#define	kVideoHeight				480.0f

extern GLuint				gTexNames[kNumTextures];

typedef enum SelectionHandleID
{
	kNoSelectionHandle = 0,
	kTopLeftCorner,
	kTopRightCorner,
	kBottomLeftCorner,
	kBottomRightCorner
} SelectionHandleID;

@class VideoMixView;

@interface LiveVideoMixerController : NSObject
{
    IBOutlet id channelController0;
    IBOutlet id channelController1;
    IBOutlet id channelController2;
    IBOutlet VideoMixView *mainView;
    IBOutlet id playButton;
    IBOutlet id positionSwitch0;
    IBOutlet id positionSwitch1;
    IBOutlet id positionSwitch2;
    
    // master time base as an option to synch the movies
    TimeBase				masterTimeBase;
    BOOL				useMasterTimeBase;
	
    // the CVDisplayLink
    BOOL				isPlaying;
    CVDisplayLinkRef			displayLink;
    CGDirectDisplayID			mainViewDisplayID;
    
    // which channel is currently in positioning mode - its -1 if no channel is in positioning mode
    SInt32				currentChannelPositioned;	
    
    // Timer for the MoviesTask call on the main thread
    NSTimer				*qtHousekeepingTimer;
    
    VideoOutSupport			*videoOutSupport;
}
- (IBAction)openChannelFile:(id)sender;
- (IBAction)setChannelPosition:(id)sender;
- (IBAction)togglePlayback:(id)sender;

- (BOOL)isPlaying;

- (NSRect)mainVideoRect;
- (void)drawContent:(NSRect)mainVideoRect mainVideoOnly:(BOOL)mainVideoOnly;

- (CVReturn)render:(const CVTimeStamp*)syncTimeStamp;

- (void)mouseDown:(NSEvent *)theEvent;

- (void)windowChangedScreen:(NSNotification*)inNotification;

@end

CVReturn myCVDisplayLinkOutputCallback(CVDisplayLinkRef displayLink, 
                                                const CVTimeStamp *inNow, 
                                                const CVTimeStamp *inOutputTime, 
                                                CVOptionFlags flagsIn, 
                                                CVOptionFlags *flagsOut, 
                                                void *displayLinkContext);
