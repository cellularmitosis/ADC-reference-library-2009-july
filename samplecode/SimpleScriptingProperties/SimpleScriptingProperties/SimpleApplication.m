/*

File: SimpleApplication.m

Abstract: for our application scripting class, we have
implemented the properties as element accessors in a category
of NSApplication. This file contains the implementation
for that.

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

Copyright (C) 2008 Apple Inc. All Rights Reserved.
 
*/ 


#import "SimpleApplication.h"
#import "scriptLog.h"


@implementation NSApplication (SimpleApplication)


	
	/* kvc method for the 'special version' AppleScript property.
	
	We have defined the 'special version' string AppleScript property as read only
	in the scripting definition file so we only provide a getter method here.	
	*/
- (NSString*) specialVersion {
	NSString *theResult = [NSString stringWithString:@"1.0"];
	SLOG(@"app's special version = %@", theResult);
	return theResult;
}




	/* our application items are implemented as a category of NSApplication so,
	as such, we don't have any instance variables for storing our application
	class' data.  So, we use globals for that storage.  */
NSNumber* gInputValue = nil;


	/* kvc methods for the 'input value' integer AppleScript property.  these are 
	implemented in the same way as most getter and setter methods, except
	we do an extra initialization check in the getter method. */
- (NSNumber*) inputValue {
		/* initial value */
	if ( gInputValue == nil ) {
		gInputValue = [[NSNumber alloc] initWithLong: 0];
	}
	SLOG(@"app's input value = %@", gInputValue);
		/* return the input value */
    return [[gInputValue retain] autorelease];
}

- (void) setInputValue:(NSNumber*)value {
	SLOG(@"set app's input value to %@", value);
    if (gInputValue != value) {
        [gInputValue release];
        gInputValue = [value copy];
    }
}

			


	/* our application items are implemented as a category of NSApplication so,
	as such, we don't have any instance variables for storing our application
	class' data.  So, we use globals for that storage.  */
NSNumber* gScalingFactor = nil;
		

	/* kvc methods for the 'scaling factor' real AppleScript property.  these are 
	implemented in the same way as most getter and setter methods, except
	we do an extra initialization check in the getter method. */
- (NSNumber*) scalingFactor {
		/* initial value */
	if ( gScalingFactor == nil ) {
		gScalingFactor = [[NSNumber alloc] numberWithDouble: 1.0];
	}
	SLOG(@"app's scaling factor = %@", gScalingFactor);
		/* return the scaling factor value */
    return [[gScalingFactor retain] autorelease];
}

- (void) setScalingFactor:(NSNumber*)value {
	SLOG(@"set app's scaling factor to %@", value);
    if (gScalingFactor != value) {
        [gScalingFactor release];
        gScalingFactor = [value copy];
    }
}



	
	/* kvc methods for the 'output value' real AppleScript property.
	
	We have defined the 'output value' AppleScript property as read only
	in the scripting definition file so we only provide a getter method here.
	
	To illustrate a calculated property value, the 'output value' property
	is calculated as the product of the 'input value' property and
	the 'scaling factor' property.
	*/
- (NSNumber*) outputValue {
	NSNumber* theResult;
	theResult = [NSNumber numberWithDouble:
		([gInputValue doubleValue] * [gScalingFactor doubleValue])];
	SLOG(@"app's output value = %@", theResult);
	return theResult;
}
	
			

			
	/* our application items are implemented as a category of NSApplication so,
	as such, we don't have any instance variables for storing our application
	class' data.  So, we use globals for that storage.  */
NSString* gDescription = nil;


	/* kvc methods for the 'description' string AppleScript property.  These are 
	implemented in the same way as most getter and setter methods, except
	we do an extra initialization check in the getter method. */
- (NSString*) description {
		/* initial value */
	if ( gDescription == nil ) {
		gDescription = [[NSString alloc] initWithString: @"no description"];
	}
	SLOG(@"app's description = %@", gDescription);
		/* return the description value */
    return [[gDescription retain] autorelease];
}

- (void) setDescription:(NSString*)value {
	SLOG(@"set app's description to %@", value);
    if (gDescription != value) {
        [gDescription release];
        gDescription = [value copy];
    }
}

			
			
	/* our application items are implemented as a category of NSApplication so,
	as such, we don't have any instance variables for storing our application
	class' data.  So, we use globals for that storage.  */
NSNumber* gReady = nil;

	/* kvc methods for the 'ready' boolean AppleScript property.  These are 
	implemented in the same way as most getter and setter methods, except
	we do an extra initialization check in the getter method. */
- (NSNumber*) ready {
		/* initial value */
	if ( gReady == nil ) {
		gReady = [[NSNumber alloc] initWithBool: YES];
	}
	SLOG(@"app's ready flag = %@", gReady);
		/* return the simplicity value */
    return [[gReady retain] autorelease];
}

- (void) setReady:(NSNumber*)value {
	SLOG(@"set app's ready flag to %@", value);
    if (gReady != value) {
        [gReady release];
        gReady = [value copy];
    }
}



	/* our application items are implemented as a category of NSApplication so,
	as such, we don't have any instance variables for storing our application
	class' data.  So, we use globals for that storage.  */
NSDate* gModificationDate = nil;

	/* kvc methods for the 'modification date' date AppleScript property.
	These are implemented in the same way as most getter and setter
	methods, except we do an extra initialization check in the
	getter method. */
- (NSDate*) modificationDate {
		/* initial value */
	if ( gModificationDate == nil ) {
		gModificationDate = [[NSDate alloc] initWithTimeIntervalSinceNow: 0];
	}
	SLOG(@"app's modification date = %@", gModificationDate);
		/* return the modification date */
    return [[gModificationDate retain] autorelease];
}

- (void) setModificationDate:(NSDate*)value {
	SLOG(@"set the modification date");
    if (gModificationDate != value) {
        [gModificationDate release];
        gModificationDate = [value copy];
    }
}
	
			
							
			
	/* The decodeSimplicity method is a utility method for converting
	the four character codes used to represent items in our enumeration
	to string values for easy reading. */
	
- (NSString*) decodeSimplicity:(NSNumber*)simplicityCode {
	NSString *theResult;
	switch ([simplicityCode longValue]) {
		case kSLBasic: theResult = @"Basic"; break;
		case kSLIntroductory: theResult = @"Introductory"; break;
		case kSLAdvanced: theResult = @"Advanced"; break;
		case kSLDifficult: theResult = @"Difficult"; break;
		default: theResult = @"Unknown"; break;
	}
	return [NSString stringWithString:theResult];
}

	



	/* our application items are implemented as a category of NSApplication so,
	as such, we don't have any instance variables for storing our application
	class' data.  So, we use globals for that storage.  */
NSNumber *gSimplicityLevel = nil;			

	/* kvc methods for the 'simplicity level' AppleScript property.
	
	this property uses an AppleScript enumeration defined in our
	scripting definition file.  The interesting thing to note here
	is that there is really nothing special you have to do in order
	to use enumeration values - they are stored in NSNumbers.*/
- (NSNumber*) simplicityLevel {
		/* initial value */
	if ( gSimplicityLevel == nil ) {
		gSimplicityLevel = [[NSNumber alloc] initWithUnsignedLong: kSLBasic];
	}
	SLOG(@"app's simplicity level = %@", [self decodeSimplicity:gSimplicityLevel]);
		/* return the simplicity value */
    return [[gSimplicityLevel retain] autorelease];
}

- (void) setSimplicityLevel:(NSNumber*)value {
	SLOG(@"set app's simplicity level to %@", [self decodeSimplicity:value]);
    if (gSimplicityLevel != value) {
        [gSimplicityLevel release];
        gSimplicityLevel = [value copy];
    }
}



	/* kvc methods for the 'scaling factor' AppleScript property.
	
	This is another example of a calculated, read-only property.
	Here we map the various simplicity levels defined in the
	enumeration to arbitrary floating point values.  This method
	illustrates how you can use the constants used by AppleScript
	to represent enumerated values in your Objective-C program. */
- (NSNumber*) simplicityFactor {
	double theFactor;
	NSNumber* theResult;
	switch ([gSimplicityLevel unsignedLongValue]) {
		case kSLBasic: theFactor = 1.2; break;
		case kSLIntroductory: theFactor = 17.3; break;
		case kSLAdvanced: theFactor = 27; break;
		case kSLDifficult: theFactor = 9000; break;
		default: theFactor = 0.0; break;
	}
	theResult = [NSNumber numberWithDouble: theFactor ];
	SLOG(@"app's simplicity factor = %@", theResult);
	return theResult;
}



@end
