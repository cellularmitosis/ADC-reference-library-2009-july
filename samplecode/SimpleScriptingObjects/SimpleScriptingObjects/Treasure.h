/*

File: Treasure.h

Abstract: declarations for the treasure class
for this example application.  Treasure is a subclass
of the Trinket class and it has some additional
properties.

Trinkets and treasures provide us with objects to
put inside of the Bucket and StrongBox container
objects.

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
#import "Trinket.h"


	/* We have implemented our 'metal' property as an
	AppleScript enumeration.  As such, each of the items in the
	enumeration is identified by a unique four character code
	stored in a long integer.
	The following enumeration lists those four character codes
	that we have defined in our scripting definition file.  We can
	use these codes to decode the enumeration values we receive
	from Cocoa */
enum {
	kTinMetal = 'PmTn',
	kPewterMetal = 'PmPw',
	kBronzeMetal = 'PmBr',
	kSilverMetal = 'PmSi',
	kGoldMetal = 'PmGo'
};


	/* The Treasure class adds a few new properties to the Trinket class.
	One point of interest in this class is the metal property that uses
	an AppleScript enumeration.  The decodeMetal class method illustrates
	how you can decode enumeration values and use them in your program.
	
	We take care of most of the 'infrastructure' type operations
	needed for scripting in the Element class (the superclass of the
	Trinket class), so all we have to worry about here is storage management
	and providing accessors for the properties.*/
@interface Treasure : Trinket {
	NSNumber *itemValue; /* storage for the 'value' AppleScript property */
	NSNumber *itemMetal; /* storage for the 'metal' AppleScript property */
}

	/* for decoding AppleScript enumeration values */
+ (NSString*) decodeMetal:(NSNumber*) metal;

	/* storage management */
-(id) init;
- (void) dealloc;

	/* kvc methods for the 'value' AppleScript property */
- (NSNumber *)value;
- (void)setValue:(NSNumber *)value;

	/* kvc methods for the 'metal' AppleScript property */
- (NSNumber *)metal;
- (void)setMetal:(NSNumber *)value;


@end
