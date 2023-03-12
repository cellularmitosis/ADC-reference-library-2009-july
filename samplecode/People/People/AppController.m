/*

File: AppController.m

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

#import <SyncServices/SyncServices.h>

#import "AppController.h"
#import "AppControllerExtensions.h"
#import "AppControllerSyncing.h"
#import "Change.h"
#import "Constants.h"

static AppController *g_sharedAppController = nil;

@implementation AppController

//
// ===========================================
// General
//

- (void)awakeFromNib
{
    g_sharedAppController = self;

    m_entityNames = [[NSArray alloc] initWithObjects:EntityContact, nil];
    
    m_appRecords = [[NSMutableArray alloc] init];
    m_changes = [[NSMutableArray alloc] init];
    
    m_syncMode = FastSync;
    
    [self readDataFile];
    [self update];

    if ([m_appRecords count] > 0) {
        [m_table selectRow:0 byExtendingSelection:NO];
        [m_table scrollRowToVisible:0];
    }

    ISyncClient *client = [self performRegistrations];
    [client setSyncAlertHandler:self selector:@selector(client:willSyncEntityNames:)];
}

+ (AppController *)sharedAppController
{
    return g_sharedAppController;
}

- (NSArray *)entityNames
{
    return m_entityNames;
}

//
// ===========================================
// Do and Undo
//

- (void)applyChange:(Change *)change
{
    if (!change)
        return;
    
    int rowToSelect = -1;
    int row = [change row];

    switch ([change type]) {
        case AddRecord:
            if (row >= [m_appRecords count])
                [m_appRecords addObject:[change record]];
            else
                [m_appRecords insertObject:[change record] atIndex:row];
            rowToSelect = row;
            break;
        case DeleteRecord:
            [m_appRecords removeObjectAtIndex:row];
            if ([m_appRecords count] > 0)
                rowToSelect = row - 1 >= 0 ? row - 1 : 0;
            break;
        case ModifyRecord:
            [m_appRecords replaceObjectAtIndex:row withObject:[change record]];
            rowToSelect = row;
            break;
    }

    [m_table reloadData];
    [self update];
    
    if (rowToSelect == -1) {
        [m_table deselectAll:self];
    }
    else {
        [m_table selectRow:rowToSelect byExtendingSelection:NO];
        [m_table scrollRowToVisible:rowToSelect];
    }

    [m_changes addObject:change];
    [[m_window undoManager] registerUndoWithTarget:self selector:@selector(unapplyChange:) object:change];
    [self writeDataFile];
}

- (void)unapplyChange:(Change *)change
{
    if (!change)
        return;
    
    int rowToSelect = -1;
    int row = [change row];

    switch ([change type]) {
        case AddRecord:
            [m_appRecords removeObjectAtIndex:row];
            if ([m_appRecords count] > 0)
                rowToSelect = row - 1 >= 0 ? row - 1 : 0;
            break;
        case DeleteRecord:
            if (row >= [m_appRecords count])
                [m_appRecords addObject:[change oldRecord]];
            else
                [m_appRecords insertObject:[change oldRecord] atIndex:row];
            rowToSelect = row;
            break;
        case ModifyRecord:
            [m_appRecords replaceObjectAtIndex:row withObject:[change oldRecord]];
            rowToSelect = row;
            break;
    }

    [m_table reloadData];
    [self update];
    
    if (rowToSelect == -1) {
        [m_table deselectAll:self];
    }
    else {
        [m_table selectRow:rowToSelect byExtendingSelection:NO];
        [m_table scrollRowToVisible:rowToSelect];
    }

    [m_changes removeLastObject];
    [[m_window undoManager] registerUndoWithTarget:self selector:@selector(applyChange:) object:change];
    [self writeDataFile];
}

//
// ===========================================
// Model and View
//

- (id)tableView:(NSTableView *)tableView 
    objectValueForTableColumn:(NSTableColumn *)tableColumn 
    row:(int)row
{
    id result = nil;
    if (row >= 0 && row < [m_appRecords count]) {
        NSDictionary *record = [m_appRecords objectAtIndex:row];
        result = [record objectForKey:[tableColumn identifier]];
    }
    return result;
}

- (void)tableView:(NSTableView *)tableView
    setObjectValue:(id)object
    forTableColumn:(NSTableColumn *)tableColumn
    row:(int)row
{
    if (row >= 0 && row < [m_appRecords count]) {
        NSDictionary *oldRecord = [m_appRecords objectAtIndex:row];
        NSDictionary *record = nil;
        if ([[tableColumn identifier] isEqual:FirstNameKey]) {
            record = [NSDictionary dictionaryWithObjectsAndKeys:
                object, FirstNameKey,
                [oldRecord objectForKey:MiddleNameKey], MiddleNameKey,
                [oldRecord objectForKey:LastNameKey], LastNameKey, 
                [oldRecord objectForKey:CompanyNameKey], CompanyNameKey,
                [oldRecord objectForKey:IdentifierKey], IdentifierKey, 
                nil];
        }
        else if ([[tableColumn identifier] isEqual:MiddleNameKey]) {
            record = [NSDictionary dictionaryWithObjectsAndKeys:
                object, MiddleNameKey, 
                [oldRecord objectForKey:FirstNameKey], FirstNameKey, 
                [oldRecord objectForKey:LastNameKey], LastNameKey, 
                [oldRecord objectForKey:CompanyNameKey], CompanyNameKey,                
                [oldRecord objectForKey:IdentifierKey], IdentifierKey, 
                nil];
        }
        else if ([[tableColumn identifier] isEqual:LastNameKey]) {
            record = [NSDictionary dictionaryWithObjectsAndKeys:
                object, LastNameKey, 
                [oldRecord objectForKey:FirstNameKey], FirstNameKey, 
                [oldRecord objectForKey:MiddleNameKey], MiddleNameKey,
                [oldRecord objectForKey:CompanyNameKey], CompanyNameKey,                
                [oldRecord objectForKey:IdentifierKey], IdentifierKey, 
                nil];
        }
        else if ([[tableColumn identifier] isEqual:CompanyNameKey]) {
            record = [NSDictionary dictionaryWithObjectsAndKeys:
                object, CompanyNameKey, 
                [oldRecord objectForKey:FirstNameKey], FirstNameKey, 
                [oldRecord objectForKey:MiddleNameKey], MiddleNameKey,
                [oldRecord objectForKey:LastNameKey], LastNameKey,
                [oldRecord objectForKey:IdentifierKey], IdentifierKey, 
                nil];
        }
        [self applyChange:[Change modifyRecordChange:record oldRecord:oldRecord row:row]];
    }
}

- (int)numberOfRowsInTableView:(NSTableView *)tableView
{
    return [m_appRecords count];
}

- (void)update
{
    // m_countLabel
    NSString *countString = nil;
    if ([m_appRecords count] == 1) {
        countString = @"1 Person";
    }
    else {
        countString = [NSString stringWithFormat:@"%d People", [m_appRecords count]];
    }
    [m_countLabel setStringValue:countString];
    
    // m_table
    int row = [m_table selectedRow];
    if (row >= 0) {
        [m_table scrollRowToVisible:row];
    }
}

//
// ===========================================
// IBActions
//

- (IBAction)delete:(id)sender
{
    int row = [m_table selectedRow];
    if (row < 0)
        return;
 
    NSDictionary *record = [m_appRecords objectAtIndex:row];
    [self applyChange:[Change deleteRecordChange:record row:row]];
}

- (IBAction)newPerson:(id)sender
{
    CFUUIDRef uuid = CFUUIDCreate(NULL);
    NSString *identifier = (NSString *)CFUUIDCreateString(NULL, uuid);
    CFRelease(uuid);

    NSDictionary *record = [NSDictionary dictionaryWithObjectsAndKeys:
        @"FirstName", FirstNameKey, 
        @"LastName", LastNameKey,
        @"", MiddleNameKey,
        @"", CompanyNameKey,
        identifier, IdentifierKey, 
        nil];

    [self applyChange:[Change addRecordChange:record row:0]];
    [m_searchField setStringValue:@""];
    [m_window makeFirstResponder:m_table];
}

- (BOOL)validateUserInterfaceItem:(id <NSValidatedUserInterfaceItem>)item
{
    int value = [item tag];
    switch (value) {
        case UsesRecordFiltering:
            [(NSMenuItem *)item setState:m_syncsUsingRecordFiltering ? NSOnState : NSOffState];
            break;
        case UsesRecordFormatting:
            [(NSMenuItem *)item setState:m_syncsUsingRecordFormatting ? NSOnState : NSOffState];
            break;
        case UsesSyncAlertHandler:
            [(NSMenuItem *)item setState:m_syncsUsingSyncAlertHandler ? NSOnState : NSOffState];
            break;
        case SyncsOnAppDeactivate:
            [(NSMenuItem *)item setState:m_syncsOnAppDeactivate ? NSOnState : NSOffState];
            break;
    }
    return YES;
}

//
// ===========================================
// Persistence
//

- (void)readDataFile
{
    NSData *data = [NSData dataWithContentsOfFile:DataFilePath];
    if (!data) {
        m_syncMode = RefreshSync;
        [m_syncModeButton selectItemAtIndex:m_syncMode];
        return;
    }

    NSPropertyListFormat format;
    NSArray *array = [NSPropertyListSerialization propertyListFromData:data 
        mutabilityOption:NSPropertyListImmutable 
        format:&format
        errorDescription:nil];
        
    if (array) {
        [m_appRecords removeAllObjects];
        [m_appRecords addObjectsFromArray:array];
        [self sortNamesAndDisplay];
    }
    else {
        NSLog(@"error reading data file");
    }
}

- (void)writeDataFile
{
    NSData *data = [NSPropertyListSerialization dataFromPropertyList:m_appRecords 
        format:NSPropertyListXMLFormat_v1_0 
        errorDescription:nil];
    if (!data) {
        NSLog(@"error converting data");
        return;    
    }

    BOOL result = [data writeToFile:DataFilePath atomically:YES];
    if (!result) {
        NSLog(@"error writing data file");
    }
}

@end
