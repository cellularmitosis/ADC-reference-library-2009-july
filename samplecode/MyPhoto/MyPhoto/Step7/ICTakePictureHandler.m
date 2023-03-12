/*
 
 File: ICTakePictureHandler.m
 
 Abstract: ICTakePictureHandler 
 
 Version: 1.0
 
 Disclaimer: IMPORTANT:  This Apple software is supplied to you by Apple
 Computer, Inc. ("Apple") in consideration of your agreement to the
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
 
 Copyright ¬© 2005 Apple Computer, Inc., All Rights Reserved
 
 */ 

#import "ICTakePictureHandler.h"

static ICTakePictureHandler * gICTakePictureHandler = NULL;

@implementation ICTakePictureHandler
// ---------------------------------------------------------------------------------------------------------------------
+ (ICTakePictureHandler*)sharedTakePictureHandler
{
    // return global gICTakePictureHandler...
    if (NULL == gICTakePictureHandler)
    {
        gICTakePictureHandler = [[ICTakePictureHandler alloc] init];
    }
    return gICTakePictureHandler;
}

// ---------------------------------------------------------------------------------------------------------------------
static void TakePictureNotificationCallback(ICAHeader* pbPtr)
{
    // TakePictureNotificationCallback gets called when there's an Image Capture event notification
    ICTakePictureHandler * handler = (ICTakePictureHandler*)pbPtr->refcon;
    if (handler)
        [handler takePictureNotification: (ICAExtendedRegisterEventNotificationPB*)pbPtr];
}

// ---------------------------------------------------------------------------------------------------------------------
- (void)showUI: (ICAObject)deviceObject
{
    // load the nib
    if (NULL == mWindow)
    {
        [NSBundle loadNibNamed: @"TakePicture"
                         owner: self];
    }

    // save the device object - we need it for the take picture...
    mDeviceObject = deviceObject;
    
    // "full screen"
    NSRect screenRect = [[NSScreen mainScreen] visibleFrame];
    [mWindow setFrame: screenRect
              display: NO];
    
    [mWindow makeKeyAndOrderFront: self];
    
    // register for notifications...
	OSErr err;
	ICARegisterEventNotificationPB pb = {};
    
	pb.header.refcon = (UInt32)self;                    // set the refcon so we can dispatch back to 'self'
	pb.notifyProc    = TakePictureNotificationCallback; // global callback proc
	pb.object        = 0;                               // 0 = all objects
	pb.notifyType    = 0;                               // 0 = all types
	err = ICARegisterEventNotification(&pb, NULL);

	// ... error handling ...
}

// ---------------------------------------------------------------------------------------------------------------------
static void ICIgnore(ICAHeader* pbPtr)
{
}

// ---------------------------------------------------------------------------------------------------------------------
- (IBAction)takePicture: (id)sender
{
    ICAObjectSendMessagePB  pb = {};
    OSErr                   err;
    
    // send the take picture command
    pb.header.refcon		= (UInt32)self;
    // ... to the device object
    pb.object  				= mDeviceObject;
    // messageType is kICAMessageCameraCaptureNewImage
    pb.message.messageType	= kICAMessageCameraCaptureNewImage;
    
    // asynchronous call - ICIgnore will get called when call completes
    err = ICAObjectSendMessage(&pb, ICIgnore);
}

// ---------------------------------------------------------------------------------------------------------------------
- (IBAction)closeWindow: (id)sender
{
    // un-register for notifications...
	OSErr err;
	ICARegisterEventNotificationPB pb = {};
    
	pb.header.refcon = (UInt32)self;                    // set the refcon so we can dispatch back to 'self'
	pb.notifyProc    = NULL;                            // NULL = unregister
	pb.object        = 0;                               // 0 = all objects
	pb.notifyType    = 0;                               // 0 = all types
	err = ICARegisterEventNotification(&pb, NULL);
    
    // close the window
    [mWindow close];
}

// ---------------------------------------------------------------------------------------------------------------------
- (void)takePictureNotification: (ICAExtendedRegisterEventNotificationPB*)pbPtr
{
    if (kExtendedNotificationPB == pbPtr->extd)
    {
        switch (pbPtr->eventType)
        {
            case kICAEventDeviceRemoved:                    // device removed : if it's our mDeviceObject - close window
                if (pbPtr->deviceObject == mDeviceObject)
                {
                    [self closeWindow: self];
                }
                break;
                
            case kICAEventObjectAdded:                      // picture was takenn & saved, download it...
                [self downloadObject: pbPtr->object];
                break;
                
            default:
                break;
        }
    }
}

// ---------------------------------------------------------------------------------------------------------------------
static void ICADownloadFileCallback (ICAHeader* pbHeader)
{
    // we use the refcon to get back to the ICAHandler
	ICTakePictureHandler * obj = (ICTakePictureHandler*)pbHeader->refcon;
	if (obj)
		[obj downloadFileCallback: (ICADownloadFilePB*) pbHeader];
}



// ---------------------------------------------------------------------------------------------------------------------
- (void) downloadFileCallback: (ICADownloadFilePB*)pbPtr
{
	if (noErr != pbPtr->header.err)
	{
		// handle error
	} else
	{
        UInt8 * path[512];
        OSErr   err = FSRefMakePath(&mDownloadedFile, (UInt8 *)path, 512);
        
        if (noErr != err)
        {
            // ... error handling ...
            
        } else
        {
            // image was downloaded - create a new image and update the UI
            NSString * fileName  = [NSString stringWithUTF8String: (const char *)path];
            NSImage *  image = [[NSImage alloc] initWithContentsOfFile: fileName];
            [mImageView setImage: image];
            [image release];
        }
	}
}

// ---------------------------------------------------------------------------------------------------------------------
- (void)downloadObject: (ICAObject)object
{
	OSErr               err;
	ICADownloadFilePB   pb = {};
    FSRef               tmpFolder;
    
    // create a FSRef for the downlaod folder - in this case it's /tmp
    FSPathMakeRef((const UInt8 *)"/tmp/", &tmpFolder, NULL);
    
	pb.header.refcon = (UInt32)self;
	pb.object        = object;
    // as flags: delete after download - we don't want to fill up the memory card...
	pb.flags         = kDeleteAfterDownload;
	pb.dirFSRef      =  &tmpFolder;
    // pass mDownloadedFile - that way we'll get the actual filename (Image Capture may add a ' 1' or ' 2' if there's 
    // a file naming conflict
    pb.fileFSRef     = &mDownloadedFile;

	// asynchronous call - callback proc will get called when call completes
    err = ICADownloadFile(&pb, ICADownloadFileCallback);
	if (noErr != err)
	{
		// handle error
	}
}

@end
