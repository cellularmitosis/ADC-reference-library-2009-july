//////////
//
//	File:		ICAUtils.c
//
//	Contains:	Image Capture convenience functions.
//
//	Written by:	Apple Developer Technical Support
//
//	Copyright:	© 2001 by Apple Computer, Inc., all rights reserved.
//
//	Change History (most recent first):
//	   
//	   <1>	 	11/28/01	srk		first file
//
//////////

/*
 IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc. ("Apple") in
 consideration of your agreement to the following terms, and your use, installation, 
 modification or redistribution of this Apple software constitutes acceptance of these 
 terms.  If you do not agree with these terms, please do not use, install, modify or 
 redistribute this Apple software.
 
 In consideration of your agreement to abide by the following terms, and subject to these 
 terms, Apple grants you a personal, non-exclusive license, under Apple’s copyrights in 
 this original Apple software (the "Apple Software"), to use, reproduce, modify and 
 redistribute the Apple Software, with or without modifications, in source and/or binary 
 forms; provided that if you redistribute the Apple Software in its entirety and without 
 modifications, you must retain this notice and the following text and disclaimers in all 
 such redistributions of the Apple Software.  Neither the name, trademarks, service marks 
 or logos of Apple Computer, Inc. may be used to endorse or promote products derived from 
 the Apple Software without specific prior written permission from Apple. Except as expressly
 stated in this notice, no other rights or licenses, express or implied, are granted by Apple
 herein, including but not limited to any patent rights that may be infringed by your 
 derivative works or by other works in which the Apple Software may be incorporated.
 
 The Apple Software is provided by Apple on an "AS IS" basis.  APPLE MAKES NO WARRANTIES, 
 EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED WARRANTIES OF NON-INFRINGEMENT, 
 MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS 
 USE AND OPERATION ALONE OR IN COMBINATION WITH YOUR PRODUCTS.
 
 IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL OR CONSEQUENTIAL 
 DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS 
 OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) ARISING IN ANY WAY OUT OF THE USE, 
 REPRODUCTION, MODIFICATION AND/OR DISTRIBUTION OF THE APPLE SOFTWARE, HOWEVER CAUSED AND 
 WHETHER UNDER THEORY OF CONTRACT, TORT (INCLUDING NEGLIGENCE), STRICT LIABILITY OR 
 OTHERWISE, EVEN IF APPLE HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include <Carbon/Carbon.h>
#include "ICAUtils.h"

//////////
//
// Get_Device_List
//
// Returns a list of ICA devices
//
//////////

ICAObject Get_Device_List()
{
    ICAGetDeviceListPB getDeviceListPB;
    ICAObject deviceList;
    OSErr err;
    
    deviceList = nil;
    memset(&getDeviceListPB, 0, sizeof(ICAGetDeviceListPB));
    err = ICAGetDeviceList(&getDeviceListPB, nil);
    if (noErr == err)
    {
        deviceList = getDeviceListPB.object;
    }
    return deviceList;
}

//////////
//
// GetNumberOfDevices
//
// Returns a count of the number of ICA devices
//
//////////

UInt32 GetNumberOfDevices ()
{
    ICAGetChildCountPB getChildCountPB;
    OSErr err;
    
    memset(&getChildCountPB, 0, sizeof(ICAGetChildCountPB));
    getChildCountPB.object = Get_Device_List();
    err = ICAGetChildCount(&getChildCountPB, nil);
    
    return getChildCountPB.count;
}

//////////
//
// GetNumberOfChildrenForObject
//
// Returns a count of the number of children for
// the given object
//
//////////

UInt32 GetNumberOfChildrenForObject (ICAObject object)
{
    ICAGetChildCountPB getChildCountPB;
    OSErr err;
    
    memset(&getChildCountPB, 0, sizeof(ICAGetChildCountPB));
    getChildCountPB.object = object;
    err = ICAGetChildCount(&getChildCountPB, nil);
    
    return getChildCountPB.count;
}

//////////
//
// GetFirstDevice
//
// Returns the first device object
//
//////////

ICAObject GetFirstDevice ()
{
    ICAGetNthChildPB getNthChildPB;
    ICAObject firstDevice = NULL;
    OSErr err;
    UInt32 count;
    
    count = GetNumberOfDevices();
    if (count > 0)
    {
        memset(&getNthChildPB, 0, sizeof(ICAGetNthChildPB));
        getNthChildPB.parentObject = Get_Device_List();
        getNthChildPB.index = 0;
        err = ICAGetNthChild (&getNthChildPB, nil);
        if (noErr == err)
        {
            firstDevice = getNthChildPB.childObject;
        }
    }
    return firstDevice;
}

//////////
//
// IsFirstDeviceACamera
//
// Returns true if the first device object
// is a camera object
//
//////////

Boolean IsFirstDeviceACamera ()
{
    ICAGetObjectInfoPB getObjectInfoPB;
    OSErr err;
    Boolean isCamera;
    
    memset(&getObjectInfoPB, 0, sizeof(ICAGetObjectInfo));
    getObjectInfoPB.object = GetFirstDevice();
    err = ICAGetObjectInfo (&getObjectInfoPB, nil);
    if (noErr == err)
    {
        isCamera = (getObjectInfoPB.objectInfo.objectType == kICADevice) && 
            (getObjectInfoPB.objectInfo.objectSubtype == kICADeviceCamera);
    }
    return isCamera;
}

//////////
//
// GetPropertyCount
//
// Returns a count of the number of properties
// for a given object
//
//////////

UInt32 GetPropertyCount (ICAObject object)
{
    ICAGetPropertyCountPB getPropertyCountPB;
    OSErr err;
    UInt32 count = 0;
    
    memset(&getPropertyCountPB, 0, sizeof(ICAGetPropertyCountPB));
    getPropertyCountPB.object = object;
    err = ICAGetPropertyCount (&getPropertyCountPB, nil);
    if (noErr == err)
    {
        count = getPropertyCountPB.count;
    }
    return count;
}

//////////
//
// GetPropertyImageDataProperty
//
// Returns the image data property
// for a given object
//
//////////

ICAProperty GetPropertyImageDataProperty (ICAObject object)
{
    ICAGetNthPropertyPB getNthPropertyPB;
    OSErr err;
    UInt32 count = GetPropertyCount(object);
    
    ICAProperty result = NULL;
    UInt32 loop;
    for (loop = 0; loop < count; loop++)
    {
        memset(&getNthPropertyPB, 0, sizeof(ICAGetNthPropertyPB));
        getNthPropertyPB.object = object;
        getNthPropertyPB.index = loop;
        err = ICAGetNthProperty(&getNthPropertyPB, nil);
        if (noErr == err)
        {
            if (getNthPropertyPB.propertyInfo.propertyType == kICAPropertyImageData)
            {
                result = getNthPropertyPB.property;
                break;
            }
        }
    }
    return result;
}

//////////
//
// GetImageDataPropertyAndSize
//
// Returns the image data property and size
// for a given object
//
//////////

OSErr GetImageDataPropertyAndSize (ICAObject imageObj,
                                    ICAProperty* imageDataProperty,
                                    UInt32* imageDataSize)
{
    ICAGetPropertyByTypePB getPropertyByTypePB;
    OSErr err;
    
    memset(&getPropertyByTypePB, 0, sizeof(ICAGetPropertyByTypePB));
    getPropertyByTypePB.object = imageObj;
    getPropertyByTypePB.propertyType = kICAPropertyImageData;
    err = ICAGetPropertyByType (&getPropertyByTypePB, nil);
    if (noErr == err)
    {
        *imageDataProperty = getPropertyByTypePB.property;
        *imageDataSize = getPropertyByTypePB.propertyInfo.dataSize;
    }
    return err;
}

//////////
//
// GetImageData
//
// Returns the image data for a given object
//
//////////

Ptr GetImageData (ICAObject image)
{
    ICAGetPropertyByTypePB getPropertyByTypePB;
    ICAGetPropertyDataPB getPropertyDataPB;
    OSErr err;
    Ptr dataPtr;
    UInt32 dataSize;
    dataPtr = nil;
    
    memset(&getPropertyByTypePB, 0, sizeof(ICAGetPropertyByTypePB));
    getPropertyByTypePB.object = image;
    getPropertyByTypePB.propertyType = kICAPropertyImageData;
    err = ICAGetPropertyByType (&getPropertyByTypePB, nil);
    if (noErr == err)
    {
        dataSize= getPropertyByTypePB.propertyInfo.dataSize;
        dataPtr = malloc(dataSize);
        if (dataPtr)
        {
            memset(&getPropertyDataPB, 0, sizeof(ICAGetPropertyDataPB));
            getPropertyDataPB.property = getPropertyByTypePB.property;
            getPropertyDataPB.startByte = 0;
            getPropertyDataPB.dataPtr = dataPtr;
            getPropertyDataPB.requestedSize = dataSize;
            getPropertyDataPB.dataType = getPropertyByTypePB.propertyInfo.dataType;
            err = ICAGetPropertyData(&getPropertyDataPB, nil);
            if (noErr != err)
            {
                free(dataPtr);
                dataPtr = nil;
            }
        }
    }
    return dataPtr;
}

//////////
//
// DeleteImageFromCamera
//
// Delete the image from the camera
//
//////////

OSErr DeleteImageFromCamera (ICAObject imageObj)
{
    ICAObjectSendMessagePB objectSendMessagePB;
    OSErr err;
    
    memset(&objectSendMessagePB, 0, sizeof(ICAObjectSendMessagePB));
    objectSendMessagePB.object = imageObj;
    objectSendMessagePB.message.messageType = kICAMessageCameraDeleteOne;
    err = ICAObjectSendMessage (&objectSendMessagePB, nil);
    
    return err;
}

//////////
//
// HandleDeviceAdded
//
// Callback function for new devices added
//
//////////

void HandleDeviceAdded(ICAObject object)
{
    // your code goes here...
}

//////////
//
// HandleDeviceAdded
//
// Callback function for removing devices
//
//////////

void HandleDeviceRemoved(ICAObject object)
{
    // your code goes here...
}

//////////
//
// NotificationCallback
//
//////////

void NotificationCallback(ICAHeader * pb)
{
    ICARegisterEventNotificationPB* thePB = (ICARegisterEventNotificationPB*)pb;
    switch (thePB->notifyType)
    {
        case kICAEventObjectAdded:
            // a new device was added
            HandleDeviceAdded(thePB->object);
        break;
        
        case kICAEventObjectRemoved:
            // an existing device was removed
            HandleDeviceRemoved(thePB->object);
        break;
    }
}

//////////
//
// SetupNotification
//
//////////

OSErr SetupNotification()
{
    ICARegisterEventNotificationPB registerEventNotificationPB;
    OSErr err;
    
    memset(&registerEventNotificationPB, 0, sizeof(ICARegisterEventNotificationPB));
    registerEventNotificationPB.object = NULL;
    registerEventNotificationPB.notifyType = kICAEventObjectAdded;
    registerEventNotificationPB.notifyProc = (ICACompletion)NotificationCallback;
    err = ICARegisterEventNotification(&registerEventNotificationPB, nil);
    
    require(err==noErr, bail);
    memset(&registerEventNotificationPB, 0, sizeof(ICARegisterEventNotificationPB));
    registerEventNotificationPB.object = NULL;
    registerEventNotificationPB.notifyType = kICAEventObjectRemoved;
    registerEventNotificationPB.notifyProc = (ICACompletion)NotificationCallback;
    err = ICARegisterEventNotification(&registerEventNotificationPB, nil);
    
    bail:
    return err;
}

//////////
//
// DownloadFileToDesktopFolder
//
// Download the image to the desktop folder
//
//////////

OSErr DownloadFileToDesktopFolder (ICAObject imageObject)
{
    OSErr err = noErr;
    ICADownloadFilePB downloadFilePB;
    FSRef desktopFolderFSRef;
    
    FSFindFolder(kOnAppropriateDisk, kDesktopFolderType, false, &desktopFolderFSRef);
    memset(&downloadFilePB, 0, sizeof(ICADownloadFilePB));
    downloadFilePB.object = imageObject;
    downloadFilePB.dirFSRef = &desktopFolderFSRef;
    downloadFilePB.flags = kCreateCustomIcon | kAddMetaDataToFinderComment;
    err = ICADownloadFile(&downloadFilePB, nil);
    
    return err;
}

//////////
//
// GetDeviceName
//
// Get the name of the device
//
//////////

OSErr GetDeviceName (ICAObject deviceObject,
                    char* nameBuffer,
                    UInt32 bufferSize)
{
    OSErr err = noErr;
    ICACopyObjectPropertyDictionaryPB dictionaryPB;
    CFDictionaryRef dict = NULL;
    
    memset(&dictionaryPB, 0, sizeof(ICACopyObjectPropertyDictionaryPB));
    dictionaryPB.object = deviceObject;
    dictionaryPB.theDict= &dict;
    err = ICACopyObjectPropertyDictionary(&dictionaryPB, nil);
    if (err == noErr)
    {
        CFStringRef str;
        str = (CFStringRef)CFDictionaryGetValue(dict, CFSTR("ifil"));
        if (str)
        {
            CFStringGetCString(str, nameBuffer, bufferSize, kCFStringEncodingASCII);
        }
        CFRelease(dict);
    }
    return err;
}

//////////
//
// getNthChild
//
// Get the nth child for the given object
//
//////////

ICAObject getNthChild(ICAObject object, UInt32 index)
{
    ICAGetNthChildPB 	getNthChildPB;
    OSErr		err;
    
    memset(&getNthChildPB, 0, sizeof(ICAGetNthChildPB));
    getNthChildPB.header.refcon = 0;
    getNthChildPB.parentObject  = object;
    getNthChildPB.index         = index;
    err = ICAGetNthChild(&getNthChildPB, NULL);
    
    if (err == noErr)
    {
        return getNthChildPB.childObject;
    }
    else
    {
        return NULL;
    }
}

//////////
//
// getObjectInfo
//
// Get the ICAObjectInfo for the given object
//
//////////

ICAObjectInfo* getObjectInfo(ICAObject object)
{
    ICAGetObjectInfoPB 	getObjectInfoPB;
    OSErr		err;
    
    memset(&getObjectInfoPB, 0, sizeof(ICAGetObjectInfoPB));
    getObjectInfoPB.header.refcon = 0;
    getObjectInfoPB.object = object;
    err = ICAGetObjectInfo(&getObjectInfoPB, NULL);
    
    if (err == noErr)
    {
        ICAObjectInfo *objectInfoPtr = (ICAObjectInfo* )NewPtr(sizeof(struct ICAObjectInfo));
        BlockMove(&getObjectInfoPB.objectInfo, objectInfoPtr, sizeof(struct ICAObjectInfo));
        return objectInfoPtr;
    }
    else
    {
        return nil;
    }
}

