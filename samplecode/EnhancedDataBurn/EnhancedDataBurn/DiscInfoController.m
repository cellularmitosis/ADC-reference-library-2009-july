/*
     File:       DiscInfoController.m
 
     Contains:   Settings panel controller that provides control over volume
                 properties of the burn hierarchy root.
 
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

#import "DiscInfoController.h"
#import "AppController.h"
#import "FSTreeNode.h"

static NSArray*	propertyTagMappings = nil;
static NSArray* filesystemNameTagMappings = nil;

@implementation DiscInfoController

+ (void) initialize
{
	// Through clever arrangement of the tags of objects in the info panel,
	// we use these tags to index into an array of the filesystem properties.
	// When one of out UI items changes and sends it's action, we look up that
	// objects tag in this array, which gives us back the proper property to 
	// use as the dictionary key for the object value of the UI object.
	propertyTagMappings = [[NSArray alloc] initWithObjects:	DRISOMacExtensions,			//0
															DRISORockRidgeExtensions,	//1
															DRISOLevel,					//2
															DRVolumeSet,				//3
															DRPublisher,				//4
															DRDataPreparer,				//5
															DRApplicationIdentifier,	//6
															DRSystemIdentifier,			//7
															DRVolumeExpirationDate,		//8
															DRVolumeEffectiveDate,		//9
															DRCopyrightFile,			//10
															DRBibliographicFile,		//11
															DRAbstractFile,				//12
															DRDefaultDate,				//13
															DRVolumeCreationDate,		//14
															DRVolumeModificationDate,	//15
															DRVolumeCheckedDate,		//16
															nil];
															
	// In a similar arrangement to above, we do the same object tag -> index mappings
	// for the filesystem names.
	filesystemNameTagMappings = [[NSArray alloc] initWithObjects:	DRAllFilesystems,
																	DRISO9660,
																	DRISO9660LevelOne,
																	DRISO9660LevelTwo,
																	DRJoliet,
																	DRHFSPlus,
																	nil];
}

- (id) init
{
	if (self = [super init])
	{
		fsProperties = [[NSMutableDictionary alloc] initWithCapacity:1];
	}
	
	return self;
}

- (void) dealloc
{
	[fsProperties release];
	fsProperties = nil;
	
	[super dealloc];
}

- (IBAction) ok:(id)sender
{
	[NSApp stopModalWithCode:NSOKButton];
}

- (IBAction) cancel:(id)sender
{
	[NSApp stopModalWithCode:NSCancelButton];
}

- (IBAction)setVolumeProperty:(id)sender
{
	if ([DRISOLevel isEqualTo:[propertyTagMappings objectAtIndex:[sender tag]]])
	{
		// The ISO level needs special handling since the objectValue of a popup menu is the index of the
		// menu item, which starts at zero. We need it to start at 1.
		[fsProperties setObject:[NSNumber numberWithInt:[sender indexOfSelectedItem] + 1] forKey:DRISOLevel];
	}
	else
	{
		// But in every other case, we can just ask for the object value. Since everything is a date
		// or plain string, this can be handled by asking for the object value and 
		// passing that along.
		[fsProperties setObject:[sender objectValue] forKey:[propertyTagMappings objectAtIndex:[sender tag]]];
	}
}

- (IBAction)selectRootDirFile:(id)sender
{
	int	returnCode;
	
	// Get the current list of root files from the mail app controller. This will be the 
	// list of files that you can choose from. For any of the special ISO root files 
	// (copyright, bibliographic, abstract), these file MUST exist in the root directory
	// of the ISO volume. This is in the spec, it's not negotiable.
	rootFiles = [appController rootFiles];
	
	// setup and run the modal dialog.
	[okButton setEnabled:NO];
	[fileList selectRow:-1 byExtendingSelection:NO];
	returnCode = [NSApp runModalForWindow:fileChooser];
	[fileChooser orderOut:self];

	// User clicked OK, so we need to update display of the filename
	// as well as update the property we're setting.
	if (returnCode == NSOKButton)
	{
		// We know which text field to simulate an action from by looking at the tag of the
		// button that was pressed and subtracting off 100. Then we look up the text field
		// in the info window using viewWithTag:. We use this to set the text shown in the 
		// field to let the user know what file they selected.
		NSTextField*	propertyView = [[infoWindow contentView] viewWithTag:[sender tag] - 100];
		
		[propertyView setObjectValue:[[selectedItem nodeData] valueForKey:@"name"]];
		[fsProperties setObject:[(FSNodeData*)[selectedItem nodeData] fsObject] forKey:[propertyTagMappings objectAtIndex:[propertyView tag]]];
	}

	// we don't need to hold onto this anymore. We'll grab it again next time.
	rootFiles = nil;
}

- (IBAction)setFilesystemMask:(id)sender
{
	NSEnumerator*	iter;
	NSControl*		cntl;
	BOOL			included = [sender state];
	unsigned long	includedFilesystems = [filesystemRoot explicitFilesystemMask];
	
	// The correct filesystem is encoded in the tag of the object.
	// The filesystem mask is just a bit set in the includedFilesystems
	// variable. So we'll shift the bit by the tag of the object sending the 
	// message to set the correct bit mask.
	if (included)
		includedFilesystems |= (1<<[sender tag]);
	else
		includedFilesystems &= ~(1<<[sender tag]);
	
	// Set the explicit filesystem mask. This mask tells the framework
	// which filesystems to generate for the disc when it's applied to the
	// root folder.
	[filesystemRoot setExplicitFilesystemMask:includedFilesystems];
	
	// Now we'll enable/disable the contents of the specfic tab we're futzing with
	// to indicate to the user that they can't set any values.
	// First get the correct contents to use. We'll get an iterator to the 
	// subviews.
	if (sender == iso)
	{
		iter = [[[isoOptions contentView] subviews] objectEnumerator];
	}
	else if (sender == hfs)
	{		
		iter = [[[hfsOptions contentView] subviews] objectEnumerator];
	}
	else if (sender == joliet)
	{		
		iter = [[[jolietOptions contentView] subviews] objectEnumerator];
	}

	// Now iterate over all of the subviews. If they respond to the 
	// -setEnabled: method, call it to enable/disable the item.
	while ((cntl = [iter nextObject]) != NULL)
	{
		if ([cntl respondsToSelector:@selector(setEnabled:)])
			[cntl setEnabled:included];
	}
}

- (IBAction)setVolumeName:(id)sender
{
	// The correct filesystem is encoded in the tag of the object.
	NSString*	filesystem;
	NSString*	volumeName;
	int			index = [sender tag];
	
	// If it's index 1 (that's ISO), we need to look at the ISO level popup.
	// For the filesystem names, we need to get specific since while we're 
	// creating an ISO filesystem, we can only get/set item/volume names
	// based on DRISO9660LevelOne or DRISO9660LevelTwo naming. The reason for
	// this is that ISO has two different methods of handling filenames on the volume
	// Each one has different characters that are valid and lengths of the strings
	// used. Other volume formats don't have this quirk, so we only need to do it
	// for ISO.
	if (index == 1)
	{
		int isoLevel = [[fsProperties objectForKey:DRISOLevel] intValue];
		index = index + (isoLevel ? isoLevel : 1);
	}

	// Get the correct filesystem based on the index we got from the object tag.
	filesystem = [filesystemNameTagMappings objectAtIndex:index];
	
	[filesystemRoot setSpecificName:[sender stringValue] forFilesystem:filesystem];
	volumeName = [filesystemRoot specificNameForFilesystem:filesystem];
	
	// Now, on the other end, we need to do some work if we're changing the Joliet 
	// volume name. Normally Joliet has a length limit of 64 UTF-16 characters. But for 
	// the volume name, there's not enough space, in fact there's only 32 bytes of space
	// so we can have at most 16 characters in the name. This unfortunately can't be handled
	// in the framework since there's no way to distinguish a file/folder that simply hasn't
	// been added to a hierarchy and the root of the filesystem (which doesn't have a parent)
	if (index == 4)
	{
		if ([volumeName length] > 16)
		{
			NSRange	jolietVolumeRange = NSMakeRange(0, 16);
			volumeName = [volumeName substringWithRange:jolietVolumeRange];
			[filesystemRoot setSpecificName:volumeName forFilesystem:filesystem];
		}
	}
	
	// reset what the user typed in since they might have used illegal characters.
	[sender setStringValue:volumeName];
}

- (IBAction)userSelectedISOLevel:(id)sender
{
	[self setVolumeProperty:sender];
	
	// When the user selects the ISO level popup menu, we'll switch the volume name shown.
	[isoName setStringValue:[filesystemRoot specificNameForFilesystem:[filesystemNameTagMappings objectAtIndex:[sender indexOfSelectedItem] + 2]]];
}

- (IBAction)openInfoPanel:(id)sender
{
	if ([infoWindow isVisible] == NO)
	{
		if ([infoWindow setFrameUsingName:[infoWindow frameAutosaveName] force:YES] == NO)
		{
			NSRect	frame = [[appController window] frame];
			NSPoint newOrigin = NSMakePoint(frame.origin.x + frame.size.width + 20, frame.origin.y + frame.size.height);
		
			[infoWindow cascadeTopLeftFromPoint:newOrigin];
		}
	}
	
	[infoWindow orderFront:self];
}

- (void) setRoot:(DRFolder*)root
{
	NSString*	volumeName;
	
	filesystemRoot = root;

	// when we update the filesystem root, make sure that the text fields for the 
	// different filesystem names are all in sync.
	[hfsName setStringValue:[filesystemRoot specificNameForFilesystem:DRHFSPlus]];
	
	if ([[fsProperties objectForKey:DRISOLevel] intValue] == 1)
	{
		[isoName setStringValue:[filesystemRoot specificNameForFilesystem:DRISO9660LevelOne]];
	}
	else
	{
		[isoName setStringValue:[filesystemRoot specificNameForFilesystem:DRISO9660LevelTwo]];
	}
	
	volumeName = [filesystemRoot specificNameForFilesystem:DRJoliet];
	if ([volumeName length] > 16)
	{
		NSRange	jolietVolumeRange = NSMakeRange(0, 16);
		volumeName = [volumeName substringWithRange:jolietVolumeRange];
		[filesystemRoot setSpecificName:volumeName forFilesystem:DRJoliet];
	}
	
	[jolietName setStringValue:volumeName];
}

- (NSDictionary*)volumeProperties
{
	return [[fsProperties retain] autorelease];
}

#pragma mark -
#pragma mark ¥¥ÊData source methods
- (int)numberOfRowsInTableView:(NSTableView *)aTableView
{
	return [rootFiles count];
}

- (id)tableView:(NSTableView *)aTableView objectValueForTableColumn:(NSTableColumn *)tableColumn row:(int)rowIndex
{
	return [[[rootFiles objectAtIndex:rowIndex] nodeData] valueForKey:[tableColumn identifier]];
}

#pragma mark -
#pragma mark ¥¥ÊTable delegate methods
- (void)tableViewSelectionDidChange:(NSNotification *)aNotification
{
	NSTableView*	tv = [aNotification object];
	if ([tv selectedRow] != -1)
	{
		selectedItem = [rootFiles objectAtIndex:[tv selectedRow]];
		[okButton setEnabled:YES];
	}
	else
	{
		[okButton setEnabled:NO];
	}
}

@end
