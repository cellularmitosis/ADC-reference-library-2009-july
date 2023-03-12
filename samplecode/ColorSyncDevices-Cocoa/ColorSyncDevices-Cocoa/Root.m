//////////
//
//	File:		Root.m
//
//	Contains:	Implementation file for the Root class.
//              This class is responsible for using ColorSync routines
//              to build up a list of ColorSync device objects.
//
//	Written by:	Apple Developer Technical Support
//
//	Copyright:	© 2002 by Apple Computer, Inc., all rights reserved.
//
//	Change History (most recent first):
//	   
//	   <1>	 	10/7/02	srk		first file
//	   <2>	 	12/11/02 srk	improved NSOutlineView interaction
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

#import "Root.h"


OSErr IterateDeviceInfo (const CMDeviceInfo *deviceInfo, void *refCon);

@implementation Root

//////////
//
// Root
//
// Class method to create a Root class object
//
//////////

static Root *rootItem = nil;

+ (Root *)rootItem 
{
    if (rootItem == nil) 
    {
        rootItem = [[Root alloc] init];
    }
    
    return rootItem;       
}

//////////
//
// init
//
//////////

-(id)init
{    
    [super init];
    
        /* iterate over all ColorSync devices and make a child node object for each device */
    NSAssert([self iterateDevices] == noErr, @"CMIterateColorDevices error");

    return self;
}

//////////
//
// iterateDevices
//
// Use CMIterateColorDevices to iterate all ColorSync devices
//
//////////

-(CMError)iterateDevices
{
	UInt32	seed = 0;
	UInt32	count = 0;
        
	seed = count = 0;
        /* build a list of all ColorSync devices */
	return (CMIterateColorDevices(&IterateDeviceInfo, &seed, &count, self));
}

//////////
//
// addChildNode
//
// add the object to our array of child objects for this node
//
//////////


-(void)addChildNode:(NSObject *)objectToAdd
{
    [childNodes addObject:objectToAdd];
}

//////////
//
// canSelectAndDisplayInfo
//
// indicates whether this object can be selected in the
// NSOutlineView and display info.
//
//////////

-(BOOL)canSelectAndDisplayInfo
{
    return NO;
}


@end

//////////
//
// IterateDeviceInfo
//
// The caller-supplied iterator function for CMIterateColorDevices
//
//////////

OSErr IterateDeviceInfo (const CMDeviceInfo *deviceInfo, void *refCon)
{
    Root *rootObj = (Root *)(refCon);	// refCon is the Root object    

        /* we must first retain the CFDictionary of localized device names passed to us 
            here in the deviceInfo.deviceName field because we will be accessing
            the values in it at a later time */
    if (deviceInfo->deviceName)
    {
        CFRetain(*(deviceInfo->deviceName));
    }

        /* create a ColorDevice object for this ColorSync device */
    ColorDevice *colorDevice = [[ColorDevice alloc] initWithCMDeviceInfo: deviceInfo];
    if (colorDevice)
    {
            /* each ColorDevice object is stored as a child node of this Root object */
        [rootObj addChildNode:colorDevice];
            /* object is now in our child node array so release it... */
        [colorDevice release];
    }

	return noErr;
}

