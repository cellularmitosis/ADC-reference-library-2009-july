/*
	Copyright: 	© Copyright 2006 Apple Computer, Inc. All rights reserved.
 
	Disclaimer:	IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc.
	("Apple") in consideration of your agreement to the following terms, and your
	use, installation, modification or redistribution of this Apple software
	constitutes acceptance of these terms.  If you do not agree with these terms,
	please do not use, install, modify or redistribute this Apple software.
 
	In consideration of your agreement to abide by the following terms, and subject
	to these terms, Apple grants you a personal, non-exclusive license, under Apple’s
	copyrights in this original Apple software (the "Apple Software"), to use,
	reproduce, modify and redistribute the Apple Software, with or without
	modifications, in source and/or binary forms; provided that if you redistribute
	the Apple Software in its entirety and without modifications, you must retain
	this notice and the following text and disclaimers in all such redistributions of
	the Apple Software.  Neither the name, trademarks, service marks or logos of
	Apple Computer, Inc. may be used to endorse or promote products derived from the
	Apple Software without specific prior written permission from Apple.  Except as
	expressly stated in this notice, no other rights or licenses, express or implied,
	are granted by Apple herein, including but not limited to any patent rights that
	may be infringed by your derivative works or by other works in which the Apple
	Software may be incorporated.
 
	The Apple Software is provided by Apple on an "AS IS" basis.  APPLE MAKES NO
	WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED
	WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS FOR A PARTICULAR
	PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND OPERATION ALONE OR IN
	COMBINATION WITH YOUR PRODUCTS.
 
	IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL OR
	CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
						   GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
	ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION, MODIFICATION AND/OR DISTRIBUTION
	OF THE APPLE SOFTWARE, HOWEVER CAUSED AND WHETHER UNDER THEORY OF CONTRACT, TORT
	(INCLUDING NEGLIGENCE), STRICT LIABILITY OR OTHERWISE, EVEN IF APPLE HAS BEEN
	ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
//
//  AppController.m
//

#import "AppController.h"


@implementation AppController

-(id)init {
	self = [super init];
	if (self) 
	{
		// send the init event to the mouseInterface
		mouseInterface = [[MouseInterface alloc] init];
	}	
	return self;
}

-(void) dealloc {
	// release the mouseInterface
	[mouseInterface release];

	[super dealloc];
}

-(void)awakeFromNib {
	
	//store the original value to restore
	origValue = [mouseInterface mouseDefaultSpeed];

	//promote original values into View
	[mySlider setFloatValue: origValue];
	[textField setFloatValue: origValue];
}

/*
 setMouseSpeedAsDefault - user wants to set the currently selected acceleration value as the 
 default - when the application is terminated, this acceleration value will be set. is used to restore the setting when the system completes.
 */
-(IBAction)setMouseSpeedAsDefault:(id)sender{
	origValue = [mySlider floatValue];
}


-(IBAction)updateMouseSpeed:(id)sender {
	// read the current mouse value from the slider.
	float value = [mySlider floatValue];
	// set the text value of the mouse setting in the window
	[textField setFloatValue:value];
	// make call to actually set this value as the mouse speed
	[mouseInterface setMouseSpeed:value];

}

/*
 incrementMouseSpeed - used when the "faster" button is hit. increment the speed setting
 to the next .1 amount. e.g 1.45 -> 1.5
*/
-(IBAction)incrementMouseSpeed:(id)sender
{
	float value = [mySlider floatValue] * 10 + kStandardIncrement + 0.1;
	int	  newValue = value;
	value = newValue;
	value = value / 10;	// cannot use value = newValue / 10, since (newValue / 10) will force integer math
	if (value > kMaxSpeedSetting)
		value = kMaxSpeedSetting;
	[mySlider setFloatValue:value];
	[textField setFloatValue:value];
	[mouseInterface setMouseSpeed:value];
}

/*
 decrementMouseSpeed - used when the "slower" button is hit. decrement the speed setting
 to the next .1 amount. e.g .76 -> .7
 */

-(IBAction)decrementMouseSpeed:(id)sender
{
	float value = [mySlider floatValue] * 10 - kStandardIncrement + 0.1;
	int	  newValue = value;
	if (newValue < 0)
		newValue = 0;
	value = newValue;
	value = value / 10;	// cannot use value = newValue / 10, since (newValue / 10) will force integer math
	[mySlider setFloatValue:value];
	[textField setFloatValue:value];
	[mouseInterface setMouseSpeed:value];
}

/*
 applicationWillTerminate - Delegate method, called when the application is about to be terminated
 used to restore the mouse acceleration setting to whatever is in "origValue"
*/
-(void)applicationWillTerminate:(NSNotification *)aNotification {

	[mouseInterface setMouseSpeed:origValue];
}

/*
 applicationWillResignActive - Delegate method, called when the application is about to become inactive -
 so as to restore the mouse acceleration setting to whatever is in "origValue"
 */
-(void)applicationWillResignActive:(NSNotification *)aNotification {
	
	[mouseInterface setMouseSpeed:origValue];
}
/*
 applicationWillBecomeActive - Delegate method, called when the application is about to become active -
 so as to restore the mouse acceleration setting to whatever is in slider
 */
-(void)applicationWillBecomeActive:(NSNotification *)aNotification {
	// read the current mouse value from the slider.
	float value = [mySlider floatValue];
	// set the text value of the mouse setting in the window
	[textField setFloatValue:value];
	// make call to actually set this value as the mouse speed
	[mouseInterface setMouseSpeed:value];
}

/* 
	quit the app if the window is closed.
*/
- (BOOL)windowShouldClose:(id)sender
{
	[NSApp terminate:nil];
	return YES;
}
@end
