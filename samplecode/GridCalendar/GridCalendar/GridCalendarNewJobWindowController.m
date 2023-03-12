/*
 
 File: GridCalendarNewJobWindowController.m
 
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
 
 Copyright 2005 Apple Computer, Inc., All Rights Reserved
 
*/

#import "GridCalendarNewJobWindowController.h"

@implementation GridCalendarNewJobWindowController

- (void)windowDidLoad;
{
    [super windowDidLoad];
    
    _startDate = [[NSCalendarDate alloc] initWithYear:2005 month:1 day:1 hour:0 minute:0 second:0 timeZone:nil];
    _endDate = [[NSCalendarDate alloc] initWithYear:2005 month:12 day:1 hour:0 minute:0 second:0 timeZone:nil];
}

- (void)dealloc;
{
    [_startDate release];
    [_endDate release];
    
    [super dealloc];
}

- (NSDictionary *)jobSpecification;
{
    // put the dates in ascending order
    
	NSCalendarDate *startDate = [NSCalendarDate dateWithTimeIntervalSinceReferenceDate:[_startDate timeIntervalSinceReferenceDate]];
    NSCalendarDate *endDate = [NSCalendarDate dateWithTimeIntervalSinceReferenceDate:[_endDate timeIntervalSinceReferenceDate]];
	
    if ([startDate compare:endDate] == NSOrderedDescending) {
        
        NSCalendarDate *tempDate = startDate;
        startDate = endDate;
        endDate = tempDate;
    }
    
    // create the name
    
    int startYearInt = [startDate yearOfCommonEra];
    int startMonthInt = [startDate monthOfYear];
    int endYearInt = [endDate yearOfCommonEra];
    int endMonthInt = [endDate monthOfYear];
    
    NSString *name = [NSString stringWithFormat:@"Months from %.2f to %.2f", startYearInt + startMonthInt/100.0, endYearInt + endMonthInt/100.0];
	
    // create task prototypes
    
	NSString *taskCommand = @"/usr/bin/cal";
    
	NSMutableDictionary *taskPrototype = [NSMutableDictionary dictionary];
    
	[taskPrototype setObject:taskCommand forKey:XGJobSpecificationCommandKey];
        
	NSMutableDictionary *taskPrototypes = [NSMutableDictionary dictionary];
    
    NSString *taskPrototypeIdentifier = taskCommand;
    
	[taskPrototypes setObject:taskPrototype forKey:taskPrototypeIdentifier];
    
	// create task specifications

	NSMutableDictionary *taskSpecifications = [NSMutableDictionary dictionary];

    int currentYearInt = startYearInt;
    int currentMonthInt = startMonthInt;
    
    BOOL done = NO;
    
    while (done == NO) {
        
        NSString *currentMonth = [NSString stringWithFormat:@"%d", currentMonthInt];
        NSString *currentYear = [NSString stringWithFormat:@"%d", currentYearInt];
        
        NSArray *taskArguments = [NSArray arrayWithObjects:currentMonth, currentYear, nil];
        
        NSMutableDictionary *taskSpecification = [NSMutableDictionary dictionary];
        
        [taskSpecification setObject:taskPrototypeIdentifier forKey:XGJobSpecificationTaskPrototypeIdentifierKey];
        [taskSpecification setObject:taskArguments forKey:XGJobSpecificationArgumentsKey];

        NSString *taskIdentifier = [NSString stringWithFormat:@"%.2f", currentYearInt + currentMonthInt/100.0];
        
        [taskSpecifications setObject:taskSpecification forKey:taskIdentifier];
        
        if (currentYearInt == endYearInt && currentMonthInt == endMonthInt) {
            
            done = YES;
        }
        else {
            
            currentMonthInt += 1;
            
            if (currentMonthInt > 12) {
                
                currentMonthInt = 1;
                currentYearInt += 1;
            }
        }
    }
    	
	// create job specification
    
	NSString *applicationIdentifier = @"com.apple.xgrid.sample.calendar";
    
	NSMutableDictionary *jobSpecification = [NSMutableDictionary dictionary];
    
	[jobSpecification setObject:name forKey:XGJobSpecificationNameKey];
	[jobSpecification setObject:applicationIdentifier forKey:XGJobSpecificationApplicationIdentifierKey];
    [jobSpecification setObject:taskPrototypes forKey:XGJobSpecificationTaskPrototypesKey];
	[jobSpecification setObject:taskSpecifications forKey:XGJobSpecificationTaskSpecificationsKey];
	
	return jobSpecification;
}

@end
