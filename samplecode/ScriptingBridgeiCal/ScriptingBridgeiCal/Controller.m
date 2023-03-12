/* File: Controller.m

Abstract: implementation for main window's controller.

Version: 1.0

(c) Copyright 2007 Apple, Inc. All rights reserved.

IMPORTANT:  This Apple software is supplied to 
you by Apple, Inc. ("Apple") in 
consideration of your agreement to the following 
terms, and your use, installation, modification 
or redistribution of this Apple software 
constitutes acceptance of these terms.  If you do 
not agree with these terms, please do not use, 
install, modify or redistribute this Apple 
software.

In consideration of your agreement to abide by 
the following terms, and subject to these terms, 
Apple grants you a personal, non-exclusive 
license, under Apple's copyrights in this 
original Apple software (the "Apple Software"), 
to use, reproduce, modify and redistribute the 
Apple Software, with or without modifications, in 
source and/or binary forms; provided that if you 
redistribute the Apple Software in its entirety 
and without modifications, you must retain this 
notice and the following text and disclaimers in 
all such redistributions of the Apple Software. 
Neither the name, trademarks, service marks or 
logos of Apple Computer, Inc. may be used to 
endorse or promote products derived from the 
Apple Software without specific prior written 
permission from Apple.  Except as expressly 
stated in this notice, no other rights or 
licenses, express or implied, are granted by 
Apple herein, including but not limited to any 
patent rights that may be infringed by your 
derivative works or by other works in which the 
Apple Software may be incorporated.

The Apple Software is provided by Apple on an "AS 
IS" basis.  APPLE MAKES NO WARRANTIES, EXPRESS OR 
IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED 
WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY 
AND FITNESS FOR A PARTICULAR PURPOSE, REGARDING 
THE APPLE SOFTWARE OR ITS USE AND OPERATION ALONE 
OR IN COMBINATION WITH YOUR PRODUCTS.

IN NO EVENT SHALL APPLE BE LIABLE FOR ANY 
SPECIAL, INDIRECT, INCIDENTAL OR CONSEQUENTIAL 
DAMAGES (INCLUDING, BUT NOT LIMITED TO, 
PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS 
OF USE, DATA, OR PROFITS; OR BUSINESS 
INTERRUPTION) ARISING IN ANY WAY OUT OF THE USE, 
REPRODUCTION, MODIFICATION AND/OR DISTRIBUTION OF 
THE APPLE SOFTWARE, HOWEVER CAUSED AND WHETHER 
UNDER THEORY OF CONTRACT, TORT (INCLUDING 
NEGLIGENCE), STRICT LIABILITY OR OTHERWISE, EVEN 
IF APPLE HAS BEEN ADVISED OF THE POSSIBILITY OF 
SUCH DAMAGE.
*/


#import "Controller.h"
#import "iCal.h"





@implementation Controller



- (void)awakeFromNib {

		/* set the date to today's date */
	[time setDateValue: [NSDate date]];

		/* locate the application, fetch it's dictionary, and synthesize all of the
		classes it describes. */
	iCalApplicationClass = nil;
			//[SBApplication applicationWithBundleIdentifier:@"com.apple.iCal"];
}




	/* we set ourself to the NSApplication's delegate in the .nib file.  Adding
	this method is a minor convenience so the sample will quit when the
	window is closed. */
- (BOOL)applicationShouldTerminateAfterLastWindowClosed:(NSApplication *)sender {
	return YES;
}





	/* called when the user clicks on the Add/Update Event button
	in the main window.  will set the start time of the named event
	in the named calendar to the indicated time (creating the calendar
	and the event if they do not already exist). */ 
- (IBAction)addUpdateEvent:(id)sender {


	/* code from Step 11: */
	
		/* reference to our iCal application object */
	iCalApplication *iCal;
	
		/* get the scripting bridge object for the target application. */
	iCal = [SBApplication applicationWithBundleIdentifier:@"com.apple.iCal"];
	
		/* bring the iCal application into view */
	[iCal activate];
	

	/* code from Step 12: */

		/* reference to our calendar object */
	iCalCalendar *theCalendar = nil;

		/* get the name of the calendar from the window. */
	NSString *calendarName = [calendar stringValue];
		
		/* get the calendar with the specified name using the
		-filteredArrayUsingPredicate: method.  */
	NSArray *matchingCalendars =
		[[iCal calendars] filteredArrayUsingPredicate:
			[NSPredicate predicateWithFormat:@"name == %@", calendarName]];
	if ( [matchingCalendars count] > 0 ) {
		theCalendar = (iCalCalendar *) [matchingCalendars objectAtIndex:0];
	}
	
		/* If no such calendar exists, then create a new one with that name. */
	if ( theCalendar == nil ) {

			/* set up the properties for the new calendar */
		NSDictionary *props =
			[NSDictionary dictionaryWithObjectsAndKeys:
				calendarName, @"name",
				nil];
		
			/* allocate and initialize the new calendar */
		theCalendar = [[[iCal classForScriptingClass:@"calendar"] alloc] initWithProperties: props];
		
			/* ...and add it to the list of calendars in the iCal application. */
		[[iCal calendars] addObject: theCalendar]; 
	}
	


	/* code from Step 13: */
	
		/* get the event with the specified name from the calendar.  If
		no such event exists, then create a new one with that name.  */

		/* reference to our event object */
	iCalEvent *theEvent;

		/* get the name of the event from the window. */
	NSString *eventName = [event stringValue];
	
		/* calculate start and end times for a one hour event
		starting at the time displayed in the window. */
	NSDate* startDate = [time dateValue];
	
		/* set the end date to the start time plus one hour (3600 seconds). */
	NSDate* endDate = [[NSDate alloc] initWithTimeInterval:3600 sinceDate:startDate];
	
	
		/* the event summary contains the name displayed in the iCal calendar windows,
		so we can't use [[[theCalendar events] objectWithName:eventName] exists]
		to find out if the event exists. 
		
		So, instead of that we're going to use filteredArrayUsingPredicate to retrieve
		an array of all of the events with a matching summary. */
		
	NSArray *matchingEvents =
		[[theCalendar events] filteredArrayUsingPredicate:
			[NSPredicate predicateWithFormat:@"summary == %@", eventName]];
		
		/* if we found at least one matching event, then we'll update the
		times for the first event with a matching summary */
	if ( [matchingEvents count] >= 1 ) {
	
			/* get the first matching event out of the response */
		theEvent = (iCalEvent *) [matchingEvents objectAtIndex:0];

			/* update the dates for the event */
		[theEvent setStartDate:startDate];
		[theEvent setEndDate:endDate];
		
	} else {
			/* otherwise, create a new event */
			
			/* set up the event properties.  Note, we're using kvc names from
			the iCal.h file to name the properties encoded in the NSDictonary. */
		NSDictionary *props = 
			[NSDictionary dictionaryWithObjectsAndKeys:
				eventName, @"summary",
				startDate, @"startDate",
				endDate, @"endDate",
				nil];

			/* create the new event */
		theEvent = [[[iCal classForScriptingClass:@"event"] alloc]
							initWithProperties: props];
		
			/* add it to the list of events for this calendar. */
		[[theCalendar events] addObject: theEvent];
		
	}
	
}



@end
