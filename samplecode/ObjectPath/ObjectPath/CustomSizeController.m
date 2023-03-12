/*

File: CustomSizeController.m

Abstract: Handles the custom size panel for changing the NSPathControl's dimensions.

Version: 1.0

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

Copyright © 2006-2007 Apple Inc. All Rights Reserved

*/

#import "CustomSizeController.h"

@implementation CustomSizeController

// -------------------------------------------------------------------------------
//	windowNibName
// -------------------------------------------------------------------------------
- (NSString*)windowNibName
{
	return @"CustomSize";
}

// -------------------------------------------------------------------------------
//	doCustomSize:startingSize:from
//
//	This is called from MyWindowController, asking to open this alert as a sheet
//	with a parent window.  Returns the desired size for the NSPathControl.
// -------------------------------------------------------------------------------
- (NSSize)doCustomSize:(NSSize)startingSize withParentWindow:parentWindow
{
	NSWindow* window = [self window];

	size = startingSize;
	cancelled = NO;

	// set the initial values
    [heightField setIntValue:size.height];
    [widthField  setIntValue:size.width];
    [[self window] makeFirstResponder:widthField];

	[NSApp beginSheet:window modalForWindow:parentWindow modalDelegate:nil didEndSelector:nil contextInfo:nil];
	[NSApp runModalForWindow:window];
	// sheet is up now...

	[NSApp endSheet:window];
	[window orderOut:self];

	return size;
}

// -------------------------------------------------------------------------------
//	done:sender
//
//	User clicked the OK button in this sheet.
//	This method carefully makes sure the width and height aren't too small by
//	preventing the sheet's dismissal.
// -------------------------------------------------------------------------------
- (IBAction)done:(id)sender
{
	BOOL validated = YES;

	// Save values
	size.height = [heightField intValue];
	size.width  = [widthField  intValue];

	if (size.height < 16)
	{
		validated = NO;
		[heightField setIntValue:16];
		[[self window] makeFirstResponder:heightField];
	}
	if (size.width < 16)
	{
		validated = NO;
		[widthField setIntValue:16];
		[[self window] makeFirstResponder:widthField];
	}

	if (validated)
		[NSApp stopModal];
}

// -------------------------------------------------------------------------------
//	cancel:sender
//
//	User clicked the Cancel button, dismiss the sheet.
// -------------------------------------------------------------------------------
- (IBAction)cancel:(id)sender
{
	[NSApp stopModal];
	cancelled = YES;
}

// -------------------------------------------------------------------------------
//	wasCancelled
//
//	Helps the controller to this sheet indicate if the user clicked Cancel.
// -------------------------------------------------------------------------------
- (BOOL)wasCancelled
{
	return cancelled;
}

@end