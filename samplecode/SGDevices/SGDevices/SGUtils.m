//////////
//
//	File:		SGUtils.m
//
//	Contains:	Implementation file for the SGUtils class.
//
//	Written by:	Apple Developer Technical Support
//
//	Copyright:	© 2002 by Apple Computer, Inc., all rights reserved.
//
//	Change History (most recent first):
//	   
//	   <1>	 	10/7/02	srk		first file
//
//////////

/*
 IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc. ("Apple") in
 consideration of your agreement to the following terms, and your use, installation, 
 modification or redistribution of this Apple software constitutes acceptance of these 
 terms.  If you do not agree with these terms, please do not use, install, modify or 
 redistribute this Apple software.
 
 In consideration of your agreement to abide by the following terms, and subject to these 
 terms, Apple grants you a personal, non-exclusive license, under AppleÕs copyrights in 
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

#import "SGUtils.h"


@implementation SGUtils

//////////
//
// init
//
//////////

-(id)init
{
    if (self == [super init])
    {
        videoChannel = NULL;
        seqGrabber = NULL;
        deviceList = NULL;
        
        [self makeSGDeviceListForVideoChannel];
    }
    return self;
}

//////////
//
// makeSGDeviceListForVideoChannel
//
// Create a video channel and build a device list using this channel
//
//////////

-(void)makeSGDeviceListForVideoChannel
{
	ComponentDescription	theDesc;
	Component				sgCompID;
    ComponentResult 		result;
	
    // Find and open a sequence grabber
    theDesc.componentType			= SeqGrabComponentType;
    theDesc.componentSubType 		= 0L;
    theDesc.componentManufacturer 	= 'appl';
    theDesc.componentFlags 			= 0L;
    theDesc.componentFlagsMask 		= 0L;	
    sgCompID = FindNextComponent (NULL, &theDesc);
    if (sgCompID != 0)
    {
        seqGrabber = OpenComponent (sgCompID);
        if (seqGrabber)
        {
            result = SGInitialize (seqGrabber);
            if (result == noErr)
            {				
                // Get a video channel
                result = SGNewChannel (seqGrabber, VideoMediaType, &videoChannel);
                if (result == noErr)
                {
                    /* build a device list */
                    result = SGGetChannelDeviceList(videoChannel,sgDeviceListIncludeInputs,&deviceList);
                }
                else	/* error!! */
                {
                    int choice;

                    choice = NSRunAlertPanel(@"Error", @"Couldn't create video channel - please connect a video device and try again", @"OK", nil, nil);
                    [NSApp terminate:self];
                }
            }
        }

    }
    
    NSAssert(seqGrabber != 0, @"FindNextComponent failed");
    NSAssert(sgCompID != 0, @"FindNextComponent failed");
    NSAssert(result == noErr, @"SG call failed");
}

//////////
//
// getDeviceNameCount
//
// Returns the number of SGDeviceName structs - this is
// found from the count field in the SGDeviceListRecord struct:
//
// struct SGDeviceListRecord {
//  short               count;
//  short               selectedIndex;
//  long                reserved;               /* zero*/
//  SGDeviceName        entry[1];
// };
//
//
//////////

-(short)getDeviceNameCount
{
    NSAssert(deviceList != nil, @"nil device list");
    
    return (*deviceList)->count;
}

//////////
//
// returnDeviceName
//
// Returns the string found in name field of the SGDeviceName structure for
// the specified SGDeviceName entry in the SGDeviceListRecord:
//
// struct SGDeviceName {
//  Str63               name;
//  Handle              icon;
//  long                flags;
//  long                refCon;
//  SGDeviceInputList   inputs;                 /* list of inputs; formerly reserved to 0*/
// };
//
// struct SGDeviceListRecord {
//  short               count;
//  short               selectedIndex;
//  long                reserved;               /* zero*/
//  SGDeviceName        entry[1];
// };
//
//
//////////

-(void)returnDeviceName:(Str63)name atIndex:(int)index
{
    SGDeviceName nameRec;
    
    NSAssert(deviceList != nil, @"nil device list");
    NSAssert(index < (*deviceList)->count, @"index out of range");

    nameRec =(*deviceList)->entry[index];
    memcpy(name, nameRec.name, sizeof(Str63));
}

//////////
//
// getDeviceInputNameCount
//
// Returns the count of the SGDeviceInputName structs corresponding to
// the given SGDeviceName struct 
//
// struct SGDeviceName {
//  Str63               name;
//  Handle              icon;
//  long                flags;
//  long                refCon;
//  SGDeviceInputList   inputs;                 /* list of inputs; formerly reserved to 0*/
// };
//
// struct SGDeviceInputListRecord {
//  short               count;
//  short               selectedIndex;
//  long                reserved;               /* zero*/
//  SGDeviceInputName   entry[1];
//};
//
//////////

-(short)getDeviceInputNameCount:(short)deviceNameIndex
{
    SGDeviceName nameRec;
    SGDeviceInputList deviceInputList = NULL;

    NSAssert(deviceList != nil, @"nil device list");    
    NSAssert(deviceNameIndex < (*deviceList)->count, @"index out of range");

    nameRec =(*deviceList)->entry[deviceNameIndex];
    deviceInputList = nameRec.inputs;
    if (deviceInputList)
    {
        return ((*deviceInputList)->count);
    }
    else
    {
        return 0;
    }
}

//////////
//
// returnDeviceInputName
//
// Returns the name from the SGDeviceInputName struct corresponding to
// the given SGDeviceName struct 
//
// struct SGDeviceInputName {
//  Str63               name;
//  Handle              icon;
//  long                flags;
//  long                reserved;               /* zero*/
// };
//
// struct SGDeviceName {
//  Str63               name;
//  Handle              icon;
//  long                flags;
//  long                refCon;
//  SGDeviceInputList   inputs;                 /* list of inputs; formerly reserved to 0*/
// };
//
//////////

-(void)returnDeviceInputName:(Str63)name atIndex:(int)index forDeviceNameIndex:(int)deviceNameIndex
{
    SGDeviceName nameRec;
    SGDeviceInputList deviceInputList = NULL;
    
    NSAssert(name != nil, @"bad name parameter");

    nameRec =(*deviceList)->entry[deviceNameIndex];
    deviceInputList = nameRec.inputs;
    if (deviceInputList)
    {
        SGDeviceInputName inputNameRec;
        
        inputNameRec = (*deviceInputList)->entry[index];
        memcpy(name, inputNameRec.name, sizeof(Str63));
    }  
}

//////////
//
// dealloc
//
// Free up our device list 
//
//////////

-(void)dealloc
{
    SGDisposeDeviceList(seqGrabber, deviceList);
}

@end
