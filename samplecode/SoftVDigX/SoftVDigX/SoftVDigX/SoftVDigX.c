/*
	File:		SoftVDigX.c
	
	Description: QuickTime Compressed Source Video Digitizer for MacOS X.
				 Note: Accompanying KEXT must be loaded for this sample to work do this by following these steps:
                 First copy the file to /tmp
                    1) sudo cp -R /Users/ed/Desktop/SoftVDigKEXT.kext /tmp/
                 Second, load the kext
                    2) sudo kextload -v /tmp/SoftVDigKEXT.kext

	Author:		QuickTime Engineering (the usual suspects)
				Developer Techical Support
				FCP information provided by FCP engineering
	
				Parts from the code of the past:
				
					SoftVDig & SoftVDig2000 originally written by QuickTime engineering.
	
					Develop Issue 14, "Video Digitizing Under QuickTime",
					for some details.
												
	Copyright: 	� Copyright 1993-2007 Apple Computer, Inc. All rights reserved.
	
	Disclaimer: IMPORTANT:  This Apple software is supplied to you by 
                Apple Inc. ("Apple") in consideration of your agreement to the
                following terms, and your use, installation, modification or
                redistribution of this Apple software constitutes acceptance of these
                terms.  If you do not agree with these terms, please do not use,
                install, modify or redistribute this Apple software.

                In consideration of your agreement to abide by the following terms, and
                subject to these terms, Apple grants you a personal, non-exclusive
                license, under Apple's copyrights in this original Apple software (the
                "Apple Software"), to use, reproduce, modify and redistribute the Apple
                Software, with or without modifications, in source and/or binary forms;
                provided that if you redistribute the Apple Software in its entirety and
                without modifications, you must retain this notice and the following
                text and disclaimers in all such redistributions of the Apple Software. 
                Neither the name, trademarks, service marks or logos of Apple Inc. 
                may be used to endorse or promote products derived from the Apple
                Software without specific prior written permission from Apple.  Except
                as expressly stated in this notice, no other rights or licenses, express
                or implied, are granted by Apple herein, including but not limited to
                any patent rights that may be infringed by your derivative works or by
                other works in which the Apple Software may be incorporated.

                The Apple Software is provided by Apple on an "AS IS" basis.  APPLE
                MAKES NO WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION
                THE IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS
                FOR A PARTICULAR PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND
                OPERATION ALONE OR IN COMBINATION WITH YOUR PRODUCTS.

                IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL
                OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
                SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
                INTERRUPTION) ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION,
                MODIFICATION AND/OR DISTRIBUTION OF THE APPLE SOFTWARE, HOWEVER CAUSED
                AND WHETHER UNDER THEORY OF CONTRACT, TORT (INCLUDING NEGLIGENCE),
                STRICT LIABILITY OR OTHERWISE, EVEN IF APPLE HAS BEEN ADVISED OF THE
                POSSIBILITY OF SUCH DAMAGE.
				
	Change History (most recent first):
        <3>     02/07/07    dts     Updated for QuickTime 7.1 - added Image Description properties
                                    as outlined in Ice Floe 19
        <2>     08/14/05    dts     updated to build universal binary
        <1>		08/14/03	dts		initial release for X
*/

/* Selected excerpts from the famous VIDEO DIGITIZING UNDER QUICKTIME by Casey King and Gary Woodcock
   Develop Issue 14 which pertain to the task at hand - Published a long time ago in the far away land
   of QuickTime 1.5.
      
   Enjoy!

   "...And the evolution of this technology has just begun. - 1993"

***** COMPRESSED SOURCE VIDEO DIGITIZERS FOR MACINTOSH OS X *****

	BACKGROUND - THE VIDEO CAPTURE PROCESS

The process of making movies involves several components. The sequence grabber component (component type 'barg')
plays an especially critical role. It's responsible for coordinating the activities of the lower-level components
to achieve different results -- like displaying video in a window, grabbing a single picture, or grabbing a movie.
By protecting application developers from having to deal with the low-level management of the video digitizer, the
sequence grabber makes it much easier to incorporate video input capabilities in applications. Note that the sequence
grabber also handles audio input devices and synchronization of picture and sound.

Data flow in the video digitizing pipeline begins with a video source for example, a video camera, VCR, direct broadcast
feed and so on. The video digitizing hardware is responsible for converting the video source to digital form and can
optionally perform resizing, color conversion, or clipping. In the absence of hardware support for these operations,
the sequence grabber will sometimes provide them.

The video digitizer's ('vdig') primary purpose in life is to provide QuickTime with a consistent software interface used
to interact with the video digitizer hardware. The data supplied by the 'vdig' component can either be displayed on screen
or be processed further through the use of the sequence grabber component.

From a video digitizing perspective, displaying live video in a window is called play through(*), and capturing video to a movie
is called capturing or grabbing. The sequence grabber calls these operations previewing and recording respectively. When a movie
is requested, an image compressor may process the data further, and the result is stored either in system memory or to disk.

(*)It is very important to note that for a compress-only video digitizer, playthough support at the 'vdig' level is ignored. Hardware
on MacOS X cannot go directly to the screen, so video digitizer functions supporting playthough are not required. Playthough
is handled by QuickTime and is mentioned in the above description for completeness.

	THE SEQUENCE GRABBER
	
The sequence grabber makes life easier for application programmers by handling all the messy details of controlling video digitizers,
sound input devices, and compressors. The sequence grabber plays such an important role in a video digitizing application that a brief
introduction is in order. The HackTV Carbon sample is a good introduction to the Sequence Grabber set of APIs.

	PREVIEWING
	
Previewing video with the sequence grabber is the equivalent of setting up the 'vdig' in play-through mode to display live video on the
computer screen. Remember that the sequence grabber performs the live video play through and devices should not do hardware play
through or attempt to directly draw to the screen on MacOS X.

In general the steps are, establish a connection to the sequence grabber component in the usual way, with OpenADefaultComponent.
Initialize the sequence grabber, set up the graphics environment, and allocate a new channel for video preview (using the seqGrabPreview flag).
If for example you want the displayed video to be one-quarter size, you would call SGGetSrcVideoBounds to see what size the source video
is. This function calls VDGetDigitizerRect, which returns the source video size equal to the digitizer rectangle. You would Scale the
height and width accordingly, and then send the new size to the sequence grabber via the SGSetChannelBounds routine.

Finally, call SGStartPreview, which turns on the video digitizer by calling the digitizer function VDSetCompressionOnOff, and previewing
begins.

Make sure to always call SGSetGWorld to establish the graphics port for the sequence grabber component. This is required even if you do
not plan to work with a video channel.

	RECORDING

Recording is very similar to previewing and is almost as simple.

There are several small differences between recording and previewing. For example you can set up the sequence grabber to record and to play
through while recording (using the seqGrabRecord | seqGrabPlayDuringRecord flags). We need to specify a file for the movie to be written
to, indicating that the movie be grabbed directly to disk. For a short movie, we could grab to memory if we wanted. By default,
the recording time is limited by the system resources available -- in this case, disk space.

The SGStartRecord call initiates the grab to disk. SGIdle is called repetitively to provide processing time to the sequence grabber.
You should call SGIdle as often as possible while recording. When the user clicks the mouse button, or when the disk is full, recording
will stop, and we call SGStop to complete the recording process.

That's all there is to simple recording. If you want to do more sophisticated tasks with the sequence grabber, such as replacing the
standard sequence grabber disk- or compression-bottleneck routines with your own, or using a DataProc for faster captures consult the
QuickTime Sequence Grabber documentation and Sample Code.

http://developer.apple.com/documentation/QuickTime/RM/CreatingMovies/SeqGrabComp/index.html
http://developer.apple.com/samplecode/Sample_Code/QuickTime/Capturing.htm

***********************************
*** SOME VIDEO DIGITIZER BASICS ***

We'll discuss a few QuickTime 'vdig' topics that seem to give developers the most trouble when they first undertake the task of
writing a video digitizer component. In practice, how you write a video digitizer component depends heavily on your particular
digitizer hardware implementation. For the sake of illustration, however, we'll simulate some hardware features with software
that provides the required functionality.

	IDENTIFYING DIGITIZER TYPES AND CAPABILITIES

No two video digitizers are alike. To make sure your 'vdig' component works smoothly with
QuickTime, it's critical to identify the capabilities your hardware provides.

This sample demonstrates hardware not attached to a FrameBuffer (a FireWire device for example):
	- Set digiOutDoesCompress, this implies the use of the Compressed Source APIs such as VDCompressDone(), VDReleaseCompressBuffer(),
	  VDCompressOneFrameAsync() and so on.
	  Note: Be sure to tell the truth when reporting the number of queued frames (queuedFrameCount number).
	  The Ptr value in VDCompressDone()- can be any memory address to read from.
	- Set digiOutDoesCompressOnly when above flag is set.
	- Set digiOutDoesNotNeedCopyOfCompressData - When the Sequence Grabber is recording, QuickTime can manage capture buffers for you
	  automatically. Sometimes, you don't want QuickTime buffer management - setting this flag informs QuickTime that you specifically
	  DO NOT want this behavior, therefore the Sequence Grabber will not buffer any of your compressed data. In other words you don't
	  want the Sequence Grabber copying your data to system memory before writing it to disk. This allows for DMA directly over the
	  PCI bus avoiding a trip though system memory before the sample data hits the disk.

The 'vdig' component interface attempts to be very flexible in allowing you to indicate what your hardware
can do. A 'vdig' specifies its type and capabilities in the DigitizerInfo structure, shown below. Two
calls -- VDGetDigitzerInfo and VDGetCurrentFlags give a client (normally the sequence grabber)
access to information contained in this structure.

  struct DigitizerInfo {
     short       vdigType;
     long        inputCapabilityFlags;
     long        outputCapabilityFlags;
     long        inputCurrentFlags;
     long        outputCurrentFlags;
     short       slot;
     GDHandle    gdh;
     GDHandle    maskgdh;
     short       minDestHeight;
     short       minDestWidth;
     short       maxDestHeight;
     short       maxDestWidth;
     short       blendLevels;
     long        reserved;
 };
 
In the vdigType field, you specify the type of digitizer you are. Compressed Source Digitizers are basic
rectangular digitizing devices - vdTypeBasic.

In the capability flags fields, you indicate to clients all capabilities a particular digitizer instance provides. 
The current flags fields use the same attribute bit fields as the capability flags, but they indicate the
currently available capabilities, not the total possible capabilities. By nature, some capabilities are
mutually exclusive. For instance, if you support NTSC and PAL input formats, at any given time you're actively
doing only one of them. The bit corresponding to the active standard is the one that would be set in the current flags,
while both would be set in the capability flags.

A full description of the flags can be found later in this sample before the SoftVDigXGetDigitizerInfo function, and
at the following URL:
	http://developer.apple.com/documentation/QuickTime/INMAC/QTC/imVideoDigComp.10.htm

Clients of your digitizer will be much happier if you truthfully state what you can and can't do. The sequence grabber,
in particular, will function much better.

	THE EXAMPLE - SoftVDigX by Kext-O-Tron
	
As an example we'll describe the "hardware" used in this sample:

A fictional digitizer company "Kext-O-Tron", announces a video digitizer with the following
capabilities:

The fictional hardware has one video input and can support only the NTSC video standard.
The fictional hardware supports compression and provides '2vuy' pixel data. (the same 30 frames over and over)
The fictional hardware only supports capture at 320x240 and does not support clipping, resizing or zooming.
The fictional hardware will be relying on the sequence grabber to do resizing, color conversion, and on-screen
placement operations.

The interesting fields and their values are:

vdigType = vdTypeBasic;
inputCapabilityFlags = digiInDoesNTSC | digiInDoesColor | digiInDoesComponent;

outputCapabilityFlags = digiOutDoes32       	|
						digiOutDoesCompress		|
						digiOutDoesCompressOnly	|
						digiOutDoesNotNeedCopyOfCompressData;

	GETTING VIDEO COORDINATE SYSTEMS STRAIGHT

http://developer.apple.com/documentation/QuickTime/INMAC/QTC/imVideoDigComp.4.htm

The coordinate system for video digitizers can seem confusing, but it's actually quite straightforward.
The critical point to keep in mind is that when referencing the video source, you're working in a coordinate
system that's specific to your digitizing hardware. All cropping rectangles are relative to this coordinate system.

Four rectangles define the video source. The MaxSrcRect defines the maximum source area that the digitizer is
capable of grabbing. Typically this area includes all or portions of the vertical and horizontal blanking areas.
Note that you don't have to define the top left point of MaxSrcRect as 0,0; this is an entirely arbitrary reference
point that 'vdig' developers can define as they choose. The other three rectangles are defined in relation to MaxSrcRect.

The ActiveSrcRect is the region of the maximum source rectangle that contains the actual video image.
The first pixel of active video is the top left corner, and the last pixel is the bottom right.

The DigitizerRect describes the area of the MaxSrcRect to be captured -- the image that the user will
actually see, although it hasn't been scaled yet. The default DigitizerRect is the same as the ActiveSrcRect.

It's not uncommon for part of the blanking signal to be displayed in the ActiveSrcRect. This is because different
source devices -- like VCRs, and broadcast signals -- send out slightly different analog signals. To align the image,
a 'vdig' client can nudge the DigitizerRect a few pixels in the appropriate direction using VDSetDigitizerRect.

To describe a cropped image, the DigitizerRect is usually defined as a portion of the ActiveSrcRect.

The last rectangle, VBlankRect, defines the area of vertical blanking. This region can contain vertical
interval time code (VITC), closed captioning, and teletext.

Remember, all rectangle coordinates are relative to MaxSrcRect.

MaxSrcRect, ActiveSrcRect, and VBlankRect are hardware dependent. The only control that a client has is in the
definition of DigitizerRect.

Note that in the SoftVDigXSetDigitizerRect function, the requested DigitizerRect must fully intersect the MaxSrcRect.

One more very important point to understand about video coordinate systems is that scaling and
translation can be specified either as a matrix or as a destination rectangle. Your digitizer may support
both transformation methods. If the matrix is NULL, use the destination rectangle; otherwise, ignore it
and use the matrix. The 'vdig' must offset the top left of the DigitizerRect to 0,0 before applying the
matrix, or the video won't be positioned correctly. This isn't necessary when using the destination
rectangle. By the way, the sequence grabber uses the destination rectangle, and not the matrix, to
specify scaling and translation, although this may change at any time.
Note: If you claim you only record at one size, this is not your problem. QuickTime will take care of
resizing the video as required.

	DIGITIZATION CONTROL

Video digitizers should time-stamp the video frames they produce and implement the VDSetTimeBase routine.
VDSetTimeBase allows clients to specify the time coordinate system the video digitizer should use when time-stamping
video frames.

A second routine called VDSetFrameRate, allows applications to tell a digitizer the precise frame rate to
use for capture. Digitizers used to capture video at only one frame rate -- as fast as possible.
However, the advent of full-frame-rate digitizing hardware and compressed-source devices has made
it increasingly important for clients to manage the tradeoff between frame rate, image size, and
compression quality. The rate in VDSetFrameRate is expressed as a fixed-point value, typically
between 0 and 29.97 (see "Frame Rates and Motion Quality" for a discussion of frame rate values).

Additionally VDGetDataRate retrieves information describing the performance capabilities of a video digitizer. It is
extremely useful because it gives clients a way to determine the performance capabilities of your video digitizer. It is
called before recording has started and allows you to report milliseconds per frame, frames per second and bytes per second.
You can safely return 0 for milliSecPerFrame, bytesPerSecond however MUST BE ACCURATE. For framesPerSecond you should return
the frame rate you can "really" do, but you can also return 0 if you can't control the frame rate. You MUST return
bytesPerSecond, this is not an option!

	COMPRESSED-SOURCE DEVICES

Compressed-source video digitizers gives users access to full-size, full-frame-rate digital video.

Eight functions are needed for servicing compressed-source video digitizers.

VDGetCompressionTypes simply returns a handle to the list of compressors that your video digitizer implements.
Each element of the list contains the component ID, type, name, format, and capabilities of a compressor.

VDSetCompression specifies which of all the possible compressors a digitizer should use. The parameters for
VDSetCompression specify the spatial quality, temporal quality, depth, and other characteristics of the compression.

VDCompressOneFrameAsync starts the digitizing process for compressed-source devices. A compressed-source digitizer
handles all the management of data buffers itself, without external assistance from the caller.

VDCompressDone allows a caller to determine when a frame has been completed.

The VDReleaseCompressBuffer tells a compressed-source device to free the buffer returned by the VDCompressDone call.

The VDResetCompressSequence call instructs a digitizer to insert a key frame into a frame-differenced image sequence as soon
as possible after it receives this call.

The VDGetImageDescription routine prompts the digitizer to return an image description structure corresponding to the current settings.
This structure is the same structure that's used to describe image data in movie files.

The VDSetCompressionOnOff routine starts and stops the digitizer. To give the digitizer adequate time to prepare itself for the
requested operation, clients must call this routine before calling VDSetCompression or VDCompressOneFrameAsync.

Typically, compressed-source devices are able to act as hardware decompressors and have a corresponding QuickTime image decompressor
component that clients can use to play back the compressed images. However, if your hardware produces compressed data that can't be
read by any of the standard QuickTime image decompressor components, you need to provide an appropriate software-only decompressor
component. This way, users who don't have your hardware will still be able to play movies produced with your compressor.

	FRAME RATES AND MOTION QUALITY

The term frame rate is frequently tossed about in discussions of the pros and cons of video digitizers.
Frame rate is the rate at which frames appear during video playback.

It's commonly held that the frame rate corresponding to full-motion video is 30 frames per second
(fps). However, 30 fps is not the only interesting frame rate or even the only "true" full-motion
frame rate.

Most video digitizing hardware have a fixed frame rate:

29.97 fps is the usual frame rate for NTSC video and broadcast production.
25 fps is the usual frame rate for PAL and SECAM video and broadcast production.
24 fps is the usual frame rate for film production.

But QuickTime does not limit frame rate to only the above values. It could be anything. A Video Digitizer will be told
what frame rate QuickTime wants to record at. If the Video Digitizer cannot achive this frame rate, QuickTime will try
it's best to do it. However, results may vary and implementing rate control on the hardware side will result in better
accuracy.

See QuickTime Image Rates and Video in Ice Floe 19 for the the full description of the issues related to TimeScale and
sample durations:

	http://developer.apple.com/quicktime/icefloe/dispatch019.html

NTSC and digital 525 video have exactly 30/1.001 frames per second. A media TimeScale of 30000 and media
sample durations of 1001, along with a movie TimeScale of 30000, provide 19.9 hours of time in 32 bits.
The 30/1.001 rate of NTSC and digital 525 is sometimes approximated to (or mistaken to be) 29.97, which does
not equal 30/1.001. The average rate of drop-frame timecode (e.g., LTC and VITC used in the video industry) over
24 hours is exactly 29.97, but the use of drop-frame timecode does not modify the video signal rate. Some existing
movies have a media TimeScale of 2997, media sample durations of 100, and a movie TimeScale of 2997. This provides
8.2 days of time before 32-bit overflow, however after 4.6 hours, this representation deviates from the actual video
timing by half a frame time. For tracks with a 2997 timescale which are longer than 4.6 hours, a frame-accurate
representation of the video timing is only possible if the frames are allowed to have differing durations.

Most applications today only deal with material shorter than 4.6 hours. These applications should be able to read video
movies with either 2997 or 30000 TimeScales. They may write either, but the 30000 TimeScale is preferred since it has a
sufficient overflow time and precisely models the video signal.

Final Cut Pro uses 2997/100 and 2500/100. It will report that is has detected a dropped frame if the returned
frame durations do not match what was expected.

In addition to frame rate, there are two other terms you should know about. If the "frame rate" measures the
speed at which the movie is played back for the viewer, the "capture rate" is the rate at which the
'vdig' hardware is capable of capturing frames. The "effective capture rate" is the number of
frames per second that end up in a QuickTime movie. Many factors can make the effective
capture rate less than the intrinsic rate the hardware can support.

*/

#pragma mark-
//-----------------------------------------------------------------------
// includes

#include <Carbon/Carbon.h>
#include <IOKit/IOKitLib.h>
#include <stdio.h>

#include <QuickTime/ImageCodec.h> // for ImageDescription extentions

#include "SoftVDigX.h"
#include "SoftVDigKEXTCommon.h"	// indexes for calling the user client

//-----------------------------------------------------------------------

/************************************************************************************/
// Setup required for ComponentDispatchHelper.c

#define VD_BASENAME()				SoftVDigX
#define	VD_GLOBALS()				SoftVDigXGlobalsPtr storage
#define CALLCOMPONENT_BASENAME()	VD_BASENAME()
#define	CALLCOMPONENT_GLOBALS()		VD_GLOBALS()

#define COMPONENT_DISPATCH_FILE		"SoftVDigXDispatch.h"

#define COMPONENT_UPP_SELECT_ROOT()	VD	

#include <CoreServices/Components.k.h>
#include <QuickTime/QuickTimeComponents.k.h>
#include <QuickTime/ComponentDispatchHelper.c>

/************************************************************************************/
#define kComponentVersion 1
/************************************************************************************/
#pragma mark- kCompressionFormat

// the compression format - we only support k422YpCbCr8CodecType
const OSType kCompressionFormat = k422YpCbCr8CodecType;

#pragma mark---- Hardware Specific Calls
/* Digitizer Hardware Specific Calls */

// GetDeviceName - return the name from the IORegistry
static kern_return_t GetDeviceName(const io_service_t inObj, char outDeviceName[])
{
	CFMutableDictionaryRef properties;
	CFStringRef strDesc;
	kern_return_t err;
	
	err = IORegistryEntryCreateCFProperties(inObj, &properties, kCFAllocatorDefault, kNilOptions);
	if (err) return err;
	
	strDesc = (CFStringRef)CFDictionaryGetValue(properties, CFSTR("DeviceName"));
    if(strDesc) {
		CFStringGetCString(strDesc, outDeviceName, 33, kCFStringEncodingMacRoman);
	}
	
	CFRelease(properties);
	
	return KERN_SUCCESS;
}

// ConnectToKEXT - attempt to find our driver and instantiate the user client
static Boolean ConnectToKEXT(SoftVDigXGlobalsPtr storage)
{
	kern_return_t	kernResult; 
    mach_port_t		masterPort;
    io_service_t	serviceObject = 0;
    io_iterator_t 	iterator;
    CFDictionaryRef	classToMatch;
	Boolean			result = true;	// assume success
    
    // return the mach port used to initiate communication with IOKit
    kernResult = IOMasterPort(MACH_PORT_NULL, &masterPort);
    if (kernResult != KERN_SUCCESS) {
		result = false;
		goto bail;
	}
    
    classToMatch = IOServiceMatching( "com_dts_iokit_SoftVDigKEXT" );
    if (NULL == classToMatch) {
		result = false;
		goto bail;
    }

    // create an io_iterator_t of all instances of our driver's class
	// that exist in the IORegistry
    kernResult = IOServiceGetMatchingServices(masterPort, classToMatch, &iterator);
    if (kernResult != KERN_SUCCESS) {
		result = false;
		goto bail;
    }    
        
    // get the first item in the iterator.
    serviceObject = IOIteratorNext(iterator);
    
    // release the io_iterator_t now that we're done with it.
    IOObjectRelease(iterator);
    
    if (0 == serviceObject){
		result = false;
		goto bail;
    }
	
	// instantiate the user client
    kernResult = IOServiceOpen(serviceObject, mach_task_self(), 0, &(storage->userClient));
	if(kernResult != KERN_SUCCESS) {
		result = false;
		goto bail;
    }
	
	// get the device name out of the registry
	kernResult = GetDeviceName(serviceObject, storage->deviceName);	
	if(kernResult != KERN_SUCCESS) {
		result = false;
		goto bail;
    }

	kernResult = IOConnectMapMemory(storage->userClient,
									1,
									mach_task_self(),
									&storage->framePtr,
									&storage->frameSize,
									kIOMapAnywhere );
bail:
	if (serviceObject) {
		// release the io_service_t now that we're done with it
		IOObjectRelease(serviceObject);
	}
	
    return result;
}

// GetMaxSrcRectFromKEXT - call the driver to get the max source rect
static void GetMaxSrcRectFromKEXT(SoftVDigXGlobalsPtr storage, Rect* rect)
{
	kern_return_t kernResult;
	IOByteCount	  rectSize = sizeof(Rect);

    kernResult = IOConnectMethodStructureIStructureO(storage->userClient,
													 kGetMaxSrcRect,	
													 0,			// input structure size
													 &rectSize,	// ptr to output structure size
													 NULL,		// ptr to input structure
													 rect);		// ptr to output structure

}

/************************************************************************************/
#pragma mark---- Utility Functions

/* Utility Functions */

static void TimeToTimeCode(const TimeRecord *inAtTime, const TimeCodeDef *inTimeCodeDef, TimeCodeTime *inTimeCodeTime)
{
	TimeRecord tr = *inAtTime;
	UInt16 fps = inTimeCodeDef->numFrames;
	SInt32 remainder;
	UInt32 frameCount;

	ConvertTimeScale(&tr, inTimeCodeDef->fTimeScale);
	frameCount = CompDiv(&tr.value, inTimeCodeDef->frameDuration, &remainder);
	
	if (inTimeCodeDef->flags & tcDropFrame) {
		UInt32 number10s, number1s;
		UInt32 fpm = fps * 60 - 2;
		UInt32 fpm10 = fps * 10 * 60 - 9 * 2;
		UInt32 frameAdjust;
		UInt32 numberFramesLeft;
		
		number10s = frameCount / fpm10;
		frameAdjust = number10s * (9 * 2);
		numberFramesLeft = frameCount % fpm10;
		if (numberFramesLeft >= fps * 60) {
			numberFramesLeft -= fps * 60;
			number1s = numberFramesLeft / fpm;
			frameAdjust += (number1s + 1) * 2;
		}
		frameCount += frameAdjust;
	}
	
	inTimeCodeTime->frames = frameCount % fps;
	frameCount /= fps;
	inTimeCodeTime->seconds = frameCount % 60;
	frameCount /= 60;
	inTimeCodeTime->minutes = frameCount % 60;
	frameCount /= 60;
	
	if (inTimeCodeDef->flags & tc24HourMax) {
		frameCount %= 24;
	}
	
	inTimeCodeTime->hours = frameCount + 1;	// we want to start at 1:00:00;00
}

/************************************************************************************/
#pragma mark---- Standard Component Calls

/* Standard Component Calls */

// Component Open Request - Required
// Note QuickTime may call you Video Digitizers Open many times - be sure to
// only perform a very minimum amount of work in this call
pascal ComponentResult SoftVDigXOpen(SoftVDigXGlobalsPtr storage, ComponentInstance self)
{
    SInt32 response;
	ComponentResult err;
    
    // 10.4.8 or later
    err = Gestalt(gestaltSystemVersion, &response);
    if (err || (response < 0x1048)) return -1;                                   
	
	// we can only have one instance
	if (CountComponentInstances((Component)self) > (short)kMaxSoftVDigXCount) return -1;

	// allocate some memory for our globals...
	storage = (SoftVDigXGlobalsPtr)NewPtrClear(sizeof(SoftVDigXGlobalsRecord));
	if (err = MemError()) goto bail;
	
	//...and tell the component manager about it
	storage->self = self;
	SetComponentInstanceStorage(self, (Handle)storage);

	// see if we can connect to the KEXT
	if(!ConnectToKEXT(storage)) return -1;
	
	// ConnectToKext gets the device name from the IORegistry, and this is
	// the base for the input name used in VDGetInputName - for our example
	// device always append " 8-bit NTSC" because it's all that we do
	strcat(storage->deviceName, " 8-bit NTSC");

	// allocate an image description for the compressed type we support
	storage->desc = (ImageDescriptionHandle)NewHandleClear(sizeof(ImageDescription));
	if (err = MemError()) goto bail;

	// initialize image description
    // For information regarding the Image Description version number
    // and proerties being added to the Image Description, please consult
    // Ice Floe Dispach 19 and note how the information applies to the formats
    // your Video Digitizer is supplying
    // http://developer.apple.com/quicktime/icefloe/dispatch019.html
    
	(**(storage->desc)).idSize			= sizeof(ImageDescription);
	(**(storage->desc)).cType			= kCompressionFormat;
	(**(storage->desc)).resvd1			= 0;
	(**(storage->desc)).resvd2			= 0;
	(**(storage->desc)).dataRefIndex	= 0;
	(**(storage->desc)).version			= 2;
	(**(storage->desc)).revisionLevel	= 0;
	(**(storage->desc)).vendor			= 'dts ';
	(**(storage->desc)).temporalQuality	= 0;            // not used
	(**(storage->desc)).spatialQuality	= codecLosslessQuality;
	(**(storage->desc)).width			= 0;			// we fill in size later
	(**(storage->desc)).height			= 0;
	(**(storage->desc)).hRes			= 72 << 16;		// not used, set to 72dpi
	(**(storage->desc)).vRes			= 72 << 16;
	(**(storage->desc)).dataSize		= 0;
	(**(storage->desc)).frameCount		= 1;
	(**(storage->desc)).depth			= 24;           
	(**(storage->desc)).clutID			= -1;			// no clut
	
	err = GetComponentIndString((Component)storage->self, (*(storage->desc))->name, 200, 2);
	if (err) goto bail;
    
    // Add the ImageDescription properties
    
    if (1 == (**(storage->desc)).version && (**(storage->desc)).revisionLevel == 1) {
     // The 'colr' extension supersedes the previously defined 'gama' ImageDescription extension.
     // Writers of QuickTime files should never write both into an ImageDescription.
     // Readers of QuickTime files should ignore 'gama' if 'colr' is present.
     
        Fixed gamma = FixRatio(22, 10);
        
        err = ICMImageDescriptionSetProperty(storage->desc, kQTPropertyClass_ImageDescription,
                                            kICMImageDescriptionPropertyID_GammaLevel,
                                            sizeof(Fixed),
                                            &gamma);
        if (err) goto bail;
    }
    
    if (2 == (**(storage->desc)).version) {
        // NOTE: these values may vary depending on your specific format - consult Ice Floe 19
   
        // Field property
        /*
         * Information about the number and order of fields, if available.
         */
        FieldInfoImageDescriptionExtension2 fieldInfo;
        
        // http://developer.apple.com/quicktime/icefloe/dispatch019.html#fiel
        fieldInfo.fields = kQTFieldsProgressiveScan;
        fieldInfo.detail = kQTFieldDetailUnknown;

        err = ICMImageDescriptionSetProperty(storage->desc, kQTPropertyClass_ImageDescription,
                                            kICMImageDescriptionPropertyID_FieldInfo,
                                            sizeof(FieldInfoImageDescriptionExtension2),
                                            &fieldInfo);
        if (err) goto bail;
                                             
        CleanApertureImageDescriptionExtension cleanAperture;
        PixelAspectRatioImageDescriptionExtension pixelAspectRatio;

        // http://developer.apple.com/quicktime/icefloe/dispatch019.html#clap
        cleanAperture.cleanApertureWidthN   = 320;
        cleanAperture.cleanApertureWidthD   = 1;
        cleanAperture.cleanApertureHeightN  = 240;
        cleanAperture.cleanApertureHeightD  = 1;
        cleanAperture.horizOffN             = 0;
        cleanAperture.horizOffD             = 1;
        cleanAperture.vertOffN              = 0;
        cleanAperture.vertOffD              = 1;

        // http://developer.apple.com/quicktime/icefloe/dispatch019.html#pasp
        pixelAspectRatio.hSpacing       = 1;
        pixelAspectRatio.vSpacing       = 1;

        // Add the ImageDescription properties

        // Clean Aperture
        /*
         * Describes the clean aperture of the buffer.
         */
        err = ICMImageDescriptionSetProperty(storage->desc, kQTPropertyClass_ImageDescription,
                                             kICMImageDescriptionPropertyID_CleanAperture,
                                             sizeof(CleanApertureImageDescriptionExtension),
                                             &cleanAperture);
        if (err) goto bail;
        
        // Pixel Aspect Ratio
        /*
         * Describes the pixel aspect ratio.
         */
        err = ICMImageDescriptionSetProperty(storage->desc, kQTPropertyClass_ImageDescription,
                                             kICMImageDescriptionPropertyID_PixelAspectRatio,
                                             sizeof(PixelAspectRatioImageDescriptionExtension),
                                             &pixelAspectRatio);
        if (err) goto bail;
                                             
        NCLCColorInfoImageDescriptionExtension colorInfo;
        
        // http://developer.apple.com/quicktime/icefloe/dispatch019.html#colr                                     
        colorInfo.colorParamType			= kVideoColorInfoImageDescriptionExtensionType;
        colorInfo.primaries					= kQTPrimaries_SMPTE_C;
        colorInfo.transferFunction			= kQTTransferFunction_ITU_R709_2;
        colorInfo.matrix					= kQTMatrix_ITU_R_601_4;
        
        // Color Info
        /*
         * Color information, if available in the
         * NCLCColorInfoImageDescriptionExtension format.
         */
        err = ICMImageDescriptionSetProperty(storage->desc, kQTPropertyClass_ImageDescription,
                                             kICMImageDescriptionPropertyID_NCLCColorInfo,
                                             sizeof(NCLCColorInfoImageDescriptionExtension),
                                             &colorInfo);
        if (err) goto bail;
    }
        
    // initialize some of the global storage members
    err = SoftVDigXGetMaxSrcRect(storage, ntscIn, &(storage->maxSrcRect));
    if (err) goto bail;
    
    storage->digiRect = storage->maxSrcRect;

    err = SoftVDigXGetVideoDefaults(storage,
                                    &(storage->blackLevel),
                                    &(storage->whiteLevel),
                                    &(storage->brightness),
                                    &(storage->hue),
                                    &(storage->saturation),
                                    &(storage->contrast),
                                    &(storage->sharpness));

bail:
	return err;
}

// Component Close Request - Required
pascal ComponentResult SoftVDigXClose(SoftVDigXGlobalsPtr storage, ComponentInstance self)
{
#pragma unused(self)

	// clean up
    if (storage) {
		if (storage->framePtr) {
			// if we mapped in some memory (in ConnectToKEXT) from the kernel, unmap it here
			IOConnectUnmapMemory(storage->userClient, 1, mach_task_self(), storage->framePtr);
		}
		
		// if we allocated memory for an image description, get rid of it
		if (storage->desc) DisposeHandle((Handle)storage->desc);
	
		// dispose of storage last
        DisposePtr((Ptr)storage);
    }

    return noErr;
}

// Component Version Request - Required
pascal ComponentResult SoftVDigXVersion(SoftVDigXGlobalsPtr storage)
{
#pragma unused(store)

	return (vdigInterfaceRev << 16) | kComponentVersion;
}

/************************************************************************************/
#pragma mark---- Video Digitizer Calls

/* Setting Source Characteristics

This section discusses the video digitizer component functions that allow applications to set the spatial
characteristics of the source video signal. Applications use these functions to set and retrieve
information about the maximum source rectangle, the active source rectangle, the vertical blanking rectangle,
and the digitizer rectangle. You can use the VDGetMaxSrcRect function in your application to get the size and
location of the maximum source rectangle. Similarly, the VDGetActiveSrcRect function allows you to get this information
about the active source rectangle, and the VDGetVBlankRect function enables you to obtain information about the vertical
blanking rectangle. You can use the VDSetDigitizerRect  function to set the size and location of the digitizer rectangle.
The VDGetDigitizerRect function lets you retrieve the size and location of this rectangle. */

// VDGetMaxSrcRect - All video digitizer components must support this function.
// 		Returns the maximum source rectangle.
pascal VideoDigitizerError SoftVDigXGetMaxSrcRect(SoftVDigXGlobalsPtr storage, short inputStd, Rect *maxSrcRect)
{
	// we only support NTSC
	if (inputStd != ntscIn) {
		DebugStr("\pSoftVDigXGetActiveSrcRect - returned qtParamErr");
		return qtParamErr;
	}

	GetMaxSrcRectFromKEXT(storage, maxSrcRect); // ask the 'hardware' for it's maximum source rectangle
	storage->maxSrcRect = *maxSrcRect; 			// remember it

    return noErr;
}

// VDGetActiveSrcRect - All video digitizer components must support this function.
// 		Return the size and location information for the active source rectangle.
pascal VideoDigitizerError SoftVDigXGetActiveSrcRect(SoftVDigXGlobalsPtr storage, short inputStd, Rect *activeSrcRect)
{
	// we only support NTSC
    if (inputStd != ntscIn) {
		DebugStr("\pSoftVDigXGetActiveSrcRect - returned qtParamErr");
		return qtParamErr;
	}
	if (EmptyRect(&storage->maxSrcRect)) return badCallOrderErr;
	
	// our active source rectangle is the same as our maximum source rectangle, yours may be different
	*activeSrcRect = storage->maxSrcRect;

    return noErr;
}

// VDSetDigitizerRect - All video digitizer components must support this function.
//		Sets the current video digitizer rectangle.
pascal VideoDigitizerError SoftVDigXSetDigitizerRect(SoftVDigXGlobalsPtr storage, Rect *digiRect)
{
    Rect tempR;

    // can't be empty
    if (!digiRect || EmptyRect(digiRect)) return qtParamErr;

    // they must intersect
    if (!SectRect(digiRect, &(storage->maxSrcRect), &tempR)) return qtParamErr;

    // completely...
    if (!MacEqualRect(digiRect, &tempR)) return qtParamErr;

    storage->digiRect = *digiRect;

    return noErr;
}

// VDGetDigitizerRec - All video digitizer components must support this function.
// 		Returns the current digitizer rectangle.
pascal VideoDigitizerError SoftVDigXGetDigitizerRect(SoftVDigXGlobalsPtr storage, Rect *digiRect)
{
	*digiRect = storage->digiRect;

	return noErr;
}

// VDGetVBlankRect - All video digitizer components must support this function.
// 		Returns the vertical blanking rectangle.
pascal VideoDigitizerError SoftVDigXGetVBlankRect(SoftVDigXGlobalsPtr storage, short inputStd, Rect *vBlankRect)
{
	Rect rEmpty = { 0 };
	
	// we have no vertical blanking so we return an empty rectangle
	// if your hardware's maximum source rectangle includes vertical blanking, you'll want
	// to return an appropriate rectangle here
	*vBlankRect = rEmpty;
	
	return noErr;
}

/************************************************************************************/
#pragma mark-

/* Controlling Analog Video

Some video digitizer components may provide functions that allow applications to control the characteristics of the input analog video signal.
The VDGetVideoDefaults function returns the suggested default values for the analog video parameters that can be affected by these functions.
A number of functions affect gamma correction. The VDSetInputGammaRecord and VDGetInputGammaRecord functions work with gamma structures.
You can use the VDSetInputGammaValue and VDGetInputGammaValue functions to allow your application to set particular gamma values.
The VDSetBlackLevelValue, VDGetBlackLevelValue , VDSetWhiteLevelValue, and VDGetWhiteLevelValue functions allow applications to work with black
levels and white levels in the source video. Black level  refers to the degree of blackness in an image. This is a common setting on a video digitizer.
The highest setting produces an all-black image; on the other hand, the lowest setting yields little, if any, black even with black objects in the scene.
Black level is a significant setting because it can be adjusted so that there is little or no noise in an image. White level  refers to the degree
of whiteness in an image. It is also a common video digitizer setting. The VDSetContrast, VDGetContrast, VDSetSharpness, and VDGetSharpness functions
allow applications to work with contrast and sharpness values in the source video. The VDGetBrightness and VDSetBrightness functions allow applications
to work with the image brightness setting. The VDSetHue, VDGetHue, VDSetSaturation, and VDGetSaturation functions allow applications to work with hue
and saturation settings in the source video. */

pascal VideoDigitizerError SoftVDigXSetBrightness(SoftVDigXGlobalsPtr storage, unsigned short *brightness)
{
	storage->brightness = *brightness;
	return noErr;
}
pascal VideoDigitizerError SoftVDigXGetBrightness(SoftVDigXGlobalsPtr storage, unsigned short *brightness)
{
	*brightness = storage->brightness;
	return noErr;
}

pascal VideoDigitizerError SoftVDigXSetContrast(SoftVDigXGlobalsPtr storage, unsigned short *contrast)
{
	storage->contrast = *contrast;
	return noErr;
}

pascal VideoDigitizerError SoftVDigXGetContrast(SoftVDigXGlobalsPtr storage, unsigned short *contrast)
{
	*contrast = storage->contrast;
	return noErr;
}

pascal VideoDigitizerError SoftVDigXSetHue(SoftVDigXGlobalsPtr storage, unsigned short *hue)
{
	storage->hue = *hue;
	return noErr;
}

pascal VideoDigitizerError SoftVDigXGetHue(SoftVDigXGlobalsPtr storage, unsigned short *hue)
{
	*hue = storage->hue;
	return noErr;
}

pascal VideoDigitizerError SoftVDigXSetSaturation(SoftVDigXGlobalsPtr storage, unsigned short *saturation)
{
	storage->saturation = *saturation;
	return noErr;
}

pascal VideoDigitizerError SoftVDigXGetSaturation(SoftVDigXGlobalsPtr storage, unsigned short *saturation)
{
	*saturation = storage->saturation;
	return noErr;
}

pascal VideoDigitizerError SoftVDigXSetSharpness(SoftVDigXGlobalsPtr storage, unsigned short *sharpness)
{
	storage->sharpness = *sharpness;
	return noErr;
}

pascal VideoDigitizerError SoftVDigXGetSharpness(SoftVDigXGlobalsPtr storage, unsigned short *sharpness)
{
	*sharpness = storage->sharpness;
	return noErr;
}

pascal VideoDigitizerError SoftVDigXSetBlackLevelValue(SoftVDigXGlobalsPtr storage, unsigned short *blackLevel)
{
	storage->blackLevel = *blackLevel;
	return noErr;
}

pascal VideoDigitizerError SoftVDigXGetBlackLevelValue(SoftVDigXGlobalsPtr storage, unsigned short *blackLevel)
{
	*blackLevel = storage->blackLevel;
	return noErr;
}

pascal VideoDigitizerError SoftVDigXSetWhiteLevelValue(SoftVDigXGlobalsPtr storage, unsigned short *whiteLevel)
{
	storage->whiteLevel = *whiteLevel;
	return noErr;
}

pascal VideoDigitizerError SoftVDigXGetWhiteLevelValue(SoftVDigXGlobalsPtr storage, unsigned short *whiteLevel)
{
	*whiteLevel = storage->whiteLevel;
	return noErr;
}

// VDGetVideoDefaults - All video digitizer components must support this function.
// 		Returns the recommended values for many of the analog video parameters that may be set by applications.
/* blackLevel - A pointer to an integer that is to receive the default black level value. Black level values range from 0 to 65,535,
				where 0 represents the maximum black value and 65,535 represents the minimum black value.
	whiteLevel - A pointer to an integer that is to receive the default white level value. White level values range from 0 to 65,535,
				where 0 represents the minimum white value and 65,535 represents the maximum white value.
	brightness - A pointer to an integer that is to receive the default brightness value. Brightness values range from 0 to 65,535,
				where 0 is the darkest possible setting and 65,535 is the lightest possible setting.
	hue - A pointer to an integer that is to receive the default hue value. Hue is similar to the tint control on a television, and
		  it is specified in degrees with complementary colors set 180 degrees apart (red is 0 degrees, green is +120 degrees, and blue
		  is -120 degrees). Video digitizer components support hue values that range from 0 (-180 degrees shift in hue) to 65,535
		  (+179 degrees shift in hue), where 32,767 represents a 0 degree shift in hue.
	saturation - A pointer to an integer that is to receive the default saturation value. The saturation value controls color intensity.
				 For example, at high saturation levels, red appears to be red; at low saturation, red appears as pink. Valid saturation
				 values range from 0 to 65,535, where 0 is the minimum saturation value and 65,535 specifies maximum saturation.
	contrast - A pointer to an integer that is to receive the default contrast value. The contrast value ranges from 0 to 65,535, where 0
			   represents no change to the basic image and larger values increase the contrast of the video image (they increase the slope of the transform).
	sharpness - A pointer to an integer that is to receive the default sharpness value. The sharpness value ranges from 0 to 65,535, where 0
				represents no sharpness filtering and 65,535 represents full sharpness filtering. Higher values result in a visual impression of
				increased picture sharpness.
*/
#define RANGE_FACTOR 655.35 // really just for debugging, allows seting a "percent" value to check in the setting dialog, we don't do anything with these values
pascal VideoDigitizerError SoftVDigXGetVideoDefaults(SoftVDigXGlobalsPtr storage,
													  unsigned short *blackLevel,
													  unsigned short *whiteLevel,
													  unsigned short *brightness,
													  unsigned short *hue,
													  unsigned short *saturation,
													  unsigned short *contrast,
												      unsigned short *sharpness)
{
	*blackLevel = 0;
	*whiteLevel = 90 * RANGE_FACTOR;
	*brightness = 50 * RANGE_FACTOR; // 32767
	*hue 		= 48 * RANGE_FACTOR;
	*saturation = 54 * RANGE_FACTOR;
	*contrast 	= 88 * RANGE_FACTOR;
	*sharpness 	= 98 * RANGE_FACTOR;

	return noErr;
}

/************************************************************************************/
#pragma mark-

/* Getting Information About Video Digitizer Components

These functions allow applications to obtain information about the capabilities
and current state of video digitizer components. Applications can use the VDGetDigitizerInfo function
to retrieve information about the capabilities of a video digitizer component and can use the
VDGetCurrentFlags function to obtain current status information from a video digitizer component. */

// VDGetDigitizerInfo - All video digitizer components must support this function.
//		Returns capability and status information about a specified video digitizer component.
// In response to this call the component formats a DigitizerInfo structure. The contents of this
// structure fully define the capabilities and current status of the video digitizer component.
// DigitizerInfo - Contains information about the capabilities and current status of a video digitizer component.
/*
  struct DigitizerInfo {
     short       vdigType;
     long        inputCapabilityFlags;
     long        outputCapabilityFlags;
     long        inputCurrentFlags;
     long        outputCurrentFlags;
     short       slot;
     GDHandle    gdh;
     GDHandle    maskgdh;
     short       minDestHeight;
     short       minDestWidth;
     short       maxDestHeight;
     short       maxDestWidth;
     short       blendLevels;
     long        reserved;
 };

	vdigType - Constant (see below) that specifies the type of video digitizer component.
	inputCapabilityFlags - Constant (see below) that specifies the capabilities of the video digitizer component with respect
						   to the input video signal.
	outputCapabilityFlags - Constant (see below) that specifies the capabilities of the video digitizer component with respect
							to the output digitized video information.
	inputCurrentFlags - Specifies the current status of the video digitizer with respect to the input video signal. Video digitizer
						components report their current input status by returning a flags field that contains 1 bit for each of the
						applicable inputCapabilityFlags constants (see below), plus additional inputCurrentFlags constants (see below)
						as appropriate. The digitizer component sets these flags to reflect its current status. When reporting input
						status, for example, a video digitizer component sets the digiInDoesGenLock flag to 1 whenever the digitizer
						component is deriving its time signal from the input video. When reporting its input capabilities, the digitizer
						component sets this flag to 1 to indicate that it can derive its timing from the input video.
	outputCurrentFlags - Specifies the current status of the video digitizer with respect to the output video signal. Video digitizer
						 components report their current output status by returning a flags field that contains 1 bit for each of the
						 applicable outputCapabilityFlags constants (see below)
	slot - Identifies the slot that contains the video digitizer interface card.
	gdh - Contains a handle to the graphics device that defines the screen to which the digitized data is to be written. Set this field
		  to NULL if your application is not constrained to a particular graphics device.
	maskgdh - Contains a handle to the graphics device that contains the mask plane. This field is used only by digitizers that clip
			  by means of mask planes.
	minDestHeight - Indicates the smallest height value the digitizer component can accommodate in its destination.
	minDestWidth - Indicates the smallest width value the digitizer component can accommodate in its destination.
	maxDestHeight - Indicates the largest height value the digitizer component can accommodate in its destination.
	maxDestWidth - Indicates the largest width value the digitizer component can accommodate in its destination.
	blendLevels - Specifies the number of blend levels the video digitizer component supports.
	reserved - Reserved. Set this field to 0.

vdigType Constants:
	vdTypeBasic - Basic video digitizer; does not support any clipping.
	vdTypeAlpha - Supports clipping by means of an alpha channel.
	vdTypeMask - Supports clipping by means of a mask plane.
	vdTypeKey - Supports clipping by means of key colors.

inputCapabilityFlags Constants:
	digiInDoesNTSC - Indicates that the video digitizer supports National Television System Committee (NTSC) format input video signals.
					 This flag is set to 1 if the digitizer component supports NTSC video.
	digiInDoesPAL - Indicates that the video digitizer component supports Phase Alternation Line (PAL) format input video signals. This
					flag is set to 1 if the digitizer component supports PAL video.
	digiInDoesSECAM  - Indicates that the video digitizer component supports Systeme Electronique Couleur avec Memoire (SECAM) format
					   input video signals. This flag is set to 1 if the digitizer component supports SECAM video.
	digiInDoesGenLock - Indicates that the video digitizer component supports genlock; that is, the digitizer can derive its timing from
						an external time base. This flag is set to 1 if the digitizer component supports genlock.
	digiInDoesComposite - Indicates that the video digitizer component supports composite input video. This flag is set to 1 if the
						  digitizer component supports composite input.
	digitInDoesSVideo - Indicates that the video digitizer component supports s-video input video. This flag is set to 1 if the digitizer
						component supports s-video input.
	digiInDoesComponent - Indicates that the video digitizer component supports RGB input video. This flag is set to 1 if the digitizer
						  component supports RGB input.
	digiInVTR_Broadcast - Indicates that the video digitizer component can distinguish between an input signal that emanates from a
						  videotape player and a broadcast signal. This flag is set to 1 if the digitizer component can differentiate between
						  the two different signal types.
	digiInDoesColor - Indicates that the video digitizer component supports color input. This flag is set to 1 if the digitizer component can
					  accept color input.
	digiInDoesBW - Indicates that the video digitizer component supports grayscale input. This flag is set to 1 if the digitizer component can
				   accept grayscale input.

outputCapabilityFlags Constants:
	digiOutDoes1 - Indicates that the video digitizer component can work with pixel maps that contain 1-bit pixels. If this flag is set to 1,
				   then the digitizer component can write images that contain 1-bit pixels. If this flag is set to 0, then the digitizer component
				   cannot handle such images.
	digiOutDoes2 - Indicates that the video digitizer component can work with pixel maps that contain 2-bit pixels. If this flag is set to 1, then
				   the digitizer component can write images that contain 2-bit pixels. If this flag is set to 0, then the digitizer component cannot
				   handle such images.
	digiOutDoes4 - Indicates that the video digitizer component can work with pixel maps that contain 4-bit pixels. If this flag is set to 1, then the
				   digitizer component can write images that contain 4-bit pixels. If this flag is set to 0, then the digitizer component cannot handle
				   such images.
	digiOutDoes8 - Indicates that the video digitizer component can work with pixel maps that contain 8-bit pixels. If this flag is set to 1, then the
				   digitizer component can write images that contain 8-bit pixels. If this flag is set to 0, then the digitizer component cannot handle
				   such images.
	digiOutDoes16 - Indicates that the video digitizer component can work with pixel maps that contain 16-bit pixels. If this flag is set to 1, then the
					digitizer component can write images that contain 16-bit pixels. If this flag is set to 0, then the digitizer component cannot handle
					such images.
	digiOutDoes32 - Indicates that the video digitizer component can work with pixel maps that contain 32-bit pixels. If this flag is set to 1, then the
					digitizer component can write images that contain 32-bit pixels. If this flag is set to 0, then the digitizer component cannot handle
					such images.
	digiOutDoesDither - Indicates that the video digitizer component supports dithering. If this flag is set to 1, the component supports dithering of colors.
						If this flag is set to 0, the digitizer component does not support dithering.
	digiOutDoesStretch - Indicates that the video digitizer component can stretch images to arbitrary sizes. If this flag is set to 1, the digitizer component
						 can stretch images. If this flag is set to 0, the digitizer component does not support stretching.
	digiOutDoesShrink - Indicates that the video digitizer component can shrink images to arbitrary sizes. If this flag is set to 1, the digitizer component can
						shrink images. If this flag is set to 0, the digitizer component does not support shrinking.
	digiOutDoesMask - Indicates that the video digitizer component can handle clipping regions. If this flag is set to 1, the digitizer component can mask to an
					  arbitrary clipping region. If this flag is set to 0, the digitizer component does not support clipping regions.
	digiOutDoesDouble - Indicates that the video digitizer component supports stretching to quadruple size when displaying the output video. The parameters for
						the stretch operation are specified in the matrix structure for the request; the component modifies the scaling attributes of the matrix.
						If this flag is set to 1, the digitizer component can stretch an image to exactly four times its original size, up to the maximum size
						specified by the maxDestHeight and maxDestWidth fields. If this flag is set to 0, the digitizer component does not support stretching to
						quadruple size.
	digiOutDoesQuad - Indicates that the video digitizer component supports stretching an image to 16 times its original size when displaying the output video.
					  The parameters for the stretch operation are specified in the matrix structure for the request; the component modifies the scaling attributes
					  of the matrix. If this flag is set to 1, the digitizer component can stretch an image to exactly 16 times its original size, up to the maximum
					  size specified by the maxDestHeight and maxDestWidth fields. If this flag is set to 0, the digitizer component does not support this capability.
	digiOutDoesQuarter - Indicates that the video digitizer component can shrink an image to one-quarter of its original size when displaying the output video. The
						 parameters for the shrink operation are specified in the matrix structure for the request; the component modifies the scaling attributes of
						 the matrix. If this flag is set to 1, the digitizer component can shrink an image to exactly one-quarter of its original size, down to the
						 minimum size specified by the minDestHeight and minDestWidth fields. If this flag is set to 0, the digitizer component does not support this
						 capability.
	digiOutDoesSixteenth - Indicates that the video digitizer component can shrink an image to 1/16 of its original size when displaying the output video. The
						   parameters for the shrink operation are specified in the matrix structure for the request; the digitizer component modifies the scaling
						   attributes of the matrix. If this flag is set to 1, the digitizer component can shrink an image to exactly 1/16 of its original size,
						   down to the minimum size specified by the minDestHeight and minDestWidth fields. If this flag is set to 0, the digitizer component does
						   not support this capability.
	digiOutDoesRotate - Indicates that the video digitizer component can rotate an image when displaying the output video. The parameters for the rotation are
						specified in the matrix structure for an operation. If this flag is set to 1, the digitizer component can rotate the image. If this flag
						is set to 0, the digitizer component cannot rotate the resulting image.
	digiOutDoesHorizFlip - Indicates that the video digitizer component can flip an image horizontally when displaying the output video. The parameters for the
						   horizontal flip are specified in the matrix structure for an operation. If this flag is set to 1, the digitizer component can flip the
						   image. If this flag is set to 0, the digitizer component cannot flip the resulting image.
	digiOutDoesVertFlip - Indicates that the video digitizer component can flip an image vertically when displaying the output video. The parameters for the
						  vertical flip are specified in the matrix structure for an operation. If this flag is set to 1, the digitizer component can flip the
						  image. If this flag is set to 0, the digitizer component cannot flip the resulting image.
	digiOutDoesSkew - Indicates that the video digitizer component can skew an image when displaying the output video. Skewing an image distorts it linearly along
					  only a single axis; for example, drawing a rectangular image into a parallelogram-shaped region. The parameters for the skew operation are
					  specified in the matrix structure for the request. If this flag is set to 1, the digitizer component can skew an image. If this flag is set
					  to 0, the digitizer component does not support this capability.
	digiOutDoesBlend - Indicates that the video digitizer component can blend the resulting image with a matte when displaying the output video. The matte is
					   provided by the application by defining either an alpha channel or a mask plane. If this flag is set to 1, the digitizer component can blend.
					   If this flag is set to 0, the digitizer component does not support this capability.
	digiOutDoesWarp - Indicates that the video digitizer component can warp an image when displaying the output video. Warping an image distorts it along one or
					  more axes, perhaps nonlinearly, in effect "bending" the result region. The parameters for the warp operation are specified in the matrix
					  structure for the request. If this flag is set to 1, the digitizer component can warp an image. If this flag is set to 0, the digitizer
					  component does not support this capability.
	digiOutDoesDMA - Indicates that the video digitizer component can write to any screen or to offscreen memory. If this flag is set to 1, the digitizer component
					 can use DMA to write to any screen or memory location.
	digiOutDoesHWPlayThru - Indicates that the video digitizer component does not need idle time in order to display its video. If this flag is set to 1, your
							application does not need to grant processor time to the digitizer component at normal display speeds.
	digiOutDoesILUT - Indicates that the video digitizer component supports inverse lookup tables for indexed color modes. If this flag is set to 1, the digitizer
					  component uses inverse lookup tables when appropriate.
	digiOutDoesKeyColor - Indicates that the video digitizer component supports clipping by means of key colors. If this flag is set to 1, the digitizer
						  component can clip to a region defined by a key color.
	digiOutDoesAsyncGrabs - Indicates that the video digitizer component can operate asynchronously. If this flag is set to 1, your digitizer implements
							VDSetupBuffers and VDGrabOneFrameAsync.
	digiOutDoesUnreadableScreenBits - Indicates that the video digitizer may place pixels on the screen that cannot be used when compressing images.
									  In other words, image data can't be read directly from the screen as source material for the compression phase.
	digiOutDoesCompress - Indicates that the video digitizer component supports compressed source devices. These devices provide compressed data directly, without
						  having to use the Image Compression Manager.
	digiOutDoesCompressOnly - Indicates that the video digitizer component only provides compressed image data; the component cannot provide displayable data.
							  This flag only applies to digitizers that support compressed source devices.
	digiOutDoesPlayThruDuringCompress - Indicates that the video digitizer component can draw images on the screen at the same time that it is delivering
										compressed image data. This flag only applies to digitizers that support compressed source devices.
	digiOutDoesCompressPartiallyVisible - Indicates that the video digitizer component doesn't need all bits visible on screen to do hardware compress.
	digiOutDoesNotNeedCopyOfCompressData - Indicates that the video digitizer component doesn't need any bufferization when providing compressed data.
										   (Mostly used if VDCompressDone returns a buffer address not in host memory)

inputCurrentFlags Constants - these are valid only during active operating conditions:
	digiInSignalLock - Indicates that the video digitizer component is locked onto the input signal. If this flag is set to 1, the digitizer component detects
					   either vertical or horizontal signal lock. This bit = horiz lock || vertical lock.
*/
pascal VideoDigitizerError SoftVDigXGetDigitizerInfo(SoftVDigXGlobalsPtr storage, DigitizerInfo *info)
{
	// fill in DigitizerInfo struct
	info->slot 					= 0;	// no longer used
	info->gdh 					= 0;	// not used
	info->maskgdh 				= 0;	// not used
	info->minDestHeight 		= 0;//(storage->maxSrcRect).bottom;	// our minimum size is equivalent to the maximum size
	info->minDestWidth 			= 0;//(storage->maxSrcRect).right;
	info->maxDestHeight 		= (storage->maxSrcRect).bottom;
	info->maxDestWidth 			= (storage->maxSrcRect).right;
	info->blendLevels 			= 0;	// not used
	info->vdigType 				= vdTypeBasic;	// basic video digitizer; does not support any clipping.
		
	// zero all the bits in the flag fields
	info->inputCapabilityFlags	= 0;
	info->outputCapabilityFlags = 0;
	info->inputCurrentFlags		= 0;
	info->outputCurrentFlags	= 0;

	// set the input capabilities flags	- see above	
	info->inputCapabilityFlags = digiInDoesNTSC | digiInDoesColor | digiInDoesComponent;
	info->outputCapabilityFlags = digiOutDoes32       		|
								  digiOutDoesCompress		|
								  digiOutDoesCompressOnly	|
								  digiOutDoesNotNeedCopyOfCompressData; // send any frame we return to the disk directly without QT trying to interleave audio and video into an intermediate buffer
                                  	
	info->inputCurrentFlags = info->inputCapabilityFlags;
	info->inputCurrentFlags |= digiInSignalLock;
	
	info->outputCurrentFlags = info->outputCapabilityFlags;
		
	// save them in our storage
    storage->inputCurrentFlags  = info->inputCurrentFlags;
    storage->outputCurrentFlags = info->outputCurrentFlags;
	
    return noErr;
}

// VDGetCurrentFlags
// 		Returns status information about a specified video digitizer component.
// This function is often more convenient than VDGetDigitizerInfo (III-2136). For example, this function provides a
// simple mechanism for determining whether a video digitizer is receiving a valid input signal. An application can
// retrieve the current input state flags and test the high-order bit by examining the sign of the returned value.
// If the value is negative (that is, the high-order bit, digiInSignalLock, is set to 1), the digitizer component
// is receiving a valid input signal.
pascal VideoDigitizerError SoftVDigXGetCurrentFlags(SoftVDigXGlobalsPtr storage, long *inputCurrentFlag, long *outputCurrentFlag) 
{
	*inputCurrentFlag = storage->inputCurrentFlags;
	*outputCurrentFlag = storage->outputCurrentFlags;
	
	return noErr;
}

/************************************************************************************/
#pragma mark-

/* Selecting an Input Source

	This section discusses the video digitizer component functions that allow applications to select
an input video source. Some of these functions provide information about the available video inputs.
Applications can use the VDGetNumberOfInputs function to determine the number of video inputs supported
by the digitizer component. The VDGetInputFormat  function allows applications to find out the video format
(composite, s-video, or component) employed by a specified input. You can use the VDSetInput function in
your application to specify the input to be used by the digitizer component. The VDGetInput function returns
the currently selected input. The VDSetInputStandard  function allows you to specify the video signaling standard
to be used by the video digitizer component. */

// VDGetNumberOfInputs - All video digitizer components must support this function.
//		Returns the number of input video sources that a video digitizer component supports.
// Video digitizer components number video sources sequentially, starting at 0. So, if a digitizer
// component supports two inputs, this function sets the field referred to by the inputs parameter to 1.		
pascal VideoDigitizerError SoftVDigXGetNumberOfInputs(SoftVDigXGlobalsPtr storage, short *inputs)
{
	*inputs = 0;	// 1 input

	return noErr;
}

// VDGetInputFormat - All video digitizer components must support this function.
//		Determines the format of the video signal provided by a specified video input source.
// Video digitizer components number video sources sequentially, starting at 0. So, to request
// information about the first video source, an application sets this parameter to 0.
/*
format Constants:
	compositeIn - The input video signal is in composite format.
	sVideoIn - The input video signal is in s-video format.
	rgbComponentIn - The input video signal is in RGB component format.
	rgbComponentSyncIn - The input is rgb component format (sync on green?).
	yuvComponentIn - The input is yuv component format.
	yuvComponentSyncIn The input is yuv component format (sync on green?).
	tvTunerIn - The input video signal from an RF tuner.
	sdiIn - The input is from a Serial Digital Interface, a SMPTE standard for digtial transmission in studios.
*/
pascal VideoDigitizerError SoftVDigXGetInputFormat(SoftVDigXGlobalsPtr storage, short input, short *format)
{
	if (input != 0 ) return qtParamErr;
	
	// hard code to yuvComponentIn - real name will be returned by GetInputName()
	*format = yuvComponentIn;
	
	return noErr;
}

// VDSetInput - All video digitizer components must support this function.
// 		Selects the input video source for a video digitizer component.
pascal VideoDigitizerError SoftVDigXSetInput(SoftVDigXGlobalsPtr storage, short input)
{
	if (input == 0) return noErr; // we only support 1 input

	return qtParamErr;
}

// VDGetInput - All video digitizer components must support this function.
//		Returns data that identifies the currently active input video source.
pascal VideoDigitizerError SoftVDigXGetInput(SoftVDigXGlobalsPtr storage, short *input)
{
	*input = 0;

	return noErr;
}

// VDSetInputStandard - All video digitizer components must support this function.
//		Specifies the input signaling standard to digitize.
/*
inputStandard Constants:
  ntscIn - current input format
  currentIn - ntsc input format
  palIn - pal input format
  secamIn - secam input format
  ntscReallyIn - ntsc input format, specifically means NTSC instead of "default"
*/
pascal VideoDigitizerError SoftVDigXSetInputStandard(SoftVDigXGlobalsPtr storage, short inputStandard)
{
	// none of the input standards are truly applicable, we only allow NTSC
	if (ntscIn == inputStandard || ntscReallyIn == inputStandard)
		return noErr;
	else {
		DebugStr( "\pSoftVDigXSetInputStandard - returned qtParamErr" );
		return qtParamErr;
	}
}

// VDGetInputName
//		Gets the name of a video input.
pascal VideoDigitizerError SoftVDigXGetInputName(SoftVDigXGlobalsPtr storage, long videoInput, Str255 name)
{
	if (videoInput != 0) {
		DebugStr( "\pSoftVDigXGetInputName - returned qtParamErr" );
		return qtParamErr;
	}
	
	c2pstrcpy(name, storage->deviceName);

	return noErr;
}

// VDGetDeviceNameAndFlags
//		Returns the name of the input device.
/*
This routine is designed to give the VDIG more control over how it is presented to the user, and to clarify the distinction between
devices and inputs. Historically, the assumption has been that there is one component registered per device and the component name is
displayed. This change lets a component choose its name after registration. vdDeviceFlagShowInputsAsDevices is meant for components
that register once and support multiple devices. The User Interface is clearer if these are presented as devices rather than inputs,
and this allows a VDIG to present itself this way without a huge restructuring. vdDeviceFlagHideDevice is for the kind of VDIG that
registers itself, and then can register a further VDIG for each device. If no hardware is available, returning this flag will omit it
from the list. As this call is being made, it is also a good time to check for hardware and register further VDIG components if needed,
allowing for lazy initialization when the application needs to find a VDIG rather than on every launch or replug.

   vdDeviceFlagShowInputsAsDevices - Tell the Panel to promote Inputs to Devices
   vdDeviceFlagHideDevice - Omit this Device enitirely from the list
*/
pascal VideoDigitizerError SoftVDigXGetDeviceNameAndFlags(SoftVDigXGlobalsPtr storage, Str255 outName, UInt32 *outNameFlags)
{	
	OSErr err = GetComponentIndString((Component)storage->self, outName, 200, 1);
	if (err) return err;
	
	*outNameFlags = vdDeviceFlagShowInputsAsDevices;

	return noErr;
}

// VDGetUniqueIDs
//		Returns a unique identifier for a particular device, which, in the case of a FireWire device, is the FireWire ID.
/* These UniqueID calls are provided, so that the VDIG can give the sequence grabber information, enabling it to restore a
particular configuration, i.e., choose a particular device and input from those available��for example, if you need to restore 
a specific camera for a set of several hot-plugged FireWire cameras. The caller can pass NIL if it is not interested in one of
the IDs. Returning 0 in an ID means you don�t have one.
*/
pascal VideoDigitizerError SoftVDigXGetUniqueIDs(SoftVDigXGlobalsPtr storage, UInt64 *outDeviceID, UInt64 * outInputID)
{
	DebugStr( "\pSoftVDigXGetUniqueIDs" );
	return digiUnimpErr;
}

// VDSelectUniqueIDs
//		Selects a particular device��for example, the IDs from the previous call.
/* Note this is a Select, not a Set. The assumption is that the Unique ID is a function of the hardware and not modifiable by the
calling application. Either a NIL pointer or 0 in the ID means, don�t care. This should restore the device and input IDs returned
by VDGetUniqueIDs. Return vdDontHaveThatUniqueIDErr if your device doesn�t have a match.
*/
pascal VideoDigitizerError SoftVDigXSelectUniqueIDs(SoftVDigXGlobalsPtr storage, const UInt64 *inDeviceID, const UInt64 *inInputID)
{
	DebugStr( "\pSoftVDigXSelectUniqueIDs" );
	return digiUnimpErr;
}

/************************************************************************************/
#pragma mark-

/* Compressed Source Support

Some video digitizer components may provide functions that allow applications to work with digitizing
devices that can provide compressed image data directly. Such devices allow applications to retrieve
compressed image data without using the Image Compression Manager. However, in order to display images
from the compressed data stream, there must be an appropriate decompressor component available to decompress
the image data. Video digitizers that can support compressed source devices set the digiOutDoesCompress  flag to
1 in their capability flags (see "Capability Flags,"  for more information about these flags). Applications can
use the VDGetCompressionTypes  function to determine the image-compression capabilities of a video digitizer.
The VDSetCompression  function allows applications to set some parameters that govern image compression.
Applications control digitization by calling the VDCompressOneFrameAsync function, which instructs the video
digitizer to create one frame of compressed image data. The VDCompressDone function returns that frame. When an
application is done with a frame, it calls the VDReleaseCompressBuffer function to free the buffer. An application
can force the digitizer to place a key frame into the sequence by calling the VDResetCompressSequence function.
Applications can turn compression on and off by calling VDSetCompressionOnOff. Applications can obtain the
digitizer's image description structure by calling the VDGetImageDescription  function. Applications can set
the digitizer's time base by calling the VDSetTimeBase function. All of the digitizing functions described in
this section support only asynchronous digitization. That is, the video digitizer works independently to digitize
each frame. Applications are free to perform other work while the digitizer works on each frame. The video digitizer
component manages its own buffer pool for use with these functions. */

// VDSetCompression
// 		Specifies certain compression parameters.
/*
	compressType - A compressor type. This value corresponds to the component subtype of the compressor component; see "Codec Identifiers" (IV-2831).
	depth - The depth at which the image is likely to be viewed. Compressors may use this as an indication of the color or grayscale resolution
	        of the image. Values of 1, 2, 4, 8, 16, 24, and 32 indicate the number of bits per pixel for color images. Values of 33, 34, 36,
			and 40 correspond to 1-bit, 2-bit, 4-bit, and 8-bit grayscale images.
	bounds - A pointer to a Rect structure that defines the desired boundaries of the compressed image.
	spatialQuality - A constant (see below) that indicates the desired image quality for each frame in the sequence.
	temporalQuality - A constant (see below) that indicates the desired temporal quality for the sequence as a whole.
	keyFrameRate - The maximum number of frames to allow between key frames. This value defines the minimum rate at which key frames are to
	               appear in the compressed sequence; however, the video digitizer may insert key frames more often than an application specifies.
				   If the application requests no temporal compression (that is, the application set the temporalQuality parameter to 0), the video
				   digitizer ignores this parameter.

spatialQuality and temporalQuality Constants:
	codecMinQuality - The minimum valid value for a CodecQ field.
	codecLowQuality - Low-quality image reproduction. This value should correspond to the lowest image quality that still results in acceptable display characteristics.
	codecNormalQuality - Image reproduction of normal quality.
	codecHighQuality - High-quality image reproduction. This value should correspond to the highest image quality that can be achieved with reasonable performance.
	codecMaxQuality - The maximum standard value for a CodecQ field.
	codecLosslessQuality - Lossless compression or decompression. This special value is valid only for components that can support lossless compression or decompression.
*/
pascal VideoDigitizerError SoftVDigXSetCompression(SoftVDigXGlobalsPtr storage, OSType compressType,
																				short depth,
																				Rect *bounds,
																				CodecQ spatialQuality,
																				CodecQ temporalQuality,
																				long keyFrameRate)
{
	if (!storage->compressOn) return badCallOrderErr;
		
	// if we get to choose...
	if (compressType == 0) {
		compressType = kCompressionFormat; // the compression format set above
		spatialQuality = codecLosslessQuality;
		temporalQuality = 0;
		keyFrameRate = 0;
	}
    
	if(compressType != kCompressionFormat) {
		DebugStr( "\pSoftVDigXSetCompression - noCodecErr" );
		return noCodecErr;
	}
	
	storage->compressType = compressType;
	storage->compressDepth = depth ? depth : 24;
	storage->compressRect = *bounds;
	storage->spatialQuality = spatialQuality;
	storage->temporalQuality = temporalQuality;
	storage->keyFrameRate = keyFrameRate;

/*  This sample only supports a single capture size, however these comments are included here as a
	guideline to the type of operation you might perform if your hardware supports multiple capture
	sizes.
	
	Determine what portion of the image to capture. The 'bounds' provided is relative to the
	Digitizer Rectangle scaled and shifted to (0,0). The scaling factor needs to be determined so
	that the preferred bounding rectangle size can be choosen.
	
	Create a matrix that scales from the Digitizer Rect to the requested bounds.

	MatrixRecord	digitizerToBounds;
	Rect			digititzerRect = storage->digiRect;
	OffsetRect(&digititzerRect, -digititzerRect.left, -digititzerRect.top);
	RectMatrix(&digitizerToBounds, &digititzerRect, bounds);
	
	Scale the desiredBounds and the desiredSection rectangles through the matrix.
	
	Rect desiredBounds	= {0, 0, 1200, 1600};
	Rect desiredSection	= storage->digiRect;
	TransformRect(&digitizerToBounds, &desiredBounds, 0);
	TransformRect(&digitizerToBounds, &desiredSection, 0);
	
	You may want to query your hardware here asking what it's current format is
	i.e. 2vuy, RGB, etc.
	currentFormat = SetHardwareFormat(compressType);

	Format format = SelectFormat(desiredBounds, compressType, depth);
	SetFormat(format, desiredBounds, desiredSection, GetFrameRate(), true);

	UInt32 width 	= desiredSection.right - desiredSection.left;
	UInt32 height	= desiredSection.bottom - desiredSection.top;
*/	
    // fill in the image desciption - width and height are the only parameters we need to fill in here
	if (storage->desc) {
        // NOTE: In all cases except DVCPROHD, the Image Description width and height
        // are the Encoded Dimensions. Setting the height and width properties negates
        // having to directly set them in the Image Description.
        // Doing both may be redundant, although safe.
        
        SInt32 width = (storage->digiRect).right - (storage->digiRect).left;
        SInt32 height = (storage->digiRect).bottom - (storage->digiRect).top;
        
		// fill in the image desciption width and height
		(*(storage->desc))->width = width;
		(*(storage->desc))->height = height;
                
        // Encoded width property
        /*
         * The width of the encoded image. Usually, but not always, this is
         * the ImageDescription's width field.
         */
        ICMImageDescriptionSetProperty(storage->desc, kQTPropertyClass_ImageDescription,
                                       kICMImageDescriptionPropertyID_EncodedWidth, sizeof(SInt32),
                                       &width);

        // Encoded height property
        /*
         * The height of the encoded image. Usually, but not always, this is
         * the ImageDescription's height field.
         */
        ICMImageDescriptionSetProperty(storage->desc, kQTPropertyClass_ImageDescription,
                                       kICMImageDescriptionPropertyID_EncodedHeight, sizeof(SInt32),
                                       &height);
	}
	
	return noErr;
}

// VDCompressOneFrameAsync
// 		Instructs the video digitizer to digitize and compress a single frame of image data.
pascal VideoDigitizerError SoftVDigXCompressOneFrameAsync(SoftVDigXGlobalsPtr storage)
{
	// signal hardware that a frame has been requested
	
	// because in this sample our KEXT is running continuosly
	// on a timer we don't do anything here
	if (!storage->compressOn) return badCallOrderErr;

	return noErr;
}

// VDCompressDone
//		Determines whether the video digitizer has finished digitizing and compressing a frame of image data.
/*
	queuedFrameCount - A pointer to the number of queued frames yet to be done. 0 means no frames. Some VDIGs may
					   return 2 even if more than 2 frames are available, and some will return 1 if any number more than 0 are available.
	theData - A pointer to a field that is to receive a pointer to the compressed image data. The digitizer returns a pointer that is valid
			  in the application's current memory mode.
	dataSize - A pointer to a field to receive a value indicating the number of bytes of compressed image data.
	similarity - A pointer to a field to receive an indication of the relative similarity of this image to the previous image in a sequence.
				 A value of 0 indicates that the current frame is a key frame in the sequence. A value of 255 indicates that the current frame
				 is identical to the previous frame. Values from 1 through 254 indicate relative similarity, ranging from very different (1) to
				 very similar (254). This field is only filled in if the temporal quality passed in with VDSetCompression (III-2174) is not 0;
				 that is, if it is not frame-differenced.
	t - A pointer to a TimeRecord (IV-2649) structure. When the operation is complete, the digitizer fills in this structure with information
		indicating when the frame was grabbed. The time value stored in this structure is in the time base that the application sets with
		VDSetTimeBase (III-2196).
Note that UInt8 *queuedFrameCount replaces Boolean *done from the pre-QuickTime 6 days. 0 (==false) still means no frames, and 1 (==true) one,
but if more than one are available the number should be returned here. The value 2 previously meant more than one frame, so some VDIGs may
return 2 even if more than 2 are available, and some will still return 1 as they are using the original definition. - Introduced in QuickTime 6.
*/
pascal VideoDigitizerError SoftVDigXCompressDone(SoftVDigXGlobalsPtr storage, UInt8 		*queuedFrameCount,
																			  Ptr   		*theData,
																			  long  		*dataSize,
																			  unsigned char *similarity,
																			  TimeRecord 	*tr)
{
	kern_return_t kernResult;
	
	*queuedFrameCount = 0;
	*theData = NULL;
	*dataSize = 0;
	*similarity = 0;	// every frame is a key frame
	
	if (!storage->compressOn) return badCallOrderErr;
	
	if (storage->captureState & vdFlagCaptureIsForPreview) {
		// *** PREVIEWING ***
		// always try and return a frame
		// frame throttle is done by the device (our KEXT) which can only return frames at the constant rate of
		// 1 every 33.3333 ms basically 30fps - if you return frames faster you would see a faster preview
		kernResult = IOConnectMethodScalarIScalarO((storage->userClient),
													kCompressDone,		// an index to the function in the Kernel.
													0,					// the number of scalar input values.
													3,					// the number of scalar output values.
													queuedFrameCount,	// a scalar output parameter.
													theData,			// a scalar output parameter.
													dataSize);			// a scalar output parameter.
		if (kernResult) return internalComponentErr;
		
		GetTimeBaseTime(storage->timeBase, kVideoTimeScale, tr);
		
	} else {
		// *** RECORDING ***
		
		// our time step (when to return the next frame) has been set, and our time base is running...
		if (storage->frameTimeStep && GetTimeBaseRate(storage->timeBase)) {
			// ...the current timebase time is greater than or equal to our next frame time...
			if (GetTimeBaseTime(storage->timeBase, kVideoTimeScale, NULL) >= storage->nextFrameTime) {
				// ...see if a frame is ready and if so deliver it
				kernResult = IOConnectMethodScalarIScalarO((storage->userClient),
															kCompressDone,		// an index to the function in the Kernel.
															0,					// the number of scalar input values.
															3,					// the number of scalar output values.
															queuedFrameCount,	// a scalar output parameter.
															theData,			// a scalar output parameter.
															dataSize);			// a scalar output parameter.
				if (kernResult) return internalComponentErr;

				if (*queuedFrameCount) {
					// one or more frames are available
					// if frameTime is valid, calculate the frame time for the frame about to be returned
					// if frameTime is not valid it's the very first frame -- we need to return the time
					// NOW which becomes our anchor point for future frames, the call to GetTimeBaseTime
					// initializes storage->nextFrameTime and returns storage->frameTime
					if (storage->frameTimeValid) {
						CompTimeValue theTimeStep = {0};

						theTimeStep.lo = storage->frameTimeStep;
						WideAdd(&storage->frameTime.value, &theTimeStep);	// increment our frame time stamp
						WideAdd(&storage->atTime.value, &theTimeStep);		// increment atTime for use in VDGetTimeCode()
					} else {
						storage->nextFrameTime = GetTimeBaseTime(storage->timeBase, kVideoTimeScale, &storage->frameTime);
						storage->frameTimeValid = true;
						
						// initialize atTime to 0 and fill in the timeCodeFormat field for use in VDGetTimeCode()
						storage->atTime = storage->frameTime;
						storage->atTime.value.lo = 0; // start this time at 0 for frame 0
						storage->timeCodeFormat.flags = ((storage->framesPerSecond >> 16) == 29) ? (tc24HourMax | tcDropFrame) : tc24HourMax;
						storage->timeCodeFormat.fTimeScale = storage->atTime.scale;
						storage->timeCodeFormat.frameDuration = storage->frameTimeStep;
						
						// calculate frames/sec, e.g. (2997 units/1 sec) * (1 frame/100 units) = 29.97 fps
						// add 50 to scale to ensure that rounding goes up or down as needed
						storage->timeCodeFormat.numFrames = (kVideoTimeScale + 50) / storage->frameTimeStep;
					}
					
					// synthesize the timecode as we go
					// fill in the timeCodeTime fields for use in VDGetTimeCode()
					TimeToTimeCode(&storage->atTime, &storage->timeCodeFormat, &storage->timeCodeTime);
					
					storage->nextFrameTime += storage->frameTimeStep;
					
					*tr = storage->frameTime;
				}
			}
		}
	}
	
	return noErr;
}

// VDReleaseCompressBuffer
// 		Frees a buffer previously returned in your VDCompressDone.
// bufferAddr - Points to the location of the buffer to be released. This address must correspond to a buffer address that the
// application obtained from VDCompressDone.
pascal VideoDigitizerError SoftVDigXReleaseCompressBuffer(SoftVDigXGlobalsPtr storage, Ptr theBufffer)
{
	kern_return_t kernResult;
	
	if (!storage->compressOn) return badCallOrderErr;
	
	// this sample currently only has a single buffer, in the case of real hardware you will need to
	// track which buffers are busy and release the buffer as pointed to by 'theBuffer' passed into this call
	kernResult = IOConnectMethodScalarIScalarO((storage->userClient),
											    kReleaseCompressBuffer,	// an index to the function in the Kernel.
												0,						// the number of scalar input values.
												0);						// the number of scalar output values.
	if (kernResult) return internalComponentErr;
	
	return noErr;
}

// VDGetImageDescription
//		Retrieves an ImageDescription (IV-2438) structure from a video digitizer.
pascal VideoDigitizerError SoftVDigXGetImageDescription(SoftVDigXGlobalsPtr storage, ImageDescriptionHandle desc)
{
	OSErr err;

	if (storage->desc) {
		long descSize;
		
		descSize = GetHandleSize((Handle)(storage->desc));
		SetHandleSize((Handle)desc, descSize);
		if (err = MemError()) goto bail;
		BlockMove((Ptr) *(storage->desc), (Ptr) *desc, descSize);
	} else {
		err = badCallOrderErr;
	}

bail:
	return err;
}

// VDResetCompressSequence
//		Forces the video digitizer to insert a key frame into a temporally compressed image sequence.
pascal VideoDigitizerError SoftVDigXResetCompressSequence(SoftVDigXGlobalsPtr storage)
{
	// every frame is a key frame for our 'hardware'
	
	return noErr;
}

// VDSetCompressionOnOff
// 		Allows an application to start and stop compression by video digitizers that can deliver either
// compressed or uncompressed image data.
// Digitizers that only provide compressed data have their digiOutDoesCompressOnly flag set to 1, rather than 0.
// These digitizers may either ignore this function or return a nonzero result code. Return noErr if there is no error.
// Applications must call this function before they call either VDSetCompression (III-2174) or VDCompressOneFrameAsync (III-2125).
// This allows the video digitizer to prepare for the operation.
pascal VideoDigitizerError SoftVDigXSetCompressionOnOff(SoftVDigXGlobalsPtr storage, Boolean newState)
{
	kern_return_t kernResult;
	VideoDigitizerError err = noErr;

	// start/stop the capture device
	if (newState != storage->compressOn) {
		storage->compressOn = newState;
	
		kernResult = IOConnectMethodScalarIScalarO(storage->userClient,
												   kSetCompressionOnOff,
												   1,		// # of input params
												   0,		// # of output params
												   newState);
		if (kernResult) err = internalComponentErr;
	}
	
	return err;
}

// VDGetCompressionTypes
//		Determines the image-compression capabilities of the video digitizer.
// There must be a decompressor component of the appropriate type available in the system if an application is to
// display images from a compressed image sequence.
// '2vuy' preferred yuv pixel format for QuickTime on MacOS X
pascal VideoDigitizerError SoftVDigXGetCompressionTypes(SoftVDigXGlobalsPtr storage, VDCompressionListHandle h)
{

	ComponentDescription cd = { compressorComponentType, kCompressionFormat, 'KeyG', 0, kAnyComponentFlagsMask };
	VDCompressionListPtr compressionList = NULL;
	SInt8	hState;
		
	hState = HGetState((Handle)h);	// remember the current state
	HUnlock((Handle)h);				// unlock the handle for resizing
	
	// Set the size to one VDCompressionList since we only support one compression type
	SetHandleSize((Handle)h, sizeof(VDCompressionList));	
	if (MemError()) return MemError(); 

	HLock((Handle)h);

	compressionList = *h;

	// List your preferred compressor first.
	//
	// Setting the .codec field to '0' tells QuickTime to use the .typeName reported here in the Video Settings
	// Compression Tab list of compression choices. This is fine for QuickTime when a Video Digitizer provides data
	// in a format for which a DECOMPRESSOR exists on the system ('2vuy' or 'yuvs' for example).
	// Final Cut Pro however, requires the codec field contain a valid component identifier for a COMPRESSOR
	// component of the type of compressed data the Video Digitizer is providing. If this is not done, Final Cut Pro
	// will present a user dialog indicating that a Codec was not found and your video digitizer will not be configurable
	// in Final Cut Pro's capture settings preference panel.
	//
	// Final Cut Pro recommends supporting 8-bit-per-component 4:2:2 '2vuy' and if able 10-bit-per-component 4:2:2
	// 'v210' as documented in Ice Floe 19. http://developer.apple.com/quicktime/icefloe/dispatch019.html
	// Lower field dominant for NTSC and Upper field dominant for PAL.
	// These two Compressors are listed as follows and are provided by Final Cut Pro:
	//		Name									 Type	SubType	  Manufacturer
	//		"Apple FCP Uncompressed 8-bit 4:2:2		'imco'  '2vuy'		'KeyG'
	//		"Apple FCP Uncompressed 10-bit 4:2:2	'imco'  'v210'		'KeyG'
	//
	// If we don't find the Final Cut Pro '2vuy' compressor indicated above we return '0'. That's fine as Final Cut Pro
	// may not be installed and because there is indeed a '2vuy' DECOMPRESSOR available with QuickTime - the Sequence Grabber
	// will not have any problems with the data we return and our typeName will appear in the Codec List.
	compressionList[0].codec 			= FindNextComponent(0, &cd); // either the found component identifier or 0
	compressionList[0].cType			= kCompressionFormat;
	compressionList[0].formatFlags		= codecInfoDepth24;
	compressionList[0].compressFlags	= codecInfoDoes32;
	compressionList[0].reserved			= 0;
	
	if (compressionList[0].codec) {
		PLstrcpy(compressionList[0].typeName, "\pUncompressed 8-bit 4:2:2");
		PLstrcpy(compressionList[0].name, "\pUncompressed 8-bit 4:2:2");
	} else {
		PLstrcpy(compressionList[0].typeName, (*storage->desc)->name);
		PLstrcpy(compressionList[0].name, (*storage->desc)->name);
	}

	// restore the handle state
	HSetState( (Handle)h, hState );

	return noErr;
}

/************************************************************************************/
#pragma mark-

// VDSetTimeBase
// 		Establishes the video digitizer's time coordinate system.
/* Video digitizers that can time-stamp the video frames they produce should implement
this routine so that applications can specify the time coordinate system the video digitizer
should use when time-stamping video frames.
*/
pascal VideoDigitizerError SoftVDigXSetTimeBase(SoftVDigXGlobalsPtr storage, TimeBase t)
{
	if (NULL == t) return qtParamErr;
	
	storage->timeBase = t;

	return noErr;
}

// VDSetFrameRate
// 		Indicates an application's desired frame rate to the video digitizer.
/* This function allows applications to tell a digitizer the precise frame rate to use for capture.
Digitizers used to capture video at only one frame rate -- as fast as possible. However, the advent
of full-frame-rate digitizing hardware and compressed-source devices has made it increasingly important
for clients to manage the tradeoff between frame rate, image size, and compression quality. The rate in
VDSetFrameRate is expressed as a fixed-point value, typically between 0 and 29.97.
*/
pascal VideoDigitizerError SoftVDigXSetFrameRate(SoftVDigXGlobalsPtr storage, Fixed framesPerSecond)
{
	if (framesPerSecond < 0 || framesPerSecond > kBestFramesPerSecond) return qtParamErr;
	
	// choose the 'native' frame rate in this case
	if(framesPerSecond == 0) framesPerSecond = kBestFramesPerSecond;
	
	storage->framesPerSecond = framesPerSecond;
	
	// reset our next time step
	storage->nextFrameTime = storage->frameTimeStep = 0;
	
	// calculate our frame time step ie. the next time we return a frame from CompressDone()
	storage->frameTimeStep = FixDiv(kVideoTimeScale, framesPerSecond);

	return noErr;
}

// VDGetDataRate
//		Retrieves information that describes the performance capabilities of a video digitizer.
/* This routine is extremely useful because it gives clients a way to determine the
performance capabilities of a digitizer. The call returns three values. The first value,
milliSecPerFrame, indicates the number of milliseconds of overhead involved in digitizing a
single frame. Overhead is defined as the average delay between the time the digitizer requests a
frame from its associated hardware and the time the device actually delivers the frame. The
second value, framesPerSecond, is the maximum rate at which the digitizer can capture video
frames in its current configuration. This value is not affected by the VDSetFrameRate call.
The last value, bytesPerSecond, indicates the data rate at which a compressed-source digitizer
can produce compressed-image data. This value varies depending on parameters for spatial and motion
quality, image size, and depth. In other words, unlike milliSecPerFrame and framesPerSecond, bytesPerSecond
isn't a static value.

	milliSecPerFrame - A pointer to a long integer. The video digitizer returns a value that indicates the number of milliseconds of synchronous
					   overhead involved in digitizing a single frame. This value includes the average delay incurred between the time when the
					   digitizer requests a frame from its associated device, and the time at which the device delivers the frame.
	framesPerSecond - A pointer to a fixed value. The video digitizer supplies the maximum rate at which it can capture video. Note that this
					  value may differ from the rate that the application set with VDSetFrameRate (III-2182).
	bytesPerSecond - A pointer to a long integer. Video digitizers that can return compressed image data return a value that indicates the
					 approximate number of bytes per second that the digitizer is generating compressed data, given the current compression and
					 frame rate settings.
*/
pascal VideoDigitizerError SoftVDigXGetDataRate(SoftVDigXGlobalsPtr storage, long *milliSecPerFrame,
																			 Fixed *framesPerSecond,
																			 long *bytesPerSecond)
{
	*milliSecPerFrame = 0;
	*framesPerSecond = kBestFramesPerSecond;
	*bytesPerSecond = FixRound(storage->framesPerSecond) * ((*(storage->desc))->width * (*(storage->desc))->height * 2); // '2vuy' is 2 bytes per pixel
	
	return noErr;
}

// VDSetDataRate
//		Instructs your video digitizer component to limit the rate at which it delivers compressed, digitized video data.
/* This function is valid only for video digitizer components that can deliver compressed video; that is, components that support
the VDCompressOneFrameAsync function. Components that support data-rate limiting set the codecInfoDoesRateConstrain flag to 1 in
the compressFlags field of the VDCompressionList structure returned by the component in response to the VDGetCompressionTypes
function. Your video digitizer component should return this data-rate limit in the bytesPerSecond parameter of the existing
VDGetDataRate function.

	bytesPerSecond - The maximum data rate requested by the application, in bytes per second. This parameter is set to 0 to remove any
					 data-rate restrictions.
*/
pascal VideoDigitizerError SoftVDigXSetDataRate(SoftVDigXGlobalsPtr storage, long bytesPerSecond)
{
	// we don't do data-rate limiting
	return digiUnimpErr;
}

// VDGetCompressionTime
//		This call allow the video digitizer to quantify the compression time for the actual quality levels that will be used.
// Confirms or quantifies a video digitizer's compression settings. The sequence grabber's video compression settings dialog box
// uses this function to snap the quality slider to the correct value when working with a compression type that is specified by the
// video digitizer.
/* 
	compressionType - This value corresponds to the component subtype of the compressor component.
	depth - The depth at which the image is to be compressed. Values of 1, 2, 4, 8, 16, 24, and 32 indicate the number of bits per pixel
	        for color images. Values of 33, 34, 36, and 40 indicate 1-bit, 2-bit, 4-bit, and 8-bit grayscale, respectively, for grayscale
			images.
	srcRect - A pointer to a Rect structure that defines the portion of the source image to compress.
	spatialQuality - A pointer to a field containing the desired compressed image quality (see below). The compressor sets this field to
					 the closest actual quality that it can achieve. A value of NIL indicates that the client does not want this information.
	temporalQuality - A pointer to a field containing the desired sequence temporal quality (see below). The compressor sets this field to
					  the closest actual quality that it can achieve. A value of NIL indicates that the client does not want this information.
	compressTime - A pointer to a field to receive the compression time, in milliseconds. Your component should return a long integer
				   indicating the maximum number of milliseconds it would require to compress the specified image. If your component cannot
				   determine the amount of time required to compress the image, set this field to 0. A value of NULL indicates that the client
				   does not want this information.
				   
spatialQuality and temporalQuality Constants:				   
	codecMinQuality - The minimum valid value for a CodecQ field.
	codecLowQuality - Low-quality image reproduction. This value should correspond to the lowest image quality that still results 
					  in acceptable display characteristics.
	codecNormalQuality - Image reproduction of normal quality.
	codecHighQuality - High-quality image reproduction. This value should correspond to the highest image quality that can be achieved
					   with reasonable performance.
	codecMaxQuality - The maximum standard value for a CodecQ field.
	codecLosslessQuality - Lossless compression or decompression. This special value is valid only for components that can support lossless
						   compression or decompression.
*/
pascal VideoDigitizerError SoftVDigXGetCompressionTime(SoftVDigXGlobalsPtr storage, OSType compressType,
																					short depth,
																					Rect *srcRect,
																					CodecQ *spatialQuality,
																					CodecQ *temporalQuality,
																					unsigned long *compressTime)
{
	if (compressType != kCompressionFormat) {
		DebugStr("\pSoftVDigXGetCompressionTime - returned noCodecErr");
		return noCodecErr;
	}
	
	*spatialQuality = codecLosslessQuality;
	*temporalQuality = 0;
	
	if (compressTime) *compressTime = 0;
	
	return noErr;
}

/************************************************************************************/
#pragma mark-

/* Utility Functions that may be supported by some video digitizer components. */

// VDSetPLLFilterType
// 		Specifies which phase locked loop (PLL) is to be active.
/* Values are 0 for broadcast mode and 1 for videotape recorder mode.
*/
pascal VideoDigitizerError SoftVDigXSetPLLFilterType(SoftVDigXGlobalsPtr storage, short pllType)
{
	return digiUnimpErr;
}

// VDGetPLLFilterType
// 		Determines which phase locked loop (PLL) mode is currently active for a video digitizer.
/* Values are 0 for broadcast mode and 1 for videotape recorder mode.
*/
pascal VideoDigitizerError SoftVDigXGetPLLFilterType(SoftVDigXGlobalsPtr storage, short *pllType)
{
	// we don't do anything with this, but the "Filter" radio
	// button shows up in the Video Settings Source tab
	*pllType = 0;
	
	return noErr;
}

// VDGetPreferredTimeScale
// 		Determines a digitizer's preferred time scale.
/* This routine allows digitizers that can time-stamp the data they create to communicate their preferred time scale to
applications. The sequence grabber, in particular, uses this call to establish the time scale of the video data it creates
from the digitizer output.
*/
pascal VideoDigitizerError SoftVDigXGetPreferredTimeScale(SoftVDigXGlobalsPtr storage, TimeScale *preferred)
{
	*preferred = kVideoTimeScale;
	
	return noErr;
}

// VDGetTimeCode
// 		Instructs your video digitizer component to return timecode information for the incoming video signal.
// Typically this function is called once, at the beginning of a capture session. The use of this function assumes that the
// timecoding for the entire capture session will be continuous.
/*	atTime - A pointer to a TimeRecord structure to receive the QuickTime movie time value corresponding to the timecode information.
	timeCodeFormat - A pointer to a TimeCodeDef structure. Your video digitizer component returns the movie's timecode definition
					 information in this structure.
	timeCodeTime - A pointer to a TimeCodeRecord structure. Your video digitizer component returns the time value corresponding to
				   the movie time contained in this structure.
*/
VideoDigitizerError SoftVDigXGetTimeCode(SoftVDigXGlobalsPtr storage, TimeRecord *atTime, void *timeCodeFormat, void *timeCodeTime)
{
	if (!storage->compressOn) return badCallOrderErr;
		
	*atTime = storage->atTime;
	*((TimeCodeDef *)timeCodeFormat) = storage->timeCodeFormat;
	*((TimeCodeTime *)timeCodeTime) = storage->timeCodeTime;
	
	return noErr;
}

// VDGetPreferredImageDimensions
//		Gets the preferred image dimensions for a video digitizer.
// This function is called by the Sequence Grabber during capture just before the captured media has been inserted
// into the track - if a preferred image size is retured and there are no other visual tracks, the width and height
// will be used to make the track size match the returned preferred size. If there are other visual tracks this is
// not done as it would break the movies composition.
VideoDigitizerError SoftVDigXGetPreferredImageDimensions(SoftVDigXGlobalsPtr storage, long *width, long *height)
{
	// we could do this but why bother
/* 
	// always 320x240 track size regardless of capture size
	*width = (*(storage->desc))->width;
	*height = (*(storage->desc))->height;
*/	
	
	return digiUnimpErr;
}

// VDCaptureStateChanging
//		Provides additional information about what is happening at the sequence grabber level.
/* It has long been a problem for VDIG writers that the sequence grabber can make a series of calls, and it is not always clear what
the intent is at the higher level. This call is designed to provide additional information about what is happening at the sequence
grabber level to the VDIG, so it can take this into account. In particular, the settings bracketing calls are designed for the VDIG
to update a series of parameters without re-initializing. One point here is that the VDIG can consider the UniqueID call to be more
important than the input number, for example. The sequence grabber is now more careful when there are multiple VDIGs available to try
to save and restore more information about which one should be used. It will still pick the closest available VDIG if the exact one is
not available, but it will not stop with the first available. In addition, the old behavior of aborting a Get/Set settings call part of
the way through if an error is returned has been changed to leave the sequence grabber in a more predictable state. It also no longer
tries to save or restore settings for SGPanels that report that their hardware is unavailable. 

   vdFlagCaptureStarting - Capture is about to start allocate bandwidth 
   vdFlagCaptureStopping - Capture is about to stop; stop queuing frames 
   vdFlagCaptureIsForPreview - Capture is just to screen for preview purposes 
   vdFlagCaptureIsForRecord - Capture is going to be recorded 
   vdFlagCaptureLowLatency - Fresh frames are more important than delivering every frame - don't queue too much
   vdFlagCaptureAlwaysUseTimeBase - Use the timebase for every frame - don't worry about making durations uniform
   vdFlagCaptureSetSettingsBegin - A series of calls are about to be made to restore settings
   vdFlagCaptureSetSettingsEnd - Finished restoring settings any set calls after this are from the app or UI
*/
pascal VideoDigitizerError SoftVDigXCaptureStateChanging(SoftVDigXGlobalsPtr storage, UInt32 inStateFlags)
{	
	storage->captureState = inStateFlags;
	
	if (storage->captureState & vdFlagCaptureStarting)
		storage->frameTimeValid = false;
	
    return noErr;
}