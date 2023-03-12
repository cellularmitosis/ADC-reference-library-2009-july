/*
File: MyCaptureView.h

Abstract: A WebKit Plug-in showing how to use the QTCapture framework
	to perform video/audio capture and recording to a QuickTime movie.

Version: 1.0

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

Copyright (C) 2007 Apple Inc. All Rights Reserved.
*/

#import "MyCaptureView.h"
#import "Plugin.h"

// Strings which indicate the current state for the input device 
const NSString *  MyCaptureViewDisconnected		= @"A device was disconnected";
const NSString *  MyCaptureViewConnected			= @"A device was connected";
const NSString *  MyCaptureViewRecording			= @"Recording";
const NSString *  MyCaptureViewFinishedRecording		= @"Finished recording";


@implementation MyCaptureView

	/* we'll use awakeFromNib to setup QTCapture notifications for 
	device connections/disconnection */
	
- (void)awakeFromNib 
{
	PDEBUG();
	
	if(!mCaptureSession)
	{
			// Create the capture session
		mCaptureSession = [[QTCaptureSession alloc] init];
	}

		// We want to get notified when devices are connected or disconnected
	[[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(deviceWasConnected:) name: QTCaptureDeviceWasConnectedNotification object:nil];
	[[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(deviceWasDisconnected:) name: QTCaptureDeviceWasDisconnectedNotification object:nil];

	// begin capture
	[self captureInit];
}

- (void) dealloc 
{
	PDEBUG();
		
	[self captureDelete];	/* stop doing capture, and perform clean-up */
	
	[mCaptureSession release];
	
	[super dealloc];
}

	/* JavaScript calls can only be made from the main Plugin class.
	This routine is called by the Plugin class to specify a method which
	can be used for this purpose. */

-(void)setPlugInCallback:(SEL)selector onTarget:(id)target
{
	PDEBUG();
	
	mNotificationTarget = selector;
	mNotificationObject = target;
}

#pragma mark Capture

	/* Begin capture 
	
	    We'll create a capture session with the following device inputs & outputs:
		- video device input
		- audio device input
		- movie file output
	    then start capturing.
	*/
-(void)captureInit
{
	PDEBUG();
	
	if ([mCaptureSession isRunning] == NO)
	{			
		BOOL success = NO;
		NSError *error;
		
		// Find a video device  
		
		QTCaptureDevice *videoDevice = [QTCaptureDevice defaultInputDeviceWithMediaType:QTMediaTypeVideo];
		success = [videoDevice open:&error];
		
		
		// If a video input device can't be found or opened, try to find and open a muxed input device
		
		if (!success) 
		{
			videoDevice = [QTCaptureDevice defaultInputDeviceWithMediaType:QTMediaTypeMuxed];
			success = [videoDevice open:&error];
		}
		
		if (!success) 
		{
			videoDevice = nil;
			// Handle error
			goto bail;
		}

		// Connect inputs and outputs to the session	

		if (videoDevice) 
		{
			//Add the video device to the session as a device input
			
			mCaptureVideoDeviceInput = [[QTCaptureDeviceInput alloc] initWithDevice:videoDevice];
			success = [mCaptureSession addInput:mCaptureVideoDeviceInput error:&error];
			if (!success) 
			{
				// Handle error
				goto bail;
			}
			
			// If the video device doesn't also supply audio, add an audio device input to the session
			
			if (![videoDevice hasMediaType:QTMediaTypeSound] && ![videoDevice hasMediaType:QTMediaTypeMuxed]) 
			{
				
				QTCaptureDevice *audioDevice = [QTCaptureDevice defaultInputDeviceWithMediaType:QTMediaTypeSound];
				success = [audioDevice open:&error];
				
				if (!success) 
				{
					audioDevice = nil;
					// Handle error
					goto bail;
				}
				
				if (audioDevice) 
				{
					mCaptureAudioDeviceInput = [[QTCaptureDeviceInput alloc] initWithDevice:audioDevice];
					
					success = [mCaptureSession addInput:mCaptureAudioDeviceInput error:&error];
					if (!success) 
					{
						// Handle error
						goto bail;
					}
				}
			}
			
			// Create the movie file output and add it to the session
			
			mCaptureMovieFileOutput = [[QTCaptureMovieFileOutput alloc] init];
			success = [mCaptureSession addOutput:mCaptureMovieFileOutput error:&error];
			if (!success) 
			{
				// Handle error
				goto bail;
			}

			// Set the QTCaptureFileOutput delegate so our QTCaptureFileOutputDelegate routine
			// didFinishRecordingToOutputFileAtURL will get called during recording.
			// Note: 
			// You must also go into Interface Builder and connect the delegate outlet of the 
			// QTCaptureView to File's Owner.
			[mCaptureMovieFileOutput setDelegate:self];

			// Associate the capture view in the UI with the session
			
			[mCaptureView setCaptureSession:mCaptureSession];
			
			// Start capturing!
			[mCaptureSession startRunning];
			
		}
		
		return;	/* QTCapture initialization was a success! */
	}
	
	bail:	/* error! */
	;

}

	/* stop capturing, and remove all device inputs/outputs */
-(void)captureDelete
{
	PDEBUG();

		/* stop capturing */
	[mCaptureSession stopRunning];
	
		/* remove video device inputs */
	if ([[mCaptureVideoDeviceInput device] isOpen])
	{
		[[mCaptureVideoDeviceInput device] close];
		[mCaptureSession removeInput:mCaptureVideoDeviceInput];
		[mCaptureVideoDeviceInput release];
		mCaptureVideoDeviceInput = nil;
	}

		/* remove audio device inputs */
	if ([[mCaptureAudioDeviceInput device] isOpen])
	{
		[[mCaptureAudioDeviceInput device] close];
		[mCaptureSession removeInput:mCaptureAudioDeviceInput];
		[mCaptureAudioDeviceInput release];
		mCaptureAudioDeviceInput = nil;
	}
	
		/* remove file outputs */
	if (mCaptureMovieFileOutput)
	{
		[mCaptureMovieFileOutput setDelegate:nil];

		[mCaptureSession removeOutput:mCaptureMovieFileOutput];
		[mCaptureMovieFileOutput release];
		mCaptureMovieFileOutput = nil;
	}
}

	/* start capturing */
-(void)startCapture
{
	if ([mCaptureSession isRunning] == NO)
	{
		if (([[mCaptureVideoDeviceInput device] isOpen] == YES) && ([[mCaptureAudioDeviceInput device] isOpen] == YES))
		{
			[mCaptureSession startRunning];
		}
	}

}

	/* stop capturing */
-(void)stopCapture
{
	if ([mCaptureSession isRunning] == YES)
	{
		[mCaptureSession stopRunning];
	}

}

#pragma mark Devices

	/* called by QTCapture when a device is connected */
-(void)deviceWasConnected:(NSNotification *)notification
{
	PDEBUG();

	[mNotificationObject performSelector:(SEL)mNotificationTarget withObject:MyCaptureViewConnected];

		// begin capturing again if we are not currently doing so
	if ([mCaptureSession isRunning] == NO)
	{
		[self captureInit];
	}
}

	/* called by QTCapture when a device is disconnected */
-(void)deviceWasDisconnected:(NSNotification *)notification
{
	PDEBUG();

	[mNotificationObject performSelector:(SEL)mNotificationTarget withObject:MyCaptureViewDisconnected];

		// tear down our current capture session if the device we were using for capture has been disconnected
	if ([mCaptureSession isRunning] == YES)
	{
		if (([[mCaptureVideoDeviceInput device] isOpen] == NO)|| ([[mCaptureAudioDeviceInput device] isOpen] == NO))
		{
			[self captureDelete];
		}
	}
}


#pragma mark Recording

// Add these start and stop recording actions, and specify the output destination for your recorded media. The output is a QuickTime movie.

- (void)startRecord
{
	PDEBUG();
	
	[mCaptureMovieFileOutput recordToOutputFileURL:[NSURL fileURLWithPath:@"/Users/Shared/My Recorded Movie.mov"]];
}

- (void)stopRecord
{
	PDEBUG();
	
	[mCaptureMovieFileOutput recordToOutputFileURL:nil];
}

#pragma mark QTCapture Delegates
/* QTCapture Delegates */

/* Delegate called by QTCapture when recording starts  */

- (void)captureOutput:(QTCaptureFileOutput *)captureOutput didStartRecordingToOutputFileAtURL:(NSURL *)fileURL forConnections:(NSArray *)connections
{
	PDEBUG();
	
	// Call JavaScript to specify that recording has started
	[mNotificationObject performSelectorOnMainThread:(SEL)mNotificationTarget withObject:MyCaptureViewRecording waitUntilDone:NO];

}


/* Delegate called by QTCapture when recording to the output file is finished. */

- (void)captureOutput:(QTCaptureFileOutput *)captureOutput didFinishRecordingToOutputFileAtURL:(NSURL *)outputFileURL forConnections:(NSArray *)connections dueToError:(NSError *)error
{
	PDEBUG();

	// Call JavaScript to specify that recording has finished
	[mNotificationObject performSelectorOnMainThread:(SEL)mNotificationTarget withObject:MyCaptureViewFinishedRecording waitUntilDone:NO];

	// Open the newly recorded file (/Users/Shared/My Recorded Movie.mov)
	[[NSWorkspace sharedWorkspace] openURL:outputFileURL];
}


@end
