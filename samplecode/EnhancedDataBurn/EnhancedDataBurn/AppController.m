/*
     File:       AppController.m
 
     Contains:   Your basic run of the mill application controller/delegate.
 
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

#import "AppController.h"
#import "FSTreeNode.h"
#import "ImageAndTextCell.h"
#import "NSArray_Extensions.h"
#import "NSOutlineView_Extensions.h"
#import <DiscRecordingUI/DiscRecordingUI.h>
#import "DiscInfoController.h"
#import "ItemInfoController.h"

@interface AppController (Private)

- (void)_addNewDataToSelection:(TreeNode *)newChild;

@end

// ================================================================
// Useful Macros
// ================================================================

#define COLUMNID_NAME		 		@"name"
#define COLUMNID_KIND		 		@"kind"

// Conveniences for accessing nodes, or the data in the node.
#define NODE(n)				((TreeNode*)n)
#define SAFENODE(n) 		((TreeNode*)((n)?(n):(treeData)))
#define NODE_DATA(n)		((FSNodeData*)[SAFENODE(n) nodeData])

static NSString* 	EDBFileTreeDragPboardType 					= @"EDBFileTreeDragPboardType";

static NSString*	EDBToolbarIdentifier 						= @"EDBToolbarIdentifier";
static NSString*	EDBDeleteToolbarItemIdentifier 				= @"EDBDeleteToolbarItemIdentifier";
static NSString*	EDBNewVirtualFolderToolbarItemIdentifier 	= @"EDBNewVirtualFolderToolbarItemIdentifier";
static NSString*	EDBAddRealItemToolbarItemIdentifier 		= @"EDBAddRealItemToolbarItemIdentifier";
static NSString*	EDBBurnDiscToolbarItemIdentifier 			= @"EDBBurnDiscToolbarItemIdentifier";
static NSString*	EDBDiscInfoToolbarItemIdentifier			= @"EDBDiscInfoToolbarItemIdentifier";
static NSString*	EDBFileInfoToolbarItemIdentifier			= @"EDBFileInfoToolbarItemIdentifier";

static NSString*	EDBSelectionChangedNotification				= @"EDBSelectionChangedNotification";
static NSString*	EDBCurrentSelection							= @"EDBCurrentSelection";

@implementation AppController

- (id) init
{
	if (self = [super init])
	{
		DRFolder*	folderObj = [DRFolder virtualFolderWithName:@"Untitled Disc"];
		FSNodeData*	nodeData = [[[FSFolderNodeData alloc] initWithFSObject:folderObj] autorelease];
		
		// Set the eplicit mask for the root object. This make sure that all items added to it
		// get the correct filesystem mask inherited from the root. If we didn't set this here
		// we'd need to worry about possible changes to how the default mask value is interpreted
		// in different versions of the framework.
		[folderObj setExplicitFilesystemMask: (DRFilesystemInclusionMaskISO9660 | DRFilesystemInclusionMaskJoliet | DRFilesystemInclusionMaskHFSPlus)];
		treeData = [[FSTreeNode treeNodeWithData:nodeData] retain];
	}
	
	return self;
}

- (void)dealloc 
{
    [treeData release];
    treeData = nil;
	
	[super dealloc];
}

- (void)awakeFromNib 
{
	NSTableColumn*		tableColumn = nil;
	ImageAndTextCell*	imageAndTextCell = nil;
	
    // Insert custom cell types into the table view, the standard one does text only.
    // We want one column to have text and images, and one to have check boxes.
    tableColumn = [outlineView tableColumnWithIdentifier: COLUMNID_NAME];
    imageAndTextCell = [[[ImageAndTextCell alloc] init] autorelease];
    [imageAndTextCell setEditable:YES];
    [tableColumn setDataCell:imageAndTextCell];
    	
    // Register to get our custom type, strings, and filenames.... try dragging each into the view!
    [outlineView registerForDraggedTypes:[NSArray arrayWithObjects:EDBFileTreeDragPboardType, NSFilenamesPboardType, nil]];

	[outlineView setAutoresizesAllColumnsToFit:YES];
	[outlineView setAllowsColumnReordering:NO];
	
	[[NSNotificationCenter defaultCenter] addObserver:itemInfoController selector:@selector(selectionChanged:) name:NSOutlineViewSelectionDidChangeNotification object:outlineView];
	
	[discInfoController setRoot:[self filesystemRoot]];

	[self setupToolbar];
}

- (NSArray*)draggedNodes
{
	return draggedNodes;
}

- (NSWindow*) window 
{
	return mainWindow;
}

- (NSArray*) rootFiles
{
	NSEnumerator*	iter = [[treeData children] objectEnumerator];
	TreeNode*		child;
	NSMutableArray*	rootFiles = [NSMutableArray arrayWithCapacity:1];
	
	while ((child = [iter nextObject]) != NULL)
	{
		// A cheap (but somewhat lame) way of identifing a file is to check to see if it's expandable.
		// If not, it's a file.
		if ([NODE_DATA(child) isExpandable] == NO)
		{
			[rootFiles addObject:child];
		}
	}
	
	return [[rootFiles copy] autorelease];
}

- (DRFolder*) filesystemRoot
{
	return (DRFolder*)[NODE_DATA(treeData) fsObject];
}

#pragma mark -
#pragma mark ••  Toolbar methods

- (void) setupToolbar
{	
	NSToolbar *toolbar = [[[NSToolbar alloc] initWithIdentifier: EDBToolbarIdentifier] autorelease];
	
	[toolbar setAllowsUserCustomization: YES];
	[toolbar setAutosavesConfiguration: YES];
	[toolbar setDisplayMode: NSToolbarDisplayModeIconOnly];
    
    // We are the delegate
	[toolbar setDelegate: self];
    
    // Attach the toolbar to the document window 
	[mainWindow setToolbar: toolbar];
}

- (NSToolbarItem *) toolbar: (NSToolbar *)toolbar itemForItemIdentifier: (NSString *) itemIdent willBeInsertedIntoToolbar:(BOOL) willBeInserted {
    // Required delegate method:  Given an item identifier, this method returns an item 
    // The toolbar will use this method to obtain toolbar items that can be displayed in the customization sheet, or in the toolbar itself 
    NSToolbarItem *toolbarItem = [[[NSToolbarItem alloc] initWithItemIdentifier: itemIdent] autorelease];
    
	if ([itemIdent isEqual: EDBDeleteToolbarItemIdentifier]) 
	{
		// Set the text label to be displayed in the toolbar and customization palette 
		[toolbarItem setLabel: @"Remove"];
		[toolbarItem setPaletteLabel: @"Remove"];
	
		// Set up a reasonable tooltip, and image   Note, these aren't localized, but you will likely want to localize many of the item's properties 
		[toolbarItem setToolTip: @"Removes the selected item from the list"];
		[toolbarItem setImage: [NSImage imageNamed: @"delete"]];
	
		// Tell the item what message to send when it is clicked 
		[toolbarItem setTarget: self];
		[toolbarItem setAction: @selector(removeItem:)];
    } 
	else if ([itemIdent isEqual: EDBNewVirtualFolderToolbarItemIdentifier]) 
	{
		// Set the text label to be displayed in the toolbar and customization palette 
		[toolbarItem setLabel: @"New Virtual Folder"];
		[toolbarItem setPaletteLabel: @"New Virtual Folder"];
	
		// Set up a reasonable tooltip, and image   Note, these aren't localized, but you will likely want to localize many of the item's properties 
		[toolbarItem setToolTip: @"Creates a new virtual folder"];
		[toolbarItem setImage: [NSImage imageNamed: @"newDirectory"]];
	
		// Tell the item what message to send when it is clicked 
		[toolbarItem setTarget: self];
		[toolbarItem setAction: @selector(newVirtualFolder:)];
    } 
	else if ([itemIdent isEqual: EDBAddRealItemToolbarItemIdentifier]) 
	{
		// Set the text label to be displayed in the toolbar and customization palette 
		[toolbarItem setLabel: @"Add File/Folder"];
		[toolbarItem setPaletteLabel: @"Add File/Folder"];
	
		// Set up a reasonable tooltip, and image   Note, these aren't localized, but you will likely want to localize many of the item's properties 
		[toolbarItem setToolTip: @"Add a new item from the filesystem"];
		[toolbarItem setImage: [NSImage imageNamed: @"addFile"]];
	
		// Tell the item what message to send when it is clicked 
		[toolbarItem setTarget: self];
		[toolbarItem setAction: @selector(addRealItem:)];
    } 
	else if ([itemIdent isEqual: EDBBurnDiscToolbarItemIdentifier]) 
	{
		// Set the text label to be displayed in the toolbar and customization palette 
		[toolbarItem setLabel: @"Burn"];
		[toolbarItem setPaletteLabel: @"Burn"];
	
		// Set up a reasonable tooltip, and image   Note, these aren't localized, but you will likely want to localize many of the item's properties 
		[toolbarItem setToolTip: @"Burn the data to disc"];
		[toolbarItem setImage: [NSImage imageNamed: @"DRBurnIcon"]];
	
		// Tell the item what message to send when it is clicked 
		[toolbarItem setTarget: self];
		[toolbarItem setAction: @selector(burnDisc:)];
    } 
	else if ([itemIdent isEqual: EDBDiscInfoToolbarItemIdentifier]) 
	{
		// Set the text label to be displayed in the toolbar and customization palette 
		[toolbarItem setLabel: @"Volume Info"];
		[toolbarItem setPaletteLabel: @"Volume Info"];
	
		// Set up a reasonable tooltip, and image   Note, these aren't localized, but you will likely want to localize many of the item's properties 
		[toolbarItem setToolTip: @"Set the properties of the volumes burned to disc"];
		[toolbarItem setImage: [NSImage imageNamed: @"cdInfo"]];
	
		// Tell the item what message to send when it is clicked 
		[toolbarItem setTarget: discInfoController];
		[toolbarItem setAction: @selector(openInfoPanel:)];
    } 
	else if ([itemIdent isEqual: EDBFileInfoToolbarItemIdentifier]) 
	{
		// Set the text label to be displayed in the toolbar and customization palette 
		[toolbarItem setLabel: @"File/Folder Info"];
		[toolbarItem setPaletteLabel: @"File/Folder Info"];
	
		// Set up a reasonable tooltip, and image   Note, these aren't localized, but you will likely want to localize many of the item's properties 
		[toolbarItem setToolTip: @"Set the properties of the file that will be written to disc"];
		[toolbarItem setImage: [NSImage imageNamed: @"itemInfo"]];
	
		// Tell the item what message to send when it is clicked 
		[toolbarItem setTarget: itemInfoController];
		[toolbarItem setAction: @selector(openInfoPanel:)];
    } 
	else 
	{
		// itemIdent refered to a toolbar item that is not provide or supported by us or cocoa 
		// Returning nil will inform the toolbar this kind of item is not supported 
		toolbarItem = nil;
    }
	
    return toolbarItem;
}

- (NSArray *) toolbarDefaultItemIdentifiers: (NSToolbar *) toolbar {
    // Required delegate method:  Returns the ordered list of items to be shown in the toolbar by default    
    // If during the toolbar's initialization, no overriding values are found in the user defaults, or if the
    // user chooses to revert to the default items this set will be used 
    return [NSArray arrayWithObjects:	EDBAddRealItemToolbarItemIdentifier,
										EDBNewVirtualFolderToolbarItemIdentifier, 
										NSToolbarSeparatorItemIdentifier, 
										EDBDiscInfoToolbarItemIdentifier,
										EDBFileInfoToolbarItemIdentifier,
										NSToolbarSeparatorItemIdentifier, 
										EDBDeleteToolbarItemIdentifier, 
										NSToolbarFlexibleSpaceItemIdentifier, 
										EDBBurnDiscToolbarItemIdentifier, nil];
}

- (NSArray *) toolbarAllowedItemIdentifiers: (NSToolbar *) toolbar {
    // Required delegate method:  Returns the list of all allowed items by identifier.  By default, the toolbar 
    // does not assume any items are allowed, even the separator.  So, every allowed item must be explicitly listed   
    // The set of allowed items is used to construct the customization palette 
    return [NSArray arrayWithObjects: 	EDBAddRealItemToolbarItemIdentifier, 														EDBNewVirtualFolderToolbarItemIdentifier, 
										EDBDiscInfoToolbarItemIdentifier,
										EDBFileInfoToolbarItemIdentifier,
										EDBDeleteToolbarItemIdentifier, 
										EDBBurnDiscToolbarItemIdentifier,
										NSToolbarSeparatorItemIdentifier, 
										NSToolbarFlexibleSpaceItemIdentifier, 
										NSToolbarCustomizeToolbarItemIdentifier,
										NSToolbarSpaceItemIdentifier, nil];
}

- (BOOL)validateToolbarItem:(NSToolbarItem *)theItem
{
	SEL	theAction = [theItem action];

	if (theAction == @selector(addRealItem:) || theAction == @selector(newVirtualFolder:))
	{
		return (burning == NO);
	}
    if (theAction == @selector(removeItem:)) 
	{
		return (burning == NO) && ([[outlineView allSelectedItems] count] > 0);
    }   
	else if (theAction == @selector(burnDisc:))
	{
		return (burning == NO) && ([treeData numberOfChildren] > 0) && ([[self filesystemRoot] explicitFilesystemMask] != 0);
	}
	else if (theAction == @selector(openInfoPanel:) && ([theItem target] == self))
	{
		return (burning == NO);
	}
	else if (theAction == @selector(openInfoPanel:) && ([theItem target] == itemInfoController))
	{
		return (burning == NO) && ([[outlineView allSelectedItems] count] > 0);
	}
		
    return YES;
}

// ================================================================
// Target / action methods. (most wired up in IB)
// ================================================================

#pragma mark -
#pragma mark •• Target / action methods
- (IBAction)newVirtualFolder:(id)sender 
{
	[NSApp beginSheet:newFolderSheet modalForWindow:mainWindow modalDelegate:self didEndSelector:@selector(newFolderSheetDidEnd:returnCode:contextInfo:) contextInfo:NULL];
}

- (void) newFolderSheetDidEnd:(NSWindow*)panel returnCode:(int)returnCode contextInfo:(void*)contextInfo
{
	[panel orderOut:self];
	
	if (returnCode == NSOKButton)
	{
		DRFolder*	folderObj = [DRFolder virtualFolderWithName:[folderName stringValue]];
		id nodeData = [[FSFolderNodeData alloc] initWithFSObject:folderObj];
		
		if (nodeData)
		{
			FSTreeNode*	newNode = [FSTreeNode treeNodeWithData:nodeData];
			[self _addNewDataToSelection:newNode];
			[nodeData release];
		}
	}
}

- (IBAction)addRealItem:(id)sender 
{
	NSOpenPanel* op = [NSOpenPanel openPanel];
	
	[op setCanChooseDirectories:YES];
	[op setCanChooseFiles:YES];
	[op setAllowsMultipleSelection:YES];
	[op setResolvesAliases:NO];
	
	[op beginSheetForDirectory:NSHomeDirectory() file:@"" types:nil modalForWindow:mainWindow modalDelegate:self didEndSelector:@selector(addRealFileEnded:returnCode:contextInfo:) contextInfo:nil];
}

- (void) addRealFileEnded:(NSOpenPanel*)panel returnCode:(int)returnCode contextInfo:(void*)contextInfo
{
	[panel orderOut:self];
	
	if (returnCode == NSOKButton)
	{
		NSArray*		paths = [panel filenames];
		NSEnumerator*	iter = [paths objectEnumerator];
		NSString*		path;
		
		while ((path = [iter nextObject]) != NULL)
		{
			BOOL		isDir;
			id 			nodeData = nil;
			
			// Now that we've got the pathnames of the files/folders the user chose, 
			// create the appropriate DRFolder or DRFile object for each path
			// and put it into a FSNodeData obejct so that the disc hierarchy
			// outline table can manage it.
			if ([[NSFileManager defaultManager] fileExistsAtPath:path isDirectory:&isDir])
			{
				if (isDir)
				{
					DRFolder*	folderObj = [DRFolder folderWithPath:path];
					nodeData = [[FSFolderNodeData alloc] initWithFSObject:folderObj];
				}
				else
				{
					DRFile*	fileObj = [DRFile fileWithPath:path];
					nodeData = [[FSFileNodeData alloc] initWithFSObject:fileObj];
				}
			}
			
			if (nodeData)
			{
				FSTreeNode*	newNode = [FSTreeNode treeNodeWithData:nodeData];
				[self _addNewDataToSelection:newNode];
				[nodeData release];
			}
		}
		[outlineView reloadData];
	}
}

- (IBAction) ok:(id)sender
{
	[NSApp endSheet:newFolderSheet returnCode:NSOKButton];
}

- (IBAction) cancel:(id)sender
{
	[NSApp endSheet:newFolderSheet returnCode:NSCancelButton];
}

- (IBAction)burnDisc:(id)sender
{
	DRBurnSetupPanel*		bsp = [DRBurnSetupPanel setupPanel];
	DRFolder*				rootFolder = (DRFolder*)[(FSNodeData*)[treeData nodeData] fsObject];
	DRTrack* 				track = [DRTrack trackForRootFolder:rootFolder];
	NSMutableDictionary*	properties;
	
	// Combine the dictionaries for the track and the filesystem properties into one.
	// This makes sure that any properties which are default for the track and 
	// we have also set in the filesystem properties are properly combined as well
	// as saving any other properties that the track hase set for it.
	properties = [[track properties] mutableCopy];
	[properties addEntriesFromDictionary:[discInfoController volumeProperties]];
	[track setProperties:properties];
	
	[bsp setDelegate:self];
	[bsp beginSetupSheetForWindow:mainWindow modalDelegate:self didEndSelector:@selector(burnSetupEnded:returnCode:contextInfo:) contextInfo:[track retain]];
}

- (void) burnSetupEnded:(DRBurnSetupPanel*)burnPanel returnCode:(int)returnCode contextInfo:(void*)contextInfo
{
	if (returnCode == NSOKButton)
	{
		DRBurnProgressPanel*	bpp = [DRBurnProgressPanel progressPanel];

		[burnPanel orderOut:self];

		[bpp setDelegate:self];
		[bpp beginProgressSheetForBurn:[burnPanel burnObject] layout:[(id)contextInfo autorelease] modalForWindow:mainWindow];
	}
}

- (void) burnProgressPanelWillBegin:(NSNotification*)aNotification
{
	burning = YES;
}

- (void) burnProgressPanelDidFinish:(NSNotification*)aNotification
{
	burning = NO;
}

- (IBAction)removeItem:(id)sender
{
    NSArray *selection = [outlineView allSelectedItems];
    
    // Tell all of the selected nodes to remove themselves from the model.
    [selection makeObjectsPerformSelector: @selector(removeFromParent)];
    [outlineView deselectAll:nil];
    [outlineView reloadData];
}

- (IBAction)outlineViewAction:(id)sender
{
	[[NSNotificationCenter defaultCenter] postNotificationName:EDBSelectionChangedNotification object:self userInfo:[NSDictionary dictionaryWithObjectsAndKeys:[outlineView allSelectedItems], EDBCurrentSelection, nil]];
}

- (BOOL)validateMenuItem:(id <NSMenuItem>)theItem
{
	SEL	theAction = [theItem action];

	if (theAction == @selector(addRealItem:) || theAction == @selector(newVirtualFolder:))
	{
		return (burning == NO);
	}
    if (theAction == @selector(removeItem:)) 
	{
		return (burning == NO) && ([[outlineView allSelectedItems] count] > 0);
    }   
	else if (theAction == @selector(burnDisc:))
	{
		return (burning == NO) && ([treeData numberOfChildren] > 0) && ([[self filesystemRoot] explicitFilesystemMask] != 0);
	}
	else if (theAction == @selector(openInfoPanel:) && ([theItem target] == self))
	{
		return (burning == NO);
	}
	else if (theAction == @selector(openInfoPanel:) && ([theItem target] == itemInfoController))
	{
		return (burning == NO) && ([[outlineView allSelectedItems] count] > 0);
	}
	else if ((theAction == @selector(terminate:)))
	{
		return (burning == NO) && ([treeData numberOfChildren] > 0);
	}	
	return YES;
}

- (NSApplicationTerminateReply)applicationShouldTerminate:(NSApplication *)sender
{
	if (burning)
		return NSTerminateCancel;
	else
		return NSTerminateNow;
}

- (void) showHelp:(id)sender
{
	BOOL	showWindow = YES;
	
	if ([[helpText string] length] == 0)
		showWindow = [helpText readRTFDFromFile:[[NSBundle mainBundle] pathForResource:@"ReadMe" ofType:@"rtfd"]];

	if (showWindow)
		[helpWindow makeKeyAndOrderFront:sender];
}

// ================================================================
//  NSOutlineView data source methods. (The required ones)
// ================================================================

#pragma mark -
#pragma mark •• Data source methods

// Required methods.
- (id)outlineView:(NSOutlineView *)olv child:(int)index ofItem:(id)item 
{
    return [SAFENODE(item) childAtIndex:index];
}

- (BOOL)outlineView:(NSOutlineView *)olv isItemExpandable:(id)item 
{
    return [NODE_DATA(item) isExpandable];
}

- (int)outlineView:(NSOutlineView *)olv numberOfChildrenOfItem:(id)item 
{
    return [SAFENODE(item) numberOfChildren];
}

- (id)outlineView:(NSOutlineView *)olv objectValueForTableColumn:(NSTableColumn *)tableColumn byItem:(id)item 
{
	return [NODE_DATA(item) valueForKey:[tableColumn identifier]];
}


// ================================================================
//  NSOutlineView delegate methods.
// ================================================================

#pragma mark -
#pragma mark •• Delegate Methods

// We need to make sure that we make a real folder virtual if
// it's about to be expanded.
- (void)outlineViewItemWillExpand:(NSNotification *)notification;
{
	id	item = SAFENODE([[notification userInfo] objectForKey:@"NSObject"]);
	[item children];
}

- (BOOL)outlineView:(NSOutlineView *)olv shouldExpandItem:(id)item 
{
    return [NODE_DATA(item) isExpandable];
}

- (void)outlineView:(NSOutlineView *)olv willDisplayCell:(NSCell *)cell forTableColumn:(NSTableColumn *)tableColumn item:(id)item 
{    
    if ([[tableColumn identifier] isEqualToString: COLUMNID_NAME]) 
	{
		[(ImageAndTextCell*)cell setImage:[NODE_DATA(item) icon]];
	}
}

// ================================================================
//  NSOutlineView data source methods. (dragging related)
// ================================================================
#pragma mark -
#pragma mark •• Dragging Methods

- (BOOL)outlineView:(NSOutlineView *)olv writeItems:(NSArray*)items toPasteboard:(NSPasteboard*)pboard 
{
    draggedNodes = items; // Don't retain since this is just holding temporaral drag information, and it is only used during a drag!  We could put this in the pboard actually.
    
    // Provide data for our custom type, and simple NSStrings.
    [pboard declareTypes:[NSArray arrayWithObjects: EDBFileTreeDragPboardType, NSStringPboardType, nil] owner:self];
    
    // the actual data doesn't matter since EDBFileTreeDragPboardType drags aren't recognized by anyone but us!.
    [pboard setData:[NSData data] forType:EDBFileTreeDragPboardType]; 
    
    // Put string data on the pboard... notice you can drag into TextEdit!
    [pboard setString:[draggedNodes description] forType: NSStringPboardType];

    return YES;
}

- (unsigned int)outlineView:(NSOutlineView*)olv validateDrop:(id <NSDraggingInfo>)info proposedItem:(id)item proposedChildIndex:(int)childIndex 
{
    // This method validates whether or not the proposal is a valid one. Returns NO if the drop should not be allowed.
    TreeNode *target = item;
    BOOL targetIsValid = YES;
	
	// Check to make sure we don't allow a node to be inserted into one of its descendants!
	if ([info draggingSource] == outlineView && 
		[[info draggingPasteboard] availableTypeFromArray:[NSArray arrayWithObject:EDBFileTreeDragPboardType]] != nil) 
	{
	    NSArray *_draggedNodes = [[[info draggingSource] dataSource] draggedNodes];
	    targetIsValid = ![target isDescendantOfNodeInArray: _draggedNodes];
	}

	if (targetIsValid)
	{
		if ([NODE_DATA(target) isExpandable] == NO)
		{
			target = [target nodeParent];
		}
		
		if (target == treeData)
		{
			target = nil;
		}
		[outlineView setDropItem:target dropChildIndex:NSOutlineViewDropOnItemIndex];
	}
	
    return targetIsValid ? NSDragOperationGeneric : NSDragOperationNone;
}

- (void)_performDropOperation:(id <NSDraggingInfo>)info ontoItem:(TreeNode*)parent 
{
    // Helper method to insert dropped data into the model. 
    NSPasteboard*	pboard = [info draggingPasteboard];
    NSMutableArray*	itemsToSelect = nil;
    
    // Do the appropriate thing depending on whether the data is EDBFileTreeDragPboardType or NSStringPboardType.
    if ([pboard availableTypeFromArray:[NSArray arrayWithObjects:EDBFileTreeDragPboardType, nil]] != nil) 
	{
        AppController *dragDataSource = [[info draggingSource] dataSource];
        NSArray *_draggedNodes = [TreeNode minimumNodeCoverFromNodesInArray: [dragDataSource draggedNodes]];
        NSEnumerator *iter = [_draggedNodes objectEnumerator];
        TreeNode *_draggedNode = nil;
        
		itemsToSelect = [NSMutableArray arrayWithArray:[outlineView allSelectedItems]];
	
        while ((_draggedNode = [iter nextObject]) != nil) 
		{
			[_draggedNode removeFromParent];
			[parent addChild:_draggedNode];
        }
    }
	else if ([pboard availableTypeFromArray:[NSArray arrayWithObjects:NSFilenamesPboardType, nil]] != nil) 
	{
		NSArray* 		paths = [pboard propertyListForType:NSFilenamesPboardType];
		NSEnumerator*	iter = [paths objectEnumerator];
		NSString*		path;
		
		itemsToSelect = [NSMutableArray arrayWithArray:[outlineView allSelectedItems]];

		while ((path = [iter nextObject]) != NULL)
		{
			id nodeData = [FSNodeData nodeDataWithPath:path];
			
			if (nodeData)
			{
				FSTreeNode*	newNode = [FSTreeNode treeNodeWithData:nodeData];
				[parent addChild:newNode];
			}
		}
	}

    [outlineView reloadData];
    [outlineView selectItems: itemsToSelect byExtendingSelection: NO];
}

- (BOOL)outlineView:(NSOutlineView*)olv acceptDrop:(id <NSDraggingInfo>)info item:(id)targetItem childIndex:(int)childIndex 
{
    TreeNode* 		dropParent = nil;
    
    // Determine the parent to insert into and the child index to insert at.
    if ([NODE_DATA(targetItem) isExpandable] == NO) 
	{
        dropParent = (TreeNode*)(childIndex == NSOutlineViewDropOnItemIndex ? [targetItem nodeParent] : targetItem);
    } 
	else
	{            
        dropParent = targetItem;
    }
    
    [self _performDropOperation:info ontoItem:SAFENODE(dropParent)];

    return YES;
}

@end

@implementation AppController (Private)

- (void)_addNewDataToSelection:(TreeNode *)newChild 
{
	int			newRow = 0;
    NSArray*	selectedNodes = [outlineView allSelectedItems];
    TreeNode*	selectedNode = ([selectedNodes count] ? [selectedNodes objectAtIndex:0] : treeData);
    TreeNode*	parentNode = nil;
		
	if ([NODE_DATA(selectedNode) isExpandable]) 
	{ 
		parentNode = selectedNode;
    }
    else 
	{ 
		parentNode = [selectedNode nodeParent]; 
		
		[outlineView expandItem:parentNode];
    }
 
 	[parentNode addChild:newChild];
	
	[outlineView reloadData];
	
    newRow = [outlineView rowForItem:newChild];
	
	if (newRow >= 0)
	{
		[outlineView selectRow:newRow byExtendingSelection:NO];
		[outlineView scrollRowToVisible:newRow];
	}
}

@end
