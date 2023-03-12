/*

File: VideoOutSupport.h

Abstract:   The VideoOutSupport wraps around the AVC VideoServices
	    to make it accessable in a Cocoa/OpenGL app.

Version: 1.0

Â© Copyright 2005 Apple Computer, Inc. All rights reserved.

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

#import "VideoOutSupport.h"

#import <pthread.h>
#import <OpenGL/gl.h>
#import <OpenGL/glext.h>
#import <AVCVideoServices/AVCVideoServices.h>
using namespace AVS;



// Prototypes
IOReturn MyAVCDeviceControllerNotification(AVCDeviceController *pAVCDeviceController, void *pRefCon, AVCDevice* pDevice);
IOReturn MyAVCDeviceMessageNotification(AVCDevice *pAVCDevice,
										natural_t messageType,
										void * messageArgument,
										void *pRefCon);
void MessageReceivedProc(UInt32 msg, UInt32 param1, UInt32 param2, void *pRefCon);
										
IOReturn FramePullProc (UInt32 *pFrameIndex, void *pRefCon);
IOReturn FrameReleaseProc (UInt32 frameIndex, void *pRefCon);

void PrintLogMessage(char *pString);

void initialzeFrameQueue(void *pRefCon);

// Globals
StringLogger logger(PrintLogMessage);


//////////////////////////////////////////////////////
// MyAVCDeviceControllerNotification
//////////////////////////////////////////////////////
IOReturn MyAVCDeviceControllerNotification(AVCDeviceController *pAVCDeviceController, void *pRefCon, AVCDevice* pDevice)
{
	IOReturn result = kIOReturnSuccess ;
	AVCDevice *pAVCDevice;
	AVCDeviceStream* pAVCDeviceStream = (AVCDeviceStream*)[(VideoOutSupport*)pRefCon getDeviceStream];
	UInt32 i;
	
	printf("\nMyAVCDeviceControllerNotification\n");
	printf("=================================\n");
	for (i=0;i<(UInt32)CFArrayGetCount(pAVCDeviceController->avcDeviceArray);i++)
	{
		pAVCDevice = (AVCDevice*) CFArrayGetValueAtIndex(pAVCDeviceController->avcDeviceArray,i);
		
		printf("\n  Device %d: %s\n",(int)i,(pAVCDevice->deviceName != nil) ? pAVCDevice->deviceName : "Unknown Device Name");
		printf("    isAttached: %s   ",(pAVCDevice->isAttached == true) ? "Yes" : "No");
		printf("capabilitiesDiscovered: %s\n",(pAVCDevice->capabilitiesDiscovered == true) ? "Yes" : "No");
		printf("    supportsFCP: %s   ",(pAVCDevice->supportsFCP == true) ? "Yes" : "No");
		printf("subUnits: 0x%08X   ",(int)pAVCDevice->subUnits);
		printf("hasTapeSubunit: %s\n",(pAVCDevice->hasTapeSubunit == true) ? "Yes" : "No");
		printf("    numInputPlugs: 0x%08X   ",(int)pAVCDevice->numInputPlugs);
		printf("numOutputPlugs: 0x%08X\n",(int)pAVCDevice->numOutputPlugs);
		printf("    isDVDevice: %s   ",(pAVCDevice->isDVDevice == true) ? "Yes" : "No");
		if (pAVCDevice->isDVDevice == true)
			printf("dvMode: 0x%02X   ",pAVCDevice->dvMode);
		printf("isMPEGDevice: %s   ",(pAVCDevice->isMPEGDevice == true) ? "Yes" : "No");
		printf("isDVCProDevice: %s\n",(pAVCDevice->isDVCProDevice == true) ? "Yes" : "No");

		// If the device is not open, open it now
		if ((pAVCDevice->isOpened() == false) && (pAVCDevice->isAttached == true) && (pAVCDevice->isDVDevice == true) && (pAVCDeviceStream == nil))
		{
			printf ("  Opening Device: %s\n",(pAVCDevice->deviceName != nil) ? pAVCDevice->deviceName : "Unknown Device Name");
			result = pAVCDevice->openDevice(MyAVCDeviceMessageNotification, pRefCon);
			if (result != kIOReturnSuccess)
				printf ("  Error Opening Device: 0x%08X\n",result);
			else
			{
				// Create a DV Transmit stream for this device
				pAVCDeviceStream = pAVCDevice->CreateDVTransmitterForDevicePlug(0,
																	FramePullProc,
																	pRefCon,
																	FrameReleaseProc,
																	pRefCon,
																	MessageReceivedProc,
																	pRefCon,
																	&logger,
																	kCyclesPerDVTransmitSegment,
																	kNumDVTransmitSegments,
																	0x00,
																	8);
				if (pAVCDeviceStream == nil)
				{
					result = pAVCDevice->closeDevice();
					if (result != kIOReturnSuccess)
						printf ("  Error Closing Device: 0x%08X\n",result);
				}
				else
				{
					printf("  Starting DV Transmit to device\n");

					[(VideoOutSupport*)pRefCon setDeviceStream:pAVCDeviceStream]; 
					[(VideoOutSupport*)pRefCon setTargetDevice:pAVCDevice]; 
				}
			}
		}
	}
	
	printf ("\n");
	return kIOReturnSuccess ;
}

//////////////////////////////////////////////////////
// MyAVCDeviceMessageNotification
//////////////////////////////////////////////////////
IOReturn MyAVCDeviceMessageNotification(AVCDevice *pAVCDevice,
										natural_t messageType,
										void * messageArgument,
										void *pRefCon)
{	
	printf("MyAVCDeviceMessageNotification:  Device: %s  Message: ",
		(pAVCDevice->deviceName != nil) ? pAVCDevice->deviceName : "Unknown Device Name");
	switch(messageType)
	{
		
		case kIOMessageServiceIsTerminated:
			printf("kIOMessageServiceIsTerminated\n");
			break;
			
		case kIOMessageServiceIsSuspended:
			printf("kIOMessageServiceIsSuspended\n");
			break;
			
		case kIOMessageServiceIsResumed:
			printf("kIOMessageServiceIsResumed\n");
			break;
			
		case kIOMessageServiceIsRequestingClose:
			printf("kIOMessageServiceIsRequestingClose\n");
			break;
			
		case kIOMessageServiceIsAttemptingOpen:
			printf("kIOMessageServiceIsAttemptingOpen\n");
			break;
			
		case kIOMessageServiceWasClosed:
			printf("kIOMessageServiceWasClosed\n");
			break;
			
		case kIOMessageServiceBusyStateChange:
			printf("kIOMessageServiceBusyStateChange\n");
			break;
			
		case kIOMessageCanDevicePowerOff:
			printf("kIOMessageCanDevicePowerOff\n");
			break;
			
		case kIOMessageDeviceWillPowerOff:
			printf("kIOMessageDeviceWillPowerOff\n");
			break;
			
		case kIOMessageDeviceWillNotPowerOff:
			printf("kIOMessageDeviceWillNotPowerOff\n");
			break;
			
		case kIOMessageDeviceHasPoweredOn:
			printf("kIOMessageDeviceHasPoweredOn\n");
			break;
			
		case kIOMessageCanSystemPowerOff:
			printf("kIOMessageCanSystemPowerOff\n");
			break;
			
		case kIOMessageSystemWillPowerOff:
			printf("kIOMessageSystemWillPowerOff\n");
			break;
			
		case kIOMessageSystemWillNotPowerOff:
			printf("kIOMessageSystemWillNotPowerOff\n");
			break;
			
		case kIOMessageCanSystemSleep:
			printf("kIOMessageCanSystemSleep\n");
			break;
			
		case kIOMessageSystemWillSleep:
			printf("kIOMessageSystemWillSleep\n");
			break;
			
		case kIOMessageSystemWillNotSleep:
			printf("kIOMessageSystemWillNotSleep\n");
			break;
			
		case kIOMessageSystemHasPoweredOn:
			printf("kIOMessageSystemHasPoweredOn\n");
			break;
			
		case kIOMessageSystemWillRestart:
			printf("kIOMessageSystemWillRestart\n");
			break;
			
		case kIOFWMessageServiceIsRequestingClose:
			printf("kIOFWMessageServiceIsRequestingClose\n");
			break;
			
		case kIOFWMessagePowerStateChanged:
			printf("kIOFWMessagePowerStateChanged\n");
			break;
			
		default:
			printf("Unknown Message\n");
			break;
	}
	
	switch (messageType)
	{
		case kIOMessageServiceIsRequestingClose:
			printf ("\n  Closing Disconnected Device: %s\n",(pAVCDevice->deviceName != nil) ? pAVCDevice->deviceName : "Unknown Device Name");
			[(VideoOutSupport*)pRefCon close];
			break;
			
		default:
			break;
	}
	
	return kIOReturnSuccess ;
}


//////////////////////////////////////////////////////
// PrintLogMessage
//////////////////////////////////////////////////////
void PrintLogMessage(char *pString)
{
	printf("%s",pString);
}

//////////////////////////////////////////////////////
// MessageReceivedProc
//////////////////////////////////////////////////////
void MessageReceivedProc(UInt32 msg, UInt32 param1, UInt32 param2, void *pRefCon)
{	
    switch (msg)
    {
	case kDVTransmitterPreparePacketFetcher:
		initialzeFrameQueue(pRefCon);
		break;
		
	default:
		break;
		    
    }
}

//////////////////////////////////////////////////////
// FramePullProc
//////////////////////////////////////////////////////
IOReturn FramePullProc (UInt32 *pFrameIndex, void *pRefCon)
{
    return [(VideoOutSupport*)pRefCon getFrame:pFrameIndex];
}

//////////////////////////////////////////////////////
// FrameReleaseProc
//////////////////////////////////////////////////////
IOReturn FrameReleaseProc (UInt32 frameIndex, void *pRefCon)
{
    [(VideoOutSupport*)pRefCon releaseFrame:frameIndex];
    return kIOReturnSuccess;
}

//////////////////////////////////////////////////////
// initialzeFrameQueue
//////////////////////////////////////////////////////
void initialzeFrameQueue(void *pRefCon)
{
    [(VideoOutSupport*)pRefCon initFrameQueues];
}



@implementation DVTransmitFrameContainer 

- (id)initWithDVTransmitFrame:(void*)inFrameRec
{
    self = [super init];

    frameRec = inFrameRec;
    return self;
}

- (void*)getDVTransmitFrame
{
    return frameRec;
}

- (UInt32)frameIndex
{
    return ((DVTransmitFrame*)(frameRec))->frameIndex;
}

- (char*)frameBuffer
{
    return (char*)((DVTransmitFrame*)(frameRec))->pFrameData;
}

- (UInt32)frameSize
{
    return ((DVTransmitFrame*)(frameRec))->frameLen;
}


@end


@implementation VideoOutSupport

- (id)init
{
    IOReturn	result = kIOReturnSuccess ;
    
    self = [super init];
    
    // setup the queue lock mutex
    pthread_mutexattr_t attr;
    
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);	// the recursive thread allows us to aquire the same lock again from the same thread
								// but still lock against another thread
    pthread_mutex_init(&queueLock, &attr);
    
    // Create the AVCDeviceController and dedicated thread
    result = CreateAVCDeviceController((AVCDeviceController**)(&fAVCDeviceController),MyAVCDeviceControllerNotification, self);
    if (!fAVCDeviceController)
    {
	    printf("Error creating AVCDeviceController object: %d\n",result);    
	    [self release];
	    
	    return nil;
    }
    return self;
}

- (id)initWithFrameSize:(NSSize)inFrameSize;
{
    self = [self init];
    
    if(self)
    {
	OSErr		theError = noErr;
	
	frameSize = inFrameSize;
	// create readback buffer and compressor
	frameBounds.left = 0;
	frameBounds.top = 0;
	frameBounds.right = (int)frameSize.width;
	frameBounds.bottom = (int)frameSize.height;
	frameRowBytes = ((int)frameBounds.right * 4 + 63) & ~63;
	frameBuffer = (char*)calloc(frameRowBytes * frameBounds.bottom, 1);
	compressorPixMap = NewPixMap();
	if(compressorPixMap == NULL) {
		NSLog(@"Memory allocation failed");
		[self release];
		return nil;
	}
	(**compressorPixMap).baseAddr = frameBuffer;
	(**compressorPixMap).rowBytes = frameRowBytes;
	(**compressorPixMap).bounds = frameBounds;
	(**compressorPixMap).pixelType = RGBDirect;
	(**compressorPixMap).pixelSize = 32;
	(**compressorPixMap).cmpCount = 4;
	(**compressorPixMap).cmpSize = 8;
#ifdef __BIG_ENDIAN__
		(**compressorPixMap).pixelFormat = k32ARGBPixelFormat;
#else
		(**compressorPixMap).pixelFormat = k32BGRAPixelFormat;
#endif
	compressorImageDescription = (ImageDescriptionHandle) NewHandleClear(sizeof(ImageDescription));
	theError = CompressSequenceBegin(&compressorImageSequence, compressorPixMap, 0, &frameBounds, NULL, 32, kDVCNTSCCodecType, bestSpeedCodec, codecNormalQuality, codecMinQuality, 1, NULL, 0, compressorImageDescription);
	if(theError == noErr)
	    theError = GetCSequenceMaxCompressionSize(compressorImageSequence, compressorPixMap, &compressionSize);

	
    }
    return self;
}

- (void)dealloc
{
    pthread_mutex_destroy(&queueLock);
    [self close];
    if(fTargetAVCDevice && fAVCDeviceStream)
    {
	((AVCDevice*)fTargetAVCDevice)->DestroyAVCDeviceStream((AVCDeviceStream*)fAVCDeviceStream);
    }
    if(compressorImageSequence)
	CDSequenceEnd(compressorImageSequence);
    if(compressorPixMap)
	DisposePixMap(compressorPixMap);
    if(compressorImageDescription)
	DisposeHandle((Handle)compressorImageDescription);
    if(frameBuffer)
	free(frameBuffer);
    [super dealloc];
}

- (void)destroyFrameQueues
{
    pthread_mutex_lock(&queueLock);
    [freeFrames release];
    freeFrames = nil;
    [renderedFrames release];
    renderedFrames = nil;
    [usedFrames release];
    usedFrames = nil;
    pthread_mutex_unlock(&queueLock);
}

- (void)initFrameQueues
{
    pthread_mutex_lock(&queueLock);
    [self destroyFrameQueues];
    freeFrames = [[NSMutableArray alloc] init];
    renderedFrames = [[NSMutableArray alloc] init];
    usedFrames = [[NSMutableArray alloc] init];
    

    UInt32			i;
    DVTransmitFrame		*pFrame;
    DVTransmitFrameContainer	*frameContainer;
    
    for (i=0;i<((AVCDeviceStream*)fAVCDeviceStream)->pDVTransmitter->getNumFrames();i++)
    {
	pFrame = ((AVCDeviceStream*)fAVCDeviceStream)->pDVTransmitter->getFrame(i);
	frameContainer = [[DVTransmitFrameContainer alloc] initWithDVTransmitFrame:pFrame];
	[freeFrames addObject:frameContainer];
	[frameContainer release];	//retained by the array
    }

    pthread_mutex_unlock(&queueLock);
}


- (void)start
{
    if(fTargetAVCDevice && fAVCDeviceStream)
    {
	((AVCDevice*)fTargetAVCDevice)->StartAVCDeviceStream((AVCDeviceStream*)fAVCDeviceStream);
	isRunning = YES;
    }
}

- (void)stop
{
    if(isRunning)
    {
	[self destroyFrameQueues];
	((AVCDevice*)fTargetAVCDevice)->StopAVCDeviceStream((AVCDeviceStream*)fAVCDeviceStream);
	isRunning = NO;
    }
}

- (void*)getDeviceStream				    //AVCDeviceStream
{
    return fAVCDeviceStream;
}

- (void)setDeviceStream:(void*)inAVCDeviceStream	    //AVCDeviceStream
{
    fAVCDeviceStream = inAVCDeviceStream;
}

- (void*)getTargetDevice;				    //AVCDevice
{
    return fTargetAVCDevice;
}

- (void)setTargetDevice:(void*)inAVCDevice		    //AVCDevice
{
    fTargetAVCDevice = inAVCDevice;
}

- (void)compressFrameFromContext:(NSOpenGLContext*)inContext sourceRect:(NSRect)sourceRect
{
    // Read back the pixels and compress
    pthread_mutex_lock(&queueLock);
    
    DVTransmitFrameContainer	*theFrameContainer = [[freeFrames lastObject] retain];
    if(theFrameContainer)
	[freeFrames removeObject:theFrameContainer];    

    pthread_mutex_unlock(&queueLock);

    if(theFrameContainer)
    {
	UInt8							similarity;
	OSErr							theError;

	[inContext makeCurrentContext];
	glPushClientAttrib(GL_CLIENT_PIXEL_STORE_BIT);
	glPixelStorei(GL_PACK_ALIGNMENT, 4);	/* Force 4-byte alignment */
	glPixelStorei(GL_PACK_ROW_LENGTH, 0);
	glPixelStorei(GL_PACK_SKIP_ROWS, 0);
	glPixelStorei(GL_PACK_SKIP_PIXELS, 0);
	glReadPixels(0, 0, frameBounds.right, frameBounds.bottom, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, frameBuffer);    
	glPopClientAttrib();
		
	theError = CompressSequenceFrame(compressorImageSequence, 
					compressorPixMap, 
					&frameBounds, 
					codecFlagUpdatePrevious, 
					[theFrameContainer frameBuffer], 
					&compressionSize, 
					&similarity, 
					NULL);
	if(theError) 
	{
	    NSLog(@"CompressSequenceFrame() failed with error %i", theError);
	    return;
	}
	
	pthread_mutex_lock(&queueLock);
	[renderedFrames addObject:theFrameContainer];
	pthread_mutex_unlock(&queueLock);
    }
    
}


- (void)releaseFrame:(UInt32)inFrameIndex
{
    pthread_mutex_lock(&queueLock);
    
    int				i = [usedFrames count];
    DVTransmitFrameContainer	*theFrameContainer = nil;
    
    while(--i >= 0)
    {
	theFrameContainer = [usedFrames objectAtIndex:i];
	if([theFrameContainer frameIndex] == inFrameIndex)
	{
	    [freeFrames addObject:theFrameContainer];
	    [usedFrames removeObject:theFrameContainer];
	}
    }
    
    pthread_mutex_unlock(&queueLock);
}

- (IOReturn)getFrame:(UInt32*)pFrameIndex
{
    IOReturn result = 0;
    DVTransmitFrameContainer *frameContainer;
    
    if (frameContainer = [self pullNextRenderedFrame])
    {
	*pFrameIndex = [frameContainer frameIndex];
    } else {
	printf("DVTransmitTest: Repeating previous frame because no frames on framequeue!\n");
	result = -1;	// Causes The previous frame to be used
    }
        
    return result;
}

- (DVTransmitFrameContainer*)pullNextRenderedFrame
{
    pthread_mutex_lock(&queueLock);
    
    DVTransmitFrameContainer	*theFrameContainer = nil;
    
    if([renderedFrames count])
    {
	theFrameContainer = [renderedFrames objectAtIndex:0];
	
	if(theFrameContainer)
	{
	    [usedFrames addObject:theFrameContainer];
	    [renderedFrames removeObject:theFrameContainer];    
	}
    }
    pthread_mutex_unlock(&queueLock);
    return theFrameContainer;
}

- (void)close
{
    [self stop];
    if(fTargetAVCDevice != nil)
	((AVCDevice*)fTargetAVCDevice)->closeDevice();
    fTargetAVCDevice = nil;
    fAVCDeviceStream = nil;
}



@end
