/*

File: TableView.m

Abstract: Part of the People project demonstrating use of the
              SyncServices framework

Version: 0.1

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

Copyright © 2005 Apple Computer, Inc., All Rights Reserved

*/ 

#import "AppController.h"
#import "NSEventExtras.h"
#import "TableView.h"

@implementation TableView

- (void)keyDown:(NSEvent *)event
{
    if ([event isDeleteKeyEvent]) {
        [[AppController sharedAppController] delete:nil];
        return;
    }
    if ([event isReturnOrEnterKeyEvent]) {
        BOOL edit = YES;
        if ([[self delegate] respondsToSelector:@selector(tableView:shouldEditTableColumn:row:)]) {
            NSTableColumn *column = [[self tableColumns] objectAtIndex:0];
            edit = [[self delegate] tableView:self shouldEditTableColumn:column row:[self selectedRow]];
        }
        if (edit) {
            [self editColumn:0 row:[self selectedRow] withEvent:event select:YES];
            return;
        }
    }
    
    [super keyDown:event];
}

- (BOOL)shouldEditNextColumnOnReturnKeyInRow:(int)row tableColumn:(NSTableColumn *)tableColumn
{
    return NO;
}

- (void)textDidEndEditing:(NSNotification *)notification
{
    // If we want to override the default behavior of editing the next column
    // when the return key is hit, we trick our superclass into thinking that
    // the editing was ended by something other than the return key.
    if ([self shouldEditNextColumnOnReturnKeyInRow:[self editedRow]
                                       tableColumn:[[self tableColumns] objectAtIndex:[self editedColumn]]]
        || [[[notification userInfo] objectForKey:@"NSTextMovement"] intValue] != NSReturnTextMovement) {
        [super textDidEndEditing:notification];
        
        // Don't muck with first responder in this case; just let the system do its thang. In many
        // cases the first responder will still be a text field, and we don't want to break that.
        return;
    }

    NSMutableDictionary *modifiedUserInfo = [NSMutableDictionary dictionaryWithDictionary:
        [notification userInfo]];
    
    // Note that NSIllegalTextMovement (0), is what occurs in cases like
    // clicking in the whitespace past all rows in the table view. It is
    // poorly named.
    [modifiedUserInfo setObject:[NSNumber numberWithInt:NSIllegalTextMovement]
                         forKey:@"NSTextMovement"];

    notification = [NSNotification notificationWithName:[notification name]
                                                 object:[notification object]
                                               userInfo:modifiedUserInfo];

    [super textDidEndEditing:notification];
    
    // Inherited behavior in this case leaves window as first responder (bug 2949629)
    [[self window] makeFirstResponder:self];
}

@end