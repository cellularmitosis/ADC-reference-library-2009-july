/*
     File:       HFSPlusController.m
 
     Contains:   FSPropertyController subclass to handle the HFS+ filesystem.
 
     Version:    Technology: Mac OS X
                 Release:    Mac OS X
 
     Copyright:  (c) 2002 by Apple Computer, Inc., all rights reserved
 
     Bugs?:      For bug reports, consult the following page on
                 the World Wide Web:
 
                     http://developer.apple.com/bugreporter/
*/

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

#import "HFSPlusController.h"

#import <Carbon/Carbon.h>

@implementation HFSPlusController

- (id) init
{
	if (self = [super init])
	{
		// Like in other places, we're doing an object tag -> property mapping to easily
		// convert between the two worlds.
		propertyMappings = [[NSArray alloc] initWithObjects:	DRCreationDate,						//0
																DRContentModificationDate,			//1
																DRAttributeModificationDate,		//2
																DRAccessDate,						//3
																DRBackupDate,						//4
																DRPosixFileMode,					//5
																DRPosixUID,							//6
																DRPosixGID,							//7
																DRHFSPlusTextEncodingHint,			//8
																DRHFSPlusCatalogNodeID,				//9
																DRMacFileType,						//10
																DRMacFileCreator,					//11
																DRMacWindowBounds,					//12
																DRMacIconLocation,					//13
																DRMacScrollPosition,				//14
																DRMacWindowView,					//15
																DRMacFinderFlags,					//16
																DRMacExtendedFinderFlags,			//17
																nil];
	}

	return self;
}

- (NSString*) filesystem
{
	// We're the controller for the HFS+ filesystem, so return the correct value.
	return DRHFSPlus;
}

- (DRFilesystemInclusionMask) mask
{
	// We're the controller for the HFS+ filesystem, so return the correct value.
	return DRFilesystemInclusionMaskHFSPlus;
}

- (void) updateSpecific
{
	Point*			iconPosition;
	Rect*			windowBounds;
	Point*			scrollPosition;
	NSData*			data;
	unsigned short	fndrFlags = 0;
	NSEnumerator*	iter;
	NSCell*			cell; 
	
	[tecHint setObjectValue:[inspectedItem propertyForKey:DRHFSPlusTextEncodingHint inFilesystem:[self filesystem] mergeWithOtherFilesystems:NO]];
	[nodeID setObjectValue:[inspectedItem propertyForKey:DRHFSPlusCatalogNodeID inFilesystem:[self filesystem] mergeWithOtherFilesystems:NO]];

	data = [inspectedItem propertyForKey:DRMacFileCreator inFilesystem:[self filesystem] mergeWithOtherFilesystems:NO];
	if (data)
		[creator setStringValue:[NSString stringWithCString:[data bytes] length:4]];
	else
		[creator setStringValue:@""];
		
	data = [inspectedItem propertyForKey:DRMacFileType inFilesystem:[self filesystem] mergeWithOtherFilesystems:NO];
	if (data)
		[type setStringValue:[NSString stringWithCString:[data bytes] length:4]];
	else
		[type setStringValue:@""];
	
	data = [inspectedItem propertyForKey:DRMacWindowBounds inFilesystem:[self filesystem] mergeWithOtherFilesystems:NO];
	if (data)
	{
		windowBounds = (Rect*)[data bytes];
	
		[boundsTop setIntValue:windowBounds->top];
		[boundsLeft setIntValue:windowBounds->left];
		[boundsBottom setIntValue:windowBounds->bottom];
		[boundsRight setIntValue:windowBounds->right];
	}
	else
	{
		[boundsTop setStringValue:@""];
		[boundsLeft setStringValue:@""];
		[boundsBottom setStringValue:@""];
		[boundsRight setStringValue:@""];
	}
	
	data = [inspectedItem propertyForKey:DRMacIconLocation inFilesystem:[self filesystem] mergeWithOtherFilesystems:NO];
	if (data)
	{
		iconPosition = (Point*)[data bytes];
		
		[iconPosX setIntValue:iconPosition->h];
		[iconPosY setIntValue:iconPosition->v];
	}
	else
	{
		[iconPosX setStringValue:@""];
		[iconPosY setStringValue:@""];
	}
	
	data = [inspectedItem propertyForKey:DRMacScrollPosition inFilesystem:[self filesystem] mergeWithOtherFilesystems:NO];
	if (data)
	{
		scrollPosition = (Point*)[data bytes];
		
		[scrollPosX setIntValue:scrollPosition->h];
		[scrollPosY setIntValue:scrollPosition->v];
	}
	else
	{
		[scrollPosX setStringValue:@""];
		[scrollPosY setStringValue:@""];
	}
	
	[viewType setObjectValue:[inspectedItem propertyForKey:DRMacWindowView inFilesystem:[self filesystem] mergeWithOtherFilesystems:NO]];
	

	fndrFlags = [[inspectedItem propertyForKey:DRMacFinderFlags inFilesystem:[self filesystem] mergeWithOtherFilesystems:NO] unsignedShortValue];
	iter = [[finderFlags cells] objectEnumerator];
	while ((cell = [iter nextObject]) != nil)
	{
		[cell setState:([cell tag] & fndrFlags)];
	}


	fndrFlags = [[inspectedItem propertyForKey:DRMacExtendedFinderFlags inFilesystem:[self filesystem] mergeWithOtherFilesystems:NO] unsignedShortValue];
	iter = [[extFinderFlags cells] objectEnumerator];
	while ((cell = [iter nextObject]) != nil)
	{
		[cell setState:([cell tag] & fndrFlags) == [cell tag]];
	}
}

- (IBAction) setIconPositionProperty:(id)sender
{
	NSData*	positionData;
	Point	iconPosition;
	
	iconPosition.h = [iconPosX intValue];
	iconPosition.v = [iconPosY intValue];
	
	positionData = [NSData dataWithBytes:&iconPosition length:sizeof(iconPosition)];
	
	[inspectedItem setProperty:positionData forKey:DRMacIconLocation inFilesystem:[self filesystem]];
}

- (IBAction) setFlagsProperty:(id)sender
{
	unsigned short	fndrFlags = 0;
	NSEnumerator*	iter = [[sender cells] objectEnumerator];
	NSCell*			cell; 

	while ((cell = [iter nextObject]) != nil)
	{
		if ([cell state])
			fndrFlags |= [cell tag];
	}
	
	[inspectedItem setProperty:[NSNumber numberWithUnsignedShort:fndrFlags] forKey:[propertyMappings objectAtIndex:[sender tag]] inFilesystem:[self filesystem]];
}

- (IBAction) setFolderBoundsProperty:(id)sender
{
	NSData*	boundsData;
	Rect	windowBounds;
	
	windowBounds.top = [boundsTop intValue];
	windowBounds.left = [boundsLeft intValue];
	windowBounds.bottom = [boundsBottom intValue];
	windowBounds.right = [boundsRight intValue];
	
	boundsData = [NSData dataWithBytes:&windowBounds length:sizeof(windowBounds)];
	
	[inspectedItem setProperty:boundsData forKey:DRMacWindowBounds inFilesystem:[self filesystem]];
}

- (IBAction) setFolderScrollPositionProperty:(id)sender
{
	NSData*	positionData;
	Point	scrollPosition;
	
	scrollPosition.h = [scrollPosX intValue];
	scrollPosition.v = [scrollPosY intValue];
	
	positionData = [NSData dataWithBytes:&scrollPosition length:sizeof(scrollPosition)];
	
	[inspectedItem setProperty:positionData forKey:DRMacScrollPosition inFilesystem:[self filesystem]];
}

- (IBAction) setTypeCreatorProperty:(id)sender
{
	NSData*		data = [[sender stringValue] dataUsingEncoding:CFStringConvertEncodingToNSStringEncoding(kCFStringEncodingMacRoman) 
										  allowLossyConversion:YES];
										  
	data = [NSData dataWithBytes:[data bytes] length:4];
	
	[inspectedItem setProperty:data forKey:[propertyMappings objectAtIndex:[sender tag]] inFilesystem:[self filesystem]];
}

@end
