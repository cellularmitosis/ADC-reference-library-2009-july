//
// File:	   ChildEditController.m
//
// Abstract:   Controller object for the edit sheet panel.
//
// Version:    1.0
//
// Disclaimer: IMPORTANT:  This Apple software is supplied to you by Apple Inc. ("Apple")
//             in consideration of your agreement to the following terms, and your use,
//             installation, modification or redistribution of this Apple software
//             constitutes acceptance of these terms.  If you do not agree with these
//             terms, please do not use, install, modify or redistribute this Apple
//             software.
//
//             In consideration of your agreement to abide by the following terms, and
//             subject to these terms, Apple grants you a personal, non - exclusive
//             license, under Apple's copyrights in this original Apple software ( the
//             "Apple Software" ), to use, reproduce, modify and redistribute the Apple
//             Software, with or without modifications, in source and / or binary forms;
//             provided that if you redistribute the Apple Software in its entirety and
//             without modifications, you must retain this notice and the following text
//             and disclaimers in all such redistributions of the Apple Software. Neither
//             the name, trademarks, service marks or logos of Apple Inc. may be used to
//             endorse or promote products derived from the Apple Software without specific
//             prior written permission from Apple.  Except as expressly stated in this
//             notice, no other rights or licenses, express or implied, are granted by
//             Apple herein, including but not limited to any patent rights that may be
//             infringed by your derivative works or by other works in which the Apple
//             Software may be incorporated.
//
//             The Apple Software is provided by Apple on an "AS IS" basis.  APPLE MAKES NO
//             WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED
//             WARRANTIES OF NON - INFRINGEMENT, MERCHANTABILITY AND FITNESS FOR A
//             PARTICULAR PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND OPERATION
//             ALONE OR IN COMBINATION WITH YOUR PRODUCTS.
//
//             IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL OR
//             CONSEQUENTIAL DAMAGES ( INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
//             SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
//             INTERRUPTION ) ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION, MODIFICATION
//             AND / OR DISTRIBUTION OF THE APPLE SOFTWARE, HOWEVER CAUSED AND WHETHER
//             UNDER THEORY OF CONTRACT, TORT ( INCLUDING NEGLIGENCE ), STRICT LIABILITY OR
//             OTHERWISE, EVEN IF APPLE HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// Copyright (C) 2007 Apple Inc. All Rights Reserved.
//

#import "ChildEditController.h"

@implementation ChildEditController

// -------------------------------------------------------------------------------
//	init:
// -------------------------------------------------------------------------------
- (id)init
{
	self = [super init];
	return self;
}

// -------------------------------------------------------------------------------
//	windowNibName:
// -------------------------------------------------------------------------------
- (NSString*)windowNibName
{
	return @"ChildEdit";
}

// -------------------------------------------------------------------------------
//	dealloc:
// -------------------------------------------------------------------------------
- (void)dealloc
{
	[super dealloc];
	[savedFields release];
}

// -------------------------------------------------------------------------------
//	edit:startingValues:from
// -------------------------------------------------------------------------------
- (NSMutableDictionary*)edit:(NSDictionary*)startingValues from:(MyWindowController*)sender
{
	NSWindow* window = [self window];

	cancelled = NO;

	NSArray* editFields = [editForm cells];
	if (startingValues != nil)
	{
		// we are editing current entry, use its values as the default
		savedFields = [startingValues retain];

		[[editFields objectAtIndex:0] setStringValue:[startingValues objectForKey:@"name"]];
		[[editFields objectAtIndex:1] setStringValue:[startingValues objectForKey:@"url"]];
	}
	else
	{
		// we are adding a new entry,
		// make sure the form fields are empty due to the fact that this controller is recycled
		// each time the user opens the sheet -
		[[editFields objectAtIndex:0] setStringValue:@""];
		[[editFields objectAtIndex:1] setStringValue:@""];
	}
	
	[NSApp beginSheet:window modalForWindow:[sender window] modalDelegate:nil didEndSelector:nil contextInfo:nil];
	[NSApp runModalForWindow:window];
	// sheet is up here...

	[NSApp endSheet:window];
	[window orderOut:self];

	return savedFields;
}

// -------------------------------------------------------------------------------
//	done:sender
// -------------------------------------------------------------------------------
- (IBAction)done:(id)sender
{
	NSArray* editFields = [editForm cells];
	if ([[[editFields objectAtIndex:1] stringValue] length] == 0)
	{
		// you must provide a URL
		NSBeep();
		return;
	}
	
	// save the values for later
	[savedFields release];
	
	NSString* urlStr;
	if (![[[editFields objectAtIndex:1] stringValue] hasPrefix:@"http://"])
	{
		urlStr = [NSString stringWithFormat:@"http://%@", [[editFields objectAtIndex:1] stringValue]];
	}
	else
	{
		urlStr = [[editFields objectAtIndex:1] stringValue];
	}
	savedFields = [NSMutableDictionary dictionaryWithObjectsAndKeys:
							 [[editFields objectAtIndex:0] stringValue], @"name",
							 urlStr, @"url",
							 nil];
	[savedFields retain];
	
	[NSApp stopModal];
}

// -------------------------------------------------------------------------------
//	cancel:sender
// -------------------------------------------------------------------------------
- (IBAction)cancel:(id)sender
{
	[NSApp stopModal];
	cancelled = YES;
}

// -------------------------------------------------------------------------------
//	wasCancelled:
// -------------------------------------------------------------------------------
- (BOOL)wasCancelled
{
	return cancelled;
}

@end