/*

File: LiveVideoMixerController.m

Abstract:   This is the application controller object.
	    It also functions as the document as it holds
	    the video channels.

Version: 1.2 added QTCapture capabilities

© Copyright 2005, 2006 Apple Computer, Inc. All rights reserved.

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

#import "LiveVideoMixerController.h"
#import "CaptureChannel.h"

#import <CoreGraphics/CGDirectDisplay.h>
#import <OpenGL/gl.h>
#import <OpenGL/glext.h>
#import <QTKit/QTKit.h>
#import <QTKit/QTCaptureDevice.h>
#import <QTKit/QTCaptureDeviceInput.h>

GLuint				gTexNames[kNumTextures];

#pragma mark -
#pragma mark LiveVideoMixerController
//--------------------------------------------------------------------------------------------------
// LiveVideoMixerController
//--------------------------------------------------------------------------------------------------

@interface LiveVideoMixerController (private)

- (void)_loadTextureFromFile:(NSString*)path 
					intoTextureName:(GLuint)texture 
					pixelFormat:(GLuint)pixelFormat;  //GL_ALPHA or GL_BGRA
					
- (SelectionHandleID)_selectionHandleTest:(NSPoint)inPoint inRect:(NSRect)inRect;
- (NSRect)_calcTargetRect:(NSRect)targetRect videoRect:(NSRect)videoRect;

- (void)_qtHousekeeping;

// for capturing
- (void)_devicesDidChange:(NSNotification *)notification;
- (void)_refreshCaptureDevicePopUp;

@end

@implementation LiveVideoMixerController

//--------------------------------------------------------------------------------------------------

- (void)awakeFromNib
{
    CVReturn				error = kCVReturnSuccess;
    CGOpenGLDisplayMask		totalDisplayMask = 0;
    GLint					virtualScreen;
    GLint					displayMask;
    NSOpenGLPixelFormat		*openGLPixelFormat = [NSOpenGLView defaultPixelFormat];
    
    
    // build up list of displays from OpenGL's pixel format
    for (virtualScreen = 0; virtualScreen < [openGLPixelFormat  numberOfVirtualScreens]; virtualScreen++)
    {
	    [openGLPixelFormat getValues:&displayMask forAttribute:NSOpenGLPFAScreenMask forVirtualScreen:virtualScreen];
	    totalDisplayMask |= displayMask;
    }

    // create display link
    mainViewDisplayID = CGMainDisplayID();  // we start with our view on the main display
    error = CVDisplayLinkCreateWithOpenGLDisplayMask(totalDisplayMask, &displayLink);
    if(error)
    {
		NSLog(@"DisplayLink created with error:%d", error);
		displayLink = nil;
		return;
    }
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(windowChangedScreen:) name:NSWindowDidMoveNotification object:nil];
    error = CVDisplayLinkSetOutputCallback(displayLink, myCVDisplayLinkOutputCallback, self);
	
    currentChannelPositioned = -1;	// no channel is currently resized
    
    [mainView setupSharedContext];
    [mainView lock];
    glGenTextures(kNumTextures, gTexNames);
    [self _loadTextureFromFile:[[NSBundle mainBundle] pathForResource:@"Star" ofType:@"tif"] intoTextureName:gTexNames[0] pixelFormat:GL_ALPHA]; 
    [self _loadTextureFromFile:[[NSBundle mainBundle] pathForResource:@"SpeechBubbleMask" ofType:@"tif"] intoTextureName:gTexNames[1] pixelFormat:GL_ALPHA]; 
    [self _loadTextureFromFile:[[NSBundle mainBundle] pathForResource:@"brushmask" ofType:@"tif"] intoTextureName:gTexNames[2] pixelFormat:GL_ALPHA];
    [self _loadTextureFromFile:[[NSBundle mainBundle] pathForResource:@"SpeechBubble" ofType:@"tif"] intoTextureName:gTexNames[kSpeechBubbleTextureID] pixelFormat:GL_BGRA]; 
    [mainView unlock];

    masterTimeBase = NULL;
    useMasterTimeBase = NO;	// run all movies off one master time base for synchronization
	
	// for capturing
	_deviceSetList = [[NSMutableArray alloc] initWithCapacity:3];
	[self _refreshCaptureDevicePopUp];
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(_devicesDidChange:) name:QTCaptureDeviceWasConnectedNotification object:nil];
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(_devicesDidChange:) name:QTCaptureDeviceWasDisconnectedNotification object:nil];
	[self selectInput:inputSelector];
}

//--------------------------------------------------------------------------------------------------

- (BOOL)windowShouldClose:(id)sender	//close box quits the app
{
    [NSApp terminate:self];
    return YES;
}

//--------------------------------------------------------------------------------------------------

- (void)dealloc
{
    if(isPlaying)
	[self togglePlayback:nil];
    [mainView lock];
    glDeleteTextures(kNumTextures, gTexNames);
    [mainView unlock];
    CVDisplayLinkRelease(displayLink);
	
	// for capturing
	[[NSNotificationCenter defaultCenter] removeObserver:self];
	[_deviceSetList release];
	
	[super dealloc];
}

//--------------------------------------------------------------------------------------------------
#pragma mark -
//--------------------------------------------------------------------------------------------------

- (IBAction)openChannelFile:(id)sender
{
    int  channelID = [sender tag];
    
    int result;

    NSArray     *fileTypes = [QTMovie movieFileTypes:QTIncludeCommonTypes];
    NSOpenPanel *theOpenPanel = [NSOpenPanel openPanel];

    // stop current playback
    if(isPlaying)
	[self togglePlayback:nil];

    [theOpenPanel setAllowsMultipleSelection:NO];

    result = [theOpenPanel runModalForDirectory:[NSHomeDirectory() stringByAppendingPathComponent:@"Movies"] file:nil types:fileTypes];

    if (result == NSOKButton) 
    {
        NSArray				*filesToOpen = [theOpenPanel filenames];
        NSString			*theFilePath = [filesToOpen objectAtIndex:0];
		VideoChannel	    *theVideoChannel = [VideoChannel createWithFilePath:theFilePath forView:mainView];
		
		switch(channelID)
		{
			case 0:
				//set the background video to be opaque by default
				[theVideoChannel setValue:[NSNumber numberWithFloat:100.0] forKey:@"channelOpacity"];
				[channelController0 addObject:theVideoChannel];
				[positionSwitch0 setEnabled:YES]; 
				masterTimeBase = [theVideoChannel timeBase];	// always use channel 0 for master timebase
				break;
			case 1:
				[channelController1 addObject:theVideoChannel];
				[positionSwitch1 setEnabled:YES]; 
				break;
			
		}

    }
}

//--------------------------------------------------------------------------------------------------

- (IBAction)setChannelPosition:(id)sender
{
    if(sender != positionSwitch0)
	    [positionSwitch0 setState:NSOffState];
    if(sender != positionSwitch1)
	    [positionSwitch1 setState:NSOffState];
    if(sender != positionSwitch2)
	    [positionSwitch2 setState:NSOffState];

    if([sender state] == NSOnState)
    {
	    currentChannelPositioned = [sender tag];
    } else {
	    currentChannelPositioned = -1;
    }
}

//--------------------------------------------------------------------------------------------------

- (IBAction)togglePlayback:(id)sender
{
    if(isPlaying)
    {
	CVDisplayLinkStop(displayLink);
	isPlaying = NO;
	[qtHousekeepingTimer invalidate];
	[qtHousekeepingTimer release];
	qtHousekeepingTimer = nil;

	[[channelController0 content] stopMovie];
	[[channelController1 content] stopMovie];
	[[channelController2 content] stopMovie];

    } else {
	Fixed rate = X2Fix(1.0);

	[[channelController0 content] prerollMovie:rate];
	[[channelController1 content] prerollMovie:rate];
	[[channelController2 content] prerollMovie:rate];

	if(CVDisplayLinkStart(displayLink) == kCVReturnSuccess)
	{
	    TimeBase mtb = useMasterTimeBase ? masterTimeBase : NULL;
	    isPlaying = YES;
		
	    [[channelController2 content] startMovie:rate usingMasterTimeBase:mtb];
	    [[channelController1 content] startMovie:rate usingMasterTimeBase:mtb];
	    [[channelController0 content] startMovie:rate usingMasterTimeBase:mtb];
    
	    MoviesTask(NULL, 0);

	    qtHousekeepingTimer = [[NSTimer scheduledTimerWithTimeInterval:0.015	    // 60fps
			target:self 
			selector:@selector(_qtHousekeeping) 
			userInfo:nil 
			repeats:YES] retain];
	    [[NSRunLoop currentRunLoop] addTimer:qtHousekeepingTimer forMode:NSDefaultRunLoopMode];
	}
    }   
    [playButton setImage:isPlaying ? [NSImage imageNamed:@"FS_Pause_Normal.tif"] : [NSImage imageNamed:@"FS_Play_Normal.tif"]];
}

//--------------------------------------------------------------------------------------------------

- (BOOL)isPlaying
{
    return isPlaying;
}

//--------------------------------------------------------------------------------------------------

- (void)_qtHousekeeping
{
    MoviesTask(NULL, 0);    // MoviesTask has to happen on the main thread as it is not thread safe
}

//--------------------------------------------------------------------------------------------------

- (CVReturn)render:(const CVTimeStamp*)syncTimeStamp
{
    BOOL dirty = NO;
    
    dirty |= [[channelController0 content] renderChannel:syncTimeStamp];
    dirty |= [[channelController1 content] renderChannel:syncTimeStamp];
    dirty |= [[channelController2 content] renderChannel:syncTimeStamp];

    if (dirty) 
		[mainView setDirty:YES];

    [mainView flushOutput];
    return kCVReturnSuccess;
}

//--------------------------------------------------------------------------------------------------

- (NSRect)mainVideoRect
{
    NSRect		destRect = [mainView bounds];
    NSRect		mainVideoRect = NSZeroRect;

    mainVideoRect.size.height = destRect.size.height - 105.0;
    mainVideoRect.size.width = destRect.size.width;
    
    if((mainVideoRect.size.width / 4.0) > (mainVideoRect.size.height / 3.0))
    {
		mainVideoRect.size.width = (mainVideoRect.size.height / 3.0) * 4.0;
    } else {
		mainVideoRect.size.height = (mainVideoRect.size.width / 4.0) * 3.0;
    }
    mainVideoRect.origin.y = 0.0;
    mainVideoRect.origin.x = (destRect.size.width - mainVideoRect.size.width) * 0.5;
    return mainVideoRect;
}

//--------------------------------------------------------------------------------------------------

- (NSRect)_calcTargetRect:(NSRect)targetRect videoRect:(NSRect)videoRect
{
    targetRect.origin.x = videoRect.origin.x + (targetRect.origin.x * (videoRect.size.width / kVideoWidth));
    targetRect.origin.y = videoRect.origin.y + (targetRect.origin.y * (videoRect.size.height / kVideoHeight));
    targetRect.size.width = targetRect.size.width * (videoRect.size.width / kVideoWidth);
    targetRect.size.height = targetRect.size.height * (videoRect.size.height / kVideoHeight);

    return targetRect;
}

//--------------------------------------------------------------------------------------------------

- (void)drawContent
{
    NSRect			mainVideoRect = [self mainVideoRect];
    NSRect			destRect = NSZeroRect;
    NSRect			thumbnailVideoRect = NSZeroRect;
    VideoChannel		*positionedChannel = nil;
	
    
    thumbnailVideoRect.size = NSMakeSize(120.0, 90.0);
    thumbnailVideoRect.origin.x = ([mainView bounds].size.width - (3 * 120.0 + 20.0) ) * 0.5;
    thumbnailVideoRect.origin.y = [mainView bounds].size.height - thumbnailVideoRect.size.height;
	
    // clear content
    glClearColor(0.0, 0.0, 0.0, 0.0);
    glClear(GL_COLOR_BUFFER_BIT);

    if([channelController0 content])
    {
		destRect = [self _calcTargetRect:[[channelController0 content] targetRect] videoRect:mainVideoRect];
		[[channelController0 content] compositeChannelThumbnailInRect:thumbnailVideoRect];
		[[channelController0 content] compositeChannelInRect:destRect];
    }
    thumbnailVideoRect.origin.x += 130.0;
    if([channelController1 content])
    {
		destRect = [self _calcTargetRect:[[channelController1 content] targetRect] videoRect:mainVideoRect];
		[[channelController1 content] compositeChannelThumbnailInRect:thumbnailVideoRect];
		[[channelController1 content] compositeChannelInRect:destRect];
    }
    thumbnailVideoRect.origin.x += 130.0;
    if([channelController2 content])
    {
		destRect = [self _calcTargetRect:[[channelController2 content] targetRect] videoRect:mainVideoRect];
		[[channelController2 content] compositeChannelThumbnailInRect:thumbnailVideoRect];
		[[channelController2 content] compositeChannelInRect:destRect];
    }
    switch(currentChannelPositioned)
    {
		case 0:
			positionedChannel = [channelController0 content];
			break;
		case 1:
			positionedChannel = [channelController1 content];
			break;
		case 2:
			positionedChannel = [channelController2 content];
			break;
		default:
			positionedChannel = nil;
			break;
    }
    if(positionedChannel)
    {
		destRect = [self _calcTargetRect:[positionedChannel targetRect] videoRect:mainVideoRect];
		[positionedChannel drawOutline:destRect];
    }
    glFlush();
}

//--------------------------------------------------------------------------------------------------

- (void)_loadTextureFromFile:(NSString*)path intoTextureName:(GLuint)texture pixelFormat:(GLuint)pixelFormat //GL_ALPHA or GL_BGRA
{

    NSBitmapImageRep	*bitmap = [[NSBitmapImageRep alloc] initWithData: [NSData dataWithContentsOfFile: path]];

    if(bitmap)
    {
	glTexParameteri(GL_TEXTURE_RECTANGLE_EXT, GL_TEXTURE_STORAGE_HINT_APPLE , GL_STORAGE_CACHED_APPLE);
	glPixelStorei(GL_UNPACK_CLIENT_STORAGE_APPLE, GL_TRUE);
	if(pixelFormat == GL_ALPHA)
	{
		glBindTexture(kTextureTarget, texture);
		glPixelStorei(GL_UNPACK_ROW_LENGTH, [bitmap bytesPerRow] );
		glTexImage2D(kTextureTarget, 0, pixelFormat, [bitmap pixelsWide], [bitmap pixelsHigh], 0, pixelFormat, GL_UNSIGNED_BYTE, [bitmap bitmapData]);
	
	} else {
		glBindTexture(kTextureTarget, texture);
		glPixelStorei(GL_UNPACK_ROW_LENGTH, [bitmap bytesPerRow] / 4);
		glTexImage2D(kTextureTarget, 0, GL_RGBA, [bitmap pixelsWide], [bitmap pixelsHigh], 0, pixelFormat, GL_UNSIGNED_INT_8_8_8_8_REV, [bitmap bitmapData]);
	}
	glTexParameteri(kTextureTarget, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(kTextureTarget, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(kTextureTarget, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(kTextureTarget, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    }
}

//--------------------------------------------------------------------------------------------------

- (SelectionHandleID)_selectionHandleTest:(NSPoint)inPoint inRect:(NSRect)inRect
{
	NSRect					selectionHandleRect = NSMakeRect(0.0, 0.0, kSelectionHandleSize, kSelectionHandleSize);
	
	selectionHandleRect.origin = inRect.origin;
	if(NSPointInRect(inPoint, selectionHandleRect))
		return kTopLeftCorner;
	selectionHandleRect.origin.x += (inRect.size.width - kSelectionHandleSize);
	if(NSPointInRect(inPoint, selectionHandleRect))
		return kTopRightCorner;
	selectionHandleRect.origin.y += (inRect.size.height - kSelectionHandleSize);
	if(NSPointInRect(inPoint, selectionHandleRect))
		return kBottomRightCorner;
	selectionHandleRect.origin.x = inRect.origin.x;
	if(NSPointInRect(inPoint, selectionHandleRect))
		return kBottomLeftCorner;
	return kNoSelectionHandle;
}

//--------------------------------------------------------------------------------------------------

- (void)mouseDown:(NSEvent *)theEvent
{
	NSRect			videoRect = [self mainVideoRect];
	NSPoint			where = [mainView convertPoint:[theEvent locationInWindow] fromView:nil];
	NSRect			channelTargetRect, destRect;
	VideoChannel		*targetChannel = nil;
	SelectionHandleID	theSelectionHandle;
	
	if(currentChannelPositioned != -1)
	{
		switch(currentChannelPositioned)
		{
			case 0:
				targetChannel = [channelController0 content];
				break;
			case 1:
				targetChannel = [channelController1 content];
				break;
			case 2:
				targetChannel = [channelController2 content];
				break;
				
			default:
				break;
		}
		channelTargetRect = [targetChannel targetRect];
		where.x -= videoRect.origin.x;
		where.y -= videoRect.origin.y;
		destRect.origin.x = channelTargetRect.origin.x * (videoRect.size.width / kVideoWidth);
		destRect.origin.y = channelTargetRect.origin.y * (videoRect.size.height / kVideoHeight);
		destRect.size.width = channelTargetRect.size.width * (videoRect.size.width / kVideoWidth);
		destRect.size.height = channelTargetRect.size.height * (videoRect.size.height / kVideoHeight);
		theSelectionHandle = [self _selectionHandleTest:where inRect:destRect];

		if(NSPointInRect(where, destRect))
		{
			NSPoint				currentPoint, startPoint, delta;
			BOOL				dragActive = YES;
			NSAutoreleasePool	*myPool = nil;
			NSEvent* 			event = NULL;
			NSWindow			*targetWindow = [mainView window];
		
			startPoint = where;

			myPool = [[NSAutoreleasePool alloc] init];
			while (dragActive)
			{
				NSRect				outRect = channelTargetRect;
				
				
				event = [targetWindow nextEventMatchingMask:(NSLeftMouseDraggedMask | NSLeftMouseUpMask)
														untilDate:[NSDate distantFuture]
														inMode:NSEventTrackingRunLoopMode
														dequeue:YES];
			
				if(!event)
					continue;
				currentPoint = [mainView convertPoint:[event locationInWindow] fromView:nil];
				currentPoint.x -= videoRect.origin.x;
				currentPoint.y -= videoRect.origin.y;
				switch ([event type])
				{
					case NSLeftMouseDragged:
						delta.x = currentPoint.x - startPoint.x;
						delta.y = currentPoint.y - startPoint.y;
						switch(theSelectionHandle)
						{
							case kTopLeftCorner:
								outRect.origin.x += delta.x;
								outRect.origin.y += delta.y;
								outRect.size.width -= delta.x;
								outRect.size.height -= delta.y;
								break;
								
							case kTopRightCorner:
								outRect.origin.y += delta.y;
								outRect.size.width += delta.x;
								outRect.size.height -= delta.y;
								break;
							
							case kBottomLeftCorner:
								outRect.origin.x += delta.x;
								outRect.size.width -= delta.x;
								outRect.size.height += delta.y;
								break;
								
							case kBottomRightCorner:
								outRect.size.width += delta.x;
								outRect.size.height += delta.y;
								break;
								
							default:
								outRect.origin.x += delta.x;
								outRect.origin.y += delta.y;
								break;
						}
						[targetChannel setTargetRect:outRect];
						break;
					
				
					case NSLeftMouseUp:
						dragActive = NO;
						break;
						
					default:
						break;
				}
			}
			[myPool release];
			myPool = nil;

		}
	}
}

//--------------------------------------------------------------------------------------------------

- (void)windowChangedScreen:(NSNotification*)inNotification
{
	NSWindow *window = [mainView window]; 
	CGDirectDisplayID displayID = (CGDirectDisplayID)[[[[window screen] deviceDescription] objectForKey:@"NSScreenNumber"] intValue];
	
	if((displayID != 0) && (mainViewDisplayID != displayID))
	{
	    CVDisplayLinkSetCurrentCGDisplay(displayLink, displayID);
	    mainViewDisplayID = displayID;
	}
}

//--------------------------------------------------------------------------------------------------

#pragma mark -
#pragma mark QTCAPTURE
//--------------------------------------------------------------------------------------------------
// QTCAPTURE
//--------------------------------------------------------------------------------------------------

// for capturing
- (IBAction)selectInput:(id)sender
{
	if([_deviceSetList count] > 0)
	{
		CaptureChannel	    *theVideoChannel = [CaptureChannel createWithQTCaptureDevice:[_deviceSetList objectAtIndex:[sender indexOfSelectedItem]] forView:mainView];
		
		[channelController2 addObject:theVideoChannel];
		[positionSwitch2 setEnabled:YES]; 
	}
}

//--------------------------------------------------------------------------------------------------

- (void)_devicesDidChange:(NSNotification *)notification
{
    [self _refreshCaptureDevicePopUp];
}

//--------------------------------------------------------------------------------------------------

- (void)_refreshCaptureDevicePopUp
{
	NSArray *captureDevices = [QTCaptureDevice inputDevices];
	
	[_deviceSetList removeAllObjects];
	NSMutableArray* audioDeviceList=[[NSMutableArray alloc] initWithCapacity:2];
	NSMutableArray* videoDeviceList=[[NSMutableArray alloc] initWithCapacity:2];

	if ([captureDevices count]) 
	{
		// Go through each input device save to device set list
		NSEnumerator *deviceEnum = [captureDevices objectEnumerator];
        QTCaptureDevice *device;
		while((device = [deviceEnum nextObject])) 
		{
			NSNumber*			availableFormat = [device attributeForKey:QTCaptureDeviceAvailableStreamFormatsAttribute];
			QTRDeviceSet*		devset = nil;
			
			if (availableFormat) 
			{
				// 1 is muxed, 2 is video, 3 is audio
				switch ([availableFormat intValue]) 
				{
					case 1: // Muxed
						devset=[[QTRDeviceSet alloc] initWithName:[device displayName] Type:0 MuxedDevice:device VideoDevice:nil AudioDevice:nil];
						[_deviceSetList addObject:devset];
						break;
					
					case 2: // Video
						devset=[[QTRDeviceSet alloc] initWithName:[device displayName] Type:1 MuxedDevice:nil VideoDevice:device AudioDevice:nil];
						[_deviceSetList addObject:devset];
						[videoDeviceList addObject:devset];
						break;
						
					case 3: // Audio
						devset=[[QTRDeviceSet alloc] initWithName:[device displayName] Type:2 MuxedDevice:nil VideoDevice:nil AudioDevice:device];
						[audioDeviceList addObject:devset];
						break;
						
					default:
						break;
				}
				[availableFormat release];
			}
			[devset release];
		}
		
		if ([audioDeviceList count] > 0 && [videoDeviceList count] > 0)
		{
			// Create device sets which matches each audio-only device with every video-only device
			NSEnumerator*		videodevenum = [videoDeviceList objectEnumerator];
			QTRDeviceSet*		videoonlydev = nil;
			while ((videoonlydev=[videodevenum nextObject]))
			{
				NSEnumerator* audiodevenum=[audioDeviceList objectEnumerator];
				QTRDeviceSet* audioonlydev=nil;
				while ((audioonlydev=[audiodevenum nextObject]))
				{
					NSString* avdevname=[[NSString alloc] initWithFormat:@"%@ + %@",[videoonlydev deviceName],[audioonlydev deviceName]];
					QTRDeviceSet* avdevset=[[QTRDeviceSet alloc] initWithName:avdevname Type:3 MuxedDevice:nil 
									VideoDevice:[videoonlydev videoCaptureDevice] AudioDevice:[audioonlydev audioCaptureDevice]];
					[_deviceSetList addObject:avdevset];
					[avdevset release];
					[avdevname release];
				}
			}
		}
	}
	[audioDeviceList release];
	[videoDeviceList release];
	
	//Remove all popup items
	[inputSelector removeAllItems];
	int devsetcount=[_deviceSetList count];
	
	if (devsetcount>0) 
	{
		// Add the device names to the popup menu
		int i=0;
		for (i=0; i<devsetcount; ++i) {
			[inputSelector insertItemWithTitle:[[_deviceSetList objectAtIndex:i] deviceName] atIndex:i];
		}
        [inputSelector selectItemAtIndex:0];
	} else {
		[inputSelector insertItemWithTitle:@"No Devices Connected" atIndex:0];
        [inputSelector setEnabled:NO];
        [inputSelector selectItemAtIndex:0];
	}
}

@end


@implementation QTRDeviceSet
- (id)init
{
	return [self initWithName:nil Type:-1 MuxedDevice:nil VideoDevice:nil AudioDevice:nil];
}

- (id)initWithName:(NSString *)Name Type:(int)Type MuxedDevice:(QTCaptureDevice*)muxedDev VideoDevice:(QTCaptureDevice*)videoDev AudioDevice:(QTCaptureDevice*)audioDev
{
	self =[super init];
	if (self) {
		_deviceName=[Name retain];
		_deviceSetType=Type;
		_muxedCaptureDevice=[muxedDev retain];
		_videoCaptureDevice=[videoDev retain];
		_audioCaptureDevice=[audioDev retain];
	}
	return self;
}

- (void)dealloc
{
	[_deviceName release];
	[_muxedCaptureDevice release];
	[_videoCaptureDevice release];
	[_audioCaptureDevice release];
	
	[super dealloc];
}

- (NSString*) deviceName
{
	return _deviceName;
}

- (int) deviceSetType
{
	return _deviceSetType;
}

- (QTCaptureDevice*) muxedCaptureDevice
{
	return _muxedCaptureDevice;
}

- (QTCaptureDevice*) videoCaptureDevice
{
	return _videoCaptureDevice;
}

- (QTCaptureDevice*) audioCaptureDevice
{
	return _audioCaptureDevice;
}

@end

#pragma mark -
#pragma mark CALLBACKS
//--------------------------------------------------------------------------------------------------
// CALLBACKS
//--------------------------------------------------------------------------------------------------

CVReturn myCVDisplayLinkOutputCallback(CVDisplayLinkRef displayLink, 
                                                const CVTimeStamp *inNow, 
                                                const CVTimeStamp *inOutputTime, 
                                                CVOptionFlags flagsIn, 
                                                CVOptionFlags *flagsOut, 
                                                void *displayLinkContext)
{
    CVReturn    error = [(LiveVideoMixerController*)displayLinkContext render:inOutputTime];
    return error;
}


