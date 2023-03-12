#import <Cocoa/Cocoa.h>
#import <Quartz/Quartz.h>

@interface Controller : NSObject {
    IBOutlet NSPanel *propertiesPanel;
    IBOutlet id keywordsField;

/************ BEGIN QUARTZ COMPOSER *************/
	
	IBOutlet QCView *qcView;
	
/************* END QUARTZ COMPOSER **************/
}

/* NSApplication delegate methods */
- (void)application:(NSApplication *)app printFiles:(NSArray *)filenames;
- (void)application:(NSApplication *)app openFiles:(NSArray *)filenames;
- (BOOL)applicationOpenUntitledFile:(NSApplication *)app;
- (NSApplicationTerminateReply)applicationShouldTerminate:(NSApplication *)app;

/* Action methods */
- (void)createNew:(id)sender;
- (void)open:(id)sender;
- (void)saveAll:(id)sender;

@end
