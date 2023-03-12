/*
 
 File: RemindersPlugIn.m
 
 Abstract: Widget plug-in that retrieves iCal calendars, events, and to do items.
           The fetched events and to do items must start now and end before or 
           at 12:59:59 PM on the last day of this year. Registers a custom 
           JavaScript object that is used by the Reminders widget.
 
 Version: <1.0>
 
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
 
 Copyright (C) 2008 Apple Inc. All Rights Reserved.
 
 */

#import "RemindersPlugIn.h"
#import <CalendarStore/CalendarStore.h>
#import "NSColorHexadecimalValue.h"
#import "CalendarItems.h"

@implementation RemindersPlugIn

#pragma mark -
#pragma mark Functions Called by JavaScript

/*
	The plug-in adds a "js" to the beginning of any method that will be exposed to JavaScript. 
    The Web Kit's webScriptNameForSelector method returns friendly names for these exposed Objective-C methods.
*/

/*
  This method is mapped to JavaScript as "calendarEvents" and is called using <plugInName>."calendarEvents".
  It returns all events starting now and ending before or at 12:59:59 PM on December 31 of the current year.
*/
-(NSArray *)js_calendarEvents:(NSString *)calendarUIDs
{
	return [self parseCalendarEvents:[self fetchCalendarEvents:[self splitStringOfUIDs:calendarUIDs]]];
}



/*
 This method is mapped to JavaScript as "calendarToDoItems" and is called using <plugInName>."calendarToDoItems".
 It returns all to do items whose due date is set at or before the end of the current year.
*/
-(NSArray *)js_calendarToDoItems:(NSString *)calendarUIDs
{
	return [self parseCalendarToDoItems:[self fetchToDoItems:[self splitStringOfUIDs:calendarUIDs]]];
}



/*
 This method is mapped to JavaScript as "calendars" and is called using <plugInName>."calendars".
 It retrieves all specified calendars.
*/
-(NSArray *)js_calendars:(NSString *)calendarUIDs
{
	return [self parseCalendars:[self splitStringOfUIDs:calendarUIDs]];
}



/*
 This method is mapped to JavaScript as "nameForCalendarWithUIDs" and is called using <plugInName>."nameForCalendarWithUIDs".
 It receives a list of calendars' uids and returns an array containing each specified calendar's name. 
*/
-(NSString *)js_nameForCalendarWithUIDs:(NSString *)calendarUIDs
{
	NSMutableArray *calendarNames = [NSMutableArray array];
	if ([calendarUIDs length] >0)
	{
		/* Get all the calendars associated with the specified uids */
		NSArray *calendars = [self fetchCalendarsWithUIDs:[self splitStringOfUIDs:calendarUIDs]];
		for (CalCalendar *aCalendar in calendars)
		{
			/* Get each calendar's name */
			[calendarNames addObject:[aCalendar.title capitalizedString]];
		}
	}	
	return [NSArray arrayWithArray:calendarNames];
}



#pragma mark -
#pragma mark Plugin calls to JavaScript
/*
 This method is called whenever users add, delete, or update iCal events.
 It calls the JavaScript "reloadEventsOrToDoItems" function and sets its argument to false;
 this informs the widge that it must rebuild its events' display.
*/
- (void)eventsChanged:(NSNotification *)notification
{
    [webScriptObject evaluateWebScript:@"reloadEventsOrToDoItems(false)"];
}



/*
 This method is called whenever users add, delete, or update iCal events.
 It calls the JavaScript "reloadEventsOrToDoItems" function with true as an argument; 
 this informs the widge that it must rebuild its to do items's display. 
*/
- (void)tasksChanged:(NSNotification *)notification
{
	[webScriptObject evaluateWebScript:@"reloadEventsOrToDoItems(true)"];
}

#pragma mark -
#pragma mark Initialize iCal
/*
 Creates and adds a new calendar, task, and event to iCal if it is empty.
*/
-(void)initializeiCal
{
	/* Get all available events and tasks */
	NSArray *allEvents = [self js_calendarEvents:@""];
	NSArray *allTasks = [self js_calendarToDoItems:@""];
	
	/* Insert new calendar, task, and event if iCal contains no events and tasks */
	if (([allEvents count] < 1) && ([allTasks count] < 1))
	{
		NSError *error;
		CalCalendarStore * store = [CalCalendarStore defaultCalendarStore];
		
		/* Create a calendar */
		CalCalendar *sampleCalendar = [CalCalendar calendar];
		sampleCalendar.title = @"RemindersWidget";
		/* Add apmtCalendar to the calendar store. The error variable will contain an error message if this step fails */
		[store saveCalendar:sampleCalendar error:&error];
		
		/* Create an event and set up its properties */
		CalEvent *event = [CalEvent event];
		/* Associate this new event to sampleCalendar */
		event.calendar = sampleCalendar;
		event.title = @"Dentist";
		event.startDate = [NSDate dateWithNaturalLanguageString:@"3pm July 15, 2008"];
		event.endDate = [NSDate dateWithNaturalLanguageString:@"4pm July 15, 2008"];
		event.location = @"1123 Fremont Avenue";
		/* Add  event to the calendar store */
		[store saveEvent:event span:0 error:&error];
		
		
		/* Create a task  */	
		CalTask *task = [CalTask task];
		/* Associate this new task to sampleCalendar */
		task.calendar = sampleCalendar;
		task.title = @"Pick up prescription";
		task.dueDate = [NSDate dateWithNaturalLanguageString:@"2pm July 15, 2008"];
		task.priority = CalPriorityHigh;
		/* Add task to the calendar store */
		[store saveTask:task error:&error];
	}
}


#pragma mark -
#pragma mark WidgetPlugin methods 
/*
 This method is called before the widget’s HTML page is fully loaded. 
*/
- (id)initWithWebView:(WebView*)webView
{
	self = [super init];
	if (self)
	{
		[self initializeiCal];
		
		/* Observing iCal to do items changes */
		[[NSNotificationCenter defaultCenter] addObserver:self 
												 selector:@selector(tasksChanged:) 
													 name:CalTasksChangedExternallyNotification object:nil];
		
		[[NSNotificationCenter defaultCenter] addObserver:self
												 selector:@selector(tasksChanged:) 
													 name:CalTasksChangedNotification object:nil];	
		
		/* Observing iCal events changes */
		[[NSNotificationCenter defaultCenter] addObserver:self 
												 selector:@selector(eventsChanged:) 
													 name:CalEventsChangedNotification 
												   object:[CalCalendarStore defaultCalendarStore]];
		
		[[NSNotificationCenter defaultCenter] addObserver:self 
												 selector:@selector(eventsChanged:)
													 name:CalEventsChangedExternallyNotification 
												   object:[CalCalendarStore defaultCalendarStore]];
	}
	return self;
}

/*
 Indicates what instance variables should be shared with JavaScript. This method should return
 YES for all instance variables with the "js" prefix and NO otherwise.
 */
+ (BOOL)isKeyExcludedFromWebScript:(const char *)name
{
	NSString* keyName = [[[NSString alloc] initWithUTF8String: name] autorelease];
	return !([ keyName hasPrefix:kJSSelectorPrefix]);
}

/*
 Provides mapping between Objective-C and JavaScript instance variable names. All Objective-C instance variables 
 will be used in JavaScript without the "js" prefix. For instance, js_uid will be called as uid in JavaScript.
 */
+ (NSString *)webScriptNameForKey:(const char *)name
{
    NSString* keyName = [[[NSString alloc] initWithUTF8String: name] autorelease];
	if ([ keyName hasPrefix:kJSSelectorPrefix] && ([ keyName length] > [kJSSelectorPrefix length]))
	{
		return [keyName substringFromIndex:[kJSSelectorPrefix length]];
	}
	return nil;
}


#pragma mark -
#pragma mark WebScripting methods 
/*
 This method provides a bridge between Objective-C and JavaScript. It creates a JavaScript object referencing 
 the plug-in, which can be used by the widget. This allows the plug-in to be called from JavaScript using 
 <plugInName>, where plugInName is the chosen plug-in name. 
*/
- (void)windowScriptObjectAvailable:(WebScriptObject*)scriptObject
{
    /* According to this statement, our plug-in will be called as RemindersPlugIn from JavaScript */
	[scriptObject setValue:self forKey:@"RemindersPlugIn"];
	webScriptObject = scriptObject;
	[webScriptObject retain];
}



/*
 Provides mapping between Objective-C and JavaScript method names. All Objective-C functions will be used in JavaScript
 without the "js" prefix. For instance, js_calendarToDoItems will be called as calendarToDoItems in JavaScript.
*/
+ (NSString *)webScriptNameForSelector:(SEL)aSelector 
{
	NSString* selectorName = NSStringFromSelector(aSelector);
	if ([selectorName hasPrefix:kJSSelectorPrefix] && ([selectorName length] > [kJSSelectorPrefix length]))
	{
		return [[[selectorName substringFromIndex:[kJSSelectorPrefix length]] componentsSeparatedByString: @":"] objectAtIndex: 0];
	}
	return nil;
}



/*
 Determines what methods can be called by JavaSript. This method should return YES for all methods with the "js" prefix 
 and NO otherwise.
*/
+ (BOOL)isSelectorExcludedFromWebScript:(SEL)aSelector 
{
	return !([NSStringFromSelector(aSelector) hasPrefix:kJSSelectorPrefix]);
}



#pragma mark -
#pragma mark Calendars Methods
/*
 Receives a list of calendars'uids, calls the fetchCalendarsWithUIDs method to get specific calendars if the list is
 not empty and retrieves all available calendars if the list is empty. 
 Builds and returns an array of CalendarItems object. Each CalendarItems object contains a calendar's uid, color, and title.
*/
-(NSArray *)parseCalendars:(NSArray *)calendarUIDs
{
	/*
	 Check the calendarUIDs array. If calendarUIDs is not empty, fetch all calendars whose uids match the ones contained in calendarUIDs.
	 If calendarUIDs is empty, fetch all available calendars. 
	 */
	NSArray *calendarStore = ([calendarUIDs count]>0)?[self fetchCalendarsWithUIDs:calendarUIDs]:[[CalCalendarStore defaultCalendarStore] calendars];
	
	NSMutableArray *arrayCalendars = [NSMutableArray array];
	/* Iterate through all calendars */
	for (CalCalendar *aCalendar in calendarStore)
	{
		/* Fetch this calendar's uid, color, and title  and store them in a CalendarItems object */
		CalendarItems *items = [[[CalendarItems alloc] initWithUID:aCalendar.uid 
														    title:[aCalendar.title capitalizedString]
														    color:[aCalendar.color hexadecimalStringValue]] autorelease];
		
		
		/* Add the result to arrayEvents */										
		[arrayCalendars addObject:items];
	}
	return [NSArray arrayWithArray:arrayCalendars];
}



/*
 Receives a list of calendars'uids and returns all calendars that correspond to the given uids.
*/
-(NSArray *)fetchCalendarsWithUIDs:(NSArray *)calendarUIDs
{
	NSMutableArray *arrayCalendars = [NSMutableArray array];
	/* Iterate through all the uids */
	for (NSString *uid in calendarUIDs)
	{
		/* Get the calendar associated with the current uid */
		CalCalendar *calendar = [[CalCalendarStore defaultCalendarStore] calendarWithUID:uid];
		if (calendar)
		{
			[arrayCalendars addObject:calendar];	
		}
	}
	return [NSArray arrayWithArray:arrayCalendars];
}



#pragma mark -
#pragma mark Events Methods
/*
 Retrieves all events starting now and ending before or at 12:59:59 PM on December 31 of this year. 
 calendarUIDs is an array containing a list of calendars' uids.
*/
-(NSArray *)fetchCalendarEvents:(NSArray *)calendarUIDs
{ 
	CalCalendarStore *calendarStore = [CalCalendarStore defaultCalendarStore];
	/* This is the default predicate, it gets events from all calendars */
	NSPredicate *eventsForThisYear = [CalCalendarStore eventPredicateWithStartDate:[NSDate date] 
																		   endDate:[self lastDayAndTimeOfTheYear] 
																		 calendars:[calendarStore calendars]];
	
	if ([calendarUIDs count] >0)
	{
		/* Get all the calendars associated with the specified uids */
		NSArray *arrayOfCalendars = [self fetchCalendarsWithUIDs:calendarUIDs];
		
		/* arrayOfCalendars may be empty, so check how many objects it contains.
		 All retrieved events must start now and end before or at 12:59:59 PM on December 31 of this year.
		 */
		if([arrayOfCalendars count] >0)
		{
			/* If arrayOfCalendars is not empty, only retrieve events belonging to the given calendars */
			eventsForThisYear = [CalCalendarStore eventPredicateWithStartDate:[NSDate date] 
																	  endDate:[self lastDayAndTimeOfTheYear] 
																	calendars:arrayOfCalendars];
		}
	}	
	/* Apply predicate and return result */
	return [calendarStore eventsWithPredicate:eventsForThisYear];
}



/*
 Builds and returns an array of CalendarItems objects. Each CalendarItems object contains an event's start date, associated 
 calendar's color, title, and startAndEndTime. startAndEndTime is a string that contains "all-day" if the event 
 occurs all day long, the event's start and end time otherwise.
*/
-(NSArray *)parseCalendarEvents:(NSArray *)events
{
	NSMutableArray *arrayEvents = [NSMutableArray array];
	/* Check if events is not empty */
	if ([events count] > 0)
	{
		/* Iterate through all events */
		for (CalEvent *aCalendarEvent in events)
		{
			/* Indicate whether an event occurs during the whole day, contains the event's beginning and ending time 
			 if the event is not all-day */
			NSString *startAndEndTime = @"all-day";
			
			/* Check if this event is all-day or not */
			if (!(aCalendarEvent.isAllDay))	
			{
				/* Get this event's beginning and ending time since it does not last for the entire day */
				startAndEndTime = [self calendarItemStartAndEndTime:aCalendarEvent.startDate endDate:aCalendarEvent.endDate];
			}
			
			/* Let's get this event's start date, associated calendar's color, title, startAndEndTime */
			CalendarItems *items = [[[CalendarItems alloc] initWithStartDate:[self calendarItemStartDate:aCalendarEvent.startDate]
									title:[aCalendarEvent.title capitalizedString]
									 color:[aCalendarEvent.calendar.color hexadecimalStringValue]
									 timeOrPriority:startAndEndTime] autorelease];
			
			/* Add the result to the arrayEvents structure	*/										
			[arrayEvents addObject:items];
		}
	}
	return [NSArray arrayWithArray:arrayEvents];
}



#pragma mark -
#pragma mark To Do Items Methods

/*
 Fetches all incomplete to do items whose due date is set at or before the end of this year.
 */
-(NSArray *)fetchToDoItems:(NSArray *)calendarUIDs
{
	CalCalendarStore *calendarStore = [CalCalendarStore defaultCalendarStore];
	/* This is the default predicate, it gets tasks from all calendars */
	NSPredicate *toDoItemsPredicate = [CalCalendarStore taskPredicateWithUncompletedTasksDueBefore:[self lastDayAndTimeOfTheYear] 
																						 calendars:[calendarStore calendars]];	
	
	if ([calendarUIDs count] >0)
	{
		/* Get all the calendars associated with the specified uids */
		NSArray *arrayOfCalendars = [self fetchCalendarsWithUIDs:calendarUIDs];
		
		/* arrayOfCalendars may be empty, so check how many objects it contains. We create a predicate that queries for 
		 all incomplete iCal tasks whose due date is set before or at 12:59:59 PM on December 31 of this year. 
		 */
		if ([arrayOfCalendars count] >0)
		{
			NSLog(@"date %@",[self calendarItemStartDate:[self lastDayAndTimeOfTheYear] ]);
			toDoItemsPredicate = [CalCalendarStore taskPredicateWithUncompletedTasksDueBefore:[self lastDayAndTimeOfTheYear] 
																					calendars:arrayOfCalendars];
		}
	}
	/* Returns all to do items that match the toDoItemsPredicate */
	return [calendarStore tasksWithPredicate:toDoItemsPredicate];	
}



/*
 Builds and returns an array of CalendarItems objects. Each CalendarItems object contains a to do item's due date, associated 
 calendar's color, title, and priority.
*/
-(NSArray *)parseCalendarToDoItems:(NSArray *)toDoItems
{
	NSMutableArray *tasks = [NSMutableArray array];
	if ([toDoItems count] >0)
	{
		/* Loop through all the to do items */
		for (CalTask *aCalendarTask in toDoItems)
		{
			
			NSString *priority =[NSString stringWithFormat:@"%@",[self calPriorityToString:[NSNumber numberWithUnsignedInt:aCalendarTask.priority]]];
			/* Get the to do item's uid, due date, color, title, and priority */
			CalendarItems *items = [[[CalendarItems alloc] initWithStartDate:[self calendarItemStartDate:aCalendarTask.dueDate]
																	   title:[aCalendarTask.title capitalizedString]
																	   color:[aCalendarTask.calendar.color hexadecimalStringValue]
															  timeOrPriority:priority] autorelease];
			
			
			/* Store the result */											
			[tasks addObject:items];
		}
		/* Sort tasks by due date */
		[tasks sortUsingFunction:sortByDate context:@"tasks"];
	}
	return [NSArray arrayWithArray:tasks];	
}



#pragma mark -
#pragma mark NSDate Format
/*
 Creates a date reference for the last day and time of the current year.
*/
-(NSDate*)lastDayAndTimeOfTheYear        
{
	/* Get a reference to the current year */
	NSInteger year = [[NSCalendarDate date] yearOfCommonEra];
	NSCalendar *gregorian = [[NSCalendar alloc] initWithCalendarIdentifier:NSGregorianCalendar];
	
	/* Construct an NSDate for this date: December 31 of this year and time: 12:59:59 PM */
	NSDateComponents *dateComponents = [[[NSDateComponents alloc] init] autorelease];
	[dateComponents setYear:year];
	[dateComponents setMonth:12];
	[dateComponents setDay:31];
	
	[dateComponents setHour:23];
	[dateComponents setMinute:59];
	[dateComponents setSecond:59];
	return [gregorian dateFromComponents:dateComponents];
}



/*
 Extracts the date component of an NSDate object that follows the format "WeekDay, Month Day YYYY".
*/
-(NSString *)calendarItemStartDate:(NSDate *)startDate
{	
	NSDateFormatter *dateFormatter = [[[NSDateFormatter alloc] init] autorelease];
	/* Build a format that returns the date using the format "WeekDay, Month Day YYYY" */
	[dateFormatter setDateStyle:NSDateFormatterFullStyle];
	return [dateFormatter stringFromDate:startDate];
}


/*
 Extracts the time component of an NSDate object that follows the format "hour:minute:second AM/PM - hour:minute:second AM/PM".
 */
-(NSString *)calendarItemStartAndEndTime:(NSDate *)startDate endDate:(NSDate *)anEndDate
{
	NSDateFormatter *timeFormatter = [[[NSDateFormatter alloc] init] autorelease];
	
	/* Create a format that returns the time using the representation hour:minute AM/PM */
	[timeFormatter setTimeStyle:NSDateFormatterShortStyle];
	
	/* Convert both startDate and endDate using timeFormatter, concatenate them, and return the result */
	return [NSString stringWithFormat:@"%@-%@", [timeFormatter stringFromDate:startDate], [timeFormatter stringFromDate:anEndDate]];
}



#pragma mark -
#pragma mark Sort 2D array
/*
   Sorts an array of CalendarItems objects by date.  
*/
NSComparisonResult sortByDate(id firstItem, id secondItem, void *context)
{
	/* Create a date formatter and set its style to "WeekDay, Month Day YYYY" */
	NSDateFormatter *firstItemFormatter = [[[NSDateFormatter alloc] init] autorelease];
	[firstItemFormatter setDateStyle:NSDateFormatterFullStyle];
	
	NSDateFormatter *secondItemFormatter = [[[NSDateFormatter alloc] init] autorelease];
	[secondItemFormatter setDateStyle:NSDateFormatterFullStyle];
	
	/* Retrieve the date string */
	NSDate *firstItemDate = [firstItemFormatter dateFromString:[firstItem js_startDate]];
	NSDate *secondItemDate = [secondItemFormatter dateFromString:[secondItem js_startDate]];
    
    /* Compare both date strings */
    NSComparisonResult comparison = [firstItemDate compare:secondItemDate];
	return comparison;
}



#pragma mark -
#pragma mark CalPriority String Representation
/*
 Converts CalPriority values (0, 1, 5, 9) to human-readable priority strings (High, Normal, Low, None).
*/
-(NSString *)calPriorityToString:(NSNumber *)priority
{
	NSString *priorityToString = @"";
	switch([priority unsignedIntValue])
	{
		case 0:
			priorityToString = @"None";
			break;
		case 1:
			priorityToString = @"High";
			break;
		case 5:
			priorityToString = @"Medium";
			break;
		case 9:
			priorityToString = @"Low";
			break;
		default:
			break;	
	}
	return priorityToString;
}



#pragma mark -
#pragma mark NSString Parsing
/*
 This method receives a string made of calendars' uids, which are concatenated together with a "&".
 If the string is empty, splitStringOfUIDs returns an empty array. splitStringOfUIDs breaks 
 the string and returns an array that contains all the string's pieces if it is not empty. 
*/ 
-(NSArray *)splitStringOfUIDs:(NSString *)string
{
	/* Check the received string's length.
	 if the length is less than or equal to 0 (empty string), return an empty aray.
	 If the length is greater than 0 (string is not empty), return an array containg all the string's components.
	 */
	NSArray *parsedStrings = ([string length]<=0) ? [NSArray array] : [string componentsSeparatedByString:@" & "];
	return parsedStrings;
}



-(void)dealloc
{
	/* Unregister the observer */
	[[NSNotificationCenter defaultCenter] removeObserver:self];
	[webScriptObject release];
	[super dealloc];
}
@end
