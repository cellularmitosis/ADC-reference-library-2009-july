/*
To display the about box add this method to your Controller:

- (IBAction)showAboutBox:(id)sender
{
    [[AboutBox sharedInstance] showPanel:sender];
}

Also hook the About menu item to an action named "showAboutBox" in your Controller
by control-dragging from the menu item to the controller.  Before the Controller's action
will show up as an option for the menu item, you will have to unhook it from it's current
routine.
*/

#import <Cocoa/Cocoa.h>

@interface AboutBox : NSObject
{
    IBOutlet id appNameField;
    IBOutlet id copyrightField;
    IBOutlet id notesField;
    IBOutlet id versionField;
}

+ (AboutBox *)sharedInstance;
- (IBAction)showPanel:(id)sender;
- (void)hiliteAndActivateURLs:(NSTextView*)textView;

@end
