//////////
//
//	File:		ProfileCategories.m
//
//	Contains:	Implementation file for the ProfileCategories class.
//
//              Creates a ProfileCategories object with information taken from a ColorDevice object
//              for the profiles ColorSync device
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

#import "ProfileCategories.h"

OSErr IterateDeviceProfile (const CMDeviceInfo *deviceInfo, const NCMDeviceProfileInfo * profileInfo, void *refCon);

@implementation ProfileCategories

//////////
//
// initWithName
//
// Init a ProfileCategories object with information taken from the ColorDevice object
// for this profiles ColorSync device
//
//////////

-(id)initWithName:(NSString *)profileTypeName deviceInfo:(ColorDevice *)deviceInfo
{
    if (self == [super init])
    {
            /* save profile type name ("currentProfile", etc.) for display */
        [self setDisplayString:profileTypeName];
        
            /* save parent ColorDevice object */
        [self setParentObject:deviceInfo];
        
            /* initialize count of number of profiles for this category to zero */
        profilesCount = 0;

            /* Use the actual profile category name as a key. We'll call the method in 
                this object's class corresponding to the passed key i.e. if "currentProfile"
                is passed as the key, call the currentProfile method in this class
                to iterate all the profiles for the category */
        [self valueForKey:profileTypeName];
        
    }
    return self;
}

//////////
//
// saveParentObject
//
// save parent ColorDevice object
//
//////////

-(void)setParentObject:(ColorDevice *)colorDeviceObj
{
        /* save parent ColorDevice object */
    savedColorDevice = colorDeviceObj;
    [savedColorDevice retain];
}

//////////
//
// setProfilesCount
//
// saved count of the number of profiles for this category
//
//////////

-(void)setProfilesCount:(UInt32)count
{
    profilesCount = count;
}

//////////
//
// iterateProfiles
//
// Iterate over all the profiles for the given device
//
//////////

-(CMError)iterateProfiles:(UInt32)flag
{
    UInt32 seed = 0;
    UInt32 count = 0;
    CMError err = noErr;
    
        /* iterate all the profiles for the given device */
    err = CMIterateDeviceProfiles(&IterateDeviceProfile, &seed, &count, flag, self);
    
        /* save profile count */
    [self setProfilesCount:count];
    
    return (err);
}

//////////
//
// currentProfiles
//
// Call iterateProfiles to iterate over all the current profiles for the given device
//
//////////

-(void)currentProfiles
{
    [self iterateProfiles:cmIterateCurrentDeviceProfiles];
}

//////////
//
// customProfiles
//
// Call iterateProfiles to iterate over all the custom profiles for the given device
//
//////////

-(void)customProfiles
{
    [self iterateProfiles:cmIterateCustomDeviceProfiles];
}

//////////
//
// factoryProfiles
//
// Call iterateProfiles to iterate over all the factory profiles for the given device
//
//////////

-(void)factoryProfiles
{
    [self iterateProfiles:cmIterateFactoryDeviceProfiles];
}

//////////
//
// profilesCount
//
//////////

-(UInt32)profileCount
{    
    return profilesCount;
}


//////////
//
// savedColorDevice
//
// Getter function - returns the parent ColorDevice object.
//
//////////

-(ColorDevice *)savedColorDevice
{
    return [[savedColorDevice retain] autorelease];
}

//////////
//
// dealloc
//
//////////

-(void)dealloc
{
    [savedColorDevice release];
    
    [super dealloc];
}


//////////
//
// canSelect
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
// IterateDeviceProfile
//
// The caller-supplied iterator function for CMIterateDeviceProfiles
//
//////////

OSErr IterateDeviceProfile (const CMDeviceInfo *deviceInfo, const NCMDeviceProfileInfo * profileInfo, void *refCon)
{
    ProfileCategories *profileCategoriesObject = (ProfileCategories *)refCon;	// refCon is the ProfileCategories object
    ColorDevice *parentColorDeviceObject = [profileCategoriesObject savedColorDevice];
    
        /* the iterator returns information for _all_ devices, so
            we'll check here to see whether we can match the current
            device. If so, we go ahead and save the information */
    if (([parentColorDeviceObject deviceID] ==  deviceInfo->deviceID) &&
            (([parentColorDeviceObject deviceClass] ==  deviceInfo->deviceClass)))
    {
        Profiles *profilesObject = nil;

            /* we must first retain the CFDictionary of localized profile names passed to us 
                here in the profileInfo.profileName field because we will be accessing
                the values in it at a later time */
        if (profileInfo->profileName)
        {
            CFRetain(profileInfo->profileName);
        }
        
                /* create a Profiles object for this profile name */
        profilesObject = [[Profiles alloc] initWithData:profileInfo devObj:parentColorDeviceObject];
        if (profilesObject)
        {
                /* add the Profiles object as a child node of this ProfileCategories object */
            [profileCategoriesObject addChildObject:profilesObject];
            [profilesObject release];
        }
    }
    
	return noErr;
}

