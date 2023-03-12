/*
 
 File: CalController.m
 
 Abstract: Bindings and notification support for Calendar data used
	by this application.  Exposes read-only collections 
	(calendars, events, tasks) as observable entities.

 Version: 1.0
 
 Disclaimer: IMPORTANT:  This Apple software is supplied to you by 
 Apple Inc. ("Apple") in consideration of your agreement to the
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
 Neither the name, trademarks, service marks or logos of Apple Inc. 
 may be used to endorse or promote products derived from the Apple
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
 
 Copyright (C) 2007 Apple Inc. All Rights Reserved.
 
 */ 

#import "CalController.h"
#import <CalendarStore/CalendarStore.h>

@implementation CalController

- (void)awakeFromNib {
	// Register a transformer object for easy generation of human-readable priority strings
	// See CalPriorityToStringTransformer implementation below
	CalPriorityToStringTransformer *prioTransformer = [[[CalPriorityToStringTransformer alloc] init] autorelease];
	[NSValueTransformer setValueTransformer:prioTransformer forName:@"CalPriorityToStringTransformer"];
	
	// Register for notifications on calendars, events and tasks so we can 
	// update the GUI to reflect any changes beneath us
	[[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(calendarsChanged:) name:CalCalendarsChangedExternallyNotification object:nil];
	[[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(calendarsChanged:) name:CalCalendarsChangedNotification object:nil];
	
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(eventsChanged:) name:CalEventsChangedExternallyNotification object:nil];
	[[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(eventsChanged:) name:CalEventsChangedNotification object:nil];
	
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(tasksChanged:) name:CalTasksChangedExternallyNotification object:nil];
	[[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(tasksChanged:) name:CalTasksChangedNotification object:nil];	
}


#pragma mark -
#pragma mark Calendar Data Bindings Support
#pragma mark -


// Set up the read-only calendars/events/tasks arrays from Calendar Store as
// observable keys for Cocoa Bindings
// This in conjunction with the notifications will allow for immediate UI updates
// whenever calendar data changes outside of this app
- (NSArray *)calendars {
	return [[CalCalendarStore defaultCalendarStore] calendars];
}

- (NSArray *)events {
	CalCalendarStore *store = [CalCalendarStore defaultCalendarStore];
    // Pull all events starting now from all calendars in the CalendarStore
	NSPredicate *allEventsPredicate = [CalCalendarStore eventPredicateWithStartDate:[NSDate date] endDate:[NSDate distantFuture] calendars:[store calendars]];
	return [store eventsWithPredicate:allEventsPredicate];
}

- (NSArray *)tasks {
	CalCalendarStore *store = [CalCalendarStore defaultCalendarStore];
    // Pull all uncompleted tasks from all calendars in the CalendarStore
	return [store tasksWithPredicate:[CalCalendarStore taskPredicateWithUncompletedTasks:[store calendars]]];
}


#pragma mark -
#pragma mark Notification Handlers
#pragma mark -


// With the observable keys set up above and the appropriate bindings in IB,
// we can trigger UI updates just by signaling changes to the keys
- (void) calendarsChanged:(NSNotification *)notification {
	[self willChangeValueForKey:@"calendars"];
	[self didChangeValueForKey:@"calendars"];
}

- (void) eventsChanged:(NSNotification *)notification {
	[self willChangeValueForKey:@"events"];
	[self didChangeValueForKey:@"events"];
}
- (void) tasksChanged:(NSNotification *)notification {
	[self willChangeValueForKey:@"tasks"];
	[self didChangeValueForKey:@"tasks"];
}

@end


#pragma mark -
#pragma mark Priority Transformer for Bindings
#pragma mark -


const NSString *highPriority = @"High";
const NSString *normPriority = @"Normal";
const NSString *lowPriority = @"Low";
const NSString *nonePriority = @"None";

// The CalPriorityToStringTransformer class allows easy conversion between CalPriority values (0-9)
// and human-readable priority strings (High, Normal, Low, None)
// This allows us to populate the priority dropdown using bindings
@implementation CalPriorityToStringTransformer

+ (Class) transformedValueClass {
	return [NSString class];
}

+ (BOOL) allowsReverseTransformation {
	return NO;
}

- (id)transformedValue:(id)value {
	CalPriority priority = [value unsignedIntValue];
	if (priority < CalPriorityHigh) {
		return nonePriority;
	} else if (priority < CalPriorityMedium) {
		return highPriority;
	} else if (priority == CalPriorityMedium) {
		return normPriority;
	}
	return lowPriority;
}

@end