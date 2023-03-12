//////////
//
//	File:		main.c
//
//	Contains:	ColorSync Devices sample code.
//
//	Written by:	ColorSync Engineering, Apple Developer Technical Support
//
//	Copyright:	© 2002 by Apple Computer, Inc., all rights reserved.
//
//	Change History (most recent first):
//	   
//	   <1>	 	9/10/02  dh, srk		first file
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

//////////
//
// header files
//
//////////

#include <Carbon/Carbon.h>

//////////
//
// prototypes
//
//////////

void PrintFSSpecAsPath (const FSSpec *spec);
void ShowColorSyncDeviceInfo (void);
void DumpDeviceScope (const CMDeviceScope *deviceScope);
void DumpNames(CFDictionaryRef dict);
void DumpDeviceInfo (const CMDeviceInfo *deviceInfo);
void DumpDeviceProfileInfo (const NCMDeviceProfileInfo *profileInfo);
OSErr IterateDeviceInfo (const CMDeviceInfo *deviceInfo, void *refCon);
OSErr IterateDeviceProfile (const CMDeviceInfo *deviceInfo, const NCMDeviceProfileInfo * profileInfo, void *refCon);
OSErr IterateDeviceProfile (const CMDeviceInfo *deviceInfo, const NCMDeviceProfileInfo * profileInfo, void *refCon);

//////////
//
// main
//
//////////

int main(int argc, char *argv[])
{
#pragma unused(argc,argv)

    ShowColorSyncDeviceInfo();
    
    return 0;
}
 

//////////
//
// PrintFSSpecAsPath
//
// Displays the native path name for the fsspec
//
//////////

void PrintFSSpecAsPath (const FSSpec *spec)
{
	OSErr err;
	FSRef newRef;
	UInt8 path[256] = "";
	
	err = FSpMakeFSRef(spec, &newRef);
	if (err == noErr)
	{
		OSStatus status;
		
		status = FSRefMakePath(
							  &newRef,
							  path,
							  256);
		if (status==0 || status==fnfErr)
			printf("%s\n", path);
		else
			printf("FSRefMakePath err=%d\n", (OSErr)status);
	}
	else
		printf("FSpMakeFSRef err=%d\n", err);
	
}


//////////
//
// DumpDeviceScope
//
// Displays the fields of the CMDeviceScope structure which
// specify a device's or a device setting's scope:
//
// struct CMDeviceScope {
//   CFStringRef         deviceUser;             /* kCFPreferencesCurrentUser | _AnyUser */
//   CFStringRef         deviceHost;             /* kCFPreferencesCurrentHost | _AnyHost */
// };
// typedef struct CMDeviceScope            CMDeviceScope;
//
//////////

void DumpDeviceScope (const CMDeviceScope *deviceScope)
{
	if (deviceScope->deviceUser == kCFPreferencesCurrentUser)
		printf("CurUser, ");
	else if (deviceScope->deviceUser == kCFPreferencesAnyUser)
		printf("AnyUser, ");
	else
		printf("???User, ");
	if (deviceScope->deviceHost == kCFPreferencesCurrentHost)
		printf("CurHost\n");
	else if (deviceScope->deviceHost == kCFPreferencesAnyHost)
		printf("AnyHost\n");
	else
		printf("???Host\n");
}

//////////
//
// DumpNamesFunct
//
// Function to display each value in a given dictionary
//
//////////

static void
DumpNamesFunct(	
			const void*				key,
			const void*				val,
			void*					context )
{
#pragma unused (context)
	printf("%s-%s, ", 
		CFStringGetCStringPtr(key, kCFStringEncodingMacRoman),
		CFStringGetCStringPtr(val, kCFStringEncodingMacRoman));
}

//////////
//
// DumpNamesFunct
//
// Specify the callback function to display each value in a given dictionary
//
//////////

void DumpNames(CFDictionaryRef dict)
{
	if (dict) CFDictionaryApplyFunction(dict, DumpNamesFunct, nil);
	printf("\n");
}

//////////
//
// DumpDeviceInfo
//
// Display all fields of the CMDeviceInfo structure:
//
// struct CMDeviceInfo {
//   UInt32              dataVersion;            /* cmDeviceInfoVersion1 */
//   CMDeviceClass       deviceClass;            /* device class */
//   CMDeviceID          deviceID;               /* device ID */
//   CMDeviceScope       deviceScope;            /* device's scope */
//   CMDeviceState       deviceState;            /* Device State flags */
//   CMDeviceProfileID   defaultProfileID;       /* Can change */
//   CFDictionaryRef *   deviceName;             /* Ptr to storage for CFDictionary of */
//                                               /* localized device names (could be nil) */
//   UInt32              profileCount;           /* Count of registered profiles */
//   UInt32              reserved;               /* Reserved for use by ColorSync */
// };
// typedef struct CMDeviceInfo             CMDeviceInfo;
// 
//////////

void DumpDeviceInfo (const CMDeviceInfo *deviceInfo)
{
	printf("--- Device Spec ---\n");
	printf("    dataVersion: %ld\n", deviceInfo->dataVersion);
	printf("    class:       %.4s\n", (char*)&(deviceInfo->deviceClass));
	printf("    ID:          %lu (%0lX)\n", deviceInfo->deviceID, deviceInfo->deviceID);
	printf("    names:       "); DumpNames(*(deviceInfo->deviceName));
	printf("    scope:       "); DumpDeviceScope(&deviceInfo->deviceScope);
	printf("    state:       %ld\n", deviceInfo->deviceState);
	printf("    dflt profID: %ld\n", deviceInfo->defaultProfileID);
	printf("    profCount:   %ld\n", deviceInfo->profileCount);
	printf("    reserved:    %ld\n", deviceInfo->reserved);
}

//////////
//
// DumpDeviceProfileInfo
//
// Display all fields of the CMDeviceInfo structure:
//
// struct NCMDeviceProfileInfo {
//   UInt32              dataVersion;            /* cmDeviceProfileInfoVersion2 */
//   CMDeviceProfileID   profileID;              /* The identifier for this profile */
//   CMProfileLocation   profileLoc;             /* The profile's location */
//   CFDictionaryRef     profileName;            /* CFDictionary of localized profile names */
//   CMDeviceProfileScope  profileScope;         /* The scope this profile applies to */
//   UInt32              reserved;               /* Reserved for use by ColorSync */
// };
// typedef struct NCMDeviceProfileInfo     NCMDeviceProfileInfo;
//
//////////

void DumpDeviceProfileInfo (const NCMDeviceProfileInfo *profileInfo)
{
	printf("    --- Device Profile Info ---\n");
	printf("        dataVersion: %ld\n", profileInfo->dataVersion);
	printf("        profile ID:  %ld\n", profileInfo->profileID);
	printf("        profile Loc: (%d) ", profileInfo->profileLoc.locType);
	if (profileInfo->profileLoc.locType == cmFileBasedProfile)
		PrintFSSpecAsPath(&profileInfo->profileLoc.u.fileLoc.spec);
	else if (profileInfo->profileLoc.locType == cmPathBasedProfile)
		printf("%s\n", profileInfo->profileLoc.u.pathLoc.path);
	else
		printf("???\n");
	printf("        names:       "); DumpNames(profileInfo->profileName);
	printf("        scope:       "); DumpDeviceScope(&profileInfo->profileScope);
	printf("        reserved:    %ld\n", profileInfo->reserved);
}

//////////
//
// IterateDeviceInfo
//
// The caller-supplied iterator function for CMIterateColorDevices
//
//////////

OSErr IterateDeviceInfo (const CMDeviceInfo *deviceInfo, void *refCon)
{
#pragma unused (refCon)

	DumpDeviceInfo(deviceInfo);
    
	return noErr;
}


//////////
//
// IterateDeviceProfile
//
// The caller-supplied iterator function for CMIterateDeviceProfiles
//
//////////

OSErr IterateDeviceProfile (const CMDeviceInfo *deviceInfo, const NCMDeviceProfileInfo * profileInfo, void *refCon)
{
#pragma unused (refCon)

	static CMDeviceInfo lastDev = {};
		
	if (memcmp(&lastDev, deviceInfo, sizeof(CMDeviceInfo)))
    {
		DumpDeviceInfo(deviceInfo);
    }
	lastDev = *deviceInfo;
		
	DumpDeviceProfileInfo(profileInfo);
    
	return noErr;
}


//////////
//
// ShowColorSyncDeviceInfo
//
// Display the following information for Colorsync devices:
//
// • Default device for the "Display" class
// • Device profile for the "Display" class
// • List of ColorSync devices
//   - device current profiles
//   - device custom profiles
//   - device factory profiles
//
//////////

void ShowColorSyncDeviceInfo (void)
{
	CMError			err = noErr;
	UInt32			seed = 0;
	UInt32			count = 0;
	CFAbsoluteTime		before = 0, after = 0;
	CMDeviceID			deviceID = 0;
	CMProfileLocation		profLoc = {};
	char*				name = nil;
	
	printf("--- CMGetDefaultDevice ---\n");
	
	before = CFAbsoluteTimeGetCurrent();
    /* get default display device */
	err = CMGetDefaultDevice('mntr', &deviceID);
	after = CFAbsoluteTimeGetCurrent();
	printf("CMGetDefaultDevice  err=%ld  ID=%ld  time=%f\n", err, deviceID, after-before);
	
	before = CFAbsoluteTimeGetCurrent();
    /* get device profile for default display device */
	err = CMGetDeviceProfile('mntr', 0, 0, &profLoc);
	after = CFAbsoluteTimeGetCurrent();
	if (profLoc.locType == cmFileBasedProfile)
	{
		name = (char*)&profLoc.u.fileLoc.spec.name[1];
		name[profLoc.u.fileLoc.spec.name[0]] = 0;
	}
	else if (profLoc.locType == cmPathBasedProfile)
		name = profLoc.u.pathLoc.path;
	printf("CMGetDeviceProfile  err=%ld  name=%s  time=%f\n", err, name, after-before);
	
	
	printf("\n--- CMIterateColorDevices ---\n");
	seed = count = 0;
	before = CFAbsoluteTimeGetCurrent();
    /* display all ColorSync devices */
	err = CMIterateColorDevices(&IterateDeviceInfo, &seed, &count, 0);
	after = CFAbsoluteTimeGetCurrent();
	printf("CMIterateColorDevices err=%ld  count=%ld  seed=%ld  time=%f\n", err, count, seed, after-before);
	
	
	printf("\n--- CMIterateDeviceProfiles cmIterateCurrentDeviceProfiles ---\n");
	seed = count = 0;
	before = CFAbsoluteTimeGetCurrent();
    /* display current device profiles for each device */
	err = CMIterateDeviceProfiles(&IterateDeviceProfile, &seed, &count, cmIterateCurrentDeviceProfiles, 0);
	after = CFAbsoluteTimeGetCurrent();
	printf("CMIterateDeviceProfiles  err=%ld  count=%ld  seed=%ld  time=%f\n", err, count, seed, after-before);
	
	printf("\n--- CMIterateDeviceProfiles cmIterateCustomDeviceProfiles ---\n");
	seed = count = 0;
	before = CFAbsoluteTimeGetCurrent();
    /* display custom device profiles */
	err = CMIterateDeviceProfiles(&IterateDeviceProfile, &seed, &count, cmIterateCustomDeviceProfiles, 0);
	after = CFAbsoluteTimeGetCurrent();
	printf("CMIterateDeviceProfiles  err=%ld  count=%ld  seed=%ld  time=%f\n", err, count, seed, after-before);
	
	printf("\n--- CMIterateDeviceProfiles cmIterateFactoryDeviceProfiles ---\n");
	seed = count = 0;
	before = CFAbsoluteTimeGetCurrent();
    /* display factory device profiles */
	err = CMIterateDeviceProfiles(&IterateDeviceProfile, &seed, &count, cmIterateFactoryDeviceProfiles, 0);
	after = CFAbsoluteTimeGetCurrent();
	printf("CMIterateDeviceProfiles  err=%ld  count=%ld  seed=%ld  time=%f\n", err, count, seed, after-before);
	
	
}
