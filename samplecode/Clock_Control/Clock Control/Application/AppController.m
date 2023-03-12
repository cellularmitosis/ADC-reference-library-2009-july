/*
 AppController.m
 Application controller object.

 Author: CP
  
 Copyright (c) 2002, Apple Computer, Inc., all rights reserved.
*/

/*
 IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc. ("Apple") in
 consideration of your agreement to the following terms, and your use, installation, 
 modification or redistribution of this Apple software constitutes acceptance of these 
 terms.  If you do not agree with these terms, please do not use, install, modify or 
 redistribute this Apple software.
 
 In consideration of your agreement to abide by the following terms, and subject to these 
 terms, Apple grants you a personal, non-exclusive license, under Apple’s copyrights in 
 this original Apple software (the "Apple Software"), to use, reproduce, modify and 
 redistribute the Apple Software, with or without modifications, in source and/or binary 
 forms; provided that if you redistribute the Apple Software in its entirety and without 
 modifications, you must retain this notice and the following text and disclaimers in all 
 such redistributions of the Apple Software.  Neither the name, trademarks, service marks 
 or logos of Apple Computer, Inc. may be used to endorse or promote products derived from 
 the Apple Software without specific prior written permission from Apple. Except as expressly
 stated in this notice, no other rights or licenses, express or implied, are granted by Apple
 herein, including but not limited to any patent rights that may be infringed by your 
 derivative works or by other works in which the Apple Software may be incorporated.
 
 The Apple Software is provided by Apple on an "AS IS" basis.  APPLE MAKES NO WARRANTIES, 
 EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED WARRANTIES OF NON-INFRINGEMENT, 
 MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS 
 USE AND OPERATION ALONE OR IN COMBINATION WITH YOUR PRODUCTS.
 
 IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL OR CONSEQUENTIAL 
 DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS 
 OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) ARISING IN ANY WAY OUT OF THE USE, 
 REPRODUCTION, MODIFICATION AND/OR DISTRIBUTION OF THE APPLE SOFTWARE, HOWEVER CAUSED AND 
 WHETHER UNDER THEORY OF CONTRACT, TORT (INCLUDING NEGLIGENCE), STRICT LIABILITY OR 
 OTHERWISE, EVEN IF APPLE HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#import "AppController.h"
#import "ClockControl.h"


@implementation AppController

- (void)dealloc {
    [appointments release];
    appointments = nil;
}

- (void)awakeFromNib {
    // Create the storage our table will use.
    appointments = [[NSMutableArray array] retain];
    
    // Be the data source... nah..nah..nah...nah....
    [myTable setDataSource: self];
    
    // Make the @"Time" column use a clock cell.
    [[myTable tableColumnWithIdentifier:@"Time"] setDataCell: [[[ClockCell alloc] init] autorelease]];
    
    // Let the text in the @"Info" column wrap.
    [[[myTable tableColumnWithIdentifier:@"Info"] dataCell] setWraps: YES];
    
    // Start with atleast one appointment.
    [self addAppointment:nil];
}

// ---------------------------------------------------------
//  Action methods
// ---------------------------------------------------------

- (void)clockTimeChanged:(id)sender {
    [timeReadout setStringValue: [sender stringValue]];
}

- (void)addAppointment:(id)sender {
    // Append a newly created data object, then reload the table contents.
    AppointmentData *appointmentData = [[AppointmentData alloc] init];
    [appointments addObject: appointmentData];
    [appointmentData release];
    [myTable reloadData];
}

- (void)removeSelectedAppointment:(id)sender {
    // Remove the selected row from the data set, then reload the table contents.
    [appointments removeObjectAtIndex: [myTable selectedRow]];
    [myTable reloadData];
}

// ---------------------------------------------------------
//  Data source methods
// ---------------------------------------------------------

- (int)numberOfRowsInTableView:(NSTableView *)tv {
    return [appointments count];
}

- (id)tableView:(NSTableView *)tv objectValueForTableColumn:(NSTableColumn *)tc row:(int)row {
    if ([[tc identifier] isEqualToString:@"Time"]) {
        return [[appointments objectAtIndex:row] time];
    } else {
        return [[appointments objectAtIndex:row] info];
    }
}

- (void)tableView:(NSTableView *)tv setObjectValue:(id)objectValue forTableColumn:(NSTableColumn *)tc row:(int)row {
    if ([[tc identifier] isEqualToString:@"Time"]) {
        [[appointments objectAtIndex:row] setTime: objectValue];
    } else {
        [[appointments objectAtIndex:row] setInfo: objectValue];
    }
}

@end
