/*
     File:       FSPropertiesController.m
 
     Contains:   Base class providing most of the functionality needed to handle
                 setting the various properties associated with the files/folders
                 in a burn hierarchy.
 
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

#import "FSPropertiesController.h"

@interface NSView (EnablingHelper)

- (void) setEnabled:(BOOL)enabledFlag deep:(BOOL)goDeep;

@end

@implementation NSView (EnablingHelper)

- (void) setEnabled:(BOOL)enabledFlag deep:(BOOL)goDeep
{
	// Dis/Enable ourselves first.
	if ([self respondsToSelector:@selector(setEnabled:)])
		[(id)self setEnabled:enabledFlag];
	
	if (goDeep)
	{
		NSEnumerator*	iter = [[self subviews] objectEnumerator];
		NSControl*		subView;

		// Iiterate over all of the subviews. If they respond to the 
		// -setEnabled: method, call it to enable/disable the item.
		// Then recurse to handle that object's subviews.
		while ((subView = [iter nextObject]) != NULL)
		{
			[subView setEnabled:enabledFlag deep:goDeep];
		}
	}
}

@end

@implementation FSPropertiesController

- (NSString*) filesystem
{
	return @"";
}

- (DRFilesystemInclusionMask) mask
{
	return 0xFFFFFFFF;
}

- (void) inspect:(DRFSObject*) item
{
	inspectedItem = item;
	
	// an item can't be added to a filesystem if it's parent isn't being added.
	// This is taken care of by checking the effective mask. Getting the effective mask
	// evaluates the masks of the item's parent, grandparent, etc and comes up with the
	// correct inclusion mask taking all of them into account.
	
	// The parent's effective mask determines if we can change the explicit mask for this
	// filesystem.
	[included setEnabled:([[inspectedItem parent] effectiveFilesystemMask] & [self mask])];
	
	// The item's explicit mask sets the value of the checkbox. While it might not be changable
	// because of the above line of code, it reflects the explicit mask accurately.
	[included setState:([inspectedItem explicitFilesystemMask] & [self mask])];
	
	// now, disable the contents based on the state and enabled-ness of the checkbox. 
	[contentView setEnabled:([included state] && [included isEnabled]) deep:YES];
	
	[self updateNames];
	[self updateDates];
	[self updatePOSIX];
	[self updateSpecific];
}

- (IBAction) setIncludedBit:(id)sender
{
	DRFilesystemInclusionMask	mask = [inspectedItem explicitFilesystemMask];
	
	// The correct filesystem is encoded in the tag of the object.
	// The filesystem mask is just a bit set in the includedFilesystems
	// variable. So we'll shift the bit by the tag of the object sending the 
	// message to set the correct bit mask.
	if ([sender state])
	{
		mask |= [self mask];
	}
	else
	{
		mask &= ~[self mask];
	}
	
	// Set the explicit filesystem mask. This mask, along with the efective mask of the 
	// parent determines if the object is included in the specified filesytem.
	// See the documentation for a more complete explanation of how all this works.
	[inspectedItem setExplicitFilesystemMask:mask];
	
	// Now we'll enable/disable the contents of the specfic tab we're futzing with
	// to indicate to the user that they can't set any values.
	[contentView setEnabled:([sender state]) deep:YES];
}

- (void) updateNames
{	
	[baseName setStringValue:[inspectedItem baseName]];
	[specificName setStringValue:[inspectedItem specificNameForFilesystem:[self filesystem]]];
	[mangledName setStringValue:[inspectedItem mangledNameForFileSystem:[self filesystem]]];
}

- (void) updateDates
{
	// Each subclass sets up an array of property keys. Tags of the object in the view 
	// hierarchy are set to the index of the tag that corresponds to the particular item
	// in the property array. So we go and look up the correct property to set by
	// querying this array by using the tag obtained from the object whose value we need to set
	// [propertyMappings objectAtIndex:[foo tag]]

	[creationDate setObjectValue:[inspectedItem propertyForKey:[propertyMappings objectAtIndex:[creationDate tag]] inFilesystem:[self filesystem] mergeWithOtherFilesystems:NO]];
	
	[contentModDate setObjectValue:[inspectedItem propertyForKey:[propertyMappings objectAtIndex:[contentModDate tag]] inFilesystem:[self filesystem] mergeWithOtherFilesystems:NO]];
	
	[attributeModDate setObjectValue:[inspectedItem propertyForKey:[propertyMappings objectAtIndex:[attributeModDate tag]] inFilesystem:[self filesystem] mergeWithOtherFilesystems:NO]];
	
	[lastAccessedDate setObjectValue:[inspectedItem propertyForKey:[propertyMappings objectAtIndex:[lastAccessedDate tag]] inFilesystem:[self filesystem] mergeWithOtherFilesystems:NO]];
	
	[backupDate setObjectValue:[inspectedItem propertyForKey:[propertyMappings objectAtIndex:[backupDate tag]] inFilesystem:[self filesystem] mergeWithOtherFilesystems:NO]];
}

- (void) updatePOSIX
{
	unsigned short	mode;
	
	// Each subclass sets up an array of property keys. Tags of the object in the view 
	// hierarchy are set to the index of the tag that corresponds to the particular item
	// in the property array. So we go and look up the correct property to set by
	// querying this array by using the tag obtained from the object whose value we need to set
	// [propertyMappings objectAtIndex:[foo tag]]

	[uid setObjectValue:[inspectedItem propertyForKey:[propertyMappings objectAtIndex:[uid tag]] inFilesystem:[self filesystem] mergeWithOtherFilesystems:NO]];
	[gid setObjectValue:[inspectedItem propertyForKey:[propertyMappings objectAtIndex:[gid tag]] inFilesystem:[self filesystem] mergeWithOtherFilesystems:NO]];

	mode = [[inspectedItem propertyForKey:[propertyMappings objectAtIndex:[perms tag]] inFilesystem:[self filesystem] mergeWithOtherFilesystems:NO] unsignedShortValue];
	
	// All we're doing here is breaking out the bits of the POSIX mode into
	// descrete pieces so we can set the checkboxes in the tab.
	[[perms cellWithTag:2] setState:(0x0001 & (mode >> 6))];
	[[perms cellWithTag:1] setState:(0x0001 & (mode >> 7))];
	[[perms cellWithTag:0] setState:(0x0001 & (mode >> 8))];

	[[perms cellWithTag:5] setState:(0x0001 & (mode >> 3))];	
	[[perms cellWithTag:4] setState:(0x0001 & (mode >> 4))];
	[[perms cellWithTag:3] setState:(0x0001 & (mode >> 5))];
	
	[[perms cellWithTag:8] setState:(0x0001 & (mode >> 0))];
	[[perms cellWithTag:7] setState:(0x0001 & (mode >> 1))];
	[[perms cellWithTag:6] setState:(0x0001 & (mode >> 2))];

	[[perms cellWithTag:11] setState:(0x0001 & (mode >> 9))];
	[[perms cellWithTag:10] setState:(0x0001 & (mode >> 10))];
	[[perms cellWithTag:9] setState:(0x0001 & (mode >> 11))];
}

- (void) updateSpecific
{
	// nothing to do, this is for subclasses to handle their specific needs
}

- (IBAction) setFileName:(id)sender
{
	// Each subclass sets up an array of property keys. Tags of the object in the view 
	// hierarchy are set to the index of the tag that corresponds to the particular item
	// in the property array. So we go and look up the correct property to set by
	// querying this array by using the tag obtained from the sender
	// [propertyMappings objectAtIndex:[sender tag]]
	[inspectedItem setSpecificName:[sender objectValue] forFilesystem:[self filesystem]];
	
	[self updateNames];
}

- (IBAction) setProperty:(id)sender
{
	id objValue = [sender objectValue];
	
	// WOW, how easy is this? All we need to do is to grab the object value from the control
	// and use it for the property value. As long as you can give back the correct object 
	// type that the property is wanting (in the case of this app and this method it's always
	// NSStrings or NSDates, but you could get clever in your own) you can just grab the
	// -objectValue and set the property.
	
	// Each subclass sets up an array of property keys. Tags of the object in the view 
	// hierarchy are set to the index of the tag that corresponds to the particular item
	// in the property array. So we go and look up the correct property to set by
	// querying this array by using the tag obtained from the sender
	// [propertyMappings objectAtIndex:[sender tag]]

	if (objValue)
		[inspectedItem setProperty:objValue forKey:[propertyMappings objectAtIndex:[sender tag]] inFilesystem:[self filesystem]];
}

- (IBAction) setPOSIXModeProperty:(id)sender
{
	unsigned short mode = 0;
	
	// combine all of the checkbox states into bit values for the POSIX mode.
	mode |= [[sender cellWithTag:2] intValue] << 6;
	mode |= [[sender cellWithTag:1] intValue] << 7;
	mode |= [[sender cellWithTag:0] intValue] << 8;

	mode |= [[sender cellWithTag:5] intValue] << 3;	
	mode |= [[sender cellWithTag:4] intValue] << 4;
	mode |= [[sender cellWithTag:3] intValue] << 5;
	
	mode |= [[sender cellWithTag:8] intValue] << 0;
	mode |= [[sender cellWithTag:7] intValue] << 1;
	mode |= [[sender cellWithTag:6] intValue] << 2;

	mode |= [[sender cellWithTag:11] intValue] << 9;
	mode |= [[sender cellWithTag:10] intValue] << 10;
	mode |= [[sender cellWithTag:9] intValue] << 11;
	
	// Each subclass sets up an array of property keys. Tags of the object in the view 
	// hierarchy are set to the index of the tag that corresponds to the particular item
	// in the property array. So we go and look up the correct property to set by
	// querying this array by using the tag obtained from the sender
	// [propertyMappings objectAtIndex:[sender tag]]

	[inspectedItem setProperty:[NSNumber numberWithUnsignedShort:mode] forKey:[propertyMappings objectAtIndex:[sender tag]] inFilesystem:[self filesystem]];
}
	
@end
