//////////
//
//	File:		ColorDevice.m
//
//	Contains:	Implementation file for the ColorDevice class.
//
//              Creates a ColorDevice object with information taken from the CMDeviceInfo
//              for the ColorSync device
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

#import "ColorDevice.h"
#import "ProfileCategories.h"
#import "Utils.h"

@implementation ColorDevice


//////////
//
// initWithCMDeviceInfo
//
// Init a ColorDevice object with information taken from the CMDeviceInfo
// for this ColorSync device
//
//////////

-(id)initWithCMDeviceInfo:(const CMDeviceInfo *)deviceInfoInit
{

    [super init];

    if (deviceInfoInit)
    {
            /* save the passed CMDeviceInfo information */
        [self setCMDeviceInfo:deviceInfoInit];
            /* build a string containing all of the names in the dictionary */
        [self makeDeviceNamesStringFromDictionary:*(deviceInfoInit->deviceName)];
            /* set display string for NSOutlineView list item to the name for the first object in the device
                name dictionary */
        [self setDisplayString:[Utils valueForFirstObjectInDictionary:(NSDictionary *)*(deviceInfoInit->deviceName)]];
            /* build the profile categories objects ("currentProfile", "factoryProfile", etc.) */
        [self buildProfileCategoriesObjects];
    }

    return self;
}

//////////
//
// buildProfileCategoriesObjects
//
// build an array of profile category objects for the strings i.e. "customProfiles",
// "currentProfiles", etc. which will all be made as child objects of this
// device object in the NSOutlineView
//
//////////

-(void)buildProfileCategoriesObjects
{
    NSArray *profileCategoryNames = nil;
    int i;

        /* build an array of profile category strings i.e. "customProfiles",
            "currentProfiles", etc. which will all be made as child objects of this
            device object in the NSOutlineView */
    profileCategoryNames = [NSArray arrayWithObjects:@"customProfiles", @"currentProfiles", @"factoryProfiles", nil];

        /* create ProfileCategories objects for each of these profileCategoryNames */
    for (i=0;i<[profileCategoryNames count];++i)
    {
        ProfileCategories *profCatObj = nil;

            /* create a ProfileCategories object for this profile category item */
        profCatObj = [[ProfileCategories alloc] initWithName:[profileCategoryNames objectAtIndex:i] deviceInfo:self];
            /* display it only if there are profiles for this category */
        if ([profCatObj profileCount])
        {
                /* each ProfileCategories object is saved as a child node of the ColorDevice object */
            [childNodes addObject:profCatObj];
                /* object is added to our array of child nodes to let's release it... */
            [profCatObj release];
        }
        else
        {
                /* there are no profiles for this category so we can get rid of
                    the object */
            [profCatObj release];
        }
    }
}

//////////
//
// makeDeviceNamesStringFromDictionary
//
// build a string containing all of the names in the dictionary
//
//////////

-(void)makeDeviceNamesStringFromDictionary:(CFDictionaryRef)dict
{
    /* Use the CMDeviceInfo.deviceName dictionary to build one string containing all 
        the Device key/name pairs (key = language code, name = device name string)
        for example:
        
        "en_US-Color LCD, de_DE_Farb-LCD, ..." 
    */
    
    if (dict != NULL)
    {
        deviceNamesStrings = [Utils makeNSStringFromCFDictionaryRef:dict];
    }
    else
    {
            /* no names in the dictionary... */
        deviceNamesStrings = @"<null>";
    }
    [deviceNamesStrings retain];
}

//////////
//
// setCMDeviceInfo
//
// set (save) the CMDeviceInfo for this object
//
//////////

-(void)setCMDeviceInfo:(const CMDeviceInfo *)newDeviceInfo
{
        /* save the passed CMDeviceInfo information */
    memcpy((void *)&deviceInfo, newDeviceInfo, sizeof(struct CMDeviceInfo));
}

//////////
//
// deviceID
//
// Getter function which returns the deviceID for this ColorSync device.
//
//////////

-(CMDeviceID)deviceID
{
    return deviceInfo.deviceID;
}

//////////
//
// deviceClass
//
// Getter function which returns the deviceClass for this ColorSync device.
//
//////////

-(CMDeviceClass)deviceClass
{
    return deviceInfo.deviceClass;
}

//////////
//
// dealloc
//
//////////

-(void)dealloc
{
    if (deviceInfo.deviceName)
    {
        CFRelease(*(deviceInfo.deviceName));
    }

    [deviceNamesStrings release];
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
    return YES;
}

//////////
//
// deviceNamesStr
//
// Getter function which returns the name string for this ColorSync device.
//
//////////

-(NSString *)deviceNamesString
{
    return [[deviceNamesStrings retain] autorelease];
}

//////////
//
// deviceClassString
//
// Getter function which returns the class string for this ColorSync device.
//
//////////

-(NSString *)deviceClassString
{
    return [Utils makeNSStringForDeviceClass:&(deviceInfo.deviceClass)];
}

//////////
//
// deviceScopeString
//
// Getter function which returns the scope string for this ColorSync device.
//
//////////

-(NSString *)deviceScopeString
{
    return [Utils makeNSStringForDeviceScope:&(deviceInfo.deviceScope)];
}

//////////
//
// profileNamesString
//
// Dummy Getter function for profile name - this object only shows device info.
//
//////////

-(NSString *)profileNamesString
{
    return @"";
}

//////////
//
// profileLocString
//
// Dummy Getter function for profile loc string - this object only shows device info.
//
//////////

-(NSString *)profileLocString
{
    return @"";
}

//////////
//
// profileScopeString
//
// Dummy Getter function for profile scope string - this object only shows device info.
//
//////////

-(NSString *)profileScopeString
{
    return @"";
}

@end
