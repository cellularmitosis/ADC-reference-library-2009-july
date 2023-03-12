// MyController.m
// Part of QTCompressionOptionsWindowTest

/*
 
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
 
 Copyright (C) 2008 Apple Inc. All Rights Reserved.
 
 */

#import "MyController.h"

@implementation MyController

#pragma mark ---- property accessors ----

@synthesize displayName = mDisplayName;
@synthesize isRecording = mIsRecording;

#pragma mark ---- initialization ----

- (void)awakeFromNib
{
    BOOL success = NO;
    NSError *error;
    
    // create the capture session
    mCaptureSession = [[QTCaptureSession alloc] init];
 
    // *** connect inputs and outputs to the session
 
    // find a video device
    QTCaptureDevice *device = [QTCaptureDevice defaultInputDeviceWithMediaType:QTMediaTypeVideo];
    if (device) {
        success = [device open:&error];
        if (!success) {
            NSLog(@"%@\n", error);
            return;
        }
        
        // add the video device to the session as device input
        mCaptureDeviceInput = [[QTCaptureDeviceInput alloc] initWithDevice:device];
        success = [mCaptureSession addInput:mCaptureDeviceInput error:&error];
        if (!success) {
            NSLog(@"%@\n", error);
            return;
        }
    }
    
    // create the movie file output and add it to the session
    mCaptureMovieFileOutput = [[QTCaptureMovieFileOutput alloc] init];
    success = [mCaptureSession addOutput:mCaptureMovieFileOutput error:&error];
    if (!success) {
        NSLog(@"%@\n", error);
        return;
    }
    
    // set the controller be the movie file output delegate
    [mCaptureMovieFileOutput setDelegate:self];
     
    // associate the capture view in the UI with the session
    [mCaptureView setCaptureSession:mCaptureSession];

    // start the capture session running
    [mCaptureSession startRunning];
    
    // ******** Compression Options Window *****
    
    // create our window with the media type and set ourselves as the delegate
    // you could also instantiate the window directly in the nib and hook up the delegate
    // simply call showWindow or setMediaType if you want to change the list of compression options shown
    mCompressionOptionsWindow = [[QTCompressionOptionsWindow alloc] initWithMediaType:[[[mCaptureMovieFileOutput connections] lastObject] mediaType]];
    if (nil == mCompressionOptionsWindow) {
        NSLog(@"Compression Options Window did not load!\n");
        return;
    }
    [mCompressionOptionsWindow setDelegate:self];
    
    // set initial UI text
    self.displayName = @"Device Native";
    
    // we want the options window to be centered on the screen
    [[mCompressionOptionsWindow window] center];
}

#pragma mark ---- actions ----

// called when the "Record" button is pressed to initiate recording
- (IBAction)startRecording:(id)sender
{
    self.isRecording = YES; // started recording so update UI
    
    // set up a 5 second timer
    [NSTimer scheduledTimerWithTimeInterval:5.0 target:self selector:@selector(stopRecording:) userInfo:nil repeats:NO];

    // tell the file output object to record to specfied location
    [mCaptureMovieFileOutput recordToOutputFileURL:[NSURL fileURLWithPath:@"/Users/Shared/QTCompressionOptionsWindowTestMovie.mov"]];
}

// display the options window
- (IBAction)showCompressionOptionsWindow:(id)sender 
{
    [mCompressionOptionsWindow showWindow:sender];
}

// tell the file output object to stop recording
- (void)stopRecording:(NSTimer *)inTimer
{
    [mCaptureMovieFileOutput recordToOutputFileURL:nil];
}

#pragma mark ---- delegates ----
#pragma mark QTCompressionOptionsWindow

// when the options window is closed this delegate method gets called
// ask for the chosen QTCompressionOptions object and configure the
// file output object accordingly
- (void)compressionOptionsWindowDidClose:(id)sender
{
    QTCompressionOptions *myCompressionOptions = [sender compressionOptions];
    
    if (nil != myCompressionOptions) {
        [mCaptureSession stopRunning];
        
        [mCaptureMovieFileOutput setCompressionOptions:myCompressionOptions forConnection:[[mCaptureMovieFileOutput connections] lastObject]];
        
        [mCaptureSession startRunning];
        
        // update the main window UI so it displays the chosen compression type
        self.displayName = [myCompressionOptions localizedDisplayName];
        
        NSLog(@"%@\n", [myCompressionOptions localizedDisplayName]);
    } else {
        NSLog(@"Bad Compression Options Object Returned!\n");
    }
}

#pragma mark QTCaptureFileOutput
// called whenever a file is finished successfully
// if the file was finished due to an error, the error is described in the error parameter
// otherwise, the error parameter equals nil
- (void)captureOutput:(QTCaptureFileOutput *)captureOutput didFinishRecordingToOutputFileAtURL:(NSURL *)outputFileURL forConnections:(NSArray *)connections dueToError:(NSError *)error
{
    // this results in QTPlayer displaying the newly recorded file
    [[NSWorkspace sharedWorkspace] openURL:outputFileURL];
    
    self.isRecording = NO;  // stopped recording so update UI
}

#pragma mark NSApplication
- (BOOL)applicationShouldTerminateAfterLastWindowClosed:(NSApplication *)sender
{
    return YES;
}

#pragma mark ---- notifications ----
#pragma mark NSWindow

// main window about to close so shut down capture
- (void)windowWillClose:(NSNotification *)notification
{
    [mCaptureSession stopRunning];
    [[mCaptureDeviceInput device] close];
    
    // not really required since we just quit
    // the app, but good for completeness
    [mCompressionOptionsWindow setDelegate:nil];
    [mCompressionOptionsWindow release];
}

@end
