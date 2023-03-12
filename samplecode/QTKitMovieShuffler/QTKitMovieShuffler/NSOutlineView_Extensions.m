/*

File: NSOutlineView_Extensions.m

Abstract: Our custom extensions to NSOutlineView to handle row selection

Version: <1.0>

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


#import "NSOutlineView_Extensions.h"

@implementation NSOutlineView (MyExtensions)

// returns the first selected item
- (id)selectedItem { return [self itemAtRow: [self selectedRow]]; }

// gives all selected items
- (NSArray*)allSelectedItems {
    NSMutableArray *items = [NSMutableArray array];
    NSEnumerator *selectedRows = [self selectedRowEnumerator];
    NSNumber *selRow = nil;
    while( (selRow = [selectedRows nextObject]) ) {
        if ([self itemAtRow:[selRow intValue]]) 
            [items addObject: [self itemAtRow:[selRow intValue]]];
    }
    return items;
}

// select all items in the specified array
- (void)selectItems:(NSArray*)items byExtendingSelection:(BOOL)extend {
    unsigned int i;
    if (extend==NO) [self deselectAll:nil];
    for (i=0;i<[items count];i++) {
        int row = [self rowForItem:[items objectAtIndex:i]];
        if(row>=0) [self selectRow: row byExtendingSelection:YES];
    }
}

// given a row value, get the next row, and
// wrap around to the beginning of the list
// if specified

- (int) nextRow:(int)curRow wrapOK:(BOOL)wrapFlag
{
	int numRows = [self numberOfRows];
	int nextRow;
	
	// check if current row is out of range
	if ((curRow < -1) || (curRow >= numRows))
	{
		return NSNotFound;
	}
	
	nextRow = curRow + 1;
	if (nextRow >= numRows)
	{
		if (YES == wrapFlag)
		{
			return (0);
		}
		else
		{
			return NSNotFound;
		}
	}

	return (nextRow);
}

// given a row value, get the next selected row, and
// wrap around to the beginning of the list if specified

- (int) nextSelectedRow:(int)curSelectedRow wrapOK:(BOOL)wrapFlag
{
	int nextSelRow;
	NSIndexSet *selRowIndexes = nil;
	
	selRowIndexes = [self selectedRowIndexes];
	if (nil == selRowIndexes) return NSNotFound;

	// if current selected row is bogus, return
	// the first selected row in the list
	if ((curSelectedRow < 0) || (curSelectedRow > [selRowIndexes lastIndex]))
	{
		return ([selRowIndexes firstIndex]);
	}

	nextSelRow = [selRowIndexes indexGreaterThanIndex: curSelectedRow];
	if (nextSelRow == NSNotFound)
	{
		if (YES == wrapFlag)
			return ([selRowIndexes firstIndex]);
		else
			return NSNotFound;
	}
	
	return(nextSelRow);
}

// get the last selected row
- (int) lastSelectedRow
{
	NSIndexSet *selRowIndexes = [self selectedRowIndexes];

	if (nil == selRowIndexes) return NSNotFound;

	return([selRowIndexes lastIndex]);
}

// get the first selected row
- (int) firstSelectedRow
{
	NSIndexSet *selRowIndexes = [self selectedRowIndexes];
	if (nil == selRowIndexes) return NSNotFound;

	return([selRowIndexes firstIndex]);
}


@end

@implementation NSOutlineView (MyActiveRowExtensions)


@end

@implementation MyOutlineView 

- (NSDragOperation)draggingSourceOperationMaskForLocal:(BOOL)isLocal {
    if (isLocal) return NSDragOperationEvery;
    else return NSDragOperationCopy;
}

- (BOOL)acceptsFirstMouse:(NSEvent *)event
{
    return YES;
}


@end

