//////////
//
//	File:		SimpleDownloadFirst.c
//
//	Contains:	Implementation file for SimpleDownloadFirstImage. Contains code which
//              acquires images from Apple Image Capture supported devices.
//
//	Written by:	Apple Image Capture Engineering
//
//	Copyright:	© 2003 by Apple Computer, Inc., all rights reserved.
//
//	Change History (most recent first):
//	   
//	   <1>	 	10/10/01	hwn		first file
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
#include <CoreFoundation/CoreFoundation.h>

#include "SimpleDownloadFirst.h"

//#define     kShowStatusMessages

void gotChildCount(ICAHeader* pb);
void GetChildCount(ICAObject icaObject);
void GetNthChild(ICAObject  parentObject,
                 UInt32		index,
                 UInt32		max);

// ————————————————————————————————————————————————————————————————————————————————————————————————
UInt32						gSimpleControllerRefCon = 0;
CFDictionaryRef				gDeviceDict = nil;
CFDictionaryRef				gImageDict = nil;
FSRef			 			gFileFSRef;
char						gCurrentFileName[32];

// ————————————————————————————————————————————————————————————————————————————————————————————————
//	imageDownloaded
// ————————————————————————————————————————————————————————————————————————————————————————————————
void imageDownloaded(ICAHeader* pb)
{
    char 				path[256];
    OSErr				err;
    ICADownloadFilePB* 	downloadedPB = (ICADownloadFilePB*)pb;

    if (dupFNErr == pb->err)
    {
    #ifdef kShowStatusMessages
        printf("image '%s' was already on disk (err = %d)\n", gCurrentFileName, pb->err);
        sprintf(path, "/tmp/%s", gCurrentFileName);
    #endif

		// You must define ImageWasDownloaded in your application,
		// and it will be called once an Image Capture device is
		// connected
        ImageWasDownloaded(gSimpleControllerRefCon, path);
    } else if (noErr == pb->err)
    {
        err = FSRefMakePath(downloadedPB->fileFSRef, path, 255);
        if (noErr == err)
        {
    #ifdef kShowStatusMessages
            printf("downloaded file = '%s'\n", path);
    #endif

            ImageWasDownloaded(gSimpleControllerRefCon, path);
        }
    } else
    {
    #ifdef kShowStatusMessages
        printf("error = %d\n", err);
    #endif
    }
}

// ————————————————————————————————————————————————————————————————————————————————————————————————
//	gotTheDeviceDict
// ————————————————————————————————————————————————————————————————————————————————————————————————
void gotTheDeviceDict(ICAHeader* pb)
{
    ICACopyObjectPropertyDictionaryPB* 	propPB = (ICACopyObjectPropertyDictionaryPB*)pb;
    CFArrayRef 							array;

    #ifdef kShowStatusMessages
    printf("got the dict\n");
    #endif

    array = CFDictionaryGetValue(*propPB->theDict, CFSTR("data"));
    if (array)
    {
        if (CFArrayGetCount(array))
        {
            CFDictionaryRef firstImage;

            firstImage = CFArrayGetValueAtIndex(array, 0);
            if (firstImage)
            {
                ICAObject 	object;
                CFNumberRef	objectRef;
                CFStringRef objectName;
                
                objectName = CFDictionaryGetValue(firstImage, CFSTR("ifil"));
                if (objectName)
                {
                    CFStringGetCString(objectName, gCurrentFileName, 32, kCFStringEncodingMacRoman);
                }

                objectRef = CFDictionaryGetValue(firstImage, CFSTR("icao"));
                if (CFNumberGetValue(objectRef, kCFNumberLongType, &object))
                {
                    ICADownloadFilePB 	pb;
                    FSRef			  	dirRef;
                    OSErr				err;

                    FSPathMakeRef("/tmp/", &dirRef, NULL);

    #ifdef kShowStatusMessages
                    printf("got the first image object: %08lX\n", (UInt32)object);
    #endif
                    
                    memset(&pb, 0, sizeof(ICADownloadFilePB));
                    pb.object   = object;
                    pb.dirFSRef = &dirRef;
                    pb.fileFSRef= &gFileFSRef;
    #ifdef kShowStatusMessages
                    printf("downloading image...\n");
    #endif
                    err = ICADownloadFile(&pb, imageDownloaded);
					
					// also look at the image obj's property data
					ICACopyObjectPropertyDictionaryPB 	propPB;
					
					memset(&propPB, 0, sizeof(ICACopyObjectPropertyDictionaryPB));
					propPB.object  = object;
					propPB.theDict = &gImageDict;
					err = ICACopyObjectPropertyDictionary(&propPB, nil);
    #ifdef kShowStatusMessages
					CFShow(gImageDict);
    #endif
                }
            }
        }
    }
	if (gImageDict)
		CFRelease(gImageDict);
	if (gDeviceDict)
		CFRelease(gDeviceDict);
}

// ————————————————————————————————————————————————————————————————————————————————————————————————
//	gotTheDevice
// ————————————————————————————————————————————————————————————————————————————————————————————————
void gotTheDevice(ICAHeader* pb)
{
    ICAGetNthChildPB* 	getNthChildPB  = (ICAGetNthChildPB*)pb;
    ICAObject 			device 			= getNthChildPB->childObject;
    OSErr				err;
    
    if (device)
    {
        ICACopyObjectPropertyDictionaryPB 	propPB;
        
        memset(&propPB, 0, sizeof(ICACopyObjectPropertyDictionaryPB));
        propPB.object  = device;
        propPB.theDict = &gDeviceDict;
        err = ICACopyObjectPropertyDictionary(&propPB, gotTheDeviceDict);
    }
    else
    {
        NoCameraDevicesFound(gSimpleControllerRefCon);
    }
}

// ————————————————————————————————————————————————————————————————————————————————————————————————
//	gotTheDeviceList
// ————————————————————————————————————————————————————————————————————————————————————————————————
void gotTheDeviceList(ICAHeader* pb)
{
    ICAGetDeviceListPB * getDevListPB = (ICAGetDeviceListPB*)pb;
    ICAObject			 deviceList = getDevListPB->object;

    #ifdef kShowStatusMessages
    printf("deviceList = %08lX\n", (UInt32)deviceList);
    #endif

    if (deviceList)
    {
        ICAGetNthChildPB 	getNthChildPB;
        OSErr				err;

        memset(&getNthChildPB, 0, sizeof(ICAGetNthChildPB));
        getNthChildPB.parentObject  = deviceList;
        getNthChildPB.index         = 0;
        err = ICAGetNthChild(&getNthChildPB, gotTheDevice);
    }
}

// ————————————————————————————————————————————————————————————————————————————————————————————————
//	SimpleDownloadFirstImage
// ————————————————————————————————————————————————————————————————————————————————————————————————
void SimpleDownloadFirstImage(UInt32 refCon)
{
    ICAGetDeviceListPB 	getDeviceListPB;
    OSErr				err;

    gSimpleControllerRefCon = refCon;

    memset (&getDeviceListPB, 0, sizeof(ICAGetDeviceListPB));
    err = ICAGetDeviceList (&getDeviceListPB, gotTheDeviceList);
}
