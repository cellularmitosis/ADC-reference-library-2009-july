/*
	File:		ColorSyncProfiles.m
	
	Description: Implementation file for ColorSyncProfile.m class. Contains code to manage
				 popups which allow the user to set the standard ColorSync display, abstract, 
				 proofer and rgb profiles.

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

#import "ColorSyncProfiles.h"


@implementation ColorSyncProfiles


//////////
//
// awakeFromNib:
// initialize all our ColorSync default profiles.
//
//////////

-(void)awakeFromNib
{
	CMError cmErr;

	mDisplayRef = NULL;
	mPrinterRef = NULL;
	mProoferRef = NULL;
	mRGBRef = NULL;

	cmErr = [ColorSyncUtils profileByUse:cmDisplayUse profileRefPtr:&mDisplayRef];
	cmErr = [ColorSyncUtils profileByUse:cmOutputUse profileRefPtr:&mPrinterRef];
	cmErr = [ColorSyncUtils profileByUse:cmProofUse profileRefPtr:&mProoferRef];
	cmErr = [ColorSyncUtils profileBySpace:cmRGBData profileRefPtr:&mRGBRef];

	if (mDisplayRef)
	{
		[mDisplayProfile setTitle:[ColorSyncUtils profileNameFromProfileRef:mDisplayRef]];
	}

	if (mPrinterRef)
	{
		[mPrinterProfile setTitle:[ColorSyncUtils profileNameFromProfileRef:mPrinterRef]];
	}

	if (mProoferRef)
	{
		[mProoferProfile setTitle:[ColorSyncUtils profileNameFromProfileRef:mProoferRef]];
	}

	if (mRGBRef)
	{
		[mRGBProfile setTitle:[ColorSyncUtils profileNameFromProfileRef:mRGBRef]];
	}
}

//////////
//
// userSetDisplayProfile:
// set a new ColorSync default display profile.
//
//////////

- (IBAction)userSetDisplayProfile:(id)sender
{
    NSString *newProfilePath = nil;
    
    newProfilePath = [Utils promptForProfileFile];
    if (newProfilePath)
    {
		CMError cmErr;
		cmErr = [ColorSyncUtils closeProfileRef:mDisplayRef];
        
		cmErr = [ColorSyncUtils setProfileByUseFromPath:cmDisplayUse theProfile:newProfilePath profileRef:&mDisplayRef];
		if (cmErr) NSLog(@"CMSetDefaultProfileByUse failed (%d)\n", cmErr);
        if (cmErr == noErr)
        {
            [mDisplayProfile setTitle:[Utils filenameFromFullPath:newProfilePath]];
        }
    }
	
	[mDisplayProfilePopup selectItemAtIndex:0];
}

//////////
//
// userSetPrinterProfile:
// set a new ColorSync default printer profile.
//
//////////

- (IBAction)userSetPrinterProfile:(id)sender
{
    NSString *newProfilePath = nil;
    
    newProfilePath = [Utils promptForProfileFile];
    if (newProfilePath)
    {
		CMError cmErr;
		cmErr = [ColorSyncUtils closeProfileRef:mPrinterRef];
        
		cmErr = [ColorSyncUtils setProfileByUseFromPath:cmOutputUse theProfile:newProfilePath profileRef:&mPrinterRef];
		if (cmErr) NSLog(@"CMSetDefaultProfileByUse failed (%d)\n", cmErr);
        if (cmErr == noErr)
        {
            [mPrinterProfile setTitle:[Utils filenameFromFullPath:newProfilePath]];
        }
    }
	
	[mPrinterProfilePopup selectItemAtIndex:0];
}

//////////
//
// userSetProoferProfile:
// set a new ColorSync default proofer profile.
//
//////////

- (IBAction)userSetProoferProfile:(id)sender
{
    NSString *newProfilePath = nil;
    
    newProfilePath = [Utils promptForProfileFile];
    if (newProfilePath)
    {
		CMError cmErr;
		cmErr = [ColorSyncUtils closeProfileRef:mProoferRef];
        
		cmErr = [ColorSyncUtils setProfileByUseFromPath:cmProofUse theProfile:newProfilePath profileRef:&mProoferRef];
		if (cmErr) NSLog(@"CMSetDefaultProfileByUse failed (%d)\n", cmErr);
        if (cmErr == noErr)
        {
            [mProoferProfile setTitle:[Utils filenameFromFullPath:newProfilePath]];
        }
    }
	
	[mProofProfilePopup selectItemAtIndex:0];
}

//////////
//
// userSetRGBProfile:
// set a new ColorSync default RGB profile.
//
//////////

- (IBAction)userSetRGBProfile:(id)sender
{
    NSString *newProfilePath = nil;
    
    newProfilePath = [Utils promptForProfileFile];
    if (newProfilePath)
    {
		CMError cmErr;
		cmErr = [ColorSyncUtils closeProfileRef:mRGBRef];
        
		cmErr = [ColorSyncUtils setProfileBySpaceFromPath:cmRGBData theProfile:newProfilePath profileRef:&mRGBRef];
		if (cmErr) NSLog(@"CMSetDefaultProfileByUse failed (%d)\n", cmErr);
        if (cmErr == noErr)
        {
            [mRGBProfile setTitle:[Utils filenameFromFullPath:newProfilePath]];
        }
    }
	
	[mRGBProfilePopup selectItemAtIndex:0];

}

//////////
//
// dealloc:
// close all our profile references.
//
//////////

-(void)dealloc
{
	if (mDisplayRef)
	{
		CMCloseProfile(mDisplayRef);
	}

	if (mPrinterRef)
	{
		CMCloseProfile(mPrinterRef);
	}

	if (mProoferRef)
	{
		CMCloseProfile(mProoferRef);
	}

	if (mRGBRef)
	{
		CMCloseProfile(mRGBRef);
	}
}

@end
