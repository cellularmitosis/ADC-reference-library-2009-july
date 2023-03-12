/*
	File:		ImageProfiles.m
	
	Description: Implementation file for ImageProfiles.m class. Contains code to manage the
				 popups which allow the user to set the image's embedded, proofer, abstract
				 and display profile and turn on/off QuickTime Graphics Importer automatic
				 ColorMatching feature on Panther.

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

#import "ImageProfiles.h"
#import "ColorMatchProfileModes.h"

ImageProfiles *gImageProfilesInstance = nil;

@implementation ImageProfiles

//////////
//
// sharedImageProfiles:
// returns the object instance.
//
//////////

+ (ImageProfiles *) sharedImageProfiles
{
    if (gImageProfilesInstance == nil)
	{
	    gImageProfilesInstance = [[self alloc] init];
	}
	
    return gImageProfilesInstance;
}

//////////
//
// awakeFromNib:
// initialize the popups.
//
//////////

-(void)awakeFromNib
{
	gImageProfilesInstance = self;

	[self disableColorSyncProfilePopups];
	[self disableQTColorSyncPopups];

	[mPantherQTColorSyncMatching setEnabled:NO];
}


//////////
//
// setMenuItemTitleForItemWithTag:
// set the menu item title for the menu item with the specified tag.
//
//////////

-(int)setMenuItemTitleForItemWithTag:(id)sender tag:(OSType)aTag title:(NSString *)aTitle
{
	int itemIndex = [sender indexOfItemWithTag:aTag];	
	[[sender itemAtIndex:itemIndex] setTitle:aTitle];
    
    return itemIndex;
}

//////////
//
// selectMenuItemForMode:
// select the menu item with the specified tag.
//
//////////

-(int)selectMenuItemForMode:(id)sender mode:(OSType)aMode
{
	int itemIndex = [sender indexOfItemWithTag:aMode];
	[sender selectItemAtIndex: itemIndex];
    
    return itemIndex;
}

//////////
//
// appendProfileNameToMenuItemTitleForMode:
// append a profile name to the menu item with the specified tag.
//
//////////

-(void)appendProfileNameToMenuItemTitleForMode:(id)sender mode:(OSType)aMode profName:(NSString *)aProfName
{
	if (aMode == kOther)
	{
    	NSMutableString *otherString = [NSMutableString stringWithCString:"Other:"];
        if (aProfName)
        {
            [otherString appendString:aProfName];
        }
        [self setMenuItemTitleForItemWithTag:sender tag:aMode title:otherString];
	}
	else if (aMode == kEmbedded)
	{
		NSMutableString *embeddedString = [NSMutableString stringWithCString: "Embedded:"];
        if (aProfName)
        {
            [embeddedString appendString:aProfName];
        }
        [self setMenuItemTitleForItemWithTag:sender tag:aMode title:embeddedString];
	}

}


//////////
//
// handlePopupSelection:
// handle selections for the image profiles - this involves opening
// the selected profile, setting the popup menu item name to the profile
// name and assigning the selected profile to the image.
//
//////////

- (IBAction)handlePopupSelection:(id)sender
{
	OSType	tag = [[sender selectedItem] tag];
	CMProfileRef aProfileRef = nil;
	MyDocument *currentDoc;
	ConcatProfileSet *docProfileSet;
	CMError cmErr;

    currentDoc = (MyDocument *)[Utils getCurrentDocument];
    if (currentDoc == NULL) goto bail;
    
    docProfileSet = [currentDoc concatProfileSetObj];
    if (docProfileSet == NULL) goto bail;
        
	if (tag == kOther)
	{
		cmErr = [ColorSyncUtils userGetNewProfile:&aProfileRef];		

        // did user press cancel in the get dialog?
		if (aProfileRef == NULL) goto bail;
 	}
	
	if (sender == mSourcePopup)
	{
		if (tag == kEmbedded)
		{
			cmErr = [[currentDoc graphicsImporterObj] imageEmbeddedProfile:&aProfileRef];
		}
		cmErr = [docProfileSet setSrcModeAndProfile:tag aProfile:aProfileRef];
	}
	else if (sender == mProofPopup)
	{
		cmErr = [docProfileSet setProofModeAndProfile:tag aProfile:aProfileRef];
	}
	else if (sender == mDestPopup)
	{
		cmErr = [docProfileSet setDestModeAndProfile:tag aProfile:aProfileRef];
	}
	else if (sender == mAbstractPopup)
	{
		cmErr = [docProfileSet setAbsModeAndProfile:tag aProfile:aProfileRef];
	}

	if (aProfileRef)
	{
		if (tag == kOther)
		{
			NSMutableString *other = [NSMutableString stringWithCString:"Other:"];
			[other appendString: [Utils filenameFromFullPath:[ColorSyncUtils profileNameFromProfileRef:aProfileRef]]];

            [self setMenuItemTitleForItemWithTag:sender tag:kOther title:other];
		}
		else if (tag == kEmbedded)
		{
			NSMutableString *embeddedProfileName = [NSMutableString stringWithCString:"Embedded:"];
			[embeddedProfileName appendString: [Utils filenameFromFullPath:[ColorSyncUtils profileNameFromProfileRef:aProfileRef]]];

            [self setMenuItemTitleForItemWithTag:sender tag:kEmbedded title:embeddedProfileName];		
		}
		else
		{
            [self setMenuItemTitleForItemWithTag:sender tag:kOther title:@"Other:"];
		}

		cmErr = [ColorSyncUtils closeProfileRef:aProfileRef];
	}		
	else 
	{
        [self setMenuItemTitleForItemWithTag:sender tag:kOther title:@"Other:"];
	}

	[currentDoc colorMatchImage];
	[currentDoc drawDocumentImage];

bail:
	return;
}

//////////
//
// updatePopupProfilesForDocument:
// update popup menu item titles to reflect current profile selections
// for the image.
//
//////////

-(void)updatePopupProfilesForDocument:(MyDocument *)document
{
	ConcatProfileSet *docProfileSet = [document concatProfileSetObj];
    GraphicsImporterImage *qtGraphicsImporterObj = [document graphicsImporterObj];

    if (docProfileSet == nil) goto bail;
    if (qtGraphicsImporterObj == nil) goto bail;
    
	// src
    
    [self selectMenuItemForMode:mSourcePopup mode:[docProfileSet srcMode]];
    // clear any previous profile name first
    [self appendProfileNameToMenuItemTitleForMode:mSourcePopup mode:kOther profName:NULL];
	[self appendProfileNameToMenuItemTitleForMode:mSourcePopup mode:kEmbedded profName:NULL];
    // now update for current mode
    [self appendProfileNameToMenuItemTitleForMode:mSourcePopup 
        mode:[docProfileSet srcMode] 
        profName:[ColorSyncUtils profileNameFromProfileRef:[docProfileSet srcModeProfile]]];

	// abstract
    
    [self selectMenuItemForMode:mAbstractPopup mode:[docProfileSet fxMode]];
    // clear any previous profile name first
    [self appendProfileNameToMenuItemTitleForMode:mAbstractPopup mode:kOther profName:NULL];
    // now update for current mode
    [self appendProfileNameToMenuItemTitleForMode:mAbstractPopup 
        mode:[docProfileSet fxMode] 
        profName:[ColorSyncUtils profileNameFromProfileRef:[docProfileSet fxModeProfile]]];

	// proof
    
    [self selectMenuItemForMode:mProofPopup mode:[docProfileSet prfMode]];
    // clear any previous profile name first
    [self appendProfileNameToMenuItemTitleForMode:mProofPopup mode:kOther profName:NULL];
    // now update for current mode
    [self appendProfileNameToMenuItemTitleForMode:mProofPopup 
        mode:[docProfileSet prfMode] 
        profName:[ColorSyncUtils profileNameFromProfileRef:[docProfileSet prfModeProfile]]];

	// dest
    
    [self selectMenuItemForMode:mDestPopup mode:[docProfileSet dstMode]];
    // clear any previous profile name first
    [self appendProfileNameToMenuItemTitleForMode:mDestPopup mode:kOther profName:NULL];
    // now update for current mode
    [self appendProfileNameToMenuItemTitleForMode:mDestPopup 
        mode:[docProfileSet dstMode] 
        profName:[ColorSyncUtils profileNameFromProfileRef:[docProfileSet destModeProfile]]];

	// QuickTime Graphics Importer ColorSync matching (Panther)
    if ([ColorSyncUtils isPantherQTColorMatchingTurnedOnForComponent:[qtGraphicsImporterObj graphicsImportComponent]])
    {
        [self enableQTColorSyncPopups];
        [self updateQTColorSyncPopupMenuItems];
        [self disableColorSyncProfilePopups];
    }
    else
    {
        [self enableColorSyncProfilePopups];
        [self disableQTColorSyncPopups];
    }

bail:

    return;
}


//////////
//
// disableQTColorSyncPopups:
// disable the QuickTime Graphics Importer ColorSync matching popup menus.
//
//////////

-(void)disableQTColorSyncPopups
{
    [mOverrideSourcePopup setEnabled:NO];
    [mOverrideDestPopup setEnabled:NO];
    if ([Utils MacOSPantherOrBetter])
    {
        [mPantherQTColorSyncMatching setState:NSOffState];
    }
    else
    {
        [mPantherQTColorSyncMatching setEnabled:NO];
    }
}

//////////
//
// enableQTColorSyncPopups:
// enable the QuickTime Graphics Importer ColorSync matching popup menus.
//
//////////

-(void)enableQTColorSyncPopups
{
    if ([Utils MacOSPantherOrBetter])
    {
        [mPantherQTColorSyncMatching setState:NSOnState];
    }
    [mOverrideSourcePopup setEnabled:YES];
    [mOverrideDestPopup setEnabled:YES];
}

//////////
//
// disableColorSyncProfilePopups:
// disable all the image profile popups.
//
//////////

-(void)disableColorSyncProfilePopups
{
	[mSourcePopup setEnabled:NO];
	[mAbstractPopup setEnabled:NO];
	[mProofPopup setEnabled:NO];
	[mDestPopup setEnabled:NO];
}

//////////
//
// enableColorSyncProfilePopups:
// enable all the image profile popups.
//
//////////

-(void)enableColorSyncProfilePopups
{
	[mSourcePopup setEnabled:YES];
	[mAbstractPopup setEnabled:YES];
	[mProofPopup setEnabled:YES];
	[mDestPopup setEnabled:YES];

    [mPantherQTColorSyncMatching setEnabled:YES];
}

//////////
//
// updateQTColorSyncPopupMenuItems:
// update popup menu item titles for the QuickTime Graphics
// Importer ColorSync matching popups.
//
//////////

-(void)updateQTColorSyncPopupMenuItems
{
	MyDocument *currentDoc = (MyDocument *)[Utils getCurrentDocument];
    GraphicsImporterImage *qtGraphicsImporterObj = [currentDoc graphicsImporterObj];
	CMProfileRef profileRef = nil;

	ComponentResult result = [qtGraphicsImporterObj QTGetDestColorSyncProfile:&profileRef];
    if (result != noErr) goto bail;
    [self appendProfileNameToMenuItemTitleForMode:mOverrideDestPopup 
        mode:kOther 
        profName:[ColorSyncUtils profileNameFromProfileRef:profileRef]];
    [self setMenuItemTitleForItemWithTag:mOverrideDestPopup tag:kNone title:@"None"];
    
	result = [qtGraphicsImporterObj QTGetSourceOverrideColorSyncProfile:&profileRef];
    if (result != noErr) goto bail;
    [self appendProfileNameToMenuItemTitleForMode:mOverrideSourcePopup 
        mode:kOther 
        profName:[ColorSyncUtils profileNameFromProfileRef:profileRef]];
    [self setMenuItemTitleForItemWithTag:mOverrideSourcePopup tag:kNone title:@"None"];

bail:

    return;
}

//////////
//
// togglePantherQTColorSyncMatching:
// turn on/off QuickTime Graphics Importer automatic ColorSync matching for
// the current document.
//
//////////

- (IBAction)togglePantherQTColorSyncMatching:(id)sender
{
	MyDocument *currentDoc = (MyDocument *)[Utils getCurrentDocument];
    GraphicsImporterImage *qtGraphicsImporterObj = [currentDoc graphicsImporterObj];

    if (qtGraphicsImporterObj == nil) goto bail;

	if ([sender state] == NSOnState)
	{
        [ColorSyncUtils setPantherQTColorMatchingForComponent:[qtGraphicsImporterObj graphicsImportComponent]];
        [currentDoc colorMatchImage];
		
        [self disableColorSyncProfilePopups];
		[self enableQTColorSyncPopups];

		[self updateQTColorSyncPopupMenuItems];
	}
	else
	{
        [ColorSyncUtils setPantherQTColorMatchingOffForComponent:[qtGraphicsImporterObj graphicsImportComponent]];
        [currentDoc colorMatchImage];

		[self enableColorSyncProfilePopups];
		[self disableQTColorSyncPopups];
	}
	
	[currentDoc drawDocumentImage];

bail:

    return;
}

//////////
//
// handleQTColorSyncPopups:
// handle selections for the QuickTime Graphics Importer ColorSync
// matching popups.
//
//////////

- (IBAction)handleQTColorSyncPopups:(id)sender
{
	OSType	tag = [[sender selectedItem] tag];
	CMProfileRef profileRef = nil;
    MyDocument *myDocument = (MyDocument *)[Utils getCurrentDocument];
    GraphicsImporterImage *qtGraphicsImporterObj = [myDocument graphicsImporterObj];

	if (sender == mOverrideSourcePopup)
	{
		if (tag == kNone)
		{
            [qtGraphicsImporterObj QTSetSourceOverrideColorSyncProfile:NULL];
		}
		else
		{
			[ColorSyncUtils userGetNewProfile:&profileRef];
		
			if (profileRef)
			{
                [qtGraphicsImporterObj QTSetSourceOverrideColorSyncProfile:profileRef];
			}
		}
	}
	else if (sender == mOverrideDestPopup)
	{
		if (tag == kNone)
		{
            [qtGraphicsImporterObj QTSetDestColorSyncProfile:NULL];
		}
		else
		{
			[ColorSyncUtils userGetNewProfile:&profileRef];
			if (profileRef)
			{
                [qtGraphicsImporterObj QTSetDestColorSyncProfile:profileRef];
			}
		}
	}

    [myDocument colorMatchImage];
    [myDocument drawDocumentImage];

	[self updateQTColorSyncPopupMenuItems];
}

//////////
//
// updatePopupStates:
// disable all popups if no document is currently being displayed.
//
//////////

-(void)updatePopupStates
{
	MyDocument *currentDoc = (MyDocument *)[Utils getCurrentDocument];
	if (!currentDoc)
	{
		[self disableColorSyncProfilePopups];
		[self disableQTColorSyncPopups];
		[mPantherQTColorSyncMatching setEnabled:NO];
	}
}


@end
