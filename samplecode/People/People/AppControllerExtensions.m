/*

File: AppControllerExtensions.m

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

#import "AppControllerExtensions.h"
#import "Constants.h"

@implementation AppController (Extensions)

- (BOOL)matchName:(NSString *)name withRecord:(NSDictionary *)record
{
    BOOL rc = NO;
    NSString *firstName = nil;
    NSString *lastName = nil;
    NSRange range = [name rangeOfString:@" "];
    if (range.location != NSNotFound) {
        firstName = [name substringToIndex:range.location];
        lastName = [name substringFromIndex:range.location + 1];
    }
    if (lastName) {
        rc = (([[record objectForKey:FirstNameKey] caseInsensitiveCompare:firstName] == NSOrderedSame) &&
              ([[record objectForKey:LastNameKey] caseInsensitiveCompare:lastName] == NSOrderedSame));
        if (rc == YES) return YES;
    }
    return (([[[record objectForKey:FirstNameKey] lowercaseString] rangeOfString:name].length > 0) ||
            ([[[record objectForKey:MiddleNameKey] lowercaseString] rangeOfString:name].length > 0) ||
            ([[[record objectForKey:LastNameKey] lowercaseString] rangeOfString:name].length > 0) ||
            ([[[record objectForKey:CompanyNameKey] lowercaseString] rangeOfString:name].length > 0));
}

- (void)_find:(NSString *)string
{
    NSString *name = [string lowercaseString];
    
    if ((name == nil) || ([name isEqualToString:@""])) return;
    
    int selectedRow = [m_table selectedRow];
    int index = -1;
    int ii, count;
    for (ii = selectedRow + 1, count = [m_appRecords count]; ii < count; ii++) {
        NSDictionary *record = [m_appRecords objectAtIndex:ii];
        if ([self matchName:name withRecord:record]) {
            index = ii;
            break;
        }
    }
    if (index == -1) {
        for (ii = 0; ii <= selectedRow; ii++) {
            NSDictionary *record = [m_appRecords objectAtIndex:ii];
            if ([self matchName:name withRecord:record]) {
                index = ii;
                break;
            }
        }
    }
    if (index != -1) {
        [m_table selectRow:index byExtendingSelection:NO];
        [m_table scrollRowToVisible:index];
    }
}

- (IBAction)find:(id)sender
{
    [self _find:[sender stringValue]];
}

static int sortFunction (id ldict, id rdict, void *context) {
    NSComparisonResult rc;
    NSString *leftName = [ldict objectForKey:LastNameKey];
    NSString *rightName = [rdict objectForKey:LastNameKey];
    
    if ((leftName != nil) && (rightName != nil)) {
        rc = [leftName caseInsensitiveCompare:rightName];
    } else if (leftName == nil) {
        rc = rightName ? NSOrderedAscending : NSOrderedSame;
    } else {
        rc = NSOrderedDescending;
    }
    if (rc == NSOrderedSame) {
        leftName = [ldict objectForKey:FirstNameKey];
        rightName = [rdict objectForKey:FirstNameKey];
        if ((leftName != nil) && (rightName != nil)) {
            rc = [leftName caseInsensitiveCompare:rightName];
        } else if (leftName == nil) {
            rc = rightName ? NSOrderedAscending : NSOrderedSame;
        } else {
            rc = NSOrderedDescending;
        }
    }
    if (rc == NSOrderedSame) {
        leftName = [ldict objectForKey:CompanyNameKey];
        rightName = [rdict objectForKey:CompanyNameKey];
        rc = [leftName caseInsensitiveCompare:rightName];
    }
    return rc;
}

- (void)sortNamesAndDisplay
{
    int index = [m_table selectedRow];
    NSDictionary *selectedRecord = nil;
    
    if (index != -1) selectedRecord = [m_appRecords objectAtIndex:index];
    [m_appRecords sortUsingFunction:sortFunction context:nil];
    
    [m_table reloadData];
    if (selectedRecord) {
        [self _find:[NSString stringWithFormat:@"%@ %@", [selectedRecord objectForKey:FirstNameKey], [selectedRecord objectForKey:LastNameKey]]];
    }
}

@end
