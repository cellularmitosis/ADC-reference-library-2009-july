/*

File: IBFragment.m

Abstract: The fragment class.

Disclaimer: IMPORTANT:  This Apple software is supplied to you by Apple
Computer, Inc. ("Apple") in consideration of your agreement to the
following terms, and your use, installation, modification or
redistribution of this Apple software constitutes acceptance of these
terms.  If you do not agree with these terms, please do not use,
install, modify or redistribute this Apple software.

In consideration of your agreement to abide by the following terms, and
subject to these terms, Apple grants you a personal, non-exclusive
license, under Apple's copyrights in this original Apple software (the
"Apple Software"), to use, reproduce, modify and redistribute the Apple
Software, with or without modifications, in source and/or binary forms;
provided that if you redistribute the Apple Software in its entirety and
without modifications, you must retain this notice and the following
text and disclaimers in all such redistributions of the Apple Software. 
Neither the name, trademarks, service marks or logos of Apple Computer,
Inc. may be used to endorse or promote products derived from the Apple
Software without specific prior written permission from Apple.  Except
as expressly stated in this notice, no other rights or licenses, express
or implied, are granted by Apple herein, including but not limited to
any patent rights that may be infringed by your derivative works or by
other works in which the Apple Software may be incorporated.

The Apple Software is provided by Apple on an "AS IS" basis.  APPLE
MAKES NO WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION
THE IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS
FOR A PARTICULAR PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND
OPERATION ALONE OR IN COMBINATION WITH YOUR PRODUCTS.

IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL
OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION,
MODIFICATION AND/OR DISTRIBUTION OF THE APPLE SOFTWARE, HOWEVER CAUSED
AND WHETHER UNDER THEORY OF CONTRACT, TORT (INCLUDING NEGLIGENCE),
STRICT LIABILITY OR OTHERWISE, EVEN IF APPLE HAS BEEN ADVISED OF THE
POSSIBILITY OF SUCH DAMAGE.

Copyright © 2006 Apple Computer, Inc., All Rights Reserved

*/

#import "IBFragment.h"
#import "IBFragmentView.h"

static const CGFloat IBFragmentHorizontalPad = 8.0f;
static const CGFloat IBFragmentVerticalPad = 4.0f;


@implementation IBFragment
- (id)init {
    if ((self = [super init])) {
        title = @"Title";
        font = [[NSFont systemFontOfSize:[NSFont systemFontSizeForControlSize:NSRegularControlSize]] retain];
    }
    return self;
}

- (id)initWithCoder:(NSCoder *)coder {
    if ((self = [super init])) {
        font = [[coder decodeObjectForKey:@"font"] retain];
        title = [[coder decodeObjectForKey:@"title"] retain];
        fragmentView = [coder decodeObjectForKey:@"fragmentView"];
    }
    return self;
}

- (void)encodeWithCoder:(NSCoder *)coder {
    [coder encodeObject:[self title] forKey:@"title"];
    [coder encodeObject:[self font] forKey:@"font"];
    [coder encodeConditionalObject:[self fragmentView] forKey:@"fragmentView"];
}

- (void)dealloc {
    [title release];
    [font release];
    [super dealloc];
}

- (void)setFragmentView:(IBFragmentView *)view {
    fragmentView = view;
}

- (IBFragmentView *)fragmentView {
    return fragmentView;
}

- (NSString *)title {
    return title;
}

- (void)setTitle:(NSString *)newTitle {
    if (title != newTitle) {
        [title release];
        title = [newTitle copy];
        [fragmentView setNeedsDisplay:YES];
    }
}

- (NSFont *)font {
    return font;
}

- (void)setFont:(NSFont *)newFont {
    if (font != newFont) {
        [font release];
        font = [newFont retain];
    }
}

- (NSDictionary *)attributes {
    return [NSDictionary dictionaryWithObjectsAndKeys:[NSColor whiteColor], NSForegroundColorAttributeName, [self font], NSFontAttributeName, nil];
}

- (NSAttributedString *)attributedTitle {
    NSMutableAttributedString *attributedTitle = [[[NSMutableAttributedString alloc] initWithString:[self title] attributes:[self attributes]] autorelease];
    NSMutableParagraphStyle *style = [[NSMutableParagraphStyle alloc] init];
    [style setLineBreakMode:NSLineBreakByTruncatingTail];
    [style setAlignment:NSCenterTextAlignment];
    [attributedTitle addAttribute:NSParagraphStyleAttributeName value:style range:NSMakeRange(0, [attributedTitle length])];
    [style release];
    return attributedTitle;
}

- (NSSize)idealSize {
    NSSize titleSize = [[self attributedTitle] size];
    titleSize.width += IBFragmentHorizontalPad;
    titleSize.height += IBFragmentVerticalPad;
    return titleSize;
}

- (CGFloat)idealWidth {
    return [self idealSize].width;
}

- (NSRect)titleRectForRect:(NSRect)rect {
    NSAttributedString *attributedTitle = [self attributedTitle];
    NSSize titleSize = [attributedTitle size];
    NSRect titleRect = NSInsetRect(rect, IBFragmentHorizontalPad / 2, IBFragmentVerticalPad / 2);
    titleRect.size.height = MIN(titleSize.height, rect.size.height);
    titleRect.origin.y = rect.size.height / 2 - titleSize.height / 2;
    return titleRect;
}

- (void)drawInRect:(NSRect)rect {
    NSRect titleRect = [self titleRectForRect:rect];
    NSAttributedString *attributedTitle = [self attributedTitle];
    [attributedTitle drawInRect:titleRect];
}
@end
