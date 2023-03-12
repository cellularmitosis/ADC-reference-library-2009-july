/*

File: LiveVideo.mm

Abstract: LiveVideo class implementation. This is a simple wrapper on top of the Sequence Grabber that uses
          the SGDataProc to decompress frames using an ICMDecompressionSession to an OpenGL VisualContext.

Version: 1.0

© Copyright 2005 Apple Computer, Inc. All rights reserved.

IMPORTANT:  This Apple software is supplied to you by Apple Computer,
Inc. ("Apple") in consideration of your agreement to the following
terms, and your use, installation, modification or redistribution of
this Apple software constitutes acceptance of these terms.  If you do
not agree with these terms, please do not use, install, modify or
redistribute this Apple software.

In consideration of your agreement to abide by the following terms, and
subject to these terms, Apple grants you a personal, non-exclusive
license, under Apple's copyrights in this original Apple software (the
"Apple Software"), to use, reproduce, modify and redistribute the Apple
Software, with or without modifications, in source and/or binary forms;
provided that if you redistribute the Apple Software in its entirety and
without modifications, you must retain this notice and the following
text and disclaimers in all such redistributions of the Apple Software.
Neither the name, trademarks, service marks or logos of Apple Computer,
Inc. may be used to endorse or promote products derived from the Apple
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

Revision History:
	<1>	08/08/2005  initial release

*/

#import "LiveVideo.h"
#import <QuickTime/QuickTime.h>
#import <QuartzCore/QuartzCore.h>

@implementation LiveVideo

// sequence grabber data proc callback set by calling SGSetDataProc
static pascal OSErr GrabberDataProc(SGChannel c, Ptr p, long len, long *offset, long chRefCon, TimeValue time, short writeType, long refCon)
{
	return [(LiveVideo*)refCon handleData:p length:len time:time];
}

static void TrackDecompression(void *decompressionTrackingRefCon,
                               OSStatus result,
                               ICMDecompressionTrackingFlags decompressionTrackingFlags,
                               CVPixelBufferRef pixelBuffer,
                               TimeValue64 displayTime,
                               TimeValue64 displayDuration,
                               ICMValidTimeFlags validTimeFlags,
                               void *sourceFrameRefCon, void *reserved)
{
}

// timer method for tasking the sequence grabber
-(void)taskGrabber
{
	static const double idleInterval = 1.0 / 60.0;

	SGIdle(_grabber);
	
	if (_timer == nil)
		_timer = [[NSTimer scheduledTimerWithTimeInterval:idleInterval target:self selector:@selector(taskGrabber) userInfo:self repeats:YES] retain];
}

// initialize the sequence grabber object
-(id)initWithVisualContext:(QTVisualContextRef)visualContext
{
	OSStatus err = noErr;
	Rect bounds = {0, 0, 480, 640};

	[super init];
    
    // retain the visual context for our own use
	_visualContext = QTVisualContextRetain(visualContext);
	_size = NSMakeSize(bounds.right, bounds.bottom);
	
    // standard SG initialization
	_grabber = OpenDefaultComponent(SeqGrabComponentType, 0);
	require_action(_grabber != NULL, bail, err = memFullErr);
	
	err = SGInitialize(_grabber);
	require_noerr(err, bail);

	err = SGSetDataRef(_grabber, 0, 0, seqGrabDontMakeMovie);
	require_noerr(err, bail);
	
	err = QTNewGWorld(&_offscreen, 32, &bounds, nil, nil, 0);
	LockPixels(GetGWorldPixMap(_offscreen));
	require_noerr(err, bail);
	
	err = SGNewChannel(_grabber, VideoMediaType, &_channel);
	require_noerr(err, bail);

	err = SGSetGWorld(_channel, _offscreen, GetGWorldDevice(_offscreen));
	require_noerr(err, bail);

	err = SGSetChannelBounds(_channel, &bounds);
	require_noerr(err, bail);

	err = SGSetChannelUsage(_channel, seqGrabRecord);
	require_noerr(err, bail);

	err = SGSetDataProc(_grabber, &GrabberDataProc, (long)self);
	require_noerr(err, bail);
	
	err = SGStartRecord(_grabber);
	require_noerr(err, bail);
	
	[self taskGrabber];
	
bail:
	if (err != noErr)
	{
		[self release];
		return nil;
	}
	
	return self;
}

// deaccolate the object
-(void)dealloc
{
	if (_timer != nil)
	{
		[_timer invalidate];
		[_timer release];
	}

	if (_grabber != NULL)
		SGStop(_grabber);
	
	ICMDecompressionSessionRelease(_session);
	
	if (_imageDescription != nil)
		DisposeHandle(Handle(_imageDescription));
	
	if (_channel != nil)
		SGDisposeChannel(_grabber, _channel);
	
	if (_grabber != nil)
		CloseComponent(_grabber);
		
	if (_offscreen != nil)
		DisposeGWorld(_offscreen);
	
    // we make sure to release the visual context
	QTVisualContextRelease(_visualContext);
    
	[super dealloc];
}

// this method is called evertime we have data from the SG data proc and is where
// the work of decompressing the frame is done
-(OSErr)handleData:(void*)data length:(long)length time:(TimeValue)timeValue
{
	OSStatus err = noErr;
	ICMFrameTimeRecord now = {0};

	// Create our decompression session the first time through
	if (_imageDescription == nil)
	{
        // create a tracking callback record, this is mandatory
        // the TrackingCallback is used to track information about queued frames,
        // pixel buffers, errors, status and so on about the decompressed frames
		ICMDecompressionTrackingCallbackRecord trackingCallback = {&TrackDecompression, self};

		err = SGGetChannelTimeScale(_channel, &_timeScale);
		require_noerr(err, bail);
		
		_imageDescription = (ImageDescriptionHandle)NewHandle(0);
		err = SGGetChannelSampleDescription(_channel, Handle(_imageDescription));
		require_noerr(err, bail);
		
        // create a decompression session for our visual context, Frames will be output to a visual context
        // If desired, the trackingCallback may be used to add additional data to pixel buffers before they are sent to the visual context
        // or to keep track of the status of the decompression
		err = ICMDecompressionSessionCreateForVisualContext(NULL, _imageDescription, NULL, _visualContext, &trackingCallback, &_session);
		require_noerr(err, bail);
	}
	
	// Fill in the frame time
	now.recordSize = sizeof(ICMFrameTimeRecord);
	*(TimeValue64*)&now.value = timeValue;
	now.scale = _timeScale;
	now.rate = fixed1;
	now.decodeTime = timeValue;
	now.frameNumber = _frameNumber++;
	now.flags = icmFrameTimeIsNonScheduledDisplayTime;

	// Enqueue frame
	err = ICMDecompressionSessionDecodeFrame(_session, (const UInt8*)data, length, NULL, &now, self);
	require_noerr(err, bail);
	
	// Force frame out
	err = ICMDecompressionSessionSetNonScheduledDisplayTime(_session, *(long long int*)&now.value, _timeScale, 0);
	require_noerr(err, bail);
	
bail:
	return err;
}

-(NSSize)size
{
	return _size;
}

@end
