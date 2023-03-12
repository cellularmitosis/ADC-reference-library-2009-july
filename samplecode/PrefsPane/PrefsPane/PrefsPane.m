/*

File: PrefsPane.m

Abstract: Main source file for this System Preference pane sample.

Version: 1.0

Disclaimer: IMPORTANT:  This Apple software is supplied to you by Apple Inc.
("Apple") in consideration of your agreement to the
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

#import "PrefsPane.h"

@implementation PrefsPane

// view tags of our prefs UI (determined in the nib)
enum {
	kEditControlTag		= 1,
	kWarningCheckTag	= 2,
	kAboutButtonTag		= 3
};

// -------------------------------------------------------------------------------
//	initWithBundle:bundle
//
//	The preference pane is being initialized, remember our bundle ID for later.
// -------------------------------------------------------------------------------
-(id)initWithBundle:(NSBundle*)bundle
{
	if ((self = [super initWithBundle:bundle]) != nil)
	{
		// do more initialization here
	}
	
	return self;
}

// -------------------------------------------------------------------------------
//	assignMainView
//
//  Set the main view of the preference pane before it returns.
// -------------------------------------------------------------------------------
- (void)assignMainView
{
	NSView* mainViewToUse = nil;
	
	// check system version number to decide which NSView layout to use
	long osVersion = 0;
	Gestalt(gestaltSystemVersion, &osVersion);
	if (osVersion < 0x1050)
	{
		mainViewToUse = mainView_10_4_or_earlier;
	}
	else
	{
		mainViewToUse = mainView_10_5_or_later;
	}
	[self setMainView: mainViewToUse];
	
	// since we are dealing with two possible NSViews to manage our UI,
	// we need to programtically set the target and actions here,
	// rather than using established connections in the nib.
	//
	NSTextField* editField = [mainViewToUse viewWithTag:kEditControlTag];
	if (editField != nil)
	{
		[editField setTarget:self];
		[editField setAction:@selector(editfieldAction:)];
	}
	NSButton* checkBox = [mainViewToUse viewWithTag:kWarningCheckTag];
	if (checkBox != nil)
	{
		[checkBox setTarget:self];
		[checkBox setAction:@selector(checkboxAction:)];
	}
	NSButton* aboutButton = [mainViewToUse viewWithTag:kAboutButtonTag];
	if (aboutButton != nil)
	{
		[aboutButton setTarget:self];
		[aboutButton setAction:@selector(aboutAction:)];
	}
	
	// set the proper key view ordering
	[self setInitialKeyView:checkBox];
	[self setFirstKeyView:checkBox];
	[self setLastKeyView:aboutButton];
}

// -------------------------------------------------------------------------------
//	mainViewDidLoad
//
//  mainViewDidLoad is invoked by the default implementation of loadMainView
//  after the main nib file has been loaded and the main view of the preference
//  pane has been set.  The default implementation does nothing.  Override
//  this method to perform any setup that must be performed after the main
//  nib file has been loaded and the main view has been set.
// -------------------------------------------------------------------------------
- (void)mainViewDidLoad
{
	NSUserDefaults* standardUserDefaults = [NSUserDefaults standardUserDefaults];
	if (standardUserDefaults) 
	{
		NSString* strValue = [standardUserDefaults objectForKey:@"editText"];
		if (strValue != nil)
			[[[self mainView] viewWithTag:kEditControlTag] setStringValue: strValue];
		
		NSNumber* switchValue = [standardUserDefaults objectForKey:@"switch"];
		if (switchValue != nil)
			[[[self mainView] viewWithTag:kWarningCheckTag] setState: [switchValue intValue]];
	}
}

// -------------------------------------------------------------------------------
//	mainNibName
//
//  mainNibName is invoked by the default implementation of loadMainView.
//
//  The default implementation returns the value of the NSMainNibFile key
//  in the bundle's Info plist. You can override this if you want to
//  dynamically select the main nib file for this preference.
//
//  The value returned must NOT include the ".nib" extension.
// -------------------------------------------------------------------------------
- (NSString*)mainNibName
{
	// here you can decide to use an alternate nib file for different
	// system versions if you are so inclinded, but in our case we use one nib,
	// and two alternate views to present our UI.
	//
	return @"PrefsPane";
}

// -------------------------------------------------------------------------------
//	aboutAction:sender
//
//	This method is called when the about button is clicked.
// -------------------------------------------------------------------------------
- (IBAction)aboutAction:(id)sender
{
	[NSApp beginSheet:[aboutWind window] modalForWindow:[[self mainView] window] modalDelegate:self didEndSelector:@selector(aboutSheetDidEnd:returnCode:contextInfo:) contextInfo:nil];
}

// -------------------------------------------------------------------------------
//	checkboxAction:
//
//	This method is called when the checkbox is checked/unchecked.
// -------------------------------------------------------------------------------
- (IBAction)checkboxAction:(id)sender
{
	// store the current checkbox value to our user defaults
	[[NSUserDefaults standardUserDefaults] setObject:[NSNumber numberWithInt: [sender state]] forKey:@"switch"];
}

// -------------------------------------------------------------------------------
//	editfieldAction:
//
//	This method is called when editing is committed/finished in our edit field.
// -------------------------------------------------------------------------------
- (IBAction)editfieldAction:(id)sender
{
	// store the current edit field value to our user defaults
	[[NSUserDefaults standardUserDefaults] setObject:[sender stringValue] forKey:@"editText"];
}

// -------------------------------------------------------------------------------
//	aboutSheetDidEnd:sheet:returnCode:contextInfo
//
//	The about sheet has ended.
// -------------------------------------------------------------------------------
- (void)aboutSheetDidEnd:(NSWindow*)sheet returnCode:(int)returnCode contextInfo:(void *)contextInfo
{
	[sheet orderOut:self];
}

// -------------------------------------------------------------------------------
//	aboutDoneAction:sender
//
//	The user dismissed the about window sheet.
// -------------------------------------------------------------------------------
- (IBAction)aboutDoneAction:(id)sender
{
	[NSApp endSheet:[aboutWind window]];
}

#pragma mark - delegate methods

// -------------------------------------------------------------------------------
//	willSelect:
// -------------------------------------------------------------------------------
- (void)willSelect
{
	NSLog(@"Our prefs panel is about to be selected.");
}

// -------------------------------------------------------------------------------
//	didSelect:
// -------------------------------------------------------------------------------
- (void)didSelect
{
	NSLog(@"Our prefs panel is selected.");
}

// -------------------------------------------------------------------------------
//	willUnselect:
// -------------------------------------------------------------------------------
- (void)willUnselect
{
	NSLog(@"Our prefs panel is about to be un-selected.");
}

// -------------------------------------------------------------------------------
//	didUnselect:
// -------------------------------------------------------------------------------
- (void)didUnselect
{
	NSLog(@"Our prefs panel is now un-selected.");
}

// -------------------------------------------------------------------------------
//	confirmSheetDidEnd:sheet:returnCode:contextInfo
// -------------------------------------------------------------------------------
- (void)confirmSheetDidEnd:(NSWindow*)sheet returnCode:(int)returnCode contextInfo:(void *)contextInfo
{
	[sheet orderOut:self];	// hide the sheet

	// decide how we want to unselect
	if (returnCode == NSAlertDefaultReturn)
		[self replyToShouldUnselect:NSUnselectNow];
	else
		[self replyToShouldUnselect:NSUnselectCancel];
}

// -------------------------------------------------------------------------------
//	shouldUnselect:
//
//	Delegate method to possibly block the unselection of this prefs pane.
// -------------------------------------------------------------------------------
- (NSPreferencePaneUnselectReply)shouldUnselect
{
	NSPreferencePaneUnselectReply result = NSUnselectNow;
	
	if ([[[[self mainView] viewWithTag:kWarningCheckTag] cell] state])	// confirm the dismissal if the checkbox is checked
	{
		// normally we return "NSUnslectNow", but since we are opening a sheet,
		// we need to return "NSUnselectLater" and have our "confirmSheetDidEnd" selector perform the dismissal decision
		//
		NSBeginAlertSheet(	@"You are about to exit this preference pane.",
							@"Yes",
							@"No",
							nil,
							[[self mainView] window],
							self,
							nil,
							@selector(confirmSheetDidEnd:returnCode:contextInfo:),
							nil,
							@"Are you sure you want to exit?");
		result = NSUnselectLater;
	}
	
	return result;
}

@end
