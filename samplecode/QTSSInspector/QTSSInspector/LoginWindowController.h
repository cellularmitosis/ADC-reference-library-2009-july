/* LoginWindowController */

#import <Cocoa/Cocoa.h>

@interface LoginWindowController : NSObject
{
    IBOutlet NSTextField *hostField;
    IBOutlet NSTextField *passwordField;
    IBOutlet NSTextField *usernameField;
    IBOutlet NSWindow *myWindow;
    IBOutlet NSPanel *myProgressPanel;
    IBOutlet NSTextField *myProgressField;
    id myInspectorController;
    id myOutlineView;
}

+ (BOOL)checkLoginForString:(NSString *)theString;
- (void)openProgressPanelForString:(NSString *)progressString;
- (IBAction)login:(id)sender;

@end
