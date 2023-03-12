/*
	File:		ColorSyncUtils.m
	
	Description: ColorSync utility routines for working with ColorSync profiles and
				 QuickTime Graphics Importer components

	Author:		Apple Developer Technical Support
	
	Copyright: 	© Copyright 2003 Apple Computer, Inc. All rights reserved.
	
	Disclaimer:	IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc.
				("Apple") in consideration of your agreement to the following terms, and your
				use, installation, modification or redistribution of this Apple software
				constitutes acceptance of these terms.  If you do not agree with these terms,
				please do not use, install, modify or redistribute this Apple software.

				In consideration of your agreement to abide by the following terms, and subject
				to these terms, Apple grants you a personal, non-exclusive license, under Apple’s
				copyrights in this original Apple software (the "Apple Software"), to use,
				reproduce, modify and redistribute the Apple Software, with or without
				modifications, in source and/or binary forms; provided that if you redistribute
				the Apple Software in its entirety and without modifications, you must retain
				this notice and the following text and disclaimers in all such redistributions of
				the Apple Software.  Neither the name, trademarks, service marks or logos of
				Apple Computer, Inc. may be used to endorse or promote products derived from the
				Apple Software without specific prior written permission from Apple.  Except as
				expressly stated in this notice, no other rights or licenses, express or implied,
				are granted by Apple herein, including but not limited to any patent rights that
				may be infringed by your derivative works or by other works in which the Apple
				Software may be incorporated.

				The Apple Software is provided by Apple on an "AS IS" basis.  APPLE MAKES NO
				WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED
				WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS FOR A PARTICULAR
				PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND OPERATION ALONE OR IN
				COMBINATION WITH YOUR PRODUCTS.

				IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL OR
				CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
				GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
				ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION, MODIFICATION AND/OR DISTRIBUTION
				OF THE APPLE SOFTWARE, HOWEVER CAUSED AND WHETHER UNDER THEORY OF CONTRACT, TORT
				(INCLUDING NEGLIGENCE), STRICT LIABILITY OR OTHERWISE, EVEN IF APPLE HAS BEEN
				ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
				
	Change History (most recent first):
	   <1>	 	11/2/03 	SRK		first file
*/

#import "ColorSyncUtils.h"


@implementation ColorSyncUtils


//////////
//
// openFileBasedProfile:
// open a profile given an FSSpec.
//
//////////

+(CMError)openFileBasedProfile:(FSSpec *)profileFSSpecPtr theProfileRef:(CMProfileRef *)aProfileRef
{
    CMProfileLocation	ploc;
	
	if (aProfileRef == NULL)
		return -1;
    
    *aProfileRef = NULL;
    
    ploc.locType = cmFileBasedProfile;
    memcpy(&ploc.u.fileLoc.spec,profileFSSpecPtr, sizeof(FSSpec));
    
    return (CMOpenProfile(aProfileRef, &ploc));
}

//////////
//
// openProfileFromNSStringPath:
// open a profile given a Cocoa NSString path to the file.
//
//////////

+(CMError)openProfileFromNSStringPath:(NSString *)aProfilePath theProfile:(CMProfileRef *)aProfileRef
{
    CMProfileLocation	ploc;
    CMError				cmErr;
	FSSpec				pathFSSpec;

	if (aProfileRef == NULL)
		return -1;

    *aProfileRef = NULL;

	cmErr = [Utils pathnameToFSSpec:aProfilePath fsSpec:&pathFSSpec];
	if (cmErr == noErr)
	{ 
		ploc.locType = cmFileBasedProfile;
		memcpy(&ploc.u.fileLoc.spec,&pathFSSpec, sizeof(FSSpec));
		
		cmErr = CMOpenProfile(aProfileRef, &ploc);
	}
    
    return cmErr;
}

//////////
//
// openHandleBasedProfile:
// open a profile given a handle.
//
//////////

+(CMError)openHandleBasedProfile:(Handle)profileHandle theProfile:(CMProfileRef *)aProfileRef
{
    CMError         err = -1;
    
	if (aProfileRef == NULL)
		return -1;

    *aProfileRef = NULL;
    
    if (profileHandle)
    {
        CMProfileLocation	ploc;

        ploc.locType = cmHandleBasedProfile;
        ploc.u.handleLoc.h = profileHandle;
        
        err = CMOpenProfile(aProfileRef, &ploc);
    }
    
    return err;
}

//////////
//
// userGetNewProfile:
// prompt the user for a profile file, and open that profile file.
//
//////////

+(CMError)userGetNewProfile:(CMProfileRef *)aProfileRef
{
    NSString *newProfilePath = nil;

	if (aProfileRef == NULL)
		return -1;

    *aProfileRef = 0;
	CMError cmErr = noErr;
	
    newProfilePath = [Utils promptForProfileFile];
    if (newProfilePath)
    {		
		cmErr = [self openProfileFromNSStringPath:newProfilePath theProfile:aProfileRef];
	}
	
	return cmErr;
}


//////////
//
// closeProfileRef:
// close the specified profile reference.
//
//////////

+(CMError)closeProfileRef:(CMProfileRef)ref
{
	return(CMCloseProfile(ref));
}

//////////
//
// profilePathFromProfileRef:
// given a profile reference, this function returns a NSString representation 
// of the profile file path.
//
//////////

+(NSString *)profilePathFromProfileRef:(CMProfileRef) profileRef
{
    CFStringRef profilePathNameCFStr = nil;
    NSString *profilePathNameNSString = nil;

	if (profileRef == NULL)
		return nil;

    profilePathNameCFStr = [ColorSyncUtils copyProfileDescriptionCFString:profileRef];
    if (profilePathNameCFStr)
    {
        profilePathNameNSString = [Utils cocoaStringFromCFStringRef:(CFStringRef)profilePathNameCFStr];
        CFRelease(profilePathNameCFStr);
    }
    
    return profilePathNameNSString;
}

//////////
//
// profileNameFromProfileRef:
// returns the profile name for the given profile reference.
//
//////////

+(NSString *)profileNameFromProfileRef:(CMProfileRef) profileRef
{
    CFStringRef profilePathNameCFStr = nil;
    NSString *theProfileName = nil;

	if (profileRef == NULL)
		return nil;

    profilePathNameCFStr = [ColorSyncUtils copyProfileDescriptionCFString:profileRef];
    if (profilePathNameCFStr)
    {
        NSString *profilePathNameNSString = nil;

        profilePathNameNSString = [Utils cocoaStringFromCFStringRef:(CFStringRef)profilePathNameCFStr];
        if (profilePathNameNSString)
        {
            theProfileName = [Utils filenameFromFullPath:profilePathNameNSString];
        }
        
        CFRelease(profilePathNameCFStr);
    }
    
    return theProfileName;
}

//////////
//
// profileNameString:
// returns the profile name from the specified profile handle.
//
//////////

+(NSString *)profileNameString:(Handle)profileHandle
{
    NSString *profileNameNSString = nil;
    CFStringRef profileNameCFStr = nil;
    CMProfileRef profileRef = NULL;
    CMError err;

	if (profileHandle == NULL)
		return nil;

    err = [ColorSyncUtils openHandleBasedProfile:profileHandle theProfile:&profileRef];
    if (profileRef)
    {
        profileNameCFStr = [ColorSyncUtils copyProfileDescriptionCFString:profileRef];
        [ColorSyncUtils closeProfileRef:profileRef];
        if (profileNameCFStr)
        {
            profileNameNSString = [Utils cocoaStringFromCFStringRef:(CFStringRef)profileNameCFStr];
            CFRelease(profileNameCFStr);
        }
    }
    
    return profileNameNSString;
}


//////////
//
// copyProfileDescriptionCFString:
// get the profile name as a CFString.
//
//////////

+(CFStringRef) copyProfileDescriptionCFString:(CMProfileRef) prof
{
    Str255         pName;
    ScriptCode     code;
    CFStringRef    str = nil;
    CMError        err;

	if (prof == NULL)
		return nil;

    // for v4 profiles, try to get the best localized name from the 'desc'/'mluc' tag
    err = CMCopyProfileLocalizedString(prof, cmProfileDescriptionTag, 0,0, &str);
    // if that didn't work...
    if (err != noErr)
    {
        // for Apple's localized v2 profiles, try to get the best localized name from the 'dscm'/'mluc' tag
        err = CMCopyProfileLocalizedString(prof, cmProfileDescriptionMLTag, 0,0, &str);
        // if that didn't work...
        if (err != noErr)
        {
            // for normal v2 profiles, get the name from the 'desc'/'desc' tag
            err = CMGetScriptProfileDescription( prof, pName, &code);
            // convert it to a CFString
            if (err == noErr)
            {
                str = CFStringCreateWithPascalString(0, pName, code);
            }
        }
    }
    
    return str;
}

//////////
//
// graphicsImporterForImageFileFromPath:
// returns the QuickTime Graphics Importer for the specified image file.
//
//////////

+(OSErr)graphicsImporterForImageFileFromPath:(NSString *)imageFilePath graphicsImporter:(ComponentInstance *)anImporterRef
{
    OSErr err = noErr;
	FSSpec fileFSSpec;

	if (imageFilePath == NULL)
		return -1;

    err = [Utils pathnameToFSSpec:imageFilePath fsSpec:&fileFSSpec];
    if (err == noErr)
    {
        err = GetGraphicsImporterForFile(&fileFSSpec, anImporterRef);
    }
	
	return err;
}

//////////
//
// embeddedProfileForImageFileFromPath:
// embeds a profile into the image file.
//
//////////

+(CMError)embeddedProfileForImageFileFromPath:(NSString *)imageFilePath profileReference:(CMProfileRef *)aProfileRef
{
	OSErr err;
	FSSpec theFSSpec;

	if (imageFilePath == NULL)
		return nil;

	err = [Utils pathnameToFSSpec:imageFilePath fsSpec:&theFSSpec ];
	if (err == noErr)
	{
		return(CMGetIndImageProfile(&theFSSpec, 1, aProfileRef));
	}

	return err;
}

//////////
//
// defaultRGBProfile:
// returns the default ColorSync RGB profile.
//
//////////

+(CMError)defaultRGBProfile:(CMProfileRef *)aProfileRef
{
    return(CMGetDefaultProfileBySpace(cmRGBData, aProfileRef));
}

//////////
//
// embedProfileInImageFile:
// embeds a profile into the image file.
//
//////////

+(CMError)embedProfileInImageFile:(FSSpec *)imageFile profileFile:(FSSpec *)profileFile
{
    CMProfileRef profRef = NULL;
    CMError cmErr = noErr;

	if (profileFile == NULL)
		return -1;

	if (imageFile == NULL)
		return -1;

    cmErr = [ColorSyncUtils openFileBasedProfile:profileFile theProfileRef:&profRef];
    if (profRef)
    {
        cmErr = CMEmbedImage(imageFile, nil, true /* replace original */, profRef);
        CMCloseProfile(profRef);
    }
    
    return (cmErr);
}

//////////
//
// embedDefaultRGBProfileIntoImageFile:
// embeds the default ColorSync RGB profile into the image file.
//
//////////

+(CMError)embedDefaultRGBProfileIntoImageFile:(NSString *)imageFilePath
{
	CMProfileRef rgbProfileRef = NULL;
	
	if (imageFilePath == NULL)
		return -1;

	CMError cmErr = [ColorSyncUtils defaultRGBProfile:&rgbProfileRef];
    if (rgbProfileRef)
    {
		FSSpec fileFSSpec;
		OSErr err = noErr;

		err = [Utils pathnameToFSSpec:imageFilePath fsSpec:&fileFSSpec];
		if (err == noErr)
		{
			cmErr = CMEmbedImage(&fileFSSpec, NULL, true /* replace original */, rgbProfileRef);
            CMCloseProfile(rgbProfileRef);
		}
		else
		{
			cmErr = err;
		}
    }
	
	return cmErr;
}


//////////
//
// colorMatchSrcOffscreenToDestOffscreen:
// color match the contents of the src offscreen using the specified color 
// world reference, and store the results into the dest offscreen.
//
//////////

+(CMError)colorMatchSrcOffscreenToDestOffscreen:(GWorldPtr)srcGWorld destGWorld:(GWorldPtr)aGWorld colorWorld:(CMWorldRef)aColorWorldRef
{
    CMBitmap		srcColorBitmap;
    CMBitmap		destColorBitmap;
	CMError			cmErr = -1;
    
    if (srcGWorld && aGWorld)
    {
		PixMapHandle		srcPixMapHndl;
		PixMapHandle		destPixMapHndl;

        srcPixMapHndl = GetGWorldPixMap(srcGWorld);
        destPixMapHndl = GetGWorldPixMap(aGWorld);
        
        if (LockPixels(srcPixMapHndl))
        {
            if (LockPixels(destPixMapHndl))
            {
                if (aColorWorldRef)
                {                    
                    [self PixMapToCMBitMap:srcPixMapHndl bm:&srcColorBitmap];
                    [self PixMapToCMBitMap:destPixMapHndl bm:&destColorBitmap];
                    
                    cmErr = CWMatchBitmap(aColorWorldRef, &srcColorBitmap, 0L,0L, &destColorBitmap);    
                }
                
                UnlockPixels (destPixMapHndl);
            }
            
            UnlockPixels (srcPixMapHndl);
        }
    }

	return cmErr;
}

//////////
//
// PixMapToCMBitMap:
// create a CMBitmap structure from a PixMap.
//
//////////

+(CMError)PixMapToCMBitMap:(PixMapHandle)pm bm:(CMBitmap*)bm
{
	Rect pixBounds;
	CMError theErr = noErr;
		
	bm->image		= GetPixBaseAddr(pm);
	GetPixBounds (pm, &pixBounds);
	bm->width		= pixBounds.right  - pixBounds.left;
	bm->height		= pixBounds.bottom - pixBounds.top;
	bm->rowBytes	= GetPixRowBytes(pm);
	bm->pixelSize	= GetPixDepth(pm);
	
	switch (bm->pixelSize)
	{
		case 16:
		bm->space = cmRGB16Space;
		break;
			
		case 32:
		bm->space = cmARGB32Space;
		break;
		
		default:
		theErr = paramErr;
		break;
	
	}
	
	return theErr;
}

//////////
//
// profileByUse:
// get the default ColorSync profile by use.
//
//////////

+(CMError)profileByUse:(OSType)use profileRefPtr:(CMProfileRef *)aProfileRef
{
    return (CMGetDefaultProfileByUse(use, aProfileRef));
}

//////////
//
// profileBySpace:
// get the default ColorSync profile by space.
//
//////////

+(CMError)profileBySpace:(OSType)use profileRefPtr:(CMProfileRef *)aProfileRef
{
    return (CMGetDefaultProfileBySpace(use, aProfileRef));
}

//////////
//
// setProfileByUseFromPath:
// set the default ColorSync profile by use from the specified profile file path.
//
//////////

+(CMError)setProfileByUseFromPath:(OSType)use theProfile:(NSString *)aProfilePath profileRef:(CMProfileRef *)aProfileRef
{
	CMError			cmErr;

	if (aProfilePath == NULL)
		return -1;

	if (aProfileRef == NULL)
		return -1;

	*aProfileRef = 0;
	cmErr = [ColorSyncUtils openProfileFromNSStringPath:aProfilePath theProfile:aProfileRef];
	if (cmErr == noErr)
	{
		cmErr = CMSetDefaultProfileByUse(use, *aProfileRef);
	}
	
	return cmErr;

}

//////////
//
// setProfileBySpaceFromPath:
// set the default ColorSync profile by space from the specified profile file path.
//
//////////

+(CMError)setProfileBySpaceFromPath:(OSType)use theProfile:(NSString *)aProfilePath profileRef:(CMProfileRef *)aProfileRef
{
	CMError			cmErr;

	if (aProfileRef == NULL)
		return -1;

	*aProfileRef = 0;
	cmErr = [ColorSyncUtils openProfileFromNSStringPath:aProfilePath theProfile:aProfileRef];
	if (cmErr == noErr)
	{
		cmErr = CMSetDefaultProfileBySpace(use, *aProfileRef);
	}
	
	return cmErr;

}

//////////
//
// setPantherQTColorMatchingForComponent:
// turn on QuickTime Graphics Importer automatic ColorSync matching (Panther only).
//
//////////

+(OSErr)setPantherQTColorMatchingForComponent:(GraphicsImportComponent)aComponent
{

	if (aComponent == NULL)
		return -1;

	if ([Utils MacOSPantherOrBetter])
	{
		long curFlags = 0;

		return(GraphicsImportSetFlags( aComponent, curFlags));
	}

	return -1;
}

//////////
//
// setPantherQTColorMatchingOffForComponent:
// turn off QuickTime Graphics Importer automatic ColorSync matching (Panther only).
//
//////////

+(OSErr)setPantherQTColorMatchingOffForComponent:(GraphicsImportComponent)aComponent
{
	if (aComponent == NULL)
		return -1;

	if ([Utils MacOSPantherOrBetter])
	{
        if ([self isPantherQTColorMatchingTurnedOnForComponent:aComponent])
        {
            long newFlag = kGraphicsImporterDontUseColorMatching;

            return(GraphicsImportSetFlags( aComponent, newFlag));
        }
	}

	return -1;
}

//////////
//
// isPantherQTColorMatchingTurnedOnForComponent:
// check to see if QuickTime Graphics Importer automatic ColorSync matching 
// (Panther only) is turned on.
//
//////////

+(Boolean)isPantherQTColorMatchingTurnedOnForComponent:(GraphicsImportComponent)aComponent
{
	if (aComponent == NULL)
		return false;

	if ([Utils MacOSPantherOrBetter])
	{
		long flags;
		OSErr err = GraphicsImportGetFlags( aComponent, &flags);
		if (err == noErr)
		{
			if (flags & kGraphicsImporterDontUseColorMatching)
			{
				return false;
			}
			else
			{
				return true;
			}
		}
	}

	return false;
}


@end
