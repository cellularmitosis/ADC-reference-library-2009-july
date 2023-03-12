// ======================================================================================================================
//  MyApplication.h
// ======================================================================================================================


#import <Cocoa/Cocoa.h>


@interface MyApplication : NSApplication
{
	IBOutlet NSPanel			*_findPanel;
	IBOutlet NSTextField		*_findPanelSearchField;
	IBOutlet NSButton			*_ignoreCaseCheckbox;
}

- (int) findOptions;
- (void) findNext: (id) sender;
- (void) findNextAndOrderOutFindPanel: (id) sender;
- (void) findPrevious: (id) sender;

@end
