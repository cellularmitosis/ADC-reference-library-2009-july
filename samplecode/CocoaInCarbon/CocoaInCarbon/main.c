 /*
	File:		main.c


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

	Copyright © 2002-2004 Apple Computer, Inc., All Rights Reserved
*/
#include <Carbon/Carbon.h>
#include "CocoaStuff.h"

enum
{
    kOpenCocoaWindow = 'COCO'
};

static OSStatus 
handleCommand(int commandID) {
    OSStatus osStatus = noErr;
    if (commandID == kEventButtonPressed) {

        osStatus = changeText(CFSTR("button pressed!"));
        require_noerr(osStatus, CantCallFunction);
    }
CantCallFunction:
    return osStatus;
}


static OSStatus
appCommandHandler(EventHandlerCallRef inCallRef, EventRef inEvent, void* userData) {
    HICommand command;
    OSStatus err = eventNotHandledErr;

    
    if (GetEventKind(inEvent) == kEventCommandProcess) {
        GetEventParameter( inEvent, kEventParamDirectObject, typeHICommand, NULL, sizeof(HICommand), NULL, &command );
        switch ( command.commandID ) {
            case kOpenCocoaWindow:
                               
                // call function to initialize Cocoa
                err = initializeCocoa(handleCommand);
                require_noerr(err, CantInitializeCocoa);
                
                // call function to show window
                err = orderWindowFront();
                require_noerr(err, CantCallFunction);
                
CantCallFunction:
CantInitializeCocoa:
                break;
            default:
                break;
        }
    }

    return err;
}

int main(int argc, char* argv[])
{
    IBNibRef 		nibRef;
    WindowRef 		window;
    OSStatus		err;
    EventTypeSpec   cmdEvent	= {kEventClassCommand, kEventCommandProcess};

    // Create a Nib reference passing the name of the nib file (without the .nib extension)
    // CreateNibReference only searches into the application bundle.
    err = CreateNibReference(CFSTR("main"), &nibRef);
    require_noerr( err, CantGetNibRef );
    
    // Once the nib reference is created, set the menu bar. "MainMenu" is the name of the menu bar
    // object. This name is set in InterfaceBuilder when the nib is created.
    err = SetMenuBarFromNib(nibRef, CFSTR("MenuBar"));
    require_noerr( err, CantSetMenuBar );
    
    // Then create a window. "MainWindow" is the name of the window object. This name is set in 
    // InterfaceBuilder when the nib is created.
    err = CreateWindowFromNib(nibRef, CFSTR("MainWindow"), &window);
    require_noerr( err, CantCreateWindow );

    // We don't need the nib reference anymore.
    DisposeNibReference(nibRef);
    
    // The window was created hidden so show it.
    ShowWindow( window );
    
    InstallApplicationEventHandler(NewEventHandlerUPP(appCommandHandler), GetEventTypeCount(cmdEvent), &cmdEvent, 0, NULL);

    // Call the event loop
    RunApplicationEventLoop();
    
CantCreateWindow:
CantSetMenuBar:
CantGetNibRef:
	return err;
}

