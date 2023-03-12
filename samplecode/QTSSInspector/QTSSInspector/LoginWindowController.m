#import "LoginWindowController.h"
#import "AdminProtocolAccessObj.h"
#import "QTSSInspectorController.h"

@implementation LoginWindowController

+ (BOOL)checkLoginForString:(NSString *)theString
{
    NSRange range = [theString rangeOfString:@"qtssusername"];
    
    if ((range.location < 0) || (range.location > [theString length]))
        return YES;
    else
        return NO;
}

- (void)awakeFromNib
{    
    [myWindow center];
}

- (void)openProgressPanelForString:(NSString *)progressString
{
    [myProgressField setStringValue:progressString];
    [[NSApplication sharedApplication] beginSheet:myProgressPanel
                                   modalForWindow:myWindow
                                    modalDelegate:self
                                   didEndSelector:nil
                                      contextInfo:nil];
    
}

- (IBAction)login:(id)sender
{
    NSString *hostname = [hostField stringValue];
    NSString *username = [usernameField stringValue];
    NSString *password = [passwordField stringValue];
    id authServerObj;

    [self openProgressPanelForString:@"Logging in…"];

    authServerObj = [[AdminProtocolAccessObj alloc] initWithUsername:username password:password host:hostname];

    [myProgressPanel close];
    [[NSApplication sharedApplication] endSheet:myProgressPanel];

    if (authServerObj) {
        [myInspectorController setMyAdminProtocolObj:authServerObj];
        [myWindow close];
        [myInspectorController showWindow:self];
        [myOutlineView reloadData];
    }
    else
        NSBeginAlertSheet(@"Invalid Password",
                          @"OK",
                          nil, nil,
                          myWindow,
                          nil, nil, nil, nil,
                          @"The username or password you entered is not recognized. Please try again.");
}

@end
