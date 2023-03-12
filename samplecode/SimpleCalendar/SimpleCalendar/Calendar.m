/*

File: Calendar.m

Abstract: Model of a calendar.

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

/*
 *  Calendar is the "model" object of a calendar that manages CalEvent objects. 
 *  The Calendar object fetches CalEvent objects, applies changes from iCal to 
 *  local CalEvent objects, and saves user changes to CalEvent objects.
 */

#import "Calendar.h"
#import <CalendarStore/CalendarStore.h>

@implementation Calendar

- (id)init
{
    self = [super init];
    if (self) {
		// Add the receiver as an observer of Calendar Store notifications
		[[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(eventsChanged:) 
			name:CalEventsChangedExternallyNotification object:[CalCalendarStore defaultCalendarStore]];
		[[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(eventsChanged:) 
			name:CalEventsChangedNotification object:[CalCalendarStore defaultCalendarStore]];

		// Create a predicate to use to fetch the events
		NSInteger year = [[NSCalendarDate date] yearOfCommonEra];
		startDate = [[NSCalendarDate dateWithYear:year month:1 day:1 hour:0 minute:0 second:0 timeZone:nil] retain]; 
		endDate = [[NSCalendarDate dateWithYear:year month:12 day:31 hour:23 minute:59 second:59 timeZone:nil] retain]; 
		NSPredicate *eventsForThisYear = [NSPredicate eventPredicateWithStartDate:startDate endDate:endDate 
		   calendars:[[CalCalendarStore defaultCalendarStore] calendars]];

		// Fetch all events for the current year
		events = [[NSMutableArray array] retain];
		[self addEventArray:[[CalCalendarStore defaultCalendarStore] eventsWithPredicate:eventsForThisYear]];
	}
    return self;
}

- (void)dealloc
{
	// Remove the receiver as an observer of Calendar Store notifications
	[[NSNotificationCenter defaultCenter] removeObserver:self 
		name:CalEventsChangedExternallyNotification object:[CalCalendarStore defaultCalendarStore]];
	[[NSNotificationCenter defaultCenter] removeObserver:self 
		name:CalEventsChangedNotification object:[CalCalendarStore defaultCalendarStore]];
	[events release];
	[startDate release];
	[endDate release];
	
	[super dealloc];
}

// Returns the receiver's events.
- (NSMutableArray *)events
{
	return events;
}

// Adds an array of events to the calendar. Uses mutableArrayValueForKey: so changes 
// made locally are updated in the UI via KVO and Cocoa Bindings. Also, adds the
// receiver as an observer of changes to all events. This is needed to save local changes
// to Calendar Store.
- (void)addEventArray:(NSArray *)someEvents
{
	NSEnumerator *eventEnumerator = [someEvents objectEnumerator];
	id event;
	
	while (event = [eventEnumerator nextObject]){
		// Observe changes to the start and end dates
		[event addObserver:self forKeyPath:@"startDate"
				   options:(NSKeyValueObservingOptionNew | NSKeyValueObservingOptionOld) 
				   context:NULL];
		[event addObserver:self forKeyPath:@"endDate"
				   options:(NSKeyValueObservingOptionNew | NSKeyValueObservingOptionOld) 
				   context:NULL];
		[event addObserver:self forKeyPath:@"title"
				   options:(NSKeyValueObservingOptionNew | NSKeyValueObservingOptionOld) 
				   context:NULL];
	}
	
	// This triggers KVO messages to CalendarController
	[[self mutableArrayValueForKey:@"events"] addObjectsFromArray:someEvents];
}

// Removes an array of events from the calendar. Uses mutableArrayValueForKey: so changes 
// made locally are updated in the UI via KVO and Cocoa Bindings. Also removes the receiver
// as an observer of the removed events.
- (void)removeEventArray:(NSArray *)someEvents
{
	NSEnumerator *eventEnumerator = [someEvents objectEnumerator];
	id event;
	
	while (event = [eventEnumerator nextObject]){
		[event removeObserver:self forKeyPath:@"startDate"];
		[event removeObserver:self forKeyPath:@"endDate"];
		[event removeObserver:self forKeyPath:@"title"];
	}
	
	// This triggers KVO messages to CalendarController
	[[self mutableArrayValueForKey:@"events"] removeObjectsInArray:someEvents];
}


// Catch and update changes made to records externally. Uses addEvents: and removeEvents: that 
// triggers updates to the UI via KVO and Cocoa Bindings.
- (void)eventsChanged:(NSNotification *)notification 
{
	NSLog(@"Invoking eventsChanged:...");
	
	// Apply delete changes to the events array.
	NSArray *deletedRecords = [[notification userInfo] valueForKey:CalDeletedRecordsKey];
	if (deletedRecords != nil){
		NSLog(@"Removing events from Calendar model deletedRecords=%@", [deletedRecords description]);
		
		// Find the local CalEvent objects to delete by matching the UIDs.
		NSEnumerator *uidEnumerator = [deletedRecords objectEnumerator];
		NSString *uid;
		NSPredicate *uidPredicate;
		NSArray *filteredEvents;
		NSMutableArray *someEvents = [NSMutableArray array];
		
		while (uid = [uidEnumerator nextObject]){
			uidPredicate = [NSPredicate predicateWithFormat:@"uid like %@", uid];
			filteredEvents = [events filteredArrayUsingPredicate:uidPredicate];
			if (filteredEvents != nil)
				[someEvents addObjectsFromArray:filteredEvents];
			else
				NSLog(@"   Can't find event for uid=%@", uid);
		}
		
		// This triggers KVO messages to CalendarController
		[self removeEventArray:someEvents];
	}
	
	// Apply insert changes to the events array.
	NSArray *insertedRecords = [[notification userInfo] valueForKey:CalInsertedRecordsKey];
	if (insertedRecords != nil){
		NSLog(@"Adding events to Calendar model insertedRecords=%@", [insertedRecords description]);
		
		// Create an array containing the CalEvent objects to add. Transforms the UIDs to CalEvent objects.
		NSEnumerator *uidEnumerator = [insertedRecords objectEnumerator];
		NSString *uid;
		NSMutableArray *someEvents = [NSMutableArray array];

		while (uid = [uidEnumerator nextObject]){
			// Since recurring events have the same UIDs, create a predicate that will fetch all events belonging to a
			// recurrence.
			NSPredicate *uidPredicate = [NSPredicate eventPredicateWithStartDate:startDate endDate:endDate UID:uid
				calendars:[[CalCalendarStore defaultCalendarStore] calendars]];
			NSArray *filteredEvents = [[CalCalendarStore defaultCalendarStore] eventsWithPredicate:uidPredicate];
			if (filteredEvents != nil)
				[someEvents addObjectsFromArray:filteredEvents];
			else
				NSLog(@"   Can't find any remote events for uid=%@", uid);
		}
		
		// This triggers KVO messages to CalendarController
		[self addEventArray:someEvents];
	}

	// Apply update changes to event properites.
	NSArray *updatedRecords = [[notification userInfo] valueForKey:CalUpdatedRecordsKey];
	if (updatedRecords != nil){
		NSLog(@"Updating events in Calendar model... updatedRecords=%@", [updatedRecords description]);
		
		// Currently, you need to delete and add the changed event.
		
		// Create an array containing the local CalEvent objects to update.
		// Finds the local CalEvent objects in the events array that have
		// the same UIDs.
		NSEnumerator *uidEnumerator = [updatedRecords objectEnumerator];
		NSString *uid;
		NSPredicate *uidPredicate;
		NSArray *filteredEvents;
		NSMutableArray *oldEvents = [NSMutableArray array], *newEvents = [NSMutableArray array];
		
		while (uid = [uidEnumerator nextObject]){
			uidPredicate = [NSPredicate predicateWithFormat:@"uid like %@", uid];
			filteredEvents = [events filteredArrayUsingPredicate:uidPredicate];
			if (filteredEvents != nil)
				[oldEvents addObjectsFromArray:filteredEvents];
			else
				NSLog(@"   Can't find any local events for uid=%@", uid);
		}
		
		// This triggers KVO messages to CalendarController
		//NSLog(@".... removing oldEvents=%@", [oldEvents description]);
		[self removeEventArray:oldEvents];

		// Create an array containing the new CalEvent objects to add. Transforms UIDs to CalEvent objects.
		uidEnumerator = [updatedRecords objectEnumerator];		
		while (uid = [uidEnumerator nextObject]){
			// Since recurring events have the same UIDs, create a predicate that will fetch all events belonging to a
			// recurrence.
			NSPredicate *uidPredicate = [NSPredicate eventPredicateWithStartDate:startDate endDate:endDate UID:uid
				calendars:[[CalCalendarStore defaultCalendarStore] calendars]];
			NSArray *filteredEvents = [[CalCalendarStore defaultCalendarStore] eventsWithPredicate:uidPredicate];
			if (filteredEvents != nil)
				[newEvents addObjectsFromArray:filteredEvents];
			else
				NSLog(@"   Can't find any remote events for uid=%@", uid);
		}
		
		// This triggers KVO messages to CalendarController
		//NSLog(@".... adding newEvents=%@", [newEvents description]);
		[self addEventArray:newEvents];
	}

	return;
}



// KVO Methods

// Applies local changes to CalEvent object properties by the UI and saves them to Calendar Store.
- (void)observeValueForKeyPath:(NSString *)keyPath ofObject:(id)object change:(NSDictionary *)change context:(void *)context
{	
 	NSLog(@"observeValueForKeyPath:... keyPath=%@", keyPath);	
	
	// A property of a record changed
	if ([object isKindOfClass:[CalEvent class]] && ([[change valueForKey:NSKeyValueChangeKindKey] intValue] == NSKeyValueChangeSetting)){
		NSLog(@"The UI or the calendar model (originated by iCal) changed a CalEvent property...");
		// Need to differentiate UI changes from iCal changes so iCal changes are not pushed back to iCal (at the moment Calendar
		// is not applying individual changes so this method is only invoked when the UI changed a property).
		
		// Tempararily, remove the receiver as an observer of Calendar Store notifications
		// (Otherwise, saving the changes will trigger a local update to the same record.)
		[[NSNotificationCenter defaultCenter] removeObserver:self 
			name:CalEventsChangedExternallyNotification object:[CalCalendarStore defaultCalendarStore]];
		[[NSNotificationCenter defaultCenter] removeObserver:self 
			name:CalEventsChangedNotification object:[CalCalendarStore defaultCalendarStore]];
		
		// Save the change to Calendar Store.
		[[CalCalendarStore defaultCalendarStore] saveEvent:object span:CalSpanThisEvent];

		// Add the receiver as an observer of Calendar Store notifications
		[[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(eventsChanged:) 
			name:CalEventsChangedExternallyNotification object:[CalCalendarStore defaultCalendarStore]];
		[[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(eventsChanged:) 
			name:CalEventsChangedNotification object:[CalCalendarStore defaultCalendarStore]];
	}	
	
	return;
}

@end
 