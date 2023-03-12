/*

File: QTRDocument.m

Abstract: Sample code for new QTKit capture classes.

Version: 1.0

Disclaimer: IMPORTANT:  This Apple software is supplied to you by Apple, Inc.
("Apple") in consideration of your agreement to the
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

Copyright © 2007 Apple, Inc., All Rights Reserved

*/


#import "QTRDocument.h"

#import <QTKit/QTKit.h>

@interface QTRDocument (QTRDocument_Private)
- (void)refreshDevices;
@end

@implementation QTRDocument

+ (void)initialize
{
	if ([self class] == [QTRDocument class]) {
		[self setKeys:[NSArray arrayWithObjects:@"selectedVideoDevice", @"selectedAudioDevice", nil] triggerChangeNotificationsForDependentKey:@"hasRecordingDevice"];
		[self setKeys:[NSArray arrayWithObjects:@"selectedVideoDevice", nil] triggerChangeNotificationsForDependentKey:@"controllableDevice"];
		[self setKeys:[NSArray arrayWithObjects:@"selectedVideoDevice", nil] triggerChangeNotificationsForDependentKey:@"selectedVideoDeviceProvidesAudio"];
		[self setKeys:[NSArray arrayWithObjects:@"selectedVideoDevice", @"selectedAudioDevice", nil] triggerChangeNotificationsForDependentKey:@"mediaFormatSummary"];
	} 
}

- (void)windowWillClose:(NSNotification *)notification
{
	// Invalidate the level meter timer here to avoid a retain cycle
	if (audioLevelTimer)
		[audioLevelTimer invalidate];
	
	// Close open devices
	[self setSelectedVideoDevice:nil];
	[self setSelectedAudioDevice:nil];
	
	// Stop the session
	[session stopRunning];
}

- (void)dealloc
{
    [[NSNotificationCenter defaultCenter] removeObserver:self];

    [session release];
	[videoDeviceInput release];
	[audioDeviceInput release];
	[movieFileOutput release];
	[audioPreviewOutput release];
	
	[videoDevices release];
	[audioDevices release];
	
	[videoPreviewFilterDescriptions release];
	[videoPreviewFilterDescription release];
		
    [super dealloc];
}

- (NSString *)windowNibName
{
    return @"QTRDocument";
}

- (void)windowControllerDidLoadNib:(NSWindowController *) aController
{
    [super windowControllerDidLoadNib:aController];
	
	// Create a capture session
	session = [[QTCaptureSession alloc] init];
	
	// Attach preview to session
	[captureView setCaptureSession:session];
	[captureView setDelegate:self];	
	
	// Attach outputs to session
	movieFileOutput = [[QTCaptureMovieFileOutput alloc] init];
	[movieFileOutput setDelegate:self];
	[session addOutput:movieFileOutput error:nil];
	
	audioPreviewOutput = [[QTCaptureAudioPreviewOutput alloc] init];
	[audioPreviewOutput setVolume:0.0];
	[session addOutput:audioPreviewOutput error:nil];	
	
	// Select devices if any exist
	NSArray *myVideoDevices = [self videoDevices];

	if ([myVideoDevices count] > 0) {
		[self setSelectedVideoDevice:[myVideoDevices objectAtIndex:0]];
	}
		
	NSArray *myAudioDevices = [self audioDevices];
	if ([myAudioDevices count] > 0) {
		[self setSelectedAudioDevice:[myAudioDevices objectAtIndex:0]];
	}	

	// Start the session
	[session startRunning];		
	// Register for notifications
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(devicesDidChange:) name:QTCaptureDeviceWasConnectedNotification object:nil];
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(devicesDidChange:) name:QTCaptureDeviceWasDisconnectedNotification object:nil];
	[[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(connectionFormatWillChange:) name:QTCaptureConnectionFormatDescriptionWillChangeNotification object:nil];
	[[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(connectionFormatDidChange:) name:QTCaptureConnectionFormatDescriptionDidChangeNotification object:nil];
	[[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(deviceAttributeWillChange:) name:QTCaptureDeviceAttributeWillChangeNotification object:nil];
	[[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(deviceAttributeDidChange:) name:QTCaptureDeviceAttributeDidChangeNotification object:nil];
		
	// Start updating the audio level meter
	audioLevelTimer = [NSTimer scheduledTimerWithTimeInterval:0.1 target:self selector:@selector(updateAudioLevels:) userInfo:nil repeats:YES];
}

- (void)alertDidEnd:(NSAlert *)alert returnCode:(NSInteger)returnCode contextInfo:(void *)contextInfo
{
	// Do nothing
}

#pragma mark Device selection
- (void)devicesDidChange:(NSNotification *)notification
{
	[self refreshDevices];
}

- (void)refreshDevices
{
	[self willChangeValueForKey:@"videoDevices"];
	[videoDevices release];
	videoDevices = [[[QTCaptureDevice inputDevicesWithMediaType:QTMediaTypeVideo] arrayByAddingObjectsFromArray:[QTCaptureDevice inputDevicesWithMediaType:QTMediaTypeMuxed]] retain];
	[self didChangeValueForKey:@"videoDevices"];
	
	[self willChangeValueForKey:@"audioDevices"];
	[audioDevices release];
	audioDevices = [[NSArray alloc] initWithArray:[QTCaptureDevice inputDevicesWithMediaType:QTMediaTypeSound]];
	[self didChangeValueForKey:@"audioDevices"];
	
	if (![videoDevices containsObject:[self selectedVideoDevice]]) {
		[self setSelectedVideoDevice:nil];
	}
	
	if (![audioDevices containsObject:[self selectedAudioDevice]]) {
		[self setSelectedAudioDevice:nil];
	}	
}

- (NSArray *)videoDevices
{
	if (!videoDevices)
		[self refreshDevices];
	
	return videoDevices;
}

- (NSArray *)audioDevices
{
	if (!audioDevices)
		[self refreshDevices];
	
	return audioDevices;
}

- (QTCaptureDevice *)selectedVideoDevice
{
	return [videoDeviceInput device];
}

- (void)setSelectedVideoDevice:(QTCaptureDevice *)selectedVideoDevice
{
	if (videoDeviceInput) {
		// Remove the old device input from the session and close the device
		[session removeInput:videoDeviceInput];
		[[videoDeviceInput device] close];
		[videoDeviceInput release];
		videoDeviceInput = nil;
	}
	
	if (selectedVideoDevice) {
		NSError *error = nil;
		BOOL success;
		
		// Try to open the new device
		success = [selectedVideoDevice open:&error];
		if (!success) {
			[[NSAlert alertWithError:error] beginSheetModalForWindow:[[[self windowControllers] objectAtIndex:0] window] modalDelegate:self didEndSelector:@selector(alertDidEnd:returnCode:contextInfo:) contextInfo:NULL];
			return;
		}
		
		// Create a device input for the device and add it to the session
		videoDeviceInput = [[QTCaptureDeviceInput alloc] initWithDevice:selectedVideoDevice];
		
		success = [session addInput:videoDeviceInput error:&error];
		if (!success) {
			[[NSAlert alertWithError:error] beginSheetModalForWindow:[[[self windowControllers] objectAtIndex:0] window] modalDelegate:self didEndSelector:@selector(alertDidEnd:returnCode:contextInfo:) contextInfo:NULL];
			[videoDeviceInput release];
			videoDeviceInput = nil;
			[selectedVideoDevice close];
			return;
		}
	}
	
	// If this video device also provides audio, don't use another audio device
	if ([self selectedVideoDeviceProvidesAudio]) {
		[self setSelectedAudioDevice:nil];
	}
}

- (QTCaptureDevice *)selectedAudioDevice
{
	return [audioDeviceInput device];
}

- (void)setSelectedAudioDevice:(QTCaptureDevice *)selectedAudioDevice
{
	if (audioDeviceInput) {
		// Remove the old device input from the session and close the device
		[session removeInput:audioDeviceInput];
		[[audioDeviceInput device] close];
		[audioDeviceInput release];
		audioDeviceInput = nil;
	}
	
	if (selectedAudioDevice && ![self selectedVideoDeviceProvidesAudio]) {
		NSError *error = nil;
		BOOL success;
		
		// Try to open the new device
		success = [selectedAudioDevice open:&error];
		if (!success) {
			[[NSAlert alertWithError:error] beginSheetModalForWindow:[[[self windowControllers] objectAtIndex:0] window] modalDelegate:self didEndSelector:@selector(alertDidEnd:returnCode:contextInfo:) contextInfo:NULL];
			return;
		}
		
		// Create a device input for the device and add it to the session
		audioDeviceInput = [[QTCaptureDeviceInput alloc] initWithDevice:selectedAudioDevice];
		
		success = [session addInput:audioDeviceInput error:&error];
		if (!success) {
			[[NSAlert alertWithError:error] beginSheetModalForWindow:[[[self windowControllers] objectAtIndex:0] window] modalDelegate:self didEndSelector:@selector(alertDidEnd:returnCode:contextInfo:) contextInfo:NULL];
			[audioDeviceInput release];
			audioDeviceInput = nil;
			[selectedAudioDevice close];
			return;
		}
	}
}

- (BOOL)selectedVideoDeviceProvidesAudio
{
	return ([[self selectedVideoDevice] hasMediaType:QTMediaTypeMuxed] || [[self selectedVideoDevice] hasMediaType:QTMediaTypeSound]);
}

#pragma mark Capture and recording

- (QTCaptureSession *)session
{
	return session;
}

- (QTCaptureAudioPreviewOutput *)audioPreviewOutput
{
	return audioPreviewOutput;
}

- (BOOL)hasRecordingDevice
{
	return ((videoDeviceInput != nil) || (audioDeviceInput != nil));
}

- (BOOL)isRecording
{
    return ([movieFileOutput outputFileURL] != nil);
}

- (void)setRecording:(BOOL)recording
{
    if (recording != [self isRecording]) {
        if (recording) {
			// Record to a temporary file, which the user will relocate when recording is finished
			char *tempNameBytes = tempnam([NSTemporaryDirectory() fileSystemRepresentation], "QTRecorder_");
			NSString *tempName = [[[NSString alloc] initWithBytesNoCopy:tempNameBytes length:strlen(tempNameBytes) encoding:NSUTF8StringEncoding freeWhenDone:YES] autorelease];
						
			[movieFileOutput recordToOutputFileURL:[NSURL fileURLWithPath:[tempName stringByAppendingPathExtension:@"mov"]]];
        } else {
            [movieFileOutput recordToOutputFileURL:nil];
        }
    }
}

// File output delegate methods
- (void)captureOutput:(QTCaptureFileOutput *)captureOutput didOutputSampleBuffer:(QTSampleBuffer *)sampleBuffer fromConnection:(QTCaptureConnection *)connection
{

}

- (void)captureOutput:(QTCaptureFileOutput *)captureOutput willStartRecordingToOutputFileAtURL:(NSURL *)fileURL forConnections:(NSArray *)connections
{
	NSLog(@"Will start recording to %@", [fileURL description]);
}

- (void)captureOutput:(QTCaptureFileOutput *)captureOutput didStartRecordingToOutputFileAtURL:(NSURL *)fileURL forConnections:(NSArray *)connections
{
	NSLog(@"Did start recording to %@", [fileURL description]);
}

- (BOOL)captureOutput:(QTCaptureFileOutput *)captureOutput shouldChangeOutputFileAtURL:(NSURL *)outputFileURL forConnections:(NSArray *)connections dueToError:(NSError *)error
{
	NSLog(@"Should change file due to error %@", [error description]);
	
	return YES;
}

- (void)captureOutput:(QTCaptureFileOutput *)captureOutput mustChangeOutputFileAtURL:(NSURL *)outputFileURL forConnections:(NSArray *)connections dueToError:(NSError *)error
{
	NSLog(@"Must change file due to error %@", [error description]);
}

- (void)captureOutput:(QTCaptureFileOutput *)captureOutput willFinishRecordingToOutputFileAtURL:(NSURL *)outputFileURL forConnections:(NSArray *)connections dueToError:(NSError *)error
{
	NSLog(@"Will finish recording to %@ due to error %@", [outputFileURL description], [error description]);
	
	// This delegate method may not be called on the main thread, so do all UI updates using performSelectorOnMainThread:
	[self performSelectorOnMainThread:@selector(willFinishRecording) withObject:nil waitUntilDone:NO];
}

- (void)willFinishRecording
{
	[self willChangeValueForKey:@"recording"];
}

- (void)captureOutput:(QTCaptureFileOutput *)captureOutput didFinishRecordingToOutputFileAtURL:(NSURL *)outputFileURL forConnections:(NSArray *)connections dueToError:(NSError *)error
{
	NSLog(@"Recorded:\n%llu Bytes\n%@ Duration", [captureOutput recordedFileSize], QTStringFromTime([captureOutput recordedDuration]));
    
	[self didChangeValueForKey:@"recording"];
	
	if (error && ![[[error userInfo] objectForKey:QTErrorRecordingSuccesfullyFinishedKey] boolValue]) {
		[[NSAlert alertWithError:error] beginSheetModalForWindow:[[[self windowControllers] objectAtIndex:0] window] modalDelegate:self didEndSelector:@selector(alertDidEnd:returnCode:contextInfo:) contextInfo:NULL];
		return;
	}
	
    // Move the recorded temporary file to a user-specified location
    NSSavePanel *savePanel = [NSSavePanel savePanel];
    
    [savePanel setRequiredFileType:@"mov"];
    [savePanel setCanSelectHiddenExtension:YES];
    [savePanel beginSheetForDirectory:nil file:nil modalForWindow:[[[self windowControllers] objectAtIndex:0] window] modalDelegate:self didEndSelector:@selector(savePanelDidEnd:returnCode:contextInfo:) contextInfo:[outputFileURL retain]]; // The output URL will be released by the delegate method
}

- (void)savePanelDidEnd:(NSSavePanel *)sheet returnCode:(int)returnCode contextInfo:(void *)contextInfo
{
	NSURL *outputFileURL = [(NSURL *)contextInfo autorelease];	// Was retained when the sheet was opened
	
    if (returnCode == NSOKButton) {
		NSString *filename = [sheet filename];
		[[NSFileManager defaultManager] movePath:[outputFileURL path] toPath:filename handler:nil];
		[[NSWorkspace sharedWorkspace] openFile:filename];
    } else {
		[[NSFileManager defaultManager] removeFileAtPath:[outputFileURL path] handler:nil];
	}
}

#pragma mark Video preview filter

- (NSArray *)videoPreviewFilterDescriptions
{
	if (!videoPreviewFilterDescriptions) {
		// Build an array of dictionaries describing filters that can be used in the video preview
		NSArray *filterNames = [NSArray arrayWithObjects:
			@"CIKaleidoscope",
			@"CIGaussianBlur",
			@"CIZoomBlur",
			@"CIColorInvert",
			@"CISepiaTone",
			@"CIBumpDistortion",
			@"CICircularWrap",
			@"CIHoleDistortion",
			@"CITorusLensDistortion",
			@"CITwirlDistortion",
			@"CIVortexDistortion",
			@"CICMYKHalftone",
			@"CIColorPosterize",
			@"CIDotScreen",
			@"CIHatchedScreen",
			@"CIBloom",
			@"CICrystallize",
			@"CIEdges",
			@"CIEdgeWork",
			@"CIGloom",
			@"CIPixellate",
			nil];
		NSMutableArray *myVideoPreviewFilterDescriptions = [NSMutableArray arrayWithCapacity:[filterNames count]];
		
		NSEnumerator *filterNamesEnumerator = [filterNames objectEnumerator];
		NSString *filterName;
		while ((filterName = [filterNamesEnumerator nextObject])) {
			[myVideoPreviewFilterDescriptions addObject:[NSDictionary dictionaryWithObjectsAndKeys:filterName, @"filterName", [CIFilter localizedNameForFilterName:filterName], @"localizedName", nil]];
		}
		
		videoPreviewFilterDescriptions = [[NSArray alloc] initWithArray:myVideoPreviewFilterDescriptions];
	}
	
	return videoPreviewFilterDescriptions;
}

- (NSDictionary*)videoPreviewFilterDescription
{
	return videoPreviewFilterDescription;
}

- (void)setVideoPreviewFilterDescription:(NSDictionary *)theVideoPreviewFilterDescription
{
	if (theVideoPreviewFilterDescription != videoPreviewFilterDescription) {
		[videoPreviewFilterDescription release];
		videoPreviewFilterDescription = [theVideoPreviewFilterDescription copy];
		
		[captureView setNeedsDisplay:YES];
	}
}

- (CIImage *)view:(QTCaptureView *)view willDisplayImage:(CIImage *)image
{
	NSDictionary *theVideoPreviewFilterDescription = [self videoPreviewFilterDescription];
	
	if (!theVideoPreviewFilterDescription) {
		return image; // do nothing
	}
	
	CIFilter *filter = [CIFilter filterWithName:[theVideoPreviewFilterDescription objectForKey:@"filterName"]];
	
	[filter setDefaults];
	[filter setValue:image forKey:@"inputImage"];  
	
	return [filter valueForKey:@"outputImage"];
}

#pragma mark Media format summary
- (NSString *)mediaFormatSummary
{
	if (!videoDeviceInput && !audioDeviceInput)
		return nil;
	
	NSMutableString *mediaFormatSummary = [NSMutableString stringWithCapacity:0];
	
	NSEnumerator *videoConnectionEnumerator = [[videoDeviceInput connections] objectEnumerator];
	QTCaptureConnection *videoConnection;
	while ((videoConnection = [videoConnectionEnumerator nextObject])) {
		[mediaFormatSummary appendString:[[videoConnection formatDescription] localizedFormatSummary]];
		[mediaFormatSummary appendString:@"\n"];
	}
	
	NSEnumerator *audioConnectionEnumerator = [[audioDeviceInput connections] objectEnumerator];
	QTCaptureConnection *audioConnection;
	while ((audioConnection = [audioConnectionEnumerator nextObject])) {
		[mediaFormatSummary appendString:[[audioConnection formatDescription] localizedFormatSummary]];
		[mediaFormatSummary appendString:@"\n"];
	}	
	
	return mediaFormatSummary;
}

- (void)connectionFormatWillChange:(NSNotification *)notification
{
	id owner = [[notification object] owner];
	if ((owner == videoDeviceInput) || (owner == audioDeviceInput)) {
		[self willChangeValueForKey:@"mediaFormatSummary"];
	}
}

- (void)connectionFormatDidChange:(NSNotification *)notification
{
	id owner = [[notification object] owner];
	if ((owner == videoDeviceInput) || (owner == audioDeviceInput)) {
		[self didChangeValueForKey:@"mediaFormatSummary"];
	}
}

#pragma mark UI updating
- (void)updateAudioLevels:(NSTimer *)timer
{
	// Get the mean audio level from the movie file output's audio connections
	
	float totalDecibels = 0.0;
	
	QTCaptureConnection *connection = nil;
	NSUInteger i = 0;
	NSUInteger numberOfPowerLevels = 0;	// Keep track of the total number of power levels in order to take the mean
	
	for (i = 0; i < [[movieFileOutput connections] count]; i++) {
		connection = [[movieFileOutput connections] objectAtIndex:i];
		
		if ([[connection mediaType] isEqualToString:QTMediaTypeSound]) {
			NSArray *powerLevels = [connection attributeForKey:QTCaptureConnectionAudioAveragePowerLevelsAttribute];
			NSUInteger j, powerLevelCount = [powerLevels count];
			
			for (j = 0; j < powerLevelCount; j++) {
				NSNumber *decibels = [powerLevels objectAtIndex:j];
				totalDecibels += [decibels floatValue];
				numberOfPowerLevels++;
			}
		}
	}
	
	if (numberOfPowerLevels > 0) {
		[audioLevelMeter setFloatValue:(pow(10., 0.05 * (totalDecibels / (float)numberOfPowerLevels)) * 20.0)];
	} else {
		[audioLevelMeter setFloatValue:0];
	}
}

#pragma mark Device controls

- (QTCaptureDevice *)controllableDevice
{
    QTCaptureDevice *selectedVideoDevice = [self selectedVideoDevice];
	
    if (selectedVideoDevice) {
        // Make sure that the the device has AVC transport controls
        if (![selectedVideoDevice attributeForKey:QTCaptureDeviceAVCTransportControlsAttribute] || [selectedVideoDevice attributeIsReadOnly:QTCaptureDeviceAVCTransportControlsAttribute])
        {
            selectedVideoDevice = nil;
        }
    }
    
    return selectedVideoDevice;
}

- (BOOL)isDevicePlaying
{
	NSDictionary *transportControls = [[self controllableDevice] attributeForKey:QTCaptureDeviceAVCTransportControlsAttribute];
	
	if (transportControls) {
		QTCaptureDeviceAVCTransportControlsSpeed speed = [[transportControls objectForKey:QTCaptureDeviceAVCTransportControlsSpeedKey] longValue];
		QTCaptureDeviceAVCTransportControlsPlaybackMode playbackMode = [[transportControls objectForKey:QTCaptureDeviceAVCTransportControlsPlaybackModeKey] unsignedLongValue];
		
		return ((speed == QTCaptureDeviceAVCTransportControlsNormalForwardSpeed) && (playbackMode == QTCaptureDeviceAVCTransportControlsPlayingMode));
	}
	
	return NO;
}

- (void)setDevicePlaying:(BOOL)playing
{
	QTCaptureDevice *device = [self controllableDevice];
	
	if (device != nil)
	{
		QTCaptureDeviceAVCTransportControlsSpeed speed = playing ? QTCaptureDeviceAVCTransportControlsNormalForwardSpeed : QTCaptureDeviceAVCTransportControlsStoppedSpeed;
		
		NSDictionary *attr = [[NSDictionary alloc] initWithObjectsAndKeys:
			[NSNumber numberWithLong:speed], QTCaptureDeviceAVCTransportControlsSpeedKey,
			[NSNumber numberWithUnsignedLong:QTCaptureDeviceAVCTransportControlsPlayingMode], QTCaptureDeviceAVCTransportControlsPlaybackModeKey,
			nil];
		
		[device setAttribute:attr forKey:QTCaptureDeviceAVCTransportControlsAttribute];
	}
}

- (IBAction)stopDevice:(id)sender
{
	QTCaptureDevice* device = [self controllableDevice];
	
	if (device != nil)
	{
		NSDictionary *attr = [[NSDictionary alloc] initWithObjectsAndKeys:
			[NSNumber numberWithLong:QTCaptureDeviceAVCTransportControlsStoppedSpeed], QTCaptureDeviceAVCTransportControlsSpeedKey,
			[NSNumber numberWithUnsignedLong:QTCaptureDeviceAVCTransportControlsNotPlayingMode], QTCaptureDeviceAVCTransportControlsPlaybackModeKey,
			nil];
		
		[device setAttribute:attr forKey:QTCaptureDeviceAVCTransportControlsAttribute];
	}
}

- (BOOL)isDeviceRewinding
{
	NSDictionary *transportControls = [[self controllableDevice] attributeForKey:QTCaptureDeviceAVCTransportControlsAttribute];
	
	if (transportControls) {
		QTCaptureDeviceAVCTransportControlsSpeed speed = [[transportControls objectForKey:QTCaptureDeviceAVCTransportControlsSpeedKey] longValue];
		
		switch (speed) {
		case QTCaptureDeviceAVCTransportControlsSlowestReverseSpeed:
		case QTCaptureDeviceAVCTransportControlsVerySlowReverseSpeed:
		case QTCaptureDeviceAVCTransportControlsSlowReverseSpeed:
		case QTCaptureDeviceAVCTransportControlsNormalReverseSpeed:
		case QTCaptureDeviceAVCTransportControlsFastReverseSpeed:
		case QTCaptureDeviceAVCTransportControlsVeryFastReverseSpeed:
		case QTCaptureDeviceAVCTransportControlsFastestReverseSpeed:
			return YES;
		default:
			return NO;
		}
	}
	
	return NO;
}

- (void)setDeviceRewinding:(BOOL)rewinding
{
	QTCaptureDevice* device = [self controllableDevice];
	
	if (device != nil)
	{
		QTCaptureDeviceAVCTransportControlsSpeed speed = rewinding ? QTCaptureDeviceAVCTransportControlsFastReverseSpeed : QTCaptureDeviceAVCTransportControlsStoppedSpeed;

		// Preserve the playback mode already set for the device
		NSDictionary *originalAttr = [device attributeForKey:QTCaptureDeviceAVCTransportControlsAttribute];
		NSNumber *playbackMode = [originalAttr objectForKey:QTCaptureDeviceAVCTransportControlsPlaybackModeKey];
		
		NSDictionary *attr = [[NSDictionary alloc] initWithObjectsAndKeys:
			[NSNumber numberWithLong:speed], QTCaptureDeviceAVCTransportControlsSpeedKey,
			playbackMode, QTCaptureDeviceAVCTransportControlsPlaybackModeKey,
			nil];
		
		[device setAttribute:attr forKey:QTCaptureDeviceAVCTransportControlsAttribute];
	}
}

- (BOOL)isDeviceFastforwarding
{
	NSDictionary *transportControls = [[self controllableDevice] attributeForKey:QTCaptureDeviceAVCTransportControlsAttribute];
	
	if (transportControls) {
		QTCaptureDeviceAVCTransportControlsSpeed speed = [[transportControls objectForKey:QTCaptureDeviceAVCTransportControlsSpeedKey] longValue];
		
		switch (speed) {
		case QTCaptureDeviceAVCTransportControlsSlowestForwardSpeed:
		case QTCaptureDeviceAVCTransportControlsVerySlowForwardSpeed:
		case QTCaptureDeviceAVCTransportControlsSlowForwardSpeed:
		case QTCaptureDeviceAVCTransportControlsFastForwardSpeed:
		case QTCaptureDeviceAVCTransportControlsVeryFastForwardSpeed:
		case QTCaptureDeviceAVCTransportControlsFastestForwardSpeed:
			return YES;
		default:
			return NO;
		}
	}
	
	return NO;
}

- (void)setDeviceFastforwarding:(BOOL)fastforwarding
{	
	QTCaptureDevice* device = [self controllableDevice];
	
	if (device != nil)
	{
		QTCaptureDeviceAVCTransportControlsSpeed speed = fastforwarding ? QTCaptureDeviceAVCTransportControlsFastForwardSpeed : QTCaptureDeviceAVCTransportControlsStoppedSpeed;
		
		// Preserve the playback mode already set for the device
		NSDictionary *originalAttr = [device attributeForKey:QTCaptureDeviceAVCTransportControlsAttribute];
		NSNumber *playbackMode = [originalAttr objectForKey:QTCaptureDeviceAVCTransportControlsPlaybackModeKey];
		
		NSDictionary *attr = [[NSDictionary alloc] initWithObjectsAndKeys:
			[NSNumber numberWithLong:speed], QTCaptureDeviceAVCTransportControlsSpeedKey,
			playbackMode, QTCaptureDeviceAVCTransportControlsPlaybackModeKey,
			nil];
		
		[device setAttribute:attr forKey:QTCaptureDeviceAVCTransportControlsAttribute];
	}
}

- (void)deviceAttributeWillChange:(NSNotification *)notification
{
	if (([notification object] == [self controllableDevice]) && [[[notification userInfo] objectForKey:QTCaptureDeviceChangedAttributeKey] isEqualToString:QTCaptureDeviceAVCTransportControlsAttribute]) {
		[self willChangeValueForKey:@"devicePlaying"];
		[self willChangeValueForKey:@"deviceFastforwarding"];
		[self willChangeValueForKey:@"deviceRewinding"];
	}
}

- (void)deviceAttributeDidChange:(NSNotification *)notification
{	
	if (([notification object] == [self controllableDevice]) && [[[notification userInfo] objectForKey:QTCaptureDeviceChangedAttributeKey] isEqualToString:QTCaptureDeviceAVCTransportControlsAttribute]) {
		[self didChangeValueForKey:@"devicePlaying"];
		[self didChangeValueForKey:@"deviceFastforwarding"];
		[self didChangeValueForKey:@"deviceRewinding"];
	}
}

@end