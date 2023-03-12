/*

File: MyWindowController.m

Abstract: Header file for this sample's main NSWindowController.

Version: 1.0

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

Copyright © 2006-2007 Apple Inc. All Rights Reserved

*/

#import "MyWindowController.h"

@implementation MyWindowController

// -------------------------------------------------------------------------------
//	dealloc:
// -------------------------------------------------------------------------------
- (void)dealloc
{
	[[NSNotificationCenter defaultCenter] removeObserver:self];
    [query release];
    [super dealloc];
}

// -------------------------------------------------------------------------------
//	awakeFromNib:
// -------------------------------------------------------------------------------
-(id)awakeFromNib
{
	// no vertical scrolling, we always want to show all rows
	[[predicateEditor enclosingScrollView] setHasVerticalScroller:NO];
	
	previousRowCount = 3;
	[predicateEditor addRow:self];
	
	// put the focus in the first text field
    id displayValue = [[predicateEditor displayValuesForRow:1] lastObject];
    if ([displayValue isKindOfClass:[NSControl class]])
		[[self window] makeFirstResponder:displayValue];
	
	// create and initalize our query
	query = [[NSMetadataQuery alloc] init];
							
	// setup our Spotlight notifications 
	NSNotificationCenter* nf = [NSNotificationCenter defaultCenter];
	[nf addObserver:self selector:@selector(queryNotification:) name:nil object:query];

	// initialize our Spotlight query, sort by contact name
	[query setSortDescriptors:[NSArray arrayWithObject:[[[NSSortDescriptor alloc] initWithKey:(id)kMDItemContactKeywords ascending:YES] autorelease]]];
	[query setDelegate: self];
	
	// start with our progress search label empty
	[progressSearchLabel setStringValue: @""];
	
	return [self init];
}

// -------------------------------------------------------------------------------
//	applicationShouldTerminateAfterLastWindowClosed:sender
//
//	NSApplication delegate method placed here so the sample conveniently quits
//	after we close the window.
// -------------------------------------------------------------------------------
- (BOOL)applicationShouldTerminateAfterLastWindowClosed:(NSApplication*)sender
{
	return YES;
}

// -------------------------------------------------------------------------------
//	loadResultsFromQuery:
// -------------------------------------------------------------------------------
- (void)loadResultsFromQuery:(NSNotification*)notif
{
	NSArray* results = [(NSMetadataQuery*)[notif object] results];
	
	NSLog(@"search count = %d", [results count]);
	NSString* foundResultsStr = [[NSString alloc] initWithFormat:@"Results found: %ld", [results count]];
	[progressSearchLabel setStringValue: foundResultsStr];
	
	// iterate through the array of results, and match to the existing stores
	NSInteger i;
	for (i = 0; i < [results count];  i++)
	{
		// get the result item
		NSMetadataItem*	item = [results objectAtIndex: i];

		NSString* cityStr = [item valueForAttribute: (NSString *)kMDItemCity];
		NSString* nameStr = [item valueForAttribute: (NSString *)kMDItemDisplayName];
		NSString* stateStr = [item valueForAttribute: (NSString *)kMDItemStateOrProvince];
		
		NSArray* phoneNumbers = [item valueForAttribute: (NSString *)kMDItemPhoneNumbers];
		NSString* phoneStr = nil;
		if (phoneNumbers != nil)
			phoneStr = [phoneNumbers objectAtIndex: 0];	// grab only the first phone number
			
		NSString* storePath = [[item valueForAttribute: (NSString *)kMDItemPath] stringByResolvingSymlinksInPath];

		// create a dictionary entry to be added to our search results array
		NSString* emptyStr = [[[NSString alloc] init] autorelease];
		NSDictionary* dict = [NSDictionary dictionaryWithObjectsAndKeys:
							 [NSString stringWithString: (nameStr != nil) ? nameStr : emptyStr], @"name",
							 [NSString stringWithString: (phoneStr != nil) ? phoneStr : emptyStr], @"phone",
							 [NSString stringWithString: (cityStr != nil) ? cityStr : emptyStr], @"city",
							 [NSString stringWithString: (stateStr != nil) ? stateStr : emptyStr], @"state",
							 [NSURL fileURLWithPath: storePath], @"url",
							 nil];
		[mySearchResults addObject: dict];
	}
}

// -------------------------------------------------------------------------------
//	queryNotification:note
// -------------------------------------------------------------------------------
- (void)queryNotification:(NSNotification*)note
{
    // the NSMetadataQuery will send back a note when updates are happening. By looking at the [note name], we can tell what is happening
    if ([[note name] isEqualToString:NSMetadataQueryDidStartGatheringNotification])
	{
        // the query has just started
        NSLog(@"search: started gathering");
		
		[progressSearch setHidden:NO];
		[progressSearch startAnimation:self];
		[progressSearch animate:self];
		[progressSearchLabel setStringValue: @"Searching..."];
    }
	else if ([[note name] isEqualToString:NSMetadataQueryDidFinishGatheringNotification])
	{
        // at this point, the query will be done. You may recieve an update later on.
        NSLog(@"search: finished gathering");
		
		[progressSearch setHidden:YES];
		[progressSearch stopAnimation:self];
	
		[self loadResultsFromQuery:note];
    }
	else if ([[note name] isEqualToString:NSMetadataQueryGatheringProgressNotification])
	{
        // the query is still gathering results...
		NSLog(@"search: progressing...");
		
		[progressSearch animate:self];
    }
	else if ([[note name] isEqualToString:NSMetadataQueryDidUpdateNotification])
	{
        // an update will happen when Spotlight notices that a file as added, removed, or modified that affected the search results.
        NSLog(@"search: an update happened.");
    }
}

// -------------------------------------------------------------------------------
//	inspect:selectedObjects
//
//	This method obtains the selected object (in our case for single selection,
//	it's the first item), and opens its URL.
// -------------------------------------------------------------------------------
- (void)inspect:(NSArray*)selectedObjects
{
	NSDictionary* objectDict = [selectedObjects objectAtIndex: 0];
	if (objectDict != nil)
	{
		NSURL* url = [objectDict valueForKey: @"url"];
		[[NSWorkspace sharedWorkspace] openURL:url];
	}
}

// -------------------------------------------------------------------------------
//	spotlightFriendlyPredicate:predicate
//
//	This method will "clean up" an NSPredicate to make it ready for Spotlight, or return nil if the predicate can't be cleaned.
//
//	Foundation's Spotlight support in NSMetdataQuery places the following requirements on an NSPredicate:
//		- Value-type (always YES or NO) predicates are not allowed
//		- Any compound predicate (other than NOT) must have at least two subpredicates
// -------------------------------------------------------------------------------
- (NSPredicate *)spotlightFriendlyPredicate:(id)predicate
{
    if ([predicate isEqual:[NSPredicate predicateWithValue:YES]] || [predicate isEqual:[NSPredicate predicateWithValue:NO]])
	{
		return nil;
	}
    else if ([predicate isKindOfClass:[NSCompoundPredicate class]])
	{
		NSCompoundPredicateType type = [predicate compoundPredicateType];
		NSMutableArray *cleanSubpredicates = [NSMutableArray array];
		for (NSPredicate *dirtySubpredicate in [predicate subpredicates])
		{
			NSPredicate *cleanSubpredicate = [self spotlightFriendlyPredicate:dirtySubpredicate];
			if (cleanSubpredicate) [cleanSubpredicates addObject:cleanSubpredicate];
		}
		
		if ([cleanSubpredicates count] == 0)
			return nil;
		else
		{
			if ([cleanSubpredicates count] == 1 && type != NSNotPredicateType)
				return [cleanSubpredicates objectAtIndex:0];
			else
				return [[[NSCompoundPredicate alloc] initWithType:type subpredicates:cleanSubpredicates] autorelease];
		}
    }
    else
	{
		return predicate;
	}
}

// -------------------------------------------------------------------------------
//	createNewSearchForPredicate:predicate:withTitle
//
// -------------------------------------------------------------------------------
- (void)createNewSearchForPredicate:(NSPredicate*)predicate withTitle:(NSString*)title
{
    if (predicate != nil)
	{
		[mySearchResults removeObjects:[mySearchResults arrangedObjects]];	// remove the old search results
		
		// always search for items in the Address Book
		NSPredicate* addrBookPredicate = [NSPredicate predicateWithFormat:@"(kMDItemKind = 'Address Book Person Data')"];
		predicate = [NSCompoundPredicate andPredicateWithSubpredicates:[NSArray arrayWithObjects:addrBookPredicate, predicate, nil]];
	   
		[query setPredicate: predicate];
		[query startQuery];
    }
}

// -------------------------------------------------------------------------------
//	predicateEditorChanged:sender
//
//  This method gets called whenever the predicate editor changes.
//	It is the action of our predicate editor and the single plate for all our updates.
//	
//	We need to do potentially three things:
//		1) Fire off a search if the user hits enter.
//		2) Add some rows if the user deleted all of them, so the user isn't left without any rows.
//		3) Resize the window if the number of rows changed (the user hit + or -).
// -------------------------------------------------------------------------------
- (IBAction)predicateEditorChanged:(id)sender
{
	// check NSApp currentEvent for the return key
    NSEvent* event = [NSApp currentEvent];
    if ([event type] == NSKeyDown)
	{
		NSString* characters = [event characters];
		if ([characters length] > 0 && [characters characterAtIndex:0] == 0x0D)
		{
			// get the predicat, which is the object value of our view
			NSPredicate* predicate = [predicateEditor objectValue];
			
			// make it Spotlight friendly
			predicate = [self spotlightFriendlyPredicate:predicate];
			if (predicate)
			{
				static NSInteger searchIndex = 0;
				NSString* title = NSLocalizedString(@"Search #%ld", @"Search title");
				[self createNewSearchForPredicate:predicate withTitle:[NSString stringWithFormat:title, (long)++searchIndex]];
			}
		}
    }
    
    // if the user deleted the first row, then add it again - no sense leaving the user with no rows
    if ([predicateEditor numberOfRows] == 0)
		[predicateEditor addRow:self];
    
    // resize the window vertically to accomodate our views:
        
    // get the new number of rows, which tells us the needed change in height,
	// note that we can't just get the view frame, because it's currently animating - this method is called before the animation is finished.
    NSInteger newRowCount = [predicateEditor numberOfRows];
    
    // if there's no change in row count, there's no need to resize anything
    if (newRowCount == previousRowCount)
		return;

    // The autoresizing masks, by default, allows the NSTableView to grow and keeps the predicate editor fixed.
	// We need to temporarily grow the predicate editor, and keep the NSTableView fixed, so we have to change the autoresizing masks.
	// Save off the old ones; we'll restore them after changing the window frame.
	NSScrollView* tableScrollView = [myTableView enclosingScrollView];
	NSUInteger oldOutlineViewMask = [tableScrollView autoresizingMask];
    
    NSScrollView* predicateEditorScrollView = [predicateEditor enclosingScrollView];
    NSUInteger oldPredicateEditorViewMask = [predicateEditorScrollView autoresizingMask];
    
	[tableScrollView setAutoresizingMask:NSViewWidthSizable | NSViewMaxYMargin];
    [predicateEditorScrollView setAutoresizingMask:NSViewWidthSizable | NSViewHeightSizable];
        
    // determine if we need to grow or shrink the window
    BOOL growing = (newRowCount > previousRowCount);
    
    // if growing, figure out by how much.  Sizes must contain nonnegative values, which is why we avoid negative floats here.
    CGFloat heightDifference = fabs([predicateEditor rowHeight] * (newRowCount - previousRowCount));
    
    // convert the size to window coordinates -
	// if we didn't do this, we would break under scale factors other than 1.
	// We don't care about the horizontal dimension, so leave that as 0.
	//
    NSSize sizeChange = [predicateEditor convertSize:NSMakeSize(0, heightDifference) toView:nil];
    
	// offset our status view
	NSRect frame = [progressView frame];
	[progressView setFrameOrigin: NSMakePoint(frame.origin.x, frame.origin.y - [predicateEditor rowHeight] * (newRowCount - previousRowCount))];
	
	// change the window frame size:
	// - if we're growing, the height goes up and the origin goes down (corresponding to growing down).
	// - if we're shrinking, the height goes down and the origin goes up.
    NSRect windowFrame = [[self window] frame];
    windowFrame.size.height += growing ? sizeChange.height : -sizeChange.height;
    windowFrame.origin.y -= growing ? sizeChange.height : -sizeChange.height;
    [[self window] setFrame:windowFrame display:YES animate:YES];
    
	// restore the autoresizing mask
	[tableScrollView setAutoresizingMask:oldOutlineViewMask];
    [predicateEditorScrollView setAutoresizingMask:oldPredicateEditorViewMask];

    previousRowCount = newRowCount;	// save our new row count
}

@end
