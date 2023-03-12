#import <Cocoa/Cocoa.h>

#import "MainOpenGLView.h"

@interface AppController : NSObject
{
    IBOutlet MainOpenGLView 	*mainGLView;
}

- (void)UpdateDrawing;
@end
