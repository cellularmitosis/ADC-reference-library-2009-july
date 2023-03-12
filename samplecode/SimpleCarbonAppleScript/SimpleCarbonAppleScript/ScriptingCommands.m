/*

File: ScriptingCommands.m

Abstract: This file includes implementation for the AppleScript
commands defined for this application.

Version: 1.0

© Copyright 2007 Apple Computer, Inc. All rights reserved.

IMPORTANT:  This Apple software is supplied to 
you by Apple Computer, Inc. ("Apple") in 
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

#import <AppKit/NSApplication.h>
#import <Carbon/Carbon.h>

#import "ScriptingCommands.h"
#import "scriptLog.h"


#define kLeftHandOSType 'LHrn'
#define kRightHandOSType 'RHrn'
#define kLeftHandControlValue 1
#define kRightHandControlValue 2
#define kPreferredHandControlSignature 'HAND'
#define kBootsControlSignature 'BOOT'
#define kBeverageControlSignature 'BEVR'
#define kHelmetControlSignature 'HELM'
#define kTitleControlSignature 'TITL'



	/* GetControlBySignature - a utility routine for retrieving controls from
	the frontmost window.  */
static OSStatus GetControlBySignature(OSType theSignature, ControlRef *theControl) {
	ControlID theID;
	OSStatus err;
	theID.signature = theSignature;
	theID.id = 0;
	err = GetControlByID( FrontNonFloatingWindow(), &theID, theControl );
	return err;
}

	
	/* This is our error handling routine.  We call this routine when an error
	needs to be reported to the user.  This routine takes the errorNumber we provide
	and the optional errorString and puts them into the reply record for the
	current Apple event.  For most OSStatus values, you do not need to provide
	an errorString as AppleScript will provide a descriptive string for you. 
	errorNumber must be a non-zero value.  */
static void SetScriptingError( OSStatus errorNumber, NSString* errorString ) {
		/* get a reference to the Apple event handling machinery */
	NSAppleEventManager* theAEM = [NSAppleEventManager sharedAppleEventManager];
	if ( theAEM != nil ) {
	
			/* get a reference to the reply record for the current Apple event */
		NSAppleEventDescriptor*	theReply = [theAEM currentReplyAppleEvent];
		if ( theReply != nil ) {
		
				/* add the numeric error code to the reply record */
			NSAppleEventDescriptor* theNumberDesc = [NSAppleEventDescriptor descriptorWithInt32:errorNumber];
			if ( theNumberDesc != nil ) {
				[theReply setDescriptor:theNumberDesc forKeyword:keyErrorNumber];
			}

				/* if a string was provided, add it to the reply record */
			if ( errorString != nil ) {
					/* get a localized copy of the string */
				NSString* localErrorString = NSLocalizedString( errorString, nil );
				if ( localErrorString != nil ) {
						/* create a descriptor containing the string */
					NSAppleEventDescriptor* theStringDesc = [NSAppleEventDescriptor descriptorWithString:localErrorString];
						/* add the string to the reply record */
					if ( theStringDesc != nil ) {
						[theReply setDescriptor:theStringDesc forKeyword:keyErrorString];
					}
				}
			}
		}
	}
}


@implementation scriptingCommands



	/* accessor functions for the title attribute of our scriptingCommands object.
	The names used for these methods are the same as the names specified in the .sdef
	file.   for the 'title' attribut, the 'getter' method is named 'title' and the 'setter'
	method is named 'setTitle' (by adding the prefix 'set' and capitalizing the name). */
- (NSString*) title {

	ControlRef theControl;
	CFStringRef theText;
	OSStatus err;
	
    scriptLog(@"scriptingCommands title");

	err = GetControlBySignature(kTitleControlSignature, &theControl);
	if (err == noErr) {
		GetControlData( theControl, kControlEntireControl, kControlStaticTextCFStringTag, 
			sizeof( theText ), &theText, NULL );
	} else {
		SetScriptingError( err, nil );
		theText = NULL;
	}
	return (NSString*) theText;
}

- (void) setTitle:(NSString*) title {

	ControlRef theControl;
	CFStringRef theText;
	OSStatus err;
	
    scriptLog(@"scriptingCommands setTitle:");
	
	theText = (CFStringRef) title;
	err = GetControlBySignature(kTitleControlSignature, &theControl);
	if (err == noErr) {
		SetControlData( theControl, kControlEntireControl, kControlStaticTextCFStringTag, 
			sizeof( theText ), &theText );
		Draw1Control(theControl);
	} else {
		SetScriptingError( err, nil );
	}
}




- (NSNumber*) helmet {

	ControlRef theControl;
	CFStringRef theText;
	NSNumber* theValue;
	OSStatus err;
	
    scriptLog(@"scriptingCommands helmet");

	err = GetControlBySignature(kHelmetControlSignature, &theControl);
	if (err == noErr) {
		GetControlData( theControl, kControlEntireControl, kControlStaticTextCFStringTag, 
			sizeof( theText ), &theText, NULL );
		theValue = [[NSNumber alloc] initWithLong: [((NSString*)theText) intValue]];
	} else {
		SetScriptingError( err, nil );
		theValue = nil;
	}
	return theValue;
}

- (void) setHelmet:(NSNumber*) helmetsize {

	ControlRef theControl;
	CFStringRef theText;
	OSStatus err;
	
    scriptLog(@"scriptingCommands setHelmet:");

	theText = (CFStringRef) [helmetsize stringValue];
	err = GetControlBySignature(kHelmetControlSignature, &theControl);
	if (err == noErr) {
		SetControlData( theControl, kControlEntireControl, kControlStaticTextCFStringTag, 
			sizeof( theText ), &theText );
		Draw1Control(theControl);
	} else {
		SetScriptingError( err, nil );
	}
}




- (NSString*) beverage {

	ControlRef theControl;
	CFStringRef theText;
	OSStatus err;
	
    scriptLog(@"scriptingCommands beverage");

	err = GetControlBySignature(kBeverageControlSignature, &theControl);
	if (err == noErr) {
		GetControlData( theControl, kControlEntireControl, kControlStaticTextCFStringTag,
			sizeof( theText ), &theText, NULL );
	} else {
		SetScriptingError( err, nil );
		theText = NULL;
	}
	return (NSString*) theText;
}


- (void) setBeverage:(NSString*) beverage {

	ControlRef theControl;
	CFStringRef theText;
	OSStatus err;
		
    scriptLog(@"scriptingCommands setBeverage:");
	
	theText = (CFStringRef) beverage;
	err = GetControlBySignature(kBeverageControlSignature, &theControl);
	if (err == noErr) {
		SetControlData( theControl, kControlEntireControl, kControlStaticTextCFStringTag, 
			sizeof( theText ), &theText );
		Draw1Control(theControl);
	} else {
		SetScriptingError( err, nil );
	}

}




- (NSNumber*) boots {

	ControlRef theControl;
	OSStatus err;
	NSNumber* theValue;
	
    scriptLog(@"scriptingCommands boots");
	
	err = GetControlBySignature(kBootsControlSignature, &theControl);
	if (err == noErr) {
		theValue = [[NSNumber alloc] initWithLong: GetControlValue(theControl)];
	} else {
		SetScriptingError( err, nil );
		theValue = nil;
	}
	return theValue;

}

- (void) setBoots:(NSNumber*) boots {

	ControlRef theControl;
	OSStatus err;

    scriptLog(@"scriptingCommands setBoots:");

	err = GetControlBySignature(kBootsControlSignature, &theControl);
	if (err == noErr) {
		SetControlValue(theControl, [boots longValue]);
	} else {
		SetScriptingError( err, nil );
	}

}


	/* preferred hand is communicated to the application by way of OSTypes stored in
	NSNumber values.  These accessor routines translate between os type values
	and the numeric control values stored in the controls representing preferred hand. */
- (NSNumber*) preferredHand {

	ControlRef theControl;
	OSStatus err;
	NSNumber* theResult;
	
    scriptLog(@"scriptingCommands preferredHand");

	err = GetControlBySignature(kPreferredHandControlSignature, &theControl);
	if (err == noErr) {	
		if ( GetControlValue(theControl) == kLeftHandControlValue ) {
			theResult = [[NSNumber alloc] initWithLong: kLeftHandOSType];
		} else if ( GetControlValue(theControl) == kRightHandControlValue ) {
			theResult = [[NSNumber alloc] initWithLong: kRightHandOSType];
		}
	} else {
		SetScriptingError( err, nil );
		theResult = nil;
	}
	return theResult;
}


- (void) setPreferredHand:(NSNumber*) newPreferredHand {

	ControlRef theControl;
	OSStatus err;

    scriptLog(@"scriptingCommands setPreferredHand:");

	err = GetControlBySignature(kPreferredHandControlSignature, &theControl);
	if (err == noErr) {	
		if ( [newPreferredHand unsignedLongValue] == kLeftHandOSType ) {
		
			SetControlValue( theControl, kLeftHandControlValue );
			
		} else if ( [newPreferredHand unsignedLongValue] == kRightHandOSType ) {
		
			SetControlValue( theControl, kRightHandControlValue );
			
		} else {

				/* invalid input value */
			SetScriptingError( paramErr, @"setPreferredHand received an invalid value" );
			
		}
	} else {
		SetScriptingError( err, nil );
	}

}

/* element accessors:
- (NSArray*) graphics;
- (NSArray*) valueInGraphicsAtIndex:(int) index;
- (NSArray*) valueInGraphicsWithName:(NSString*) name;
- (NSArray*) valueInGraphicsWithUniqueID:(id) uniqueID;
*/

@end



