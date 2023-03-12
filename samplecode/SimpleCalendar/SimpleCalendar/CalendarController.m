/*

File: CalendarController.h

Abstract: Controls a calendar model object.

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

#import "CalendarController.h"
#import "Calendar.h"
#import "DayController.h"
#import <CalendarStore/CalendarStore.h>

static int DaysPerWeek = 7; // just in case you want to display a different number of days per week, ha-ha

// CalendarController is a traditional Cocoa controller that encapsulates a Calendar view of events. 
// Calendar is the owner of the Calendar.nib file and has a view and window. The model is a collection of calendar years. 
// Each calendar has an array of months, and each month has an array of weeks. The Calendar view is a box containing tiled subviews.
// Each subview represents a day in the current month. The day view consists of a composite view and controller.
// The content of each controller is set to one of the models of a day.

// Instances of Calendar observe changes to the iCal events. When events are added or removed from iCal, 
// the CalendarController adds and removes those objects from the day models, consequently updating the 
// view of the current month.

// This class also provides methods for moving one year forward and backward, and moving one month forward and backward.

@implementation CalendarController

- (id)init
{
    self = [super init];
    if (self) {
		_calendarYears = [[NSMutableArray array] retain];

		// Initially create the current year, and add years later as needed--as events are added the dates are checked.
		NSCalendarDate *today = [NSCalendarDate calendarDate];
		NSCalendarDate *year = [NSCalendarDate dateWithYear:[today yearOfCommonEra]
													  month:1 day:1 hour:0 minute:0 second:0
												   timeZone:[today timeZone]];
		[_calendarYears addObject:[self _createCalendarYear:year]];
		//NSLog(@"calendarYears=%@", [_calendarYears description]);
		
		// Load my window and view
		if (![NSBundle loadNibNamed:@"Calendar.nib" owner:self]) {
			NSLog(@"failed to load Calendar.nib");
		}
		[window setBackgroundColor:[NSColor whiteColor]];
		
		NSRect superFrame = [view frame];
		_dayControllers = [[NSMutableArray array] retain];
		int i;
		float x = 0.0, y = superFrame.size.height;
		float w = superFrame.size.width/7;
		float h = superFrame.size.height/5;
		for (i = 0; i < 5; i++){
			int j;
			for (j= 0; j < 7; j++){
				DayController *dayCtlr = [[DayController alloc] init];
				[_dayControllers addObject:dayCtlr];
				[[dayCtlr view] setFrame:NSMakeRect(x, y - h, w, h)];
				[view addSubview:[dayCtlr view]]; // will retain view
				x += w-1;
			}
			y -= h-1; // the last frame height
			x = 0.0;
		}
		[self setYearObject:[self firstYear]];
    }
    return self;
}

- (void)dealloc {
    [_calendarYears release];
	[_dayControllers release];
	[model release];

    [super dealloc];
}

// Returns the data source used by this Calendar.
- (id)model
{
	return model;
}

// Sets the receiver's data source to dataSource.
- (void)setModel:(id)anObject
{
	// Observe changes to Event records maintained by the model.
	
	// Remove the receiver as an observer of the old data source
	[model removeObserver:self forKeyPath:@"events"];
	
	// Remove all events belonging to the existing model from the calendar controller and views
	[self removeEvents:[model events]];
	
	[model release];
	model = [anObject retain];
	
	// Add events from the new model to the calendar controller and views
	[self addEvents:[model events]];
	
	// Add the receiver as an observer of the events array using a key path
	[model addObserver:self forKeyPath:@"events"
					   options:(NSKeyValueObservingOptionNew | NSKeyValueObservingOptionOld) context:NULL];
}

- (id)firstYear
{
	return [_calendarYears objectAtIndex:0];
}

// Convenience methods for testing

- (id)firstMonth
{
	if ([_calendarYears count] > 0){
		NSArray *months = [[_calendarYears objectAtIndex:0] valueForKey:@"months"];
		return [months objectAtIndex:0];
	}

	return nil;
}

// Accessor Methods

- (id)window
{
	return window;
}

- (id)year
{
	return _year;
}

- (void)displayToday
{
	NSCalendarDate *today = [NSCalendarDate calendarDate];
	[self setYearObject:[self yearWithDate:today]];
	[self setMonth:[[_year valueForKey:@"months"] objectAtIndex:([today monthOfYear]-1)]];
}

- (void)setYearObject:(id)anObject
{
	// No need to retain it, it is an object in the calendar year array
	_year = anObject;
	[self setMonth:[[_year valueForKey:@"months"] objectAtIndex:0]]; // defaults to the first month of the year
}

- (id)nextYear
{
	int nextIndex;
	if (_year == nil)
		nextIndex = 0;
	else
		nextIndex = [_calendarYears indexOfObject:_year] + 1;
	if (nextIndex >= [_calendarYears count])
		nextIndex = 0; // wrap around
	
	[self setYearObject:[_calendarYears objectAtIndex:nextIndex]];
	return _year;
}

- (id)previousYear
{
	int nextIndex;
	if (_year == nil)
		nextIndex = 0;
	else
		nextIndex = [_calendarYears indexOfObject:_year] - 1;
	if (nextIndex < 0)
		nextIndex = [_calendarYears count]-1; // wrap around
	
	[self setYearObject:[_calendarYears objectAtIndex:nextIndex]];
	return _year;
}

- (id)month
{
	return _month;
}

- (void)setMonth:(id)anObject
{
	//NSLog(@"setMonth: anObject=%@", [anObject valueForKey:@"date"]);
	// No need to retain it, it is an object in the current year
	_month = anObject;
	
	// Set the controller models so that the first month of the year is displayed
	NSEnumerator *weekEnumerator = [[_month valueForKey:@"weeks"] objectEnumerator];
	NSDictionary *week;
	NSEnumerator *controllerEnumerator = [_dayControllers objectEnumerator];
	while (week = [weekEnumerator nextObject]){
		[[controllerEnumerator nextObject] setModel:[week valueForKey:@"Sun"]];
		[[controllerEnumerator nextObject] setModel:[week valueForKey:@"Mon"]];
		[[controllerEnumerator nextObject] setModel:[week valueForKey:@"Tue"]];
		[[controllerEnumerator nextObject] setModel:[week valueForKey:@"Wed"]];
		[[controllerEnumerator nextObject] setModel:[week valueForKey:@"Thu"]];
		[[controllerEnumerator nextObject] setModel:[week valueForKey:@"Fri"]];
		[[controllerEnumerator nextObject] setModel:[week valueForKey:@"Sat"]];
	}
	
	id controller;
	while (controller = [controllerEnumerator nextObject]){
		[controller setModel:nil]; // unused controller-view pairs
	}
}

// Returns the next month object. May wrap to the first month of the first year if there is no next year.
- (id)nextMonth
{
	//NSLog(@"nextMonth _year=%@ _month=%@", [_year valueForKey:@"date"], [_month valueForKey:@"date"]);
	int nextIndex;
	if (_month == nil)
		nextIndex = 0;
	else
		nextIndex = [[_year valueForKey:@"months"] indexOfObject:_month] + 1;
	if (nextIndex >= [[_year valueForKey:@"months"] count]){
		// Go to the last month of the previous year
		(void)[self nextYear];
		nextIndex = 0; // wrap around
	}
	
	[self setMonth:[[_year valueForKey:@"months"] objectAtIndex:nextIndex]];
	//NSLog(@"nextMonth index=%d", nextIndex);
	return _month;
}

// Returns the previous month object. May wrap to the last month of the last year if there is no previous year.
- (id)previousMonth
{
	int nextIndex;
	if (_month == nil)
		nextIndex = 0;
	else
		nextIndex = [[_year valueForKey:@"months"] indexOfObject:_month] - 1;
	if (nextIndex < 0){
		// Go to the last month of the previous year
		(void)[self previousYear];
		nextIndex = [[_year valueForKey:@"months"] count] - 1; 
	}
	
	[self setMonth:[[_year valueForKey:@"months"] objectAtIndex:nextIndex]];
	//NSLog(@"previousMonth index=%d", nextIndex);
	return _month;
}

// Returns the array of calendar years.
- (id)calendarYears
{
	return _calendarYears;
}

// Returns the day model for the specified date.
- (id)dayWithDate:(NSCalendarDate *)date
{
	id year = [self yearWithDate:date];
	if (year == nil) return nil;

	// find the day
	id month = [[year valueForKey:@"months"] objectAtIndex:[date monthOfYear]-1];
	int weekOfMonth; // value of 0 through 4 or 5
	{
		int dayOfMonth = [date dayOfMonth];
		NSCalendarDate *m = [NSCalendarDate dateWithYear:[date yearOfCommonEra] 
													   month:[date monthOfYear] day:1 hour:0 minute:0 second:0
													timeZone:[date timeZone]];
		int firstDayOfWeek = [m dayOfWeek];
		weekOfMonth = (dayOfMonth + firstDayOfWeek - 1)/DaysPerWeek;
	}
	id week = [[month valueForKey:@"weeks"] objectAtIndex:weekOfMonth];
	id day = [week valueForKey:[date descriptionWithCalendarFormat:@"%a"]];
	
	return day;
}

// Return the days in the specified date range. Used to get the days for a multi-day event.
- (NSMutableArray *)daysFromDate:(NSCalendarDate *)startDate toDate:(NSCalendarDate *)endDate
{
	NSMutableArray *days = [NSMutableArray array];
	int ii, dd, mm;

	// Nothing to compute if there's no start date
	if ((startDate == nil) || [startDate isEqual:[NSNull null]])
		return nil;
	
	// Always add the first day of the event
	[days addObject:[self dayWithDate:startDate]];
	
	// Check for multi-day event
	//NSLog(@"endDate=%@ startDate=%@", [endDate description], [startDate description]);
	if ((endDate != nil) && ![endDate isEqual:[NSNull null]] && ([startDate dayOfCommonEra] < [endDate dayOfCommonEra])){
		[endDate years:NULL months:NULL days:NULL hours:NULL minutes:&mm seconds:NULL sinceDate:startDate];
		dd = (mm-1)/1440; // adjust by one minute for all day events
		NSCalendarDate *nextDate = startDate;
		for (ii = 0; ii < dd; ii++){
			nextDate =[nextDate dateByAddingYears:0 months:0 days:1 hours:0 minutes:0 seconds:0];
			[days addObject:[self dayWithDate:nextDate]];
		}
	}
	return days;
}

// Returns the year model for the specified date
- (id)yearWithDate:(NSCalendarDate *)date
{
	if ((date == nil) || [date isEqual:[NSNull null]]) return nil;
	NSEnumerator *yearEnumerator = [_calendarYears objectEnumerator];
	id year;
	
	// find the matching year
	for (year = [yearEnumerator nextObject]; 
		 (year != nil) && ([date yearOfCommonEra] != [[year valueForKey:@"date"] yearOfCommonEra]);
		 year = [yearEnumerator nextObject]){
	}

	return year;
}

// Adding and Removing Events

// Adds an array of events to the calendar.
- (void)addEvents:(NSArray *)events
{
	NSEnumerator *eventEnumerator = [events objectEnumerator];
	id event;
	
	while (event = [eventEnumerator nextObject]){
		[self addEvent:event];
	}
}

// Adds a single event to the calendar.
- (void)addEvent:(id)event
{
    // Convert dates to calendar date values
	NSCalendarDate *startDate = [[event valueForKey:@"startDate"] dateWithCalendarFormat:nil timeZone:nil];
	NSCalendarDate *endDate = [[event valueForKey:@"endDate"] dateWithCalendarFormat:nil timeZone:nil];
	
	// Check to see if you need to add the year for this event
	if ([self yearWithDate:startDate] == nil){
		NSCalendarDate *year = [NSCalendarDate dateWithYear:[startDate yearOfCommonEra] 
													  month:1 day:1 hour:0 minute:0 second:0
												   timeZone:[startDate timeZone]];
		// Add the new year and then sort the years by date
		[_calendarYears addObject:[self _createCalendarYear:year]];
		id dateDescriptor = [[[NSSortDescriptor alloc] initWithKey:@"date" ascending:YES] autorelease];
		[_calendarYears sortUsingDescriptors:[NSArray arrayWithObject:dateDescriptor]];
		/*
		{
			NSEnumerator *yearEnumerator = [_calendarYears objectEnumerator];
			id aYear;
			while (aYear = [yearEnumerator nextObject]){
				NSLog(@"year=%d", [[aYear valueForKey:@"date"] yearOfCommonEra]);
			}
		}
		 */
	}
	NSMutableArray *days = [self daysFromDate:startDate toDate:endDate];
	[self addEvent:event toDays:days];
	
	return;
}

// Adds a multi-day event to the specified days on the calendar.
- (void)addEvent:(id)event toDays:(NSArray *)days
{
	NSEnumerator *dayEnumerator = [days objectEnumerator];
	id day;
	while (day = [dayEnumerator nextObject]){
		id events = [day mutableArrayValueForKey:@"events"];
		if (events == nil){
			[day setValue:[NSMutableArray array] forKey:@"events"];
			events = [day mutableArrayValueForKey:@"events"]; 
		}
		[events addObject:event];
		[day setValue:event forKey:@"currentEvent"]; // select the last added event
	}
	// Observe changes to the start and end dates
	[event addObserver:self forKeyPath:@"startDate"
			   options:(NSKeyValueObservingOptionNew | NSKeyValueObservingOptionOld) 
			   context:NULL];
	[event addObserver:self forKeyPath:@"endDate"
			   options:(NSKeyValueObservingOptionNew | NSKeyValueObservingOptionOld) 
			   context:NULL];
	return;
}

// Removes the array of events from the calendar.
- (void)removeEvents:(NSArray *)events
{
	NSEnumerator *eventEnumerator = [events objectEnumerator];
	id event;
	
	while (event = [eventEnumerator nextObject]){
		[self removeEvent:event];
	}
}

// Removes a single event from the calendar.
- (void)removeEvent:(id)event
{
	NSCalendarDate *startDate = [[event valueForKey:@"startDate"] dateWithCalendarFormat:nil timeZone:nil];
	NSCalendarDate *endDate = [[event valueForKey:@"endDate"] dateWithCalendarFormat:nil timeZone:nil];
	NSMutableArray *days = [self daysFromDate:startDate toDate:endDate];
	[self removeEvent:event fromDays:days];
}

// Removes a multi-day event from the specified days of the calendar.
- (void)removeEvent:(id)event fromDays:(NSArray *)days
{
	NSEnumerator *dayEnumerator = [days objectEnumerator];
	id day;
	while (day = [dayEnumerator nextObject]){
		id events = [day mutableArrayValueForKey:@"events"];
		[events removeObject:event];
	}
	
	// Remove the calendar as an observer of the start and end date
	[event removeObserver:self forKeyPath:@"startDate"];
	[event removeObserver:self forKeyPath:@"endDate"];
	return;
}


// KVO Methods

// Observers changes to the data source. When records are added and removed they are added and removed from the calendar
// model and calendar views are updated via Cocoa Bindings.
- (void)observeValueForKeyPath:(NSString *)keyPath ofObject:(id)object change:(NSDictionary *)change context:(void *)context
{	
    // #warning Workaround for observer method change dictionary ValueChangeKind
	// NOTE: This method is currently being invoked without the change dictionary set as expected. The method is invoked
	// repeatedly with the NSKeyValueChangeKindKey equal to NSKeyValueChangeSetting, none of the other types of changes are
	// sent. The NSKeyValueChangeNewKey contains ALL the records, not just the added ones. Hence there is a workaround below
	// that can be removed if this issue is fixed.
	// 04-06-13 Still appears to be a bug in the Tiger seed release.
	
	NSLog(@"observeValueForKeyPath:... keyPath=%@ object=%@", keyPath, (object == model) ? @"model" : [object description]);
	
	// The model's events array changed
	if ([object isKindOfClass:[Calendar class]] && [keyPath isEqual:keyPath]){
		NSLog(@"newRecords=%@", [[[change valueForKey:NSKeyValueChangeNewKey] valueForKey:@"title"] description]);
		NSLog(@"oldRecords=%@", [[[change valueForKey:NSKeyValueChangeOldKey] valueForKey:@"title"] description]);
		NSLog(@"change=%d", [[change valueForKey:NSKeyValueChangeKindKey] intValue]);
		
		// Work around is to compare the two arrays and figure out if this is a change, remove, or add operation
		int changeKind = [[change valueForKey:NSKeyValueChangeKindKey] intValue];
		id newRecords = [change valueForKey:NSKeyValueChangeNewKey];
		id oldRecords = [change valueForKey:NSKeyValueChangeOldKey];
		NSMutableArray *delta = nil;
		if ([newRecords count] > [oldRecords count]){
			// guess that a record was added
			delta = [NSMutableArray arrayWithArray:newRecords];
			[delta removeObjectsInArray:oldRecords];
			changeKind = NSKeyValueChangeInsertion;
		}
		else if ([newRecords count] < [oldRecords count]){
			// guess that a record was removed
			delta = [NSMutableArray arrayWithArray:oldRecords];
			[delta removeObjectsInArray:newRecords];
			changeKind = NSKeyValueChangeRemoval;
		}		
		else{
			// This is probably a wrong conclusion because a record could have been replaced. In which case,
			// invoking _didAddRecords:forEntityName: on all the old and new records is harmless because it will
			// just mark records that don't have primaryKeys yet (won't harm exisitng records).
			changeKind = NSKeyValueChangeSetting;
		}
		
		if (changeKind ==  NSKeyValueChangeSetting){
			//NSLog(@"change=NSKeyValueChangeSetting");
			[self addEvents:[change valueForKey:NSKeyValueChangeNewKey]];
		}		
		if (changeKind == NSKeyValueChangeInsertion){
			//NSLog(@"change=NSKeyValueChangeInsertion");
			[self addEvents:delta];	
		}
		if (changeKind  == NSKeyValueChangeRemoval){
			//NSLog(@"change=NSKeyValueChangeRemoval");
			[self removeEvents:delta];
		}
		if (changeKind == NSKeyValueChangeReplacement){
			//NSLog(@"change=NSKeyValueChangeReplacement");
			[self removeEvents:[change valueForKey:NSKeyValueChangeOldKey]];
			[self addEvents:[change valueForKey:NSKeyValueChangeNewKey]];					
		}
	}
	
	// A property of a record changed
	else if ([object isKindOfClass:[CalEvent class]] && ([[change valueForKey:NSKeyValueChangeKindKey] intValue] == NSKeyValueChangeSetting)){
		NSLog(@"The UI or the calendar model (originated by iCal) changed a CalEvent property...");
		// Assumes the start or end date of an event changed
		if ([keyPath isEqual:@"startDate"] || [keyPath isEqual:@"endDate"]){
			NSCalendarDate *oldDate = [change valueForKey:NSKeyValueChangeOldKey];
			NSLog(@"oldDate=%@", [oldDate description]);
			NSMutableArray *days;
			if ([keyPath isEqual:@"startDate"])
				days = [self daysFromDate:oldDate toDate:[object valueForKey:@"endDate"]];
			else
				days = [self daysFromDate:[object valueForKey:@"startDate"] toDate:oldDate];
			[self removeEvent:object fromDays:days];
			[self addEvent:object];
		}
		// Any other types of changes are handled by Cocoa Bindings
	}	
	
	return;
}


// Action Methods

// Action method to display the previous month. May wrap to the last month of the last year if there is no previous year.
- (IBAction)previousMonth:(id)sender
{
	[self previousMonth];
	return;
}

// Action method to display the next month. May wrap to the first month of the first year if there is no next year.
- (IBAction)nextMonth:(id)sender
{
	[self nextMonth];
	return;
}


// Model Creation Methods--these methods are used to create the calendar, year, month, and day model objects.

// Returns an array of years that we are modeling
- (id)_createCalendarYear:(NSCalendarDate *)year {
	id dict = [self _createYearWithDate:year];
	[dict setValue:[self _createMonthsOfYear:year] forKey:@"months"];
	return dict;
}

// Returns an array of months for the given year where each month has keys: date, name, weeks
- (id)_createMonthsOfYear:(NSCalendarDate *)year
{
	NSMutableArray *slots = [NSMutableArray array];
	NSCalendarDate *month = [[year copy] autorelease];
	int j, y = [month yearOfCommonEra];
	
	for (j = y; j == y;){
		// Create a dictionary representing this month
		id dict = [self _createMonthWithDate:month];
		[dict setValue:[self _createWeeksOfMonth:month] forKey:@"weeks"];
		[slots addObject:dict];
		month = [month dateByAddingYears:0 months:1 days:0 hours:0 minutes:0 seconds:0];
		j = [month yearOfCommonEra];
	}
	
	return slots;
}

// Returns an NSArray containing weeks of a month where each week is a dictionary (see daysOfWeek:)
- (id)_createWeeksOfMonth:(NSCalendarDate *)month
{
	NSMutableArray *slots = [NSMutableArray array];
	NSCalendarDate *week = [[month copy] autorelease];
	int j, m = [week monthOfYear];
	
	for (j = m; j == m;){
		[slots addObject:[self _createDaysOfWeek:week]];
		week = [week dateByAddingYears:0 months:0 days:(DaysPerWeek - [week dayOfWeek]) hours:0 minutes:0 seconds:0];
		j = [week monthOfYear];
	}
	
	return slots;
}

// Returns a dictionary representation of a week in a month withkeys: Sun, Mon, Tues, Wed, Thurs, Fri, Sat
- (id)_createDaysOfWeek:(NSCalendarDate *)week
{
	NSMutableArray *slots =[NSMutableArray array];
	NSCalendarDate *day;
	int m = [week monthOfYear];
	int i, j;
	
	// Create preceeding month's days to fill out the week
	int firstDayOfWeek = [week dayOfWeek];
	for (i=0; i < firstDayOfWeek; i++){
		day = [week dateByAddingYears:0 months:0 days:(i-firstDayOfWeek) hours:0 minutes:0 seconds:0];
		[slots addObject:day];
	}
	
	// Create remaining days
	day = [[week copy] autorelease];
	for (i = firstDayOfWeek, j = m; (i < DaysPerWeek) && (j == m); i++){
		[slots addObject:day];
		day = [day dateByAddingYears:0 months:0 days:1 hours:0 minutes:0 seconds:0];
		j = [day monthOfYear];
	}
	
	// Now you have an array containing the dates in this week to use to create a dictionary representation of the week
	NSMutableDictionary *dict = [NSMutableDictionary dictionary];
	
	for (i = 0; i < [slots count]; i++) {
		id value = [self _createDayWithDate:[slots objectAtIndex:i]];
		id key = [[slots objectAtIndex:i] descriptionWithCalendarFormat:@"%a"];
		//NSLog(@"day key=%@", key);
		[dict setValue:value forKey:key];
	}
	
	// NOTE: You probably want to share the day objects from the previous or preceeding months with the previous
	// or preceeding weeks. Should enhance this later to do so if this causes problems (multiple reps of the same day).
	
	return dict;
	
}

// Returns a dictionary representation of a year where the year keys are: date, months
- (id)_createYearWithDate:(NSCalendarDate *)date
{
	//NSLog(@"yearWithDate: %@", [date description]);
	NSMutableDictionary *year = [NSMutableDictionary dictionary];
	[year setValue:date forKey:@"date"];
	// more keys will be added later
	return year;
}

// Returns a dictionary representation of a month where the month keys are: date, name, weeks
- (id)_createMonthWithDate:(NSCalendarDate *)date
{
	//NSLog(@"monthWithDate: %@", [date description]);
	NSMutableDictionary *month = [NSMutableDictionary dictionary];
	[month setValue:date forKey:@"date"];
	[month setValue:[date descriptionWithCalendarFormat:@"%B"] forKey:@"name"]; // not necessary, is derived
	// more keys will be added later
	return month;
}

// Returns a dictionary representation of a day where day keys are: date, events. Events are added to the day later.
- (id)_createDayWithDate:(NSCalendarDate *)date
{
	//NSLog(@"dayWithDate: %@", [date description]);
	NSMutableDictionary *day = [NSMutableDictionary dictionary];
	[day setValue:date forKey:@"date"];
	//[day setValue:[NSMutableArray array] forKey:@"events"];
	// more keys will be added later
	return day;
}


@end
