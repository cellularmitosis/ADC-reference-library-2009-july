/* QTSSInspectorController */

#import <Cocoa/Cocoa.h>

@interface QTSSInspectorController : NSWindowController
{
    IBOutlet NSOutlineView *myOutlineView;
    NSMutableDictionary *myDataCache;
    NSMutableArray *myStringCache;
    id myAdminProtocolObj;
    IBOutlet NSPanel *myProgressPanel;
    IBOutlet NSTextField *myProgressField;
}

- (NSDictionary *)contentsOfPath:(NSString *)path;
- (NSString *)keepString:(NSString *)theString;
- (id)myAdminProtocolObj;
- (void)setMyAdminProtocolObj:(id)obj;
- (IBAction)refresh:(id)sender;

@end
