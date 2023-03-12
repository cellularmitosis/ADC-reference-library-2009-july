/*

File: SayHello.mm

Abstract:
		This is the implementation file for SayHello, a class that displays
		messages using different languages (Objective-C and C++) and runtime
		environments (Carbon and Cocoa).

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

Copyright © 2001-2007 Apple Inc. All Rights Reserved

*/

#import "SayHello.h"

#include "FooClass.h"
#include <Carbon/Carbon.h>


@implementation SayHello

//=============================================================================
// Objective-C call from Objective-C
//
- (void)message1:(id)sender
{
    // display an alert message on the screen
    NSRunAlertPanel(@"Regular Obj-C from Obj-C",@"Hello, World! This is a regular old NSRunAlertPanel call in Cocoa!",@"OK",NULL,NULL);
}


//=============================================================================
// This function shows that you can call C++ code from an Objective-C method when using
// the Objective-C++ compiler.
// FYI, if you need to pass a C++ object to a pure ObjC file (a .m file compiled by the regular
// ObjC compiler), you'll need to cast the object to a void *.
//
- (void)message2:(id)sender
{
    int howMany;
    NSString *theAnswer;
    Foo	* myCPlusPlusObj; //A C++ object
    
    //Create and use a new C++ object
    myCPlusPlusObj=new Foo();
    howMany=myCPlusPlusObj->getVariable();
    delete myCPlusPlusObj;
    //Here we put together an NSString that contains the value returned by fooGetVariable()
    theAnswer=[NSString stringWithFormat:@"Hello, World! When our C++ object is queried, it tells us that the number is %i!",howMany];
    //Display an alert message on the screen with the string generated above
    NSRunAlertPanel(@"C++ from Obj-C",theAnswer,@"OK",NULL,NULL);
}

//=============================================================================
static OSStatus appCommandHandler(EventHandlerCallRef inCallRef, EventRef inEvent, void* userData);

// This function shows how to call Carbon commands from Objective-C/Cocoa; simply include
// the proper headers and the Carbon frameworks in your project, and away you go!
//
- (void)message3:(id)sender
{
    // this calls into Carbon
	CFBundleRef bundleRef;
    IBNibRef    nibRef;
    OSStatus    err = noErr;
 
	bundleRef = CFBundleGetMainBundle();	// get our bundle reference of this app
	
	// get our "Carbon-based" nib file's reference
    err = CreateNibReferenceWithCFBundle(bundleRef, CFSTR("Hello"), &nibRef);
    if (err != noErr)
	{
		NSLog(@"failed to create carbon nib reference");
	}
	else
	{
		WindowRef window = nil;
		
		// load the "Alert" dialog from the nib file
		err = CreateWindowFromNib(nibRef, CFSTR("Alert"), &window);
		if (err != noErr)
		{
			NSLog(@"failed to create carbon window from nib");
		}
		else
		{
			// install the necessary Carbon Events to interace with the dialog
			EventTypeSpec cmdEvent = { kEventClassCommand, kEventCommandProcess };
			InstallApplicationEventHandler(NewEventHandlerUPP(appCommandHandler), GetEventTypeCount(cmdEvent), &cmdEvent, window, nil);
			ShowWindow(window);
			DisposeNibReference(nibRef);
		}
	}
}

// Carbon Event handler to manage our alert
static OSStatus appCommandHandler(EventHandlerCallRef inCallRef, EventRef inEvent, void* userData)
{
    OSStatus err = eventNotHandledErr;
	
    if (GetEventKind(inEvent) == kEventCommandProcess)
	{
        HICommand command;
		GetEventParameter(inEvent, kEventParamDirectObject, typeHICommand, NULL, sizeof(HICommand), NULL, &command);
        switch (command.commandID)
		{
            case kHICommandOK:
			{
				// OK button was pressed, remove the window
				DisposeWindow((WindowRef)userData);
                break;
			}
			
            default:
                break;
        }
    }

    return err;
}

@end
