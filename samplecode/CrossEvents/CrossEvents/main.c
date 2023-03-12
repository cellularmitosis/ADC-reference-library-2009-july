/*

File: main.c

Abstract: Main source for this sample project.

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

#include <Carbon/Carbon.h>

#include "EditViewController.h"
#include "common.h"


static OSStatus AppEventHandler(EventHandlerCallRef inCaller, EventRef inEvent, void* inRefcon);
static OSStatus HandleNew(void);
static OSStatus WindowEventHandler(EventHandlerCallRef inCaller, EventRef inEvent, void* inRefcon);

HIViewRef gEditField;
static IBNibRef sNibRef;


//--------------------------------------------------------------------------------------------
// Since NSNotifications work between Objective-C objects, we need to establish a "listener"
// object to receive notifications from the Cocoa side (from our EditViewController object).
//
@interface ourObserver : NSObject
{}
@end

@implementation ourObserver
-(void)textChanged:(NSNotification*)note
{
	verify(gEditField);
	HIViewSetText(gEditField, (CFStringRef)[[note userInfo] objectForKey:@"changedText"]);
}
@end

ourObserver* observer;


//--------------------------------------------------------------------------------------------
#define kCarbonEdit	'carb'	// signature for the HITextView control
#define kCocoaEdit	'coca'	// signature of the HICocoaView that embeds the NSTextField

const HIViewID kCarbonEditID	= { kCarbonEdit, 1 };	// HITextView control
const HIViewID kCocoaEditID		= { kCocoaEdit, 2 };	// HICocoaView control


//--------------------------------------------------------------------------------------------
int main(int argc, char* argv[])
{
	OSStatus err = noErr;
    static const EventTypeSpec kAppEvents[] = { { kEventClassCommand, kEventCommandProcess } };

    // Create a Nib reference, passing the name of the nib file (without the .nib extension).
    // CreateNibReference only searches into the application bundle.
    err = CreateNibReference(CFSTR("main"), &sNibRef);
    require_noerr(err, CantGetNibRef);
    
    // Once the nib reference is created, set the menu bar. "MainMenu" is the name of the menu bar
    // object. This name is set in InterfaceBuilder when the nib is created.
    err = SetMenuBarFromNib(sNibRef, CFSTR("MenuBar"));
    require_noerr(err, CantSetMenuBar);
    
    // Install our handler for common commands on the application target
    InstallApplicationEventHandler(NewEventHandlerUPP(AppEventHandler), GetEventTypeCount(kAppEvents), kAppEvents, 0, NULL);
    
	NSApplicationLoad();	// needed for Carbon based applications which call into Cocoa

	NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];	// make sure to setup our own autorelease pool as a Carbon app
	
	observer = [[ourObserver alloc] init];
    [[NSNotificationCenter defaultCenter] addObserver:observer selector:@selector(textChanged:) name:CustomEventNotification object:nil];
	
	HandleNew();	// create our window
    
	[pool release];
	
    RunApplicationEventLoop();   // run the event loop

	[[NSNotificationCenter defaultCenter] removeObserver:observer];
	[observer release];
	
CantSetMenuBar:
CantGetNibRef:
    return err;
}

//--------------------------------------------------------------------------------------------
static OSStatus AppEventHandler(EventHandlerCallRef inCaller, EventRef inEvent, void* inRefcon)
{
    OSStatus result = eventNotHandledErr;
    
    switch (GetEventClass(inEvent))
    {
        case kEventClassCommand:
        {
            HICommandExtended cmd;
            verify_noerr(GetEventParameter(inEvent, kEventParamDirectObject, typeHICommand, NULL, sizeof(cmd), NULL, &cmd));
            
            switch (GetEventKind(inEvent))
            {
                case kEventCommandProcess:
					switch (cmd.commandID)
                    {
                        case kHICommandNew:
                            result = HandleNew();
                            break;
                            
                        default:
                            break;
                    }
                    break;
            }
            break;
        }
            
        default:
            break;
    }
    
    return result;
}

//--------------------------------------------------------------------------------------------
DEFINE_ONE_SHOT_HANDLER_GETTER(WindowEventHandler)

//--------------------------------------------------------------------------------------------
static OSStatus HandleNew()
{
    OSStatus err = noErr;
    WindowRef window;
    static const EventTypeSpec kWindowEvents[] = { { kEventClassKeyboard, kEventRawKeyDown } };
    
    // Create a window. "MainWindow" is the name of the window object. This name is set in 
    // InterfaceBuilder when the nib is created.
    err = CreateWindowFromNib(sNibRef, CFSTR("MainWindow"), &window);
    require_noerr(err, CantCreateWindow);

    // Install a command handler on the window. We don't use this handler yet, but nearly all
    // Carbon apps will need to handle commands, so this saves everyone a little typing.
    InstallWindowEventHandler(window, GetWindowEventHandlerUPP(), GetEventTypeCount(kWindowEvents), kWindowEvents, window, NULL);

	// setup our HIView to contain the Cocoa WebKit view
	// here we will use NSViewController to gain access to our nib-based NSView.
	//
	EditViewController* viewController = [[EditViewController alloc] initWithNibName:@"EditView" bundle:nil];
	SetWRefCon(window, (SRefCon)viewController);	// used for view controller disposal when the window is closed
	
	HIViewRef view;
	err = HIViewFindByID(HIViewGetRoot(window), kCocoaEditID, &view);
	NSView* cocoaView = [viewController view];			// get a reference to our NSView to set in our HICocoaView
	if (cocoaView != NULL)
		err = HICocoaViewSetView(view, cocoaView);	// embed our NSView within our HICocoaView

	err = HIViewFindByID(HIViewGetRoot(window), kCarbonEditID, &gEditField);
		
    // The window was created hidden, so show it
    ShowWindow(window);
    
CantCreateWindow:
    return err;
}

//--------------------------------------------------------------------------------------------
static OSStatus WindowEventHandler(EventHandlerCallRef inCaller, EventRef inEvent, void* inRefcon)
{
    OSStatus err = eventNotHandledErr;
    
    switch (GetEventClass(inEvent))
    {
		case kEventClassKeyboard:
		{
			switch (GetEventKind(inEvent))
			{
				case kEventRawKeyDown:
				{
					// make sure the key event goes through so we can fetch the changed text
					err = CallNextEventHandler(inCaller, inEvent);
					
					CFStringRef changedText = NULL;
					changedText = HIViewCopyText(gEditField);
					if (changedText != NULL)
					{
						// package up the Carbon Event and include the text from the HITextView
						EventRef evt;
						if (CreateEvent(NULL, kEventClassCustom, kEventCustomCharTyped, 0, kEventAttributeNone, &evt) == noErr)
						{
							CFIndex strSize = CFStringGetLength(changedText) * sizeof(UniChar);
							
							UniChar* buffer = NULL;
							buffer = malloc(strSize * sizeof(UniChar));
							if (buffer != NULL)
							{
								CFStringGetCharacters(changedText, CFRangeMake(0, strSize), buffer);
								SetEventParameter(evt, kEventParamKeyUnicodes, typeUnicodeText, strSize, buffer);
								
								// send our custom CarbonEvent to our Cocoa NSViewCotroller that the text has changed
								PostEventToQueue(GetMainEventQueue(), evt, kEventPriorityStandard);
								
								free(buffer);
							}
						}
						CFRelease(changedText);
					}
					break;
				}
			}
			break;
		}
		
        default:
            break;
    }
    
    return err;
}
