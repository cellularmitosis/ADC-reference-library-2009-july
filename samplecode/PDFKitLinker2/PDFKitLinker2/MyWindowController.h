// ======================================================================================================================
//  MyWindowController.h
// ======================================================================================================================


#import <Cocoa/Cocoa.h>
#import <Quartz/Quartz.h>
#import "MyPDFView.h"


#define kLinkEditMode				0
#define kLinkTestMode				1


@interface MyWindowController : NSWindowController
{
	IBOutlet NSSplitView			*_splitView;
	IBOutlet MyPDFView				*_pdfView;
	IBOutlet NSTableView			*_searchResultsTable;
	IBOutlet NSDrawer				*_drawer;
	
	IBOutlet NSSegmentedControl		*_backForwardView;			// Toolbar: Back-Forward.
	NSToolbarItem					*_toolbarBackForwardItem;	//    "
	IBOutlet NSTextField			*_pageNumberView;			// Toolbar: Page number.
	NSToolbarItem					*_toolbarPageNumberItem;	//    "
	IBOutlet id						_pageNumberPanel;			//    "
	IBOutlet NSTextField			*_pageNumberPanelText;		//    "
	IBOutlet NSTextField			*_pageNumberPanelRange;		//    "
	IBOutlet NSSegmentedControl		*_viewModeView;				// Toolbar: View Mode.
	NSToolbarItem					*_toolbarViewModeItem;		//    "
	IBOutlet NSSearchField			*_searchFieldView;			// Toolbar: Search Field.
	NSToolbarItem					*_toolbarSearchFieldItem;	//    "
	IBOutlet NSPopUpButton			*_editTestView;				// Toolbar: Edit-Test Modes.
	NSToolbarItem					*_toolbarEditTestItem;		//    "
	NSMutableArray					*_searchResults;			// Searching
	NSMutableArray					*_sampleStrings;			// Searching
	NSDate							*_searchTime;
	
	PDFOutline						*_outline;					// Outline
	IBOutlet NSOutlineView			*_outlineView;				//    "
	BOOL							_ignoreNotification;		//    "
	
	IBOutlet NSMatrix				*_linkMatrix;				// Link tools
	IBOutlet NSTabView				*_linkTabView;
	IBOutlet NSButton				*_setDestinationButton;
	IBOutlet NSTextField			*_linkURLText;
	IBOutlet NSTextField			*_linkDestinationText;
	int								_linkMode;
	BOOL							_edited;
	PDFAnnotationLink				*_activeLink;
	
	IBOutlet NSProgressIndicator	*_saveProgressBar;			// Saving.
	IBOutlet NSPanel				*_saveWindow;
}

- (void) setupDocumentNotifications;
- (PDFView *) pdfView;
- (void) setSearchResultsViewHeight: (float) height;

- (void) updateOutlineSelection;

- (void) setupToolbarForWindow: (NSWindow *) window;

- (void) writePDFToURL: (NSURL *) URL;

- (void) doGoBackForward: (id) sender;
- (void) updateBackForwardState: (NSNotification *) notification;
- (void) doGoToNextPage: (id) sender;
- (void) doGoToPreviousPage: (id) sender;
- (void) doGoToPage: (id) sender;
- (void) goToPageNumberEntered: (id) sender;
- (void) goToPageNumberCanceled: (id) sender;
- (void) updatePageNumberField: (NSNotification *) notification;

- (void) doViewMode: (id) sender;
- (void) updateViewMode: (NSNotification *) notification;

- (void) doSearch: (id) sender;
- (NSAttributedString *) getContextualStringFromSelection: (PDFSelection *) instance;

- (void) doEditMode: (id) sender;
- (void) doTestMode: (id) sender;
- (void) updateLinkModeState;
- (int) linkMode;

// -------- link methods
- (void) newActiveAnnotation: (NSNotification *) notification;
- (void) updateLinkTools;
- (void) linkTypeMatrixHit: (id) sender;
- (void) linkURLEntered: (id) sender;
- (void) linkSetDestinationHit: (id) sender;

// -------- utility methods
- (int) getPageIndexFromLabel: (NSString *) label;
@end
