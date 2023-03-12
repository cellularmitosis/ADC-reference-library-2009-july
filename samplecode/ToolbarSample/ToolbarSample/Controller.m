/*
File:		Controller.m

Description: 	This is the implementation file for the Controller class, which implements the object used to
		control/initialize this application and as the NSToolbar delegate.

Author:		MCF

Copyright: 	� Copyright 2001 Apple Computer, Inc. All rights reserved.

Disclaimer:	IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc.
                ("Apple") in consideration of your agreement to the following terms, and your
                use, installation, modification or redistribution of this Apple software
                constitutes acceptance of these terms.  If you do not agree with these terms,
                please do not use, install, modify or redistribute this Apple software.

                In consideration of your agreement to abide by the following terms, and subject
                to these terms, Apple grants you a personal, non-exclusive license, under Apple�s
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

05/2001 - MCF - initial version

*/

#import "Controller.h"


// All NSToolbarItems have a unique identifer associated with them, used to tell your delegate/controller what 
// toolbar items to initialize and return at various points.  Typically, for a given identifier, you need to 
// generate a copy of your "master" toolbar item, and return it autoreleased.  The function below takes an
// NSMutableDictionary to hold your master NSToolbarItems and a bunch of NSToolbarItem paramenters,
// and it creates a new NSToolbarItem with those parameters, adding it to the dictionary.  Then the dictionary
// can be used from -toolbar:itemForItemIdentifier:willBeInsertedIntoToolbar: to generate a new copy of the 
// requested NSToolbarItem (when the toolbar wants to redraw, for instance) by simply duplicating and returning
// the NSToolbarItem that has the same identifier in the dictionary.  Plus, it's easy to call this function
// repeatedly to generate lots of NSToolbarItems for your toolbar.
// -------
// label, palettelabel, toolTip, action, and menu can all be NULL, depending upon what you want the item to do
static void addToolbarItem(NSMutableDictionary *theDict,NSString *identifier,NSString *label,NSString *paletteLabel,NSString *toolTip,id target,SEL settingSelector, id itemContent,SEL action, NSMenu * menu)
{
    NSMenuItem *mItem;
    // here we create the NSToolbarItem and setup its attributes in line with the parameters
    NSToolbarItem *item = [[[NSToolbarItem alloc] initWithItemIdentifier:identifier] autorelease];
    [item setLabel:label];
    [item setPaletteLabel:paletteLabel];
    [item setToolTip:toolTip];
    [item setTarget:target];
    // the settingSelector parameter can either be @selector(setView:) or @selector(setImage:).  Pass in the right
    // one depending upon whether your NSToolbarItem will have a custom view or an image, respectively
    // (in the itemContent parameter).  Then this next line will do the right thing automatically.
    [item performSelector:settingSelector withObject:itemContent];
    [item setAction:action];
    // If this NSToolbarItem is supposed to have a menu "form representation" associated with it (for text-only mode),
    // we set it up here.  Actually, you have to hand an NSMenuItem (not a complete NSMenu) to the toolbar item,
    // so we create a dummy NSMenuItem that has our real menu as a submenu.
    if (menu!=NULL)
    {
	// we actually need an NSMenuItem here, so we construct one
	mItem=[[[NSMenuItem alloc] init] autorelease];
	[mItem setSubmenu: menu];
	[mItem setTitle: [menu title]];
	[item setMenuFormRepresentation:mItem];
    }
    // Now that we've setup all the settings for this new toolbar item, we add it to the dictionary.
    // The dictionary retains the toolbar item for us, which is why we could autorelease it when we created
    // it (above).
    [theDict setObject:item forKey:identifier];
}

@implementation Controller

// When we launch, we have to get our NSToolbar set up.  This involves creating a new one, adding the NSToolbarItems,
// and installing the toolbar in our window.
-(void)awakeFromNib
{
    NSFont *theFont;
    NSToolbar *toolbar=[[[NSToolbar alloc] initWithIdentifier:@"myToolbar"] autorelease];
    
    // Here we create the dictionary to hold all of our "master" NSToolbarItems.
    toolbarItems=[[NSMutableDictionary dictionary] retain];
    // Now lets create three NSToolbarItems; 2 using custom views, and a standard one using an image.
    // We call our special processing function to do the initialization and add them to the dictionary.
    addToolbarItem(toolbarItems,@"FontStyle",@"Font Style",@"Font Style",@"Change your font style",self,@selector(setView:),popUpView,NULL,fontStyleMenu);
    addToolbarItem(toolbarItems,@"FontSize",@"Font Size",@"Font Size",@"Grow or shrink the size of your font",self,@selector(setView:),fontSizeView,NULL,fontSizeMenu);
    // often using an image will be your standard case.  You'll notice that a selector is passed
    // for the action (blueText:), which will be called when the image-containing toolbar item is clicked.
    addToolbarItem(toolbarItems,@"BlueLetter",@"Blue Text",@"Blue Text",@"This toggles blue text on/off",self,@selector(setImage:),[NSImage imageNamed:@"blueLetter.tif"],@selector(blueText:),NULL);
     
    // the toolbar wants to know who is going to handle processing of NSToolbarItems for it.  This controller will.
    [toolbar setDelegate:self];
    // If you pass NO here, you turn off the customization palette.  The palette is normally handled automatically
    // for you by NSWindow's -runToolbarCustomizationPalette: method; you'll notice that the "Customize Toolbar"
    // menu item is hooked up to that method in Interface Builder.  Interface Builder currently doesn't automatically 
    // show this action (or the -toggleToolbarShown: action) for First Responder/NSWindow (this is a bug), so you 
    // have to manually add those methods to the First Responder in Interface Builder (by hitting return on the First Responder and 
    // adding the new actions in the usual way) if you want to wire up menus to them.
    [toolbar setAllowsUserCustomization:YES];

    // tell the toolbar that it should save any configuration changes to user defaults.  ie. mode changes, or reordering will persist. 
    // specifically they will be written in the app domain using the toolbar identifier as the key. 
    [toolbar setAutosavesConfiguration: YES]; 
    
    // tell the toolbar to show icons only by default
    [toolbar setDisplayMode: NSToolbarDisplayModeIconOnly];
    // install the toolbar.
    [theWindow setToolbar:toolbar];
    
    // We initialize our font size control here to 12-point font, and set our contentView (an NSTextView) to that size
    [fontSizeStepper setIntValue:12];
    theFont=[contentView font];
    theFont=[[NSFontManager sharedFontManager] convertFont:theFont toSize:[fontSizeStepper intValue]];
    [contentView setFont:theFont];
    
    // This is a state variable that keeps track of whether we're set to plain-text, bold, or italic font
    fontStylePicked=1;
}

// When you have the toolbar in text-only mode, this action is called
// by the toolbar item's menu to make the font bigger.  We just change the stepper control and
// call our -changeFontSize: action.
-(IBAction) fontSizeBigger:(id)sender
{
    [fontSizeStepper setIntValue:[fontSizeStepper intValue]+1];
    [self changeFontSize:NULL];
}

// When you have the toolbar in text-only mode, this action is called
// by the toolbar item's menu to make the font smaller.  We just change the stepper control and
// call our -changeFontSize: action.
-(IBAction) fontSizeSmaller:(id)sender
{
    [fontSizeStepper setIntValue:[fontSizeStepper intValue]-1];
    [self changeFontSize:NULL];
}

// This action is called to change the font size.  It's called by the NSPopUpButton in the toolbar item's 
// custom view, and also by the above routines called from the toolbar item's menu (in text-only mode).
-(IBAction) changeFontSize:(id)sender
{
    NSFont *theFont;
    
    [fontSizeField takeIntValueFrom:fontSizeStepper];
    theFont=[contentView font];
    theFont=[[NSFontManager sharedFontManager] convertFont:theFont toSize:[fontSizeStepper intValue]];
    [contentView setFont:theFont];
}

-(BOOL)validateMenuItem:(NSMenuItem *)menuItem
{
    if ([menuItem action]==@selector(changeFontStyle:))
    {
	// If we got this far, then the menuItem is either a part of the font-style-toobar-item-custom-view's
	// (wow, say that six times fast) NSPopUp Button, or it's a part of the menuFormRepresentation's NSMenu.
	// If it's the former, then the menu item's state (whether there is a check next to it, etc.) is handled
	// for us, but if not, then we have to do it ourselves.  The tags are changed on the menu so that they
	// match the fontStylePicked variable, unlike the popup button's, which don't.
	if ([menuItem tag]==fontStylePicked)
	[menuItem setState:NSOnState];
    }
    return YES;
}

// This action is called from the change font style toolbar item, both from the NSPopUpButton in the custom view,
// and from the menuFormRepresentation menu.
-(IBAction) changeFontStyle:(id)sender
{
    NSFont *theFont;
    int itemIndex;

    // If the sender is an NSMenuItem then this is the menuFormRepresentation.  Otherwise, we're being called from
    // the NSPopUpButton.  We need to check this to find out how to calculate the index of the picked menu item.
    if ([NSStringFromClass([sender class]) isEqual:@"NSMenuItem"])
    {
	itemIndex=[[sender menu] indexOfItem:sender]-1; // for ordinary NSMenus, the title is item #0, so we have to offset things
    }
    else
    {
	itemIndex=[sender indexOfSelectedItem]; // this is an NSPopUpButton, so the first useful item really is #0
    }
    
    [fontSizeField takeIntValueFrom:fontSizeStepper];
    theFont=[contentView font];

    // set the font properties depending upon what was selected
    if (itemIndex==0)
    {
	theFont=[[NSFontManager sharedFontManager] convertFont:theFont toNotHaveTrait:NSItalicFontMask];
        theFont=[[NSFontManager sharedFontManager] convertFont:theFont toNotHaveTrait:NSBoldFontMask];
    }
    else if (itemIndex==1)
    {
	theFont=[[NSFontManager sharedFontManager] convertFont:theFont toNotHaveTrait:NSItalicFontMask];
        theFont=[[NSFontManager sharedFontManager] convertFont:theFont toHaveTrait:NSBoldFontMask];
    }
    else if (itemIndex==2)
    {
	theFont=[[NSFontManager sharedFontManager] convertFont:theFont toNotHaveTrait:NSBoldFontMask];
        theFont=[[NSFontManager sharedFontManager] convertFont:theFont toHaveTrait:NSItalicFontMask];
    }
    // make sure the fontStylePicked variable matches the menu selection plus 1, which also matches
    // the menu item tags in the menuFormRepresentation (see the menu in Interface Builder), so
    // that -validateMenuItem: can do its work. 
    fontStylePicked=itemIndex+1;
    [contentView setFont:theFont];
}

// This is called by the appropriate toolbar item to toggle blue text on/off.
-(IBAction) blueText:(id)sender
{
    if (![[contentView textColor] isEqual:[NSColor blueColor]])
    {
	[contentView setTextColor:[NSColor blueColor]];
    }
    else
    {
	[contentView setTextColor:[NSColor blackColor]];
    }
}

// We don't do anything useful here (and we don't really have to implement this method) but you could
// if you wanted to. If, however, you want to have validation checks on your standard NSToolbarItems
// (with images) and have inactive ones grayed out, then this is the method for you.
// It isn't called for custom NSToolbarItems (with custom views); you'd have to override -validate:
// (see NSToolbarItem.h for a discussion) to get it to do so if you wanted it to.
// If you don't implement this method, the NSToolbarItems are enabled by default.
- (BOOL)validateToolbarItem:(NSToolbarItem *)theItem
{
    // You could check [theItem itemIdentifier] here and take appropriate action if you wanted to
    return YES;
}

// This is an optional delegate method, called when a new item is about to be added to the toolbar.
// This is a good spot to set up initial state information for toolbar items, particularly ones
// that you don't directly control yourself (like with NSToolbarPrintItemIdentifier here).
// The notification's object is the toolbar, and the @"item" key in the userInfo is the toolbar item
// being added.
- (void) toolbarWillAddItem: (NSNotification *) notif
{
    NSToolbarItem *addedItem = [[notif userInfo] objectForKey: @"item"];
    // Is this the printing toolbar item?  If so, then we want to redirect it's action to ourselves
    // so we can handle the printing properly; hence, we give it a new target.
    if ([[addedItem itemIdentifier] isEqual: NSToolbarPrintItemIdentifier])
    {
        [addedItem setToolTip: @"Print your document"];
        [addedItem setTarget: self];
    }
}  

// The NSToolbarPrintItem NSToolbarItem will sent the -printDocument: message to its target.
// Since we wired its target to be ourselves in -toolbarWillAddItem:, we get called here when
// the user tries to print by clicking the toolbar item.
- (void) printDocument:(id) sender
{
    NSPrintOperation *printOperation = [NSPrintOperation printOperationWithView: contentView];
    [printOperation runOperationModalForWindow: [contentView window] delegate: nil didRunSelector: NULL contextInfo:
 NULL];
}


// This method is required of NSToolbar delegates.  It takes an identifier, and returns the matching NSToolbarItem.
// It also takes a parameter telling whether this toolbar item is going into an actual toolbar, or whether it's
// going to be displayed in a customization palette.
- (NSToolbarItem *)toolbar:(NSToolbar *)toolbar itemForItemIdentifier:(NSString *)itemIdentifier willBeInsertedIntoToolbar:(BOOL)flag
{
    // We create and autorelease a new NSToolbarItem, and then go through the process of setting up its
    // attributes from the master toolbar item matching that identifier in our dictionary of items.
    NSToolbarItem *newItem = [[[NSToolbarItem alloc] initWithItemIdentifier:itemIdentifier] autorelease];
    NSToolbarItem *item=[toolbarItems objectForKey:itemIdentifier];
    
    [newItem setLabel:[item label]];
    [newItem setPaletteLabel:[item paletteLabel]];
    if ([item view]!=NULL)
    {
	[newItem setView:[item view]];
    }
    else
    {
	[newItem setImage:[item image]];
    }
    [newItem setToolTip:[item toolTip]];
    [newItem setTarget:[item target]];
    [newItem setAction:[item action]];
    [newItem setMenuFormRepresentation:[item menuFormRepresentation]];
    // If we have a custom view, we *have* to set the min/max size - otherwise, it'll default to 0,0 and the custom
    // view won't show up at all!  This doesn't affect toolbar items with images, however.
    if ([newItem view]!=NULL)
    {
	[newItem setMinSize:[[item view] bounds].size];
	[newItem setMaxSize:[[item view] bounds].size];
    }

    return newItem;
}

// This method is required of NSToolbar delegates.  It returns an array holding identifiers for the default
// set of toolbar items.  It can also be called by the customization palette to display the default toolbar.    
- (NSArray *)toolbarDefaultItemIdentifiers:(NSToolbar*)toolbar
{
    return [NSArray arrayWithObjects:@"FontStyle",@"FontSize",NSToolbarSeparatorItemIdentifier,@"BlueLetter",NSToolbarPrintItemIdentifier,nil];
}

// This method is required of NSToolbar delegates.  It returns an array holding identifiers for all allowed
// toolbar items in this toolbar.  Any not listed here will not be available in the customization palette.
- (NSArray *)toolbarAllowedItemIdentifiers:(NSToolbar*)toolbar
{
    return [NSArray arrayWithObjects:@"FontStyle",@"FontSize",NSToolbarSeparatorItemIdentifier,@"BlueLetter", NSToolbarSpaceItemIdentifier,NSToolbarFlexibleSpaceItemIdentifier,NSToolbarPrintItemIdentifier,nil];
}

// throw away our toolbar items dictionary
- (void) dealloc
{
    [toolbarItems release];
    [super dealloc];
}


@end
