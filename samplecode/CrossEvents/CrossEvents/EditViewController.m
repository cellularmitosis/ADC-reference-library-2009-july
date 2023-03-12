/*

File: EventViewController.m

Abstract: Source for maintaining the NSTextField inside the HICocoaView.

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

Copyright © 2007 Apple Inc. All Rights Reserved

*/

#import "EditViewController.h"
#import "common.h"

@implementation EditViewController

// -------------------------------------------------------------------------------
//	MyEventHandler
//
//	This is our event handler that captures the custom Carbon Event from the
//	Carbon side.
// -------------------------------------------------------------------------------
OSStatus MyEventHandler(EventHandlerCallRef nextHandler, EventRef inEvent, void* userData)
{
	OSStatus status = eventNotHandledErr;
	NSTextField* targetEditField = (NSTextField*)userData;
	
	if (targetEditField != nil)
	{
		switch (GetEventClass(inEvent))
		{
			case kEventClassCustom:
			{
				switch (GetEventKind(inEvent))
				{
					case kEventCustomCharTyped:
					{
						UniChar mUnicodeText[255];
						UInt32 bytecount = 0;
						verify_noerr(GetEventParameter(inEvent, kEventParamKeyUnicodes, typeUnicodeText, NULL, sizeof(mUnicodeText), &bytecount, (char*)&mUnicodeText));
						
						NSString* finalStr = [NSString stringWithCharacters:mUnicodeText length:bytecount/2];
						[targetEditField setStringValue: finalStr];
						break;
					}
				}
				break;
			}
		}
	}
	
	return status;
}

// -------------------------------------------------------------------------------
//	awakeFromNib:
// -------------------------------------------------------------------------------
- (void)awakeFromNib
{
	// install our event handler to listen for characters typed
	static const EventTypeSpec kMyEvents[] = { { kEventClassCustom, kEventCustomCharTyped } };
	InstallEventHandler(GetApplicationEventTarget(), NewEventHandlerUPP(MyEventHandler), GetEventTypeCount(kMyEvents), kMyEvents, (void*)editField, NULL);
}


NSString *CustomEventNotification = @"CustomEventNotification";

// -------------------------------------------------------------------------------
//	controlTextDidChange:
//
//	The object in our notification if the NSTextField.
//	Forward this "text changed" event to the Carbon side using NSNotification.
// -------------------------------------------------------------------------------
- (void)controlTextDidChange:(NSNotification*)notification
{
	NSTextField* notif_obj = [notification object];
	NSString* newStr = [notif_obj stringValue];

	// tell our Carbon Window and HITextView that the text in NSTextField has changed:
	NSDictionary* notifInfo = [NSDictionary dictionaryWithObject:newStr forKey:@"changedText"];
	[[NSNotificationCenter defaultCenter] postNotificationName:CustomEventNotification object:nil userInfo:notifInfo];
}

@end
