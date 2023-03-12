/*
     File:       ItemInfoController.m
 
     Contains:   Settings panel controller that provides control over file/folder
                 properties of items in the burn hierarchy.
 
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

#import "ItemInfoController.h"
#import "NSOutlineView_Extensions.h"
#import "AppController.h"
#import "FSTreeNode.h"
#import "HFSPlusController.h"
#import "ISOController.h"
#import "JolietController.h"

@implementation ItemInfoController

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

- (BOOL)validateToolbarItem:(NSToolbarItem*)theItem
{
	return (BOOL)[appController validateToolbarItem:theItem];
}

- (BOOL)validateMenuItem:(id <NSMenuItem>)theItem 
{
	return (BOOL)[appController validateMenuItem:theItem];
}

- (void) selectionChanged:(NSNotification*)aNotification
{
	NSOutlineView*	outlineView = [aNotification object];
	
	if ([[outlineView allSelectedItems] count] == 0)
	{
		[tabs selectTabViewItemAtIndex:2];
	}
	else if ([[outlineView allSelectedItems] count] > 1)
	{
		[tabs selectTabViewItemAtIndex:1];
	}
	else
	{
		TreeNode*	selectedNode = [[outlineView allSelectedItems] objectAtIndex:0];
		DRFSObject*	itemToInspect = [((FSNodeData*)[selectedNode nodeData]) fsObject];
		[tabs selectTabViewItemAtIndex:0];

		[hfsController inspect:itemToInspect];
		[isoController inspect:itemToInspect];
		[jolietController inspect:itemToInspect];
	}
}

@end
