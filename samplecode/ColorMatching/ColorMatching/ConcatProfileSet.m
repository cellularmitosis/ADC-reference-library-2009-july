/*
	File:		ConcatProfileSet.m
	
	Description: Code to keep track of an image's profile & mode for each of the source, 
				 destination, abstract and proofer profile selections

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

#import "ConcatProfileSet.h"
#import "ColorSyncUtils.h"


@implementation ConcatProfileSet


//////////
//
// init:
// initialize our profiles and modes.
//
//////////

- (id)init
{
    CMError cmErr;

	srcProf = nil;
	absProf = nil;
	prfProf = nil;
	dstProf = nil;
    
    // assign default profiles and modes
    cmErr = CMGetDefaultProfileBySpace(cmRGBData, &srcProf);
	sourceMode = kRGB; 

	effectsMode	= kNone;
	proofMode = kNone;

    cmErr = CMGetDefaultProfileByUse(cmDisplayUse, &dstProf);
	destinationMode = kDisplay;

	return (self);
}

#pragma mark -

//////////
//
// buildColorWorld:
// build a ColorSync color world from a ConcatProfileSet object.
//
//////////

-(CMError)buildColorWorld:(CMWorldRef *)aColorWorldRefPtr
{
    CMProfileRef	profileRef = 0;
	UInt32			profCount = 0;
	struct {
		OSType cmm;
		UInt32 flags;
		UInt32 flagsMask;
		UInt32 profileCount;																														
		NCMConcatProfileSpec profileSpecs[4];
	}	set = {0, cmGamutCheckingMask, cmGamutCheckingMask, 0,
					{	{0, kDeviceToPCS, nil}, 
						{0, kPCSToPCS, nil}, 
						{0, kPCSToPCS, nil},
						{0, kPCSToPCS, nil} } };
	
	if (aColorWorldRefPtr == NULL)
		return -1;

	*aColorWorldRefPtr = nil;    
	
	// Source
    profileRef = [self srcModeProfile];
    if (profileRef)
    {
        set.profileSpecs[profCount++].profile = profileRef;
    }
    
	// Effects (F/X)
    profileRef = [self fxModeProfile];
    if (profileRef)
    {
        set.profileSpecs[profCount++].profile = profileRef;
    }

    // Proofer
    profileRef = [self prfModeProfile];
    if (profileRef)
    {
        set.profileSpecs[profCount++].profile = profileRef;
    }
	
    // Destination
    profileRef = [self destModeProfile];
    if (profileRef)
    {
        set.profileSpecs[profCount++].profile = profileRef;
	}
    
	set.profileCount = profCount;
	set.profileSpecs[profCount-1].transformTag = kPCSToDevice;
	
	return(NCWConcatColorWorld(aColorWorldRefPtr, (NCMConcatProfileSet *)&set, nil, 0));
}

#pragma mark -

//////////
//
// replaceSavedProfile
// replace a saved profile ref with another.
//
//////////

-(CMError)replaceSavedProfile:(CMProfileRef)newProfileRef currentSavedProfile:(CMProfileRef *)oldProfile
{
	CMError cmErr = noErr;
	
	if (*oldProfile != newProfileRef)
	{
		cmErr = CMCloseProfile(*oldProfile);
	}
	
	*oldProfile = newProfileRef;
	if (newProfileRef)
	{
		cmErr = CMCloneProfileRef(newProfileRef);
	}

	return cmErr;
}


//////////
//
// setSrcProfile:
// Set the saved source profile ref.
//
//////////

- (CMError)setSrcProfile:(CMProfileRef)aProfileRef
{
	return ([self replaceSavedProfile:aProfileRef currentSavedProfile:&srcProf]);
}

//////////
//
// setProoferProfile:
// Set the saved proofer profile ref.
//
//////////

- (CMError)setProoferProfile:(CMProfileRef)aProfileRef
{
	return ([self replaceSavedProfile:aProfileRef currentSavedProfile:&prfProf]);
}

//////////
//
// setDestProfile:
// Set the saved destination profile ref.
//
//////////

- (CMError)setDestProfile:(CMProfileRef)aProfileRef
{
	return ([self replaceSavedProfile:aProfileRef currentSavedProfile:&dstProf]);
}

//////////
//
// setAbstractProfile:
// Set the saved abstract profile ref.
//
//////////

- (CMError)setAbstractProfile:(CMProfileRef)aProfileRef
{
    return ([self replaceSavedProfile:aProfileRef currentSavedProfile:&absProf]);
}

#pragma mark -

//////////
//
// getProfileByUseOrSpace:
// return the default ColorSync profile by space or use.
//
//////////

-(CMError)getProfileByUseOrSpace:(OSType)colorSpaceMode profile:(CMProfileRef *)aProfileRefPtr
{
	switch(colorSpaceMode)
	{
		case kRGB:
		case kSRGB:
		{
			return ([ColorSyncUtils profileBySpace:colorSpaceMode profileRefPtr:aProfileRefPtr]);
		}
		break;

		case kProofer:
		case kPrinter:
		case kDisplay:
		{
			return ([ColorSyncUtils profileByUse:colorSpaceMode profileRefPtr:aProfileRefPtr]);
		}
		break;
	}
	
	return noErr;
}

//////////
//
// srcModeProfile:
// return the source profile for the current mode setting.
//
//////////

- (CMProfileRef)srcModeProfile
{
	[self getProfileByUseOrSpace:sourceMode profile:&srcProf];
	
	return srcProf;
}

//////////
//
// fxModeProfile:
// return the abstract profile for the current mode setting.
//
//////////

- (CMProfileRef)fxModeProfile
{
	return absProf;
}

//////////
//
// prfModeProfile:
// return the proofer profile for the current mode setting.
//
//////////

- (CMProfileRef)prfModeProfile
{
	[self getProfileByUseOrSpace:proofMode profile:&prfProf];

	return prfProf;
}

//////////
//
// destModeProfile:
// return the dest profile for the current mode setting.
//
//////////

- (CMProfileRef)destModeProfile
{
	[self getProfileByUseOrSpace:destinationMode profile:&dstProf];

	return dstProf;
}


#pragma mark -


//////////
//
// setSrcModeAndProfile:
// set the source mode setting and profile.
//
//////////

- (CMError)setSrcModeAndProfile:(ConcatProfileMode)aProfileMode aProfile:(CMProfileRef)aProfileRef
{
	CMError cmErr = noErr;
	
	if (aProfileMode == kEmbedded || aProfileMode == kOther)
	{
		cmErr = [self setSrcProfile:aProfileRef];
	}
	else
	{
		cmErr = [self getProfileByUseOrSpace:aProfileMode profile:&srcProf];
	}

	sourceMode = aProfileMode;
	
	return cmErr;
}

//////////
//
// setAbsModeAndProfile:
// set the abstract mode setting and profile.
//
//////////

- (CMError)setAbsModeAndProfile:(ConcatProfileMode)aProfileMode aProfile:(CMProfileRef)aProfileRef
{
	CMError cmErr = noErr;

	if (aProfileMode == kOther || aProfileMode == kNone)
	{
		cmErr = [self setAbstractProfile:aProfileRef];
	}
	else
	{
		cmErr = [self getProfileByUseOrSpace:aProfileMode profile:&absProf];
	}
	
	effectsMode = aProfileMode;
	
	return cmErr;
}

//////////
//
// setProofModeAndProfile:
// set the proofer mode setting and profile.
//
//////////

- (CMError)setProofModeAndProfile:(ConcatProfileMode)aProfileMode aProfile:(CMProfileRef)aProfileRef
{
	CMError cmErr = noErr;

	if (aProfileMode == kOther || aProfileMode == kNone)
	{
		cmErr = [self setProoferProfile:aProfileRef];
	}
	else
	{
		cmErr = [self getProfileByUseOrSpace:aProfileMode profile:&prfProf];
	}
	
	proofMode = aProfileMode;
	
	return cmErr;
}

//////////
//
// setDestModeAndProfile:
// set the destination mode setting and profile.
//
//////////

- (CMError)setDestModeAndProfile:(ConcatProfileMode)aProfileMode aProfile:(CMProfileRef)aProfileRef
{
	CMError cmErr = noErr;

	if (aProfileMode == kOther)
	{
		cmErr = [self setDestProfile:aProfileRef];
	}
	else
	{
		cmErr = [self getProfileByUseOrSpace:aProfileMode profile:&dstProf];
	}
	
	destinationMode = aProfileMode;
	
	return cmErr;
}

#pragma mark -

//////////
//
// srcMode:
// return the mode setting for the source profile.
//
//////////

- (ConcatProfileMode)srcMode
{
	return sourceMode; 
}

//////////
//
// fxMode:
// return the mode setting for the abstract profile.
//
//////////

- (ConcatProfileMode)fxMode
{
	return effectsMode;
}

//////////
//
// prfMode:
// return the mode setting for the proofer profile.
//
//////////

- (ConcatProfileMode)prfMode
{
	return proofMode;
}

//////////
//
// dstMode:
// return the mode setting for the dest profile.
//
//////////

- (ConcatProfileMode)dstMode
{
	return destinationMode;
}

#pragma mark -

//////////
//
// dealloc:
// close all our open profile refs.
//
//////////

-(void)dealloc
{
    if (srcProf)
    {
        CMCloseProfile(srcProf);
    }
    if (prfProf)
    {
        CMCloseProfile(prfProf);
    }
    if (absProf)
    {
        CMCloseProfile(absProf);
    }
    if (dstProf)
    {
        CMCloseProfile(dstProf);
    }
}

@end
