/*
 
 File: CalendarItems.m
 
 Abstract: Creates an object that stores specific properties of 
           an iCal calendar, event, or to do item.
 
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

#import "CalendarItems.h"

/* Prefix for all methods and instance variables that will be exposed in the widget */
NSString *const kJSSelectorPrefix = @"js_";

@implementation CalendarItems
@synthesize js_uid;
@synthesize js_startDate;
@synthesize js_title;
@synthesize js_color;
@synthesize js_timeOrPriority;

/*
   Creates a CalendarItems instance, which will store data for events and to do items.
*/
-(id)initWithStartDate:(NSString *)aDate 
				 title:(NSString *)aTitle 
				 color:(NSString *)aColor 
		timeOrPriority:(NSString *)aValue
{
	self = [super init];
	if (self!= nil) 
	{
		self.js_startDate = aDate;
		self.js_title = aTitle;
		self.js_color = aColor;
		self.js_timeOrPriority = aValue;
	}
	return self;	
}



/*
	Creates a CalendarItems instance, which will store data for calendars.
*/
-(id)initWithUID:(NSString *)aUid
		   title:(NSString *)aTitle 
		   color:(NSString *)aColor
{
	self = [super init];
	if (self!= nil) 
	{
		self.js_uid = aUid;
		self.js_title = aTitle;
		self.js_color = aColor;
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



-(void)dealloc
{
	[js_uid release];
	[js_startDate release];
	[js_title release];
	[js_color release];
	[js_timeOrPriority release];
	[super dealloc];
}
@end
