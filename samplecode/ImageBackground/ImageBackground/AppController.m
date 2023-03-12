/*
	AppController.m
	Copyright © 2006, Apple Computer, Inc., all rights reserved.

	Application Controller Object, and Outline View Data Source.
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
#import "SimpleTreeNode.h"
#import "ImageAndTextCell.h"

// ================================================================
// Useful Macros
// ================================================================
#define INITIAL_INFODICT	@"InitInfo"
#define COLUMNID_NAME		@"NameColumn"

// Conveniences for accessing nodes, or the data in the node.
#define NODE(n)				((SimpleTreeNode*)n)
#define NODE_DATA(n) 		((SimpleNodeData*)[NODE((n)) nodeData])
#define SAFENODE(n) 		((SimpleTreeNode*)((n)?(n):(treeData)))

static NSImage* image1 = nil;
static NSImage* image2 = nil;

@implementation AppController

- (id)init
{
    NSDictionary *initData = nil;
    
    self = [super init];
    if (self==nil) return nil;
    
    // Load our initial outline view data from INITIAL_INFODICT.
    initData = [NSDictionary dictionaryWithContentsOfFile: [[NSBundle mainBundle] pathForResource: INITIAL_INFODICT ofType: @"dict"]];
    treeData = [[SimpleTreeNode treeFromDictionary: initData] retain];
    
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
    // Insert custom cell types into the table view, the standard one does text only.
    // We want one column to have text and images, and one to have check boxes.
    //
	NSTableColumn* tableColumn = [outlineView tableColumnWithIdentifier: COLUMNID_NAME];
    ImageAndTextCell* imageAndTextCell = [[[ImageAndTextCell alloc] init] autorelease];
    [imageAndTextCell setEditable: YES];
    [tableColumn setDataCell:imageAndTextCell];
	
	[self loadBackgroundImages];
	[self doToggleImageBackground: [allowBackgroundImage state]];
	
	folderImage = [NSImage imageNamed:@"folderImage.tiff"];
	leafImage = [NSImage imageNamed:@"leafImage.tiff"];	
}

- (void)loadBackgroundImages
{
	// load the images files from our resource path
	NSString* imageName;
	
	if (image1 == nil)
	{
		// load the image: use this method to obtain the file from resource path
		imageName = [[NSBundle mainBundle] pathForResource:@"Lake Don Pedro1" ofType:@"jpg"];
		image1 = [[NSImage alloc] initWithContentsOfFile:imageName];	// note no need to retain since we call "alloc"
	}
	
	if (image2 == nil)
	{
		// load the image: use this method to obtain the file from resource path
		imageName = [[NSBundle mainBundle] pathForResource:@"Lake Don Pedro2" ofType:@"jpg"];
		image2 = [[NSImage alloc] initWithContentsOfFile:imageName];	// note no need to retain since we call "alloc"
	}
}

- (void)doToggleImageBackground:(BOOL)useImageBackground
{
	if (useImageBackground)	// is the Background Image checkbox checked?
	{
		[self loadBackgroundImages];

		[outlineView setBackgroundImage: image1];
		[tableView setBackgroundImage: image2];
	}
	else
	{
		[tableView clearBackgroundImage];
		[outlineView clearBackgroundImage];
	}
}


// ================================================================
// Target / action methods. (most wired up in IB)
// ================================================================

- (IBAction)toggleImageBackground:(id)sender
{
	[self doToggleImageBackground: [sender state]];
}


// ================================================================
//  NSOutlineView data source methods. (The required ones)
// ================================================================

// Required methods.
- (id)outlineView:(NSOutlineView*)olv child:(int)index ofItem:(id)item
{
    return [SAFENODE(item) childAtIndex:index];
}

- (BOOL)outlineView:(NSOutlineView*)olv isItemExpandable:(id)item
{
    return [NODE_DATA(item) isGroup];
}

- (int)outlineView:(NSOutlineView*)olv numberOfChildrenOfItem:(id)item
{
    return [SAFENODE(item) numberOfChildren];
}

- (id)outlineView:(NSOutlineView*)olv objectValueForTableColumn:(NSTableColumn*)tableColumn byItem:(id)item
{
    id objectValue = nil;
    
    // The return value from this method is used to configure the state of the items cell via setObjectValue:
    if([[tableColumn identifier] isEqualToString: COLUMNID_NAME])
	{
		objectValue = [NODE_DATA(item) name];
    }
    
    return objectValue;
}

- (void)outlineView:(NSOutlineView*)olv setObjectValue:(id)object forTableColumn:(NSTableColumn*)tableColumn byItem:(id)item
{
    if([[tableColumn identifier] isEqualToString: COLUMNID_NAME])
	{
		[NODE_DATA(item) setName: object];
    }
}

// ================================================================
//  NSOutlineView delegate methods.
// ================================================================

- (BOOL)outlineView:(NSOutlineView*)olv shouldExpandItem:(id)item
{
    return [NODE_DATA(item) isExpandable];
}

- (void)outlineView:(NSOutlineView*)olv willDisplayCell:(NSCell*)cell forTableColumn:(NSTableColumn*)tableColumn item:(id)item
{    
    if ([[tableColumn identifier] isEqualToString: COLUMNID_NAME])
	{
		// make sure the image and text cell has an image
		if (item && ![NODE_DATA(item) iconRep])
		{
			if ([NODE_DATA(item) isLeaf])
				[NODE_DATA(item) setIconRep: leafImage];
			else
				[NODE_DATA(item) setIconRep: folderImage];
		}
		
		// set the image here since the value returned from outlineView:objectValueForTableColumn:... didn't specify the image part...
		[(ImageAndTextCell*)cell setImage: [NODE_DATA(item) iconRep]];
    }
}


// ================================================================
//  NSTableView dataSource method
// ================================================================

- (int)numberOfRowsInTableView: (NSTableView*)aTableView
{
    return 15;
}

- (id)tableView: (NSTableView*)tableView objectValueForTableColumn: (NSTableColumn*)tableColumn row:(int)rowIndex
{
    int columnNumber = [[tableColumn identifier] intValue];
    
    return [NSString stringWithFormat: @"%d", ((rowIndex + 1) * (columnNumber == 0 ? 1 : columnNumber))];
}

@end

