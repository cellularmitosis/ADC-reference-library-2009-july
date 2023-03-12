// ======================================================================================================================
//  MyWindowController.m
// ======================================================================================================================


#import "MyApplication.h"
#import "MyWindowController.h"


#define kPDFViewXDelta			0
#define kPDFViewYDelta			70

#define kURLLink				0
#define kDestinationLink		1


static NSString *ToolbarBackForward					= @"Back Forward";
static NSString *ToolbarPreviousPage				= @"Previous Page";
static NSString *ToolbarNextPage					= @"Next Page";
static NSString *ToolbarPageNumber					= @"Page Number";
static NSString *ToolbarViewMode					= @"View Mode";
static NSString *ToolbarSearch						= @"Search";
static NSString *ToolbarEditTest					= @"EditTest";
static NSString *ToolbarToggleDrawer				= @"ToggleDrawer";


@implementation MyWindowController
// =================================================================================================== MyWindowController
// -------------------------------------------------------------------------------------------------------- windowDidLoad

- (void) windowDidLoad
{
	PDFDocument	*pdfDoc;
	NSRect		visibleScreen;
	NSSize		pageSize;
	float		scaleFactor;
	
	// Create PDFDocument.
	pdfDoc = [[PDFDocument alloc] initWithURL: [NSURL fileURLWithPath: [[self document] fileName]]];
	
	// Set document.
	[_pdfView setDocument: pdfDoc];
	[pdfDoc release];
	
	// Default display mode.
	[_pdfView setAutoScales: YES];
	[_pdfView setDisplaysPageBreaks: NO];

	// Get outline (if any).
	_outline = [[[_pdfView document] outlineRoot] retain];
	if (_outline)
	{
		if ([[_pdfView document] isLocked] == NO)
		{
			[_outlineView reloadData];
			[_outlineView setAutoresizesOutlineColumn: NO];
			
			// Expand items.
			if ([_outlineView numberOfRows] == 1)
				[_outlineView expandItem: [_outlineView itemAtRow: 0] expandChildren: NO];
			[self updateOutlineSelection];
			
			// Always open drawer if there is an outline and unencrypted PDF.
			[[[[self window] drawers] objectAtIndex: 0] open];
		}
	}
	
	// How big to create the window?
	// Visible frame for main screen.
	visibleScreen = [[NSScreen mainScreen] visibleFrame];
	
	// Taking into account the toolbars, etc. in the UI.
	visibleScreen.size.width -= kPDFViewXDelta;
	visibleScreen.size.height -= kPDFViewYDelta;

	// If continuous and multi-page, subtract space for a vertical scrollbar.
	if ((([_pdfView displayMode] & 0x01) == 0x01) && ([[_pdfView document] pageCount] > 1))
		visibleScreen.size.width -= [NSScroller scrollerWidth];
	
	// Page size.
	pageSize = [_pdfView rowSizeForPage: [_pdfView currentPage]];
	
	// Determine limiting scale factor.
	scaleFactor = visibleScreen.size.width / pageSize.width;
	if (visibleScreen.size.height / pageSize.height < scaleFactor)
		scaleFactor = visibleScreen.size.height / pageSize.height;
	
	// Scale bounds.
	pageSize.width = floorf(pageSize.width * scaleFactor);
	pageSize.height = floorf(pageSize.height * scaleFactor);
	
	// Set the window size.
	[[self window] setContentSize: pageSize];
	
	// Close the search results.
	[self setSearchResultsViewHeight: 0];
	
	// Create toolbar.
	[self setupToolbarForWindow: [self window]];
	
	// Internal notification.
	[[NSNotificationCenter defaultCenter] addObserver: self selector: @selector(newActiveAnnotation:) 
			name: @"newActiveAnnotation" object: _pdfView];
	
	// Establish notifications for this document.
	[self setupDocumentNotifications];
	
	// State.
	[self updateLinkTools];
}

// ------------------------------------------------------------------------------------------- setupDocumentNotifications

- (void) setupDocumentNotifications
{
	// Find notifications.
	[[NSNotificationCenter defaultCenter] addObserver: self selector: @selector(startFind:) 
			name: PDFDocumentDidBeginFindNotification object: [_pdfView document]];
	[[NSNotificationCenter defaultCenter] addObserver: self selector: @selector(endFind:) 
			name: PDFDocumentDidEndFindNotification object: [_pdfView document]];
	
	// Document saving progress notifications.
	[[NSNotificationCenter defaultCenter] addObserver: self selector: @selector(documentBeginWrite:) 
			name: @"PDFDidBeginDocumentWrite" object: [_pdfView document]];
	[[NSNotificationCenter defaultCenter] addObserver: self selector: @selector(documentEndWrite:) 
			name: @"PDFDidEndDocumentWrite" object: [_pdfView document]];
	[[NSNotificationCenter defaultCenter] addObserver: self selector: @selector(documentEndPageWrite:) 
			name: @"PDFDidEndPageWrite" object: [_pdfView document]];
	
	// Delegate.
	[[_pdfView document] setDelegate: self];
}

// -------------------------------------------------------------------------------------------------------------- dealloc

- (void) dealloc
{
	// No more notifications.
	[[NSNotificationCenter defaultCenter] removeObserver: self];
	
	// Remove back-forward toolbar item.
	if (_toolbarBackForwardItem)
	{
		[_toolbarBackForwardItem release];
		[_backForwardView release];
	}
	
	// Remove page number toolbar item.
	if (_toolbarPageNumberItem)
	{
		[_toolbarPageNumberItem release];
		[_pageNumberView release];
	}

	// Remove page number toolbar item.
	if (_toolbarViewModeItem)
	{
		[_toolbarViewModeItem release];
		[_viewModeView release];
	}

	// Remove search toolbar item.
	if (_toolbarSearchFieldItem)
	{
		[_toolbarSearchFieldItem release];
		[_searchFieldView release];
	}

	// Remove back-forward toolbar item.
	if (_toolbarEditTestItem)
	{
		[_toolbarEditTestItem release];
		[_editTestView release];
	}
	
	// Search clean-up.
	[_searchResults release];
	[_sampleStrings release];
	
	// Release outline.
	if (_outline)
		[_outline release];
	
	// Call super.
	[super dealloc];
}

// -------------------------------------------------------------------------------------------------------------- pdfView

- (PDFView *) pdfView
{
	return _pdfView;
}

// ------------------------------------------------------------------------------------------- setSearchResultsViewHeight

- (void) setSearchResultsViewHeight: (float) height
{
	NSRect		frameBounds;
	float		wasHeight;
	NSArray		*subViews;
	
	// Get subviews of split view.
	subViews = [_splitView subviews];
	
	// Get current height of search results view (view on top, view zero).
	frameBounds = [[subViews objectAtIndex: 0] frame];
	wasHeight = frameBounds.size.height;
	
	// Set it's frame to reflect new height.
	frameBounds.size.height = height;
	frameBounds.origin.y += wasHeight - height;
	
	// Adjust lower view (PDFView, view 1).
	[[subViews objectAtIndex: 0] setFrame: frameBounds];
	frameBounds = [[subViews objectAtIndex: 1] frame];
	frameBounds.size.height += wasHeight - height;
	[[subViews objectAtIndex: 1] setFrame: frameBounds];
	
	// Do we need to call this?  It doesn't seem to hurt.
	[_splitView adjustSubviews];
}

#pragma mark -------- NSTableView delegate methods
// ---------------------------------------------------------------------------------------------- numberOfRowsInTableView

- (int) numberOfRowsInTableView: (NSTableView *) aTableView
{
	if (aTableView == _searchResultsTable)
		return ([_searchResults count]);
	else
		return 0;
}

// ------------------------------------------------------------------------------ tableView:objectValueForTableColumn:row

- (id) tableView: (NSTableView *) aTableView objectValueForTableColumn: (NSTableColumn *) theColumn row: (int) rowIndex
{
	if (aTableView == _searchResultsTable)
	{
		if ([[theColumn identifier] isEqualToString: @"page"])
			return ([[[[_searchResults objectAtIndex: rowIndex] pages] objectAtIndex: 0] label]);
		else if ([[theColumn identifier] isEqualToString: @"section"])
			return ([[[_pdfView document] outlineItemForSelection: [_searchResults objectAtIndex: rowIndex]] label]);
		else if ([[theColumn identifier] isEqualToString: @"text"])
			return ([_sampleStrings objectAtIndex: rowIndex]);
		else
			return NULL;
	}
	else
	{
		return NULL;
	}
}

// ------------------------------------------------------------------------------------------ tableViewSelectionDidChange

- (void) tableViewSelectionDidChange: (NSNotification *) notification
{
	int			rowIndex;
	
	if ([notification object] == _searchResultsTable)
	{
		// What was selected. Skip out if the row has not changed.
		rowIndex = [(NSTableView *)[notification object] selectedRow];
		if (rowIndex >= 0)
		{
			[_pdfView setCurrentSelection: [_searchResults objectAtIndex: rowIndex]];
			[_pdfView scrollSelectionToVisible: self];
		}
	}
}

#pragma mark -------- NSOutlineView methods
// ----------------------------------------------------------------------------------- outlineView:numberOfChildrenOfItem

- (int) outlineView: (NSOutlineView *) outlineView numberOfChildrenOfItem: (id) item
{
	if (item == NULL)
	{
		if ((_outline) && ([[_pdfView document] isLocked] == NO))
			return [_outline numberOfChildren];
		else
			return 0;
	}
	else
		return [(PDFOutline *)item numberOfChildren];
}

// --------------------------------------------------------------------------------------------- outlineView:child:ofItem

- (id) outlineView: (NSOutlineView *) outlineView child: (int) index ofItem: (id) item
{
	if (item == NULL)
	{
		if ((_outline) && ([[_pdfView document] isLocked] == NO))
			return [[_outline childAtIndex: index] retain];
		else
			return NULL;
	}
	else
		return [[(PDFOutline *)item childAtIndex: index] retain];
}

// ----------------------------------------------------------------------------------------- outlineView:isItemExpandable

- (BOOL) outlineView: (NSOutlineView *) outlineView isItemExpandable: (id) item
{
	if (item == NULL)
	{
		if ((_outline) && ([[_pdfView document] isLocked] == NO))
			return ([_outline numberOfChildren] > 0);
		else
			return NO;
	}
	else
		return ([(PDFOutline *)item numberOfChildren] > 0);
}

// ------------------------------------------------------------------------- outlineView:objectValueForTableColumn:byItem

- (id) outlineView: (NSOutlineView *) outlineView objectValueForTableColumn: (NSTableColumn *) tableColumn 
		byItem: (id) item
{
	return [(PDFOutline *)item label];
}

// ---------------------------------------------------------------------------------------- outlineViewSelectionDidChange

- (void) outlineViewSelectionDidChange: (NSNotification *) notification
{
	// Get the destination associated with the search result list. Tell the PDFView to go there.
	if (([notification object] == _outlineView) && (_ignoreNotification == NO))
		[_pdfView goToDestination: [[_outlineView itemAtRow: [_outlineView selectedRow]] destination]];
}

// --------------------------------------------------------------------------------------------- outlineViewItemDidExpand

- (void) outlineViewItemDidExpand: (NSNotification *) notification
{
	[self updateOutlineSelection];
}

// ------------------------------------------------------------------------------------------- outlineViewItemDidCollapse

- (void) outlineViewItemDidCollapse: (NSNotification *) notification
{
	[self updateOutlineSelection];
}

// ----------------------------------------------------------------------------------------------- updateOutlineSelection

- (void) updateOutlineSelection
{
	PDFOutline	*outlineItem;
	int			pageIndex;
	int			numRows;
	int			i;
	
	// Skip out if this PDF has no outline.
	if (_outline == NULL)
		return;
	
	// Get index of current page.
	pageIndex = [[_pdfView document] indexForPage: [_pdfView currentPage]];
	
	// Test that the current selection is still valid.
	outlineItem = (PDFOutline *)[_outlineView itemAtRow: [_outlineView selectedRow]];
	if ([[_pdfView document] indexForPage: [[outlineItem destination] page]] == pageIndex)
		return;
	
	// Walk outline view looking for best firstpage number match.
	numRows = [_outlineView numberOfRows];
	for (i = 0; i < numRows; i++)
	{
		// Get the destination of the given row....
		outlineItem = (PDFOutline *)[_outlineView itemAtRow: i];
		
		if ([[_pdfView document] indexForPage: [[outlineItem destination] page]] == pageIndex)
		{
			_ignoreNotification = YES;
			[_outlineView selectRow: i byExtendingSelection: NO];
			_ignoreNotification = NO;
			break;
		}
		else if ([[_pdfView document] indexForPage: [[outlineItem destination] page]] > pageIndex)
		{
			_ignoreNotification = YES;
			if (i < 1)				
				[_outlineView selectRow: 0 byExtendingSelection: NO];
			else
				[_outlineView selectRow: i - 1 byExtendingSelection: NO];
			_ignoreNotification = NO;
			break;
		}
	}
}

#pragma mark -------- toolbar methods
// ------------------------------------------------------------------------------------------------ setupToolbarForWindow
// Create a new toolbar instance, and attach it to our document window.

- (void) setupToolbarForWindow: (NSWindow *) window
{
	NSToolbar		*toolbar;
	
	// Allocate it.
	toolbar = [[NSToolbar alloc] initWithIdentifier: @"PDF Linker Toolbar"];
	
	// Set up toolbar properties: Allow customization, give a default display mode, and 
	// remember state in user defaults.
	[toolbar setAllowsUserCustomization: YES];
	[toolbar setAutosavesConfiguration: YES];
	[toolbar setDisplayMode: NSToolbarDisplayModeIconAndLabel];
	
	// We are the delegate
	[toolbar setDelegate: self];
	
	// Attach the toolbar to the document window.
	[window setToolbar: toolbar];
	
	// Done.
	[toolbar release];
}

// -------------------------------------------------------------- toolbar:itemForItemIdentifier:willBeInsertedIntoToolbar
// Required delegate method. Given an item identifier, self method returns an 
// item. The toolbar will use self method to obtain toolbar items that can be 
// displayed in the customization sheet, or in the toolbar itself.

- (NSToolbarItem *) toolbar: (NSToolbar *) toolbar itemForItemIdentifier: (NSString *) itemIdent 
		willBeInsertedIntoToolbar: (BOOL) willBeInserted
{
	NSToolbarItem	*toolbarItem;
	
	// Create a new toolbar item.
	toolbarItem = [[[NSToolbarItem alloc] initWithItemIdentifier: itemIdent] autorelease];
	
	if ([itemIdent isEqualToString: ToolbarBackForward])
	{
		// Set the text label to be displayed in the toolbar, customization palette and tooltip.
		[toolbarItem setLabel: NSLocalizedString(@"Back/Forward", NULL)];
		[toolbarItem setPaletteLabel: NSLocalizedString(@"Back/Forward", NULL)];
		[toolbarItem setToolTip: NSLocalizedString(@"Go Back or Forward", NULL)];
		
		// Set toolbar item view.
		[_backForwardView retain];
		[toolbarItem setView: _backForwardView];
		[toolbarItem setMinSize: [_backForwardView frame].size];
		[toolbarItem setMaxSize: [_backForwardView frame].size];
		
		if (willBeInserted)
		{
			NSMenu		*submenu = NULL;
			NSMenuItem	*submenuItem1 = NULL;
			NSMenuItem	*submenuItem2 = NULL;
			NSMenuItem	*menuFormRep = NULL;
			
			// Create sub menu.
			submenu = [[[NSMenu alloc] init] autorelease];
			
			// Create Back menu item - add to sub menu.
			submenuItem1 = [[[NSMenuItem alloc] initWithTitle: NSLocalizedString(@"Back", NULL) 
					action: @selector(doGoBack:) keyEquivalent: @""] autorelease];
			[submenuItem1 setTarget: self];
			[submenu addItem: submenuItem1];
			
			// Create Forward menu item - add to sub menu.
			submenuItem2 = [[[NSMenuItem alloc] initWithTitle: NSLocalizedString(@"Forward", NULL) 
					action: @selector(doGoForward:) keyEquivalent: @""] autorelease];
			[submenuItem2 setTarget: self];
			[submenu addItem: submenuItem2];
			
			// Create menu form representation - set it.
			menuFormRep = [[[NSMenuItem alloc] init] autorelease];
			[menuFormRep setTitle: [toolbarItem label]];
			[menuFormRep setSubmenu: submenu];
			[toolbarItem setMenuFormRepresentation: menuFormRep];
		}
	}
	else if ([itemIdent isEqualToString: ToolbarPreviousPage])
	{
		// Set the text label to be displayed in the toolbar, customization palette and tooltip.
		[toolbarItem setLabel: NSLocalizedString(@"Previous", NULL)];
		[toolbarItem setPaletteLabel: NSLocalizedString(@"Previous", NULL)];
		[toolbarItem setToolTip: NSLocalizedString(@"Go To Previous Page", NULL)];
		
		// Set image.
		[toolbarItem setImage: [NSImage imageNamed: @"ToolbarPreviousPageImage"]];
		
		// Tell the item what message to send when it is clicked.
		[toolbarItem setTarget: self];
		[toolbarItem setAction: @selector(doGoToPreviousPage:)];
	}
	else if ([itemIdent isEqualToString: ToolbarNextPage])
	{
		// Set the text label to be displayed in the toolbar and customization palette 
		[toolbarItem setLabel: NSLocalizedString(@"Next", NULL)];
		[toolbarItem setPaletteLabel: NSLocalizedString(@"Next", NULL)];
		
		// Set up a reasonable tooltip, and image.
		[toolbarItem setToolTip: NSLocalizedString(@"Go To Next Page", NULL)];
		[toolbarItem setImage: [NSImage imageNamed: @"ToolbarNextPageImage"]];
		
		// Tell the item what message to send when it is clicked.
		[toolbarItem setTarget: self];
		[toolbarItem setAction: @selector(doGoToNextPage:)];
	}
	else if ([itemIdent isEqual: ToolbarPageNumber])
	{
		// Set up the standard properties .
		[toolbarItem setLabel: NSLocalizedString(@"Page", NULL)];
		[toolbarItem setPaletteLabel: NSLocalizedString(@"Page", NULL)];
		[toolbarItem setToolTip: NSLocalizedString(@"Go To Page", NULL)];
		
		// Set toolbar item view.
		[_pageNumberView retain];
		[toolbarItem setView: _pageNumberView];
		[toolbarItem setMinSize: NSMakeSize(50, NSHeight([_pageNumberView frame]))];
		[toolbarItem setMaxSize: NSMakeSize(56, NSHeight([_pageNumberView frame]))];
		
		if (willBeInserted)
		{
			NSMenu		*submenu = NULL;
			NSMenuItem	*submenuItem = NULL;
			NSMenuItem	*menuFormRep = NULL;
			
			// Create sub menu.
			submenu = [[[NSMenu alloc] init] autorelease];
			
			// Create Page Dialog item.
			submenuItem = [[[NSMenuItem alloc] initWithTitle: NSLocalizedString(@"Go To Page Panel", NULL) 
					action: @selector(doGoToPagePanel:) keyEquivalent: @""] autorelease];
			[submenuItem setTarget: self];
			[submenu addItem: submenuItem];
			
			// Create menu form representation - set it.		
			menuFormRep = [[[NSMenuItem alloc] init] autorelease];
			[menuFormRep setTitle: [toolbarItem label]];
			[menuFormRep setSubmenu: submenu];
			[toolbarItem setMenuFormRepresentation: menuFormRep];
		}
		else
		{
			toolbarItem = [[toolbarItem copy] autorelease];
			[(NSTextField *)[toolbarItem view] setStringValue: @"--"];
		}
	}
	else if ([itemIdent isEqualToString: ToolbarViewMode])
	{
		// Set the text label to be displayed in the toolbar, customization palette and tooltip.
		[toolbarItem setLabel: NSLocalizedString(@"View Mode", NULL)];
		[toolbarItem setPaletteLabel: NSLocalizedString(@"View Mode", NULL)];
		[toolbarItem setToolTip: NSLocalizedString(@"Change the Viewing Mode", NULL)];
		
		// Set toolbar item view.
		[_viewModeView retain];
		[toolbarItem setView: _viewModeView];
		[toolbarItem setMinSize: [_viewModeView frame].size];
		[toolbarItem setMaxSize: [_viewModeView frame].size];
	}
	else if ([itemIdent isEqual: ToolbarSearch])
	{
		// Set up the standard properties .
		[toolbarItem setLabel: NSLocalizedString(@"Search", NULL)];
		[toolbarItem setPaletteLabel: NSLocalizedString(@"Search", NULL)];
		[toolbarItem setToolTip: NSLocalizedString(@"Search document", NULL)];
		
		// Set toolbar item view.
		[_searchFieldView retain];
		[toolbarItem setView: _searchFieldView];
		[toolbarItem setMinSize: NSMakeSize(128, NSHeight([_searchFieldView frame]))];
		[toolbarItem setMaxSize: NSMakeSize(256, NSHeight([_searchFieldView frame]))];
		
		if (willBeInserted)
		{
			NSMenu		*submenu = NULL;
			NSMenuItem	*submenuItem = NULL;
			NSMenuItem	*menuFormRep = NULL;
			
			// Create sub menu.
			submenu = [[[NSMenu alloc] init] autorelease];
			
			// Create Search panel item.
			submenuItem = [[[NSMenuItem alloc] initWithTitle: NSLocalizedString(@"Search Panel", NULL) 
					action: @selector(doSearch:) keyEquivalent: @""] autorelease];
			[submenuItem setTarget: self];
			[submenu addItem: submenuItem];
			
			// Create menu form representation - set it.		
			menuFormRep = [[[NSMenuItem alloc] init] autorelease];
			[menuFormRep setTitle: [toolbarItem label]];
			[menuFormRep setSubmenu: submenu];
			[toolbarItem setMenuFormRepresentation: menuFormRep];
		}
	}
	else if ([itemIdent isEqualToString: ToolbarEditTest])
	{
		// Set the text label to be displayed in the toolbar and customization palette 
		[toolbarItem setLabel: NSLocalizedString(@"Link Mode", NULL)];
		[toolbarItem setPaletteLabel: NSLocalizedString(@"Link Mode", NULL)];
		[toolbarItem setToolTip: NSLocalizedString(@"Switch between Edit and Test modes", NULL)];
		
		// Use a custom view (a menu).
		[_editTestView retain];
		[toolbarItem setView: _editTestView];
		[toolbarItem setMinSize: [[toolbarItem view] frame].size];
		[toolbarItem setMaxSize: [[toolbarItem view] frame].size];
		
		if (willBeInserted)
		{
			NSMenu		*submenu = NULL;
			NSMenuItem	*submenuItem1 = NULL;
			NSMenuItem	*submenuItem2 = NULL;
			NSMenuItem	*menuFormRep = NULL;
			
			// Tell the item what message to send when it is clicked.
			[toolbarItem setTarget: self];
			
			// Create sub menu.
			submenu = [[[NSMenu alloc] init] autorelease];
			
			// Create Media Box menu item - add to sub menu.
			submenuItem1 = [[[NSMenuItem alloc] initWithTitle: NSLocalizedString(@"Edit Links", NULL) 
					action: @selector(doEditMode:) keyEquivalent: @""] autorelease];
			[submenuItem1 setTarget: self];
			[submenu addItem: submenuItem1];
			
			// Create Crop Box menu item - add to sub menu.
			submenuItem2 = [[[NSMenuItem alloc] initWithTitle: NSLocalizedString(@"Test Links", NULL) 
					action: @selector(doTestMode:) keyEquivalent: @""] autorelease];
			[submenuItem2 setTarget: self];
			[submenu addItem: submenuItem2];
			
			// Create menu form representation - set it.
			menuFormRep = [[[NSMenuItem alloc] init] autorelease];
			[menuFormRep setTitle: [toolbarItem label]];
			[menuFormRep setSubmenu: submenu];
			[toolbarItem setMenuFormRepresentation: menuFormRep];
		}
		else
		{
			toolbarItem = [[toolbarItem copy] autorelease];
		}
	}
	else if ([itemIdent isEqualToString: ToolbarToggleDrawer])
	{
		// Set the text label to be displayed in the toolbar and customization palette 
		[toolbarItem setLabel: NSLocalizedString(@"Outline", NULL)];
		[toolbarItem setPaletteLabel: NSLocalizedString(@"Outline", NULL)];
		
		// Set up a reasonable tooltip, and image.
		[toolbarItem setToolTip: NSLocalizedString(@"Show or Hide Outline", NULL)];
		[toolbarItem setImage: [NSImage imageNamed: @"ToolbarDrawerImage"]];
		
		// Tell the item what message to send when it is clicked.
		[toolbarItem setTarget: self];
		[toolbarItem setAction: @selector(toggleDrawer:)];
	}
	else
	{
		// Not identified, not supported. 
		toolbarItem = NULL;
	}
	
	return toolbarItem;
}

// ---------------------------------------------------------------------------------------- toolbarDefaultItemIdentifiers
// Required delegate method. Returns the ordered list of items to be shown in 
// the toolbar by default. If during the toolbar's initialization, no overriding 
// values are found in the user defaults, or if the user chooses to revert to 
// the default items self set will be used.

- (NSArray *) toolbarDefaultItemIdentifiers: (NSToolbar *) toolbar
{
	return [NSArray arrayWithObjects: 
			ToolbarPreviousPage, ToolbarNextPage, ToolbarBackForward, ToolbarPageNumber, 
			NSToolbarSeparatorItemIdentifier, 
			ToolbarEditTest, 
			NSToolbarFlexibleSpaceItemIdentifier, 
			ToolbarSearch, ToolbarToggleDrawer, 
			NULL];
}

// ---------------------------------------------------------------------------------------- toolbarAllowedItemIdentifiers
// Required delegate method. Returns the list of all allowed items by identifier.
// By default, the toolbar does not assume any items are allowed, even the 
// separator. So, every allowed item must be explicitly listed. The set of 
// allowed items is used to construct the customization palette.

- (NSArray *) toolbarAllowedItemIdentifiers: (NSToolbar *) toolbar
{
	return [NSArray arrayWithObjects:  
			ToolbarPreviousPage, ToolbarNextPage, ToolbarBackForward, ToolbarPageNumber, ToolbarViewMode, 
			ToolbarEditTest, ToolbarSearch, ToolbarToggleDrawer, 
			NSToolbarSeparatorItemIdentifier, NSToolbarSpaceItemIdentifier, NSToolbarFlexibleSpaceItemIdentifier, 
			NSToolbarCustomizeToolbarItemIdentifier, NSToolbarPrintItemIdentifier, 
			NULL];
}

// --------------------------------------------------------------------------------------------------- toolbarWillAddItem
// Optional delegate method. Before an new item is added to the toolbar, self 
// notification is posted self is the best place to notice a new item is going 
// into the toolbar. For instance, if you need to cache a reference to the 
// toolbar item or need to set up some initial state, self is the best place 
// to do it. The notification object is the toolbar to which the item is being 
// added. The item being added is found by referencing the @"item" key in the userInfo.

- (void) toolbarWillAddItem: (NSNotification *) theNotification
{
	NSToolbarItem	*addedItem;
	
	// Toolbar item added.
	addedItem = [[theNotification userInfo] objectForKey: @"item"];
	
	// See if it is one we're interested in.
	if ([[addedItem itemIdentifier] isEqualToString: ToolbarBackForward])
	{
		_toolbarBackForwardItem = [addedItem retain];
		
		// Listen for these.
		[[NSNotificationCenter defaultCenter] addObserver: self selector: @selector(updateBackForwardState:) 
				name: PDFViewChangedHistoryNotification object: _pdfView];
		
		// Update.
		[self updateBackForwardState: NULL];
	}
	else if ([[addedItem itemIdentifier] isEqualToString: ToolbarPageNumber])
	{
		_toolbarPageNumberItem = [addedItem retain];
		
		// Listen for these.
		[[NSNotificationCenter defaultCenter] addObserver: self selector: @selector(updatePageNumberField:) 
				name: PDFViewPageChangedNotification object: _pdfView];
		
		// Update.
		[self updatePageNumberField: NULL];
	}
	else if ([[addedItem itemIdentifier] isEqualToString: ToolbarViewMode])
	{
		_toolbarViewModeItem = [addedItem retain];
		
		// Update.
		[self updateViewMode: NULL];
	}
	else if ([[addedItem itemIdentifier] isEqualToString: ToolbarSearch])
	{
		_toolbarSearchFieldItem = [addedItem retain];
	}
	else if ([[addedItem itemIdentifier] isEqualToString: ToolbarEditTest])
	{
		_toolbarEditTestItem = [addedItem retain];
	}
}

// ------------------------------------------------------------------------------------------------- toolbarDidRemoveItem

- (void) toolbarDidRemoveItem: (NSNotification *) theNotification
{
	NSToolbarItem	*removedItem;
	
	// Which item is going away?
	removedItem = [[theNotification userInfo] objectForKey: @"item"];
	
	if ([[removedItem itemIdentifier] isEqualToString: ToolbarBackForward])
	{
		// No longer listen.
		[[NSNotificationCenter defaultCenter] removeObserver: self name: PDFViewChangedHistoryNotification 
				object: _pdfView];
		
		// Release.
		if (_toolbarBackForwardItem)
			[_toolbarBackForwardItem release];
		_toolbarBackForwardItem = NULL;
	}
	else if ([[removedItem itemIdentifier] isEqualToString: ToolbarPageNumber])
	{
		// No longer listen.
		[[NSNotificationCenter defaultCenter] removeObserver: self name: PDFViewPageChangedNotification 
				object: _pdfView];
		
		// Release.
		if (_toolbarPageNumberItem)
			[_toolbarPageNumberItem release];
		_toolbarPageNumberItem = NULL;
	}
	else if ([[removedItem itemIdentifier] isEqualToString: ToolbarViewMode])
	{
		// Release.
		if (_toolbarViewModeItem)
			[_toolbarViewModeItem release];
		_toolbarViewModeItem = NULL;
	}
	else if ([[removedItem itemIdentifier] isEqualToString: ToolbarSearch])
	{
		// Release.
		if (_toolbarSearchFieldItem)
			[_toolbarSearchFieldItem release];
		_toolbarSearchFieldItem = NULL;
	}
	else if ([[removedItem itemIdentifier] isEqualToString: ToolbarBackForward])
	{
		// Release.
		if (_toolbarEditTestItem)
			[_toolbarEditTestItem release];
		_toolbarEditTestItem = NULL;
	}
}

// -------------------------------------------------------------------------------------------------- validateToolbarItem
// Optional method. Self message is sent to us since we are the target of some 
// toolbar item actions (for example: of the save items action) 

- (BOOL) validateToolbarItem: (NSToolbarItem *) toolbarItem
{
	BOOL		enable = YES;
	
	if ([toolbarItem action] == @selector(doGoToPreviousPage:))
	{
		enable = [_pdfView canGoToPreviousPage];
	}
	else if ([toolbarItem action] == @selector(doGoToNextPage:))
	{
		enable = [_pdfView canGoToNextPage];
	}
	else if ([toolbarItem action] == @selector(doGoToPage:))
	{
		// Enabled if the current document is multipage.
		enable = ([[_pdfView document] pageCount] > 1);
	}
	else if ([toolbarItem action] == @selector(toggleDrawer:))
	{
		// Enabled if we have an outline for this PDF.
		enable = (_outline != NULL);
	}
	
	return enable;
}

// --------------------------------------------------------------------------------------------------------- saveDocument

- (void) saveDocument: (id) sender
{
	[self writePDFToURL: NULL];
}

// ------------------------------------------------------------------------------------------------------- saveDocumentAs

- (void) saveDocumentAs: (id) sender
{
	NSSavePanel	*savePanel;
	
	// Create save panel, require PDF.
	savePanel = [NSSavePanel savePanel];
	[savePanel setRequiredFileType: @"pdf"];
	
	// Run save panel.
	if ([savePanel runModal] == NSFileHandlingPanelOKButton)
		[self writePDFToURL: [savePanel URL]];
}

// -------------------------------------------------------------------------------------------------------- writePDFToURL

- (void) writePDFToURL: (NSURL *) URL
{
	// Currently in PDFKit, when a PDF is saved, the resulting PDF loses its outline.
	if (_outline)
		[_outline release];
	_outline = NULL;
	
	if (URL == NULL)
	{
		[[_pdfView document] writeToURL: [[_pdfView document] documentURL]];
	}
	else
	{
		[[_pdfView document] writeToURL: URL];
		[_pdfView setDocument: [[[PDFDocument alloc] initWithURL: URL] autorelease]];
		[self setupDocumentNotifications];
	}
	
	// Clear edited flag.
	[[self window] setDocumentEdited: NO];
}

// -------------------------------------------------------------------------------------------------------- printDocument

- (void) printDocument: (id) sender
{
	// Pass down to PDF view.
	[_pdfView printDocument: sender];
}

#pragma mark --------  actions
// ------------------------------------------------------------------------------------------------------ doGoBackForward

- (void) doGoBackForward: (id) sender
{
	// Segment with tag eqial to zero is the Back, otherwise Forward.
	if ([[sender cell] tagForSegment: [sender selectedSegment]] == 0)
		[_pdfView goBack: sender];
	else
		[_pdfView goForward: sender];
}

// ------------------------------------------------------------------------------------------------------------- doGoBack

- (void) doGoBack: (id) sender
{
	[_pdfView goBack: sender];
}

// ---------------------------------------------------------------------------------------------------------- doGoForward

- (void) doGoForward: (id) sender
{
	[_pdfView goForward: sender];
}

// ---------------------------------------------------------------------------------------------- refreshBackForwardState

- (void) updateBackForwardState: (NSNotification *) notification
{
	// Update segemented control state.
	[_backForwardView setEnabled: [_pdfView canGoBack] forSegment: 0];
	[_backForwardView setEnabled: [_pdfView canGoForward] forSegment: 1];
}

// ------------------------------------------------------------------------------------------------------- doGoToNextPage

- (void) doGoToNextPage: (id) sender
{
	[_pdfView goToNextPage: sender];
}

// --------------------------------------------------------------------------------------------------- doGoToPreviousPage

- (void) doGoToPreviousPage: (id) sender
{
	[_pdfView goToPreviousPage: sender];
}

// ----------------------------------------------------------------------------------------------------------- doGoToPage

- (void) doGoToPage: (id) sender
{
	int			newPage;
	
	// Make sure page number entered is valid.
	newPage = [self getPageIndexFromLabel: [sender stringValue]];
	if ((newPage < 1) || (newPage > [[_pdfView document] pageCount]))
	{
		// Error.
		[self updatePageNumberField: NULL];
		NSBeep();
	}
	else
	{
		// Go to that page.
		[_pdfView goToPage: [[_pdfView document] pageAtIndex: newPage - 1]];
	}
}

// ------------------------------------------------------------------------------------------------------ doGoToPagePanel

- (void) doGoToPagePanel: (id) sender
{
	// Specify page range.
	[_pageNumberPanelRange setStringValue: [NSString stringWithFormat:  
			NSLocalizedString(@"Go to page (%@ to %@):", NULL), [[[_pdfView document] pageAtIndex: 0] label], 
			[[[_pdfView document] pageAtIndex: [[_pdfView document] pageCount] - 1] label]]];
	
	// Populate initially with current page label.
	[_pageNumberPanelText setStringValue: [[_pdfView currentPage] label]];
	
	// Bring up the page number panel as a sheet.
	[NSApp beginSheet: _pageNumberPanel modalForWindow: [self window] modalDelegate: self 
			didEndSelector: @selector(goToPagePanelDidEnd: returnCode: contextInfo:) contextInfo: NULL];
}

// -------------------------------------------------------------------------------------------------- goToPagePanelDidEnd

- (void) goToPagePanelDidEnd: (NSWindow *) sheet returnCode: (int) returnCode contextInfo: (void *) contextInfo
{
	// Close.
	[_pageNumberPanel close];
	
	// Make sure page number entered is valid.
	if ((returnCode < 1) || (returnCode > [[_pdfView document] pageCount]))
	{
		// Zero may indicate user canceled, don't beep in that case.
		if (returnCode != 0)
			NSBeep();
		
		return;
	}
	
	// Go to that page.
	[_pdfView goToPage: [[_pdfView document] pageAtIndex: returnCode - 1]];
}

// ------------------------------------------------------------------------------------------------ goToPageNumberEntered

- (void) goToPageNumberEntered: (id) sender
{
	[NSApp endSheet: _pageNumberPanel returnCode: [self getPageIndexFromLabel: [_pageNumberPanelText stringValue]]];
}

// ----------------------------------------------------------------------------------------------- goToPageNumberCanceled

- (void) goToPageNumberCanceled: (id) sender
{
	// Return zero (an invalid page number since are UI is 1-based) indicateing user canceled.
	[NSApp endSheet: _pageNumberPanel returnCode: 0];
}

// ------------------------------------------------------------------------------------------------ updatePageNumberField

- (void) updatePageNumberField: (NSNotification *) notification
{
	// Update label displayed in "go-to-page text field".
	[_pageNumberView setStringValue: [[_pdfView currentPage] label]];
	
	// Make sure current outline item is selected.
	[self updateOutlineSelection];
}

// ----------------------------------------------------------------------------------------------------------- doViewMode

- (void) doViewMode: (id) sender
{
	switch ([[sender cell] tagForSegment: [sender selectedSegment]])
	{
		case 0:
		[_pdfView setDisplayMode: kPDFDisplaySinglePageContinuous];
		break;
		
		case 1:
		[_pdfView setDisplayMode: kPDFDisplaySinglePage];		
		break;
		
		case 2:
		[_pdfView setDisplayMode: kPDFDisplayTwoUp];		
		break;
	}
}

// ------------------------------------------------------------------------------------------------------- updateViewMode

- (void) updateViewMode: (NSNotification *) notification
{
	switch ([_pdfView displayMode])
	{
		case kPDFDisplaySinglePageContinuous:
		[_viewModeView setSelectedSegment: 0];
		break;
		
		case kPDFDisplaySinglePage:
		[_viewModeView setSelectedSegment: 1];
		break;
		
		case kPDFDisplayTwoUp:
		[_viewModeView setSelectedSegment: 2];
		break;
	}
}

// --------------------------------------------------------------------------------------------------- doFindAllInstances

- (void) doFindAllInstances: (id) sender
{
	[_searchFieldView selectText: self];
}

// ------------------------------------------------------------------------------------------------------------- doSearch

- (void) doSearch: (id) sender
{
	NSString	*searchString;
	
	// Cancel find if in progress.
	if ([[_pdfView document] isFinding])
		[[_pdfView document] cancelFindString];
	
	// User cancelled (empty string sent).
	if ([[sender stringValue] length] == 0)
		return;
	
	// Lazily allocate _searchResults.
	if (_searchResults == NULL)
		_searchResults = [[NSMutableArray alloc] initWithCapacity: 10];
	
	// Lazily allocate _sampleStrings.
	if (_sampleStrings == NULL)
		_sampleStrings = [[NSMutableArray alloc] initWithCapacity: 10];
	
	// Open search results if required.
	if ([[[_splitView subviews] objectAtIndex: 0] frame].size.height == 0.0)
		[self setSearchResultsViewHeight: 80.0];
	
	// Normalize search string using Unicode Normalization Form KD.
	searchString = [[sender stringValue] decomposedStringWithCompatibilityMapping];
	
	// Do the search (will search forward, non-literal, and case-insensitive).
	[[_pdfView document] beginFindString: searchString withOptions: NSCaseInsensitiveSearch];
}

// ------------------------------------------------------------------------------------------------------------ startFind

- (void) startFind: (NSNotification *) notification
{
	// Empty arrays.
	[_searchResults removeAllObjects];
	[_sampleStrings removeAllObjects];
	
	// Clear search results table.
	[_searchResultsTable reloadData];
	
	// Note start time.
	_searchTime = [[NSDate alloc] initWithTimeIntervalSinceNow: 0.0];
}

// ------------------------------------------------------------------------------------------------------- didMatchString
// Called when an instance was located. Delegates can instantiate.

- (void) didMatchString: (PDFSelection *) instance
{
	PDFSelection	*instanceCopy;
	NSDate			*newTime;
	unsigned		count;
	
	// Add page label to our array.
	instanceCopy = [instance copy];
	[_searchResults addObject: instanceCopy];
	count = [_searchResults count];
	
	// Get a string containing a contextual sample of the string searched for and add to array.
	[_sampleStrings addObject: [self getContextualStringFromSelection: instance]];
	
	// How much time since we were last called (updating the table view too frequently can be slow for performance).
	newTime = [NSDate date];
	if (([newTime timeIntervalSinceDate: _searchTime] > 1.0) || (count == 1))
	{
		// Force a reload.
		[_searchResultsTable reloadData];
		
		[_searchTime release];
		_searchTime = [newTime retain];
		
		// Handle found first search result.
		if (count == 1)
		{
			// Select first item (search result) in table view.
			[_searchResultsTable selectRowIndexes: [NSIndexSet indexSetWithIndex: 0] byExtendingSelection: NO];
		}
	}
}

// ------------------------------------------------------------------------------------- getContextualStringFromSelection

- (NSAttributedString *) getContextualStringFromSelection: (PDFSelection *) instance
{
	NSMutableAttributedString	*attributedSample;
	NSString					*searchString;
	NSMutableString				*sample;
	NSString					*rawSample;
	unsigned int				count;
	unsigned int				i;
	unichar						ellipse = 0x2026;
	NSRange						searchRange;
	NSRange						foundRange;
	NSMutableParagraphStyle		*paragraphStyle = NULL;
	
	// Get search string.
	searchString = [instance string];
	
	// Extend selection.
	[instance extendSelectionAtStart: 32];
	[instance extendSelectionAtEnd: 128];
	
	// Get string from sample.
	rawSample = [instance string];
	count = [rawSample length];
	
	// String to hold non-<CR> characters from rawSample.
	sample = [NSMutableString stringWithCapacity: count + 32 + 128];
	[sample setString: [NSString stringWithCharacters: &ellipse length: 1]];
	
	// Keep all characters except <LF>.
	for (i = 0; i < count; i++)
	{
		unichar		oneChar;
		
		oneChar = [rawSample characterAtIndex: i];
		if (oneChar == 0x000A)
			[sample appendString: @" "];
		else
			[sample appendString: [NSString stringWithCharacters: &oneChar length: 1]];
	}
	
	// Follow with elipses.
	[sample appendString: [NSString stringWithCharacters: &ellipse length: 1]];
	
	// Finally, create attributed string.
 	attributedSample = [[NSMutableAttributedString alloc] initWithString: sample];
	
	// Find instances of search string and "bold" them.
	searchRange.location = 0;
	searchRange.length = [sample length];
	do
	{
		// Search for the string.
		foundRange = [sample rangeOfString: searchString options: NSCaseInsensitiveSearch range: searchRange];
		
		// Did we find it?
		if (foundRange.location != NSNotFound)
		{
			// Bold the text range where the search term was found.
			[attributedSample setAttributes: [NSDictionary dictionaryWithObjectsAndKeys: [NSFont boldSystemFontOfSize: 
					[NSFont systemFontSize]], NSFontAttributeName, NULL] range: foundRange];
			
			// Advance the search range.
			searchRange.location = foundRange.location + foundRange.length;
			searchRange.length = [sample length] - searchRange.location;
		}
	}
	while (foundRange.location != NSNotFound);
	
	// Create paragraph style that indicates truncation style.
	paragraphStyle = [[NSParagraphStyle defaultParagraphStyle] mutableCopy];
	[paragraphStyle setLineBreakMode: NSLineBreakByTruncatingTail];
	
	// Add paragraph style.
    [attributedSample addAttributes: [[NSMutableDictionary alloc] initWithObjectsAndKeys: 
			paragraphStyle, NSParagraphStyleAttributeName, NULL] range: NSMakeRange(0, [attributedSample length])];
	
	// Clean.
	[paragraphStyle release];
	
	return attributedSample;
}

// -------------------------------------------------------------------------------------------------------------- endFind

- (void) endFind: (NSNotification *) notification
{
	// Force a reload.
	[_searchTime release];
	[_searchResultsTable reloadData];		
}

// ----------------------------------------------------------------------------------------------------------- doEditMode

- (void) doEditMode: (id) sender
{
	_linkMode = kLinkEditMode;
	[self updateLinkModeState];
	[self updateLinkTools];
	[_pdfView setNeedsDisplay: YES];
}

// ----------------------------------------------------------------------------------------------------------- doTestMode

- (void) doTestMode: (id) sender
{
	_linkMode = kLinkTestMode;
	
	[_pdfView setActiveAnnotation: NULL];
	
	[self updateLinkModeState];
	[self updateLinkTools];
	[_pdfView setNeedsDisplay: YES];
}

// -------------------------------------------------------------------------------------------------- updateLinkModeState

- (void) updateLinkModeState
{
	if (_toolbarEditTestItem)
		[(NSPopUpButton *)[_toolbarEditTestItem view] selectItemAtIndex: [self linkMode]];
}

// ------------------------------------------------------------------------------------------------------------- linkMode

- (int) linkMode
{
	return _linkMode;
}

// --------------------------------------------------------------------------------------------------------- toggleDrawer

- (void) toggleDrawer: (id) sender
{
	[_drawer toggle: sender];
}

// ----------------------------------------------------------------------------------------------------- validateMenuItem

- (BOOL) validateMenuItem: (NSMenuItem *) menuItem
{
	BOOL		enable = YES;
	
	if ([menuItem action] == @selector(doFindAllInstances:))
	{
		enable = (_toolbarSearchFieldItem != NULL);
	}
	else if ([menuItem action] == @selector(doUseSelectionForFind:))
	{
		enable = [_pdfView currentSelection] != NULL;
	}
	else if ([menuItem action] == @selector(doGoToNextPage:))
	{
		enable = [_pdfView canGoToNextPage];
	}
	else if ([menuItem action] == @selector(doGoToPreviousPage:))
	{
		enable = [_pdfView canGoToPreviousPage];
	}
	else if ([menuItem action] == @selector(doGoBack:))
	{
		enable = [_pdfView canGoBack];
	}
	else if ([menuItem action] == @selector(doGoForward:))
	{
		enable = [_pdfView canGoForward];
	}
	else if ([menuItem action] == @selector(doEditMode:))
	{
		[menuItem setState: [self linkMode] == kLinkEditMode];
	}
	else if ([menuItem action] == @selector(doTestMode:))
	{
		[menuItem setState: [self linkMode] == kLinkTestMode];
	}
	else if ([menuItem action] == @selector(doNewLink:))
	{
		enable = ([self linkMode] == kLinkEditMode);
	}
	
	return enable;
}

#pragma mark -------- link methods
// -------------------------------------------------------------------------------------------------- newActiveAnnotation

- (void) newActiveAnnotation: (NSNotification *) notification
{
	PDFAnnotationLink	*activeLink;
	
	if (_edited)
	{
		PDFAnnotationLink	*wasActiveLink;
		
		wasActiveLink = [[notification userInfo] objectForKey: @"wasActiveAnnotation"];
		if (wasActiveLink != NULL)
		{
			if ([_linkMatrix selectedRow] == kURLLink)
				[wasActiveLink setDestination: NULL];
			else
				[wasActiveLink setURL: NULL];
		}
	}
	
	_edited = NO;
	
	activeLink = [_pdfView activeAnnotation];
	if (activeLink)
	{
		if ([activeLink destination] != NULL)
		{
			PDFDestination	*destination;
			NSPoint			point;
			
			// Link has a destination associated with it.
			[_linkMatrix selectCellAtRow: kDestinationLink column: 0];
			[_linkTabView selectTabViewItemAtIndex: kDestinationLink];
			
			destination = [activeLink destination];
			point = [destination point];
			[_linkDestinationText setStringValue: [NSString stringWithFormat:  
					NSLocalizedString(@"Page %@; (%.1f, %.1f)", NULL), 
					[[destination page] label], point.x, point.y]];
		}
		else if ([activeLink URL] != NULL)
		{
			// Link has a URL associated with it.
			[_linkMatrix selectCellAtRow: kURLLink column: 0];
			[_linkTabView selectTabViewItemAtIndex: kURLLink];
			[_linkURLText setStringValue: [[activeLink URL] absoluteString]];
		}
		else
		{
			// Unsupported link action.
			[_linkMatrix selectCellAtRow: 1 column: 0];
			[_linkTabView selectTabViewItemAtIndex: 1];
			[_linkDestinationText setStringValue: NSLocalizedString(@"Unsupported link.", NULL)];
		}
	}
	
	[self updateLinkTools];
}

// ------------------------------------------------------------------------------------------------------ updateLinkTools

- (void) updateLinkTools
{
	PDFAnnotationLink	*activeLink;
	
	activeLink = [_pdfView activeAnnotation];
	
	if (([self linkMode] == kLinkEditMode) && (activeLink != NULL))
	{
		// Enable all controls.
		[_linkMatrix setEnabled: YES];
		[_setDestinationButton setEnabled: YES];
		[_linkURLText setEnabled: YES];
		[_linkDestinationText setEnabled: YES];
	}
	else
	{
		// Disable all controls - no selected link.
		[_linkMatrix setEnabled: NO];
		[_setDestinationButton setEnabled: NO];
		[_linkURLText setEnabled: NO];
		[_linkURLText setStringValue: @""];
		[_linkDestinationText setEnabled: NO];
		[_linkDestinationText setStringValue: @""];
	}
}

// ---------------------------------------------------------------------------------------------------- linkTypeMatrixHit

- (void) linkTypeMatrixHit: (id) sender
{
	PDFAnnotationLink	*activeLink;
	
	activeLink = [_pdfView activeAnnotation];
	
	if ([sender selectedRow] == kURLLink)
	{
		if ([activeLink URL] == NULL)
			[_linkURLText setStringValue: @""];
		else
			[_linkURLText setStringValue: [[activeLink URL] absoluteString]];
		
		[_linkTabView selectTabViewItemAtIndex: kURLLink];
		[[self window] makeFirstResponder: _linkURLText];
	}
	else
	{
		if ([activeLink destination] == NULL)
		{
			[_linkDestinationText setStringValue: @""];
		}
		else
		{
			PDFDestination	*destination;
			NSPoint			point;
			
			[_linkTabView selectTabViewItemAtIndex: kDestinationLink];
			
			destination = [activeLink destination];
			point = [destination point];
			[_linkDestinationText setStringValue: [NSString stringWithFormat:  
					NSLocalizedString(@"Page %@; (%.1f, %.1f)", NULL), 
					[[destination page] label], point.x, point.y]];
		}
	}
	
	[self updateLinkTools];
}

// ------------------------------------------------------------------------------------------------------- linkURLEntered

- (void) linkURLEntered: (id) sender
{
	PDFAnnotationLink	*activeLink;
	
	// We ought to have an active link.
	activeLink = [_pdfView activeAnnotation];
	if (activeLink == NULL)
		return;
	
	// Set it.
	[activeLink setURL: [NSURL URLWithString: [sender stringValue]]];
	
	// Reflect new URL.
	if ([activeLink URL] == NULL)
		[_linkURLText setStringValue: @""];
	else
		[_linkURLText setStringValue: [[activeLink URL] absoluteString]];
	
	// Set edited flag.
	[[self window] setDocumentEdited: YES];
}

// ------------------------------------------------------------------------------------------------ linkSetDestinationHit

- (void) linkSetDestinationHit: (id) sender
{
	PDFAnnotationLink	*activeLink;
	PDFPage				*topLeftPage;
	PDFDestination		*destination;
	NSRect				displayBox;
	NSPoint				topLeftPoint;
	
	// We ought to have an active link.
	activeLink = [_pdfView activeAnnotation];
	if (activeLink == NULL)
		return;
	
	// What page is at the top-left of the PDFView?
	topLeftPoint = NSMakePoint(0.0, [_pdfView frame].size.height);
	topLeftPage = [_pdfView pageForPoint: topLeftPoint nearest: YES];
	
	// Get top-left point in page-space (re-using topLeftPoint) and clip to display box.
	topLeftPoint = [_pdfView convertPoint: topLeftPoint toPage: topLeftPage];
	displayBox = [topLeftPage boundsForBox: [_pdfView displayBox]];
	if (topLeftPoint.x < displayBox.origin.x)
		topLeftPoint.x = displayBox.origin.x;
	if (topLeftPoint.x > NSMaxX(displayBox))
		topLeftPoint.x = NSMaxX(displayBox);
	if (topLeftPoint.y < displayBox.origin.y)
		topLeftPoint.y = displayBox.origin.y;
	if (topLeftPoint.y > NSMaxY(displayBox))
		topLeftPoint.y = NSMaxY(displayBox);
	
	// Create a PDFDestination.
	destination = [[PDFDestination alloc] initWithPage: topLeftPage atPoint: topLeftPoint];
	
	// Set it.
	[activeLink setDestination: destination];
	
	// Reflect new destination.
	if ([activeLink destination] == NULL)
	{
		[_linkDestinationText setStringValue: @""];
	}
	else
	{
		PDFDestination	*actualDestination;
		NSPoint			point;
		
		[_linkTabView selectTabViewItemAtIndex: kDestinationLink];
		
		actualDestination = [activeLink destination];
		point = [actualDestination point];
		[_linkDestinationText setStringValue: [NSString stringWithFormat:  
				NSLocalizedString(@"Page %@; (%.1f, %.1f)", NULL), 
				[[actualDestination page] label], point.x, point.y]];
	}
	
	// Set edited flag.
	[[self window] setDocumentEdited: YES];
}

// ------------------------------------------------------------------------------------------------------------ doNewLink

- (void) doNewLink: (id) sender
{
	PDFAnnotationLink	*newLink;
	PDFPage				*page;
	PDFSelection		*selection;
	NSRect				bounds;
	
	// Determine bounds to use for new link annotation.
	selection = [_pdfView currentSelection];
	if (selection != NULL)
	{
		// Get bounds (page space) for selection (first page in case selection spans multiple pages).
		page = [[selection pages] objectAtIndex: 0];
		bounds = [selection boundsForPage: page];
	}
	else
	{
		NSRect		viewFrame;
		NSPoint		center;
		NSSize		defaultSize;
		
		// Get center of the PDFView.
		viewFrame = [_pdfView frame];
		center = NSMakePoint(NSMidX(viewFrame), NSMidY(viewFrame));
		
		// Convert to "page space".
		page = [_pdfView pageForPoint: center nearest: YES];
		center = [_pdfView convertPoint: center toPage: page];
		
		// Create a 
		defaultSize = [_pdfView defaultNewLinkSize];
		bounds = NSMakeRect(center.x - (defaultSize.width / 2.0), center.y - (defaultSize.height / 2.0), 
				defaultSize.width, defaultSize.height);
	}
	
	// Create annotation and add to page.
	newLink = [[PDFAnnotationLink alloc] initWithBounds: bounds];
	[page addAnnotation: newLink];
	
	[_pdfView setActiveAnnotation: newLink];
	
	// Set edited flag.
	[[self window] setDocumentEdited: YES];
}

#pragma mark -------- save progress
// --------------------------------------------------------------------------------------------------- documentBeginWrite

- (void) documentBeginWrite: (NSNotification *) notification
{
	// Establish maximum and current value for progress bar.
	[_saveProgressBar setMaxValue: (double)[[_pdfView document] pageCount]];
	[_saveProgressBar setDoubleValue: 0.0];
	
	// Bring up the save panel as a sheet.
	[NSApp beginSheet: _saveWindow modalForWindow: [self window] modalDelegate: self 
			didEndSelector: @selector(saveProgressSheetDidEnd: returnCode: contextInfo:) contextInfo: NULL];
}

// ----------------------------------------------------------------------------------------------------- documentEndWrite

- (void) documentEndWrite: (NSNotification *) notification
{
	[NSApp endSheet: _saveWindow];
}

// ------------------------------------------------------------------------------------------------- documentEndPageWrite

- (void) documentEndPageWrite: (NSNotification *) notification
{
	[_saveProgressBar setDoubleValue: [[[notification userInfo] objectForKey: @"PDFDocumentPageIndex"] floatValue]];
	[_saveProgressBar displayIfNeeded];
}

// ---------------------------------------------------------------------------------------------- saveProgressSheetDidEnd

- (void) saveProgressSheetDidEnd: (NSWindow *) sheet returnCode: (int) returnCode contextInfo: (void *) contextInfo
{
	[_saveWindow close];
}

#pragma mark -------- utility methods
// ------------------------------------------------------------------------------------------------ getPageIndexFromLabel
// Given a page label (might be "1" or "2" or might be "i" or "iv") try to return the index of the
// page that has that label (or -1 if none). In this exceptional case, pages are 1-based.

- (int) getPageIndexFromLabel: (NSString *) label
{
	int			index = -1;
	int			count;
	int			i;
	
	// Handle empty string.
	if ([label length] < 1)
		goto bail;
	
	// Walk through all pages and compare the page label against 'label'.
	count = [[_pdfView document] pageCount];
	for (i = 0; i < count; i++)
	{
		if ([[[[_pdfView document] pageAtIndex: i] label] isEqualToString: label])
		{
			// Got it.
			index = i + 1;
			break;
		}
	}
	
bail:
	
	return index;
}

@end
