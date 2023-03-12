/*

File: MyWindowController.m

Abstract: Header file for this sample's main NSWindowController "TestWindow"

Version: 1.0

Disclaimer: IMPORTANT:  This Apple software is supplied to you by Apple
Inc. ("Apple") in consideration of your agreement to the
following terms, and your use, installation, modification or
redistribution of this Apple software constitutes acceptance of these
terms.  If you do not agree with these terms, please do not use,
install, modify or redistribute this Apple software.

In consideration of your agreement to abide by the following terms, and
subject to these terms, Apple grants you a personal, non-exclusive
license, under Apple's copyrights in this original Apple software (the
"Apple Software"), to use, reproduce, modify and redistribute the Apple
Software, with or without modifications, in source and/or binary forms;
provided that if you redistribute the Apple Software in its entirety and
without modifications, you must retain this notice and the following
text and disclaimers in all such redistributions of the Apple Software. 
Neither the name, trademarks, service marks or logos of Apple Inc.
may be used to endorse or promote products derived from the Apple
Software without specific prior written permission from Apple.  Except
as expressly stated in this notice, no other rights or licenses, express
or implied, are granted by Apple herein, including but not limited to
any patent rights that may be infringed by your derivative works or by
other works in which the Apple Software may be incorporated.

The Apple Software is provided by Apple on an "AS IS" basis.  APPLE
MAKES NO WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION
THE IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS
FOR A PARTICULAR PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND
OPERATION ALONE OR IN COMBINATION WITH YOUR PRODUCTS.

IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL
OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION,
MODIFICATION AND/OR DISTRIBUTION OF THE APPLE SOFTWARE, HOWEVER CAUSED
AND WHETHER UNDER THEORY OF CONTRACT, TORT (INCLUDING NEGLIGENCE),
STRICT LIABILITY OR OTHERWISE, EVEN IF APPLE HAS BEEN ADVISED OF THE
POSSIBILITY OF SUCH DAMAGE.

Copyright © 2006-2007 Apple Inc. All Rights Reserved

*/

#import "MyWindowController.h"

@implementation MyWindowController


// explanation string to instruct the user what double-click operation on a path component does:
NSString *const kFinderTitle = @"Double-click a path component to reveal it in the Finder.";
NSString *const kCustomTitle = @"Double-click a path component to reveal it on the web."; 

// explanation/place holder string to instruct the user on how they can set the path
NSString *const kExplainTitle = @"<html><center>Drag a file system object to this area or click 'Set Path...'</center></html>";


// -------------------------------------------------------------------------------
//	initWithPath:newPath
// -------------------------------------------------------------------------------
- initWithPath:(NSString*)newPath
{
    return [super initWithWindowNibName:@"TestWindow"];
}

// -------------------------------------------------------------------------------
//	dealloc
// -------------------------------------------------------------------------------
-(void)dealloc
{
	[customSizeController release];
	[super dealloc];
}

// -------------------------------------------------------------------------------
//	awakeFromNib
// -------------------------------------------------------------------------------
- (void)awakeFromNib
{
	[myPathControl setPathStyle: NSPathStyleStandard];
	NSColor* defaultColor = [myPathControl backgroundColor];
	[bkColorWell setColor: defaultColor];
	
	// make the place holder string
	NSString* myHTMLString = [NSString stringWithString:kExplainTitle];
	NSData* myData = [myHTMLString dataUsingEncoding:NSUTF8StringEncoding];
	NSAttributedString* textToBeInserted = [[[NSAttributedString alloc] initWithHTML:myData documentAttributes:nil] autorelease];
	[[myPathControl cell] setPlaceholderAttributedString: textToBeInserted];
	
    [myPathControl setTarget:self];
    [myPathControl setDoubleAction:@selector(pathControlDoubleClick:)];
	[myPathControl setDelegate:self];
	
	[explainText setHidden: YES];
}

// -------------------------------------------------------------------------------
//	updateExplainText:
//
//	This get called whenever we need to update the explanation string that instructs
//	the user how they can reveal the path component (via the Finder or Safari).
// -------------------------------------------------------------------------------
-(void)updateExplainText
{	
	NSUInteger numItems = [[myPathControl pathComponentCells] count];
	
	// if the control has a path (more then 0 components), output the explanatory text
	if (numItems > 0)
	{
		[explainText setHidden: NO];
		
		if ([myPathControl pathStyle] == NSPathStyleStandard || [myPathControl pathStyle] == NSPathStyleNavigationBar)
		{
			if (useCustomPath)
				[explainText setStringValue:kCustomTitle];
			else
				[explainText setStringValue:kFinderTitle];
		}
		else
		{
			// all other path styles have no explanatory text
			[explainText setStringValue:@""];
		}
	}
	else
	{
		// no path components, don't use explanatory text
		[explainText setHidden:YES];
	}
	
	[explainText setNeedsDisplay: YES];
}

// -------------------------------------------------------------------------------
//	openPanelDidEnd:returnCode:contextInfo:
//
//	Called when the NSOpenPanel (via "Set Path..." button) completes.
// -------------------------------------------------------------------------------
- (void)openPanelDidEnd:(NSOpenPanel*)panel returnCode:(int)returnCode contextInfo:(void *)contextInfo
{
	// hide the open panel
	[panel orderOut:self];
	
	// if the return code wasn't ok, don't do anything.
	if (returnCode != NSOKButton)
		return;
	
	// get the first URL returned from the Open Panel and set it at the first path component of the control
	NSArray* paths = [panel URLs];
	NSURL* url = [paths objectAtIndex: 0];
	[myPathControl setURL: url];
	
	[self updateExplainText];	// update the explanation text to show the user how they can reveal the path component
}

// -------------------------------------------------------------------------------
//	getPath:sender:
//
//	User clicked "Set Path..." button to pick a file system object.
// -------------------------------------------------------------------------------
- (IBAction)getPath:(id)sender
{
	NSOpenPanel* panel = [NSOpenPanel openPanel];
	[panel setAllowsMultipleSelection:NO];
	[panel setCanChooseDirectories:YES];
	[panel setCanChooseFiles:YES];
	[panel setResolvesAliases:YES];
	[panel setTitle:@"Choose a file object"];
	[panel setPrompt:@"Choose"];
	
	[panel beginSheetForDirectory:nil file:nil types:nil modalForWindow:[self window]
		   modalDelegate:self
		   didEndSelector:@selector(openPanelDidEnd:returnCode:contextInfo:)
		   contextInfo:self];
}

// -------------------------------------------------------------------------------
//	selectStyle:sender:
//
//	The user selected a path style from the list of radio buttons, so we update the
//	path control and explanation text as appropriate.
// -------------------------------------------------------------------------------
- (IBAction)selectStyle:(id)sender
{
	NSInteger tag = [[sender selectedCell]tag];
	switch (tag)
	{
		case NSPathStyleStandard:
			[myPathControl setPathStyle: NSPathStyleStandard];
			break;
		
		case NSPathStyleNavigationBar:
			[myPathControl setPathStyle: NSPathStyleNavigationBar];
			break;
			
		case NSPathStylePopUp:
			[myPathControl setPathStyle: NSPathStylePopUp];
			break;
	}
	[self updateExplainText];	// update the explanation text to show the user how they can reveal the path component
}

// -------------------------------------------------------------------------------
//	selectSize:sender:
//
//	The user chose which font size to use when displaying the path component titles.
//	It sets both the font size and control size for the control.
// -------------------------------------------------------------------------------
- (IBAction)selectSize:(id)sender
{
	NSControlSize size = [[sender selectedCell] tag];
	[myPathControl setFont:[NSFont systemFontOfSize: [NSFont systemFontSizeForControlSize: size]]];
	[[myPathControl cell] setControlSize:size];
}

// -------------------------------------------------------------------------------
//	setColor:sender:
//
//	The user wants to choose a different background color.
//	We ask the NSColorWell which color was returned from its color panel and
//	in turn set the background color the the control.
// -------------------------------------------------------------------------------
- (IBAction)setColor:(id)sender
{
	NSColor* newColor;
	NSColorWell* colorWell = sender;
	newColor = [colorWell color];
	[myPathControl setBackgroundColor: newColor];
}

// -------------------------------------------------------------------------------
//	changeSize:sender:
//
//	The user wants to change the frame size of the NSPathControl.
//	First we allocate the controller object for the sheet (if not already created),
//	and invoke the controller's method that opens the sheet.
//
//	The results are returned and if the user did not cancel the sheet, set its
//	bounds, frame and force it to update.
// -------------------------------------------------------------------------------
- (IBAction)changeSize:(id)sender
{
	if (customSizeController == nil)
		customSizeController = [[CustomSizeController alloc] init];
		
	NSRect currentBounds = [myPathControl bounds];
	NSSize currentSize = NSMakeSize(currentBounds.size.width, currentBounds.size.height);
	NSSize newCustomSize = [customSizeController doCustomSize:currentSize withParentWindow:[self window]];
	
	if (![customSizeController wasCancelled])
	{
		[myPathControl setBoundsSize:newCustomSize];
		[myPathControl setFrameSize:newCustomSize];
		[myPathControl setNeedsDisplay:YES];
	}
}

// -------------------------------------------------------------------------------
//	willDisplayOpenPanel:openPanel:
//
//	Delegate method to NSPathControl to determine how the NSOpenPanel will look/behave.
// -------------------------------------------------------------------------------
- (void)pathControl:(NSPathControl*)pathControl willDisplayOpenPanel:(NSOpenPanel*)openPanel
{	
	// change the wind title and choose buttons titles
	[openPanel setAllowsMultipleSelection:NO];
	[openPanel setCanChooseDirectories:YES];
	[openPanel setCanChooseFiles:YES];
	[openPanel setResolvesAliases:YES];
	[openPanel setTitle:@"Choose a file object"];
	[openPanel setPrompt:@"Choose"];
}

// -------------------------------------------------------------------------------
//	pathControlDoubleClick:sender:
//
//  This method is the "double-click" action for the control.
//	Since we are a standard or navigation style we ask for the control's path component.
// -------------------------------------------------------------------------------
- (void)pathControlDoubleClick:(id)sender
{
    if ([myPathControl clickedPathComponentCell] != nil)
		[[NSWorkspace sharedWorkspace] openURL:[[myPathControl clickedPathComponentCell] URL]];
}

// -------------------------------------------------------------------------------
//	menuItemAction:sender:
//
//  This is the action method from our custom menu item: "Reveal in  Finder".
//	Since we are a popup we ask for the control's URL (not one of the path components).
// -------------------------------------------------------------------------------
- (void)menuItemAction:(id)sender
{
	[[NSWorkspace sharedWorkspace] openURL:[myPathControl URL]];
}

// -------------------------------------------------------------------------------
//	willPopUpMenu:menu:
//
//	Delegate method on NSPathControl (as NSPathStylePopUp) that determines what popup menus 
//	will look like.  In our case we add "Reveal in Finder".
// -------------------------------------------------------------------------------
- (void)pathControl:(NSPathControl*)pathControl willPopUpMenu:(NSMenu*)menu
{
	if (!useCustomPath)
	{
		// add the "Reveal in Finder" menu item (but only for file system paths, not our custom path);
		NSMenuItem* newItem = [[NSMenuItem allocWithZone:[NSMenu menuZone]] initWithTitle:@"Reveal in Finder" action:@selector(menuItemAction:) keyEquivalent:@""];
		[newItem setTarget:self];
		
		[menu addItem:[NSMenuItem separatorItem]];
		[menu addItem:newItem];
		[newItem release];
	}
	else
	{
		// remove the "Choose..." and separator menu items since we have a custom path
		[menu removeItemAtIndex:0];
		[menu removeItemAtIndex:0];
	}
}

// -------------------------------------------------------------------------------
//	validateDrop:pinfo
//
//	This method is called when something is dragged over the control.
//	Return NSDragOperationNone to refuse the drop, or anything else to accept it.
// -------------------------------------------------------------------------------
- (NSDragOperation)pathControl:(NSPathControl*)pathControl validateDrop:(id <NSDraggingInfo>)info
{
	return NSDragOperationCopy;
}

// -------------------------------------------------------------------------------
//	acceptDrop:info
//
//	In order to accept the dropped contents previously accepted from
//	validateDrop:, you must implement this method.  We get the new URL from the
//	pasteboardand set it to the path control, and update the explanatory text if needed. 
// -------------------------------------------------------------------------------
-(BOOL)pathControl:(NSPathControl*)pathControl acceptDrop:(id <NSDraggingInfo>)info
{
	BOOL result = NO;
	
	NSURL* url = [NSURL URLFromPasteboard:[info draggingPasteboard]];
	if (url != nil)
	{
		[myPathControl setURL: url];
		[self updateExplainText];	// the user how they can reveal the path component
		result = YES;
	}

	return result;
}

// -------------------------------------------------------------------------------
//	shouldDragPathComponentCell:pathComponentCell:pasteboard
//
//	This method is called when a drag is about to begin.
//	Is shows how to customize dragging by preventing "volumes" from being dragged.
// -------------------------------------------------------------------------------
- (BOOL)pathControl:(NSPathControl*)pathControl shouldDragPathComponentCell:(NSPathComponentCell*)pathComponentCell withPasteboard:(NSPasteboard*)pasteboard
{
	BOOL result = YES;
	
	NSURL* url = [pathComponentCell URL];
	if (url && [url isFileURL])
	{
		NSArray* pathPieces = [[url path] pathComponents];
		if ([pathPieces count] < 4)
			result = NO;	// don't allow dragging volumes
	}
	
	return result;
}


#pragma mark Custom Path Support

// -------------------------------------------------------------------------------
//	componentCellFromType:withIconType:fromTitle:withURL
//
//	This is a utility/factory method (used by useCustomPath) for returning a
//	NSPathComponent cell basedicon, title and URL information.  Each NSPathComponent
//	needs an icon, url and title.
// -------------------------------------------------------------------------------
- (NSPathComponentCell*)componentCellFromType:(OSType)withIconType fromTitle:(NSString*)title withURL:(NSURL*)url 
{
	NSPathComponentCell* componentCell = [[NSPathComponentCell alloc] init];
	
	NSImage* iconImage = [[NSWorkspace sharedWorkspace] iconForFileType:NSFileTypeForHFSTypeCode(withIconType)];
	[componentCell setImage:iconImage];
	[componentCell setURL:url];
	[componentCell setTitle: title];
	
	return componentCell;
}

// -------------------------------------------------------------------------------
//	iconWithType:useCustomPath
//
//	The user wants to create a custom generated path for NSPathControl.
//	Here we make an arbitrary path based on ADC's website pointing to Mac OX - Cocoa.
// -------------------------------------------------------------------------------
- (IBAction)useCustomPath:(id)sender
{
	[setPathButton setEnabled: !useCustomPath];
	
	if (useCustomPath)
	{
		// assemble a set of custom cells we want to display into an array
		// before we set that info to the path control:
		//
		NSMutableArray* pathComponentArray = [NSMutableArray array];
		
		// use utility method to obtain a NSPathComponentCell based on icon, title and URL
		NSPathComponentCell* componentCell = [self componentCellFromType:kAppleLogoIcon
													fromTitle:@"Apple"
													withURL:[NSURL URLWithString:@"http://www.apple.com"]];
		[pathComponentArray addObject: componentCell];
		[componentCell release];
		
		componentCell = [self componentCellFromType:kInternetLocationNewsIcon
								fromTitle:@"Mac OS X"
								withURL:[NSURL URLWithString:@"http://www.apple.com/macosx/"]];
		[pathComponentArray addObject: componentCell];
		[componentCell release];
		
		componentCell = [self componentCellFromType:kGenericURLIcon
								fromTitle:@"Developer"
								withURL:[NSURL URLWithString:@"http://developer.apple.com/macosx/"]];
		[pathComponentArray addObject: componentCell];
		[componentCell release];
		
		componentCell = [self componentCellFromType:kHelpIcon
								fromTitle:@"Cocoa"
								withURL:[NSURL URLWithString:@"http://developer.apple.com/cocoa/"]];
		[pathComponentArray addObject: componentCell];
		[componentCell release];
		
		[myPathControl setPathComponentCells: pathComponentArray];
	}
	else
	{
		// user unchecked the "Custom Path" checkbox: remove the custom path items (if any)
		[myPathControl setPathComponentCells: [[[NSMutableArray alloc] init] autorelease]];
	}
	
	[self updateExplainText];	// update the explanation text to show the user how they can reveal the path component
}

@end
