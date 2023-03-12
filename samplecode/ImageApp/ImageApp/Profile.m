/*

File: Profile.m

Abstract: Profile.m class implementation

Version: <1.0>

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

Copyright � 2005-2008 Apple Inc. All Rights Reserved.

Change History (most recent first):
            1/08   added CFRelease() call to arrayOfAllProfilesWithSpace: 
                    method to fix leak

*/

#import "Profile.h"

// Callback routine with a description of a profile that is 
// called during an iteration through the available profiles.
//
static OSErr profileIterate (CMProfileIterateData *info, void *refCon)
{
    NSMutableArray* array = (NSMutableArray*) refCon;

    Profile* prof = [Profile profileWithIterateData:info];
    if (prof)
        [array addObject:prof];

    return noErr;
}



@implementation Profile

// return an array of all profiles for the given color space
//
+ (NSArray*) arrayOfAllProfilesWithSpace:(OSType)space
{
    CFIndex  i, count;
    NSArray* profArray = [Profile arrayOfAllProfiles];
    NSMutableArray* profs = [NSMutableArray arrayWithCapacity:0];

    count = [profArray count];
    for (i=0; i<count; i++)
    {
        Profile* prof = (Profile*)[profArray objectAtIndex:i];
        OSType  pClass = [prof classType];
        
        if ([prof spaceType]==space && [prof description] && 
            (pClass==cmDisplayClass || pClass==cmOutputClass))
            [profs addObject:prof];
    }

    [profArray release];
    return profs;
}

// return an array of all profiles
//
+ (NSArray*) arrayOfAllProfiles
{
    NSMutableArray* profs = [[NSMutableArray arrayWithCapacity:0] retain];
    
    CMProfileIterateUPP iterUPP = NewCMProfileIterateUPP(profileIterate);
    CMIterateColorSyncFolder(iterUPP, NULL, 0L, profs);
    DisposeCMProfileIterateUPP(iterUPP);

    return (NSArray*)profs;
}

// default RGB profiile
//
+ (Profile*) profileDefaultRGB
{
    NSString* path = [[[NSUserDefaultsController sharedUserDefaultsController] defaults]
                        objectForKey:@"DefaultRGBProfile"];
    return [Profile profileWithPath:path];
}

// default Gray profile
//
+ (Profile*) profileDefaultGray
{
    NSString* path = [[[NSUserDefaultsController sharedUserDefaultsController] defaults]
                        objectForKey:@"DefaultGrayProfile"];
    return [Profile profileWithPath:path];
}

// default CMYK profile
//
+ (Profile*) profileDefaultCMYK
{
    NSString* path = [[[NSUserDefaultsController sharedUserDefaultsController] defaults]
                        objectForKey:@"DefaultCMYKProfile"];
    return [Profile profileWithPath:path];
}

// build profile from Generic RGB
//
+ (Profile*) profileWithGenericRGB
{
    return [[[Profile alloc] initWithGenericRGB] autorelease];
}

// build profile from Linear RGB
//
+ (Profile*) profileWithLinearRGB
{
    return [[[Profile alloc] initWithLinearRGB] autorelease];
}

// build profile from iterate data
//
+ (Profile*) profileWithIterateData:(CMProfileIterateData*) data
{
    return [[[Profile alloc] initWithIterateData:data] autorelease];
}

// build profile from path
//
+ (Profile*) profileWithPath:(NSString*) path
{
    return [[[Profile alloc] initWithPath:path] autorelease];
}

// build profile from Generic RGB
//
- (Profile*) initWithGenericRGB
{
    mLocation.locType  = cmPathBasedProfile;
    strcpy(mLocation.u.pathLoc.path, "/System/Library/ColorSync/Profiles/Generic RGB Profile.icc");
    mClass = cmDisplayClass;
    mSpace = cmRGBData;

    if (CMOpenProfile(&mRef, &mLocation) == noErr)
    {
        return self;
    }
    else
    {
        [self autorelease];
        return nil;
    }
}


- (Profile*) initWithLinearRGB
{
    static const UInt8 data[0x220] = 
        "\x00\x00\x02\x20\x61\x70\x70\x6c\x02\x20\x00\x00\x6d\x6e\x74\x72"
        "\x52\x47\x42\x20\x58\x59\x5a\x20\x07\xd2\x00\x05\x00\x0d\x00\x0c"
        "\x00\x00\x00\x00\x61\x63\x73\x70\x41\x50\x50\x4c\x00\x00\x00\x00"
        "\x61\x70\x70\x6c\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\xf6\xd6\x00\x01\x00\x00\x00\x00\xd3\x2d"
        "\x61\x70\x70\x6c\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x0a\x72\x58\x59\x5a\x00\x00\x00\xfc\x00\x00\x00\x14"
        "\x67\x58\x59\x5a\x00\x00\x01\x10\x00\x00\x00\x14\x62\x58\x59\x5a"
        "\x00\x00\x01\x24\x00\x00\x00\x14\x77\x74\x70\x74\x00\x00\x01\x38"
        "\x00\x00\x00\x14\x63\x68\x61\x64\x00\x00\x01\x4c\x00\x00\x00\x2c"
        "\x72\x54\x52\x43\x00\x00\x01\x78\x00\x00\x00\x0e\x67\x54\x52\x43"
        "\x00\x00\x01\x78\x00\x00\x00\x0e\x62\x54\x52\x43\x00\x00\x01\x78"
        "\x00\x00\x00\x0e\x64\x65\x73\x63\x00\x00\x01\xb0\x00\x00\x00\x6d"
        "\x63\x70\x72\x74\x00\x00\x01\x88\x00\x00\x00\x26\x58\x59\x5a\x20"
        "\x00\x00\x00\x00\x00\x00\x74\x4b\x00\x00\x3e\x1d\x00\x00\x03\xcb"
        "\x58\x59\x5a\x20\x00\x00\x00\x00\x00\x00\x5a\x73\x00\x00\xac\xa6"
        "\x00\x00\x17\x26\x58\x59\x5a\x20\x00\x00\x00\x00\x00\x00\x28\x18"
        "\x00\x00\x15\x57\x00\x00\xb8\x33\x58\x59\x5a\x20\x00\x00\x00\x00"
        "\x00\x00\xf3\x52\x00\x01\x00\x00\x00\x01\x16\xcf\x73\x66\x33\x32"
        "\x00\x00\x00\x00\x00\x01\x0c\x42\x00\x00\x05\xde\xff\xff\xf3\x26"
        "\x00\x00\x07\x92\x00\x00\xfd\x91\xff\xff\xfb\xa2\xff\xff\xfd\xa3"
        "\x00\x00\x03\xdc\x00\x00\xc0\x6c\x63\x75\x72\x76\x00\x00\x00\x00"
        "\x00\x00\x00\x01\x01\x00\x00\x00\x74\x65\x78\x74\x00\x00\x00\x00"
        "\x43\x6f\x70\x79\x72\x69\x67\x68\x74\x20\x41\x70\x70\x6c\x65\x20"
        "\x43\x6f\x6d\x70\x75\x74\x65\x72\x20\x49\x6e\x63\x2e\x00\x00\x00"
        "\x64\x65\x73\x63\x00\x00\x00\x00\x00\x00\x00\x13\x4c\x69\x6e\x65"
        "\x61\x72\x20\x52\x47\x42\x20\x50\x72\x6f\x66\x69\x6c\x65\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00";

    mLocation.locType  = cmBufferBasedProfile;
    mLocation.u.bufferLoc.buffer = (void*)data;
    mLocation.u.bufferLoc.size = 0x220;
    mClass = cmDisplayClass;
    mSpace = cmRGBData;

    if (CMOpenProfile(&mRef, &mLocation) == noErr)
    {
        return self;
    }
    else
    {
        [self autorelease];
        return nil;
    }
}


- (Profile*) initWithIterateData:(CMProfileIterateData*) info
{
    const size_t kMaxProfNameLen = 36;

    mLocation  = info->location;
    mClass = info->header.profileClass;
    mSpace = info->header.dataColorSpace;

    if (info->uniCodeNameCount > 1)
    {
        CFIndex numChars = info->uniCodeNameCount - 1;
        if (numChars > kMaxProfNameLen)
            numChars = kMaxProfNameLen;
        mName = [[NSString stringWithCharacters:info->uniCodeName length:numChars] retain];
    }

    return self;
}


- (Profile*) initWithPath:(NSString*) path
{
    if (path)
    {
        mPath = [path retain];
        
        mLocation.locType = cmPathBasedProfile;
        strncpy(mLocation.u.pathLoc.path, [path fileSystemRepresentation], 255);
        
        CMAppleProfileHeader header;
        if (noErr==CMGetProfileHeader([self ref], &header))
        {
            mClass = header.cm2.profileClass;
            mSpace = header.cm2.dataColorSpace;
        }
        else
        {
            [self autorelease];
            return nil;
        }
    }

    return self;
}


- (void) dealloc
{
    CMCloseProfile(mRef);
    CGColorSpaceRelease(mColorspace);
    [mName release];
    [mPath release];
    [super dealloc];
}


- (CMProfileRef) ref
{
    if (mRef == NULL)
        (void) CMOpenProfile(&mRef, &mLocation);

    return mRef;
}


- (CMProfileLocation*) location
{
    return &mLocation;
}


- (OSType) classType
{
    return mClass;
}


- (OSType) spaceType
{
    return mSpace;
}


// profile description string
//
- (NSString*) description
{
    if (mName==nil)
        CMCopyProfileDescriptionString([self ref], (CFStringRef*) &mName);

    return mName;
}


- (NSString*) path
{
    if (mPath == nil)
    {
        if (mLocation.locType == cmFileBasedProfile)
        {
            FSRef       fsref;
            UInt8       path[1024];
            if (FSpMakeFSRef(&(mLocation.u.fileLoc.spec), &fsref) == noErr &&
                FSRefMakePath(&fsref, path, 1024) == noErr)
                mPath = [[NSString stringWithUTF8String:(const char *)path] retain];
        }
        else if (mLocation.locType == cmPathBasedProfile)
        {
            mPath = [[NSString stringWithUTF8String:mLocation.u.pathLoc.path] retain];
        }
    }

    return mPath;
}


- (BOOL) isEqual:(id)obj
{
    if ([obj isKindOfClass:[self class]])
        return [[self path] isEqualToString:[obj path]];
    return [super isEqual:obj];
}


- (CGColorSpaceRef) colorspace
{
    if (mColorspace == nil)
        mColorspace = CGColorSpaceCreateWithPlatformColorSpace([self ref]);
    return mColorspace;
}


// Build a 3D lookup texture for use with soft-proofing
// The resulting table is suitable for use in OpenGL to accelerate   
// color management in hardware.
//
- (NSData*) dataForCISoftproofTextureWithGridSize:(size_t) grid
{
    NSData*				nsdata = nil;
    NCMConcatProfileSet* set = nil;
    size_t				count = (grid*grid*grid) * 4;
    size_t              size;
    UInt8*				data8 = nil;
    CMWorldRef			cw = nil;
    CMProfileRef		displayProf = nil;
    Profile*			linRGB = nil;

    // profile for transform
    linRGB = [Profile profileWithLinearRGB];
    if (linRGB == nil)
        goto bail;

    // specify size of resulting data
    size = count * sizeof(float);
    nsdata = [NSMutableData dataWithLength:size];
    if (nsdata == nil)
        goto bail;

    // now build our color world transform
    size = offsetof(NCMConcatProfileSet, profileSpecs[3]);
    set = (NCMConcatProfileSet *) calloc(1, size);
    if (set==nil)
        goto bail;

    set->cmm = 0000;
    set->flagsMask = 0xFFFFFFFF;
    set->profileCount  = 3;
    set->flags = (cmBestMode) << 16 | cmGamutCheckingMask;

    set->profileSpecs[0].profile = [linRGB ref];
    set->profileSpecs[1].profile = [self ref];
    set->profileSpecs[2].profile = [linRGB ref];

    set->profileSpecs[0].renderingIntent = kUseProfileIntent;
    set->profileSpecs[1].renderingIntent = kUseProfileIntent;
    set->profileSpecs[2].renderingIntent = kUseProfileIntent;

    set->profileSpecs[0].transformTag = kDeviceToPCS;
    set->profileSpecs[1].transformTag = kPCSToPCS;
    set->profileSpecs[2].transformTag = kPCSToDevice;

    // Define a color world for color transformations among concatenated profiles.
    if (NCWConcatColorWorld (&cw, set, nil, nil) != noErr)
        goto bail;

    size = count * sizeof(UInt8);
    data8 = malloc(size);

    // CWFillLookupTexture fills a 3d lookup texture from a colorworld.
    // The resulting table is suitable for use in OpenGL to accelerate color management in 
    // hardware.

    //  cmTextureRGBtoRGBX8 = RGB to 8-bit RGBx texture

    if (CWFillLookupTexture (cw, grid, cmTextureRGBtoRGBX8, size, data8) != noErr)
        goto bail;

    float* dataPtr = (float*) [(NSMutableData*)nsdata mutableBytes];
    if (dataPtr == nil)
        goto bail;

    size_t i;
    for (i=0; i<count; i++)
        dataPtr[i] = (float)data8[i]/255.0;

bail:

    if (data8) free(data8);
    if (displayProf) CMCloseProfile (displayProf);
    if (cw) CWDisposeColorWorld(cw);
    if (set) free(set);
    return nsdata;
}


@end


// Assign user preferred default profiles if image is not tagged with a profile
//
CGImageRef CGImageCreateCopyWithDefaultSpace (CGImageRef image)
{
    if (image==nil)
        return nil;
    
    CGImageRef newImage = nil;

    Profile* dfltRGB  = [Profile profileDefaultRGB];
    Profile* dfltGray = [Profile profileDefaultGray];
    Profile* dfltCMYK = [Profile profileDefaultCMYK];

    CGColorSpaceRef devRGB  = CGColorSpaceCreateDeviceRGB();
    CGColorSpaceRef devGray = CGColorSpaceCreateDeviceGray();
    CGColorSpaceRef devCMYK = CGColorSpaceCreateDeviceCMYK();

    if (dfltRGB && CFEqual(devRGB,CGImageGetColorSpace(image)))
        newImage = CGImageCreateCopyWithColorSpace(image, [dfltRGB colorspace]);
    if (dfltGray && CFEqual(devGray,CGImageGetColorSpace(image)))
        newImage = CGImageCreateCopyWithColorSpace(image, [dfltGray colorspace]);
    if (dfltCMYK && CFEqual(devCMYK,CGImageGetColorSpace(image)))
        newImage = CGImageCreateCopyWithColorSpace(image, [dfltCMYK colorspace]);

    if (newImage == nil)
        newImage = CGImageRetain(image);

    CFRelease(devRGB);
    CFRelease(devGray);
    CFRelease(devCMYK);

    return newImage;
}


