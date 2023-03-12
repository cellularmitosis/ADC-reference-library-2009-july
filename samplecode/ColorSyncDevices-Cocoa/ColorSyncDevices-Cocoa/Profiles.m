//////////
//
//	File:		Profiles.m
//
//	Contains:	Implementation file for the Profiles class.
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

#import "Profiles.h"


@implementation Profiles

//////////
//
// initWithData
//
// Init a Profiles object with information taken from the ColorDevice object
// for this profiles ColorSync device
//
//////////

-(id)initWithData:(const NCMDeviceProfileInfo *)theProfileInfo devObj:(ColorDevice *)deviceObj
{
    [super init];
    if (theProfileInfo)
    {
            /* profileName is the dictionary of all the names for this profile */
        NSString *firstProfileName;

            /* save the passed NCMDeviceProfileInfo information */
        [self setProfileInfo:theProfileInfo];
    
            /* save the passed ColorDevice object - we'll use it when we want to display
                in the window more specific device information for the profile */
        [self setParentObject:deviceObj];
                
            /* Get the first profile name in the list - we will use it to display in
                our NSOutlineView for this item */
        firstProfileName = [Utils valueForFirstObjectInDictionary:(NSDictionary *)(theProfileInfo->profileName)];
        if (firstProfileName)
        {
            NSString *profileNameHeader = nil;
                /* pre-pend "name[1]: " in front of the name string to indicate we are
                    showing the first string in the array */
            profileNameHeader = @"name[1]: ";
                /* specify the first profile name for display in the NSOutlineView for this list item */
            [self setDisplayString:[profileNameHeader stringByAppendingString:firstProfileName]];
        }
        else
        {
                /* no profile name was found */
            [self setDisplayString:@"<none>"];
        }
    }

    return self;
}

//////////
//
// setProfileInfo
//
// save the passed NCMDeviceProfileInfo information
// with this object so we can access it later.
//
//////////

-(void)setProfileInfo:(const NCMDeviceProfileInfo *)theProfileInfo
{
        /* save the passed NCMDeviceProfileInfo information */
    memcpy((void *)&profileInfo, theProfileInfo, sizeof(struct NCMDeviceProfileInfo));
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
    parentDeviceObj = colorDeviceObj;
    [parentDeviceObj retain];
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
// profileNamesString
//
// Getter function which returns the profile names string
//
//////////

-(NSString *)profileNamesString
{
    NSString *profNamesStr = nil;
    
    profNamesStr = [Utils makeNSStringFromCFDictionaryRef:(profileInfo.profileName)];
    if (profNamesStr)
    {
        return profNamesStr;
    }
    else
    {
        return @"<null>";
    }
}

//////////
//
// profileLocString
//
// Getter function which returns the profile loc. string
//
//////////

-(NSString *)profileLocString
{
    return [Utils makeNSStringForProfileLoc:&profileInfo.profileLoc];
}

//////////
//
// profileScopeString
//
// Getter function which returns the profile scope string
//
//////////

-(NSString *)profileScopeString
{
    return [Utils makeNSStringForDeviceScope:&(profileInfo.profileScope)];
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
    return [parentDeviceObj deviceNamesString];
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
    return [parentDeviceObj deviceClassString];
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
    return [parentDeviceObj deviceScopeString];
}

//////////
//
// dealloc
//
//////////

-(void)dealloc
{
        /* release the CFDictionary of localized profile names */
    if (profileInfo.profileName)
    {
        CFRelease(profileInfo.profileName);
    }
}


@end
