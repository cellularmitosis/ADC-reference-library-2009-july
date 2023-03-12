/*
        Controller.h
        TextViewDelegate

        Author: DD
*/

#import <AppKit/AppKit.h>

@interface Controller : NSObject {
    unsigned committedLength;
}

- (BOOL)textView:(NSTextView *)textView shouldChangeTextInRange:(NSRange)affectedCharRange replacementString:(NSString *)replacementString;
- (BOOL)textView:(NSTextView *)textView doCommandBySelector:(SEL)commandSelector;

@end
