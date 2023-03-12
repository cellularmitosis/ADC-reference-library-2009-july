/*

File: PlaybackTime.m

Abstract: Implementation file for the playback timer class in CocoaDVDPlayer, an
ADC Reference Library sample project.

Version: 1.0

IMPORTANT:	This Apple software is supplied to you by Apple Computer, Inc.
("Apple") in consideration of your agreement to the following terms, and your
use, installation, modification or redistribution of this Apple software
constitutes acceptance of these terms.	If you do not agree with these terms,
please do not use, install, modify or redistribute this Apple software.

In consideration of your agreement to abide by the following terms, and subject
to these terms, Apple grants you a personal, non-exclusive license, under
Apple's copyrights in this original Apple software (the "Apple Software"), to
use, reproduce, modify and redistribute the Apple Software, with or without
modifications, in source and/or binary forms; provided that if you redistribute
the Apple Software in its entirety and without modifications, you must retain
this notice and the following text and disclaimers in all such redistributions
of the Apple Software. Neither the name, trademarks, service marks or logos of
Apple Computer, Inc. may be used to endorse or promote products derived from the
Apple Software without specific prior written permission from Apple.  Except as
expressly stated in this notice, no other rights or licenses, express or
implied, are granted by Apple herein, including but not limited to any patent
rights that may be infringed by your derivative works or by other works in which
the Apple Software may be incorporated.

The Apple Software is provided by Apple on an "AS IS" basis.  APPLE MAKES NO
WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED
WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS FOR A PARTICULAR
PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND OPERATION ALONE OR IN
COMBINATION WITH YOUR PRODUCTS.

IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION, MODIFICATION AND/OR
DISTRIBUTION OF THE APPLE SOFTWARE, HOWEVER CAUSED AND WHETHER UNDER THEORY OF
CONTRACT, TORT (INCLUDING NEGLIGENCE), STRICT LIABILITY OR OTHERWISE, EVEN IF
APPLE HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

Copyright � 2005 Apple Computer, Inc., All Rights Reserved

*/ 

#import "PlaybackTime.h"


/*
********************************************************************************
**
**		Class: PlaybackTime
**
********************************************************************************
*/

/* This method is used inside this file only. Instead of declaring it in
PlaybackTime.h, we declare them here in a category that extends the class. */

@interface PlaybackTime (InternalMethods)
- (void) setTimeText;
@end


@implementation PlaybackTime

- (id) init
{
	[super init];

	/* default is to display elapsed time */
	timeBase = kTimeBaseElapsed;

	return self;
}


/* When the user clicks the mouse in the playback time field, this method
responds by toggling the time base between elapsed and remaining time. */

- (void) mouseDown: (NSEvent *)event
{
	if (timeBase == kTimeBaseElapsed) {
		timeBase = kTimeBaseRemaining;
	}
	else {
		timeBase = kTimeBaseElapsed;
	}

	[self setTimeText];
	[super mouseDown:event];
}


/* This method is used by the Controller object during playback to update the
elapsed and remaining time, in response to time events. */

- (void) setTimeElapsed: (int)inElapsed timeRemaining: (int)inRemaining
{ 
	timeElapsed = inElapsed;
	timeRemaining = inRemaining;
	[self setTimeText];
}


/* This method updates the text used to display the time in the control
window. */

- (void) setTimeText
{
	UInt32 time, hours, mins, secs;
	NSString *format;

	if (timeBase == kTimeBaseElapsed) {
		time = (timeElapsed + 500) / 1000;
		format = @"%02u:%02u:%02u";
	}
	else {
		time = (timeRemaining + 500) / 1000;
		format = @"-%02u:%02u:%02u";
	}

	hours = time / 3600;
	time -= hours * 3600;
	mins = time / 60;
	secs = time % 60;

	NSString *timeString = [[NSString alloc] initWithFormat:format, hours, mins, secs];
	[self setStringValue:timeString];
	[timeString release];

	[self setNeedsDisplay];
}

@end
