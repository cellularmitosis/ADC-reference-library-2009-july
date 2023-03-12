 /*
 
 File: AppController.m
 
 Abstract: Controller code for responding to control events and creating new
	CalTask/CalEvent objects.

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

#import "AppController.h"

@implementation AppController

@synthesize calItemTitle, calItemStartDate, calItemEndDate;


#pragma mark -
#pragma mark GUI Response Methods
#pragma mark -


- (IBAction)showTaskCreationDialog:(id)sender {
    // Set default values for the title, start date and priority
    // Cocoa bindings will clear out the related fields in the sheet
	self.calItemTitle = nil;
	self.calItemStartDate = [NSDate date];
	[NSApp beginSheet:taskCreationDialog
		modalForWindow:mainWindow
		modalDelegate:self
		didEndSelector:@selector(didEndSheet:returnCode:contextInfo:)
		contextInfo:nil];
}

- (IBAction)showEventCreationDialog:(id)sender {
    // Set default values for the title and start/end date
    // Cocoa bindings will clear out the related fields in the sheet
	self.calItemTitle = nil;
	self.calItemStartDate = [NSDate date];
	self.calItemEndDate = [NSDate dateWithTimeIntervalSinceNow:(3600)];
	[NSApp beginSheet:eventCreationDialog
		modalForWindow:mainWindow
		modalDelegate:self
		didEndSelector:@selector(didEndSheet:returnCode:contextInfo:)
		contextInfo:nil];
}

// Called when the "Add" button is pressed on the event/task entry sheet
// This starts the sheet dismissal process
- (IBAction)dismissDialog:(id)sender {
	[NSApp endSheet:[sender window]];
}

- (void)didEndSheet:(NSWindow *)sheet returnCode:(int)returnCode contextInfo:(void *)contextInfo {
	// Find out which calendar was selected for the new event/task
	// We do this using the calendarData array controller which is bound to the calendar popups in the sheet
    CalCalendar *selectedCalendar = nil;
    int count = [[calendarData selectedObjects] count];
    if (count > 0) {
        NSString *selectedCalendarUID = [[[calendarData selectedObjects] objectAtIndex:0] uid];
        selectedCalendar = [[CalCalendarStore defaultCalendarStore] calendarWithUID:selectedCalendarUID];
    }
    // Create an event/task based on which sheet was used
    if (sheet == taskCreationDialog) {
        if (!self.calItemTitle) self.calItemTitle = @"My Task";
        [self createNewTaskWithCalendar:selectedCalendar title:self.calItemTitle priority:[priorityPopup selectedTag] dueDate:self.calItemStartDate];
    } else {
        if (!self.calItemTitle) self.calItemTitle = @"My Event";
        [self createNewEventWithCalendar:selectedCalendar title:self.calItemTitle startDate:self.calItemStartDate endDate:self.calItemEndDate];
    }
    
    // Dismiss the sheet
    [sheet orderOut:self]; 
}


#pragma mark -
#pragma mark Record Creation Methods
#pragma mark -


// These methods create an iCal event or task (To Do).  Implement the methods using the Calendar Store API.

- (id)createNewEventWithCalendar:(CalCalendar *)calendar title:(NSString *)title startDate:(NSDate *)startDate endDate:(NSDate *)endDate {
	// Create a new CalEvent object
	
	// Set the calendar, title, start date and end date on the new event
	// using the parameters passed to this method

	// Save the new event to the calendar store (CalCalendarStore) and return it
	
    return nil;
}

- (id)createNewTaskWithCalendar:(CalCalendar *)calendar title:(NSString *)title priority:(CalPriority)priority dueDate:(NSDate *)dueDate {
	// Create a new CalTask object

	// Set the calendar, title, priority and due date on the new task
	// using the parameters passed to this method
	
	// Save the new task to the calendar store (CalCalendarStore) and return it

	return nil;
}
@end