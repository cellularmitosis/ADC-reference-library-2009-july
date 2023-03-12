/*

File: HelloController.m

Abstract: This is the implementation file for the class that handles the interaction between the GUI
			and the Hello objects, allowing you to switch messages and receiver objects.  Calling Help
			Viewer to display our help file is also handled here.

Version: 1.3

Disclaimer: IMPORTANT:  This Apple software is supplied to you by Apple
Inc. ("Apple") in consideration of your agreement to the
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

Copyright © 2000-2007 Apple Inc. All Rights Reserved

*/

// we use #import to make sure the header is only read in once
#import "HelloController.h"

@implementation HelloController

// This method is called when the user picks a different message 
// to display by clicking a different radio button
- (IBAction)switchMessage:(id)sender
{
	// sender is the NSMatrix containing the radio buttons.
	// We ask the sender for which row (radio button) is selected and add one
	// to compensate for counting from zero.
	int which = [sender selectedRow] + 1;
	
	// We now set our NSButton's action to be the message corresponding to the radio button selection.
	// +[NSString stringWithFormat:...] is used to concatenate "message" and the message number.  
	// NSSelectorFromString converts the message name string to an actual message structure that
	// Objective-C can use.
	[helloButton setAction:NSSelectorFromString([NSString stringWithFormat:@"%@%d:", @"message", which])];        
}

// This method is called when the user picks a different object to 
// receive messages using the PopUp menu
- (IBAction)switchObject:(id)sender
{
	// sender is the NSPopUpMenu containing Hello object choices.
	// We ask the sender for which menu item is selected and add one
	// to compensate for counting from zero.
	int which = [sender indexOfSelectedItem] + 1;

	// Based on which menu item is selected, we set the target (the receiving object)
	// of the helloButton to point to either hello1 or hello2.
	if (which == 1)
		[helloButton setTarget:hello1];
	else
		[helloButton setTarget:hello2];
}

// awakeFromNib is called when this object is done being unpacked from the nib file;
// at this point, we can do any needed initialization before turning app control over to the user
- (void)awakeFromNib
{
    // We don't actually need to do anything here, so it's empty
}

// Handling the Help menu:
// -----------------------
// It used to be that we had a routine here that opened our ReadMe.html help file using Help Viewer via
// NSWorkspace's -openFile:withApplication:, but now we do things the "modern" way.  This means that we
// let Cocoa handle things automatically for us.  The only thing we need is a folder containing our help
// files that sits in the Resources folder inside the SimpleCocoaApp bundle, a new meta tag in our help title
// page (the "AppleTitle" tag), and a few Info.plist keys (see the "Expert" view of Targets -> Application Settings).
// The two Info.plist keys needed are CFBundleHelpBookFolder and CFBundleHelpBookName.  Once that is done,
// Help Viewer will automatically open the Help page when the Help menu item is selected, etc.  No code is needed!

@end
