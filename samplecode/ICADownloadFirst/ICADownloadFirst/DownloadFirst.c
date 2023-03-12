//////////
//
//	File:		DownloadFirst.c
//
//	Contains:	Implementation file for DownloadFirstImage.
//
//	Written by:	Apple Image Capture Engineering
//
//	Copyright:	© 2001-2002 by Apple Computer, Inc., all rights reserved.
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

#include "DownloadFirst.h"
void gotChildCount(ICAHeader* pb);
void GetChildCount(ICAObject icaObject);
void GetNthChild(ICAObject  parentObject,
                 UInt32		index,
                 UInt32		max);

// ————————————————————————————————————————————————————————————————————————————————————————————————
UInt32 		gControllerRefCon = 0;
ICAObject 	gFirstImage = 0;

// ————————————————————————————————————————————————————————————————————————————————————————————————
//	gotImageData
// ————————————————————————————————————————————————————————————————————————————————————————————————
void gotImageData(ICAHeader* pb)
{
    ICAGetPropertyDataPB*	gotPropertyDataPB = (ICAGetPropertyDataPB*)pb;

    if (gotPropertyDataPB->actualSize)
    {
        char*	path = "/tmp/ICASample.jpg";
        FILE * 	fi = fopen(path, "w");

        fwrite((Ptr)gotPropertyDataPB->dataPtr, sizeof(UInt8), gotPropertyDataPB->actualSize, fi);
        fclose(fi);
        free(gotPropertyDataPB->dataPtr);

        printf("image was downloaded ('%s')\n", path);

        ImageWasDownloaded(gControllerRefCon, path);
    }
}

// ————————————————————————————————————————————————————————————————————————————————————————————————
//	gotImageDataProperty
// ————————————————————————————————————————————————————————————————————————————————————————————————
void gotImageDataProperty(ICAHeader* pb)
{
    ICAGetPropertyByTypePB* propertyByType 	  = (ICAGetPropertyByTypePB*)pb;
    ICAProperty 			imageDataProperty = propertyByType->property;

    printf("imageDataProperty = %08lX\n", (UInt32)imageDataProperty);

    if (imageDataProperty)
    {
        ICAGetPropertyDataPB	getPropertyDataPB;
        OSErr					err;
        void* 					imageDataPtr;

        imageDataPtr = malloc(propertyByType->propertyInfo.dataSize);
        memset (&getPropertyDataPB, 0, sizeof(ICAGetPropertyDataPB));
        getPropertyDataPB.property 	    = imageDataProperty;
        getPropertyDataPB.startByte	    = 0;
        getPropertyDataPB.requestedSize = propertyByType->propertyInfo.dataSize;
        getPropertyDataPB.dataPtr 		= imageDataPtr;
        err = ICAGetPropertyData(&getPropertyDataPB, gotImageData);
    }
}

// ————————————————————————————————————————————————————————————————————————————————————————————————
//	gotNthChild
// ————————————————————————————————————————————————————————————————————————————————————————————————
void gotNthChild(ICAHeader* pb)
{
    ICAGetNthChildPB* 	getNthChildPB  = (ICAGetNthChildPB*)pb;
    ICAObject 			child 			= getNthChildPB->childObject;


    if (child)
    {
        if (getNthChildPB->childInfo.objectSubtype == kICAFileImage)
        {
            ICAGetPropertyByTypePB 	getPropertyByTypePB;
            OSErr					err;

            printf("got image = %08lX\n", (UInt32)child);
            if (gFirstImage == 0)
            {
                gFirstImage = child;
                printf("found first image: %08lX\n", (UInt32)gFirstImage);
                memset (&getPropertyByTypePB, 0, sizeof(ICAGetPropertyByTypePB));
                getPropertyByTypePB.header.refcon 	= pb->refcon;
                getPropertyByTypePB.object 	   		= child;
                getPropertyByTypePB.propertyType  	= kICAPropertyImageData;
                err = ICAGetPropertyByType(&getPropertyByTypePB, gotImageDataProperty);
            }
        } else
        {
            if (getNthChildPB->header.refcon - 1 > getNthChildPB->index)
            {
                GetNthChild(getNthChildPB->parentObject, getNthChildPB->index+1, getNthChildPB->header.refcon);
            } else
            {
                if (getNthChildPB->childInfo.objectType == kICADirectory)
                {
                    printf("got directory = %08lX\n", (UInt32)child);
                    GetChildCount(child);
                } else if (getNthChildPB->childInfo.objectType == kICADevice)
                {
                    printf("got device = %08lX\n", (UInt32)child);
                    GetChildCount(child);
                }
            }
        }
    } else
    {
        printf("child = 0x00000000\n");
    }
}

// ————————————————————————————————————————————————————————————————————————————————————————————————
//	GetNthChild
// ————————————————————————————————————————————————————————————————————————————————————————————————
void GetNthChild(ICAObject  parentObject,
                 UInt32		index,
                 UInt32		max)
{
    ICAGetNthChildPB 	getNthChildPB;
    OSErr				err;
    
    memset(&getNthChildPB, 0, sizeof(ICAGetNthChildPB));
    getNthChildPB.header.refcon = max;
    getNthChildPB.parentObject  = parentObject;
    getNthChildPB.index         = index;
    err = ICAGetNthChild(&getNthChildPB, gotNthChild);
}

// ————————————————————————————————————————————————————————————————————————————————————————————————
//	gotChildCount
// ————————————————————————————————————————————————————————————————————————————————————————————————
void gotChildCount(ICAHeader* pb)
{
    ICAGetChildCountPB* childCountPB	= (ICAGetChildCountPB*)pb;
    ICAObject 			icaObject		= childCountPB->object;
    UInt32	  			count			= childCountPB->count;

    printf("child count for %08lX = %ld\n", (UInt32)icaObject, count);

    if (count > 0)
    {
        GetNthChild(icaObject, 0, count);
    }
}

// ————————————————————————————————————————————————————————————————————————————————————————————————
//	GetChildCount
// ————————————————————————————————————————————————————————————————————————————————————————————————
void GetChildCount(ICAObject icaObject)
{
    ICAGetChildCountPB 	getChildCountPB;
    OSErr				err;

    memset (&getChildCountPB, 0, sizeof(ICAGetChildCountPB));
    getChildCountPB.object	= icaObject;
    err = ICAGetChildCount(&getChildCountPB, gotChildCount);
}

// ————————————————————————————————————————————————————————————————————————————————————————————————
//	gotDeviceList
// ————————————————————————————————————————————————————————————————————————————————————————————————
void gotDeviceList(ICAHeader* pb)
{
    ICAGetDeviceListPB * getDevListPB = (ICAGetDeviceListPB*)pb;
    ICAObject			 deviceList = getDevListPB->object;

    printf("deviceList = %08lX\n", (UInt32)deviceList);

    if (deviceList)
    {
        GetChildCount(deviceList);
    }
}

// ————————————————————————————————————————————————————————————————————————————————————————————————
//	DownloadFirstImage
// ————————————————————————————————————————————————————————————————————————————————————————————————
void DownloadFirstImage(UInt32 refCon)
{
    ICAGetDeviceListPB 	getDeviceListPB;
    OSErr				err;

    gControllerRefCon = refCon;
    
    memset (&getDeviceListPB, 0, sizeof(ICAGetDeviceListPB));
    err = ICAGetDeviceList (&getDeviceListPB, gotDeviceList);
}

