/*

File: SimpleApplication.m

Abstract: for our application scripting class, we have
implemented the properties as element accessors in a
category of NSApplication. This file contains the
interface for that.

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



#import <Cocoa/Cocoa.h>

@interface NSApplication (SimpleApplication)


	/* kvc method for the 'special version' AppleScript property.
	
	We have defined the 'special version' AppleScript property as read only
	in the scripting definition file so we only provide a getter method here.	
	*/
- (NSString*) specialVersion;


	/* kvc methods for the 'input value' AppleScript property. */
- (NSNumber*) inputValue;
- (void) setInputValue:(NSNumber*)value;


	/* kvc methods for the 'scaling factor' AppleScript property. */
- (NSNumber*) scalingFactor;
- (void) setScalingFactor:(NSNumber*)value;


	/* kvc methods for the 'output value' AppleScript property.
	
	We have defined the 'output value' AppleScript property as read only
	in the scripting definition file so we only provide a getter method here.
	
	To illustrate a calculated property value, the 'output value' property
	is calculated as the product of the 'input value' property and
	the 'scaling factor' property.
	*/
- (NSNumber*) outputValue;


	/* kvc methods for the 'description' AppleScript property. */
- (NSString*) description;
- (void) setDescription:(NSString*)value;


	/* kvc methods for the 'ready' AppleScript property. */
- (NSNumber*) ready;
- (void) setReady:(NSNumber*)value;


	/* kvc methods for the 'modification date' AppleScript property. */
- (NSDate*) modificationDate;
- (void) setModificationDate:(NSDate*)value;



	/* we have defined an enumeration in our scripting definition
	file that we are using as the value stored in the simplicity level
	property.  The actual values passed back from AppleScript to
	Objective-C are encoded using the four byte codes we specified
	in the enumeration.  For convenience, we have associated those
	values with symbolic constants in the following enumeration:
	*/
enum {
	kSLBasic = 'SiBa',
	kSLIntroductory = 'SiIn',
	kSLAdvanced = 'SiAd',
	kSLDifficult = 'SiDi'
};

	/* and we have defined the following method for converting those
	code values into string values.  */
- (NSString*) decodeSimplicity:(NSNumber*)simplicityCode;


	/* kvc methods for the 'simplicity level' AppleScript property.
	
	this property uses the enumeration discussed in the comment above.*/
- (NSNumber*) simplicityLevel;
- (void) setSimplicityLevel:(NSNumber*)value;


	/* kvc methods for the 'scaling factor' AppleScript property.
	
	This is another example of a calculated property.  Here we map
	the various simplicity levels defined in the enumeration to
	arbitrary floating point values.  */
- (NSNumber*) simplicityFactor;


@end
